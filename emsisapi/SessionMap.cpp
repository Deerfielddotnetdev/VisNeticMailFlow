#include "stdafx.h"
#include "SecurityFns.h"
#include "SessionMap.h"
#include "RegistryFns.h"
#include "AttachFns.h"
#include "PurgeFns.h"
#include "RoutingEngine.h"

void CAgentSession::Refresh( CODBCQuery& m_query )
{
	TCHAR szFilename[STYLESHEETS_FILENAME_LENGTH] = {0};
	long szFilenameLen;
	TAgents agent;
	CMarkupSTL xmlgen;
	bool bIsAdmin = IsAgentAdmin( m_query, m_AgentID );
	long nEscalateAgentValid = 0;
	long nEscalateGroupValid = 0;
	long nApproverCount = 0;
	long nApproverGroupCount = 0;	

	m_query.Initialize();
	
	BINDPARAM_LONG( m_query, m_AgentID );
	BINDCOL_TCHAR( m_query, agent.m_Name );
	BINDCOL_TCHAR( m_query, szFilename );
	BINDCOL_BIT( m_query, agent.m_UseIM );
	BINDCOL_BIT( m_query, agent.m_NewMessageFormat );
	BINDCOL_LONG( m_query, agent.m_EscalateToAgentID );
	BINDCOL_BIT( m_query, agent.m_QuoteMsgInReply );
	BINDCOL_TCHAR( m_query, agent.m_ReplyQuotedPrefix );
	BINDCOL_LONG( m_query, agent.m_MaxReportRowsPerPage );
	BINDCOL_BIT( m_query, agent.m_RequireGetOldest );
	BINDCOL_BIT( m_query, agent.m_ForceSpellCheck );
	BINDCOL_BIT( m_query, agent.m_SignatureTopReply );
	BINDCOL_BIT( m_query, agent.m_IsDeleted );
	BINDCOL_BIT( m_query, agent.m_ShowMessagesInbound );
	BINDCOL_BIT( m_query, agent.m_UsePreviewPane );
	BINDCOL_TINYINT( m_query, agent.m_CloseTicket );
	BINDCOL_TINYINT( m_query, agent.m_RouteToInbox );
	BINDCOL_BIT( m_query, agent.m_UseMarkAsRead );
	BINDCOL_LONG( m_query, agent.m_MarkAsReadSeconds );
	BINDCOL_LONG( m_query, agent.m_OutboundApprovalFromID );
	BINDCOL_BIT( m_query, agent.m_UseAutoFill );
	BINDCOL_LONG( m_query, agent.m_DefaultTicketBoxID );
	BINDCOL_LONG( m_query, agent.m_DefaultTicketDblClick );
	BINDCOL_LONG( m_query, agent.m_ReadReceipt );
	BINDCOL_LONG( m_query, agent.m_DeliveryConfirmation );
	BINDCOL_LONG( m_query, agent.m_StatusID );
	BINDCOL_TCHAR( m_query, agent.m_StatusText );	
	BINDCOL_TCHAR( m_query, agent.m_OnlineText );	
	BINDCOL_TCHAR( m_query, agent.m_AwayText );	
	BINDCOL_TCHAR( m_query, agent.m_NotAvailText );	
	BINDCOL_TCHAR( m_query, agent.m_DndText );	
	BINDCOL_TCHAR( m_query, agent.m_OfflineText );	
	BINDCOL_TCHAR( m_query, agent.m_OooText );
	BINDCOL_LONG( m_query, agent.m_AutoStatusTypeID );
	BINDCOL_LONG( m_query, agent.m_AutoStatusMin );
	BINDCOL_LONG( m_query, agent.m_LogoutStatusID );	
	BINDCOL_TCHAR( m_query, szLogoutText );
	BINDCOL_TCHAR( m_query, szAutoText );
	BINDCOL_LONG_NOLEN( m_query, nEscalateAgentValid );
	BINDCOL_LONG_NOLEN( m_query, nEscalateGroupValid );
	BINDCOL_LONG_NOLEN( m_query, nApproverCount );
	BINDCOL_LONG_NOLEN( m_query, nApproverGroupCount );
	
	m_query.Execute(  _T("SELECT A.Name,S.Filename,")
					_T("UseIM,NewMessageFormat,EscalateToAgentID,QuoteMsgInReply,ReplyQuotedPrefix,MaxReportRowsPerPage,RequireGetOldest,")
					_T("ForceSpellCheck, SignatureTopReply, IsDeleted, ShowMessagesInbound, UsePreviewPane,")
					_T("CloseTicket, RouteToInbox, UseMarkAsRead, MarkAsReadSeconds,OutboundApprovalFromID,UseAutoFill,DefaultTicketBoxID, ")
					_T("DefaultTicketDblClick,ReadReceipt,DeliveryConfirmation,StatusID,StatusText, ")
					_T("OnlineText,AwayText,NotAvailText,DndText,OfflineText,OooText,AutoStatusTypeID,AutoStatusMin,LogoutStatusID, ")
					_T("(SELECT CASE LogoutStatusID WHEN 5 THEN OfflineText WHEN 6 THEN OooText END), ")
					_T("(SELECT CASE AutoStatusTypeID WHEN 1 THEN OnlineText WHEN 2 THEN AwayText WHEN 3 THEN NotAvailText WHEN 4 THEN DndText WHEN 5 THEN OfflineText WHEN 6 THEN OooText END), ")
					_T("(SELECT COUNT(*) FROM Agents WHERE AgentID=A.EscalateToAgentID AND IsEnabled=1 AND IsDeleted=0), ")
					_T("(SELECT COUNT(*) FROM Groups INNER JOIN AgentGroupings ON Groups.GroupID=AgentGroupings.GroupID INNER JOIN Agents ON Agents.AgentID=AgentGroupings.AgentID WHERE Groups.GroupID=Abs(A.EscalateToAgentID) AND Groups.IsEscalationGroup=1 AND Groups.IsDeleted=0 AND Agents.IsDeleted=0 AND Agents.IsEnabled=1), ")
					_T("(SELECT COUNT(*) FROM Agents WHERE A.AgentID=OutboundApprovalFromID AND IsEnabled=1 AND IsDeleted=0), ")
					_T("(SELECT COUNT(*) FROM Agents WHERE A.AgentID IN ")
					_T("(SELECT AgentGroupings.AgentID FROM AgentGroupings WHERE AgentGroupings.GroupID IN (SELECT ")
					_T("Abs(OutboundApprovalFromID) FROM Agents WHERE OutboundApprovalFromID < 0 ))AND AgentID=A.AgentID) ")
					_T("FROM Agents as A LEFT OUTER JOIN StyleSheets as S ")
					_T("ON A.StyleSheetID = S.StyleSheetID ")
					_T("WHERE AgentID=? AND IsDeleted=0 AND IsEnabled=1 ") );

	if( m_query.Fetch() == S_OK )
	{
		if( nEscalateAgentValid == 0 && nEscalateGroupValid == 0 )
		{
			agent.m_EscalateToAgentID = 0;			
		}
	
		int m_nOutbound = 0;		

		if ( agent.m_OutboundApprovalFromID != 0 )
		{
			m_nOutbound = 1;
		}

		if ( nApproverCount > 0 || nApproverGroupCount > 0 )
		{
			m_bIsOutboundApprover = 1;
		}
		else
		{
			m_bIsOutboundApprover = 0;
		}
		

		xmlgen.AddElem(_T("user"));
		xmlgen.AddChildElem(_T("id"), m_AgentID);
		xmlgen.AddChildElem(_T("name"),agent.m_Name);
		xmlgen.AddChildElem(_T("useim"), agent.m_UseIM);
		xmlgen.AddChildElem(_T("newmessageformat"), agent.m_NewMessageFormat);
		xmlgen.AddChildElem(_T("useautofill"), agent.m_UseAutoFill);
		xmlgen.AddChildElem(_T("outboundapproval"), m_nOutbound);
		xmlgen.AddChildElem(_T("outboundapprover"), m_bIsOutboundApprover);		

		if( szFilenameLen == SQL_NULL_DATA )
		{
			xmlgen.AddChildElem(_T("stylesheetname"), _T("./stylesheets/standard.css"));	
		}
		else
		{
			xmlgen.AddChildElem(_T("stylesheetname"), szFilename);
		}
		
		// Add the menu background color.
		if ( _tcscmp( szFilename, _T("./stylesheets/earth.css") ) == 0 )
		{
			xmlgen.AddChildElem(_T("menubackcolor"), "#90C0E8");
		}
		else if ( _tcscmp( szFilename, _T("./stylesheets/hotdog.css") ) == 0 )
		{
			xmlgen.AddChildElem(_T("menubackcolor"), "#FEC51D");
		}
		else
		{
			xmlgen.AddChildElem(_T("menubackcolor"), _T("#D4D0C8"));
		}

		xmlgen.AddChildElem(_T("isadmin"), bIsAdmin );
		xmlgen.AddChildElem(_T("escalatetoagentid"), agent.m_EscalateToAgentID );	
		xmlgen.AddChildElem(_T("quotemsginreply"), agent.m_QuoteMsgInReply );
		xmlgen.AddChildElem(_T("replyquotedprefix"), agent.m_ReplyQuotedPrefix );
		xmlgen.AddChildElem(_T("forcespellcheck"), agent.m_ForceSpellCheck );
		xmlgen.AddChildElem(_T("signaturetopreply"), agent.m_SignatureTopReply );
		xmlgen.AddChildElem(_T("showmessagesinbound"), agent.m_ShowMessagesInbound );
		xmlgen.AddChildElem(_T("usepreviewpane"), agent.m_UsePreviewPane );
		xmlgen.AddChildElem(_T("closeticketaftersend"), agent.m_CloseTicket );
		xmlgen.AddChildElem(_T("autoroutetoinbox"), agent.m_RouteToInbox );
		xmlgen.AddChildElem(_T("usemarkasread"), agent.m_UseMarkAsRead );
		xmlgen.AddChildElem(_T("markasreadseconds"), agent.m_MarkAsReadSeconds);
		xmlgen.AddChildElem(_T("defaultticketboxid"), agent.m_DefaultTicketBoxID);	
		xmlgen.AddChildElem(_T("defaultticketdblclick"), agent.m_DefaultTicketDblClick);
		xmlgen.AddChildElem(_T("readreceipt"), agent.m_ReadReceipt);	
		xmlgen.AddChildElem(_T("deliveryconfirmation"), agent.m_DeliveryConfirmation);	
		xmlgen.AddChildElem(_T("statusid"), agent.m_StatusID);
		xmlgen.AddChildElem(_T("statustext"), agent.m_StatusText);
		xmlgen.AddChildElem(_T("onlinetext"), agent.m_OnlineText);
		xmlgen.AddChildElem(_T("awaytext"), agent.m_AwayText);
		xmlgen.AddChildElem(_T("notavailtext"), agent.m_NotAvailText);
		xmlgen.AddChildElem(_T("dndtext"), agent.m_DndText);
		xmlgen.AddChildElem(_T("offlinetext"), agent.m_OfflineText);
		xmlgen.AddChildElem(_T("oootext"), agent.m_OooText);
		xmlgen.AddChildElem(_T("autostatustypeid"), agent.m_AutoStatusTypeID );
		xmlgen.AddChildElem(_T("autostatusmin"), agent.m_AutoStatusMin );
		xmlgen.AddChildElem(_T("logoutstatusid"), agent.m_LogoutStatusID );
		xmlgen.AddChildElem(_T("logouttext"), szLogoutText );
		xmlgen.AddChildElem(_T("autotext"), szAutoText );		

		xmlgen.AddChildElem( _T("access") );
		xmlgen.IntoElem();

#if defined(DEMO_VERSION)
		xmlgen.AddChildElem( _T("contact"),CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID, EMS_OBJECT_TYPE_CONTACT, 0) );
		xmlgen.AddChildElem( _T("stdresp"), CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_STD_RESP, 0) );
#else
		xmlgen.AddChildElem( _T("contact"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID, EMS_OBJECT_TYPE_CONTACT, 0) );

		xmlgen.AddChildElem( _T("stdresp"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_STD_RESP, 0) );

		xmlgen.AddChildElem( _T("stdreports"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_STANDARD_REPORTS, 0) );

		xmlgen.AddChildElem( _T("schedreports"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_SCHEDULED_REPORTS, 0) );

		xmlgen.AddChildElem( _T("autoopen"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_AUTO_TICKET_OPEN, 0) );

		xmlgen.AddChildElem( _T("agentaudit"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_AGENT_AUDIT, 0) );

		xmlgen.AddChildElem( _T("voip"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_VOIP, 0) );

		xmlgen.AddChildElem( _T("pref"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_AGENT_PREFERENCES, 0) );

		xmlgen.AddChildElem( _T("prefgen"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_GENERAL, 0) );

		xmlgen.AddChildElem( _T("preftb"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_TICKETBOX, 0) );

		xmlgen.AddChildElem( _T("prefav"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_AGENT_VIEWS, 0) );

		xmlgen.AddChildElem( _T("prefsig"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_SIGNATURES, 0) );

		xmlgen.AddChildElem( _T("prefmydict"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_MY_DICTIONARY, 0) );

		xmlgen.AddChildElem( _T("prefmyfold"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_MY_FOLDERS, 0) );

		xmlgen.AddChildElem( _T("prefmyal"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_MY_ALERTS, 0) );

		xmlgen.AddChildElem( _T("prefmycont"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_MY_CONTACTS, 0) );

		xmlgen.AddChildElem( _T("rrtm"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_ROUTE_REPLIES, 0) );

		xmlgen.AddChildElem( _T("ctas"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_CLOSE_TICKET, 0) );

		xmlgen.AddChildElem( _T("rrr"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_REQUEST_READ, 0) );

		xmlgen.AddChildElem( _T("rdc"), bIsAdmin ? EMS_DELETE_ACCESS : 
		CAccessControl::GetInstance().GetAgentRightLevel( m_query, m_AgentID,EMS_OBJECT_TYPE_REQUEST_DELIVERY, 0) );
#endif

		xmlgen.OutOfElem();

		m_AgentXML = xmlgen.GetDoc();
		m_IsAdmin = bIsAdmin;
		m_UseAutoFill = (agent.m_UseAutoFill!=0);
		m_AgentUserName = agent.m_Name;
		m_AgentStatusTypeID = agent.m_StatusID;
		m_AgentStatusText = agent.m_StatusText;
		m_AgentAutoStatusTypeID = agent.m_AutoStatusTypeID;
		m_AgentAutoStatusMin = agent.m_AutoStatusMin;
		m_LogoutStatusID = agent.m_LogoutStatusID;
		m_LogoutText = szLogoutText;
		m_AutoText = szAutoText;
		m_EscalateToAgentID = agent.m_EscalateToAgentID;
		m_RequireGetOldest = agent.m_RequireGetOldest;
		m_nMaxRowsPerPage = (agent.m_MaxReportRowsPerPage > 0) ? agent.m_MaxReportRowsPerPage : EMS_DEFAULT_PAGE_ROWS;

		dca::String x;
		x.Format("CAgentSession::Refresh - Agent: [%s] RequireGetOldest set to: [%d]", agent.m_Name, m_RequireGetOldest);
		DebugReporter::Instance().DisplayMessage(x.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
	}
	else
	{
		m_IsDeleted = true;
	}

	m_bIsDirty = false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSessionMap::CSessionMap() : m_query(m_db)
{
	s_Version = "6.9.4.1";
	nMutexWait = 5000;	
	
	try
	{
		GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("MutexWait"), nMutexWait);
		dca::String z;
		z.Format("CSessionMap - MutexWait set to: [%d]", nMutexWait);
		DebugReporter::Instance().DisplayMessage(z.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				
		m_db.Connect();
		LoadSessionSettings();
		if(nDBEnabled == 1)
		{
			if(nWSEnabled == 1)
			{
				InsertSessionMonitor();
			}
			LoadSessionsFromDB();
			RemoveOldBrowserSessionsFromDB();			
		}
		else
		{
			LoadSessions();
		}
		LoadStatus();
				
	}
	catch(...) 
	{ 
		CloseDatabase();
	}
}

CSessionMap::~CSessionMap()
{
	try
	{
		CloseDatabase();
	}
	catch(...){}
}

////////////////////////////////////////////////////////////////////////////////
// 
// InsertSessionMonitor
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::InsertSessionMonitor(void)
{
	bool bNewSessionMonitor = true;
	
	//Check to see if the SessionMonitor exists
	m_query.Reset(true);
	BINDPARAM_LONG( m_query, nServerID );
	BINDPARAM_LONG( m_query, nProcessID );
	m_query.Execute( _T("SELECT * FROM SessionMonitors WHERE ServerID = ? AND ProcessID=?") );
	if( m_query.Fetch() == S_OK )
	{
		bNewSessionMonitor = false;
	}	
	
	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );
	m_query.Reset(true);
	BINDPARAM_TIME_NOLEN( m_query, now );
	BINDPARAM_LONG( m_query, nServerID );
	BINDPARAM_LONG( m_query, nProcessID );		
	if(bNewSessionMonitor)
	{
		m_query.Execute(_T("INSERT INTO SessionMonitors (CheckIn,ServerID,ProcessID) VALUES (?,?,?)"));
	}
	else
	{
		m_query.Execute(_T("UPDATE SessionMonitors SET CheckIn=? WHERE ServerID=? AND ProcessID=?"));
	}

}	
////////////////////////////////////////////////////////////////////////////////
// 
// LoadSessions
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::LoadSessions(void)
{
	CMarkupSTL xmlgen;
	FILETIME ft;
	tstring	sVersion;
	CODBCConn	m_db;
	CODBCQuery	m_query;

	// load the sessions file into the XML parser
	if ( !xmlgen.Load( _T("sessions.xml") ) )
	{
		// reset all locks
		m_bNeedToResetLocks = true;

		return;
	}

	// Check the version of sessions.xml - ignore if from previous version
	if( xmlgen.FindChildElem( _T("Version") ) )
	{
		sVersion = xmlgen.GetChildData();
	}
	if( sVersion.compare( s_Version ) != 0 )
	{
		DeleteFile( _T("sessions.xml") );
		return;
	}

	xmlgen.ResetPos();
	xmlgen.FindElem( _T("root") );

	// Load browser sessions
	if(m_BrowserMapMutex.AcquireLock(nMutexWait))
	{
		while( xmlgen.FindChildElem( _T("BrowserSession") ) )
		{
			tstring szSessionID;
			CBrowserSession session;

			szSessionID = xmlgen.GetChildAttrib( _T("ID") );
			session.m_AgentID = _ttoi( xmlgen.GetChildAttrib( _T("AgentID") ).c_str() );
		
			ft.dwHighDateTime = _ttoi( xmlgen.GetChildAttrib( _T("LastRequestHigh") ).c_str() );
			ft.dwLowDateTime = _ttoi( xmlgen.GetChildAttrib( _T("LastRequestLow") ).c_str() );

			FileTimeToSystemTime( &ft, &session.m_LastRequest );

			ft.dwHighDateTime = _ttoi( xmlgen.GetChildAttrib( _T("LastAutoHigh") ).c_str() );
			ft.dwLowDateTime = _ttoi( xmlgen.GetChildAttrib( _T("LastAutoLow") ).c_str() );

			session.m_IP = xmlgen.GetChildAttrib( _T("IP") );
			session.m_UserAgent = xmlgen.GetChildAttrib( _T("UserAgent") );

			session.m_RightHandPane = xmlgen.GetChildData();

			m_BrowserMap[ szSessionID ] = session;
		
		}
		m_BrowserMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::LoadSessions - Could not aquire BrowserMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::LoadSessions - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}

	// Load Agent Sessions	
	if(m_AgentMapMutex.AcquireLock(nMutexWait))
	{
		while( xmlgen.FindChildElem( _T("AgentSession") ) )
		{
			CAgentSession session;

			session.m_AgentID = _ttoi( xmlgen.GetChildAttrib( _T("AgentID") ).c_str() );
		
			ft.dwHighDateTime = _ttoi( xmlgen.GetChildAttrib( _T("CreateTimeHigh") ).c_str() );
			ft.dwLowDateTime = _ttoi( xmlgen.GetChildAttrib( _T("CreateTimeLow") ).c_str() );

			FileTimeToSystemTime( &ft, &session.m_CreateTime );

			session.m_IsAdmin = ( _ttoi( xmlgen.GetChildAttrib( _T("IsAdmin") ).c_str() ) != 0);
			session.m_AgentUserName = xmlgen.GetChildAttrib( _T("AgentUserName") );
			session.m_EscalateToAgentID =  _ttoi(xmlgen.GetChildAttrib( _T("EscalateToAgentID")).c_str());
			session.m_bIsOutboundApprover = ( _ttoi( xmlgen.GetChildAttrib( _T("OutboundApprover") ).c_str() ) != 0);
			session.m_nMaxRowsPerPage = _ttoi( xmlgen.GetChildAttrib( _T("MaxRowsPerPage")).c_str());
			session.m_AgentIP = xmlgen.GetChildAttrib( _T("IP") );
			session.m_AgentStatusTypeID = _ttoi( xmlgen.GetChildAttrib( _T("StatusID")).c_str());
			session.m_AgentStatusText = xmlgen.GetChildAttrib( _T("StatusText") );
			session.m_AgentAutoStatusTypeID = _ttoi( xmlgen.GetChildAttrib( _T("AutoStatusTypeID")).c_str());
			session.m_AgentAutoStatusMin = _ttoi( xmlgen.GetChildAttrib( _T("AutoStatusMin")).c_str());
			session.m_LogoutStatusID = _ttoi( xmlgen.GetChildAttrib( _T("LogoutStatusID")).c_str());
			session.m_LogoutText = xmlgen.GetChildAttrib( _T("LogoutText") );
			session.m_AutoText = xmlgen.GetChildAttrib( _T("AutoText"));
			session.m_PreAutoID = _ttoi( xmlgen.GetChildAttrib( _T("PreAutoID")).c_str());
			session.m_PreAutoText = xmlgen.GetChildAttrib( _T("PreAutoText"));
			session.m_bIsDirty = true;

			xmlgen.IntoElem();
			xmlgen.FindChildElem( _T("user") );
			session.m_AgentXML = xmlgen.GetChildSubDoc();
			xmlgen.OutOfElem();

			m_AgentMap[ session.m_AgentID ] = session;

			//RefreshAgentSession( session, m_query, false);
		}
		m_AgentMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::LoadSessions - Could not aquire AgentMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::LoadSessions - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
	
	// load the list of agent id's which we need to reset
	if ( xmlgen.FindChildElem( _T("ResetAllLocks") ) )
	{
		m_bNeedToResetLocks = true;
	}
	else
	{
		if(m_ResetLocksMutex.AcquireLock(nMutexWait))
		{
			while( xmlgen.FindChildElem( _T("ResetLock") ) )
			{
				m_ResetLockList.push_back( _ttoi( xmlgen.GetChildAttrib( _T("AgentID") ).c_str() ) );
			}
			// do we need to reset locks?
			m_bNeedToResetLocks = (m_ResetLockList.size() > 0);
			m_ResetLocksMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::LoadSessions - Could not aquire ResetLocksMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::LoadSessions - Could not aquire ResetLocksMutex.", DebugReporter::ISAPI, GetCurrentThreadId());				
		}
	}
		
	// delete the sessions file ( we don't want it be used again if we crash)
	// DeleteFile( _T("sessions.xml"));
}

////////////////////////////////////////////////////////////////////////////////
// 
// LoadSessionsFromDB
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::LoadSessionsFromDB(void)
{
	if(!m_db.IsConnected()){try{m_db.Connect();}catch(...){}}	
	
	TCHAR szSessionID[51] = {0};
	TCHAR szIP[51] = {0};
	TCHAR szUserAgent[51] = {0};
	TCHAR szPreAutoText[126] = {0};
	int nAgentID;
	int nPreAutoID;
	TIMESTAMP_STRUCT lr;
	long lrLen;
	TIMESTAMP_STRUCT ct;
	long ctLen;	
	
	m_query.Initialize();
	
	BINDCOL_TCHAR_NOLEN( m_query, szSessionID );
	BINDCOL_TCHAR_NOLEN( m_query, szIP );
	BINDCOL_LONG_NOLEN( m_query, nAgentID );
	BINDCOL_TIME( m_query, lr );
	BINDCOL_TIME( m_query, ct );
	BINDCOL_TCHAR_NOLEN( m_query, szUserAgent );
	m_query.Execute(  _T("SELECT SessionID,BrowserIP,AgentID,LastRequest,")
					_T("CreateTime,UserAgent ")
					_T("FROM BrowserSessions") );
	
	if(m_BrowserMapMutex.AcquireLock(nMutexWait))
	{	
		while( m_query.Fetch() == S_OK )
		{
			CBrowserSession session;

			session.m_AgentID = nAgentID;
			TimeStampToSystemTime(lr, session.m_LastRequest);
			TimeStampToSystemTime(ct, session.m_CreateTime);
			session.m_IP.assign(szIP);
			session.m_UserAgent.assign(szUserAgent);		

			m_BrowserMap[ szSessionID ] = session;
		}
		m_BrowserMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::LoadSessionsFromDB - Could not aquire BrowserMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::LoadSessionsFromDB - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}

	m_query.Reset(true);
	
	BINDCOL_LONG_NOLEN( m_query, nAgentID );
	BINDCOL_TIME( m_query, ct );
	BINDCOL_TIME( m_query, lr );
	BINDCOL_LONG_NOLEN( m_query, nPreAutoID );
	BINDCOL_TCHAR_NOLEN( m_query, szPreAutoText );	
	m_query.Execute(  _T("SELECT AgentID,CreateTime,LastAuto,PreAutoID,PreAutoText ")
					_T("FROM AgentSessions") );
	
	if(m_AgentMapMutex.AcquireLock(nMutexWait))
	{
		while( m_query.Fetch() == S_OK )
		{
			CAgentSession session;

			session.m_AgentID = nAgentID;	
			TimeStampToSystemTime(lr, session.m_LastAutoAway);
			TimeStampToSystemTime(ct, session.m_CreateTime);
			session.m_PreAutoID = nPreAutoID;
			session.m_PreAutoText.assign(szPreAutoText);
			session.m_bIsDirty = true;
			m_AgentMap[ session.m_AgentID ] = session;
		}

		map<unsigned int,CAgentSession>::iterator iter;
		
		for ( iter = m_AgentMap.begin(); iter != m_AgentMap.end(); iter++ )
		{
			CAgentSession session;

			session.m_AgentID = iter->second.m_AgentID;
			session.m_CreateTime = iter->second.m_CreateTime;
			session.m_LastAutoAway = iter->second.m_LastAutoAway;
			session.m_PreAutoID = iter->second.m_PreAutoID;
			session.m_PreAutoText.assign(iter->second.m_PreAutoText);		
			//RefreshAgentSession( session, m_query, false);				
		}
		m_AgentMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::LoadSessionsFromDB - Could not aquire AgentMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::LoadSessionsFromDB - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// LoadStatus
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::LoadStatus(void)
{
	if(!m_db.IsConnected()){try{m_db.Connect();}catch(...){}}

	try
	{
		int nStatusID;
		int nAgentID;
		int nLoggedIn = 0;
		dca::String f;			
		TCHAR szStatusText[AGENTS_STATUS_TEXT_LENGTH] = {0};
		long szStatusTextLen = 0;
		dca::String s;

		DebugReporter::Instance().DisplayMessage("CSessionMap::LoadStatus - Getting Agent Status from DB", DebugReporter::ISAPI, GetCurrentThreadId());

		m_query.Initialize();
		m_query.SetCustomTimeout(10);
		BINDCOL_LONG_NOLEN( m_query, nAgentID );
		BINDCOL_LONG_NOLEN( m_query, nStatusID );
		BINDCOL_TCHAR_NOLEN( m_query, szStatusText );
		m_query.Execute(  _T("SELECT AgentID,StatusID,StatusText FROM Agents WHERE IsDeleted=0 AND IsEnabled=1"));
		
		if(m_StatusMapMutex.AcquireLock(nMutexWait))
		{
			if(m_AgentMapMutex.AcquireLock(nMutexWait))
			{
				while( m_query.Fetch() == S_OK )
				{
					nLoggedIn = 0;
					CAgentStatus status;

					if(nStatusID < 0 || nStatusID > 6){nStatusID=1;}
					
					map<unsigned int,CAgentSession>::iterator AgentIter = m_AgentMap.find( nAgentID );
					if( AgentIter != m_AgentMap.end() )
					{
						s.Format("CSessionMap::LoadStatus - Updating AgentMap for AgentID [%d] to StatusID [%d] and StatusText [%s]", nAgentID, nStatusID, szStatusText);
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

						AgentIter->second.m_AgentStatusTypeID = nStatusID;
						AgentIter->second.m_AgentStatusText.assign(szStatusText);
						nLoggedIn = 1;
					}

					status.m_AgentID = nAgentID;
					status.m_AgentStatusTypeID = nStatusID;
					status.m_LoggedIn = nLoggedIn;
					status.m_AgentStatusText.assign(szStatusText);			
				
					map<unsigned int,CAgentStatus>::iterator StatusIter = m_StatusMap.find(status.m_AgentID);
					if( StatusIter != m_StatusMap.end() )
					{
						s.Format("CSessionMap::LoadStatus - Updating StatusMap for AgentID [%d] to StatusID [%d] StatusText [%s] and LoggedIn [%d]", status.m_AgentID, status.m_AgentStatusTypeID, status.m_AgentStatusText.c_str(), status.m_LoggedIn);
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

						StatusIter->second.m_AgentStatusTypeID = status.m_AgentStatusTypeID;
						StatusIter->second.m_LoggedIn = status.m_LoggedIn;
						StatusIter->second.m_AgentStatusText.assign(status.m_AgentStatusText);
					}
					else
					{
						m_StatusMap[ status.m_AgentID ] = status;		
					}			
				}
				m_AgentMapMutex.ReleaseLock();
			}
			else
			{
				CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
				if(pRoutingEngine)
				{
					pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
						_T("CSessionMap::LoadStatus - Could not aquire AgentMapMutex.") );
				}				
				DebugReporter::Instance().DisplayMessage("CSessionMap::LoadStatus - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());				
			}
			m_StatusMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::LoadStatus - Could not aquire StatusMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::LoadStatus - Could not aquire StatusMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());			
		}
	}
	catch(...)
	{
		m_AgentMapMutex.ReleaseLock();
		m_StatusMapMutex.ReleaseLock();

		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_ERROR(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error loading Agent Status") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::LoadStatus - Error loading Agent Status", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SyncSessionsWithDB
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::SyncSessionsWithDB(void)
{
	DebugReporter::Instance().DisplayMessage("CSessionMap::SyncSessionsWithDB - Entering.", DebugReporter::ISAPI, GetCurrentThreadId());
	
	if(!m_db.IsConnected()){try{m_db.Connect();}catch(...){}}
	
	try
	{
		DebugReporter::Instance().DisplayMessage("CSessionMap::SyncSessionsWithDB - Get Browser Sessions from DB.", DebugReporter::ISAPI, GetCurrentThreadId());
		
		BROWSER_SESSION_MAP bMap;
		BROWSER_SESSION_MAP::iterator bIter;		
		BROWSER_SESSION_MAP::iterator BrowserIter;
		map<unsigned int,CAgentSession>::iterator AgentIter;
		TIMESTAMP_STRUCT lr;
		long lrLen;
		dca::String s;
				
		//Get Browser Sessions From DB
		bMap.clear();
		TCHAR szSID[51] = {0};
		long szSIDLen;
		m_query.Initialize();	
		BINDCOL_TCHAR( m_query, szSID );
		BINDCOL_TIME( m_query, lr );
		m_query.Execute(  _T("SELECT SessionID,LastRequest FROM BrowserSessions") );
		while( m_query.Fetch() == S_OK )
		{
			CBrowserSession bSession;
			TimeStampToSystemTime(lr, bSession.m_LastRequest);		
			bMap[szSID] = bSession;	
		}

		DebugReporter::Instance().DisplayMessage("CSessionMap::SyncSessionsWithDB - Get Agent Sessions from DB.", DebugReporter::ISAPI, GetCurrentThreadId());
		
		//Get Agent Sessions From DB
		int nASID;
		int nAID;
		TIMESTAMP_STRUCT la;
		long laLen;		
		map<unsigned int,CAgentSession> aMap;
		map<unsigned int,CAgentSession>::iterator aIter;
		map<int,int> mAs;
		map<int,int>::iterator mAsIter;
		int nPAID;
		TCHAR szPreAutoText[126] = {0};
		long szPreAutoTextLen;		
		m_query.Initialize();	
		BINDCOL_LONG_NOLEN( m_query, nASID );
		BINDCOL_LONG_NOLEN( m_query, nAID );
		BINDCOL_TIME( m_query, la );
		BINDCOL_LONG_NOLEN( m_query, nPAID );
		BINDCOL_TCHAR( m_query, szPreAutoText );
		m_query.Execute(  _T("SELECT AgentSessionID,AgentID,LastAuto,PreAutoID,PreAutoText FROM AgentSessions") );
		while( m_query.Fetch() == S_OK )
		{
			CAgentSession aSession;
			aSession.m_AgentID = nAID;
			TimeStampToSystemTime(la, aSession.m_LastAutoAway);
			aSession.m_PreAutoID = nPAID;
			aSession.m_PreAutoText.assign(szPreAutoText);
			aMap[nASID] = aSession;
		}

		if(m_BrowserMapMutex.AcquireLock(nMutexWait))
		{
			DebugReporter::Instance().DisplayMessage("CSessionMap::SyncSessionsWithDB - Browser Map Locked.", DebugReporter::ISAPI, GetCurrentThreadId());
		
			for ( BrowserIter = m_BrowserMap.begin(); BrowserIter != m_BrowserMap.end(); BrowserIter++ )
			{
				int nUpdate = 0;
				for ( bIter = bMap.begin(); bIter != bMap.end(); bIter++ )
				{
					if( (_tcsicmp( BrowserIter->first.c_str(), bIter->first.c_str() ) == 0) )
					{
						s.Format("CSessionMap::SyncSessionsWithDB - Found SessionID [%s] in DB and Map", BrowserIter->first.c_str());
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						
						nUpdate = 1;
						TIMESTAMP_STRUCT dbLast;
						long dbLastLen=0;		
						TIMESTAMP_STRUCT sessionLast;
						long sessionLastLen=0;		
						CEMSString sDate;
						SystemTimeToTimeStamp(bIter->second.m_LastRequest,dbLast);
						GetFullDateTimeString(bIter->second.m_LastRequest,sDate);
						s.Format("CSessionMap::SyncSessionsWithDB - Converting DB SystemTime LastRequest [%s].", sDate.c_str());
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						GetFullDateTimeString(dbLast,dbLastLen,sDate);
						s.Format("CSessionMap::SyncSessionsWithDB - Converting DB TimeStamp LastRequest [%s].", sDate.c_str());
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						
						SystemTimeToTimeStamp(BrowserIter->second.m_LastRequest,sessionLast);
						GetFullDateTimeString(BrowserIter->second.m_LastRequest,sDate);
						s.Format("CSessionMap::SyncSessionsWithDB - Converting Map SystemTime LastRequest [%s].", sDate.c_str());
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						GetFullDateTimeString(sessionLast,sessionLastLen,sDate);
						s.Format("CSessionMap::SyncSessionsWithDB - Converting Map TimeStamp LastRequest [%s].", sDate.c_str());
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						
						if(OrderTimeStamps(dbLast,sessionLast))
						{
							tstring sSessionID(BrowserIter->first.c_str());
					
							s.Format("CSessionMap::SyncSessionsWithDB - Updating LastRequest for SessionID [%s] in DB", BrowserIter->first.c_str());
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

							m_query.Reset(true);
							BINDPARAM_TIME( m_query, sessionLast );
							BINDPARAM_TCHAR_STRING( m_query, sSessionID );
							m_query.Execute(  _T("UPDATE BrowserSessions ")
								_T("SET LastRequest=? WHERE SessionID = ?") );							
						}
						else if(CompareTimeStamps(dbLast,sessionLast) != 0)
						{
							s.Format("CSessionMap::SyncSessionsWithDB - Updating LastRequest for SessionID [%s] in Map", BrowserIter->first.c_str());
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
												
							memcpy( &BrowserIter->second.m_LastRequest, &bIter->second.m_LastRequest, sizeof(SYSTEMTIME) );				
						}
						else
						{
							s.Format("CSessionMap::SyncSessionsWithDB - LastRequest for for SessionID [%s] matches.", BrowserIter->first.c_str());
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						}
					}
				}					
				
				if(nUpdate == 0)
				{
					//Not in DB remove from m_BrowserMap
					s.Format("CSessionMap::SyncSessionsWithDB - SessionID [%s] not in DB, removing from Map.", BrowserIter->first.c_str());
					DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

					BrowserIter = m_BrowserMap.erase( BrowserIter );			
				}
			}			

			if(m_AgentMapMutex.AcquireLock(nMutexWait))
			{
				DebugReporter::Instance().DisplayMessage("CSessionMap::SyncSessionsWithDB - Agent Map Locked.", DebugReporter::ISAPI, GetCurrentThreadId());
		
				for ( AgentIter = m_AgentMap.begin(); AgentIter != m_AgentMap.end(); AgentIter++ )
				{
					int nUpdate = 0;
					for ( aIter = aMap.begin(); aIter != aMap.end(); aIter++ )
					{
						if( AgentIter->second.m_AgentID == aIter->second.m_AgentID )
						{
							s.Format("CSessionMap::SyncSessionsWithDB - Found AgentID [%d] in DB and Map", AgentIter->second.m_AgentID);
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

							nUpdate = 1;
							TIMESTAMP_STRUCT dbAuto;
							long dbAutoLen=0;		
							TIMESTAMP_STRUCT sessionAuto;
							long sessionAutoLen=0;		
							CEMSString sDate;
							
							SystemTimeToTimeStamp(aIter->second.m_LastAutoAway,dbAuto);
							GetFullDateTimeString(aIter->second.m_LastAutoAway,sDate);
							s.Format("CSessionMap::SyncSessionsWithDB - Converting DB SystemTime LastAuto [%s].", sDate.c_str());
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
							GetFullDateTimeString(dbAuto,dbAutoLen,sDate);
							s.Format("CSessionMap::SyncSessionsWithDB - Converting DB TimeStamp LastAuto [%s].", sDate.c_str());
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
							
							SystemTimeToTimeStamp(AgentIter->second.m_LastAutoAway,sessionAuto);
							GetFullDateTimeString(AgentIter->second.m_LastAutoAway,sDate);
							s.Format("CSessionMap::SyncSessionsWithDB - Converting Map SystemTime LastAuto [%s].", sDate.c_str());
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
							GetFullDateTimeString(sessionAuto,sessionAutoLen,sDate);
							s.Format("CSessionMap::SyncSessionsWithDB - Converting Map TimeStamp LastAuto [%s].", sDate.c_str());
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

							if(OrderTimeStamps(dbAuto,sessionAuto))
							{
								s.Format("CSessionMap::SyncSessionsWithDB - Updating Last LastAutoAway for AgentID [%d] in DB", AgentIter->second.m_AgentID);
								DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

								int nAgentID = AgentIter->second.m_AgentID;
								int nPreAutoID = AgentIter->second.m_PreAutoID;
								if(nPreAutoID < 0){nPreAutoID=0;}
								tstring sPreAutoText(AgentIter->second.m_PreAutoText);		
								
								m_query.Reset(true);
								BINDPARAM_TIME( m_query, sessionAuto );
								BINDPARAM_LONG( m_query, nPreAutoID );
								BINDPARAM_TCHAR_STRING( m_query, sPreAutoText );
								BINDPARAM_LONG( m_query, nAgentID );						
								m_query.Execute(  _T("UPDATE AgentSessions ")
									_T("SET LastAuto=?,PreAutoID=?,PreAutoText=? WHERE AgentID = ?") );
							}
							else if(CompareTimeStamps(dbAuto,sessionAuto) != 0)
							{
								s.Format("CSessionMap::SyncSessionsWithDB - Updating Last LastAutoAway for AgentID [%d] in Map", AgentIter->second.m_AgentID);
								DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
													
								memcpy( &AgentIter->second.m_LastAutoAway, &aIter->second.m_LastAutoAway, sizeof(SYSTEMTIME) );
								AgentIter->second.m_PreAutoText.assign(aIter->second.m_PreAutoText);
								AgentIter->second.m_PreAutoID = aIter->second.m_PreAutoID;
							}
							else
							{
								s.Format("CSessionMap::SyncSessionsWithDB - LastAutoAway for AgentID [%d] matches.", AgentIter->second.m_AgentID);
								DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
							}
						}
					}							

					if(nUpdate == 0)
					{
						//Not in DB, remove from m_AgentMap
						AgentIter = m_AgentMap.erase( AgentIter );
					}
				}
				m_AgentMapMutex.ReleaseLock();
			}
			else
			{
				CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
				if(pRoutingEngine)
				{
					pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
						_T("CSessionMap::SyncSessionsWithDB - Could not aquire AgentMapMutex.") );
				}
				DebugReporter::Instance().DisplayMessage("CSessionMap::SyncSessionsWithDB - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
			}
			m_BrowserMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::SyncSessionsWithDB - Could not aquire BrowserMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::SyncSessionsWithDB - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
	catch(ODBCError_t error) 
	{ 
		m_AgentMapMutex.ReleaseLock();
		m_BrowserMapMutex.ReleaseLock();
		
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMSERROR( EMS_LOG_SEVERITY_ERROR,
											EMSERR_ISAPI_EXTENSION,
											EMS_LOG_DATABASE_MAINTENANCE,
			                                E_ODBCError),
								  "ODBC Error running SQL Query: %s", 
									error.szErrMsg );
			
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error syncing sessions with DB") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::SyncSessionsWithDB - Error syncing sessions with DB", DebugReporter::ISAPI, GetCurrentThreadId());
	}	
	catch(...)
	{
		m_AgentMapMutex.ReleaseLock();
		m_BrowserMapMutex.ReleaseLock();
		
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error syncing sessions with DB") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::SyncSessionsWithDB - Error syncing sessions with DB", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SaveSessionToDB
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::SaveSessionToDB(const tstring& sSessionID, unsigned int nAgentID, bool bNewBrowserSession)
{
	if(!m_db.IsConnected()){try{m_db.Connect();}catch(...){}}
	
	try
	{
		if(m_BrowserMapMutex.AcquireLock(nMutexWait))
		{
			BROWSER_SESSION_MAP::iterator BrowserIter = m_BrowserMap.find( sSessionID );
			if( BrowserIter != m_BrowserMap.end() )
			{
				TIMESTAMP_STRUCT lr;
				long lrLen;
				TIMESTAMP_STRUCT ct;
				long ctLen;			
				SystemTimeToTimeStamp( BrowserIter->second.m_LastRequest, lr );
				lrLen = sizeof(TIMESTAMP_STRUCT);
				SystemTimeToTimeStamp( BrowserIter->second.m_CreateTime, ct );
				ctLen = sizeof(TIMESTAMP_STRUCT);
				tstring sSessionID(BrowserIter->first.c_str());
				int nAgentID = BrowserIter->second.m_AgentID;
				tstring sUserAgent(BrowserIter->second.m_UserAgent);
				tstring sIP(BrowserIter->second.m_IP);
				tstring sLastPage(BrowserIter->second.m_RightHandPane);
				if(sLastPage.length() > 254)
				{
					sLastPage = sLastPage.substr(0,254);
				}
									
				m_query.Initialize();		
				BINDPARAM_TCHAR_STRING( m_query, sIP );
				BINDPARAM_LONG( m_query, nAgentID );
				BINDPARAM_TIME( m_query, lr );
				BINDPARAM_TIME( m_query, ct );
				BINDPARAM_TCHAR_STRING( m_query, sUserAgent );
				BINDPARAM_TCHAR_STRING( m_query, sSessionID );
				
				if(bNewBrowserSession)
				{
					//Insert new BrowserSession					
					m_query.Execute(  _T("INSERT INTO BrowserSessions ")
							_T("(BrowserIP,AgentID,LastRequest,")
							_T("CreateTime,UserAgent,SessionID) VALUES (?,?,?,?,?,?)") );
				}
				else
				{
					//Update existing BrowserSession
					m_query.Execute(  _T("UPDATE BrowserSessions ")
							_T("SET BrowserIP=?,AgentID=?,LastRequest=?,")
							_T("CreateTime=?,UserAgent=? WHERE SessionID=?") );
				}

				if(m_AgentMapMutex.AcquireLock(nMutexWait))
				{
					bool bNewAgentSession = true;
					//Check to see if AgentSession exists
					m_query.Reset(true);
					BINDPARAM_LONG( m_query, nAgentID );
					m_query.Execute( _T("SELECT * FROM AgentSessions WHERE AgentID = ?") );
					if( m_query.Fetch() == S_OK )
					{
						bNewAgentSession = false;
					}	
					
					map<unsigned int,CAgentSession>::iterator AgentIter = m_AgentMap.find( nAgentID );
					if( AgentIter != m_AgentMap.end() )
					{
						TIMESTAMP_STRUCT lr;
						long lrLen;
						SystemTimeToTimeStamp( AgentIter->second.m_LastAutoAway, lr );
						lrLen = sizeof(TIMESTAMP_STRUCT);
						int nPreAutoID = AgentIter->second.m_PreAutoID;
						if(nPreAutoID < 0){nPreAutoID=0;}
						tstring sPreAutoText(AgentIter->second.m_PreAutoText);		
						
						m_query.Reset(true);						
						BINDPARAM_TIME( m_query, lr );
						BINDPARAM_LONG( m_query, nPreAutoID );
						BINDPARAM_TCHAR_STRING( m_query, sPreAutoText );
						BINDPARAM_LONG( m_query, nAgentID );
						if(bNewAgentSession)
						{
							m_query.Execute(_T("INSERT INTO AgentSessions (LastAuto,PreAutoID,PreAutoText,AgentID) VALUES (?,?,?,?)"));
						}
						else
						{
							m_query.Execute(_T("UPDATE AgentSessions SET LastAuto=?,PreAutoID=?,PreAutoText=? WHERE AgentID=?"));
						}
						
					}
					m_AgentMapMutex.ReleaseLock();
				}
				else
				{
					CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
					if(pRoutingEngine)
					{
						pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
							_T("CSessionMap::SaveSessionToDB - Could not aquire AgentMapMutex.") );
					}
					DebugReporter::Instance().DisplayMessage("CSessionMap::SaveSessionToDB - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
				}
			}
			m_BrowserMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::SaveSessionToDB - Could not aquire BrowserMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::SaveSessionToDB - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
	catch(...)
	{
		m_AgentMapMutex.ReleaseLock();
		m_BrowserMapMutex.ReleaseLock();

		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error saving session to DB") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::SaveSessionToDB - Error saving session to DB", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SaveSessions
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::SaveSessions(void)
{
	try
	{
		CMarkupSTL xmlgen;

		xmlgen.AddElem( _T("root") );

		GetSessionXML( xmlgen );
		
		xmlgen.AddChildElem( _T("Version"), s_Version );
		xmlgen.Save( _T("sessions.xml") );
	}
	catch(...)
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error saving sessions to File") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::SaveSessions - Error saving sessions to File", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// AddSession - add a new session to the map
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::AddSession( TCHAR* szSessionID, CAgentSession& session,
							  CBrowserSession& bsession )
{
	try
	{
		bool bNewBrowserSession = true;
		bool bRemoveOldSession = false;
		int nReplacedAgentID;

		GetLocalTime( &bsession.m_LastRequest );
		GetLocalTime( &bsession.m_CreateTime );

		// Add the browser session to the browser map
		if(m_BrowserMapMutex.AcquireLock(nMutexWait))
		{
			BROWSER_SESSION_MAP::iterator BrowserIter = m_BrowserMap.find( szSessionID );

			if( BrowserIter == m_BrowserMap.end() )
			{
				m_BrowserMap[ szSessionID ] = bsession;
			}
			else
			{
				bNewBrowserSession = false;
				// Session already exists
				nReplacedAgentID = BrowserIter->second.m_AgentID;
				if( nReplacedAgentID != bsession.m_AgentID )
				{
					bRemoveOldSession = true;
				}
				BrowserIter->second = bsession;
			}
			m_BrowserMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::AddSession - Could not aquire BrowserMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::AddSession - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
		}

		{
			// Add the agent session to the agent map
			if(m_AgentMapMutex.AcquireLock(nMutexWait))
			{
				map<unsigned int,CAgentSession>::iterator AgentIter = m_AgentMap.find( session.m_AgentID );

				if( AgentIter == m_AgentMap.end() )
				{
					memcpy( &session.m_CreateTime, &bsession.m_CreateTime, sizeof(SYSTEMTIME) );
					memcpy( &session.m_LastAutoAway, &bsession.m_CreateTime, sizeof(SYSTEMTIME) );
					m_AgentMap[ session.m_AgentID ] = session;
				}
				if( bRemoveOldSession )
				{
					TimeOutSession( nReplacedAgentID );
				}
				m_AgentMapMutex.ReleaseLock();
			}
			else
			{
				CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
				if(pRoutingEngine)
				{
					pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
						_T("CSessionMap::AddSession - Could not aquire AgentMapMutex.") );
				}
				DebugReporter::Instance().DisplayMessage("CSessionMap::AddSession - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
			}
		}
		
		if(nDBEnabled == 1)
		{
			SaveSessionToDB(szSessionID,session.m_AgentID,bNewBrowserSession);
		}
	}
	catch(...)
	{
		m_AgentMapMutex.ReleaseLock();
		m_BrowserMapMutex.ReleaseLock();
		
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error adding session") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::AddSession - Error adding session", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetSession - search for a session in the map
// 
////////////////////////////////////////////////////////////////////////////////
int CSessionMap::GetSession( TCHAR* szSessionID, CAgentSession& session, 
							 CBrowserSession& bsession, int nTicketBoxList )
{
	int retval = -1;
	dca::String s;
		
	try
	{
		DebugReporter::Instance().DisplayMessage("CSessionMap::GetSession - Entering", DebugReporter::ISAPI, GetCurrentThreadId());						

		s.Format("CSessionMap::GetSession - SessionID [%s]", szSessionID);
		DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

		if(m_BrowserMapMutex.AcquireLock(nMutexWait))
		{
		
			BROWSER_SESSION_MAP::iterator iter;

			for(iter = m_BrowserMap.begin();
				iter != m_BrowserMap.end();
				iter++)
			{
				//s.Format("CSessionMap::GetSession - Browser Map SessionID [%s] AgentID [%d] IP Address [%s]", iter->first.c_str(),(iter->second).m_AgentID,(iter->second).m_IP.c_str());
				//DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				if(!iter->first.compare(szSessionID))
				{
					if(nTicketBoxList != 1)
					{
						// Refresh the last request time
						s.Format("CSessionMap::GetSession - Setting last request time for Browser Map SessionID [%s] AgentID [%d]", iter->first.c_str(),(iter->second).m_AgentID);
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						GetLocalTime( &((iter->second).m_LastRequest) );
					}
					
					// Retrieve a copy of the session
					DebugReporter::Instance().DisplayMessage("CSessionMap::GetSession - Setting bsession", DebugReporter::ISAPI, GetCurrentThreadId());
					bsession = (iter->second);

					// set the return value to success
					retval = 0;
					break;
				}
			}
			if(retval == -1)
			{
				s.Format("CSessionMap::GetSession - Retval is -1 for SessionID [%s]", szSessionID);
				DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				if(nDBEnabled == 1)
				{
					//Session not found in Browser Sessions, check DB
					s.Format("CSessionMap::GetSession - SessionID [%s] not found in Browser Sessions, check the DB", szSessionID);
					DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				
					if(!m_db.IsConnected()){try{m_db.Connect();}catch(...){}}	
			
					TCHAR szIP[51] = {0};
					TCHAR szUserAgent[51] = {0};
					int nAgentID;
					TIMESTAMP_STRUCT lr;
					long lrLen;
					TIMESTAMP_STRUCT ct;
					long ctLen;
					int nTicketBoxViewID;
					
					m_query.Initialize();
					
					BINDPARAM_TCHAR( m_query, szSessionID );
					BINDCOL_TCHAR_NOLEN( m_query, szIP );
					BINDCOL_LONG_NOLEN( m_query, nAgentID );
					BINDCOL_TIME( m_query, lr );
					BINDCOL_TIME( m_query, ct );
					BINDCOL_TCHAR_NOLEN( m_query, szUserAgent );
					BINDCOL_LONG_NOLEN( m_query, nTicketBoxViewID );				
					m_query.Execute(  _T("SELECT bs.BrowserIP,bs.AgentID,bs.LastRequest,bs.CreateTime,bs.UserAgent,tbv.TicketBoxViewID ")
									_T("FROM BrowserSessions AS bs INNER JOIN TicketBoxViews AS tbv ON bs.AgentID=tbv.AgentID ")
									_T("WHERE bs.SessionID=? AND tbv.AgentBoxID=bs.AgentID AND tbv.TicketBoxViewTypeID=1") );
					
					if( m_query.Fetch() == S_OK )
					{
						s.Format("CSessionMap::GetSession - Browser Session for SessionID [%s] found in DB", szSessionID);
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				
						CBrowserSession bzsession;

						bzsession.m_AgentID = nAgentID;	
						TimeStampToSystemTime(lr, bzsession.m_LastRequest);
						TimeStampToSystemTime(ct, bzsession.m_CreateTime);
						bzsession.m_IP.assign(szIP);
						bzsession.m_UserAgent.assign(szUserAgent);

						CEMSString sURL;
						sURL.Format( _T("mytickets.ems?TicketBoxView=%d"), nTicketBoxViewID );
						bzsession.m_RightHandPane = sURL.c_str();

						s.Format("CSessionMap::GetSession - Adding Browser Session for SessionID [%s] to Browser Map", szSessionID);
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						m_BrowserMap[ szSessionID ] = bzsession;
						bsession = bzsession;					
						retval = 0;
					}
				}
			}
			m_BrowserMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::GetSession - Could not aquire BrowserMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::GetSession - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
		}
		

		if( retval == 0 )
		{
			retval = -2;
			
			s.Format("CSessionMap::GetSession - Checking Agent Map for AgentID [%d] SessionID [%s]", bsession.m_AgentID, szSessionID);
			DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				
			if(m_AgentMapMutex.AcquireLock(nMutexWait))
			{
				map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( bsession.m_AgentID );

				if( iter != m_AgentMap.end() )
				{
					s.Format("CSessionMap::GetSession - Found AgentID [%d] in Agent Map", bsession.m_AgentID);
					DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					if(nTicketBoxList == 0)
					{
						if(iter->second.m_PreAutoID >= 1)
						{
							SetAgentStatus(bsession.m_AgentID,iter->second.m_PreAutoID,iter->second.m_PreAutoText, false);												
						}
						GetLocalTime( &((iter->second).m_LastAutoAway) );
						iter->second.m_PreAutoID = 0;
						iter->second.m_PreAutoText.assign("");				
					}				
					
					// Retrieve a copy of the session
					DebugReporter::Instance().DisplayMessage("CSessionMap::GetSession - Setting session", DebugReporter::ISAPI, GetCurrentThreadId());
					session = (iter->second);

					// set the return value to success
					retval = 0;
				}
				else
				{
					if(nDBEnabled == 1)
					{
						s.Format("CSessionMap::GetSession - AgentID [%d] not found in Agent Sessions, check the DB", bsession.m_AgentID);
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						
						//Agent Session not found, check DB
						TCHAR szPreAutoText[126] = {0};
						int nPreAutoID;
						TIMESTAMP_STRUCT lr;
						long lrLen;
						TIMESTAMP_STRUCT ct;
						long ctLen;	

						m_query.Initialize();

						BINDPARAM_LONG( m_query, bsession.m_AgentID );
						BINDCOL_TIME( m_query, ct );
						BINDCOL_TIME( m_query, lr );
						BINDCOL_LONG_NOLEN( m_query, nPreAutoID );
						BINDCOL_TCHAR_NOLEN( m_query, szPreAutoText );	
						m_query.Execute(  _T("SELECT CreateTime,LastAuto,PreAutoID,PreAutoText ")
										_T("FROM AgentSessions WHERE AgentID=?") );
						
						if( m_query.Fetch() == S_OK )
						{
							s.Format("CSessionMap::GetSession - Agent Session for AgentID [%d] found in DB", bsession.m_AgentID);
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
							
							session.m_AgentID = bsession.m_AgentID;	
							TimeStampToSystemTime(lr, session.m_LastAutoAway);
							TimeStampToSystemTime(ct, session.m_CreateTime);
							session.m_PreAutoID = nPreAutoID;
							session.m_PreAutoText.assign(szPreAutoText);
							
							s.Format("CSessionMap::GetSession - Adding Agent Session for AgentID [%d] to Agent Map", bsession.m_AgentID);
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
							
							m_AgentMap[ session.m_AgentID ] = session;
							RefreshAgentSession( session, m_query, false);
						}

						map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( bsession.m_AgentID );

						if( iter != m_AgentMap.end() )
						{
							if(nTicketBoxList == 0)
							{
								if(iter->second.m_PreAutoID >= 1)
								{
									SetAgentStatus(bsession.m_AgentID,iter->second.m_PreAutoID,iter->second.m_PreAutoText, false);												
								}
								GetLocalTime( &((iter->second).m_LastAutoAway) );
								iter->second.m_PreAutoID = 0;
								iter->second.m_PreAutoText.assign("");
							}				
							
							// Retrieve a copy of the session
							session = (iter->second);

							// set the return value to success
							retval = 0;
						}
					}
				}
				m_AgentMapMutex.ReleaseLock();
			}
			else
			{
				CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
				if(pRoutingEngine)
				{
					pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
						_T("CSessionMap::GetSession - Could not aquire AgentMapMutex.") );
				}
				DebugReporter::Instance().DisplayMessage("CSessionMap::GetSession - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
			}		
		}
		
		/*if ( retval == 0 )
		{
			s.Format("CSessionMap::GetSession - Found AgentID [%d] in Agent Map and SessionID [%s] in Browser Map", bsession.m_AgentID,szSessionID);					
		}
		else if ( retval == -1 )
		{
			s.Format("CSessionMap::GetSession - Did not find SessionID [%s] in Browser Map", szSessionID);
		}
		else if ( retval == -2 )
		{
			s.Format("CSessionMap::GetSession - Found SessionID [%s] in Browser Map but did not find AgentID [%d] in Agent Map", szSessionID,bsession.m_AgentID);
		}
		DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
	}
	catch(...)
	{
		m_AgentMapMutex.ReleaseLock();
		m_BrowserMapMutex.ReleaseLock();
		
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error getting session") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::GetSession - Error getting session", DebugReporter::ISAPI, GetCurrentThreadId());
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetSessionIDFromIP - return the first sessionID from the same IP address
// 
////////////////////////////////////////////////////////////////////////////////
int CSessionMap::GetSessionIDFromIP(dca::String& sClientIP, int nAgentID, dca::String& sSessionID )
{
	int retval = -1;
	try
	{
		//dca::String s;
		
		//s.Format("CSessionMap::GetSessionIDFromIP - IP Address [%s] AgentID [%d]", sClientIP.c_str(), nAgentID);
		//DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
		if(m_BrowserMapMutex.AcquireLock(nMutexWait))
		{
			BROWSER_SESSION_MAP::iterator iter = m_BrowserMap.begin();

			while( iter != m_BrowserMap.end() )
			{
				//s.Format("CSessionMap::GetSessionIDFromIP - Browser Map SessionID [%s] AgentID [%d] IP Address [%s]", iter->first.c_str(),(iter->second).m_AgentID,(iter->second).m_IP.c_str());
				//DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
							
				if(    iter->second.m_AgentID == nAgentID 
					&& iter->second.m_IP.compare(sClientIP.c_str()) == 0 )
				{
					// Retrieve a copy of the sessionID
					sSessionID = (iter->first);

					// set the return value to success
					retval = 0;

					// break the loop
					iter = m_BrowserMap.end();
				}
				else
				{
					iter++;
				}
			}
			m_BrowserMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::GetSessionIDFromIP - Could not aquire BrowserMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::GetSessionIDFromIP - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
		}		

		/*if ( retval == 0 )
		{
			s.Format("CSessionMap::GetSessionIDFromIP - Found SessionID [%s] in Browser Map", sSessionID.c_str());					
		}
		else if ( retval == -1 )
		{
			s.Format("CSessionMap::GetSessionIDFromIP - Did not find SessionID in Browser Map");
		}
		
		DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());	*/
	
	}
	catch(...)
	{
		m_BrowserMapMutex.ReleaseLock();
		
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error getting SessionID from IP") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::GetSessionIDFromIP - Error getting SessionID from IP", DebugReporter::ISAPI, GetCurrentThreadId());
	}
	
	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetLoginTestResults
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::SetLoginTestResults( unsigned int AgentID, int nResults )
{
	try
	{
		if(m_AgentMapMutex.AcquireLock(nMutexWait))
		{		
			map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( AgentID );

			if( iter != m_AgentMap.end() )
			{
				iter->second.m_nLoginTestResult = nResults;
			}
			m_AgentMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::SetLoginTestResults - Could not aquire AgentMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::SetLoginTestResults - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
	catch(...)
	{
		m_AgentMapMutex.ReleaseLock();
		
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error setting login test results") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::SetLoginTestResults - Error setting login test results", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetRightHandPane
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::SetRightHandPane( tstring& sSessionID, LPCTSTR szURL )
{
	try
	{
		if(m_BrowserMapMutex.AcquireLock(nMutexWait))
		{
			BROWSER_SESSION_MAP::iterator iter = m_BrowserMap.find( sSessionID );

			if( iter != m_BrowserMap.end() )
			{
				iter->second.m_RightHandPane = szURL;
			}
			m_BrowserMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::SetRightHandPane - Could not aquire BrowserMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::SetRightHandPane - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
	catch(...)
	{
		m_BrowserMapMutex.ReleaseLock();
		
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error setting right hand pane") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::SetRightHandPane - Error getting setting right hand pane", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// SetDBMaintResults
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::SetDBMaintResults( unsigned int AgentID, int nResults )
{
	if(m_AgentMapMutex.AcquireLock(nMutexWait))
	{
		map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( AgentID );

		if( iter != m_AgentMap.end() )
		{
			iter->second.m_nDBMaintProgress = nResults;
		}
		m_AgentMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::SetDBMaintResults - Could not aquire AgentMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::SetDBMaintResults - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetAgentIP
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::GetAgentIP( unsigned int AgentID, tstring& sIP )
{
	if(m_AgentMapMutex.AcquireLock(nMutexWait))
	{
		map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( AgentID );

		if( iter != m_AgentMap.end() )
		{
			sIP = iter->second.m_AgentIP;
		}
		m_AgentMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::GetAgentIP - Could not aquire AgentMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::GetAgentIP - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// InvalidateAgentSession
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::InvalidateAgentSession( unsigned int AgentID )
{
	if(m_AgentMapMutex.AcquireLock(nMutexWait))
	{	
		map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( AgentID );

		if( iter != m_AgentMap.end() )
		{
			iter->second.m_bIsDirty = true;
		}
		m_AgentMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::InvalidateAgentSession - Could not aquire AgentMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::InvalidateAgentSession - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// InvalidateAllAgentSessions
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::InvalidateAllAgentSessions( void )
{
	if(m_AgentMapMutex.AcquireLock(nMutexWait))
	{	
		map<unsigned int,CAgentSession>::iterator iter;
		
		for ( iter = m_AgentMap.begin(); iter != m_AgentMap.end(); iter++ )
		{
			iter->second.m_bIsDirty = true;
		}
		m_AgentMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::InvalidateAllAgentSessions - Could not aquire AgentMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::InvalidateAllAgentSessions - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}

	LoadSessionSettings();	
}

////////////////////////////////////////////////////////////////////////////////
// 
// QueueSessionMonitors
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::QueueSessionMonitors( int nActionID, int nTargetID )
{
	if(nWSEnabled == 1)
	{
		if(vMonList.size() > 0)
		{
			for( vIter = vMonList.begin(); vIter != vMonList.end(); vIter++ )
			{
				int nSid = vIter->nServerID;
				int nPid = vIter->nProcessID;
				if(nActionID != 4 || (nActionID == 4 && nServerID != nSid))
				{
					m_query.Initialize();
					BINDPARAM_LONG( m_query, nSid );	
					BINDPARAM_LONG( m_query, nPid );	
					BINDPARAM_LONG( m_query, nActionID );
					BINDPARAM_LONG( m_query, nTargetID );
					m_query.Execute(_T("INSERT INTO SessionMonitorQueue (ServerID,ProcessID,ActionID,TargetID) VALUES (?,?,?,?)"));
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// RefreshAgentSession
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::RefreshAgentSession( CAgentSession& session, CODBCQuery& m_query, bool bLockAgent )
{
	// First refresh our copy of the session.
	session.Refresh( m_query );

	if(bLockAgent)
	{
		if(m_AgentMapMutex.AcquireLock(nMutexWait))
		{
			
			map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( session.m_AgentID );

			if( iter != m_AgentMap.end() )
			{	
				if( session.m_IsDeleted )
				{	
					// Remove the Agent Session
					m_AgentMap.erase( iter );

					// Remove all browser sessions with this AgentID			
					
					if(m_BrowserMapMutex.AcquireLock(nMutexWait))
					{
						BROWSER_SESSION_MAP::iterator browserIter = m_BrowserMap.begin();
					
						while( browserIter != m_BrowserMap.end() )
						{
							if( browserIter->second.m_AgentID == session.m_AgentID )
							{
								browserIter = m_BrowserMap.erase( browserIter );
							}
							else
							{
								browserIter++;
							}
						}
						m_BrowserMapMutex.ReleaseLock();
					}
					else
					{
						CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
						if(pRoutingEngine)
						{
							pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
								_T("CSessionMap::RefreshAgentSession - Could not aquire BrowserMapMutex with bLockAgent.") );
						}
						DebugReporter::Instance().DisplayMessage("CSessionMap::RefreshAgentSession - Could not aquire BrowserMapMutex with bLockAgent.", DebugReporter::ISAPI, GetCurrentThreadId());
					}
					
					if(m_ResetLocksMutex.AcquireLock(nMutexWait))
					{
						m_ResetLockList.push_back( session.m_AgentID );
						m_bNeedToResetLocks = true;
						m_ResetLocksMutex.ReleaseLock();
					}
					else
					{
						CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
						if(pRoutingEngine)
						{
							pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
								_T("CSessionMap::RefreshAgentSession - Could not aquire ResetLocksMutex with bLockAgent.") );
						}
						DebugReporter::Instance().DisplayMessage("CSessionMap::RefreshAgentSession - Could not aquire ResetLocksMutex with bLockAgent.", DebugReporter::ISAPI, GetCurrentThreadId());				
					}
				}
				else
				{
					// Refresh the session in the list
					iter->second = session;
				}
			}
			m_AgentMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::RefreshAgentSession - Could not aquire AgentMapMutex with bLockAgent.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::RefreshAgentSession - Could not aquire AgentMapMutex with bLockAgent.", DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
	else
	{
		if(m_AgentMapMutex.AcquireLock(nMutexWait))
		{
			map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( session.m_AgentID );

			if( iter != m_AgentMap.end() )
			{	
				if( session.m_IsDeleted )
				{	
					
					// Remove the Agent Session
					m_AgentMap.erase( iter );

					// Remove all browser sessions with this AgentID
					
					if(m_BrowserMapMutex.AcquireLock(nMutexWait))
					{					
						BROWSER_SESSION_MAP::iterator browserIter = m_BrowserMap.begin();
					
						while( browserIter != m_BrowserMap.end() )
						{
							if( browserIter->second.m_AgentID == session.m_AgentID )
							{
								browserIter = m_BrowserMap.erase( browserIter );
							}
							else
							{
								browserIter++;
							}
						}
						m_BrowserMapMutex.ReleaseLock();
					}
					else
					{
						CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
						if(pRoutingEngine)
						{
							pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
								_T("CSessionMap::RefreshAgentSession - Could not aquire BrowserMapMutex with !bLockAgent.") );
						}
						DebugReporter::Instance().DisplayMessage("CSessionMap::RefreshAgentSession - Could not aquire BrowserMapMutex with !bLockAgent.", DebugReporter::ISAPI, GetCurrentThreadId());
					}				

					if(m_ResetLocksMutex.AcquireLock(nMutexWait))
					{
						m_ResetLockList.push_back( session.m_AgentID );
						m_bNeedToResetLocks = true;
						m_ResetLocksMutex.ReleaseLock();
					}
					else
					{
						CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
						if(pRoutingEngine)
						{
							pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
								_T("CSessionMap::RefreshAgentSession - Could not aquire ResetLocksMutex with !bLockAgent.") );
						}
						DebugReporter::Instance().DisplayMessage("CSessionMap::RefreshAgentSession - Could not aquire ResetLocksMutex with !bLockAgent.", DebugReporter::ISAPI, GetCurrentThreadId());				
					}
				}
				else
				{
					// Refresh the session in the list
					iter->second = session;
				}
			}
			m_AgentMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::RefreshAgentSession - Could not aquire AgentMapMutex with !bLockAgent.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::RefreshAgentSession - Could not aquire AgentMapMutex with !bLockAgent.", DebugReporter::ISAPI, GetCurrentThreadId());
		}		
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// CreateNewSessionID 
//
// Note szSessionID must be at least SESSIONID_LENGTH+1 TCHARs
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::CreateNewSessionID( TCHAR* szSessionID )
{
	ZeroMemory( szSessionID, (SESSIONID_LENGTH+1) * sizeof(TCHAR) );

	// generate a random session ID
	for( int i = 0; i < SESSIONID_LENGTH; i++ )
	{
		szSessionID[i] = 'A' + rand() % 26;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// RemoveOldSessions
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::RemoveOldSessions( unsigned int nSessionTimeout )
{
	list<unsigned int> AgentIDsToCheck;
	list<unsigned int>::iterator iter2;
	SYSTEMTIME st;
	__int64 cutoff,last,automin,autolast;	
	GetLocalTime( &st );
	SystemTimeToFileTime( &st, (FILETIME*)&cutoff );
	CRoutingEngine*	pRoutingEngine = new CRoutingEngine;

	cutoff -= __int64(nSessionTimeout * 60)  * __int64( 10000000 );
	
	if(m_BrowserMapMutex.AcquireLock(nMutexWait))
	{	
		if(m_AgentMapMutex.AcquireLock(nMutexWait))
		{		
		
			BROWSER_SESSION_MAP::iterator iter = m_BrowserMap.begin();
			while( iter != m_BrowserMap.end() )
			{
				SystemTimeToFileTime( &(iter->second.m_LastRequest), (FILETIME*)&last );		

				int nAgentID = iter->second.m_AgentID;
				tstring sSessionID = iter->first;

				map<unsigned int,CAgentSession>::iterator iter3 = m_AgentMap.find( nAgentID );

				if( iter3 != m_AgentMap.end() )
				{
					SystemTimeToFileTime( &(iter3->second.m_LastAutoAway), (FILETIME*)&autolast );
					if(iter3->second.m_AgentAutoStatusTypeID != iter3->second.m_AgentStatusTypeID)
					{
						unsigned int nAutoMin = iter3->second.m_AgentAutoStatusMin;
						if(nAutoMin > 0)
						{
							SystemTimeToFileTime( &st, (FILETIME*)&automin );
							automin -= __int64(nAutoMin * 60)  * __int64( 10000000 );
							if(autolast < automin && iter3->second.m_AgentStatusTypeID == 1)
							{
								// Save the current status
								GetLocalTime( &(iter3->second.m_LastAutoAway) );				
								iter3->second.m_PreAutoID = iter3->second.m_AgentStatusTypeID;
								iter3->second.m_PreAutoText.assign(iter3->second.m_AgentStatusText);
								SetAgentStatus(nAgentID,iter3->second.m_AgentAutoStatusTypeID,iter3->second.m_AutoText, false);
							}
						}
					}
				}

				if((last < cutoff) && (nSessionTimeout > 0))
				{	
					iter = m_BrowserMap.erase( iter );
					DeleteBrowserSessionFromDB(sSessionID);			
					AgentIDsToCheck.push_back( nAgentID );

					if(pRoutingEngine)
					{
						XAgentNames name;
						g_ThreadPool.GetSharedObjects().XMLCache().m_AgentNames.Query( nAgentID, name );
							
						pRoutingEngine->LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
														EMSERR_ISAPI_EXTENSION,
														EMS_LOG_AUTHENTICATION,
														E_Logout),
												"%s was logged out of Session %s due to time out", 
												name.m_Name,sSessionID.c_str() );
					}
				}
				else
				{
					iter++;
				}
			}
			m_AgentMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::RemoveOldSessions - Could not aquire AgentMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::RemoveOldSessions - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
		}

		for( iter2 = AgentIDsToCheck.begin(); iter2 != AgentIDsToCheck.end(); iter2++ )
		{
			if(nSessionExpireAll == 1)
			{
				RemoveAllSessionsForAgent( *iter2 );
				if(pRoutingEngine)
				{
					XAgentNames name;
					g_ThreadPool.GetSharedObjects().XMLCache().m_AgentNames.Query( *iter2, name );
						
					pRoutingEngine->LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
													EMSERR_ISAPI_EXTENSION,
													EMS_LOG_AUTHENTICATION,
													E_Logout),
											"%s was logged out of all Sessions due to one session timing out", 
											name.m_Name );
				}
			}
			else
			{
				// Check if there are more Browser Sessions for this Agent
				BROWSER_SESSION_MAP::iterator BrowserIter;
				bool bFound=false;
				for ( BrowserIter = m_BrowserMap.begin(); BrowserIter != m_BrowserMap.end(); BrowserIter++ )
				{
					if(BrowserIter->second.m_AgentID == *iter2)
					{
						bFound=true;
						break;
					}
				}
				
				if(!bFound)
				{
					if(pRoutingEngine)
					{
						XAgentNames name;
						g_ThreadPool.GetSharedObjects().XMLCache().m_AgentNames.Query( *iter2, name );
							
						pRoutingEngine->LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
														EMSERR_ISAPI_EXTENSION,
														EMS_LOG_AUTHENTICATION,
														E_Logout),
												"%s was logged out of all Sessions", 
												name.m_Name );
					}
					TimeOutSession( *iter2 );
				}
			}
		}
		m_BrowserMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::RemoveOldSessions - Could not aquire BrowserMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::RemoveOldSessions - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}

	RemoveOldBrowserSessionsFromDB();
}

////////////////////////////////////////////////////////////////////////////////
// 
// LogAgentAction
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::LogAgentAction( CODBCQuery& m_query, long nAgentID, long nActionID )
{
	m_query.Initialize();
	BINDPARAM_LONG( m_query, nAgentID );
	BINDPARAM_LONG( m_query, nActionID );
	m_query.Execute(_T("INSERT INTO AgentActivityLog (AgentID, ActivityID) VALUES (?, ?)"));	
}

////////////////////////////////////////////////////////////////////////////////
// 
// TimeOutSession
// 
////////////////////////////////////////////////////////////////////////////////
bool CSessionMap::TimeOutSession( unsigned int AgentID, bool bLockAgent )
{
	bool bRet = false;
	if(!m_db.IsConnected()){try{m_db.Connect();}catch(...){}}
	CRoutingEngine*	pRoutingEngine = new CRoutingEngine;

	if(bLockAgent)
	{
		if(m_AgentMapMutex.AcquireLock(nMutexWait))
		{
			map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( AgentID );

			if( iter != m_AgentMap.end() )
			{
				int nLogoutStatusID = iter->second.m_LogoutStatusID;
				TCHAR szStatusText[AGENTS_STATUS_TEXT_LENGTH] = {0};
				long szStatusTextLen = 0;
				_tcscpy( szStatusText, iter->second.m_LogoutText.c_str() );

				if(nLogoutStatusID < 0)
				{
					DebugReporter::Instance().DisplayMessage("CSessionMap::TimeOutSession - nLogoutStatusID < 0", DebugReporter::ISAPI, GetCurrentThreadId());	
				}
		
				m_query.Initialize();
				BINDPARAM_LONG( m_query, nLogoutStatusID );
				BINDPARAM_TCHAR( m_query, szStatusText );
				BINDPARAM_LONG( m_query, AgentID );	
				m_query.Execute(_T("UPDATE Agents SET StatusID=?,StatusText=? WHERE AgentID=?"));
			
				if(nDBEnabled == 1)
				{
					DeleteAgentSessionFromDB(AgentID);
				}
				m_AgentMap.erase( iter );
				bRet = true;

				// add the agent to the list of agents whose
				// locks must be reset...
				if(m_ResetLocksMutex.AcquireLock(nMutexWait))
				{
					m_ResetLockList.push_back( AgentID );
					m_bNeedToResetLocks = true;
					m_ResetLocksMutex.ReleaseLock();
				}
				else
				{
					CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
					if(pRoutingEngine)
					{
						pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
							_T("CSessionMap::TimeOutSession - Could not aquire ResetLocksMutex lock with bLockAgent.") );
					}
					DebugReporter::Instance().DisplayMessage("CSessionMap::TimeOutSession - Could not aquire ResetLocksMutex lock with bLockAgent.", DebugReporter::ISAPI, GetCurrentThreadId());				
				}
			}

			m_AgentMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::TimeOutSession - Could not aquire AgentMapMutex with bLockAgent.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::TimeOutSession - Could not aquire AgentMapMutex with bLockAgent.", DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
	else
	{
		if(m_AgentMapMutex.AcquireLock(nMutexWait))
		{
			map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( AgentID );

			if( iter != m_AgentMap.end() )
			{
				int nLogoutStatusID = iter->second.m_LogoutStatusID;
				TCHAR szStatusText[AGENTS_STATUS_TEXT_LENGTH] = {0};
				long szStatusTextLen = 0;
				_tcscpy( szStatusText, iter->second.m_LogoutText.c_str() );
		
				if(nLogoutStatusID < 0)
				{
					DebugReporter::Instance().DisplayMessage("CSessionMap::TimeOutSession - nLogoutStatusID < 0", DebugReporter::ISAPI, GetCurrentThreadId());	
				}
		
				m_query.Initialize();
				BINDPARAM_LONG( m_query, nLogoutStatusID );
				BINDPARAM_TCHAR( m_query, szStatusText );
				BINDPARAM_LONG( m_query, AgentID );	
				m_query.Execute(_T("UPDATE Agents SET StatusID=?,StatusText=? WHERE AgentID=?"));
			
				if(nDBEnabled == 1)
				{
					DeleteAgentSessionFromDB(AgentID);
				}
				m_AgentMap.erase( iter );
				bRet = true;

				// add the agent to the list of agents whose
				// locks must be reset...
				if(m_ResetLocksMutex.AcquireLock(nMutexWait))
				{
					m_ResetLockList.push_back( AgentID );
					m_bNeedToResetLocks = true;
					m_ResetLocksMutex.ReleaseLock();
				}
				else
				{
					CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
					if(pRoutingEngine)
					{
						pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
							_T("CSessionMap::TimeOutSession - Could not aquire ResetLocksMutex lock with !bLockAgent.") );
					}
					DebugReporter::Instance().DisplayMessage("CSessionMap::TimeOutSession - Could not aquire ResetLocksMutex lock with !bLockAgent.", DebugReporter::ISAPI, GetCurrentThreadId());				
				}				
			}
			m_AgentMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::TimeOutSession - Could not aquire AgentMapMutex with !bLockAgent.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::TimeOutSession - Could not aquire AgentMapMutex with !bLockAgent.", DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
	return bRet;
}

void CSessionMap::DeleteAgentSessionFromDB( unsigned int AgentID )
{
	m_query.Initialize();
	BINDPARAM_LONG( m_query, AgentID );
	m_query.Execute(_T("DELETE FROM AgentSessions WHERE AgentID=?"));
}

void CSessionMap::DeleteBrowserSessionFromDB( tstring& sessionID )
{
	m_query.Initialize();
	BINDPARAM_TCHAR_STRING( m_query, sessionID );
	m_query.Execute(_T("DELETE FROM BrowserSessions WHERE SessionID=?"));
}

void CSessionMap::RemoveOldBrowserSessionsFromDB(void)  
{
	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );
					
	m_query.Initialize();	
	BINDPARAM_LONG(m_query, nSessionTimeOut);		
	BINDPARAM_TIME_NOLEN( m_query, now );
	m_query.Execute(  _T("DELETE FROM BrowserSessions WHERE LastRequest < DATEADD(minute,-?,?) OR ")
					_T("BrowserSessionID < (SELECT MAX(BrowserSessionID) FROM BrowserSessions B2 WHERE B2.SessionID = BrowserSessions.SessionID)"));
	
	RemoveOldAgentSessionsFromDB();
}

void CSessionMap::RemoveOldAgentSessionsFromDB(void)
{
	m_query.Initialize();
	m_query.Execute(  _T("DELETE FROM AgentSessions WHERE AgentID NOT IN (SELECT DISTINCT AgentID FROM BrowserSessions) OR ")
					_T("AgentSessionID < ( SELECT MAX(AgentSessionID) FROM AgentSessions A2 WHERE A2.AgentID = AgentSessions.AgentID)"));
}

////////////////////////////////////////////////////////////////////////////////
// 
// RemoveSession
// 
////////////////////////////////////////////////////////////////////////////////
bool CSessionMap::RemoveSession( tstring& sSessionID, bool bFromSessionList, int AgentID )
{
	int nAgentID = 0;
	CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
	bool bFound=false;
		

	if(m_BrowserMapMutex.AcquireLock(nMutexWait))
	{		
		// First remove the browser session
		{
			BROWSER_SESSION_MAP::iterator iter = m_BrowserMap.find( sSessionID );

			if( iter != m_BrowserMap.end() )
			{
				nAgentID = iter->second.m_AgentID;			
				DeleteBrowserSessionFromDB(sSessionID);
				m_BrowserMap.erase( iter );

				if(bFromSessionList)
				{
					if(pRoutingEngine)
					{
						XAgentNames name;
						XAgentNames name2;
						g_ThreadPool.GetSharedObjects().XMLCache().m_AgentNames.Query( nAgentID, name );
						g_ThreadPool.GetSharedObjects().XMLCache().m_AgentNames.Query( AgentID, name2 );
						pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_Logout),
						_T("%s was logged out of Session %s by %s"), name.m_Name,sSessionID.c_str(),name2.m_Name );
					}
				}
			}		
		}

		// Check if there are more Browser Sessions for this Agent
		BROWSER_SESSION_MAP::iterator BrowserIter;
		
		for ( BrowserIter = m_BrowserMap.begin(); BrowserIter != m_BrowserMap.end(); BrowserIter++ )
		{
			if(BrowserIter->second.m_AgentID == nAgentID)
			{
				bFound=true;
				break;
			}
		}
		m_BrowserMapMutex.ReleaseLock();
	}
	else
	{
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::RemoveSession - Could not aquire BrowserMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::RemoveSession - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}

	if(!bFound)
	{
		return TimeOutSession( nAgentID, true );
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RemoveAllSessionsForAgent
// 
////////////////////////////////////////////////////////////////////////////////
bool CSessionMap::RemoveAllSessionsForAgent( unsigned int nAgentID )
{
	list<tstring> RemoveSessionList;
	CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
	
	if(m_BrowserMapMutex.AcquireLock(nMutexWait))
	{		
		// First remove the browser sessions
		BROWSER_SESSION_MAP::iterator BrowserIter;
		
		for ( BrowserIter = m_BrowserMap.begin(); BrowserIter != m_BrowserMap.end(); BrowserIter++ )
		{
			if(BrowserIter->second.m_AgentID == nAgentID)
			{
				tstring sSessionID(BrowserIter->first.c_str());
				RemoveSessionList.push_back(sSessionID);
			}
		}

		list<tstring>::iterator iter;
		for( iter = RemoveSessionList.begin(); iter != RemoveSessionList.end(); iter++ )
		{
			BROWSER_SESSION_MAP::iterator iter2 = m_BrowserMap.find( *iter );

			if( iter2 != m_BrowserMap.end() )
			{
				tstring sSessionID(iter2->first);
				DeleteBrowserSessionFromDB(*iter);
				m_BrowserMap.erase( iter2 );
				if(pRoutingEngine)
				{
					XAgentNames name;
					g_ThreadPool.GetSharedObjects().XMLCache().m_AgentNames.Query( nAgentID, name );
						
					pRoutingEngine->LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
													EMSERR_ISAPI_EXTENSION,
													EMS_LOG_AUTHENTICATION,
													E_Logout),
											"%s was logged out of Session %s", 
											name.m_Name,sSessionID.c_str() );
				}
			}
		}
		m_BrowserMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::RemoveAllSessionsForAgent - Could not aquire BrowserMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::RemoveAllSessionsForAgent - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}

	return TimeOutSession( nAgentID, true );
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetSessionXML
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::GetSessionXML( CMarkupSTL& xmlgen )
{
	BROWSER_SESSION_MAP::iterator BrowserIter;
	map<unsigned int,CAgentSession>::iterator AgentIter;
	FILETIME ft;
	CEMSString sTime;
	
	if(m_BrowserMapMutex.AcquireLock(nMutexWait))
	{

		for ( BrowserIter = m_BrowserMap.begin(); BrowserIter != m_BrowserMap.end(); BrowserIter++ )
		{
			xmlgen.AddChildElem( _T("BrowserSession") );
			xmlgen.AddChildAttrib( _T("ID"), BrowserIter->first.c_str() );
			xmlgen.AddChildAttrib( _T("AgentID"), BrowserIter->second.m_AgentID );
			
			SystemTimeToFileTime( &(BrowserIter->second.m_LastRequest), &ft);
			xmlgen.AddChildAttrib( _T("LastRequestHigh"), ft.dwHighDateTime );
			xmlgen.AddChildAttrib( _T("LastRequestLow"), ft.dwLowDateTime );

			SystemTimeToFileTime( &(BrowserIter->second.m_CreateTime), &ft);
			xmlgen.AddChildAttrib( _T("CreateTimeHigh"), ft.dwHighDateTime );
			xmlgen.AddChildAttrib( _T("CreateTimeLow"), ft.dwLowDateTime );

			xmlgen.AddChildAttrib( _T("IP"), BrowserIter->second.m_IP.c_str() );
			xmlgen.AddChildAttrib( _T("UserAgent"), BrowserIter->second.m_UserAgent.c_str() );
		
			GetDateTimeString( BrowserIter->second.m_LastRequest, sTime );
			xmlgen.AddChildAttrib( _T("LastRequest"), sTime.c_str() );

			GetDateTimeString( BrowserIter->second.m_CreateTime, sTime );
			xmlgen.AddChildAttrib( _T("CreateTime"), sTime.c_str() );

			xmlgen.SetChildData(  BrowserIter->second.m_RightHandPane.c_str() );
		}
		m_BrowserMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::GetSessionXML - Could not aquire BrowserMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::GetSessionXML - Could not aquire BrowserMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
	

	{
		// Save agent session map to disk
		if(m_AgentMapMutex.AcquireLock(nMutexWait))
		{
			for ( AgentIter = m_AgentMap.begin(); AgentIter != m_AgentMap.end(); AgentIter++ )
			{
				int nStatusID = AgentIter->second.m_AgentAutoStatusTypeID;
				tstring sStatusText(AgentIter->second.m_AgentStatusText);

				xmlgen.AddChildElem( _T("AgentSession") );
				xmlgen.AddChildAttrib( _T("AgentID"), AgentIter->second.m_AgentID );
				xmlgen.AddChildAttrib( _T("IsAdmin"), AgentIter->second.m_IsAdmin );
				
				SystemTimeToFileTime( &(AgentIter->second.m_CreateTime ), &ft);
				xmlgen.AddChildAttrib( _T("CreateTimeHigh"), ft.dwHighDateTime );
				xmlgen.AddChildAttrib( _T("CreateTimeLow"), ft.dwLowDateTime );
				
				SystemTimeToFileTime( &(AgentIter->second.m_LastAutoAway), &ft);
				xmlgen.AddChildAttrib( _T("LastAutoHigh"), ft.dwHighDateTime );
				xmlgen.AddChildAttrib( _T("LastAutoLow"), ft.dwLowDateTime );
				
				xmlgen.AddChildAttrib( _T("AgentUserName"), AgentIter->second.m_AgentUserName.c_str() );
				xmlgen.AddChildAttrib( _T("EscalateToAgentID"), AgentIter->second.m_EscalateToAgentID );
				xmlgen.AddChildAttrib( _T("OutboundApprover"), AgentIter->second.m_bIsOutboundApprover );
				xmlgen.AddChildAttrib( _T("MaxRowsPerPage"), AgentIter->second.m_nMaxRowsPerPage );
				xmlgen.AddChildAttrib( _T("IP"), AgentIter->second.m_AgentIP.c_str() );
				xmlgen.AddChildAttrib( _T("StatusID"), AgentIter->second.m_AgentStatusTypeID );
				xmlgen.AddChildAttrib( _T("StatusText"), AgentIter->second.m_AgentStatusText.c_str() );
				xmlgen.AddChildAttrib( _T("AutoStatusTypeID"), AgentIter->second.m_AgentAutoStatusTypeID );
				xmlgen.AddChildAttrib( _T("AutoStatusMin"), AgentIter->second.m_AgentAutoStatusMin );
				xmlgen.AddChildAttrib( _T("LogoutStatusID"), AgentIter->second.m_LogoutStatusID );
				xmlgen.AddChildAttrib( _T("AutoText"), AgentIter->second.m_AutoText.c_str() );
				xmlgen.AddChildAttrib( _T("LogoutText"), AgentIter->second.m_LogoutText.c_str() );
				xmlgen.AddChildAttrib( _T("PreAutoID"), AgentIter->second.m_PreAutoID );
				xmlgen.AddChildAttrib( _T("PreAutoText"), AgentIter->second.m_PreAutoText.c_str() );			
				
				GetDateTimeString( AgentIter->second.m_CreateTime, sTime );
				xmlgen.AddChildAttrib( _T("CreateTime"), sTime.c_str() );

				GetDateTimeString( AgentIter->second.m_LastAutoAway, sTime );
				xmlgen.AddChildAttrib( _T("LastAutoRequest"), sTime.c_str() );

				xmlgen.IntoElem();
				xmlgen.AddChildSubDoc( AgentIter->second.m_AgentXML.c_str() );
				xmlgen.OutOfElem();
			}
			m_AgentMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::GetSessionXML - Could not aquire AgentMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::GetSessionXML - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}

	// add information about locks which still need to be reset...
	if ( m_bNeedToResetLocks )
	{
		if(m_ResetLocksMutex.AcquireLock(nMutexWait))
		{
			if ( m_ResetLockList.size() > 0 )
			{
				list<unsigned int>::iterator iter;
				
				for( iter = m_ResetLockList.begin(); iter != m_ResetLockList.end(); iter++ )
				{
					xmlgen.AddChildElem( _T("ResetLock") );
					xmlgen.AddChildAttrib( _T("AgentID"), *iter );
				}
			}
			else
			{
				xmlgen.AddChildElem( _T("ResetAllLocks") );
			}
			m_ResetLocksMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::GetSessionXML - Could not aquire ResetLocksMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::GetSessionXML - Could not aquire ResetLocksMutex.", DebugReporter::ISAPI, GetCurrentThreadId());				
		}		
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Reset locks for the agentIDs contained in m_ResetLockList.  
||				The worker thread (which has the database connection) 
||				checks to see if locks need to be reset before each job.	              
\*--------------------------------------------------------------------------*/
void CSessionMap::ResetLocks( CODBCQuery& query )
{
	CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
	
	if (!m_bNeedToResetLocks) 
		return;
	
	m_bNeedToResetLocks = false;
	
	if ( m_ResetLockList.size() > 0 )
	{
		if(m_ResetLocksMutex.AcquireLock(nMutexWait))
		{
			CEMSString sAgentIDs, sChunk;
			TCHAR szBuf[32];

			// Log the results
			tstring sAal;
			tstring sAaa;
			long nSpid;
			char m_szValue[255];

			query.Initialize();
			BINDCOL_LONG_NOLEN( query, nSpid );
			BINDCOL_CHAR_NOLEN( query, m_szValue );
			query.Execute( _T("SELECT ServerParameterID,DataValue ")
						_T("FROM ServerParameters ")
						_T("WHERE ServerParameterID=48 OR ServerParameterID=49") );

			while( query.Fetch() == S_OK )
			{
				if (nSpid == 48)
				{
					sAal=m_szValue;
				}
				else
				{
					sAaa=m_szValue;
				}
			}
			
			// build a comma delimited list of agentIDs
			list<unsigned int>::iterator iter;
			for( iter = m_ResetLockList.begin(); iter != m_ResetLockList.end(); iter++ )
			{
				if( _ttoi( sAal.c_str() ) > 0 )
				{
					if( _ttoi( sAaa.c_str() ) > 0 )
					{
						LogAgentAction( query, *iter, 2 );
					}
				}
				_itot( *iter, szBuf, 10 );
				sAgentIDs.append( szBuf );
				sAgentIDs.append( _T(",") );
			}

			while( sAgentIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
			{
				FreeReservedTicketIDs( query, sChunk );
				FreeReservedMsgIDs( query, sChunk );
				UnlockTickets( query, sChunk );
				
				// log that we are resetting locks
				if(pRoutingEngine)
				{
					pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_ResetLock),
						_T("Reset Locks for AgentID (%s)"), sChunk.c_str() );
				}
				
			}

			// clear the list
			m_ResetLockList.clear();
			m_ResetLocksMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::ResetLocks - Could not aquire ResetLocksMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::ResetLocks - Could not aquire ResetLocksMutex.", DebugReporter::ISAPI, GetCurrentThreadId());				
		}
	}
	else
	{
		FreeReservedTicketIDs( query );
		FreeReservedMsgIDs( query );
		UnlockTickets( query );

		// log that we are resetting locks for all agents
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_ResetLock),
				_T("Reset locks for all agents") );
		}
	}	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Unlock tickets locked by a CDL of Agents.  If no agentIDs 
||				are specified, all tickets will be unlocked.
\*--------------------------------------------------------------------------*/
void CSessionMap::UnlockTickets( CODBCQuery& query, tstring sAgentIDs )
{
	CEMSString sQuery;
	CEMSString sWhere;

	if ( sAgentIDs.length() )
	{
		sWhere.Format( _T("!= 0 AND LockedTime>GetDate()-1 AND LockedBy IN (%s)"), sAgentIDs.c_str() );
	}
	else
	{
		sWhere = _T("!= 0");
	}
	
	query.Initialize();
	sQuery.Format( _T("UPDATE Tickets SET LockedBy=0 WHERE LockedBy %s"), sWhere.c_str() );
	query.Execute( sQuery.c_str() );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Delete "Reserved" Ticket IDs for a CDL of AgentsIDs.
||				If no agentIDs are specified, all reserved tickets will
||				be deleted.
\*--------------------------------------------------------------------------*/
void CSessionMap::FreeReservedTicketIDs( CODBCQuery& query, tstring sAgentIDs )
{
	CEMSString sQuery;
	CEMSString sWhere;

	if ( sAgentIDs.length() )
		sWhere.Format( _T("AND OwnerID IN(%s)"), sAgentIDs.c_str() );

	query.Initialize();
	sQuery.Format( _T("DELETE FROM TicketNotesRead WHERE TicketNoteID IN")
				   _T("(SELECT TicketNoteID FROM TicketNotes WHERE TicketID IN")
				   _T("(SELECT TicketID FROM Tickets WHERE IsDeleted=3 %s))"), sWhere.c_str() );
	query.Execute( sQuery.c_str() );
	
	query.Initialize();
	sQuery.Format( _T("DELETE FROM TicketNotes WHERE TicketID IN")
				   _T("(SELECT TicketID FROM Tickets WHERE IsDeleted=3 %s)"), sWhere.c_str() );
	query.Execute( sQuery.c_str() );

	query.Reset();
	sQuery.Format( _T("DELETE FROM TicketFieldsTicket WHERE TicketID IN")
		           _T("(SELECT TicketID FROM Tickets WHERE IsDeleted=3 %s)"), sWhere.c_str() );
	query.Execute( sQuery.c_str() );
	
	query.Reset();
	sQuery.Format( _T("DELETE FROM Tickets WHERE TicketID IN")
		           _T("(SELECT TicketID FROM Tickets WHERE IsDeleted=3 %s)"), sWhere.c_str() );
	query.Execute( sQuery.c_str() );	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Delete "Reserved" Message IDs for a CDL of AgentsIDs.
||				If no AgentIDs are specified, all reserved messages will
||				be deleted.
\*--------------------------------------------------------------------------*/
void CSessionMap::FreeReservedMsgIDs( CODBCQuery& query, tstring sAgentIDs )
{
	CEMSString sQuery;
	CEMSString sWhere;
	TAttachments attach;
	tstring sFullAttachPath;
	list<unsigned int> AttachmentIDList;
	list<unsigned int>::iterator iter;

	if ( sAgentIDs.length() )
		sWhere.Format( _T("AND AgentID IN(%s)"), sAgentIDs.c_str() );

	// get a list of all the attachment IDs associated with the reserved messages
	query.Initialize();
	BINDCOL_LONG_NOLEN( query, attach.m_AttachmentID );
	sQuery.Format( _T("SELECT A.AttachmentID FROM Attachments as A ")
				   _T("INNER JOIN OutboundMessageAttachments as O on A.AttachmentID = O.AttachmentID ")
				   _T("WHERE OutboundMessageID IN (SELECT OutboundMessageID FROM OutboundMessages ")
				   _T("WHERE IsDeleted=3 %s)"), sWhere.c_str() );
	query.Execute( sQuery.c_str() );

	while( query.Fetch() == S_OK )
	{
		AttachmentIDList.push_back( attach.m_AttachmentID );
	}
	
	// delete records from OutboundMessageAttachments
	query.Reset();
	sQuery.Format( _T("DELETE FROM OutboundMessageAttachments ")
		           _T("WHERE OutboundMessageID IN (SELECT OutboundMessageID FROM OutboundMessages ")
				   _T("WHERE IsDeleted=3 %s)"), sWhere.c_str() );
	query.Execute( sQuery.c_str() );

	// for each attachment
	for ( iter = AttachmentIDList.begin(); iter != AttachmentIDList.end(); iter++ )
	{
		// if the file is no longer referenced, delete it from the disk
		// and remove the record in the Attachments table
		if ( GetAttachmentReferenceCount( query, *iter ) == 0 )
		{
			attach.m_AttachmentID = *iter;
			attach.Query( query );
			
			attach.m_IsInbound ? GetFullInboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath ) :
			                     GetFullOutboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath );
			
			DeleteFile( sFullAttachPath.c_str() );
			
			attach.Delete( query );
		}
	}
	
	// delete records from OutboundMessageQueue
	query.Reset();
	sQuery.Format( _T("DELETE FROM OutboundMessageQueue ")
		           _T("WHERE OutboundMessageID IN (SELECT OutboundMessageID FROM OutboundMessages ")
				   _T("WHERE IsDeleted=3 %s)"), sWhere.c_str() );
	query.Execute( sQuery.c_str() );

	// delete records from the OutboundMessages table
	query.Reset();
	sQuery.Format( _T("DELETE FROM OutboundMessages ")
				   _T("WHERE OutboundMessageID IN (SELECT OutboundMessageID FROM OutboundMessages ")
			       _T("WHERE IsDeleted=3 %s)"), sWhere.c_str() );
	query.Execute( sQuery.c_str() );
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetTempType
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::SetTempType( unsigned int AgentID, int nTempType )
{
	if(m_AgentMapMutex.AcquireLock(nMutexWait))
	{	
		map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( AgentID );

		if( iter != m_AgentMap.end() )
		{
			iter->second.m_TempType = nTempType;
		}
		m_AgentMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::SetTempType - Could not aquire AgentMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::SetTempType - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetTempData
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::SetTempData( unsigned int AgentID, tstring sTempData )
{
	if(m_AgentMapMutex.AcquireLock(nMutexWait))
	{
		map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( AgentID );

		if( iter != m_AgentMap.end() )
		{
			iter->second.m_TempData = sTempData;
		}
		m_AgentMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::SetTempData - Could not aquire AgentMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::SetTempData - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetTempType
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::GetTempType( unsigned int AgentID, int& nTempType )
{
	if(m_AgentMapMutex.AcquireLock(nMutexWait))
	{	
		map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( AgentID );

		if( iter != m_AgentMap.end() )
		{
			nTempType = iter->second.m_TempType;
		}
		m_AgentMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::GetTempType - Could not aquire AgentMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::GetTempType - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetTempData
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::GetTempData( unsigned int AgentID, tstring& sTempData )
{
	if(m_AgentMapMutex.AcquireLock(nMutexWait))
	{
		map<unsigned int,CAgentSession>::iterator iter = m_AgentMap.find( AgentID );

		if( iter != m_AgentMap.end() )
		{
			sTempData = iter->second.m_TempData;
		}
		m_AgentMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::GetTempData - Could not aquire AgentMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::GetTempData - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetAgentStatus
// 
////////////////////////////////////////////////////////////////////////////////
int CSessionMap::GetAgentStatus( unsigned int AgentID, tstring& sStatusText, int& nLoggedIn )
{
	int nStatusType=0;
	nLoggedIn = 0;
	//dca::String f;

	try
	{
		if(m_StatusMapMutex.AcquireLock(nMutexWait))
		{
			map<unsigned int,CAgentStatus>::iterator iter = m_StatusMap.find( AgentID );
			if( iter != m_StatusMap.end() )
			{
				//f.Format("CSessionMap::GetAgentStatus - AgentID [%d] found in StatusMap", AgentID);
				//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

				nStatusType = iter->second.m_AgentStatusTypeID;
				sStatusText = iter->second.m_AgentStatusText;
				nLoggedIn = iter->second.m_LoggedIn;
			}
			else
			{
				//f.Format("CSessionMap::GetAgentStatus - AgentID [%d] not found in StatusMap, returning Offline", AgentID);
				//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				
				nStatusType = 5;
				sStatusText.assign(_T("Offline"));		
			}
			m_StatusMapMutex.ReleaseLock();
		}
		else
		{
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			if(pRoutingEngine)
			{
				pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
					_T("CSessionMap::GetAgentStatus - Could not aquire StatusMapMutex.") );
			}
			DebugReporter::Instance().DisplayMessage("CSessionMap::GetAgentStatus - Could not aquire StatusMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
		}

		return nStatusType;
	}
	catch(...)
	{
		m_StatusMapMutex.ReleaseLock();

		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("Error Getting Agent Status") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::LoadStatus - Error Getting Agent Status", DebugReporter::ISAPI, GetCurrentThreadId());

		nLoggedIn=0;
		return 5;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetAgentStatus
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::SetAgentStatus( unsigned int AgentID, unsigned int nStatusID, tstring sStatusText, bool bLockAgent )
{
	bool bUpdate = false;
	dca::String f;
	
	f.Format("CSessionMap::SetAgentStatus - Set Status for AgentID [%d] StatusID [%d] and StatusText [%d]", AgentID, nStatusID, sStatusText.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	if(m_StatusMapMutex.AcquireLock(nMutexWait))
	{
		map<unsigned int,CAgentStatus>::iterator iter = m_StatusMap.find( AgentID );
		if( iter != m_StatusMap.end() )
		{
			if(nStatusID < 0){nStatusID=1;sStatusText.assign(_T("Online"));}
			
			f.Format("CSessionMap::SetAgentStatus - AgentID [%d] found in StatusMap, updating", AgentID);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			
			if(iter->second.m_AgentStatusTypeID != nStatusID)
			{
				iter->second.m_AgentStatusTypeID = nStatusID;
				bUpdate = true;
			}
			if(iter->second.m_AgentStatusText != sStatusText)
			{
				iter->second.m_AgentStatusText = sStatusText;
				bUpdate = true;
			}
		}
		else
		{
			CAgentStatus status;
					
			if(nStatusID < 0){nStatusID=1;sStatusText.assign(_T("Online"));}
			
			status.m_AgentID = AgentID;
			status.m_AgentStatusTypeID = nStatusID;
			status.m_AgentStatusText.assign(sStatusText);
		
			f.Format("CSessionMap::SetAgentStatus - AgentID [%d] not found in StatusMap, adding now", AgentID);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			
			m_StatusMap[ status.m_AgentID ] = status;		
			bUpdate = true;
		}

		if(bUpdate)
		{
			f.Format("CSessionMap::SetAgentStatus - Status for AgentID [%d] has changed, Updating DB", AgentID);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

			TCHAR szStatusText[AGENTS_STATUS_TEXT_LENGTH] = {0};
			long szStatusTextLen = 0;
			_tcscpy( szStatusText, sStatusText.c_str() );
			
			m_query.Initialize();
			BINDPARAM_LONG( m_query, nStatusID );
			BINDPARAM_TCHAR( m_query, szStatusText );
			BINDPARAM_LONG( m_query, AgentID );	
			m_query.Execute(_T("UPDATE Agents SET StatusID=?,StatusText=? WHERE AgentID=?"));
			
			if(m_AgentMapMutex.AcquireLock(nMutexWait))
			{
				if(bLockAgent)
				{				
					map<unsigned int,CAgentSession>::iterator AgentIter = m_AgentMap.find( AgentID );
					if( AgentIter != m_AgentMap.end() )
					{
						f.Format("CSessionMap::SetAgentStatus - AgentID [%d] found in AgentMap, updating", AgentID);
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						
						AgentIter->second.m_AgentStatusTypeID = nStatusID;
						AgentIter->second.m_AgentStatusText.assign(sStatusText);
					}				
				}
				else
				{
					map<unsigned int,CAgentSession>::iterator AgentIter = m_AgentMap.find( AgentID );
					if( AgentIter != m_AgentMap.end() )
					{
						f.Format("CSessionMap::SetAgentStatus - AgentID [%d] found in AgentMap, updating", AgentID);
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						
						AgentIter->second.m_AgentStatusTypeID = nStatusID;
						AgentIter->second.m_AgentStatusText.assign(sStatusText);
					}
				}
				m_AgentMapMutex.ReleaseLock();
			}
			else
			{
				CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
				if(pRoutingEngine)
				{
					pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
						_T("CSessionMap::SetAgentStatus - Could not aquire AgentMapMutex.") );
				}
				DebugReporter::Instance().DisplayMessage("CSessionMap::SetAgentStatus - Could not aquire AgentMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
			}
		}
		m_StatusMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CSessionMap::SetAgentStatus - Could not aquire StatusMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CSessionMap::SetAgentStatus - Could not aquire StatusMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// LoadSessionSettings
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::LoadSessionSettings(void)
{
	long nSpid;
	char m_szValue[255];
	
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nSpid );
	BINDCOL_CHAR_NOLEN( m_query, m_szValue );
	m_query.Execute(_T("SELECT ServerParameterID,DataValue FROM ServerParameters WHERE ServerParameterID IN (96,132,133,134,135,136,137,138,141,146,148)"));
	while( m_query.Fetch() == S_OK )
	{
		dca::String sTemp(m_szValue);
		switch(nSpid)			
		{
		case 96:
			nSessionTimeOut = sTemp.ToInt();
			break;
		case 132:
			nWSEnabled = sTemp.ToInt();
			break;
		case 133:
			nSessionFreq = sTemp.ToInt();
			break;
		case 134:
			nStatusFreq = sTemp.ToInt();
			break;
		case 135:
			nDBFreq = sTemp.ToInt();
			break;
		case 136:
			nDBEnabled = sTemp.ToInt();
			break;
		case 137:
			nQueueFull = sTemp.ToInt();
			break;
		case 138:
			nQueueSize = sTemp.ToInt();
			break;
		case 141:
			nCharSet = sTemp.ToInt();
			break;
		case 146:
			nSessionExpireAll = sTemp.ToInt();
			break;
		case 148:
			nTimeZone = sTemp.ToInt();
			break;
		}
	}
	
	int nTicketSubject=0;
	int nInboundSubject=0;
	int nInboundBody=0;
	int nOutboundSubject=0;
	int nOutboundBody=0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nTicketSubject );
	BINDCOL_LONG_NOLEN( m_query, nInboundSubject );
	BINDCOL_LONG_NOLEN( m_query, nInboundBody );
	BINDCOL_LONG_NOLEN( m_query, nOutboundSubject );
	BINDCOL_LONG_NOLEN( m_query, nOutboundBody );
	m_query.Execute( _T("SELECT COLUMNPROPERTY(OBJECT_ID('Tickets'), 'Subject', 'IsFulltextIndexed'),")
					 _T("COLUMNPROPERTY(OBJECT_ID('InboundMessages'), 'Subject', 'IsFulltextIndexed'),")
                     _T("COLUMNPROPERTY(OBJECT_ID('InboundMessages'), 'Body', 'IsFulltextIndexed'),")
                     _T("COLUMNPROPERTY(OBJECT_ID('OutboundMessages'), 'Subject', 'IsFulltextIndexed'),")
                     _T("COLUMNPROPERTY(OBJECT_ID('OutboundMessages'), 'Body', 'IsFulltextIndexed')" ));

	if ( m_query.Fetch() == S_OK )
	{
		if(nTicketSubject==1 && nInboundSubject==1 && nInboundBody==1 && nOutboundSubject==1 && nOutboundBody==1)
		{
			bFullText = true;			
		}
	}
	
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               EMS_SERVER_ID_VALUE, nServerID ) != ERROR_SUCCESS)
	{
		nServerID = 1;
	}
	
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               _T("CacheRefresh"), nCacheRefresh ) != ERROR_SUCCESS)
	{
		nCacheRefresh = 0;
	}
	
	nProcessID = GetCurrentProcessId();
	
	if(nWSEnabled == 1)
	{
		nDBEnabled = 1;		
	}		
}

////////////////////////////////////////////////////////////////////////////////
// 
// LoadSessionMonitors
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::LoadSessionMonitors(void)
{
	vMonList.clear();
	
	if(nWSEnabled == 1)
	{
		TIMESTAMP_STRUCT now;
		GetTimeStamp( now );
		m_query.Initialize();
		BINDPARAM_LONG( m_query, nServerID );
		BINDPARAM_LONG( m_query, nProcessID );
		BINDPARAM_TIME_NOLEN( m_query, now );
		BINDCOL_LONG_NOLEN( m_query, m_MonitorList.nSessionMonitorID );
		BINDCOL_LONG_NOLEN( m_query, m_MonitorList.nServerID );
		BINDCOL_LONG_NOLEN( m_query, m_MonitorList.nProcessID );
		m_query.Execute(_T("SELECT SessionMonitorID,ServerID,ProcessID FROM SessionMonitors WHERE SessionMonitorID NOT IN ")
						_T("(SELECT SessionMonitorID FROM SessionMonitors WHERE ServerID=? AND ProcessID=?) AND CheckIn > DATEADD(minute,-10,?)"));
		while( m_query.Fetch() == S_OK )
		{
			vMonList.push_back(m_MonitorList);
		}		
	}		
}

////////////////////////////////////////////////////////////////////////////////
// 
// CloseDatabase
// 
////////////////////////////////////////////////////////////////////////////////
void CSessionMap::CloseDatabase(void)
{
	if( m_db.IsConnected() )
	{
		m_query.Close();
		m_db.Disconnect();
	}	
}