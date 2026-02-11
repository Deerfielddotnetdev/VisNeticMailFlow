// SMTPConnection.cpp: implementation of the CSMTPConnection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MailComponents.h"
#include "SMTPConnection.h"
#include "ODBCConn.h"
#include "ODBCQuery.h"
#include "SingleCrit.h"
#include "MessageIO.h"
#include "Utility.h"
#include "md5class.h"
#include "FileEncoder.h"
#include "AttachFns.h"
#include "InboundMessage.h"
#include "ServerParameters.h"
#include "RegistryFns.h"
#include "TicketHistoryFns.h"
#include "QueryClasses.h"
#include "EmailDate.h"
#include "DateFns.h"

extern dca::Mutex g_csDB;
extern CODBCConn g_odbcConn;

static const string sAttachMarkerStart	= "<x-mailstream-attachment-marker>";
static const string sAttachMarkerEnd	= "</x-mailstream-attachment-marker>";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSMTPConnection::CSMTPConnection()
{
	m_pMsgDest = NULL;
}

CSMTPConnection::~CSMTPConnection()
{
	ClearMessages();
	if(m_mailman.get_IsSmtpConnected())
	{
		m_mailman.CloseSmtpConnection();
	}
	m_mailman.dispose();
	dca::String f;
	f.Format("CSMTPConnection::~CSMTPConnection() - Disposing SMTPConn for MessageDestinationID: %d", m_pMsgDest->GetMessageDestID());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
}

void CSMTPConnection::SetMsgDest(CSMTPDest* const pMsgDest)
{
	assert(pMsgDest);

	m_pMsgDest = pMsgDest;	
}

const int CSMTPConnection::DeliverMessages()
{
	int nRet = 0;
	bool bRet;
	long maxWaitMillisec = m_pMsgDest->GetConnTimeoutSecs()*1000;

	// set the "last processed" entry in the database
	// to do: log error?
	nRet = SetLastProcessed();
	if (nRet != 0)
	{
		// logpoint: Failed to update last-processed value for message destination ID %d (server: %s, port: %d)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRet),
						IDS_STRING_MAILCOMP_LASTCHECKED_UPDATE_FAILURE,
						m_pMsgDest->GetDescription().c_str());

		assert(0);
		return nRet;
	}

	// get waiting message count
	// to do: log error?
	LONG lCount = 0;
	nRet = GetWaitingMsgCount(lCount);
	if (nRet != 0)
	{
		// logpoint: Failed to retrieve waiting message count for message destination ID %d (server: %s, port: %d)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRet),
						IDS_STRING_MAILCOMP_OUTBOUND_WAITING_COUNT_READ_FAILURE,
						m_pMsgDest->GetDescription().c_str());

		//assert(0);
		return nRet;
	}

	// logpoint: Message destination has %d messages to process (server: %s, port: %d)
	CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_OUTBOUND_MESSAGING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_OUTBOUND_MSGS_WAITING,
					m_pMsgDest->GetDescription().c_str(),
					lCount);

	// if there are no messages waiting, we're done
	if (lCount == 0)
		return 0;

	// otherwise, we need to connect, auth, etc.
	// initialize session transcript and append new session indicator
	InitTranscript();
	AppendTranscript(IDS_STRING_MAILCOMP_TRANSCRIPT_SMTP_CONNECTING, m_pMsgDest->GetServerAddress().c_str(), m_pMsgDest->GetSMTPPort());

	// unlock Chilkat
	bRet = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
	if (bRet != true)
	{
		nRet = 1;
		DebugReporter::Instance().DisplayMessage("CSMTPConnection::DeliverMessages() - Failed to unlock Global component", DebugReporter::MAIL);
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRet),
						EMS_STRING_ERROR_INITIALIZE_COMPONENT);

		if(m_pMsgDest->GetErrorCode() == 0)
		{
			CreateAlert( EMS_ALERT_EVENT_OUTBOUND_DELIVERY, (EMS_STRING_ERROR_INITIALIZE_COMPONENT));
			SetErrorCode(1);
		}
		if (HandleDelFailureAll() != 0)
			assert(0);

		return nRet;
	}	

	nRet = SetHello();
	if (nRet != 0)
	{
		LINETRACE(_T("Failed to set HELO Host: %d\n"), nRet);

		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_SMTP_HELLO_FAILURE,
						m_pMsgDest->GetDescription().c_str(),
						m_pMsgDest->GetServerAddress().c_str());

		if (HandleDelFailureAll() != 0)
			assert(0);

		return nRet;
	}

	// establish server connection
	if (m_pMsgDest->GetIsSSL())
	{
		nRet = InitSecureConnection();
	}
	else
	{
		nRet = InitConnection();
	}
		
	if (nRet != 0)	
	{
		// logpoint: Failed to initialize connection to SMTP server (server: %s, port: %d)
		nRet = 1;
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_SMTP_SERVER_CONN_FAILURE,
						m_pMsgDest->GetDescription().c_str(),
						m_pMsgDest->GetServerAddress().c_str(),
						m_pMsgDest->GetSMTPPort());

		if (m_pMsgDest->GetErrorCode() == 0)
		{
			CreateAlert( EMS_ALERT_EVENT_OUTBOUND_DELIVERY, (EMS_STRING_MAILCOMP_SMTP_SERVER_CONN_FAILURE),
							m_pMsgDest->GetDescription().c_str(),
							m_pMsgDest->GetServerAddress().c_str(),
							m_pMsgDest->GetSMTPPort());

			SetErrorCode(1);
		}

		if (HandleDelFailureAll() != 0)
			assert(0);

		return nRet;
	}

	if (m_pMsgDest->GetUsesSMTPAuth())
	{
		if(m_pMsgDest->GetOAuthHostID()>0)
		{
			nRet = AuthenticateOAuth2();			
		}
		else
		{
			nRet = Authenticate();			
		}
		
		if (nRet != 0)
		{
			// logpoint: Failed to authenticate to SMTP server (username: %s, server: %s, port: %d)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_OUTBOUND_MESSAGING,
										nRet),
							EMS_STRING_MAILCOMP_SMTPAUTH_FAILURE,
							m_pMsgDest->GetDescription().c_str(),
							m_pMsgDest->GetServerAddress().c_str(),
							m_pMsgDest->GetAuthUser().c_str());

			if (m_pMsgDest->GetErrorCode() == 0)
			{
				CreateAlert( EMS_ALERT_EVENT_OUTBOUND_DELIVERY, (EMS_STRING_MAILCOMP_SMTPAUTH_FAILURE),
							m_pMsgDest->GetDescription().c_str(),
							m_pMsgDest->GetServerAddress().c_str(),
							m_pMsgDest->GetAuthUser().c_str());

				SetErrorCode(1);
			}
			
			if (HandleDelFailureAll() != 0)
				assert(0);

			return nRet;
		}
	}

	if (m_pMsgDest->GetErrorCode() > 0)
	{
		SetErrorCode(0);
	}

	// get waiting messages from db
	bool bMoreWaiting = true;
	while (bMoreWaiting)
	{
		// clear msg map
		ClearMessages();

		// get outbound message batch
		nRet = GetOutboundMsgs(bMoreWaiting);
		if (nRet != 0)
		{
			// logpoint: Failed to retrieve outbound messages from database (dest server: %s, port: %d)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_OUTBOUND_MESSAGING,
										nRet),
							EMS_STRING_MAILCOMP_OUTBOUND_MSG_DBREAD_FAILURE,
							m_pMsgDest->GetDescription().c_str());

			assert(nRet != ERROR_MEMORY_ALLOCATION);

			break;
		}

		// if there are no messages to process, we're done
		if (m_OutboundMsgs.size() == 0)
			break;

		// get outbound message attachments
		nRet = GetOutboundMsgAttachments();
		if (nRet != 0)
		{
			// logpoint: Failed to retrieve message attachment info from database (server: %s, port: %d)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_OUTBOUND_MESSAGING,
										nRet),
							EMS_STRING_MAILCOMP_ATTACHMENT_DB_READ_FAILURE,
							m_pMsgDest->GetDescription().c_str());

			break;
		}

		// processes the outbound messages
		nRet = SendMessageSet();
		if (nRet != 0)
		{
			// logpoint: Failed to process outbound messages (server: %s, port: %d)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_OUTBOUND_MESSAGING,
										nRet),
							EMS_STRING_MAILCOMP_OUTBOUND_MSG_PROCESSING_FAILURE,
							m_pMsgDest->GetDescription().c_str());

			break;
		}		
	}

	ClearMessages();

	// send quit command to server; this occurs only if message sending
	// operations terminate "normally"
	if (Quit() != 0)
	{
		LINETRACE(_T("SMTP QUIT command failed\n"));
		// don't fail here, just close the socket
	}

	// logpoint: Outbound message delivery complete (server: %s, port: %d)
	CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_OUTBOUND_MESSAGING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_OUTBOUND_DEL_COMPLETE,
					m_pMsgDest->GetDescription().c_str());

	return nRet;
}

void CSMTPConnection::Stop()
{
	if (m_mailman.get_IsSmtpConnected())
	{
		m_mailman.CloseSmtpConnection();
	}	
}

const int CSMTPConnection::GetOutboundMsgs(bool& bMoreWaiting)
{
	bMoreWaiting = false;

	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		OutboundMessages_t out;
		BINDCOL_LONG(query, out.m_OutboundMessageID);
		BINDCOL_LONG(query, out.m_TicketID);
		BINDCOL_TINYINT(query, out.m_FooterLocation);
		BINDCOL_WCHAR(query, out.m_EmailFrom);
		BINDCOL_WCHAR(query, out.m_EmailPrimaryTo);
		BINDCOL_TIME(query, out.m_EmailDateTime);
		BINDCOL_WCHAR(query, out.m_MediaType);
		BINDCOL_WCHAR(query, out.m_MediaSubType);
		BINDCOL_WCHAR(query, out.m_Subject);
		BINDCOL_LONG(query, out.m_OutboundMessageStateID);
		BINDCOL_LONG(query, out.m_PriorityID);
		BINDCOL_TINYINT(query, out.m_ReadReceipt);
		BINDCOL_TINYINT(query, out.m_DeliveryConfirmation);
		BINDCOL_LONG(query, out.m_ReplyToIDIsInbound);
		
		OutboundMessageQueue_t queue;
		BINDCOL_LONG(query, queue.m_SpoolFileGenerated);
		BINDCOL_TIME(query, queue.m_DateSpooled);
		BINDCOL_TIME(query, queue.m_LastAttemptedDelivery);

		TicketBoxes_t tb;
		BINDCOL_WCHAR(query, tb.m_ReplyToEmailAddress);
		BINDCOL_WCHAR(query, tb.m_ReturnPathEmailAddress);

		MessageTracking_t mt;
		BINDCOL_WCHAR(query, mt.m_HeadMsgID);
		BINDCOL_LONG(query, mt.m_MessageID);

		TicketBoxFooters_t footer;
		TicketBoxHeaders_t header;

		LONG nMsgDestID = m_pMsgDest->GetMessageDestID();
		BINDPARAM_LONG(query, nMsgDestID);

		// note: header and footer are added to the message during the spool
		// file generation process

		query.Execute(_T("SELECT o.OutboundMessageID, o.TicketID, o.FooterLocation, o.EmailFrom, o.EmailPrimaryTo, ")
				        _T("o.EmailDateTime, o.MediaType, o.MediaSubType, o.Subject, ")
						_T("o.OutboundMessageStateID, o.PriorityID, o.ReadReceipt, o.DeliveryConfirmation, o.ReplyToIDIsInbound, q.SpoolFileGenerated, q.DateSpooled, q.LastAttemptedDelivery, tb.ReplyToEmailAddress, tb.ReturnPathEmailAddress, mt.HeadMsgId, im.TicketID, ")
						_T("o.Body, o.EmailTo, o.EmailCc, o.EmailBcc, o.EmailReplyTo, h.Header, f.Footer FROM OutboundMessages AS o ")
						_T("INNER JOIN OutboundMessageQueue AS q ON o.OutboundMessageID = q.OutboundMessageID ")
						_T("INNER JOIN MessageDestinations as d ON q.MessageDestinationID = d.MessageDestinationID ")
						_T("LEFT OUTER JOIN Tickets as t ON o.TicketID = t.TicketID ")
						_T("LEFT OUTER JOIN TicketBoxes as tb ON tb.TicketBoxID = t.TicketBoxID ")						
						_T("LEFT OUTER JOIN MessageTracking as mt ON o.ReplyToMsgID = mt.MessageID ")						
						_T("LEFT OUTER JOIN InboundMessages as im ON o.ReplyToMsgID = im.InboundMessageID ")						
						_T("LEFT OUTER JOIN TicketBoxHeaders as h ON o.TicketBoxHeaderID = h.HeaderID ")
						_T("LEFT OUTER JOIN TicketBoxFooters as f ON o.TicketBoxFooterID = f.FooterID ")
						_T("WHERE o.IsDeleted = 0 AND q.IsApproved = 1 ")
						_T("AND d.IsActive = 1 AND q.MessageDestinationID = ? ")
						_T("AND o.ArchiveID = 0 ")
						_T("AND o.OutboundMessageStateID IN (3, 4, 6) ORDER BY q.LastAttemptedDelivery ASC"));

		USES_CONVERSION;

		// iterate over the sql fetch; note that we
		// won't instantiate more than X outbound message objects at
		// a time in order to avoid memory spikes
		int i = 0;
		while (query.Fetch() == S_OK)
		{
			if (++i >= 15)
			{
				bMoreWaiting = true;
				break;
			}

			GETDATA_TEXT(query, out.m_Body);
			GETDATA_TEXT(query, out.m_EmailTo);
			GETDATA_TEXT(query, out.m_EmailCc);
			GETDATA_TEXT(query, out.m_EmailBcc);
			GETDATA_TEXT(query, out.m_EmailReplyTo);
			GETDATA_TEXT(query, header.m_Header);
			GETDATA_TEXT(query, footer.m_Footer);

			// transform the raw database data into an outbound message
			COutboundMessage* pOut = new COutboundMessage;
			if(pOut)
			{
				string sData; // temp storage
				int nLen;
				dca::String f;						

				f.Format("CSMTPConnection::GetOutboundMsgs - Processing OutboundMessageID [%d]", out.m_OutboundMessageID);
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				
				// outbound message id
				pOut->SetMessageID(out.m_OutboundMessageID);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - TicketID", DebugReporter::MAIL);

				// ticket id
				if(out.m_TicketID == 0)
				{
					pOut->SetTicketID(mt.m_MessageID);
				}
				else
				{
					pOut->SetTicketID(out.m_TicketID);
				}				

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - footer location", DebugReporter::MAIL);

				// footer location
				pOut->SetFooterLoc(out.m_FooterLocation);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - read receipt", DebugReporter::MAIL);

				// read receipt
				pOut->SetReadReceipt(out.m_ReadReceipt);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - delivery confirmation", DebugReporter::MAIL);

				// delivery confirmation
				pOut->SetDeliveryConfirmation(out.m_DeliveryConfirmation);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - message destination", DebugReporter::MAIL);

				// message destination id
				pOut->SetMessageDestID(nMsgDestID);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - from", DebugReporter::MAIL);

				// parse out the email from full name
				ParseAddress(out.m_EmailFrom, pOut->GetOriginator());

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - to", DebugReporter::MAIL);

				// to addresses
				ParseAddresses(out.m_EmailTo, pOut->GetToRecipients());

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - cc", DebugReporter::MAIL);

				// cc
				ParseAddresses(out.m_EmailCc, pOut->GetCCRecipients());

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - bcc", DebugReporter::MAIL);
				
				// bcc
				ParseAddresses(out.m_EmailBcc, pOut->GetBCCRecipients());

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - reply-to", DebugReporter::MAIL);
				
				// reply-to
				if( wcsicmp( out.m_EmailReplyTo, L"" ) == 0 )
				{
					ParseAddresses(tb.m_ReplyToEmailAddress, pOut->GetReplyToList());
				}
				else
				{
					ParseAddresses(out.m_EmailReplyTo, pOut->GetReplyToList());
				}

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - return-path", DebugReporter::MAIL);
				
				// return-path
				ParseAddresses(tb.m_ReturnPathEmailAddress, pOut->GetReturnPathList());

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - reply to ID is inbound", DebugReporter::MAIL);

				if(out.m_ReplyToIDIsInbound)
				{
					dca::String msgid(mt.m_HeadMsgID);
					pOut->GetInReplyTo().SetText(msgid.c_str(), "ISO-8859-1");
					pOut->GetReferences().SetText(msgid.c_str(), "ISO-8859-1");
				}

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - date/time", DebugReporter::MAIL);
				
				// message date/time
				pOut->GetDate().FromSqlTime(out.m_EmailDateTime);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - date spooled", DebugReporter::MAIL);

				// date spooled
				pOut->GetDateSpooled().FromSqlTime(queue.m_DateSpooled);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - last delivery attempt", DebugReporter::MAIL);

				// last attempted delivery
				pOut->GetLastDeliveryAttempt().FromSqlTime(queue.m_LastAttemptedDelivery);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - subject", DebugReporter::MAIL);

				// subject				
				tstring sTemp(out.m_Subject);
				int len = (int) sTemp.length();
				DwBool has8bitChars = DwFalse;
				for (int i=0; i < len; ++i) 
				{
					int ch = sTemp[i] & 0xff;
					if (ch < 32 || 126 < ch) 
					{
						has8bitChars = DwTrue;
						break;
					}
				}
				
				if( has8bitChars )
				{
					DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - has8bitChars", DebugReporter::MAIL);

					USES_CONVERSION;
					DwString sEncoded;					
					DwString sSub(T2A(sTemp.c_str()));
					DwEncodeBase64(sSub, sEncoded);
					std::string s(sEncoded.c_str());
					while ( s.find ("\r\n") != string::npos )
					{
    					s.erase ( s.find ("\r\n"), 2 );
					}
					sSub.assign("");
					sSub.append("=?utf-8?B?");
					sSub.append(s.c_str());
					sSub.append("?=");
					pOut->GetSubject().SetText(sSub.c_str(), "ISO-8859-1");
				}
				else
				{
					DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - no8bitChars", DebugReporter::MAIL);

					dca::String sub(out.m_Subject);
					pOut->GetSubject().SetText(sub.c_str(), "ISO-8859-1");
				}				
				
				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - message state", DebugReporter::MAIL);

				
				// outbound message state
				pOut->SetOutboundMessageState(out.m_OutboundMessageStateID);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - footer", DebugReporter::MAIL);

				// footer
				if (footer.m_FooterLen != -1)
				{
					int nFooterLen = 0;
					nFooterLen = WideCharToMultiByte( CP_ACP, 0, footer.m_Footer, -1, NULL, 0, NULL, NULL );
					sData.resize(nFooterLen);
					nLen = WideCharToMultiByte(CP_ACP, 0, footer.m_Footer, -1, (char*)sData.data(), nFooterLen, NULL, NULL);
					if (nLen == 0)
					{
						DWORD lastError = GetLastError();
						f.Format("CSMTPConnection::GetOutboundMsgs - Failed to convert footer for OutboundMessageID [%d] with Error [%d]", out.m_OutboundMessageID, lastError);
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						//return ERROR_MEMORY_ALLOCATION;

						// logpoint: [%s] failed to prepare outbound message ID: (%d) for sending 
						CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
													EMSERR_MAIL_COMPONENTS,
													EMS_LOG_OUTBOUND_MESSAGING,
													0),
										EMS_STRING_MAILCOMP_OUTBOUND_MSG_PREP_FAILURE,
										m_pMsgDest->GetDescription().c_str(),
										out.m_OutboundMessageID);
						continue;
					}
					sData.resize(nLen-1);
					
					if( wcsicmp( out.m_MediaSubType, L"html" ) == 0 )
					{
						EscapeHTMLAndNewLines( sData );
					}

					pOut->GetFooter().SetText(sData, "ISO-8859-1");
				}

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - header", DebugReporter::MAIL);

				// header
				if (header.m_HeaderLen != -1)
				{
					int nHeaderLen = 0;
					nHeaderLen = WideCharToMultiByte( CP_ACP, 0, header.m_Header, -1, NULL, 0, NULL, NULL );
					sData.resize(nHeaderLen);
					nLen = WideCharToMultiByte(CP_ACP, 0, header.m_Header, -1, (char*)sData.data(), nHeaderLen, NULL, NULL);
					if (nLen == 0)
					{
						DWORD lastError = GetLastError();
						f.Format("CSMTPConnection::GetOutboundMsgs - Failed to convert header for OutboundMessageID [%d] with Error [%d]", out.m_OutboundMessageID, lastError);
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						//return ERROR_MEMORY_ALLOCATION;

						// logpoint: [%s] failed to prepare outbound message ID: (%d) for sending 
						CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
													EMSERR_MAIL_COMPONENTS,
													EMS_LOG_OUTBOUND_MESSAGING,
													0),
										EMS_STRING_MAILCOMP_OUTBOUND_MSG_PREP_FAILURE,
										m_pMsgDest->GetDescription().c_str(),
										out.m_OutboundMessageID);
						continue;
					}
					sData.resize(nLen-1);

					if( wcsicmp( out.m_MediaSubType, L"html" ) == 0 )
					{
						EscapeHTMLAndNewLines( sData );
					}

					pOut->GetHeader().SetText(sData, "ISO-8859-1");
				}
				
				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - body", DebugReporter::MAIL);

				// body
				if (out.m_BodyLen != -1)
				{
					int nBodyLen = 0;

					nBodyLen = WideCharToMultiByte( CP_ACP, 0, out.m_Body, -1, NULL, 0, NULL, NULL );
					sData.resize(nBodyLen);
					nLen = WideCharToMultiByte(CP_ACP, 0, out.m_Body, -1, (char*)sData.data(), nBodyLen, NULL, NULL);
					if (nLen == 0)
					{
						DWORD lastError = GetLastError();
						f.Format("CSMTPConnection::GetOutboundMsgs - Failed to convert body for OutboundMessageID [%d] with Error [%d]", out.m_OutboundMessageID, lastError);
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						//return ERROR_MEMORY_ALLOCATION;

						// logpoint: [%s] failed to prepare outbound message ID: (%d) for sending 
						CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
													EMSERR_MAIL_COMPONENTS,
													EMS_LOG_OUTBOUND_MESSAGING,
													0),
										EMS_STRING_MAILCOMP_OUTBOUND_MSG_PREP_FAILURE,
										m_pMsgDest->GetDescription().c_str(),
										out.m_OutboundMessageID);
						continue;
					}
					sData.resize(nLen-1);
					pOut->GetMemoText().SetText(sData, "ISO-8859-1");
				}

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - media type/subtype", DebugReporter::MAIL);

				// media type and subtype
				{
					dca::String sMedia(out.m_MediaType);
					pOut->GetMemoText().SetMediaType(sMedia.c_str());

					dca::String sMediaSubType(out.m_MediaSubType);
					pOut->GetMemoText().SetMediaSubType(sMediaSubType);
				}

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - set spool file generated", DebugReporter::MAIL);

				// spool file generated (bool)
				pOut->SetSpoolFileGenerated(queue.m_SpoolFileGenerated);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - priority ID", DebugReporter::MAIL);

				// priority ID
				pOut->SetPriorityID(out.m_PriorityID);

				DebugReporter::Instance().DisplayMessage("CSMTPConnection::GetOutboundMsgs - push back", DebugReporter::MAIL);

				// add it to the list
				m_OutboundMsgs.push_back(pOut);				
			}
		} 
	}
	catch(ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}

void CSMTPConnection::ClearMessages()
{
	for (	MSG_CONT::iterator cur(m_OutboundMsgs.begin()); 
			cur != m_OutboundMsgs.end(); 
			++cur)
	{
        if (*cur)
			delete *cur;
    }
	
	m_OutboundMsgs.clear();
}

void CSMTPConnection::ParseAddresses(const wchar_t *addrs, CEmailAddressList &elist)
{
	USES_CONVERSION;

	assert(addrs);

	// addresses must be COMMA-separated to be properly parsed;
	// look for semi-colons and replace them with commas
	string sAddrs(T2A(addrs));
	string repl_with(",");
	std::string::size_type pos = 0;
	while((pos = sAddrs.find(';', pos)) != std::string::npos) 
	{
		sAddrs.replace(pos, 1, repl_with);
		pos++;
	}

	// replace "" with "
	pos = 0;
	while((pos = sAddrs.find("\"\"", pos)) != std::string::npos) 
	{
		sAddrs.replace(pos, 2, "\"");
		pos++;
	}	

	DwAddressList addrList;
	addrList.FromString(sAddrs.c_str());
	addrList.Parse();
	int num = addrList.NumAddresses();
	for (int i=0; i<num; i++)
	{
		const DwAddress& addr = addrList.AddressAt(i);
		if (addr.IsMailbox()) 
		{
			const DwMailbox& rcpt = (const DwMailbox&) addr;
			DwString inetName = rcpt.LocalPart();
			inetName += "@";
			inetName += rcpt.Domain();
			inetName.Trim();
			if ((inetName[0] != '<') && 
				(inetName[inetName.size()-1] != '>'))
			{
				inetName.insert(0, "<");
				inetName.append(">");
			}

			DwString personalName, charset;
			rcpt.GetFullName(personalName, charset);
			int len = (int) personalName.length();
			DwBool has8bitChars = DwFalse;
			for (int i=0; i < len; ++i) 
			{
				int ch = personalName[i] & 0xff;
				if (ch < 32 || 126 < ch) 
				{
					has8bitChars = DwTrue;
					break;
				}
			}
			if( has8bitChars )
			{
				DwString sEncoded;					
				DwEncodeBase64(personalName, sEncoded);
				std::string s(sEncoded.c_str());
				while ( s.find ("\r\n") != string::npos )
				{
    				s.erase ( s.find ("\r\n"), 2 );
				}
				personalName.assign("");
				personalName.append("=?utf-8?B?");
				personalName.append(s.c_str());
				personalName.append("?=");				
			}
			
			CEmailAddress eaddr(inetName.c_str(), personalName.c_str(), charset.c_str());
			elist.Add(eaddr);
			
		}					
	}
}

const int CSMTPConnection::GetOutboundMsgAttachments()
{
	USES_CONVERSION;

	list<int>::iterator iter;
	list<int> MissingAttachmentIDs;

	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		// msg store iteration
		for (	MSG_CONT::iterator cur(m_OutboundMsgs.begin()); 
				cur != m_OutboundMsgs.end(); 
				++cur)
		{
			COutboundMessage* pMsg = (COutboundMessage*)*cur;
			assert(pMsg);
			if (pMsg)
			{
				int nMsgId = pMsg->GetMessageID();
				assert(nMsgId > 0);

				Attachments_t oAttach;
				BINDCOL_LONG(query, oAttach.m_AttachmentID);
				BINDCOL_WCHAR(query, oAttach.m_AttachmentLocation);
				BINDCOL_WCHAR(query, oAttach.m_MediaType);
				BINDCOL_WCHAR(query, oAttach.m_MediaSubType);
				BINDCOL_WCHAR(query, oAttach.m_ContentDisposition);
				BINDCOL_WCHAR(query, oAttach.m_FileName);
				BINDCOL_BIT( query, oAttach.m_IsInbound);
				BINDCOL_WCHAR(query, oAttach.m_ContentID);
				BINDPARAM_LONG(query, nMsgId);

				query.Execute(	_T("select Attachments.AttachmentID, AttachmentLocation, ")
								_T("MediaType, MediaSubType, ContentDisposition, FileName, IsInbound, ContentID ")
								_T("FROM Attachments ")
								_T("INNER JOIN OutboundMessageAttachments ")
								_T("ON Attachments.AttachmentID=OutboundMessageAttachments.AttachmentID ")
								_T("WHERE OutboundMessageID = ?"));

				CAttachmentList& aList = pMsg->GetAttachments();

				// iterate over the sql fetch
				while (query.Fetch() == S_OK)
				{
					CAttachment attach;
					dca::WString sAttachPath;

					attach.SetAttachmentID(oAttach.m_AttachmentID);
					attach.SetFileName(T2A(oAttach.m_FileName));
					attach.SetFullPath(oAttach.m_AttachmentLocation);
					// note: the message media type and subtype should always be set here
					attach.SetMediaType(T2A(oAttach.m_MediaType), T2A(oAttach.m_MediaSubType));
					attach.SetContentID(T2A(oAttach.m_ContentID));

					assert(attach.GetType().size() != 0);
					assert(attach.GetSubType().size() != 0);

					attach.SetCharSet("ISO-8859-1");
					attach.SetContentDisposition(oAttach.m_ContentDisposition);

					attach.SetIsInbound( oAttach.m_IsInbound);

					// Generate the full path
					if( attach.GetIsInbound() != 0 )
					{
						GetFullInboundAttachPath( (TCHAR*) attach.GetFullpath().c_str(), sAttachPath);
					}
					else
					{
						GetFullOutboundAttachPath( (TCHAR*) attach.GetFullpath().c_str(), sAttachPath);
					}
					attach.SetFullPath( sAttachPath.c_str() );

					if( VerifyFileExists( sAttachPath.c_str() ) == false )
					{
						MissingAttachmentIDs.push_back( oAttach.m_AttachmentID );
						CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
													EMSERR_MAIL_COMPONENTS,
													EMS_LOG_OUTBOUND_MESSAGING,
													ERROR_FILESYSTEM),
										EMS_STRING_MAILCOMP_ATTACHMENT_NOT_FOUND,
										sAttachPath.c_str() );

					}
					else
					{
						// note: the attachment content is set later, as spool file is generated
						aList.AddAttachment(attach);
					}
				}

				// Remove the links for attachments we couldn't find on disk
				for( iter = MissingAttachmentIDs.begin(); iter != MissingAttachmentIDs.end(); iter++ )
				{
					query.Reset(true);
					BINDPARAM_LONG( query, nMsgId );
					BINDPARAM_LONG( query, *iter );
					query.Execute( _T("DELETE FROM OutboundMessageAttachments ")
						           _T("WHERE OutboundMessageID=? AND AttachmentID=?") );	
				}
	
				// reset the query object
				query.Reset(true);
			}
		}
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}

// reads attachment content from an attachment file
const int CSMTPConnection::ReadAttachmentContent(const tstring &sAttachFile, DwString &sContent)
{
	assert(sAttachFile.size() != 0);
	if (sAttachFile.size() == 0)
		return ERROR_BAD_PARAMETER;

	// open the source file
	HANDLE hFile = CreateFile(	sAttachFile.c_str(), 
								GENERIC_READ, 
								0, 
								NULL, 
								OPEN_EXISTING, 
								FILE_ATTRIBUTE_NORMAL, 
								NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LINETRACE(_T("Failed in call to CreateFile, GetLastError returned %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
		assert(0);
		return ERROR_FILESYSTEM;
	}

	// read the file, 64k at a time, into our string object
	const int nChunkSize = 64*1024;
	char buffer[nChunkSize];
	DWORD dwRead;
	while (1)
	{
		if (!ReadFile(hFile, buffer, nChunkSize, &dwRead, NULL))
		{
			LINETRACE(_T("Failed to read from file: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
			assert(0);
			return ERROR_FILESYSTEM;
		}

		if (dwRead == 0)
			break;

		sContent.append(buffer, dwRead);
	}

	CloseHandle(hFile);

	return 0;
}

const int CSMTPConnection::SendMessageSet()
{
	int nRet = 0;

	// msg count check is enforced in DeliverMessages()
	assert(m_OutboundMsgs.size() > 0);

	// iterate over messages
	for (	MSG_CONT::iterator cur(m_OutboundMsgs.begin()); 
			cur != m_OutboundMsgs.end(); 
			++cur)
	{
		COutboundMessage* pMsg = (COutboundMessage*)*cur;
		assert(pMsg);
		if (pMsg)
		{
			// prepare this message for sending
			nRet = PrepareOutboundMsg(pMsg);
			if (nRet != 0)
			{
				// logpoint: Failed to prepare outbound message for sending (server: %s, port: %d, msg id: %d)
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
											EMSERR_MAIL_COMPONENTS,
											EMS_LOG_OUTBOUND_MESSAGING,
											nRet),
								EMS_STRING_MAILCOMP_OUTBOUND_MSG_PREP_FAILURE,
								m_pMsgDest->GetDescription().c_str(),
								pMsg->GetMessageID());

				// handle delivery failure (flags message as failed if needed)
				nRet = HandleDelFailure(pMsg);
				if (nRet != 0)
				{
					// logpoint: Error handling message delivery failure (server: %s, port: %d, msg id: %d)
					CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
												EMSERR_MAIL_COMPONENTS,
												EMS_LOG_OUTBOUND_MESSAGING,
												nRet),
									EMS_STRING_MAILCOMP_MSGDEL_FAILUREHANDLER_FAILURE,
									m_pMsgDest->GetDescription().c_str(),
									pMsg->GetMessageID());
					
					break; // stop processing
				}

				continue; // next message
			}

			{
				dca::String f;
				dca::String s(pMsg->GetSubject().GetText().c_str());
				f.Format("CSMTPConnection::SendMessageSet - Getting ready to send message with subject [ %s ]", s.c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			}

			// send the message out
			bool bSafeCont = true;
			nRet = SendMsg(pMsg, bSafeCont);
			if (nRet != 0)
			{
				// SendMsg() will check the transaction state with Reset(); if this fails, then
				// the transaction state is likely corrupt and we'll need to abort processing
				if (!bSafeCont)
				{
					pMsg->SetOutboundMessageState(EMS_OUTBOUND_MESSAGE_STATE_RETRY);
					LINETRACE(_T("Aborting message processing due to unsafe or invalid transaction state\n"));
					break;
				}

				// the following SendMsg() return codes will cause the outbound message
				// to fail immediately and permanently (delivery will NOT be re-attempted)
				if (nRet == ERROR_NO_VALID_MSG_RECIPIENTS ||			// no valid recipients
					nRet == ERROR_OUTBOUND_MSG_MAXSIZE_EXCEEDED ||		// max outbound msg size exceeded
					nRet == ERROR_PERMANENT_MSG_DEL_FAILURE)			// permanent delivery failure
				{
					pMsg->SetOutboundMessageState(EMS_OUTBOUND_MESSAGE_STATE_FAILED);
				}
				
				// if this is a command failure, evaluate the return code for failure
				// permanence
				if (nRet == ERROR_COMMAND_FAILED)
				{
					int nErr = atoi(m_sResponse.c_str());
					if (nErr >= 500 && nErr <= 599)
						pMsg->SetOutboundMessageState(EMS_OUTBOUND_MESSAGE_STATE_FAILED);
				}

				// handle delivery failure (flags message as failed if needed)
				nRet = HandleDelFailure(pMsg);
				if (nRet != 0)
				{
					// logpoint: Error handling message delivery failure (server: %s, port: %d, msg id: %d)
					CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
												EMSERR_MAIL_COMPONENTS,
												EMS_LOG_OUTBOUND_MESSAGING,
												nRet),
									EMS_STRING_MAILCOMP_MSGDEL_FAILUREHANDLER_FAILURE,
									m_pMsgDest->GetDescription().c_str(),
									pMsg->GetMessageID());

					assert(0);
					break; // stop processing
				}
			}
			else
			{
				// handle the succesful delivery; if this function fails, then this message
				// may be re-delivered at a later date
				// to do: evaluate this further
				{
					dca::String f;
					dca::String s(pMsg->GetSubject().GetText().c_str());
					f.Format("CSMTPConnection::SendMessageSet - Successfully sent message with subject [ %s ], handle successful delivery", s.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				}
				
				nRet = HandleDelSuccess(pMsg);
				if (nRet != 0)
				{
					// logpoint: Message delivery post-processing failure (server: %s, port: %d, msg id: %d)
					CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
												EMSERR_MAIL_COMPONENTS,
												EMS_LOG_OUTBOUND_MESSAGING,
												nRet),
									EMS_STRING_MAILCOMP_DELIVERY_SUCCESS_HANDLER_FAILURE,
									m_pMsgDest->GetDescription().c_str(),
									pMsg->GetMessageID());

					assert(0);
					break; // stop processing
				}
				
				// logpoint: Message successfully delivered to mail server (server: %s, port: %d, msg id: %d)
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
											EMSERR_MAIL_COMPONENTS,
											EMS_LOG_OUTBOUND_MESSAGING,
											ERROR_NO_ERROR),
								EMS_STRING_MAILCOMP_MSG_SENTTO_MAIL_SERVER,
								m_pMsgDest->GetDescription().c_str(),
								pMsg->GetMessageID(),
								m_pMsgDest->GetServerAddress().c_str());
			}

		} // if pMsg

	}

	return nRet;
}

// send a specific message over the socket
const int CSMTPConnection::SendMsg(COutboundMessage* const pMsg, bool& bSafeCont)
{
	int nRet;

	DebugReporter::Instance().DisplayMessage("CSMTPConnection::SendMsg - attempting to get spool file size", DebugReporter::MAIL);

	// get the size of the outbound spool file
	DWORD dwSpoolFileSize = 0;
	nRet = GetSpoolFileSize(pMsg, dwSpoolFileSize);
	if (nRet == 0)
	{
		UINT nSizeK = (dwSpoolFileSize / 1024);
		UINT nSizeLimitK = m_pMsgDest->GetMaxOutboundMsgSize();

		// we know the size of the spool file, so compare it to the 
		// max outbound msg size attribute
		if ((nSizeLimitK != 0) && (nSizeK > nSizeLimitK))
		{
			// Message size (including encoding) exceeded maximum allowable (%dk)\r\n
			AppendTranscript(IDS_STRING_MAILCOMP_TRANSCRIPT_MAX_OUTBOUND_SIZE_EXCEEDED, nSizeLimitK);

			// logpoint: Outbound message ID %d size (%dk, includes encoding) exceeds defined limit of %dk
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_OUTBOUND_MESSAGING,
										ERROR_OUTBOUND_MSG_MAXSIZE_EXCEEDED),
							IDS_STRING_MAILCOMP_OUTBOUND_MSG_SIZE_EXCEEDS_MAX,
							m_pMsgDest->GetDescription().c_str(),
							nSizeK,
							pMsg->GetMessageID(),
							nSizeLimitK);

			return ERROR_OUTBOUND_MSG_MAXSIZE_EXCEEDED;
		}
	}
	else
	{
		// if we can't get the spool file size (should be very rare), continue
		// message processing
		dca::String f;
		f.Format("CSMTPConnection::SendMsg - Could not determine size of outbound spool file: %d", nRet);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	}

	DebugReporter::Instance().DisplayMessage("CSMTPConnection::SendMsg - sending reset command", DebugReporter::MAIL);

	// reset effectively checks the transaction state; if a proper response is not
	// received from the reset command, it's pretty likely that we're in trouble
	nRet = Reset();
	if (nRet != 0)
	{
		dca::String f;
		f.Format("CSMTPConnection::SendMsg - SMTP RSET failure: %d", nRet);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		bSafeCont = false;
		return nRet;
	}

	DebugReporter::Instance().DisplayMessage("CSMTPConnection::SendMsg - sending the spooled message file", DebugReporter::MAIL);

	
	nRet = SendEmail(pMsg);
	if (nRet != 0)
	{
		dca::String f;
		f.Format("CSMTPConnection::SendMsg - Sending the spooled message file failed with error: [ %d ]", nRet);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

		return nRet;
	}

	DebugReporter::Instance().DisplayMessage("CSMTPConnection::SendMsg - successfully sent the message file, returning", DebugReporter::MAIL);

	return 0; // success
}

// generate a message spool file
const int CSMTPConnection::GenerateSpoolFile(COutboundMessage* const pMsg)
{
	// get the spool path
	CMessageIO msgio;
	tstring sSpoolPath;
	if (!msgio.GetOutboundSpoolPath(sSpoolPath, pMsg->GetMessageID()))
	{
		DebugReporter::Instance().DisplayMessage("CSMTPConnection::GenerateSpoolFile - Failed to get outbound spool path", DebugReporter::MAIL);
		return ERROR_FILESYSTEM;
	}

	// here we will set the content for each attachment relating to this
	// specific message; the stored content is actually a pair of marker
	// tags that wrap the full attachment file path, which is then later
	// interpolated with the base64-encoded attachment file content;
	// this prevents us from having to read large attachment files into memory
	USES_CONVERSION;
	for (int i=0; i<pMsg->GetAttachments().GetCount(); i++)
	{
		CAttachment* pAttach = pMsg->GetAttachments().GetAttachment(i);
		assert(pAttach);
		if (pAttach)
		{
			DwString aContent;

			// formulate a special attachment marker, and store it as the
			// attachment content
			aContent.append(sAttachMarkerStart.c_str());
			aContent.append(T2A(pAttach->GetFullpath().c_str()));
			aContent.append(sAttachMarkerEnd.c_str());

			pAttach->SetContent(aContent);
		}
	}

	// merge the header and footer into the message body
	MergeHeaderFooter(pMsg);

	// if we're going to send an 8BITMIME-compliant message, then we'll
	// need to do dot-stuffing, otherwise the message will be Q-P encoded
	// and doesn't require it;
	// NOTE: in order for this to work properly, we must have already retrieved
	// server capabilities (e.g. have successfully negotiated the HELO/EHLO sequence)
	// if (m_esmtp_capabilities.b8BitMIME)
	//	DoDotStuffing(pMsg);
	
	// serialize the message to a string buffer
	DwString msg;
	pMsg->SerializeToString(msg, m_esmtp_capabilities.b8BitMIME);
	
	/////////////////////////////////////////////////////////////////
	// write out the spool file

	// ensure path exists
	TCHAR dir[_MAX_DIR];
	TCHAR drive[_MAX_DRIVE];
	_tsplitpath(sSpoolPath.c_str(), drive, dir, NULL, NULL);
	tstring sDir = tstring(drive) + tstring(dir);
	if (!CUtility::EnsurePathExists(sDir))
	{
		dca::String f;
		f.Format("CSMTPConnection::GenerateSpoolFile - Unable to ensure path exists: %s", sDir.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return ERROR_FILESYSTEM;
	}

	{
		dca::String f;
		dca::String s(sSpoolPath.c_str());
		f.Format("CSMTPConnection::GenerateSpoolFile - Attempting to create spool file [ %s ]", s.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	}

	// create a spool file and open it for writing
	HANDLE  hFile = CreateFile(	sSpoolPath.c_str(),
								GENERIC_WRITE,
								0,
								NULL,
								CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL,
								0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		// logpoint: Failed to create outbound spool file (%s): code %n
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									ERROR_FILESYSTEM),
						IDS_STRING_MAILCOMP_SPOOLFILE_CREATE_FAILED,
						sSpoolPath.c_str(),
						GetLastError());

		LINETRACE(_T("Unable to create spool file: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
		dca::String f;
		f.Format("Unable to create spool file: %d (%s)\n", GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return ERROR_FILESYSTEM;
	}

	DWORD dwWritten;
	size_t posStart = 0;
	while (1)
	{
		// search for the attachment marker start tag
		size_t posCur = msg.find(sAttachMarkerStart.c_str(), posStart);
		if (posCur == DwString::npos)
			break;

		// found an attachment marker start tag, let's find the closing tag
		size_t posEnd = msg.find(sAttachMarkerEnd.c_str(), posCur);
		if (posEnd == DwString::npos)
		{
			// the start marker was found, but the end marker wasn't;
			// this should never happen, actually
			LINETRACE(_T("Failed to find attachment marker end tag\n"));
			assert(0);
			break;
		}

		// the attachment filename exists between the two marker positions
		DwString sFileName = msg.substr(	posCur + sAttachMarkerStart.size(), 
											posEnd - (posCur + sAttachMarkerStart.size()));
	

		// write out everything up to this marker position
		if (!WriteFile(hFile, msg.c_str(), posCur,	&dwWritten,	NULL))
		{
			LINETRACE(_T("Unable to write to spool file: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
			CloseHandle(hFile);
			DeleteFile(sSpoolPath.c_str());
			assert(0);
			return ERROR_FILESYSTEM;
		}

		// remove everything in the msg string up to the end of the end marker
		// position
		msg.erase(posStart, posEnd + sAttachMarkerEnd.size());

		// open the attachment file
		HANDLE hAttach = CreateFile( A2T(sFileName.c_str()), 
									GENERIC_READ, 
									FILE_SHARE_READ, 
									NULL, 
									OPEN_EXISTING, 
									FILE_ATTRIBUTE_NORMAL, 
									NULL);
		if (hAttach == INVALID_HANDLE_VALUE)
		{
			LINETRACE(_T("CreateFile call failed: %d (%s). Unable to open msg attachment file (%s).\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str(), A2T(sFileName.c_str()) );
			CloseHandle(hFile);
			DeleteFile(sSpoolPath.c_str());
			return ERROR_FILESYSTEM;
		}

		// encode the attachment file into base64, with our open spool file as the
		// destination
		CFileEncoder fe;
		int nRet = fe.Encode(hAttach, hFile, CFileEncoder::type_base64);
		if (nRet != 0)
		{
			LINETRACE(_T("Failed to encode the attachment file %s: code %d\n"), sFileName.c_str(), nRet);
			assert(0);
			CloseHandle(hAttach);
			CloseHandle(hFile);
			DeleteFile(sSpoolPath.c_str());
			return nRet;
		}

		CloseHandle(hAttach);
	}

	// write out any remaining portion of the string buffer to the file
	// (e.g. will typically come in the form of a boundary marker)
	if (!WriteFile(hFile, msg.c_str(), msg.size(),	&dwWritten,	NULL))
	{
		LINETRACE(_T("Unable to write to spool file: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
		CloseHandle(hFile);
		DeleteFile(sSpoolPath.c_str());
		assert(0);
		return ERROR_FILESYSTEM;
	}

	CloseHandle(hFile);

	return 0; // success
}

// says hello to the server
const int CSMTPConnection::SetHello()
{
	int nRet;
	USES_CONVERSION;
	char namebuf[WSADESCRIPTION_LEN + 1];

	tstring sHeloHostName = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_VERIFY_HELO_HOST_NAME, _T(""));

	if(sHeloHostName.length() > 0)
	{
		m_mailman.put_HeloHostname(T2A(sHeloHostName.c_str()));
	}
	else
	{
		nRet = gethostname(namebuf, WSADESCRIPTION_LEN + 1);
		if (nRet != SOCKET_ERROR)
		{
			const char * sTemp(namebuf);
			m_mailman.put_HeloHostname(sTemp);
		}
	}
	
	return 0;
}

// compares a response against the expected result
const BOOL CSMTPConnection::GetIsResponseGood(LPCSTR szResponse, const UINT nLower, const UINT nUpper)
{
	// codes between 200 and 399 indicate acceptance; 
	// codes between 400 and 499 indicate temporary rejection; 
	// codes between 500 and 599 indicate permanent rejection.
	UINT nCode = atoi(szResponse);
	if ((nCode >= nLower) && (nCode <= nUpper))
		return TRUE;

	return FALSE;
}

// parse the server capabilities out of the EHLO response lines
void CSMTPConnection::ParseCapabilities(const DwString& sEHLOResponse)
{
	// make a upper-case copy for case-insensitive parsing
	DwString sParse(sEHLOResponse);
	sParse.ConvertToUpperCase();

	size_t pos;

	// 8-bit MIME?
	pos = sParse.find("8BITMIME", 0);
	if (pos != DwString::npos)
		m_esmtp_capabilities.b8BitMIME = TRUE;

	// DSN?
	pos = sParse.find("DSN", 0);
	if (pos != DwString::npos)
		m_esmtp_capabilities.bDsn = TRUE;

	// SIZE supported? if so, what is max inbound msg size?
	pos = sParse.find("SIZE", 0);
	if (pos != DwString::npos)
	{
		m_esmtp_capabilities.bSize = TRUE;
		
		size_t epos = sParse.find("\r\n", pos);
		if (epos != DwString::npos)
		{
			DwString sSize = sParse.substr(pos, epos-pos);
			m_esmtp_capabilities.nSize = atoi(sSize.c_str());
		}
	}

	// auth supported? if so, check for CRAM-MD5 & LOGIN
	size_t apos = sParse.find("AUTH", 0);
	if (apos != DwString::npos)
	{
		size_t md5pos = sParse.find("CRAM-MD5", apos);
		if (md5pos != DwString::npos)
		{
			m_esmtp_capabilities.bSmtpAuthMD5 = TRUE;
		}

		size_t lpos = sParse.find("LOGIN", apos);
		if (lpos != DwString::npos)
		{
			m_esmtp_capabilities.bSmtpAuthLogin = TRUE;
		}
	}
}

// authenticate to the mail server
const int CSMTPConnection::Authenticate()
{
	USES_CONVERSION;
	bool success = false;
	
	m_mailman.put_SmtpUsername(T2A(m_pMsgDest->GetAuthUser().c_str()));
	m_mailman.put_SmtpPassword(T2A(m_pMsgDest->GetAuthPass().c_str()));
	
	success = m_mailman.SmtpAuthenticate();
	if(success != true)
	{
		return 1;
	}
	
	return 0; // successful LOGIN authentication
}

const int CSMTPConnection::AuthenticateOAuth2()
{
	USES_CONVERSION;
	int nRet;
	CODBCQuery query(g_odbcConn);
	TOAuthHosts oh;
	tstring aToken;
	tstring rToken;
	tstring sError;
	int nChrSize=0;
	bool success = false;

	try
	{					
		// Query the OAuth Host
		oh.m_OAuthHostID = m_pMsgDest->GetOAuthHostID();
		oh.Query(query);

		// Check the expiration of the AccessToken		
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		const SYSTEMTIME ateTime = m_pMsgDest->GetAccessTokenExpire();
		TIMESTAMP_STRUCT tsSys;
		long tsSysLen=0;		
		TIMESTAMP_STRUCT tsAte;
		long tsAteLen=0;
		SystemTimeToTimeStamp(sysTime,tsSys);
		SystemTimeToTimeStamp(ateTime,tsAte);
		nRet = OrderTimeStamps( tsSys, tsAte );

		if(nRet != 1)
		{
			// AccessToken expired, get a new one using the RefreshToken
			nRet = RefreshToken();
			if(nRet != 0)
			{
				//refreshing the token failed
				DebugReporter::Instance().DisplayMessage("CSMTPConnection::AuthenticateOAuth2() - Refreshing the token failed", DebugReporter::MAIL);
				return nRet;
			}
		}

	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	catch (CEMSException eex)
	{
		dca::String f;
		dca::String e(eex.GetErrorString());
		f.Format("CSMTPConnection::AuthenticateOAuth2() - EMS Exception occured: %d, %s", eex.GetErrorCode(), e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return eex.GetErrorCode();
	}

	const char * accessToken = T2A(m_pMsgDest->GetAccessToken().c_str());					
	m_oauth2.put_AccessToken(accessToken);

	const char * userName = T2A(m_pMsgDest->GetAuthUser().c_str());
	m_mailman.put_SmtpUsername(userName);
	m_mailman.put_OAuth2AccessToken(m_oauth2.accessToken());
	
	success = m_mailman.SmtpAuthenticate();
    //refresh the accesstoken and try again
	if(!success)
	{
		//refresh the accesstoken and try again
		nRet = RefreshToken();
		if(nRet != 0)
		{
			//refreshing the token failed
			DebugReporter::Instance().DisplayMessage("CSMTPConnection::AuthenticateOAuth2() - Refreshing the token failed", DebugReporter::MAIL);
			return nRet;
		}
		
		m_mailman.put_OAuth2AccessToken(m_oauth2.accessToken());
		success = m_mailman.SmtpAuthenticate();
		if(!success)
		{
			return 1;
		}
	}
	
	return 0;
}

// send the RSET command to mail server
const int CSMTPConnection::Reset()
{
	bool success = false;

	success = m_mailman.SmtpReset();
	if(success != true)
	{
		return 1;
	}
	
	return 0;
}

// initialize the server connect & retrieve initial server response
const int CSMTPConnection::InitConnection()
{
	USES_CONVERSION;	
	int nRet = 0;
	int maxWaitMillisec = m_pMsgDest->GetConnTimeoutSecs()*1000;

	m_mailman.put_SmtpSsl(false);
	m_mailman.put_StartTLS(false);
	m_mailman.put_StartTLSifPossible(false);
	m_mailman.put_SmtpHost(T2A(m_pMsgDest->GetServerAddress().c_str()));
	m_mailman.put_SmtpPort(m_pMsgDest->GetSMTPPort());
	m_mailman.put_ConnectTimeout(maxWaitMillisec);

	bool bRet = m_mailman.SmtpConnect();
	if(!bRet)
	{
		CkString sError;
		m_mailman.get_LastErrorText(sError);
		dca::String f;
		dca::String s(m_pMsgDest->GetServerAddress().c_str());
		dca::String n(m_pMsgDest->GetAuthUser().c_str());
		dca::String r(sError.getString());
		f.Format("CSMTPConnection::InitConnection() - SMTP connection failed (server: %s, account: %s, response: %s)", s.c_str(), n.c_str(), r.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return 1;
	}
	
	return 0; // success
}

const int CSMTPConnection::InitSecureConnection()
{
	USES_CONVERSION;
	
	bool bRet;
    int maxWaitMillisec = m_pMsgDest->GetConnTimeoutSecs()*1000;
	
	m_mailman.put_SslProtocol("TLS 1.0 or higher");
	m_mailman.put_SslAllowedCiphers("best-practices");
	if(m_pMsgDest->GetSslMode() == 1)
	{
		m_mailman.put_SmtpSsl(true);
	}
	else
	{
		m_mailman.put_SmtpSsl(false);
		m_mailman.put_StartTLS(true);			
	}
	m_mailman.put_SmtpHost(T2A(m_pMsgDest->GetServerAddress().c_str()));
	m_mailman.put_SmtpPort(m_pMsgDest->GetSMTPPort());
	m_mailman.put_ConnectTimeout(maxWaitMillisec);
	
	
	bRet = m_mailman.SmtpConnect();
    
	if(!bRet)
	{
		CkString sError;
		m_mailman.get_LastErrorText(sError);
		dca::String f;
		dca::String s(m_pMsgDest->GetServerAddress().c_str());
		dca::String n(m_pMsgDest->GetAuthUser().c_str());
		dca::String r(sError.getString());
		f.Format("CSMTPConnection::InitSecureConnection() - SMTP connection failed (server: %s, account: %s, response: %s)", s.c_str(), n.c_str(), r.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return 1;
	}
	
	return 0; // success
}

// send the quit command to the server
const int CSMTPConnection::Quit()
{
	bool success = false;

	success = m_mailman.CloseSmtpConnection();
	if(success != true)
	{
		return 1;
	}
	
	return 0;
}

// set the spoolfilegenerated flag for a specific outbound message
const int CSMTPConnection::SetSpoolFileGenerated(COutboundMessage* const pMsg, const BOOL bTRUE)
{
	// lock access to the database object
	dca::Lock lock(g_csDB);
	
	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		LONG lTrue = bTRUE;
		BINDPARAM_LONG(query, lTrue);
		
		LONG nOutboundMsgID = pMsg->GetMessageID();
		BINDPARAM_LONG(query, nOutboundMsgID);
		
		query.Execute(	_T("UPDATE OutboundMessageQueue ")
						_T("SET SpoolFileGenerated = ? ")
						_T("WHERE OutboundMessageID = ?"));
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	
	pMsg->SetSpoolFileGenerated(TRUE);

	return 0;
}

// updates the oubound state for a specific message
const int CSMTPConnection::UpdateOutboundMsgState(COutboundMessage* const pMsg)
{
	// lock access to the database object
	dca::Lock lock(g_csDB);
	
	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		LONG nOutboundState = pMsg->GetOutboundMessageState();
		BINDPARAM_LONG(query, nOutboundState);
		
		LONG nOutboundMsgID = pMsg->GetMessageID();
		BINDPARAM_LONG(query, nOutboundMsgID);
		
		query.Execute(	_T("UPDATE OutboundMessages ")
						_T("SET OutboundMessageStateID = ? ")
						_T("WHERE OutboundMessageID = ?"));
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0; // success
}

// sends the mail from command
const int CSMTPConnection::MailFrom(COutboundMessage* const pMsg, const DWORD dwSpoolFileSize)
{
	int nRet;

	// if we know the spool file size and ESMTP SIZE is supported, then report
	// the message size
	if (dwSpoolFileSize != 0 && m_esmtp_capabilities.bSize)
		_snprintf(m_szCmd, DEFAULT_BUFFER_LENGTH, "MAIL FROM:%s SIZE=%d\r\n", pMsg->GetOriginator().GetInetName().c_str(), dwSpoolFileSize);
	else
		_snprintf(m_szCmd, DEFAULT_BUFFER_LENGTH, "MAIL FROM:%s\r\n", pMsg->GetOriginator().GetInetName().c_str());

	nRet = GetCommandResponse(m_szCmd, 250, 252);
	if (nRet != 0)
	{
		LINETRACE(_T("Error encountered during MAIL FROM command (%d)\n"), nRet);
		return nRet;
	}

	return 0;
}

// processes all the receivers of a specific message
const int CSMTPConnection::RcptTo(COutboundMessage* const pMsg)
{
	int nRet = 0;
	bool bDeliveryConf = false;
	CEmailAddressList rejectList;
	CEmailAddressList acceptList;		

	if (pMsg->GetDeliveryConf() == 1)
		bDeliveryConf = true;

	if (pMsg->GetToRecipients().GetCount() == 0 && pMsg->GetCCRecipients().GetCount() == 0 && pMsg->GetBCCRecipients().GetCount() == 0)
		return ERROR_NO_VALID_MSG_RECIPIENTS;
	
	nRet = SendRecipientList(pMsg->GetToRecipients(), bDeliveryConf, rejectList);
	if (nRet)
	{
		DebugReporter::Instance().DisplayMessage("CSMTPConnection::RcptTo - Failed to send TO recipients list to server", DebugReporter::MAIL);	
		return nRet;
	}

	nRet = SendRecipientList(pMsg->GetCCRecipients(), bDeliveryConf, rejectList);
	if (nRet)
	{
		DebugReporter::Instance().DisplayMessage("CSMTPConnection::RcptTo - Failed to send CC recipients list to server", DebugReporter::MAIL);	
		return nRet;
	}

	nRet = SendRecipientList(pMsg->GetBCCRecipients(), bDeliveryConf, rejectList);
	if (nRet)
	{
		DebugReporter::Instance().DisplayMessage("CSMTPConnection::RcptTo - Failed to send BCC recipients list to server", DebugReporter::MAIL);	
		return nRet;
	}

	int nEnableVerification = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_ENABLE_EMAIL_VERIFICATION, 0);
	
	// were there any rctp to failures?
	if (rejectList.GetCount() > 0)
	{
		// did they all fail, or just some of them?
		int nTotal =	pMsg->GetBCCRecipients().GetCount() + 
						pMsg->GetToRecipients().GetCount() +
						pMsg->GetCCRecipients().GetCount();
		if (nTotal <= rejectList.GetCount())
		{
			// they all failed, so consider the entire message as bad
			
			// note: don't generate an alert here, as it will be bounced
			// when the message delivery ultimately fails (e.g. after retry period)

			// return an error (ultimately will flag the message as failed)
			return ERROR_NO_VALID_MSG_RECIPIENTS;
		}
		else
		{
			USES_CONVERSION;

			try
			{
				CEMSString errMsg;
				errMsg.Format(	IDS_RCPT_TO_PARTIAL_FAILURE,									// resource
								m_pMsgDest->GetServerAddress().c_str(),							// server addr
								pMsg->GetAddressListString(rejectList).c_str(),					// rejected				
								A2T(pMsg->GetOriginator().GetInetName().c_str()),				// from
								pMsg->GetToListString().c_str(),								// to
								pMsg->GetCcListString().c_str(),								// cc
								pMsg->GetAddressListString(pMsg->GetBCCRecipients()).c_str(),	// bcc
								A2T(pMsg->GetSubject().GetText().c_str()));						// subject

				// bounce it
				BounceMessage(pMsg, errMsg);
			}
			catch (CEMSException EMSException)
			{
				dca::String f;
				f.Format("CSMTPConnection::RcptTo - Error while formatting EMS string (%s)", EMSException.GetErrorString());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			}
		}
		
		if(nEnableVerification == 1)
		{
	
			int nToCount = pMsg->GetToRecipients().GetCount();
			int i;
			for (i=0; i<nToCount; i++)
			{
				CEmailAddress& eAddress = pMsg->GetToRecipients().Get(i);
				
				int nFailCount = rejectList.GetCount();
				bool bMatch = false;
				for (int x=0; x<nFailCount; x++)
				{
					CEmailAddress& eAdd = rejectList.Get(x);
					try
					{
						dca::String f;
						f.Format("CSMTPConnection::RcptTo - Comparing To: email address (%s) to rejected address (%s)", eAddress.GetInetName().c_str(),eAdd.GetInetName().c_str());
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						if( strcmp( eAddress.GetInetName().c_str(), eAdd.GetInetName().c_str() ) == 0 )
						{
							bMatch = true;
							break;
						}
					}
					catch(...){}
				}
				if(!bMatch)
				{
					dca::String f;
					f.Format("CSMTPConnection::RcptTo - Adding To: email address (%s) to accept list", eAddress.GetInetName().c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					acceptList.Add(eAddress);
				}						
			}
			nToCount = pMsg->GetCCRecipients().GetCount();
			for (int i=0; i<nToCount; i++)
			{
				CEmailAddress& eAddress = pMsg->GetCCRecipients().Get(i);
				
				int nFailCount = rejectList.GetCount();
				bool bMatch = false;
				for (int x=0; x<nFailCount; x++)
				{
					CEmailAddress& eAdd = rejectList.Get(x);
					try
					{
						dca::String f;
						f.Format("CSMTPConnection::RcptTo - Comparing Cc: email address (%s) to rejected address (%s)", eAddress.GetInetName().c_str(),eAdd.GetInetName().c_str());
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						if( strcmp( eAddress.GetInetName().c_str(), eAdd.GetInetName().c_str() ) == 0 )
						{
							bMatch = true;
							break;
						}
					}
					catch(...){}
				}
				if(!bMatch)
				{
					dca::String f;
					f.Format("CSMTPConnection::RcptTo - Adding Cc: email address (%s) to accept list", eAddress.GetInetName().c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					acceptList.Add(eAddress);
				}						
			}
			nToCount = pMsg->GetBCCRecipients().GetCount();
			for (int i=0; i<nToCount; i++)
			{
				CEmailAddress& eAddress = pMsg->GetBCCRecipients().Get(i);
				
				int nFailCount = rejectList.GetCount();
				bool bMatch = false;
				for (int x=0; x<nFailCount; x++)
				{
					CEmailAddress& eAdd = rejectList.Get(x);
					try
					{
						dca::String f;
						f.Format("CSMTPConnection::RcptTo - Comparing Bcc: email address (%s) to rejected address (%s)", eAddress.GetInetName().c_str(),eAdd.GetInetName().c_str());
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						if( strcmp( eAddress.GetInetName().c_str(), eAdd.GetInetName().c_str() ) == 0 )
						{
							bMatch = true;
							break;
						}
					}
					catch(...){}
				}
				if(!bMatch)
				{
					dca::String f;
					f.Format("CSMTPConnection::RcptTo - Adding Bcc: email address (%s) to accept list", eAddress.GetInetName().c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					acceptList.Add(eAddress);
				}						
			}
		}
	}
	else
	{
		if(nEnableVerification == 1)
		{
	
			int nToCount = pMsg->GetToRecipients().GetCount();
			int i;
			for (i=0; i<nToCount; i++)
			{
				CEmailAddress& eAddress = pMsg->GetToRecipients().Get(i);
				dca::String f;
				f.Format("CSMTPConnection::RcptTo - Adding To: email address (%s) to accept list", eAddress.GetInetName().c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				acceptList.Add(eAddress);								
			}
			nToCount = pMsg->GetCCRecipients().GetCount();
			for (i=0; i<nToCount; i++)
			{
				CEmailAddress& eAddress = pMsg->GetCCRecipients().Get(i);
				dca::String f;
				f.Format("CSMTPConnection::RcptTo - Adding Cc: email address (%s) to accept list", eAddress.GetInetName().c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				acceptList.Add(eAddress);								
			}
			nToCount = pMsg->GetBCCRecipients().GetCount();
			for (i=0; i<nToCount; i++)
			{
				CEmailAddress& eAddress = pMsg->GetBCCRecipients().Get(i);
				dca::String f;
				f.Format("CSMTPConnection::RcptTo - Adding Bcc: email address (%s) to accept list", eAddress.GetInetName().c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				acceptList.Add(eAddress);								
			}
		}
	}

	if(nEnableVerification == 1)
	{
		// process the accept and reject lists
		int nUp = UpdateContacts(acceptList,rejectList);	
		if (nUp)
		{
			DebugReporter::Instance().DisplayMessage("CSMTPConnection::RcptTo - Failed to update contacts", DebugReporter::MAIL);			
		}
	}
	
	return 0; // success
}

// update email address status
const int CSMTPConnection::UpdateContacts(CEmailAddressList &aList, CEmailAddressList &rList)
{
	int nEnableVerification = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_ENABLE_EMAIL_VERIFICATION, 0);
	if(nEnableVerification == 1)
	{
		int nDecrement = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_VERIFY_CONTACT_EMAIL_DECREMENT, 0);
		int nIncrement = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_VERIFY_CONTACT_EMAIL_INCREMENT, 0);
		
		CEmailDate cur;
		TIMESTAMP_STRUCT Now = cur.GetSQLTime();

		// lock access to the database object
		dca::Lock lock(g_csDB);

		try
		{
			USES_CONVERSION;
			CODBCQuery query(g_odbcConn);
			int nCount;
			int i;
				
			if(nDecrement > 0)
			{
				nCount = aList.GetCount();
				for (i=0; i<nCount; i++)
				{
					CEmailAddress& eAddress = aList.Get(i);
					tstring sAddress;
					sAddress.assign(A2T(eAddress.GetInetName().c_str()));
					if(sAddress.at(0) == '<')
					{
						sAddress = sAddress.erase(0,1);						
					}
					if(sAddress.at(sAddress.size()-1) == '>')
					{
						sAddress = sAddress.erase(sAddress.size()-1,1);						
					}
					query.Initialize();
					BINDPARAM_TIME_NOLEN( query, Now );
					BINDPARAM_TCHAR_STRING( query, sAddress );
					query.Execute( L"UPDATE PersonalData SET StatusID=(StatusID-1),StatusDate=? "
								L"WHERE StatusID>0 AND DataValue=? AND ContactID NOT IN "
								L"(SELECT ContactID FROM Contacts WHERE IsDeleted=1)" );

				}
			}
			
			if(nIncrement > 0)
			{
				nCount = rList.GetCount();
				for (i=0; i<nCount; i++)
				{
					CEmailAddress& eAddress = rList.Get(i);
					tstring sAddress;
					sAddress.assign(A2T(eAddress.GetInetName().c_str()));
					if(sAddress.at(0) == '<')
					{
						sAddress = sAddress.erase(0,1);						
					}
					if(sAddress.at(sAddress.size()-1) == '>')
					{
						sAddress = sAddress.erase(sAddress.size()-1,1);						
					}
					query.Initialize();
					BINDPARAM_LONG( query, nIncrement );
					BINDPARAM_TIME_NOLEN( query, Now );
					BINDPARAM_TCHAR_STRING( query, sAddress );
					query.Execute( L"UPDATE PersonalData SET StatusID=(StatusID+?),StatusDate=? "
								L"WHERE DataValue=? AND ContactID NOT IN "
								L"(SELECT ContactID FROM Contacts WHERE IsDeleted=1)" );

				}
			}
		}
		catch (ODBCError_t oerr)
		{
			HandleDBError(&oerr);
			return ERROR_DATABASE;
		}
	}

	return 0; // success
}

// sends a command and compares it against an expected result in one function call
const int CSMTPConnection::GetCommandResponse(LPCSTR szCmd, const int nLower, const int nUpper, const BOOL bIsSensitive)
{
	return 0;
}

// sends a specific recipient list and tracks rejected recipients
const int CSMTPConnection::SendRecipientList(CEmailAddressList &eList, const BOOL bDeliveryConf, CEmailAddressList& rejectList)
{
	int nRet;

	USES_CONVERSION;

	// iterate over recipient list
	int nToCount = eList.GetCount();
	for (int i=0; i<nToCount; i++)
	{
		CEmailAddress& eAddress = eList.Get(i);
		
		if(bDeliveryConf == 1 && m_esmtp_capabilities.bDsn)
		{
			_snprintf(m_szCmd, DEFAULT_BUFFER_LENGTH, "RCPT TO: %s NOTIFY=SUCCESS,FAILURE,DELAY\r\n", eAddress.GetInetName().c_str());
		}
		else
		{
			_snprintf(m_szCmd, DEFAULT_BUFFER_LENGTH, "RCPT TO: %s\r\n", eAddress.GetInetName().c_str());
		}
		
		nRet = GetCommandResponse(m_szCmd, 250, 252);
		if (nRet != 0)
		{
			// this specific rcpt command failed, so add this user to the
			// "reject list"
			dca::String f;
			f.Format("CSMTPConnection::SendRecipientList - Adding email address (%s) to reject list", eAddress.GetInetName().c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			rejectList.Add(eAddress);

			// logpoint: SMTP server rejected message recipient (server: %s, port: %d): %s
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_OUTBOUND_MESSAGING,
										nRet),
							EMS_STRING_MAILCOMP_RCPTO_REJECT_WARNING_LOG,
							m_pMsgDest->GetDescription().c_str(),
							m_pMsgDest->GetServerAddress().c_str(),
							A2T(eAddress.GetInetName().c_str()));			
		}
	}

	return 0;
}

// send message data
const int CSMTPConnection::SendEmail(COutboundMessage* const pMsg)
{
	int nRet=0;
	USES_CONVERSION;
	
	// get the spool file location
	CMessageIO msgio;
	tstring sSpoolPath;
	msgio.GetOutboundSpoolPath(sSpoolPath, pMsg->GetMessageID());


	{
		dca::String f;
		dca::String t(sSpoolPath.c_str());
		f.Format("CSMTPConnection::SendData - Got spool file at location [ %s ]. Attempting to open", t.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	}

	// open the spool file (note: if the spool file doesn't exist
	// (even though it should), reset the "spoolfilegenerated" flag in
	// the database and bail
	HANDLE  hFile = CreateFile(	sSpoolPath.c_str(),
								GENERIC_READ,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwErr = GetLastError();

		// logpoint: Failed to open outbound spool file (path: %s, getlasterror: %d, msg id: %d)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									ERROR_FILESYSTEM),
						EMS_STRING_MAILCOMP_OPEN_SPOOLFILE_FAILURE,
						sSpoolPath.c_str(),
						dwErr,
						pMsg->GetMessageID());


		if (dwErr == ERROR_FILE_NOT_FOUND)
		{
			DebugReporter::Instance().DisplayMessage("CSMTPConnection::SendData - Spool file not found reset generate flag", DebugReporter::MAIL);
			// if the spool file isn't found, then the safest course is to
			// regenerate it upon the next pass
			SetSpoolFileGenerated(pMsg, FALSE);
		}

		return ERROR_FILESYSTEM;
	}
	
	{
		dca::String f;
		dca::String t(sSpoolPath.c_str());
		f.Format("CSMTPConnection::SendData - Spool file [ %s ] successfully opened, attempting to close then send", t.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	}
	
	CloseHandle(hFile);
	CkEmail email;

    bool success = email.LoadEml(T2A(sSpoolPath.c_str()));
	if (success != true)
	{
		// logpoint: Failed to load spool file (path: %s, getlasterror: %d, msg id: %d)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									ERROR_FILESYSTEM),
						EMS_STRING_MAILCOMP_SPOOLFILE_READ_FAILURE,
						sSpoolPath.c_str(),
						GetLastError(),
						pMsg->GetMessageID());

		assert(0);
		{
			dca::String f;
			dca::String t(sSpoolPath.c_str());
			f.Format("CSMTPConnection::SendData - Failed to load spool file [ %s ]", t.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		}

		return ERROR_FILESYSTEM;

	}

	CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_SENDING_MSG,
						m_pMsgDest->GetDescription().c_str(),
                        pMsg->GetMessageID(),
						m_pMsgDest->GetServerAddress().c_str());

	m_mailman.put_AutoGenMessageId(false);
	
	//Add any BCC recipients
	if(pMsg->GetBCCRecipients().GetCount() > 0)
	{
		int nBccCount = pMsg->GetBCCRecipients().GetCount();
		for (int i=0; i<nBccCount; i++)
		{
			CEmailAddress& eAddress = pMsg->GetBCCRecipients().Get(i);
			std::string sAddress(eAddress.GetInetName().c_str());
			if(sAddress.find ("<") != string::npos )
			{
    			sAddress.erase(sAddress.find ("<"), 1);
			}
			if(sAddress.find (">") != string::npos )
			{
    			sAddress.erase(sAddress.find (">"), 1);
			}			
			email.AddBcc("",sAddress.c_str());							
		}
	}	

	success = m_mailman.SendEmail(email);
    if (success != true)
	{
        CkString sLog;
		m_mailman.get_SmtpSessionLog(sLog);
		sLog.toCRLF();
		CkStringArray *sLines = sLog.split2("\r\n",false,false,false);
		int nNumLines = sLines->get_Count();
		sLines->GetString(nNumLines-1,sLog);
		delete sLines;
		
		{
			dca::String f;
			dca::String t(sSpoolPath.c_str());
			f.Format("CSMTPConnection::SendEmail - Email [%s] received SMTP response: [%s]", t.c_str(), sLog.getString());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		}

		if(sLog.beginsWith("5"))
		{
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_SMTP_RESPONSE_LOG,
						m_pMsgDest->GetDescription().c_str(),
                        A2T(sLog.getString()),
						m_pMsgDest->GetServerAddress().c_str());
			
			return ERROR_PERMANENT_MSG_DEL_FAILURE;
		}
		else if(sLog.beginsWith("4"))
		{
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_SMTP_RESPONSE_LOG,
						m_pMsgDest->GetDescription().c_str(),
                        A2T(sLog.getString()),
						m_pMsgDest->GetServerAddress().c_str());
			
			return ERROR_TEMP_MSG_DEL_FAILURE;
		}
    }

	return 0;
}

// set the last attempted delivery date/time for a specific message
const int CSMTPConnection::SetLastAttemptedDelivery(COutboundMessage* const pMsg)
{
	CEmailDate cur;	// default constructor sets to current date

	pMsg->SetLastDeliveryAttempt(cur);

	// lock access to the database object
	dca::Lock lock(g_csDB);
	
	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		TIMESTAMP_STRUCT last = cur.GetSQLTime();
		LONG lastLen = 0;
		BINDPARAM_TIME(query, last);
		
		LONG nOutboundMsgID = pMsg->GetMessageID();
		BINDPARAM_LONG(query, nOutboundMsgID);
		
		query.Execute(	_T("UPDATE OutboundMessageQueue ")
						_T("SET LastAttemptedDelivery = ? ")
						_T("WHERE OutboundMessageID = ?"));
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}

// remove a specific message from the outbound message queue
const int CSMTPConnection::DeleteMsgFromQueue(COutboundMessage* const pMsg)
{
	// lock access to the database object
	dca::Lock lock(g_csDB);
	
	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();
		
		LONG nOutboundMsgID = pMsg->GetMessageID();
		BINDPARAM_LONG(query, nOutboundMsgID);
		
		query.Execute(	_T("DELETE ")
						_T("FROM OutboundMessageQueue ")
						_T("WHERE OutboundMessageID = ?"));
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0; // success
}

// merge the header/footer into the message body
void CSMTPConnection::MergeHeaderFooter(COutboundMessage *pMsg)
{
	string sBuf;
	int pos;
	string sTemp2;
	string sTemp3;
	string sTemp;
	dca::String f;
	
	DebugReporter::Instance().DisplayMessage("CSMTPConnection::MergeHeaderFooter - Entering", DebugReporter::MAIL);			
		
	int nUseBody = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_TICKET_TRACKING, 0);
	
	f.Format("CSMTPConnection::MergeHeaderFooter - Ticket Tracking set to [%d]", nUseBody);
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	
	// header
	if (pMsg->GetHeader().GetText().size() > 0)
		sBuf.append(pMsg->GetHeader().GetText());

	// footer and footer location
	if (pMsg->GetFooterLoc() == 1 && pMsg->GetFooter().GetText().size() > 0)
	{
		// find the Original Message tag in the body
		sTemp.assign(pMsg->GetMemoText().GetText());
		if (sTemp.find ("-----Original Message-----") != string::npos)
		{
			pos = sTemp.find("-----Original Message-----") - 2;
			if( pos < 0)
			{
				pos=0;
			}
			if(pos > 0)
			{
				sTemp2.assign(pMsg->GetMemoText().GetText().substr(0,pos));			
			}
			sTemp3.assign(pMsg->GetMemoText().GetText().substr(pos,pMsg->GetMemoText().GetText().size()));
			sBuf.append(sTemp2);
			sBuf.append(pMsg->GetFooter().GetText());
			sBuf.append(sTemp3);
		}		
		else
		{
			// msg body
			sBuf.append(pMsg->GetMemoText().GetText());

			// footer
			if (pMsg->GetFooter().GetText().size() > 0)
				sBuf.append(pMsg->GetFooter().GetText());
		}		
	}
	else
	{
		// msg body
		sBuf.append(pMsg->GetMemoText().GetText());

		// footer
		if (pMsg->GetFooter().GetText().size() > 0)
			sBuf.append(pMsg->GetFooter().GetText());
	}

	if ( nUseBody == 1 )
	{
		UINT nUsePar=0;
		GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseParInSubject"), nUsePar );
		
		dca::String s;
		if(nUsePar)
		{
			s.Format("\r\n\r\n(Ticket tracking data, please do not remove. %d:%d)\r\n",pMsg->GetTicketID(),pMsg->GetMessageID());
		}
		else
		{
			s.Format("\r\n\r\n[Ticket tracking data, please do not remove. %d:%d]\r\n",pMsg->GetTicketID(),pMsg->GetMessageID());
		}
				
		if( strcmp( pMsg->GetMemoText().GetMediaSubType().c_str(), "html" ) == 0 )
		{
			EscapeHTMLAndNewLines( s );
		}
		sBuf.append(s);
	}

	pMsg->GetMemoText().SetText(sBuf, pMsg->GetMemoText().GetCharset());
	
	DebugReporter::Instance().DisplayMessage("CSMTPConnection::MergeHeaderFooter - Leaving", DebugReporter::MAIL);	
}

// delete the spool file associated with a specific message
const int CSMTPConnection::DelSpoolFile(COutboundMessage *pMsg)
{
	// delete the spool file
	CMessageIO msgio;
	tstring sSpoolPath;
	msgio.GetOutboundSpoolPath(sSpoolPath, pMsg->GetMessageID());
	if (!DeleteFile(sSpoolPath.c_str()))
		return ERROR_FILESYSTEM;

	return 0;
}

// handles the failed delivery of a specific message
const int CSMTPConnection::HandleDelFailure(COutboundMessage *const pMsg)
{
	int nRet = 0;
	USES_CONVERSION;
	bool bIsFailurePerm = false;

	// if we already forcibly set this message state to failed, then the
	// failure should be treated as permanent
	if (pMsg->GetOutboundMessageState() == EMS_OUTBOUND_MESSAGE_STATE_FAILED)
		bIsFailurePerm = true;

	// otherwise we need to determine whether the rety period has expired; if so,
	// then the failure is permanent
	else
	{
		// get spooled message re-try period from registry (default is 3 days (72 hours))
		UINT nRetryHours = m_pMsgDest->GetMaxSendRetryHours();

		// how many hours have passed since the message was spooled?
		UINT nDiffSecs = 0;
		SYSTEMTIME stSpooled = pMsg->GetDateSpooled().GetSystemTime();
		SYSTEMTIME curTime;
		GetLocalTime(&curTime);
		if (!CEmailDate::GetDiffSecs(stSpooled, curTime, nDiffSecs))
		{
			LINETRACE(_T("GetDiffSecs failed in CSMTPConnection::HandleDelFailure()\n"));
			assert(0);
			return ERROR_UNKNOWN_ERROR;
		}

		// if the message has been sitting around for more than X hours
		UINT nSpooledHours = nDiffSecs / 3600;
		if (nSpooledHours > nRetryHours)
		{
			bIsFailurePerm = true;

			// flag the message delivery as having failed in the database
			// (won't be re-attempted upon next send interval)
			pMsg->SetOutboundMessageState(EMS_OUTBOUND_MESSAGE_STATE_FAILED);
		}
		else
		{
			pMsg->SetOutboundMessageState(EMS_OUTBOUND_MESSAGE_STATE_RETRY);
		}
	}

	// if failure is permanent, 
	if (bIsFailurePerm == true)
	{
		// if a spool file was generated
		if (pMsg->GetSpoolFileGenerated())
		{
			// delete it
			nRet = DelSpoolFile(pMsg);
			if (nRet != 0)
			{
				LINETRACE(_T("Failed to delete message spool file (MsgID %d; error %d)\n"), pMsg->GetMessageID(), nRet);
				assert(0);

				if (GetLastError() != ERROR_FILE_NOT_FOUND)
					return nRet;
			}

			// reset the spool file generated flag in the outbound message
			// queue; this will ensure that if a failed outbound delivery
			// is ever re-spooled, the spool file will be re-generated
			nRet = SetSpoolFileGenerated(pMsg, FALSE);
			if (nRet != 0)
			{
				assert(0);
				return nRet;
			}
		}

		nRet = UpdateOutboundMsgState(pMsg);
		if (nRet != 0)
		{
			// if we can't flag this message state as "failed", then message
			// delivery will be re-attempted upon next send interval
			LINETRACE(_T("Failed to update outbound message state (MsgID %d; error %d)\n"), pMsg->GetMessageID(), nRet);
			assert(0);
			return nRet;
		}

		// logpoint: Permanent outbound message delivery failure (server: %s, port: %d, msg id: %d)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									ERROR_PERMANENT_MSG_DEL_FAILURE),
						EMS_STRING_MAILCOMP_PERMANENT_MSG_DELIVERY_FAILURE,
						m_pMsgDest->GetDescription().c_str(),
						pMsg->GetMessageID());

		CkString sLog;
		m_mailman.get_SmtpSessionLog(sLog);
		m_sSessionTrans.append(A2T(sLog.getString()));

		// administrative alert: same text as above
		CreateAlert(	EMS_ALERT_EVENT_OUTBOUND_DELIVERY,
						EMS_STRING_MAILCOMP_PERMANENT_MSG_DELIVERY_FAILURE_ALERT,
						m_pMsgDest->GetDescription().c_str(),
						pMsg->GetMessageID(),
						m_sSessionTrans.c_str());

		// bounce message back to the sender; note that we
		// don't want to bounce anything from our internal email system
		// address, as this has the potential to create a message loop
		tstring sSystemEmail = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS, _T("EmailSystem@MailFlow"));
		if (pMsg->GetOriginator().GetInetName().find(T2A(sSystemEmail.c_str()), 0) == string::npos)
		{
			try
			{				
				CEMSString errMsg;
				errMsg.Format(	EMS_STRING_MAILCOMP_FATAL_DELIVERY_TEMPLATE,					// resource
								A2T(pMsg->GetOriginator().GetInetName().c_str()),				// from
								pMsg->GetToListString().c_str(),								// to
								pMsg->GetCcListString().c_str(),								// cc
								pMsg->GetAddressListString(pMsg->GetBCCRecipients()).c_str(),	// bcc
								A2T(pMsg->GetSubject().GetText().c_str()),						// subject
								m_sSessionTrans.c_str());										// session transcript

				// bounce it
				BounceMessage(pMsg, errMsg);
			}
			catch (CEMSException EMSException)
			{
				LINETRACE(_T("Error while formatting EMS string (%s)\n"), EMSException.GetErrorString());
				assert(0);
				// note: notification will not be sent in this case, but should never happen
			}
		}		
	}
	else
	{
		// update the outbound message state in the database to "retry"...
		nRet = UpdateOutboundMsgState(pMsg);

		if (nRet != 0)
		{
			LINETRACE(_T("Failed to update outbound message state (MsgID %d; error %d)\n"), pMsg->GetMessageID(), nRet);
		}

		// logpoint: Temporary outbound message delivery failure (server: %s, port: %d, msg id: %d) - delivery will be re-attempted
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									ERROR_TEMP_MSG_DEL_FAILURE),
						EMS_STRING_MAILCOMP_TEMP_MSG_DELIVERY_FAILURE,
						m_pMsgDest->GetDescription().c_str(),
						pMsg->GetMessageID(),
						m_pMsgDest->GetServerAddress().c_str());

		// note: users are not notified of temporary delivery failures
		// to do: should they be?
	}

	return 0;
}

// handles the successful delivery of a specific message
const int CSMTPConnection::HandleDelSuccess(COutboundMessage *const pMsg)
{
	int nRet = 0;

	// change outbound message state to "sent"
	pMsg->SetOutboundMessageState(EMS_OUTBOUND_MESSAGE_STATE_SENT);
	nRet = UpdateOutboundMsgState(pMsg);
	if (nRet != 0)
	{
		LINETRACE(_T("Failed to update outbound message state (MsgID %d; error %d)\n"), pMsg->GetMessageID(), nRet);
		assert(0);
		return nRet;
	}

	// delete row from queued msgs table
	nRet = DeleteMsgFromQueue(pMsg);
	if (nRet != 0)
	{
		LINETRACE(_T("Failed to remove message from outbound queue (MsgID %d; error %d)\n"), pMsg->GetMessageID(), nRet);
		assert(0);
		return nRet;
	}

	// delete the outbound spool file
	assert(pMsg->GetSpoolFileGenerated());
	nRet = DelSpoolFile(pMsg);
	if (nRet != 0)
	{
		LINETRACE(_T("Failed to delete message spool file (MsgID %d; error %d)\n"), pMsg->GetMessageID(), nRet);
		assert(0);
		return nRet;
	}
	
	// if TicketID=0 set the message IsDeleted flag = 1
	if (pMsg->GetTicketID() == 0)
	{
		
		nRet = DeleteMessage(pMsg);
		if (nRet != 0)
		{
			LINETRACE(_T("Failed to delete message with TicketID of 0 (MsgID %d; error %d)\n"), pMsg->GetMessageID(), nRet);
			assert(0);
			return nRet;
		}
	}

	return 0;
}

// prepares a specific outbound message for delivery
// - gen spool file if needed
// - set the outbound msg state to RETRY if needed
// - set last attempted delivery date to current date/time
const int CSMTPConnection::PrepareOutboundMsg(COutboundMessage *const pMsg)
{
	int nRet = 0;
	
	// if there's no spool file generated, then let's build one 
	// prior to sending
	if (!pMsg->GetSpoolFileGenerated())
	{
		// generate the spool file
		nRet = GenerateSpoolFile(pMsg);
		if (nRet != 0)
		{
			// logpoint: Failed to generate outbound message spool file (Message ID: %d)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_OUTBOUND_MESSAGING,
										nRet),
							IDS_STRING_MAILCOMP_SPOOLFILE_GEN_FAILURE,
							pMsg->GetMessageID());
			return nRet;
		}

		// flag spool file as generated
		nRet = SetSpoolFileGenerated(pMsg, TRUE);
		if (nRet != 0)
		{
			LINETRACE(_T("Failed to set spool file generated flag (MsgID %d; error %d)\n"), pMsg->GetMessageID(), nRet);	
			return nRet;
		}
	}

	// change message delivery status to "in progress" (if it isn't already)
	if (pMsg->GetOutboundMessageState() != EMS_OUTBOUND_MESSAGE_STATE_INPROGRESS)
	{
		pMsg->SetOutboundMessageState(EMS_OUTBOUND_MESSAGE_STATE_INPROGRESS);
		nRet = UpdateOutboundMsgState(pMsg);
		if (nRet != 0)
		{
			LINETRACE(_T("Failed to update outbound message state (MsgID %d; error %d)\n"), pMsg->GetMessageID(), nRet);
			assert(0);
			return nRet;
		}
	}

	// set the "last attempted delivery" field
	nRet = SetLastAttemptedDelivery(pMsg);
	if (nRet != 0)
	{
		LINETRACE(_T("Failed to update date delivery was last attempted (MsgID %d; error %d)\n"), pMsg->GetMessageID(), nRet);
		assert(0);
		return nRet;
	}

	return 0;
}

// handles delivery failure for every message destined for the
// mail server we're trying to connect to
const int CSMTPConnection::HandleDelFailureAll()
{
	int nRet = 0;

	// iterate over messages
	for (	MSG_CONT::iterator cur(m_OutboundMsgs.begin()); 
			cur != m_OutboundMsgs.end(); 
			++cur)
	{
		COutboundMessage* pMsg = (COutboundMessage*)*cur;
		assert(pMsg);
		if (pMsg)
		{
			// handle delivery failure (flags message as failed if needed)
			nRet = HandleDelFailure(pMsg);
			if (nRet != 0)
			{
				LINETRACE(_T("Failed to handle delivery failure: %d\n"), nRet);
				return nRet;
			}
		}
	}

	return 0;
}

void CSMTPConnection::ParseAddress(const wchar_t *addrs, CEmailAddress &eAddr)
{
	assert(addrs);

	USES_CONVERSION;

	DwMailbox rcpt;
	rcpt.FromString(T2A(addrs));
	rcpt.Parse();

	DwString inetName = rcpt.LocalPart();
	inetName += "@";
	inetName += rcpt.Domain();
	inetName.Trim();
	if ((inetName[0] != '<') && 
		(inetName[inetName.size()-1] != '>'))
	{
		inetName.insert(0, "<");
		inetName.append(">");
	}

	DwString personalName, charset;
	rcpt.GetFullName(personalName, charset);	

	int len = (int) personalName.length();
	DwBool has8bitChars = DwFalse;
	for (int i=0; i < len; ++i) 
	{
		int ch = personalName[i] & 0xff;
		if (ch < 32 || 126 < ch) 
		{
			has8bitChars = DwTrue;
			break;
		}
	}
	if( has8bitChars )
	{
		DwString sEncoded;					
		DwEncodeBase64(personalName, sEncoded);
		std::string s(sEncoded.c_str());
		while ( s.find ("\r\n") != string::npos )
		{
    		s.erase ( s.find ("\r\n"), 2 );
		}
		personalName.assign("");
		personalName.append("=?utf-8?B?");
		personalName.append(s.c_str());
		personalName.append("?=");				
	}
	
	eAddr.Set(inetName.c_str(), personalName.c_str(), charset.c_str());
}

const int CSMTPConnection::SetLastProcessed()
{
	LONG lMsgDestID = m_pMsgDest->GetMessageDestID();

	CEmailDate date; // constructor assigns current date
	TIMESTAMP_STRUCT last = date.GetSQLTime();

	m_pMsgDest->SetLastProcessed(date.GetSystemTime());

	// lock access to the database object
	dca::Lock lock(g_csDB);
	
	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		LONG lastLen = 0;
		BINDPARAM_TIME(query, last);
		BINDPARAM_LONG(query, lMsgDestID);

		query.Execute(	_T("UPDATE MessageDestinations ")
						_T("SET LastProcessInterval = ? ")
						_T("WHERE MessageDestinationID = ?"));
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}

const int CSMTPConnection::GetSpoolFileSize(COutboundMessage *const pMsg, DWORD &dwSize)
{
	// get the spool file location
	CMessageIO msgio;
	tstring sSpoolPath;
	msgio.GetOutboundSpoolPath(sSpoolPath, pMsg->GetMessageID());
	
	// open the spool file
	HANDLE  hFile = CreateFile(	sSpoolPath.c_str(),
								GENERIC_READ,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			// if the spool file isn't found, then the safest course is to
			// regenerate it upon the next pass
			SetSpoolFileGenerated(pMsg, FALSE);
		}

		return ERROR_FILESYSTEM;
	}

	DWORD dwFileSizeHigh = 0;
	dwSize = GetFileSize(hFile, &dwFileSizeHigh);
	if (dwSize == INVALID_FILE_SIZE)
	{
		LINETRACE(_T("Warning - spool file size unknown (GetFileSize err code: %d)\n"), GetLastError());
		assert(0);
		CloseHandle(hFile);
		return ERROR_FILESYSTEM;
	}

	// fail if file is greater than 4GB in size, return error
	if (dwFileSizeHigh)
	{
		LINETRACE(_T("CSMTPConnection::GetSpoolFileSize() error: file size is greater than 4GB\n"));
		assert(0);
		CloseHandle(hFile);
		return ERROR_FILESYSTEM;
	}

	CloseHandle(hFile);

	return 0;
}

void CSMTPConnection::AppendTranscript(UINT nResID, ...)
{
	// format the text
	try
	{
		va_list va;
		va_start(va, nResID);	
		CEMSString emsString;
		emsString.FormatArgList(nResID, va);
		va_end(va);

		m_sSessionTrans.append(emsString.c_str());
	}
	catch (CEMSException EMSException)
	{
		LINETRACE(_T("Error during string formatting (%s)\n"), EMSException.GetErrorString());
		assert(0);
	}	
}

void CSMTPConnection::AppendTranscript(const TCHAR* szText, bool bIsServerComm/* = false*/, bool IsServerCommIn/* = false*/ )
{
	// if this transcript lines represents a communication with the
	// server, let's indicate the direction of communications
	if (bIsServerComm)
	{
		if (IsServerCommIn)
			m_sSessionTrans.append(_T("<<< "));
		else
			m_sSessionTrans.append(_T(">>> "));
	}

	m_sSessionTrans.append(szText);
}

void CSMTPConnection::BounceMessage(COutboundMessage *const pMsg, CEMSString& errMsg)
{
	USES_CONVERSION;

	// send a high-priority "bounce message" to the sender of this email;
	// NOTE: bounce messages are inserted directly into the InboundMessages table,
	// effectively bypassing any routing rule processing; also, the MessageSourceID
	// for these entries will always be 0 (zero)

	tstring sSystemEmail = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS, _T("EmailSystem@MailFlow"));

	CInboundMessage in;
	in.SetTicketID(pMsg->GetTicketID());
	in.SetReplyToMsgID(pMsg->GetMessageID());
	in.GetDate().SetCurrent();
	in.GetSubject().SetText("Message Delivery Failure", "ISO-8859-1");
	in.GetToRecipients().Add(pMsg->GetOriginator());
	in.GetReceivedFor() = pMsg->GetOriginator();
	in.GetOriginator().Set(T2A(sSystemEmail.c_str()), "MailFlow Mail Subsystem", "ISO-8859-1");
	in.GetMemoText().SetText(T2A(errMsg.c_str()), "ISO-8859-1");
	in.GetMemoText().SetMediaType("text");
	in.GetMemoText().SetMediaSubType("plain");
	in.SetPriorityID(EMS_PRIORITY_HIGHEST);
	
	int nRet = in.SaveMessageDB(0,true);
	if (nRet == 0)
	{
		nRet = in.CommitSave();		
		// lock access to the database object
		dca::Lock lock(g_csDB);
		CODBCQuery query(g_odbcConn);
		THAddInboundMsg( query, pMsg->GetTicketID(),0, in.GetMessageID(), 0 );
	}

	if (nRet != 0)
	{
		// logpoint: Failed to send message delivery failure notification to sender (sender: %s)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_BOUNCE_FAILED,
						A2T(pMsg->GetOriginator().GetInetName().c_str()));
	}

	// re-open the associated ticket if its closed
	nRet = ReopenTicketIfClosed(pMsg->GetTicketID());
	if (nRet != 0)
	{
		LINETRACE(_T("Failed to re-open ticket (ticketid %d) for bounced message (msgid %d)\n"), pMsg->GetTicketID(), pMsg->GetMessageID());
	}
}

const int CSMTPConnection::GetWaitingMsgCount(LONG& lCount)
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();
		BINDCOL_LONG_NOLEN(query, lCount);

		LONG nMsgDestID = m_pMsgDest->GetMessageDestID();
		BINDPARAM_LONG(query, nMsgDestID);

		query.Execute(_T("SELECT COUNT(*) FROM OutboundMessageQueue AS q ")
				        _T("INNER JOIN MessageDestinations as d ON q.MessageDestinationID = d.MessageDestinationID ")
						_T("INNER JOIN OutboundMessages AS o ON q.OutboundMessageID = o.OutboundMessageID ")
						_T("LEFT OUTER JOIN TicketBoxHeaders as h ON o.TicketBoxHeaderID = h.HeaderID ")
						_T("LEFT OUTER JOIN TicketBoxFooters as f ON o.TicketBoxFooterID = f.FooterID ")
						_T("WHERE o.IsDeleted = 0 AND q.IsApproved = 1 ")
						_T("AND d.IsActive = 1 AND q.MessageDestinationID = ? ")
						_T("AND o.OutboundMessageStateID IN (3, 4, 6)"));

		query.Fetch();

	}
	catch(ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}

void CSMTPConnection::InitTranscript()
{
	m_sSessionTrans = _T("");
}

// re-open a specific ticket only if its current status
// is closed
const int CSMTPConnection::ReopenTicketIfClosed(int nTicketID)
{
	assert(nTicketID > -1);
	if (nTicketID < 0)
		return ERROR_BAD_PARAMETER;	

	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		unsigned int nOldTicketStateID;
		unsigned int nOldPriorityID;

		BINDCOL_LONG_NOLEN(query, nOldTicketStateID);
		BINDCOL_LONG_NOLEN(query, nOldPriorityID);
		BINDPARAM_LONG(query, nTicketID);
		query.Execute(	_T("SELECT TicketStateID,PriorityID ")
						_T("FROM Tickets ")
						_T("WHERE TicketID=?"));

		if( query.Fetch() == S_OK )
		{
			query.Reset(true);
			BINDPARAM_LONG(query, nTicketID);
			query.Execute(	_T("UPDATE	Tickets ")
							_T("SET		TicketStateID = 4,PriorityID=1 ")
							_T("WHERE	TicketID = ?"));

			// log the change in the ticket history
			THChangeState( query, nTicketID, 0, nOldTicketStateID, 4, 0, 0, 0, 0);		

		}		
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// EscapeHTMLAndNewLines
// 
////////////////////////////////////////////////////////////////////////////////
void CSMTPConnection::EscapeHTMLAndNewLines( string& str )
{
	string sNewString;
	sNewString.reserve( (str.size() * 5) / 4 );
	string::iterator pos;
	
	for ( pos = str.begin(); pos < str.end(); pos++ )
	{
		switch( *pos )
		{
		case _T('>'):
			sNewString.append( "&gt;" );
			break;

		case _T('<'):
			sNewString.append( "&lt;" );
			break;

		case _T('\r'):
			sNewString.append( "<br>" );
			break;

		default:
			sNewString.append( 1, *pos );
			break;
		}
	}

	str.assign( sNewString );
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete message
// 
////////////////////////////////////////////////////////////////////////////////
const int CSMTPConnection::DeleteMessage(COutboundMessage* const pMsg)
{
	// lock access to the database object
	dca::Lock lock(g_csDB);
	
	try
	{
		CEmailDate date;
		TIMESTAMP_STRUCT last = date.GetSQLTime();
		LONG lastLen = 0;
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		LONG nOutboundMsgID = pMsg->GetMessageID();
		BINDPARAM_TIME(query, last);
		BINDPARAM_LONG(query, nOutboundMsgID);
		
		query.Execute(	_T("UPDATE OutboundMessages ")
						_T("SET IsDeleted=1, DeletedTime=? ")
						_T("WHERE OutboundMessageID = ?"));
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0; 
}

const int CSMTPConnection::SetErrorCode(LONG nErrorCode)
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{					
		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();
		
		// bind the errorcode param
		LONG nErrorCodeLen = 0;
		BINDPARAM_LONG(query, nErrorCode);
				
		// bind the message destination id param
		int nMsgDestID = m_pMsgDest->GetMessageDestID();
		LONG nMsgDestIDLen = 0;
		BINDPARAM_LONG(query, nMsgDestID);
		
		// execute the query
		query.Execute(	_T("UPDATE	MessageDestinations ")
						_T("SET		ErrorCode = ? ")
						_T("WHERE	MessageDestinationID = ?"));

	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	catch (CEMSException eex)
	{
		dca::String f;
		dca::String e(eex.GetErrorString());
		f.Format("CSMTPConnection::SetErrorCode() - EMS Exception occured: %d, %s", eex.GetErrorCode(), e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return eex.GetErrorCode();
	}

	return 0; // success
}

const int CSMTPConnection::RefreshToken()
{
	// lock access to the database object
	dca::Lock lock(g_csDB);
		
	USES_CONVERSION;
	tstring aToken;
	tstring rToken;
	tstring sError;
	bool success = false;	
	TOAuthHosts oh;
	try
	{					
		CODBCQuery query(g_odbcConn);
	
		// Query the OAuth Host
		oh.m_OAuthHostID = m_pMsgDest->GetOAuthHostID();
		oh.Query(query);
		
		const char * tokenEndPoint = T2A(oh.m_TokenEndPoint);
		m_oauth2.put_TokenEndpoint(tokenEndPoint);

		const char * clientID = T2A(oh.m_ClientID);
		m_oauth2.put_ClientId(clientID);
		
		const char * clientSecret = T2A(oh.m_ClientSecret);			
		m_oauth2.put_ClientSecret(clientSecret);

		const char * refreshToken = T2A(m_pMsgDest->GetRefreshToken().c_str());					
		m_oauth2.put_RefreshToken(refreshToken);

		success = m_oauth2.RefreshAccessToken();
		if (success != true) {
			const char * lastError = m_oauth2.lastErrorText();
			dca::String f;
			dca::String e(lastError);
			f.Format("CSMTPConnection::RefreshToken() - Error occurred during RefreshToken(): %s", e.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			return 1;
		}
		
		aToken.assign(A2T(m_oauth2.accessToken()));
		rToken.assign(A2T(m_oauth2.refreshToken()));

		m_pMsgDest->SetAccessToken(aToken);
		m_pMsgDest->SetRefreshToken(rToken);
		
		time_t now;
		time( &now ); 
		TIMESTAMP_STRUCT tsaTokenExpire;
		long tsaTokenExpireLen=0;
		SecondsToTimeStamp(now + 3599, tsaTokenExpire);

		LONG lMessageDestinationID = m_pMsgDest->GetMessageDestID();
		
		query.Initialize();
		BINDPARAM_TEXT_STRING(query, aToken);
		BINDPARAM_TEXT_STRING(query, rToken);
		BINDPARAM_TIME (query, tsaTokenExpire);
		BINDPARAM_LONG(query, lMessageDestinationID);
		query.Execute(	_T("UPDATE MessageDestinations SET AccessToken=?,RefreshToken=?,AccessTokenExpire=? WHERE MessageDestinationID=?"));
	
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	catch (CEMSException eex)
	{
		dca::String f;
		dca::String e(eex.GetErrorString());
		f.Format("CSMTPConnection::RefreshToken() - EMS Exception occured: %d, %s", eex.GetErrorCode(), e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return eex.GetErrorCode();
	}	
	
	return 0;
}