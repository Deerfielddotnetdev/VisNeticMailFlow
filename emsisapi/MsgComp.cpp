/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/MsgComp.cpp,v 1.2.2.3.2.2 2006/09/05 14:16:08 markm Exp $
||
||
||                                         
||  COMMENTS:	Message Composition  
||              
\\*************************************************************************/

#include "stdafx.h"
#include "MsgComp.h"
#include "UploadMap.h"
#include "ContactFns.h"
#include "StringFns.h"
#include "AttachFns.h"
#include "MessageIO.h"
#include "StdResponse.h"
#include "TicketHistoryFns.h"
#include "PurgeFns.h"
#include "ArchiveFns.h"
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CMsgComp::CMsgComp(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData), 
m_Ticket(ISAPIData), m_InboundMessage(ISAPIData), m_OutboundMessage(ISAPIData)
{
	m_nPageType = EMS_NewMessage;
	m_UseHTML = 0;
	m_bDraft = false;
	m_bReplyToAll = false;
	m_bCreatedTicket = false;
	m_nArchiveID = 0;
	m_originalInboundMessageID = 0;
	m_bOutboundApproval = false;
	
	// don't automatically unlock the ticket
	m_Ticket.DoNotUnlock();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CMsgComp::Run( CURLAction& action )
{
	dca::String info;
	int nNewMsgID;

	// release locks if the compose window is closed...
	if ( GetISAPIData().GetURLLong( _T("ReleaseTicketLock"), m_Ticket.m_TicketID, true ) )
	{
		DebugReporter::Instance().DisplayMessage("CMsgComp::Run - Release locks if the compose window is closed.", DebugReporter::ISAPI, GetCurrentThreadId());
		ReleaseLock();
		return 0;
	}
	
	SetPageType();
	
	if ( GetISAPIData().GetURLLong( _T("MSG"), m_SrcMsg.MsgID, true ) )
	{
		tstring sMsgType;
		GetISAPIData().GetURLString( _T("TYPE"), sMsgType );
		m_SrcMsg.IsInbound = sMsgType.compare( _T("inbound") ) == 0;
	}
	
	if ( GetISAPIData().GetXMLPost() )
	{	
		tstring sAction;
		GetISAPIData().GetXMLString( _T("ACTION"), sAction );
	
		if ( sAction.compare(_T("save")) == 0 )
			m_bDraft = true;
		
		DebugReporter::Instance().DisplayMessage("CMsgComp::Run - Decoding the form data", DebugReporter::ISAPI, GetCurrentThreadId());

		// decode the form data
		DecodeForm();
		
		DebugReporter::Instance().DisplayMessage("CMsgComp::Run - Lock the ticket and confirm we still have edit rights for ticket", DebugReporter::ISAPI, GetCurrentThreadId());

		// lock the ticket and confirm that we still
		// have edit rights for the ticket!
		m_Ticket.Lock(true);

		DebugReporter::Instance().DisplayMessage("CMsgComp::Run - Refresh parent view if neccessary", DebugReporter::ISAPI, GetCurrentThreadId());
		
		// do we need to refresh the parent view? will this operation 
		// change the contents of the ticket list
		RefreshParentView();

		try
		{
			GetQuery().Initialize();
			GetQuery().Execute( _T("BEGIN TRANSACTION") );	

			SaveMessage();

			GetQuery().Initialize();
			GetQuery().Execute( _T("COMMIT TRANSACTION") );
		}
		catch( ... )
		{
			GetQuery().Initialize();
			GetQuery().Execute( _T("ROLLBACK TRANSACTION") );
			throw;
		}

		info.Format("CMsgComp::Run - Ready to return to information to IIS for agent id %d this was an xml post", GetAgentID());
		DebugReporter::Instance().DisplayMessage(info.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

		return 0;
	}

	info.Format("CMsgComp::Run - Agent ID %d is getting ready to compose message", GetAgentID());
	DebugReporter::Instance().DisplayMessage(info.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	
	if ( m_nPageType == EMS_NewMessage )
	{
		DebugReporter::Instance().DisplayMessage("CMsgComp::Run - This is a new message", DebugReporter::ISAPI, GetCurrentThreadId());

		if ( GetISAPIData().GetURLLong( _T("TICKETID"), m_Ticket.m_TicketID, true ) )
		{
			m_Ticket.Query();
			m_Ticket.Lock(true);
		}
		else
		{
			m_Ticket.ReserveID();
			m_bCreatedTicket = true;
		}
	}
	else
	{
		DebugReporter::Instance().DisplayMessage("CMsgComp::Run - This is a reply query source msg", DebugReporter::ISAPI, GetCurrentThreadId());

		QuerySourceMsg();
		m_Ticket.Lock(true);
	}
	
	// get the ID of the new message
	if ( m_nPageType != EMS_SendDraft )
	{
		DebugReporter::Instance().DisplayMessage("CMsgComp::Run - Get new message id", DebugReporter::ISAPI, GetCurrentThreadId());

		COutboundMessage msg(m_ISAPIData);
		nNewMsgID = msg.ReserveID( m_Ticket.m_TicketID );

		m_originalInboundMessageID = m_InboundMessage.m_InboundMessageID;
		
		m_SrcMsg.IsInbound ? m_InboundMessage.m_InboundMessageID = nNewMsgID :
							 m_OutboundMessage.m_OutboundMessageID = nNewMsgID;
	}
	
	// add attachments from the source message
	if ( m_nPageType == EMS_ForwardMessage || m_nPageType == EMS_SendAgain )
	{
		DebugReporter::Instance().DisplayMessage("CMsgComp::Run - Add attachments", DebugReporter::ISAPI, GetCurrentThreadId());

		AddSourceMsgAttachments( nNewMsgID );
	}
	else if ( m_nPageType == EMS_ReplyToMessage )
	{
		DebugReporter::Instance().DisplayMessage("CMsgComp::Run - This is a reply attach inline images", DebugReporter::ISAPI, GetCurrentThreadId());

		AttachInlineImages( GetQuery(), 
			                m_SrcMsg.IsInbound ? m_InboundMessage.m_Body : m_OutboundMessage.m_Body,
							m_SrcMsg.MsgID, m_SrcMsg.IsInbound, nNewMsgID, m_nArchiveID );
	}

	DebugReporter::Instance().DisplayMessage("CMsgComp::Run - Generate xml", DebugReporter::ISAPI, GetCurrentThreadId());

	if( (m_OutboundMessage.m_OutboundMessageStateID == 7) && (m_OutboundMessage.m_AgentID != GetAgentID()))
	{
		m_OutboundMessage.Return();
	}
	GenerateXML();

	info.Format("CMsgComp::Run - Ready to return information to IIS for agent id %d", GetAgentID());
	DebugReporter::Instance().DisplayMessage(info.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Release the ticket and message     
\*--------------------------------------------------------------------------*/
void CMsgComp::ReleaseLock(void)
{
	int nTicketNoteID=0;
	
	// delete the reserved outbound message
	if ( GetISAPIData().GetURLLong( _T("Message"), m_OutboundMessage.m_OutboundMessageID, true ) )
	{
		// query the message
		GetQuery().Initialize();
		BINDCOL_TINYINT( GetQuery(), m_OutboundMessage.m_IsDeleted );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_OutboundMessageID );
		GetQuery().Execute( _T("SELECT IsDeleted FROM OutboundMessages WHERE OutboundMessageID=?") );
				
		if ( GetQuery().Fetch() == S_OK )
		{
			if ( m_OutboundMessage.m_IsDeleted == EMS_DELETE_OPTION_RESERVED )
			{
				CEMSString sMsgID;
				sMsgID.Format( _T("%d"), m_OutboundMessage.m_OutboundMessageID );
				PurgeOutboundMessages( sMsgID, GetQuery() );
			}
		}
	}

	// delete the reserved ticket note
	if ( GetISAPIData().GetURLLong( _T("Note"), nTicketNoteID, true ) )
	{
		
		CAttachment attachment(m_ISAPIData);
		list<CAttachment> AttachmentList;
		list<CAttachment>::iterator iter;

		// list the ticket note attachments
		attachment.ListNoteAttachments( nTicketNoteID, 1, AttachmentList );

		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), nTicketNoteID );
		GetQuery().Execute(_T("DELETE FROM NoteAttachments WHERE NoteID=? AND NoteTypeID=1") );

		for ( iter = AttachmentList.begin(); iter != AttachmentList.end(); iter++ )
		{
			if ( GetAttachmentReferenceCount( GetQuery(), iter->m_AttachmentID ) == 0 )
			{
				iter->DeleteNoteAttachment();
			}			
		}
		
		// delete any ticket note read entries
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), nTicketNoteID );
		GetQuery().Execute(_T("DELETE FROM TicketNotesRead WHERE TicketNoteID=?") );
		
		// delete ticket notes
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), nTicketNoteID );
		GetQuery().Execute(_T("DELETE FROM TicketNotes WHERE TicketNoteID=?") );
			
	}

	// unlock the ticket or release the reserved ticket
	GetQuery().Initialize();
	BINDCOL_TINYINT( GetQuery(), m_Ticket.m_IsDeleted );
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
	GetQuery().Execute( _T("SELECT IsDeleted FROM Tickets WHERE TicketID=?") );

	if ( GetQuery().Fetch() == S_OK )
	{
		if ( m_Ticket.m_IsDeleted == EMS_DELETE_OPTION_RESERVED )
		{
			// delete any ticket note attachments
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
			GetQuery().Execute(_T("DELETE FROM Attachments WHERE AttachmentID IN (SELECT AttachmentID FROM NoteAttachments WHERE NoteTypeID=1 AND NoteID IN (SELECT TicketNoteID FROM TicketNotes WHERE TicketID=?))") );

			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
			GetQuery().Execute(_T("DELETE FROM NoteAttachments WHERE NoteTypeID=1 AND NoteID IN (SELECT TicketNoteID FROM TicketNotes WHERE TicketID=?)") );

			// delete any ticket note read entries
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
			GetQuery().Execute(_T("DELETE FROM TicketNotesRead WHERE TicketNoteID IN (SELECT TicketNoteID FROM TicketNotes WHERE TicketID=?)") );
			
			// delete ticket notes
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );			
			GetQuery().Execute(_T("DELETE FROM TicketNotes WHERE TicketID=?") );
			
			// delete the ticket
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
			GetQuery().Execute(_T("DELETE FROM Tickets WHERE TicketID=?") );

			// delete any custom fields
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
			GetQuery().Execute(_T("DELETE FROM TicketFieldsTicket WHERE TicketID=?") );
		}
		else
		{
			// unlock the ticket
			m_Ticket.UnLock(true);
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Read Form Parameters	              
\*--------------------------------------------------------------------------*/
void CMsgComp::DecodeForm( void )
{
	CEMSString sParam;

	GetISAPIData().GetXMLLong( _T("TICKETID"), m_Ticket.m_TicketID );
	GetISAPIData().GetXMLLong( _T("FOLDERID"), m_Ticket.m_FolderID );
	GetISAPIData().GetXMLLong( _T("MESSAGEID"), m_OutboundMessage.m_OutboundMessageID );

	GetISAPIData().GetXMLLong( _T("TICKETBOX"), m_Ticket.m_TicketBoxID );
	
	GetISAPIData().GetXMLLong( _T("TICKETPRIORITY"), m_Ticket.m_PriorityID );
	GetISAPIData().GetXMLLong( _T("MESSAGEPRIORITY"), m_OutboundMessage.m_PriorityID );
	GetISAPIData().GetXMLLong( _T("SIGNATURE"), m_OutboundMessage.m_SignatureID );
	GetISAPIData().GetXMLLong( _T("USEHTML"), m_UseHTML );
	
	GetISAPIData().GetXMLLong( _T("TICKETSTATE"), m_Ticket.m_TicketStateID, true );
	GetISAPIData().GetXMLLong( _T("CLOSETICKET"), m_OutboundMessage.m_DraftCloseTicket, true );
	
	GetISAPIData().GetXMLLong( _T("TICKETCATEGORY"), m_OutboundMessage.m_TicketCategoryID, true );
		
	GetISAPIData().GetXMLLong( _T("OWNER"), m_Ticket.m_OwnerID, true );
	GetISAPIData().GetXMLLong( _T("ROUTETOINBOX"), m_OutboundMessage.m_DraftRouteToMe, true );
	GetISAPIData().GetXMLLong( _T("READRECEIPT"), m_OutboundMessage.m_ReadReceipt, true );
	GetISAPIData().GetXMLLong( _T("DELIVERYCONFIRMATION"), m_OutboundMessage.m_DeliveryConfirmation, true );

	GetISAPIData().GetXMLTCHAR( _T("TO"), &m_OutboundMessage.m_EmailTo, m_OutboundMessage.m_EmailToLen, m_OutboundMessage.m_EmailToAllocated );
	GetISAPIData().GetXMLTCHAR( _T("FROM"), m_OutboundMessage.m_EmailFrom, 255);
	GetISAPIData().GetXMLTCHAR( _T("CC"), &m_OutboundMessage.m_EmailCc, m_OutboundMessage.m_EmailCcLen, m_OutboundMessage.m_EmailCcAllocated );
	GetISAPIData().GetXMLTCHAR( _T("BCC"), &m_OutboundMessage.m_EmailBcc, m_OutboundMessage.m_EmailBccLen, m_OutboundMessage.m_EmailBccAllocated );
	GetISAPIData().GetXMLTCHAR( _T("SUBJECT"), m_OutboundMessage.m_Subject, 255);
	GetISAPIData().GetXMLTCHAR( _T("BODY"), &m_OutboundMessage.m_Body, m_OutboundMessage.m_BodyLen, m_OutboundMessage.m_BodyAllocated );

	GetISAPIData().GetXMLLong( _T("chkUseTickler"), m_Ticket.m_UseTickler, true);

	int nMultiMail = 0;
	int nNewTicket = 0;
	int nOmitTracking = 0;
	GetISAPIData().GetXMLLong( _T("chkMultiMail"), nMultiMail, true);
	GetISAPIData().GetXMLLong( _T("chkNewTicket"), nNewTicket, true);
	GetISAPIData().GetXMLLong( _T("chkOmitTracking"), nOmitTracking, true);

	if(nMultiMail == 1)
	{
		if(nNewTicket == 1)
		{
			if(nOmitTracking == 1)
			{
				m_OutboundMessage.m_MultiMail = 7;
			}
			else
			{
				m_OutboundMessage.m_MultiMail = 6;
			}
		}
		else
		{
			if(nOmitTracking == 1)
			{
				m_OutboundMessage.m_MultiMail = 5;
			}
			else
			{
				m_OutboundMessage.m_MultiMail = 4;
			}
		}
	}
	else
	{
		m_OutboundMessage.m_MultiMail = 0;
	}

	
	// get the OwnerID from either the OwnerID form field or the
	// RouteToInbox form field
	if ( m_Ticket.m_OwnerID == 0)
		m_Ticket.m_OwnerID = m_OutboundMessage.m_DraftRouteToMe==1 ? GetAgentID() : 0;

	if( m_Ticket.m_OwnerID == 0)
		m_Ticket.m_FolderID = 0;

	// set the TicketState?	
	if ( m_Ticket.m_TicketStateID == 0 )
	{
		m_Ticket.m_TicketStateID = m_OutboundMessage.m_DraftCloseTicket==1 ? EMS_TICKETSTATEID_CLOSED : EMS_TICKETSTATEID_OPEN;
	}
	
	if(m_Ticket.m_UseTickler)
	{
		CEMSString sDate;
		if (!GetISAPIData().GetFormString( _T("REOPEN_DATE"), sDate, true ))
				GetISAPIData().GetXMLString( _T("REOPEN_DATE"), sDate, true );

		//GetISAPIData().GetXMLString( _T("REOPEN_DATE"), sDate);
		sDate.CDLGetTimeStamp(m_Ticket.m_ReOpenTime);
		m_Ticket.m_ReOpenTime.second=0;
		m_Ticket.m_ReOpenTime.fraction=0;

		int nHr=0;
		int nMin=0;
		int nAmPm=0;

		GetISAPIData().GetXMLLong( _T("OpenHr"), nHr, true );
		GetISAPIData().GetXMLLong( _T("OpenMin"), nMin, true );
		GetISAPIData().GetXMLLong( _T("OpenAmPm"), nAmPm, true );
					
		if(nHr == 12 && nAmPm == 1)
		{
			m_Ticket.m_ReOpenTime.hour = 0;
		}
		else if(nAmPm == 2 && nHr == 12)
		{
			m_Ticket.m_ReOpenTime.hour = 12;
		}
		else if (nAmPm == 2)
		{
			m_Ticket.m_ReOpenTime.hour = nHr + 12;
		}
		else
		{
			m_Ticket.m_ReOpenTime.hour = nHr;
		}

		m_Ticket.m_ReOpenTime.minute = nMin;

		long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
		TIMESTAMP_STRUCT tsLocal;
		long tsLocalLen=0;

		if(lTzBias != 1)
		{
			if(ConvertFromTimeZone(m_Ticket.m_ReOpenTime,lTzBias,tsLocal))
			{
				m_Ticket.m_ReOpenTime.year = tsLocal.year;
				m_Ticket.m_ReOpenTime.month = tsLocal.month;
				m_Ticket.m_ReOpenTime.day = tsLocal.day;
				m_Ticket.m_ReOpenTime.hour = tsLocal.hour;
				m_Ticket.m_ReOpenTime.minute = tsLocal.minute;
				m_Ticket.m_ReOpenTime.second = tsLocal.second;
			}			
		}
		
		m_Ticket.m_TicketStateID = EMS_TICKETSTATEID_CLOSED;
	}

	// store the unaltered subject to use in newly created tickets
	m_sOutboundMessageSubject = m_OutboundMessage.m_Subject;

	// was a "reserved" ticket created 
	m_bCreatedTicket = m_nPageType == EMS_NewMessage && 
		GetISAPIData().m_URL.find( _T("TICKETID") ) == GetISAPIData().m_URL.end();

	//Get list of TicketFields configured for this TicketBox
	m_tft.clear();
	TTicketFieldsTicketBox tftb;
	TTicketFieldsTicket tft;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tftb.m_TicketFieldID );
	BINDCOL_LONG( GetQuery(), tft.m_TicketFieldTypeID );
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketBoxID );
	GetQuery().Execute( _T("SELECT tftb.TicketFieldID,tf.TicketFieldTypeID ")
	               _T("FROM TicketFieldsTicketBox tftb INNER JOIN TicketFields tf ON tftb.TicketFieldID=tf.TicketFieldID ")
	               _T("WHERE tftb.TicketBoxID=? ORDER BY TicketBoxID") );	
	while( GetQuery().Fetch() == S_OK )
	{
		sParam.Format( _T("input%d"), tftb.m_TicketFieldID );
		GetISAPIData().GetXMLTCHAR( (TCHAR*) sParam.c_str(), tft.m_DataValue, 255, true);
		tft.m_TicketID = m_Ticket.m_TicketID;
		tft.m_TicketFieldID = tftb.m_TicketFieldID;		
		m_tft.push_back(tft);
	}
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Set the page requested	              
\*--------------------------------------------------------------------------*/
void CMsgComp::SetPageType( void )
{
	// determine the page requested
	if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("replytomessage") ) == 0 )
	{
		m_nPageType = EMS_ReplyToMessage;
	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("replytoall") ) == 0 )
	{
		m_nPageType = EMS_ReplyToMessage;
		m_bReplyToAll = true;
	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("forwardmessage") ) == 0 )
	{
		m_nPageType = EMS_ForwardMessage;
	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("sendagain") ) == 0 )
	{
		m_nPageType = EMS_SendAgain;
	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("senddraft") ) == 0 )
	{
		m_nPageType = EMS_SendDraft;
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds the page type to the XML	              
\*--------------------------------------------------------------------------*/
void CMsgComp::AddPageTypeXML(void)
{
	switch( m_nPageType )
	{
	case EMS_NewMessage:
		GetXMLGen().AddChildAttrib( _T("MessageAction"), _T("New") );
		break;
		
	case EMS_ReplyToMessage:
		GetXMLGen().AddChildAttrib( _T("MessageAction"), _T("Reply") );
		GetXMLGen().AddChildAttrib( _T("ReplyToAll"), m_bReplyToAll ? 1 : 0 );
		break;
		
	case EMS_ForwardMessage:
		GetXMLGen().AddChildAttrib( _T("MessageAction"), _T("Forward") );
		break;
		
	case EMS_SendAgain:
		GetXMLGen().AddChildAttrib( _T("MessageAction"), _T("SendAgain") );
		break;
		
	case EMS_SendDraft:
		GetXMLGen().AddChildAttrib( _T("MessageAction"), _T("SendDraft") );
		break;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML from class members	              
\*--------------------------------------------------------------------------*/
void CMsgComp::GenerateXML(void)
{	
	if( m_nPageType == EMS_NewMessage )
	{
		GetXMLGen().AddChildElem( _T("Message") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_OutboundMessage.m_OutboundMessageID );
		GetXMLGen().AddChildAttrib( _T("PriorityID"), EMS_PRIORITY_NORMAL );
		GetXMLGen().AddChildAttrib( _T("OutboundMsgTypeID"), 1 );
		
					
		AddPageTypeXML();	

		if ( m_bCreatedTicket )
		{
			int nContactID;
			if ( GetISAPIData().GetURLLong( _T("ContactID"), nContactID, true ) )
				AddContactEmailAddr( nContactID );

			GetXMLGen().AddChildElem( _T("Ticket") );
			GetXMLGen().AddChildAttrib( _T("ID"), m_Ticket.m_TicketID );
			GetXMLGen().AddChildAttrib( _T("TicketCategoryID"), m_Ticket.m_TicketCategoryID );
			GetXMLGen().AddChildAttrib( _T("PriorityID"), EMS_PRIORITY_NORMAL );
			GetXMLGen().AddChildAttrib( _T("NewTicket"), 1 );
		}
		else
		{
			// get a list of the email addresses associated with the ticket
			CEMSString sAddresses;
						
			BuildTicketEmailAddresses( sAddresses );
			
			GetXMLGen().IntoElem();

			GetXMLGen().AddChildElem( _T("To") );
			GetXMLGen().SetChildData( sAddresses.c_str(), TRUE );			
			
			// set the default message subject to that of the ticket
			GetXMLGen().AddChildElem( _T("Subject") );
			GetXMLGen().SetChildData( m_Ticket.m_Subject , TRUE );
				
			GetXMLGen().OutOfElem();

			m_Ticket.GenerateXML();
		}
	}
	else
	{
		// generate message XML
		m_SrcMsg.IsInbound ? m_InboundMessage.GenerateXML( true, 0, true ) 
			               : m_OutboundMessage.GenerateXML( true, 0, true );
		AddPageTypeXML();
		m_Ticket.GenerateXML();
	}

	int nSignatureID = 0;
	int nTempID = 0;
	long nTempIDLen;
	int nTicketBoxID = 0;
	int nTicketCategoryID = 0;
	int nAgentID = GetAgentID();
		
	GetISAPIData().GetURLLong( _T("TicketBoxID"),nTicketBoxID, true );

	if ( m_Ticket.m_TicketBoxID < 2 )
	{
		if ( nTicketBoxID == 0 )
		{
			nTicketBoxID = m_Ticket.m_TicketBoxID;
		}
		else
		{
			m_Ticket.m_TicketBoxID = nTicketBoxID;
		}
	}
	else
	{
		nTicketBoxID = m_Ticket.m_TicketBoxID;
	}

	if ( m_Ticket.m_TicketCategoryID == 0 )
	{
		GetISAPIData().GetURLLong( _T("TicketCategoryID"),nTicketCategoryID, true ); 		
	}
	else
	{
		nTicketCategoryID = m_Ticket.m_TicketCategoryID;
	}

	if ( m_nPageType == EMS_SendDraft || m_nPageType == EMS_SendAgain )
	{
		nSignatureID = m_OutboundMessage.m_SignatureID;
	}
	else if ( nTicketBoxID > 1 )
	{
		// is there a  default signature for this agent and ticketbox?
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), nTempID );
		BINDPARAM_LONG( GetQuery(), nAgentID );
		BINDPARAM_LONG( GetQuery(), nAgentID );
		BINDPARAM_LONG( GetQuery(), nTicketBoxID );
		GetQuery().Execute( _T("SELECT Signatures.SignatureID,Signatures.AgentID ")
							_T("FROM Signatures LEFT OUTER JOIN AgentGroupings ")
							_T("ON Signatures.GroupID = AgentGroupings.GroupID ")
							_T("WHERE (Signatures.AgentID=? OR AgentGroupings.AgentID=?) ")
							_T("AND Signatures.ObjectTypeID=2 AND Signatures.ActualID=? ")
							_T("ORDER BY Signatures.AgentID") );
			
		while( GetQuery().Fetch() == S_OK )
		{
			nSignatureID=nTempID;
		}		
	}
	
	if ( nTicketCategoryID > 1 && nSignatureID == 0 )
	{
		// is there a  default signature for this agent and ticketcategory?
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), nTempID );
		BINDPARAM_LONG( GetQuery(), nAgentID );
		BINDPARAM_LONG( GetQuery(), nAgentID );
		BINDPARAM_LONG( GetQuery(), nTicketCategoryID );
		GetQuery().Execute( _T("SELECT Signatures.SignatureID,Signatures.AgentID ")
							_T("FROM Signatures LEFT OUTER JOIN AgentGroupings ")
							_T("ON Signatures.GroupID = AgentGroupings.GroupID ")
							_T("WHERE (Signatures.AgentID=? OR AgentGroupings.AgentID=?) ")
							_T("AND Signatures.ObjectTypeID=6 AND Signatures.ActualID=? ")
							_T("ORDER BY Signatures.AgentID") );
			
		while( GetQuery().Fetch() == S_OK )
		{
			nSignatureID=nTempID;
		}		
		
	}	

	// XML for XSL
	ListStdResps();
	ListTicketPriorities();
	ListSignatures( GetAgentID(), GetXMLGen(), true, false, nSignatureID );
	ListTicketBoxNames( EMS_EDIT_ACCESS, GetXMLGen(), m_bCreatedTicket ? -1 : m_Ticket.m_TicketBoxID );
	ListTicketBoxDefaultEmailAddressName( EMS_EDIT_ACCESS, GetXMLGen(), m_bCreatedTicket ? -1 : m_Ticket.m_TicketBoxID );
	ListEnabledAgentNames( EMS_EDIT_ACCESS );
	ListTicketStates();

	// add the upload ID for use with the progress indicator
	GetXMLGen().AddChildElem( _T("Upload") );
	GetXMLGen().AddChildAttrib( _T("ID"), CUploadMap::GetInstance().GetUploadID() );

	// XML for the document
	GetXMLDoc().AddElem( _T("DATA") );
	ListSignatures( GetAgentID(), GetXMLDoc(), false, true, 0 );
	ListAgentEmails( GetAgentID(), GetXMLDoc() );
	ListTicketBoxNames( EMS_EDIT_ACCESS, GetXMLDoc(), m_bCreatedTicket ? -1 : m_Ticket.m_TicketBoxID );
	ListTicketBoxDefaultEmailAddressName( EMS_EDIT_ACCESS, GetXMLDoc(), m_bCreatedTicket ? -1 : m_Ticket.m_TicketBoxID );
	ListTicketBoxAddrs( GetXMLDoc() );
	
	//Get list of FromFormat for all TicketBoxes
	TTicketBoxes tb;	
	GetXMLDoc().AddChildElem( _T("TicketBoxesFromFormat") );
	GetXMLDoc().IntoElem();
    GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tb.m_TicketBoxID );
	BINDCOL_LONG( GetQuery(), tb.m_FromFormat );
	BINDCOL_LONG( GetQuery(), tb.m_MultiMail );
	BINDCOL_LONG( GetQuery(), tb.m_RequireTC );
	GetQuery().Execute( _T("SELECT TicketBoxID,FromFormat,MultiMail,RequireTC ")
	               _T("FROM TicketBoxes ")
	               _T("ORDER BY TicketBoxID") );	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLDoc().AddChildElem(_T("TicketBoxFromFormat"));
		GetXMLDoc().AddChildAttrib( _T("ID"), tb.m_TicketBoxID );
		GetXMLDoc().AddChildAttrib( _T("FromFormat"), tb.m_FromFormat );
		GetXMLDoc().AddChildAttrib( _T("MultiMail"), tb.m_MultiMail );
		GetXMLDoc().AddChildAttrib( _T("RequireTC"), tb.m_RequireTC );
	}
	GetXMLDoc().OutOfElem();

	//Get list of TicketFields configured for all TicketBoxes
	TTicketFieldsTicketBox tftb;
	TCHAR Description[50];
	long DescriptionLen;
	int nTicketFieldTypeID;
	GetXMLDoc().AddChildElem( _T("TicketFieldTicketBoxes") );
	GetXMLDoc().IntoElem();
    GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tftb.m_TicketBoxID );
	BINDCOL_LONG( GetQuery(), tftb.m_TicketFieldID );
	BINDCOL_TCHAR( GetQuery(), Description );
	BINDCOL_LONG_NOLEN( GetQuery(), nTicketFieldTypeID );
	BINDCOL_BIT( GetQuery(), tftb.m_IsRequired );
	BINDCOL_BIT( GetQuery(), tftb.m_IsViewed );
	BINDCOL_BIT( GetQuery(), tftb.m_SetDefault );
	BINDCOL_TCHAR( GetQuery(), tftb.m_DefaultValue );
	BINDCOL_LONG( GetQuery(), tftb.m_TicketFieldsTicketBoxID );
	GetQuery().Execute( _T("SELECT tftb.TicketBoxID,tftb.TicketFieldID,tf.Description,tf.TicketFieldTypeID,tftb.IsRequired,tftb.IsViewed,tftb.SetDefault,tftb.DefaultValue,tftb.TicketFieldsTicketBoxID ")
	               _T("FROM TicketFieldsTicketBox tftb INNER JOIN TicketFields tf ON tftb.TicketFieldID=tf.TicketFieldID ")
	               _T("ORDER BY TicketBoxID") );	
	while( GetQuery().Fetch() == S_OK )
	{
		if(nTicketFieldTypeID == 1)
		{
			tstring sVal;
			sVal.assign( tftb.m_DefaultValue );

			//$$DateTimeNow$$
			tstring::size_type pos = sVal.find( _T("$$DateTimeNow$$") );
			if( pos != tstring::npos )
			{
				TIMESTAMP_STRUCT Now;
				GetTimeStamp( Now );
				long NowLen = sizeof(Now);
				CEMSString sDate;
				GetDateTimeString( Now, NowLen, sDate );
				sVal.replace( pos, 15, sDate.c_str() );
				_sntprintf( tftb.m_DefaultValue, TICKETFIELDS_VALUE_LENGTH - 1, sVal.c_str() );
			}
		}

		GetXMLDoc().AddChildElem(_T("TicketFieldTicketBox"));
		GetXMLDoc().AddChildAttrib( _T("TBID"), tftb.m_TicketBoxID );
		GetXMLDoc().AddChildAttrib( _T("TFID"), tftb.m_TicketFieldID );
		GetXMLDoc().AddChildAttrib( _T("TFNAME"), Description );
		GetXMLDoc().AddChildAttrib( _T("TFTYPE"), nTicketFieldTypeID );
		GetXMLDoc().AddChildAttrib( _T("IsRequired"), tftb.m_IsRequired );
		GetXMLDoc().AddChildAttrib( _T("IsViewed"), tftb.m_IsViewed );
		GetXMLDoc().AddChildAttrib( _T("SetDefault"), tftb.m_SetDefault );
		GetXMLDoc().AddChildAttrib( _T("DefaultValue"), tftb.m_DefaultValue );
	}
	GetXMLDoc().OutOfElem();

	//Get Options for type of TicketField select
	TTicketFieldOptions tfo;
	int nTBID;
	GetXMLDoc().AddChildElem( _T("TicketFieldOptions") );
	GetXMLDoc().IntoElem();
    GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tfo.m_TicketFieldOptionID );
	BINDCOL_LONG( GetQuery(), tfo.m_TicketFieldID );
	BINDCOL_TCHAR( GetQuery(), tfo.m_OptionValue );
	BINDCOL_LONG_NOLEN( GetQuery(), nTBID );
	GetQuery().Execute( _T("SELECT tfo.TicketFieldOptionID,tfo.TicketFieldID,tfo.OptionValue,tftb.TicketBoxID ")
	               _T("FROM TicketFieldOptions tfo ")
	               _T("INNER JOIN TicketFieldsTicketBox tftb ON tfo.TicketFieldID=tftb.TicketFieldID ")
	               _T("ORDER BY tftb.TicketBoxID,tfo.TicketFieldID,tfo.OptionOrder") );	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLDoc().AddChildElem(_T("Option"));
		GetXMLDoc().AddChildAttrib( _T("TFOID"), tfo.m_TicketFieldOptionID );
		GetXMLDoc().AddChildAttrib( _T("TFID"), tfo.m_TicketFieldID );
		GetXMLDoc().AddChildAttrib( _T("OPTIONVALUE"), tfo.m_OptionValue );
		GetXMLDoc().AddChildAttrib( _T("TBID"), nTBID );
	}
	GetXMLDoc().OutOfElem();

	if( !m_bCreatedTicket )
	{		
		TTicketFieldsTicket tft;
		GetXMLDoc().AddChildElem( _T("TicketFields") );
		GetXMLDoc().IntoElem();
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), tft.m_TicketFieldID );
		BINDCOL_TCHAR( GetQuery(), tft.m_DataValue );
		BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
		GetQuery().Execute( _T("SELECT TicketFieldID, DataValue FROM TicketFieldsTicket ")
							_T("WHERE TicketID=?") );
		while ( GetQuery().Fetch() == S_OK )
		{
			GetXMLDoc().AddChildElem( _T("Field") );
			GetXMLDoc().AddChildAttrib( _T("TFID") , tft.m_TicketFieldID );
			GetXMLDoc().AddChildAttrib( _T("DataValue") , tft.m_DataValue );	
		}
		GetXMLDoc().OutOfElem();	
	}
	
	TTicketCategories pdt;
	pdt.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("TicketCategoryNames") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketCategory") );
		GetXMLGen().AddChildAttrib( _T("ID"), pdt.m_TicketCategoryID );
		GetXMLGen().AddChildAttrib( _T("Description"), pdt.m_Description );
	}
	GetXMLGen().OutOfElem();

	TDictionary dc;
	int nDcEnabled = 0;
	dc.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("Dictionaries") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		if(dc.m_IsEnabled){nDcEnabled++;}
		GetXMLGen().AddChildElem( _T("Dictionary") );
		GetXMLGen().AddChildAttrib( _T("ID"), dc.m_DictionaryID );
		GetXMLGen().AddChildAttrib( _T("Description"), dc.m_Description );
		GetXMLGen().AddChildAttrib( _T("TlxFile"), dc.m_TlxFile );
		GetXMLGen().AddChildAttrib( _T("ClxFile"), dc.m_ClxFile );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), dc.m_IsEnabled );
	}
	GetXMLGen().OutOfElem();
	tstring sDictionaryID;
	GetServerParameter( EMS_SRVPARAM_DEFAULT_DICTIONARY_ID, sDictionaryID, "1" );
	int nDictionaryID = _ttoi( sDictionaryID.c_str() );
	GetXMLGen().AddChildElem(_T("Spell"));
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(_T("Check"));
	GetXMLGen().AddChildAttrib( _T("DcEnabled"), nDcEnabled );
	if(nDcEnabled > 1)
	{
		TAgents a;
		a.m_AgentID = nAgentID;
		a.Query(GetQuery());
		if(a.m_DictionaryID != nDictionaryID){nDictionaryID=a.m_DictionaryID;}
		GetXMLGen().AddChildAttrib( _T("DictionaryPrompt"), a.m_DictionaryPrompt );
	}	
	GetXMLGen().AddChildAttrib( _T("DictionaryID"), nDictionaryID );		
	GetXMLGen().OutOfElem();

	if ( GetSession().m_UseAutoFill )
	{
        BuildContactEmailAddresses();	
	}

	BuildContactGroups();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query and populate the source message.  	              
\*--------------------------------------------------------------------------*/
void CMsgComp::QuerySourceMsg(void)
{
	// if a source message was not specified, we are done...
	if ( !m_SrcMsg.MsgID )
		return;
	
	m_SrcMsg.IsInbound ? m_InboundMessage.SetID(m_SrcMsg.MsgID) :m_OutboundMessage.SetID(m_SrcMsg.MsgID);

	// query the message
	if( GetISAPIData().GetURLLong( _T("ArchiveID"), m_nArchiveID, true ) && m_nArchiveID != 0 )
	{
		list<Attachments_t> attachList;
		list<Attachments_t>::iterator iter;
		CAttachment att( GetISAPIData() );
		tstring sArchive;
		int retval;

		att.m_FileExists = true;

		if( m_SrcMsg.IsInbound )
		{
			retval = GetInboundMessageFromArchive( GetQuery(), m_nArchiveID, 
												   m_InboundMessage, m_Ticket, 
												   attachList, sArchive );
			if( retval == Arc_Success )
			{
				for( iter = attachList.begin(); iter != attachList.end(); iter++ )
				{
					att.Copy( &(*iter) );
					m_InboundMessage.GetAttachmentList().push_back( att );
				}
			}
		}
		else 
		{
			retval = GetOutboundMessageFromArchive( GetQuery(), m_nArchiveID, 
													m_OutboundMessage, m_Ticket,
													attachList, sArchive );
			if( retval == Arc_Success )
			{
				for( iter = attachList.begin(); iter != attachList.end(); iter++ )
				{
					att.Copy( &(*iter) );
					m_OutboundMessage.GetAttachmentList().push_back( att );
				}
			}
		}

		if ( retval != Arc_Success )
		{
			CEMSString sErrMsg;
			sErrMsg.Format( _T("Error %d reading archive: %s"), retval, sArchive.c_str() );
			THROW_EMS_EXCEPTION_NOLOG( E_ReadingArchive, sErrMsg.c_str() );	
		}
	}
	else
	{
		m_SrcMsg.IsInbound ? m_InboundMessage.Query() : m_OutboundMessage.Query();
	}
	
	// do processing of the source message
	m_SrcMsg.IsInbound ? ProcessSrcMsg( m_InboundMessage ) : ProcessSrcMsg( m_OutboundMessage );
	MangleSubject( m_SrcMsg.IsInbound ? m_InboundMessage.m_Subject : m_OutboundMessage.m_Subject);
	
	if ( m_bReplyToAll )
	{
		m_SrcMsg.IsInbound ? ReplyToAll(m_InboundMessage) : ReplyToAll(m_OutboundMessage);
	}
	else if( m_nPageType == EMS_ReplyToMessage || m_nPageType == EMS_ForwardMessage )
	{
		// clear the CC: and BCC: fields:
		if( m_SrcMsg.IsInbound )
		{
			m_InboundMessage.m_EmailCcLen = 0;
			m_InboundMessage.m_EmailCc[0] = _T('\0');
		}
		else
		{
			m_OutboundMessage.m_EmailCcLen = 0;
			m_OutboundMessage.m_EmailCc[0] = _T('\0');
			m_OutboundMessage.m_EmailBccLen = 0;
			m_OutboundMessage.m_EmailBcc[0] = _T('\0');
		}
	}

	// fix DevTrack issue 389
	if ( m_nPageType != EMS_SendDraft && m_nPageType != EMS_SendAgain )
	{
		m_InboundMessage.m_PriorityID = EMS_PRIORITY_NORMAL;
		m_OutboundMessage.m_PriorityID = EMS_PRIORITY_NORMAL;
	}
	
	// query the Ticket
	m_Ticket.m_TicketID = m_SrcMsg.IsInbound ? m_InboundMessage.m_TicketID : m_OutboundMessage.m_TicketID;
	m_Ticket.Query();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Preform post processing of the source message 
\*--------------------------------------------------------------------------*/
template<class T> void CMsgComp::ProcessSrcMsg(T& Message)
{
	CEMSString sString;
	bool bIsText = _tcsicmp( Message.m_MediaSubType, _T("html") ) != 0;
	
	// add a header for replies and forwards
	if ( m_nPageType == EMS_ForwardMessage || m_nPageType == EMS_ReplyToMessage )
	{
		LPCTSTR szDateLabel = _T("Sent");
		CEMSString sDateTime;

		GetDateTimeString( Message.m_EmailDateTime, Message.m_EmailDateTimeLen, sDateTime);
		
		if ( bIsText )
		{
			if(Message.m_EmailCcLen > 0)
			{
				sString.Format( _T("-----Original Message-----\r\nFrom: %s\r\n%s: %s\r\n")
					_T("To: %s\r\nCc: %s\r\nSubject: %s\r\n\r\n"), Message.m_EmailFrom, szDateLabel, sDateTime.c_str(),  
					Message.m_EmailTo, Message.m_EmailCc, Message.m_Subject );
			}
			else
			{
				sString.Format( _T("-----Original Message-----\r\nFrom: %s\r\n%s: %s\r\n")
					_T("To: %s\r\nSubject: %s\r\n\r\n"), Message.m_EmailFrom, szDateLabel, sDateTime.c_str(), 
					Message.m_EmailTo, Message.m_Subject );
			}
		}
		else
		{
			CEMSString sFrom, sTo, sCc, sSubject;

			sFrom.assign( Message.m_EmailFrom );
			sFrom.EscapeHTML();

			sTo.assign( Message.m_EmailTo );
			sTo.EscapeHTML();

			sCc.assign( Message.m_EmailCc );
			sCc.EscapeHTML();

			sSubject.assign( Message.m_Subject );
			sSubject.EscapeHTML();

			if(Message.m_EmailCcLen > 0)
			{
				sString.Format( _T("-----Original Message-----<br/>From: %s<br/>%s: %s<br/>")
					_T("To: %s<br/>Cc: %s<br/>Subject: %s<br/><br/>"), sFrom.c_str(), szDateLabel, sDateTime.c_str(), 
					sTo.c_str(), sCc.c_str(), sSubject.c_str()  );
			}
			else
			{
				sString.Format( _T("-----Original Message-----<br/>From: %s<br/>%s: %s<br/>")
					_T("To: %s<br/>Subject: %s<br/><br/>"), sFrom.c_str(), szDateLabel, sDateTime.c_str(), 
					sTo.c_str(), sSubject.c_str()  );
			}
		}
	}

	if ( bIsText || sString.length() )
	{
		// append the message body to our string
		sString.append( Message.m_Body );
	
		// MJM - Fix for DevTrack issue 1099. < > are escaped to &lt; &gt; for text messages...
		if ( bIsText )
			sString.EscapeHTMLAndNewLines();
	
		// write our string back to the message object
		PutStringProperty( sString , &Message.m_Body, &Message.m_BodyAllocated );
	}

	// MJM - javascript escape the subject, this allows for Unicode
	// subjects to be displayed...
	CEMSString sSubject = m_SrcMsg.IsInbound ? m_InboundMessage.m_Subject : m_OutboundMessage.m_Subject;
#ifdef UNICODE
	sSubject.EscapeJavascript(false);
#endif
	_tcscpy( Message.m_Subject, sSubject.c_str() );

	return;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Copy attachments from the source message	              
\*--------------------------------------------------------------------------*/
void CMsgComp::AddSourceMsgAttachments( int nMsgID )
{
	list<CAttachment>* pAttachmentList;
	list<CAttachment>::iterator iter;
	tstring sContentID;

	pAttachmentList = (m_SrcMsg.IsInbound) ? &m_InboundMessage.GetAttachmentList()
										   : &m_OutboundMessage.GetAttachmentList();
	
	TOutboundMessageAttachments outbound_attach;
	outbound_attach.m_OutboundMessageID = nMsgID;

	for ( iter = pAttachmentList->begin(); iter != pAttachmentList->end(); iter++ )
	{
		if( m_nArchiveID != 0 )
		{
			// We need to create the attachment files and records
			RestoreAttachmentFromArchive( GetQuery(), m_nArchiveID, 0, 
				iter->m_AttachmentID, sContentID );
		}

		// virus scan the attachment if it hasn't been already
		if ( iter->m_FileExists && iter->m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_NOT_SCANNED )
			VirusScanAttachment( *iter );

		outbound_attach.m_AttachmentID = iter->m_AttachmentID;
		outbound_attach.Insert( GetQuery() );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Support for ReplyToAll	              
\*--------------------------------------------------------------------------*/
template<class T> void CMsgComp::ReplyToAll(T& Message)
{
	bool bRemovedAddr = false;
	list<EmailAddr_t> AddrList;
		
	// add the reply-to or from address to the list
	ProcessEmailAddrString( Message.m_EmailReplyToLen ? Message.m_EmailReplyTo : Message.m_EmailFrom, 
		                    AddrList, false, true );
	
	// add the other To: addresses to the list
	ProcessEmailAddrString( Message.m_EmailTo, AddrList, false, true );

	// for inbound messages we do not want to reply to ourselves!!
	if ( m_SrcMsg.IsInbound )
	{
		list<EmailAddr_t>::iterator iter = AddrList.begin();
		dca::String sAgentEmail;
		dca::String sTicketBoxEmail;

		GetAgentEmail(sAgentEmail);
		GetTicketBoxEmail(sTicketBoxEmail);

		while ( iter != AddrList.end() )
		{
			if ( iter->m_sEmailAddr.compare( Message.m_EmailPrimaryTo ) == 0)
			{
				iter = AddrList.erase( iter );
				bRemovedAddr = true;
			}
			else if((sAgentEmail.size()) && (iter->m_sEmailAddr.compare(sAgentEmail) == 0))
			{
				iter = AddrList.erase( iter );
				bRemovedAddr = true;
			}
			else if((sTicketBoxEmail.size()) && (iter->m_sEmailAddr.compare(sTicketBoxEmail) == 0))
			{
				iter = AddrList.erase( iter );
				bRemovedAddr = true;
			}
			else
			{
				iter++;
			}
		}

		// Clean up agent and ticketbox emails as well.

	}

	// build the new To: address string
	FormatEmailAddrString( AddrList, &Message.m_EmailReplyTo, Message.m_EmailReplyToLen, Message.m_EmailReplyToAllocated );
	
	// if this is an outbound message, or if we have already
	// removed our address, we are done!!
	if ( !m_SrcMsg.IsInbound || bRemovedAddr )
		return;

	// attempt to find the primary to address in the CC addresses
	bRemovedAddr = false;
	AddrList.clear();

	ProcessEmailAddrString( Message.m_EmailCc, AddrList, false, true );
	list<EmailAddr_t>::iterator iter = AddrList.begin();

	dca::String sAgentEmail;
	dca::String sTicketBoxEmail;

	GetAgentEmail(sAgentEmail);
	GetTicketBoxEmail(sTicketBoxEmail);
		
	while ( iter != AddrList.end() )
	{
		if ( iter->m_sEmailAddr.compare( Message.m_EmailPrimaryTo ) == 0)
		{
			iter = AddrList.erase( iter );
			bRemovedAddr = true;
		}
		else if((sAgentEmail.size()) && (iter->m_sEmailAddr.compare(sAgentEmail) == 0))
		{
			iter = AddrList.erase( iter );
			bRemovedAddr = true;
		}
		else if((sTicketBoxEmail.size()) && (iter->m_sEmailAddr.compare(sTicketBoxEmail) == 0))
		{
			iter = AddrList.erase( iter );
			bRemovedAddr = true;
		}
		else
		{
			iter++;
		}
	}
	
	// we only need to reformat the list of cc: addresses if we removed our address
	if ( bRemovedAddr )
		FormatEmailAddrString( AddrList, &Message.m_EmailCc, Message.m_EmailCcLen, Message.m_EmailCcAllocated );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	 Puts the RE: or FW: in the subject.             
\*--------------------------------------------------------------------------*/
void CMsgComp::MangleSubject( LPTSTR szMsgSubject )
{
	tstring sPrefix;
	tstring sSubject;

	switch( m_nPageType )
	{
		case EMS_ReplyToMessage:
			sPrefix = _T("RE: ");
			break;
			
		case EMS_ForwardMessage:
			sPrefix = _T("FW: ");
			break;

		default:
			return;
	}

	// only add the prefix if it's not already there...
	if( _tcsnicmp( szMsgSubject, sPrefix.c_str(), sPrefix.size() ) != 0 )
		sSubject += sPrefix;

	// copy the orginal subject to the temp buffer
	sSubject += szMsgSubject;

	// copy the temp buffer back to the original
	_tcsncpy( szMsgSubject, sSubject.c_str(), INBOUNDMESSAGES_SUBJECT_LENGTH-1);
	
	// add ellipses if we truncated the subject
	if ( sSubject.length() > INBOUNDMESSAGES_SUBJECT_LENGTH-1)
	{	
		szMsgSubject[INBOUNDMESSAGES_SUBJECT_LENGTH-2] = _T('.');
		szMsgSubject[INBOUNDMESSAGES_SUBJECT_LENGTH-3] = _T('.');
		szMsgSubject[INBOUNDMESSAGES_SUBJECT_LENGTH-4] = _T('.');
	}
	
	szMsgSubject[INBOUNDMESSAGES_SUBJECT_LENGTH-1] = _T('\0');
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Saves and optionally sends the message	              
\*--------------------------------------------------------------------------*/
void CMsgComp::SaveMessage( void )
{	
	// check to see if Agent requires outbound message approval
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), m_nApprovalID );
	BINDPARAM_LONG( GetQuery(), GetSession().m_AgentID );
	GetQuery().Execute( _T("SELECT OutboundApprovalFromID FROM Agents WHERE AgentID=? ") );
	if ( GetQuery().Fetch() == S_OK )
	{
		if ( m_nApprovalID != 0 )
		{
			m_bOutboundApproval = true;
		}
	}
	
	// if we created the ticket, check security
	if ( m_bCreatedTicket )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, m_Ticket.m_TicketBoxID, EMS_EDIT_ACCESS );
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_CATEGORY, m_OutboundMessage.m_TicketCategoryID, EMS_EDIT_ACCESS );

		if ( m_Ticket.m_OwnerID > 0)
			RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_Ticket.m_OwnerID, EMS_ENUM_ACCESS );
	}

	if(m_OutboundMessage.m_MultiMail != 0 && !m_bDraft)
	{
		int nNumTo = 0;
		list<EmailAddr_t> ToList;					
		try
		{
			if ( m_OutboundMessage.m_EmailTo[strlen(m_OutboundMessage.m_EmailTo)-1] == _T(';') || m_OutboundMessage.m_EmailTo[strlen(m_OutboundMessage.m_EmailTo)-1] == _T(','))
					m_OutboundMessage.m_EmailTo[strlen(m_OutboundMessage.m_EmailTo)-1] = _T('\0');

			nNumTo = ProcessEmailAddrString( m_OutboundMessage.m_EmailTo, ToList );
			if(nNumTo < 2)
			{
				THROW_EMS_EXCEPTION_NOLOG( E_FormValidation, _T("The To: field must contain at least two valid email addresses if MultiMail is enabled") );
			}
			else
			{
				list<EmailAddr_t>::iterator iter = ToList.begin();
				int nCount = 0;
				CAttachment attachment(m_ISAPIData);
				list<CAttachment> pAttachmentList;
				attachment.ListOutboundMessageAttachments( m_OutboundMessage.m_OutboundMessageID, pAttachmentList );
				
				list<CAttachment>::iterator iter2;
				tstring sContentID;

				TOutboundMessageAttachments outbound_attach;

				while ( iter != ToList.end() )
				{
					list<EmailAddr_t> tList;
					tList.push_back( *iter );
					FormatEmailAddrString( tList, &m_OutboundMessage.m_EmailTo, m_OutboundMessage.m_EmailToLen, m_OutboundMessage.m_EmailToAllocated );
					if(nCount > 0)
					{
						COutboundMessage msg(m_ISAPIData);
						if(m_OutboundMessage.m_MultiMail > 5)
						{
							CTicket tkt(m_ISAPIData);
							int nNewTicketID = tkt.ReserveID();
							m_Ticket.m_TicketID = nNewTicketID;
							m_bCreatedTicket = true;
						}
						int nNewMsgID = msg.ReserveID( m_Ticket.m_TicketID );
						m_OutboundMessage.m_OutboundMessageID = nNewMsgID;
						outbound_attach.m_OutboundMessageID = nNewMsgID;

						for ( iter2 = pAttachmentList.begin(); iter2 != pAttachmentList.end(); iter2++ )
						{
							if( m_nArchiveID != 0 )
							{
								RestoreAttachmentFromArchive( GetQuery(), m_nArchiveID, 0, 
									iter2->m_AttachmentID, sContentID );
							}

							if ( iter2->m_FileExists && iter2->m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_NOT_SCANNED )
								VirusScanAttachment( *iter2 );

							outbound_attach.m_AttachmentID = iter2->m_AttachmentID;
							outbound_attach.Insert( GetQuery() );
						}
					}
					
					CEMSString sBody;
					CEMSString sBodySave;
					sBody.assign( m_OutboundMessage.m_Body );
					sBodySave.assign( m_OutboundMessage.m_Body );
					sBody.SetVariables(iter->m_sEmailAddr.c_str());
					PutStringProperty( sBody, &m_OutboundMessage.m_Body, &m_OutboundMessage.m_BodyAllocated );
					
					CreateMessage();
					
					PutStringProperty( sBodySave, &m_OutboundMessage.m_Body, &m_OutboundMessage.m_BodyAllocated );	
					
					
					if(nCount == 0 && (m_OutboundMessage.m_MultiMail == 4 || m_OutboundMessage.m_MultiMail == 5))
					{
						if ( m_bCreatedTicket )
						{
							UpdateTicket();
						}
						else
						{
							m_Ticket.ChangeTicketBox( m_Ticket.m_TicketBoxID );			
							m_Ticket.SetTicketCategory( m_OutboundMessage.m_TicketCategoryID );
							m_Ticket.ChangePriority( m_Ticket.m_PriorityID );
							m_Ticket.SetFolderID();			

							if ( !m_bDraft )
							{
								m_Ticket.Reassign( m_Ticket.m_OwnerID );
								m_Ticket.SetState( m_Ticket.m_TicketStateID );
								if( m_Ticket.m_UseTickler != -1 && m_Ticket.m_ReOpenTime.year != 0 && m_Ticket.m_TicketStateID==1)
								{
										m_Ticket.SetTickler();
								}
							}
							
							// Add/Update any Custom Ticket Fields
							for( tftIter = m_tft.begin(); tftIter != m_tft.end(); tftIter++ )
							{
								m_Ticket.AddTicketFieldTicket(tftIter->m_TicketID,tftIter->m_TicketFieldID,tftIter->m_TicketFieldTypeID, tftIter->m_DataValue);
							}
						
							m_Ticket.UnLock(true);							
						}						
					}
					else if(m_OutboundMessage.m_MultiMail > 5)
					{
						UpdateTicket();
					}
					
					BuildTicketContacts( GetQuery(), m_Ticket.m_TicketID );
					
					THAddOutboundMsg( GetQuery(), m_Ticket.m_TicketID, GetSession().m_AgentID, m_OutboundMessage.m_OutboundMessageID, m_Ticket.m_TicketBoxID );

					if ( _ttoi( m_Ticket.sAal.c_str() ) == 1 && _ttoi( m_Ticket.sTma.c_str() ) == 1 )
					{
						m_Ticket.LogAgentAction(GetSession().m_AgentID,11,m_OutboundMessage.m_OutboundMessageID,m_Ticket.m_TicketID,1,_T(""),_T(""));
					}
										
					SendMessage();
					
					iter++;
					nCount++;
				}
			}
		}
		catch( CEMSException EMSException )
		{
			THROW_VALIDATION_EXCEPTION( _T("to"), EMSException.GetErrorString() );
		}			
	}
	else
	{
		// create the message and add it to the ticket
		CreateMessage();	

		if ( m_bCreatedTicket )
		{
			UpdateTicket();
		}
		else
		{
			// if the ticket already existed, just update
			// its properties...
			m_Ticket.ChangeTicketBox( m_Ticket.m_TicketBoxID );			
			m_Ticket.SetTicketCategory( m_OutboundMessage.m_TicketCategoryID );
			m_Ticket.ChangePriority( m_Ticket.m_PriorityID );
			m_Ticket.SetFolderID();			

			if ( !m_bDraft )
			{
				m_Ticket.Reassign( m_Ticket.m_OwnerID );
				m_Ticket.SetState( m_Ticket.m_TicketStateID );
				if( m_Ticket.m_UseTickler != -1 && m_Ticket.m_ReOpenTime.year != 0 && m_Ticket.m_TicketStateID==1)
				{
						m_Ticket.SetTickler();
				}
			}
			
			// Add/Update any Custom Ticket Fields
			for( tftIter = m_tft.begin(); tftIter != m_tft.end(); tftIter++ )
			{
				m_Ticket.AddTicketFieldTicket(tftIter->m_TicketID,tftIter->m_TicketFieldID,tftIter->m_TicketFieldTypeID, tftIter->m_DataValue);
			}
		
			m_Ticket.UnLock(true);
		}
		
		// build the contacts string for the ticket
		BuildTicketContacts( GetQuery(), m_Ticket.m_TicketID );

		// send the message
		if ( !m_bDraft )
		{
			THAddOutboundMsg( GetQuery(), m_Ticket.m_TicketID, GetSession().m_AgentID, m_OutboundMessage.m_OutboundMessageID, m_Ticket.m_TicketBoxID );

			if ( _ttoi( m_Ticket.sAal.c_str() ) == 1 && _ttoi( m_Ticket.sTma.c_str() ) == 1 )
			{
				m_Ticket.LogAgentAction(GetSession().m_AgentID,11,m_OutboundMessage.m_OutboundMessageID,m_Ticket.m_TicketID,1,_T(""),_T(""));
			}
			SendMessage();
		}
		else
		{
			THSaveMsg( GetQuery(), m_Ticket.m_TicketID, GetSession().m_AgentID, m_OutboundMessage.m_OutboundMessageID );

			if ( _ttoi( m_Ticket.sAal.c_str() ) == 1 && _ttoi( m_Ticket.sTma.c_str() ) == 1 )
			{
				m_Ticket.LogAgentAction(GetSession().m_AgentID,25,m_OutboundMessage.m_OutboundMessageID,m_Ticket.m_TicketID,1,_T(""),_T(""));
			}
			
			GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,EMSERR_ISAPI_EXTENSION,
									EMS_LOG_OUTBOUND_MESSAGING, 0 ), _T("[%s] Saved Outbound Message (%d) as draft"), 
									GetAgentName(), m_OutboundMessage.m_OutboundMessageID ); 	
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Creates the outbound message	              
\*--------------------------------------------------------------------------*/
void CMsgComp::CreateMessage( void )
{
	list<EmailAddr_t> ContactList;
	list<int> ContactIDs;
	tstring sCleanedString;
	int noTo = 0;
	int noCc = 0;
	int noBcc = 0;
		
	// virus scan the message body
	/*CEMSString sVirusName;
	int nRet = VirusScanString( m_OutboundMessage.m_Body, m_OutboundMessage.m_BodyLen, sVirusName, sCleanedString );

	if ( nRet == E_CleanedBody )
	{
		PutStringProperty( sCleanedString, &m_OutboundMessage.m_Body, &m_OutboundMessage.m_BodyAllocated );
	}
	else if ( nRet != 0 )
	{
		CEMSString sError;
		sError.Format( EMS_STRING_ERROR_MSG_BODY, sVirusName.c_str() );
		THROW_EMS_EXCEPTION( E_InfectedBody, sError );
	}*/
	
	// validate FROM address
	CEMSString sFromAddress( m_OutboundMessage.m_EmailFrom );
	if ( !sFromAddress.ValidateEmailAddr() )
		THROW_VALIDATION_EXCEPTION( _T("from"), _T("The from email address is invalid") );

	// validate TO address
	try
	{
		// if the last character in the string is a delimiter, remove it
		if ( m_OutboundMessage.m_EmailTo[strlen(m_OutboundMessage.m_EmailTo)-1] == _T(';') || m_OutboundMessage.m_EmailTo[strlen(m_OutboundMessage.m_EmailTo)-1] == _T(','))
				m_OutboundMessage.m_EmailTo[strlen(m_OutboundMessage.m_EmailTo)-1] = _T('\0');

		int nRC = ProcessEmailAddrString( m_OutboundMessage.m_EmailTo, ContactList );
		
		if(nRC < 1 && !m_bDraft )
		{
			if(nRC == 0)
			{
				//THROW_EMS_EXCEPTION_NOLOG( E_FormValidation, CEMSString( EMS_STRING_ERROR_INVALID_TO_ADDRESS ) );
				noTo = 1;
			}
			else
			{
				CEMSString res(EMS_STRING_ERROR_INVALID_TO_ADDRESS_2);
				dca::String err;
				err.Format(res.c_str(), _T("to"));
				THROW_EMS_EXCEPTION_NOLOG( E_FormValidation, err.c_str() );
			}
		}
	}
	catch( CEMSException EMSException )
	{
		THROW_VALIDATION_EXCEPTION( _T("to"), EMSException.GetErrorString() );
	}

	// validate CC address
	try
	{
		// if the last character in the string is a delimiter, remove it
		if ( m_OutboundMessage.m_EmailCc[strlen(m_OutboundMessage.m_EmailCc)-1] == _T(';') || m_OutboundMessage.m_EmailCc[strlen(m_OutboundMessage.m_EmailCc)-1] == _T(','))
				m_OutboundMessage.m_EmailCc[strlen(m_OutboundMessage.m_EmailCc)-1] = _T('\0');

		int nRC = ProcessEmailAddrString( m_OutboundMessage.m_EmailCc, ContactList );

		if(nRC < 1 && !m_bDraft )
		{
			if(nRC == 0)
			{
				noCc = 1;
			}
			else
			{
				CEMSString res(EMS_STRING_ERROR_INVALID_TO_ADDRESS_2);
				dca::String err;
				err.Format(res.c_str(), _T("cc"));
				THROW_EMS_EXCEPTION_NOLOG( E_FormValidation, err.c_str() );
			}
		}		
	}
	catch( CEMSException EMSException )
	{
		THROW_VALIDATION_EXCEPTION( _T("cc"), EMSException.GetErrorString() );
	}

	// validate BCC address
	try
	{
		// if the last character in the string is a delimiter, remove it
		if ( m_OutboundMessage.m_EmailBcc[strlen(m_OutboundMessage.m_EmailBcc)-1] == _T(';') || m_OutboundMessage.m_EmailBcc[strlen(m_OutboundMessage.m_EmailBcc)-1] == _T(','))
				m_OutboundMessage.m_EmailBcc[strlen(m_OutboundMessage.m_EmailBcc)-1] = _T('\0');

		int nRC = ProcessEmailAddrString( m_OutboundMessage.m_EmailBcc, ContactList );

		if(nRC < 1 && !m_bDraft )
		{
			if(nRC == 0)
			{
				noBcc = 1;
			}
			else
			{
				CEMSString res(EMS_STRING_ERROR_INVALID_TO_ADDRESS_2);
				dca::String err;
				err.Format(res.c_str(), _T("bcc"));
				THROW_EMS_EXCEPTION_NOLOG( E_FormValidation, err.c_str() );
			}
		}
	}
	catch( CEMSException EMSException )
	{
		THROW_VALIDATION_EXCEPTION( _T("bcc"), EMSException.GetErrorString() );
	}

	if(noTo ==1 && noCc==1 && noBcc==1)
	{
		THROW_EMS_EXCEPTION_NOLOG( E_FormValidation, _T("The To, CC or BCC field must contain at least one valid email address") );
	}	
	
	if(noTo == 0)
	{
		// copy as much of the TO: address to the PrimaryTo field as we can 
		_tcsncpy( m_OutboundMessage.m_EmailPrimaryTo, m_OutboundMessage.m_EmailTo, 255);
	}	
	// set the mime-type
	_tcscpy( m_OutboundMessage.m_MediaSubType, m_UseHTML ? _T("html") : _T("plain") );
			
	// set other message properties
	m_OutboundMessage.m_TicketID = m_Ticket.m_TicketID;

	if ( !m_bDraft )
	{
		m_OutboundMessage.m_OutboundMessageStateID = !m_bOutboundApproval ? EMS_OUTBOUND_MESSAGE_STATE_UNTOUCHED : EMS_OUTBOUND_MESSAGE_STATE_WAITINGAPPROVAL;
	}
	else
	{
		m_OutboundMessage.m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_DRAFT;
	}
		
	if ( m_nPageType == EMS_SendAgain )
	{
		m_OutboundMessage.GetOutboundMessageType( m_SrcMsg.MsgID );
		m_OutboundMessage.GetReplyToIDAndType( m_SrcMsg.MsgID );
	}
	else if ( m_nPageType != EMS_SendDraft )
	{
		m_OutboundMessage.m_OutboundMessageTypeID = m_nPageType;
		m_OutboundMessage.m_ReplyToMsgID = m_SrcMsg.MsgID;
		m_OutboundMessage.m_ReplyToIDIsInbound = m_SrcMsg.IsInbound;
	}
	
	if ( !m_bDraft )
	{
		GetTicketBoxHeaderFooter();
		int nTicketTracking = 0;
		tstring sTicketTracking;
		GetServerParameter( EMS_SRVPARAM_TICKET_TRACKING, sTicketTracking, "0" );
		nTicketTracking = _ttoi( sTicketTracking.c_str() );
		if(m_OutboundMessage.m_MultiMail != 7 && m_OutboundMessage.m_MultiMail != 5)
		{
			bool bDoTrack = true;
			if(m_OutboundMessage.m_MultiMail == 0)
			{
				//Check to see if any of the recipients or recipient domains are in the Ticket Tracking Bypass
				map<unsigned int, XTrackingBypass> tbymap;
				map<unsigned int, XTrackingBypass>::iterator tbyiter;	
				GetXMLCache().m_TrackingBypass.GetMap(tbymap);
				if(tbymap.size() > 0)
				{
					//Check To: email addresses
					list<EmailAddr_t> addlist;
					list<EmailAddr_t>::iterator additer;		
					tstring sValue;
					int nType = 0;
					int addNum = ProcessEmailAddrString( m_OutboundMessage.m_EmailTo, addlist );
					if(addNum > 0)
					{
						bool bFound = false;
						for ( tbyiter = tbymap.begin(); tbyiter != tbymap.end(); tbyiter++ )
						{
							sValue.assign(tbyiter->second.m_EmailValue);
							nType = tbyiter->second.m_EmailTypeID;
							for ( additer = addlist.begin(); additer != addlist.end(); additer++ )
							{
								tstring sAdd;
								sAdd.assign(additer->m_sEmailAddr.c_str());
								if(nType == 0)
								{
									if( _tcsicmp( sValue.c_str(), sAdd.c_str() ) == 0 )
									{
										bFound = true;
										break;
									}									 
								}
								else
								{
									tstring sDom;
									sDom.assign(sAdd.substr(sAdd.find_last_of(_T("@"))+1, sAdd.length() - sAdd.find_last_of(_T("@"))+1));
									if( _tcsicmp( sValue.c_str(), sDom.c_str() ) == 0 )
									{
										bFound = true;
										break;
									}
								}
							}
							if(bFound)
							{
								bDoTrack = false;
								break;
							}							
						}
					}
					
					if(bDoTrack)
					{
						addlist.clear();
						addNum = ProcessEmailAddrString( m_OutboundMessage.m_EmailCc, addlist );
						if(addNum > 0)
						{
							bool bFound = false;
							for ( tbyiter = tbymap.begin(); tbyiter != tbymap.end(); tbyiter++ )
							{
								sValue.assign(tbyiter->second.m_EmailValue);
								nType = tbyiter->second.m_EmailTypeID;
								for ( additer = addlist.begin(); additer != addlist.end(); additer++ )
								{
									tstring sAdd;
									sAdd.assign(additer->m_sEmailAddr.c_str());
									if(nType == 0)
									{
										if( _tcsicmp( sValue.c_str(), sAdd.c_str() ) == 0 )
										{
											bFound = true;
											break;
										}									 
									}
									else
									{
										tstring sDom;
										sDom.assign(sAdd.substr(sAdd.find_last_of(_T("@"))+1, sAdd.length() - sAdd.find_last_of(_T("@"))+1));
										if( _tcsicmp( sValue.c_str(), sDom.c_str() ) == 0 )
										{
											bFound = true;
											break;
										}
									}
								}
								if(bFound)
								{
									bDoTrack = false;
									break;
								}							
							}
						}
					}
					if(bDoTrack)
					{
						addlist.clear();
						addNum = ProcessEmailAddrString( m_OutboundMessage.m_EmailBcc, addlist );
						if(addNum > 0)
						{
							bool bFound = false;
							for ( tbyiter = tbymap.begin(); tbyiter != tbymap.end(); tbyiter++ )
							{
								sValue.assign(tbyiter->second.m_EmailValue);
								nType = tbyiter->second.m_EmailTypeID;
								for ( additer = addlist.begin(); additer != addlist.end(); additer++ )
								{
									tstring sAdd;
									sAdd.assign(additer->m_sEmailAddr.c_str());
									if(nType == 0)
									{
										if( _tcsicmp( sValue.c_str(), sAdd.c_str() ) == 0 )
										{
											bFound = true;
											break;
										}									 
									}
									else
									{
										tstring sDom;
										sDom.assign(sAdd.substr(sAdd.find_last_of(_T("@"))+1, sAdd.length() - sAdd.find_last_of(_T("@"))+1));
										if( _tcsicmp( sValue.c_str(), sDom.c_str() ) == 0 )
										{
											bFound = true;
											break;
										}
									}
								}
								if(bFound)
								{
									bDoTrack = false;
									break;
								}							
							}
						}
					}
				}
			}
			if(bDoTrack)
			{
				PutTicketIDInSubject( m_Ticket.m_TicketID, m_OutboundMessage.m_OutboundMessageID, m_OutboundMessage.m_Subject, nTicketTracking );		
			}
			else
			{
				try
				{
					tstring sSubject(m_OutboundMessage.m_Subject);
					RemoveTicketIDFromSubject(sSubject);
					_tcscpy( m_OutboundMessage.m_Subject,sSubject.c_str() );
					if(m_bCreatedTicket)
					{
						m_sOutboundMessageSubject.assign(sSubject);
					}
				}
				catch(...){}
			}
		}
	}

	// add the contacts to the message and ticket
	ProcessContacts( ContactList, ContactIDs );
	int nAgentID = GetAgentID();

	list<int>::iterator iter;

	for ( iter = ContactIDs.begin(); iter != ContactIDs.end(); iter++ )
	{
		AddContactToTicket( GetQuery(), *iter, m_Ticket.m_TicketID, false );	
		AddContactToOutboundMsg( GetQuery(), *iter, m_OutboundMessage.m_OutboundMessageID, nAgentID );
	}
	
	if ( !m_bDraft && m_UseHTML )
	{
		// preform final processing of the HTML message before sending
		CEMSString sBody;
		sBody.assign( m_OutboundMessage.m_Body );
		sBody.FormatHTMLMsgForSend( m_OutboundMessage.m_OutboundMessageID );
		PutStringProperty( sBody, &m_OutboundMessage.m_Body, &m_OutboundMessage.m_BodyAllocated );
	}

	if ( m_UseHTML )
	{
		CEMSString sBody;
		sBody.assign( m_OutboundMessage.m_Body );
		sBody.FixInlineTags();
		sBody.FormatHTMLSigForSend();
		PutStringProperty( sBody, &m_OutboundMessage.m_Body, &m_OutboundMessage.m_BodyAllocated );
		AttachSigImages( GetQuery(), m_OutboundMessage.m_Body, m_OutboundMessage.m_OutboundMessageID);		
	}
	else
	{
		bool bIsIos = false;
		dca::String::size_type pos;
		tstring sUserAgent(GetBrowserSession().m_UserAgent);
		dca::String sTemp = sUserAgent.c_str();
		pos = sTemp.find("iPad");	
		if(pos != dca::String::npos)
		{
			bIsIos = true;
		}
		
		if(!bIsIos)
		{
			pos = sTemp.find("iPhone");	
			if(pos != dca::String::npos)
			{
				bIsIos = true;
			}
		}

		if(!bIsIos)
		{
			pos = sTemp.find("iPod");	
			if(pos != dca::String::npos)
			{
				bIsIos = true;
			}
		}

		if ( sTemp == "Chrome" || sTemp == "Safari" || bIsIos )
		{
			CEMSString sBody;
			sBody.assign( m_OutboundMessage.m_Body );
			sBody.FixLineEnds();
			PutStringProperty( sBody, &m_OutboundMessage.m_Body, &m_OutboundMessage.m_BodyAllocated );
		}		
	}	

	// update the outbound message
	UpdateMessage();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds the OutboundMessageID to the OutboundMsgQueue	              
\*--------------------------------------------------------------------------*/
void CMsgComp::SendMessage(void)
{	
	// insert the message into the outbound message queue
	TIMESTAMP_STRUCT now;
	GetQuery().Initialize();
	
	GetTimeStamp( now );
	
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_OutboundMessageID );
	BINDPARAM_TIME_NOLEN( GetQuery(), now );
	
	if ( m_bOutboundApproval )
	{
		GetQuery().Execute( _T("INSERT INTO OutboundMessageQueue (OutboundMessageID, DateSpooled, IsApproved) ")
							_T("VALUES (?,?,0)"));

		if ( m_nApprovalID != 0 )
		{
			//Add to approval queue
			TApprovals ap;
			ap.m_ApprovalObjectTypeID=1;
			if ( m_nApprovalID < 0 )
			{
				m_nApprovalID = abs(m_nApprovalID);
				ap.m_ApproverAgentID=0;
				ap.m_ApproverGroupID=m_nApprovalID;
			}
			else
			{
				ap.m_ApproverGroupID=0;
				ap.m_ApproverAgentID=m_nApprovalID;
			}
			ap.m_RequestAgentID = GetSession().m_AgentID;
			ap.m_ActualID = m_OutboundMessage.m_OutboundMessageID;
			ap.Insert( GetQuery() );
		}

		//Send alerts
		CheckAgentAlerts(GetAgentID(), m_OutboundMessage.m_Subject );
	}
	else
	{
		GetQuery().Execute( _T("INSERT INTO OutboundMessageQueue (OutboundMessageID, DateSpooled, IsApproved) ")
							_T("VALUES (?,?,1)"));
			
	}	

	GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,EMSERR_ISAPI_EXTENSION,
		EMS_LOG_OUTBOUND_MESSAGING, 0 ), _T("[%s] added Outbound Message ID: (%d) to the OutboundMessageQueue"), 
							  GetAgentName(), m_OutboundMessage.m_OutboundMessageID ); 
}

void CMsgComp::CheckAgentAlerts( int nAgentID, TCHAR szSubject[TICKETS_SUBJECT_LENGTH] )
{
	TAlerts alert;
	std::list<TAlerts> aeidList;
	std::list<TAlerts>::iterator aeidIter;
	bool bAlertAssign=false;
	dca::String t;
	CEMSString sSubject;
	int nAlertToID;
	list<int> AgentIDs;
	list<int>::iterator iter;

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nAgentID );
	BINDCOL_LONG_NOLEN( GetQuery(), nAlertToID );
	GetQuery().Execute( _T("SELECT OutboundApprovalFromID FROM Agents WHERE AgentID=? ") );
	if(GetQuery().Fetch() == S_OK)
	{
		if ( nAlertToID > 0 )
		{
			//Alerting to an Agent
			
			AgentIDs.push_back( nAlertToID );			
		}
		else if ( nAlertToID < 0 )
		{
			//Alerting to a group
			int iGroupID = abs( nAlertToID );
	
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), iGroupID );
			BINDCOL_LONG_NOLEN( GetQuery(), nAlertToID );			
			GetQuery().Execute( _T("SELECT AgentGroupings.AgentID ")
							_T("FROM AgentGroupings ")
							_T("INNER JOIN Groups ON AgentGroupings.GroupID = Groups.GroupID ")
							_T("WHERE AgentGroupings.GroupID=?") );

			while( GetQuery().Fetch() == S_OK )
			{
				AgentIDs.push_back( nAlertToID );				
			}			
		}
	}

	for( iter = AgentIDs.begin(); iter != AgentIDs.end(); iter++ )
	{
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), *iter );
		BINDCOL_LONG( GetQuery(), alert.m_AlertID );
		BINDCOL_LONG( GetQuery(), alert.m_AlertEventID );
		BINDCOL_LONG( GetQuery(), alert.m_AlertMethodID );
		BINDCOL_TCHAR( GetQuery(), alert.m_EmailAddress );
		GetQuery().Execute( _T("SELECT AlertID, AlertEventID, AlertMethodID, EmailAddress FROM Alerts WHERE AgentID=? AND AlertEventID=15 ") );
		while(GetQuery().Fetch() == S_OK)
		{
			/*dca::String t2(alert.m_EmailAddress);
			t.Format("CMsgComp::CheckAgentAlerts - Agent [%d] configured for AlertEventID [%d] AlertMethodID [%d] EmailAddress [%s]", *iter, alert.m_AlertEventID,alert.m_AlertMethodID,t2.c_str() );
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ISAPI);*/
			aeidList.push_back(alert);
		}
	}

	//sSubject = szSubjectLen ? szSubject : _T("[No Subject]");
	tstring tsSubject = szSubject;
	if ( tsSubject.size() == 0 )
	{
		sSubject = _T("[No Subject]");
	}
	else
	{
		sSubject = szSubject;
	}	
	
	for(aeidIter = aeidList.begin(); aeidIter != aeidList.end(); aeidIter++)
	{
		CEMSString strAlertBody;
		tstring sName;
		GetAgentName(nAgentID, sName);
		strAlertBody.Format( _T("Outbound Message %d from Agent %s requires approval|||An Outbound message from Agent %s requires approval.\n\nOutbound Message #%d %s\r\n"), m_OutboundMessage.m_OutboundMessageID, sName.c_str(), sName.c_str(), m_OutboundMessage.m_OutboundMessageID, sSubject.c_str() );						
		SendOutboundApprovalAlert( aeidIter->m_AlertID, strAlertBody.c_str() );				
	}
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Update the ticket record which we inserted as a place
||				holder.  If the place holder record has been deleted,
||				set the identity and insert it.	              
\*--------------------------------------------------------------------------*/
void CMsgComp::UpdateTicket(void)
{
	bool bTicketExists = false;

	// check if the reserved ticket still exists in 
	// the database...
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
	GetQuery().Execute( _T("SELECT TicketID FROM Tickets WHERE TicketID=?") );
	
	if ( GetQuery().Fetch() == S_OK )
		bTicketExists = true;

	// copy the subject from the outbound message
	_tcsncpy( m_Ticket.m_Subject, m_sOutboundMessageSubject.c_str(), 255);
	m_Ticket.m_IsDeleted = 0;
	
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketStateID );
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketBoxID );
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_OwnerID );
	BINDPARAM_TCHAR( GetQuery(), m_Ticket.m_Subject );
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_PriorityID );
	BINDPARAM_TIME( GetQuery(), m_Ticket.m_DateCreated );
	BINDPARAM_TIME( GetQuery(), m_Ticket.m_OpenTimestamp );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_TicketCategoryID );
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );

	if ( bTicketExists )
	{
		// update and unlock the ticket
		GetQuery().Execute( _T("UPDATE Tickets ")
							_T("SET TicketStateID=?,TicketBoxID=?,OwnerID=?,Subject=?,PriorityID=?,DateCreated=?,")
							_T("OpenTimestamp=?,TicketCategoryID=?,IsDeleted=0,LockedBy=0 WHERE TicketID=?") );

		if ( GetQuery().GetRowCount() != 1)
		{
			THROW_EMS_EXCEPTION( E_InvalidID, 
				CEMSString(_T("Unable to update ticket, the ticket no longer exsists in the database.")) );
		}
	}
	else
	{
		GetQuery().Execute( _T("SET IDENTITY_INSERT Tickets ON ") );

		GetQuery().Execute( _T("INSERT INTO Tickets ")
							_T("(TicketStateID,TicketBoxID,OwnerID,Subject,PriorityID,DateCreated,")
							_T("OpenTimestamp,TicketCategoryID,IsDeleted,LockedBy,TicketID,Contacts) ")
							_T("VALUES (?,?,?,?,?,?,?,?,0,0,?,'') ") );

		GetQuery().Execute( _T("SET IDENTITY_INSERT Tickets OFF ") );
	}

	if( m_Ticket.m_UseTickler != -1 && m_Ticket.m_ReOpenTime.year != 0 && m_Ticket.m_TicketStateID==1)
	{
			m_Ticket.SetTickler();
	}

	// Add/Update any Custom Ticket Fields
	for( tftIter = m_tft.begin(); tftIter != m_tft.end(); tftIter++ )
	{
		m_Ticket.AddTicketFieldTicket(tftIter->m_TicketID,tftIter->m_TicketFieldID,tftIter->m_TicketFieldTypeID, tftIter->m_DataValue);
	}
	
	THCreateTicket( GetQuery(), m_Ticket.m_TicketID, GetAgentID(), 0, m_Ticket.m_TicketStateID, m_Ticket.m_TicketBoxID, m_Ticket.m_OwnerID, m_Ticket.m_PriorityID, m_Ticket.m_TicketCategoryID );

	//Log it to Agent Activity
	if( _ttoi( m_Ticket.sAal.c_str() ) == 1 && _ttoi( m_Ticket.sTc.c_str() ) == 1 )
	{
		m_Ticket.LogAgentAction(GetAgentID(),4,m_Ticket.m_TicketID,m_Ticket.m_TicketBoxID,0,_T(""),_T(""));		
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Update the outbound message record which we inserted 
||				as a place holder.  If the place holder record has been 
||				deleted set the identity and insert it.	              
\*--------------------------------------------------------------------------*/
void CMsgComp::UpdateMessage( void )
{
	bool bMsgExists = false;
	bool bMsgUpdated = true;

	// check if the reserved message still exists in the database...
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_OutboundMessageID );
	GetQuery().Execute( _T("SELECT OutboundMessageID FROM OutboundMessages WHERE OutboundMessageID=?") );
	
	if ( GetQuery().Fetch() == S_OK )
		bMsgExists = true;

	if(bMsgExists)
	{
		if(m_OutboundMessage.m_ReplyToMsgID == 0 && m_OutboundMessage.m_ReplyToIDIsInbound == 0)
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_OutboundMessageID );
			BINDCOL_LONG( GetQuery(), m_OutboundMessage.m_ReplyToMsgID);
			BINDCOL_LONG( GetQuery(), m_OutboundMessage.m_ReplyToIDIsInbound);
			GetQuery().Execute( _T("SELECT ReplyToMsgID, ReplyToIDIsInbound FROM OutboundMessages WHERE OutboundMessageID=?") );
			GetQuery().Fetch();
		}
	}

	m_OutboundMessage.m_IsDeleted = 0;

	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_OutboundMessage.m_EmailFrom );
	BINDPARAM_TCHAR( GetQuery(), m_OutboundMessage.m_EmailPrimaryTo );
	BINDPARAM_TCHAR( GetQuery(), m_OutboundMessage.m_Subject );
	BINDPARAM_TCHAR( GetQuery(), m_OutboundMessage.m_MediaType );
	BINDPARAM_TCHAR( GetQuery(), m_OutboundMessage.m_MediaSubType );
	BINDPARAM_TIME( GetQuery(), m_OutboundMessage.m_EmailDateTime );
	BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_IsDeleted );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_AgentID );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_ReplyToMsgID );
	BINDPARAM_BIT( GetQuery(), m_OutboundMessage.m_ReplyToIDIsInbound );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_OutboundMessageTypeID );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_OutboundMessageStateID );
	BINDPARAM_TEXT( GetQuery(), m_OutboundMessage.m_Body );
	BINDPARAM_TEXT( GetQuery(), m_OutboundMessage.m_EmailTo );
	BINDPARAM_TEXT( GetQuery(), m_OutboundMessage.m_EmailCc );
	BINDPARAM_TEXT( GetQuery(), m_OutboundMessage.m_EmailBcc );
	BINDPARAM_TEXT( GetQuery(), m_OutboundMessage.m_EmailReplyTo );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_TicketBoxHeaderID );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_TicketBoxFooterID );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_PriorityID );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_SignatureID );
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketBoxID );
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_TicketCategoryID );
	BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_DraftCloseTicket );
	BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_DraftRouteToMe );
	BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_FooterLocation );
	BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_ReadReceipt );
	BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_DeliveryConfirmation );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_MultiMail );
	BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_OutboundMessageID );

	
	GetQuery().Execute( _T("UPDATE OutboundMessages ")
		_T("SET EmailFrom=?,EmailPrimaryTo=?,Subject=?,MediaType=?,MediaSubType=?,EmailDateTime=?,")
		_T("IsDeleted=?,AgentID=?,ReplyToMsgID=?,ReplyToIDIsInbound=?,OutboundMessageTypeID=?,")
		_T("OutboundMessageStateID=?,Body=?,EmailTo=?,EmailCc=?,EmailBcc=?,EmailReplyTo=?,TicketBoxHeaderID=?,")
		_T("TicketBoxFooterID=?,PriorityID=?,SignatureID=?,OriginalTicketBoxID=?,TicketID=?,TicketCategoryID=?,")
		_T("DraftCloseTicket=?,DraftRouteToMe=?,FooterLocation=?,ReadReceipt=?,DeliveryConfirmation=?,MultiMail=? ")
		_T("WHERE OutboundMessageID=?") );

	if ( GetQuery().GetRowCount() != 1)
		bMsgUpdated=false;
	
		
	if ( !bMsgUpdated )
	{
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_OutboundMessageID );
		BINDPARAM_TCHAR( GetQuery(), m_OutboundMessage.m_EmailFrom );
		BINDPARAM_TCHAR( GetQuery(), m_OutboundMessage.m_EmailPrimaryTo );
		BINDPARAM_TCHAR( GetQuery(), m_OutboundMessage.m_Subject );
		BINDPARAM_TCHAR( GetQuery(), m_OutboundMessage.m_MediaType );
		BINDPARAM_TCHAR( GetQuery(), m_OutboundMessage.m_MediaSubType );
		BINDPARAM_TIME( GetQuery(), m_OutboundMessage.m_EmailDateTime );
		BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_IsDeleted );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_AgentID );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_ReplyToMsgID );
		BINDPARAM_BIT( GetQuery(), m_OutboundMessage.m_ReplyToIDIsInbound );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_OutboundMessageTypeID );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_OutboundMessageStateID );
		BINDPARAM_TEXT( GetQuery(), m_OutboundMessage.m_Body );
		BINDPARAM_TEXT( GetQuery(), m_OutboundMessage.m_EmailTo );
		BINDPARAM_TEXT( GetQuery(), m_OutboundMessage.m_EmailCc );
		BINDPARAM_TEXT( GetQuery(), m_OutboundMessage.m_EmailBcc );
		BINDPARAM_TEXT( GetQuery(), m_OutboundMessage.m_EmailReplyTo );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_TicketBoxHeaderID );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_TicketBoxFooterID );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_PriorityID );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_SignatureID );
		BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketBoxID );
		BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_TicketCategoryID );
		BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_DraftCloseTicket );
		BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_DraftRouteToMe );
		BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_FooterLocation );
		BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_ReadReceipt );
		BINDPARAM_TINYINT( GetQuery(), m_OutboundMessage.m_DeliveryConfirmation );	
		BINDPARAM_LONG( GetQuery(), m_OutboundMessage.m_MultiMail );
			
		GetQuery().Execute( _T("SET IDENTITY_INSERT OutboundMessages ON ") );

		GetQuery().Execute( 
			_T("INSERT INTO OutboundMessages ")
			_T("(OutboundMessageID,EmailFrom,EmailPrimaryTo,Subject,MediaType,MediaSubType,EmailDateTime,")
			_T("IsDeleted,AgentID,ReplyToMsgID,ReplyToIDIsInbound,OutboundMessageTypeID,")
			_T("OutboundMessageStateID,Body,EmailTo,EmailCc,EmailBcc,EmailReplyTo,TicketBoxHeaderID,")
			_T("TicketBoxFooterID,PriorityID,SignatureID,OriginalTicketBoxID,TicketID,TicketCategoryID,")
			_T("DraftCloseTicket,DraftRouteToMe,FooterLocation,ReadReceipt,DeliveryConfirmation,MultiMail) ")
			_T("VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) ") );

		if ( GetQuery().GetRowCount() != 1)
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_ERROR_UPDATE_MSG) );

		GetQuery().Execute( _T("SET IDENTITY_INSERT OutboundMessages OFF ") );
	}	
}
/*---------------------------------------------------------------------------\                     
||  Comments:	Generates a list of ContactIDs from a list of contact names
||				and addresses.  Contacts will be created if they are not
||				found in the database.
\*--------------------------------------------------------------------------*/
void CMsgComp::ProcessContacts( list<EmailAddr_t>& ContactList, list<int>& ContactIDs )
{
	CCreateContactMutex lock;
	list<EmailAddr_t>::iterator iter;
	int nContactID;

	// security check - the agent must have at least read access to contacts
	unsigned char access = 
		RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS );

	// obtain the new contact lock		
	if( lock.AcquireLock( 1000 ) == false )
		THROW_EMS_EXCEPTION( E_CreateContact, CEMSString(EMS_STRING_ERROR_CREATE_CONTACT) );

	for ( iter = ContactList.begin(); iter != ContactList.end(); iter++ )
	{	
		// attempt to find the contact, if we can't find it, create it..
		if ( ( nContactID = FindContactByEmail( GetQuery(), iter->m_sEmailAddr.c_str() ) ) == 0)
		{
			// didn't find the contact, if we don't have edit rights, throw an error
			if (access < EMS_EDIT_ACCESS)
				LogSecurityViolation( EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS );
			
			// create the contact
			if ((nContactID = CreateContactFromEmail( GetQuery(), iter->m_sEmailAddr.c_str(), iter->m_sName.c_str() )) == 0)
				THROW_EMS_EXCEPTION( E_CreateContact, CEMSString(EMS_STRING_ERROR_CREATE_CONTACT) );
		}

		// add the ContactID to the list
		ContactIDs.push_back( nContactID );		
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the TicketBox header and footer IDs for a ticketbox        
\*--------------------------------------------------------------------------*/
void CMsgComp::GetTicketBoxHeaderFooter( void )
{
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), m_OutboundMessage.m_TicketBoxHeaderID );
	BINDCOL_LONG_NOLEN( GetQuery(), m_OutboundMessage.m_TicketBoxFooterID );
	BINDCOL_TINYINT_NOLEN( GetQuery(), m_OutboundMessage.m_FooterLocation );
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketBoxID );
	
	GetQuery().Execute( _T("SELECT HeaderID, FooterID, FooterLocation FROM TicketBoxes WHERE TicketBoxID = ?") );
	
	if ( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Invalid TicketBoxID %d"), m_Ticket.m_TicketBoxID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds the  To: address when sending a message to a contact ID
\*--------------------------------------------------------------------------*/
void CMsgComp::AddContactEmailAddr( int nContactID )
{
	TCHAR Name[CONTACTS_NAME_LENGTH] = {0};
	long NameLen;
	TCHAR Email[PERSONALDATA_DATAVALUE_LENGTH] = {0};
	long EmailLen;
	CEMSString sFullAddress;

	GetQuery().Initialize();

	BINDCOL_TCHAR( GetQuery(), Name );
	BINDCOL_TCHAR( GetQuery(), Email );
	BINDPARAM_LONG( GetQuery(), nContactID );
	
	GetQuery().Execute( _T("SELECT Name, (SELECT DataValue FROM PersonalData ")
						_T("WHERE PersonalDataID = Contacts.DefaultEmailAddressID) ")
						_T("FROM Contacts WHERE ContactID=?") );

	if ( GetQuery().Fetch() == S_OK && EmailLen )
	{
		if ( NameLen )
		{
			sFullAddress.Format( _T("\"%s\" <%s>"), Name, Email ); 
		}
		else
		{
			sFullAddress = Email;
		}

		// set the default message subject to that of the ticket
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("To") );
		GetXMLGen().SetChildData( sFullAddress.c_str(), TRUE );
		GetXMLGen().OutOfElem();
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Returns a string of the primary email address of
||				all contacts associated with the ticket
\*--------------------------------------------------------------------------*/
void CMsgComp::BuildTicketEmailAddresses( CEMSString& sAddresses )
{
	TCHAR Email[PERSONALDATA_DATAVALUE_LENGTH];
	long EmailLen;
	
	// clear the string
	sAddresses.erase();
	
	// query the database for the primary email address
	// for all contacts associated with the ticket
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
	BINDCOL_TCHAR( GetQuery(), Email );
	
	GetQuery().Execute( _T("SELECT (SELECT DataValue FROM PersonalData WHERE PersonalDataID = Contacts.DefaultEmailAddressID ) ")
		                _T("FROM TicketContacts INNER JOIN Contacts ON TicketContacts.ContactID = Contacts.ContactID ")
						_T("WHERE TicketID=? AND Contacts.IsDeleted=0 ORDER BY TicketContactID") );
	
	// for each contact
	int nIndex = 0;
	
	while ( GetQuery().Fetch() == S_OK )
	{
		if ( EmailLen && EmailLen != SQL_NULL_DATA )
		{	
			// if the string would be too long, we are done
			if ( sAddresses.length() + EmailLen > PERSONALDATA_DATAVALUE_LENGTH - 2 )
				return;
			
			if( nIndex )
				sAddresses.append( _T("; ") );
			
			sAddresses.append(Email);
			
			nIndex++;	
		}
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Returns a string of contact email addresses 
\*--------------------------------------------------------------------------*/
void CMsgComp::BuildContactEmailAddresses( void )
{
	TCHAR Email[PERSONALDATA_DATAVALUE_LENGTH];
	long EmailLen;
	TCHAR Name[PERSONALDATA_DATAVALUE_LENGTH];
	long NameLen;
	int nAgentID = GetAgentID();
	 
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nAgentID );
	BINDCOL_TCHAR( GetQuery(), Name );
	BINDCOL_TCHAR( GetQuery(), Email );
	
	GetQuery().Execute( _T("SELECT Contacts.Name,(SELECT DataValue FROM PersonalData WHERE PersonalDataID = Contacts.DefaultEmailAddressID ) ")
		                _T("FROM AgentContacts INNER JOIN Contacts ON AgentContacts.ContactID = Contacts.ContactID ")
						_T("WHERE AgentContacts.AgentID=? AND Contacts.IsDeleted=0") );
	
	int nIndex = 0;
	GetXMLGen().AddChildElem( _T("Contacts") );
	GetXMLGen().IntoElem();
	while ( GetQuery().Fetch() == S_OK )
	{
		CEMSString sName;
		sName.reserve( PERSONALDATA_DATAVALUE_LENGTH );
		sName.assign( Name );
		if(sName.IsStringValid())
		{
			sName.EscapeJavascript();
		}
		else
		{
			sName.Format(_T(""));
		}

		CEMSString sEmail;
		sEmail.reserve( PERSONALDATA_DATAVALUE_LENGTH );
		sEmail.assign( Email );
		sEmail.EscapeJavascript();

		GetXMLGen().AddChildElem( _T("Contact") );
		GetXMLGen().AddChildAttrib( _T("Index"), nIndex );
		GetXMLGen().AddChildAttrib( _T("Name"), sName.c_str() );
		GetXMLGen().AddChildAttrib( _T("Email"), sEmail.c_str() );
		nIndex++;
	}
	GetXMLGen().OutOfElem();
		
}

/*---------------------------------------------------------------------------\            
||  Comments:	Returns a string of contact email addresses 
\*--------------------------------------------------------------------------*/
void CMsgComp::BuildContactGroups( void )
{
	TCHAR GroupName[GROUPS_GROUPNAME_LENGTH];
	long GroupNameLen;	
	int nAgentID = GetAgentID();
		 
	int ContactGroupID;
	list<TContacts> CGMap;	
	TContacts tc;	
	
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), tc.m_ContactID );
	BINDCOL_TCHAR( GetQuery(), tc.m_Name );
	BINDCOL_TCHAR( GetQuery(), tc.m_WebPageAddress );
	BINDPARAM_LONG( GetQuery(), nAgentID );	
	GetQuery().Execute( _T("SELECT cg.ContactGroupID, c.Name, ")
					    _T("(SELECT DataValue FROM PersonalData WHERE PersonalDataID=c.DefaultEmailAddressID) ")
						_T("FROM ContactGroups cg INNER JOIN ContactGrouping cgr ON cg.ContactGroupID=cgr.ContactGroupID ") 
						_T("INNER JOIN Contacts c ON cgr.ContactID = c.ContactID ")
						_T("WHERE cg.OwnerID=? OR cg.OwnerID=0 AND c.IsDeleted=0 ORDER BY cg.ContactGroupID") );	
	
	while( GetQuery().Fetch() == S_OK )
	{
		CGMap.push_back(tc);		
	}

	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), ContactGroupID );
	BINDCOL_TCHAR( GetQuery(), GroupName );	
	BINDPARAM_LONG( GetQuery(), nAgentID );
	GetQuery().Execute( _T("SELECT G.ContactGroupID,G.GroupName ")
						_T("FROM ContactGroups AS G ") 
						_T("WHERE G.OwnerID = ? OR G.OwnerID = 0 ")
						_T("ORDER BY G.GroupName ") );
	
	GetXMLGen().AddChildElem( _T("ContactGroups") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Group") );
		GetXMLGen().AddChildAttrib( _T("GroupName"), GroupName );
		GetXMLGen().IntoElem();
		list<TContacts>::iterator iter = CGMap.begin();
		while ( iter != CGMap.end() )
		{
			if ( iter->m_ContactID == ContactGroupID )
			{
				GetXMLGen().AddChildElem( _T("Member") );
				GetXMLGen().AddChildAttrib( _T("GroupName"), GroupName );
				CEMSString sName;
				sName.reserve( PERSONALDATA_DATAVALUE_LENGTH );
				sName.assign( iter->m_Name );
				if(sName.IsStringValid())
				{
					sName.EscapeJavascript();
				}
				else
				{
					sName.Format(_T(""));
				}
				GetXMLGen().AddChildAttrib( _T("Name"), sName.c_str() );
				CEMSString sEmail;
				sEmail.reserve( PERSONALDATA_DATAVALUE_LENGTH );
				sEmail.assign( iter->m_WebPageAddress );
				sEmail.EscapeJavascript();
				GetXMLGen().AddChildAttrib( _T("Email"), sEmail.c_str() );
			}
			iter++;	
		}
		GetXMLGen().OutOfElem();
	}
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	List Frequently Used and Favorite Standard Responses for 
||				pulldown menus. 	              
\*--------------------------------------------------------------------------*/
void CMsgComp::ListStdResps( void )
{
	// check security
	unsigned char access = 2;
	
	CStdResponse StandardResponse(m_ISAPIData);
	
	GetXMLGen().AddChildElem( _T("MostUsedStdResponses") );
	
	if ( access >= EMS_READ_ACCESS )
	{
		GetXMLGen().IntoElem();
		StandardResponse.ListMostFrequentUsed();
		GetXMLGen().OutOfElem();
	}
	
	GetXMLGen().AddChildElem( _T("FavoriteStdResponses") );
	
	if ( access >= EMS_READ_ACCESS )
	{
		GetXMLGen().IntoElem();
		StandardResponse.ListMyFavorites();
		GetXMLGen().OutOfElem();
	}
	
	if( m_Ticket.m_TicketBoxID > 0 )
	{
		GetXMLGen().AddChildElem( _T("ByTicketBox") );
	
		if ( access >= EMS_READ_ACCESS )
		{
			GetXMLGen().IntoElem();
			StandardResponse.ListByTicketBox( m_Ticket.m_TicketBoxID );
			GetXMLGen().OutOfElem();
		}
	}

	GetXMLGen().AddChildElem( _T("ByTicketCategory") );
	
	if ( access >= EMS_READ_ACCESS )
	{
		
		if ( m_Ticket.m_TicketCategoryID == 1 )
		{
			// check to see if a ticket category ID is already set
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
			BINDCOL_LONG( GetQuery(), m_Ticket.m_TicketCategoryID);
			GetQuery().Execute( _T("SELECT TicketCategoryID FROM Tickets WHERE TicketID=?") );
			GetQuery().Fetch();
		}
				
		GetXMLGen().IntoElem();
		StandardResponse.ListByTicketCategory( m_Ticket.m_TicketCategoryID);
		GetXMLGen().OutOfElem();
	}

	GetXMLGen().AddChildElem( _T("KeywordSearchResults") );

	if ( access >= EMS_READ_ACCESS )
	{
		if(m_SrcMsg.IsInbound)
		{
			long lStandardResponseID = 0;
			long lScore = 0;
			TCHAR tSubject[STANDARDRESPONSES_SUBJECT_LENGTH];
			long tSubjectLen;
			ZeroMemory(tSubject, STANDARDRESPONSES_SUBJECT_LENGTH);
			TCHAR* pStandardResponse = 0;
			long pStandardResponseAllocated = 0;
 			long pStandardResponseLen = 0;

			pStandardResponseAllocated = EMS_TEXT_BUFF_LEN_SEED;
			pStandardResponse = (TCHAR*)calloc( EMS_TEXT_BUFF_LEN_SEED, 1 );

			GetQuery().Initialize();

			BINDPARAM_LONG(GetQuery(), m_originalInboundMessageID);
			
			GetQuery().Execute(_T("SELECT TOP 25 SRKeywordResults.StandardResponseID, SRKeywordResults.Score, StandardResponses.Subject, StandardResponses.StandardResponse FROM SRKeywordResults INNER JOIN StandardResponses ON SRKeywordResults.StandardResponseID = StandardResponses.StandardResponseID WHERE (SRKeywordResults.InboundMessageID = ?) AND StandardResponses.IsDeleted = 0 ORDER BY SRKeywordResults.Score DESC"));

			BINDCOL_LONG_NOLEN(GetQuery(), lStandardResponseID);
			BINDCOL_LONG_NOLEN(GetQuery(), lScore);
			BINDCOL_TCHAR(GetQuery(), tSubject);

			GetXMLGen().IntoElem();
			int nRowIndex = 0;
			while(GetQuery().Fetch() == S_OK)
			{
				GETDATA_TEXT(GetQuery(), pStandardResponse);

				CEMSString sSubject;
				CEMSString sString;
				sSubject.reserve( STANDARDRESPONSES_SUBJECT_LENGTH );

				GetXMLGen().AddChildElem( _T("StdResponse") );
				GetXMLGen().AddChildAttrib( _T("rowIndex"), nRowIndex++ );
				GetXMLGen().AddChildAttrib( _T("ID"), lStandardResponseID );

				sSubject.assign( tSubject );
				sSubject.EscapeJavascript();
				sSubject.EscapeHTML();
		
				GetXMLGen().AddChildAttrib( _T("Subject"), sSubject.c_str() );
				GetXMLGen().AddChildAttrib(_T("Score"), lScore );

				GetXMLGen().IntoElem();
				sString.assign(pStandardResponse);
				sString.EscapeJavascript();
				sString.EscapeHTML();
				GetXMLGen().AddChildElem( _T("SRBody"), sString.c_str());
				GetXMLGen().OutOfElem();
			}
			GetXMLGen().OutOfElem();

			if (pStandardResponseAllocated > 0) free( pStandardResponse );
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Determine if the message composition pop-up window
||				needs to refresh its parent.	              
\*--------------------------------------------------------------------------*/
void CMsgComp::RefreshParentView( void )
{
	TTicketBoxViews TBoxView;
	bool bRefresh = false;
	int nInitialView;
	int nParentView;

	GetISAPIData().GetXMLLong( _T("TicketBoxView"), nInitialView );
	GetISAPIData().GetXMLLong( _T("ParentTicketBoxView"), nParentView );
	
	TBoxView.m_TicketBoxViewID = nParentView;

	if ( TBoxView.m_TicketBoxViewID > 0 )
	{
		if ( TBoxView.Query( GetQuery() ) != S_OK )
			return;
	}
	else
	{
		TBoxView.m_AgentBoxID = 0;
		TBoxView.m_ShowClosedItems = TRUE;
		TBoxView.m_ShowOwnedItems = TRUE;
		TBoxView.m_TicketBoxViewTypeID = EMS_PUBLIC;
	}
		
	switch ( TBoxView.m_TicketBoxViewTypeID )
	{
	case EMS_INBOX:

		// the ticket must have been displayed in the intial view, and the agent 
		// hasn't switched views...
		if ( nInitialView == nParentView && m_nPageType != EMS_NewMessage )
		{	
			bRefresh = true;
			break;
		}

		// ticket will be displayed in the view...
		if ((m_Ticket.m_OwnerID == TBoxView.m_AgentBoxID && 
		    (m_Ticket.m_TicketStateID != EMS_TICKETSTATEID_CLOSED || TBoxView.m_ShowClosedItems))) 
		{
			bRefresh = true;
			break;
		}

		// the ticket is currently displayed in the view...
		if ( !m_bCreatedTicket )
		{
			CTicket InitialTicket(m_ISAPIData);
			InitialTicket.m_TicketID = m_Ticket.m_TicketID;
			InitialTicket.Query();

			if (InitialTicket.m_OwnerID == TBoxView.m_AgentBoxID &&	
				(InitialTicket.m_TicketStateID != EMS_TICKETSTATEID_CLOSED || TBoxView.m_ShowClosedItems))
			{
				bRefresh = true;
				break;
			}
		}

		break;

	case EMS_PUBLIC:

		// the ticket must have been displayed in the intial view, and the agent 
		// hasn't switched views...
		if ( nInitialView == nParentView && m_nPageType != EMS_NewMessage )
		{	
			bRefresh = true;
			break;
		}

		// ticket will be displayed in the view...
		if ((m_Ticket.m_TicketBoxID == TBoxView.m_TicketBoxID) && 
			(!m_Ticket.m_TicketStateID != EMS_TICKETSTATEID_CLOSED || TBoxView.m_ShowClosedItems) &&
			(m_Ticket.m_OwnerID == 0 || TBoxView.m_ShowOwnedItems )) 
		{
			bRefresh = true;
			break;
		}

		if ( !m_bCreatedTicket )
		{
			CTicket InitialTicket(m_ISAPIData);
			InitialTicket.m_TicketID = m_Ticket.m_TicketID;
			InitialTicket.Query();

			// the ticket is currently displayed in the view
			if ((InitialTicket.m_TicketBoxID == TBoxView.m_TicketBoxID) && 
				(InitialTicket.m_TicketStateID != EMS_TICKETSTATEID_CLOSED || TBoxView.m_ShowClosedItems) &&
				(InitialTicket.m_OwnerID == 0 || TBoxView.m_ShowOwnedItems)) 
			{
				bRefresh = true;
				break;
			}
		}

		break;

	case EMS_DRAFTS:
			
		// always refresh for drafts
		bRefresh = true;
		break;
		
	case EMS_OUTBOX:
	case EMS_SENT_ITEMS:

		if ( !m_bDraft )
		{
			// refresh if the view is for the logged in agent
			if ( TBoxView.m_AgentBoxID == GetAgentID() )
				bRefresh = true;
		}

		break;
	}

	if ( bRefresh )
		GetXMLGen().AddChildElem( _T("RefreshView" ) );
}

void CMsgComp::GetAgentEmail(dca::String& agentEmail)
{
	dca::String255 sEmail;
	long sEmailLen;
	ZeroMemory(sEmail, 256);
	long lAgentID = GetISAPIData().m_pSession->m_AgentID;

	GetQuery().Initialize();
	BINDPARAM_LONG(GetQuery(), lAgentID);
	BINDCOL_TCHAR(GetQuery(), sEmail);

	GetQuery().Execute(_T("SELECT PersonalData.DataValue FROM PersonalData INNER JOIN Agents ON PersonalData.AgentID = Agents.AgentID WHERE (PersonalData.PersonalDataTypeID = 1) AND (PersonalData.ContactID = 0) AND (Agents.AgentID = ?)"));
	
	if(GetQuery().Fetch() == S_OK)
		agentEmail.assign(sEmail);
}

void CMsgComp::GetTicketBoxEmail(dca::String& ticketboxEmail)
{
	dca::String255 sEmail;
	long sEmailLen;
	ZeroMemory(sEmail, 256);

	GetQuery().Initialize();
	BINDPARAM_LONG(GetQuery(), m_InboundMessage.m_TicketID);
	BINDPARAM_LONG(GetQuery(), m_InboundMessage.m_InboundMessageID);
	BINDCOL_TCHAR(GetQuery(), sEmail);

	GetQuery().Execute(_T("SELECT TicketBoxes.DefaultEmailAddress FROM TicketBoxes INNER JOIN Tickets ON TicketBoxes.TicketBoxID = Tickets.TicketBoxID INNER JOIN InboundMessages ON Tickets.TicketID = InboundMessages.TicketID WHERE (InboundMessages.TicketID = ?) AND (InboundMessages.InboundMessageID = ?)"));

	if(GetQuery().Fetch() == S_OK)
		ticketboxEmail.assign(sEmail);
}