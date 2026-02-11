/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketHistory.cpp,v 1.2.2.1 2006/02/23 20:37:12 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS: Lists the history for a ticket   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketHistory.h"
#include "DateFns.h"
#include "SecurityFns.h"
#include "Ticket.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CTicketHistory::CTicketHistory(CISAPIData& ISAPIData)
	:CXMLDataClass(ISAPIData)
{
	m_nMaxMsgThreads  = 0;
	m_nMaxNoteThreads = 0;
	m_bShowTicketDetails = 0;
	m_bDetailedThreading = 0;
	m_nIsDeleted = 0;
	GetServerParameter( EMS_SRVPARAM_TICKET_LIST_DATE, sDateTime );
	nDateTime = _ttoi( sDateTime.c_str() );	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CTicketHistory::Run( CURLAction& action )
{	
	int nID;
	
	// get options
	GetISAPIData().GetURLLong( _T("ShowTicketDetails"), m_bShowTicketDetails, true );
	GetISAPIData().GetURLLong( _T("Threaded"), m_bDetailedThreading, true );
	GetISAPIData().GetURLLong( _T("IsDeleted"), m_nIsDeleted, true );

	// max thread limit?
	GetISAPIData().GetURLLong( _T("MaxMsgThreads"), m_nMaxMsgThreads, true );

	GetISAPIData().GetURLLong( _T("MaxNoteThreads"), m_nMaxNoteThreads, true );
	
	if (GetISAPIData().GetURLLong( _T("ID"), nID ))
		ListTicketHistory( nID );
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates a threaded message history list for the ticket	              
\*--------------------------------------------------------------------------*/
void CTicketHistory::ListTicketHistory( int nTicketID )
{
	int nThreads = 0;

	// query the ticket and check security
	GetTicketInfo( nTicketID );

	GetXMLGen().IntoElem();
	GetQuery().Initialize();

	CTicketHistoryMsg msg;
	BINDCOL_LONG_NOLEN( GetQuery(), msg.m_ID );
	BINDCOL_BIT_NOLEN( GetQuery(), msg.m_IsInbound );
	BINDCOL_TCHAR( GetQuery(), msg.m_Subject );
	BINDCOL_TCHAR( GetQuery(), msg.m_EmailAddr );
	BINDCOL_LONG_NOLEN( GetQuery(), msg.m_Prority );
	BINDCOL_LONG_NOLEN( GetQuery(), msg.m_AgentID );
	BINDCOL_LONG_NOLEN( GetQuery(), msg.m_ReplyToMsgID );
	BINDCOL_BIT_NOLEN( GetQuery(), msg.m_ReplyToIDIsInbound );
	BINDCOL_LONG_NOLEN( GetQuery(), msg.m_ContactID );
	BINDCOL_LONG_NOLEN( GetQuery(), msg.m_OutboundMsgTypeID );
	BINDCOL_LONG_NOLEN( GetQuery(), msg.m_OutboundMsgStateID );
	BINDCOL_TIME( GetQuery(), msg.m_Date );
	BINDCOL_LONG_NOLEN( GetQuery(), msg.m_AttachmentCount);
	
	BINDPARAM_LONG( GetQuery(), nTicketID );
	BINDPARAM_LONG( GetQuery(), nTicketID );
			
	if(m_nIsDeleted)
	{
		if(nDateTime > 0)
		{
			GetQuery().Execute( 
			
				_T("SELECT InboundMessageID,1,Subject,EmailFrom,PriorityID,0,ReplyToMsgID,ReplyToIDIsInbound,")
				_T("ContactID,0,0,EmailDateTime as Date,(SELECT COUNT(*) InboundMessageID FROM InboundMessageAttachments ")
				_T("WHERE InboundMessageID = I.InboundMessageID) ")
				_T("FROM InboundMessages as I WHERE TicketID=? AND IsDeleted=1 UNION ")

				_T("SELECT OutboundMessageID,0,Subject,EmailFrom,PriorityID,AgentID,ReplyToMsgID,ReplyToIDIsInbound,")
				_T("0,OutboundMessageTypeID,OutboundMessageStateID,EmailDateTime as Date,")
				_T("(SELECT COUNT(*) OutboundMessageID FROM OutboundMessageAttachments ")
				_T("WHERE OutboundMessageID = O.OutboundMessageID) ")
				_T("FROM OutboundMessages as O WHERE TicketID=? AND IsDeleted=1 ")

				_T("ORDER BY Date") );
		}
		else
		{
			GetQuery().Execute( 
			
				_T("SELECT InboundMessageID,1,Subject,EmailFrom,PriorityID,0,ReplyToMsgID,ReplyToIDIsInbound,")
				_T("ContactID,0,0,DateReceived as Date,(SELECT COUNT(*) InboundMessageID FROM InboundMessageAttachments ")
				_T("WHERE InboundMessageID = I.InboundMessageID) ")
				_T("FROM InboundMessages as I WHERE TicketID=? AND IsDeleted=1 UNION ")

				_T("SELECT OutboundMessageID,0,Subject,EmailFrom,PriorityID,AgentID,ReplyToMsgID,ReplyToIDIsInbound,")
				_T("0,OutboundMessageTypeID,OutboundMessageStateID,EmailDateTime as Date,")
				_T("(SELECT COUNT(*) OutboundMessageID FROM OutboundMessageAttachments ")
				_T("WHERE OutboundMessageID = O.OutboundMessageID) ")
				_T("FROM OutboundMessages as O WHERE TicketID=? AND IsDeleted=1 ")

				_T("ORDER BY Date") );
		}
	}
	else
	{
		if(nDateTime > 0)
		{
			GetQuery().Execute( 
			
			_T("SELECT InboundMessageID,1,Subject,EmailFrom,PriorityID,0,ReplyToMsgID,ReplyToIDIsInbound,")
			_T("ContactID,0,0,EmailDateTime as Date,(SELECT COUNT(*) InboundMessageID FROM InboundMessageAttachments ")
			_T("WHERE InboundMessageID = I.InboundMessageID) ")
			_T("FROM InboundMessages as I WHERE TicketID=? AND IsDeleted=0 UNION ")

			_T("SELECT OutboundMessageID,0,Subject,EmailFrom,PriorityID,AgentID,ReplyToMsgID,ReplyToIDIsInbound,")
			_T("0,OutboundMessageTypeID,OutboundMessageStateID,EmailDateTime as Date,")
			_T("(SELECT COUNT(*) OutboundMessageID FROM OutboundMessageAttachments ")
			_T("WHERE OutboundMessageID = O.OutboundMessageID) ")
			_T("FROM OutboundMessages as O WHERE TicketID=? AND IsDeleted=0 ")

			_T("ORDER BY Date") );
		}
		else
		{
			GetQuery().Execute( 
			
			_T("SELECT InboundMessageID,1,Subject,EmailFrom,PriorityID,0,ReplyToMsgID,ReplyToIDIsInbound,")
			_T("ContactID,0,0,DateReceived as Date,(SELECT COUNT(*) InboundMessageID FROM InboundMessageAttachments ")
			_T("WHERE InboundMessageID = I.InboundMessageID) ")
			_T("FROM InboundMessages as I WHERE TicketID=? AND IsDeleted=0 UNION ")

			_T("SELECT OutboundMessageID,0,Subject,EmailFrom,PriorityID,AgentID,ReplyToMsgID,ReplyToIDIsInbound,")
			_T("0,OutboundMessageTypeID,OutboundMessageStateID,EmailDateTime as Date,")
			_T("(SELECT COUNT(*) OutboundMessageID FROM OutboundMessageAttachments ")
			_T("WHERE OutboundMessageID = O.OutboundMessageID) ")
			_T("FROM OutboundMessages as O WHERE TicketID=? AND IsDeleted=0 ")

			_T("ORDER BY Date") );
		}		
	}
	
	MsgID_t MsgKey;
	MsgID_t ThreadKey;
	
	while( GetQuery().Fetch() == S_OK )
	{
		if ( !m_bDetailedThreading  && msg.m_IsInbound )
		{
			msg.m_ReplyToIDIsInbound = 0;
			msg.m_ReplyToMsgID = 0;
		}
		
		MsgKey.MsgID = msg.m_ID;
		MsgKey.IsInbound = msg.m_IsInbound;
		
		// add the message ID to a list the list allows us 
		// to maintain the order returned from the database
		m_MsgList.push_back(MsgKey);

		// add the details of the message to a map
		// this allows us fast access to any message
		m_MsgMap.insert(pair<MsgID_t, CTicketHistoryMsg>(MsgKey, msg));
		
		// add each messages "parent" to a map this allows us to 
		// quickly determine which message are part of the top level
		if ( m_bDetailedThreading || !msg.m_IsInbound )
		{
			ThreadKey.MsgID = msg.m_ReplyToMsgID;
			ThreadKey.IsInbound = msg.m_ReplyToIDIsInbound;
			m_ThreadingMap.insert(pair<MsgID_t, MsgID_t>(ThreadKey, MsgKey));
		}
	}
	
	list<MsgID_t>::reverse_iterator ListIter;
	MsgMap::iterator MsgMapIter; ;

	// walk through the list of messages in the order
	// which they were returned from the databse
	for(ListIter = m_MsgList.rbegin(); ListIter != m_MsgList.rend(); ListIter++)
	{
		// lookup the message in the map.
		if ( (MsgMapIter = m_MsgMap.find( *ListIter )) != m_MsgMap.end() )
		{
			MsgID_t ThreadKey;
			ThreadKey.MsgID = MsgMapIter->second.m_ReplyToMsgID;
			ThreadKey.IsInbound = MsgMapIter->second.m_ReplyToIDIsInbound;

			// if this message is not a reply to another message in the ticket
			// it is at the top level
			if ( m_MsgMap.find( ThreadKey ) == m_MsgMap.end() )
			{
				// have we reached the max message thread count?
				if ( (m_nMaxMsgThreads) && (nThreads >= m_nMaxMsgThreads) )
				{
					GetXMLGen().AddChildElem( _T("More") );
					GetXMLGen().AddChildAttrib( _T("Description"), _T("Max thread limit reached") );
					break;
				}

				// add the message to the XML
				GenMsgXML( MsgMapIter->second );

				// find children (outbound) of this message
				FindChildMessages( *ListIter );

				// we added a thread
				nThreads++;
			}
		}
	}


	GetXMLGen().OutOfElem();

	// list any ticket notes
	ListTicketNotes( nTicketID );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Finds messages that are children of nMsgID	              
\*--------------------------------------------------------------------------*/
void CTicketHistory::FindChildMessages( MsgID_t Msg )
{
	MsgMap::iterator MsgIter;
	ThreadingMap::iterator start = m_ThreadingMap.lower_bound(Msg);
	ThreadingMap::iterator end = m_ThreadingMap.upper_bound(Msg);

	GetXMLGen().IntoElem();

	// for each child message...
	while ((start != end) && (start != m_ThreadingMap.end()))
	{
		MsgIter = m_MsgMap.find( start->second );

		if ( MsgIter != m_MsgMap.end() )
		{
			GenMsgXML( MsgIter->second );
			FindChildMessages( MsgIter->first );
		}

		start++;
	}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add the message to the XML
\*--------------------------------------------------------------------------*/
void CTicketHistory::GenMsgXML( CTicketHistoryMsg& msg)
{
	CEMSString sDateTime;
	CEMSString sSubject;
	long lAgentID = GetISAPIData().m_pSession->m_AgentID;

	GetXMLGen().AddChildElem(_T("Message"));

	GetXMLGen().AddChildAttrib( _T("ID"), msg.m_ID );
	GetXMLGen().AddChildAttrib( _T("Inbound"), msg.m_IsInbound );
	
	if(msg.m_IsInbound)
	{
		if(!UseCutoffDate(msg))
		{
			GetQuery().Initialize();
			BINDPARAM_LONG(GetQuery(), msg.m_ID);
			BINDPARAM_LONG(GetQuery(), lAgentID);
			GetQuery().Execute(_T("SELECT InboundMessageReadID FROM InboundMessageRead WHERE (InboundMessageID = ?) AND (AgentID = ?)"));

			if(GetQuery().Fetch() == S_OK)
				GetXMLGen().AddChildAttrib( _T("beenread"), 1);
			else
				GetXMLGen().AddChildAttrib( _T("beenread"), 0);
			
			GetXMLGen().AddChildAttrib( _T("usecuttoffdate"),0);
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("beenread"), 1);
			GetXMLGen().AddChildAttrib( _T("usecuttoffdate"),1);
		}
	}
	else
	{
		GetXMLGen().AddChildAttrib( _T("beenread"), 1);
	}

	GetXMLGen().AddChildAttrib( _T("Priority"), msg.m_Prority );
	AddPriority( _T("PriorityName"), msg.m_Prority );

	if(msg.m_IsInbound)
	{
		GetXMLGen().AddChildAttrib( _T("ContactID"), msg.m_ContactID );
	}
	else
	{
		if(msg.m_AgentID == 0)
		{
			GetXMLGen().AddChildAttrib( _T("AgentName"), _T("System") );
		}
		else
		{
			AddAgentName( _T("AgentName"), msg.m_AgentID, _T("System") );
		}
	}
					  
	
	GetXMLGen().AddChildAttrib( _T("EmailAddr"), msg.m_EmailAddr );

	sSubject.assign( msg.m_Subject );
	sSubject.EscapeHTML();
	sSubject.EscapeJavascript(false);
	GetXMLGen().AddChildAttrib( _T("Subject"), sSubject.c_str() );

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;

	if(lTzBias != 1)
	{
		if(ConvertToTimeZone(msg.m_Date,lTzBias,tsLocal))
		{
			GetDateTimeString( tsLocal, tsLocalLen, sDateTime );
		}
		else
		{
			GetDateTimeString( msg.m_Date, msg.m_DateLen, sDateTime);
		}
	}
	else
	{
		GetDateTimeString( msg.m_Date, msg.m_DateLen, sDateTime);
	}

	GetXMLGen().AddChildAttrib( _T("Date"), sDateTime.c_str() );
	
	GetXMLGen().AddChildAttrib( _T("AttachCount"), msg.m_AttachmentCount );

	
	GetXMLGen().AddChildAttrib( _T("IsDraft"), !msg.m_IsInbound && msg.m_OutboundMsgStateID == EMS_OUTBOUND_MESSAGE_STATE_DRAFT );
	GetXMLGen().AddChildAttrib( _T("OutboundType"), msg.m_OutboundMsgTypeID );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add info about the ticket to the XML	              
\*--------------------------------------------------------------------------*/
void CTicketHistory::GetTicketInfo( int nTicketID )
{
	// query for ticket information
	// also checks security
	CTicket tkt(m_ISAPIData);	
	tkt.m_TicketID = nTicketID;
	tkt.Query();

	if ( m_bShowTicketDetails)
	{
		// output XML
		tkt.GenerateXML();
	}
	else
	{
		GetXMLGen().AddChildElem(_T("Ticket"));
		GetXMLGen().SetChildAttrib( _T("ID"), nTicketID );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the Ticket Notes for the specified Ticket	    
||				Assumes that a security check has already been preformed          
\*--------------------------------------------------------------------------*/
void CTicketHistory::ListTicketNotes( int nTicketID )
{
	TTicketNotes Note;
	CEMSString sDateCreated;
	std::list<TTicketNotes> noteList;
	int nNote = 0;
	int nUnreadNote = 0;
	
	GetUnReadNoteCount(nTicketID, nNote, nUnreadNote);
	
	GetQuery().Initialize();
	
	BINDCOL_LONG(GetQuery(), Note.m_TicketNoteID );
	BINDCOL_LONG(GetQuery(), Note.m_NumAttach );
	BINDCOL_TIME(GetQuery(), Note.m_DateCreated );
	BINDCOL_LONG(GetQuery(), Note.m_AgentID );
	BINDCOL_BIT( GetQuery(), Note.m_IsVoipNote );
	
	BINDPARAM_LONG(GetQuery(), nTicketID );
	
	// execute the query
	GetQuery().Execute( _T("SELECT TicketNoteID, (SELECT COUNT(1) FROM NoteAttachments WHERE NoteID=TicketNoteID AND NoteTypeID=1),DateCreated, AgentID, IsVoipNote, Note ")
		_T("FROM TicketNotes ")
		_T("WHERE TicketID = ? ")
		_T("ORDER BY DateCreated DESC"));
	
	GetXMLGen().AddChildElem( _T("TicketNotes") );
	GetXMLGen().AddChildAttrib(_T("unreadnotecount"), nUnreadNote);
	GetXMLGen().AddChildAttrib(_T("notecount"), nNote);
	GetXMLGen().IntoElem();

	int nThreads = 0;

	while( ( GetQuery().Fetch() ) == S_OK )
	{
		GETDATA_TEXT( GetQuery(), Note.m_Note );

		noteList.push_back(Note);
	}

	std::list<TTicketNotes>::iterator iNote;
	bool m_IsVoipNote = false;

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;
		
	for(iNote = noteList.begin();
		iNote != noteList.end();
		iNote++)
	{
		// have we reached the max message thread count?
		if ( (m_nMaxNoteThreads) && (nThreads >= m_nMaxNoteThreads) )
		{
			GetXMLGen().AddChildElem( _T("More") );
			GetXMLGen().AddChildAttrib( _T("Description"), _T("More notes are avaliable") );
			break;
		}

		// if the note is too long, truncate it
		if( (*iNote).m_NoteLen > 128 )
		{
			// add ellipsis
			(*iNote).m_Note[128] = _T('\0');
			(*iNote).m_Note[127] = _T('.');
			(*iNote).m_Note[126] = _T('.');
			(*iNote).m_Note[125] = _T('.');
		}

		GetXMLGen().AddChildElem( _T("TicketNote") );
		GetXMLGen().AddChildAttrib( _T("TicketNoteID"), (*iNote).m_TicketNoteID );
		GetXMLGen().AddChildAttrib( _T("NumAttach"), (*iNote).m_NumAttach );
		if((*iNote).m_IsVoipNote)
			m_IsVoipNote = true;
		
		GetXMLGen().AddChildAttrib( _T("IsVoipNote"), m_IsVoipNote );

		if(UseCutoffDate((*iNote)))
		{
			GetXMLGen().AddChildAttrib( _T("beenread"), 1);
		}
		else
			GetXMLGen().AddChildAttrib( _T("beenread"), SetTicketNoteBeenRead((*iNote).m_TicketNoteID));

		if(lTzBias != 1)
		{
			if(ConvertToTimeZone((*iNote).m_DateCreated,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDateCreated );
			}
			else
			{
				GetDateTimeString( (*iNote).m_DateCreated, (*iNote).m_DateCreatedLen, sDateCreated );
			}
		}
		else
		{
			GetDateTimeString( (*iNote).m_DateCreated, (*iNote).m_DateCreatedLen, sDateCreated );
		}
			
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDateCreated.c_str() );
		AddAgentName( _T("Agent"), (*iNote).m_AgentID, _T("Not Available") );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("Preview") );
		GetXMLGen().SetChildData( (*iNote).m_Note, TRUE );

		GetXMLGen().OutOfElem();

		nThreads++;
	}

	GetXMLGen().OutOfElem();
}

void CTicketHistory::GetUnReadNoteCount(int nTicketID, int& nNC, int& nUNC)
{
	TIMESTAMP_STRUCT ts;
	long tsLen = sizeof(TIMESTAMP_STRUCT);
	ZeroMemory(&ts, sizeof(TIMESTAMP_STRUCT));

	int nCountMsg = 0;
	int nTotalCountMsg = 0;
	int nCountNote = 0;
	int nTotalCountNote = 0;
	long lAgentID = GetISAPIData().m_pSession->m_AgentID;

	if(CXMLDataClass::UseCutoffDate(ts))
	{
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), lAgentID);
		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);

		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);

		BINDCOL_LONG_NOLEN(GetQuery(), nCountNote);
		BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountNote);

		CEMSString sQuery;
		sQuery.Format(_T("SELECT (SELECT COUNT(*) FROM TicketNotesRead INNER JOIN TicketNotes ON TicketNotesRead.TicketNoteID = TicketNotes.TicketNoteID INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (TicketNotesRead.AgentID = ?) AND (Tickets.TicketID = ?) AND (TicketNotes.DateCreated > ?)), ")
					  _T("(SELECT COUNT(*) FROM TicketNotes WHERE (TicketID = ?) AND (TicketNotes.DateCreated > ?))"));

		GetQuery().Execute(sQuery.c_str());
		GetQuery().Fetch();
	}
	else
	{
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), lAgentID);
		BINDPARAM_LONG(GetQuery(), nTicketID);

		BINDPARAM_LONG(GetQuery(), nTicketID);

		BINDCOL_LONG_NOLEN(GetQuery(), nCountNote);
		BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountNote);

		CEMSString sQuery;
		sQuery.Format(_T("SELECT (SELECT COUNT(*) FROM TicketNotesRead INNER JOIN TicketNotes ON TicketNotesRead.TicketNoteID = TicketNotes.TicketNoteID INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (TicketNotesRead.AgentID = ?) AND (Tickets.TicketID = ?)), ")
					  _T("(SELECT COUNT(*) FROM TicketNotes WHERE (TicketID = ?))"));

		GetQuery().Execute(sQuery.c_str());
		GetQuery().Fetch();
	}

	nNC = nTotalCountNote;
	nUNC = nTotalCountNote - nCountNote;
}

int CTicketHistory::SetTicketNoteBeenRead(long lTicketNote)
{
	// has this message been read by agent
	int beenRead = 0;
	long lAgentID = GetISAPIData().m_pSession->m_AgentID;
	long lBeenReadID;
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), lTicketNote );
	BINDPARAM_LONG( GetQuery(), lAgentID );
	BINDCOL_LONG_NOLEN( GetQuery(), lBeenReadID );
	GetQuery().Execute(_T("SELECT TicketNotesReadID FROM TicketNotesRead WHERE TicketNoteID = ? AND AgentID = ?"));
	if(GetQuery().Fetch() == S_OK)
		beenRead = 1;

	return beenRead;
}

bool CTicketHistory::UseCutoffDate(CTicketHistoryMsg& msg)
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

		recvTime.wYear = msg.m_Date.year;
		recvTime.wDay = msg.m_Date.day;
		recvTime.wMonth = msg.m_Date.month;
		recvTime.wHour = msg.m_Date.hour;
		recvTime.wMinute = msg.m_Date.minute;
		recvTime.wSecond = msg.m_Date.second;

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

bool CTicketHistory::UseCutoffDate(TTicketNotes& notes)
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

		recvTime.wYear = notes.m_DateCreated.year;
		recvTime.wDay = notes.m_DateCreated.day;
		recvTime.wMonth = notes.m_DateCreated.month;
		recvTime.wHour = notes.m_DateCreated.hour;
		recvTime.wMinute = notes.m_DateCreated.minute;
		recvTime.wSecond = notes.m_DateCreated.second;

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


