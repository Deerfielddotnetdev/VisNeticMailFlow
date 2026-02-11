/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/TicketBoxViewFns.cpp,v 1.2 2005/11/29 21:30:04 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketBoxViewFns.h"

// arrary of column name
LPTSTR g_XMLColumns[] =
{
	_T("Subject"),
	_T("Contact"),
	_T("Date"),
	_T("Priority"),
	_T("State"),
	_T("To"),
	_T("Owner"),
	_T("MsgCount"),
	_T("AttachCount"),
	_T("TicketBox"),
	_T("ticketid"),
	_T("NoteCount"),
	_T("Category"),
	_T("Agent"),
	_T("From"),
	NULL,
};

/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Creates private ticketbox views.
||				Use this function to add an agent to an agent's homepage.
||				When creating a new agent specify the new Agent's ID for
||				both nAgentID and nAgentBoxID.
||
||				nAgentID	- Specifies the AgentID who owns the views 
||				nAgentBoxID - Specifies the AgentID of the agent that 
||							  the views relate to.  
||				           
\*--------------------------------------------------------------------------*/
void TBVAddAgent(CODBCQuery &query, int nAgentID, int nAgentBoxID)
{
	// don't add views for an agent twice ...	
	query.Initialize();
	
	BINDPARAM_LONG(query, nAgentID);
	BINDPARAM_LONG(query, nAgentBoxID);	
	
	query.Execute( _T("SELECT TicketBoxViewID FROM TicketBoxViews ")
		           _T("WHERE AgentID=? AND AgentBoxID=?") );
	
	if (query.Fetch() == S_OK)
		return;

	try
	{
		query.Initialize();
		query.Execute( _T("BEGIN TRANSACTION") );	

		// create the views
		TTicketBoxViews tbv;
		tbv.m_AgentID = nAgentID;
		tbv.m_AgentBoxID = nAgentBoxID;
		tbv.m_SortField = EMS_COLUMN_DATE;
		tbv.m_UseDefault = 1;
				
		// create inbox
		tbv.m_SortAscending = 1;
		tbv.m_TicketBoxViewTypeID = EMS_INBOX;
		tbv.Insert( query );

		// create outbox
		tbv.m_TicketBoxViewTypeID = EMS_OUTBOX;
		tbv.m_SortAscending = 0;
		tbv.Insert( query );

		// create sent items
		tbv.m_TicketBoxViewTypeID = EMS_SENT_ITEMS;
		tbv.Insert( query );

		// create drafts
		tbv.m_TicketBoxViewTypeID = EMS_DRAFTS;
		tbv.Insert( query );
		
		// create alerts
		tbv.m_TicketBoxViewTypeID = EMS_ALERTS;
		tbv.Insert( query );

		// create approvals
		tbv.m_TicketBoxViewTypeID = EMS_APPROVALS;
		tbv.Insert( query );

		// create notes
		tbv.m_TicketBoxViewTypeID = EMS_NOTES;
		tbv.Insert( query );

		query.Initialize();
		query.Execute( _T("COMMIT TRANSACTION") );
	}
	catch( ... )
	{
		query.Initialize();
		query.Execute( _T("ROLLBACK TRANSACTION") );
		throw;
	}
}


/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Deletes private ticketbox views.
||				Use this function to delete an agent from an agent's homepage.
||
||				nAgentID	- Specifies the AgentID who owns the views 
||				nAgentBoxID - Specifies the AgentID of the agent that 
||							  the views relate to.  
||		           
\*--------------------------------------------------------------------------*/
void TBVRemoveAgent(CODBCQuery &query, int nAgentID, int nAgentBoxID)
{
	// remove the views
	query.Initialize();

	BINDPARAM_LONG(query, nAgentID);
	BINDPARAM_LONG(query, nAgentBoxID);	
	
	query.Execute( _T("DELETE FROM TicketBoxViews WHERE AgentID = ? AND AgentBoxID = ?") );
}


/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Deletes all ticketbox views for an agentID
||				use this function when deleting an agent.	  
||				Returns the number of TicketBoxView records deleted.            
\*--------------------------------------------------------------------------*/
int DeleteAllAgentViews(CODBCQuery &query, int nAgentID)
{
	// sanity check
	if (nAgentID < 1)
		return 0;

	query.Initialize();
	
	BINDPARAM_LONG(query, nAgentID);
	BINDPARAM_LONG(query, nAgentID);

	query.Execute( _T("DELETE FROM TicketBoxViews WHERE AgentID = ? OR AgentBoxID = ?") );

	return query.GetRowCount();
}


/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Deletes all ticketbox views for a ticketbox
||				use this function when deleting a ticketbox.	  
||				Returns the number of TicketBoxView records deleted.            
\*--------------------------------------------------------------------------*/
int DeleteAllTicketBoxViews(CODBCQuery &query, int nTicketBoxID)
{
	// sanity check
	if (nTicketBoxID < 1)
		return 0;

	query.Initialize();
	
	BINDPARAM_LONG(query, nTicketBoxID);

	query.Execute( _T("DELETE FROM TicketBoxViews WHERE TicketBoxID = ?") );

	return query.GetRowCount();
}


/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Returns the number of messages in a ticketbox view	              
\*--------------------------------------------------------------------------*/
int GetMsgCount( CODBCQuery &query, int nTypeID, int nAgentBoxID )
{
	CEMSString sQuery;
	LPCTSTR szMsgStateID = NULL;
	int nMsgs;

	switch(nTypeID)
	{
	case EMS_OUTBOX:
		szMsgStateID = _T("> 2");
		break;
		
	case EMS_SENT_ITEMS:
		szMsgStateID = _T("= 2");
		break;
		
	case EMS_DRAFTS:
		szMsgStateID = _T("= 1");
		break;

	case EMS_ALERTS:
		break;

	case EMS_APPROVALS:
		szMsgStateID = _T("= 7");
		break;

	case EMS_NOTES:
		break;

	default:
		// we should never get here
		CEMSString sError;
		sError.Format( _T("ERROR - Invalid TicketBoxViewTypeID (%d)"), nTypeID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	// preform the query
	query.Initialize();

	if ( nTypeID == EMS_ALERTS )
	{
		BINDPARAM_LONG(query, nAgentBoxID);
		BINDCOL_LONG_NOLEN(query, nMsgs);
		sQuery.assign( _T("SELECT COUNT(*) FROM AlertMsgs WHERE AgentID=?"));
	}
	else if ( nTypeID == EMS_APPROVALS )
	{
		BINDPARAM_LONG(query, nAgentBoxID);
		BINDPARAM_LONG(query, nAgentBoxID);
		BINDCOL_LONG_NOLEN(query, nMsgs);
		sQuery.assign( _T("SELECT COUNT(*) FROM Approvals WHERE ApproverAgentID=? ")
					   _T("OR ApproverGroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=?) "));
	}
	else if ( nTypeID == EMS_NOTES )
	{
		BINDPARAM_LONG(query, nAgentBoxID);
		BINDCOL_LONG_NOLEN(query, nMsgs);
		sQuery.assign( _T("SELECT COUNT(*) FROM ContactNotes WHERE AgentID=? ")
					   _T("AND ContactID=0 "));
	}
	else
	{
		BINDPARAM_LONG(query, nAgentBoxID);
		BINDCOL_LONG_NOLEN(query, nMsgs);

		// format the query
		sQuery.Format( _T("SELECT COUNT(*) FROM OutboundMessages ")
					   _T("WHERE AgentID = ? AND IsDeleted = 0 AND OutboundMessageStateID %s"), szMsgStateID );
	}

	query.Execute( sQuery.c_str() );

	query.Fetch();

	return nMsgs;
}


/*---------------------------------------------------------------------------\                     
||  Comments: Returns the number of tickets in a ticketbox view	 	              
\*--------------------------------------------------------------------------*/
int GetTicketCount( CODBCQuery &query, int nTypeID, int nTicketBoxID, int nAgentBoxID, 
					unsigned char bOwnedItems, unsigned char bClosedItems, int nFolderID )
{		
	CEMSString sQuery;
	CEMSString sOwnerID;
	CEMSString sTicketBoxID;
	CEMSString sFolderID;
	LPCTSTR szTicketStateID = bClosedItems ? _T("") : _T("AND TicketStateID > 1");
	int nTickets;

	switch(nTypeID)
	{
	case EMS_INBOX:
		sOwnerID.Format( _T("AND OwnerID = %d"), nAgentBoxID);
		sFolderID.Format( _T("AND FolderID = %d"), nFolderID);
		break;
		
	case EMS_PUBLIC:

		sTicketBoxID.Format( _T("AND Tickets.TicketBoxID = %d"), nTicketBoxID );

		if (!bOwnedItems)
			sOwnerID.assign( _T("AND OwnerID = 0") );
		
		break;
		
	default:
		// we should never get here
		CEMSString sError;
		sError.Format( _T("Invalid TicketBoxViewTypeID (%d)"), nTypeID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	// preform the query
	query.Initialize();

	BINDCOL_LONG_NOLEN(query, nTickets);

	// format the query
	sQuery.Format(_T("SELECT COUNT(*) FROM Tickets WHERE Tickets.IsDeleted = 0 %s %s %s %s "),
		          sFolderID.c_str(), szTicketStateID, sOwnerID.c_str(), sTicketBoxID.c_str() );	
	
	query.Execute( sQuery.c_str() );
	
	query.Fetch();
	
	return nTickets;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns an int for use in the database 
||				TicketBoxViews.SortField	              
\*--------------------------------------------------------------------------*/
int GetSortField(tstring& sSortField)
{
	for (int i = 0; g_XMLColumns[i]; i++)
	{
		if (sSortField.compare(g_XMLColumns[i]) == 0)
			return i;
	}
	
	return -1;
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the column name given an int              
\*--------------------------------------------------------------------------*/
TCHAR* GetXMLColumnName(int nSortField)
{
	if ((nSortField < 0) || (nSortField >= EMS_COLUMN_END))
	{
		return g_XMLColumns[EMS_COLUMN_DATE];
	}
	
	return g_XMLColumns[nSortField];
}
