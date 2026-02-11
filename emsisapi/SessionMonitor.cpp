// SessionMonitor.cpp: implementation of the CSessionMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SessionMonitor.h"
#include "uploadmap.h"
#include "EMSEvent.h"
#include "RegistryFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSessionMonitor::CSessionMonitor( CSharedObjects* pShared, list<CThread*>& ThreadList )
  : CThread( pShared ), m_ThreadList(ThreadList),  m_query(m_db)
{
	m_bDBMaintenanceRunning = false;
	m_nLastRun = 60;
	m_nLastSync = 0;
	m_nLastStatus = 0;
	m_nLastDB = 0;
	m_nLastCache = 0;
}

CSessionMonitor::~CSessionMonitor()
{

}

unsigned int CSessionMonitor::Run()
{
	m_RoutingEngine.LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_SystemStarting),
					_T("Session Monitor Thread [%d] Loading"), GetCurrentThreadId() );

	DWORD dwRet;
	bool bRun = true;
	bool bWasRunning = false;
	
	int nCacheRefresh = m_pShared->SessionMap().GetCacheRefresh()*60;
	int nServerID = m_pShared->SessionMap().GetServerID();
	int nQueueSize = m_pShared->SessionMap().GetQueueSize();
	
	m_pShared->Queue().SetQueueSize(nQueueSize);
	dca::String o;
	o.Format("CSessionMonitor::Run - Setting Queue Size to [%d].", nQueueSize);
	DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	o.Format("CSessionMonitor::Run - Setting Cache Refresh to [%d].", nCacheRefresh);
	DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	int nCharSet = m_pShared->SessionMap().GetCharSet();
	m_pShared->m_nCharSet = nCharSet;
	
	o.Format("CSessionMonitor::Run - Setting CharSet to [%d].", nCharSet);
	DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	int nTimeZone = m_pShared->SessionMap().GetTimeZone();
	m_pShared->m_nTimeZone = nTimeZone;
	
	o.Format("CSessionMonitor::Run - Setting TimeZone to [%d].", nTimeZone);
	DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	bool bFullText = m_pShared->SessionMap().GetFullText();
	m_pShared->m_bFullText = bFullText;

	if(bFullText)
	{
		DebugReporter::Instance().DisplayMessage("CSessionMonitor::Run - Full text index search enabled", DebugReporter::ISAPI, GetCurrentThreadId());
	}
	
	CheckInToDB( nServerID, true );

	HANDLE hEvents[2] = { m_hKillEvent, m_hReloadEvent };

	while ( bRun )
	{
		// Check if DB Maintenance is running.
		m_bDBMaintenanceRunning = m_DBMaintenanceMutex.IsLocked();
		
		// Update the global on state change
		if( m_bDBMaintenanceRunning != bWasRunning )
		{
			m_pShared->m_bDBMaintenanceRunning = bWasRunning = m_bDBMaintenanceRunning;
			//m_pShared->SessionMap().QueueSessionMonitors(4,m_bDBMaintenanceRunning ? 1:0);
		}

		// Could wait for other events here as well...
		dwRet = WaitForMultipleObjects( 2, hEvents, FALSE, 1000 );

		switch( dwRet )
		{
		case WAIT_TIMEOUT:
			m_nLastRun++;
			m_nLastSync++;
			m_nLastStatus++;
			m_nLastDB++;
			if(nCacheRefresh > 0)
			{
				m_nLastCache++;
			}

			if( m_nLastSync > m_pShared->SessionMap().GetSessionFreq() )
			{
				m_nLastSync = 0;
				CheckMonitorQueue();
			}
				
			if( m_bDBMaintenanceRunning == false )
			{
				if( m_nLastCache > nCacheRefresh && nCacheRefresh > 0 )
				{
					m_nLastCache = 0;
					o.Format("CSessionMonitor::Run - Reloading Cache Maps per Cache Refresh setting: [%d].", nCacheRefresh);
					DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					m_pShared->XMLCache().InvalidateAllMaps();
					m_pShared->XMLCache().BuildAllMaps(m_query);
			
				}
				if( m_nLastStatus > m_pShared->SessionMap().GetStatusFreq() )
				{
					m_nLastStatus = 0;
					m_pShared->SessionMap().LoadStatus();
				}
				if( m_nLastDB > m_pShared->SessionMap().GetDBFreq() )
				{
					m_nLastDB = 0;
					if(m_pShared->SessionMap().GetDBSessionsEnabled() == 1)
					{
						m_pShared->SessionMap().SyncSessionsWithDB();
					}
				}
				if( m_nLastRun > 60 )
				{
					m_nLastRun = 0;
					
					int nSessionTimeout = m_pShared->SessionMap().GetSessionTimeOut();
					
					MaintainThreadCount();					
					
					m_pShared->SessionMap().RemoveOldSessions( nSessionTimeout );

					CUploadMap::GetInstance().RemoveOldItems();
					CheckInToDB( nServerID, true );				

					if(m_pShared->SessionMap().GetDBSessionsEnabled() == 0)
					{
						m_pShared->SessionMap().SaveSessions();
					}

					m_pShared->SessionMap().LoadSessionMonitors();
				}
			}
			break;

		case WAIT_OBJECT_0:
			m_RoutingEngine.LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_SystemStopping),
					_T("Session Monitor Thread [%d] received Stop Signal"), GetCurrentThreadId() );

			if(m_pShared->SessionMap().GetDBSessionsEnabled() == 0)
			{
				m_pShared->SessionMap().SaveSessions();
			}
			bRun = false;
			break;

		case WAIT_OBJECT_0 + 1:
			// Reload event			
			m_pShared->XMLCache().InvalidateAllMaps();
			m_pShared->XMLCache().BuildAllMaps(m_query);
			m_pShared->SessionMap().QueueSessionMonitors(3,0);
			nQueueSize = m_pShared->SessionMap().GetQueueSize();	
			m_pShared->Queue().SetQueueSize(nQueueSize);
			o.Format("CSessionMonitor::Run - Reload Event setting Queue Size to %d.", nQueueSize);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			nCharSet = m_pShared->SessionMap().GetCharSet();
			m_pShared->m_nCharSet = nCharSet;			
			o.Format("CSessionMonitor::Run - Reload Event setting CharSet to %d.", nCharSet);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			nTimeZone = m_pShared->SessionMap().GetTimeZone();
			m_pShared->m_nTimeZone = nTimeZone;			
			o.Format("CSessionMonitor::Run - Reload Event setting TimeZone to %d.", nTimeZone);
			bFullText = m_pShared->SessionMap().GetFullText();
			m_pShared->m_bFullText = bFullText;
			if(bFullText)
			{
				DebugReporter::Instance().DisplayMessage("CSessionMonitor::Run - Reload Event full text index search enabled", DebugReporter::ISAPI, GetCurrentThreadId());
			}
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			ResetEvent( m_hReloadEvent );
			break;
		}
	}

	m_RoutingEngine.LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_SystemStopping),
					_T("Session Monitor Thread [%d] Unloading"), GetCurrentThreadId() );

	CheckInToDB( nServerID, false );
	return 0;
}

void CSessionMonitor::MaintainThreadCount(void)
{
	XServerParameters srvparam;
	UINT nThreads;
	CThread* pThread;	

	if( m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_ISAPI_THREADS, srvparam ) )
	{
		nThreads = _ttoi( srvparam.m_DataValue );

		// sanity-check the value
		if( nThreads < 5 )
		{
			nThreads = 5;
		}
		else if( nThreads > 128 )
		{
			nThreads = 128;
		}

		// Adjust the thread pool up
		while( m_ThreadList.size() < nThreads )
		{
			pThread = new CWorkerThread( m_pShared );

			// start the thread with its own kill object
			pThread->SetKillEvent( INVALID_HANDLE_VALUE );
			pThread->StartThread();

			m_ThreadList.push_back( pThread );	
		}

		// Adjust the thread pool down
		while ( m_ThreadList.size() > nThreads )
		{
			pThread = m_ThreadList.front();
			
			m_ThreadList.pop_front();

			SetEvent( pThread->GetKillEvent() );

			if( WAIT_TIMEOUT == WaitForSingleObject( pThread->GetHandle(), 5000 ) )
			{
				TerminateThread( pThread->GetHandle(), -1 );
			}
			delete pThread;
		}
	}
}

void CSessionMonitor::CheckMonitorQueue( void )
{
	vQueue.clear();

	try
	{
		if(m_pShared->SessionMap().GetWebSessionsEnabled() == 1)
		{
			int nServerID = m_pShared->SessionMap().GetServerID();
			int nProcessID = m_pShared->SessionMap().GetProcessID();
								
			m_query.Initialize();
			BINDPARAM_LONG( m_query, nServerID );
			BINDPARAM_LONG( m_query, nProcessID );
			BINDCOL_LONG_NOLEN( m_query, m_Queue.nQueueID );
			BINDCOL_LONG_NOLEN( m_query, m_Queue.nActionID );
			BINDCOL_LONG_NOLEN( m_query, m_Queue.nTargetID );
			m_query.Execute( _T("SELECT QueueID,ActionID,TargetID FROM SessionMonitorQueue WHERE ServerID=? AND ProcessID=? ORDER BY QueueID"));
			int numRows =0;
			while( m_query.Fetch() == S_OK )
			{
				numRows++;
				vQueue.push_back(m_Queue);
			}
			if(numRows > 0)
			{
				m_Queue.nQueueID++;
				m_query.Reset(true);
				BINDPARAM_LONG( m_query, nProcessID );
				BINDPARAM_LONG( m_query, m_Queue.nQueueID );
				m_query.Execute( _T("DELETE FROM SessionMonitorQueue WHERE ProcessID = ? AND QueueID < ?"));
			}

			for ( vIter = vQueue.begin(); vIter != vQueue.end(); vIter++ )
			{
				switch(vIter->nActionID)			
				{
				case 1:
					if(vIter->nTargetID==0)
					{
						m_pShared->SessionMap().InvalidateAllAgentSessions();
					}
					else
					{
						m_pShared->SessionMap().InvalidateAgentSession(vIter->nTargetID);
					}				
					break;
				case 2:
					m_RoutingEngine.ReloadConfig( EMS_ServerParameters );
					g_ThreadPool.ReLoadAllThreads();
					break;
				case 3:
					m_pShared->XMLCache().SetDirty(true);						
					switch(vIter->nTargetID)
					{
					case 0:
						m_pShared->XMLCache().InvalidateAllMaps();
						break;
					case 1:
						m_pShared->XMLCache().m_AgentEmails.Invalidate();
						m_pShared->XMLCache().m_AgentNames.Invalidate();
						m_pShared->XMLCache().m_AgentNamesWOAdmin.Invalidate();
						m_pShared->XMLCache().m_AgentIDs.Invalidate();
						m_pShared->XMLCache().m_EnabledAgentIDs.Invalidate();
						m_pShared->XMLCache().m_AgentSignatureIDs.Invalidate();
						break;
					case 2:
						m_pShared->XMLCache().m_GroupNames.Invalidate();
						m_pShared->XMLCache().m_GroupNamesWOAdmin.Invalidate();
						m_pShared->XMLCache().m_AgentSignatureIDs.Invalidate();
						break;
					case 3:
						m_pShared->XMLCache().m_TicketBoxAddrs.Invalidate();
						m_pShared->XMLCache().m_TicketBoxNames.Invalidate();
						m_pShared->XMLCache().m_TicketBoxIDs.Invalidate();
						m_pShared->XMLCache().m_DefaultEmailAddressName.Invalidate();
						break;
					case 4:
						m_pShared->XMLCache().m_MsgDestNames.Invalidate();
						break;
					case 5:
						m_pShared->XMLCache().m_MsgSourceNames.Invalidate();
						m_pShared->XMLCache().m_MessageSourceTypeMatch.Invalidate();
						break;
					case 6:
						m_pShared->XMLCache().m_RoutingRuleNames.Invalidate();
						break;
					case 7:
						m_pShared->XMLCache().m_ProcessingRuleNames.Invalidate();
						break;
					case 8:
						m_pShared->XMLCache().m_Signatures.Invalidate();
						m_pShared->XMLCache().m_AgentSignatureIDs.Invalidate();
						break;
					case 9:
						m_pShared->XMLCache().m_StdRespCategories.Invalidate();
						break;
					case 10:
						m_pShared->XMLCache().m_ServerParameters.Invalidate();
						break;
					case 11:
						m_pShared->XMLCache().m_PersonalDataTypes.Invalidate();
						break;
					case 12:
						m_pShared->XMLCache().m_AutoActionTypes.Invalidate();
						break;
					case 13:
						m_pShared->XMLCache().m_AutoActionEvents.Invalidate();
						break;
					case 14:
						m_pShared->XMLCache().m_TicketCategories.Invalidate();
						break;
					case 15:
						m_pShared->XMLCache().m_TicketBoxOwners.Invalidate();
						break;
					case 16:
						m_pShared->XMLCache().m_AutoMessages.Invalidate();
						break;
					case 17:
						m_pShared->XMLCache().m_AutoResponses.Invalidate();
						break;
					case 18:
						m_pShared->XMLCache().m_AgeAlerts.Invalidate();
						break;
					case 19:
						m_pShared->XMLCache().m_WaterMarkAlerts.Invalidate();
						break;
					case 20:
						m_pShared->XMLCache().m_AgentEmails.Invalidate();
						break;
					}
					m_pShared->XMLCache().BuildAllMaps(m_query);			
				case 4:
					/*if(vIter->nTargetID == 1)
					{
						m_DBMaintenanceMutex.AcquireLock(100);
					}
					else
					{
						if(m_DBMaintenanceMutex.IsLocked())
						{
							m_DBMaintenanceMutex.ReleaseLock();
						}
					}*/
					break;				
				}
			}
		}
	}
	catch(...)
	{
		try
		{
			m_RoutingEngine.LogIt( EMSERROR( EMS_LOG_SEVERITY_ERROR,
														EMSERR_ISAPI_EXTENSION,
														EMS_LOG_ISAPI_EXTENSION,
														E_SQLUnknown),
												  "An Unkown or undefined exception has occurred in the EMSISAPI Check Monitor Queue." );
		}
		catch(...)
		{
		}
	}
}
void CSessionMonitor::CheckInToDB( int nServerID, bool bIn)
{
	try
	{
		if( !m_db.IsConnected() )
		{
			m_db.Connect();			
		}

		TIMESTAMP_STRUCT Now;
		GetTimeStamp( Now );
		int nProcessID = m_pShared->SessionMap().GetProcessID();
		int nWSEnabled = m_pShared->SessionMap().GetWebSessionsEnabled();

		m_query.Initialize();
		BINDPARAM_TIME_NOLEN( m_query, Now );
		BINDPARAM_LONG( m_query, nServerID );
		BINDPARAM_LONG( m_query, nProcessID );
		if ( bIn )
		{
			m_query.Execute( _T("UPDATE Servers SET WebStatus=1,WebCheckIn=? WHERE ServerID=?") );
			if(nWSEnabled == 1)
			{
				m_query.Execute(_T("UPDATE SessionMonitors SET CheckIn=? WHERE ServerID=? AND ProcessID=?"));
			}
		}
		else
		{
			m_query.Execute( _T("UPDATE Servers SET WebStatus=0,WebCheckIn=? WHERE ServerID=?") );
			if(nWSEnabled == 1)
			{
				m_query.Reset(true);
				BINDPARAM_LONG( m_query, nServerID );
				BINDPARAM_LONG( m_query, nProcessID );
				m_query.Execute( _T("DELETE FROM SessionMonitors WHERE ServerID=? AND ProcessID=?"));
				m_query.Reset(false);
				m_query.Execute( _T("DELETE FROM SessionMonitorQueue WHERE ServerID=? AND ProcessID=?"));
			}			
		}
	}
	catch(...)
	{
		try
		{
			m_RoutingEngine.LogIt( EMSERROR( EMS_LOG_SEVERITY_ERROR,
														EMSERR_ISAPI_EXTENSION,
														EMS_LOG_ISAPI_EXTENSION,
														E_SQLUnknown),
												  "An Unkown or undefined exception has occurred in the EMSISAPI DB CheckIn." );
		}
		catch(...)
		{
		}
	}
	if ( m_pShared->SessionMap().GetNeedToResetLocks() )
	{
		m_pShared->SessionMap().ResetLocks( m_query );
	}
}
