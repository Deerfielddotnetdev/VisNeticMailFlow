////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/AlertSender.cpp,v 1.1 2005/08/09 16:40:55 markm Exp $
//
//  Copyright © 2000 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// This object is a memmber of WorkerThread
//
//////////////////////////////////////////////////////////////////////
// AlertSender.cpp: implementation of the CAlertSender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AlertSender.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAlertSender::CAlertSender( CODBCQuery& m_query ) : m_query(m_query)
{

}

CAlertSender::~CAlertSender()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// SendAlert
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertSender::SendAlert( long AlertEventID, AlertInfo* pAlertInfo )
{
	CAlert alert( AlertEventID, pAlertInfo );
	m_Queue.push_back( alert );
}


////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CAlertSender::Run(void)
{
	if( m_Queue.size() == 0 )
	{
		return S_FALSE;
	}
	
	DebugReporter::Instance().DisplayMessage("CAlertSender::Run", DebugReporter::ENGINE);	
	
	HandleAlert( m_Queue.front() );

	// Remove the item from the queue
	m_Queue.pop_front();

	if( m_Queue.size() == 0 )
	{
		return S_FALSE;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// HandleAlert
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertSender::HandleAlert( CAlert& alert )
{
	CAlertRecipient recipient;
	vector<CAlertRecipient>::iterator iter;
	int nAlertToTypeID;
	long nAlertToTypeIDLen;
	int nAlertToID;
	long nAlertToIDLen;
	
	// MJM - the from email address for all alerts is 
	// now the system's email address
	/*m_query.Initialize();
	int nID = EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS;
	BINDPARAM_LONG( m_query, nID );
	BINDCOL_TCHAR( m_query, recipient.m_FromEmailAddress );
	
	m_query.Execute( _T("SELECT DataValue FROM ServerParameters ")
					 _T("WHERE ServerParameterID=?") );
	m_query.Fetch();*/

	const wchar_t* pSysEmail = g_Object.GetParameter( EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS );	
	wcscpy( recipient.m_FromEmailAddress, pSysEmail );
	
	// Handle custom Age and WaterMark Alerts
	if ( alert.m_pAlertInfo->AlertID )
	{
		if ( alert.m_AlertEventID == 1 || alert.m_AlertEventID == 2 )
		{
			//WaterMark Alert, let's get the alert info using the AlertID
			m_query.Initialize();
			BINDPARAM_LONG( m_query, alert.m_pAlertInfo->AlertID );
			BINDCOL_LONG( m_query, nAlertToTypeID );
			BINDCOL_LONG( m_query, nAlertToID );
			BINDCOL_LONG( m_query, recipient.m_AlertMethodID );
			BINDCOL_TCHAR( m_query, recipient.m_EmailAddress );
			BINDCOL_TCHAR( m_query, recipient.m_FromEmailAddress );
			BINDCOL_TCHAR( m_query, recipient.m_AlertDescription );
			m_query.Execute( _T("SELECT AlertToTypeID,AlertToID,AlertMethodID,EmailAddress,FromEmailAddress,Description ")
							_T("FROM WaterMarkAlerts WHERE WaterMarkAlertID=?") );
		}
		else if ( alert.m_AlertEventID == 3 )
		{
			//Age Alert, let's get the alert info using the AlertID
			m_query.Initialize();
			BINDPARAM_LONG( m_query, alert.m_pAlertInfo->AlertID );
			BINDCOL_LONG( m_query, nAlertToTypeID );
			BINDCOL_LONG( m_query, nAlertToID );
			BINDCOL_LONG( m_query, recipient.m_AlertMethodID );
			BINDCOL_TCHAR( m_query, recipient.m_EmailAddress );
			BINDCOL_TCHAR( m_query, recipient.m_FromEmailAddress );
			BINDCOL_TCHAR( m_query, recipient.m_AlertDescription );
			m_query.Execute( _T("SELECT AlertToTypeID,AlertToID,AlertMethodID,EmailAddress,FromEmailAddress,Description ")
							_T("FROM AgeAlerts WHERE AgeAlertID=?") );
		}
		else if  ( alert.m_AlertEventID == 11 )
		{
            //Report Alert, let's get the alert info using the AlertID
			m_query.Initialize();
			BINDPARAM_LONG( m_query, alert.m_pAlertInfo->AlertID );
			BINDCOL_LONG( m_query, nAlertToID );
			BINDCOL_TCHAR( m_query, recipient.m_AlertDescription );
			m_query.Execute( _T("SELECT OwnerID,Description ")
							_T("FROM ReportScheduled INNER JOIN ReportResults ")
							_T("ON ReportResults.ScheduledReportID=ReportScheduled.ScheduledReportID ")
							_T("WHERE ReportResults.ReportResultID=?") );
		}
		else if  ( alert.m_AlertEventID >= 12 && alert.m_AlertEventID <= 17 )
		{
            //Agent Alert, let's get the alert info using the AlertID
			m_query.Initialize();
			BINDPARAM_LONG( m_query, alert.m_pAlertInfo->AlertID );
			BINDCOL_LONG( m_query, nAlertToID );
			BINDCOL_LONG( m_query, recipient.m_AlertMethodID );
			BINDCOL_TCHAR( m_query, recipient.m_EmailAddress );
			BINDCOL_TCHAR( m_query, recipient.m_AlertDescription );
			m_query.Execute( _T("SELECT AgentID,AlertMethodID,EmailAddress,Description ")
							_T("FROM Alerts INNER JOIN AlertEvents ")
							_T("ON Alerts.AlertEventID=AlertEvents.AlertEventID ")
							_T("WHERE Alerts.AlertID=?") );
		}
				
		while( m_query.Fetch() == S_OK )
		{
			if ( alert.m_AlertEventID == 11 )
			{
				recipient.m_AlertMethodID = 1;
				nAlertToTypeID = 0;
			}
			else if ( alert.m_AlertEventID >= 12 && alert.m_AlertEventID <= 17 )
			{
				nAlertToTypeID = 0;
				recipient.m_AlertEventID = alert.m_AlertEventID;				
			}
			if ( nAlertToTypeID == 1 )
			{
				recipient.m_GroupID = nAlertToID;				
			}
			else
			{
				recipient.m_GroupID = 0;				
			}
						
			recipient.m_AgentID = nAlertToID;
			recipient.m_AlertEventID = alert.m_AlertEventID;
			alert.m_Recipients.push_back( recipient );
		}
	}
	else
	{
		if( alert.m_Recipients.size() == 0 )
		{
			m_query.Initialize();

			if( alert.m_pAlertInfo->TicketBoxID )
			{
				// Ticket Box Alert
				BINDPARAM_LONG( m_query, alert.m_pAlertInfo->TicketBoxID );
				BINDPARAM_LONG( m_query, alert.m_AlertEventID );
				BINDPARAM_LONG( m_query, alert.m_pAlertInfo->TicketBoxID );

				BINDCOL_LONG( m_query, recipient.m_AlertMethodID );
				BINDCOL_LONG( m_query, recipient.m_AgentID );
				BINDCOL_LONG( m_query, recipient.m_GroupID );
				BINDCOL_WCHAR( m_query, recipient.m_AlertDescription );
				BINDCOL_WCHAR( m_query, recipient.m_EmailAddress );
				BINDCOL_TCHAR( m_query, recipient.m_TicketBoxName );

				m_query.Execute( L"SELECT AlertMethodID,AgentID,GroupID,AlertEvents.Description,EmailAddress,TicketBoxes.Name "
								L"FROM Alerts "
								L"INNER JOIN AlertEvents ON Alerts.AlertEventID=AlertEvents.AlertEventID "
								L"INNER JOIN TicketBoxes ON TicketBoxes.TicketBoxID=? "
								L"WHERE Alerts.AlertEventID=? AND (Alerts.TicketBoxID=? OR Alerts.TicketBoxID=0)" );
			}
			else
			{
				// Normal Alert
				BINDPARAM_LONG( m_query, alert.m_AlertEventID );
				BINDCOL_LONG( m_query, recipient.m_AlertMethodID );
				BINDCOL_LONG( m_query, recipient.m_AgentID );
				BINDCOL_LONG( m_query, recipient.m_GroupID );
				BINDCOL_WCHAR( m_query, recipient.m_AlertDescription );
				BINDCOL_WCHAR( m_query, recipient.m_EmailAddress );

				m_query.Execute( L"SELECT AlertMethodID,AgentID,GroupID,Description,EmailAddress "
								L"FROM Alerts "
								L"INNER JOIN AlertEvents ON Alerts.AlertEventID=AlertEvents.AlertEventID "
								L"WHERE Alerts.AlertEventID=?" );
			}


			while( m_query.Fetch() == S_OK )
			{
				recipient.m_AlertEventID = alert.m_AlertEventID;
				alert.m_Recipients.push_back( recipient );

			}
		}
	}
	
	if  ( alert.m_AlertEventID == 6 || alert.m_AlertEventID == 7 )
	{
        tstring sTemp = alert.m_pAlertInfo->wcsText;
		if(_tcsicmp( sTemp.c_str(), _T("MailComponents DLL has requested a restart, see the debug log for details") ) == 0)
		{
			int retval=-1;
			dca::String t;

			retval = g_Object.m_MessagingComponents.Shutdown();

			if(retval == 0)
			{
				t.Format("CAlertSender::HandleAlert - Successfully unloaded mailcomponents.dll due to request - mail send and receive is disabled.");
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
					
				retval = g_Object.m_MessagingComponents.LoadDLL();							

				if( retval )
				{
					CEMSString s;
					s.LoadString( EMS_STRING_ERROR_LOADING_MAILCOMPONENTS );
					_Module.LogEvent( s.c_str(), retval );
					Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, L"Error %d loading mailcomponents.dll - mail send and receive is disabled.", retval );
					t.Format("CAlertSender::HandleAlert - Error %d loading mailcomponents.dll - mail send and receive is disabled.", retval);
					DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
					retval = 0;
				}
				else
				{
					t.Format("CAlertSender::HandleAlert - Successfully loaded mailcomponents.dll");
					DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

					retval = g_Object.m_MessagingComponents.Initialize();
					if(retval)
					{
						t.Format("CAlertSender::HandleAlert - Error initializing mailcomponents.dll.");
						DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
					}
					else
					{
						t.Format("CAlertSender::HandleAlert - Successfully initialized mailcomponents.dll.");
						DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
					}
				}
			}
			else
			{
				t.Format("CAlertSender::HandleAlert - Failed to unload mailcomponents.dll - mail send and receive may be disabled.");
				DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);							
			}		
		}
	}

	for(iter = alert.m_Recipients.begin(); iter != alert.m_Recipients.end(); ++iter)
	{
		if( iter->m_bSent == FALSE )
		{
			DispatchAlert( alert, *iter );
			iter->m_bSent = TRUE;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// DispatchAlert
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertSender::DispatchAlert( CAlert& alert, CAlertRecipient& recipient )
{
	switch( recipient.m_AlertMethodID )
	{
	case EMS_ALERT_METHOD_INTERNAL_EMAIL:
		SendInternalEmail( alert, recipient );
		break;

	case EMS_ALERT_METHOD_EXTERNAL_EMAIL:
		SendExternalEmail( alert, recipient );
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendInstantMsg
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertSender::SendInstantMsg( CAlert& alert, CAlertRecipient& recipient )
{
	TInstantMessages IM;
	CEMSString sBody;

	IM.m_ToAgentID = recipient.m_AgentID;
	IM.m_FromAgentID = 0;	// system
	GetTimeStamp( IM.m_TimeStamp );
	IM.m_TimeStampLen = sizeof( TIMESTAMP_STRUCT );

	sBody.Format( _T("ALERT: %s\n%s"), recipient.m_AlertDescription, alert.m_pAlertInfo->wcsText );

	PutStringProperty( sBody, &IM.m_Body, &IM.m_BodyAllocated );

	IM.Insert( m_query );
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendInternalEmail
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertSender::SendInternalEmail( CAlert& alert, CAlertRecipient& recipient )
{
	TAlertMsgs IM;
	list<int> AgentIDs;
	list<int>::iterator iter;
	int AgentID;
	int TicketID = alert.m_pAlertInfo->AlertID;
	int nAlertMsgID;

	lstrcpy( IM.m_Subject, _T("") );

	if( recipient.m_GroupID )
	{
		// Get agentids for group
		m_query.Initialize();
		BINDPARAM_LONG( m_query, recipient.m_GroupID );
		BINDCOL_LONG_NOLEN( m_query, AgentID );
		m_query.Execute( _T("SELECT AgentID ")
						 _T("FROM AgentGroupings ")
						 _T("WHERE GroupID=?") );

		while( m_query.Fetch() == S_OK )
		{
			AgentIDs.push_back( AgentID );
		}

	}
	else
	{
		AgentIDs.push_back( recipient.m_AgentID );
	}

	for( iter = AgentIDs.begin(); iter != AgentIDs.end(); iter++ )
	{
		
		GetTimeStamp( IM.m_DateCreated );
		IM.m_DateCreatedLen = sizeof( TIMESTAMP_STRUCT );
		IM.m_AlertEventID = recipient.m_AlertEventID;
		IM.m_AgentID = *iter;
		
		//lstrcpy( IM.m_Subject, _T("Test") );
		if ( alert.m_pAlertInfo->AlertID )
		{
			lstrcpy( IM.m_Subject, recipient.m_AlertDescription );
		}
		else if ( recipient.m_TicketBoxNameLen > 0 )
		{
			lstrcpy( IM.m_Subject, recipient.m_TicketBoxName );
		}
		
		if ( alert.m_AlertEventID >= 12 && alert.m_AlertEventID <= 17 )
		{
			//extract the subject from the alertinfo
			//Copy to a tstring and look for "|||"
			tstring sTemp = alert.m_pAlertInfo->wcsText;
			tstring::size_type pos = sTemp.find( _T("|||") );
			tstring sSubject = sTemp.substr(0,pos);
			wcscpy( IM.m_Subject, (_T("%s"),sSubject.c_str()) );
			tstring sBody = sTemp.substr(pos+3,sTemp.length());
			wchar_t* szBody = (wchar_t*) HeapAlloc (GetProcessHeap (), 0, sizeof(wchar_t) * (wcslen(sBody.c_str())+1) );
			wcscpy(szBody,(_T("%s"),sBody.c_str()));
			PutStringPropertyW( szBody, &IM.m_Body, &IM.m_BodyAllocated );
			if ( alert.m_AlertEventID >= 12 && alert.m_AlertEventID <= 14 )
			{			
				//extract the TicketID from the body
				pos = sBody.find( _T("Ticket #") );
				if ( pos != tstring::npos )
				{
					tstring::size_type pos2 = sBody.find(_T(" "),pos+8);
					tstring sTicketID = sBody.substr(pos+8,pos2-(pos+8));
					try
					{
						IM.m_TicketID = _ttoi( sTicketID.c_str() );
					}
					catch(...)
					{
						//DebugReporter::Instance().DisplayMessage("CAlertSender::SendInternalEmail - Exception caught while getting TicketID.", DebugReporter::ENGINE);
					}
				}
			}
			else if ( alert.m_AlertEventID >= 15 && alert.m_AlertEventID <= 17 )
			{
				//extract the OutboundMessageID from the body
				pos = sBody.find( _T("Message #") );
				if ( pos != tstring::npos )
				{
					tstring::size_type pos2 = sBody.find(_T(" "),pos+9);
					tstring sTicketID = sBody.substr(pos+9,pos2-(pos+9));
					try
					{
						IM.m_TicketID = _ttoi( sTicketID.c_str() );
					}
					catch(...)
					{
						//DebugReporter::Instance()::Instance().DisplayMessage("CAlertSender::SendInternalEmail - Exception caught while getting OutboundMessageID.", DebugReporter::ENGINE);
					}
				}
			}
		}
		else
		{
			PutStringPropertyW( alert.m_pAlertInfo->wcsText, &IM.m_Body, &IM.m_BodyAllocated );
		}		
		IM.Insert( m_query );
		if ( recipient.m_AlertEventID == 11 )
		{
			nAlertMsgID = IM.m_AlertMsgID;

			//Update the TicketID
			m_query.Initialize();
			BINDPARAM_LONG( m_query, TicketID );
			BINDPARAM_LONG( m_query, nAlertMsgID );
			m_query.Execute( _T("UPDATE AlertMsgs SET TicketID=? WHERE AlertMsgID=?"));
		}
						
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendExternalEmail
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertSender::SendExternalEmail( CAlert& alert, CAlertRecipient& recipient )
{
	TOutboundMessages alertmsg;
	
	// auto-reply to this message

	alertmsg.m_AgentID = 0;		// System is sending this message
	GetTimeStamp( alertmsg.m_EmailDateTime );
	alertmsg.m_EmailDateTimeLen = sizeof( TIMESTAMP_STRUCT );
	alertmsg.m_TicketID = 0;

	alertmsg.m_OutboundMessageTypeID = EMS_OUTBOUND_MESSAGE_TYPE_NEW;
	alertmsg.m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_UNTOUCHED;
	alertmsg.m_IsDeleted = 2;
	wcscpy( alertmsg.m_MediaType, _T("TEXT") );
	wcscpy( alertmsg.m_MediaSubType, _T("PLAIN") );

	// Copy in the body of the alert
	
	if ( alert.m_AlertEventID >= 12 && alert.m_AlertEventID <= 17 )
	{
		//extract the subject from the alertinfo
		//Copy to a tstring and look for "|||"
		tstring sTemp = alert.m_pAlertInfo->wcsText;
		tstring::size_type pos = sTemp.find( _T("|||") );
		tstring sSubject = sTemp.substr(0,pos);
		wcscpy( recipient.m_AlertDescription, (_T("%s"),sSubject.c_str()) );
		tstring sBody = sTemp.substr(pos+3,sTemp.length());
		wchar_t* szBody = (wchar_t*) HeapAlloc (GetProcessHeap (), 0, sizeof(wchar_t) * (wcslen(sBody.c_str())+1) );
		wcscpy(szBody,(_T("%s"),sBody.c_str()));
		PutStringPropertyW( szBody, &alertmsg.m_Body, &alertmsg.m_BodyAllocated );
	}
	else
	{
		PutStringPropertyW( alert.m_pAlertInfo->wcsText, &alertmsg.m_Body, &alertmsg.m_BodyAllocated );
	}

	// Put the Re: in front of the subject, if there isn't one already
	swprintf( alertmsg.m_Subject, _T("MailFlow Alert: %s"), recipient.m_AlertDescription );			

	// Set the Email To: Address
	wcscpy( alertmsg.m_EmailPrimaryTo, recipient.m_EmailAddress );
	PutStringPropertyW( alertmsg.m_EmailPrimaryTo, &alertmsg.m_EmailTo, &alertmsg.m_EmailToAllocated );

	// Set the Email From: Address
	wcscpy( alertmsg.m_EmailFrom, recipient.m_FromEmailAddress );

	// Copy the EmailFrom -> EmailReplyTo
	PutStringPropertyW( alertmsg.m_EmailFrom, &alertmsg.m_EmailReplyTo, &alertmsg.m_EmailReplyToAllocated );

	// Create the message record
	alertmsg.Insert( m_query );

	//	Put In Outbound Queue
	m_query.Reset();
	BINDPARAM_LONG( m_query, alertmsg.m_OutboundMessageID );
	BINDPARAM_TIME( m_query, alertmsg.m_EmailDateTime );
	m_query.Execute( L"INSERT INTO OutboundMessageQueue "
					 L"(OutboundMessageID,DateSpooled,IsApproved) "
					 L"VALUES "
					 L"(?,?,1)" );

	// undelete the message
	m_query.Initialize();
	BINDPARAM_LONG( m_query, alertmsg.m_OutboundMessageID );
	m_query.Execute( L"UPDATE OutboundMessages "
					 L"SET IsDeleted=0 "
					 L"WHERE OutboundMessageID=?");

}

////////////////////////////////////////////////////////////////////////////////
// 
// UnInitialize
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertSender::UnInitialize(void)
{
	// Handle all remaining alerts
	while( m_Queue.size() > 0 )
	{
		HandleAlert( m_Queue.front() );
		m_Queue.pop_front();
	}
}

