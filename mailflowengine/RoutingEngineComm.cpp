////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/RoutingEngineComm.cpp,v 1.1.6.1 2006/07/18 12:50:09 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// RoutingEngineComm.cpp : Implementation of CRoutingEngineComm

#include "stdafx.h"
#include "MailStreamEngine.h"
#include "RoutingEngineComm.h"
#include "VirusScanningThread.h"
#include "EMSEvent.h"

extern long g_dwDBMaintResults;

/////////////////////////////////////////////////////////////////////////////
// CRoutingEngineComm


CRoutingEngineComm::CRoutingEngineComm()
{
}

CRoutingEngineComm::~CRoutingEngineComm()
{
}


STDMETHODIMP CRoutingEngineComm::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IRoutingEngineComm
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// 
// method ReloadConfig - Reload Workflow Engine Configuration from database
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::ReloadConfig( long nConfigurationItem )
{
	// Create a manual reset event
	HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// We duplicate a handle so that we don't have to worry about who
	// closes the event handle.
	HANDLE hEventDuplicate;

	DuplicateHandle( GetCurrentProcess(), hEvent,
					 GetCurrentProcess(), &hEventDuplicate, 
					 0, FALSE, DUPLICATE_SAME_ACCESS );

	g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::Worker, WM_RELOAD_CONFIG, 
		                                        (WPARAM) hEventDuplicate, nConfigurationItem );

	g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::WebSession, WM_RELOAD_CONFIG, 0, 0 );

	// Reload Virus Scanner config
	//g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::VirusScanner, WM_RELOAD_CONFIG,0, 0 );

	if(    nConfigurationItem == EMS_MessageSources 
		|| nConfigurationItem == EMS_MessageDestinations 
		|| nConfigurationItem == EMS_Registration 
		|| nConfigurationItem == EMS_ServerParameters
		|| nConfigurationItem == EMS_LogConfig)
	{
		g_Object.m_MessagingComponents.RefreshServerParams();
	}

	// tell the ISAPI to reload
	if( nConfigurationItem == EMS_ISAPI )
	{
		CReloadISAPIEvent reloadEvent;
		SetEvent( reloadEvent.GetHandle() );
	}


	// Tell other servers to reload
	g_Object.SetServerReload();	

	WaitForSingleObject( hEvent, 5000 );

	CloseHandle( hEvent );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// method ProcessInboundQueue - Process all message sources
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::ProcessInboundQueue()
{
	g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::Worker, WM_PROCESS_INBOUND, 0, 0 );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// method ProcessOutboundQueue - Process all message destinations
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::ProcessOutboundQueue()
{
	g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::Worker, WM_PROCESS_OUTBOUND, 0, 0 );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Log
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::Log(long ErrorCode, BSTR Text)
{
	wchar_t* wcsText = (wchar_t*) HeapAlloc (GetProcessHeap (), 0, sizeof(wchar_t) * (wcslen(Text)+1) );

	wcscpy( wcsText, Text );
	

	g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::Worker, WM_WRITE_LOG, 
		                                        ErrorCode, (LPARAM)wcsText );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendAlert
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::SendAlert(long AlertEventID, long TicketBoxID, BSTR Text)
{

	AlertInfo* pAlertInfo = (AlertInfo*) HeapAlloc ( GetProcessHeap (), 0, 
		                                             sizeof(AlertInfo) + (SysStringLen(Text)*2) );

	if ( AlertEventID == 14 || AlertEventID == 15 || AlertEventID == 16 || AlertEventID == 17)
	{
		pAlertInfo->TicketBoxID = 0;
		pAlertInfo->AlertID = TicketBoxID;
	}
	else
	{
		pAlertInfo->TicketBoxID = TicketBoxID;
		pAlertInfo->AlertID = 0;		
	}

	wcscpy( pAlertInfo->wcsText, Text );

	g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::Worker, WM_SEND_ALERT, 
		                                        AlertEventID,(LPARAM) pAlertInfo );
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 
// TestPOP3Auth
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::TestPOP3Auth( BSTR HostName, long nPort, VARIANT_BOOL IsAPOP, BSTR UserName,
		                                       BSTR Password, long TimeoutSecs, long isSSL, long* pError )
{
	//DebugReporter::Instance().DisplayMessage("CRoutingEngineComm::TestPOP3Auth - Ready to call message component", DebugReporter::ENGINE);

	*pError = g_Object.m_MessagingComponents.TestPOP3Auth( HostName, nPort, (IsAPOP == 0) ? FALSE : TRUE,
														   UserName, Password, TimeoutSecs, isSSL );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// TestSMTPAuth
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::TestSMTPAuth( BSTR HostName, long nPort, BSTR UserName,
		                                       BSTR Password, long TimeoutSecs, long isSSL, long* pError )
{
	*pError = g_Object.m_MessagingComponents.TestSMTPAuth( HostName, nPort, UserName, Password, 
		                                                   TimeoutSecs, isSSL );

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 
// DoDBMaintenance
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::DoDBMaintenance( long* pError )
{
	// Create a manual reset event
	HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// We duplicate a handle so that we don't have to worry about who
	// closes the event handle.
	HANDLE hEventDuplicate;

	DuplicateHandle( GetCurrentProcess(), hEvent,
					 GetCurrentProcess(), &hEventDuplicate, 
					 0, FALSE, DUPLICATE_SAME_ACCESS );


	g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::DBMaint, WM_DB_MAINTENANCE_NOW, 
		                                        (WPARAM) 0, (LPARAM) hEventDuplicate );

	WaitForSingleObject( hEvent, INFINITE );

	CloseHandle( hEvent );

	*pError = g_dwDBMaintResults;

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 
// RestoreArchive
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::RestoreArchive( long ArchiveID, long* pError )
{
	// Create a manual reset event
	HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// We duplicate a handle so that we don't have to worry about who
	// closes the event handle.
	HANDLE hEventDuplicate;

	DuplicateHandle( GetCurrentProcess(), hEvent,
					 GetCurrentProcess(), &hEventDuplicate, 
					 0, FALSE, DUPLICATE_SAME_ACCESS );


	g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::Worker, WM_RESTORE_ARCHIVE, 
										        (WPARAM) ArchiveID, (LPARAM) hEventDuplicate  );

	WaitForSingleObject( hEvent, INFINITE );

	CloseHandle( hEvent );

	*pError = g_dwDBMaintResults;

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 
// RestoreBackup
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::RestoreBackup( BSTR BackupFile, long* pError )
{
	// Create a manual reset event
	HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// We duplicate a handle so that we don't have to worry about who
	// closes the event handle.
	HANDLE hEventDuplicate;

	DuplicateHandle( GetCurrentProcess(), hEvent,
					 GetCurrentProcess(), &hEventDuplicate, 
					 0, FALSE, DUPLICATE_SAME_ACCESS );

	wchar_t* wcsBackupFile = new wchar_t[wcslen(BackupFile)+1];

	wcscpy( wcsBackupFile, BackupFile );

	g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::Worker, WM_RESTORE_BACKUP,
		                                        (WPARAM) wcsBackupFile, (LPARAM) hEventDuplicate );

	WaitForSingleObject( hEvent, INFINITE );

	CloseHandle( hEvent );

	*pError = g_dwDBMaintResults;

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 
// ScanFile
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::ScanFile( BSTR File, long Options, BSTR* VirusName, 
										   BSTR* QuarantineLoc, long* pError )
{
	VirusScanInfo vsinfo;
	ZeroMemory( &vsinfo, sizeof(vsinfo) );

	dca::String f(File);
	strncpy( vsinfo.szFile, f.c_str(), MAX_PATH );
	vsinfo.bFile = true;
	vsinfo.nOptions = Options;

	// Create a manual reset event
	HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// We duplicate a handle so that we don't have to worry about who
	// closes the event handle.

	DuplicateHandle( GetCurrentProcess(), hEvent,
					 GetCurrentProcess(), &vsinfo.hEvent, 
					 0, FALSE, DUPLICATE_SAME_ACCESS );


	//g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::VirusScanner, WM_VIRUS_SCAN, (WPARAM) &vsinfo, (LPARAM) NULL  );

	WaitForSingleObject( hEvent, INFINITE );

	CloseHandle( hEvent );

	// Handle return code and do logging.
	*pError = vsinfo.nRet;

	if( vsinfo.szVirusName[0] )
	{
		dca::WString v(vsinfo.szVirusName);
		*VirusName = SysAllocString( v.c_str() );
	}
	else
	{
		*VirusName = NULL;
	}

	if( vsinfo.szQuarantineLoc[0] )
	{
		dca::WString q(vsinfo.szQuarantineLoc);
		*QuarantineLoc = SysAllocString( q.c_str() );
	}
	else
	{
		*QuarantineLoc = NULL;
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 
// ScanString
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::ScanString( BSTR String, BSTR* VirusName, BSTR* CleanedString, 
											 long* pError )
{
	VirusScanInfo vsinfo;
	ZeroMemory( &vsinfo, sizeof(vsinfo) );

	int len = SysStringLen(String);
		
	vsinfo.bFile = false;
	vsinfo.szString = (char*) GlobalAlloc( GMEM_FIXED, len + 1 );

	// Convert to multi-byte
	WideCharToMultiByte( CP_ACP, 0, String, len+1, vsinfo.szString, len+1, NULL, NULL );

	// Create a manual reset event
	HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// We duplicate a handle so that we don't have to worry about who
	// closes the event handle.

	DuplicateHandle( GetCurrentProcess(), hEvent,
					 GetCurrentProcess(), &vsinfo.hEvent, 
					 0, FALSE, DUPLICATE_SAME_ACCESS );


	//g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::VirusScanner, WM_VIRUS_SCAN,(WPARAM) &vsinfo, (LPARAM) NULL  );

	WaitForSingleObject( hEvent, INFINITE );

	CloseHandle( hEvent );
	
	// Handle return code and do logging.
	*pError = vsinfo.nRet;

	if( vsinfo.szVirusName[0] )
	{
		dca::WString v(vsinfo.szVirusName);
		*VirusName = SysAllocString( v.c_str() );
	}
	else
	{
		*VirusName = NULL;
	}

	if( vsinfo.nRet == CKAVScan::Success_Object_Has_Been_Cleaned )
	{
		int newlen = strlen(vsinfo.szString)+1;
		wchar_t* wcsString = new wchar_t[newlen];
		MultiByteToWideChar( CP_ACP, 0, vsinfo.szString, newlen, wcsString, newlen );
		*CleanedString = SysAllocString( wcsString );
		delete[] wcsString;
	}
	else
	{
		*CleanedString = NULL;
	}

	GlobalFree( vsinfo.szString );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetAVInfo
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineComm::GetAVInfo( BSTR* Version, BSTR *RecordCount, 
											BSTR* LastUpdate, long* pError )
{

	dca::WString v(CVirusScanningThread::m_sVersion.c_str());
	*Version = SysAllocString( v.c_str() );

	dca::WString r(CVirusScanningThread::m_sRecordCount.c_str());
	*RecordCount = SysAllocString( r.c_str() );

	dca::WString l(CVirusScanningThread::m_sLastUpdate.c_str());
	*LastUpdate = SysAllocString( l.c_str() );

	*pError = 0;

	return S_OK;
}
