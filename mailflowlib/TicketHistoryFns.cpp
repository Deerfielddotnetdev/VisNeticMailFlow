/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/TicketHistoryFns.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Ticket History Functions  
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketHistoryFns.h"

#include "QueryClasses.h"
#include "DateFns.h"


////////////////////////////////////////////////////////////////////////////////
// 
// THInsert
// 
////////////////////////////////////////////////////////////////////////////////
void THInsert( CODBCQuery& query, TTicketHistory& th )
{
	GetTimeStamp( th.m_DateTime );
	th.m_DateTimeLen = sizeof(TIMESTAMP_STRUCT);

	th.Insert( query );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THChangeTicketBox
// 
////////////////////////////////////////////////////////////////////////////////
void THChangeTicketBox( CODBCQuery& query, int TicketID, int AgentID, int OldTicketBoxID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_MODIFIED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 1;
	th.m_ID2 = OldTicketBoxID;
	th.m_TicketStateID = TicketStateID;
	th.m_TicketBoxID = TicketBoxID;
	th.m_OwnerID = OwnerID;
	th.m_PriorityID = PriorityID;
	th.m_TicketCategoryID = TicketCategoryID;

	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THChangePriority
// 
////////////////////////////////////////////////////////////////////////////////
void THChangePriority( CODBCQuery& query, int TicketID, int AgentID, int OldPriorityID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_MODIFIED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 4;
	th.m_ID2 = OldPriorityID;
	th.m_TicketStateID = TicketStateID;
	th.m_TicketBoxID = TicketBoxID;
	th.m_OwnerID = OwnerID;
	th.m_PriorityID = PriorityID;
	th.m_TicketCategoryID = TicketCategoryID;

	THInsert( query, th );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THChangeCategory
// 
////////////////////////////////////////////////////////////////////////////////
void THChangeCategory( CODBCQuery& query, int TicketID, int AgentID, int OldTicketCategoryID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_MODIFIED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 5;
	th.m_ID2 = OldTicketCategoryID;
	th.m_TicketStateID = TicketStateID;
	th.m_TicketBoxID = TicketBoxID;
	th.m_OwnerID = OwnerID;
	th.m_PriorityID = PriorityID;
	th.m_TicketCategoryID = TicketCategoryID;

	THInsert( query, th );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THChangeState
// 
////////////////////////////////////////////////////////////////////////////////
void THChangeState( CODBCQuery& query, int TicketID, int AgentID, int OldStateID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_MODIFIED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 3;
	th.m_ID2 = OldStateID;
	th.m_TicketStateID = TicketStateID;
	th.m_TicketBoxID = TicketBoxID;
	th.m_OwnerID = OwnerID;
	th.m_PriorityID = PriorityID;
	th.m_TicketCategoryID = TicketCategoryID;

	THInsert( query, th );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THChangeOwner
// 
////////////////////////////////////////////////////////////////////////////////
void THChangeOwner( CODBCQuery& query, int TicketID, int AgentID, int OldOwnerID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_MODIFIED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 2;
	th.m_ID2 = OldOwnerID;
	th.m_TicketStateID = TicketStateID;
	th.m_TicketBoxID = TicketBoxID;
	th.m_OwnerID = OwnerID;
	th.m_PriorityID = PriorityID;
	th.m_TicketCategoryID = TicketCategoryID;

	THInsert( query, th );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THEscalate
// 
////////////////////////////////////////////////////////////////////////////////
void THEscalate( CODBCQuery& query, int TicketID, int AgentID, int OldOwnerID, int OldStateID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_ESCALATED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = OldStateID;
	th.m_ID2 = OldOwnerID;
	th.m_TicketStateID = TicketStateID;
	th.m_TicketBoxID = TicketBoxID;
	th.m_OwnerID = OwnerID;
	th.m_PriorityID = PriorityID;
	th.m_TicketCategoryID = TicketCategoryID;

	THInsert( query, th );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THAgeAlert
// 
////////////////////////////////////////////////////////////////////////////////
void THAgeAlert( CODBCQuery& query, int TicketID, int OldPriorityID, int TicketBoxID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_AGE_ALERTED;
	th.m_TicketID = TicketID;
	th.m_ID1 = OldPriorityID;

	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THAddInboundMsg
// 
////////////////////////////////////////////////////////////////////////////////
void THAddInboundMsg( CODBCQuery& query, int TicketID, int AgentID, int InboundMsgID, int TicketBoxID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_ADD_MSG;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 1;
	th.m_ID2 = InboundMsgID;
	th.m_TicketBoxID = TicketBoxID;

	THInsert( query, th );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THAddOutboundMsg
// 
////////////////////////////////////////////////////////////////////////////////
void THAddOutboundMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID, int TicketBoxID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_ADD_MSG;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID2 = OutboundMsgID;
	th.m_TicketBoxID = TicketBoxID;

	THInsert( query, th );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THAddNote
// 
////////////////////////////////////////////////////////////////////////////////
void THAddNote( CODBCQuery& query, int TicketID, int AgentID, int NoteID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_ADD_NOTE;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 0;
	th.m_ID2 = NoteID;

	THInsert( query, th );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THDelInboundMsg
// 
////////////////////////////////////////////////////////////////////////////////
void THDelInboundMsg( CODBCQuery& query, int TicketID, int AgentID, int InboundMsgID, int TicketBoxID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_DEL_MSG;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 1;
	th.m_ID2 = InboundMsgID;
	th.m_TicketBoxID = TicketBoxID;

	THInsert( query, th );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THDelOutboundMsg
// 
////////////////////////////////////////////////////////////////////////////////
void THDelOutboundMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID, int TicketBoxID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_DEL_MSG;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID2 = OutboundMsgID;
	th.m_TicketBoxID = TicketBoxID;

	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THRevokeOutboundMsg
// 
////////////////////////////////////////////////////////////////////////////////
void THRevokeOutboundMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_REVOKE_MSG;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID2 = OutboundMsgID;

	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THReleaseOutboundMsg
// 
////////////////////////////////////////////////////////////////////////////////
void THReleaseOutboundMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_RELEASE_MSG;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID2 = OutboundMsgID;

	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THReturnOutboundMsg
// 
////////////////////////////////////////////////////////////////////////////////
void THReturnOutboundMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_RETURN_MSG;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID2 = OutboundMsgID;

	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THDelNote
// 
////////////////////////////////////////////////////////////////////////////////
void THDelNote( CODBCQuery& query, int TicketID, int AgentID, int NoteID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_DEL_NOTE;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 0;
	th.m_ID2 = NoteID;

	THInsert( query, th );
}


////////////////////////////////////////////////////////////////////////////////
// 
// THCreateTicket
// 
////////////////////////////////////////////////////////////////////////////////
void THCreateTicket( CODBCQuery& query, int TicketID, int AgentID, int RoutingRuleID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_CREATED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = RoutingRuleID;
	th.m_TicketStateID = TicketStateID;
	th.m_TicketBoxID = TicketBoxID;
	th.m_OwnerID = OwnerID;
	th.m_PriorityID = PriorityID;
	th.m_TicketCategoryID = TicketCategoryID;

	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THRestoreTicket
// 
////////////////////////////////////////////////////////////////////////////////
void THRestoreTicket( CODBCQuery& query, int TicketID, int AgentID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_RESTORED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 0;
	th.m_ID2 = 0;
	th.m_TicketStateID = TicketStateID;
	th.m_TicketBoxID = TicketBoxID;
	th.m_OwnerID = OwnerID;
	th.m_PriorityID = PriorityID;
	th.m_TicketCategoryID = TicketCategoryID;

	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THDeleteTicket
// 
////////////////////////////////////////////////////////////////////////////////
void THDeleteTicket( CODBCQuery& query, int TicketID, int AgentID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_DELETED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 0;
	th.m_ID2 = 0;
	th.m_TicketStateID = TicketStateID;
	th.m_TicketBoxID = TicketBoxID;
	th.m_OwnerID = OwnerID;
	th.m_PriorityID = PriorityID;
	th.m_TicketCategoryID = TicketCategoryID;

	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THMergeTicket
// 
////////////////////////////////////////////////////////////////////////////////
void THMergeTicket( CODBCQuery& query, int TicketID, int AgentID, int OldTicketID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_MERGED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = OldTicketID;
		
	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THLinkTicket
// 
////////////////////////////////////////////////////////////////////////////////
void THLinkTicket( CODBCQuery& query, int TicketID, int AgentID, tstring LinkName )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_LINKED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	_tcscpy(th.m_DataValue, LinkName.c_str());
		
	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THUnlinkTicket
// 
////////////////////////////////////////////////////////////////////////////////
void THUnlinkTicket( CODBCQuery& query, int TicketID, int AgentID, tstring LinkName )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_UNLINKED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	_tcscpy(th.m_DataValue, LinkName.c_str());
		
	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THAddOutboundMsg
// 
////////////////////////////////////////////////////////////////////////////////
void THSaveMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_SAVED_MSG;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID2 = OutboundMsgID;

	THInsert( query, th );
}

////////////////////////////////////////////////////////////////////////////////
// 
// THChangeTicketField
// 
////////////////////////////////////////////////////////////////////////////////
void THChangeTicketField( CODBCQuery& query, int TicketID, int AgentID, int TicketFieldID, tstring DataValue )
{
	TTicketHistory th;

	th.m_TicketActionID = EMS_TICKETACTIONID_MODIFIED;
	th.m_TicketID = TicketID;
	th.m_AgentID = AgentID;
	th.m_ID1 = 6;
	th.m_ID2 = TicketFieldID;
	_tcscpy(th.m_DataValue, DataValue.c_str());
		
	THInsert( query, th );
}
