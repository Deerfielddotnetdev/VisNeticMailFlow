/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ContactHistory.cpp,v 1.2.2.2 2006/01/31 20:39:21 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ContactHistory.h"
#include "TicketList.h"
#include "Ticket.h"	
#include ".\contacthistory.h"
#include "ContactFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction/Destruction	              
\*--------------------------------------------------------------------------*/
CContactHistory::CContactHistory(CISAPIData& ISAPIData) : CXMLDataClass( ISAPIData )
{
}

CContactHistory::~CContactHistory()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CContactHistory::Run(CURLAction& action)
{
	int nID;
	tstring sAction;
	CEMSString sEmail;
	CEMSString sPhone;

	if ((GetISAPIData().GetURLString( _T("Action"), sAction, true )) ||
		(GetISAPIData().GetXMLString( _T("Action"), sAction, true )))
	{
		if (sAction.compare(_T("delete")) == 0)
			DeleteTicket();
		
		else if (sAction.compare(_T("takeownership")) == 0)
			TakeOwnership();
		
		else if ( sAction.compare( _T("escalate") ) == 0 )
			Escalate();

	}
	else if (GetISAPIData().GetURLString( _T("Email"), sEmail, true ))
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS );
		nID = FindContactByEmail( GetQuery(), sEmail.c_str() );
		if ( nID > 0 )
		{
			ListContactHistory( nID );
		}
		else
		{
			THROW_EMS_EXCEPTION( E_AccessDenied, _T("Unable to find contact record!") );
		}
		
	}
	else if (GetISAPIData().GetURLString( _T("Phone"), sPhone, true ))
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS );
		nID = FindContactByPhone( GetQuery(), sPhone.c_str() );
		if ( nID > 0 )
		{
			ListContactHistory( nID );
		}
		else
		{
			THROW_EMS_EXCEPTION( E_AccessDenied, _T("Unable to find contact record!") );
		}
		
	}
	else if (GetISAPIData().GetURLLong( _T("ContactID"), nID ))
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS );
		ListContactHistory( nID );
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	List the tickets associated with a contactID	              
\*--------------------------------------------------------------------------*/
void CContactHistory::ListContactHistory( int nContactID )
{
	int nTicketBoxView = 0;
	if ( GetISAPIData().GetURLLong( _T("TicketBoxView"), nTicketBoxView, true ) )
	{
		GetXMLGen().AddChildElem( _T("TicketBoxView") );
		GetXMLGen().AddChildAttrib( _T("ID"), nTicketBoxView );
	}

	// dump out info about the contact
	GetContactInfo( nContactID );

	// get the ticket count
	int nTicketCount = 0;

	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), nTicketCount );
	BINDPARAM_LONG( GetQuery(), nContactID );
	
	GetQuery().Execute( _T("SELECT COUNT(*) FROM Tickets WHERE IsDeleted=0 AND TicketID IN ")
		_T("(SELECT TicketID FROM TicketContacts WHERE ContactID=?)") );
	
	GetQuery().Fetch();
	
	// calculate the page count
	int nPageCount = nTicketCount / GetMaxRowsPerPage();
	
	if ((nPageCount == 0) || (nTicketCount % GetMaxRowsPerPage())) 
		nPageCount++;
	
	// calculate the current page
	int nCurrentPage = 0;
	
	GetISAPIData().GetURLLong( _T("CurrentPage"), nCurrentPage, true );
	
	// make sure that the current page is atleast 1
	if (nCurrentPage < 1) nCurrentPage = 1;
	
	// are there are less pages then the page requested
	if (nPageCount < nCurrentPage)
		nCurrentPage = nPageCount;
	
	// generate paging XML
	GetXMLGen().AddChildElem(_T("Page"));
	GetXMLGen().AddChildAttrib(_T("Current"), nCurrentPage);
	GetXMLGen().AddChildAttrib(_T("Count"), nPageCount);
	GetXMLGen().AddChildAttrib(_T("OpenItems"), nTicketCount);
	
	// calculate the first and last rows of the rowset that we will care about
	int nStartRow = nCurrentPage > 1 ? ((nCurrentPage - 1) * GetMaxRowsPerPage()) + 1 : 1;
	int nEndRow = nCurrentPage * GetMaxRowsPerPage();
	
	// XML required by TicketList
	GetXMLGen().AddChildElem( _T("TicketBoxItemList") );
	GetXMLGen().AddChildAttrib( _T("Type"), _T("ContactHistory") );
	GetXMLGen().AddChildAttrib( _T("SortBy"), _T("-1") );
	GetXMLGen().AddChildAttrib( _T("SortAscending"), _T("-1") );
	
	// we must have atleast read access to the ticket
	int nRequiredAccess = EMS_READ_ACCESS;
	
	// do we have read access to the default ACL
	// we have to call tihs before GetQuery.Initialize() as it may preform a query
	bool bUseDefaultACL = GetAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, 0 ) >= EMS_READ_ACCESS;
	
	// allocate storage
	CTicketListItem* pTicketArray = NULL;

	if ((pTicketArray = new CTicketListItem[GetMaxRowsPerPage()]) == NULL)
		THROW_EMS_EXCEPTION( E_MemoryError, CEMSString(EMS_STRING_ERROR_MEMORY) );
	
	try
	{
		GetQuery().Initialize(GetMaxRowsPerPage(), sizeof(CTicketListItem));
		GetQuery().EnableScrollCursor();

		BINDCOL_LONG_NOLEN( GetQuery(), pTicketArray[0].m_TicketID );
		BINDCOL_LONG_NOLEN( GetQuery(), pTicketArray[0].m_TicketBoxID );
		BINDCOL_LONG_NOLEN( GetQuery(), pTicketArray[0].m_OwnerID );
		BINDCOL_LONG_NOLEN( GetQuery(), pTicketArray[0].m_LockedBy );	
		BINDCOL_LONG_NOLEN( GetQuery(), pTicketArray[0].m_PriorityID);
		BINDCOL_LONG_NOLEN( GetQuery(), pTicketArray[0].m_TicketStateID );
		BINDCOL_TCHAR( GetQuery(), pTicketArray[0].m_Subject );
		BINDCOL_TCHAR( GetQuery(), pTicketArray[0].m_Contacts );
		BINDCOL_TIME( GetQuery(), pTicketArray[0].m_OpenTimestamp );
		BINDCOL_BIT_NOLEN( GetQuery(), pTicketArray[0].m_UseTickler );
		BINDCOL_LONG_NOLEN( GetQuery(), pTicketArray[0].m_MessageCount );
		BINDCOL_LONG_NOLEN( GetQuery(), pTicketArray[0].m_NoteCount );
		
		BINDPARAM_LONG( GetQuery(), nContactID );
		
		CEMSString sQuery;
		
		sQuery.Format( _T("SELECT TOP %d Tickets.TicketID, Tickets.TicketBoxID, Tickets.OwnerID, Tickets.LockedBy, ")
			_T("PriorityID, TicketStateID, Subject, Contacts, OpenTimeStamp, UseTickler, ")
			_T("(SELECT COUNT(*) FROM InboundMessages WHERE Tickets.TicketID = InboundMessages.TicketID AND IsDeleted=0) + ") 
			_T("(SELECT COUNT(*) FROM OutboundMessages WHERE Tickets.TicketID = OutboundMessages.TicketID AND IsDeleted=0) as MsgCount, ")
			_T("(SELECT COUNT(*) FROM TicketNotes WHERE Tickets.TicketID = TicketNotes.TicketID) as NoteCount, ")
			_T("(SELECT COUNT(*) FROM InboundMessages WHERE Tickets.TicketID = InboundMessages.TicketID AND IsDeleted=0) ")
			_T("FROM Tickets "), nEndRow );


		// check security if the agent is not part
		// of the administrators group...
		if ( !GetIsAdmin() )
		{
			sQuery.append( _T("INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID ")
				_T("INNER JOIN Objects ON TicketBoxes.ObjectID = Objects.ObjectID ") );
		}

		// add the where clause
		sQuery.append( _T("WHERE IsDeleted=0 AND TicketID IN (SELECT TicketID FROM TicketContacts WHERE ContactID=?)") );
			
		// check security if the agent is not part
		// of the administrators group...
		if ( !GetIsAdmin() )
		{
			sQuery.append( _T(" AND (") );
			
			// if the agent has rights to the "default ticketbox" the agent has rights
			// to all ticketboxes that use default rights
			if ( bUseDefaultACL )
			{
				sQuery.append( _T("UseDefaultRights=1 OR (UseDefaultRights=0 AND ") );
			}
			else
			{
				sQuery.append( _T("UseDefaultRights=0 AND (") );
			}
			
			BINDPARAM_LONG( GetQuery(), nRequiredAccess );
			BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
			BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
			
			sQuery.append(	_T("EXISTS (SELECT TOP 1 AccessControlID FROM AccessControl ")
				_T("WHERE ObjectID = TicketBoxes.ObjectID AND AccessLevel >=? AND (AgentID=? OR GroupID IN ")
				_T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=?)))))") );
		}
		
		// add the ORDER BY clause
		sQuery.append( _T(" ORDER BY OpenTimeStamp DESC, TicketID DESC") );
		
		// execute the query
		GetQuery().Execute( sQuery.c_str() );
		
		// fetch the rows
		GetQuery().FetchScroll( SQL_FETCH_ABSOLUTE, nStartRow);
		
		// generate XML
		int nRowCount = GetQuery().GetRowsFetched();
		CEMSString sDate;
		
		GetXMLGen().IntoElem();

		for(int i = 0; i < nRowCount; i++)
		{
			GetXMLGen().AddChildElem( _T("Item") );
			GetXMLGen().AddChildAttrib( _T("ID"), pTicketArray[i].m_TicketID );
			
			AddTicketBoxName( _T("TicketBox"), pTicketArray[i].m_TicketBoxID );
			AddAgentName( _T("Owner"), pTicketArray[i].m_OwnerID );
			
			GetXMLGen().AddChildAttrib( _T("Subject"), pTicketArray[i].m_SubjectLen ? pTicketArray[i].m_Subject : _T("[No Subject]") );
			GetXMLGen().AddChildAttrib( _T("Contact"), pTicketArray[i].m_ContactsLen ? pTicketArray[i].m_Contacts : _T("[No Contacts]") );
			
			if( (pTicketArray[i].m_UseTickler) && (pTicketArray[i].m_TicketStateID == 1))
			{
				GetXMLGen().AddChildAttrib( _T("State"), pTicketArray[i].m_TicketStateID + 6 );
				GetXMLGen().AddChildAttrib( _T("StateName"), _T("Closed - Set To Reopen") );
			}
			else
			{
				GetXMLGen().AddChildAttrib( _T("State"), pTicketArray[i].m_TicketStateID );
				AddTicketStateName( _T("StateName"), pTicketArray[i].m_TicketStateID );
			}
			
			GetXMLGen().AddChildAttrib( _T("Priority"), pTicketArray[i].m_PriorityID );
			AddPriority( _T("PriorityName"), pTicketArray[i].m_PriorityID );
			
			GetDateTimeString( pTicketArray[i].m_OpenTimestamp, pTicketArray[i].m_OpenTimestampLen, sDate );
			GetXMLGen().AddChildAttrib( _T("Date"), sDate.c_str() );
			
			GetXMLGen().AddChildAttrib( _T("LockedBy"), pTicketArray[i].m_LockedBy == GetAgentID() ? 0 : pTicketArray[i].m_LockedBy );
			GetXMLGen().AddChildAttrib( _T("MsgCount"), pTicketArray[i].m_MessageCount );
			GetXMLGen().AddChildAttrib( _T("NoteCount"), pTicketArray[i].m_NoteCount );

			int nUnreadMsg = 0;
			int nUnreadNote = 0;
			GetUnReadMessageCountAndGetUnReadNoteCount(pTicketArray[i].m_TicketID, nUnreadMsg, nUnreadNote);

			if(nUnreadMsg)
				GetXMLGen().AddChildAttrib( _T("hasunread") , 1);
			else
				GetXMLGen().AddChildAttrib( _T("hasunread") , 0);

			GetXMLGen().AddChildAttrib(_T("unreadcount"), nUnreadMsg);

			if(nUnreadNote)
				GetXMLGen().AddChildAttrib( _T("hasunreadnotes") , 1);
			else
				GetXMLGen().AddChildAttrib( _T("hasunreadnotes") , 0);

			GetXMLGen().AddChildAttrib(_T("unreadnotecount"), nUnreadNote);
		}

		GetXMLGen().OutOfElem();
		
		if (pTicketArray)
			delete[] pTicketArray;
	}
	catch(...)
	{
		if (pTicketArray)
			delete[] pTicketArray;

		throw;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add info about the contact to the XML	              
\*--------------------------------------------------------------------------*/
void CContactHistory::GetContactInfo( int nContactID )
{
	// check security
	unsigned char Access = RequireAgentRightLevel(EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS);
	
	// query
	TCHAR Name[CONTACTS_NAME_LENGTH];
	long  NameLen;
	TCHAR Email[PERSONALDATA_DATAVALUE_LENGTH];
	
	GetQuery().Initialize();

	BINDCOL_TCHAR( GetQuery(), Name );
	BINDCOL_TCHAR_NOLEN( GetQuery(), Email );
	BINDPARAM_LONG( GetQuery(), nContactID );
	
	GetQuery().Execute( _T("SELECT Contacts.Name, ")
		_T("(SELECT DataValue FROM PersonalData WHERE PersonalDataID = Contacts.DefaultEmailAddressID) ")
		_T("FROM Contacts ")
		_T("WHERE ContactID=?"));
	
	if ( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Invalid ContactID %d\n"), nContactID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	// generate the XML
	GetXMLGen().AddChildElem(_T("Contact"));
	GetXMLGen().AddChildAttrib(_T("ID"), nContactID);
	GetXMLGen().AddChildAttrib(_T("AccessLevel"), Access);
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(_T("Name"), NameLen ? Name : Email );
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Delete ticket(s)
\*--------------------------------------------------------------------------*/
void CContactHistory::DeleteTicket()
{
	// get the list of tickets	
	CEMSString sTicketIDs;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);
	
	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		Ticket.Delete(true);
	}
	
	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Take ownership of the selected tickets.	              
\*--------------------------------------------------------------------------*/
void CContactHistory::TakeOwnership( void )
{
	// get the list of tickets	
	CEMSString sTicketIDs;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);

	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		Ticket.Reassign( GetAgentID() );
	}

	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Escalate the selected tickets.	              
\*--------------------------------------------------------------------------*/
void CContactHistory::Escalate( void )
{
	// get the list of tickets	
	CEMSString sTicketIDs;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);

	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		Ticket.Escalate();
	}

	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
}

void CContactHistory::GetUnReadMessageCountAndGetUnReadNoteCount(int nTicketID, int& nMC, int& nNC)
{
	TIMESTAMP_STRUCT ts;
	long tsLen = sizeof(TIMESTAMP_STRUCT);
	ZeroMemory(&ts, sizeof(TIMESTAMP_STRUCT));

	int nCountMsg = 0;
	int nTotalCountMsg = 0;
	int nCountNote = 0;
	int nTotalCountNote = 0;
	long lAgentID = GetISAPIData().m_pSession->m_AgentID;

	if(UseCutoffDate(ts))
	{
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), lAgentID);
		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);

		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);

		BINDPARAM_LONG(GetQuery(), lAgentID);
		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);

		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);

		BINDCOL_LONG_NOLEN(GetQuery(), nCountMsg);
		BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountMsg);
		BINDCOL_LONG_NOLEN(GetQuery(), nCountNote);
		BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountNote);

		CEMSString sQuery;
		sQuery.Format(_T("SELECT (SELECT COUNT(*) FROM InboundMessageRead INNER JOIN InboundMessages ON InboundMessageRead.InboundMessageID = InboundMessages.InboundMessageID INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID WHERE (InboundMessageRead.AgentID = ?) AND (Tickets.TicketID = ?) AND (InboundMessages.DateReceived > ?) AND (InboundMessages.IsDeleted=0)), ")
			          _T("(SELECT COUNT(*) FROM InboundMessages WHERE (TicketID = ?) AND (InboundMessages.DateReceived > ?) AND (InboundMessages.IsDeleted=0)), ")
					  _T("(SELECT COUNT(*) FROM TicketNotesRead INNER JOIN TicketNotes ON TicketNotesRead.TicketNoteID = TicketNotes.TicketNoteID INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (TicketNotesRead.AgentID = ?) AND (Tickets.TicketID = ?) AND (TicketNotes.DateCreated > ?)), ")
					  _T("(SELECT COUNT(*) FROM TicketNotes WHERE (TicketID = ?) AND (TicketNotes.DateCreated > ?))"));

		GetQuery().Execute(sQuery.c_str());
		GetQuery().Fetch();
	}
	else
	{
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), lAgentID);
		BINDPARAM_LONG(GetQuery(), nTicketID);

		BINDPARAM_LONG(GetQuery(), nTicketID);

		BINDPARAM_LONG(GetQuery(), lAgentID);
		BINDPARAM_LONG(GetQuery(), nTicketID);

		BINDPARAM_LONG(GetQuery(), nTicketID);

		BINDCOL_LONG_NOLEN(GetQuery(), nCountMsg);
		BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountMsg);
		BINDCOL_LONG_NOLEN(GetQuery(), nCountNote);
		BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountNote);

		CEMSString sQuery;
		sQuery.Format(_T("SELECT (SELECT COUNT(*) FROM InboundMessageRead INNER JOIN InboundMessages ON InboundMessageRead.InboundMessageID = InboundMessages.InboundMessageID INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID WHERE (InboundMessageRead.AgentID = ?) AND (Tickets.TicketID = ?) AND (InboundMessages.IsDeleted=0)), ")
			          _T("(SELECT COUNT(*) FROM InboundMessages WHERE (TicketID = ?) AND (InboundMessages.IsDeleted=0)), ")
					  _T("(SELECT COUNT(*) FROM TicketNotesRead INNER JOIN TicketNotes ON TicketNotesRead.TicketNoteID = TicketNotes.TicketNoteID INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (TicketNotesRead.AgentID = ?) AND (Tickets.TicketID = ?)), ")
					  _T("(SELECT COUNT(*) FROM TicketNotes WHERE (TicketID = ?))"));

		GetQuery().Execute(sQuery.c_str());
		GetQuery().Fetch();
	}

	nMC = nTotalCountMsg - nCountMsg;
	nNC = nTotalCountNote - nCountNote;
}
