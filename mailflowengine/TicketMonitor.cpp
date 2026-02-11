#include "stdafx.h"
#include "TicketMonitor.h"
#include "TicketBoxViewFns.h"
#include "InboundMsg.h"
#include "OutboundMsg.h"
#include "PurgeFns.h"
#include "TicketHistoryFns.h"
#include "AttachFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTicketMonitor::CTicketMonitor( CODBCQuery& m_query ) : m_query(m_query)
{
	m_LastAutoReplyCheck = 0;
	m_LastTbClosedAutoReplyCheck = 0;
	m_LastTbCreatedAutoReplyCheck = 0;
	m_LastTcClosedAutoReplyCheck = 0;
	m_LastTcCreatedAutoReplyCheck = 0;
	m_LastAgeAlertCheck = 0;
	m_LastWaterMarkCheck = 0;
	m_LastCustomWaterMarkTbCheck = 0; 
	m_LastCustomWaterMarkTcCheck = 0; 
	m_LastAutoMessageTbClosedCheck = 0;
	m_LastAutoMessageTbCreatedCheck = 0;
	m_LastAutoMessageTcClosedCheck = 0;
	m_LastAutoMessageTcCreatedCheck = 0;
	m_LastAgeAlertTbCheck = 0;	
	m_LastAgeAlertTcCheck = 0;
	m_LastLockedTicketCheck = 0;
	m_LastAutoActionCheck = 0;
}

CTicketMonitor::~CTicketMonitor()
{

}


////////////////////////////////////////////////////////////////////////////////
// 
//  Run
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CTicketMonitor::Run(void)
{
	map<unsigned int,tstring> AlertMsgBody;
	long TickCount = GetTickCount();
	bool bActionTaken = false;

	if ( _wtoi(g_Object.GetParameter( EMS_SRVPARAM_TICKET_MONITOR )) )
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::Run - Entering", DebugReporter::ENGINE);
		
		try
		{
			// Auto Actions
			if( m_LastAutoActionCheck == 0 || abs(TickCount - m_LastAutoActionCheck) > AutoActionCheckIntervalMs )
			{
				ProcessAutoActions();
				m_LastAutoActionCheck = TickCount;
			}
			
			// Auto-Replies
			if( m_LastAutoReplyCheck == 0 || abs(TickCount - m_LastAutoReplyCheck) > AutoReplyCheckIntervalMs )
			{
				if( GetAutoReplyTicket() )
				{
					// handle it
					SendAutoReply();
					bActionTaken = true;
				}
				m_LastAutoReplyCheck = TickCount;
			}

			if( m_LastTbClosedAutoReplyCheck == 0 || abs(TickCount - m_LastTbClosedAutoReplyCheck) > AutoReplyCheckTbClosedIntervalMs )
			{
				if( GetCustomAutoReplyTicket(0) )
				{
					// handle it
					SendCustomAutoReply();
					bActionTaken = true;
				}
				m_LastTbClosedAutoReplyCheck = TickCount;
			}

			if( m_LastTbCreatedAutoReplyCheck == 0 || abs(TickCount - m_LastTbCreatedAutoReplyCheck) > AutoReplyCheckTbCreatedIntervalMs )
			{
				if( GetCustomAutoReplyTicket(1) )
				{
					// handle it
					SendCustomAutoReply();
					bActionTaken = true;
				}
				m_LastTbCreatedAutoReplyCheck = TickCount;
			}

			if( m_LastTcClosedAutoReplyCheck == 0 || abs(TickCount - m_LastTcClosedAutoReplyCheck) > AutoReplyCheckTcClosedIntervalMs )
			{
				if( GetCustomAutoReplyTicket(2) )
				{
					// handle it
					SendCustomAutoReply();
					bActionTaken = true;
				}
				m_LastTcClosedAutoReplyCheck = TickCount;
			}

			if( m_LastTcCreatedAutoReplyCheck == 0 || abs(TickCount - m_LastTcCreatedAutoReplyCheck) > AutoReplyCheckTcCreatedIntervalMs )
			{
				if( GetCustomAutoReplyTicket(3) )
				{
					// handle it
					SendCustomAutoReply();
					bActionTaken = true;
				}
				m_LastTcCreatedAutoReplyCheck = TickCount;
			}

			// Auto Messages
			if( m_LastAutoMessageTbClosedCheck == 0 || abs(TickCount - m_LastAutoMessageTbClosedCheck) > AutoMessageTbClosedCheckIntervalMs )
			{
				if( GetAutoMessage( 0 ) )
				{
					// handle it
					SendAutoMessage();
					bActionTaken = true;
				}
				m_LastAutoMessageTbClosedCheck = TickCount;
			}

			if( m_LastAutoMessageTbCreatedCheck == 0 || abs(TickCount - m_LastAutoMessageTbCreatedCheck) > AutoMessageTbCreatedCheckIntervalMs )
			{
				if( GetAutoMessage( 1 ) )
				{
					// handle it
					SendAutoMessage();
					bActionTaken = true;
				}
				m_LastAutoMessageTbCreatedCheck = TickCount;
			}

			if( m_LastAutoMessageTcClosedCheck == 0 || abs(TickCount - m_LastAutoMessageTcClosedCheck) > AutoMessageTcClosedCheckIntervalMs )
			{
				if( GetAutoMessage( 2 ) )
				{
					// handle it
					SendAutoMessage();
					bActionTaken = true;
				}
				m_LastAutoMessageTcClosedCheck = TickCount;
			}

			if( m_LastAutoMessageTcCreatedCheck == 0 || abs(TickCount - m_LastAutoMessageTcCreatedCheck) > AutoMessageTcCreatedCheckIntervalMs )
			{
				if( GetAutoMessage( 3 ) )
				{
					// handle it
					SendAutoMessage();
					bActionTaken = true;
				}
				m_LastAutoMessageTcCreatedCheck = TickCount;
			}

			// Age Alerts
			if( m_LastAgeAlertCheck == 0 || abs(TickCount - m_LastAgeAlertCheck) > AgeAlertCheckIntervalMs )
			{
				AlertMsgBody.clear();
				if( GetAgedTicket( AlertMsgBody ) )
				{
					// handle it
					HandleAgeAlert( AlertMsgBody );
					bActionTaken = true;
				}
				m_LastAgeAlertCheck = TickCount;
			}

			if( m_LastAgeAlertTbCheck == 0 || abs(TickCount - m_LastAgeAlertTbCheck) > CustomAgeAlertTbCheckIntervalMs )
			{
				AlertMsgBody.clear();
				if( GetCustomAgedTicket( 0, AlertMsgBody ) )
				{
					// handle it
					HandleCustomAgeAlert( AlertMsgBody );
					bActionTaken = true;
				}
				m_LastAgeAlertTbCheck = TickCount;
			}

			if( m_LastAgeAlertTcCheck == 0 || abs(TickCount - m_LastAgeAlertTcCheck) > CustomAgeAlertTcCheckIntervalMs )
			{
				AlertMsgBody.clear();
				if( GetCustomAgedTicket( 1, AlertMsgBody ) )
				{
					// handle it
					HandleCustomAgeAlert( AlertMsgBody );
					bActionTaken = true;
				}
				m_LastAgeAlertTcCheck = TickCount;
			}

			// WaterMarks
			if( m_LastWaterMarkCheck == 0 || abs(TickCount - m_LastWaterMarkCheck) > WaterMarkCheckIntervalMs )
			{
				if(CheckWaterMarks()){bActionTaken = true;}
				m_LastWaterMarkCheck = TickCount;
			}

			if( m_LastCustomWaterMarkTbCheck == 0 || abs(TickCount - m_LastCustomWaterMarkTbCheck) > CustomWaterMarkTbCheckIntervalMs )
			{
				if(CheckCustomWaterMarks(0)){bActionTaken = true;}
				m_LastCustomWaterMarkTbCheck = TickCount;
			}
			
			if( m_LastCustomWaterMarkTcCheck == 0 || abs(TickCount - m_LastCustomWaterMarkTcCheck) > CustomWaterMarkTcCheckIntervalMs )
			{
				if(CheckCustomWaterMarks(1)){bActionTaken = true;}
				m_LastCustomWaterMarkTcCheck = TickCount;
			}
			
			// Locked Tickets
			if( m_LastLockedTicketCheck == 0 || abs(TickCount - m_LastLockedTicketCheck) > LockedTicketCheckIntervalMs )
			{
				if(CheckLockedTickets()){bActionTaken = true;}
				m_LastLockedTicketCheck = TickCount;
			}	
		}
		catch(dca::Exception& e)
		{
			try
			{
				dca::String er;
				dca::String x(e.GetMessage());
				er.Format("CTicketMonitor::Run - %s", x.c_str());
				DebugReporter::Instance().DisplayMessage(er.c_str(), DebugReporter::ENGINE);
				Log( E_DBMonitorError, L"%s\n", e.GetMessage());				
			}
			catch(...)
			{
				DebugReporter::Instance().DisplayMessage("CTicketMonitor::Run - Leaving with Exception", DebugReporter::ENGINE);
			}
		}		
	}

	if(bActionTaken)
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::Run - Leaving - Tickets Processed", DebugReporter::ENGINE);
		return S_OK;
	}
	else
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::Run - Leaving - No Tickets Processed", DebugReporter::ENGINE);
		return S_FALSE;
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetAutoReplyTicket
// 
////////////////////////////////////////////////////////////////////////////////
bool CTicketMonitor::GetAutoReplyTicket( void )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetAutoReplyTicket - Entering", DebugReporter::ENGINE);
	TIMESTAMP_STRUCT Now;
	HRESULT hresult;
	
	GetTimeStamp( Now );

	m_query.Initialize();

	BINDPARAM_TIME_NOLEN( m_query, Now );
	BINDCOL_LONG( m_query, m_Auto_t.m_TicketID );
	BINDCOL_LONG( m_query, m_Auto_t.m_AutoReplyStdRespID );
	BINDCOL_LONG( m_query, m_Auto_t.m_AutoReplyQuoteMsg );
	BINDCOL_LONG( m_query, m_Auto_t.m_AutoReplyCloseTicket );
	BINDCOL_LONG( m_query, m_Auto_t.m_AutoReplyInTicket );
	BINDCOL_TCHAR( m_query, m_Auto_t.m_AutoReplyFrom );
	BINDCOL_TCHAR( m_query, m_Auto_t.m_AutoReplyFromName );
	BINDCOL_TCHAR( m_query, m_Auto_t.m_TicketBoxName );
	BINDCOL_LONG_NOLEN( m_query, m_Auto_t.m_TicketBoxHeaderID );
	BINDCOL_LONG_NOLEN( m_query, m_Auto_t.m_TicketBoxFooterID );
	BINDCOL_TINYINT_NOLEN( m_query, m_Auto_t.m_FooterLocation );

	if( m_query.GetODBCConn()->GetDBMSType() == CODBCConn::MySQL )
	{
		m_query.Execute( L"SELECT TicketID,AutoReplyWithStdResponse,AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket,DefaultEmailAddress,DefaultEmailAddressName,Name,HeaderID,FooterID,FooterLocation "
					     L"FROM Tickets INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
						 L"WHERE IsDeleted=0 AND AutoReplyEnable = 1 "
						 L"AND TicketStateID>1 "
						 L"AND AutoReplied = 0 "
						 L"AND DATE_ADD(Tickets.OpenTimestamp, INTERVAL AutoReplyThreshHoldMins MINUTE) < ? "
						 L"AND (SELECT COUNT(*) FROM OutboundMessages WHERE Tickets.TicketID = OutboundMessages.TicketID AND OutboundMessages.OutboundMessageTypeID=3) = 0 "
					     L"ORDER BY Tickets.TicketID " 
					     L"LIMIT 1" );
	}
	else
	{
		m_query.Execute( L"SELECT TOP 1 TicketID,AutoReplyWithStdResponse,AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket,DefaultEmailAddress,DefaultEmailAddressName,Name,HeaderID,FooterID,FooterLocation "
					     L"FROM Tickets INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
					     L"WHERE IsDeleted=0 AND AutoReplyEnable = 1 "
						 L"AND TicketStateID>1 "
						 L"AND AutoReplied = 0 "
						 L"AND DATEADD(minute,AutoReplyThreshHoldMins,Tickets.OpenTimestamp) < ? "
 						 L"AND (SELECT COUNT(*) FROM OutboundMessages WHERE Tickets.TicketID = OutboundMessages.TicketID AND OutboundMessages.OutboundMessageTypeID=3) = 0 "
					     L"ORDER BY Tickets.DateCreated " );
	}


	if( (hresult = m_query.Fetch()) == S_OK )
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetAutoReplyTicket - Returning true", DebugReporter::ENGINE);
		return true;
	}

	DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetAutoReplyTicket - Returning false", DebugReporter::ENGINE);
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendAutoReply
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketMonitor::SendAutoReply( void )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - Entering", DebugReporter::ENGINE);
	
	CInboundMsg inmsg(m_query);
	COutboundMsg outmsg(m_query);
	TTickets tkt;

	// Send the auto-reply

	if( inmsg.GetFirstMessageInTicket( m_Auto_t.m_TicketID, tkt.m_TicketBoxID ) )
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - inmsg.GetFirstMessageInTicket", DebugReporter::ENGINE);
		
		TCHAR szAutoReplyFrom[OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH];

		DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - inmsg.GetFirstMessageInTicket - set primary to:", DebugReporter::ENGINE);
				
				// Set the Primary To: Address
		if( wcslen( m_Auto_t.m_AutoReplyFromName ) > 0 )
		{
			_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
						L"\"%s\" <%s>", m_Auto_t.m_AutoReplyFromName, m_Auto_t.m_AutoReplyFrom );
		}
		else if( wcslen( m_Auto_t.m_TicketBoxName ) > 0 )
		{
			_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
						L"\"%s\" <%s>", m_Auto_t.m_TicketBoxName, m_Auto_t.m_AutoReplyFrom );
		}
		else
		{
			_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
						L"<%s>", m_Auto_t.m_AutoReplyFrom );
		}

		outmsg.m_OriginalTicketBoxID = tkt.m_TicketBoxID;
		outmsg.m_TicketBoxFooterID = m_Auto_t.m_TicketBoxFooterID;
		outmsg.m_TicketBoxHeaderID = m_Auto_t.m_TicketBoxHeaderID;
		outmsg.m_FooterLocation = m_Auto_t.m_FooterLocation;

		dca::String t;
		t.Format("CTicketMonitor::SendAutoReply - inmsg.GetFirstMessageInTicket - send auto reply:  IMID[%d] SRID[%d] TID[%d] TBID[%d]",  inmsg.m_InboundMessageID, m_Auto_t.m_AutoReplyStdRespID, inmsg.m_TicketID, tkt.m_TicketBoxID);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		
		if( outmsg.AutoReply( m_Auto_t.m_AutoReplyStdRespID, m_Auto_t.m_AutoReplyQuoteMsg, inmsg, szAutoReplyFrom, (m_Auto_t.m_AutoReplyInTicket!=0) ) == false )
			return;
	}
	else
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - !inmsg.GetFirstMessageInTicket - set AutoReplied flag", DebugReporter::ENGINE);
		
		// Set the autoreplied flag
		m_query.Reset(true);

		BINDPARAM_LONG( m_query, m_Auto_t.m_TicketID );

		// Leave the ticket state alone
		m_query.Execute( L"UPDATE Tickets "
						 L"SET AutoReplied = 1 "
						 L"WHERE TicketID = ?" );
		return;
	}
	

	if( m_Auto_t.m_AutoReplyCloseTicket )
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - m_Auto_t.m_AutoReplyCloseTicket - get ticket details", DebugReporter::ENGINE);
		
		m_query.Reset(true);
		
		BINDCOL_LONG( m_query, tkt.m_TicketStateID );
		BINDCOL_LONG( m_query, tkt.m_OpenMins );
		BINDCOL_TIME( m_query, tkt.m_OpenTimestamp );
		
		BINDPARAM_LONG( m_query, m_Auto_t.m_TicketID );
		
		m_query.Execute( _T("SELECT TicketStateID, OpenMins, OpenTimestamp FROM Tickets WHERE TicketID = ?" ));

		// [MER] - we weren't doing a fetch here, so the query above never returned
		// a result
		m_query.Fetch(); // if the fetch fails, so will the following condition
						
		// if the ticket wasn't "on hold" calculate the minutes
		// the ticket was open...
		if ( tkt.m_TicketStateID == EMS_TICKETSTATEID_ESCALATED 
			|| tkt.m_TicketStateID == EMS_TICKETSTATEID_OPEN )
		{
			DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - m_Auto_t.m_AutoReplyCloseTicket - update open mins and close", DebugReporter::ENGINE);
		
			TIMESTAMP_STRUCT CurrentTime;

			GetTimeStamp(CurrentTime);
			
			int nLastOpenSecs = tkt.m_OpenTimestampLen != SQL_NULL_DATA ? CompareTimeStamps( tkt.m_OpenTimestamp, CurrentTime ) : 0;
			
			if (nLastOpenSecs > 0)
			{
				// update the open minute count
				tkt.m_OpenMins += (nLastOpenSecs / 60);

				// round to the next minute?
				if ( (nLastOpenSecs % 60) >= 30 )
					tkt.m_OpenMins++;
			}
			
			m_query.Reset(true);

			BINDPARAM_LONG( m_query, tkt.m_OpenMins );
			BINDPARAM_LONG( m_query, m_Auto_t.m_TicketID );
			
			m_query.Execute( L"UPDATE Tickets "
							 L"SET AutoReplied=1,TicketStateID=1,OpenMins=? "
							 L"WHERE TicketID=?" );
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - m_Auto_t.m_AutoReplyCloseTicket - close", DebugReporter::ENGINE);
		
			// clear the timestamp
			m_query.Reset(true);

			BINDPARAM_LONG( m_query, m_Auto_t.m_TicketID );

			// Close the ticket
			m_query.Execute( L"UPDATE Tickets "
							 L"SET AgeAlerted=0,AutoReplied=1,TicketStateID=1 "
							 L"WHERE TicketID = ?" );
		}

		DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - m_Auto_t.m_AutoReplyCloseTicket - remove custom age alerts", DebugReporter::ENGINE);
		
		//Remove any custom age alert records, if they exist
		m_query.Reset(true);
		BINDPARAM_LONG( m_query, m_Auto_t.m_TicketID );		
		m_query.Execute( L"DELETE FROM AgeAlertsSent WHERE TicketID = ?" );

	}
	else
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - !m_Auto_t.m_AutoReplyCloseTicket - set AutoReplied flag", DebugReporter::ENGINE);
		
		// Set the autoreplied flag
		m_query.Reset(true);

		BINDPARAM_LONG( m_query, m_Auto_t.m_TicketID );

		// Leave the ticket state alone
		m_query.Execute( L"UPDATE Tickets "
						 L"SET AutoReplied = 1 "
						 L"WHERE TicketID = ?" );
	}

	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - add to ticket history", DebugReporter::ENGINE);
	// Add to ticket history
	THAddOutboundMsg( m_query, m_Auto_t.m_TicketID, 0, outmsg.m_OutboundMessageID, m_Auto_t.m_TicketBoxID );
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoReply - Leaving", DebugReporter::ENGINE);
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetCustomAutoReplyTicket
// 
////////////////////////////////////////////////////////////////////////////////
bool CTicketMonitor::GetCustomAutoReplyTicket( int nGetType )
{
	dca::String t;
	t.Format("CTicketMonitor::GetCustomAutoReplyTicket For Type [%d] - Entering",  nGetType);
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

	TIMESTAMP_STRUCT Now;
	bool bRet = false;
	
	GetTimeStamp( Now );

	Auto_t mAuto;

	m_query.Initialize();

	BINDPARAM_TIME_NOLEN( m_query, Now );
	BINDCOL_LONG( m_query, mAuto.m_TicketID );
	BINDCOL_LONG( m_query, mAuto.m_AutoReplyStdRespID );
	BINDCOL_LONG( m_query, mAuto.m_AutoReplyQuoteMsg );
	BINDCOL_LONG( m_query, mAuto.m_AutoReplyCloseTicket );
	BINDCOL_LONG( m_query, mAuto.m_AutoReplyInTicket );
	BINDCOL_TCHAR( m_query, mAuto.m_AutoReplyFrom );
	BINDCOL_TCHAR( m_query, mAuto.m_AutoReplyFromName );
	BINDCOL_TCHAR( m_query, mAuto.m_TicketBoxName );
	BINDCOL_LONG_NOLEN( m_query, mAuto.m_TicketBoxHeaderID );
	BINDCOL_LONG_NOLEN( m_query, mAuto.m_TicketBoxFooterID );
	BINDCOL_TINYINT_NOLEN( m_query, mAuto.m_FooterLocation );
	BINDCOL_LONG( m_query, mAuto.m_SendFromTypeID );
	BINDCOL_TCHAR( m_query, mAuto.m_NewReplyToAddress );
	BINDCOL_TCHAR( m_query, mAuto.m_NewReplyToName );
	BINDCOL_TCHAR( m_query, mAuto.m_Subject );
	BINDCOL_LONG( m_query, mAuto.m_SendToPercent );
	BINDCOL_LONG( m_query, mAuto.m_OmitTracking );
	BINDCOL_LONG( m_query, mAuto.m_AutoResponseID );
	BINDCOL_LONG( m_query, mAuto.m_HitCount );
	BINDCOL_LONG( m_query, mAuto.m_FailCount );
	BINDCOL_LONG( m_query, mAuto.m_TotalCount );	
	
	switch( nGetType )
	{
	case 0:
		{
			m_query.Execute( L"SELECT t.TicketID,ar.SrToSendID,ar.AutoReplyQuoteMsg,ar.AutoReplyCloseTicket,ar.AutoReplyInTicket, "
			L"tb.DefaultEmailAddress,tb.DefaultEmailAddressName,tb.Name,ar.HeaderID,ar.FooterID,ar.FooterLocation, "
			L"ar.SendFromTypeID,ar.ReplyToAddress,ar.ReplyToName,sr.Subject, "
			L"ar.SendToPercent,ar.OmitTracking,ar.AutoResponseID,ar.HitCount,ar.FailCount,ar.TotalCount "
			L"FROM Tickets t INNER JOIN TicketBoxes tb ON t.TicketBoxID = tb.TicketBoxID "
			L"INNER JOIN AutoResponses ar ON tb.TicketBoxID = ar.SendToID "
			L"INNER JOIN StandardResponses sr ON sr.StandardResponseID = ar.SrToSendID "
			L"INNER JOIN TicketHistory th ON t.TicketID=th.TicketID " 
			L"WHERE t.IsDeleted=0 AND t.TicketStateID=1 "
			L"AND ar.SendToTypeID=0 AND ar.IsEnabled=1 AND ar.WhenToSendTypeID=1 "
			L"AND th.TicketActionID=2 AND th.ID1=3 AND th.ID2=2 "
			L"AND th.DateTime>DATEADD(minute,-ar.WhenToSendVal,ar.DateEdited) " 
			L"AND DATEADD(minute,ar.WhenToSendVal,th.DateTime)< ? " 
			L"AND t.TicketID NOT IN (SELECT TicketID FROM AutoResponsesSent WHERE TicketID=t.TicketID AND AutoResponseID=ar.AutoResponseID) "
 			L"ORDER BY t.TicketID " );			
		}
		break;
	case 1:
		{
			m_query.Execute( L"SELECT Tickets.TicketID,AutoResponses.SrToSendID,AutoResponses.AutoReplyQuoteMsg,AutoResponses.AutoReplyCloseTicket,AutoResponses.AutoReplyInTicket, "
			L"TicketBoxes.DefaultEmailAddress,TicketBoxes.DefaultEmailAddressName,TicketBoxes.Name,AutoResponses.HeaderID,AutoResponses.FooterID,AutoResponses.FooterLocation, "
			L"AutoResponses.SendFromTypeID,AutoResponses.ReplyToAddress,AutoResponses.ReplyToName,StandardResponses.Subject, "
			L"AutoResponses.SendToPercent,AutoResponses.OmitTracking,AutoResponses.AutoResponseID,AutoResponses.HitCount,AutoResponses.FailCount,AutoResponses.TotalCount "
			L"FROM Tickets INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
			L"INNER JOIN AutoResponses ON TicketBoxes.TicketBoxID = AutoResponses.SendToID "
			L"INNER JOIN StandardResponses ON StandardResponses.StandardResponseID = AutoResponses.SrToSendID "
			L"WHERE Tickets.IsDeleted=0 "
			L"AND Tickets.TicketStateID>1 "
			L"AND AutoResponses.SendToTypeID=0 "
			L"AND AutoResponses.IsEnabled=1 "
			L"AND Tickets.DateCreated > DATEADD(minute,-AutoResponses.WhenToSendVal,AutoResponses.DateEdited) "
			L"AND DATEADD(minute,AutoResponses.WhenToSendVal,Tickets.DateCreated) < ? "
			L"AND AutoResponses.WhenToSendTypeID=0 "
			L"AND Tickets.TicketID NOT IN (SELECT TicketID FROM AutoResponsesSent WHERE TicketID=Tickets.TicketID AND AutoResponseID=AutoResponses.AutoResponseID) "
 			L"ORDER BY Tickets.TicketID " );
		}
		break;
	case 2:
		{
			m_query.Execute( L"SELECT t.TicketID,ar.SrToSendID,ar.AutoReplyQuoteMsg,ar.AutoReplyCloseTicket,ar.AutoReplyInTicket, "
			L"tb.DefaultEmailAddress,tb.DefaultEmailAddressName,tb.Name,ar.HeaderID,ar.FooterID,ar.FooterLocation, "
			L"ar.SendFromTypeID,ar.ReplyToAddress,ar.ReplyToName,sr.Subject, "
			L"ar.SendToPercent,ar.OmitTracking,ar.AutoResponseID,ar.HitCount,ar.FailCount,ar.TotalCount "
			L"FROM Tickets t INNER JOIN TicketBoxes tb ON t.TicketBoxID = tb.TicketBoxID "
			L"INNER JOIN TicketCategories tc ON t.TicketCategoryID = tc.TicketCategoryID "
			L"INNER JOIN AutoResponses ar ON tc.TicketCategoryID = ar.SendToID "
			L"INNER JOIN StandardResponses sr ON sr.StandardResponseID = ar.SrToSendID "
			L"INNER JOIN TicketHistory th ON t.TicketID=th.TicketID " 
			L"WHERE t.IsDeleted=0 AND t.TicketStateID=1 "
			L"AND ar.SendToTypeID=1 AND ar.IsEnabled=1 AND ar.WhenToSendTypeID=1 "
			L"AND th.TicketActionID=2 AND th.ID1=3 AND th.ID2=2 "
			L"AND th.DateTime>DATEADD(minute,-ar.WhenToSendVal,ar.DateEdited) " 
			L"AND DATEADD(minute,ar.WhenToSendVal,th.DateTime)< ? " 
			L"AND t.TicketID NOT IN (SELECT TicketID FROM AutoResponsesSent WHERE TicketID=t.TicketID AND AutoResponseID=ar.AutoResponseID) "
 			L"ORDER BY t.TicketID " );			
		}
		break;
	case 3:
		{
			m_query.Execute( L"SELECT Tickets.TicketID,AutoResponses.SrToSendID,AutoResponses.AutoReplyQuoteMsg,AutoResponses.AutoReplyCloseTicket,AutoResponses.AutoReplyInTicket, "
			L"TicketBoxes.DefaultEmailAddress,TicketBoxes.DefaultEmailAddressName,TicketBoxes.Name,AutoResponses.HeaderID,AutoResponses.FooterID,AutoResponses.FooterLocation, "
			L"AutoResponses.SendFromTypeID,AutoResponses.ReplyToAddress,AutoResponses.ReplyToName,StandardResponses.Subject, "
			L"AutoResponses.SendToPercent,AutoResponses.OmitTracking,AutoResponses.AutoResponseID,AutoResponses.HitCount,AutoResponses.FailCount,AutoResponses.TotalCount "
			L"FROM Tickets INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
			L"INNER JOIN TicketCategories ON Tickets.TicketCategoryID = TicketCategories.TicketCategoryID "
			L"INNER JOIN AutoResponses ON TicketCategories.TicketCategoryID = AutoResponses.SendToID "
			L"INNER JOIN StandardResponses ON StandardResponses.StandardResponseID = AutoResponses.SrToSendID "
			L"WHERE Tickets.IsDeleted=0 "
			L"AND Tickets.TicketStateID>1 "
			L"AND AutoResponses.SendToTypeID=1 "
			L"AND AutoResponses.IsEnabled=1 "
			L"AND Tickets.DateCreated > DATEADD(minute,-AutoResponses.WhenToSendVal,AutoResponses.DateEdited) "
			L"AND DATEADD(minute,AutoResponses.WhenToSendVal,Tickets.DateCreated) < ? "
			L"AND AutoResponses.WhenToSendTypeID=0 "
			L"AND Tickets.TicketID NOT IN (SELECT TicketID FROM AutoResponsesSent WHERE TicketID=Tickets.TicketID AND AutoResponseID=AutoResponses.AutoResponseID) "
 			L"ORDER BY Tickets.TicketID " );
		}
		break;
	}
	
	m_a.clear();

	while (m_query.Fetch() == S_OK)
	{
		bRet = true;
		m_a.push_back(mAuto);
	}
	
	if(bRet)
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetCustomAutoReplyTicket - Returning true", DebugReporter::ENGINE);
		return true;
	}
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetCustomAutoReplyTicket - Returning false", DebugReporter::ENGINE);
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendCustomAutoReply
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketMonitor::SendCustomAutoReply( void )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendCustomAutoReply - Entering", DebugReporter::ENGINE);

	for( aIter = m_a.begin(); aIter != m_a.end(); aIter++ )
	{
		bool nSentOk = true;
		bool nSendIt = true;
		int nFailCode=0; //0=OK, 1=NoInboundMessageInTicket, 3=SkippedDueToPercentage, 4=SendFailedOther
		COutboundMsg outmsg(m_query);
		CInboundMsg inmsg(m_query);
		
		TTickets tkt;
		TCHAR szAutoReplyFrom[OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH];
		TIMESTAMP_STRUCT Now;
		GetTimeStamp( Now );
		int nNewTicketID;
		
		double nPercentSent = ((double)aIter->m_HitCount/aIter->m_TotalCount)*100;
		
		if ( nPercentSent != 0 )
		{
			if ( nPercentSent > (double)aIter->m_SendToPercent )
			{
				nSendIt = false;
			}				
		}
			
		if ( nSendIt )
		{
			if( inmsg.GetFirstMessageInTicket( aIter->m_TicketID, tkt.m_TicketBoxID ) )
			{
				
				// Set the From: Address

				switch ( aIter->m_SendFromTypeID )
				{
				case 0:
					{
						if( wcslen( aIter->m_AutoReplyFromName ) > 0 )
						{
							_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
										L"\"%s\" <%s>", aIter->m_AutoReplyFromName, aIter->m_AutoReplyFrom );
						}
						else
						{
							_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
										L"\"%s\" <%s>", aIter->m_TicketBoxName, aIter->m_AutoReplyFrom );
						}
					}
					break;
				case 1:
					{
						if( wcslen( aIter->m_TicketBoxName ) > 0 )
						{
							_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
										L"\"%s\" <%s>", aIter->m_TicketBoxName, inmsg.m_EmailPrimaryTo );
						}
						else
						{
							_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
										L"<%s>", inmsg.m_EmailPrimaryTo );
						}	
					}
					break;
				case 2:
					{
						if( wcslen( aIter->m_NewReplyToName ) > 0 )
						{
							_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
										L"\"%s\" <%s>", aIter->m_NewReplyToName, aIter->m_NewReplyToAddress );
						}
						else
						{
							_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
										L"<%s>", aIter->m_NewReplyToAddress );
						}	
					}
					break;
				}

				if ( wcslen( szAutoReplyFrom ) == 0 )
				{
					if( wcslen( inmsg.m_EmailFromName ) > 0 )
					{
						_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
									L"\"%s\" <%s>", inmsg.m_EmailFromName, inmsg.m_EmailFrom );
					}
					else
					{
						_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
									L"<%s>", inmsg.m_EmailFrom );
					}	
				}
					
				outmsg.m_OriginalTicketBoxID = tkt.m_TicketBoxID;
				outmsg.m_TicketCategoryID = tkt.m_TicketCategoryID;
				outmsg.m_TicketBoxFooterID = aIter->m_TicketBoxFooterID;
				outmsg.m_FooterLocation = aIter->m_FooterLocation;
				outmsg.m_TicketBoxHeaderID = aIter->m_TicketBoxHeaderID;
				outmsg.m_TicketID = inmsg.m_TicketID;
				outmsg.m_AgentID = 0;
				
				wcscpy( outmsg.m_Subject, aIter->m_Subject );
						
				if( outmsg.AutoReply( aIter->m_AutoReplyStdRespID, aIter->m_AutoReplyQuoteMsg, inmsg, szAutoReplyFrom, (aIter->m_AutoReplyInTicket!=0), aIter->m_OmitTracking ) == false )
				{
					nSentOk = false;
					nFailCode = 4;
				}
				
			}
			else
			{
				nSentOk = false;
				nFailCode = 1;
			}
		}
		else
		{
			nSentOk = false;
			nFailCode = 3;
		}
			
		//Mark this message as sent
		m_query.Initialize();
		BINDPARAM_LONG( m_query, aIter->m_AutoResponseID );
		BINDPARAM_LONG( m_query, aIter->m_TicketID );
		BINDPARAM_LONG( m_query, nFailCode );
		m_query.Execute( L"INSERT INTO AutoResponsesSent (AutoResponseID,TicketID,ResultCode) VALUES (?,?,?)" );
			
		if ( nSentOk )
		{
			//Up the hit count
			aIter->m_HitCount = aIter->m_HitCount + 1;
			// Add to ticket history
			THAddOutboundMsg( m_query, aIter->m_TicketID, 0, outmsg.m_OutboundMessageID, aIter->m_TicketBoxID );

			if( aIter->m_AutoReplyCloseTicket )
			{
				m_query.Reset(true);
				
				BINDCOL_LONG( m_query, tkt.m_TicketStateID );
				BINDCOL_LONG( m_query, tkt.m_OpenMins );
				BINDCOL_TIME( m_query, tkt.m_OpenTimestamp );
				
				BINDPARAM_LONG( m_query, aIter->m_TicketID );
				
				m_query.Execute( _T("SELECT TicketStateID, OpenMins, OpenTimestamp FROM Tickets WHERE TicketID = ?" ));

				// [MER] - we weren't doing a fetch here, so the query above never returned
				// a result
				m_query.Fetch(); // if the fetch fails, so will the following condition
								
				// if the ticket wasn't "on hold" calculate the minutes
				// the ticket was open...
				if ( tkt.m_TicketStateID == EMS_TICKETSTATEID_ESCALATED 
					|| tkt.m_TicketStateID == EMS_TICKETSTATEID_OPEN )
				{
					TIMESTAMP_STRUCT CurrentTime;

					GetTimeStamp(CurrentTime);
					
					int nLastOpenSecs = tkt.m_OpenTimestampLen != SQL_NULL_DATA ? CompareTimeStamps( tkt.m_OpenTimestamp, CurrentTime ) : 0;
					
					if (nLastOpenSecs > 0)
					{
						// update the open minute count
						tkt.m_OpenMins += (nLastOpenSecs / 60);

						// round to the next minute?
						if ( (nLastOpenSecs % 60) >= 30 )
							tkt.m_OpenMins++;
					}
					
					m_query.Reset(true);

					BINDPARAM_LONG( m_query, tkt.m_OpenMins );
					BINDPARAM_LONG( m_query, aIter->m_TicketID );
					
					m_query.Execute( L"UPDATE Tickets "
									L"SET AgeAlerted=0,AutoReplied=1,TicketStateID=1,OpenMins=? "
									L"WHERE TicketID=?" );
				}
				else
				{
					// clear the timestamp
					m_query.Reset(true);

					BINDPARAM_LONG( m_query, aIter->m_TicketID );

					// Close the ticket
					m_query.Execute( L"UPDATE Tickets "
									L"SET AgeAlerted=0,AutoReplied=1,TicketStateID=1 "
									L"WHERE TicketID = ?" );
				}
				
				//Remove any custom age alert records, if they exist
				m_query.Reset(true);
				BINDPARAM_LONG( m_query, aIter->m_TicketID );		
				m_query.Execute( L"DELETE FROM AgeAlertsSent WHERE TicketID = ?" );
			}
		}
		else
		{
			//Up the fail count
			aIter->m_FailCount = aIter->m_FailCount + 1;
		}
		
		//Update the counts
		aIter->m_TotalCount = aIter->m_TotalCount + 1;
		m_query.Initialize();
		BINDPARAM_LONG( m_query, aIter->m_HitCount );
		BINDPARAM_LONG( m_query, aIter->m_FailCount );
		BINDPARAM_LONG( m_query, aIter->m_TotalCount );
		BINDPARAM_LONG( m_query, aIter->m_AutoResponseID );
		m_query.Execute( L"UPDATE AutoResponses SET HitCount=?,FailCount=?,TotalCount=? WHERE AutoResponseID=?");
	}
	

	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendCustomAutoReply - Leaving", DebugReporter::ENGINE);

	return;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetAutoMessage
// 
////////////////////////////////////////////////////////////////////////////////
bool CTicketMonitor::GetAutoMessage( int nGetType )
{
	dca::String t;
	t.Format("CTicketMonitor::GetAutoMessage For Type [%d] - Entering",  nGetType);
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	
	TIMESTAMP_STRUCT Now;
	bool bRet = false;
	
	GetTimeStamp( Now );

	Auto_t mAuto;

	m_query.Initialize();

	BINDPARAM_TIME_NOLEN( m_query, Now );
	BINDCOL_LONG( m_query, mAuto.m_TicketID );
	BINDCOL_LONG( m_query, mAuto.m_AutoReplyStdRespID );
	BINDCOL_TCHAR( m_query, mAuto.m_AutoReplyFrom );
	BINDCOL_TCHAR( m_query, mAuto.m_AutoReplyFromName );
	BINDCOL_TCHAR( m_query, mAuto.m_TicketBoxName );
	BINDCOL_LONG_NOLEN( m_query, mAuto.m_TicketBoxHeaderID );
	BINDCOL_LONG_NOLEN( m_query, mAuto.m_TicketBoxFooterID );
	BINDCOL_TINYINT_NOLEN( m_query, mAuto.m_FooterLocation );
	BINDCOL_LONG( m_query, mAuto.m_SendFromTypeID );
	BINDCOL_LONG( m_query, mAuto.m_CreateNewTicket );
	BINDCOL_TCHAR( m_query, mAuto.m_NewReplyToAddress );
	BINDCOL_TCHAR( m_query, mAuto.m_NewReplyToName );
	BINDCOL_LONG( m_query, mAuto.m_NewOwnerID );
	BINDCOL_LONG( m_query, mAuto.m_NewTicketBoxID );
	BINDCOL_LONG( m_query, mAuto.m_NewTicketCategoryID );
	BINDCOL_TCHAR( m_query, mAuto.m_Subject );
	BINDCOL_LONG( m_query, mAuto.m_SendToPercent );
	BINDCOL_LONG( m_query, mAuto.m_OmitTracking );
	BINDCOL_LONG( m_query, mAuto.m_AutoMessageID );
	BINDCOL_LONG( m_query, mAuto.m_HitCount );
	BINDCOL_LONG( m_query, mAuto.m_FailCount );
	BINDCOL_LONG( m_query, mAuto.m_TotalCount );
		
	switch( nGetType )
	{
	case 0:
		{
			m_query.Execute( L"SELECT t.TicketID,am.SrToSendID,tb.DefaultEmailAddress,tb.DefaultEmailAddressName,tb.Name, " 
			L"tb.HeaderID,tb.FooterID,tb.FooterLocation,am.SendFromTypeID,am.CreateNewTicket,am.ReplyToAddress,am.ReplyToName, "
			L"am.OwnerID,am.TicketBoxID,am.TicketCategoryID,sr.Subject,am.SendToPercent,am.OmitTracking,am.AutoMessageID,am.HitCount,am.FailCount,am.TotalCount " 
			L"FROM Tickets t INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID " 
			L"INNER JOIN AutoMessages am ON tb.TicketBoxID=am.SendToID " 
			L"INNER JOIN StandardResponses sr ON sr.StandardResponseID=am.SrToSendID " 
			L"INNER JOIN TicketHistory th ON t.TicketID=th.TicketID " 
			L"WHERE t.IsDeleted=0 AND t.TicketStateID=1 " 
			L"AND am.SendToTypeID=0 AND am.IsEnabled=1 AND am.WhenToSendTypeID=1 " 
			L"AND th.TicketActionID=2 AND th.ID1=3 AND th.ID2=2 "
			L"AND th.DateTime>DATEADD(minute,-am.WhenToSendVal,am.DateEdited) " 
			L"AND DATEADD(minute,am.WhenToSendVal,th.DateTime)< ? " 
			L"AND t.TicketID NOT IN (SELECT TicketID FROM AutoMessagesSent WHERE TicketID=T.TicketID AND AutoMessageID=am.AutoMessageID) " 
			L"ORDER BY t.TicketID " );

		}
		break;
	case 1:
		{
			m_query.Execute( L"SELECT TicketID,AutoMessages.SrToSendID,TicketBoxes.DefaultEmailAddress,TicketBoxes.DefaultEmailAddressName,Name, "
			L"HeaderID,FooterID,FooterLocation,AutoMessages.SendFromTypeID,AutoMessages.CreateNewTicket,AutoMessages.ReplyToAddress,AutoMessages.ReplyToName, "
			L"AutoMessages.OwnerID,AutoMessages.TicketBoxID,AutoMessages.TicketCategoryID,StandardResponses.Subject,AutoMessages.SendToPercent,AutoMessages.OmitTracking,AutoMessages.AutoMessageID,AutoMessages.HitCount,AutoMessages.FailCount,AutoMessages.TotalCount "
			L"FROM Tickets INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
			L"INNER JOIN AutoMessages ON TicketBoxes.TicketBoxID = AutoMessages.SendToID "
			L"INNER JOIN StandardResponses ON StandardResponses.StandardResponseID = AutoMessages.SrToSendID "
			L"WHERE Tickets.IsDeleted=0 "
			L"AND Tickets.TicketStateID>1 "
			L"AND AutoMessages.SendToTypeID=0 "
			L"AND AutoMessages.IsEnabled=1 "
			L"AND Tickets.DateCreated > DATEADD(minute,-AutoMessages.WhenToSendVal,AutoMessages.DateEdited) "
			L"AND DATEADD(minute,AutoMessages.WhenToSendVal,Tickets.DateCreated) < ? "
			L"AND AutoMessages.WhenToSendTypeID=0 "
			L"AND Tickets.TicketID NOT IN (SELECT TicketID FROM AutoMessagesSent WHERE TicketID=Tickets.TicketID AND AutoMessageID=AutoMessages.AutoMessageID) "
 			L"ORDER BY Tickets.TicketID " );
		}
		break;
	case 2:
		{
			m_query.Execute( L"SELECT t.TicketID,am.SrToSendID,tb.DefaultEmailAddress,tb.DefaultEmailAddressName,tb.Name, " 
			L"tb.HeaderID,tb.FooterID,tb.FooterLocation,am.SendFromTypeID,am.CreateNewTicket,am.ReplyToAddress,am.ReplyToName, "
			L"am.OwnerID,am.TicketBoxID,am.TicketCategoryID,sr.Subject,am.SendToPercent,am.OmitTracking,am.AutoMessageID,am.HitCount,am.FailCount,am.TotalCount " 
			L"FROM Tickets t INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID " 
			L"INNER JOIN TicketCategories tc ON t.TicketCategoryID = tc.TicketCategoryID "
			L"INNER JOIN AutoMessages am ON tc.TicketCategoryID = am.SendToID "
			L"INNER JOIN StandardResponses sr ON sr.StandardResponseID=am.SrToSendID " 
			L"INNER JOIN TicketHistory th ON t.TicketID=th.TicketID " 
			L"WHERE t.IsDeleted=0 AND t.TicketStateID=1 " 
			L"AND am.SendToTypeID=1 AND am.IsEnabled=1 AND am.WhenToSendTypeID=1 " 
			L"AND th.TicketActionID=2 AND th.ID1=3 AND th.ID2=2 "
			L"AND th.DateTime>DATEADD(minute,-am.WhenToSendVal,am.DateEdited) " 
			L"AND DATEADD(minute,am.WhenToSendVal,th.DateTime)< ? " 
			L"AND t.TicketID NOT IN (SELECT TicketID FROM AutoMessagesSent WHERE TicketID=T.TicketID AND AutoMessageID=am.AutoMessageID) " 
			L"ORDER BY t.TicketID " );
		}
		break;
	case 3:
		{
			m_query.Execute( L"SELECT TicketID,AutoMessages.SrToSendID,TicketBoxes.DefaultEmailAddress,TicketBoxes.DefaultEmailAddressName,Name, "
			L"HeaderID,FooterID,FooterLocation,AutoMessages.SendFromTypeID,AutoMessages.CreateNewTicket,AutoMessages.ReplyToAddress,AutoMessages.ReplyToName, "
			L"AutoMessages.OwnerID,AutoMessages.TicketBoxID,AutoMessages.TicketCategoryID,StandardResponses.Subject,AutoMessages.SendToPercent,AutoMessages.OmitTracking,AutoMessages.AutoMessageID,AutoMessages.HitCount,AutoMessages.FailCount,AutoMessages.TotalCount "
			L"FROM Tickets INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
			L"INNER JOIN TicketCategories ON Tickets.TicketCategoryID = TicketCategories.TicketCategoryID "
			L"INNER JOIN AutoMessages ON TicketCategories.TicketCategoryID = AutoMessages.SendToID "
			L"INNER JOIN StandardResponses ON StandardResponses.StandardResponseID = AutoMessages.SrToSendID "
			L"WHERE Tickets.IsDeleted=0 "
			L"AND Tickets.TicketStateID>1 "
			L"AND AutoMessages.SendToTypeID=1 "
			L"AND AutoMessages.IsEnabled=1 "
			L"AND Tickets.DateCreated > DATEADD(minute,-AutoMessages.WhenToSendVal,AutoMessages.DateEdited) "
			L"AND DATEADD(minute,AutoMessages.WhenToSendVal,Tickets.DateCreated) < ? "
			L"AND AutoMessages.WhenToSendTypeID=0 "
			L"AND Tickets.TicketID NOT IN (SELECT TicketID FROM AutoMessagesSent WHERE TicketID=Tickets.TicketID AND AutoMessageID=AutoMessages.AutoMessageID) "
 			L"ORDER BY Tickets.TicketID " );
		}
		break;
	}
		
	m_a.clear();

	while (m_query.Fetch() == S_OK)
	{
		bRet = true;
		m_a.push_back(mAuto);
	}
	
	if(bRet)
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetAutoMessage - Returning true", DebugReporter::ENGINE);
		return true;
	}

	DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetAutoMessage - Returning false", DebugReporter::ENGINE);
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendAutoMessage
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketMonitor::SendAutoMessage( void )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoMessage - Entering", DebugReporter::ENGINE);

	for( aIter = m_a.begin(); aIter != m_a.end(); aIter++ )
	{
		bool nSentOk = true;
		bool nSendIt = true;
		int nFailCode=0; //0=OK, 1=NoInboundMessageInTicket, 2=ContactSetToNoMessage, 3=SkippedDueToPercentage, 4=SendFailedOther
		COutboundMsg outmsg(m_query);
		CInboundMsg inmsg(m_query);
		
		TTickets tkt;
		TCHAR szAutoReplyFrom[OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH];
		TIMESTAMP_STRUCT Now;
		GetTimeStamp( Now );
		int nNewTicketID;
		
		double nPercentSent = ((double)aIter->m_HitCount/aIter->m_TotalCount)*100;
		
		if ( nPercentSent != 0 )
		{
			if ( nPercentSent > (double)aIter->m_SendToPercent )
			{
				nSendIt = false;
			}				
		}
			
		if ( nSendIt )
		{
			if( inmsg.GetFirstMessageInTicket( aIter->m_TicketID, tkt.m_TicketBoxID ) )
			{
				//Check to see if we should send to this contact
				bool nCanSend = GetContactSendMessage( inmsg.m_ContactID );

				if ( nCanSend )
				{
					// Set the From: Address

					switch ( aIter->m_SendFromTypeID )
					{
					case 0:
						{
							if( wcslen( aIter->m_AutoReplyFromName ) > 0 )
							{
								_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
											L"\"%s\" <%s>", aIter->m_AutoReplyFromName, aIter->m_AutoReplyFrom );
							}
							else
							{
								_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
											L"\"%s\" <%s>", aIter->m_TicketBoxName, aIter->m_AutoReplyFrom );
							}
						}
						break;
					case 1:
						{
							if( wcslen( aIter->m_TicketBoxName ) > 0 )
							{
								_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
											L"\"%s\" <%s>", aIter->m_TicketBoxName, inmsg.m_EmailPrimaryTo );
							}
							else
							{
								_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
											L"<%s>", inmsg.m_EmailPrimaryTo );
							}	
						}
						break;
					case 2:
						{
							if( wcslen( aIter->m_NewReplyToName ) > 0 )
							{
								_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
											L"\"%s\" <%s>", aIter->m_NewReplyToName, aIter->m_NewReplyToAddress );
							}
							else
							{
								_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
											L"<%s>", aIter->m_NewReplyToAddress );
							}	
						}
						break;
					}

					if ( wcslen( szAutoReplyFrom ) == 0 )
					{
						if( wcslen( inmsg.m_EmailFromName ) > 0 )
						{
							_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
										L"\"%s\" <%s>", inmsg.m_EmailFromName, inmsg.m_EmailFrom );
						}
						else
						{
							_snwprintf( szAutoReplyFrom, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
										L"<%s>", inmsg.m_EmailFrom );
						}	
					}

					if ( aIter->m_CreateNewTicket == 1 )
					{
						if( wcslen(inmsg.m_EmailFromName) > 0 )
						{
							_snwprintf( newTicket.m_Contacts, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
										L"%s", inmsg.m_EmailFromName );
						}
						else
						{
							_snwprintf( newTicket.m_Contacts, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH,
										L"%s", inmsg.m_EmailFrom );
						}
						
						newTicket.m_LockedBy = 0;
						newTicket.m_OwnerID = aIter->m_NewOwnerID;
						newTicket.m_IsDeleted = 0;
						newTicket.m_TicketCategoryID = aIter->m_NewTicketCategoryID;
						newTicket.m_TicketBoxID = aIter->m_NewTicketBoxID;
						wcscpy( newTicket.m_Subject, aIter->m_Subject );
						newTicket.m_DateCreated = Now;
						newTicket.m_DateCreatedLen = sizeof( newTicket.m_DateCreated );
						newTicket.m_OpenTimestamp = Now;
						newTicket.m_OpenTimestampLen = sizeof( newTicket.m_OpenTimestamp );
						newTicket.m_TicketStateID = 1;
						newTicket.m_PriorityID = 3;
						newTicket.m_AutoReplied = 1;

						newTicket.Insert( m_query );
						
						outmsg.m_OriginalTicketBoxID = aIter->m_NewTicketBoxID;
						outmsg.m_TicketCategoryID = aIter->m_NewTicketCategoryID;
						outmsg.m_TicketID = newTicket.m_TicketID;
						outmsg.m_AgentID = aIter->m_NewOwnerID;
						nNewTicketID = newTicket.m_TicketID;
									
						m_query.Initialize();
						BINDPARAM_LONG( m_query, aIter->m_NewTicketBoxID);
						BINDCOL_LONG_NOLEN( m_query, outmsg.m_TicketBoxHeaderID );
						BINDCOL_LONG_NOLEN( m_query, outmsg.m_TicketBoxFooterID );
						m_query.Execute( L"SELECT HeaderID,FooterID FROM TicketBoxes WHERE TicketBoxID=? "); 
						
					}
					else
					{
						outmsg.m_OriginalTicketBoxID = tkt.m_TicketBoxID;
						outmsg.m_TicketCategoryID = tkt.m_TicketCategoryID;
						outmsg.m_TicketBoxFooterID = aIter->m_TicketBoxFooterID;
						outmsg.m_FooterLocation = aIter->m_FooterLocation;
						outmsg.m_TicketBoxHeaderID = aIter->m_TicketBoxHeaderID;
						outmsg.m_TicketID = inmsg.m_TicketID;
						outmsg.m_AgentID = 0;					
					}
					
					wcscpy( outmsg.m_Subject, aIter->m_Subject );
						
					if( outmsg.AutoMessage( aIter->m_AutoReplyStdRespID, inmsg, szAutoReplyFrom, aIter->m_CreateNewTicket, aIter->m_OmitTracking ) == false )
					{
						if ( aIter->m_CreateNewTicket == 1 )
						{
							m_query.Initialize();
							BINDPARAM_LONG( m_query, nNewTicketID );
							m_query.Execute( L"DELETE FROM TicketFieldsTicket WHERE TicketID=?");

							m_query.Reset( true );
							BINDPARAM_LONG( m_query, nNewTicketID );
							m_query.Execute( L"DELETE FROM Tickets WHERE TicketID=?"); 
						}
						nSentOk = false;
						nFailCode = 4;
					}
				}
				else
				{
					nSentOk = false;
					nFailCode = 2;
				}
			}
			else
			{
				nSentOk = false;
				nFailCode = 1;
			}
		}
		else
		{
			nSentOk = false;
			nFailCode = 3;
		}

		//Mark this message as sent
		m_query.Initialize();
		BINDPARAM_LONG( m_query, aIter->m_AutoMessageID );
		BINDPARAM_LONG( m_query, aIter->m_TicketID );
		BINDPARAM_LONG( m_query, nFailCode );
		m_query.Execute( L"INSERT INTO AutoMessagesSent (AutoMessageID,TicketID,ResultCode) VALUES (?,?,?)" );
			
		if ( nSentOk )
		{
			//Up the hit count
			aIter->m_HitCount = aIter->m_HitCount + 1;
			// Add to ticket history
			THAddOutboundMsg( m_query, aIter->m_TicketID, 0, outmsg.m_OutboundMessageID, aIter->m_TicketBoxID );
		}
		else
		{
			//Up the fail count
			aIter->m_FailCount = aIter->m_FailCount + 1;
		}
		
		//Update the counts
		aIter->m_TotalCount = aIter->m_TotalCount + 1;
		m_query.Initialize();
		BINDPARAM_LONG( m_query, aIter->m_HitCount );
		BINDPARAM_LONG( m_query, aIter->m_FailCount );
		BINDPARAM_LONG( m_query, aIter->m_TotalCount );
		BINDPARAM_LONG( m_query, aIter->m_AutoMessageID );
		m_query.Execute( L"UPDATE AutoMessages SET HitCount=?,FailCount=?,TotalCount=? WHERE AutoMessageID=?");
		
	}
	
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAutoMessage - Leaving", DebugReporter::ENGINE);
	return;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetContactSendMessage
// 
////////////////////////////////////////////////////////////////////////////////
bool CTicketMonitor::GetContactSendMessage( int nContactID )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetContactSendMessage - Entering", DebugReporter::ENGINE);

	long m_szSendMessageLen;
		
	// query for contact send message value
	m_query.Reset();
	BINDPARAM_LONG( m_query, nContactID );
	BINDCOL_WCHAR( m_query, m_szSendMessage );

	m_query.Execute( _T("SELECT PersonalData.DataValue ")
					 _T("FROM PersonalData INNER JOIN Contacts ON Contacts.ContactID = PersonalData.ContactID ")
					 _T("INNER JOIN PersonalDataTypes ON PersonalDataTypes.PersonalDataTypeID = PersonalData.PersonalDataTypeID ")
					 _T("WHERE Contacts.ContactID=? AND PersonalDataTypes.TypeName='Send Auto Message'") );
	m_query.Fetch();

	if ( m_query.GetRowCount() > 0 )
	{
		unsigned int i=0;
		unsigned int x=m_szSendMessageLen;

		for ( i = 0; i < x; i++ )
		{
			m_szSendMessage[i] = tolower(m_szSendMessage[i]);
		}
		
		//Copy to a tstring and look for "no"
		tstring sSendMessage = m_szSendMessage;
		tstring::size_type pos = sSendMessage.find( _T("no") );
		if( pos != tstring::npos )
		{
			DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetContactSendMessage - Returning false", DebugReporter::ENGINE);
			return false;
		}		
	}
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetContactSendMessage - Returning true", DebugReporter::ENGINE);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetCustomAgedTicket
// 
////////////////////////////////////////////////////////////////////////////////
bool CTicketMonitor::GetCustomAgedTicket( int nGetType, map<unsigned int,tstring>& AlertMsgBody )
{
	dca::String t;
	t.Format("CTicketMonitor::GetCustomAgedTicket For Type [%d] - Entering",  nGetType);
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	
	map<unsigned int,tstring>::iterator iter;
	TIMESTAMP_STRUCT Now;
	CEMSString sMsgInfo;
	TCHAR szSubject[TICKETS_SUBJECT_LENGTH];
	tstring sSubject;
	int nTicketBoxID;
	int nTicketID;
	

	m_caa.clear();

	GetTimeStamp( Now );

	m_query.Initialize();

	BINDPARAM_TIME_NOLEN( m_query, Now );
	BINDCOL_LONG( m_query, caa.m_TicketID );
	BINDCOL_LONG( m_query, caa.m_TicketBoxID );
	BINDCOL_TCHAR_NOLEN( m_query, szSubject );
	BINDCOL_LONG( m_query, caa.m_AgeAlertID );
	BINDCOL_LONG( m_query, caa.m_AlertOnTypeID );
		
	if ( nGetType == 0 )
	{
		m_query.Execute( L"SELECT Tickets.TicketID,Tickets.TicketBoxID,Tickets.Subject,AgeAlerts.AgeAlertID "
				     L"FROM Tickets INNER JOIN AgeAlerts ON Tickets.TicketBoxID = AgeAlerts.AlertOnID "
					 L"WHERE Tickets.IsDeleted=0 AND AgeAlerts.AlertOnTypeID=0 AND Tickets.TicketStateID>1 AND AgeAlerts.IsEnabled=1 "
					 L"AND Tickets.TicketID NOT IN (SELECT TicketID FROM AgeAlertsSent WHERE TicketID=Tickets.TicketID AND AgeAlertID=AgeAlerts.AgeAlertID) "
					 L"AND DATEADD([minute], AgeAlerts.ThresholdMins,Tickets.OpenTimestamp) < ? "
 					 L"ORDER BY AgeAlerts.AgeAlertID,Tickets.TicketID" );
	}
	else
	{
		m_query.Execute( L"SELECT Tickets.TicketID,Tickets.TicketCategoryID,Tickets.Subject,AgeAlerts.AgeAlertID "
				     L"FROM Tickets INNER JOIN AgeAlerts ON Tickets.TicketCategoryID = AgeAlerts.AlertOnID "
					 L"WHERE Tickets.IsDeleted=0 AND AgeAlerts.AlertOnTypeID=1 AND Tickets.TicketStateID>1 AND AgeAlerts.IsEnabled=1 "
					 L"AND Tickets.TicketID NOT IN (SELECT TicketID FROM AgeAlertsSent WHERE TicketID=Tickets.TicketID AND AgeAlertID=AgeAlerts.AgeAlertID) "
					 L"AND DATEADD([minute], AgeAlerts.ThresholdMins,Tickets.OpenTimestamp) < ? "
 					 L"ORDER BY AgeAlerts.AgeAlertID,Tickets.TicketID" );

	}
	
	while( m_query.Fetch() == S_OK )
	{
		//m_AgeAlertedTickets.insert( pair<unsigned int,unsigned int> (m_TicketBoxID, m_TicketID) );
		m_caa.push_back( caa );
		nTicketBoxID = caa.m_TicketBoxID;
		nTicketID = caa.m_TicketID;

		// Replace all comma's with periods so we won't break the parsing
		sSubject.assign( szSubject );
		int pos = 0;
		while ( (pos = sSubject.find( _T(','), pos)) != tstring::npos )
		{
			sSubject.replace( pos, 1, 1, _T('.') );		
		}

		sMsgInfo.Format( _T("Ticket #%d %s\r\n"), nTicketID, sSubject.c_str() );

		iter = AlertMsgBody.find( nTicketBoxID );
		if( iter != AlertMsgBody.end() )
		{
			iter->second.append( sMsgInfo.c_str() );
		}
		else
		{
			AlertMsgBody[nTicketBoxID] = (tstring) sMsgInfo;
		}
	}

	if(m_caa.size() > 0){DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetCustomAgedTicket - Returning true", DebugReporter::ENGINE);}
	else{DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetCustomAgedTicket - Returning false", DebugReporter::ENGINE);}

	return (m_caa.size() > 0);
}

////////////////////////////////////////////////////////////////////////////////
// 
// HandleCustomAgeAlert
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketMonitor::HandleCustomAgeAlert( map<unsigned int,tstring>& AlertMsgBody )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::HandleCustomAgeAlert - Entering", DebugReporter::ENGINE);

	multimap<unsigned int,unsigned int>::iterator iter;
	int TicketBoxID = 0;
	int nTickets = 0;
	int nPriorityID;
	int nTicketBoxID;
	int nTicketID;
	int nAgeAlertID;
	int nAlertOnTypeID;
	int nAgeAlertSentID;
	long nAgeAlertSentIDLen;
	bool nAgeAlertSent = false;

	// Send the alerts

	for( caaIter = m_caa.begin(); caaIter != m_caa.end(); caaIter++ )
	{
		nTicketBoxID = caaIter->m_TicketBoxID;
		nTicketID = caaIter->m_TicketID;
	
		
		if( nTicketBoxID != TicketBoxID )
		{
			if( TicketBoxID != 0 )
			{
				SendCustomAgeAlert( nAlertOnTypeID, nAgeAlertID, TicketBoxID, nTickets, AlertMsgBody[TicketBoxID] );
				//Update the hit count
				m_query.Reset(true);
				BINDPARAM_LONG( m_query, nAgeAlertID );
				m_query.Execute( L"UPDATE AgeAlerts SET HitCount=HitCount+1 WHERE AgeAlertID=?");
			}
			TicketBoxID = nTicketBoxID;
			nTickets = 0;
		}
		nAlertOnTypeID = caaIter->m_AlertOnTypeID;
		nAgeAlertID = caaIter->m_AgeAlertID;
		nTickets++;		

		// Get the old ticket priority
		m_query.Reset(true);
		BINDPARAM_LONG( m_query, nTicketID );
		BINDCOL_LONG_NOLEN( m_query, nPriorityID );
		m_query.Execute( L"SELECT PriorityID FROM Tickets WHERE TicketID = ? " );
		m_query.Fetch();


		// change the ticket priority
		m_query.Reset(true);
		BINDPARAM_LONG( m_query, nTicketID );
		m_query.Execute( L"UPDATE Tickets "
						 L"SET PriorityID = 1 "
						 L"WHERE TicketID = ? " );

		//update or create the AgeAlertsSent record
		m_query.Reset(true);
		BINDPARAM_LONG( m_query, nTicketID );
		BINDPARAM_LONG( m_query, nAgeAlertID );
		BINDCOL_LONG( m_query, nAgeAlertSentID );
		
		m_query.Execute( L"SELECT AgeAlertSentID FROM AgeAlertsSent WHERE TicketID=? AND AgeAlertID=?" );
		if( m_query.Fetch() == S_OK )
		{
			nAgeAlertSent = true;
		}
				
		if ( nAgeAlertSent )
		{
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, nTicketID );
			BINDPARAM_LONG( m_query, nAgeAlertID );
			BINDPARAM_LONG( m_query, nAgeAlertSentID );
			m_query.Execute( L"UPDATE AgeAlertsSent "
							L"SET TicketID=?,AgeAlertID=? "
							L"WHERE AgeAlertSentID = ? " );
		}
		else
		{
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, nTicketID );
			BINDPARAM_LONG( m_query, nAgeAlertID );
			m_query.Execute( L"INSERT INTO AgeAlertsSent (TicketID,AgeAlertID) VALUES (?,?)" );
		}

		nAgeAlertSent = false;
		
		// Log it in TicketHistory
		THAgeAlert( m_query, nTicketID, nPriorityID, nTicketBoxID );
	}
	
	if( nTickets > 0 )
	{
		SendCustomAgeAlert( nAlertOnTypeID, nAgeAlertID, TicketBoxID, nTickets, AlertMsgBody[TicketBoxID] );
		
		//Update the hit count
		m_query.Reset(true);
		BINDPARAM_LONG( m_query, nAgeAlertID );
		m_query.Execute( L"UPDATE AgeAlerts SET HitCount=HitCount+1 WHERE AgeAlertID=?");
	}
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::HandleCustomAgeAlert - Leaving", DebugReporter::ENGINE);
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendCustomAgeAlert
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketMonitor::SendCustomAgeAlert( int nAlertOnTypeID, int nAgeAlertID, int TicketBoxID, int nTickets, tstring& msgBody )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendCustomAgeAlert - Entering", DebugReporter::ENGINE);

	TCHAR szName[TICKETBOXES_NAME_LENGTH];
	long szNameLen;
	
	if ( nAlertOnTypeID == 0 )
	{
		m_query.Reset(true);
		BINDCOL_TCHAR( m_query, szName );
		BINDPARAM_LONG( m_query, TicketBoxID );
		m_query.Execute( L"SELECT Name FROM TicketBoxes WHERE TicketBoxID=? " );
		if( m_query.Fetch() == S_OK )
		{
			CustomAlert( EMS_ALERT_EVENT_AGE_ALERT, nAgeAlertID, 
				_T("%d ticket(s) in the %s TicketBox have been open longer than the age alerting threshold.\n\n%s"),
				nTickets, szName, msgBody.c_str() );		
		}
	}
	else
	{
		m_query.Reset(true);
		BINDCOL_TCHAR( m_query, szName );
		BINDPARAM_LONG( m_query, TicketBoxID );
		m_query.Execute( L"SELECT Description FROM TicketCategories WHERE TicketCategoryID=? " );
		if( m_query.Fetch() == S_OK )
		{
			CustomAlert( EMS_ALERT_EVENT_AGE_ALERT, nAgeAlertID, 
				_T("%d ticket(s) in the %s TicketBox have been open longer than the age alerting threshold.\n\n%s"),
				nTickets, szName, msgBody.c_str() );		
		}
	}
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendCustomAgeAlert - Leaving", DebugReporter::ENGINE);
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetAgedTicket
// 
////////////////////////////////////////////////////////////////////////////////
bool CTicketMonitor::GetAgedTicket( map<unsigned int,tstring>& AlertMsgBody )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetAgedTicket - Entering", DebugReporter::ENGINE);

	map<unsigned int,tstring>::iterator iter;
	TIMESTAMP_STRUCT Now;
	CEMSString sMsgInfo;
	TCHAR szSubject[TICKETS_SUBJECT_LENGTH];
	tstring sSubject;

	m_AgeAlertedTickets.clear();

	GetTimeStamp( Now );

	m_query.Initialize();

	BINDPARAM_TIME_NOLEN( m_query, Now );
	BINDCOL_LONG( m_query, m_Auto_t.m_TicketID );
	BINDCOL_LONG( m_query, m_Auto_t.m_TicketBoxID );
	BINDCOL_TCHAR_NOLEN( m_query, szSubject );

	m_query.Execute( L"SELECT TicketID,Tickets.TicketBoxID,Subject "
				     L"FROM Tickets INNER JOIN TicketBoxes ON Tickets.TicketBoxID = TicketBoxes.TicketBoxID "
				     L"WHERE IsDeleted = 0 AND AgeAlertingEnabled = 1 "
					 L"AND TicketStateID>1 "
					 L"AND AgeAlerted = 0 "
					 L"AND DATEADD(minute,AgeAlertingThresholdMins,Tickets.OpenTimestamp) < ? "
 					 L"ORDER BY Tickets.TicketID" );

	while( m_query.Fetch() == S_OK )
	{
		m_AgeAlertedTickets.insert( pair<unsigned int,unsigned int> (m_Auto_t.m_TicketBoxID, m_Auto_t.m_TicketID) );

		// Replace all comma's with periods so we won't break the parsing
		sSubject.assign( szSubject );
		int pos = 0;
		while ( (pos = sSubject.find( _T(','), pos)) != tstring::npos )
		{
			sSubject.replace( pos, 1, 1, _T('.') );		
		}

		sMsgInfo.Format( _T("Ticket #%d %s\r\n"), m_Auto_t.m_TicketID, sSubject.c_str() );

		iter = AlertMsgBody.find( m_Auto_t.m_TicketBoxID );
		if( iter != AlertMsgBody.end() )
		{
			iter->second.append( sMsgInfo.c_str() );
		}
		else
		{
			AlertMsgBody[m_Auto_t.m_TicketBoxID] = (tstring) sMsgInfo;
		}
	}

	if(m_AgeAlertedTickets.size() > 0){DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetAgedTicket - Returning true", DebugReporter::ENGINE);}
	else{DebugReporter::Instance().DisplayMessage("CTicketMonitor::GetAgedTicket - Returning false", DebugReporter::ENGINE);}

	return (m_AgeAlertedTickets.size() > 0);
}

////////////////////////////////////////////////////////////////////////////////
// 
// HandleAgeAlert
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketMonitor::HandleAgeAlert( map<unsigned int,tstring>& AlertMsgBody )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::HandleAgeAlert - Entering", DebugReporter::ENGINE);

	multimap<unsigned int,unsigned int>::iterator iter;
	int TicketBoxID = 0;
	int nTickets = 0;
	int nPriorityID;

	// Send the alerts

	for( iter = m_AgeAlertedTickets.begin(); iter != m_AgeAlertedTickets.end(); iter++ )
	{
		if( iter->first != TicketBoxID )
		{
			if( TicketBoxID != 0 )
			{
				SendAgeAlert( TicketBoxID, nTickets, AlertMsgBody[TicketBoxID] );			
			}
			TicketBoxID = iter->first;
			nTickets = 0;
		}

		nTickets++;		

		// Get the old ticket priority
		m_query.Reset(true);
		BINDPARAM_LONG( m_query, iter->second );
		BINDCOL_LONG_NOLEN( m_query, nPriorityID );
		m_query.Execute( L"SELECT PriorityID FROM Tickets WHERE TicketID = ? " );
		m_query.Fetch();


		// change the ticket priority
		m_query.Reset(true);
		BINDPARAM_LONG( m_query, iter->second );
		m_query.Execute( L"UPDATE Tickets "
						 L"SET PriorityID = 1,AgeAlerted = 1 "
						 L"WHERE TicketID = ? " );

		// Log it in TicketHistory
		THAgeAlert( m_query, iter->second, nPriorityID, TicketBoxID );
	}

	if( nTickets > 0 )
	{
		SendAgeAlert( TicketBoxID, nTickets, AlertMsgBody[TicketBoxID] );
	}
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::HandleAgeAlert - Leaving", DebugReporter::ENGINE);
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendAgeAlert
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketMonitor::SendAgeAlert( int TicketBoxID, int nTickets, tstring& msgBody )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAgeAlert - Entering", DebugReporter::ENGINE);

	TCHAR szName[TICKETBOXES_NAME_LENGTH];
	long szNameLen;
	
	m_query.Reset(true);
	BINDCOL_TCHAR( m_query, szName );
	BINDPARAM_LONG( m_query, TicketBoxID );
	m_query.Execute( L"SELECT Name FROM TicketBoxes WHERE TicketBoxID=? " );
	if( m_query.Fetch() == S_OK )
	{
		Alert( EMS_ALERT_EVENT_AGE_ALERT, TicketBoxID, 
			   _T("%d ticket(s) in the %s TicketBox have been open longer than the age alerting threshold.\n\n%s"),
			   nTickets, szName, msgBody.c_str() );
	}

	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendAgeAlert - Leaving", DebugReporter::ENGINE);
}


////////////////////////////////////////////////////////////////////////////////
// 
// CheckWaterMarks
// 
////////////////////////////////////////////////////////////////////////////////
bool CTicketMonitor::CheckWaterMarks( void )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::CheckWaterMarks - Entering", DebugReporter::ENGINE);

	list<TTicketBoxes> TicketBoxList;
	list<TTicketBoxes>::iterator iter;
	TTicketBoxes info;
	int TicketCount;
	bool bActionTaken = false;

	info.PrepareList( m_query );

	while( m_query.Fetch() == S_OK )
	{
		TicketBoxList.push_back( info );
	}

	for( iter = TicketBoxList.begin(); iter != TicketBoxList.end(); iter++ )
	{
		// Get the open ticket count
		TicketCount = GetTicketCount( m_query, EMS_PUBLIC, iter->m_TicketBoxID, 0, 1, 0 );

		switch( iter->m_WaterMarkStatus )
		{
		case 0:
			if( iter->m_HighWaterMark && (TicketCount >= iter->m_HighWaterMark) )
			{
				SendWaterMarkAlert( &(*iter), TicketCount, EMS_ALERT_EVENT_HIGH_WATERMARK );
				bActionTaken = true;
			}
			else if( iter->m_LowWaterMark && (TicketCount >= iter->m_LowWaterMark) )
			{
				SendWaterMarkAlert( &(*iter), TicketCount, EMS_ALERT_EVENT_LOW_WATERMARK );
				bActionTaken = true;
			}
			break;

		case 1:
			if( iter->m_HighWaterMark && (TicketCount >= iter->m_HighWaterMark) )
			{
				SendWaterMarkAlert( &(*iter), TicketCount, EMS_ALERT_EVENT_HIGH_WATERMARK );
				bActionTaken = true;
			}
			else if( (iter->m_LowWaterMark == 0) || 
				     (iter->m_LowWaterMark && (TicketCount < iter->m_LowWaterMark)) )
			{
				SetTicketBoxStatus( iter->m_TicketBoxID, 0 );
				bActionTaken = true;
			}
			break;

		case 2:
			if( iter->m_LowWaterMark )
			{
				if( iter->m_HighWaterMark )
				{
					// Both LowWaterMark and HighWaterMark are set
					if( TicketCount < iter->m_HighWaterMark && TicketCount >= iter->m_LowWaterMark )
					{
						SetTicketBoxStatus( iter->m_TicketBoxID, 1 );
						bActionTaken = true;
					}
					else if ( TicketCount < iter->m_LowWaterMark )
					{
						SetTicketBoxStatus( iter->m_TicketBoxID, 0 );
						bActionTaken = true;
					}
				}
			}
			else
			{
				if( iter->m_HighWaterMark )
				{
					if( TicketCount < iter->m_HighWaterMark )
					{
						SetTicketBoxStatus( iter->m_TicketBoxID, 0 );
						bActionTaken = true;
					}
				}
				else
				{
					SetTicketBoxStatus( iter->m_TicketBoxID, 0 );
					bActionTaken = true;
				}
			}
			break;	
		}
	}
	
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::CheckWaterMarks - Returning", DebugReporter::ENGINE);

	return bActionTaken;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendWaterMarkAlert
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketMonitor::SendWaterMarkAlert( TTicketBoxes* pInfo, int Count, int AlertEventID )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendWaterMarkAlert - Entering", DebugReporter::ENGINE);

	CEMSString sAlertMsg;
	
	sAlertMsg.Format( _T("The %s TicketBox has %d open ticket(s)"),
		              pInfo->m_Name, Count );

	Alert( AlertEventID, pInfo->m_TicketBoxID, (wchar_t*) sAlertMsg.c_str() );
	
	switch( AlertEventID )
	{
	case EMS_ALERT_EVENT_HIGH_WATERMARK:
		SetTicketBoxStatus( pInfo->m_TicketBoxID, 2 );
		break;
		
	case EMS_ALERT_EVENT_LOW_WATERMARK:
		SetTicketBoxStatus( pInfo->m_TicketBoxID, 1 );
		break;
	}

	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendWaterMarkAlert - Leaving", DebugReporter::ENGINE);
}

void CTicketMonitor::SetTicketBoxStatus( int TicketBoxID, int Status )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SetTicketBoxStatus - Entering", DebugReporter::ENGINE);
	m_query.Reset();
	BINDPARAM_LONG( m_query, Status );
	BINDPARAM_LONG( m_query, TicketBoxID );
	m_query.Execute( _T("UPDATE TicketBoxes SET WaterMarkStatus=? WHERE TicketBoxID=?") );
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SetTicketBoxStatus - Leaving", DebugReporter::ENGINE);
}

////////////////////////////////////////////////////////////////////////////////
// 
// Check Custom WaterMarks
// 
////////////////////////////////////////////////////////////////////////////////
bool CTicketMonitor::CheckCustomWaterMarks( int nCheckType )
{
	dca::String t;
	t.Format("CTicketMonitor::CheckCustomWaterMarks For Type [%d] - Entering",  nCheckType);
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	
	list<TWaterMarkAlerts> wmaList;
	list<TWaterMarkAlerts>::iterator iter;
	TWaterMarkAlerts wma;	

	m_query.Initialize();
	BINDCOL_TCHAR( m_query, wma.m_Description );
	BINDCOL_LONG( m_query, wma.m_EditedByID );
	BINDCOL_LONG( m_query, wma.m_WaterMarkAlertID );
	BINDCOL_LONG( m_query, wma.m_HitCount );
	BINDCOL_LONG( m_query, wma.m_LowWaterMark );
	BINDCOL_LONG( m_query, wma.m_HighWaterMark );
	BINDCOL_BIT( m_query, wma.m_SendLowAlert );
	BINDCOL_TINYINT( m_query, wma.m_WaterMarkStatus );
	if ( nCheckType == 0 )
	{
		m_query.Execute( _T("SELECT TicketBoxes.Name, (SELECT COUNT(*) FROM Tickets WHERE Tickets.TicketBoxID=TicketBoxes.TicketBoxID AND Tickets.IsDeleted=0 ")
		  _T("AND Tickets.TicketStateID>1) AS TicketCount,WaterMarkAlerts.WaterMarkAlertID,WaterMarkAlerts.HitCount, ")
		  _T("WaterMarkAlerts.LowWaterMark,WaterMarkAlerts.HighWaterMark,WaterMarkAlerts.SendLowAlert, ")
		  _T("WaterMarkAlerts.WaterMarkStatus ")
		  _T("FROM WaterMarkAlerts INNER JOIN TicketBoxes ON WaterMarkAlerts.AlertOnID = TicketBoxes.TicketBoxID ")
		  _T("WHERE WaterMarkAlerts.AlertOnTypeID=0 AND WaterMarkAlerts.IsEnabled = 1 "));
	}
	else
	{
		m_query.Execute( _T("SELECT TicketCategories.Description, (SELECT COUNT(*) FROM Tickets WHERE Tickets.TicketCategoryID=TicketCategories.TicketCategoryID AND Tickets.IsDeleted=0 ")
		  _T("AND Tickets.TicketStateID>1) AS TicketCount,WaterMarkAlerts.WaterMarkAlertID,WaterMarkAlerts.HitCount, ")
		  _T("WaterMarkAlerts.LowWaterMark,WaterMarkAlerts.HighWaterMark,WaterMarkAlerts.SendLowAlert, ")
		  _T("WaterMarkAlerts.WaterMarkStatus ")
		  _T("FROM WaterMarkAlerts INNER JOIN TicketCategories ON WaterMarkAlerts.AlertOnID=TicketCategories.TicketCategoryID ")
		  _T("WHERE WaterMarkAlerts.AlertOnTypeID=1 AND WaterMarkAlerts.IsEnabled = 1 "));

	}
	
	while( m_query.Fetch() == S_OK )
	{
		wmaList.push_back( wma );
	}

	for( iter = wmaList.begin(); iter != wmaList.end(); iter++ )
	{
		wcscpy( m_Auto_t.m_TicketBoxName, iter->m_Description );
		
		switch( iter->m_WaterMarkStatus )
		{
		case 0:
			if( iter->m_EditedByID >= iter->m_HighWaterMark )
			{
				SendCustomWaterMarkAlert( iter->m_WaterMarkAlertID, iter->m_EditedByID, nCheckType, EMS_ALERT_EVENT_HIGH_WATERMARK );
				iter->m_HitCount++;
				SetWaterMarkHitCount(iter->m_WaterMarkAlertID, iter->m_HitCount);				
			}
			break;

		case 1:
			{

			}
			break;

		case 2:
			if( iter->m_EditedByID <= iter->m_LowWaterMark )
			{
				if ( iter->m_SendLowAlert == 1 )
				{
                    SendCustomWaterMarkAlert( iter->m_WaterMarkAlertID, iter->m_EditedByID, nCheckType, EMS_ALERT_EVENT_LOW_WATERMARK );
				}
				else
				{
					SetWaterMarkStatus( iter->m_WaterMarkAlertID, 0 );
				}
			}
			break;	
		}
	}
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::CheckCustomWaterMarks - Returning", DebugReporter::ENGINE);

	return false;
}


////////////////////////////////////////////////////////////////////////////////
// 
// SendCustomWaterMarkAlert
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketMonitor::SendCustomWaterMarkAlert( int AlertID, int Count, int nCheckType, int AlertEventID )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendCustomWaterMarkAlert - Entering", DebugReporter::ENGINE);

	CEMSString sAlertMsg;
		
	switch( AlertEventID )
	{
	case EMS_ALERT_EVENT_HIGH_WATERMARK:
		if ( nCheckType == 0 )
		{
			sAlertMsg.Format( _T("The number of open tickets in the %s TicketBox has increased to %d"),
							m_Auto_t.m_TicketBoxName, Count );
		}
		else
		{
			sAlertMsg.Format( _T("The number of open tickets in the %s Ticket Category has increased to %d"),
							m_Auto_t.m_TicketBoxName, Count );
		}
		SetWaterMarkStatus( AlertID, 2 );
		break;
		
	case EMS_ALERT_EVENT_LOW_WATERMARK:
		if ( nCheckType == 0 )
		{
			sAlertMsg.Format( _T("The number of open tickets in the %s TicketBox has decreased to %d"),
							m_Auto_t.m_TicketBoxName, Count );
		}
		else
		{
			sAlertMsg.Format( _T("The number of open tickets in the %s Ticket Category has decreased to %d"),
							m_Auto_t.m_TicketBoxName, Count );
		}		
		SetWaterMarkStatus( AlertID, 0 );
		break;	
	}
	
	CustomAlert( AlertEventID, (long)AlertID, (wchar_t*) sAlertMsg.c_str() );

	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SendCustomWaterMarkAlert - Leaving", DebugReporter::ENGINE);
}


void CTicketMonitor::SetWaterMarkStatus( int WaterMarkAlertID, int Status )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SetWaterMarkStatus - Entering", DebugReporter::ENGINE);
	m_query.Reset();
	BINDPARAM_LONG( m_query, Status );
	BINDPARAM_LONG( m_query, WaterMarkAlertID );
	m_query.Execute( _T("UPDATE WaterMarkAlerts SET WaterMarkStatus=? WHERE WaterMarkAlertID=?") );
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SetWaterMarkStatus - Leaving", DebugReporter::ENGINE);
}

void CTicketMonitor::SetWaterMarkHitCount( int WaterMarkAlertID, int HitCount )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SetWaterMarkHitCount - Entering", DebugReporter::ENGINE);
	m_query.Reset();
	BINDPARAM_LONG( m_query, HitCount );
	BINDPARAM_LONG( m_query, WaterMarkAlertID );
	m_query.Execute( L"UPDATE WaterMarkAlerts SET HitCount=? WHERE WaterMarkAlertID=?");
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::SetWaterMarkHitCount - Leaving", DebugReporter::ENGINE);
}

////////////////////////////////////////////////////////////////////////////////
// 
// CheckWaterMarks
// 
////////////////////////////////////////////////////////////////////////////////
bool CTicketMonitor::CheckLockedTickets( void )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::CheckLockedTickets - Entering", DebugReporter::ENGINE);

	list<TTickets> TicketList;
	list<TTickets>::iterator iter;
	TTickets m_tkt;
	TIMESTAMP_STRUCT CurrentTime;
	GetTimeStamp(CurrentTime);
	bool bActionTaken = false;
	
	//Get Max Lock Time Server Parameter
	int nMaxTicketLockTime = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_MAX_TICKET_LOCK_TIME ) );
	int nMaxLockTime = nMaxTicketLockTime;
	if(nMaxLockTime == 0)
	{
		nMaxLockTime = 60;
	}	

	m_query.Initialize();
	BINDCOL_LONG( m_query, m_tkt.m_TicketID );
	BINDCOL_TIME( m_query, m_tkt.m_LockedTime );
	BINDCOL_TIME( m_query, m_tkt.m_DateCreated );
	m_query.Execute( L"SELECT TicketID,LockedTime,DateCreated FROM Tickets WHERE LockedBy>0 AND IsDeleted=3");
	while( m_query.Fetch() == S_OK )
	{
		TicketList.push_back(m_tkt);	
	}

	for( iter = TicketList.begin(); iter != TicketList.end(); iter++ )
	{	
		if ( iter->m_LockedTimeLen != SQL_NULL_DATA )
		{
			int nTimeLocked = CompareTimeStamps( iter->m_LockedTime, CurrentTime )/60;
			if ( nTimeLocked >= nMaxLockTime )
			{
				//Unlock then Delete the Ticket
				FreeReservedMsgIDs( iter->m_TicketID );
				UnlockTicket( iter->m_TicketID );
				DeleteTicket( iter->m_TicketID );
				bActionTaken = true;
			}
		}
		else
		{
			int nTimeLocked = CompareTimeStamps( iter->m_DateCreated, CurrentTime )/60;
			if ( nTimeLocked >= nMaxLockTime )
			{
				//Unlock the Ticket
				FreeReservedMsgIDs( iter->m_TicketID );
				UnlockTicket( iter->m_TicketID );
				DeleteTicket( iter->m_TicketID );
				bActionTaken = true;
			}
		}
	}	
		
	if ( nMaxTicketLockTime == 0 )
	{
		//Max Lock Time is set to infinity (0)
		return bActionTaken;
	}
	
	TicketList.clear();
	m_query.Initialize();
	BINDCOL_LONG( m_query, m_tkt.m_TicketID );
	BINDCOL_TIME( m_query, m_tkt.m_LockedTime );
	BINDCOL_TIME( m_query, m_tkt.m_DateCreated );
	m_query.Execute( L"SELECT TicketID,LockedTime,DateCreated FROM Tickets WHERE LockedBy>0 AND IsDeleted=0");
	while( m_query.Fetch() == S_OK )
	{
		TicketList.push_back(m_tkt);	
	}

	for( iter = TicketList.begin(); iter != TicketList.end(); iter++ )
	{	
		if ( iter->m_LockedTimeLen != SQL_NULL_DATA )
		{
			int nTimeLocked = CompareTimeStamps( iter->m_LockedTime, CurrentTime )/60;
			if ( nTimeLocked >= nMaxTicketLockTime )
			{
				//Unlock the Ticket
				FreeReservedMsgIDs( iter->m_TicketID );
				UnlockTicket( iter->m_TicketID );
				bActionTaken = true;
			}
		}
		else
		{
			int nTimeLocked = CompareTimeStamps( iter->m_DateCreated, CurrentTime )/60;
			if ( nTimeLocked >= nMaxTicketLockTime )
			{
				//Unlock the Ticket
				FreeReservedMsgIDs( iter->m_TicketID );
				UnlockTicket( iter->m_TicketID );
				bActionTaken = true;
			}
		}
	}
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::CheckLockedTickets - Leaving", DebugReporter::ENGINE);
	return bActionTaken;
}

void CTicketMonitor::FreeReservedMsgIDs( int TicketID )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::FreeReservedMsgIDs - Entering", DebugReporter::ENGINE);

	CEMSString sQuery;
	CEMSString sWhere;
	TAttachments attach;
	tstring sFullAttachPath;
	list<unsigned int> AttachmentIDList;
	list<unsigned int>::iterator iter;

	// get a list of all the attachment IDs associated with the reserved messages
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, attach.m_AttachmentID );
	BINDPARAM_LONG( m_query, TicketID );
	m_query.Execute( L"SELECT A.AttachmentID FROM Attachments as A "
					 L"INNER JOIN OutboundMessageAttachments as O on A.AttachmentID = O.AttachmentID "
					 L"WHERE OutboundMessageID IN (SELECT OutboundMessageID FROM OutboundMessages "
					 L"WHERE IsDeleted=3 AND TicketID=?)");
	while( m_query.Fetch() == S_OK )
	{
		AttachmentIDList.push_back( attach.m_AttachmentID );
	}
	
	// delete records from OutboundMessageAttachments
	m_query.Reset();
	BINDPARAM_LONG( m_query, TicketID );
	m_query.Execute( L"DELETE FROM OutboundMessageAttachments "
					 L"WHERE OutboundMessageID IN (SELECT OutboundMessageID FROM OutboundMessages "
					 L"WHERE IsDeleted=3 AND TicketID=?)");
	
	// for each attachment
	for ( iter = AttachmentIDList.begin(); iter != AttachmentIDList.end(); iter++ )
	{
		// if the file is no longer referenced, delete it from the disk
		// and remove the record in the Attachments table
		if ( GetAttachmentReferenceCount( m_query, *iter ) == 0 )
		{
			attach.m_AttachmentID = *iter;
			attach.Query( m_query );
			
			attach.m_IsInbound ? GetFullInboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath ) :
			                     GetFullOutboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath );
			
			DeleteFile( sFullAttachPath.c_str() );
			
			attach.Delete( m_query );
		}
	}
	
	// delete records from the OutboundMessages table
	m_query.Reset();
	BINDPARAM_LONG( m_query, TicketID );
	m_query.Execute( L"DELETE FROM OutboundMessages "
					 L"WHERE OutboundMessageID IN (SELECT OutboundMessageID FROM OutboundMessages "
					 L"WHERE IsDeleted=3 AND TicketID=?)");

	DebugReporter::Instance().DisplayMessage("CTicketMonitor::FreeReservedMsgIDs - Leaving", DebugReporter::ENGINE);
}

void CTicketMonitor::UnlockTicket( int TicketID )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::UnlockTicket - Entering", DebugReporter::ENGINE);
	m_query.Initialize();
	BINDPARAM_LONG( m_query, TicketID );
	m_query.Execute( L"UPDATE Tickets SET LockedBy=0 WHERE TicketID=?" );
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::UnlockTicket - Leaving", DebugReporter::ENGINE);

}

void CTicketMonitor::DeleteTicket( int m_TicketID )
{	
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::DeleteTicket - Entering", DebugReporter::ENGINE);

	try
	{
		// do everything inside a transaction
		m_query.Initialize();
		m_query.Execute( _T("BEGIN TRANSACTION") );	

		//Get the current Ticket Data
		TTickets ticket;	
		ticket.m_TicketID = m_TicketID;
		ticket.Query(m_query);

		// get the current time
		TIMESTAMP_STRUCT time;
		GetTimeStamp(time);

		// delete inbound messages that are part of the ticket
		m_query.Initialize();
		
		BINDPARAM_TIME_NOLEN( m_query, time );
		BINDPARAM_LONG( m_query, m_TicketID );
		
		m_query.Execute( _T("UPDATE InboundMessages SET IsDeleted=1,DeletedTime=?,DeletedBy=0 WHERE TicketID=?"));
		
		// delete outbound messages
		m_query.Reset(false);
		m_query.Execute( _T("UPDATE OutboundMessages SET IsDeleted=1,DeletedTime=?,DeletedBy=0 WHERE TicketID=?"));
		
		// delete entries in the outbound message queue
		m_query.Initialize();
		BINDPARAM_LONG(m_query, m_TicketID );
		m_query.Execute( _T("DELETE FROM OutboundMessageQueue WHERE OutboundMessageID IN ")
							_T("(SELECT OutboundMessageID FROM OutboundMessages WHERE TicketID = ?)"));

		// remove any entries in the approvals queue
		m_query.Reset(false);
		m_query.Execute( _T("DELETE FROM Approvals WHERE ApprovalObjectTypeID=1 AND ActualID IN (SELECT OutboundMessageID FROM OutboundMessages WHERE TicketID = ?)") );
		
		// delete the ticket
		m_query.Initialize();
		
		BINDPARAM_TIME_NOLEN( m_query, time );
		BINDPARAM_LONG( m_query, m_TicketID );
		
		m_query.Execute( _T("UPDATE Tickets SET IsDeleted=1,LockedBy=0,DeletedTime=?,DeletedBy=0 WHERE TicketID=?") );		
		
		ticket.m_IsDeleted = EMS_DELETE_OPTION_DELETE_TO_WASTE_BASKET;
		ticket.m_DeletedTime = time;
		ticket.m_DeletedBy = 0;
		ticket.m_LockedBy = 0;		
	
		// update the ticket history
		THDeleteTicket( m_query, m_TicketID, 0, ticket.m_TicketStateID, ticket.m_TicketBoxID, ticket.m_OwnerID, ticket.m_PriorityID, ticket.m_TicketCategoryID );
		
		// we are done, commit the transaction
		m_query.Initialize();
		m_query.Execute( _T("COMMIT TRANSACTION") );
	}
	catch ( ... )
	{
		m_query.Initialize();
		m_query.Execute( _T("ROLLBACK TRANSACTION") );		
	}
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::DeleteTicket - Leaving", DebugReporter::ENGINE);
}

void CTicketMonitor::ProcessAutoActions(void)
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::ProcessAutoActions - Entering", DebugReporter::ENGINE);

	try
	{
		list<int> TicketList;
		list<int>::iterator iter;
		TIMESTAMP_STRUCT Now;
		TTickets m_Ticket;
		dca::String t;
		CEMSString sDateCreated;
		CEMSString sWhere;
		CEMSString sQuery;
		
		m_query.Initialize();
		BINDCOL_LONG( m_query, aa.m_TicketBoxID );
		BINDCOL_LONG( m_query, aa.m_AutoActionID );
		BINDCOL_LONG( m_query, aa.m_AutoActionVal );
		BINDCOL_LONG( m_query, aa.m_AutoActionFreq );
		BINDCOL_LONG( m_query, aa.m_AutoActionEventID );
		BINDCOL_LONG( m_query, aa.m_AutoActionTypeID );
		BINDCOL_LONG( m_query, aa.m_AutoActionTargetID );
		BINDCOL_TIME( m_query, aa.m_DateCreated );
		m_query.Execute( _T("SELECT TicketBoxID,AutoActionID,AutoActionVal,AutoActionFreq,AutoActionEventID,AutoActionTypeID,AutoActionTargetID,DateCreated ")
							_T("FROM AutoActions") );

		m_aa.clear();

		while( m_query.Fetch() == S_OK )
		{
			m_aa.push_back( aa );			
		}

		for( aaIter = m_aa.begin(); aaIter != m_aa.end(); aaIter++ )
		{
			GetDateTimeString( aaIter->m_DateCreated, sizeof(aaIter->m_DateCreated), sDateCreated, true, false );
			dca::String x(sDateCreated.c_str());
			t.Format("CTicketMonitor::ProcessAutoActions - %d:%d:%d:%d:%d:%d:%d:%s",  aaIter->m_TicketBoxID,
																					aaIter->m_AutoActionID,
																					aaIter->m_AutoActionVal,
																					aaIter->m_AutoActionFreq,
																					aaIter->m_AutoActionEventID,
																					aaIter->m_AutoActionTypeID,
																					aaIter->m_AutoActionTargetID,
																					x.c_str());
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);	
			
			long lMins = 0;
			switch(aaIter->m_AutoActionFreq)
			{
			case 0:
				lMins = aaIter->m_AutoActionVal;
				break;
			case 1:
				lMins = aaIter->m_AutoActionVal * 60;
				break;
			case 2:
				lMins = aaIter->m_AutoActionVal * 1440;
				break;
			}
			
			sWhere.Format(_T(""));        		
			switch(aaIter->m_AutoActionTypeID)
			{
			case 1:
				sWhere.Format(_T("AND t.OwnerID <> %d"), aaIter->m_AutoActionTargetID);
				break;
			case 2:
				sWhere.Format(_T("AND t.TicketStateID <> %d"), aaIter->m_AutoActionTargetID);
				break;
			case 4:
				sWhere.Format(_T("AND t.TicketStateID <> 4 AND t.OwnerID <> %d"), aaIter->m_AutoActionTargetID);
				break;
			}
			
			GetTimeStamp( Now );		
			m_query.Initialize();
			BINDCOL_LONG( m_query, m_Auto_t.m_TicketID );
			BINDPARAM_LONG( m_query, lMins );
			BINDPARAM_TIME( m_query, aaIter->m_DateCreated );
			BINDPARAM_LONG( m_query, lMins );
			BINDPARAM_TIME_NOLEN( m_query, Now );
			BINDPARAM_LONG( m_query, aaIter->m_TicketBoxID );

			switch( aaIter->m_AutoActionEventID )
			{
			case EMS_AUTO_ACTION_TICKET_OPENED:			
				sQuery.Format(   _T("SELECT DISTINCT t.TicketID FROM Tickets t ") 
								_T("INNER JOIN TicketHistory th ON t.TicketID = th.TicketID ") 
								_T("WHERE t.IsDeleted = 0 AND th.TicketActionID NOT IN (3,6,7,9,11,12,13) AND th.DateTime > DATEADD(minute, -?, ?) ") 
								_T("AND th.DateTime < DATEADD(minute, -?, ?) AND TicketActionID=2 AND ID1=3 AND ID2=1 ")
								_T("AND th.DateTime >= (SELECT MAX(DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID NOT IN (3,6,7,9,11,12,13)) ")					 
								_T("AND t.TicketBoxID=? %s"), sWhere.c_str() );				
				m_query.Execute(sQuery.c_str());

				break;
			case EMS_AUTO_ACTION_TICKET_CREATED:
				sQuery.Format(	_T("SELECT DISTINCT t.TicketID FROM Tickets t ") 
								_T("WHERE t.IsDeleted = 0 AND t.DateCreated BETWEEN DATEADD(minute, -?, ?) ")
								_T("AND DATEADD(minute, -?, ?) ")
								_T("AND t.TicketBoxID=? %s"), sWhere.c_str() );
				m_query.Execute(sQuery.c_str());

				break;
			case EMS_AUTO_ACTION_TICKET_CLOSED:
				sQuery.Format(	_T("SELECT DISTINCT t.TicketID FROM Tickets t ") 
								_T("INNER JOIN TicketHistory th ON t.TicketID = th.TicketID ") 
								_T("WHERE t.IsDeleted = 0 AND th.TicketActionID NOT IN (3,6,7,9,11,12,13) AND th.DateTime > DATEADD(minute, -?, ?) ") 
								_T("AND th.DateTime < DATEADD(minute, -?, ?) AND TicketActionID=2 AND ID1=3 AND ID2=2 ")
								_T("AND th.DateTime >= (SELECT MAX(DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID NOT IN (3,6,7,9,11,12,13)) ")					 
								_T("AND t.TicketBoxID=? %s"), sWhere.c_str() );
				m_query.Execute(sQuery.c_str());
				
				break;	
			case EMS_AUTO_ACTION_TICKET_ESCALATED:
				sQuery.Format(	_T("SELECT DISTINCT t.TicketID FROM Tickets t ") 
								_T("INNER JOIN TicketHistory th ON t.TicketID = th.TicketID ") 
								_T("WHERE t.IsDeleted = 0 AND th.TicketActionID NOT IN (3,6,7,9,11,12,13) AND th.DateTime > DATEADD(minute, -?, ?) ") 
								_T("AND th.DateTime < DATEADD(minute, -?, ?) AND TicketActionID=4 ")
								_T("AND th.DateTime >= (SELECT MAX(DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID NOT IN (3,6,7,9,11,12,13)) ")					 
								_T("AND t.TicketBoxID=? %s"), sWhere.c_str() );
				m_query.Execute(sQuery.c_str());
				
				break;	
			case EMS_AUTO_ACTION_TICKET_MOVED:
				sQuery.Format(	_T("SELECT DISTINCT t.TicketID FROM Tickets t ") 
								_T("INNER JOIN TicketHistory th ON t.TicketID = th.TicketID ") 
								_T("WHERE t.IsDeleted = 0 AND th.TicketActionID NOT IN (3,6,7,9,11,12,13) AND th.DateTime > DATEADD(minute, -?, ?) ") 
								_T("AND th.DateTime < DATEADD(minute, -?, ?) AND TicketActionID=2 AND ID1=1 ")
								_T("AND th.DateTime >= (SELECT MAX(DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID NOT IN (3,6,7,9,11,12,13)) ")					 
								_T("AND t.TicketBoxID=? %s"), sWhere.c_str() );
				m_query.Execute(sQuery.c_str());
				
				break;	
			case EMS_AUTO_ACTION_TICKET_REASSIGNED:
				sQuery.Format(	_T("SELECT DISTINCT t.TicketID FROM Tickets t ") 
								_T("INNER JOIN TicketHistory th ON t.TicketID = th.TicketID ") 
								_T("WHERE t.IsDeleted = 0 AND th.TicketActionID NOT IN (3,6,7,9,11,12,13) AND th.DateTime > DATEADD(minute, -?, ?) ") 
								_T("AND th.DateTime < DATEADD(minute, -?, ?) AND TicketActionID=2 AND ID1=2 ")
								_T("AND th.DateTime >= (SELECT MAX(DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID NOT IN (3,6,7,9,11,12,13)) ")					 
								_T("AND t.TicketBoxID=? %s"), sWhere.c_str() );
				m_query.Execute(sQuery.c_str());
				
				break;	
			case EMS_AUTO_ACTION_INBOUND_ADDED:
				sQuery.Format(	_T("SELECT DISTINCT t.TicketID FROM Tickets t ") 
								_T("INNER JOIN TicketHistory th ON t.TicketID = th.TicketID ") 
								_T("WHERE t.IsDeleted = 0 AND th.TicketActionID NOT IN (3,6,7,9,11,12,13) AND th.DateTime > DATEADD(minute, -?, ?) ") 
								_T("AND th.DateTime < DATEADD(minute, -?, ?) AND TicketActionID=5 AND ID1=1 ")
								_T("AND th.DateTime >= (SELECT MAX(DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID NOT IN (3,6,7,9,11,12,13)) ")					 
								_T("AND t.TicketBoxID=? %s"), sWhere.c_str() );
				m_query.Execute(sQuery.c_str());
				
				break;	
			case EMS_AUTO_ACTION_OUTBOUND_ADDED:
				sQuery.Format(	_T("SELECT DISTINCT t.TicketID FROM Tickets t ") 
								_T("INNER JOIN TicketHistory th ON t.TicketID = th.TicketID ") 
								_T("WHERE t.IsDeleted = 0 AND th.TicketActionID NOT IN (3,6,7,9,11,12,13) AND th.DateTime > DATEADD(minute, -?, ?) ") 
								_T("AND th.DateTime < DATEADD(minute, -?, ?) AND TicketActionID=5 AND ID1=0 ")
								_T("AND th.DateTime >= (SELECT MAX(DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID NOT IN (3,6,7,9,11,12,13)) ")					 
								_T("AND t.TicketBoxID=? %s"), sWhere.c_str() );
				m_query.Execute(sQuery.c_str());
				
				break;	
			case EMS_AUTO_ACTION_NOTE_ADDED:
				sQuery.Format(	_T("SELECT DISTINCT t.TicketID FROM Tickets t ") 
								_T("INNER JOIN TicketHistory th ON t.TicketID = th.TicketID ") 
								_T("WHERE t.IsDeleted = 0 AND th.TicketActionID NOT IN (3,6,7,9,11,12,13) AND th.DateTime > DATEADD(minute, -?, ?) ") 
								_T("AND th.DateTime < DATEADD(minute, -?, ?) AND TicketActionID=10 ")
								_T("AND th.DateTime >= (SELECT MAX(DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID NOT IN (3,6,7,9,11,12,13)) ")					 
								_T("AND t.TicketBoxID=? %s"), sWhere.c_str() );
				m_query.Execute(sQuery.c_str());
				
				break;	
			}

			dca::String q(sQuery.c_str());
			t.Format("CTicketMonitor::ProcessAutoActions - Executed query:[%s]", q.c_str());
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
				
			TicketList.clear();

			while( m_query.Fetch() == S_OK )
			{
				t.Format("CTicketMonitor::ProcessAutoActions - Adding TicketID:%d", m_Auto_t.m_TicketID);
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
				TicketList.push_back(m_Auto_t.m_TicketID);
			}
			
			if(TicketList.size() == 0)
			{
				DebugReporter::Instance().DisplayMessage("CTicketMonitor::ProcessAutoActions - No Tickets to Process", DebugReporter::ENGINE);
			}

			for ( iter = TicketList.begin(); iter != TicketList.end(); iter++ )
			{
				t.Format("CTicketMonitor::ProcessAutoActions - Processing TicketID:%d", *iter );
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);	
				m_Ticket.m_TicketID = *iter;
				m_Ticket.Query( m_query );
				
				m_query.Initialize();				
				switch( aaIter->m_AutoActionTypeID )
				{
				case EMS_AUTO_ACTION_CHANGE_OWNER:
					BINDPARAM_LONG( m_query, aaIter->m_AutoActionTargetID );					
					BINDPARAM_LONG( m_query, *iter );
					m_query.Execute( L"UPDATE Tickets SET OwnerID=? WHERE TicketID=?");
					THChangeOwner( m_query, m_Ticket.m_TicketID, 0, m_Ticket.m_OwnerID, m_Ticket.m_TicketStateID, m_Ticket.m_TicketBoxID, aaIter->m_AutoActionTargetID, m_Ticket.m_PriorityID, m_Ticket.m_TicketCategoryID );
				break;
				case EMS_AUTO_ACTION_CHANGE_STATE:
					BINDPARAM_LONG( m_query, aaIter->m_AutoActionTargetID );
					BINDPARAM_LONG( m_query, *iter );
					m_query.Execute( L"UPDATE Tickets SET TicketStateID=? WHERE TicketID=?");
					THChangeState( m_query, m_Ticket.m_TicketID, 0, m_Ticket.m_TicketStateID, aaIter->m_AutoActionTargetID, m_Ticket.m_TicketBoxID, m_Ticket.m_OwnerID, m_Ticket.m_PriorityID, m_Ticket.m_TicketCategoryID );
				break;
				case EMS_AUTO_ACTION_DELETE:
					GetTimeStamp( Now );
					BINDPARAM_LONG( m_query, *iter );
					m_query.Execute( _T("DELETE FROM OutboundMessageQueue WHERE OutboundMessageID IN ")
										_T("(SELECT OutboundMessageID FROM OutboundMessages WHERE TicketID = ?)"));

					m_query.Reset(false);
					m_query.Execute( _T("DELETE FROM Approvals WHERE ApprovalObjectTypeID=1 AND ActualID IN (SELECT OutboundMessageID FROM OutboundMessages WHERE TicketID = ?)") );
					
					m_query.Reset(true);				
					BINDPARAM_TIME_NOLEN( m_query, Now );
					BINDPARAM_LONG( m_query, *iter );
					m_query.Execute( L"UPDATE InboundMessages SET IsDeleted=1,DeletedTime=?,DeletedBy=0 WHERE TicketID=?");
					m_query.Reset(false);
					m_query.Execute( L"UPDATE OutboundMessages SET IsDeleted=1,DeletedTime=?,DeletedBy=0 WHERE TicketID=?");
					m_query.Reset(false);
					m_query.Execute( L"UPDATE Tickets SET TicketStateID=5,IsDeleted=1,DeletedTime=?,DeletedBy=0 WHERE TicketID=?");
									
					THDeleteTicket( m_query, m_Ticket.m_TicketID, 0, 5, m_Ticket.m_TicketBoxID,m_Ticket.m_OwnerID, m_Ticket.m_PriorityID, m_Ticket.m_TicketCategoryID );
				break;
				case EMS_AUTO_ACTION_ESCALATE:					
					EscalateTicket( *iter, aaIter->m_TicketBoxID, aaIter->m_AutoActionTargetID );
				break;
				case EMS_AUTO_ACTION_MOVE:
					BINDPARAM_LONG( m_query, aaIter->m_AutoActionTargetID );
					BINDPARAM_LONG( m_query, *iter );
					m_query.Execute( L"UPDATE Tickets SET TicketBoxID=? WHERE TicketID=?");
					THChangeTicketBox( m_query, m_Ticket.m_TicketID, 0, m_Ticket.m_TicketBoxID, m_Ticket.m_TicketStateID, aaIter->m_AutoActionTargetID, m_Ticket.m_OwnerID, m_Ticket.m_PriorityID, m_Ticket.m_TicketCategoryID );
				break;
				}
			}
		}
	}
	catch(dca::Exception& e)
	{
		try
		{
			dca::String er;
			dca::String x(e.GetMessage());
			er.Format("CTicketMonitor::ProcessAutoActions - %s", x.c_str());
			DebugReporter::Instance().DisplayMessage(er.c_str(), DebugReporter::ENGINE);
			Log( E_DBMonitorError, L"%s\n", e.GetMessage());				
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("CTicketMonitor::ProcessAutoActions - DCA Exception", DebugReporter::ENGINE);
		}
	}
	catch(ODBCError_t error) 
	{ 
		try
		{
			dca::String er;
			dca::String x(error.szErrMsg);
			er.Format("CTicketMonitor::ProcessAutoActions - %s", x.c_str());
			DebugReporter::Instance().DisplayMessage(er.c_str(), DebugReporter::ENGINE);
			Log( E_DBMonitorError, L"%s\n", error.szErrMsg);				
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("CTicketMonitor::ProcessAutoActions - ODBC Error", DebugReporter::ENGINE);
		}
		
	}	
	catch( ... )
	{
		DebugReporter::Instance().DisplayMessage("CTicketMonitor::ProcessAutoActions - Unhandled Exception", DebugReporter::ENGINE);	
	}	
	
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::ProcessAutoActions - Leaving", DebugReporter::ENGINE);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Escalate Ticket	              
\*--------------------------------------------------------------------------*/
void CTicketMonitor::EscalateTicket( int nTicketID, int nTicketBoxID, int nEscalateToID )
{
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::EscalateTicket - Entering", DebugReporter::ENGINE);

	try
	{
		int nOldStateID;
		int nOldOwnerID;
		int m_PriorityID;
		int m_TicketCategoryID;
		dca::String t;

		set<long> EscalateAgentIDs;
		CEMSString strSubject;
		CEMSString strAlertSubject;
		CEMSString strAlertBody;
		dca::WString strEmptyString = "";
		CEMSString strAgentName;
		CEMSString sName;

		strAgentName.Format( _T("System"), sName.c_str());

		list<int> AgentIDs;
		list<int>::iterator iter;
		int AgentID;
		int iTen = 10;
		int nEscalateToTicketBoxID;
		int nUseEscTicketBox;
				
		m_query.Initialize();
		
		BINDCOL_LONG_NOLEN( m_query, nOldStateID );
		BINDCOL_LONG_NOLEN( m_query, nOldOwnerID );		
		BINDCOL_LONG_NOLEN( m_query,  m_PriorityID );
		BINDCOL_LONG_NOLEN( m_query, m_TicketCategoryID );		
		BINDPARAM_LONG( m_query, nTicketID );		
		m_query.Execute( _T("SELECT TicketStateID, OwnerID, PriorityID, TicketCategoryID FROM Tickets WHERE TicketID = ?" ) );
		
		if ( m_query.Fetch() != S_OK )
		{
			t.Format("CTicketMonitor::EscalateTicket - Invalid TicketID [%d]",  nTicketID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);			
		}
		
		if (nOldOwnerID == nEscalateToID)
			return;
		
		try
		{
			TCHAR szSubject[TICKETS_SUBJECT_LENGTH] = {0};
			long szSubjectLen = 0;
			
			m_query.Initialize();
			
			BINDCOL_TCHAR( m_query, szSubject );
			BINDPARAM_LONG( m_query, nTicketID );

			m_query.Execute( _T("SELECT Subject FROM Tickets WHERE TicketID = ?") );
			
			if ( m_query.Fetch() == S_OK )
				strSubject = szSubjectLen ? szSubject : _T("[No Subject]");
			
		}
		catch(...) 
		{
			// just eat any exception
		}

		AgentIDs.push_back( nEscalateToID );
		strAlertSubject.Format( _T("Ticket [ %d ] Escalated by the System :: %s"), nTicketID, strAgentName.c_str(), strSubject.c_str() );
		strAlertBody.Format( _T("Ticket Escalated\n\nTicket #%d :: %s\r\n"), nTicketID, strSubject.c_str() );
		
		int m_TicketStateID = EMS_TICKETSTATEID_ESCALATED;
		
		int m_OwnerID = nEscalateToID;
		nEscalateToTicketBoxID = nTicketBoxID;
		
		m_query.Initialize();
		BINDPARAM_LONG( m_query, m_TicketStateID );
		BINDPARAM_LONG( m_query, m_OwnerID );
		BINDPARAM_LONG( m_query, nEscalateToTicketBoxID );
		BINDPARAM_LONG( m_query, nTicketID );
		
		m_query.Execute( _T("UPDATE TICKETS SET TicketStateID=?, OWNERID=?, TicketBoxID=? WHERE TicketID = ?") );
		
		if ( m_query.GetRowCount() != 1 )
		{
			t.Format("CTicketMonitor::EscalateTicket - Invalid TicketID [%d]",  nTicketID);
			DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		}
		
		for( iter = AgentIDs.begin(); iter != AgentIDs.end(); iter++ )
		{
			m_query.Initialize();
			BINDPARAM_LONG( m_query, iTen );
			BINDPARAM_LONG( m_query, *iter );
			BINDPARAM_TCHAR_STRING( m_query, strAlertSubject );
			BINDPARAM_TCHAR_STRING( m_query, strAlertBody );
			BINDPARAM_LONG( m_query, nTicketID );
			m_query.Execute(_T("INSERT INTO AlertMsgs (AlertEventID, AgentID, Subject, Body, TicketID) VALUES (?, ?, ?, ?, ?)"));
		}
		
		// log the change in the ticket history
		THEscalate( m_query, nTicketID, 0, nOldOwnerID, nOldStateID, m_TicketStateID, nTicketBoxID, m_OwnerID, m_PriorityID, m_TicketCategoryID );
				
	}
	catch(...) 
	{
		// just eat any exception
	}
	DebugReporter::Instance().DisplayMessage("CTicketMonitor::EscalateTicket - Leaving", DebugReporter::ENGINE);
}

int CTicketMonitor::FinishCurrentJob(void)
{
	return 0;
}