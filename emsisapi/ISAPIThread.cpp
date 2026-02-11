// ISAPIThread.cpp: implementation of the CISAPIThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma warning (disable:4101)
#include "ISAPIThread.h"
#include "URLDecoder.h"
#include "RegistryFns.h"
#include ".\isapithread.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CISAPIThread::CISAPIThread(CSharedObjects* pShared) : CThread(pShared),  m_id(0)
{
}

CISAPIThread::~CISAPIThread()
{

}

int CISAPIThread::GetSourcePaths(void)
{
	tstring HTMLPath;
	tstring sTemp;
	
	_tcscpy( m_szURLSubDir, _T("/") );

	
	if( ERROR_SUCCESS == GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
									   EMS_URL_SUBDIR_VALUE, sTemp ) )
	{
		// Make sure we have a slash at the end
		if( sTemp.length() == 0 || sTemp.at( sTemp.length() - 1 ) != _T('/') )
		{
			sTemp += _T('/');
		}

		_tcscpy( m_szURLSubDir, sTemp.c_str() );
	}

	// everything looks ok
	return 0;
}


void CISAPIThread::InitializeRequest( void )
{
	// Initialize headers
	m_bHeaderSet = FALSE;
	m_szHeader.erase();

}

void CISAPIThread::GetUserAgent( tstring& sUserAgent )
{
	TCHAR szUserAgent[1025];
	DWORD dwSize = 1024;
	TCHAR* szIE = NULL;
	TCHAR* szIE11 = NULL;
	TCHAR* szOpera;	

	// Check User Agent
	if( m_pECB->GetServerVariable( m_pECB->ConnID, "HTTP_USER_AGENT", szUserAgent, &dwSize ) )
	{
		szUserAgent[dwSize] = 0;
	}
	else
	{
		szUserAgent[0] = 0;
	}

	//DEBUGPRINT(_T("# EMSIsapi.CISAPIThread.GetUserAgent - HTTP_USER_AGENT [ %s ]"),szUserAgent);

	szIE = _tcsstr( szUserAgent, _T("MSIE ") );

	if(!szIE)
	{
		szIE11 = _tcsstr( szUserAgent, _T("rv:11") );
	}
	
	if( szIE )
	{
		// Pull out the IE version
		sUserAgent = _T("Internet.Explorer.");
		szIE += 5;
		while( szIE - szUserAgent < MAX_PATH && *szIE != _T(';') && *szIE )
		{
			sUserAgent.append( 1, *szIE++ );			
		}		
	}
	else if( szIE11 )
	{
		// Pull out the IE version
		sUserAgent = _T("Internet.Explorer.");
		szIE11 += 3;
		while( szIE11 - szUserAgent < MAX_PATH && *szIE11 != _T(')') && *szIE11 )
		{
			sUserAgent.append( 1, *szIE11++ );			
		}	
	}
	else
	{
		TCHAR* szNetscape = _tcsstr( szUserAgent, _T("Gecko") );

		if( szNetscape )
		{
			TCHAR* sziPad = _tcsstr( szUserAgent, _T("iPad") );
			TCHAR* sziPhone = _tcsstr( szUserAgent, _T("iPhone") );
			TCHAR* sziPod = _tcsstr( szUserAgent, _T("iPod") );
			TCHAR* szAndroid = _tcsstr( szUserAgent, _T("Android") );
			TCHAR* szFirefox = _tcsstr( szUserAgent, _T("Firefox") );
			TCHAR* szSafari = _tcsstr( szUserAgent, _T("Safari") );
			TCHAR* szChrome = _tcsstr( szUserAgent, _T("Chrome") );
			TCHAR* szCriOS = _tcsstr( szUserAgent, _T("CriOS") );
			
			if( sziPad && szSafari && szCriOS)
			{
				sUserAgent.assign( _T("iPad.Chrome"));
			}
			else if( sziPad && szSafari)
			{
				sUserAgent.assign( _T("iPad.Safari"));
			}
			else if( sziPad )
			{
				sUserAgent.assign( _T("iPad"));
			}
			else if( sziPhone && szSafari && szCriOS)
			{
				sUserAgent.assign( _T("iPhone.Chrome"));
			}
			else if( sziPhone && szSafari)
			{
				sUserAgent.assign( _T("iPhone.Safari"));
			}
			else if( sziPhone )
			{
				sUserAgent.assign( _T("iPhone"));
			}
			else if( sziPod && szSafari && szCriOS)
			{
				sUserAgent.assign( _T("iPod.Chrome"));
			}
			else if( sziPod && szSafari)
			{
				sUserAgent.assign( _T("iPod.Safari"));
			}
			else if( sziPod )
			{
				sUserAgent.assign( _T("iPod"));
			}
			else if( szAndroid && szChrome )
			{
				sUserAgent.assign( _T("Android.Chrome"));
			}
			else if( szAndroid && szSafari )
			{
				sUserAgent.assign( _T("Android.Safari"));
			}
			else if ( szFirefox )
			{
				sUserAgent.assign( _T("Firefox"));
			}
			else if ( szChrome )
			{
				sUserAgent.assign( _T("Chrome"));
			}
			else if ( szSafari )
			{
				sUserAgent.assign( _T("Safari"));
			}
			else
			{
                sUserAgent.assign( _T("Gecko"));
			}
		}
		else
		{
			TCHAR* szMailFlow = _tcsstr( szUserAgent, _T("MailFlowClient") );
			TCHAR* sz3CX = _tcsstr( szUserAgent, _T("3CXPlugIn") );
			szOpera = _tcsstr( szUserAgent, _T("Opera") );

			if( szMailFlow || sz3CX )
			{
				sUserAgent.assign( szUserAgent );
			}
			else if ( szOpera )
			{
				sUserAgent.assign( "Opera" );
			}
			else
			{
				// Grab the whole string
				sUserAgent.assign( _T("Unsupported") );
			}
		}
	}

}

void CISAPIThread::GetUserOS( tstring& sUserOS )
{
	TCHAR szUserAgent[1025];
	DWORD dwSize = 1024;
	
	// Check User Agent
	if( m_pECB->GetServerVariable( m_pECB->ConnID, "HTTP_USER_AGENT", szUserAgent, &dwSize ) )
	{
		szUserAgent[dwSize] = 0;
	}
	else
	{
		szUserAgent[0] = 0;
	}

	TCHAR* sziPad = _tcsstr( szUserAgent, _T("iPad") );
	TCHAR* sziPhone = _tcsstr( szUserAgent, _T("iPhone") );
	TCHAR* sziPod = _tcsstr( szUserAgent, _T("iPod") );
	TCHAR* szAndroid = _tcsstr( szUserAgent, _T("Android") );
	TCHAR* szWin = _tcsstr( szUserAgent, _T("Windows") );
	TCHAR* szOsx = _tcsstr( szUserAgent, _T("OS X") );
	TCHAR* szMac = _tcsstr( szUserAgent, _T("Mac") );
	TCHAR* szLinux = _tcsstr( szUserAgent, _T("Linux") );

	if( sziPad || sziPhone || sziPod )
	{
		sUserOS.assign( _T("iOS"));
	}
	else if( szAndroid )
	{
		sUserOS.assign( _T("Android"));
	}
	else if ( szWin )
	{
		sUserOS.assign( _T("Windows"));
	}
	else if ( szOsx )
	{
		sUserOS.assign( _T("OS X"));
	}
	else if ( szMac )
	{
		sUserOS.assign( _T("Mac"));
	}
	else if ( szLinux )
	{
		sUserOS.assign( _T("Linux"));
	}
	else
	{
        sUserOS.assign( _T("Unknown"));
	}
}

void CISAPIThread::GetServerSecure(void)
{
	TCHAR szSecureConnection[16];
	DWORD dwSize = sizeof(szSecureConnection) - 1;

	m_bHTTPS = false;

	// Check for our cookie
	if( m_pECB->GetServerVariable( m_pECB->ConnID, "HTTPS", szSecureConnection, &dwSize ) )
	{
		szSecureConnection[dwSize] = 0;
		if( _tcsnicmp( szSecureConnection, _T("yes"), 3 ) == 0
			|| _tcsnicmp( szSecureConnection, _T("on"), 2 ) == 0 )
		{
			m_bHTTPS = true;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetClientIP
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIThread::GetClientIP(dca::String& IP )
{
	DWORD dwSize = sizeof(m_buffer) - 1;

	if( m_pECB->GetServerVariable( m_pECB->ConnID, "REMOTE_ADDR", m_buffer, &dwSize ) )
	{
		IP.assign((TCHAR*)m_buffer);
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetSessionCookie
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIThread::GetSessionCookie(void)
{
	DWORD dwSize = sizeof(m_buffer) - 1;

	m_bSessionCookieFound = FALSE;

	//DebugReporter::Instance().DisplayMessage("CISAPIThread::GetSessionCookie - checking for cookie", DebugReporter::ISAPI, GetCurrentThreadId());
	// Check for our cookie
	if( m_pECB->GetServerVariable( m_pECB->ConnID, "HTTP_COOKIE", m_buffer, &dwSize ) )
	{
		char* p = strstr( (char*)m_buffer, COOKIE_NAME );

		if( p )
		{			
			p += strlen(COOKIE_NAME);
			
			TCHAR* q = m_szSessionID;
			while( *p != 0 && *p != ';' )
			{
					*q++ = *p++;
			}
			
			// null terminate
			*q = 0;
			
			m_bSessionCookieFound = TRUE;

			/*dca::String b;
			b.Format("CISAPIThread::GetSessionCookie - found cookie %s", m_szSessionID);
			DebugReporter::Instance().DisplayMessage(b.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
		}
		else
		{
			//DebugReporter::Instance().DisplayMessage("CISAPIThread::GetSessionCookie - cookie buffer was null", DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
	else{
		//DebugReporter::Instance().DisplayMessage("CISAPIThread::GetSessionCookie - Server variable HTTP_COOKIE was invalid", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// AddHeaders - Appends additional headers to the response
// 
// szHeaders - additional headers (with "\r\n" at end of each line)
// 
// This function can be called any number of times before calling FormatHeaders
// but not after.
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIThread::AddHeaders( TCHAR* szHeaders )
{
	m_szHeader += szHeaders;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Formats the final version of the response headers
//
// szStatus - the HTTP status code and info, if NULL then "200 OK" is used.
// szContentType - the MIME type, if NULL then "text/html" is used.
// dwContentLength - length of the content in bytes
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIThread::FormatHeaders( TCHAR* szContentType, DWORD dwContentLength, bool bIsAttach )
{
	CEMSString sContentHeader;
	int nOffset = m_szHeader.size();
	
	m_bHeaderSet = TRUE;

	// Set defaults
	if( szContentType == NULL )
	{
		if(m_pShared->m_nCharSet==1)
		{
			szContentType = _T("text/html");
		}
		else
		{
			szContentType = _T("text/html;charset=UTF-8");
		}
	}
	
	// copy in the status code
	_tcscpy( m_szHeaderStatus, _T("200 OK") );

	if(m_pShared->m_nCharSet==1)
	{
		if(bIsAttach)
		{
			sContentHeader.Format( _T("Cache-Control: no-cache\r\nExpires: -1\r\nCharset: iso-8859-1\r\nContent-type: %s\r\nContent-length: %d\r\nX-UA-Compatible: IE=10\r\n\r\n"),
   							szContentType, dwContentLength );
		}
		else
		{
			sContentHeader.Format( _T("Cache-Control: no-cache\r\nExpires: -1\r\nCharset: iso-8859-1\r\nContent-type: %s\r\nContent-length: %d\r\nX-UA-Compatible: IE=5\r\n\r\n"),
   							szContentType, dwContentLength );
		}
	}
	else
	{
		if(bIsAttach)
		{
			sContentHeader.Format( _T("Cache-Control: no-cache\r\nExpires: -1\r\nCharset: utf-8\r\nContent-type: %s\r\nContent-length: %d\r\nX-UA-Compatible: IE=10\r\n\r\n"),
   							szContentType, dwContentLength );
		}
		else
		{
			sContentHeader.Format( _T("Cache-Control: no-cache\r\nExpires: -1\r\nCharset: utf-8\r\nContent-type: %s\r\nContent-length: %d\r\nX-UA-Compatible: IE=5\r\n\r\n"),
   							szContentType, dwContentLength );
		}
	}
	

	m_szHeader += sContentHeader;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetCookieHeader
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIThread::SetCookieHeader(void)
{
	TCHAR szCookieHeaders[256];

	LPCTSTR szFormat = _T("Set-Cookie: %s%s;expires=Thu 31-Dec-2099 11:59:59 GMT; path=/; HttpOnly\r\n");
	_sntprintf( (TCHAR*) szCookieHeaders, 255, 
		        szFormat, COOKIE_NAME, m_szSessionID );

	AddHeaders( szCookieHeaders );

	XServerParameters srvparam;
	tstring sUserAgent;
	GetUserAgent(sUserAgent);
	tstring::size_type pos;
	
	pos = sUserAgent.find(_T("3CXPlugIn"));			
	if( pos != tstring::npos )
	{
		tstring sRequiredPluginVersion;
		m_pShared->XMLCache().m_ServerParameters.Query(EMS_SRVPARAM_3CX_PLUGIN_VERSION, srvparam);
		sRequiredPluginVersion = srvparam.m_DataValue;
		szFormat = _T("Set-Cookie: %s%s;expires=Thu 31-Dec-2099 11:59:59 GMT; path=/;\r\n");
		_sntprintf( (TCHAR*) szCookieHeaders, 255, 
					szFormat, "RequiredPlugInVersion=", sRequiredPluginVersion.c_str() );
		AddHeaders( szCookieHeaders );
	}	
	
	pos = sUserAgent.find(_T("MailFlowClient"));
	if( pos != tstring::npos )
	{
		tstring sVersion;
		m_pShared->XMLCache().m_ServerParameters.Query(EMS_SRVPARAM_MAILFLOW_CLIENT_VERSION, srvparam);
		sVersion = srvparam.m_DataValue;
		szFormat = _T("Set-Cookie: %s%s;expires=Thu 31-Dec-2099 11:59:59 GMT; path=/;\r\n");
		_sntprintf( (TCHAR*) szCookieHeaders, 255, 
					szFormat, "RequiredClientVersion=", sVersion.c_str() );
		AddHeaders( szCookieHeaders );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Send HTML in a string
// 
////////////////////////////////////////////////////////////////////////////////
int CISAPIThread::SendString( tstring& html, bool bIsAttach )
{
	DWORD dwBytes = html.size();
	TCHAR sIP[256];
	DWORD dwSize = 255;

	ZeroMemory(sIP,256);

	//DebugReporter::Instance().DisplayMessage("CISAPIThread::SendString - send headers", DebugReporter::ISAPI, GetCurrentThreadId());

	SendHeaders( NULL, dwBytes, bIsAttach );

	if (m_pECB)
	{
		m_pECB->GetServerVariable(m_pECB->ConnID,"REMOTE_ADDR",sIP,&dwSize);

		try
		{
			//DebugReporter::Instance().DisplayMessage("CISAPIThread::SendString - sync write", DebugReporter::ISAPI, GetCurrentThreadId());

			if(!SyncWrite(html.c_str(),dwBytes))
				throw dca::Exception(GetLastError(), L"Could not write to client");
		}
		catch(dca::Exception e)
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Send XML in a string
// 
////////////////////////////////////////////////////////////////////////////////
int CISAPIThread::SendXML( tstring& xml )
{
	if(m_pShared->m_nCharSet==1)
	{
		xml.insert( 0, _T("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>") );
	}
	else
	{
		xml.insert( 0, _T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>") );
	}	

	DWORD dwBytes = xml.size();
	
	/* Hack for chars under 32  - The real fix is to put all strings in CDATA */
	{
		TCHAR* p = (TCHAR*)xml.c_str();
		UINT i = 0;
		for( i = 0; i < dwBytes; i++ )
		{
			if( (*p >= 0 && *p < 9) || (*p == 11) || (*p == 12) || (*p > 13 && *p < 32) )
			{
				*p = _T('?');
			}
			p++;
		}
	}

	if(m_pShared->m_nCharSet==1)
	{
		SendHeaders( _T("text/xml"), dwBytes );
	}
	else
	{
		SendHeaders( _T("text/xml;charset=UTF-8"), dwBytes );
	}	

	try
	{
		if(!SyncWrite(xml.c_str(),dwBytes))
			throw dca::Exception(GetLastError(), L"Could not write to client");
	}
	catch(...)
	{
		return 1;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendHeaders
// 
// szContentType - the MIME type, if NULL then "text/html" is used.
// dwContentLength - length of the content in bytes
//
////////////////////////////////////////////////////////////////////////////////
void CISAPIThread::SendHeaders( TCHAR* szContentType, DWORD dwBytes, bool bIsAttach )

{
 	HSE_SEND_HEADER_EX_INFO HeaderExInfo;


	if( m_bHeaderSet == FALSE )
	{	
		// Format the headers if necessary
		FormatHeaders( szContentType, dwBytes, bIsAttach );
	}

#ifdef _UNICODE
	int nChars = _tcslen(m_szHeaderStatus)+1;
	char* szHeaderStatus = new char[nChars];
	WideCharToMultiByte( CP_ACP, 0, m_szHeaderStatus, nChars, szHeaderStatus, nChars, 0, 0 );
	HeaderExInfo.pszStatus = szHeaderStatus;
	nChars = m_szHeader.size()+1;
	char* szHeader = new char[nChars];
	WideCharToMultiByte( CP_ACP, 0, m_szHeader.c_str(), nChars, szHeader, nChars, 0, 0 );
	HeaderExInfo.pszHeader = szHeader;
#else
	HeaderExInfo.pszStatus = m_szHeaderStatus;
    HeaderExInfo.pszHeader = m_szHeader.c_str();
#endif
    HeaderExInfo.cchStatus = strlen( HeaderExInfo.pszStatus );
    HeaderExInfo.cchHeader = strlen( HeaderExInfo.pszHeader );
    HeaderExInfo.fKeepConn = FALSE;

    //
    // send headers using IIS-provided callback
    // (note - if we needed to keep connection open,
    //  we would set fKeepConn to TRUE *and* we would
    //  need to provide correct Content-Length: header)

    if (m_pECB)
	{
		m_pECB->ServerSupportFunction( m_pECB->ConnID,
									   HSE_REQ_SEND_RESPONSE_HEADER_EX,
								       &HeaderExInfo,
								       NULL, NULL );
	}

#ifdef _UNICODE
	if( szHeader ) delete[] szHeader;
	if( szHeaderStatus ) delete[] szHeaderStatus;
#endif

}

////////////////////////////////////////////////////////////////////////////////
// 
// This function ends the request, and returns control to the web server
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIThread::FinishRequest(unsigned int id)
{
    DebugReporter::Instance().DisplayMessage("CISAPIThread::FinishRequest - Entering", DebugReporter::ISAPI, GetCurrentThreadId());

	try
	{
		if (m_pECB)
		{
			dca::String o;
			o.Format("CISAPIThread::FinishRequest - send HSE_REQ_DONE_WITH_SESSION for conn id [ %d ] job id [ %d ] in thread [ %d ]", m_pECB->ConnID,m_id, m_nThreadID);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			m_pECB->ServerSupportFunction( m_pECB->ConnID, HSE_REQ_DONE_WITH_SESSION, NULL, NULL, 0 );
		}
	}
	catch(...)
	{
		DebugReporter::Instance().DisplayMessage("CISAPIThread::FinishRequest - Caught exception", DebugReporter::ISAPI, GetCurrentThreadId());
	}
	
	DebugReporter::Instance().DisplayMessage("CISAPIThread::FinishRequest - Leaving", DebugReporter::ISAPI, GetCurrentThreadId());	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Redirect client
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIThread::Redirect( const TCHAR* szPage )
{
	int nPort = 0;
	CEMSString sRedirectHeader;

	_tcscpy( m_szHeaderStatus, _T("302 Object Moved.") );

	// We don't have info in the registry about how the URL is formed.
	// We attempt to construct an absolute URL from information about
	// the original request.

	sRedirectHeader.Format( _T("Location: %s\r\n\r\n"), szPage );
		
	m_szHeader += sRedirectHeader;
	m_bHeaderSet = TRUE;

	SendHeaders( NULL, 0);
}

////////////////////////////////////////////////////////////////////////////////
// 
// Return 404 - not found
// 
////////////////////////////////////////////////////////////////////////////////
void CISAPIThread::ReturnFileNotFound( void )
{
	m_szHeader.assign( _T("\r\n\r\n<html><head><title>Object Not Found</title></head><body><h1>HTTP/1.1 404 Object Not Found</h1></body></html>") );
	_tcscpy( m_szHeaderStatus, _T("404 Object not found.") );

	m_bHeaderSet = TRUE;

	SendHeaders( NULL, 0);
}

////////////////////////////////////////////////////////////////////////////////
// 
// Download a file
// 
////////////////////////////////////////////////////////////////////////////////
int CISAPIThread::SendFile( const TCHAR* szFileOnDisk, const TCHAR* szFileName, const TCHAR* szMediaType )
{
	try
	{
		dca::String o;
		if(lstrcmpi(m_pShared->sWebType.c_str(), "B") && m_pShared->m_nIISVer > 6)	
		//if(lstrcmpi(m_pShared->sWebType.c_str(), "B"))
		{
			CFilePacket*	pPacket = new CFilePacket();
			DWORD dwFileSize;

			TCHAR HEADER_TEMPLATE[] = "HTTP/1.0 200 OK\r\nContent-Type: %s; name=%s\r\nContent-length: %d\r\nContent-disposition: attachment; filename=\"%s\"\r\n\r\n";
			
			if(pPacket)
			{
				DWORD dwSize;
				HANDLE hFile;
				
				hFile = CreateFile(
					szFileOnDisk,
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
					NULL
					);

				if (hFile == INVALID_HANDLE_VALUE)
				{
					o.Format("CISAPIThread::SendFile - ConnID:[%d] error occurred creating handle for FileName:[%s]", m_pECB->ConnID,szFileOnDisk);
					DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					ReturnFileNotFound();
					return -1;
				}

				HSE_TF_INFO htf;
				ZeroMemory(&htf, sizeof (HSE_TF_INFO));

				htf.BytesToWrite	= 0;
				htf.Offset			= 0;
				htf.pszStatusCode	= 0;
				htf.pTail			= 0;
				htf.TailLength		= 0;

				htf.pfnHseIO	= (PFN_HSE_IO_COMPLETION)FileComplete;
				htf.dwFlags		= HSE_IO_ASYNC | HSE_IO_DISCONNECT_AFTER_SEND;
				htf.pContext	= (PVOID)hFile;
				htf.pHead		= pPacket->m_headers;
				htf.hFile		= hFile;
				htf.pszStatusCode = "200 OK";

				if(INVALID_HANDLE_VALUE != htf.hFile)
				{
					pPacket->m_hFile	= htf.hFile;

					dwFileSize = GetFileSize(htf.hFile, NULL );
					
					o.Format("CISAPIThread::SendFile - Using HSE_REQ_TRANSMIT_FILE with ConnID:[%d] WebType:[%s] MediaType:[%s] FileName:[%s] FileSize:[%d]", m_pECB->ConnID,m_pShared->sWebType.c_str(),szMediaType,szFileOnDisk,dwFileSize);
					DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

					_sntprintf(pPacket->m_headers,
							CFilePacket::MAX_HEADERS - 1,
							HEADER_TEMPLATE,
							szMediaType,szFileName,dwFileSize,szFileName);

					htf.HeadLength	= lstrlen(pPacket->m_headers);

					if(lstrlen(pPacket->m_headers) - 1 > (long)htf.HeadLength)
					{
					}

					if(m_pECB->ServerSupportFunction(m_pECB->ConnID,HSE_REQ_TRANSMIT_FILE,&htf,0,0))
					{
						pPacket = 0;						
					}
				}

				if(pPacket)
					delete pPacket;

			}
		}
		else
		{
			HANDLE hFile;
			DWORD dwBytes = 8192;
			DWORD dwFileSize;

			hFile = CreateFile( szFileOnDisk, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

			if( hFile == NULL || hFile == INVALID_HANDLE_VALUE )
			{
				ReturnFileNotFound();
				return -1;
			}

			dwFileSize = GetFileSize( hFile, NULL );
			
			o.Format("CISAPIThread::SendFile - Using WriteClient with WebType:[%s] MediaType:[%s] FileName:[%s] FileSize:[%d]", m_pShared->sWebType.c_str(),szMediaType,szFileName,dwFileSize);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

			if( dwFileSize == 0xFFFFFFFF )
			{
				CloseHandle(hFile);
				ReturnFileNotFound();
				return 0;
			}

			_tcscpy( m_szHeaderStatus, _T("200 OK") );
			m_szHeader.Format( _T("Content-type: %s; name=%s\r\n")
							_T("Content-length: %d\r\n") 
							_T("Content-disposition: attachment; filename=%s\r\n\r\n"),
							szMediaType, szFileName, dwFileSize, szFileName );


			m_bHeaderSet = TRUE;

			SendHeaders( NULL, 0 );

			ZeroMemory(m_buffer,8192);

			while( ReadFile( hFile, m_buffer, dwBytes, &dwBytes, NULL ) && dwBytes > 0 )
			{
				if( FALSE == m_pECB->WriteClient( m_pECB->ConnID, m_buffer, &dwBytes, HSE_IO_SYNC) )
				{
					break;
				}

				ZeroMemory(m_buffer,8192);
				dwBytes = 8192;
			}

			CloseHandle( hFile );
		}
	}
	catch(...)
	{
		return 1;
	}

	return 0;
}

void CALLBACK CISAPIThread::FileComplete(EXTENSION_CONTROL_BLOCK* pECB,PVOID pContext, DWORD count, DWORD error)
{
	//-------------------------------------------------------------------------
	// remember these callbacks operate outside the scope of other calls, treat
	// them like seperate entry points into the dll and make sure you don't let
	// exceptions leak out.  
	//-------------------------------------------------------------------------
	
	dca::String o;
					
	try
	{
		o.Format("CISAPIThread::FileComplete - Entering for file with ConnID:[%d]", pECB->ConnID);
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

		try
		{
			HANDLE hFile = (HANDLE)pContext;
			CloseHandle(hFile);
			DWORD dwStatus = HSE_STATUS_SUCCESS;
			if (pECB)
			{
				o.Format("CISAPIThread::FileComplete - send HSE_REQ_DONE_WITH_SESSION for conn id [%d]", pECB->ConnID);
				DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				pECB->ServerSupportFunction( pECB->ConnID, HSE_REQ_DONE_WITH_SESSION, &dwStatus, NULL, NULL );
			}
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("CISAPIThread::FileComplete - Caught exception in HSE_REQ_DONE_WITH_SESSION", DebugReporter::ISAPI, GetCurrentThreadId());
		}
		
		if(error)
		{
			o.Format("CISAPIThread::FileComplete - Returned Error:[%d]", error);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());				
		}
	}
	catch(...)
	{
		DebugReporter::Instance().DisplayMessage("CISAPIThread::FileComplete - Caught exception in pPacket", DebugReporter::ISAPI, GetCurrentThreadId());
	}
	DebugReporter::Instance().DisplayMessage("CISAPIThread::FileComplete - Leaving", DebugReporter::ISAPI, GetCurrentThreadId());		
}

// Hopefully a fix for spellcheck issue on IIS5
// Added by Mark Mohr on 8/20/2004
bool CISAPIThread::SyncWrite(const char* lpResponse, DWORD& dwResponse)
{
	DWORD dwBufLen = 0;
	char* szResponse = (char*)lpResponse;
	DWORD dwCurResponse = dwResponse;

	dwResponse = 0;

	dca::String ip;
	GetClientIP(ip);

	{
		/*dca::String o;
		o.Format("CISAPIThread::SyncWrite - begin write to client at ip %s", ip.c_str());
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
	}

	while(dwCurResponse > 0)
	{
		if(dwCurResponse >= 2920)
		{
			dwBufLen = 2920;
		}
		else
		{
			dwBufLen = dwCurResponse;
		}

		if(!m_pECB->WriteClient(m_pECB->ConnID, (void*)szResponse, &dwBufLen, HSE_IO_SYNC))
		{
			// Error occurred.
			return false;
		}
		else
		{
			dwCurResponse -= dwBufLen;
			dwResponse += dwBufLen;
			szResponse += dwBufLen;
		}
	}

	{
		/*dca::String o;
		o.Format("CISAPIThread::SendString - end write to client at ip %s", ip.c_str());
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
	}

	return true;
}
