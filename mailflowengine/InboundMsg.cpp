////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/InboundMsg.cpp,v 1.1.6.1 2006/07/18 12:50:09 markm Exp $
//
//  Copyright © 2000 Deerfield.com, all rights reserved
//
/////////////////////////////////////////////////////////////////////
//
// InboundMsg.cpp: implementation of the CInboundMsg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InboundMsg.h"
#include "AttachFns.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInboundMsg::CInboundMsg(CODBCQuery& query ) : m_query(query)
{
	szMatchEmailTo = NULL;
	szMatchEmailCc = NULL;
	szMatchEmailFrom = NULL;
}

CInboundMsg::~CInboundMsg()
{
	FreeMatchBuffers();
}

////////////////////////////////////////////////////////////////////////////////
// 
// FreeMatchBuffers
// 
////////////////////////////////////////////////////////////////////////////////
void CInboundMsg::FreeMatchBuffers(void)
{
	if( szMatchEmailTo )
		delete[] szMatchEmailTo;
	if( szMatchEmailFrom )
		delete[] szMatchEmailFrom;
	if( szMatchEmailCc )
		delete[] szMatchEmailCc;

	szMatchEmailTo = szMatchEmailFrom = szMatchEmailCc = NULL;
}
	
////////////////////////////////////////////////////////////////////////////////
// 
// AllocBuffer
// 
////////////////////////////////////////////////////////////////////////////////
void CInboundMsg::AllocBuffer( wchar_t* szSource, wchar_t** pszCopy )
{
	int i,nLen;
	wchar_t* szCopy;

	nLen = wcslen( szSource );

	// allocate a buffer
	szCopy = new wchar_t[nLen+1];

	// copy and convert to upper case
	for( i = 0; i < nLen; i++ )
	{
		szCopy[i] = towupper( szSource[i] );
	}
	// zero terminate
	szCopy[i] = 0;

	*pszCopy = szCopy;
}


////////////////////////////////////////////////////////////////////////////////
// 
// AllocMatchBuffers
// 
////////////////////////////////////////////////////////////////////////////////
void CInboundMsg::AllocMatchBuffers(void)
{
	AllocBuffer( m_EmailTo, &szMatchEmailTo );
	AllocBuffer( m_EmailCc, &szMatchEmailCc );
	AllocBuffer( m_EmailFrom, &szMatchEmailFrom );
}



////////////////////////////////////////////////////////////////////////////////
// 
// GetNewMessage - Called by routing engine
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CInboundMsg::GetNewMessage( tstring& sLockedMsgIDs )
{
	int nResult;
	CEMSString sSQL;

	// Initialize the structure
	m_MessageSourceTypeID = 0;
	m_InboundMessageID = 0;
	m_MessageSourceID = 0;
	m_Subject[0] = L'0';
	m_EmailFrom[0] = L'0';
	m_EmailFromName[0] = L'0';
	m_EmailPrimaryTo[0] = L'0';
	m_MediaType[0] = L'0';
	m_MediaSubType[0] = L'0';
	m_PriorityID = 0;
	
	m_query.Initialize();

	BINDCOL_LONG( m_query, m_InboundMessageID );
	BINDCOL_LONG( m_query, m_MessageSourceID );
	BINDCOL_WCHAR( m_query, m_Subject );
	BINDCOL_WCHAR( m_query, m_EmailFrom );
	BINDCOL_WCHAR( m_query, m_EmailFromName );
	BINDCOL_WCHAR( m_query, m_EmailPrimaryTo );
	BINDCOL_WCHAR( m_query, m_MediaType );
	BINDCOL_WCHAR( m_query, m_MediaSubType );
	BINDCOL_LONG( m_query, m_PriorityID );
	BINDCOL_TIME( m_query, m_DateReceived );
	BINDCOL_LONG( m_query, m_TicketID );
	BINDCOL_LONG( m_query, m_OriginalTicketCategoryID );
	BINDCOL_LONG( m_query, m_ReplyToMsgID );

	if(sLockedMsgIDs.size() > 0)
	{
		sSQL.Format( L"SELECT TOP 1 InboundMessages.InboundMessageID, MessageSourceID,"
					 L"Subject,EmailFrom,EmailFromName,EmailPrimaryTo,MediaType,"
					 L"MediaSubType,PriorityID,DateReceived,TicketID,OriginalTicketCategoryID,ReplyToMsgID,Body,EmailTo,EmailCc,PopHeaders,EmailReplyTo "
					 L"FROM InboundMessageQueue INNER JOIN InboundMessages ON InboundMessageQueue.InboundMessageID = InboundMessages.InboundMessageID "
					 L"WHERE InboundMessages.IsDeleted=0 AND InboundMessages.InboundMessageID NOT IN (%s) "
					 L"ORDER BY DateReceived", sLockedMsgIDs.c_str() );
	}
	else
	{
		sSQL.assign( L"SELECT TOP 1 InboundMessages.InboundMessageID, MessageSourceID,"
					 L"Subject,EmailFrom,EmailFromName,EmailPrimaryTo,MediaType,"
					 L"MediaSubType,PriorityID,DateReceived,TicketID,OriginalTicketCategoryID,ReplyToMsgID,Body,EmailTo,EmailCc,PopHeaders,EmailReplyTo "
					 L"FROM InboundMessageQueue INNER JOIN InboundMessages ON InboundMessageQueue.InboundMessageID = InboundMessages.InboundMessageID "
					 L"WHERE InboundMessages.IsDeleted=0 "
					 L"ORDER BY DateReceived" );
	}

	//dca::String e(sSQL.c_str());
	//dca::String f;
	//f.Format("CInboundMsg::GetNewMessage - query [ %s ]", e.c_str());
	//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);

	m_query.Execute( sSQL.c_str() );

	nResult = m_query.Fetch();

	// Get the Message Body
	if( nResult == S_OK )
	{
		GETDATA_TEXT( m_query, m_Body );
		GETDATA_TEXT( m_query, m_EmailTo );
		GETDATA_TEXT( m_query, m_EmailCc );
		GETDATA_TEXT( m_query, m_PopHeaders );
		GETDATA_TEXT( m_query, m_EmailReplyTo );

		FreeMatchBuffers();
		AllocMatchBuffers();

		// If email address empty, put something in there
		if( wcslen( m_EmailFrom ) == 0 )
		{
			wcscpy( m_EmailFrom, L"no email address" );
		}
	}
	else
	{
		//DebugReporter::Instance().DisplayMessage("CInboundMsg::GetNewMessage nothing to do.", DebugReporter::ENGINE);
	}
	
	return nResult;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetTicketAndRoutingRule
// 
////////////////////////////////////////////////////////////////////////////////
int CInboundMsg::SetRouteInfo( long TicketID, long RoutingRuleID, long ContactID,
							   long TicketBoxID, long OwnerID, long TicketCategoryID )
{
	m_query.Initialize();

	BINDPARAM_LONG( m_query, TicketID );
	BINDPARAM_LONG( m_query, RoutingRuleID );
	BINDPARAM_LONG( m_query, ContactID );	
	BINDPARAM_LONG( m_query, m_ReplyToMsgID );
	BINDPARAM_LONG( m_query, TicketBoxID );
	BINDPARAM_LONG( m_query, OwnerID );
	BINDPARAM_LONG( m_query, TicketCategoryID );
	BINDPARAM_LONG( m_query, m_InboundMessageID );

	m_query.Execute( L"UPDATE InboundMessages "
				   L"SET TicketID=?,"
				   L"RoutingRuleID=?,"
				   L"ContactID=?, "
				   L"ReplyToMsgID=?, "
				   L"OriginalTicketBoxID=?, "
				   L"OriginalOwnerID=?, "
				   L"OriginalTicketCategoryID=? "
				   L"WHERE InboundMessageID=?");

	return m_query.GetRowCount();
}

////////////////////////////////////////////////////////////////////////////////
// 
// RemoveFromInboundQueue
// 
////////////////////////////////////////////////////////////////////////////////
int CInboundMsg::RemoveFromInboundQueue( void )
{
	m_query.Initialize();

	BINDPARAM_LONG( m_query, m_InboundMessageID );
	
	m_query.Execute( L"DELETE FROM InboundMessageQueue "
				   L"WHERE InboundMessageID=?");

	return m_query.GetRowCount();
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete
// 
////////////////////////////////////////////////////////////////////////////////
int CInboundMsg::Delete( bool bToWasteBasket )
{
	int nRowsAffected;
	list<unsigned int> AttachmentIDList;
	list<unsigned int>::iterator iter;
	int nAttachID;
	int nRefCount;

	m_query.Initialize();

	
	if( bToWasteBasket )
	{
		TIMESTAMP_STRUCT now;
		GetTimeStamp(now);

		BINDPARAM_TIME_NOLEN( m_query, now );
		BINDPARAM_LONG( m_query, m_InboundMessageID );
		m_query.Execute( L"UPDATE InboundMessages "
					   L"SET IsDeleted=1,"
					   L"DeletedTime=?,"
					   L"DeletedBy=0 "
					   L"WHERE InboundMessageID=?" );
	}
	else
	{
		dca::WString sActualFilename;
		TCHAR szAttachLocation[ATTACHMENTS_ATTACHMENTLOCATION_LENGTH];
		long szAttachLocationLen;

		// Delete attachments
		BINDPARAM_LONG( m_query, m_InboundMessageID  );
		BINDCOL_LONG_NOLEN( m_query, nAttachID );
		BINDCOL_TCHAR( m_query, szAttachLocation );
		BINDCOL_LONG_NOLEN( m_query, nRefCount );
		m_query.Execute( L"SELECT A.AttachmentID,AttachmentLocation, "
						 L"((SELECT COUNT(*) FROM InboundMessageAttachments WHERE AttachmentID=A.AttachmentID) + "
						 L"(SELECT COUNT(*) FROM OutboundMessageAttachments WHERE AttachmentID=A.AttachmentID) + "
						 L"(SELECT COUNT(*) FROM StdResponseAttachments WHERE AttachmentID=A.AttachmentID)) "
						 L"FROM Attachments as A "
						 L"INNER JOIN InboundMessageAttachments as I ON A.AttachmentID = I.AttachmentID "
					     L"WHERE InboundMessageID=?" );
		
		while( m_query.Fetch() == S_OK )
		{
			if( nRefCount == 1 )
			{
				GetFullInboundAttachPath( szAttachLocation, sActualFilename );
				DeleteFile( sActualFilename.c_str() );
				AttachmentIDList.push_back( nAttachID );
			}
		}

		m_query.Reset(true );
		BINDPARAM_LONG( m_query, m_InboundMessageID  );
		m_query.Execute( L"DELETE FROM InboundMessageAttachments "
					     L"WHERE InboundMessageID=?");		

		for( iter = AttachmentIDList.begin(); iter != AttachmentIDList.end(); iter++ )
		{
			m_query.Reset(true );
			BINDPARAM_LONG( m_query, *iter );
			m_query.Execute( L"DELETE FROM Attachments "
						     L"WHERE AttachmentID=?");		
		}

		m_query.Reset(true );
		BINDPARAM_LONG( m_query, m_InboundMessageID  );
		m_query.Execute( L"DELETE FROM InboundMessages "
					     L"WHERE InboundMessageID=?");		
	}

	nRowsAffected = m_query.GetRowCount();

	return nRowsAffected;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetFirstMessageInTicket
// 
////////////////////////////////////////////////////////////////////////////////
bool CInboundMsg::GetFirstMessageInTicket( int TicketID, int& TicketBoxID )
{
	m_query.Initialize();
	BINDPARAM_LONG( m_query, TicketID );
	BINDCOL_LONG( m_query, m_InboundMessageID );
	BINDCOL_LONG_NOLEN( m_query, TicketBoxID );

	m_query.Execute( L"SELECT TOP 1 InboundMessageID,TicketBoxID "
		             L"FROM InboundMessages "
					 L"INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID "
					 L"WHERE Tickets.TicketID=? AND InboundMessages.IsDeleted=0 "
					 L"ORDER BY InboundMessageID" );

	if( m_query.Fetch() != S_OK )
	{
		// No undeleted inbound messages in ticket
		return false;
	}

	if( Query( m_query ) != S_OK )
	{
		// Error querying for message
		return false;
	}

	// Got it!
	return true;
}
