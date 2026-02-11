/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketBoxView.cpp,v 1.2.2.1.2.1 2006/07/18 12:55:03 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketBoxView.h"
#include "Listclasses.h"
#include "MarkupSTL.h"
#include "TicketBoxViewFns.h"

/*---------------------------------------------------------------------------\             
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CTicketBoxView::Run( CURLAction& action )
{
	int nType = 0;
	
	GetServerParameter(43, _sEnableMyTickets, "0");
	GetServerParameter(44, _sEnablePublicTicketBoxes, "0");
	GetServerParameter(45, _sEnableAgentViews, "0");

	if (!GetISAPIData().GetXMLLong( _T("type"), nType, true ))
	{
		GetISAPIData().GetURLLong( _T("type"), nType, true);
	}
	
	if(nType == 0)
	{
		List();
	}
	else
	{
		ListAgentStatus();
	}

	return 0;
}

/*---------------------------------------------------------------------------\             
||  Comments:	Build the logged on agent's TicketBox list	              
\*--------------------------------------------------------------------------*/
void CTicketBoxView::List()
{
	CEMSString sURL;
	dca::String f;

	GetXMLGen().AddChildElem(_T("TicketBoxList"));
	
	//DebugReporter::Instance().DisplayMessage("CTicketBoxView::List - Set and add the Update URL", DebugReporter::ISAPI, GetCurrentThreadId());	

	// Add the URL for updates
	sURL.Format( _T("%sticketboxlist.ems?OutputXML=1"),
					GetISAPIData().m_sURLSubDir.c_str() );
	GetXMLGen().AddChildAttrib( _T("URL"), sURL.c_str() );

	GetXMLGen().IntoElem();

	//f.Format("CTicketBoxView::List - Update URL set to: %s, ListPrivateViews()", sURL.c_str() );
	//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	
	ListPrivateViews();

	//DebugReporter::Instance().DisplayMessage("CTicketBoxView::List - ListPublicViews()", DebugReporter::ISAPI, GetCurrentThreadId());
	
	ListPublicViews();
}


/*---------------------------------------------------------------------------\             
||  Comments:	Get the status of Agents	              
\*--------------------------------------------------------------------------*/
void CTicketBoxView::ListAgentStatus()
{
	CEMSString sURL;
	
	GetXMLGen().AddChildElem(_T("TicketBoxList"));
	
	// Add the URL for updates
	sURL.Format( _T("%sticketboxlist.ems?OutputXML=1"),
					GetISAPIData().m_sURLSubDir.c_str() );
	GetXMLGen().AddChildAttrib( _T("URL"), sURL.c_str() );

	GetXMLGen().IntoElem();

	CTBVPrivateList ListItem;
	int nCurrentAgentBoxID = 0;	
	int nAgentID = GetAgentID();

	long m_lStart;
	long m_lEnd;
	long m_lElapsed;
	dca::String f;
	
	m_lStart = GetTickCount();
	
	// preform the query
	ListItem.List( GetQuery(), GetAgentID() );

	// collect the results into memory
	list<CTBVPrivateList> List;
	while (GetQuery().Fetch() == S_OK)
	{
		if ( !(ListItem.m_TypeID == EMS_APPROVALS && ListItem.m_AgentBoxID != nAgentID) )
		{
            List.push_back(ListItem);
		}
	}
	
	// build the XML
	list<CTBVPrivateList>::iterator iter;

	for ( iter = List.begin(); iter != List.end(); iter++ )
	{
		// if this is a new agent, build the agent tag
		if (nCurrentAgentBoxID != iter->m_AgentBoxID)
		{
			// if this isn't the first time move up one 
			// level this puts us under the <TicketBoxList> tag
			if ( iter != List.begin() )
				GetXMLGen().OutOfElem();

			// write the agent data to the XML string
			GetXMLGen().AddChildElem( _T("Agent") );
			AddAgentName( _T("Name"), iter->m_AgentBoxID );
			GetXMLGen().SetChildAttrib( _T("ID"), iter->m_AgentBoxID);
			GetXMLGen().SetChildAttrib( _T("LoggedOnAgent"), iter->m_AgentBoxID == GetAgentID() ? _T("1") : _T("0") );

			tstring sStatusText(_T("Offline"));
			int nLoggedIn = 0;
			
			int nStatusID = m_ISAPIData.m_SessionMap.GetAgentStatus(iter->m_AgentBoxID,sStatusText,nLoggedIn);
			
			f.Format("CTicketBoxView::ListAgentStatus Setting status XML for AgentID [%d] to StatusID [%d] StatusText [%s] and LoggedIn [%d]", iter->m_AgentBoxID, nStatusID, sStatusText.c_str(), nLoggedIn);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			
			GetXMLGen().SetChildAttrib( _T("StatusID"), nStatusID);
			GetXMLGen().SetChildAttrib( _T("StatusText"), sStatusText.c_str());
			GetXMLGen().SetChildAttrib( _T("LoggedIn"), nLoggedIn);

			// set the status
			if (GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, iter->m_AgentBoxID ) < EMS_READ_ACCESS )
			{
				GetXMLGen().SetChildAttrib( _T("Status"), EMS_TICKETBOX_ACCESSDENIED );
			}
			else
			{
				GetXMLGen().SetChildAttrib( _T("Status"), _T("0") );
			}

			// move down one level, this places us 
			// under the agent tag that was just created
			GetXMLGen().IntoElem();

			nCurrentAgentBoxID = iter->m_AgentBoxID;
		}		

	}
	
	// move up one level this puts us
	// under the <TicketBoxList> tag
	GetXMLGen().OutOfElem();

	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CTicketBoxView::ListAgentStatus AgentID: %d - Elapsed: %d", nAgentID,m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
}

/*---------------------------------------------------------------------------\             
||  Comments:	Build the list of private TicketBoxes	              
\*--------------------------------------------------------------------------*/
void CTicketBoxView::ListPrivateViews()
{
	DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - Entering", DebugReporter::ISAPI, GetCurrentThreadId());
	
	CTBVPrivateList ListItem;
	int nCurrentAgentBoxID = 0;
	int nOpenItems;
	int nInOpenItems;
	int BoldAlerts;
	int nUnreadAlerts;
	bool bFoundApprovals = false;
	bool bFoundMyNotes = false;
	int nAgentID = GetAgentID();
	int nViewID = 0;
	
	long m_lStart;
	long m_lEnd;
	long m_lElapsed;
	dca::String f;
	
	DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - GetTickCount()", DebugReporter::ISAPI, GetCurrentThreadId());
	
	m_lStart = GetTickCount();
	
	// preform the query
	DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - ListItem.List()", DebugReporter::ISAPI, GetCurrentThreadId());
	
	ListItem.List( GetQuery(), GetAgentID() );

	// collect the results into memory
	DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - Add items to CTBVPrivateList", DebugReporter::ISAPI, GetCurrentThreadId());
	
	list<CTBVPrivateList> List;
	while (GetQuery().Fetch() == S_OK)
	{
		if ( ListItem.m_TypeID == EMS_APPROVALS && ListItem.m_AgentBoxID == nAgentID )
			bFoundApprovals=true;

		if ( ListItem.m_TypeID == EMS_NOTES && ListItem.m_AgentBoxID == nAgentID)
			bFoundMyNotes=true;

		if ( !(ListItem.m_TypeID == EMS_APPROVALS && ListItem.m_AgentBoxID != nAgentID) )
		{
            List.push_back(ListItem);
		}
	}

	if ( !bFoundApprovals )
	{
		DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - Did not find Approvals, Add then relist", DebugReporter::ISAPI, GetCurrentThreadId());
	
		bool bAddApprovals = false;

		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), nAgentID );
		BINDPARAM_LONG( GetQuery(), nAgentID );
		BINDCOL_LONG_NOLEN(GetQuery(), nViewID);

		GetQuery().Execute( _T("SELECT TicketBoxViewID FROM TicketBoxViews ")
								_T("WHERE AgentID=? AND AgentBoxID=? AND TicketBoxViewTypeID=10") );

		if(GetQuery().Fetch() != S_OK)
		{
			bAddApprovals = true;			
		}
		
		if(bAddApprovals)
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), nAgentID );
			BINDPARAM_LONG( GetQuery(), nAgentID );
			GetQuery().Execute( _T("INSERT INTO TicketBoxViews ")
								_T("(AgentID,AgentBoxID,TicketBoxViewTypeID,SortField,SortAscending) ")
								_T("VALUES (?,?,10,2,1)") );

			List.clear();		
			ListItem.List( GetQuery(), GetAgentID() );
			while (GetQuery().Fetch() == S_OK)
			{
				if ( !(ListItem.m_TypeID == EMS_APPROVALS && ListItem.m_AgentBoxID != nAgentID) )
				{
					List.push_back(ListItem);
				}
			}
		}
		else
		{
			ListItem.m_TypeID=EMS_APPROVALS;
			ListItem.m_AgentBoxID=nAgentID;
			ListItem.m_ViewID=nViewID;
			List.push_back(ListItem);
		}
	}

	if ( !bFoundMyNotes )
	{
		DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - Did not find MyNotes, Add then relist", DebugReporter::ISAPI, GetCurrentThreadId());
	
		bool bAddMyNotes = false;

		nViewID=0;
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), nAgentID );
		BINDPARAM_LONG( GetQuery(), nAgentID );
		BINDCOL_LONG_NOLEN(GetQuery(), nViewID);
		GetQuery().Execute( _T("SELECT TicketBoxViewID FROM TicketBoxViews ")
								_T("WHERE AgentID=? AND AgentBoxID=? AND TicketBoxViewTypeID=11") );

		if(GetQuery().Fetch() != S_OK)
		{
			bAddMyNotes = true;			
		}
		
		if(bAddMyNotes)
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), nAgentID );
			BINDPARAM_LONG( GetQuery(), nAgentID );
			GetQuery().Execute( _T("INSERT INTO TicketBoxViews ")
									_T("(AgentID,AgentBoxID,TicketBoxViewTypeID,SortField,SortAscending) ")
									_T("VALUES (?,?,11,2,1)") );

			

			List.clear();		
			ListItem.List( GetQuery(), GetAgentID() );
			while (GetQuery().Fetch() == S_OK)
			{
				if ( !(ListItem.m_TypeID == EMS_APPROVALS && ListItem.m_AgentBoxID != nAgentID) )
				{
					List.push_back(ListItem);
				}
			}
		}
		else
		{
			ListItem.m_TypeID=EMS_NOTES;
			ListItem.m_AgentBoxID=nAgentID;
			ListItem.m_ViewID=nViewID;
			List.push_back(ListItem);
		}
	}
	
	// build the XML
	list<CTBVPrivateList>::iterator iter;

	DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - Build the XML", DebugReporter::ISAPI, GetCurrentThreadId());
	
	for ( iter = List.begin(); iter != List.end(); iter++ )
	{
		nOpenItems = 0;
		nInOpenItems = 0;
		BoldAlerts = 0;
		nUnreadAlerts = 0;

		// if this is a new agent, build the agent tag
		if (nCurrentAgentBoxID != iter->m_AgentBoxID)
		{
			// if this isn't the first time move up one 
			// level this puts us under the <TicketBoxList> tag
			if ( iter != List.begin() )
				GetXMLGen().OutOfElem();

			// write the agent data to the XML string
			GetXMLGen().AddChildElem( _T("Agent") );
			
			DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - AddAgentName()", DebugReporter::ISAPI, GetCurrentThreadId());
			
			AddAgentName( _T("Name"), iter->m_AgentBoxID );
			GetXMLGen().SetChildAttrib( _T("ID"), iter->m_AgentBoxID);
			GetXMLGen().SetChildAttrib( _T("LoggedOnAgent"), iter->m_AgentBoxID == GetAgentID() ? _T("1") : _T("0") );

			tstring sStatusText(_T("Offline"));
			int nLoggedIn = 0;
			
			DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - GetAgentStatus()", DebugReporter::ISAPI, GetCurrentThreadId());
			
			int nStatusID = m_ISAPIData.m_SessionMap.GetAgentStatus(iter->m_AgentBoxID,sStatusText,nLoggedIn);

			f.Format("CTicketBoxView::ListPrivateViews Setting status XML for AgentID [%d] to StatusID [%d] StatusText [%s] and LoggedIn [%d]", iter->m_AgentBoxID, nStatusID, sStatusText.c_str(), nLoggedIn);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			
			GetXMLGen().SetChildAttrib( _T("StatusID"), nStatusID);
			GetXMLGen().SetChildAttrib( _T("StatusText"), sStatusText.c_str());
			GetXMLGen().SetChildAttrib( _T("LoggedIn"), nLoggedIn);

			DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - Set the Status", DebugReporter::ISAPI, GetCurrentThreadId());
			// set the status
			if (GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, iter->m_AgentBoxID ) < EMS_READ_ACCESS )
			{
				GetXMLGen().SetChildAttrib( _T("Status"), EMS_TICKETBOX_ACCESSDENIED );
			}
			else if (GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, iter->m_AgentBoxID ) == EMS_DELETE_ACCESS )
			{
				GetXMLGen().SetChildAttrib( _T("Status"), _T("4") );
			}
			else
			{
				GetXMLGen().SetChildAttrib( _T("Status"), _T("0") );
			}

			// move down one level, this places us 
			// under the agent tag that was just created
			GetXMLGen().IntoElem();

			nCurrentAgentBoxID = iter->m_AgentBoxID;
		}
		
		if (iter->m_TypeID == EMS_ALERTS)
		{
			DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - Alerts", DebugReporter::ISAPI, GetCurrentThreadId());
			
			int nViewed, nCount, nUnread;

			GetQuery().Initialize();
			BINDCOL_LONG_NOLEN(GetQuery(), nViewed );
			BINDCOL_LONG_NOLEN(GetQuery(), nCount );
			BINDCOL_LONG_NOLEN(GetQuery(), nUnread );
			BINDPARAM_LONG(GetQuery(), iter->m_AgentBoxID );
			BINDPARAM_LONG(GetQuery(), iter->m_AgentBoxID );
			GetQuery().Execute( _T("SELECT Viewed,COUNT(*),(SELECT Count(*) FROM AlertMsgs WHERE AgentID=? AND Viewed=0) FROM AlertMsgs WHERE AgentID=? GROUP BY Viewed") );

			if ( GetQuery().Fetch() == S_OK )
			{
				nOpenItems = nCount;
				nUnreadAlerts = nUnread;
			
				if (!BoldAlerts)
					BoldAlerts = !nViewed;				
			}
				
			if ( GetQuery().Fetch() == S_OK )
			{
				nOpenItems += nCount;
				nUnreadAlerts = nUnread;
					
				if (!BoldAlerts)
					BoldAlerts = !nViewed;			
			}
		}
		//else if (iter->m_TypeID != EMS_SENT_ITEMS && iter->m_TypeID != EMS_INBOX)
		else if (iter->m_TypeID != EMS_INBOX)
		{
			f.Format("CTicketBoxView::ListPrivateViews GetMsgCount(TypeID:%d AgentBoxID:%d)", iter->m_TypeID,iter->m_AgentBoxID );
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			
			nOpenItems = GetMsgCount( GetQuery(), iter->m_TypeID, iter->m_AgentBoxID );
		}
	
		// write the XML data for the ticketbox
		GetXMLGen().AddChildElem(_T("TicketBox"));
		GetXMLGen().SetChildAttrib( _T("ViewID"), iter->m_ViewID );

		DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - AddTicketBoxViewTypeName()", DebugReporter::ISAPI, GetCurrentThreadId());
			
		AddTicketBoxViewTypeName( _T("Name"), iter->m_TypeID );
		if(iter->m_TypeID != EMS_INBOX)
			GetXMLGen().SetChildAttrib( _T("Count"), nOpenItems );

		if ( iter->m_AgentBoxID == GetAgentID() && iter->m_TypeID == EMS_ALERTS )
		{
			DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPrivateViews - Bold Alerts", DebugReporter::ISAPI, GetCurrentThreadId());
		
			GetXMLGen().SetChildAttrib( _T("Bold"), BoldAlerts );
			if(_sEnableMyTickets.ToInt() != 0)
			{
				GetXMLGen().SetChildAttrib( _T("unread_count"), nUnreadAlerts );			
			}
			else
			{
				GetXMLGen().SetChildAttrib( _T("unread_count"), "-1");
			}			
		}
		
		GetXMLGen().SetChildAttrib( _T("TypeID"), iter->m_TypeID );

		if(iter->m_TypeID == EMS_INBOX)
		{
			if(iter->m_AgentBoxID == GetAgentID())
			{
				//DcaTrace(_T("*  CTicketBoxView::ListPrivateViews() - EnableMyTicketBoxRead [ %d ]"), _sEnableMyTickets.ToInt());
				if(_sEnableMyTickets.ToInt() != 0)
				{
					f.Format("CTicketBoxView::ListPrivateViews GetUnreadMessageCountPrivate(AgentBoxID:%d ShowClosed:%d)", iter->m_AgentBoxID,iter->m_ShowClosedItems );
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					
					GetXMLGen().SetChildAttrib( _T("unread_count"), GetUnreadMessageCountPrivate(iter->m_AgentBoxID, iter->m_ShowClosedItems));
				}
				else
				{
					GetXMLGen().SetChildAttrib( _T("unread_count"), "-1");
				}
			}
			else
			{
				//DcaTrace(_T("*  CTicketBoxView::ListPrivateViews() - EnableAgentBoxRead [ %d ]"), _sEnableAgentViews.ToInt());

				if(_sEnableAgentViews.ToInt() != 0)
				{
					f.Format("CTicketBoxView::ListPrivateViews GetUnreadMessageCountPrivate(AgentBoxID:%d ShowClosed:%d)", iter->m_AgentBoxID,iter->m_ShowClosedItems );
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					
					GetXMLGen().SetChildAttrib( _T("unread_count"), GetUnreadMessageCountPrivate(iter->m_AgentBoxID, iter->m_ShowClosedItems));
				}
				else
				{
					GetXMLGen().SetChildAttrib( _T("unread_count"), "-1");
				}
			}

			f.Format("CTicketBoxView::ListPrivateViews GetTicketCount(TypeID:%d AgentBoxID:%d ShowClosed:%d)", iter->m_TypeID,iter->m_AgentBoxID,iter->m_ShowClosedItems );
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

			nInOpenItems = GetTicketCount( GetQuery(), iter->m_TypeID, NULL, iter->m_AgentBoxID, 
				                         NULL, iter->m_ShowClosedItems );

			GetXMLGen().SetChildAttrib( _T("Count"), nInOpenItems );
			
			GetXMLGen().IntoElem();
			ListMyFolders(iter->m_AgentBoxID);
			GetXMLGen().OutOfElem();
		}

	}
	
	// move up one level this puts us
	// under the <TicketBoxList> tag
	GetXMLGen().OutOfElem();

	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CTicketBoxView::ListPrivateViews AgentID: %d - Elapsed: %d", nAgentID,m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
}

/*---------------------------------------------------------------------------\             
||  Comments:	              
\*--------------------------------------------------------------------------*/
void CTicketBoxView::ListPublicViews()
{	
	CTBVPublicList ListItem;
	int nStatus;
	
	long m_lStart;
	long m_lEnd;
	long m_lElapsed;
	dca::String f;
	
	f.Format("CTicketBoxView::ListPublicViews - Entering for AgentID: %d", GetAgentID());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	
	m_lStart = GetTickCount();

	DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPublicViews - g_ThreadPool.GetSharedObjects().m_bUseATC", DebugReporter::ISAPI, GetCurrentThreadId());

	bool bUseATC = g_ThreadPool.GetSharedObjects().m_bUseATC;

	DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPublicViews - ListItem.List()", DebugReporter::ISAPI, GetCurrentThreadId());

	// preform the query
	ListItem.List(GetQuery(), GetAgentID(), GetIsAdmin(), bUseATC );
	
	DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPublicViews - Add items to CTBVPublicList", DebugReporter::ISAPI, GetCurrentThreadId());

	// collect the results into memory
	list<CTBVPublicList> List;
	while (GetQuery().Fetch() == S_OK)
	{
		List.push_back(ListItem);
	}
	
	list<CTBVPublicList>::iterator iter;
	
	DebugReporter::Instance().DisplayMessage("CTicketBoxView::ListPublicViews - Build the XML", DebugReporter::ISAPI, GetCurrentThreadId());

	for ( iter = List.begin(); iter != List.end(); iter++ )
	{	
		nStatus = EMS_TICKETBOX_NORMAL;

		GetXMLGen().AddChildElem(_T("TicketBox"));
		GetXMLGen().AddChildAttrib( _T("ViewID"), iter->nViewID );
		AddTicketBoxName( _T("Name"), iter->nTicketBoxID );
		GetXMLGen().AddChildAttrib( _T("TypeID"), EMS_PUBLIC );

		if ( GetAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, iter->nTicketBoxID ) < EMS_READ_ACCESS )
		{
			nStatus = EMS_TICKETBOX_ACCESSDENIED;
			iter->nCount = 0;
		}
		else
		{
			if ( iter->WaterMarkStatus == 1)
			{
				nStatus = EMS_TICKETBOX_LOWWATERMARK;
			}
			else if ( iter->WaterMarkStatus == 2 )
			{
				nStatus = EMS_TICKETBOX_HIGHWATERMARK;
			}
		}
		
		GetXMLGen().SetChildAttrib( _T("Count"), iter->nCount );
		GetXMLGen().SetChildAttrib( _T("Status"), nStatus );

		if(_sEnablePublicTicketBoxes.ToInt() != 0)
			GetXMLGen().SetChildAttrib( _T("unread_count"), GetUnreadMessageCountPublic(iter->nTicketBoxID, iter->nShowClosedItems, iter->nShowOwnedItems, iter->nUnreadMode));
		else
			GetXMLGen().SetChildAttrib( _T("unread_count"), "-1");
	}

	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CTicketBoxView::ListPublicViews AgentID: %d - Elapsed: %d", GetAgentID(),m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
}

int CTicketBoxView::GetUnreadMessageCountPrivate(int nAgentID, unsigned int nShowClosed, int nFolderID)
{
	int		nTotalCount = 0;
	unsigned int nState = 1 - nShowClosed;
	CEMSString sQuery;
	TIMESTAMP_STRUCT ts;
	long tsLen = sizeof(TIMESTAMP_STRUCT);
	ZeroMemory(&ts, sizeof(TIMESTAMP_STRUCT));
	
	GetQuery().Initialize();
	if(UseCutoffDate(ts))
	{
		
			BINDPARAM_LONG(GetQuery(), nAgentID);
			BINDPARAM_LONG(GetQuery(), nFolderID);
			BINDPARAM_LONG(GetQuery(), nState);
			BINDPARAM_TIME(GetQuery(), ts);
			BINDPARAM_LONG(GetQuery(), nAgentID);
			BINDPARAM_TIME(GetQuery(), ts);
			BINDPARAM_LONG(GetQuery(), nAgentID);
			BINDCOL_LONG_NOLEN(GetQuery(), nTotalCount);
			
			sQuery.Format(_T("SELECT COUNT(TI.TicketID) FROM Tickets TI WHERE (TI.OwnerID=?) AND (TI.FolderID=?) AND ")
							   _T("(TI.IsDeleted=0) AND (TI.TicketStateID>?) AND TI.TicketID IN ")
							   _T("((SELECT DISTINCT(TicketID) FROM InboundMessages IM WHERE IM.TicketID=TI.TicketID AND IM.IsDeleted=0 AND IM.DateReceived > ? AND IM.InboundMessageID NOT IN ")
								_T("(SELECT InboundMessageID FROM InboundMessageRead WHERE AgentID=?)) ")
								_T("UNION ")
								_T("(SELECT DISTINCT(TicketID) FROM TicketNotes TN WHERE TN.TicketID=TI.TicketID AND TN.DateCreated > ? AND TN.TicketNoteID NOT IN ")
								_T("(SELECT TicketNoteID FROM TicketNotesRead WHERE AgentID=?)))"));	
			
			
	}
	else
	{
		
			BINDPARAM_LONG(GetQuery(), nAgentID);
			BINDPARAM_LONG(GetQuery(), nFolderID);
			BINDPARAM_LONG(GetQuery(), nState);
			BINDPARAM_LONG(GetQuery(), nAgentID);
			BINDPARAM_LONG(GetQuery(), nAgentID);
			BINDCOL_LONG_NOLEN(GetQuery(), nTotalCount);
			
			sQuery.Format(_T("SELECT COUNT(TI.TicketID) FROM Tickets TI WHERE (TI.OwnerID=?) AND (TI.FolderID=?) AND ")
							   _T("(TI.IsDeleted=0) AND (TI.TicketStateID>?) AND TI.TicketID IN ")
							   _T("((SELECT DISTINCT(TicketID) FROM InboundMessages IM WHERE IM.TicketID=TI.TicketID AND IM.IsDeleted=0 AND IM.InboundMessageID NOT IN ")
								_T("(SELECT InboundMessageID FROM InboundMessageRead WHERE AgentID=?)) ")
								_T("UNION ")
								_T("(SELECT DISTINCT(TicketID) FROM TicketNotes TN WHERE TN.TicketID=TI.TicketID AND TN.TicketNoteID NOT IN ")
								_T("(SELECT TicketNoteID FROM TicketNotesRead WHERE AgentID=?)))"));
			
				
	}

	GetQuery().Execute(sQuery.c_str());
	GetQuery().Fetch();
	
	return nTotalCount;
	
}

int CTicketBoxView::GetUnreadMessageCountPublic(int nTicketBoxID, unsigned int nShowClosed, unsigned int nShowOwned, unsigned int nUnreadMode)
{
	int		nTotalCount = 0;
	unsigned int nState = 1 - nShowClosed;
	dca::String sOwned;
	CEMSString sQuery;
	long lAgentID = GetISAPIData().m_pSession->m_AgentID;
	TIMESTAMP_STRUCT ts;
	long tsLen = sizeof(TIMESTAMP_STRUCT);
	ZeroMemory(&ts, sizeof(TIMESTAMP_STRUCT));
	
	long m_lStart;
	long m_lEnd;
	long m_lElapsed;
	dca::String f;

	if(nShowOwned != 1)
		sOwned.assign(_T(" AND (TI.OwnerID = 0)"));
	
	m_lStart = GetTickCount();

	GetQuery().Initialize();
	if(UseCutoffDate(ts))
	{
		if ( !nUnreadMode )
		{
			if ( !GetIsAdmin() && g_ThreadPool.GetSharedObjects().m_bUseATC )
			{
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDPARAM_LONG(GetQuery(), nTicketBoxID);
				BINDPARAM_LONG(GetQuery(), nState);
				BINDPARAM_TIME(GetQuery(), ts);
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDPARAM_TIME(GetQuery(), ts);
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDCOL_LONG_NOLEN(GetQuery(), nTotalCount);
				
				sQuery.Format(_T("SELECT COUNT(TI.TicketID) FROM Tickets TI INNER JOIN AgentTicketCategories ATC ON TI.TicketCategoryID=ATC.TicketCategoryID ")
								_T("WHERE (ATC.AgentID=?) AND (TI.TicketBoxID=?) AND (TI.IsDeleted=0) AND (TI.TicketStateID>?) AND ")
								_T("TI.TicketID IN ")
								_T("((SELECT DISTINCT(TicketID) FROM InboundMessages IM WHERE IM.TicketID=TI.TicketID AND IM.IsDeleted=0 AND IM.DateReceived > ? AND IM.InboundMessageID NOT IN ")
								_T("(SELECT InboundMessageID FROM InboundMessageRead WHERE AgentID=?)) ")
								_T("UNION ")
								_T("(SELECT DISTINCT(TicketID) FROM TicketNotes TN WHERE TN.TicketID=TI.TicketID AND TN.DateCreated > ? AND TN.TicketNoteID NOT IN ")
								_T("(SELECT TicketNoteID FROM TicketNotesRead WHERE AgentID=?))) %s"), sOwned.c_str());						
			}
			else
			{
				BINDPARAM_LONG(GetQuery(), nTicketBoxID);
				BINDPARAM_LONG(GetQuery(), nState);
				BINDPARAM_TIME(GetQuery(), ts);
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDPARAM_TIME(GetQuery(), ts);
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDCOL_LONG_NOLEN(GetQuery(), nTotalCount);
				
				sQuery.Format(_T("SELECT COUNT(TI.TicketID) FROM Tickets TI WHERE (TI.TicketBoxID=?) AND ")
								_T("(TI.IsDeleted=0) AND (TI.TicketStateID>?) AND TI.TicketID IN ")
								_T("((SELECT DISTINCT(TicketID) FROM InboundMessages IM WHERE IM.TicketID=TI.TicketID AND IM.IsDeleted=0 AND IM.DateReceived > ? AND IM.InboundMessageID NOT IN ")
								_T("(SELECT InboundMessageID FROM InboundMessageRead WHERE AgentID=?)) ")
								_T("UNION ")
								_T("(SELECT DISTINCT(TicketID) FROM TicketNotes TN WHERE TN.TicketID=TI.TicketID AND TN.DateCreated > ? AND TN.TicketNoteID NOT IN ")
								_T("(SELECT TicketNoteID FROM TicketNotesRead WHERE AgentID=?))) %s"), sOwned.c_str());				
			}
		}
		else
		{
			if ( !GetIsAdmin() && g_ThreadPool.GetSharedObjects().m_bUseATC )
			{
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDPARAM_LONG(GetQuery(), nTicketBoxID);
				BINDPARAM_LONG(GetQuery(), nState);
				BINDPARAM_TIME(GetQuery(), ts);
				BINDPARAM_TIME(GetQuery(), ts);
				BINDCOL_LONG_NOLEN(GetQuery(), nTotalCount);
				
				sQuery.Format(_T("SELECT COUNT(TI.TicketID) FROM Tickets TI INNER JOIN AgentTicketCategories ATC ON TI.TicketCategoryID=ATC.TicketCategoryID ")
								_T("WHERE (ATC.AgentID=?) AND (TI.TicketBoxID=?) AND (TI.IsDeleted=0) AND (TI.TicketStateID>?) AND ")
								_T("TI.TicketID IN ")
								_T("((SELECT DISTINCT(TicketID) FROM InboundMessages IM WHERE IM.TicketID=TI.TicketID AND IM.IsDeleted=0 AND IM.DateReceived > ? AND IM.InboundMessageID NOT IN ")
								_T("(SELECT InboundMessageID FROM InboundMessageRead)) ")
								_T("UNION ")
								_T("(SELECT DISTINCT(TicketID) FROM TicketNotes TN WHERE TN.TicketID=TI.TicketID AND TN.DateCreated > ? AND TN.TicketNoteID NOT IN ")
								_T("(SELECT TicketNoteID FROM TicketNotesRead))) %s"), sOwned.c_str());	
						
			}
			else
			{
				BINDPARAM_LONG(GetQuery(), nTicketBoxID);
				BINDPARAM_LONG(GetQuery(), nState);
				BINDPARAM_TIME(GetQuery(), ts);
				BINDPARAM_TIME(GetQuery(), ts);
				BINDCOL_LONG_NOLEN(GetQuery(), nTotalCount);
				
				sQuery.Format(_T("SELECT COUNT(TI.TicketID) FROM Tickets TI WHERE (TI.TicketBoxID=?) AND ")
								_T("(TI.IsDeleted=0) AND (TI.TicketStateID>?) AND TI.TicketID IN ")
								_T("((SELECT DISTINCT(TicketID) FROM InboundMessages IM WHERE IM.TicketID=TI.TicketID AND IM.IsDeleted=0 AND IM.DateReceived > ? AND IM.InboundMessageID NOT IN ")
								_T("(SELECT InboundMessageID FROM InboundMessageRead)) ")
								_T("UNION ")
								_T("(SELECT DISTINCT(TicketID) FROM TicketNotes TN WHERE TN.TicketID=TI.TicketID AND TN.DateCreated > ? AND TN.TicketNoteID NOT IN ")
								_T("(SELECT TicketNoteID FROM TicketNotesRead))) %s"), sOwned.c_str());	
				
			}	
		}
	}
	else
	{
		if ( !nUnreadMode )
		{
			if ( !GetIsAdmin() && g_ThreadPool.GetSharedObjects().m_bUseATC )
			{
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDPARAM_LONG(GetQuery(), nTicketBoxID);
				BINDPARAM_LONG(GetQuery(), nState);
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDCOL_LONG_NOLEN(GetQuery(), nTotalCount);
				
				sQuery.Format(_T("SELECT COUNT(TI.TicketID) FROM Tickets TI INNER JOIN AgentTicketCategories ATC ON TI.TicketCategoryID=ATC.TicketCategoryID ")
								_T("WHERE (ATC.AgentID=?) AND (TI.TicketBoxID=?) AND (TI.IsDeleted=0) AND (TI.TicketStateID>?) AND ")
								_T("TI.TicketID IN ")
								_T("((SELECT DISTINCT(TicketID) FROM InboundMessages IM WHERE IM.TicketID=TI.TicketID AND IM.IsDeleted=0 AND IM.InboundMessageID NOT IN ")
								_T("(SELECT InboundMessageID FROM InboundMessageRead WHERE AgentID=?)) ")
								_T("UNION ")
								_T("(SELECT DISTINCT(TicketID) FROM TicketNotes TN WHERE TN.TicketID=TI.TicketID AND TN.TicketNoteID NOT IN ")
								_T("(SELECT TicketNoteID FROM TicketNotesRead WHERE AgentID=?))) %s"), sOwned.c_str());	
				
			}
			else
			{
				BINDPARAM_LONG(GetQuery(), nTicketBoxID);
				BINDPARAM_LONG(GetQuery(), nState);
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDCOL_LONG_NOLEN(GetQuery(), nTotalCount);
				
				sQuery.Format(_T("SELECT COUNT(TI.TicketID) FROM Tickets TI WHERE (TI.TicketBoxID=?) AND ")
								_T("(TI.IsDeleted=0) AND (TI.TicketStateID>?) AND TI.TicketID IN ")
								_T("((SELECT DISTINCT(TicketID) FROM InboundMessages IM WHERE IM.TicketID=TI.TicketID AND IM.IsDeleted=0 AND IM.InboundMessageID NOT IN ")
								_T("(SELECT InboundMessageID FROM InboundMessageRead WHERE AgentID=?)) ")
								_T("UNION ")
								_T("(SELECT DISTINCT(TicketID) FROM TicketNotes TN WHERE TN.TicketID=TI.TicketID AND TN.TicketNoteID NOT IN ")
								_T("(SELECT TicketNoteID FROM TicketNotesRead WHERE AgentID=?))) %s"), sOwned.c_str());
				
			}
		}
		else
		{
				if ( !GetIsAdmin() && g_ThreadPool.GetSharedObjects().m_bUseATC )
			{
				BINDPARAM_LONG(GetQuery(), lAgentID);
				BINDPARAM_LONG(GetQuery(), nTicketBoxID);
				BINDPARAM_LONG(GetQuery(), nState);
				BINDCOL_LONG_NOLEN(GetQuery(), nTotalCount);
				
				sQuery.Format(_T("SELECT COUNT(TI.TicketID) FROM Tickets TI INNER JOIN AgentTicketCategories ATC ON TI.TicketCategoryID=ATC.TicketCategoryID ")
								_T("WHERE (ATC.AgentID=?) AND (TI.TicketBoxID=?) AND (TI.IsDeleted=0) AND (TI.TicketStateID>?) AND ")
								_T("TI.TicketID IN ")
								_T("((SELECT DISTINCT(TicketID) FROM InboundMessages IM WHERE IM.TicketID=TI.TicketID AND IM.IsDeleted=0 AND IM.InboundMessageID NOT IN ")
								_T("(SELECT InboundMessageID FROM InboundMessageRead)) ")
								_T("UNION ")
								_T("(SELECT DISTINCT(TicketID) FROM TicketNotes TN WHERE TN.TicketID=TI.TicketID AND TN.TicketNoteID NOT IN ")
								_T("(SELECT TicketNoteID FROM TicketNotesRead))) %s"), sOwned.c_str());
				
			}
			else
			{
				BINDPARAM_LONG(GetQuery(), nTicketBoxID);
				BINDPARAM_LONG(GetQuery(), nState);
				BINDCOL_LONG_NOLEN(GetQuery(), nTotalCount);
				
				sQuery.Format(_T("SELECT COUNT(TI.TicketID) FROM Tickets TI WHERE (TI.TicketBoxID=?) AND ")
								_T("(TI.IsDeleted=0) AND (TI.TicketStateID>?) AND TI.TicketID IN ")
								_T("((SELECT DISTINCT(TicketID) FROM InboundMessages IM WHERE IM.TicketID=TI.TicketID AND IM.IsDeleted=0 AND IM.InboundMessageID NOT IN ")
								_T("(SELECT InboundMessageID FROM InboundMessageRead)) ")
								_T("UNION ")
								_T("(SELECT DISTINCT(TicketID) FROM TicketNotes TN WHERE TN.TicketID=TI.TicketID AND TN.TicketNoteID NOT IN ")
								_T("(SELECT TicketNoteID FROM TicketNotesRead))) %s"), sOwned.c_str());	
				
			}
		}
	}

	GetQuery().Execute(sQuery.c_str());
	GetQuery().Fetch();
	
	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CTicketBoxView::GetUnreadMessageCountPublic TicketBoxID: %d AgentID: %d Flags: %d%d%d - Elapsed: %d", nTicketBoxID, lAgentID, nShowClosed, nShowOwned, nUnreadMode, m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	return nTotalCount;
	
}

void CTicketBoxView::ListMyFolders(int nAgentID)
{
	CUSTOMTICKETBOXVIEW_STRUCT ctbv;
	std::list<CUSTOMTICKETBOXVIEW_STRUCT> ctbvList;

	long m_lStart;
	long m_lEnd;
	long m_lElapsed;
	dca::String f;
	
	m_lStart = GetTickCount();

	GetQuery().Initialize();	
	BINDPARAM_LONG( GetQuery(), nAgentID);
	BINDCOL_LONG_NOLEN( GetQuery(), ctbv._nID);
	BINDCOL_TCHAR( GetQuery(), ctbv._sName);
	BINDCOL_LONG_NOLEN( GetQuery(), ctbv._nTypeID);
	BINDCOL_LONG_NOLEN( GetQuery(), ctbv._nAgentBoxID);
	BINDCOL_BIT_NOLEN( GetQuery(), ctbv._ucShowClosedItems);
	
	GetQuery().Execute( _T("SELECT Folders.FolderID, Folders.Name, TicketBoxViews.TicketBoxViewTypeID, TicketBoxViews.AgentBoxID, TicketBoxViews.ShowClosedItems FROM Folders INNER JOIN TicketBoxViews ON Folders.ParentID = TicketBoxViews.TicketBoxViewID WHERE (Folders.AgentID = ?)"));

	while ( GetQuery().Fetch() == S_OK )
	{
		ctbvList.push_back(ctbv);
	}

	std::list<CUSTOMTICKETBOXVIEW_STRUCT>::iterator iList;

	GetXMLGen().AddChildElem( _T("myfolders") );

	for(iList = ctbvList.begin();
		iList != ctbvList.end();
		iList++)
	{
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("folder") );
		GetXMLGen().AddChildAttrib( _T("id"), (*iList)._nID );

		int nOpenItems = GetTicketCount( GetQuery(), (*iList)._nTypeID, NULL, (*iList)._nAgentBoxID, 
				                     NULL, (*iList)._ucShowClosedItems, (*iList)._nID );

		int nReadItems = -1;

		if(nAgentID == GetAgentID())
		{
			//DcaTrace(_T("*  CTicketBoxView::ListMyFolders() - EnableMyTicketBoxRead [ %d ]"), _sEnableMyTickets.ToInt());

			if(_sEnableMyTickets.ToInt() != 0)
				nReadItems = GetUnreadMessageCountPrivate(nAgentID, (*iList)._ucShowClosedItems, (*iList)._nID);
		}
		else
		{
			//DcaTrace(_T("*  CTicketBoxView::ListMyFolders() - EnableAgentViewsRead [ %d ]"), _sEnableAgentViews.ToInt());

			if(_sEnableAgentViews.ToInt() != 0)
				nReadItems = GetUnreadMessageCountPrivate(nAgentID, (*iList)._ucShowClosedItems, (*iList)._nID);
		}

		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("ticketcount"), nOpenItems );
		GetXMLGen().AddChildElem( _T("unreadticketcount"), nReadItems );
		GetXMLGen().AddChildElem( _T("ownerid"), nAgentID );
		GetXMLGen().AddChildElem( _T("name"), (*iList)._sName );
		GetXMLGen().OutOfElem();

		GetXMLGen().OutOfElem();
	}

	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	//f.Format("CTicketBoxView::ListMyFolders AgentID: %d - Elapsed: %d", nAgentID,m_lElapsed );
	//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
}
