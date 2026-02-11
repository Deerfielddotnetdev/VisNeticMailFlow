// RoutingEngine.cpp: implementation of the CRoutingEngine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RoutingEngine.h"
#include "KAVScan.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRoutingEngine::CRoutingEngine()
{
	m_pRoutingEngine = NULL;
}

CRoutingEngine::~CRoutingEngine()
{
	if( m_pRoutingEngine )
	{
		m_pRoutingEngine->Release();
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// If no interface pointer exists, create one. If Interface pointer created
// successfully return TRUE, otherwise FALSE
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::InterfacePointerGood(void)
{
	HRESULT hres;

	if( m_pRoutingEngine == NULL )
	{
		try
		{
			hres = CoCreateInstance( CLSID_RoutingEngineComm, NULL, CLSCTX_LOCAL_SERVER, 
								 IID_IRoutingEngineComm, (void**) &m_pRoutingEngine );

			if(FAILED(hres))
			{
				throw dca::Exception(hres, L"Could not create CLSID_RoutingEngineComm");
			}
		}
		catch(dca::Exception e)
		{
			dca::String error(e.GetMessage());
			dca::String f;
			f.Format("CRoutingEngine::InterfacePointerGood - %s\n", error.c_str());
			OutputDebugString(f.c_str());

			return FALSE;
		}
		catch(...)
		{
			OutputDebugString("CRoutingEngine::InterfacePointerGood - an unkown or undefined execption occurred\n");
			return FALSE;
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Handle a COM error by releasing the interface pointer
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::HandleCOMError( HRESULT hres )
{
	if( SUCCEEDED(hres))
	{
		return TRUE;
	}
	else
	{
		m_pRoutingEngine->Release();
		m_pRoutingEngine = NULL;

		return FALSE;
	}
}


////////////////////////////////////////////////////////////////////////////////
//
// Logs an entry if the given log entry type is enabled. The text for the log
// entry is pulled from the database. Optional arguments are merged with the
// string using vwprintf.
//
// Returns TRUE if log was successful, or FALSE if there was an error 
// communicating with the RoutingEngine
//
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::LogIt( int ErrorCode, unsigned int nStringID, ... )
{
	CEMSString szLogString;
	va_list va;
	
	va_start( va, nStringID );
	szLogString.FormatArgList(nStringID, va);
	va_end( va );	
	
	return WriteLogEntry( ErrorCode, szLogString );
}


////////////////////////////////////////////////////////////////////////////////
// 
// Same as above, but uses hard-coded format string.
// 
// Returns TRUE if log was successful, or FALSE if there was an error 
// communicating with the RoutingEngine
//
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::LogIt( int ErrorCode, TCHAR* wcsFormat, ... )
{
	CEMSString szLogString;
	va_list va;
	
	va_start( va, wcsFormat );
	szLogString.FormatArgList(wcsFormat, va);
	va_end( va );	
	
	return WriteLogEntry( ErrorCode, szLogString );
}


////////////////////////////////////////////////////////////////////////////////
// 
// LogEntry - Sends the log entry to the routing engine
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::WriteLogEntry( int ErrorCode, tstring& text )
{
	BSTR bstrText;
	BOOL bRetVal = FALSE;

	if( InterfacePointerGood() )
	{

	#ifdef _UNICODE 

		bstrText = SysAllocString( text.c_str() );
		
	#else

		wchar_t wcsFormattedString[512];
		MultiByteToWideChar( CP_ACP, 0, text.c_str(), text.size()+1,
							 wcsFormattedString, 512 );

		bstrText = SysAllocString( wcsFormattedString );

	#endif

		// Put in the ISAPI Filter component ID
		ErrorCode = ( ErrorCode & 0xf0ffffff) | (((EMSERR_ISAPI_EXTENSION)&0xf)<<24);

		bRetVal = HandleCOMError( m_pRoutingEngine->Log( ErrorCode, bstrText ) );

		// Try twice 
		if( bRetVal == FALSE )
		{
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->Log( ErrorCode, bstrText ) );
			}
		}

		SysFreeString( bstrText );
	}

	return bRetVal;
}

////////////////////////////////////////////////////////////////////////////////
// 
// ReloadConfig - force the routing engine to reload its tables
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::ReloadConfig( long nConfigItem )
{
	BOOL bRetVal = FALSE;

	if( InterfacePointerGood() )
	{

		bRetVal = HandleCOMError( m_pRoutingEngine->ReloadConfig( nConfigItem ) );

		// Try twice 
		if( bRetVal == FALSE )
		{
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->ReloadConfig( nConfigItem ) );
			}
		}
	}

	return bRetVal;
}


////////////////////////////////////////////////////////////////////////////////
// 
// TestPOP3Auth
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::TestPOP3Auth( BSTR HostName, long nPort, VARIANT_BOOL IsAPOP, BSTR UserName,
								   BSTR Password, long TimeoutSecs, long isSSL, long* pError )
{
	BOOL bRetVal = FALSE;

	OutputDebugString("CRoutingEngine::TestPOP3Auth - is interface good\n");
	if( InterfacePointerGood() )
	{

		OutputDebugString("CRoutingEngine::TestPOP3Auth - yes interface good\n");
		bRetVal = HandleCOMError( m_pRoutingEngine->TestPOP3Auth( HostName, nPort, IsAPOP, 
			                                                      UserName, Password, TimeoutSecs, isSSL, pError ) );

		// Try twice 
		if( bRetVal == FALSE )
		{
			OutputDebugString("CRoutingEngine::TestPOP3Auth - failed lets try again\n");
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->TestPOP3Auth( HostName, nPort, IsAPOP, 
			                                                              UserName, Password, TimeoutSecs, isSSL, pError ) );
			}
			else{
				OutputDebugString("CRoutingEngine::TestPOP3Auth - yes interface bad when trying second time\n");
			}
		}
	}
	else
	{
		OutputDebugString("CRoutingEngine::TestPOP3Auth - yes interface bad\n");
	}

	return bRetVal;
}


////////////////////////////////////////////////////////////////////////////////
// 
// TestSMTPAuth
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::TestSMTPAuth( BSTR HostName, long nPort, BSTR UserName,
		                           BSTR Password, long TimeoutSecs, long isSSL, long* pError )
{
	BOOL bRetVal = FALSE;

	if( InterfacePointerGood() )
	{

		bRetVal = HandleCOMError( m_pRoutingEngine->TestSMTPAuth( HostName, nPort, UserName,
			                                                      Password, TimeoutSecs, isSSL, pError ) );

		// Try twice 
		if( bRetVal == FALSE )
		{
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->TestSMTPAuth( HostName, nPort, UserName,
			                                                              Password, TimeoutSecs, isSSL, pError ) );
			}
		}
	}

	return bRetVal;
}


////////////////////////////////////////////////////////////////////////////////
// 
// ProcessOutboundQueue
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::ProcessOutboundQueue(void)
{
	BOOL bRetVal = FALSE;

	if( InterfacePointerGood() )
	{

		bRetVal = HandleCOMError( m_pRoutingEngine->ProcessOutboundQueue() );

		// Try twice 
		if( bRetVal == FALSE )
		{
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->ProcessOutboundQueue() );
			}
		}
	}

	return bRetVal;
}


////////////////////////////////////////////////////////////////////////////////
// 
// ProcessInboundQueue
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::ProcessInboundQueue(void)
{
	BOOL bRetVal = FALSE;

	if( InterfacePointerGood() )
	{
		bRetVal = HandleCOMError( m_pRoutingEngine->ProcessInboundQueue() );

		// Try twice 
		if( bRetVal == FALSE )
		{
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->ProcessInboundQueue() );
			}
		}
	}

	return bRetVal;
}


////////////////////////////////////////////////////////////////////////////////
// 
// SendAlert
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::SendAlert(long AlertEventID, long TicketBoxID, BSTR Text)
{
	BOOL bRetVal = FALSE;

	if( InterfacePointerGood() )
	{
		bRetVal = HandleCOMError( m_pRoutingEngine->SendAlert( AlertEventID, TicketBoxID, Text ) );

		// Try twice 
		if( bRetVal == FALSE )
		{
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->SendAlert( AlertEventID, TicketBoxID, Text ) );
			}
		}
	}

	return bRetVal;
}


////////////////////////////////////////////////////////////////////////////////
// 
// DoDBMaintenance
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::DoDBMaintenance( long* pError )
{
	BOOL bRetVal = FALSE;

	//DEBUGPRINT(_T("EMSIsapi::CRoutingEngine::DoDBMaintenance - entered"));

	//DEBUGPRINT(_T("EMSIsapi::CRoutingEngine::DoDBMaintenance - test interface pointer"));

	if( InterfacePointerGood() )
	{
		//DEBUGPRINT(_T("EMSIsapi::CRoutingEngine::DoDBMaintenance - good interface pointer"));

		bRetVal = HandleCOMError( m_pRoutingEngine->DoDBMaintenance( pError ) );

		//DEBUGPRINT(_T("EMSIsapi::CRoutingEngine::DoDBMaintenance - did maintenance run"));

		// Try twice 
		if( bRetVal == FALSE )
		{
			//DEBUGPRINT(_T("EMSIsapi::CRoutingEngine::DoDBMaintenance - maintenance did not run"));

			if ( InterfacePointerGood() )
			{
				//DEBUGPRINT(_T("EMSIsapi::CRoutingEngine::DoDBMaintenance - try again"));

				bRetVal = HandleCOMError( m_pRoutingEngine->DoDBMaintenance( pError ) );
			}
		}
	}
	else
	{
		//DEBUGPRINT(_T("EMSIsapi::CRoutingEngine::DoDBMaintenance - bad interface pointer"));
	}

	return bRetVal;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreArchive
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::RestoreArchive( long ArchiveID, long* pError )
{
	BOOL bRetVal = FALSE;

	if( InterfacePointerGood() )
	{
		bRetVal = HandleCOMError( m_pRoutingEngine->RestoreArchive( ArchiveID, pError ) );

		// Try twice 
		if( bRetVal == FALSE )
		{
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->RestoreArchive( ArchiveID, pError ) );
			}
		}
	}

	return bRetVal;
}
	
////////////////////////////////////////////////////////////////////////////////
// 
// RestoreBackup
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::RestoreBackup( BSTR BackupFile, long* pError )
{
	BOOL bRetVal = FALSE;

	if( InterfacePointerGood() )
	{
		bRetVal = HandleCOMError( m_pRoutingEngine->RestoreBackup( BackupFile, pError ) );

		// Try twice 
		if( bRetVal == FALSE )
		{
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->RestoreBackup( BackupFile, pError ) );
			}
		}
	}

	return bRetVal;
}

////////////////////////////////////////////////////////////////////////////////
// 
// VirusScanFile
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::VirusScanFile( LPCTSTR szFile, long Options, tstring& sVirusName, 
								    tstring& sQuarantineLoc, long* pError )
{
	BOOL bRetVal = FALSE;

	try
	{
		dca::BString bFile(szFile);
		dca::BString bVirus;
		dca::BString bQuarantineLoc;

		if( InterfacePointerGood() )
		{
			bRetVal = HandleCOMError( m_pRoutingEngine->ScanFile( bFile, Options, bVirus.GetRef(), bQuarantineLoc.GetRef(), pError ) );

			// Try twice 
			if( bRetVal == FALSE )
			{
				if ( InterfacePointerGood() )
				{
					bRetVal = HandleCOMError( m_pRoutingEngine->ScanFile( bFile, Options, bVirus.GetRef(), bQuarantineLoc.GetRef(), pError ) );
				}
			}
		}

		if(bVirus.Get() != NULL )
		{
			dca::String sVirus(bVirus);

			sVirusName.assign(sVirus.c_str());
		}

		if(bQuarantineLoc.Get() != NULL )
		{
			dca::String sQuarantine(bQuarantineLoc);
			
			sQuarantineLoc.assign(sQuarantine.c_str());
		}
	}
	catch(dca::Exception e)
	{
		LPCWSTR s = e.GetMessage();
	}

	return bRetVal;
}

////////////////////////////////////////////////////////////////////////////////
// 
// VirusScanString
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::VirusScanString( LPCTSTR szString, tstring& sVirusName, 
									  tstring& sCleanedString, long* pError )
{
	BOOL bRetVal = FALSE;
	BSTR bstrString;
	BSTR bstrVirus = NULL;
	BSTR bstrCleanedString = NULL;

#ifdef _UNICODE 

	bstrString = SysAllocString( szString );
		
#else
	
	int nLen = _tcslen( szString );
	if (nLen < 1 )
	{
		*pError = 0;
		return TRUE;
	}

	wchar_t* wcsFormattedString = new wchar_t[nLen+1];
	MultiByteToWideChar( CP_ACP, 0, szString, nLen+1,
						 wcsFormattedString, nLen+1 );
	bstrString = SysAllocString( wcsFormattedString );
	delete[] wcsFormattedString;

#endif

	if( InterfacePointerGood() )
	{
		bRetVal = HandleCOMError( m_pRoutingEngine->ScanString( bstrString, &bstrVirus, &bstrCleanedString, pError ) );

		// Try twice 
		if( bRetVal == FALSE )
		{
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->ScanString( bstrString, &bstrVirus, &bstrCleanedString, pError ) );
			}
		}
	}

	SysFreeString( bstrString );

	if( bstrVirus != NULL )
	{
		int Len = SysStringLen(bstrVirus) + 1;
		sVirusName.resize( Len );

		WideCharToMultiByte( CP_ACP, 0, bstrVirus, Len,
		                     (char*) sVirusName.c_str(), Len, NULL, NULL );
		SysFreeString( bstrVirus );
	}

	if( bstrCleanedString != NULL && 
		*pError == CKAVScan::Success_Object_Has_Been_Cleaned )
	{
		int Len = SysStringLen(bstrCleanedString) + 1;
		sCleanedString.resize( Len );

		WideCharToMultiByte( CP_ACP, 0, bstrCleanedString, Len,
		                     (char*) sCleanedString.c_str(), Len, NULL, NULL );
		SysFreeString( bstrCleanedString );
	}

	return bRetVal;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetAVInfo
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngine::GetAVInfo( tstring& sVersion, tstring& sRecordCount,
							    tstring& sLastUpdate, long* pError )
{
	BOOL bRetVal = FALSE;
	BSTR bstrVersion = NULL;
	BSTR bstrLastUpdate = NULL;
	BSTR bstrRecordCount = NULL;
	int Len;

	if( InterfacePointerGood() )
	{
		bRetVal = HandleCOMError( m_pRoutingEngine->GetAVInfo( &bstrVersion, &bstrRecordCount, 
															   &bstrLastUpdate, pError ) );

		// Try twice 
		if( bRetVal == FALSE )
		{
			if ( InterfacePointerGood() )
			{
				bRetVal = HandleCOMError( m_pRoutingEngine->GetAVInfo( &bstrVersion, &bstrRecordCount, 
															           &bstrLastUpdate, pError ) );
			}
		}
	}


	if( bstrVersion )
	{
#ifdef UNICODE
		sVersion = bstrVersion;
#else
		Len = SysStringLen(bstrVersion) + 1;
		sVersion.resize( Len );
		WideCharToMultiByte( CP_ACP, 0, bstrVersion, Len,
		                     (char*) sVersion.c_str(), Len, NULL, NULL );
#endif
		SysFreeString( bstrVersion );
	}

	if( bstrRecordCount )
	{
#ifdef UNICODE
		sRecordCount = bstrRecordCount;
#else
		Len = SysStringLen(bstrRecordCount) + 1;
		sRecordCount.resize( Len );
		WideCharToMultiByte( CP_ACP, 0, bstrRecordCount, Len,
		                     (char*) sRecordCount.c_str(), Len, NULL, NULL );
#endif
		SysFreeString( bstrRecordCount );
	}

	if( bstrLastUpdate )
	{
#ifdef UNICODE
		sLastUpdate = bstrLastUpdate;
#else
		Len = SysStringLen(bstrLastUpdate) + 1;
		sLastUpdate.resize( Len );
		WideCharToMultiByte( CP_ACP, 0, bstrLastUpdate, Len,
		                     (char*) sLastUpdate.c_str(), Len, NULL, NULL );
#endif
		SysFreeString( bstrLastUpdate );
	}


	return bRetVal;
}
