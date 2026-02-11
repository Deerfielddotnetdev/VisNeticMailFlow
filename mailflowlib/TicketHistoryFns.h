/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/TicketHistoryFns.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Ticket History Helper Functions
||              
\\*************************************************************************/

#pragma once

void THCreateTicket( CODBCQuery& query, int TicketID, int AgentID, int RoutingRuleID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID );
void THChangeTicketBox( CODBCQuery& query, int TicketID, int AgentID, int OldTicketBoxID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID );
void THChangePriority( CODBCQuery& query, int TicketID, int AgentID, int OldPriorityID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID );
void THChangeState( CODBCQuery& query, int TicketID, int AgentID, int OldStateID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID );
void THChangeCategory( CODBCQuery& query, int TicketID, int AgentID, int OldTicketCategoryID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID );
void THChangeOwner( CODBCQuery& query, int TicketID, int AgentID, int OldOwnerID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID );
void THEscalate( CODBCQuery& query, int TicketID, int AgentID, int OldOwnerID, int OldStateID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID );
void THAgeAlert( CODBCQuery& query, int TicketID, int OldPriorityID, int TicketBoxID );
void THAddInboundMsg( CODBCQuery& query, int TicketID, int AgentID, int InboundMsgID, int TicketBoxID );
void THAddOutboundMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID, int TicketBoxID );
void THDelInboundMsg( CODBCQuery& query, int TicketID, int AgentID, int InboundMsgID, int TicketBoxID );
void THDelOutboundMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID, int TicketBoxID );
void THRevokeOutboundMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID );
void THReleaseOutboundMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID );
void THReturnOutboundMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID );
void THAddNote( CODBCQuery& query, int TicketID, int AgentID, int NoteID );
void THDelNote( CODBCQuery& query, int TicketID, int AgentID, int NoteID );
void THRestoreTicket( CODBCQuery& query, int TicketID, int AgentID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID );
void THDeleteTicket( CODBCQuery& query, int TicketID, int AgentID, int TicketStateID, int TicketBoxID, int OwnerID, int PriorityID, int TicketCategoryID );
void THMergeTicket( CODBCQuery& query, int TicketID, int AgentID, int OldTicketID );
void THSaveMsg( CODBCQuery& query, int TicketID, int AgentID, int OutboundMsgID );
void THLinkTicket( CODBCQuery& query, int TicketID, int AgentID, tstring LinkName );
void THUnlinkTicket( CODBCQuery& query, int TicketID, int AgentID, tstring LinkName );
void THChangeTicketField( CODBCQuery& query, int TicketID, int AgentID, int TicketFieldID, tstring DataValue );
