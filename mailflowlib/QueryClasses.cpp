////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMLIBRARY/QueryClasses.cpp,v 1.2.2.2 2006/01/11 16:41:48 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// ODBC Query classes derived from the data classes
//
// Original Filename: MailFlow-Schema.xml
//
// Generated: Fri Mar 28 09:06:37 2003

//
////////////////////////////////////////////////////////////////////////////////
//
// NOTE: These classes are non-virtual. This means that if you derive a class
//       from them, you cannot use the base class pointer to operate on the
//       class. This shouldn't be a big deal as these are meant to be data
//       classes only. (STB)
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "QueryClasses.h"
#include "DateFns.h"

/* class TAccessControl - 
		Controls access to various objects from specific agents and groups.
	 */
void TAccessControl::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_GroupID );
	BINDPARAM_TINYINT( query, m_AccessLevel );
	BINDPARAM_LONG( query, m_ObjectID );
	query.Execute( _T("INSERT INTO AccessControl ")
	               _T("(AgentID,GroupID,AccessLevel,ObjectID) ")
	               _T("VALUES")
	               _T("(?,?,?,?)") );
	m_AccessControlID = query.GetLastInsertedID();
}

int TAccessControl::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_GroupID );
	BINDPARAM_TINYINT( query, m_AccessLevel );
	BINDPARAM_LONG( query, m_ObjectID );
	BINDPARAM_LONG( query, m_AccessControlID );
	query.Execute( _T("UPDATE AccessControl ")
	               _T("SET AgentID=?,GroupID=?,AccessLevel=?,ObjectID=? ")
	               _T("WHERE AccessControlID=?") );
	return query.GetRowCount();
}

int TAccessControl::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AccessControlID );
	query.Execute( _T("DELETE FROM AccessControl ")
	               _T("WHERE AccessControlID=?") );
	return query.GetRowCount();
}

int TAccessControl::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_TINYINT( query, m_AccessLevel );
	BINDCOL_LONG( query, m_ObjectID );
	BINDPARAM_LONG( query, m_AccessControlID );
	query.Execute( _T("SELECT AgentID,GroupID,AccessLevel,ObjectID ")
	               _T("FROM AccessControl ")
	               _T("WHERE AccessControlID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAccessControl::GetLongData( CODBCQuery& query )
{
}

void TAccessControl::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AccessControlID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_TINYINT( query, m_AccessLevel );
	BINDCOL_LONG( query, m_ObjectID );
	query.Execute( _T("SELECT AccessControlID,AgentID,GroupID,AccessLevel,ObjectID ")
	               _T("FROM AccessControl ")
	               _T("ORDER BY AccessControlID") );
}

/* class TAgentGroupings - 
		Segments agents into logical groups.
	 */
void TAgentGroupings::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_GroupID );
	query.Execute( _T("INSERT INTO AgentGroupings ")
	               _T("(AgentID,GroupID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_AgentGroupingID = query.GetLastInsertedID();
}

int TAgentGroupings::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_GroupID );
	BINDPARAM_LONG( query, m_AgentGroupingID );
	query.Execute( _T("UPDATE AgentGroupings ")
	               _T("SET AgentID=?,GroupID=? ")
	               _T("WHERE AgentGroupingID=?") );
	return query.GetRowCount();
}

int TAgentGroupings::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentGroupingID );
	query.Execute( _T("DELETE FROM AgentGroupings ")
	               _T("WHERE AgentGroupingID=?") );
	return query.GetRowCount();
}

int TAgentGroupings::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_GroupID );
	BINDPARAM_LONG( query, m_AgentGroupingID );
	query.Execute( _T("SELECT AgentID,GroupID ")
	               _T("FROM AgentGroupings ")
	               _T("WHERE AgentGroupingID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAgentGroupings::GetLongData( CODBCQuery& query )
{
}

void TAgentGroupings::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AgentGroupingID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_GroupID );
	query.Execute( _T("SELECT AgentGroupingID,AgentID,GroupID ")
	               _T("FROM AgentGroupings ")
	               _T("ORDER BY AgentGroupingID") );
}

/* class TAgents - 
		Agents are users of the system. Agents can represent 
		any level of user (e.g. an agent, supervisor, etc.).
	 */
void TAgents::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_LoginName );
	BINDPARAM_TCHAR( query, m_Password );
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_ObjectID );
	BINDPARAM_LONG( query, m_DefaultAgentAddressID );
	BINDPARAM_LONG( query, m_MaxReportRowsPerPage );
	BINDPARAM_BIT( query, m_StdResponseApprovalsRequired );
	BINDPARAM_BIT( query, m_MessageApprovalsRequired );
	BINDPARAM_BIT( query, m_QuoteMsgInReply );
	BINDPARAM_TCHAR( query, m_ReplyQuotedPrefix );
	BINDPARAM_TCHAR( query, m_NTDomain );
	BINDPARAM_LONG( query, m_IsNTUser );
	BINDPARAM_LONG( query, m_StyleSheetID );
	BINDPARAM_BIT( query, m_IsDeleted );
	BINDPARAM_BIT( query, m_UseIM );
	BINDPARAM_BIT( query, m_NewMessageFormat );
	BINDPARAM_LONG( query, m_EscalateToAgentID );
	BINDPARAM_BIT( query, m_RequireGetOldest );
	BINDPARAM_BIT( query, m_HasLoggedIn );
	BINDPARAM_LONG( query, m_DefaultSignatureID );
	BINDPARAM_BIT( query, m_ForceSpellCheck );
	BINDPARAM_BIT( query, m_SignatureTopReply );
	BINDPARAM_BIT( query, m_ShowMessagesInbound );
	BINDPARAM_BIT( query, m_UsePreviewPane );
	BINDPARAM_TINYINT( query, m_CloseTicket );
	BINDPARAM_TINYINT( query, m_RouteToInbox );
	BINDPARAM_LONG( query, m_OutboxHoldTime );
	BINDPARAM_BIT( query, m_UseMarkAsRead );
	BINDPARAM_LONG( query, m_MarkAsReadSeconds);
	BINDPARAM_LONG( query, m_OutboundApprovalFromID );
	BINDPARAM_BIT( query, m_UseAutoFill );
	BINDPARAM_LONG( query, m_DefaultTicketBoxID );
	BINDPARAM_LONG( query, m_DefaultTicketDblClick );
	BINDPARAM_LONG( query, m_ReadReceipt );
	BINDPARAM_LONG( query, m_DeliveryConfirmation );
	BINDPARAM_LONG( query, m_StatusID );
	BINDPARAM_TCHAR( query, m_StatusText );	
	BINDPARAM_LONG( query, m_AutoStatusTypeID );
	BINDPARAM_LONG( query, m_AutoStatusMin );
	BINDPARAM_LONG( query, m_LoginStatusID );
	BINDPARAM_LONG( query, m_LogoutStatusID );
	BINDPARAM_TCHAR( query, m_OnlineText );	
	BINDPARAM_TCHAR( query, m_AwayText );	
	BINDPARAM_TCHAR( query, m_NotAvailText );	
	BINDPARAM_TCHAR( query, m_DndText );	
	BINDPARAM_TCHAR( query, m_OfflineText );	
	BINDPARAM_TCHAR( query, m_OooText );	
	BINDPARAM_LONG( query, m_TimeZoneID);
	BINDPARAM_LONG( query, m_DictionaryID );
	BINDPARAM_BIT( query, m_DictionaryPrompt );
	query.Execute( _T("INSERT INTO Agents ")
	               _T("(LoginName,Password,Name,IsEnabled,ObjectID,DefaultAgentAddressID, ")
				   _T("MaxReportRowsPerPage,StdResponseApprovalsRequired,MessageApprovalsRequired, ")
				   _T("QuoteMsgInReply,ReplyQuotedPrefix,NTDomain,IsNTUser,StyleSheetID,IsDeleted, ")
				   _T("UseIM,NewMessageFormat,EscalateToAgentID,RequireGetOldest,HasLoggedIn,DefaultSignatureID, ")
				   _T("ForceSpellCheck,SignatureTopReply,ShowMessagesInbound,UsePreviewPane,CloseTicket, ")
				   _T("RouteToInbox,OutboxHoldTime,UseMarkAsRead,MarkAsReadSeconds,OutboundApprovalFromID, ")
				   _T("UseAutoFill,DefaultTicketBoxID,DefaultTicketDblClick,ReadReceipt,DeliveryConfirmation, ")
				   _T("StatusID,StatusText,AutoStatusTypeID,AutoStatusMin,LoginStatusID,LogoutStatusID, ")
				   _T("OnlineText,AwayText,NotAvailText,DndText,OfflineText,OooText,TimeZoneID,DictionaryID,DictionaryPrompt) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_AgentID = query.GetLastInsertedID();
}

int TAgents::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_LoginName );
	BINDPARAM_TCHAR( query, m_Password );
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_DefaultAgentAddressID );
	BINDPARAM_LONG( query, m_MaxReportRowsPerPage );
	BINDPARAM_BIT( query, m_StdResponseApprovalsRequired );
	BINDPARAM_BIT( query, m_MessageApprovalsRequired );
	BINDPARAM_BIT( query, m_QuoteMsgInReply );
	BINDPARAM_TCHAR( query, m_ReplyQuotedPrefix );
	BINDPARAM_TCHAR( query, m_NTDomain );
	BINDPARAM_LONG( query, m_IsNTUser );
	BINDPARAM_LONG( query, m_StyleSheetID );
	BINDPARAM_BIT( query, m_IsDeleted );
	BINDPARAM_BIT( query, m_UseIM );
	BINDPARAM_BIT( query, m_NewMessageFormat );
	BINDPARAM_LONG( query, m_EscalateToAgentID );
	BINDPARAM_BIT( query, m_RequireGetOldest );
	BINDPARAM_BIT( query, m_HasLoggedIn );
	BINDPARAM_LONG( query, m_DefaultSignatureID );
	BINDPARAM_BIT( query, m_ForceSpellCheck );
	BINDPARAM_BIT( query, m_SignatureTopReply );
	BINDPARAM_BIT( query, m_ShowMessagesInbound );
	BINDPARAM_BIT( query, m_UsePreviewPane );
	BINDPARAM_TINYINT( query, m_CloseTicket );
	BINDPARAM_TINYINT( query, m_RouteToInbox );
	BINDPARAM_LONG( query, m_OutboxHoldTime );
	BINDPARAM_LONG( query, m_UseMarkAsRead );
	BINDPARAM_LONG( query, m_MarkAsReadSeconds );
	BINDPARAM_LONG( query, m_OutboundApprovalFromID );
	BINDPARAM_BIT( query, m_UseAutoFill );
	BINDPARAM_LONG( query, m_DefaultTicketBoxID );
	BINDPARAM_LONG( query, m_DefaultTicketDblClick );
	BINDPARAM_LONG( query, m_ReadReceipt );
	BINDPARAM_LONG( query, m_DeliveryConfirmation );
	BINDPARAM_LONG( query, m_StatusID );
	BINDPARAM_TCHAR( query, m_StatusText );	
	BINDPARAM_LONG( query, m_AutoStatusTypeID );
	BINDPARAM_LONG( query, m_AutoStatusMin );
	BINDPARAM_LONG( query, m_LoginStatusID );
	BINDPARAM_LONG( query, m_LogoutStatusID );
	BINDPARAM_TCHAR( query, m_OnlineText );	
	BINDPARAM_TCHAR( query, m_AwayText );	
	BINDPARAM_TCHAR( query, m_NotAvailText );	
	BINDPARAM_TCHAR( query, m_DndText );	
	BINDPARAM_TCHAR( query, m_OfflineText );	
	BINDPARAM_TCHAR( query, m_OooText );	
	BINDPARAM_LONG( query, m_TimeZoneID);
	BINDPARAM_LONG( query, m_DictionaryID );
	BINDPARAM_BIT( query, m_DictionaryPrompt );
	BINDPARAM_LONG( query, m_AgentID );
	query.Execute( _T("UPDATE Agents ")
	               _T("SET LoginName=?,Password=?,Name=?,IsEnabled=?,DefaultAgentAddressID=?, ")
				   _T("MaxReportRowsPerPage=?,StdResponseApprovalsRequired=?,MessageApprovalsRequired=?, ")
				   _T("QuoteMsgInReply=?,ReplyQuotedPrefix=?,NTDomain=?,IsNTUser=?,StyleSheetID=?,IsDeleted=?, ")
				   _T("UseIM=?,NewMessageFormat=?,EscalateToAgentID=?,RequireGetOldest=?,HasLoggedIn=?, ")
				   _T("DefaultSignatureID=?,ForceSpellCheck=?,SignatureTopReply=?,ShowMessagesInbound=?, ")
				   _T("UsePreviewPane=?,CloseTicket=?,RouteToInbox=?,OutboxHoldTime=?,UseMarkAsRead=?, ")
				   _T("MarkAsReadSeconds=?,OutboundApprovalFromID=?,UseAutoFill=?,DefaultTicketBoxID=?, ")
				   _T("DefaultTicketDblClick=?,ReadReceipt=?,DeliveryConfirmation=?,StatusID=?, ")
				   _T("StatusText=?,AutoStatusTypeID=?,AutoStatusMin=?,LoginStatusID=?,LogoutStatusID=?, ")
				   _T("OnlineText=?,AwayText=?,NotAvailText=?,DndText=?,OfflineText=?,OooText=?,TimeZoneID=?,DictionaryID=?,DictionaryPrompt=? ")
	               _T("WHERE AgentID=?") );
	return query.GetRowCount();
}

int TAgents::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	query.Execute( _T("DELETE FROM Agents ")
	               _T("WHERE AgentID=?") );
	return query.GetRowCount();
}

int TAgents::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_LoginName );
	BINDCOL_TCHAR( query, m_Password );
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_LONG( query, m_DefaultAgentAddressID );
	BINDCOL_LONG( query, m_MaxReportRowsPerPage );
	BINDCOL_BIT( query, m_StdResponseApprovalsRequired );
	BINDCOL_BIT( query, m_MessageApprovalsRequired );
	BINDCOL_BIT( query, m_QuoteMsgInReply );
	BINDCOL_TCHAR( query, m_ReplyQuotedPrefix );
	BINDCOL_TCHAR( query, m_NTDomain );
	BINDCOL_LONG( query, m_IsNTUser );
	BINDCOL_LONG( query, m_StyleSheetID );
	BINDCOL_BIT( query, m_IsDeleted );
	BINDCOL_BIT( query, m_UseIM );
	BINDCOL_BIT( query, m_NewMessageFormat );
	BINDCOL_LONG( query, m_EscalateToAgentID );
	BINDCOL_BIT( query, m_RequireGetOldest );
	BINDCOL_BIT( query, m_HasLoggedIn );
	BINDCOL_LONG( query, m_DefaultSignatureID );
	BINDCOL_BIT( query, m_ForceSpellCheck );
	BINDCOL_BIT( query, m_SignatureTopReply );
	BINDCOL_BIT( query, m_ShowMessagesInbound );
	BINDCOL_BIT( query, m_UsePreviewPane );
	BINDCOL_TINYINT( query, m_CloseTicket );
	BINDCOL_TINYINT( query, m_RouteToInbox );
	BINDCOL_LONG( query, m_OutboxHoldTime );
	BINDCOL_BIT( query, m_UseMarkAsRead );
	BINDCOL_LONG( query, m_MarkAsReadSeconds );
	BINDCOL_LONG( query, m_OutboundApprovalFromID );
	BINDCOL_BIT( query, m_UseAutoFill );
	BINDCOL_LONG( query, m_DefaultTicketBoxID );
	BINDCOL_LONG( query, m_DefaultTicketDblClick );
	BINDCOL_LONG( query, m_ReadReceipt );
	BINDCOL_LONG( query, m_DeliveryConfirmation );
	BINDCOL_LONG( query, m_StatusID );
	BINDCOL_TCHAR( query, m_StatusText );
	BINDCOL_LONG( query, m_AutoStatusTypeID );
	BINDCOL_LONG( query, m_AutoStatusMin );
	BINDCOL_LONG( query, m_LoginStatusID );
	BINDCOL_LONG( query, m_LogoutStatusID );
	BINDCOL_TCHAR( query, m_OnlineText );
	BINDCOL_TCHAR( query, m_AwayText );
	BINDCOL_TCHAR( query, m_NotAvailText );
	BINDCOL_TCHAR( query, m_DndText );
	BINDCOL_TCHAR( query, m_OfflineText );
	BINDCOL_TCHAR( query, m_OooText );	
	BINDCOL_LONG( query, m_TimeZoneID);
	BINDCOL_LONG( query, m_DictionaryID );
	BINDCOL_BIT( query, m_DictionaryPrompt );
	BINDPARAM_LONG( query, m_AgentID );
	query.Execute( _T("SELECT LoginName,Password,Name,IsEnabled,ObjectID,DefaultAgentAddressID, ")
				   _T("MaxReportRowsPerPage,StdResponseApprovalsRequired,MessageApprovalsRequired, ")
				   _T("QuoteMsgInReply,ReplyQuotedPrefix,NTDomain,IsNTUser,StyleSheetID,IsDeleted,UseIM, ")
				   _T("NewMessageFormat,EscalateToAgentID,RequireGetOldest,HasLoggedIn,DefaultSignatureID, ")
				   _T("ForceSpellCheck,SignatureTopReply,ShowMessagesInbound,UsePreviewPane,CloseTicket, ")
				   _T("RouteToInbox,OutboxHoldTime,UseMarkAsRead,MarkAsReadSeconds,OutboundApprovalFromID, ")
				   _T("UseAutoFill,DefaultTicketBoxID,DefaultTicketDblClick,ReadReceipt,DeliveryConfirmation, ")
				   _T("StatusID,StatusText,AutoStatusTypeID,AutoStatusMin,LoginStatusID,LogoutStatusID, ")
				   _T("OnlineText,AwayText,NotAvailText,DndText,OfflineText,OooText,TimeZoneID,DictionaryID,DictionaryPrompt ")
	               _T("FROM Agents ")
	               _T("WHERE AgentID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAgents::GetLongData( CODBCQuery& query )
{
}

void TAgents::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TCHAR( query, m_LoginName );
	BINDCOL_TCHAR( query, m_Password );
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_LONG( query, m_DefaultAgentAddressID );
	BINDCOL_LONG( query, m_MaxReportRowsPerPage );
	BINDCOL_BIT( query, m_StdResponseApprovalsRequired );
	BINDCOL_BIT( query, m_MessageApprovalsRequired );
	BINDCOL_BIT( query, m_QuoteMsgInReply );
	BINDCOL_TCHAR( query, m_ReplyQuotedPrefix );
	BINDCOL_TCHAR( query, m_NTDomain );
	BINDCOL_LONG( query, m_IsNTUser );
	BINDCOL_LONG( query, m_StyleSheetID );
	BINDCOL_BIT( query, m_IsDeleted );
	BINDCOL_BIT( query, m_UseIM );
	BINDCOL_BIT( query, m_NewMessageFormat );
	BINDCOL_LONG( query, m_EscalateToAgentID );
	BINDCOL_BIT( query, m_RequireGetOldest );
	BINDCOL_BIT( query, m_HasLoggedIn );
	BINDCOL_LONG( query, m_DefaultSignatureID );
	BINDCOL_BIT( query, m_ForceSpellCheck );
	BINDCOL_BIT( query, m_SignatureTopReply );
	BINDCOL_BIT( query, m_ShowMessagesInbound );
	BINDCOL_BIT( query, m_UsePreviewPane );
	BINDCOL_TINYINT( query, m_CloseTicket );
	BINDCOL_TINYINT( query, m_RouteToInbox );
	BINDCOL_LONG( query, m_OutboxHoldTime );
	BINDCOL_BIT( query, m_UseMarkAsRead );
	BINDCOL_LONG( query, m_MarkAsReadSeconds);
	BINDCOL_LONG( query, m_OutboundApprovalFromID );
	BINDCOL_BIT( query, m_UseAutoFill );
	BINDCOL_LONG( query, m_DefaultTicketBoxID );
	BINDCOL_LONG( query, m_DefaultTicketDblClick );
	BINDCOL_LONG( query, m_ReadReceipt );
	BINDCOL_LONG( query, m_DeliveryConfirmation );
	BINDCOL_LONG( query, m_StatusID );
	BINDCOL_TCHAR( query, m_StatusText );
	BINDCOL_LONG( query, m_AutoStatusTypeID );
	BINDCOL_LONG( query, m_AutoStatusMin );
	BINDCOL_LONG( query, m_LoginStatusID );
	BINDCOL_LONG( query, m_LogoutStatusID );
	BINDCOL_TCHAR( query, m_OnlineText );
	BINDCOL_TCHAR( query, m_AwayText );
	BINDCOL_TCHAR( query, m_NotAvailText );
	BINDCOL_TCHAR( query, m_DndText );
	BINDCOL_TCHAR( query, m_OfflineText );
	BINDCOL_TCHAR( query, m_OooText );	
	BINDCOL_LONG( query, m_TimeZoneID);
	BINDCOL_LONG( query, m_DictionaryID );
	BINDCOL_BIT( query, m_DictionaryPrompt );
	query.Execute( _T("SELECT AgentID,LoginName,Password,Name,IsEnabled,ObjectID,DefaultAgentAddressID, ")
				   _T("MaxReportRowsPerPage,StdResponseApprovalsRequired,MessageApprovalsRequired, ")
				   _T("QuoteMsgInReply,ReplyQuotedPrefix,NTDomain,IsNTUser,StyleSheetID,IsDeleted,UseIM, ")
				   _T("NewMessageFormat,EscalateToAgentID,RequireGetOldest,HasLoggedIn,DefaultSignatureID, ")
				   _T("ForceSpellCheck,SignatureTopReply,ShowMessagesInbound,UsePreviewPane,CloseTicket, ")
				   _T("RouteToInbox,OutboxHoldTime,UseMarkAsRead,MarkAsReadSeconds,OutboundApprovalFromID, ")
				   _T("UseAutoFill,DefaultTicketBoxID,DefaultTicketDblClick,ReadReceipt,DeliveryConfirmation, ")
				   _T("StatusID,StatusText,AutoStatusTypeID,AutoStatusMin,LoginStatusID,LogoutStatusID, ")
	               _T("OnlineText,AwayText,NotAvailText,DndText,OfflineText,OooText,TimeZoneID,DictionaryID,DictionaryPrompt ")
	               _T("FROM Agents ")
	               _T("ORDER BY AgentID") );
}

/* class TAlertEvents - 
		The list of events that trigger alerts.
	 */
void TAlertEvents::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_NeedTicketBoxID );
	query.Execute( _T("INSERT INTO AlertEvents ")
	               _T("(Description,NeedTicketBoxID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_AlertEventID = query.GetLastInsertedID();
}

int TAlertEvents::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_NeedTicketBoxID );
	BINDPARAM_LONG( query, m_AlertEventID );
	query.Execute( _T("UPDATE AlertEvents ")
	               _T("SET Description=?,NeedTicketBoxID=? ")
	               _T("WHERE AlertEventID=?") );
	return query.GetRowCount();
}

int TAlertEvents::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AlertEventID );
	query.Execute( _T("DELETE FROM AlertEvents ")
	               _T("WHERE AlertEventID=?") );
	return query.GetRowCount();
}

int TAlertEvents::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_NeedTicketBoxID );
	BINDPARAM_LONG( query, m_AlertEventID );
	query.Execute( _T("SELECT Description,NeedTicketBoxID ")
	               _T("FROM AlertEvents ")
	               _T("WHERE AlertEventID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAlertEvents::GetLongData( CODBCQuery& query )
{
}

void TAlertEvents::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AlertEventID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_NeedTicketBoxID );
	query.Execute( _T("SELECT AlertEventID,Description,NeedTicketBoxID ")
	               _T("FROM AlertEvents ")
	               _T("ORDER BY AlertEventID") );
}

/* class TAlertMethods - 
		The list of methods used to send an alert.
	 */
void TAlertMethods::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO AlertMethods ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_AlertMethodID = query.GetLastInsertedID();
}

int TAlertMethods::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_AlertMethodID );
	query.Execute( _T("UPDATE AlertMethods ")
	               _T("SET Description=? ")
	               _T("WHERE AlertMethodID=?") );
	return query.GetRowCount();
}

int TAlertMethods::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AlertMethodID );
	query.Execute( _T("DELETE FROM AlertMethods ")
	               _T("WHERE AlertMethodID=?") );
	return query.GetRowCount();
}

int TAlertMethods::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_AlertMethodID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM AlertMethods ")
	               _T("WHERE AlertMethodID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAlertMethods::GetLongData( CODBCQuery& query )
{
}

void TAlertMethods::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AlertMethodID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT AlertMethodID,Description ")
	               _T("FROM AlertMethods ")
	               _T("ORDER BY AlertMethodID") );
}

/* class TAlertMsgs - 
		Contains alerts messages.
	 */
void TAlertMsgs::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AlertEventID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_TINYINT( query, m_Viewed );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_TEXT( query, m_Body );
	query.Execute( _T("INSERT INTO AlertMsgs ")
	               _T("(AlertEventID,AgentID,TicketID,Viewed,DateCreated,Subject,Body) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?)") );
	m_AlertMsgID = query.GetLastInsertedID();
}

int TAlertMsgs::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AlertEventID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_TINYINT( query, m_Viewed );
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_TEXT( query, m_Body );
	BINDPARAM_LONG( query, m_AlertMsgID );
	query.Execute( _T("UPDATE AlertMsgs ")
	               _T("SET AlertEventID=?,AgentID=?,TicketID=?,Viewed=?,Subject=?,Body=? ")
	               _T("WHERE AlertMsgID=?") );
	return query.GetRowCount();
}

int TAlertMsgs::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AlertMsgID );
	query.Execute( _T("DELETE FROM AlertMsgs ")
	               _T("WHERE AlertMsgID=?") );
	return query.GetRowCount();
}

int TAlertMsgs::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AlertEventID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_TINYINT( query, m_Viewed );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TCHAR( query, m_Subject );
	BINDPARAM_LONG( query, m_AlertMsgID );
    query.Execute( _T("SELECT AlertEventID,AgentID,TicketID,Viewed,DateCreated,Subject,Body ")
	               _T("FROM AlertMsgs ")
	               _T("WHERE AlertMsgID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAlertMsgs::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Body );
}

void TAlertMsgs::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AlertMsgID );
	BINDCOL_LONG( query, m_AlertEventID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_TINYINT( query, m_Viewed );
	BINDCOL_TIME( query, m_DateCreated );
	BINDPARAM_TCHAR( query, m_Subject );
	query.Execute( _T("SELECT AlertMsgID,AlertEventID,AgentID,TicketID,Viewed,DateCreated,Subject,Body ")
	               _T("FROM AlertMsgs ")
	               _T("ORDER BY AlertMsgID") );
}

/* class TAlerts - 
		Active alert definitions.
	 */
void TAlerts::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AlertEventID );
	BINDPARAM_LONG( query, m_AlertMethodID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_GroupID );
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_TCHAR( query, m_FromEmailAddress );
	query.Execute( _T("INSERT INTO Alerts ")
	               _T("(AlertEventID,AlertMethodID,AgentID,GroupID,TicketBoxID,EmailAddress,FromEmailAddress) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?)") );
	m_AlertID = query.GetLastInsertedID();
}

int TAlerts::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AlertEventID );
	BINDPARAM_LONG( query, m_AlertMethodID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_GroupID );
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_TCHAR( query, m_FromEmailAddress );
	BINDPARAM_LONG( query, m_AlertID );
	query.Execute( _T("UPDATE Alerts ")
	               _T("SET AlertEventID=?,AlertMethodID=?,AgentID=?,GroupID=?,TicketBoxID=?,EmailAddress=?,FromEmailAddress=? ")
	               _T("WHERE AlertID=?") );
	return query.GetRowCount();
}

int TAlerts::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AlertID );
	query.Execute( _T("DELETE FROM Alerts ")
	               _T("WHERE AlertID=?") );
	return query.GetRowCount();
}

int TAlerts::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AlertEventID );
	BINDCOL_LONG( query, m_AlertMethodID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_TCHAR( query, m_FromEmailAddress );
	BINDPARAM_LONG( query, m_AlertID );
	query.Execute( _T("SELECT AlertEventID,AlertMethodID,AgentID,GroupID,TicketBoxID,EmailAddress,FromEmailAddress ")
	               _T("FROM Alerts ")
	               _T("WHERE AlertID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAlerts::GetLongData( CODBCQuery& query )
{
}

void TAlerts::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AlertID );
	BINDCOL_LONG( query, m_AlertEventID );
	BINDCOL_LONG( query, m_AlertMethodID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_TCHAR( query, m_FromEmailAddress );
	query.Execute( _T("SELECT AlertID,AlertEventID,AlertMethodID,AgentID,GroupID,TicketBoxID,EmailAddress,FromEmailAddress ")
	               _T("FROM Alerts ")
	               _T("ORDER BY AlertID") );
}

/* class TApprovalObjectTypes - 
		This table identifies the various types of objects that can be channeled 
		through the approval process. Examples include:
		Messages, Standard responses, etc.
	 */
void TApprovalObjectTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	query.Execute( _T("INSERT INTO ApprovalObjectTypes ")
	               _T("(Name) ")
	               _T("VALUES")
	               _T("(?)") );
	m_ApprovalObjectTypeID = query.GetLastInsertedID();
}

int TApprovalObjectTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_LONG( query, m_ApprovalObjectTypeID );
	query.Execute( _T("UPDATE ApprovalObjectTypes ")
	               _T("SET Name=? ")
	               _T("WHERE ApprovalObjectTypeID=?") );
	return query.GetRowCount();
}

int TApprovalObjectTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ApprovalObjectTypeID );
	query.Execute( _T("DELETE FROM ApprovalObjectTypes ")
	               _T("WHERE ApprovalObjectTypeID=?") );
	return query.GetRowCount();
}

int TApprovalObjectTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Name );
	BINDPARAM_LONG( query, m_ApprovalObjectTypeID );
	query.Execute( _T("SELECT Name ")
	               _T("FROM ApprovalObjectTypes ")
	               _T("WHERE ApprovalObjectTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TApprovalObjectTypes::GetLongData( CODBCQuery& query )
{
}

void TApprovalObjectTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ApprovalObjectTypeID );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute( _T("SELECT ApprovalObjectTypeID,Name ")
	               _T("FROM ApprovalObjectTypes ")
	               _T("ORDER BY ApprovalObjectTypeID") );
}

/* class TApprovals - 
		In scenarios where agents require approvals for specifc actions (e.g. sending a message), 
		this table determine who is responsible for approving the action (either a user/agent or group).
	 */
void TApprovals::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_RequestAgentID );
	BINDPARAM_LONG( query, m_ApproverAgentID );
	BINDPARAM_LONG( query, m_ApproverGroupID );
	BINDPARAM_LONG( query, m_ApprovalObjectTypeID );
	BINDPARAM_LONG( query, m_ActualID );
	query.Execute( _T("INSERT INTO Approvals ")
	               _T("(RequestAgentID,ApproverAgentID,ApproverGroupID,ApprovalObjectTypeID,ActualID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?)") );
	m_ApprovalID = query.GetLastInsertedID();
}

int TApprovals::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_RequestAgentID );
	BINDPARAM_LONG( query, m_ApproverAgentID );
	BINDPARAM_LONG( query, m_ApproverGroupID );
	BINDPARAM_LONG( query, m_ApprovalObjectTypeID );
	BINDPARAM_LONG( query, m_ActualID );
	BINDPARAM_LONG( query, m_ApprovalID );
	query.Execute( _T("UPDATE Approvals ")
	               _T("SET RequestAgentID=?,ApproverAgentID=?,ApproverGroupID=?,ApprovalObjectTypeID=?,ActualID=? ")
	               _T("WHERE ApprovalID=?") );
	return query.GetRowCount();
}

int TApprovals::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ApprovalID );
	query.Execute( _T("DELETE FROM Approvals ")
	               _T("WHERE ApprovalID=?") );
	return query.GetRowCount();
}

int TApprovals::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_RequestAgentID );
	BINDCOL_LONG( query, m_ApproverAgentID );
	BINDCOL_LONG( query, m_ApproverGroupID );
	BINDCOL_LONG( query, m_ApprovalObjectTypeID );
	BINDCOL_LONG( query, m_ActualID );
	BINDPARAM_LONG( query, m_ApprovalID );
	query.Execute( _T("SELECT RequestAgentID,ApproverAgentID,ApproverGroupID,ApprovalObjectTypeID,ActualID ")
	               _T("FROM Approvals ")
	               _T("WHERE ApprovalID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TApprovals::GetLongData( CODBCQuery& query )
{
}

void TApprovals::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ApprovalID );
	BINDCOL_LONG( query, m_RequestAgentID );
	BINDCOL_LONG( query, m_ApproverAgentID );
	BINDCOL_LONG( query, m_ApproverGroupID );
	BINDCOL_LONG( query, m_ApprovalObjectTypeID );
	BINDCOL_LONG( query, m_ActualID );
	query.Execute( _T("SELECT ApprovalID,RequestAgentID,ApproverAgentID,ApproverGroupID,ApprovalObjectTypeID,ActualID ")
	               _T("FROM Approvals ")
	               _T("ORDER BY ApprovalID") );
}

/* class TArchives - 
		Used to track archive/purge operations.
	 */
void TArchives::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_TINYINT( query, m_Purged );
	BINDPARAM_TCHAR( query, m_ArcFilePath );
	BINDPARAM_LONG( query, m_InMsgRecords );
	BINDPARAM_LONG( query, m_InAttRecords );
	BINDPARAM_LONG( query, m_OutMsgRecords );
	BINDPARAM_LONG( query, m_OutAttRecords );
	BINDPARAM_LONG( query, m_NoteAttRecords );
	query.Execute( _T("INSERT INTO Archives ")
	               _T("(DateCreated,Purged,ArcFilePath,InMsgRecords,InAttRecords,OutMsgRecords,OutAttRecords,NoteAttRecords) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?)") );
	m_ArchiveID = query.GetLastInsertedID();
}

int TArchives::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TINYINT( query, m_Purged );
	BINDPARAM_TCHAR( query, m_ArcFilePath );
	BINDPARAM_LONG( query, m_InMsgRecords );
	BINDPARAM_LONG( query, m_InAttRecords );
	BINDPARAM_LONG( query, m_OutMsgRecords );
	BINDPARAM_LONG( query, m_OutAttRecords );
	BINDPARAM_LONG( query, m_NoteAttRecords );
	BINDPARAM_LONG( query, m_ArchiveID );
	query.Execute( _T("UPDATE Archives ")
	               _T("SET Purged=?,ArcFilePath=?,InMsgRecords=?,InAttRecords=?,OutMsgRecords=?,OutAttRecords=?,NoteAttRecords=? ")
	               _T("WHERE ArchiveID=?") );
	return query.GetRowCount();
}

int TArchives::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ArchiveID );
	query.Execute( _T("DELETE FROM Archives ")
	               _T("WHERE ArchiveID=?") );
	return query.GetRowCount();
}

int TArchives::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TINYINT( query, m_Purged );
	BINDCOL_TCHAR( query, m_ArcFilePath );
	BINDCOL_LONG( query, m_InMsgRecords );
	BINDCOL_LONG( query, m_InAttRecords );
	BINDCOL_LONG( query, m_OutMsgRecords );
	BINDCOL_LONG( query, m_OutAttRecords );
	BINDCOL_LONG( query, m_NoteAttRecords );
	BINDPARAM_LONG( query, m_ArchiveID );
	query.Execute( _T("SELECT DateCreated,Purged,ArcFilePath,InMsgRecords,InAttRecords,OutMsgRecords,OutAttRecords,NoteAttRecords ")
	               _T("FROM Archives ")
	               _T("WHERE ArchiveID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TArchives::GetLongData( CODBCQuery& query )
{
}

void TArchives::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ArchiveID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TINYINT( query, m_Purged );
	BINDCOL_TCHAR( query, m_ArcFilePath );
	BINDCOL_LONG( query, m_InMsgRecords );
	BINDCOL_LONG( query, m_InAttRecords );
	BINDCOL_LONG( query, m_OutMsgRecords );
	BINDCOL_LONG( query, m_OutAttRecords );
	BINDCOL_LONG( query, m_NoteAttRecords );
	query.Execute( _T("SELECT ArchiveID,DateCreated,Purged,ArcFilePath,InMsgRecords,InAttRecords,OutMsgRecords,OutAttRecords,NoteAttRecords ")
	               _T("FROM Archives ")
	               _T("ORDER BY ArchiveID") );
}

/* class TAttachments - 
		Attachments
	 */
void TAttachments::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_AttachmentLocation );
	BINDPARAM_TCHAR( query, m_MediaType );
	BINDPARAM_TCHAR( query, m_MediaSubType );
	BINDPARAM_TCHAR( query, m_ContentDisposition );
	BINDPARAM_TCHAR( query, m_FileName );
	BINDPARAM_LONG( query, m_FileSize );
	BINDPARAM_LONG( query, m_VirusScanStateID );
	BINDPARAM_TCHAR( query, m_VirusName );
	BINDPARAM_TCHAR( query, m_ContentID );
	BINDPARAM_BIT( query, m_IsInbound );
	query.Execute( _T("INSERT INTO Attachments ")
	               _T("(AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?)") );
	m_AttachmentID = query.GetLastInsertedID();
}

int TAttachments::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_AttachmentLocation );
	BINDPARAM_TCHAR( query, m_MediaType );
	BINDPARAM_TCHAR( query, m_MediaSubType );
	BINDPARAM_TCHAR( query, m_ContentDisposition );
	BINDPARAM_TCHAR( query, m_FileName );
	BINDPARAM_LONG( query, m_FileSize );
	BINDPARAM_LONG( query, m_VirusScanStateID );
	BINDPARAM_TCHAR( query, m_VirusName );
	BINDPARAM_TCHAR( query, m_ContentID );
	BINDPARAM_BIT( query, m_IsInbound );
	BINDPARAM_LONG( query, m_AttachmentID );
	query.Execute( _T("UPDATE Attachments ")
	               _T("SET AttachmentLocation=?,MediaType=?,MediaSubType=?,ContentDisposition=?,FileName=?,FileSize=?,VirusScanStateID=?,VirusName=?,ContentID=?,IsInbound=? ")
	               _T("WHERE AttachmentID=?") );
	return query.GetRowCount();
}

int TAttachments::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AttachmentID );
	query.Execute( _T("DELETE FROM Attachments ")
	               _T("WHERE AttachmentID=?") );
	return query.GetRowCount();
}

int TAttachments::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_AttachmentLocation );
	BINDCOL_TCHAR( query, m_MediaType );
	BINDCOL_TCHAR( query, m_MediaSubType );
	BINDCOL_TCHAR( query, m_ContentDisposition );
	BINDCOL_TCHAR( query, m_FileName );
	BINDCOL_LONG( query, m_FileSize );
	BINDCOL_LONG( query, m_VirusScanStateID );
	BINDCOL_TCHAR( query, m_VirusName );
	BINDCOL_TCHAR( query, m_ContentID );
	BINDCOL_BIT( query, m_IsInbound );
	BINDPARAM_LONG( query, m_AttachmentID );
	query.Execute( _T("SELECT AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound ")
	               _T("FROM Attachments ")
	               _T("WHERE AttachmentID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAttachments::GetLongData( CODBCQuery& query )
{
}

void TAttachments::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AttachmentID );
	BINDCOL_TCHAR( query, m_AttachmentLocation );
	BINDCOL_TCHAR( query, m_MediaType );
	BINDCOL_TCHAR( query, m_MediaSubType );
	BINDCOL_TCHAR( query, m_ContentDisposition );
	BINDCOL_TCHAR( query, m_FileName );
	BINDCOL_LONG( query, m_FileSize );
	BINDCOL_LONG( query, m_VirusScanStateID );
	BINDCOL_TCHAR( query, m_VirusName );
	BINDCOL_TCHAR( query, m_ContentID );
	BINDCOL_BIT( query, m_IsInbound );
	query.Execute( _T("SELECT AttachmentID,AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound ")
	               _T("FROM Attachments ")
	               _T("ORDER BY AttachmentID") );
}

/* class TContactNotes - 
		Contact notes will be used in a similar capactiy as TicketNotes, 
		both by users for collaboration purposes and by the system directly. 
		The system may, for example, create a contact note when the contact is merged with another.
	 */
void TContactNotes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_TEXT( query, m_Note );
	BINDPARAM_BIT( query, m_IsVoipNote );
	BINDPARAM_TCHAR( query, m_ElapsedTime );
	BINDPARAM_TCHAR( query, m_Subject );
	if ( m_StartTimeLen <= 0 || m_StopTimeLen <= 0 )
	{
		query.Execute( _T("INSERT INTO ContactNotes ")
	               _T("(ContactID,AgentID,DateCreated,Note,IsVoipNote,ElapsedTime,Subject) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?)") );	
	}
	else
	{
		BINDPARAM_TIME( query, m_StartTime );
		BINDPARAM_TIME( query, m_StopTime );
		query.Execute( _T("INSERT INTO ContactNotes ")
	               _T("(ContactID,AgentID,DateCreated,Note,IsVoipNote,ElapsedTime,Subject,StartTime,StopTime) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?)") );	
	}
	m_ContactNoteID = query.GetLastInsertedID();
}

int TContactNotes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_TEXT( query, m_Note );
	BINDPARAM_BIT( query, m_IsVoipNote );
	BINDPARAM_TCHAR( query, m_Subject );
	if ( m_StartTimeLen <= 0 || m_StopTimeLen <= 0 )
	{
		BINDPARAM_LONG( query, m_ContactNoteID );
		query.Execute( _T("UPDATE ContactNotes ")
	               _T("SET ContactID=?,AgentID=?,Note=?,IsVoipNote=?,Subject=? ")
	               _T("WHERE ContactNoteID=?") );	
	}
	else
	{
		BINDPARAM_TCHAR( query, m_ElapsedTime );
		BINDPARAM_TIME( query, m_StartTime );
		BINDPARAM_TIME( query, m_StopTime );
		BINDPARAM_LONG( query, m_ContactNoteID );
		query.Execute( _T("UPDATE ContactNotes ")
	               _T("SET ContactID=?,AgentID=?,Note=?,IsVoipNote=?,Subject=?,ElapsedTime=?,StartTime=?,StopTime=? ")
	               _T("WHERE ContactNoteID=?") );		
	}
	return query.GetRowCount();
}

int TContactNotes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ContactNoteID );
	query.Execute( _T("DELETE FROM ContactNotes ")
	               _T("WHERE ContactNoteID=?") );
	return query.GetRowCount();
}

int TContactNotes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_ContactID );
	BINDCOL_LONG( query, m_NumAttach );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_BIT( query, m_IsVoipNote );
	BINDCOL_TIME( query, m_StartTime );
	BINDCOL_TIME( query, m_StopTime );
	BINDCOL_TCHAR( query, m_ElapsedTime );
	BINDCOL_TCHAR( query, m_Subject );
	BINDPARAM_LONG( query, m_ContactNoteID );
	query.Execute( _T("SELECT ContactID,(SELECT COUNT(1) FROM NoteAttachments WHERE NoteID=ContactNoteID AND NoteTypeID=2),AgentID,DateCreated,IsVoipNote,StartTime,StopTime,ElapsedTime,Subject,Note ")
	               _T("FROM ContactNotes ")
	               _T("WHERE ContactNoteID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TContactNotes::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Note );
}

void TContactNotes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ContactNoteID );
	BINDCOL_LONG( query, m_ContactID );
	BINDCOL_LONG( query, m_NumAttach );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_BIT( query, m_IsVoipNote );
	BINDCOL_TIME( query, m_StartTime );
	BINDCOL_TIME( query, m_StopTime );
	BINDCOL_TCHAR( query, m_ElapsedTime );
	BINDCOL_TCHAR( query, m_Subject );
	query.Execute( _T("SELECT ContactNoteID,ContactID,(SELECT COUNT(1) FROM NoteAttachments WHERE NoteID=ContactNoteID AND NoteTypeID=2),AgentID,DateCreated,IsVoipNote,StartTime,StopTime,ElapsedTime,Subject,Note ")
	               _T("FROM ContactNotes ")
	               _T("ORDER BY ContactNoteID") );
}

/* class TContacts - 
		Contacts are customers or external entities.
	 */
void TContacts::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_TCHAR( query, m_CompanyName );
	BINDPARAM_TCHAR( query, m_Title );
	BINDPARAM_TCHAR( query, m_WebPageAddress );
	BINDPARAM_TCHAR( query, m_StreetAddress1 );
	BINDPARAM_TCHAR( query, m_StreetAddress2 );
	BINDPARAM_TCHAR( query, m_City );
	BINDPARAM_TCHAR( query, m_State );
	BINDPARAM_TCHAR( query, m_ZipCode );
	BINDPARAM_TCHAR( query, m_Country );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_TIME( query, m_DeletedTime );
	BINDPARAM_LONG( query, m_DeletedBy );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_DefaultEmailAddressID );
	query.Execute( _T("INSERT INTO Contacts ")
	               _T("(Name,CompanyName,Title,WebPageAddress,StreetAddress1,StreetAddress2,City,State,ZipCode,Country,DateCreated,IsDeleted,DeletedTime,DeletedBy,OwnerID,DefaultEmailAddressID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_ContactID = query.GetLastInsertedID();
}

int TContacts::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_TCHAR( query, m_CompanyName );
	BINDPARAM_TCHAR( query, m_Title );
	BINDPARAM_TCHAR( query, m_WebPageAddress );
	BINDPARAM_TCHAR( query, m_StreetAddress1 );
	BINDPARAM_TCHAR( query, m_StreetAddress2 );
	BINDPARAM_TCHAR( query, m_City );
	BINDPARAM_TCHAR( query, m_State );
	BINDPARAM_TCHAR( query, m_ZipCode );
	BINDPARAM_TCHAR( query, m_Country );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_TIME( query, m_DeletedTime );
	BINDPARAM_LONG( query, m_DeletedBy );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_DefaultEmailAddressID );
	BINDPARAM_LONG( query, m_ContactID );
	query.Execute( _T("UPDATE Contacts ")
	               _T("SET Name=?,CompanyName=?,Title=?,WebPageAddress=?,StreetAddress1=?,StreetAddress2=?,City=?,State=?,ZipCode=?,Country=?,IsDeleted=?,DeletedTime=?,DeletedBy=?,OwnerID=?,DefaultEmailAddressID=? ")
	               _T("WHERE ContactID=?") );
	return query.GetRowCount();
}

int TContacts::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ContactID );
	query.Execute( _T("DELETE FROM Contacts ")
	               _T("WHERE ContactID=?") );
	return query.GetRowCount();
}

int TContacts::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_TCHAR( query, m_CompanyName );
	BINDCOL_TCHAR( query, m_Title );
	BINDCOL_TCHAR( query, m_WebPageAddress );
	BINDCOL_TCHAR( query, m_StreetAddress1 );
	BINDCOL_TCHAR( query, m_StreetAddress2 );
	BINDCOL_TCHAR( query, m_City );
	BINDCOL_TCHAR( query, m_State );
	BINDCOL_TCHAR( query, m_ZipCode );
	BINDCOL_TCHAR( query, m_Country );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_TIME( query, m_DeletedTime );
	BINDCOL_LONG( query, m_DeletedBy );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_LONG( query, m_DefaultEmailAddressID );
	BINDPARAM_LONG( query, m_ContactID );
	query.Execute( _T("SELECT Name,CompanyName,Title,WebPageAddress,StreetAddress1,StreetAddress2,City,State,ZipCode,Country,DateCreated,IsDeleted,DeletedTime,DeletedBy,OwnerID,DefaultEmailAddressID ")
	               _T("FROM Contacts ")
	               _T("WHERE ContactID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TContacts::GetLongData( CODBCQuery& query )
{
}

void TContacts::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ContactID );
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_TCHAR( query, m_CompanyName );
	BINDCOL_TCHAR( query, m_Title );
	BINDCOL_TCHAR( query, m_WebPageAddress );
	BINDCOL_TCHAR( query, m_StreetAddress1 );
	BINDCOL_TCHAR( query, m_StreetAddress2 );
	BINDCOL_TCHAR( query, m_City );
	BINDCOL_TCHAR( query, m_State );
	BINDCOL_TCHAR( query, m_ZipCode );
	BINDCOL_TCHAR( query, m_Country );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_TIME( query, m_DeletedTime );
	BINDCOL_LONG( query, m_DeletedBy );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_LONG( query, m_DefaultEmailAddressID );
	query.Execute( _T("SELECT ContactID,Name,CompanyName,Title,WebPageAddress,StreetAddress1,StreetAddress2,City,State,ZipCode,Country,DateCreated,IsDeleted,DeletedTime,DeletedBy,OwnerID,DefaultEmailAddressID ")
	               _T("FROM Contacts ")
	               _T("ORDER BY ContactID") );
}

/* class TContactGroups - 
		Groups of contacts.
	 */
void TContactGroups::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_GroupName );
	BINDPARAM_LONG( query, m_OwnerID );
	query.Execute( _T("INSERT INTO ContactGroups ")
	               _T("(GroupName,OwnerID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_ContactGroupID = query.GetLastInsertedID();
}

int TContactGroups::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_GroupName );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_ContactGroupID );
	query.Execute( _T("UPDATE ContactGroups ")
	               _T("SET GroupName=?,OwnerID=? ")
	               _T("WHERE ContactGroupID=?") );
	return query.GetRowCount();
}

int TContactGroups::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ContactGroupID );
	query.Execute( _T("DELETE FROM ContactGroups ")
	               _T("WHERE GroupID=?") );
	return query.GetRowCount();
}

int TContactGroups::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_GroupName );
	BINDCOL_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_ContactGroupID );
	query.Execute( _T("SELECT GroupName,OwnerID ")
	               _T("FROM ContactGroups ")
	               _T("WHERE ContactGroupID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TContactGroups::GetLongData( CODBCQuery& query )
{
}

void TContactGroups::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ContactGroupID );
	BINDCOL_TCHAR( query, m_GroupName );
	BINDCOL_LONG( query, m_OwnerID );
	query.Execute( _T("SELECT ContactGroupID,GroupName,OwnerID ")
	               _T("FROM ContactGroups ")
	               _T("ORDER BY ContactGroupID") );
}

/* class TContactGrouping - 
		Segments contacts into logical groups.
	 */
void TContactGrouping::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_ContactGroupID );
	query.Execute( _T("INSERT INTO ContactGrouping ")
	               _T("(ContactID,ContactGroupID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_ContactGroupingID = query.GetLastInsertedID();
}

int TContactGrouping::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_ContactGroupID );
	BINDPARAM_LONG( query, m_ContactGroupingID );
	query.Execute( _T("UPDATE ContactGrouping ")
	               _T("SET ContactID=?,ContactGroupID=? ")
	               _T("WHERE ContactGroupingID=?") );
	return query.GetRowCount();
}

int TContactGrouping::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ContactGroupingID );
	query.Execute( _T("DELETE FROM ContactGrouping ")
	               _T("WHERE ContactGroupingID=?") );
	return query.GetRowCount();
}

int TContactGrouping::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_ContactID );
	BINDCOL_LONG( query, m_ContactGroupID );
	BINDPARAM_LONG( query, m_ContactGroupingID );
	query.Execute( _T("SELECT ContactID,ContactGroupID ")
	               _T("FROM ContactGrouping ")
	               _T("WHERE ContactGroupingID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TContactGrouping::GetLongData( CODBCQuery& query )
{
}

void TContactGrouping::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ContactGroupingID );
	BINDCOL_LONG( query, m_ContactID );
	BINDCOL_LONG( query, m_ContactGroupID );
	query.Execute( _T("SELECT ContactGroupingID,ContactID,ContactGroupID ")
	               _T("FROM ContactGrouping ")
	               _T("ORDER BY ContactGroupingID") );
}

/* class TCustomDictionary - 
		Contains global and user custom dictionaries.
	 */
void TCustomDictionary::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_TCHAR( query, m_Word );
	query.Execute( _T("INSERT INTO CustomDictionary ")
	               _T("(AgentID,Word) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_CustomDictionaryID = query.GetLastInsertedID();
}

int TCustomDictionary::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_TCHAR( query, m_Word );
	BINDPARAM_LONG( query, m_CustomDictionaryID );
	query.Execute( _T("UPDATE CustomDictionary ")
	               _T("SET AgentID=?,Word=? ")
	               _T("WHERE CustomDictionaryID=?") );
	return query.GetRowCount();
}

int TCustomDictionary::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_CustomDictionaryID );
	query.Execute( _T("DELETE FROM CustomDictionary ")
	               _T("WHERE CustomDictionaryID=?") );
	return query.GetRowCount();
}

int TCustomDictionary::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TCHAR( query, m_Word );
	BINDPARAM_LONG( query, m_CustomDictionaryID );
	query.Execute( _T("SELECT AgentID,Word ")
	               _T("FROM CustomDictionary ")
	               _T("WHERE CustomDictionaryID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TCustomDictionary::GetLongData( CODBCQuery& query )
{
}

void TCustomDictionary::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_CustomDictionaryID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TCHAR( query, m_Word );
	query.Execute( _T("SELECT CustomDictionaryID,AgentID,Word ")
	               _T("FROM CustomDictionary ")
	               _T("ORDER BY CustomDictionaryID") );
}

/* class TForwardCCAddresses - 
		Table of "cc" addresses used when forwarding messasges matched by the routing rules.
	 */
void TForwardCCAddresses::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	query.Execute( _T("INSERT INTO ForwardCCAddresses ")
	               _T("(EmailAddress,RoutingRuleID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_AddressID = query.GetLastInsertedID();
}

int TForwardCCAddresses::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_AddressID );
	query.Execute( _T("UPDATE ForwardCCAddresses ")
	               _T("SET EmailAddress=?,RoutingRuleID=? ")
	               _T("WHERE AddressID=?") );
	return query.GetRowCount();
}

int TForwardCCAddresses::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AddressID );
	query.Execute( _T("DELETE FROM ForwardCCAddresses ")
	               _T("WHERE AddressID=?") );
	return query.GetRowCount();
}

int TForwardCCAddresses::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_AddressID );
	query.Execute( _T("SELECT EmailAddress,RoutingRuleID ")
	               _T("FROM ForwardCCAddresses ")
	               _T("WHERE AddressID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TForwardCCAddresses::GetLongData( CODBCQuery& query )
{
}

void TForwardCCAddresses::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AddressID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_RoutingRuleID );
	query.Execute( _T("SELECT AddressID,EmailAddress,RoutingRuleID ")
	               _T("FROM ForwardCCAddresses ")
	               _T("ORDER BY AddressID") );
}

/* class TForwardToAddresses - 
		Table of "to" addresses used when forwarding messages matched by the routing rules.
	 */
void TForwardToAddresses::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	query.Execute( _T("INSERT INTO ForwardToAddresses ")
	               _T("(EmailAddress,RoutingRuleID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_AddressID = query.GetLastInsertedID();
}

int TForwardToAddresses::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_AddressID );
	query.Execute( _T("UPDATE ForwardToAddresses ")
	               _T("SET EmailAddress=?,RoutingRuleID=? ")
	               _T("WHERE AddressID=?") );
	return query.GetRowCount();
}

int TForwardToAddresses::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AddressID );
	query.Execute( _T("DELETE FROM ForwardToAddresses ")
	               _T("WHERE AddressID=?") );
	return query.GetRowCount();
}

int TForwardToAddresses::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_AddressID );
	query.Execute( _T("SELECT EmailAddress,RoutingRuleID ")
	               _T("FROM ForwardToAddresses ")
	               _T("WHERE AddressID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TForwardToAddresses::GetLongData( CODBCQuery& query )
{
}

void TForwardToAddresses::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AddressID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_RoutingRuleID );
	query.Execute( _T("SELECT AddressID,EmailAddress,RoutingRuleID ")
	               _T("FROM ForwardToAddresses ")
	               _T("ORDER BY AddressID") );
}

/* class TGroups - 
		Groups of agents.
	 */
void TGroups::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_GroupName );
	BINDPARAM_LONG( query, m_ObjectID );
	BINDPARAM_BIT( query, m_IsDeleted );
	BINDPARAM_BIT( query, m_IsEscGroup );
	BINDPARAM_BIT( query, m_UseEscTicketBox );
	BINDPARAM_LONG( query, m_AssignToTicketBoxID );
	BINDPARAM_LONG( query, m_TimeZoneID );
	BINDPARAM_LONG( query, m_DictionaryID );
	query.Execute( _T("INSERT INTO Groups ")
	               _T("(GroupName,ObjectID,IsDeleted,IsEscalationGroup,UseEscTicketBox,AssignToTicketBoxID,TimeZoneID,DictionaryID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?)") );
	m_GroupID = query.GetLastInsertedID();
}

/* class TGroups - 
		Groups of agents.
	 */
int TGroups::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_GroupName );
	BINDPARAM_BIT( query, m_IsDeleted );
	BINDPARAM_BIT( query, m_IsEscGroup );
	BINDPARAM_BIT( query, m_UseEscTicketBox );
	BINDPARAM_LONG( query, m_AssignToTicketBoxID );
	BINDPARAM_LONG( query, m_TimeZoneID );
	BINDPARAM_LONG( query, m_DictionaryID );
	BINDPARAM_LONG( query, m_GroupID );
	query.Execute( _T("UPDATE Groups ")
	               _T("SET GroupName=?,IsDeleted=?,IsEscalationGroup=?,UseEscTicketBox=?,AssignToTicketBoxID=?,TimeZoneID=?,DictionaryID=? ")
	               _T("WHERE GroupID=?") );
	return query.GetRowCount();
}

int TGroups::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_GroupID );
	query.Execute( _T("DELETE FROM Groups ")
	               _T("WHERE GroupID=?") );
	return query.GetRowCount();
}

int TGroups::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_GroupName );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_BIT( query, m_IsDeleted );
	BINDCOL_BIT( query, m_IsEscGroup );
	BINDCOL_BIT( query, m_UseEscTicketBox );
	BINDCOL_LONG( query, m_AssignToTicketBoxID );
	BINDCOL_LONG( query, m_TimeZoneID );
	BINDCOL_LONG( query, m_DictionaryID );
	BINDPARAM_LONG( query, m_GroupID );
	query.Execute( _T("SELECT GroupName,ObjectID,IsDeleted,IsEscalationGroup,UseEscTicketBox,AssignToTicketBoxID,TimeZoneID,DictionaryID ")
	               _T("FROM Groups ")
	               _T("WHERE GroupID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TGroups::GetLongData( CODBCQuery& query )
{
}

void TGroups::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_TCHAR( query, m_GroupName );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_BIT( query, m_IsDeleted );
	BINDCOL_BIT( query, m_IsEscGroup );
	BINDCOL_BIT( query, m_UseEscTicketBox );
	BINDCOL_LONG( query, m_AssignToTicketBoxID );
	BINDCOL_LONG( query, m_TimeZoneID );
	BINDCOL_LONG( query, m_DictionaryID );
	query.Execute( _T("SELECT GroupID,GroupName,ObjectID,IsDeleted,IsEscalationGroup,UseEscTicketBox,AssignToTicketBoxID,TimeZoneID,DictionaryID ")
	               _T("FROM Groups ")
	               _T("ORDER BY GroupID") );
}

/* class TIPRanges - 
		Defines ranges of IP addresses for login restrictions.
	 */
void TIPRanges::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_GroupID );
	BINDPARAM_LONG( query, m_LowerRange );
	BINDPARAM_LONG( query, m_UpperRange );
	query.Execute( _T("INSERT INTO IPRanges ")
	               _T("(AgentID,GroupID,LowerRange,UpperRange) ")
	               _T("VALUES")
	               _T("(?,?,?,?)") );
	m_IPRangeID = query.GetLastInsertedID();
}

int TIPRanges::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_GroupID );
	BINDPARAM_LONG( query, m_LowerRange );
	BINDPARAM_LONG( query, m_UpperRange );
	BINDPARAM_LONG( query, m_IPRangeID );
	query.Execute( _T("UPDATE IPRanges ")
	               _T("SET AgentID=?,GroupID=?,LowerRange=?,UpperRange=? ")
	               _T("WHERE IPRangeID=?") );
	return query.GetRowCount();
}

int TIPRanges::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_IPRangeID );
	query.Execute( _T("DELETE FROM IPRanges ")
	               _T("WHERE IPRangeID=?") );
	return query.GetRowCount();
}

int TIPRanges::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_LONG( query, m_LowerRange );
	BINDCOL_LONG( query, m_UpperRange );
	BINDPARAM_LONG( query, m_IPRangeID );
	query.Execute( _T("SELECT AgentID,GroupID,LowerRange,UpperRange ")
	               _T("FROM IPRanges ")
	               _T("WHERE IPRangeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TIPRanges::GetLongData( CODBCQuery& query )
{
}

void TIPRanges::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_IPRangeID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_LONG( query, m_LowerRange );
	BINDCOL_LONG( query, m_UpperRange );
	query.Execute( _T("SELECT IPRangeID,AgentID,GroupID,LowerRange,UpperRange ")
	               _T("FROM IPRanges ")
	               _T("ORDER BY IPRangeID") );
}

/* class TInboundMessageAttachments - 
		Attachments to inbound messages.
	 */
void TInboundMessageAttachments::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AttachmentID );
	BINDPARAM_LONG( query, m_InboundMessageID );
	query.Execute( _T("INSERT INTO InboundMessageAttachments ")
	               _T("(AttachmentID,InboundMessageID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_InboundMessageAttachmentID = query.GetLastInsertedID();
}

int TInboundMessageAttachments::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AttachmentID );
	BINDPARAM_LONG( query, m_InboundMessageID );
	BINDPARAM_LONG( query, m_InboundMessageAttachmentID );
	query.Execute( _T("UPDATE InboundMessageAttachments ")
	               _T("SET AttachmentID=?,InboundMessageID=? ")
	               _T("WHERE InboundMessageAttachmentID=?") );
	return query.GetRowCount();
}

int TInboundMessageAttachments::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InboundMessageAttachmentID );
	query.Execute( _T("DELETE FROM InboundMessageAttachments ")
	               _T("WHERE InboundMessageAttachmentID=?") );
	return query.GetRowCount();
}

int TInboundMessageAttachments::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AttachmentID );
	BINDCOL_LONG( query, m_InboundMessageID );
	BINDPARAM_LONG( query, m_InboundMessageAttachmentID );
	query.Execute( _T("SELECT AttachmentID,InboundMessageID ")
	               _T("FROM InboundMessageAttachments ")
	               _T("WHERE InboundMessageAttachmentID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TInboundMessageAttachments::GetLongData( CODBCQuery& query )
{
}

void TInboundMessageAttachments::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_InboundMessageAttachmentID );
	BINDCOL_LONG( query, m_AttachmentID );
	BINDCOL_LONG( query, m_InboundMessageID );
	query.Execute( _T("SELECT InboundMessageAttachmentID,AttachmentID,InboundMessageID ")
	               _T("FROM InboundMessageAttachments ")
	               _T("ORDER BY InboundMessageAttachmentID") );
}

/* class TInboundMessageQueue - 
		Queue of inbound messages retrieved by the email gateway service.
	 */
void TInboundMessageQueue::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InboundMessageID );
	query.Execute( _T("INSERT INTO InboundMessageQueue ")
	               _T("(InboundMessageID) ")
	               _T("VALUES")
	               _T("(?)") );
	m_InboundMessageQueueID = query.GetLastInsertedID();
}

int TInboundMessageQueue::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InboundMessageID );
	BINDPARAM_LONG( query, m_InboundMessageQueueID );
	query.Execute( _T("UPDATE InboundMessageQueue ")
	               _T("SET InboundMessageID=? ")
	               _T("WHERE InboundMessageQueueID=?") );
	return query.GetRowCount();
}

int TInboundMessageQueue::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InboundMessageQueueID );
	query.Execute( _T("DELETE FROM InboundMessageQueue ")
	               _T("WHERE InboundMessageQueueID=?") );
	return query.GetRowCount();
}

int TInboundMessageQueue::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_InboundMessageID );
	BINDPARAM_LONG( query, m_InboundMessageQueueID );
	query.Execute( _T("SELECT InboundMessageID ")
	               _T("FROM InboundMessageQueue ")
	               _T("WHERE InboundMessageQueueID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TInboundMessageQueue::GetLongData( CODBCQuery& query )
{
}

void TInboundMessageQueue::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_InboundMessageQueueID );
	BINDCOL_LONG( query, m_InboundMessageID );
	query.Execute( _T("SELECT InboundMessageQueueID,InboundMessageID ")
	               _T("FROM InboundMessageQueue ")
	               _T("ORDER BY InboundMessageQueueID") );
}

/* class TInboundMessages - 
		Table of inbound messages.
	 */
void TInboundMessages::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_TCHAR( query, m_EmailFrom );
	BINDPARAM_TCHAR( query, m_EmailFromName );
	BINDPARAM_TCHAR( query, m_EmailPrimaryTo );
	BINDPARAM_TIME( query, m_EmailDateTime );
	BINDPARAM_TIME( query, m_DateReceived );
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_TCHAR( query, m_MediaType );
	BINDPARAM_TCHAR( query, m_MediaSubType );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_LONG( query, m_DeletedBy );
	BINDPARAM_TIME( query, m_DeletedTime );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_MessageSourceID );
	BINDPARAM_LONG( query, m_VirusScanStateID );
	BINDPARAM_LONG( query, m_ReplyToMsgID );
	BINDPARAM_BIT( query, m_ReplyToIDIsInbound );
	BINDPARAM_TCHAR( query, m_VirusName );
	BINDPARAM_TEXT( query, m_Body );
	BINDPARAM_TEXT( query, m_PopHeaders );
	BINDPARAM_TEXT( query, m_EmailTo );
	BINDPARAM_TEXT( query, m_EmailCc );
	BINDPARAM_TEXT( query, m_EmailReplyTo );
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_PriorityID );
	BINDPARAM_LONG( query, m_ArchiveID );
	BINDPARAM_LONG( query, m_OriginalTicketBoxID );
	BINDPARAM_LONG( query, m_OriginalTicketCategoryID );
	BINDPARAM_LONG( query, m_OriginalOwnerID );
	BINDPARAM_TINYINT( query, m_ReadReceipt );
	BINDPARAM_TCHAR( query, m_ReadReceiptTo );
	query.Execute( _T("INSERT INTO InboundMessages ")
	               _T("(TicketID,EmailFrom,EmailFromName,EmailPrimaryTo,EmailDateTime,DateReceived,Subject,MediaType,MediaSubType,IsDeleted,DeletedBy,DeletedTime,RoutingRuleID,MessageSourceID,VirusScanStateID,ReplyToMsgID,ReplyToIDIsInbound,VirusName,Body,PopHeaders,EmailTo,EmailCc,EmailReplyTo,ContactID,PriorityID,ArchiveID,OriginalTicketBoxID,OriginalTicketCategoryID,OriginalOwnerID,ReadReceipt,ReadReceiptTo) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_InboundMessageID = query.GetLastInsertedID();
}

int TInboundMessages::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_TCHAR( query, m_EmailFrom );
	BINDPARAM_TCHAR( query, m_EmailFromName );
	BINDPARAM_TCHAR( query, m_EmailPrimaryTo );
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_TCHAR( query, m_MediaType );
	BINDPARAM_TCHAR( query, m_MediaSubType );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_LONG( query, m_DeletedBy );
	BINDPARAM_TIME( query, m_DeletedTime );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_MessageSourceID );
	BINDPARAM_LONG( query, m_VirusScanStateID );
	BINDPARAM_LONG( query, m_ReplyToMsgID );
	BINDPARAM_BIT( query, m_ReplyToIDIsInbound );
	BINDPARAM_TCHAR( query, m_VirusName );
	BINDPARAM_TEXT( query, m_Body );
	BINDPARAM_TEXT( query, m_PopHeaders );
	BINDPARAM_TEXT( query, m_EmailTo );
	BINDPARAM_TEXT( query, m_EmailCc );
	BINDPARAM_TEXT( query, m_EmailReplyTo );
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_PriorityID );
	BINDPARAM_LONG( query, m_ArchiveID );
	BINDPARAM_LONG( query, m_OriginalTicketBoxID );
	BINDPARAM_LONG( query, m_OriginalTicketCategoryID );
	BINDPARAM_LONG( query, m_OriginalOwnerID );
	BINDPARAM_TINYINT( query, m_ReadReceipt );
	BINDPARAM_TCHAR( query, m_ReadReceiptTo );
	BINDPARAM_LONG( query, m_InboundMessageID );
	query.Execute( _T("UPDATE InboundMessages ")
	               _T("SET TicketID=?,EmailFrom=?,EmailFromName=?,EmailPrimaryTo=?,Subject=?,MediaType=?,MediaSubType=?,IsDeleted=?,DeletedBy=?,DeletedTime=?,RoutingRuleID=?,MessageSourceID=?,VirusScanStateID=?,ReplyToMsgID=?,ReplyToIDIsInbound=?,VirusName=?,Body=?,PopHeaders=?,EmailTo=?,EmailCc=?,EmailReplyTo=?,ContactID=?,PriorityID=?,ArchiveID=?,OriginalTicketBoxID=?,OriginalTicketCategoryID=?,OriginalOwnerID=?,ReadReceipt=?,ReadReceiptTo=? ")
	               _T("WHERE InboundMessageID=?") );
	return query.GetRowCount();
}

int TInboundMessages::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InboundMessageID );
	query.Execute( _T("DELETE FROM InboundMessages ")
	               _T("WHERE InboundMessageID=?") );
	return query.GetRowCount();
}

int TInboundMessages::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_TCHAR( query, m_EmailFrom );
	BINDCOL_TCHAR( query, m_EmailFromName );
	BINDCOL_TCHAR( query, m_EmailPrimaryTo );
	BINDCOL_TIME( query, m_EmailDateTime );
	BINDCOL_TIME( query, m_DateReceived );
	BINDCOL_TCHAR( query, m_Subject );
	BINDCOL_TCHAR( query, m_MediaType );
	BINDCOL_TCHAR( query, m_MediaSubType );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_LONG( query, m_DeletedBy );
	BINDCOL_TIME( query, m_DeletedTime );
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDCOL_LONG( query, m_MessageSourceID );
	BINDCOL_LONG( query, m_VirusScanStateID );
	BINDCOL_LONG( query, m_ReplyToMsgID );
	BINDCOL_BIT( query, m_ReplyToIDIsInbound );
	BINDCOL_TCHAR( query, m_VirusName );
	BINDCOL_LONG( query, m_ContactID );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_LONG( query, m_ArchiveID );
	BINDCOL_LONG( query, m_OriginalTicketBoxID );
	BINDCOL_LONG( query, m_OriginalTicketCategoryID );
	BINDCOL_LONG( query, m_OriginalOwnerID );
	BINDCOL_TINYINT( query, m_ReadReceipt );
	BINDCOL_TCHAR( query, m_ReadReceiptTo );
	BINDPARAM_LONG( query, m_InboundMessageID );
	query.Execute( _T("SELECT TicketID,EmailFrom,EmailFromName,EmailPrimaryTo,EmailDateTime,DateReceived,Subject,MediaType,MediaSubType,IsDeleted,DeletedBy,DeletedTime,RoutingRuleID,MessageSourceID,VirusScanStateID,ReplyToMsgID,ReplyToIDIsInbound,VirusName,ContactID,PriorityID,ArchiveID,OriginalTicketBoxID,OriginalTicketCategoryID,OriginalOwnerID,ReadReceipt,ReadReceiptTo,Body,PopHeaders,EmailTo,EmailCc,EmailReplyTo ")
	               _T("FROM InboundMessages ")
	               _T("WHERE InboundMessageID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TInboundMessages::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Body );
	GETDATA_TEXT( query, m_PopHeaders );
	GETDATA_TEXT( query, m_EmailTo );
	GETDATA_TEXT( query, m_EmailCc );
	GETDATA_TEXT( query, m_EmailReplyTo );
}

void TInboundMessages::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_InboundMessageID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_TCHAR( query, m_EmailFrom );
	BINDCOL_TCHAR( query, m_EmailFromName );
	BINDCOL_TCHAR( query, m_EmailPrimaryTo );
	BINDCOL_TIME( query, m_EmailDateTime );
	BINDCOL_TIME( query, m_DateReceived );
	BINDCOL_TCHAR( query, m_Subject );
	BINDCOL_TCHAR( query, m_MediaType );
	BINDCOL_TCHAR( query, m_MediaSubType );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_LONG( query, m_DeletedBy );
	BINDCOL_TIME( query, m_DeletedTime );
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDCOL_LONG( query, m_MessageSourceID );
	BINDCOL_LONG( query, m_VirusScanStateID );
	BINDCOL_LONG( query, m_ReplyToMsgID );
	BINDCOL_BIT( query, m_ReplyToIDIsInbound );
	BINDCOL_TCHAR( query, m_VirusName );
	BINDCOL_LONG( query, m_ContactID );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_LONG( query, m_ArchiveID );
	BINDCOL_LONG( query, m_OriginalTicketBoxID );
	BINDCOL_LONG( query, m_OriginalTicketCategoryID );
	BINDCOL_LONG( query, m_OriginalOwnerID );
	BINDCOL_TINYINT( query, m_ReadReceipt );
	BINDCOL_TCHAR( query, m_ReadReceiptTo );
	query.Execute( _T("SELECT InboundMessageID,TicketID,EmailFrom,EmailFromName,EmailPrimaryTo,EmailDateTime,DateReceived,Subject,MediaType,MediaSubType,IsDeleted,DeletedBy,DeletedTime,RoutingRuleID,MessageSourceID,VirusScanStateID,ReplyToMsgID,ReplyToIDIsInbound,VirusName,ContactID,PriorityID,ArchiveID,OriginalTicketBoxID,OriginalTicketCategoryID,OriginalOwnerID,ReadReceipt,ReadReceiptTo,Body,PopHeaders,EmailTo,EmailCc,EmailReplyTo ")
	               _T("FROM InboundMessages ")
	               _T("ORDER BY InboundMessageID") );
}

//*****************************************************************************
// TInboundMessage Functions

void TInboundMessageRead::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InboundMessageID );
	BINDPARAM_LONG( query, m_AgentID );
	query.Execute( _T("INSERT INTO InboundMessageRead ")
	               _T("(InboundMessageID, AgentID) ")
	               _T("VALUES")
	               _T("(?,?)") );

	m_InboundMessageReadID = query.GetLastInsertedID();
}

int TInboundMessageRead::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InboundMessageID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_InboundMessageReadID );
	query.Execute( _T("UPDATE InboundMessageRead ")
	               _T("SET InboundMessageID=?,AgentID=? ")
	               _T("WHERE InboundMessageReadID=?") );
	return query.GetRowCount();
}

int TInboundMessageRead::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InboundMessageReadID );
	query.Execute( _T("DELETE FROM InboundMessageRead ")
	               _T("WHERE InboundMessageReadID=?") );
	return query.GetRowCount();
}

int TInboundMessageRead::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_InboundMessageID );
	BINDCOL_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_InboundMessageReadID );
	query.Execute( _T("SELECT InboundMessageID, AgentID ")
	               _T("FROM InboundMessageRead ")
	               _T("WHERE InboundMessageReadID=?") );
	nResult = query.Fetch();

	return nResult;
}

void TInboundMessageRead::GetLongData( CODBCQuery& query )
{
}

void TInboundMessageRead::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_InboundMessageReadID );
	BINDCOL_LONG( query, m_InboundMessageID );
	BINDCOL_LONG( query, m_AgentID );
	query.Execute( _T("SELECT InboundMessageReadID, InboundMessageID, AgentID ")
	               _T("FROM InboundMessageRead ")
	               _T("ORDER BY InboundMessageReadID") );
}

// End of TInboundMessageRead Functions
//*****************************************************************************

//*****************************************************************************
// TSRKeywordResults Functions

void TSRKeywordResults::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InboundMessageID );
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_Score );

	query.Execute( _T("INSERT INTO SRKeywordResults ")
	               _T("(InboundMessageID, StandardResponseID, Score) ")
	               _T("VALUES")
	               _T("(?,?,?)") );

	m_SRKeywordResultsID = query.GetLastInsertedID();
}

int TSRKeywordResults::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InboundMessageID );
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_Score );
	BINDPARAM_LONG( query, m_SRKeywordResultsID );
	query.Execute( _T("UPDATE SRKeywordResults ")
	               _T("SET InboundMessageID=?,StandardResponseID=?,Score=? ")
	               _T("WHERE SRKeywordResultsID=?") );

	return query.GetRowCount();
}

int TSRKeywordResults::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_SRKeywordResultsID );
	query.Execute( _T("DELETE FROM SRKeywordResults ")
	               _T("WHERE SRKeywordResultsID=?") );
	return query.GetRowCount();
}

int TSRKeywordResults::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_InboundMessageID );
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_Score );
	BINDPARAM_LONG( query, m_SRKeywordResultsID );
	query.Execute( _T("SELECT InboundMessageID, StandardResponseID, Score ")
	               _T("FROM SRKeywordResults ")
	               _T("WHERE SRKeywordResultsID=?") );
	nResult = query.Fetch();

	return nResult;
}

void TSRKeywordResults::GetLongData( CODBCQuery& query )
{
}

void TSRKeywordResults::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_SRKeywordResultsID );
	BINDCOL_LONG( query, m_InboundMessageID );
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_Score );
	query.Execute( _T("SELECT SRKeywordResultsID, InboundMessageID, StandardResponseID, Score ")
	               _T("FROM SRKeywordResults ")
	               _T("ORDER BY SRKeywordResultsID") );
}

// End of TSRKeywordResults Functions
//*****************************************************************************

//*****************************************************************************
// TTicketNotesRead Functions

void TTicketNotesRead::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketNoteID );
	BINDPARAM_LONG( query, m_AgentID );

	query.Execute( _T("INSERT INTO TicketNotesRead ")
	               _T("(TicketNoteID, AgentID) ")
	               _T("VALUES")
	               _T("(?,?)") );

	m_TicketNotesReadID = query.GetLastInsertedID();
}

int TTicketNotesRead::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketNoteID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TicketNotesReadID );
	query.Execute( _T("UPDATE TicketNotesRead ")
	               _T("SET TicketNoteID=?,AgentID=? ")
	               _T("WHERE TicketNotesReadID=?") );
	return query.GetRowCount();
}

int TTicketNotesRead::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketNotesReadID );
	query.Execute( _T("DELETE FROM TicketNotesRead ")
	               _T("WHERE TicketNotesReadID=?") );
	return query.GetRowCount();
}

int TTicketNotesRead::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketNoteID );
	BINDCOL_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TicketNotesReadID );
	query.Execute( _T("SELECT TicketNoteID, AgentID ")
	               _T("FROM TicketNotesRead ")
	               _T("WHERE TicketNotesReadID=?") );
	nResult = query.Fetch();

	return nResult;
}

void TTicketNotesRead::GetLongData( CODBCQuery& query )
{
}

void TTicketNotesRead::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketNotesReadID );
	BINDCOL_LONG( query, m_TicketNoteID );
	BINDCOL_LONG( query, m_AgentID );
	query.Execute( _T("SELECT TicketNotesReadID, TicketNoteID, AgentID ")
	               _T("FROM TicketNotesRead ")
	               _T("ORDER BY TicketNotesReadID") );
}

// End of TTicketNotesRead Functions
//*****************************************************************************

//*****************************************************************************
// TFolders Functions

void TFolders::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name);
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_ParentID );

	query.Execute( _T("INSERT INTO Folders ")
	               _T("(Name, AgentID, ParentID) ")
	               _T("VALUES")
	               _T("(?,?,?)") );

	m_FolderID = query.GetLastInsertedID();
}

int TFolders::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name);
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_ParentID );
	BINDPARAM_LONG( query, m_FolderID );
	query.Execute( _T("UPDATE Folders ")
	               _T("SET Name=?,AgentID=?,ParentID=? ")
	               _T("WHERE FolderID=?") );
	return query.GetRowCount();
}

int TFolders::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_FolderID );
	query.Execute( _T("DELETE FROM Folders ")
	               _T("WHERE FolderID=?") );
	return query.GetRowCount();
}

int TFolders::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR(query, m_Name);
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ParentID );
	BINDPARAM_LONG( query, m_FolderID );
	query.Execute( _T("SELECT Name, AgentID, ParentID ")
	               _T("FROM Folders ")
	               _T("WHERE FolderID=?") );
	nResult = query.Fetch();

	return nResult;
}

void TFolders::GetLongData( CODBCQuery& query )
{
}

void TFolders::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_FolderID );
	BINDCOL_TCHAR(query, m_Name);
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ParentID );
	query.Execute( _T("SELECT FolderID, Name, AgentID, ParentID ")
	               _T("FROM Folders ")
	               _T("ORDER BY FolderID") );
}

// End of TFolders Functions
//*****************************************************************************

/* class TInstantMessages - 
		Where instant messages are stored.
	 */
void TInstantMessages::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TEXT( query, m_Body );
	BINDPARAM_LONG( query, m_FromAgentID );
	BINDPARAM_LONG( query, m_ToAgentID );
	BINDPARAM_TIME( query, m_TimeStamp );
	query.Execute( _T("INSERT INTO InstantMessages ")
	               _T("(Body,FromAgentID,ToAgentID,TimeStamp) ")
	               _T("VALUES")
	               _T("(?,?,?,?)") );
	m_InstantMessageID = query.GetLastInsertedID();
}

int TInstantMessages::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TEXT( query, m_Body );
	BINDPARAM_LONG( query, m_FromAgentID );
	BINDPARAM_LONG( query, m_ToAgentID );
	BINDPARAM_LONG( query, m_InstantMessageID );
	query.Execute( _T("UPDATE InstantMessages ")
	               _T("SET Body=?,FromAgentID=?,ToAgentID=? ")
	               _T("WHERE InstantMessageID=?") );
	return query.GetRowCount();
}

int TInstantMessages::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_InstantMessageID );
	query.Execute( _T("DELETE FROM InstantMessages ")
	               _T("WHERE InstantMessageID=?") );
	return query.GetRowCount();
}

int TInstantMessages::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_FromAgentID );
	BINDCOL_LONG( query, m_ToAgentID );
	BINDCOL_TIME( query, m_TimeStamp );
	BINDPARAM_LONG( query, m_InstantMessageID );
	query.Execute( _T("SELECT FromAgentID,ToAgentID,TimeStamp,Body ")
	               _T("FROM InstantMessages ")
	               _T("WHERE InstantMessageID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TInstantMessages::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Body );
}

void TInstantMessages::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_InstantMessageID );
	BINDCOL_LONG( query, m_FromAgentID );
	BINDCOL_LONG( query, m_ToAgentID );
	BINDCOL_TIME( query, m_TimeStamp );
	query.Execute( _T("SELECT InstantMessageID,FromAgentID,ToAgentID,TimeStamp,Body ")
	               _T("FROM InstantMessages ")
	               _T("ORDER BY InstantMessageID") );
}

/* class TLogEntryTypes - 
		Types of log entries that can be turned on or off.
	 */
void TLogEntryTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_TypeDescrip );
	BINDPARAM_LONG( query, m_SeverityLevels );
	query.Execute( _T("INSERT INTO LogEntryTypes ")
	               _T("(TypeDescrip,SeverityLevels) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_LogEntryTypeID = query.GetLastInsertedID();
}

int TLogEntryTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_TypeDescrip );
	BINDPARAM_LONG( query, m_SeverityLevels );
	BINDPARAM_LONG( query, m_LogEntryTypeID );
	query.Execute( _T("UPDATE LogEntryTypes ")
	               _T("SET TypeDescrip=?,SeverityLevels=? ")
	               _T("WHERE LogEntryTypeID=?") );
	return query.GetRowCount();
}

int TLogEntryTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_LogEntryTypeID );
	query.Execute( _T("DELETE FROM LogEntryTypes ")
	               _T("WHERE LogEntryTypeID=?") );
	return query.GetRowCount();
}

int TLogEntryTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_TypeDescrip );
	BINDCOL_LONG( query, m_SeverityLevels );
	BINDPARAM_LONG( query, m_LogEntryTypeID );
	query.Execute( _T("SELECT TypeDescrip,SeverityLevels ")
	               _T("FROM LogEntryTypes ")
	               _T("WHERE LogEntryTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TLogEntryTypes::GetLongData( CODBCQuery& query )
{
}

void TLogEntryTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_LogEntryTypeID );
	BINDCOL_TCHAR( query, m_TypeDescrip );
	BINDCOL_LONG( query, m_SeverityLevels );
	query.Execute( _T("SELECT LogEntryTypeID,TypeDescrip,SeverityLevels ")
	               _T("FROM LogEntryTypes ")
	               _T("ORDER BY LogEntryTypeID") );
}

/* class TLog - 
		Enumerates the log entries.
	 */
void TLog::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TIME( query, m_LogTime );
	BINDPARAM_LONG( query, m_ServerID );
	BINDPARAM_LONG( query, m_ErrorCode );
	BINDPARAM_LONG( query, m_LogSeverityID );
	BINDPARAM_LONG( query, m_LogEntryTypeID );
	BINDPARAM_TCHAR( query, m_LogText );
	query.Execute( _T("INSERT INTO Log ")
	               _T("(LogTime,ServerID,ErrorCode,LogSeverityID,LogEntryTypeID,LogText) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?)") );
	m_LogID = query.GetLastInsertedID();
}

int TLog::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TIME( query, m_LogTime );
	BINDPARAM_LONG( query, m_ServerID );
	BINDPARAM_LONG( query, m_ErrorCode );
	BINDPARAM_LONG( query, m_LogSeverityID );
	BINDPARAM_LONG( query, m_LogEntryTypeID );
	BINDPARAM_TCHAR( query, m_LogText );
	BINDPARAM_LONG( query, m_LogID );
	query.Execute( _T("UPDATE Log ")
	               _T("SET LogTime=?, ServerID=?,ErrorCode=?,LogSeverityID=?,LogEntryTypeID=?,LogText=? ")
	               _T("WHERE LogID=?") );
	return query.GetRowCount();
}

int TLog::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_LogID );
	query.Execute( _T("DELETE FROM Log ")
	               _T("WHERE LogID=?") );
	return query.GetRowCount();
}

int TLog::DeleteByServer( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ServerID );
	query.Execute( _T("DELETE FROM Log ")
	               _T("WHERE ServerID=?") );
	return query.GetRowCount();
}

int TLog::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TIME( query, m_LogTime );
	BINDCOL_LONG( query, m_ServerID );
	BINDCOL_LONG( query, m_ErrorCode );
	BINDCOL_LONG( query, m_LogSeverityID );
	BINDCOL_LONG( query, m_LogEntryTypeID );
	BINDCOL_TCHAR( query, m_LogText );
	BINDPARAM_LONG( query, m_LogID );
	query.Execute( _T("SELECT LogTime,ServerID,ErrorCode,LogSeverityID,LogEntryTypeID,LogText ")
	               _T("FROM Log ")
	               _T("WHERE LogID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TLog::GetLongData( CODBCQuery& query )
{
}

void TLog::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_LogID );
	BINDCOL_TIME( query, m_LogTime );
	BINDCOL_LONG( query, m_ServerID );
	BINDCOL_LONG( query, m_ErrorCode );
	BINDCOL_LONG( query, m_LogSeverityID );
	BINDCOL_LONG( query, m_LogEntryTypeID );
	BINDCOL_TCHAR( query, m_LogText );
	query.Execute( _T("SELECT LogID,LogTime,ServerID,ErrorCode,LogSeverityID,LogEntryTypeID,LogText ")
	               _T("FROM Log ")
	               _T("ORDER BY LogID") );
}

void TLog::PrepareListByServer( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_LogID );
	BINDCOL_TIME( query, m_LogTime );
	BINDCOL_LONG( query, m_ServerID );
	BINDCOL_LONG( query, m_ErrorCode );
	BINDCOL_LONG( query, m_LogSeverityID );
	BINDCOL_LONG( query, m_LogEntryTypeID );
	BINDCOL_TCHAR( query, m_LogText );
	BINDPARAM_LONG( query, m_ServerID );	
	query.Execute( _T("SELECT LogID,LogTime,ServerID,ErrorCode,LogSeverityID,LogEntryTypeID,LogText ")
	               _T("FROM Log ")
	               _T("WHERE ServerID=? ORDER BY LogID") );
}

/* class TLogSeverity - 
		Enumerates the severity levels associated with log entries.
	 */
void TLogSeverity::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO LogSeverity ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_LogSeverityID = query.GetLastInsertedID();
}

int TLogSeverity::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_LogSeverityID );
	query.Execute( _T("UPDATE LogSeverity ")
	               _T("SET Description=? ")
	               _T("WHERE LogSeverityID=?") );
	return query.GetRowCount();
}

int TLogSeverity::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_LogSeverityID );
	query.Execute( _T("DELETE FROM LogSeverity ")
	               _T("WHERE LogSeverityID=?") );
	return query.GetRowCount();
}

int TLogSeverity::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_LogSeverityID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM LogSeverity ")
	               _T("WHERE LogSeverityID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TLogSeverity::GetLongData( CODBCQuery& query )
{
}

void TLogSeverity::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_LogSeverityID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT LogSeverityID,Description ")
	               _T("FROM LogSeverity ")
	               _T("ORDER BY LogSeverityID") );
}

/* class TMatchFromAddresses - 
		Allows routing rules to match one or more "from" email addresses.
	 */
void TMatchFromAddresses::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	query.Execute( _T("INSERT INTO MatchFromAddresses ")
	               _T("(EmailAddress,RoutingRuleID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_MatchID = query.GetLastInsertedID();
}

int TMatchFromAddresses::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("UPDATE MatchFromAddresses ")
	               _T("SET EmailAddress=?,RoutingRuleID=? ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchFromAddresses::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("DELETE FROM MatchFromAddresses ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchFromAddresses::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("SELECT EmailAddress,RoutingRuleID ")
	               _T("FROM MatchFromAddresses ")
	               _T("WHERE MatchID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMatchFromAddresses::GetLongData( CODBCQuery& query )
{
}

void TMatchFromAddresses::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MatchID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_RoutingRuleID );
	query.Execute( _T("SELECT MatchID,EmailAddress,RoutingRuleID ")
	               _T("FROM MatchFromAddresses ")
	               _T("ORDER BY MatchID") );
}

/* class TMatchFromAddressP - 
		Allows routing rules to match one or more "from" email addresses.
	 */
void TMatchFromAddressP::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_ProcessingRuleID );
	query.Execute( _T("INSERT INTO MatchFromAddressP ")
	               _T("(EmailAddress,ProcessingRuleID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_MatchID = query.GetLastInsertedID();
}

int TMatchFromAddressP::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_ProcessingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("UPDATE MatchFromAddressP ")
	               _T("SET EmailAddress=?,ProcessingRuleID=? ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchFromAddressP::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("DELETE FROM MatchFromAddressP ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchFromAddressP::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_ProcessingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("SELECT EmailAddress,ProcessingRuleID ")
	               _T("FROM MatchFromAddressP ")
	               _T("WHERE MatchID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMatchFromAddressP::GetLongData( CODBCQuery& query )
{
}

void TMatchFromAddressP::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MatchID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_ProcessingRuleID );
	query.Execute( _T("SELECT MatchID,EmailAddress,ProcessingRuleID ")
	               _T("FROM MatchFromAddressP ")
	               _T("ORDER BY MatchID") );
}

/* class TMatchText - 
		Contains text which routing rules attempt to match in various locations (e.g. msg body, subject, etc.).
	 */
void TMatchText::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TEXT( query, m_MatchText );
	BINDPARAM_TINYINT( query, m_MatchLocation );
	BINDPARAM_BIT( query, m_IsRegEx );			
	BINDPARAM_LONG( query, m_RoutingRuleID );
	query.Execute( _T("INSERT INTO MatchText ")
	               _T("(MatchText,MatchLocation,IsRegEx,RoutingRuleID) ")
	               _T("VALUES")
	               _T("(?,?,?,?)") );
	m_MatchID = query.GetLastInsertedID();
}

int TMatchText::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TEXT( query, m_MatchText );
	BINDPARAM_TINYINT( query, m_MatchLocation );
	BINDPARAM_BIT( query, m_IsRegEx );			
	BINDPARAM_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("UPDATE MatchText ")
	               _T("SET MatchText=?,MatchLocation=?,IsRegEx=?,RoutingRuleID=? ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchText::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("DELETE FROM MatchText ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchText::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TINYINT( query, m_MatchLocation );
	BINDCOL_BIT( query, m_IsRegEx );
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("SELECT MatchLocation,IsRegEx,RoutingRuleID,MatchText ")
	               _T("FROM MatchText ")
	               _T("WHERE MatchID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMatchText::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_MatchText );
}

void TMatchText::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MatchID );
	BINDCOL_TINYINT( query, m_MatchLocation );
	BINDCOL_LONG( query, m_RoutingRuleID );
	query.Execute( _T("SELECT MatchID,MatchLocation,RoutingRuleID,MatchText ")
	               _T("FROM MatchText ")
	               _T("ORDER BY MatchID") );
}

/* class TMatchTextP - 
		Contains text which processing rules attempt to match in various locations (e.g. msg body, subject, etc.).
	 */
void TMatchTextP::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TEXT( query, m_MatchText );
	BINDPARAM_TINYINT( query, m_MatchLocation );
	BINDPARAM_BIT( query, m_IsRegEx );			
	BINDPARAM_LONG( query, m_ProcessingRuleID );
	query.Execute( _T("INSERT INTO MatchTextP ")
	               _T("(MatchText,MatchLocation,IsRegEx,ProcessingRuleID) ")
	               _T("VALUES")
	               _T("(?,?,?,?)") );
	m_MatchID = query.GetLastInsertedID();
}

int TMatchTextP::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TEXT( query, m_MatchText );
	BINDPARAM_TINYINT( query, m_MatchLocation );
	BINDPARAM_BIT( query, m_IsRegEx );			
	BINDPARAM_LONG( query, m_ProcessingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("UPDATE MatchTextP ")
	               _T("SET MatchText=?,MatchLocation=?,IsRegEx=?,ProcessingRuleID=? ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchTextP::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("DELETE FROM MatchTextP ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchTextP::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TINYINT( query, m_MatchLocation );
	BINDCOL_BIT( query, m_IsRegEx );
	BINDCOL_LONG( query, m_ProcessingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("SELECT MatchLocation,IsRegEx,ProcessingRuleID,MatchText ")
	               _T("FROM MatchTextP ")
	               _T("WHERE MatchID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMatchTextP::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_MatchText );
}

void TMatchTextP::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MatchID );
	BINDCOL_TINYINT( query, m_MatchLocation );
	BINDCOL_LONG( query, m_ProcessingRuleID );
	query.Execute( _T("SELECT MatchID,MatchLocation,ProcessingRuleID,MatchText ")
	               _T("FROM MatchTextP ")
	               _T("ORDER BY MatchID") );
}

/* class TMatchToAddresses - 
		Table of "to" addresses to be matched by the routing rules.
	 */
void TMatchToAddresses::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	query.Execute( _T("INSERT INTO MatchToAddresses ")
	               _T("(EmailAddress,RoutingRuleID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_MatchID = query.GetLastInsertedID();
}

int TMatchToAddresses::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("UPDATE MatchToAddresses ")
	               _T("SET EmailAddress=?,RoutingRuleID=? ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchToAddresses::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("DELETE FROM MatchToAddresses ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchToAddresses::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("SELECT EmailAddress,RoutingRuleID ")
	               _T("FROM MatchToAddresses ")
	               _T("WHERE MatchID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMatchToAddresses::GetLongData( CODBCQuery& query )
{
}

void TMatchToAddresses::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MatchID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_RoutingRuleID );
	query.Execute( _T("SELECT MatchID,EmailAddress,RoutingRuleID ")
	               _T("FROM MatchToAddresses ")
	               _T("ORDER BY MatchID") );
}

/* class TMatchToAddressP - 
		Table of "to" addresses to be matched by the routing rules.
	 */
void TMatchToAddressP::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_ProcessingRuleID );
	query.Execute( _T("INSERT INTO MatchToAddressP ")
	               _T("(EmailAddress,ProcessingRuleID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_MatchID = query.GetLastInsertedID();
}

int TMatchToAddressP::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_LONG( query, m_ProcessingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("UPDATE MatchToAddresses ")
	               _T("SET EmailAddress=?,ProcessingRuleID=? ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchToAddressP::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("DELETE FROM MatchToAddressP ")
	               _T("WHERE MatchID=?") );
	return query.GetRowCount();
}

int TMatchToAddressP::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_ProcessingRuleID );
	BINDPARAM_LONG( query, m_MatchID );
	query.Execute( _T("SELECT EmailAddress,ProcessingRuleID ")
	               _T("FROM MatchToAddressP ")
	               _T("WHERE MatchID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMatchToAddressP::GetLongData( CODBCQuery& query )
{
}

void TMatchToAddressP::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MatchID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_LONG( query, m_ProcessingRuleID );
	query.Execute( _T("SELECT MatchID,EmailAddress,ProcessingRuleID ")
	               _T("FROM MatchToAddressP ")
	               _T("ORDER BY MatchID") );
}

/* class TMessageDestinations - 
		Message destinations represent outbound message gateways.
	 */
void TMessageDestinations::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_ServerAddress );
	BINDPARAM_BIT( query, m_UseSMTPAuth );
	BINDPARAM_TCHAR( query, m_AuthUser );
	BINDPARAM_TCHAR( query, m_AuthPass );
	BINDPARAM_LONG( query, m_SMTPPort );
	BINDPARAM_BIT( query, m_IsActive );
	BINDPARAM_LONG( query, m_ConnTimeoutSecs );
	BINDPARAM_LONG( query, m_MaxOutboundMsgSize );
	BINDPARAM_LONG( query, m_MaxSendRetryHours );
	BINDPARAM_TIME( query, m_LastProcessInterval );
	BINDPARAM_LONG( query, m_ProcessFreqMins );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_ServerID );
	BINDPARAM_BIT( query, m_IsSSL );
	BINDPARAM_LONG( query, m_SSLMode );
	BINDPARAM_LONG( query, m_OfficeHours );
	BINDPARAM_LONG( query, m_ErrorCode );
	BINDPARAM_TEXT( query, m_AccessToken );
	BINDPARAM_TIME( query, m_AccessTokenExpire );
	BINDPARAM_TEXT( query, m_RefreshToken );
	BINDPARAM_TIME( query, m_RefreshTokenExpire );
	BINDPARAM_LONG( query, m_OAuthHostID );
	query.Execute( _T("INSERT INTO MessageDestinations ")
	               _T("(ServerAddress,UseSMTPAuth,AuthUser,AuthPass,SMTPPort,IsActive,ConnTimeoutSecs,MaxOutboundMsgSize,MaxSendRetryHours,LastProcessInterval,ProcessFreqMins,Description,ServerID,IsSSL,SSLMode,OfficeHours,ErrorCode,AccessToken,AccessTokenExpire,RefreshToken,RefreshTokenExpire,OAuthHostID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_MessageDestinationID = query.GetLastInsertedID();
}

int TMessageDestinations::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_ServerAddress );
	BINDPARAM_BIT( query, m_UseSMTPAuth );
	BINDPARAM_TCHAR( query, m_AuthUser );
	BINDPARAM_TCHAR( query, m_AuthPass );
	BINDPARAM_LONG( query, m_SMTPPort );
	BINDPARAM_BIT( query, m_IsActive );
	BINDPARAM_LONG( query, m_ConnTimeoutSecs );
	BINDPARAM_LONG( query, m_MaxOutboundMsgSize );
	BINDPARAM_LONG( query, m_MaxSendRetryHours );
	BINDPARAM_TIME( query, m_LastProcessInterval );
	BINDPARAM_LONG( query, m_ProcessFreqMins );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_ServerID );
	BINDPARAM_BIT( query, m_IsSSL );
	BINDPARAM_LONG( query, m_SSLMode );
	BINDPARAM_LONG( query, m_OfficeHours );
	BINDPARAM_LONG( query, m_ErrorCode );
	BINDPARAM_TEXT( query, m_AccessToken );
	BINDPARAM_TIME( query, m_AccessTokenExpire );
	BINDPARAM_TEXT( query, m_RefreshToken );
	BINDPARAM_TIME( query, m_RefreshTokenExpire );
	BINDPARAM_LONG( query, m_OAuthHostID );
	BINDPARAM_LONG( query, m_MessageDestinationID );	
	query.Execute( _T("UPDATE MessageDestinations ")
	               _T("SET ServerAddress=?,UseSMTPAuth=?,AuthUser=?,AuthPass=?,SMTPPort=?,IsActive=?,ConnTimeoutSecs=?,MaxOutboundMsgSize=?,MaxSendRetryHours=?,LastProcessInterval=?,ProcessFreqMins=?,Description=?,ServerID=?,IsSSL=?,SSLMode=?,OfficeHours=?,ErrorCode=?,AccessToken=?,AccessTokenExpire=?,RefreshToken=?,RefreshTokenExpire=?,OAuthHostID=? ")
	               _T("WHERE MessageDestinationID=?") );
	return query.GetRowCount();
}

int TMessageDestinations::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MessageDestinationID );
	query.Execute( _T("DELETE FROM MessageDestinations ")
	               _T("WHERE MessageDestinationID=?") );
	return query.GetRowCount();
}

int TMessageDestinations::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_ServerAddress );
	BINDCOL_BIT( query, m_UseSMTPAuth );
	BINDCOL_TCHAR( query, m_AuthUser );
	BINDCOL_TCHAR( query, m_AuthPass );
	BINDCOL_LONG( query, m_SMTPPort );
	BINDCOL_BIT( query, m_IsActive );
	BINDCOL_LONG( query, m_ConnTimeoutSecs );
	BINDCOL_LONG( query, m_MaxOutboundMsgSize );
	BINDCOL_LONG( query, m_MaxSendRetryHours );
	BINDCOL_TIME( query, m_LastProcessInterval );
	BINDCOL_LONG( query, m_ProcessFreqMins );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_ServerID );
	BINDCOL_BIT( query, m_IsSSL );
	BINDCOL_LONG( query, m_SSLMode);
	BINDCOL_LONG( query, m_OfficeHours);
	BINDCOL_LONG( query, m_ErrorCode);
	BINDCOL_TIME( query, m_AccessTokenExpire );
	BINDCOL_TIME( query, m_RefreshTokenExpire );
	BINDCOL_LONG( query, m_OAuthHostID );
	BINDPARAM_LONG( query, m_MessageDestinationID );
	query.Execute( _T("SELECT ServerAddress,UseSMTPAuth,AuthUser,AuthPass,SMTPPort,IsActive,ConnTimeoutSecs,MaxOutboundMsgSize,MaxSendRetryHours,LastProcessInterval,ProcessFreqMins,Description,ServerID,IsSSL,SSLMode,OfficeHours,ErrorCode,AccessTokenExpire,RefreshTokenExpire,OAuthHostID,AccessToken,RefreshToken ")
	               _T("FROM MessageDestinations ")
	               _T("WHERE MessageDestinationID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMessageDestinations::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_AccessToken );
	GETDATA_TEXT( query, m_RefreshToken );
}

void TMessageDestinations::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MessageDestinationID );
	BINDCOL_TCHAR( query, m_ServerAddress );
	BINDCOL_BIT( query, m_UseSMTPAuth );
	BINDCOL_TCHAR( query, m_AuthUser );
	BINDCOL_TCHAR( query, m_AuthPass );
	BINDCOL_LONG( query, m_SMTPPort );
	BINDCOL_BIT( query, m_IsActive );
	BINDCOL_LONG( query, m_ConnTimeoutSecs );
	BINDCOL_LONG( query, m_MaxOutboundMsgSize );
	BINDCOL_LONG( query, m_MaxSendRetryHours );
	BINDCOL_TIME( query, m_LastProcessInterval );
	BINDCOL_LONG( query, m_ProcessFreqMins );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_ServerID );
	BINDCOL_BIT( query, m_IsSSL );
	BINDCOL_LONG( query, m_SSLMode);
	BINDCOL_LONG( query, m_OfficeHours);
	BINDCOL_LONG( query, m_ErrorCode);
	BINDCOL_TIME( query, m_AccessTokenExpire );
	BINDCOL_TIME( query, m_RefreshTokenExpire );
	BINDCOL_LONG( query, m_OAuthHostID );
	query.Execute( _T("SELECT MessageDestinationID,ServerAddress,UseSMTPAuth,AuthUser,AuthPass,SMTPPort,IsActive,ConnTimeoutSecs,MaxOutboundMsgSize,MaxSendRetryHours,LastProcessInterval,ProcessFreqMins,Description,ServerID,IsSSL,SSLMode,OfficeHours,ErrorCode,AccessTokenExpire,RefreshTokenExpire,OAuthHostID ")
	               _T("FROM MessageDestinations ")
	               _T("ORDER BY MessageDestinationID") );
}

/* class TMessageSourceTypes - 
 	     Classifies message sources for routing purposes
	 */
void TMessageSourceTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO MessageSourceTypes ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_MessageSourceTypeID = query.GetLastInsertedID();
}

int TMessageSourceTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_MessageSourceTypeID );
	query.Execute( _T("UPDATE MessageSourceTypes ")
	               _T("SET Description=? ")
	               _T("WHERE MessageSourceTypeID=?") );
	return query.GetRowCount();
}

int TMessageSourceTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MessageSourceTypeID );
	query.Execute( _T("DELETE FROM MessageSourceTypes ")
	               _T("WHERE MessageSourceTypeID=?") );
	return query.GetRowCount();
}

int TMessageSourceTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_MessageSourceTypeID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM MessageSourceTypes ")
	               _T("WHERE MessageSourceTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMessageSourceTypes::GetLongData( CODBCQuery& query )
{
}

void TMessageSourceTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MessageSourceTypeID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT MessageSourceTypeID,Description ")
	               _T("FROM MessageSourceTypes ")
	               _T("ORDER BY MessageSourceTypeID") );
}

/* class TMessageSources - 
		Designates sources for email messages.
	 */
void TMessageSources::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_RemoteAddress );
	BINDPARAM_LONG( query, m_RemotePort );
	BINDPARAM_TCHAR( query, m_AuthUserName );
	BINDPARAM_TCHAR( query, m_AuthPassword );
	BINDPARAM_BIT( query, m_IsAPOP );
	BINDPARAM_BIT( query, m_LeaveCopiesOnServer );
	BINDPARAM_BIT( query, m_IsActive );
	BINDPARAM_LONG( query, m_CheckFreqMins );
	BINDPARAM_LONG( query, m_MaxInboundMsgSize );
	BINDPARAM_LONG( query, m_ConnTimeoutSecs );
	BINDPARAM_TIME( query, m_LastChecked );
	BINDPARAM_LONG( query, m_MessageDestinationID );
	BINDPARAM_LONG( query, m_MessageSourceTypeID );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_UseReplyTo );
	BINDPARAM_LONG( query, m_ServerID );
	BINDPARAM_BIT( query, m_IsSSL );
	BINDPARAM_LONG( query, m_OfficeHours );
	BINDPARAM_LONG( query, m_ZipAttach );
	BINDPARAM_LONG( query, m_DupMsg );
	BINDPARAM_LONG( query, m_LeaveCopiesDays );
	BINDPARAM_LONG( query, m_SkipDownloadDays );
	BINDPARAM_LONG( query, m_DateFilters );
	BINDPARAM_LONG( query, m_ErrorCode );
	BINDPARAM_TEXT( query, m_AccessToken );
	BINDPARAM_TIME( query, m_AccessTokenExpire );
	BINDPARAM_TEXT( query, m_RefreshToken );
	BINDPARAM_TIME( query, m_RefreshTokenExpire );
	BINDPARAM_LONG( query, m_OAuthHostID );
	query.Execute( _T("INSERT INTO MessageSources ")
	               _T("(RemoteAddress,RemotePort,AuthUserName,AuthPassword,IsAPOP,LeaveCopiesOnServer,IsActive,CheckFreqMins,MaxInboundMsgSize,ConnTimeoutSecs,LastChecked,MessageDestinationID,MessageSourceTypeID,Description,UseReplyTo,ServerID,IsSSL,OfficeHours,ZipAttach,DupMsg,LeaveCopiesDays,SkipDownloadDays,DateFilters,ErrorCode,AccessToken,AccessTokenExpire,RefreshToken,RefreshTokenExpire,OAuthHostID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_MessageSourceID = query.GetLastInsertedID();
}

int TMessageSources::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_RemoteAddress );
	BINDPARAM_LONG( query, m_RemotePort );
	BINDPARAM_TCHAR( query, m_AuthUserName );
	BINDPARAM_TCHAR( query, m_AuthPassword );
	BINDPARAM_BIT( query, m_IsAPOP );
	BINDPARAM_BIT( query, m_LeaveCopiesOnServer );
	BINDPARAM_BIT( query, m_IsActive );
	BINDPARAM_LONG( query, m_CheckFreqMins );
	BINDPARAM_LONG( query, m_MaxInboundMsgSize );
	BINDPARAM_LONG( query, m_ConnTimeoutSecs );
	BINDPARAM_TIME( query, m_LastChecked );
	BINDPARAM_LONG( query, m_MessageDestinationID );
	BINDPARAM_LONG( query, m_MessageSourceTypeID );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_UseReplyTo );
	BINDPARAM_LONG( query, m_ServerID );
	BINDPARAM_BIT( query, m_IsSSL );
	BINDPARAM_LONG( query, m_OfficeHours );
	BINDPARAM_LONG( query, m_ZipAttach );
	BINDPARAM_LONG( query, m_DupMsg );
	BINDPARAM_LONG( query, m_LeaveCopiesDays );
	BINDPARAM_LONG( query, m_SkipDownloadDays );
	BINDPARAM_LONG( query, m_DateFilters );
	BINDPARAM_LONG( query, m_ErrorCode );
	BINDPARAM_TEXT( query, m_AccessToken );
	BINDPARAM_TIME( query, m_AccessTokenExpire );
	BINDPARAM_TEXT( query, m_RefreshToken );
	BINDPARAM_TIME( query, m_RefreshTokenExpire );
	BINDPARAM_LONG( query, m_OAuthHostID );
	BINDPARAM_LONG( query, m_MessageSourceID );
	query.Execute( _T("UPDATE MessageSources ")
	               _T("SET RemoteAddress=?,RemotePort=?,AuthUserName=?,AuthPassword=?,IsAPOP=?,LeaveCopiesOnServer=?,IsActive=?,CheckFreqMins=?,MaxInboundMsgSize=?,ConnTimeoutSecs=?,LastChecked=?,MessageDestinationID=?,MessageSourceTypeID=?,Description=?,UseReplyTo=?,ServerID=?,IsSSL=?,OfficeHours=?,ZipAttach=?,DupMsg=?,LeaveCopiesDays=?,SkipDownloadDays=?,DateFilters=?,ErrorCode=?,AccessToken=?,AccessTokenExpire=?,RefreshToken=?,RefreshTokenExpire=?,OAuthHostID=? ")
	               _T("WHERE MessageSourceID=?") );
	return query.GetRowCount();
}

int TMessageSources::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MessageSourceID );
	query.Execute( _T("DELETE FROM MessageSources ")
	               _T("WHERE MessageSourceID=?") );
	return query.GetRowCount();
}

int TMessageSources::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_RemoteAddress );
	BINDCOL_LONG( query, m_RemotePort );
	BINDCOL_TCHAR( query, m_AuthUserName );
	BINDCOL_TCHAR( query, m_AuthPassword );
	BINDCOL_BIT( query, m_IsAPOP );
	BINDCOL_BIT( query, m_LeaveCopiesOnServer );
	BINDCOL_BIT( query, m_IsActive );
	BINDCOL_LONG( query, m_CheckFreqMins );
	BINDCOL_LONG( query, m_MaxInboundMsgSize );
	BINDCOL_LONG( query, m_ConnTimeoutSecs );
	BINDCOL_TIME( query, m_LastChecked );
	BINDCOL_LONG( query, m_MessageDestinationID );
	BINDCOL_LONG( query, m_MessageSourceTypeID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_UseReplyTo );
	BINDCOL_LONG( query, m_ServerID );
	BINDCOL_BIT( query, m_IsSSL );
	BINDCOL_LONG( query, m_OfficeHours );
	BINDCOL_LONG( query, m_ZipAttach );
	BINDCOL_LONG( query, m_DupMsg );
	BINDCOL_LONG( query, m_LeaveCopiesDays );
	BINDCOL_LONG( query, m_SkipDownloadDays );
	BINDCOL_LONG( query, m_DateFilters );
	BINDCOL_LONG( query, m_ErrorCode );
	BINDCOL_TIME( query, m_AccessTokenExpire );
	BINDCOL_TIME( query, m_RefreshTokenExpire );
	BINDCOL_LONG( query, m_OAuthHostID );
	BINDPARAM_LONG( query, m_MessageSourceID );
	query.Execute( _T("SELECT RemoteAddress,RemotePort,AuthUserName,AuthPassword,IsAPOP,LeaveCopiesOnServer,IsActive,CheckFreqMins,MaxInboundMsgSize,ConnTimeoutSecs,LastChecked,MessageDestinationID,MessageSourceTypeID,Description,UseReplyTo,ServerID,IsSSL,OfficeHours,ZipAttach,DupMsg,LeaveCopiesDays,SkipDownloadDays,DateFilters,ErrorCode,AccessTokenExpire,RefreshTokenExpire,OAuthHostID,AccessToken,RefreshToken ")
	               _T("FROM MessageSources ")
	               _T("WHERE MessageSourceID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMessageSources::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_AccessToken );
	GETDATA_TEXT( query, m_RefreshToken );	
}

void TMessageSources::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MessageSourceID );
	BINDCOL_TCHAR( query, m_RemoteAddress );
	BINDCOL_LONG( query, m_RemotePort );
	BINDCOL_TCHAR( query, m_AuthUserName );
	BINDCOL_TCHAR( query, m_AuthPassword );
	BINDCOL_BIT( query, m_IsAPOP );
	BINDCOL_BIT( query, m_LeaveCopiesOnServer );
	BINDCOL_BIT( query, m_IsActive );
	BINDCOL_LONG( query, m_CheckFreqMins );
	BINDCOL_LONG( query, m_MaxInboundMsgSize );
	BINDCOL_LONG( query, m_ConnTimeoutSecs );
	BINDCOL_TIME( query, m_LastChecked );
	BINDCOL_LONG( query, m_MessageDestinationID );
	BINDCOL_LONG( query, m_MessageSourceTypeID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_UseReplyTo );
	BINDCOL_LONG( query, m_ServerID );
	BINDCOL_BIT( query, m_IsSSL );
	BINDCOL_LONG( query, m_OfficeHours );
	BINDCOL_LONG( query, m_ZipAttach );
	BINDCOL_LONG( query, m_DupMsg );
	BINDCOL_LONG( query, m_LeaveCopiesDays );
	BINDCOL_LONG( query, m_SkipDownloadDays );
	BINDCOL_LONG( query, m_DateFilters );
	BINDCOL_LONG( query, m_ErrorCode );
	BINDCOL_TIME( query, m_AccessTokenExpire );
	BINDCOL_TIME( query, m_RefreshTokenExpire );
	BINDCOL_LONG( query, m_OAuthHostID );
	query.Execute( _T("SELECT MessageSourceID,RemoteAddress,RemotePort,AuthUserName,AuthPassword,IsAPOP,LeaveCopiesOnServer,IsActive,CheckFreqMins,MaxInboundMsgSize,ConnTimeoutSecs,LastChecked,MessageDestinationID,MessageSourceTypeID,Description,UseReplyTo,ServerID,IsSSL,OfficeHours,ZipAttach,DupMsg,LeaveCopiesDays,SkipDownloadDays,DateFilters,ErrorCode,AccessTokenExpire,RefreshTokenExpire,OAuthHostID ")
	               _T("FROM MessageSources ")
	               _T("ORDER BY MessageSourceID") );
}

/* class TMsgApprovalQueue - 
		Queues message approval requests.
	 */
void TMsgApprovalQueue::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMessageID );
	BINDPARAM_TIME( query, m_DateRequested );
	BINDPARAM_TIME( query, m_DateHandled );
	BINDPARAM_BIT( query, m_IsApproved );
	BINDPARAM_LONG( query, m_RequestAgentID );
	BINDPARAM_LONG( query, m_ApproverAgentID );
	BINDPARAM_TEXT( query, m_Note );
	query.Execute( _T("INSERT INTO MsgApprovalQueue ")
	               _T("(OutboundMessageID,DateRequested,DateHandled,IsApproved,RequestAgentID,ApproverAgentID,Note) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?)") );
	m_MsgApprovalQueueID = query.GetLastInsertedID();
}

int TMsgApprovalQueue::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMessageID );
	BINDPARAM_TIME( query, m_DateRequested );
	BINDPARAM_TIME( query, m_DateHandled );
	BINDPARAM_BIT( query, m_IsApproved );
	BINDPARAM_LONG( query, m_RequestAgentID );
	BINDPARAM_LONG( query, m_ApproverAgentID );
	BINDPARAM_TEXT( query, m_Note );
	BINDPARAM_LONG( query, m_MsgApprovalQueueID );
	query.Execute( _T("UPDATE MsgApprovalQueue ")
	               _T("SET OutboundMessageID=?,DateRequested=?,DateHandled=?,IsApproved=?,RequestAgentID=?,ApproverAgentID=?,Note=? ")
	               _T("WHERE MsgApprovalQueueID=?") );
	return query.GetRowCount();
}

int TMsgApprovalQueue::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MsgApprovalQueueID );
	query.Execute( _T("DELETE FROM MsgApprovalQueue ")
	               _T("WHERE MsgApprovalQueueID=?") );
	return query.GetRowCount();
}

int TMsgApprovalQueue::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_OutboundMessageID );
	BINDCOL_TIME( query, m_DateRequested );
	BINDCOL_TIME( query, m_DateHandled );
	BINDCOL_BIT( query, m_IsApproved );
	BINDCOL_LONG( query, m_RequestAgentID );
	BINDCOL_LONG( query, m_ApproverAgentID );
	BINDPARAM_LONG( query, m_MsgApprovalQueueID );
	query.Execute( _T("SELECT OutboundMessageID,DateRequested,DateHandled,IsApproved,RequestAgentID,ApproverAgentID,Note ")
	               _T("FROM MsgApprovalQueue ")
	               _T("WHERE MsgApprovalQueueID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMsgApprovalQueue::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Note );
}

void TMsgApprovalQueue::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MsgApprovalQueueID );
	BINDCOL_LONG( query, m_OutboundMessageID );
	BINDCOL_TIME( query, m_DateRequested );
	BINDCOL_TIME( query, m_DateHandled );
	BINDCOL_BIT( query, m_IsApproved );
	BINDCOL_LONG( query, m_RequestAgentID );
	BINDCOL_LONG( query, m_ApproverAgentID );
	query.Execute( _T("SELECT MsgApprovalQueueID,OutboundMessageID,DateRequested,DateHandled,IsApproved,RequestAgentID,ApproverAgentID,Note ")
	               _T("FROM MsgApprovalQueue ")
	               _T("ORDER BY MsgApprovalQueueID") );
}

/* class TObjectTypes - 
		Object types describe the various types of securable objects in the system.
	 */
void TObjectTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO ObjectTypes ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_ObjectTypeID = query.GetLastInsertedID();
}

int TObjectTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_ObjectTypeID );
	query.Execute( _T("UPDATE ObjectTypes ")
	               _T("SET Description=? ")
	               _T("WHERE ObjectTypeID=?") );
	return query.GetRowCount();
}

int TObjectTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ObjectTypeID );
	query.Execute( _T("DELETE FROM ObjectTypes ")
	               _T("WHERE ObjectTypeID=?") );
	return query.GetRowCount();
}

int TObjectTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_ObjectTypeID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM ObjectTypes ")
	               _T("WHERE ObjectTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TObjectTypes::GetLongData( CODBCQuery& query )
{
}

void TObjectTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ObjectTypeID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT ObjectTypeID,Description ")
	               _T("FROM ObjectTypes ")
	               _T("ORDER BY ObjectTypeID") );
}

/* class TObjects - 
		Securable objects.
	 */
void TObjects::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ActualID );
	BINDPARAM_LONG( query, m_ObjectTypeID );
	BINDPARAM_BIT( query, m_BuiltIn );
	BINDPARAM_TINYINT( query, m_UseDefaultRights );
	BINDPARAM_TIME( query, m_DateCreated );
	query.Execute( _T("INSERT INTO Objects ")
	               _T("(ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?)") );
	m_ObjectID = query.GetLastInsertedID();
}

int TObjects::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TINYINT( query, m_UseDefaultRights );
	BINDPARAM_LONG( query, m_ObjectID );
	query.Execute( _T("UPDATE Objects ")
	               _T("SET UseDefaultRights=? ")
	               _T("WHERE ObjectID=?") );
	return query.GetRowCount();
}

int TObjects::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ObjectID );
	query.Execute( _T("DELETE FROM Objects ")
	               _T("WHERE ObjectID=?") );
	return query.GetRowCount();
}

int TObjects::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_ActualID );
	BINDCOL_LONG( query, m_ObjectTypeID );
	BINDCOL_BIT( query, m_BuiltIn );
	BINDCOL_TINYINT( query, m_UseDefaultRights );
	BINDCOL_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_ObjectID );
	query.Execute( _T("SELECT ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated ")
	               _T("FROM Objects ")
	               _T("WHERE ObjectID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TObjects::GetLongData( CODBCQuery& query )
{
}

void TObjects::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_LONG( query, m_ActualID );
	BINDCOL_LONG( query, m_ObjectTypeID );
	BINDCOL_BIT( query, m_BuiltIn );
	BINDCOL_TINYINT( query, m_UseDefaultRights );
	BINDCOL_TIME( query, m_DateCreated );
	query.Execute( _T("SELECT ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated ")
	               _T("FROM Objects ")
	               _T("ORDER BY ObjectID") );
}

/* class TOutboundMessageAttachments - 
		Attachments to outbound messages.
	 */
void TOutboundMessageAttachments::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AttachmentID );
	BINDPARAM_LONG( query, m_OutboundMessageID );
	query.Execute( _T("INSERT INTO OutboundMessageAttachments ")
	               _T("(AttachmentID,OutboundMessageID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_OutboundMessageAttachmentID = query.GetLastInsertedID();
}

int TOutboundMessageAttachments::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AttachmentID );
	BINDPARAM_LONG( query, m_OutboundMessageID );
	BINDPARAM_LONG( query, m_OutboundMessageAttachmentID );
	query.Execute( _T("UPDATE OutboundMessageAttachments ")
	               _T("SET AttachmentID=?,OutboundMessageID=? ")
	               _T("WHERE OutboundMessageAttachmentID=?") );
	return query.GetRowCount();
}

int TOutboundMessageAttachments::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMessageAttachmentID );
	query.Execute( _T("DELETE FROM OutboundMessageAttachments ")
	               _T("WHERE OutboundMessageAttachmentID=?") );
	return query.GetRowCount();
}

int TOutboundMessageAttachments::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AttachmentID );
	BINDCOL_LONG( query, m_OutboundMessageID );
	BINDPARAM_LONG( query, m_OutboundMessageAttachmentID );
	query.Execute( _T("SELECT AttachmentID,OutboundMessageID ")
	               _T("FROM OutboundMessageAttachments ")
	               _T("WHERE OutboundMessageAttachmentID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TOutboundMessageAttachments::GetLongData( CODBCQuery& query )
{
}

void TOutboundMessageAttachments::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_OutboundMessageAttachmentID );
	BINDCOL_LONG( query, m_AttachmentID );
	BINDCOL_LONG( query, m_OutboundMessageID );
	query.Execute( _T("SELECT OutboundMessageAttachmentID,AttachmentID,OutboundMessageID ")
	               _T("FROM OutboundMessageAttachments ")
	               _T("ORDER BY OutboundMessageAttachmentID") );
}

/* class TNoteAttachments - 
		Attachments to notes.
	 */
void TNoteAttachments::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AttachmentID );
	BINDPARAM_LONG( query, m_NoteID );
	BINDPARAM_LONG( query, m_NoteTypeID );
	query.Execute( _T("INSERT INTO NoteAttachments ")
	               _T("(AttachmentID,NoteID,NoteTypeID) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_NoteAttachmentID = query.GetLastInsertedID();
}

int TNoteAttachments::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AttachmentID );
	BINDPARAM_LONG( query, m_NoteID );
	BINDPARAM_LONG( query, m_NoteTypeID );
	BINDPARAM_LONG( query, m_NoteAttachmentID );
	query.Execute( _T("UPDATE NoteAttachments ")
	               _T("SET AttachmentID=?,NoteID=?,NoteTypeID=? ")
	               _T("WHERE NoteAttachmentID=?") );
	return query.GetRowCount();
}

int TNoteAttachments::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_NoteAttachmentID );
	query.Execute( _T("DELETE FROM NoteAttachments ")
	               _T("WHERE NoteAttachmentID=?") );
	return query.GetRowCount();
}

int TNoteAttachments::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AttachmentID );
	BINDCOL_LONG( query, m_NoteID );
	BINDCOL_LONG( query, m_NoteTypeID );
	BINDPARAM_LONG( query, m_NoteAttachmentID );
	query.Execute( _T("SELECT AttachmentID,NoteID,NoteTypeID ")
	               _T("FROM NoteAttachments ")
	               _T("WHERE NoteAttachmentID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TNoteAttachments::GetLongData( CODBCQuery& query )
{
}

void TNoteAttachments::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_NoteAttachmentID );
	BINDCOL_LONG( query, m_AttachmentID );
	BINDCOL_LONG( query, m_NoteID );
	BINDCOL_LONG( query, m_NoteTypeID );
	query.Execute( _T("SELECT NoteAttachmentID,AttachmentID,NoteID,NoteTypeID ")
	               _T("FROM NoteAttachments ")
	               _T("ORDER BY NoteAttachmentID") );
}

/* class TOutboundMessageContacts - 
		Relates ticket boxes to contacts.
	 */
void TOutboundMessageContacts::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMessageID );
	BINDPARAM_LONG( query, m_ContactID );
	query.Execute( _T("INSERT INTO OutboundMessageContacts ")
	               _T("(OutboundMessageID,ContactID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_OutboundMessageContactID = query.GetLastInsertedID();
}

int TOutboundMessageContacts::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMessageID );
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_OutboundMessageContactID );
	query.Execute( _T("UPDATE OutboundMessageContacts ")
	               _T("SET OutboundMessageID=?,ContactID=? ")
	               _T("WHERE OutboundMessageContactID=?") );
	return query.GetRowCount();
}

int TOutboundMessageContacts::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMessageContactID );
	query.Execute( _T("DELETE FROM OutboundMessageContacts ")
	               _T("WHERE OutboundMessageContactID=?") );
	return query.GetRowCount();
}

int TOutboundMessageContacts::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_OutboundMessageID );
	BINDCOL_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_OutboundMessageContactID );
	query.Execute( _T("SELECT OutboundMessageID,ContactID ")
	               _T("FROM OutboundMessageContacts ")
	               _T("WHERE OutboundMessageContactID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TOutboundMessageContacts::GetLongData( CODBCQuery& query )
{
}

void TOutboundMessageContacts::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_OutboundMessageContactID );
	BINDCOL_LONG( query, m_OutboundMessageID );
	BINDCOL_LONG( query, m_ContactID );
	query.Execute( _T("SELECT OutboundMessageContactID,OutboundMessageID,ContactID ")
	               _T("FROM OutboundMessageContacts ")
	               _T("ORDER BY OutboundMessageContactID") );
}

/* class TOutboundMessageQueue - 
		Outbound message queue.
	 */
void TOutboundMessageQueue::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMessageID );
	BINDPARAM_LONG( query, m_MessageDestinationID );
	BINDPARAM_BIT( query, m_SpoolFileGenerated );
	BINDPARAM_BIT( query, m_IsApproved );
	BINDPARAM_TIME( query, m_DateSpooled );
	BINDPARAM_TIME( query, m_LastAttemptedDelivery );
	query.Execute( _T("INSERT INTO OutboundMessageQueue ")
	               _T("(OutboundMessageID,MessageDestinationID,SpoolFileGenerated,IsApproved,DateSpooled,LastAttemptedDelivery) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?)") );
	m_OutBoundMessageQueueID = query.GetLastInsertedID();
}

int TOutboundMessageQueue::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMessageID );
	BINDPARAM_LONG( query, m_MessageDestinationID );
	BINDPARAM_BIT( query, m_SpoolFileGenerated );
	BINDPARAM_BIT( query, m_IsApproved );
	BINDPARAM_TIME( query, m_LastAttemptedDelivery );
	BINDPARAM_LONG( query, m_OutBoundMessageQueueID );
	query.Execute( _T("UPDATE OutboundMessageQueue ")
	               _T("SET OutboundMessageID=?,MessageDestinationID=?,SpoolFileGenerated=?,IsApproved=?,LastAttemptedDelivery=? ")
	               _T("WHERE OutBoundMessageQueueID=?") );
	return query.GetRowCount();
}

int TOutboundMessageQueue::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutBoundMessageQueueID );
	query.Execute( _T("DELETE FROM OutboundMessageQueue ")
	               _T("WHERE OutBoundMessageQueueID=?") );
	return query.GetRowCount();
}

int TOutboundMessageQueue::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_OutboundMessageID );
	BINDCOL_LONG( query, m_MessageDestinationID );
	BINDCOL_BIT( query, m_SpoolFileGenerated );
	BINDCOL_BIT( query, m_IsApproved );
	BINDCOL_TIME( query, m_DateSpooled );
	BINDCOL_TIME( query, m_LastAttemptedDelivery );
	BINDPARAM_LONG( query, m_OutBoundMessageQueueID );
	query.Execute( _T("SELECT OutboundMessageID,MessageDestinationID,SpoolFileGenerated,IsApproved,DateSpooled,LastAttemptedDelivery ")
	               _T("FROM OutboundMessageQueue ")
	               _T("WHERE OutBoundMessageQueueID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TOutboundMessageQueue::GetLongData( CODBCQuery& query )
{
}

void TOutboundMessageQueue::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_OutBoundMessageQueueID );
	BINDCOL_LONG( query, m_OutboundMessageID );
	BINDCOL_LONG( query, m_MessageDestinationID );
	BINDCOL_BIT( query, m_SpoolFileGenerated );
	BINDCOL_BIT( query, m_IsApproved );
	BINDCOL_TIME( query, m_DateSpooled );
	BINDCOL_TIME( query, m_LastAttemptedDelivery );
	query.Execute( _T("SELECT OutBoundMessageQueueID,OutboundMessageID,MessageDestinationID,SpoolFileGenerated,IsApproved,DateSpooled,LastAttemptedDelivery ")
	               _T("FROM OutboundMessageQueue ")
	               _T("ORDER BY OutBoundMessageQueueID") );
}

/* class TOutboundMessageStates - 
		Message states will include the following:
		Draft, Sent, Untouched, Retry, Failed
	 */
void TOutboundMessageStates::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_MessageStateName );
	query.Execute( _T("INSERT INTO OutboundMessageStates ")
	               _T("(MessageStateName) ")
	               _T("VALUES")
	               _T("(?)") );
	m_OutboundMsgStateID = query.GetLastInsertedID();
}

int TOutboundMessageStates::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_MessageStateName );
	BINDPARAM_LONG( query, m_OutboundMsgStateID );
	query.Execute( _T("UPDATE OutboundMessageStates ")
	               _T("SET MessageStateName=? ")
	               _T("WHERE OutboundMsgStateID=?") );
	return query.GetRowCount();
}

int TOutboundMessageStates::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMsgStateID );
	query.Execute( _T("DELETE FROM OutboundMessageStates ")
	               _T("WHERE OutboundMsgStateID=?") );
	return query.GetRowCount();
}

int TOutboundMessageStates::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_MessageStateName );
	BINDPARAM_LONG( query, m_OutboundMsgStateID );
	query.Execute( _T("SELECT MessageStateName ")
	               _T("FROM OutboundMessageStates ")
	               _T("WHERE OutboundMsgStateID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TOutboundMessageStates::GetLongData( CODBCQuery& query )
{
}

void TOutboundMessageStates::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_OutboundMsgStateID );
	BINDCOL_TCHAR( query, m_MessageStateName );
	query.Execute( _T("SELECT OutboundMsgStateID,MessageStateName ")
	               _T("FROM OutboundMessageStates ")
	               _T("ORDER BY OutboundMsgStateID") );
}

/* class TOutboundMessageTypes - 
		Describes different types of outbound messages.
	 */
void TOutboundMessageTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO OutboundMessageTypes ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_OutboundMessageTypeID = query.GetLastInsertedID();
}

int TOutboundMessageTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_OutboundMessageTypeID );
	query.Execute( _T("UPDATE OutboundMessageTypes ")
	               _T("SET Description=? ")
	               _T("WHERE OutboundMessageTypeID=?") );
	return query.GetRowCount();
}

int TOutboundMessageTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMessageTypeID );
	query.Execute( _T("DELETE FROM OutboundMessageTypes ")
	               _T("WHERE OutboundMessageTypeID=?") );
	return query.GetRowCount();
}

int TOutboundMessageTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_OutboundMessageTypeID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM OutboundMessageTypes ")
	               _T("WHERE OutboundMessageTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TOutboundMessageTypes::GetLongData( CODBCQuery& query )
{
}

void TOutboundMessageTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_OutboundMessageTypeID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT OutboundMessageTypeID,Description ")
	               _T("FROM OutboundMessageTypes ")
	               _T("ORDER BY OutboundMessageTypeID") );
}

/* class TOutboundMessages - 
		Messages created in the system.
	 */
void TOutboundMessages::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_TCHAR( query, m_EmailFrom );
	BINDPARAM_TCHAR( query, m_EmailPrimaryTo );
	BINDPARAM_TIME( query, m_EmailDateTime );
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_TCHAR( query, m_MediaType );
	BINDPARAM_TCHAR( query, m_MediaSubType );
	BINDPARAM_LONG( query, m_DeletedBy );
	BINDPARAM_TIME( query, m_DeletedTime );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_ReplyToMsgID );
	BINDPARAM_BIT( query, m_ReplyToIDIsInbound );
	BINDPARAM_LONG( query, m_OutboundMessageTypeID );
	BINDPARAM_LONG( query, m_OutboundMessageStateID );
	BINDPARAM_TEXT( query, m_Body );
	BINDPARAM_TEXT( query, m_EmailTo );
	BINDPARAM_TEXT( query, m_EmailCc );
	BINDPARAM_TEXT( query, m_EmailBcc );
	BINDPARAM_TEXT( query, m_EmailReplyTo );
	BINDPARAM_LONG( query, m_TicketBoxHeaderID );
	BINDPARAM_LONG( query, m_TicketBoxFooterID );
	BINDPARAM_LONG( query, m_PriorityID );
	BINDPARAM_LONG( query, m_ArchiveID );
	BINDPARAM_LONG( query, m_SignatureID );
	BINDPARAM_LONG( query, m_OriginalTicketBoxID );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	BINDPARAM_TINYINT( query, m_DraftCloseTicket );
	BINDPARAM_TINYINT( query, m_DraftRouteToMe );
	BINDPARAM_TINYINT( query, m_FooterLocation );
	BINDPARAM_TINYINT( query, m_ReadReceipt );
	BINDPARAM_TINYINT( query, m_DeliveryConfirmation );
	BINDPARAM_LONG( query, m_MultiMail );
	query.Execute( _T("INSERT INTO OutboundMessages ")
	               _T("(TicketID,EmailFrom,EmailPrimaryTo,EmailDateTime,Subject,MediaType,MediaSubType,DeletedBy,DeletedTime,IsDeleted,AgentID,ReplyToMsgID,ReplyToIDIsInbound,OutboundMessageTypeID,OutboundMessageStateID,Body,EmailTo,EmailCc,EmailBcc,EmailReplyTo,TicketBoxHeaderID,TicketBoxFooterID,PriorityID,ArchiveID,SignatureID,OriginalTicketBoxID,TicketCategoryID,DraftCloseTicket,DraftRouteToMe,FooterLocation,ReadReceipt,DeliveryConfirmation,MultiMail) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_OutboundMessageID = query.GetLastInsertedID();
}

int TOutboundMessages::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_TCHAR( query, m_EmailFrom );
	BINDPARAM_TCHAR( query, m_EmailPrimaryTo );
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_TCHAR( query, m_MediaType );
	BINDPARAM_TCHAR( query, m_MediaSubType );
	BINDPARAM_LONG( query, m_DeletedBy );
	BINDPARAM_TIME( query, m_DeletedTime );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_ReplyToMsgID );
	BINDPARAM_BIT( query, m_ReplyToIDIsInbound );
	BINDPARAM_LONG( query, m_OutboundMessageTypeID );
	BINDPARAM_LONG( query, m_OutboundMessageStateID );
	BINDPARAM_TEXT( query, m_Body );
	BINDPARAM_TEXT( query, m_EmailTo );
	BINDPARAM_TEXT( query, m_EmailCc );
	BINDPARAM_TEXT( query, m_EmailBcc );
	BINDPARAM_TEXT( query, m_EmailReplyTo );
	BINDPARAM_LONG( query, m_TicketBoxHeaderID );
	BINDPARAM_LONG( query, m_TicketBoxFooterID );
	BINDPARAM_LONG( query, m_PriorityID );
	BINDPARAM_LONG( query, m_ArchiveID );
	BINDPARAM_LONG( query, m_SignatureID );
	BINDPARAM_LONG( query, m_OriginalTicketBoxID );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	BINDPARAM_TINYINT( query, m_DraftCloseTicket );
	BINDPARAM_TINYINT( query, m_DraftRouteToMe );
	BINDPARAM_TINYINT( query, m_FooterLocation );
	BINDPARAM_TINYINT( query, m_ReadReceipt );
	BINDPARAM_TINYINT( query, m_DeliveryConfirmation );
	BINDPARAM_LONG( query, m_MultiMail );
	BINDPARAM_LONG( query, m_OutboundMessageID );
	query.Execute( _T("UPDATE OutboundMessages ")
	               _T("SET TicketID=?,EmailFrom=?,EmailPrimaryTo=?,Subject=?,MediaType=?,MediaSubType=?,DeletedBy=?,DeletedTime=?,IsDeleted=?,AgentID=?,ReplyToMsgID=?,ReplyToIDIsInbound=?,OutboundMessageTypeID=?,OutboundMessageStateID=?,Body=?,EmailTo=?,EmailCc=?,EmailBcc=?,EmailReplyTo=?,TicketBoxHeaderID=?,TicketBoxFooterID=?,PriorityID=?,ArchiveID=?,SignatureID=?,OriginalTicketBoxID=?,TicketCategoryID=?,DraftCloseTicket=?,DraftRouteToMe=?,FooterLocation=?,ReadReceipt=?,DeliveryConfirmation=?,MultiMail=? ")
	               _T("WHERE OutboundMessageID=?") );
	return query.GetRowCount();
}

int TOutboundMessages::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OutboundMessageID );
	query.Execute( _T("DELETE FROM OutboundMessages ")
	               _T("WHERE OutboundMessageID=?") );
	return query.GetRowCount();
}

int TOutboundMessages::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_TCHAR( query, m_EmailFrom );
	BINDCOL_TCHAR( query, m_EmailPrimaryTo );
	BINDCOL_TIME( query, m_EmailDateTime );
	BINDCOL_TCHAR( query, m_Subject );
	BINDCOL_TCHAR( query, m_MediaType );
	BINDCOL_TCHAR( query, m_MediaSubType );
	BINDCOL_LONG( query, m_DeletedBy );
	BINDCOL_TIME( query, m_DeletedTime );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ReplyToMsgID );
	BINDCOL_BIT( query, m_ReplyToIDIsInbound );
	BINDCOL_LONG( query, m_OutboundMessageTypeID );
	BINDCOL_LONG( query, m_OutboundMessageStateID );
	BINDCOL_LONG( query, m_TicketBoxHeaderID );
	BINDCOL_LONG( query, m_TicketBoxFooterID );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_LONG( query, m_ArchiveID );
	BINDCOL_LONG( query, m_SignatureID );
	BINDCOL_LONG( query, m_OriginalTicketBoxID );
	BINDCOL_LONG( query, m_TicketCategoryID );
	BINDCOL_TINYINT( query, m_DraftCloseTicket );
	BINDCOL_TINYINT( query, m_DraftRouteToMe );
	BINDCOL_TINYINT( query, m_FooterLocation );
	BINDCOL_TINYINT( query, m_ReadReceipt );
	BINDCOL_TINYINT( query, m_DeliveryConfirmation );
	BINDCOL_LONG( query, m_MultiMail );
	BINDPARAM_LONG( query, m_OutboundMessageID );
	query.Execute( _T("SELECT TicketID,EmailFrom,EmailPrimaryTo,EmailDateTime,Subject,MediaType,MediaSubType,DeletedBy,DeletedTime,IsDeleted,AgentID,ReplyToMsgID,ReplyToIDIsInbound,OutboundMessageTypeID,OutboundMessageStateID,TicketBoxHeaderID,TicketBoxFooterID,PriorityID,ArchiveID,SignatureID,OriginalTicketBoxID,TicketCategoryID,DraftCloseTicket,DraftRouteToMe,FooterLocation,ReadReceipt,DeliveryConfirmation,MultiMail,Body,EmailTo,EmailCc,EmailBcc,EmailReplyTo ")
	               _T("FROM OutboundMessages ")
	               _T("WHERE OutboundMessageID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TOutboundMessages::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Body );
	GETDATA_TEXT( query, m_EmailTo );
	GETDATA_TEXT( query, m_EmailCc );
	GETDATA_TEXT( query, m_EmailBcc );
	GETDATA_TEXT( query, m_EmailReplyTo );
}

void TOutboundMessages::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_OutboundMessageID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_TCHAR( query, m_EmailFrom );
	BINDCOL_TCHAR( query, m_EmailPrimaryTo );
	BINDCOL_TIME( query, m_EmailDateTime );
	BINDCOL_TCHAR( query, m_Subject );
	BINDCOL_TCHAR( query, m_MediaType );
	BINDCOL_TCHAR( query, m_MediaSubType );
	BINDCOL_LONG( query, m_DeletedBy );
	BINDCOL_TIME( query, m_DeletedTime );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ReplyToMsgID );
	BINDCOL_BIT( query, m_ReplyToIDIsInbound );
	BINDCOL_LONG( query, m_OutboundMessageTypeID );
	BINDCOL_LONG( query, m_OutboundMessageStateID );
	BINDCOL_LONG( query, m_TicketBoxHeaderID );
	BINDCOL_LONG( query, m_TicketBoxFooterID );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_LONG( query, m_ArchiveID );
	BINDCOL_LONG( query, m_SignatureID );
	BINDCOL_LONG( query, m_OriginalTicketBoxID );
	BINDCOL_LONG( query, m_TicketCategoryID );
	BINDCOL_TINYINT( query, m_DraftCloseTicket );
	BINDCOL_TINYINT( query, m_DraftRouteToMe );
	BINDCOL_TINYINT( query, m_FooterLocation );
	BINDCOL_TINYINT( query, m_ReadReceipt );
	BINDCOL_TINYINT( query, m_DeliveryConfirmation );
	BINDCOL_LONG( query, m_MultiMail );
	query.Execute( _T("SELECT OutboundMessageID,TicketID,EmailFrom,EmailPrimaryTo,EmailDateTime,Subject,MediaType,MediaSubType,DeletedBy,DeletedTime,IsDeleted,AgentID,ReplyToMsgID,ReplyToIDIsInbound,OutboundMessageTypeID,OutboundMessageStateID,TicketBoxHeaderID,TicketBoxFooterID,PriorityID,ArchiveID,SignatureID,OriginalTicketBoxID,TicketCategoryID,DraftCloseTicket,DraftRouteToMe,FooterLocation,ReadReceipt,DeliveryConfirmation,MultiMail,Body,EmailTo,EmailCc,EmailBcc,EmailReplyTo ")
	               _T("FROM OutboundMessages ")
	               _T("ORDER BY OutboundMessageID") );
}

/* class TPersonalData - 
		Contains personal information such as email addresses, phone numbers
		and custom data that is related to a specific contact or agent. 
	 */
void TPersonalData::Insert( CODBCQuery& query )
{
	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );
		
	query.Initialize();
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_PersonalDataTypeID );
	BINDPARAM_TCHAR( query, m_DataValue );
	BINDPARAM_TCHAR( query, m_Note );
	BINDPARAM_LONG( query, m_StatusID );
	if( m_StatusDate.year == 0 && m_StatusDate.month == 0 && m_StatusDate.day == 0 )
	{
		BINDPARAM_TIME_NOLEN( query, now );	
	}
	else
	{
		BINDPARAM_TIME( query, m_StatusDate );	
	}
	query.Execute( _T("INSERT INTO PersonalData ")
	               _T("(ContactID,AgentID,PersonalDataTypeID,DataValue,Note,StatusID,StatusDate) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?)") );
	m_PersonalDataID = query.GetLastInsertedID();
}

int TPersonalData::Update( CODBCQuery& query )
{
	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );
		
	query.Initialize();
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_PersonalDataTypeID );
	BINDPARAM_TCHAR( query, m_DataValue );
	BINDPARAM_TCHAR( query, m_Note );
	BINDPARAM_LONG( query, m_StatusID );
	if( m_StatusDate.year == 0 && m_StatusDate.month == 0 && m_StatusDate.day == 0 )
	{
		BINDPARAM_TIME_NOLEN( query, now );	
	}
	else
	{
		BINDPARAM_TIME( query, m_StatusDate );	
	}
	BINDPARAM_LONG( query, m_PersonalDataID );
	query.Execute( _T("UPDATE PersonalData ")
	               _T("SET ContactID=?,AgentID=?,PersonalDataTypeID=?,DataValue=?,Note=?,StatusID=?,StatusDate=? ")
	               _T("WHERE PersonalDataID=?") );
	return query.GetRowCount();
}

int TPersonalData::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_PersonalDataID );
	query.Execute( _T("DELETE FROM PersonalData ")
	               _T("WHERE PersonalDataID=?") );
	return query.GetRowCount();
}

int TPersonalData::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_ContactID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_PersonalDataTypeID );
	BINDCOL_TCHAR( query, m_DataValue );
	BINDCOL_TCHAR( query, m_Note );
	BINDCOL_LONG( query, m_StatusID );
	BINDCOL_TIME( query, m_StatusDate );
	BINDPARAM_LONG( query, m_PersonalDataID );
	query.Execute( _T("SELECT ContactID,AgentID,PersonalDataTypeID,DataValue,Note,StatusID,StatusDate ")
	               _T("FROM PersonalData ")
	               _T("WHERE PersonalDataID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TPersonalData::GetLongData( CODBCQuery& query )
{
}

void TPersonalData::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_PersonalDataID );
	BINDCOL_LONG( query, m_ContactID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_PersonalDataTypeID );
	BINDCOL_TCHAR( query, m_DataValue );
	BINDCOL_TCHAR( query, m_Note );
	BINDCOL_LONG( query, m_StatusID );
	BINDCOL_TIME( query, m_StatusDate );
	query.Execute( _T("SELECT PersonalDataID,ContactID,AgentID,PersonalDataTypeID,DataValue,Note,StatusID,StatusDate ")
	               _T("FROM PersonalData ")
	               _T("ORDER BY PersonalDataID") );
}

/* class TPersonalDataTypes - 
		Types of personal data such as email, business phone, ICQ# etc.
	 */
void TPersonalDataTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_TypeName );
	BINDPARAM_BIT( query, m_BuiltIn );
	query.Execute( _T("INSERT INTO PersonalDataTypes ")
	               _T("(TypeName,BuiltIn) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_PersonalDataTypeID = query.GetLastInsertedID();
}

int TPersonalDataTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_TypeName );
	BINDPARAM_LONG( query, m_PersonalDataTypeID );
	query.Execute( _T("UPDATE PersonalDataTypes ")
	               _T("SET TypeName=? ")
	               _T("WHERE PersonalDataTypeID=?") );
	return query.GetRowCount();
}

int TPersonalDataTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_PersonalDataTypeID );
	query.Execute( _T("DELETE FROM PersonalDataTypes ")
	               _T("WHERE PersonalDataTypeID=?") );
	return query.GetRowCount();
}

int TPersonalDataTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_TypeName );
	BINDCOL_BIT( query, m_BuiltIn );
	BINDPARAM_LONG( query, m_PersonalDataTypeID );
	query.Execute( _T("SELECT TypeName,BuiltIn ")
	               _T("FROM PersonalDataTypes ")
	               _T("WHERE PersonalDataTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TPersonalDataTypes::GetLongData( CODBCQuery& query )
{
}

void TPersonalDataTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_PersonalDataTypeID );
	BINDCOL_TCHAR( query, m_TypeName );
	BINDCOL_BIT( query, m_BuiltIn );
	query.Execute( _T("SELECT PersonalDataTypeID,TypeName,BuiltIn ")
	               _T("FROM PersonalDataTypes ")
	               _T("ORDER BY PersonalDataTypeID") );
}

/* class TBypass - 
		Contains items that will be bypassed by various functions. 
	 */
void TBypass::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_BypassTypeID );
	BINDPARAM_TCHAR( query, m_BypassValue );
	BINDPARAM_TIME( query, m_DateCreated );
	query.Execute( _T("INSERT INTO Bypass ")
	               _T("(BypassTypeID,BypassValue,DateCreated) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_BypassID = query.GetLastInsertedID();
}

int TBypass::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_BypassTypeID );
	BINDPARAM_TCHAR( query, m_BypassValue );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_BypassID );
	query.Execute( _T("UPDATE Bypass ")
	               _T("SET BypassTypeID=?,BypassValue=?,DateCreated=? ")
	               _T("WHERE BypassID=?") );
	return query.GetRowCount();
}

int TBypass::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_BypassID );
	query.Execute( _T("DELETE FROM Bypass ")
	               _T("WHERE BypassID=?") );
	return query.GetRowCount();
}

int TBypass::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_BypassTypeID );
	BINDCOL_TCHAR( query, m_BypassValue );
	BINDCOL_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_BypassID );
	query.Execute( _T("SELECT BypassTypeID,BypassValue,DateCreated ")
	               _T("FROM Bypass ")
	               _T("WHERE BypassID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TBypass::GetLongData( CODBCQuery& query )
{
}

void TBypass::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_BypassID );
	BINDCOL_LONG( query, m_BypassTypeID );
	BINDCOL_TCHAR( query, m_BypassValue );
	BINDCOL_TIME( query, m_DateCreated );
	query.Execute( _T("SELECT BypassID,BypassTypeID,BypassValue,DateCreated ")
	               _T("FROM Bypass ")
	               _T("ORDER BY BypassValue") );
}

/* class TEmail - 
		Contains items that will be bypassed by various functions. 
	 */
void TEmail::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_EmailTypeID );
	BINDPARAM_TCHAR( query, m_EmailValue );
	BINDPARAM_TIME( query, m_DateCreated );
	query.Execute( _T("INSERT INTO Email ")
	               _T("(EmailTypeID,EmailValue,DateCreated) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_EmailID = query.GetLastInsertedID();
}

int TEmail::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_EmailTypeID );
	BINDPARAM_TCHAR( query, m_EmailValue );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_EmailID );
	query.Execute( _T("UPDATE Email ")
	               _T("SET EmailTypeID=?,EmailValue=?,DateCreated=? ")
	               _T("WHERE EmailID=?") );
	return query.GetRowCount();
}

int TEmail::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_EmailID );
	query.Execute( _T("DELETE FROM Email ")
	               _T("WHERE EmailID=?") );
	return query.GetRowCount();
}

int TEmail::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_EmailTypeID );
	BINDCOL_TCHAR( query, m_EmailValue );
	BINDCOL_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_EmailID );
	query.Execute( _T("SELECT EmailTypeID,EmailValue,DateCreated ")
	               _T("FROM Email ")
	               _T("WHERE EmailID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TEmail::GetLongData( CODBCQuery& query )
{
}

void TEmail::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_EmailID );
	BINDCOL_LONG( query, m_EmailTypeID );
	BINDCOL_TCHAR( query, m_EmailValue );
	BINDCOL_TIME( query, m_DateCreated );
	query.Execute( _T("SELECT EmailID,EmailTypeID,EmailValue,DateCreated ")
	               _T("FROM Email ")
	               _T("ORDER BY EmailValue") );
}

/* class TAutoActionTypes - 
		Types of auto Ticket actions such as delete, move, etc.
	 */
void TAutoActionTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_TypeName );
	BINDPARAM_BIT( query, m_BuiltIn );
	query.Execute( _T("INSERT INTO AutoActionTypes ")
	               _T("(TypeName,BuiltIn) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_AutoActionTypeID = query.GetLastInsertedID();
}

int TAutoActionTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_TypeName );
	BINDPARAM_LONG( query, m_AutoActionTypeID );
	query.Execute( _T("UPDATE AutoActionTypes ")
	               _T("SET TypeName=? ")
	               _T("WHERE AutoActionTypeID=?") );
	return query.GetRowCount();
}

int TAutoActionTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoActionTypeID );
	query.Execute( _T("DELETE FROM AutoActionTypes ")
	               _T("WHERE AutoActionTypeID=?") );
	return query.GetRowCount();
}

int TAutoActionTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_TypeName );
	BINDCOL_BIT( query, m_BuiltIn );
	BINDPARAM_LONG( query, m_AutoActionTypeID );
	query.Execute( _T("SELECT TypeName,BuiltIn ")
	               _T("FROM AutoActionTypes ")
	               _T("WHERE AutoActionTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAutoActionTypes::GetLongData( CODBCQuery& query )
{
}

void TAutoActionTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AutoActionTypeID );
	BINDCOL_TCHAR( query, m_TypeName );
	BINDCOL_BIT( query, m_BuiltIn );
	query.Execute( _T("SELECT AutoActionTypeID,TypeName,BuiltIn ")
	               _T("FROM AutoActionTypes ")
	               _T("ORDER BY AutoActionTypeID") );
}

/* class TAutoActionEvents - 
		Types of auto Ticket Events such as delete, move, etc.
	 */
void TAutoActionEvents::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EventName );
	BINDPARAM_BIT( query, m_BuiltIn );
	query.Execute( _T("INSERT INTO AutoActionEvents ")
	               _T("(EventName,BuiltIn) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_AutoActionEventID = query.GetLastInsertedID();
}

int TAutoActionEvents::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EventName );
	BINDPARAM_LONG( query, m_AutoActionEventID );
	query.Execute( _T("UPDATE AutoActionEvents ")
	               _T("SET EventName=? ")
	               _T("WHERE AutoActionEventID=?") );
	return query.GetRowCount();
}

int TAutoActionEvents::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoActionEventID );
	query.Execute( _T("DELETE FROM AutoActionEvents ")
	               _T("WHERE AutoActionEventID=?") );
	return query.GetRowCount();
}

int TAutoActionEvents::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_EventName );
	BINDCOL_BIT( query, m_BuiltIn );
	BINDPARAM_LONG( query, m_AutoActionEventID );
	query.Execute( _T("SELECT EventName,BuiltIn ")
	               _T("FROM AutoActionEvents ")
	               _T("WHERE AutoActionEventID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAutoActionEvents::GetLongData( CODBCQuery& query )
{
}

void TAutoActionEvents::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AutoActionEventID );
	BINDCOL_TCHAR( query, m_EventName );
	BINDCOL_BIT( query, m_BuiltIn );
	query.Execute( _T("SELECT AutoActionEventID,EventName,BuiltIn ")
	               _T("FROM AutoActionEvents ")
	               _T("ORDER BY AutoActionEventID") );
}

/* class TAutoActions */
void TAutoActions::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_AutoActionVal );
	BINDPARAM_LONG( query, m_AutoActionFreq );
	BINDPARAM_LONG( query, m_AutoActionEventID );
	BINDPARAM_LONG( query, m_AutoActionTypeID );
	BINDPARAM_LONG( query, m_AutoActionTargetID );	
	query.Execute( _T("INSERT INTO AutoActions ")
	               _T("(TicketBoxID,AutoActionVal,AutoActionFreq,AutoActionEventID,AutoActionTypeID,AutoActionTargetID) ")
				   _T("VALUES")
				   _T("(?,?,?,?,?,?)") );
	m_AutoActionID = query.GetLastInsertedID();
}

int TAutoActions::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_AutoActionVal );
	BINDPARAM_LONG( query, m_AutoActionFreq );
	BINDPARAM_LONG( query, m_AutoActionEventID );
	BINDPARAM_LONG( query, m_AutoActionTypeID );
	BINDPARAM_LONG( query, m_AutoActionTargetID );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_AutoActionID );
	query.Execute( _T("UPDATE AutoActions ")
	               _T("SET TicketBoxID=?,AutoActionVal=?,AutoActionFreq=?,AutoActionEventID=?,AutoActionTypeID=?,AutoActionTargetID=?,DateCreated=? ")
	               _T("WHERE AutoActionID=?") );
	return query.GetRowCount();
}

int TAutoActions::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoActionID );
	query.Execute( _T("DELETE FROM AutoActions ")
	               _T("WHERE AutoActionID=?") );
	return query.GetRowCount();
}

int TAutoActions::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_AutoActionVal );
	BINDCOL_LONG( query, m_AutoActionFreq );
	BINDCOL_LONG( query, m_AutoActionEventID );
	BINDCOL_LONG( query, m_AutoActionTypeID );
	BINDCOL_LONG( query, m_AutoActionTargetID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_AutoActionID );
	query.Execute( _T("SELECT TicketBoxID,AutoActionVal,AutoActionFreq,AutoActionEventID,AutoActionTypeID,AutoActionTargetID,DateCreated ")
	               _T("FROM AutoActions ")
	               _T("WHERE AutoActionID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAutoActions::GetLongData( CODBCQuery& query )
{
}

void TAutoActions::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AutoActionID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_AutoActionVal );
	BINDCOL_LONG( query, m_AutoActionFreq );
	BINDCOL_LONG( query, m_AutoActionEventID );
	BINDCOL_LONG( query, m_AutoActionTypeID );
	BINDCOL_LONG( query, m_AutoActionTargetID );
	BINDCOL_TIME( query, m_DateCreated );	
	query.Execute( _T("SELECT AutoActionID,TicketBoxID,AutoActionVal,AutoActionFreq,AutoActionEventID,AutoActionTypeID,AutoActionTargetID,DateCreated ")
	               _T("FROM AutoActions ")
	               _T("ORDER BY AutoActionID") );
}

/* class TOfficeHours */
void TOfficeHours::Insert( CODBCQuery& query )
{
	m_TimeStartLen = sizeof(m_TimeStart);
	m_TimeEndLen = sizeof(m_TimeEnd);	
	
	query.Initialize();
	BINDPARAM_LONG( query, m_TypeID );
	BINDPARAM_LONG( query, m_ActualID );
	BINDPARAM_LONG( query, m_StartHr );
	BINDPARAM_LONG( query, m_StartMin );
	BINDPARAM_LONG( query, m_StartAmPm );
	BINDPARAM_LONG( query, m_EndHr );
	BINDPARAM_LONG( query, m_EndMin );
	BINDPARAM_LONG( query, m_EndAmPm );
	BINDPARAM_LONG( query, m_DayID );
	if ( m_TimeStartLen <= 0 || m_TimeEndLen <= 0 || m_TimeStart.year == 0 || m_TimeEnd.year == 0 )
	{
		query.Execute( _T("INSERT INTO OfficeHours ")
	               _T("(TypeID,ActualID,StartHr,StartMin,StartAmPm,EndHr,EndMin,EndAmPm,DayID) ")
				   _T("VALUES")
				   _T("(?,?,?,?,?,?,?,?,?)") );
	}
	else
	{
		BINDPARAM_TCHAR( query, m_Description );
		BINDPARAM_TIME( query, m_TimeStart );
		BINDPARAM_TIME( query, m_TimeEnd );
		query.Execute( _T("INSERT INTO OfficeHours ")
				_T("(TypeID,ActualID,StartHr,StartMin,StartAmPm,EndHr,EndMin,EndAmPm,DayID,Description,TimeStart,TimeEnd) ")
				_T("VALUES")
				_T("(?,?,?,?,?,?,?,?,?,?,?,?)") );
	}

	m_OfficeHourID = query.GetLastInsertedID();
}

int TOfficeHours::Update( CODBCQuery& query )
{
	m_TimeStartLen = sizeof(m_TimeStart);
	m_TimeEndLen = sizeof(m_TimeEnd);	
	
	query.Initialize();
	BINDPARAM_LONG( query, m_TypeID );
	BINDPARAM_LONG( query, m_ActualID );
	BINDPARAM_LONG( query, m_StartHr );
	BINDPARAM_LONG( query, m_StartMin );
	BINDPARAM_LONG( query, m_StartAmPm );
	BINDPARAM_LONG( query, m_EndHr );
	BINDPARAM_LONG( query, m_EndMin );
	BINDPARAM_LONG( query, m_EndAmPm );
	BINDPARAM_LONG( query, m_DayID );
	if ( m_TimeStartLen <= 0 || m_TimeEndLen <= 0 || m_TimeStart.year == 0 || m_TimeEnd.year == 0 )
	{
		BINDPARAM_LONG( query, m_OfficeHourID );
		query.Execute( _T("UPDATE OfficeHours ")
					_T("SET TypeID=?,ActualID=?,StartHr=?,StartMin=?,StartAmPm=?,EndHr=?,EndMin=?,EndAmPm=?,DayID=? ")
					_T("WHERE OfficeHourID=?") );
	}
	else
	{
		BINDPARAM_TCHAR( query, m_Description );
		BINDPARAM_TIME( query, m_TimeStart );
		BINDPARAM_TIME( query, m_TimeEnd );
		BINDPARAM_LONG( query, m_OfficeHourID );
		query.Execute( _T("UPDATE OfficeHours ")
					_T("SET TypeID=?,ActualID=?,StartHr=?,StartMin=?,StartAmPm=?,EndHr=?,EndMin=?,EndAmPm=?,DayID=?,Description=?,TimeStart=?,TimeEnd=? ")
					_T("WHERE OfficeHourID=?") );
	}
	
	return query.GetRowCount();
}

int TOfficeHours::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OfficeHourID );
	query.Execute( _T("DELETE FROM OfficeHours ")
	               _T("WHERE OfficeHourID=?") );
	return query.GetRowCount();
}

int TOfficeHours::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TypeID );
	BINDCOL_LONG( query, m_ActualID );
	BINDCOL_LONG( query, m_StartHr );
	BINDCOL_LONG( query, m_StartMin );
	BINDCOL_LONG( query, m_StartAmPm );
	BINDCOL_LONG( query, m_EndHr );
	BINDCOL_LONG( query, m_EndMin );
	BINDCOL_LONG( query, m_EndAmPm );
	BINDCOL_LONG( query, m_DayID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TIME( query, m_TimeStart );
	BINDCOL_TIME( query, m_TimeEnd );
	BINDPARAM_LONG( query, m_OfficeHourID );
	query.Execute( _T("SELECT TypeID,ActualID,StartHr,StartMin,StartAmPm,EndHr,EndMin,EndAmPm,DayID,Description,TimeStart,TimeEnd ")
	               _T("FROM OfficeHours ")
	               _T("WHERE OfficeHourID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TOfficeHours::GetLongData( CODBCQuery& query )
{
}

void TOfficeHours::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_OfficeHourID );
	BINDCOL_LONG( query, m_TypeID );
	BINDCOL_LONG( query, m_ActualID );
	BINDCOL_LONG( query, m_StartHr );
	BINDCOL_LONG( query, m_StartMin );
	BINDCOL_LONG( query, m_StartAmPm );
	BINDCOL_LONG( query, m_EndHr );
	BINDCOL_LONG( query, m_EndMin );
	BINDCOL_LONG( query, m_EndAmPm );
	BINDCOL_LONG( query, m_DayID );	
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TIME( query, m_TimeStart );
	BINDCOL_TIME( query, m_TimeEnd );
	query.Execute( _T("SELECT OfficeHourID,TypeID,ActualID,StartHr,StartMin,StartAmPm,EndHr,EndMin,EndAmPm,DayID,Description,TimeStart,TimeEnd ")
	               _T("FROM OfficeHours ")
	               _T("ORDER BY TimeStart,TimeEnd") );
}

/* class TAgentContacts */
void TAgentContacts::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_ContactID );
	query.Execute( _T("INSERT INTO AgentContacts ")
	               _T("(AgentID,ContactID) ")
				   _T("VALUES")
				   _T("(?,?)") );
	m_AgentContactID = query.GetLastInsertedID();
}

int TAgentContacts::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_AgentContactID );
	query.Execute( _T("UPDATE AgentContacts ")
	               _T("SET AgentID=?,ContactID=? ")
	               _T("WHERE AgentContactID=?") );
	return query.GetRowCount();
}

int TAgentContacts::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentContactID );
	query.Execute( _T("DELETE FROM AgentContacts ")
	               _T("WHERE AgentContactID=?") );
	return query.GetRowCount();
}

int TAgentContacts::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_AgentContactID );
	query.Execute( _T("SELECT AgentID,ContactID ")
	               _T("FROM AgentContacts ")
	               _T("WHERE AgentContactID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAgentContacts::GetLongData( CODBCQuery& query )
{
}

void TAgentContacts::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AgentContactID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ContactID );
	query.Execute( _T("SELECT AgentContactID,AgentID,ContactID ")
	               _T("FROM AgentContacts ")
	               _T("ORDER BY AgentContactID") );
}

/* class TTicketCategories - 
		Ticket categories.
	 */
void TTicketCategories::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_ObjectID );
	BINDPARAM_BIT( query, m_BuiltIn );
	query.Execute( _T("INSERT INTO TicketCategories ")
	               _T("(Description,ObjectID,BuiltIn) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_TicketCategoryID = query.GetLastInsertedID();
}

int TTicketCategories::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_ObjectID );
	BINDPARAM_BIT( query, m_BuiltIn );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	query.Execute( _T("UPDATE TicketCategories ")
	               _T("SET Description=?,ObjectID=?,BuiltIn=? ")
	               _T("WHERE TicketCategoryID=?") );
	return query.GetRowCount();
}

int TTicketCategories::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketCategoryID );
	query.Execute( _T("DELETE FROM TicketCategories ")
	               _T("WHERE TicketCategoryID=?") );
	return query.GetRowCount();
}

int TTicketCategories::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_BIT( query, m_BuiltIn );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	query.Execute( _T("SELECT Description,ObjectID,BuiltIn ")
	               _T("FROM TicketCategories ")
	               _T("WHERE TicketCategoryID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketCategories::GetLongData( CODBCQuery& query )
{
}

void TTicketCategories::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketCategoryID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_BIT( query, m_BuiltIn );
	query.Execute( _T("SELECT TicketCategoryID,Description,ObjectID,BuiltIn ")
	               _T("FROM TicketCategories ")
	               _T("ORDER BY Description") );
}

/* class TTicketBoxOwners - 
		TicketBox Owners.
	 */
void TTicketBoxOwners::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO TicketBoxOwners ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_TicketBoxOwnerID = query.GetLastInsertedID();
}

int TTicketBoxOwners::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_TicketBoxOwnerID );
	query.Execute( _T("UPDATE TicketBoxOwners ")
	               _T("SET Description=? ")
	               _T("WHERE TicketBoxOwnerID=?") );
	return query.GetRowCount();
}

int TTicketBoxOwners::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxOwnerID );
	query.Execute( _T("DELETE FROM TicketBoxOwners ")
	               _T("WHERE TicketBoxOwnerID=?") );
	return query.GetRowCount();
}

int TTicketBoxOwners::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_TicketBoxOwnerID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM TicketBoxOwners ")
	               _T("WHERE TicketBoxOwnerID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketBoxOwners::GetLongData( CODBCQuery& query )
{
}

void TTicketBoxOwners::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxOwnerID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT TicketBoxOwnerID,Description ")
	               _T("FROM TicketBoxOwners ")
	               _T("ORDER BY Description") );
}

/* class TTicketBoxTicketBoxOwners - 
		TicketBox Owners.
	 */
void TTicketBoxTicketBoxOwners::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_TicketBoxOwnerID);
	query.Execute( _T("INSERT INTO TicketBoxTicketBoxOwners ")
	               _T("(TicketBoxID,TicketBoxOwnerID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_TbTboID = query.GetLastInsertedID();
}

int TTicketBoxTicketBoxOwners::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_TicketBoxOwnerID );
	BINDPARAM_LONG( query, m_TbTboID );
	query.Execute( _T("UPDATE TicketBoxTicketBoxOwners ")
	               _T("SET TicketBoxID=?,TicketBoxOwnerID=? ")
	               _T("WHERE TicketBoxTicketBoxOwnerID=?") );
	return query.GetRowCount();
}

int TTicketBoxTicketBoxOwners::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TbTboID );
	query.Execute( _T("DELETE FROM TicketBoxTicketBoxOwners ")
	               _T("WHERE m_TbTboID=?") );
	return query.GetRowCount();
}

int TTicketBoxTicketBoxOwners::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_TicketBoxOwnerID );
	BINDPARAM_LONG( query, m_TbTboID );
	query.Execute( _T("SELECT TicketBoxID, TicketBoxOwnerID ")
	               _T("FROM TicketBoxTicketBoxOwners ")
	               _T("WHERE m_TbTboID=?") );
	nResult = query.Fetch();
	return nResult;
}

void TTicketBoxTicketBoxOwners::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_TicketBoxOwnerID );
	BINDCOL_LONG( query, m_TbTboID );
	query.Execute( _T("SELECT TicketBoxID,TicketBoxOwnerID,TbTboID ")
	               _T("FROM TicketBoxTicketBoxOwners ")
	               _T("ORDER BY TicketBoxOwnerID") );
}

/* class TTicketBoxRouting - 
		TicketBox Routing.
	 */
void TTicketBoxRouting::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_RoutingRuleID);
	BINDPARAM_LONG( query, m_Percentage);
	query.Execute( _T("INSERT INTO TicketBoxRouting ")
	               _T("(TicketBoxID,RoutingRuleID,Percentage) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_TicketBoxRoutingID = query.GetLastInsertedID();
}

int TTicketBoxRouting::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_RoutingRuleID);
	BINDPARAM_LONG( query, m_Percentage);
	BINDPARAM_LONG( query, m_TicketBoxRoutingID );
	query.Execute( _T("UPDATE TicketBoxRouting ")
	               _T("SET TicketBoxID=?,RoutingRuleID=?,Percentage=? ")
	               _T("WHERE TicketBoxRoutingID=?") );
	return query.GetRowCount();
}

int TTicketBoxRouting::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxRoutingID );
	query.Execute( _T("DELETE FROM TicketBoxRouting ")
	               _T("WHERE TicketBoxRoutingID=?") );
	return query.GetRowCount();
}

int TTicketBoxRouting::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_RoutingRuleID);
	BINDCOL_LONG( query, m_Percentage);
	BINDPARAM_LONG( query, m_TicketBoxRoutingID );
	query.Execute( _T("SELECT TicketBoxID, RoutingRuleID,Percentage ")
	               _T("FROM TicketBoxRouting ")
	               _T("WHERE TicketBoxRoutingID=?") );
	nResult = query.Fetch();
	return nResult;
}

void TTicketBoxRouting::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_RoutingRuleID);
	BINDCOL_LONG( query, m_Percentage);
	BINDCOL_LONG( query, m_TicketBoxRoutingID );
	query.Execute( _T("SELECT TicketBoxID,RoutingRuleID,Percentage,TicketBoxRoutingID ")
	               _T("FROM TicketBoxRouting ")
	               _T("ORDER BY RoutingRuleID") );
}

/* class TReportTypes - 
		Report Types.
	 */
void TReportTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_AllowCustom );
	query.Execute( _T("INSERT INTO ReportTypes ")
	               _T("(Description,IsEnabled,AllowCustom) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_ReportTypeID = query.GetLastInsertedID();
}

int TReportTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_AllowCustom );
	BINDPARAM_LONG( query, m_ReportTypeID );
	query.Execute( _T("UPDATE ReportTypes ")
	               _T("SET Description=?,IsEnabled=?,AllowCustom=? ")
	               _T("WHERE ReportTypeID=?") );
	return query.GetRowCount();
}

int TReportTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ReportTypeID );
	query.Execute( _T("DELETE FROM ReportTypes ")
	               _T("WHERE ReportTypeID=?") );
	return query.GetRowCount();
}

int TReportTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_AllowCustom );
	BINDPARAM_LONG( query, m_ReportTypeID );
	query.Execute( _T("SELECT Description,IsEnabled,AllowCustom ")
	               _T("FROM ReportTypes ")
	               _T("WHERE ReportTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TReportTypes::GetLongData( CODBCQuery& query )
{
}

void TReportTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_AllowCustom );
	BINDCOL_LONG( query, m_ReportTypeID );
	query.Execute( _T("SELECT Description,IsEnabled,AllowCustom,ReportTypeID ")
	               _T("FROM ReportTypes ")
	               _T("ORDER BY ReportTypeID") );
}

/* class TReportObjects - 
		Report Objects.
	 */
void TReportObjects::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_AllowCustom );
	query.Execute( _T("INSERT INTO ReportObjects ")
	               _T("(Description,IsEnabled,AllowCustom) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_ReportObjectID = query.GetLastInsertedID();
}

int TReportObjects::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_AllowCustom );
	BINDPARAM_LONG( query, m_ReportObjectID );
	query.Execute( _T("UPDATE ReportObjects ")
	               _T("SET Description=?,IsEnabled=?,AllowCustom=? ")
	               _T("WHERE ReportObjectID=?") );
	return query.GetRowCount();
}

int TReportObjects::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ReportObjectID );
	query.Execute( _T("DELETE FROM ReportObjects ")
	               _T("WHERE ReportObjectID=?") );
	return query.GetRowCount();
}

int TReportObjects::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_AllowCustom );
	BINDPARAM_LONG( query, m_ReportObjectID );
	query.Execute( _T("SELECT Description,IsEnabled,AllowCustom ")
	               _T("FROM ReportObjects ")
	               _T("WHERE ReportObjectID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TReportObjects::GetLongData( CODBCQuery& query )
{
}

void TReportObjects::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_AllowCustom );
	BINDCOL_LONG( query, m_ReportObjectID );
	query.Execute( _T("SELECT Description,IsEnabled,AllowCustom,ReportObjectID ")
	               _T("FROM ReportObjects ")
	               _T("ORDER BY ReportObjectID") );
}

/* class TReportResults - 
		Report Results.
	 */
void TReportResults::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_ResultFile );
	BINDPARAM_LONG( query, m_ResultCode );
	BINDPARAM_TIME( query, m_DateRan );
	BINDPARAM_TIME( query, m_DateFrom );
	BINDPARAM_TIME( query, m_DateTo );
	BINDPARAM_LONG( query, m_ScheduledReportID );
	query.Execute( _T("INSERT INTO ReportResults ")
	               _T("(ResultFile,ResultCode,DateRan,DateFrom,DateTo,ScheduledReportID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?)") );
	m_ReportResultID = query.GetLastInsertedID();
}

int TReportResults::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_ResultFile );
	BINDPARAM_LONG( query, m_ResultCode );
	BINDPARAM_TIME( query, m_DateRan );
	BINDPARAM_TIME( query, m_DateFrom );
	BINDPARAM_TIME( query, m_DateTo );
	BINDPARAM_LONG( query, m_ScheduledReportID );
	BINDPARAM_LONG( query, m_ReportResultID );
	query.Execute( _T("UPDATE ReportResults ")
	               _T("SET ResultFile=?,ResultCode=?,DateRan=?,DateFrom=?,DateTo=?,ScheduledReportID=? ")
	               _T("WHERE ReportResultID=?") );
	return query.GetRowCount();
}

int TReportResults::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ReportResultID );
	query.Execute( _T("DELETE FROM ReportResults ")
	               _T("WHERE ReportResultID=?") );
	return query.GetRowCount();
}

void TReportResults::Query( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ReportResultID );
	BINDCOL_TCHAR( query, m_ResultFile );
	BINDCOL_LONG( query, m_ResultCode );
	BINDCOL_TIME( query, m_DateRan );
	BINDCOL_TIME( query, m_DateFrom );
	BINDCOL_TIME( query, m_DateTo );
	BINDPARAM_LONG( query, m_ScheduledReportID );
	query.Execute( _T("SELECT ReportResultID,ResultFile,ResultCode,DateRan,DateFrom,DateTo ")
	               _T("FROM ReportResults ")
	               _T("WHERE ScheduledReportID=?") );
}

int TReportResults::Query2( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_ScheduledReportID );
	BINDCOL_TCHAR( query, m_ResultFile );
	BINDCOL_LONG( query, m_ResultCode );
	BINDCOL_TIME( query, m_DateRan );
	BINDCOL_TIME( query, m_DateFrom );
	BINDCOL_TIME( query, m_DateTo );
	BINDPARAM_LONG( query, m_ReportResultID );
	query.Execute( _T("SELECT ScheduledReportID,ResultFile,ResultCode,DateRan,DateFrom,DateTo ")
	               _T("FROM ReportResults ")
	               _T("WHERE ReportResultID=?") );
	nResult = query.Fetch();
	return nResult;
}

int TReportResults::QueryOne( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_ScheduledReportID );
	BINDCOL_TCHAR( query, m_ResultFile );
	BINDCOL_LONG( query, m_ResultCode );
	BINDCOL_TIME( query, m_DateRan );
	BINDCOL_TIME( query, m_DateFrom );
	BINDCOL_TIME( query, m_DateTo );
	BINDPARAM_LONG( query, m_ReportResultID );
	query.Execute( _T("SELECT ScheduledReportID,ResultFile,ResultCode,DateRan,DateFrom,DateTo ")
	               _T("FROM ReportResults ")
	               _T("WHERE ReportResultID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}void TReportResults::GetLongData( CODBCQuery& query )
{
}

/* class TReportResultRows - 
		Report Result Rows.
	 */
void TReportResultRows::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ReportResultID );
	BINDPARAM_TCHAR( query, m_Col1 );
	BINDPARAM_TCHAR( query, m_Col2 );
	BINDPARAM_LONG( query, m_Col3 );
	BINDPARAM_LONG( query, m_Col4 );
	BINDPARAM_LONG( query, m_Col5 );
	BINDPARAM_LONG( query, m_Col6 );
	BINDPARAM_LONG( query, m_Col7 );
	BINDPARAM_LONG( query, m_Col8 );
	BINDPARAM_LONG( query, m_Col9 );
	BINDPARAM_LONG( query, m_Col10 );
	BINDPARAM_LONG( query, m_Col11 );
	BINDPARAM_LONG( query, m_Col12 );
	BINDPARAM_LONG( query, m_Col13 );
	query.Execute( _T("INSERT INTO ReportResultRows ")
	               _T("(ReportResultID,Col1,Col2,Col3,Col4,Col5,Col6,Col7,Col8,Col9,Col10,Col11,Col12,Col13) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_ReportResultRowsID = query.GetLastInsertedID();
}

int TReportResultRows::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ReportResultID );
	BINDPARAM_TCHAR( query, m_Col1 );
	BINDPARAM_TCHAR( query, m_Col2 );
	BINDPARAM_LONG( query, m_Col3 );
	BINDPARAM_LONG( query, m_Col4 );
	BINDPARAM_LONG( query, m_Col5 );
	BINDPARAM_LONG( query, m_Col6 );
	BINDPARAM_LONG( query, m_Col7 );
	BINDPARAM_LONG( query, m_Col8 );
	BINDPARAM_LONG( query, m_Col9 );
	BINDPARAM_LONG( query, m_Col10 );
	BINDPARAM_LONG( query, m_Col11 );
	BINDPARAM_LONG( query, m_Col12 );
	BINDPARAM_LONG( query, m_Col13 );
	BINDPARAM_LONG( query, m_ReportResultRowsID );
	query.Execute( _T("UPDATE ReportResultRows ")
	               _T("SET ReportResultID=?,m_Col1=?,m_Col2=?,m_Col3=?,m_Col4=?,m_Col5=?,m_Col6=?,m_Col7=?,m_Col8=?,m_Col9=?,m_Col10=?,m_Col11=?,m_Col12=?,m_Col13=? ")
	               _T("WHERE ReportResultRowsID=?") );
	return query.GetRowCount();
}

int TReportResultRows::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ReportResultID );
	query.Execute( _T("DELETE FROM ReportResultRows ")
	               _T("WHERE ReportResultID=?") );
	return query.GetRowCount();
}

void TReportResultRows::Query( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_TCHAR( query, m_Col1 );
	BINDCOL_TCHAR( query, m_Col2 );
	BINDCOL_LONG( query, m_Col3 );
	BINDCOL_LONG( query, m_Col4 );
	BINDCOL_LONG( query, m_Col5 );
	BINDCOL_LONG( query, m_Col6 );
	BINDCOL_LONG( query, m_Col7 );
	BINDCOL_LONG( query, m_Col8 );
	BINDCOL_LONG( query, m_Col9 );
	BINDCOL_LONG( query, m_Col10 );
	BINDCOL_LONG( query, m_Col11 );
	BINDCOL_LONG( query, m_Col12 );
	BINDCOL_LONG( query, m_Col13 );
	BINDPARAM_LONG( query, m_ReportResultID );
	query.Execute( _T("SELECT Col1,Col2,Col3,Col4,Col5,Col6,Col7,Col8,Col9,Col10,Col11,Col12,Col13 ")
	               _T("FROM ReportResultRows ")
	               _T("WHERE ReportResultID=?") );	
}

int TReportResultRows::Query2( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_ReportResultID );
	BINDCOL_TCHAR( query, m_Col1 );
	BINDCOL_TCHAR( query, m_Col2 );
	BINDCOL_LONG( query, m_Col3 );
	BINDCOL_LONG( query, m_Col4 );
	BINDCOL_LONG( query, m_Col5 );
	BINDCOL_LONG( query, m_Col6 );
	BINDCOL_LONG( query, m_Col7 );
	BINDCOL_LONG( query, m_Col8 );
	BINDCOL_LONG( query, m_Col9 );
	BINDCOL_LONG( query, m_Col10 );
	BINDCOL_LONG( query, m_Col11 );
	BINDCOL_LONG( query, m_Col12 );
	BINDCOL_LONG( query, m_Col13 );
	BINDPARAM_LONG( query, m_ReportResultRowsID );
	query.Execute( _T("SELECT ReportResultID,Col1,Col2,Col3,Col4,Col5,Col6,Col7,Col8,Col9,Col10,Col11,Col12,Col13 ")
	               _T("FROM ReportResultRows ")
	               _T("WHERE ReportResultRowsID=?") );
	nResult = query.Fetch();
	return nResult;
}

void TReportResultRows::GetLongData( CODBCQuery& query )
{
}

/* class TScheduleReport - 
		Report Scheduler.
	 */
void TScheduleReport::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_ReportID );
	BINDPARAM_BIT( query, m_IsCustom );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_CreatedByID );
	BINDPARAM_LONG( query, m_EditedByID );
	BINDPARAM_TIME( query, m_NextRunTime );
	BINDPARAM_LONG( query, m_RunFreq );
	BINDPARAM_LONG( query, m_RunOn );
	BINDPARAM_BIT( query, m_RunSun );
	BINDPARAM_BIT( query, m_RunMon );
	BINDPARAM_BIT( query, m_RunTue );
	BINDPARAM_BIT( query, m_RunWed );
	BINDPARAM_BIT( query, m_RunThur );
	BINDPARAM_BIT( query, m_RunFri );
	BINDPARAM_BIT( query, m_RunSat );
	BINDPARAM_LONG( query, m_RunAtHour );
	BINDPARAM_LONG( query, m_RunAtMin );
	BINDPARAM_LONG( query, m_SumPeriod );
	BINDPARAM_LONG( query, m_MaxResults );
	BINDPARAM_BIT( query, m_SendAlertToOwner );
	BINDPARAM_BIT( query, m_SendResultToEmail );
	BINDPARAM_TCHAR( query, m_ResultEmailTo );
	BINDPARAM_BIT( query, m_IncludeResultFile );
	BINDPARAM_BIT( query, m_SaveResultToFile );
	BINDPARAM_LONG( query, m_KeepNumResultFile );
	BINDPARAM_BIT( query, m_AllowConsolidation );
	BINDPARAM_LONG( query, m_TargetID );
	BINDPARAM_BIT( query, m_Flag1 );
	BINDPARAM_BIT( query, m_Flag2 );
	BINDPARAM_LONG( query, m_TicketStateID );
	query.Execute( _T("INSERT INTO ReportScheduled ")
	               _T("(Description,ReportID,IsCustom,IsEnabled,OwnerID,CreatedBy,EditedBy,NextRunTime,RunFreq,RunOn,RunSun,RunMon,RunTue,RunWed,RunThur,RunFri,RunSat,RunAtHour,RunAtMin,SumPeriod,MaxResults,SendAlertToOwner,SendResultToEmail,ResultEmailTo,IncludeResultFile,SaveResultToFile,KeepNumResultFile,AllowConsolidation,TargetID,Flag1,Flag2,TicketStateID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_ScheduledReportID = query.GetLastInsertedID();
}

int TScheduleReport::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_ReportID );
	BINDPARAM_BIT( query, m_IsCustom );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_EditedByID );
	BINDPARAM_TIME( query, m_DateEdited );
	BINDPARAM_TIME( query, m_NextRunTime );
	BINDPARAM_LONG( query, m_RunFreq );
	BINDPARAM_LONG( query, m_RunOn );
	BINDPARAM_BIT( query, m_RunSun );
	BINDPARAM_BIT( query, m_RunMon );
	BINDPARAM_BIT( query, m_RunTue );
	BINDPARAM_BIT( query, m_RunWed );
	BINDPARAM_BIT( query, m_RunThur );
	BINDPARAM_BIT( query, m_RunFri );
	BINDPARAM_BIT( query, m_RunSat );
	BINDPARAM_LONG( query, m_RunAtHour );
	BINDPARAM_LONG( query, m_RunAtMin );
	BINDPARAM_LONG( query, m_SumPeriod );
	BINDPARAM_LONG( query, m_MaxResults );
	BINDPARAM_BIT( query, m_SendAlertToOwner );
	BINDPARAM_BIT( query, m_SendResultToEmail );
	BINDPARAM_TCHAR( query, m_ResultEmailTo );
	BINDPARAM_BIT( query, m_IncludeResultFile );
	BINDPARAM_BIT( query, m_SaveResultToFile );
	BINDPARAM_LONG( query, m_KeepNumResultFile );
	BINDPARAM_BIT( query, m_AllowConsolidation );
	BINDPARAM_LONG( query, m_TargetID );
	BINDPARAM_BIT( query, m_Flag1 );
	BINDPARAM_BIT( query, m_Flag2 );
	BINDPARAM_LONG( query, m_TicketStateID );
	BINDPARAM_LONG( query, m_ScheduledReportID );
	query.Execute( _T("UPDATE ReportScheduled ")
	               _T("SET Description=?,ReportID=?,IsCustom=?,IsEnabled=?,OwnerID=?,EditedBy=?,DateEdited=?,NextRunTime=?,RunFreq=?,RunOn=?,RunSun=?,RunMon=?,RunTue=?,RunWed=?,RunThur=?,RunFri=?,RunSat=?,RunAtHour=?,RunAtMin=?,SumPeriod=?,MaxResults=?,SendAlertToOwner=?,SendResultToEmail=?,ResultEmailTo=?,IncludeResultFile=?,SaveResultToFile=?,KeepNumResultFile=?,AllowConsolidation=?,TargetID=?,Flag1=?,Flag2=?,TicketStateID=? ")
	               _T("WHERE ScheduledReportID=?") );
	return query.GetRowCount();
}

int TScheduleReport::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ScheduledReportID );
	query.Execute( _T("DELETE FROM ReportScheduled ")
	               _T("WHERE ScheduledReportID=?") );
	return query.GetRowCount();
}

int TScheduleReport::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_ReportID );
	BINDCOL_BIT( query, m_IsCustom );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_LONG( query, m_EditedByID );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_TIME( query, m_LastRunTime );
	BINDCOL_LONG( query, m_LastResultCode );
	BINDCOL_TIME( query, m_NextRunTime );
	BINDCOL_LONG( query, m_RunFreq );
	BINDCOL_LONG( query, m_RunOn );
	BINDCOL_BIT( query, m_RunSun );
	BINDCOL_BIT( query, m_RunMon );
	BINDCOL_BIT( query, m_RunTue );
	BINDCOL_BIT( query, m_RunWed );
	BINDCOL_BIT( query, m_RunThur );
	BINDCOL_BIT( query, m_RunFri );
	BINDCOL_BIT( query, m_RunSat );
	BINDCOL_LONG( query, m_RunAtHour );
	BINDCOL_LONG( query, m_RunAtMin );
	BINDCOL_LONG( query, m_SumPeriod );
	BINDCOL_LONG( query, m_MaxResults );
	BINDCOL_BIT( query, m_SendAlertToOwner );
	BINDCOL_BIT( query, m_SendResultToEmail );
	BINDCOL_TCHAR( query, m_ResultEmailTo );
	BINDCOL_BIT( query, m_IncludeResultFile );
	BINDCOL_BIT( query, m_SaveResultToFile );
	BINDCOL_LONG( query, m_KeepNumResultFile );
	BINDCOL_BIT( query, m_AllowConsolidation );
	BINDCOL_LONG( query, m_TargetID );
	BINDCOL_BIT( query, m_Flag1 );
	BINDCOL_BIT( query, m_Flag2 );
	BINDCOL_LONG( query, m_TicketStateID );
	BINDPARAM_LONG( query, m_ScheduledReportID );
	query.Execute( _T("SELECT Description,ReportID,IsCustom,IsEnabled,OwnerID,CreatedBy,DateCreated,EditedBy,DateEdited,LastRunTime,LastResultCode,NextRunTime,RunFreq,RunOn,RunSun,RunMon,RunTue,RunWed,RunThur,RunFri,RunSat,RunAtHour,RunAtMin,SumPeriod,MaxResults,SendAlertToOwner,SendResultToEmail,ResultEmailTo,IncludeResultFile,SaveResultToFile,KeepNumResultFile,AllowConsolidation,TargetID,Flag1,Flag2,TicketStateID ")
	               _T("FROM ReportScheduled ")
	               _T("WHERE ScheduledReportID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TScheduleReport::GetLongData( CODBCQuery& query )
{
}

void TScheduleReport::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ScheduledReportID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_LONG( query, m_EditedByID );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_TIME( query, m_LastRunTime );
	BINDCOL_LONG( query, m_LastResultCode );
	BINDCOL_TIME( query, m_NextRunTime );
	query.Execute( _T("SELECT ScheduledReportID,Description,IsEnabled,OwnerID,CreatedBy,DateCreated,EditedBy,DateEdited,LastRunTime,LastResultCode,NextRunTime ")
	               _T("FROM ReportScheduled ")
				   _T("ORDER BY OwnerID,Description") );
}

void TScheduleReport::PrepareOwnerList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ScheduledReportID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_LONG( query, m_EditedByID );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_TIME( query, m_LastRunTime );
	BINDCOL_LONG( query, m_LastResultCode );
	BINDCOL_TIME( query, m_NextRunTime );
	BINDPARAM_LONG( query, m_OwnerID );
	query.Execute( _T("SELECT ScheduledReportID,Description,IsEnabled,CreatedBy,DateCreated,EditedBy,DateEdited,LastRunTime,LastResultCode,NextRunTime ")
	               _T("FROM ReportScheduled ")
				   _T("WHERE OwnerID=? ") 
	               _T("ORDER BY Description") );
}

/* class TStandardReport - 
		Standard Reports
	 */

void TStandardReport::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_CanSchedule );
	BINDPARAM_LONG( query, m_ReportTypeID );
	BINDPARAM_LONG( query, m_ReportObjectID );
	query.Execute( _T("INSERT INTO ReportStandard ")
	               _T("(Description,IsEnabled,CanSchedule,ReportTypeID,ReportObjectID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?)") );
	m_StandardReportID = query.GetLastInsertedID();
}

int TStandardReport::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_CanSchedule );
	BINDPARAM_LONG( query, m_ReportTypeID );
	BINDPARAM_LONG( query, m_ReportObjectID );
	BINDPARAM_LONG( query, m_StandardReportID );
	query.Execute( _T("UPDATE ReportStandard ")
	               _T("SET Description=?,IsEnabled=?,CanSchedule=?,ReportTypeID=?,ReportObjectID=? ")
	               _T("WHERE StandardReportID=?") );
	return query.GetRowCount();
}

int TStandardReport::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_CanSchedule );
	BINDCOL_LONG( query, m_ReportTypeID );
	BINDCOL_LONG( query, m_ReportObjectID );
	BINDPARAM_LONG( query, m_StandardReportID );
	query.Execute( _T("SELECT Description,IsEnabled,CanSchedule,ReportTypeID,ReportObjectID ")
	               _T("FROM ReportStandard ")
	               _T("WHERE StandardReportID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TStandardReport::GetLongData( CODBCQuery& query )
{
}

void TStandardReport::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_StandardReportID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_CanSchedule );
	BINDCOL_LONG( query, m_ReportTypeID );
	BINDCOL_LONG( query, m_ReportObjectID );
	
	query.Execute( _T("SELECT StandardReportID,Description,IsEnabled,CanSchedule,ReportTypeID,ReportObjectID ")
	               _T("FROM ReportStandard ")
	               _T("Order By Description") );
}


/* class TCustomReport - 
		Custom Reports
	 */

void TCustomReport::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_ReportTypeID );
	BINDPARAM_LONG( query, m_ReportObjectID );
	BINDPARAM_BIT( query, m_BuiltIn );
	BINDPARAM_LONG( query, m_CreatedBy );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_EditedBy );
	BINDPARAM_TIME( query, m_DateEdited );
	BINDPARAM_TEXT( query, m_Query );
	query.Execute( _T("INSERT INTO ReportCustom ")
	               _T("(Description,IsEnabled,ReportTypeID,ReportObjectID,BuiltIn,CreatedBy,DateCreated,EditedBy,DateEdited,Query) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?)") );
	m_CustomReportID = query.GetLastInsertedID();
}

int TCustomReport::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_ReportTypeID );
	BINDPARAM_LONG( query, m_ReportObjectID );
	BINDPARAM_BIT( query, m_BuiltIn );
	BINDPARAM_LONG( query, m_CreatedBy );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_EditedBy );
	BINDPARAM_TIME( query, m_DateEdited );
	BINDPARAM_TEXT( query, m_Query );
	BINDPARAM_LONG( query, m_CustomReportID );
	query.Execute( _T("UPDATE ReportCustom ")
	               _T("SET Description=?,IsEnabled=?,ReportTypeID=?,ReportObjectID=?,BuiltIn=?CreatedBy=?,DateCreated=?,EditedBy=?,DateEdited=?,Query=? ")
	               _T("WHERE CustomReportID=?") );
	return query.GetRowCount();
}

int TCustomReport::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_ReportTypeID );
	BINDCOL_LONG( query, m_ReportObjectID );
	BINDCOL_BIT( query, m_BuiltIn );
	BINDCOL_LONG( query, m_CreatedBy );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_LONG( query, m_EditedBy );
	BINDCOL_TIME( query, m_DateEdited );
	BINDPARAM_LONG( query, m_CustomReportID );
	query.Execute( _T("SELECT Description,IsEnabled,ReportTypeID,ReportObjectID,BuiltIn,CreatedBy,DateCreated,EditedBy,DateEdited,Query ")
	               _T("FROM ReportCustom ")
	               _T("WHERE CustomReportID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TCustomReport::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Query );
}

void TCustomReport::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_ReportTypeID );
	BINDCOL_LONG( query, m_ReportObjectID );
	BINDCOL_BIT( query, m_BuiltIn );
	BINDCOL_LONG( query, m_CreatedBy );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_LONG( query, m_EditedBy );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_LONG( query, m_CustomReportID );
	query.Execute( _T("SELECT Description,IsEnabled,ReportTypeID,ReportObjectID,BuiltIn,CreatedBy,DateCreated,EditedBy,DateEdited,CustomReportID,Query ")
	               _T("FROM ReportCustom ")
	               _T("Order By Description") );
}

/* class TAutoMessages - 
		Auto Messages.
	 */
void TAutoMessages::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_LONG( query, m_FailCount );
	BINDPARAM_LONG( query, m_TotalCount );
	BINDPARAM_LONG( query, m_SendToTypeID );
	BINDPARAM_LONG( query, m_SendToID );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_WhenToSendVal );
	BINDPARAM_LONG( query, m_WhenToSendFreq );
	BINDPARAM_LONG( query, m_WhenToSendTypeID );
	BINDPARAM_LONG( query, m_SrToSendID );
	BINDPARAM_LONG( query, m_SendFromTypeID );
	BINDPARAM_BIT( query, m_CreateNewTicket );
	BINDPARAM_TCHAR( query, m_ReplyToAddress );
	BINDPARAM_TCHAR( query, m_ReplyToName );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	BINDPARAM_LONG( query, m_CreatedByID );
	BINDPARAM_LONG( query, m_EditedByID );
	BINDPARAM_LONG( query, m_SendToPercent );
	BINDPARAM_LONG( query, m_OmitTracking );
	query.Execute( _T("INSERT INTO AutoMessages ")
	               _T("(Description,HitCount,FailCount,TotalCount,SendToTypeID,SendToID,IsEnabled,WhenToSendVal,WhenToSendFreq,WhenToSendTypeID,SrToSendID,SendFromTypeID,CreateNewTicket,ReplyToAddress,ReplyToName,OwnerID,TicketBoxID,TicketCategoryID,CreatedByID,EditedByID,SendToPercent,OmitTracking) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_AutoMessageID = query.GetLastInsertedID();
}

int TAutoMessages::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_LONG( query, m_FailCount );
	BINDPARAM_LONG( query, m_TotalCount );
	BINDPARAM_LONG( query, m_SendToTypeID );
	BINDPARAM_LONG( query, m_SendToID );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_WhenToSendVal );
	BINDPARAM_LONG( query, m_WhenToSendFreq );
	BINDPARAM_LONG( query, m_WhenToSendTypeID );
	BINDPARAM_LONG( query, m_SrToSendID );
	BINDPARAM_LONG( query, m_SendFromTypeID );
	BINDPARAM_BIT( query, m_CreateNewTicket );
	BINDPARAM_TCHAR( query, m_ReplyToAddress );
	BINDPARAM_TCHAR( query, m_ReplyToName );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	BINDPARAM_TIME( query, m_DateEdited );
	BINDPARAM_LONG( query, m_EditedByID );
	BINDPARAM_LONG( query, m_SendToPercent );
	BINDPARAM_LONG( query, m_OmitTracking );
	BINDPARAM_LONG( query, m_AutoMessageID );
	query.Execute( _T("UPDATE AutoMessages ")
	               _T("SET Description=?,HitCount=?,FailCount=?,TotalCount=?,SendToTypeID=?,SendToID=?,IsEnabled=?,WhenToSendVal=?,WhenToSendFreq=?,WhenToSendTypeID=?,SrToSendID=?,SendFromTypeId=?,CreateNewTicket=?,ReplyToAddress=?,ReplyToName=?,OwnerID=?,TicketBoxID=?,TicketCategoryID=?,DateEdited=?,EditedByID=?,SendToPercent=?,OmitTracking=? ")
	               _T("WHERE AutoMessageID=?") );
	return query.GetRowCount();
}

int TAutoMessages::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoMessageID );
	query.Execute( _T("DELETE FROM AutoMessages ")
	               _T("WHERE AutoMessageID=?") );
	return query.GetRowCount();
}

int TAutoMessages::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_FailCount );
	BINDCOL_LONG( query, m_TotalCount );
	BINDCOL_LONG( query, m_SendToTypeID );
	BINDCOL_LONG( query, m_SendToID );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_WhenToSendVal );
	BINDCOL_LONG( query, m_WhenToSendFreq );
	BINDCOL_LONG( query, m_WhenToSendTypeID );
	BINDCOL_LONG( query, m_SrToSendID );
	BINDCOL_LONG( query, m_SendFromTypeID );
	BINDCOL_BIT( query, m_CreateNewTicket );
	BINDCOL_TCHAR( query, m_ReplyToAddress );
	BINDCOL_TCHAR( query, m_ReplyToName );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_TicketCategoryID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_LONG( query, m_EditedByID );
	BINDCOL_LONG( query, m_SendToPercent );
	BINDCOL_LONG( query, m_OmitTracking );
	BINDPARAM_LONG( query, m_AutoMessageID );
	query.Execute( _T("SELECT Description,HitCount,FailCount,TotalCount,SendToTypeID,SendToID,IsEnabled,WhenToSendVal,WhenToSendFreq,WhenToSendTypeID,SrToSendID,SendFromTypeID,CreateNewTicket,ReplyToAddress,ReplyToName,OwnerID,TicketBoxID,TicketCategoryID,DateCreated,DateEdited,CreatedByID,EditedByID,SendToPercent,OmitTracking ")
	               _T("FROM AutoMessages ")
	               _T("WHERE AutoMessageID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAutoMessages::GetLongData( CODBCQuery& query )
{
}

void TAutoMessages::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AutoMessageID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_FailCount );
	BINDCOL_LONG( query, m_TotalCount );
	BINDCOL_LONG( query, m_SendToTypeID );
	BINDCOL_LONG( query, m_SendToID );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_WhenToSendVal );
	BINDCOL_LONG( query, m_WhenToSendFreq );
	BINDCOL_LONG( query, m_WhenToSendTypeID );
	BINDCOL_LONG( query, m_SrToSendID );
	BINDCOL_LONG( query, m_SendFromTypeID );
	BINDCOL_BIT( query, m_CreateNewTicket );
	BINDCOL_TCHAR( query, m_ReplyToAddress );
	BINDCOL_TCHAR( query, m_ReplyToName );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_TicketCategoryID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_LONG( query, m_EditedByID );
	BINDCOL_LONG( query, m_SendToPercent );
	BINDCOL_LONG( query, m_OmitTracking );
	query.Execute( _T("SELECT AutoMessageID,Description,HitCount,FailCount,TotalCount,SendToTypeID,SendToID,IsEnabled,WhenToSendVal,WhenToSendFreq,WhenToSendTypeID,SrToSendID,SendFromTypeID,CreateNewTicket,ReplyToAddress,ReplyToName,OwnerID,TicketBoxID,TicketCategoryID,DateCreated,DateEdited,CreatedByID,EditedByID,SendToPercent,OmitTracking ")
	               _T("FROM AutoMessages ")
	               _T("ORDER BY Description") );
}


/* class TAutoMessagesSent - 
		
	 */
void TAutoMessagesSent::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoMessageID );
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_ResultCode );
	query.Execute( _T("INSERT INTO AutoMessagesSent ")
	               _T("(AutoMessageID,TicketID,ResultCode) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_AutoMessageSentID = query.GetLastInsertedID();
}

int TAutoMessagesSent::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoMessageID );
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_ResultCode );
	BINDPARAM_LONG( query, m_AutoMessageSentID );
	query.Execute( _T("UPDATE AutoMessagesSent ")
	               _T("SET AutoMessageID=?,TicketID=?,ReultCode=? ")
	               _T("WHERE AutoMessageSentID=?") );
	return query.GetRowCount();
}

int TAutoMessagesSent::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoMessageSentID );
	query.Execute( _T("DELETE FROM AutoMessagesSent ")
	               _T("WHERE AutoMessageSentID=?") );
	return query.GetRowCount();
}

int TAutoMessagesSent::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AutoMessageID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_ResultCode );
	BINDPARAM_LONG( query, m_AutoMessageSentID );
	query.Execute( _T("SELECT AutoMessageID,TicketID,ResultCode ")
	               _T("FROM AutoMessagesSent ")
	               _T("WHERE AutoMessageSentID=?") );
	nResult = query.Fetch();
	return nResult;
}

void TAutoMessagesSent::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AutoMessageID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_ResultCode );
	BINDCOL_LONG( query, m_AutoMessageSentID );
	query.Execute( _T("SELECT AutoMessageID,TicketID,ResultCode,AutoMessageSentID ")
	               _T("FROM AutoMessagesSent ")
	               _T("ORDER BY AutoMessageID") );
}

/* class TAutoResponses - 
		AutoResponses.
	 */
void TAutoResponses::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_LONG( query, m_FailCount );
	BINDPARAM_LONG( query, m_TotalCount );
	BINDPARAM_LONG( query, m_SendToTypeID );
	BINDPARAM_LONG( query, m_SendToID );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_WhenToSendVal );
	BINDPARAM_LONG( query, m_WhenToSendFreq );
	BINDPARAM_LONG( query, m_WhenToSendTypeID );
	BINDPARAM_LONG( query, m_SrToSendID );
	BINDPARAM_BIT( query, m_AutoReplyQuoteMsg );
	BINDPARAM_BIT( query, m_AutoReplyCloseTicket );
	BINDPARAM_BIT( query, m_AutoReplyInTicket );
	BINDPARAM_LONG( query, m_HeaderID );
	BINDPARAM_LONG( query, m_FooterID );
	BINDPARAM_LONG( query, m_SendFromTypeID );
	BINDPARAM_TCHAR( query, m_ReplyToAddress );
	BINDPARAM_TCHAR( query, m_ReplyToName );
	BINDPARAM_LONG( query, m_CreatedByID );
	BINDPARAM_LONG( query, m_EditedByID );
	BINDPARAM_LONG( query, m_SendToPercent );
	BINDPARAM_LONG( query, m_OmitTracking );
	BINDPARAM_TINYINT( query, m_FooterLocation );
	query.Execute( _T("INSERT INTO AutoResponses ")
	               _T("(Description,HitCount,FailCount,TotalCount,SendToTypeID,SendToID,IsEnabled,WhenToSendVal,WhenToSendFreq,WhenToSendTypeID,SrToSendID,AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket,HeaderID,FooterID,SendFromTypeID,ReplyToAddress,ReplyToName,CreatedByID,EditedByID,SendToPercent,OmitTracking,FooterLocation) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_AutoResponseID = query.GetLastInsertedID();
}

int TAutoResponses::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_LONG( query, m_FailCount );
	BINDPARAM_LONG( query, m_TotalCount );
	BINDPARAM_LONG( query, m_SendToTypeID );
	BINDPARAM_LONG( query, m_SendToID );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_WhenToSendVal );
	BINDPARAM_LONG( query, m_WhenToSendFreq );
	BINDPARAM_LONG( query, m_WhenToSendTypeID );
	BINDPARAM_LONG( query, m_SrToSendID );
	BINDPARAM_BIT( query, m_AutoReplyQuoteMsg );
	BINDPARAM_BIT( query, m_AutoReplyCloseTicket );
	BINDPARAM_BIT( query, m_AutoReplyInTicket );
	BINDPARAM_LONG( query, m_HeaderID );
	BINDPARAM_LONG( query, m_FooterID );
	BINDPARAM_LONG( query, m_SendFromTypeID );
	BINDPARAM_TCHAR( query, m_ReplyToAddress );
	BINDPARAM_TCHAR( query, m_ReplyToName );
	BINDPARAM_TIME( query, m_DateEdited );
	BINDPARAM_LONG( query, m_EditedByID );
	BINDPARAM_LONG( query, m_SendToPercent );
	BINDPARAM_LONG( query, m_OmitTracking );
	BINDPARAM_TINYINT( query, m_FooterLocation );
	BINDPARAM_LONG( query, m_AutoResponseID );
	query.Execute( _T("UPDATE AutoResponses ")
	               _T("SET Description=?,HitCount=?,FailCount=?,TotalCount=?,SendToTypeID=?,SendToID=?,IsEnabled=?,WhenToSendVal=?,WhenToSendFreq=?,WhenToSendTypeID=?,SrToSendID=?,AutoReplyQuoteMsg=?,AutoReplyCloseTicket=?,AutoReplyInTicket=?,HeaderID=?,FooterID=?,SendFromTypeID=?,ReplyToAddress=?,ReplyToName=?,DateEdited=?,EditedByID=?,SendToPercent=?,OmitTracking=?,FooterLocation=? ")
	               _T("WHERE AutoResponseID=?") );
	return query.GetRowCount();
}

int TAutoResponses::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoResponseID );
	query.Execute( _T("DELETE FROM AutoResponses ")
	               _T("WHERE AutoResponseID=?") );
	return query.GetRowCount();
}

int TAutoResponses::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_FailCount );
	BINDCOL_LONG( query, m_TotalCount );
	BINDCOL_LONG( query, m_SendToTypeID );
	BINDCOL_LONG( query, m_SendToID );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_WhenToSendVal );
	BINDCOL_LONG( query, m_WhenToSendFreq );
	BINDCOL_LONG( query, m_WhenToSendTypeID );
	BINDCOL_LONG( query, m_SrToSendID );
	BINDCOL_BIT( query, m_AutoReplyQuoteMsg );
	BINDCOL_BIT( query, m_AutoReplyCloseTicket );
	BINDCOL_BIT( query, m_AutoReplyInTicket );
	BINDCOL_LONG( query, m_HeaderID );
	BINDCOL_LONG( query, m_FooterID );
	BINDCOL_LONG( query, m_SendFromTypeID );
	BINDCOL_TCHAR( query, m_ReplyToAddress );
	BINDCOL_TCHAR( query, m_ReplyToName );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_LONG( query, m_EditedByID );
	BINDCOL_LONG( query, m_SendToPercent );
	BINDCOL_LONG( query, m_OmitTracking );
	BINDCOL_TINYINT( query, m_FooterLocation );
	BINDPARAM_LONG( query, m_AutoResponseID );
	query.Execute( _T("SELECT Description,HitCount,FailCount,TotalCount,SendToTypeID,SendToID,IsEnabled,WhenToSendVal,WhenToSendFreq,WhenToSendTypeID,SrToSendID,AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket,HeaderID,FooterID,SendFromTypeID,ReplyToAddress,ReplyToName,DateCreated,DateEdited,CreatedByID,EditedByID,SendToPercent,OmitTracking,FooterLocation ")
	               _T("FROM AutoResponses ")
	               _T("WHERE AutoResponseID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAutoResponses::GetLongData( CODBCQuery& query )
{
}

void TAutoResponses::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AutoResponseID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_FailCount );
	BINDCOL_LONG( query, m_TotalCount );
	BINDCOL_LONG( query, m_SendToTypeID );
	BINDCOL_LONG( query, m_SendToID );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_WhenToSendVal );
	BINDCOL_LONG( query, m_WhenToSendFreq );
	BINDCOL_LONG( query, m_WhenToSendTypeID );
	BINDCOL_LONG( query, m_SrToSendID );
	BINDCOL_BIT( query, m_AutoReplyQuoteMsg );
	BINDCOL_BIT( query, m_AutoReplyCloseTicket );
	BINDCOL_BIT( query, m_AutoReplyInTicket );
	BINDCOL_LONG( query, m_HeaderID );
	BINDCOL_LONG( query, m_FooterID );
	BINDCOL_LONG( query, m_SendFromTypeID );
	BINDCOL_TCHAR( query, m_ReplyToAddress );
	BINDCOL_TCHAR( query, m_ReplyToName );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_LONG( query, m_EditedByID );
	BINDCOL_LONG( query, m_SendToPercent );
	BINDCOL_LONG( query, m_OmitTracking );
	BINDCOL_TINYINT( query, m_FooterLocation );
	query.Execute( _T("SELECT AutoResponseID,Description,HitCount,FailCount,TotalCount,SendToTypeID,SendToID,IsEnabled,WhenToSendVal,WhenToSendFreq,WhenToSendTypeID,SrToSendID,AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket,HeaderID,FooterID,SendFromTypeID,ReplyToAddress,ReplyToName,DateCreated,DateEdited,CreatedByID,EditedByID,SendToPercent,OmitTracking,FooterLocation ")
	               _T("FROM AutoResponses ")
	               _T("ORDER BY Description") );
}


/* class TAutoResponsesSent - 
		
	 */
void TAutoResponsesSent::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoResponseID );
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_ResultCode );
	query.Execute( _T("INSERT INTO AutoResponsesSent ")
	               _T("(AutoResponseID,TicketID,ResultCode) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_AutoResponseSentID = query.GetLastInsertedID();
}

int TAutoResponsesSent::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoResponseID );
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_ResultCode );
	BINDPARAM_LONG( query, m_AutoResponseSentID );
	query.Execute( _T("UPDATE AutoResponsesSent ")
	               _T("SET AutoResponseID=?,TicketID=?,ReultCode=? ")
	               _T("WHERE AutoResponseSentID=?") );
	return query.GetRowCount();
}

int TAutoResponsesSent::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AutoResponseSentID );
	query.Execute( _T("DELETE FROM AutoResponsesSent ")
	               _T("WHERE AutoResponseSentID=?") );
	return query.GetRowCount();
}

int TAutoResponsesSent::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AutoResponseID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_ResultCode );
	BINDPARAM_LONG( query, m_AutoResponseSentID );
	query.Execute( _T("SELECT AutoResponseID,TicketID,ResultCode ")
	               _T("FROM AutoResponsesSent ")
	               _T("WHERE AutoResponseSentID=?") );
	nResult = query.Fetch();
	return nResult;
}

void TAutoResponsesSent::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AutoResponseID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_ResultCode );
	BINDCOL_LONG( query, m_AutoResponseSentID );
	query.Execute( _T("SELECT AutoResponseID,TicketID,ResultCode,AutoResponseSentID ")
	               _T("FROM AutoResponsesSent ")
	               _T("ORDER BY AutoResponseID") );
}

/* class TWaterMarkAlerts - 
		Custom WaterMark Alerts.
	 */
void TWaterMarkAlerts::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_LONG( query, m_LowWaterMark );
	BINDPARAM_LONG( query, m_HighWaterMark );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_SendLowAlert );
	BINDPARAM_LONG( query, m_AlertOnTypeID );
	BINDPARAM_LONG( query, m_AlertOnID );
	BINDPARAM_LONG( query, m_AlertToTypeID );
	BINDPARAM_LONG( query, m_AlertToID );
	BINDPARAM_LONG( query, m_AlertMethodID );
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_TCHAR( query, m_FromEmailAddress );
	BINDPARAM_LONG( query, m_CreatedByID );
	BINDPARAM_LONG( query, m_EditedByID );
	BINDPARAM_TINYINT( query, m_WaterMarkStatus );	
	query.Execute( _T("INSERT INTO WaterMarkAlerts ")
	               _T("(Description,HitCount,LowWaterMark,HighWaterMark,IsEnabled,SendLowAlert,AlertOnTypeID,AlertOnID,AlertToTypeID,AlertToID,AlertMethodID,EmailAddress,FromEmailAddress,CreatedByID,EditedByID,WaterMarkStatus) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_WaterMarkAlertID = query.GetLastInsertedID();
}

int TWaterMarkAlerts::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_LONG( query, m_LowWaterMark );
	BINDPARAM_LONG( query, m_HighWaterMark );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_SendLowAlert );
	BINDPARAM_LONG( query, m_AlertOnTypeID );
	BINDPARAM_LONG( query, m_AlertOnID );
	BINDPARAM_LONG( query, m_AlertToTypeID );
	BINDPARAM_LONG( query, m_AlertToID );
	BINDPARAM_LONG( query, m_AlertMethodID );
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_TCHAR( query, m_FromEmailAddress );
	BINDPARAM_TIME( query, m_DateEdited );
	BINDPARAM_LONG( query, m_EditedByID );
	BINDPARAM_TINYINT( query, m_WaterMarkStatus );		
	BINDPARAM_LONG( query, m_WaterMarkAlertID );
	query.Execute( _T("UPDATE WaterMarkAlerts ")
	               _T("SET Description=?,HitCount=?,LowWaterMark=?,HighWaterMark=?,IsEnabled=?,SendLowAlert=?,AlertOnTypeID=?,AlertOnID=?,AlertToTypeID=?,AlertToID=?,AlertMethodID=?,EmailAddress=?,FromEmailAddress=?,DateEdited=?,EditedByID=?,WaterMarkStatus=? ")
	               _T("WHERE WaterMarkAlertID=?") );
	return query.GetRowCount();
}

int TWaterMarkAlerts::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_WaterMarkAlertID );
	query.Execute( _T("DELETE FROM WaterMarkAlerts ")
	               _T("WHERE WaterMarkAlertID=?") );
	return query.GetRowCount();
}

int TWaterMarkAlerts::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_LowWaterMark );
	BINDCOL_LONG( query, m_HighWaterMark );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_SendLowAlert );
	BINDCOL_LONG( query, m_AlertOnTypeID );
	BINDCOL_LONG( query, m_AlertOnID );
	BINDCOL_LONG( query, m_AlertToTypeID );
	BINDCOL_LONG( query, m_AlertToID );
	BINDCOL_LONG( query, m_AlertMethodID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_TCHAR( query, m_FromEmailAddress );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_LONG( query, m_EditedByID );
	BINDCOL_TINYINT( query, m_WaterMarkStatus );	
	BINDPARAM_LONG( query, m_WaterMarkAlertID );
	query.Execute( _T("SELECT Description,HitCount,LowWaterMark,HighWaterMark,IsEnabled,SendLowAlert,AlertOnTypeID,AlertOnID,AlertToTypeID,AlertToID,AlertMethodID,EmailAddress,FromEmailAddress,DateCreated,DateEdited,CreatedByID,EditedByID,WaterMarkStatus ")
	               _T("FROM WaterMarkAlerts ")
	               _T("WHERE WaterMarkAlertID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TWaterMarkAlerts::GetLongData( CODBCQuery& query )
{
}

void TWaterMarkAlerts::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_WaterMarkAlertID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_LowWaterMark );
	BINDCOL_LONG( query, m_HighWaterMark );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_SendLowAlert );
	BINDCOL_LONG( query, m_AlertOnTypeID );
	BINDCOL_LONG( query, m_AlertOnID );
	BINDCOL_LONG( query, m_AlertToTypeID );
	BINDCOL_LONG( query, m_AlertToID );
	BINDCOL_LONG( query, m_AlertMethodID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_TCHAR( query, m_FromEmailAddress );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_LONG( query, m_EditedByID );
	BINDCOL_TINYINT( query, m_WaterMarkStatus );	
	query.Execute( _T("SELECT WaterMarkAlertID,Description,HitCount,LowWaterMark,HighWaterMark,IsEnabled,SendLowAlert,AlertOnTypeID,AlertOnID,AlertToTypeID,AlertToID,AlertMethodID,EmailAddress,FromEmailAddress,DateCreated,DateEdited,CreatedByID,EditedByID,WaterMarkStatus ")
	               _T("FROM WaterMarkAlerts ")
	               _T("ORDER BY Description") );
}


/* class TAgeAlerts - 
		Custom Age Alerts.
	 */
void TAgeAlerts::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_LONG( query, m_ThresholdMins );
	BINDPARAM_LONG( query, m_ThresholdFreq );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_NoAlertIfRepliedTo );
	BINDPARAM_LONG( query, m_AlertOnTypeID );
	BINDPARAM_LONG( query, m_AlertOnID );
	BINDPARAM_LONG( query, m_AlertToTypeID );
	BINDPARAM_LONG( query, m_AlertToID );
	BINDPARAM_LONG( query, m_AlertMethodID );
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_TCHAR( query, m_FromEmailAddress );
	BINDPARAM_LONG( query, m_CreatedByID );
	BINDPARAM_LONG( query, m_EditedByID );
	query.Execute( _T("INSERT INTO AgeAlerts ")
	               _T("(Description,HitCount,ThresholdMins,ThresholdFreq,IsEnabled,NoAlertIfRepliedTo,AlertOnTypeID,AlertOnID,AlertToTypeID,AlertToID,AlertMethodID,EmailAddress,FromEmailAddress,CreatedByID,EditedByID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_AgeAlertID = query.GetLastInsertedID();
}

int TAgeAlerts::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_LONG( query, m_ThresholdMins );
	BINDPARAM_LONG( query, m_ThresholdFreq );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_NoAlertIfRepliedTo );
	BINDPARAM_LONG( query, m_AlertOnTypeID );
	BINDPARAM_LONG( query, m_AlertOnID );
	BINDPARAM_LONG( query, m_AlertToTypeID );
	BINDPARAM_LONG( query, m_AlertToID );
	BINDPARAM_LONG( query, m_AlertMethodID );
	BINDPARAM_TCHAR( query, m_EmailAddress );
	BINDPARAM_TCHAR( query, m_FromEmailAddress );
	BINDPARAM_TIME( query, m_DateEdited );
	BINDPARAM_LONG( query, m_EditedByID );
	BINDPARAM_LONG( query, m_AgeAlertID );
	query.Execute( _T("UPDATE AgeAlerts ")
	               _T("SET Description=?,HitCount=?,ThresholdMins=?,ThresholdFreq=?,IsEnabled=?,NoAlertIfRepliedTo=?,AlertOnTypeID=?,AlertOnID=?,AlertToTypeID=?,AlertToID=?,AlertMethodID=?,EmailAddress=?,FromEmailAddress=?,DateEdited=?,EditedByID=? ")
	               _T("WHERE AgeAlertID=?") );
	return query.GetRowCount();
}

int TAgeAlerts::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgeAlertID );
	query.Execute( _T("DELETE FROM AgeAlerts ")
	               _T("WHERE AgeAlertID=?") );
	return query.GetRowCount();
}

int TAgeAlerts::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_ThresholdMins );
	BINDCOL_LONG( query, m_ThresholdFreq );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_NoAlertIfRepliedTo );
	BINDCOL_LONG( query, m_AlertOnTypeID );
	BINDCOL_LONG( query, m_AlertOnID );
	BINDCOL_LONG( query, m_AlertToTypeID );
	BINDCOL_LONG( query, m_AlertToID );
	BINDCOL_LONG( query, m_AlertMethodID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_TCHAR( query, m_FromEmailAddress );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_LONG( query, m_EditedByID );
	BINDPARAM_LONG( query, m_AgeAlertID );
	query.Execute( _T("SELECT Description,HitCount,ThresholdMins,ThresholdFreq,IsEnabled,NoAlertIfRepliedTo,AlertOnTypeID,AlertOnID,AlertToTypeID,AlertToID,AlertMethodID,EmailAddress,FromEmailAddress,DateCreated,DateEdited,CreatedByID,EditedByID ")
	               _T("FROM AgeAlerts ")
	               _T("WHERE AgeAlertID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAgeAlerts::GetLongData( CODBCQuery& query )
{
}

void TAgeAlerts::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AgeAlertID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_ThresholdMins );
	BINDCOL_LONG( query, m_ThresholdFreq );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_NoAlertIfRepliedTo );
	BINDCOL_LONG( query, m_AlertOnTypeID );
	BINDCOL_LONG( query, m_AlertOnID );
	BINDCOL_LONG( query, m_AlertToTypeID );
	BINDCOL_LONG( query, m_AlertToID );
	BINDCOL_LONG( query, m_AlertMethodID );
	BINDCOL_TCHAR( query, m_EmailAddress );
	BINDCOL_TCHAR( query, m_FromEmailAddress );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TIME( query, m_DateEdited );
	BINDCOL_LONG( query, m_CreatedByID );
	BINDCOL_LONG( query, m_EditedByID );
	query.Execute( _T("SELECT AgeAlertID,Description,HitCount,ThresholdMins,ThresholdFreq,IsEnabled,NoAlertIfRepliedTo,AlertOnTypeID,AlertOnID,AlertToTypeID,AlertToID,AlertMethodID,EmailAddress,FromEmailAddress,DateCreated,DateEdited,CreatedByID,EditedByID ")
	               _T("FROM AgeAlerts ")
	               _T("ORDER BY Description") );
}


/* class TAgeAlertsSent - 
		
	 */
void TAgeAlertsSent::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgeAlertID );
	BINDPARAM_LONG( query, m_TicketID );
	query.Execute( _T("INSERT INTO AgeAlertsSent ")
	               _T("(AgeAlertID,TicketID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_AgeAlertSentID = query.GetLastInsertedID();
}

int TAgeAlertsSent::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgeAlertID );
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_AgeAlertSentID );
	query.Execute( _T("UPDATE AgeAlertsSent ")
	               _T("SET AgeAlertID=?,TicketID=? ")
	               _T("WHERE AgeAlertSentID=?") );
	return query.GetRowCount();
}

int TAgeAlertsSent::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgeAlertSentID );
	query.Execute( _T("DELETE FROM AgeAlertsSent ")
	               _T("WHERE AgeAlertSentID=?") );
	return query.GetRowCount();
}

int TAgeAlertsSent::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgeAlertID );
	BINDCOL_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_AgeAlertSentID );
	query.Execute( _T("SELECT AgeAlertID,TicketID ")
	               _T("FROM AgeAlertsSent ")
	               _T("WHERE AgeAlertSentID=?") );
	nResult = query.Fetch();
	return nResult;
}

void TAgeAlertsSent::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AgeAlertID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_AgeAlertSentID );
	query.Execute( _T("SELECT AgeAlertID,TicketID,AgeAlertSentID ")
	               _T("FROM AgeAlertsSent ")
	               _T("ORDER BY AgeAlertID") );
}

/* class TPriorities - 
		Priority levels assignable to tickets and messages.
		Examples include: low, medium-low, medium, medium-high, high.
	 */
void TPriorities::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_LONG( query, m_SortIndex );
	query.Execute( _T("INSERT INTO Priorities ")
	               _T("(Name,SortIndex) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_PriorityID = query.GetLastInsertedID();
}

int TPriorities::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_LONG( query, m_SortIndex );
	BINDPARAM_LONG( query, m_PriorityID );
	query.Execute( _T("UPDATE Priorities ")
	               _T("SET Name=?,SortIndex=? ")
	               _T("WHERE PriorityID=?") );
	return query.GetRowCount();
}

int TPriorities::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_PriorityID );
	query.Execute( _T("DELETE FROM Priorities ")
	               _T("WHERE PriorityID=?") );
	return query.GetRowCount();
}

int TPriorities::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_LONG( query, m_SortIndex );
	BINDPARAM_LONG( query, m_PriorityID );
	query.Execute( _T("SELECT Name,SortIndex ")
	               _T("FROM Priorities ")
	               _T("WHERE PriorityID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TPriorities::GetLongData( CODBCQuery& query )
{
}

void TPriorities::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_LONG( query, m_SortIndex );
	query.Execute( _T("SELECT PriorityID,Name,SortIndex ")
	               _T("FROM Priorities ")
	               _T("ORDER BY PriorityID") );
}

/* class TRoutingRules - 
		Message routing rules.
	 */
void TRoutingRules::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_RuleDescrip );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_OrderIndex );
	BINDPARAM_BIT( query, m_AutoReplyEnable );
	BINDPARAM_LONG( query, m_AutoReplyWithStdResponse );
	BINDPARAM_BIT( query, m_AutoReplyQuoteMsg );
	BINDPARAM_BIT( query, m_AutoReplyCloseTicket );
	BINDPARAM_BIT( query, m_ForwardEnable );
	BINDPARAM_LONG( query, m_ForwardFromAgent );
	BINDPARAM_BIT( query, m_ForwardFromContact );
	BINDPARAM_LONG( query, m_AssignToTicketBox );
	BINDPARAM_BIT( query, m_AssignToAgentEnable );
	BINDPARAM_LONG( query, m_AssignToAgent );
	BINDPARAM_BIT( query, m_DeleteImmediatelyEnable );
	BINDPARAM_TINYINT( query, m_DeleteImmediately );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_LONG( query, m_PriorityID );
	BINDPARAM_LONG( query, m_MessageSourceTypeID );
	BINDPARAM_TCHAR( query, m_AutoReplyFrom );
	BINDPARAM_LONG( query, m_AssignToAgentAlg );
	BINDPARAM_LONG( query, m_AssignToTicketBoxAlg );
	BINDPARAM_BIT( query, m_AssignUniqueTicketID);
	BINDPARAM_LONG( query, m_AssignToTicketCategory);
	BINDPARAM_BIT( query, m_AlertEnable );
	BINDPARAM_BIT( query, m_AlertIncludeSubject );
	BINDPARAM_LONG( query, m_AlertToAgentID );
	BINDPARAM_TCHAR( query, m_AlertText );
	BINDPARAM_BIT( query, m_ToOrFrom );
	BINDPARAM_BIT( query, m_ConsiderAllOwned );
	BINDPARAM_BIT( query, m_DoProcessingRules );
	BINDPARAM_LONG( query, m_LastOwnerID );
	BINDPARAM_LONG( query, m_MatchMethod );
	BINDPARAM_TCHAR( query, m_ForwardFromEmail );
	BINDPARAM_TCHAR( query, m_ForwardFromName );
	BINDPARAM_BIT( query, m_ForwardInTicket );
	BINDPARAM_BIT( query, m_AutoReplyInTicket );
	BINDPARAM_BIT( query, m_AllowRemoteReply );
	BINDPARAM_LONG( query, m_DoNotAssign );
	BINDPARAM_BIT( query, m_QuoteOriginal );
	BINDPARAM_LONG( query, m_MultiMail );
	BINDPARAM_LONG( query, m_SetOpenOwner );
	BINDPARAM_LONG( query, m_OfficeHours );
	BINDPARAM_LONG( query, m_IgnoreTracking );
	query.Execute( _T("INSERT INTO RoutingRules ")
	               _T("(RuleDescrip,IsEnabled,OrderIndex,AutoReplyEnable,AutoReplyWithStdResponse,AutoReplyQuoteMsg,AutoReplyCloseTicket,ForwardEnable,ForwardFromAgent,ForwardFromContact,AssignToTicketBox,AssignToAgentEnable,AssignToAgent,DeleteImmediatelyEnable,DeleteImmediately,HitCount,PriorityID,MessageSourceTypeID,AutoReplyFrom,AssignToAgentAlg,AssignToTicketBoxAlg,AssignUniqueTicketID,AssignToTicketCategory,AlertEnable,AlertIncludeSubject,AlertToAgentID,AlertText,ToOrFrom,ConsiderAllOwned,DoProcessingRules,LastOwnerID,MatchMethod,ForwardFromEmail,ForwardFromName,ForwardInTicket,AutoReplyInTicket,AllowRemoteReply,DoNotAssign,QuoteOriginal,MultiMail,SetOpenOwner,OfficeHours,IgnoreTracking) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_RoutingRuleID = query.GetLastInsertedID();
}

int TRoutingRules::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_RuleDescrip );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_OrderIndex );
	BINDPARAM_BIT( query, m_AutoReplyEnable );
	BINDPARAM_LONG( query, m_AutoReplyWithStdResponse );
	BINDPARAM_BIT( query, m_AutoReplyQuoteMsg );
	BINDPARAM_BIT( query, m_AutoReplyCloseTicket );
	BINDPARAM_BIT( query, m_ForwardEnable );
	BINDPARAM_LONG( query, m_ForwardFromAgent );
	BINDPARAM_BIT( query, m_ForwardFromContact );
	BINDPARAM_LONG( query, m_AssignToTicketBox );
	BINDPARAM_BIT( query, m_AssignToAgentEnable );
	BINDPARAM_LONG( query, m_AssignToAgent );
	BINDPARAM_BIT( query, m_DeleteImmediatelyEnable );
	BINDPARAM_TINYINT( query, m_DeleteImmediately );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_LONG( query, m_PriorityID );
	BINDPARAM_LONG( query, m_MessageSourceTypeID );
	BINDPARAM_TCHAR( query, m_AutoReplyFrom );
	BINDPARAM_LONG( query, m_AssignToAgentAlg );
	BINDPARAM_LONG( query, m_AssignToTicketBoxAlg );
	BINDPARAM_BIT( query,  m_AssignUniqueTicketID );
	BINDPARAM_LONG( query, m_AssignToTicketCategory);
	BINDPARAM_BIT( query, m_AlertEnable );
	BINDPARAM_BIT( query, m_AlertIncludeSubject );
	BINDPARAM_LONG( query, m_AlertToAgentID );
	BINDPARAM_TCHAR( query, m_AlertText );
	BINDPARAM_BIT( query, m_ToOrFrom );
	BINDPARAM_BIT( query, m_ConsiderAllOwned );
	BINDPARAM_BIT( query, m_DoProcessingRules );
	BINDPARAM_LONG( query, m_LastOwnerID );
	BINDPARAM_LONG( query, m_MatchMethod );
	BINDPARAM_TCHAR( query, m_ForwardFromEmail );
	BINDPARAM_TCHAR( query, m_ForwardFromName );
	BINDPARAM_BIT( query, m_ForwardInTicket );
	BINDPARAM_BIT( query, m_AutoReplyInTicket );
	BINDPARAM_BIT( query, m_AllowRemoteReply );
	BINDPARAM_LONG( query, m_DoNotAssign );
	BINDPARAM_BIT( query, m_QuoteOriginal );
	BINDPARAM_LONG( query, m_MultiMail );
	BINDPARAM_LONG( query, m_SetOpenOwner );
	BINDPARAM_LONG( query, m_OfficeHours );
	BINDPARAM_LONG( query, m_IgnoreTracking );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	query.Execute( _T("UPDATE RoutingRules ")
	               _T("SET RuleDescrip=?,IsEnabled=?,OrderIndex=?,AutoReplyEnable=?,AutoReplyWithStdResponse=?,AutoReplyQuoteMsg=?,AutoReplyCloseTicket=?,ForwardEnable=?,ForwardFromAgent=?,ForwardFromContact=?,AssignToTicketBox=?,AssignToAgentEnable=?,AssignToAgent=?,DeleteImmediatelyEnable=?,DeleteImmediately=?,HitCount=?,PriorityID=?,MessageSourceTypeID=?,AutoReplyFrom=?,AssignToAgentAlg=?,AssignToTicketBoxAlg=?,AssignUniqueTicketID=?,AssignToTicketCategory=?,AlertEnable=?,AlertIncludeSubject=?,AlertToAgentID=?,AlertText=?,ToOrFrom=?,ConsiderAllOwned=?,DoProcessingRules=?,LastOwnerID=?,MatchMethod=?,ForwardFromEmail=?,ForwardFromName=?,ForwardInTicket=?,AutoReplyInTicket=?,AllowRemoteReply=?,DoNotAssign=?,QuoteOriginal=?,MultiMail=?,SetOpenOwner=?,OfficeHours=?,IgnoreTracking=? ")
	               _T("WHERE RoutingRuleID=?") );
	return query.GetRowCount();
}

int TRoutingRules::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_RoutingRuleID );
	query.Execute( _T("DELETE FROM RoutingRules ")
	               _T("WHERE RoutingRuleID=?") );
	return query.GetRowCount();
}

int TRoutingRules::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_RuleDescrip );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_OrderIndex );
	BINDCOL_BIT( query, m_AutoReplyEnable );
	BINDCOL_LONG( query, m_AutoReplyWithStdResponse );
	BINDCOL_BIT( query, m_AutoReplyQuoteMsg );
	BINDCOL_BIT( query, m_AutoReplyCloseTicket );
	BINDCOL_BIT( query, m_ForwardEnable );
	BINDCOL_LONG( query, m_ForwardFromAgent );
	BINDCOL_BIT( query, m_ForwardFromContact );
	BINDCOL_LONG( query, m_AssignToTicketBox );
	BINDCOL_BIT( query, m_AssignToAgentEnable );
	BINDCOL_LONG( query, m_AssignToAgent );
	BINDCOL_BIT( query, m_DeleteImmediatelyEnable );
	BINDCOL_TINYINT( query, m_DeleteImmediately );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_LONG( query, m_MessageSourceTypeID );
	BINDCOL_TCHAR( query, m_AutoReplyFrom );
	BINDCOL_LONG( query, m_AssignToAgentAlg );
	BINDCOL_LONG( query, m_AssignToTicketBoxAlg );
	BINDCOL_BIT( query,  m_AssignUniqueTicketID );
	BINDCOL_LONG( query, m_AssignToTicketCategory);
	BINDCOL_BIT( query, m_AlertEnable );
	BINDCOL_BIT( query, m_AlertIncludeSubject );
	BINDCOL_LONG( query, m_AlertToAgentID );
	BINDCOL_TCHAR( query, m_AlertText );	
	BINDCOL_BIT( query,  m_ToOrFrom );
	BINDCOL_BIT( query,  m_ConsiderAllOwned );
	BINDCOL_BIT( query,  m_DoProcessingRules );
	BINDCOL_LONG( query, m_LastOwnerID );
	BINDCOL_LONG( query, m_MatchMethod );
	BINDCOL_TCHAR( query, m_ForwardFromEmail );
	BINDCOL_TCHAR( query, m_ForwardFromName );
	BINDCOL_BIT( query, m_ForwardInTicket );
	BINDCOL_BIT( query, m_AutoReplyInTicket );
	BINDCOL_BIT( query, m_AllowRemoteReply );
	BINDCOL_LONG( query, m_DoNotAssign );
	BINDCOL_BIT( query, m_QuoteOriginal );
	BINDCOL_LONG( query, m_MultiMail );
	BINDCOL_LONG( query, m_SetOpenOwner );
	BINDCOL_LONG( query, m_OfficeHours );
	BINDCOL_LONG( query, m_IgnoreTracking );
	BINDPARAM_LONG( query, m_RoutingRuleID );
	query.Execute( _T("SELECT RuleDescrip,IsEnabled,OrderIndex,AutoReplyEnable,AutoReplyWithStdResponse,AutoReplyQuoteMsg,AutoReplyCloseTicket,ForwardEnable,ForwardFromAgent,ForwardFromContact,AssignToTicketBox,AssignToAgentEnable,AssignToAgent,DeleteImmediatelyEnable,DeleteImmediately,HitCount,PriorityID,MessageSourceTypeID,AutoReplyFrom,AssignToAgentAlg,AssignToTicketBoxAlg,AssignUniqueTicketID,AssignToTicketCategory,AlertEnable,AlertIncludeSubject,AlertToAgentID,AlertText,ToOrFrom,ConsiderAllOwned,DoProcessingRules,LastOwnerID,MatchMethod,ForwardFromEmail,ForwardFromName,ForwardInTicket,AutoReplyInTicket,AllowRemoteReply,DoNotAssign,QuoteOriginal,MultiMail,SetOpenOwner,OfficeHours,IgnoreTracking ")
	               _T("FROM RoutingRules ")
	               _T("WHERE RoutingRuleID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TRoutingRules::GetLongData( CODBCQuery& query )
{
}

void TRoutingRules::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDCOL_TCHAR( query, m_RuleDescrip );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_OrderIndex );
	BINDCOL_BIT( query, m_AutoReplyEnable );
	BINDCOL_LONG( query, m_AutoReplyWithStdResponse );
	BINDCOL_BIT( query, m_AutoReplyQuoteMsg );
	BINDCOL_BIT( query, m_AutoReplyCloseTicket );
	BINDCOL_BIT( query, m_ForwardEnable );
	BINDCOL_LONG( query, m_ForwardFromAgent );
	BINDCOL_BIT( query, m_ForwardFromContact );
	BINDCOL_LONG( query, m_AssignToTicketBox );
	BINDCOL_BIT( query, m_AssignToAgentEnable );
	BINDCOL_LONG( query, m_AssignToAgent );
	BINDCOL_BIT( query, m_DeleteImmediatelyEnable );
	BINDCOL_TINYINT( query, m_DeleteImmediately );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_LONG( query, m_MessageSourceTypeID );
	BINDCOL_TCHAR( query, m_AutoReplyFrom );
	BINDCOL_LONG( query, m_AssignToAgentAlg );
	BINDCOL_LONG( query, m_AssignToTicketBoxAlg );
	BINDCOL_BIT( query,  m_AssignUniqueTicketID );
	BINDCOL_LONG( query, m_AssignToTicketCategory);
	BINDCOL_BIT( query, m_AlertEnable );
	BINDCOL_BIT( query, m_AlertIncludeSubject );
	BINDCOL_LONG( query, m_AlertToAgentID );
	BINDCOL_TCHAR( query, m_AlertText );	
	BINDCOL_BIT( query,  m_ToOrFrom );
	BINDCOL_BIT( query,  m_ConsiderAllOwned );
	BINDCOL_BIT( query,  m_DoProcessingRules );
	BINDCOL_LONG( query, m_LastOwnerID );
	BINDCOL_LONG( query, m_MatchMethod );
	BINDCOL_TCHAR( query, m_ForwardFromEmail );
	BINDCOL_TCHAR( query, m_ForwardFromName );
	BINDCOL_BIT( query, m_ForwardInTicket );
	BINDCOL_BIT( query, m_AutoReplyInTicket );
	BINDCOL_BIT( query, m_AllowRemoteReply );
	BINDCOL_LONG( query, m_DoNotAssign );
	BINDCOL_BIT( query, m_QuoteOriginal );
	BINDCOL_LONG( query, m_MultiMail );
	BINDCOL_LONG( query, m_SetOpenOwner );
	BINDCOL_LONG( query, m_OfficeHours );
	BINDCOL_LONG( query, m_IgnoreTracking );
	query.Execute( _T("SELECT RoutingRuleID,RuleDescrip,IsEnabled,OrderIndex,AutoReplyEnable,AutoReplyWithStdResponse,AutoReplyQuoteMsg,AutoReplyCloseTicket,ForwardEnable,ForwardFromAgent,ForwardFromContact,AssignToTicketBox,AssignToAgentEnable,AssignToAgent,DeleteImmediatelyEnable,DeleteImmediately,HitCount,PriorityID,MessageSourceTypeID,AutoReplyFrom,AssignToAgentAlg,AssignToTicketBoxAlg,AssignUniqueTicketID,AssignToTicketCategory,AlertEnable,AlertIncludeSubject,AlertToAgentID,AlertText,ToOrFrom,ConsiderAllOwned,DoProcessingRules,LastOwnerID,MatchMethod,ForwardFromEmail,ForwardFromName,ForwardInTicket,AutoReplyInTicket,AllowRemoteReply,DoNotAssign,QuoteOriginal,MultiMail,SetOpenOwner,OfficeHours,IgnoreTracking ")
	               _T("FROM RoutingRules ")
	               _T("ORDER BY RoutingRuleID") );
}

/* class TProcessingRules - 
		Message processing rules.
	 */
void TProcessingRules::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_RuleDescrip );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_OrderIndex );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_BIT( query, m_PrePost );
	BINDPARAM_LONG( query, m_ActionType );
	BINDPARAM_LONG( query, m_ActionID );
	query.Execute( _T("INSERT INTO ProcessingRules ")
	               _T("(RuleDescrip,IsEnabled,OrderIndex,HitCount,PrePost,ActionType,ActionID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?)") );
	m_ProcessingRuleID = query.GetLastInsertedID();
}

int TProcessingRules::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_RuleDescrip );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_OrderIndex );
	BINDPARAM_LONG( query, m_HitCount );
	BINDPARAM_BIT( query, m_PrePost );
	BINDPARAM_LONG( query, m_ActionType );
	BINDPARAM_LONG( query, m_ActionID );
	BINDPARAM_LONG( query, m_ProcessingRuleID );
	query.Execute( _T("UPDATE ProcessingRules ")
	               _T("SET RuleDescrip=?,IsEnabled=?,OrderIndex=?,HitCount=?,PrePost=?,ActionType=?,ActionID=? ")
	               _T("WHERE ProcessingRuleID=?") );
	return query.GetRowCount();
}

int TProcessingRules::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ProcessingRuleID );
	query.Execute( _T("DELETE FROM ProcessingRules ")
	               _T("WHERE ProcessingRuleID=?") );
	return query.GetRowCount();
}

int TProcessingRules::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_RuleDescrip );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_OrderIndex );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_BIT( query, m_PrePost );
	BINDCOL_LONG( query, m_ActionType );
	BINDCOL_LONG( query, m_ActionID );
	BINDPARAM_LONG( query, m_ProcessingRuleID );
	query.Execute( _T("SELECT RuleDescrip,IsEnabled,OrderIndex,HitCount,PrePost,ActionType,ActionID ")
	               _T("FROM ProcessingRules ")
	               _T("WHERE ProcessingRuleID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TProcessingRules::GetLongData( CODBCQuery& query )
{
}

void TProcessingRules::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ProcessingRuleID );
	BINDCOL_TCHAR( query, m_RuleDescrip );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_LONG( query, m_OrderIndex );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_BIT( query, m_PrePost );
	BINDCOL_LONG( query, m_ActionType );
	BINDCOL_LONG( query, m_ActionID );
	query.Execute( _T("SELECT ProcessingRuleID,RuleDescrip,IsEnabled,OrderIndex,HitCount,PrePost,ActionType,ActionID ")
	               _T("FROM ProcessingRules ")
	               _T("ORDER BY ProcessingRuleID") );
}

/* class TDateFilters - 
		Date filters.
	 */
void TDateFilters::Insert( CODBCQuery& query )
{
	tstring sTemp(m_RegEx);
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_WhereToCheck );
	BINDPARAM_TEXT_STRING( query, sTemp );
	BINDPARAM_TCHAR( query, m_Header );
	BINDPARAM_LONG( query, m_HeaderValueTypeID );
	BINDPARAM_TCHAR( query, m_HeaderValue );
	query.Execute( _T("INSERT INTO DateFilters ")
	               _T("(Description,WhereToCheck,RegEx,Header,HeaderValueTypeID,HeaderValue) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?)") );
	m_DateFilterID = query.GetLastInsertedID();
}

int TDateFilters::Update( CODBCQuery& query )
{
	tstring sTemp(m_RegEx);
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_WhereToCheck );
	BINDPARAM_TEXT_STRING( query, sTemp );
	BINDPARAM_TCHAR( query, m_Header );
	BINDPARAM_LONG( query, m_HeaderValueTypeID );
	BINDPARAM_TCHAR( query, m_HeaderValue );
	BINDPARAM_LONG( query, m_DateFilterID );
	query.Execute( _T("UPDATE DateFilters ")
	               _T("SET Description=?,WhereToCheck=?,RegEx=?,Header=?,HeaderValueTypeID=?,HeaderValue=? ")
	               _T("WHERE DateFilterID=?") );
	return query.GetRowCount();
}

int TDateFilters::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_DateFilterID );
	query.Execute( _T("DELETE FROM DateFilters ")
	               _T("WHERE DateFilterID=?") );
	return query.GetRowCount();
}

int TDateFilters::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_WhereToCheck );
	BINDCOL_TCHAR( query, m_RegEx );
	BINDCOL_TCHAR( query, m_Header );
	BINDCOL_LONG( query, m_HeaderValueTypeID );
	BINDCOL_TCHAR( query, m_HeaderValue );
	BINDPARAM_LONG( query, m_DateFilterID );
	query.Execute( _T("SELECT Description,WhereToCheck,RegEx,Header,HeaderValueTypeID,HeaderValue ")
	               _T("FROM DateFilters ")
	               _T("WHERE DateFilterID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TDateFilters::GetLongData( CODBCQuery& query )
{
}

void TDateFilters::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_DateFilterID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_WhereToCheck );
	BINDCOL_TCHAR( query, m_RegEx );
	BINDCOL_TCHAR( query, m_Header );
	BINDCOL_LONG( query, m_HeaderValueTypeID );
	BINDCOL_TCHAR( query, m_HeaderValue );
	query.Execute( _T("SELECT DateFilterID,Description,WhereToCheck,RegEx,Header,HeaderValueTypeID,HeaderValue ")
	               _T("FROM DateFilters ")
	               _T("ORDER BY Description") );
}

/* class TServerParameters - 
		Defines global server parameters.
	 */
void TServerParameters::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_DataValue );
	query.Execute( _T("INSERT INTO ServerParameters ")
	               _T("(Description,DataValue) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_ServerParameterID = query.GetLastInsertedID();
}

int TServerParameters::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_DataValue );
	BINDPARAM_LONG( query, m_ServerParameterID );
	query.Execute( _T("UPDATE ServerParameters ")
	               _T("SET Description=?,DataValue=? ")
	               _T("WHERE ServerParameterID=?") );
	return query.GetRowCount();
}

int TServerParameters::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ServerParameterID );
	query.Execute( _T("DELETE FROM ServerParameters ")
	               _T("WHERE ServerParameterID=?") );
	return query.GetRowCount();
}

int TServerParameters::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_DataValue );
	BINDPARAM_LONG( query, m_ServerParameterID );
	query.Execute( _T("SELECT Description,DataValue ")
	               _T("FROM ServerParameters ")
	               _T("WHERE ServerParameterID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TServerParameters::GetLongData( CODBCQuery& query )
{
}

void TServerParameters::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ServerParameterID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_DataValue );
	query.Execute( _T("SELECT ServerParameterID,Description,DataValue ")
	               _T("FROM ServerParameters ")
	               _T("ORDER BY ServerParameterID") );
}

/* class TSignatures - 
		Signatures used by groups and agents.
	 */
void TSignatures::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_GroupID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_ObjectTypeID );
	BINDPARAM_LONG( query, m_ActualID );
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_TEXT( query, m_Signature );
	query.Execute( _T("INSERT INTO Signatures ")
	               _T("(GroupID,AgentID,ObjectTypeID,ActualID,Name,Signature) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?)") );
	m_SignatureID = query.GetLastInsertedID();
}

int TSignatures::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_GroupID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_ObjectTypeID );
	BINDPARAM_LONG( query, m_ActualID );
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_TEXT( query, m_Signature );
	BINDPARAM_LONG( query, m_SignatureID );
	query.Execute( _T("UPDATE Signatures ")
	               _T("SET GroupID=?,AgentID=?,ObjectTypeID=?,ActualID=?,Name=?,Signature=? ")
	               _T("WHERE SignatureID=?") );
	return query.GetRowCount();
}

int TSignatures::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_SignatureID );
	query.Execute( _T("DELETE FROM Signatures ")
	               _T("WHERE SignatureID=?") );
	return query.GetRowCount();
}

int TSignatures::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ObjectTypeID );
	BINDCOL_LONG( query, m_ActualID );
	BINDCOL_TCHAR( query, m_Name );
	BINDPARAM_LONG( query, m_SignatureID );
	query.Execute( _T("SELECT GroupID,AgentID,ObjectTypeID,ActualID,Name,Signature ")
	               _T("FROM Signatures ")
	               _T("WHERE SignatureID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TSignatures::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Signature );
}

void TSignatures::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_SignatureID );
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ObjectTypeID );
	BINDCOL_LONG( query, m_ActualID );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute( _T("SELECT SignatureID,GroupID,AgentID,ObjectTypeID,ActualID,Name,Signature ")
	               _T("FROM Signatures ")
	               _T("ORDER BY SignatureID") );
}

/* class TStandardResponseUsage - 
		Logs usage of standard responses by specific agents.
	 */
void TStandardResponseUsage::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_TIME( query, m_DateUsed );
	query.Execute( _T("INSERT INTO StandardResponseUsage ")
	               _T("(StandardResponseID,AgentID,DateUsed) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_StdResponseUsageID = query.GetLastInsertedID();
}

int TStandardResponseUsage::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_TIME( query, m_DateUsed );
	BINDPARAM_LONG( query, m_StdResponseUsageID );
	query.Execute( _T("UPDATE StandardResponseUsage ")
	               _T("SET StandardResponseID=?,AgentID=?,DateUsed=? ")
	               _T("WHERE StdResponseUsageID=?") );
	return query.GetRowCount();
}

int TStandardResponseUsage::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StdResponseUsageID );
	query.Execute( _T("DELETE FROM StandardResponseUsage ")
	               _T("WHERE StdResponseUsageID=?") );
	return query.GetRowCount();
}

int TStandardResponseUsage::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_StandardResponseID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TIME( query, m_DateUsed );
	BINDPARAM_LONG( query, m_StdResponseUsageID );
	query.Execute( _T("SELECT StandardResponseID,AgentID,DateUsed ")
	               _T("FROM StandardResponseUsage ")
	               _T("WHERE StdResponseUsageID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TStandardResponseUsage::GetLongData( CODBCQuery& query )
{
}

void TStandardResponseUsage::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_StdResponseUsageID );
	BINDCOL_LONG( query, m_StandardResponseID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TIME( query, m_DateUsed );
	query.Execute( _T("SELECT StdResponseUsageID,StandardResponseID,AgentID,DateUsed ")
	               _T("FROM StandardResponseUsage ")
	               _T("ORDER BY StdResponseUsageID") );
}

/* class TStandardResponses - 
		Standard responses
	 */
void TStandardResponses::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_LONG( query, m_StdResponseCatID );
	BINDPARAM_TEXT( query, m_StandardResponse );
	BINDPARAM_BIT( query, m_IsApproved );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_TIME( query, m_DeletedTime );
	BINDPARAM_LONG( query, m_DeletedBy );
	BINDPARAM_TEXT( query, m_Note );
	BINDPARAM_TIME( query, m_DateModified );
	BINDPARAM_LONG( query, m_ModifiedBy );
	BINDPARAM_BIT( query, m_UseKeywords );
	BINDPARAM_TCHAR( query, m_Keywords );
	query.Execute( _T("INSERT INTO StandardResponses ")
	               _T("(Subject,StdResponseCatID,StandardResponse,IsApproved,AgentID,DateCreated,IsDeleted,DeletedTime,DeletedBy,Note,DateModified,ModifiedBy,UseKeywords,Keywords) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_StandardResponseID = query.GetLastInsertedID();
}

int TStandardResponses::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_LONG( query, m_StdResponseCatID );
	BINDPARAM_TEXT( query, m_StandardResponse );
	BINDPARAM_BIT( query, m_IsApproved );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_TIME( query, m_DeletedTime );
	BINDPARAM_LONG( query, m_DeletedBy );
	BINDPARAM_TEXT( query, m_Note );
	BINDPARAM_TIME( query, m_DateModified );
	BINDPARAM_LONG( query, m_ModifiedBy );
	BINDPARAM_BIT( query, m_UseKeywords );
	BINDPARAM_TCHAR( query, m_Keywords );
	BINDPARAM_LONG( query, m_StandardResponseID );
	query.Execute( _T("UPDATE StandardResponses ")
	               _T("SET Subject=?,StdResponseCatID=?,StandardResponse=?,IsApproved=?,AgentID=?,IsDeleted=?,DeletedTime=?,DeletedBy=?,Note=?,DateModified=?,ModifiedBy=?,UseKeywords=?,Keywords=? ")
	               _T("WHERE StandardResponseID=?") );
	return query.GetRowCount();
}

int TStandardResponses::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StandardResponseID );
	query.Execute( _T("DELETE FROM StandardResponses ")
	               _T("WHERE StandardResponseID=?") );
	return query.GetRowCount();
}

int TStandardResponses::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Subject );
	BINDCOL_LONG( query, m_StdResponseCatID );
	BINDCOL_BIT( query, m_IsApproved );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_TIME( query, m_DeletedTime );
	BINDCOL_LONG( query, m_DeletedBy );
	BINDCOL_TIME( query, m_DateModified );
	BINDCOL_LONG( query, m_ModifiedBy );
	BINDCOL_BIT( query, m_UseKeywords );
	BINDCOL_TCHAR( query, m_Keywords );

	BINDPARAM_LONG( query, m_StandardResponseID );
	query.Execute( _T("SELECT Subject,StdResponseCatID,IsApproved,AgentID,DateCreated,IsDeleted,DeletedTime,DeletedBy,DateModified,ModifiedBy,UseKeywords,Keywords,StandardResponse,Note ")
	               _T("FROM StandardResponses ")
	               _T("WHERE StandardResponseID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TStandardResponses::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_StandardResponse );
	GETDATA_TEXT( query, m_Note );
}

void TStandardResponses::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_StandardResponseID );
	BINDCOL_TCHAR( query, m_Subject );
	BINDCOL_LONG( query, m_StdResponseCatID );
	BINDCOL_BIT( query, m_IsApproved );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_TIME( query, m_DeletedTime );
	BINDCOL_LONG( query, m_DeletedBy );
	BINDCOL_TIME( query, m_DateModified );
	BINDCOL_LONG( query, m_ModifiedBy );
	BINDCOL_BIT( query, m_UseKeywords );
	BINDCOL_TCHAR( query, m_Keywords );
	query.Execute( _T("SELECT StandardResponseID,Subject,StdResponseCatID,IsApproved,AgentID,DateCreated,IsDeleted,DeletedTime,DeletedBy,DateModified,ModifiedBy,UseKeywords,Keywords,StandardResponse,Note ")
	               _T("FROM StandardResponses ")
	               _T("ORDER BY StandardResponseID") );
}

/* class TStdRespApprovalQueue - 
		Queues the standard response approval requests. 
		Std responses will not become active until they have been approved by 
		a designated "approver" (as indicated by the Approvals table).
		If an approval is required on a std response edit (versus a new std response), 
		the EditsStdRespID table column is populated with the ID of the std response 
		to be replaced once the edit is approved.
	 */
void TStdRespApprovalQueue::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TIME( query, m_DateRequested );
	BINDPARAM_TIME( query, m_DateHandled );
	BINDPARAM_BIT( query, m_IsApproved );
	BINDPARAM_LONG( query, m_RequestAgentID );
	BINDPARAM_LONG( query, m_ApproverAgentID );
	BINDPARAM_TEXT( query, m_Note );
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_EditsStdRspID );
	query.Execute( _T("INSERT INTO StdRespApprovalQueue ")
	               _T("(DateRequested,DateHandled,IsApproved,RequestAgentID,ApproverAgentID,Note,StandardResponseID,EditsStdRspID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?)") );
	m_StdRespApprovalQueueID = query.GetLastInsertedID();
}

int TStdRespApprovalQueue::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TIME( query, m_DateRequested );
	BINDPARAM_TIME( query, m_DateHandled );
	BINDPARAM_BIT( query, m_IsApproved );
	BINDPARAM_LONG( query, m_RequestAgentID );
	BINDPARAM_LONG( query, m_ApproverAgentID );
	BINDPARAM_TEXT( query, m_Note );
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_EditsStdRspID );
	BINDPARAM_LONG( query, m_StdRespApprovalQueueID );
	query.Execute( _T("UPDATE StdRespApprovalQueue ")
	               _T("SET DateRequested=?,DateHandled=?,IsApproved=?,RequestAgentID=?,ApproverAgentID=?,Note=?,StandardResponseID=?,EditsStdRspID=? ")
	               _T("WHERE StdRespApprovalQueueID=?") );
	return query.GetRowCount();
}

int TStdRespApprovalQueue::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StdRespApprovalQueueID );
	query.Execute( _T("DELETE FROM StdRespApprovalQueue ")
	               _T("WHERE StdRespApprovalQueueID=?") );
	return query.GetRowCount();
}

int TStdRespApprovalQueue::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TIME( query, m_DateRequested );
	BINDCOL_TIME( query, m_DateHandled );
	BINDCOL_BIT( query, m_IsApproved );
	BINDCOL_LONG( query, m_RequestAgentID );
	BINDCOL_LONG( query, m_ApproverAgentID );
	BINDCOL_LONG( query, m_StandardResponseID );
	BINDCOL_LONG( query, m_EditsStdRspID );
	BINDPARAM_LONG( query, m_StdRespApprovalQueueID );
	query.Execute( _T("SELECT DateRequested,DateHandled,IsApproved,RequestAgentID,ApproverAgentID,StandardResponseID,EditsStdRspID,Note ")
	               _T("FROM StdRespApprovalQueue ")
	               _T("WHERE StdRespApprovalQueueID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TStdRespApprovalQueue::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Note );
}

void TStdRespApprovalQueue::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_StdRespApprovalQueueID );
	BINDCOL_TIME( query, m_DateRequested );
	BINDCOL_TIME( query, m_DateHandled );
	BINDCOL_BIT( query, m_IsApproved );
	BINDCOL_LONG( query, m_RequestAgentID );
	BINDCOL_LONG( query, m_ApproverAgentID );
	BINDCOL_LONG( query, m_StandardResponseID );
	BINDCOL_LONG( query, m_EditsStdRspID );
	query.Execute( _T("SELECT StdRespApprovalQueueID,DateRequested,DateHandled,IsApproved,RequestAgentID,ApproverAgentID,StandardResponseID,EditsStdRspID,Note ")
	               _T("FROM StdRespApprovalQueue ")
	               _T("ORDER BY StdRespApprovalQueueID") );
}

/* class TStdResponseAttachments - 
		Allows attachments to relate directly to statndard responses.
	 */
void TStdResponseAttachments::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_AttachmentID );
	query.Execute( _T("INSERT INTO StdResponseAttachments ")
	               _T("(StandardResponseID,AttachmentID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_StdResponseAttachID = query.GetLastInsertedID();
}

int TStdResponseAttachments::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_AttachmentID );
	BINDPARAM_LONG( query, m_StdResponseAttachID );
	query.Execute( _T("UPDATE StdResponseAttachments ")
	               _T("SET StandardResponseID=?,AttachmentID=? ")
	               _T("WHERE StdResponseAttachID=?") );
	return query.GetRowCount();
}

int TStdResponseAttachments::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StdResponseAttachID );
	query.Execute( _T("DELETE FROM StdResponseAttachments ")
	               _T("WHERE StdResponseAttachID=?") );
	return query.GetRowCount();
}

int TStdResponseAttachments::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_StandardResponseID );
	BINDCOL_LONG( query, m_AttachmentID );
	BINDPARAM_LONG( query, m_StdResponseAttachID );
	query.Execute( _T("SELECT StandardResponseID,AttachmentID ")
	               _T("FROM StdResponseAttachments ")
	               _T("WHERE StdResponseAttachID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TStdResponseAttachments::GetLongData( CODBCQuery& query )
{
}

void TStdResponseAttachments::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_StdResponseAttachID );
	BINDCOL_LONG( query, m_StandardResponseID );
	BINDCOL_LONG( query, m_AttachmentID );
	query.Execute( _T("SELECT StdResponseAttachID,StandardResponseID,AttachmentID ")
	               _T("FROM StdResponseAttachments ")
	               _T("ORDER BY StdResponseAttachID") );
}

/* class TStdResponseCategories - 
		Organizes the standard responses into groups.
	 */
void TStdResponseCategories::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_CategoryName );
	query.Execute( _T("INSERT INTO StdResponseCategories ")
	               _T("(CategoryName) ")
	               _T("VALUES")
	               _T("(?)") );
	m_StdResponseCatID = query.GetLastInsertedID();
}

int TStdResponseCategories::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_CategoryName );
	BINDPARAM_LONG( query, m_ObjectID );
	BINDPARAM_LONG( query, m_StdResponseCatID );
	query.Execute( _T("UPDATE StdResponseCategories ")
	               _T("SET CategoryName=?,ObjectID=? ")
	               _T("WHERE StdResponseCatID=?") );
	return query.GetRowCount();
}

int TStdResponseCategories::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StdResponseCatID );
	query.Execute( _T("DELETE FROM StdResponseCategories ")
	               _T("WHERE StdResponseCatID=?") );
	return query.GetRowCount();
}

int TStdResponseCategories::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_CategoryName );
	BINDCOL_LONG( query, m_ObjectID );
	BINDPARAM_LONG( query, m_StdResponseCatID );
	query.Execute( _T("SELECT CategoryName,ObjectID ")
	               _T("FROM StdResponseCategories ")
	               _T("WHERE StdResponseCatID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TStdResponseCategories::GetLongData( CODBCQuery& query )
{
}

void TStdResponseCategories::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_StdResponseCatID );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_TCHAR( query, m_CategoryName );
	query.Execute( _T("SELECT StdResponseCatID,ObjectID,CategoryName ")
	               _T("FROM StdResponseCategories ")
	               _T("ORDER BY StdResponseCatID") );
}

/* class TStdResponseFavorites - 
		Logs usage of standard responses by specific agents.
	 */
void TStdResponseFavorites::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_AgentID );
	query.Execute( _T("INSERT INTO StdResponseFavorites ")
	               _T("(StandardResponseID,AgentID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_StdResponseFavoritesID = query.GetLastInsertedID();
}

int TStdResponseFavorites::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StandardResponseID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_StdResponseFavoritesID );
	query.Execute( _T("UPDATE StdResponseFavorites ")
	               _T("SET StandardResponseID=?,AgentID=? ")
	               _T("WHERE StdResponseFavoritesID=?") );
	return query.GetRowCount();
}

int TStdResponseFavorites::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StdResponseFavoritesID );
	query.Execute( _T("DELETE FROM StdResponseFavorites ")
	               _T("WHERE StdResponseFavoritesID=?") );
	return query.GetRowCount();
}

int TStdResponseFavorites::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_StandardResponseID );
	BINDCOL_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_StdResponseFavoritesID );
	query.Execute( _T("SELECT StandardResponseID,AgentID ")
	               _T("FROM StdResponseFavorites ")
	               _T("WHERE StdResponseFavoritesID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TStdResponseFavorites::GetLongData( CODBCQuery& query )
{
}

void TStdResponseFavorites::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_StdResponseFavoritesID );
	BINDCOL_LONG( query, m_StandardResponseID );
	BINDCOL_LONG( query, m_AgentID );
	query.Execute( _T("SELECT StdResponseFavoritesID,StandardResponseID,AgentID ")
	               _T("FROM StdResponseFavorites ")
	               _T("ORDER BY StdResponseFavoritesID") );
}

/* class TStyleSheets - 
		The list of style sheets available.
	 */
void TStyleSheets::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_TCHAR( query, m_Filename );
	query.Execute( _T("INSERT INTO StyleSheets ")
	               _T("(Name,Filename) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_StyleSheetID = query.GetLastInsertedID();
}

int TStyleSheets::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_TCHAR( query, m_Filename );
	BINDPARAM_LONG( query, m_StyleSheetID );
	query.Execute( _T("UPDATE StyleSheets ")
	               _T("SET Name=?,Filename=? ")
	               _T("WHERE StyleSheetID=?") );
	return query.GetRowCount();
}

int TStyleSheets::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_StyleSheetID );
	query.Execute( _T("DELETE FROM StyleSheets ")
	               _T("WHERE StyleSheetID=?") );
	return query.GetRowCount();
}

int TStyleSheets::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_TCHAR( query, m_Filename );
	BINDPARAM_LONG( query, m_StyleSheetID );
	query.Execute( _T("SELECT Name,Filename ")
	               _T("FROM StyleSheets ")
	               _T("WHERE StyleSheetID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TStyleSheets::GetLongData( CODBCQuery& query )
{
}

void TStyleSheets::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_StyleSheetID );
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_TCHAR( query, m_Filename );
	query.Execute( _T("SELECT StyleSheetID,Name,Filename ")
	               _T("FROM StyleSheets ")
	               _T("ORDER BY StyleSheetID") );
}

/* class TTaskList - 
		Agent-level task list.
	 */
void TTaskList::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TaskStatusID );
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_TCHAR( query, m_TaskDescrip );
	BINDPARAM_TINYINT( query, m_PercentageComplete );
	BINDPARAM_TIME( query, m_DueDate );
	BINDPARAM_TIME( query, m_StartDate );
	BINDPARAM_TIME( query, m_Reminder );
	BINDPARAM_LONG( query, m_TaskPriorityID );
	query.Execute( _T("INSERT INTO TaskList ")
	               _T("(AgentID,TaskStatusID,Subject,TaskDescrip,PercentageComplete,DueDate,StartDate,Reminder,TaskPriorityID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?)") );
	m_TaskID = query.GetLastInsertedID();
}

int TTaskList::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TaskStatusID );
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_TCHAR( query, m_TaskDescrip );
	BINDPARAM_TINYINT( query, m_PercentageComplete );
	BINDPARAM_TIME( query, m_DueDate );
	BINDPARAM_TIME( query, m_StartDate );
	BINDPARAM_TIME( query, m_Reminder );
	BINDPARAM_LONG( query, m_TaskPriorityID );
	BINDPARAM_LONG( query, m_TaskID );
	query.Execute( _T("UPDATE TaskList ")
	               _T("SET AgentID=?,TaskStatusID=?,Subject=?,TaskDescrip=?,PercentageComplete=?,DueDate=?,StartDate=?,Reminder=?,TaskPriorityID=? ")
	               _T("WHERE TaskID=?") );
	return query.GetRowCount();
}

int TTaskList::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TaskID );
	query.Execute( _T("DELETE FROM TaskList ")
	               _T("WHERE TaskID=?") );
	return query.GetRowCount();
}

int TTaskList::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_TaskStatusID );
	BINDCOL_TCHAR( query, m_Subject );
	BINDCOL_TCHAR( query, m_TaskDescrip );
	BINDCOL_TINYINT( query, m_PercentageComplete );
	BINDCOL_TIME( query, m_DueDate );
	BINDCOL_TIME( query, m_StartDate );
	BINDCOL_TIME( query, m_Reminder );
	BINDCOL_LONG( query, m_TaskPriorityID );
	BINDPARAM_LONG( query, m_TaskID );
	query.Execute( _T("SELECT AgentID,TaskStatusID,Subject,TaskDescrip,PercentageComplete,DueDate,StartDate,Reminder,TaskPriorityID ")
	               _T("FROM TaskList ")
	               _T("WHERE TaskID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTaskList::GetLongData( CODBCQuery& query )
{
}

void TTaskList::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TaskID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_TaskStatusID );
	BINDCOL_TCHAR( query, m_Subject );
	BINDCOL_TCHAR( query, m_TaskDescrip );
	BINDCOL_TINYINT( query, m_PercentageComplete );
	BINDCOL_TIME( query, m_DueDate );
	BINDCOL_TIME( query, m_StartDate );
	BINDCOL_TIME( query, m_Reminder );
	BINDCOL_LONG( query, m_TaskPriorityID );
	query.Execute( _T("SELECT TaskID,AgentID,TaskStatusID,Subject,TaskDescrip,PercentageComplete,DueDate,StartDate,Reminder,TaskPriorityID ")
	               _T("FROM TaskList ")
	               _T("ORDER BY TaskID") );
}

/* class TTaskPriorities - 
		Priorities of tasks. Examples include: low,	normal,	and high.
	 */
void TTaskPriorities::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_PriorityName );
	BINDPARAM_BIT( query, m_IsDefault );
	query.Execute( _T("INSERT INTO TaskPriorities ")
	               _T("(PriorityName,IsDefault) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_TaskPriorityID = query.GetLastInsertedID();
}

int TTaskPriorities::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_PriorityName );
	BINDPARAM_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_TaskPriorityID );
	query.Execute( _T("UPDATE TaskPriorities ")
	               _T("SET PriorityName=?,IsDefault=? ")
	               _T("WHERE TaskPriorityID=?") );
	return query.GetRowCount();
}

int TTaskPriorities::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TaskPriorityID );
	query.Execute( _T("DELETE FROM TaskPriorities ")
	               _T("WHERE TaskPriorityID=?") );
	return query.GetRowCount();
}

int TTaskPriorities::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_PriorityName );
	BINDCOL_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_TaskPriorityID );
	query.Execute( _T("SELECT PriorityName,IsDefault ")
	               _T("FROM TaskPriorities ")
	               _T("WHERE TaskPriorityID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTaskPriorities::GetLongData( CODBCQuery& query )
{
}

void TTaskPriorities::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TaskPriorityID );
	BINDCOL_TCHAR( query, m_PriorityName );
	BINDCOL_BIT( query, m_IsDefault );
	query.Execute( _T("SELECT TaskPriorityID,PriorityName,IsDefault ")
	               _T("FROM TaskPriorities ")
	               _T("ORDER BY TaskPriorityID") );
}

/* class TTaskStatuses - 
		Status of current specific task. 
		Examples: Complete, In progress, Not started, Deferred, Waiting on someone else.
	 */
void TTaskStatuses::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_StatusDescrip );
	BINDPARAM_BIT( query, m_IsDefault );
	query.Execute( _T("INSERT INTO TaskStatuses ")
	               _T("(StatusDescrip,IsDefault) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_TaskStatusID = query.GetLastInsertedID();
}

int TTaskStatuses::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_StatusDescrip );
	BINDPARAM_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_TaskStatusID );
	query.Execute( _T("UPDATE TaskStatuses ")
	               _T("SET StatusDescrip=?,IsDefault=? ")
	               _T("WHERE TaskStatusID=?") );
	return query.GetRowCount();
}

int TTaskStatuses::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TaskStatusID );
	query.Execute( _T("DELETE FROM TaskStatuses ")
	               _T("WHERE TaskStatusID=?") );
	return query.GetRowCount();
}

int TTaskStatuses::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_StatusDescrip );
	BINDCOL_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_TaskStatusID );
	query.Execute( _T("SELECT StatusDescrip,IsDefault ")
	               _T("FROM TaskStatuses ")
	               _T("WHERE TaskStatusID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTaskStatuses::GetLongData( CODBCQuery& query )
{
}

void TTaskStatuses::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TaskStatusID );
	BINDCOL_TCHAR( query, m_StatusDescrip );
	BINDCOL_BIT( query, m_IsDefault );
	query.Execute( _T("SELECT TaskStatusID,StatusDescrip,IsDefault ")
	               _T("FROM TaskStatuses ")
	               _T("ORDER BY TaskStatusID") );
}

/* class TTicketActions - 
		Enumerates the different ticket actions stored in the TicketActions table
	 */
void TTicketActions::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO TicketActions ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_TicketActionID = query.GetLastInsertedID();
}

int TTicketActions::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_TicketActionID );
	query.Execute( _T("UPDATE TicketActions ")
	               _T("SET Description=? ")
	               _T("WHERE TicketActionID=?") );
	return query.GetRowCount();
}

int TTicketActions::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketActionID );
	query.Execute( _T("DELETE FROM TicketActions ")
	               _T("WHERE TicketActionID=?") );
	return query.GetRowCount();
}

int TTicketActions::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_TicketActionID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM TicketActions ")
	               _T("WHERE TicketActionID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketActions::GetLongData( CODBCQuery& query )
{
}

void TTicketActions::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketActionID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT TicketActionID,Description ")
	               _T("FROM TicketActions ")
	               _T("ORDER BY TicketActionID") );
}

/* class TTicketBoxFooters - 
		Footers used by ticket boxes.
	 */
void TTicketBoxFooters::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TEXT( query, m_Footer );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	query.Execute( _T("INSERT INTO TicketBoxFooters ")
	               _T("(Description,Footer,IsDeleted) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_FooterID = query.GetLastInsertedID();
}

int TTicketBoxFooters::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TEXT( query, m_Footer );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_LONG( query, m_FooterID );
	query.Execute( _T("UPDATE TicketBoxFooters ")
	               _T("SET Description=?,Footer=?,IsDeleted=? ")
	               _T("WHERE FooterID=?") );
	return query.GetRowCount();
}

int TTicketBoxFooters::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_FooterID );
	query.Execute( _T("DELETE FROM TicketBoxFooters ")
	               _T("WHERE FooterID=?") );
	return query.GetRowCount();
}

int TTicketBoxFooters::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDPARAM_LONG( query, m_FooterID );
	query.Execute( _T("SELECT Description,IsDeleted,Footer ")
	               _T("FROM TicketBoxFooters ")
	               _T("WHERE FooterID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketBoxFooters::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Footer );
}

void TTicketBoxFooters::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_FooterID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TINYINT( query, m_IsDeleted );
	query.Execute( _T("SELECT FooterID,Description,IsDeleted,Footer ")
	               _T("FROM TicketBoxFooters ")
	               _T("ORDER BY FooterID") );
}

/* class TTicketBoxHeaders - 
		Headers used by ticket boxes.
	 */
void TTicketBoxHeaders::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TEXT( query, m_Header );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	query.Execute( _T("INSERT INTO TicketBoxHeaders ")
	               _T("(Description,Header,IsDeleted) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_HeaderID = query.GetLastInsertedID();
}

int TTicketBoxHeaders::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TEXT( query, m_Header );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_LONG( query, m_HeaderID );
	query.Execute( _T("UPDATE TicketBoxHeaders ")
	               _T("SET Description=?,Header=?,IsDeleted=? ")
	               _T("WHERE HeaderID=?") );
	return query.GetRowCount();
}

int TTicketBoxHeaders::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_HeaderID );
	query.Execute( _T("DELETE FROM TicketBoxHeaders ")
	               _T("WHERE HeaderID=?") );
	return query.GetRowCount();
}

int TTicketBoxHeaders::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDPARAM_LONG( query, m_HeaderID );
	query.Execute( _T("SELECT Description,IsDeleted,Header ")
	               _T("FROM TicketBoxHeaders ")
	               _T("WHERE HeaderID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketBoxHeaders::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Header );
}

void TTicketBoxHeaders::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_HeaderID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TINYINT( query, m_IsDeleted );
	query.Execute( _T("SELECT HeaderID,Description,IsDeleted,Header ")
	               _T("FROM TicketBoxHeaders ")
	               _T("ORDER BY HeaderID") );
}

/* class TTicketBoxViewTypes - 
		The types of ticket box views applicable to specific ticket boxes.
	 */
void TTicketBoxViewTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	query.Execute( _T("INSERT INTO TicketBoxViewTypes ")
	               _T("(Name) ")
	               _T("VALUES")
	               _T("(?)") );
	m_TicketBoxViewTypeID = query.GetLastInsertedID();
}

int TTicketBoxViewTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_LONG( query, m_TicketBoxViewTypeID );
	query.Execute( _T("UPDATE TicketBoxViewTypes ")
	               _T("SET Name=? ")
	               _T("WHERE TicketBoxViewTypeID=?") );
	return query.GetRowCount();
}

int TTicketBoxViewTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxViewTypeID );
	query.Execute( _T("DELETE FROM TicketBoxViewTypes ")
	               _T("WHERE TicketBoxViewTypeID=?") );
	return query.GetRowCount();
}

int TTicketBoxViewTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Name );
	BINDPARAM_LONG( query, m_TicketBoxViewTypeID );
	query.Execute( _T("SELECT Name ")
	               _T("FROM TicketBoxViewTypes ")
	               _T("WHERE TicketBoxViewTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketBoxViewTypes::GetLongData( CODBCQuery& query )
{
}

void TTicketBoxViewTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxViewTypeID );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute( _T("SELECT TicketBoxViewTypeID,Name ")
	               _T("FROM TicketBoxViewTypes ")
	               _T("ORDER BY TicketBoxViewTypeID") );
}

/* class TTicketBoxViews - 
		Whenever a ticket box is created for (or made accessible to) an agent, a 
		TicketBoxView must be created that represents that agent's view into the ticket box.
		View-related parameters, such as sort order, sort field, and the columns to display, 
		must be maintained within this table to ensure a consistent representation of views 
		to the agent regardless of which computer he/she uses.
	 */
void TTicketBoxViews::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_AgentBoxID );
	BINDPARAM_BIT( query, m_ShowOwnedItems );
	BINDPARAM_BIT( query, m_ShowClosedItems );
	BINDPARAM_BIT( query, m_SortAscending );
	BINDPARAM_LONG( query, m_SortField );
	BINDPARAM_LONG( query, m_TicketBoxViewTypeID );
	BINDPARAM_BIT( query, m_ShowState );
	BINDPARAM_BIT( query, m_ShowPriority );
	BINDPARAM_BIT( query, m_ShowNumNotes );
	BINDPARAM_BIT( query, m_ShowTicketID );
	BINDPARAM_BIT( query, m_ShowNumMsgs );
	BINDPARAM_BIT( query, m_ShowSubject );
	BINDPARAM_BIT( query, m_ShowContact );
	BINDPARAM_BIT( query, m_ShowDate );
	BINDPARAM_BIT( query, m_ShowCategory );
	BINDPARAM_BIT( query, m_ShowOwner );
	BINDPARAM_BIT( query, m_ShowTicketBox );
	BINDPARAM_BIT( query, m_UseDefault );	
	query.Execute( _T("INSERT INTO TicketBoxViews ")
	               _T("(AgentID,TicketBoxID,AgentBoxID,ShowOwnedItems,ShowClosedItems,SortAscending,SortField,TicketBoxViewTypeID,ShowState,ShowPriority,ShowNumNotes,ShowTicketID,ShowNumMsgs,ShowSubject,ShowContact,ShowDate,ShowCategory,ShowOwner,ShowTicketBox,UseDefault) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_TicketBoxViewID = query.GetLastInsertedID();
}

int TTicketBoxViews::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_AgentBoxID );
	BINDPARAM_BIT( query, m_ShowOwnedItems );
	BINDPARAM_BIT( query, m_ShowClosedItems );
	BINDPARAM_BIT( query, m_SortAscending );
	BINDPARAM_LONG( query, m_SortField );
	BINDPARAM_LONG( query, m_TicketBoxViewTypeID );
	BINDPARAM_BIT( query, m_ShowState );
	BINDPARAM_BIT( query, m_ShowPriority );
	BINDPARAM_BIT( query, m_ShowNumNotes );
	BINDPARAM_BIT( query, m_ShowTicketID );
	BINDPARAM_BIT( query, m_ShowNumMsgs );
	BINDPARAM_BIT( query, m_ShowSubject );
	BINDPARAM_BIT( query, m_ShowContact );
	BINDPARAM_BIT( query, m_ShowDate );
	BINDPARAM_BIT( query, m_ShowCategory );
	BINDPARAM_BIT( query, m_ShowOwner );
	BINDPARAM_BIT( query, m_ShowTicketBox );
	BINDPARAM_BIT( query, m_UseDefault );	
	BINDPARAM_LONG( query, m_TicketBoxViewID );
	query.Execute( _T("UPDATE TicketBoxViews ")
	               _T("SET AgentID=?,TicketBoxID=?,AgentBoxID=?,ShowOwnedItems=?,ShowClosedItems=?,SortAscending=?,SortField=?,TicketBoxViewTypeID=?,ShowState=?,ShowPriority=?,ShowNumNotes=?,ShowTicketID=?,ShowNumMsgs=?,ShowSubject=?,ShowContact=?,ShowDate=?,ShowCategory=?,ShowOwner=?,ShowTicketBox=?,UseDefault=? ")
	               _T("WHERE TicketBoxViewID=?") );
	return query.GetRowCount();
}

int TTicketBoxViews::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxViewID );
	query.Execute( _T("DELETE FROM TicketBoxViews ")
	               _T("WHERE TicketBoxViewID=?") );
	return query.GetRowCount();
}

int TTicketBoxViews::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_AgentBoxID );
	BINDCOL_BIT( query, m_ShowOwnedItems );
	BINDCOL_BIT( query, m_ShowClosedItems );
	BINDCOL_BIT( query, m_SortAscending );
	BINDCOL_LONG( query, m_SortField );
	BINDCOL_LONG( query, m_TicketBoxViewTypeID );
	BINDCOL_BIT( query, m_ShowState );
	BINDCOL_BIT( query, m_ShowPriority );
	BINDCOL_BIT( query, m_ShowNumNotes );
	BINDCOL_BIT( query, m_ShowTicketID );
	BINDCOL_BIT( query, m_ShowNumMsgs );
	BINDCOL_BIT( query, m_ShowSubject );
	BINDCOL_BIT( query, m_ShowContact );
	BINDCOL_BIT( query, m_ShowDate );
	BINDCOL_BIT( query, m_ShowCategory );
	BINDCOL_BIT( query, m_ShowOwner );
	BINDCOL_BIT( query, m_ShowTicketBox );
	BINDCOL_BIT( query, m_UseDefault );	
	BINDPARAM_LONG( query, m_TicketBoxViewID );
	query.Execute( _T("SELECT AgentID,TicketBoxID,AgentBoxID,ShowOwnedItems,ShowClosedItems,SortAscending,SortField,TicketBoxViewTypeID,ShowState,ShowPriority,ShowNumNotes,ShowTicketID,ShowNumMsgs,ShowSubject,ShowContact,ShowDate,ShowCategory,ShowOwner,ShowTicketBox,UseDefault ")
	               _T("FROM TicketBoxViews ")
	               _T("WHERE TicketBoxViewID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketBoxViews::GetLongData( CODBCQuery& query )
{
}

void TTicketBoxViews::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxViewID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_AgentBoxID );
	BINDCOL_BIT( query, m_ShowOwnedItems );
	BINDCOL_BIT( query, m_ShowClosedItems );
	BINDCOL_BIT( query, m_SortAscending );
	BINDCOL_LONG( query, m_SortField );
	BINDCOL_LONG( query, m_TicketBoxViewTypeID );
	BINDCOL_BIT( query, m_ShowState );
	BINDCOL_BIT( query, m_ShowPriority );
	BINDCOL_BIT( query, m_ShowNumNotes );
	BINDCOL_BIT( query, m_ShowTicketID );
	BINDCOL_BIT( query, m_ShowNumMsgs );
	BINDCOL_BIT( query, m_ShowSubject );
	BINDCOL_BIT( query, m_ShowContact );
	BINDCOL_BIT( query, m_ShowDate );
	BINDCOL_BIT( query, m_ShowCategory );
	BINDCOL_BIT( query, m_ShowOwner );
	BINDCOL_BIT( query, m_ShowTicketBox );
	BINDCOL_BIT( query, m_UseDefault );		
	query.Execute( _T("SELECT TicketBoxViewID,AgentID,TicketBoxID,AgentBoxID,ShowOwnedItems,ShowClosedItems,SortAscending,SortField,TicketBoxViewTypeID,ShowState,ShowPriority,ShowNumNotes,ShowTicketID,ShowNumMsgs,ShowSubject,ShowContact,ShowDate,ShowCategory,ShowOwner,ShowTicketBox,UseDefault ")
	               _T("FROM TicketBoxViews ")
	               _T("ORDER BY TicketBoxViewID") );
}

/* class TTicketBoxes - 
		Ticket boxes.
	 */
void TTicketBoxes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_AgeAlertingEnabled );
	BINDPARAM_LONG( query, m_AgeAlertingThresholdMins );
	BINDPARAM_LONG( query, m_ObjectID );
	BINDPARAM_LONG( query, m_MaxRecordsSize );
	BINDPARAM_BIT( query, m_RequireGetOldest );
	BINDPARAM_LONG( query, m_LowWaterMark );
	BINDPARAM_LONG( query, m_HighWaterMark );
	BINDPARAM_BIT( query, m_AutoReplyEnable );
	BINDPARAM_LONG( query, m_AutoReplyWithStdResponse );
	BINDPARAM_LONG( query, m_AutoReplyThreshHoldMins );
	BINDPARAM_BIT( query, m_AutoReplyQuoteMsg );
	BINDPARAM_BIT( query, m_AutoReplyCloseTicket );
	BINDPARAM_BIT( query, m_AutoReplyInTicket );
	BINDPARAM_LONG( query, m_HeaderID );
	BINDPARAM_LONG( query, m_FooterID );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_TCHAR( query, m_DefaultEmailAddress );
	BINDPARAM_TINYINT( query, m_WaterMarkStatus );
	BINDPARAM_TCHAR( query, m_DefaultEmailAddressName );
	BINDPARAM_TINYINT( query, m_FooterLocation );	
	BINDPARAM_LONG( query, m_MessageDestinationID );
	BINDPARAM_LONG( query, m_UnreadMode );
	BINDPARAM_LONG( query, m_FromFormat );
	BINDPARAM_LONG( query, m_TicketLink );
	BINDPARAM_LONG( query, m_MultiMail );
	BINDPARAM_LONG( query, m_RequireTC );
	BINDPARAM_TCHAR( query, m_ReplyToEmailAddress );
	BINDPARAM_TCHAR( query, m_ReturnPathEmailAddress );
	query.Execute( _T("INSERT INTO TicketBoxes ")
	               _T("(Name,Description,AgeAlertingEnabled,AgeAlertingThresholdMins,ObjectID,MaxRecordsSize,RequireGetOldest,LowWaterMark,HighWaterMark,AutoReplyEnable,AutoReplyWithStdResponse,AutoReplyThreshHoldMins,AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket,HeaderID,FooterID,OwnerID,DefaultEmailAddress,WaterMarkStatus,DefaultEmailAddressName,FooterLocation,MessageDestinationID,UnreadMode,FromFormat,TicketLink,MultiMail,RequireTC,ReplyToEmailAddress,ReturnPathEmailAddress) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_TicketBoxID = query.GetLastInsertedID();
}

int TTicketBoxes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Name );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_BIT( query, m_AgeAlertingEnabled );
	BINDPARAM_LONG( query, m_AgeAlertingThresholdMins );
	BINDPARAM_LONG( query, m_MaxRecordsSize );
	BINDPARAM_BIT( query, m_RequireGetOldest );
	BINDPARAM_LONG( query, m_LowWaterMark );
	BINDPARAM_LONG( query, m_HighWaterMark );
	BINDPARAM_BIT( query, m_AutoReplyEnable );
	BINDPARAM_LONG( query, m_AutoReplyWithStdResponse );
	BINDPARAM_LONG( query, m_AutoReplyThreshHoldMins );
	BINDPARAM_BIT( query, m_AutoReplyQuoteMsg );
	BINDPARAM_BIT( query, m_AutoReplyCloseTicket );
	BINDPARAM_BIT( query, m_AutoReplyInTicket );
	BINDPARAM_LONG( query, m_HeaderID );
	BINDPARAM_LONG( query, m_FooterID );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_TCHAR( query, m_DefaultEmailAddress );
	BINDPARAM_TINYINT( query, m_WaterMarkStatus );
	BINDPARAM_TCHAR( query, m_DefaultEmailAddressName );
	BINDPARAM_TINYINT( query, m_FooterLocation );
	BINDPARAM_LONG( query, m_MessageDestinationID );
	BINDPARAM_LONG( query, m_UnreadMode );
	BINDPARAM_LONG( query, m_FromFormat );
	BINDPARAM_LONG( query, m_TicketLink );
	BINDPARAM_LONG( query, m_MultiMail );
	BINDPARAM_LONG( query, m_RequireTC );
	BINDPARAM_TCHAR( query, m_ReplyToEmailAddress );
	BINDPARAM_TCHAR( query, m_ReturnPathEmailAddress );
	BINDPARAM_LONG( query, m_TicketBoxID );
	query.Execute( _T("UPDATE TicketBoxes ")
	               _T("SET Name=?,Description=?,AgeAlertingEnabled=?,AgeAlertingThresholdMins=?,MaxRecordsSize=?,RequireGetOldest=?,LowWaterMark=?,HighWaterMark=?,AutoReplyEnable=?,AutoReplyWithStdResponse=?,AutoReplyThreshHoldMins=?,AutoReplyQuoteMsg=?,AutoReplyCloseTicket=?,AutoReplyInTicket=?,HeaderID=?,FooterID=?,OwnerID=?,DefaultEmailAddress=?,WaterMarkStatus=?,DefaultEmailAddressName=?,FooterLocation=?,MessageDestinationID=?,UnreadMode=?,FromFormat=?,TicketLink=?,MultiMail=?,RequireTC=?,ReplyToEmailAddress=?,ReturnPathEmailAddress=? ")
	               _T("WHERE TicketBoxID=?") );
	return query.GetRowCount();
}

int TTicketBoxes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxID );
	query.Execute( _T("DELETE FROM TicketBoxes ")
	               _T("WHERE TicketBoxID=?") );
	return query.GetRowCount();
}

int TTicketBoxes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_AgeAlertingEnabled );
	BINDCOL_LONG( query, m_AgeAlertingThresholdMins );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_LONG( query, m_MaxRecordsSize );
	BINDCOL_BIT( query, m_RequireGetOldest );
	BINDCOL_LONG( query, m_LowWaterMark );
	BINDCOL_LONG( query, m_HighWaterMark );
	BINDCOL_BIT( query, m_AutoReplyEnable );
	BINDCOL_LONG( query, m_AutoReplyWithStdResponse );
	BINDCOL_LONG( query, m_AutoReplyThreshHoldMins );
	BINDCOL_BIT( query, m_AutoReplyQuoteMsg );
	BINDCOL_BIT( query, m_AutoReplyCloseTicket );
	BINDCOL_BIT( query, m_AutoReplyInTicket );
	BINDCOL_LONG( query, m_HeaderID );
	BINDCOL_LONG( query, m_FooterID );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_TCHAR( query, m_DefaultEmailAddress );
	BINDCOL_TINYINT( query, m_WaterMarkStatus );
	BINDCOL_TCHAR( query, m_DefaultEmailAddressName );
	BINDCOL_TINYINT( query, m_FooterLocation );
	BINDCOL_LONG( query, m_MessageDestinationID );
	BINDCOL_LONG( query, m_UnreadMode );
	BINDCOL_LONG( query, m_FromFormat );
	BINDCOL_LONG( query, m_TicketLink );
	BINDCOL_LONG( query, m_MultiMail );
	BINDCOL_LONG( query, m_RequireTC );
	BINDCOL_TCHAR( query, m_ReplyToEmailAddress );
	BINDCOL_TCHAR( query, m_ReturnPathEmailAddress );
	BINDPARAM_LONG( query, m_TicketBoxID );
	query.Execute( _T("SELECT Name,Description,AgeAlertingEnabled,AgeAlertingThresholdMins,ObjectID,MaxRecordsSize,RequireGetOldest,LowWaterMark,HighWaterMark,AutoReplyEnable,AutoReplyWithStdResponse,AutoReplyThreshHoldMins,AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket,HeaderID,FooterID,OwnerID,DefaultEmailAddress,WaterMarkStatus,DefaultEmailAddressName,FooterLocation,MessageDestinationID,UnreadMode,FromFormat,TicketLink,MultiMail,RequireTC,ReplyToEmailAddress,ReturnPathEmailAddress ")
	               _T("FROM TicketBoxes ")
	               _T("WHERE TicketBoxID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketBoxes::GetLongData( CODBCQuery& query )
{
}

void TTicketBoxes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_BIT( query, m_AgeAlertingEnabled );
	BINDCOL_LONG( query, m_AgeAlertingThresholdMins );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_LONG( query, m_MaxRecordsSize );
	BINDCOL_BIT( query, m_RequireGetOldest );
	BINDCOL_LONG( query, m_LowWaterMark );
	BINDCOL_LONG( query, m_HighWaterMark );
	BINDCOL_BIT( query, m_AutoReplyEnable );
	BINDCOL_LONG( query, m_AutoReplyWithStdResponse );
	BINDCOL_LONG( query, m_AutoReplyThreshHoldMins );
	BINDCOL_BIT( query, m_AutoReplyQuoteMsg );
	BINDCOL_BIT( query, m_AutoReplyCloseTicket );
	BINDCOL_BIT( query, m_AutoReplyInTicket );
	BINDCOL_LONG( query, m_HeaderID );
	BINDCOL_LONG( query, m_FooterID );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_TCHAR( query, m_DefaultEmailAddress );
	BINDCOL_TINYINT( query, m_WaterMarkStatus );
	BINDCOL_TCHAR( query, m_DefaultEmailAddressName );
	BINDCOL_TINYINT( query, m_FooterLocation );
	BINDCOL_LONG( query, m_MessageDestinationID );
	BINDCOL_LONG( query, m_UnreadMode );
	BINDCOL_LONG( query, m_FromFormat );
	BINDCOL_LONG( query, m_TicketLink );
	BINDCOL_LONG( query, m_MultiMail );
	BINDCOL_LONG( query, m_RequireTC );
	BINDCOL_TCHAR( query, m_ReplyToEmailAddress );
	BINDCOL_TCHAR( query, m_ReturnPathEmailAddress );
	query.Execute( _T("SELECT TicketBoxID,Name,Description,AgeAlertingEnabled,AgeAlertingThresholdMins,ObjectID,MaxRecordsSize,RequireGetOldest,LowWaterMark,HighWaterMark,AutoReplyEnable,AutoReplyWithStdResponse,AutoReplyThreshHoldMins,AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket,HeaderID,FooterID,OwnerID,DefaultEmailAddress,WaterMarkStatus,DefaultEmailAddressName,FooterLocation,MessageDestinationID,UnreadMode,FromFormat,TicketLink,MultiMail,RequireTC,ReplyToEmailAddress,ReturnPathEmailAddress ")
	               _T("FROM TicketBoxes ")
	               _T("ORDER BY TicketBoxID") );
}

/* class TTicketContacts - 
		Relates ticket boxes to contacts.
	 */
void TTicketContacts::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_ContactID );
	query.Execute( _T("INSERT INTO TicketContacts ")
	               _T("(TicketID,ContactID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_TicketContactID = query.GetLastInsertedID();
}

int TTicketContacts::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_TicketContactID );
	query.Execute( _T("UPDATE TicketContacts ")
	               _T("SET TicketID=?,ContactID=? ")
	               _T("WHERE TicketContactID=?") );
	return query.GetRowCount();
}

int TTicketContacts::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketContactID );
	query.Execute( _T("DELETE FROM TicketContacts ")
	               _T("WHERE TicketContactID=?") );
	return query.GetRowCount();
}

int TTicketContacts::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_TicketContactID );
	query.Execute( _T("SELECT TicketID,ContactID ")
	               _T("FROM TicketContacts ")
	               _T("WHERE TicketContactID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketContacts::GetLongData( CODBCQuery& query )
{
}

void TTicketContacts::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketContactID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_ContactID );
	query.Execute( _T("SELECT TicketContactID,TicketID,ContactID ")
	               _T("FROM TicketContacts ")
	               _T("ORDER BY TicketContactID") );
}

/* class TTicketHistory - 
		Contains history information about the ticket
	 */
void TTicketHistory::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_TIME( query, m_DateTime );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TicketActionID );
	BINDPARAM_LONG( query, m_ID1 );
	BINDPARAM_LONG( query, m_ID2 );
	BINDPARAM_LONG( query, m_TicketStateID );
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_PriorityID );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	BINDPARAM_TCHAR( query, m_DataValue );
	query.Execute( _T("INSERT INTO TicketHistory ")
	               _T("(TicketID,DateTime,AgentID,TicketActionID,ID1,ID2,TicketStateID,TicketBoxID,OwnerID,PriorityID,TicketCategoryID,DataValue) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_TicketHistoryID = query.GetLastInsertedID();
}

int TTicketHistory::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TicketActionID );
	BINDPARAM_LONG( query, m_ID1 );
	BINDPARAM_LONG( query, m_ID2 );
	BINDPARAM_LONG( query, m_TicketStateID );
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_PriorityID );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	BINDPARAM_TCHAR( query, m_DataValue );
	BINDPARAM_LONG( query, m_TicketHistoryID );
	query.Execute( _T("UPDATE TicketHistory ")
	               _T("SET TicketID=?,AgentID=?,TicketActionID=?,ID1=?,ID2=?,TicketStateID=?,TicketBoxID=?,OwnerID=?,PriorityID=?,TicketCategoryID=?,DataValue=? ")
	               _T("WHERE TicketHistoryID=?") );
	return query.GetRowCount();
}

int TTicketHistory::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketHistoryID );
	query.Execute( _T("DELETE FROM TicketHistory ")
	               _T("WHERE TicketHistoryID=?") );
	return query.GetRowCount();
}

int TTicketHistory::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_TIME( query, m_DateTime );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_TicketActionID );
	BINDCOL_LONG( query, m_ID1 );
	BINDCOL_LONG( query, m_ID2 );
	BINDCOL_LONG( query, m_TicketStateID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_LONG( query, m_TicketCategoryID );
	BINDCOL_TCHAR( query, m_DataValue );	
	BINDPARAM_LONG( query, m_TicketHistoryID );
	query.Execute( _T("SELECT TicketID,DateTime,AgentID,TicketActionID,ID1,ID2,TicketStateID,TicketBoxID,OwnerID,PriorityID,TicketCategoryID,DataValue ")
	               _T("FROM TicketHistory ")
	               _T("WHERE TicketHistoryID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketHistory::GetLongData( CODBCQuery& query )
{
}

void TTicketHistory::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketHistoryID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_TIME( query, m_DateTime );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_TicketActionID );
	BINDCOL_LONG( query, m_ID1 );
	BINDCOL_LONG( query, m_ID2 );
	BINDCOL_LONG( query, m_TicketStateID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_LONG( query, m_TicketCategoryID );
	BINDCOL_TCHAR( query, m_DataValue );	
	query.Execute( _T("SELECT TicketHistoryID,TicketID,DateTime,AgentID,TicketActionID,ID1,ID2,TicketStateID,TicketBoxID,OwnerID,PriorityID,TicketCategoryID,DataValue ")
	               _T("FROM TicketHistory ")
	               _T("ORDER BY TicketHistoryID") );
}

/* class TTicketNotes - 
		Allows for multiple notes to be related to tickets. 
		Some ticket notes may be generated by the system to 
		indicate transfer of ownership, etc.
	 */
void TTicketNotes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_TEXT( query, m_Note );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_BIT( query, m_IsVoipNote );
	BINDPARAM_TCHAR( query, m_ElapsedTime );
	BINDPARAM_LONG( query, m_ContactID );	
	if ( m_StartTimeLen <= 0 || m_StopTimeLen <= 0 )
	{
		query.Execute( _T("INSERT INTO TicketNotes ")
	               _T("(TicketID,AgentID,Note,DateCreated,IsVoipNote,ElapsedTime,ContactID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?)") );	
	}
	else
	{
		BINDPARAM_TIME( query, m_StartTime );
		BINDPARAM_TIME( query, m_StopTime );
		query.Execute( _T("INSERT INTO TicketNotes ")
	               _T("(TicketID,AgentID,Note,DateCreated,IsVoipNote,ElapsedTime,ContactID,StartTime,StopTime) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?)") );	
	}
	m_TicketNoteID = query.GetLastInsertedID();
}

int TTicketNotes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_TEXT( query, m_Note );
	BINDPARAM_BIT( query, m_IsVoipNote );
	if ( m_StartTimeLen <= 0 || m_StopTimeLen <= 0 )
	{
		BINDPARAM_LONG( query, m_TicketNoteID );
		query.Execute( _T("UPDATE TicketNotes ")
					_T("SET TicketID=?,AgentID=?,Note=?,IsVoipNote=? ")
					_T("WHERE TicketNoteID=?") );
		return query.GetRowCount();
	}
	else
	{
		BINDPARAM_TCHAR( query, m_ElapsedTime );
		BINDPARAM_TIME( query, m_StartTime );
		BINDPARAM_TIME( query, m_StopTime );
		BINDPARAM_LONG( query, m_TicketNoteID );
		query.Execute( _T("UPDATE TicketNotes ")
					_T("SET TicketID=?,AgentID=?,Note=?,IsVoipNote=?,ElapsedTime=?,StartTime=?,StopTime=? ")
					_T("WHERE TicketNoteID=?") );
		return query.GetRowCount();	
	}
}

int TTicketNotes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketNoteID );
	query.Execute( _T("DELETE FROM TicketNotes ")
	               _T("WHERE TicketNoteID=?") );
	return query.GetRowCount();
}

int TTicketNotes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_NumAttach );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_BIT( query, m_IsVoipNote );
	BINDCOL_TIME( query, m_StartTime );
	BINDCOL_TIME( query, m_StopTime );
	BINDCOL_TCHAR( query, m_ElapsedTime );
	BINDCOL_LONG( query, m_ContactID );
	BINDPARAM_LONG( query, m_TicketNoteID );
	query.Execute( _T("SELECT TicketID,(SELECT COUNT(1) FROM NoteAttachments WHERE NoteID=TicketNoteID AND NoteTypeID=1),AgentID,DateCreated,IsVoipNote,StartTime,StopTime,ElapsedTime,ContactID,Note ")
	               _T("FROM TicketNotes ")
	               _T("WHERE TicketNoteID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketNotes::GetLongData( CODBCQuery& query )
{
	GETDATA_TEXT( query, m_Note );
}

void TTicketNotes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketNoteID );
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_NumAttach );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_BIT( query, m_IsVoipNote );
	BINDCOL_TIME( query, m_StartTime );
	BINDCOL_TIME( query, m_StopTime );
	BINDCOL_TCHAR( query, m_ElapsedTime );
	BINDCOL_LONG( query, m_ContactID );
	query.Execute( _T("SELECT TicketNoteID,TicketID,(SELECT COUNT(1) FROM NoteAttachments WHERE NoteID=TicketNoteID AND NoteTypeID=1),AgentID,DateCreated,IsVoipNote,StartTime,StopTime,ElapsedTime,ContactID,Note ")
	               _T("FROM TicketNotes ")
	               _T("ORDER BY TicketNoteID") );
}

/* class TTicketStates - 
		The state of the ticket. Examples include:
		open, closed, and escalated.
	 */
void TTicketStates::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_HTMLColorName );
	query.Execute( _T("INSERT INTO TicketStates ")
	               _T("(Description,HTMLColorName) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_TicketStateID = query.GetLastInsertedID();
}

int TTicketStates::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_HTMLColorName );
	BINDPARAM_LONG( query, m_TicketStateID );
	query.Execute( _T("UPDATE TicketStates ")
	               _T("SET Description=?,HTMLColorName=? ")
	               _T("WHERE TicketStateID=?") );
	return query.GetRowCount();
}

int TTicketStates::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketStateID );
	query.Execute( _T("DELETE FROM TicketStates ")
	               _T("WHERE TicketStateID=?") );
	return query.GetRowCount();
}

int TTicketStates::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_HTMLColorName );
	BINDPARAM_LONG( query, m_TicketStateID );
	query.Execute( _T("SELECT Description,HTMLColorName ")
	               _T("FROM TicketStates ")
	               _T("WHERE TicketStateID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketStates::GetLongData( CODBCQuery& query )
{
}

void TTicketStates::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketStateID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_HTMLColorName );
	query.Execute( _T("SELECT TicketStateID,Description,HTMLColorName ")
	               _T("FROM TicketStates ")
	               _T("ORDER BY TicketStateID") );
}


/* class TTickets - 
		Tickets represent a logical dialog with a specific contact. 
		Tickets can be thought of as "incidents", and should relate to a 
		specific issue, premise, or problem.
	 */
void TTickets::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketStateID );
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_TCHAR( query, m_Contacts );
	BINDPARAM_LONG( query, m_PriorityID );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_TIME( query, m_OpenTimestamp );
	BINDPARAM_LONG( query, m_OpenMins );
	BINDPARAM_BIT( query, m_AutoReplied );
	BINDPARAM_BIT( query, m_AgeAlerted );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_TIME( query, m_DeletedTime );
	BINDPARAM_LONG( query, m_DeletedBy );
	BINDPARAM_LONG( query, m_LockedBy );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	query.Execute( _T("INSERT INTO Tickets ")
	               _T("(TicketStateID,TicketBoxID,OwnerID,Subject,Contacts,PriorityID,DateCreated,OpenTimestamp,OpenMins,AutoReplied,AgeAlerted,IsDeleted,DeletedTime,DeletedBy,LockedBy,TicketCategoryID) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)") );
	m_TicketID = query.GetLastInsertedID();
}

int TTickets::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketStateID );
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_TCHAR( query, m_Subject );
	BINDPARAM_TCHAR( query, m_Contacts );
	BINDPARAM_LONG( query, m_PriorityID );
	BINDPARAM_TIME( query, m_OpenTimestamp );
	BINDPARAM_LONG( query, m_OpenMins );
	BINDPARAM_BIT( query, m_AutoReplied );
	BINDPARAM_BIT( query, m_AgeAlerted );
	BINDPARAM_TINYINT( query, m_IsDeleted );
	BINDPARAM_TIME( query, m_DeletedTime );
	BINDPARAM_LONG( query, m_DeletedBy );
	BINDPARAM_LONG( query, m_LockedBy );
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	query.Execute( _T("UPDATE Tickets ")
	               _T("SET TicketStateID=?,TicketBoxID=?,OwnerID=?,Subject=?,Contacts=?,PriorityID=?,OpenTimestamp=?,OpenMins=?,AutoReplied=?,AgeAlerted=?,IsDeleted=?,DeletedTime=?,DeletedBy=?,LockedBy=?,TicketCategoryID=? ")
	               _T("WHERE TicketID=?") );
	return query.GetRowCount();
}

int TTickets::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	query.Execute( _T("DELETE FROM Tickets ")
	               _T("WHERE TicketID=?") );
	return query.GetRowCount();
}

int TTickets::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketStateID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_TCHAR( query, m_Subject );
	BINDCOL_TCHAR( query, m_Contacts );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TIME( query, m_OpenTimestamp );
	BINDCOL_LONG( query, m_OpenMins );
	BINDCOL_BIT( query, m_AutoReplied );
	BINDCOL_BIT( query, m_AgeAlerted );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_TIME( query, m_DeletedTime );
	BINDCOL_LONG( query, m_DeletedBy );
	BINDCOL_TIME( query, m_LockedTime );
	BINDCOL_LONG( query, m_LockedBy );
	BINDCOL_BIT( query, m_UseTickler);
	BINDCOL_TIME( query, m_ReOpenTime);
	BINDCOL_LONG( query, m_FolderID);
	BINDCOL_LONG( query, m_TicketCategoryID);
	BINDPARAM_LONG( query, m_TicketID );

	query.Execute( _T("SELECT TicketStateID,TicketBoxID,OwnerID,Subject,Contacts,PriorityID,DateCreated,OpenTimestamp,OpenMins,AutoReplied,AgeAlerted,IsDeleted,DeletedTime,DeletedBy,LockedTime,LockedBy,UseTickler,TicklerDateToReopen,FolderID,TicketCategoryID ")
	               _T("FROM Tickets ")
	               _T("WHERE TicketID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTickets::GetLongData( CODBCQuery& query )
{
}

void TTickets::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_TicketStateID );
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_TCHAR( query, m_Subject );
	BINDCOL_TCHAR( query, m_Contacts );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_TIME( query, m_OpenTimestamp );
	BINDCOL_LONG( query, m_OpenMins );
	BINDCOL_BIT( query, m_AutoReplied );
	BINDCOL_BIT( query, m_AgeAlerted );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_TIME( query, m_DeletedTime );
	BINDCOL_LONG( query, m_DeletedBy );
	BINDCOL_TIME( query, m_LockedTime );
	BINDCOL_LONG( query, m_LockedBy );
	BINDCOL_BIT( query, m_UseTickler);
	BINDCOL_TIME( query, m_ReOpenTime);
	BINDCOL_LONG( query, m_TicketCategoryID );
	
	query.Execute( _T("SELECT TicketID,TicketStateID,TicketBoxID,OwnerID,Subject,Contacts,PriorityID,DateCreated,OpenTimestamp,OpenMins,AutoReplied,AgeAlerted,IsDeleted,DeletedTime,DeletedBy,LockedTime,LockedBy,UseTickler,TicklerDateToReopen,TicketCategoryID ")
	               _T("FROM Tickets ")
	               _T("ORDER BY TicketID") );
}

/* class TAgentActivity - 
		Agent Activity Audit.
	 */
void TAgentActivity::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_ActivityID );
	BINDPARAM_LONG( query, m_ID1 );
	BINDPARAM_LONG( query, m_ID2 );
	BINDPARAM_LONG( query, m_ID3 );
	BINDPARAM_TCHAR( query, m_Data1 );
	BINDPARAM_TCHAR( query, m_Data2 );	
	query.Execute( _T("INSERT INTO AgentActivityLog ")
	               _T("(AgentID,ActivityID,ActivityDate,ID1,ID2,ID3,Data1,Data2) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?)") );
	m_AgentActivityID = query.GetLastInsertedID();
}

int TAgentActivity::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_ActivityID );
	BINDPARAM_LONG( query, m_ID1 );
	BINDPARAM_LONG( query, m_ID2 );
	BINDPARAM_LONG( query, m_ID3 );
	BINDPARAM_TCHAR( query, m_Data1 );
	BINDPARAM_TCHAR( query, m_Data2 );
	BINDPARAM_LONG( query, m_AgentActivityID );	
	query.Execute( _T("UPDATE AgentActivityLog ")
	               _T("SET AgentID=?,ActivityID=?,ActivityDate=?,ID1=?,ID2=?,ID3=?,Data1=?,Data2=? ")
	               _T("WHERE AgentActivityID=?") );
	return query.GetRowCount();
}

int TAgentActivity::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentActivityID );
	query.Execute( _T("DELETE FROM AgentActivityLog ")
	               _T("WHERE AgentActivityID=?") );
	return query.GetRowCount();
}

int TAgentActivity::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ActivityID );
	BINDCOL_TIME( query, m_ActivityDate );
	BINDCOL_LONG( query, m_ID1 );
	BINDCOL_LONG( query, m_ID2 );
	BINDCOL_LONG( query, m_ID3 );
	BINDCOL_TCHAR( query, m_Data1 );
	BINDCOL_TCHAR( query, m_Data2 );
	BINDPARAM_LONG( query, m_AgentActivityID );
	query.Execute( _T("SELECT AgentID,ActivityID,ActivityDate,ID1,ID2,ID3,Data1,Data2 ")
	               _T("FROM AgentActivityLog ")
	               _T("WHERE AgentActivityID=?") );
	nResult = query.Fetch();
	
	return nResult;
}

void TAgentActivity::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AgentActivityID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ActivityID );
	BINDCOL_TIME( query, m_ActivityDate );
	BINDCOL_LONG( query, m_ID1 );
	BINDCOL_LONG( query, m_ID2 );
	BINDCOL_LONG( query, m_ID3 );
	BINDCOL_TCHAR( query, m_Data1 );
	BINDCOL_TCHAR( query, m_Data2 );
	query.Execute( _T("SELECT AgentActivityID,AgentID,ActivityID,ActivityDate,ID1,ID2,ID3,Data1,Data2 ")
	               _T("FROM AgentActivityLog") );
}

/* class TAgentActivities - 
		Agent Activities.
	 */
void TAgentActivities::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO AgentActivities ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_AgentActivityID = query.GetLastInsertedID();
}

int TAgentActivities::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_AgentActivityID );
	query.Execute( _T("UPDATE AgentActivities ")
	               _T("SET Description=? ")
	               _T("WHERE AgentActivityID=?") );
	return query.GetRowCount();
}

int TAgentActivities::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentActivityID );
	query.Execute( _T("DELETE FROM AgentActivities ")
	               _T("WHERE AgentActivityID=?") );
	return query.GetRowCount();
}

int TAgentActivities::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_AgentActivityID );

	query.Execute( _T("SELECT Description ")
	               _T("FROM AgentActivities ")
	               _T("WHERE AgentActivityID=?") );
	nResult = query.Fetch();
	
	return nResult;
}

void TAgentActivities::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AgentActivityID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT AgentActivityID,Description ")
	               _T("FROM AgentActivities") );
}

/* class TAgentTicketCategories - 
		Agent Ticket Categories.
	 */
void TAgentTicketCategories::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	query.Execute( _T("INSERT INTO AgentTicketCategories ")
	               _T("(AgentID,TicketCategoryID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_AgentTicketCategoryID = query.GetLastInsertedID();
}

int TAgentTicketCategories::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_TicketCategoryID );
	BINDPARAM_LONG( query, m_AgentTicketCategoryID );
	query.Execute( _T("UPDATE AgentTicketCategories ")
	               _T("SET AgentID=?,TicketCategoryID=? ")
	               _T("WHERE AgentTicketCategoryID=?") );
	return query.GetRowCount();
}

int TAgentTicketCategories::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentTicketCategoryID );
	query.Execute( _T("DELETE FROM AgentTicketCategories ")
	               _T("WHERE AgentTicketCategoryID=?") );
	return query.GetRowCount();
}

int TAgentTicketCategories::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_TicketCategoryID );
	BINDPARAM_LONG( query, m_AgentTicketCategoryID );
	query.Execute( _T("SELECT AgentID,TicketCategoryID ")
	               _T("FROM AgentTicketCategories ")
	               _T("WHERE AgentTicketCategoryID=?") );
	nResult = query.Fetch();
	
	return nResult;
}

void TAgentTicketCategories::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AgentTicketCategoryID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_TicketCategoryID );
	query.Execute( _T("SELECT AgentTicketCategoryID,AgentID,TicketCategoryID ")
	               _T("FROM AgentTicketCategories ") );
}

/* class TTicketLinks - 
		Ticket Link definitions.
	 */
void TTicketLinks::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_LinkName );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_IsDeleted );
	query.Execute( _T("INSERT INTO TicketLinks ")
	               _T("(LinkName,DateCreated,OwnerID,IsDeleted) ")
	               _T("VALUES")
	               _T("(?,?,?,?)") );
	m_TicketLinkID = query.GetLastInsertedID();
}

int TTicketLinks::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_LinkName );
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_OwnerID );
	BINDPARAM_LONG( query, m_IsDeleted );
	BINDPARAM_LONG( query, m_TicketLinkID );
	query.Execute( _T("UPDATE TicketLinks ")
	               _T("SET LinkName=?,DateCreated=?,OwnerID=?,IsDeleted=? ")
	               _T("WHERE TicketLinkID=?") );
	return query.GetRowCount();
}

int TTicketLinks::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketLinkID );
	query.Execute( _T("DELETE FROM TicketLinks ")
	               _T("WHERE TicketLinkID=?") );
	return query.GetRowCount();
}

int TTicketLinks::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_LinkName );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_LONG( query, m_IsDeleted );
	BINDPARAM_LONG( query, m_TicketLinkID );
	query.Execute( _T("SELECT LinkName,DateCreated,OwnerID,IsDeleted ")
	               _T("FROM TicketLinks ")
	               _T("WHERE TicketLinkID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketLinks::GetLongData( CODBCQuery& query )
{
}

void TTicketLinks::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_TCHAR( query, m_LinkName );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_LONG( query, m_OwnerID );
	BINDCOL_LONG( query, m_IsDeleted );
	BINDCOL_LONG( query, m_TicketLinkID );
	query.Execute( _T("SELECT LinkName,DateCreated,OwnerID,IsDeleted,TicketLinkID ")
	               _T("FROM TicketLinks") );
}

/* class TTicketLinksTicket - 
		Ticket Link to Ticket definitions.
	 */
void TTicketLinksTicket::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_TicketLinkID );	
	query.Execute( _T("INSERT INTO TicketLinksTicket ")
	               _T("(TicketID,TicketLinkID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_TicketLinksTicketID = query.GetLastInsertedID();
}

int TTicketLinksTicket::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_TicketLinkID );
	BINDPARAM_LONG( query, m_TicketLinksTicketID );
	query.Execute( _T("UPDATE TicketLinksTicket ")
	               _T("SET TicketID=?,TicketLinkID=? ")
	               _T("WHERE TicketLinksTicketID=?") );
	return query.GetRowCount();
}

int TTicketLinksTicket::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketLinksTicketID );
	query.Execute( _T("DELETE FROM TicketLinksTicket ")
	               _T("WHERE TicketLinksTicketID=?") );
	return query.GetRowCount();
}

int TTicketLinksTicket::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_TicketLinkID );
	BINDPARAM_LONG( query, m_TicketLinksTicketID );
	query.Execute( _T("SELECT TicketID,TicketLinkID ")
	               _T("FROM TicketLinksTicket ")
	               _T("WHERE TicketLinksTicketID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketLinksTicket::GetLongData( CODBCQuery& query )
{
}

void TTicketLinksTicket::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_TicketLinkID );
	BINDCOL_LONG( query, m_TicketLinksTicketID );
	query.Execute( _T("SELECT TicketID,TicketLinkID,TicketLinksTicketID ")
	               _T("FROM TicketLinksTicket ")
	               _T("ORDER BY TicketID") );
}

/* class TTicketFields - 
		Ticket Field definitions.
	 */
void TTicketFields::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_TicketFieldTypeID );
	query.Execute( _T("INSERT INTO TicketFields ")
	               _T("(Description,TicketFieldTypeID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_TicketFieldID = query.GetLastInsertedID();
}

int TTicketFields::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_TicketFieldTypeID );
	BINDPARAM_LONG( query, m_TicketFieldID );
	query.Execute( _T("UPDATE TicketFields ")
	               _T("SET Description=?,TicketFieldTypeID=? ")
	               _T("WHERE TicketFieldID=?") );
	return query.GetRowCount();
}

int TTicketFields::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketFieldID );
	query.Execute( _T("DELETE FROM TicketFields ")
	               _T("WHERE TicketFieldID=?") );
	return query.GetRowCount();
}

int TTicketFields::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_TicketFieldTypeID );
	BINDPARAM_LONG( query, m_TicketFieldID );
	query.Execute( _T("SELECT Description,TicketFieldTypeID ")
	               _T("FROM TicketFields ")
	               _T("WHERE TicketFieldID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketFields::GetLongData( CODBCQuery& query )
{
}

void TTicketFields::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_TicketFieldTypeID );
	BINDCOL_LONG( query, m_TicketFieldID );
	query.Execute( _T("SELECT Description, TicketFieldTypeID, TicketFieldID ")
	               _T("FROM TicketFields ")
	               _T("ORDER BY Description") );
}

/* class TTicketFieldTypes - 
		Ticket Field Type definitions.
	 */
void TTicketFieldTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO TicketFieldTypes ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_TicketFieldTypeID = query.GetLastInsertedID();
}

int TTicketFieldTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_TicketFieldTypeID );
	query.Execute( _T("UPDATE TicketFieldTypes ")
	               _T("SET Description=? ")
	               _T("WHERE TicketFieldTypeID=?") );
	return query.GetRowCount();
}

int TTicketFieldTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketFieldTypeID );
	query.Execute( _T("DELETE FROM TicketFieldTypes ")
	               _T("WHERE TicketFieldTypeID=?") );
	return query.GetRowCount();
}

int TTicketFieldTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_TicketFieldTypeID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM TicketFieldTypes ")
	               _T("WHERE TicketFieldTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketFieldTypes::GetLongData( CODBCQuery& query )
{
}

void TTicketFieldTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_TicketFieldTypeID );
	query.Execute( _T("SELECT Description, TicketFieldTypeID ")
	               _T("FROM TicketFieldTypes ")
	               _T("ORDER BY Description") );
}

/* class TTicketFieldOptions - 
		Ticket Field Option definitions.
	 */
void TTicketFieldOptions::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketFieldID );
	BINDPARAM_TCHAR( query, m_OptionValue );
	BINDPARAM_LONG( query, m_OptionOrder );
	query.Execute( _T("INSERT INTO TicketFieldOptions ")
	               _T("(TicketFieldID,OptionValue,OptionOrder) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_TicketFieldID = query.GetLastInsertedID();
}

int TTicketFieldOptions::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketFieldID );
	BINDPARAM_TCHAR( query, m_OptionValue );
	BINDPARAM_LONG( query, m_OptionOrder );
	BINDPARAM_LONG( query, m_TicketFieldOptionID );
	query.Execute( _T("UPDATE TicketFieldOptions ")
	               _T("SET TicketFieldID=?,OptionValue=?,OptionOrder=? ")
	               _T("WHERE TicketFieldOptionID=?") );
	return query.GetRowCount();
}

int TTicketFieldOptions::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketFieldOptionID );
	query.Execute( _T("DELETE FROM TicketFieldOptions ")
	               _T("WHERE TicketFieldOptionID=?") );
	return query.GetRowCount();
}

int TTicketFieldOptions::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketFieldID );
	BINDCOL_TCHAR( query, m_OptionValue );
	BINDCOL_LONG( query, m_OptionOrder );
	BINDPARAM_LONG( query, m_TicketFieldOptionID );
	query.Execute( _T("SELECT TicketFieldID,OptionValue,OptionOrder ")
	               _T("FROM TicketFieldOptions ")
	               _T("WHERE TicketFieldOptionID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketFieldOptions::GetLongData( CODBCQuery& query )
{
}

void TTicketFieldOptions::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketFieldID );
	BINDCOL_TCHAR( query, m_OptionValue );
	BINDCOL_LONG( query, m_OptionOrder );
	BINDCOL_LONG( query, m_TicketFieldOptionID );
	query.Execute( _T("SELECT TicketFieldID,OptionValue,OptionOrder,TicketFieldOptionID ")
	               _T("FROM TicketFieldOptions ")
	               _T("ORDER BY TicketFieldID,OptionOrder") );
}

/* class TTicketFieldsTicketBox - 
		Ticket Field to TicketBox definitions.
	 */
void TTicketFieldsTicketBox::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_TicketFieldID );
	BINDPARAM_BIT( query, m_IsRequired );
	BINDPARAM_BIT( query, m_IsViewed );
	BINDPARAM_BIT( query, m_SetDefault );
	BINDPARAM_TCHAR( query, m_DefaultValue );
	query.Execute( _T("INSERT INTO TicketFieldsTicketBox ")
	               _T("(TicketBoxID,TicketFieldID,IsRequired,IsViewed,SetDefault,DefaultValue) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?)") );
	m_TicketFieldsTicketBoxID = query.GetLastInsertedID();
}

int TTicketFieldsTicketBox::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxID );
	BINDPARAM_LONG( query, m_TicketFieldID );
	BINDPARAM_BIT( query, m_IsRequired );
	BINDPARAM_BIT( query, m_IsViewed );
	BINDPARAM_BIT( query, m_SetDefault );
	BINDPARAM_TCHAR( query, m_DefaultValue );
	BINDPARAM_LONG( query, m_TicketFieldsTicketBoxID );
	query.Execute( _T("UPDATE TicketFieldsTicketBox ")
	               _T("SET TicketBoxID=?,TicketFieldID=?,IsRequired=?,IsViewed=?,SetDefault=?,DefaultValue=? ")
	               _T("WHERE TicketFieldsTicketBoxID=?") );
	return query.GetRowCount();
}

int TTicketFieldsTicketBox::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketFieldsTicketBoxID );
	query.Execute( _T("DELETE FROM TicketFieldsTicketBox ")
	               _T("WHERE TicketFieldsTicketBoxID=?") );
	return query.GetRowCount();
}

int TTicketFieldsTicketBox::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_TicketFieldID );
	BINDCOL_BIT( query, m_IsRequired );
	BINDCOL_BIT( query, m_IsViewed );
	BINDCOL_BIT( query, m_SetDefault );
	BINDCOL_TCHAR( query, m_DefaultValue );
	BINDPARAM_LONG( query, m_TicketFieldsTicketBoxID );
	query.Execute( _T("SELECT TicketBoxID,TicketFieldID,IsRequired,IsViewed,SetDefault,DefaultValue ")
	               _T("FROM TicketFieldsTicketBox ")
	               _T("WHERE TicketFieldsTicketBoxID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

int TTicketFieldsTicketBox::QueryByTB( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketFieldsTicketBoxID );
	BINDCOL_LONG( query, m_TicketFieldID );
	BINDCOL_BIT( query, m_IsRequired );
	BINDCOL_BIT( query, m_IsViewed );
	BINDCOL_BIT( query, m_SetDefault );
	BINDCOL_TCHAR( query, m_DefaultValue );
	BINDPARAM_LONG( query, m_TicketBoxID );
	query.Execute( _T("SELECT TicketFieldsTicketBoxID,TicketFieldID,IsRequired,IsViewed,SetDefault,DefaultValue ")
	               _T("FROM TicketFieldsTicketBox ")
	               _T("WHERE TicketBoxID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketFieldsTicketBox::GetLongData( CODBCQuery& query )
{
}

void TTicketFieldsTicketBox::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_LONG( query, m_TicketFieldID );
	BINDCOL_BIT( query, m_IsRequired );
	BINDCOL_BIT( query, m_IsViewed );
	BINDCOL_BIT( query, m_SetDefault );
	BINDCOL_TCHAR( query, m_DefaultValue );
	BINDCOL_LONG( query, m_TicketFieldsTicketBoxID );
	query.Execute( _T("SELECT TicketBoxID,TicketFieldID,IsRequired,IsViewed,SetDefault,DefaultValue,TicketFieldsTicketBoxID ")
	               _T("FROM TicketFieldsTicketBox ")
	               _T("ORDER BY TicketBoxID") );
}

/* class TTicketFieldsTicket - 
		Ticket Field to Ticket definitions.
	 */
void TTicketFieldsTicket::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_TicketFieldID );
	BINDPARAM_TCHAR( query, m_DataValue );
	query.Execute( _T("INSERT INTO TicketFieldsTicket ")
	               _T("(TicketID,TicketFieldID,DataValue) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_TicketFieldsTicketID = query.GetLastInsertedID();
}

int TTicketFieldsTicket::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketID );
	BINDPARAM_LONG( query, m_TicketFieldID );
	BINDPARAM_TCHAR( query, m_DataValue );
	BINDPARAM_LONG( query, m_TicketFieldsTicketID );
	query.Execute( _T("UPDATE TicketFieldsTicket ")
	               _T("SET TicketID=?,TicketFieldID=?,DataValue=? ")
	               _T("WHERE TicketFieldsTicketID=?") );
	return query.GetRowCount();
}

int TTicketFieldsTicket::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketFieldsTicketID );
	query.Execute( _T("DELETE FROM TicketFieldsTicket ")
	               _T("WHERE TicketFieldsTicketID=?") );
	return query.GetRowCount();
}

int TTicketFieldsTicket::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_TicketFieldID );
	BINDCOL_TCHAR( query, m_DataValue );
	BINDPARAM_LONG( query, m_TicketFieldsTicketID );
	query.Execute( _T("SELECT TicketID,TicketFieldID,DataValue ")
	               _T("FROM TicketFieldsTicket ")
	               _T("WHERE TicketFieldsTicketID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketFieldsTicket::GetLongData( CODBCQuery& query )
{
}

void TTicketFieldsTicket::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketID );
	BINDCOL_LONG( query, m_TicketFieldID );
	BINDCOL_TCHAR( query, m_DataValue );
	BINDCOL_LONG( query, m_TicketFieldsTicketID );
	query.Execute( _T("SELECT TicketID,TicketFieldID,DataValue,TicketFieldsTicketID ")
	               _T("FROM TicketFieldsTicket ")
	               _T("ORDER BY TicketID") );
}

/* class TTicketFieldViews - 
		Ticket Field Views.
	 */
void TTicketFieldViews::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxViewID );
	BINDPARAM_LONG( query, m_TicketFieldID );
	BINDPARAM_BIT( query, m_ShowField );
	query.Execute( _T("INSERT INTO TicketFieldViews ")
	               _T("(TicketBoxViewID,TicketFieldID,ShowField) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_TicketFieldViewID = query.GetLastInsertedID();
}

int TTicketFieldViews::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketBoxViewID );
	BINDPARAM_LONG( query, m_TicketFieldID );
	BINDPARAM_BIT( query, m_ShowField );
	BINDPARAM_LONG( query, m_TicketFieldViewID );
	query.Execute( _T("UPDATE TicketFieldViews ")
	               _T("SET TicketBoxViewID=?,TicketFieldID=?,ShowField=? ")
	               _T("WHERE TicketFieldViewID=?") );
	return query.GetRowCount();
}

int TTicketFieldViews::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TicketFieldViewID );
	query.Execute( _T("DELETE FROM TicketFieldViews ")
	               _T("WHERE TicketFieldViewID=?") );
	return query.GetRowCount();
}

int TTicketFieldViews::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxViewID );
	BINDCOL_LONG( query, m_TicketFieldID );
	BINDCOL_BIT( query, m_ShowField );
	BINDPARAM_LONG( query, m_TicketFieldViewID );
	query.Execute( _T("SELECT TicketBoxViewID,TicketFieldID,ShowField ")
	               _T("FROM TicketFieldViews ")
	               _T("WHERE TicketFieldViewID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTicketFieldViews::GetLongData( CODBCQuery& query )
{
}

void TTicketFieldViews::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxViewID );
	BINDCOL_LONG( query, m_TicketFieldID );
	BINDCOL_BIT( query, m_ShowField );
	BINDCOL_LONG( query, m_TicketFieldViewID );
	query.Execute( _T("SELECT TicketBoxViewID,TicketFieldID,ShowField,TicketFieldViewID ")
	               _T("FROM TicketFieldViews ")
	               _T("ORDER BY TicketBoxViewID") );
}

/* class TTransactionTypes - 
		Types of transactions
	 */
void TTransactionTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO TransactionTypes ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_TransactionTypeID = query.GetLastInsertedID();
}

int TTransactionTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_TransactionTypeID );
	query.Execute( _T("UPDATE TransactionTypes ")
	               _T("SET Description=? ")
	               _T("WHERE TransactionTypeID=?") );
	return query.GetRowCount();
}

int TTransactionTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TransactionTypeID );
	query.Execute( _T("DELETE FROM TransactionTypes ")
	               _T("WHERE TransactionTypeID=?") );
	return query.GetRowCount();
}

int TTransactionTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_TransactionTypeID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM TransactionTypes ")
	               _T("WHERE TransactionTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTransactionTypes::GetLongData( CODBCQuery& query )
{
}

void TTransactionTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TransactionTypeID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT TransactionTypeID,Description ")
	               _T("FROM TransactionTypes ")
	               _T("ORDER BY TransactionTypeID") );
}

/* class TTransactions - 
		Stores homemade transaction details. Rows are deleted when the transaction is completed.
	 */
void TTransactions::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TIME( query, m_DateCreated );
	BINDPARAM_LONG( query, m_TransactionTypeID );
	BINDPARAM_LONG( query, m_ObjectID );
	BINDPARAM_TCHAR( query, m_ParameterList );
	BINDPARAM_LONG( query, m_Progress );
	query.Execute( _T("INSERT INTO Transactions ")
	               _T("(DateCreated,TransactionTypeID,ObjectID,ParameterList,Progress) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?)") );
	m_TransactionID = query.GetLastInsertedID();
}

int TTransactions::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_ParameterList );
	BINDPARAM_LONG( query, m_Progress );
	BINDPARAM_LONG( query, m_TransactionID );
	query.Execute( _T("UPDATE Transactions ")
	               _T("SET ParameterList=?,Progress=? ")
	               _T("WHERE TransactionID=?") );
	return query.GetRowCount();
}

int TTransactions::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TransactionID );
	query.Execute( _T("DELETE FROM Transactions ")
	               _T("WHERE TransactionID=?") );
	return query.GetRowCount();
}

int TTransactions::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_LONG( query, m_TransactionTypeID );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_TCHAR( query, m_ParameterList );
	BINDCOL_LONG( query, m_Progress );
	BINDPARAM_LONG( query, m_TransactionID );
	query.Execute( _T("SELECT DateCreated,TransactionTypeID,ObjectID,ParameterList,Progress ")
	               _T("FROM Transactions ")
	               _T("WHERE TransactionID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTransactions::GetLongData( CODBCQuery& query )
{
}

void TTransactions::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TransactionID );
	BINDCOL_TIME( query, m_DateCreated );
	BINDCOL_LONG( query, m_TransactionTypeID );
	BINDCOL_LONG( query, m_ObjectID );
	BINDCOL_TCHAR( query, m_ParameterList );
	BINDCOL_LONG( query, m_Progress );
	query.Execute( _T("SELECT TransactionID,DateCreated,TransactionTypeID,ObjectID,ParameterList,Progress ")
	               _T("FROM Transactions ")
	               _T("ORDER BY TransactionID") );
}

/* class TUIDLData - 
		Contains POP3 message retrieval synchronization data.
	 */
void TUIDLData::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MessageSourceID );
	BINDPARAM_TCHAR( query, m_Identifier );
	BINDPARAM_BIT( query, m_IsPartial );
	BINDPARAM_TIME( query, m_UIDLDate );
	query.Execute( _T("INSERT INTO UIDLData ")
	               _T("(MessageSourceID,Identifier,IsPartial,UIDLDate) ")
	               _T("VALUES")
	               _T("(?,?,?,?)") );
	m_UIDLID = query.GetLastInsertedID();
}

int TUIDLData::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MessageSourceID );
	BINDPARAM_TCHAR( query, m_Identifier );
	BINDPARAM_BIT( query, m_IsPartial );
	BINDPARAM_TIME( query, m_UIDLDate );
	BINDPARAM_LONG( query, m_UIDLID );
	query.Execute( _T("UPDATE UIDLData ")
	               _T("SET MessageSourceID=?,Identifier=?,IsPartial=?,UIDLDate=? ")
	               _T("WHERE UIDLID=?") );
	return query.GetRowCount();
}

int TUIDLData::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_UIDLID );
	query.Execute( _T("DELETE FROM UIDLData ")
	               _T("WHERE UIDLID=?") );
	return query.GetRowCount();
}

int TUIDLData::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_MessageSourceID );
	BINDCOL_TCHAR( query, m_Identifier );
	BINDCOL_BIT( query, m_IsPartial );
	BINDCOL_TIME( query, m_UIDLDate );
	BINDPARAM_LONG( query, m_UIDLID );
	query.Execute( _T("SELECT MessageSourceID,Identifier,IsPartial,UIDLDate ")
	               _T("FROM UIDLData ")
	               _T("WHERE UIDLID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TUIDLData::GetLongData( CODBCQuery& query )
{
}

void TUIDLData::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_UIDLID );
	BINDCOL_LONG( query, m_MessageSourceID );
	BINDCOL_TCHAR( query, m_Identifier );
	BINDCOL_BIT( query, m_IsPartial );
	BINDCOL_TIME( query, m_UIDLDate );
	query.Execute( _T("SELECT UIDLID,MessageSourceID,Identifier,IsPartial,UIDLDate ")
	               _T("FROM UIDLData ")
	               _T("ORDER BY UIDLID") );
}

/* class TVirusScanStates - 
		Indicates the current state of the virus scanning process.
	 */
void TVirusScanStates::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO VirusScanStates ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_VirusScanStateID = query.GetLastInsertedID();
}

int TVirusScanStates::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_VirusScanStateID );
	query.Execute( _T("UPDATE VirusScanStates ")
	               _T("SET Description=? ")
	               _T("WHERE VirusScanStateID=?") );
	return query.GetRowCount();
}

int TVirusScanStates::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_VirusScanStateID );
	query.Execute( _T("DELETE FROM VirusScanStates ")
	               _T("WHERE VirusScanStateID=?") );
	return query.GetRowCount();
}

int TVirusScanStates::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_VirusScanStateID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM VirusScanStates ")
	               _T("WHERE VirusScanStateID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TVirusScanStates::GetLongData( CODBCQuery& query )
{
}

void TVirusScanStates::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_VirusScanStateID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT VirusScanStateID,Description ")
	               _T("FROM VirusScanStates ")
	               _T("ORDER BY VirusScanStateID") );
}

/* class TServers - */
void TServers::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_RegistrationKey );
	BINDPARAM_LONG( query, m_Status );
	BINDPARAM_LONG( query, m_ReloadConfig );
	BINDPARAM_TIME( query, m_CheckIn );
	BINDPARAM_TCHAR( query, m_SpoolFolder );
	BINDPARAM_TCHAR( query, m_TempFolder );
	BINDPARAM_LONG( query, m_WebStatus );
	BINDPARAM_TIME( query, m_WebCheckIn );	
	query.Execute( _T("INSERT INTO Servers ")
	               _T("(Description,RegistrationKey,Status,ReloadConfig,CheckIn,SpoolFolder,TempFolder,WebStatus,WebCheckIn) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?)") );
	m_ServerID = query.GetLastInsertedID();
}

int TServers::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_RegistrationKey );
	BINDPARAM_LONG( query, m_Status );
	BINDPARAM_LONG( query, m_ReloadConfig );
	BINDPARAM_TIME( query, m_CheckIn );
	BINDPARAM_TCHAR( query, m_SpoolFolder );
	BINDPARAM_TCHAR( query, m_TempFolder );
	BINDPARAM_LONG( query, m_WebStatus );
	BINDPARAM_TIME( query, m_WebCheckIn );	
	BINDPARAM_LONG( query, m_ServerID );	
	query.Execute( _T("UPDATE Servers ")
	               _T("SET Description=?,RegistrationKey=?,Status=?,ReloadConfig=?,CheckIn=?,SpoolFolder=?,TempFolder=?,WebStatus=?,WebCheckIn=? ")
	               _T("WHERE ServerID=?") );
	return query.GetRowCount();
}

int TServers::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ServerID );
	query.Execute( _T("DELETE FROM Servers ")
	               _T("WHERE ServerID=?") );
	return query.GetRowCount();
}

int TServers::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_RegistrationKey );
	BINDCOL_LONG( query, m_Status );
	BINDCOL_LONG( query, m_ReloadConfig );
	BINDCOL_TIME( query, m_CheckIn );
	BINDCOL_TCHAR( query, m_SpoolFolder );
	BINDCOL_TCHAR( query, m_TempFolder );
	BINDCOL_LONG( query, m_WebStatus );
	BINDCOL_TIME( query, m_WebCheckIn );
	BINDPARAM_LONG( query, m_ServerID );
	query.Execute( _T("SELECT Description,RegistrationKey,Status,ReloadConfig,CheckIn,SpoolFolder,TempFolder,WebStatus,WebCheckIn ")
	               _T("FROM Servers ")
	               _T("WHERE ServerID=?") );
	nResult = query.Fetch();
	return nResult;
}

void TServers::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ServerID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_RegistrationKey );
	BINDCOL_LONG( query, m_Status );
	BINDCOL_LONG( query, m_ReloadConfig );
	BINDCOL_TIME( query, m_CheckIn );
	BINDCOL_TCHAR( query, m_SpoolFolder );
	BINDCOL_TCHAR( query, m_TempFolder );
	BINDCOL_LONG( query, m_WebStatus );
	BINDCOL_TIME( query, m_WebCheckIn );	
	query.Execute( _T("SELECT ServerID,Description,RegistrationKey,Status,ReloadConfig,CheckIn,SpoolFolder,TempFolder,WebStatus,WebCheckIn ")
	               _T("FROM Servers ORDER BY Description ") );	              
}

/* class TServerTaskTypes - */
void TServerTaskTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO ServerTaskTypes ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_ServerTaskTypeID = query.GetLastInsertedID();
}

int TServerTaskTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_ServerTaskTypeID );	
	query.Execute( _T("UPDATE ServerTaskTypes ")
	               _T("SET Description=? ")
	               _T("WHERE ServerTaskTypeID=?") );
	return query.GetRowCount();
}

int TServerTaskTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ServerTaskTypeID );
	query.Execute( _T("DELETE FROM ServerTaskTypes ")
	               _T("WHERE ServerTaskTypeID=?") );
	return query.GetRowCount();
}

int TServerTaskTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_ServerTaskTypeID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM ServerTaskTypes ")
	               _T("WHERE ServerTaskTypeID=?") );
	nResult = query.Fetch();
	return nResult;
}

void TServerTaskTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ServerTaskTypeID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT ServerTaskTypeID,Description ")
	               _T("FROM ServerTaskTypes ORDER BY Description ") );	              
}

/* class TServerTasks - */
void TServerTasks::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ServerTaskTypeID );
	BINDPARAM_LONG( query, m_ServerID );
	query.Execute( _T("INSERT INTO ServerTasks ")
	               _T("(ServerTaskTypeID,ServerID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_ServerTaskID = query.GetLastInsertedID();
}

int TServerTasks::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ServerTaskTypeID );
	BINDPARAM_LONG( query, m_ServerID );
	BINDPARAM_LONG( query, m_ServerTaskID );	
	query.Execute( _T("UPDATE ServerTasks ")
	               _T("SET ServerTaskTypeID=?,ServerID=? ")
	               _T("WHERE ServerTaskID=?") );
	return query.GetRowCount();
}

int TServerTasks::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_ServerTaskID );
	query.Execute( _T("DELETE FROM ServerTasks ")
	               _T("WHERE ServerTaskID=?") );
	return query.GetRowCount();
}

int TServerTasks::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_ServerTaskTypeID );
	BINDCOL_LONG( query, m_ServerID );
	BINDPARAM_LONG( query, m_ServerTaskID );
	query.Execute( _T("SELECT ServerTaskTypeID,ServerID ")
	               _T("FROM ServerTasks ")
	               _T("WHERE ServerTaskID=?") );
	nResult = query.Fetch();
	return nResult;
}

void TServerTasks::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_ServerTaskID );
	BINDCOL_LONG( query, m_ServerTaskTypeID );
	BINDCOL_LONG( query, m_ServerID );
	query.Execute( _T("SELECT ServerTaskID,ServerTaskTypeID,ServerID ")
	               _T("FROM ServerTasks ") );	              
}

/* class TVoipServers - */
void TVoipServers::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_VoipServerTypeID );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_Url );
	BINDPARAM_LONG( query, m_Port );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_IsDefault );
	query.Execute( _T("INSERT INTO VoipServers ")
	               _T("(VoipServerTypeID,Description,Url,Port,IsEnabled,IsDefault) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?)") );
	m_VoipServerID = query.GetLastInsertedID();
}

int TVoipServers::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_VoipServerTypeID );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_Url );
	BINDPARAM_LONG( query, m_Port );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_VoipServerID );	
	query.Execute( _T("UPDATE VoipServers ")
	               _T("SET VoipServerTypeID=?,Description=?,Url=?,Port=?,IsEnabled=?,IsDefault=? ")
	               _T("WHERE VoipServerID=?") );
	return query.GetRowCount();
}

int TVoipServers::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_VoipServerID );
	query.Execute( _T("DELETE FROM VoipServers ")
	               _T("WHERE VoipServerID=?") );
	return query.GetRowCount();
}

int TVoipServers::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_VoipServerTypeID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_Url );
	BINDCOL_LONG( query, m_Port );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_VoipServerID );
	query.Execute( _T("SELECT VoipServerTypeID,Description,Url,Port,IsEnabled,IsDefault ")
	               _T("FROM VoipServers ")
	               _T("WHERE VoipServerID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TVoipServers::GetLongData( CODBCQuery& query )
{
}

void TVoipServers::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_VoipServerID );
	BINDCOL_LONG( query, m_VoipServerTypeID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_Url );
	BINDCOL_LONG( query, m_Port );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_IsDefault );
	query.Execute( _T("SELECT VoipServerID,VoipServerTypeID,Description,Url,Port,IsEnabled,IsDefault ")
	               _T("FROM VoipServers ORDER BY IsDefault DESC,Description ") );	              
}

/* class TVoipServerTypes - */
void TVoipServerTypes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	query.Execute( _T("INSERT INTO VoipServerTypes ")
	               _T("(Description) ")
	               _T("VALUES")
	               _T("(?)") );
	m_VoipServerTypeID = query.GetLastInsertedID();
}

int TVoipServerTypes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_VoipServerTypeID );
	query.Execute( _T("UPDATE VoipServerTypes ")
	               _T("SET Description=? ")
	               _T("WHERE VoipServerTypeID=?") );
	return query.GetRowCount();
}

int TVoipServerTypes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_VoipServerTypeID );
	query.Execute( _T("DELETE FROM VoipServerTypes ")
	               _T("WHERE VoipServerTypeID=?") );
	return query.GetRowCount();
}

int TVoipServerTypes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_VoipServerTypeID );
	query.Execute( _T("SELECT Description ")
	               _T("FROM VoipServerTypes ")
	               _T("WHERE VoipServerTypeID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TVoipServerTypes::GetLongData( CODBCQuery& query )
{
}

void TVoipServerTypes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_VoipServerTypeID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute( _T("SELECT VoipServerTypeID,Description ")
	               _T("FROM VoipServerTypes ORDER BY Description") );	              
}

/* class TVoipExtensions - */
void TVoipExtensions::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_VoipServerID );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_Extension );
	BINDPARAM_TCHAR( query, m_Pin );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_IsDefault );
	query.Execute( _T("INSERT INTO VoipExtensions ")
	               _T("(AgentID,VoipServerID,Description,Extension,Pin,IsEnabled,IsDefault) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?)") );
	m_VoipExtensionID = query.GetLastInsertedID();
}

int TVoipExtensions::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_VoipServerID );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_LONG( query, m_Extension );
	BINDPARAM_TCHAR( query, m_Pin );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_VoipExtensionID );	
	query.Execute( _T("UPDATE VoipExtensions ")
	               _T("SET AgentID=?,VoipServerID=?,Description=?,Extension=?,Pin=?,IsEnabled=?,IsDefault=? ")
	               _T("WHERE VoipExtensionID=?") );
	return query.GetRowCount();
}

int TVoipExtensions::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_VoipExtensionID );
	query.Execute( _T("DELETE FROM VoipExtensions ")
	               _T("WHERE VoipExtensionID=?") );
	return query.GetRowCount();
}

int TVoipExtensions::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_VoipServerID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_Extension );
	BINDCOL_TCHAR( query, m_Pin );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_VoipExtensionID );
	query.Execute( _T("SELECT AgentID,VoipServerID,Description,Extension,Pin,IsEnabled,IsDefault ")
	               _T("FROM VoipExtensions ")
	               _T("WHERE VoipExtensionID=? ORDER BY IsDefault DESC,Description") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TVoipExtensions::GetLongData( CODBCQuery& query )
{
}

void TVoipExtensions::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_VoipExtensionID );
	BINDCOL_LONG( query, m_VoipServerID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_LONG( query, m_Extension );
	BINDCOL_TCHAR( query, m_Pin );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_AgentID );
	query.Execute( _T("SELECT VoipExtensionID,VoipServerID,Description,Extension,Pin,IsEnabled,IsDefault ")
	               _T("FROM VoipExtensions WHERE AgentID=? ORDER BY IsDefault DESC,Description") );	              
}

/* class TVoipDialingCodes - */
void TVoipDialingCodes::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_VoipServerID );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_DialingCode );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_IsDefault );
	query.Execute( _T("INSERT INTO VoipDialingCodes ")
	               _T("(VoipServerID,Description,DialingCode,IsEnabled,IsDefault) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?)") );
	m_VoipDialingCodeID = query.GetLastInsertedID();
}

int TVoipDialingCodes::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_VoipServerID );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_DialingCode );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_VoipDialingCodeID );	
	query.Execute( _T("UPDATE VoipDialingCodes ")
	               _T("SET VoipServerID=?,Description=?,DialingCode=?,IsEnabled=?,IsDefault=? ")
	               _T("WHERE VoipDialingCodeID=?") );
	return query.GetRowCount();
}

int TVoipDialingCodes::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_VoipDialingCodeID );
	query.Execute( _T("DELETE FROM VoipDialingCodes ")
	               _T("WHERE VoipDialingCodeID=?") );
	return query.GetRowCount();
}

int TVoipDialingCodes::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_VoipServerID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_DialingCode );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_VoipDialingCodeID );
	query.Execute( _T("SELECT VoipServerID,Description,DialingCode,IsEnabled,IsDefault ")
	               _T("FROM VoipDialingCodes ")
	               _T("WHERE VoipDialingCodeID=? ORDER BY IsDefault DESC,Description ") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TVoipDialingCodes::GetLongData( CODBCQuery& query )
{
}

void TVoipDialingCodes::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_VoipDialingCodeID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_DialingCode );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDCOL_BIT( query, m_IsDefault );
	BINDPARAM_LONG( query, m_VoipServerID );
	query.Execute( _T("SELECT VoipDialingCodeID,Description,DialingCode,IsEnabled,IsDefault ")
	               _T("FROM VoipDialingCodes WHERE VoipServerID=? ORDER BY IsDefault DESC,Description ") );	              
}

/* class TAssignToAgents - 
		Table of Agents used by the routing rules.
	 */
void TAssignToAgents::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_AgentID );
	query.Execute( _T("INSERT INTO AgentRouting ")
	               _T("(RoutingRuleID,AgentID) ")
	               _T("VALUES")
	               _T("(?,?)") );
	m_AgentRoutingID = query.GetLastInsertedID();
}

int TAssignToAgents::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_RoutingRuleID );
	BINDPARAM_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_AgentRoutingID );
	query.Execute( _T("UPDATE AgentRouting ")
	               _T("SET RoutingRuleID=?,AgentID=? ")
	               _T("WHERE AgentRoutingID=?") );
	return query.GetRowCount();
}

int TAssignToAgents::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_AgentRoutingID );
	query.Execute( _T("DELETE FROM AgentRouting ")
	               _T("WHERE AgentRoutingID=?") );
	return query.GetRowCount();
}

int TAssignToAgents::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDCOL_LONG( query, m_AgentID );
	BINDPARAM_LONG( query, m_AgentRoutingID );
	query.Execute( _T("SELECT RoutingRuleID,AgentID ")
	               _T("FROM AgentRouting ")
	               _T("WHERE AgentRoutingID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TAssignToAgents::GetLongData( CODBCQuery& query )
{
}

void TAssignToAgents::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_AgentRoutingID );
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDCOL_LONG( query, m_AgentID );
	query.Execute( _T("SELECT AgentRoutingID,RoutingRuleID,AgentID ")
	               _T("FROM AgentRouting ")
	               _T("ORDER BY AgentRoutingID") );
}

/* class TTimeZones - 
		Contains Time Zones.
	*/
	
void TTimeZones::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_DisplayName );
	BINDPARAM_TCHAR( query, m_StandardName );
	BINDPARAM_LONG( query, m_UTCOffset );
	query.Execute( _T("INSERT INTO TimeZones ")
	               _T("(DisplayName,StandardName,UTCOffset) ")
	               _T("VALUES")
	               _T("(?,?,?)") );
	m_TimeZoneID = query.GetLastInsertedID();
}

int TTimeZones::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_DisplayName );
	BINDPARAM_TCHAR( query, m_StandardName );
	BINDPARAM_LONG( query, m_UTCOffset );
	BINDPARAM_LONG( query, m_TimeZoneID );
	query.Execute( _T("UPDATE TimeZones ")
	               _T("SET DisplayName=?,StandardName=?,UTCOffset=? ")
	               _T("WHERE TimeZoneID=?") );
	return query.GetRowCount();
}

int TTimeZones::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_TimeZoneID );
	query.Execute( _T("DELETE FROM TimeZones ")
	               _T("WHERE TimeZoneID=?") );
	return query.GetRowCount();
}

int TTimeZones::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_DisplayName );
	BINDCOL_TCHAR( query, m_StandardName );
	BINDCOL_LONG( query, m_UTCOffset );
	BINDPARAM_LONG( query, m_TimeZoneID );
	query.Execute( _T("SELECT DisplayName,StandardName,UTCOffset ")
	               _T("FROM TimeZones ")
	               _T("WHERE TimeZoneID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TTimeZones::GetLongData( CODBCQuery& query )
{
}

void TTimeZones::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_TimeZoneID );
	BINDCOL_TCHAR( query, m_DisplayName );
	BINDCOL_TCHAR( query, m_StandardName );
	BINDCOL_LONG( query, m_UTCOffset );
	query.Execute( _T("SELECT TimeZoneID,DisplayName,StandardName,UTCOffset ")
	               _T("FROM TimeZones ")
	               _T("ORDER BY TimeZoneID") );
}

/* class TDictionary - 
		Contains Dictionaries.
	*/
	
void TDictionary::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_LanguageID );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_TlxFile );
	BINDPARAM_TCHAR( query, m_ClxFile );
	BINDPARAM_BIT( query, m_IsEnabled );
	query.Execute( _T("INSERT INTO Dictionary ")
	               _T("(LanguageID,Description,TlxFile,ClxFile,IsEnabled) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?)") );
	m_DictionaryID = query.GetLastInsertedID();
}

int TDictionary::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_LanguageID );
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_TlxFile );
	BINDPARAM_TCHAR( query, m_ClxFile );
	BINDPARAM_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_DictionaryID );
	query.Execute( _T("UPDATE Dictionary ")
	               _T("SET LanguageID=?,Description=?,TlxFile=?,ClxFile=?,IsEnabled=? ")
	               _T("WHERE DictionaryID=?") );
	return query.GetRowCount();
}

int TDictionary::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_DictionaryID );
	query.Execute( _T("DELETE FROM Dictionary ")
	               _T("WHERE DictionaryID=?") );
	return query.GetRowCount();
}

int TDictionary::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_LanguageID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_TlxFile );
	BINDCOL_TCHAR( query, m_ClxFile );
	BINDCOL_BIT( query, m_IsEnabled );
	BINDPARAM_LONG( query, m_DictionaryID );
	query.Execute( _T("SELECT LanguageID,Description,TlxFile,ClxFile,IsEnabled ")
	               _T("FROM Dictionary ")
	               _T("WHERE DictionaryID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TDictionary::GetLongData( CODBCQuery& query )
{
}

void TDictionary::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_DictionaryID );
	BINDCOL_TCHAR( query, m_LanguageID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_TlxFile );
	BINDCOL_TCHAR( query, m_ClxFile );
	BINDCOL_BIT( query, m_IsEnabled );
	query.Execute( _T("SELECT DictionaryID,LanguageID,Description,TlxFile,ClxFile,IsEnabled ")
	               _T("FROM Dictionary ")
	               _T("ORDER BY Description") );
}

/* class TMessageTracking - 
		Contains Message Tracking Data.
	*/
	
void TMessageTracking::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MessageID );
	BINDPARAM_LONG( query, m_MessageSourceID );
	BINDPARAM_TCHAR( query, m_HeadMsgID );
	BINDPARAM_TCHAR( query, m_HeadInReplyTo );
	BINDPARAM_TCHAR( query, m_HeadReferences );
	query.Execute( _T("INSERT INTO Dictionary ")
	               _T("(MessageID,MessageSourceID,HeadMsgID,HeadInReplyTo,HeadReferences) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?)") );
	m_MessageTrackingID = query.GetLastInsertedID();
}

int TMessageTracking::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MessageID );
	BINDPARAM_LONG( query, m_MessageSourceID );
	BINDPARAM_TCHAR( query, m_HeadMsgID );
	BINDPARAM_TCHAR( query, m_HeadInReplyTo );
	BINDPARAM_TCHAR( query, m_HeadReferences );
	BINDPARAM_LONG( query, m_MessageTrackingID );
	query.Execute( _T("UPDATE MessageTracking ")
	               _T("SET MessageID=?,MessageSourceID=?,HeadMsgID=?,HeadInReplyTo=?,HeadReferences=? ")
	               _T("WHERE MessageTrackingID=?") );
	return query.GetRowCount();
}

int TMessageTracking::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_MessageTrackingID );
	query.Execute( _T("DELETE FROM MessageTracking ")
	               _T("WHERE MessageTrackingID=?") );
	return query.GetRowCount();
}

int TMessageTracking::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_LONG( query, m_MessageID );
	BINDCOL_LONG( query, m_MessageSourceID );
	BINDCOL_TCHAR( query, m_HeadMsgID );
	BINDCOL_TCHAR( query, m_HeadInReplyTo );
	BINDCOL_TCHAR( query, m_HeadReferences );
	BINDPARAM_LONG( query, m_MessageTrackingID );
	query.Execute( _T("SELECT MessageID,MessageSourceID,HeadMsgID,HeadInReplyTo,HeadReferences ")
	               _T("FROM MessageTracking ")
	               _T("WHERE MessageTrackingID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TMessageTracking::GetLongData( CODBCQuery& query )
{
}

void TMessageTracking::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_MessageTrackingID );
	BINDCOL_LONG( query, m_MessageID );
	BINDCOL_LONG( query, m_MessageSourceID );
	BINDCOL_TCHAR( query, m_HeadMsgID );
	BINDCOL_TCHAR( query, m_HeadInReplyTo );
	BINDCOL_TCHAR( query, m_HeadReferences );
	query.Execute( _T("SELECT MessageTrackingID,MessageID,MessageSourceID,HeadMsgID,HeadInReplyTo,HeadReferences ")
	               _T("FROM MessageTracking ")
	               _T("ORDER BY MessageTrackingID") );
}

/* class TOAuthHosts - 
		Contains OAuth Hosts.
	*/
	
void TOAuthHosts::Insert( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_AuthEndPoint );
	BINDPARAM_TCHAR( query, m_TokenEndPoint );
	BINDPARAM_TCHAR( query, m_ClientID );
	BINDPARAM_TCHAR( query, m_ClientSecret );
	BINDPARAM_TCHAR( query, m_Scope );
	query.Execute( _T("INSERT INTO OAuthHosts ")
	               _T("(Description,AuthEndPoint,TokenEndPoint,ClientID,ClientSecret,Scope) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?)") );
	m_OAuthHostID = query.GetLastInsertedID();
}

int TOAuthHosts::Update( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_TCHAR( query, m_Description );
	BINDPARAM_TCHAR( query, m_AuthEndPoint );
	BINDPARAM_TCHAR( query, m_TokenEndPoint );
	BINDPARAM_TCHAR( query, m_ClientID );
	BINDPARAM_TCHAR( query, m_ClientSecret );
	BINDPARAM_TCHAR( query, m_Scope );
	BINDPARAM_LONG( query, m_OAuthHostID );
	query.Execute( _T("UPDATE OAuthHosts ")
	               _T("SET Description=?,AuthEndPoint=?,TokenEndPoint=?,ClientID=?,ClientSecret=?,Scope=? ")
	               _T("WHERE OAuthHostID=?") );
	return query.GetRowCount();
}

int TOAuthHosts::Delete( CODBCQuery& query )
{
	query.Initialize();
	BINDPARAM_LONG( query, m_OAuthHostID );
	query.Execute( _T("DELETE FROM OAuthHosts ")
	               _T("WHERE OAuthHostID=?") );
	return query.GetRowCount();
}

int TOAuthHosts::Query( CODBCQuery& query )
{
	int nResult;
	query.Initialize();
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_AuthEndPoint );
	BINDCOL_TCHAR( query, m_TokenEndPoint );
	BINDCOL_TCHAR( query, m_ClientID );
	BINDCOL_TCHAR( query, m_ClientSecret );
	BINDCOL_TCHAR( query, m_Scope );
	BINDPARAM_LONG( query, m_OAuthHostID );
	query.Execute( _T("SELECT Description,AuthEndPoint,TokenEndPoint,ClientID,ClientSecret,Scope ")
	               _T("FROM OAuthHosts ")
	               _T("WHERE OAuthHostID=?") );
	nResult = query.Fetch();
	if( nResult == S_OK )
	{
		GetLongData( query );
	}
	return nResult;
}

void TOAuthHosts::GetLongData( CODBCQuery& query )
{
}

void TOAuthHosts::PrepareList( CODBCQuery& query )
{
	query.Initialize();
	BINDCOL_LONG( query, m_OAuthHostID );
	BINDCOL_TCHAR( query, m_Description );
	BINDCOL_TCHAR( query, m_AuthEndPoint );
	BINDCOL_TCHAR( query, m_TokenEndPoint );
	BINDCOL_TCHAR( query, m_ClientID );
	BINDCOL_TCHAR( query, m_ClientSecret );
	BINDCOL_TCHAR( query, m_Scope );
	query.Execute( _T("SELECT OAuthHostID,Description,AuthEndPoint,TokenEndPoint,ClientID,ClientSecret,Scope ")
	               _T("FROM OAuthHosts ")
	               _T("ORDER BY Description") );
}