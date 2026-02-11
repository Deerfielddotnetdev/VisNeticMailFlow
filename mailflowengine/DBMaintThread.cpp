#include "stdafx.h"
#include "DBMaintThread.h"
#include "EMSEvent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DBMaintThread::DBMaintThread() : m_query(m_db)
{
	m_bInitialized = FALSE;
	m_bNeverConnectedToDB = TRUE;
	
	m_pDBMaint = new CDBMaintenance( m_query, m_db );
		
	m_nConnectToDBFailures = 0;	
}

DBMaintThread::~DBMaintThread()
{
	delete m_pDBMaint;	
}

////////////////////////////////////////////////////////////////////////////////
// 
//  Initialize
// 
////////////////////////////////////////////////////////////////////////////////
void DBMaintThread::Initialize(void)
{
	int nRet;
		
	// Make sure our allocations in the constructor were successful:
	if( !m_pDBMaint )
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

	if( m_bInitialized == FALSE )
	{
		DebugReporter::Instance().DisplayMessage("DBMaintThread::Initialize - Calling GetDBParams()", DebugReporter::ENGINE);
		if SUCCEEDED( g_Object.GetDBParams( m_db) )
		{
			DebugReporter::Instance().DisplayMessage("DBMaintThread::Initialize - GetDBParams() succeeded", DebugReporter::ENGINE);
			m_bInitialized = TRUE;
			
			Log( E_DBMaintenanceInfo, L"DB Maintenance Thread Starting\n" );
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("DBMaintThread::Initialize - GetDBParams() failed", DebugReporter::ENGINE);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
//  Uninitialize
// 
////////////////////////////////////////////////////////////////////////////////
void DBMaintThread::Uninitialize()
{
	try
	{	
		Log( E_DBMaintenanceInfo, L"DB Maintenance Thread Shutting Down\n" );
		
		if( m_db.IsConnected() )
		{
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
unsigned int DBMaintThread::Run()
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
						bStuffToDo = DoWork();
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
BOOL DBMaintThread::DoWork(void)
{
	BOOL bRet = FALSE;

	// Any database maintenance to do?
	if( m_pDBMaint->Run() == S_OK )
	{
		DebugReporter::Instance().DisplayMessage("DBMaintThread::DoWork - Database maintenance was run", DebugReporter::ENGINE);
		bRet = TRUE;
	}
	
	return bRet;
}


////////////////////////////////////////////////////////////////////////////////
// 
// HandleMsg
// 
////////////////////////////////////////////////////////////////////////////////
void DBMaintThread::HandleMsg( MSG* msg )
{
	HANDLE hEvent;

	switch( msg->message )
	{
	case WM_DB_MAINTENANCE_NOW:

		// WPARAM is not used and  LPARAM is an event to signal when done
		hEvent = (HANDLE)msg->lParam;	
		try
		{
			m_pDBMaint->RunMaintenance( true );	
		}
		catch( ... )
		{
			SetEvent( hEvent );	
			CloseHandle( hEvent );
			throw;
		}
		SetEvent( hEvent );	
		CloseHandle( hEvent );
		break;

	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// HandleODBCError
// 
////////////////////////////////////////////////////////////////////////////////
void DBMaintThread::HandleODBCError( ODBCError_t* pErr )
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

	Log( E_DBMaintErrorODBC, szDebug );

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
void DBMaintThread::HandleException( wchar_t* wcsDescription )
{
	_Module.m_status.dwWin32ExitCode = ERROR_EXCEPTION_IN_SERVICE;
	_Module.LogEvent( wcsDescription );		
	Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, L"VisNetic MailFlow Service stopping due to unhandled exception: %s", 
		   wcsDescription );
	WaitForSingleObject( m_hKillEvent, 1000 );
	PostThreadMessage( _Module.dwThreadID, WM_QUIT, 0, 0);
	m_bRun = FALSE;
}
