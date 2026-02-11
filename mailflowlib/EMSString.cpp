/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/EMSString.cpp,v 1.2 2005/11/29 21:30:04 markm Exp $
||
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "EMSString.h"
#include "RegistryFns.h"

HMODULE CEMSString::m_hResourceDLL = NULL;

LPCTSTR lpszLoadError		=  _T("Unable to load resource DLL");
LPCTSTR lpszStringNotFound	=  _T("Unable to find string (%d) in resource DLL");
LPCTSTR lpszStringError		=  _T("Unable to load string");
LPCTSTR lpszFormatError		=  _T("Unable to format string");

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CEMSString::CEMSString()
{
	m_nCDLPos = 0;
}

CEMSString::CEMSString( LPCTSTR szString )
{
	m_nCDLPos = 0;
	this->assign( szString );
}

CEMSString::CEMSString( unsigned int nResourceID )
{
	m_nCDLPos = 0;	
	LoadString( nResourceID );
}


/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Loads the resource dll	              
\*--------------------------------------------------------------------------*/
int CEMSString::LoadResourceDLL()
{
	int nRet;

	// bail if the DLL is already loaded
	if (m_hResourceDLL)
	{
		return 0;
	}

	tstring sResourceDLLPath;

	nRet = GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_REG_RESOURCE_PATH, sResourceDLLPath);

	if (nRet != ERROR_SUCCESS)
	{
		return nRet;
	}

	if (!(m_hResourceDLL = LoadLibraryEx(sResourceDLLPath.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE)))
	{
		nRet = GetLastError();
		return nRet;
	}

	return 0;
}

/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Unload the resource dll	              
\*--------------------------------------------------------------------------*/
int CEMSString::UnloadResourceDLL()
{
	if (!m_hResourceDLL)
	{
		return 0;
	}
	
	if (FreeLibrary(m_hResourceDLL))
	{
		m_hResourceDLL = NULL;
		return 0;
	}

	int nRet;
	nRet = GetLastError();
	
	return nRet;
}

/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Load a string from the string table	              
\*--------------------------------------------------------------------------*/
void CEMSString::LoadString(unsigned int nStringID)
{
	try
	{
		// load the resource dll if we need to
		if (m_hResourceDLL == NULL)
		{
			if (LoadResourceDLL() != 0)
				THROW_EMS_EXCEPTION( E_UnableToLoadResourceDLL, lpszLoadError );
		}
		
		// load the string from the resource dll
		WCHAR     *pwchMem, *pwchCur;
		UINT      idRsrcBlk = nStringID / 16 + 1;
		int       strIndex  = nStringID % 16;
		HRSRC     hResource = NULL;
		
		// find the string table in the resource block
		if (!(hResource = FindResourceEx( m_hResourceDLL, RT_STRING, MAKEINTRESOURCE(idRsrcBlk), 
										  MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL) )))
		{
			// if we couldn't find the resource block, we were unable to find the string
			CEMSString sError;
			sError.Format( lpszStringNotFound, nStringID );
			THROW_EMS_EXCEPTION( E_StringNotFound, sError );
		}

		// get a pointer to the string table
		if (!(pwchMem = (WCHAR *) LoadResource( m_hResourceDLL, hResource )))
		{
			THROW_EMS_EXCEPTION( E_StringNotFound, lpszStringError);
		}	

		// pointer for walking through the string table
		pwchCur = pwchMem;

		for(int i = 0; i < 16; i++ )
		{	
			if( *pwchCur )
			{
				int cchString = *pwchCur;  // string size in characters.
				pwchCur++;
				
				// we found the string
				if( i == strIndex )
				{
#ifndef UNICODE
					// allocate storage
					this->resize(cchString);

					// convert from uncode to ANSI
					if (WideCharToMultiByte(CP_ACP, 0, pwchCur, cchString, (char*) this->data(), cchString, NULL, NULL ) < 1)
					{
						THROW_EMS_EXCEPTION( E_StringNotFound, lpszStringError );
					}
#else
					this->assign(pwchCur, cchString);
#endif	
					return;
				}
				
				// move on to the next string
				pwchCur += cchString;
			}
			else
				pwchCur++;
		}

		// if we get this far we were unable to find the string
		CEMSString sError;
		sError.Format( lpszStringNotFound, nStringID );
		THROW_EMS_EXCEPTION( E_StringNotFound, sError );
	}
	catch(...)
	{
		this->assign(lpszStringError);
		throw;
	}
}

/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Loads and formats a string from the resource file.
||				Assigns the formated string to *this	              
\*--------------------------------------------------------------------------*/
void CEMSString::Format(unsigned int nStringID, ...)
{
	va_list va;
	va_start( va, nStringID );
			
	FormatArgList(nStringID, va);

	va_end(va);
}

/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Formats szFormat and assigns the formated string to *this	              
\*--------------------------------------------------------------------------*/
void CEMSString::Format(LPCTSTR szFormat, ...)
{	
	va_list va;
	va_start( va, szFormat );

	FormatArgList(szFormat, va);

	va_end(va);
}

/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Loads and formats a string from the resource file
||				using a function argument list.  Assigns the formated 
||				string to *this
\*--------------------------------------------------------------------------*/
void CEMSString::FormatArgList(unsigned int nStringID, va_list& va)
{
	try
	{
		// load the string 
		LoadString(nStringID);

		// save the string that was loaded from the
		// resource file	
		tstring sFormat;
		sFormat = this->c_str();
		
		// preform the format
		FormatArgList(sFormat.c_str(), va);
	}
	catch (...) 
	{
		this->assign(lpszFormatError);
		throw;
	}
}

/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Formats a szFormat using a function argument list
||				Assigns the formated string to *this
\*--------------------------------------------------------------------------*/
void CEMSString::FormatArgList(LPCTSTR szFormat, va_list& va)
{
	try
	{
		// reserve space in the string
		int nSize = 256;
		this->resize(nSize);
			
		// attempt to format the string
		int nRet = _vsntprintf( (TCHAR*) this->data(), nSize, szFormat, va );

		// if we need more space, allocate it and try again
		while (nRet == -1)
		{
			// double the size each time through
			nSize *= 2;
			
			this->resize(nSize);
			nRet = _vsntprintf( (TCHAR*) this->data(), nSize, szFormat, va );
		}

		// if an error occured
		if (nRet < 0)
		{
			THROW_EMS_EXCEPTION( E_MemoryError, _T("Unable to format string") );
		}
		else
		{
			// shrink the string
			this->resize(nRet);
		}
	}
	catch(...)
	{
		this->assign(lpszFormatError);
		THROW_EMS_EXCEPTION( E_MemoryError, _T("Unable to format string") );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Trims any whitespace off the beginning and end of the
||				string.	              
\*--------------------------------------------------------------------------*/
void CEMSString::TrimWhiteSpace( void )
{
	CEMSString::iterator pos;

	// start at the front of the string
	pos = this->begin();

	while ( pos != this->end() && _istspace( *pos ) )
	{
		this->erase( pos );
		pos++;
	}
	
	// now start at the end of the string
	pos = this->end();
	
	// if the string is empty, bail out
	if ( pos == this->begin() )
		return;
	
	pos--;
	
	while ( pos != this->begin() && _istspace( *pos ) )
	{
		this->erase( pos );
		pos--;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds escape characters for use with javascript string
||				literals...		              
\*--------------------------------------------------------------------------*/
void CEMSString::EscapeJavascript( bool bAllowNewLine /*= true*/ )
{
	CEMSString::iterator iter = this->begin();

	while ( iter != this->end() )
	{
		switch( *iter )
		{
		case _T('\''):
		case _T('\"'):
		case _T('\\'):
			iter = this->insert( iter, _T('\\') );
			iter++;
			break;

		case _T('\r'):
			if (bAllowNewLine)
			{
				*iter = _T('\\');
				iter++;
				iter = this->insert( iter, _T('r') );
			}
			else
			{
				iter = this->erase( iter );
				continue;
			}
			break;

		case _T('\n'):
			if (bAllowNewLine)
			{
				*iter = _T('\\');
				iter++;
				iter = this->insert( iter, _T('n') );
			}
			else
			{
				iter = this->erase( iter );
				continue;
			}
		}

		iter++;
	}

	// convert HTML escape codes for UNICODE characters to Javascript escape codes
	// for UNICODE characters
	CEMSString sFormat;
	size_t start;
	size_t end;
	int nCharCode;

	start = this->find( _T("&#") );
	
	while ( start != CEMSString::npos )
	{
		end = this->find( _T(";"), start );

		if ( end != CEMSString::npos && end - start < 8 )
		{
			if ( nCharCode = _ttoi(this->substr( start+2, end-2 - start ).c_str()) )
			{
				sFormat.Format( _T("\\u%04X"), nCharCode );
				this->erase( start, end+1 - start );
				this->insert( start, sFormat.c_str() );
			}
		}

		start = this->find( _T("&#"), ++start );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds escape characters for use comma delimited files
\*--------------------------------------------------------------------------*/
void CEMSString::EscapeCSV( void  )
{
	CEMSString::iterator pos;

	for ( pos = this->begin(); pos != this->end(); pos++ )
	{
		switch( *pos )
		{
			case _T('\"'):
				pos = this->insert( pos, _T('\"') );
				pos++;
				break;
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Escapes a string for use with SQL.	              
\*--------------------------------------------------------------------------*/
void CEMSString::EscapeSQL( void )
{
	CEMSString::iterator pos;
	
	for ( pos = this->begin(); pos != this->end(); pos++ )
	{
		switch( *pos )
		{
		case _T('['):
		case _T('%'):
		case _T('_'):
			pos = this->insert( pos, _T('[') );
			pos++;
			pos++;
			
			if ( pos != this->end() )
			{
				pos = this->insert( pos, _T(']') );
				pos++;
			}
			else
			{
				this->append( _T("]") );
				return;
			}
			break;
			
		default:
			break;
		}
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Escape HTML	              
\*--------------------------------------------------------------------------*/
void CEMSString::EscapeHTML(void)
{
	size_t pos;
	CEMSString sString = _T("<");
	pos = this->find( sString );
	
	while ( pos != CEMSString::npos )
	{
		this->erase( pos, sString.length() );
		this->insert( pos, _T("&lt;") );
		
		pos = this->find( sString, pos );
	}
	
	sString = _T(">");
	pos = this->find( sString );
	
	while ( pos != CEMSString::npos )
	{
		this->erase( pos, sString.length() );
		this->insert( pos, _T("&gt;") );
		
		pos = this->find( sString, pos );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Convert HTML character codes	              
\*--------------------------------------------------------------------------*/
void CEMSString::ConvertHTMLCharCodes(void)
{
	size_t pos;
	CEMSString sString = _T("&quot;");
	
	pos = this->find( sString );
	while ( pos != CEMSString::npos )
	{
		this->erase( pos, sString.length() );
		this->insert( pos, _T("\"") );
		
		pos = this->find( sString, pos );
	}
	
	sString = _T("&lt;");
	pos = this->find( sString );
	
	while ( pos != CEMSString::npos )
	{
		this->erase( pos, sString.length() );
		this->insert( pos, _T("<") );
		
		pos = this->find( sString, pos );
	}
	
	sString = _T("&gt;");
	pos = this->find( sString );
	
	while ( pos != CEMSString::npos )
	{
		this->erase( pos, sString.length() );
		this->insert( pos, _T(">") );
		
		pos = this->find( sString, pos );
	}
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Validates that the string is a valid email address	              
\*--------------------------------------------------------------------------*/
bool CEMSString::ValidateEmailAddr( void )
{
	// an email address is always atleast 4 characters
	if ( this->length() < 5 )
		return false;

	size_t pos = this->find( _T('@') );
	
	size_t pos2 = this->find( _T('@'), pos+1 );
	if ( pos2 != CEMSString::npos )
	{
		// Can't have more than one '@' sign
		return false;
	}

	// must contain an at sign, and it cannot be at the start or the end
	if ( pos == CEMSString::npos || pos == 0 || pos == this->length() - 1 )
		return false;

	pos = this->rfind( _T('.') );
	
	// must contain a period, and it cannot be at the start or the end
	if ( pos == CEMSString::npos || pos == 0 || pos == this->length() - 1 )
		return false;

	return true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Validates that the string is a valid email address	              
\*--------------------------------------------------------------------------*/
bool CEMSString::ValidateHostName( void )
{
	// a hostname must be at least 4 characters
	if ( this->length() < 4 )
		return false;

	size_t pos = this->rfind( _T('.') );
	
	// must contain a period, and it cannot be at the start or the end
	if ( pos == CEMSString::npos || pos == 0 || pos == this->length() - 1 )
		return false;

	//string str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.";
	tstring str = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.");
	
	LPCTSTR sPos;
	int nPlace=0;
	for ( sPos = this->c_str(); *sPos; sPos++ )
	{
		if( *sPos == '-' )
		{
			if(nPlace == this->length() - 1 || nPlace == 0)
			{
				return false;
			}
		}
		try
		{
			pos = str.find(*sPos);
		}
		catch(...)
		{
			return false;
		}
		if( pos == CEMSString::npos )
		{
			return false;
		}
		nPlace++;
	}	

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//     These methods are for iterating through comma-delimited lists of IDs   //
////////////////////////////////////////////////////////////////////////////////
// 
// CDLInit - Initialize the iteration - call this first
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSString::CDLInit( void )
{
	m_nCDLPos = 0;
}
	

////////////////////////////////////////////////////////////////////////////////
// 
// CDLGetNextInt - Gets the next int in the list
//
// returns true if an int was returned or false if the list is exhausted
////////////////////////////////////////////////////////////////////////////////
bool CEMSString::CDLGetNextInt( int& nInt )
{
	tstring sInt;
	sInt.reserve(10);

	if( m_nCDLPos >= size() )
		return false;
	
	while( m_nCDLPos < size() )
	{
		if( at(m_nCDLPos) == _T(',') )
		{
			m_nCDLPos++;	// skip the comma
			break;
		}

		sInt += at(m_nCDLPos++);
	}
		
	nInt = _ttoi( sInt.c_str() );

	return (sInt.size() > 0);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the next string in a comma-delimited list of 
||				strings	              
\*--------------------------------------------------------------------------*/
bool CEMSString::CDLGetNextString( tstring& sString )
{	
	if( m_nCDLPos >= size() )
		return false;

	// clear the string
	sString.erase();

	// reserve a little bit of space 
	sString.reserve(32);
	
	while( m_nCDLPos < size() )
	{
		if( at(m_nCDLPos) == _T(',') )
		{
			m_nCDLPos++;	// skip the comma
			break;
		}
		
		// add this character to our string
		sString.append( &at(m_nCDLPos++), 1 );
	}
	
	return (sString.size() > 0);
}


////////////////////////////////////////////////////////////////////////////////
// 
// CDLGetNextChunk - Returns the next nChunkSize IDs in a string (w/o the comma 
//                   at the end)
// 
// returns true if a chunk was returned or false if the list is exhausted
////////////////////////////////////////////////////////////////////////////////
bool CEMSString::CDLGetNextChunk( int nChunkSize, tstring& sChunk )
{
	//size_type pos = 0;

	sChunk.erase();
	sChunk.reserve(nChunkSize*10);

	if( m_nCDLPos >= size() )
		return false;
	
	while( m_nCDLPos < size() )
	{
		if( at(m_nCDLPos) == _T(',') )
		{
			if( --nChunkSize == 0 )
			{
				m_nCDLPos++;	// skip the comma
				break;
			}
		}

		if( at(m_nCDLPos) != _T(' ') )	// add the character if its not a space
			sChunk += at(m_nCDLPos);

		m_nCDLPos++;
	}
		
	// Get rid of the trailing comma
	if( sChunk.size() > 0 && sChunk.at( sChunk.size() - 1 ) == _T(',') )
		sChunk.resize( sChunk.size() - 1 );

	return (sChunk.size() > 0);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Populates a TIMESTAMP_STRUCT from a comma-delimited string 
||				in the format of mm,dd,yyyy             
\*--------------------------------------------------------------------------*/
void CEMSString::CDLGetTimeStamp( TIMESTAMP_STRUCT& time )
{
	int nDatePart;

	CDLInit();

	if (!CDLGetNextInt( nDatePart ) || nDatePart < 0 || nDatePart > 12)
		THROW_EMS_EXCEPTION( E_InvalidParameters, _T("Invalid Date String") );

	time.month = nDatePart;

	if (!CDLGetNextInt( nDatePart ) || nDatePart < 0 || nDatePart > 31)
		THROW_EMS_EXCEPTION( E_InvalidParameters, _T("Invalid Date String") );
		
	time.day = nDatePart;

	if (!CDLGetNextInt( nDatePart ) || nDatePart < 0 )
		THROW_EMS_EXCEPTION( E_InvalidParameters, _T("Invalid Date String") );
		
	time.year = nDatePart;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FormatBytes
// 
// sets the contents to a textual display of # of bytes
//
////////////////////////////////////////////////////////////////////////////////
void CEMSString::FormatBytes( int BytesLow, int BytesHigh )
{
	FILETIME ft;
	__int64 nBytes;
	float f;
	
	ft.dwHighDateTime = BytesHigh;
	ft.dwLowDateTime = BytesLow;

	nBytes = *(__int64*)(&ft);

	if( nBytes < (__int64)1024 )
	{
		Format( _T("%d Bytes"), BytesLow );
	}
	else if( nBytes < (__int64)1048576 )
	{
		f = (float) (nBytes / 1024.0f);
		Format( _T("%5.1f KB"), f );
	}
	else if( nBytes < (__int64)1073741824 )
	{
		f = (float) (nBytes / 1048576.0f);
		Format( _T("%5.1f MB"), f );
	}
	else 
	{
		f = (float) (nBytes / 1073741824.0f);
		Format( _T("%5.1f GB"), f );
	}
}



static TCHAR szHex[17] = _T("werijlmnpqtosdfx");

void encipher(unsigned long *const v,unsigned long *const w, const unsigned long *const k);
/*
{
	register unsigned long y=v[0],z=v[1],sum=0,delta=0x9E3779B9,
		a=k[0],b=k[1],c=k[2],d=k[3],n=32;
	
	while(n-->0)
	{
		sum += delta;
		y += (z<<4)+a ^ z+sum ^ (z>>5)+b;
		z += (y<<4)+c ^ y+sum ^ (y>>5)+d;
	}
	
	w[0]=y; w[1]=z;
}
*/


void decipher(unsigned long *const v,unsigned long *const w, const unsigned long *const k);
/*
{
	register unsigned long y=v[0],z=v[1],sum=0xC6EF3720,
		delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],
		d=k[3],n=32;
	
	// sum = delta<<5, in general sum = delta * n 
	
	while(n-->0)
	{
		z -= (y<<4)+c ^ y+sum ^ (y>>5)+d;
		y -= (z<<4)+a ^ z+sum ^ (z>>5)+b;
		sum -= delta;
	}
	
	w[0]=y; w[1]=z;
}
*/

// An attempt to hide the key value by creating it dynamically
void GetKey( unsigned long* k )
{
	k[3] = 0x171692AB;
	k[0] = 0xED9723EF;
	k[2] = 0x054C3D90;
	k[1] = 0x47698482;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Encrypt
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSString::Encrypt()
{
	long nBufSize = 0;
	LPSTR pBuffer = NULL;
	long nTotalStringLen = 0;
	int i;
	unsigned long k[4];		
	unsigned long nTemp[2];
	LPSTR szObfuscated;
	LPSTR szPtr;

	// Concatenate all the strings in a single buffer, separated by zeros
	nTotalStringLen = ( size() + 1);

	// and pad for encryption.
	nBufSize = nTotalStringLen + 8;

	// Allocate a buffer to hold the strings and for in-place encryption
	pBuffer = (LPSTR) malloc( nBufSize );
	if ( pBuffer == NULL )
	{
		return; // Memory Allocation Error
	}

	// Allocate a buffer to hold the text version of the encrypted buffer
	szObfuscated = (LPSTR) malloc( nBufSize * 2  );
	if ( szObfuscated == NULL )
	{
		free( pBuffer );
		return; // Memory Allocation Error
	}
	
	// Clear the buffers
	memset( pBuffer, '\0', nBufSize );
	memset( szObfuscated, '\0', nBufSize * 2 );

	// Copy the string to the buffers
#ifdef UNICODE
	WideCharToMultiByte( CP_ACP, 0, c_str(), nTotalStringLen, pBuffer, nBufSize, NULL, NULL );
#else
	_tcscpy( pBuffer, c_str() );
#endif
	
	// Initialize encryption key
	GetKey( k );

	// Encrypt buffer using a 64-bit block cipher. 8 bytes 
	// are encrypted in a single call to encipher, and then
	// we advance 4 bytes and repeat.
	for ( i = 0; i < nTotalStringLen; i += 4 )
	{
		encipher( (unsigned long*) &pBuffer[i], nTemp, k );
		memcpy( &pBuffer[i], nTemp, 8 );
	}

	// Update total length of string
	nTotalStringLen = i + 4;

	szPtr = szObfuscated;

	// Convert each binary char into 2 hex digits
	for ( i = 0; i < nTotalStringLen; i++ )
	{
		if( pBuffer[i] == 0)
		{
			*szPtr++ = (char) -1;
			*szPtr++ = (char) 1;
		}
		else if( pBuffer[i] == (char) -1 )
		{
			*szPtr++ = (char) -1;
			*szPtr++ = (char) 2;
		}
		else
		{
			*szPtr++ = pBuffer[i];
		}
	}

	free (pBuffer);
	
#ifdef UNICODE
	LPWSTR wcsTemp;
	int Len = strlen( szObfuscated ) + 1;
	wcsTemp = (LPWSTR) malloc( Len * 2 );
	MultiByteToWideChar( CP_ACP, 0, szObfuscated, Len, wcsTemp, Len*2 );
	assign( wcsTemp );
	free( wcsTemp );

#else
	assign( szObfuscated );
#endif

	free( szObfuscated );
}

////////////////////////////////////////////////////////////////////////////////
// 
// Decrypt
//
//  If the string is empty after the decryption, then original was no generated
//  with Encrypt().
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSString::Decrypt()
{
	int i;
	long nHexLength;
	LPSTR pBuffer;
	long nBufferLength;
	unsigned long k[4];		// Create Key
	unsigned long nTemp[2];
	LPSTR pBuff;
	LPSTR szPtr;

	// Double-check the length of the string
	nHexLength = size();

	if ( nHexLength < 1 )
		return;	// empty string

	// Allocate a buffer to hold the binary data
	nBufferLength = nHexLength + 8;
	pBuffer = (LPSTR) malloc( nBufferLength );

	if ( pBuffer == NULL )
		return;	// Memory allocation error

	// Clear the buffer
	memset( pBuffer, 0, nBufferLength );

	// STB: This conversion was necessary as retrieving encrypted strings from
	// registry created some 2-byte chars, e.g. 0x9B
#ifdef UNICODE
	WideCharToMultiByte( CP_ACP, 0, c_str(), size(), pBuffer, nBufferLength, NULL, NULL);
#else
	strcpy( pBuffer, c_str() );
#endif

	szPtr = pBuffer;
	pBuff = pBuffer;

	// Convert Hex into binary, 2 hex characters = 1 binary char
	for ( i = 0 ; i < nHexLength; i++ )
	{
		if( pBuff[i] == (char) -1 )
		{
			i++;
			if( pBuff[i] == (char) 1 )
			{
				*szPtr++ = 0;
			}
			else if( pBuff[i] == (char) 2 )
			{
				*szPtr++ = (char) -1;
			}
			else
			{
				// Illegal!
				free(pBuffer);
				assign( _T("") );
				return;
			}

		}
		else
		{
			*szPtr++ = pBuff[i];
		}
	}

	// Initialize encryption key
	GetKey( k );

	nHexLength = (szPtr - pBuffer);

	// This couldn't have been an encrypted string!
	if( nHexLength < 8 )
	{
		free(pBuffer);
		assign( _T("") );
		return;
	}
	
	// Decrypt the buffer by starting at the end of the string
	// to perform the reverse operation as Encrypt.
	for ( i = (nHexLength - 8); i >= 0; i -= 4 )
	{
		decipher( (unsigned long*) &pBuffer[i], nTemp, k );
		memcpy( &pBuffer[i], nTemp, 8 );
	}

#ifdef UNICODE
	i = strlen(pBuffer);
	resize( i );
	MultiByteToWideChar( CP_ACP, 0, pBuffer, i, (LPTSTR) c_str(), i );
#else
	assign( pBuffer );
#endif

	free( pBuffer );
}


////////////////////////////////////////////////////////////////////////////////
// 
// EscapeBody
//
// For HTML Messages, this means converting links to inline content to links to
// download.ems. For Text messages, this means hyperlinking URIs.
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSString::EscapeBody( int nIsHTML, int nMsgID, bool bIsInbound, 
							 int nArchiveID, bool bForEdit )
{
	LPCTSTR pos;
	
	if( nIsHTML )
	{
		tstring sNewString;
		sNewString.reserve( (size() * 5) / 4 );

		bool bInTag = false;

		for ( pos = this->c_str(); *pos; pos++ )
		{
			if( bInTag )
			{
				switch( *pos )
				{
				case _T('>'):
					bInTag = false;
					break;

				case _T('s'): case _T('S'):
					if( _tcsnicmp( pos, _T("src="), 4 ) == 0 )
					{	
						sNewString.append( _T("src=") );
						pos += 4;

						// The quote is optional
						if( *pos == _T('\"') )
						{
							sNewString.append( 1, _T('\"') );
							pos++;
						}

						if( _tcsnicmp( pos, _T("cid:"), 4 ) == 0 )
						{	
							CEMSString sURL;

							sURL.Format( _T("download.ems?msgid=%d&inbound=%d&archiveid=%d&cid="), 
										 nMsgID, 
										 (bIsInbound) ? 1 : 0,
										 nArchiveID );

							sNewString.append( sURL );

							pos += 4;
						}
					}
					break;
				}
			}
			else
			{
				bInTag = ( *pos == _T('<') );
			}

			sNewString.append( 1, *pos );
		}

		assign( sNewString );
	}
	else if( bForEdit == false )
	{	
		EscapeHTMLAndNewLines();

		int nPos = find( _T("://") );
	
		while ( nPos != CEMSString::npos )
		{
			if( nPos >= 4 && ( compare( nPos - 4, 4, _T("http") ) == 0 ) )
			{
				// http
				nPos -= 4;
				HyperLink( nPos );
			}
			else if( nPos >= 5 && ( compare( nPos - 5, 5, _T("https") ) == 0 ) )
			{
				// https
				nPos -= 5;
				HyperLink( nPos );
			}
			else if( nPos >= 3 && ( compare( nPos - 3, 3, _T("ftp") ) == 0 ) )
			{
				// ftp
				nPos -= 3;
				HyperLink( nPos );
			}
			else if( nPos >= 6 && ( compare( nPos - 6, 6, _T("mailto") ) == 0 ) )
			{
				// mailto
				nPos -= 6;
				HyperLink( nPos  );
			}

			nPos = this->find( _T("://"), nPos + 3 );
		}

	}

}

////////////////////////////////////////////////////////////////////////////////
// 
// HyperLink
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSString::HyperLink( int& nPos )
{
	CEMSString::iterator pos;
	CEMSString sURI = _T("<a href=\"");

	pos = begin() + nPos;

	while( pos < end() && *pos != _T(' ') && *pos != _T('<') && *pos != _T('\r')  )
	{
		sURI.append( 1, *pos );
		pos++;
	}

	sURI.append( _T("\" target=_blank>") );

	insert( nPos, sURI );

	nPos += ( 2 * sURI.size() ) - 25;	//  <a href=" + " target=_blank> = 25 characters

	insert( nPos, _T("</a>") );

	nPos += 4;
}



////////////////////////////////////////////////////////////////////////////////
// 
// EscapeHTMLAndNewLines
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSString::EscapeHTMLAndNewLines( void )
{
	tstring sNewString;
	sNewString.reserve( (size() * 5) / 4 );
	CEMSString::iterator pos;
	
	for ( pos = begin(); pos < end(); pos++ )
	{
		switch( *pos )
		{
		case _T('>'):
			sNewString.append( _T("&gt;") );
			break;

		case _T('<'):
			sNewString.append( _T("&lt;") );
			break;

		case _T('\r'):
			sNewString.append( _T("<br/>") );
			break;

		//case _T('\n'):
		//	break;

		default:
			sNewString.append( 1, *pos );
			break;
		}
	}

	assign( sNewString );
}


////////////////////////////////////////////////////////////////////////////////
// 
//  FormatMsgForSend
//  
//  Look for 'src' attributes of HTML tags and see if they were modified by
//  EscapeHTML above, if so, undo that change. Note that after the text is saved,
//  links change from relative to absolute, so have to undo that too.
// 
//	MJM - In addition the signature <DIV> that is present in HTML messages is 
//	removed from the message.
//
////////////////////////////////////////////////////////////////////////////////
void CEMSString::FormatHTMLMsgForSend( int nOutboundMsgID )
{
	CEMSString sSearch;
	tstring sNewString;
	bool bInTag = false;
	
	sNewString.reserve( size() );
	sSearch.Format( _T("download.ems?msgid=%d&amp;inbound=0&amp;archiveid=0&amp;cid="), nOutboundMsgID );

	LPCTSTR pos; 
	LPCTSTR lookahead; 

	for ( pos = this->c_str(); *pos; pos++ )
	{
		if( bInTag )
		{
			switch( *pos )
			{
			case _T('>'):
				
				bInTag = false;
				break;
		
			case _T('s'): case _T('S'):

				if( _tcsnicmp( pos, _T("src="), 4 ) == 0 )
				{
					// found a source attribute!
					sNewString.append( _T("src=") );
					pos += 4;

					// The quote is optional
					if( *pos == _T('\"') )
					{
						sNewString.append( 1, *pos );
						pos++;
					}

					lookahead = pos;

					// see if we can find a matching string before we find a double-quote.
					while( *lookahead )
					{
						if( *lookahead == _T('\"') )
						{
							break;
						}
						else if ( *lookahead == _T('d') )
						{
							if( _tcsnicmp( lookahead, sSearch.c_str(), sSearch.size() ) == 0 )
							{
								sNewString.append( _T("cid:") );
								pos = lookahead + sSearch.size();
								break;
							}
						}
						lookahead++;
					}
				}
				break;
			
			case _T('d'): case _T('D'):
			
				// remove the signature DIV from the HTML message
				if( _tcsnicmp( pos, _T("DIV id=vissignature>"), 20 ) == 0)
				{
					pos += 20;
					sNewString.erase( sNewString.length() - 1 , 1 );
				
					while( *pos )
					{
						if( *pos == _T('<') )
						{
							if( _tcsnicmp( pos, _T("</DIV>"), 6 ) == 0 )
							{
								pos += 6;
								break;
							}
						}
						
						sNewString.append( 1, *pos );
						pos++;
					}
				}
				else if( _tcsnicmp( pos, _T("DIV id=\"vissignature\">"), 22 ) == 0)
				{
					pos += 22;
					sNewString.erase( sNewString.length() - 1 , 1 );
				
					while( *pos )
					{
						if( *pos == _T('<') )
						{
							if( _tcsnicmp( pos, _T("</DIV>"), 6 ) == 0 )
							{
								pos += 6;
								break;
							}
						}
						
						sNewString.append( 1, *pos );
						pos++;
					}
				}
				break;
			}
		}
		else
		{
			bInTag = ( *pos == _T('<') );
		}

		sNewString.append( 1, *pos );
	}

	assign( sNewString );
}

void CEMSString::FormatHTMLSigForSend( void )
{
	CEMSString sSearch;
	tstring sNewString;
	bool bInTag = false;
	
	sNewString.reserve( size() );
	sSearch.Format( _T("download.ems?cid=") );

	LPCTSTR pos; 
	LPCTSTR lookahead; 

	for ( pos = this->c_str(); *pos; pos++ )
	{
		if( bInTag )
		{
			switch( *pos )
			{
			case _T('>'):
				
				bInTag = false;
				break;
		
			case _T('s'): case _T('S'):

				if( _tcsnicmp( pos, _T("src="), 4 ) == 0 )
				{
					// found a source attribute!
					sNewString.append( _T("src=") );
					pos += 4;

					// The quote is optional
					if( *pos == _T('\"') )
					{
						sNewString.append( 1, *pos );
						pos++;
					}

					lookahead = pos;

					// see if we can find a matching string before we find a double-quote.
					while( *lookahead )
					{
						if( *lookahead == _T('\"') )
						{
							break;
						}
						else if ( *lookahead == _T('d') )
						{
							if( _tcsnicmp( lookahead, sSearch.c_str(), sSearch.size() ) == 0 )
							{
								sNewString.append( _T("cid:") );
								pos = lookahead + sSearch.size();
								break;
							}
						}
						lookahead++;
					}
				}
				break;			
			}
		}
		else
		{
			bInTag = ( *pos == _T('<') );
		}

		sNewString.append( 1, *pos );
	}

	assign( sNewString );
}

void CEMSString::FixInlineTags( bool bForSig )
{
	CEMSString sSearch;
	tstring sNewString;
	tstring sNewTag;
	bool bInTag = false;
	bool bTempFound = false;
	
	sNewString.reserve( size() );
	sNewTag.reserve( size() );
	sSearch.Format( _T("tempid=") );

	LPCTSTR pos; 
	LPCTSTR lookahead; 

	for ( pos = this->c_str(); *pos; pos++ )
	{
		if( bInTag )
		{
			switch( *pos )
			{
			case _T('>'):
				
				bInTag = false;
				break;
		
			case _T('i'): case _T('I'):

				if( _tcsnicmp( pos, _T("img "), 4 ) == 0 )
				{
					// found an image tag
					sNewTag.append( _T("img ") );
					pos += 4;

					lookahead = pos;

					// see if we can find a matching string before we close the tag
					while( *lookahead )
					{
						if( *lookahead == _T('>') )
						{
							bInTag = false;
							break;
						}
						else if( *lookahead ==_T('>') && bTempFound )
						{
							pos = lookahead+1;
							break;
						}
						else if( *lookahead == _T('\"') && bTempFound )
						{
							pos = lookahead+2;
							break;
						}
						else if ( *lookahead == _T('t') )
						{
							if( _tcsnicmp( lookahead, _T("tempid="), 7 ) == 0 )
							{
								bTempFound = true;
								if ( bForSig )
								{
									sNewTag.append( _T("src=/download.ems?cid="));
								}
								else
								{
									sNewTag.append( _T("src=cid:"));
								}
								
								lookahead += sSearch.size();								
							}
						}
						else if ( bTempFound )
						{
							sNewTag.append( 1, *lookahead );
						}
						lookahead++;
					}
					
					if ( bTempFound )
					{
						sNewString.append(sNewTag);
						bTempFound = false;
						sNewTag.clear();
					}
					else
					{
						sNewString.append( _T("img ") );
					}
				}
				break;			
			}			
		}
		else
		{
			bInTag = ( *pos == _T('<') );
		}

		sNewString.append( 1, *pos );
	}

	assign( sNewString );
}

////////////////////////////////////////////////////////////////////////////////
// 
// PrefixBody - used to quote a text message when sending an auto-reply
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSString::PrefixBody( LPCTSTR szPrefix )
{
	CEMSString::size_type pos = 0;

	insert( pos, szPrefix );

	while( (pos = find( _T("\n"), pos )) != CEMSString::npos )
	{
		if( pos < size() )
		{
			pos++;
			insert( pos, szPrefix );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Fix Line Endings
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSString::FixLineEnds( void )
{
	CEMSString::size_type pos = 0;

	while( (pos = find( _T("\n"), pos )) != CEMSString::npos )
	{
		if( pos < size() )
		{
			insert( pos, _T("\r") );
			pos++;pos++;
		}
	}
}

void CEMSString::RemoveScriptTags( void )
{
	size_t pos;
	size_t cpos;
	CEMSString sScript = _T("<script");
	CEMSString sCloseScript = _T("</script>");
	
	pos = this->find( sScript );
	while ( pos != CEMSString::npos )
	{
		cpos = this->find( sCloseScript, pos );
		if ( cpos != CEMSString::npos )
		{
			this->erase( pos, (cpos+9)-pos );
		}						
		else if( cpos == CEMSString::npos )
		{
			break;
		}
		pos = this->find( sScript, pos );
	}

	CEMSString sBase = _T("<base");
	CEMSString sCloseBase = _T(">");
	pos = this->find( sBase );
	while ( pos != CEMSString::npos )
	{
		cpos = this->find( sCloseBase, pos );
		if ( cpos != CEMSString::npos )
		{
			this->erase( pos, (cpos+1)-pos );
		}						
		else if( cpos == CEMSString::npos )
		{
			break;
		}
		pos = this->find( sBase, pos );
	}

	CEMSString sHead = _T("<head");
	CEMSString sCloseHead = _T("</head>");
	
	pos = this->find( sHead );
	while ( pos != CEMSString::npos )
	{
		cpos = this->find( sCloseHead, pos );
		if ( cpos != CEMSString::npos )
		{
			this->erase( pos, (cpos+7)-pos );
		}						
		else if( cpos == CEMSString::npos )
		{
			break;
		}
		pos = this->find( sHead, pos );
	}
}

void CEMSString::FixBodyTag(void)
{
	size_t pos;
	CEMSString sString = _T("contentEditable=true");
	
	pos = this->find( sString );
	while ( pos != CEMSString::npos )
	{
		this->erase( pos, sString.length() );
		this->insert( pos, _T("contentEditable=false") );
		
		pos = this->find( sString, pos );
	}	
}

void CEMSString::FixCommentTag(void)
{
	size_t pos;
	CEMSString sString = _T("<!--[");
	
	pos = this->find( sString );
	while ( pos != CEMSString::npos )
	{
		this->erase( pos, sString.length() );
		this->insert( pos, _T("<!-- [") );
		
		pos = this->find( sString, pos );
	}
	
	sString = _T("]-->");

	pos = this->find( sString );
	while ( pos != CEMSString::npos )
	{
		this->erase( pos, sString.length() );
		this->insert( pos, _T("] -->") );
		
		pos = this->find( sString, pos );
	}
}

bool CEMSString::IsStringValid(void)
{
	size_t pos;
	//string str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-'\"ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥ƒáíóúñÑµ¿¡ßÔ";
	tstring str = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-'\"ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥ƒáíóúñÑµ¿¡ßÔ");
	
	LPCTSTR sPos; 
	for ( sPos = this->c_str(); *sPos; sPos++ )
	{
		if( *sPos == '\"' || *sPos == ' ')
		{
			continue;
		}
		try
		{
			pos = str.find(*sPos);
		}
		catch(...)
		{
			return false;
		}
		if( pos == CEMSString::npos )
		{
			return false;
		}
	}	
	return true;
}

void CEMSString::SetVariables( LPCTSTR szEmail )
{
	size_t pos;
	CEMSString sString = _T("%%email%%");
	
	pos = this->find( sString );
	while ( pos != CEMSString::npos )
	{
		this->erase( pos, sString.length() );
		this->insert( pos, szEmail );
		
		pos = this->find( sString, pos );
	}
}

bool CEMSString::ToUTF8(void)
{
	wstring sData;
	dca::String sTemp(c_str());
	int Len = sTemp.length() + 1;
	
	int nWcsSize = MultiByteToWideChar( CP_ACP, 0, sTemp.c_str(), Len, NULL, 0  );
	sData.resize( nWcsSize, L' ' );
	MultiByteToWideChar( CP_ACP, 0, sTemp.c_str(), Len, (LPWSTR)(sData.c_str()), nWcsSize  );

	int nChrSize = WideCharToMultiByte( CP_UTF8, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
	char* szBuff = new char[nChrSize+1];
	WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
	szBuff[nChrSize] = 0;

    Format( _T("%s"),szBuff );
	
	return true;
}

bool CEMSString::ToAscii(void)
{
	wstring sData;
	dca::String sTemp(c_str());
	int Len = sTemp.length() + 1;
	
	int nWcsSize = MultiByteToWideChar( CP_UTF8, 0, sTemp.c_str(), Len, NULL, 0  );
	sData.resize( nWcsSize, L' ' );
	MultiByteToWideChar( CP_UTF8, 0, sTemp.c_str(), Len, (LPWSTR)(sData.c_str()), nWcsSize  );

	int nChrSize = WideCharToMultiByte( CP_ACP, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
	char* szBuff = new char[nChrSize+1];
	WideCharToMultiByte( CP_ACP, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
	szBuff[nChrSize] = 0;

	Format( _T("%s"),szBuff );
		
	return true;
}

void CEMSString::DoubleQuoteRealName( void )
{
	CEMSString::iterator pos;

	pos = this->end();
	
	// if the string is empty, bail out
	if ( pos == this->begin() )
		return;

	this->TrimWhiteSpace();
	
	pos = this->begin();

	while (pos != this->end())
	{
		if(*pos == '\"')
		{
			this->erase( pos );
		}
		pos++;
	}
	
	pos = this->begin();
	this->insert( pos, _T('\"') );
	this->append( _T("\"") );
}