/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/ArchiveTables.cpp,v 1.2.2.2 2005/12/07 20:01:47 markm Exp $
||
||
||                                         
||  COMMENTS:	Archive Table Functions
||              
\\*************************************************************************/

// NOTE! This code is in development and not compiled in the library yet!

#include "stdafx.h"
#include "ArchiveFns.h"
#include "QueryClasses.h"
#include "RegistryFns.h"
#include "StringFns.h"
#include "DebugReporter.h"
#include ".\MailStreamEngine.h"

// Helper macros - note that they all depend on a local variable being defined: 
// int ret;

#define GETBYTE(x)   if( (ret=arc.GetByteField( x )) != Arc_Success ) return ret
#define GETDECIMAL(x)if( (ret=arc.GetDecimalField( x )) != Arc_Success ) return ret
#define GETLONG(x)   if( (ret=arc.GetLongField( x )) != Arc_Success ) return ret
#define GETDATE(x)   if( (ret=arc.GetDateField( x, x##Len )) != Arc_Success ) return ret
#define GETCHAR(x,y) if( (ret=arc.GetCharField( x, y )) != Arc_Success ) return ret
#define GETTEXT(x)   if( (ret=arc.GetTextField( x, x##Len, x##Allocated )) != Arc_Success ) return ret
#define GETFILE(x,y,z,o,p)   if( (ret=arc.GetFileField( x, y, z, o, p )) != Arc_Success ) return ret

#define PUTBYTE(x)   if( (ret=arc.PutByteField( x )) != Arc_Success ) return ret
#define PUTLONG(x)   if( (ret=arc.PutLongField( x )) != Arc_Success ) return ret
#define PUTDATE(x)   if( (ret=arc.PutDateField( x, x##Len )) != Arc_Success ) return ret
#define PUTCHAR(x)   if( (ret=arc.PutCharField( x )) != Arc_Success ) return ret
#define PUTTEXT(x)   if( (ret=arc.PutTextField( x, x##Len )) != Arc_Success ) return ret
#define PUTFILE(x,y,z)   if( (ret=arc.PutFileField( x, y, z )) != Arc_Success ) return ret


////////////////////////////////////////////////////////////////////////////////
// 
// StuffAccessControlRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAccessControlRecord( CArchiveFile& arc, long ID )
{
	TAccessControl acl;
	int ret;

	// Set the object ID
	acl.m_AccessControlID = ID;

	// Query from the databse
	if( acl.Query( arc.GetQuery() ) == S_OK )
	{
		// Stuff the data
		PUTBYTE( acl.m_AccessLevel );
		PUTLONG( acl.m_AgentID );
		PUTLONG( acl.m_GroupID );
		PUTLONG( acl.m_ObjectID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAccessControlRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAccessControlRecord( CArchiveFile& arc, long ID )
{
	TAccessControl acl;
	int ret = Arc_Success;

	// Set the object ID
	acl.m_AccessControlID = ID;

	GETBYTE( acl.m_AccessLevel );
	GETLONG( acl.m_AgentID );
	arc.TranslateID( TABLEID_Agents, acl.m_AgentID );
	GETLONG( acl.m_GroupID );
	arc.TranslateID( TABLEID_Groups, acl.m_GroupID );
	GETLONG( acl.m_ObjectID );
	arc.TranslateID( TABLEID_Objects, acl.m_ObjectID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AccessControl].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( acl.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), acl.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), acl.m_GroupID );
		BINDPARAM_LONG( arc.GetQuery(), acl.m_ObjectID );
		BINDCOL_LONG( arc.GetQuery(), acl.m_AccessControlID );
		arc.GetQuery().Execute( _T("SELECT AccessControlID FROM AccessControl ")
								_T("WHERE AgentID=? AND GroupID=? AND ObjectID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( acl.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//acl.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), acl.m_AccessControlID );
			BINDPARAM_LONG( arc.GetQuery(), acl.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), acl.m_GroupID );
			BINDPARAM_TINYINT( arc.GetQuery(), acl.m_AccessLevel );
			BINDPARAM_LONG( arc.GetQuery(), acl.m_ObjectID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AccessControl ON INSERT INTO AccessControl ")
						_T("(AccessControlID,AgentID,GroupID,AccessLevel,ObjectID) ")
						_T("VALUES")
						_T("(?,?,?,?,?) SET IDENTITY_INSERT AccessControl OFF") );
		}
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffAgentGroupingRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAgentGroupingRecord( CArchiveFile& arc, long ID )
{
	TAgentGroupings ag;
	int ret;

	// Set the object ID
	ag.m_AgentGroupingID = ID;

	// Query from the databse
	if( ag.Query( arc.GetQuery() ) == S_OK )
	{
		// Stuff the data
		PUTLONG( ag.m_AgentID );
		PUTLONG( ag.m_GroupID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAgentGroupingRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAgentGroupingRecord( CArchiveFile& arc, long ID )
{
	TAgentGroupings ag;
	int ret = Arc_Success;

	ag.m_AgentGroupingID = ID;
	GETLONG( ag.m_AgentID );
	arc.TranslateID( TABLEID_Agents, ag.m_AgentID );
	GETLONG( ag.m_GroupID );
	arc.TranslateID( TABLEID_Groups, ag.m_GroupID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AgentGroupings].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ag.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), ag.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), ag.m_GroupID );
		BINDCOL_LONG( arc.GetQuery(), ag.m_AgentGroupingID );
		arc.GetQuery().Execute( _T("SELECT AgentGroupingID FROM AgentGroupings ")
								_T("WHERE AgentID=? AND GroupID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
				return ( ag.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
		else
		{
			//ag.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ag.m_AgentGroupingID );
			BINDPARAM_LONG( arc.GetQuery(), ag.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), ag.m_GroupID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AgentGroupings ON INSERT INTO AgentGroupings ")
						_T("(AgentGroupingID,AgentID,GroupID) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT AgentGroupings OFF") );
		}
	}


	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffAgentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAgentRecord( CArchiveFile& arc, long ID )
{
	TAgents agent;
	int ret;

	// Set the object ID
	agent.m_AgentID = ID;

	// Query from the databse
	if( agent.Query( arc.GetQuery() ) == S_OK )
	{
		// Stuff the data
		arc.PutCharField(agent.m_LoginName);
		arc.PutCharField( agent.m_Password );
		arc.PutCharField( agent.m_Name );
		arc.PutByteField( agent.m_IsEnabled );
		PUTLONG( agent.m_ObjectID );
		PUTLONG( agent.m_DefaultAgentAddressID );
		PUTLONG( agent.m_MaxReportRowsPerPage );
		arc.PutByteField( agent.m_StdResponseApprovalsRequired );
		arc.PutByteField( agent.m_MessageApprovalsRequired );
		arc.PutByteField( agent.m_QuoteMsgInReply );
		arc.PutCharField( agent.m_ReplyQuotedPrefix );
		arc.PutCharField( agent.m_NTDomain );
		PUTLONG( agent.m_IsNTUser );
		PUTLONG( agent.m_StyleSheetID );
		arc.PutByteField( agent.m_IsDeleted );
		arc.PutByteField( agent.m_UseIM );
		arc.PutByteField( agent.m_NewMessageFormat );
		PUTLONG( agent.m_EscalateToAgentID );
		arc.PutByteField( agent.m_RequireGetOldest );
		arc.PutByteField( agent.m_HasLoggedIn );
		PUTLONG( agent.m_DefaultSignatureID );
		arc.PutByteField( agent.m_ForceSpellCheck );
		arc.PutByteField( agent.m_SignatureTopReply );
		arc.PutByteField( agent.m_ShowMessagesInbound );
		arc.PutByteField( agent.m_UsePreviewPane );
		arc.PutByteField( agent.m_CloseTicket );
		arc.PutByteField( agent.m_RouteToInbox );
		PUTLONG( agent.m_OutboxHoldTime );
		PUTBYTE( agent.m_UseMarkAsRead );
		PUTLONG( agent.m_MarkAsReadSeconds );
		PUTLONG( agent.m_OutboundApprovalFromID );
		arc.PutByteField( agent.m_UseAutoFill );
		PUTLONG( agent.m_DefaultTicketBoxID );
		PUTLONG( agent.m_DefaultTicketDblClick );
		PUTLONG( agent.m_ReadReceipt );
		PUTLONG( agent.m_DeliveryConfirmation );
		PUTLONG( agent.m_StatusID );
		arc.PutCharField( agent.m_StatusText );
		PUTLONG( agent.m_AutoStatusTypeID );
		PUTLONG( agent.m_AutoStatusMin );
		PUTLONG( agent.m_LoginStatusID );
		PUTLONG( agent.m_LogoutStatusID );
		arc.PutCharField( agent.m_OnlineText );
		arc.PutCharField( agent.m_AwayText );
		arc.PutCharField( agent.m_NotAvailText );
		arc.PutCharField( agent.m_DndText );
		arc.PutCharField( agent.m_OfflineText );
		arc.PutCharField( agent.m_OooText );
		PUTLONG( agent.m_TimeZoneID );
		PUTLONG( agent.m_DictionaryID );
		arc.PutByteField( agent.m_DictionaryPrompt );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAgentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAgentRecord( CArchiveFile& arc, long ID )
{
	TAgents agent;
	int ret = Arc_Success;

	agent.m_AgentID = ID;
	
	GETCHAR( agent.m_LoginName, AGENTS_LOGINNAME_LENGTH );
	GETCHAR( agent.m_Password, AGENTS_PASSWORD_LENGTH );
	GETCHAR( agent.m_Name, AGENTS_NAME_LENGTH );
	GETBYTE( agent.m_IsEnabled );
	GETLONG( agent.m_ObjectID );
	arc.TranslateID( TABLEID_Objects,agent.m_ObjectID );
	GETLONG( agent.m_DefaultAgentAddressID );	// This is resolved in FixupCircularDependencies
	GETLONG( agent.m_MaxReportRowsPerPage );
	GETBYTE( agent.m_StdResponseApprovalsRequired );
	GETBYTE( agent.m_MessageApprovalsRequired );
	GETBYTE( agent.m_QuoteMsgInReply );
	GETCHAR( agent.m_ReplyQuotedPrefix, AGENTS_REPLYQUOTEDPREFIX_LENGTH );
	GETCHAR( agent.m_NTDomain, AGENTS_NTDOMAIN_LENGTH );
	GETLONG( agent.m_IsNTUser );
	GETLONG( agent.m_StyleSheetID );			// These IDs are static
	GETBYTE( agent.m_IsDeleted );
	GETBYTE( agent.m_UseIM );
	GETBYTE( agent.m_NewMessageFormat );
	GETLONG( agent.m_EscalateToAgentID );
	GETBYTE( agent.m_RequireGetOldest );
	GETBYTE( agent.m_HasLoggedIn );
	GETLONG( agent.m_DefaultSignatureID );		// This is resolved in FixupCircularDependencies
	GETBYTE( agent.m_ForceSpellCheck );
	GETBYTE( agent.m_SignatureTopReply );
	GETBYTE( agent.m_ShowMessagesInbound );
	GETBYTE( agent.m_UsePreviewPane );
	GETBYTE( agent.m_CloseTicket );
	GETBYTE( agent.m_RouteToInbox );
	GETLONG( agent.m_OutboxHoldTime );
	GETBYTE( agent.m_UseMarkAsRead );
	GETLONG( agent.m_MarkAsReadSeconds );
	GETLONG( agent.m_OutboundApprovalFromID );
	GETBYTE( agent.m_UseAutoFill );
	GETLONG( agent.m_DefaultTicketBoxID );
	GETLONG( agent.m_DefaultTicketDblClick );
	GETLONG( agent.m_ReadReceipt );
	GETLONG( agent.m_DeliveryConfirmation );
	GETLONG( agent.m_StatusID );
	GETCHAR( agent.m_StatusText, AGENTS_STATUS_TEXT_LENGTH );
	GETLONG( agent.m_AutoStatusTypeID );
	GETLONG( agent.m_AutoStatusMin );
	GETLONG( agent.m_LoginStatusID );
	GETLONG( agent.m_LogoutStatusID );
	GETCHAR( agent.m_OnlineText, AGENTS_STATUS_TEXT_LENGTH );
	GETCHAR( agent.m_AwayText, AGENTS_STATUS_TEXT_LENGTH );
	GETCHAR( agent.m_NotAvailText, AGENTS_STATUS_TEXT_LENGTH );
	GETCHAR( agent.m_DndText, AGENTS_STATUS_TEXT_LENGTH );
	GETCHAR( agent.m_OfflineText, AGENTS_STATUS_TEXT_LENGTH );
	GETCHAR( agent.m_OooText, AGENTS_STATUS_TEXT_LENGTH );
	GETLONG( agent.m_TimeZoneID );
	GETLONG( agent.m_DictionaryID );
	GETBYTE( agent.m_DictionaryPrompt );
			
	// check if record already exists
	arc.GetQuery().Initialize();
	BINDPARAM_TCHAR( arc.GetQuery(), agent.m_LoginName );
	BINDPARAM_TCHAR( arc.GetQuery(), agent.m_Name );
	BINDCOL_LONG( arc.GetQuery(), agent.m_AgentID );
	arc.GetQuery().Execute( _T("SELECT AgentID FROM Agents ")
							_T("WHERE (LoginName=? AND Name=?)") );

	if( arc.GetQuery().Fetch() == S_OK )
	{
		// Record exists, we now have the new ID
		ret = ( agent.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
	}
	else
	{
		//agent.Insert( arc.GetQuery() );

			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), agent.m_AgentID );
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_LoginName );
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_Password );
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_Name );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_IsEnabled );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_ObjectID );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_DefaultAgentAddressID );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_MaxReportRowsPerPage );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_StdResponseApprovalsRequired );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_MessageApprovalsRequired );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_QuoteMsgInReply );
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_ReplyQuotedPrefix );
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_NTDomain );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_IsNTUser );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_StyleSheetID );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_IsDeleted );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_UseIM );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_NewMessageFormat );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_EscalateToAgentID );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_RequireGetOldest );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_HasLoggedIn );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_DefaultSignatureID );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_ForceSpellCheck );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_SignatureTopReply );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_ShowMessagesInbound );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_UsePreviewPane );
			BINDPARAM_TINYINT( arc.GetQuery(), agent.m_CloseTicket );
			BINDPARAM_TINYINT( arc.GetQuery(), agent.m_RouteToInbox );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_OutboxHoldTime );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_UseMarkAsRead );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_MarkAsReadSeconds);
			BINDPARAM_LONG( arc.GetQuery(), agent.m_OutboundApprovalFromID );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_UseAutoFill );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_DefaultTicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_DefaultTicketDblClick );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_ReadReceipt );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_DeliveryConfirmation );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_StatusID );
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_StatusText );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_AutoStatusTypeID );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_AutoStatusMin );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_LoginStatusID );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_LogoutStatusID );
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_OnlineText );	
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_AwayText );	
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_NotAvailText );	
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_DndText );	
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_OfflineText );	
			BINDPARAM_TCHAR( arc.GetQuery(), agent.m_OooText );	
			BINDPARAM_LONG( arc.GetQuery(), agent.m_TimeZoneID );
			BINDPARAM_LONG( arc.GetQuery(), agent.m_DictionaryID );
			BINDPARAM_BIT( arc.GetQuery(), agent.m_DictionaryPrompt );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Agents ON INSERT INTO Agents ")
	               _T("(AgentID,LoginName,Password,Name,IsEnabled,ObjectID,DefaultAgentAddressID, ")
				   _T("MaxReportRowsPerPage,StdResponseApprovalsRequired,MessageApprovalsRequired, ")
				   _T("QuoteMsgInReply,ReplyQuotedPrefix,NTDomain,IsNTUser,StyleSheetID,IsDeleted,UseIM, ")
				   _T("NewMessageFormat,EscalateToAgentID,RequireGetOldest,HasLoggedIn,DefaultSignatureID, ")
				   _T("ForceSpellCheck,SignatureTopReply,ShowMessagesInbound,UsePreviewPane,CloseTicket, ")
				   _T("RouteToInbox,OutboxHoldTime,UseMarkAsRead,MarkAsReadSeconds,OutboundApprovalFromID, ")
				   _T("UseAutoFill,DefaultTicketBoxID,DefaultTicketDblClick,ReadReceipt,DeliveryConfirmation, ")
				   _T("StatusID,StatusText,AutoStatusTypeID,AutoStatusMin,LoginStatusID,LogoutStatusID,")
				   _T("OnlineText,AwayText,NotAvailText,DndText,OfflineText,OooText,TimeZoneID,DictionaryID,DictionaryPrompt) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT Agents OFF") );

	}

	// save the new ID in a map
	//arc.AddIDMapping( TABLEID_Agents, ID, agent.m_AgentID );
	//arc.AddActualObjectID( agent.m_ObjectID, agent.m_AgentID );

	return ret;
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffAlertEventRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAlertEventRecord( CArchiveFile& arc, long ID )
{
	TAlertEvents ae;
	int ret;

	// Set the object ID
	ae.m_AlertEventID = ID;

	// Query from the databse
	if( ae.Query( arc.GetQuery() ) == S_OK )
	{
		// Stuff the data
		PUTCHAR( ae.m_Description );
		PUTBYTE( ae.m_NeedTicketBoxID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAlertEventRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAlertEventRecord( CArchiveFile& arc, long ID )
{
	TAlertEvents ae;
	int ret = Arc_Success;

	ae.m_AlertEventID = ID;

	GETCHAR( ae.m_Description, ALERTEVENTS_DESCRIPTION_LENGTH );
	GETBYTE( ae.m_NeedTicketBoxID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AlertEvents].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ae.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), ae.m_Description );
		BINDCOL_LONG( arc.GetQuery(), ae.m_AlertEventID );
		arc.GetQuery().Execute( _T("SELECT AlertEventID FROM AlertEvents ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ae.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ae.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ae.m_AlertEventID );
			BINDPARAM_TCHAR( arc.GetQuery(), ae.m_Description );
			BINDPARAM_BIT( arc.GetQuery(), ae.m_NeedTicketBoxID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AlertEvents ON INSERT INTO AlertEvents ")
						_T("(AlertEventID,Description,NeedTicketBoxID) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT AlertEvents OFF") );

		}
	}

	return ret;
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffAlertMethodRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAlertMethodRecord( CArchiveFile& arc, long ID )
{
	TAlertMethods am;
	int ret;

	// Set the object ID
	am.m_AlertMethodID = ID;

	// Query from the databse
	if( am.Query( arc.GetQuery() ) == S_OK )
	{
		// Stuff the data
		PUTCHAR( am.m_Description );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAlertMethodRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAlertMethodRecord( CArchiveFile& arc, long ID )
{
	TAlertMethods am;
	int ret = Arc_Success;

	am.m_AlertMethodID = ID;

	GETCHAR( am.m_Description, ALERTMETHODS_DESCRIPTION_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AlertMethods].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( am.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), am.m_Description );
		BINDCOL_LONG( arc.GetQuery(), am.m_AlertMethodID );
		arc.GetQuery().Execute( _T("SELECT AlertMethodID FROM AlertMethods ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( am.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//am.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), am.m_AlertMethodID );
			BINDPARAM_TCHAR( arc.GetQuery(), am.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AlertMethods ON INSERT INTO AlertMethods ")
						_T("(m_AlertMethodID,Description) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT AlertMethods OFF") );
		}
	}

	return ret;	 
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffAlertMsgRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAlertMsgRecord( CArchiveFile& arc, long ID )
{
	TAlertMsgs am;
	int ret;

	// Set the object ID
	am.m_AlertMsgID = ID;

	// Query from the databse
	if( am.Query( arc.GetQuery() ) == S_OK )
	
	{
		// Stuff the data
		PUTLONG( am.m_AlertEventID );
		PUTLONG( am.m_AgentID );
		PUTBYTE( am.m_Viewed);
		PUTDATE( am.m_DateCreated );
		PUTTEXT( am.m_Body );
		PUTLONG( am.m_TicketID );
		PUTCHAR( am.m_Subject );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAlertMsgRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAlertMsgRecord( CArchiveFile& arc, long ID )
{
	TAlertMsgs am;
	int ret = Arc_Success;

	am.m_AlertMsgID = ID;

	GETLONG( am.m_AlertEventID );
	GETLONG( am.m_AgentID );
	arc.TranslateID( TABLEID_Agents, am.m_AgentID );
	GETBYTE( am.m_Viewed);
	GETDATE( am.m_DateCreated );
	GETTEXT( am.m_Body );
	GETLONG( am.m_TicketID );
	GETCHAR( am.m_Subject, 255 );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AlertMsgs].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( am.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), am.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), am.m_AlertEventID );
		BINDPARAM_TIME( arc.GetQuery(), am.m_DateCreated );
		BINDPARAM_LONG( arc.GetQuery(), am.m_TicketID );
		BINDPARAM_TCHAR( arc.GetQuery(), am.m_Subject );
		BINDCOL_LONG( arc.GetQuery(), am.m_AlertMsgID );
		arc.GetQuery().Execute( _T("SELECT AlertMsgID FROM AlertMsgs ")
								_T("WHERE AgentID=? AND AlertEventID=? AND DateCreated=? AND TicketID=? AND Subject=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( am.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//am.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), am.m_AlertMsgID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_AlertEventID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_TicketID );
			BINDPARAM_TINYINT( arc.GetQuery(), am.m_Viewed );
			BINDPARAM_TIME( arc.GetQuery(), am.m_DateCreated );
			BINDPARAM_TCHAR( arc.GetQuery(), am.m_Subject );
			BINDPARAM_TEXT( arc.GetQuery(), am.m_Body );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AlertMsgs ON INSERT INTO AlertMsgs ")
						_T("(AlertMsgID,AlertEventID,AgentID,TicketID,Viewed,DateCreated,Subject,Body) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?) SET IDENTITY_INSERT AlertMsgs OFF") );
		}
	}

	return ret;	 
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffAlertRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAlertRecord( CArchiveFile& arc, long ID )
{
	TAlerts alert;
	int ret;
	alert.m_AlertID = ID;

	// Query from the databse
	if( alert.Query( arc.GetQuery() ) == S_OK )
	{
		// Stuff the data
		PUTLONG( alert.m_AlertEventID );
		PUTLONG( alert.m_AlertMethodID );
		PUTLONG( alert.m_AgentID );
		PUTLONG( alert.m_GroupID );
		PUTLONG( alert.m_TicketBoxID );
		PUTCHAR( alert.m_EmailAddress );
		PUTCHAR( alert.m_FromEmailAddress );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAlertRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAlertRecord( CArchiveFile& arc, long ID )
{
	TAlerts alert;
	int ret = Arc_Success;

	alert.m_AlertID = ID;

	GETLONG( alert.m_AlertEventID );
	GETLONG( alert.m_AlertMethodID );
	GETLONG( alert.m_AgentID );
	arc.TranslateID( TABLEID_Agents, alert.m_AgentID );
	GETLONG( alert.m_GroupID );
	arc.TranslateID( TABLEID_Groups, alert.m_GroupID );
	GETLONG( alert.m_TicketBoxID );
	arc.TranslateID( TABLEID_TicketBoxes, alert.m_TicketBoxID );
	GETCHAR( alert.m_EmailAddress, ALERTS_EMAILADDRESS_LENGTH );
	GETCHAR( alert.m_FromEmailAddress, ALERTS_FROMEMAILADDRESS_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Alerts].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( alert.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), alert.m_AlertEventID );
		BINDPARAM_LONG( arc.GetQuery(), alert.m_AlertMethodID );
		BINDPARAM_LONG( arc.GetQuery(), alert.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), alert.m_GroupID );
		BINDPARAM_LONG( arc.GetQuery(), alert.m_TicketBoxID );
		BINDCOL_LONG( arc.GetQuery(), alert.m_AlertID );
		arc.GetQuery().Execute( _T("SELECT AlertID FROM Alerts ")
								_T("WHERE AlertEventID=? AND AlertMethodID=? AND ")
								_T("AgentID=? AND GroupID=? AND TicketBoxID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( alert.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//alert.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), alert.m_AlertID );
			BINDPARAM_LONG( arc.GetQuery(), alert.m_AlertEventID );
			BINDPARAM_LONG( arc.GetQuery(), alert. m_AlertMethodID );
			BINDPARAM_LONG( arc.GetQuery(), alert.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), alert.m_GroupID );
			BINDPARAM_LONG( arc.GetQuery(), alert.m_TicketBoxID );
			BINDPARAM_TCHAR( arc.GetQuery(), alert.m_EmailAddress );
			BINDPARAM_TCHAR( arc.GetQuery(), alert.m_FromEmailAddress );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Alerts ON INSERT INTO Alerts ")
						_T("(AlertID,AlertEventID,AlertMethodID,AgentID,GroupID,TicketBoxID,EmailAddress,FromEmailAddress) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?) SET IDENTITY_INSERT Alerts OFF") );
		}
	}

	return ret;	 
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffContactNoteRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffContactNoteRecord( CArchiveFile& arc, long ID )
{
	TContactNotes cn;
	int ret;
	cn.m_ContactNoteID = ID;

	// Query from the databse
	if( cn.Query( arc.GetQuery() ) == S_OK )
	{
		// Stuff the data
		PUTLONG( cn.m_ContactID );
		PUTLONG( cn.m_AgentID );
		PUTDATE( cn.m_DateCreated );
		PUTTEXT( cn.m_Note );
		PUTBYTE( cn.m_IsVoipNote );
		PUTDATE( cn.m_StartTime );
		PUTDATE( cn.m_StopTime );
		PUTCHAR( cn.m_ElapsedTime );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveContactNoteRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveContactNoteRecord( CArchiveFile& arc, long ID )
{
	TContactNotes cn;
	int ret = Arc_Success;

	cn.m_ContactNoteID = ID;

	GETLONG( cn.m_ContactID );
	arc.TranslateID( TABLEID_Contacts, cn.m_ContactID );
	GETLONG( cn.m_AgentID );
	arc.TranslateID( TABLEID_Agents, cn.m_AgentID );
	GETDATE( cn.m_DateCreated );
	GETTEXT( cn.m_Note );
	GETBYTE( cn.m_IsVoipNote );
	GETDATE( cn.m_StartTime );
	GETDATE( cn.m_StopTime );
	GETCHAR( cn.m_ElapsedTime, CONTACTS_COMPANYNAME_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ContactNotes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( cn.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), cn.m_ContactID );
		BINDPARAM_LONG( arc.GetQuery(), cn.m_AgentID );
		BINDPARAM_TIME( arc.GetQuery(), cn.m_DateCreated );
		BINDCOL_LONG( arc.GetQuery(), cn.m_ContactNoteID );
		arc.GetQuery().Execute( _T("SELECT ContactNoteID FROM ContactNotes ")
								_T("WHERE ContactID=? AND AgentID=? AND DateCreated=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( cn.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//cn.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), cn.m_ContactNoteID );
			BINDPARAM_LONG( arc.GetQuery(), cn.m_ContactID );
			BINDPARAM_LONG( arc.GetQuery(), cn.m_AgentID );
			BINDPARAM_TIME( arc.GetQuery(), cn.m_DateCreated );
			BINDPARAM_TEXT( arc.GetQuery(), cn.m_Note );
			BINDPARAM_BIT( arc.GetQuery(), cn.m_IsVoipNote );
			BINDPARAM_TCHAR( arc.GetQuery(), cn.m_ElapsedTime );
			if ( cn.m_StartTimeLen <= 0 || cn.m_StopTimeLen <= 0 )
			{
				arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ContactNotes ON INSERT INTO ContactNotes ")
						_T("(ContactNoteID,ContactID,AgentID,DateCreated,Note,IsVoipNote,ElapsedTime) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?) SET IDENTITY_INSERT ContactNotes OFF") );
			}
			else
			{
				BINDPARAM_TIME( arc.GetQuery(), cn.m_StartTime );
				BINDPARAM_TIME( arc.GetQuery(), cn.m_StopTime );
				arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ContactNotes ON INSERT INTO ContactNotes ")
						_T("(ContactNoteID,ContactID,AgentID,DateCreated,Note,IsVoipNote,ElapsedTime,StartTime,StopTime) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT ContactNotes OFF") );
			}	
		}
	}

	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffContactRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffContactRecord( CArchiveFile& arc, long ID )
{
	TContacts c;
	int ret;
	c.m_ContactID = ID;

	// Query from the databse
	if( c.Query( arc.GetQuery() ) == S_OK )
	{
		// Stuff the data
		PUTCHAR( c.m_Name );
		PUTCHAR( c.m_CompanyName );
		PUTCHAR( c.m_Title );
		PUTCHAR( c.m_WebPageAddress );
		PUTCHAR( c.m_StreetAddress1 );
		PUTCHAR( c.m_StreetAddress2 );
		PUTCHAR( c.m_City );
		PUTCHAR( c.m_State );
		PUTCHAR( c.m_ZipCode );
		PUTCHAR( c.m_Country );
		PUTDATE( c.m_DateCreated );
		PUTBYTE( c.m_IsDeleted );
		PUTDATE( c.m_DeletedTime );
		PUTLONG( c.m_DeletedBy );
		PUTLONG( c.m_OwnerID );
		PUTLONG( c.m_DefaultEmailAddressID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveContactRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveContactRecord( CArchiveFile& arc, long ID )
{
	TContacts c;
	int ret = Arc_Success;
	
	c.m_ContactID = ID;

	GETCHAR( c.m_Name, CONTACTS_NAME_LENGTH );
	GETCHAR( c.m_CompanyName, CONTACTS_COMPANYNAME_LENGTH );
	GETCHAR( c.m_Title, CONTACTS_TITLE_LENGTH );
	GETCHAR( c.m_WebPageAddress, CONTACTS_WEBPAGEADDRESS_LENGTH );
	GETCHAR( c.m_StreetAddress1, CONTACTS_STREETADDRESS1_LENGTH );
	GETCHAR( c.m_StreetAddress2, CONTACTS_STREETADDRESS2_LENGTH );
	GETCHAR( c.m_City, CONTACTS_CITY_LENGTH );
	GETCHAR( c.m_State, CONTACTS_STATE_LENGTH );
	GETCHAR( c.m_ZipCode, CONTACTS_ZIPCODE_LENGTH );
	GETCHAR( c.m_Country, CONTACTS_COUNTRY_LENGTH );
	GETDATE( c.m_DateCreated );
	GETBYTE( c.m_IsDeleted );
	GETDATE( c.m_DeletedTime );
	GETLONG( c.m_DeletedBy );
	arc.TranslateID( TABLEID_Agents, c.m_DeletedBy );
	GETLONG( c.m_OwnerID );
	arc.TranslateID( TABLEID_Agents, c.m_OwnerID );
	GETLONG( c.m_DefaultEmailAddressID );
	// NOTE: arc.Translate DefaultEmailAddressID later

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Contacts].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( c.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), c.m_Name );
		BINDPARAM_TIME( arc.GetQuery(), c.m_DateCreated );
		BINDCOL_LONG( arc.GetQuery(), c.m_ContactID );
		arc.GetQuery().Execute( _T("SELECT ContactID FROM Contacts ")
								_T("WHERE Name=? AND DateCreated=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( c.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//c.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), c.m_ContactID );
			BINDPARAM_TCHAR( arc.GetQuery(), c.m_Name );
			BINDPARAM_TCHAR( arc.GetQuery(), c.m_CompanyName );
			BINDPARAM_TCHAR( arc.GetQuery(), c.m_Title );
			BINDPARAM_TCHAR( arc.GetQuery(), c.m_WebPageAddress );
			BINDPARAM_TCHAR( arc.GetQuery(), c.m_StreetAddress1 );
			BINDPARAM_TCHAR( arc.GetQuery(), c.m_StreetAddress2 );
			BINDPARAM_TCHAR( arc.GetQuery(), c.m_City );
			BINDPARAM_TCHAR( arc.GetQuery(), c.m_State );
			BINDPARAM_TCHAR( arc.GetQuery(), c.m_ZipCode );
			BINDPARAM_TCHAR( arc.GetQuery(), c.m_Country );
			BINDPARAM_TIME( arc.GetQuery(), c.m_DateCreated );
			BINDPARAM_TINYINT( arc.GetQuery(), c.m_IsDeleted );
			BINDPARAM_TIME( arc.GetQuery(), c.m_DeletedTime );
			BINDPARAM_LONG( arc.GetQuery(), c.m_DeletedBy );
			BINDPARAM_LONG( arc.GetQuery(), c.m_OwnerID );
			BINDPARAM_LONG( arc.GetQuery(), c.m_DefaultEmailAddressID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Contacts ON INSERT INTO Contacts ")
						_T("(ContactID,Name,CompanyName,Title,WebPageAddress,StreetAddress1,StreetAddress2,City,State,ZipCode,Country,DateCreated,IsDeleted,DeletedTime,DeletedBy,OwnerID,DefaultEmailAddressID) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT Contacts OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_Contacts, ID, c.m_ContactID );

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffContactGroupRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffContactGroupRecord( CArchiveFile& arc, long ID )
{
	TContactGroups g;
	int ret;
	g.m_ContactGroupID = ID;

	// Query from the databse
	if( g.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( g.m_GroupName );
		PUTLONG( g.m_OwnerID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveContactGroupRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveContactGroupRecord( CArchiveFile& arc, long ID )
{
	TContactGroups g;
	int ret= Arc_Success;

	g.m_ContactGroupID = ID;

	GETCHAR( g.m_GroupName, GROUPS_GROUPNAME_LENGTH );
	GETLONG( g.m_OwnerID );
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ContactGroups].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( g.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), g.m_GroupName );
		BINDCOL_LONG( arc.GetQuery(), g.m_ContactGroupID );
		arc.GetQuery().Execute( _T("SELECT ContactGroupID FROM ContactGroups ")
								_T("WHERE GroupName=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( g.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//g.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), g.m_ContactGroupID );
			BINDPARAM_TCHAR( arc.GetQuery(), g.m_GroupName );
			BINDPARAM_LONG( arc.GetQuery(), g.m_OwnerID );			
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ContactGroups ON INSERT INTO ContactGroups ")
						_T("(ContactGroupID,GroupName,OwnerID) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT ContactGroups OFF") );
		}
	}

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffContactGroupingRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffContactGroupingRecord( CArchiveFile& arc, long ID )
{
	TContactGrouping ag;
	int ret;

	// Set the object ID
	ag.m_ContactGroupingID = ID;

	// Query from the databse
	if( ag.Query( arc.GetQuery() ) == S_OK )
	{
		// Stuff the data
		PUTLONG( ag.m_ContactID );
		PUTLONG( ag.m_ContactGroupID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveContactGroupingRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveContactGroupingRecord( CArchiveFile& arc, long ID )
{
	TContactGrouping ag;
	int ret = Arc_Success;

	ag.m_ContactGroupingID = ID;
	GETLONG( ag.m_ContactID );
	GETLONG( ag.m_ContactGroupID );
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ContactGrouping].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ag.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), ag.m_ContactID );
		BINDPARAM_LONG( arc.GetQuery(), ag.m_ContactGroupID );
		BINDCOL_LONG( arc.GetQuery(), ag.m_ContactGroupingID );
		arc.GetQuery().Execute( _T("SELECT ContactGroupingID FROM ContactGroupings ")
								_T("WHERE ContactID=? AND ContactGroupID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
				return ( ag.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
		else
		{
			//ag.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ag.m_ContactGroupingID );
			BINDPARAM_LONG( arc.GetQuery(), ag.m_ContactID );
			BINDPARAM_LONG( arc.GetQuery(), ag.m_ContactGroupID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ContactGrouping ON INSERT INTO ContactGrouping ")
						_T("(ContactGroupingID,ContactID,ContactGroupID) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT ContactGrouping OFF") );
		}
	}


	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffForwardCCAddressRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffForwardCCAddressRecord( CArchiveFile& arc, long ID )
{
	TForwardCCAddresses fca;
	int ret;

	fca.m_AddressID = ID;

	// Query from the databse
	if( fca.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( fca.m_RoutingRuleID );
		PUTCHAR( fca.m_EmailAddress );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveForwardCCAddressRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveForwardCCAddressRecord( CArchiveFile& arc, long ID )
{
	TForwardCCAddresses fca;
	int ret = Arc_Success;

	fca.m_AddressID = ID;

	GETLONG( fca.m_RoutingRuleID );
	arc.TranslateID( TABLEID_RoutingRules, fca.m_RoutingRuleID );
	GETCHAR( fca.m_EmailAddress, FORWARDCCADDRESSES_EMAILADDRESS_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ForwardCCAddresses].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( fca.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), fca.m_RoutingRuleID );
		BINDPARAM_TCHAR( arc.GetQuery(), fca.m_EmailAddress );
		BINDCOL_LONG( arc.GetQuery(), fca.m_AddressID );
		arc.GetQuery().Execute( _T("SELECT AddressID FROM ForwardCCAddresses ")
								_T("WHERE RoutingRuleID=? AND EmailAddress=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( fca.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//fca.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), fca.m_AddressID );
			BINDPARAM_TCHAR( arc.GetQuery(), fca.m_EmailAddress );
			BINDPARAM_LONG( arc.GetQuery(), fca.m_RoutingRuleID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ForwardCCAddresses ON INSERT INTO ForwardCCAddresses ")
						_T("(AddressID,EmailAddress,RoutingRuleID) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT ForwardCCAddresses OFF") );
		}
	}

	return ret;	
}



////////////////////////////////////////////////////////////////////////////////
// 
// StuffForwardToAddressRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffForwardToAddressRecord( CArchiveFile& arc, long ID )
{
	TForwardToAddresses fta;
	int ret;

	fta.m_AddressID = ID;

	// Query from the databse
	if( fta.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( fta.m_RoutingRuleID );
		PUTCHAR( fta.m_EmailAddress );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveForwardToAddressRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveForwardToAddressRecord( CArchiveFile& arc, long ID )
{
	TForwardToAddresses fta;
	int ret = Arc_Success;

	fta.m_AddressID = ID;

	GETLONG( fta.m_RoutingRuleID );
	arc.TranslateID( TABLEID_RoutingRules, fta.m_RoutingRuleID );
	GETCHAR( fta.m_EmailAddress, FORWARDCCADDRESSES_EMAILADDRESS_LENGTH );


	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ForwardCCAddresses].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( fta.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), fta.m_RoutingRuleID );
		BINDPARAM_TCHAR( arc.GetQuery(), fta.m_EmailAddress );
		BINDCOL_LONG( arc.GetQuery(), fta.m_AddressID );
		arc.GetQuery().Execute( _T("SELECT AddressID FROM ForwardToAddresses ")
								_T("WHERE RoutingRuleID=? AND EmailAddress=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( fta.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//fta.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), fta.m_AddressID );
			BINDPARAM_TCHAR( arc.GetQuery(), fta.m_EmailAddress );
			BINDPARAM_LONG( arc.GetQuery(), fta.m_RoutingRuleID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ForwardToAddresses ON INSERT INTO ForwardToAddresses ")
						_T("(AddressID,EmailAddress,RoutingRuleID) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT ForwardToAddresses OFF") );
		}
	}

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffGroupRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffGroupRecord( CArchiveFile& arc, long ID )
{
	TGroups g;
	int ret;
	g.m_GroupID = ID;

	// Query from the databse
	if( g.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( g.m_GroupName );
		PUTLONG( g.m_ObjectID );
		PUTBYTE( g.m_IsDeleted );
		PUTBYTE( g.m_IsEscGroup );
		PUTBYTE( g.m_UseEscTicketBox );
		PUTLONG( g.m_AssignToTicketBoxID );
		PUTLONG( g.m_TimeZoneID );
		PUTLONG( g.m_DictionaryID );
		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveGroupRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveGroupRecord( CArchiveFile& arc, long ID )
{
	TGroups g;
	int ret= Arc_Success;

	g.m_GroupID = ID;

	GETCHAR( g.m_GroupName, GROUPS_GROUPNAME_LENGTH );
	GETLONG( g.m_ObjectID );
	arc.TranslateID( TABLEID_Objects, g.m_ObjectID );
	GETBYTE( g.m_IsDeleted );
	GETBYTE( g.m_IsEscGroup );
	GETBYTE( g.m_UseEscTicketBox );
	GETLONG( g.m_AssignToTicketBoxID );
	GETLONG( g.m_TimeZoneID );
	GETLONG( g.m_DictionaryID );
	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Groups].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( g.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), g.m_GroupName );
		BINDCOL_LONG( arc.GetQuery(), g.m_GroupID );
		arc.GetQuery().Execute( _T("SELECT GroupID FROM Groups ")
								_T("WHERE GroupName=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( g.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//g.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), g.m_GroupID );
			BINDPARAM_TCHAR( arc.GetQuery(), g.m_GroupName );
			BINDPARAM_LONG( arc.GetQuery(), g.m_ObjectID );
			BINDPARAM_BIT( arc.GetQuery(), g.m_IsDeleted );
			BINDPARAM_BIT( arc.GetQuery(), g.m_IsEscGroup );
			BINDPARAM_BIT( arc.GetQuery(), g.m_UseEscTicketBox );
			BINDPARAM_LONG( arc.GetQuery(), g.m_AssignToTicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), g.m_TimeZoneID );
			BINDPARAM_LONG( arc.GetQuery(), g.m_DictionaryID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Groups ON INSERT INTO Groups ")
						_T("(GroupID,GroupName,ObjectID,IsDeleted,IsEscalationGroup,UseEscTicketBox,AssignToTicketBoxID,TimeZoneID,DictionaryID) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?) SET IDENTITY_INSERT Groups OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_Groups, ID, g.m_GroupID );
	arc.AddActualObjectID( g.m_ObjectID, g.m_GroupID );

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffIPRangeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffIPRangeRecord( CArchiveFile& arc, long ID )
{
	TIPRanges ipr;
	int ret;
	ipr.m_IPRangeID = ID;

	// Query from the databse
	if( ipr.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( ipr.m_AgentID );
		PUTLONG( ipr.m_GroupID );
		PUTLONG( ipr.m_LowerRange );
		PUTLONG( ipr.m_UpperRange );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveIPRangeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveIPRangeRecord( CArchiveFile& arc, long ID )
{
	TIPRanges ipr;
	int ret = Arc_Success;

	ipr.m_IPRangeID = ID;

	GETLONG( ipr.m_AgentID );
	arc.TranslateID( TABLEID_Agents, ipr.m_AgentID );
	GETLONG( ipr.m_GroupID );
	arc.TranslateID( TABLEID_Groups, ipr.m_GroupID );
	GETLONG( ipr.m_LowerRange );
	GETLONG( ipr.m_UpperRange );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_IPRanges].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ipr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), ipr.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), ipr.m_GroupID );
		BINDPARAM_LONG( arc.GetQuery(), ipr.m_LowerRange );
		BINDPARAM_LONG( arc.GetQuery(), ipr.m_UpperRange );
		BINDCOL_LONG( arc.GetQuery(), ipr.m_IPRangeID );
		arc.GetQuery().Execute( _T("SELECT IPRangeID FROM IPRanges ")
								_T("WHERE AgentID=? AND GroupID=? AND LowerRange=? AND UpperRange=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ipr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ipr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ipr.m_IPRangeID );
			BINDPARAM_LONG( arc.GetQuery(), ipr.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), ipr.m_GroupID );
			BINDPARAM_LONG( arc.GetQuery(), ipr.m_LowerRange );
			BINDPARAM_LONG( arc.GetQuery(), ipr.m_UpperRange );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT IPRanges ON INSERT INTO IPRanges ")
						_T("(IPRangeID,AgentID,GroupID,LowerRange,UpperRange) ")
						_T("VALUES")
						_T("(?,?,?,?,?) SET IDENTITY_INSERT IPRanges OFF") );
		}
	}

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffAttachmentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAttachmentRecord( CArchiveFile& arc, long ID )
{
	TAttachments att;
	int ret;
	att.m_AttachmentID = ID;
	CEMSString sTemp;

	// Query from the databse
	if( att.Query( arc.GetQuery() ) == S_OK )
	{
		bool bIsNote = false;
		
		if ( _tcscmp( att.m_ContentDisposition, _T("noteattach")) == 0 )
		{
			bIsNote = true;
		}
		
		PUTCHAR( att.m_AttachmentLocation );
		PUTLONG( att.m_FileSize );
		PUTLONG( att.m_VirusScanStateID );
		PUTCHAR( att.m_VirusName );
		PUTCHAR( att.m_MediaType );
		PUTCHAR( att.m_MediaSubType );
		PUTCHAR( att.m_ContentDisposition );
		PUTCHAR( att.m_FileName );
		PUTCHAR( att.m_ContentID );
		PUTBYTE( att.m_IsInbound );
		/*sTemp.assign(att.m_AttachmentLocation);
		PutStringProperty( sTemp , &att.m_AttachLoc, &att.m_AttachLocAllocated );		
		arc.PutTextField(att.m_AttachLoc, att.m_AttachLocAllocated );
		sTemp.assign(att.m_FileName);
		PutStringProperty( sTemp , &att.m_AttachName, &att.m_AttachNameAllocated );
		arc.PutTextField( att.m_AttachName, att.m_AttachNameAllocated );*/
		PUTFILE( att.m_AttachmentLocation, (att.m_IsInbound != 0 ? true : false ), bIsNote);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveAttachmentToStruct(  CArchiveFile& arc, Attachments_t& att )
{
	int ret = Arc_Success;
	
	GETCHAR( att.m_AttachmentLocation, ATTACHMENTS_ATTACHMENTLOCATION_LENGTH );
	GETLONG( att.m_FileSize );
	GETLONG( att.m_VirusScanStateID );
	GETCHAR( att.m_VirusName, ATTACHMENTS_VIRUSNAME_LENGTH );
	GETCHAR( att.m_MediaType, ATTACHMENTS_MEDIATYPE_LENGTH );
	GETCHAR( att.m_MediaSubType, ATTACHMENTS_MEDIASUBTYPE_LENGTH );
	GETCHAR( att.m_ContentDisposition, ATTACHMENTS_CONTENTDISPOSITION_LENGTH );
	GETCHAR( att.m_FileName, ATTACHMENTS_FILENAME_LENGTH );
	GETCHAR( att.m_ContentID, ATTACHMENTS_CONTENTID_LENGTH );
	GETBYTE( att.m_IsInbound );

	/*if( arc.GetHdr().Version >= 0x05000000 )
	{
		CEMSString sTemp;
		GETTEXT(att.m_AttachLoc);
		if (!_tcscmp(att.m_AttachmentLocation, att.m_AttachLoc) == 0 )
		{
			_tcscpy(att.m_AttachmentLocation, att.m_AttachLoc);	
		}
		GETTEXT(att.m_AttachName );
		if (!_tcscmp(att.m_FileName, att.m_AttachName) == 0 )
		{
			_tcscpy(att.m_FileName, att.m_AttachName);
		}
	}*/

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAttachmentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAttachmentRecord( CArchiveFile& arc, long ID )
{
	TAttachments att;
	int ret;
	att.m_AttachmentID = ID;

	ret = RetrieveAttachmentToStruct( arc, att );

	bool bIsNote = false;
	if ( _tcscmp( att.m_ContentDisposition, _T("noteattach")) == 0 )
	{
		bIsNote = true;
	}

	if( ret != Arc_Success )
	{
		return ret;
	}

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Attachments].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			// TODO: Log this
			GETFILE( att.m_AttachmentLocation, (att.m_IsInbound ? true : false), ATTACHMENTS_ATTACHMENTLOCATION_LENGTH, att.m_FileName, bIsNote );
			ret = ( att.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), att.m_AttachmentLocation );
		BINDPARAM_LONG( arc.GetQuery(), att.m_FileSize );		
		BINDCOL_LONG( arc.GetQuery(), att.m_AttachmentID );
		arc.GetQuery().Execute( _T("SELECT AttachmentID FROM Attachments ")
								_T("WHERE AttachmentLocation=? AND FileSize=? ") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				// TODO: Log this
				GETFILE( att.m_AttachmentLocation, (att.m_IsInbound != 0 ? true : false), ATTACHMENTS_ATTACHMENTLOCATION_LENGTH, att.m_FileName, bIsNote );
				ret = ( att.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			ret = arc.GetFileField( att.m_AttachmentLocation, (att.m_IsInbound != 0 ? true : false), ATTACHMENTS_ATTACHMENTLOCATION_LENGTH, att.m_FileName, bIsNote );
			if(ret != Arc_Success)
			{
				if(_tcslen(att.m_AttachmentLocation) < _tcslen(att.m_FileName))
				{
					CEMSString sTemp;
					sTemp.Format(_T("Error restoring attachment [%s]"),att.m_FileName);
					ret = Arc_Success;
					IRoutingEngineComm* pRoutingEngine;
					HRESULT hres;

					hres = CoCreateInstance( CLSID_RoutingEngineComm, NULL, CLSCTX_LOCAL_SERVER, 
											IID_IRoutingEngineComm, (void**) &pRoutingEngine );

					if ( SUCCEEDED( hres ) )
					{
						dca::BString bstr(sTemp.c_str());						
						pRoutingEngine->Log( EMSERROR( EMS_LOG_SEVERITY_ERROR,
											EMSERR_ROUTING_ENGINE,
											EMS_LOG_DATABASE_MAINTENANCE,
											Arc_ErrCreatingFile), bstr );
						pRoutingEngine->Release();
						SysFreeString( bstr );
					}					
				}				
			}
			
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), att.m_AttachmentID );
			BINDPARAM_TCHAR( arc.GetQuery(), att.m_AttachmentLocation );
			BINDPARAM_TCHAR( arc.GetQuery(), att.m_MediaType );
			BINDPARAM_TCHAR( arc.GetQuery(), att.m_MediaSubType );
			BINDPARAM_TCHAR( arc.GetQuery(), att.m_ContentDisposition );
			BINDPARAM_TCHAR( arc.GetQuery(), att.m_FileName );
			BINDPARAM_LONG( arc.GetQuery(), att.m_FileSize );
			BINDPARAM_LONG( arc.GetQuery(), att.m_VirusScanStateID );
			BINDPARAM_TCHAR( arc.GetQuery(), att.m_VirusName );
			BINDPARAM_TCHAR( arc.GetQuery(), att.m_ContentID );
			BINDPARAM_BIT( arc.GetQuery(), att.m_IsInbound );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Attachments ON INSERT INTO Attachments ")
						_T("(AttachmentID,AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT Attachments OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_Attachments, ID, att.m_AttachmentID );

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffInboundMessageAttachmentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffInboundMessageAttachmentRecord( CArchiveFile& arc, long ID )
{
	TInboundMessageAttachments ina;
	int ret;
	ina.m_InboundMessageAttachmentID = ID;

		// Query from the databse
	if( ina.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( ina.m_InboundMessageID );
		PUTLONG( ina.m_AttachmentID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveInboundMessageAttachmentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveInboundMessageAttachmentRecord( CArchiveFile& arc, long ID )
{
	TInboundMessageAttachments ina;
	int ret = Arc_Success;
	
	ina.m_InboundMessageAttachmentID = ID;

	GETLONG( ina.m_InboundMessageID );
	arc.TranslateID( TABLEID_InboundMessages, ina.m_InboundMessageID );
	GETLONG( ina.m_AttachmentID );
	arc.TranslateID( TABLEID_Attachments, ina.m_AttachmentID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_InboundMessageAttachments].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ina.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), ina.m_InboundMessageID );
		BINDPARAM_LONG( arc.GetQuery(), ina.m_AttachmentID );
		BINDCOL_LONG( arc.GetQuery(), ina.m_InboundMessageAttachmentID );
		arc.GetQuery().Execute( _T("SELECT InboundMessageAttachmentID FROM InboundMessageAttachments ")
								_T("WHERE InboundMessageID=? AND AttachmentID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ina.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			ina.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}



////////////////////////////////////////////////////////////////////////////////
// 
// StuffInboundMessageQueueRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffInboundMessageQueueRecord( CArchiveFile& arc, long ID )
{
	TInboundMessageQueue imq;
	int ret;
	imq.m_InboundMessageQueueID = ID;

	// Query from the databse
	if( imq.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( imq.m_InboundMessageID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveInboundMessageQueueRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveInboundMessageQueueRecord( CArchiveFile& arc, long ID )
{
	TInboundMessageQueue imq;
	int ret = Arc_Success;

	imq.m_InboundMessageQueueID = ID;

	GETLONG( imq.m_InboundMessageID );
	arc.TranslateID( TABLEID_InboundMessages, imq.m_InboundMessageID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_InboundMessageQueue].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( imq.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), imq.m_InboundMessageID );
		BINDCOL_LONG( arc.GetQuery(), imq.m_InboundMessageQueueID );
		arc.GetQuery().Execute( _T("SELECT InboundMessageQueueID FROM InboundMessageQueue ")
								_T("WHERE InboundMessageID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( imq.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			try
			{
				imq.Insert( arc.GetQuery() );
			}
			catch(...)
			{

			}
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffInboundMessageRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffInboundMessageRecord( CArchiveFile& arc, long ID )
{
	TInboundMessages im;
	int ret;
	im.m_InboundMessageID = ID;

	// Query from the databse
	if( im.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( im.m_TicketID );
		PUTCHAR( im.m_EmailFrom );
		PUTCHAR( im.m_EmailFromName );
		PUTCHAR( im.m_EmailPrimaryTo );
		PUTDATE( im.m_EmailDateTime );
		PUTCHAR( im.m_Subject );
		PUTCHAR( im.m_MediaType );
		PUTCHAR( im.m_MediaSubType );
		PUTBYTE( im.m_IsDeleted );
		PUTLONG( im.m_DeletedBy );
		PUTDATE( im.m_DeletedTime );
		PUTLONG( im.m_RoutingRuleID );
		PUTLONG( im.m_MessageSourceID );
		PUTLONG( im.m_VirusScanStateID );
		PUTLONG( im.m_ReplyToMsgID );
		PUTBYTE( im.m_ReplyToIDIsInbound );
		PUTCHAR( im.m_VirusName );
		PUTTEXT( im.m_Body );
		PUTTEXT( im.m_PopHeaders );
		PUTTEXT( im.m_EmailTo );
		PUTTEXT( im.m_EmailCc );
		PUTTEXT( im.m_EmailReplyTo );
		PUTLONG( im.m_ContactID );
		PUTLONG( im.m_PriorityID );
		PUTLONG( im.m_ArchiveID );
		PUTDATE( im.m_DateReceived );
		PUTLONG( im.m_OriginalTicketBoxID );
		PUTLONG( im.m_OriginalOwnerID );
		PUTLONG( im.m_OriginalTicketCategoryID );
		PUTBYTE( im.m_ReadReceipt );
		PUTCHAR( im.m_ReadReceiptTo );				
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveInboundMessageToStruct( CArchiveFile& arc, InboundMessages_t& im )
{
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Entering", DebugReporter::LIB);
	
	int ret = Arc_Success;

	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - TicketID", DebugReporter::LIB);
	GETLONG( im.m_TicketID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Translate TicketID", DebugReporter::LIB);
	arc.TranslateID( TABLEID_Tickets, im.m_TicketID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - EmailFrom", DebugReporter::LIB);
	GETCHAR( im.m_EmailFrom, INBOUNDMESSAGES_EMAILFROM_LENGTH );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - EmailFromName", DebugReporter::LIB);
	GETCHAR( im.m_EmailFromName, INBOUNDMESSAGES_EMAILFROMNAME_LENGTH );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - EmailPrimaryTo", DebugReporter::LIB);
	GETCHAR( im.m_EmailPrimaryTo, INBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - EmailDateTime", DebugReporter::LIB);
	GETDATE( im.m_EmailDateTime );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Subject", DebugReporter::LIB);
	GETCHAR( im.m_Subject, INBOUNDMESSAGES_SUBJECT_LENGTH );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - MediaType", DebugReporter::LIB);
	GETCHAR( im.m_MediaType, INBOUNDMESSAGES_MEDIATYPE_LENGTH );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - MediaSubType", DebugReporter::LIB);
	GETCHAR( im.m_MediaSubType, INBOUNDMESSAGES_MEDIASUBTYPE_LENGTH );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - IsDeleted", DebugReporter::LIB);
	GETBYTE( im.m_IsDeleted );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - DeletedBy", DebugReporter::LIB);
	GETLONG( im.m_DeletedBy );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Translate DeltedBy", DebugReporter::LIB);
	arc.TranslateID( TABLEID_Tickets, im.m_DeletedBy );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - DeletedTime", DebugReporter::LIB);
	GETDATE( im.m_DeletedTime );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - RoutingRuleID", DebugReporter::LIB);
	GETLONG( im.m_RoutingRuleID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Translate RoutingRuleID", DebugReporter::LIB);
	arc.TranslateID( TABLEID_RoutingRules, im.m_RoutingRuleID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - MessageSourceID", DebugReporter::LIB);
	GETLONG( im.m_MessageSourceID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Translate MessageSourceID", DebugReporter::LIB);
	arc.TranslateID( TABLEID_MessageSources, im.m_MessageSourceID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - VirusScanStateID", DebugReporter::LIB);
	GETLONG( im.m_VirusScanStateID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - ReplyToMsgID", DebugReporter::LIB);
	GETLONG( im.m_ReplyToMsgID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - ReplyToMsgIDIsInbound", DebugReporter::LIB);
	GETBYTE( im.m_ReplyToIDIsInbound );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - VirusName", DebugReporter::LIB);
	GETCHAR( im.m_VirusName, INBOUNDMESSAGES_VIRUSNAME_LENGTH );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Body", DebugReporter::LIB);
	GETTEXT( im.m_Body );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - PopHeaders", DebugReporter::LIB);
	GETTEXT( im.m_PopHeaders );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - EmailTo", DebugReporter::LIB);
	GETTEXT( im.m_EmailTo );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - EmailCc", DebugReporter::LIB);
	GETTEXT( im.m_EmailCc );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - EmailReplyTo", DebugReporter::LIB);
	GETTEXT( im.m_EmailReplyTo );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - ContactID", DebugReporter::LIB);
	GETLONG( im.m_ContactID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Translate ContactID", DebugReporter::LIB);
	arc.TranslateID( TABLEID_Contacts, im.m_ContactID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - PriorityID", DebugReporter::LIB);
	GETLONG( im.m_PriorityID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - ArchiveID", DebugReporter::LIB);
	GETLONG( im.m_ArchiveID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - DateReceived", DebugReporter::LIB);
	GETDATE( im.m_DateReceived );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - OriginalTicketBoxID", DebugReporter::LIB);
	GETLONG( im.m_OriginalTicketBoxID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Translate OriginalTicketBoxID", DebugReporter::LIB);
	arc.TranslateID( TABLEID_TicketBoxes, im.m_OriginalTicketBoxID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - OriginalOwnerID", DebugReporter::LIB);
	GETLONG( im.m_OriginalOwnerID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Translate OriginalOwnerID", DebugReporter::LIB);
	arc.TranslateID( TABLEID_Agents, im.m_OriginalOwnerID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - OriginalTicketCategoryID", DebugReporter::LIB);
	GETLONG( im.m_OriginalTicketCategoryID );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - ReadReceipt", DebugReporter::LIB);
	GETBYTE( im.m_ReadReceipt );
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - ReadReceiptTo", DebugReporter::LIB);
	GETCHAR( im.m_ReadReceiptTo, INBOUNDMESSAGES_EMAILFROM_LENGTH );	
	
	DebugReporter::Instance().DisplayMessage("RetrieveInboundMessageToStruct - Leaving", DebugReporter::LIB);

	return ret;
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveInboundMessageRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveInboundMessageRecord( CArchiveFile& arc, long ID )
{
	TInboundMessages im;
	int ret;
	im.m_InboundMessageID = ID;
	
	ret = RetrieveInboundMessageToStruct( arc, im );
	if( ret != Arc_Success )
	{
		return ret;
	}

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_InboundMessages].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( im.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), im.m_Subject );
		BINDPARAM_LONG( arc.GetQuery(), im.m_TicketID );
		BINDPARAM_TIME( arc.GetQuery(), im.m_DateReceived );
		BINDCOL_LONG( arc.GetQuery(), im.m_InboundMessageID );
		arc.GetQuery().Execute( _T("SELECT InboundMessageID FROM InboundMessages ")
								_T("WHERE Subject=? AND TicketID=? AND DateReceived=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( im.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//im.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), im.m_InboundMessageID );
			BINDPARAM_LONG( arc.GetQuery(), im.m_TicketID );
			BINDPARAM_TCHAR( arc.GetQuery(), im.m_EmailFrom );
			BINDPARAM_TCHAR( arc.GetQuery(), im.m_EmailFromName );
			BINDPARAM_TCHAR( arc.GetQuery(), im.m_EmailPrimaryTo );
			BINDPARAM_TIME( arc.GetQuery(), im.m_EmailDateTime );
			BINDPARAM_TIME( arc.GetQuery(), im.m_DateReceived );
			BINDPARAM_TCHAR( arc.GetQuery(), im.m_Subject );
			BINDPARAM_TCHAR( arc.GetQuery(), im.m_MediaType );
			BINDPARAM_TCHAR( arc.GetQuery(), im.m_MediaSubType );
			BINDPARAM_TINYINT( arc.GetQuery(), im.m_IsDeleted );
			BINDPARAM_LONG( arc.GetQuery(), im.m_DeletedBy );
			BINDPARAM_TIME( arc.GetQuery(), im.m_DeletedTime );
			BINDPARAM_LONG( arc.GetQuery(), im.m_RoutingRuleID );
			BINDPARAM_LONG( arc.GetQuery(), im.m_MessageSourceID );
			BINDPARAM_LONG( arc.GetQuery(), im.m_VirusScanStateID );
			BINDPARAM_LONG( arc.GetQuery(), im.m_ReplyToMsgID );
			BINDPARAM_BIT( arc.GetQuery(), im.m_ReplyToIDIsInbound );
			BINDPARAM_TCHAR( arc.GetQuery(), im.m_VirusName );
			BINDPARAM_TEXT( arc.GetQuery(), im.m_Body );
			BINDPARAM_TEXT( arc.GetQuery(), im.m_PopHeaders );
			BINDPARAM_TEXT( arc.GetQuery(), im.m_EmailTo );
			BINDPARAM_TEXT( arc.GetQuery(), im.m_EmailCc );
			BINDPARAM_TEXT( arc.GetQuery(), im.m_EmailReplyTo );
			BINDPARAM_LONG( arc.GetQuery(), im.m_ContactID );
			BINDPARAM_LONG( arc.GetQuery(), im.m_PriorityID );
			BINDPARAM_LONG( arc.GetQuery(), im.m_ArchiveID );
			BINDPARAM_LONG( arc.GetQuery(), im.m_OriginalTicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), im.m_OriginalTicketCategoryID );
			BINDPARAM_LONG( arc.GetQuery(), im.m_OriginalOwnerID );
			BINDPARAM_TINYINT( arc.GetQuery(), im.m_ReadReceipt );
			BINDPARAM_TCHAR( arc.GetQuery(), im.m_ReadReceiptTo );			
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT InboundMessages ON INSERT INTO InboundMessages ")
						_T("(InboundMessageID,TicketID,EmailFrom,EmailFromName,EmailPrimaryTo,EmailDateTime,DateReceived,Subject,MediaType,MediaSubType,IsDeleted,DeletedBy,DeletedTime,RoutingRuleID,MessageSourceID,VirusScanStateID,ReplyToMsgID,ReplyToIDIsInbound,VirusName,Body,PopHeaders,EmailTo,EmailCc,EmailReplyTo,ContactID,PriorityID,ArchiveID,OriginalTicketBoxID,OriginalTicketCategoryID,OriginalOwnerID,ReadReceipt,ReadReceiptTo) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT InboundMessages OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_InboundMessages, ID, im.m_InboundMessageID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffLogEntryTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffLogEntryTypeRecord( CArchiveFile& arc, long ID )
{
	TLogEntryTypes le;
	int ret;
	le.m_LogEntryTypeID = ID;

	// Query from the databse
	if( le.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( le.m_TypeDescrip );
		PUTLONG( le.m_SeverityLevels );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveLogEntryTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveLogEntryTypeRecord( CArchiveFile& arc, long ID )
{
	TLogEntryTypes le;
	int ret = Arc_Success;

	le.m_LogEntryTypeID = ID;

	GETCHAR( le.m_TypeDescrip, LOGENTRYTYPES_TYPEDESCRIP_LENGTH );
	GETLONG( le.m_SeverityLevels );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_LogEntryTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( le.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), le.m_TypeDescrip );
		BINDCOL_LONG( arc.GetQuery(), le.m_LogEntryTypeID );
		arc.GetQuery().Execute( _T("SELECT LogEntryTypeID FROM LogEntryTypes ")
								_T("WHERE TypeDescrip=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( le.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//le.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), le.m_LogEntryTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), le.m_TypeDescrip );
			BINDPARAM_LONG( arc.GetQuery(), le.m_SeverityLevels );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT LogEntryTypes ON INSERT INTO LogEntryTypes ")
						_T("(LogEntryTypeID,TypeDescrip,SeverityLevels) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT LogEntryTypes OFF") );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffLogSeverityRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffLogSeverityRecord( CArchiveFile& arc, long ID )
{
	TLogSeverity ls;
	int ret;
	ls.m_LogSeverityID = ID;

	// Query from the databse
	if( ls.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( ls.m_Description );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveLogSeverityRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveLogSeverityRecord( CArchiveFile& arc, long ID )
{
	TLogSeverity ls;
	int ret = Arc_Success;

	ls.m_LogSeverityID = ID;

	GETCHAR( ls.m_Description, LOGSEVERITY_DESCRIPTION_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_LogSeverity].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ls.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), ls.m_Description );
		BINDCOL_LONG( arc.GetQuery(), ls.m_LogSeverityID );
		arc.GetQuery().Execute( _T("SELECT LogSeverityID FROM LogSeverity ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ls.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ls.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ls.m_LogSeverityID );
			BINDPARAM_TCHAR( arc.GetQuery(), ls.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT LogSeverity ON INSERT INTO LogSeverity ")
						_T("(LogSeverityID,Description) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT LogSeverity OFF") );
		}
	}
		
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffMatchFromAddressRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffMatchFromAddressRecord( CArchiveFile& arc, long ID )
{
	TMatchFromAddresses mfa;
	int ret;
	mfa.m_MatchID = ID;

	// Query from the databse
	if( mfa.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( mfa.m_RoutingRuleID );
		PUTCHAR( mfa.m_EmailAddress );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveMatchFromAddressRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveMatchFromAddressRecord( CArchiveFile& arc, long ID )
{
	TMatchFromAddresses mfa;
	int ret = Arc_Success;

	mfa.m_MatchID = ID;

	GETLONG( mfa.m_RoutingRuleID );
	arc.TranslateID( TABLEID_RoutingRules, mfa.m_RoutingRuleID );
	GETCHAR( mfa.m_EmailAddress, MATCHFROMADDRESSES_EMAILADDRESS_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_MatchFromAddresses].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( mfa.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), mfa.m_RoutingRuleID );
		BINDPARAM_TCHAR( arc.GetQuery(), mfa.m_EmailAddress );
		BINDCOL_LONG( arc.GetQuery(), mfa.m_MatchID );
		arc.GetQuery().Execute( _T("SELECT MatchID FROM MatchFromAddresses ")
								_T("WHERE RoutingRuleID=? AND EmailAddress=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( mfa.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			mfa.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffMatchTextRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffMatchTextRecord( CArchiveFile& arc, long ID )
{
	TMatchText mt;
	int ret;
	mt.m_MatchID = ID;

	// Query from the databse
	if( mt.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( mt.m_RoutingRuleID );
		PUTBYTE( mt.m_MatchLocation );
		PUTBYTE( mt.m_IsRegEx );
		PUTTEXT( mt.m_MatchText );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveMatchTextRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveMatchTextRecord( CArchiveFile& arc, long ID )
{
	int ret = Arc_Success;

	try
	{
		TMatchText mt;

		mt.m_MatchID = ID;

		GETLONG( mt.m_RoutingRuleID );
		arc.TranslateID( TABLEID_RoutingRules, mt.m_RoutingRuleID );
		GETBYTE( mt.m_MatchLocation );
		GETBYTE( mt.m_IsRegEx );		
		GETTEXT( mt.m_MatchText );

		// Is the ID reserved?
		if( ID <= g_ArcTblInfo[TABLEID_MatchText].ReservedID ) 
		{ 
			if( arc.Overwrite() )
			{
				ret = ( mt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//DEBUGPRINT(_T("* SELECT MatchID FROM MatchText WHERE MatchLocation=%d AND MatchText LIKE %s\n"), mt.m_MatchLocation,mt.m_MatchText);
			// Check if record already exists
			arc.GetQuery().Initialize();
			BINDPARAM_TINYINT( arc.GetQuery(), mt.m_MatchLocation );
			BINDPARAM_BIT( arc.GetQuery(), mt.m_IsRegEx );
			BINDPARAM_TCHAR( arc.GetQuery(), mt.m_MatchText );			
			BINDCOL_LONG( arc.GetQuery(), mt.m_MatchID );
			arc.GetQuery().Execute( _T("SELECT MatchID FROM MatchText ")
									_T("WHERE MatchLocation=? AND IsRegEx=? AND MatchText LIKE ?") );

			if( arc.GetQuery().Fetch() == S_OK )
			{
				// Record exists, we now have the new ID
				if( arc.Overwrite() )
				{
					ret = ( mt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
				}
			}
			else
			{
				mt.Insert( arc.GetQuery() );
			}
		}
	}
	catch(...)
	{
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffMatchToAddressRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffMatchToAddressRecord( CArchiveFile& arc, long ID )
{
	TMatchToAddresses mta;
	int ret;
	mta.m_MatchID = ID;

	// Query from the databse
	if( mta.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( mta.m_RoutingRuleID );
		PUTCHAR( mta.m_EmailAddress );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveMatchToAddressRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveMatchToAddressRecord( CArchiveFile& arc, long ID )
{
	TMatchToAddresses mta;
	int ret = Arc_Success;

	mta.m_MatchID = ID;

	GETLONG( mta.m_RoutingRuleID );
	arc.TranslateID( TABLEID_RoutingRules, mta.m_RoutingRuleID );
	GETCHAR( mta.m_EmailAddress, MATCHFROMADDRESSES_EMAILADDRESS_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_MatchToAddresses].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( mta.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), mta.m_RoutingRuleID );
		BINDPARAM_TCHAR( arc.GetQuery(), mta.m_EmailAddress );
		BINDCOL_LONG( arc.GetQuery(), mta.m_MatchID );
		arc.GetQuery().Execute( _T("SELECT MatchID FROM MatchToAddresses ")
								_T("WHERE RoutingRuleID=? AND EmailAddress=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( mta.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			mta.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffMessageDestinationRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffMessageDestinationRecord( CArchiveFile& arc, long ID )
{
	TMessageDestinations md;
	int ret;
	md.m_MessageDestinationID = ID;

	dca::String f;
	f.Format("StuffMessageDestinationRecord - Processing MessageDestinationID [%d]",ID);
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
	
	// Query from the databse
	if( md.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( md.m_ServerAddress );
		PUTBYTE( md.m_UseSMTPAuth );
		PUTCHAR( md.m_AuthUser );
		PUTCHAR( md.m_AuthPass );
		PUTLONG( md.m_SMTPPort );
		PUTBYTE( md.m_IsActive );
		PUTLONG( md.m_ConnTimeoutSecs );
		PUTLONG( md.m_MaxOutboundMsgSize );
		PUTLONG( md.m_MaxSendRetryHours );
		PUTDATE( md.m_LastProcessInterval );
		PUTLONG( md.m_ProcessFreqMins );
		PUTCHAR( md.m_Description );
		PUTLONG( md.m_ServerID );
		PUTBYTE( md.m_IsSSL );
		PUTLONG( md.m_SSLMode );
		PUTLONG( md.m_OfficeHours );
		PUTLONG( md.m_ErrorCode );
		PUTTEXT( md.m_AccessToken );
		PUTDATE( md.m_AccessTokenExpire );
		PUTTEXT( md.m_RefreshToken );
		PUTDATE( md.m_RefreshTokenExpire );
		PUTLONG( md.m_OAuthHostID );		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveMessageDestinationRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveMessageDestinationRecord( CArchiveFile& arc, long ID )
{
	TMessageDestinations md;
	int ret = Arc_Success;

	md.m_MessageDestinationID = ID;

	GETCHAR( md.m_ServerAddress, MESSAGEDESTINATIONS_SERVERADDRESS_LENGTH );
	GETBYTE( md.m_UseSMTPAuth );
	GETCHAR( md.m_AuthUser, MESSAGEDESTINATIONS_AUTHUSER_LENGTH );
	GETCHAR( md.m_AuthPass, MESSAGEDESTINATIONS_AUTHPASS_LENGTH );
	GETLONG( md.m_SMTPPort );
	GETBYTE( md.m_IsActive );
	GETLONG( md.m_ConnTimeoutSecs );
	GETLONG( md.m_MaxOutboundMsgSize );
	GETLONG( md.m_MaxSendRetryHours );
	GETDATE( md.m_LastProcessInterval );
	GETLONG( md.m_ProcessFreqMins );
	GETCHAR( md.m_Description, MESSAGEDESTINATIONS_DESCRIPTION_LENGTH );
	GETLONG( md.m_ServerID );
	GETBYTE( md.m_IsSSL );
	GETLONG( md.m_SSLMode );
	GETLONG( md.m_OfficeHours );
	GETLONG( md.m_ErrorCode );
	GETTEXT( md.m_AccessToken );
	GETDATE( md.m_AccessTokenExpire );
	GETTEXT( md.m_RefreshToken );
	GETDATE( md.m_RefreshTokenExpire );	
	GETLONG( md.m_OAuthHostID );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_MessageDestinations].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( md.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), md.m_ServerAddress );
		BINDPARAM_LONG( arc.GetQuery(), md.m_SMTPPort );
		BINDPARAM_TCHAR( arc.GetQuery(), md.m_AuthUser );
		BINDCOL_LONG( arc.GetQuery(), md.m_MessageDestinationID );
		arc.GetQuery().Execute( _T("SELECT MessageDestinationID FROM MessageDestinations ")
								_T("WHERE ServerAddress=? AND SMTPPort=? AND AuthUser=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( md.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//md.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), md.m_MessageDestinationID );
			BINDPARAM_TCHAR( arc.GetQuery(), md.m_ServerAddress );
			BINDPARAM_BIT( arc.GetQuery(), md.m_UseSMTPAuth );
			BINDPARAM_TCHAR( arc.GetQuery(), md.m_AuthUser );
			BINDPARAM_TCHAR( arc.GetQuery(), md.m_AuthPass );
			BINDPARAM_LONG( arc.GetQuery(), md.m_SMTPPort );
			BINDPARAM_BIT( arc.GetQuery(), md.m_IsActive );
			BINDPARAM_LONG( arc.GetQuery(), md.m_ConnTimeoutSecs );
			BINDPARAM_LONG( arc.GetQuery(), md.m_MaxOutboundMsgSize );
			BINDPARAM_LONG( arc.GetQuery(), md.m_MaxSendRetryHours );
			BINDPARAM_TIME( arc.GetQuery(), md.m_LastProcessInterval );
			BINDPARAM_LONG( arc.GetQuery(), md.m_ProcessFreqMins );
			BINDPARAM_TCHAR( arc.GetQuery(), md.m_Description );
			BINDPARAM_LONG( arc.GetQuery(), md.m_ServerID );
			BINDPARAM_BIT( arc.GetQuery(), md.m_IsSSL );
			BINDPARAM_LONG( arc.GetQuery(), md.m_SSLMode );
			BINDPARAM_LONG( arc.GetQuery(), md.m_OfficeHours );
			BINDPARAM_LONG( arc.GetQuery(), md.m_ErrorCode );
			BINDPARAM_TEXT( arc.GetQuery(), md.m_AccessToken );
			BINDPARAM_TIME( arc.GetQuery(), md.m_AccessTokenExpire );
			BINDPARAM_TEXT( arc.GetQuery(), md.m_RefreshToken );
			BINDPARAM_TIME( arc.GetQuery(), md.m_RefreshTokenExpire );			
			BINDPARAM_LONG( arc.GetQuery(), md.m_OAuthHostID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT MessageDestinations ON INSERT INTO MessageDestinations ")
						_T("(MessageDestinationID,ServerAddress,UseSMTPAuth,AuthUser,AuthPass,SMTPPort,IsActive,ConnTimeoutSecs,MaxOutboundMsgSize,MaxSendRetryHours,LastProcessInterval,ProcessFreqMins,Description,ServerID,IsSSL,SSLMode,OfficeHours,ErrorCode,AccessToken,AccessTokenExpire,RefreshToken,RefreshTokenExpire,OAuthHostID) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT MessageDestinations OFF") );
		}
	}
	
	arc.AddIDMapping( TABLEID_MessageDestinations, ID, md.m_MessageDestinationID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffMessageSourceTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffMessageSourceTypeRecord( CArchiveFile& arc, long ID )
{
	TMessageSourceTypes mst;
	int ret;
	mst.m_MessageSourceTypeID = ID;

	// Query from the databse
	if( mst.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( mst.m_Description );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveMessageSourceTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveMessageSourceTypeRecord( CArchiveFile& arc, long ID )
{
	TMessageSourceTypes mst;
	int ret = Arc_Success;

	mst.m_MessageSourceTypeID = ID;

	GETCHAR( mst.m_Description, MESSAGESOURCETYPES_DESCRIPTION_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_MessageSourceTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( mst.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), mst.m_Description );
		BINDCOL_LONG( arc.GetQuery(), mst.m_MessageSourceTypeID );
		arc.GetQuery().Execute( _T("SELECT MessageSourceTypeID FROM MessageSourceTypes ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( mst.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//mst.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), mst.m_MessageSourceTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), mst.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT MessageSourceTypes ON INSERT INTO MessageSourceTypes ")
						_T("(MessageSourceTypeID,Description) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT MessageSourceTypes OFF") );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffMessageSourceRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffMessageSourceRecord( CArchiveFile& arc, long ID )
{
	TMessageSources ms;
	int ret;
	ms.m_MessageSourceID = ID;

	// Query from the databse
	if( ms.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( ms.m_RemoteAddress );
		PUTLONG( ms.m_RemotePort );
		PUTCHAR( ms.m_AuthUserName );
		PUTCHAR( ms.m_AuthPassword );
		PUTBYTE( ms.m_IsAPOP );
		PUTBYTE( ms.m_LeaveCopiesOnServer );
		PUTBYTE( ms.m_IsActive );
		PUTLONG( ms.m_CheckFreqMins );
		PUTLONG( ms.m_MaxInboundMsgSize );
		PUTLONG( ms.m_ConnTimeoutSecs );
		PUTDATE( ms.m_LastChecked );
		PUTLONG( ms.m_MessageDestinationID );
		PUTLONG( ms.m_MessageSourceTypeID );
		PUTCHAR( ms.m_Description );
		PUTLONG( ms.m_ServerID );
		PUTBYTE( ms.m_IsSSL );
		PUTLONG( ms.m_OfficeHours );
		PUTLONG( ms.m_ZipAttach );
		PUTLONG( ms.m_DupMsg );
		PUTLONG( ms.m_LeaveCopiesDays );
		PUTLONG( ms.m_SkipDownloadDays );
		PUTLONG( ms.m_DateFilters );
		PUTLONG( ms.m_ErrorCode );
		PUTTEXT( ms.m_AccessToken );
		PUTDATE( ms.m_AccessTokenExpire );
		PUTTEXT( ms.m_RefreshToken );
		PUTDATE( ms.m_RefreshTokenExpire );		
		PUTLONG( ms.m_OAuthHostID );		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveMessageSourceRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveMessageSourceRecord( CArchiveFile& arc, long ID )
{
	TMessageSources ms;
	int ret = Arc_Success;

	ms.m_MessageSourceID = ID;

	GETCHAR( ms.m_RemoteAddress, MESSAGESOURCES_REMOTEADDRESS_LENGTH );
	GETLONG( ms.m_RemotePort );
	GETCHAR( ms.m_AuthUserName, MESSAGESOURCES_AUTHUSERNAME_LENGTH );
	GETCHAR( ms.m_AuthPassword, MESSAGESOURCES_AUTHPASSWORD_LENGTH );
	GETBYTE( ms.m_IsAPOP );
	GETBYTE( ms.m_LeaveCopiesOnServer );
	GETBYTE( ms.m_IsActive );
	GETLONG( ms.m_CheckFreqMins );
	GETLONG( ms.m_MaxInboundMsgSize );
	GETLONG( ms.m_ConnTimeoutSecs );
	GETDATE( ms.m_LastChecked );
	GETLONG( ms.m_MessageDestinationID );
	arc.TranslateID( TABLEID_MessageDestinations, ms.m_MessageDestinationID );
	GETLONG( ms.m_MessageSourceTypeID );
	GETCHAR( ms.m_Description, MESSAGESOURCES_DESCRIPTION_LENGTH );
	GETLONG( ms.m_ServerID );
	GETBYTE( ms.m_IsSSL );
	GETLONG( ms.m_OfficeHours );
	GETLONG( ms.m_ZipAttach );
	GETLONG( ms.m_DupMsg );
	GETLONG( ms.m_LeaveCopiesDays );
	GETLONG( ms.m_SkipDownloadDays );
	GETLONG( ms.m_DateFilters );
	GETLONG( ms.m_ErrorCode );
	GETTEXT( ms.m_AccessToken );
	GETDATE( ms.m_AccessTokenExpire );
	GETTEXT( ms.m_RefreshToken );
	GETDATE( ms.m_RefreshTokenExpire );	
	GETLONG( ms.m_OAuthHostID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_MessageSources].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ms.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), ms.m_RemoteAddress );
		BINDPARAM_LONG( arc.GetQuery(), ms.m_RemotePort );
		BINDPARAM_TCHAR( arc.GetQuery(), ms.m_AuthUserName );
		BINDCOL_LONG( arc.GetQuery(), ms.m_MessageSourceID );
		arc.GetQuery().Execute( _T("SELECT MessageSourceID FROM MessageSources ")
								_T("WHERE RemoteAddress=? AND RemotePort=? AND AuthUserName=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ms.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ms.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ms.m_MessageSourceID );
			BINDPARAM_TCHAR( arc.GetQuery(), ms.m_RemoteAddress );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_RemotePort );
			BINDPARAM_TCHAR( arc.GetQuery(), ms.m_AuthUserName );
			BINDPARAM_TCHAR( arc.GetQuery(), ms.m_AuthPassword );
			BINDPARAM_BIT( arc.GetQuery(), ms.m_IsAPOP );
			BINDPARAM_BIT( arc.GetQuery(), ms.m_LeaveCopiesOnServer );
			BINDPARAM_BIT( arc.GetQuery(), ms.m_IsActive );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_CheckFreqMins );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_MaxInboundMsgSize );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_ConnTimeoutSecs );
			BINDPARAM_TIME( arc.GetQuery(), ms.m_LastChecked );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_MessageDestinationID );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_MessageSourceTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), ms.m_Description );
			BINDPARAM_BIT( arc.GetQuery(), ms.m_UseReplyTo );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_ServerID );
			BINDPARAM_BIT( arc.GetQuery(), ms.m_IsSSL );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_OfficeHours );			
			BINDPARAM_LONG( arc.GetQuery(), ms.m_ZipAttach );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_DupMsg );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_LeaveCopiesDays );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_SkipDownloadDays );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_DateFilters );
			BINDPARAM_LONG( arc.GetQuery(), ms.m_ErrorCode );
			BINDPARAM_TEXT( arc.GetQuery(), ms.m_AccessToken );
			BINDPARAM_TIME( arc.GetQuery(), ms.m_AccessTokenExpire );
			BINDPARAM_TEXT( arc.GetQuery(), ms.m_RefreshToken );
			BINDPARAM_TIME( arc.GetQuery(), ms.m_RefreshTokenExpire );			
			BINDPARAM_LONG( arc.GetQuery(), ms.m_OAuthHostID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT MessageSources ON INSERT INTO MessageSources ")
						_T("(MessageSourceID,RemoteAddress,RemotePort,AuthUserName,AuthPassword,IsAPOP,LeaveCopiesOnServer,IsActive,CheckFreqMins,MaxInboundMsgSize,ConnTimeoutSecs,LastChecked,MessageDestinationID,MessageSourceTypeID,Description,UseReplyTo,ServerID,IsSSL,OfficeHours,ZipAttach,DupMsg,LeaveCopiesDays,SkipDownloadDays,DateFilters,ErrorCode,AccessToken,AccessTokenExpire,RefreshToken,RefreshTokenExpire,OAuthHostID) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT MessageSources OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_MessageSources, ID, ms.m_MessageSourceID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffObjectTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffObjectTypeRecord( CArchiveFile& arc, long ID )
{
	TObjectTypes ot;
	int ret;
	ot.m_ObjectTypeID = ID;

	// Query from the databse
	if( ot.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( ot.m_Description );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveObjectTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveObjectTypeRecord( CArchiveFile& arc, long ID )
{
	TObjectTypes ot;
	int ret = Arc_Success;

	ot.m_ObjectTypeID = ID;

	GETCHAR( ot.m_Description, OBJECTTYPES_DESCRIPTION_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ObjectTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ot.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), ot.m_Description );
		BINDCOL_LONG( arc.GetQuery(), ot.m_ObjectTypeID );
		arc.GetQuery().Execute( _T("SELECT ObjectTypeID FROM ObjectTypes ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ot.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ot.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ot.m_ObjectTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), ot.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ObjectTypes ON INSERT INTO ObjectTypes ")
						_T("(ObjectTypeID,Description) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT ObjectTypes OFF") );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffObjectRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffObjectRecord( CArchiveFile& arc, long ID )
{
	TObjects o;
	int ret;
	o.m_ObjectID = ID;

	// Query from the databse
	if( o.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( o.m_ActualID );
		PUTLONG( o.m_ObjectTypeID );
		PUTBYTE( o.m_BuiltIn );
		PUTBYTE( o.m_UseDefaultRights );
		PUTDATE( o.m_DateCreated );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveObjectRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveObjectRecord( CArchiveFile& arc, long ID )
{
	TObjects o;
	int ret = Arc_Success;

	o.m_ObjectID = ID;

	GETLONG( o.m_ActualID );
	GETLONG( o.m_ObjectTypeID );
	GETBYTE( o.m_BuiltIn );
	GETBYTE( o.m_UseDefaultRights );
	GETDATE( o.m_DateCreated );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Objects].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( o.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), o.m_ObjectTypeID );
		BINDPARAM_TIME( arc.GetQuery(), o.m_DateCreated );
		BINDCOL_LONG( arc.GetQuery(), o.m_ObjectID );
		arc.GetQuery().Execute( _T("SELECT ObjectID FROM Objects ")
								_T("WHERE ObjectTypeID=? AND DateCreated=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( o.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//o.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), o.m_ObjectID );
			BINDPARAM_LONG( arc.GetQuery(), o.m_ActualID );
			BINDPARAM_LONG( arc.GetQuery(), o.m_ObjectTypeID );
			BINDPARAM_BIT( arc.GetQuery(), o.m_BuiltIn );
			BINDPARAM_TINYINT( arc.GetQuery(), o.m_UseDefaultRights );
			BINDPARAM_TIME( arc.GetQuery(), o.m_DateCreated );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects ")
						_T("(ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?) SET IDENTITY_INSERT Objects OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_Objects, ID, o.m_ObjectID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffOutboundMessageAttachmentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffOutboundMessageAttachmentRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessageAttachments oma;
	int ret;
	oma.m_OutboundMessageAttachmentID = ID;

	// Query from the databse
	if( oma.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( oma.m_OutboundMessageID );
		PUTLONG( oma.m_AttachmentID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveOutboundMessageAttachmentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveOutboundMessageAttachmentRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessageAttachments oma;
	int ret = Arc_Success;
	
	oma.m_OutboundMessageAttachmentID = ID;

	GETLONG( oma.m_OutboundMessageID );
	arc.TranslateID( TABLEID_OutboundMessages, oma.m_OutboundMessageID );
	GETLONG( oma.m_AttachmentID );
	arc.TranslateID( TABLEID_Attachments, oma.m_AttachmentID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_OutboundMessageAttachments].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( oma.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), oma.m_OutboundMessageID );
		BINDPARAM_LONG( arc.GetQuery(), oma.m_AttachmentID );
		BINDCOL_LONG( arc.GetQuery(), oma.m_OutboundMessageAttachmentID );
		arc.GetQuery().Execute( _T("SELECT OutboundMessageAttachmentID FROM OutboundMessageAttachments ")
								_T("WHERE OutboundMessageID=? AND AttachmentID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( oma.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			oma.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffOutboundMessageContactRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffOutboundMessageContactRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessageContacts omc;
	int ret;
	omc.m_OutboundMessageContactID = ID;

	// Query from the databse
	if( omc.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( omc.m_ContactID );
		PUTLONG( omc.m_OutboundMessageID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveOutboundMessageContactRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveOutboundMessageContactRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessageContacts omc;
	int ret = Arc_Success;

	omc.m_OutboundMessageContactID = ID;

	GETLONG( omc.m_ContactID );
	arc.TranslateID( TABLEID_Contacts, omc.m_ContactID );
	GETLONG( omc.m_OutboundMessageID );
	arc.TranslateID( TABLEID_OutboundMessages, omc.m_OutboundMessageID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_OutboundMessageContacts].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( omc.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), omc.m_OutboundMessageID );
		BINDPARAM_LONG( arc.GetQuery(), omc.m_ContactID );
		BINDCOL_LONG( arc.GetQuery(), omc.m_OutboundMessageContactID );
		arc.GetQuery().Execute( _T("SELECT OutboundMessageContactID FROM OutboundMessageContacts ")
								_T("WHERE OutboundMessageID=? AND ContactID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( omc.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			omc.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffOutboundMessageQueueRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffOutboundMessageQueueRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessageQueue omq;
	int ret;
	omq.m_OutBoundMessageQueueID = ID;

	// Query from the databse
	if( omq.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( omq.m_OutboundMessageID );
		PUTLONG( omq.m_MessageDestinationID );
		PUTBYTE( omq.m_SpoolFileGenerated );
		PUTBYTE( omq.m_IsApproved );
		PUTDATE( omq.m_DateSpooled );
		PUTDATE( omq.m_LastAttemptedDelivery );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveOutboundMessageQueueRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveOutboundMessageQueueRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessageQueue omq;
	int ret = Arc_Success;
	omq.m_OutBoundMessageQueueID = ID;

	GETLONG( omq.m_OutboundMessageID );
	arc.TranslateID( TABLEID_OutboundMessages, omq.m_OutboundMessageID );
	GETLONG( omq.m_MessageDestinationID );
	arc.TranslateID( TABLEID_MessageDestinations, omq.m_MessageDestinationID );
	GETBYTE( omq.m_SpoolFileGenerated );
	GETBYTE( omq.m_IsApproved );
	GETDATE( omq.m_DateSpooled );
	GETDATE( omq.m_LastAttemptedDelivery );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_OutboundMessageQueue].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( omq.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), omq.m_OutboundMessageID );
		BINDCOL_LONG( arc.GetQuery(), omq.m_OutBoundMessageQueueID );
		arc.GetQuery().Execute( _T("SELECT OutBoundMessageQueueID FROM OutboundMessageQueue ")
								_T("WHERE OutboundMessageID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( omq.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			omq.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffOutboundMessageStateRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffOutboundMessageStateRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessageStates oms;
	int ret;
	oms.m_OutboundMsgStateID = ID;

	// Query from the databse
	if( oms.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( oms.m_MessageStateName );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveOutboundMessageStateRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveOutboundMessageStateRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessageStates oms;
	int ret = Arc_Success;

	oms.m_OutboundMsgStateID = ID;

	GETCHAR( oms.m_MessageStateName, OUTBOUNDMESSAGESTATES_MESSAGESTATENAME_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_OutboundMessageStates].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( oms.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), oms.m_MessageStateName );
		BINDCOL_LONG( arc.GetQuery(), oms.m_OutboundMsgStateID );
		arc.GetQuery().Execute( _T("SELECT OutboundMsgStateID FROM OutboundMessageStates ")
								_T("WHERE MessageStateName=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( oms.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//oms.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), oms.m_OutboundMsgStateID );
			BINDPARAM_TCHAR( arc.GetQuery(), oms.m_MessageStateName );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT OutboundMessageStates ON INSERT INTO OutboundMessageStates ")
						_T("(OutboundMsgStateID,MessageStateName) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT OutboundMessageStates OFF") );
		}
	}

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffOutboundMessageTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffOutboundMessageTypeRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessageTypes omt;
	int ret;
	omt.m_OutboundMessageTypeID = ID;

	// Query from the databse
	if( omt.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( omt.m_Description );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveOutboundMessageTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveOutboundMessageTypeRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessageTypes omt;
	int ret = Arc_Success;

	omt.m_OutboundMessageTypeID = ID;

	GETCHAR( omt.m_Description, OUTBOUNDMESSAGETYPES_DESCRIPTION_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_OutboundMessageTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( omt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), omt.m_Description );
		BINDCOL_LONG( arc.GetQuery(), omt.m_OutboundMessageTypeID );
		arc.GetQuery().Execute( _T("SELECT OutboundMessageTypeID FROM OutboundMessageTypes ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( omt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//omt.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), omt.m_OutboundMessageTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), omt.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT OutboundMessageTypes ON INSERT INTO OutboundMessageTypes ")
						_T("(OutboundMessageTypeID,Description) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT OutboundMessageTypes OFF") );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffOutboundMessageRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffOutboundMessageRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessages om;
	int ret;
	om.m_OutboundMessageID = ID;

	// Query from the databse
	if( om.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( om.m_TicketID );
		PUTCHAR( om.m_EmailFrom );
		PUTCHAR( om.m_EmailPrimaryTo );
		PUTDATE( om.m_EmailDateTime );
		PUTCHAR( om.m_Subject );
		PUTCHAR( om.m_MediaType );
		PUTCHAR( om.m_MediaSubType );
		PUTLONG( om.m_DeletedBy );
		PUTDATE( om.m_DeletedTime );
		PUTBYTE( om.m_IsDeleted );
		PUTLONG( om.m_AgentID );
		PUTLONG( om.m_ReplyToMsgID );
		PUTBYTE( om.m_ReplyToIDIsInbound );
		PUTLONG( om.m_OutboundMessageTypeID );
		PUTLONG( om.m_OutboundMessageStateID );
		PUTTEXT( om.m_Body );
		PUTTEXT( om.m_EmailTo );
		PUTTEXT( om.m_EmailCc );
		PUTTEXT( om.m_EmailBcc );
		PUTTEXT( om.m_EmailReplyTo );
		PUTLONG( om.m_TicketBoxHeaderID );
		PUTLONG( om.m_TicketBoxFooterID );
		PUTLONG( om.m_PriorityID );
		PUTLONG( om.m_ArchiveID );
		PUTLONG( om.m_SignatureID );
		PUTLONG( om.m_OriginalTicketBoxID );
		PUTBYTE( om.m_DraftCloseTicket );
		PUTBYTE( om.m_DraftRouteToMe );
		PUTLONG( om.m_TicketCategoryID );
		PUTBYTE( om.m_FooterLocation );
		PUTBYTE( om.m_ReadReceipt );
		PUTBYTE( om.m_DeliveryConfirmation );
		PUTLONG( om.m_MultiMail );
				
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


int RetrieveOutboundMessageToStruct( CArchiveFile& arc, OutboundMessages_t& om )
{
	int ret = Arc_Success;

	GETLONG( om.m_TicketID );
	arc.TranslateID( TABLEID_Tickets, om.m_TicketID );
	GETCHAR( om.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH );
	GETCHAR( om.m_EmailPrimaryTo, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH );
	GETDATE( om.m_EmailDateTime );
	GETCHAR( om.m_Subject, OUTBOUNDMESSAGES_SUBJECT_LENGTH );
	GETCHAR( om.m_MediaType, OUTBOUNDMESSAGES_MEDIATYPE_LENGTH );
	GETCHAR( om.m_MediaSubType, OUTBOUNDMESSAGES_MEDIASUBTYPE_LENGTH );
	GETLONG( om.m_DeletedBy );
	arc.TranslateID( TABLEID_Agents, om.m_DeletedBy );
	GETDATE( om.m_DeletedTime );
	GETBYTE( om.m_IsDeleted );
	GETLONG( om.m_AgentID );
	arc.TranslateID( TABLEID_Agents, om.m_AgentID );
	GETLONG( om.m_ReplyToMsgID );
	GETBYTE( om.m_ReplyToIDIsInbound );
	GETLONG( om.m_OutboundMessageTypeID );
	GETLONG( om.m_OutboundMessageStateID );
	GETTEXT( om.m_Body );
	GETTEXT( om.m_EmailTo );
	GETTEXT( om.m_EmailCc );
	GETTEXT( om.m_EmailBcc );
	GETTEXT( om.m_EmailReplyTo );
	GETLONG( om.m_TicketBoxHeaderID );
	arc.TranslateID( TABLEID_TicketBoxHeaders, om.m_TicketBoxHeaderID );
	GETLONG( om.m_TicketBoxFooterID );
	arc.TranslateID( TABLEID_TicketBoxFooters, om.m_TicketBoxFooterID );
	GETLONG( om.m_PriorityID );
	GETLONG( om.m_ArchiveID );
	GETLONG( om.m_SignatureID );
	arc.TranslateID( TABLEID_Signatures, om.m_SignatureID );
	GETLONG( om.m_OriginalTicketBoxID );
	arc.TranslateID( TABLEID_TicketBoxes, om.m_OriginalTicketBoxID );
	
	if( arc.GetHdr().Version >= 0x02000007 )
	{
		GETBYTE( om.m_DraftCloseTicket );
		GETBYTE( om.m_DraftRouteToMe );
		GETLONG( om.m_TicketCategoryID );
		GETBYTE( om.m_FooterLocation );
		GETBYTE( om.m_ReadReceipt );
		GETBYTE( om.m_DeliveryConfirmation );
		GETLONG( om.m_MultiMail );	
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveOutboundMessageRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveOutboundMessageRecord( CArchiveFile& arc, long ID )
{
	TOutboundMessages om;
	int ret = Arc_Success;

	om.m_OutboundMessageID = ID;

	ret = RetrieveOutboundMessageToStruct( arc, om );

	if( ret != Arc_Success )
	{
		return ret;
	}

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_OutboundMessages].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( om.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), om.m_Subject );
		BINDPARAM_LONG( arc.GetQuery(), om.m_TicketID );
		BINDPARAM_TIME( arc.GetQuery(), om.m_EmailDateTime );
		BINDCOL_LONG( arc.GetQuery(), om.m_OutboundMessageID );
		arc.GetQuery().Execute( _T("SELECT OutboundMessageID FROM OutboundMessages ")
								_T("WHERE Subject=? AND TicketID=? AND EmailDateTime=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( om.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//om.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), om.m_OutboundMessageID );
			BINDPARAM_LONG( arc.GetQuery(), om.m_TicketID );
			BINDPARAM_TCHAR( arc.GetQuery(), om.m_EmailFrom );
			BINDPARAM_TCHAR( arc.GetQuery(), om.m_EmailPrimaryTo );
			BINDPARAM_TIME( arc.GetQuery(), om.m_EmailDateTime );
			BINDPARAM_TCHAR( arc.GetQuery(), om.m_Subject );
			BINDPARAM_TCHAR( arc.GetQuery(), om.m_MediaType );
			BINDPARAM_TCHAR( arc.GetQuery(), om.m_MediaSubType );
			BINDPARAM_LONG( arc.GetQuery(), om.m_DeletedBy );
			BINDPARAM_TIME( arc.GetQuery(), om.m_DeletedTime );
			BINDPARAM_TINYINT( arc.GetQuery(), om.m_IsDeleted );
			BINDPARAM_LONG( arc.GetQuery(), om.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), om.m_ReplyToMsgID );
			BINDPARAM_BIT( arc.GetQuery(), om.m_ReplyToIDIsInbound );
			BINDPARAM_LONG( arc.GetQuery(), om.m_OutboundMessageTypeID );
			BINDPARAM_LONG( arc.GetQuery(), om.m_OutboundMessageStateID );
			BINDPARAM_TEXT( arc.GetQuery(), om.m_Body );
			BINDPARAM_TEXT( arc.GetQuery(), om.m_EmailTo );
			BINDPARAM_TEXT( arc.GetQuery(), om.m_EmailCc );
			BINDPARAM_TEXT( arc.GetQuery(), om.m_EmailBcc );
			BINDPARAM_TEXT( arc.GetQuery(), om.m_EmailReplyTo );
			BINDPARAM_LONG( arc.GetQuery(), om.m_TicketBoxHeaderID );
			BINDPARAM_LONG( arc.GetQuery(), om.m_TicketBoxFooterID );
			BINDPARAM_LONG( arc.GetQuery(), om.m_PriorityID );
			BINDPARAM_LONG( arc.GetQuery(), om.m_ArchiveID );
			BINDPARAM_LONG( arc.GetQuery(), om.m_SignatureID );
			BINDPARAM_LONG( arc.GetQuery(), om.m_OriginalTicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), om.m_TicketCategoryID );
			BINDPARAM_TINYINT( arc.GetQuery(), om.m_DraftCloseTicket );
			BINDPARAM_TINYINT( arc.GetQuery(), om.m_DraftRouteToMe );
			BINDPARAM_TINYINT( arc.GetQuery(), om.m_FooterLocation );
			BINDPARAM_TINYINT( arc.GetQuery(), om.m_ReadReceipt );
			BINDPARAM_TINYINT( arc.GetQuery(), om.m_DeliveryConfirmation );
			BINDPARAM_LONG( arc.GetQuery(), om.m_MultiMail );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT OutboundMessages ON INSERT INTO OutboundMessages ")
						_T("(OutboundMessageID,TicketID,EmailFrom,EmailPrimaryTo,EmailDateTime,Subject,MediaType,MediaSubType,DeletedBy,DeletedTime,IsDeleted,AgentID,ReplyToMsgID,ReplyToIDIsInbound,OutboundMessageTypeID,OutboundMessageStateID,Body,EmailTo,EmailCc,EmailBcc,EmailReplyTo,TicketBoxHeaderID,TicketBoxFooterID,PriorityID,ArchiveID,SignatureID,OriginalTicketBoxID,TicketCategoryID,DraftCloseTicket,DraftRouteToMe,FooterLocation,ReadReceipt,DeliveryConfirmation,MultiMail) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT OutboundMessages OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_OutboundMessages, ID, om.m_OutboundMessageID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffPersonalDataRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffPersonalDataRecord( CArchiveFile& arc, long ID )
{
	TPersonalData pd;
	int ret;
	pd.m_PersonalDataID = ID;

	// Query from the databse
	if( pd.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( pd.m_ContactID );
		PUTLONG( pd.m_AgentID );
		PUTLONG( pd.m_PersonalDataTypeID );
		PUTCHAR( pd.m_DataValue );
		PUTCHAR( pd.m_Note );
		PUTLONG( pd.m_StatusID );
		PUTDATE( pd.m_StatusDate );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrievePersonalDataRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrievePersonalDataRecord( CArchiveFile& arc, long ID )
{
	TPersonalData pd;
	int ret = Arc_Success;

	pd.m_PersonalDataID = ID;

	GETLONG( pd.m_ContactID );
	arc.TranslateID( TABLEID_Contacts, pd.m_ContactID );
	GETLONG( pd.m_AgentID );
	arc.TranslateID( TABLEID_Agents, pd.m_AgentID );
	GETLONG( pd.m_PersonalDataTypeID );
	arc.TranslateID( TABLEID_PersonalDataTypes, pd.m_PersonalDataTypeID );
	GETCHAR( pd.m_DataValue, PERSONALDATA_DATAVALUE_LENGTH );
	GETCHAR( pd.m_Note, PERSONALDATA_NOTE_LENGTH );
	GETLONG( pd.m_StatusID );
	GETDATE( pd.m_StatusDate );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_PersonalData].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( pd.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		if( pd.m_AgentID > 0 )
		{
			arc.GetQuery().Initialize();
			BINDPARAM_TCHAR( arc.GetQuery(), pd.m_DataValue );
			BINDPARAM_LONG( arc.GetQuery(), pd.m_PersonalDataTypeID );
			BINDPARAM_LONG( arc.GetQuery(), pd.m_AgentID );
			BINDCOL_LONG( arc.GetQuery(), pd.m_PersonalDataID );
			arc.GetQuery().Execute( _T("SELECT PersonalDataID FROM PersonalData ")
									_T("WHERE DataValue=? AND PersonalDataTypeID=? AND AgentID=?") );		
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_TCHAR( arc.GetQuery(), pd.m_DataValue );
			BINDPARAM_LONG( arc.GetQuery(), pd.m_PersonalDataTypeID );
			BINDPARAM_LONG( arc.GetQuery(), pd.m_ContactID );
			BINDCOL_LONG( arc.GetQuery(), pd.m_PersonalDataID );
			arc.GetQuery().Execute( _T("SELECT PersonalDataID FROM PersonalData ")
									_T("WHERE DataValue=? AND PersonalDataTypeID=? AND ContactID=?") );
		}

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( pd.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//pd.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), pd.m_PersonalDataID );
			BINDPARAM_LONG( arc.GetQuery(), pd.m_ContactID );
			BINDPARAM_LONG( arc.GetQuery(), pd.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), pd.m_PersonalDataTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), pd.m_DataValue );
			BINDPARAM_TCHAR( arc.GetQuery(), pd.m_Note );
			BINDPARAM_LONG( arc.GetQuery(), pd.m_StatusID );
			BINDPARAM_TIME( arc.GetQuery(), pd.m_StatusDate );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT PersonalData ON INSERT INTO PersonalData ")
						_T("(PersonalDataID,ContactID,AgentID,PersonalDataTypeID,DataValue,Note,StatusID,StatusDate) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?) SET IDENTITY_INSERT PersonalData OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_PersonalData, ID, pd.m_PersonalDataID );

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffPersonalDataTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffPersonalDataTypeRecord( CArchiveFile& arc, long ID )
{
	TPersonalDataTypes pdt;
	int ret;
	pdt.m_PersonalDataTypeID = ID;

	// Query from the databse
	if( pdt.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( pdt.m_TypeName );
		PUTBYTE( pdt.m_BuiltIn );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrievePersonalDataTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrievePersonalDataTypeRecord( CArchiveFile& arc, long ID )
{
	TPersonalDataTypes pdt;
	int ret = Arc_Success;

	pdt.m_PersonalDataTypeID = ID;

	GETCHAR( pdt.m_TypeName, PERSONALDATATYPES_TYPENAME_LENGTH );
	GETBYTE( pdt.m_BuiltIn );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_PersonalDataTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( pdt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), pdt.m_TypeName );
		BINDCOL_LONG( arc.GetQuery(), pdt.m_PersonalDataTypeID );
		arc.GetQuery().Execute( _T("SELECT PersonalDataTypeID FROM PersonalDataTypes ")
								_T("WHERE TypeName=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( pdt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//pdt.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), pdt.m_PersonalDataTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), pdt.m_TypeName );
			BINDPARAM_BIT( arc.GetQuery(), pdt.m_BuiltIn );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT PersonalDataTypes ON INSERT INTO PersonalDataTypes ")
						_T("(PersonalDataTypeID,TypeName,BuiltIn) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT PersonalDataTypes OFF") );
		}
	}
	
	arc.AddIDMapping( TABLEID_PersonalDataTypes, ID, pdt.m_PersonalDataTypeID );

	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffBypassRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffBypassRecord( CArchiveFile& arc, long ID )
{
	TBypass b;
	int ret;
	b.m_BypassID = ID;

	// Query from the databse
	if( b.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( b.m_BypassTypeID );
		PUTCHAR( b.m_BypassValue );
		PUTDATE( b.m_DateCreated );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveBypassRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveBypassRecord( CArchiveFile& arc, long ID )
{
	TBypass b;
	int ret = Arc_Success;

	b.m_BypassID = ID;

	GETLONG( b.m_BypassTypeID );
	GETCHAR( b.m_BypassValue, PERSONALDATA_DATAVALUE_LENGTH );
	GETDATE( b.m_DateCreated );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Bypass].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( b.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), b.m_BypassValue );
		BINDPARAM_LONG( arc.GetQuery(), b.m_BypassTypeID );
		BINDCOL_LONG( arc.GetQuery(), b.m_BypassID );
		arc.GetQuery().Execute( _T("SELECT BypassID FROM Bypass ")
								_T("WHERE BypassValue=? AND BypassTypeID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( b.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//b.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), b.m_BypassID );
			BINDPARAM_LONG( arc.GetQuery(), b.m_BypassTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), b.m_BypassValue );
			BINDPARAM_TIME( arc.GetQuery(), b.m_DateCreated );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Bypass ON INSERT INTO Bypass ")
						_T("(BypassID,BypassTypeID,BypassValue,DateCreated) ")
						_T("VALUES")
						_T("(?,?,?,?) SET IDENTITY_INSERT Bypass OFF") );
		}
	}

	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffEmailRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffEmailRecord( CArchiveFile& arc, long ID )
{
	TEmail b;
	int ret;
	b.m_EmailID = ID;

	// Query from the databse
	if( b.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( b.m_EmailTypeID );
		PUTCHAR( b.m_EmailValue );
		PUTDATE( b.m_DateCreated );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveEmailRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveEmailRecord( CArchiveFile& arc, long ID )
{
	TEmail b;
	int ret = Arc_Success;

	b.m_EmailID = ID;

	GETLONG( b.m_EmailTypeID );
	GETCHAR( b.m_EmailValue, PERSONALDATA_DATAVALUE_LENGTH );
	GETDATE( b.m_DateCreated );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Email].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( b.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), b.m_EmailValue );
		BINDPARAM_LONG( arc.GetQuery(), b.m_EmailTypeID );
		BINDCOL_LONG( arc.GetQuery(), b.m_EmailID );
		arc.GetQuery().Execute( _T("SELECT EmailID FROM Email ")
								_T("WHERE EmailValue=? AND EmailTypeID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( b.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//b.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), b.m_EmailID );
			BINDPARAM_LONG( arc.GetQuery(), b.m_EmailTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), b.m_EmailValue );
			BINDPARAM_TIME( arc.GetQuery(), b.m_DateCreated );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Email ON INSERT INTO Email ")
						_T("(EmailID,EmailTypeID,EmailValue,DateCreated) ")
						_T("VALUES")
						_T("(?,?,?,?) SET IDENTITY_INSERT Email OFF") );
		}
	}

	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffPriorityRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffPriorityRecord( CArchiveFile& arc, long ID )
{
	TPriorities p;
	int ret;
	p.m_PriorityID = ID;

	// Query from the databse
	if( p.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( p.m_Name );
		PUTLONG( p.m_SortIndex );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrievePriorityRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrievePriorityRecord( CArchiveFile& arc, long ID )
{
	TPriorities p;
	int ret;
	p.m_PriorityID = ID;

	GETCHAR( p.m_Name, PRIORITIES_NAME_LENGTH );
	GETLONG( p.m_SortIndex );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Priorities].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( p.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), p.m_Name );
		BINDCOL_LONG( arc.GetQuery(), p.m_PriorityID );
		arc.GetQuery().Execute( _T("SELECT PriorityID FROM Priorities ")
								_T("WHERE Name=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( p.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//p.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), p.m_PriorityID );
			BINDPARAM_TCHAR( arc.GetQuery(), p.m_Name );
			BINDPARAM_LONG( arc.GetQuery(), p.m_SortIndex );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Priorities ON INSERT INTO Priorities ")
						_T("(PriorityID,Name,SortIndex) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT Priorities OFF") );
		}
	}
	
	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffRoutingRuleRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffRoutingRuleRecord( CArchiveFile& arc, long ID )
{
	TRoutingRules rr;
	int ret;
	rr.m_RoutingRuleID = ID;

	// Query from the databse
	if( rr.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( rr.m_RuleDescrip );
		PUTBYTE( rr.m_IsEnabled );
		PUTLONG( rr.m_OrderIndex );
		PUTBYTE( rr.m_AutoReplyEnable );
		PUTLONG( rr.m_AutoReplyWithStdResponse );
		PUTBYTE( rr.m_AutoReplyQuoteMsg );
		PUTBYTE( rr.m_AutoReplyCloseTicket );
		PUTBYTE( rr.m_ForwardEnable );
		PUTLONG( rr.m_ForwardFromAgent );
		PUTBYTE( rr.m_ForwardFromContact );
		PUTLONG( rr.m_AssignToTicketBox );
		PUTBYTE( rr.m_AssignToAgentEnable );
		PUTLONG( rr.m_AssignToAgent );
		PUTBYTE( rr.m_DeleteImmediatelyEnable );
		PUTBYTE( rr.m_DeleteImmediately );
		PUTLONG( rr.m_HitCount );
		PUTLONG( rr.m_PriorityID );
		PUTLONG( rr.m_MessageSourceTypeID );
		PUTCHAR( rr.m_AutoReplyFrom );
		PUTLONG( rr.m_AssignToAgentAlg );
		PUTLONG( rr.m_AssignToTicketBoxAlg );
		PUTBYTE( rr.m_AssignUniqueTicketID );
		PUTLONG( rr.m_AssignToTicketCategory);
		PUTBYTE( rr.m_AlertEnable );
		PUTBYTE( rr.m_AlertIncludeSubject );
		PUTLONG( rr.m_AlertToAgentID );
		PUTCHAR( rr.m_AlertText );
		PUTBYTE( rr.m_ToOrFrom );
		PUTBYTE( rr.m_ConsiderAllOwned );
		PUTBYTE( rr.m_DoProcessingRules );
		PUTLONG( rr.m_LastOwnerID );
		PUTLONG( rr.m_MatchMethod );
		PUTCHAR( rr.m_ForwardFromEmail );
		PUTCHAR( rr.m_ForwardFromName );
		PUTBYTE( rr.m_ForwardInTicket );	
		PUTBYTE( rr.m_AutoReplyInTicket );
		PUTBYTE( rr.m_AllowRemoteReply );
		PUTLONG( rr.m_DoNotAssign );
		PUTBYTE( rr.m_QuoteOriginal );
		PUTLONG( rr.m_MultiMail );
		PUTLONG( rr.m_SetOpenOwner );
		PUTLONG( rr.m_OfficeHours );
		PUTLONG( rr.m_IgnoreTracking );
		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveRoutingRuleRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveRoutingRuleRecord( CArchiveFile& arc, long ID )
{
	TRoutingRules rr;
	int ret = Arc_Success;

	rr.m_RoutingRuleID = ID;

	GETCHAR( rr.m_RuleDescrip, ROUTINGRULES_RULEDESCRIP_LENGTH );
	GETBYTE( rr.m_IsEnabled );
	GETLONG( rr.m_OrderIndex );
	GETBYTE( rr.m_AutoReplyEnable );
	GETLONG( rr.m_AutoReplyWithStdResponse );
	arc.TranslateID( TABLEID_StandardResponses, rr.m_AutoReplyWithStdResponse );
	GETBYTE( rr.m_AutoReplyQuoteMsg );
	GETBYTE( rr.m_AutoReplyCloseTicket );
	GETBYTE( rr.m_ForwardEnable );
	GETLONG( rr.m_ForwardFromAgent );
	arc.TranslateID( TABLEID_Agents, rr.m_ForwardFromAgent );
	GETBYTE( rr.m_ForwardFromContact );
	GETLONG( rr.m_AssignToTicketBox );
	arc.TranslateID( TABLEID_TicketBoxes, rr.m_AssignToTicketBox );
	GETBYTE( rr.m_AssignToAgentEnable );
	GETLONG( rr.m_AssignToAgent );
	arc.TranslateID( TABLEID_Agents, rr.m_AssignToAgent );
	GETBYTE( rr.m_DeleteImmediatelyEnable );
	GETBYTE( rr.m_DeleteImmediately );
	GETLONG( rr.m_HitCount );
	GETLONG( rr.m_PriorityID );
	GETLONG( rr.m_MessageSourceTypeID );
	arc.TranslateID( TABLEID_MessageSources, rr.m_MessageSourceTypeID );
	GETCHAR( rr.m_AutoReplyFrom, ROUTINGRULES_AUTOREPLYFROM_LENGTH );
	GETLONG( rr.m_AssignToAgentAlg );
	GETLONG( rr.m_AssignToTicketBoxAlg );
	GETBYTE( rr.m_AssignUniqueTicketID );
	GETLONG( rr.m_AssignToTicketCategory);
	GETBYTE( rr.m_AlertEnable );
	GETBYTE( rr.m_AlertIncludeSubject );
	GETLONG( rr.m_AlertToAgentID );
	GETCHAR( rr.m_AlertText, 255 );
	GETBYTE( rr.m_ToOrFrom );
	GETBYTE( rr.m_ConsiderAllOwned );
	GETBYTE( rr.m_DoProcessingRules );
	GETLONG( rr.m_LastOwnerID );
	GETLONG( rr.m_MatchMethod );
	GETCHAR( rr.m_ForwardFromEmail, ROUTINGRULES_RULEDESCRIP_LENGTH );
	GETCHAR( rr.m_ForwardFromName, ROUTINGRULES_RULEDESCRIP_LENGTH );
	GETBYTE( rr.m_ForwardInTicket );
	GETBYTE( rr.m_AutoReplyInTicket );
	GETBYTE( rr.m_AllowRemoteReply );
	GETLONG( rr.m_DoNotAssign );
	GETBYTE( rr.m_QuoteOriginal );
	GETLONG( rr.m_MultiMail );
	GETLONG( rr.m_SetOpenOwner );
	GETLONG( rr.m_OfficeHours );
	GETLONG( rr.m_IgnoreTracking );
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_RoutingRules].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( rr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), rr.m_RuleDescrip );
		BINDPARAM_LONG( arc.GetQuery(), rr.m_AssignToTicketBox );
		BINDPARAM_LONG( arc.GetQuery(), rr.m_MessageSourceTypeID );
		BINDPARAM_LONG( arc.GetQuery(), rr.m_OrderIndex );
		BINDCOL_LONG( arc.GetQuery(), rr.m_RoutingRuleID );
		arc.GetQuery().Execute( _T("SELECT RoutingRuleID FROM RoutingRules ")
								_T("WHERE RuleDescrip=? AND AssignToTicketBox=? AND MessageSourceTypeID=? AND OrderIndex=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( rr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//rr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), rr.m_RoutingRuleID );
			BINDPARAM_TCHAR( arc.GetQuery(), rr.m_RuleDescrip );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_IsEnabled );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_OrderIndex );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_AutoReplyEnable );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_AutoReplyWithStdResponse );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_AutoReplyQuoteMsg );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_AutoReplyCloseTicket );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_ForwardEnable );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_ForwardFromAgent );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_ForwardFromContact );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_AssignToTicketBox );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_AssignToAgentEnable );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_AssignToAgent );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_DeleteImmediatelyEnable );
			BINDPARAM_TINYINT( arc.GetQuery(), rr.m_DeleteImmediately );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_HitCount );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_PriorityID );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_MessageSourceTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), rr.m_AutoReplyFrom );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_AssignToAgentAlg );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_AssignToTicketBoxAlg );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_AssignUniqueTicketID);
			BINDPARAM_LONG( arc.GetQuery(), rr.m_AssignToTicketCategory);
			BINDPARAM_BIT( arc.GetQuery(), rr.m_AlertEnable );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_AlertIncludeSubject );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_AlertToAgentID );
			BINDPARAM_TCHAR( arc.GetQuery(), rr.m_AlertText );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_ToOrFrom );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_ConsiderAllOwned );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_DoProcessingRules );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_LastOwnerID );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_MatchMethod );
			BINDPARAM_TCHAR( arc.GetQuery(), rr.m_ForwardFromEmail );
			BINDPARAM_TCHAR( arc.GetQuery(), rr.m_ForwardFromName );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_ForwardInTicket );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_AutoReplyInTicket );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_AllowRemoteReply );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_DoNotAssign );
			BINDPARAM_BIT( arc.GetQuery(), rr.m_QuoteOriginal );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_MultiMail );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_SetOpenOwner );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_OfficeHours );
			BINDPARAM_LONG( arc.GetQuery(), rr.m_IgnoreTracking );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT RoutingRules ON INSERT INTO RoutingRules ")
						_T("(RoutingRuleID,RuleDescrip,IsEnabled,OrderIndex,AutoReplyEnable,AutoReplyWithStdResponse,AutoReplyQuoteMsg,AutoReplyCloseTicket,ForwardEnable,ForwardFromAgent,ForwardFromContact,AssignToTicketBox,AssignToAgentEnable,AssignToAgent,DeleteImmediatelyEnable,DeleteImmediately,HitCount,PriorityID,MessageSourceTypeID,AutoReplyFrom,AssignToAgentAlg,AssignToTicketBoxAlg, AssignUniqueTicketID, AssignToTicketCategory, AlertEnable, AlertIncludeSubject, AlertToAgentID, AlertText, ToOrFrom, ConsiderAllOwned, DoProcessingRules, LastOwnerID,MatchMethod,ForwardFromEmail,ForwardFromName,ForwardInTicket,AutoReplyInTicket,AllowRemoteReply,DoNotAssign,QuoteOriginal,MultiMail,SetOpenOwner,OfficeHours,IgnoreTracking) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT RoutingRules OFF") );
		}
	}
	
	arc.AddIDMapping( TABLEID_RoutingRules, ID, rr.m_RoutingRuleID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffServerParameters
// 
////////////////////////////////////////////////////////////////////////////////
int StuffServerParameterRecord( CArchiveFile& arc, long ID )
{
	TServerParameters sp;
	int ret;
	sp.m_ServerParameterID = ID;

	// Query from the databse
	if( sp.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( sp.m_Description );
		PUTCHAR( sp.m_DataValue );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveServerParameters
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveServerParameterRecord( CArchiveFile& arc, long ID )
{
	TServerParameters sp;
	int ret = Arc_Success;

	sp.m_ServerParameterID = ID;

	GETCHAR( sp.m_Description, SERVERPARAMETERS_DESCRIPTION_LENGTH );
	GETCHAR( sp.m_DataValue, SERVERPARAMETERS_DATAVALUE_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ServerParameters].ReservedID ) 
	{ 
		// don't update the database version
		if ( ID != EMS_SRVPARAM_DATABASE_VERSION )
			ret = ( sp.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), sp.m_Description );
		BINDCOL_LONG( arc.GetQuery(), sp.m_ServerParameterID );
		arc.GetQuery().Execute( _T("SELECT ServerParameterID FROM ServerParameters ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( sp.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//sp.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), sp.m_ServerParameterID );
			BINDPARAM_TCHAR( arc.GetQuery(), sp.m_Description );
			BINDPARAM_TCHAR( arc.GetQuery(), sp.m_DataValue );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ServerParameters ON INSERT INTO ServerParameters ")
						_T("(ServerParameterID,Description,DataValue) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT ServerParameters OFF") );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffSignatureRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffSignatureRecord( CArchiveFile& arc, long ID )
{
	TSignatures sig;
	int ret;
	sig.m_SignatureID = ID;

	// Query from the databse
	if( sig.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( sig.m_GroupID );
		PUTLONG( sig.m_AgentID );
		PUTLONG( sig.m_ObjectTypeID );
		PUTLONG( sig.m_ActualID );
		PUTCHAR( sig.m_Name );
		PUTTEXT( sig.m_Signature );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveSignatureRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveSignatureRecord( CArchiveFile& arc, long ID )
{
	TSignatures sig;
	int ret = Arc_Success;

	sig.m_SignatureID = ID;

	GETLONG( sig.m_GroupID );
	arc.TranslateID( TABLEID_Groups, sig.m_GroupID );
	GETLONG( sig.m_AgentID );
	arc.TranslateID( TABLEID_Agents, sig.m_AgentID );
	GETLONG( sig.m_ObjectTypeID );
	GETLONG( sig.m_ActualID );
	GETCHAR( sig.m_Name, SIGNATURES_NAME_LENGTH );
	GETTEXT( sig.m_Signature );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Signatures].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( sig.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), sig.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), sig.m_GroupID );
		BINDPARAM_TCHAR( arc.GetQuery(), sig.m_Name );
		BINDCOL_LONG( arc.GetQuery(), sig.m_SignatureID );
		arc.GetQuery().Execute( _T("SELECT SignatureID FROM Signatures ")
								_T("WHERE AgentID=? AND GroupID=? AND Name=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( sig.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//sig.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), sig.m_SignatureID );
			BINDPARAM_LONG( arc.GetQuery(), sig.m_GroupID );
			BINDPARAM_LONG( arc.GetQuery(), sig.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), sig.m_ObjectTypeID );
			BINDPARAM_LONG( arc.GetQuery(), sig.m_ActualID );
			BINDPARAM_TCHAR( arc.GetQuery(), sig.m_Name );
			BINDPARAM_TEXT( arc.GetQuery(), sig.m_Signature );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Signatures ON INSERT INTO Signatures ")
						_T("(SignatureID,GroupID,AgentID,ObjectTypeID,ActualID,Name,Signature) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?) SET IDENTITY_INSERT Signatures OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_Signatures, ID, sig.m_SignatureID );

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffStdRespUsageRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffStdRespUsageRecord( CArchiveFile& arc, long ID )
{
	TStandardResponseUsage sru;
	int ret;
	sru.m_StdResponseUsageID = ID;

	// Query from the databse
	if( sru.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( sru.m_StandardResponseID );
		PUTLONG( sru.m_AgentID );
		PUTDATE( sru.m_DateUsed );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveStdRespUsageRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveStdRespUsageRecord( CArchiveFile& arc, long ID )
{
	TStandardResponseUsage sru;
	int ret = Arc_Success;

	sru.m_StdResponseUsageID = ID;

	GETLONG( sru.m_StandardResponseID );
	arc.TranslateID( TABLEID_StandardResponses, sru.m_StandardResponseID );
	GETLONG( sru.m_AgentID );
	arc.TranslateID( TABLEID_Agents, sru.m_AgentID );
	GETDATE( sru.m_DateUsed );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_StandardResponseUsage].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( sru.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), sru.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), sru.m_StandardResponseID );
		BINDPARAM_TIME( arc.GetQuery(), sru.m_DateUsed );
		BINDCOL_LONG( arc.GetQuery(), sru.m_StdResponseUsageID );
		arc.GetQuery().Execute( _T("SELECT StdResponseUsageID FROM StandardResponseUsage ")
								_T("WHERE AgentID=? AND StandardResponseID=? AND DateUsed=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( sru.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			sru.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}



////////////////////////////////////////////////////////////////////////////////
// 
// StuffStandardResponseRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffStandardResponseRecord( CArchiveFile& arc, long ID )
{
	TStandardResponses sr;
	int ret;
	sr.m_StandardResponseID = ID;

	// Query from the databse
	if( sr.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( sr.m_Subject );
		PUTLONG( sr.m_StdResponseCatID );
		PUTTEXT( sr.m_StandardResponse );
		PUTBYTE( sr.m_IsApproved );
		PUTLONG( sr.m_AgentID );
		PUTDATE( sr.m_DateCreated );
		PUTBYTE( sr.m_IsDeleted );
		PUTDATE( sr.m_DeletedTime );
		PUTLONG( sr.m_DeletedBy );
		PUTTEXT( sr.m_Note );
		PUTDATE( sr.m_DateModified );
		PUTLONG( sr.m_ModifiedBy );
		PUTBYTE( sr.m_UseKeywords );
		PUTCHAR( sr.m_Keywords );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveStandardResponseRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveStandardResponseRecord( CArchiveFile& arc, long ID )
{
	TStandardResponses sr;
	int ret = Arc_Success;

	sr.m_StandardResponseID = ID;

	GETCHAR( sr.m_Subject, STANDARDRESPONSES_SUBJECT_LENGTH );
	GETLONG( sr.m_StdResponseCatID );
	arc.TranslateID( TABLEID_StdResponseCategories, sr.m_StdResponseCatID );
	GETTEXT( sr.m_StandardResponse );
	GETBYTE( sr.m_IsApproved );
	GETLONG( sr.m_AgentID );
	arc.TranslateID( TABLEID_Agents, sr.m_AgentID );
	GETDATE( sr.m_DateCreated );
	GETBYTE( sr.m_IsDeleted );
	GETDATE( sr.m_DeletedTime );
	GETLONG( sr.m_DeletedBy );
	arc.TranslateID( TABLEID_Agents, sr.m_DeletedBy );
	GETTEXT( sr.m_Note );
	GETDATE( sr.m_DateModified );
	GETLONG( sr.m_ModifiedBy );
	arc.TranslateID( TABLEID_Agents, sr.m_ModifiedBy );
	GETBYTE( sr.m_UseKeywords );
	GETCHAR( sr.m_Keywords, 255 );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_StandardResponses].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( sr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), sr.m_Subject );
		BINDPARAM_TIME( arc.GetQuery(), sr.m_DateCreated );
		BINDPARAM_LONG( arc.GetQuery(), sr.m_StdResponseCatID );
		BINDCOL_LONG( arc.GetQuery(), sr.m_StandardResponseID );
		arc.GetQuery().Execute( _T("SELECT StandardResponseID FROM StandardResponses ")
								_T("WHERE Subject=? AND DateCreated=? AND StdResponseCatID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( sr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//sr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), sr.m_StandardResponseID );
			BINDPARAM_TCHAR( arc.GetQuery(), sr.m_Subject );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_StdResponseCatID );
			BINDPARAM_TEXT( arc.GetQuery(), sr.m_StandardResponse );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_IsApproved );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_AgentID );
			BINDPARAM_TIME( arc.GetQuery(), sr.m_DateCreated );
			BINDPARAM_TINYINT( arc.GetQuery(), sr.m_IsDeleted );
			BINDPARAM_TIME( arc.GetQuery(), sr.m_DeletedTime );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_DeletedBy );
			BINDPARAM_TEXT( arc.GetQuery(), sr.m_Note );
			BINDPARAM_TIME( arc.GetQuery(), sr.m_DateModified );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_ModifiedBy );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_UseKeywords );
			BINDPARAM_TCHAR( arc.GetQuery(), sr.m_Keywords );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT StandardResponses ON INSERT INTO StandardResponses ")
						_T("(StandardResponseID,Subject,StdResponseCatID,StandardResponse,IsApproved,AgentID,DateCreated,IsDeleted,DeletedTime,DeletedBy,Note,DateModified,ModifiedBy,UseKeywords,Keywords) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT StandardResponses OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_StandardResponses, ID, sr.m_StandardResponseID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffStdResponseAttachmentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffStdResponseAttachmentRecord( CArchiveFile& arc, long ID )
{
	TStdResponseAttachments sra;
	int ret;
	sra.m_StdResponseAttachID = ID;

	// Query from the databse
	if( sra.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( sra.m_StandardResponseID );
		PUTLONG( sra.m_AttachmentID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveStdResponseAttachmentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveStdResponseAttachmentRecord( CArchiveFile& arc, long ID )
{
	TStdResponseAttachments sra;
	int ret = Arc_Success;

	sra.m_StdResponseAttachID = ID;

	GETLONG( sra.m_StandardResponseID );
	arc.TranslateID( TABLEID_StandardResponses, sra.m_StandardResponseID );
	GETLONG( sra.m_AttachmentID );
	arc.TranslateID( TABLEID_Attachments, sra.m_AttachmentID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_StdResponseAttachments].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( sra.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), sra.m_StandardResponseID );
		BINDPARAM_LONG( arc.GetQuery(), sra.m_AttachmentID );
		BINDCOL_LONG( arc.GetQuery(), sra.m_StdResponseAttachID );
		arc.GetQuery().Execute( _T("SELECT StdResponseAttachID FROM StdResponseAttachments ")
								_T("WHERE StandardResponseID=? AND AttachmentID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( sra.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			sra.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffStdResponseCatRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffStdResponseCatRecord( CArchiveFile& arc, long ID )
{
	TStdResponseCategories src;
	int ret;
	src.m_StdResponseCatID = ID;

	// Query from the databse
	if( src.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( src.m_CategoryName );
		PUTLONG(src.m_ObjectID);
		PUTBYTE(src.m_BuiltIn);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveStdResponseCatRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveStdResponseCatRecord( CArchiveFile& arc, long ID )
{
	TStdResponseCategories src;
	int ret = Arc_Success;

	src.m_StdResponseCatID = ID;

	GETCHAR( src.m_CategoryName, STDRESPONSECATEGORIES_CATEGORYNAME_LENGTH );
	GETLONG(src.m_ObjectID);
	GETBYTE(src.m_BuiltIn);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_StdResponseCategories].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( src.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), src.m_CategoryName );
		BINDCOL_LONG( arc.GetQuery(), src.m_StdResponseCatID );
		arc.GetQuery().Execute( _T("SELECT StdResponseCatID FROM StdResponseCategories ")
								_T("WHERE CategoryName=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( src.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//src.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), src.m_StdResponseCatID );
			BINDPARAM_TCHAR( arc.GetQuery(), src.m_CategoryName );
			BINDPARAM_LONG( arc.GetQuery(), src.m_ObjectID );
			BINDPARAM_BIT( arc.GetQuery(), src.m_BuiltIn );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT StdResponseCategories ON INSERT INTO StdResponseCategories ")
						_T("(StdResponseCatID,CategoryName,ObjectID,BuiltIn) ")
						_T("VALUES")
						_T("(?,?,?,?) SET IDENTITY_INSERT StdResponseCategories OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_StdResponseCategories, ID, src.m_StdResponseCatID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffStdResponseFavoriteRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffStdResponseFavoriteRecord( CArchiveFile& arc, long ID )
{
	TStdResponseFavorites srf;
	int ret;
	srf.m_StdResponseFavoritesID = ID;

	// Query from the databse
	if( srf.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( srf.m_AgentID );
		PUTLONG( srf.m_StandardResponseID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveStdResponseFavoriteRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveStdResponseFavoriteRecord( CArchiveFile& arc, long ID )
{
	TStdResponseFavorites srf;
	int ret = Arc_Success;
	
	srf.m_StdResponseFavoritesID = ID;

	GETLONG( srf.m_AgentID );
	arc.TranslateID( TABLEID_Agents, srf.m_AgentID );
	GETLONG( srf.m_StandardResponseID );
	arc.TranslateID( TABLEID_StandardResponses, srf.m_StandardResponseID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_StdResponseFavorites].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( srf.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), srf.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), srf.m_StandardResponseID );
		BINDCOL_LONG( arc.GetQuery(), srf.m_StdResponseFavoritesID );
		arc.GetQuery().Execute( _T("SELECT StdResponseFavoritesID FROM StdResponseFavorites ")
								_T("WHERE AgentID=? AND StandardResponseID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( srf.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			srf.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffStyleSheetRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffStyleSheetRecord( CArchiveFile& arc, long ID )
{
	TStyleSheets ss;
	int ret;
	ss.m_StyleSheetID = ID;

	// Query from the databse
	if( ss.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( ss.m_Name );
		PUTCHAR( ss.m_Filename );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveStyleSheetRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveStyleSheetRecord( CArchiveFile& arc, long ID )
{
	TStyleSheets ss;
	int ret = Arc_Success;

	ss.m_StyleSheetID = ID;

	GETCHAR( ss.m_Name, STYLESHEETS_NAME_LENGTH );
	GETCHAR( ss.m_Filename, STYLESHEETS_FILENAME_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_StyleSheets].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ss.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), ss.m_Name );
		BINDCOL_LONG( arc.GetQuery(), ss.m_StyleSheetID );
		arc.GetQuery().Execute( _T("SELECT StyleSheetID FROM StyleSheets ")
								_T("WHERE Name=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ss.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ss.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ss.m_StyleSheetID );
			BINDPARAM_TCHAR( arc.GetQuery(), ss.m_Name );
			BINDPARAM_TCHAR( arc.GetQuery(), ss.m_Filename );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT StyleSheets ON INSERT INTO StyleSheets ")
						_T("(StyleSheetID,Name,Filename) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT StyleSheets OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_StyleSheets, ID, ss.m_StyleSheetID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketBoxFooterRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketBoxFooterRecord( CArchiveFile& arc, long ID )
{
	TTicketBoxFooters tbf;
	int ret;
	tbf.m_FooterID = ID;

	// Query from the databse
	if( tbf.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( tbf.m_Description );
		PUTTEXT( tbf.m_Footer );
		PUTBYTE( tbf.m_IsDeleted );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketBoxFooterRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketBoxFooterRecord( CArchiveFile& arc, long ID )
{
	TTicketBoxFooters tbf;
	int ret = Arc_Success;

	tbf.m_FooterID = ID;

	GETCHAR( tbf.m_Description, TICKETBOXFOOTERS_DESCRIPTION_LENGTH );
	GETTEXT( tbf.m_Footer );
	GETBYTE( tbf.m_IsDeleted );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketBoxFooters].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tbf.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tbf.m_Description );
		BINDCOL_LONG( arc.GetQuery(), tbf.m_FooterID );
		arc.GetQuery().Execute( _T("SELECT FooterID FROM TicketBoxFooters ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tbf.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//tbf.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tbf.m_FooterID );
			BINDPARAM_TCHAR( arc.GetQuery(), tbf.m_Description );
			BINDPARAM_TEXT( arc.GetQuery(), tbf.m_Footer );
			BINDPARAM_TINYINT( arc.GetQuery(), tbf.m_IsDeleted );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketBoxFooters ON INSERT INTO TicketBoxFooters ")
						_T("(FooterID,Description,Footer,IsDeleted) ")
						_T("VALUES")
						_T("(?,?,?,?) SET IDENTITY_INSERT TicketBoxFooters OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_TicketBoxFooters, ID, tbf.m_FooterID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketBoxHeaderRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketBoxHeaderRecord( CArchiveFile& arc, long ID )
{
	TTicketBoxHeaders tbh;
	int ret;
	tbh.m_HeaderID = ID;

	// Query from the databse
	if( tbh.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( tbh.m_Description );
		PUTTEXT( tbh.m_Header );
		PUTBYTE( tbh.m_IsDeleted );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketBoxFooterRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketBoxHeaderRecord( CArchiveFile& arc, long ID )
{
	TTicketBoxHeaders tbh;
	int ret= Arc_Success;

	tbh.m_HeaderID = ID;

	GETCHAR( tbh.m_Description, TICKETBOXHEADERS_DESCRIPTION_LENGTH );
	GETTEXT( tbh.m_Header );
	GETBYTE( tbh.m_IsDeleted );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketBoxHeaders].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tbh.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tbh.m_Description );
		BINDCOL_LONG( arc.GetQuery(), tbh.m_HeaderID );
		arc.GetQuery().Execute( _T("SELECT HeaderID FROM TicketBoxHeaders ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tbh.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//tbh.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tbh.m_HeaderID );
			BINDPARAM_TCHAR( arc.GetQuery(), tbh.m_Description );
			BINDPARAM_TEXT( arc.GetQuery(), tbh.m_Header );
			BINDPARAM_TINYINT( arc.GetQuery(), tbh.m_IsDeleted );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketBoxHeaders ON INSERT INTO TicketBoxHeaders ")
						_T("(HeaderID,Description,Header,IsDeleted) ")
						_T("VALUES")
						_T("(?,?,?,?) SET IDENTITY_INSERT TicketBoxHeaders OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_TicketBoxHeaders, ID, tbh.m_HeaderID );
	
	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketBoxViewTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketBoxViewTypeRecord( CArchiveFile& arc, long ID )
{
	TTicketBoxViewTypes tbvt;
	int ret;
	tbvt.m_TicketBoxViewTypeID = ID;

	// Query from the databse
	if( tbvt.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( tbvt.m_Name );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketBoxViewTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketBoxViewTypeRecord( CArchiveFile& arc, long ID )
{
	TTicketBoxViewTypes tbvt;
	int ret = Arc_Success;

	tbvt.m_TicketBoxViewTypeID = ID;

	GETCHAR( tbvt.m_Name, TICKETBOXVIEWTYPES_NAME_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketBoxViewTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tbvt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tbvt.m_Name );
		BINDCOL_LONG( arc.GetQuery(), tbvt.m_TicketBoxViewTypeID );
		arc.GetQuery().Execute( _T("SELECT TicketBoxViewTypeID FROM TicketBoxViewTypes ")
								_T("WHERE Name=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tbvt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//tbvt.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tbvt.m_TicketBoxViewTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), tbvt.m_Name );			
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketBoxViewTypes ON INSERT INTO TicketBoxViewTypes ")
						_T("(TicketBoxViewTypeID,Name) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT TicketBoxViewTypes OFF") );

		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketBoxViewRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketBoxViewRecord( CArchiveFile& arc, long ID )
{
	TTicketBoxViews tbv;
	int ret;
	tbv.m_TicketBoxViewID = ID;

	// Query from the databse
	if( tbv.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( tbv.m_AgentID );
		PUTLONG( tbv.m_TicketBoxID );
		PUTLONG( tbv.m_AgentBoxID );
		PUTBYTE( tbv.m_ShowOwnedItems );
		PUTBYTE( tbv.m_ShowClosedItems );
		PUTBYTE( tbv.m_SortAscending );
		PUTLONG( tbv.m_SortField );
		PUTLONG( tbv.m_TicketBoxViewTypeID );
		PUTBYTE( tbv.m_ShowState );
		PUTBYTE( tbv.m_ShowPriority );
		PUTBYTE( tbv.m_ShowNumNotes );
		PUTBYTE( tbv.m_ShowTicketID );
		PUTBYTE( tbv.m_ShowNumMsgs );
		PUTBYTE( tbv.m_ShowSubject );
		PUTBYTE( tbv.m_ShowContact );
		PUTBYTE( tbv.m_ShowDate );
		PUTBYTE( tbv.m_ShowCategory );
		PUTBYTE( tbv.m_ShowOwner );
		PUTBYTE( tbv.m_ShowTicketBox );
		PUTBYTE( tbv.m_UseDefault );			
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketBoxViewRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketBoxViewRecord( CArchiveFile& arc, long ID )
{
	TTicketBoxViews tbv;
	int ret = Arc_Success;

	tbv.m_TicketBoxViewID = ID;

	GETLONG( tbv.m_AgentID );
	arc.TranslateID( TABLEID_Agents, tbv.m_AgentID );
	GETLONG( tbv.m_TicketBoxID );
	arc.TranslateID( TABLEID_TicketBoxes, tbv.m_TicketBoxID );
	GETLONG( tbv.m_AgentBoxID );
	arc.TranslateID( TABLEID_Agents, tbv.m_AgentBoxID );
	GETBYTE( tbv.m_ShowOwnedItems );
	GETBYTE( tbv.m_ShowClosedItems );
	GETBYTE( tbv.m_SortAscending );
	GETLONG( tbv.m_SortField );
	GETLONG( tbv.m_TicketBoxViewTypeID );
	GETBYTE( tbv.m_ShowState );
	GETBYTE( tbv.m_ShowPriority );
	GETBYTE( tbv.m_ShowNumNotes );
	GETBYTE( tbv.m_ShowTicketID );
	GETBYTE( tbv.m_ShowNumMsgs );
	GETBYTE( tbv.m_ShowSubject );
	GETBYTE( tbv.m_ShowContact );
	GETBYTE( tbv.m_ShowDate );
	GETBYTE( tbv.m_ShowCategory );
	GETBYTE( tbv.m_ShowOwner );
	GETBYTE( tbv.m_ShowTicketBox );
	GETBYTE( tbv.m_UseDefault );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketBoxViews].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tbv.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), tbv.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), tbv.m_TicketBoxID );
		BINDPARAM_LONG( arc.GetQuery(), tbv.m_AgentBoxID );
		BINDPARAM_LONG( arc.GetQuery(), tbv.m_TicketBoxViewTypeID );
		BINDCOL_LONG( arc.GetQuery(), tbv.m_TicketBoxViewID );
		arc.GetQuery().Execute( _T("SELECT TicketBoxViewID FROM TicketBoxViews ")
								_T("WHERE AgentID=? AND TicketBoxID=? AND AgentBoxID=? AND TicketBoxViewTypeID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tbv.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//tbv.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tbv.m_TicketBoxViewID );
			BINDPARAM_LONG( arc.GetQuery(), tbv.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), tbv.m_TicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), tbv.m_AgentBoxID );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowOwnedItems );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowClosedItems );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_SortAscending );
			BINDPARAM_LONG( arc.GetQuery(), tbv.m_SortField );
			BINDPARAM_LONG( arc.GetQuery(), tbv.m_TicketBoxViewTypeID );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowState );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowPriority );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowNumNotes );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowTicketID );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowNumMsgs );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowSubject );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowContact );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowDate );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowCategory );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowOwner );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_ShowTicketBox );
			BINDPARAM_BIT( arc.GetQuery(), tbv.m_UseDefault );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketBoxViews ON INSERT INTO TicketBoxViews ")
						_T("(TicketBoxViewID,AgentID,TicketBoxID,AgentBoxID,ShowOwnedItems,ShowClosedItems,SortAscending,SortField,TicketBoxViewTypeID,ShowState,ShowPriority,ShowNumNotes,ShowTicketID,ShowNumMsgs,ShowSubject,ShowContact,ShowDate,ShowCategory,ShowOwner,ShowTicketBox,UseDefault) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT TicketBoxViews OFF") );
		}
	}

	arc.AddIDMapping(TABLEID_TicketBoxViews, ID, tbv.m_TicketBoxViewID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketBoxRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketBoxRecord( CArchiveFile& arc, long ID )
{
	TTicketBoxes tb;
	int ret;
	tb.m_TicketBoxID = ID;

	// Query from the databse
	if( tb.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( tb.m_Name );
		PUTCHAR( tb.m_Description );
		PUTBYTE( tb.m_AgeAlertingEnabled );
		PUTLONG( tb.m_AgeAlertingThresholdMins );
		PUTLONG( tb.m_ObjectID );
		PUTLONG( tb.m_MaxRecordsSize );
		PUTBYTE( tb.m_RequireGetOldest );
		PUTLONG( tb.m_LowWaterMark );
		PUTLONG( tb.m_HighWaterMark );
		PUTBYTE( tb.m_AutoReplyEnable );
		PUTLONG( tb.m_AutoReplyWithStdResponse );
		PUTLONG( tb.m_AutoReplyThreshHoldMins );
		PUTBYTE( tb.m_AutoReplyQuoteMsg );
		PUTBYTE( tb.m_AutoReplyCloseTicket );
		PUTBYTE( tb.m_AutoReplyInTicket );
		PUTLONG( tb.m_HeaderID );
		PUTLONG( tb.m_FooterID );
		PUTCHAR( tb.m_DefaultEmailAddress );
		PUTBYTE( tb.m_WaterMarkStatus );
		PUTCHAR( tb.m_DefaultEmailAddressName );
		PUTLONG( tb.m_OwnerID );
		PUTBYTE( tb.m_FooterLocation );
		PUTLONG( tb.m_MessageDestinationID );
		PUTLONG( tb.m_UnreadMode );
		PUTLONG( tb.m_FromFormat );
		PUTLONG( tb.m_TicketLink );
		PUTLONG( tb.m_MultiMail );
		PUTLONG( tb.m_RequireTC );
		PUTCHAR( tb.m_ReplyToEmailAddress );
		PUTCHAR( tb.m_ReturnPathEmailAddress );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketBoxRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketBoxRecord( CArchiveFile& arc, long ID )
{
	TTicketBoxes tb;
	int ret = Arc_Success;

	tb.m_TicketBoxID = ID;

	GETCHAR( tb.m_Name, TICKETBOXES_NAME_LENGTH );
	GETCHAR( tb.m_Description, TICKETBOXES_DESCRIPTION_LENGTH );
	GETBYTE( tb.m_AgeAlertingEnabled );
	GETLONG( tb.m_AgeAlertingThresholdMins );
	GETLONG( tb.m_ObjectID );
	arc.TranslateID( TABLEID_Objects, tb.m_ObjectID );
	GETLONG( tb.m_MaxRecordsSize );
	GETBYTE( tb.m_RequireGetOldest );
	GETLONG( tb.m_LowWaterMark );
	GETLONG( tb.m_HighWaterMark );
	GETBYTE( tb.m_AutoReplyEnable );
	GETLONG( tb.m_AutoReplyWithStdResponse );
	arc.TranslateID( TABLEID_StandardResponses, tb.m_AutoReplyWithStdResponse );
	GETLONG( tb.m_AutoReplyThreshHoldMins );
	GETBYTE( tb.m_AutoReplyQuoteMsg );
	GETBYTE( tb.m_AutoReplyCloseTicket );
	GETBYTE( tb.m_AutoReplyInTicket );
	GETLONG( tb.m_HeaderID );
	arc.TranslateID( TABLEID_TicketBoxHeaders, tb.m_HeaderID );
	GETLONG( tb.m_FooterID );
	arc.TranslateID( TABLEID_TicketBoxFooters, tb.m_FooterID );
	GETCHAR( tb.m_DefaultEmailAddress, TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH );
	GETBYTE( tb.m_WaterMarkStatus );
	GETCHAR( tb.m_DefaultEmailAddressName, TICKETBOXES_NAME_LENGTH );
	GETLONG( tb.m_OwnerID );
	GETBYTE( tb.m_FooterLocation );	
	GETLONG( tb.m_MessageDestinationID );
	GETLONG( tb.m_UnreadMode );
	GETLONG( tb.m_FromFormat );
	GETLONG( tb.m_TicketLink );
	GETLONG( tb.m_MultiMail );
	GETLONG( tb.m_RequireTC );
	GETCHAR( tb.m_ReplyToEmailAddress, TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH );
	GETCHAR( tb.m_ReturnPathEmailAddress, TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH );	
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketBoxes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tb.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tb.m_Name );
		BINDCOL_LONG( arc.GetQuery(), tb.m_TicketBoxID );
		arc.GetQuery().Execute( _T("SELECT TicketBoxID FROM TicketBoxes ")
								_T("WHERE Name=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tb.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//if( arc.OpCode() == Arc_Op_Import )
			//{
			//	// If we are doing an import, we must create the object first
			//	int ObjectTypeID = EMS_OBJECT_TYPE_TICKET_BOX;
			//	arc.GetQuery().Initialize();
			//	BINDPARAM_LONG(arc.GetQuery(), ObjectTypeID);
			//	arc.GetQuery().Execute( _T("INSERT INTO Objects (ObjectTypeID) VALUES (?)") );
			//	tb.m_ObjectID = arc.GetQuery().GetLastInsertedID();
			//}

			//tb.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tb.m_TicketBoxID );
			BINDPARAM_TCHAR( arc.GetQuery(), tb.m_Name );
			BINDPARAM_TCHAR( arc.GetQuery(), tb.m_Description );
			BINDPARAM_BIT( arc.GetQuery(), tb.m_AgeAlertingEnabled );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_AgeAlertingThresholdMins );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_ObjectID );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_MaxRecordsSize );
			BINDPARAM_BIT( arc.GetQuery(), tb.m_RequireGetOldest );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_LowWaterMark );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_HighWaterMark );
			BINDPARAM_BIT( arc.GetQuery(), tb.m_AutoReplyEnable );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_AutoReplyWithStdResponse );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_AutoReplyThreshHoldMins );
			BINDPARAM_BIT( arc.GetQuery(), tb.m_AutoReplyQuoteMsg );
			BINDPARAM_BIT( arc.GetQuery(), tb.m_AutoReplyCloseTicket );
			BINDPARAM_BIT( arc.GetQuery(), tb.m_AutoReplyInTicket );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_HeaderID );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_FooterID );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_OwnerID );
			BINDPARAM_TCHAR( arc.GetQuery(), tb.m_DefaultEmailAddress );
			BINDPARAM_TINYINT( arc.GetQuery(), tb.m_WaterMarkStatus );
			BINDPARAM_TCHAR( arc.GetQuery(), tb.m_DefaultEmailAddressName );
			BINDPARAM_TINYINT( arc.GetQuery(), tb.m_FooterLocation );			
			BINDPARAM_LONG( arc.GetQuery(), tb.m_MessageDestinationID );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_UnreadMode );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_FromFormat );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_TicketLink );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_MultiMail );
			BINDPARAM_LONG( arc.GetQuery(), tb.m_RequireTC );
			BINDPARAM_TCHAR( arc.GetQuery(), tb.m_ReplyToEmailAddress );
			BINDPARAM_TCHAR( arc.GetQuery(), tb.m_ReturnPathEmailAddress );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketBoxes ON INSERT INTO TicketBoxes ")
						_T("(TicketBoxID,Name,Description,AgeAlertingEnabled,AgeAlertingThresholdMins,ObjectID,MaxRecordsSize,RequireGetOldest,LowWaterMark,HighWaterMark,AutoReplyEnable,AutoReplyWithStdResponse,AutoReplyThreshHoldMins,AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket,HeaderID,FooterID,OwnerID,DefaultEmailAddress,WaterMarkStatus,DefaultEmailAddressName,FooterLocation,MessageDestinationID,UnreadMode,FromFormat,TicketLink,MultiMail,RequireTC,ReplyToEmailAddress,ReturnPathEmailAddress) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT TicketBoxes OFF") );
		}
	}

	arc.AddIDMapping( TABLEID_TicketBoxes, ID, tb.m_TicketBoxID );
	arc.AddActualObjectID( tb.m_ObjectID, tb.m_TicketBoxID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketContactRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketContactRecord( CArchiveFile& arc, long ID )
{
	TTicketContacts tc;
	int ret;
	tc.m_TicketContactID = ID;

	// Query from the databse
	if( tc.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( tc.m_TicketID );
		PUTLONG( tc.m_ContactID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketContactRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketContactRecord( CArchiveFile& arc, long ID )
{
	TTicketContacts tc;
	int ret = Arc_Success;
	tc.m_TicketContactID = ID;

	GETLONG( tc.m_TicketID );
	arc.TranslateID( TABLEID_Tickets, tc.m_TicketID );
	GETLONG( tc.m_ContactID );
	arc.TranslateID( TABLEID_Contacts, tc.m_ContactID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketContacts].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tc.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), tc.m_TicketID );
		BINDPARAM_LONG( arc.GetQuery(), tc.m_ContactID );
		BINDCOL_LONG( arc.GetQuery(), tc.m_TicketContactID );
		arc.GetQuery().Execute( _T("SELECT TicketContactID FROM TicketContacts ")
								_T("WHERE TicketID=? AND ContactID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tc.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			tc.Insert( arc.GetQuery() );
		}
	}

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketNoteRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketNoteRecord( CArchiveFile& arc, long ID )
{
	TTicketNotes tn;
	int ret;
	tn.m_TicketNoteID = ID;

	// Query from the databse
	if( tn.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( tn.m_TicketID );
		PUTLONG( tn.m_AgentID );
		PUTTEXT( tn.m_Note );
		PUTDATE( tn.m_DateCreated );
		PUTBYTE( tn.m_IsVoipNote );
		PUTDATE( tn.m_StartTime );
		PUTDATE( tn.m_StopTime );
		PUTCHAR( tn.m_ElapsedTime );
		PUTLONG( tn.m_ContactID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketNoteRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketNoteRecord( CArchiveFile& arc, long ID )
{
	TTicketNotes tn;
	int ret = Arc_Success;

	tn.m_TicketNoteID = ID;

	GETLONG( tn.m_TicketID );
	arc.TranslateID( TABLEID_Tickets, tn.m_TicketID );
	GETLONG( tn.m_AgentID );
	arc.TranslateID( TABLEID_Agents, tn.m_AgentID );
	GETTEXT( tn.m_Note );
	GETDATE( tn.m_DateCreated );
	GETBYTE( tn.m_IsVoipNote );
	GETDATE( tn.m_StartTime );
	GETDATE( tn.m_StopTime );
	GETCHAR( tn.m_ElapsedTime, CONTACTS_COMPANYNAME_LENGTH );
	GETLONG( tn.m_ContactID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketNotes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tn.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), tn.m_TicketID );
		BINDPARAM_LONG( arc.GetQuery(), tn.m_AgentID );
		BINDPARAM_TIME( arc.GetQuery(), tn.m_DateCreated );
		BINDCOL_LONG( arc.GetQuery(), tn.m_TicketNoteID );
		arc.GetQuery().Execute( _T("SELECT TicketNoteID FROM TicketNotes ")
								_T("WHERE TicketID=? AND AgentID=? AND DateCreated=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tn.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//tn.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tn.m_TicketNoteID );
			BINDPARAM_LONG( arc.GetQuery(), tn.m_TicketID );
			BINDPARAM_LONG( arc.GetQuery(), tn.m_AgentID );
			BINDPARAM_TEXT( arc.GetQuery(), tn.m_Note );
			BINDPARAM_TIME( arc.GetQuery(), tn.m_DateCreated );
			BINDPARAM_BIT( arc.GetQuery(), tn.m_IsVoipNote );
			BINDPARAM_TCHAR( arc.GetQuery(), tn.m_ElapsedTime );
			BINDPARAM_LONG( arc.GetQuery(), tn.m_ContactID );
			if ( tn.m_StartTimeLen <= 0 || tn.m_StopTimeLen <= 0 )
			{
				arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketNotes ON INSERT INTO TicketNotes ")
						_T("(TicketNoteID,TicketID,AgentID,Note,DateCreated,IsVoipNote,ElapsedTime,ContactID) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?) SET IDENTITY_INSERT TicketNotes OFF") );
			}
			else
			{
				BINDPARAM_TIME( arc.GetQuery(), tn.m_StartTime );
				BINDPARAM_TIME( arc.GetQuery(), tn.m_StopTime );
				arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketNotes ON INSERT INTO TicketNotes ")
						_T("(TicketNoteID,TicketID,AgentID,Note,DateCreated,IsVoipNote,ElapsedTime,ContactID,StartTime,StopTime) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT TicketNotes OFF") );
			}			
		}
	}

	arc.AddIDMapping(TABLEID_TicketNotes, ID, tn.m_TicketNoteID );
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketStateRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketStateRecord( CArchiveFile& arc, long ID )
{
	TTicketStates ts;
	int ret;
	ts.m_TicketStateID = ID;

	// Query from the databse
	if( ts.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( ts.m_Description );
		PUTCHAR( ts.m_HTMLColorName );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketStateRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketStateRecord( CArchiveFile& arc, long ID )
{
	TTicketStates ts;
	int ret = Arc_Success;

	ts.m_TicketStateID = ID;

	GETCHAR( ts.m_Description, TICKETSTATES_DESCRIPTION_LENGTH );
	GETCHAR( ts.m_HTMLColorName, TICKETSTATES_HTMLCOLORNAME_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketStates].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ts.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), ts.m_Description );
		BINDCOL_LONG( arc.GetQuery(), ts.m_TicketStateID );
		arc.GetQuery().Execute( _T("SELECT TicketStateID FROM TicketStates ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ts.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ts.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ts.m_TicketStateID );
			BINDPARAM_TCHAR( arc.GetQuery(), ts.m_Description );
			BINDPARAM_TCHAR( arc.GetQuery(), ts.m_HTMLColorName );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketStates ON INSERT INTO TicketStates ")
						_T("(TicketStateID,Description,HTMLColorName) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT TicketStates OFF") );
		}
	}

	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketRecord( CArchiveFile& arc, long ID )
{
	TTickets t;
	int ret;
	t.m_TicketID = ID;

	// Query from the databse
	if( t.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( t.m_TicketStateID );
		PUTLONG( t.m_TicketBoxID );
		PUTLONG( t.m_OwnerID );
		PUTCHAR( t.m_Subject );
		PUTCHAR( t.m_Contacts );
		PUTLONG( t.m_PriorityID );
		PUTDATE( t.m_DateCreated );
		PUTDATE( t.m_OpenTimestamp );
		PUTLONG( t.m_OpenMins );
		PUTBYTE( t.m_AutoReplied );
		PUTBYTE( t.m_AgeAlerted );
		PUTBYTE( t.m_IsDeleted );
		PUTDATE( t.m_DeletedTime );
		PUTLONG( t.m_DeletedBy );
		PUTLONG( t.m_LockedBy );
		PUTLONG( t.m_UseTickler );
		PUTDATE( t.m_ReOpenTime );
		PUTLONG( t.m_FolderID );
		PUTLONG( t.m_TicketCategoryID );
		PUTDATE( t.m_LockedTime );		

//		PUTLONG( t.m_OriginalTicketBoxID );
//		PUTLONG( t.m_OriginalAgentID );

	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


int RetrieveTicketToStruct( CArchiveFile& arc, Tickets_t& t )
{
	int ret = Arc_Success;

	GETLONG( t.m_TicketStateID );
	GETLONG( t.m_TicketBoxID );
	arc.TranslateID( TABLEID_TicketBoxes, t.m_TicketBoxID );
	GETLONG( t.m_OwnerID );
	arc.TranslateID( TABLEID_Agents, t.m_OwnerID );
	GETCHAR( t.m_Subject, TICKETS_SUBJECT_LENGTH );
	GETCHAR( t.m_Contacts, TICKETS_CONTACTS_LENGTH );
	GETLONG( t.m_PriorityID );
	GETDATE( t.m_DateCreated );
	GETDATE( t.m_OpenTimestamp );
	GETLONG( t.m_OpenMins );
	GETBYTE( t.m_AutoReplied );
	GETBYTE( t.m_AgeAlerted );
	GETBYTE( t.m_IsDeleted );
	GETDATE( t.m_DeletedTime );
	GETLONG( t.m_DeletedBy );
	arc.TranslateID( TABLEID_Agents, t.m_DeletedBy );
	GETLONG( t.m_LockedBy );
	arc.TranslateID( TABLEID_Agents, t.m_LockedBy );
	GETLONG( t.m_UseTickler );
	GETDATE( t.m_ReOpenTime );
	GETLONG( t.m_FolderID );
	GETLONG( t.m_TicketCategoryID );
	GETDATE( t.m_LockedTime );
//	GETLONG( t.m_OriginalTicketBoxID );
//	arc.TranslateID( TABLEID_TicketBoxes, t.m_OriginalTicketBoxID );
//	GETLONG( t.m_OriginalAgentID );
//	arc.TranslateID( TABLEID_Agents, t.m_OriginalAgentID  );

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketRecord( CArchiveFile& arc, long ID )
{
	TTickets t;
	int ret = Arc_Success;
	int nCount = 0;

	t.m_TicketID = ID;

	ret = RetrieveTicketToStruct( arc, t );

	if( ret != Arc_Success )
	{
		return ret;
	}

	// Check if record already exists
	arc.GetQuery().Initialize();
	BINDPARAM_LONG( arc.GetQuery(), t.m_TicketID );
	BINDCOL_LONG_NOLEN( arc.GetQuery(), nCount );
	arc.GetQuery().Execute( _T("SELECT COUNT(*) FROM Tickets WHERE TicketID=?") );

	if( arc.GetQuery().Fetch() == S_OK && nCount > 0 )
	{
		// Ticket ID already exists, overwrite?
		if( arc.Overwrite() )
		{
			ret = ( t.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		arc.GetQuery().Initialize();

		BINDPARAM_LONG( arc.GetQuery(), t.m_TicketID );
		BINDPARAM_LONG( arc.GetQuery(), t.m_TicketStateID );
		BINDPARAM_LONG( arc.GetQuery(), t.m_TicketBoxID );
		BINDPARAM_LONG( arc.GetQuery(), t.m_OwnerID );
		BINDPARAM_TCHAR( arc.GetQuery(), t.m_Subject );
		BINDPARAM_TCHAR( arc.GetQuery(), t.m_Contacts );
		BINDPARAM_LONG( arc.GetQuery(), t.m_PriorityID );
		BINDPARAM_TIME( arc.GetQuery(), t.m_DateCreated );
		BINDPARAM_TIME( arc.GetQuery(), t.m_OpenTimestamp );
		BINDPARAM_LONG( arc.GetQuery(), t.m_OpenMins );
		BINDPARAM_BIT( arc.GetQuery(), t.m_AutoReplied );
		BINDPARAM_BIT( arc.GetQuery(), t.m_AgeAlerted );
		BINDPARAM_TINYINT( arc.GetQuery(), t.m_IsDeleted );
		BINDPARAM_TIME( arc.GetQuery(), t.m_DeletedTime );
		BINDPARAM_LONG( arc.GetQuery(), t.m_DeletedBy );
		BINDPARAM_LONG( arc.GetQuery(), t.m_LockedBy );
		BINDPARAM_LONG( arc.GetQuery(), t.m_UseTickler );
		BINDPARAM_TIME( arc.GetQuery(), t.m_ReOpenTime );
		BINDPARAM_LONG( arc.GetQuery(), t.m_FolderID );
		BINDPARAM_LONG( arc.GetQuery(), t.m_TicketCategoryID );
		BINDPARAM_TIME( arc.GetQuery(), t.m_LockedTime );		

		arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Tickets ON"));
		arc.GetQuery().Execute( _T("INSERT INTO Tickets ")
								_T("(TicketID,TicketStateID,TicketBoxID,OwnerID,Subject,Contacts,PriorityID,DateCreated,OpenTimestamp,OpenMins,AutoReplied,AgeAlerted,IsDeleted,DeletedTime,DeletedBy,LockedBy,UseTickler,TicklerDateToReopen,FolderID,TicketCategoryID,LockedTime) ")
								_T("VALUES")
								_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
		arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Tickets OFF"));
		
	}

	return ret;
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffUIDLDataRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffUIDLDataRecord( CArchiveFile& arc, long ID )
{
	TUIDLData ud;
	int ret;
	ud.m_UIDLID = ID;

	// Query from the databse
	if( ud.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( ud.m_MessageSourceID );
		PUTCHAR( ud.m_Identifier );
		PUTBYTE( ud.m_IsPartial );
		PUTDATE( ud.m_UIDLDate );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveUIDLDataRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveUIDLDataRecord( CArchiveFile& arc, long ID )
{
	TUIDLData ud;
	int ret = Arc_Success;

	ud.m_UIDLID = ID;

	GETLONG( ud.m_MessageSourceID );
	arc.TranslateID( TABLEID_MessageSources, ud.m_MessageSourceID );
	GETCHAR( ud.m_Identifier, UIDLDATA_IDENTIFIER_LENGTH );
	GETBYTE( ud.m_IsPartial );
	GETDATE( ud.m_UIDLDate );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_UIDLData].ReservedID ) 
	{ 
		if( arc.Overwrite() )
			ret = ( ud.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), ud.m_Identifier );
		BINDPARAM_LONG( arc.GetQuery(), ud.m_MessageSourceID );
		BINDCOL_LONG( arc.GetQuery(), ud.m_UIDLID );
		arc.GetQuery().Execute( _T("SELECT UIDLID FROM UIDLData ")
								_T("WHERE Identifier=? AND MessageSourceID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
				ret = ( ud.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
		else
		{
			ud.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffUIDLDataRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffCustomDictRecord( CArchiveFile& arc, long ID )
{
	TCustomDictionary cd;
	int ret;
	cd.m_CustomDictionaryID = ID;

	// Query from the databse
	if( cd.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( cd.m_AgentID );
		PUTCHAR( cd.m_Word );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveUIDLDataRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveCustomDictRecord( CArchiveFile& arc, long ID )
{
	TCustomDictionary cd;
	int ret = Arc_Success;

	cd.m_CustomDictionaryID = ID;

	GETLONG( cd.m_AgentID );
	arc.TranslateID( TABLEID_Agents, cd.m_AgentID );
	GETCHAR( cd.m_Word, CUSTOMDICTIONARY_WORD_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_UIDLData].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( cd.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), cd.m_AgentID );
		BINDPARAM_TCHAR( arc.GetQuery(), cd.m_Word );
		BINDCOL_LONG( arc.GetQuery(), cd.m_CustomDictionaryID );
		arc.GetQuery().Execute( _T("SELECT CustomDictionaryID FROM CustomDictionary ")
								_T("WHERE AgentID=? AND Word=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( cd.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			cd.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffVirusScanStateRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffVirusScanStateRecord( CArchiveFile& arc, long ID )
{
	TVirusScanStates vss;
	int ret;
	vss.m_VirusScanStateID = ID;

	// Query from the databse
	if( vss.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( vss.m_Description );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveVirusScanStateRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveVirusScanStateRecord( CArchiveFile& arc, long ID )
{
	TVirusScanStates vss;
	int ret = Arc_Success;

	vss.m_VirusScanStateID = ID;

	GETCHAR( vss.m_Description, VIRUSSCANSTATES_DESCRIPTION_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_VirusScanStates].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( vss.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), vss.m_Description );
		BINDCOL_LONG( arc.GetQuery(), vss.m_VirusScanStateID );
		arc.GetQuery().Execute( _T("SELECT VirusScanStateID FROM VirusScanStates ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( vss.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//vss.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), vss.m_VirusScanStateID );
			BINDPARAM_TCHAR( arc.GetQuery(), vss.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT VirusScanStates ON INSERT INTO VirusScanStates ")
						_T("(VirusScanStateID,Description) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT VirusScanStates OFF") );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketActionRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketActionRecord( CArchiveFile& arc, long ID )
{
	TTicketActions ta;
	int ret;
	ta.m_TicketActionID = ID;

	// Query from the databse
	if( ta.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( ta.m_Description );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketActionRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketActionRecord( CArchiveFile& arc, long ID )
{
	TTicketActions ta;
	int ret = Arc_Success;

	ta.m_TicketActionID = ID;

	GETCHAR( ta.m_Description, TICKETACTIONS_DESCRIPTION_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketActions].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ta.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), ta.m_Description );
		BINDCOL_LONG( arc.GetQuery(), ta.m_TicketActionID );
		arc.GetQuery().Execute( _T("SELECT TicketActionID FROM TicketActions ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ta.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ta.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ta.m_TicketActionID );
			BINDPARAM_TCHAR( arc.GetQuery(), ta.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketActions ON INSERT INTO TicketActions ")
						_T("(TicketActionID,Description) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT TicketActions OFF") );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffTicketHistoryRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffTicketHistoryRecord( CArchiveFile& arc, long ID )
{
	TTicketHistory th;
	int ret;
	th.m_TicketHistoryID = ID;

	// Query from the databse
	if( th.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( th.m_TicketID );
		PUTDATE( th.m_DateTime );
		PUTLONG( th.m_AgentID );
		PUTLONG( th.m_TicketActionID );
		PUTLONG( th.m_ID1 );
		PUTLONG( th.m_ID2 );
		PUTLONG( th.m_TicketStateID );
		PUTLONG( th.m_TicketBoxID );
		PUTLONG( th.m_OwnerID );
		PUTLONG( th.m_PriorityID );
		PUTLONG( th.m_TicketCategoryID );
		PUTCHAR( th.m_DataValue );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveTicketHistoryRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveTicketHistoryRecord( CArchiveFile& arc, long ID )
{
	TTicketHistory th;
	int ret = Arc_Success;

	th.m_TicketHistoryID = ID;

	GETLONG( th.m_TicketID );
	arc.TranslateID( TABLEID_Tickets, th.m_TicketID );
	GETDATE( th.m_DateTime );
	GETLONG( th.m_AgentID );
	arc.TranslateID( TABLEID_Agents, th.m_AgentID );
	GETLONG( th.m_TicketActionID );
	GETLONG( th.m_ID1 );
	GETLONG( th.m_ID2 );
	GETLONG( th.m_TicketStateID );
	GETLONG( th.m_TicketBoxID );
	GETLONG( th.m_OwnerID );
	GETLONG( th.m_PriorityID );
	GETLONG( th.m_TicketCategoryID );
	GETCHAR( th.m_DataValue, 51 );

	switch( th.m_TicketActionID )
	{
	case EMS_TICKETACTIONID_CREATED:
		arc.TranslateID( TABLEID_RoutingRules, th.m_ID1 );
		break;
		
	case EMS_TICKETACTIONID_MODIFIED:
		switch( th.m_ID1 )
		{
		case 1:
			arc.TranslateID( TABLEID_TicketBoxes, th.m_ID2 );
			break;
			
		case 2:
			arc.TranslateID( TABLEID_Agents, th.m_ID2 );
			break;
		}
		break;
				
	case EMS_TICKETACTIONID_ESCALATED:
		arc.TranslateID( TABLEID_Agents, th.m_ID2 );
		break;
		
	case EMS_TICKETACTIONID_ADD_MSG:
	case EMS_TICKETACTIONID_DEL_MSG:
		if( th.m_ID1 == 0 )
		{
			arc.TranslateID( TABLEID_OutboundMessages, th.m_ID2 );
		}
		else
		{
			arc.TranslateID( TABLEID_InboundMessages, th.m_ID2 );
		}
		break;
	}

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketHistory].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( th.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), th.m_TicketID );
		BINDPARAM_TIME( arc.GetQuery(), th.m_DateTime );
		BINDPARAM_LONG( arc.GetQuery(), th.m_TicketActionID );
		BINDCOL_LONG( arc.GetQuery(), th.m_TicketHistoryID );
		arc.GetQuery().Execute( _T("SELECT TicketHistoryID FROM TicketHistory ")
								_T("WHERE TicketID=? AND DateTime=? AND TicketActionID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( th.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//th.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), th.m_TicketHistoryID );
			BINDPARAM_LONG( arc.GetQuery(), th.m_TicketID );
			BINDPARAM_TIME( arc.GetQuery(), th.m_DateTime );
			BINDPARAM_LONG( arc.GetQuery(), th.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), th.m_TicketActionID );
			BINDPARAM_LONG( arc.GetQuery(), th.m_ID1 );
			BINDPARAM_LONG( arc.GetQuery(), th.m_ID2 );
			BINDPARAM_LONG( arc.GetQuery(), th.m_TicketStateID );
			BINDPARAM_LONG( arc.GetQuery(), th.m_TicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), th.m_OwnerID );
			BINDPARAM_LONG( arc.GetQuery(), th.m_PriorityID );
			BINDPARAM_LONG( arc.GetQuery(), th.m_TicketCategoryID );
			BINDPARAM_TCHAR( arc.GetQuery(), th.m_DataValue );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketHistory ON INSERT INTO TicketHistory ")
						_T("(TicketHistoryID,TicketID,DateTime,AgentID,TicketActionID,ID1,ID2,TicketStateID,TicketBoxID,OwnerID,PriorityID,TicketCategoryID,DataValue) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT TicketHistory OFF") );			
		}
	}
	
	return ret;	
}

//*****************************************************************************
// Stuff and Retrieve for InboundMessageRead

int StuffInboundMessageReadRecord(CArchiveFile& arc, long ID)
{
	TInboundMessageRead timr;
	int ret;
	timr.m_InboundMessageReadID = ID;

	// Query from the databse
	if(timr.Query( arc.GetQuery()) == S_OK )
	{
		PUTLONG(timr.m_InboundMessageID);
		PUTLONG(timr.m_AgentID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveInboundMessageReadRecord(CArchiveFile& arc, long ID)
{
	TInboundMessageRead timr;
	int ret = Arc_Success;

	timr.m_InboundMessageReadID = ID;

	GETLONG(timr.m_InboundMessageID);
	arc.TranslateID(TABLEID_InboundMessages, timr.m_InboundMessageID);
	GETLONG(timr.m_AgentID );
	arc.TranslateID(TABLEID_Agents, timr.m_AgentID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_InboundMessageRead].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (timr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), timr.m_InboundMessageID );
		BINDPARAM_LONG( arc.GetQuery(), timr.m_AgentID );
		BINDCOL_LONG( arc.GetQuery(), timr.m_InboundMessageReadID );
		arc.GetQuery().Execute( _T("SELECT InboundMessageReadID FROM InboundMessageRead ")
								_T("WHERE InboundMessageID=? AND AgentID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( timr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			timr.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for InboundMessageRead
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for SRKeywordResults

int StuffSRKeywordResultsRecord(CArchiveFile& arc, long ID)
{
	TSRKeywordResults tsrkr;
	int ret;
	tsrkr.m_SRKeywordResultsID = ID;

	// Query from the databse
	if(tsrkr.Query( arc.GetQuery()) == S_OK )
	{
		PUTLONG(tsrkr.m_InboundMessageID);
		PUTLONG(tsrkr.m_StandardResponseID);
		PUTLONG(tsrkr.m_Score);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveSRKeywordResultsRecord(CArchiveFile& arc, long ID)
{
	TSRKeywordResults tsrkr;
	int ret = Arc_Success;

	tsrkr.m_SRKeywordResultsID = ID;

	GETLONG(tsrkr.m_InboundMessageID);
	arc.TranslateID(TABLEID_InboundMessages, tsrkr.m_InboundMessageID);
	GETLONG(tsrkr.m_StandardResponseID );
	arc.TranslateID(TABLEID_StandardResponses, tsrkr.m_StandardResponseID);
	GETLONG(tsrkr.m_Score);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_SRKeywordResults].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (tsrkr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), tsrkr.m_InboundMessageID );
		BINDPARAM_LONG( arc.GetQuery(), tsrkr.m_StandardResponseID );
		BINDPARAM_LONG( arc.GetQuery(), tsrkr.m_Score );
		BINDCOL_LONG( arc.GetQuery(), tsrkr.m_SRKeywordResultsID );
		arc.GetQuery().Execute( _T("SELECT SRKeywordResultsID FROM SRKeywordResults ")
								_T("WHERE InboundMessageID=? AND StandardResponseID=? AND Score=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tsrkr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			try
			{
				tsrkr.Insert( arc.GetQuery() );
			}
			catch(...)
			{
				//dca::TString sErr;
				///sErr.Format( _T("Restore:: Caught EMS exception: %s"), e.GetErrorString() );
				//LogDBMaintError( E_DBMaintErrorRestore, nErr, sErr );
			}
		}
	}
	
	return ret;
}

// End of Stuff and Retrieve for SRKeywordResults
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketNotesRead

int StuffTicketNotesReadRecord(CArchiveFile& arc, long ID)
{
	TTicketNotesRead ttnr;
	int ret;
	ttnr.m_TicketNotesReadID = ID;

	// Query from the databse
	if(ttnr.Query( arc.GetQuery()) == S_OK )
	{
		PUTLONG(ttnr.m_TicketNoteID);
		PUTLONG(ttnr.m_AgentID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketNotesReadRecord(CArchiveFile& arc, long ID)
{
	TTicketNotesRead ttnr;
	int ret = Arc_Success;

	ttnr.m_TicketNotesReadID = ID;

	GETLONG(ttnr.m_TicketNoteID);
	arc.TranslateID(TABLEID_TicketNotes, ttnr.m_TicketNoteID);
	GETLONG(ttnr.m_AgentID);
	arc.TranslateID(TABLEID_Agents, ttnr.m_AgentID);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketNotesRead].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (ttnr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), ttnr.m_TicketNoteID );
		BINDPARAM_LONG( arc.GetQuery(), ttnr.m_AgentID );
		BINDCOL_LONG( arc.GetQuery(), ttnr.m_TicketNotesReadID );
		arc.GetQuery().Execute( _T("SELECT TicketNotesReadID FROM TicketNotesRead ")
								_T("WHERE TicketNoteID=? AND AgentID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ttnr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			ttnr.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for TicketNotesRead
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for Folders

int StuffFolderRecord(CArchiveFile& arc, long ID)
{
	TFolders tf;
	int ret;
	tf.m_FolderID = ID;

	// Query from the databse
	if(tf.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(tf.m_Name);
		PUTLONG(tf.m_AgentID);
		PUTLONG(tf.m_ParentID);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveFolderRecord(CArchiveFile& arc, long ID)
{
	TFolders tf;
	int ret = Arc_Success;

	tf.m_FolderID = ID;

	GETCHAR(tf.m_Name, 55);
	GETLONG(tf.m_AgentID);
	arc.TranslateID(TABLEID_Agents, tf.m_AgentID);
	GETLONG(tf.m_ParentID);
	arc.TranslateID(TABLEID_TicketBoxViews, tf.m_ParentID);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Folders].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (tf.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tf.m_Name );
		BINDPARAM_LONG( arc.GetQuery(), tf.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), tf.m_ParentID );
		BINDCOL_LONG( arc.GetQuery(), tf.m_FolderID );
		arc.GetQuery().Execute( _T("SELECT FolderID FROM Folders ")
								_T("WHERE Name=? AND AgentID=? AND ParentID=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (tf.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tf.m_FolderID );
			BINDPARAM_TCHAR( arc.GetQuery(), tf.m_Name);
			BINDPARAM_LONG( arc.GetQuery(), tf.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), tf.m_ParentID );

			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Folders ON INSERT INTO Folders ")
						            _T("(FolderID, Name, AgentID, ParentID) ")
						            _T("VALUES")
						            _T("(?,?,?,?) SET IDENTITY_INSERT Folders OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for Folders
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for AgeAlerts

int StuffAgeAlertRecord(CArchiveFile& arc, long ID)
{
	TAgeAlerts aa;
	int ret;
	aa.m_AgeAlertID = ID;

	// Query from the databse
	if(aa.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(aa.m_Description);
		PUTLONG(aa.m_HitCount);
		PUTLONG(aa.m_ThresholdMins);
		PUTLONG(aa.m_ThresholdFreq);
		PUTBYTE(aa.m_IsEnabled);
		PUTBYTE(aa.m_NoAlertIfRepliedTo);
		PUTLONG(aa.m_AlertOnTypeID);
		PUTLONG(aa.m_AlertOnID);
		PUTLONG(aa.m_AlertToTypeID);
		PUTLONG(aa.m_AlertToID);
		PUTLONG(aa.m_AlertMethodID);
		PUTCHAR(aa.m_EmailAddress);
		PUTCHAR(aa.m_FromEmailAddress);
		PUTDATE(aa.m_DateCreated );
		PUTDATE(aa.m_DateEdited );
		PUTLONG(aa.m_CreatedByID);
		PUTLONG(aa.m_EditedByID);	
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveAgeAlertRecord(CArchiveFile& arc, long ID)
{
	TAgeAlerts aa;
	int ret = Arc_Success;
	aa.m_AgeAlertID = ID;

	GETCHAR(aa.m_Description, 255);
	GETLONG(aa.m_HitCount);
	GETLONG(aa.m_ThresholdMins);
	GETLONG(aa.m_ThresholdFreq);
	GETBYTE(aa.m_IsEnabled);
	GETBYTE(aa.m_NoAlertIfRepliedTo);
	GETLONG(aa.m_AlertOnTypeID);
	GETLONG(aa.m_AlertOnID);
	GETLONG(aa.m_AlertToTypeID);
	GETLONG(aa.m_AlertToID);
	GETLONG(aa.m_AlertMethodID);
	GETCHAR(aa.m_EmailAddress, 255);
	GETCHAR(aa.m_FromEmailAddress, 255);
	GETDATE(aa.m_DateCreated );
	GETDATE(aa.m_DateEdited );
	GETLONG(aa.m_CreatedByID);
	arc.TranslateID(TABLEID_Agents, aa.m_CreatedByID);
	GETLONG(aa.m_EditedByID);
	arc.TranslateID(TABLEID_Agents, aa.m_EditedByID);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AgeAlerts].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (aa.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), aa.m_Description );
		BINDCOL_LONG( arc.GetQuery(), aa.m_AgeAlertID );
		arc.GetQuery().Execute( _T("SELECT AgeAlertID FROM AgeAlerts ")
								_T("WHERE Description=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (aa.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AgeAlertID );
			BINDPARAM_TCHAR( arc.GetQuery(), aa.m_Description );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_HitCount );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_ThresholdMins );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_ThresholdFreq );
			BINDPARAM_BIT( arc.GetQuery(), aa.m_IsEnabled );
			BINDPARAM_BIT( arc.GetQuery(), aa.m_NoAlertIfRepliedTo );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AlertOnTypeID );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AlertOnID );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AlertToTypeID );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AlertToID );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AlertMethodID );
			BINDPARAM_TCHAR( arc.GetQuery(), aa.m_EmailAddress );
			BINDPARAM_TCHAR( arc.GetQuery(), aa.m_FromEmailAddress );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_CreatedByID );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_EditedByID );
			BINDPARAM_TIME( arc.GetQuery(), aa.m_DateCreated );
			BINDPARAM_TIME( arc.GetQuery(), aa.m_DateEdited );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AgeAlerts ON INSERT INTO AgeAlerts ")
						_T("(AgeAlertID,Description,HitCount,ThresholdMins,ThresholdFreq,IsEnabled,NoAlertIfRepliedTo,AlertOnTypeID,AlertOnID,AlertToTypeID,AlertToID,AlertMethodID,EmailAddress,FromEmailAddress,CreatedByID,EditedByID,DateCreated,DateEdited) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT AgeAlerts OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for AgeAlerts
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for AgeAlertSent

int StuffAgeAlertSentRecord(CArchiveFile& arc, long ID)
{
	TAgeAlertsSent aas;
	int ret;
	aas.m_AgeAlertSentID = ID;

	// Query from the databse
	if(aas.Query( arc.GetQuery()) == S_OK )
	{
		PUTLONG(aas.m_AgeAlertID);
		PUTLONG(aas.m_TicketID);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveAgeAlertSentRecord(CArchiveFile& arc, long ID)
{
	TAgeAlertsSent aas;
	int ret = Arc_Success;
	aas.m_AgeAlertSentID = ID;

	GETLONG(aas.m_AgeAlertID);
	arc.TranslateID(TABLEID_AgeAlerts, aas.m_AgeAlertID);
	GETLONG(aas.m_TicketID);
	arc.TranslateID(TABLEID_Tickets, aas.m_TicketID);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AgeAlertsSent].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (aas.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), aas.m_AgeAlertID );
		BINDPARAM_LONG( arc.GetQuery(), aas.m_TicketID );
		BINDCOL_LONG( arc.GetQuery(), aas.m_AgeAlertSentID );
		arc.GetQuery().Execute( _T("SELECT AgeAlertSentID FROM AgeAlertsSent ")
								_T("WHERE AgeAlertID=? AND TicketID=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (aas.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), aas.m_AgeAlertSentID );
			BINDPARAM_LONG( arc.GetQuery(), aas.m_AgeAlertID );
			BINDPARAM_LONG( arc.GetQuery(), aas.m_TicketID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AgeAlertsSent ON INSERT INTO AgeAlertsSent ")
						            _T("(AgeAlertSentID,AgeAlertID,TicketID) ")
						            _T("VALUES")
						            _T("(?,?,?) SET IDENTITY_INSERT AgeAlertsSent OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for AgeAlertSent
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for AutoMessages

int StuffAutoMessageRecord(CArchiveFile& arc, long ID)
{
	TAutoMessages am;
	int ret;
	am.m_AutoMessageID = ID;

	// Query from the databse
	if(am.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(am.m_Description);
		PUTLONG(am.m_HitCount);
		PUTLONG(am.m_FailCount);
		PUTLONG(am.m_TotalCount);
		PUTLONG(am.m_SendToTypeID);
		PUTLONG(am.m_SendToID);
		PUTBYTE(am.m_IsEnabled);
		PUTLONG(am.m_WhenToSendVal);
		PUTLONG(am.m_WhenToSendFreq);
		PUTLONG(am.m_WhenToSendTypeID);
		PUTLONG(am.m_SrToSendID);
		PUTLONG(am.m_SendFromTypeID);
		PUTBYTE(am.m_CreateNewTicket);
		PUTCHAR(am.m_ReplyToAddress);
		PUTCHAR(am.m_ReplyToName);
		PUTLONG(am.m_OwnerID);
		PUTLONG(am.m_TicketBoxID);
		PUTLONG(am.m_TicketCategoryID);
		PUTDATE(am.m_DateCreated);
		PUTDATE(am.m_DateEdited);
		PUTLONG(am.m_CreatedByID);
		PUTLONG(am.m_EditedByID);
		PUTLONG(am.m_SendToPercent);
		PUTLONG(am.m_OmitTracking);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveAutoMessageRecord(CArchiveFile& arc, long ID)
{
	TAutoMessages am;
	int ret = Arc_Success;
	am.m_AutoMessageID = ID;

	GETCHAR(am.m_Description, 255);
	GETLONG(am.m_HitCount);
	GETLONG(am.m_FailCount);
	GETLONG(am.m_TotalCount);
	GETLONG(am.m_SendToTypeID);
	GETLONG(am.m_SendToID);
	GETBYTE(am.m_IsEnabled);
	GETLONG(am.m_WhenToSendVal);
	GETLONG(am.m_WhenToSendFreq);
	GETLONG(am.m_WhenToSendTypeID);
	GETLONG(am.m_SrToSendID);
	GETLONG(am.m_SendFromTypeID);
	GETBYTE(am.m_CreateNewTicket);
	GETCHAR(am.m_ReplyToAddress, 255);
	GETCHAR(am.m_ReplyToName, 255);
	GETLONG(am.m_OwnerID);
	GETLONG(am.m_TicketBoxID);
	GETLONG(am.m_TicketCategoryID);
	GETDATE(am.m_DateCreated);
	GETDATE(am.m_DateEdited);
	GETLONG(am.m_CreatedByID);
	arc.TranslateID(TABLEID_Agents, am.m_CreatedByID);
	GETLONG(am.m_EditedByID);
	arc.TranslateID(TABLEID_Agents, am.m_EditedByID);
	GETLONG(am.m_SendToPercent);
	GETLONG(am.m_OmitTracking);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AutoMessages].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (am.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), am.m_Description );
		BINDCOL_LONG( arc.GetQuery(), am.m_AutoMessageID );
		arc.GetQuery().Execute( _T("SELECT AutoMessageID FROM AutoMessages ")
								_T("WHERE Description=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (am.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), am.m_AutoMessageID );
			BINDPARAM_TCHAR( arc.GetQuery(), am.m_Description );
			BINDPARAM_LONG( arc.GetQuery(), am.m_HitCount );
			BINDPARAM_LONG( arc.GetQuery(), am.m_FailCount );
			BINDPARAM_LONG( arc.GetQuery(), am.m_TotalCount );
			BINDPARAM_LONG( arc.GetQuery(), am.m_SendToTypeID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_SendToID );
			BINDPARAM_BIT( arc.GetQuery(), am.m_IsEnabled );
			BINDPARAM_LONG( arc.GetQuery(), am.m_WhenToSendVal );
			BINDPARAM_LONG( arc.GetQuery(), am.m_WhenToSendFreq );
			BINDPARAM_LONG( arc.GetQuery(), am.m_WhenToSendTypeID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_SrToSendID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_SendFromTypeID );
			BINDPARAM_BIT( arc.GetQuery(), am.m_CreateNewTicket );
			BINDPARAM_TCHAR( arc.GetQuery(), am.m_ReplyToAddress );
			BINDPARAM_TCHAR( arc.GetQuery(), am.m_ReplyToName );
			BINDPARAM_LONG( arc.GetQuery(), am.m_OwnerID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_TicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_TicketCategoryID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_CreatedByID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_EditedByID );
			BINDPARAM_LONG( arc.GetQuery(), am.m_SendToPercent );
			BINDPARAM_LONG( arc.GetQuery(), am.m_OmitTracking );
			BINDPARAM_TIME( arc.GetQuery(), am.m_DateCreated );
			BINDPARAM_TIME( arc.GetQuery(), am.m_DateEdited );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AutoMessages ON INSERT INTO AutoMessages ")
						_T("(AutoMessageID,Description,HitCount,FailCount,TotalCount,SendToTypeID,SendToID,IsEnabled,WhenToSendVal,WhenToSendFreq,WhenToSendTypeID,SrToSendID,SendFromTypeID,CreateNewTicket,ReplyToAddress,ReplyToName,OwnerID,TicketBoxID,TicketCategoryID,CreatedByID,EditedByID,SendToPercent,OmitTracking,DateCreated,DateEdited) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT AutoMessages OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for AutoMessages
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for AutoMessageSent

int StuffAutoMessageSentRecord(CArchiveFile& arc, long ID)
{
	TAutoMessagesSent ams;
	int ret;
	ams.m_AutoMessageSentID = ID;

	// Query from the databse
	if(ams.Query( arc.GetQuery()) == S_OK )
	{
		PUTLONG(ams.m_AutoMessageID);
		PUTLONG(ams.m_TicketID);
		PUTLONG(ams.m_ResultCode);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveAutoMessageSentRecord(CArchiveFile& arc, long ID)
{
	TAutoMessagesSent ams;
	int ret = Arc_Success;
	ams.m_AutoMessageSentID = ID;

	GETLONG(ams.m_AutoMessageID);
	arc.TranslateID(TABLEID_AutoMessages, ams.m_AutoMessageID);
	GETLONG(ams.m_TicketID);
	arc.TranslateID(TABLEID_Tickets, ams.m_TicketID);
	GETLONG(ams.m_ResultCode);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AutoMessagesSent].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (ams.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), ams.m_AutoMessageID );
		BINDPARAM_LONG( arc.GetQuery(), ams.m_TicketID );
		BINDCOL_LONG( arc.GetQuery(), ams.m_AutoMessageSentID );
		arc.GetQuery().Execute( _T("SELECT AutoMessageSentID FROM AutoMessagesSent ")
								_T("WHERE AutoMessageID=? AND TicketID=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (ams.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ams.m_AutoMessageSentID );
			BINDPARAM_LONG( arc.GetQuery(), ams.m_AutoMessageID );
			BINDPARAM_LONG( arc.GetQuery(), ams.m_TicketID );
			BINDPARAM_LONG( arc.GetQuery(), ams.m_ResultCode );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AutoMessagesSent ON INSERT INTO AutoMessagesSent ")
						            _T("(AutoMessageSentID,AutoMessageID,TicketID,ResultCode) ")
						            _T("VALUES")
						            _T("(?,?,?,?) SET IDENTITY_INSERT AutoMessagesSent OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for AutoMessageSent
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for AutoResponses

int StuffAutoResponseRecord(CArchiveFile& arc, long ID)
{
	TAutoResponses am;
	int ret;
	am.m_AutoResponseID = ID;

	// Query from the databse
	if(am.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(am.m_Description);
		PUTLONG(am.m_HitCount);
		PUTLONG(am.m_FailCount);
		PUTLONG(am.m_TotalCount);
		PUTLONG(am.m_SendToTypeID);
		PUTLONG(am.m_SendToID);
		PUTBYTE(am.m_IsEnabled);
		PUTLONG(am.m_WhenToSendVal);
		PUTLONG(am.m_WhenToSendFreq);
		PUTLONG(am.m_WhenToSendTypeID);
		PUTLONG(am.m_SrToSendID);
		PUTBYTE(am.m_AutoReplyQuoteMsg);
		PUTBYTE(am.m_AutoReplyCloseTicket);
		PUTBYTE(am.m_AutoReplyInTicket);
		PUTLONG(am.m_HeaderID);
		PUTLONG(am.m_FooterID);
		PUTLONG(am.m_SendFromTypeID);
		PUTCHAR(am.m_ReplyToAddress);
		PUTCHAR(am.m_ReplyToName);
		PUTDATE(am.m_DateCreated);
		PUTDATE(am.m_DateEdited);
		PUTLONG(am.m_CreatedByID);
		PUTLONG(am.m_EditedByID);
		PUTLONG(am.m_SendToPercent);
		PUTBYTE(am.m_FooterLocation);
		PUTLONG(am.m_OmitTracking);
		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveAutoResponseRecord(CArchiveFile& arc, long ID)
{
	TAutoResponses am;
	int ret = Arc_Success;
	am.m_AutoResponseID = ID;

	GETCHAR(am.m_Description, 255);
	GETLONG(am.m_HitCount);
	GETLONG(am.m_FailCount);
	GETLONG(am.m_TotalCount);
	GETLONG(am.m_SendToTypeID);
	GETLONG(am.m_SendToID);
	GETBYTE(am.m_IsEnabled);
	GETLONG(am.m_WhenToSendVal);
	GETLONG(am.m_WhenToSendFreq);
	GETLONG(am.m_WhenToSendTypeID);
	GETLONG(am.m_SrToSendID);
	GETBYTE(am.m_AutoReplyQuoteMsg);
	GETBYTE(am.m_AutoReplyCloseTicket);
	GETBYTE(am.m_AutoReplyInTicket);
	GETLONG(am.m_HeaderID);
	GETLONG(am.m_FooterID);
	GETLONG(am.m_SendFromTypeID);
	GETCHAR(am.m_ReplyToAddress, 255);
	GETCHAR(am.m_ReplyToName, 255);
	GETDATE(am.m_DateCreated);
	GETDATE(am.m_DateEdited);
	GETLONG(am.m_CreatedByID);
	arc.TranslateID(TABLEID_Agents, am.m_CreatedByID);
	GETLONG(am.m_EditedByID);
	arc.TranslateID(TABLEID_Agents, am.m_EditedByID);
	GETLONG(am.m_SendToPercent);
	GETBYTE(am.m_FooterLocation);
	GETLONG(am.m_OmitTracking);
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AutoResponses].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (am.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), am.m_Description );
		BINDCOL_LONG( arc.GetQuery(), am.m_AutoResponseID );
		arc.GetQuery().Execute( _T("SELECT AutoResponseID FROM AutoResponses ")
								_T("WHERE Description=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (am.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG(arc.GetQuery(), am.m_AutoResponseID);
			BINDPARAM_TCHAR(arc.GetQuery(), am.m_Description);
			BINDPARAM_LONG(arc.GetQuery(), am.m_HitCount);
			BINDPARAM_LONG(arc.GetQuery(), am.m_FailCount);
			BINDPARAM_LONG(arc.GetQuery(), am.m_TotalCount);
			BINDPARAM_LONG(arc.GetQuery(), am.m_SendToTypeID);
			BINDPARAM_LONG(arc.GetQuery(), am.m_SendToID);
			BINDPARAM_BIT(arc.GetQuery(), am.m_IsEnabled);
			BINDPARAM_LONG(arc.GetQuery(), am.m_WhenToSendVal);
			BINDPARAM_LONG(arc.GetQuery(), am.m_WhenToSendFreq);
			BINDPARAM_LONG(arc.GetQuery(), am.m_WhenToSendTypeID);
			BINDPARAM_LONG(arc.GetQuery(), am.m_SrToSendID);
			BINDPARAM_BIT(arc.GetQuery(), am.m_AutoReplyQuoteMsg);
			BINDPARAM_BIT(arc.GetQuery(), am.m_AutoReplyCloseTicket);
			BINDPARAM_BIT(arc.GetQuery(), am.m_AutoReplyInTicket);
			BINDPARAM_LONG(arc.GetQuery(), am.m_HeaderID);
			BINDPARAM_LONG(arc.GetQuery(), am.m_FooterID);
			BINDPARAM_LONG(arc.GetQuery(), am.m_SendFromTypeID);
			BINDPARAM_TCHAR(arc.GetQuery(), am.m_ReplyToAddress);
			BINDPARAM_TCHAR(arc.GetQuery(), am.m_ReplyToName);
			BINDPARAM_LONG(arc.GetQuery(), am.m_CreatedByID);
			BINDPARAM_LONG(arc.GetQuery(), am.m_EditedByID);
			BINDPARAM_LONG(arc.GetQuery(), am.m_SendToPercent);
			BINDPARAM_LONG(arc.GetQuery(), am.m_OmitTracking);
			BINDPARAM_TIME(arc.GetQuery(), am.m_DateCreated);
			BINDPARAM_TIME(arc.GetQuery(), am.m_DateEdited);
			BINDPARAM_TINYINT(arc.GetQuery(), am.m_FooterLocation);
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AutoResponses ON INSERT INTO AutoResponses ")
						_T("(AutoResponseID,Description,HitCount,FailCount,TotalCount,SendToTypeID,SendToID,IsEnabled,WhenToSendVal,WhenToSendFreq,WhenToSendTypeID,SrToSendID,AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket,HeaderID,FooterID,SendFromTypeID,ReplyToAddress,ReplyToName,CreatedByID,EditedByID,SendToPercent,OmitTracking,DateCreated,DateEdited,FooterLocation) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT AutoResponses OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for AutoResponses
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for AutoResponseSent

int StuffAutoResponseSentRecord(CArchiveFile& arc, long ID)
{
	TAutoResponsesSent ams;
	int ret;
	ams.m_AutoResponseSentID = ID;

	// Query from the databse
	if(ams.Query( arc.GetQuery()) == S_OK )
	{
		PUTLONG(ams.m_AutoResponseID);
		PUTLONG(ams.m_TicketID);
		PUTLONG(ams.m_ResultCode);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveAutoResponseSentRecord(CArchiveFile& arc, long ID)
{
	TAutoResponsesSent ams;
	int ret = Arc_Success;
	ams.m_AutoResponseSentID = ID;

	GETLONG(ams.m_AutoResponseID);
	arc.TranslateID(TABLEID_AutoResponses, ams.m_AutoResponseID);
	GETLONG(ams.m_TicketID);
	arc.TranslateID(TABLEID_Tickets, ams.m_TicketID);
	GETLONG(ams.m_ResultCode);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AutoResponsesSent].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (ams.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), ams.m_AutoResponseID );
		BINDPARAM_LONG( arc.GetQuery(), ams.m_TicketID );
		BINDCOL_LONG( arc.GetQuery(), ams.m_AutoResponseSentID );
		arc.GetQuery().Execute( _T("SELECT AutoResponseSentID FROM AutoResponsesSent ")
								_T("WHERE AutoResponseID=? AND TicketID=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (ams.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ams.m_AutoResponseSentID );
			BINDPARAM_LONG( arc.GetQuery(), ams.m_AutoResponseID );
			BINDPARAM_LONG( arc.GetQuery(), ams.m_TicketID );
			BINDPARAM_LONG( arc.GetQuery(), ams.m_ResultCode );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AutoResponsesSent ON INSERT INTO AutoResponsesSent ")
						            _T("(AutoResponseSentID,AutoResponseID,TicketID,ResultCode) ")
						            _T("VALUES")
						            _T("(?,?,?,?) SET IDENTITY_INSERT AutoResponsesSent OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for AutoResponseSent
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketBoxOwner

int StuffTicketBoxOwnerRecord(CArchiveFile& arc, long ID)
{
	TTicketBoxOwners tbo;
	int ret;
	tbo.m_TicketBoxOwnerID = ID;

	// Query from the databse
	if(tbo.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(tbo.m_Description);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketBoxOwnerRecord(CArchiveFile& arc, long ID)
{
	TTicketBoxOwners tbo;
	int ret = Arc_Success;
	tbo.m_TicketBoxOwnerID = ID;

	GETCHAR(tbo.m_Description, 50);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketBoxOwners].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (tbo.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tbo.m_Description );
		BINDCOL_LONG( arc.GetQuery(), tbo.m_TicketBoxOwnerID );
		arc.GetQuery().Execute( _T("SELECT TicketBoxOwnerID FROM TicketBoxOwners ")
								_T("WHERE Description=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (tbo.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tbo.m_TicketBoxOwnerID );
			BINDPARAM_TCHAR( arc.GetQuery(), tbo.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketBoxOwners ON INSERT INTO TicketBoxOwners ")
						            _T("(TicketBoxOwnerID,Description) ")
						            _T("VALUES")
						            _T("(?,?) SET IDENTITY_INSERT TicketBoxOwners OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for TicketBoxOwners
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketBoxTicketBoxOwner

int StuffTicketBoxTicketBoxOwnerRecord(CArchiveFile& arc, long ID)
{
	TTicketBoxTicketBoxOwners tbtbo;
	int ret;
	tbtbo.m_TbTboID = ID;

	// Query from the databse
	if(tbtbo.Query( arc.GetQuery()) == S_OK )
	{
		PUTLONG(tbtbo.m_TicketBoxID);
		PUTLONG(tbtbo.m_TicketBoxOwnerID);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketBoxTicketBoxOwnerRecord(CArchiveFile& arc, long ID)
{
	TTicketBoxTicketBoxOwners tbtbo;
	int ret = Arc_Success;
	tbtbo.m_TbTboID = ID;

	GETLONG(tbtbo.m_TicketBoxID);
	GETLONG(tbtbo.m_TicketBoxOwnerID);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketBoxTicketBoxOwner].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (tbtbo.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), tbtbo.m_TicketBoxID );
		BINDPARAM_LONG( arc.GetQuery(), tbtbo.m_TicketBoxOwnerID );
		BINDCOL_LONG( arc.GetQuery(), tbtbo.m_TbTboID );
		arc.GetQuery().Execute( _T("SELECT TbTboID FROM TicketBoxTicketBoxOwners ")
								_T("WHERE TicketBoxID=? AND TicketBoxOwnerID=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (tbtbo.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tbtbo.m_TbTboID );
			BINDPARAM_LONG( arc.GetQuery(), tbtbo.m_TicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), tbtbo.m_TicketBoxOwnerID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketBoxTicketBoxOwners ON INSERT INTO TicketBoxTicketBoxOwners ")
						            _T("(TbTboID,TicketBoxID,TicketBoxOwnerID) ")
						            _T("VALUES")
						            _T("(?,?,?) SET IDENTITY_INSERT TicketBoxTicketBoxOwners OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for TicketBoxTicketBoxOwners
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketBoxRouting

int StuffTicketBoxRoutingRecord(CArchiveFile& arc, long ID)
{
	TTicketBoxRouting tbr;
	int ret;
	tbr.m_TicketBoxRoutingID = ID;

	// Query from the databse
	if(tbr.Query( arc.GetQuery()) == S_OK )
	{
		PUTLONG(tbr.m_TicketBoxID);
		PUTLONG(tbr.m_RoutingRuleID);
		PUTLONG(tbr.m_Percentage);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketBoxRoutingRecord(CArchiveFile& arc, long ID)
{
	TTicketBoxRouting tbr;
	int ret = Arc_Success;
	tbr.m_TicketBoxRoutingID = ID;


	GETLONG(tbr.m_TicketBoxID);
	GETLONG(tbr.m_RoutingRuleID);
	GETLONG(tbr.m_Percentage);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketBoxRouting].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (tbr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), tbr.m_TicketBoxID );
		BINDPARAM_LONG( arc.GetQuery(), tbr.m_RoutingRuleID );
		BINDCOL_LONG( arc.GetQuery(), tbr.m_TicketBoxRoutingID );
		arc.GetQuery().Execute( _T("SELECT TicketBoxRoutingID FROM TicketBoxRouting ")
								_T("WHERE TicketBoxID=? AND RoutingRuleID=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (tbr.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tbr.m_TicketBoxRoutingID );
			BINDPARAM_LONG( arc.GetQuery(), tbr.m_TicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), tbr.m_RoutingRuleID );
			BINDPARAM_LONG( arc.GetQuery(), tbr.m_Percentage );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketBoxRouting ON INSERT INTO TicketBoxRouting ")
						            _T("(TicketBoxRoutingID,TicketBoxID,RoutingRuleID,Percentage) ")
						            _T("VALUES")
						            _T("(?,?,?,?) SET IDENTITY_INSERT TicketBoxRouting OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for TicketBoxRouting
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketCategories

int StuffTicketCategoryRecord(CArchiveFile& arc, long ID)
{
	TTicketCategories tc;
	int ret;
	tc.m_TicketCategoryID = ID;

	// Query from the databse
	if(tc.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(tc.m_Description);
		PUTLONG(tc.m_ObjectID);
		PUTBYTE(tc.m_BuiltIn);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketCategoryRecord(CArchiveFile& arc, long ID)
{
	TTicketCategories tc;
	int ret = Arc_Success;
	tc.m_TicketCategoryID = ID;

	GETCHAR(tc.m_Description, TICKETCATEGORIES_DESCRIPTION_LENGTH);
	GETLONG(tc.m_ObjectID);
	GETBYTE(tc.m_BuiltIn);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketCategories].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (tc.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tc.m_Description );
		BINDCOL_LONG( arc.GetQuery(), tc.m_TicketCategoryID );
		arc.GetQuery().Execute( _T("SELECT TicketCategoryID FROM TicketCategories ")
								_T("WHERE Description=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (tc.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tc.m_TicketCategoryID );
			BINDPARAM_TCHAR( arc.GetQuery(), tc.m_Description );
			BINDPARAM_LONG( arc.GetQuery(), tc.m_ObjectID );
			BINDPARAM_BIT( arc.GetQuery(), tc.m_BuiltIn );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketCategories ON INSERT INTO TicketCategories ")
						            _T("(TicketCategoryID,Description,ObjectID,BuiltIn) ")
						            _T("VALUES")
						            _T("(?,?,?,?) SET IDENTITY_INSERT TicketCategories OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for TicketCategories
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for WaterMarkAlerts

int StuffWaterMarkAlertRecord(CArchiveFile& arc, long ID)
{
	TWaterMarkAlerts wa;
	int ret;
	wa.m_WaterMarkAlertID = ID;

	// Query from the databse
	if(wa.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(wa.m_Description);
		PUTLONG(wa.m_HitCount);
		PUTLONG(wa.m_LowWaterMark);
		PUTLONG(wa.m_HighWaterMark);
		PUTBYTE(wa.m_IsEnabled);
		PUTBYTE(wa.m_SendLowAlert);
		PUTLONG(wa.m_AlertOnTypeID);
		PUTLONG(wa.m_AlertOnID);
		PUTLONG(wa.m_AlertToTypeID);
		PUTLONG(wa.m_AlertToID);
		PUTLONG(wa.m_AlertMethodID);
		PUTCHAR(wa.m_EmailAddress);
		PUTCHAR(wa.m_FromEmailAddress);
		PUTDATE(wa.m_DateCreated);
		PUTDATE(wa.m_DateEdited);
		PUTLONG(wa.m_CreatedByID);
		PUTLONG(wa.m_EditedByID);
		PUTBYTE(wa.m_WaterMarkStatus);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveWaterMarkAlertRecord(CArchiveFile& arc, long ID)
{
	TWaterMarkAlerts wa;
	int ret = Arc_Success;
	wa.m_WaterMarkAlertID = ID;

	GETCHAR(wa.m_Description, 255);
	GETLONG(wa.m_HitCount);
	GETLONG(wa.m_LowWaterMark);
	GETLONG(wa.m_HighWaterMark);
	GETBYTE(wa.m_IsEnabled);
	GETBYTE(wa.m_SendLowAlert);
	GETLONG(wa.m_AlertOnTypeID);
	GETLONG(wa.m_AlertOnID);
	GETLONG(wa.m_AlertToTypeID);
	GETLONG(wa.m_AlertToID);
	GETLONG(wa.m_AlertMethodID);
	GETCHAR(wa.m_EmailAddress, 255);
	GETCHAR(wa.m_FromEmailAddress, 255);
	GETDATE(wa.m_DateCreated);
	GETDATE(wa.m_DateEdited);
	GETLONG(wa.m_CreatedByID);
	arc.TranslateID(TABLEID_Agents, wa.m_CreatedByID);
	GETLONG(wa.m_EditedByID);
	arc.TranslateID(TABLEID_Agents, wa.m_EditedByID);
	GETBYTE(wa.m_WaterMarkStatus);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_WaterMarkAlerts].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (wa.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), wa.m_Description );
		BINDCOL_LONG( arc.GetQuery(), wa.m_WaterMarkAlertID );
		arc.GetQuery().Execute( _T("SELECT WaterMarkAlertID FROM WaterMarkAlerts ")
								_T("WHERE Description=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (wa.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_TCHAR( arc.GetQuery(), wa.m_Description );
			BINDPARAM_LONG( arc.GetQuery(), wa.m_HitCount );
			BINDPARAM_LONG( arc.GetQuery(), wa.m_LowWaterMark );
			BINDPARAM_LONG( arc.GetQuery(), wa.m_HighWaterMark );
			BINDPARAM_BIT( arc.GetQuery(), wa.m_IsEnabled );
			BINDPARAM_BIT( arc.GetQuery(), wa.m_SendLowAlert );
			BINDPARAM_LONG( arc.GetQuery(), wa.m_AlertOnTypeID );
			BINDPARAM_LONG( arc.GetQuery(), wa.m_AlertOnID );
			BINDPARAM_LONG( arc.GetQuery(), wa.m_AlertToTypeID );
			BINDPARAM_LONG( arc.GetQuery(), wa.m_AlertToID );
			BINDPARAM_LONG( arc.GetQuery(), wa.m_AlertMethodID );
			BINDPARAM_TCHAR( arc.GetQuery(), wa.m_EmailAddress );
			BINDPARAM_TCHAR( arc.GetQuery(), wa.m_FromEmailAddress );
			BINDPARAM_LONG( arc.GetQuery(), wa.m_CreatedByID );
			BINDPARAM_LONG( arc.GetQuery(), wa.m_EditedByID );
			BINDPARAM_TINYINT( arc.GetQuery(), wa.m_WaterMarkStatus );
			BINDPARAM_TIME( arc.GetQuery(), wa.m_DateCreated );
			BINDPARAM_TIME( arc.GetQuery(), wa.m_DateEdited );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT WaterMarkAlerts ON INSERT INTO WaterMarkAlerts ")
						_T("(Description,HitCount,LowWaterMark,HighWaterMark,IsEnabled,SendLowAlert,AlertOnTypeID,AlertOnID,AlertToTypeID,AlertToID,AlertMethodID,EmailAddress,FromEmailAddress,CreatedByID,EditedByID,WaterMarkStatus,DateCreated,DateEdited) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT WaterMarkAlerts OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for WaterMarkAlerts
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for ProcessingRules
int StuffProcessingRuleRecord( CArchiveFile& arc, long ID )
{
	TProcessingRules pr;
	int ret;
	pr.m_ProcessingRuleID = ID;

	// Query from the databse
	if( pr.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( pr.m_RuleDescrip );
		PUTBYTE( pr.m_IsEnabled );
		PUTLONG( pr.m_OrderIndex );
		PUTLONG( pr.m_HitCount );
		PUTBYTE( pr.m_PrePost );
		PUTLONG( pr.m_ActionType );
		PUTLONG( pr.m_ActionID );		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveProcessingRuleRecord( CArchiveFile& arc, long ID )
{
	TProcessingRules pr;
	int ret = Arc_Success;
	pr.m_ProcessingRuleID = ID;

	GETCHAR( pr.m_RuleDescrip, 125 );
	PUTBYTE( pr.m_IsEnabled );
	PUTLONG( pr.m_OrderIndex );
	PUTLONG( pr.m_HitCount );
	PUTBYTE( pr.m_PrePost );
	PUTLONG( pr.m_ActionType );
	PUTLONG( pr.m_ActionID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ProcessingRules].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( pr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), pr.m_RuleDescrip );
		BINDPARAM_LONG( arc.GetQuery(), pr.m_OrderIndex );
		BINDCOL_LONG( arc.GetQuery(), pr.m_ProcessingRuleID );
		arc.GetQuery().Execute( _T("SELECT ProcessingRuleID FROM ProcessingRules ")
								_T("WHERE RuleDescrip=? AND OrderIndex=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( pr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//pr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), pr.m_ProcessingRuleID );
			BINDPARAM_TCHAR( arc.GetQuery(), pr.m_RuleDescrip );
			BINDPARAM_BIT( arc.GetQuery(), pr.m_IsEnabled );
			BINDPARAM_LONG( arc.GetQuery(), pr.m_OrderIndex );
			BINDPARAM_LONG( arc.GetQuery(), pr.m_HitCount );
			BINDPARAM_BIT( arc.GetQuery(), pr.m_PrePost );
			BINDPARAM_LONG( arc.GetQuery(), pr.m_ActionType );
			BINDPARAM_LONG( arc.GetQuery(), pr.m_ActionID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ProcessingRules ON INSERT INTO ProcessingRules ")
						_T("(ProcessingRuleID,RuleDescrip,IsEnabled,OrderIndex,HitCount,PrePost,ActionType,ActionID) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?) SET IDENTITY_INSERT ProcessingRules OFF") );
		}
	}
	
	arc.AddIDMapping( TABLEID_ProcessingRules, ID, pr.m_ProcessingRuleID );
	
	return ret;	
}
// End of Stuff and Retrieve for ProcessingRules
//*****************************************************************************

////////////////////////////////////////////////////////////////////////////////
// 
// StuffMatchFromAddressRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffMatchFromAddressPRecord( CArchiveFile& arc, long ID )
{
	TMatchFromAddressP mfa;
	int ret;
	mfa.m_MatchID = ID;

	// Query from the databse
	if( mfa.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( mfa.m_ProcessingRuleID );
		PUTCHAR( mfa.m_EmailAddress );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveMatchFromAddressPRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveMatchFromAddressPRecord( CArchiveFile& arc, long ID )
{
	TMatchFromAddressP mfa;
	int ret = Arc_Success;

	mfa.m_MatchID = ID;

	GETLONG( mfa.m_ProcessingRuleID );
	arc.TranslateID( TABLEID_ProcessingRules, mfa.m_ProcessingRuleID );
	GETCHAR( mfa.m_EmailAddress, MATCHFROMADDRESSES_EMAILADDRESS_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_MatchFromAddressP].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( mfa.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), mfa.m_ProcessingRuleID );
		BINDPARAM_TCHAR( arc.GetQuery(), mfa.m_EmailAddress );
		BINDCOL_LONG( arc.GetQuery(), mfa.m_MatchID );
		arc.GetQuery().Execute( _T("SELECT MatchID FROM MatchFromAddressP ")
								_T("WHERE ProcessingRuleID=? AND EmailAddress=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( mfa.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			mfa.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffMatchTextPRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffMatchTextPRecord( CArchiveFile& arc, long ID )
{
	TMatchTextP mt;
	int ret;
	mt.m_MatchID = ID;

	// Query from the databse
	if( mt.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( mt.m_ProcessingRuleID );
		PUTBYTE( mt.m_MatchLocation );
		PUTBYTE( mt.m_IsRegEx );
		PUTTEXT( mt.m_MatchText );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveMatchTextPRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveMatchTextPRecord( CArchiveFile& arc, long ID )
{
	int ret = Arc_Success;

	try
	{
		TMatchTextP mt;

		mt.m_MatchID = ID;

		GETLONG( mt.m_ProcessingRuleID );
		arc.TranslateID( TABLEID_ProcessingRules, mt.m_ProcessingRuleID );
		GETBYTE( mt.m_MatchLocation );
		GETBYTE( mt.m_IsRegEx );
		GETTEXT( mt.m_MatchText );

		// Is the ID reserved?
		if( ID <= g_ArcTblInfo[TABLEID_MatchTextP].ReservedID ) 
		{ 
			if( arc.Overwrite() )
			{
				ret = ( mt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//DEBUGPRINT(_T("* SELECT MatchID FROM MatchText WHERE MatchLocation=%d AND MatchText LIKE %s\n"), mt.m_MatchLocation,mt.m_MatchText);
			// Check if record already exists
			arc.GetQuery().Initialize();
			BINDPARAM_TINYINT( arc.GetQuery(), mt.m_MatchLocation );
			BINDPARAM_BIT( arc.GetQuery(), mt.m_IsRegEx );
			BINDPARAM_TCHAR( arc.GetQuery(), mt.m_MatchText );
			BINDCOL_LONG( arc.GetQuery(), mt.m_MatchID );
			arc.GetQuery().Execute( _T("SELECT MatchID FROM MatchTextP ")
									_T("WHERE MatchLocation=? AND IsRegEx=? AND MatchText LIKE ?") );

			if( arc.GetQuery().Fetch() == S_OK )
			{
				// Record exists, we now have the new ID
				if( arc.Overwrite() )
				{
					ret = ( mt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
				}
			}
			else
			{
				mt.Insert( arc.GetQuery() );
			}
		}
	}
	catch(...)
	{
	}
	
	return ret;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// StuffMatchToAddressPRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffMatchToAddressPRecord( CArchiveFile& arc, long ID )
{
	TMatchToAddressP mta;
	int ret;
	mta.m_MatchID = ID;

	// Query from the databse
	if( mta.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( mta.m_ProcessingRuleID );
		PUTCHAR( mta.m_EmailAddress );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveMatchToAddressPRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveMatchToAddressPRecord( CArchiveFile& arc, long ID )
{
	TMatchToAddressP mta;
	int ret = Arc_Success;

	mta.m_MatchID = ID;

	GETLONG( mta.m_ProcessingRuleID );
	arc.TranslateID( TABLEID_ProcessingRules, mta.m_ProcessingRuleID );
	GETCHAR( mta.m_EmailAddress, MATCHFROMADDRESSES_EMAILADDRESS_LENGTH );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_MatchToAddressP].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( mta.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), mta.m_ProcessingRuleID );
		BINDPARAM_TCHAR( arc.GetQuery(), mta.m_EmailAddress );
		BINDCOL_LONG( arc.GetQuery(), mta.m_MatchID );
		arc.GetQuery().Execute( _T("SELECT MatchID FROM MatchToAddressP ")
								_T("WHERE ProcessingRuleID=? AND EmailAddress=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( mta.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			mta.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}


//*****************************************************************************
// Stuff and Retrieve for ReportTypes
int StuffReportTypeRecord( CArchiveFile& arc, long ID )
{
	TReportTypes rt;
	int ret;
	rt.m_ReportTypeID = ID;

	// Query from the databse
	if( rt.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( rt.m_Description );
		PUTBYTE( rt.m_IsEnabled );
		PUTBYTE( rt.m_AllowCustom );	
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveReportTypeRecord( CArchiveFile& arc, long ID )
{
	TReportTypes rt;
	int ret = Arc_Success;
	rt.m_ReportTypeID = ID;

	GETCHAR( rt.m_Description, 50 );
	GETBYTE( rt.m_IsEnabled );
	GETBYTE( rt.m_AllowCustom );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ReportTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( rt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), rt.m_Description );
		BINDCOL_LONG( arc.GetQuery(), rt.m_ReportTypeID );
		arc.GetQuery().Execute( _T("SELECT ReportTypeID FROM ReportTypes ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( rt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//rt.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), rt.m_ReportTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), rt.m_Description );
			BINDPARAM_BIT( arc.GetQuery(), rt.m_IsEnabled );
			BINDPARAM_BIT( arc.GetQuery(), rt.m_AllowCustom );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ReportTypes ON INSERT INTO ReportTypes ")
						_T("(ReportTypeID,Description,IsEnabled,AllowCustom) ")
						_T("VALUES")
						_T("(?,?,?,?) SET IDENTITY_INSERT ReportTypes OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for ReportTypes
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for ReportObjects
int StuffReportObjectRecord( CArchiveFile& arc, long ID )
{
	TReportObjects ro;
	int ret;
	ro.m_ReportObjectID = ID;

	// Query from the databse
	if( ro.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( ro.m_Description );
		PUTBYTE( ro.m_IsEnabled );
		PUTBYTE( ro.m_AllowCustom );	
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveReportObjectRecord( CArchiveFile& arc, long ID )
{
	TReportObjects ro;
	int ret = Arc_Success;
	ro.m_ReportObjectID = ID;

	GETCHAR( ro.m_Description, 50 );
	GETBYTE( ro.m_IsEnabled );
	GETBYTE( ro.m_AllowCustom );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ReportObjects].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ro.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), ro.m_Description );
		BINDCOL_LONG( arc.GetQuery(), ro.m_ReportObjectID );
		arc.GetQuery().Execute( _T("SELECT ReportObjectID FROM ReportObjects ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ro.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ro.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ro.m_ReportObjectID );
			BINDPARAM_TCHAR( arc.GetQuery(), ro.m_Description );
			BINDPARAM_BIT( arc.GetQuery(), ro.m_IsEnabled );
			BINDPARAM_BIT( arc.GetQuery(), ro.m_AllowCustom );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ReportObjects ON INSERT INTO ReportObjects ")
						_T("(ReportObjectID,Description,IsEnabled,AllowCustom) ")
						_T("VALUES")
						_T("(?,?,?,?) SET IDENTITY_INSERT ReportObjects OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for ReportObjects
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for ReportResults
int StuffReportResultRecord( CArchiveFile& arc, long ID )
{
	TReportResults rptr;
	int ret;
	rptr.m_ReportResultID = ID;

	// Query from the databse
	if( rptr.Query2( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( rptr.m_ScheduledReportID );
		PUTCHAR( rptr.m_ResultFile );
		PUTLONG( rptr.m_ResultCode );
		PUTDATE( rptr.m_DateRan );
		PUTDATE( rptr.m_DateFrom );
		PUTDATE( rptr.m_DateTo );	
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveReportResultRecord( CArchiveFile& arc, long ID )
{
	TReportResults rptr;
	int ret = Arc_Success;
	rptr.m_ReportResultID = ID;

	GETLONG( rptr.m_ScheduledReportID );
	GETCHAR( rptr.m_ResultFile, 255 );
	GETLONG( rptr.m_ResultCode );
	GETDATE( rptr.m_DateRan );
	GETDATE( rptr.m_DateFrom );
	GETDATE( rptr.m_DateTo );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ReportResults].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( rptr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), rptr.m_ScheduledReportID );
		BINDPARAM_TCHAR( arc.GetQuery(), rptr.m_ResultFile );
		BINDCOL_LONG( arc.GetQuery(), rptr.m_ReportResultID );
		arc.GetQuery().Execute( _T("SELECT ReportResultID FROM ReportResults ")
								_T("WHERE ScheduledReportID=? AND ResultFile=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( rptr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//rptr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), rptr.m_ReportResultID );
			BINDPARAM_TCHAR( arc.GetQuery(), rptr.m_ResultFile );
			BINDPARAM_LONG( arc.GetQuery(), rptr.m_ResultCode );
			BINDPARAM_TIME( arc.GetQuery(), rptr.m_DateRan );
			BINDPARAM_TIME( arc.GetQuery(), rptr.m_DateFrom );
			BINDPARAM_TIME( arc.GetQuery(), rptr.m_DateTo );
			BINDPARAM_LONG( arc.GetQuery(), rptr.m_ScheduledReportID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ReportResults ON INSERT INTO ReportResults ")
						_T("(ReportResultID,ResultFile,ResultCode,DateRan,DateFrom,DateTo,ScheduledReportID) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?) SET IDENTITY_INSERT ReportResults OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for ReportResults
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for ReportResultRows
int StuffReportResultRowRecord( CArchiveFile& arc, long ID )
{
	TReportResultRows rrr;
	int ret;
	rrr.m_ReportResultRowsID = ID;

	// Query from the databse
	if( rrr.Query2( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( rrr.m_ReportResultID );
		PUTCHAR( rrr.m_Col1 );
		PUTCHAR( rrr.m_Col2 );
		PUTLONG( rrr.m_Col3 );
		PUTLONG( rrr.m_Col4 );
		PUTLONG( rrr.m_Col5 );
		PUTLONG( rrr.m_Col6 );
		PUTLONG( rrr.m_Col7 );
		PUTLONG( rrr.m_Col8 );
		PUTLONG( rrr.m_Col9 );
		PUTLONG( rrr.m_Col10 );
		PUTLONG( rrr.m_Col11 );
		PUTLONG( rrr.m_Col12 );
		PUTLONG( rrr.m_Col13 );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveReportResultRowRecord( CArchiveFile& arc, long ID )
{
	TReportResultRows rrr;
	int ret = Arc_Success;
	rrr.m_ReportResultRowsID = ID;

	GETLONG( rrr.m_ReportResultID );
	GETCHAR( rrr.m_Col1, 50 );
	GETCHAR( rrr.m_Col2, 50 );
	GETLONG( rrr.m_Col3 );
	GETDECIMAL( rrr.m_Col4 );
	GETLONG( rrr.m_Col5 );
	GETLONG( rrr.m_Col6 );
	GETLONG( rrr.m_Col7 );
	GETLONG( rrr.m_Col8 );
	GETLONG( rrr.m_Col9 );
	GETLONG( rrr.m_Col10 );
	GETLONG( rrr.m_Col11 );
	GETLONG( rrr.m_Col12 );
	GETLONG( rrr.m_Col13 );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ReportResultRows].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( rrr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), rrr.m_ReportResultID );
		BINDPARAM_TCHAR( arc.GetQuery(), rrr.m_Col1 );
		BINDCOL_LONG( arc.GetQuery(), rrr.m_ReportResultRowsID );
		arc.GetQuery().Execute( _T("SELECT ReportResultRowsID FROM ReportResultRows ")
								_T("WHERE ReportResultID=? AND Col1=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( rrr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			rrr.Insert( arc.GetQuery() );
		}
	}
		
	return ret;	
}
// End of Stuff and Retrieve for ReportResultRows
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for ReportCustom
int StuffReportCustomRecord( CArchiveFile& arc, long ID )
{
	TCustomReport cr;
	int ret;
	cr.m_CustomReportID = ID;

	// Query from the databse
	if( cr.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( cr.m_Description );
		PUTBYTE( cr.m_IsEnabled );
		PUTLONG( cr.m_ReportTypeID );
		PUTLONG( cr.m_ReportObjectID );
		PUTBYTE( cr.m_BuiltIn );
		PUTLONG( cr.m_CreatedBy );
		PUTDATE( cr.m_DateCreated );
		PUTLONG( cr.m_EditedBy );
		PUTDATE( cr.m_DateEdited );
		PUTTEXT( cr.m_Query );	
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveReportCustomRecord( CArchiveFile& arc, long ID )
{
	TCustomReport cr;
	int ret = Arc_Success;
	cr.m_CustomReportID = ID;

	GETCHAR( cr.m_Description, 255 );
	GETBYTE( cr.m_IsEnabled );
	GETLONG( cr.m_ReportTypeID );
	GETLONG( cr.m_ReportObjectID );
	GETBYTE( cr.m_BuiltIn );
	GETLONG( cr.m_CreatedBy );
	GETDATE( cr.m_DateCreated );
	GETLONG( cr.m_EditedBy );
	GETDATE( cr.m_DateEdited );
	GETTEXT( cr.m_Query );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ReportCustom].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( cr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), cr.m_Description );
		BINDCOL_LONG( arc.GetQuery(), cr.m_CustomReportID );
		arc.GetQuery().Execute( _T("SELECT CustomReportID FROM ReportCustom ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( cr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//cr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), cr.m_CustomReportID );
			BINDPARAM_TCHAR( arc.GetQuery(), cr.m_Description );
			BINDPARAM_BIT( arc.GetQuery(), cr.m_IsEnabled );
			BINDPARAM_LONG( arc.GetQuery(), cr.m_ReportTypeID );
			BINDPARAM_LONG( arc.GetQuery(), cr.m_ReportObjectID );
			BINDPARAM_BIT( arc.GetQuery(), cr.m_BuiltIn );
			BINDPARAM_LONG( arc.GetQuery(), cr.m_CreatedBy );
			BINDPARAM_TIME( arc.GetQuery(), cr.m_DateCreated );
			BINDPARAM_LONG( arc.GetQuery(), cr.m_EditedBy );
			BINDPARAM_TIME( arc.GetQuery(), cr.m_DateEdited );
			BINDPARAM_TEXT( arc.GetQuery(), cr.m_Query );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ReportCustom ON INSERT INTO ReportCustom ")
						_T("(CustomReportID,Description,IsEnabled,ReportTypeID,ReportObjectID,BuiltIn,CreatedBy,DateCreated,EditedBy,DateEdited,Query) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT ReportCustom OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for ReportCustom
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for ReportStandard
int StuffReportStandardRecord( CArchiveFile& arc, long ID )
{
	TStandardReport sr;
	int ret;
	sr.m_StandardReportID = ID;

	// Query from the databse
	if( sr.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( sr.m_Description );
		PUTBYTE( sr.m_IsEnabled );
		PUTBYTE( sr.m_CanSchedule );
		PUTLONG( sr.m_ReportTypeID );
		PUTLONG( sr.m_ReportObjectID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveReportStandardRecord( CArchiveFile& arc, long ID )
{
	TStandardReport sr;
	int ret = Arc_Success;
	sr.m_StandardReportID = ID;

	GETCHAR( sr.m_Description, 255 );
	GETBYTE( sr.m_IsEnabled );
	GETBYTE( sr.m_CanSchedule );
	GETLONG( sr.m_ReportTypeID );
	GETLONG( sr.m_ReportObjectID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ReportStandard].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( sr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), sr.m_Description );
		BINDCOL_LONG( arc.GetQuery(), sr.m_StandardReportID );
		arc.GetQuery().Execute( _T("SELECT StandardReportID FROM ReportStandard ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( sr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//sr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), sr.m_StandardReportID );
			BINDPARAM_TCHAR( arc.GetQuery(), sr.m_Description );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_IsEnabled );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_CanSchedule );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_ReportTypeID );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_ReportObjectID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ReportStandard ON INSERT INTO ReportStandard ")
						_T("(StandardReportID,Description,IsEnabled,CanSchedule,ReportTypeID,ReportObjectID) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?) SET IDENTITY_INSERT ReportStandard OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for ReportStandard
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for ReportScheduled
int StuffReportScheduledRecord( CArchiveFile& arc, long ID )
{
	TScheduleReport sr;
	int ret;
	sr.m_ScheduledReportID = ID;

	// Query from the databse
	if( sr.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( sr.m_Description );
		PUTLONG( sr.m_ReportID );
		PUTBYTE( sr.m_IsCustom );
		PUTBYTE( sr.m_IsEnabled );
		PUTLONG( sr.m_OwnerID );
		PUTLONG( sr.m_EditedByID );
		PUTDATE( sr.m_DateEdited );
		PUTDATE( sr.m_NextRunTime );
		PUTLONG( sr.m_RunFreq );
		PUTLONG( sr.m_RunOn );
		PUTBYTE( sr.m_RunSun );
		PUTBYTE( sr.m_RunMon );
		PUTBYTE( sr.m_RunTue );
		PUTBYTE( sr.m_RunWed );
		PUTBYTE( sr.m_RunThur );
		PUTBYTE( sr.m_RunFri );
		PUTBYTE( sr.m_RunSat );
		PUTLONG( sr.m_RunAtHour );
		PUTLONG( sr.m_RunAtMin );
		PUTLONG( sr.m_SumPeriod );
		PUTLONG( sr.m_MaxResults );
		PUTBYTE( sr.m_SendAlertToOwner );
		PUTBYTE( sr.m_SendResultToEmail );
		PUTCHAR( sr.m_ResultEmailTo );
		PUTBYTE( sr.m_IncludeResultFile );
		PUTBYTE( sr.m_SaveResultToFile );
		PUTLONG( sr.m_KeepNumResultFile );
		PUTBYTE( sr.m_AllowConsolidation );
		PUTLONG( sr.m_TargetID );
		PUTBYTE( sr.m_Flag1 );
		PUTBYTE( sr.m_Flag2 );
		PUTLONG( sr.m_TicketStateID );
		PUTLONG( sr.m_CreatedByID );
		PUTDATE( sr.m_DateCreated );
		PUTDATE( sr.m_LastRunTime );
		PUTLONG( sr.m_LastResultCode );
		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveReportScheduledRecord( CArchiveFile& arc, long ID )
{
	TScheduleReport sr;
	int ret = Arc_Success;
	sr.m_ScheduledReportID = ID;

	GETCHAR( sr.m_Description, 255 );
	GETLONG( sr.m_ReportID );
	GETBYTE( sr.m_IsCustom );
	GETBYTE( sr.m_IsEnabled );
	GETLONG( sr.m_OwnerID );
	GETLONG( sr.m_EditedByID );
	GETDATE( sr.m_DateEdited );
	GETDATE( sr.m_NextRunTime );
	GETLONG( sr.m_RunFreq );
	GETLONG( sr.m_RunOn );
	GETBYTE( sr.m_RunSun );
	GETBYTE( sr.m_RunMon );
	GETBYTE( sr.m_RunTue );
	GETBYTE( sr.m_RunWed );
	GETBYTE( sr.m_RunThur );
	GETBYTE( sr.m_RunFri );
	GETBYTE( sr.m_RunSat );
	GETLONG( sr.m_RunAtHour );
	GETLONG( sr.m_RunAtMin );
	GETLONG( sr.m_SumPeriod );
	GETLONG( sr.m_MaxResults );
	GETBYTE( sr.m_SendAlertToOwner );
	GETBYTE( sr.m_SendResultToEmail );
	GETCHAR( sr.m_ResultEmailTo, 255 );
	GETBYTE( sr.m_IncludeResultFile );
	GETBYTE( sr.m_SaveResultToFile );
	GETLONG( sr.m_KeepNumResultFile );
	GETBYTE( sr.m_AllowConsolidation );
	GETLONG( sr.m_TargetID );
	GETBYTE( sr.m_Flag1 );
	GETBYTE( sr.m_Flag2 );
	GETLONG( sr.m_TicketStateID );
	GETLONG( sr.m_CreatedByID );
	GETDATE( sr.m_DateCreated );
	GETDATE( sr.m_LastRunTime );
	GETLONG( sr.m_LastResultCode );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ReportScheduled].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( sr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), sr.m_Description );
		BINDCOL_LONG( arc.GetQuery(), sr.m_ScheduledReportID );
		arc.GetQuery().Execute( _T("SELECT ScheduledReportID FROM ReportScheduled ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( sr.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//sr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), sr.m_ScheduledReportID );
			BINDPARAM_TCHAR( arc.GetQuery(), sr.m_Description );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_ReportID );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_IsCustom );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_IsEnabled );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_OwnerID );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_CreatedByID );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_EditedByID );
			BINDPARAM_TIME( arc.GetQuery(), sr.m_NextRunTime );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_RunFreq );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_RunOn );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_RunSun );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_RunMon );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_RunTue );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_RunWed );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_RunThur );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_RunFri );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_RunSat );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_RunAtHour );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_RunAtMin );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_SumPeriod );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_MaxResults );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_SendAlertToOwner );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_SendResultToEmail );
			BINDPARAM_TCHAR( arc.GetQuery(), sr.m_ResultEmailTo );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_IncludeResultFile );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_SaveResultToFile );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_KeepNumResultFile );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_AllowConsolidation );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_TargetID );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_Flag1 );
			BINDPARAM_BIT( arc.GetQuery(), sr.m_Flag2 );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_TicketStateID );
			BINDPARAM_TIME( arc.GetQuery(), sr.m_DateCreated );
			BINDPARAM_TIME( arc.GetQuery(), sr.m_DateEdited );
			BINDPARAM_TIME( arc.GetQuery(), sr.m_LastRunTime );
			BINDPARAM_LONG( arc.GetQuery(), sr.m_LastResultCode );			
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ReportScheduled ON INSERT INTO ReportScheduled ")
						_T("(ScheduledReportID,Description,ReportID,IsCustom,IsEnabled,OwnerID,CreatedBy,EditedBy,NextRunTime,RunFreq,RunOn,RunSun,RunMon,RunTue,RunWed,RunThur,RunFri,RunSat,RunAtHour,RunAtMin,SumPeriod,MaxResults,SendAlertToOwner,SendResultToEmail,ResultEmailTo,IncludeResultFile,SaveResultToFile,KeepNumResultFile,AllowConsolidation,TargetID,Flag1,Flag2,TicketStateID,DateCreated,DateEdited,LastRunTime,LastResultCode) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT ReportScheduled OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for ReportScheduled
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketFields
int StuffTicketFieldRecord( CArchiveFile& arc, long ID )
{
	TTicketFields tf;
	int ret;
	tf.m_TicketFieldID = ID;

	// Query from the databse
	if( tf.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( tf.m_Description );
		PUTLONG( tf.m_TicketFieldTypeID );		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketFieldRecord( CArchiveFile& arc, long ID )
{
	TTicketFields tf;
	int ret = Arc_Success;
	tf.m_TicketFieldID = ID;

	GETCHAR( tf.m_Description, 50 );
	GETLONG( tf.m_TicketFieldTypeID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketFields].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tf.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tf.m_Description );
		BINDCOL_LONG( arc.GetQuery(), tf.m_TicketFieldID );
		arc.GetQuery().Execute( _T("SELECT TicketFieldID FROM TicketFields ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tf.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tf.m_TicketFieldID );
			BINDPARAM_TCHAR( arc.GetQuery(), tf.m_Description );
			BINDPARAM_LONG( arc.GetQuery(), tf.m_TicketFieldTypeID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketFields ON INSERT INTO TicketFields ")
						_T("(TicketFieldID,Description,TicketFieldTypeID) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT TicketFields OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for TicketFields
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketFieldTypes
int StuffTicketFieldTypeRecord( CArchiveFile& arc, long ID )
{
	TTicketFieldTypes tft;
	int ret;
	tft.m_TicketFieldTypeID = ID;

	// Query from the databse
	if( tft.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( tft.m_Description );				
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketFieldTypeRecord( CArchiveFile& arc, long ID )
{
	TTicketFieldTypes tft;
	int ret = Arc_Success;
	tft.m_TicketFieldTypeID = ID;

	GETCHAR( tft.m_Description, 50 );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketFieldTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tft.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tft.m_Description );
		BINDCOL_LONG( arc.GetQuery(), tft.m_TicketFieldTypeID );
		arc.GetQuery().Execute( _T("SELECT TicketFieldTypeID FROM TicketFieldTypes ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tft.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tft.m_TicketFieldTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), tft.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketFieldTypes ON INSERT INTO TicketFieldTypes ")
						_T("(TicketFieldTypeID,Description) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT TicketFieldTypes OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for TicketFieldTypes
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketFieldOptions
int StuffTicketFieldOptionRecord( CArchiveFile& arc, long ID )
{
	TTicketFieldOptions tfo;
	int ret;
	tfo.m_TicketFieldOptionID = ID;

	// Query from the databse
	if( tfo.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( tfo.m_TicketFieldID );
		PUTCHAR( tfo.m_OptionValue );
		PUTLONG( tfo.m_OptionOrder );	
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketFieldOptionRecord( CArchiveFile& arc, long ID )
{
	TTicketFieldOptions tfo;
	int ret = Arc_Success;
	tfo.m_TicketFieldOptionID = ID;

	GETLONG( tfo.m_TicketFieldID );
	GETCHAR( tfo.m_OptionValue, 50 );
	GETLONG( tfo.m_OptionOrder );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketFieldOptions].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tfo.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), tfo.m_TicketFieldOptionID );
		BINDPARAM_LONG( arc.GetQuery(), tfo.m_TicketFieldID );
		BINDPARAM_TCHAR( arc.GetQuery(), tfo.m_OptionValue );
		BINDPARAM_LONG( arc.GetQuery(), tfo.m_OptionOrder );
		arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketFieldOptions ON INSERT INTO TicketFieldOptions ")
					_T("(TicketFieldOptionID,TicketFieldID,OptionValue,OptionOrder) ")
					_T("VALUES")
					_T("(?,?,?,?) SET IDENTITY_INSERT TicketFieldOptions OFF") );
	}
	
	return ret;	
}

// End of Stuff and Retrieve for TicketFieldOptions
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketFieldsTicket
int StuffTicketFieldsTicketRecord( CArchiveFile& arc, long ID )
{
	TTicketFieldsTicket tft;
	int ret;
	tft.m_TicketFieldsTicketID = ID;

	// Query from the databse
	if( tft.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( tft.m_TicketID );
		PUTLONG( tft.m_TicketFieldID );
		PUTCHAR( tft.m_DataValue );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketFieldsTicketRecord( CArchiveFile& arc, long ID )
{
	TTicketFieldsTicket tft;
	int ret = Arc_Success;
	tft.m_TicketFieldsTicketID = ID;

	GETLONG( tft.m_TicketID );
	GETLONG( tft.m_TicketFieldID );
	GETCHAR( tft.m_DataValue, 255 );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketFieldsTicket].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tft.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), tft.m_TicketID );
		BINDPARAM_LONG( arc.GetQuery(), tft.m_TicketFieldID );
		BINDCOL_LONG( arc.GetQuery(), tft.m_TicketFieldsTicketID );
		arc.GetQuery().Execute( _T("SELECT TicketFieldsTicketID FROM TicketFieldsTicket ")
								_T("WHERE TicketID=? AND TicketFieldID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tft.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tft.m_TicketFieldsTicketID );
			BINDPARAM_LONG( arc.GetQuery(), tft.m_TicketID );
			BINDPARAM_LONG( arc.GetQuery(), tft.m_TicketFieldID );
			BINDPARAM_TCHAR( arc.GetQuery(), tft.m_DataValue );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketFieldsTicket ON INSERT INTO TicketFieldsTicket ")
						_T("(TicketFieldsTicketID,TicketID,TicketFieldID,DataValue) ")
						_T("VALUES")
						_T("(?,?,?,?) SET IDENTITY_INSERT TicketFieldsTicket OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for TicketFieldsTicket
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketFieldsTicketBox
int StuffTicketFieldsTicketBoxRecord( CArchiveFile& arc, long ID )
{
	TTicketFieldsTicketBox tftb;
	int ret;
	tftb.m_TicketFieldsTicketBoxID = ID;

	// Query from the databse
	if( tftb.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( tftb.m_TicketBoxID );
		PUTLONG( tftb.m_TicketFieldID );
		PUTBYTE( tftb.m_IsRequired );
		PUTBYTE( tftb.m_IsViewed );
		PUTBYTE( tftb.m_SetDefault );
        PUTCHAR( tftb.m_DefaultValue );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketFieldsTicketBoxRecord( CArchiveFile& arc, long ID )
{
	TTicketFieldsTicketBox tftb;
	int ret = Arc_Success;
	tftb.m_TicketFieldsTicketBoxID = ID;

	GETLONG( tftb.m_TicketBoxID );
	GETLONG( tftb.m_TicketFieldID );
	GETBYTE( tftb.m_IsRequired );
	GETBYTE( tftb.m_IsViewed );
	GETBYTE( tftb.m_SetDefault );
    GETCHAR( tftb.m_DefaultValue, 255 );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketFieldsTicketBox].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tftb.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), tftb.m_TicketBoxID );
		BINDPARAM_LONG( arc.GetQuery(), tftb.m_TicketFieldID );
		BINDCOL_LONG( arc.GetQuery(), tftb.m_TicketFieldsTicketBoxID );
		arc.GetQuery().Execute( _T("SELECT TicketFieldsTicketBoxID FROM TicketFieldsTicketBox ")
								_T("WHERE TicketBoxID=? AND TicketFieldID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tftb.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tftb.m_TicketFieldsTicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), tftb.m_TicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), tftb.m_TicketFieldID );
			BINDPARAM_BIT( arc.GetQuery(), tftb.m_IsRequired );
			BINDPARAM_BIT( arc.GetQuery(), tftb.m_IsViewed );
			BINDPARAM_BIT( arc.GetQuery(), tftb.m_SetDefault );
			BINDPARAM_TCHAR( arc.GetQuery(), tftb.m_DefaultValue );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketFieldsTicketBox ON INSERT INTO TicketFieldsTicketBox ")
						_T("(TicketFieldsTicketBoxID,TicketBoxID,TicketFieldID,IsRequired,IsViewed,SetDefault,DefaultValue) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?) SET IDENTITY_INSERT TicketFieldsTicket OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for TicketFieldsTicketBox
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for AgentActivities
int StuffAgentActivitiesRecord( CArchiveFile& arc, long ID )
{
	TAgentActivities aa;
	int ret;
	aa.m_AgentActivityID = ID;

	// Query from the databse
	if( aa.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( aa.m_Description );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveAgentActivitiesRecord( CArchiveFile& arc, long ID )
{
	TAgentActivities aa;
	int ret = Arc_Success;
	aa.m_AgentActivityID = ID;

	GETCHAR( aa.m_Description, 50 );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AgentActivities].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( aa.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), aa.m_Description );
		BINDCOL_LONG( arc.GetQuery(), aa.m_AgentActivityID );
		arc.GetQuery().Execute( _T("SELECT AgentActivityID FROM AgentActivities ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( aa.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AgentActivityID );
			BINDPARAM_TCHAR( arc.GetQuery(), aa.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AgentActivities ON INSERT INTO AgentActivities ")
						_T("(AgentActivityID,Description) ")
						_T("VALUES")
						_T("(?,?) SET IDENTITY_INSERT AgentActivities OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for AgentActivities
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for AgentActivityLog
int StuffAgentActivityLogRecord( CArchiveFile& arc, long ID )
{
	TAgentActivity aal;
	int ret;
	aal.m_AgentActivityID = ID;

	// Query from the databse
	if( aal.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( aal.m_AgentID );
		PUTLONG( aal.m_ActivityID );
		PUTDATE( aal.m_ActivityDate );
		PUTLONG( aal.m_ID1 );
		PUTLONG( aal.m_ID2 );
		PUTLONG( aal.m_ID3 );
		PUTCHAR( aal.m_Data1 );
		PUTCHAR( aal.m_Data2 );		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveAgentActivityLogRecord( CArchiveFile& arc, long ID )
{
	TAgentActivity aal;
	int ret = Arc_Success;
	aal.m_AgentActivityID = ID;

	GETLONG( aal.m_AgentID );
	GETLONG( aal.m_ActivityID );
	GETDATE( aal.m_ActivityDate );
	GETLONG( aal.m_ID1 );
	GETLONG( aal.m_ID2 );
	GETLONG( aal.m_ID3 );
	GETCHAR( aal.m_Data1, 50 );
	GETCHAR( aal.m_Data2, 50 );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AgentActivityLog].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( aal.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), aal.m_AgentActivityID );
		BINDPARAM_LONG( arc.GetQuery(), aal.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), aal.m_ActivityID );
		BINDPARAM_TIME( arc.GetQuery(), aal.m_ActivityDate );		
		arc.GetQuery().Execute( _T("SELECT AgentActivityID FROM AgentActivityLog ")
	                            _T("WHERE AgentID=? AND ActivityID=? AND ActivityDate=?") );
		
		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( aal.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//cr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), aal.m_AgentActivityID );
			BINDPARAM_LONG( arc.GetQuery(), aal.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), aal.m_ActivityID );
			BINDPARAM_TIME( arc.GetQuery(), aal.m_ActivityDate );
			BINDPARAM_LONG( arc.GetQuery(), aal.m_ID1 );
			BINDPARAM_LONG( arc.GetQuery(), aal.m_ID2 );
			BINDPARAM_LONG( arc.GetQuery(), aal.m_ID3 );
			BINDPARAM_TCHAR( arc.GetQuery(), aal.m_Data1 );
			BINDPARAM_TCHAR( arc.GetQuery(), aal.m_Data2 );	
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AgentActivityLog ON INSERT INTO AgentActivityLog ")
	               _T("(AgentActivityID,AgentID,ActivityID,ActivityDate,ID1,ID2,ID3,Data1,Data2) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT AgentActivityLog OFF") );						
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for AgentActivityLog
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for AgentTicketCategories
int StuffAgentTicketCategoriesRecord( CArchiveFile& arc, long ID )
{
	TAgentTicketCategories atc;
	int ret;
	atc.m_AgentTicketCategoryID = ID;

	// Query from the databse
	if( atc.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( atc.m_AgentID );
		PUTLONG( atc.m_TicketCategoryID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveAgentTicketCategoriesRecord( CArchiveFile& arc, long ID )
{
	TAgentTicketCategories atc;
	int ret;
	atc.m_AgentTicketCategoryID = ID;

	GETLONG( atc.m_AgentID );
	GETLONG( atc.m_TicketCategoryID );
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AgentTicketCategories].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( atc.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), atc.m_AgentTicketCategoryID );
		BINDPARAM_LONG( arc.GetQuery(), atc.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), atc.m_TicketCategoryID );
		arc.GetQuery().Execute( _T("SELECT AgentTicketCategoryID FROM AgentTicketCategories ")
								_T("WHERE AgentID=? AND TicketCategoryID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( atc.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), atc.m_AgentTicketCategoryID );
			BINDPARAM_LONG( arc.GetQuery(), atc.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), atc.m_TicketCategoryID );			
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AgentTicketCategories ON INSERT INTO AgentTicketCategories ")
						_T("(AgentTicketCategoryID,AgentID,TicketCategoryID) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT AgentTicketCategories OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for AgentTicketCategories
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for TicketFieldViews
int StuffTicketFieldViewRecord( CArchiveFile& arc, long ID )
{
	TTicketFieldViews tfv;
	int ret;
	tfv.m_TicketFieldViewID = ID;

	// Query from the databse
	if( tfv.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( tfv.m_TicketBoxViewID );
		PUTLONG( tfv.m_TicketFieldID );
		PUTBYTE( tfv.m_ShowField );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketFieldViewRecord( CArchiveFile& arc, long ID )
{
	TTicketFieldViews tfv;
	int ret;
	tfv.m_TicketFieldViewID = ID;

	GETLONG( tfv.m_TicketBoxViewID );
	GETLONG( tfv.m_TicketFieldID );
	GETBYTE( tfv.m_ShowField );
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketFieldViews].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( tfv.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), tfv.m_TicketFieldViewID );
		BINDPARAM_LONG( arc.GetQuery(), tfv.m_TicketBoxViewID );
		BINDPARAM_LONG( arc.GetQuery(), tfv.m_TicketFieldID );
		BINDPARAM_BIT( arc.GetQuery(), tfv.m_ShowField );
		arc.GetQuery().Execute( _T("SELECT TicketFieldViewID FROM TicketFieldViews ")
								_T("WHERE TicketBoxViewID=? AND TicketFieldID=? AND ShowField=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( tfv.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tfv.m_TicketFieldViewID );
			BINDPARAM_LONG( arc.GetQuery(), tfv.m_TicketBoxViewID );
			BINDPARAM_LONG( arc.GetQuery(), tfv.m_TicketFieldID );
			BINDPARAM_BIT( arc.GetQuery(), tfv.m_ShowField );			
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketFieldViews ON INSERT INTO TicketFieldViews ")
						_T("(TicketFieldViewID,TicketBoxViewID,TicketFieldID,ShowField) ")
						_T("VALUES")
						_T("(?,?,?,?) SET IDENTITY_INSERT TicketFieldViews OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for TicketFieldViews
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for VoipDialingCodes
int StuffVoipDialingCodeRecord( CArchiveFile& arc, long ID )
{
	TVoipDialingCodes vdc;
	int ret;
	vdc.m_VoipDialingCodeID = ID;

	// Query from the databse
	if( vdc.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( vdc.m_VoipServerID );
		PUTCHAR( vdc.m_Description );
		PUTCHAR( vdc.m_DialingCode );
		PUTBYTE( vdc.m_IsEnabled );
		PUTBYTE( vdc.m_IsDefault );			
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveVoipDialingCodeRecord( CArchiveFile& arc, long ID )
{
	TVoipDialingCodes vdc;
	int ret;
	vdc.m_VoipDialingCodeID = ID;

	GETLONG( vdc.m_VoipServerID );
	GETCHAR( vdc.m_Description, 50 );
	GETCHAR( vdc.m_DialingCode, 50 );
	GETBYTE( vdc.m_IsEnabled );
	GETBYTE( vdc.m_IsDefault );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_VoipDialingCodes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( vdc.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), vdc.m_VoipDialingCodeID );
		BINDPARAM_LONG( arc.GetQuery(), vdc.m_VoipServerID );
		BINDPARAM_TCHAR( arc.GetQuery(), vdc.m_Description );
		arc.GetQuery().Execute( _T("SELECT VoipDialingCodeID FROM VoipDialingCodes ")
	                            _T("WHERE VoipServerID=? AND Description=?") );
		
		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( vdc.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//vdc.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), vdc.m_VoipDialingCodeID );
			BINDPARAM_LONG( arc.GetQuery(), vdc.m_VoipServerID );
			BINDPARAM_TCHAR( arc.GetQuery(), vdc.m_Description );
			BINDPARAM_TCHAR( arc.GetQuery(), vdc.m_DialingCode );
			BINDPARAM_BIT( arc.GetQuery(), vdc.m_IsEnabled );
			BINDPARAM_BIT( arc.GetQuery(), vdc.m_IsDefault );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT VoipDialingCodes ON INSERT INTO VoipDialingCodes ")
	               _T("(VoipDialingCodeID,VoipServerID,Description,DialingCode,IsEnabled,IsDefault) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?) SET IDENTITY_INSERT VoipDialingCodes OFF") );						
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for VoipDialingCodes
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for VoipExtensions
int StuffVoipExtensionRecord( CArchiveFile& arc, long ID )
{
	TVoipExtensions ve;
	int ret;
	ve.m_VoipExtensionID = ID;

	// Query from the databse
	if( ve.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( ve.m_AgentID );
		PUTLONG( ve.m_VoipServerID );
		PUTCHAR( ve.m_Description );
		PUTLONG( ve.m_Extension );
		PUTCHAR( ve.m_Pin );
		PUTBYTE( ve.m_IsEnabled );
		PUTBYTE( ve.m_IsDefault );			
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveVoipExtensionRecord( CArchiveFile& arc, long ID )
{
	TVoipExtensions ve;
	int ret;
	ve.m_VoipExtensionID = ID;

	GETLONG( ve.m_AgentID );
	GETLONG( ve.m_VoipServerID );
	GETCHAR( ve.m_Description, 50 );
	GETLONG( ve.m_Extension );
	GETCHAR( ve.m_Pin, 50 );
	GETBYTE( ve.m_IsEnabled );
	GETBYTE( ve.m_IsDefault );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_VoipExtensions].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ve.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), ve.m_VoipExtensionID );
		BINDPARAM_LONG( arc.GetQuery(), ve.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), ve.m_VoipServerID );
		BINDPARAM_TCHAR( arc.GetQuery(), ve.m_Description );
		arc.GetQuery().Execute( _T("SELECT VoipExtensionID FROM VoipExtensions ")
	                            _T("WHERE AgentID=? AND VoipServerID=? AND Description=?") );
		
		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ve.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ve.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ve.m_VoipExtensionID );
			BINDPARAM_LONG( arc.GetQuery(), ve.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), ve.m_VoipServerID );
			BINDPARAM_TCHAR( arc.GetQuery(), ve.m_Description );
			BINDPARAM_LONG( arc.GetQuery(), ve.m_Extension );
			BINDPARAM_TCHAR( arc.GetQuery(), ve.m_Pin );
			BINDPARAM_BIT( arc.GetQuery(), ve.m_IsEnabled );
			BINDPARAM_BIT( arc.GetQuery(), ve.m_IsDefault );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT VoipExtensions ON INSERT INTO VoipExtensions ")
	               _T("(VoipExtensionID,AgentID,VoipServerID,Description,Extension,Pin,IsEnabled,IsDefault) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?) SET IDENTITY_INSERT VoipExtensions OFF") );						
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for VoipExtensions
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for VoipServers
int StuffVoipServerRecord( CArchiveFile& arc, long ID )
{
	TVoipServers vs;
	int ret;
	vs.m_VoipServerID = ID;

	// Query from the databse
	if( vs.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( vs.m_VoipServerTypeID );
		PUTCHAR( vs.m_Description );
		PUTCHAR( vs.m_Url );
		PUTLONG( vs.m_Port );
		PUTBYTE( vs.m_IsEnabled );
		PUTBYTE( vs.m_IsDefault );			
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveVoipServerRecord( CArchiveFile& arc, long ID )
{
	TVoipServers vs;
	int ret;
	vs.m_VoipServerID = ID;

	GETLONG( vs.m_VoipServerTypeID );
	GETCHAR( vs.m_Description, 50 );
	GETCHAR( vs.m_Url, 255 );
	GETLONG( vs.m_Port );
	GETBYTE( vs.m_IsEnabled );
	GETBYTE( vs.m_IsDefault );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_VoipServers].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( vs.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), vs.m_VoipServerID );
		BINDPARAM_TCHAR( arc.GetQuery(), vs.m_Description );
		arc.GetQuery().Execute( _T("SELECT VoipServerID FROM VoipServers ")
	                            _T("WHERE Description=?") );
		
		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( vs.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//vs.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), vs.m_VoipServerID );
			BINDPARAM_LONG( arc.GetQuery(), vs.m_VoipServerTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), vs.m_Description );
			BINDPARAM_TCHAR( arc.GetQuery(), vs.m_Url );
			BINDPARAM_LONG( arc.GetQuery(), vs.m_Port );
			BINDPARAM_BIT( arc.GetQuery(), vs.m_IsEnabled );
			BINDPARAM_BIT( arc.GetQuery(), vs.m_IsDefault );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT VoipServers ON INSERT INTO VoipServers ")
	               _T("(VoipServerID,VoipServerTypeID,Description,Url,Port,IsEnabled,IsDefault) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?) SET IDENTITY_INSERT VoipServers OFF") );						
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for VoipServers
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for VoipServerTypes
int StuffVoipServerTypeRecord( CArchiveFile& arc, long ID )
{
	TVoipServerTypes vst;
	int ret;
	vst.m_VoipServerTypeID = ID;

	// Query from the databse
	if( vst.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( vst.m_Description );				
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveVoipServerTypeRecord( CArchiveFile& arc, long ID )
{
	TVoipServerTypes vst;
	int ret;
	vst.m_VoipServerTypeID = ID;

	GETCHAR( vst.m_Description, 50 );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_VoipServerTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( vst.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), vst.m_VoipServerTypeID );
		BINDPARAM_TCHAR( arc.GetQuery(), vst.m_Description );
		arc.GetQuery().Execute( _T("SELECT VoipServerTypeID FROM VoipServerTypes ")
	                            _T("WHERE Description=?") );
		
		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( vst.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//vst.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), vst.m_VoipServerTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), vst.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT VoipServerTypes ON INSERT INTO VoipServerTypes ")
	               _T("(VoipServerTypeID,Description) ")
	               _T("VALUES")
	               _T("(?,?) SET IDENTITY_INSERT VoipServerTypes OFF") );						
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for VoipServerTypes
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for ApprovalObjectTypes
int StuffApprovalObjectTypeRecord( CArchiveFile& arc, long ID )
{
	TApprovalObjectTypes aot;
	int ret;
	aot.m_ApprovalObjectTypeID = ID;
	
	// Query from the databse
	if( aot.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( aot.m_Name );				
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveApprovalObjectTypeRecord( CArchiveFile& arc, long ID )
{
	TApprovalObjectTypes aot;
	int ret;
	aot.m_ApprovalObjectTypeID = ID;

	GETCHAR( aot.m_Name, 55 );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ApprovalObjectTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( aot.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), aot.m_ApprovalObjectTypeID );
		BINDPARAM_TCHAR( arc.GetQuery(), aot.m_Name );
		arc.GetQuery().Execute( _T("SELECT ApprovalObjectTypeID FROM ApprovalObjectTypes ")
	                            _T("WHERE Name=?") );
		
		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( aot.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//aot.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), aot.m_ApprovalObjectTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), aot.m_Name );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ApprovalObjectTypes ON INSERT INTO ApprovalObjectTypes ")
	               _T("(ApprovalObjectTypeID,Name) ")
	               _T("VALUES")
	               _T("(?,?) SET IDENTITY_INSERT ApprovalObjectTypes OFF") );						
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for ApprovalObjectTypes
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for Approvals
int StuffApprovalRecord( CArchiveFile& arc, long ID )
{
	TApprovals ap;
	int ret;
	ap.m_ApprovalID = ID;

	// Query from the databse
	if( ap.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( ap.m_RequestAgentID );
		PUTLONG( ap.m_ApproverAgentID );
		PUTLONG( ap.m_ApproverGroupID );
		PUTLONG( ap.m_ApprovalObjectTypeID );
		PUTLONG( ap.m_ActualID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveApprovalRecord( CArchiveFile& arc, long ID )
{
	TApprovals ap;
	int ret;
	ap.m_ApprovalID = ID;

	GETLONG( ap.m_RequestAgentID );
	GETLONG( ap.m_ApproverAgentID );
	GETLONG( ap.m_ApproverGroupID );
	GETLONG( ap.m_ApprovalObjectTypeID );
	GETLONG( ap.m_ActualID );
	
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Approvals].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ap.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), ap.m_ApprovalID );
		BINDPARAM_LONG( arc.GetQuery(), ap.m_ActualID );
		BINDPARAM_LONG( arc.GetQuery(), ap.m_ApprovalObjectTypeID );
		BINDPARAM_LONG( arc.GetQuery(), ap.m_RequestAgentID );
		arc.GetQuery().Execute( _T("SELECT ApprovalID FROM Approvals ")
								_T("WHERE ActualID=? AND ApprovalObjectTypeID=? AND RequestAgentID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ap.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ap.m_ApprovalID );
			BINDPARAM_LONG( arc.GetQuery(), ap.m_ActualID );
			BINDPARAM_LONG( arc.GetQuery(), ap.m_ApprovalObjectTypeID );
			BINDPARAM_LONG( arc.GetQuery(), ap.m_ApproverAgentID );
			BINDPARAM_LONG( arc.GetQuery(), ap.m_ApproverGroupID );
			BINDPARAM_LONG( arc.GetQuery(), ap.m_RequestAgentID );		
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Approvals ON INSERT INTO Approvals ")
						_T("(ApprovalID,ActualID,ApprovalObjectTypeID,ApproverAgentID,ApproverGroupID,RequestAgentID) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?) SET IDENTITY_INSERT Approvals OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for Approvals
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for Log
int StuffLogRecord( CArchiveFile& arc, long ID )
{
	TLog lo;
	int ret;
	lo.m_LogID = ID;

	// Query from the databse
	if( lo.Query( arc.GetQuery() ) == S_OK )
	{
		PUTDATE( lo.m_LogTime );
		PUTLONG( lo.m_ServerID );
		PUTLONG( lo.m_ErrorCode );
		PUTLONG( lo.m_LogSeverityID );
		PUTLONG( lo.m_LogEntryTypeID );
		PUTCHAR( lo.m_LogText );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveLogRecord( CArchiveFile& arc, long ID )
{
	TLog lo;
	int ret;
	lo.m_LogID = ID;

	GETDATE( lo.m_LogTime );
	GETLONG( lo.m_ServerID );
	GETLONG( lo.m_ErrorCode );
	GETLONG( lo.m_LogSeverityID );
	GETLONG( lo.m_LogEntryTypeID );
	GETCHAR( lo.m_LogText, 255 );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Log].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( lo.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), lo.m_ServerID );
		BINDPARAM_LONG( arc.GetQuery(), lo.m_ErrorCode );
		BINDPARAM_LONG( arc.GetQuery(), lo.m_LogSeverityID );
		BINDPARAM_TIME( arc.GetQuery(), lo.m_LogTime );
		BINDCOL_LONG( arc.GetQuery(), lo.m_LogID );
		arc.GetQuery().Execute( _T("SELECT LogID FROM Log ")
								_T("WHERE ServerID=? AND ErrorCode=? AND LogSeverityID=? AND LogTime=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( lo.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//sr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), lo.m_LogID );
			BINDPARAM_TIME( arc.GetQuery(), lo.m_LogTime );
			BINDPARAM_LONG( arc.GetQuery(), lo.m_ServerID );
			BINDPARAM_LONG( arc.GetQuery(), lo.m_ErrorCode );
			BINDPARAM_LONG( arc.GetQuery(), lo.m_LogSeverityID );
			BINDPARAM_LONG( arc.GetQuery(), lo.m_LogEntryTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), lo.m_LogText );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Log ON INSERT INTO Log ")
						_T("(LogID,LogTime,ServerID,ErrorCode,LogSeverityID,LogEntryTypeID,LogText) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?) SET IDENTITY_INSERT Log OFF") );
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for Log
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for Servers
int StuffServerRecord( CArchiveFile& arc, long ID )
{
	TServers sv;
	int ret;
	sv.m_ServerID = ID;

	// Query from the databse
	if( sv.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( sv.m_Description );
		PUTCHAR( sv.m_RegistrationKey );
		PUTLONG( sv.m_Status );
		PUTLONG( sv.m_ReloadConfig );
		PUTDATE( sv.m_CheckIn );
		PUTCHAR( sv.m_SpoolFolder );
		PUTCHAR( sv.m_TempFolder );
		PUTLONG( sv.m_WebStatus );
		PUTDATE( sv.m_WebCheckIn );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveServerRecord( CArchiveFile& arc, long ID )
{
	TServers sv;
	int ret;
	sv.m_ServerID = ID;

	GETCHAR( sv.m_Description,50 );
	GETCHAR( sv.m_RegistrationKey, 50 );
	GETLONG( sv.m_Status );
	GETLONG( sv.m_ReloadConfig );
	GETDATE( sv.m_CheckIn );
	GETCHAR( sv.m_SpoolFolder, 255 );
	GETCHAR( sv.m_TempFolder, 255 );
	GETLONG( sv.m_WebStatus );
	GETDATE( sv.m_WebCheckIn );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Servers].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( sv.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), sv.m_Description );
		BINDCOL_LONG( arc.GetQuery(), sv.m_ServerID );
		arc.GetQuery().Execute( _T("SELECT ServerID FROM Servers ")
								_T("WHERE Description=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( sv.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//sv.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), sv.m_ServerID );
			BINDPARAM_TCHAR( arc.GetQuery(), sv.m_Description );
			BINDPARAM_TCHAR( arc.GetQuery(), sv.m_RegistrationKey );
			BINDPARAM_LONG( arc.GetQuery(), sv.m_Status );
			BINDPARAM_LONG( arc.GetQuery(), sv.m_ReloadConfig );
			BINDPARAM_TIME( arc.GetQuery(), sv.m_CheckIn );
			BINDPARAM_TCHAR( arc.GetQuery(), sv.m_SpoolFolder );
			BINDPARAM_TCHAR( arc.GetQuery(), sv.m_TempFolder );
			BINDPARAM_LONG( arc.GetQuery(), sv.m_WebStatus );
			BINDPARAM_TIME( arc.GetQuery(), sv.m_WebCheckIn );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Servers ON INSERT INTO Servers ")
						_T("(ServerID,Description,RegistrationKey,Status,ReloadConfig,CheckIn,SpoolFolder,TempFolder,WebStatus,WebCheckIn) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT Servers OFF") );		
		
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for Servers
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for ServerTaskTypes
int StuffServerTaskTypeRecord( CArchiveFile& arc, long ID )
{
	TServerTaskTypes stt;
	int ret;
	stt.m_ServerTaskTypeID = ID;
	
	// Query from the databse
	if( stt.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( stt.m_Description );				
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveServerTaskTypeRecord( CArchiveFile& arc, long ID )
{
	TServerTaskTypes stt;
	int ret;
	stt.m_ServerTaskTypeID = ID;

	GETCHAR( stt.m_Description, 50 );	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ServerTaskTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( stt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), stt.m_ServerTaskTypeID );
		BINDPARAM_TCHAR( arc.GetQuery(), stt.m_Description );
		arc.GetQuery().Execute( _T("SELECT ServerTaskTypeID FROM ServerTaskTypes ")
	                            _T("WHERE Description=?") );
		
		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( stt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//stt.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), stt.m_ServerTaskTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), stt.m_Description );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ServerTaskTypes ON INSERT INTO ServerTaskTypes ")
	               _T("(ServerTaskTypeID,Description) ")
	               _T("VALUES")
	               _T("(?,?) SET IDENTITY_INSERT ServerTaskTypes OFF") );						
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for ServerTaskTypes
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for ServerTasks
int StuffServerTaskRecord( CArchiveFile& arc, long ID )
{
	TServerTasks st;
	int ret;
	st.m_ServerTaskID = ID;
	
	// Query from the databse
	if( st.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( st.m_ServerTaskTypeID  );				
		PUTLONG( st.m_ServerID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveServerTaskRecord( CArchiveFile& arc, long ID )
{
	TServerTasks st;
	int ret;
	st.m_ServerTaskID = ID;

	GETLONG( st.m_ServerTaskTypeID );	
	GETLONG( st.m_ServerID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_ServerTasks].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( st.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), st.m_ServerTaskID );
		BINDPARAM_LONG( arc.GetQuery(), st.m_ServerTaskTypeID );
		BINDPARAM_LONG( arc.GetQuery(), st.m_ServerID );		
		arc.GetQuery().Execute( _T("SELECT ServerTaskID FROM ServerTasks ")
	                            _T("WHERE ServerTaskTypeID=? AND ServerID=?") );
		
		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( st.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//st.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), st.m_ServerTaskID );
			BINDPARAM_LONG( arc.GetQuery(), st.m_ServerTaskTypeID );
			BINDPARAM_LONG( arc.GetQuery(), st.m_ServerID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT ServerTasks ON INSERT INTO ServerTasks ")
	               _T("(ServerTaskID,ServerTaskTypeID,ServerID) ")
	               _T("VALUES")
	               _T("(?,?,?) SET IDENTITY_INSERT ServerTasks OFF") );						
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for ServerTasks
//*****************************************************************************

////////////////////////////////////////////////////////////////////////////////
// 
// StuffNoteAttachmentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffNoteAttachmentRecord( CArchiveFile& arc, long ID )
{
	TNoteAttachments na;
	int ret;
	na.m_NoteAttachmentID = ID;

	// Query from the databse
	if( na.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( na.m_NoteID );
		PUTLONG( na.m_AttachmentID );
		PUTLONG( na.m_NoteTypeID );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveNoteAttachmentRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveNoteAttachmentRecord( CArchiveFile& arc, long ID )
{
	TNoteAttachments na;
	int ret = Arc_Success;
	
	na.m_NoteAttachmentID = ID;

	GETLONG( na.m_NoteID );
	GETLONG( na.m_NoteTypeID );
	GETLONG( na.m_AttachmentID );
	arc.TranslateID( TABLEID_Attachments, na.m_AttachmentID );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_NoteAttachments].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( na.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), na.m_NoteID );
		BINDPARAM_LONG( arc.GetQuery(), na.m_NoteTypeID );
		BINDPARAM_LONG( arc.GetQuery(), na.m_AttachmentID );
		BINDCOL_LONG( arc.GetQuery(), na.m_NoteAttachmentID );
		arc.GetQuery().Execute( _T("SELECT NoteAttachmentID FROM NoteAttachments ")
								_T("WHERE NoteID=? AND NoteTypeID=? AND AttachmentID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( na.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			na.Insert( arc.GetQuery() );
		}
	}
	
	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffAutoActionTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAutoActionTypeRecord( CArchiveFile& arc, long ID )
{
	TAutoActionTypes aat;
	int ret;
	aat.m_AutoActionTypeID = ID;

	// Query from the databse
	if( aat.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( aat.m_TypeName );
		PUTBYTE( aat.m_BuiltIn );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAutoActionTypeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAutoActionTypeRecord( CArchiveFile& arc, long ID )
{
	TAutoActionTypes aat;
	int ret = Arc_Success;

	aat.m_AutoActionTypeID = ID;

	GETCHAR( aat.m_TypeName, PERSONALDATATYPES_TYPENAME_LENGTH );
	GETBYTE( aat.m_BuiltIn );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AutoActionTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( aat.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), aat.m_TypeName );
		BINDCOL_LONG( arc.GetQuery(), aat.m_AutoActionTypeID );
		arc.GetQuery().Execute( _T("SELECT AutoActionTypeID FROM AutoActionTypes ")
								_T("WHERE TypeName=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( aat.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//pdt.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), aat.m_AutoActionTypeID );
			BINDPARAM_TCHAR( arc.GetQuery(), aat.m_TypeName );
			BINDPARAM_BIT( arc.GetQuery(), aat.m_BuiltIn );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AutoActionTypes ON INSERT INTO AutoActionTypes ")
						_T("(AutoActionTypeID,TypeName,BuiltIn) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT AutoActionTypes OFF") );
		}
	}
	
	arc.AddIDMapping( TABLEID_AutoActionTypes, ID, aat.m_AutoActionTypeID );

	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffAutoActionEventRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAutoActionEventRecord( CArchiveFile& arc, long ID )
{
	TAutoActionEvents aae;
	int ret;
	aae.m_AutoActionEventID = ID;

	// Query from the databse
	if( aae.Query( arc.GetQuery() ) == S_OK )
	{
		PUTCHAR( aae.m_EventName );
		PUTBYTE( aae.m_BuiltIn );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAutoActionEventRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAutoActionEventRecord( CArchiveFile& arc, long ID )
{
	TAutoActionEvents aae;
	int ret = Arc_Success;

	aae.m_AutoActionEventID = ID;

	GETCHAR( aae.m_EventName, PERSONALDATATYPES_TYPENAME_LENGTH );
	GETBYTE( aae.m_BuiltIn );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AutoActionTypes].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( aae.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), aae.m_EventName );
		BINDCOL_LONG( arc.GetQuery(), aae.m_AutoActionEventID );
		arc.GetQuery().Execute( _T("SELECT AutoActionEventID FROM AutoActionEvents ")
								_T("WHERE EventName=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( aae.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), aae.m_AutoActionEventID );
			BINDPARAM_TCHAR( arc.GetQuery(), aae.m_EventName );
			BINDPARAM_BIT( arc.GetQuery(), aae.m_BuiltIn );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AutoActionEvents ON INSERT INTO AutoActionEvents ")
						_T("(AutoActionEventID,TypeName,BuiltIn) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT AutoActionEvents OFF") );
		}
	}
	
	arc.AddIDMapping( TABLEID_AutoActionEvents, ID, aae.m_AutoActionEventID );

	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffAutoActionRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAutoActionRecord( CArchiveFile& arc, long ID )
{
	TAutoActions aa;
	int ret;
	aa.m_AutoActionID = ID;

	// Query from the databse
	if( aa.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( aa.m_TicketBoxID );
		PUTLONG( aa.m_AutoActionVal );
		PUTLONG( aa.m_AutoActionFreq );
		PUTLONG( aa.m_AutoActionEventID );
		PUTLONG( aa.m_AutoActionTypeID );
		PUTLONG( aa.m_AutoActionTargetID );
		PUTDATE( aa.m_DateCreated );		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAutoActionRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAutoActionRecord( CArchiveFile& arc, long ID )
{
	TAutoActions aa;
	int ret = Arc_Success;

	aa.m_AutoActionID = ID;

	GETLONG( aa.m_TicketBoxID );
	GETLONG( aa.m_AutoActionVal );
	GETLONG( aa.m_AutoActionFreq );
	GETLONG( aa.m_AutoActionEventID );
	GETLONG( aa.m_AutoActionTypeID );
	GETLONG( aa.m_AutoActionTargetID );
	GETDATE( aa.m_DateCreated );
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AutoActions].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( aa.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), aa.m_TicketBoxID );
		BINDPARAM_LONG( arc.GetQuery(), aa.m_AutoActionEventID );
		BINDPARAM_LONG( arc.GetQuery(), aa.m_AutoActionTypeID );
		BINDPARAM_LONG( arc.GetQuery(), aa.m_AutoActionTargetID );
		BINDCOL_LONG( arc.GetQuery(), aa.m_AutoActionID );
		arc.GetQuery().Execute( _T("SELECT AutoActionID FROM AutoActions ")
								_T("WHERE TicketBoxID=? AND AutoActionEventID=? AND AutoActionTypeID=? AND AutoActionTargetID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( aa.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//th.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AutoActionID );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_TicketBoxID );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AutoActionVal );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AutoActionFreq );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AutoActionEventID );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AutoActionTypeID );
			BINDPARAM_LONG( arc.GetQuery(), aa.m_AutoActionTargetID );
			BINDPARAM_TIME( arc.GetQuery(), aa.m_DateCreated );			
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AutoActions ON INSERT INTO AutoAction ")
						_T("(AutoActionID,TicketBoxID,AutoActionVal,AutoActionFreq,AutoActionEventID,AutoActionTypeID,AutoActionTargetID,DateCreated) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?) SET IDENTITY_INSERT AutoActions OFF") );			
		}
	}
	
	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffOfficeHourRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffOfficeHourRecord( CArchiveFile& arc, long ID )
{
	TOfficeHours oh;
	int ret;
	oh.m_OfficeHourID = ID;

	// Query from the databse
	if( oh.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( oh.m_TypeID );
		PUTLONG( oh.m_ActualID );
		PUTLONG( oh.m_StartHr );
		PUTLONG( oh.m_StartMin );
		PUTLONG( oh.m_StartAmPm );
		PUTLONG( oh.m_EndHr );
		PUTLONG( oh.m_EndMin );	
		PUTLONG( oh.m_EndAmPm );
		PUTLONG( oh.m_DayID );
		PUTCHAR( oh.m_Description );
		PUTDATE( oh.m_TimeStart );
		PUTDATE( oh.m_TimeEnd );
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveOfficeHourRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveOfficeHourRecord( CArchiveFile& arc, long ID )
{
	TOfficeHours oh;
	int ret = Arc_Success;

	oh.m_OfficeHourID = ID;

	GETLONG( oh.m_TypeID );
	GETLONG( oh.m_ActualID );
	GETLONG( oh.m_StartHr );
	GETLONG( oh.m_StartMin );
	GETLONG( oh.m_StartAmPm );
	GETLONG( oh.m_EndHr );
	GETLONG( oh.m_EndMin );	
	GETLONG( oh.m_EndAmPm );
	GETLONG( oh.m_DayID );
	GETCHAR(oh.m_Description, TICKETS_SUBJECT_LENGTH);
	GETDATE( oh.m_TimeStart );
	GETDATE( oh.m_TimeEnd );
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_OfficeHours].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( oh.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		if ( oh.m_TimeStartLen <= 0 || oh.m_TimeEndLen <= 0 )
		{
			BINDPARAM_LONG( arc.GetQuery(), oh.m_TypeID );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_ActualID );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_DayID );
			BINDCOL_LONG( arc.GetQuery(), oh.m_OfficeHourID );
			arc.GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
									_T("WHERE TypeID=? AND ActualID=? AND DayID=?") );
		}
		else
		{
			BINDPARAM_LONG( arc.GetQuery(), oh.m_TypeID );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_ActualID );
			BINDPARAM_TIME( arc.GetQuery(), oh.m_TimeStart );
			BINDPARAM_TIME( arc.GetQuery(), oh.m_TimeEnd );
			BINDCOL_LONG( arc.GetQuery(), oh.m_OfficeHourID );
			arc.GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
									_T("WHERE TypeID=? AND ActualID=? AND TimeStart=? AND TimeEnd=?") );
		}

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( oh.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//th.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), oh.m_OfficeHourID );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_TypeID );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_ActualID );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_StartHr );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_StartMin );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_StartAmPm );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_EndHr );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_EndMin );	
			BINDPARAM_LONG( arc.GetQuery(), oh.m_EndAmPm );
			BINDPARAM_LONG( arc.GetQuery(), oh.m_DayID );
			if ( oh.m_TimeStartLen <= 0 || oh.m_TimeEndLen <= 0 )
			{
				arc.GetQuery().Execute( _T("SET IDENTITY_INSERT OfficeHours ON INSERT INTO OfficeHours ")
						_T("(OfficeHourID,TypeID,ActualID,StartHr,StartMin,StartAmPm,EndHr,EndMin,EndAmPm,DayID) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT OfficeHours OFF") );
			}
			else
			{
				BINDPARAM_TCHAR( arc.GetQuery(), oh.m_Description );
				BINDPARAM_TIME( arc.GetQuery(), oh.m_TimeStart );
				BINDPARAM_TIME( arc.GetQuery(), oh.m_TimeEnd );
				arc.GetQuery().Execute( _T("SET IDENTITY_INSERT OfficeHours ON INSERT INTO OfficeHours ")
						_T("(OfficeHourID,TypeID,ActualID,StartHr,StartMin,StartAmPm,EndHr,EndMin,EndAmPm,DayID,Description,TimeStart,TimeEnd) ")
						_T("VALUES")
						_T("(?,?,?,?,?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT OfficeHours OFF") );
			}
		}
	}
	
	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffAgentContactRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAgentContactRecord( CArchiveFile& arc, long ID )
{
	TAgentContacts ac;
	int ret;
	ac.m_AgentContactID = ID;

	// Query from the databse
	if( ac.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( ac.m_AgentID );
		PUTLONG( ac.m_ContactID );		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAgentContactRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAgentContactRecord( CArchiveFile& arc, long ID )
{
	TAgentContacts ac;
	int ret = Arc_Success;

	ac.m_AgentContactID = ID;

	GETLONG( ac.m_AgentID );
	GETLONG( ac.m_ContactID );	
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AgentContacts].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ac.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), ac.m_AgentID );
		BINDPARAM_LONG( arc.GetQuery(), ac.m_ContactID );
		BINDCOL_LONG( arc.GetQuery(), ac.m_AgentContactID );
		arc.GetQuery().Execute( _T("SELECT AgentContactID FROM AgentContacts ")
								_T("WHERE AgentID=? AND ContactID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ac.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ac.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ac.m_AgentContactID );
			BINDPARAM_LONG( arc.GetQuery(), ac.m_AgentID );
			BINDPARAM_LONG( arc.GetQuery(), ac.m_ContactID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AgentContacts ON INSERT INTO AgentContacts ")
						_T("(AgentContactID,AgentID,ContactID) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT AgentContacts OFF") );			
		}
	}
	
	return ret;	
}

////////////////////////////////////////////////////////////////////////////////
// 
// StuffAgentRoutingRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffAgentRoutingRecord( CArchiveFile& arc, long ID )
{
	TAssignToAgents ar;
	int ret;
	ar.m_AgentRoutingID = ID;

	// Query from the databse
	if( ar.Query( arc.GetQuery() ) == S_OK )
	{
		PUTLONG( ar.m_RoutingRuleID );
		PUTLONG( ar.m_AgentID );		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// RetrieveAgentRoutingRecord
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveAgentRoutingRecord( CArchiveFile& arc, long ID )
{
	TAssignToAgents ar;
	int ret = Arc_Success;

	ar.m_AgentRoutingID = ID;

	GETLONG( ar.m_RoutingRuleID );	
	GETLONG( ar.m_AgentID );
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_AgentRouting].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ar.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), ar.m_RoutingRuleID );
		BINDPARAM_LONG( arc.GetQuery(), ar.m_AgentID );
		BINDCOL_LONG( arc.GetQuery(), ar.m_AgentRoutingID );
		arc.GetQuery().Execute( _T("SELECT AgentRoutingID FROM AgentRouting ")
								_T("WHERE RoutingRuleID=? AND AgentID=?") );

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ar.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//ar.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ar.m_AgentRoutingID );
			BINDPARAM_LONG( arc.GetQuery(), ar.m_RoutingRuleID );
			BINDPARAM_LONG( arc.GetQuery(), ar.m_AgentID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT AgentRouting ON INSERT INTO AgentRouting ")
						_T("(AgentRoutingID,RoutingRuleID,AgentID) ")
						_T("VALUES")
						_T("(?,?,?) SET IDENTITY_INSERT AgentRouting OFF") );			
		}
	}
	
	return ret;	
}

//*****************************************************************************
// Stuff and Retrieve for TicketLinks

int StuffTicketLinkRecord(CArchiveFile& arc, long ID)
{
	TTicketLinks tl;
	int ret;
	tl.m_TicketLinkID = ID;

	// Query from the databse
	if(tl.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(tl.m_LinkName);
		PUTDATE(tl.m_DateCreated);
		PUTLONG(tl.m_OwnerID);
		PUTLONG(tl.m_IsDeleted);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketLinkRecord(CArchiveFile& arc, long ID)
{
	TTicketLinks tl;
	int ret = Arc_Success;
	tl.m_TicketLinkID = ID;

	GETCHAR(tl.m_LinkName, LINKNAME_LENGTH);
	GETDATE(tl.m_DateCreated);
	GETLONG(tl.m_OwnerID);
	GETLONG(tl.m_IsDeleted);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketLinks].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (tl.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tl.m_LinkName );
		BINDCOL_LONG( arc.GetQuery(), tl.m_TicketLinkID );
		arc.GetQuery().Execute( _T("SELECT TicketLinkID FROM TicketLinks ")
								_T("WHERE LinkName=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (tl.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tl.m_TicketLinkID );
			BINDPARAM_TCHAR( arc.GetQuery(), tl.m_LinkName );
			BINDPARAM_TIME( arc.GetQuery(), tl.m_DateCreated );
			BINDPARAM_LONG( arc.GetQuery(), tl.m_OwnerID );
			BINDPARAM_LONG( arc.GetQuery(), tl.m_IsDeleted );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketLinks ON INSERT INTO TicketLinks ")
						            _T("(TicketLinkID,LinkName,DateCreated,OwnerID,IsDeleted) ")
						            _T("VALUES")
						            _T("(?,?,?,?,?) SET IDENTITY_INSERT TicketLinks OFF") );
		}
	}
	
	return ret;	
}

//*****************************************************************************
// Stuff and Retrieve for TicketLinksTicket

int StuffTicketLinksTicketRecord(CArchiveFile& arc, long ID)
{
	TTicketLinksTicket tl;
	int ret;
	tl.m_TicketLinksTicketID = ID;

	// Query from the databse
	if(tl.Query( arc.GetQuery()) == S_OK )
	{
		PUTLONG(tl.m_TicketID);
		PUTLONG(tl.m_TicketLinkID);
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTicketLinksTicketRecord(CArchiveFile& arc, long ID)
{
	TTicketLinksTicket tl;
	int ret = Arc_Success;
	tl.m_TicketLinksTicketID = ID;

	GETLONG(tl.m_TicketID);
	GETLONG(tl.m_TicketLinkID);

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TicketLinksTicket].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (tl.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), tl.m_TicketID );
		BINDPARAM_LONG( arc.GetQuery(), tl.m_TicketLinkID );
		BINDCOL_LONG( arc.GetQuery(), tl.m_TicketLinksTicketID );
		arc.GetQuery().Execute( _T("SELECT TicketLinksTicketID FROM TicketLinksTicket ")
								_T("WHERE TicketID=? AND TicketLinkID=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (tl.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tl.m_TicketLinksTicketID );
			BINDPARAM_LONG( arc.GetQuery(), tl.m_TicketID );
			BINDPARAM_LONG( arc.GetQuery(), tl.m_TicketLinkID );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TicketLinksTicket ON INSERT INTO TicketLinksTicket ")
						            _T("(TicketLinksTicketID,TicketID,TicketLinkID) ")
						            _T("VALUES")
						            _T("(?,?,?) SET IDENTITY_INSERT TicketLinksTicket OFF") );
		}
	}
	
	return ret;	
}

// End of Stuff and Retrieve for TicketLinksTicket
//*****************************************************************************

//*****************************************************************************
// Stuff and Retrieve for MessageTracking

int StuffMessageTrackingRecord(CArchiveFile& arc, long ID)
{
	TMessageTracking mt;
	int ret;
	mt.m_MessageTrackingID = ID;

	// Query from the databse
	if(mt.Query( arc.GetQuery()) == S_OK )
	{
		PUTLONG(mt.m_MessageID);
		PUTLONG(mt.m_MessageSourceID);
		PUTCHAR(mt.m_HeadMsgID);
		PUTCHAR(mt.m_HeadInReplyTo);
		PUTCHAR(mt.m_HeadReferences);		
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveMessageTrackingRecord(CArchiveFile& arc, long ID)
{
	TMessageTracking mt;
	int ret = Arc_Success;
	mt.m_MessageTrackingID = ID;

	GETLONG(mt.m_MessageID);
	GETLONG(mt.m_MessageSourceID);
	GETCHAR(mt.m_HeadMsgID, 255);
	GETCHAR(mt.m_HeadInReplyTo, 255);
	GETCHAR(mt.m_HeadReferences, 255);
	

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_MessageTracking].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (mt.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_LONG( arc.GetQuery(), mt.m_MessageID );
		BINDPARAM_LONG( arc.GetQuery(), mt.m_MessageSourceID );
		BINDCOL_LONG( arc.GetQuery(), mt.m_MessageTrackingID );
		arc.GetQuery().Execute( _T("SELECT MessageTrackingID FROM MessageTracking ")
								_T("WHERE MessageID=? AND MessageSourceID=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (mt.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), mt.m_MessageTrackingID );
			BINDPARAM_LONG( arc.GetQuery(), mt.m_MessageID );
			BINDPARAM_LONG( arc.GetQuery(), mt.m_MessageSourceID );
			BINDPARAM_TCHAR( arc.GetQuery(), mt.m_HeadMsgID );
			BINDPARAM_TCHAR( arc.GetQuery(), mt.m_HeadInReplyTo );
			BINDPARAM_TCHAR( arc.GetQuery(), mt.m_HeadReferences );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT MessageTracking ON INSERT INTO MessageTracking ")
						            _T("(MessageTrackingID,MessageID,MessageSourceID,HeadMsgID,HeadInReplyTo,HeadReferences) ")
						            _T("VALUES")
						            _T("(?,?,?,?,?,?) SET IDENTITY_INSERT MessageTracking OFF") );
		}
	}
	
	return ret;	
}

//*****************************************************************************
// Stuff and Retrieve for TimeZones

int StuffTimeZonesRecord(CArchiveFile& arc, long ID)
{
	TTimeZones tz;
	int ret;
	tz.m_TimeZoneID = ID;

	// Query from the databse
	if(tz.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(tz.m_DisplayName);
		PUTCHAR(tz.m_StandardName);
		PUTLONG(tz.m_UTCOffset);				
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveTimeZonesRecord(CArchiveFile& arc, long ID)
{
	TTimeZones tz;
	int ret = Arc_Success;
	tz.m_TimeZoneID = ID;

	GETCHAR(tz.m_DisplayName, 100);
	GETCHAR(tz.m_StandardName, 100);
	GETLONG(tz.m_UTCOffset);
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_TimeZones].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (tz.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), tz.m_DisplayName );
		BINDCOL_LONG( arc.GetQuery(), tz.m_TimeZoneID );
		arc.GetQuery().Execute( _T("SELECT TimeZoneID FROM TimeZones ")
								_T("WHERE DisplayName=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (tz.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), tz.m_TimeZoneID );
			BINDPARAM_TCHAR( arc.GetQuery(), tz.m_DisplayName );
			BINDPARAM_TCHAR( arc.GetQuery(), tz.m_StandardName );
			BINDPARAM_LONG( arc.GetQuery(), tz.m_UTCOffset );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT TimeZones ON INSERT INTO TimeZones ")
						            _T("(TimeZoneID,DisplayName,StandardName,UTCOffset) ")
						            _T("VALUES")
						            _T("(?,?,?,?) SET IDENTITY_INSERT TimeZones OFF") );
		}
	}
	
	return ret;	
}

//*****************************************************************************
// Stuff and Retrieve for Dictionary

int StuffDictionaryRecord(CArchiveFile& arc, long ID)
{
	TDictionary dc;
	int ret;
	dc.m_DictionaryID = ID;

	// Query from the databse
	if(dc.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(dc.m_LanguageID);
		PUTCHAR(dc.m_Description);
		PUTCHAR(dc.m_TlxFile);
		PUTCHAR(dc.m_ClxFile);
		PUTBYTE(dc.m_IsEnabled);				
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveDictionaryRecord(CArchiveFile& arc, long ID)
{
	TDictionary dc;
	int ret = Arc_Success;
	dc.m_DictionaryID = ID;

	GETCHAR(dc.m_LanguageID, 2);
	GETCHAR(dc.m_Description, 50);
	GETCHAR(dc.m_TlxFile, 20);
	GETCHAR(dc.m_ClxFile, 20);
	GETBYTE(dc.m_IsEnabled);
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Dictionary].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (dc.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), dc.m_Description );
		BINDCOL_LONG( arc.GetQuery(), dc.m_DictionaryID );
		arc.GetQuery().Execute( _T("SELECT DictionaryID FROM Dictionary ")
								_T("WHERE Description=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (dc.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), dc.m_DictionaryID );
			BINDPARAM_TCHAR( arc.GetQuery(), dc.m_LanguageID );
			BINDPARAM_TCHAR( arc.GetQuery(), dc.m_Description );
			BINDPARAM_TCHAR( arc.GetQuery(), dc.m_TlxFile );
			BINDPARAM_TCHAR( arc.GetQuery(), dc.m_ClxFile );
			BINDPARAM_BIT( arc.GetQuery(), dc.m_IsEnabled );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Dictionary ON INSERT INTO Dictionary ")
						            _T("(DictionaryID,LanguageID,Description,TlxFile,ClxFile,IsEnabled) ")
						            _T("VALUES")
						            _T("(?,?,?,?,?,?) SET IDENTITY_INSERT Dictionary OFF") );
		}
	}
	
	return ret;	
}

//*****************************************************************************
// Stuff and Retrieve for OAuthHosts

int StuffOAuthHostsRecord(CArchiveFile& arc, long ID)
{
	TOAuthHosts oh;
	int ret;
	oh.m_OAuthHostID = ID;

	// Query from the databse
	if(oh.Query( arc.GetQuery()) == S_OK )
	{
		PUTCHAR(oh.m_Description);
		PUTCHAR(oh.m_AuthEndPoint);
		PUTCHAR(oh.m_TokenEndPoint);
		PUTCHAR(oh.m_ClientID);
		PUTCHAR(oh.m_ClientSecret);
		PUTCHAR(oh.m_Scope);						
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveOAuthHostsRecord(CArchiveFile& arc, long ID)
{
	TOAuthHosts oh;
	int ret = Arc_Success;
	oh.m_OAuthHostID = ID;

	GETCHAR(oh.m_Description, OAUTH_LENGTH);
	GETCHAR(oh.m_AuthEndPoint, OAUTH_LENGTH);
	GETCHAR(oh.m_TokenEndPoint, OAUTH_LENGTH);
	GETCHAR(oh.m_ClientID, OAUTH_LENGTH);
	GETCHAR(oh.m_ClientSecret, OAUTH_LENGTH);
	GETCHAR(oh.m_Scope, OAUTH_SCOPE_LENGTH);
	
	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_OAuthHosts].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = (oh.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDPARAM_TCHAR( arc.GetQuery(), oh.m_Description );
		BINDCOL_LONG( arc.GetQuery(), oh.m_OAuthHostID );
		arc.GetQuery().Execute( _T("SELECT OAuthHostID FROM OAuthHosts ")
								_T("WHERE Description=?"));

		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = (oh.Update( arc.GetQuery()) == 1) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), oh.m_OAuthHostID );
			BINDPARAM_TCHAR( arc.GetQuery(), oh.m_Description );
			BINDPARAM_TCHAR( arc.GetQuery(), oh.m_AuthEndPoint );
			BINDPARAM_TCHAR( arc.GetQuery(), oh.m_TokenEndPoint );
			BINDPARAM_TCHAR( arc.GetQuery(), oh.m_ClientID );
			BINDPARAM_TCHAR( arc.GetQuery(), oh.m_ClientSecret );
			BINDPARAM_TCHAR( arc.GetQuery(), oh.m_Scope );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT OAuthHosts ON INSERT INTO OAuthHosts ")
						            _T("(OAuthHostID,Description,AuthEndPoint,TokenEndPoint,ClientID,ClientSecret,Scope) ")
						            _T("VALUES")
						            _T("(?,?,?,?,?,?,?) SET IDENTITY_INSERT OAuthHosts OFF") );
		}
	}
	
	return ret;	
}

//*****************************************************************************
// Stuff and Retrieve for Archives
int StuffArchiveRecord( CArchiveFile& arc, long ID )
{
	TArchives ar;
	int ret;
	ar.m_ArchiveID = ID;

	// Query from the databse
	if( ar.Query( arc.GetQuery() ) == S_OK )
	{
		PUTDATE( ar.m_DateCreated );
		PUTBYTE( ar.m_Purged );
		PUTCHAR( ar.m_ArcFilePath );
		PUTLONG( ar.m_InMsgRecords );
		PUTLONG( ar.m_InAttRecords );
		PUTLONG( ar.m_OutMsgRecords );
		PUTLONG( ar.m_OutAttRecords );	
	}
	else
	{
		return Arc_ErrIDNotFound;
	}

	return Arc_Success;	
}

int RetrieveArchiveRecord( CArchiveFile& arc, long ID )
{
	TArchives ar;
	int ret;
	ar.m_ArchiveID = ID;

	GETDATE( ar.m_DateCreated );
	GETBYTE( ar.m_Purged );
	GETCHAR( ar.m_ArcFilePath, 255 );
	GETLONG( ar.m_InMsgRecords );
	GETLONG( ar.m_InAttRecords );
	GETLONG( ar.m_OutMsgRecords );
	GETLONG( ar.m_OutAttRecords );

	// Is the ID reserved?
	if( ID <= g_ArcTblInfo[TABLEID_Archives].ReservedID ) 
	{ 
		if( arc.Overwrite() )
		{
			ret = ( ar.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
		}
	}
	else
	{
		// Check if record already exists
		arc.GetQuery().Initialize();
		BINDCOL_LONG( arc.GetQuery(), ar.m_ArchiveID );
		BINDPARAM_TIME( arc.GetQuery(), ar.m_DateCreated );
		BINDPARAM_LONG( arc.GetQuery(), ar.m_InMsgRecords );
		BINDPARAM_LONG( arc.GetQuery(), ar.m_OutMsgRecords );		
		arc.GetQuery().Execute( _T("SELECT ArchiveID FROM Archives ")
	                            _T("WHERE DateCreated=? AND InMsgRecords=? AND OutMsgRecords=?") );
		
		if( arc.GetQuery().Fetch() == S_OK )
		{
			// Record exists, we now have the new ID
			if( arc.Overwrite() )
			{
				ret = ( ar.Update( arc.GetQuery()) == 1 ) ? Arc_Success : Arc_ErrIDNotFound;
			}
		}
		else
		{
			//cr.Insert( arc.GetQuery() );
			arc.GetQuery().Initialize();
			BINDPARAM_LONG( arc.GetQuery(), ar.m_ArchiveID );
			BINDPARAM_TIME( arc.GetQuery(), ar.m_DateCreated );
			BINDPARAM_BIT( arc.GetQuery(), ar.m_Purged );
			BINDPARAM_TCHAR( arc.GetQuery(), ar.m_ArcFilePath );
			BINDPARAM_LONG( arc.GetQuery(), ar.m_InMsgRecords );
			BINDPARAM_LONG( arc.GetQuery(), ar.m_InAttRecords );
			BINDPARAM_LONG( arc.GetQuery(), ar.m_OutMsgRecords );
			BINDPARAM_LONG( arc.GetQuery(), ar.m_OutAttRecords );
			BINDPARAM_LONG( arc.GetQuery(), ar.m_NoteAttRecords );
			arc.GetQuery().Execute( _T("SET IDENTITY_INSERT Archives ON INSERT INTO Archives ")
	               _T("(ArchiveID,DateCreated,Purged,ArcFilePath,InMsgRecords,InAttRecords,OutMsgRecords,OutAttRecords,NoteAttRecords) ")
	               _T("VALUES")
	               _T("(?,?,?,?,?,?,?,?,?) SET IDENTITY_INSERT Archives OFF") );						
		}
	}
	
	return ret;	
}
// End of Stuff and Retrieve for Archives
//*****************************************************************************

////////////////////////////////////////////////////////////////////////////////
// 
// StuffRegistryRecord
// 
////////////////////////////////////////////////////////////////////////////////
int StuffRegistryRecord( CArchiveFile& arc, long ID )
{
	tstring sValue;
	int ret;

	// Put all registry keys and values in the archive

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_DATABASE_DSN_VALUE, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_DATABASE_TYPE_VALUE, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_LOG_PATH_VALUE, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_INSTALL_PATH_VALUE, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_REG_RESOURCE_PATH, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_ARCHIVE_PATH_VALUE, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_BACKUP_PATH_VALUE, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );
	
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_URL_SUBDIR_VALUE, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_VERSION_VALUE, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_DB_VERSION_VALUE, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, REG_KEY_AV, _T("QuarantineFolder"), sValue );	
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("OutboundAttachFolder"), sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("InboundAttachFolder"), sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("SpoolFolder"), sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_REPORT_PATH_VALUE, sValue );
	PUTCHAR( (LPTSTR) sValue.c_str() );

	return Arc_Success;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// Retrieve
// 
////////////////////////////////////////////////////////////////////////////////
int RetrieveRegistryRecord( CArchiveFile& arc, long ID )
{
	TCHAR szValue[256];
	int ret;

	if( arc.ReplaceConfig() )
	{
		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_DATABASE_DSN_VALUE, szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_DATABASE_TYPE_VALUE, szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_LOG_PATH_VALUE, szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_INSTALL_PATH_VALUE, szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_REG_RESOURCE_PATH, szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_ARCHIVE_PATH_VALUE, szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_BACKUP_PATH_VALUE, szValue );
		
		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_URL_SUBDIR_VALUE, szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_VERSION_VALUE, szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_DB_VERSION_VALUE, szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, REG_KEY_AV, _T("QuarantineFolder"), szValue );	

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("OutboundAttachFolder"), szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("InboundAttachFolder"), szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("SpoolFolder"), szValue );

		GETCHAR( szValue, 256 );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_REPORT_PATH_VALUE, szValue );

	}
	return Arc_Success;	
}


// The order these tables are backed up
int g_ArcTblOrder[TABLEID_COUNT] = 
{
	TABLEID_StyleSheets,
	TABLEID_ObjectTypes,
	TABLEID_LogEntryTypes,
	TABLEID_LogSeverity,
	TABLEID_PersonalDataTypes,
	TABLEID_AlertEvents,
	TABLEID_AlertMethods,
	TABLEID_ServerParameters,
	TABLEID_VirusScanStates,
	TABLEID_Registry,						
	TABLEID_TicketStates,
	TABLEID_Priorities,
	TABLEID_OutboundMessageStates,
	TABLEID_OutboundMessageTypes,
	TABLEID_MessageSourceTypes,
	TABLEID_StdResponseCategories,
	TABLEID_TicketBoxViewTypes,
	TABLEID_TicketActions,
	TABLEID_Objects,
	TABLEID_Agents,
	TABLEID_Groups,	
	TABLEID_Signatures,
	TABLEID_Contacts,
	TABLEID_PersonalData,
	TABLEID_StandardResponses,
	TABLEID_TicketBoxFooters,
	TABLEID_TicketBoxHeaders,
	TABLEID_TicketBoxes,
	TABLEID_Tickets,
	TABLEID_MessageDestinations,
	TABLEID_MessageSources,
	TABLEID_RoutingRules,
	TABLEID_OutboundMessages,
	TABLEID_InboundMessages,
	TABLEID_Attachments,
	TABLEID_ContactNotes,
	TABLEID_AccessControl,
	TABLEID_AgentGroupings,
	TABLEID_AlertMsgs,
	TABLEID_Alerts,
	TABLEID_ForwardCCAddresses,
	TABLEID_ForwardToAddresses,
	TABLEID_MatchFromAddresses,
	TABLEID_MatchText,
	TABLEID_MatchToAddresses,
	TABLEID_IPRanges,
	TABLEID_InboundMessageAttachments,
	TABLEID_InboundMessageQueue,
	TABLEID_OutboundMessageAttachments,
	TABLEID_OutboundMessageContacts,
	TABLEID_OutboundMessageQueue,
	TABLEID_StandardResponseUsage,
	TABLEID_StdResponseAttachments,
	TABLEID_StdResponseFavorites,
	TABLEID_TicketBoxViews,
	TABLEID_TicketContacts,
	TABLEID_TicketNotes,
	TABLEID_UIDLData,
	TABLEID_CustomDictionary,
	TABLEID_TicketHistory,
	TABLEID_InboundMessageRead,
	TABLEID_SRKeywordResults,
	TABLEID_TicketNotesRead,
	TABLEID_Folders,
	TABLEID_AgeAlerts,
	TABLEID_AgeAlertsSent,
	TABLEID_AutoMessages,
	TABLEID_AutoMessagesSent,
	TABLEID_AutoResponses,
	TABLEID_AutoResponsesSent,
	TABLEID_TicketBoxOwners,
	TABLEID_TicketBoxTicketBoxOwner,
	TABLEID_TicketBoxRouting,
	TABLEID_TicketCategories,
	TABLEID_WaterMarkAlerts,	
	TABLEID_ProcessingRules,
	TABLEID_MatchTextP,
	TABLEID_MatchFromAddressP,
	TABLEID_MatchToAddressP,
    TABLEID_ReportCustom,
	TABLEID_ReportObjects,
	TABLEID_ReportTypes,
	TABLEID_ReportStandard,
	TABLEID_ReportScheduled,
	TABLEID_ReportResultRows,
	TABLEID_ReportResults,
	TABLEID_TicketFields,
	TABLEID_TicketFieldsTicket,
	TABLEID_TicketFieldsTicketBox,
	TABLEID_AgentActivities,
	TABLEID_AgentActivityLog,
	TABLEID_AgentTicketCategories,
	TABLEID_TicketFieldViews,
	TABLEID_VoipDialingCodes,
	TABLEID_VoipExtensions,
	TABLEID_VoipServers,
	TABLEID_VoipServerTypes,
	TABLEID_ApprovalObjectTypes,
	TABLEID_Approvals,
	TABLEID_Log,
	TABLEID_Servers,
	TABLEID_ServerTaskTypes,
	TABLEID_ServerTasks,
	TABLEID_NoteAttachments,
	TABLEID_AutoActionTypes,
	TABLEID_AutoActionEvents,
	TABLEID_AutoActions,
	TABLEID_OfficeHours,
	TABLEID_AgentContacts,
	TABLEID_AgentRouting,
	TABLEID_TicketLinks,
	TABLEID_TicketLinksTicket,
	TABLEID_ContactGroups,
	TABLEID_ContactGrouping,
	TABLEID_TicketFieldTypes,
	TABLEID_TicketFieldOptions,
	TABLEID_MessageTracking,
	TABLEID_Bypass,	
	TABLEID_Email,
	TABLEID_TimeZones,
	TABLEID_Dictionary,
	TABLEID_Archives
};

// NOTE! The order must be the same as enum ArchiveTableIDs
ArchiveTableInfo g_ArcTblInfo[TABLEID_COUNT] = 
{
	{ StuffStyleSheetRecord, RetrieveStyleSheetRecord, _T("StyleSheets"), _T("StyleSheetID"), 0 },
	{ StuffObjectTypeRecord, RetrieveObjectTypeRecord, _T("ObjectTypes"), _T("ObjectTypeID"), EMS_OBJECTID_ADMINISTRATOR_AGENT },
	{ StuffLogEntryTypeRecord, RetrieveLogEntryTypeRecord, _T("LogEntryTypes"), _T("LogEntryTypeID"), EMS_LOG_VIRUS_SCANNING },
	{ StuffLogSeverityRecord, RetrieveLogSeverityRecord, _T("LogSeverity"), _T("LogSeverityID"), EMS_LOG_SEVERITY_CRITICAL_ERROR },
	{ StuffPersonalDataTypeRecord, RetrievePersonalDataTypeRecord, _T("PersonalDataTypes"), _T("PersonalDataTypeID"), 1 },
	{ StuffAlertEventRecord, RetrieveAlertEventRecord, _T("AlertEvents"), _T("AlertEventID"), EMS_ALERT_EVENT_CRITICAL_ERROR },
	{ StuffAlertMethodRecord, RetrieveAlertMethodRecord, _T("AlertMethods"), _T("AlertMethodID"), EMS_ALERT_METHOD_EXTERNAL_EMAIL },
	{ StuffServerParameterRecord, RetrieveServerParameterRecord, _T("ServerParameters"), _T("ServerParameterID"), EMS_SRVPARAM_AGENTVIEWS_READ_UPDATE },
	{ StuffVirusScanStateRecord, RetrieveVirusScanStateRecord, _T("VirusScanStates"), _T("VirusScanStateID"), EMS_VIRUS_SCAN_STATE_RESTORED },
	{ StuffRegistryRecord, RetrieveRegistryRecord, _T("Registry"), _T("NA"), 0 },
	{ StuffTicketStateRecord, RetrieveTicketStateRecord, _T("TicketStates"), _T("TicketStateID"), EMS_TICKETSTATEID_REOPENED },
	{ StuffPriorityRecord, RetrievePriorityRecord, _T("Priorities"), _T("PriorityID"), EMS_PRIORITY_LOWEST },
	{ StuffOutboundMessageStateRecord, RetrieveOutboundMessageStateRecord, _T("OutboundMessageStates"), _T("OutboundMsgStateID"), EMS_OUTBOUND_MESSAGE_STATE_INPROGRESS },
	{ StuffOutboundMessageTypeRecord, RetrieveOutboundMessageTypeRecord, _T("OutboundMessageTypes"), _T("OutboundMessageTypeID"), EMS_OUTBOUND_MESSAGE_TYPE_REPLY },
	{ StuffMessageSourceTypeRecord, RetrieveMessageSourceTypeRecord, _T("MessageSourceTypes"), _T("MessageSourceTypeID"), EMS_MESSAGE_SOURCE_TYPE_POP3 },
	{ StuffStdResponseCatRecord, RetrieveStdResponseCatRecord, _T("StdResponseCategories"), _T("StdResponseCatID"), EMS_STDRESPCAT_DRAFTS },
	{ StuffTicketBoxViewTypeRecord, RetrieveTicketBoxViewTypeRecord, _T("TicketBoxViewTypes"), _T("TicketBoxViewTypeID"), EMS_ALERTS },
	{ StuffObjectRecord, RetrieveObjectRecord, _T("Objects"), _T("ObjectID"), EMS_OBJECTID_ADMINISTRATOR_AGENT },
	{ StuffAgentRecord, RetrieveAgentRecord, _T("Agents"), _T("AgentID"), EMS_AGENTID_ADMINISTRATOR },
	{ StuffGroupRecord, RetrieveGroupRecord, _T("Groups"), _T("GroupID"), EMS_GROUPID_ADMINISTRATORS },
	{ StuffSignatureRecord, RetrieveSignatureRecord, _T("Signatures"), _T("SignatureID"), 0 },
	{ StuffContactRecord, RetrieveContactRecord, _T("Contacts"), _T("ContactID"), 0 },
	{ StuffPersonalDataRecord, RetrievePersonalDataRecord, _T("PersonalData"), _T("PersonalDataID"), 0 },
	{ StuffStandardResponseRecord, RetrieveStandardResponseRecord, _T("StandardResponses"), _T("StandardResponseID"), 0 },
	{ StuffTicketBoxFooterRecord, RetrieveTicketBoxFooterRecord, _T("TicketBoxFooters"), _T("FooterID"), 0 },
	{ StuffTicketBoxHeaderRecord, RetrieveTicketBoxHeaderRecord, _T("TicketBoxHeaders"), _T("HeaderID"), 0 },
	{ StuffTicketBoxRecord, RetrieveTicketBoxRecord, _T("TicketBoxes"), _T("TicketBoxID"), EMS_TICKETBOX_UNASSIGNED },
	{ StuffTicketRecord, RetrieveTicketRecord, _T("Tickets"), _T("TicketID"), 0 },
	{ StuffMessageDestinationRecord, RetrieveMessageDestinationRecord, _T("MessageDestinations"), _T("MessageDestinationID"), 0 },
	{ StuffMessageSourceRecord, RetrieveMessageSourceRecord, _T("MessageSources"), _T("MessageSourceID"), 0 },
	{ StuffRoutingRuleRecord, RetrieveRoutingRuleRecord, _T("RoutingRules"), _T("RoutingRuleID"), 0 },
	{ StuffOutboundMessageRecord, RetrieveOutboundMessageRecord, _T("OutboundMessages"), _T("OutboundMessageID"), 0 },
	{ StuffInboundMessageRecord, RetrieveInboundMessageRecord, _T("InboundMessages"), _T("InboundMessageID"), 0 },
	{ StuffContactNoteRecord, RetrieveContactNoteRecord, _T("ContactNotes"), _T("ContactNoteID"), 0 },
	{ StuffAccessControlRecord, RetrieveAccessControlRecord, _T("AccessControl"), _T("AccessControlID"), 0 },
	{ StuffAgentGroupingRecord, RetrieveAgentGroupingRecord, _T("AgentGroupings"), _T("AgentGroupingID"), 0 },
	{ StuffAlertMsgRecord, RetrieveAlertMsgRecord, _T("AlertMsgs"), _T("AlertMsgID"), 0 },
	{ StuffAlertRecord, RetrieveAlertRecord, _T("Alerts"), _T("AlertID"), 0 },
	{ StuffForwardCCAddressRecord, RetrieveForwardCCAddressRecord, _T("ForwardCCAddresses"), _T("AddressID"), 0 },
	{ StuffForwardToAddressRecord, RetrieveForwardToAddressRecord, _T("ForwardToAddresses"), _T("AddressID"), 0 },
	{ StuffMatchFromAddressRecord, RetrieveMatchFromAddressRecord, _T("MatchFromAddresses"), _T("MatchID"), 0 },
	{ StuffMatchTextRecord, RetrieveMatchTextRecord, _T("MatchText"), _T("MatchID"), 0 },
	{ StuffMatchToAddressRecord, RetrieveMatchToAddressRecord, _T("MatchToAddresses"), _T("MatchID"), 0 },
	{ StuffIPRangeRecord, RetrieveIPRangeRecord, _T("IPRanges"), _T("IPRangeID"), 0 },
	{ StuffInboundMessageAttachmentRecord, RetrieveInboundMessageAttachmentRecord, _T("InboundMessageAttachments"), _T("InboundMessageAttachmentID"), 0 },
	{ StuffInboundMessageQueueRecord, RetrieveInboundMessageQueueRecord, _T("InboundMessageQueue"), _T("InboundMessageQueueID"), 0 },
	{ StuffOutboundMessageAttachmentRecord, RetrieveOutboundMessageAttachmentRecord, _T("OutboundMessageAttachments"), _T("OutboundMessageAttachmentID"), 0 },
	{ StuffOutboundMessageContactRecord, RetrieveOutboundMessageContactRecord, _T("OutboundMessageContacts"), _T("OutboundMessageContactID"), 0 },
	{ StuffOutboundMessageQueueRecord, RetrieveOutboundMessageQueueRecord, _T("OutboundMessageQueue"), _T("OutboundMessageQueueID"), 0 },
	{ StuffStdRespUsageRecord, RetrieveStdRespUsageRecord, _T("StandardResponseUsage"), _T("StdResponseUsageID"), 0 },
	{ StuffStdResponseAttachmentRecord, RetrieveStdResponseAttachmentRecord, _T("StdResponseAttachments"), _T("StdResponseAttachID"), 0 },
	{ StuffStdResponseFavoriteRecord, RetrieveStdResponseFavoriteRecord, _T("StdResponseFavorites"), _T("StdResponseFavoritesID"), 0 },
	{ StuffTicketBoxViewRecord, RetrieveTicketBoxViewRecord, _T("TicketBoxViews"), _T("TicketBoxViewID"), 0 },
	{ StuffTicketContactRecord, RetrieveTicketContactRecord, _T("TicketContacts"), _T("TicketContactID"), 0 },
	{ StuffTicketNoteRecord, RetrieveTicketNoteRecord, _T("TicketNotes"), _T("TicketNoteID"), 0 },
	{ StuffUIDLDataRecord, RetrieveUIDLDataRecord, _T("UIDLData"), _T("UIDLID"), 0 },
	{ StuffCustomDictRecord, RetrieveCustomDictRecord, _T("CustomDictionary"), _T("CustomDictionaryID"), 0 },
	{ StuffTicketActionRecord, RetrieveTicketActionRecord, _T("TicketActions"), _T("TicketActionID"), EMS_TICKETACTIONID_ARC_MSG },
	{ StuffTicketHistoryRecord, RetrieveTicketHistoryRecord, _T("TicketHistory"), _T("TicketHistoryID"), 0 },
	{ StuffAttachmentRecord, RetrieveAttachmentRecord, _T("Attachments"), _T("AttachmentID"), 0 },
	{ StuffInboundMessageReadRecord, RetrieveInboundMessageReadRecord, _T("InboundMessageRead"), _T("InboundMessageReadID"), 0 },
	{ StuffSRKeywordResultsRecord, RetrieveSRKeywordResultsRecord, _T("SRKeywordResults"), _T("SRKeywordResultsID"), 0 },
	{ StuffTicketNotesReadRecord, RetrieveTicketNotesReadRecord, _T("TicketNotesRead"), _T("TicketNotesReadID"), 0 },
	{ StuffFolderRecord, RetrieveFolderRecord, _T("Folders"), _T("FolderID"), 0 },
	{ StuffAgeAlertRecord, RetrieveAgeAlertRecord, _T("AgeAlerts"), _T("AgeAlertID"), 0 },
	{ StuffAgeAlertSentRecord, RetrieveAgeAlertSentRecord, _T("AgeAlertsSent"), _T("AgeAlertSentID"), 0 },
	{ StuffAutoMessageRecord, RetrieveAutoMessageRecord, _T("AutoMessages"), _T("AutoMessageID"), 0 },
	{ StuffAutoMessageSentRecord, RetrieveAutoMessageSentRecord, _T("AutoMessagesSent"), _T("AutoMessageSentID"), 0 },
	{ StuffAutoResponseRecord, RetrieveAutoResponseRecord, _T("AutoResponses"), _T("AutoResponseID"), 0 },
	{ StuffAutoResponseSentRecord, RetrieveAutoResponseSentRecord, _T("AutoResponsesSent"), _T("AutoResponseSentID"), 0 },
	{ StuffTicketBoxOwnerRecord, RetrieveTicketBoxOwnerRecord, _T("TicketBoxOwners"), _T("TicketBoxOwnerID"), 0 },
	{ StuffTicketBoxTicketBoxOwnerRecord, RetrieveTicketBoxTicketBoxOwnerRecord, _T("TicketBoxTicketBoxOwner"), _T("TbTboID"), 0 },
	{ StuffTicketBoxRoutingRecord, RetrieveTicketBoxRoutingRecord, _T("TicketBoxRouting"), _T("TicketBoxRoutingID"), 0 },
	{ StuffTicketCategoryRecord, RetrieveTicketCategoryRecord, _T("TicketCategories"), _T("TicketCategoryID"), 0 },
	{ StuffWaterMarkAlertRecord, RetrieveWaterMarkAlertRecord, _T("WaterMarkAlerts"), _T("WaterMarkAlertID"), 0 },
	{ StuffProcessingRuleRecord, RetrieveProcessingRuleRecord, _T("ProcessingRules"), _T("ProcessingRuleID"), 0 },
	{ StuffMatchTextPRecord, RetrieveMatchTextPRecord, _T("MatchTextP"), _T("MatchID"), 0 },
	{ StuffMatchFromAddressPRecord, RetrieveMatchFromAddressPRecord, _T("MatchFromAddressP"), _T("MatchID"), 0 },
	{ StuffMatchToAddressPRecord, RetrieveMatchToAddressPRecord, _T("MatchToAddressP"), _T("MatchID"), 0 },
	{ StuffReportCustomRecord, RetrieveReportCustomRecord, _T("ReportCustom"), _T("CustomReportID"), 0 },
	{ StuffReportObjectRecord, RetrieveReportObjectRecord, _T("ReportObjects"), _T("ReportObjectID"), 0 },
	{ StuffReportTypeRecord, RetrieveReportTypeRecord, _T("ReportTypes"), _T("ReportTypeID"), 0 },
	{ StuffReportStandardRecord, RetrieveReportStandardRecord, _T("ReportStandard"), _T("StandardReportID"), 0 },
	{ StuffReportScheduledRecord, RetrieveReportScheduledRecord, _T("ReportScheduled"), _T("ScheduledReportID"), 0 },
	{ StuffReportResultRowRecord, RetrieveReportResultRowRecord, _T("ReportResultRows"), _T("ReportResultRowsID"), 0 },
	{ StuffReportResultRecord, RetrieveReportResultRecord, _T("ReportResults"), _T("ReportResultID"), 0 },
	{ StuffTicketFieldRecord, RetrieveTicketFieldRecord, _T("TicketFields"), _T("TicketFieldID"), 0 },
	{ StuffTicketFieldsTicketRecord, RetrieveTicketFieldsTicketRecord, _T("TicketFieldsTicket"), _T("TicketFieldsTicketID"), 0 },
	{ StuffTicketFieldsTicketBoxRecord, RetrieveTicketFieldsTicketBoxRecord, _T("TicketFieldsTicketBox"), _T("TicketFieldsTicketBoxID"), 0 },
	{ StuffAgentActivitiesRecord, RetrieveAgentActivitiesRecord, _T("AgentActivities"), _T("AgentActivityID"), 0 },
	{ StuffAgentActivityLogRecord, RetrieveAgentActivityLogRecord, _T("AgentActivityLog"), _T("AgentActivityID"), 0 },
	{ StuffAgentTicketCategoriesRecord, RetrieveAgentTicketCategoriesRecord, _T("AgentTicketCategories"), _T("AgentTicketCategoryID"), 0 },
	{ StuffTicketFieldViewRecord, RetrieveTicketFieldViewRecord, _T("TicketFieldViews"), _T("TicketFieldViewID"), 0 },
	{ StuffVoipDialingCodeRecord, RetrieveVoipDialingCodeRecord, _T("VoipDialingCodes"), _T("VoipDialingCodeID"), 0 },
	{ StuffVoipExtensionRecord, RetrieveVoipExtensionRecord, _T("VoipExtensions"), _T("VoipExtensionID"), 0 },
	{ StuffVoipServerRecord, RetrieveVoipServerRecord, _T("VoipServers"), _T("VoipServerID"), 0 },
	{ StuffVoipServerTypeRecord, RetrieveVoipServerTypeRecord, _T("VoipServerTypes"), _T("VoipServerTypeID"), 0 },
	{ StuffApprovalObjectTypeRecord, RetrieveApprovalObjectTypeRecord, _T("ApprovalObjectTypes"), _T("ApprovalObjectTypeID"), 0 },
	{ StuffApprovalRecord, RetrieveApprovalRecord, _T("Approvals"), _T("ApprovalID"), 0 },
	{ StuffLogRecord, RetrieveLogRecord, _T("Log"), _T("LogID"), 0 },
	{ StuffServerRecord, RetrieveServerRecord, _T("Servers"), _T("ServerID"), 0 },
	{ StuffServerTaskTypeRecord, RetrieveServerTaskTypeRecord, _T("ServerTaskTypes"), _T("ServerTaskTypeID"), 0 },
	{ StuffServerTaskRecord, RetrieveServerTaskRecord, _T("ServerTasks"), _T("ServerTaskID"), 0 },
	{ StuffNoteAttachmentRecord, RetrieveNoteAttachmentRecord, _T("NoteAttachments"), _T("NoteAttachmentID"), 0 },
	{ StuffAutoActionTypeRecord, RetrieveAutoActionTypeRecord, _T("AutoActionTypes"), _T("AutoActionTypeID"), 0 },
	{ StuffAutoActionEventRecord, RetrieveAutoActionEventRecord, _T("AutoActionEvents"), _T("AutoActionEventID"), 0 },
	{ StuffAutoActionRecord, RetrieveAutoActionRecord, _T("AutoActions"), _T("AutoActionID"), 0 },
	{ StuffOfficeHourRecord, RetrieveOfficeHourRecord, _T("OfficeHours"), _T("OfficeHourID"), 0 },
	{ StuffAgentContactRecord, RetrieveAgentContactRecord, _T("AgentContacts"), _T("AgentContactID"), 0 },
	{ StuffAgentRoutingRecord, RetrieveAgentRoutingRecord, _T("AgentRouting"), _T("AgentRoutingID"), 0 },
	{ StuffTicketLinkRecord, RetrieveTicketLinkRecord, _T("TicketLinks"), _T("TicketLinkID"), 0 },
	{ StuffTicketLinksTicketRecord, RetrieveTicketLinksTicketRecord, _T("TicketLinksTicket"), _T("TicketLinksTicketID"), 0 },
	{ StuffContactGroupRecord, RetrieveContactGroupRecord, _T("ContactGroups"), _T("ContactGroupID"), 0 },
	{ StuffContactGroupingRecord, RetrieveContactGroupingRecord, _T("ContactGrouping"), _T("ContactGroupingID"), 0 },
	{ StuffTicketFieldTypeRecord, RetrieveTicketFieldTypeRecord, _T("TicketFieldTypes"), _T("TicketFieldTypeID"), 0 },
	{ StuffTicketFieldOptionRecord, RetrieveTicketFieldOptionRecord, _T("TicketFieldOptions"), _T("TicketFieldOptionID"), 0 },
	{ StuffMessageTrackingRecord, RetrieveMessageTrackingRecord, _T("MessageTracking"), _T("MessageTrackingID"), 0 },
	{ StuffBypassRecord, RetrieveBypassRecord, _T("Bypass"), _T("BypassID"), 0 },
	{ StuffEmailRecord, RetrieveEmailRecord, _T("Email"), _T("EmailID"), 0 },
	{ StuffTimeZonesRecord, RetrieveTimeZonesRecord, _T("TimeZones"), _T("TimeZoneID"), 0 },
	{ StuffDictionaryRecord, RetrieveDictionaryRecord, _T("Dictionary"), _T("DictionaryID"), 0 },
	{ StuffOAuthHostsRecord, RetrieveOAuthHostsRecord, _T("OAuthHosts"), _T("OAuthHostID"), 0 },
	{ StuffArchiveRecord, RetrieveArchiveRecord, _T("Archives"), _T("ArchiveID"), 0 },
};
