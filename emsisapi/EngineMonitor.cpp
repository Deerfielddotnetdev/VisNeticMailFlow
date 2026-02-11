// EngineMonitor.cpp: implementation of the CEngineMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EngineMonitor.h"
#include "RegistryFns.h"
#include <iostream>
#include <windows.h>
#include <winsvc.h>
#include <string>
#include<tchar.h>
#include <process.h>
#include <tlhelp32.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEngineMonitor::CEngineMonitor(CSharedObjects* pShared)
  :  CThread( pShared ),m_query(m_db)
{
	m_bDBMaintenanceRunning = false;
	m_nLastRun = 60;	
}

CEngineMonitor::~CEngineMonitor()
{

}

unsigned int CEngineMonitor::Run()
{
	m_RoutingEngine.LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_SystemStarting),
					_T("Engine Monitor Thread [%d] Loading"), GetCurrentThreadId() );

	bool bRun = true;
	DWORD dwRet;
	int nRet = 0;	

	HANDLE hEvents[2] = { m_hKillEvent, m_hReloadEvent };

	while ( bRun )
	{
		// Check if DB Maintenance is running.
		m_bDBMaintenanceRunning = m_DBMaintenanceMutex.IsLocked();
		
		// Could wait for other events here as well...
		dwRet = WaitForMultipleObjects( 2, hEvents, FALSE, 1000 );

		switch( dwRet )
		{
		case WAIT_TIMEOUT:
			m_nLastRun++;
			
			if( m_bDBMaintenanceRunning == false )
			{
				if( m_nLastRun > 60 )
				{
					m_nLastRun = 0;
					
					nRet = CheckEngine();
				}
			}
			break;

		case WAIT_OBJECT_0:
			m_RoutingEngine.LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_SystemStopping),
					_T("Engine Monitor Thread [%d] received Stop Signal"), GetCurrentThreadId() );
			
			bRun = false;
			break;

		case WAIT_OBJECT_0 + 1:
			
			ResetEvent( m_hReloadEvent );
			break;
		}
	}

	m_RoutingEngine.LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_SystemStopping),
					_T("Engine Monitor Thread [%d] Unloading"), GetCurrentThreadId() );

	return 0;
}

int CEngineMonitor::CheckEngine(void)
{
	UINT nRestartEngine = 0;
	
	GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,  _T("MailCheckRestartEngine"), nRestartEngine);

	if(nRestartEngine == 1)
	{
		m_RoutingEngine.LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
														EMSERR_ISAPI_EXTENSION,
														EMS_LOG_DBMONITOR,
														E_SystemStopping),
												  "Engine Monitor has received a restart request from the VisNetic MailFlow Engine Service" );

		DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor has received a restart request from the VisNetic MailFlow Engine Service", DebugReporter::ISAPI);
		
		std::string Service = "VisNetic MailFlow Engine";
		SERVICE_STATUS Status;
	    
		SC_HANDLE SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		SC_HANDLE SHandle = OpenService(SCManager, Service.c_str(), SC_MANAGER_ALL_ACCESS);
	    
		if(SHandle == NULL)
		{
			// log failure to open Service
			m_RoutingEngine.LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
														EMSERR_ISAPI_EXTENSION,
														EMS_LOG_DBMONITOR,
														E_SystemStopping),
												  "Engine Monitor failed to open VisNetic MailFlow Engine Service" );


			DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor failed to open VisNetic MailFlow Engine Service", DebugReporter::ISAPI);

			CloseServiceHandle(SCManager);
			
			return 1;
		}
	    
		QueryServiceStatus(SHandle, &Status);
		if ( Status.dwCurrentState != SERVICE_RUNNING )
		{
			// service is not running, is it already stopped?
			if ( Status.dwCurrentState == SERVICE_STOPPED )
			{
				// service is already stopped, goto start
				m_RoutingEngine.LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
														EMSERR_ISAPI_EXTENSION,
														EMS_LOG_DBMONITOR,
														E_SystemStopping),
												  "Engine Monitor detected VisNetic MailFlow Engine Service already stopped" );

				DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor detected VisNetic MailFlow Engine Service already stopped", DebugReporter::ISAPI);
			}
			else if ( Status.dwCurrentState == SERVICE_STOP_PENDING )
			{
				// service is pending stop, goto kill
				m_RoutingEngine.LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
														EMSERR_ISAPI_EXTENSION,
														EMS_LOG_DBMONITOR,
														E_SystemStopping),
												  "Engine Monitor detected VisNetic MailFlow Engine Service is pending stop" );

				DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor detected VisNetic MailFlow Engine Service is pending Stop", DebugReporter::ISAPI);
			}
			else if ( Status.dwCurrentState == SERVICE_START_PENDING )
			{
				// service is pending start, goto kill
				m_RoutingEngine.LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
														EMSERR_ISAPI_EXTENSION,
														EMS_LOG_DBMONITOR,
														E_SystemStopping),
												  "Engine Monitor detected VisNetic MailFlow Engine Service is pending start" );

				DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor detected VisNetic MailFlow Engine Service is pending Start", DebugReporter::ISAPI);
			}
			else
			{
				// service in unknown state, goto kill
				m_RoutingEngine.LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
														EMSERR_ISAPI_EXTENSION,
														EMS_LOG_DBMONITOR,
														E_SystemStopping),
												  "Engine Monitor detected VisNetic MailFlow Engine Service in unknown state" );

				DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor detected VisNetic MailFlow Engine Service in unknown state", DebugReporter::ISAPI);
			}
		}

		// if service is running, let's send the stop command
		if ( Status.dwCurrentState == SERVICE_RUNNING )
		{
			if(!ControlService(SHandle, SERVICE_CONTROL_STOP, &Status))
			{
				DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor failed to send Stop signal to the VisNetic MailFlow Engine Service", DebugReporter::ISAPI);
			}
			else
			{
				DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor successfully sent Stop signal to the VisNetic MailFlow Engine Service", DebugReporter::ISAPI);
			}
		}

		int nCount = 0;
		do
		{
			Sleep( 1000 );
			++nCount;			
			QueryServiceStatus(SHandle, &Status);
			//std::cout << "Checking Service Status...\n";
			
		}
		while(Status.dwCurrentState != SERVICE_STOPPED && nCount < 60);
	    
		if ( Status.dwCurrentState != SERVICE_STOPPED )
		{
			// failed to stop the service, let's kill the process		
			DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor failed to stop the VisNetic MailFlow Engine Service", DebugReporter::ISAPI);

			//kill the process

			if (!GetProcessList())
			{
				// failed to stop the service, let's kill the process		
				DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor failed to kill the mailflowengine.exe process", DebugReporter::ISAPI);
				CloseServiceHandle(SCManager);
				CloseServiceHandle(SHandle);
				return 1;
			}
		}
	    		
		if(!StartService(SHandle, 0, NULL))
		{
			DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor failed to Start the VisNetic MailFlow Engine Service", DebugReporter::ISAPI);
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CEngineMonitor::CheckEngine - Engine Monitor successfully started the VisNetic MailFlow Engine Service", DebugReporter::ISAPI);
			WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("MailCheckRestartEngine"), 0 );
		}
	    	    
		CloseServiceHandle(SCManager);
		CloseServiceHandle(SHandle);
	}

	return 0;
}

BOOL CEngineMonitor::GetProcessList( void )
{
  HANDLE hProcessSnap;
  PROCESSENTRY32 pe32;  

  // Take a snapshot of all processes in the system.
  hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  if( hProcessSnap == INVALID_HANDLE_VALUE )
  {   
    return( FALSE );
  }

  // Set the size of the structure before using it.
  pe32.dwSize = sizeof( PROCESSENTRY32 );

  // Retrieve information about the first process,
  // and exit if unsuccessful
  if( !Process32First( hProcessSnap, &pe32 ) )
  {   
    CloseHandle( hProcessSnap );  // clean the snapshot object
    return( FALSE );
  }

  // Now walk the snapshot of processes 
  do
  {  
    string str(pe32.szExeFile);

    if(str == "mailflowengine.exe" || str == "mailfl~1.exe") // put the name of your process you want to kill 
    {
        DebugReporter::Instance().DisplayMessage("CEngineMonitor::GetProcessList - Got processid for mailflowengine.exe", DebugReporter::ISAPI);
		TerminateMyProcess(pe32.th32ProcessID, 1);
    } 
  } while( Process32Next( hProcessSnap, &pe32 ) );

  CloseHandle( hProcessSnap );
  return( TRUE );
}

BOOL CEngineMonitor::TerminateMyProcess(DWORD dwProcessId, UINT uExitCode)
{
    DWORD dwDesiredAccess = PROCESS_TERMINATE;
    BOOL  bInheritHandle  = FALSE;
    HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
    if (hProcess == NULL)
        return FALSE;

    BOOL result = TerminateProcess(hProcess, uExitCode);
	
	if ( result == 0)
	{
		DebugReporter::Instance().DisplayMessage("CEngineMonitor::TerminateMyProcess - TerminateProcess() for mailflowengine.exe failed", DebugReporter::ISAPI);
	}
	else
	{
		DebugReporter::Instance().DisplayMessage("CEngineMonitor::TerminateMyProcess - TerminateProcess() for mailflowengine.exe succeeded", DebugReporter::ISAPI);
	}

    CloseHandle(hProcess);

    return result;
}