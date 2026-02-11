////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/ThreadManager.cpp,v 1.2.2.1.2.1 2006/07/18 12:50:09 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// ThreadManager.cpp: implementation of the CThreadManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ThreadManager.h"
#include "WorkerThread.h"
#include "VirusScanningThread.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CThreadManager::CThreadManager()
{
	m_hKillEvent = INVALID_HANDLE_VALUE;
	m_nThreadsStarted = 0;
}

CThreadManager::~CThreadManager()
{

}

int CThreadManager::StartAllThreads()
{
	int i;

	// Create the global kill event
	m_hKillEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hStopEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	try
	{
		// Create the worker thread
		m_pThreads[Worker] = new CWorkerThread;
		m_nThreadsStarted = 1;
		DebugReporter::Instance().DisplayMessage("CThreadManager::StartAllThreads - WorkerThread created", DebugReporter::ENGINE);
	}
	catch(...)
	{
		m_pThreads[Worker] = NULL;
	}	

	try
	{
		// Create the tickler thread
		m_pThreads[Tickler] = new TicklerSystemThread;
		m_nThreadsStarted = 2;
		DebugReporter::Instance().DisplayMessage("CThreadManager::StartAllThreads - TicklerThread created", DebugReporter::ENGINE);
	}
	catch(...)
	{
		m_pThreads[Tickler] = NULL;
	}
	
	try
	{
		// Create the dbmonitor thread
		m_pThreads[DBMonitor] = new DBMonitorThread;
		m_nThreadsStarted = 3;
		DebugReporter::Instance().DisplayMessage("CThreadManager::StartAllThreads - DBMonitorThread created", DebugReporter::ENGINE);
	}
	catch(...)
	{
		m_pThreads[DBMonitor] = NULL;
	}

	try
	{
		// Create the web sessions thread
		m_pThreads[WebSession] = new WebSessionThread;
		m_nThreadsStarted = 4;
		DebugReporter::Instance().DisplayMessage("CThreadManager::StartAllThreads - WebSessionThread created", DebugReporter::ENGINE);
	}
	catch(...)
	{
		m_pThreads[WebSession] = NULL;
	}

	try
	{
		// Create the DB Maintenance thread
		m_pThreads[DBMaint] = new DBMaintThread;
		m_nThreadsStarted = 5;
		DebugReporter::Instance().DisplayMessage("CThreadManager::StartAllThreads - DBMaintThread created", DebugReporter::ENGINE);
	}
	catch(...)
	{
		m_pThreads[DBMaint] = NULL;
	}

	//try
	//{
	//	// Create the av thread
	//	m_pThreads[VirusScanner] = new CVirusScanningThread;
	//	m_nThreadsStarted = 5;
	//	DebugReporter::Instance().DisplayMessage("CThreadManager::StartAllThreads - VirusScanningThread created", DebugReporter::ENGINE);
	//}
	//catch(...)
	//{
	//	m_pThreads[VirusScanner] = NULL;
	//}
	
	for ( i = 0; i < ThreadCount; i++ )
	{
		if(m_pThreads[i])
		{
			// Set the kill event of all threads
			m_pThreads[i]->SetKillEvent( m_hKillEvent );

			// Set the kill event of all threads
			m_pThreads[i]->SetStopEvent( m_hStopEvent );

			// Start the thread
			m_pThreads[i]->StartThread();
		}
	}	

	DebugReporter::Instance().DisplayMessage("CThreadManager::StartAllThreads - Wait for the threads to create a message queue", DebugReporter::ENGINE);
	// Wait for the threads to create a message queue
	for ( i = 0; i < ThreadCount; i++ )
	{
		if(m_pThreads[i])
		{
			if(i==2)
			{
				WaitForSingleObject( m_pThreads[i]->m_hReadyEvent, 6000 );
			}
			else
			{
				WaitForSingleObject( m_pThreads[i]->m_hReadyEvent, 1000 );
			}			
		}
	}

	return 0;
}

int CThreadManager::RestartThread(int nThread)
{
	HANDLE threadHandle;
	bool bThreadAlive;
	DWORD dwRet;

	try
	{
		
		bThreadAlive = true;
		threadHandle = m_pThreads[nThread]->GetHandle();
		
		dca::String t;
		t.Format("CThreadManager::RestartThread - Shutting down thread %d", nThread );
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

		TerminateThread(threadHandle, 0);

		while( bThreadAlive )
		{
			dwRet = MsgWaitForMultipleObjects( 1, &threadHandle, FALSE, 30000, QS_ALLEVENTS );

			switch( dwRet )
			{
			case WAIT_TIMEOUT:
				bThreadAlive = false;
				break;

			case WAIT_OBJECT_0 + 1:		
				MSG msg;
				while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
				{	
					DispatchMessage(&msg);
				}
				break;

			case WAIT_OBJECT_0:			
				bThreadAlive = false;
				break;
			}		
		}
		
		t.Format("CThreadManager::RestartThread - Successfully shut down thread %d", nThread );
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);				
	}
	catch(...)
	{
		
	}
	
	switch( nThread )
	{
	case 0:
		try
		{
			// Create the worker thread
			m_pThreads[Worker] = new CWorkerThread;
			DebugReporter::Instance().DisplayMessage("CThreadManager::StartAllThreads - WorkerThread created", DebugReporter::ENGINE);
		}
		catch(...)
		{
			m_pThreads[Worker] = NULL;
		}	
		break;

	case 1:		
		try
		{
			// Create the tickler thread
			m_pThreads[Tickler] = new TicklerSystemThread;
			DebugReporter::Instance().DisplayMessage("CThreadManager::StartAllThreads - TicklerThread created", DebugReporter::ENGINE);
		}
		catch(...)
		{
			m_pThreads[Tickler] = NULL;
		}
		break;

	case 2:			
		try
		{
			// Create the dbmonitor thread
			m_pThreads[DBMonitor] = new DBMonitorThread;
			DebugReporter::Instance().DisplayMessage("CThreadManager::RestartThread - DBMonitorThread created", DebugReporter::ENGINE);
		}
		catch(...)
		{
			m_pThreads[DBMonitor] = NULL;
		}
		break;
	case 3:			
		try
		{
			// Create the web sessions thread
			m_pThreads[WebSession] = new WebSessionThread;
			DebugReporter::Instance().DisplayMessage("CThreadManager::RestartThread - WebSessionThread created", DebugReporter::ENGINE);
		}
		catch(...)
		{
			m_pThreads[WebSession] = NULL;
		}
		break;
	case 4:			
		try
		{
			// Create the DB Maint thread
			m_pThreads[DBMaint] = new DBMaintThread;
			DebugReporter::Instance().DisplayMessage("CThreadManager::RestartThread - DbMaintThread created", DebugReporter::ENGINE);
		}
		catch(...)
		{
			m_pThreads[DBMaint] = NULL;
		}
		break;
	}	
	

	if(m_pThreads[nThread])
	{
		// Set the kill event of the thread
		m_pThreads[nThread]->SetKillEvent( m_hKillEvent );

		// Set the stop event of the thread
		m_pThreads[nThread]->SetStopEvent( m_hStopEvent );

		// Start the thread
		m_pThreads[nThread]->StartThread();
	}

	DebugReporter::Instance().DisplayMessage("CThreadManager::RestartThread - Wait for the thread to create a message queue", DebugReporter::ENGINE);
	// Wait for the thread to create a message queue
	if(m_pThreads[nThread])
	{
		if(nThread==2)
		{
			WaitForSingleObject( m_pThreads[nThread]->m_hReadyEvent, 6000 );
		}
		else
		{
			WaitForSingleObject( m_pThreads[nThread]->m_hReadyEvent, 1000 );
		}			
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetStopEvent
// 
////////////////////////////////////////////////////////////////////////////////
void CThreadManager::SetStopEvent(void)
{
	// Set the global stop event
	if( m_hStopEvent != INVALID_HANDLE_VALUE )
	{
		SetEvent( m_hStopEvent );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// StopAllThreads
// 
////////////////////////////////////////////////////////////////////////////////
int CThreadManager::StopAllThreads()
{
	HANDLE threadHandle;
	bool bThreadAlive;
	DWORD dwRet;
	int i;	

	DebugReporter::Instance().DisplayMessage("CThreadManager::StopAllThreads - Entered", DebugReporter::ENGINE);
	
	// Set the global kill event
	if( m_hKillEvent != INVALID_HANDLE_VALUE )
	{
		SetEvent( m_hKillEvent );
	}

	// Wait for all threads to exit
	// -- May want to do a single WaitForMultipleObjects
	// -- and check for threads hanging
	//
	
	try
	{
		for ( i = 0; i < m_nThreadsStarted; i++ )
		{
			bThreadAlive = true;
			threadHandle = m_pThreads[i]->GetHandle();
			
			dca::String t;
			t.Format("CThreadManager::StopAllThreads - Shutting down thread %d", i );
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

			while( bThreadAlive )
			{
				dwRet = MsgWaitForMultipleObjects( 1, &threadHandle, FALSE, 30000, QS_ALLEVENTS );

				switch( dwRet )
				{
				case WAIT_TIMEOUT:
					bThreadAlive = false;
					break;

				case WAIT_OBJECT_0 + 1:		// Message in queue
					MSG msg;
					while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
					{	
						DispatchMessage(&msg);
					}
					break;

				case WAIT_OBJECT_0:			// Kill Event is set.
					bThreadAlive = false;
					break;
				}		
			}
			
			t.Format("CThreadManager::StopAllThreads - Successfully shut down thread %d", i );
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		}

		// delete the threads
		for ( i = 0; i < m_nThreadsStarted; i++ )
		{
			delete m_pThreads[i];
			dca::String t;
			t.Format("CThreadManager::StopAllThreads - Deleted thread %d", i );
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

		}

		if( m_hKillEvent != INVALID_HANDLE_VALUE )
		{
			// delete the global kill event and exit
			CloseHandle( m_hKillEvent );
			DebugReporter::Instance().DisplayMessage("CThreadManager::StopAllThreads - Closed kill event handle", DebugReporter::ENGINE);
		}

		if( m_hStopEvent != INVALID_HANDLE_VALUE )
		{
			// delete the global kill event and exit
			CloseHandle( m_hStopEvent );
			DebugReporter::Instance().DisplayMessage("CThreadManager::StopAllThreads - Closed stop event handle", DebugReporter::ENGINE);
		}

		m_hStopEvent = INVALID_HANDLE_VALUE;
		m_hKillEvent = INVALID_HANDLE_VALUE;
		m_nThreadsStarted = 0;
	}
	catch(...)
	{
		
	}

	return 0;
}

int CThreadManager::PostThreadMessage( int nThread, UINT Msg, 
									   WPARAM wParam, LPARAM lParam )
{
	// sanity check
	if( nThread < 0 || nThread >= m_nThreadsStarted )
		return -1;
	
	return ::PostThreadMessage( m_pThreads[nThread]->GetID(), 
		                        Msg, wParam, lParam );
}

DWORD CThreadManager::GetThreadID( int nThread )
{
	if( nThread < 0 || nThread >= m_nThreadsStarted )
		return 0;

	return m_pThreads[nThread]->GetID();
}
