////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMLIBRARY/dataclasses.h,v 1.2.2.3.2.1 2006/05/01 15:44:10 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// Data classes generated from database schema
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

#ifndef DATA_CLASSES_H
#define DATA_CLASSES_H

#include "EMSIDs.h"

#define EMS_TEXT_BUFF_LEN_SEED 32

/* struct AccessControl_t - 
		Controls access to various objects from specific agents and groups.
	 */
typedef struct AccessControl_t
{
	AccessControl_t() // default constructor
	{
		ZeroMemory( this, sizeof(AccessControl_t) );
	}
	int m_AccessControlID;	/* 
			Uniquely identifies the access control item.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AccessControlIDLen;

	int m_AgentID;	/* 
			AgentID access level applies to, or zero when GroupID defined.
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_GroupID;	/* 
			GroupID access level applies to, or zero when AgentID defined.
		 Default: 0 SQL Type: INTEGER */
	long m_GroupIDLen;

	unsigned char m_AccessLevel;	/* 
			The access level, 0 = no access, 1 = enumerate, 2 = read, 3 = edit, 
			and 4 = delete. Higher values imply all lower values. (e.g. level 4 
			can delete, edit and read, and enumerate)
		 Default: 0 SQL Type: TINYINT */
	long m_AccessLevelLen;

	int m_ObjectID;	/* 
			ObjectID access level applies to.
		 Default: 0 SQL Type: INTEGER */
	long m_ObjectIDLen;

} AccessControl_t;

inline bool operator==(const AccessControl_t& A,const AccessControl_t& B)
{ return A.m_AccessControlID == B.m_AccessControlID; }
inline bool operator!=(const AccessControl_t& A,const AccessControl_t& B)
{ return A.m_AccessControlID != B.m_AccessControlID; }

/* struct AgentGroupings_t - 
		Segments agents into logical groups.
	 */
typedef struct AgentGroupings_t
{
	AgentGroupings_t() // default constructor
	{
		ZeroMemory( this, sizeof(AgentGroupings_t) );
	}
	int m_AgentGroupingID;	/* 
			Uniquely identifies agent groupings.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgentGroupingIDLen;

	int m_AgentID;	/* 
			AgentID this applies to.
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_GroupID;	/* 
			Group of which this agent is a member.
		 Default: 0 SQL Type: INTEGER */
	long m_GroupIDLen;

} AgentGroupings_t;

inline bool operator==(const AgentGroupings_t& A,const AgentGroupings_t& B)
{ return A.m_AgentGroupingID == B.m_AgentGroupingID; }
inline bool operator!=(const AgentGroupings_t& A,const AgentGroupings_t& B)
{ return A.m_AgentGroupingID != B.m_AgentGroupingID; }

/* struct Agents_t - 
		Agents are users of the system. Agents can represent 
		any level of user (e.g. an agent, supervisor, etc.).
	 */
typedef struct Agents_t
{
	Agents_t() // default constructor
	{
		ZeroMemory( this, sizeof(Agents_t) );
		m_IsEnabled = 1;
		m_MaxReportRowsPerPage = 25;
		m_QuoteMsgInReply = 1;
		m_StyleSheetID = 1;
		m_UseIM = 1;
		m_NewMessageFormat = 0;
		m_ForceSpellCheck = 1;
		m_SignatureTopReply = 1;
		m_ShowMessagesInbound = 1;
		m_UsePreviewPane = 1;
		m_CloseTicket = 2;
		m_RouteToInbox = 2;
		m_MarkAsReadSeconds = 5;
		m_UseAutoFill = 0;
		m_DefaultTicketBoxID = 1;
		m_DefaultTicketDblClick = 0;
		m_ReadReceipt = 0;
		m_DeliveryConfirmation = 0;
		m_StatusID = 0;
		m_AutoStatusTypeID = 0;
		m_AutoStatusMin = 0;
		m_LoginStatusID = 1;
		m_LogoutStatusID = 5;
		m_TimeZoneID = 0;
		m_DictionaryID = 1;
		m_DictionaryPrompt = 0;
	}
	int m_AgentID;	/* 
			Uniquely identifies agents.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgentIDLen;

	TCHAR m_LoginName[AGENTS_LOGINNAME_LENGTH];	/* 
			Login name.
		 SQL Type: VARCHAR(50) */
	long m_LoginNameLen;

	TCHAR m_Password[AGENTS_PASSWORD_LENGTH];	/* 
			Password.
		 SQL Type: VARCHAR(50) */
	long m_PasswordLen;

	TCHAR m_Name[AGENTS_NAME_LENGTH];	/* 
			The agent's name in whatever format it may be.
		 SQL Type: VARCHAR(128) */
	long m_NameLen;

	unsigned char m_IsEnabled;	/* 
			Is agent account enabled? Disabled agents can't login.
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	int m_ObjectID;	/* 
			Security Object ID.
		 Default: 0 SQL Type: INTEGER */
	long m_ObjectIDLen;

	int m_DefaultAgentAddressID;	/* 
			The default agent contact data (could be of any contact type).
		 Default: 0 SQL Type: INTEGER */
	long m_DefaultAgentAddressIDLen;

	int m_MaxReportRowsPerPage;	/* 
			Maximum number of report rows per page.
		 Default: 25 SQL Type: INTEGER */
	long m_MaxReportRowsPerPageLen;

	unsigned char m_StdResponseApprovalsRequired;	/* 
			Are standard response approvals required for this agent?
		 Default: 0 SQL Type: BIT */
	long m_StdResponseApprovalsRequiredLen;

	unsigned char m_MessageApprovalsRequired;	/* 
			Do outbound messages composed by this user require approval?
		 Default: 0 SQL Type: BIT */
	long m_MessageApprovalsRequiredLen;

	unsigned char m_QuoteMsgInReply;	/* 
			Should replies quote the original message?
		 Default: 1 SQL Type: BIT */
	long m_QuoteMsgInReplyLen;

	TCHAR m_ReplyQuotedPrefix[AGENTS_REPLYQUOTEDPREFIX_LENGTH];	/* 
			The prefix used in quoted replies (NULL for none).
		 SQL Type: CHAR(2) */
	long m_ReplyQuotedPrefixLen;

	TCHAR m_NTDomain[AGENTS_NTDOMAIN_LENGTH];	/* 
			The NT domain this user belongs to (NULL if none).
		 SQL Type: VARCHAR(125) */
	long m_NTDomainLen;

	int m_IsNTUser;	/* 
			Should be treated (and authed) as NT domain user?
		 Default: 0 SQL Type: INTEGER */
	long m_IsNTUserLen;

	int m_StyleSheetID;	/* 
			ID of style sheet that enacts the user's current theme (NULL if default).
		 Default: 1 SQL Type: INTEGER */
	long m_StyleSheetIDLen;

	unsigned char m_IsDeleted;	/* 
			Indicates that the agent shouldn't be listed 
			and can be removed in an archive or purge operation.
		 Default: 0 SQL Type: BIT */
	long m_IsDeletedLen;

	unsigned char m_UseIM;	/* 
			Indicates whether the agent uses instant messaging.
		 Default: 1 SQL Type: BIT */
	long m_UseIMLen;

	unsigned char m_NewMessageFormat;	/* 
			Indicates the default new message format (text=0,html=1,default=0).
		 Default: 1 SQL Type: BIT */
	long m_NewMessageFormatLen;

	unsigned char m_UseAutoFill;	/* 
			Auto fill email addresses from AgentContacts?
		 Default: 1 SQL Type: BIT */
	long m_UseAutoFillLen;

	int m_EscalateToAgentID;	/* 
			Who does this agent escalalate tickets to? (0 = nobody)
		 Default: 0 SQL Type: INTEGER */
	long m_EscalateToAgentIDLen;

	int m_OutboundApprovalFromID;	/* 
			Who does this agent escalalate tickets to? (0 = nobody)
		 Default: 0 SQL Type: INTEGER */
	long m_OutboundApprovalFromIDLen;

	unsigned char m_RequireGetOldest;	/* 
			Indicates whether the agent is required to use the Get Oldest Button
		 Default: 0 SQL Type: BIT */
	long m_RequireGetOldestLen;

	unsigned char m_HasLoggedIn;	/* 
			Indicates whether the agent has ever logged in. The first
			time they login the preferences page is displayed.
		 Default: 0 SQL Type: BIT */
	long m_HasLoggedInLen;

	int m_DefaultSignatureID;	/* 
			ID of default signature
		 Default: 0 SQL Type: INTEGER */
	long m_DefaultSignatureIDLen;

	int m_DefaultTicketBoxID;	/* 
			ID of default ticketbox
		 Default: 1 SQL Type: INTEGER */
	long m_DefaultTicketBoxIDLen;

	int m_ReadReceipt;	/* 
			Outbound read receipt mode
		 Default: 0 SQL Type: INTEGER */
	long m_ReadReceiptLen;

	int m_DeliveryConfirmation;	/* 
			Outbound delivery confirmation mode
		 Default: 0 SQL Type: INTEGER */
	long m_DeliveryConfirmationLen;

	int m_DefaultTicketDblClick;	/* 
			Default Ticket DoubleClick Action
		 Default: 0 SQL Type: INTEGER */
	long m_DefaultTicketDblClickLen;

	unsigned char m_ForceSpellCheck;	/* 
			Force spellcheck before the agent sends a message?
		 Default: 1 SQL Type: BIT */
	long m_ForceSpellCheckLen;

	unsigned char m_SignatureTopReply;	/* 
			Should the signature be inserted before or after the quoted text when replying to a message.
		 Default: 1 SQL Type: BIT */
	long m_SignatureTopReplyLen;

	unsigned char m_ShowMessagesInbound;	/* 
			How to show TicketHistory pane by default.
		 Default: 1 SQL Type: BIT */
	long m_ShowMessagesInboundLen;

	unsigned char m_UsePreviewPane;	/* 
			Use preview pane?
		 Default: 1 SQL Type: BIT */
	long m_UsePreviewPaneLen;

	unsigned char m_CloseTicket;	/* 
			Default value for "Close ticket upon reply" option.
		 Default: 2 SQL Type: TINYINT */
	long m_CloseTicketLen;

	unsigned char m_RouteToInbox;	/* 
			Default value for "Route Replies to My Tickets" option.
		 Default: 2 SQL Type: TINYINT */
	long m_RouteToInboxLen;

	int m_OutboxHoldTime;	/* 
			How long should messages stay in outbox before being sent?
		 Default: 0 SQL Type: INTEGER */
	long m_OutboxHoldTimeLen;

	unsigned char m_UseMarkAsRead;
	long m_UseMarkAsReadLen;

	int m_MarkAsReadSeconds; // How long before marking a message as read. Default is 5.
	long m_MarkAsReadSecondsLen;

	int m_StatusID;	/* 
			The Agents present status.
		 Default: 0 SQL Type: INTEGER */
	long m_StatusIDLen;
	
	TCHAR m_StatusText[AGENTS_STATUS_TEXT_LENGTH];	/* 
			The Agents present status text.
		 SQL Type: VARCHAR(125) */
	long m_StatusTextLen;

	int m_AutoStatusTypeID;	/* 
			The Agents auto status type.
		 Default: 0 SQL Type: INTEGER */
	long m_AutoStatusTypeIDLen;
	
	int m_AutoStatusMin;	/* 
			The number of minutes of inactivity before auto status is set.
		 Default: 0 SQL Type: INTEGER */
	long m_AutoStatusMinLen;

	int m_LoginStatusID;	/* 
			The Agents status at login.
		 Default: 0 SQL Type: INTEGER */
	long m_LoginStatusIDLen;
	
	int m_LogoutStatusID;	/* 
			The Agents status at logout.
		 Default: 0 SQL Type: INTEGER */
	long m_LogoutStatusIDLen;

	TCHAR m_OnlineText[AGENTS_STATUS_TEXT_LENGTH];	/* 
			The Agents online status text.
		 SQL Type: VARCHAR(125) */
	long m_OnlineTextLen;

	TCHAR m_AwayText[AGENTS_STATUS_TEXT_LENGTH];	/* 
			The Agents away status text.
		 SQL Type: VARCHAR(125) */
	long m_AwayTextLen;

	TCHAR m_NotAvailText[AGENTS_STATUS_TEXT_LENGTH];	/* 
			The Agents not available status text.
		 SQL Type: VARCHAR(125) */
	long m_NotAvailTextLen;

	TCHAR m_DndText[AGENTS_STATUS_TEXT_LENGTH];	/* 
			The Agents do not disturb status text.
		 SQL Type: VARCHAR(125) */
	long m_DndTextLen;

	TCHAR m_OfflineText[AGENTS_STATUS_TEXT_LENGTH];	/* 
			The Agents offline status text.
		 SQL Type: VARCHAR(125) */
	long m_OfflineTextLen;

	TCHAR m_OooText[AGENTS_STATUS_TEXT_LENGTH];	/* 
			The Agents out of office status text.
		 SQL Type: VARCHAR(125) */
	long m_OooTextLen;

	int m_TimeZoneID;	/* 
			Default TimeZone
		 Default: 0 SQL Type: INTEGER */
	long m_TimeZoneIDLen;

	int m_DictionaryID;	/* 
			Default Dictionary
		 Default: 1 SQL Type: INTEGER */
	long m_DictionaryIDLen;

	unsigned char m_DictionaryPrompt;	/* 
			Prompt Agent to select SpellCheck Dictionary.
		 Default: 0 SQL Type: BIT */
	long m_DictionaryPromptLen;
	
} Agents_t;

inline bool operator==(const Agents_t& A,const Agents_t& B)
{ return A.m_AgentID == B.m_AgentID; }
inline bool operator!=(const Agents_t& A,const Agents_t& B)
{ return A.m_AgentID != B.m_AgentID; }

/* struct AlertEvents_t - 
		The list of events that trigger alerts.
	 */
typedef struct AlertEvents_t
{
	AlertEvents_t() // default constructor
	{
		ZeroMemory( this, sizeof(AlertEvents_t) );
	}
	int m_AlertEventID;	/* 
			Uniquely identifies an alert event.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AlertEventIDLen;

	TCHAR m_Description[ALERTEVENTS_DESCRIPTION_LENGTH];	/* 
			A human-readable description of an alert event.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	unsigned char m_NeedTicketBoxID;	/* 
			Indicates whether this event requires the context of a ticketboxid.
		 Default: 0 SQL Type: BIT */
	long m_NeedTicketBoxIDLen;

} AlertEvents_t;

inline bool operator==(const AlertEvents_t& A,const AlertEvents_t& B)
{ return A.m_AlertEventID == B.m_AlertEventID; }
inline bool operator!=(const AlertEvents_t& A,const AlertEvents_t& B)
{ return A.m_AlertEventID != B.m_AlertEventID; }

/* struct AlertMethods_t - 
		The list of methods used to send an alert.
	 */
typedef struct AlertMethods_t
{
	AlertMethods_t() // default constructor
	{
		ZeroMemory( this, sizeof(AlertMethods_t) );
	}
	int m_AlertMethodID;	/* 
			Uniquely identifies an alert method.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AlertMethodIDLen;

	TCHAR m_Description[ALERTMETHODS_DESCRIPTION_LENGTH];	/* 
			A human-readable description of an alert method.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

} AlertMethods_t;

inline bool operator==(const AlertMethods_t& A,const AlertMethods_t& B)
{ return A.m_AlertMethodID == B.m_AlertMethodID; }
inline bool operator!=(const AlertMethods_t& A,const AlertMethods_t& B)
{ return A.m_AlertMethodID != B.m_AlertMethodID; }

/* struct AlertMsgs_t - 
		Contains alerts messages.
	 */
typedef struct AlertMsgs_t
{
	AlertMsgs_t() // default constructor
	{
		ZeroMemory( this, sizeof(AlertMsgs_t) );
		m_DateCreatedLen = SQL_NULL_DATA;
		m_BodyAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Body = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
	}
	~AlertMsgs_t() // beware! nonvirtual destructor
	{
		if (m_BodyAllocated > 0) free( m_Body );
	}
	AlertMsgs_t( const AlertMsgs_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(AlertMsgs_t));
		if (m_BodyAllocated > 0)
		{
			m_Body = (TCHAR*)calloc( m_BodyAllocated, 1 );
			memcpy(m_Body,i.m_Body,m_BodyAllocated);
		}
	}
	AlertMsgs_t& operator=(const AlertMsgs_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_BodyAllocated > 0) free( m_Body );
			memcpy(this,&i,sizeof(AlertMsgs_t));
			if (m_BodyAllocated > 0)
			{
				m_Body = (TCHAR*)calloc( m_BodyAllocated, 1 );
				memcpy(m_Body,i.m_Body,m_BodyAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_AlertMsgID;	/* 
			Uniquely identifies an alert message.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AlertMsgIDLen;

	int m_AlertEventID;	/* 
			Identifies the alert event.
		 SQL Type: INTEGER */
	long m_AlertEventIDLen;

	int m_TicketID;	/* 
			Identifies the ticket associated with an alert.
		 SQL Type: INTEGER */
	long m_TicketIDLen;

	int m_AgentID;	/* 
			Identifies the alert recipient.
		 SQL Type: INTEGER */
	long m_AgentIDLen;

	unsigned char m_Viewed;	/* 
			Has the agent seen this alert?
		 Default: 0 SQL Type: TINYINT */
	long m_ViewedLen;

	TCHAR m_Subject[ALERTMSGS_SUBJECT_LENGTH];	/* 
			Optional alert subject.
		 SQL Type: VARCHAR(255) */
	long m_SubjectLen;

	TCHAR m_AlertName[ALERTMSGS_SUBJECT_LENGTH];	/* 
			Optional alert subject.
		 SQL Type: VARCHAR(255) */
	long m_AlertNameLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			Date/time the alert message was sent.
		 SQL Type: DATETIME */
	long m_DateCreatedLen;

	TCHAR* m_Body;	/* 
			Arbritrary text provided by the sender of the alert
		 SQL Type: TEXT */
	long m_BodyAllocated;
 	long m_BodyLen;

} AlertMsgs_t;

inline bool operator==(const AlertMsgs_t& A,const AlertMsgs_t& B)
{ return A.m_AlertMsgID == B.m_AlertMsgID; }
inline bool operator!=(const AlertMsgs_t& A,const AlertMsgs_t& B)
{ return A.m_AlertMsgID != B.m_AlertMsgID; }

/* struct Alerts_t - 
		Active alert definitions.
	 */
typedef struct Alerts_t
{
	Alerts_t() // default constructor
	{
		ZeroMemory( this, sizeof(Alerts_t) );
	}
	int m_AlertID;	/* 
			Uniquely identifies an alert.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AlertIDLen;

	int m_AlertEventID;	/* 
			The event that triggers the alert.
		 Default: 0 SQL Type: INTEGER */
	long m_AlertEventIDLen;

	int m_AlertMethodID;	/* 
			How to send the alert.
		 Default: 0 SQL Type: INTEGER */
	long m_AlertMethodIDLen;

	int m_AgentID;	/* 
			The Agent to send the alert to. (mutually exclusive with GroupID)
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_GroupID;	/* 
			The Group to send the alert to. (mutually exclusive with AgentID)
		 Default: 0 SQL Type: INTEGER */
	long m_GroupIDLen;

	int m_TicketBoxID;	/* 
			The ID of the ticketbox related to the event (if any)
		 Default: 0 SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	TCHAR m_EmailAddress[ALERTS_EMAILADDRESS_LENGTH];	/* 
			The email address for the external email alert method.
		 SQL Type: VARCHAR(255) */
	long m_EmailAddressLen;

	TCHAR m_FromEmailAddress[ALERTS_FROMEMAILADDRESS_LENGTH];	/* 
			The FROM: email address for the external email alert method.
		 SQL Type: VARCHAR(255) */
	long m_FromEmailAddressLen;

} Alerts_t;

inline bool operator==(const Alerts_t& A,const Alerts_t& B)
{ return A.m_AlertID == B.m_AlertID; }
inline bool operator!=(const Alerts_t& A,const Alerts_t& B)
{ return A.m_AlertID != B.m_AlertID; }

/* struct ApprovalObjectTypes_t - 
		This table identifies the various types of objects that can be channeled 
		through the approval process. Examples include:
		Messages, Standard responses, etc.
	 */
typedef struct ApprovalObjectTypes_t
{
	ApprovalObjectTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(ApprovalObjectTypes_t) );
	}
	int m_ApprovalObjectTypeID;	/* 
			Uniquely identifies the approval object type.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ApprovalObjectTypeIDLen;

	TCHAR m_Name[APPROVALOBJECTTYPES_NAME_LENGTH];	/* 
			The name of the approval object type (e.g. message or std response) 
		 SQL Type: VARCHAR(55) */
	long m_NameLen;

} ApprovalObjectTypes_t;

inline bool operator==(const ApprovalObjectTypes_t& A,const ApprovalObjectTypes_t& B)
{ return A.m_ApprovalObjectTypeID == B.m_ApprovalObjectTypeID; }
inline bool operator!=(const ApprovalObjectTypes_t& A,const ApprovalObjectTypes_t& B)
{ return A.m_ApprovalObjectTypeID != B.m_ApprovalObjectTypeID; }

/* struct Approvals_t - 
		In scenarios where agents require approvals for specifc actions (e.g. sending a message), 
		this table determine who is responsible for approving the action (either a user/agent or group).
	 */
typedef struct Approvals_t
{
	Approvals_t() // default constructor
	{
		ZeroMemory( this, sizeof(Approvals_t) );
	}
	int m_ApprovalID;	/* 
			Uniquely identifies an approval entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ApprovalIDLen;

	int m_RequestAgentID;	/* 
			Agent requesting approval.
		 Default: 0 SQL Type: INTEGER */
	long m_RequestAgentIDLen;

	int m_ApproverAgentID;	/* 
			Approver agent ID (when approver is a single agent).
		 Default: 0 SQL Type: INTEGER */
	long m_ApproverAgentIDLen;

	int m_ApproverGroupID;	/* 
			Approver group ID (when approver can be anyone from a group).
		 Default: 0 SQL Type: INTEGER */
	long m_ApproverGroupIDLen;

	int m_ApprovalObjectTypeID;	/* 
			Defines the type of action that needs approving.
		 Default: 0 SQL Type: INTEGER */
	long m_ApprovalObjectTypeIDLen;

	int m_ActualID;	/* 
			Uniquely identifies the approval object type.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ActualIDLen;

} Approvals_t;

inline bool operator==(const Approvals_t& A,const Approvals_t& B)
{ return A.m_ApprovalID == B.m_ApprovalID; }
inline bool operator!=(const Approvals_t& A,const Approvals_t& B)
{ return A.m_ApprovalID != B.m_ApprovalID; }

/* struct Archives_t - 
		Used to track archive/purge operations.
	 */
typedef struct Archives_t
{
	Archives_t() // default constructor
	{
		ZeroMemory( this, sizeof(Archives_t) );
		m_DateCreatedLen = SQL_NULL_DATA;
	}
	int m_ArchiveID;	/* 
			Uniquely identifies an archive.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ArchiveIDLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			Email date/time the archive was created.
		 SQL Type: DATETIME */
	long m_DateCreatedLen;

	unsigned char m_Purged;	/* 
			1 if this was a purge operation, otherwise 0 for archive.
		 Default: 0 SQL Type: TINYINT */
	long m_PurgedLen;

	TCHAR m_ArcFilePath[ARCHIVES_ARCFILEPATH_LENGTH];	/* 
			Location of the archive file.
		 SQL Type: VARCHAR(255) */
	long m_ArcFilePathLen;

	int m_InMsgRecords;	/* 
			Number of inbound message records in this archive.
		 Default: 0 SQL Type: INTEGER */
	long m_InMsgRecordsLen;

	int m_InAttRecords;	/* 
			Number of inbound attachment records in this archive.
		 Default: 0 SQL Type: INTEGER */
	long m_InAttRecordsLen;

	int m_OutMsgRecords;	/* 
			Number of outbound message records in this archive.
		 Default: 0 SQL Type: INTEGER */
	long m_OutMsgRecordsLen;

	int m_OutAttRecords;	/* 
			Number of outbound attachment records in this archive.
		 Default: 0 SQL Type: INTEGER */
	long m_OutAttRecordsLen;

	int m_NoteAttRecords;	/* 
			Number of TicketNote attachment records in this archive.
		 Default: 0 SQL Type: INTEGER */
	long m_NoteAttRecordsLen;

} Archives_t;

inline bool operator==(const Archives_t& A,const Archives_t& B)
{ return A.m_ArchiveID == B.m_ArchiveID; }
inline bool operator!=(const Archives_t& A,const Archives_t& B)
{ return A.m_ArchiveID != B.m_ArchiveID; }

/* struct Attachments_t - 
		Attachments
	 */
typedef struct Attachments_t
{
	Attachments_t() // default constructor
	{
		ZeroMemory( this, sizeof(Attachments_t) );
		m_VirusScanStateID = 1;
		/*m_AttachLocAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_AttachLoc = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_AttachNameAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_AttachName = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );*/
	}
	//~Attachments_t() // beware! nonvirtual destructor
	//{
	//	if (m_AttachLocAllocated > 0) free( m_AttachLoc );
	//	if (m_AttachNameAllocated > 0) free( m_AttachName );
	//}
	//Attachments_t( const Attachments_t& i ) // copy constructor
	//{
	//	memcpy(this,&i,sizeof(Attachments_t));
	//	if (m_AttachLocAllocated > 0)
	//	{
	//		m_AttachLoc = (TCHAR*)calloc( m_AttachLocAllocated, 1 );
	//		memcpy(m_AttachLoc,i.m_AttachLoc,m_AttachLocAllocated);
	//	}
	//	if (m_AttachNameAllocated > 0)
	//	{
	//		m_AttachName = (TCHAR*)calloc( m_AttachNameAllocated, 1 );
	//		memcpy(m_AttachName,i.m_AttachName,m_AttachNameAllocated);
	//	}
	//}
	//Attachments_t& operator=(const Attachments_t& i) // assignment operator
	//{
	//	if (this != &i) // trap for assignments to self
	//	{
	//		if (m_AttachLocAllocated > 0) free( m_AttachLoc );
	//		if (m_AttachNameAllocated > 0) free( m_AttachName );
	//		memcpy(this,&i,sizeof(Attachments_t));
	//		if (m_AttachLocAllocated > 0)
	//		{
	//			m_AttachLoc = (TCHAR*)calloc( m_AttachLocAllocated, 1 );
	//			memcpy(m_AttachLoc,i.m_AttachLoc,m_AttachLocAllocated);
	//		}
	//		if (m_AttachNameAllocated > 0)
	//		{
	//			m_AttachName = (TCHAR*)calloc( m_AttachNameAllocated, 1 );
	//			memcpy(m_AttachName,i.m_AttachName,m_AttachNameAllocated);
	//		}
	//	}
	//	return *this; // return reference to self
	//}
	
	int m_AttachmentID;	/* 
			Uniquely identifies the attachment
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AttachmentIDLen;

	TCHAR m_AttachmentLocation[ATTACHMENTS_ATTACHMENTLOCATION_LENGTH];	/* 
			Location of the attachment file on the server.
		 SQL Type: VARCHAR(255) */
	long m_AttachmentLocationLen;

	TCHAR m_MediaType[ATTACHMENTS_MEDIATYPE_LENGTH];	/* 
			MIME content type of attachment.
		 SQL Type: VARCHAR(125) */
	long m_MediaTypeLen;

	TCHAR m_MediaSubType[ATTACHMENTS_MEDIASUBTYPE_LENGTH];	/* 
			MIME content sub-type of attachment.
		 SQL Type: VARCHAR(125) */
	long m_MediaSubTypeLen;

	TCHAR m_ContentDisposition[ATTACHMENTS_CONTENTDISPOSITION_LENGTH];	/* 
			MIME content-disposition; will typically be either
			"inline" or "attachment"
		 SQL Type: VARCHAR(125) */
	long m_ContentDispositionLen;

	TCHAR m_FileName[ATTACHMENTS_FILENAME_LENGTH];	/* 
			Name of the attachment file on the server. 
		 SQL Type: VARCHAR(255) */
	long m_FileNameLen;

	int m_FileSize;	/* 
			Size of the attachment file on the server. 
		 Default: 0 SQL Type: INTEGER */
	long m_FileSizeLen;

	int m_VirusScanStateID;	/* 
			The virus scan state of the attachment.
		 Default: 1 SQL Type: INTEGER */
	long m_VirusScanStateIDLen;

	TCHAR m_VirusName[ATTACHMENTS_VIRUSNAME_LENGTH];	/* 
			Name of the virus infecting this attachment file, if any.
		 SQL Type: VARCHAR(125) */
	long m_VirusNameLen;

	TCHAR m_ContentID[ATTACHMENTS_CONTENTID_LENGTH];	/* 
			Attachment Content ID. 
		 SQL Type: VARCHAR(255) */
	long m_ContentIDLen;

	unsigned char m_IsInbound;	/* 
			Is the attachment in the inbound or outbound attachment folder?
		 Default: 0 SQL Type: BIT */
	long m_IsInboundLen;

	TCHAR* m_AttachLoc;	
	long m_AttachLocAllocated;
 	long m_AttachLocLen;

	TCHAR* m_AttachName;	
	long m_AttachNameAllocated;
 	long m_AttachNameLen;

} Attachments_t;

inline bool operator==(const Attachments_t& A,const Attachments_t& B)
{ return A.m_AttachmentID == B.m_AttachmentID; }
inline bool operator!=(const Attachments_t& A,const Attachments_t& B)
{ return A.m_AttachmentID != B.m_AttachmentID; }

/* struct ContactNotes_t - 
		Contact notes will be used in a similar capactiy as TicketNotes, 
		both by users for collaboration purposes and by the system directly. 
		The system may, for example, create a contact note when the contact is merged with another.
	 */
typedef struct ContactNotes_t
{
	ContactNotes_t() // default constructor
	{
		ZeroMemory( this, sizeof(ContactNotes_t) );
		m_DateCreatedLen = SQL_NULL_DATA;
		m_NoteAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Note = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
	}
	~ContactNotes_t() // beware! nonvirtual destructor
	{
		if (m_NoteAllocated > 0) free( m_Note );
	}
	ContactNotes_t( const ContactNotes_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(ContactNotes_t));
		if (m_NoteAllocated > 0)
		{
			m_Note = (TCHAR*)calloc( m_NoteAllocated, 1 );
			memcpy(m_Note,i.m_Note,m_NoteAllocated);
		}
	}
	ContactNotes_t& operator=(const ContactNotes_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_NoteAllocated > 0) free( m_Note );
			memcpy(this,&i,sizeof(ContactNotes_t));
			if (m_NoteAllocated > 0)
			{
				m_Note = (TCHAR*)calloc( m_NoteAllocated, 1 );
				memcpy(m_Note,i.m_Note,m_NoteAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_ContactNoteID;	/* 
			Uniquely identifes the contact note.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ContactNoteIDLen;

	int m_NumAttach;	
			
	long m_NumAttachLen;

	int m_ContactID;	/* 
			Contact to which the note relates.			
		 Default: 0 SQL Type: INTEGER */
	long m_ContactIDLen;

	int m_AgentID;	/* 
			ID of agent making the note (NULL for system-generated).
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			Date/time contact note was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	TCHAR* m_Note;	/* 
			The body of the note.
		 SQL Type: TEXT */
	long m_NoteAllocated;
 	long m_NoteLen;

	unsigned char m_IsVoipNote;	/* 
			Is this a VOIP note?
		 Default: 0 SQL Type: BIT */
	long m_IsVoipNoteLen;

	TIMESTAMP_STRUCT m_StartTime;	/* 
			Date/time the VOIP call was started.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_StartTimeLen;

	TIMESTAMP_STRUCT m_StopTime;	/* 
			Date/time the VOIP call was stopped.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_StopTimeLen;

	TCHAR m_ElapsedTime[TICKETBOXES_NAME_LENGTH];	/* 
			Short name of ticket box.
		 SQL Type: VARCHAR(50) */
	long m_ElapsedTimeLen;

	TCHAR m_Subject[TICKETBOXES_NAME_LENGTH];	/* 
			Subject of the note.
		 SQL Type: VARCHAR(50) */
	long m_SubjectLen;

} ContactNotes_t;

inline bool operator==(const ContactNotes_t& A,const ContactNotes_t& B)
{ return A.m_ContactNoteID == B.m_ContactNoteID; }
inline bool operator!=(const ContactNotes_t& A,const ContactNotes_t& B)
{ return A.m_ContactNoteID != B.m_ContactNoteID; }

/* struct Contacts_t - 
		Contacts are customers or external entities.
	 */
typedef struct Contacts_t
{
	Contacts_t() // default constructor
	{
		ZeroMemory( this, sizeof(Contacts_t) );
		m_DateCreatedLen = SQL_NULL_DATA;
		m_DeletedTimeLen = SQL_NULL_DATA;
	}
	int m_ContactID;	/* 
			Uniquely identifies contacts.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ContactIDLen;

	TCHAR m_Name[CONTACTS_NAME_LENGTH];	/* 
			Name (however it's entered).
		 SQL Type: VARCHAR(255) */
	long m_NameLen;

	TCHAR m_CompanyName[CONTACTS_COMPANYNAME_LENGTH];	/* 
			Company name.
		 SQL Type: VARCHAR(50) */
	long m_CompanyNameLen;

	TCHAR m_Title[CONTACTS_TITLE_LENGTH];	/* 
			Title.
		 SQL Type: VARCHAR(20) */
	long m_TitleLen;

	TCHAR m_WebPageAddress[CONTACTS_WEBPAGEADDRESS_LENGTH];	/* 
			Web page address (e.g. http://...).
		 SQL Type: VARCHAR(255) */
	long m_WebPageAddressLen;

	TCHAR m_StreetAddress1[CONTACTS_STREETADDRESS1_LENGTH];	/* 
			Street address 1.
		 SQL Type: VARCHAR(75) */
	long m_StreetAddress1Len;

	TCHAR m_StreetAddress2[CONTACTS_STREETADDRESS2_LENGTH];	/* 
			Street address 2.
		 SQL Type: VARCHAR(75) */
	long m_StreetAddress2Len;

	TCHAR m_City[CONTACTS_CITY_LENGTH];	/* 
			City.
		 SQL Type: VARCHAR(35) */
	long m_CityLen;

	TCHAR m_State[CONTACTS_STATE_LENGTH];	/* 
			State.
		 SQL Type: VARCHAR(30) */
	long m_StateLen;

	TCHAR m_ZipCode[CONTACTS_ZIPCODE_LENGTH];	/* 
			Postal code.
		 SQL Type: VARCHAR(20) */
	long m_ZipCodeLen;

	TCHAR m_Country[CONTACTS_COUNTRY_LENGTH];	/* 
			Country.
		 SQL Type: VARCHAR(55) */
	long m_CountryLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			Date contact was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	unsigned char m_IsDeleted;	/* 
			Indicates that the contact shouldn't be listed and can be removed in an archive or purge operation.
		 Default: 0 SQL Type: TINYINT */
	long m_IsDeletedLen;

	TIMESTAMP_STRUCT m_DeletedTime;	/* 
			Date/time the contact was deleted.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DeletedTimeLen;

	int m_DeletedBy;	/* 
			The AgentID of who deleted the contact.
		 Default: 0 SQL Type: INTEGER */
	long m_DeletedByLen;

	int m_OwnerID;	/* 
			The AgentID of contact owner, default is none.
		 Default: 0 SQL Type: INTEGER */
	long m_OwnerIDLen;

	int m_DefaultEmailAddressID;	/* 
			The PersonalDataID of the agent's default email address.
		 Default: 0 SQL Type: INTEGER */
	long m_DefaultEmailAddressIDLen;

} Contacts_t;

inline bool operator==(const Contacts_t& A,const Contacts_t& B)
{ return A.m_ContactID == B.m_ContactID; }
inline bool operator!=(const Contacts_t& A,const Contacts_t& B)
{ return A.m_ContactID != B.m_ContactID; }

/* struct ContactGroups_t - 
		Groups of contacts.
	 */
typedef struct ContactGroups_t
{
	ContactGroups_t() // default constructor
	{
		ZeroMemory( this, sizeof(ContactGroups_t) );
	}
	int m_ContactGroupID;	/* 
			Uniquely identifies contact groups.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ContactGroupIDLen;

	TCHAR m_GroupName[GROUPS_GROUPNAME_LENGTH];	/* 
			Short name of group
		 SQL Type: VARCHAR(50) */
	long m_GroupNameLen;

	int m_OwnerID;	/* 
			Owner ID
		 Default: 0 SQL Type: INTEGER */
	long m_OwnerIDLen;	

} ContactGroups_t;

inline bool operator==(const ContactGroups_t& A,const ContactGroups_t& B)
{ return A.m_ContactGroupID == B.m_ContactGroupID; }
inline bool operator!=(const ContactGroups_t& A,const ContactGroups_t& B)
{ return A.m_ContactGroupID != B.m_ContactGroupID; }

/* struct ContactGrouping_t - 
		Segments contacts into logical groups.
	 */
typedef struct ContactGrouping_t
{
	ContactGrouping_t() // default constructor
	{
		ZeroMemory( this, sizeof(ContactGrouping_t) );
	}
	int m_ContactGroupingID;	/* 
			Uniquely identifies Contact groupings.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ContactGroupingIDLen;

	int m_ContactID;	/* 
			ContactID this applies to.
		 Default: 0 SQL Type: INTEGER */
	long m_ContactIDLen;

	int m_ContactGroupID;	/* 
			Group of which this Contact is a member.
		 Default: 0 SQL Type: INTEGER */
	long m_ContactGroupIDLen;

} ContactGrouping_t;

inline bool operator==(const ContactGrouping_t& A,const ContactGrouping_t& B)
{ return A.m_ContactGroupingID == B.m_ContactGroupingID; }
inline bool operator!=(const ContactGrouping_t& A,const ContactGrouping_t& B)
{ return A.m_ContactGroupingID != B.m_ContactGroupingID; }

/* struct ScheduleReport_t - 
		Report Scheduler.
	 */
typedef struct ScheduleReport_t
{
	ScheduleReport_t() // default constructor
	{
		ZeroMemory( this, sizeof(ScheduleReport_t) );		
	}
	int m_ScheduledReportID;	/* 
			Uniquely identifies scheduled report.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ScheduledReportIDLen;

	TCHAR m_Description[AUTOMESSAGES_DESCRIPTION_LENGTH];	/* 
			Description of scheduled report.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	int m_ReportID;	/* 
			The type of report ID.
		 Default: 0 SQL Type: INTEGER */
	long m_ReportIDLen;

	unsigned char m_IsCustom;	/* 
			Is this a custom report?
		 Default: 1 SQL Type: BIT */
	long m_IsCustomLen;

	unsigned char m_IsEnabled;	/* 
			Is this report enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	int m_OwnerID;	/* 
			Who owns it.
		 Default: 0 SQL Type: INTEGER */
	long m_OwnerIDLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			The date/time this report was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	TIMESTAMP_STRUCT m_DateEdited;	/* 
			The date/time this report was last edited.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateEditedLen;

	int m_CreatedByID;	/* 
			The ID of the agent that created this report.
		 Default: 0 SQL Type: INTEGER */
	long m_CreatedByIDLen;

	int m_EditedByID;	/* 
			The ID of the agent that last edited this report.
		 Default: 0 SQL Type: INTEGER */
	long m_EditedByIDLen;
	
	TIMESTAMP_STRUCT m_LastRunTime;	/* 
			The date/time this report was last run.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_LastRunTimeLen;

	int m_LastResultCode;	/* 
			The result code of the last run.
		 Default: 0 SQL Type: INTEGER */
	long m_LastResultCodeLen;

	TIMESTAMP_STRUCT m_NextRunTime;	/* 
			When should this report run again.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_NextRunTimeLen;

	int m_RunFreq;	/* 
			How often should this run in min.
		 Default: 0 SQL Type: INTEGER */
	long m_RunFreqLen;

	int m_RunOn;	/* 
			The days of the week this report should run on.
		 Default: 0 SQL Type: INTEGER */
	long m_RunOnLen;

	unsigned char m_RunSun;	/* 
			Is this a custom report?
		 Default: 1 SQL Type: BIT */
	long m_RunSunLen;

	unsigned char m_RunMon;	/* 
			Is this a custom report?
		 Default: 1 SQL Type: BIT */
	long m_RunMonLen;

	unsigned char m_RunTue;	/* 
			Is this a custom report?
		 Default: 1 SQL Type: BIT */
	long m_RunTueLen;

	unsigned char m_RunWed;	/* 
			Is this a custom report?
		 Default: 1 SQL Type: BIT */
	long m_RunWedLen;

	unsigned char m_RunThur;	/* 
			Is this a custom report?
		 Default: 1 SQL Type: BIT */
	long m_RunThurLen;

	unsigned char m_RunFri;	/* 
			Is this a custom report?
		 Default: 1 SQL Type: BIT */
	long m_RunFriLen;

	unsigned char m_RunSat;	/* 
			Is this a custom report?
		 Default: 1 SQL Type: BIT */
	long m_RunSatLen;

	int m_RunAtHour;	/* 
			The time of day the report should be run.
		 Default: 12:01 AM SQL Type: varchar */
	long m_RunAtHourLen;

	int m_RunAtMin;	/* 
			The time of day the report should be run.
		 Default: 12:01 AM SQL Type: varchar */
	long m_RunAtMinLen;

	int m_SumPeriod;	/* 
			The time of day the report should be run.
		 Default: 12:01 AM SQL Type: varchar */
	long m_SumPeriodLen;

	int m_MaxResults;	/* 
			The time of day the report should be run.
		 Default: 12:01 AM SQL Type: varchar */
	long m_MaxResultsLen;

	unsigned char m_SendAlertToOwner;	/* 
			Should we save the result file to disk.
		 Default: 1 SQL Type: BIT */
	long m_SendAlertToOwnerLen;

	unsigned char m_SendResultToEmail;	/* 
			Should we save the result file to disk.
		 Default: 1 SQL Type: BIT */
	long m_SendResultToEmailLen;

	TCHAR m_ResultEmailTo[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			Email address to send the result to, if method is email.
		 SQL Type: VARCHAR(255) */
	long m_ResultEmailToLen;
	
	unsigned char m_IncludeResultFile;	/* 
			Should we attach the result file to the email.
		 Default: 1 SQL Type: BIT */
	long m_IncludeResultFileLen;

	unsigned char m_SaveResultToFile;	/* 
			Should we save the result file to disk.
		 Default: 1 SQL Type: BIT */
	long m_SaveResultToFileLen;

	int m_KeepNumResultFile;	/* 
			If we're doing an internal alert, who to.
		 Default: 0 SQL Type: INTEGER */
	long m_KeepNumResultFileLen;

	unsigned char m_AllowConsolidation;	/* 
			Should we allow the results to be consolidated.
		 Default: 1 SQL Type: BIT */
	long m_AllowConsolidationLen;

	int m_TargetID;	/* 
			If we're doing an internal alert, who to.
		 Default: 0 SQL Type: INTEGER */
	long m_TargetIDLen;

	unsigned char m_Flag1;	/* 
			Should we allow the results to be consolidated.
		 Default: 1 SQL Type: BIT */
	long m_Flag1Len;

	unsigned char m_Flag2;	/* 
			Should we allow the results to be consolidated.
		 Default: 1 SQL Type: BIT */
	long m_Flag2Len;

	int m_TicketStateID;	/* 
			If we're doing an internal alert, who to.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketStateIDLen;
	
	
} ScheduleReport_t;

inline bool operator==(const ScheduleReport_t& A,const ScheduleReport_t& B)
{ return A.m_ScheduledReportID == B.m_ScheduledReportID; }
inline bool operator!=(const ScheduleReport_t& A,const ScheduleReport_t& B)
{ return A.m_ScheduledReportID != B.m_ScheduledReportID; }

/* struct ReportTypes_t - 
		Report Types.
	 */
typedef struct ReportTypes_t
{
	ReportTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(ReportTypes_t) );
		m_IsEnabled = 1;
	}
	int m_ReportTypeID;	/* 
			Uniquely identifies a processing rule.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ReportTypeIDLen;

	TCHAR m_Description[STDRESPONSECATEGORIES_CATEGORYNAME_LENGTH];	/* 
			Short description of the type.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;

	unsigned char m_IsEnabled;	/* 
			Is this rule enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	unsigned char m_AllowCustom;	/* 
			Is this rule run before or after routing rules?
		 Default: 1 SQL Type: BIT */
	long m_AllowCustomLen;

} ReportTypes_t;

inline bool operator==(const ReportTypes_t& A,const ReportTypes_t& B)
{ return A.m_ReportTypeID == B.m_ReportTypeID; }
inline bool operator!=(const ReportTypes_t& A,const ReportTypes_t& B)
{ return A.m_ReportTypeID != B.m_ReportTypeID; }

/* struct ReportObjects_t - 
		Report Objects.
	 */
typedef struct ReportObjects_t
{
	ReportObjects_t() // default constructor
	{
		ZeroMemory( this, sizeof(ReportObjects_t) );
		m_IsEnabled = 1;
	}
	int m_ReportObjectID;	/* 
			Uniquely identifies a processing rule.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ReportObjectIDLen;

	TCHAR m_Description[STDRESPONSECATEGORIES_CATEGORYNAME_LENGTH];	/* 
			Short description of the type.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;

	unsigned char m_IsEnabled;	/* 
			Is this rule enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	unsigned char m_AllowCustom;	/* 
			Is this rule run before or after routing rules?
		 Default: 1 SQL Type: BIT */
	long m_AllowCustomLen;

} ReportObjects_t;

inline bool operator==(const ReportObjects_t& A,const ReportObjects_t& B)
{ return A.m_ReportObjectID == B.m_ReportObjectID; }
inline bool operator!=(const ReportObjects_t& A,const ReportObjects_t& B)
{ return A.m_ReportObjectID != B.m_ReportObjectID; }

/* struct ReportResults_t - 
		Table of report results
	 */
typedef struct ReportResults_t
{
	ReportResults_t() // default constructor
	{
		ZeroMemory( this, sizeof(ReportResults_t) );
	}
	int m_ReportResultID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ReportResultIDLen;

	int m_ScheduledReportID;	/* 
			Report Result ID.
		 Default: 0 SQL Type: INTEGER */
	long m_ScheduledReportIDLen;

	int m_ResultCode;	/* 
			Routing rule ID that seeks a match.
		 Default: 0 SQL Type: INTEGER */
	long m_ResultCodeLen;

	TCHAR m_ResultFile[AUTOMESSAGES_DESCRIPTION_LENGTH];	/* 
			Description of scheduled report.
		 SQL Type: VARCHAR(255) */
	long m_ResultFileLen;

	TIMESTAMP_STRUCT m_DateRan;	/* 
			The date/time this report was last run.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateRanLen;

	TIMESTAMP_STRUCT m_DateFrom;	/* 
			The date/time this report was last run.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateFromLen;	

	TIMESTAMP_STRUCT m_DateTo;	/* 
			The date/time this report was last run.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateToLen;	

} ReportResults_t;

inline bool operator==(const ReportResults_t& A,const ReportResults_t& B)
{ return A.m_ReportResultID == B.m_ReportResultID; }
inline bool operator!=(const ReportResults_t& A,const ReportResults_t& B)
{ return A.m_ReportResultID != B.m_ReportResultID; }


/* struct ReportResultRows_t - 
		Table of report result rows
	 */
typedef struct ReportResultRows_t
{
	ReportResultRows_t() // default constructor
	{
		ZeroMemory( this, sizeof(ReportResultRows_t) );
	}
	int m_ReportResultRowsID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ReportResultRowsIDLen;

	int m_ReportResultID;	/* 
			Report Result ID.
		 Default: 0 SQL Type: INTEGER */
	long m_ReportResultIDLen;

	TCHAR m_Col1[GROUPS_GROUPNAME_LENGTH];	/* 
			Description of standard report.
		 SQL Type: VARCHAR(255) */
	long m_Col1Len;
	
	TCHAR m_Col2[GROUPS_GROUPNAME_LENGTH];	/* 
			Description of standard report.
		 SQL Type: VARCHAR(255) */
	long m_Col2Len;
	
	int m_Col3;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col3Len;

	long m_Col4;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col4Len;

	int m_Col5;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col5Len;

	int m_Col6;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col6Len;

	int m_Col7;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col7Len;
	
	int m_Col8;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col8Len;

	int m_Col9;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col9Len;

	int m_Col10;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col10Len;

	int m_Col11;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col11Len;

	int m_Col12;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col12Len;

	int m_Col13;	/* 
		Default: 0 SQL Type: INTEGER */
	long m_Col13Len;

} ReportResultRows_t;

inline bool operator==(const ReportResultRows_t& A,const ReportResultRows_t& B)
{ return A.m_ReportResultRowsID == B.m_ReportResultRowsID; }
inline bool operator!=(const ReportResultRows_t& A,const ReportResultRows_t& B)
{ return A.m_ReportResultRowsID != B.m_ReportResultRowsID; }


/* struct StandardReport_t - 
		Standard Reports.
	 */
typedef struct StandardReport_t
{
	StandardReport_t() // default constructor
	{
		ZeroMemory( this, sizeof(StandardReport_t) );		
	}
	int m_StandardReportID;	/* 
			Uniquely identifies standard report.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_StandardReportIDLen;

	TCHAR m_Description[AUTOMESSAGES_DESCRIPTION_LENGTH];	/* 
			Description of standard report.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	unsigned char m_IsEnabled;	/* 
			Is this report enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	unsigned char m_CanSchedule;	/* 
			Can this report be scheduled?
		 Default: 1 SQL Type: BIT */
	long m_CanScheduleLen;

	int m_ReportTypeID;	/* 
			Who owns it.
		 Default: 0 SQL Type: INTEGER */
	long m_ReportTypeIDLen;

	int m_ReportObjectID;	/* 
			The ID of the agent that created this report.
		 Default: 0 SQL Type: INTEGER */
	long m_ReportObjectIDLen;

} StandardReport_t;

inline bool operator==(const StandardReport_t& A,const StandardReport_t& B)
{ return A.m_StandardReportID == B.m_StandardReportID; }
inline bool operator!=(const StandardReport_t& A,const StandardReport_t& B)
{ return A.m_StandardReportID != B.m_StandardReportID; }

/* struct CustomReport_t - 
		Custom Reports.
	 */
typedef struct CustomReport_t
{
	CustomReport_t() // default constructor
	{
		ZeroMemory( this, sizeof(CustomReport_t) );		
	}
	int m_CustomReportID;	/* 
			Uniquely identifies standard report.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_CustomReportIDLen;

	TCHAR m_Description[AUTOMESSAGES_DESCRIPTION_LENGTH];	/* 
			Description of standard report.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	unsigned char m_IsEnabled;	/* 
			Is this report enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	int m_ReportTypeID;	/* 
			Who owns it.
		 Default: 0 SQL Type: INTEGER */
	long m_ReportTypeIDLen;

	int m_ReportObjectID;	/* 
			The ID of the agent that created this report.
		 Default: 0 SQL Type: INTEGER */
	long m_ReportObjectIDLen;

	unsigned char m_BuiltIn;	/* 
			Can this report be scheduled?
		 Default: 1 SQL Type: BIT */
	long m_BuiltInLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			The date/time the message was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	TIMESTAMP_STRUCT m_DateEdited;	/* 
			The date/time the message was last edited.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateEditedLen;

	int m_CreatedBy;	/* 
			The ID of the agent that created the message.
		 Default: 0 SQL Type: INTEGER */
	long m_CreatedByLen;

	int m_EditedBy;	/* 
			The ID of the agent that last edited the message.
		 Default: 0 SQL Type: INTEGER */
	long m_EditedByLen;

	TCHAR* m_Query;	/* 
			The report query
		 SQL Type: TEXT */
	long m_QueryAllocated;
 	long m_QueryLen;

} CustomReport_t;

inline bool operator==(const CustomReport_t& A,const CustomReport_t& B)
{ return A.m_CustomReportID == B.m_CustomReportID; }
inline bool operator!=(const CustomReport_t& A,const CustomReport_t& B)
{ return A.m_CustomReportID != B.m_CustomReportID; }

/* struct AutoMessages_t - 
		Auto Messages.
	 */
typedef struct AutoMessages_t
{
	AutoMessages_t() // default constructor
	{
		ZeroMemory( this, sizeof(AutoMessages_t) );		
	}
	int m_AutoMessageID;	/* 
			Uniquely identifies auto messages.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AutoMessageIDLen;

	TCHAR m_Description[AUTOMESSAGES_DESCRIPTION_LENGTH];	/* 
			Description of auto message.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	int m_SendToTypeID;	/* 
			The type we're sending to (TicketBox, TicketCategory, etc).
		 Default: 0 SQL Type: INTEGER */
	long m_SendToTypeIDLen;

	int m_SendToID;	/* 
			The ID of the object we're sending to.
		 Default: 0 SQL Type: INTEGER */
	long m_SendToIDLen;

	unsigned char m_IsEnabled;	/* 
			Is this auto message enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	int m_WhenToSendVal;	/* 
			Value of when to send frequency.
		 Default: 0 SQL Type: INTEGER */
	long m_WhenToSendValLen;

	int m_WhenToSendFreq;	/* 
			How often to send (min, hr or day).
		 Default: 0 SQL Type: INTEGER */
	long m_WhenToSendFreqLen;

	int m_WhenToSendTypeID;	/* 
			After ticket create or close.
		 Default: 0 SQL Type: INTEGER */
	long m_WhenToSendTypeIDLen;

	int m_SrToSendID;	/* 
			The Standard Response to send.
		 Default: 0 SQL Type: INTEGER */
	long m_SrToSendIDLen;

	int m_SendFromTypeID;	/* 
			The Standard Response to send.
		 Default: 0 SQL Type: INTEGER */
	long m_SendFromTypeIDLen;

	unsigned char m_CreateNewTicket;	/* 
			Associate the message with the original ticket?
		 Default: 0 SQL Type: BIT */
	long m_CreateNewTicketLen;

	TCHAR m_ReplyToAddress[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			Email address to use as the reply-to of the message.
		 SQL Type: VARCHAR(255) */
	long m_ReplyToAddressLen;
	
	TCHAR m_ReplyToName[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			Email address name to use as the reply-to of the message.
		 SQL Type: VARCHAR(255) */
	long m_ReplyToNameLen;
	
	int m_OwnerID;	/* 
			The owner to assign to the message ticket.
		 Default: 0 SQL Type: INTEGER */
	long m_OwnerIDLen;

	int m_TicketBoxID;	/* 
			The ticketbox to assign the message to.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	int m_TicketCategoryID;	/* 
			The ticket category to assign the message to.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketCategoryIDLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			The date/time the message was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	TIMESTAMP_STRUCT m_DateEdited;	/* 
			The date/time the message was last edited.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateEditedLen;

	int m_CreatedByID;	/* 
			The ID of the agent that created the message.
		 Default: 0 SQL Type: INTEGER */
	long m_CreatedByIDLen;

	int m_EditedByID;	/* 
			The ID of the agent that last edited the message.
		 Default: 0 SQL Type: INTEGER */
	long m_EditedByIDLen;

	int m_SendToPercent;	/* 
			The percentage of recipients to send the message to.
		 Default: 100 SQL Type: INTEGER */
	long m_SendToPercentLen;

	int m_HitCount;	/* 
			The number of messages sent.
		 Default: 100 SQL Type: INTEGER */
	long m_HitCountLen;

	int m_FailCount;	/* 
			The number of messages sent.
		 Default: 100 SQL Type: INTEGER */
	long m_FailCountLen;

	int m_TotalCount;	/* 
			The number of messages sent.
		 Default: 100 SQL Type: INTEGER */
	long m_TotalCountLen;

	int m_OmitTracking;	/*
		 Default: 0 SQL Type: INTEGER */
	long m_OmitTrackingLen;
	
} AutoMessages_t;

inline bool operator==(const AutoMessages_t& A,const AutoMessages_t& B)
{ return A.m_AutoMessageID == B.m_AutoMessageID; }
inline bool operator!=(const AutoMessages_t& A,const AutoMessages_t& B)
{ return A.m_AutoMessageID != B.m_AutoMessageID; }


/* struct AutoMessagesSent_t - 
		Custom Age Alerts.
	 */
typedef struct AutoMessagesSent_t
{
	AutoMessagesSent_t() // default constructor
	{
		ZeroMemory( this, sizeof(AutoMessagesSent_t) );		
	}
	int m_AutoMessageSentID;	/* 
			Uniquely identifies age alerts.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AutoMessageSentIDLen;

	int m_AutoMessageID;	/* 
			The age alert id.
		 Default: 0 SQL Type: INTEGER */
	long m_AutoMessageIDLen;

	int m_TicketID;	/* 
			The ticket id the alert was sent for.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketIDLen;
	
	int m_ResultCode;	/* 
			The ticket id the alert was sent for.
		 Default: 0 SQL Type: INTEGER */
	long m_ResultCodeLen;

} AutoMessagesSent_t;

inline bool operator==(const AutoMessagesSent_t& A,const AutoMessagesSent_t& B)
{ return A.m_AutoMessageSentID == B.m_AutoMessageSentID; }
inline bool operator!=(const AutoMessagesSent_t& A,const AutoMessagesSent_t& B)
{ return A.m_AutoMessageSentID != B.m_AutoMessageSentID; }


/* struct AutoResponses_t - 
		AutoResponses.
	 */
typedef struct AutoResponses_t
{
	AutoResponses_t() // default constructor
	{
		ZeroMemory( this, sizeof(AutoResponses_t) );		
	}
	int m_AutoResponseID;	/* 
			Uniquely identifies auto responses.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AutoResponseIDLen;

	TCHAR m_Description[AUTOMESSAGES_DESCRIPTION_LENGTH];	/* 
			Description of auto message.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	int m_HitCount;	/* 
			The number of messages sent.
		 Default: 100 SQL Type: INTEGER */
	long m_HitCountLen;

	int m_FailCount;	/* 
			The number of messages sent.
		 Default: 100 SQL Type: INTEGER */
	long m_FailCountLen;

	int m_TotalCount;	/* 
			The number of messages sent.
		 Default: 100 SQL Type: INTEGER */
	long m_TotalCountLen;
	
	int m_SendToTypeID;	/* 
			The type we're sending to (TicketBox, TicketCategory, etc).
		 Default: 0 SQL Type: INTEGER */
	long m_SendToTypeIDLen;

	int m_SendToID;	/* 
			The ID of the object we're sending to.
		 Default: 0 SQL Type: INTEGER */
	long m_SendToIDLen;

	unsigned char m_IsEnabled;	/* 
			Is this auto message enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	int m_WhenToSendVal;	/* 
			Value of when to send frequency.
		 Default: 0 SQL Type: INTEGER */
	long m_WhenToSendValLen;

	int m_WhenToSendFreq;	/* 
			How often to send (min, hr or day).
		 Default: 0 SQL Type: INTEGER */
	long m_WhenToSendFreqLen;

	int m_WhenToSendTypeID;	/* 
			After ticket create or close.
		 Default: 0 SQL Type: INTEGER */
	long m_WhenToSendTypeIDLen;

	int m_SrToSendID;	/* 
			The Standard Response to send.
		 Default: 0 SQL Type: INTEGER */
	long m_SrToSendIDLen;

	unsigned char m_AutoReplyQuoteMsg;	/* 
			Is this auto message enabled?
		 Default: 1 SQL Type: BIT */
	long m_AutoReplyQuoteMsgLen;

	unsigned char m_AutoReplyCloseTicket;	/* 
			Is this auto message enabled?
		 Default: 1 SQL Type: BIT */
	long m_AutoReplyCloseTicketLen;

	unsigned char m_AutoReplyInTicket;	/* 
			Include a copy of the Auto Response in the Ticket?
		 Default: 1 SQL Type: BIT */
	long m_AutoReplyInTicketLen;

	int m_HeaderID;	/* 
			The Standard Response to send.
		 Default: 0 SQL Type: INTEGER */
	long m_HeaderIDLen;

	int m_FooterID;	/* 
			The Standard Response to send.
		 Default: 0 SQL Type: INTEGER */
	long m_FooterIDLen;

	unsigned char m_FooterLocation;	/* 
			0 = Bottom, 1 = Above Original Message
		 Default: 0 SQL Type: TINYINT */
	long m_FooterLocationLen;

	int m_SendFromTypeID;	/* 
			The Standard Response to send.
		 Default: 0 SQL Type: INTEGER */
	long m_SendFromTypeIDLen;

	TCHAR m_ReplyToAddress[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			Email address to use as the reply-to of the message.
		 SQL Type: VARCHAR(255) */
	long m_ReplyToAddressLen;
	
	TCHAR m_ReplyToName[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			Email address name to use as the reply-to of the message.
		 SQL Type: VARCHAR(255) */
	long m_ReplyToNameLen;
	
	TIMESTAMP_STRUCT m_DateCreated;	/* 
			The date/time the message was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	TIMESTAMP_STRUCT m_DateEdited;	/* 
			The date/time the message was last edited.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateEditedLen;

	int m_CreatedByID;	/* 
			The ID of the agent that created the message.
		 Default: 0 SQL Type: INTEGER */
	long m_CreatedByIDLen;

	int m_EditedByID;	/* 
			The ID of the agent that last edited the message.
		 Default: 0 SQL Type: INTEGER */
	long m_EditedByIDLen;

	int m_SendToPercent;	/* 
			The percentage of recipients to send the message to.
		 Default: 100 SQL Type: INTEGER */
	long m_SendToPercentLen;

	int m_OmitTracking;	/* 
			Default: 0 SQL Type: INTEGER */
	long m_OmitTrackingLen;
	
} AutoResponses_t;

inline bool operator==(const AutoResponses_t& A,const AutoResponses_t& B)
{ return A.m_AutoResponseID == B.m_AutoResponseID; }
inline bool operator!=(const AutoResponses_t& A,const AutoResponses_t& B)
{ return A.m_AutoResponseID != B.m_AutoResponseID; }


/* struct AutoResponsesSent_t - 
		Custom Age Alerts.
	 */
typedef struct AutoResponsesSent_t
{
	AutoResponsesSent_t() // default constructor
	{
		ZeroMemory( this, sizeof(AutoResponsesSent_t) );		
	}
	int m_AutoResponseSentID;	/* 
			Uniquely identifies age alerts.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AutoResponseSentIDLen;

	int m_AutoResponseID;	/* 
			The age alert id.
		 Default: 0 SQL Type: INTEGER */
	long m_AutoResponseIDLen;

	int m_TicketID;	/* 
			The ticket id the alert was sent for.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketIDLen;
	
	int m_ResultCode;	/* 
			The ticket id the alert was sent for.
		 Default: 0 SQL Type: INTEGER */
	long m_ResultCodeLen;

} AutoResponsesSent_t;

inline bool operator==(const AutoResponsesSent_t& A,const AutoResponsesSent_t& B)
{ return A.m_AutoResponseSentID == B.m_AutoResponseSentID; }
inline bool operator!=(const AutoResponsesSent_t& A,const AutoResponsesSent_t& B)
{ return A.m_AutoResponseSentID != B.m_AutoResponseSentID; }


/* struct AgeAlerts_t - 
		Custom Age Alerts.
	 */
typedef struct AgeAlerts_t
{
	AgeAlerts_t() // default constructor
	{
		ZeroMemory( this, sizeof(AgeAlerts_t) );		
	}
	int m_AgeAlertID;	/* 
			Uniquely identifies age alerts.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgeAlertIDLen;

	TCHAR m_Description[AUTOMESSAGES_DESCRIPTION_LENGTH];	/* 
			Description of age alert.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	int m_HitCount;	/* 
			The number of alerts sent.
		 Default: 0 SQL Type: INTEGER */
	long m_HitCountLen;

	int m_ThresholdMins;	/* 
			The number of minutes.
		 Default: 0 SQL Type: INTEGER */
	long m_ThresholdMinsLen;

	int m_ThresholdFreq;	/* 
			The number of minutes.
		 Default: 0 SQL Type: INTEGER */
	long m_ThresholdFreqLen;

	unsigned char m_IsEnabled;	/* 
			Is this age alert enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	unsigned char m_NoAlertIfRepliedTo;	/* 
			Don't alert if the message has been replied to?
		 Default: 1 SQL Type: BIT */
	long m_NoAlertIfRepliedToLen;

	int m_AlertOnTypeID;	/* 
			The type we're sending to (TicketBox, TicketCategory, etc).
		 Default: 0 SQL Type: INTEGER */
	long m_AlertOnTypeIDLen;

	int m_AlertOnID;	/* 
			The ID of the object we're alerting on.
		 Default: 0 SQL Type: INTEGER */
	long m_AlertOnIDLen;

	int m_AlertToTypeID;	/* 
			The type we're alerting to (Agent, Group, etc).
		 Default: 0 SQL Type: INTEGER */
	long m_AlertToTypeIDLen;

	int m_AlertToID;	/* 
			The ID of the object we're alerting to.
		 Default: 0 SQL Type: INTEGER */
	long m_AlertToIDLen;

	int m_AlertMethodID;	/* 
			Internal or External Email.
		 Default: 0 SQL Type: INTEGER */
	long m_AlertMethodIDLen;

	TCHAR m_EmailAddress[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			Email address to send alert to, if method is email.
		 SQL Type: VARCHAR(255) */
	long m_EmailAddressLen;
	
	TCHAR m_FromEmailAddress[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			From email address, if method is email.
		 SQL Type: VARCHAR(255) */
	long m_FromEmailAddressLen;
	
	TIMESTAMP_STRUCT m_DateCreated;	/* 
			The date/time the message was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	TIMESTAMP_STRUCT m_DateEdited;	/* 
			The date/time the message was last edited.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateEditedLen;

	int m_CreatedByID;	/* 
			The ID of the agent that created the message.
		 Default: 0 SQL Type: INTEGER */
	long m_CreatedByIDLen;

	int m_EditedByID;	/* 
			The ID of the agent that last edited the message.
		 Default: 0 SQL Type: INTEGER */
	long m_EditedByIDLen;
	
} AgeAlerts_t;

inline bool operator==(const AgeAlerts_t& A,const AgeAlerts_t& B)
{ return A.m_AgeAlertID == B.m_AgeAlertID; }
inline bool operator!=(const AgeAlerts_t& A,const AgeAlerts_t& B)
{ return A.m_AgeAlertID != B.m_AgeAlertID; }

/* struct AgeAlertsSent_t - 
		Custom Age Alerts.
	 */
typedef struct AgeAlertsSent_t
{
	AgeAlertsSent_t() // default constructor
	{
		ZeroMemory( this, sizeof(AgeAlertsSent_t) );		
	}
	int m_AgeAlertSentID;	/* 
			Uniquely identifies age alerts.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgeAlertSentIDLen;

	int m_AgeAlertID;	/* 
			The age alert id.
		 Default: 0 SQL Type: INTEGER */
	long m_AgeAlertIDLen;

	int m_TicketID;	/* 
			The ticket id the alert was sent for.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketIDLen;
	
} AgeAlertsSent_t;

inline bool operator==(const AgeAlertsSent_t& A,const AgeAlertsSent_t& B)
{ return A.m_AgeAlertSentID == B.m_AgeAlertSentID; }
inline bool operator!=(const AgeAlertsSent_t& A,const AgeAlertsSent_t& B)
{ return A.m_AgeAlertSentID != B.m_AgeAlertSentID; }

/* struct WaterMarkAlerts_t - 
		Custom WaterMark Alerts.
	 */
typedef struct WaterMarkAlerts_t
{
	WaterMarkAlerts_t() // default constructor
	{
		ZeroMemory( this, sizeof(WaterMarkAlerts_t) );		
	}
	int m_WaterMarkAlertID;	/* 
			Uniquely identifies age alerts.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_WaterMarkAlertIDLen;

	TCHAR m_Description[AUTOMESSAGES_DESCRIPTION_LENGTH];	/* 
			Description of age alert.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	int m_HitCount;	/* 
			The number of alerts sent.
		 Default: 0 SQL Type: INTEGER */
	long m_HitCountLen;

	int m_LowWaterMark;	/* 
			The number of minutes.
		 Default: 0 SQL Type: INTEGER */
	long m_LowWaterMarkLen;

	int m_HighWaterMark;	/* 
			The number of minutes.
		 Default: 0 SQL Type: INTEGER */
	long m_HighWaterMarkLen;

	unsigned char m_IsEnabled;	/* 
			Is this age alert enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	unsigned char m_SendLowAlert;	/* 
			Don't alert if the message has been replied to?
		 Default: 1 SQL Type: BIT */
	long m_SendLowAlertLen;

	int m_AlertOnTypeID;	/* 
			The type we're sending to (TicketBox, TicketCategory, etc).
		 Default: 0 SQL Type: INTEGER */
	long m_AlertOnTypeIDLen;

	int m_AlertOnID;	/* 
			The ID of the object we're alerting on.
		 Default: 0 SQL Type: INTEGER */
	long m_AlertOnIDLen;

	int m_AlertToTypeID;	/* 
			The type we're alerting to (Agent, Group, etc).
		 Default: 0 SQL Type: INTEGER */
	long m_AlertToTypeIDLen;

	int m_AlertToID;	/* 
			The ID of the object we're alerting to.
		 Default: 0 SQL Type: INTEGER */
	long m_AlertToIDLen;

	int m_AlertMethodID;	/* 
			Internal or External Email.
		 Default: 0 SQL Type: INTEGER */
	long m_AlertMethodIDLen;

	TCHAR m_EmailAddress[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			Email address to send alert to, if method is email.
		 SQL Type: VARCHAR(255) */
	long m_EmailAddressLen;
	
	TCHAR m_FromEmailAddress[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			From email address, if method is email.
		 SQL Type: VARCHAR(255) */
	long m_FromEmailAddressLen;
	
	TIMESTAMP_STRUCT m_DateCreated;	/* 
			The date/time the message was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	TIMESTAMP_STRUCT m_DateEdited;	/* 
			The date/time the message was last edited.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateEditedLen;

	int m_CreatedByID;	/* 
			The ID of the agent that created the message.
		 Default: 0 SQL Type: INTEGER */
	long m_CreatedByIDLen;

	int m_EditedByID;	/* 
			The ID of the agent that last edited the message.
		 Default: 0 SQL Type: INTEGER */
	long m_EditedByIDLen;

	unsigned char m_WaterMarkStatus;	/* 
			0 = Ok, 1 = Above Low Watermark, 2 = Above High Watermark
		 Default: 0 SQL Type: TINYINT */
	long m_WaterMarkStatusLen;	
	
} WaterMarkAlerts_t;

inline bool operator==(const WaterMarkAlerts_t& A,const WaterMarkAlerts_t& B)
{ return A.m_WaterMarkAlertID == B.m_WaterMarkAlertID; }
inline bool operator!=(const WaterMarkAlerts_t& A,const WaterMarkAlerts_t& B)
{ return A.m_WaterMarkAlertID != B.m_WaterMarkAlertID; }

/* struct CustomDictionary_t - 
		Contains global and user custom dictionaries.
	 */
typedef struct CustomDictionary_t
{
	CustomDictionary_t() // default constructor
	{
		ZeroMemory( this, sizeof(CustomDictionary_t) );
	}
	int m_CustomDictionaryID;	/* 
			Uniquely identifies a custom dictionary entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_CustomDictionaryIDLen;

	int m_AgentID;	/* 
			The AgentID to whom the entry belongs (0 if the entry is global).
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	TCHAR m_Word[CUSTOMDICTIONARY_WORD_LENGTH];	/* 
			The actual word to be ignored by the spell checker.
		 SQL Type: VARCHAR(255) */
	long m_WordLen;

} CustomDictionary_t;

inline bool operator==(const CustomDictionary_t& A,const CustomDictionary_t& B)
{ return A.m_CustomDictionaryID == B.m_CustomDictionaryID; }
inline bool operator!=(const CustomDictionary_t& A,const CustomDictionary_t& B)
{ return A.m_CustomDictionaryID != B.m_CustomDictionaryID; }

/* struct ForwardCCAddresses_t - 
		Table of "cc" addresses used when forwarding messasges matched by the routing rules.
	 */
typedef struct ForwardCCAddresses_t
{
	ForwardCCAddresses_t() // default constructor
	{
		ZeroMemory( this, sizeof(ForwardCCAddresses_t) );
	}
	int m_AddressID;	/* 
			Uniquely identifies a forward cc address.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AddressIDLen;

	TCHAR m_EmailAddress[FORWARDCCADDRESSES_EMAILADDRESS_LENGTH];	/* 
			Email address to cc the forward message to.
		 SQL Type: VARCHAR(255) */
	long m_EmailAddressLen;

	int m_RoutingRuleID;	/* 
			Routing rule ID that seeks a match.
		 Default: 0 SQL Type: INTEGER */
	long m_RoutingRuleIDLen;

} ForwardCCAddresses_t;

inline bool operator==(const ForwardCCAddresses_t& A,const ForwardCCAddresses_t& B)
{ return A.m_AddressID == B.m_AddressID; }
inline bool operator!=(const ForwardCCAddresses_t& A,const ForwardCCAddresses_t& B)
{ return A.m_AddressID != B.m_AddressID; }

/* struct ForwardToAddresses_t - 
		Table of "to" addresses used when forwarding messages matched by the routing rules.
	 */
typedef struct ForwardToAddresses_t
{
	ForwardToAddresses_t() // default constructor
	{
		ZeroMemory( this, sizeof(ForwardToAddresses_t) );
	}
	int m_AddressID;	/* 
			Uniquely identifies a forward to address.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AddressIDLen;

	TCHAR m_EmailAddress[FORWARDTOADDRESSES_EMAILADDRESS_LENGTH];	/* 
			Email address to forward the message to
		 SQL Type: VARCHAR(255) */
	long m_EmailAddressLen;

	int m_RoutingRuleID;	/* 
			Routing rule ID that seeks a match.
		 Default: 0 SQL Type: INTEGER */
	long m_RoutingRuleIDLen;

} ForwardToAddresses_t;

inline bool operator==(const ForwardToAddresses_t& A,const ForwardToAddresses_t& B)
{ return A.m_AddressID == B.m_AddressID; }
inline bool operator!=(const ForwardToAddresses_t& A,const ForwardToAddresses_t& B)
{ return A.m_AddressID != B.m_AddressID; }

/* struct Groups_t - 
		Groups of agents.
	 */
typedef struct Groups_t
{
	Groups_t() // default constructor
	{
		ZeroMemory( this, sizeof(Groups_t) );
	}
	int m_GroupID;	/* 
			Uniquely identifies groups.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_GroupIDLen;

	TCHAR m_GroupName[GROUPS_GROUPNAME_LENGTH];	/* 
			Short name of group
		 SQL Type: VARCHAR(50) */
	long m_GroupNameLen;

	int m_ObjectID;	/* 
			Security object ID
		 Default: 0 SQL Type: INTEGER */
	long m_ObjectIDLen;

	unsigned char m_IsDeleted;	/* 
			Indicates that the group has been deleted 
			and can be removed in a purge or archive operation.  
		 Default: 0 SQL Type: BIT */
	long m_IsDeletedLen;

	unsigned char m_IsEscGroup;	/* 
			Indicates that the group is an escalation group.			
		 Default: 0 SQL Type: BIT */
	long m_IsEscGroupLen;

	unsigned char m_UseEscTicketBox;	/* 
			Escalate tickets to a defined TicketBox.  
		 Default: 0 SQL Type: BIT */
	long m_UseEscTicketBoxLen;

	int m_AssignToTicketBoxID;	/* 
			Assign tickets escalated to this group to this TicketBox.
		 Default: 0 SQL Type: INTEGER */
	long m_AssignToTicketBoxIDLen;

	int m_TimeZoneID;	/* 
			Default TimeZone
		 Default: 0 SQL Type: INTEGER */
	long m_TimeZoneIDLen;

	int m_DictionaryID;	/* 
			Default Dictionary
		 Default: 1 SQL Type: INTEGER */
	long m_DictionaryIDLen;

} Groups_t;

inline bool operator==(const Groups_t& A,const Groups_t& B)
{ return A.m_GroupID == B.m_GroupID; }
inline bool operator!=(const Groups_t& A,const Groups_t& B)
{ return A.m_GroupID != B.m_GroupID; }

/* struct IPRanges_t - 
		Defines ranges of IP addresses for login restrictions.
	 */
typedef struct IPRanges_t
{
	IPRanges_t() // default constructor
	{
		ZeroMemory( this, sizeof(IPRanges_t) );
	}
	int m_IPRangeID;	/* 
			Uniquely identifies the IP range.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_IPRangeIDLen;

	int m_AgentID;	/* 
			AgentID this IP range applies to.
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_GroupID;	/* 
			Group this IP range applies to.
		 Default: 0 SQL Type: INTEGER */
	long m_GroupIDLen;

	int m_LowerRange;	/* 
			The lowermost IP address in network byte order.
		 Default: 0 SQL Type: INTEGER */
	long m_LowerRangeLen;

	int m_UpperRange;	/* 
			The uppermost IP address in network byte order.
		 Default: 0 SQL Type: INTEGER */
	long m_UpperRangeLen;

} IPRanges_t;

inline bool operator==(const IPRanges_t& A,const IPRanges_t& B)
{ return A.m_IPRangeID == B.m_IPRangeID; }
inline bool operator!=(const IPRanges_t& A,const IPRanges_t& B)
{ return A.m_IPRangeID != B.m_IPRangeID; }

/* struct InboundMessageAttachments_t - 
		Attachments to inbound messages.
	 */
typedef struct InboundMessageAttachments_t
{
	InboundMessageAttachments_t() // default constructor
	{
		ZeroMemory( this, sizeof(InboundMessageAttachments_t) );
	}
	int m_InboundMessageAttachmentID;	/* 
			Uniquely identifies inbound message attachments.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_InboundMessageAttachmentIDLen;

	int m_AttachmentID;	/* 
			The ID of the attachment.
		 Default: 0 SQL Type: INTEGER */
	long m_AttachmentIDLen;

	int m_InboundMessageID;	/* 
			Message this attachment belongs to.
		 Default: 0 SQL Type: INTEGER */
	long m_InboundMessageIDLen;

} InboundMessageAttachments_t;

inline bool operator==(const InboundMessageAttachments_t& A,const InboundMessageAttachments_t& B)
{ return A.m_InboundMessageAttachmentID == B.m_InboundMessageAttachmentID; }
inline bool operator!=(const InboundMessageAttachments_t& A,const InboundMessageAttachments_t& B)
{ return A.m_InboundMessageAttachmentID != B.m_InboundMessageAttachmentID; }

/* struct InboundMessageQueue_t - 
		Queue of inbound messages retrieved by the email gateway service.
	 */
typedef struct InboundMessageQueue_t
{
	InboundMessageQueue_t() // default constructor
	{
		ZeroMemory( this, sizeof(InboundMessageQueue_t) );
	}
	int m_InboundMessageQueueID;	/* 
			Uniquely identifies an entry in the inbound message queue.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_InboundMessageQueueIDLen;

	int m_InboundMessageID;	/* 
			The ID of the message in the inbound messages table.
		 Default: 0 SQL Type: INTEGER */
	long m_InboundMessageIDLen;

} InboundMessageQueue_t;

inline bool operator==(const InboundMessageQueue_t& A,const InboundMessageQueue_t& B)
{ return A.m_InboundMessageQueueID == B.m_InboundMessageQueueID; }
inline bool operator!=(const InboundMessageQueue_t& A,const InboundMessageQueue_t& B)
{ return A.m_InboundMessageQueueID != B.m_InboundMessageQueueID; }

/* struct InboundMessages_t - 
		Table of inbound messages.
	 */
typedef struct InboundMessages_t
{
	InboundMessages_t() // default constructor
	{
		ZeroMemory( this, sizeof(InboundMessages_t) );
		m_EmailDateTimeLen = SQL_NULL_DATA;
		m_DateReceivedLen = SQL_NULL_DATA;
		m_DeletedTimeLen = SQL_NULL_DATA;
		m_VirusScanStateID = 1;
		m_BodyAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Body = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_PopHeadersAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_PopHeaders = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_EmailToAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_EmailTo = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_EmailCcAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_EmailCc = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_EmailReplyToAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_EmailReplyTo = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_PriorityID = 3;
	}
	~InboundMessages_t() // beware! nonvirtual destructor
	{
		if (m_BodyAllocated > 0) free( m_Body );
		if (m_PopHeadersAllocated > 0) free( m_PopHeaders );
		if (m_EmailToAllocated > 0) free( m_EmailTo );
		if (m_EmailCcAllocated > 0) free( m_EmailCc );
		if (m_EmailReplyToAllocated > 0) free( m_EmailReplyTo );
	}
	InboundMessages_t( const InboundMessages_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(InboundMessages_t));
		if (m_BodyAllocated > 0)
		{
			m_Body = (TCHAR*)calloc( m_BodyAllocated, 1 );
			memcpy(m_Body,i.m_Body,m_BodyAllocated);
		}
		if (m_PopHeadersAllocated > 0)
		{
			m_PopHeaders = (TCHAR*)calloc( m_PopHeadersAllocated, 1 );
			memcpy(m_PopHeaders,i.m_PopHeaders,m_PopHeadersAllocated);
		}
		if (m_EmailToAllocated > 0)
		{
			m_EmailTo = (TCHAR*)calloc( m_EmailToAllocated, 1 );
			memcpy(m_EmailTo,i.m_EmailTo,m_EmailToAllocated);
		}
		if (m_EmailCcAllocated > 0)
		{
			m_EmailCc = (TCHAR*)calloc( m_EmailCcAllocated, 1 );
			memcpy(m_EmailCc,i.m_EmailCc,m_EmailCcAllocated);
		}
		if (m_EmailReplyToAllocated > 0)
		{
			m_EmailReplyTo = (TCHAR*)calloc( m_EmailReplyToAllocated, 1 );
			memcpy(m_EmailReplyTo,i.m_EmailReplyTo,m_EmailReplyToAllocated);
		}
	}
	InboundMessages_t& operator=(const InboundMessages_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_BodyAllocated > 0) free( m_Body );
			if (m_PopHeadersAllocated > 0) free( m_PopHeaders );
			if (m_EmailToAllocated > 0) free( m_EmailTo );
			if (m_EmailCcAllocated > 0) free( m_EmailCc );
			if (m_EmailReplyToAllocated > 0) free( m_EmailReplyTo );
			memcpy(this,&i,sizeof(InboundMessages_t));
			if (m_BodyAllocated > 0)
			{
				m_Body = (TCHAR*)calloc( m_BodyAllocated, 1 );
				memcpy(m_Body,i.m_Body,m_BodyAllocated);
			}
			if (m_PopHeadersAllocated > 0)
			{
				m_PopHeaders = (TCHAR*)calloc( m_PopHeadersAllocated, 1 );
				memcpy(m_PopHeaders,i.m_PopHeaders,m_PopHeadersAllocated);
			}
			if (m_EmailToAllocated > 0)
			{
				m_EmailTo = (TCHAR*)calloc( m_EmailToAllocated, 1 );
				memcpy(m_EmailTo,i.m_EmailTo,m_EmailToAllocated);
			}
			if (m_EmailCcAllocated > 0)
			{
				m_EmailCc = (TCHAR*)calloc( m_EmailCcAllocated, 1 );
				memcpy(m_EmailCc,i.m_EmailCc,m_EmailCcAllocated);
			}
			if (m_EmailReplyToAllocated > 0)
			{
				m_EmailReplyTo = (TCHAR*)calloc( m_EmailReplyToAllocated, 1 );
				memcpy(m_EmailReplyTo,i.m_EmailReplyTo,m_EmailReplyToAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_InboundMessageID;	/* 
			Uniquely identifies an inbound message.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_InboundMessageIDLen;

	int m_TicketID;	/* 
			The ticket this message belongs to.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketIDLen;

	TCHAR m_EmailFrom[INBOUNDMESSAGES_EMAILFROM_LENGTH];	/* 
			The email address the message was sent from (inside the angle brackets).
		 SQL Type: VARCHAR(255) */
	long m_EmailFromLen;

	TCHAR m_EmailFromName[INBOUNDMESSAGES_EMAILFROMNAME_LENGTH];	/* 
			The name associated with the from email address (outside the angle brackets).
		 SQL Type: VARCHAR(128) */
	long m_EmailFromNameLen;

	TCHAR m_EmailPrimaryTo[INBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH];	/* 
			The primary email to address.
		 SQL Type: VARCHAR(255) */
	long m_EmailPrimaryToLen;

	TIMESTAMP_STRUCT m_EmailDateTime;	/* 
			Email date/time stamp message was sent.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_EmailDateTimeLen;

	TIMESTAMP_STRUCT m_DateReceived;	/* 
			Email date/time stamp message was received.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateReceivedLen;

	TCHAR m_Subject[INBOUNDMESSAGES_SUBJECT_LENGTH];	/* 
			Email subject line.
		 SQL Type: VARCHAR(255) */
	long m_SubjectLen;

	TCHAR m_MediaType[INBOUNDMESSAGES_MEDIATYPE_LENGTH];	/* 
			MIME content type of attachment.
		 SQL Type: VARCHAR(125) */
	long m_MediaTypeLen;

	TCHAR m_MediaSubType[INBOUNDMESSAGES_MEDIASUBTYPE_LENGTH];	/* 
			MIME content sub-type of attachment.
		 SQL Type: VARCHAR(125) */
	long m_MediaSubTypeLen;

	unsigned char m_IsDeleted;	/* 
			Is this message deleted?
		 Default: 0 SQL Type: TINYINT */
	long m_IsDeletedLen;

	int m_DeletedBy;	/* 
			The AgentID of the user that deleted this message (0 if by system)
		 Default: 0 SQL Type: INTEGER */
	long m_DeletedByLen;

	TIMESTAMP_STRUCT m_DeletedTime;	/* 
			The date/time the message was deleted.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DeletedTimeLen;

	int m_RoutingRuleID;	/* 
			The routing rule used to route this message.
		 Default: 0 SQL Type: INTEGER */
	long m_RoutingRuleIDLen;

	int m_MessageSourceID;	/* 
			The source this message was received from.
		 Default: 0 SQL Type: INTEGER */
	long m_MessageSourceIDLen;

	int m_VirusScanStateID;	/* 
			The current virus scan state of the message body.
		 Default: 1 SQL Type: INTEGER */
	long m_VirusScanStateIDLen;

	int m_ReplyToMsgID;	/* 
			The outbound message this was in reply to.
		 Default: 0 SQL Type: INTEGER */
	long m_ReplyToMsgIDLen;

	unsigned char m_ReplyToIDIsInbound;	/* 
			Is ReplyToMsgID inbound or outbount?
		 Default: 0 SQL Type: BIT */
	long m_ReplyToIDIsInboundLen;

	TCHAR m_VirusName[INBOUNDMESSAGES_VIRUSNAME_LENGTH];	/* 
			Name of the virus infecting this attachment file, if any.
		 SQL Type: VARCHAR(125) */
	long m_VirusNameLen;

	TCHAR* m_Body;	/* 
			The body of the message.
		 SQL Type: TEXT */
	long m_BodyAllocated;
 	long m_BodyLen;

	TCHAR* m_PopHeaders;	/* 
			The POP3 headers.
		 SQL Type: TEXT */
	long m_PopHeadersAllocated;
 	long m_PopHeadersLen;

	TCHAR* m_EmailTo;	/* 
			The full list of to: addresses.
		 SQL Type: TEXT */
	long m_EmailToAllocated;
 	long m_EmailToLen;

	TCHAR* m_EmailCc;	/* 
			The cc field.
		 SQL Type: TEXT */
	long m_EmailCcAllocated;
 	long m_EmailCcLen;

	TCHAR* m_EmailReplyTo;	/* 
			The reply-to field.
		 SQL Type: TEXT */
	long m_EmailReplyToAllocated;
 	long m_EmailReplyToLen;

	int m_ContactID;	/* 
			The contact that this inbound message was from.
		 Default: 0 SQL Type: INTEGER */
	long m_ContactIDLen;

	int m_PriorityID;	/* 
			The priority of the message
		 Default: 3 SQL Type: INTEGER */
	long m_PriorityIDLen;

	int m_ArchiveID;	/* 
			If this message was archived, the corresponding archive ID.
		 Default: 0 SQL Type: INTEGER */
	long m_ArchiveIDLen;

	int m_OriginalTicketBoxID;	/* 
			The TicketBoxID of the ticket when the message was created.
		 Default: 0 SQL Type: INTEGER */
	long m_OriginalTicketBoxIDLen;

	int m_OriginalOwnerID;	/* 
			The OwnerID of the ticket when the message was created.
		 Default: 0 SQL Type: INTEGER */
	long m_OriginalOwnerIDLen;

	int m_OriginalTicketCategoryID;	/* 
			The ticket category ID of the ticket when the message was created.
		 Default: 0 SQL Type: INTEGER */
	long m_OriginalTicketCategoryIDLen;

	unsigned char m_ReadReceipt;	/* 
			Read Receipt Request Status.
		 Default: 0 SQL Type: TINYINT */
	long m_ReadReceiptLen;

	TCHAR m_ReadReceiptTo[INBOUNDMESSAGES_EMAILFROM_LENGTH];	/* 
			The email address the MDN should be sent to.
		 SQL Type: VARCHAR(255) */
	long m_ReadReceiptToLen;
	
} InboundMessages_t;

inline bool operator==(const InboundMessages_t& A,const InboundMessages_t& B)
{ return A.m_InboundMessageID == B.m_InboundMessageID; }
inline bool operator!=(const InboundMessages_t& A,const InboundMessages_t& B)
{ return A.m_InboundMessageID != B.m_InboundMessageID; }

//*****************************************************************************
// Added by Mark Mohr 11/30/2005

struct InboundMessageRead_t
{
	int m_InboundMessageReadID;
	long m_InboundMessageReadIDLen;

	int m_InboundMessageID;
	long m_InboundMessageIDLen;

	int m_AgentID;
	long m_AgentIDLen;

	InboundMessageRead_t()
	{
		ZeroMemory(this, sizeof(InboundMessageRead_t));
	}
};

inline bool operator==(const InboundMessageRead_t& A,const InboundMessageRead_t& B)
{ return A.m_InboundMessageReadID == B.m_InboundMessageReadID; }
inline bool operator!=(const InboundMessageRead_t& A,const InboundMessageRead_t& B)
{ return A.m_InboundMessageReadID != B.m_InboundMessageReadID; }

//*****************************************************************************

//*****************************************************************************
// Added by Mark Mohr 12/1/2005

struct SRKeywordResults_t
{
	int m_SRKeywordResultsID;
	long m_SRKeywordResultsIDLen;

	int m_InboundMessageID;
	long m_InboundMessageIDLen;

	int m_StandardResponseID;
	long m_StandardResponseIDLen;

	int m_Score;
	long m_ScoreLen;

	SRKeywordResults_t()
	{
		ZeroMemory(this, sizeof(SRKeywordResults_t));
	}
};

inline bool operator==(const SRKeywordResults_t& A,const SRKeywordResults_t& B)
{ return A.m_SRKeywordResultsID == B.m_SRKeywordResultsID; }
inline bool operator!=(const SRKeywordResults_t& A,const SRKeywordResults_t& B)
{ return A.m_SRKeywordResultsID != B.m_SRKeywordResultsID; }

//*****************************************************************************

struct TicketNotesRead_t
{
	int m_TicketNotesReadID;
	long m_TicketNotesReadIDLen;

	int m_TicketNoteID;
	long m_TicketNoteIDLen;

	int m_AgentID;
	long m_AgentIDLen;

	TicketNotesRead_t()
	{
		ZeroMemory(this, sizeof(TicketNotesRead_t));
	}
};

inline bool operator==(const TicketNotesRead_t& A,const TicketNotesRead_t& B)
{ return A.m_TicketNotesReadID == B.m_TicketNotesReadID; }
inline bool operator!=(const TicketNotesRead_t& A,const TicketNotesRead_t& B)
{ return A.m_TicketNotesReadID != B.m_TicketNotesReadID; }

struct Folders_t
{
	int m_FolderID;
	long m_FolderIDLen;

	TCHAR m_Name[56];
	long m_NameLen;

	int m_AgentID;
	long m_AgentIDLen;

	int m_ParentID;
	long m_ParentIDLen;

	Folders_t()
	{
		ZeroMemory(this, sizeof(Folders_t));
	}
};

inline bool operator==(const Folders_t& A,const Folders_t& B)
{ return A.m_FolderID == B.m_FolderID; }
inline bool operator!=(const Folders_t& A,const Folders_t& B)
{ return A.m_FolderID != B.m_FolderID; }

//*****************************************************************************
//*****************************************************************************

/* struct InstantMessages_t - 
		Where instant messages are stored.
	 */
typedef struct InstantMessages_t
{
	InstantMessages_t() // default constructor
	{
		ZeroMemory( this, sizeof(InstantMessages_t) );
		m_BodyAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Body = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_TimeStampLen = SQL_NULL_DATA;
	}
	~InstantMessages_t() // beware! nonvirtual destructor
	{
		if (m_BodyAllocated > 0) free( m_Body );
	}
	InstantMessages_t( const InstantMessages_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(InstantMessages_t));
		if (m_BodyAllocated > 0)
		{
			m_Body = (TCHAR*)calloc( m_BodyAllocated, 1 );
			memcpy(m_Body,i.m_Body,m_BodyAllocated);
		}
	}
	InstantMessages_t& operator=(const InstantMessages_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_BodyAllocated > 0) free( m_Body );
			memcpy(this,&i,sizeof(InstantMessages_t));
			if (m_BodyAllocated > 0)
			{
				m_Body = (TCHAR*)calloc( m_BodyAllocated, 1 );
				memcpy(m_Body,i.m_Body,m_BodyAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_InstantMessageID;	/* 
			Uniquely identifies an instant message
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_InstantMessageIDLen;

	TCHAR* m_Body;	/* 
			Contains the body of the message
		 SQL Type: TEXT */
	long m_BodyAllocated;
 	long m_BodyLen;

	int m_FromAgentID;	/* 
			The ID of the Agent that sent the message or zero if from system.
		 Default: 0 SQL Type: INTEGER */
	long m_FromAgentIDLen;

	int m_ToAgentID;	/* 
			The ID of the destination Agent.
		 Default: 0 SQL Type: INTEGER */
	long m_ToAgentIDLen;

	TIMESTAMP_STRUCT m_TimeStamp;	/* 
			When the instant message was sent.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_TimeStampLen;

} InstantMessages_t;

inline bool operator==(const InstantMessages_t& A,const InstantMessages_t& B)
{ return A.m_InstantMessageID == B.m_InstantMessageID; }
inline bool operator!=(const InstantMessages_t& A,const InstantMessages_t& B)
{ return A.m_InstantMessageID != B.m_InstantMessageID; }

/* struct LogEntryTypes_t - 
		Types of log entries that can be turned on or off.
	 */
typedef struct LogEntryTypes_t
{
	LogEntryTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(LogEntryTypes_t) );
		m_SeverityLevels = 14;
	}
	int m_LogEntryTypeID;	/* 
			Uniquely identifies a log entry type.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_LogEntryTypeIDLen;

	TCHAR m_TypeDescrip[LOGENTRYTYPES_TYPEDESCRIP_LENGTH];	/* 
			Log entry type description (e.g. event audit, debug info, system).
		 SQL Type: VARCHAR(125) */
	long m_TypeDescripLen;

	int m_SeverityLevels;	/* 
			A bitfield indicating what severity levels to log.
		 Default: 14 SQL Type: INTEGER */
	long m_SeverityLevelsLen;

} LogEntryTypes_t;

inline bool operator==(const LogEntryTypes_t& A,const LogEntryTypes_t& B)
{ return A.m_LogEntryTypeID == B.m_LogEntryTypeID; }
inline bool operator!=(const LogEntryTypes_t& A,const LogEntryTypes_t& B)
{ return A.m_LogEntryTypeID != B.m_LogEntryTypeID; }

/* struct Log_t - 
		Enumerates the log entries.
	 */
typedef struct Log_t
{
	Log_t() // default constructor
	{
		ZeroMemory( this, sizeof(Log_t) );
	}
	int m_LogID;	/* 
			Uniquely identifies the log entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_LogIDLen;

	TIMESTAMP_STRUCT m_LogTime;	/* 
			Log entry date/time.
		 SQL Type: DATETIME */
	long m_LogTimeLen;

	int m_ServerID;	/* 
			Uniquely identifies the server the entry pertains to.
		 SQL Type: INTEGER */
	long m_ServerIDLen;

	int m_ErrorCode;	/* 
			The error code.
		 SQL Type: INTEGER */
	long m_ErrorCodeLen;

	int m_LogSeverityID;	/* 
			The log severity.
		 SQL Type: INTEGER */
	long m_LogSeverityIDLen;

	int m_LogEntryTypeID;	/* 
			The log entry type.
		 SQL Type: INTEGER */
	long m_LogEntryTypeIDLen;

	TCHAR m_LogText[LOG_TEXT_LENGTH];	/* 
			Log Text
		 SQL Type: VARCHAR(255) */
	long m_LogTextLen;

} Log_t;

inline bool operator==(const Log_t& A,const Log_t& B)
{ return A.m_LogID == B.m_LogID; }
inline bool operator!=(const Log_t& A,const Log_t& B)
{ return A.m_LogID != B.m_LogID; }

/* struct LogSeverity_t - 
		Enumerates the severity levels associated with log entries.
	 */
typedef struct LogSeverity_t
{
	LogSeverity_t() // default constructor
	{
		ZeroMemory( this, sizeof(LogSeverity_t) );
	}
	int m_LogSeverityID;	/* 
			Uniquely identifies the log severity level.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_LogSeverityIDLen;

	TCHAR m_Description[LOGSEVERITY_DESCRIPTION_LENGTH];	/* 
			Severity description (e.g. warning, error, informational)
		 SQL Type: VARCHAR(55) */
	long m_DescriptionLen;

} LogSeverity_t;

inline bool operator==(const LogSeverity_t& A,const LogSeverity_t& B)
{ return A.m_LogSeverityID == B.m_LogSeverityID; }
inline bool operator!=(const LogSeverity_t& A,const LogSeverity_t& B)
{ return A.m_LogSeverityID != B.m_LogSeverityID; }

/* struct MatchFromAddresses_t - 
		Allows routing rules to match one or more "from" email addresses.
	 */
typedef struct MatchFromAddresses_t
{
	MatchFromAddresses_t() // default constructor
	{
		ZeroMemory( this, sizeof(MatchFromAddresses_t) );
	}
	int m_MatchID;	/* 
			Uniquely identifies a match fom address.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MatchIDLen;

	TCHAR m_EmailAddress[MATCHFROMADDRESSES_EMAILADDRESS_LENGTH];	/* 
			Email address to match.
		 SQL Type: VARCHAR(255) */
	long m_EmailAddressLen;

	int m_RoutingRuleID;	/* 
			Routing rule that seeks a match.
		 Default: 0 SQL Type: INTEGER */
	long m_RoutingRuleIDLen;

} MatchFromAddresses_t;

inline bool operator==(const MatchFromAddresses_t& A,const MatchFromAddresses_t& B)
{ return A.m_MatchID == B.m_MatchID; }
inline bool operator!=(const MatchFromAddresses_t& A,const MatchFromAddresses_t& B)
{ return A.m_MatchID != B.m_MatchID; }

/* struct MatchFromAddressP_t - 
		Allows routing rules to match one or more "from" email addresses.
	 */
typedef struct MatchFromAddressP_t
{
	MatchFromAddressP_t() // default constructor
	{
		ZeroMemory( this, sizeof(MatchFromAddressP_t) );
	}
	int m_MatchID;	/* 
			Uniquely identifies a match fom address.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MatchIDLen;

	TCHAR m_EmailAddress[MATCHFROMADDRESSES_EMAILADDRESS_LENGTH];	/* 
			Email address to match.
		 SQL Type: VARCHAR(255) */
	long m_EmailAddressLen;

	int m_ProcessingRuleID;	/* 
			Routing rule that seeks a match.
		 Default: 0 SQL Type: INTEGER */
	long m_ProcessingRuleIDLen;

} MatchFromAddressP_t;

inline bool operator==(const MatchFromAddressP_t& A,const MatchFromAddressP_t& B)
{ return A.m_MatchID == B.m_MatchID; }
inline bool operator!=(const MatchFromAddressP_t& A,const MatchFromAddressP_t& B)
{ return A.m_MatchID != B.m_MatchID; }

/* struct MatchText_t - 
		Contains text which routing rules attempt to match in various locations (e.g. msg body, subject, etc.).
	 */
typedef struct MatchText_t
{
	MatchText_t() // default constructor
	{
		ZeroMemory( this, sizeof(MatchText_t) );
		m_MatchTextAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_MatchText = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
	}
	~MatchText_t() // beware! nonvirtual destructor
	{
		if (m_MatchTextAllocated > 0) free( m_MatchText );
	}
	MatchText_t( const MatchText_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(MatchText_t));
		if (m_MatchTextAllocated > 0)
		{
			m_MatchText = (TCHAR*)calloc( m_MatchTextAllocated, 1 );
			memcpy(m_MatchText,i.m_MatchText,m_MatchTextAllocated);
		}
	}
	MatchText_t& operator=(const MatchText_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_MatchTextAllocated > 0) free( m_MatchText );
			memcpy(this,&i,sizeof(MatchText_t));
			if (m_MatchTextAllocated > 0)
			{
				m_MatchText = (TCHAR*)calloc( m_MatchTextAllocated, 1 );
				memcpy(m_MatchText,i.m_MatchText,m_MatchTextAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_MatchID;	/* 
			Uniquely identifies the match text.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MatchIDLen;

	TCHAR* m_MatchText;	/* 
			The text to match.
		 SQL Type: TEXT */
	long m_MatchTextAllocated;
 	long m_MatchTextLen;

	unsigned char m_MatchLocation;	/* 
			Where to match the text (0 = subject, 1 = body) - bitfield?
		 SQL Type: TINYINT */
	long m_MatchLocationLen;

	int m_RoutingRuleID;	/* 
			The routing rule that seeks the match.
		 Default: 0 SQL Type: INTEGER */
	long m_RoutingRuleIDLen;

	unsigned char m_IsRegEx;	/* 
			Is the text to match a RegEx statement?
		 Default: 0 SQL Type: BIT */
	long m_IsRegExLen;	

} MatchText_t;

inline bool operator==(const MatchText_t& A,const MatchText_t& B)
{ return A.m_MatchID == B.m_MatchID; }
inline bool operator!=(const MatchText_t& A,const MatchText_t& B)
{ return A.m_MatchID != B.m_MatchID; }

/* struct MatchTextP_t - 
		Contains text which processing rules attempt to match in various locations (e.g. msg body, subject, etc.).
	 */
typedef struct MatchTextP_t
{
	MatchTextP_t() // default constructor
	{
		ZeroMemory( this, sizeof(MatchTextP_t) );
		m_MatchTextAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_MatchText = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
	}
	~MatchTextP_t() // beware! nonvirtual destructor
	{
		if (m_MatchTextAllocated > 0) free( m_MatchText );
	}
	MatchTextP_t( const MatchTextP_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(MatchTextP_t));
		if (m_MatchTextAllocated > 0)
		{
			m_MatchText = (TCHAR*)calloc( m_MatchTextAllocated, 1 );
			memcpy(m_MatchText,i.m_MatchText,m_MatchTextAllocated);
		}
	}
	MatchTextP_t& operator=(const MatchTextP_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_MatchTextAllocated > 0) free( m_MatchText );
			memcpy(this,&i,sizeof(MatchTextP_t));
			if (m_MatchTextAllocated > 0)
			{
				m_MatchText = (TCHAR*)calloc( m_MatchTextAllocated, 1 );
				memcpy(m_MatchText,i.m_MatchText,m_MatchTextAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_MatchID;	/* 
			Uniquely identifies the match text.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MatchIDLen;

	TCHAR* m_MatchText;	/* 
			The text to match.
		 SQL Type: TEXT */
	long m_MatchTextAllocated;
 	long m_MatchTextLen;

	unsigned char m_MatchLocation;	/* 
			Where to match the text (0 = subject, 1 = body) - bitfield?
		 SQL Type: TINYINT */
	long m_MatchLocationLen;

	int m_ProcessingRuleID;	/* 
			The routing rule that seeks the match.
		 Default: 0 SQL Type: INTEGER */
	long m_ProcessingRuleIDLen;

	unsigned char m_IsRegEx;	/* 
			Is the text to match a RegEx statement?
		 Default: 0 SQL Type: BIT */
	long m_IsRegExLen;	

} MatchTextP_t;

inline bool operator==(const MatchTextP_t& A,const MatchTextP_t& B)
{ return A.m_MatchID == B.m_MatchID; }
inline bool operator!=(const MatchTextP_t& A,const MatchTextP_t& B)
{ return A.m_MatchID != B.m_MatchID; }

/* struct MatchToAddresses_t - 
		Table of "to" addresses to be matched by the routing rules.
	 */
typedef struct MatchToAddresses_t
{
	MatchToAddresses_t() // default constructor
	{
		ZeroMemory( this, sizeof(MatchToAddresses_t) );
	}
	int m_MatchID;	/* 
			Uniquely identifies a match to address.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MatchIDLen;

	TCHAR m_EmailAddress[MATCHTOADDRESSES_EMAILADDRESS_LENGTH];	/* 
			Email address to match.
		 SQL Type: VARCHAR(255) */
	long m_EmailAddressLen;

	int m_RoutingRuleID;	/* 
			Routing rule ID that seeks a match.
		 Default: 0 SQL Type: INTEGER */
	long m_RoutingRuleIDLen;
	
} MatchToAddresses_t;

inline bool operator==(const MatchToAddresses_t& A,const MatchToAddresses_t& B)
{ return A.m_MatchID == B.m_MatchID; }
inline bool operator!=(const MatchToAddresses_t& A,const MatchToAddresses_t& B)
{ return A.m_MatchID != B.m_MatchID; }

/* struct MatchToAddressP_t - 
		Table of "to" addresses to be matched by the routing rules.
	 */
typedef struct MatchToAddressP_t
{
	MatchToAddressP_t() // default constructor
	{
		ZeroMemory( this, sizeof(MatchToAddressP_t) );
	}
	int m_MatchID;	/* 
			Uniquely identifies a match to address.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MatchIDLen;

	TCHAR m_EmailAddress[MATCHTOADDRESSES_EMAILADDRESS_LENGTH];	/* 
			Email address to match.
		 SQL Type: VARCHAR(255) */
	long m_EmailAddressLen;

	int m_ProcessingRuleID;	/* 
			Routing rule ID that seeks a match.
		 Default: 0 SQL Type: INTEGER */
	long m_ProcessingRuleIDLen;


} MatchToAddressP_t;

inline bool operator==(const MatchToAddressP_t& A,const MatchToAddressP_t& B)
{ return A.m_MatchID == B.m_MatchID; }
inline bool operator!=(const MatchToAddressP_t& A,const MatchToAddressP_t& B)
{ return A.m_MatchID != B.m_MatchID; }

/* struct CustomAgeAlerts_t - 
		Table of age alerts.
	 */
typedef struct CustomAgeAlerts_t
{
	CustomAgeAlerts_t() // default constructor
	{
		ZeroMemory( this, sizeof(CustomAgeAlerts_t) );
	}
	int m_AgeAlertID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgeAlertIDLen;

	int m_AlertOnTypeID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AlertOnTypeIDLen;

	int m_TicketBoxID;	/* 
			TicketBox ID.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	int m_TicketID;	/* 
			Routing rule ID that seeks a match.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketIDLen;
	
} CustomAgeAlerts_t;

inline bool operator==(const CustomAgeAlerts_t& A,const CustomAgeAlerts_t& B)
{ return A.m_AgeAlertID == B.m_AgeAlertID; }
inline bool operator!=(const CustomAgeAlerts_t& A,const CustomAgeAlerts_t& B)
{ return A.m_AgeAlertID != B.m_AgeAlertID; }

/* struct AssignToAgents_t - 
		Table of Agents to be used by the routing rules.
	 */
typedef struct AssignToAgents_t
{
	AssignToAgents_t() // default constructor
	{
		ZeroMemory( this, sizeof(AssignToAgents_t) );
	}
	int m_AgentRoutingID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgentRoutingIDLen;

	int m_AgentID;	/* 
			Agent ID.
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_RoutingRuleID;	/* 
			Routing rule ID that seeks a match.
		 Default: 0 SQL Type: INTEGER */
	long m_RoutingRuleIDLen;	

} AssignToAgents_t;

inline bool operator==(const AssignToAgents_t& A,const AssignToAgents_t& B)
{ return A.m_AgentRoutingID == B.m_AgentRoutingID; }
inline bool operator!=(const AssignToAgents_t& A,const AssignToAgents_t& B)
{ return A.m_AgentRoutingID != B.m_AgentRoutingID; }

/* struct AssignToTicketBoxes_t - 
		Table of ticketboxes to be used by the routing rules.
	 */
typedef struct AssignToTicketBoxes_t
{
	AssignToTicketBoxes_t() // default constructor
	{
		ZeroMemory( this, sizeof(AssignToTicketBoxes_t) );
	}
	int m_TicketBoxRoutingID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxRoutingIDLen;

	int m_TicketBoxID;	/* 
			TicketBox ID.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	int m_RoutingRuleID;	/* 
			Routing rule ID that seeks a match.
		 Default: 0 SQL Type: INTEGER */
	long m_RoutingRuleIDLen;

	int m_Percentage;	/* 
			Percentage to be used by Routing Rule.
		 Default: 0 SQL Type: INTEGER */
	long m_PercentageLen;

} AssignToTicketBoxes_t;

inline bool operator==(const AssignToTicketBoxes_t& A,const AssignToTicketBoxes_t& B)
{ return A.m_TicketBoxRoutingID == B.m_TicketBoxRoutingID; }
inline bool operator!=(const AssignToTicketBoxes_t& A,const AssignToTicketBoxes_t& B)
{ return A.m_TicketBoxRoutingID != B.m_TicketBoxRoutingID; }

/* struct MessageDestinations_t - 
		Message destinations represent outbound message gateways.
	 */
typedef struct MessageDestinations_t
{
	MessageDestinations_t() // default constructor
	{
		ZeroMemory( this, sizeof(MessageDestinations_t) );
		m_AccessTokenAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_AccessToken = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_RefreshTokenAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_RefreshToken = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_SMTPPort = 25;
		m_IsActive = 1;
		m_ConnTimeoutSecs = 300;
		m_MaxOutboundMsgSize = 5000;
		m_MaxSendRetryHours = 48;
		m_LastProcessIntervalLen = SQL_NULL_DATA;
		m_ProcessFreqMins = 1;
		m_ErrorCode = 0;
		m_AccessTokenExpireLen = SQL_NULL_DATA;
		m_RefreshTokenExpireLen = SQL_NULL_DATA;
		m_OAuthHostID = 0;
	}
	~MessageDestinations_t() // beware! nonvirtual destructor
	{
		if (m_AccessTokenAllocated > 0) free( m_AccessToken );
		if (m_RefreshTokenAllocated > 0) free( m_RefreshToken );
	}
	MessageDestinations_t( const MessageDestinations_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(MessageDestinations_t));
		if (m_AccessTokenAllocated > 0)
		{
			m_AccessToken = (TCHAR*)calloc( m_AccessTokenAllocated, 1 );
			memcpy(m_AccessToken,i.m_AccessToken,m_AccessTokenAllocated);
		}
		if (m_RefreshTokenAllocated > 0)
		{
			m_RefreshToken = (TCHAR*)calloc( m_RefreshTokenAllocated, 1 );
			memcpy(m_RefreshToken,i.m_RefreshToken,m_RefreshTokenAllocated);
		}
	}
	MessageDestinations_t& operator=(const MessageDestinations_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_AccessTokenAllocated > 0) free( m_AccessToken );
			if (m_RefreshTokenAllocated > 0) free( m_RefreshToken );
			memcpy(this,&i,sizeof(MessageDestinations_t));
			if (m_AccessTokenAllocated > 0)
			{
				m_AccessToken = (TCHAR*)calloc( m_AccessTokenAllocated, 1 );
				memcpy(m_AccessToken,i.m_AccessToken,m_AccessTokenAllocated);
			}
			if (m_RefreshTokenAllocated > 0)
			{
				m_RefreshToken = (TCHAR*)calloc( m_RefreshTokenAllocated, 1 );
				memcpy(m_RefreshToken,i.m_RefreshToken,m_RefreshTokenAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_MessageDestinationID;	/* 
			Uniquely identifies a message destination.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MessageDestinationIDLen;

	TCHAR m_ServerAddress[MESSAGEDESTINATIONS_SERVERADDRESS_LENGTH];	/* 
			 Remote mail server address (name or ip).
		 SQL Type: VARCHAR(255) */
	long m_ServerAddressLen;

	unsigned char m_UseSMTPAuth;	/* 
			Uses SMTP Auth (yes/no).
		 Default: 0 SQL Type: BIT */
	long m_UseSMTPAuthLen;

	TCHAR m_AuthUser[MESSAGEDESTINATIONS_AUTHUSER_LENGTH];	/* 
			Username for auth method.
		 SQL Type: VARCHAR(125) */
	long m_AuthUserLen;

	TCHAR m_AuthPass[MESSAGEDESTINATIONS_AUTHPASS_LENGTH];	/* 
			Password for auth method.
		 SQL Type: VARCHAR(125) */
	long m_AuthPassLen;

	int m_SMTPPort;	/* 
			SMTP port to connect to on remote host.
		 Default: 25 SQL Type: INTEGER */
	long m_SMTPPortLen;

	unsigned char m_IsActive;	/* 
			Is destination active (yes/no).
		 Default: 1 SQL Type: BIT */
	long m_IsActiveLen;

	int m_ConnTimeoutSecs;	/* 
			Connection timeout in seconds
		 Default: 300 SQL Type: INTEGER */
	long m_ConnTimeoutSecsLen;

	int m_MaxOutboundMsgSize;	/* 
			Maximum outbound message size (in K).
		 Default: 5000 SQL Type: INTEGER */
	long m_MaxOutboundMsgSizeLen;

	int m_MaxSendRetryHours;	/* 
			Maximum number of hours that outbound message delivery will
			be attempted in re-try state.
		 Default: 48 SQL Type: INTEGER */
	long m_MaxSendRetryHoursLen;

	TIMESTAMP_STRUCT m_LastProcessInterval;	/* 
			Last time outbound mail was processed.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_LastProcessIntervalLen;

	int m_ProcessFreqMins;	/* 
			The frequency of SMTP processing intervals in minutes.
		 Default: 1 SQL Type: INTEGER */
	long m_ProcessFreqMinsLen;

	TCHAR m_Description[MESSAGEDESTINATIONS_DESCRIPTION_LENGTH];	/* 
			A human-readable name for convenience.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	int m_ServerID;	/* 
			The server responsible for this message destination.
		  */
	long m_ServerIDLen;

	unsigned char m_IsSSL;	/* 
			Is this an SSL connection.
		 Default: 0 SQL Type: BIT */
	long m_IsSSLLen;

	int m_SSLMode;	/* 
			The SSL mode to utilize (implicit, starttls, etc...) default 0.
		  */
	long m_SSLModeLen;

	int m_OfficeHours;	/*
		  Default: 0 SQL Type: INTEGER */
	long m_OfficeHoursLen;

	int m_ErrorCode;	/*
		  Default: 0 SQL Type: INTEGER */
	long m_ErrorCodeLen;

	TCHAR* m_AccessToken;	/* 
			OAuth2 Token.
		 SQL Type: TEXT */
	long m_AccessTokenAllocated;
	long m_AccessTokenLen;

	TIMESTAMP_STRUCT m_AccessTokenExpire;	/* 
			When the AccessToken expires.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_AccessTokenExpireLen;

	TCHAR* m_RefreshToken;	/* 
			OAuth2 Refresh Token.
		 SQL Type: TEXT */
	long m_RefreshTokenAllocated;
	long m_RefreshTokenLen;

	TIMESTAMP_STRUCT m_RefreshTokenExpire;	/* 
			When the RefreshToken expires.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_RefreshTokenExpireLen;

	int m_OAuthHostID;	/* 
			Does this MessageDestination use OAuth2.
		 Default: 0 SQL Type: INTEGER */
	long m_OAuthHostIDLen;	
	
} MessageDestinations_t;

inline bool operator==(const MessageDestinations_t& A,const MessageDestinations_t& B)
{ return A.m_MessageDestinationID == B.m_MessageDestinationID; }
inline bool operator!=(const MessageDestinations_t& A,const MessageDestinations_t& B)
{ return A.m_MessageDestinationID != B.m_MessageDestinationID; }

/* struct MessageSourceTypes_t - 
 	     Classifies message sources for routing purposes
	 */
typedef struct MessageSourceTypes_t
{
	MessageSourceTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(MessageSourceTypes_t) );
	}
	int m_MessageSourceTypeID;	/* 
			Uniquely identifies a message source type
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MessageSourceTypeIDLen;

	TCHAR m_Description[MESSAGESOURCETYPES_DESCRIPTION_LENGTH];	/* 
			Describes the message source type. 
			Suitable for use in a pull-down.
		 SQL Type: VARCHAR(32) */
	long m_DescriptionLen;

} MessageSourceTypes_t;

inline bool operator==(const MessageSourceTypes_t& A,const MessageSourceTypes_t& B)
{ return A.m_MessageSourceTypeID == B.m_MessageSourceTypeID; }
inline bool operator!=(const MessageSourceTypes_t& A,const MessageSourceTypes_t& B)
{ return A.m_MessageSourceTypeID != B.m_MessageSourceTypeID; }

/* struct MessageSources_t - 
		Designates sources for email messages.
	 */
typedef struct MessageSources_t
{
	MessageSources_t() // default constructor
	{
		ZeroMemory( this, sizeof(MessageSources_t) );
		m_AccessTokenAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_AccessToken = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_RefreshTokenAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_RefreshToken = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_RemotePort = 110;
		m_IsActive = 1;
		m_CheckFreqMins = 2;
		m_MaxInboundMsgSize = 5000;
		m_ConnTimeoutSecs = 300;
		m_LastCheckedLen = SQL_NULL_DATA;
		m_LeaveCopiesOnServer = 1;
		m_ZipAttach = 0;
		m_DupMsg = 0;
		m_LeaveCopiesDays = 0;
		m_SkipDownloadDays = 7;
		m_DateFilters = 0;
		m_ErrorCode = 0;
		m_AccessTokenExpireLen = SQL_NULL_DATA;
		m_RefreshTokenExpireLen = SQL_NULL_DATA;
		m_OAuthHostID = 0;
	}
	~MessageSources_t() // beware! nonvirtual destructor
	{
		if (m_AccessTokenAllocated > 0) free( m_AccessToken );
		if (m_RefreshTokenAllocated > 0) free( m_RefreshToken );
	}
	MessageSources_t( const MessageSources_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(MessageSources_t));
		if (m_AccessTokenAllocated > 0)
		{
			m_AccessToken = (TCHAR*)calloc( m_AccessTokenAllocated, 1 );
			memcpy(m_AccessToken,i.m_AccessToken,m_AccessTokenAllocated);
		}
		if (m_RefreshTokenAllocated > 0)
		{
			m_RefreshToken = (TCHAR*)calloc( m_RefreshTokenAllocated, 1 );
			memcpy(m_RefreshToken,i.m_RefreshToken,m_RefreshTokenAllocated);
		}
	}
	MessageSources_t& operator=(const MessageSources_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_AccessTokenAllocated > 0) free( m_AccessToken );
			if (m_RefreshTokenAllocated > 0) free( m_RefreshToken );
			memcpy(this,&i,sizeof(MessageSources_t));
			if (m_AccessTokenAllocated > 0)
			{
				m_AccessToken = (TCHAR*)calloc( m_AccessTokenAllocated, 1 );
				memcpy(m_AccessToken,i.m_AccessToken,m_AccessTokenAllocated);
			}
			if (m_RefreshTokenAllocated > 0)
			{
				m_RefreshToken = (TCHAR*)calloc( m_RefreshTokenAllocated, 1 );
				memcpy(m_RefreshToken,i.m_RefreshToken,m_RefreshTokenAllocated);
			}
		}
		return *this; // return reference to self
	}	
	
	int m_MessageSourceID;	/* 
			Uniquely identifies message sources.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MessageSourceIDLen;

	TCHAR m_RemoteAddress[MESSAGESOURCES_REMOTEADDRESS_LENGTH];	/* 
			Remote server address (ip or host name).
		 SQL Type: VARCHAR(255) */
	long m_RemoteAddressLen;

	int m_RemotePort;	/* 
			Remote server message retrieval port.
		 Default: 110 SQL Type: INTEGER */
	long m_RemotePortLen;

	TCHAR m_AuthUserName[MESSAGESOURCES_AUTHUSERNAME_LENGTH];	/* 
			Authentication username.
		 SQL Type: VARCHAR(125) */
	long m_AuthUserNameLen;

	TCHAR m_AuthPassword[MESSAGESOURCES_AUTHPASSWORD_LENGTH];	/* 
			Authentication password.
		 SQL Type: VARCHAR(125) */
	long m_AuthPasswordLen;

	unsigned char m_IsAPOP;	/* 
			Is this an APOP account (yes/no).
		 Default: 0 SQL Type: BIT */
	long m_IsAPOPLen;

	unsigned char m_LeaveCopiesOnServer;	/* 
			Should message copies be left on remote server?
		 Default: 0 SQL Type: BIT */
	long m_LeaveCopiesOnServerLen;

	unsigned char m_IsActive;	/* 
			Is this message source active?
		 Default: 1 SQL Type: BIT */
	long m_IsActiveLen;

	int m_CheckFreqMins;	/* 
			How frequently to check msg source (in minutes).
		 Default: 2 SQL Type: INTEGER */
	long m_CheckFreqMinsLen;

	int m_MaxInboundMsgSize;	/* 
			Maximum inbound message size (in K).
		 Default: 5000 SQL Type: INTEGER */
	long m_MaxInboundMsgSizeLen;

	int m_ConnTimeoutSecs;	/* 
			Remote host conneciton timeout (in seconds).
		 Default: 300 SQL Type: INTEGER */
	long m_ConnTimeoutSecsLen;

	TIMESTAMP_STRUCT m_LastChecked;	/* 
			Last time this message source was checked (NULL for never).
		 ALLOW NULLS SQL Type: DATETIME */
	long m_LastCheckedLen;

	int m_MessageDestinationID;	/* 
			The outbound message destination this message source should utilize.
		 Default: 0 SQL Type: INTEGER */
	long m_MessageDestinationIDLen;

	int m_MessageSourceTypeID;	/* 
			Type of message source.
		 Default: 0 SQL Type: INTEGER */
	long m_MessageSourceTypeIDLen;

	TCHAR m_Description[MESSAGESOURCES_DESCRIPTION_LENGTH];	/* 
			A human-readable name for convenience.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	unsigned char m_UseReplyTo;	/* 
			Use "Reply To:" email address as contact address?
		 Default: 1 SQL Type: BIT */
	long m_UseReplyToLen;

	int m_ServerID;	/* 
			The server responsible for this message destination.
		  */
	long m_ServerIDLen;

	unsigned char m_IsSSL;	/* 
			Is this an SSL connection.
		 Default: 0 SQL Type: BIT */
	long m_IsSSLLen;

	int m_OfficeHours;	/*
		  Default: 0 SQL Type: INTEGER */
	long m_OfficeHoursLen;

	int m_ZipAttach;	/*
		  Default: 0 SQL Type: INTEGER */
	long m_ZipAttachLen;

	int m_DupMsg;	/*
		  Default: 0 SQL Type: INTEGER */
	long m_DupMsgLen;

	int m_LeaveCopiesDays;	/*
		  Default: 0 SQL Type: INTEGER */
	long m_LeaveCopiesDaysLen;

	int m_SkipDownloadDays;	/*
		  Default: 7 SQL Type: INTEGER */
	long m_SkipDownloadDaysLen;

	int m_DateFilters;	/*
		  Default: 0 SQL Type: INTEGER */
	long m_DateFiltersLen;

	int m_ErrorCode;	/*
		  Default: 0 SQL Type: INTEGER */
	long m_ErrorCodeLen;

	TCHAR* m_AccessToken;	/* 
			OAuth2 Token.
		 SQL Type: TEXT */
	long m_AccessTokenAllocated;
	long m_AccessTokenLen;

	TIMESTAMP_STRUCT m_AccessTokenExpire;	/* 
			When the AccessToken expires.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_AccessTokenExpireLen;

	TCHAR* m_RefreshToken;	/* 
			OAuth2 Refresh Token.
		 SQL Type: TEXT */
	long m_RefreshTokenAllocated;
	long m_RefreshTokenLen;

	TIMESTAMP_STRUCT m_RefreshTokenExpire;	/* 
			When the RefreshToken expires.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_RefreshTokenExpireLen;

	int m_OAuthHostID;	/* 
			Does this MessageSource use OAuth2.
		 Default: 0 SQL Type: INTEGER */
	long m_OAuthHostIDLen;	
	
} MessageSources_t;

inline bool operator==(const MessageSources_t& A,const MessageSources_t& B)
{ return A.m_MessageSourceID == B.m_MessageSourceID; }
inline bool operator!=(const MessageSources_t& A,const MessageSources_t& B)
{ return A.m_MessageSourceID != B.m_MessageSourceID; }

/* struct MsgApprovalQueue_t - 
		Queues message approval requests.
	 */
typedef struct MsgApprovalQueue_t
{
	MsgApprovalQueue_t() // default constructor
	{
		ZeroMemory( this, sizeof(MsgApprovalQueue_t) );
		m_DateRequestedLen = SQL_NULL_DATA;
		m_DateHandledLen = SQL_NULL_DATA;
		m_NoteAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Note = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
	}
	~MsgApprovalQueue_t() // beware! nonvirtual destructor
	{
		if (m_NoteAllocated > 0) free( m_Note );
	}
	MsgApprovalQueue_t( const MsgApprovalQueue_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(MsgApprovalQueue_t));
		if (m_NoteAllocated > 0)
		{
			m_Note = (TCHAR*)calloc( m_NoteAllocated, 1 );
			memcpy(m_Note,i.m_Note,m_NoteAllocated);
		}
	}
	MsgApprovalQueue_t& operator=(const MsgApprovalQueue_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_NoteAllocated > 0) free( m_Note );
			memcpy(this,&i,sizeof(MsgApprovalQueue_t));
			if (m_NoteAllocated > 0)
			{
				m_Note = (TCHAR*)calloc( m_NoteAllocated, 1 );
				memcpy(m_Note,i.m_Note,m_NoteAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_MsgApprovalQueueID;	/* 
			Uniquely identifies a message queued for approval.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MsgApprovalQueueIDLen;

	int m_OutboundMessageID;	/* 
			The ID of the outbound message waiting for approval.
		 Default: 0 SQL Type: INTEGER */
	long m_OutboundMessageIDLen;

	TIMESTAMP_STRUCT m_DateRequested;	/* 
			Date/time the approval was requested.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateRequestedLen;

	TIMESTAMP_STRUCT m_DateHandled;	/* 
			Date/time the approval was handled (by approver)
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateHandledLen;

	unsigned char m_IsApproved;	/* 
			Whether the message is approved or not.
		 Default: 0 SQL Type: BIT */
	long m_IsApprovedLen;

	int m_RequestAgentID;	/* 
			Agent requesting message approval.
		 Default: 0 SQL Type: INTEGER */
	long m_RequestAgentIDLen;

	int m_ApproverAgentID;	/* 
			Agent that actually handled the approval.
		 Default: 0 SQL Type: INTEGER */
	long m_ApproverAgentIDLen;

	TCHAR* m_Note;	/* 
			Note applicable to this action.
		 SQL Type: TEXT */
	long m_NoteAllocated;
 	long m_NoteLen;

} MsgApprovalQueue_t;

inline bool operator==(const MsgApprovalQueue_t& A,const MsgApprovalQueue_t& B)
{ return A.m_MsgApprovalQueueID == B.m_MsgApprovalQueueID; }
inline bool operator!=(const MsgApprovalQueue_t& A,const MsgApprovalQueue_t& B)
{ return A.m_MsgApprovalQueueID != B.m_MsgApprovalQueueID; }

/* struct ObjectTypes_t - 
		Object types describe the various types of securable objects in the system.
	 */
typedef struct ObjectTypes_t
{
	ObjectTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(ObjectTypes_t) );
	}
	int m_ObjectTypeID;	/* 
			Uniquely identifies the class of objects.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ObjectTypeIDLen;

	TCHAR m_Description[OBJECTTYPES_DESCRIPTION_LENGTH];	/* 
			Descritive text for this object type
		 SQL Type: VARCHAR(125) */
	long m_DescriptionLen;

} ObjectTypes_t;

inline bool operator==(const ObjectTypes_t& A,const ObjectTypes_t& B)
{ return A.m_ObjectTypeID == B.m_ObjectTypeID; }
inline bool operator!=(const ObjectTypes_t& A,const ObjectTypes_t& B)
{ return A.m_ObjectTypeID != B.m_ObjectTypeID; }

/* struct Objects_t - 
		Securable objects.
	 */
typedef struct Objects_t
{
	Objects_t() // default constructor
	{
		ZeroMemory( this, sizeof(Objects_t) );
		m_UseDefaultRights = 1;
		m_DateCreatedLen = SQL_NULL_DATA;
	}
	int m_ObjectID;	/* 
			Uniquely identifies objects
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ObjectIDLen;

	int m_ActualID;	/* 
			The actual object ID as it relates to a specific object type.
		 Default: 0 SQL Type: INTEGER */
	long m_ActualIDLen;

	int m_ObjectTypeID;	/* 
			The type of object.
		 Default: 0 SQL Type: INTEGER */
	long m_ObjectTypeIDLen;

	unsigned char m_BuiltIn;	/* 
			Is this a built-in object?
		 Default: 0 SQL Type: BIT */
	long m_BuiltInLen;

	unsigned char m_UseDefaultRights;	/* 
			Use the default rights for the object type.
		 Default: 1 SQL Type: TINYINT */
	long m_UseDefaultRightsLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			Date/time the object was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

} Objects_t;

inline bool operator==(const Objects_t& A,const Objects_t& B)
{ return A.m_ObjectID == B.m_ObjectID; }
inline bool operator!=(const Objects_t& A,const Objects_t& B)
{ return A.m_ObjectID != B.m_ObjectID; }


/* struct NoteAttachments_t - 
		Attachments to notes.
	 */
typedef struct NoteAttachments_t
{
	NoteAttachments_t() // default constructor
	{
		ZeroMemory( this, sizeof(NoteAttachments_t) );
	}
	int m_NoteAttachmentID;	/* 
			Uniquely identifies the note attachment
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_NoteAttachmentIDLen;

	int m_AttachmentID;	/* 
			The ID of the attachment
		 Default: 0 SQL Type: INTEGER */
	long m_AttachmentIDLen;

	int m_NoteID;	/* 
			The ID of the note.
		 Default: 0 SQL Type: INTEGER */
	long m_NoteIDLen;

	int m_NoteTypeID;	/* 
			The type of the note.
		 Default: 0 SQL Type: INTEGER */
	long m_NoteTypeIDLen;

} NoteAttachments_t;

inline bool operator==(const NoteAttachments_t& A,const NoteAttachments_t& B)
{ return A.m_NoteAttachmentID == B.m_NoteAttachmentID; }
inline bool operator!=(const NoteAttachments_t& A,const NoteAttachments_t& B)
{ return A.m_NoteAttachmentID != B.m_NoteAttachmentID; }


/* struct OutboundMessageAttachments_t - 
		Attachments to outbound messages.
	 */
typedef struct OutboundMessageAttachments_t
{
	OutboundMessageAttachments_t() // default constructor
	{
		ZeroMemory( this, sizeof(OutboundMessageAttachments_t) );
	}
	int m_OutboundMessageAttachmentID;	/* 
			Uniquely identifies the outbound message attachment
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_OutboundMessageAttachmentIDLen;

	int m_AttachmentID;	/* 
			The ID of the attachment
		 Default: 0 SQL Type: INTEGER */
	long m_AttachmentIDLen;

	int m_OutboundMessageID;	/* 
			The ID of the outbound message.
		 Default: 0 SQL Type: INTEGER */
	long m_OutboundMessageIDLen;

} OutboundMessageAttachments_t;

inline bool operator==(const OutboundMessageAttachments_t& A,const OutboundMessageAttachments_t& B)
{ return A.m_OutboundMessageAttachmentID == B.m_OutboundMessageAttachmentID; }
inline bool operator!=(const OutboundMessageAttachments_t& A,const OutboundMessageAttachments_t& B)
{ return A.m_OutboundMessageAttachmentID != B.m_OutboundMessageAttachmentID; }

/* struct OutboundMessageContacts_t - 
		Relates ticket boxes to contacts.
	 */
typedef struct OutboundMessageContacts_t
{
	OutboundMessageContacts_t() // default constructor
	{
		ZeroMemory( this, sizeof(OutboundMessageContacts_t) );
	}
	int m_OutboundMessageContactID;	/* 
			Uniquely identifies an entry in the table.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_OutboundMessageContactIDLen;

	int m_OutboundMessageID;	/* 
			Identifies the outbound message.
		 Default: 0 SQL Type: INTEGER */
	long m_OutboundMessageIDLen;

	int m_ContactID;	/* 
			The contact that this outbound message was sent to.
		 Default: 0 SQL Type: INTEGER */
	long m_ContactIDLen;

} OutboundMessageContacts_t;

inline bool operator==(const OutboundMessageContacts_t& A,const OutboundMessageContacts_t& B)
{ return A.m_OutboundMessageContactID == B.m_OutboundMessageContactID; }
inline bool operator!=(const OutboundMessageContacts_t& A,const OutboundMessageContacts_t& B)
{ return A.m_OutboundMessageContactID != B.m_OutboundMessageContactID; }

/* struct OutboundMessageQueue_t - 
		Outbound message queue.
	 */
typedef struct OutboundMessageQueue_t
{
	OutboundMessageQueue_t() // default constructor
	{
		ZeroMemory( this, sizeof(OutboundMessageQueue_t) );
		m_DateSpooledLen = SQL_NULL_DATA;
		m_LastAttemptedDeliveryLen = SQL_NULL_DATA;
	}
	int m_OutBoundMessageQueueID;	/* 
			Uniquely identifies an entry in the outbound message queue.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_OutBoundMessageQueueIDLen;

	int m_OutboundMessageID;	/* 
			The message ID in the outbound messages table.
		 Default: 0 SQL Type: INTEGER */
	long m_OutboundMessageIDLen;

	int m_MessageDestinationID;	/* 
			The destination this outbound message will use.
		 Default: 0 SQL Type: INTEGER */
	long m_MessageDestinationIDLen;

	unsigned char m_SpoolFileGenerated;	/* 
			Has the spool file been generated?
		 Default: 0 SQL Type: BIT */
	long m_SpoolFileGeneratedLen;

	unsigned char m_IsApproved;	/* 
			Is message approved to leave the system?
		 Default: 0 SQL Type: BIT */
	long m_IsApprovedLen;

	TIMESTAMP_STRUCT m_DateSpooled;	/* 
			The date the outbound message was CREATED. Note: column name
			is misleading; this isn't the date/time the msg is spooled to the
			file system.
		 SQL Type: DATETIME */
	long m_DateSpooledLen;

	TIMESTAMP_STRUCT m_LastAttemptedDelivery;	/* 
			When message delivery was last attempted.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_LastAttemptedDeliveryLen;

} OutboundMessageQueue_t;

inline bool operator==(const OutboundMessageQueue_t& A,const OutboundMessageQueue_t& B)
{ return A.m_OutBoundMessageQueueID == B.m_OutBoundMessageQueueID; }
inline bool operator!=(const OutboundMessageQueue_t& A,const OutboundMessageQueue_t& B)
{ return A.m_OutBoundMessageQueueID != B.m_OutBoundMessageQueueID; }

/* struct OutboundMessageStates_t - 
		Message states will include the following:
		Draft, Sent, Untouched, Retry, Failed
	 */
typedef struct OutboundMessageStates_t
{
	OutboundMessageStates_t() // default constructor
	{
		ZeroMemory( this, sizeof(OutboundMessageStates_t) );
	}
	int m_OutboundMsgStateID;	/* 
			Uniquely identifes an outbound message state.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_OutboundMsgStateIDLen;

	TCHAR m_MessageStateName[OUTBOUNDMESSAGESTATES_MESSAGESTATENAME_LENGTH];	/* 
			Human-readable description of the outbound message state.
		 SQL Type: VARCHAR(50) */
	long m_MessageStateNameLen;

} OutboundMessageStates_t;

inline bool operator==(const OutboundMessageStates_t& A,const OutboundMessageStates_t& B)
{ return A.m_OutboundMsgStateID == B.m_OutboundMsgStateID; }
inline bool operator!=(const OutboundMessageStates_t& A,const OutboundMessageStates_t& B)
{ return A.m_OutboundMsgStateID != B.m_OutboundMsgStateID; }

/* struct OutboundMessageTypes_t - 
		Describes different types of outbound messages.
	 */
typedef struct OutboundMessageTypes_t
{
	OutboundMessageTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(OutboundMessageTypes_t) );
	}
	int m_OutboundMessageTypeID;	/* 
			Uniquely identifies the outbound message type.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_OutboundMessageTypeIDLen;

	TCHAR m_Description[OUTBOUNDMESSAGETYPES_DESCRIPTION_LENGTH];	/* 
			A human-readable description of the outbound message type.
		 SQL Type: VARCHAR(64) */
	long m_DescriptionLen;

} OutboundMessageTypes_t;

inline bool operator==(const OutboundMessageTypes_t& A,const OutboundMessageTypes_t& B)
{ return A.m_OutboundMessageTypeID == B.m_OutboundMessageTypeID; }
inline bool operator!=(const OutboundMessageTypes_t& A,const OutboundMessageTypes_t& B)
{ return A.m_OutboundMessageTypeID != B.m_OutboundMessageTypeID; }

/* struct OutboundMessages_t - 
		Messages created in the system.
	 */
typedef struct OutboundMessages_t
{
	OutboundMessages_t() // default constructor
	{
		ZeroMemory( this, sizeof(OutboundMessages_t) );
		m_EmailDateTimeLen = SQL_NULL_DATA;
		m_DeletedTimeLen = SQL_NULL_DATA;
		m_BodyAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Body = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_EmailToAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_EmailTo = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_EmailCcAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_EmailCc = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_EmailBccAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_EmailBcc = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_EmailReplyToAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_EmailReplyTo = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_PriorityID = 3;
	}
	~OutboundMessages_t() // beware! nonvirtual destructor
	{
		if (m_BodyAllocated > 0) free( m_Body );
		if (m_EmailToAllocated > 0) free( m_EmailTo );
		if (m_EmailCcAllocated > 0) free( m_EmailCc );
		if (m_EmailBccAllocated > 0) free( m_EmailBcc );
		if (m_EmailReplyToAllocated > 0) free( m_EmailReplyTo );
	}
	OutboundMessages_t( const OutboundMessages_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(OutboundMessages_t));
		if (m_BodyAllocated > 0)
		{
			m_Body = (TCHAR*)calloc( m_BodyAllocated, 1 );
			memcpy(m_Body,i.m_Body,m_BodyAllocated);
		}
		if (m_EmailToAllocated > 0)
		{
			m_EmailTo = (TCHAR*)calloc( m_EmailToAllocated, 1 );
			memcpy(m_EmailTo,i.m_EmailTo,m_EmailToAllocated);
		}
		if (m_EmailCcAllocated > 0)
		{
			m_EmailCc = (TCHAR*)calloc( m_EmailCcAllocated, 1 );
			memcpy(m_EmailCc,i.m_EmailCc,m_EmailCcAllocated);
		}
		if (m_EmailBccAllocated > 0)
		{
			m_EmailBcc = (TCHAR*)calloc( m_EmailBccAllocated, 1 );
			memcpy(m_EmailBcc,i.m_EmailBcc,m_EmailBccAllocated);
		}
		if (m_EmailReplyToAllocated > 0)
		{
			m_EmailReplyTo = (TCHAR*)calloc( m_EmailReplyToAllocated, 1 );
			memcpy(m_EmailReplyTo,i.m_EmailReplyTo,m_EmailReplyToAllocated);
		}
	}
	OutboundMessages_t& operator=(const OutboundMessages_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_BodyAllocated > 0) free( m_Body );
			if (m_EmailToAllocated > 0) free( m_EmailTo );
			if (m_EmailCcAllocated > 0) free( m_EmailCc );
			if (m_EmailBccAllocated > 0) free( m_EmailBcc );
			if (m_EmailReplyToAllocated > 0) free( m_EmailReplyTo );
			memcpy(this,&i,sizeof(OutboundMessages_t));
			if (m_BodyAllocated > 0)
			{
				m_Body = (TCHAR*)calloc( m_BodyAllocated, 1 );
				memcpy(m_Body,i.m_Body,m_BodyAllocated);
			}
			if (m_EmailToAllocated > 0)
			{
				m_EmailTo = (TCHAR*)calloc( m_EmailToAllocated, 1 );
				memcpy(m_EmailTo,i.m_EmailTo,m_EmailToAllocated);
			}
			if (m_EmailCcAllocated > 0)
			{
				m_EmailCc = (TCHAR*)calloc( m_EmailCcAllocated, 1 );
				memcpy(m_EmailCc,i.m_EmailCc,m_EmailCcAllocated);
			}
			if (m_EmailBccAllocated > 0)
			{
				m_EmailBcc = (TCHAR*)calloc( m_EmailBccAllocated, 1 );
				memcpy(m_EmailBcc,i.m_EmailBcc,m_EmailBccAllocated);
			}
			if (m_EmailReplyToAllocated > 0)
			{
				m_EmailReplyTo = (TCHAR*)calloc( m_EmailReplyToAllocated, 1 );
				memcpy(m_EmailReplyTo,i.m_EmailReplyTo,m_EmailReplyToAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_OutboundMessageID;	/* 
			Uniquely identifies the outbound message.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_OutboundMessageIDLen;

	int m_TicketID;	/* 
			The ticket the oubound message belongs to.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketIDLen;

	int m_TicketCategoryID;	/* 
			The ticket category the oubound message belongs to.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketCategoryIDLen;

	TCHAR m_EmailFrom[OUTBOUNDMESSAGES_EMAILFROM_LENGTH];	/* 
			The email address the message was sent from (inside the angle brackets).
		 SQL Type: VARCHAR(255) */
	long m_EmailFromLen;

	TCHAR m_EmailPrimaryTo[OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH];	/* 
			The primary email address the message was addressed to.
			Although this contains the same information as the EmailTo:,
			this column is suitable for building indices and used for display.
		 SQL Type: VARCHAR(255) */
	long m_EmailPrimaryToLen;

	TIMESTAMP_STRUCT m_EmailDateTime;	/* 
			The time when the message was placed in the outbound queue.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_EmailDateTimeLen;

	TCHAR m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH];	/* 
			The subject of the message.
		 SQL Type: VARCHAR(255) */
	long m_SubjectLen;

	TCHAR m_MediaType[OUTBOUNDMESSAGES_MEDIATYPE_LENGTH];	/* 
			MIME content type of attachment.
		 SQL Type: VARCHAR(125) */
	long m_MediaTypeLen;

	TCHAR m_MediaSubType[OUTBOUNDMESSAGES_MEDIASUBTYPE_LENGTH];	/* 
			MIME content sub-type of attachment.
		 SQL Type: VARCHAR(125) */
	long m_MediaSubTypeLen;

	int m_DeletedBy;	/* 
			The AgentID of the user that deleted this message.
		 Default: 0 SQL Type: INTEGER */
	long m_DeletedByLen;

	TIMESTAMP_STRUCT m_DeletedTime;	/* 
			The date/time the message was deleted.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DeletedTimeLen;

	unsigned char m_IsDeleted;	/* 
			TRUE if message deleted, FALSE if not.
		 Default: 0 SQL Type: TINYINT */
	long m_IsDeletedLen;

	int m_AgentID;	/* 
			The AgentID of the user that composed the message.
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_ReplyToMsgID;	/* 
			The inbound message this was in reply to.
		 Default: 0 SQL Type: INTEGER */
	long m_ReplyToMsgIDLen;

	unsigned char m_ReplyToIDIsInbound;	/* 
			Is ReplyToMsgID inbound or outbount?
		 Default: 0 SQL Type: BIT */
	long m_ReplyToIDIsInboundLen;

	int m_OutboundMessageTypeID;	/* 
			The the type of outbound message: reply, forward, new, etc.
		 Default: 0 SQL Type: INTEGER */
	long m_OutboundMessageTypeIDLen;

	int m_OutboundMessageStateID;	/* 
			The the type of outbound message: reply, forward, new, etc.
		 Default: 0 SQL Type: INTEGER */
	long m_OutboundMessageStateIDLen;

	TCHAR* m_Body;	/* 
			The body of the message.
		 SQL Type: TEXT */
	long m_BodyAllocated;
 	long m_BodyLen;

	TCHAR* m_EmailTo;	/* 
			The full list of to: addresses.
		 SQL Type: TEXT */
	long m_EmailToAllocated;
 	long m_EmailToLen;

	TCHAR* m_EmailCc;	/* 
			The cc field.
		 SQL Type: TEXT */
	long m_EmailCcAllocated;
 	long m_EmailCcLen;

	TCHAR* m_EmailBcc;	/* 
			The cc field.
		 SQL Type: TEXT */
	long m_EmailBccAllocated;
 	long m_EmailBccLen;

	TCHAR* m_EmailReplyTo;	/* 
			The reply-to field.
		 SQL Type: TEXT */
	long m_EmailReplyToAllocated;
 	long m_EmailReplyToLen;

	int m_TicketBoxHeaderID;	/* 
			The message header id from TicketBoxHeaders.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketBoxHeaderIDLen;

	int m_TicketBoxFooterID;	/* 
			The message footer id from TicketBoxFooters.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketBoxFooterIDLen;

	unsigned char m_FooterLocation;	/* 
			0 = Bottom, 1 = Above Original Message
		 Default: 0 SQL Type: TINYINT */
	long m_FooterLocationLen;

	int m_PriorityID;	/* 
			The priority of the message
		 Default: 3 SQL Type: INTEGER */
	long m_PriorityIDLen;

	int m_ArchiveID;	/* 
			If this message was archived, the corresponding archive ID.
			Otherwise 0.
		 Default: 0 SQL Type: INTEGER */
	long m_ArchiveIDLen;

	int m_SignatureID;	/* 
			ID of the signature to use with this message. (for drafts)
		 Default: 0 SQL Type: INTEGER */
	long m_SignatureIDLen;

	int m_OriginalTicketBoxID;	/* 
			The TicketBoxID of the ticket when the message was created.
		 Default: 0 SQL Type: INTEGER */
	long m_OriginalTicketBoxIDLen;

	unsigned char m_DraftCloseTicket;	/* 
			Stores the value of "Close ticket upon sending message" for drafts.
		 Default: 0 SQL Type: TINYINT */
	long m_DraftCloseTicketLen;

	unsigned char m_DraftRouteToMe;	/* 
			Stores the value of "Route Replies to My Tickets" for drafts.
		 Default: 0 SQL Type: TINYINT */
	long m_DraftRouteToMeLen;

	unsigned char m_ReadReceipt;	/* 
			Request Read Receipt.
		 Default: 0 SQL Type: TINYINT */
	long m_ReadReceiptLen;

	unsigned char m_DeliveryConfirmation;	/* 
			Delivery Confirmation.
		 Default: 0 SQL Type: TINYINT */
	long m_DeliveryConfirmationLen;

	int m_MultiMail;	/*
		 Default: 0 SQL Type: INTEGER */
	long m_MultiMailLen;

} OutboundMessages_t;

inline bool operator==(const OutboundMessages_t& A,const OutboundMessages_t& B)
{ return A.m_OutboundMessageID == B.m_OutboundMessageID; }
inline bool operator!=(const OutboundMessages_t& A,const OutboundMessages_t& B)
{ return A.m_OutboundMessageID != B.m_OutboundMessageID; }

/* struct PersonalData_t - 
		Contains personal information such as email addresses, phone numbers
		and custom data that is related to a specific contact or agent. 
	 */
typedef struct PersonalData_t
{
	PersonalData_t() // default constructor
	{
		ZeroMemory( this, sizeof(PersonalData_t) );
	}
	int m_PersonalDataID;	/* 
			Uniquely identifies a piece of personal data.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_PersonalDataIDLen;

	int m_ContactID;	/* 
			ContactID this applies to. (mutually exclusive w/ AgentID)
		 Default: 0 SQL Type: INTEGER */
	long m_ContactIDLen;

	int m_AgentID;	/* 
			AgentID data applies to. (mutually exclusive w/ ContactID)
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_PersonalDataTypeID;	/* 
			The type of contact data (e.g. phone, email, etc.)
		 Default: 0 SQL Type: INTEGER */
	long m_PersonalDataTypeIDLen;

	TCHAR m_DataValue[PERSONALDATA_DATAVALUE_LENGTH];	/* 
			The contact data itself.
		 SQL Type: VARCHAR(255) */
	long m_DataValueLen;

	TCHAR m_Note[PERSONALDATA_NOTE_LENGTH];	/* 
			Note.
		 SQL Type: VARCHAR(255) */
	long m_NoteLen;

	int m_StatusID;	/*			
		 Default: 0 SQL Type: INTEGER */
	long m_StatusIDLen;

	TIMESTAMP_STRUCT m_StatusDate;	/*			
		 Default: GetDate() No NULLS SQL Type: DATETIME */
	long m_StatusDateLen;

} PersonalData_t;

inline bool operator==(const PersonalData_t& A,const PersonalData_t& B)
{ return A.m_PersonalDataID == B.m_PersonalDataID; }
inline bool operator!=(const PersonalData_t& A,const PersonalData_t& B)
{ return A.m_PersonalDataID != B.m_PersonalDataID; }

/* struct PersonalDataTypes_t - 
		Types of personal data such as email, business phone, ICQ# etc.
	 */
typedef struct PersonalDataTypes_t
{
	PersonalDataTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(PersonalDataTypes_t) );
	}

	int m_PersonalDataTypeID;	/* 
			Uniquely identifies a personal data type.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_PersonalDataTypeIDLen;

	TCHAR m_TypeName[PERSONALDATATYPES_TYPENAME_LENGTH];	/* 
			Human-readable name of personal data type.
		 SQL Type: VARCHAR(64) */
	long m_TypeNameLen;

	unsigned char m_BuiltIn;	/* 
			If the contact type is "built-in", it's required by the system 
			(don't allow it to be deleted or edited).
		 Default: 0 SQL Type: BIT */
	long m_BuiltInLen;

} PersonalDataTypes_t;

inline bool operator==(const PersonalDataTypes_t& A,const PersonalDataTypes_t& B)
{ return A.m_PersonalDataTypeID == B.m_PersonalDataTypeID; }
inline bool operator!=(const PersonalDataTypes_t& A,const PersonalDataTypes_t& B)
{ return A.m_PersonalDataTypeID != B.m_PersonalDataTypeID; }

/* struct Bypass_t - 
		Contains items that will be bypassed by various functions. 
	 */
typedef struct Bypass_t
{
	Bypass_t() // default constructor
	{
		ZeroMemory( this, sizeof(Bypass_t) );
	}
	int m_BypassID;	/* 
			Uniquely identifies a bypass item.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_BypassIDLen;

	int m_BypassTypeID;	/* 
			Default: 0 SQL Type: INTEGER */
	long m_BypassTypeIDLen;

	TCHAR m_BypassValue[PERSONALDATA_DATAVALUE_LENGTH];	/* 
			The bypass value.
		 SQL Type: VARCHAR(255) */
	long m_BypassValueLen;

	TIMESTAMP_STRUCT m_DateCreated;	/*			
		 Default: GetDate() No NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

} Bypass_t;

inline bool operator==(const Bypass_t& A,const Bypass_t& B)
{ return A.m_BypassID == B.m_BypassID; }
inline bool operator!=(const Bypass_t& A,const Bypass_t& B)
{ return A.m_BypassID != B.m_BypassID; }

/* struct Email_t - 
		Contains items that will be bypassed by various functions. 
	 */
typedef struct Email_t
{
	Email_t() // default constructor
	{
		ZeroMemory( this, sizeof(Email_t) );
	}
	int m_EmailID;	/* 
			Uniquely identifies a Email item.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_EmailIDLen;

	int m_EmailTypeID;	/* 
			Default: 0 SQL Type: INTEGER */
	long m_EmailTypeIDLen;

	TCHAR m_EmailValue[PERSONALDATA_DATAVALUE_LENGTH];	/* 
			The Email value.
		 SQL Type: VARCHAR(255) */
	long m_EmailValueLen;

	TIMESTAMP_STRUCT m_DateCreated;	/*			
		 Default: GetDate() No NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

} Email_t;

inline bool operator==(const Email_t& A,const Email_t& B)
{ return A.m_EmailID == B.m_EmailID; }
inline bool operator!=(const Email_t& A,const Email_t& B)
{ return A.m_EmailID != B.m_EmailID; }

/* struct TicketCategories_t - 
		Ticket categories.
	 */
typedef struct TicketCategories_t
{
	TicketCategories_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketCategories_t) );
	}
	int m_TicketCategoryID;	/* 
			Uniquely identifies a ticket category.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketCategoryIDLen;

	TCHAR m_Description[TICKETCATEGORIES_DESCRIPTION_LENGTH];	/* 
			Human-readable name of ticket category.
		 SQL Type: VARCHAR(64) */
	long m_DescriptionLen;

	int m_ObjectID;	/* 
			Security ObjectID.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ObjectIDLen;

	unsigned char m_BuiltIn;	/* 
			If the contact type is "built-in", it's required by the system 
			(don't allow it to be deleted or edited).
		 Default: 0 SQL Type: BIT */
	long m_BuiltInLen;

} TicketCategories_t;

inline bool operator==(const TicketCategories_t& A,const TicketCategories_t& B)
{ return A.m_TicketCategoryID == B.m_TicketCategoryID; }
inline bool operator!=(const TicketCategories_t& A,const TicketCategories_t& B)
{ return A.m_TicketCategoryID != B.m_TicketCategoryID; }


/* struct TicketBoxOwners_t - 
		TicketBox Owners.
	 */
typedef struct TicketBoxOwners_t
{
	TicketBoxOwners_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketBoxOwners_t) );
	}
	int m_TicketBoxOwnerID;	/* 
			Uniquely identifies a ticketbox owner.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxOwnerIDLen;

	TCHAR m_Description[TICKETBOXOWNERS_DESCRIPTION_LENGTH];	/* 
			Human-readable name of ticketbox owner.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;

} TicketBoxOwners_t;

inline bool operator==(const TicketBoxOwners_t& A,const TicketBoxOwners_t& B)
{ return A.m_TicketBoxOwnerID == B.m_TicketBoxOwnerID; }
inline bool operator!=(const TicketBoxOwners_t& A,const TicketBoxOwners_t& B)
{ return A.m_TicketBoxOwnerID != B.m_TicketBoxOwnerID; }

/* struct TicketBoxTicketBoxOwners_t - 
		TicketBox Owners.
	 */
typedef struct TicketBoxTicketBoxOwners_t
{
	TicketBoxTicketBoxOwners_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketBoxTicketBoxOwners_t) );
	}
	int m_TbTboID;	/* 
			Uniquely identifies a ticketbox owner.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TbTboIDLen;

	int m_TicketBoxID;	/* 
			Uniquely identifies a ticketbox owner.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	int m_TicketBoxOwnerID;	/* 
			Uniquely identifies a ticketbox owner.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxOwnerIDLen;	

} TicketBoxTicketBoxOwners_t;

inline bool operator==(const TicketBoxTicketBoxOwners_t& A,const TicketBoxTicketBoxOwners_t& B)
{ return A.m_TbTboID == B.m_TbTboID; }
inline bool operator!=(const TicketBoxTicketBoxOwners_t& A,const TicketBoxTicketBoxOwners_t& B)
{ return A.m_TbTboID != B.m_TbTboID; }


/* struct TicketBoxRouting_t - 
		TicketBox Routing.
	 */
typedef struct TicketBoxRouting_t
{
	TicketBoxRouting_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketBoxRouting_t) );
	}
	int m_TicketBoxRoutingID;	/* 
			Uniquely identifies a ticketbox owner.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxRoutingIDLen;

	int m_TicketBoxID;	/* 
			Uniquely identifies a ticketbox owner.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	int m_RoutingRuleID;	/* 
			Uniquely identifies a ticketbox owner.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_RoutingRuleIDLen;

	int m_Percentage;	/* 
			Uniquely identifies a ticketbox owner.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_PercentageLen;

} TicketBoxRouting_t;

inline bool operator==(const TicketBoxRouting_t& A,const TicketBoxRouting_t& B)
{ return A.m_TicketBoxRoutingID == B.m_TicketBoxRoutingID; }
inline bool operator!=(const TicketBoxRouting_t& A,const TicketBoxRouting_t& B)
{ return A.m_TicketBoxRoutingID != B.m_TicketBoxRoutingID; }


/* struct Priorities_t - 
		Priority levels assignable to tickets and messages.
		Examples include: low, medium-low, medium, medium-high, high.
	 */
typedef struct Priorities_t
{
	Priorities_t() // default constructor
	{
		ZeroMemory( this, sizeof(Priorities_t) );
	}
	int m_PriorityID;	/* 
			Uniquely identifies the priority.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_PriorityIDLen;

	TCHAR m_Name[PRIORITIES_NAME_LENGTH];	/* 
		 SQL Type: VARCHAR(50) */
	long m_NameLen;

	int m_SortIndex;	/* 
			Index to be used for sorting.
		 Default: 0 SQL Type: INTEGER */
	long m_SortIndexLen;

} Priorities_t;

inline bool operator==(const Priorities_t& A,const Priorities_t& B)
{ return A.m_PriorityID == B.m_PriorityID; }
inline bool operator!=(const Priorities_t& A,const Priorities_t& B)
{ return A.m_PriorityID != B.m_PriorityID; }

/* struct RoutingRules_t - 
		Message routing rules.
	 */
typedef struct RoutingRules_t
{
	RoutingRules_t() // default constructor
	{
		ZeroMemory( this, sizeof(RoutingRules_t) );
		m_IsEnabled = 1;
		m_AutoReplyQuoteMsg = 1;
		m_ForwardFromContact = 1;
		m_AssignToAgentAlg = 4;
		m_AssignToTicketBoxAlg = 1;
	}
	int m_RoutingRuleID;	/* 
			Uniquely identifies a routing rule.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_RoutingRuleIDLen;

	TCHAR m_RuleDescrip[ROUTINGRULES_RULEDESCRIP_LENGTH];	/* 
			Short description of rule.
		 SQL Type: VARCHAR(125) */
	long m_RuleDescripLen;

	unsigned char m_IsEnabled;	/* 
			Is this rule enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	int m_OrderIndex;	/* 
			Allows for ordering of rules.
		 Default: 0 SQL Type: INTEGER */
	long m_OrderIndexLen;

	unsigned char m_AutoReplyEnable;	/* 
			Enable auto-reply
		 Default: 0 SQL Type: BIT */
	long m_AutoReplyEnableLen;

	int m_AutoReplyWithStdResponse;	/* 
			Auto-reply with a std reponse ID
		 Default: 0 SQL Type: INTEGER */
	long m_AutoReplyWithStdResponseLen;

	unsigned char m_AutoReplyQuoteMsg;	/* 
			When Auto-replying, quote original message in outbound message?
		 Default: 1 SQL Type: BIT */
	long m_AutoReplyQuoteMsgLen;

	unsigned char m_AutoReplyCloseTicket;	/* 
			When Auto-Replying, should the ticket be left open?
		 Default: 0 SQL Type: BIT */
	long m_AutoReplyCloseTicketLen;

	unsigned char m_ForwardEnable;	/* 
			Enable forwarding
		 Default: 0 SQL Type: BIT */
	long m_ForwardEnableLen;

	int m_ForwardFromAgent;	/* 
			If forwarded, show as from this agent.
		 Default: 0 SQL Type: INTEGER */
	long m_ForwardFromAgentLen;

	unsigned char m_ForwardFromContact;	/* 
			If forwarded, show as from original contact?
		 Default: 1 SQL Type: BIT */
	long m_ForwardFromContactLen;

	int m_AssignToTicketBox;	/* 
			Assign to this ticket box.
		 Default: 0 SQL Type: INTEGER */
	long m_AssignToTicketBoxLen;

	int m_AssignToTicketCategory;	/* 
			Assign to this ticket category.
		 Default: 0 SQL Type: INTEGER */
	long m_AssignToTicketCategoryLen;

	unsigned char m_AssignToAgentEnable;	/* 
			If true, new tickets are assigned to the agent specified by AssignToAgent
		 Default: 0 SQL Type: BIT */
	long m_AssignToAgentEnableLen;

	unsigned char m_AlertEnable;	/* 
			If true, an alert will be sent to the AlertToAgent
		 Default: 0 SQL Type: BIT */
	long m_AlertEnableLen;

	unsigned char m_AlertIncludeSubject;	/* 
			If true, the ticket subject will be included in the alert
		 Default: 0 SQL Type: BIT */
	long m_AlertIncludeSubjectLen;

	int m_AlertToAgentID;	/* 
			Alert to this agent.
		 Default: 0 SQL Type: INTEGER */
	long m_AlertToAgentIDLen;

	TCHAR m_AlertText[ROUTINGRULES_ALERTTEXT_LENGTH];	/* 
			Text to include in alert.
		 SQL Type: VARCHAR(255) */
	long m_AlertTextLen;

	unsigned char m_ToOrFrom;	/* 
			If true, the to or from email address will be used
		 Default: 0 SQL Type: BIT */
	long m_ToOrFromLen;

	int m_AssignToAgent;	/* 
			Assign to this agent.
		 Default: 0 SQL Type: INTEGER */
	long m_AssignToAgentLen;

	unsigned char m_DeleteImmediatelyEnable;	/* 
			Enables DeleteImmediately processing
		 Default: 0 SQL Type: BIT */
	long m_DeleteImmediatelyEnableLen;

	unsigned char m_DeleteImmediately;	/* 
			Delete immediately (0 = off, 1 = route to trash, 2 = shred now, bypassing trash)
		 Default: 0 SQL Type: TINYINT */
	long m_DeleteImmediatelyLen;

	int m_HitCount;	/* 
			Number of times this routing rule has been employed (since counter was last reset)
		 Default: 0 SQL Type: INTEGER */
	long m_HitCountLen;

	int m_PriorityID;	/* 
			The priority to apply to messages that match this rule.
		 Default: 0 SQL Type: INTEGER */
	long m_PriorityIDLen;

	int m_MessageSourceTypeID;	/* 
			Originally, this was: Only match this message source type or 0 for any.
			It was changed to be the MessageSourceID to route for, or 0 to match any.
		 Default: 0 SQL Type: INTEGER */
	long m_MessageSourceTypeIDLen;

	TCHAR m_AutoReplyFrom[ROUTINGRULES_AUTOREPLYFROM_LENGTH];	/* 
			From: email address for auto-replies
		 SQL Type: VARCHAR(255) */
	long m_AutoReplyFromLen;

	int m_AssignToAgentAlg;	/* 
			Algorithm used do assign tickets to agents.
			0 - Owner = AssignToAgent
			1 - Owner = Agent w/ matching email, else AssignToAgent
			2 - Owner = Contact owner, else AssignToAgent
			3 - Owner = Agent w/ matching email, Contact owner, else AssignToAgent
			4 - Owner = Contact owner, Agent w/ matching email, else AssignToAgent
		 Default: 4 SQL Type: INTEGER */
	long m_AssignToAgentAlgLen;

	int m_AssignToTicketBoxAlg;	/* 
			Algorithm used to assign tickets to ticketboxes.
			0 = TicketBox = AssignToTicketBox
			1 = TicketBox = TicketBox w/ matching email address, else AssignToTicketBox
		 Default: 1 SQL Type: INTEGER */
	long m_AssignToTicketBoxAlgLen;

	unsigned char m_ConsiderAllOwned;
	long m_ConsiderAllOwnedLen;
	
	unsigned char m_AssignUniqueTicketID;
	long m_AssignUniqueTicketIDLen;

	unsigned char m_DoProcessingRules;
	long m_DoProcessingRulesLen;

	int m_LastOwnerID;	/* 
			Records the last Agent assigned as Owner when using Round Robin
		 Default: 0 SQL Type: INTEGER */
	long m_LastOwnerIDLen;

	int m_MatchMethod;	/* 
			Which method to use to search for matching Ticket
		 Default: 0 SQL Type: INTEGER */
	long m_MatchMethodLen;

	TCHAR m_ForwardFromEmail[ROUTINGRULES_RULEDESCRIP_LENGTH];	/* 
			Forward from email address.
		 SQL Type: VARCHAR(125) */
	long m_ForwardFromEmailLen;

	TCHAR m_ForwardFromName[ROUTINGRULES_RULEDESCRIP_LENGTH];	/* 
			Forward from name.
		 SQL Type: VARCHAR(125) */
	long m_ForwardFromNameLen;

	unsigned char m_ForwardInTicket;	/* 
			Place Forwarded Message in Ticket
		 Default: 1 SQL Type: BIT */
	long m_ForwardInTicketLen;	

	unsigned char m_AutoReplyInTicket;	/* 
			Place Auto Reply in Ticket
		 Default: 1 SQL Type: BIT */
	long m_AutoReplyInTicketLen;

	unsigned char m_AllowRemoteReply;	/* 
			Allow remote replies from email clients?
		 Default: 1 SQL Type: BIT */
	long m_AllowRemoteReplyLen;

	int m_DoNotAssign;	/* 
			6 bit binary for do not assign
			Right to Left Online=32, Away=16, NotAvail=8, DND=4, Offline=2, Out of Office=1
			None selected would be 0
			All selected would be 63
		 Default: 0 SQL Type: INTEGER */
	long m_DoNotAssignLen;

	unsigned char m_QuoteOriginal;	/* 
			Quote Forwarded Message
		 Default: 0 SQL Type: BIT */
	long m_QuoteOriginalLen;

	int m_MultiMail;	/*			
		 Default: 0 SQL Type: INTEGER */
	long m_MultiMailLen;

	int m_SetOpenOwner;	/*			
		 Default: 0 SQL Type: INTEGER */
	long m_SetOpenOwnerLen;

	int m_OfficeHours;	/*			
		 Default: 0 SQL Type: INTEGER */
	long m_OfficeHoursLen;

	int m_IgnoreTracking;	/*			
		 Default: 0 SQL Type: INTEGER */
	long m_IgnoreTrackingLen;

} RoutingRules_t;

inline bool operator==(const RoutingRules_t& A,const RoutingRules_t& B)
{ return A.m_RoutingRuleID == B.m_RoutingRuleID; }
inline bool operator!=(const RoutingRules_t& A,const RoutingRules_t& B)
{ return A.m_RoutingRuleID != B.m_RoutingRuleID; }

/* struct ProcessingRules_t - 
		Message processing rules.
	 */
typedef struct ProcessingRules_t
{
	ProcessingRules_t() // default constructor
	{
		ZeroMemory( this, sizeof(ProcessingRules_t) );
		m_IsEnabled = 1;
	}
	int m_ProcessingRuleID;	/* 
			Uniquely identifies a processing rule.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ProcessingRuleIDLen;

	TCHAR m_RuleDescrip[PROCESSINGRULES_RULEDESCRIP_LENGTH];	/* 
			Short description of rule.
		 SQL Type: VARCHAR(125) */
	long m_RuleDescripLen;

	unsigned char m_IsEnabled;	/* 
			Is this rule enabled?
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	unsigned char m_PrePost;	/* 
			Is this rule run before or after routing rules?
		 Default: 1 SQL Type: BIT */
	long m_PrePostLen;

	int m_ActionType;	/* 
			Uniquely identifies a processing rule.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ActionTypeLen;

	int m_ActionID;	/* 
			Uniquely identifies a processing rule.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ActionIDLen;

	int m_OrderIndex;	/* 
			Allows for ordering of rules.
		 Default: 0 SQL Type: INTEGER */
	long m_OrderIndexLen;

	int m_HitCount;	/* 
			Number of times this routing rule has been employed (since counter was last reset)
		 Default: 0 SQL Type: INTEGER */
	long m_HitCountLen;

} ProcessingRules_t;

inline bool operator==(const ProcessingRules_t& A,const ProcessingRules_t& B)
{ return A.m_ProcessingRuleID == B.m_ProcessingRuleID; }
inline bool operator!=(const ProcessingRules_t& A,const ProcessingRules_t& B)
{ return A.m_ProcessingRuleID != B.m_ProcessingRuleID; }

/* struct DateFilters_t - 
		Date filters.
	 */
typedef struct DateFilters_t
{
	DateFilters_t() // default constructor
	{
		ZeroMemory( this, sizeof(DateFilters_t) );
	}
	int m_DateFilterID;	/* 
			Uniquely identifies a date filter.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_DateFilterIDLen;

	TCHAR m_Description[PROCESSINGRULES_RULEDESCRIP_LENGTH];	/* 
			Short description of rule.
		 SQL Type: VARCHAR(125) */
	long m_DescriptionLen;

	int m_WhereToCheck;	/* 
			What part of the message this rule will be tested against.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_WhereToCheckLen;

	TCHAR m_RegEx[REGULAR_EXPRESSION_LENGTH];	/* 
			Regular Expression.
		 SQL Type: VARCHAR(255) */
	long m_RegExLen;

	TCHAR m_Header[HEADER_LENGTH];	/* 
			The message header to write results to.
		 SQL Type: VARCHAR(50) */
	long m_HeaderLen;

	int m_HeaderValueTypeID;	/* 
			The type of value to write to the message header.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_HeaderValueTypeIDLen;

	TCHAR m_HeaderValue[HEADER_LENGTH];	/* 
			The message header value.
		 SQL Type: VARCHAR(50) */
	long m_HeaderValueLen;

} DateFilters_t;

inline bool operator==(const DateFilters_t& A,const DateFilters_t& B)
{ return A.m_DateFilterID == B.m_DateFilterID; }
inline bool operator!=(const DateFilters_t& A,const DateFilters_t& B)
{ return A.m_DateFilterID != B.m_DateFilterID; }

/* struct ServerParameters_t - 
		Defines global server parameters.
	 */
typedef struct ServerParameters_t
{
	ServerParameters_t() // default constructor
	{
		ZeroMemory( this, sizeof(ServerParameters_t) );
	}
	int m_ServerParameterID;	/* 
			Uniquely identifies the server parameter.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ServerParameterIDLen;

	TCHAR m_Description[SERVERPARAMETERS_DESCRIPTION_LENGTH];	/* 
			A human-readable description of the global parameter
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	TCHAR m_DataValue[SERVERPARAMETERS_DATAVALUE_LENGTH];	/* 
			The current setting of this parameter.
		 SQL Type: VARCHAR(255) */
	long m_DataValueLen;

} ServerParameters_t;

inline bool operator==(const ServerParameters_t& A,const ServerParameters_t& B)
{ return A.m_ServerParameterID == B.m_ServerParameterID; }
inline bool operator!=(const ServerParameters_t& A,const ServerParameters_t& B)
{ return A.m_ServerParameterID != B.m_ServerParameterID; }

/* struct Signatures_t - 
		Signatures used by groups and agents.
	 */
typedef struct Signatures_t
{
	Signatures_t() // default constructor
	{
		ZeroMemory( this, sizeof(Signatures_t) );
		m_SignatureAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Signature = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
	}
	~Signatures_t() // beware! nonvirtual destructor
	{
		if (m_SignatureAllocated > 0) free( m_Signature );
	}
	Signatures_t( const Signatures_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(Signatures_t));
		if (m_SignatureAllocated > 0)
		{
			m_Signature = (TCHAR*)calloc( m_SignatureAllocated, 1 );
			memcpy(m_Signature,i.m_Signature,m_SignatureAllocated);
		}
	}
	Signatures_t& operator=(const Signatures_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_SignatureAllocated > 0) free( m_Signature );
			memcpy(this,&i,sizeof(Signatures_t));
			if (m_SignatureAllocated > 0)
			{
				m_Signature = (TCHAR*)calloc( m_SignatureAllocated, 1 );
				memcpy(m_Signature,i.m_Signature,m_SignatureAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_SignatureID;	/* 
			Uniquely identifies a group signature.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_SignatureIDLen;

	int m_GroupID;	/* 
			Group this signature belongs to (mutually exclusive w/ AgentID).
		 Default: 0 SQL Type: INTEGER */
	long m_GroupIDLen;

	int m_AgentID;	/* 
			Agent this signature belongs to (mutually exclusive w/ GroupID).
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_ObjectTypeID;	/*			
		 Default: 0 SQL Type: INTEGER */
	long m_ObjectTypeIDLen;

	int m_ActualID;	/*			
		 Default: 0 SQL Type: INTEGER */
	long m_ActualIDLen;

	TCHAR m_Name[SIGNATURES_NAME_LENGTH];	/* 
			A short name suitable for populating a list.
		 SQL Type: VARCHAR(64) */
	long m_NameLen;

	TCHAR* m_Signature;	/* 
			The signature itself.
		 SQL Type: TEXT */
	long m_SignatureAllocated;
 	long m_SignatureLen;

} Signatures_t;

inline bool operator==(const Signatures_t& A,const Signatures_t& B)
{ return A.m_SignatureID == B.m_SignatureID; }
inline bool operator!=(const Signatures_t& A,const Signatures_t& B)
{ return A.m_SignatureID != B.m_SignatureID; }

/* struct StandardResponseUsage_t - 
		Logs usage of standard responses by specific agents.
	 */
typedef struct StandardResponseUsage_t
{
	StandardResponseUsage_t() // default constructor
	{
		ZeroMemory( this, sizeof(StandardResponseUsage_t) );
		m_DateUsedLen = SQL_NULL_DATA;
	}
	int m_StdResponseUsageID;	/* 
			Uniquely identifies a standard reponse usage.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_StdResponseUsageIDLen;

	int m_StandardResponseID;	/* 
			Standard repsonse ID used.
		 Default: 0 SQL Type: INTEGER */
	long m_StandardResponseIDLen;

	int m_AgentID;	/* 
			The AgentID who used the standard response.
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	TIMESTAMP_STRUCT m_DateUsed;	/* 
			Date/time std response was used.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateUsedLen;

} StandardResponseUsage_t;

inline bool operator==(const StandardResponseUsage_t& A,const StandardResponseUsage_t& B)
{ return A.m_StdResponseUsageID == B.m_StdResponseUsageID; }
inline bool operator!=(const StandardResponseUsage_t& A,const StandardResponseUsage_t& B)
{ return A.m_StdResponseUsageID != B.m_StdResponseUsageID; }

/* struct StandardResponses_t - 
		Standard responses
	 */
typedef struct StandardResponses_t
{
	StandardResponses_t() // default constructor
	{
		ZeroMemory( this, sizeof(StandardResponses_t) );
		m_StandardResponseAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_StandardResponse = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_DateCreatedLen = SQL_NULL_DATA;
		m_DeletedTimeLen = SQL_NULL_DATA;
		m_NoteAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Note = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_DateModifiedLen = SQL_NULL_DATA;
	}
	~StandardResponses_t() // beware! nonvirtual destructor
	{
		if (m_StandardResponseAllocated > 0) free( m_StandardResponse );
		if (m_NoteAllocated > 0) free( m_Note );
	}
	StandardResponses_t( const StandardResponses_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(StandardResponses_t));
		if (m_StandardResponseAllocated > 0)
		{
			m_StandardResponse = (TCHAR*)calloc( m_StandardResponseAllocated, 1 );
			memcpy(m_StandardResponse,i.m_StandardResponse,m_StandardResponseAllocated);
		}
		if (m_NoteAllocated > 0)
		{
			m_Note = (TCHAR*)calloc( m_NoteAllocated, 1 );
			memcpy(m_Note,i.m_Note,m_NoteAllocated);
		}
	}
	StandardResponses_t& operator=(const StandardResponses_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_StandardResponseAllocated > 0) free( m_StandardResponse );
			if (m_NoteAllocated > 0) free( m_Note );
			memcpy(this,&i,sizeof(StandardResponses_t));
			if (m_StandardResponseAllocated > 0)
			{
				m_StandardResponse = (TCHAR*)calloc( m_StandardResponseAllocated, 1 );
				memcpy(m_StandardResponse,i.m_StandardResponse,m_StandardResponseAllocated);
			}
			if (m_NoteAllocated > 0)
			{
				m_Note = (TCHAR*)calloc( m_NoteAllocated, 1 );
				memcpy(m_Note,i.m_Note,m_NoteAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_StandardResponseID;	/* 
			Uniquely identifies a standard response.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_StandardResponseIDLen;

	TCHAR m_Subject[STANDARDRESPONSES_SUBJECT_LENGTH];	/* 
			Short subject of the standard response.
		 SQL Type: VARCHAR(100) */
	long m_SubjectLen;

	int m_StdResponseCatID;	/* 
			Category ID (0 for uncategorized).
		 Default: 0 SQL Type: INTEGER */
	long m_StdResponseCatIDLen;

	TCHAR* m_StandardResponse;	/* 
			Standard response text.
		 SQL Type: TEXT */
	long m_StandardResponseAllocated;
 	long m_StandardResponseLen;

	unsigned char m_IsApproved;	/* 
			Has std response been approved?
		 Default: 0 SQL Type: BIT */
	long m_IsApprovedLen;

	int m_AgentID;	/* 
			Agent that created the std response.
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			Date/time the std response was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	unsigned char m_IsDeleted;	/* 
			Indicates that the standard response shouldn't be listed and can be removed in an archive or purge operation.
		 Default: 0 SQL Type: TINYINT */
	long m_IsDeletedLen;

	TIMESTAMP_STRUCT m_DeletedTime;	/* 
			Date/time the standard response was deleted.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DeletedTimeLen;

	int m_DeletedBy;	/* 
			The AgentID of who deleted the standard response.
		 Default: 0 SQL Type: INTEGER */
	long m_DeletedByLen;

	TCHAR* m_Note;	/* 
			Note relating to the standard response (for internal use only).
		 SQL Type: TEXT */
	long m_NoteAllocated;
 	long m_NoteLen;

	TIMESTAMP_STRUCT m_DateModified;	/* 
			Date/time the std response was last modified.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateModifiedLen;

	int m_ModifiedBy;	/* 
			The ID of the agent that last modified this standard response
		 Default: 0 SQL Type: INTEGER */
	long m_ModifiedByLen;

	unsigned char m_UseKeywords;
	long m_UseKeywordsLen;

	TCHAR m_Keywords[256];
	long m_KeywordsLen;

} StandardResponses_t;

inline bool operator==(const StandardResponses_t& A,const StandardResponses_t& B)
{ return A.m_StandardResponseID == B.m_StandardResponseID; }
inline bool operator!=(const StandardResponses_t& A,const StandardResponses_t& B)
{ return A.m_StandardResponseID != B.m_StandardResponseID; }

/* struct StdRespApprovalQueue_t - 
		Queues the standard response approval requests. 
		Std responses will not become active until they have been approved by 
		a designated "approver" (as indicated by the Approvals table).
		If an approval is required on a std response edit (versus a new std response), 
		the EditsStdRespID table column is populated with the ID of the std response 
		to be replaced once the edit is approved.
	 */
typedef struct StdRespApprovalQueue_t
{
	StdRespApprovalQueue_t() // default constructor
	{
		ZeroMemory( this, sizeof(StdRespApprovalQueue_t) );
		m_DateRequestedLen = SQL_NULL_DATA;
		m_DateHandledLen = SQL_NULL_DATA;
		m_NoteAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Note = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
	}
	~StdRespApprovalQueue_t() // beware! nonvirtual destructor
	{
		if (m_NoteAllocated > 0) free( m_Note );
	}
	StdRespApprovalQueue_t( const StdRespApprovalQueue_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(StdRespApprovalQueue_t));
		if (m_NoteAllocated > 0)
		{
			m_Note = (TCHAR*)calloc( m_NoteAllocated, 1 );
			memcpy(m_Note,i.m_Note,m_NoteAllocated);
		}
	}
	StdRespApprovalQueue_t& operator=(const StdRespApprovalQueue_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_NoteAllocated > 0) free( m_Note );
			memcpy(this,&i,sizeof(StdRespApprovalQueue_t));
			if (m_NoteAllocated > 0)
			{
				m_Note = (TCHAR*)calloc( m_NoteAllocated, 1 );
				memcpy(m_Note,i.m_Note,m_NoteAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_StdRespApprovalQueueID;	/* 
			Uniquely identifies the standard response item in the queue.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_StdRespApprovalQueueIDLen;

	TIMESTAMP_STRUCT m_DateRequested;	/* 
			Date/time the approval was requested.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateRequestedLen;

	TIMESTAMP_STRUCT m_DateHandled;	/* 
			Date/time the approval request was handled.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateHandledLen;

	unsigned char m_IsApproved;	/* 
			Whether the std response is approved or not.
		 Default: 0 SQL Type: BIT */
	long m_IsApprovedLen;

	int m_RequestAgentID;	/* 
			Agent making the approval request.
		 Default: 0 SQL Type: INTEGER */
	long m_RequestAgentIDLen;

	int m_ApproverAgentID;	/* 
			Agent that actually processed approval.
		 Default: 0 SQL Type: INTEGER */
	long m_ApproverAgentIDLen;

	TCHAR* m_Note;	/* 
			Note for this approval action.
		 SQL Type: TEXT */
	long m_NoteAllocated;
 	long m_NoteLen;

	int m_StandardResponseID;	/* 
			Standard response requiring approval.
		 Default: 0 SQL Type: INTEGER */
	long m_StandardResponseIDLen;

	int m_EditsStdRspID;	/* 
			The std response ID that this one will replace once approved.
		 Default: 0 SQL Type: INTEGER */
	long m_EditsStdRspIDLen;

} StdRespApprovalQueue_t;

inline bool operator==(const StdRespApprovalQueue_t& A,const StdRespApprovalQueue_t& B)
{ return A.m_StdRespApprovalQueueID == B.m_StdRespApprovalQueueID; }
inline bool operator!=(const StdRespApprovalQueue_t& A,const StdRespApprovalQueue_t& B)
{ return A.m_StdRespApprovalQueueID != B.m_StdRespApprovalQueueID; }

/* struct StdResponseAttachments_t - 
		Allows attachments to relate directly to statndard responses.
	 */
typedef struct StdResponseAttachments_t
{
	StdResponseAttachments_t() // default constructor
	{
		ZeroMemory( this, sizeof(StdResponseAttachments_t) );
	}
	int m_StdResponseAttachID;	/* 
			Uniquely identifies a standard response attachment.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_StdResponseAttachIDLen;

	int m_StandardResponseID;	/* 
			Standard response that owns attachment.
		 Default: 0 SQL Type: INTEGER */
	long m_StandardResponseIDLen;

	int m_AttachmentID;	/* 
			ID of the attachment.
		 Default: 0 SQL Type: INTEGER */
	long m_AttachmentIDLen;

} StdResponseAttachments_t;

inline bool operator==(const StdResponseAttachments_t& A,const StdResponseAttachments_t& B)
{ return A.m_StdResponseAttachID == B.m_StdResponseAttachID; }
inline bool operator!=(const StdResponseAttachments_t& A,const StdResponseAttachments_t& B)
{ return A.m_StdResponseAttachID != B.m_StdResponseAttachID; }

/* struct StdResponseCategories_t - 
		Organizes the standard responses into groups.
	 */
typedef struct StdResponseCategories_t
{
	StdResponseCategories_t() // default constructor
	{
		ZeroMemory( this, sizeof(StdResponseCategories_t) );
	}
	int m_StdResponseCatID;	/* 
			Uniquely identifies a standard response category.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_StdResponseCatIDLen;

	TCHAR m_CategoryName[STDRESPONSECATEGORIES_CATEGORYNAME_LENGTH];	/* 
			A human-readable description of the category.
		 SQL Type: VARCHAR(50) */
	long m_CategoryNameLen;

	int m_ObjectID;	/*
		 Default: 0 SQL Type: INTEGER */
	long m_ObjectIDLen;

	unsigned char m_BuiltIn;	/*
		 Default: 1 SQL Type: BIT */
	long m_BuiltInLen;

} StdResponseCategories_t;

inline bool operator==(const StdResponseCategories_t& A,const StdResponseCategories_t& B)
{ return A.m_StdResponseCatID == B.m_StdResponseCatID; }
inline bool operator!=(const StdResponseCategories_t& A,const StdResponseCategories_t& B)
{ return A.m_StdResponseCatID != B.m_StdResponseCatID; }

/* struct StdResponseFavorites_t - 
		Logs usage of standard responses by specific agents.
	 */
typedef struct StdResponseFavorites_t
{
	StdResponseFavorites_t() // default constructor
	{
		ZeroMemory( this, sizeof(StdResponseFavorites_t) );
	}
	int m_StdResponseFavoritesID;	/* 
			Uniquely identifies a standard reponse favorite entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_StdResponseFavoritesIDLen;

	int m_StandardResponseID;	/* 
			Standard repsonse ID used.
		 Default: 0 SQL Type: INTEGER */
	long m_StandardResponseIDLen;

	int m_AgentID;	/* 
			The AgentID who used the standard response.
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

} StdResponseFavorites_t;

inline bool operator==(const StdResponseFavorites_t& A,const StdResponseFavorites_t& B)
{ return A.m_StdResponseFavoritesID == B.m_StdResponseFavoritesID; }
inline bool operator!=(const StdResponseFavorites_t& A,const StdResponseFavorites_t& B)
{ return A.m_StdResponseFavoritesID != B.m_StdResponseFavoritesID; }

/* struct StyleSheets_t - 
		The list of style sheets available.
	 */
typedef struct StyleSheets_t
{
	StyleSheets_t() // default constructor
	{
		ZeroMemory( this, sizeof(StyleSheets_t) );
	}
	int m_StyleSheetID;	/* 
			Uniquely identifies an available style sheet.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_StyleSheetIDLen;

	TCHAR m_Name[STYLESHEETS_NAME_LENGTH];	/* 
			The name of the style (or skin).
		 SQL Type: VARCHAR(45) */
	long m_NameLen;

	TCHAR m_Filename[STYLESHEETS_FILENAME_LENGTH];	/* 
			The file that implements the style (.css file)
		 SQL Type: VARCHAR(255) */
	long m_FilenameLen;

} StyleSheets_t;

inline bool operator==(const StyleSheets_t& A,const StyleSheets_t& B)
{ return A.m_StyleSheetID == B.m_StyleSheetID; }
inline bool operator!=(const StyleSheets_t& A,const StyleSheets_t& B)
{ return A.m_StyleSheetID != B.m_StyleSheetID; }

/* struct TaskList_t - 
		Agent-level task list.
	 */
typedef struct TaskList_t
{
	TaskList_t() // default constructor
	{
		ZeroMemory( this, sizeof(TaskList_t) );
		m_DueDateLen = SQL_NULL_DATA;
		m_StartDateLen = SQL_NULL_DATA;
		m_ReminderLen = SQL_NULL_DATA;
	}
	int m_TaskID;	/* 
			Uniquely identifies task list item.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TaskIDLen;

	int m_AgentID;	/* 
			Agent that owns this task.
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_TaskStatusID;	/* 
			Status of this task.
		 Default: 0 SQL Type: INTEGER */
	long m_TaskStatusIDLen;

	TCHAR m_Subject[TASKLIST_SUBJECT_LENGTH];	/* 
			Subject of this task.
		 SQL Type: VARCHAR(125) */
	long m_SubjectLen;

	TCHAR m_TaskDescrip[TASKLIST_TASKDESCRIP_LENGTH];	/* 
			Description of the task.
		 SQL Type: VARCHAR(255) */
	long m_TaskDescripLen;

	unsigned char m_PercentageComplete;	/* 
			Percentage complete.
		 SQL Type: TINYINT */
	long m_PercentageCompleteLen;

	TIMESTAMP_STRUCT m_DueDate;	/* 
			Date/time the task is due (NULL for none).
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DueDateLen;

	TIMESTAMP_STRUCT m_StartDate;	/* 
			Date/time the task starts (NULL for none).
		 ALLOW NULLS SQL Type: DATETIME */
	long m_StartDateLen;

	TIMESTAMP_STRUCT m_Reminder;	/* 
			Date/time to remind the user about this task (NULL for none).
		 ALLOW NULLS SQL Type: DATETIME */
	long m_ReminderLen;

	int m_TaskPriorityID;	/* 
			Priority of task.
		 Default: 0 SQL Type: INTEGER */
	long m_TaskPriorityIDLen;

} TaskList_t;

inline bool operator==(const TaskList_t& A,const TaskList_t& B)
{ return A.m_TaskID == B.m_TaskID; }
inline bool operator!=(const TaskList_t& A,const TaskList_t& B)
{ return A.m_TaskID != B.m_TaskID; }

/* struct TaskPriorities_t - 
		Priorities of tasks. Examples include: low,	normal,	and high.
	 */
typedef struct TaskPriorities_t
{
	TaskPriorities_t() // default constructor
	{
		ZeroMemory( this, sizeof(TaskPriorities_t) );
	}
	int m_TaskPriorityID;	/* 
			Uniquely identifies the task priority.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TaskPriorityIDLen;

	TCHAR m_PriorityName[TASKPRIORITIES_PRIORITYNAME_LENGTH];	/* 
			Name of the priority (e.g. low, normal, high)
		 SQL Type: VARCHAR(50) */
	long m_PriorityNameLen;

	unsigned char m_IsDefault;	/* 
			This priority is the default for newly created tasks.
		 Default: 0 SQL Type: BIT */
	long m_IsDefaultLen;

} TaskPriorities_t;

inline bool operator==(const TaskPriorities_t& A,const TaskPriorities_t& B)
{ return A.m_TaskPriorityID == B.m_TaskPriorityID; }
inline bool operator!=(const TaskPriorities_t& A,const TaskPriorities_t& B)
{ return A.m_TaskPriorityID != B.m_TaskPriorityID; }

/* struct TaskStatuses_t - 
		Status of current specific task. 
		Examples: Complete, In progress, Not started, Deferred, Waiting on someone else.
	 */
typedef struct TaskStatuses_t
{
	TaskStatuses_t() // default constructor
	{
		ZeroMemory( this, sizeof(TaskStatuses_t) );
	}
	int m_TaskStatusID;	/* 
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TaskStatusIDLen;

	TCHAR m_StatusDescrip[TASKSTATUSES_STATUSDESCRIP_LENGTH];	/* 
			Description of task status (e.g. complete, in progres, etc.)
		 SQL Type: VARCHAR(25) */
	long m_StatusDescripLen;

	unsigned char m_IsDefault;	/* 
			This status is the default for newly created tasks.
		 Default: 0 SQL Type: BIT */
	long m_IsDefaultLen;

} TaskStatuses_t;

inline bool operator==(const TaskStatuses_t& A,const TaskStatuses_t& B)
{ return A.m_TaskStatusID == B.m_TaskStatusID; }
inline bool operator!=(const TaskStatuses_t& A,const TaskStatuses_t& B)
{ return A.m_TaskStatusID != B.m_TaskStatusID; }

/* struct TicketActions_t - 
		Enumerates the different ticket actions stored in the TicketHistory table
	 */
typedef struct TicketActions_t
{
	TicketActions_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketActions_t) );
	}
	int m_TicketActionID;	/* 
			Uniquely identifies the Ticket action.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketActionIDLen;

	TCHAR m_Description[TICKETACTIONS_DESCRIPTION_LENGTH];	/* 
			Description of the action.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

} TicketActions_t;

inline bool operator==(const TicketActions_t& A,const TicketActions_t& B)
{ return A.m_TicketActionID == B.m_TicketActionID; }
inline bool operator!=(const TicketActions_t& A,const TicketActions_t& B)
{ return A.m_TicketActionID != B.m_TicketActionID; }

/* struct TicketBoxFooters_t - 
		Footers used by ticket boxes.
	 */
typedef struct TicketBoxFooters_t
{
	TicketBoxFooters_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketBoxFooters_t) );
		m_FooterAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Footer = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
	}
	~TicketBoxFooters_t() // beware! nonvirtual destructor
	{
		if (m_FooterAllocated > 0) free( m_Footer );
	}
	TicketBoxFooters_t( const TicketBoxFooters_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(TicketBoxFooters_t));
		if (m_FooterAllocated > 0)
		{
			m_Footer = (TCHAR*)calloc( m_FooterAllocated, 1 );
			memcpy(m_Footer,i.m_Footer,m_FooterAllocated);
		}
	}
	TicketBoxFooters_t& operator=(const TicketBoxFooters_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_FooterAllocated > 0) free( m_Footer );
			memcpy(this,&i,sizeof(TicketBoxFooters_t));
			if (m_FooterAllocated > 0)
			{
				m_Footer = (TCHAR*)calloc( m_FooterAllocated, 1 );
				memcpy(m_Footer,i.m_Footer,m_FooterAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_FooterID;	/* 
			Uniquely identifies a footer.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_FooterIDLen;

	TCHAR m_Description[TICKETBOXFOOTERS_DESCRIPTION_LENGTH];	/* 
			Short description of the footer.
		 SQL Type: VARCHAR(125) */
	long m_DescriptionLen;

	TCHAR* m_Footer;	/* 
			The footer itself.
		 SQL Type: TEXT */
	long m_FooterAllocated;
 	long m_FooterLen;

	unsigned char m_IsDeleted;	/* 
			Is this ticket box footer deleted?
		 Default: 0 SQL Type: TINYINT */
	long m_IsDeletedLen;

} TicketBoxFooters_t;

inline bool operator==(const TicketBoxFooters_t& A,const TicketBoxFooters_t& B)
{ return A.m_FooterID == B.m_FooterID; }
inline bool operator!=(const TicketBoxFooters_t& A,const TicketBoxFooters_t& B)
{ return A.m_FooterID != B.m_FooterID; }

/* struct TicketBoxHeaders_t - 
		Headers used by ticket boxes.
	 */
typedef struct TicketBoxHeaders_t
{
	TicketBoxHeaders_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketBoxHeaders_t) );
		m_HeaderAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Header = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
	}
	~TicketBoxHeaders_t() // beware! nonvirtual destructor
	{
		if (m_HeaderAllocated > 0) free( m_Header );
	}
	TicketBoxHeaders_t( const TicketBoxHeaders_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(TicketBoxHeaders_t));
		if (m_HeaderAllocated > 0)
		{
			m_Header = (TCHAR*)calloc( m_HeaderAllocated, 1 );
			memcpy(m_Header,i.m_Header,m_HeaderAllocated);
		}
	}
	TicketBoxHeaders_t& operator=(const TicketBoxHeaders_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_HeaderAllocated > 0) free( m_Header );
			memcpy(this,&i,sizeof(TicketBoxHeaders_t));
			if (m_HeaderAllocated > 0)
			{
				m_Header = (TCHAR*)calloc( m_HeaderAllocated, 1 );
				memcpy(m_Header,i.m_Header,m_HeaderAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_HeaderID;	/* 
			Uniquely identifies a header.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_HeaderIDLen;

	TCHAR m_Description[TICKETBOXHEADERS_DESCRIPTION_LENGTH];	/* 
			Short description of the header.
		 SQL Type: VARCHAR(125) */
	long m_DescriptionLen;

	TCHAR* m_Header;	/* 
			The header itself.
		 SQL Type: TEXT */
	long m_HeaderAllocated;
 	long m_HeaderLen;

	unsigned char m_IsDeleted;	/* 
			Is this ticket box header deleted?
		 Default: 0 SQL Type: TINYINT */
	long m_IsDeletedLen;

} TicketBoxHeaders_t;

inline bool operator==(const TicketBoxHeaders_t& A,const TicketBoxHeaders_t& B)
{ return A.m_HeaderID == B.m_HeaderID; }
inline bool operator!=(const TicketBoxHeaders_t& A,const TicketBoxHeaders_t& B)
{ return A.m_HeaderID != B.m_HeaderID; }

/* struct TicketBoxViewTypes_t - 
		The types of ticket box views applicable to specific ticket boxes.
	 */
typedef struct TicketBoxViewTypes_t
{
	TicketBoxViewTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketBoxViewTypes_t) );
	}
	int m_TicketBoxViewTypeID;	/* 
			Uniquely identifies the ticket box view type.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxViewTypeIDLen;

	TCHAR m_Name[TICKETBOXVIEWTYPES_NAME_LENGTH];	/* 
			The human-readable name of the ticketbox view type.
			Examples include: Public, Inbox, Outbox.
		 SQL Type: VARCHAR(55) */
	long m_NameLen;

} TicketBoxViewTypes_t;

inline bool operator==(const TicketBoxViewTypes_t& A,const TicketBoxViewTypes_t& B)
{ return A.m_TicketBoxViewTypeID == B.m_TicketBoxViewTypeID; }
inline bool operator!=(const TicketBoxViewTypes_t& A,const TicketBoxViewTypes_t& B)
{ return A.m_TicketBoxViewTypeID != B.m_TicketBoxViewTypeID; }

/* struct TicketBoxViews_t - 
		Whenever a ticket box is created for (or made accessible to) an agent, a 
		TicketBoxView must be created that represents that agent's view into the ticket box.
		View-related parameters, such as sort order, sort field, and the columns to display, 
		must be maintained within this table to ensure a consistent representation of views 
		to the agent regardless of which computer he/she uses.
	 */
typedef struct TicketBoxViews_t
{
	TicketBoxViews_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketBoxViews_t) );
		m_SortField = 2;
	}
	int m_TicketBoxViewID;	/* 
			Uniquely identifies the ticket box view.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxViewIDLen;

	int m_AgentID;	/* 
			The AgentID of the agent who will display this view
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_TicketBoxID;	/* 
			Used to specify which public ticketbox will be displayed.  Zero if this view
			refers to a private ticketbox.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	int m_AgentBoxID;	/* 
			Used to specify which agent's Inbox, OutBox, Sent Items, Drafts, or Deleted Items
			will be displayed  Zero if this view refers to a public ticket box.
		 Default: 0 SQL Type: INTEGER */
	long m_AgentBoxIDLen;

	unsigned char m_ShowOwnedItems;	/* 
			Display items that are owned by an agent.
		 Default: 0 SQL Type: BIT */
	long m_ShowOwnedItemsLen;

	unsigned char m_ShowClosedItems;	/* 
			Display items that have a status of closed.
		 Default: 0 SQL Type: BIT */
	long m_ShowClosedItemsLen;

	unsigned char m_SortAscending;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_SortAscendingLen;

	int m_SortField;	/* 
			The field by which we sort this view.
		 Default: 2 SQL Type: INTEGER */
	long m_SortFieldLen;

	int m_TicketBoxViewTypeID;	/* 
			The type of ticket box view (Inbox, OutBox, Etc.)
		 Default: 0 SQL Type: INTEGER */
	long m_TicketBoxViewTypeIDLen;

	unsigned char m_ShowState;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowStateLen;

	unsigned char m_ShowPriority;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowPriorityLen;

	unsigned char m_ShowNumNotes;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowNumNotesLen;

	unsigned char m_ShowTicketID;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowTicketIDLen;

	unsigned char m_ShowNumMsgs;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowNumMsgsLen;

	unsigned char m_ShowSubject;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowSubjectLen;

	unsigned char m_ShowContact;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowContactLen;

	unsigned char m_ShowDate;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowDateLen;

	unsigned char m_ShowCategory;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowCategoryLen;

	unsigned char m_ShowOwner;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowOwnerLen;

	unsigned char m_ShowTicketBox;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_ShowTicketBoxLen;

	unsigned char m_UseDefault;	/* 
			Sort ascending if true, sort descending if false.
		 Default: 0 SQL Type: BIT */
	long m_UseDefaultLen;

} TicketBoxViews_t;

inline bool operator==(const TicketBoxViews_t& A,const TicketBoxViews_t& B)
{ return A.m_TicketBoxViewID == B.m_TicketBoxViewID; }
inline bool operator!=(const TicketBoxViews_t& A,const TicketBoxViews_t& B)
{ return A.m_TicketBoxViewID != B.m_TicketBoxViewID; }

/* struct TicketBoxes_t - 
		Ticket boxes.
	 */
typedef struct TicketBoxes_t
{
	TicketBoxes_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketBoxes_t) );
		m_AutoReplyQuoteMsg = 1;
	}
	int m_TicketBoxID;	/* 
			Uniquely identifies ticket boxes.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	TCHAR m_Name[TICKETBOXES_NAME_LENGTH];	/* 
			Short name of ticket box.
		 SQL Type: VARCHAR(50) */
	long m_NameLen;

	TCHAR m_Description[TICKETBOXES_DESCRIPTION_LENGTH];	/* 
			Description of ticket box.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	unsigned char m_AgeAlertingEnabled;	/* 
			Is age alerting enabled (alert after ticket reaches certain age)?
		 Default: 0 SQL Type: BIT */
	long m_AgeAlertingEnabledLen;

	int m_AgeAlertingThresholdMins;	/* 
			Age alerting time threshold (in mins).
		 Default: 0 SQL Type: INTEGER */
	long m_AgeAlertingThresholdMinsLen;

	int m_ObjectID;	/* 
			Security objectID
		 Default: 0 SQL Type: INTEGER */
	long m_ObjectIDLen;

	int m_MaxRecordsSize;	/* 
			Maximum number of tickets this box will hold (0 for unlimited)
		 Default: 0 SQL Type: INTEGER */
	long m_MaxRecordsSizeLen;

	unsigned char m_RequireGetOldest;	/* 
			Force agents to use get oldest button?
		 Default: 0 SQL Type: BIT */
	long m_RequireGetOldestLen;

	int m_LowWaterMark;	/* 
			Low water mark threshold.
		 Default: 0 SQL Type: INTEGER */
	long m_LowWaterMarkLen;

	int m_HighWaterMark;	/* 
			High water mark threshold.
		 Default: 0 SQL Type: INTEGER */
	long m_HighWaterMarkLen;

	unsigned char m_AutoReplyEnable;	/* 
			Enable auto-reply
		 Default: 0 SQL Type: BIT */
	long m_AutoReplyEnableLen;

	int m_AutoReplyWithStdResponse;	/* 
			Auto-reply with a std reponse ID
		 Default: 0 SQL Type: INTEGER */
	long m_AutoReplyWithStdResponseLen;

	int m_AutoReplyThreshHoldMins;	/* 
			Auto-reply if the msg isn't handled within X mins.
		 Default: 0 SQL Type: INTEGER */
	long m_AutoReplyThreshHoldMinsLen;

	unsigned char m_AutoReplyQuoteMsg;	/* 
			When Auto-replying, quote original message in outbound message?
		 Default: 1 SQL Type: BIT */
	long m_AutoReplyQuoteMsgLen;

	unsigned char m_AutoReplyCloseTicket;	/* 
			When Auto-Replying, should the ticket be left open?
		 Default: 0 SQL Type: BIT */
	long m_AutoReplyCloseTicketLen;

	int m_HeaderID;	/* 
			The ID of the header used by this ticketbox.
		 Default: 0 SQL Type: INTEGER */
	long m_HeaderIDLen;

	int m_FooterID;	/* 
			The ID of the footer used by this ticketbox.
		 Default: 0 SQL Type: INTEGER */
	long m_FooterIDLen;

	int m_OwnerID;	/* 
			The ID of the owner of this ticketbox.
		 Default: 0 SQL Type: INTEGER */
	long m_OwnerIDLen;

	TCHAR m_DefaultEmailAddress[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			Default email address of ticket box.
		 SQL Type: VARCHAR(255) */
	long m_DefaultEmailAddressLen;

	unsigned char m_WaterMarkStatus;	/* 
			0 = Ok, 1 = Above Low Watermark, 2 = Above High Watermark
		 Default: 0 SQL Type: TINYINT */
	long m_WaterMarkStatusLen;

	TCHAR m_DefaultEmailAddressName[TICKETBOXES_NAME_LENGTH];	/* 
			Short name of ticket box.
		 SQL Type: VARCHAR(50) */
	long m_DefaultEmailAddressNameLen;

	unsigned char m_FooterLocation;	/* 
			0 = Bottom, 1 = Above Original Message
		 Default: 0 SQL Type: TINYINT */
	long m_FooterLocationLen;

	unsigned char m_AutoReplyInTicket;	/* 
			Place Auto Reply in Ticket
		 Default: 1 SQL Type: BIT */
	long m_AutoReplyInTicketLen;

	int m_MessageDestinationID;	/* 
			The Message Destination to use for this TicketBox.
		 Default: 0 SQL Type: INTEGER */
	long m_MessageDestinationIDLen;

	int m_UnreadMode;	/* 
			The TicketBox Unread Mode.
		 Default: 0 SQL Type: INTEGER */
	long m_UnreadModeLen;

	int m_FromFormat;	/* 
			The TicketBox From Format.
		 Default: 0 SQL Type: INTEGER */
	long m_FromFormatLen;

	int m_TicketLink;	/* 
			Allow Ticket Linking.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketLinkLen;

	int m_MultiMail;	/*
		 Default: 0 SQL Type: INTEGER */
	long m_MultiMailLen;

	int m_RequireTC;	/*
		 Default: 0 SQL Type: INTEGER */
	long m_RequireTCLen;

	TCHAR m_ReplyToEmailAddress[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			Reply to email address of ticket box.
		 SQL Type: VARCHAR(255) */
	long m_ReplyToEmailAddressLen;

	TCHAR m_ReturnPathEmailAddress[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];	/* 
			Return path email address of ticket box.
		 SQL Type: VARCHAR(255) */
	long m_ReturnPathEmailAddressLen;
	
} TicketBoxes_t;

inline bool operator==(const TicketBoxes_t& A,const TicketBoxes_t& B)
{ return A.m_TicketBoxID == B.m_TicketBoxID; }
inline bool operator!=(const TicketBoxes_t& A,const TicketBoxes_t& B)
{ return A.m_TicketBoxID != B.m_TicketBoxID; }

/* struct TicketContacts_t - 
		Relates ticket boxes to contacts.
	 */
typedef struct TicketContacts_t
{
	TicketContacts_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketContacts_t) );
	}
	int m_TicketContactID;	/* 
			Uniquely identifies an entry in the table.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketContactIDLen;

	int m_TicketID;	/* 
			Identifies the ticket.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketIDLen;

	int m_ContactID;	/* 
			The contact that this ticket relates to.
		 Default: 0 SQL Type: INTEGER */
	long m_ContactIDLen;

} TicketContacts_t;

inline bool operator==(const TicketContacts_t& A,const TicketContacts_t& B)
{ return A.m_TicketContactID == B.m_TicketContactID; }
inline bool operator!=(const TicketContacts_t& A,const TicketContacts_t& B)
{ return A.m_TicketContactID != B.m_TicketContactID; }

/* struct TicketHistory_t - 
		Contains history information about the ticket
	 */
typedef struct TicketHistory_t
{
	TicketHistory_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketHistory_t) );
		m_DateTimeLen = SQL_NULL_DATA;
	}
	int m_TicketHistoryID;	/* 
			Uniquely identifies the ticket history entry
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketHistoryIDLen;

	int m_TicketID;	/* 
			The TicketID being referenced.
		 SQL Type: INTEGER */
	long m_TicketIDLen;

	TIMESTAMP_STRUCT m_DateTime;	/* 
			When the action occurred.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateTimeLen;

	int m_AgentID;	/* 
			The ID of the Agent who performed the action, or zero if done by system.
		 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_TicketActionID;	/* 
			The ID of the action performed.
		 SQL Type: INTEGER */
	long m_TicketActionIDLen;

	int m_ID1;	/* 
			See the TicketActions table for interpretation of this field.
		 SQL Type: INTEGER */
	long m_ID1Len;

	int m_ID2;	/* 
			See the TicketActions table for interpretation of this field.
		 SQL Type: INTEGER */
	long m_ID2Len;

	int m_TicketStateID;	/* 
			The TicketStateID being referenced.
		 SQL Type: INTEGER */
	long m_TicketStateIDLen;

	int m_TicketBoxID;	/* 
			The TicketBoxID being referenced.
		 SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	int m_OwnerID;	/* 
			The OwnerID being referenced.
		 SQL Type: INTEGER */
	long m_OwnerIDLen;

	int m_PriorityID;	/* 
			The PriorityID being referenced.
		 SQL Type: INTEGER */
	long m_PriorityIDLen;

	int m_TicketCategoryID;	/* 
			The TicketID being referenced.
		 SQL Type: INTEGER */
	long m_TicketCategoryIDLen;

	TCHAR m_DataValue[LINKNAME_LENGTH];	/* 			
		 SQL Type: VARCHAR(50) */
	long m_DataValueLen;

} TicketHistory_t;

inline bool operator==(const TicketHistory_t& A,const TicketHistory_t& B)
{ return A.m_TicketHistoryID == B.m_TicketHistoryID; }
inline bool operator!=(const TicketHistory_t& A,const TicketHistory_t& B)
{ return A.m_TicketHistoryID != B.m_TicketHistoryID; }

/* struct TicketNotes_t - 
		Allows for multiple notes to be related to tickets. 
		Some ticket notes may be generated by the system to 
		indicate transfer of ownership, etc.
	 */
typedef struct TicketNotes_t
{
	TicketNotes_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketNotes_t) );
		m_NoteAllocated = EMS_TEXT_BUFF_LEN_SEED;
		m_Note = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );
		m_DateCreatedLen = SQL_NULL_DATA;
	}
	~TicketNotes_t() // beware! nonvirtual destructor
	{
		if (m_NoteAllocated > 0) free( m_Note );
	}
	TicketNotes_t( const TicketNotes_t& i ) // copy constructor
	{
		memcpy(this,&i,sizeof(TicketNotes_t));
		if (m_NoteAllocated > 0)
		{
			m_Note = (TCHAR*)calloc( m_NoteAllocated, 1 );
			memcpy(m_Note,i.m_Note,m_NoteAllocated);
		}
	}
	TicketNotes_t& operator=(const TicketNotes_t& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_NoteAllocated > 0) free( m_Note );
			memcpy(this,&i,sizeof(TicketNotes_t));
			if (m_NoteAllocated > 0)
			{
				m_Note = (TCHAR*)calloc( m_NoteAllocated, 1 );
				memcpy(m_Note,i.m_Note,m_NoteAllocated);
			}
		}
		return *this; // return reference to self
	}
	int m_TicketNoteID;	/* 
			Uniquely identifies the ticket note.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketNoteIDLen;

	int m_NumAttach;	
			
	long m_NumAttachLen;

	int m_TicketID;	/* 
			Ticket the note relates to.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketIDLen;

	int m_AgentID;	/* 
			Agent who wrote the note (or NULL for system-generated)
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	TCHAR* m_Note;	/* 
			The note itself.
		 SQL Type: TEXT */
	long m_NoteAllocated;
 	long m_NoteLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			Date/time the note was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	unsigned char m_IsVoipNote;	/* 
			Is this a VOIP note?
		 Default: 0 SQL Type: BIT */
	long m_IsVoipNoteLen;

	TIMESTAMP_STRUCT m_StartTime;	/* 
			Date/time the VOIP call was started.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_StartTimeLen;

	TIMESTAMP_STRUCT m_StopTime;	/* 
			Date/time the VOIP call was stopped.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_StopTimeLen;

	TCHAR m_ElapsedTime[TICKETBOXES_NAME_LENGTH];	/* 
			Short name of ticket box.
		 SQL Type: VARCHAR(50) */
	long m_ElapsedTimeLen;

	int m_ContactID;	/* 
			Contact the note relates to.
		 Default: 0 SQL Type: INTEGER */
	long m_ContactIDLen;

} TicketNotes_t;

inline bool operator==(const TicketNotes_t& A,const TicketNotes_t& B)
{ return A.m_TicketNoteID == B.m_TicketNoteID; }
inline bool operator!=(const TicketNotes_t& A,const TicketNotes_t& B)
{ return A.m_TicketNoteID != B.m_TicketNoteID; }

/* struct TicketStates_t - 
		The state of the ticket. Examples include:
		open, closed, and escalated.
	 */
typedef struct TicketStates_t
{
	TicketStates_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketStates_t) );
	}
	int m_TicketStateID;	/* 
			Uniquely identifies a ticket state.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketStateIDLen;

	TCHAR m_Description[TICKETSTATES_DESCRIPTION_LENGTH];	/* 
			Description of ticket state.
		 SQL Type: VARCHAR(25) */
	long m_DescriptionLen;

	TCHAR m_HTMLColorName[TICKETSTATES_HTMLCOLORNAME_LENGTH];	/* 
			Color used to display tickets of this state.
		 SQL Type: VARCHAR(25) */
	long m_HTMLColorNameLen;

} TicketStates_t;

inline bool operator==(const TicketStates_t& A,const TicketStates_t& B)
{ return A.m_TicketStateID == B.m_TicketStateID; }
inline bool operator!=(const TicketStates_t& A,const TicketStates_t& B)
{ return A.m_TicketStateID != B.m_TicketStateID; }

/* struct Tickets_t - 
		Tickets represent a logical dialog with a specific contact. 
		Tickets can be thought of as "incidents", and should relate to a 
		specific issue, premise, or problem.
	 */
typedef struct Tickets_t
{
	Tickets_t() // default constructor
	{
		ZeroMemory( this, sizeof(Tickets_t) );
		m_DateCreatedLen = SQL_NULL_DATA;
		m_OpenTimestampLen = SQL_NULL_DATA;
		m_DeletedTimeLen = SQL_NULL_DATA;
		m_ReOpenTimeLen = SQL_NULL_DATA;
		m_LockedTimeLen = SQL_NULL_DATA;
	}
	int m_TicketID;	/* 
			Uniquely identifies a ticket.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketIDLen;

	int m_TicketStateID;	/* 
			The state of the ticket.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketStateIDLen;

	int m_TicketBoxID;	/* 
			TicketBox this ticket belongs to.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	int m_OwnerID;	/* 
			The AgentID of the agent who owns the ticket.  An agent owns any tickets which
			are in thier inbox.
		 Default: 0 SQL Type: INTEGER */
	long m_OwnerIDLen;

	TCHAR m_Subject[TICKETS_SUBJECT_LENGTH];	/* 
			The subject of the ticket.
		 SQL Type: VARCHAR(255) */
	long m_SubjectLen;

	TCHAR m_Contacts[TICKETS_CONTACTS_LENGTH];	/* 
			The contacts whom this ticket references.
		 SQL Type: VARCHAR(255) */
	long m_ContactsLen;

	int m_PriorityID;	/* 
			Priority level of this ticket (low, medium, high, etc.).
		 Default: 0 SQL Type: INTEGER */
	long m_PriorityIDLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			Date/time the ticket was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	TIMESTAMP_STRUCT m_OpenTimestamp;	/* 
			Date/time the ticket was last opened.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_OpenTimestampLen;

	int m_OpenMins;	/* 
			Number of minutes that the ticket has been open.
		 Default: 0 SQL Type: INTEGER */
	long m_OpenMinsLen;

	unsigned char m_AutoReplied;	/* 
			Has this ticket been auto-replied to?
		 Default: 0 SQL Type: BIT */
	long m_AutoRepliedLen;

	unsigned char m_AgeAlerted;	/* 
			Has this ticket been age alerted?
		 Default: 0 SQL Type: BIT */
	long m_AgeAlertedLen;

	unsigned char m_IsDeleted;	/* 
			Is this ticket deleted?
		 Default: 0 SQL Type: TINYINT */
	long m_IsDeletedLen;

	TIMESTAMP_STRUCT m_DeletedTime;	/* 
			Date/time the ticket was deleted.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DeletedTimeLen;

	int m_DeletedBy;	/* 
			The AgentID of who deleted the contact.
		 Default: 0 SQL Type: INTEGER */
	long m_DeletedByLen;

	TIMESTAMP_STRUCT m_LockedTime;	/* 
			Date/time the ticket was locked.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_LockedTimeLen;

	int m_LockedBy;	/* 
			Is this ticket locked by an agent? 0 = unlocked.
		 Default: 0 SQL Type: INTEGER */
	long m_LockedByLen;

	int m_TicketCategoryID;	/* 
			The ticket category this ticket belongs to.
		 Default: 0 SQL Type: INTEGER */
	long m_TicketCategoryIDLen;

	int m_UseTickler;
	long m_UseTicklerLen;

	TIMESTAMP_STRUCT m_ReOpenTime;
	long m_ReOpenTimeLen;

	int m_FolderID;
	long m_FolderIDLen;

	TCHAR m_LinkDescr[TICKETS_SUBJECT_LENGTH]; 
	long m_LinkDescrLen;

} Tickets_t;

inline bool operator==(const Tickets_t& A,const Tickets_t& B)
{ return A.m_TicketID == B.m_TicketID; }
inline bool operator!=(const Tickets_t& A,const Tickets_t& B)
{ return A.m_TicketID != B.m_TicketID; }


/* struct AgentActivity_t - 
		Agent Activity Audit.
	 */
typedef struct AgentActivity_t
{
	AgentActivity_t() // default constructor
	{
		ZeroMemory( this, sizeof(AgentActivity_t) );		
	}
	int m_AgentActivityID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgentActivityIDLen;

	int m_AgentID;	/* 
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_ActivityID;	/* 
		 Default: 0 SQL Type: INTEGER */
	long m_ActivityIDLen;

	TIMESTAMP_STRUCT m_ActivityDate;	/* 
			Date/time the activity occured.
		 Default: getdate() */
	long m_ActivityDateLen;

	int m_ID1;	/* 
		 Default: 0 SQL Type: INTEGER */
	long m_ID1Len;

	int m_ID2;	/* 
		 Default: 0 SQL Type: INTEGER */
	long m_ID2Len;

	int m_ID3;	/* 
		 Default: 0 SQL Type: INTEGER */
	long m_ID3Len;

	TCHAR m_Data1[TICKETFIELDS_DESCRIPTION_LENGTH];	/* 
			Data1.
		 SQL Type: VARCHAR(50) */
	long m_Data1Len;

	TCHAR m_Data2[TICKETFIELDS_DESCRIPTION_LENGTH];	/* 
			Data2.
		 SQL Type: VARCHAR(50) */
	long m_Data2Len;	

} AgentActivity_t;

inline bool operator==(const AgentActivity_t& A,const AgentActivity_t& B)
{ return A.m_AgentActivityID == B.m_AgentActivityID; }
inline bool operator!=(const AgentActivity_t& A,const AgentActivity_t& B)
{ return A.m_AgentActivityID != B.m_AgentActivityID; }


/* struct AgentActivities_t - 
		Agent Activities.
	 */
typedef struct AgentActivities_t
{
	AgentActivities_t() // default constructor
	{
		ZeroMemory( this, sizeof(AgentActivities_t) );		
	}
	int m_AgentActivityID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgentActivityIDLen;

	TCHAR m_Description[TICKETFIELDS_DESCRIPTION_LENGTH];	/* 
			Description.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;	

} AgentActivities_t;

inline bool operator==(const AgentActivities_t& A,const AgentActivities_t& B)
{ return A.m_AgentActivityID == B.m_AgentActivityID; }
inline bool operator!=(const AgentActivities_t& A,const AgentActivities_t& B)
{ return A.m_AgentActivityID != B.m_AgentActivityID; }


/* struct AgentTicketCategories_t - 
		Agent Ticket Categories.
	 */
typedef struct AgentTicketCategories_t
{
	AgentTicketCategories_t() // default constructor
	{
		ZeroMemory( this, sizeof(AgentTicketCategories_t) );		
	}
	int m_AgentTicketCategoryID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgentTicketCategoryIDLen;

	int m_AgentID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_TicketCategoryID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketCategoryIDLen;		

} AgentTicketCategories_t;

inline bool operator==(const AgentTicketCategories_t& A,const AgentTicketCategories_t& B)
{ return A.m_AgentTicketCategoryID == B.m_AgentTicketCategoryID; }
inline bool operator!=(const AgentTicketCategories_t& A,const AgentTicketCategories_t& B)
{ return A.m_AgentTicketCategoryID != B.m_AgentTicketCategoryID; }


/* struct TicketFields_t - 
		Ticket Fields are administrator defined fields.
	 */
typedef struct TicketFields_t
{
	TicketFields_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketFields_t) );
	}
	int m_TicketFieldID;	/* 
			Uniquely identifies a ticket field.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketFieldIDLen;

	TCHAR m_Description[TICKETFIELDS_DESCRIPTION_LENGTH]; /* 
			The description of the ticket field.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;

	int m_TicketFieldTypeID;	/* 
			The type of custom TicketField */
	long m_TicketFieldTypeIDLen;

} TicketFields_t;

inline bool operator==(const TicketFields_t& A,const TicketFields_t& B)
{ return A.m_TicketFieldID == B.m_TicketFieldID; }
inline bool operator!=(const TicketFields_t& A,const TicketFields_t& B)
{ return A.m_TicketFieldID != B.m_TicketFieldID; }


/* struct TicketFieldTypes_t - 
		Ticket Field Types are system defined fields.
	 */
typedef struct TicketFieldTypes_t
{
	TicketFieldTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketFieldTypes_t) );
	}
	int m_TicketFieldTypeID;	/* 
			Uniquely identifies a ticket field type.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketFieldTypeIDLen;

	TCHAR m_Description[TICKETFIELDS_DESCRIPTION_LENGTH]; /* 
			The description of the ticket field type.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;	

} TicketFieldTypes_t;

inline bool operator==(const TicketFieldTypes_t& A,const TicketFieldTypes_t& B)
{ return A.m_TicketFieldTypeID == B.m_TicketFieldTypeID; }
inline bool operator!=(const TicketFieldTypes_t& A,const TicketFieldTypes_t& B)
{ return A.m_TicketFieldTypeID != B.m_TicketFieldTypeID; }


/* struct TicketFieldOptions_t - 
		Ticket Fields Options are administrator defined fields.
	 */
typedef struct TicketFieldOptions_t
{
	TicketFieldOptions_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketFieldOptions_t) );
	}
	int m_TicketFieldOptionID;	/* 
			Uniquely identifies a ticket field option.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketFieldOptionIDLen;

	int m_TicketFieldID;	/* 
			The TicketFieldID this option is associated with */
	long m_TicketFieldIDLen;
	
	TCHAR m_OptionValue[TICKETFIELDS_DESCRIPTION_LENGTH]; /* 
			The value of the option.
		 SQL Type: VARCHAR(50) */
	long m_OptionValueLen;

	int m_OptionOrder;	/* 
			The display order */
	long m_OptionOrderLen;

} TicketFieldOptions_t;

inline bool operator==(const TicketFieldOptions_t& A,const TicketFieldOptions_t& B)
{ return A.m_TicketFieldOptionID == B.m_TicketFieldOptionID; }
inline bool operator!=(const TicketFieldOptions_t& A,const TicketFieldOptions_t& B)
{ return A.m_TicketFieldOptionID != B.m_TicketFieldOptionID; }


/* struct TicketFieldsTicketBox_t - 
		Ticket Fields to TicketBox.
	 */
typedef struct TicketFieldsTicketBox_t
{
	TicketFieldsTicketBox_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketFieldsTicketBox_t) );
	}
	int m_TicketFieldsTicketBoxID;	/* 
			Uniquely identifies a ticket field TicketBox combination.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketFieldsTicketBoxIDLen;

	int m_TicketBoxID;	/* 
			Uniquely identifies a TicketBox.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	int m_TicketFieldID;	/* 
			Uniquely identifies a ticket field.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketFieldIDLen;

	unsigned char m_IsRequired;	/* 
			TRUE if field is required for Tickets in this TicketBox, FALSE if not.
		 Default: 0 SQL Type: BIT */
	long m_IsRequiredLen;
	
	unsigned char m_IsViewed;	/* 
			TRUE if field is displayed in Ticket Views, FALSE if not.
		 Default: 1 SQL Type: BIT */
	long m_IsViewedLen;
	
	unsigned char m_SetDefault;	/* 
			TRUE if the default value should be assigned to all new Tickets
			created in this TicketBox, FALSE if not.
		 Default: 0 SQL Type: BIT */
	long m_SetDefaultLen;
	
	TCHAR m_DefaultValue[TICKETFIELDS_VALUE_LENGTH];	/* 
			The default value for Tickets in this TicketBox.
		 SQL Type: VARCHAR(255) */
	long m_DefaultValueLen;

} TicketFieldsTicketBox_t;

inline bool operator==(const TicketFieldsTicketBox_t& A,const TicketFieldsTicketBox_t& B)
{ return A.m_TicketFieldsTicketBoxID == B.m_TicketFieldsTicketBoxID; }
inline bool operator!=(const TicketFieldsTicketBox_t& A,const TicketFieldsTicketBox_t& B)
{ return A.m_TicketFieldsTicketBoxID != B.m_TicketFieldsTicketBoxID; }


/* struct TicketFieldsTicket_t - 
		Ticket Fields to Ticket.
	 */
typedef struct TicketFieldsTicket_t
{
	TicketFieldsTicket_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketFieldsTicket_t) );
	}
	int m_TicketFieldsTicketID;	/* 
			Uniquely identifies a ticket field Ticket combination.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketFieldsTicketIDLen;

	int m_TicketID;	/* 
			Uniquely identifies a TicketBox.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketIDLen;

	int m_TicketFieldID;	/* 
			Uniquely identifies a Ticket Field.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketFieldIDLen;

	int m_TicketFieldTypeID;
	long m_TicketFieldTypeIDLen;

	TCHAR m_DataValue[TICKETFIELDS_VALUE_LENGTH];	/* 
			The default value for Tickets in this TicketBox.
		 SQL Type: VARCHAR(255) */
	long m_DataValueLen;

} TicketFieldsTicket_t;

inline bool operator==(const TicketFieldsTicket_t& A,const TicketFieldsTicket_t& B)
{ return A.m_TicketFieldsTicketID == B.m_TicketFieldsTicketID; }
inline bool operator!=(const TicketFieldsTicket_t& A,const TicketFieldsTicket_t& B)
{ return A.m_TicketFieldsTicketID != B.m_TicketFieldsTicketID; }

/* struct TicketFieldViews_t - 
		Ticket Field Views.
	 */
typedef struct TicketFieldViews_t
{
	TicketFieldViews_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketFieldViews_t) );
	}
	int m_TicketFieldViewID;	/* 
			Uniquely identifies a ticket field view.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketFieldViewIDLen;

	int m_TicketBoxViewID;	/* 
			Uniquely identifies a TicketBoxView.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketBoxViewIDLen;

	int m_TicketFieldID;	/* 
			Uniquely identifies a Ticket Field.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketFieldIDLen;

	unsigned char m_ShowField;	/* 
			TRUE if the field should be shown in ticket list views.
		 Default: 0 SQL Type: BIT */
	long m_ShowFieldLen;	

} TicketFieldViews_t;

inline bool operator==(const TicketFieldViews_t& A,const TicketFieldViews_t& B)
{ return A.m_TicketFieldViewID == B.m_TicketFieldViewID; }
inline bool operator!=(const TicketFieldViews_t& A,const TicketFieldViews_t& B)
{ return A.m_TicketFieldViewID != B.m_TicketFieldViewID; }

/* struct MessageTracking_t - 
		Message Tracking fields.
	 */
typedef struct MessageTracking_t
{
	MessageTracking_t() // default constructor
	{
		ZeroMemory( this, sizeof(MessageTracking_t) );
	}
	int m_MessageTrackingID;	/* 
			Uniquely identifies a message tracking entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MessageTrackingIDLen;

	int m_MessageID;	/* 
			Uniquely identifies a message entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MessageIDLen;

	int m_MessageSourceID;	/* 
			Uniquely identifies a message source entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MessageSourceIDLen;

	TCHAR m_HeadMsgID[256]; /* 
			The message-id header.
		 SQL Type: VARCHAR(50) */
	long m_HeadMsgIDLen;	

	TCHAR m_HeadInReplyTo[256]; /* 
			The in-reply-to header.
		 SQL Type: VARCHAR(50) */
	long m_HeadInReplyToLen;

	TCHAR m_HeadReferences[256]; /* 
			The references header.
		 SQL Type: VARCHAR(50) */
	long m_HeadReferencesLen;

} MessageTracking_t;

inline bool operator==(const MessageTracking_t& A,const MessageTracking_t& B)
{ return A.m_MessageTrackingID == B.m_MessageTrackingID; }
inline bool operator!=(const MessageTracking_t& A,const MessageTracking_t& B)
{ return A.m_MessageTrackingID != B.m_MessageTrackingID; }


/* struct TransactionTypes_t - 
		Types of transactions
	 */
typedef struct TransactionTypes_t
{
	TransactionTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(TransactionTypes_t) );
	}
	int m_TransactionTypeID;	/* 
			Uniquely identifies the transaction type.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TransactionTypeIDLen;

	TCHAR m_Description[TRANSACTIONTYPES_DESCRIPTION_LENGTH];	/* 
			A description of the transaction.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

} TransactionTypes_t;

inline bool operator==(const TransactionTypes_t& A,const TransactionTypes_t& B)
{ return A.m_TransactionTypeID == B.m_TransactionTypeID; }
inline bool operator!=(const TransactionTypes_t& A,const TransactionTypes_t& B)
{ return A.m_TransactionTypeID != B.m_TransactionTypeID; }

/* struct Transactions_t - 
		Stores homemade transaction details. Rows are deleted when the transaction is completed.
	 */
typedef struct Transactions_t
{
	Transactions_t() // default constructor
	{
		ZeroMemory( this, sizeof(Transactions_t) );
		m_DateCreatedLen = SQL_NULL_DATA;
	}
	int m_TransactionID;	/* 
			Uniquely identifies transactions
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TransactionIDLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			When the transaction was initiated.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	int m_TransactionTypeID;	/* 
			The type of transaction
		 SQL Type: INTEGER */
	long m_TransactionTypeIDLen;

	int m_ObjectID;	/* 
			The ID of the primary object the transaction affects.
		 SQL Type: INTEGER */
	long m_ObjectIDLen;

	TCHAR m_ParameterList[TRANSACTIONS_PARAMETERLIST_LENGTH];	/* 
			List of parameters encoded in a string
		 SQL Type: VARCHAR(255) */
	long m_ParameterListLen;

	int m_Progress;	/* 
			An optional checkpoint used to indicate transaction progress.
		 SQL Type: INTEGER */
	long m_ProgressLen;

} Transactions_t;

inline bool operator==(const Transactions_t& A,const Transactions_t& B)
{ return A.m_TransactionID == B.m_TransactionID; }
inline bool operator!=(const Transactions_t& A,const Transactions_t& B)
{ return A.m_TransactionID != B.m_TransactionID; }

/* struct UIDLData_t - 
		Contains POP3 message retrieval synchronization data.
	 */
typedef struct UIDLData_t
{
	UIDLData_t() // default constructor
	{
		ZeroMemory( this, sizeof(UIDLData_t) );
	}
	int m_UIDLID;	/* 
			Uniquely identifies a UIDL entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_UIDLIDLen;

	int m_MessageSourceID;	/* 
			The message source this entry relates to.
		 SQL Type: INTEGER */
	long m_MessageSourceIDLen;

	TCHAR m_Identifier[UIDLDATA_IDENTIFIER_LENGTH];	/* 
			The server-assigned string identifier.
		 SQL Type: VARCHAR(255) */
	long m_IdentifierLen;

	unsigned char m_IsPartial;	/* 
			TRUE if message was partially downloaded, FALSE if not.
		 Default: 0 SQL Type: BIT */
	long m_IsPartialLen;

	TIMESTAMP_STRUCT m_UIDLDate;	/* 
			Date/time the UIDL was entered in DB.
		 SQL Type: DATETIME */
	long m_UIDLDateLen;

} UIDLData_t;

inline bool operator==(const UIDLData_t& A,const UIDLData_t& B)
{ return A.m_UIDLID == B.m_UIDLID; }
inline bool operator!=(const UIDLData_t& A,const UIDLData_t& B)
{ return A.m_UIDLID != B.m_UIDLID; }

/* struct VirusScanStates_t - 
		Indicates the current state of the virus scanning process.
	 */
typedef struct VirusScanStates_t
{
	VirusScanStates_t() // default constructor
	{
		ZeroMemory( this, sizeof(VirusScanStates_t) );
	}
	int m_VirusScanStateID;	/* 
			Uniquely identifies virus scan states.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_VirusScanStateIDLen;

	TCHAR m_Description[VIRUSSCANSTATES_DESCRIPTION_LENGTH];	/* 
			Human-readable description of virus scan state.
		 SQL Type: VARCHAR(55) */
	long m_DescriptionLen;

} VirusScanStates_t;

inline bool operator==(const VirusScanStates_t& A,const VirusScanStates_t& B)
{ return A.m_VirusScanStateID == B.m_VirusScanStateID; }
inline bool operator!=(const VirusScanStates_t& A,const VirusScanStates_t& B)
{ return A.m_VirusScanStateID != B.m_VirusScanStateID; }

/* struct Servers_t - 
		
	 */
typedef struct Servers_t
{
	Servers_t() // default constructor
	{
		ZeroMemory( this, sizeof(Servers_t) );
	}
	int m_ServerID;	/* 
			Uniquely identifies server.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ServerIDLen;

	TCHAR m_Description[SERVER_DESCRIPTION_LENGTH];	/* 
			Human-readable description of server description.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;

	TCHAR m_RegistrationKey[SERVER_REGISTRATION_KEY_LENGTH];	/* 
			Human-readable description of server registration key.
		 SQL Type: VARCHAR(50) */
	long m_RegistrationKeyLen;

	int m_Status;	/* 
			Uniquely identifies server.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_StatusLen;

	int m_ReloadConfig;	/* 
			Uniquely identifies server.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ReloadConfigLen;

	TIMESTAMP_STRUCT m_CheckIn;	/* 
			Date/time the alert message was sent.
		 SQL Type: DATETIME */
	long m_CheckInLen;

	TCHAR m_SpoolFolder[SERVERPARAMETERS_DATAVALUE_LENGTH];	/* 
			Human-readable description of server description.
		 SQL Type: VARCHAR(50) */
	long m_SpoolFolderLen;

	TCHAR m_TempFolder[SERVERPARAMETERS_DATAVALUE_LENGTH];	/* 
			Human-readable description of server description.
		 SQL Type: VARCHAR(50) */
	long m_TempFolderLen;

	int m_WebStatus;	/* 
			Uniquely identifies server.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_WebStatusLen;

	TIMESTAMP_STRUCT m_WebCheckIn;	/* 
			Date/time the alert message was sent.
		 SQL Type: DATETIME */
	long m_WebCheckInLen;

} Servers_t;

inline bool operator==(const Servers_t& A,const Servers_t& B)
{ return A.m_ServerID == B.m_ServerID; }
inline bool operator!=(const Servers_t& A,const Servers_t& B)
{ return A.m_ServerID != B.m_ServerID; }

/* struct ServerTaskTypes_t - 
		
	 */
typedef struct ServerTaskTypes_t
{
	ServerTaskTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(ServerTaskTypes_t) );
	}
	int m_ServerTaskTypeID;	/* 
			Uniquely identifies server task type.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ServerTaskTypeIDLen;

	TCHAR m_Description[SERVER_TASKTYPE_DESCRIPTION_LENGTH];	/* 
			Human-readable description of server task type description.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;	

} ServerTaskTypes_t;

inline bool operator==(const ServerTaskTypes_t& A,const ServerTaskTypes_t& B)
{ return A.m_ServerTaskTypeID == B.m_ServerTaskTypeID; }
inline bool operator!=(const ServerTaskTypes_t& A,const ServerTaskTypes_t& B)
{ return A.m_ServerTaskTypeID != B.m_ServerTaskTypeID; }

/* struct ServerTasks_t - 
		
	 */
typedef struct ServerTasks_t
{
	ServerTasks_t() // default constructor
	{
		ZeroMemory( this, sizeof(ServerTasks_t) );
	}
	int m_ServerTaskID;	/* 
			Uniquely identifies server task.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ServerTaskIDLen;

	int m_ServerTaskTypeID;	/* 
			Uniquely identifies server task.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ServerTaskTypeIDLen;

	int m_ServerID;	/* 
			Uniquely identifies server task.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_ServerIDLen;

} ServerTasks_t;

inline bool operator==(const ServerTasks_t& A,const ServerTasks_t& B)
{ return A.m_ServerTaskID == B.m_ServerTaskID; }
inline bool operator!=(const ServerTasks_t& A,const ServerTasks_t& B)
{ return A.m_ServerTaskID != B.m_ServerTaskID; }

/* struct VoipServers_t - 
		
	 */
typedef struct VoipServers_t
{
	VoipServers_t() // default constructor
	{
		ZeroMemory( this, sizeof(VoipServers_t) );
	}
	int m_VoipServerID;	/* 
			Uniquely identifies voip server.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_VoipServerIDLen;

	int m_VoipServerTypeID;	/* 
			Identifies voip server type.
		  */
	long m_VoipServerTypeIDLen;

	TCHAR m_Description[VOIPSERVER_DESCRIPTION_LENGTH];	/* 
			Human-readable description of voip server description.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;

	TCHAR m_Url[VOIPSERVER_URL_LENGTH];	/* 
			Human-readable url of voip server.
		 SQL Type: VARCHAR(255) */
	long m_UrlLen;

	int m_Port;	/* 
			VOIP Server API Port */
	long m_PortLen;

	unsigned char m_IsEnabled;	/* 
			Is voip server enabled?.
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	unsigned char m_IsDefault;	/* 
			Is this voip server the default server?.
		 Default: 1 SQL Type: BIT */
	long m_IsDefaultLen;

} VoipServers_t;

inline bool operator==(const VoipServers_t& A,const VoipServers_t& B)
{ return A.m_VoipServerID == B.m_VoipServerID; }
inline bool operator!=(const VoipServers_t& A,const VoipServers_t& B)
{ return A.m_VoipServerID != B.m_VoipServerID; }


/* struct VoipServerTypes_t - 
		
	 */
typedef struct VoipServerTypes_t
{
	VoipServerTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(VoipServerTypes_t) );
	}
	int m_VoipServerTypeID;	/* 
			Uniquely identifies voip server.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_VoipServerTypeIDLen;

	TCHAR m_Description[VOIPSERVER_DESCRIPTION_LENGTH];	/* 
			Human-readable description of voip server description.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;	

} VoipServerTypes_t;

inline bool operator==(const VoipServerTypes_t& A,const VoipServerTypes_t& B)
{ return A.m_VoipServerTypeID == B.m_VoipServerTypeID; }
inline bool operator!=(const VoipServerTypes_t& A,const VoipServerTypes_t& B)
{ return A.m_VoipServerTypeID != B.m_VoipServerTypeID; }

/* struct VoipExtensions_t - 
		
	 */
typedef struct VoipExtensions_t
{
	VoipExtensions_t() // default constructor
	{
		ZeroMemory( this, sizeof(VoipExtensions_t) );
	}
	int m_VoipExtensionID;	/* 
			Uniquely identifies voip server.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_VoipExtensionIDLen;

	int m_VoipServerID;	/* 
			Identifies voip server type.
		  */
	long m_VoipServerIDLen;

	int m_AgentID;	/* 
			Identifies voip server type.
		  */
	long m_AgentIDLen;

	TCHAR m_Description[VOIPSERVER_DESCRIPTION_LENGTH];	/* 
			Human-readable description of voip server description.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;

	TCHAR m_Url[VOIPSERVER_URL_LENGTH];	/* 
			Human-readable url of voip server.
		 SQL Type: VARCHAR(255) */
	long m_UrlLen;

	int m_Extension;	/* 
			VOIP Server API Port */
	long m_ExtensionLen;

	TCHAR m_Pin[VOIPSERVER_DESCRIPTION_LENGTH];	/* 
			Human-readable description of voip server description.
		 SQL Type: VARCHAR(50) */
	long m_PinLen;

	unsigned char m_IsEnabled;	/* 
			Is voip server enabled?.
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	unsigned char m_IsDefault;	/* 
			Is this voip server the default server?.
		 Default: 1 SQL Type: BIT */
	long m_IsDefaultLen;

} VoipExtensions_t;

inline bool operator==(const VoipExtensions_t& A,const VoipExtensions_t& B)
{ return A.m_VoipExtensionID == B.m_VoipExtensionID; }
inline bool operator!=(const VoipExtensions_t& A,const VoipExtensions_t& B)
{ return A.m_VoipExtensionID != B.m_VoipExtensionID; }


/* struct VoipDialingCodes_t - 
		
	 */
typedef struct VoipDialingCodes_t
{
	VoipDialingCodes_t() // default constructor
	{
		ZeroMemory( this, sizeof(VoipDialingCodes_t) );
	}
	int m_VoipDialingCodeID;	/* 
			Uniquely identifies voip server.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_VoipDialingCodeIDLen;

	int m_VoipServerID;	/* 
			Identifies voip server type.
		  */
	long m_VoipServerIDLen;

	TCHAR m_Description[VOIPSERVER_DESCRIPTION_LENGTH];	/* 
			Human-readable description of voip server description.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;

	TCHAR m_DialingCode[VOIPSERVER_URL_LENGTH];	/* 
			Human-readable url of voip server.
		 SQL Type: VARCHAR(255) */
	long m_DialingCodeLen;

	unsigned char m_IsEnabled;	/* 
			Is voip server enabled?.
		 Default: 1 SQL Type: BIT */
	long m_IsEnabledLen;

	unsigned char m_IsDefault;	/* 
			Is this voip server the default server?.
		 Default: 1 SQL Type: BIT */
	long m_IsDefaultLen;

} VoipDialingCodes_t;

inline bool operator==(const VoipDialingCodes_t& A,const VoipDialingCodes_t& B)
{ return A.m_VoipDialingCodeID == B.m_VoipDialingCodeID; }
inline bool operator!=(const VoipDialingCodes_t& A,const VoipDialingCodes_t& B)
{ return A.m_VoipDialingCodeID != B.m_VoipDialingCodeID; }

typedef struct TBReports_t
{
	TBReports_t() // default constructor
	{
		ZeroMemory( this, sizeof(TBReports_t) );
	}
		
	TCHAR Name[TICKETBOXES_NAME_LENGTH];
	long NameLen; 
	TCHAR Owner[AGENTS_NAME_LENGTH];
	long OwnerLen; 
	int nOpen;
	int nClosed;
	int nTotal;
	int nPctClosed;

} TBReports_t;

typedef struct OpenTicketReports_t
{
	OpenTicketReports_t() // default constructor
	{
		ZeroMemory( this, sizeof(OpenTicketReports_t) );
	}
		
	TCHAR m_TicketBox[TICKETBOXES_NAME_LENGTH];
	long m_TicketBoxLen;
	int m_TicketID;
	TCHAR m_Subject[TICKETS_SUBJECT_LENGTH];
	long m_SubjectLen;
	TIMESTAMP_STRUCT m_DateCreated;
	long m_DateCreatedLen;
	TCHAR m_TicketState[TICKETSTATES_DESCRIPTION_LENGTH];
	long m_TicketStateLen;
	int m_NumNotes;
	TIMESTAMP_STRUCT m_LastUpdate;
	long m_LastUpdateLen;
	TCHAR m_TicketCategory[TICKETCATEGORIES_DESCRIPTION_LENGTH];
	long m_TicketCategoryLen;
	TCHAR m_Owner[AGENTS_NAME_LENGTH];
	long m_OwnerLen;
	TCHAR m_Group[GROUPS_GROUPNAME_LENGTH];
	long m_GroupLen;
	int m_DaysCreated;
	int m_DaysUpdated;

} OpenTicketReports_t;

/* struct AutoActionTypes_t - 
		Types of auto Ticket actions such as move, close, etc.
	 */
typedef struct AutoActionTypes_t
{
	AutoActionTypes_t() // default constructor
	{
		ZeroMemory( this, sizeof(AutoActionTypes_t) );
	}
	
	int m_AutoActionTypeID;	/* 
			Uniquely identifies an auto action type.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AutoActionTypeIDLen;

	TCHAR m_TypeName[PERSONALDATATYPES_TYPENAME_LENGTH];	/* 
			Human-readable name of auto action type.
		 SQL Type: VARCHAR(64) */
	long m_TypeNameLen;

	unsigned char m_BuiltIn;	/* 
			If the type is "built-in", it's required by the system 
			(don't allow it to be deleted or edited).
		 Default: 0 SQL Type: BIT */
	long m_BuiltInLen;

} AutoActionTypes_t;

inline bool operator==(const AutoActionTypes_t& A,const AutoActionTypes_t& B)
{ return A.m_AutoActionTypeID == B.m_AutoActionTypeID; }
inline bool operator!=(const AutoActionTypes_t& A,const AutoActionTypes_t& B)
{ return A.m_AutoActionTypeID != B.m_AutoActionTypeID; }

/* struct AutoActionEvents_t - 
		Types of auto Ticket Events such as move, close, etc.
	 */
typedef struct AutoActionEvents_t
{
	AutoActionEvents_t() // default constructor
	{
		ZeroMemory( this, sizeof(AutoActionEvents_t) );
	}
	
	int m_AutoActionEventID;	/* 
			Uniquely identifies an auto action Event.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AutoActionEventIDLen;

	TCHAR m_EventName[PERSONALDATATYPES_TYPENAME_LENGTH];	/* 
			Human-readable name of auto action Event.
		 SQL Type: VARCHAR(64) */
	long m_EventNameLen;

	unsigned char m_BuiltIn;	/* 
			If the type is "built-in", it's required by the system 
			(don't allow it to be deleted or edited).
		 Default: 0 SQL Type: BIT */
	long m_BuiltInLen;

} AutoActionEvents_t;

inline bool operator==(const AutoActionEvents_t& A,const AutoActionEvents_t& B)
{ return A.m_AutoActionEventID == B.m_AutoActionEventID; }
inline bool operator!=(const AutoActionEvents_t& A,const AutoActionEvents_t& B)
{ return A.m_AutoActionEventID != B.m_AutoActionEventID; }

/* struct AutoActions_t */
typedef struct AutoActions_t
{
	AutoActions_t() // default constructor
	{
		ZeroMemory( this, sizeof(AutoActions_t) );
		m_DateCreatedLen = SQL_NULL_DATA;
	}
	int m_AutoActionID;	/* 
			Uniquely identifies the auto action entry
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AutoActionIDLen;

	int m_TicketBoxID;	/* 
			The TicketBoxID being referenced.
		 SQL Type: INTEGER */
	long m_TicketBoxIDLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			When the auto action was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	int m_AutoActionVal;	/* 
			The value of the freq.
		 SQL Type: INTEGER */
	long m_AutoActionValLen;

	int m_AutoActionFreq;	/* 
			0=min,1=hr,2=day.
		 SQL Type: INTEGER */
	long m_AutoActionFreqLen;

	int m_AutoActionEventID;	/* 
			See the AutoActionEvents table for interpretation of this field.
		 SQL Type: INTEGER */
	long m_AutoActionEventIDLen;

	int m_AutoActionTypeID;	/* 
			See the AutoActionTypes table for interpretation of this field.
		 SQL Type: INTEGER */
	long m_AutoActionTypeIDLen;

	int m_AutoActionTargetID;	/* 
			The targetid of the AutoActionType.
		 SQL Type: INTEGER */
	long m_AutoActionTargetIDLen;	

} AutoActions_t;

inline bool operator==(const AutoActions_t& A,const AutoActions_t& B)
{ return A.m_AutoActionID == B.m_AutoActionID; }
inline bool operator!=(const AutoActions_t& A,const AutoActions_t& B)
{ return A.m_AutoActionID != B.m_AutoActionID; }

/* struct OfficeHours_t - 
		Table of office hours
	 */
typedef struct OfficeHours_t
{
	OfficeHours_t() // default constructor
	{
		ZeroMemory( this, sizeof(OfficeHours_t) );
	}
	int m_OfficeHourID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_OfficeHourIDLen;

	int m_TypeID;	/*
		 Default: 0 SQL Type: INTEGER */
	long m_TypeIDLen;

	int m_ActualID;	/* 			
		 Default: 0 SQL Type: INTEGER */
	long m_ActualIDLen;

	int m_StartHr;	/*
		 Default: 0 SQL Type: INTEGER */
	long m_StartHrLen;

	int m_StartMin;	/* 
		 Default: 0 SQL Type: INTEGER */
	long m_StartMinLen;

	int m_StartAmPm;	/*
		 Default: 0 SQL Type: INTEGER */
	long m_StartAmPmLen;

	int m_EndHr;	/* 
		 Default: 0 SQL Type: INTEGER */
	long m_EndHrLen;

	int m_EndMin;	/* 
		 Default: 0 SQL Type: INTEGER */
	long m_EndMinLen;

	int m_EndAmPm;	/* 
		 Default: 0 SQL Type: INTEGER */
	long m_EndAmPmLen;

	int m_DayID;	/* 
		 Default: 0 SQL Type: INTEGER */
	long m_DayIDLen;

	TCHAR m_Description[TICKETS_SUBJECT_LENGTH];
	long m_DescriptionLen;
	
	TIMESTAMP_STRUCT m_TimeStart;	/*			
		 ALLOW NULLS SQL Type: DATETIME */
	long m_TimeStartLen;

	TIMESTAMP_STRUCT m_TimeEnd;	/*			
		 ALLOW NULLS SQL Type: DATETIME */
	long m_TimeEndLen;

} OfficeHours_t;

inline bool operator==(const OfficeHours_t& A,const OfficeHours_t& B)
{ return A.m_OfficeHourID == B.m_OfficeHourID; }
inline bool operator!=(const OfficeHours_t& A,const OfficeHours_t& B)
{ return A.m_OfficeHourID != B.m_OfficeHourID; }

/* struct TicketLinks_t - 
		Ticket Links.
	 */
typedef struct TicketLinks_t
{
	TicketLinks_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketLinks_t) );
	}
	
	int m_TicketLinkID;	/* 
			Uniquely identifies an Ticket Link.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketLinkIDLen;

	TCHAR m_LinkName[LINKNAME_LENGTH];	/* 
			Human-readable name of Ticket Link.
		 SQL Type: VARCHAR(50) */
	long m_LinkNameLen;

	TIMESTAMP_STRUCT m_DateCreated;	/* 
			When the Ticket Link was created.
		 ALLOW NULLS SQL Type: DATETIME */
	long m_DateCreatedLen;

	int m_OwnerID;	/* 
			Default: 0 SQL Type: INTEGER */
	long m_OwnerIDLen;

	int m_IsDeleted;	/* 
			Default: 0 SQL Type: INTEGER */
	long m_IsDeletedLen;

} TicketLinks_t;

inline bool operator==(const TicketLinks_t& A,const TicketLinks_t& B)
{ return A.m_TicketLinkID == B.m_TicketLinkID; }
inline bool operator!=(const TicketLinks_t& A,const TicketLinks_t& B)
{ return A.m_TicketLinkID != B.m_TicketLinkID; }

/* struct TicketLinksTicket_t - 
		Ties TicketLinks to Tickets.
	 */
typedef struct TicketLinksTicket_t
{
	TicketLinksTicket_t() // default constructor
	{
		ZeroMemory( this, sizeof(TicketLinksTicket_t) );
	}
	
	int m_TicketLinksTicketID;	/* 
			Uniquely identifies an Ticket Link.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TicketLinksTicketIDLen;

	int m_TicketLinkID;	/* 
			Default: 0 SQL Type: INTEGER */
	long m_TicketLinkIDLen;

	int m_TicketID;	/* 
			Default: 0 SQL Type: INTEGER */
	long m_TicketIDLen;

} TicketLinksTicket_t;

inline bool operator==(const TicketLinksTicket_t& A,const TicketLinksTicket_t& B)
{ return A.m_TicketLinksTicketID == B.m_TicketLinksTicketID; }
inline bool operator!=(const TicketLinksTicket_t& A,const TicketLinksTicket_t& B)
{ return A.m_TicketLinksTicketID != B.m_TicketLinksTicketID; }

/* struct AgentContacts_t - 
		Table of agent contacts
	 */
typedef struct AgentContacts_t
{
	AgentContacts_t() // default constructor
	{
		ZeroMemory( this, sizeof(AgentContacts_t) );
	}
	int m_AgentContactID;	/* 
			Uniquely identifies an entry.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_AgentContactIDLen;

	int m_AgentID;	/*
		 Default: 0 SQL Type: INTEGER */
	long m_AgentIDLen;

	int m_ContactID;	/* 			
		 Default: 0 SQL Type: INTEGER */
	long m_ContactIDLen;	

} AgentContacts_t;

inline bool operator==(const AgentContacts_t& A,const AgentContacts_t& B)
{ return A.m_AgentContactID == B.m_AgentContactID; }
inline bool operator!=(const AgentContacts_t& A,const AgentContacts_t& B)
{ return A.m_AgentContactID != B.m_AgentContactID; }

/* struct TimeZones_t - 
		Contains Time Zones. 
	 */
typedef struct TimeZones_t
{
	TimeZones_t() // default constructor
	{
		ZeroMemory( this, sizeof(TimeZones_t) );
	}
	int m_TimeZoneID;	/* 
			Uniquely identifies a Time Zone.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_TimeZoneIDLen;

	TCHAR m_DisplayName[TIMEZONES_DISPLAYNAME_LENGTH];	/* 
			Display Name.
		 SQL Type: VARCHAR(100) */
	long m_DisplayNameLen;

	TCHAR m_StandardName[TIMEZONES_STANDARDNAME_LENGTH];	/* 
			Standard Name.
		 SQL Type: VARCHAR(100) */
	long m_StandardNameLen;

	int m_UTCOffset;	/* 
			Default: 0 SQL Type: INTEGER */
	long m_UTCOffsetLen;

} TimeZones_t;

inline bool operator==(const TimeZones_t& A,const TimeZones_t& B)
{ return A.m_TimeZoneID == B.m_TimeZoneID; }
inline bool operator!=(const TimeZones_t& A,const TimeZones_t& B)
{ return A.m_TimeZoneID != B.m_TimeZoneID; }

/* struct Dictionary_t - 
		Contains Dictionaries. 
	 */
typedef struct Dictionary_t
{
	Dictionary_t() // default constructor
	{
		ZeroMemory( this, sizeof(Dictionary_t) );
	}
	int m_DictionaryID;	/* 
			Uniquely identifies a Dictionary.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_DictionaryIDLen;

	TCHAR m_LanguageID[DICTIONARY_LANGUAGEID_LENGTH];	/* 
			Two character Language ID.
		 SQL Type: VARCHAR(2) */
	long m_LanguageIDLen;

	TCHAR m_Description[DICTIONARY_DESCRIPTION_LENGTH];	/* 
			Description.
		 SQL Type: VARCHAR(50) */
	long m_DescriptionLen;

	TCHAR m_TlxFile[DICTIONARY_FILE_LENGTH];	/* 
			TLX File.
		 SQL Type: VARCHAR(20) */
	long m_TlxFileLen;

	TCHAR m_ClxFile[DICTIONARY_FILE_LENGTH];	/* 
			CLX File.
		 SQL Type: VARCHAR(20) */
	long m_ClxFileLen;

	unsigned char m_IsEnabled;	/* 
			Dictionary is enabled for selection?
		 Default: 0 SQL Type: BIT */
	long m_IsEnabledLen;

} Dictionary_t;

inline bool operator==(const Dictionary_t& A,const Dictionary_t& B)
{ return A.m_DictionaryID == B.m_DictionaryID; }
inline bool operator!=(const Dictionary_t& A,const Dictionary_t& B)
{ return A.m_DictionaryID != B.m_DictionaryID; }

/* struct OAuthHosts_t - 
		Contains OAuth Hosts. 
	 */
typedef struct OAuthHosts_t
{
	OAuthHosts_t() // default constructor
	{
		ZeroMemory( this, sizeof(OAuthHosts_t) );
	}
	int m_OAuthHostID;	/* 
			Uniquely identifies an OAuth Host.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_OAuthHostIDLen;

	TCHAR m_Description[OAUTH_LENGTH];	/* 
			Description.
		 SQL Type: VARCHAR(255) */
	long m_DescriptionLen;

	TCHAR m_AuthEndPoint[OAUTH_LENGTH];	/* 
			AuthEndPoint.
		 SQL Type: VARCHAR(255) */
	long m_AuthEndPointLen;

	TCHAR m_TokenEndPoint[OAUTH_LENGTH];	/* 
			TokenEndPoint.
		 SQL Type: VARCHAR(255) */
	long m_TokenEndPointLen;

	TCHAR m_ClientID[OAUTH_LENGTH];	/* 
			ClientID.
		 SQL Type: VARCHAR(255) */
	long m_ClientIDLen;

	TCHAR m_ClientSecret[OAUTH_LENGTH];	/* 
			Client Secret.
		 SQL Type: VARCHAR(255) */
	long m_ClientSecretLen;

	TCHAR m_Scope[OAUTH_SCOPE_LENGTH];	/* 
			Scope.
		 SQL Type: VARCHAR(255) */
	long m_ScopeLen;

} OAuthHosts_t;

inline bool operator==(const OAuthHosts_t& A,const OAuthHosts_t& B)
{ return A.m_OAuthHostID == B.m_OAuthHostID; }
inline bool operator!=(const OAuthHosts_t& A,const OAuthHosts_t& B)
{ return A.m_OAuthHostID != B.m_OAuthHostID; }

#endif // DATA_CLASSES_H
