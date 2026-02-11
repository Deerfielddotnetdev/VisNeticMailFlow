#include "stdafx.h"
#include "MsgRouter.h"
#include "ContactFns.h"
#include "EMSMutex.h"
#include "MessageIO.h"
#include "AttachFns.h"
#include "OutboundMsg.h"
#include "TicketHistoryFns.h"
#include ".\msgrouter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMsgRouter::CMsgRouter( CODBCQuery& query ) : m_query(query), m_msg(query)
{
	m_MRState = MRState_UnInitialized;
	m_nLastState = m_MRState;
	m_nFailedCnt = 0;
	m_LastRefreshTime = 0;	// Never

}

CMsgRouter::~CMsgRouter()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// RefreshRules
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::RefreshRules(void)
{
	TMessageSources MsgSrc;
	dca::String t;

	m_rules.Refresh( m_query );
	m_rulesP.Refresh( m_query );

	if( m_LastRefreshTime != 0 )
	{
		if ( GetTickCount() - m_LastRefreshTime < RefreshRuleIntervalMs )
		{
			m_MRState = MRState_GetMessage;
			return;
		}
	}

	m_MsgSrcs.clear();

	MsgSrc.PrepareList( m_query );

	while( m_query.Fetch() == S_OK )
	{
		m_MsgSrcs[MsgSrc.m_MessageSourceID] = MsgSrc;
	}

	m_AgentStatusMap.clear();
	int nAgentID;
	int nStatusID;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nAgentID );
	BINDCOL_LONG_NOLEN( m_query, nStatusID );
	m_query.Execute( L"SELECT AgentID,StatusID FROM Agents WHERE IsDeleted=0 AND IsEnabled=1" );
	while( m_query.Fetch() == S_OK )
	{
		m_AgentStatusMap.insert( pair<unsigned int,unsigned int> (nAgentID, nStatusID) );
	}
	
	m_Bypass.clear();
	TBypass b;
	int nValue = _wtoi( g_Object.GetParameter(EMS_SRVPARAM_AGENT_EMAIL_BYPASS));
	int nMax = 0;
	if(nValue == 1)
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, nMax );
		m_query.Execute( _T("SELECT CASE WHEN MAX(EmailID) IS NULL THEN 1 ELSE MAX(EmailID) END FROM Email") );
		m_query.Fetch();		
	}
	
	m_query.Initialize();
	BINDCOL_LONG( m_query, b.m_BypassID );
	BINDCOL_LONG( m_query, b.m_BypassTypeID );
	BINDCOL_TCHAR( m_query, b.m_BypassValue );
	m_query.Execute(	_T("SELECT EmailID,EmailTypeID,EmailValue ")
		            _T("FROM Email WHERE EmailTypeID < 2 ORDER BY EmailValue") );
	
	while( m_query.Fetch() == S_OK )
	{
		tstring w(b.m_BypassValue);
		dca::String x(w.c_str());
		t.Format("CMsgRouter::RefreshRules - Adding Tracking Bypass Value: [%s] TypeID: [%d] to m_Bypass map", x.c_str(), b.m_BypassTypeID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		
		m_Bypass.insert( pair<int,TBypass> (b.m_BypassID, b) );
	}

	if(nValue == 1)
	{
		TCHAR AgentAddress[PERSONALDATA_DATAVALUE_LENGTH];
		long AgentAddressLen;
		int AgentID;

		m_query.Initialize();
		
		BINDCOL_LONG_NOLEN( m_query, AgentID );
		BINDCOL_TCHAR( m_query, AgentAddress );
		
		m_query.Execute(	_T("SELECT Agents.AgentID, DataValue ")
						_T("FROM Agents ")
						_T("INNER JOIN PersonalData on PersonalData.AgentID = Agents.AgentID ")
						_T("WHERE PersonalDataTypeID=1 AND Agents.IsEnabled=1 AND Agents.IsDeleted=0 ")
						_T("ORDER BY DataValue") );
		
		while( m_query.Fetch() == S_OK )
		{	
			if ( AgentAddressLen > 0 )
			{
				nMax++;
				b.m_BypassID = nMax;
				b.m_BypassTypeID = 0;
				_tcscpy(b.m_BypassValue, AgentAddress);
				
				tstring w(b.m_BypassValue);
				dca::String x(w.c_str());
				t.Format("CMsgRouter::RefreshRules - Adding Agent Email: [%s] TypeID: [%d] to m_Bypass map", x.c_str(), b.m_BypassTypeID);
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

				m_Bypass.insert( pair<int,TBypass> (b.m_BypassID, b) );				
			}
		}
	}

	// Set the last refresh time
	m_LastRefreshTime = GetTickCount();

	m_MRState = MRState_GetMessage;
}

////////////////////////////////////////////////////////////////////////////////
// 
// WalkLockedMessages
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::WalkLockedMessages( tstring& sMsgIDs )
{
	map<unsigned int,unsigned int>::iterator iter;
	CEMSString sMsgID;
	int nLockedBy;

	iter = m_LockedTicketMap.begin();

	// Build a comma-delimited list of InboundMessageIDs belonging to locked tickets.
	while( iter != m_LockedTicketMap.end() )
	{
		// Check if ticket has been unlocked

		m_query.Initialize();
		BINDPARAM_LONG( m_query, iter->second );
		BINDCOL_LONG_NOLEN( m_query, nLockedBy );
		m_query.Execute( L"SELECT LockedBy FROM Tickets WHERE TicketID=?" );

		// If this ticket is unlocked, removed the entry from the list
		if( m_query.Fetch() == S_OK && nLockedBy == 0 )
		{
			iter = m_LockedTicketMap.erase( iter );
		}
		else
		{
			sMsgID.Format( _T("%d,"), iter->first );
			sMsgIDs.append( sMsgID );
			iter++;
		}
	}
	
	// Remove the last comma
	if( sMsgIDs.size() > 0 )
	{
		sMsgIDs.resize( sMsgIDs.size()-1 );
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetNewMessage
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::GetNewMessage( void )
{
	tstring sLockedMsgIDs;

	WalkLockedMessages( sLockedMsgIDs );

	// initialize algorithm parameters
	m_AutoReplyCloseTicketFlag = false;
	m_bGotNewMsg = false;
	m_RoutingRuleID = 0;
	m_bNewTicket = false;
	m_bNewContact = false;
	
	ZeroMemory( &m_tkt, sizeof(m_tkt) );
	ZeroMemory( m_szTicketBoxName, sizeof(m_szTicketBoxName) );
	ZeroMemory( m_szContactName, sizeof(m_szContactName) );

	//DebugReporter::Instance().DisplayMessage("CMsgRouter::FindRuleMatch - Look for a message in inbound queue", DebugReporter::ENGINE);

	// Look for a message in the inbound queue
	if( m_msg.GetNewMessage( sLockedMsgIDs ) == S_OK )
	{
		m_bGotNewMsg = true;
		m_MRState = MRState_CheckContact;

		// Get the MessageSourcTypeID from the map of Message Sources
		map<int,TMessageSources>::iterator iter = m_MsgSrcs.find( m_msg.m_MessageSourceID );
		if( iter != m_MsgSrcs.end() )
		{
			m_msg.m_MessageSourceTypeID =  iter->second.m_MessageSourceTypeID;
			Log( E_GotMessage, EMS_STRING_RTR_NEW_MESSAGE, iter->second.m_Description );
		}
		else
		{
			// unknown message source
			m_msg.m_MessageSourceTypeID = 0;
			Log( E_GotMessage, EMS_STRING_RTR_NEW_MESSAGE, _T("Unknown") );
		}
			
		return;
	}
	
	m_MRState = MRState_Finished;
}


////////////////////////////////////////////////////////////////////////////////
// 
// CheckContact - See if contact exists in database. If not, create one.
// 
// Sets the m_ContactID and m_AgentID member on return
//
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::CheckContact( void )
{
	m_msg.m_ContactID = GetContactIDFromEmail( m_query, m_msg.m_EmailFrom, 
		                                       m_msg.m_EmailFromName, m_bNewContact );

	// Make sure we were successful
	if( m_msg.m_ContactID == 0)
		return;

	if( m_bNewContact )
	{
		// Log it
		if ( _tcslen( m_msg.m_EmailFromName ) > 0 )
		{
			Log( E_CreatedContact, EMS_STRING_RTR_NEW_CONTACT,
				 m_msg.m_EmailFromName, m_msg.m_InboundMessageID );
			_tcscpy( m_szContactName, m_msg.m_EmailFromName );
		}
		else
		{
			Log( E_CreatedContact, EMS_STRING_RTR_NEW_CONTACT,
				 m_msg.m_EmailFrom, m_msg.m_InboundMessageID );
			_tcscpy( m_szContactName, m_msg.m_EmailFrom );
		}
	}
	else
	{
		GetContactName();
	}

	m_MRState = MRState_FindRuleMatch;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetContactName
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::GetContactName(void)
{
	long m_szContactNameLen;
	long m_szContactEmailLen;
	
	// query for contact name
	m_query.Reset();
	BINDPARAM_LONG( m_query, m_msg.m_ContactID );
	BINDCOL_WCHAR( m_query, m_szContactName );
	BINDCOL_WCHAR( m_query, m_szContactEmail );

	m_query.Execute( _T("SELECT Contacts.Name,PersonalData.DataValue ")
					 _T("FROM Contacts INNER JOIN PersonalData ON Contacts.DefaultEmailAddressID = PersonalData.PersonalDataID ")
					 _T("WHERE Contacts.ContactID=?") );
	m_query.Fetch();

	if( m_szContactName[0] == 0 )
	{
		_tcscpy( m_szContactName, m_szContactEmail );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// ImplicitRoutingRule
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::ImplicitRoutingRule(void)
{
	bool bMatch = false;
	int nMatchAgentAlg = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_MATCH_AGENT ) );
	int nMatchTicketBoxAlg = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_MATCH_TICKETBOX ) );
	int nMatchAgentID = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_MATCH_AGENTID ) );
	int nMatchTicketBoxID = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_MATCH_TICKETBOXID ) );
	int nMatchTicketCategoryID = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_MATCH_TICKETCATEGORYID ) );
	int nDenyOnline = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_ONLINE ) );
	int nDenyAway = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_AWAY ) );
	int nDenyNotAvail = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_NOTAVAIL ) );
	int nDenyOffline = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_OFFLINE ) );
	int nDenyOoo = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_OOO ) );	
	
	// Set default priority
	m_tkt.m_PriorityID = EMS_PRIORITY_NORMAL;
	m_RoutingRuleID = 0; // no rule matched

	// Set the default ticket category
	m_tkt.m_TicketCategoryID = nMatchTicketCategoryID;

	AssignToTicketBoxAlgorithm( nMatchTicketBoxAlg, nMatchTicketBoxID );

	// Just in case
	if( m_tkt.m_TicketBoxID == 0 )
	{
		m_tkt.m_TicketBoxID = EMS_TICKETBOX_UNASSIGNED;
	}

	// Just in case
	if( m_tkt.m_TicketCategoryID == 0 )
	{
		m_tkt.m_TicketCategoryID = 1;
	}

	AssignToAgentAlgorithm( nMatchAgentAlg, nMatchAgentID, nDenyOnline, nDenyAway, nDenyNotAvail, nDenyOffline, nDenyOoo );
}


////////////////////////////////////////////////////////////////////////////////
// 
// AssignToTicketBoxWithMatchingAddress
// 
////////////////////////////////////////////////////////////////////////////////
bool CMsgRouter::AssignToTicketBoxWithMatchingAddress(void)
{
	if( wcslen( m_msg.m_EmailPrimaryTo ) > 0 )
	{
		// does a ticketbox have a matching email address?
		m_query.Reset( true );
		BINDPARAM_WCHAR( m_query, m_msg.m_EmailPrimaryTo );
		BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
		BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
		m_query.Execute( L"SELECT TicketBoxID,Name FROM TicketBoxes "
						 L"WHERE DefaultEmailAddress=? " );

		if( m_query.Fetch() == S_OK )
		{
			return true;
		}

		m_query.Reset( true );
		BINDPARAM_WCHAR( m_query, m_msg.m_EmailPrimaryTo );
		BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
		BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
		m_query.Execute( L"SELECT ABS(pd.AgentID),tb.Name FROM PersonalData pd "
						 L"INNER JOIN TicketBoxes tb ON ABS(pd.AgentID)=tb.TicketBoxID "
						 L"WHERE pd.DataValue=? AND pd.AgentID < 0 " );

		if( m_query.Fetch() == S_OK )
		{
			return true;
		}
	}
	
	list<EmailAddr_t>::iterator iter;
	list<EmailAddr_t> emaillist;

	ProcessEmailAddrString( m_msg.m_EmailTo, emaillist, false );

	for( iter = emaillist.begin(); (iter != emaillist.end()) && (m_tkt.m_TicketBoxID == 0); iter++ )
	{
		// does a ticketbox have a matching email address?
		m_query.Reset( true );
		BINDPARAM_WCHAR( m_query, (wchar_t*)iter->m_sEmailAddr.c_str() );
		BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
		BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
		m_query.Execute( L"SELECT TicketBoxID,Name FROM TicketBoxes "
						 L"WHERE DefaultEmailAddress=? " );
		if( m_query.Fetch() == S_OK )
			return true;

		m_query.Reset( true );
		BINDPARAM_WCHAR( m_query, (wchar_t*)iter->m_sEmailAddr.c_str() );
		BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
		BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
		m_query.Execute( L"SELECT ABS(pd.AgentID),tb.Name FROM PersonalData pd "
						 L"INNER JOIN TicketBoxes tb ON ABS(pd.AgentID)=tb.TicketBoxID "
						 L"WHERE pd.DataValue=? AND pd.AgentID < 0 " );

		if( m_query.Fetch() == S_OK )
			return true;		
	}

	emaillist.clear();
	ProcessEmailAddrString( m_msg.m_EmailCc, emaillist, false );

	for( iter = emaillist.begin(); (iter != emaillist.end()) && (m_tkt.m_TicketBoxID == 0); iter++ )
	{
		// does a ticketbox have a matching email address?
		m_query.Reset( true );
		BINDPARAM_WCHAR( m_query, (wchar_t*)iter->m_sEmailAddr.c_str() );
		BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
		BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
		m_query.Execute( L"SELECT TicketBoxID,Name FROM TicketBoxes "
						 L"WHERE DefaultEmailAddress=? " );
		if( m_query.Fetch() == S_OK )
			return true;

		m_query.Reset( true );
		BINDPARAM_WCHAR( m_query, (wchar_t*)iter->m_sEmailAddr.c_str() );
		BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
		BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
		m_query.Execute( L"SELECT ABS(pd.AgentID),tb.Name FROM PersonalData pd "
						 L"INNER JOIN TicketBoxes tb ON ABS(pd.AgentID)=tb.TicketBoxID "
						 L"WHERE pd.DataValue=? AND pd.AgentID < 0 " );

		if( m_query.Fetch() == S_OK )
			return true;		
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// AssignToTicketBoxByPercent
// 
////////////////////////////////////////////////////////////////////////////////
bool CMsgRouter::AssignToTicketBoxByPercent(void)
{
	
	unsigned int nTotalPercent=0;
	unsigned int nTotalTickets=0;
	int nTicketBoxSelected=0;
	double nPercentDifference=0;
	int nRoundRobin;
	int nMinTicketBoxID;
	int nVal = 0;
	dca::String t;
	
	DebugReporter::Instance().DisplayMessage("CMsgRouter::AssignToTicketBoxByPercent - Entering", DebugReporter::ENGINE);
	
	nRoundRobin = GetRegDword( L"RoundRobin" );
	
	if( m_pRule != NULL )
	{
		// Do TicketBox Routing records exist for this Routing Rule?
		m_query.Reset();
		m_query.Initialize();
		BINDPARAM_LONG( m_query, m_pRule->m_RoutingRuleID );
		BINDCOL_LONG( m_query, attb.m_TicketBoxID );
		BINDCOL_LONG( m_query, attb.m_Percentage );
		BINDCOL_LONG( m_query, attb.m_TicketBoxRoutingID );
		if ( m_pRule->m_ConsiderAllOwned )
		{
			m_query.Execute( L"SELECT TicketBoxID,Percentage, "
							 L"(SELECT COUNT(*) FROM Tickets INNER JOIN TicketBoxes ON Tickets.TicketBoxID=TicketBoxes.TicketBoxID "
							 L"WHERE Tickets.TicketStateID<>1 AND Tickets.TicketStateID<>5 AND Tickets.IsDeleted=0 AND "
							 L"TicketBoxes.OwnerID=(SELECT TicketBoxes.OwnerID FROM TicketBoxes WHERE TicketBoxes.TicketBoxID=TBR.TicketBoxID)) "
							 L"FROM TicketBoxRouting AS TBR "
							 L"WHERE RoutingRuleID=? Order By TBR.TicketBoxID" );
		}
		else
		{
			m_query.Execute( L"SELECT TicketBoxID,Percentage, "
							 L"(SELECT COUNT(*) FROM Tickets WHERE TicketBoxID=TBR.TicketBoxID AND (Tickets.TicketStateID <> 1 AND Tickets.TicketStateID <> 5) AND Tickets.IsDeleted=0)"
							 L"FROM TicketBoxRouting AS TBR "
							 L"WHERE RoutingRuleID=?  Order By TBR.TicketBoxID" );	
		}
		
		m_attb.clear();
		while( m_query.Fetch() == S_OK )
		{
			m_attb.push_back( attb );
			nTotalPercent = nTotalPercent + attb.m_Percentage;
			nTotalTickets = nTotalTickets + attb.m_TicketBoxRoutingID;			
		}
		
		t.Format("CMsgRouter::AssignToTicketBoxByPercent - Total Percent [%d] Total Tickets [%d]", nTotalPercent, nTotalTickets);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

		if ( nTotalPercent == 100 )
		{
			for( attbIter = m_attb.begin(); attbIter != m_attb.end(); attbIter++ )
			{
				int nTicketBoxID = attbIter->m_TicketBoxID;
				int nTicketBoxPercentage = attbIter->m_Percentage;
				int nNumTicketBoxTickets = attbIter->m_TicketBoxRoutingID;
				
				double nTicketBoxPercentOfTotal = ((double)nNumTicketBoxTickets/nTotalTickets)*100;
				
				t.Format("CMsgRouter::AssignToTicketBoxByPercent - TicketBoxID [%d] Ticket Box Percent [%d] Number of Tickets [%d] Percent of Total [%d]", attbIter->m_TicketBoxID, attbIter->m_Percentage, attbIter->m_TicketBoxRoutingID, (int)nTicketBoxPercentOfTotal);
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);			
				
				if ( nVal == 0 )
				{
					nMinTicketBoxID = nTicketBoxID;
				}
				nVal++;

				if ( nRoundRobin >= 0 )
				{
					
					t.Format("CMsgRouter::AssignToTicketBoxByPercent - RoundRobin [%d]", nRoundRobin);
					DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
					
					if ( nTicketBoxID > nRoundRobin )
					{
						nTicketBoxSelected = nTicketBoxID;
						SetRegDword( L"RoundRobin", nTicketBoxID );
						t.Format("CMsgRouter::AssignToTicketBoxByPercent - Returning True TicketBox Selected [%d]",  nTicketBoxSelected);
						DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

						m_tkt.m_TicketBoxID = nTicketBoxSelected;
						return true;
					}
				}
				else if ( (nTicketBoxPercentOfTotal <= (double)nTicketBoxPercentage) || (nNumTicketBoxTickets == 0) )
				{
					if ( (((double)nTicketBoxPercentage-nTicketBoxPercentOfTotal) >= (double)nPercentDifference) || (nNumTicketBoxTickets == 0) )
					{
						nTicketBoxSelected = nTicketBoxID;
						nPercentDifference = nTicketBoxPercentage-nTicketBoxPercentOfTotal;
					}
				}
			}
		}
			
		if ( nTicketBoxSelected > 0 )
		{
			
			t.Format("CMsgRouter::AssignToTicketBoxByPercent - Returning True TicketBox Selected [%d]",  nTicketBoxSelected);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

			m_tkt.m_TicketBoxID = nTicketBoxSelected;
			return true;
		}
		else if ( nRoundRobin >= 0 && nMinTicketBoxID >= 0 )
		{
			nTicketBoxSelected = nMinTicketBoxID;
			SetRegDword( L"RoundRobin", nMinTicketBoxID );
			t.Format("CMsgRouter::AssignToTicketBoxByPercent - Returning True TicketBox Selected [%d]",  nTicketBoxSelected);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

			m_tkt.m_TicketBoxID = nTicketBoxSelected;
			return true;
		}
	}
	
	DebugReporter::Instance().DisplayMessage("CMsgRouter::AssignToTicketBoxByPercent - Returning False", DebugReporter::ENGINE);

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// AssignToTicketBoxAlgorithm
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::AssignToTicketBoxAlgorithm( int AlgID, int TicketBoxID )
{
	dca::String t;

	switch( AlgID )
	{
	case 0:
		m_tkt.m_TicketBoxID = TicketBoxID;
		t.Format("CMsgRouter::AssignToTicketBoxAlgorithm - Assigning InboundMessageID [%d] to Default TicketBoxID [%d]", m_msg.m_InboundMessageID, m_tkt.m_TicketBoxID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		break;

	case 1:
		if( AssignToTicketBoxWithMatchingAddress() == false )
		{
			m_tkt.m_TicketBoxID = TicketBoxID;
			t.Format("CMsgRouter::AssignToTicketBoxAlgorithm - Assign to TicketBox with Matching Email Address false, Assigning InboundMessageID [%d] to Default TicketBoxID [%d]", m_msg.m_InboundMessageID, m_tkt.m_TicketBoxID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
		}
		else
		{
			t.Format("CMsgRouter::AssignToTicketBoxAlgorithm - Assign to TicketBox with Matching Email Address true, Assigning InboundMessageID [%d] to TicketBoxID [%d]", m_msg.m_InboundMessageID, m_tkt.m_TicketBoxID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
		}
		break;
	case 2:
		if( AssignToTicketBoxByPercent() == false )
		{
			m_tkt.m_TicketBoxID = TicketBoxID;
			t.Format("CMsgRouter::AssignToTicketBoxAlgorithm - Assign to TicketBox by Percent false, Assigning InboundMessageID [%d] to Default TicketBoxID [%d]", m_msg.m_InboundMessageID, m_tkt.m_TicketBoxID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
		}
		else
		{
			t.Format("CMsgRouter::AssignToTicketBoxAlgorithm - Assign to TicketBox by Percent true, Assigning InboundMessageID [%d] to TicketBoxID [%d]", m_msg.m_InboundMessageID, m_tkt.m_TicketBoxID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
		}
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// FindRuleMatch
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::FindRuleMatch(void)
{
	dca::String t;
	t.Format("CMsgRouter::FindRuleMatch - Entered for InboundMessageID [%d]", m_msg.m_InboundMessageID);
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	
	// Scan routing rules for match
	m_pRule = m_rules.GetRuleMatch( m_msg );
	
	if( m_pRule == NULL )
	{
		// No rule matched
		t.Format("CMsgRouter::FindRuleMatch - No rule matched for InboundMessageID [%d], processing Default Routing Rule", m_msg.m_InboundMessageID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
				
		ImplicitRoutingRule();
	}
	else
	{
		dca::String x(m_pRule->m_RuleDescrip);
		t.Format("CMsgRouter::FindRuleMatch - Rule [%s] matched for InboundMessageID [%d]", x.c_str(), m_msg.m_InboundMessageID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		
		AssignToTicketBoxAlgorithm( m_pRule->m_AssignToTicketBoxAlg, m_pRule->m_AssignToTicketBox );

		m_RoutingRuleID = m_pRule->m_RoutingRuleID;
		m_tkt.m_TicketCategoryID = m_pRule->m_AssignToTicketCategory;

		if( m_pRule->m_PriorityID == 0 )
		{
			// Use the message priority
			m_tkt.m_PriorityID = m_msg.m_PriorityID;
		}
		else
		{
			// Assign the priority
			m_tkt.m_PriorityID = m_pRule->m_PriorityID;
		}
		
		if ( m_pRule->m_DoProcessingRules )
		{
			// Scan processing rules for match
			m_pRuleP = m_rulesP.GetRuleMatch( m_msg );
			
			if( m_pRuleP == NULL )
			{
				// No rule matched
				t.Format("CMsgRouter::FindRuleMatch - No Processing Rule matched for InboundMessageID [%d]", m_msg.m_InboundMessageID);
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
				
			}
			else
			{
				dca::String x(m_pRuleP->m_RuleDescrip);
				t.Format("CMsgRouter::FindRuleMatch - Processing Rule [%s] matched for InboundMessageID [%d]", x.c_str(), m_msg.m_InboundMessageID);
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		
				m_ProcessingRuleID = m_pRuleP->m_ProcessingRuleID;
				m_tkt.m_TicketCategoryID = m_pRuleP->m_ActionID;
				m_ActionID = m_pRuleP->m_ActionID;
				m_pRuleP->BumpHitCount( m_query );
				Log( E_RuleMatch, EMS_STRING_RTR_RULE_MATCH,  m_pRuleP->m_RuleDescrip  );
			}
		}
	}

	m_MRState = MRState_BumpHitCount;
}

////////////////////////////////////////////////////////////////////////////////
// 
// BumpHitCount
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::BumpHitCount( void )
{
	// nothing to do if no rule matched!
	if( m_RoutingRuleID != 0 )
	{		
		m_pRule->BumpHitCount( m_query );

		Log( E_RuleMatch, EMS_STRING_RTR_RULE_MATCH,  m_pRule->m_RuleDescrip  );
	}

	m_MRState = MRState_PutInTicketBox;
}


////////////////////////////////////////////////////////////////////////////////
// 
// PutInTicketBox
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::PutInTicketBox( void )
{
	int nTicketID = 0;
	int nReplyToMsgID = 0;
	dca::String t;

	// skip this step if the rule says to delete the message
	if( m_RoutingRuleID != 0 && 
		m_pRule->m_DeleteImmediatelyEnable &&
		m_pRule->m_DeleteImmediately == EMS_DELETE_OPTION_DELETE_PERMANENTLY )
	{
		m_MRState = MRState_CreateAutoReplies;

		return;
	}

	int nTicketTracking = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_TICKET_TRACKING ) );	

	// Check if the MailComponents set the ticketID
	if( m_msg.m_TicketID && GetPreRoutedTicketInfo() )
	{
		// TicketID and ReplyToMsgID are already correct and should
		// have everything else we need to route this message
		m_tkt.m_TicketID = m_msg.m_TicketID;		
		t.Format("CMsgRouter::PutInTicketBox - TicketID [%d] for InboundMessageID [%d] previously set by MailComponents", m_tkt.m_TicketID, m_msg.m_InboundMessageID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	}
	else if( GetTicketIDFromHeader(m_msg.m_InboundMessageID, nTicketID, nReplyToMsgID ) && 
		GetTicketInfo( nTicketID, nReplyToMsgID ))
	{
		m_msg.m_TicketID = m_tkt.m_TicketID;
		t.Format("CMsgRouter::PutInTicketBox - TicketID [%d] for InboundMessageID [%d] found in Header", m_tkt.m_TicketID, m_msg.m_InboundMessageID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	}	
	else if( GetTicketIDFromSubject( m_msg.m_Subject, nTicketID, nReplyToMsgID ) && 
		GetTicketInfo( nTicketID, nReplyToMsgID ))
	{
		m_msg.m_TicketID = m_tkt.m_TicketID;
		t.Format("CMsgRouter::PutInTicketBox - TicketID [%d] for InboundMessageID [%d] found in Subject", m_tkt.m_TicketID, m_msg.m_InboundMessageID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	}
	else if( GetTicketIDFromBody(m_msg.m_Body, nTicketID, nReplyToMsgID ) && 
		GetTicketInfo( nTicketID, nReplyToMsgID ))
	{
		m_msg.m_TicketID = m_tkt.m_TicketID;
		t.Format("CMsgRouter::PutInTicketBox - TicketID [%d] for InboundMessageID [%d] found in Body", m_tkt.m_TicketID, m_msg.m_InboundMessageID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	}	
	else
	{
		// check the destination TicketBox for an open ticket
		// from the same contact with a matching subject.
		dca::WString sMsgSubject(m_msg.m_Subject);
		dca::WString sSQLSubject( _T("%") );
		int nTicketID = 0;
		TCHAR szSubject[TICKETS_SUBJECT_LENGTH] = {0};

		// if we don't find a match we will need to
		// create a new ticket
		m_bNewTicket = true;
		bool bAssignNew = false;

		if(m_pRule)
		{
			if(m_pRule->m_AssignUniqueTicketID == 1)
				bAssignNew = true;
		}
		else
		{
			 if(_wtoi( g_Object.GetParameter(EMS_SRVPARAM_DEF_RR_NEW_TICKET)))
				 bAssignNew = true;
		}

		if(bAssignNew != true)
		{
			t.Format("CMsgRouter::PutInTicketBox - TicketID for InboundMessageID [%d] not found, checking for matching Subject and Contact", m_msg.m_InboundMessageID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
			
			// remove any prefixes (re:,fwd:,etc) from the message's subject
			CleanupSubject(sMsgSubject);
			dca::WString sTempSubject(sMsgSubject);

			int nUseEsc = 0;
			TCHAR cEscChar[2];
			cEscChar[0] = 0x0000;
			cEscChar[1] = 0x0000;

			if(SetEscapeCharacter(cEscChar, sTempSubject))
			{
				if(UpdateSubjectWithEscape(sTempSubject, 0x005B, cEscChar))
					nUseEsc = 1;

				if(UpdateSubjectWithEscape(sTempSubject, 0x005D, cEscChar))
					nUseEsc = 1;

				if(UpdateSubjectWithEscape(sTempSubject, 0x0025, cEscChar))
					nUseEsc = 1;

				if(UpdateSubjectWithEscape(sTempSubject, 0x005E, cEscChar))
					nUseEsc = 1;

				if(UpdateSubjectWithEscape(sTempSubject, 0x005F, cEscChar))
					nUseEsc = 1;
			}

			sSQLSubject.append(sTempSubject);

			int nMatchMethod = 0;
			
			if(m_pRule)
			{
				nMatchMethod = m_pRule->m_MatchMethod;					
			}
			else
			{
				nMatchMethod = _wtoi( g_Object.GetParameter(EMS_SRVPARAM_DEF_RR_MATCH_METHOD));
			}

			m_query.Initialize();
			BINDPARAM_LONG( m_query, m_tkt.m_TicketBoxID );
			BINDPARAM_LONG( m_query, m_msg.m_ContactID );
			BINDPARAM_TCHAR( m_query, (void*) sSQLSubject.c_str() );
			BINDCOL_LONG_NOLEN( m_query, nTicketID );
			BINDCOL_TCHAR_NOLEN( m_query, szSubject );

			if(nMatchMethod == 0 )
			{
				CEMSString cmd;
				if(nUseEsc)
					cmd.Format(_T("SELECT Tickets.TicketID,Subject FROM Tickets INNER JOIN TicketContacts ON Tickets.TicketID = TicketContacts.TicketID WHERE TicketBoxID=? AND ContactID=? AND TicketStateID <> 1 AND IsDeleted=0 AND Subject LIKE ? ESCAPE '%s' ORDER BY OpenTimestamp DESC"), cEscChar);
				else
					cmd.Format(_T("SELECT Tickets.TicketID,Subject FROM Tickets INNER JOIN TicketContacts ON Tickets.TicketID = TicketContacts.TicketID WHERE TicketBoxID=? AND ContactID=? AND TicketStateID <> 1 AND IsDeleted=0 AND Subject LIKE ? ORDER BY OpenTimestamp DESC"));

				m_query.Execute(cmd.c_str());

				dca::WString sTicketSubject;

				while ( m_query.Fetch() == S_OK )
				{
					// remove any prefixes from the ticket's subject
					sTicketSubject = szSubject;
					CleanupSubject(sTicketSubject);

					// compare the message's and ticket's subjects (without any prefixes)
					if ( _tcsicmp(sMsgSubject.c_str(), sTicketSubject.c_str()) == 0 )
					{
						// if the two match, use this ticket
						m_tkt.m_TicketID = nTicketID;
						m_tkt.Query(m_query);
						m_msg.m_TicketID = m_tkt.m_TicketID;
						m_bNewTicket = false;

						break;
					}
				}
			}
			else if (nMatchMethod == 1)
			{
				BINDCOL_LONG_NOLEN( m_query, nReplyToMsgID );
				CEMSString cmd;
				if(nUseEsc)					
					cmd.Format(_T("SELECT TOP 1 om.TicketID,om.Subject,om.OutboundMessageID FROM OutboundMessages om INNER JOIN TicketContacts tc ON om.TicketID = tc.TicketID INNER JOIN Tickets t ON om.TicketID=t.TicketID WHERE t.TicketBoxID=? AND tc.ContactID=? AND t.IsDeleted=0 AND om.IsDeleted=0 AND om.Subject LIKE ? ESCAPE '%s' ORDER BY om.EmailDateTime DESC"), cEscChar);
				else
					cmd.Format(_T("SELECT TOP 1 om.TicketID,om.Subject,om.OutboundMessageID FROM OutboundMessages om INNER JOIN TicketContacts tc ON om.TicketID = tc.TicketID INNER JOIN Tickets t ON om.TicketID=t.TicketID WHERE t.TicketBoxID=? AND tc.ContactID=? AND t.IsDeleted=0 AND om.IsDeleted=0 AND om.Subject LIKE ? ORDER BY om.EmailDateTime DESC"));
				
				m_query.Execute(cmd.c_str());

				dca::WString sTicketSubject;

				if ( m_query.Fetch() == S_OK )
				{
					// remove any prefixes from the ticket's subject
					sTicketSubject = szSubject;
					CleanupSubject(sTicketSubject);

					// compare the message's and ticket's subjects (without any prefixes)
					if ( _tcsicmp(sMsgSubject.c_str(), sTicketSubject.c_str()) == 0 )
					{
						// if the two match, use this ticket
						m_tkt.m_TicketID = nTicketID;
						m_tkt.Query(m_query);
						m_msg.m_TicketID = m_tkt.m_TicketID;
						m_msg.m_ReplyToMsgID = nReplyToMsgID;
						m_msg.m_ReplyToIDIsInbound = 0;
						m_bNewTicket = false;						
					}
				}
				else
				{
					m_query.Initialize();
					BINDPARAM_LONG( m_query, m_tkt.m_TicketBoxID );
					BINDPARAM_LONG( m_query, m_msg.m_ContactID );
					BINDPARAM_TCHAR( m_query, (void*) sSQLSubject.c_str() );
					BINDCOL_LONG_NOLEN( m_query, nTicketID );
					BINDCOL_TCHAR_NOLEN( m_query, szSubject );
					CEMSString cmd;
					if(nUseEsc)
						cmd.Format(_T("SELECT Tickets.TicketID,Subject FROM Tickets INNER JOIN TicketContacts ON Tickets.TicketID = TicketContacts.TicketID WHERE TicketBoxID=? AND ContactID=? AND TicketStateID <> 1 AND IsDeleted=0 AND Subject LIKE ? ESCAPE '%s' ORDER BY OpenTimestamp DESC"), cEscChar);
					else
						cmd.Format(_T("SELECT Tickets.TicketID,Subject FROM Tickets INNER JOIN TicketContacts ON Tickets.TicketID = TicketContacts.TicketID WHERE TicketBoxID=? AND ContactID=? AND TicketStateID <> 1 AND IsDeleted=0 AND Subject LIKE ? ORDER BY OpenTimestamp DESC"));

					m_query.Execute(cmd.c_str());

					dca::WString sTicketSubject;

					while ( m_query.Fetch() == S_OK )
					{
						// remove any prefixes from the ticket's subject
						sTicketSubject = szSubject;
						CleanupSubject(sTicketSubject);

						// compare the message's and ticket's subjects (without any prefixes)
						if ( _tcsicmp(sMsgSubject.c_str(), sTicketSubject.c_str()) == 0 )
						{
							// if the two match, use this ticket
							m_tkt.m_TicketID = nTicketID;
							m_tkt.Query(m_query);
							m_msg.m_TicketID = m_tkt.m_TicketID;
							m_bNewTicket = false;

							break;
						}
					}
				}
			}
			if ( !m_bNewTicket )
			{
				t.Format("CMsgRouter::PutInTicketBox - Found TicketID [%d] with matching Subject and Contact for InboundMessageID [%d]", m_msg.m_TicketID, m_msg.m_InboundMessageID);
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
			}
		}
		else
		{
			t.Format("CMsgRouter::PutInTicketBox - TicketID for InboundMessageID [%d] not found and Subject and Contact match not set", m_msg.m_InboundMessageID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
		}
	}

	if( m_pRule != NULL )
	{
		if(!m_bNewTicket && m_pRule->m_IgnoreTracking == 1 && m_pRule->m_AssignToTicketBoxAlg == 0)
		{
			//m_tkt.Query(m_query);
			int nTempTicketBoxID=0;
			m_query.Initialize();
			BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
			BINDCOL_LONG_NOLEN( m_query, nTempTicketBoxID );
			
			m_query.Execute( L"SELECT TicketBoxID FROM Tickets WHERE TicketID=?" );
				
			m_query.Fetch();

			if(m_pRule->m_AssignToTicketBox != nTempTicketBoxID)
			{
				//Was the Ticket moved from the default TicketBox?
				int nTempTicketID=0;
				m_query.Initialize();
				BINDPARAM_LONG( m_query, m_pRule->m_AssignToTicketBox );
				BINDPARAM_LONG( m_query, nTempTicketBoxID );
				BINDCOL_LONG_NOLEN( m_query, nTempTicketID );
				
				m_query.Execute( L"SELECT TicketID FROM TicketHistory WHERE TicketActionID=2 AND ID1=1 AND ID2=? and TicketBoxID=?" );
					
				if( m_query.Fetch() == S_OK )
				{
					//The Ticket was moved
					t.Format("CMsgRouter::PutInTicketBox - TicketID for InboundMessageID [%d] was moved from Default TicketBox", m_msg.m_InboundMessageID);
					DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);			
				}
				else
				{
					t.Format("CMsgRouter::PutInTicketBox - TicketID for InboundMessageID [%d] was not in Default TicketBox", m_msg.m_InboundMessageID);
					DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
					m_tkt.m_TicketBoxID = m_pRule->m_AssignToTicketBox;
					m_bNewTicket = true;
				}
			}
		}
	}
	
	// Create a new ticket if flag is set.
	if( m_bNewTicket )
	{
		t.Format("CMsgRouter::PutInTicketBox - TicketID for InboundMessageID [%d] not set, creating new Ticket", m_msg.m_InboundMessageID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		CreateNewTicket();
	}
	else
	{
		// Check if ticket is locked - if so, skip for now
		// Removed in version 6.5.0.0
		/*if( m_tkt.m_LockedBy != 0 )
		{
			Log( E_PutInTicketBox, L"New message received for TicketID %d, but ticket is locked by AgentID %d",
			     m_tkt.m_TicketID, m_tkt.m_LockedBy );

			t.Format("CMsgRouter::PutInTicketBox - TicketID [%d] set for InboundMessageID [%d] but Ticket locked by AgentID [%d]", m_tkt.m_TicketID, m_msg.m_InboundMessageID, m_tkt.m_LockedBy);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		
			m_LockedTicketMap.insert( pair<unsigned int,unsigned int>(m_msg.ID(), m_tkt.ID()) );

			m_MRState = MRState_RefreshRules;
			return;
		}*/

		AddContactToTicket( m_query, m_msg.m_ContactID, m_tkt.m_TicketID );
	}

	if( m_tkt.m_TicketStateID != EMS_TICKETSTATEID_OPEN && m_AutoReplyCloseTicketFlag == false )
	{
		set<long> EscalateAgentIDs;
		long oldOwnerID = m_tkt.m_OwnerID;
		SetTicketState( EMS_TICKETSTATEID_OPEN );
		ValidateOwnerID( EscalateAgentIDs );

		if( m_tkt.m_OwnerID != oldOwnerID )
		{
			m_query.Reset();
			BINDPARAM_LONG( m_query, m_tkt.m_OwnerID );
			BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
			m_query.Execute( L"UPDATE Tickets "
							 L"SET OwnerID=?, UseTickler = 0, TicklerDateToReopen = NULL "
							 L"WHERE TicketID=?");

			// Log it in TicketHistory
			THEscalate( m_query, m_tkt.m_TicketID, 0, oldOwnerID, m_tkt.m_TicketStateID, m_tkt.m_TicketStateID, m_tkt.m_TicketBoxID, m_tkt.m_OwnerID, m_tkt.m_PriorityID, m_tkt.m_TicketCategoryID );
		}
	}

	bool bCheckOwnerStatus = false;
	if(m_pRule)
	{
		if(!m_bNewTicket && m_pRule->m_SetOpenOwner)
		{
			bCheckOwnerStatus = true;
		}
	}
	else
	{
		if(!m_bNewTicket && _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_CHECK_OPEN_OWNER)))
		{
			bCheckOwnerStatus = true;
		}
	}

	if(bCheckOwnerStatus)
	{
		long oldOwnerID = m_tkt.m_OwnerID;
		if(oldOwnerID > 0)
		{
			CheckOwnerStatus();

			if( m_tkt.m_OwnerID != oldOwnerID )
			{
				m_query.Reset();
				BINDPARAM_LONG( m_query, m_tkt.m_OwnerID );
				BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
				m_query.Execute( L"UPDATE Tickets "
								L"SET OwnerID=?, UseTickler = 0, TicklerDateToReopen = NULL "
								L"WHERE TicketID=?");

				// Log it in TicketHistory
				THChangeOwner( m_query, m_tkt.m_TicketID, 0, oldOwnerID, m_tkt.m_TicketStateID, m_tkt.m_TicketBoxID, m_tkt.m_OwnerID, m_tkt.m_PriorityID, m_tkt.m_TicketCategoryID );			
			}
		}
	}
	
	m_msg.SetRouteInfo( m_tkt.m_TicketID, m_RoutingRuleID, m_msg.m_ContactID, 
		m_tkt.m_TicketBoxID, m_tkt.m_OwnerID, m_tkt.m_TicketCategoryID );

	t.Format("CMsgRouter::PutInTicketBox - Setting InboundMessageID [%d] to TicketID [%d], RoutingRuleID [%d], ContactID [%d], TicketBoxID [%d], OwnerID [%d], TicketCategoryID [%d]", m_msg.m_InboundMessageID, m_tkt.m_TicketID, m_RoutingRuleID, m_msg.m_ContactID, m_tkt.m_TicketBoxID, m_tkt.m_OwnerID, m_tkt.m_TicketCategoryID);
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		

	// If we created a new ticket, undelete it
	if( m_bNewTicket )
	{
		if( m_RoutingRuleID != 0 && 
			m_pRule->m_DeleteImmediatelyEnable &&
			m_pRule->m_DeleteImmediately == EMS_DELETE_OPTION_DELETE_TO_WASTE_BASKET )
		{
			t.Format("CMsgRouter::PutInTicketBox - New Ticket with TicketID [%d] created for InboundMessageID [%d], Routing Rule set to Delete Ticket", m_tkt.m_TicketID, m_msg.m_InboundMessageID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
		
			// We'll leave the ticket as deleted, so set the deleted time
			TIMESTAMP_STRUCT now;
			GetTimeStamp( now );

			m_query.Reset();
			BINDPARAM_TIME_NOLEN( m_query, now );
			BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
			m_query.Execute( L"UPDATE Tickets "
							 L"SET TicketStateID=5,DeletedTime=?,DeletedBy=0 "
							 L"WHERE TicketID=?");

			m_tkt.m_TicketStateID=5;

			THDeleteTicket( m_query, m_tkt.m_TicketID, 0, m_tkt.m_TicketStateID, m_tkt.m_TicketBoxID, m_tkt.m_OwnerID, m_tkt.m_PriorityID, m_tkt.m_TicketCategoryID );

		}
		else
		{
			t.Format("CMsgRouter::PutInTicketBox - New Ticket with TicketID [%d] created for InboundMessageID [%d], Clearing Ticket Deleted flag", m_tkt.m_TicketID, m_msg.m_InboundMessageID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
		
			UnDeleteTicket();			
		}

		THAddInboundMsg( m_query, m_tkt.m_TicketID, 0, m_msg.m_InboundMessageID, m_tkt.m_TicketBoxID );

		//If the Ticket Category was set, log it to Ticket History
		if ( m_tkt.m_TicketCategoryID > 1 )
		{
			THChangeCategory( m_query, m_tkt.m_TicketID, 0, 1, m_tkt.m_TicketStateID, m_tkt.m_TicketBoxID, m_tkt.m_OwnerID, m_tkt.m_PriorityID, m_tkt.m_TicketCategoryID );
		}		
	}
	else
	{
		THAddInboundMsg( m_query, m_tkt.m_TicketID, 0, m_msg.m_InboundMessageID, m_tkt.m_TicketBoxID );
	}


	// Determine the ticket box name if not set already
	if( m_szTicketBoxName[0] == L'\0' )
	{
		if( m_RoutingRuleID != 0 )
		{
			if( m_pRule->m_TicketboxNameLen == SQL_NULL_DATA )
			{
				wcscpy( m_szTicketBoxName, _T("Unknown") );
			}
			else
			{
				wcscpy( m_szTicketBoxName, m_pRule->m_TicketboxName );
			}
		}
		else
		{
			wcscpy( m_szTicketBoxName, _T("Unassigned") );
		}
	}

	Log( E_PutInTicketBox, EMS_STRING_RTR_PUT_IN_TICKETBOX, m_szTicketBoxName );

	dca::String x(m_szTicketBoxName);
	t.Format("CMsgRouter::PutInTicketBox - InboundMessageID [%d] routed to TicketBox [%s]", m_msg.m_InboundMessageID, x.c_str());
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	
	m_MRState = MRState_CreateAutoReplies;
}

void CMsgRouter::CheckAgentAlerts( int nOwnerID, bool bIsContactOwner )
{
	std::list<TAlerts> aeidList;
	std::list<TAlerts>::iterator aeidIter;
	dca::String t;
	TAlerts alert;
	CEMSString strAlertBody;
	tstring strSubject;
	strSubject.assign( m_msg.m_Subject );
	RemoveTicketIDFromSubject( strSubject );
		
	m_query.Initialize();
	BINDPARAM_LONG( m_query, nOwnerID );
	BINDCOL_LONG( m_query, alert.m_AlertID );
	BINDCOL_LONG( m_query, alert.m_AlertEventID );
	BINDCOL_LONG( m_query, alert.m_AlertMethodID );
	BINDCOL_TCHAR( m_query, alert.m_EmailAddress );
	m_query.Execute( L"SELECT AlertID,AlertEventID,AlertMethodID,EmailAddress FROM Alerts WHERE AgentID=? AND AlertEventID IN (12,13,14)");
	while(m_query.Fetch() == S_OK)
	{
		dca::String t2(alert.m_EmailAddress);
		t.Format("CMsgRouter::CheckAgentAlerts - Agent [%d] configured for AlertEventID [%d] AlertMethodID [%d] EmailAddress [%s]", nOwnerID, alert.m_AlertEventID,alert.m_AlertMethodID,t2.c_str() );
		//DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		alert.m_AgentID=nOwnerID;
		aeidList.push_back(alert);
	}		

	if( m_bNewTicket )
	{
		for(aeidIter = aeidList.begin(); aeidIter != aeidList.end(); aeidIter++)
		{
			if(!bIsContactOwner)
			{
                if(aeidIter->m_AlertEventID == 14)
				{
					//DebugReporter::Instance().DisplayMessage("CMsgRouter::CheckAgentAlerts - Agent Alert new Ticket assigned", DebugReporter::ENGINE);
					if( m_RoutingRuleID != 0 )
					{
						strAlertBody.Format( _T("Ticket %d assigned to you by Routing Rule %s|||A Ticket has been assigned to you by Routing Rule %s.\n\nTicket #%d %s\r\n"),m_tkt.m_TicketID, m_pRule->m_RuleDescrip, m_pRule->m_RuleDescrip, m_tkt.m_TicketID, strSubject.c_str() );						
					}
					else
					{
						strAlertBody.Format( _T("Ticket %d assigned to you by the Default Routing Rule|||A Ticket has been assigned to you by the Default Routing Rule.\n\nTicket #%d %s\r\n"),m_msg.m_TicketID, m_tkt.m_TicketID, strSubject.c_str() );						
					}
					CustomAlert(EMS_ALERT_EVENT_TICKET_ASSIGNED,aeidIter->m_AlertID,_T("%s"), strAlertBody.c_str() );
				}				
			}
			else
			{
				if(aeidIter->m_AlertEventID == 13)
				{
					//DebugReporter::Instance().DisplayMessage("CMsgRouter::CheckAgentAlerts - Agent Alert new Ticket from contact", DebugReporter::ENGINE);
					GetContactName();			
					strAlertBody.Format( _T("A new message from Contact <%s> has been added to Ticket %d|||A new message from a Contact you own <%s> has been added to a Ticket.\n\nTicket #%d %s\r\n"),m_szContactEmail,m_tkt.m_TicketID,m_szContactEmail,m_tkt.m_TicketID,strSubject.c_str() );
					CustomAlert(EMS_ALERT_EVENT_INBOUND_OWNED_CONTACT,aeidIter->m_AlertID,_T("%s"), strAlertBody.c_str() );
				}
			}				
		}
	}
	else
	{
		for(aeidIter = aeidList.begin(); aeidIter != aeidList.end(); aeidIter++)
		{
			if(!bIsContactOwner)
			{
                if(aeidIter->m_AlertEventID == 12)
				{
					//DebugReporter::Instance().DisplayMessage("CMsgRouter::CheckAgentAlerts - Agent Alert new message to ticket", DebugReporter::ENGINE);
					strAlertBody.Format( _T("A new message has been added to Ticket %d|||A new message has been added to a Ticket you own.\n\nTicket #%d %s\r\n"), m_tkt.m_TicketID, m_tkt.m_TicketID, strSubject.c_str() );
					CustomAlert(EMS_ALERT_EVENT_INBOUND_OWNED_TICKET,aeidIter->m_AlertID,_T("%s"), strAlertBody.c_str() );
				}
			}
			else
			{
				if(aeidIter->m_AlertEventID == 13)
				{
					//DebugReporter::Instance().DisplayMessage("CMsgRouter::CheckAgentAlerts - Agent Alert new message from contact", DebugReporter::ENGINE);
					GetContactName();			
					strAlertBody.Format( _T("A new message from Contact <%s> has been added to Ticket %d|||A new message from a Contact you own <%s> has been added to a Ticket.\n\nTicket #%d %s\r\n"),m_szContactEmail,m_tkt.m_TicketID,m_szContactEmail,m_tkt.m_TicketID,strSubject.c_str() );
					CustomAlert(EMS_ALERT_EVENT_INBOUND_OWNED_CONTACT,aeidIter->m_AlertID,_T("%s"), strAlertBody.c_str() );
				}
			}
		}
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// ForwardCopy
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::ForwardCopy( void )
{
	DebugReporter::Instance().DisplayMessage("CMsgRouter::ForwardCopy - Entering", DebugReporter::ENGINE);
	if( m_RoutingRuleID != 0 )
	{
		if( m_pRule->m_ForwardEnable != 0 && m_pRule->m_fta.size() > 0 )
		{
			dca::String t;
			t.Format("CMsgRouter::ForwardCopy - Forwarding copy of InboundMessageID [%d] in TicketID [%d]", m_msg.m_InboundMessageID, m_tkt.m_TicketID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

			bool bIsRemoteReply = false;
			int nOriginalMsgID = 0;
			int nOmitTracking = 0;

			if( m_pRule->m_AllowRemoteReply!=0 )
			{
				if( m_msg.m_ReplyToMsgID > 0 )
				{
					//Is the nReplyToMsgID an original forwarded message?
					m_query.Initialize();
					BINDPARAM_LONG( m_query, m_msg.m_ReplyToMsgID );
					BINDCOL_LONG_NOLEN( m_query, nOriginalMsgID );					
					m_query.Execute( L"SELECT ReplyToMsgID FROM OutboundMessages "
									 L"WHERE OutboundMessageID=? AND ReplyToIDIsInbound=1" );
					
					if( m_query.Fetch() == S_OK )
					{
						if( nOriginalMsgID > 0 )
						{
							//Ok we have the original InboundMessageID, is the from address in this message in the forward to list?
							int nAddressID;
							m_query.Reset();
							BINDPARAM_TCHAR( m_query, m_msg.m_EmailFrom );
							BINDPARAM_LONG( m_query, m_pRule->m_RoutingRuleID );
							BINDCOL_LONG_NOLEN( m_query, nAddressID );					
							m_query.Execute( L"SELECT AddressID FROM ForwardToAddresses "
											L"WHERE EmailAddress=? AND RoutingRuleID=?" );

							if( m_query.Fetch() == S_OK )
							{
								if( nAddressID > 0 )
								{
									//Ok, from address is in the list
									bIsRemoteReply = true;									
								}
								else
								{
									//From address not in the ForwardTo list, check ForwardCC
									m_query.Reset();
									BINDPARAM_TCHAR( m_query, m_msg.m_EmailFrom );
									BINDPARAM_LONG( m_query, m_pRule->m_RoutingRuleID );
									BINDCOL_LONG_NOLEN( m_query, nAddressID );					
									m_query.Execute( L"SELECT AddressID FROM ForwardCCAddresses "
													L"WHERE EmailAddress=? AND RoutingRuleID=?" );

									if( m_query.Fetch() == S_OK )
									{
										if( nAddressID > 0 )
										{
											//Ok, from address is in the list
											bIsRemoteReply = true;
											t.Format("CMsgRouter::ForwardCopy - InboundMessageID [%d] in TicketID [%d] is a Remote Reply", m_msg.m_InboundMessageID, m_tkt.m_TicketID);
											DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
										}
									}
								}
							}
						}
					}
				}
			}
			
			//Check to see if any of the recipients or recipient domains are in the Ticket Tracking Bypass
			bool bDoTrack = true;
			map<int,TBypass>::iterator iter;
			if(m_Bypass.size() > 0)
			{
				//Check To: email addresses
				t.Format("CMsgRouter::ForwardCopy - Checking to see if recipients or domain are in the Tracking Bypass");
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
				tstring sValue;
				int nType = 0;				
				bool bFound = false;
				

				if(m_pRule->m_fta.size() > 0)
				{
					for ( iter = m_Bypass.begin(); iter != m_Bypass.end(); iter++ )
					{
						sValue.assign(iter->second.m_BypassValue);
						nType = iter->second.m_BypassTypeID;
						
						dca::String x(sValue.c_str());
						t.Format("CMsgRouter::ForwardCopy - Checking Bypass Value [%s] TypeID [%d] against recipients", x.c_str(), nType);
						DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
						
						vector<CForwardToAddr>::iterator additer;										
						for ( additer = m_pRule->m_fta.begin(); additer != m_pRule->m_fta.end(); additer++ )
						{
							tstring sAdd;
							sAdd.assign(additer->m_EmailAddress);							

							if(nType == 0)
							{
								dca::String y(sAdd.c_str());
								t.Format("CMsgRouter::ForwardCopy - Checking To: Recipient [%s]", y.c_str());
								DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

								if( _tcsicmp( sValue.c_str(), sAdd.c_str() ) == 0 )
								{
									bFound = true;

									dca::String z(sAdd.c_str());
									t.Format("CMsgRouter::ForwardCopy - To: Recipient [%s] is in the Bypass", z.c_str());
									DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

									break;
								}									 
							}
							else
							{
								tstring sDom;
								sDom.assign(sAdd.substr(sAdd.find_last_of(_T("@"))+1, sAdd.length() - sAdd.find_last_of(_T("@"))+1));

								t.Format("CMsgRouter::ForwardCopy - Checking To: Domain [%s]", sDom.c_str());
								DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

								if( _tcsicmp( sValue.c_str(), sDom.c_str() ) == 0 )
								{
									bFound = true;

									t.Format("CMsgRouter::ForwardCopy - To: Domain [%s] is in the Bypass", sDom.c_str());
									DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

									break;
								}
							}
						}
						if(bFound)
						{
							bDoTrack = false;
							break;
						}							
					}
				}

				if(bDoTrack)
				{
					//Check Cc: email addresses
					tstring sValue;
					int nType = 0;				
					bool bFound = false;
					if(m_pRule->m_fca.size() > 0)
					{
						for ( iter = m_Bypass.begin(); iter != m_Bypass.end(); iter++ )
						{
							sValue.assign(iter->second.m_BypassValue);
							nType = iter->second.m_BypassTypeID;
							vector<CForwardCCAddr>::iterator additer;
							for ( additer = m_pRule->m_fca.begin(); additer != m_pRule->m_fca.end(); additer++ )
							{
								tstring sAdd;
								sAdd.assign(additer->m_EmailAddress);
								if(nType == 0)
								{
									t.Format("CMsgRouter::ForwardCopy - Checking Cc: Recipient [%s]", sAdd.c_str());
									DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

									if( _tcsicmp( sValue.c_str(), sAdd.c_str() ) == 0 )
									{
										bFound = true;

										t.Format("CMsgRouter::ForwardCopy - Cc: Recipient [%s] is in the Bypass", sAdd.c_str());
										DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

										break;
									}									 
								}
								else
								{
									tstring sDom;
									sDom.assign(sAdd.substr(sAdd.find_last_of(_T("@"))+1, sAdd.length() - sAdd.find_last_of(_T("@"))+1));
									
									t.Format("CMsgRouter::ForwardCopy - Checking Cc: Domain [%s]", sDom.c_str());
									DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

									if( _tcsicmp( sValue.c_str(), sDom.c_str() ) == 0 )
									{
										bFound = true;

										t.Format("CMsgRouter::ForwardCopy - Cc: Domain [%s] is in the Bypass", sDom.c_str());
										DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

										break;
									}
								}
							}
							if(bFound)
							{
								bDoTrack = false;
								break;
							}							
						}
					}
				}
			}
					
			if(!bDoTrack)
			{
				nOmitTracking = 1;
			}

			if( bIsRemoteReply && nOriginalMsgID > 0 )
			{
				COutboundMsg forwardmsg( m_query );				

				forwardmsg.m_OriginalTicketBoxID = m_tkt.m_TicketBoxID;
				forwardmsg.m_TicketCategoryID = m_tkt.m_TicketCategoryID;
				
				CInboundMsg	origMsg( m_query );

				// this is a remote reply, we need to turn this into an outbound message back to the original sender
				// nOriginalMsgID should be the original inbound message
				origMsg.m_InboundMessageID = nOriginalMsgID;
				origMsg.Query( m_query );

				forwardmsg.m_AgentID = 0;
				forwardmsg.m_ReplyToMsgID = nOriginalMsgID;

				// set the from address

				if( m_pRule->m_ForwardFromContact )
				{
					// forward from sender
					_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
								L"%s <%s>", m_msg.m_EmailFromName, m_msg.m_EmailFrom );
				}
				else if ( m_pRule->m_ForwardFromAgent == -1 )
				{
					if( _tcslen(m_pRule->m_ForwardFromName)< 1 )
					{
						// forward from agent, but no default address defined (use name instead)
						_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
								L"%s <%s>", m_pRule->m_ForwardFromEmail, m_pRule->m_ForwardFromEmail);
					}
					else
					{
						// forward from agent default address
						_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
								L"%s <%s>", m_pRule->m_ForwardFromName,  m_pRule->m_ForwardFromEmail );
					}
				}
				else
				{
					if( m_pRule->m_AgentAddressLen == SQL_NULL_DATA )
					{
						// forward from agent, but no default address defined (use name instead)
						_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
								L"%s <no default email address>", m_pRule->m_AgentName );
					}
					else
					{
						// forward from agent default address
						_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
								L"%s <%s>", m_pRule->m_AgentName,  m_pRule->m_AgentAddress );
					}
				}

				// is the from address of this message associated with an Agent?
				int nAgentID = 0;
				m_query.Initialize();
				BINDPARAM_TCHAR( m_query, m_msg.m_EmailFrom );
				BINDCOL_LONG_NOLEN( m_query, nAgentID );					
				m_query.Execute( L"SELECT TOP 1 PersonalData.AgentID FROM PersonalData "
								 L"INNER JOIN Agents ON PersonalData.AgentID=Agents.AgentID "
								 L"WHERE PersonalData.DataValue=? AND Agents.IsEnabled = 1" );								

				if( m_query.Fetch() == S_OK )
				{
					if( nAgentID > 0 )
					{
						forwardmsg.m_AgentID = nAgentID;
					}					
				}

				// set the to
				m_pRule->m_fta.clear();
				m_pRule->m_fca.clear();
				CForwardToAddr fta;
				wcscpy( fta.m_EmailAddress, origMsg.m_EmailFrom );
				m_pRule->m_fta.push_back( fta );

				// Put the RE: in front of the subject, if there isn't one already
				if( wcsnicmp( origMsg.m_Subject, L"RE:", 3) == 0 )
				{
					wcscpy( forwardmsg.m_Subject, origMsg.m_Subject );
				}
				else
				{
					if( _snwprintf( forwardmsg.m_Subject, OUTBOUNDMESSAGES_SUBJECT_LENGTH, 
									L"RE: %s", origMsg.m_Subject ) < 0 )
					{
						// We overran the buffer
						origMsg.m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-1] = _T('\0');
						origMsg.m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-2] = _T('.');
						origMsg.m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-3] = _T('.');
						origMsg.m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-4] = _T('.');
					}					
				}

				forwardmsg.Forward( m_msg, m_pRule->m_fta, m_pRule->m_fca, true, true, false, (m_pRule->m_QuoteOriginal!=0), nOmitTracking );
				THAddOutboundMsg( m_query, m_tkt.m_TicketID, forwardmsg.m_AgentID, forwardmsg.m_OutboundMessageID, m_tkt.m_TicketBoxID );

				if ( m_pRule->m_ForwardInTicket == 0 )
				{
					TIMESTAMP_STRUCT now;
					GetTimeStamp( now );

					m_msg.m_TicketID = 0;
					m_msg.m_IsDeleted = 1;
					m_query.Initialize();

					BINDPARAM_LONG( m_query, m_msg.m_TicketID );
					BINDPARAM_TINYINT( m_query, m_msg.m_IsDeleted );
					BINDPARAM_TIME_NOLEN( m_query, now );
					BINDPARAM_LONG( m_query, m_msg.m_InboundMessageID );
					m_query.Execute( L"UPDATE InboundMessages SET TicketID=?,IsDeleted=?,DeletedTime=? WHERE InboundMessageID=?");

					THDelInboundMsg( m_query, m_tkt.m_TicketID, 0, m_msg.m_InboundMessageID, m_tkt.m_TicketBoxID );
				}
				RebuildTicketContacts( m_query, m_tkt.m_TicketID );
			}
			else
			{
				if(m_pRule->m_MultiMail == 1 && m_pRule->m_fta.size() > 1 )
				{
					vector<CForwardToAddr>::iterator ftaIter;
					ftaIter = m_pRule->m_fta.begin();					
					while ( ftaIter != m_pRule->m_fta.end() )
					{
						vector<CForwardToAddr> m_fta2;					
						m_fta2.push_back( *ftaIter );
						
						COutboundMsg forwardmsg( m_query );
						forwardmsg.m_OriginalTicketBoxID = m_tkt.m_TicketBoxID;
						forwardmsg.m_TicketCategoryID = m_tkt.m_TicketCategoryID;					
						
						// Set the from address
						if( m_pRule->m_ForwardFromContact )
						{
							// forward from sender
							_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
										L"%s <%s>", m_msg.m_EmailFromName, m_msg.m_EmailFrom );
						}
						else if ( m_pRule->m_ForwardFromAgent == -1 )
						{
							if( _tcslen(m_pRule->m_ForwardFromName)< 1 )
							{
								// forward from agent, but no default address defined (use name instead)
								_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
										L"%s <%s>", m_pRule->m_ForwardFromEmail, m_pRule->m_ForwardFromEmail);
							}
							else
							{
								// forward from agent default address
								_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
										L"%s <%s>", m_pRule->m_ForwardFromName,  m_pRule->m_ForwardFromEmail );
							}
						}
						else
						{
							if( m_pRule->m_AgentAddressLen == SQL_NULL_DATA )
							{
								// forward from agent, but no default address defined (use name instead)
								_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
										L"%s <no default email address>", m_pRule->m_AgentName );
							}
							else
							{
								// forward from agent default address
								_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
										L"%s <%s>", m_pRule->m_AgentName,  m_pRule->m_AgentAddress );
							}
						}					
						forwardmsg.Forward( m_msg, m_fta2, m_pRule->m_fca, (m_pRule->m_ForwardInTicket!=0), false, (m_pRule->m_AllowRemoteReply!=0), (m_pRule->m_QuoteOriginal!=0), nOmitTracking );
						THAddOutboundMsg( m_query, m_tkt.m_TicketID, forwardmsg.m_AgentID, forwardmsg.m_OutboundMessageID, forwardmsg.m_OriginalTicketBoxID );
						ftaIter++;
					}
				}
				else
				{
					COutboundMsg forwardmsg( m_query );				

					forwardmsg.m_OriginalTicketBoxID = m_tkt.m_TicketBoxID;
					forwardmsg.m_TicketCategoryID = m_tkt.m_TicketCategoryID;					
					
					// Set the from address
					if( m_pRule->m_ForwardFromContact )
					{
						// forward from sender
						_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
									L"%s <%s>", m_msg.m_EmailFromName, m_msg.m_EmailFrom );
					}
					else if ( m_pRule->m_ForwardFromAgent == -1 )
					{
						if( _tcslen(m_pRule->m_ForwardFromName)< 1 )
						{
							// forward from agent, but no default address defined (use name instead)
							_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
									L"%s <%s>", m_pRule->m_ForwardFromEmail, m_pRule->m_ForwardFromEmail);
						}
						else
						{
							// forward from agent default address
							_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
									L"%s <%s>", m_pRule->m_ForwardFromName,  m_pRule->m_ForwardFromEmail );
						}
					}
					else
					{
						if( m_pRule->m_AgentAddressLen == SQL_NULL_DATA )
						{
							// forward from agent, but no default address defined (use name instead)
							_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
									L"%s <no default email address>", m_pRule->m_AgentName );
						}
						else
						{
							// forward from agent default address
							_snwprintf( forwardmsg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH,
									L"%s <%s>", m_pRule->m_AgentName,  m_pRule->m_AgentAddress );
						}
					}					
					forwardmsg.Forward( m_msg, m_pRule->m_fta, m_pRule->m_fca, (m_pRule->m_ForwardInTicket!=0), false, (m_pRule->m_AllowRemoteReply!=0), (m_pRule->m_QuoteOriginal!=0), nOmitTracking );
					THAddOutboundMsg( m_query, m_tkt.m_TicketID, forwardmsg.m_AgentID, forwardmsg.m_OutboundMessageID, m_tkt.m_TicketBoxID );					
				}
				
				
			}
		}
	}

	DebugReporter::Instance().DisplayMessage("CMsgRouter::ForwardCopy - Leaving", DebugReporter::ENGINE);
	
	m_MRState = MRState_AutoReply;
}

void CMsgRouter::CreateAutoReplies()
{
	if( m_bNewTicket )
	{
		dca::String t;
		t.Format("CMsgRouter::CreateAutoReplies - Processing Auto Replies for InboundMessageID [%d] in new TicketID [%d]", m_msg.m_InboundMessageID, m_tkt.m_TicketID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

		SRIDWITHKEYWORD sridWKD;

		m_query.Initialize();
		m_query.Execute(_T("SELECT StandardResponseID, Keywords FROM StandardResponses WHERE (UseKeywords = 1)"));
		BINDCOL_LONG(m_query, sridWKD._lStandardResponseID);
		BINDCOL_TCHAR(m_query, sridWKD._sKeywords);

		std::list<SRIDWITHKEYWORD> sridList;

		while(m_query.Fetch() == S_OK)
		{
			sridList.push_back(sridWKD);
		}

		if ( sridList.size() > 0 )
		{
			t.Format("CMsgRouter::CreateAutoReplies - Processing Standard Response Keywords for InboundMessageID [%d]", m_msg.m_InboundMessageID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		}
		std::list<SRIDWITHKEYWORD>::iterator sridIter;
		CTextMatcher tm;

		for(sridIter = sridList.begin();
			sridIter != sridList.end();
			sridIter++)
		{
			dca::WString sKeywords((*sridIter)._sKeywords);

			dca::WString::size_type pos = dca::WString::npos;

			bool bFoundOne = false;
			int nCount = 0;
			int nCountFound = 0;

			do
			{
				pos = sKeywords.find(';');

				if(pos != dca::WString::npos)
				{
					dca::WString sKeyword = sKeywords.substr(0,pos);
					sKeywords = sKeywords.erase(0, pos + 1);

					if(tm.FindString(sKeyword.c_str(), m_msg.m_Body))
					{
						bFoundOne = true;
						nCountFound++;
					}

					nCount++;
				}
				else
				{
					if(sKeywords.size())
					{
						if(tm.FindString(sKeywords.c_str(), m_msg.m_Body))
						{
							bFoundOne = true;
							nCountFound++;
						}

						nCount++;
					}
				}
			
			}while(pos != dca::WString::npos);

			if(bFoundOne)
			{
				float p = ((float)nCountFound / (float)nCount) * 100;

				m_query.Initialize();
				dca::WString sCmd;
				sCmd.Format(_T("INSERT INTO SRKeywordResults (InboundMessageID, StandardResponseID, Score) VALUES (%d, %d, %d)"),m_msg.m_InboundMessageID,(*sridIter)._lStandardResponseID,(int)p);
				m_query.Execute(sCmd.c_str());

				t.Format("CMsgRouter::CreateAutoReplies - Found Standard Response Keywords for InboundMessageID [%d] StandardResponseID [%d] Percent Match [%d]", m_msg.m_InboundMessageID,(*sridIter)._lStandardResponseID,(int)p);
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
			}
		}
	}

	m_MRState = MRState_Forward;
}

////////////////////////////////////////////////////////////////////////////////
// 
// AutoReply
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::AutoReply( void )
{	
	if( m_RoutingRuleID != 0 )
	{
		if( m_pRule->m_AutoReplyEnable != 0 && m_bNewTicket )
		{
			COutboundMsg replymsg( m_query );

			replymsg.m_OriginalTicketBoxID = m_tkt.m_TicketBoxID;
			replymsg.m_TicketCategoryID = m_pRule->m_AssignToTicketCategory;
			
			// auto-reply to this message
			if( replymsg.AutoReply( m_pRule->m_AutoReplyWithStdResponse,
					                m_pRule->m_AutoReplyQuoteMsg, m_msg,
									m_pRule->m_AutoReplyFrom, (m_pRule->m_AutoReplyInTicket!=0) ) == true )
			{	
				// Make sure this ticket isn't auto-replied to again.
				m_query.Reset(true);
				BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
				m_query.Execute( L"UPDATE Tickets "
								 L"SET AutoReplied=1 "
								 L"WHERE TicketID=?" );

				THAddOutboundMsg( m_query, m_tkt.m_TicketID, 0, replymsg.m_OutboundMessageID, m_tkt.m_TicketBoxID );
			}
		}
	}

	m_MRState = MRState_RemoveFromQueue;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetTicketInfo - If the message subject contained a ticketID, then this
//                 function verifies the validitity of assigning this message to
//                 the ticket.
// 
////////////////////////////////////////////////////////////////////////////////
bool CMsgRouter::GetTicketInfo( int nTicketID, int nMsgID )
{

	// validate the outboundmessage is associated with the ticket
	m_query.Initialize();
    BINDPARAM_LONG( m_query, nTicketID );
	BINDPARAM_LONG( m_query, nMsgID );
	BINDCOL_LONG( m_query, m_tkt.m_TicketStateID );
	BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
	BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
	BINDCOL_LONG( m_query, m_tkt.m_OwnerID );
	BINDCOL_LONG( m_query, m_tkt.m_TicketCategoryID );
	BINDCOL_LONG( m_query, m_tkt.m_LockedBy );

	m_query.Execute( L"SELECT Tickets.TicketStateID,Tickets.TicketBoxID,TicketBoxes.Name,Tickets.OwnerID,Tickets.TicketCategoryID,LockedBy "
					 L"FROM Tickets "
					 L"INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
					 L"INNER JOIN OutboundMessages ON Tickets.TicketID = OutboundMessages.TicketID "
					 L"WHERE Tickets.TicketID=? "
					 L"AND OutboundMessages.OutboundMessageID=? AND Tickets.IsDeleted=0 " );
	
	if( m_query.Fetch() == S_OK )
	{
		// we found a ticket that matches!
		m_tkt.m_TicketID = nTicketID;
		m_msg.m_ReplyToMsgID = nMsgID;
	}
	else
	{
		int nTempTicketID;
		long nTempTicketIDLen;
		
		// check to see if the Outbound Message referenced in the Subject has been moved to another Ticket
		m_query.Initialize();
		BINDPARAM_LONG( m_query, nMsgID );
		BINDPARAM_LONG( m_query, nTicketID );
		BINDCOL_LONG( m_query, nTempTicketID );
		BINDCOL_LONG( m_query, m_tkt.m_TicketStateID );
		BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
		BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
		BINDCOL_LONG( m_query, m_tkt.m_OwnerID );
		BINDCOL_LONG( m_query, m_tkt.m_TicketCategoryID );
		BINDCOL_LONG( m_query, m_tkt.m_LockedBy );

		m_query.Execute( L"SELECT DISTINCT Tickets.TicketID,Tickets.TicketStateID,Tickets.TicketBoxID,TicketBoxes.Name,Tickets.OwnerID,Tickets.TicketCategoryID,LockedBy "
						L"FROM Tickets "
						L"INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
						L"INNER JOIN OutboundMessages ON Tickets.TicketID = OutboundMessages.TicketID "
						L"INNER JOIN TicketHistory ON TicketHistory.ID2 = OutboundMessages.OutboundMessageID "
						L"WHERE OutboundMessages.OutboundMessageID=? AND Tickets.IsDeleted=0 AND TicketHistory.TicketActionID = 6 AND TicketHistory.TicketID=? " );
			
		if( m_query.Fetch() == S_OK )
		{
			m_tkt.m_TicketID = nTempTicketID;
			m_msg.m_ReplyToMsgID = nMsgID;
		}
		else
		{
			// check to see if the Outbound Message referenced in the Subject is a forwarded message with TicketID=0
			m_query.Initialize();
			BINDPARAM_LONG( m_query, nMsgID );
			BINDCOL_LONG( m_query, nTempTicketID );
			m_query.Execute( L"SELECT TicketID "
							L"FROM OutboundMessages "
							L"WHERE OutboundMessageID=? AND TicketID=0 AND AgentID=0 AND OutboundMessageTypeID=2 " );
				
			if( m_query.Fetch() == S_OK )
			{
				//get the Ticket info
				m_tkt.m_TicketID = nTicketID;
				m_msg.m_ReplyToMsgID = nMsgID;
				m_query.Initialize();
				BINDPARAM_LONG( m_query, nTicketID );			
				BINDCOL_LONG( m_query, m_tkt.m_TicketStateID );
				BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
				BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
				BINDCOL_LONG( m_query, m_tkt.m_OwnerID );
				BINDCOL_LONG( m_query, m_tkt.m_TicketCategoryID );
				BINDCOL_LONG( m_query, m_tkt.m_LockedBy );

				m_query.Execute( L"SELECT Tickets.TicketStateID,Tickets.TicketBoxID,TicketBoxes.Name,Tickets.OwnerID,Tickets.TicketCategoryID,LockedBy "
								L"FROM Tickets "
								L"INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
								L"WHERE Tickets.TicketID=? AND Tickets.IsDeleted=0 " );
					
				if( m_query.Fetch() != S_OK )
				{
					return false;	
				}								
			}
			else
			{
				// check to see if the Ticket was merged with another Ticket
				m_query.Initialize();
				BINDPARAM_LONG( m_query, nTicketID );
				BINDCOL_LONG( m_query, nTempTicketID );
				m_query.Execute( L"SELECT TicketID FROM TicketHistory WHERE TicketActionID=18 AND ID1=?" );
					
				if( m_query.Fetch() == S_OK )
				{
					m_query.Initialize();
					BINDPARAM_LONG( m_query, nTempTicketID );
					BINDPARAM_LONG( m_query, nMsgID );
					BINDCOL_LONG( m_query, m_tkt.m_TicketStateID );
					BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
					BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
					BINDCOL_LONG( m_query, m_tkt.m_OwnerID );
					BINDCOL_LONG( m_query, m_tkt.m_TicketCategoryID );
					BINDCOL_LONG( m_query, m_tkt.m_LockedBy );

					m_query.Execute( L"SELECT Tickets.TicketStateID,Tickets.TicketBoxID,TicketBoxes.Name,Tickets.OwnerID,Tickets.TicketCategoryID,LockedBy "
									L"FROM Tickets "
									L"INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
									L"INNER JOIN OutboundMessages ON Tickets.TicketID = OutboundMessages.TicketID "
									L"WHERE Tickets.TicketID=? "
									L"AND OutboundMessages.OutboundMessageID=? AND Tickets.IsDeleted=0 " );
					
					if( m_query.Fetch() == S_OK )
					{
						m_tkt.m_TicketID = nTempTicketID;
						m_msg.m_ReplyToMsgID = nMsgID;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetTicketInfo - If the message subject contained a ticketID, then this
//                 function verifies the validitity of assigning this message to
//                 the ticket.
// 
////////////////////////////////////////////////////////////////////////////////
bool CMsgRouter::GetPreRoutedTicketInfo( void )
{
	// validate the contact and outboundmessage are associated with ticket
	m_query.Initialize();
    BINDPARAM_LONG( m_query, m_msg.m_TicketID );
  	BINDCOL_LONG( m_query, m_tkt.m_TicketStateID );
	BINDCOL_LONG( m_query, m_tkt.m_TicketBoxID );
	BINDCOL_WCHAR_NOLEN( m_query, m_szTicketBoxName );
	BINDCOL_LONG( m_query, m_tkt.m_OwnerID );
	BINDCOL_LONG( m_query, m_tkt.m_TicketCategoryID );
	BINDCOL_LONG( m_query, m_tkt.m_LockedBy );

	m_query.Execute( L"SELECT Tickets.TicketStateID,Tickets.TicketBoxID,TicketBoxes.Name,Tickets.OwnerID,Tickets.TicketCategoryID,LockedBy "
					 L"FROM Tickets "
					 L"INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
					 L"WHERE Tickets.TicketID=? AND Tickets.IsDeleted=0 " );
	
	if( m_query.Fetch() == S_OK )
	{
		return true;
	}
	else
	{
		// no such luck!
		return false;
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// AssignToAgentWithMatchingAddress
// 
////////////////////////////////////////////////////////////////////////////////
bool CMsgRouter::AssignToAgentWithMatchingAddress(void)
{
	if( wcslen( m_msg.m_EmailPrimaryTo ) > 0 )
	{
		// does an agent have a matching email address?
		m_query.Reset( true );
		BINDPARAM_WCHAR( m_query, m_msg.m_EmailPrimaryTo );
		BINDCOL_LONG( m_query, m_tkt.m_OwnerID );
		m_query.Execute( L"SELECT PersonalData.AgentID FROM PersonalData "
						 L"INNER JOIN Agents ON PersonalData.AgentID = Agents.AgentID "
						 L"WHERE PersonalData.PersonalDataTypeID=1 "
						 L"AND IsDeleted=0 "
						 L"AND PersonalData.DataValue=?" );

		if( m_query.Fetch() == S_OK )
			return true;
	}
	else
	{
		list<EmailAddr_t>::iterator iter;
		list<EmailAddr_t> emaillist;
		ProcessEmailAddrString( m_msg.m_EmailTo, emaillist, false );

		for( iter = emaillist.begin(); iter != emaillist.end(); iter++ )
		{
			// does an agent have a matching email address?
			m_query.Reset( true );
			BINDPARAM_WCHAR( m_query, (wchar_t*)iter->m_sEmailAddr.c_str() );
			BINDCOL_LONG( m_query, m_tkt.m_OwnerID );
			m_query.Execute( L"SELECT PersonalData.AgentID FROM PersonalData "
							 L"INNER JOIN Agents ON PersonalData.AgentID = Agents.AgentID "
							 L"WHERE PersonalData.PersonalDataTypeID=1 "
							 L"AND IsDeleted=0 "
							 L"AND PersonalData.DataValue=?" );
			
			if( m_query.Fetch() == S_OK )
				return true;
		}
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////
// 
// AssignToAgentWhoOwnsContact
// 
////////////////////////////////////////////////////////////////////////////////
bool CMsgRouter::AssignToAgentWhoOwnsContact(void)
{
	if( m_bNewContact )
		return false;

	// Does contact have an owner?	
	m_query.Reset( true );
	BINDPARAM_LONG( m_query, m_msg.m_ContactID );
	BINDCOL_LONG( m_query, m_tkt.m_OwnerID );
	m_query.Execute( L"SELECT OwnerID FROM Contacts "
					 L"WHERE ContactID=?" );

	if( ( m_query.Fetch() == S_OK ) && ( m_tkt.m_OwnerID != 0 ) )
		return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// AssignToAgentByRoundRobin
// 
////////////////////////////////////////////////////////////////////////////////
bool CMsgRouter::AssignToAgentByRoundRobin(int DenyOnline, int DenyAway, int DenyNotAvail, int DenyOffline, int DenyOoo)
{
	int nAgentSelected=0;	
	int nMinAgentID=0;
	int nAgentID=0;
	int nStatusID=0;
	bool bOwnerSet=false;
	bool bMinSet=false;
	dca::String t;

	DebugReporter::Instance().DisplayMessage("CMsgRouter::AssignToAgentByRoundRobin - Entering", DebugReporter::ENGINE);
	
	if( m_pRule != NULL )
	{
		// Do TicketBox Routing records exist for this Routing Rule?		
		m_query.Initialize();
		BINDPARAM_LONG( m_query, m_pRule->m_RoutingRuleID );
		BINDCOL_LONG_NOLEN( m_query, nAgentID );
		BINDCOL_LONG_NOLEN( m_query, nStatusID );
		m_query.Execute( L"SELECT ar.AgentID,a.StatusID "
						 L"FROM AgentRouting AS ar INNER JOIN Agents AS a ON ar.AgentID=a.AgentID "
						 L"WHERE ar.RoutingRuleID=? AND a.IsDeleted=0 AND a.IsEnabled=1 Order By ar.AgentID" );
		
		while( m_query.Fetch() == S_OK )
		{
			t.Format("CMsgRouter::AssignToAgentByRoundRobin - AgentID: [%d] StatusID: [%d] LastOwnerID: [%d] MinAgentID: [%d] AgentSelected: [%d]",  nAgentID,nStatusID, m_pRule->m_LastOwnerID,nMinAgentID,nAgentSelected);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
			
			if (!bMinSet)
			{
				bool bSkip = false;

				switch( nStatusID )
				{
				case 1:
					if( DenyOnline > 0 )
					{
						bSkip = true;
					}
					break;
					
				case 2:
					if( DenyAway > 0 )
					{
						bSkip = true;
					}
					break;
					
				case 3:
					if( DenyNotAvail > 0 )
					{
						bSkip = true;
					}
					break;
					
				case 5:
					if( DenyOffline > 0 )
					{
						bSkip = true;
					}
					break;

				case 6:
					if( DenyOoo > 0 )
					{
						bSkip = true;
					}
					break;
				}
				if(!bSkip)
				{
                    nMinAgentID = nAgentID;
                    bMinSet=true;
				}
			}

			if (nAgentID > m_pRule->m_LastOwnerID && nAgentSelected == 0)
			{
				bool bSkip = false;				

				switch( nStatusID )
				{
				case 1:
					if( DenyOnline > 0 )
					{
						bSkip = true;
					}
					break;
					
				case 2:
					if( DenyAway > 0 )
					{
						bSkip = true;
					}
					break;
					
				case 3:
					if( DenyNotAvail > 0 )
					{
						bSkip = true;
					}
					break;
					
				case 5:
					if( DenyOffline > 0 )
					{
						bSkip = true;
					}
					break;

				case 6:
					if( DenyOoo > 0 )
					{
						bSkip = true;
					}
					break;
				}
				if(!bSkip)
                    nAgentSelected = nAgentID;
			}
		}		
			
		if ( nAgentSelected > 0 )
		{
			bOwnerSet=true;			
		}
		else if ( nMinAgentID > 0 )
		{
			bOwnerSet=true;
			nAgentSelected = nMinAgentID;
		}
		if(bOwnerSet)
		{
			t.Format("CMsgRouter::AssignToAgentByRoundRobin - Returning True, Agent Selected [%d]",  nAgentSelected);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

			m_tkt.m_OwnerID = nAgentSelected;
			m_pRule->m_LastOwnerID = nAgentSelected;

			m_query.Initialize();
			BINDPARAM_LONG( m_query, m_pRule->m_LastOwnerID );
			BINDPARAM_LONG( m_query, m_pRule->m_RoutingRuleID );
			m_query.Execute( L"Update RoutingRules SET LastOwnerID=? WHERE RoutingRuleID=?" );		

			return true;
		}
	}
	
	DebugReporter::Instance().DisplayMessage("CMsgRouter::AssignToAgentByRoundRobin - Returning False", DebugReporter::ENGINE);

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// AssignToAgentAlgorithm
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::AssignToAgentAlgorithm( int AlgID, int AgentID, int DenyOnline, int DenyAway, int DenyNotAvail, int DenyOffline, int DenyOoo )
{
	dca::String t;	
	t.Format("CMsgRouter::AssignToAgentAlgorithm - AlgID: [%d] AgentID: [%d] DenyOnline: [%d] DenyAway: [%d] DenyNotAvail: [%d] DenyOffline: [%d] DenyOoo: [%d]", AlgID,AgentID,DenyOnline,DenyAway,DenyNotAvail,DenyOffline,DenyOoo);
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	
	switch( AlgID )
	{
	case 0:
		m_tkt.m_OwnerID = AgentID;
		break;

	case 1:
		if( AssignToAgentWithMatchingAddress() == false )
		{
			m_tkt.m_OwnerID = AgentID;
		}
		break;

	case 2:
		if( AssignToAgentWhoOwnsContact() == false )
		{
			m_tkt.m_OwnerID = AgentID;
		}
		break;

	case 3:
		if( AssignToAgentWithMatchingAddress() == false )
		{
			if( AssignToAgentWhoOwnsContact() == false )
			{
				m_tkt.m_OwnerID = AgentID;
			}
		}
		break;

	case 4:
		if( AssignToAgentWhoOwnsContact() == false )
		{
			if( AssignToAgentWithMatchingAddress() == false )
			{
				m_tkt.m_OwnerID = AgentID;
			}
		}
		break;
	
	case 5:
		if( AssignToAgentByRoundRobin(DenyOnline, DenyAway, DenyNotAvail, DenyOffline, DenyOoo) == false )
		{
			if( AssignToAgentWithMatchingAddress() == false )
			{
				m_tkt.m_OwnerID = AgentID;
			}
		}
		break;
	}
	
	if(AlgID < 5)
	{
		int nStatusID = -1;
		map<unsigned int,unsigned int>::iterator iter;
		iter = m_AgentStatusMap.begin();
		while( iter != m_AgentStatusMap.end() )
		{
			if(m_tkt.m_OwnerID == iter->first)
			{
				nStatusID = iter->second;
				break;
			}
			iter++;
		}
		
		t.Format("CMsgRouter::AssignToAgentAlgorithm - AgentID: [%d] nStatusID: [%d]", AgentID,nStatusID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		
		if(nStatusID > 0)
		{
			switch( nStatusID )
			{
			case 1:
				if( DenyOnline > 0 )
				{
					m_tkt.m_OwnerID = 0;
				}
				break;
				
			case 2:
				if( DenyAway > 0 )
				{
					m_tkt.m_OwnerID = 0;
				}
				break;
				
			case 3:
				if( DenyNotAvail > 0 )
				{
					m_tkt.m_OwnerID = 0;
				}
				break;
				
			case 5:
				if( DenyOffline > 0 )
				{
					m_tkt.m_OwnerID = 0;
				}
				break;

			case 6:
				if( DenyOoo > 0 )
				{
					m_tkt.m_OwnerID = 0;
				}
				break;
			}
		}
		
		t.Format("CMsgRouter::AssignToAgentAlgorithm - OwnerID: [%d]", m_tkt.m_OwnerID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// CreateNewTicket
// 
////////////////////////////////////////////////////////////////////////////////
unsigned int CMsgRouter::CreateNewTicket( void )
{
	// Determine who owns this contact if anybody
	if( m_pRule )
	{
		m_AutoReplyCloseTicketFlag = ( m_pRule->m_AutoReplyEnable != 0 
			                           && m_pRule->m_AutoReplyCloseTicket != 0) ? true : false;

		AssignToAgentAlgorithm( m_pRule->m_AssignToAgentAlg, m_pRule->m_AssignToAgent, m_pRule->nOnline, m_pRule->nAway, m_pRule->nNotAvail, m_pRule->nOffline, m_pRule->nOoo);

		m_tkt.m_TicketStateID = ( m_AutoReplyCloseTicketFlag
							  || (m_RoutingRuleID != 0 && (m_pRule->m_DeleteImmediatelyEnable && 
							                               m_pRule->m_DeleteImmediately == EMS_DELETE_OPTION_DELETE_TO_WASTE_BASKET)))
							? EMS_TICKETSTATEID_CLOSED : EMS_TICKETSTATEID_OPEN;
	}
	else
	{
		m_tkt.m_TicketStateID = EMS_TICKETSTATEID_OPEN;
	}

	// create a new ticket
	CreateNewTicketForContact();

	return m_tkt.m_TicketID;
}



////////////////////////////////////////////////////////////////////////////////
// 
// UnDeleteTicket
// 
////////////////////////////////////////////////////////////////////////////////
int CMsgRouter::UnDeleteTicket( void )
{
	m_query.Initialize();

	// make the message a part of the ticket

	BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
	
	m_query.Execute( L"UPDATE Tickets "
				     L"SET IsDeleted=0 "
				     L"WHERE TicketID=?");

	return m_query.GetRowCount();
}


int CMsgRouter::SetTicketState(  long TicketStateID )
{
	THChangeState( m_query, m_tkt.m_TicketID, 0, m_tkt.m_TicketStateID, TicketStateID, m_tkt.m_TicketBoxID, m_tkt.m_OwnerID, m_tkt.m_PriorityID, m_tkt.m_TicketCategoryID );
	
	switch( TicketStateID )
	{
	case EMS_TICKETSTATEID_OPEN:
		{
			m_query.Initialize();

			BINDPARAM_LONG( m_query, TicketStateID );
			BINDPARAM_TIME_NOLEN( m_query, m_msg.m_DateReceived );
			BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
			
			m_query.Execute( L"UPDATE Tickets "
						     L"SET TicketStateID=?, OpenTimestamp=?, UseTickler = 0, TicklerDateToReopen = NULL "
						     L"WHERE TicketID=?" );	

			return m_query.GetRowCount();
		}
	case EMS_TICKETSTATEID_CLOSED:
	case EMS_TICKETSTATEID_ONHOLD:
	case EMS_TICKETSTATEID_ESCALATED:
	case EMS_TICKETSTATEID_REOPENED:
		{
			TIMESTAMP_STRUCT CurrentTime;
			
			m_query.Initialize();
			
			BINDCOL_LONG_NOLEN( m_query, m_tkt.m_TicketStateID );
			BINDCOL_LONG_NOLEN( m_query, m_tkt.m_OpenMins );
			BINDCOL_TIME( m_query, m_tkt.m_OpenTimestamp );
			
			BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
			
			m_query.Execute( _T("SELECT TicketStateID, OpenMins, OpenTimestamp FROM Tickets WHERE TicketID = ?" ));
					
			// if the ticket wasn't "on hold" calculate the minutes
			// the ticket was open...
			if ( m_tkt.m_TicketStateID == EMS_TICKETSTATEID_ESCALATED 
				|| m_tkt.m_TicketStateID == EMS_TICKETSTATEID_OPEN )
			{
				GetTimeStamp(CurrentTime);
				
				int nLastOpenSecs = m_tkt.m_OpenTimestampLen != SQL_NULL_DATA ? CompareTimeStamps( m_tkt.m_OpenTimestamp, CurrentTime ) : 0;
				
				if (nLastOpenSecs > 0)
				{
					// update the open minute count
					m_tkt.m_OpenMins += (nLastOpenSecs / 60);

					// round to the next minute?
					if ( (nLastOpenSecs % 60) >= 30 )
						m_tkt.m_OpenMins++;
				}
				
				m_query.Initialize();

				BINDPARAM_LONG( m_query, TicketStateID );
				BINDPARAM_LONG( m_query, m_tkt.m_OpenMins );
				BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
				
				m_query.Execute( L"UPDATE Tickets "
								 L"SET TicketStateID=?, OpenMins=?, UseTickler = 0, TicklerDateToReopen = NULL "
								 L"WHERE TicketID=?" );
			}
			else
			{
				m_query.Initialize();

				BINDPARAM_LONG( m_query, TicketStateID );
				BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
				
				m_query.Execute( L"UPDATE Tickets "
								 L"SET TicketStateID=? "
								 L"WHERE TicketID=?" );
			}
		}

	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// CreateNewTicketForContact
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::CreateNewTicketForContact( void )
{
	set<long> EscalateAgentIDs;
	tstring strSubject;
	CEMSString strAlertSubject;
	CEMSString strAlertBody;
	dca::WString strEmptyString = "";

	list<int> AgentIDs;
	list<int>::iterator iter;
	int AgentID;
	int iNine = 9;
		
	ValidateOwnerID( EscalateAgentIDs );

	strSubject.assign( m_msg.m_Subject );

	// Remove the ticketID if present
	RemoveTicketIDFromSubject( strSubject );
	
	// Create the Tickets row
	m_query.Initialize();

	BINDPARAM_LONG( m_query, m_tkt.m_TicketStateID );
	BINDPARAM_LONG( m_query, m_tkt.m_TicketBoxID );
	BINDPARAM_LONG( m_query, m_tkt.m_OwnerID );
	BINDPARAM_LONG( m_query, m_tkt.m_PriorityID );		
	BINDPARAM_TIME_NOLEN( m_query, m_msg.m_DateReceived );
	BINDPARAM_TIME_NOLEN( m_query, m_msg.m_DateReceived );
	BINDPARAM_WCHAR( m_query, m_szContactName );
	BINDPARAM_TCHAR_STRING( m_query, strSubject );
	BINDPARAM_LONG( m_query, m_tkt.m_TicketCategoryID );
	
	dca::WString w;
	w.Format(L"CMsgRouter::CreateNewTicketForContact - Ready to insert into Tickets, TicketBoxID [%d]", m_tkt.m_TicketBoxID);
	dca::String t(w.c_str());
	//DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

	m_query.Execute( L"INSERT INTO Tickets "
				   L"(TicketStateID,TicketBoxID,OwnerID,PriorityID,DateCreated,OpenTimestamp,IsDeleted,Contacts,Subject,TicketCategoryID) " 
				   L"VALUES "
				   L"(?,?,?,?,?,?,1,?,?,?)" );

	m_tkt.m_TicketID = m_query.GetLastInsertedID();
	m_msg.m_TicketID = m_tkt.m_TicketID;
	
	// Add row to TicketContacts

	m_query.Reset(true);

	BINDPARAM_LONG( m_query, m_tkt.m_TicketID );
	BINDPARAM_LONG( m_query, m_msg.m_ContactID );

	m_query.Execute( L"INSERT INTO TicketContacts "
				   L"(TicketID,ContactID) "
				   L"VALUES "
				   L"(?,?)" );	

	//Get list of TicketFields configured for all TicketBoxes
	TTicketFieldsTicket tft;
	m_tft.clear();
	m_query.Reset(true);
	BINDCOL_LONG( m_query, tft.m_TicketFieldID );
	BINDCOL_TCHAR( m_query, tft.m_DataValue );
	BINDPARAM_LONG( m_query, m_tkt.m_TicketBoxID );
	m_query.Execute( L"SELECT TicketFieldID,DefaultValue FROM TicketFieldsTicketBox WHERE SetDefault=1 AND TicketBoxID=?" );	              
	while( m_query.Fetch() == S_OK )
	{
		tft.m_TicketID = m_tkt.m_TicketID;
		m_tft.push_back(tft);
	}

	for( tftIter = m_tft.begin(); tftIter != m_tft.end(); tftIter++ )
	{
		// Add any default custom ticket fields
		AddTicketFieldTicket(tftIter->m_TicketID,tftIter->m_TicketFieldID,tftIter->m_DataValue);
	}
	
	// Log it in TicketHistory
	THCreateTicket( m_query, m_tkt.m_TicketID, 0, ( m_pRule != NULL ) ? m_pRule->ID() : 0, m_tkt.m_TicketStateID, m_tkt.m_TicketBoxID, m_tkt.m_OwnerID, m_tkt.m_PriorityID, m_tkt.m_TicketCategoryID );
		
	// Alert to Agent, if enabled
	if( m_pRule && m_pRule->m_AlertEnable && m_pRule->m_AlertToAgentID != 0 )
	{
		//Form the Subject
		if( m_pRule->m_AlertIncludeSubject )
		{
			
			strAlertSubject.Format( _T("Ticket [ %d ] Created :: %s"), m_msg.m_TicketID, strSubject.c_str() );
		}
		else
		{
			strAlertSubject.Format( _T("Ticket [ %d ] Created"), m_msg.m_TicketID );
		}
		
		//Form the Body
		if( m_pRule->m_AlertText != strEmptyString )
		{
			if( m_pRule->m_AlertIncludeSubject )
			{
				strAlertBody.Format( _T("%s\n\nTicket #%d %s\r\n"), m_pRule->m_AlertText, m_msg.m_TicketID, strSubject.c_str() );
			}
			else
			{
				strAlertBody.Format( _T("%s\n\nTicket #%d\r\n"), m_pRule->m_AlertText, m_msg.m_TicketID );
			}
		}
		else
		{
			if( m_pRule->m_AlertIncludeSubject )
			{
				strAlertBody.Format( _T("Ticket Created\n\nTicket %d %s\r\n"), m_msg.m_TicketID, strSubject.c_str() );
			}
			else
			{
				strAlertBody.Format( _T("Ticket Created\n\nTicket #%d\r\n"), m_msg.m_TicketID );
			}
		}
		
		// Create the Alert
		
		//If group, get the agents in the group
		if( m_pRule->m_AlertToAgentID < 0 )
		{
			// Get agentids for group
			int iGroupID = abs(m_pRule->m_AlertToAgentID);
	
			m_query.Initialize();
			BINDPARAM_LONG( m_query, iGroupID );
			BINDCOL_LONG_NOLEN( m_query, AgentID );
			m_query.Execute( _T("SELECT AgentID ")
							_T("FROM AgentGroupings ")
							_T("WHERE GroupID=?") );

			while( m_query.Fetch() == S_OK )
			{
				AgentIDs.push_back( AgentID );
			}
		}
		else
		{
			AgentIDs.push_back( m_pRule->m_AlertToAgentID );			
		}

		for( iter = AgentIDs.begin(); iter != AgentIDs.end(); iter++ )
		{
			m_query.Initialize();
			BINDPARAM_LONG( m_query, iNine );
			BINDPARAM_LONG( m_query, *iter );
			BINDPARAM_TCHAR_STRING( m_query, strAlertSubject );
			BINDPARAM_TCHAR_STRING( m_query, strAlertBody );
			m_query.Execute( L"INSERT INTO AlertMsgs "
						L"(AlertEventID,AgentID,Subject,Body ) " 
						L"VALUES "
						L"(?,?,?,?)" );
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// ShredRule
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::ShredRule( void )
{
	bool bCheckAgentAlerts = true;
	
	if( m_RoutingRuleID != 0 )
	{	
		if( m_pRule->m_DeleteImmediatelyEnable )
		{
			bCheckAgentAlerts = false;		
			
			if( m_pRule->m_DeleteImmediately == EMS_DELETE_OPTION_DELETE_TO_WASTE_BASKET )
			{
				// TO DO: check return

				m_msg.Delete( true );
				
				THDelInboundMsg( m_query, m_tkt.m_TicketID, 0, m_msg.m_InboundMessageID, m_tkt.m_TicketBoxID );

				Log( E_RouteToTrash, EMS_STRING_RTR_ROUTE_TO_TRASH );
			}
			else if ( m_pRule->m_DeleteImmediately == EMS_DELETE_OPTION_DELETE_PERMANENTLY )
			{
				// TO DO: check return
				
				m_msg.Delete( false );

				Log( E_DeletePermanently, EMS_STRING_RTR_DELETE_PERMANENTLY );
			}
		}
	}
	
	if ( bCheckAgentAlerts )
	{
		//Check to see if we need to send any Agent Alerts
		try
		{
			int nTicketOwnerID = m_tkt.m_OwnerID;
			int nContactOwnerID = 0;
			dca::String t;
			
			if(m_msg.m_ContactID > 0)
			{
				//Try to get the contact owner ID
				m_query.Reset();
				BINDPARAM_LONG( m_query, m_msg.m_ContactID );
				BINDCOL_LONG_NOLEN( m_query, nContactOwnerID );
				m_query.Execute( L"SELECT OwnerID FROM Contacts WHERE ContactID=? ");
				m_query.Fetch();
			}

			if(nContactOwnerID > 0 && (nContactOwnerID != nTicketOwnerID))
			{
				t.Format("CMsgRouter::ShredRule - Ready to check for Agent Alerts for Contact Owner AgentID [%d]", nContactOwnerID);
				//DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
				CheckAgentAlerts(nContactOwnerID, true);
			}
			
			if(nTicketOwnerID > 0 )
			{
				t.Format("CMsgRouter::ShredRule - Ready to check for Agent Alerts for Ticket Owner AgentID [%d]", nTicketOwnerID);
				//DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
				CheckAgentAlerts(nTicketOwnerID, false);			
			}
		}
		catch( ODBCError_t error )
		{
			Log( E_RoutingException, L"ODBC Error %s checking agent alerts %d in state %d\n",
				error.szErrMsg, m_msg.m_InboundMessageID, m_MRState );
			m_nFailedCnt++;
			throw;
		}	
		catch( ... )
		{
			Log( E_RoutingException, L"Unhandled exception checking agent alerts %d in state %d\n",
				m_msg.m_InboundMessageID, m_MRState );
			m_nFailedCnt++;
			throw;
		}	
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// RemoveFromQueue
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::RemoveFromQueue(void)
{
	m_msg.RemoveFromInboundQueue();
	ShredRule();
	m_MRState = MRState_Finished;			
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetDSN - Get the DSN connection string from the registry
// 
////////////////////////////////////////////////////////////////////////////////
unsigned int CMsgRouter::GetRegDword( wchar_t* szName )
{
	DWORD	dwValue = 0;
	DWORD	dwSize = sizeof(DWORD);
	DWORD	dwType = REG_DWORD;
	HKEY	hKey;
	
	// open the key
	if (RegOpenKeyEx( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 0, KEY_EXECUTE, &hKey) == ERROR_SUCCESS)
	{
		// get the value from the registry
		RegQueryValueEx(hKey, szName, NULL, &dwType, (LPBYTE)&dwValue, &dwSize);

		// close the key
		RegCloseKey(hKey);
	}

	return dwValue;
}


void CMsgRouter::SetRegDword( wchar_t* szName, DWORD dwValue )
{
	HKEY	hKey;
	
	// open the key
	if (RegOpenKeyEx( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
	{
		// Set the value
		RegSetValueEx( hKey, szName, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD) );

		// close the key
		RegCloseKey(hKey);
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Initialize - restore state
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::Initialize(void)
{
	int OldState = 0;

	// Get the rule list, if this is first time
	if( m_MRState == MRState_UnInitialized)
	{
		RefreshRules();
	}

	// ok, we're ready to go
	OldState = GetRegDword( L"MsgRouterState" );

	switch( OldState )
	{
	case MRState_BumpHitCount:
	case MRState_Forward:
	case MRState_AutoReply:
	case MRState_PutInTicketBox:
	case MRState_RemoveFromQueue:

		// set the state
		m_MRState = OldState;

		// Get ContactID, RoutingRuleID and InboundMessageID
		m_msg.m_ContactID = GetRegDword( L"MsgRouterContactID" );
		GetContactName();
		
		m_RoutingRuleID = GetRegDword( L"MsgRouterRoutingRuleID" );
		m_msg.m_InboundMessageID = GetRegDword( L"MsgRouterInboundMessageID" );
		m_tkt.m_TicketID = GetRegDword( L"MsgRouterTicketID" );
		m_bNewTicket = (GetRegDword( L"MsgRouterNewTicket" ) == 0) ? false : true;

		// find the rule that matched.
		m_pRule = m_rules.FindRule( m_RoutingRuleID );
		
		// pull the new message from the database.
		m_msg.Query( m_query );

		// The matching rule disappeared!
		if( m_pRule == NULL )
		{
			m_MRState = MRState_FindRuleMatch;
		}
		else
		{
			m_tkt.m_TicketBoxID = m_pRule->m_AssignToTicketBox;
			if( m_pRule->m_PriorityID )
			{
				m_tkt.m_PriorityID = m_pRule->m_PriorityID;		
			}
			else
			{
				m_tkt.m_PriorityID = m_msg.m_PriorityID;		
			}
		}
		break;


	default:
		m_MRState = MRState_GetMessage;
		break;
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
//  Run
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CMsgRouter::Run(void)
{	
	
	if ( _wtoi(g_Object.GetParameter( EMS_SRVPARAM_MSG_ROUTER ))  )
	{
		DebugReporter::Instance().DisplayMessage("CMsgRouter::Run", DebugReporter::ENGINE);
		
		// Reset our state if finished
		if( m_MRState == MRState_Finished )
		{
			m_MRState = MRState_RefreshRules;
		}

		while( m_MRState != MRState_Finished )
		{

			m_nLastState = m_MRState;

			try
			{
				if( m_nFailedCnt > 10 )
				{
					// Attempt to process via the Default Routing Rule
					dca::String t;	
					t.Format("CMsgRouter::Run - InboundMessage [%d] unroutable, attempting to process via the Default Routing Rule", m_msg.m_InboundMessageID);
					DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
					
					Log( E_RoutingException, L"Inbound message %d unroutable, attempting to process via the Default Routing Rule",
							m_msg.m_InboundMessageID );

					try
					{
						ImplicitRoutingRule();
						PutInTicketBox();
						RemoveFromQueue();
						m_nFailedCnt = 0;						
					}
					catch(...)
					{
						dca::String t;	
						t.Format("CMsgRouter::Run - InboundMessage [%d] unroutable, removing from queue", m_msg.m_InboundMessageID);
						DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
											
						Log( E_RoutingException, L"Inbound message %d unroutable, removing from inbound queue",
							m_msg.m_InboundMessageID );
						
						RemoveFromQueue();
						m_nFailedCnt = 0;
						m_MRState = MRState_RefreshRules;
					}
				}

				switch( m_MRState )
				{
				case MRState_UnInitialized:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Initialize", DebugReporter::ENGINE);
					Initialize();
					break;

				case MRState_RefreshRules:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Refresh rules", DebugReporter::ENGINE);
					RefreshRules();
					break;

				case MRState_GetMessage:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Get new messages", DebugReporter::ENGINE);
					GetNewMessage();
					break;

				case MRState_CheckContact:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Check contact", DebugReporter::ENGINE);
					CheckContact();
					break;

				case MRState_FindRuleMatch:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Find rule match", DebugReporter::ENGINE);
					FindRuleMatch();
					break;

				case MRState_BumpHitCount:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Bump hit count", DebugReporter::ENGINE);
					BumpHitCount();
					break;

				case MRState_PutInTicketBox:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Put in ticket box", DebugReporter::ENGINE);
					PutInTicketBox();
					break;

				case MRState_CreateAutoReplies:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Create auto replies", DebugReporter::ENGINE);
					CreateAutoReplies();
					break;

				case MRState_Forward:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Forward copy", DebugReporter::ENGINE);
					ForwardCopy();
					break;

				case MRState_AutoReply:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Auto reply", DebugReporter::ENGINE);
					AutoReply();
					break;

				case MRState_RemoveFromQueue:
					DebugReporter::Instance().DisplayMessage("CMsgRouter::Run - Remove from queue", DebugReporter::ENGINE);
					RemoveFromQueue();
					break;
				}
			}
			catch( ODBCError_t error )
			{
				dca::String t;
				dca::String x(error.szErrMsg);
				t.Format("CMsgRouter::Run - ODBC Error %s routing inbound message %d in state %d", x.c_str(), m_msg.m_InboundMessageID, m_MRState);
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

				Log( E_RoutingException, L"ODBC Error %s routing inbound message %d in state %d\n",
					error.szErrMsg, m_msg.m_InboundMessageID, m_MRState );
				m_nFailedCnt++;
				throw;
			}	
			catch( ... )
			{
				dca::String t;
				t.Format("CMsgRouter::Run - Unhandled exception routing inbound message %d in state %d", m_msg.m_InboundMessageID, m_MRState);
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

				Log( E_RoutingException, L"Unhandled exception routing inbound message %d in state %d\n",
					m_msg.m_InboundMessageID, m_MRState );
				m_nFailedCnt++;				
			}

			if( (m_nLastState == m_MRState) && 
				(m_MRState > MRState_GetMessage) && 
				(m_MRState < MRState_RemoveFromQueue) )
			{
				m_nFailedCnt++;
			}
			else
			{
				m_nFailedCnt = 0;
			}
		}


		if(m_bGotNewMsg)
		{
			return S_OK;
		}
	}

	// nothing to do!
	return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// 
// UnInitialize - save state
// 
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::UnInitialize(void)
{
	switch( m_MRState )
	{
	case MRState_Finished:
	case MRState_RefreshRules:
		// Nothing to do!
		break;

	case MRState_GetMessage:
	case MRState_CheckContact:
	case MRState_FindRuleMatch:
		// These steps can be repeated without harm
		m_MRState = MRState_RefreshRules;
		break;

	case MRState_BumpHitCount:
	case MRState_Forward:
	case MRState_AutoReply:
	case MRState_PutInTicketBox:
	case MRState_RemoveFromQueue:
		// Save ContactID, RoutingRuleID and InboundMessageID
		SetRegDword( L"MsgRouterContactID", m_msg.m_ContactID );
		SetRegDword( L"MsgRouterRoutingRuleID", m_RoutingRuleID );
		SetRegDword( L"MsgRouterInboundMessageID", m_msg.m_InboundMessageID );
		SetRegDword( L"MsgRouterTicketID", m_tkt.m_TicketID );
		SetRegDword( L"MsgRouterNewTicket", (m_bNewTicket) ? 1 : 0 );
		break;
	}

	// Save Current State
	SetRegDword( L"MsgRouterState", m_MRState );
}

////////////////////////////////////////////////////////////////////////////////
// 
// FinishCurrentJob
// 
////////////////////////////////////////////////////////////////////////////////
int CMsgRouter::FinishCurrentJob(void)
{
	// Force a rule refresh for the next iteration
	m_rules.m_LastRefreshTime = 0;
	
	switch( m_MRState )
	{
		// Idle state - nothing to be done
		case MRState_RefreshRules:
		case MRState_UnInitialized:
		case MRState_Finished:
			return 0;

		// Some processing has been done, but can be discarded.
		case MRState_GetMessage:
		case MRState_CheckContact:
		case MRState_FindRuleMatch:
			m_MRState = MRState_RefreshRules;
			return 0;

		default:
			while( m_MRState != MRState_Finished )
			{
				Run();
			}
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// ValidateOwnerID - verify that Agent corresponding to the ownerID exists in 
//                   the database, and is enabled and not deleted.
// 
// Since the escalation is recursive, the AgentIDs set is used to prevent
// loops. If a loop is detected, the ticket owner will be reset to 0
//
////////////////////////////////////////////////////////////////////////////////
void CMsgRouter::ValidateOwnerID( set<long> AgentIDs )
{
	long IsEnabled;
	long IsDeleted;
	long EscalateToAgentID;
	
	m_query.Initialize();
	
	BINDPARAM_LONG( m_query, m_tkt.m_OwnerID );
	BINDCOL_LONG_NOLEN( m_query, IsEnabled );
	BINDCOL_LONG_NOLEN( m_query, IsDeleted );
	BINDCOL_LONG_NOLEN( m_query, EscalateToAgentID );

	m_query.Execute( L"SELECT IsEnabled,IsDeleted,EscalateToAgentID "
				     L"FROM Agents " 
					 L"WHERE AgentID=?" );

	if( m_query.Fetch() == S_OK )
	{
		if( IsEnabled == 0 || IsDeleted != 0 )
		{
			if( EscalateToAgentID != 0 && AgentIDs.find(EscalateToAgentID) == AgentIDs.end() )
			{
				m_tkt.m_OwnerID = EscalateToAgentID;

				AgentIDs.insert( EscalateToAgentID );

				ValidateOwnerID( AgentIDs );
			}
			else
			{
				// Reset owner
				m_tkt.m_OwnerID = 0;
			}
		}
	}
	else
	{
		// Reset owner
		m_tkt.m_OwnerID = 0;
	}
}

void CMsgRouter::CheckOwnerStatus( void )
{
	int nStatusID;
	
	m_query.Initialize();	
	BINDPARAM_LONG( m_query, m_tkt.m_OwnerID );
	BINDCOL_LONG_NOLEN( m_query, nStatusID );

	m_query.Execute( L"SELECT StatusID "
				     L"FROM Agents " 
					 L"WHERE AgentID=? AND IsEnabled=1 AND IsDeleted=0" );

	if( m_query.Fetch() == S_OK )
	{
		if(m_pRule)
		{
			if((nStatusID == 1 && m_pRule->nOnline == 1) || (nStatusID == 2 && m_pRule->nAway == 1) || (nStatusID == 3 && m_pRule->nNotAvail == 1) || (nStatusID == 5 && m_pRule->nOffline == 1) || (nStatusID == 6 && m_pRule->nOoo == 1))
			{
				m_tkt.m_OwnerID = 0;			
			}
		}
		else
		{
			int nDenyOnline = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_ONLINE ) );
			int nDenyAway = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_AWAY ) );
			int nDenyNotAvail = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_NOTAVAIL ) );
			int nDenyOffline = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_OFFLINE ) );
			int nDenyOoo = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DEF_RR_OOO ) );
			if((nStatusID == 1 && nDenyOnline == 1) || (nStatusID == 2 && nDenyAway == 1) || (nStatusID == 3 && nDenyNotAvail == 1) || (nStatusID == 5 && nDenyOffline == 1) || (nStatusID == 6 && nDenyOoo == 1))
			{
				m_tkt.m_OwnerID = 0;			
			}
		}
	}
	else
	{
		m_tkt.m_OwnerID = 0;
	}
}

bool CMsgRouter::GetTicketIDFromHeader( int InboundMessageID, int& TicketID, int& MsgID )
{
	MessageTracking_t mt;
	m_query.Initialize();	
	BINDPARAM_LONG( m_query, InboundMessageID );
	BINDCOL_TCHAR( m_query, mt.m_HeadInReplyTo );
	BINDCOL_TCHAR( m_query, mt.m_HeadReferences );
	m_query.Execute( L"SELECT HeadInReplyTo,HeadReferences "
				     L"FROM MessageTracking " 
					 L"WHERE MessageID=? AND MessageSourceID > 0" );

	if( m_query.Fetch() == S_OK )
	{
		if( _stscanf( mt.m_HeadInReplyTo, _T("<VisNetic.MailFlow.%d.%d"), &TicketID, &MsgID ) == 2 )
		{
			return true;
		}
		else if( _stscanf( mt.m_HeadInReplyTo, _T("VisNetic.MailFlow.%d.%d"), &TicketID, &MsgID ) == 2 )
		{
			return true;
		}
		else
		{
			if( _stscanf( mt.m_HeadReferences, _T("<VisNetic.MailFlow.%d.%d"), &TicketID, &MsgID ) == 2 )
			{
				return true;
			}
			else if( _stscanf( mt.m_HeadReferences, _T("VisNetic.MailFlow.%d.%d"), &TicketID, &MsgID ) == 2 )
			{
				return true;
			}

			try
			{
				tstring sHead(mt.m_HeadReferences);
				tstring::size_type pos = sHead.find( _T("VisNetic.MailFlow.") );
				tstring sTemp = sHead.substr(pos,sHead.length());
				_tcscpy( mt.m_HeadReferences,sTemp.c_str() );
				if( _stscanf( mt.m_HeadReferences, _T("VisNetic.MailFlow.%d.%d"), &TicketID, &MsgID ) == 2 )
				{
					return true;
				}
			}
			catch(...)
			{
				return false;
			}
		}
	}
	return false;
}

bool CMsgRouter::SetEscapeCharacter(TCHAR* pEscChar, dca::WString& rSrchStr)
{
	dca::WString::size_type npos = rSrchStr.find(0x0021);	// !

	if(npos == dca::WString::npos)
	{
		pEscChar[0] = 0x0021;
		return true;
	}

	npos = rSrchStr.find(0x007E);	// ~
	if(npos == dca::WString::npos)
	{
		pEscChar[0] = 0x007E;
		return true;
	}

	npos = rSrchStr.find(0x0040);	// @
	if(npos == dca::WString::npos)
	{
		pEscChar[0] = 0x0040;
		return true;
	}

	npos = rSrchStr.find(0x0023);	// #
	if(npos == dca::WString::npos)
	{
		pEscChar[0] = 0x0023;
		return true;
	}

	npos = rSrchStr.find(0x0024); // $
	if(npos == dca::WString::npos)
	{
		pEscChar[0] = 0x0024;
		return true;
	}

	npos = rSrchStr.find(0x0026);	// &
	if(npos == dca::WString::npos)
	{
		pEscChar[0] = 0x0026;
		return true;
	}

	npos = rSrchStr.find(0x002A);	// *
	if(npos == dca::WString::npos)
	{
		pEscChar[0] = 0x002A;
		return true;
	}

	npos = rSrchStr.find(0x007C); // |
	if(npos == dca::WString::npos)
	{
		pEscChar[0] = 0x007C;
		return true;
	}

	npos = rSrchStr.find(0x003F);
	if(npos == dca::WString::npos)
	{
		pEscChar[0] = 0x003F; // ?
		return true;
	}

	return false;
}

bool CMsgRouter::UpdateSubjectWithEscape(dca::WString& rSubject, TCHAR tChar, TCHAR* pEscChar)
{
	dca::WString::size_type pos = rSubject.find(tChar);
	bool bReturn = false;

	if(pos != dca::WString::npos)
	{
		pos = 0;
		do
		{
			pos = rSubject.find(tChar, pos);

			if(pos != dca::WString::npos)
			{
				rSubject.insert(pos, pEscChar, 1);
				pos++;
				pos++;

				bReturn = true;

				if(pos >= rSubject.size())
					pos = dca::WString::npos;
			}

		}while(pos != dca::WString::npos);
	}

	return bReturn;
}

bool CMsgRouter::CleanupReplyInSubject(dca::WString& rSubject)
{
	TCHAR sREPrefixes[] = _T("RE: \0Re: \0re: \0rE: \0\0");

	TCHAR* pPrefix = sREPrefixes;

	while(pPrefix[0] != 0x00)
	{
		dca::WString::size_type pos = rSubject.find(pPrefix, 0);

		if(pos != dca::WString::npos && pos == 0)
		{
			rSubject = rSubject.erase(0, lstrlen(pPrefix));
			return true;
		}

		pPrefix += (lstrlen(pPrefix) + 1);;
	}

	return false;
}

bool CMsgRouter::CleanupForwardInSubject(dca::WString& rSubject)
{
	TCHAR sREPrefixes[] = _T("FW: \0Fw: \0fw: \0fW: \0FWD: \0FwD: \0FWd: \0Fwd: \0fwd: \0fWd: \0fWD: \0fwD: \0\0");

	TCHAR* pPrefix = sREPrefixes;

	while(pPrefix[0] != 0x00)
	{
		dca::WString::size_type pos = rSubject.find(pPrefix, 0);

		if(pos != dca::WString::npos && pos == 0)
		{
			rSubject = rSubject.erase(0, lstrlen(pPrefix));
			return true;
		}

		pPrefix += (lstrlen(pPrefix) + 1);
	}

	return false;
}

void CMsgRouter::CleanupSubject(dca::WString& rSubject)
{
	bool bNotClean = true;
	while(bNotClean)
	{
		bNotClean = CleanupReplyInSubject(rSubject);
		
		if(CleanupForwardInSubject(rSubject))
			bNotClean = true;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds or updates a TicketField	              
\*--------------------------------------------------------------------------*/
void CMsgRouter::AddTicketFieldTicket( long TicketID, long TicketFieldID, TCHAR* DataValue )
{
	try
	{
		tstring sVal;
		sVal.assign( DataValue );

		//$$DateTimeNow$$
		tstring::size_type pos = sVal.find( _T("$$DateTimeNow$$") );
		if( pos != tstring::npos )
		{
			TIMESTAMP_STRUCT Now;
			GetTimeStamp( Now );
			long NowLen = sizeof(Now);
			CEMSString sDate;
			GetDateTimeString( Now, NowLen, sDate );
			sVal.replace( pos, 15, sDate.c_str() );
			_sntprintf( DataValue, TICKETFIELDS_VALUE_LENGTH - 1, sVal.c_str() );
		}
		
		// Let's add it
		m_query.Initialize();
		BINDPARAM_LONG( m_query, TicketID );
		BINDPARAM_LONG( m_query, TicketFieldID );
		BINDPARAM_TCHAR( m_query, DataValue );
		m_query.Execute( L"INSERT INTO TicketFieldsTicket (TicketID,TicketFieldID,DataValue) VALUES (?,?,?)" );		
	}
	catch( ODBCError_t error )
	{
		Log( E_RoutingException, L"ODBC Error %s adding custom ticket field default value for ticket %d\n",
				error.szErrMsg, TicketID );
	}	
	catch( ... )
	{
		Log( E_RoutingException, L"Unhandled exception adding custom ticket field default value for ticket %d\n",
				TicketID );
	}	
}
