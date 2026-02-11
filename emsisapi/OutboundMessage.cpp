/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/OutboundMessage.cpp,v 1.2 2005/11/29 21:16:26 markm Exp $
||
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "OutboundMessage.h"
#include "Ticket.h"
#include "AttachFns.h"
#include "StringFns.h"
#include "ContactFns.h"
#include "TicketHistoryFns.h"
#include "TicketNotes.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction/Destruction	              
\*--------------------------------------------------------------------------*/
COutboundMessage::COutboundMessage(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{
	GetTimeStamp( m_EmailDateTime );
	m_EmailDateTimeLen = sizeof(m_EmailDateTime);
	
	m_IsDeleted = EMS_DELETE_OPTION_DELETE_PERMANENTLY;
	m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_DRAFT;
	m_OutboundMessageTypeID  = EMS_OUTBOUND_MESSAGE_TYPE_NEW;
	m_PriorityID = EMS_PRIORITY_NORMAL;
	
	m_AgentID = GetAgentID();
	_tcscpy(m_MediaType, _T("text"));
	
	m_RepliedToWhenLen = SQL_NULL_DATA;	
}

COutboundMessage::~COutboundMessage()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates XML with the information for an 
||				OutboundMessage              
\*--------------------------------------------------------------------------*/
void COutboundMessage::GenerateXML( bool bEscapeBody, int nArchiveID, bool bForEdit )
{
	CEMSString sDateTime;
	CEMSString sBody;
	CEMSString sSubject;
	int nIsHTML = _tcsicmp(m_MediaSubType, _T("html")) == 0 ? 1 : 0;
	
	GetXMLGen().AddChildElem( _T("Message") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_OutboundMessageID );
	GetXMLGen().AddChildAttrib( _T("IsDeleted"), m_IsDeleted );
	GetXMLGen().AddChildAttrib( _T("IsArchived"), nArchiveID ? _T("1") : _T("0") );
	GetXMLGen().AddChildAttrib( _T("Type"), _T("outbound"));
	GetXMLGen().AddChildAttrib( _T("HTML"), nIsHTML );
	GetXMLGen().AddChildAttrib( _T("OutboundMsgTypeID"), m_OutboundMessageTypeID );
	GetXMLGen().AddChildAttrib( _T("OutboundMsgStateID"), m_OutboundMessageStateID );
	GetXMLGen().AddChildAttrib( _T("PriorityID"), m_PriorityID );
	GetXMLGen().AddChildAttrib( _T("SignatureID"), m_SignatureID );
	GetXMLGen().AddChildAttrib( _T("ReadReceipt"), m_ReadReceipt );
	GetXMLGen().AddChildAttrib( _T("DeliveryConfirmation"), m_DeliveryConfirmation );

	if ( m_OutboundMessageTypeID == EMS_OUTBOUND_MESSAGE_STATE_DRAFT )
	{
		GetXMLGen().AddChildAttrib( _T("DraftRouteToMe"), m_DraftRouteToMe );
		GetXMLGen().AddChildAttrib( _T("DraftCloseTicket"), m_DraftCloseTicket );
		GetXMLGen().AddChildAttrib( _T("MultiMail"), m_MultiMail );
	}

	AddAgentName( _T("ComposedBy"), m_AgentID, m_AgentID ? _T("Unknown") : _T("Auto-Responder") );
	
	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;

	if(lTzBias != 1)
	{
		if(ConvertToTimeZone(m_EmailDateTime,lTzBias,tsLocal))
		{
			GetDateTimeString( tsLocal, tsLocalLen, sDateTime );
		}
		else
		{
			GetDateTimeString( m_EmailDateTime, m_EmailDateTimeLen, sDateTime);
		}
	}
	else
	{
		GetDateTimeString( m_EmailDateTime, m_EmailDateTimeLen, sDateTime);
	}
	
	GetXMLGen().AddChildAttrib( _T("EmailDateTime"), sDateTime.c_str() );
	
	GetXMLGen().IntoElem();
	
	GetXMLGen().AddChildElem( _T("To"), m_EmailTo );
	GetXMLGen().AddChildElem( _T("ReplyTo"), m_EmailReplyToLen ? m_EmailReplyTo : m_EmailFrom );
	GetXMLGen().AddChildElem( _T("From"), m_EmailFrom );
	GetXMLGen().AddChildElem( _T("cc"), m_EmailCc );
	GetXMLGen().AddChildElem( _T("Bcc"), m_EmailBcc );
	
	if( bForEdit )
	{
		GetXMLGen().AddChildElem( _T("Subject"), m_Subject );
	}
	else
	{
		sSubject.assign( m_Subject );
		sSubject.EscapeHTML();
		GetXMLGen().AddChildElem( _T("Subject"), sSubject.c_str() );
	}

	GetXMLGen().AddChildElem( _T("messagebody") );
	
	bool bHasOrigMsg = false;
		
	if( bEscapeBody )
	{
		sBody.assign( m_Body );		
		sBody.EscapeBody( nIsHTML, m_OutboundMessageID, 0, nArchiveID, bForEdit );
		/*if( bForEdit )
			RemoveTicketIDFromBody(sBody);*/		

		GetXMLGen().SetChildData( sBody.c_str(), TRUE );
	}
	else
	{
		sBody.assign( m_Body );
		/*if( bForEdit )
			RemoveTicketIDFromBody(sBody);*/		
		
		GetXMLGen().SetChildData( sBody.c_str(), TRUE );
	}
	
	// list the attachments
	if ( !m_IsDeleted && !m_ArchiveID )
	{
		list<CAttachment>::iterator iter;
		
		for ( iter = m_AttachList.begin(); iter != m_AttachList.end(); iter++ )
		{
			iter->GenerateXML();
		}
	}
	
	// has this message been replied to?
	if( m_RepliedToWhenLen != SQL_NULL_DATA )
	{
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(m_RepliedToWhen,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDateTime );
			}
			else
			{
				GetDateTimeString( m_RepliedToWhen, m_RepliedToWhenLen, sDateTime);
			}
		}
		else
		{
			GetDateTimeString( m_RepliedToWhen, m_RepliedToWhenLen, sDateTime);
		}	
		GetXMLGen().AddChildElem( _T("RepliedTo"), sDateTime.c_str() );
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query for the OutboundMessage	              
\*--------------------------------------------------------------------------*/
void COutboundMessage::Query( void )
{
	if ( !m_OutboundMessageID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("An OutboundMessageID must be specified") );
	
	if ( TOutboundMessages::Query( GetQuery() ) != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("Outbound Message (%d) does not exist in the database"), m_OutboundMessageID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	// check security
	CTicket Ticket(m_ISAPIData, m_TicketID);
	Ticket.RequireRead();
	
	if ( m_IsDeleted )
	{
		PutStringProperty( tstring(_T("Message Has Been Deleted")) , &m_Body, &m_BodyAllocated );
	}
	else if ( m_ArchiveID )
	{
		TArchives ArchiveTable;
		ArchiveTable.m_ArchiveID = m_ArchiveID;
		
		ArchiveTable.Query( GetQuery() );
		
		CEMSString sString;
		sString.Format( _T("This message has been archived to: %s"), ArchiveTable.m_ArcFilePath );
		
		PutStringProperty(sString, &m_Body, &m_BodyAllocated );		
	}
	
	// has this message been replied to?
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );
	BINDCOL_TIME( GetQuery(), m_RepliedToWhen );
	GetQuery().Execute( _T("SELECT EmailDateTime FROM InboundMessages ")
		                _T("WHERE ReplyToMsgID=? and ReplyToIDIsInbound=0 ")
						_T("ORDER BY EmailDateTime Desc") );
	GetQuery().Fetch();

	// query the attachments
	CAttachment attachment(m_ISAPIData);
	attachment.ListOutboundMessageAttachments( m_OutboundMessageID, m_AttachList );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Delete the message	              
\*--------------------------------------------------------------------------*/
void COutboundMessage::Delete( void )
{
	if ( !m_OutboundMessageID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Unable to Delete Message: An Outbound Message ID must be specified") );
	
	try
	{
		// get the ticket ID
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), m_TicketID );
		BINDCOL_LONG_NOLEN( GetQuery(), m_OutboundMessageStateID );
		BINDCOL_LONG_NOLEN( GetQuery(), m_AgentID );
		BINDPARAM_LONG(GetQuery(), m_OutboundMessageID );
		GetQuery().Execute( _T("SELECT TicketID, OutboundMessageStateID, AgentID FROM OutboundMessages WHERE OutboundMessageID=?") );
		
		if ( GetQuery().Fetch() != S_OK)
		{
			CEMSString sError;
			sError.Format( _T("Outbound Message (%d) does not exist"), m_OutboundMessageID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		CTicket Ticket(m_ISAPIData, m_TicketID);
		Ticket.Query();

		// check security
		if ( m_OutboundMessageStateID == EMS_OUTBOUND_MESSAGE_STATE_DRAFT )
		{
			if ( m_AgentID != GetAgentID())
			{
				RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_DELETE_ACCESS );
			}
			else
			{
				RequireAgentRightLevel( EMS_OBJECT_TYPE_VOIP, 0, EMS_DELETE_ACCESS );
			}
		}
		else
		{
			Ticket.RequireDelete();
		}

		// lock the ticket
		Ticket.Lock(false);
		
		// if this is the only message in the ticket delete the ticket as well
		if ( Ticket.GetMsgCount() <= 1 )
		{
			Ticket.Delete(false);
		}
		else
		{
			TIMESTAMP_STRUCT time;
			GetTimeStamp(time);
				
			GetQuery().Initialize();
				
			// delete the message
			BINDPARAM_TIME_NOLEN( GetQuery(), time );
			BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );
			GetQuery().Execute(_T("UPDATE OutboundMessages SET IsDeleted=1, DeletedTime=? WHERE OutboundMessageID=?"));
				
			// delete entries in the OutboundMessageQueue
			GetQuery().Initialize();
			BINDPARAM_LONG(GetQuery(), m_OutboundMessageID);
			GetQuery().Execute(_T("DELETE FROM OutboundMessageQueue WHERE OutboundMessageID = ?"));
			
			// remove any entries in the approvals queue
			GetQuery().Reset(false);
			GetQuery().Execute( _T("DELETE FROM Approvals WHERE ApprovalObjectTypeID=1 AND ActualID=?") );

			// remove any outbound approval alerts regarding this message
			GetQuery().Reset(false);
			GetQuery().Execute( _T("DELETE FROM AlertMsgs WHERE AlertEventID IN (15,16,17) AND TicketID=?") );

			// Log it in system-generated ticket history
			THDelOutboundMsg( GetQuery(), m_TicketID, GetAgentID(), m_OutboundMessageID, Ticket.m_TicketBoxID );

			if ( _ttoi(Ticket.sAal.c_str()) == 1 && _ttoi(Ticket.sTmd.c_str()) == 1 )
			{
				Ticket.LogAgentAction(GetAgentID(),12,m_OutboundMessageID,m_TicketID,1,_T(""),_T(""));
			}
		}

		m_IsDeleted = EMS_DELETE_OPTION_DELETE_TO_WASTE_BASKET;
		Ticket.UnLock(true);
	}
	catch ( CEMSException Exception )
	{
		CEMSString sError;

		// if the exception wasn't because of an invalid id
		// query the ticket to get the subject...
		if ( Exception.GetErrorCode() != E_InvalidID )
		{
			TCHAR szSubject[OUTBOUNDMESSAGES_SUBJECT_LENGTH] = {0};
			long szSubjectLen = 0;

			try
			{
				// get the subject of the ticket
				GetQuery().Initialize();
				
				BINDCOL_TCHAR( GetQuery(), szSubject );
				BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );
				
				GetQuery().Execute( _T("SELECT Subject FROM OutboundMessages WHERE OutboundMessageID = ?") );
				GetQuery().Fetch();
			}
			catch(...) {}

			sError.Format( _T("Unable to delete message (%s):\n\nError: %s"), 
				szSubjectLen ? szSubject : _T("[No Subject]"), Exception.GetErrorString() );
		}
		// if the exception was because of an invalid id just use the id
		else
		{
			sError.Format( _T("Unable to delete message (%d):\n\nError: %s"), 
				           m_OutboundMessageID, Exception.GetErrorString() );
		}

		// rethrow the exception using the new error strin
		THROW_EMS_EXCEPTION_NOLOG( Exception.GetErrorCode(), sError );
	}	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Delete the message	              
\*--------------------------------------------------------------------------*/
void COutboundMessage::DeleteOrphan( void )
{
	if ( !m_OutboundMessageID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Unable to Delete Message: An Outbound Message ID must be specified") );
	
	try
	{
		// get the ticket ID
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), m_TicketID );
		BINDCOL_LONG_NOLEN( GetQuery(), m_OutboundMessageTypeID );
		BINDCOL_LONG_NOLEN( GetQuery(), m_AgentID );
		BINDPARAM_LONG(GetQuery(), m_OutboundMessageID );
		GetQuery().Execute( _T("SELECT TicketID, OutboundMessageTypeID, AgentID FROM OutboundMessages WHERE OutboundMessageID=?") );
		
		if ( GetQuery().Fetch() != S_OK)
		{
			CEMSString sError;
			sError.Format( _T("Outbound Message (%d) does not exist"), m_OutboundMessageID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}

		
		TIMESTAMP_STRUCT time;
		GetTimeStamp(time);
			
		GetQuery().Initialize();
		int nAgentID = GetAgentID();
		// delete the message
		BINDPARAM_TIME_NOLEN( GetQuery(), time );
		BINDPARAM_LONG( GetQuery(), nAgentID );
		BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );
		GetQuery().Execute(_T("UPDATE OutboundMessages SET IsDeleted=1, DeletedTime=?, DeletedBy=? WHERE OutboundMessageID=?"));
			
		// delete entries in the OutboundMessageQueue
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), m_OutboundMessageID);
		GetQuery().Execute(_T("DELETE FROM OutboundMessageQueue WHERE OutboundMessageID = ?"));
		
		// remove any entries in the approvals queue
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM Approvals WHERE ApprovalObjectTypeID=1 AND ActualID=?") );

		// remove any outbound approval alerts regarding this message
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM AlertMsgs WHERE AlertEventID IN (15,16,17) AND TicketID=?") );

		m_IsDeleted = EMS_DELETE_OPTION_DELETE_TO_WASTE_BASKET;

	}
	catch ( CEMSException Exception )
	{
		CEMSString sError;

		// if the exception wasn't because of an invalid id
		// query the ticket to get the subject...
		if ( Exception.GetErrorCode() != E_InvalidID )
		{
			TCHAR szSubject[OUTBOUNDMESSAGES_SUBJECT_LENGTH] = {0};
			long szSubjectLen = 0;

			try
			{
				// get the subject of the ticket
				GetQuery().Initialize();
				
				BINDCOL_TCHAR( GetQuery(), szSubject );
				BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );
				
				GetQuery().Execute( _T("SELECT Subject FROM OutboundMessages WHERE OutboundMessageID = ?") );
				GetQuery().Fetch();
			}
			catch(...) {}

			sError.Format( _T("Unable to delete message (%s):\n\nError: %s"), 
				szSubjectLen ? szSubject : _T("[No Subject]"), Exception.GetErrorString() );
		}
		// if the exception was because of an invalid id just use the id
		else
		{
			sError.Format( _T("Unable to delete message (%d):\n\nError: %s"), 
				           m_OutboundMessageID, Exception.GetErrorString() );
		}

		// rethrow the exception using the new error string
		THROW_EMS_EXCEPTION_NOLOG( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Reserves an outbound message ID.  	
\*--------------------------------------------------------------------------*/
int COutboundMessage::ReserveID( int nTicketID )
{
	m_TicketID = nTicketID;
	m_IsDeleted = EMS_DELETE_OPTION_RESERVED;

	Insert( GetQuery() );

	return m_OutboundMessageID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Sends the outbound message back to the drafts folder	              
\*--------------------------------------------------------------------------*/
void COutboundMessage::Revoke( void )
{
	try
	{
		if ( !m_OutboundMessageID )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("Invalid Message ID") );

		GetQuery().Initialize();

		BINDCOL_LONG( GetQuery(), m_TicketID );
		BINDCOL_LONG( GetQuery(), m_OutboundMessageStateID );
		BINDCOL_LONG( GetQuery(), m_AgentID );
		
		BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );

		GetQuery().Execute( _T("SELECT TicketID,OutboundMessageStateID,AgentID FROM OutboundMessages WHERE OutboundMessageID=?") );

		if ( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("The message no longer exists") );

		// check security
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_EDIT_ACCESS );
		
		// make sure that it hasn't already been sent
		if ( m_OutboundMessageStateID == EMS_OUTBOUND_MESSAGE_STATE_SENT || 
			 m_OutboundMessageStateID == EMS_OUTBOUND_MESSAGE_STATE_INPROGRESS )
		{
			THROW_EMS_EXCEPTION( E_MsgAlreadySent, _T("The message has already been sent") );
		}
		
		CTicket Ticket(m_ISAPIData, m_TicketID);

		// remove any entries in the outbound message queue
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );
		GetQuery().Execute( _T("DELETE FROM OutboundMessageQueue WHERE OutboundMessageID=?") ); 
		
		// change the state of the message to draft
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE OutboundMessages SET OutboundMessageStateID=1 WHERE OutboundMessageID=?") );

		// remove any entries in the approvals queue
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM Approvals WHERE ApprovalObjectTypeID=1 AND ActualID=?") );

		// remove any outbound approval alerts regarding this message
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM AlertMsgs WHERE AlertEventID IN (15,16,17) AND TicketID=?") );

		// log entry in ticket history and agent activity
		THRevokeOutboundMsg( GetQuery(), m_TicketID, GetAgentID(), m_OutboundMessageID );

		if ( _ttoi(Ticket.sAal.c_str()) == 1 && _ttoi(Ticket.sTmd.c_str()) == 1 )
		{
			Ticket.LogAgentAction(GetAgentID(),20,m_OutboundMessageID,m_TicketID,1,_T(""),_T(""));
		}

	}
	catch ( CEMSException Exception )
	{
		CEMSString sError;

		// if the exception wasn't because of an invalid id
		// query the ticket to get the subject...
		if ( Exception.GetErrorCode() != E_InvalidID )
		{
			TCHAR szSubject[OUTBOUNDMESSAGES_SUBJECT_LENGTH] = {0};
			long szSubjectLen = 0;

			try
			{
				// get the subject of the ticket
				GetQuery().Initialize();
				
				BINDCOL_TCHAR( GetQuery(), szSubject );
				BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );
				
				GetQuery().Execute( _T("SELECT Subject FROM OutboundMessages WHERE OutboundMessageID = ?") );
				GetQuery().Fetch();
			}
			catch(...) {}

			sError.Format( _T("Unable to revoke message (%s):\n\nError: %s"), 
				szSubjectLen ? szSubject : _T("[No Subject]"), Exception.GetErrorString() );
		}
		// if the exception was because of an invalid id
		// just use the id
		else
		{
			sError.Format( _T("Unable to revoke message (%d):\n\nError: %s"), 
				           m_OutboundMessageID, Exception.GetErrorString() );
		}

		// rethrow the exception using the new error string
		THROW_EMS_EXCEPTION_NOLOG( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Releases the message for delivery	              
\*--------------------------------------------------------------------------*/
void COutboundMessage::Release( void )
{
	tstring sNote;
	
	try
	{
		if ( !m_OutboundMessageID )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("Invalid Message ID") );

		GetQuery().Initialize();

		BINDCOL_LONG( GetQuery(), m_TicketID );
		BINDCOL_LONG( GetQuery(), m_OutboundMessageStateID );
		BINDCOL_LONG( GetQuery(), m_AgentID );
		
		BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );

		GetQuery().Execute( _T("SELECT TicketID,OutboundMessageStateID,AgentID FROM OutboundMessages WHERE OutboundMessageID=?") );

		if ( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("The message no longer exists") );

		// make sure that it hasn't already been sent
		if ( m_OutboundMessageStateID == EMS_OUTBOUND_MESSAGE_STATE_SENT || 
			 m_OutboundMessageStateID == EMS_OUTBOUND_MESSAGE_STATE_INPROGRESS )
		{
			THROW_EMS_EXCEPTION( E_MsgAlreadySent, _T("The message has already been sent") );
		}
		
		CTicket Ticket(m_ISAPIData, m_TicketID);

		GetISAPIData().GetXMLString( _T("NOTE"), sNote, true );
	
		// set the outbound queue to approved
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE OutboundMessageQueue SET IsApproved=1 WHERE OutboundMessageID=?") );

		// change the state of the message to untouched
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE OutboundMessages SET OutboundMessageStateID=3 WHERE OutboundMessageID=?") );

		// remove any entries in the approvals queue
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM Approvals WHERE ApprovalObjectTypeID=1 AND ActualID=?") );

		// remove any outbound approval required alerts regarding this message
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM AlertMsgs WHERE AlertEventID=15 AND TicketID=?") );

		// send outbound approved alerts
		CheckAgentAlerts(m_AgentID, 16);		

		// log entry in ticket history and agent activity
		THReleaseOutboundMsg( GetQuery(), m_TicketID, GetAgentID(), m_OutboundMessageID );

		if ( _ttoi(Ticket.sAal.c_str()) == 1 && _ttoi(Ticket.sTmd.c_str()) == 1 )
		{
			Ticket.LogAgentAction(GetAgentID(),21,m_OutboundMessageID,m_TicketID,1,_T(""),_T(""));
		}

		if ( sNote.length() )
		{
			TTicketNotes Note;
			PutStringProperty( sNote, &Note.m_Note, &Note.m_NoteAllocated );
			Note.m_TicketID = m_TicketID;
			Note.m_AgentID = GetAgentID();
			GetTimeStamp( Note.m_DateCreated );
			Note.m_DateCreatedLen = sizeof(TIMESTAMP_STRUCT);
			Note.m_StopTimeLen = SQL_NULL_DATA;
			Note.m_StopTimeLen = SQL_NULL_DATA;
			Note.Insert( GetQuery() );
		
			// Log it in system-generated ticket history
			THAddNote( GetQuery(), m_TicketID, GetAgentID(), Note.m_TicketNoteID );
			// Mark the note we just added as read			
			CTicketNotes::UpdateTHNoteRead(GetQuery(), Note.m_AgentID, Note.m_TicketID, 1, Note.m_TicketNoteID);			
		}
		
	}
	catch ( CEMSException Exception )
	{
		CEMSString sError;

		// if the exception wasn't because of an invalid id
		// query the ticket to get the subject...
		if ( Exception.GetErrorCode() != E_InvalidID )
		{
			TCHAR szSubject[OUTBOUNDMESSAGES_SUBJECT_LENGTH] = {0};
			long szSubjectLen = 0;

			try
			{
				// get the subject of the ticket
				GetQuery().Initialize();
				
				BINDCOL_TCHAR( GetQuery(), szSubject );
				BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );
				
				GetQuery().Execute( _T("SELECT Subject FROM OutboundMessages WHERE OutboundMessageID = ?") );
				GetQuery().Fetch();
			}
			catch(...) {}

			sError.Format( _T("Unable to release message (%s):\n\nError: %s"), 
				szSubjectLen ? szSubject : _T("[No Subject]"), Exception.GetErrorString() );
		}
		// if the exception was because of an invalid id
		// just use the id
		else
		{
			sError.Format( _T("Unable to release message (%d):\n\nError: %s"), 
				           m_OutboundMessageID, Exception.GetErrorString() );
		}

		// rethrow the exception using the new error string
		THROW_EMS_EXCEPTION_NOLOG( Exception.GetErrorCode(), sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the outbound message to the sending Agent drafts folder	              
\*--------------------------------------------------------------------------*/
void COutboundMessage::Return( void )
{
	tstring sNote;
	
	try
	{
		if ( !m_OutboundMessageID )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("Invalid Message ID") );

		GetQuery().Initialize();

		BINDCOL_LONG( GetQuery(), m_TicketID );
		BINDCOL_LONG( GetQuery(), m_OutboundMessageStateID );
		BINDCOL_LONG( GetQuery(), m_AgentID );
		
		BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );

		GetQuery().Execute( _T("SELECT TicketID,OutboundMessageStateID,AgentID FROM OutboundMessages WHERE OutboundMessageID=?") );

		if ( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("The message no longer exists") );

		// make sure that it hasn't already been sent
		if ( m_OutboundMessageStateID == EMS_OUTBOUND_MESSAGE_STATE_SENT || 
			 m_OutboundMessageStateID == EMS_OUTBOUND_MESSAGE_STATE_INPROGRESS )
		{
			THROW_EMS_EXCEPTION( E_MsgAlreadySent, _T("The message has already been sent") );
		}
		
		// make sure that it hasn't already been returned
		if ( m_OutboundMessageStateID == EMS_OUTBOUND_MESSAGE_STATE_DRAFT  )
		{
			THROW_EMS_EXCEPTION( E_MsgAlreadySent, _T("The message has already been returned") );
		}

		CTicket Ticket(m_ISAPIData, m_TicketID);

		GetISAPIData().GetXMLString( _T("NOTE"), sNote, true );	

		// remove from the outboundmessagequeue
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM OutboundMessageQueue WHERE OutboundMessageID=?") );

		// change the state of the message to draft
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE OutboundMessages SET OutboundMessageStateID=1 WHERE OutboundMessageID=?") );

		// remove any entries in the approvals queue
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM Approvals WHERE ApprovalObjectTypeID=1 AND ActualID=?") );

		// remove any outbound approval required alerts regarding this message
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM AlertMsgs WHERE AlertEventID=15 AND TicketID=?") );

		// send outbound returned alerts
		CheckAgentAlerts(m_AgentID, 17);		

		// log entry in ticket history and agent activity
		THReturnOutboundMsg( GetQuery(), m_TicketID, GetAgentID(), m_OutboundMessageID );

		if ( _ttoi(Ticket.sAal.c_str()) == 1 && _ttoi(Ticket.sTmd.c_str()) == 1 )
		{
			Ticket.LogAgentAction(GetAgentID(),22,m_OutboundMessageID,m_TicketID,1,_T(""),_T(""));
		}

		if ( sNote.length() )
		{
			TTicketNotes Note;
			PutStringProperty( sNote, &Note.m_Note, &Note.m_NoteAllocated );
			Note.m_TicketID = m_TicketID;
			Note.m_AgentID = GetAgentID();
			GetTimeStamp( Note.m_DateCreated );
			Note.m_DateCreatedLen = sizeof(TIMESTAMP_STRUCT);
			Note.m_StopTimeLen = SQL_NULL_DATA;
			Note.m_StopTimeLen = SQL_NULL_DATA;
			Note.Insert( GetQuery() );
		
			// Log it in system-generated ticket history
			THAddNote( GetQuery(), m_TicketID, GetAgentID(), Note.m_TicketNoteID );
			// Mark the note we just added as read			
			CTicketNotes::UpdateTHNoteRead(GetQuery(), Note.m_AgentID, Note.m_TicketID, 1, Note.m_TicketNoteID);			
		}
	}
	catch ( CEMSException Exception )
	{
		CEMSString sError;

		// if the exception wasn't because of an invalid id
		// query the ticket to get the subject...
		if ( Exception.GetErrorCode() != E_InvalidID )
		{
			TCHAR szSubject[OUTBOUNDMESSAGES_SUBJECT_LENGTH] = {0};
			long szSubjectLen = 0;

			try
			{
				// get the subject of the ticket
				GetQuery().Initialize();
				
				BINDCOL_TCHAR( GetQuery(), szSubject );
				BINDPARAM_LONG( GetQuery(), m_OutboundMessageID );
				
				GetQuery().Execute( _T("SELECT Subject FROM OutboundMessages WHERE OutboundMessageID = ?") );
				GetQuery().Fetch();
			}
			catch(...) {}

			sError.Format( _T("Unable to release message (%s):\n\nError: %s"), 
				szSubjectLen ? szSubject : _T("[No Subject]"), Exception.GetErrorString() );
		}
		// if the exception was because of an invalid id
		// just use the id
		else
		{
			sError.Format( _T("Unable to release message (%d):\n\nError: %s"), 
				           m_OutboundMessageID, Exception.GetErrorString() );
		}

		// rethrow the exception using the new error string
		THROW_EMS_EXCEPTION_NOLOG( Exception.GetErrorCode(), sError );
	}
}

void COutboundMessage::CheckAgentAlerts( int nAgentID, int nAlertEventID )
{
	TAlerts alert;
	std::list<TAlerts> aeidList;
	std::list<TAlerts>::iterator aeidIter;
	bool bAlertAssign=false;
	dca::String t;
	
	GetQuery().Initialize();	
	BINDPARAM_LONG( GetQuery(), nAgentID );
	BINDPARAM_LONG( GetQuery(), nAlertEventID );
	BINDCOL_LONG( GetQuery(), alert.m_AlertID );
	BINDCOL_LONG( GetQuery(), alert.m_AlertEventID );
	BINDCOL_LONG( GetQuery(), alert.m_AlertMethodID );
	BINDCOL_TCHAR( GetQuery(), alert.m_EmailAddress );
	GetQuery().Execute( _T("SELECT AlertID, AlertEventID, AlertMethodID, EmailAddress FROM Alerts WHERE AgentID=? AND AlertEventID=? ") );
	while(GetQuery().Fetch() == S_OK)
	{
		/*dca::String t2(alert.m_EmailAddress);
		t.Format("COutboundMessage::CheckAgentAlerts - Agent [%d] configured for AlertEventID [%d] AlertMethodID [%d] EmailAddress [%s]", nAgentID, alert.m_AlertEventID,alert.m_AlertMethodID,t2.c_str() );
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ISAPI);*/
		aeidList.push_back(alert);
	}
	
	for(aeidIter = aeidList.begin(); aeidIter != aeidList.end(); aeidIter++)
	{
		CEMSString strAlertBody;
		tstring sName;
		GetAgentName(GetAgentID(), sName);
		if(nAlertEventID==16)
		{
			strAlertBody.Format( _T("Outbound Message %d approved by Agent %s |||An Outbound message you sent has been approved by Agent %s.\n\nOutbound Message #%d in Ticket #%d approved by Agent %s\r\n"), m_OutboundMessageID, sName.c_str(), sName.c_str(), m_OutboundMessageID, m_TicketID, sName.c_str() );						
			SendOutboundApprovedAlert( aeidIter->m_AlertID, strAlertBody.c_str() );				
		}
		else if(nAlertEventID==17)
		{
			strAlertBody.Format( _T("Outbound Message %d returned by Agent %s |||An Outbound message you sent has been returned by Agent %s.\n\nOutbound Message #%d in Ticket #%d returned by Agent %s\r\n"), m_OutboundMessageID, sName.c_str(), sName.c_str(), m_OutboundMessageID, m_TicketID, sName.c_str() );						
			SendOutboundReturnedAlert( aeidIter->m_AlertID, strAlertBody.c_str() );		
		}
	}
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Populates the OutboundMessaage Type for the message.	              
\*--------------------------------------------------------------------------*/
void COutboundMessage::GetOutboundMessageType( int nOutboundMsgID )
{
	if ( !m_OutboundMessageID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("An Outbound Message ID must be specified") );

	GetQuery().Initialize();

	BINDCOL_LONG( GetQuery(), m_OutboundMessageTypeID );
	BINDPARAM_LONG( GetQuery(), nOutboundMsgID );

	GetQuery().Execute( _T("SELECT OutboundMessageTypeID FROM OutboundMessages WHERE OutboundMessageID=?" ) );

	if ( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("The source outbound message (%d) does not exist in the database"), m_OutboundMessageID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Populates the ReplyTo ID and type for the message.	              
\*--------------------------------------------------------------------------*/
void COutboundMessage::GetReplyToIDAndType( int nOutboundMsgID )
{
	if ( !m_OutboundMessageID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("An Outbound Message ID must be specified") );

	GetQuery().Initialize();

	BINDCOL_LONG( GetQuery(), m_ReplyToMsgID );
	BINDCOL_BIT( GetQuery(), m_ReplyToIDIsInbound );
	BINDPARAM_LONG( GetQuery(), nOutboundMsgID );

	GetQuery().Execute( _T("SELECT ReplyToMsgID,ReplyToIDIsInbound FROM OutboundMessages WHERE OutboundMessageID=?" ) );

	if ( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("The source outbound message (%d) does not exist in the database"), m_OutboundMessageID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
}

void COutboundMessage::LogAgentAction( long nAgentID,long nActionID,long nId1,long nId2,long nId3,CEMSString sData1,CEMSString sData2 )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nAgentID );
	BINDPARAM_LONG( GetQuery(), nActionID );
	BINDPARAM_LONG( GetQuery(), nId1 );
	BINDPARAM_LONG( GetQuery(), nId2 );
	BINDPARAM_LONG( GetQuery(), nId3 );
	BINDPARAM_TCHAR_STRING( GetQuery(), sData1 );
	BINDPARAM_TCHAR_STRING( GetQuery(), sData2 );
	GetQuery().Execute(_T("INSERT INTO AgentActivityLog (AgentID,ActivityID,ID1,ID2,ID3,Data1,Data2) VALUES (?,?,?,?,?,?,?)"));	
}