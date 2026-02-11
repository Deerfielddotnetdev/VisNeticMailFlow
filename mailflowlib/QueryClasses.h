////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMLIBRARY/QueryClasses.h,v 1.1.4.1 2005/12/07 14:49:35 markm Exp $
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

#ifndef EMS_T_CLASSES_H
#define EMS_T_CLASSES_H

/* class TAccessControl - 
		Controls access to various objects from specific agents and groups.
	 */
class TAccessControl : public AccessControl_t
{
public:
	int& ID(void) { return m_AccessControlID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAgentGroupings - 
		Segments agents into logical groups.
	 */
class TAgentGroupings : public AgentGroupings_t
{
public:
	int& ID(void) { return m_AgentGroupingID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAgents - 
		Agents are users of the system. Agents can represent 
		any level of user (e.g. an agent, supervisor, etc.).
	 */
class TAgents : public Agents_t
{
public:
	int& ID(void) { return m_AgentID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAlertEvents - 
		The list of events that trigger alerts.
	 */
class TAlertEvents : public AlertEvents_t
{
public:
	int& ID(void) { return m_AlertEventID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAlertMethods - 
		The list of methods used to send an alert.
	 */
class TAlertMethods : public AlertMethods_t
{
public:
	int& ID(void) { return m_AlertMethodID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAlertMsgs - 
		Contains alerts messages.
	 */
class TAlertMsgs : public AlertMsgs_t
{
public:
	int& ID(void) { return m_AlertMsgID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAlerts - 
		Active alert definitions.
	 */
class TAlerts : public Alerts_t
{
public:
	int& ID(void) { return m_AlertID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TApprovalObjectTypes - 
		This table identifies the various types of objects that can be channeled 
		through the approval process. Examples include:
		Messages, Standard responses, etc.
	 */
class TApprovalObjectTypes : public ApprovalObjectTypes_t
{
public:
	int& ID(void) { return m_ApprovalObjectTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TApprovals - 
		In scenarios where agents require approvals for specifc actions (e.g. sending a message), 
		this table determine who is responsible for approving the action (either a user/agent or group).
	 */
class TApprovals : public Approvals_t
{
public:
	int& ID(void) { return m_ApprovalID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TArchives - 
		Used to track archive/purge operations.
	 */
class TArchives : public Archives_t
{
public:
	int& ID(void) { return m_ArchiveID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAttachments - 
		Attachments
	 */
class TAttachments : public Attachments_t
{
public:
	int& ID(void) { return m_AttachmentID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TContactNotes - 
		Contact notes will be used in a similar capactiy as TicketNotes, 
		both by users for collaboration purposes and by the system directly. 
		The system may, for example, create a contact note when the contact is merged with another.
	 */
class TContactNotes : public ContactNotes_t
{
public:
	int& ID(void) { return m_ContactNoteID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TContacts - 
		Contacts are customers or external entities.
	 */
class TContacts : public Contacts_t
{
public:
	int& ID(void) { return m_ContactID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TContactGroups - 
		Groups of contacts.
	 */
class TContactGroups : public ContactGroups_t
{
public:
	int& ID(void) { return m_ContactGroupID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TContactGroupings - 
		Segments contacts into logical groups.
	 */
class TContactGrouping : public ContactGrouping_t
{
public:
	int& ID(void) { return m_ContactGroupingID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TCustomDictionary - 
		Contains global and user custom dictionaries.
	 */
class TCustomDictionary : public CustomDictionary_t
{
public:
	int& ID(void) { return m_CustomDictionaryID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TForwardCCAddresses - 
		Table of "cc" addresses used when forwarding messasges matched by the routing rules.
	 */
class TForwardCCAddresses : public ForwardCCAddresses_t
{
public:
	int& ID(void) { return m_AddressID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TForwardToAddresses - 
		Table of "to" addresses used when forwarding messages matched by the routing rules.
	 */
class TForwardToAddresses : public ForwardToAddresses_t
{
public:
	int& ID(void) { return m_AddressID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TCustomAgeAlerts - 
		Table of age alerted ticketboxes used when processing custom age alerts.
	 */
class TCustomAgeAlerts : public CustomAgeAlerts_t
{
public:
	int& ID(void) { return m_AgeAlertID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAssignToAgents - 
		Table of Agents used by the routing rules.
	 */
class TAssignToAgents : public AssignToAgents_t
{
public:
	int& ID(void) { return m_AgentID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAssignToTicketBoxes - 
		Table of ticketboxes used when assigning to multiple the routing rules.
	 */
class TAssignToTicketBoxes : public AssignToTicketBoxes_t
{
public:
	int& ID(void) { return m_TicketBoxID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TGroups - 
		Groups of agents.
	 */
class TGroups : public Groups_t
{
public:
	int& ID(void) { return m_GroupID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TIPRanges - 
		Defines ranges of IP addresses for login restrictions.
	 */
class TIPRanges : public IPRanges_t
{
public:
	int& ID(void) { return m_IPRangeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TInboundMessageAttachments - 
		Attachments to inbound messages.
	 */
class TInboundMessageAttachments : public InboundMessageAttachments_t
{
public:
	int& ID(void) { return m_InboundMessageAttachmentID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TInboundMessageQueue - 
		Queue of inbound messages retrieved by the email gateway service.
	 */
class TInboundMessageQueue : public InboundMessageQueue_t
{
public:
	int& ID(void) { return m_InboundMessageQueueID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TInboundMessages - 
		Table of inbound messages.
	 */
class TInboundMessages : public InboundMessages_t
{
public:
	int& ID(void) { return m_InboundMessageID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TInboundMessageRead : public InboundMessageRead_t
{
public:
	int& ID(void) { return m_InboundMessageReadID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TSRKeywordResults : public SRKeywordResults_t
{
public:
	int& ID(void) { return m_SRKeywordResultsID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TTicketNotesRead : public TicketNotesRead_t
{
public:
	int& ID(void) { return m_TicketNotesReadID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TFolders : public Folders_t
{
public:
	int& ID(void) { return m_FolderID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TInstantMessages - 
		Where instant messages are stored.
	 */
class TInstantMessages : public InstantMessages_t
{
public:
	int& ID(void) { return m_InstantMessageID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TLogEntryTypes - 
		Types of log entries that can be turned on or off.
	 */
class TLogEntryTypes : public LogEntryTypes_t
{
public:
	int& ID(void) { return m_LogEntryTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TLog - 
		Enumerates the log entries.
	 */
class TLog : public Log_t
{
public:
	int& ID(void) { return m_LogID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int DeleteByServer( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
	void PrepareListByServer( CODBCQuery& query );
};

/* class TLogSeverity - 
		Enumerates the severity levels associated with log entries.
	 */
class TLogSeverity : public LogSeverity_t
{
public:
	int& ID(void) { return m_LogSeverityID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TMatchFromAddresses - 
		Allows routing rules to match one or more "from" email addresses.
	 */
class TMatchFromAddresses : public MatchFromAddresses_t
{
public:
	int& ID(void) { return m_MatchID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TMatchFromAddressP - 
		Allows routing rules to match one or more "from" email addresses.
	 */
class TMatchFromAddressP : public MatchFromAddressP_t
{
public:
	int& ID(void) { return m_MatchID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TMatchText - 
		Contains text which routing rules attempt to match in various locations (e.g. msg body, subject, etc.).
	 */
class TMatchText : public MatchText_t
{
public:
	int& ID(void) { return m_MatchID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TMatchTextP - 
		Contains text which routing rules attempt to match in various locations (e.g. msg body, subject, etc.).
	 */
class TMatchTextP : public MatchTextP_t
{
public:
	int& ID(void) { return m_MatchID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TMatchToAddresses - 
		Table of "to" addresses to be matched by the routing rules.
	 */
class TMatchToAddresses : public MatchToAddresses_t
{
public:
	int& ID(void) { return m_MatchID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TMatchToAddressP - 
		Table of "to" addresses to be matched by the routing rules.
	 */
class TMatchToAddressP : public MatchToAddressP_t
{
public:
	int& ID(void) { return m_MatchID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TMessageDestinations - 
		Message destinations represent outbound message gateways.
	 */
class TMessageDestinations : public MessageDestinations_t
{
public:
	int& ID(void) { return m_MessageDestinationID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TMessageSourceTypes - 
 	     Classifies message sources for routing purposes
	 */
class TMessageSourceTypes : public MessageSourceTypes_t
{
public:
	int& ID(void) { return m_MessageSourceTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TMessageSources - 
		Designates sources for email messages.
	 */
class TMessageSources : public MessageSources_t
{
public:
	int& ID(void) { return m_MessageSourceID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TMsgApprovalQueue - 
		Queues message approval requests.
	 */
class TMsgApprovalQueue : public MsgApprovalQueue_t
{
public:
	int& ID(void) { return m_MsgApprovalQueueID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TObjectTypes - 
		Object types describe the various types of securable objects in the system.
	 */
class TObjectTypes : public ObjectTypes_t
{
public:
	int& ID(void) { return m_ObjectTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TObjects - 
		Securable objects.
	 */
class TObjects : public Objects_t
{
public:
	int& ID(void) { return m_ObjectID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TOutboundMessageAttachments - 
		Attachments to outbound messages.
	 */
class TOutboundMessageAttachments : public OutboundMessageAttachments_t
{
public:
	int& ID(void) { return m_OutboundMessageAttachmentID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TNoteAttachments - 
		Attachments to notes.
	 */
class TNoteAttachments : public NoteAttachments_t
{
public:
	int& ID(void) { return m_NoteAttachmentID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TOutboundMessageContacts - 
		Relates ticket boxes to contacts.
	 */
class TOutboundMessageContacts : public OutboundMessageContacts_t
{
public:
	int& ID(void) { return m_OutboundMessageContactID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TOutboundMessageQueue - 
		Outbound message queue.
	 */
class TOutboundMessageQueue : public OutboundMessageQueue_t
{
public:
	int& ID(void) { return m_OutBoundMessageQueueID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TOutboundMessageStates - 
		Message states will include the following:
		Draft, Sent, Untouched, Retry, Failed
	 */
class TOutboundMessageStates : public OutboundMessageStates_t
{
public:
	int& ID(void) { return m_OutboundMsgStateID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TOutboundMessageTypes - 
		Describes different types of outbound messages.
	 */
class TOutboundMessageTypes : public OutboundMessageTypes_t
{
public:
	int& ID(void) { return m_OutboundMessageTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TOutboundMessages - 
		Messages created in the system.
	 */
class TOutboundMessages : public OutboundMessages_t
{
public:
	int& ID(void) { return m_OutboundMessageID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TPersonalData - 
		Contains personal information such as email addresses, phone numbers
		and custom data that is related to a specific contact or agent. 
	 */
class TPersonalData : public PersonalData_t
{
public:
	int& ID(void) { return m_PersonalDataID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TPersonalDataTypes - 
		Types of personal data such as email, business phone, ICQ# etc.
	 */
class TPersonalDataTypes : public PersonalDataTypes_t
{
public:
	int& ID(void) { return m_PersonalDataTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TBypass - 
		Contains items that will be bypassed by various functions. 
	 */
class TBypass : public Bypass_t
{
public:
	int& ID(void) { return m_BypassID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TEmail - 
		Contains items that will be bypassed by various functions. 
	 */
class TEmail : public Email_t
{
public:
	int& ID(void) { return m_EmailID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TAutoActionTypes : public AutoActionTypes_t
{
public:
	int& ID(void) { return m_AutoActionTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TAutoActionEvents : public AutoActionEvents_t
{
public:
	int& ID(void) { return m_AutoActionEventID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TAutoActions : public AutoActions_t
{
public:
	int& ID(void) { return m_AutoActionID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TAgentContacts : public AgentContacts_t
{
public:
	int& ID(void) { return m_AgentContactID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TOfficeHours : public OfficeHours_t
{
public:
	int& ID(void) { return m_OfficeHourID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketCategories - 
		Custom ticket categories.
	 */
class TTicketCategories : public TicketCategories_t
{
public:
	int& ID(void) { return m_TicketCategoryID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketBoxOwners - 
		TicketBox Owners.
	 */
class TTicketBoxOwners : public TicketBoxOwners_t
{
public:
	int& ID(void) { return m_TicketBoxOwnerID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketBoxTicketBoxOwners - 
		TicketBox Owners.
	 */
class TTicketBoxTicketBoxOwners : public TicketBoxTicketBoxOwners_t
{
public:
	int& ID(void) { return m_TbTboID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketBoxRouting - 
		TicketBox Routing.
	 */
class TTicketBoxRouting : public TicketBoxRouting_t
{
public:
	int& ID(void) { return m_TicketBoxRoutingID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAutoMessages - 
		Auto Messages.
	 */
class TAutoMessages : public AutoMessages_t
{
public:
	int& ID(void) { return m_AutoMessageID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAutoMessagesSent - 
		Custom Age Alerts.
	 */
class TAutoMessagesSent : public AutoMessagesSent_t
{
public:
	int& ID(void) { return m_AutoMessageSentID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAutoResponses - 
		Auto Responses.
	 */
class TAutoResponses : public AutoResponses_t
{
public:
	int& ID(void) { return m_AutoResponseID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAutoResponses - 
		Custom Age Alerts.
	 */
class TAutoResponsesSent : public AutoResponsesSent_t
{
public:
	int& ID(void) { return m_AutoResponseSentID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAgeAlerts - 
		Age Alerts.
	 */
class TAgeAlerts : public AgeAlerts_t
{
public:
	int& ID(void) { return m_AgeAlertID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAgeAlertsSent - 
		Custom Age Alerts.
	 */
class TAgeAlertsSent : public AgeAlertsSent_t
{
public:
	int& ID(void) { return m_AgeAlertSentID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TWaterMarkAlerts - 
		Custom WaterMark Alerts.
	 */
class TWaterMarkAlerts : public WaterMarkAlerts_t
{
public:
	int& ID(void) { return m_WaterMarkAlertID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TPriorities - 
		Priority levels assignable to tickets and messages.
		Examples include: low, medium-low, medium, medium-high, high.
	 */
class TPriorities : public Priorities_t
{
public:
	int& ID(void) { return m_PriorityID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TRoutingRules - 
		Message routing rules.
	 */
class TRoutingRules : public RoutingRules_t
{
public:
	int& ID(void) { return m_RoutingRuleID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TProcessingRules - 
		Message processing rules.
	 */
class TProcessingRules : public ProcessingRules_t
{
public:
	int& ID(void) { return m_ProcessingRuleID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TDateFilters - 
		Date filters.
	 */
class TDateFilters : public DateFilters_t
{
public:
	int& ID(void) { return m_DateFilterID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TScheduleReport - 
		Report Scheduler.
	 */
class TScheduleReport : public ScheduleReport_t
{
public:
	int& ID(void) { return m_ScheduledReportID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
	void PrepareOwnerList( CODBCQuery& query );
};

/* class TReportTypes - 
		Report Types.
	 */
class TReportTypes : public ReportTypes_t
{
public:
	int& ID(void) { return m_ReportTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TReportObjects - 
		Report Objects.
	 */
class TReportObjects : public ReportObjects_t
{
public:
	int& ID(void) { return m_ReportObjectID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};


class TReportResults : public ReportResults_t
{
public:
	int& ID(void) { return m_ReportResultID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	void Query( CODBCQuery& query );
	int Query2( CODBCQuery& query );
	int QueryOne( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );	
};

class TReportResultRows : public ReportResultRows_t
{
public:
	int& ID(void) { return m_ReportResultRowsID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	void Query( CODBCQuery& query );
	int Query2( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );	
};

class TStandardReport : public StandardReport_t
{
public:
	int& ID(void) { return m_StandardReportID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );	
};

class TCustomReport : public CustomReport_t
{
public:
	int& ID(void) { return m_CustomReportID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );	
};

/* class TServerParameters - 
		Defines global server parameters.
	 */
class TServerParameters : public ServerParameters_t
{
public:
	int& ID(void) { return m_ServerParameterID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TSignatures - 
		Signatures used by groups and agents.
	 */
class TSignatures : public Signatures_t
{
public:
	int& ID(void) { return m_SignatureID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TStandardResponseUsage - 
		Logs usage of standard responses by specific agents.
	 */
class TStandardResponseUsage : public StandardResponseUsage_t
{
public:
	int& ID(void) { return m_StdResponseUsageID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TStandardResponses - 
		Standard responses
	 */
class TStandardResponses : public StandardResponses_t
{
public:
	int& ID(void) { return m_StandardResponseID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TStdRespApprovalQueue - 
		Queues the standard response approval requests. 
		Std responses will not become active until they have been approved by 
		a designated "approver" (as indicated by the Approvals table).
		If an approval is required on a std response edit (versus a new std response), 
		the EditsStdRespID table column is populated with the ID of the std response 
		to be replaced once the edit is approved.
	 */
class TStdRespApprovalQueue : public StdRespApprovalQueue_t
{
public:
	int& ID(void) { return m_StdRespApprovalQueueID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TStdResponseAttachments - 
		Allows attachments to relate directly to statndard responses.
	 */
class TStdResponseAttachments : public StdResponseAttachments_t
{
public:
	int& ID(void) { return m_StdResponseAttachID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TStdResponseCategories - 
		Organizes the standard responses into groups.
	 */
class TStdResponseCategories : public StdResponseCategories_t
{
public:
	int& ID(void) { return m_StdResponseCatID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TStdResponseFavorites - 
		Logs usage of standard responses by specific agents.
	 */
class TStdResponseFavorites : public StdResponseFavorites_t
{
public:
	int& ID(void) { return m_StdResponseFavoritesID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TStyleSheets - 
		The list of style sheets available.
	 */
class TStyleSheets : public StyleSheets_t
{
public:
	int& ID(void) { return m_StyleSheetID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTaskList - 
		Agent-level task list.
	 */
class TTaskList : public TaskList_t
{
public:
	int& ID(void) { return m_TaskID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTaskPriorities - 
		Priorities of tasks. Examples include: low,	normal,	and high.
	 */
class TTaskPriorities : public TaskPriorities_t
{
public:
	int& ID(void) { return m_TaskPriorityID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTaskStatuses - 
		Status of current specific task. 
		Examples: Complete, In progress, Not started, Deferred, Waiting on someone else.
	 */
class TTaskStatuses : public TaskStatuses_t
{
public:
	int& ID(void) { return m_TaskStatusID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketActions - 
		Enumerates the different ticket actions stored in the TicketHistory table
	 */
class TTicketActions : public TicketActions_t
{
public:
	int& ID(void) { return m_TicketActionID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketBoxFooters - 
		Footers used by ticket boxes.
	 */
class TTicketBoxFooters : public TicketBoxFooters_t
{
public:
	int& ID(void) { return m_FooterID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketBoxHeaders - 
		Headers used by ticket boxes.
	 */
class TTicketBoxHeaders : public TicketBoxHeaders_t
{
public:
	int& ID(void) { return m_HeaderID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketBoxViewTypes - 
		The types of ticket box views applicable to specific ticket boxes.
	 */
class TTicketBoxViewTypes : public TicketBoxViewTypes_t
{
public:
	int& ID(void) { return m_TicketBoxViewTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketBoxViews - 
		Whenever a ticket box is created for (or made accessible to) an agent, a 
		TicketBoxView must be created that represents that agent's view into the ticket box.
		View-related parameters, such as sort order, sort field, and the columns to display, 
		must be maintained within this table to ensure a consistent representation of views 
		to the agent regardless of which computer he/she uses.
	 */
class TTicketBoxViews : public TicketBoxViews_t
{
public:
	int& ID(void) { return m_TicketBoxViewID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketBoxes - 
		Ticket boxes.
	 */
class TTicketBoxes : public TicketBoxes_t
{
public:
	int& ID(void) { return m_TicketBoxID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketContacts - 
		Relates ticket boxes to contacts.
	 */
class TTicketContacts : public TicketContacts_t
{
public:
	int& ID(void) { return m_TicketContactID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketHistory - 
		Contains history information about the ticket
	 */
class TTicketHistory : public TicketHistory_t
{
public:
	int& ID(void) { return m_TicketHistoryID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketNotes - 
		Allows for multiple notes to be related to tickets. 
		Some ticket notes may be generated by the system to 
		indicate transfer of ownership, etc.
	 */
class TTicketNotes : public TicketNotes_t
{
public:
	int& ID(void) { return m_TicketNoteID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketStates - 
		The state of the ticket. Examples include:
		open, closed, and escalated.
	 */
class TTicketStates : public TicketStates_t
{
public:
	int& ID(void) { return m_TicketStateID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTickets - 
		Tickets represent a logical dialog with a specific contact. 
		Tickets can be thought of as "incidents", and should relate to a 
		specific issue, premise, or problem.
	 */
class TTickets : public Tickets_t
{
public:
	int& ID(void) { return m_TicketID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketLinks - 
		Ticket Links.
	 */
class TTicketLinks : public TicketLinks_t
{
public:
	int& ID(void) { return m_TicketLinkID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketLinksTicket - 
		Ticket Links to Ticket definitions.
	 */
class TTicketLinksTicket : public TicketLinksTicket_t
{
public:
	int& ID(void) { return m_TicketLinksTicketID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	int QueryByTB( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAgentActivity - 
		Agent Activity Audit.
	 */
class TAgentActivity : public AgentActivity_t
{
public:
	int& ID(void) { return m_AgentActivityID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAgentActivities - 
		Agent Activities.
	 */
class TAgentActivities : public AgentActivities_t
{
public:
	int& ID(void) { return m_AgentActivityID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TAgentTicketCategories - 
		Agent Ticket Categories.
	 */
class TAgentTicketCategories : public AgentTicketCategories_t
{
public:
	int& ID(void) { return m_AgentTicketCategoryID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketFields - 
		Ticket Fields definitions.
	 */
class TTicketFields : public TicketFields_t
{
public:
	int& ID(void) { return m_TicketFieldID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketFieldTypes - 
		Ticket Field Types definitions.
	 */
class TTicketFieldTypes : public TicketFieldTypes_t
{
public:
	int& ID(void) { return m_TicketFieldTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketFieldOptions - 
		Ticket Field Option definitions.
	 */
class TTicketFieldOptions : public TicketFieldOptions_t
{
public:
	int& ID(void) { return m_TicketFieldOptionID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketFieldsTicketBox - 
		Ticket Fields to TicketBox definitions.
	 */
class TTicketFieldsTicketBox : public TicketFieldsTicketBox_t
{
public:
	int& ID(void) { return m_TicketFieldsTicketBoxID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	int QueryByTB( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketFieldsTicket - 
		Ticket Fields to Ticket definitions.
	 */
class TTicketFieldsTicket : public TicketFieldsTicket_t
{
public:
	int& ID(void) { return m_TicketFieldsTicketID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTicketFieldViews - 
		Ticket Field Views.
	 */
class TTicketFieldViews : public TicketFieldViews_t
{
public:
	int& ID(void) { return m_TicketFieldViewID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTransactionTypes - 
		Types of transactions
	 */
class TTransactionTypes : public TransactionTypes_t
{
public:
	int& ID(void) { return m_TransactionTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TTransactions - 
		Stores homemade transaction details. Rows are deleted when the transaction is completed.
	 */
class TTransactions : public Transactions_t
{
public:
	int& ID(void) { return m_TransactionID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TUIDLData - 
		Contains POP3 message retrieval synchronization data.
	 */
class TUIDLData : public UIDLData_t
{
public:
	int& ID(void) { return m_UIDLID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TVirusScanStates - 
		Indicates the current state of the virus scanning process.
	 */
class TVirusScanStates : public VirusScanStates_t
{
public:
	int& ID(void) { return m_VirusScanStateID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TServers : public Servers_t
{
public:
	int& ID(void) { return m_ServerID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TServerTaskTypes : public ServerTaskTypes_t
{
public:
	int& ID(void) { return m_ServerTaskTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TServerTasks : public ServerTasks_t
{
public:
	int& ID(void) { return m_ServerTaskID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TVoipServers : public VoipServers_t
{
public:
	int& ID(void) { return m_VoipServerID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TVoipServerTypes : public VoipServerTypes_t
{
public:
	int& ID(void) { return m_VoipServerTypeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TVoipExtensions : public VoipExtensions_t
{
public:
	int& ID(void) { return m_VoipExtensionID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TVoipDialingCodes : public VoipDialingCodes_t
{
public:
	int& ID(void) { return m_VoipDialingCodeID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

/* class TBypass - 
		Contains items that will be bypassed by various functions. 
	 */
class TTimeZones : public TimeZones_t
{
public:
	int& ID(void) { return m_TimeZoneID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TDictionary : public Dictionary_t
{
public:
	int& ID(void) { return m_DictionaryID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TMessageTracking : public MessageTracking_t
{
public:
	int& ID(void) { return m_MessageTrackingID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

class TOAuthHosts : public OAuthHosts_t
{
public:
	int& ID(void) { return m_OAuthHostID; }
	void Insert( CODBCQuery& query );
	int Update( CODBCQuery& query );
	int Delete( CODBCQuery& query );
	int Query( CODBCQuery& query );
	void GetLongData( CODBCQuery& query );
	void PrepareList( CODBCQuery& query );
};

#endif // EMS_T_CLASSES_H
