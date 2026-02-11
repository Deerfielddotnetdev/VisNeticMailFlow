// URLDecoder.cpp: implementation of the CURLDecoder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "URLDecoder.h"
#include "MessageIO.h"
#include "AttachFns.h"
#include "EMSMutex.h"
#include ".\MailFlowServer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CURLDecoder::CURLDecoder()
{
}

CURLDecoder::~CURLDecoder()
{
}

void CURLDecoder::SetBuffer( TCHAR* szBuffer, int nLength )
{
	ptr = szBuffer;
	end = ptr + nLength;
	keyptr = szKey;
	keyend = szKey + sizeof(szKey);
	m_bGotKey = false;
	m_bGotEqualSign = false;
}

int CURLDecoder::GetParameter( ISAPIPARAM& param )
{
	while( ptr < end )
	{
		if( m_bGotEqualSign == false && *ptr == '=' )
		{
			// null terminate key
			*keyptr = '\0';
			m_bGotKey = true;
			// set the starting point of the value
			start = ptr + 1;
			m_bGotEqualSign = true;
		}
		else if ( *ptr == '&' || *ptr == '\0' || *ptr == '\r' || *ptr == '\n' )
		{
			if( m_bGotKey )
			{
				SetParameter( param );
				return 1;
			}
		}
		else if ( *ptr == '+' )
		{
			if( !m_bGotKey && keyptr < keyend )	
			{
				*keyptr++ = ' ';
			}
		}
		else if ( *ptr == '%' )
		{
			if( !m_bGotKey && keyptr < keyend )	
			{
				int NibbleHigh,NibbleLow;

				if( end - ptr > 2 )
				{
					ptr++;
					NibbleHigh = HexDigitToInt(*ptr);
					ptr++;
					NibbleLow = HexDigitToInt(*ptr);
					if( NibbleHigh == -1 || NibbleLow == -1)
					{
						// Error
					}
					else
					{
						*keyptr++ = toupper(( NibbleHigh << 4 | NibbleLow ));
					}
				}
			}
		}
		else
		{
			if( !m_bGotKey && keyptr < keyend )
			{
				*keyptr++ = toupper(*ptr);
			}
		}

		ptr++;
	}	

	if( m_bGotKey )
	{
		SetParameter( param );
		return 1;
	}

	return 0;
}


void CURLDecoder::SetParameter( ISAPIPARAM& param )
{
	param.m_szValue = start;
	param.nLength = ptr - start;
	param.m_bIsFile = false;

	// reset algorithm
	m_bGotKey = false;
	keyptr = szKey;
	m_bGotEqualSign = false;
}


int CURLDecoder::HexDigitToInt( char c )
/*++

Purpose:
    HexDigitToInt simply converts a hex-based character to an int.

Arguments:
    tc - character to convert

Returns:
    binary value of the character (0-15)
    -1 if the character is not hex digit

--*/
{
    if ( c >= '0' && c <= '9' ) {
        return ( c - '0' );
    }

    if ( tolower( c ) >= 'a' && tolower( c ) <= 'f' ) {
        return ( tolower( c ) - 'a'  + 10 );
    }

    return -1;
}


int CURLDecoder::DecodeValue( ISAPIPARAM& param, TCHAR* szBuffer, int nBufferLength, bool bURLEncoded )
{
	keyptr = szBuffer;
	keyend = keyptr + (nBufferLength - 1);
	ptr = param.m_szValue;
	end = ptr + param.nLength;

	if( bURLEncoded )
	{
		while( ptr < end )
		{
			switch ( *ptr )
			{
				
			case '+':	

				if( keyptr < keyend )
				{
					*keyptr++ = ' ';
				}
				else
				{
					// throw an exception as the buffer isn't large enough for the paramater...
					THROW_EMS_EXCEPTION( E_ParameterTooLarge, _T("Parameter Too Large") );
				}

				break;

			case '%':

				if( keyptr < keyend )
				{
					int NibbleHigh,NibbleLow;
					
					if( end - ptr > 2 )
					{
						ptr++;
						NibbleHigh = HexDigitToInt(*ptr);
						
						ptr++;
						NibbleLow = HexDigitToInt(*ptr);
						
						if( NibbleHigh != -1 || NibbleLow != -1)
							*keyptr++ = (char)( NibbleHigh << 4 | NibbleLow );
					}
				}

				break;

			case '&':
			case '\0':
			case '\r':
			case '\n':

				break;

			default:

				if( keyptr < keyend )
				{
					*keyptr++ = *ptr;
				}
				else
				{
					// throw an exception as the buffer isn't large enough for the paramater...
					THROW_EMS_EXCEPTION( E_ParameterTooLarge, _T("Parameter Too Large") );
				}
			}

			ptr++;
		}	
	}
	else
	{
		while( ptr < end )
		{
			if( keyptr < keyend )
			{
				*keyptr++ = *ptr++;
			}
			else
			{
				// throw an exception as the buffer isn't large enough for the paramater...
				THROW_EMS_EXCEPTION( E_ParameterTooLarge, _T("Parameter Too Large") );
			}
		}
	}

	*keyptr = '\0';

	return (keyptr - szBuffer);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves form post data into a tstring	              
\*--------------------------------------------------------------------------*/
bool CURLDecoder::GetFormString(LPCTSTR szField, tstring& sValue, bool bOptional, bool bSpell)
{
	ISAPIPARAM param;
	
	if (FindFormField(szField, param, bSpell))
	{	
		if (param.nLength > 0)
		{
			sValue.resize(param.nLength + 1);
			int nSize = DecodeValue(param, (TCHAR*) sValue.data(), param.nLength + 1);
			sValue.resize(nSize);
		}
		else
		{
			sValue.resize(0);
		}
	}
	else
	{
		if (!bOptional)
		{
			CEMSString sError;
			sError.Format(EMS_STRING_E_FormFieldNotFound, szField);
			THROW_EMS_EXCEPTION( E_FormFieldNotFound, sError );
		}
			
		return false;
	}
	
	return true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves URL paramater data into a tstring	              
\*--------------------------------------------------------------------------*/
bool CURLDecoder::GetURLString(LPCTSTR szField, tstring& sValue, bool bOptional)
{
	ISAPIPARAM param;
	
	if (FindURLParam(szField, param))
	{
		if (param.nLength > 0)
		{
			sValue.resize(param.nLength + 1);
			int nSize = DecodeValue(param, (TCHAR*) sValue.data(), param.nLength + 1, true );
			sValue.resize(nSize);
		}
		else
		{
			sValue.resize(0);
		}
	}
	else
	{
		if (!bOptional)
		{
			CEMSString sError;
			sError.Format(EMS_STRING_E_URLVarNotFound, szField);
			THROW_EMS_EXCEPTION( E_URLVarNotFound, sError );
		}
		
		return false;
	}
	
	return true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves form post data into a TCHAR buffer                     
\*--------------------------------------------------------------------------*/
bool CURLDecoder::GetFormTCHAR(LPCTSTR szField, TCHAR* szValue, int szValueLen, bool bOptional)
{
	ISAPIPARAM param;
	
	if (FindFormField(szField, param))
	{
		// to do - throw back an error if the data was too long
		DecodeValue(param, szValue, szValueLen);
	}
	else
	{
		if (!bOptional)
		{
			CEMSString sError;
			sError.Format(EMS_STRING_E_FormFieldNotFound, szField);
			THROW_EMS_EXCEPTION( E_FormFieldNotFound, sError );
		}
		
		return false;
	}
	
	return true;
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves form post data into a TCHAR buffer
||				Does not limit length...                     
\*--------------------------------------------------------------------------*/
bool CURLDecoder::GetFormTCHAR(LPCTSTR szField, TCHAR** ppBuffer, long& nLength, long& nAllocated, bool bOptional)
{
	ISAPIPARAM param;
	
	if (FindFormField(szField, param))
	{
		nLength = param.nLength + 1;

		// do we need to allocate more memory
		if( nAllocated < nLength )
		{
			// Allocate the new buffer
			TCHAR* wcsNewBuffer = (TCHAR*)calloc( nLength, 1 );
			
			if( nAllocated > 0 )
			{
				// Free the old buffer
				free( *ppBuffer );
			}
			
			// Set the old pointer to the new buffer
			*ppBuffer = wcsNewBuffer;
			nAllocated = nLength;
		}
		
		nLength = DecodeValue(param, *ppBuffer, nLength);
	}
	else
	{
		if (!bOptional)
		{
			CEMSString sError;
			sError.Format(EMS_STRING_E_FormFieldNotFound, szField);
			THROW_EMS_EXCEPTION( E_FormFieldNotFound, sError );
		}
		
		return false;
	}
	
	return true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves URL paramater data into a TCHAR buffer             
\*--------------------------------------------------------------------------*/
bool CURLDecoder::GetURLTCHAR(LPCTSTR szField, TCHAR* szValue, int szValueLen, bool bOptional)
{
	ISAPIPARAM param;
	
	if (FindURLParam(szField, param))
	{
		// to do - throw back an error if the data was too long
		DecodeValue( param, szValue, szValueLen, true );
	}
	else
	{
		if (!bOptional)
		{
			CEMSString sError;
			sError.Format(EMS_STRING_E_URLVarNotFound, szField);
			THROW_EMS_EXCEPTION( E_URLVarNotFound, sError );
		}
		
		return false;
	}
	
	return true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves form post data into an int             
\*--------------------------------------------------------------------------*/
bool CURLDecoder::GetFormLong(LPCTSTR szField, int& nValue, bool bOptional)
{
	ISAPIPARAM param;
	
	if (FindFormField(szField, param))
	{
		TCHAR szTemp[12];
		
		DecodeValue(param, (TCHAR*) &szTemp, 11);		
		nValue = _ttoi(szTemp);
	}
	else
	{
		if (!bOptional)
		{
			CEMSString sError;
			sError.Format(EMS_STRING_E_FormFieldNotFound, szField);
			THROW_EMS_EXCEPTION( E_FormFieldNotFound, sError );
		}
		
		return false;
	}
	
	return true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves URL paramater data into an int             
\*--------------------------------------------------------------------------*/
bool CURLDecoder::GetURLLong(LPCTSTR szField, int& nValue, bool bOptional)
{
	ISAPIPARAM param;
	
	if (FindURLParam(szField, param))
	{
		TCHAR szTemp[12];
		
		DecodeValue( param, (TCHAR*) &szTemp, 11, true );		
		nValue = _ttoi(szTemp);
	}
	else
	{
		if (!bOptional)
		{
			CEMSString sError;
			sError.Format(EMS_STRING_E_URLVarNotFound, szField);
			THROW_EMS_EXCEPTION( E_URLVarNotFound, sError );
		}
		
		return false;
	}
	
	return true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves form post data into an int             
\*--------------------------------------------------------------------------*/
bool CURLDecoder::GetFormLong(LPCTSTR szField, unsigned char& nValue, bool bOptional)
{
	ISAPIPARAM param;
	
	if (FindFormField(szField, param))
	{
		TCHAR szTemp[12];
		
		DecodeValue(param, (TCHAR*) &szTemp, 11);		
		nValue = (unsigned char) _ttoi(szTemp);
	}
	else
	{
		if (!bOptional)
		{
			CEMSString sError;
			sError.Format(EMS_STRING_E_FormFieldNotFound, szField);
			THROW_EMS_EXCEPTION( E_FormFieldNotFound, sError );
		}
		
		return false;
	}
	
	return true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves form post data into an bit (unsigned char)            
\*--------------------------------------------------------------------------*/
bool CURLDecoder::GetURLLong(LPCTSTR szField, unsigned char& nValue, bool bOptional)
{
	ISAPIPARAM param;
	
	if (FindURLParam(szField, param))
	{
		TCHAR szTemp[12];
		
		DecodeValue( param, (TCHAR*) &szTemp, 11, true );		
		nValue = _ttoi(szTemp) ? 1 : 0;
	}
	else
	{
		if (!bOptional)
		{
			CEMSString sError;
			sError.Format(EMS_STRING_E_URLVarNotFound, szField);
			THROW_EMS_EXCEPTION( E_URLVarNotFound, sError );
		}
		
		return false;
	}
	
	return true;
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves form field data into a bit (unsigned char)		              
\*--------------------------------------------------------------------------*/
void CURLDecoder::GetFormBit(LPCTSTR szField, unsigned char& nValue)
{
	ISAPIPARAM param;
	nValue = FindFormField(szField, param) ? TRUE : FALSE;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetFormFile - Attempt to save the file specified in the form post
//				 by the field szField to a file named sActualFilename.
//				 sOrigFilename contains the original name of the file as
//				 specified by the form field.
//				 Returns 0 if successful, or an error code on failure
// 
////////////////////////////////////////////////////////////////////////////////
int CURLDecoder::GetFormFile( LPCTSTR szField, tstring& sOrigFilename, 
							 tstring& sActualFilename, tstring& sMimeType, 
							 tstring& sMimeSubType, int& nBytes, bool bIsSig )
{
	OutputDebugString("CURLDecoder::GetFormFile - entered\n");

	ISAPIPARAM param;
	COutboundAttachMutex mutex;
	CMessageIO msgIO;
	nBytes = 0;

	if (!FindFormField(szField, param))
		return E_FormFieldNotFound;

	if ( !m_bURLEncoded )
	{
		if( param.m_Filename.length() < 1 )
			return E_FormFieldNotFound;

		DWORD dwRet,dwBytes,dwWritten = 0;
				
		sOrigFilename = param.m_Filename;
		sActualFilename = param.m_Filename;
		
		// break the content type apart
		size_t nPos = param.m_ContentType.find('/', 0);
		
		if (nPos != tstring::npos)
		{
			sMimeType = param.m_ContentType.substr(0, nPos);
			sMimeSubType = param.m_ContentType.substr( nPos + 1, param.m_ContentType.size() - (nPos+1));
			if ( sMimeSubType == "pjpeg")
				sMimeSubType = "jpeg";
		}
		else
		{
			sMimeType = _T("TEXT");
			sMimeSubType = _T("PLAIN");
		}
		
		// obtain a unique filename
		if( !mutex.AcquireLock( 10000 ) )
		{
			CEMSString sError;
			sError.Format(EMS_STRING_ERROR_ATTACH_PATH, sOrigFilename.c_str());
			THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
		}

		OutputDebugString("CURLDecoder::GetFormFile - get outbound attach path\n");
		if ( bIsSig )
		{
			if(!msgIO.GetOutboundAttachPath(sActualFilename, 0))
			{
				CEMSString sError;
				sError.Format(EMS_STRING_ERROR_ATTACH_PATH, sOrigFilename.c_str());
				THROW_EMS_EXCEPTION(E_WritingAttachment, sError);
			}
		}
		else
		{
			if(!msgIO.GetOutboundAttachPath(sActualFilename, 1))
			{
				CEMSString sError;
				sError.Format(EMS_STRING_ERROR_ATTACH_PATH, sOrigFilename.c_str());
				THROW_EMS_EXCEPTION(E_WritingAttachment, sError);
			}
		}

		TCHAR dir[_MAX_DIR];
		TCHAR drive[_MAX_DRIVE];

		_tsplitpath(sActualFilename.c_str(), drive, dir, NULL, NULL);

		OutputDebugString("CURLDecoder::GetFormFile - MailFlowServer ensure path\n");
		MailFlowServer::EnsurePathExists(tstring(drive) + tstring(dir));
		
		// create the file
		HANDLE hFile = CreateFile( sActualFilename.c_str(), GENERIC_WRITE, 0, NULL, 
								CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		
		// error creating file
		if ( hFile == INVALID_HANDLE_VALUE || hFile == NULL)
		{
			if ( hFile )
				CloseHandle( hFile );

			CEMSString sError;
			sError.Format(EMS_STRING_ERROR_ATTACH_CREATEFILE, sOrigFilename.c_str(), GetLastError());
			THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
		}

		mutex.ReleaseLock();

		// loop until all the data is written
		while( true )
		{
			dwRet = WriteFile( hFile, (BYTE*)(param.m_szValue) + dwWritten, param.nLength - dwWritten, &dwBytes, NULL );
			
			if( dwRet == 0 )
			{
				DWORD dwErr = GetLastError();

				CloseHandle( hFile );

				// delete the temporary file that we created
				DeleteFile( sActualFilename.c_str() );

				if( dwErr == ERROR_HANDLE_DISK_FULL || dwErr == ERROR_DISK_FULL )
					return E_DiskFull;

				// error writing the file
				CEMSString sError;
				sError.Format( EMS_STRING_ERROR_ATTACH_WRITEFILE, sOrigFilename.c_str(), dwErr );
				THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
			}
			else
			{
				dwWritten += dwBytes;
				if((int)dwWritten >= param.nLength )
				{
					nBytes = dwWritten;
					CloseHandle( hFile );
					return 0;
				}
			}
		}
	}

	return 0;
}
////////////////////////////////////////////////////////////////////////////////
// 
// GetFormFile - Attempt to save the file specified in the form post
//				 by the field szField to a file named sActualFilename.
//				 sOrigFilename contains the original name of the file as
//				 specified by the form field.
//				 Returns 0 if successful, or an error code on failure
// 
////////////////////////////////////////////////////////////////////////////////
int CURLDecoder::GetFormNoteFile( LPCTSTR szField, tstring& sOrigFilename, 
							 tstring& sActualFilename, tstring& sMimeType, 
							 tstring& sMimeSubType, int& nBytes )
{
	OutputDebugString("CURLDecoder::GetFormNoteFile - entered\n");

	ISAPIPARAM param;
	COutboundAttachMutex mutex;
	CMessageIO msgIO;
	nBytes = 0;

	if (!FindFormField(szField, param))
		return E_FormFieldNotFound;

	if ( !m_bURLEncoded )
	{
		if( param.m_Filename.length() < 1 )
			return E_FormFieldNotFound;

		DWORD dwRet,dwBytes,dwWritten = 0;
				
		sOrigFilename = param.m_Filename;
		sActualFilename = param.m_Filename;
		
		// break the content type apart
		size_t nPos = param.m_ContentType.find('/', 0);
		
		if (nPos != tstring::npos)
		{
			sMimeType = param.m_ContentType.substr(0, nPos);
			sMimeSubType = param.m_ContentType.substr( nPos + 1, param.m_ContentType.size() - (nPos+1));
		}
		else
		{
			sMimeType = _T("TEXT");
			sMimeSubType = _T("PLAIN");
		}
		
		// obtain a unique filename
		if( !mutex.AcquireLock( 10000 ) )
		{
			CEMSString sError;
			sError.Format(EMS_STRING_ERROR_ATTACH_PATH, sOrigFilename.c_str());
			THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
		}

		OutputDebugString("CURLDecoder::GetFormFile - get note attach path\n");
		if(!msgIO.GetNoteAttachPath(sActualFilename, 1))
		{
			CEMSString sError;
			sError.Format(EMS_STRING_ERROR_ATTACH_PATH, sOrigFilename.c_str());
			THROW_EMS_EXCEPTION(E_WritingAttachment, sError);
		}

		TCHAR dir[_MAX_DIR];
		TCHAR drive[_MAX_DRIVE];

		_tsplitpath(sActualFilename.c_str(), drive, dir, NULL, NULL);

		OutputDebugString("CURLDecoder::GetFormFile - MailFlowServer ensure path\n");
		MailFlowServer::EnsurePathExists(tstring(drive) + tstring(dir));
		
		// create the file
		HANDLE hFile = CreateFile( sActualFilename.c_str(), GENERIC_WRITE, 0, NULL, 
								CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		
		// error creating file
		if ( hFile == INVALID_HANDLE_VALUE || hFile == NULL)
		{
			if ( hFile )
				CloseHandle( hFile );

			CEMSString sError;
			sError.Format(EMS_STRING_ERROR_ATTACH_CREATEFILE, sOrigFilename.c_str(), GetLastError());
			THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
		}

		mutex.ReleaseLock();

		// loop until all the data is written
		while( true )
		{
			dwRet = WriteFile( hFile, (BYTE*)(param.m_szValue) + dwWritten, param.nLength - dwWritten, &dwBytes, NULL );
			
			if( dwRet == 0 )
			{
				DWORD dwErr = GetLastError();

				CloseHandle( hFile );

				// delete the temporary file that we created
				DeleteFile( sActualFilename.c_str() );

				if( dwErr == ERROR_HANDLE_DISK_FULL || dwErr == ERROR_DISK_FULL )
					return E_DiskFull;

				// error writing the file
				CEMSString sError;
				sError.Format( EMS_STRING_ERROR_ATTACH_WRITEFILE, sOrigFilename.c_str(), dwErr );
				THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
			}
			else
			{
				dwWritten += dwBytes;
				if((int)dwWritten >= param.nLength )
				{
					nBytes = dwWritten;
					CloseHandle( hFile );
					return 0;
				}
			}
		}
	}
	else
	{
		// MJM - ICEWarp web server (control.exe) handles uploaded files in 
		// a non-standard way...
		tstring sTempPath;
		tstring sType;
		tstring sFieldName;

		// get the path to the temporary file
		// created by the web server
		if (!FindFormField(szField, param))
			return E_FormFieldNotFound;

		GetFormString( szField, sTempPath );

		// get the original filename
		sFieldName = szField;
		sFieldName += _T("_NAME");

		if (!FindFormField(sFieldName.c_str(), param))
			return E_FormFieldNotFound;

		GetFormString( sFieldName.c_str(), sOrigFilename );
		sActualFilename = sOrigFilename;

		// get the content type
		sFieldName = szField;
		sFieldName += _T("_TYPE");

		if (!FindFormField(sFieldName.c_str(), param))
			return E_FormFieldNotFound;

		GetFormString( sFieldName.c_str(), sType );

		// break the content type apart
		size_t nPos = sType.find('/', 0);

		if (nPos != tstring::npos)
		{
			sMimeType = sType.substr(0, nPos);
			sMimeSubType = sType.substr( nPos + 1, sType.size() - (nPos+1));
		}
		else
		{
			sMimeType = _T("TEXT");
			sMimeSubType = _T("PLAIN");
		}

		// get the size of the file
		sFieldName = szField;
		sFieldName += _T("_SIZE");

		if (!FindFormField(sFieldName.c_str(), param))
			return E_FormFieldNotFound;

		GetFormLong( sFieldName.c_str(), nBytes );

		// obtain a unique filename
		if( !mutex.AcquireLock( 10000 ) )
		{
			CEMSString sError;
			sError.Format(EMS_STRING_ERROR_ATTACH_PATH, sOrigFilename.c_str());
			THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
		}

		if(!msgIO.GetOutboundAttachPath(sActualFilename, 1))
		{
			CEMSString sError;
			sError.Format(EMS_STRING_ERROR_ATTACH_PATH, sOrigFilename.c_str());
			THROW_EMS_EXCEPTION(E_WritingAttachment, sError);
		}

		TCHAR dir[_MAX_DIR];
		TCHAR drive[_MAX_DRIVE];

		_tsplitpath(sActualFilename.c_str(), drive, dir, NULL, NULL);
		MailFlowServer::EnsurePathExists(tstring(drive) + tstring(dir));

		// move the temp file to the filename which we reserved
		if (!MoveFileEx( sTempPath.c_str(), sActualFilename.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH ))
		{
			CEMSString sError;
			sError.Format(EMS_STRING_ERROR_ATTACH_PATH, sOrigFilename.c_str());
			THROW_EMS_EXCEPTION(E_WritingAttachment, sError);
		}
	}

	return 0;
}

BOOL CURLDecoder::FindURLParam( LPCTSTR lpszKey, ISAPIPARAM& param )
{

	TCHAR szKeyName[MAX_KEY_LENGTH];
	TCHAR *p,*q,*e;
	
	// Convert key names to upper case and limit the length
	p = (TCHAR*) lpszKey;
	q = szKeyName;
	e = szKeyName + MAX_KEY_LENGTH - 1;

	while( *p != _T('\0') && q < e )
	{
		*q++ = toupper( *p++ );
	}

	// zero terminate
	*q = 0;
	
	map<tstring,ISAPIPARAM>::iterator iter = m_URL.find( (tstring)szKeyName );

	if( iter != m_URL.end() )
	{
		param = (iter->second);
		return TRUE;
	}

	// not found
	return FALSE;
}

BOOL CURLDecoder::FindFormField( LPCTSTR lpszKey, ISAPIPARAM& param, bool bSpell )
{
	TCHAR szKeyName[MAX_KEY_LENGTH];
	TCHAR *p,*q,*e;
	
	// Convert key names to upper case and limit the length
	p = (TCHAR*) lpszKey;
	q = szKeyName;
	e = szKeyName + MAX_KEY_LENGTH - 1;

	while( *p != _T('\0') && q < e )
	{
		*q++ = toupper( *p++ );
	}

	// zero terminate
	*q = 0;

	if(bSpell){
		
		map<tstring,ISAPIPARAM>::iterator itertest;
	
		for(itertest = m_Form.begin();
			itertest != m_Form.end();
			++itertest){

				//DebugReporter::Instance().DisplayMessage(itertest->first.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			}

		/*dca::String up;
		up.Format("* CURLDecoder::FindFormField - Spell checker test... in key = %s, con key = %s", lpszKey, szKeyName);
		DebugReporter::Instance().DisplayMessage(up.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
	}
	
	map<tstring,ISAPIPARAM>::iterator iter = m_Form.find( (tstring)szKeyName );

	if( iter != m_Form.end() )
	{
		param = (iter->second);
		return TRUE;
	}

	// not found
	return FALSE;	
}

