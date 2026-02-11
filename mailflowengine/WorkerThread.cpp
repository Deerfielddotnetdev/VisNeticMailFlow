////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/WorkerThread.cpp,v 1.2 2005/11/29 21:36:28 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// WorkerThread.cpp: implementation of the CWorkerThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MailStreamEngine.h"
#include "RoutingEngineComm.h"
#include "WorkerThread.h"
#include "EMSEvent.h"

#include "klav.h"
#include "klav_i.c"

extern CServiceModule _Module;
static CLogWriter* s_pLogger = NULL;
static CAlertSender* s_pAlerter = NULL;

////////////////////////////////////////////////////////////////////////////////
// 
// Logging functions for worker thread children.
// 
////////////////////////////////////////////////////////////////////////////////
void Log( long ErrorCode, wchar_t* wcsText, ... )
{
	CEMSString szLogString;

	if( s_pLogger )
	{
		va_list va;
		va_start( va, wcsText );
		szLogString.FormatArgList(wcsText, va);
		va_end(va);

		wchar_t* pString = (wchar_t*) HeapAlloc (GetProcessHeap (), 0, sizeof(wchar_t) * (szLogString.size()+1) );
		wcscpy( pString, szLogString.c_str() );

		s_pLogger->Log( ErrorCode, pString );

	}
}
	
void Log( long ErrorCode, unsigned int StringID, ... )
{
	CEMSString szLogString;

	if( s_pLogger )
	{
		va_list va;
		va_start( va, StringID );
		szLogString.FormatArgList(StringID, va);
		va_end(va);

		wchar_t* pString = (wchar_t*) HeapAlloc (GetProcessHeap (), 0, sizeof(wchar_t) * (szLogString.size()+1) );
		wcscpy( pString, szLogString.c_str() );

		s_pLogger->Log( ErrorCode, (wchar_t*) pString );
	}
}



void Alert( long AlertEventID, long TicketBoxID, wchar_t* wcsText, ... )
{
	if( s_pAlerter )
	{
		CEMSString szLogString;

		va_list va;
		va_start( va, wcsText );
		szLogString.FormatArgList(wcsText, va);
		va_end(va);

		AlertInfo* pAlertInfo = (AlertInfo*) HeapAlloc ( GetProcessHeap (), 0,
			                                             sizeof(AlertInfo) + (szLogString.size()*2) );

		pAlertInfo->TicketBoxID = TicketBoxID;
		pAlertInfo->AlertID = 0;
		
		wcscpy( pAlertInfo->wcsText, szLogString.c_str() );

		s_pAlerter->SendAlert( AlertEventID, pAlertInfo );
	}	
}

void Alert( long AlertEventID, long TicketBoxID, unsigned int StringID, ... )
{
	
	if( s_pAlerter )
	{
		CEMSString szLogString;
			
		va_list va;
		va_start( va, StringID );
		szLogString.FormatArgList(StringID, va);
		va_end(va);

		AlertInfo* pAlertInfo = (AlertInfo*) HeapAlloc ( GetProcessHeap (), 0, 
														 sizeof(AlertInfo) + (szLogString.size()*2) );

		pAlertInfo->TicketBoxID = TicketBoxID;
		pAlertInfo->AlertID = 0;
		
		wcscpy( pAlertInfo->wcsText, szLogString.c_str() );

		s_pAlerter->SendAlert( AlertEventID, pAlertInfo );
	}
}
	
void CustomAlert( long AlertEventID, long AlertID, wchar_t* wcsText, ... )
{
	if( s_pAlerter )
	{
		CEMSString szLogString;

		va_list va;
		va_start( va, wcsText );
		szLogString.FormatArgList(wcsText, va);
		va_end(va);

		AlertInfo* pAlertInfo = (AlertInfo*) HeapAlloc ( GetProcessHeap (), 0,
			                                             sizeof(AlertInfo) + (szLogString.size()*2) );
		
		pAlertInfo->TicketBoxID = 0;
		pAlertInfo->AlertID = AlertID;
		
		wcscpy( pAlertInfo->wcsText, szLogString.c_str() );

		s_pAlerter->SendAlert( AlertEventID, pAlertInfo );
	}	
}
	


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWorkerThread::CWorkerThread() : m_query(m_db)
{
	m_bInitialized = FALSE;
	m_bNeverConnectedToDB = TRUE;

	m_pRouter = new CMsgRouter( m_query );
	//m_pMonitor = new CTicketMonitor( m_query );
	s_pLogger = m_pLogger = new CLogWriter( m_query );
	s_pAlerter = m_pAlerter = new CAlertSender( m_query );
	m_pDBMaint = new CDBMaintenance( m_query, m_db );
	m_pReporter = new CReporter( m_query );
	
	m_nConnectToDBFailures = 0;
	m_dwLastRegCheck = 0;
}

CWorkerThread::~CWorkerThread()
{
	delete m_pRouter;
	//delete m_pMonitor;
	s_pLogger = NULL;
	delete m_pLogger;
	delete m_pAlerter;
	s_pAlerter = NULL;
	delete m_pDBMaint;
	delete m_pReporter;
}


////////////////////////////////////////////////////////////////////////////////
// 
//  Initialize
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::Initialize(void)
{
	int nRet;
		
	// Make sure our allocations in the constructor were successful:
	//if( !m_pRouter || !m_pMonitor || !m_pLogger || !m_pAlerter || !m_pDBMaint || !m_pReporter )
	if( !m_pRouter || !m_pLogger || !m_pAlerter || !m_pDBMaint || !m_pReporter )
	{
		CEMSString s;
		s.LoadString( EMS_STRING_ERROR_MEMORY );
		HandleException( (wchar_t*) s.c_str() );
		return;
	}

	m_pDBMaint->SetKillEvent( m_hKillEvent );

	// We can only get here if no exception is thrown
	m_bNeverConnectedToDB = FALSE;
	m_nConnectToDBFailures = 0;
	
	WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("MailCheckRestartEngine"), 0 );

	if( m_bInitialized == FALSE )
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::Initialize - Calling GetDBParams()", DebugReporter::ENGINE);
		if SUCCEEDED( g_Object.GetDBParams( m_db) )
		{
			DebugReporter::Instance().DisplayMessage("CWorkerThread::Initialize - GetDBParams() succeeded", DebugReporter::ENGINE);
			m_bInitialized = TRUE;
			
			m_pLogger->Log( E_EngineStarting, EMS_STRING_SYSTEM_STARTING );
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CWorkerThread::Initialize - GetDBParams() failed", DebugReporter::ENGINE);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
//  Uninitialize
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::Uninitialize()
{
	try
	{	
		m_pRouter->UnInitialize();
		m_pLogger->Log( E_EngineStopping, EMS_STRING_SYSTEM_STOPPING );
		m_pLogger->UnInitialize();

		if( m_db.IsConnected() )
		{
			UINT nServerID;
			if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
							EMS_SERVER_ID_VALUE, nServerID ) != ERROR_SUCCESS)
			{
				nServerID = 1;
			}
			TIMESTAMP_STRUCT Now;
			GetTimeStamp( Now );
			m_query.Initialize();
			BINDPARAM_TIME_NOLEN( m_query, Now );
			BINDPARAM_LONG( m_query, nServerID );
			m_query.Execute( L"UPDATE Servers SET Status=0,CheckIn=?,ReloadConfig=0 WHERE ServerID=?" );
			
			m_pAlerter->UnInitialize();
			m_query.Close();
			m_db.Disconnect();
		}
	}
	catch( ODBCError_t error )
	{
		if( error.nErrorCode == E_DSNNotInRegistry )
		{
			CEMSString s;
			s.LoadString( EMS_STRING_ERROR_NODSN );
			wcscpy( error.szErrMsg, s.c_str() );
		}

		// Close the database connection
		if( m_db.IsConnected() )
		{
			m_query.Close();
			m_db.Disconnect();
		}	
	}		
	catch( ... )
	{
		CEMSString s;
		s.LoadString( EMS_STRING_ERROR_UNHANDLED_EXCEPTION );
		HandleException( (wchar_t*) s.c_str() );
	}

	m_bInitialized = FALSE;
}


////////////////////////////////////////////////////////////////////////////////
// 
//  Run
// 
////////////////////////////////////////////////////////////////////////////////
unsigned int CWorkerThread::Run()
{
	DWORD dwRet;
	MSG msg;
	BOOL bStuffToDo = FALSE;
	BOOL bNeverConnectedToDB = TRUE;
	DWORD dwWaitTime;

	m_bRun = TRUE;

	// Create the message queue
	PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );
	SetEvent( m_hReadyEvent );

	while ( m_bRun )
	{
		try
		{
			// If we have stuff to do, let's get it done, otherwise we can free
			// up the CPU for other processes.
			dwWaitTime = ( bStuffToDo ) ? 0 : 1000;

			bStuffToDo = FALSE;

			// Could wait for other events here as well...
			dwRet = MsgWaitForMultipleObjects( 1, &m_hKillEvent, FALSE, dwWaitTime, QS_ALLEVENTS );

			switch( dwRet )
			{
			case WAIT_TIMEOUT:
				if( m_bInitialized )
				{
					if( m_db.IsConnected() )
					{
						if( CheckRegistration() == true )
						{
							bStuffToDo = DoWork();
						}
					}
					else
					{
						m_db.Connect();
					}
				}
				else
				{
					Initialize();
				}
				break;

			case WAIT_OBJECT_0 + 1:		// Message in queue
				while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
				{				
					HandleMsg( &msg );
					bStuffToDo = TRUE;
				}
				break;

			case WAIT_OBJECT_0:			// Kill Event is set.
				m_bRun = FALSE;
				break;
			}
		}
		catch( ODBCError_t error )
		{
			HandleODBCError( &error );
		}		
		catch( ... )
		{
			CEMSString s;
			s.LoadString( EMS_STRING_ERROR_UNHANDLED_EXCEPTION );
			HandleException( (wchar_t*) s.c_str() );
		}
	}
 
	Uninitialize();

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// DoWork - Objects are only asked to work with a valid database connection 
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CWorkerThread::DoWork(void)
{
	BOOL bRet = FALSE;

	// Any messages to route?
	if( m_pRouter->Run() == S_OK )
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::DoWork - Messages were routed", DebugReporter::ENGINE);
		bRet = TRUE;
	}
	
	// Any tickets alerts to send?
	//if( m_pMonitor->Run() == S_OK )
	//{
		//DebugReporter::Instance().DisplayMessage("CWorkerThread::DoWork - Ticket alerts were sent", DebugReporter::ENGINE);
		//bRet = TRUE;
	//}
	
	// Any alerts need to be written?
	if( m_pAlerter->Run() == S_OK )
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::DoWork - Ticket alerts were written", DebugReporter::ENGINE);
		bRet = TRUE;
	}

	// Any log items to be written?
	if( m_pLogger->Run() == S_OK )
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::DoWork - Ticket log items were written", DebugReporter::ENGINE);
		bRet = TRUE;
	}

	// Has the Engine checked in lately?
	if( CheckEngine() == true )
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::DoWork - DBMonitor thread was restarted", DebugReporter::ENGINE);
		bRet = TRUE;
	}

	// Any database maintenance to do?
	//if( m_pDBMaint->Run() == S_OK )
	//{
		//DebugReporter::Instance().DisplayMessage("CWorkerThread::DoWork - Database maintenance was run", DebugReporter::ENGINE);
		//bRet = TRUE;
	//}
	
	// Any reports to run?
	if( m_pReporter->Run() == S_OK )
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::DoWork - Scheduled reports were processed", DebugReporter::ENGINE);
		bRet = TRUE;
	}	

	return bRet;
}


////////////////////////////////////////////////////////////////////////////////
// 
// HandleMsg
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::HandleMsg( MSG* msg )
{
	HANDLE hEvent;

	switch( msg->message )
	{
	case WM_RELOAD_CONFIG:
		{
			FinishJobsInProgress();

			hEvent = (HANDLE)msg->wParam;

			SetEvent( hEvent );	
			CloseHandle( hEvent );

			// Force a refresh of the logging configuration
			m_pLogger->m_bMapLoaded = FALSE;
			DebugReporter::Instance().SetInitialized(0);
			g_Object.GetDBParams( m_db );

		}

		break;

	case WM_PROCESS_OUTBOUND:
		g_Object.m_MessagingComponents.SendMailNow();
		break;

	case WM_PROCESS_INBOUND:
		g_Object.m_MessagingComponents.CheckMailNow();
		break;

	case WM_RETRY_OUTBOUND:
		break;
		
	case WM_SEND_ALERT:
		m_pAlerter->SendAlert( msg->wParam, (AlertInfo*)msg->lParam );
		break;

	case WM_WRITE_LOG:
		m_pLogger->Log( msg->wParam, (wchar_t*)msg->lParam );
		break;
		

	case WM_DB_MAINTENANCE_NOW:

		FinishJobsInProgress();

		// WPARAM is not used and  LPARAM is an event to signal when done
		hEvent = (HANDLE)msg->lParam;	
		try
		{
			// g_Object.m_MessagingComponents.Shutdown();
			m_pDBMaint->RunMaintenance( true );	
		}
		catch( ... )
		{
			// g_Object.m_MessagingComponents.Initialize();
			SetEvent( hEvent );	
			CloseHandle( hEvent );
			throw;
		}
		// g_Object.m_MessagingComponents.Initialize();
		SetEvent( hEvent );	
		CloseHandle( hEvent );
		break;

	case WM_RESTORE_ARCHIVE:

		FinishJobsInProgress();

		// WPARAM is the Archive ID. LPARAM is an event to signal when done
		hEvent = (HANDLE)msg->lParam;	
		try
		{
			g_Object.m_MessagingComponents.Shutdown();
			m_pDBMaint->RestoreArchive( msg->wParam );
		}
		catch( ... )
		{
			g_Object.m_MessagingComponents.Initialize();
			SetEvent( hEvent );	
			CloseHandle( hEvent );
			throw;
		}
		g_Object.m_MessagingComponents.Initialize();
		SetEvent( hEvent );	
		CloseHandle( hEvent );
		
		{
			// Force the ISAPI to reload
			CReloadISAPIEvent reloadEvent;
			SetEvent( reloadEvent.GetHandle() );
		}

		break;

	case WM_RESTORE_BACKUP:

		FinishJobsInProgress();

		// WPARAM is a pointer to a wchar_t string that has been allocated with new[]
		//        The worker thread will delete[] this buffer. LPARAM is an event to signal when done
		hEvent = (HANDLE)msg->lParam;	
		try
		{
			g_Object.m_MessagingComponents.Shutdown();
			m_pDBMaint->RestoreBackup( (wchar_t*)msg->wParam );
		}
		catch( ... )
		{
			g_Object.m_MessagingComponents.Initialize();
			SetEvent( hEvent );	
			CloseHandle( hEvent );
			throw;
		}

		g_Object.m_MessagingComponents.Initialize();
		
		// Force a refresh of the logging configuration
		m_pLogger->m_bMapLoaded = FALSE;
		g_Object.GetDBParams( m_db);

		SetEvent( hEvent );	
		CloseHandle( hEvent );

		{
			// Force the ISAPI to reload
			CReloadISAPIEvent reloadEvent;
			SetEvent( reloadEvent.GetHandle() );
		}

		break;
		
	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// FinishJobsInProgress
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::FinishJobsInProgress(void)
{
	BOOL bFinished = FALSE;

	while( !bFinished )
	{
		try
		{
			m_pRouter->FinishCurrentJob();
			//m_pMonitor->FinishCurrentJob();			
			bFinished = TRUE;
		}
		catch( ODBCError_t error )
		{
			HandleODBCError( &error );
		}	
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// HandleODBCError
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::HandleODBCError( ODBCError_t* pErr )
{
	wchar_t szDebug[1024];


	if( pErr->nErrorCode == E_DSNNotInRegistry )
	{
		CEMSString s;
		s.LoadString( EMS_STRING_ERROR_NODSN );
		wcscpy( pErr->szErrMsg, s.c_str() );
	}

	swprintf( szDebug, L"Caught ODBCError: %d, %s\n", 
			  pErr->nErrorCode, pErr->szErrMsg );

	Log( E_WorkerODBCError, szDebug );

	// Close the database connection
	if( m_db.IsConnected() )
	{
		m_query.Close();
		m_db.Disconnect();
	}

	if( m_bNeverConnectedToDB )
	{
		m_nConnectToDBFailures++;

		if( m_nConnectToDBFailures > 9 )
		{
			// log an error to the event log and bail
			_Module.m_status.dwWin32ExitCode = ERROR_DATABASE_DOES_NOT_EXIST;
			_Module.LogEvent( L"Error opening database connection: %s", pErr->szErrMsg );		
			PostThreadMessage( _Module.dwThreadID, WM_QUIT, 0, 0);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// HandleException
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::HandleException( wchar_t* wcsDescription )
{
	_Module.m_status.dwWin32ExitCode = ERROR_EXCEPTION_IN_SERVICE;
	_Module.LogEvent( wcsDescription );		
	Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, L"VisNetic MailFlow Service stopping due to unhandled exception: %s", 
		   wcsDescription );
	WaitForSingleObject( m_hKillEvent, 1000 );
	PostThreadMessage( _Module.dwThreadID, WM_QUIT, 0, 0);
	m_bRun = FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// 
// CheckRegistration
// 
////////////////////////////////////////////////////////////////////////////////
bool CWorkerThread::CheckRegistration(void)
{
	//DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Entering registration check", DebugReporter::ENGINE);	
	
	DWORD dwTicks = GetTickCount();
	int nAllowedAgents = 0;
	int nAgentsInDB = 0;
	int nAllowedServers = 0;
	int nServersInDB = 0;
	wchar_t* szWarning;

	if(( m_dwLastRegCheck == 0 ) || ((dwTicks - m_dwLastRegCheck) > (60*1000)))
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Getting key info via global object", DebugReporter::ENGINE);	
		
		g_Object.m_LicenseMgr.GetKeyInfo( m_query );

		if( g_Object.m_LicenseMgr.m_MFKeyPresent == false )
		{
			DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Key missing", DebugReporter::ENGINE);	
			
			_Module.m_status.dwWin32ExitCode = CLASS_E_NOTLICENSED;
			_Module.LogEvent( L"Product is not registered" );
			szWarning = L"VisNetic MailFlow Service stopping because product is not registered.";
			Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, szWarning );
			Log( E_RegistrationWarning, (wchar_t*) szWarning );
			// Give the system time to log the event.
			WaitForSingleObject( m_hKillEvent, 1000 );
			PostThreadMessage( _Module.dwThreadID, WM_QUIT, 0, 0);
			m_bRun = FALSE;
			return false;
		}
		else if(g_Object.m_LicenseMgr.m_MFKeyExpiredBuildDate == true)
		{
			DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - License not valid for build date", DebugReporter::ENGINE);	
			
			_Module.m_status.dwWin32ExitCode = CLASS_E_NOTLICENSED;
			_Module.LogEvent( L"Product is not registered" );
			szWarning = L"VisNetic MailFlow Service stopping because product is not registered.";
			Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, szWarning );
			Log( E_RegistrationWarning, (wchar_t*) szWarning );
			// Give the system time to log the event.
			WaitForSingleObject( m_hKillEvent, 1000 );
			PostThreadMessage( _Module.dwThreadID, WM_QUIT, 0, 0);
			m_bRun = FALSE;
			return false;
		}
		else if(g_Object.m_LicenseMgr.m_MFKeyExpired == true && g_Object.m_LicenseMgr.m_MFKeyInfo.KeyType == DCIKEY_KeyType_Evaluation)
		{
			DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Expired evaluation", DebugReporter::ENGINE);	
			
			_Module.m_status.dwWin32ExitCode = CLASS_E_NOTLICENSED;
			_Module.LogEvent( L"Product is not registered" );
			szWarning = L"VisNetic MailFlow Service stopping because product is not registered.";
			Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, szWarning );
			Log( E_RegistrationWarning, (wchar_t*) szWarning );
			// Give the system time to log the event.
			WaitForSingleObject( m_hKillEvent, 1000 );
			PostThreadMessage( _Module.dwThreadID, WM_QUIT, 0, 0);
			m_bRun = FALSE;
			return false;
		}

		DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Key is present, within build date and not expired eval.", DebugReporter::ENGINE);	

		// If Virus Scanning is enabled, verify the key
		if( _wtoi( g_Object.GetParameter( EMS_SRVPARAM_ANTIVIRUS_ENABLE ) ) )
		{
			DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - AV is enabled, entering AV key check", DebugReporter::ENGINE);	
			
			try
			{
				// Virus Scanning is enabled.
				IAVPAntivirus* pAVPAntivirus;
				bool bDisableScanning = false;

				DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Checking to see if AV is installed", DebugReporter::ENGINE);	

				// First Check if antivirus is installed
				HRESULT hr = CoCreateInstance(	CLSID_AVPAntivirus, NULL, CLSCTX_LOCAL_SERVER, IID_IAVPAntivirus, (void**) &pAVPAntivirus );

				if (SUCCEEDED(hr)) 
				{
					// Virus Scanning is installed.

					DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - AV is installed", DebugReporter::ENGINE);	

					pAVPAntivirus->Release();

					// If no key, or key is expired
					if( g_Object.m_LicenseMgr.m_AVKeyPresent == false || g_Object.m_LicenseMgr.m_AVKeyExpired == true )
					{
						// If an eval key expired or no key is present, disable scanning
						if( g_Object.m_LicenseMgr.m_AVKeyExpired == true && g_Object.m_LicenseMgr.m_AVKeyInfo.KeyType == DCIKEY_KeyType_Evaluation ) 
						{
							DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - AV is enabled, eval key expired", DebugReporter::ENGINE);	
							
							// eval key expired, disable anti-virus
							bDisableScanning = true;			
							szWarning = L"AntiVirus scanning has been disabled because the evaluation period expired.";
							Alert( EMS_ALERT_EVENT_PROD_REGISTRATION, 0, szWarning );
							Log( E_RegistrationWarning, (wchar_t*) szWarning );
						}
						else if( g_Object.m_LicenseMgr.m_AVKeyPresent == false  )
						{
							DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - AV is enabled, key is missing", DebugReporter::ENGINE);	
							
							// no reg key, disable anti-virus
							bDisableScanning = true;			
							szWarning = L"AntiVirus scanning has been disabled because no registration key was found.";
							Alert( EMS_ALERT_EVENT_PROD_REGISTRATION, 0, szWarning );
							Log( E_RegistrationWarning, (wchar_t*) szWarning );
						}
						// Otherwise - reg key expired - Change the action to delete
						else if( _wtoi( g_Object.GetParameter( EMS_SRVPARAM_ANTIVIRUS_ACTION ) ) != 1 )
						{
							DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - AV is enabled, key is expired, setting reduced functionality", DebugReporter::ENGINE);	
							
							szWarning = L"AntiVirus scanning is now operating in reduced functionality mode because a registered key has expired.";
							Alert( EMS_ALERT_EVENT_PROD_REGISTRATION, 0, szWarning );
							Log( E_RegistrationWarning, (wchar_t*) szWarning );
							int nID = EMS_SRVPARAM_ANTIVIRUS_ACTION;
							m_query.Initialize();
							BINDPARAM_LONG( m_query, nID );
							m_query.Execute( L"UPDATE ServerParameters SET DataValue='1' WHERE ServerParameterID=?" );
						}
					}
				}
				else
				{
					// antivirus is not installed, disable anti-virus
					// Virus Scanning is installed.
					DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - AV not installed, disabling scanning", DebugReporter::ENGINE);	
					
					bDisableScanning = true;

					szWarning = L"AntiVirus scanning has been disabled because the AntiVirus "
								L"Plug-In is not installed.";

					Alert( EMS_ALERT_EVENT_PROD_REGISTRATION, 0, szWarning );

					Log( E_RegistrationWarning, (wchar_t*) szWarning );
				}


				if( bDisableScanning )
				{
					int nID = EMS_SRVPARAM_ANTIVIRUS_ENABLE;
					m_query.Initialize();
					BINDPARAM_LONG( m_query, nID );
					m_query.Execute( L"UPDATE ServerParameters SET DataValue='0' WHERE ServerParameterID=?" );
				}
			}
			catch(...)
			{
			}					
		}
		else
		{
			// Virus Scanning was not enabled.
			//DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - AV not enabled.", DebugReporter::ENGINE);	
		}

		DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Checking user size", DebugReporter::ENGINE);	

		// Enforce user count
		nAllowedAgents = g_Object.m_LicenseMgr.m_MFKeyAgentCount;

		dca::String t;
		t.Format("CWorkerThread::CheckRegistration - Number allowed Agents set to: %d", nAllowedAgents );
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, nAgentsInDB );
		m_query.Execute( L"SELECT COUNT(*) FROM Agents WHERE IsEnabled=1 AND IsDeleted=0 AND AgentID<>1" );
		m_query.Fetch();

		t.Format("CWorkerThread::CheckRegistration - Number Agents enabled in DB: %d", nAgentsInDB );
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

		if ( nAgentsInDB > nAllowedAgents )
		{
			DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Number of enabled Agents exceeds allowed, disabling extra Agents", DebugReporter::ENGINE);	
			
			CEMSString sSQL;

			// Add one to account for Admin account which is ID 1
			nAllowedAgents++;

			sSQL.Format( L"UPDATE Agents SET IsEnabled=0 WHERE AgentID NOT IN "
				         L"(SELECT TOP %d AgentID FROM Agents WHERE IsEnabled=1 "
						 L"AND IsDeleted=0 ORDER BY AgentID)", nAllowedAgents );

			m_query.Initialize();
			m_query.Execute( sSQL.c_str() );

			szWarning = L"More agents were found in the database than your registration allows. "
						L"These additional agents have been disabled.";
			Alert( EMS_ALERT_EVENT_PROD_REGISTRATION, 0, szWarning );
			Log( E_RegistrationWarning, (wchar_t*) szWarning );
		}
		
		DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Checking server size", DebugReporter::ENGINE);	

		nAllowedServers = g_Object.m_LicenseMgr.m_MFKeyServerCount;
		if(nAllowedServers == 0){nAllowedServers = 1;}

		t.Format("CWorkerThread::CheckRegistration - Number allowed Servers set to: %d", nAllowedServers );
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, nServersInDB );
		m_query.Execute( L"SELECT COUNT(DISTINCT st.ServerID) FROM ServerTasks st INNER JOIN Servers s ON st.ServerID=s.ServerID" );
		m_query.Fetch();

		t.Format("CWorkerThread::CheckRegistration - Number Servers in DB: %d", nServersInDB );
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

		if ( nServersInDB > nAllowedServers )
		{
			DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Number of Servers exceeds allowed, disabling extra Servers", DebugReporter::ENGINE);	
			int nMasterServerID = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_MASTER_SERVER ) );
						
			CEMSString sSQL;

			sSQL.Format( L"DELETE FROM ServerTasks WHERE ServerID = "
				         L"(SELECT TOP 1 st.ServerID FROM ServerTasks st INNER JOIN Servers s ON st.ServerID=s.ServerID "
						 L"WHERE s.ServerID <> %d "
						 L"ORDER BY st.ServerID DESC)",nMasterServerID);

			m_query.Initialize();
			m_query.Execute( sSQL.c_str() );

			szWarning = L"More servers were found in the database than your registration allows. "
						L"The Server Tasks for these servers have been disabled.";
			Alert( EMS_ALERT_EVENT_PROD_REGISTRATION, 0, szWarning );
			Log( E_RegistrationWarning, (wchar_t*) szWarning );
		}

		// Send Alerts when expire date is approaching	
		SendRegAlert( L"VisNetic MailFlow", 
			          g_Object.m_LicenseMgr.m_MFKeyInfo.ExpireDay,
			          g_Object.m_LicenseMgr.m_MFKeyInfo.ExpireMonth,
					  g_Object.m_LicenseMgr.m_MFKeyInfo.ExpireYear,
					  g_Object.m_LicenseMgr.m_MFKeyInfo.KeyType,
					  EMS_SRVPARAM_LAST_MAILFLOW_REG_ALERT );

		// If AV Key valid, then check if we need to send a expiration alert
		/*if(    g_Object.m_LicenseMgr.m_AVKeyPresent == true 
			&& g_Object.m_LicenseMgr.m_AVKeyExpired == false )
		{
			SendRegAlert( L"VisNetic AntiVirus for MailFlow", 
						  g_Object.m_LicenseMgr.m_AVKeyInfo.ExpireDay,
						  g_Object.m_LicenseMgr.m_AVKeyInfo.ExpireMonth,
						  g_Object.m_LicenseMgr.m_AVKeyInfo.ExpireYear,
						  g_Object.m_LicenseMgr.m_AVKeyInfo.KeyType,
						  EMS_SRVPARAM_LAST_ANTIVIRUS_REG_ALERT );
		}*/
	
		m_dwLastRegCheck = dwTicks;
	}

	//DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckRegistration - Leaving registration check.", DebugReporter::ENGINE);	

	return true;
}


////////////////////////////////////////////////////////////////////////////////
// 
// CheckEngine
// 
////////////////////////////////////////////////////////////////////////////////
bool CWorkerThread::CheckEngine(void)
{
	dca::String t;	
	long TickCount = GetTickCount();
	
	if(m_LastDBCheckIn == 0){m_LastDBCheckIn = TickCount;}
    long lDiff = TickCount - m_LastDBCheckIn;
	
	if( lDiff > 60000 || lDiff < 0 )
	{
		//Get the last time the Engine checked in
		UINT nServerID = 0;
		GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_SERVER_ID_VALUE, nServerID );
		if(nServerID > 0)
		{
			m_LastDBCheckIn = TickCount;

			TIMESTAMP_STRUCT Now;
			GetTimeStamp( Now );

			long nDiff = 0;
			m_query.Initialize();
			BINDCOL_LONG_NOLEN( m_query, nDiff );
			BINDPARAM_TIME_NOLEN( m_query, Now );
			BINDPARAM_LONG( m_query, nServerID );
			m_query.Execute( L"SELECT DATEDIFF (s, CheckIn, ?) FROM Servers WHERE ServerID = ?" );
			m_query.Fetch();

			t.Format("CWorkerThread::CheckEngine - DBMonitor Thread checked in %d seconds ago", nDiff );
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

			if(nDiff >= 600)
			{
				//DBMonitor thread has not reported in over 10 min, let's restart it
				DebugReporter::Instance().DisplayMessage("CWorkerThread::CheckEngine - Restarting DBMonitor Thread", DebugReporter::ENGINE);
				g_Object.m_ThreadManager.RestartThread(2);
				return true;
			}
			
		}
		else
		{
			m_LastDBCheckIn = TickCount;			
		}
	}
	return false;	
}


void CWorkerThread::SendRegAlert( wchar_t* szProduct, int ExpireDay, int ExpireMonth, int ExpireYear,
								  int KeyType, int SrvParamID )
{
	const wchar_t* pLastAlert = g_Object.GetParameter( SrvParamID );
	SYSTEMTIME lastAlert;
	SYSTEMTIME now;
	SYSTEMTIME regexpire;

	FILETIME ftLastAlert;
	FILETIME ftNow;
	FILETIME ftRegExpire;
	long Day,Month,Year,nDays;

	ZeroMemory( &regexpire, sizeof(regexpire) );
	regexpire.wDay = ExpireDay;
	regexpire.wMonth = ExpireMonth;
	regexpire.wYear = ExpireYear;

	GetLocalTime( &now );
	now.wHour = now.wMinute = now.wSecond = now.wMilliseconds = 0;

	SystemTimeToFileTime( &now, &ftNow );
	SystemTimeToFileTime( &regexpire, &ftRegExpire );

	nDays = (long)(   (*(__int64*)(&ftRegExpire) - *(__int64*)(&ftNow)) 
		            / (__int64(10000000) * __int64(60*60*24)));

	// Are we within 30 days of the registration expiring?
	if( nDays == 30 || nDays == 15 || (nDays > 0 && nDays < 6 ) )
	{
		ZeroMemory( &lastAlert, sizeof(lastAlert) );

		swscanf( pLastAlert, L"%d/%d/%d", &Month, &Day, &Year );

		lastAlert.wDay = (WORD)Day;
		lastAlert.wMonth = (WORD)Month;
		lastAlert.wYear = (WORD)Year;

		SystemTimeToFileTime( &lastAlert, &ftLastAlert );

		// Was the last alert at least a day ago?
		if( (*(__int64*)(&ftNow) - *(__int64*)(&ftLastAlert) >= 
			 __int64( 10000000 ) * __int64( 60*60*24 ) ) )
		{
			CEMSString sMsg;

			if( KeyType == DCIKEY_KeyType_Evaluation )
			{
				sMsg.Format( L"%s evaluation expires in %d day(s)", szProduct, nDays ); 
			}
			else
			{
				sMsg.Format( L"%s registration expires in %d day(s)", szProduct, nDays ); 
			}

			Alert( EMS_ALERT_EVENT_PROD_REGISTRATION, 0, (wchar_t*) sMsg.c_str() );
			Log( E_RegistrationWarning, (wchar_t*) sMsg.c_str() );

			sMsg.Format( L"%d/%d/%d", now.wMonth, now.wDay, now.wYear );

			int nID = SrvParamID;
			m_query.Initialize();
			BINDPARAM_WCHAR( m_query, (wchar_t*) sMsg.c_str() );
			BINDPARAM_LONG( m_query, nID );
			m_query.Execute( L"UPDATE ServerParameters SET DataValue=? WHERE ServerParameterID=?" );

			g_Object.SetParameter( SrvParamID, sMsg.c_str() );
		}	
	}
}
