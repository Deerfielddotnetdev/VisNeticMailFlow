/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Ticket.cpp,v 1.2.2.4 2006/02/16 16:09:52 markm Exp $
||
||
||                                         
||  COMMENTS:  Ticket Functions 
||              
\\*************************************************************************/

#include "stdafx.h"
#include "Ticket.h"
#include "TicketHistoryFns.h"
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CTicket::CTicket(CISAPIData& ISAPIData, int nTicketID /*=0*/ ) : CXMLDataClass(ISAPIData)
{
	m_TicketID = nTicketID;
	m_nLockCount = 0;
	m_Access = EMS_NO_ACCESS;

	// set the created date
	GetTimeStamp( m_DateCreated );
	m_DateCreatedLen = sizeof(m_DateCreated);
	
	// set the open timestamp
	m_OpenTimestamp = m_DateCreated;
	m_OpenTimestampLen = sizeof(m_OpenTimestamp);
	
	// mark the item as partially created
	m_IsDeleted = EMS_DELETE_OPTION_DELETE_PERMANENTLY;
	
	// set other defaults required by foreign keys
	m_PriorityID    = EMS_PRIORITY_NORMAL;
	m_TicketBoxID   = EMS_TICKETBOX_UNASSIGNED;
	m_TicketStateID = EMS_TICKETSTATEID_OPEN;

	// the ticket can be unlocked during destruction
	m_bDoNotUnlock = false;

	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_LOG, sAal );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_DELETE, sTd );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_CREATE, sTc );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MOVE, sTm );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_ESCALATE, sTe );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_REASSIGN, sTr );	
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_RESTORE, sTrs );	
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_OPEN, sTo );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_CLOSE, sTcl );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_ADD, sTma );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_DELETE, sTmd );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_REVOKE, sTmrev );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_RELEASE, sTmrel );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_RETURN, sTmret );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_COPIED, sTmcop );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MERGE, sTmerg );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_SAVED, sTmsav );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_LINK_CREATED, sLc );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_LINK_DELETED, sLd );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_LINKED, sTl );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_UNLINKED, sTu );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_CHANGE_CATEGORY, sTcc );
	GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_CHANGE_FIELD, sTcf );
	GetServerParameter( EMS_SRVPARAM_DENY_TICKET_ONLINE, sDenyOnline );
	GetServerParameter( EMS_SRVPARAM_DENY_TICKET_AWAY, sDenyAway );
	GetServerParameter( EMS_SRVPARAM_DENY_TICKET_NOTAVAIL, sDenyNotAvail );
	GetServerParameter( EMS_SRVPARAM_DENY_TICKET_OFFLINE, sDenyOffline );
	GetServerParameter( EMS_SRVPARAM_DENY_TICKET_OOO, sDenyOoo );
	GetServerParameter( EMS_SRVPARAM_ALLOW_TICKET_ADMIN, sAllowAdmin );
	GetServerParameter( EMS_SRVPARAM_ALLOW_TICKET_AGENT, sAllowAgent );	
}

/*---------------------------------------------------------------------------\            
||  Comments:	Destruction
\*--------------------------------------------------------------------------*/
CTicket::~CTicket()
{
	// make sure that the ticket is unlocked
	if ( !m_bDoNotUnlock && (m_nLockCount && m_TicketID))
		UnLock(true);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates an XML node for the ticket
\*--------------------------------------------------------------------------*/
void CTicket::GenerateXML( void )
{	
	// add m_Ticket information
	CEMSString sDateTime;
	CEMSString sReopenTime;

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;

	if(lTzBias != 1)
	{
		if(ConvertToTimeZone(m_DateCreated,lTzBias,tsLocal))
		{
			GetDateTimeString( tsLocal, tsLocalLen, sDateTime );
		}
		else
		{
			GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDateTime);
		}
	}
	else
	{
		GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDateTime);
	}

	if(m_UseTickler)
	{
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(m_ReOpenTime,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sReopenTime );
				m_ReOpenTime = tsLocal;
			}
			else
			{
				GetDateTimeString( m_ReOpenTime, m_ReOpenTimeLen, sReopenTime );
			}
		}
		else
		{
			GetDateTimeString( m_ReOpenTime, m_ReOpenTimeLen, sReopenTime );
		}
	}
		
	GetXMLGen().AddChildElem( _T("Ticket") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_TicketID );

	GetXMLGen().AddChildAttrib( _T("TicketBoxID"), m_TicketBoxID );
	AddTicketBoxName( _T("TicketBox"), m_TicketBoxID );

	GetXMLGen().AddChildAttrib( _T("PriorityID"), m_PriorityID );
	AddPriority( _T("PriorityName"), m_PriorityID );

	if(m_TicketStateID == EMS_TICKETSTATEID_REOPENED)
		m_TicketStateID = EMS_TICKETSTATEID_OPEN;

	GetXMLGen().AddChildAttrib( _T("StateID"), m_TicketStateID );
	AddTicketStateName( _T("StateName"), m_TicketStateID );
	
	GetXMLGen().AddChildAttrib( _T("OwnerID"), m_OwnerID );
	AddAgentName( _T("Owner"), m_OwnerID );
	
	GetXMLGen().AddChildAttrib( _T("DateCreated"), sDateTime.c_str() );
	GetXMLGen().AddChildAttrib( _T("OpenMins"), m_OpenMins );

	GetXMLGen().AddChildAttrib( _T("LockedBy"), m_LockedBy == GetAgentID() ? 0 : m_LockedBy );
	AddAgentName( _T("LockedByName"), m_LockedBy == GetAgentID() ? 0 : m_LockedBy); 
	
	GetXMLGen().AddChildAttrib( _T("AccessLevel"), m_Access );

	GetXMLGen().AddChildAttrib( _T("Subject"), m_SubjectLen ? m_Subject : _T("[No Subject]") );
	GetXMLGen().AddChildAttrib( _T("Contact"), m_ContactsLen ? m_Contacts : _T("[No Contacts]") );

	GetXMLGen().AddChildAttrib( _T("FolderID"), m_FolderID );

	GetXMLGen().AddChildAttrib( _T("usetickler"), m_UseTickler );
	GetXMLGen().AddChildAttrib( _T("reopentime"), sReopenTime.size() ? sReopenTime.c_str() : "NULL");
	if(m_UseTickler)
	{
		int nOpenAmPm = 1;
		if(m_ReOpenTime.hour > 11)
		{
			nOpenAmPm = 2;
			m_ReOpenTime.hour = m_ReOpenTime.hour - 12;
		}
		GetXMLGen().AddChildAttrib( _T("OpenHr"), m_ReOpenTime.hour );
		GetXMLGen().AddChildAttrib( _T("OpenMin"), m_ReOpenTime.minute );
		GetXMLGen().AddChildAttrib( _T("OpenAmPm"), nOpenAmPm );
	}

	GetXMLGen().AddChildAttrib( _T("TicketCategoryID"), m_TicketCategoryID );
	AddTicketCategoryName( _T("TicketCategoryName"), m_TicketCategoryID );	
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query ticket information from the database	              
\*--------------------------------------------------------------------------*/
void CTicket::Query( void )
{
	if ( m_TicketID < 1 )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("A Ticket ID must be specified") );

	if ( TTickets::Query( GetQuery() ) != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("Unable to query ticket (%d)"), m_TicketID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
	
	// check security - read access is required
	m_Access = RequireRead(false);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Inserts a new ticket.
\*--------------------------------------------------------------------------*/
int CTicket::Insert( void )
{
	// check security
	if ( m_TicketBoxID > 0 )
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, m_TicketBoxID, EMS_EDIT_ACCESS );
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_CATEGORY, m_TicketCategoryID, EMS_EDIT_ACCESS );
	
	if ( m_OwnerID > 0)
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_OwnerID, EMS_ENUM_ACCESS );

	// insert the ticket
	TTickets::Insert( GetQuery() );

	return m_TicketID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a new ticket ID.  If the agent has a reserved 
||				TicketID (from a canceled composition) it will be used.
||				Supports the scenario where the same user may have multiple
||				concurrent sessions.	              
\*--------------------------------------------------------------------------*/
int CTicket::ReserveID(void)
{
	// insert a new ticket
	m_LockedBy = GetAgentID();
	m_OwnerID  = GetAgentID();
	m_IsDeleted = EMS_DELETE_OPTION_RESERVED;
	
	// insert the ticket
	TTickets::m_TicketCategoryID = 1;
	TTickets::Insert( GetQuery() );
	
	Lock(false);

	return m_TicketID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes the ticket	              
\*--------------------------------------------------------------------------*/
void CTicket::Delete( bool bCheckSecurity  )
{	
	try
	{
		// do everything inside a transaction
		GetQuery().Initialize();
		GetQuery().Execute( _T("BEGIN TRANSACTION") );	

		// check security
		if ( bCheckSecurity )
			RequireDelete();

		//Get the current Ticket Data
		TTickets ticket;	
		ticket.m_TicketID = m_TicketID;
		if ( ticket.Query( GetQuery() ) != S_OK)
		{
			CEMSString sError;
			sError.Format( _T("Unable to query ticket (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// lock the ticket
		Lock(false);

		// get the current time
		TIMESTAMP_STRUCT time;
		GetTimeStamp(time);

		// delete inbound messages that are part of the ticket
		GetQuery().Initialize();
		
		BINDPARAM_TIME_NOLEN( GetQuery(), time );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("UPDATE InboundMessages SET IsDeleted=1,DeletedTime=?,DeletedBy=? WHERE TicketID=?"));
		
		// delete outbound messages
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE OutboundMessages SET IsDeleted=1,DeletedTime=?,DeletedBy=? WHERE TicketID=?"));
		
		// delete entries in the outbound message queue
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), m_TicketID );
		GetQuery().Execute( _T("DELETE FROM OutboundMessageQueue WHERE OutboundMessageID IN ")
							_T("(SELECT OutboundMessageID FROM OutboundMessages WHERE TicketID = ?)"));

		// remove any entries in the approvals queue
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM Approvals WHERE ApprovalObjectTypeID=1 AND ActualID IN (SELECT OutboundMessageID FROM OutboundMessages WHERE TicketID = ?)") );
		
		// delete the ticket
		GetQuery().Initialize();
		
		BINDPARAM_TIME_NOLEN( GetQuery(), time );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("UPDATE Tickets SET TicketStateID=5,IsDeleted=1,LockedBy=0,DeletedTime=?,DeletedBy=? WHERE TicketID=?") );		
		
		m_IsDeleted = EMS_DELETE_OPTION_DELETE_TO_WASTE_BASKET;
		m_DeletedTime = time;
		m_DeletedBy = GetAgentID();
		m_LockedBy = 0;
		m_nLockCount = 0;
	
		// update the ticket history
		THDeleteTicket( GetQuery(), m_TicketID, GetAgentID(), ticket.m_TicketStateID, ticket.m_TicketBoxID, ticket.m_OwnerID, ticket.m_PriorityID, ticket.m_TicketCategoryID );
		
		//Log it to Agent Activity
		if( _ttoi( sAal.c_str() ) > 0 )
		{
			if( _ttoi( sTd.c_str() ) > 0 )
			{
				LogAgentAction(GetAgentID(),3,m_TicketID,0,0,_T(""),_T(""));
			}
		}

		// we are done, commit the transaction
		GetQuery().Initialize();
		GetQuery().Execute( _T("COMMIT TRANSACTION") );
	}
	catch ( CEMSException Exception )
	{
		GetQuery().Initialize();
		GetQuery().Execute( _T("ROLLBACK TRANSACTION") );

		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to delete ticket (%s):\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Reassigns the ticket to the specified agent	              
\*--------------------------------------------------------------------------*/
void CTicket::Reassign( int nAgentID )
{
	int nTicketCategoryID;
	UINT nRequireAgentEdit=0;
	
	try
	{
		// sanity check
		if ( m_TicketID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("A Ticket ID must be specified") );
		
		//Get the current Ticket Data
		TTickets ticket;	
		ticket.m_TicketID = m_TicketID;
		if ( ticket.Query( GetQuery() ) != S_OK)
		{
			CEMSString sError;
			sError.Format( _T("Unable to query ticket (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
				
		m_OwnerID = ticket.m_OwnerID;
		nTicketCategoryID = ticket.m_TicketCategoryID;
		GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("RequireAgentEdit"), nRequireAgentEdit );
		
		if ( m_OwnerID != nAgentID )
		{
			if ( nRequireAgentEdit )
			{
				RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, nAgentID, EMS_EDIT_ACCESS );			
			}
			else
			{
				RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, nAgentID, EMS_ENUM_ACCESS );			
			}
			
			RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_CATEGORY, nTicketCategoryID, EMS_EDIT_ACCESS );
			
			unsigned char RightLevel;
			RightLevel = CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), nAgentID, EMS_OBJECT_TYPE_TICKET_CATEGORY, nTicketCategoryID );
			if ( RightLevel < 2 )
			{
				CEMSString sError;
				sError.Format( _T("The Agent you are assigning this ticket to does not have Edit rights to the Ticket Category!"));
				THROW_EMS_EXCEPTION( E_InvalidID, sError );
			}

			bool bSkipStatus = false;
			if((GetIsAdmin() && _ttoi( sAllowAdmin.c_str() ) > 0) || (!GetIsAdmin() && (nAgentID == GetAgentID()) && _ttoi( sAllowAgent.c_str() ) > 0) || nAgentID == 0)
				bSkipStatus = true;

			if(!bSkipStatus)
			{
				// Get the target Agent Status
				tstring sStatus;
				int nLoggedIn;
				int nStatus = m_ISAPIData.m_SessionMap.GetAgentStatus(nAgentID,sStatus,nLoggedIn);
				switch( nStatus )
				{
				case 1:
					if( _ttoi( sDenyOnline.c_str() ) > 0 )
					{
						CEMSString sError;
						sError.Format( _T("The Agent you are assigning this ticket to is Online!"));
						THROW_EMS_EXCEPTION( E_EMSException, sError );
					}
					break;
					
				case 2:
					if( _ttoi( sDenyAway.c_str() ) > 0 )
					{
						CEMSString sError;
						sError.Format( _T("The Agent you are assigning this ticket to is Away!"));
						THROW_EMS_EXCEPTION( E_EMSException, sError );
					}
					break;
					
				case 3:
					if( _ttoi( sDenyNotAvail.c_str() ) > 0 )
					{
						CEMSString sError;
						sError.Format( _T("The Agent you are assigning this ticket to is Not Available!"));
						THROW_EMS_EXCEPTION( E_EMSException, sError );
					}
					break;
					
				case 5:
					if( _ttoi( sDenyOffline.c_str() ) > 0 )
					{
						CEMSString sError;
						sError.Format( _T("The Agent you are assigning this ticket to is Offline!"));
						THROW_EMS_EXCEPTION( E_EMSException, sError );
					}
					break;

				case 6:
					if( _ttoi( sDenyOoo.c_str() ) > 0 )
					{
						CEMSString sError;
						sError.Format( _T("The Agent you are assigning this ticket to is Out of Office!"));
						THROW_EMS_EXCEPTION( E_EMSException, sError );
					}
					break;
				}
			}
			// lock the ticket
			//Lock( true );
		
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), nAgentID );
			BINDPARAM_LONG( GetQuery(), m_TicketID );
			GetQuery().Execute( _T("UPDATE Tickets SET OwnerID=? WHERE TicketID=?") );
			
			if ( GetQuery().GetRowCount() != 1 )
			{
				CEMSString sError;
				sError.Format( _T("Ticket (%d) no longer exists"), m_TicketID );
				THROW_EMS_EXCEPTION( E_InvalidID, sError );
			}

			// update the ticket history
			THChangeOwner( GetQuery(), m_TicketID, GetAgentID(), m_OwnerID, ticket.m_TicketStateID, ticket.m_TicketBoxID, nAgentID, ticket.m_PriorityID, ticket.m_TicketCategoryID );

			//Log it to Agent Activity
			if( _ttoi( sAal.c_str() ) > 0 )
			{
				if( _ttoi( sTr.c_str() ) > 0 )
				{
					LogAgentAction(GetAgentID(),7,m_TicketID,m_OwnerID,nAgentID,_T(""),_T(""));
				}
			}

			if(nAgentID != GetAgentID() )
			{
				//Check for Agent Alerts
				CheckAgentAlerts(GetAgentID(), nAgentID, ticket.m_Subject, ticket.m_SubjectLen );
			}
			

			// update the class member
			m_OwnerID = nAgentID;

			// unlock the ticket
			//UnLock();
		}
	}
	catch ( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to reassign ticket (%s):\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

void CTicket::CheckAgentAlerts( int nOwnerID, int nAgentID, TCHAR szSubject[TICKETS_SUBJECT_LENGTH], long szSubjectLen  )
{
	TAlerts alert;
	std::list<TAlerts> aeidList;
	std::list<TAlerts>::iterator aeidIter;
	bool bAlertAssign=false;
	dca::String t;
	CEMSString sSubject;
		
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nAgentID );
	BINDCOL_LONG( GetQuery(), alert.m_AlertID );
	BINDCOL_LONG( GetQuery(), alert.m_AlertEventID );
	BINDCOL_LONG( GetQuery(), alert.m_AlertMethodID );
	BINDCOL_TCHAR( GetQuery(), alert.m_EmailAddress );
	GetQuery().Execute( _T("SELECT AlertID, AlertEventID, AlertMethodID, EmailAddress FROM Alerts WHERE AgentID=? AND AlertEventID=14 ") );
	while(GetQuery().Fetch() == S_OK)
	{
		/*dca::String t2(alert.m_EmailAddress);
		t.Format("CTicket::CheckAgentAlerts - Agent [%d] configured for AlertEventID [%d] AlertMethodID [%d] EmailAddress [%s]", nOwnerID, alert.m_AlertEventID,alert.m_AlertMethodID,t2.c_str() );
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ISAPI);*/
		aeidList.push_back(alert);
	}

	sSubject = szSubjectLen ? szSubject : _T("[No Subject]");
	for(aeidIter = aeidList.begin(); aeidIter != aeidList.end(); aeidIter++)
	{
		CEMSString strAlertBody;
		tstring sName;
		GetAgentName(nOwnerID, sName);
		strAlertBody.Format( _T("Ticket %d has been assigned to you by Agent %s|||A Ticket has been assigned to you by Agent %s.\n\nTicket #%d %s\r\n"), m_TicketID, sName.c_str(), sName.c_str(), m_TicketID, sSubject.c_str() );						
		SendTicketAssignAlert( aeidIter->m_AlertID, strAlertBody.c_str() );				
	}
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Updates the ticket to the specified priority
\*--------------------------------------------------------------------------*/
void CTicket::ChangePriority( int nPriorityID )
{
	try
	{
		// sanity check
		if ( m_TicketID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("A Ticket ID must be specified") );

		//Get the current Ticket Data
		TTickets ticket;	
		ticket.m_TicketID = m_TicketID;
		if ( ticket.Query( GetQuery() ) != S_OK)
		{
			CEMSString sError;
			sError.Format( _T("Unable to query ticket (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
				
		m_PriorityID = ticket.m_PriorityID;
		
		// get the current PriorityID
		/*GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), m_PriorityID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		GetQuery().Execute( _T("SELECT PriorityID FROM Tickets WHERE TicketID = ?" ));
		
		if ( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("Invalid Ticket ID") );
		*/
		
		if ( m_PriorityID != nPriorityID )
		{	
			// lock the ticket
			//Lock(true);
		
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), nPriorityID );
			BINDPARAM_LONG( GetQuery(), m_TicketID );
			GetQuery().Execute( _T("UPDATE Tickets SET PriorityID=? WHERE TicketID=?") );
			
			if ( GetQuery().GetRowCount() != 1 )
			{
				CEMSString sError;
				sError.Format( _T("Ticket (%d) no longer exists"), m_TicketID );
				THROW_EMS_EXCEPTION( E_InvalidID, sError );
			}

			// update the ticket history
			THChangePriority( GetQuery(), m_TicketID, GetAgentID(), m_PriorityID, ticket.m_TicketStateID, ticket.m_TicketBoxID, ticket.m_OwnerID, nPriorityID, ticket.m_TicketCategoryID );

			// update the class member
			m_PriorityID = nPriorityID;

			// unlock the ticket
			//UnLock();
		}
	}
	catch ( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to change the priority of ticket (%s):\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Moves the ticket to the specified TicketBox
\*--------------------------------------------------------------------------*/
void CTicket::ChangeTicketBox( int nTicketBoxID )
{
	unsigned char fromAccess;
	unsigned char toAccess;
	try
	{
		// sanity check
		if ( m_TicketID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("A Ticket ID must be specified") );

		//Get the current Ticket Data
		TTickets ticket;	
		ticket.m_TicketID = m_TicketID;
		if ( ticket.Query( GetQuery() ) != S_OK)
		{
			CEMSString sError;
			sError.Format( _T("Unable to query ticket (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
		
		m_TicketBoxID = ticket.m_TicketBoxID;

		//// get the current TicketBox
		//GetQuery().Initialize();
		//BINDCOL_LONG( GetQuery(), m_TicketBoxID );
		//BINDPARAM_LONG( GetQuery(), m_TicketID );
		//GetQuery().Execute( _T("SELECT TicketBoxID FROM Tickets WHERE TicketID = ?" ));
		//
		//if ( GetQuery().Fetch() != S_OK )
		//	THROW_EMS_EXCEPTION( E_InvalidID, _T("Invalid Ticket ID") );
		
		if ( nTicketBoxID != m_TicketBoxID )
		{		
			// check security
			RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, m_TicketBoxID, EMS_EDIT_ACCESS );
			RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, nTicketBoxID, EMS_ENUM_ACCESS );

			fromAccess = GetAgentRightLevel(EMS_OBJECT_TYPE_TICKET_BOX,m_TicketBoxID);
			toAccess = GetAgentRightLevel(EMS_OBJECT_TYPE_TICKET_BOX,nTicketBoxID);

			if ( fromAccess < toAccess )
			{
				CEMSString sError;
				sError.Format( _T("Insufficient rights to the source or destination TicketBox!") );
				THROW_EMS_EXCEPTION( E_InvalidID, sError );
			}


			// lock the ticket
			//Lock(false);
		
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), nTicketBoxID );
			BINDPARAM_LONG( GetQuery(), m_TicketID );
			GetQuery().Execute( _T("UPDATE Tickets SET TicketBoxID=? WHERE TicketID=?") );
			
			if ( GetQuery().GetRowCount() != 1 )
			{
				CEMSString sError;
				sError.Format( _T("The Ticket (%d) no longer exists"), m_TicketID );
				THROW_EMS_EXCEPTION( E_InvalidID, sError );
			}

			// update the ticket history
			THChangeTicketBox( GetQuery(), m_TicketID, GetAgentID(), m_TicketBoxID, ticket.m_TicketStateID, nTicketBoxID, ticket.m_OwnerID, ticket.m_PriorityID, ticket.m_TicketCategoryID );
			
			//Log it to Agent Activity
			if( _ttoi( sAal.c_str() ) > 0 )
			{
				if( _ttoi( sTm.c_str() ) > 0 )
				{
					LogAgentAction(GetAgentID(),5,m_TicketID,m_TicketBoxID,nTicketBoxID,_T(""),_T(""));
				}
			}

			// update the class member
			m_TicketBoxID = nTicketBoxID;

			// unlock the ticket
			//UnLock();
		}
	}
	catch ( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to move ticket (%s):\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Commits the ticket to the database	              
\*--------------------------------------------------------------------------*/
void CTicket::Commit( void )
{
	GetQuery().Initialize();
	
	BINDPARAM_LONG( GetQuery(), m_TicketID );
	
	GetQuery().Execute( _T("UPDATE Tickets SET IsDeleted = 0 WHERE TicketID = ?") );
	
	if (GetQuery().GetRowCount() != 1)
	{
		CEMSString sError;
		sError.Format( _T("The TicketID (%d) was invalid"), m_TicketID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Locks the Ticket	              
\*--------------------------------------------------------------------------*/
void CTicket::Lock( bool bCheckSecurity /* = false */ )
{
	GetTimeStamp(m_LockedTime);

	// only lock the ticket the first time...
	// otherwise just increment the count
	if ( !m_nLockCount )
	{
		// check security
		if ( bCheckSecurity )
			RequireEdit();
		
		// attempt to lock the ticket
		GetQuery().Initialize();
		BINDPARAM_TIME_NOLEN( GetQuery(), m_LockedTime );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		GetQuery().Execute( _T("UPDATE Tickets SET LockedTime=?,LockedBy=? WHERE TicketID=? AND LockedBy IN (0,?)") );

		// if the ticket is already locked or doesn't exist
		if ( GetQuery().GetRowCount() != 1)
		{
			TCHAR szSubject[TICKETS_SUBJECT_LENGTH] = {0};
			int nLockedBy = 0;

			GetQuery().Initialize();
			BINDCOL_TCHAR_NOLEN( GetQuery(), szSubject );
			BINDCOL_LONG_NOLEN( GetQuery(), nLockedBy );
			BINDPARAM_LONG( GetQuery(), m_TicketID );
			GetQuery().Execute( _T("SELECT Subject, LockedBy FROM Tickets WHERE TicketID=?") );

			if ( GetQuery().Fetch() == S_OK )
			{
				XAgentNames AgentName;

				if (!GetXMLCache().m_AgentNames.Query( nLockedBy, AgentName ))
					_tcscpy( AgentName.m_Name, _T("Unknown") );
			
				CEMSString sError;
				sError.Format( _T("An agent [%s] is currently modifying ticket [%s]. ")
					           _T("To prevent potential data loss, you are being prevented ")
							   _T("from modifying this ticket until the agent is finished."), 
							   AgentName.m_Name, szSubject );

				THROW_EMS_EXCEPTION_NOLOG( E_TicketLocked, sError );
			}
			else
			{
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( _T("The ticket no longer exists") ) );
			}
		}

		// update that the ticket has been locked
		m_LockedBy = m_ISAPIData.m_pSession->m_AgentID;		
	}
	// increment lock reference count
	m_nLockCount++;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Unlocks the Ticket.  The ticket will only be unlocked
||				if the ticket was locked by the agent.	              
\*--------------------------------------------------------------------------*/
void CTicket::UnLock( bool bForce /*=false*/ )
{
	// do we need to unlock the ticket
	
	dca::String t;
	t.Format("CTicket::Unlock - TicketID [%d] AgentID [%d] LockCount [%d] DoNotUnlock [%d] Force [%d]", m_TicketID, GetAgentID(), m_nLockCount, m_bDoNotUnlock, bForce );
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ISAPI);
	
	if ( ( m_nLockCount == 1 && !m_bDoNotUnlock) || bForce )
	{
		int nReservedMsgCount;
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nReservedMsgCount );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		GetQuery().Execute( _T("SELECT COUNT(*) FROM OutboundMessages WHERE IsDeleted=3 AND TicketID=?") );
		GetQuery().Fetch();

		if ( nReservedMsgCount > 0 )
		{
			return;  
		}

		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );

		GetQuery().Execute( _T("UPDATE Tickets SET LockedBy=0 WHERE TicketID=? AND LockedBy IN (0,?)") );

		if ( GetQuery().GetRowCount() == 1 )
		{
			m_nLockCount = 0;
			m_LockedBy = 0;
			return; //true
		}
	}
	else if ( m_nLockCount > 1 )
	{
		m_nLockCount--;
	}
}

int CTicket::AdminUnLock()
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketID );
	
	GetQuery().Execute( _T("UPDATE Tickets SET LockedBy=0 WHERE TicketID=?") );

	if ( GetQuery().GetRowCount() == 1 )
	{
		m_nLockCount = 0;
		m_LockedBy = 0;
		return 0;
	}

	return 1;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Sets the ticket to the specified state	              
\*--------------------------------------------------------------------------*/
void CTicket::SetState( int nTicketStateID )
{
	switch( nTicketStateID )
	{
	case EMS_TICKETSTATEID_CLOSED:
		Close();
		break;
		
	case EMS_TICKETSTATEID_OPEN:
		Open();
		break;
		
	case EMS_TICKETSTATEID_ESCALATED:
		Escalate();
		break;
		
	case EMS_TICKETSTATEID_ONHOLD:
		PlaceOnHold();
		break;

	case EMS_TICKETSTATEID_WAITING:
		PlaceOnWait();
		break;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Updates the ticket to the specified category
\*--------------------------------------------------------------------------*/
void CTicket::SetTicketCategory( int nTicketCategoryID )
{

	try
	{
		// sanity check
		if ( m_TicketID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("A Ticket ID must be specified") );

		//Get the current Ticket Data
		TTickets ticket;	
		ticket.m_TicketID = m_TicketID;
		if ( ticket.Query( GetQuery() ) != S_OK)
		{
			CEMSString sError;
			sError.Format( _T("Unable to query ticket (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
				
		m_TicketCategoryID = ticket.m_TicketCategoryID;
		
		if ( m_TicketCategoryID != nTicketCategoryID )
		{	
			// check security
			RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_CATEGORY, m_TicketCategoryID, EMS_EDIT_ACCESS );
			RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_CATEGORY, nTicketCategoryID, EMS_EDIT_ACCESS );
			
			// lock the ticket
			//Lock(true);
		
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), nTicketCategoryID );
			BINDPARAM_LONG( GetQuery(), m_TicketID );
			GetQuery().Execute( _T("UPDATE Tickets SET TicketCategoryID=? WHERE TicketID=?") );
			
			if ( GetQuery().GetRowCount() != 1 )
			{
				CEMSString sError;
				sError.Format( _T("Ticket (%d) no longer exists"), m_TicketID );
				THROW_EMS_EXCEPTION( E_InvalidID, sError );
			}

			// update the ticket history
			THChangeCategory( GetQuery(), m_TicketID, GetAgentID(), m_TicketCategoryID, ticket.m_TicketStateID, ticket.m_TicketBoxID, ticket.m_OwnerID, ticket.m_PriorityID, nTicketCategoryID );

			//Log it to Agent Activity
			if( _ttoi( sAal.c_str() ) > 0 )
			{
				if( _ttoi( sTcc.c_str() ) > 0 )
				{
					LogAgentAction(GetAgentID(),30,m_TicketID,m_TicketCategoryID,nTicketCategoryID,_T(""),_T(""));
				}
			}

			// update the class member
			m_TicketCategoryID = nTicketCategoryID;

			// unlock the ticket
			//UnLock();
		}
	}
	catch ( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to change the category of ticket (%s):\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Sets the ticket to the specified state	              
\*--------------------------------------------------------------------------*/
void CTicket::SetTickler()
{
	try
	{
		// sanity check
		if ( m_TicketID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("A Ticket ID must be specified") );

		// lock the ticket
		//Lock(true);
		
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_UseTickler );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_ReOpenTime );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		GetQuery().Execute( _T("UPDATE Tickets SET UseTickler = ?, TicklerDateToReopen = ? WHERE TicketID=?") );
			
		if ( GetQuery().GetRowCount() != 1 )
		{
			CEMSString sError;
			sError.Format( _T("Ticket (%d) no longer exists"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// unlock the ticket
		//UnLock();
	}
	catch ( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to set date to reopen for ticket (%s):\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

void CTicket::UnSetTickler()
{
	try
	{
		// sanity check
		if ( m_TicketID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("A Ticket ID must be specified") );

		// lock the ticket
		//Lock(true);
		
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		GetQuery().Execute( _T("UPDATE Tickets SET UseTickler = 0, TicklerDateToReopen = NULL WHERE TicketID=?") );
			
		if ( GetQuery().GetRowCount() != 1 )
		{
			CEMSString sError;
			sError.Format( _T("Ticket (%d) no longer exists"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// unlock the ticket
		//UnLock();
	}
	catch ( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to set date to reopen for ticket (%s):\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Open Ticket	              
\*--------------------------------------------------------------------------*/
void CTicket::Open( bool bLog /*=true*/ )
{
	try
	{
		int nOldTicketStateID;

		// get the current TicketStateID
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nOldTicketStateID );
		BINDCOL_LONG( GetQuery(), m_TicketBoxID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("SELECT TicketStateID, TicketBoxID FROM Tickets WHERE TicketID = ?" ));
		
		if ( GetQuery().Fetch() != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
		
		// if the ticket is already open, bail out
		if (nOldTicketStateID == EMS_TICKETSTATEID_OPEN)
			return;

		// lock the ticket
		//Lock(true);

		// update the ticket
		m_TicketStateID = EMS_TICKETSTATEID_OPEN;
		
		GetQuery().Initialize();
		
		if ( nOldTicketStateID == EMS_TICKETSTATEID_ESCALATED)
		{
			// if the ticket was escalated, we don't need to reset the
			// open timestamp
			BINDPARAM_LONG( GetQuery(), m_TicketStateID );
			BINDPARAM_LONG( GetQuery(), m_TicketID );
			GetQuery().Execute( _T("UPDATE Tickets SET TicketStateID = ? WHERE TicketID = ?") );
		}
		else
		{
			TIMESTAMP_STRUCT CurrentTime;
			GetTimeStamp(CurrentTime);
			BINDPARAM_LONG( GetQuery(), m_TicketStateID );
			BINDPARAM_TIME_NOLEN( GetQuery(), CurrentTime );
			BINDPARAM_LONG( GetQuery(), m_TicketID );
			GetQuery().Execute( _T("UPDATE Tickets SET TicketStateID = ?, OpenTimestamp = ? WHERE TicketID = ?") );
		}
			
		if ( GetQuery().GetRowCount() != 1 )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// log the change in the ticket history
		if ( bLog )
			THChangeState( GetQuery(), m_TicketID, GetSession().m_AgentID, nOldTicketStateID, m_TicketStateID, m_TicketBoxID, m_OwnerID, m_PriorityID, m_TicketCategoryID );

		//Log it to Agent Activity
		if( _ttoi( sAal.c_str() ) > 0 )
		{
			if( _ttoi( sTo.c_str() ) > 0 )
			{
				LogAgentAction(GetAgentID(),9,m_TicketID,nOldTicketStateID,0,_T(""),_T(""));
			}
		}
		
		// unlock the ticket
		//UnLock();
	}
	catch( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to open ticket [%s]:\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Close Ticket	              
\*--------------------------------------------------------------------------*/
void CTicket::Close( void )
{
	try
	{
		TIMESTAMP_STRUCT CurrentTime;
		int nOldTicketStateID;

		// check security
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nOldTicketStateID );
		BINDCOL_LONG_NOLEN( GetQuery(), m_OpenMins );
		BINDCOL_TIME( GetQuery(), m_OpenTimestamp );
		BINDCOL_LONG( GetQuery(), m_TicketBoxID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("SELECT TicketStateID, OpenMins, OpenTimestamp, TicketBoxID FROM Tickets WHERE TicketID = ?" ));
		
		if ( GetQuery().Fetch() != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
		
		// if the ticket is already closed, bail out
		if (nOldTicketStateID == EMS_TICKETSTATEID_CLOSED)
			return;

		// lock the ticket
		//Lock(true);
		
		// if the ticket wasn't "on hold" calculate the minutes
		// the ticket was open...
		if ( nOldTicketStateID != EMS_TICKETSTATEID_ONHOLD )
		{
			GetTimeStamp(CurrentTime);
			
			int nLastOpenSecs = m_OpenTimestampLen != SQL_NULL_DATA ? 
				                CompareTimeStamps( m_OpenTimestamp, CurrentTime ) : 0;
			
			if (nLastOpenSecs > 0)
			{
				// update the open minute count
				m_OpenMins += (nLastOpenSecs / 60);

				// round to the next minute?
				if ( (nLastOpenSecs % 60) >= 30 )
					m_OpenMins++;
			}
		}

		// update the ticket
		m_TicketStateID = EMS_TICKETSTATEID_CLOSED;
		
		//Remove any custom age alert records, if they exist
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketID );		
		GetQuery().Execute( _T("DELETE FROM AgeAlertsSent WHERE TicketID = ?") );
		
		GetQuery().Initialize();
		
		BINDPARAM_LONG( GetQuery(), m_TicketStateID );
		BINDPARAM_LONG( GetQuery(), m_OpenMins );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("UPDATE Tickets SET AgeAlerted=0, TicketStateID = ?, OpenMins = ? WHERE TicketID = ?") );
		
		if ( GetQuery().GetRowCount() != 1 )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// log the change in the ticket history
		THChangeState( GetQuery(), m_TicketID, GetSession().m_AgentID, nOldTicketStateID, m_TicketStateID, m_TicketBoxID, m_OwnerID, m_PriorityID, m_TicketCategoryID );

		//Log it to Agent Activity
		if( _ttoi( sAal.c_str() ) > 0 )
		{
			if( _ttoi( sTcl.c_str() ) > 0 )
			{
				LogAgentAction(GetAgentID(),10,m_TicketID,nOldTicketStateID,0,_T(""),_T(""));
			}
		}

		// unlock the ticket
		//UnLock();
	}
	catch( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to close ticket [%s]:\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Escalate Ticket	              
\*--------------------------------------------------------------------------*/
void CTicket::Escalate( void )
{
	try
	{
		int nOldStateID;
		int nOldOwnerID;

		set<long> EscalateAgentIDs;
		CEMSString strSubject;
		CEMSString strAlertSubject;
		CEMSString strAlertBody;
		dca::WString strEmptyString = "";
		CEMSString strAgentName;
		CEMSString sName;

		GetAgentName( GetSession().m_AgentID, sName );
		strAgentName.Format( _T("%s"), sName.c_str());

		list<int> AgentIDs;
		list<int>::iterator iter;
		int AgentID;
		int iTen = 10;
		int nEscalateToTicketBoxID;
		int nUseEscTicketBox;
	
		// TODO - don't escalate to deleted or inactive agents!!!
		
		// sanity check
		if ( GetSession().m_EscalateToAgentID == 0 )
		{
			CEMSString sError;
			sError.Format( _T("There is no agent specified to esclate this ticket to") );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
			
		GetQuery().Initialize();
		
		BINDCOL_LONG_NOLEN( GetQuery(), nOldStateID );
		BINDCOL_LONG_NOLEN( GetQuery(), nOldOwnerID );
		BINDCOL_LONG( GetQuery(), m_TicketBoxID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("SELECT TicketStateID, OwnerID, TicketBoxID FROM Tickets WHERE TicketID = ?" ) );
		
		if ( GetQuery().Fetch() != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
		
		if (nOldOwnerID == GetSession().m_EscalateToAgentID)
			return;
		
		// lock the ticket
		//Lock(true);

		// if the ticket is closed, open the ticket
		if (nOldStateID == EMS_TICKETSTATEID_CLOSED)
			Open(false);

		GetSubjectForTicket( strSubject );

		if ( GetSession().m_EscalateToAgentID > 0 )
		{
			//Escalating to an Agent
			
			AgentIDs.push_back( GetSession().m_EscalateToAgentID );

			
			strAlertSubject.Format( _T("Ticket [ %d ] Escalated by %s :: %s"), m_TicketID, strAgentName.c_str(), strSubject.c_str() );
			strAlertBody.Format( _T("Ticket Escalated\n\nTicket #%d :: %s\r\n"), m_TicketID, strSubject.c_str() );
		}
		else
		{
			//Escalating to an Escalation Group
			// Get agentids for group
			int iGroupID = abs( GetSession().m_EscalateToAgentID );
	
			TCHAR szGroupName[TICKETS_SUBJECT_LENGTH] = {0};
			long szGroupNameLen = 0;
			tstring strGroupName;
			
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), iGroupID );
			BINDCOL_LONG_NOLEN( GetQuery(), AgentID );
			BINDCOL_TCHAR_NOLEN( GetQuery(), szGroupName );
			BINDCOL_LONG_NOLEN( GetQuery(), nEscalateToTicketBoxID );
			BINDCOL_LONG_NOLEN( GetQuery(), nUseEscTicketBox );
			GetQuery().Execute( _T("SELECT AgentGroupings.AgentID,Groups.GroupName,Groups.AssignToTicketBoxID,Groups.UseEscTicketBox ")
							_T("FROM AgentGroupings ")
							_T("INNER JOIN Groups ON AgentGroupings.GroupID = Groups.GroupID ")
							_T("WHERE AgentGroupings.GroupID=?") );

			while( GetQuery().Fetch() == S_OK )
			{
				AgentIDs.push_back( AgentID );
				strGroupName.assign( szGroupName );
			}

			strAlertSubject.Format( _T("Ticket [ %d ] Escalated by %s to Group [%s] :: %s"), m_TicketID, strAgentName.c_str(), strGroupName.c_str(), strSubject.c_str() );
			strAlertBody.Format( _T("Ticket Escalated to Group [%s]\n\nTicket #%d :: %s\r\n"), strGroupName.c_str(), m_TicketID, strSubject.c_str() );
		}
		
		// escalate the ticket
		m_TicketStateID = EMS_TICKETSTATEID_ESCALATED;
		
		if ( GetSession().m_EscalateToAgentID < 0 )
		{
			m_OwnerID = 0;
			if ( nUseEscTicketBox != 1 )
				nEscalateToTicketBoxID = m_TicketBoxID;
		}
		else
		{
			m_OwnerID = GetSession().m_EscalateToAgentID;
			nEscalateToTicketBoxID = m_TicketBoxID;
		}
		
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketStateID );
		BINDPARAM_LONG( GetQuery(), m_OwnerID );
		BINDPARAM_LONG( GetQuery(), nEscalateToTicketBoxID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("UPDATE TICKETS SET TicketStateID=?, OWNERID=?, TicketBoxID=? WHERE TicketID = ?") );
		
		if ( GetQuery().GetRowCount() != 1 )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
		
		
		//Send the Alert
		for( iter = AgentIDs.begin(); iter != AgentIDs.end(); iter++ )
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), iTen );
			BINDPARAM_LONG( GetQuery(), *iter );
			BINDPARAM_TCHAR_STRING( GetQuery(), strAlertSubject );
			BINDPARAM_TCHAR_STRING( GetQuery(), strAlertBody );
			BINDPARAM_LONG( GetQuery(), m_TicketID );
			GetQuery().Execute(_T("INSERT INTO AlertMsgs (AlertEventID, AgentID, Subject, Body, TicketID) VALUES (?, ?, ?, ?, ?)"));
		}
		
		// log the change in the ticket history
		THEscalate( GetQuery(), m_TicketID, GetSession().m_AgentID, nOldOwnerID, nOldStateID, m_TicketStateID, m_TicketBoxID, m_OwnerID, m_PriorityID, m_TicketCategoryID );
		
		//Log it to Agent Activity
			if( _ttoi( sAal.c_str() ) > 0 )
			{
				if( _ttoi( sTe.c_str() ) > 0 )
				{
					LogAgentAction(GetAgentID(),6,m_TicketID,0,0,_T(""),_T(""));
				}
			}
		// unlock the ticket
		//UnLock();
	}
	catch( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to escalate ticket [%s]:\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Place Ticket On Hold	              
\*--------------------------------------------------------------------------*/
void CTicket::PlaceOnHold(void)
{
	try
	{
		TIMESTAMP_STRUCT CurrentTime;
		int nOldTicketStateID;
		
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nOldTicketStateID );
		BINDCOL_LONG_NOLEN( GetQuery(), m_OpenMins );
		BINDCOL_TIME( GetQuery(), m_OpenTimestamp );
		BINDCOL_LONG( GetQuery(), m_TicketBoxID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("SELECT TicketStateID, OpenMins, OpenTimestamp, TicketBoxID FROM Tickets WHERE TicketID = ?" ));
		
		if ( GetQuery().Fetch() != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// if the ticket is already closed, bail out
		if (nOldTicketStateID == EMS_TICKETSTATEID_ONHOLD)
			return;
		
		// calculate open mins
		GetTimeStamp(CurrentTime);
		
		int nLastOpenSecs = m_OpenTimestampLen != SQL_NULL_DATA ? CompareTimeStamps( m_OpenTimestamp, CurrentTime ) : 0;
		
		if (nLastOpenSecs > 0)
		{
			// update the open minute count
			m_OpenMins += (nLastOpenSecs / 60);
			
			// round to the next minute?
			if ( (nLastOpenSecs % 60) >= 30 )
				m_OpenMins++;
		}
		
		// lock the ticket
		//Lock(true); 
		
		// update the ticket
		m_TicketStateID = EMS_TICKETSTATEID_ONHOLD;
		
		GetQuery().Initialize();
		
		BINDPARAM_LONG( GetQuery(), m_TicketStateID );
		BINDPARAM_LONG( GetQuery(), m_OpenMins );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("UPDATE Tickets SET TicketStateID = ?, OpenMins = ? WHERE TicketID = ?") );
		
		if ( GetQuery().GetRowCount() != 1 )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// log the change in the ticket history
		THChangeState( GetQuery(), m_TicketID, GetAgentID(), nOldTicketStateID, m_TicketStateID, m_TicketBoxID, m_OwnerID, m_PriorityID, m_TicketCategoryID );
		
		// unlock the ticket
		//UnLock();
	}
	catch( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to place ticket [%s] on hold :\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Place Ticket On Hold	              
\*--------------------------------------------------------------------------*/
void CTicket::PlaceOnWait(void)
{
	try
	{
		TIMESTAMP_STRUCT CurrentTime;
		int nOldTicketStateID;
		
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nOldTicketStateID );
		BINDCOL_LONG_NOLEN( GetQuery(), m_OpenMins );
		BINDCOL_TIME( GetQuery(), m_OpenTimestamp );
		BINDCOL_LONG( GetQuery(), m_TicketBoxID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("SELECT TicketStateID, OpenMins, OpenTimestamp, TicketBoxID FROM Tickets WHERE TicketID = ?" ));
		
		if ( GetQuery().Fetch() != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// if the ticket is already waiting, bail out
		if (nOldTicketStateID == EMS_TICKETSTATEID_WAITING)
			return;
		
		// calculate open mins
		GetTimeStamp(CurrentTime);
		
		int nLastOpenSecs = m_OpenTimestampLen != SQL_NULL_DATA ? CompareTimeStamps( m_OpenTimestamp, CurrentTime ) : 0;
		
		if (nLastOpenSecs > 0)
		{
			// update the open minute count
			m_OpenMins += (nLastOpenSecs / 60);
			
			// round to the next minute?
			if ( (nLastOpenSecs % 60) >= 30 )
				m_OpenMins++;
		}
		
		// lock the ticket
		//Lock(true); 
		
		// update the ticket
		m_TicketStateID = EMS_TICKETSTATEID_WAITING;
		
		GetQuery().Initialize();
		
		BINDPARAM_LONG( GetQuery(), m_TicketStateID );
		BINDPARAM_LONG( GetQuery(), m_OpenMins );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("UPDATE Tickets SET TicketStateID = ?, OpenMins = ? WHERE TicketID = ?") );
		
		if ( GetQuery().GetRowCount() != 1 )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// log the change in the ticket history
		THChangeState( GetQuery(), m_TicketID, GetAgentID(), nOldTicketStateID, m_TicketStateID, m_TicketBoxID, m_OwnerID, m_PriorityID, m_TicketCategoryID );
		
		// unlock the ticket
		//UnLock();
	}
	catch( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to place ticket [%s] on hold :\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Returns how many undeleted messages are in 
||				the ticket.	              
\*--------------------------------------------------------------------------*/
int CTicket::GetMsgCount( bool bCheckSecurity /*=false*/ )
{
	int nMsgCount;
	
	// check security
	if (bCheckSecurity)
		RequireRead();
	
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), nMsgCount );
	BINDPARAM_LONG( GetQuery(), m_TicketID );
	
	GetQuery().Execute(	_T("SELECT (SELECT COUNT(*) FROM InboundMessages WHERE IsDeleted=0 AND ")
		_T("TicketID = Tickets.TicketID) + ")
		_T("(SELECT COUNT(*) FROM OutboundMessages WHERE IsDeleted=0 AND ")
		_T("TicketID = Tickets.TicketID) ")
		_T("FROM Tickets ")
		_T("WHERE TicketID=? AND IsDeleted=0") );
	
	// fetch the record
	if ( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Ticket (%d) no longer exists"), m_TicketID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	return nMsgCount;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Get the current TicketBoxID and OwnerID for a ticket	              
\*--------------------------------------------------------------------------*/
void CTicket::GetTicketBoxAndOwner( int& nTicketBoxID, int& nOwnerID )
{
	// sanity check
	if ( m_TicketID < 1 )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("A valid Ticket ID must be specified") );
	
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID );
	BINDCOL_LONG_NOLEN( GetQuery(), nOwnerID );
	BINDPARAM_LONG( GetQuery(), m_TicketID );
	
	// we must query for the TicketBoxID and OwnerID of the ticket
	GetQuery().Execute( _T("SELECT TicketBoxID, OwnerID FROM Tickets WHERE TicketID = ?" ));
	
	if ( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Invalid TicketID (%d)"), m_TicketID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Requires that the agent has read rights for the 
||				specified ticket box and owner             
\*--------------------------------------------------------------------------*/
unsigned char CTicket::RequireRead( bool bQuery /*=true*/ )
{
	// admins have full control
	if ( GetIsAdmin() ) return EMS_DELETE_ACCESS;

	int nTicketBoxID;
	int nOwnerID;

	if ( bQuery )
	{
		GetTicketBoxAndOwner( nTicketBoxID, nOwnerID );
	}
	else
	{
		nTicketBoxID = m_TicketBoxID;
		nOwnerID = m_OwnerID;
	}

	if ( nOwnerID > 0)
	{
		unsigned char TicketBox;
		
		if (( TicketBox = GetAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, nTicketBoxID )) == EMS_DELETE_ACCESS )
			return EMS_DELETE_ACCESS;

		unsigned char Owner = GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, nOwnerID );
		unsigned Access = max( Owner, min( TicketBox, EMS_EDIT_ACCESS) );

		if ( Access < EMS_READ_ACCESS )
			LogSecurityViolation( EMS_OBJECT_TYPE_TICKET_BOX, nTicketBoxID, EMS_READ_ACCESS );
			
		return Access;
	}
	else 
	{
		return	RequireAgentRightLevel(EMS_OBJECT_TYPE_TICKET_BOX, nTicketBoxID, EMS_READ_ACCESS);
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Requires that the agent has edit rights for the 
||				specified ticket box or owner              
\*--------------------------------------------------------------------------*/
unsigned char CTicket::RequireEdit( bool bQuery /*=true*/ )
{
	// admins have full control
	if ( GetIsAdmin() ) return EMS_DELETE_ACCESS;
	
	int nTicketBoxID;
	int nOwnerID;
	
	if ( bQuery )
	{
		GetTicketBoxAndOwner( nTicketBoxID, nOwnerID );
	}
	else
	{
		nTicketBoxID = m_TicketBoxID;
		nOwnerID = m_OwnerID;
	}
	
	if ( nOwnerID > 0)
	{
		unsigned char TicketBox;
		
		if (( TicketBox = GetAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, nTicketBoxID )) == EMS_DELETE_ACCESS )
			return EMS_DELETE_ACCESS;
		
		unsigned char Owner = GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, nOwnerID );
		
		if ( max(Owner, TicketBox) < EMS_EDIT_ACCESS )
			LogSecurityViolation( EMS_OBJECT_TYPE_TICKET_BOX, nTicketBoxID, EMS_EDIT_ACCESS );
	
		return EMS_EDIT_ACCESS;
	}
	else 
	{
		return	RequireAgentRightLevel(EMS_OBJECT_TYPE_TICKET_BOX, nTicketBoxID, EMS_EDIT_ACCESS);
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Requires that the agent has delete rights for the 
||				specified ticket box	              
\*--------------------------------------------------------------------------*/
unsigned char CTicket::RequireDelete( bool bQuery /*=true*/ )
{
	// admins have full control
	if ( GetIsAdmin() ) return EMS_DELETE_ACCESS;

	int nTicketBoxID;
	int nOwnerID;
	
	if ( bQuery )
	{
		GetTicketBoxAndOwner( nTicketBoxID, nOwnerID );
	}
	else
	{
		nTicketBoxID = m_TicketBoxID;
		nOwnerID = m_OwnerID;
	}
	
	return RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, nTicketBoxID, EMS_DELETE_ACCESS );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the subject of the ticket for use in exception error
||				strings.  This function will not cause an exception
\*--------------------------------------------------------------------------*/
void CTicket::GetSubjectForException( CEMSException Exception, CEMSString& sSubject )
{
	try
	{
		if ( Exception.GetErrorCode() != E_InvalidID )
		{
			TCHAR szSubject[TICKETS_SUBJECT_LENGTH] = {0};
			long szSubjectLen = 0;

			// get the subject of the ticket
			GetQuery().Initialize();
			
			BINDCOL_TCHAR( GetQuery(), szSubject );
			BINDPARAM_LONG( GetQuery(), m_TicketID );

			GetQuery().Execute( _T("SELECT Subject FROM Tickets WHERE TicketID = ?") );
			
			if ( GetQuery().Fetch() == S_OK )
				sSubject = szSubjectLen ? szSubject : _T("[No Subject]");
		}
		else
		{
			sSubject.Format( _T("%d"), m_TicketID );
		}
	}
	catch(...) 
	{
		// just eat any exception
	}
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the subject of the ticket 
\*--------------------------------------------------------------------------*/
void CTicket::GetSubjectForTicket( CEMSString& sSubject )
{
	try
	{
		
		TCHAR szSubject[TICKETS_SUBJECT_LENGTH] = {0};
		long szSubjectLen = 0;

		// get the subject of the ticket
		GetQuery().Initialize();
		
		BINDCOL_TCHAR( GetQuery(), szSubject );
		BINDPARAM_LONG( GetQuery(), m_TicketID );

		GetQuery().Execute( _T("SELECT Subject FROM Tickets WHERE TicketID = ?") );
		
		if ( GetQuery().Fetch() == S_OK )
			sSubject = szSubjectLen ? szSubject : _T("[No Subject]");
		
	}
	catch(...) 
	{
		// just eat any exception
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetNoteCount - Return the number of ticketnotes
// 
////////////////////////////////////////////////////////////////////////////////
int CTicket::GetNoteCount(void)
{
	int nNotes = 0;

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketID );
	BINDCOL_LONG_NOLEN( GetQuery(), nNotes );
	GetQuery().Execute( _T("SELECT COUNT(*) FROM TicketNotes WHERE TicketID = ? ") );
	GetQuery().Fetch();

	return nNotes;
}

void CTicket::SetFolderID()
{
	try
	{
		// sanity check
		if ( m_TicketID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("A Ticket ID must be specified") );

		// lock the ticket
		//Lock(false);
		
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_FolderID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		GetQuery().Execute( _T("UPDATE Tickets SET FolderID = ? WHERE TicketID=?"));
			
		if(GetQuery().GetRowCount() != 1)
		{
			CEMSString sError;
			sError.Format( _T("Ticket (%d) no longer exists"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// unlock the ticket
		//UnLock();
	}
	catch ( CEMSException Exception )
	{
		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to set date to reopen for ticket (%s):\n\nError: %s"), sSubject.c_str(), Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
}

void CTicket::LogAgentAction( long nAgentID,long nActionID,long nId1,long nId2,long nId3,CEMSString sData1,CEMSString sData2 )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nAgentID );
	BINDPARAM_LONG( GetQuery(), nActionID );
	BINDPARAM_LONG( GetQuery(), nId1 );
	BINDPARAM_LONG( GetQuery(), nId2 );
	BINDPARAM_LONG( GetQuery(), nId3 );
	BINDPARAM_TCHAR_STRING( GetQuery(), sData1 );
	BINDPARAM_TCHAR_STRING( GetQuery(), sData2 );
	GetQuery().Execute(_T("INSERT INTO AgentActivityLog (AgentID,ActivityID,ID1,ID2,ID3,Data1,Data2) VALUES (?,?,?,?,?,?,?)"));	
}

bool CTicket::Merge( int nDestTicketID, bool bCheckSecurity )
{
	// sanity check
	if (nDestTicketID < 1 )
		THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, _T("Destination TicketID not defined!")  );

	// verify the destination Ticket can be queried
	TTickets ticket;	
	ticket.m_TicketID = nDestTicketID;
	if ( ticket.Query( GetQuery() ) != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("Unable to query ticket (%d)"), nDestTicketID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
	
	try
	{
		// do everything inside a transaction
		GetQuery().Initialize();
		GetQuery().Execute( _T("BEGIN TRANSACTION") );	

		// check security
		if ( bCheckSecurity )
			RequireEdit();

		//Get the current Ticket Data
		ticket.m_TicketID = m_TicketID;
		if ( ticket.Query( GetQuery() ) != S_OK)
		{
			CEMSString sError;
			sError.Format( _T("Unable to query ticket (%d)"), m_TicketID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		// get the current time
		TIMESTAMP_STRUCT time;
		GetTimeStamp(time);

		// lock the ticket
		Lock(false);

		// update inbound messages
		GetQuery().Initialize();
		
		BINDPARAM_LONG( GetQuery(), nDestTicketID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("UPDATE InboundMessages SET TicketID=? WHERE TicketID=?"));
		
		// update outbound messages
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE OutboundMessages SET TicketID=? WHERE TicketID=?"));

		// update ticket notes
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE TicketNotes SET TicketID=? WHERE TicketID=?"));

		// update ticket contacts
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE TicketContacts SET TicketID=? WHERE TicketID=?"));

		// update ticket history
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE TicketHistory SET TicketID=? WHERE TicketID=?"));

		// update alert messages
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE AlertMsgs SET TicketID=? WHERE TicketID=?"));

		// delete custom ticket fields
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		GetQuery().Execute( _T("DELETE FROM TicketFieldsTicket WHERE TicketID=?"));
		
		// delete the ticket
		GetQuery().Initialize();
		BINDPARAM_TIME_NOLEN( GetQuery(), time );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		BINDPARAM_LONG( GetQuery(), m_TicketID );
		
		GetQuery().Execute( _T("UPDATE Tickets SET TicketStateID=5,IsDeleted=1,LockedBy=0,DeletedTime=?,DeletedBy=? WHERE TicketID=?") );		
		
		m_IsDeleted = EMS_DELETE_OPTION_DELETE_TO_WASTE_BASKET;
		m_TicketStateID = 5;
		m_DeletedTime = time;
		m_DeletedBy = GetAgentID();
		m_LockedBy = 0;
		m_nLockCount = 0;
	
		// update the ticket history for the deleted ticket
		THMergeTicket( GetQuery(), m_TicketID, GetAgentID(), nDestTicketID );

		// update the ticket history for the deleted ticket
		THDeleteTicket( GetQuery(), m_TicketID, GetAgentID(), ticket.m_TicketStateID, ticket.m_TicketBoxID, ticket.m_OwnerID, ticket.m_PriorityID, ticket.m_TicketCategoryID );

		// update the ticket history for the destination ticket
		THMergeTicket( GetQuery(), nDestTicketID, GetAgentID(), m_TicketID );
				
		//Log it to Agent Activity
		if( _ttoi( sAal.c_str() ) > 0 )
		{
			if( _ttoi( sTd.c_str() ) > 0 )
			{
				LogAgentAction(GetAgentID(),24,m_TicketID,0,0,_T(""),_T(""));
			}
		}

		// we are done, commit the transaction
		GetQuery().Initialize();
		GetQuery().Execute( _T("COMMIT TRANSACTION") );
	}
	catch ( CEMSException Exception )
	{
		GetQuery().Initialize();
		GetQuery().Execute( _T("ROLLBACK TRANSACTION") );

		CEMSString sSubject;
		GetSubjectForException( Exception, sSubject );
			
		// rethrow the exception using our custom description
		CEMSString sError;
		sError.Format( _T("Unable to merge ticket (%d):\n\nError: %s"), m_TicketID, Exception.GetErrorString() );
		THROW_EMS_EXCEPTION( Exception.GetErrorCode(), sError );
	}
	
	return true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds or updates a TicketField	              
\*--------------------------------------------------------------------------*/
void CTicket::AddTicketFieldTicket( long TicketID, long TicketFieldID, long TicketFieldTypeID, TCHAR* DataValue )
{
	int numRows = 0;
	tstring sDataValue;
	sDataValue = ( _T("%s"),DataValue);
	TTicketFieldsTicket tft;
	TTicketFieldOptions tfo;
	CEMSString sLogValue;
	sLogValue.Format(_T("%s"),DataValue);

	try
	{
		if(TicketFieldTypeID == 3)
		{
			GetQuery().Initialize();
			BINDCOL_TCHAR( GetQuery(), tfo.m_OptionValue );
			BINDPARAM_TCHAR( GetQuery(), DataValue );
			GetQuery().Execute( _T("SELECT OptionValue FROM TicketFieldOptions ")
							_T("WHERE TicketFieldOptionID=?") );
			GetQuery().Fetch();
			sLogValue.Format(_T("%s"),tfo.m_OptionValue);
		}
		
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), tft.m_TicketFieldsTicketID );
		BINDCOL_TCHAR( GetQuery(), tft.m_DataValue );		
		BINDPARAM_LONG( GetQuery(), TicketID );
		BINDPARAM_LONG( GetQuery(), TicketFieldID );
		GetQuery().Execute( _T("SELECT TicketFieldsTicketID,DataValue FROM TicketFieldsTicket ")
							_T("WHERE TicketID=? AND TicketFieldID=?") );		

		GetQuery().Fetch();
		numRows = GetQuery().GetRowCount();
		if( numRows == 0 )
		{
			if ((TicketFieldTypeID == 1 && sDataValue.length() > 0) || (TicketFieldTypeID == 2 && _tcscmp( DataValue, _T("1") ) == 0) || (TicketFieldTypeID == 3 && _tcscmp( DataValue, _T("0") ) != 0))
			{
				//Doesn't exist, let's add it				
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), TicketID );
				BINDPARAM_LONG( GetQuery(), TicketFieldID );
				BINDPARAM_TCHAR( GetQuery(), DataValue );
				GetQuery().Execute( _T("INSERT INTO TicketFieldsTicket ")
									_T("(TicketID,TicketFieldID,DataValue) VALUES (?,?,?)") );

				if(TicketFieldTypeID == 3)
				{
					THChangeTicketField( GetQuery(), TicketID, GetAgentID(), TicketFieldID, tfo.m_OptionValue );
				}
				else
				{
					THChangeTicketField( GetQuery(), TicketID, GetAgentID(), TicketFieldID, sDataValue );
				}
				
				//Log it to Agent Activity
				if( _ttoi( sAal.c_str() ) > 0 )
				{
					if( _ttoi( sTcf.c_str() ) > 0 )
					{
						LogAgentAction(GetAgentID(),31,m_TicketID,TicketFieldID,0,sLogValue,_T(""));
					}
				}
			}
		}
		else if ( numRows == 1 )
		{
			if ( _tcscmp( DataValue, tft.m_DataValue ) != 0 )
			{
			
				//Exists and changed, let's update it
				GetQuery().Initialize();
				BINDPARAM_TCHAR( GetQuery(), DataValue );
				BINDPARAM_LONG( GetQuery(), TicketID );
				BINDPARAM_LONG( GetQuery(), TicketFieldID );
				GetQuery().Execute( _T("UPDATE TicketFieldsTicket ")
									_T("SET DataValue=? WHERE TicketID=? AND TicketFieldID=?") );

				if(TicketFieldTypeID == 3)
				{
					THChangeTicketField( GetQuery(), TicketID, GetAgentID(), TicketFieldID, tfo.m_OptionValue );
				}
				else
				{
					THChangeTicketField( GetQuery(), TicketID, GetAgentID(), TicketFieldID, sDataValue );
				}
				
				//Log it to Agent Activity
				if( _ttoi( sAal.c_str() ) > 0 )
				{
					if( _ttoi( sTcf.c_str() ) > 0 )
					{
						LogAgentAction(GetAgentID(),31,m_TicketID,TicketFieldID,0,sLogValue,_T(""));
					}
				}

			}
		}
		else
		{
			//Found more than one, let's delete them all and then add a new one
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), TicketID );
			BINDPARAM_LONG( GetQuery(), TicketFieldID );
			GetQuery().Execute( _T("DELETE FROM TicketFieldsTicket ")
								_T("WHERE TicketID=? AND TicketFieldID=?") );

			if ( sDataValue.length() > 0 )
			{
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), TicketID );
				BINDPARAM_LONG( GetQuery(), TicketFieldID );
				BINDPARAM_TCHAR( GetQuery(), DataValue );
				GetQuery().Execute( _T("INSERT INTO TicketFieldsTicket ")
									_T("(TicketID,TicketFieldID,DataValue) VALUES (?,?,?)") );

				if(TicketFieldTypeID == 3)
				{
					THChangeTicketField( GetQuery(), TicketID, GetAgentID(), TicketFieldID, tfo.m_OptionValue );
				}
				else
				{
					THChangeTicketField( GetQuery(), TicketID, GetAgentID(), TicketFieldID, sDataValue );
				}
				
				//Log it to Agent Activity
				if( _ttoi( sAal.c_str() ) > 0 )
				{
					if( _ttoi( sTcf.c_str() ) > 0 )
					{
						LogAgentAction(GetAgentID(),31,m_TicketID,TicketFieldID,0,sLogValue,_T(""));
					}
				}
			}
		}
	}
	catch(...)
	{
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Unable to update Custom Ticket Field(s)") );
	}
	
}
