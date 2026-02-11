// OutboundMsg.cpp: implementation of the COutboundMsg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutboundMsg.h"
#include "EMSMutex.h"
#include "MessageIO.h"
#include "AttachFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COutboundMsg::COutboundMsg(CODBCQuery& m_query ) : m_query(m_query)
{

}

COutboundMsg::~COutboundMsg()
{

}


////////////////////////////////////////////////////////////////////////////////
// 
// PutInOutboundQueue
// 
////////////////////////////////////////////////////////////////////////////////
int COutboundMsg::PutInOutboundQueue( void )
{
	TIMESTAMP_STRUCT now;
	m_query.Initialize();

	GetTimeStamp( now );

	// make the message a part of the ticket

	BINDPARAM_LONG( m_query, m_OutboundMessageID );
	BINDPARAM_TIME_NOLEN( m_query, now );
	
	m_query.Execute( L"INSERT INTO OutboundMessageQueue "
				   L"(OutboundMessageID,DateSpooled,IsApproved) "
				   L"VALUES "
				   L"(?,?,1)" );

	return m_query.GetRowCount();	
}

////////////////////////////////////////////////////////////////////////////////
// 
// UnDeleteMsg
// 
////////////////////////////////////////////////////////////////////////////////
int COutboundMsg::UnDeleteMsg( void )
{
	m_query.Initialize();

	// make the message a part of the ticket

	BINDPARAM_LONG( m_query, m_OutboundMessageID );
	
	m_query.Execute( L"UPDATE OutboundMessages "
				     L"SET IsDeleted=0 "
				     L"WHERE OutboundMessageID=?");

	return m_query.GetRowCount();	
}


////////////////////////////////////////////////////////////////////////////////
// 
// Forward - Caller must set EmailFrom before calling!
// 
////////////////////////////////////////////////////////////////////////////////
void COutboundMsg::Forward( CInboundMsg& msg, 
						    vector<CForwardToAddr> fta,
							vector<CForwardCCAddr> fca, bool bInTicket, bool bIsRemoteReply, bool bAllowRemoteReply, bool bQuoteOriginal, unsigned int nOmitTracking )
{
	DebugReporter::Instance().DisplayMessage("COutboundMsg::Forward - Entering", DebugReporter::ENGINE);
	
	tstring address;
	vector<CForwardToAddr>::iterator ftaIter;
	vector<CForwardCCAddr>::iterator fcaIter;

	// Forward a copy of this message

	if( !bIsRemoteReply )	
		m_AgentID = 0;		// System is sending this message
	
	GetTimeStamp( m_EmailDateTime );
	m_EmailDateTimeLen = sizeof( TIMESTAMP_STRUCT );
	m_TicketID =  ( bInTicket ) ? msg.m_TicketID : 0;	
	wcscpy( m_EmailPrimaryTo, fta[0].m_EmailAddress );

	if( !bIsRemoteReply )	
		m_ReplyToMsgID = msg.m_InboundMessageID;

	m_ReplyToIDIsInbound = 1;
	m_TicketCategoryID = msg.m_OriginalTicketCategoryID;

	wcscpy( m_MediaType, msg.m_MediaType );
	wcscpy( m_MediaSubType, msg.m_MediaSubType );
	
	if( !bIsRemoteReply )
	{
		// Put the Fw: in front of the subject, if there isn't one already
		if( wcsnicmp( msg.m_Subject, L"FW:", 3) == 0 )
		{
			wcscpy( m_Subject, msg.m_Subject );
		}
		else
		{
			if( _snwprintf( m_Subject, OUTBOUNDMESSAGES_SUBJECT_LENGTH, 
							L"FW: %s", msg.m_Subject ) < 0 )
			{
				// We overran the buffer
				m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-1] = _T('\0');
				m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-2] = _T('.');
				m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-3] = _T('.');
				m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-4] = _T('.');
			}
		}
	}

	m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_UNTOUCHED;
	m_OutboundMessageTypeID = EMS_OUTBOUND_MESSAGE_TYPE_FORWARD;
	m_IsDeleted = 2;
	
	if(bQuoteOriginal)
	{
		CEMSString sQuotedHdr;
		CEMSString sRecvd;
		tstring sBody;
		CEMSString sTo;
		CEMSString sCc;
		CEMSString sStdRespBody;
		CEMSString sFrom, sSubject;
		tstring::size_type posTagOpen;
		tstring::size_type posTagClose;
		bool bIsOldSR = true;
		
		// format the FROM: address
		if ( wcslen( msg.m_EmailFromName ) > 0)
		{
			sFrom.Format( L"\"%s\" <%s>", msg.m_EmailFromName, msg.m_EmailFrom );
		}
		else
		{
			sFrom.assign( msg.m_EmailFrom );
		}

		sTo.assign( msg.m_EmailTo );
		sCc.assign( msg.m_EmailCc );

		if( _tcsicmp( msg.m_MediaSubType, _T("html") ) == 0 )
		{
			tstring::size_type pos;
			tstring sStyle = L"<style>"
								L"#replyquoted {border-left:2px solid blue;margin-left:5px;padding-left:5px;padding-top:10px;padding-bottom:10px;}"
								L"#replyunquoted {border-left:0px;margin-left:0px;padding-left:0px;padding-top:0px;padding-bottom:0px;z-index: 10;color: blue;}"
							L"</style>";

			tstring sOpenDiv = L"<DIV id=replyquoted>";
			tstring sCloseDiv = L"</DIV>";

			sBody = msg.m_Body;

			pos = 0;

			FindOrInsert( pos, L"<HTML>", sBody );
			FindOrInsert( pos, L"<HEAD>", sBody );

			sBody.insert( pos, sStyle );
			pos += sStyle.length();

			FindOrInsert( pos, L"</HEAD>", sBody );			

			sFrom.EscapeHTML();

			sSubject.assign( msg.m_Subject );
			sSubject.EscapeHTML();
			
			sBody.insert( pos, sOpenDiv.c_str() );
			pos += sOpenDiv.length();

			GetDateTimeString( msg.m_DateReceived, msg.m_DateReceivedLen, sRecvd );

			sTo.EscapeHTML();
			sCc.EscapeHTML();

			if(msg.m_EmailCcLen > 0)
			{
				sQuotedHdr.Format( _T("-----Original Message-----<br/>From: %s<br/>Received: %s<br/>")
								_T("To: %s<br/>Cc: %s<br/>Subject: %s<br/><br/>"), sFrom.c_str(), sRecvd.c_str(), 
								sTo.c_str(), sCc.c_str(), sSubject.c_str()  );
			}
			else
			{
				sQuotedHdr.Format( _T("-----Original Message-----<br/>From: %s<br/>Received: %s<br/>")
								_T("To: %s<br/>Subject: %s<br/><br/>"), sFrom.c_str(), 
								sRecvd.c_str(), sTo.c_str(), sSubject.c_str()  );
			}			

			sBody.insert( pos, sQuotedHdr.c_str() );
			pos += sQuotedHdr.length();

			FindOrInsert( pos, L"</HTML>", sBody, true );

			sBody.insert( pos - 7, sCloseDiv.c_str() );
			pos += sCloseDiv.length();
		}
		else
		{
			// Text message
			
			GetDateTimeString( msg.m_DateReceived, msg.m_DateReceivedLen, sRecvd );

			if(msg.m_EmailCcLen > 0)
			{
				sQuotedHdr.Format( _T("\r\n> -----Original Message-----\r\n> From: %s\r\n> Received: %s\r\n")
								_T("> To: %s\r\n> Cc: %s\r\n> Subject: %s\r\n>\r\n"), sFrom.c_str(), 
								sRecvd.c_str(), sTo.c_str(), sCc.c_str(), msg.m_Subject  );				
			}
			else
			{
				sQuotedHdr.Format( _T("\r\n> -----Original Message-----\r\n> From: %s\r\n> Received: %s\r\n")
								_T("> To: %s\r\n> Subject: %s\r\n>\r\n"), sFrom.c_str(), 
								sRecvd.c_str(), sTo.c_str(), 
								msg.m_Subject  );
			}			

			sBody.append( sQuotedHdr );
			sStdRespBody = msg.m_Body;
			sStdRespBody.PrefixBody( _T("> ") );
			sBody.append( sStdRespBody.c_str() );
		}

		// Set the body in the TClass
		PutStringPropertyW( (TCHAR*)sBody.c_str(), &m_Body, &m_BodyAllocated );

	}
	else
	{
		PutStringPropertyW( msg.m_Body, &m_Body, &m_BodyAllocated );
	}	

	// Build list of to: address fields
	ftaIter = fta.begin();
	address = ftaIter->m_EmailAddress;
	ftaIter++;
	while ( ftaIter != fta.end() )
	{
		address += (tstring)_T("; ");
		address += ftaIter->m_EmailAddress;
		ftaIter++;
	}
	PutStringPropertyW( (TCHAR*)address.c_str(), &m_EmailTo, &m_EmailToAllocated );
	
	if( fca.size() > 0 )
	{
		// Build list of cc: address fields
		fcaIter = fca.begin();
		address = fcaIter->m_EmailAddress;
		fcaIter++;
		while ( fcaIter != fca.end() )
		{
			address += (tstring)_T("; ");
			address += fcaIter->m_EmailAddress;
			fcaIter++;
		}
		PutStringPropertyW( (TCHAR*)address.c_str(), &m_EmailCc, &m_EmailCcAllocated );
	}
	
	if( wcslen( m_EmailReplyTo ) == 0 )
	{
		// Copy the from address into the replyto field
		PutStringPropertyW( m_EmailFrom, &m_EmailReplyTo, &m_EmailReplyToAllocated );
	}

	Insert( m_query );

	PutInOutboundQueue();

	if ( bInTicket || bAllowRemoteReply )
	{
		int nTicketTracking = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_TICKET_TRACKING ) );	

		int nTicketID = 0;

		if ( bInTicket )
		{
			nTicketID = m_TicketID;
		}
		else
		{
			nTicketID = msg.m_TicketID;
		}
				
		if(!nOmitTracking)
		{
			PutTicketIDInSubject( nTicketID, m_OutboundMessageID, m_Subject, nTicketTracking );

			m_query.Initialize();
			BINDPARAM_WCHAR( m_query, m_Subject );
			BINDPARAM_LONG( m_query, m_OutboundMessageID );
			m_query.Execute( L"UPDATE OutboundMessages "
							L"SET Subject=? "
							L"WHERE OutboundMessageID=?" );
		}
	}
	
	ForwardAttachments( msg );

	// Commmit the message
	UnDeleteMsg();

	DebugReporter::Instance().DisplayMessage("COutboundMsg::Forward - Leaving", DebugReporter::ENGINE);
}

////////////////////////////////////////////////////////////////////////////////
// 
// ForwardAttachments
// 
////////////////////////////////////////////////////////////////////////////////
int COutboundMsg::ForwardAttachments( CInboundMsg& msg )
{
	list<int> AttachList;
	list<int>::iterator iter;
	int nAttachID;
	int nVirusScanStateID;

	// Forward attachments
	m_query.Initialize();
	BINDPARAM_LONG( m_query, msg.m_InboundMessageID  );
	BINDCOL_LONG_NOLEN( m_query, nAttachID );
	BINDCOL_LONG_NOLEN( m_query, nVirusScanStateID );

	m_query.Execute( L"SELECT Attachments.AttachmentID,VirusScanStateID "
					 L"FROM Attachments "
					 L"INNER JOIN InboundMessageAttachments "
					 L"ON Attachments.AttachmentID = InboundMessageAttachments.AttachmentID "
					 L"WHERE InboundMessageID=?" );
	
	while( m_query.Fetch() == S_OK )
	{
		if( !EMS_IS_INFECTED( nVirusScanStateID ) )
		{
			AttachList.push_back( nAttachID );
		}
	}

	for( iter = AttachList.begin(); iter != AttachList.end(); iter++ )
	{	
		// We copied the file, now create the database record
		m_query.Initialize();
		BINDPARAM_LONG( m_query, m_OutboundMessageID  );
		BINDPARAM_LONG( m_query, *iter );

		m_query.Execute( L"INSERT INTO OutboundMessageAttachments "
						 L"(OutboundMessageID,AttachmentID) "
						 L"VALUES "
						 L"(?,?)" );
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// AutoReply
// 
////////////////////////////////////////////////////////////////////////////////
bool COutboundMsg::AutoReply( int StdRespID, int AutoReplyQuoteMsg, 
							 CInboundMsg& msg, TCHAR* szAutoReplyFrom, bool bInTicket, unsigned int nOmitTracking )
{
	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - entering", DebugReporter::ENGINE);
		
	//Check to see if recipient or recipient domain is in Auto Reply Bypass
	tstring sTemp((wcslen( msg.m_EmailReplyTo ) > 0) ? msg.m_EmailReplyTo : msg.m_EmailFrom);

	if(sTemp.size() < 5)
	{
		return true;
	}

	if(sTemp.at(0) == '<')
	{
		sTemp = sTemp.erase(0,1);						
	}
	if(sTemp.at(sTemp.size()-1) == '>')
	{
		sTemp = sTemp.erase(sTemp.size()-1,1);						
	}
	tstring::size_type pos = sTemp.find_last_of( _T("@") );
	tstring sDomain = sTemp.substr(pos+1,sTemp.length());
	int nEmailID = 0;
	m_query.Initialize();
	BINDPARAM_TCHAR_STRING( m_query, sTemp );
	BINDPARAM_TCHAR_STRING( m_query, sDomain );
	BINDCOL_LONG_NOLEN( m_query, nEmailID );
	m_query.Execute( L"SELECT TOP 1 EmailID FROM Email WHERE (EmailValue = ? AND EmailTypeID = 2) OR (EmailValue = ? AND EmailTypeID = 3)" );
	m_query.Fetch();

	if(nEmailID > 0)
	{
		return true;
	}

	if(nOmitTracking == 0)
	{
		//Check to see if recipient or recipient domain is in Ticket Tracking Bypass
		nEmailID = 0;
		m_query.Initialize();
		BINDPARAM_TCHAR_STRING( m_query, sTemp );
		BINDPARAM_TCHAR_STRING( m_query, sDomain );
		BINDCOL_LONG_NOLEN( m_query, nEmailID );
		m_query.Execute( L"SELECT TOP 1 EmailID FROM Email WHERE (EmailValue = ? AND EmailTypeID = 0) OR (EmailValue = ? AND EmailTypeID = 1)" );
		m_query.Fetch();

		if(nEmailID > 0)
		{
			nOmitTracking = 1;
		}
	}

	m_AgentID = 0;
	GetTimeStamp( m_EmailDateTime );
	m_EmailDateTimeLen = sizeof( TIMESTAMP_STRUCT );
	m_TicketID =  ( bInTicket ) ? msg.m_TicketID : 0;
	m_ReplyToMsgID = msg.m_InboundMessageID;
	m_ReplyToIDIsInbound = 1;
	m_TicketCategoryID = msg.m_OriginalTicketCategoryID;
	
	wcscpy( m_MediaType, msg.m_MediaType );
	wcscpy( m_MediaSubType, msg.m_MediaSubType );

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - put RE in Subject", DebugReporter::ENGINE);

	// Put the Re: in front of the subject, if there isn't one already
	if( wcsnicmp( msg.m_Subject, L"RE:", 3) == 0 )
	{
		wcscpy( m_Subject, msg.m_Subject );
	}
	else
	{
		if( _snwprintf( m_Subject, OUTBOUNDMESSAGES_SUBJECT_LENGTH, 
					L"RE: %s", msg.m_Subject ) < 0 )
		{
			// We overran the buffer
			m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-1] = _T('\0');
			m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-2] = _T('.');
			m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-3] = _T('.');
			m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH-4] = _T('.');
		}
	}

	m_OutboundMessageTypeID = EMS_OUTBOUND_MESSAGE_TYPE_REPLY;
	m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_UNTOUCHED;
	m_IsDeleted = 2;
	
	if ( StdRespID > 0 )
	{
		DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - insert SR body", DebugReporter::ENGINE);
		InsertStdRespBody( StdRespID, AutoReplyQuoteMsg, msg );
	}

	if( wcslen( szAutoReplyFrom ) > 0 )
	{
		wcscpy( m_EmailFrom, szAutoReplyFrom );
	}
	else
	{
		wcscpy( m_EmailFrom, _T("\"MailFlow Mail Subsystem\" <EmailSystem@MailFlow>") );
	}
	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - set the primary To: address", DebugReporter::ENGINE);
	if( wcslen( msg.m_EmailReplyTo ) > 0 )
	{
		// Set the Primary To: Address
		wcsncpy( m_EmailPrimaryTo, msg.m_EmailReplyTo, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH );

		// And the To: address
		PutStringPropertyW( msg.m_EmailReplyTo, &m_EmailTo, &m_EmailToAllocated );
	}
	else
	{
		// Set the Primary To: Address
		if( wcslen(msg.m_EmailFromName) > 0 )
		{
			_snwprintf( m_EmailPrimaryTo, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
						L"\"%s\" <%s>", msg.m_EmailFromName, msg.m_EmailFrom );
		}
		else
		{
			_snwprintf( m_EmailPrimaryTo, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
						L"<%s>", msg.m_EmailFrom );
		}

		// And the To: address
		PutStringPropertyW( m_EmailPrimaryTo, &m_EmailTo, &m_EmailToAllocated );
	}

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - copy EmailFrom -> EmailReplyTo", DebugReporter::ENGINE);
	// Copy the EmailFrom -> EmailReplyTo
	PutStringPropertyW( m_EmailFrom, &m_EmailReplyTo, &m_EmailReplyToAllocated );

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - insert the msg record", DebugReporter::ENGINE);
	// Create the message record
	try
	{
		Insert( m_query );
	}
	catch(ODBCError_t error) 
	{ 
		try
		{
			dca::String er;
			dca::String x(error.szErrMsg);
			er.Format("COutboundMsg::AutoReply - %s", x.c_str());
			DebugReporter::Instance().DisplayMessage(er.c_str(), DebugReporter::ENGINE);
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - ODBC Error", DebugReporter::ENGINE);
		}
		return false;
	}	
	catch( ... )
	{
		DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - Unhandled Exception", DebugReporter::ENGINE);
		return false;
	}	

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - add SR attachments", DebugReporter::ENGINE);
	
	// Attempt to add attachments
	if ( StdRespID > 0 )
	{
		AddStdRespAttachments( StdRespID );
	}

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - attach inline images", DebugReporter::ENGINE);
	
	// Attach inline images referenced by the message body
	if( AutoReplyQuoteMsg )
	{
		AttachInlineImages( m_query, msg.m_Body, msg.m_InboundMessageID, 
							true, m_OutboundMessageID );
	}

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - attach sig images", DebugReporter::ENGINE);
	
	// Attach inline images referenced by the standard response
	AttachSigImages(m_query, m_Body, m_OutboundMessageID);

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - put in outbound queue", DebugReporter::ENGINE);
	
	PutInOutboundQueue();

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - insert outbound message contacts", DebugReporter::ENGINE);
	
	// Put entry in OutboundMessagesContacts
	m_query.Initialize();
	BINDPARAM_LONG( m_query, m_OutboundMessageID );
	BINDPARAM_LONG( m_query, msg.m_ContactID );
	m_query.Execute( L"INSERT INTO OutboundMessageContacts "
						L"(OutboundMessageID,ContactID) "
						L"VALUES "							
						L"(?,?)" );

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - CheckForVariables", DebugReporter::ENGINE);
	//Check for any variables in the subject
	CheckForVariables();
	
// Note that we have to insert first to get the outboundmessageID
	// before we can format the subject
	int nTicketTracking = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_TICKET_TRACKING ) );	

	if ( bInTicket )
	{
		if(nOmitTracking == 0)
		{
			DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - put TicketID in Subject", DebugReporter::ENGINE);
		
			PutTicketIDInSubject( m_TicketID, m_OutboundMessageID, m_Subject, nTicketTracking );
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - Omit Ticket Tracking enabled, remove TicketID from Subject", DebugReporter::ENGINE);
			tstring strSubject(m_Subject);
			RemoveTicketIDFromSubject( strSubject );
			_tcsncpy(m_Subject, strSubject.c_str(), 255);			
		}
	}

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - update the Subject in DB", DebugReporter::ENGINE);
	
	m_query.Initialize();
	BINDPARAM_WCHAR( m_query, m_Subject );
	BINDPARAM_TEXT( m_query, m_Body );
	BINDPARAM_LONG( m_query, m_OutboundMessageID );
	m_query.Execute( L"UPDATE OutboundMessages "
					 L"SET Subject=?,Body=? "
					 L"WHERE OutboundMessageID=?" );

	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - UnDeleteMsg", DebugReporter::ENGINE);
	
	// Commmit the message
	UnDeleteMsg();	
	DebugReporter::Instance().DisplayMessage("COutboundMsg::AutoReply - leaving", DebugReporter::ENGINE);
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Auto Message
// 
////////////////////////////////////////////////////////////////////////////////
bool COutboundMsg::AutoMessage( int StdRespID, CInboundMsg& msg, TCHAR* szAutoReplyFrom, int m_CreateNewTicket, unsigned int nOmitTracking )
{
	GetTimeStamp( m_EmailDateTime );
	m_EmailDateTimeLen = sizeof( TIMESTAMP_STRUCT );
	m_ReplyToMsgID = 0;
	m_ReplyToIDIsInbound = 0;
	
	wcscpy( m_MediaType, msg.m_MediaType );
	wcscpy( m_MediaSubType, msg.m_MediaSubType );

	m_OutboundMessageTypeID = EMS_OUTBOUND_MESSAGE_TYPE_NEW;
	m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_UNTOUCHED;
	m_IsDeleted = 2;
	
	if ( StdRespID > 0 )
	{
		InsertStdRespBody( StdRespID, 0, msg );
	}

	if( wcslen( szAutoReplyFrom ) > 0 )
	{
		wcscpy( m_EmailFrom, szAutoReplyFrom );
	}
	else
	{
		wcscpy( m_EmailFrom, _T("\"MailFlow Mail Subsystem\" <EmailSystem@MailFlow>") );
	}
	
	if( wcslen( msg.m_EmailReplyTo ) > 0 )
	{
		// Set the Primary To: Address
		wcsncpy( m_EmailPrimaryTo, msg.m_EmailReplyTo, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH );

		// And the To: address
		PutStringPropertyW( msg.m_EmailReplyTo, &m_EmailTo, &m_EmailToAllocated );
	}
	else
	{
		// Set the Primary To: Address
		if( wcslen(msg.m_EmailFromName) > 0 )
		{
			_snwprintf( m_EmailPrimaryTo, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
						L"\"%s\" <%s>", msg.m_EmailFromName, msg.m_EmailFrom );
		}
		else
		{
			_snwprintf( m_EmailPrimaryTo, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
						L"<%s>", msg.m_EmailFrom );
		}

		// And the To: address
		PutStringPropertyW( m_EmailPrimaryTo, &m_EmailTo, &m_EmailToAllocated );
	}


	// Copy the EmailFrom -> EmailReplyTo
	PutStringPropertyW( m_EmailFrom, &m_EmailReplyTo, &m_EmailReplyToAllocated );

	// Create the message record
	Insert( m_query );

	// Attempt to add attachments
	if ( StdRespID > 0 )
	{
		AddStdRespAttachments( StdRespID );
	}

	// Attach inline images referenced by the standard response
	AttachSigImages(m_query, m_Body, m_OutboundMessageID);

	PutInOutboundQueue();

	// Put entry in OutboundMessagesContacts
	m_query.Initialize();
	BINDPARAM_LONG( m_query, m_OutboundMessageID );
	BINDPARAM_LONG( m_query, msg.m_ContactID );
	m_query.Execute( L"INSERT INTO OutboundMessageContacts "
						L"(OutboundMessageID,ContactID) "
						L"VALUES "							
						L"(?,?)" );

	//Check for any variables in the subject
	CheckForVariables();
	
	if ( m_CreateNewTicket == 1 )
	{
		//Update the Ticket Subject
		m_query.Initialize();
		BINDPARAM_WCHAR( m_query, m_Subject );
		BINDPARAM_LONG( m_query, m_TicketID );
		m_query.Execute( L"UPDATE Tickets "
						L"SET Subject=? "
						L"WHERE TicketID=?" );
	}
	
	if(nOmitTracking == 0)
	{
		//Put the ticket and outbound message ID's in the subject
		int nTicketTracking = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_TICKET_TRACKING ) );	

		PutTicketIDInSubject( m_TicketID, m_OutboundMessageID, m_Subject, nTicketTracking );
	}
	else
	{
		tstring strSubject(m_Subject);
		RemoveTicketIDFromSubject( strSubject );
		_tcsncpy(m_Subject, strSubject.c_str(), 255);			
	}
	

	//Update the subject in the outbound message
	m_query.Initialize();
	BINDPARAM_WCHAR( m_query, m_Subject );
	BINDPARAM_LONG( m_query, m_OutboundMessageID );
	m_query.Execute( L"UPDATE OutboundMessages "
					 L"SET Subject=? "
					 L"WHERE OutboundMessageID=?" );
	

	// Commmit the message
	UnDeleteMsg();	


	return true;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Check for variables
//
////////////////////////////////////////////////////////////////////////////////
void COutboundMsg::CheckForVariables(void)
{
	DebugReporter::Instance().DisplayMessage("COutboundMsg::CheckForVariables - check subject", DebugReporter::ENGINE);
	
	tstring sSubject;
	CEMSString sTicketID;
	sTicketID.Format( _T("%d"), m_TicketID );

	sSubject.assign( m_Subject );

	//$$TicketID$$
	tstring::size_type pos = sSubject.find( _T("$$TicketID$$") );
	if( pos != tstring::npos )
	{
		sSubject.replace( pos, 12, sTicketID.c_str() );
	}

	_sntprintf( m_Subject, INBOUNDMESSAGES_SUBJECT_LENGTH - 1, sSubject.c_str() );

	DebugReporter::Instance().DisplayMessage("COutboundMsg::CheckForVariables - check body", DebugReporter::ENGINE);
	
	tstring sBody;
	tstring ssTicketID(sTicketID.c_str());
	sBody =  m_Body;
	pos = 0;
	while((pos = sBody.find( _T("$$TicketID$$"), pos )) != tstring::npos)
	{
		sBody.replace( pos, 12, sTicketID.c_str() );
		DebugReporter::Instance().DisplayMessage("COutboundMsg::CheckForVariables - found TicketID", DebugReporter::ENGINE);
		pos += ssTicketID.size();
	}

	PutStringPropertyW( (TCHAR*)sBody.c_str(), &m_Body, &m_BodyAllocated );
}

////////////////////////////////////////////////////////////////////////////////
// 
// InsertStandardResponse
// 
////////////////////////////////////////////////////////////////////////////////
bool COutboundMsg::InsertStdRespBody( int StdRespID, int AutoReplyQuoteMsg, 
									  CInboundMsg& msg )
{
	TStandardResponses stdresp;
	CEMSString sQuotedHdr;
	CEMSString sRecvd;
	tstring sBody;
	CEMSString sTo;
	CEMSString sCc;
	CEMSString sStdRespBody;
	CEMSString sFrom, sSubject;
	tstring sTemp;
	tstring::size_type posTagOpen;
	tstring::size_type posTagClose;
	bool bIsOldSR = true;

	// format the FROM: address
	if ( wcslen( msg.m_EmailFromName ) > 0)
	{
		sFrom.Format( L"\"%s\" <%s>", msg.m_EmailFromName, msg.m_EmailFrom );
	}
	else
	{
		sFrom.assign( msg.m_EmailFrom );
	}

	sTo.assign( msg.m_EmailTo );
	sCc.assign( msg.m_EmailCc );

	stdresp.m_StandardResponseID = StdRespID;

	// Get the standard response info
	if( stdresp.Query( m_query ) != S_OK )
		return false;

	sTemp = stdresp.m_StandardResponse;
	posTagOpen = sTemp.find(_T("<"));
	posTagClose = sTemp.find(_T(">"));
	if( posTagOpen != tstring::npos )
	{
		if ( posTagClose != tstring::npos && posTagClose > posTagOpen )
            bIsOldSR = false;
	}

	if( _tcsicmp( msg.m_MediaSubType, _T("html") ) == 0 )
	{
		tstring::size_type pos;
		tstring sStyle = L"<style>"
							L"#replyquoted {border-left:2px solid blue;margin-left:5px;padding-left:5px;padding-top:10px;padding-bottom:10px;}"
							L"#replyunquoted {border-left:0px;margin-left:0px;padding-left:0px;padding-top:0px;padding-bottom:0px;z-index: 10;color: blue;}"
						L"</style>";

		tstring sOpenDiv = L"<DIV id=replyquoted>";
		tstring sCloseDiv = L"</DIV>";

		// HTML message

		if( AutoReplyQuoteMsg )
		{
			sBody = msg.m_Body;
		}

		pos = 0;

		FindOrInsert( pos, L"<HTML>", sBody );
		FindOrInsert( pos, L"<HEAD>", sBody );

		sBody.insert( pos, sStyle );
		pos += sStyle.length();

		FindOrInsert( pos, L"</HEAD>", sBody );

		sStdRespBody = stdresp.m_StandardResponse;
		if ( bIsOldSR )
		{
			sStdRespBody.EscapeBody( 0, 0, false );
		}
		else
		{
			sStdRespBody.FormatHTMLSigForSend();
		}		
		
		sBody.insert( pos, sStdRespBody.c_str() );
		pos += sStdRespBody.size();

		if( AutoReplyQuoteMsg )
		{
			sFrom.EscapeHTML();

			sSubject.assign( msg.m_Subject );
			sSubject.EscapeHTML();
			
			sBody.insert( pos, sOpenDiv.c_str() );
			pos += sOpenDiv.length();

			GetDateTimeString( msg.m_DateReceived, msg.m_DateReceivedLen, sRecvd );

			sTo.EscapeHTML();
			sCc.EscapeHTML();

			if(msg.m_EmailCcLen > 0)
			{
				sQuotedHdr.Format( _T("-----Original Message-----<br/>From: %s<br/>Received: %s<br/>")
							    _T("To: %s<br/>Cc: %s<br/>Subject: %s<br/><br/>"), sFrom.c_str(), sRecvd.c_str(), 
							    sTo.c_str(), sCc.c_str(), sSubject.c_str()  );
			}
			else
			{
				sQuotedHdr.Format( _T("-----Original Message-----<br/>From: %s<br/>Received: %s<br/>")
								   _T("To: %s<br/>Subject: %s<br/><br/>"), sFrom.c_str(), 
								   sRecvd.c_str(), sTo.c_str(), sSubject.c_str()  );
			}			

			sBody.insert( pos, sQuotedHdr.c_str() );
			pos += sQuotedHdr.length();
		}

		FindOrInsert( pos, L"</HTML>", sBody, true );

		if( AutoReplyQuoteMsg )
		{
			sBody.insert( pos - 7, sCloseDiv.c_str() );
			pos += sCloseDiv.length();
		}
	}
	else
	{
		// Text message
		
		sBody = stdresp.m_StandardResponse;
		
		if ( !bIsOldSR )
		{
			CkGlobal m_glob;
			// unlock Chilkat
			bool success = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
			if (success != true)
			{
				DebugReporter::Instance().DisplayMessage("COutboundMsg::InsertStdRespBody() - Failed to unlock Global component", DebugReporter::MAIL);
				Log(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
											EMSERR_ROUTING_ENGINE,
											EMS_LOG_OUTBOUND_MESSAGING,
											0),
								EMS_STRING_ERROR_INITIALIZE_COMPONENT);

				return false;
			}
			
			int nChrSize = WideCharToMultiByte( CP_ACP, 0, sBody.c_str(), sBody.size(), NULL, 0, NULL, NULL );
			char* html = new char[nChrSize+1];
			WideCharToMultiByte( CP_ACP, 0, sBody.c_str(), sBody.size(), html, nChrSize, NULL, NULL );
			html[nChrSize] = 0;

			const char * plainText;

			plainText = h2t.toText(html);
			
			wstring sData;
			int Len = strlen( plainText ) + 1;
						
			int nWcsSize = MultiByteToWideChar( CP_ACP, 0, plainText, Len, NULL, 0  );
			sData.resize( nWcsSize, L' ' );
			MultiByteToWideChar( CP_ACP, 0, plainText, Len, (LPWSTR)(sData.c_str()), nWcsSize  );

			sBody.assign(sData.c_str());
		}

		if( AutoReplyQuoteMsg )
		{
			GetDateTimeString( msg.m_DateReceived, msg.m_DateReceivedLen, sRecvd );

			if(msg.m_EmailCcLen > 0)
			{
				sQuotedHdr.Format( _T("\r\n> -----Original Message-----\r\n> From: %s\r\n> Received: %s\r\n")
								_T("> To: %s\r\nCc: %s\r\n> Subject: %s\r\n>\r\n"), sFrom.c_str(), 
								sRecvd.c_str(), sTo.c_str(), sCc.c_str(), msg.m_Subject  );				
			}
			else
			{
				sQuotedHdr.Format( _T("\r\n> -----Original Message-----\r\n> From: %s\r\n> Received: %s\r\n")
								_T("> To: %s\r\n> Subject: %s\r\n>\r\n"), sFrom.c_str(), 
								sRecvd.c_str(), sTo.c_str(), 
								msg.m_Subject  );
			}			

			sBody.append( sQuotedHdr );
			sStdRespBody = msg.m_Body;
			sStdRespBody.PrefixBody( _T("> ") );
			sBody.append( sStdRespBody.c_str() );
		}
	}

	// Set the body in the TClass
	PutStringPropertyW( (TCHAR*)sBody.c_str(), &m_Body, &m_BodyAllocated );

	sTemp = stdresp.m_Subject;
	tstring::size_type pos = sTemp.find( _T("$$TicketID$$"), 0 );
	if( pos != tstring::npos )
	{
		sTemp.insert(0, _T(" "));
		sBody = m_Subject;
		sBody.append(sTemp);
		_sntprintf( m_Subject, INBOUNDMESSAGES_SUBJECT_LENGTH - 1, sBody.c_str() );
	}
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FindOrInsert
// 
////////////////////////////////////////////////////////////////////////////////
void COutboundMsg::FindOrInsert( tstring::size_type& lastpos, wchar_t* szTag, 
							   tstring& sBody, bool bAppend )
{
	tstring sTag = szTag;
	tstring::size_type pos;

	pos = sBody.find( L'<', lastpos );
	while(	pos < sBody.length() && 
			pos != tstring::npos && 
			wcsnicmp( sBody.c_str() + pos, sTag.c_str(), sTag.length()-1 ) != 0 )
	{
		pos = sBody.find( L'<', pos + 1 );
	}

	if( pos == sBody.length() || pos == tstring::npos )
	{
		if( bAppend )
		{
			sBody.append( sTag );
		}
		else
		{
			sBody.insert( lastpos, sTag );
		}
		pos = lastpos;
	}

	// [MER] We're making an assumption here that the tag in question houses
	// no attributes
	// pos += sTag.length();

	// if we inserted or appended, we know the length
	if (pos == lastpos)
		pos += sTag.length();
	else
	{
		// let's try to find the end of the tag
		tstring::size_type endtag = sBody.find(L'>', pos + 1);
		if (endtag == tstring::npos)
			pos += sTag.length();
		else
			pos = endtag + 1;
	}
	lastpos = pos;
}

////////////////////////////////////////////////////////////////////////////////
// 
// AddStdRespAttachments
// 
////////////////////////////////////////////////////////////////////////////////
int COutboundMsg::AddStdRespAttachments( int StdRespID )
{
	list<int> AttachList;
	list<int>::iterator iter;
	int nAttachID;
	int nVirusScanStateID;

	// Forward attachments
	m_query.Initialize();
	BINDPARAM_LONG( m_query, StdRespID );
	BINDCOL_LONG_NOLEN( m_query, nAttachID );
	BINDCOL_LONG_NOLEN( m_query, nVirusScanStateID );

	m_query.Execute( L"SELECT Attachments.AttachmentID,VirusScanStateID "
					 L"FROM Attachments  "
					 L"INNER JOIN StdResponseAttachments "
					 L"ON Attachments.AttachmentID = StdResponseAttachments.AttachmentID "
					 L"WHERE StandardResponseID=?" );
	
	while( m_query.Fetch() == S_OK )
	{
		if( !EMS_IS_INFECTED( nVirusScanStateID) )
		{
			AttachList.push_back( nAttachID );
		}
	}

	for( iter = AttachList.begin(); iter != AttachList.end(); iter++ )
	{	

		// Now create the database record
		m_query.Initialize();
		BINDPARAM_LONG( m_query, m_OutboundMessageID  );
		BINDPARAM_LONG( m_query, *iter );

		m_query.Execute( L"INSERT INTO OutboundMessageAttachments "
						 L"(OutboundMessageID,AttachmentID) "
						 L"VALUES "
						 L"(?,?)" );
	}

	// insert a record into standard response usage for the system AgentID=0
	m_query.Initialize();
	BINDPARAM_LONG( m_query, StdRespID );
	m_query.Execute( L"INSERT INTO StandardResponseUsage "
		             L"(StandardResponseID,AgentID,DateUsed) "
					 L"VALUES "
					 L"(?,0,getdate()) " );

	return 0;
}
