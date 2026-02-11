// VirusScanningThread.cpp: implementation of the CVirusScanningThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VirusScanningThread.h"

// Static variables
dca::String CVirusScanningThread::m_sVersion;
dca::String CVirusScanningThread::m_sRecordCount;
dca::String CVirusScanningThread::m_sLastUpdate;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVirusScanningThread::CVirusScanningThread()
{
	m_bInitialized = FALSE;
}

CVirusScanningThread::~CVirusScanningThread()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
// Log - post a log message to the worker thread
// 
////////////////////////////////////////////////////////////////////////////////
void CVirusScanningThread::Log( int Code, LPCWSTR szMsg )
{
	wchar_t* wcsText = (wchar_t*) HeapAlloc ( GetProcessHeap (), 0, 
					                          sizeof(wchar_t) * (wcslen(szMsg)+1) );

	wcscpy( wcsText, szMsg );
	
	g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::Worker, WM_WRITE_LOG, 
												Code, (LPARAM)wcsText );
}

////////////////////////////////////////////////////////////////////////////////
// 
// Initialize
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CVirusScanningThread::Initialize(void)
{
	try
	{
		if( m_bInitialized == FALSE )
		{
			if( m_scanner.GetVersionInfo( m_sVersion, m_sLastUpdate, m_sRecordCount ) == 0 )
			{
				dca::String sLogMsg;
				m_bInitialized = TRUE;
				
				sLogMsg.Format( "VisNetic Anti-Virus version %s initialized\n", m_sVersion.c_str() );
				wchar_t wcsMsg[255];
				mbstowcs(wcsMsg,sLogMsg.c_str(),255);
								
				Log( E_VirusScanningInfo,  wcsMsg);
			}
		}
#ifdef _DEBUG
		else
		{
			m_bInitialized = TRUE;
		}
#endif
	}
	catch( ... )
	{	
		m_bInitialized = FALSE;
	}

	return m_bInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// 
// UnInitialize
// 
////////////////////////////////////////////////////////////////////////////////
void CVirusScanningThread::UnInitialize(void)
{
	if( m_bInitialized == TRUE )
	{
		try
		{
			m_scanner.UnInitialize();
			m_bInitialized = FALSE;
		}
		catch( ... )
		{
		}
	}
	#ifdef _DEBUG
		else
		{
		}
#endif
}



////////////////////////////////////////////////////////////////////////////////
// 
//  Run
// 
////////////////////////////////////////////////////////////////////////////////
unsigned int CVirusScanningThread::Run()
{
	DWORD dwRet;
	MSG msg;
	BOOL bRun = TRUE;
	
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Create the message queue
	PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );
	SetEvent( m_hReadyEvent );


	Initialize();
	
	// This is the normal loop
	while ( bRun )
	{
		dwRet = MsgWaitForMultipleObjects( 1, &m_hStopEvent, FALSE, INFINITE, QS_ALLEVENTS );
		
		switch( dwRet )
		{
		case WAIT_TIMEOUT:
			{
			}
			break;
			
		case WAIT_OBJECT_0 + 1:		// Message in queue
			while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{		
				try
				{
					if( msg.message == WM_VIRUS_SCAN )
					{
						if( Initialize() == TRUE )
						{
							Scan( (VirusScanInfo*) msg.wParam );
						}				
					}
					else if( msg.message == WM_RELOAD_CONFIG )
					{
						UnInitialize();
						Initialize();
					}
					else
					{
					}
				}
				catch(...)
				{
				}
			}
			break;
			
		case WAIT_OBJECT_0:			// Stop Event is set.
			{
				bRun = FALSE;
			}
			break;
		}
	}

	bRun = TRUE;

	// This loop flushes the queue
	while ( bRun )
	{
		dwRet = MsgWaitForMultipleObjects( 1, &m_hKillEvent, FALSE, INFINITE, QS_ALLEVENTS );
		
		switch( dwRet )
		{
		case WAIT_TIMEOUT:
			{
			}
			break;
			
		case WAIT_OBJECT_0 + 1:		// Message in queue
			while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				try
				{
					if( msg.message == WM_VIRUS_SCAN )
					{
						((VirusScanInfo*) msg.wParam)->nRet = CKAVScan::Err_Unexpected_Failure;
						SetEvent( ((VirusScanInfo*) msg.wParam)->hEvent );
					}
				}
				catch(...)
				{
				}
			}
			break;
			
		case WAIT_OBJECT_0:			// Kill Event is set.
			{
				bRun = FALSE;
			}
			break;
		}
	}

	UnInitialize();

	CoUninitialize();
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Scan
// 
////////////////////////////////////////////////////////////////////////////////
void CVirusScanningThread::Scan( VirusScanInfo* pVSInfo )
{
	try
	{
		int nAction = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_ANTIVIRUS_ACTION ) );

		if( pVSInfo->bFile )
		{
			int nUnscannableAction = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_ANTIVIRUS_UNSCANNABLE_ACTION ) );
			int nSuspiciousAction = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_ANTIVIRUS_SUSPICIOUS_ACTION ) );
			
			// Override the system setting if we are doing a repair-only
			if( pVSInfo->nOptions == 1 )
			{
				nAction = CKAVScan::RepairOnly;
			}
			
			pVSInfo->nRet = m_scanner.ScanFile( pVSInfo->szFile, nAction, nSuspiciousAction, nUnscannableAction );
		}
		else
		{
			pVSInfo->nRet = m_scanner.ScanString( pVSInfo->szString, nAction );
		}

		strncpy( pVSInfo->szVirusName, m_scanner.GetVirusName(), MAX_PATH );
		strncpy( pVSInfo->szQuarantineLoc, m_scanner.GetQuarantineLoc().c_str(), MAX_PATH );
	}
	catch( ... )	// If we get an unhandled exception, attempt to re-initialize
	{
		pVSInfo->nRet = CKAVScan::Err_Unexpected_Failure;

		UnInitialize();

		Log( E_VirusScanningError, _T("Unhandled exception while virus scanning") );
	}

	SetEvent( pVSInfo->hEvent );
}
