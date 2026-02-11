/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TBVInfo.cpp,v 1.2 2005/11/29 21:16:27 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TBVInfo.h"
#include "TicketBoxViewFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Queries a ticketbox view.  Returns additional information
||				needed for MessageList and TicketList	              
\*--------------------------------------------------------------------------*/
void CTBVInfo::Query(CODBCQuery& query)
{
	query.Initialize();
	
	BINDCOL_LONG_NOLEN(query, m_AgentID);
	BINDCOL_LONG_NOLEN(query, m_TicketBoxID);
	BINDCOL_LONG_NOLEN(query, m_AgentBoxID);
	BINDCOL_LONG_NOLEN(query, m_SortField);
	BINDCOL_LONG_NOLEN(query, m_ViewTypeID);
	BINDCOL_BIT_NOLEN(query, m_SortAsc);
	BINDCOL_BIT_NOLEN(query, m_ShowOwnedItems);
	BINDCOL_BIT_NOLEN(query, m_ShowClosedItems);
	BINDCOL_BIT_NOLEN(query, m_ShowState);
	BINDCOL_BIT_NOLEN(query, m_ShowPriority);
	BINDCOL_BIT_NOLEN(query, m_ShowNumNotes);
	BINDCOL_BIT_NOLEN(query, m_ShowTicketID);
	BINDCOL_BIT_NOLEN(query, m_ShowNumMsgs);
	BINDCOL_BIT_NOLEN(query, m_ShowSubject);
	BINDCOL_BIT_NOLEN(query, m_ShowContact);
	BINDCOL_BIT_NOLEN(query, m_ShowDate);
	BINDCOL_BIT_NOLEN(query, m_ShowCategory);
	BINDCOL_BIT_NOLEN(query, m_ShowOwner);
	BINDCOL_BIT_NOLEN(query, m_ShowTicketBox);
	BINDCOL_BIT_NOLEN(query, m_UseDefault);
	BINDPARAM_LONG(query, m_TicketBoxViewID);
	
	query.Execute( _T("SELECT AgentID, TicketBoxID, AgentBoxID, SortField, ")
		           _T("TicketBoxViewTypeID, SortAscending, ShowOwnedItems, ShowClosedItems, ")
				   _T("ShowState,ShowPriority,ShowNumNotes,ShowTicketID,ShowNumMsgs, ")
				   _T("ShowSubject,ShowContact,ShowDate,ShowCategory,ShowOwner,ShowTicketBox,UseDefault ")
		           _T("FROM TicketBoxViews ")
		           _T("WHERE TicketBoxViewID = ?"));
	
	if (query.Fetch() != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("The specified view (%d) does not exist"), m_TicketBoxViewID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Updates open item and page counts	              
\*--------------------------------------------------------------------------*/
int CTBVInfo::GetItemCount(CODBCQuery& query, int nFolderID)
{
	int nItemCount;

	// get the open item count
	if ((m_ViewTypeID == EMS_INBOX) || (m_ViewTypeID == EMS_PUBLIC))
	{
		nItemCount = GetTicketCount( query, m_ViewTypeID, m_TicketBoxID, m_AgentBoxID, 
									  m_ShowOwnedItems, m_ShowClosedItems, nFolderID );
	}
	else
	{
		nItemCount = GetMsgCount( query, m_ViewTypeID, m_AgentBoxID );
	}

	return nItemCount;
}