/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/MessageList.cpp,v 1.2 2005/11/29 21:16:26 markm Exp $
||
||
||                                         
||  COMMENTS:	Generates a list of messages from a TicketBoxID   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "MessageList.h"
#include "DateFns.h"
#include "TicketBoxViewFns.h"
#include "TBVInfo.h"
#include "OutboundMessage.h"
#include "InboundMessage.h"
#include "TicketNotes.h"

/*---------------------------------------------------------------------------\             
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CMessageList::CMessageList(CISAPIData& ISAPIData) : CPagedList(ISAPIData)
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Called by the worker thread to handle the request
\*--------------------------------------------------------------------------*/
int CMessageList::Run(CURLAction &action)
{
	tstring sAction;
	int nInOut;

	if(GetISAPIData().GetURLLong( _T("Queue"), nInOut, true ))
	{
		// check for Admin
		if( GetSession().m_IsAdmin == false )
		{
			GetRoutingEngine().LogIt(	EMS_ISAPI_LOG_ERROR( EMS_LOG_SECURITY_VIOLATION, E_AccessDenied),
				EMS_ISAPI_ERROR_SECURITY_VIOLATION_ADMIN_ONLY,
				GetSession().m_AgentUserName.c_str(),
				GetISAPIData().m_sPage.c_str() );
				
			THROW_EMS_EXCEPTION_NOLOG( E_AccessDenied, CEMSString( EMS_STRING_ERROR_ACCESS_DENIED_ADMIN ) ); 
		}
		
		// read the action
		if ( GetISAPIData().GetXMLPost() )
		{
			GetISAPIData().GetXMLString( _T("Action"), sAction, true);
		}
		else
		{
			GetISAPIData().GetFormString( _T("Action"), sAction, true);
		}

		if ( sAction.length() )
		{
			if (sAction.compare(_T("remove")) == 0)
				return RemoveMessage(nInOut);
			
			if (sAction.compare( _T("revoke") ) == 0 )
				return RevokeFromQueue();

			if (sAction.compare( _T("queueit") ) == 0 )
				return PutInQueue();

			if (sAction.compare( _T("delete") ) == 0 )
			{
				if ( nInOut == 2 )
				{
					return DeleteOrphan();
				}
				else
				{
					return DeleteOutboundOrphan();
				}
			}

			if (sAction.compare( _T("deleteAll") ) == 0 )
			{
				if ( nInOut == 2 )
				{
					return DeleteAllOrphans();
				}
				else
				{
					return DeleteAllOutboundOrphans();
				}
			}
		}
		
		return ListQueue( nInOut );
	}

	// get the TicketBoxViewID
	GetISAPIData().GetURLLong( _T("TicketBoxView"), m_TBView.m_TicketBoxViewID );

	// get info about the TicketBoxView
	m_TBView.Query( GetQuery() );

	// does the view type use CMessageList
	switch(m_TBView.m_ViewTypeID) 
	{
	case EMS_OUTBOX:
	case EMS_APPROVALS:
	case EMS_SENT_ITEMS:
	case EMS_DRAFTS:
		{
		}
		break;
		
	default:
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}

	// read the action
	if ( GetISAPIData().GetXMLPost() )
	{
		GetISAPIData().GetXMLString( _T("Action"), sAction, true);
	}
	else
	{
		GetISAPIData().GetFormString( _T("Action"), sAction, true);
	}

	if ( sAction.length() )
	{
		if (sAction.compare(_T("sort")) == 0)
			return ChangeSortOrder();
		
		if (sAction.compare(_T("delete")) == 0)
			return DeleteMessage();

		if (sAction.compare( _T("revoke") ) == 0 )
			return RevokeMessage();

		if (sAction.compare( _T("release") ) == 0 )
			return ReleaseMessage();

		if (sAction.compare( _T("return") ) == 0 )
			return ReturnMessage();
	}
	
	// check security -- this is done here because the other functions check
	// security before calling List()
	RequireAgentRightLevel(EMS_OBJECT_TYPE_AGENT, m_TBView.m_AgentBoxID, EMS_READ_ACCESS);
	
	return List();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the messages in a TicketBoxView	              
\*--------------------------------------------------------------------------*/
int CMessageList::List()
{	
	// update the page count
	SetItemCount( m_TBView.GetItemCount( GetQuery() ) );
		
	// add page count information
	AddPageXML();
	
	// add ticketbox information
	GetXMLGen().AddChildElem( _T("TicketBoxItemList") );
	GetXMLGen().AddChildAttrib( _T("TicketBoxView"), m_TBView.m_TicketBoxViewID );
	
	GetXMLGen().AddChildAttrib( _T("SortBy"), GetXMLSortColumnName());
	GetXMLGen().AddChildAttrib( _T("SortAscending"), m_TBView.m_SortAsc);
	
	AddAgentName( _T("Agent"), m_TBView.m_AgentBoxID );
	GetXMLGen().AddChildAttrib( _T("TypeID"), m_TBView.m_ViewTypeID );
	AddTicketBoxViewTypeName( _T("TicketBoxName"), m_TBView.m_ViewTypeID );
	
	// format the query
	CEMSString sQuery;
	FormatQueryString(sQuery);
	
	// preform the query
	GetQuery().Initialize(GetMaxRowsPerPage(), sizeof(CMessageListItem));
	GetQuery().EnableScrollCursor();
	
	// allocate storage
	CMessageListItem* pArray = NULL;
	
	if ((pArray = new CMessageListItem[GetMaxRowsPerPage()]) == NULL)
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_ERROR_MEMORY) );  
	
	try
	{
		// execute the query
		GetQuery().Execute(sQuery.c_str());
		
		// bind
		BINDCOL_TCHAR(GetQuery(), pArray[0].m_Subject);
		BINDCOL_TCHAR_NOLEN(GetQuery(), pArray[0].m_EmailAddr);
		BINDCOL_TIME(GetQuery(), pArray[0].m_EmailDateTime);
		BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_MsgStateID);
		BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_Prority);
		BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_AttachmentCount);
		BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_MessageID);
		BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_TicketID);
		
		if (m_TBView.m_ViewTypeID == EMS_APPROVALS)
			BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_AgentID);
			BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_TicketBoxID);
			BINDCOL_TCHAR_NOLEN(GetQuery(), pArray[0].m_EmailAddrFrom);
			BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_TicketNoteCount);
		
		// fetch the data
		GetQuery().FetchScroll( SQL_FETCH_ABSOLUTE, GetStartRow() );
		
		// generate the XML
		GenMessageListXML(pArray);
		
		// close the cursor
		GetQuery().Reset();
	}
	catch(...)
	{
		if (pArray)
		{
			delete[] pArray;
			pArray = NULL;
		}
		
		throw;
	}
	
	if (pArray)
	{
		delete[] pArray;
		pArray = NULL;
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the messages in a queue	              
\*--------------------------------------------------------------------------*/
int CMessageList::ListQueue( int nInOut )
{	
	int nMsgs=0;
	CEMSString sSQL;

	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), nMsgs );
	if(nInOut == 1)
	{
		GetQuery().Execute( _T("SELECT COUNT(*)FROM InboundMessageQueue imq INNER JOIN InboundMessages im ")
							_T("ON imq.InboundMessageID = im.InboundMessageID ") );
	}
	else if (nInOut == 0)
	{
		GetQuery().Execute( _T("SELECT COUNT(*)FROM OutboundMessageQueue omq INNER JOIN OutboundMessages om ")
							_T("ON omq.OutboundMessageID = om.OutboundMessageID ") );
	}
	else if (nInOut == 3)
	{
		GetQuery().Execute( _T("SELECT COUNT(*)FROM OutboundMessages ")
							_T("WHERE OutboundMessages.TicketID=0 AND OutboundMessages.IsDeleted=0 ") );
	}
	else
	{
		GetQuery().Execute( _T("SELECT COUNT(*)FROM InboundMessages ")
							_T("WHERE TicketID=0 AND InboundMessageID NOT IN (SELECT InboundMessageID FROM InboundMessageQueue) ") );
	}

	GetQuery().Fetch();

	// update the page count
	SetItemCount( nMsgs );
		
	// add page count information
	AddPageXML();

	GetXMLGen().AddChildElem( _T("TicketBoxItemList") );
	GetXMLGen().AddChildAttrib( _T("QueueID"), nInOut );
	
	// preform the query
	GetQuery().Initialize(GetMaxRowsPerPage(), sizeof(CMessageListItem));
	GetQuery().EnableScrollCursor();
	
	// allocate storage
	CMessageListItem* pArray = NULL;
	
	if ((pArray = new CMessageListItem[GetMaxRowsPerPage()]) == NULL)
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_ERROR_MEMORY) );  
	
	try
	{
		// bind
		BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_MessageID);
		BINDCOL_TCHAR(GetQuery(), pArray[0].m_Subject);
		BINDCOL_TCHAR_NOLEN(GetQuery(), pArray[0].m_EmailAddrFrom);
		BINDCOL_TCHAR_NOLEN(GetQuery(), pArray[0].m_EmailAddr);
		BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_Prority);		
		BINDCOL_TIME(GetQuery(), pArray[0].m_EmailDateTime);
		BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_MsgStateID);
		BINDCOL_LONG_NOLEN(GetQuery(), pArray[0].m_TicketID);

		if(nInOut == 1)
		{
			sSQL.Format( _T("SELECT InboundMessages.InboundMessageID,Subject,EmailFrom,EmailPrimaryTo, ")
						 _T("PriorityID,DateReceived,0,TicketID ")
						 _T("FROM InboundMessageQueue INNER JOIN InboundMessages ON InboundMessageQueue.InboundMessageID = InboundMessages.InboundMessageID ")
						 _T("WHERE InboundMessages.IsDeleted=0 ")
						 _T("ORDER BY DateReceived") );
		}
		else if (nInOut == 0)
		{
			sSQL.Format( _T("SELECT OutboundMessages.OutboundMessageID,Subject,EmailFrom,EmailPrimaryTo, ")
						 _T("PriorityID,EmailDateTime,OutboundMessageStateID,TicketID ")
						 _T("FROM OutboundMessageQueue INNER JOIN OutboundMessages ON OutboundMessageQueue.OutboundMessageID = OutboundMessages.OutboundMessageID ")
						 _T("WHERE OutboundMessages.IsDeleted=0 ")
						 _T("ORDER BY EmailDateTime") );
		}
		else if (nInOut == 3)
		{
			sSQL.Format( _T("SELECT OutboundMessages.OutboundMessageID,Subject,EmailFrom,EmailPrimaryTo, ")
						 _T("PriorityID,EmailDateTime,OutboundMessageStateID,TicketID ")
						 _T("FROM OutboundMessages ")
						 _T("WHERE OutboundMessages.TicketID=0 AND OutboundMessages.IsDeleted=0 ")
						 _T("ORDER BY EmailDateTime") );
		}
		else
		{
			sSQL.Format( _T("SELECT InboundMessageID,Subject,EmailFrom,EmailPrimaryTo, ")
						 _T("PriorityID,DateReceived,0,TicketID ")
						 _T("FROM InboundMessages ")
						 _T("WHERE TicketID=0 AND IsDeleted=0 AND InboundMessageID NOT IN (SELECT InboundMessageID FROM InboundMessageQueue) ")
						 _T("ORDER BY DateReceived") );
		}
		
		GetQuery().Execute(sSQL.c_str());

		// fetch the data
		GetQuery().FetchScroll( SQL_FETCH_ABSOLUTE, GetStartRow() );
		
		// generate the XML
		GenMessageListXML(pArray);
		
		// close the cursor
		GetQuery().Reset();
	}
	catch(...)
	{
		if (pArray)
		{
			delete[] pArray;
			pArray = NULL;
		}
		
		throw;
	}
	
	if (pArray)
	{
		delete[] pArray;
		pArray = NULL;
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Prepares the query string for Outbox, Sent Items, and Drafts	              
\*--------------------------------------------------------------------------*/
void CMessageList::FormatQueryString(CEMSString& sQuery)
{
	LPCTSTR szMsgStateID = NULL;
	
	if (m_TBView.m_ViewTypeID == EMS_OUTBOX)
	{
		szMsgStateID = _T("> 2");
	}
	else if (m_TBView.m_ViewTypeID == EMS_SENT_ITEMS)
	{
		szMsgStateID = _T("= 2");
	}
	else if (m_TBView.m_ViewTypeID == EMS_APPROVALS)
	{
		szMsgStateID = _T("= 7");
	}
	else
	{
		szMsgStateID = _T("= 1");
	}
	
	if (m_TBView.m_ViewTypeID != EMS_APPROVALS)
	{
		sQuery.Format( _T("SELECT TOP %d Subject, EmailPrimaryTo, EmailDateTime, OutboundMessageStateID, PriorityID, ")
			_T("(SELECT COUNT(*) OutboundMessageID FROM OutboundMessageAttachments ")
			_T("WHERE OutboundMessageID = OutboundMessages.OutboundMessageID) as AttachCount, ")
			_T("OutboundMessageID, TicketID ")
			_T("FROM OutboundMessages ")
			_T("WHERE AgentID = %d AND OutboundMessageStateID %s AND IsDeleted = 0 ")
			_T("ORDER BY %s %s, OutboundMessageID"),
			GetEndRow(), m_TBView.m_AgentBoxID, szMsgStateID, GetOrderBy(), SORT_ORDER(m_TBView.m_SortAsc));
	}
	else
	{
		sQuery.Format( _T("SELECT TOP %d Subject, EmailPrimaryTo, EmailDateTime, OutboundMessageStateID, PriorityID, ")
			_T("(SELECT COUNT(*) OutboundMessageID FROM OutboundMessageAttachments ")
			_T("WHERE OutboundMessageID = OutboundMessages.OutboundMessageID) as AttachCount, ")
			_T("OutboundMessageID, TicketID, OutboundMessages.AgentID, (SELECT TicketBoxID FROM Tickets WHERE TicketID=OutboundMessages.TicketID) as TicketBoxID, EmailFrom, ")
			_T("(SELECT COUNT(*) TicketID FROM TicketNotes WHERE TicketNotes.TicketID=OutboundMessages.TicketID ) as TicketNoteCount ")
			_T("FROM OutboundMessages ")
			_T("WHERE OutboundMessageID IN (SELECT ActualID FROM Approvals WHERE ApprovalObjectTypeID=1 AND (ApproverAgentID=%d OR ApproverGroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=%d))) AND OutboundMessageStateID %s AND IsDeleted = 0 ")
			_T("ORDER BY %s %s, OutboundMessageID"),
			GetEndRow(), m_TBView.m_AgentBoxID, m_TBView.m_AgentBoxID, szMsgStateID, GetOrderBy(), SORT_ORDER(m_TBView.m_SortAsc));
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the XML for the list of messages
\*--------------------------------------------------------------------------*/
void CMessageList::GenMessageListXML(CMessageListItem* pArray)
{
	CEMSString sSubject;
	CEMSString sDateTime;

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;
		
	GetXMLGen().IntoElem();
	
	for(UINT i = 0; i < GetQuery().GetRowsFetched(); i++)
	{
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(pArray[i].m_EmailDateTime,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDateTime );
			}
			else
			{
				GetDateTimeString(pArray[i].m_EmailDateTime, pArray[i].m_EmailDateTimeLen, sDateTime );
			}
		}
		else
		{
			GetDateTimeString(pArray[i].m_EmailDateTime, pArray[i].m_EmailDateTimeLen, sDateTime );
		}
				
		GetXMLGen().AddChildElem(_T("Item"));
		GetXMLGen().AddChildAttrib( _T("ID"), pArray[i].m_MessageID );
		GetXMLGen().AddChildAttrib( _T("TicketID"), pArray[i].m_TicketID );
		sSubject.assign( pArray[i].m_SubjectLen ? pArray[i].m_Subject : _T("[No Subject]") );
		sSubject.EscapeHTML();
		GetXMLGen().AddChildAttrib( _T("Subject"), sSubject.c_str() );
		GetXMLGen().AddChildAttrib( _T("Date"), sDateTime.c_str() );
		GetXMLGen().AddChildAttrib( _T("To"), pArray[i].m_EmailAddr );
		
		GetXMLGen().AddChildAttrib( _T("State"), pArray[i].m_MsgStateID );
		AddOutboundMsgStateName( _T("StateName"), pArray[i].m_MsgStateID );
		
		GetXMLGen().AddChildAttrib( _T("Prority"), pArray[i].m_Prority );
		AddPriority( _T("PriorityName"), pArray[i].m_Prority );
		
		GetXMLGen().AddChildAttrib( _T("AttachCount"), pArray[i].m_AttachmentCount );

		GetXMLGen().AddChildAttrib( _T("From"), pArray[i].m_EmailAddrFrom );

		if (m_TBView.m_ViewTypeID == EMS_APPROVALS)
			AddAgentName( _T("AgentName"), pArray[i].m_AgentID );
			AddTicketBoxName( _T("TBName"), pArray[i].m_TicketBoxID );			
			GetXMLGen().AddChildAttrib( _T("NoteCount"), pArray[i].m_TicketNoteCount );

	}
	
	GetXMLGen().OutOfElem();
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when the agent deletes a message from Inbound Orphans   
\*--------------------------------------------------------------------------------------------*/
int CMessageList::DeleteOrphan()
{
	CInboundMessage InMsg(m_ISAPIData);
	CEMSString sMsgIDs;
	int nMsgID;	
	bool bRefresh = false;
	
	GetISAPIData().GetXMLString( _T("selectId"), sMsgIDs);
	
	while ( sMsgIDs.CDLGetNextInt(nMsgID) )
	{	
		InMsg.m_InboundMessageID = nMsgID;
	
		// delete the outbound message
		InMsg.Delete();

		if ( !bRefresh )
		{
			// if we were able to delete a ticket
			// add the flag to refresh the page
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
	}
	
	// update the item and page count
	// SetItemCount( m_TBView.GetItemCount( GetQuery() ) );
	
	return ListQueue(2);
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when the agent deletes all messages from Inbound Orphans   
\*--------------------------------------------------------------------------------------------*/
int CMessageList::DeleteAllOrphans()
{
	TIMESTAMP_STRUCT time;
	GetTimeStamp(time);
	
	GetQuery().Initialize();
		
	BINDPARAM_TIME_NOLEN( GetQuery(), time );
	BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		
	GetQuery().Execute(_T("UPDATE InboundMessages SET IsDeleted = 1, DeletedTime = ?, DeletedBy = ? WHERE TicketID=0 AND IsDeleted=0 AND InboundMessageID NOT IN (SELECT InboundMessageID FROM InboundMessageQueue)"));
	
	GetXMLGen().AddChildElem( _T("Refresh") );
	return ListQueue(2);
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when the agent deletes a message from Outbound Orphans  
\*--------------------------------------------------------------------------------------------*/
int CMessageList::DeleteOutboundOrphan()
{
	COutboundMessage OutMsg(m_ISAPIData);
	CEMSString sMsgIDs;
	int nMsgID;	
	bool bRefresh = false;
	
	GetISAPIData().GetXMLString( _T("selectId"), sMsgIDs);
	
	while ( sMsgIDs.CDLGetNextInt(nMsgID) )
	{	
		OutMsg.m_OutboundMessageID = nMsgID;
	
		// delete the outbound message
		OutMsg.DeleteOrphan();

		if ( !bRefresh )
		{
			// if we were able to delete a ticket
			// add the flag to refresh the page
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
	}
	
	return ListQueue(3);
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when the agent deletes all messages from Outbound Orphans   
\*--------------------------------------------------------------------------------------------*/
int CMessageList::DeleteAllOutboundOrphans()
{
	int nOutboundMessageID;
	list<unsigned int> MessageIDList;
	list<unsigned int>::iterator iter;
	COutboundMessage OutMsg(m_ISAPIData);
	bool bRefresh = false;
	CEMSString sSQL;
	
	sSQL.Format( _T("SELECT OutboundMessageID ")
						 _T("FROM OutboundMessages ")
						 _T("WHERE TicketID=0 AND IsDeleted=0") );
	
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), nOutboundMessageID );
	GetQuery().Execute(sSQL.c_str());

	while( GetQuery().Fetch() == S_OK )
	{
		MessageIDList.push_back( nOutboundMessageID );
	}
	
	for ( iter = MessageIDList.begin(); iter != MessageIDList.end(); iter++ )
	{
		OutMsg.m_OutboundMessageID = *iter;	
		OutMsg.DeleteOrphan();

		if ( !bRefresh )
		{
			// if we were able to delete a ticket
			// add the flag to refresh the page
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
	}
	
	return ListQueue(3);
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when the agent deletes a message    
\*--------------------------------------------------------------------------------------------*/
int CMessageList::DeleteMessage()
{
	COutboundMessage Msg(m_ISAPIData);
	CEMSString sMsgIDs;
	int nMsgID;	
	bool bRefresh = false;
	
	GetISAPIData().GetXMLString( _T("selectId"), sMsgIDs);
	
	while ( sMsgIDs.CDLGetNextInt(nMsgID) )
	{	
		Msg.m_OutboundMessageID = nMsgID;
	
		// delete the outbound message
		Msg.Delete();

		if ( !bRefresh )
		{
			// if we were able to delete a ticket
			// add the flag to refresh the page
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
	}
	
	// update the item and page count
	// SetItemCount( m_TBView.GetItemCount( GetQuery() ) );
	
	return List();
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when the agent revokes a message    
\*--------------------------------------------------------------------------------------------*/
int CMessageList::RevokeMessage()
{
	COutboundMessage Msg(m_ISAPIData);
	CEMSString sMsgIDs;
	int nMsgID;
	bool bRefresh = false;

	GetISAPIData().GetXMLString( _T("selectId"), sMsgIDs);
	
	while ( sMsgIDs.CDLGetNextInt(nMsgID) )
	{	
		Msg.m_OutboundMessageID = nMsgID;
	
		// revoke the outbound message
		Msg.Revoke();

		if ( !bRefresh )
		{
			// if we were able to revoke the message
			// add the flag to refresh the page
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
	}
	
	// update the item and page count
	// SetItemCount( m_TBView.GetItemCount( GetQuery() ) );
	
	return List();
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when the admin revokes a message from the outbound queue    
\*--------------------------------------------------------------------------------------------*/
int CMessageList::RevokeFromQueue()
{
	COutboundMessage Msg(m_ISAPIData);
	CEMSString sMsgIDs;
	int nMsgID;
	bool bRefresh = false;

	GetISAPIData().GetXMLString( _T("selectId"), sMsgIDs);
	
	while ( sMsgIDs.CDLGetNextInt(nMsgID) )
	{	
		Msg.m_OutboundMessageID = nMsgID;
	
		// revoke the outbound message
		Msg.Revoke();

		if ( !bRefresh )
		{
			// if we were able to revoke the message
			// add the flag to refresh the page
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
	}
	
	return ListQueue(0);
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when the admin removes a message from the inbound queue   
\*--------------------------------------------------------------------------------------------*/
int CMessageList::RemoveMessage( int nInOut )
{
	CEMSString sMsgIDs;
	int nMsgID;
	bool bRefresh = false;

	GetISAPIData().GetXMLString( _T("selectId"), sMsgIDs);
	
	while ( sMsgIDs.CDLGetNextInt(nMsgID) )
	{	
		GetQuery().Initialize();		
		BINDPARAM_LONG(GetQuery(), nMsgID);
		if(nInOut)
		{
			GetQuery().Execute(_T("DELETE FROM InboundMessageQueue WHERE InboundMessageID=? "));
		}
		else
		{
			GetQuery().Execute(_T("DELETE FROM OutboundMessageQueue WHERE OutboundMessageID=? "));
		}
	}
	
	return ListQueue(1);
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when the admin requeues messages from inbound orphans   
\*--------------------------------------------------------------------------------------------*/
int CMessageList::PutInQueue()
{
	CEMSString sMsgIDs;
	int nMsgID;
	bool bRefresh = false;

	GetISAPIData().GetXMLString( _T("selectId"), sMsgIDs);
	
	while ( sMsgIDs.CDLGetNextInt(nMsgID) )
	{	
		GetQuery().Initialize();		
		BINDPARAM_LONG(GetQuery(), nMsgID);
		GetQuery().Execute(_T("INSERT INTO InboundMessageQueue (InboundMessageID) VALUES (?) "));
	}
	
	return ListQueue(1);
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when an approver releases a message    
\*--------------------------------------------------------------------------------------------*/
int CMessageList::ReleaseMessage()
{
	COutboundMessage Msg(m_ISAPIData);
	CEMSString sMsgIDs;
	int nMsgID;
	bool bRefresh = false;

	GetISAPIData().GetXMLString( _T("selectId"), sMsgIDs);
	
	while ( sMsgIDs.CDLGetNextInt(nMsgID) )
	{	
		Msg.m_OutboundMessageID = nMsgID;
	
		// release the outbound message
		Msg.Release();

		if ( !bRefresh )
		{
			// if we were able to release the message
			// add the flag to refresh the page
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
	}
	
	// update the item and page count
	// SetItemCount( m_TBView.GetItemCount( GetQuery() ) );
	
	return List();
}

/*--------------------------------------------------------------------------------------------\                     
||  Comments:	Called when an approver returns a message    
\*--------------------------------------------------------------------------------------------*/
int CMessageList::ReturnMessage()
{
	COutboundMessage Msg(m_ISAPIData);
	CEMSString sMsgIDs;
	int nMsgID;
	bool bRefresh = false;

	GetISAPIData().GetXMLString( _T("selectId"), sMsgIDs);
	
	while ( sMsgIDs.CDLGetNextInt(nMsgID) )
	{	
		Msg.m_OutboundMessageID = nMsgID;
	
		// delete the outbound message
		Msg.Return();

		if ( !bRefresh )
		{
			// if we were able to return the message
			// add the flag to refresh the page
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
	}
	
	// update the item and page count
	// SetItemCount( m_TBView.GetItemCount( GetQuery() ) );
	
	return List();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Called when the agent changes the sort order for the view	              
\*--------------------------------------------------------------------------*/
int CMessageList::ChangeSortOrder()
{	
	// get the sort field
	tstring sSortBy;
	GetISAPIData().GetFormString( _T("SortBy"), sSortBy );
	m_TBView.m_SortField = GetSortField(sSortBy);
	
	// get the sort order
	GetISAPIData().GetFormLong( _T("SortAscending"), m_TBView.m_SortAsc );
	
	// change the sort order
	GetQuery().Initialize();
	
	BINDPARAM_LONG(GetQuery(), m_TBView.m_SortField);
	BINDPARAM_BIT(GetQuery(), m_TBView.m_SortAsc);
	BINDPARAM_LONG(GetQuery(), m_TBView.m_TicketBoxViewID);
	
	GetQuery().Execute(_T("UPDATE TicketBoxViews SET SortField = ?, SortAscending = ? WHERE TicketBoxViewID = ?"));
	
	
	// now list the messages
	return List();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a string for use in the ORDER BY clause
||				from the SortFieldID             
\*--------------------------------------------------------------------------*/
LPCTSTR CMessageList::GetOrderBy()
{
	switch (m_TBView.m_SortField)
	{
	case EMS_COLUMN_SUBJECT:
		return _T("Subject");
		
	case EMS_COLUMN_TO:
		return _T("EmailPrimaryTo");
		
	case EMS_COLUMN_DATE:
		return _T("EmailDateTime");
		
	case EMS_COLUMN_PRIORITY:
		return _T("PriorityID");
		
	case EMS_COLUMN_STATE:
		return _T("OutboundMessageStateID");
		
	case EMS_COLUMN_ATTACH_COUNT:
		return _T("AttachCount");
		
	case EMS_COLUMN_TICKETID:
		return _T("TicketID");
		
	default:
		return _T("EmailDateTime");
	}
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Verifies that the sort column is valid for this view type and
||				returns the name of the sort column as expected by XML
\*--------------------------------------------------------------------------*/
LPCTSTR CMessageList::GetXMLSortColumnName(void)
{
	switch (m_TBView.m_SortField)
	{
		
	case EMS_COLUMN_SUBJECT:
	case EMS_COLUMN_DATE:
	case EMS_COLUMN_TO:
	case EMS_COLUMN_STATE:
	case EMS_COLUMN_PRIORITY:
	case EMS_COLUMN_ATTACH_COUNT:
	case EMS_COLUMN_AGENT:
	case EMS_COLUMN_FROM:
	case EMS_COLUMN_TICKETBOX:
	case EMS_COLUMN_TICKETID:
		break;
		
	case EMS_COLUMN_CONTACTS:
		m_TBView.m_SortField = EMS_COLUMN_TO;
		break;
		
		// if the sort column isn't valid, reset it to the default
	default:
		
		m_TBView.m_SortField = EMS_COLUMN_DATE;
		m_TBView.m_SortAsc = FALSE;
	}
	
	return GetXMLColumnName(m_TBView.m_SortField);
}
