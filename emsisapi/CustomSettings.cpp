#include "stdafx.h"
#include ".\CustomSettings.h"
#include ".\MailFlowServer.h"
#include "TBVInfo.h"

CustomSettings::CustomSettings(CISAPIData& ISAPIData)
	:CXMLDataClass(ISAPIData)
{
	nMaxBypassID=0;
	nMaxEmailID=0;
}

CustomSettings::~CustomSettings(void)
{
}

int CustomSettings::Run(CURLAction& action)
{
	// Check security
	RequireAdmin();

	std::string sAction;
	if( !GetISAPIData().GetXMLString( _T("action"), sAction, true ))
	{
		GetISAPIData().GetXMLString("action", sAction, true);
	}
	

	if(!lstrcmpi(sAction.c_str(),"update"))
	{
		UpdateSettings();
	}
	else if(!lstrcmpi(sAction.c_str(),"ctas"))
	{
		UpdateCTAS();		
	}
	else if(!lstrcmpi(sAction.c_str(),"rrtm"))
	{
		UpdateRRTM();		
	}
	else if(!lstrcmpi(sAction.c_str(),"applyag"))
	{
		return ApplyAgent();		
	}
	else if(!lstrcmpi(sAction.c_str(),"applytb"))
	{
		return ApplyTicketBox();		
	}
	else if(!lstrcmpi(sAction.c_str(),"applyrr"))
	{
		return ApplyRoutingRule();		
	}
	else if(!lstrcmpi(sAction.c_str(),"applyms"))
	{
		return ApplyMessageSource();		
	}
	else if(!lstrcmpi(sAction.c_str(),"applymd"))
	{
		return ApplyMessageDestination();		
	}
	else if(!lstrcmpi(sAction.c_str(),"addexc"))
	{
		AddException();		
	}
	else if(!lstrcmpi(sAction.c_str(),"delexc"))
	{
		DeleteException();		
	}

	if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("bulksettings") ) == 0 )
	{
		return BulkSettings(action);
	}
	else
	{
		return GetSettings(action);
	}
}

int CustomSettings::GetSettings(CURLAction& action)
{
	ListEnabledAgentNames();
	ListGroupNames();

	GetXMLGen().AddChildElem( _T("BypassTypes") );
	GetXMLGen().IntoElem();	
	GetXMLGen().AddChildElem( _T("BypassType") );
	GetXMLGen().AddChildAttrib( _T("ID"), 0 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Email Address") );
	GetXMLGen().AddChildElem( _T("BypassType") );
	GetXMLGen().AddChildAttrib( _T("ID"), 1 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Domain") );	
	GetXMLGen().OutOfElem();

	// Set the max BypassID	
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN(GetQuery(), nMaxBypassID );
	GetQuery().Execute( _T("SELECT CASE WHEN  MAX(BypassID) IS NULL THEN 0 ELSE MAX(BypassID) END FROM Bypass") );
	GetQuery().Fetch();

	b.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("Bypass") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Item") );
		GetXMLGen().AddChildAttrib( _T("ID"), b.m_BypassID );
		GetXMLGen().AddChildAttrib( _T("TypeID"), b.m_BypassTypeID );
		GetXMLGen().AddChildAttrib( _T("Value"), b.m_BypassValue );
	}
	GetXMLGen().OutOfElem();

	TTimeZones tz;
	tz.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("TimeZones") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TimeZone") );
		GetXMLGen().AddChildAttrib( _T("ID"), tz.m_TimeZoneID );
		GetXMLGen().AddChildAttrib( _T("DisplayName"), tz.m_DisplayName );		
	}
	GetXMLGen().OutOfElem();

	TDictionary dc;
	dc.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("Dictionaries") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Dictionary") );
		GetXMLGen().AddChildAttrib( _T("ID"), dc.m_DictionaryID );
		GetXMLGen().AddChildAttrib( _T("Description"), dc.m_Description );
		GetXMLGen().AddChildAttrib( _T("TlxFile"), dc.m_TlxFile );
		GetXMLGen().AddChildAttrib( _T("ClxFile"), dc.m_ClxFile );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), dc.m_IsEnabled );
	}
	GetXMLGen().OutOfElem();
	
	GetXMLGen().AddChildElem( _T("EmailTypes") );
	GetXMLGen().IntoElem();	
	GetXMLGen().AddChildElem( _T("EmailType") );
	GetXMLGen().AddChildAttrib( _T("ID"), 0 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Email Address") );
	GetXMLGen().AddChildElem( _T("EmailType") );
	GetXMLGen().AddChildAttrib( _T("ID"), 1 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Domain") );	
	GetXMLGen().OutOfElem();

	GetXMLGen().AddChildElem( _T("AutoEmailTypes") );
	GetXMLGen().IntoElem();	
	GetXMLGen().AddChildElem( _T("EmailType") );
	GetXMLGen().AddChildAttrib( _T("ID"), 2 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Email Address") );
	GetXMLGen().AddChildElem( _T("EmailType") );
	GetXMLGen().AddChildAttrib( _T("ID"), 3 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Domain") );	
	GetXMLGen().OutOfElem();

	// Set the max EmailID	
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN(GetQuery(), nMaxEmailID );
	GetQuery().Execute( _T("SELECT CASE WHEN  MAX(EmailID) IS NULL THEN 0 ELSE MAX(EmailID) END FROM Email") );
	GetQuery().Fetch();

	TBypass tB;	
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tB.m_BypassID );
	BINDCOL_LONG( GetQuery(), tB.m_BypassTypeID );
	BINDCOL_TCHAR( GetQuery(), tB.m_BypassValue );
	BINDCOL_TIME( GetQuery(), tB.m_DateCreated );
	GetQuery().Execute( _T("SELECT EmailID,EmailTypeID,EmailValue,DateCreated ")
	               _T("FROM Email ")
	               _T("ORDER BY EmailValue") );

	GetXMLGen().AddChildElem( _T("Email") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Item") );
		GetXMLGen().AddChildAttrib( _T("ID"), tB.m_BypassID );
		GetXMLGen().AddChildAttrib( _T("TypeID"), tB.m_BypassTypeID );
		GetXMLGen().AddChildAttrib( _T("Value"), tB.m_BypassValue );
	}
	GetXMLGen().OutOfElem();

	// Delete old Office Hour Exceptions
	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );
	GetQuery().Initialize();
	BINDPARAM_TIME_NOLEN( GetQuery(), now );
	GetQuery().Execute( _T("DELETE FROM OfficeHours ")
						_T("WHERE OfficeHourID>7 AND TypeID<>0 AND TimeEnd < ?") );
	
	
	TOfficeHours oh;
	CEMSString sDate;
	oh.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("Exceptions") );
	GetXMLGen().IntoElem();
	while(GetQuery().Fetch() == S_OK)
	{
		if ( oh.m_TypeID == 1 ||
			 oh.m_TypeID == 2 ||
			 oh.m_TypeID == 4 ||
			 oh.m_TypeID == 5 ||
			 oh.m_TypeID == 7 ||
			 oh.m_TypeID == 8 ||
			 oh.m_TypeID == 10 ||
			 oh.m_TypeID == 11)
		{
			GetXMLGen().AddChildElem( _T("Exception") );
			GetXMLGen().AddChildAttrib( _T("ID"), oh.m_OfficeHourID );
			GetXMLGen().AddChildAttrib( _T("TypeID"), oh.m_TypeID );
			GetXMLGen().AddChildAttrib( _T("Description"), oh.m_Description );
			GetDateTimeString( oh.m_TimeStart, oh.m_TimeStartLen, sDate );
			GetXMLGen().AddChildAttrib( _T("TimeStart"), sDate.c_str() );
			GetDateTimeString( oh.m_TimeEnd, oh.m_TimeEndLen, sDate );
			GetXMLGen().AddChildAttrib( _T("TimeEnd"), sDate.c_str() );			
		}
	}
	GetXMLGen().OutOfElem();

	GetXMLGen().AddChildElem(_T("customsettings"));

	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(_T("BypassMaxID"), nMaxBypassID);
	GetXMLGen().OutOfElem();

	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(_T("EmailMaxID"), nMaxEmailID);
	GetXMLGen().OutOfElem();

	SetXMLOutput(29, _T("5"), _T("MaxISAPI"));
	SetXMLOutput(20, _T("5"), _T("MaxPOP3"));
	SetXMLOutput(21, _T("5"), _T("MaxSMTP"));

	SetXMLOutput(40, _T("60"), _T("dbtimeout"));
	SetXMLOutput(41, _T("0"), _T("usecufoffreaddate"));
	SetXMLOutput(42, _T(""), _T("cufoffreaddate"));

	SetXMLOutput(43, _T("0"), _T("enableMyTickets"));
	SetXMLOutput(44, _T("0"), _T("enablePublicTicketBoxes"));
	SetXMLOutput(45, _T("0"), _T("enableAgentViews"));

	SetXMLOutput(47, _T("0"), _T("enableScheduledReports"));

	SetXMLOutput(48, _T("0"), _T("enableAgentActivity"));
	SetXMLOutput(49, _T("0"), _T("enableAgentAuth"));
	SetXMLOutput(50, _T("0"), _T("enableTicketDelete"));
	SetXMLOutput(51, _T("0"), _T("enableTicketCreate"));
	SetXMLOutput(52, _T("0"), _T("enableTicketMove"));
	SetXMLOutput(53, _T("0"), _T("enableTicketEscalate"));
	SetXMLOutput(54, _T("0"), _T("enableTicketReassign"));
	SetXMLOutput(55, _T("0"), _T("enableTicketRestore"));
	SetXMLOutput(56, _T("0"), _T("enableTicketOpen"));
	SetXMLOutput(57, _T("0"), _T("enableTicketClose"));
	SetXMLOutput(58, _T("0"), _T("enableTicketMessageAdd"));
	SetXMLOutput(59, _T("0"), _T("enableTicketMessageDelete"));
	SetXMLOutput(66, _T("60"), _T("maxTicketLockTime"));
	SetXMLOutput(67, _T("0"), _T("ticketListDate"));
	SetXMLOutput(84, _T("0"), _T("enableTicketMessageRevoke"));
	SetXMLOutput(85, _T("0"), _T("enableTicketMessageRelease"));
	SetXMLOutput(86, _T("0"), _T("enableTicketMessageReturn"));
	SetXMLOutput(87, _T("0"), _T("PlugIn3cxVersion"));
	SetXMLOutput(88, _T("0"), _T("MailFlowClientVersion"));
	SetXMLOutput(90, _T("0"), _T("TicketTracking"));
	SetXMLOutput(93, _T("0"), _T("enableGetOldestSearch"));
	SetXMLOutput(94, _T("0"), _T("CloseTicketAfterSend"));
	SetXMLOutput(95, _T("0"), _T("AutoRouteToInbox"));
	SetXMLOutput(96, _T("20"), _T("SessionTimeout"));
	SetXMLOutput(97, _T("0"), _T("chkKeepAlive"));
	SetXMLOutput(98, _T("0"), _T("chkOnline"));
	SetXMLOutput(99, _T("0"), _T("chkAway"));
	SetXMLOutput(100, _T("0"), _T("chkNotAvail"));
	SetXMLOutput(101, _T("0"), _T("chkOffline"));
	SetXMLOutput(102, _T("0"), _T("chkOoo"));
	SetXMLOutput(103, _T("1"), _T("chkAdminOr"));
	SetXMLOutput(104, _T("1"), _T("chkAgentOr"));
	SetXMLOutput(113, _T("1"), _T("enableTicketLinkCreated"));
	SetXMLOutput(114, _T("1"), _T("enableTicketLinkDeleted"));
	SetXMLOutput(115, _T("1"), _T("enableTicketLinked"));
	SetXMLOutput(116, _T("1"), _T("enableTicketUnlinked"));
	SetXMLOutput(117, _T("0"), _T("DateEntryFormat"));
	SetXMLOutput(119, _T("0"), _T("EmailVerifyDays"));
	SetXMLOutput(120, _T("0"), _T("EmailVerifyMx"));
	SetXMLOutput(121, _T("0"), _T("EmailVerifyCatchAll"));
	SetXMLOutput(122, _T("0"), _T("EmailVerifyRemove"));
	SetXMLOutput(123, _T("0"), _T("EmailVerifyDelete"));
	SetXMLOutput(124, _T("0"), _T("EmailVerifyDecrement"));
	SetXMLOutput(125, _T("0"), _T("EmailVerifyIncrement"));
	SetXMLOutput(126, _T(""), _T("HeloHostName"));
	SetXMLOutput(127, _T("10"), _T("ConnectTimeout"));
	SetXMLOutput(128, _T("30"), _T("UpdateFrequency"));
	SetXMLOutput(129, _T("0"), _T("enableTicketChangeCategory"));
	SetXMLOutput(130, _T("0"), _T("enableTicketChangeField"));
	SetXMLOutput(131, _T("0"), _T("enableVerification"));
	SetXMLOutput(132, _T("0"), _T("ShareAgentSessions"));
	SetXMLOutput(133, _T("30"), _T("SessionFreq"));
	SetXMLOutput(134, _T("30"), _T("StatusFreq"));
	SetXMLOutput(135, _T("60"), _T("DBSessionFreq"));
	SetXMLOutput(136, _T("0"), _T("DbAgentSessions"));
	SetXMLOutput(137, _T("0"), _T("QueueFull"));
	SetXMLOutput(138, _T("32"), _T("QueueSize"));
	//SetXMLOutput(141, _T("0"), _T("Charset"));
	SetXMLOutput(142, _T("0"), _T("MsgDate"));
	SetXMLOutput(143, _T("0"), _T("AgentEmailBypass"));
	SetXMLOutput(144, _T("1"), _T("RequireDate"));
	SetXMLOutput(145, _T("0"), _T("chkLogoffAll"));
	SetXMLOutput(146, _T("0"), _T("chkExpireAll"));
	SetXMLOutput(147, _T("0"), _T("chkAllowLogoff"));
	SetXMLOutput(148, _T("0"), _T("TimeZoneID"));
	SetXMLOutput(149, _T("0"), _T("DictionaryID"));

	oh.m_OfficeHourID=1;
	oh.Query( GetQuery() );
	GetXMLGen().AddChildAttrib( _T("SunHrStart"), oh.m_StartHr );
	GetXMLGen().AddChildAttrib( _T("SunMinStart"), oh.m_StartMin );
	GetXMLGen().AddChildAttrib( _T("SunAmPmStart"), oh.m_StartAmPm );
	GetXMLGen().AddChildAttrib( _T("SunHrEnd"), oh.m_EndHr );
	GetXMLGen().AddChildAttrib( _T("SunMinEnd"), oh.m_EndMin );
	GetXMLGen().AddChildAttrib( _T("SunAmPmEnd"), oh.m_EndAmPm );
	
	oh.m_OfficeHourID=2;
	oh.Query( GetQuery() );
	GetXMLGen().AddChildAttrib( _T("MonHrStart"), oh.m_StartHr );
	GetXMLGen().AddChildAttrib( _T("MonMinStart"), oh.m_StartMin );
	GetXMLGen().AddChildAttrib( _T("MonAmPmStart"), oh.m_StartAmPm );
	GetXMLGen().AddChildAttrib( _T("MonHrEnd"), oh.m_EndHr );
	GetXMLGen().AddChildAttrib( _T("MonMinEnd"), oh.m_EndMin );
	GetXMLGen().AddChildAttrib( _T("MonAmPmEnd"), oh.m_EndAmPm );
	
	oh.m_OfficeHourID=3;
	oh.Query( GetQuery() );
	GetXMLGen().AddChildAttrib( _T("TueHrStart"), oh.m_StartHr );
	GetXMLGen().AddChildAttrib( _T("TueMinStart"), oh.m_StartMin );
	GetXMLGen().AddChildAttrib( _T("TueAmPmStart"), oh.m_StartAmPm );
	GetXMLGen().AddChildAttrib( _T("TueHrEnd"), oh.m_EndHr );
	GetXMLGen().AddChildAttrib( _T("TueMinEnd"), oh.m_EndMin );
	GetXMLGen().AddChildAttrib( _T("TueAmPmEnd"), oh.m_EndAmPm );
	
	oh.m_OfficeHourID=4;
	oh.Query( GetQuery() );
	GetXMLGen().AddChildAttrib( _T("WedHrStart"), oh.m_StartHr );
	GetXMLGen().AddChildAttrib( _T("WedMinStart"), oh.m_StartMin );
	GetXMLGen().AddChildAttrib( _T("WedAmPmStart"), oh.m_StartAmPm );
	GetXMLGen().AddChildAttrib( _T("WedHrEnd"), oh.m_EndHr );
	GetXMLGen().AddChildAttrib( _T("WedMinEnd"), oh.m_EndMin );
	GetXMLGen().AddChildAttrib( _T("WedAmPmEnd"), oh.m_EndAmPm );
	
	oh.m_OfficeHourID=5;
	oh.Query( GetQuery() );
	GetXMLGen().AddChildAttrib( _T("ThuHrStart"), oh.m_StartHr );
	GetXMLGen().AddChildAttrib( _T("ThuMinStart"), oh.m_StartMin );
	GetXMLGen().AddChildAttrib( _T("ThuAmPmStart"), oh.m_StartAmPm );
	GetXMLGen().AddChildAttrib( _T("ThuHrEnd"), oh.m_EndHr );
	GetXMLGen().AddChildAttrib( _T("ThuMinEnd"), oh.m_EndMin );
	GetXMLGen().AddChildAttrib( _T("ThuAmPmEnd"), oh.m_EndAmPm );
	
	oh.m_OfficeHourID=6;
	oh.Query( GetQuery() );
	GetXMLGen().AddChildAttrib( _T("FriHrStart"), oh.m_StartHr );
	GetXMLGen().AddChildAttrib( _T("FriMinStart"), oh.m_StartMin );
	GetXMLGen().AddChildAttrib( _T("FriAmPmStart"), oh.m_StartAmPm );
	GetXMLGen().AddChildAttrib( _T("FriHrEnd"), oh.m_EndHr );
	GetXMLGen().AddChildAttrib( _T("FriMinEnd"), oh.m_EndMin );
	GetXMLGen().AddChildAttrib( _T("FriAmPmEnd"), oh.m_EndAmPm );
	
	oh.m_OfficeHourID=7;
	oh.Query( GetQuery() );
	GetXMLGen().AddChildAttrib( _T("SatHrStart"), oh.m_StartHr );
	GetXMLGen().AddChildAttrib( _T("SatMinStart"), oh.m_StartMin );
	GetXMLGen().AddChildAttrib( _T("SatAmPmStart"), oh.m_StartAmPm );
	GetXMLGen().AddChildAttrib( _T("SatHrEnd"), oh.m_EndHr );
	GetXMLGen().AddChildAttrib( _T("SatMinEnd"), oh.m_EndMin );
	GetXMLGen().AddChildAttrib( _T("SatAmPmEnd"), oh.m_EndAmPm );

	return 0;
}

int CustomSettings::BulkSettings(CURLAction& action)
{
	ListAgentNames();
	ListGroupNames();
	ListStyleSheetNames();
	ListTicketBoxNames(GetXMLGen());
	ListRoutingRules();
	ListMessageSources();
	ListMessageDestinations();
	ListTicketBoxOwners();
	return 0;
}

int CustomSettings::ApplyAgent(void)
{
	list<int> id;
	list<int>::iterator idIter;
	int nAgentID = -1;
	int nGroupID = -1;
	CEMSString sGroups;
	CEMSString sAgents;

	sAgents.Format(_T(""));
	GetISAPIData().GetXMLString( _T("AgentID"), sAgents, true );	
	sAgents.CDLInit();
	id.clear();
	while( sAgents.CDLGetNextInt( nAgentID ) )
	{
		if(nAgentID == 0)
		{
			break;
		}		
	}

	if(nAgentID != 0)
	{
		//Get Groups		
		sGroups.Format(_T(""));
		GetISAPIData().GetXMLString( _T("GroupID"), sGroups, true );
		sGroups.CDLInit();
		while( sGroups.CDLGetNextInt( nGroupID ) )
		{
			if(nGroupID == 0 || nGroupID == 1)
			{
				break;
			}		
		}
	}

	if(nAgentID == 0 || nGroupID == 0 || nGroupID == 1)
	{
		//Update all Agents
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nAgentID );		
		GetQuery().Execute( _T("SELECT AgentID ")
							_T("FROM Agents ")
							_T("WHERE IsDeleted=0") );
		while ( GetQuery().Fetch() == S_OK )
		{
			id.push_back(nAgentID);			
		}		
	}
	else
	{
		//Update Agents that are in sAgents and in Groups in sGroups
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nAgentID );		
		BINDPARAM_TCHAR_STRING( GetQuery(), sAgents );
		BINDPARAM_TCHAR_STRING( GetQuery(), sGroups );
		GetQuery().Execute( _T("SELECT DISTINCT AgentID ")
							_T("FROM Agents ")
							_T("WHERE AgentID IN (?) OR AgentID IN (SELECT DISTINCT AgentID FROM AgentGroupings WHERE GroupID IN (?))") );
		while ( GetQuery().Fetch() == S_OK )
		{
			id.push_back(nAgentID);			
		}			
	}

	//Get the settings that need to be udpated
	TAgents tempAgent;

	GetISAPIData().GetXMLLong( _T("AgentStateID"), tempAgent.m_IsEnabled );
	GetISAPIData().GetXMLLong( _T("GetOldestID"), tempAgent.m_RequireGetOldest );
		
	GetISAPIData().GetXMLLong( _T("MaxReportRowsPerPage"), tempAgent.m_MaxReportRowsPerPage );
	GetISAPIData().GetXMLLong( _T("QuoteMsgInReply"), tempAgent.m_QuoteMsgInReply );
	GetISAPIData().GetXMLLong( _T("StyleSheetID"), tempAgent.m_StyleSheetID );
	GetISAPIData().GetXMLTCHAR( _T("ReplyQuotedPrefix"), tempAgent.m_ReplyQuotedPrefix, 3 );
	GetISAPIData().GetXMLLong( _T("NewMessageFormat"), tempAgent.m_NewMessageFormat );
	GetISAPIData().GetXMLLong( _T("UseAutoFill"), tempAgent.m_UseAutoFill );
	GetISAPIData().GetXMLLong( _T("ForceSpellCheck"), tempAgent.m_ForceSpellCheck );
	GetISAPIData().GetXMLLong( _T("SignatureTopReply"), tempAgent.m_SignatureTopReply );
	GetISAPIData().GetXMLLong( _T("UsePreviewPane"), tempAgent.m_UsePreviewPane );
	GetISAPIData().GetXMLLong( _T("ShowMessages"), tempAgent.m_ShowMessagesInbound );
	GetISAPIData().GetXMLLong( _T("AutoRouteToInbox"), tempAgent.m_RouteToInbox );
	GetISAPIData().GetXMLLong( _T("CloseTicketAfterSend"), tempAgent.m_CloseTicket );
	GetISAPIData().GetXMLLong( _T("OutboxHoldTime"), tempAgent.m_OutboxHoldTime );
	GetISAPIData().GetXMLLong( _T("txtMarkMsgRead"), tempAgent.m_MarkAsReadSeconds );
	if(tempAgent.m_MarkAsReadSeconds > 0)
	{
		tempAgent.m_UseMarkAsRead = 1;
	}
	GetISAPIData().GetXMLLong( _T("DefaultTicketBoxID"), tempAgent.m_DefaultTicketBoxID );
	GetISAPIData().GetXMLLong( _T("TicketDblClick"), tempAgent.m_DefaultTicketDblClick );
	GetISAPIData().GetXMLLong( _T("ReadReceipt"), tempAgent.m_ReadReceipt );
	GetISAPIData().GetXMLLong( _T("DeliveryConfirmation"), tempAgent.m_DeliveryConfirmation );

	GetISAPIData().GetXMLLong( _T("DefaultOnlineStatus"), tempAgent.m_LoginStatusID );
	GetISAPIData().GetXMLLong( _T("DefaultOfflineStatus"), tempAgent.m_LogoutStatusID );
	GetISAPIData().GetXMLLong( _T("AutoStatusTypeID"), tempAgent.m_AutoStatusTypeID );
	GetISAPIData().GetXMLLong( _T("AutoStatusMin"), tempAgent.m_AutoStatusMin );
	GetISAPIData().GetXMLTCHAR( _T("OnlineText"), tempAgent.m_OnlineText, AGENTS_STATUS_TEXT_LENGTH );
	GetISAPIData().GetXMLTCHAR( _T("AwayText"), tempAgent.m_AwayText, AGENTS_STATUS_TEXT_LENGTH );
	GetISAPIData().GetXMLTCHAR( _T("NotAvailText"), tempAgent.m_NotAvailText, AGENTS_STATUS_TEXT_LENGTH );
	GetISAPIData().GetXMLTCHAR( _T("OfflineText"), tempAgent.m_OfflineText, AGENTS_STATUS_TEXT_LENGTH );
	GetISAPIData().GetXMLTCHAR( _T("OooText"), tempAgent.m_OooText, AGENTS_STATUS_TEXT_LENGTH );
	
	CTBVInfo m_TBView;
	GetISAPIData().GetXMLLong( _T("ShowState"), m_TBView.m_ShowState);
	GetISAPIData().GetXMLLong( _T("ShowPriority"), m_TBView.m_ShowPriority);
	GetISAPIData().GetXMLLong( _T("ShowNumNotes"), m_TBView.m_ShowNumNotes);
	GetISAPIData().GetXMLLong( _T("ShowTicketID"), m_TBView.m_ShowTicketID);
	GetISAPIData().GetXMLLong( _T("ShowNumMsgs"), m_TBView.m_ShowNumMsgs);
	GetISAPIData().GetXMLLong( _T("ShowSubject"), m_TBView.m_ShowSubject);
	GetISAPIData().GetXMLLong( _T("ShowContact"), m_TBView.m_ShowContact);
	GetISAPIData().GetXMLLong( _T("ShowDate"), m_TBView.m_ShowDate);
	GetISAPIData().GetXMLLong( _T("ShowCategory"), m_TBView.m_ShowCategory);
	GetISAPIData().GetXMLLong( _T("ShowOwner"), m_TBView.m_ShowOwner);
	GetISAPIData().GetXMLLong( _T("ShowTicketBox"), m_TBView.m_ShowTicketBox);

	for( idIter = id.begin(); idIter != id.end(); idIter++ )
	{
		try
		{
			TAgents agent;
			agent.m_AgentID = *idIter;
			if(agent.m_AgentID > 1)
			{
				agent.Query(GetQuery());
				int nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_state"),nVal);
				if(nVal)
				{
					agent.m_IsEnabled = tempAgent.m_IsEnabled;
				}		
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_getoldest"),nVal);
				if(nVal)
				{
					agent.m_RequireGetOldest = tempAgent.m_RequireGetOldest;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_stylesht"),nVal);
				if(nVal)
				{
					agent.m_StyleSheetID = tempAgent.m_StyleSheetID;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_maxrows"),nVal);
				if(nVal)
				{
					agent.m_MaxReportRowsPerPage = tempAgent.m_MaxReportRowsPerPage;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_dblclk"),nVal);
				if(nVal)
				{
					agent.m_DefaultTicketDblClick = tempAgent.m_DefaultTicketDblClick;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_showmsg"),nVal);
				if(nVal)
				{
					agent.m_ShowMessagesInbound = tempAgent.m_ShowMessagesInbound;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_preview"),nVal);
				if(nVal)
				{
					agent.m_UsePreviewPane = tempAgent.m_UsePreviewPane;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_markread"),nVal);
				if(nVal)
				{
					agent.m_UseMarkAsRead = tempAgent.m_UseMarkAsRead;
					agent.m_MarkAsReadSeconds = tempAgent.m_MarkAsReadSeconds;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_quote"),nVal);
				if(nVal)
				{
					agent.m_QuoteMsgInReply = tempAgent.m_QuoteMsgInReply;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_prefix"),nVal);
				if(nVal)
				{
					_tcscpy( agent.m_ReplyQuotedPrefix, tempAgent.m_ReplyQuotedPrefix );
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_sigreply"),nVal);
				if(nVal)
				{
					agent.m_SignatureTopReply = tempAgent.m_SignatureTopReply;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_default"),nVal);
				if(nVal)
				{
					agent.m_NewMessageFormat = tempAgent.m_NewMessageFormat;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_autofill"),nVal);
				if(nVal)
				{
					agent.m_UseAutoFill = tempAgent.m_UseAutoFill;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_spell"),nVal);
				if(nVal)
				{
					agent.m_ForceSpellCheck = tempAgent.m_ForceSpellCheck;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_deftb"),nVal);
				if(nVal)
				{
					agent.m_DefaultTicketBoxID = tempAgent.m_DefaultTicketBoxID;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_route"),nVal);
				if(nVal)
				{
					agent.m_RouteToInbox = tempAgent.m_RouteToInbox;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_close"),nVal);
				if(nVal)
				{
					agent.m_CloseTicket = tempAgent.m_CloseTicket;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_readrcpt"),nVal);
				if(nVal)
				{
					agent.m_ReadReceipt = tempAgent.m_ReadReceipt;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_confirm"),nVal);
				if(nVal)
				{
					agent.m_DeliveryConfirmation = tempAgent.m_DeliveryConfirmation;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_keepsent"),nVal);
				if(nVal)
				{
					agent.m_OutboxHoldTime = tempAgent.m_OutboxHoldTime;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_deflogin"),nVal);
				if(nVal)
				{
					agent.m_LoginStatusID = tempAgent.m_LoginStatusID;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_deflogout"),nVal);
				if(nVal)
				{
					agent.m_LogoutStatusID = tempAgent.m_LogoutStatusID;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_autoset"),nVal);
				if(nVal)
				{
					agent.m_AutoStatusTypeID = tempAgent.m_AutoStatusTypeID;
					agent.m_AutoStatusMin = tempAgent.m_AutoStatusMin;
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_ontxt"),nVal);
				if(nVal)
				{
					_tcscpy( agent.m_OnlineText, tempAgent.m_OnlineText );
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_awaytxt"),nVal);
				if(nVal)
				{
					_tcscpy( agent.m_AwayText, tempAgent.m_AwayText );
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_notavailtxt"),nVal);
				if(nVal)
				{
					_tcscpy( agent.m_NotAvailText, tempAgent.m_NotAvailText );
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_offtxt"),nVal);
				if(nVal)
				{
					_tcscpy( agent.m_OfflineText, tempAgent.m_OfflineText );
				}
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_oootxt"),nVal);
				if(nVal)
				{
					_tcscpy( agent.m_OooText, tempAgent.m_OooText );
				}
				
				nVal=0;
				GetISAPIData().GetXMLLong( _T("ag_tbv"),nVal);
				if(nVal)
				{
					GetQuery().Initialize();
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowState );
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowPriority );
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowNumNotes );
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowTicketID );
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowNumMsgs );
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowSubject );
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowContact );
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowDate );
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowCategory );
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowOwner );
					BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowTicketBox );
					BINDPARAM_LONG( GetQuery(), agent.m_AgentID );
					GetQuery().Execute( _T("UPDATE TicketBoxViews ")
										_T("SET ShowState=?,ShowPriority=?,ShowNumNotes=?,ShowTicketID=?,ShowNumMsgs=?,ShowSubject=?,ShowContact=?,ShowDate=?,ShowCategory=?,ShowOwner=?,ShowTicketBox=? ")
										_T("WHERE AgentID=? AND TicketBoxViewTypeID=9") );
				}
						
				agent.Update(GetQuery());
				GetISAPIData().m_SessionMap.InvalidateAgentSession(agent.m_AgentID);
				GetISAPIData().m_SessionMap.QueueSessionMonitors(1,agent.m_AgentID);
			}
		}
		catch(...){return 1;}
	}
	
	if(id.size() == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int CustomSettings::ApplyTicketBox(void)
{
	list<int> id;
	list<int>::iterator idIter;
	int nTBID = -1;
	CEMSString sTBs;
	
	sTBs.Format(_T(""));
	GetISAPIData().GetXMLString( _T("TicketBoxID"), sTBs, true );	
	sTBs.CDLInit();
	id.clear();
	while( sTBs.CDLGetNextInt( nTBID ) )
	{
		if(nTBID == 0)
		{
			break;
		}
		else
		{
			id.push_back(nTBID);
		}
	}

	if(nTBID == 0)
	{
		//Update all TBs
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nTBID );		
		GetQuery().Execute( _T("SELECT TicketBoxID ")
							_T("FROM TicketBoxes") );
		while ( GetQuery().Fetch() == S_OK )
		{
			id.push_back(nTBID);			
		}		
	}
	
	//Get the settings that need to be udpated
	TTicketBoxes tempTB;

	GetISAPIData().GetXMLLong( _T("OwnerID"), tempTB.m_OwnerID );
	GetISAPIData().GetXMLLong( _T("MsgDestID"), tempTB.m_MessageDestinationID );
	GetISAPIData().GetXMLLong( _T("TBGetOld"), tempTB.m_RequireGetOldest );
	GetISAPIData().GetXMLLong( _T("RequireTC"), tempTB.m_RequireTC );
	GetISAPIData().GetXMLLong( _T("GlobalUnread"), tempTB.m_UnreadMode );
	GetISAPIData().GetXMLLong( _T("MultiMail"), tempTB.m_MultiMail );
	GetISAPIData().GetXMLLong( _T("fromFormat"), tempTB.m_FromFormat );
	GetISAPIData().GetXMLLong( _T("TicketLink"), tempTB.m_TicketLink );	

	for( idIter = id.begin(); idIter != id.end(); idIter++ )
	{
		try
		{
			TTicketBoxes tb;
			tb.m_TicketBoxID = *idIter;
			tb.Query(GetQuery());
			int nVal=0;
			GetISAPIData().GetXMLLong( _T("tb_owner"),nVal);
			if(nVal)
			{
				tb.m_OwnerID = tempTB.m_OwnerID;
			}		
			nVal=0;
			GetISAPIData().GetXMLLong( _T("tb_msgdest"),nVal);
			if(nVal)
			{
				tb.m_MessageDestinationID = tempTB.m_MessageDestinationID;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("tb_getold"),nVal);
			if(nVal)
			{
				tb.m_RequireGetOldest = tempTB.m_RequireGetOldest;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("tb_reqtc"),nVal);
			if(nVal)
			{
				tb.m_RequireTC = tempTB.m_RequireTC;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("tb_globalunread"),nVal);
			if(nVal)
			{
				tb.m_UnreadMode = tempTB.m_UnreadMode;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("tb_multi"),nVal);
			if(nVal)
			{
				tb.m_MultiMail = tempTB.m_MultiMail;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("tb_fromadd"),nVal);
			if(nVal)
			{
				tb.m_FromFormat = tempTB.m_FromFormat;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("tb_linking"),nVal);
			if(nVal)
			{
				tb.m_TicketLink = tempTB.m_TicketLink;
			}
			
			tb.Update(GetQuery());			
		}
		catch(...){return 1;}
	}
	
	if(id.size() == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int CustomSettings::ApplyRoutingRule(void)
{
	list<int> id;
	list<int>::iterator idIter;
	int nRRID = -1;
	CEMSString sRRs;
	
	sRRs.Format(_T(""));
	GetISAPIData().GetXMLString( _T("RoutingRuleID"), sRRs, true );	
	sRRs.CDLInit();
	id.clear();
	while( sRRs.CDLGetNextInt( nRRID ) )
	{
		if(nRRID == 0)
		{
			break;
		}
		else
		{
			id.push_back(nRRID);
		}
	}

	if(nRRID == 0)
	{
		//Update all RRs
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nRRID );		
		GetQuery().Execute( _T("SELECT RoutingRuleID ")
							_T("FROM RoutingRules") );
		while ( GetQuery().Fetch() == S_OK )
		{
			id.push_back(nRRID);			
		}		
	}
	
	//Get the settings that need to be udpated
	TRoutingRules tempRR;

	GetISAPIData().GetXMLLong( _T("rrEnabled"), tempRR.m_IsEnabled );
	GetISAPIData().GetXMLLong( _T("DoProcessing"), tempRR.m_DoProcessingRules );
	
	for( idIter = id.begin(); idIter != id.end(); idIter++ )
	{
		try
		{
			TRoutingRules rr;
			rr.m_RoutingRuleID = *idIter;
			rr.Query(GetQuery());
			int nVal=0;
			GetISAPIData().GetXMLLong( _T("rr_rrenabled"),nVal);
			if(nVal)
			{
				rr.m_IsEnabled = tempRR.m_IsEnabled;
			}		
			nVal=0;
			GetISAPIData().GetXMLLong( _T("rr_doprocess"),nVal);
			if(nVal)
			{
				rr.m_DoProcessingRules = tempRR.m_DoProcessingRules;
			}
						
			rr.Update(GetQuery());			
		}
		catch(...){return 1;}
	}
	
	if(id.size() == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int CustomSettings::ApplyMessageSource(void)
{
	list<int> id;
	list<int>::iterator idIter;
	int nMSID = -1;
	CEMSString sMSs;
	
	sMSs.Format(_T(""));
	GetISAPIData().GetXMLString( _T("MessageSourceID"), sMSs, true );	
	sMSs.CDLInit();
	id.clear();
	while( sMSs.CDLGetNextInt( nMSID ) )
	{
		if(nMSID == 0)
		{
			break;
		}
		else
		{
			id.push_back(nMSID);
		}
	}

	if(nMSID == 0)
	{
		//Update all MSs
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nMSID );		
		GetQuery().Execute( _T("SELECT MessageSourceID ")
							_T("FROM MessageSources") );
		while ( GetQuery().Fetch() == S_OK )
		{
			id.push_back(nMSID);			
		}		
	}
	
	//Get the settings that need to be udpated
	TMessageSources tempMS;

	GetISAPIData().GetXMLLong( _T("msEnabled"), tempMS.m_IsActive );
	GetISAPIData().GetXMLLong( _T("lvCopies"), tempMS.m_LeaveCopiesOnServer );
	GetISAPIData().GetXMLLong( _T("leaveCopiesDays"), tempMS.m_LeaveCopiesDays );
	GetISAPIData().GetXMLLong( _T("skipDownloadDays"), tempMS.m_SkipDownloadDays );
	GetISAPIData().GetXMLLong( _T("ignoreDups"), tempMS.m_DupMsg );
	GetISAPIData().GetXMLLong( _T("msOfficeHrs"), tempMS.m_OfficeHours );
	GetISAPIData().GetXMLLong( _T("useReplyTo"), tempMS.m_UseReplyTo );
	GetISAPIData().GetXMLLong( _T("ckInterval"), tempMS.m_CheckFreqMins );
	GetISAPIData().GetXMLLong( _T("msTimeout"), tempMS.m_ConnTimeoutSecs );
	GetISAPIData().GetXMLLong( _T("msMaxSize"), tempMS.m_MaxInboundMsgSize );
	GetISAPIData().GetXMLLong( _T("zipAttach"), tempMS.m_ZipAttach );	
	
	for( idIter = id.begin(); idIter != id.end(); idIter++ )
	{
		try
		{
			TMessageSources ms;
			ms.m_MessageSourceID = *idIter;
			ms.Query(GetQuery());
			int nVal=0;
			GetISAPIData().GetXMLLong( _T("ms_msenabled"),nVal);
			if(nVal)
			{
				ms.m_IsActive = tempMS.m_IsActive;
			}		
			nVal=0;
			GetISAPIData().GetXMLLong( _T("ms_lvcopies"),nVal);
			if(nVal)
			{
				ms.m_LeaveCopiesOnServer = tempMS.m_LeaveCopiesOnServer;
				ms.m_LeaveCopiesDays = tempMS.m_LeaveCopiesDays;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("ms_skipdownload"),nVal);
			if(nVal)
			{
				ms.m_SkipDownloadDays = tempMS.m_SkipDownloadDays;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("ms_ignoredups"),nVal);
			if(nVal)
			{
				ms.m_DupMsg = tempMS.m_DupMsg;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("ms_msofficehrs"),nVal);
			if(nVal)
			{
				ms.m_OfficeHours = tempMS.m_OfficeHours;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("ms_usereplyto"),nVal);
			if(nVal)
			{
				ms.m_UseReplyTo = tempMS.m_UseReplyTo;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("ms_ckinterval"),nVal);
			if(nVal)
			{
				ms.m_CheckFreqMins = tempMS.m_CheckFreqMins;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("ms_mstimeout"),nVal);
			if(nVal)
			{
				ms.m_ConnTimeoutSecs = tempMS.m_ConnTimeoutSecs;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("ms_msmaxsize"),nVal);
			if(nVal)
			{
				ms.m_MaxInboundMsgSize = tempMS.m_MaxInboundMsgSize;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("ms_zipattach"),nVal);
			if(nVal)
			{
				ms.m_ZipAttach = tempMS.m_ZipAttach;
			}
									
			ms.Update(GetQuery());			
		}
		catch(...){return 1;}
	}
	
	if(id.size() == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int CustomSettings::ApplyMessageDestination(void)
{
	list<int> id;
	list<int>::iterator idIter;
	int nMDID = -1;
	CEMSString sMDs;
	
	sMDs.Format(_T(""));
	GetISAPIData().GetXMLString( _T("MessageDestinationID"), sMDs, true );	
	sMDs.CDLInit();
	id.clear();
	while( sMDs.CDLGetNextInt( nMDID ) )
	{
		if(nMDID == 0)
		{
			break;
		}
		else
		{
			id.push_back(nMDID);
		}
	}

	if(nMDID == 0)
	{
		//Update all MDs
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nMDID );		
		GetQuery().Execute( _T("SELECT MessageDestinationID ")
							_T("FROM MessageDestinations") );
		while ( GetQuery().Fetch() == S_OK )
		{
			id.push_back(nMDID);			
		}		
	}
	
	//Get the settings that need to be udpated
	TMessageDestinations tempMD;

	GetISAPIData().GetXMLLong( _T("mdEnabled"), tempMD.m_IsActive );
	GetISAPIData().GetXMLLong( _T("delInterval"), tempMD.m_ProcessFreqMins );
	GetISAPIData().GetXMLLong( _T("mdOfficeHrs"), tempMD.m_OfficeHours );
	GetISAPIData().GetXMLLong( _T("mdTimeout"), tempMD.m_ConnTimeoutSecs );
	GetISAPIData().GetXMLLong( _T("mdMaxSize"), tempMD.m_MaxOutboundMsgSize );
	GetISAPIData().GetXMLLong( _T("retrySend"), tempMD.m_MaxSendRetryHours );	
	
	for( idIter = id.begin(); idIter != id.end(); idIter++ )
	{
		try
		{
			TMessageDestinations md;
			md.m_MessageDestinationID = *idIter;
			md.Query(GetQuery());
			int nVal=0;
			GetISAPIData().GetXMLLong( _T("md_mdenabled"),nVal);
			if(nVal)
			{
				md.m_IsActive = tempMD.m_IsActive;
			}		
			nVal=0;
			GetISAPIData().GetXMLLong( _T("delinterval"),nVal);
			if(nVal)
			{
				md.m_ProcessFreqMins = tempMD.m_ProcessFreqMins;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("md_mdofficehrs"),nVal);
			if(nVal)
			{
				md.m_OfficeHours = tempMD.m_OfficeHours;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("md_mdtimeout"),nVal);
			if(nVal)
			{
				md.m_ConnTimeoutSecs = tempMD.m_ConnTimeoutSecs;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("md_mdmaxsize"),nVal);
			if(nVal)
			{
				md.m_MaxOutboundMsgSize = tempMD.m_MaxOutboundMsgSize;
			}
			nVal=0;
			GetISAPIData().GetXMLLong( _T("md_retry"),nVal);
			if(nVal)
			{
				md.m_MaxSendRetryHours = tempMD.m_MaxSendRetryHours;
			}
									
			md.Update(GetQuery());			
		}
		catch(...){return 1;}
	}
	
	if(id.size() == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}	
}

void CustomSettings::UpdateCTAS(void)
{
	int nCloseTicketAfterSend;
	tstring sAgentGroup;
	int nActualID;
	int nAgentID;
	list<int> id;
	list<int>::iterator idIter;
	
	GetISAPIData().GetXMLLong( _T("CloseTicketAfterSend"), nCloseTicketAfterSend );
	GetISAPIData().GetXMLString( _T("AgentOrGroup1"), sAgentGroup );
	if( sAgentGroup == "Agent" )
	{
		GetISAPIData().GetXMLLong( _T("Agent1"), nActualID );		
	}
	else
	{
		GetISAPIData().GetXMLLong( _T("Group1"), nActualID );		
	}
	
	if(nActualID > 0)
	{
		if( sAgentGroup == "Group" )
		{
			GetQuery().Initialize();
			BINDCOL_LONG_NOLEN( GetQuery(), nAgentID );		
			BINDPARAM_LONG( GetQuery(), nActualID );
			GetQuery().Execute( _T("SELECT AgentID ")
								_T("FROM AgentGroupings ")
								_T("WHERE GroupID=?") );
			while ( GetQuery().Fetch() == S_OK )
			{
				id.push_back(nAgentID);
			}
		}
		else
		{
			id.push_back(nActualID);
		}

		for( idIter = id.begin(); idIter != id.end(); idIter++ )
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), nCloseTicketAfterSend );		
			BINDPARAM_LONG( GetQuery(), *idIter );
			GetQuery().Execute( _T("UPDATE Agents SET CloseTicket=? WHERE AgentID=?") );
		}
		GetISAPIData().m_SessionMap.InvalidateAllAgentSessions();
		GetISAPIData().m_SessionMap.QueueSessionMonitors(1,0);
	}
}

void CustomSettings::UpdateRRTM(void)
{
	int nRouteRepliesToMe;
	tstring sAgentGroup;
	int nActualID;
	int nAgentID;
	list<int> id;
	list<int>::iterator idIter;
	
	GetISAPIData().GetXMLLong( _T("AutoRouteToInbox"), nRouteRepliesToMe );
	GetISAPIData().GetXMLString( _T("AgentOrGroup2"), sAgentGroup );
	if( sAgentGroup == "Agent" )
	{
		GetISAPIData().GetXMLLong( _T("Agent2"), nActualID );		
	}
	else
	{
		GetISAPIData().GetXMLLong( _T("Group2"), nActualID );		
	}
	
	if(nActualID > 0)
	{
		if( sAgentGroup == "Group" )
		{
			GetQuery().Initialize();
			BINDCOL_LONG_NOLEN( GetQuery(), nAgentID );		
			BINDPARAM_LONG( GetQuery(), nActualID );
			GetQuery().Execute( _T("SELECT AgentID ")
								_T("FROM AgentGroupings ")
								_T("WHERE GroupID=?") );
			while ( GetQuery().Fetch() == S_OK )
			{
				id.push_back(nAgentID);
			}
		}
		else
		{
			id.push_back(nActualID);
		}

		for( idIter = id.begin(); idIter != id.end(); idIter++ )
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), nRouteRepliesToMe );		
			BINDPARAM_LONG( GetQuery(), *idIter );
			GetQuery().Execute( _T("UPDATE Agents SET RouteToInbox=? WHERE AgentID=?") );
		}
		GetISAPIData().m_SessionMap.InvalidateAllAgentSessions();
		GetISAPIData().m_SessionMap.QueueSessionMonitors(1,0);
	}
	
}

void CustomSettings::AddException(void)
{
	int nExcType;
	CEMSString sExcDesc;
	CEMSString sExcDate;
	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;
	CEMSString sDate;
	
	GetISAPIData().GetXMLLong( _T("ExcType"), nExcType );
	GetISAPIData().GetXMLString("ExcDesc", sExcDesc);
	GetISAPIData().GetXMLString("dtExcObj",sExcDate);
	
	sExcDate.CDLGetTimeStamp( m_StartDate );
	sExcDate.CDLGetTimeStamp( m_EndDate );

	TOfficeHours oh;
	
    GetISAPIData().GetXMLLong( _T("ExcHrStart"), oh.m_StartHr );
	GetISAPIData().GetXMLLong( _T("ExcMinStart"), oh.m_StartMin );
	GetISAPIData().GetXMLLong( _T("ExcAmPmStart"), oh.m_StartAmPm );
	GetISAPIData().GetXMLLong( _T("ExcHrEnd"), oh.m_EndHr );
	GetISAPIData().GetXMLLong( _T("ExcMinEnd"), oh.m_EndMin );
	GetISAPIData().GetXMLLong( _T("ExcAmPmEnd"), oh.m_EndAmPm );
	
	int hr = 0;
	int min = 0;
	
	//Build begin date/time
	if(oh.m_StartHr == 12 && oh.m_StartAmPm == 1)
	{
		m_StartDate.hour = 0;
	}
	else if(oh.m_StartAmPm == 2 && oh.m_StartHr == 12)
	{
		m_StartDate.hour = 12;
	}
	else if (oh.m_StartAmPm == 2)
	{
		m_StartDate.hour = oh.m_StartHr + 12;
	}
	else
	{
		m_StartDate.hour = oh.m_StartHr;
	}

	m_StartDate.minute = oh.m_StartMin;
	m_StartDate.second = 0;
	m_StartDate.fraction = 0;

	//Build end date/time
	if(oh.m_EndHr == 12 && oh.m_EndAmPm == 1)
	{
		m_EndDate.hour = 0;
	}
	else if(oh.m_EndAmPm == 2 && oh.m_EndHr == 12)
	{
		m_EndDate.hour = 12;
	}
	else if (oh.m_EndAmPm == 2)
	{
		m_EndDate.hour = oh.m_EndHr + 12;
	}
	else
	{
		m_EndDate.hour = oh.m_EndHr;
	}

	m_EndDate.minute = oh.m_EndMin;
	m_EndDate.second = 0;
	m_EndDate.fraction = 0;

	//Add to DB
	oh.m_TypeID=nExcType;
	oh.m_ActualID=0;
	oh.m_DayID=0;
	oh.m_StartAmPm=0;
	oh.m_StartHr=0;
	oh.m_StartMin=0;
	oh.m_EndAmPm=0;
	oh.m_EndHr=0;
	oh.m_EndMin=0;

	_tcscpy( oh.m_Description, sExcDesc.c_str()  );
	oh.m_TimeStart = m_StartDate;
	oh.m_TimeEnd = m_EndDate;

	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );
	BOOL bIsFuture = OrderTimeStamps( now , m_EndDate );	
	if(bIsFuture == 0)
	{
		THROW_EMS_EXCEPTION( E_InvalidParameters, _T("End Time must be in the future!") );
	}
	
	BOOL bIsLater = OrderTimeStamps( m_StartDate, m_EndDate );
	if(bIsLater == 0)
	{
		THROW_EMS_EXCEPTION( E_InvalidParameters, _T("End Time must be after Start Time!") );
	}
	
	oh.Insert( GetQuery() );
}

void CustomSettings::DeleteException(void)
{
	TOfficeHours oh;
	oh.m_OfficeHourID = 0;
	GetISAPIData().GetXMLLong( _T("ooExcIDtoDel"), oh.m_OfficeHourID );
	if( oh.m_OfficeHourID < 8 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}
	oh.Delete( GetQuery() );	
}

void CustomSettings::UpdateSettings(void)
{
	CEMSString sCollection;
	CEMSString sParam;
	dca::String sEmailVerifyDays;	
	dca::String sEmailVerifyMx;	
	dca::String sEmailVerifyCatchAll;	
	dca::String sEmailVerifyRemove;	
	dca::String sEmailVerifyDelete;	
	dca::String sEmailVerifyDecrement;	
	dca::String sEmailVerifyIncrement;	
	dca::String sHeloHostName;
	dca::String sConnectTimeout;
	dca::String sDBTimeout;
	dca::String sSessionTimeout;
	dca::String sUpdateFrequency;
	std::string sUseCutoff;
	dca::String sMaxTicketLockTime;
	std::string sCutoffDate;
	dca::String s3cxClientVersion;
	dca::String sMailFlowClientVersion;
	dca::String sMaxISAPI;
	dca::String sMaxPOP3;
	dca::String sMaxSMTP;
	dca::String sQueueFull;
	GetISAPIData().GetXMLString("QueueFull",sQueueFull, true);
	dca::String sQueueSize;
	GetISAPIData().GetXMLString("QueueSize",sQueueSize, true);
	dca::String sSessionFreq;
	GetISAPIData().GetXMLString("SessionFreq",sSessionFreq, true);
	dca::String sStatusFreq;
	GetISAPIData().GetXMLString("StatusFreq",sStatusFreq, true);
	dca::String sDBFreq;
	GetISAPIData().GetXMLString("DBSessionFreq",sDBFreq, true);
	std::string sDbAgentSessions;
	GetISAPIData().GetXMLString("DbAgentSessions",sDbAgentSessions, true);
	std::string sShareAgentSessions;
	GetISAPIData().GetXMLString("ShareAgentSessions",sShareAgentSessions, true);	

	GetISAPIData().GetXMLString("EmailVerifyDays", sEmailVerifyDays);
	GetISAPIData().GetXMLString("EmailVerifyMx", sEmailVerifyMx);
	GetISAPIData().GetXMLString("EmailVerifyCatchAll", sEmailVerifyCatchAll);
	GetISAPIData().GetXMLString("EmailVerifyRemove", sEmailVerifyRemove);
	GetISAPIData().GetXMLString("EmailVerifyDelete", sEmailVerifyDelete);
	GetISAPIData().GetXMLString("EmailVerifyDecrement", sEmailVerifyDecrement);
	GetISAPIData().GetXMLString("EmailVerifyIncrement", sEmailVerifyIncrement);
	GetISAPIData().GetXMLString("HeloHostName", sHeloHostName, true);
	GetISAPIData().GetXMLString("ConnectTimeout", sConnectTimeout);
	GetISAPIData().GetXMLString("MaxISAPI", sMaxISAPI);
	GetISAPIData().GetXMLString("MaxPOP3", sMaxPOP3);
	GetISAPIData().GetXMLString("MaxSMTP", sMaxSMTP);
	GetISAPIData().GetXMLString("txtDBTimeout", sDBTimeout);
	GetISAPIData().GetXMLString("SessionTimeout", sSessionTimeout);
	GetISAPIData().GetXMLString("UpdateFrequency", sUpdateFrequency);
	GetISAPIData().GetXMLString("3cxClientVersion", s3cxClientVersion);
	GetISAPIData().GetXMLString("MailFlowClientVersion", sMailFlowClientVersion);	
	GetISAPIData().GetXMLString("txtMaxTicketLockTime", sMaxTicketLockTime);
	if(!GetISAPIData().GetXMLString("chkCutoffDate",sUseCutoff, true))
		sUseCutoff.assign(_T("off"));

	std::string sEnableMyTickets;
	GetISAPIData().GetXMLString("chkEnableMyTickets",sEnableMyTickets, true);

	std::string sEnablePublicTicketBoxes;
	GetISAPIData().GetXMLString("chkEnablePublicTicketBoxes",sEnablePublicTicketBoxes, true);

	std::string sEnableAgentViews;
	GetISAPIData().GetXMLString("chkEnableAgentViews",sEnableAgentViews, true);

	std::string sEnableScheduledReports;
	GetISAPIData().GetXMLString("chkEnableScheduledReports",sEnableScheduledReports, true);

	std::string sEnableAgentActivity;
	GetISAPIData().GetXMLString("chkEnableAgentActivity",sEnableAgentActivity, true);

	std::string sEnableVerification;
	GetISAPIData().GetXMLString("chkEnableVerification",sEnableVerification, true);	

	std::string sEnableAgentAuth;
	GetISAPIData().GetXMLString("chkEnableAgentAuth",sEnableAgentAuth, true);

	std::string sEnableTicketDelete;
	GetISAPIData().GetXMLString("chkEnableTicketDelete",sEnableTicketDelete, true);

	std::string sEnableTicketCreate;
	GetISAPIData().GetXMLString("chkEnableTicketCreate",sEnableTicketCreate, true);

	std::string sEnableTicketMove;
	GetISAPIData().GetXMLString("chkEnableTicketMove",sEnableTicketMove, true);

	std::string sEnableTicketEscalate;
	GetISAPIData().GetXMLString("chkEnableTicketEscalate",sEnableTicketEscalate, true);

	std::string sEnableTicketReassign;
	GetISAPIData().GetXMLString("chkEnableTicketReassign",sEnableTicketReassign, true);

	std::string sEnableTicketRestore;
	GetISAPIData().GetXMLString("chkEnableTicketRestore",sEnableTicketRestore, true);

	std::string sEnableTicketOpen;
	GetISAPIData().GetXMLString("chkEnableTicketOpen",sEnableTicketOpen, true);

	std::string sEnableTicketClose;
	GetISAPIData().GetXMLString("chkEnableTicketClose",sEnableTicketClose, true);

	std::string sEnableTicketMessageAdd;
	GetISAPIData().GetXMLString("chkEnableTicketMessageAdd",sEnableTicketMessageAdd, true);

	std::string sEnableTicketMessageDelete;
	GetISAPIData().GetXMLString("chkEnableTicketMessageDelete",sEnableTicketMessageDelete, true);

	std::string sEnableTicketMessageRevoke;
	GetISAPIData().GetXMLString("chkEnableTicketMessageRevoke",sEnableTicketMessageRevoke, true);

	std::string sEnableTicketMessageRelease;
	GetISAPIData().GetXMLString("chkEnableTicketMessageRelease",sEnableTicketMessageRelease, true);
	std::string sEnableTicketMessageReturn;
	GetISAPIData().GetXMLString("chkEnableTicketMessageReturn",sEnableTicketMessageReturn, true);
	std::string sEnableTicketLinked;
	GetISAPIData().GetXMLString("chkEnableTicketLinked",sEnableTicketLinked, true);
	std::string sEnableTicketUnlinked;
	GetISAPIData().GetXMLString("chkEnableTicketUnlinked",sEnableTicketUnlinked, true);
	std::string sEnableTicketLinkCreated;
	GetISAPIData().GetXMLString("chkEnableTicketLinkCreated",sEnableTicketLinkCreated, true);
	std::string sEnableTicketLinkDeleted;
	GetISAPIData().GetXMLString("chkEnableTicketLinkDeleted",sEnableTicketLinkDeleted, true);
	std::string sEnableTicketChangeCategory;
	GetISAPIData().GetXMLString("chkEnableTicketChangeCategory",sEnableTicketChangeCategory, true);
	std::string sEnableTicketChangeField;
	GetISAPIData().GetXMLString("chkEnableTicketChangeField",sEnableTicketChangeField, true);

	TDictionary dc;
	vector<TDictionary> d;
	vector<TDictionary>::iterator dIter;
	dc.PrepareList( GetQuery() );
	while( GetQuery().Fetch() == S_OK )
	{
		d.push_back(dc);
	}	
	for( dIter = d.begin(); dIter != d.end(); dIter++ )
	{
		dca::String sCheck = "0";
		switch(dIter->m_DictionaryID)
		{
		case 1:
			{GetISAPIData().GetXMLString("chkDictionary1",sCheck, true);}
			break;
		case 2:
			{GetISAPIData().GetXMLString("chkDictionary2",sCheck, true);}
			break;
		case 3:
			{GetISAPIData().GetXMLString("chkDictionary3",sCheck, true);}
			break;
		case 4:
			{GetISAPIData().GetXMLString("chkDictionary4",sCheck, true);}
			break;
		case 5:
			{GetISAPIData().GetXMLString("chkDictionary5",sCheck, true);}
			break;
		case 6:
			{GetISAPIData().GetXMLString("chkDictionary6",sCheck, true);}
			break;
		case 7:
			{GetISAPIData().GetXMLString("chkDictionary7",sCheck, true);}
			break;
		case 8:
			{GetISAPIData().GetXMLString("chkDictionary8",sCheck, true);}
			break;
		case 9:
			{GetISAPIData().GetXMLString("chkDictionary9",sCheck, true);}
			break;
		case 10:
			{GetISAPIData().GetXMLString("chkDictionary10",sCheck, true);}
			break;
		case 11:
			{GetISAPIData().GetXMLString("chkDictionary11",sCheck, true);}
			break;
		case 12:
			{GetISAPIData().GetXMLString("chkDictionary12",sCheck, true);}
			break;		
		}
		int nCheck = sCheck.ToInt();
		if((nCheck != dIter->m_IsEnabled) && nCheck)
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), dIter->m_DictionaryID );
			GetQuery().Execute( _T("UPDATE Dictionary ")
						_T("SET IsEnabled=1 ")
						_T("WHERE DictionaryID=?") );
		}
		else if(nCheck != dIter->m_IsEnabled)
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), dIter->m_DictionaryID );
			GetQuery().Execute( _T("UPDATE Dictionary ")
						_T("SET IsEnabled=0 ")
						_T("WHERE DictionaryID=?") );
		}
	}

	GetISAPIData().GetXMLString("dtCutOffObj",sCutoffDate);

	if(sConnectTimeout.ToInt() < 5)
		sConnectTimeout.FromInt(5);

	
	if(sUpdateFrequency.ToInt() < 10 && sUpdateFrequency.ToInt() != 0)
		sUpdateFrequency.FromInt(10);

	if(sMaxISAPI.ToInt() < 5)
		sMaxISAPI.FromInt(5);

	if(sMaxPOP3.ToInt() < 5)
		sMaxPOP3.FromInt(5);

	if(sMaxSMTP.ToInt() < 5)
		sMaxSMTP.FromInt(5);

	if(sDBTimeout.ToInt() < 60)
		sDBTimeout.FromInt(60);

	if(sMaxTicketLockTime.ToInt() != 0 && sMaxTicketLockTime.ToInt() < 20 )
	{
		sMaxTicketLockTime.FromInt(60);
	}
	
	if(sHeloHostName.length() > 0)
	{
		CEMSString sHeloTemp;
		sHeloTemp.Format(_T("%s"),sHeloHostName.c_str());	
		if (!sHeloTemp.ValidateHostName())
		{
			CEMSString sError;
			sError.Format( _T("The HELO Host Name [%s] is invalid"), sHeloHostName.c_str() );
			THROW_VALIDATION_EXCEPTION( _T("HELO Host Name"), sError );
		}
	}
	
	/*int nCharset;
	GetISAPIData().GetXMLLong( _T("Charset"), nCharset );
	std::string sCharset;
	switch(nCharset)
	{
	case 0: 
		sCharset.assign(_T("0"));
		break;

	case 1:
		sCharset.assign(_T("1"));
		break;
		
	default:
		sCharset.assign(_T("0"));
		break;
	}*/

	std::string sRequireDate;
	if(!GetISAPIData().GetXMLString("chkRequireDate",sRequireDate, true))
		sRequireDate.assign(_T("1"));

	std::string sLogoffAll;
	if(!GetISAPIData().GetXMLString("chkLogoffAll",sLogoffAll, true))
		sLogoffAll.assign(_T("0"));

	std::string sExpireAll;
	if(!GetISAPIData().GetXMLString("chkExpireAll",sExpireAll, true))
		sExpireAll.assign(_T("0"));

	std::string sAllowLogoff;
	if(!GetISAPIData().GetXMLString("chkAllowLogoff",sAllowLogoff, true))
		sAllowLogoff.assign(_T("0"));

	int nTimeZoneID;
	dca::String sTimeZoneID;
	GetISAPIData().GetXMLLong( _T("TimeZoneID"), nTimeZoneID );
	sTimeZoneID.FromInt(nTimeZoneID);

	int nDictionaryID;
	dca::String sDictionaryID;
	GetISAPIData().GetXMLLong( _T("DictionaryID"), nDictionaryID );
	sDictionaryID.FromInt(nDictionaryID);

	int nMsgDate;
	GetISAPIData().GetXMLLong( _T("MsgDate"), nMsgDate );
	std::string sMsgDate;
	switch(nMsgDate)
	{
	case 0: 
		sMsgDate.assign(_T("0"));
		break;

	case 1:
		sMsgDate.assign(_T("1"));
		break;
		
	default:
		sMsgDate.assign(_T("0"));
		break;
	}

	std::string sAgentEmailBypass;
	GetISAPIData().GetXMLString("chkAgentEmailBypass",sAgentEmailBypass, true);
	
	tstring sTemp;
	GetISAPIData().GetXMLString( _T("BypassIdCollection"), sTemp, true );

	if( _tcsicmp( sTemp.c_str(), _T("") ) != 0)
	{
		TIMESTAMP_STRUCT now;
		GetTimeStamp( now );
		sCollection.Format(_T("%s"),sTemp.c_str());
		GetQuery().Initialize();
		m_b.clear();

		b.PrepareList( GetQuery() );
		while( GetQuery().Fetch() == S_OK )
		{
			m_b.push_back(b);
		}

		// Get the collection
		vector<TBypass> tb;
		vector<TBypass>::iterator tbIter;
		TBypass ttb;
		int nBID;
		bool bFound = false;
		bool bDup = false;

		sCollection.CDLInit();
		while( sCollection.CDLGetNextInt( nBID ) )
		{
			ttb.m_BypassID = nBID;						
			sParam.Format( _T("Bypass%d"), nBID );
			GetISAPIData().GetXMLTCHAR( _T(sParam.c_str()), ttb.m_BypassValue, 255 );
			sParam.Format( _T("BypassType%d"), nBID );
			GetISAPIData().GetXMLLong( _T(sParam.c_str()), ttb.m_BypassTypeID );

			if(ttb.m_BypassTypeID == 0)
			{
				CEMSString sString;
				sString.Format(_T("%s"),ttb.m_BypassValue);	
				if (!sString.ValidateEmailAddr())
				{
					CEMSString sError;
					sError.Format( _T("The email address specified [%s] is invalid"), sString.c_str() );
					THROW_VALIDATION_EXCEPTION( _T("Bypass Value"), sError );
				}
			}
			else if(ttb.m_BypassTypeID == 1)
			{
				CEMSString sString;
				sString.Format(_T("%s"),ttb.m_BypassValue);	
				if (!sString.ValidateHostName())
				{
					CEMSString sError;
					sError.Format( _T("The domain specified [%s] is invalid"), sString.c_str() );
					THROW_VALIDATION_EXCEPTION( _T("Bypass Value"), sError );
				}
			}
			//check for dups
			bDup = false;
			for( tbIter = tb.begin(); tbIter != tb.end(); tbIter++ )
			{
				if(ttb.m_BypassTypeID == tbIter->m_BypassTypeID && _tcscmp(ttb.m_BypassValue,tbIter->m_BypassValue) == 0)
				{
					bDup = true;
					break;
				}
			}
			if(!bDup)
			{
				tb.push_back( ttb );
			}
		}
		
		// Add missing Options
		for( tbIter = tb.begin(); tbIter != tb.end(); tbIter++ )
		{
			bFound = false;
			bDup = false;
			for( mbIter = m_b.begin(); mbIter != m_b.end(); mbIter++ )
			{
				if (mbIter->m_BypassID == tbIter->m_BypassID)
				{
					bFound = true;
					break;
				}
				else if(mbIter->m_BypassTypeID == tbIter->m_BypassTypeID && mbIter->m_BypassValue == tbIter->m_BypassValue)
				{
					bDup = true;
					break;
				}
			}
			
			if(bFound && !bDup)
			{
				// This option already exists, let's update it
				ttb.m_BypassID = tbIter->m_BypassID;
				ttb.m_BypassTypeID = tbIter->m_BypassTypeID;
				strncpy(ttb.m_BypassValue, tbIter->m_BypassValue, PERSONALDATA_DATAVALUE_LENGTH);
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), ttb.m_BypassTypeID );
				BINDPARAM_TCHAR( GetQuery(), ttb.m_BypassValue );
				BINDPARAM_LONG( GetQuery(), ttb.m_BypassID );
				GetQuery().Execute( _T("UPDATE Bypass ")
							_T("SET BypassTypeID=?,BypassValue=? ")
							_T("WHERE BypassID=?") );
			}
			else if(!bDup)
			{
				// This option does not exist, let's add it
				ttb.m_BypassTypeID = tbIter->m_BypassTypeID;
				strncpy(ttb.m_BypassValue, tbIter->m_BypassValue, PERSONALDATA_DATAVALUE_LENGTH);
				ttb.m_DateCreated = now;
				ttb.Insert( GetQuery() );				
			}
		}
		
		// Delete extra Options
		for( mbIter = m_b.begin(); mbIter != m_b.end(); mbIter++ )
		{
			bFound = false;
			for( tbIter = tb.begin(); tbIter != tb.end(); tbIter++ )
			{
				if ( mbIter->m_BypassID == tbIter->m_BypassID )
				{
					bFound = true;
					break;
				}
			}
			if ( !bFound )
			{
				// It's an extra, let's delete it
				ttb.m_BypassID = mbIter->m_BypassID;
				ttb.Delete( GetQuery() );				
			}
		}
	}
	else
	{
		GetQuery().Initialize();
		GetQuery().Execute( _T("DELETE FROM Bypass") );
	}
	
	sTemp.assign(_T(""));
	GetISAPIData().GetXMLString( _T("EmailIdCollection"), sTemp, true );

	if( _tcsicmp( sTemp.c_str(), _T("") ) != 0)
	{
		TIMESTAMP_STRUCT now;
		GetTimeStamp( now );
		sCollection.Format(_T("%s"),sTemp.c_str());
		GetQuery().Initialize();
		m_b.clear();

		TBypass tB;	
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), tB.m_BypassID );
		BINDCOL_LONG( GetQuery(), tB.m_BypassTypeID );
		BINDCOL_TCHAR( GetQuery(), tB.m_BypassValue );
		BINDCOL_TIME( GetQuery(), tB.m_DateCreated );
		GetQuery().Execute( _T("SELECT EmailID,EmailTypeID,EmailValue,DateCreated ")
					_T("FROM Email WHERE EmailTypeID < 2")
					_T("ORDER BY EmailValue") );		
		while( GetQuery().Fetch() == S_OK )
		{
			m_b.push_back(tB);
		}
		
		// Get the collection
		vector<TBypass> tb;
		vector<TBypass>::iterator tbIter;
		TBypass ttb;
		int nBID;
		bool bFound = false;
		bool bDup = false;

		sCollection.CDLInit();
		while( sCollection.CDLGetNextInt( nBID ) )
		{
			ttb.m_BypassID = nBID;						
			sParam.Format( _T("Email%d"), nBID );
			GetISAPIData().GetXMLTCHAR( _T(sParam.c_str()), ttb.m_BypassValue, 255 );
			sParam.Format( _T("EmailType%d"), nBID );
			GetISAPIData().GetXMLLong( _T(sParam.c_str()), ttb.m_BypassTypeID );

			if(ttb.m_BypassTypeID == 0)
			{
				CEMSString sString;
				sString.Format(_T("%s"),ttb.m_BypassValue);	
				if (!sString.ValidateEmailAddr())
				{
					CEMSString sError;
					sError.Format( _T("The email address specified [%s] is invalid"), sString.c_str() );
					THROW_VALIDATION_EXCEPTION( _T("Bypass Value"), sError );
				}
			}
			else if(ttb.m_BypassTypeID == 1)
			{
				CEMSString sString;
				sString.Format(_T("%s"),ttb.m_BypassValue);	
				if (!sString.ValidateHostName())
				{
					CEMSString sError;
					sError.Format( _T("The domain specified [%s] is invalid"), sString.c_str() );
					THROW_VALIDATION_EXCEPTION( _T("Bypass Value"), sError );
				}
			}
			//check for dups
			bDup = false;
			for( tbIter = tb.begin(); tbIter != tb.end(); tbIter++ )
			{
				if(ttb.m_BypassTypeID == tbIter->m_BypassTypeID && _tcscmp(ttb.m_BypassValue,tbIter->m_BypassValue) == 0)
				{
					bDup = true;
					break;
				}
			}
			if(!bDup)
			{
				tb.push_back( ttb );
			}
		}
		
		// Add missing Options
		for( tbIter = tb.begin(); tbIter != tb.end(); tbIter++ )
		{
			bFound = false;
			bDup = false;
			for( mbIter = m_b.begin(); mbIter != m_b.end(); mbIter++ )
			{
				if (mbIter->m_BypassID == tbIter->m_BypassID)
				{
					bFound = true;
					break;
				}
				else if(mbIter->m_BypassTypeID == tbIter->m_BypassTypeID && mbIter->m_BypassValue == tbIter->m_BypassValue)
				{
					bDup = true;
					break;
				}
			}
			
			if(bFound && !bDup)
			{
				// This option already exists, let's update it
				ttb.m_BypassID = tbIter->m_BypassID;
				ttb.m_BypassTypeID = tbIter->m_BypassTypeID;
				strncpy(ttb.m_BypassValue, tbIter->m_BypassValue, PERSONALDATA_DATAVALUE_LENGTH);
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), ttb.m_BypassTypeID );
				BINDPARAM_TCHAR( GetQuery(), ttb.m_BypassValue );
				BINDPARAM_LONG( GetQuery(), ttb.m_BypassID );
				GetQuery().Execute( _T("UPDATE Email ")
							_T("SET EmailTypeID=?,EmailValue=? ")
							_T("WHERE EmailID=?") );
			}
			else if(!bDup)
			{
				// This option does not exist, let's add it
				ttb.m_BypassTypeID = tbIter->m_BypassTypeID;
				strncpy(ttb.m_BypassValue, tbIter->m_BypassValue, PERSONALDATA_DATAVALUE_LENGTH);
				ttb.m_DateCreated = now;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), ttb.m_BypassTypeID );
				BINDPARAM_TCHAR( GetQuery(), ttb.m_BypassValue );
				BINDPARAM_TIME( GetQuery(), ttb.m_DateCreated );
				GetQuery().Execute( _T("INSERT INTO Email (EmailTypeID,EmailValue,DateCreated) VALUES (?,?,?)") );				
			}
		}
		
		// Delete extra Options
		for( mbIter = m_b.begin(); mbIter != m_b.end(); mbIter++ )
		{
			bFound = false;
			for( tbIter = tb.begin(); tbIter != tb.end(); tbIter++ )
			{
				if ( mbIter->m_BypassID == tbIter->m_BypassID )
				{
					bFound = true;
					break;
				}
			}
			if ( !bFound )
			{
				// It's an extra, let's delete it
				ttb.m_BypassID = mbIter->m_BypassID;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), ttb.m_BypassID );
				GetQuery().Execute( _T("DELETE FROM Email ")
	               _T("WHERE EmailID=?") );
			}
		}
	}
	else
	{
		GetQuery().Initialize();
		GetQuery().Execute( _T("DELETE FROM Email WHERE EmailTypeID < 2") );
	}

	sTemp.assign(_T(""));
	GetISAPIData().GetXMLString( _T("AutoEmailIdCollection"), sTemp, true );

	if( _tcsicmp( sTemp.c_str(), _T("") ) != 0)
	{
		TIMESTAMP_STRUCT now;
		GetTimeStamp( now );
		sCollection.Format(_T("%s"),sTemp.c_str());
		GetQuery().Initialize();
		m_b.clear();

		TBypass tB;	
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), tB.m_BypassID );
		BINDCOL_LONG( GetQuery(), tB.m_BypassTypeID );
		BINDCOL_TCHAR( GetQuery(), tB.m_BypassValue );
		BINDCOL_TIME( GetQuery(), tB.m_DateCreated );
		GetQuery().Execute( _T("SELECT EmailID,EmailTypeID,EmailValue,DateCreated ")
					_T("FROM Email WHERE EmailTypeID IN (2,3)")
					_T("ORDER BY EmailValue") );		
		while( GetQuery().Fetch() == S_OK )
		{
			m_b.push_back(tB);
		}
		
		// Get the collection
		vector<TBypass> tb;
		vector<TBypass>::iterator tbIter;
		TBypass ttb;
		int nBID;
		bool bFound = false;
		bool bDup = false;

		sCollection.CDLInit();
		while( sCollection.CDLGetNextInt( nBID ) )
		{
			ttb.m_BypassID = nBID;						
			sParam.Format( _T("AutoEmail%d"), nBID );
			GetISAPIData().GetXMLTCHAR( _T(sParam.c_str()), ttb.m_BypassValue, 255 );
			sParam.Format( _T("AutoEmailType%d"), nBID );
			GetISAPIData().GetXMLLong( _T(sParam.c_str()), ttb.m_BypassTypeID );

			if(ttb.m_BypassTypeID == 2)
			{
				CEMSString sString;
				sString.Format(_T("%s"),ttb.m_BypassValue);	
				if (!sString.ValidateEmailAddr())
				{
					CEMSString sError;
					sError.Format( _T("The email address specified [%s] is invalid"), sString.c_str() );
					THROW_VALIDATION_EXCEPTION( _T("Bypass Value"), sError );
				}
			}
			else if(ttb.m_BypassTypeID == 3)
			{
				CEMSString sString;
				sString.Format(_T("%s"),ttb.m_BypassValue);	
				if (!sString.ValidateHostName())
				{
					CEMSString sError;
					sError.Format( _T("The domain specified [%s] is invalid"), sString.c_str() );
					THROW_VALIDATION_EXCEPTION( _T("Bypass Value"), sError );
				}
			}
			//check for dups
			bDup = false;
			for( tbIter = tb.begin(); tbIter != tb.end(); tbIter++ )
			{
				if(ttb.m_BypassTypeID == tbIter->m_BypassTypeID && _tcscmp(ttb.m_BypassValue,tbIter->m_BypassValue) == 0)
				{
					bDup = true;
					break;
				}
			}
			if(!bDup)
			{
				tb.push_back( ttb );
			}
		}
		
		// Add missing Options
		for( tbIter = tb.begin(); tbIter != tb.end(); tbIter++ )
		{
			bFound = false;
			bDup = false;
			for( mbIter = m_b.begin(); mbIter != m_b.end(); mbIter++ )
			{
				if (mbIter->m_BypassID == tbIter->m_BypassID)
				{
					bFound = true;
					break;
				}
				else if(mbIter->m_BypassTypeID == tbIter->m_BypassTypeID && mbIter->m_BypassValue == tbIter->m_BypassValue)
				{
					bDup = true;
					break;
				}
			}
			
			if(bFound && !bDup)
			{
				// This option already exists, let's update it
				ttb.m_BypassID = tbIter->m_BypassID;
				ttb.m_BypassTypeID = tbIter->m_BypassTypeID;
				strncpy(ttb.m_BypassValue, tbIter->m_BypassValue, PERSONALDATA_DATAVALUE_LENGTH);
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), ttb.m_BypassTypeID );
				BINDPARAM_TCHAR( GetQuery(), ttb.m_BypassValue );
				BINDPARAM_LONG( GetQuery(), ttb.m_BypassID );
				GetQuery().Execute( _T("UPDATE Email ")
							_T("SET EmailTypeID=?,EmailValue=? ")
							_T("WHERE EmailID=?") );
			}
			else if(!bDup)
			{
				// This option does not exist, let's add it
				ttb.m_BypassTypeID = tbIter->m_BypassTypeID;
				strncpy(ttb.m_BypassValue, tbIter->m_BypassValue, PERSONALDATA_DATAVALUE_LENGTH);
				ttb.m_DateCreated = now;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), ttb.m_BypassTypeID );
				BINDPARAM_TCHAR( GetQuery(), ttb.m_BypassValue );
				BINDPARAM_TIME( GetQuery(), ttb.m_DateCreated );
				GetQuery().Execute( _T("INSERT INTO Email (EmailTypeID,EmailValue,DateCreated) VALUES (?,?,?)") );				
			}
		}
		
		// Delete extra Options
		for( mbIter = m_b.begin(); mbIter != m_b.end(); mbIter++ )
		{
			bFound = false;
			for( tbIter = tb.begin(); tbIter != tb.end(); tbIter++ )
			{
				if ( mbIter->m_BypassID == tbIter->m_BypassID )
				{
					bFound = true;
					break;
				}
			}
			if ( !bFound )
			{
				// It's an extra, let's delete it
				ttb.m_BypassID = mbIter->m_BypassID;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), ttb.m_BypassID );
				GetQuery().Execute( _T("DELETE FROM Email ")
	               _T("WHERE EmailID=?") );
			}
		}
	}
	else
	{
		GetQuery().Initialize();
		GetQuery().Execute( _T("DELETE FROM Email WHERE EmailTypeID IN (2,3)") );
	}

	SetDatabaseSettings(29, sMaxISAPI);
	SetDatabaseSettings(20, sMaxPOP3);
	SetDatabaseSettings(21, sMaxSMTP);
	SetDatabaseSettings(40, sDBTimeout);
	SetDatabaseSettings(66, sMaxTicketLockTime);
	SetDatabaseSettings(119, sEmailVerifyDays);
	SetDatabaseSettings(120, sEmailVerifyMx);
	SetDatabaseSettings(121, sEmailVerifyCatchAll);
	SetDatabaseSettings(122, sEmailVerifyRemove);
	SetDatabaseSettings(123, sEmailVerifyDelete);
	SetDatabaseSettings(124, sEmailVerifyDecrement);
	SetDatabaseSettings(125, sEmailVerifyIncrement);
	SetDatabaseSettings(126, sHeloHostName);
	SetDatabaseSettings(127, sConnectTimeout);
	SetDatabaseSettings(128, sUpdateFrequency);
	SetDatabaseSettings(129, sEnableTicketChangeCategory);
	SetDatabaseSettings(130, sEnableTicketChangeField);
	SetDatabaseSettings(131, sEnableVerification);
	
	SetDatabaseSettings(132, sShareAgentSessions);
	if(sSessionFreq.ToInt() < 5 || sSessionFreq.ToInt() > 60)
		sSessionFreq.FromInt(30);
	SetDatabaseSettings(133, sSessionFreq);
	if(sStatusFreq.ToInt() < 5 || sStatusFreq.ToInt() > 60)
		sStatusFreq.FromInt(30);
	SetDatabaseSettings(134, sStatusFreq);
	if(sDBFreq.ToInt() < 30 || sDBFreq.ToInt() > 60)
		sDBFreq.FromInt(60);
	SetDatabaseSettings(135, sDBFreq);
	if(sShareAgentSessions == "1")
		sDbAgentSessions = "1";
	SetDatabaseSettings(136, sDbAgentSessions);	
	if(sQueueFull.ToInt() > 256)
		sQueueFull.FromInt(0);
	SetDatabaseSettings(137, sQueueFull);
	if(sQueueSize.ToInt() < 32 || sQueueSize.ToInt() > 256)
		sQueueSize.FromInt(32);
	SetDatabaseSettings(138, sQueueSize);
	//SetDatabaseSettings(141, sCharset);
	SetDatabaseSettings(142, sMsgDate);
	SetDatabaseSettings(143, sAgentEmailBypass);
	SetDatabaseSettings(144, sRequireDate);
	SetDatabaseSettings(145, sLogoffAll);
	SetDatabaseSettings(146, sExpireAll);
	SetDatabaseSettings(147, sAllowLogoff);
	SetDatabaseSettings(148, sTimeZoneID);
	SetDatabaseSettings(149, sDictionaryID);

	if(!sUseCutoff.compare(_T("off")))
	{
		sCutoffDate.erase();
		sUseCutoff.assign(_T("0"));
	}
	else
	{
		
		std::string::size_type pos = std::string::npos;
		do
		{
			pos = sCutoffDate.find(",");

			if(pos != std::string::npos)
				sCutoffDate = sCutoffDate.replace(pos,1,"/");

		}while(pos != std::string::npos);

		sUseCutoff.assign(_T("1"));
	}

	std::string sKeepAlive;
	if(!GetISAPIData().GetXMLString("chkKeepAlive",sKeepAlive, true))
		sKeepAlive.assign(_T("0"));

	std::string sOnline;
	if(!GetISAPIData().GetXMLString("chkOnline",sOnline, true))
		sOnline.assign(_T("0"));

	std::string sAway;
	if(!GetISAPIData().GetXMLString("chkAway",sAway, true))
		sAway.assign(_T("0"));

	std::string sNotAvail;
	if(!GetISAPIData().GetXMLString("chkNotAvail",sNotAvail, true))
		sNotAvail.assign(_T("0"));

	std::string sOffline;
	if(!GetISAPIData().GetXMLString("chkOffline",sOffline, true))
		sOffline.assign(_T("0"));

	std::string sOoo;
	if(!GetISAPIData().GetXMLString("chkOoo",sOoo, true))
		sOoo.assign(_T("0"));

	std::string sAdminOr;
	if(!GetISAPIData().GetXMLString("chkAdminOr",sAdminOr, true))
		sAdminOr.assign(_T("0"));

	std::string sAgentOr;
	if(!GetISAPIData().GetXMLString("chkAgentOr",sAgentOr, true))
		sAgentOr.assign(_T("0"));

	std::string sTicketListDate;
	if(!GetISAPIData().GetXMLString("chkTicketListDate",sTicketListDate, true))
		sTicketListDate.assign(_T("0"));

	std::string sTicketTracking;
	if(!GetISAPIData().GetXMLString("chkTicketTracking",sTicketTracking, true))
		sTicketTracking.assign(_T("0"));

	std::string sEnableGetOldestSearch;
	GetISAPIData().GetXMLString("chkEnableGetOldestSearch",sEnableGetOldestSearch, true);

	int nCloseTicketAfterSend;
	GetISAPIData().GetXMLLong( _T("CloseTicketAfterSend"), nCloseTicketAfterSend );
	std::string sCloseTicketAfterSend;
	switch(nCloseTicketAfterSend)
	{
	case 0: 
		sCloseTicketAfterSend.assign(_T("0"));
		break;

	case 1:
		sCloseTicketAfterSend.assign(_T("1"));
		break;
		
	case 2:
		sCloseTicketAfterSend.assign(_T("2"));
		break;

	default:
		sCloseTicketAfterSend.assign(_T("2"));
		break;
	}

	int nRouteRepliesToMe;
	GetISAPIData().GetXMLLong( _T("AutoRouteToInbox"), nRouteRepliesToMe );
	std::string sRouteRepliesToMe;
	switch(nRouteRepliesToMe)
	{
	case 0: 
		sRouteRepliesToMe.assign(_T("0"));
		break;

	case 1:
		sRouteRepliesToMe.assign(_T("1"));
		break;
		
	case 2:
		sRouteRepliesToMe.assign(_T("2"));
		break;

	default:
		sRouteRepliesToMe.assign(_T("2"));
		break;
	}
	
	int nDateEntryFormat;
	GetISAPIData().GetXMLLong( _T("DateEntryFormat"), nDateEntryFormat );
	std::string sDateEntryFormat;
	switch(nDateEntryFormat)
	{
	case 0: 
		sDateEntryFormat.assign(_T("0"));
		break;

	case 1:
		sDateEntryFormat.assign(_T("1"));
		break;
		
	default:
		sDateEntryFormat.assign(_T("0"));
		break;
	}
	
	SetDatabaseSettings(41, sUseCutoff);
	SetDatabaseSettings(42, sCutoffDate);
	SetDatabaseSettings(43, sEnableMyTickets);
	SetDatabaseSettings(44, sEnablePublicTicketBoxes);
	SetDatabaseSettings(45, sEnableAgentViews);
	SetDatabaseSettings(47, sEnableScheduledReports);
	SetDatabaseSettings(48, sEnableAgentActivity);
	SetDatabaseSettings(49, sEnableAgentAuth);
	SetDatabaseSettings(50, sEnableTicketDelete);
	SetDatabaseSettings(51, sEnableTicketCreate);
	SetDatabaseSettings(52, sEnableTicketMove);
	SetDatabaseSettings(53, sEnableTicketEscalate);
	SetDatabaseSettings(54, sEnableTicketReassign);
	SetDatabaseSettings(55, sEnableTicketRestore);
	SetDatabaseSettings(56, sEnableTicketOpen);
	SetDatabaseSettings(57, sEnableTicketClose);
	SetDatabaseSettings(58, sEnableTicketMessageAdd);
	SetDatabaseSettings(59, sEnableTicketMessageDelete);
	SetDatabaseSettings(67, sTicketListDate);
	SetDatabaseSettings(84, sEnableTicketMessageRevoke);
	SetDatabaseSettings(85, sEnableTicketMessageRelease);
	SetDatabaseSettings(86, sEnableTicketMessageReturn);
	//SetDatabaseSettings(87, s3cxClientVersion);
	SetDatabaseSettings(88, sMailFlowClientVersion);
	SetDatabaseSettings(90, sTicketTracking);
	SetDatabaseSettings(93, sEnableGetOldestSearch);
	SetDatabaseSettings(94, sCloseTicketAfterSend);
	SetDatabaseSettings(95, sRouteRepliesToMe);
	SetDatabaseSettings(96, sSessionTimeout);
	SetDatabaseSettings(97, sKeepAlive);
	SetDatabaseSettings(98, sOnline);
	SetDatabaseSettings(99, sAway);
	SetDatabaseSettings(100, sNotAvail);
	SetDatabaseSettings(101, sOffline);
	SetDatabaseSettings(102, sOoo);
	SetDatabaseSettings(103, sAdminOr);
	SetDatabaseSettings(104, sAgentOr);
	SetDatabaseSettings(113, sEnableTicketLinkCreated);
	SetDatabaseSettings(114, sEnableTicketLinkDeleted);
	SetDatabaseSettings(115, sEnableTicketLinked);
	SetDatabaseSettings(116, sEnableTicketUnlinked);
	SetDatabaseSettings(117, sDateEntryFormat);		
	
	TOfficeHours oh;
	
	// Sunday
    GetISAPIData().GetXMLLong( _T("SunHrStart"), oh.m_StartHr );
	GetISAPIData().GetXMLLong( _T("SunMinStart"), oh.m_StartMin );
	GetISAPIData().GetXMLLong( _T("SunAmPmStart"), oh.m_StartAmPm );
	GetISAPIData().GetXMLLong( _T("SunHrEnd"), oh.m_EndHr );
	GetISAPIData().GetXMLLong( _T("SunMinEnd"), oh.m_EndMin );
	GetISAPIData().GetXMLLong( _T("SunAmPMEnd"), oh.m_EndAmPm );
	
	if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Sunday Office Hours End Time must be after Start Time!") );}

	// get the ID
	GetQuery().Initialize();
	BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
	GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
	                    _T("WHERE TypeID=0 AND ActualID=0 AND DayID=1") );
	GetQuery().Fetch();

	if( oh.m_OfficeHourID > 0 )
	{
		// update
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=1;
		oh.Update( GetQuery() );
	}
	else
	{
		// insert
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=1;
		oh.Insert( GetQuery() );
	}

	// Monday
	GetISAPIData().GetXMLLong( _T("MonHrStart"), oh.m_StartHr );
	GetISAPIData().GetXMLLong( _T("MonMinStart"), oh.m_StartMin );
	GetISAPIData().GetXMLLong( _T("MonAmPmStart"), oh.m_StartAmPm );
	GetISAPIData().GetXMLLong( _T("MonHrEnd"), oh.m_EndHr );
	GetISAPIData().GetXMLLong( _T("MonMinEnd"), oh.m_EndMin );
	GetISAPIData().GetXMLLong( _T("MonAmPMEnd"), oh.m_EndAmPm );

	if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Monday Office Hours End Time must be after Start Time!") );}

	// get the ID
	oh.m_OfficeHourID=0;
	GetQuery().Initialize();
	BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
	GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
	                    _T("WHERE TypeID=0 AND ActualID=0 AND DayID=2") );
	GetQuery().Fetch();

	if( oh.m_OfficeHourID > 0 )
	{
		// update
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=2;
		oh.Update( GetQuery() );
	}
	else
	{
		// insert
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=2;
		oh.Insert( GetQuery() );
	}

	// Tuesday
	GetISAPIData().GetXMLLong( _T("TueHrStart"), oh.m_StartHr );
	GetISAPIData().GetXMLLong( _T("TueMinStart"), oh.m_StartMin );
	GetISAPIData().GetXMLLong( _T("TueAmPmStart"), oh.m_StartAmPm );
	GetISAPIData().GetXMLLong( _T("TueHrEnd"), oh.m_EndHr );
	GetISAPIData().GetXMLLong( _T("TueMinEnd"), oh.m_EndMin );
	GetISAPIData().GetXMLLong( _T("TueAmPMEnd"), oh.m_EndAmPm );

	if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Tuesday Office Hours End Time must be after Start Time!") );}

	// get the ID
	oh.m_OfficeHourID=0;
	GetQuery().Initialize();
	BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
	GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
	                    _T("WHERE TypeID=0 AND ActualID=0 AND DayID=3") );
	GetQuery().Fetch();

	if( oh.m_OfficeHourID > 0 )
	{
		// update
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=3;
		oh.Update( GetQuery() );
	}
	else
	{
		// insert
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=3;
		oh.Insert( GetQuery() );
	}

	// Wednesday
	GetISAPIData().GetXMLLong( _T("WedHrStart"), oh.m_StartHr );
	GetISAPIData().GetXMLLong( _T("WedMinStart"), oh.m_StartMin );
	GetISAPIData().GetXMLLong( _T("WedAmPmStart"), oh.m_StartAmPm );
	GetISAPIData().GetXMLLong( _T("WedHrEnd"), oh.m_EndHr );
	GetISAPIData().GetXMLLong( _T("WedMinEnd"), oh.m_EndMin );
	GetISAPIData().GetXMLLong( _T("WedAmPMEnd"), oh.m_EndAmPm );

	if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Wednesday Office Hours End Time must be after Start Time!") );}

	// get the ID
	oh.m_OfficeHourID=0;
	GetQuery().Initialize();
	BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
	GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
	                    _T("WHERE TypeID=0 AND ActualID=0 AND DayID=4") );
	GetQuery().Fetch();

	if( oh.m_OfficeHourID > 0 )
	{
		// update
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=4;
		oh.Update( GetQuery() );
	}
	else
	{
		// insert
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=4;
		oh.Insert( GetQuery() );
	}

	// Thursday
	GetISAPIData().GetXMLLong( _T("ThuHrStart"), oh.m_StartHr );
	GetISAPIData().GetXMLLong( _T("ThuMinStart"), oh.m_StartMin );
	GetISAPIData().GetXMLLong( _T("ThuAmPmStart"), oh.m_StartAmPm );
	GetISAPIData().GetXMLLong( _T("ThuHrEnd"), oh.m_EndHr );
	GetISAPIData().GetXMLLong( _T("ThuMinEnd"), oh.m_EndMin );
	GetISAPIData().GetXMLLong( _T("ThuAmPMEnd"), oh.m_EndAmPm );

	if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Thursday Office Hours End Time must be after Start Time!") );}

	// get the ID
	oh.m_OfficeHourID=0;
	GetQuery().Initialize();
	BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
	GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
	                    _T("WHERE TypeID=0 AND ActualID=0 AND DayID=5") );
	GetQuery().Fetch();

	if( oh.m_OfficeHourID > 0 )
	{
		// update
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=5;
		oh.Update( GetQuery() );
	}
	else
	{
		// insert
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=5;
		oh.Insert( GetQuery() );
	}

	// Friday
	GetISAPIData().GetXMLLong( _T("FriHrStart"), oh.m_StartHr );
	GetISAPIData().GetXMLLong( _T("FriMinStart"), oh.m_StartMin );
	GetISAPIData().GetXMLLong( _T("FriAmPmStart"), oh.m_StartAmPm );
	GetISAPIData().GetXMLLong( _T("FriHrEnd"), oh.m_EndHr );
	GetISAPIData().GetXMLLong( _T("FriMinEnd"), oh.m_EndMin );
	GetISAPIData().GetXMLLong( _T("FriAmPMEnd"), oh.m_EndAmPm );

	if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Friday Office Hours End Time must be after Start Time!") );}

	// get the ID
	oh.m_OfficeHourID=0;
	GetQuery().Initialize();
	BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
	GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
	                    _T("WHERE TypeID=0 AND ActualID=0 AND DayID=6") );
	GetQuery().Fetch();

	if( oh.m_OfficeHourID > 0 )
	{
		// update
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=6;
		oh.Update( GetQuery() );
	}
	else
	{
		// insert
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=6;
		oh.Insert( GetQuery() );
	}

	// Saturday
	GetISAPIData().GetXMLLong( _T("SatHrStart"), oh.m_StartHr );
	GetISAPIData().GetXMLLong( _T("SatMinStart"), oh.m_StartMin );
	GetISAPIData().GetXMLLong( _T("SatAmPmStart"), oh.m_StartAmPm );
	GetISAPIData().GetXMLLong( _T("SatHrEnd"), oh.m_EndHr );
	GetISAPIData().GetXMLLong( _T("SatMinEnd"), oh.m_EndMin );
	GetISAPIData().GetXMLLong( _T("SatAmPMEnd"), oh.m_EndAmPm );

	if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Saturday Office Hours End Time must be after Start Time!") );}

	// get the ID
	oh.m_OfficeHourID=0;
	GetQuery().Initialize();
	BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
	GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
	                    _T("WHERE TypeID=0 AND ActualID=0 AND DayID=7") );
	GetQuery().Fetch();

	if( oh.m_OfficeHourID > 0 )
	{
		// update
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=7;
		oh.Update( GetQuery() );
	}
	else
	{
		// insert
		oh.m_TypeID=0;
		oh.m_ActualID=0;
		oh.m_DayID=7;
		oh.Insert( GetQuery() );
	}
	
	GetISAPIData().m_SessionMap.QueueSessionMonitors(2,0);
	GetISAPIData().m_SessionMap.QueueSessionMonitors(1,0);
	GetISAPIData().m_SessionMap.InvalidateAllAgentSessions();
	InvalidateServerParameters(true);
	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_ServerParameters );
	g_ThreadPool.ReLoadAllThreads();
}

bool CustomSettings::IsTimeLater(TOfficeHours oh)
{
	if(oh.m_StartAmPm==0 || oh.m_StartHr==0 || oh.m_StartMin==60 || oh.m_EndAmPm==0 || oh.m_EndHr==0 || oh.m_EndMin==60)
	{
		if(oh.m_StartAmPm!=0 || oh.m_StartHr!=0 || oh.m_StartMin!=60 || oh.m_EndAmPm!=0 || oh.m_EndHr!=0 || oh.m_EndMin!=60)
		{
			THROW_EMS_EXCEPTION( E_InvalidParameters, _T("Invalid Time in Office Hours!") );
		}
		else
		{
			return true;
		}
	}
		
	TIMESTAMP_STRUCT StartTime;
	TIMESTAMP_STRUCT EndTime;
	GetTimeStamp( StartTime );
	GetTimeStamp( EndTime );
	if(oh.m_StartHr == 12 && oh.m_StartAmPm == 1)
	{
		StartTime.hour = 0;
	}
	else if(oh.m_StartAmPm == 2 && oh.m_StartHr == 12)
	{
		StartTime.hour = 12;
	}
	else if (oh.m_StartAmPm == 2)
	{
		StartTime.hour = oh.m_StartHr + 12;
	}
	else
	{
		StartTime.hour = oh.m_StartHr;
	}
	StartTime.minute = oh.m_StartMin;
	StartTime.second = 0;
	StartTime.fraction = 0;
	if(oh.m_EndHr == 12 && oh.m_EndAmPm == 1)
	{
		EndTime.hour = 0;
	}
	else if(oh.m_EndAmPm == 2 && oh.m_EndHr == 12)
	{
		EndTime.hour = 12;
	}
	else if (oh.m_EndAmPm == 2)
	{
		EndTime.hour = oh.m_EndHr + 12;
	}
	else
	{
		EndTime.hour = oh.m_EndHr;
	}
	EndTime.minute = oh.m_EndMin;
	EndTime.second = 0;
	EndTime.fraction = 0;
	BOOL bIsFuture = OrderTimeStamps( StartTime , EndTime );	
	if(bIsFuture == 0)
	{
		return false;
	}
	
	return true;
}
void CustomSettings::SetXMLOutput(int nID, LPCTSTR lpDefault, LPCTSTR lpChildElem)
{
	TServerParameters servParams;
	servParams.m_ServerParameterID = nID;

	int nResult = servParams.Query(GetQuery());

	if(nResult != S_OK)
		lstrcpy(servParams.m_DataValue, lpDefault);

	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(lpChildElem, servParams.m_DataValue);
	GetXMLGen().OutOfElem();
}

void CustomSettings::SetDatabaseSettings(int nID, std::string& sDataValue)
{
	int m_ServerParameterID = nID;
	long m_ServerParameterIDLen = 0;
	TCHAR m_DataValue[SERVERPARAMETERS_DATAVALUE_LENGTH];
	long m_DataValueLen = 0;

	ZeroMemory(m_DataValue, SERVERPARAMETERS_DATAVALUE_LENGTH);

	lstrcpyn(m_DataValue, sDataValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1);

	GetQuery().Initialize();
	BINDPARAM_TCHAR(GetQuery(), m_DataValue );
	BINDPARAM_LONG(GetQuery(), m_ServerParameterID );
	GetQuery().Execute( _T("UPDATE ServerParameters ")
	                    _T("SET DataValue=? ")
	                    _T("WHERE ServerParameterID=?") );
}
