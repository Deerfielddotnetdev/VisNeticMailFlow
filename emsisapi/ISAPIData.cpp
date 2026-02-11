/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ISAPIData.cpp,v 1.2.4.1 2006/07/18 12:55:03 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ISAPIData.h"
#include "UploadMap.h"


/*---------------------------------------------------------------------------\                    
||  Comments:	Constructor	              
\*--------------------------------------------------------------------------*/
CISAPIData::CISAPIData(CODBCQuery& query, CXMLCache& XMLCache, CSessionMap& SessionMap,CLicenseMgr& licenseMgr )
	:m_query(query)
	,m_XMLCache(XMLCache)
	,m_SessionMap(SessionMap)
	,m_LicenseMgr(licenseMgr)
{
	m_pECB	   = NULL;
	m_pSession = NULL;
	m_pBrowserSession = NULL;
	m_UploadID = 0;
	m_bXMLPost = false;
	m_bUseExceptionEMS = true;
}

/*---------------------------------------------------------------------------\                  
||  Comments:	Destructor	              
\*--------------------------------------------------------------------------*/
CISAPIData::~CISAPIData()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Initialize	              
\*--------------------------------------------------------------------------*/
void CISAPIData::Initialize( EXTENSION_CONTROL_BLOCK* pECB )
{
	// reset vars
	m_UploadID = 0;
	m_bXMLPost = false;
	m_bUseExceptionEMS = true;

	m_URL.clear();
	m_Form.clear();
	m_XMLFormMap.clear();

	//DcaTrace(_T("* CISAPIData::Initialize - Query String [ %s ]."),pECB->lpszQueryString);

	// find the first slash starting from the end
	TCHAR* p = _tcsrchr( pECB->lpszPathInfo, _T('/') );

	if( p == NULL )
	{
		// ? what to do if no slash found?
		m_sPage = pECB->lpszPathInfo;
	}
	else	
	{
		// advance past the '/'
		m_sPage = p + 1;
	}

	// trim off the extension
	int dotloc = m_sPage.find_first_of( _T('.') );
	if( dotloc != tstring::npos )
		m_sPage.resize( dotloc );
	

	// init XML to be translated w/ XSL
	m_xmlgen.SetDoc( NULL );
	m_xmlgen.AddElem(_T("root"));
	
	m_pECB = pECB;

	try
	{
		GetParameters( m_bURLEncoded );
	}
	catch(...)
	{
		if ( m_UploadID )
		{
			CUploadMap::GetInstance().UpdateProgress( m_UploadID, -1 );
		}

		throw;
	}

	// Defaults
	m_sChgNotSavedMsg.assign( _T("Your changes have not been saved!") );

	// init XML to be appended to HTML
	m_xmldoc.SetDoc( NULL );
}

TCHAR* CISAPIData::ClientBuffer(void)
{
	return m_bClientDataInMemFile ? (TCHAR*) m_MemFile.GetBuffer() : (TCHAR*) m_pECB->lpbData;
}

unsigned int CISAPIData::ClientBufferLength(void)
{
	return m_bClientDataInMemFile ? m_MemFileBytesUsed : m_pECB->cbAvailable;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetParameters
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIData::GetParameters( bool& bURLEncoded )
{
	ISAPIPARAM param;
	DWORD dwSize;
	
	// always get the URL parameters
	SetBuffer( (TCHAR*)(m_pECB->lpszQueryString), _tcslen(m_pECB->lpszQueryString) );

	//DcaTrace(_T("# CISAPIData::GetParameters - Query String [ %s ]."),m_pECB->lpszQueryString);

	while ( GetParameter( param ) )
	{
		m_URL[GetKey()] = param;
	}
	
	// look for an unique ID, which allows us to display
	// the progress of uploads
	GetURLLong( _T("UploadID"), m_UploadID, true );
	
	// Check if all data has been read from the client, if not read
	// the remaining data into a memory mapped file.
	if( m_pECB->cbTotalBytes == m_pECB->cbAvailable )
	{
		// all the client data has been read by the web server
		m_bClientDataInMemFile = false;
	}
	else
	{
		// read client data into memory file.
		m_bClientDataInMemFile = true;

		if( ReadClientData() != 0 )
			THROW_EMS_EXCEPTION( E_InvalidRequest, _T("Error reading from client") );
	}

	// get the content type
	m_szContentType[0] = _T('\0');
	dwSize = sizeof(m_szContentType) - sizeof(TCHAR);

	m_pECB->GetServerVariable( m_pECB->ConnID, "CONTENT_TYPE", m_szContentType, &dwSize );

	// is this a form post?
	if ( _tcsnicmp( m_pECB->lpszMethod, "POST", 4 ) == 0 )
	{
		//DebugReporter::Instance().DisplayMessage("* CISAPIData::GetParameters - this is a form post", DebugReporter::ISAPI, GetCurrentThreadId());

		//dca::String x;
		//x.Format("* CISAPIData::GetParameters - CONTENT_TYPE = %s", m_szContentType);
		//DebugReporter::Instance().DisplayMessage(x.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

		if ( m_URL.find( tstring(_T("POSTXML")) ) != m_URL.end() )
		{
			// form post is an XML document
			//DebugReporter::Instance().DisplayMessage("* CISAPIData::GetParameters - this is a xml form post", DebugReporter::ISAPI, GetCurrentThreadId());
			m_bXMLPost = true;
			m_bUseExceptionEMS = false;
			bURLEncoded = false;
			ParseXMLPost();
		}
		else if( ( _tcsnicmp( m_szContentType, _T("application/x-www-form-urlencoded"), 
			_tcslen( _T("application/x-www-form-urlencoded") ) ) == 0 ) )
		{
			// form post is url encoded
			//DebugReporter::Instance().DisplayMessage("* CISAPIData::GetParameters - this is a encoded form post", DebugReporter::ISAPI, GetCurrentThreadId());
			bURLEncoded = true;
			
			// Get form fields if the content-type is url encoded - uses CURLDecoder
			//x.Format("* CISAPIData::GetParameters - %s", ClientBuffer());
			//DebugReporter::Instance().DisplayMessage(x.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			SetBuffer( ClientBuffer(), ClientBufferLength() );
			while ( GetParameter( param ) )
			{
				//if( (strcmpi(GetKey(), "PASSWORD")) ){
					//dca::String up;
					//up.Format("* CISAPIData::GetParameters - key = %s", GetKey());
					//DebugReporter::Instance().DisplayMessage(up.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					//up.clear();
					//up.Format("* CISAPIData::GetParameters - value = %s, length = %d", param.m_szValue, param.nLength);
					//DebugReporter::Instance().DisplayMessage(up.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				//}
				m_Form[GetKey()] = param;
			}
		}
		else if( _tcsnicmp( m_szContentType, _T("multipart/form-data"), 
			_tcslen(_T("multipart/form-data")) ) == 0 )
		{
			// form post is multipart
			//DebugReporter::Instance().DisplayMessage("* CISAPIData::GetParameters - this is a multipart form post", DebugReporter::ISAPI, GetCurrentThreadId());
			bURLEncoded = false;
			ParseMultiPart();
		}
		else{
			//x.Format("- CISAPIData::GetParameters - CONTENT_TYPE not used");
			//DebugReporter::Instance().DisplayMessage(x.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
	else{

		//DebugReporter::Instance().DisplayMessage("* CISAPIData::GetParameters - this is not a form post", DebugReporter::ISAPI, GetCurrentThreadId());

	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds the URL paramaters to the XML             
\*--------------------------------------------------------------------------*/
void CISAPIData::AddURLParamsToXML( CMarkupSTL& xml )
{
	// add the URL paramaters to the XML
	xml.AddChildElem( _T("URLPARAMETERS") );
	xml.IntoElem();
	
	map<tstring,ISAPIPARAM>::iterator iter;
	
	for( iter = m_URL.begin(); iter != m_URL.end(); iter++ )
	{
		TCHAR* buffer = new TCHAR[iter->second.nLength+1];
		DecodeValue( iter->second, buffer, iter->second.nLength+1, true );
		CEMSString sTemp(buffer);
		sTemp.EscapeHTML();
		_tcscpy( buffer, sTemp.c_str() );

		sTemp.Format( _T("%s"),iter->first.c_str());
		sTemp.EscapeHTML();
		xml.AddChildElem( sTemp.c_str() );
		xml.SetChildData( buffer, TRUE );
	}
	
	xml.OutOfElem();
}

////////////////////////////////////////////////////////////////////////////////
// 
// ReadClientData
// 
////////////////////////////////////////////////////////////////////////////////
int CISAPIData::ReadClientData( void )
{
	int nRet = 0;
	DWORD dwBuf;
	
	// Make sure we have enough space
	if( m_MemFile.GetLength() < m_pECB->cbTotalBytes )
	{
		// Reallocate the memory map
		m_MemFile.Map( m_pECB->cbTotalBytes );
	}
	
	// copy the available buffer into the memory mapped file
	CopyMemory( m_MemFile.GetBuffer(), m_pECB->lpbData, m_pECB->cbAvailable );
	m_MemFileBytesUsed = m_pECB->cbAvailable;
	
	// set upload progress
	if ( m_UploadID )
	{
		float Percent = ((float) m_MemFileBytesUsed / (float) m_pECB->cbTotalBytes) * 100;
		CUploadMap::GetInstance().UpdateProgress( m_UploadID, (int)Percent );
	}

	// now loop until all client data has been read
	while( m_pECB->cbTotalBytes > m_MemFileBytesUsed )
	{
		dwBuf = m_pECB->cbTotalBytes - m_MemFileBytesUsed;

		if( dwBuf > 65536 )
			dwBuf = 65536;

		if( !m_pECB->ReadClient( m_pECB->ConnID, 
			                     m_MemFile.GetBuffer() + m_MemFileBytesUsed, 
								 &dwBuf ) )
		{
			nRet = -1;
			break;
		}
		else
		{
			if( dwBuf > 0 )
			{
				m_MemFileBytesUsed += dwBuf;

				// set upload progress
				if ( m_UploadID )
				{
					int nPercent = CUploadMap::GetInstance().GetProgress( m_UploadID );
					if(nPercent == 101)
					{
						nRet = 0;
						break;
					}
					float Percent = ((float) m_MemFileBytesUsed / (float) m_pECB->cbTotalBytes) * 100;
					CUploadMap::GetInstance().UpdateProgress( m_UploadID, (int)Percent );
				}
			}
			else 
			{
				nRet = -2;
				break;
			}
		}
	}
	
	return nRet;
}



////////////////////////////////////////////////////////////////////////////////
// 
// ParseMultiPart - parse multipart/form-data encoded form data 
// 
////////////////////////////////////////////////////////////////////////////////
int CISAPIData::ParseMultiPart( void )
{
	ISAPIPARAM param;
	TCHAR* p;
	TCHAR* e;
	tstring sKey;
	bool bFoundPart = false;
	tstring::size_type pos;

	GetBoundary();

	p = ClientBuffer() + 1;
	e = p + ClientBufferLength();

	while( p < e )
	{
		if( (*p == _T('-')) && _tcsncmp( --p, m_sBoundary.c_str(), m_sBoundary.size() ) == 0 )
		{
			if( bFoundPart )
			{
				param.nLength = p - param.m_szValue - 2;

				if( param.nLength < 0)
					param.nLength = 0;
				

				m_Form[sKey.c_str()] = param;
				bFoundPart = false;
			}

			p += m_sBoundary.size();

			// Look for the special sequence that indicates the last boundary
			if( _tcsncmp( p, _T("--"), 2 ) == 0 )
			{
				return 0;
			}

			// Skip the carriage return - line feed
			if( _tcsncmp( p, _T("\r\n"), 2 ) == 0 )
				p += 2;

			// the leading spaces are important!
			if( ExtractQuotedField( p, e, _T(" name="), sKey, TRUE ) == 0 )
			{	
				param.m_bIsFile = ( ExtractQuotedField( p, e, _T(" filename="), param.m_Filename, FALSE ) == 0 ) ? 1 : 0;

				// Remove the path from the filename
				pos = param.m_Filename.find_last_of( _T('\\') );
				if( pos != tstring::npos )
				{
					param.m_Filename.erase( 0, pos+1 );
				}

				// advance to the CR-LF
				while( p < e && _tcsncmp( p, _T("\r\n"), 2 ) != 0 )
				{
					p++;
				}

				ExtractTag( p+2, e, _T("Content-Type:"), param.m_ContentType );
				
				// now find the start of the data
				while( p < e && _tcsncmp( p, _T("\r\n\r\n"), 4 ) != 0 )
				{
					p++;
				}
				if( p == e )
					return 0;
				
				p += 4;
				param.m_szValue = p;
				bFoundPart = true;
			}
		}
		else
		{
			p = p + 2;
		}
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Parses an XML document which contains form data.	              
\*--------------------------------------------------------------------------*/
void CISAPIData::ParseXMLPost()
{
	//wchar_t szBuffer[16];
	wstring sData;
	//UINT p;

	// as the post is encoded as UTF-8, convert the post data to UNICODE
	int nWcsSize = MultiByteToWideChar( CP_UTF8, 0, ClientBuffer(), ClientBufferLength(), NULL, 0  );
	sData.resize( nWcsSize, L' ' );
	MultiByteToWideChar( CP_UTF8, 0, ClientBuffer(), ClientBufferLength(), (LPWSTR)(sData.c_str()), nWcsSize  );

	// release the client buffer
	if ( m_bClientDataInMemFile )
	{
		m_MemFile.Close();
		m_bClientDataInMemFile = false;
	}

	// Convert any UNICODE chars to HTML equivalents
	/*for( p = 0; p < sData.size(); p++ )
	{
		if( sData[p] > 255 )
		{
			swprintf( szBuffer, L"&#%d;", sData[p] );
			sData.replace( p, 1, szBuffer );
		}
	}*/

	// convert the UNICODE buffer back into ANSI
	int nChrSize = WideCharToMultiByte( CP_UTF8, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
	char* szBuff = new char[nChrSize+1];
	WideCharToMultiByte( CP_UTF8, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
	szBuff[nChrSize] = 0;


	// load the client bugger into the XML object
	m_xmlgen.SetDoc( szBuff, nChrSize );

	// free memory
	delete[] szBuff;
	
	try
	{
		if ( m_xmlgen.FindElem( _T("root") ) )
		{
			if ( m_xmlgen.FindChildElem( _T("Form") ) )
			{
				tstring::iterator iter;
				tstring sFieldName;

				m_xmlgen.IntoElem();

				while ( m_xmlgen.FindChildElem() )
				{
					// convert the name to upper case
					sFieldName = m_xmlgen.GetChildTagName();

					for ( iter = sFieldName.begin(); iter != sFieldName.end(); iter++ )
					{
						*iter = toupper(*iter);
					}

					
					// add the parameter to the map
					m_XMLFormMap[sFieldName] = m_xmlgen.GetChildData(); 
				}
			}
		}
	}
	catch(...)
	{
		m_xmlgen.SetDoc( NULL );
		m_xmlgen.AddElem( _T("root") );
		throw;
	}

	m_xmlgen.SetDoc( NULL );
	m_xmlgen.AddElem( _T("root") );
}


////////////////////////////////////////////////////////////////////////////////
// 
// ExtractQuotedField - extract a quoted field from a line
// 
////////////////////////////////////////////////////////////////////////////////
int CISAPIData::ExtractQuotedField( TCHAR* p, TCHAR* e, TCHAR* szFieldName, 
								    tstring& sField, BOOL UpperCase )
{
	TCHAR szField[MAX_PATH];
	int FieldLength = _tcslen( szFieldName );
	TCHAR* q;
	TCHAR* f;

	while( p < e && *p != _T('\r') && *p != _T('\n') )
	{
		if( _tcsncmp( szFieldName, p, FieldLength )	== 0 )
		{
			p += FieldLength;
			if( *p == _T('"') )
			{
				p++;
				q = szField;
				f= q + MAX_PATH - 1;

				while( p < e && q < f && *p != _T('"') )
				{
					if( UpperCase )
					{
						*q++ = toupper( *p++ );
					}
					else
					{
						*q++ = *p++;
					}
				}
				*q = _T('\0');

				// copy the field name to the tstring
				sField = szField;

				return 0;
			}
		}

		p++;
	}

	return -1; // not found
}

////////////////////////////////////////////////////////////////////////////////
// 
// ExtractQuotedField - extract a tag from a line
// 
////////////////////////////////////////////////////////////////////////////////
int CISAPIData::ExtractTag( TCHAR* p, TCHAR* e, TCHAR* szFieldName, tstring& sField )
{
	TCHAR szField[MAX_PATH];
	int FieldLength = _tcslen( szFieldName );
	TCHAR* q;
	TCHAR* f;

	while( p < e && *p != _T('\r') && *p != _T('\n') && *p != _T(' ') )
	{
		if( _tcsncmp( szFieldName, p, FieldLength )	== 0 )
		{
			p += FieldLength;
			
			// skip past the white space
			while( *p == _T(' ') )
				p++;

			q = szField;
			f= q + MAX_PATH - 1;

			while( p < e && q < f && *p != _T('\r') && *p != _T('\n') && *p != _T(' ')  )
			{
				*q++ = *p++;
			}
			*q = _T('\0');

			// copy the field name to the tstring
			sField = szField;
			return 0;
		}

		p++;
	}

	return -1; // not found
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetBoundary - Get the Boundary string
// 
////////////////////////////////////////////////////////////////////////////////
int CISAPIData::GetBoundary(void)
{
	TCHAR szBoundary[256];
	TCHAR* p = m_szContentType;
	TCHAR* e;
	TCHAR* q = szBoundary;
	TCHAR* f = szBoundary + sizeof(szBoundary) - 1;

	enum ParseStates { LookingForBoundary, Boundary, FoundBoundary };
	int nState = LookingForBoundary;

	m_sBoundary = _T("--");

	e = m_szContentType + _tcslen( m_szContentType );

	while( nState < FoundBoundary && p < e && q < f)
	{
		switch( nState )	
		{			
		case LookingForBoundary:
			if( _tcsnicmp( p, _T("boundary="), 9 ) == 0 )
			{
				p += 9;
				nState = Boundary;
			}
			else
			{
				p++;
			}
			break;
			
		case Boundary:
			if( *p == _T(';') || *p == _T('\0') || *p == _T('\r') || *p == _T('\n') )
			{
				// We're done
				*q = _T('\0');
				nState = FoundBoundary;
			}
			else
			{
				*q++ = *p++;
			}
			break;				
		}
	}
	
	if( nState == Boundary )
	{
		*q = _T('\0');
		nState = FoundBoundary;
	}

	// If we didn't find the boundary in the Content-Type header, grab the first line 
	// of client data instead.
	if( nState != FoundBoundary )
	{
		p = ClientBuffer();
		q = szBoundary;
		e = p + ClientBufferLength();
		f = szBoundary + sizeof(szBoundary) - 1;

		while( p < e && q < f && *p != _T('\r') && *p != _T('\n') )
		{
			*q++ = *p++;
		}
		*q = 0;
	}

	m_sBoundary += szBoundary;

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// SetErrorXML
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIData::SetErrorXML( int ErrorCode, const char* szErrorType, const char* szErrorMsg, LPCTSTR szField )
{
	CEMSString sEscError( szErrorMsg );
	sEscError.EscapeJavascript();

	m_xmlgen.ResetPos();
	m_xmlgen.FindElem();
	m_xmlgen.AddChildElem(_T("error"));
	m_xmlgen.IntoElem();
	m_xmlgen.AddChildElem(_T("title"), szErrorType);
	m_xmlgen.AddChildElem(_T("code"), ErrorCode);
	m_xmlgen.AddChildElem(_T("message"), szErrorMsg);
	m_xmlgen.OutOfElem();

	// For XML posts
	m_xmlgen.AddChildElem(_T("postresults"));
	m_xmlgen.IntoElem();
	m_xmlgen.AddChildElem(_T("complete"), _T("false"));
	m_xmlgen.AddChildElem(_T("formfield"), szField);
	m_xmlgen.AddChildElem(_T("error"), sEscError.c_str());
	m_xmlgen.OutOfElem();
}



////////////////////////////////////////////////////////////////////////////////
// 
// GetXMLTCHAR
// 
////////////////////////////////////////////////////////////////////////////////
bool CISAPIData::GetXMLTCHAR( LPCTSTR szField, TCHAR* szValue, int szValueLen, 
							 bool bOptional )
{
	tstring sField(szField);
	tstring::iterator strIter;
	
	for ( strIter = sField.begin(); strIter != sField.end(); strIter++ )
	{
		*strIter = toupper(*strIter);
	}
	
	map<tstring, tstring>::iterator iter;
	
	if( (iter = m_XMLFormMap.find( sField )) != m_XMLFormMap.end() )
	{
		_tcsncpy( szValue, iter->second.c_str(), szValueLen );
		return true;
	}
	
	if (!bOptional)
	{
		CEMSString sError;
		sError.Format(EMS_STRING_E_FormFieldNotFound, szField);
		THROW_EMS_EXCEPTION( E_FormFieldNotFound, sError );
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetXMLTCHAR
// 
////////////////////////////////////////////////////////////////////////////////
bool CISAPIData::GetXMLTCHAR( LPCTSTR szField ,TCHAR** ppBuffer, long& nLength, 
							 long& nAllocated, bool bOptional )
{
	tstring sField(szField);
	tstring::iterator strIter;
	
	for ( strIter = sField.begin(); strIter != sField.end(); strIter++ )
	{
		*strIter = toupper(*strIter);
	}
	
	map<tstring, tstring>::iterator iter;
	
	if( (iter = m_XMLFormMap.find( sField )) != m_XMLFormMap.end() )
	{
		nLength = iter->second.size() + 1;

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

		_tcsncpy( *ppBuffer, iter->second.c_str(), nLength );
		return true;
	}
	
	if (!bOptional)
	{
		CEMSString sError;
		sError.Format(EMS_STRING_E_FormFieldNotFound, szField);
		THROW_EMS_EXCEPTION( E_FormFieldNotFound, sError );
	}
	
	return false;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetXMLLong
// 
////////////////////////////////////////////////////////////////////////////////
bool CISAPIData::GetXMLLong(LPCTSTR szField, int& nValue, bool bOptional )
{
	tstring sField(szField);
	tstring::iterator strIter;
	
	for ( strIter = sField.begin(); strIter != sField.end(); strIter++ )
	{
		*strIter = toupper(*strIter);
	}
	
	map<tstring, tstring>::iterator iter;
	
	if( (iter = m_XMLFormMap.find( sField )) != m_XMLFormMap.end() )
	{
		nValue = _ttoi( iter->second.c_str() );
		return true;
	}
	
	if (!bOptional)
	{
		CEMSString sError;
		sError.Format(EMS_STRING_E_FormFieldNotFound, szField);
		THROW_EMS_EXCEPTION( E_FormFieldNotFound, sError );
	}
	
	return false;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetXMLLong
// 
////////////////////////////////////////////////////////////////////////////////
bool CISAPIData::GetXMLLong(LPCTSTR szField, unsigned char& nValue, bool bOptional )
{
	tstring sField(szField);
	tstring::iterator strIter;
	
	for ( strIter = sField.begin(); strIter != sField.end(); strIter++ )
	{
		*strIter = toupper(*strIter);
	}
	
	map<tstring, tstring>::iterator iter;
	
	if( (iter = m_XMLFormMap.find( sField )) != m_XMLFormMap.end() )
	{
		nValue = (unsigned char) _ttoi( iter->second.c_str() );
		return true;
	}
	
	if (!bOptional)
	{
		CEMSString sError;
		sError.Format(EMS_STRING_E_FormFieldNotFound, szField);
		THROW_EMS_EXCEPTION( E_FormFieldNotFound, sError );
	}
	
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetXMLString
// 
////////////////////////////////////////////////////////////////////////////////
bool CISAPIData::GetXMLString( LPCTSTR szField, tstring& sValue, bool bOptional )
{
	tstring sField(szField);
	tstring::iterator strIter;
	
	for ( strIter = sField.begin(); strIter != sField.end(); strIter++ )
	{
		*strIter = toupper(*strIter);
	}
	
	map<tstring, tstring>::iterator iter;
	
	if( (iter = m_XMLFormMap.find( sField )) != m_XMLFormMap.end() )
	{
		sValue = iter->second;
		return true;
	}
	
	if (!bOptional)
	{
		CEMSString sError;
		sError.Format(EMS_STRING_E_FormFieldNotFound, szField);
		THROW_EMS_EXCEPTION( E_FormFieldNotFound, sError );
	}

	return false;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetRightHandPane
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIData::SetRightHandPane( LPCTSTR szURL )
{
	if( szURL == NULL )
	{
		CEMSString sURL = m_pECB->lpszPathInfo;
		if( strlen( m_pECB->lpszQueryString ) > 0 )
		{
			sURL.append( "?" );
			sURL.append( m_pECB->lpszQueryString );
		}
		m_SessionMap.SetRightHandPane( m_sSessionID, sURL.c_str() );
	}
	else
	{
		m_SessionMap.SetRightHandPane( m_sSessionID, szURL );
	}
}	
