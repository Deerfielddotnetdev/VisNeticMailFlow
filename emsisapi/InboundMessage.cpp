/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/InboundMessage.cpp,v 1.2.2.2 2006/02/23 20:37:11 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Inbound Message Class   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "InboundMessage.h"
#include "Ticket.h"
#include "attachfns.h"
#include "contactfns.h"
#include "stringfns.h"
#include "TicketHistoryFns.h"
#include <CkCrypt2.h>
#include <CkString.h>

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction/Destruction	              
\*--------------------------------------------------------------------------*/
CInboundMessage::CInboundMessage(CISAPIData& ISAPIData )
	:CXMLDataClass( ISAPIData )
	,m_BeenRead(0)
	,m_UseCutoffDate(0)
{
	// set the created date
	GetTimeStamp( m_EmailDateTime );
	m_EmailDateTimeLen = sizeof(m_EmailDateTime);

	GetTimeStamp( m_DateReceived );
	m_DateReceivedLen = sizeof(m_DateReceived);
	
	// mark the item as partially created
	m_IsDeleted = EMS_DELETE_OPTION_DELETE_PERMANENTLY;

	m_RepliedToWhenLen = SQL_NULL_DATA;
}

CInboundMessage::~CInboundMessage()
{

}

int CInboundMessage::Run(CURLAction& action)
{
	try
	{	
		int nMsgID;
		int nBeenRead;
		bool bExist = false;

		GetISAPIData().GetURLLong( _T("msgID"), nMsgID);
		GetISAPIData().GetURLLong( _T("beenRead"), nBeenRead);

		long lAgentID = GetISAPIData().m_pSession->m_AgentID;

		if(!UseCutoffDate())
		{
			dca::String sCmd;
			sCmd.Format(_T("SELECT InboundMessageReadID FROM InboundMessageRead WHERE InboundMessageID = %d AND AgentID = %d"), nMsgID, lAgentID);
			GetQuery().Initialize();
			GetQuery().Execute(sCmd.c_str());
			if(GetQuery().Fetch() == S_OK)
				bExist = true;

			if(nBeenRead)
			{
				if(!bExist)
				{
					GetQuery().Initialize();
					BINDPARAM_LONG( GetQuery(), nMsgID );
					BINDPARAM_LONG( GetQuery(), lAgentID );
					GetQuery().Execute(_T("INSERT INTO InboundMessageRead (InboundMessageID, AgentID) VALUES (?, ?)"));

					long nTicketID;
					long nTicketBoxID;
					GetQuery().Initialize();
					BINDPARAM_LONG( GetQuery(), nMsgID);
					BINDCOL_LONG_NOLEN( GetQuery(), nTicketID); 
					BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID); 
					GetQuery().Execute(_T("SELECT t.TicketID,t.TicketBoxID FROM InboundMessages im INNER JOIN Tickets t ON im.TicketID=t.TicketID WHERE (InboundMessageID = ?)"));

					if(GetQuery().Fetch() == S_OK)
					{
						CInboundMessage::UpdateTicketHistoryMsgRead(GetQuery(), lAgentID, nTicketID, 1, nMsgID, nTicketBoxID);
					}
				}
			}
			else
			{
				if(bExist)
				{
					GetQuery().Initialize();
					BINDPARAM_LONG( GetQuery(), nMsgID );
					BINDPARAM_LONG( GetQuery(), lAgentID);
					GetQuery().Execute(_T("DELETE FROM InboundMessageRead WHERE InboundMessageID = ? AND AgentID = ?"));

					long nTicketID;
					long nTicketBoxID;
					GetQuery().Initialize();
					BINDPARAM_LONG( GetQuery(), nMsgID);
					BINDCOL_LONG_NOLEN( GetQuery(), nTicketID); 
					BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID); 
					GetQuery().Execute(_T("SELECT t.TicketID,t.TicketBoxID FROM InboundMessages im INNER JOIN Tickets t ON im.TicketID=t.TicketID WHERE (InboundMessageID = ?)"));

					if(GetQuery().Fetch() == S_OK)
					{
						CInboundMessage::UpdateTicketHistoryMsgRead(GetQuery(), lAgentID, nTicketID, 0, nMsgID, nTicketBoxID);
					}
				}
			}
		}

		GetXMLGen().AddChildElem( _T("postresults"));
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("complete"), _T("true") );
		GetXMLGen().OutOfElem();
	}
	catch(...)
	{
		GetXMLGen().AddChildElem( _T("postresults"));
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("complete"), _T("false") );
		GetXMLGen().OutOfElem();
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML representing the message
\*--------------------------------------------------------------------------*/
void CInboundMessage::GenerateXML( bool bEscapeBody, int nArchiveID, bool bForEdit )
{
	CEMSString sDateTime;
	CEMSString sBody;
	CEMSString sSubject;
	int nIsHTML = _tcsicmp(m_MediaSubType, _T("html")) == 0 ? 1 : 0;
	
	GetXMLGen().AddChildElem( _T("Message") );

	GetXMLGen().AddChildAttrib( _T("ID"), m_InboundMessageID );
	GetXMLGen().AddChildAttrib( _T("IsDeleted"), m_IsDeleted );
	GetXMLGen().AddChildAttrib( _T("IsArchived"), nArchiveID ? _T("1") : _T("0") );
	GetXMLGen().AddChildAttrib( _T("usecuttoffdate"), m_UseCutoffDate ? _T("1") : _T("0") );
	GetXMLGen().AddChildAttrib( _T("beenread"), m_BeenRead ? _T("1") : _T("0") );
	GetXMLGen().AddChildAttrib( _T("Type"), _T("inbound"));
	
	GetXMLGen().AddChildAttrib( _T("HTML"), nIsHTML );
	
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
	
	if(lTzBias != 1)
	{
		if(ConvertToTimeZone(m_DateReceived,lTzBias,tsLocal))
		{
			GetDateTimeString( tsLocal, tsLocalLen, sDateTime );
		}
		else
		{
			GetDateTimeString( m_DateReceived, m_DateReceivedLen, sDateTime);
		}
	}
	else
	{
		GetDateTimeString( m_DateReceived, m_DateReceivedLen, sDateTime);
	}	
	GetXMLGen().AddChildAttrib( _T("DateReceived"), sDateTime.c_str() );

	GetXMLGen().AddChildAttrib( _T("PriorityID"), m_PriorityID );
	GetXMLGen().AddChildAttrib( _T("ContactID"), m_ContactID );
	
	GetXMLGen().IntoElem();
	
	GetXMLGen().AddChildElem( _T("POPHeaders") );
	sBody.assign( m_PopHeaders );
	sBody.EscapeHTMLAndNewLines();
	GetXMLGen().SetChildData( sBody.c_str(), TRUE );
	
	GetXMLGen().AddChildElem( _T("PrimaryTo"), m_EmailPrimaryTo );
	GetXMLGen().AddChildElem( _T("ReplyTo"), m_EmailReplyToLen ? m_EmailReplyTo : m_EmailFrom );
	GetXMLGen().AddChildElem( _T("To"), m_EmailTo );
	GetXMLGen().AddChildElem( _T("From"), m_EmailFrom );
	GetXMLGen().AddChildElem( _T("cc"), m_EmailCc );
	
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
	long start = GetTickCount();
	if( bEscapeBody )
	{
		sBody.assign( m_Body );
		sBody.EscapeBody( nIsHTML, m_InboundMessageID, bForEdit ? 0 : 1, nArchiveID, bForEdit );
		sBody.RemoveScriptTags();
		sBody.FixBodyTag();
		if( nIsHTML )
			sBody.FixCommentTag();
		
		/*if( bForEdit )
			RemoveTicketIDFromBody(sBody);*/

		GetXMLGen().SetChildData( sBody.c_str(), TRUE );
	}
	else
	{
		sBody.assign( m_Body );
		sBody.RemoveScriptTags();
		/*if( bForEdit )
			RemoveTicketIDFromBody(sBody);*/

		GetXMLGen().SetChildData( sBody.c_str(), TRUE );
	}
	long end = GetTickCount();
	long elapsed = end-start;
	dca::String f;
	f.Format("CInboundMessage::GenerateXML - Generate Body XML Begin:%d - End:%d - Elapsed:%d", start,end,elapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	
	// list the attachments
	if ( !m_IsDeleted && !m_ArchiveID )
	{
		list<CAttachment>::iterator iter;
		
		for ( iter = m_AttachList.begin(); iter != m_AttachList.end(); iter++ )
		{
			iter->GenerateXML();
		}
	}
	
	// Has this message been replied to?
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
||  Comments:	Query	              
\*--------------------------------------------------------------------------*/
void CInboundMessage::Query( void )
{
	if ( !m_InboundMessageID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("An InboundMessageID must be specified") );
	
	// query the database
	if ( TInboundMessages::Query( GetQuery() ) != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("Inbound Message (%d) does not exist in the database"), m_InboundMessageID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	// check security
	CTicket Ticket(m_ISAPIData);
	Ticket.m_TicketID = m_TicketID;
	Ticket.RequireRead();

	// virus scan the message body
	VirusScanBody();
	
	// format the To: and Cc: address strings
	FormatEmailAddrString( &m_EmailTo, m_EmailToLen, m_EmailToAllocated);
	FormatEmailAddrString( &m_EmailCc, m_EmailCcLen, m_EmailCcAllocated);
	FormatEmailAddrString( &m_EmailReplyTo, m_EmailReplyToLen, m_EmailReplyToAllocated);

	// format the FROM: address
	if ( m_EmailFromNameLen > 0)
	{
		CEMSString sString;
		sString.Format( "\"%s\" <%s>", m_EmailFromName, m_EmailFrom );
		
		if ( sString.size() < 255 )
			_tcsncpy( m_EmailFrom, sString.c_str(), 255 );		
	}

	// format the message body
	if ( m_IsDeleted )
	{
		PutStringProperty( tstring(_T("Message has been deleted")) , &m_Body, &m_BodyAllocated );
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
	else if ( m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_VIRUS_FOUND || 
		      m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_SUSPICIOUS )
	{
		// TODO - the admin needs a method to access the message
		// even though its infected...

		// build the virus report
		CEMSString sVirusMsg;
		sVirusMsg.Format(IDS_ISAPI_ERROR_INFECTED_BODY, m_VirusName );
		
		// replace the body with the virus report
		PutStringProperty(sVirusMsg, &m_Body, &m_BodyAllocated );
	}
	
	// has this message been replied to?
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_InboundMessageID );
	BINDCOL_TIME( GetQuery(), m_RepliedToWhen );
	GetQuery().Execute( _T("SELECT EmailDateTime FROM OutboundMessages ")
		                _T("WHERE ReplyToMsgID=? and ReplyToIDIsInbound=1 ")
						_T("ORDER BY EmailDateTime Desc") );
	GetQuery().Fetch();

	GetQuery().Initialize();

	// has this message been read by agent
	if(!UseCutoffDate())
	{
		long lAgentID = GetISAPIData().m_pSession->m_AgentID;
		long lBeenReadID;
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_InboundMessageID );
		BINDPARAM_LONG( GetQuery(), lAgentID );
		BINDCOL_LONG_NOLEN( GetQuery(), lBeenReadID );
		GetQuery().Execute(_T("SELECT InboundMessageReadID FROM InboundMessageRead WHERE InboundMessageID = ? AND AgentID = ?"));
		if(GetQuery().Fetch() == S_OK)
			m_BeenRead = 1;
	}
	else
	{
		m_BeenRead = 1;
		m_UseCutoffDate = 1;
	}
	
	// query the attachments
	CAttachment attachment(m_ISAPIData);
	attachment.ListInboundMessageAttachments( m_InboundMessageID, m_AttachList );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	QueryMinimal	              
\*--------------------------------------------------------------------------*/
void CInboundMessage::QueryMinimal( void )
{
	if ( !m_InboundMessageID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("An InboundMessageID must be specified") );
	
	// query the database
	if ( TInboundMessages::Query( GetQuery() ) != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("Inbound Message (%d) does not exist in the database"), m_InboundMessageID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Virus scans the message body. Query must be called 
||				before this function.	              
\*--------------------------------------------------------------------------*/
void CInboundMessage::VirusScanBody(void)
{
	// bail out if virus scanning isn't enabled
	tstring sValue;
	GetServerParameter( EMS_SRVPARAM_ANTIVIRUS_ENABLE, sValue );
	
	if (sValue.compare( _T("0") ) == 0 )
		return;
	
	// if the message hasn't been scanned, scan it now
	if ( m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_NOT_SCANNED )
	{
		CEMSString sVirusName;
		tstring sCleanedString;
		
		int nRet = VirusScanString( m_Body, m_BodyLen, sVirusName, sCleanedString );

		GetQuery().Initialize();

		if ( nRet == E_InfectedBody )
		{
			m_VirusScanStateID = EMS_VIRUS_SCAN_STATE_VIRUS_FOUND;
			_tcsncpy( m_VirusName, sVirusName.c_str(), 125 );

			BINDPARAM_LONG( GetQuery(), m_VirusScanStateID );
			BINDPARAM_TCHAR( GetQuery(), m_VirusName );
			BINDPARAM_LONG( GetQuery(), m_InboundMessageID );
			
			GetQuery().Execute( _T("UPDATE InboundMessages SET VirusScanStateID=?,VirusName=? ")
				                _T("WHERE InboundMessageID=?") );
		}
		else if ( nRet == E_CleanedBody )
		{
			m_VirusScanStateID = EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_REPAIRED;
			PutStringProperty(sCleanedString, &m_Body, &m_BodyAllocated );
			
			BINDPARAM_LONG( GetQuery(), m_VirusScanStateID );
			BINDPARAM_TEXT( GetQuery(), m_Body );
			BINDPARAM_LONG( GetQuery(), m_InboundMessageID );
			
			GetQuery().Execute( _T("UPDATE InboundMessages SET VirusScanStateID=?,Body=? WHERE InboundMessageID=?") );
		}
		else
		{
			m_VirusScanStateID = EMS_VIRUS_SCAN_STATE_VIRUS_FREE;
			
			BINDPARAM_LONG( GetQuery(), m_VirusScanStateID );
			BINDPARAM_LONG( GetQuery(), m_InboundMessageID );
			
			GetQuery().Execute( _T("UPDATE InboundMessages SET VirusScanStateID=? WHERE InboundMessageID=?") );
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Delete the message
\*--------------------------------------------------------------------------*/
void CInboundMessage::Delete( void )
{
	if ( !m_InboundMessageID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("An Inbound Message ID must be specified") );

	// get the Ticket ID
	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), m_TicketID );
	BINDPARAM_LONG(GetQuery(), m_InboundMessageID );

	GetQuery().Execute( _T("SELECT TicketID FROM InboundMessages WHERE InboundMessageID=?") );

	if ( GetQuery().Fetch() != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("Unable to delete message: Inbound Message (%d) does not exist"), m_InboundMessageID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	if ( m_TicketID > 0 )
	{
		// check security
		CTicket Ticket(m_ISAPIData, m_TicketID);
		Ticket.Query();
		Ticket.RequireDelete();

		// lock the ticket
		Ticket.Lock();

		// if this is the only message in the ticket
		// delete the ticket as well
		if ( Ticket.GetMsgCount() <= 1 )
		{
			Ticket.Delete(false);
		}
		else
		{
			// delete the message	
			TIMESTAMP_STRUCT time;
			GetTimeStamp(time);
			
			GetQuery().Initialize();
				
			BINDPARAM_TIME_NOLEN( GetQuery(), time );
			BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
			BINDPARAM_LONG( GetQuery(), m_InboundMessageID );
				
			GetQuery().Execute(_T("UPDATE InboundMessages SET IsDeleted = 1, DeletedTime = ?, DeletedBy = ? WHERE InboundMessageID = ?"));
			
			// Log it in system-generated ticket history
			THDelInboundMsg( GetQuery(), m_TicketID, GetAgentID(), m_InboundMessageID, Ticket.m_TicketBoxID );

			if ( _ttoi(Ticket.sAal.c_str()) == 1 && _ttoi(Ticket.sTmd.c_str()) == 1 )
			{
				Ticket.LogAgentAction(GetAgentID(),12,m_InboundMessageID,m_TicketID,0,_T(""),_T(""));
			}
			
		}
	}
	else
	{
		// delete the message	
		TIMESTAMP_STRUCT time;
		GetTimeStamp(time);
		
		GetQuery().Initialize();
			
		BINDPARAM_TIME_NOLEN( GetQuery(), time );
		BINDPARAM_LONG( GetQuery(), m_InboundMessageID );
			
		GetQuery().Execute(_T("UPDATE InboundMessages SET IsDeleted = 1, DeletedTime = ? WHERE InboundMessageID = ?"));			
	}

	m_IsDeleted = EMS_DELETE_OPTION_DELETE_TO_WASTE_BASKET;
}

bool CInboundMessage::UseCutoffDate()
{
	TServerParameters servParams;
	servParams.m_ServerParameterID = 41;

	int nResult = servParams.Query(GetQuery());

	if(nResult != S_OK)
		return false;

	if(!lstrcmp(servParams.m_DataValue,_T("1")))
	{
		servParams.m_ServerParameterID = 42;

		nResult = servParams.Query(GetQuery());

		if((nResult != S_OK) || (lstrlen(servParams.m_DataValue) == 0))
			return false;

		SYSTEMTIME recvTime;
		ZeroMemory(&recvTime, sizeof(SYSTEMTIME));

		recvTime.wYear = m_DateReceived.year;
		recvTime.wDay = m_DateReceived.day;
		recvTime.wMonth = m_DateReceived.month;
		recvTime.wHour = m_DateReceived.hour;
		recvTime.wMinute = m_DateReceived.minute;
		recvTime.wSecond = m_DateReceived.second;

		SYSTEMTIME cutoffTime;
		ZeroMemory(&cutoffTime, sizeof(SYSTEMTIME));

		dca::String sDate(servParams.m_DataValue);

		dca::String::size_type pos = dca::String::npos;

		int nDatePos = 0;
		do
		{
			pos = sDate.find('/');

			if(pos != dca::String::npos)
			{
				dca::String temp = sDate.substr(0, pos);
				sDate = sDate.erase(0, pos + 1);
				switch(nDatePos)
				{
				case 0:
					{
						cutoffTime.wMonth = temp.ToInt();
						nDatePos++;
					}
					break;
				case 1:
					{
						cutoffTime.wDay = temp.ToInt();
						nDatePos++;
					}
					break;
				}
			}
			else
			{
				if(nDatePos == 2)
				{
					cutoffTime.wYear = sDate.ToInt();
				}
			}
		}while(pos != dca::String::npos);

		cutoffTime.wHour = 23;
		cutoffTime.wMinute = 59;
		cutoffTime.wSecond = 59;

		FILETIME ftRecv;
		FILETIME ftCutoff;

		SystemTimeToFileTime(&recvTime, &ftRecv);
		SystemTimeToFileTime(&cutoffTime, &ftCutoff);

		if(CompareFileTime(&ftCutoff, &ftRecv) == 1)
			return true;

		return false;
	}

	return false;
}

void CInboundMessage::UpdateTicketHistoryMsgRead(CODBCQuery& query, int nAgentID, int nTicketID, int nRead, int nMsgID, int nTicketBoxID)
{
	SYSTEMTIME curTime;
	::GetLocalTime(&curTime);

	TTicketHistory tth;
	tth.m_AgentID = nAgentID;
	tth.m_TicketID = nTicketID;
	tth.m_TicketActionID = EMS_TICKETACTIONID_READ_MSG;
	tth.m_ID1 = nRead;
	tth.m_ID2 = nMsgID;
	tth.m_DateTime.year = curTime.wYear;
	tth.m_DateTime.month = curTime.wMonth;
	tth.m_DateTime.day = curTime.wDay;
	tth.m_DateTime.hour = curTime.wHour;
	tth.m_DateTime.minute = curTime.wMinute;
	tth.m_DateTime.second = curTime.wSecond;
	tth.m_DateTimeLen = sizeof(TIMESTAMP_STRUCT);
	tth.m_TicketBoxID = nTicketBoxID;

	tth.Insert(query);
}
