/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Demo.cpp,v 1.2.2.1.2.1 2006/07/18 12:55:02 markm Exp $
||
||
||                                         
||  COMMENTS:
||              
\\*************************************************************************/
#include "stdafx.h"
#include ".\demo.h"
#include <time.h>
#include "TicketHistoryFns.h"
#include "TicketBoxViewFns.h"
#include "StringFns.h"
#include "RegistryFns.h"
#include "TicketHistoryFns.h"
#include "Ticket.h"
#include "InboundMessage.h"
#include "OutboundMessage.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CDemo::CDemo(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	// handle exceptions in the local EMS file
	GetISAPIData().SetUseExceptionEMS(false);
	
	// act as the administrator for this request
	m_ISAPIData.m_pSession->m_AgentID = EMS_AGENTID_ADMINISTRATOR;

	// set agent vars
	m_HasLoggedIn = 1;
	m_ShowMessagesInbound = 0;
	m_EscalateToAgentID = 2;
	m_DefaultSignatureID = 3;
	_tcscpy( m_ReplyQuotedPrefix, _T(">") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Destruction	              
\*--------------------------------------------------------------------------*/
CDemo::~CDemo(void)
{
}

/*---------------------------------------------------------------------------\             
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CDemo::Run( CURLAction& action )
{
	try
	{
		tstring sAction;
		GetISAPIData().GetFormString( _T("ACTION"), sAction, true );
		
		if ( sAction.compare( _T("create") ) == 0 )
		{
			GetQuery().Initialize();
			GetQuery().Execute( _T("BEGIN TRANSACTION") );	

			CreateAgent(action);

			GetQuery().Initialize();
			GetQuery().Execute( _T("COMMIT TRANSACTION") );
		}
	}
	catch(ODBCError_t e){

		CEMSString sError;
		sError.Format( _T("%s"),e.szErrMsg);
        GenerateXML();
		GetQuery().Initialize();
		GetQuery().Execute( _T("ROLLBACK TRANSACTION") );

	}
	catch(...)
	{
		GenerateXML();
		GetQuery().Initialize();
		GetQuery().Execute( _T("ROLLBACK TRANSACTION") );
		throw;
	}

	return 0;
}

void CDemo::DecodeForm()
{
	GetISAPIData().GetFormTCHAR( _T("LoginName"), m_LoginName, 50 );
	GetISAPIData().GetFormTCHAR( _T("Password"), m_Password, 50 );
	GetISAPIData().GetFormTCHAR( _T("PasswordConfirm"), m_PasswordConfirm, 50 );
	GetISAPIData().GetFormTCHAR( _T("Name"), m_Name, 128 );
}

void CDemo::GenerateXML()
{
	GetXMLGen().AddChildElem( _T("LoginName"), m_LoginName );
	GetXMLGen().AddChildElem( _T("Password"), m_Password );
	GetXMLGen().AddChildElem( _T("PasswordConfirm"), m_PasswordConfirm );
	GetXMLGen().AddChildElem( _T("Name"), m_Name );
}

void CDemo::CreateAgent(CURLAction& action)
{
	DecodeForm();

	// check for duplicate login names
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_AgentID );
	BINDPARAM_TCHAR( GetQuery(), m_LoginName );
	GetQuery().Execute( _T("SELECT AgentID FROM Agents ")
		_T("WHERE AgentID<>? AND IsDeleted=0 AND LoginName=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, CEMSString(_T("The specified login name is already in use.  Please specify a different login name."))  );

	// create the object record
	TObjects object;
	GetTimeStamp( object.m_DateCreated );
	object.m_DateCreatedLen = sizeof(TIMESTAMP_STRUCT);
	object.m_ObjectTypeID = EMS_OBJECT_TYPE_AGENT;
	object.m_UseDefaultRights = 1;
	object.Insert( GetQuery() );
	m_ObjectID = object.m_ObjectID;

	// create the new agent
	EncryptPassword();
	TAgents::Insert( GetQuery() );
	DecryptPassword();

	// Update the object row with the actual ID
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_AgentID );
	BINDPARAM_LONG( GetQuery(), m_ObjectID );
	GetQuery().Execute( _T("UPDATE Objects SET ActualID=? WHERE ObjectID=?") );

	// create an access control item so that the new agent has edit access to himself
	/*GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_AgentID );
	BINDPARAM_LONG( GetQuery(), m_ObjectID );
	GetQuery().Execute( _T("INSERT INTO AccessControl ")
		_T("(AgentID,GroupID,AccessLevel,ObjectID) ")
		_T("VALUES (?,0,3,?)") );*/

	// create a record in PersonalData for the default
	// email address...
	TPersonalData pd;
	pd.m_AgentID = m_AgentID;
	pd.m_PersonalDataTypeID = EMS_PERSONAL_DATA_EMAIL;
	_tcscpy( pd.m_DataValue, m_LoginName );
	_tcscat( pd.m_DataValue, _T("@demo.visnetic.com") );
	pd.Insert( GetQuery() );

	// update the agent record with the ID of the default 
	// email address...
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), pd.m_PersonalDataID );
	BINDPARAM_LONG( GetQuery(), m_AgentID );
	GetQuery().Execute( _T("UPDATE Agents SET DefaultAgentAddressID=? WHERE AgentID=?") );

	// add the agent to the everyone group
	TAgentGroupings ag;
	ag.m_AgentID = m_AgentID;
	ag.m_GroupID = EMS_GROUPID_EVERYONE;
	ag.Insert(GetQuery());

	// add the agent to the administrators group
	ag.m_GroupID = EMS_GROUPID_ADMINISTRATORS;
	ag.Insert(GetQuery());

	// add default ticket box views
	TTicketBoxViews tbv;
	tbv.m_AgentID = m_AgentID;
	tbv.m_SortField = EMS_COLUMN_DATE;
	tbv.m_SortAscending = 0;
	tbv.m_TicketBoxViewTypeID = EMS_PUBLIC;
	tbv.m_UseDefault = true;
	
	tbv.m_TicketBoxID = 2;
	tbv.Insert( GetQuery() );
	
	tbv.m_TicketBoxID = 3;
	tbv.Insert( GetQuery() );
	
	tbv.m_TicketBoxID = 4;
	tbv.Insert( GetQuery() );

	TBVAddAgent( GetQuery(), m_AgentID, m_AgentID );
	TBVAddAgent( GetQuery(), m_AgentID, 3 );
	TBVAddAgent( GetQuery(), m_AgentID, 4 );

	// add favorite standard response records
	TStdResponseFavorites stdfav;
	stdfav.m_AgentID = m_AgentID;
	stdfav.m_StandardResponseID = 1;
	stdfav.Insert( GetQuery() );
	stdfav.m_StandardResponseID = 2;
	stdfav.Insert( GetQuery() );

	// add welcome message
	CTicket Ticket( m_ISAPIData );
	Ticket.m_IsDeleted = EMS_DELETE_OPTION_DO_NOT_DELETE;
	Ticket.m_OwnerID = m_AgentID;
	Ticket.m_TicketStateID = EMS_TICKETSTATEID_OPEN;
	_tcscpy( Ticket.m_Subject, _T("Welcome to VisNetic MailFlow") );
	_tcscpy( Ticket.m_Contacts, _T("MailFlow Demo") );
	Ticket.m_TicketCategoryID = 1;
	Ticket.Insert();

	// add an inbound message to the ticket
	CInboundMessage msg(m_ISAPIData);
	msg.m_IsDeleted = 0;
	GetTimeStamp(msg.m_DateReceived);
	msg.m_DateReceivedLen = sizeof(TIMESTAMP_STRUCT);
	msg.m_OriginalOwnerID = m_AgentID;
	msg.m_OriginalTicketBoxID = Ticket.m_TicketBoxID;
	_tcscpy( msg.m_MediaType, _T("text") );
	_tcscpy( msg.m_MediaSubType, _T("html") );
	_tcscpy( msg.m_EmailTo, pd.m_DataValue );
	msg.m_TicketID = Ticket.m_TicketID;
	msg.m_ContactID = 2;
	_tcscpy( msg.m_EmailFrom, _T("mailflow@demo.visnetic.com") );
	_tcscpy( msg.m_EmailFromName, _T("MailFlow") );
	_tcscpy( msg.m_Subject, _T("Welcome to VisNetic MailFlow") );
	GetBodyFromFile( _T("welcome.htm"), &msg.m_Body, msg.m_BodyAllocated );
	msg.Insert( GetQuery() );

	// insert a TicketContacts record
	TTicketContacts TicketContact;
	TicketContact.m_ContactID = 2;
	TicketContact.m_TicketID = Ticket.m_TicketID;
	TicketContact.Insert( GetQuery() );

	// add the ticket history event
	THCreateTicket( GetQuery(), Ticket.m_TicketID, 0, 0, Ticket.m_TicketStateID, Ticket.m_TicketBoxID, Ticket.m_OwnerID, Ticket.m_PriorityID, Ticket.m_TicketCategoryID );
	THAddInboundMsg( GetQuery(), Ticket.m_TicketID, 0, msg.m_InboundMessageID, Ticket.m_TicketBoxID );

	// change the subject and add another ticket
	_tcscpy( Ticket.m_Subject, _T("Ticket with Note") );
	Ticket.m_TicketCategoryID = 1;
	Ticket.Insert();

	// create the contact for this ticket
	TicketContact.m_TicketID = Ticket.m_TicketID;
	TicketContact.Insert( GetQuery() );

	// add a note to the ticket
	TTicketNotes Note;
	Note.m_AgentID = m_AgentID;
	GetTimeStamp(Note.m_DateCreated);
	Note.m_DateCreatedLen = sizeof(TIMESTAMP_STRUCT);
	Note.m_TicketID = Ticket.m_TicketID;
	CEMSString sString = _T("This is an example ticket note.");
	PutStringProperty( sString , &Note.m_Note, &Note.m_NoteAllocated );
	Note.Insert(GetQuery());

	// add ticket history event
	THCreateTicket( GetQuery(), Ticket.m_TicketID, 0, 0, Ticket.m_TicketStateID, Ticket.m_TicketBoxID, Ticket.m_OwnerID, Ticket.m_PriorityID, Ticket.m_TicketCategoryID );

	// add ticket to support ticket box
	AddSupportTicket();

	// add ticket to sales ticket box
	AddSalesTicket();

	// add ticket to the info ticket box
	AddInfoTicket();

	// invalidate cache objects
	InvalidateAgentEmails();
	InvalidateAgents();

	// redirect to the login page
	CEMSString sURL;
	sURL.Format( _T("login.ems?username=%s&password=%s"), m_LoginName, m_Password );
	action.SetRedirectURL( (TCHAR*) sURL.c_str() );
}


void CDemo::AddSupportTicket(void)
{
	CTicket Ticket(m_ISAPIData);
	CInboundMessage InMsg(m_ISAPIData);
	COutboundMessage OutMsg(m_ISAPIData);
	TTicketContacts TicketContact;
	TOutboundMessageContacts OutboundMessageContact;
	TTicketHistory th;
	tstring sString;
	time_t now;
	
	// get the current time in seconds
	time( &now ); 

	// create the ticket
	Ticket.m_IsDeleted = 0;
	Ticket.m_AutoReplied = 1;
	Ticket.m_OwnerID = 0;
	Ticket.m_TicketBoxID = 2;
	Ticket.m_TicketStateID = EMS_TICKETSTATEID_OPEN;
	_tcscpy( Ticket.m_Contacts, _T("Your Customer") );
	_tcscpy( Ticket.m_Subject, _T("Support inquiry for ") );
	_tcscat( Ticket.m_Subject, m_Name );
	Ticket.m_TicketCategoryID = 1;
	Ticket.Insert();

	_tcscpy( Ticket.m_Subject, _T("Stability question?") );

	// create the contact for this ticket
	TicketContact.m_ContactID = 1;
	TicketContact.m_TicketID = Ticket.m_TicketID;
	TicketContact.Insert( GetQuery() );

	// create the ticket history record
	th.m_TicketActionID = EMS_TICKETACTIONID_CREATED;
	th.m_TicketID = Ticket.m_TicketID;
	SecondsToTimeStamp( now - 3600, th.m_DateTime  );
	th.m_DateTimeLen = sizeof(TIMESTAMP_STRUCT);
	th.Insert( GetQuery() );

	// add a inbound message to the ticket
	InMsg.m_IsDeleted = 0;
	InMsg.m_TicketID = Ticket.m_TicketID;
	SecondsToTimeStamp( now - 3600, InMsg.m_DateReceived  );
	SecondsToTimeStamp( now - 3600, InMsg.m_EmailDateTime  );
	InMsg.m_ContactID = 1;
	InMsg.m_OriginalOwnerID = 0;
	InMsg.m_OriginalTicketBoxID = Ticket.m_TicketBoxID;
	_tcscpy( InMsg.m_MediaSubType, _T("plain") );
	_tcscpy( InMsg.m_EmailTo, _T("support@demo.visnetic.com") );
	_tcscpy( InMsg.m_EmailFrom, _T("anyone@isp.com") );
	_tcscpy( InMsg.m_EmailFromName, _T("Your Customer") );
	_tcscpy( InMsg.m_Subject, Ticket.m_Subject );
	GetBodyFromFile( _T("support1.txt"), &InMsg.m_Body, InMsg.m_BodyAllocated );
	InMsg.Insert( GetQuery() );

	// add ticket history record
	th.m_TicketActionID = EMS_TICKETACTIONID_ADD_MSG;
	th.m_ID1 = 1;
	th.m_ID2 = InMsg.m_InboundMessageID;
	th.Insert( GetQuery() );

	// add an outbound message (auto-reply)
	OutMsg.m_IsDeleted = 0;
	OutMsg.m_OutboundMessageTypeID = EMS_OUTBOUND_MESSAGE_TYPE_REPLY;
	OutMsg.m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_SENT;
	OutMsg.m_AgentID = 0;
	SecondsToTimeStamp( now - 3600, OutMsg.m_EmailDateTime  );
	OutMsg.m_OriginalTicketBoxID = 2;
	OutMsg.m_TicketID = Ticket.m_TicketID;
	OutMsg.m_ReplyToIDIsInbound = 1;
	OutMsg.m_ReplyToMsgID = InMsg.m_InboundMessageID;
	_tcscpy( OutMsg.m_MediaSubType, _T("plain") );
	_tcscpy( OutMsg.m_Subject, _T("RE: ") );
	_tcscat( OutMsg.m_Subject, InMsg.m_Subject );
	_tcscpy( OutMsg.m_EmailFrom, _T("support@demo.visnetic.com") );
	sString = _T("\"Your Customer\" <anyone@isp.com>");
	_tcscpy( OutMsg.m_EmailPrimaryTo, sString.c_str() );
	PutStringProperty( sString , &OutMsg.m_EmailTo, &OutMsg.m_EmailToAllocated );
	GetBodyFromFile( _T("support2.txt"), &OutMsg.m_Body, OutMsg.m_BodyAllocated );
	OutMsg.Insert( GetQuery() );

	// add the outbound message contact
	OutboundMessageContact.m_OutboundMessageContactID = 1;
	OutboundMessageContact.m_OutboundMessageID = OutMsg.m_OutboundMessageID;
	OutboundMessageContact.Insert( GetQuery() );

	// add ticket history record
	th.m_ID1 = 0;
	th.m_ID2 = OutMsg.m_OutboundMessageID;
	th.Insert( GetQuery() );

	// add another outbound message to the ticket
	OutMsg.m_AgentID = m_AgentID;
	SecondsToTimeStamp( now - 1700, OutMsg.m_EmailDateTime  );
	GetBodyFromFile( _T("support3.txt"), &OutMsg.m_Body, OutMsg.m_BodyAllocated );
	OutMsg.Insert( GetQuery() );

	// add the outbound message contact
	OutboundMessageContact.m_OutboundMessageID = OutMsg.m_OutboundMessageID;
	OutboundMessageContact.Insert( GetQuery() );

	// add ticket history record
	th.m_AgentID = m_AgentID;
	th.m_ID2 = OutMsg.m_OutboundMessageID;
	SecondsToTimeStamp( now - 1700, th.m_DateTime  );
	th.Insert( GetQuery() );

	// add another inbound message to the ticket
	InMsg.m_ReplyToIDIsInbound = 0;
	InMsg.m_ReplyToMsgID = OutMsg.m_OutboundMessageID;
	GetTimeStamp(InMsg.m_DateReceived);
	GetTimeStamp(InMsg.m_EmailDateTime);
	_tcscpy( InMsg.m_Subject, _T("RE: ") );
	_tcscat( InMsg.m_Subject, Ticket.m_Subject );
	GetBodyFromFile( _T("support4.txt"), &InMsg.m_Body, InMsg.m_BodyAllocated );
	InMsg.Insert( GetQuery() );

	// add ticket history record
	THAddInboundMsg( GetQuery(), Ticket.m_TicketID, 0, InMsg.m_InboundMessageID, Ticket.m_TicketBoxID );
}

void CDemo::AddSalesTicket(void)
{
	CTicket Ticket(m_ISAPIData);
	CInboundMessage InMsg(m_ISAPIData);
	COutboundMessage OutMsg(m_ISAPIData);
	TTicketContacts TicketContact;
	TOutboundMessageContacts OutboundMessageContact;
	TTicketHistory th;
	tstring sString;
	time_t now;

	// get the current time in seconds
	time( &now ); 

	// create the ticket
	Ticket.m_IsDeleted = 0;
	Ticket.m_AutoReplied = 1;
	Ticket.m_OwnerID = 0;
	Ticket.m_TicketBoxID = 3;
	Ticket.m_TicketStateID = EMS_TICKETSTATEID_OPEN;
	_tcscpy( Ticket.m_Contacts, _T("Your Customer") );
	_tcscpy( Ticket.m_Subject, _T("Sales inquiry for ") );
	_tcscat( Ticket.m_Subject, m_Name );
	Ticket.m_TicketCategoryID = 1;
	Ticket.Insert();

	_tcscpy( Ticket.m_Subject, _T("Product pricing?") );

	// create the ticket contact record
	TicketContact.m_ContactID = 1;
	TicketContact.m_TicketID = Ticket.m_TicketID;
	TicketContact.Insert( GetQuery() );

	// create the ticket history record
	th.m_TicketActionID = EMS_TICKETACTIONID_CREATED;
	th.m_TicketID = Ticket.m_TicketID;
	SecondsToTimeStamp( now - 3600, th.m_DateTime  );
	th.m_DateTimeLen = sizeof(TIMESTAMP_STRUCT);
	th.Insert( GetQuery() );

	// add a inbound message to the ticket
	InMsg.m_IsDeleted = 0;
	InMsg.m_TicketID = Ticket.m_TicketID;
	SecondsToTimeStamp( now - 3600, InMsg.m_DateReceived  );
	SecondsToTimeStamp( now - 3600, InMsg.m_EmailDateTime  );
	InMsg.m_ContactID = 1;
	InMsg.m_OriginalOwnerID = 0;
	InMsg.m_OriginalTicketBoxID = Ticket.m_TicketBoxID;
	_tcscpy( InMsg.m_MediaSubType, _T("plain") );
	_tcscpy( InMsg.m_EmailTo, _T("sales@demo.visnetic.com") );
	_tcscpy( InMsg.m_EmailFrom, _T("anyone@isp.com") );
	_tcscpy( InMsg.m_EmailFromName, _T("Your Customer") );
	_tcscpy( InMsg.m_Subject, Ticket.m_Subject );
	GetBodyFromFile( _T("sales1.txt"), &InMsg.m_Body, InMsg.m_BodyAllocated );
	InMsg.Insert( GetQuery() );

	// add ticket history record
	th.m_TicketActionID = EMS_TICKETACTIONID_ADD_MSG;
	th.m_ID1 = 1;
	th.m_ID2 = InMsg.m_InboundMessageID;
	th.Insert( GetQuery() );

	// add an outbound message
	OutMsg.m_IsDeleted = 0;
	OutMsg.m_OutboundMessageTypeID = EMS_OUTBOUND_MESSAGE_TYPE_REPLY;
	OutMsg.m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_SENT;
	OutMsg.m_AgentID = m_AgentID;
	SecondsToTimeStamp( now - 1800, OutMsg.m_EmailDateTime  );
	OutMsg.m_OriginalTicketBoxID = 2;
	OutMsg.m_TicketID = Ticket.m_TicketID;
	OutMsg.m_ReplyToIDIsInbound = 1;
	OutMsg.m_ReplyToMsgID = InMsg.m_InboundMessageID;
	_tcscpy( OutMsg.m_MediaSubType, _T("plain") );
	_tcscpy( OutMsg.m_Subject, _T("RE: ") );
	_tcscat( OutMsg.m_Subject, Ticket.m_Subject );
	_tcscpy( OutMsg.m_EmailFrom, _T("sales@demo.visnetic.com") );
	sString = _T("\"Your Customer\" <anyone@isp.com>");
	_tcscpy( OutMsg.m_EmailPrimaryTo, sString.c_str() );
	PutStringProperty( sString , &OutMsg.m_EmailTo, &OutMsg.m_EmailToAllocated );
	GetBodyFromFile( _T("sales2.txt"), &OutMsg.m_Body, OutMsg.m_BodyAllocated );
	OutMsg.Insert( GetQuery() );

	// add the outbound message contact
	OutboundMessageContact.m_OutboundMessageContactID = 1;
	OutboundMessageContact.m_OutboundMessageID = OutMsg.m_OutboundMessageID;
	OutboundMessageContact.Insert( GetQuery() );

	// add ticket history record
	th.m_AgentID = m_AgentID;
	th.m_ID1 = 0;
	th.m_ID2 = OutMsg.m_OutboundMessageID;
	SecondsToTimeStamp( now - 1800, th.m_DateTime  );
	th.Insert( GetQuery() );

	// add another inbound message to the ticket
	InMsg.m_ReplyToIDIsInbound = 0;
	InMsg.m_ReplyToMsgID = OutMsg.m_OutboundMessageID;
	GetTimeStamp(InMsg.m_DateReceived);
	GetTimeStamp(InMsg.m_EmailDateTime);
	_tcscpy( InMsg.m_Subject, _T("RE: ") );
	_tcscat( InMsg.m_Subject, Ticket.m_Subject );
	GetBodyFromFile( _T("sales3.txt"), &InMsg.m_Body, InMsg.m_BodyAllocated );
	InMsg.Insert( GetQuery() );

	// add ticket history record
	THAddInboundMsg( GetQuery(), Ticket.m_TicketID, 0, InMsg.m_InboundMessageID, Ticket.m_TicketBoxID );
}

void CDemo::AddInfoTicket(void)
{
	CTicket Ticket(m_ISAPIData);
	CInboundMessage InMsg(m_ISAPIData);
	TTicketContacts TicketContact;
	TTicketHistory th;
	tstring sString;
	time_t now;

	// get the current time in seconds
	time( &now ); 

	// create the ticket
	Ticket.m_IsDeleted = 0;
	Ticket.m_AutoReplied = 1;
	Ticket.m_OwnerID = 0;
	Ticket.m_TicketBoxID = 4;
	Ticket.m_TicketStateID = EMS_TICKETSTATEID_OPEN;
	_tcscpy( Ticket.m_Contacts, _T("Your Customer") );
	_tcscpy( Ticket.m_Subject, _T("General inquiry for ") );
	_tcscat( Ticket.m_Subject, m_Name );
	Ticket.m_TicketCategoryID = 1;
	Ticket.Insert();

	_tcscpy( Ticket.m_Subject, _T("What is your mailing address?") );

	// create the ticket contact record
	TicketContact.m_ContactID = 1;
	TicketContact.m_TicketID = Ticket.m_TicketID;
	TicketContact.Insert( GetQuery() );

	// create the ticket history record
	th.m_TicketActionID = EMS_TICKETACTIONID_CREATED;
	th.m_TicketID = Ticket.m_TicketID;
	SecondsToTimeStamp( now - 3600, th.m_DateTime  );
	th.m_DateTimeLen = sizeof(TIMESTAMP_STRUCT);
	th.Insert( GetQuery() );

	// add a inbound message to the ticket
	InMsg.m_IsDeleted = 0;
	InMsg.m_TicketID = Ticket.m_TicketID;
	SecondsToTimeStamp( now - 3600, InMsg.m_DateReceived  );
	SecondsToTimeStamp( now - 3600, InMsg.m_EmailDateTime  );
	InMsg.m_ContactID = 1;
	InMsg.m_OriginalOwnerID = 0;
	InMsg.m_OriginalTicketBoxID = Ticket.m_TicketBoxID;
	_tcscpy( InMsg.m_MediaSubType, _T("plain") );
	_tcscpy( InMsg.m_EmailTo, _T("info@demo.visnetic.com") );
	_tcscpy( InMsg.m_EmailFrom, _T("anyone@isp.com") );
	_tcscpy( InMsg.m_EmailFromName, _T("Your Customer") );
	_tcscpy( InMsg.m_Subject, Ticket.m_Subject );
	GetBodyFromFile( _T("info1.txt"), &InMsg.m_Body, InMsg.m_BodyAllocated );
	InMsg.Insert( GetQuery() );

	// add ticket history record
	th.m_TicketActionID = EMS_TICKETACTIONID_ADD_MSG;
	th.m_ID1 = 1;
	th.m_ID2 = InMsg.m_InboundMessageID;
	th.Insert( GetQuery() );
}


void CDemo::GetBodyFromFile( LPCTSTR szFile, TCHAR** ppBuffer, long& nBytesAllocated )
{
	// get the full path to the file
	tstring sFullPath;
	GetDemoMsgPath( sFullPath );
	sFullPath += _T("\\");
	sFullPath += szFile;

	// attempt to open the file
	FILE* fp = fopen( sFullPath.c_str(), "rb" );

	if (!fp)
	{
		CEMSString sError( _T("Unable to open file: ") );
		sError += sFullPath;
		THROW_EMS_EXCEPTION( E_SystemError, sError.c_str() );
	}

	// determine the file length
	fseek( fp, 0L, SEEK_END );
	int nFileLen = ftell(fp);
	int nByteLen = (nFileLen + 1) * sizeof(TCHAR);
	fseek( fp, 0L, SEEK_SET );

	// free any memory that was already allocated
	if ( nBytesAllocated )
		free( *ppBuffer );

	// allocate a new memory buffer
	*ppBuffer = (TCHAR*)calloc( nByteLen, 1 );

	if (*ppBuffer == NULL)
		THROW_EMS_EXCEPTION( E_MemoryError, _T("Unable to allocate memory") );

	nBytesAllocated = nByteLen;

	if ( fread( *ppBuffer, nFileLen, 1, fp ) == 1 )
	{
		(*ppBuffer)[nFileLen] = 0;
	}
	else
	{
		(*ppBuffer)[0] = 0;
	}

	// close the file
	fclose(fp);
}

void CDemo::GetDemoMsgPath(tstring& sPath )
{
	// get the installation path from the registry
	if ( GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_INSTALL_PATH_VALUE, sPath ) != ERROR_SUCCESS)
		THROW_EMS_EXCEPTION( E_SystemError, _T("Error reading InstallPath key from registry"));

	// make sure it has a backslash at the end.
	if( sPath.length() > 0 && sPath.at( sPath.length() - 1 ) != _T('\\') )
		sPath += _T("\\");

	// append the rest of the path
	sPath += _T("demo");
}

void CDemo::EncryptPassword()
{
	CEMSString sPassword;

	sPassword.assign( m_Password );
	sPassword.Encrypt();
	strncpy( m_Password, sPassword.c_str(), AGENTS_PASSWORD_LENGTH-1 );	
}

void CDemo::DecryptPassword()
{
	CEMSString sPassword;

	sPassword.assign( m_Password );
	sPassword.Decrypt();
	strncpy( m_Password, sPassword.c_str(), AGENTS_PASSWORD_LENGTH-1 );
}
