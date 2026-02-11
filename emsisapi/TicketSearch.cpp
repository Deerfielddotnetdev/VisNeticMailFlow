/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketSearch.cpp,v 1.3.2.1.2.1 2006/07/18 12:55:03 markm Exp $
||
||
||                                         
||  COMMENTS:  Ticket Search 
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketSearch.h"
#include "DateFns.h"
#include "Ticket.h"
#include ".\ticketsearch.h"
#include "TicketHistoryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CTicketSearch::CTicketSearch(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nODBCTimeout = 60;
	m_nMaxLines = EMS_DEFAULT_PAGE_ROWS;
	
	ZeroMemory( &m_CreatedStart, sizeof(TIMESTAMP_STRUCT) );
	ZeroMemory( &m_CreatedStop, sizeof(TIMESTAMP_STRUCT) );
	ZeroMemory( &m_OpenedStart, sizeof(TIMESTAMP_STRUCT) );
	ZeroMemory( &m_OpenedStop, sizeof(TIMESTAMP_STRUCT) );
	ZeroMemory( &m_EmailStart, sizeof(TIMESTAMP_STRUCT) );
	ZeroMemory( &m_EmailStop, sizeof(TIMESTAMP_STRUCT) );

	m_ExactSubject = 0;
	m_AndOr = 0;
	m_ExactMsgSubject = 0;
	m_ExactContactEmail = 0;
	m_ExactContactName = 0;
	m_nSearchMsgType = 0;
	m_nMessageID = 0;
	m_nTicketBoxID = 0;
	m_nOwnerID = -1;
	m_nPriorityID = 0;
	m_nTicketStateID = 0;
	m_nTicketCategoryID = -1;
	m_nTicketIDMatch = 0;
	m_UseCreatedDate = 0;
	m_UseOpenedDate = 0;
	m_UseEmailDate = 0;
	m_nTicketLockStateID = -1;
	m_nTicketLink = 0;
	nTicketLinkID = 0;
	m_ExactTicketField = 0;
	m_nFullText = 0;
}

CTicketSearch::~CTicketSearch()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CTicketSearch::Run(CURLAction& action)
{
	// get the action
	tstring sAction;
	
	GetISAPIData().GetXMLPost() ? GetISAPIData().GetXMLString( _T("ACTION"), sAction, true ) :
								  GetISAPIData().GetURLString( _T("ACTION"), sAction, true );

	if ( sAction.compare( _T("search") ) == 0 )
	{
		DecodeForm();
		
		GetXMLGen().AddChildElem( _T("TicketBoxItemList") );
		GetXMLGen().AddChildAttrib( _T("Type"), _T("Search") );
		GetXMLGen().AddChildAttrib( _T("SortBy"), m_nSortBy );
		GetXMLGen().AddChildAttrib( _T("SortAscending"), m_nSortAscending );
		GetXMLGen().AddChildAttrib( _T("RequireGetOldest"), _T("0") );
		
		PreformQuery();
	}
	else if ( sAction.compare( _T("delete") ) == 0 )
	{
		DeleteTicket();
	}
	else if ( sAction.compare( _T("restore") ) == 0 )
	{
		RestoreTicket();
	}
	else if ( sAction.compare( _T("unlock") ) == 0 )
	{
		UnlockTicket();
	}
	else if (sAction.compare(_T("takeownership")) == 0)
	{
		TakeOwnership();
	}
	else if ( sAction.compare( _T("escalate") ) == 0 )
	{
		Escalate();
	}
	else
	{
		dca::String sRequireDate;
		GetServerParameter(EMS_SRVPARAM_SEARCH_REQUIRE_DATE, sRequireDate, "1");
		int nRequireDate = sRequireDate.ToInt();
		GetXMLGen().AddChildElem( _T("Parameters") );
		GetXMLGen().AddChildAttrib( _T("MaxRecs"), GetSession().m_nMaxRowsPerPage );		
		GetXMLGen().AddChildAttrib( _T("FullText"), g_ThreadPool.GetSharedObjects().m_bFullText ? 1:0 );
		GetXMLGen().AddChildAttrib( _T("RequireDate"), nRequireDate );
		
		SetODBCSetting();
		GetXMLGen().AddChildAttrib( _T("ODBC_TIMEOUT"), m_nODBCTimeout );
	
		// xml for pulldowns
		ListAgentNames( EMS_READ_ACCESS );
		ListTicketBoxNames( EMS_READ_ACCESS, GetXMLGen() );
		ListTicketPriorities();
		ListTicketStates();
		//ListTicketCategoryNames( GetXMLGen() );

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

		TTicketFields tf;
		tf.PrepareList( GetQuery() );
		GetXMLGen().AddChildElem( _T("TicketFields") );
		GetXMLGen().IntoElem();
		while( GetQuery().Fetch() == S_OK )
		{
			GetXMLGen().AddChildElem( _T("TicketField") );
			GetXMLGen().AddChildAttrib( _T("ID"), tf.m_TicketFieldID );
			GetXMLGen().AddChildAttrib( _T("TYPE"), tf.m_TicketFieldTypeID );
			GetXMLGen().AddChildAttrib( _T("Description"), tf.m_Description );
		}
		GetXMLGen().OutOfElem();

		GetXMLDoc().AddElem( _T("DATA") );
		TTicketFieldOptions tfo;
		tfo.PrepareList( GetQuery() );
		GetXMLDoc().AddChildElem( _T("TicketFieldOptions") );
		GetXMLDoc().IntoElem();
		while( GetQuery().Fetch() == S_OK )
		{
			GetXMLDoc().AddChildElem( _T("TicketFieldOption") );
			GetXMLDoc().AddChildAttrib( _T("TFID"), tfo.m_TicketFieldID );
			GetXMLDoc().AddChildAttrib( _T("TFOID"), tfo.m_TicketFieldOptionID );
			GetXMLDoc().AddChildAttrib( _T("OptionValue"), tfo.m_OptionValue );
		}
		GetXMLDoc().OutOfElem();

		// add a default contact email address
		// if we were called from change ticket...
		if (m_ChangeTicketMsg.MsgID > 0)
			ChangeTicketDefaultEmailAddr();
	}

	// were we called from the change ticket dialog?
	if ( GetISAPIData().GetFormLong( _T("MsgID"), m_ChangeTicketMsg.MsgID, true ) ||
		GetISAPIData().GetURLLong( _T("MsgID"), m_ChangeTicketMsg.MsgID, true ) )
	{
		if ( m_ChangeTicketMsg.MsgID > 0 )
		{
			tstring sType;
			
			if (!GetISAPIData().GetFormString( _T("MsgType"), sType, true ))
				GetISAPIData().GetURLString( _T("MsgType"), sType );
			
			m_ChangeTicketMsg.IsInbound = (sType.compare( _T("inbound") ) == 0);
			
			// add the XML flag
			GetXMLGen().AddChildElem( _T("ChangeTicket") );
			GetXMLGen().AddChildAttrib( _T("MsgID"), m_ChangeTicketMsg.MsgID );
			GetXMLGen().AddChildAttrib(	_T("MsgType"), sType.c_str() );
		}
	}
	
	// are we merging tickets?
	if ( GetISAPIData().GetFormString( _T("MergeID"), m_sMergeID, true ) ||
		GetISAPIData().GetURLString( _T("MergeID"), m_sMergeID, true ) )
	{
		// add the XML flag
		GetXMLGen().AddChildElem( _T("MergeTicket") );
		GetXMLGen().AddChildAttrib( _T("MergeID"), m_sMergeID.c_str() );
	}

	// are we linking tickets?
	if ( GetISAPIData().GetFormString( _T("LinkID"), m_sLinkID, true ) ||
		GetISAPIData().GetURLString( _T("LinkID"), m_sLinkID, true ) )
	{
		int nTbView = 0;
		CEMSString sLinkName;
		GetISAPIData().GetURLString( _T("linkName"), sLinkName );
		sLinkName.TrimWhiteSpace();

		TCHAR szLinkName[GROUPS_GROUPNAME_LENGTH];
		_tcscpy( szLinkName, sLinkName.c_str() );
		
		// Check for duplicates
		GetQuery().Initialize();
		BINDPARAM_TCHAR( GetQuery(), szLinkName );
		GetQuery().Execute( _T("SELECT TicketLinkID FROM TicketLinks ")
							_T("WHERE LinkName=? ") );

		if( GetQuery().Fetch() == S_OK )
		{
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Link Name must be unique")  );
		}
				
		// add the XML flag
		GetXMLGen().AddChildElem( _T("LinkTicket") );
		GetXMLGen().AddChildAttrib( _T("LinkID"), m_sLinkID.c_str() );
		GetXMLGen().AddChildAttrib( _T("LinkName"), sLinkName.c_str() );
		if ( GetISAPIData().GetFormLong( _T("linkTbView"), nTbView, true ) ||
		GetISAPIData().GetURLLong( _T("linkTbView"), nTbView, true ) )
		{
			GetXMLGen().AddChildAttrib( _T("LinkTbView"), nTbView );
		}
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Read form parameters
\*--------------------------------------------------------------------------*/
void CTicketSearch::DecodeForm( void )
{
	//DebugReporter::Instance().DisplayMessage("CTicketSearch::DecodeForm - Entered function", DebugReporter::ISAPI, GetCurrentThreadId());

	GetISAPIData().GetURLString( _T("SUBJECT"), m_sSubject, true );
	GetISAPIData().GetURLLong( _T("SUBJECT_EXACT"), m_ExactSubject, true );
	GetISAPIData().GetURLString( _T("SUBJECT2"), m_sSubject2, true );
	GetISAPIData().GetURLLong( _T("ANDOR"), m_AndOr, true );
	GetISAPIData().GetURLLong( _T("FULLTEXT"), m_nFullText, true );
	
	GetISAPIData().GetURLString( _T("CONTACT_NAME"), m_sContactName, true );
	GetISAPIData().GetURLLong( _T("CONTACT_NAME_EXACT"), m_ExactContactName, true );
	
	GetISAPIData().GetURLString( _T("CONTACT_EMAIL"), m_sContactEmail, true );
	GetISAPIData().GetURLLong( _T("CONTACT_EMAIL_EXACT"), m_ExactContactEmail, true );
	
	GetISAPIData().GetURLString( _T("TICKETID"), m_sTicketID, true );
	GetISAPIData().GetURLLong( _T("TICKETLINK"), m_nTicketLink, true );	
	GetISAPIData().GetURLLong( _T("TICKETLINKID"), nTicketLinkID, true );	
	GetISAPIData().GetURLLong( _T("TICKETIDMATCH"), m_nTicketIDMatch, true );	

	GetISAPIData().GetURLLong( _T("TICKETBOXID"), m_nTicketBoxID, true );
	GetISAPIData().GetURLLong( _T("OWNERID"), m_nOwnerID, true );
	GetISAPIData().GetURLLong( _T("PRIORITYID"), m_nPriorityID, true );			
	GetISAPIData().GetURLLong( _T("STATEID"), m_nTicketStateID, true );
	GetISAPIData().GetURLLong( _T("TICKETCATEGORYID"), m_nTicketCategoryID, true );
	GetISAPIData().GetURLLong( _T("TICKETLOCKSTATEID"), m_nTicketLockStateID, true );
	
	GetISAPIData().GetURLString( _T("MSGSUBJECT"), m_sMsgSubject, true );
	GetISAPIData().GetURLLong( _T("MSGSUBJECT_EXACT"), m_ExactMsgSubject, true );
	GetISAPIData().GetURLString( _T("MESSAGE_BODY"), m_sMsgBody, true );
	GetISAPIData().GetURLLong( _T("MESSAGETYPE"), m_nSearchMsgType, true );
	GetISAPIData().GetURLLong( _T("MESSAGEID"), m_nMessageID, true );

	dca::String o;
	o.Format("CTicketSearch::DecodeForm - MessageType: [%d] MessageID: [%d]", m_nSearchMsgType, m_nMessageID );
	DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	GetISAPIData().GetURLString( _T("TICKET_NOTE"), m_sTicketNote, true );

	GetISAPIData().GetURLLong( _T("ODBC_TIMEOUT"), m_nODBCTimeout, true );
	GetISAPIData().GetURLLong( _T("MAXLINES"), m_nMaxLines, true );
	GetISAPIData().GetURLLong( _T("SORTBY"), m_nSortBy, true );
	GetISAPIData().GetURLLong( _T("SORTASCENDING"), m_nSortAscending, true );

	GetISAPIData().GetURLLong( _T("USE_CREATED_DATE"), m_UseCreatedDate, true );
	GetISAPIData().GetURLString( _T("TICKETFIELDID"), m_sTicketFieldID, true );
	int nTemp = 0;
	int i=0;
	while ( m_sTicketFieldID.CDLGetNextInt( nTemp ) )
	{
		if(i==0)
		{
			m_nTicketFieldID = nTemp;
	
		}
		else
		{
			m_nTicketFieldTypeID = nTemp;
		}
		i++;
	}

	m_nTicketFieldValue=0;
	m_sTicketFieldValue.Format(_T(""));
	
	switch(m_nTicketFieldTypeID)
	{
	case 1:
		GetISAPIData().GetURLString( _T("TICKETFIELDVALUE"), m_sTicketFieldValue, true );
		GetISAPIData().GetURLLong( _T("TF_EXACT"), m_ExactTicketField, true );
		break;
	case 2:
		GetISAPIData().GetURLLong( _T("TICKETFIELDVALUE"), m_nTicketFieldValue );
		break;
	case 3:
		GetISAPIData().GetURLLong( _T("TICKETFIELDVALUE"), m_nTicketFieldValue );
		break;
	}
	
	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;

	if ( m_UseCreatedDate )
	{
		CEMSString sDate;
		GetISAPIData().GetURLString( _T("CREATE_START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_CreatedStart );

		if(lTzBias != 1)
		{
			if(ConvertFromTimeZone(m_CreatedStart,lTzBias,tsLocal))
			{
				m_CreatedStart.year = tsLocal.year;
				m_CreatedStart.month = tsLocal.month;
				m_CreatedStart.day = tsLocal.day;
				m_CreatedStart.hour = tsLocal.hour;
				m_CreatedStart.minute = tsLocal.minute;
				m_CreatedStart.second = tsLocal.second;
			}			
		}

		GetISAPIData().GetURLString( _T("CREATE_END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_CreatedStop );

		m_CreatedStop.hour = 23;
		m_CreatedStop.minute = 59;
		m_CreatedStop.second = 59;

		if(lTzBias != 1)
		{
			if(ConvertFromTimeZone(m_CreatedStop,lTzBias,tsLocal))
			{
				m_CreatedStop.year = tsLocal.year;
				m_CreatedStop.month = tsLocal.month;
				m_CreatedStop.day = tsLocal.day;
				m_CreatedStop.hour = tsLocal.hour;
				m_CreatedStop.minute = tsLocal.minute;
				m_CreatedStop.second = tsLocal.second;
			}			
		}
	}

	GetISAPIData().GetURLLong( _T("USE_OPENED_DATE"), m_UseOpenedDate, true );

	if ( m_UseOpenedDate )
	{
		CEMSString sDate;
		GetISAPIData().GetURLString( _T("OPEN_START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_OpenedStart );

		if(lTzBias != 1)
		{
			if(ConvertFromTimeZone(m_OpenedStart,lTzBias,tsLocal))
			{
				m_OpenedStart.year = tsLocal.year;
				m_OpenedStart.month = tsLocal.month;
				m_OpenedStart.day = tsLocal.day;
				m_OpenedStart.hour = tsLocal.hour;
				m_OpenedStart.minute = tsLocal.minute;
				m_OpenedStart.second = tsLocal.second;
			}			
		}
		
		GetISAPIData().GetURLString( _T("OPEN_END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_OpenedStop );
		
		m_OpenedStop.hour = 23;
		m_OpenedStop.minute = 59;
		m_OpenedStop.second = 59;

		if(lTzBias != 1)
		{
			if(ConvertFromTimeZone(m_OpenedStop,lTzBias,tsLocal))
			{
				m_OpenedStop.year = tsLocal.year;
				m_OpenedStop.month = tsLocal.month;
				m_OpenedStop.day = tsLocal.day;
				m_OpenedStop.hour = tsLocal.hour;
				m_OpenedStop.minute = tsLocal.minute;
				m_OpenedStop.second = tsLocal.second;
			}			
		}
	}

	GetISAPIData().GetURLLong( _T("USE_EMAIL_DATE"), m_UseEmailDate, true );

	if ( m_UseEmailDate )
	{
		CEMSString sDate;
		GetISAPIData().GetURLString( _T("EMAIL_START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EmailStart );

		if(lTzBias != 1)
		{
			if(ConvertFromTimeZone(m_EmailStart,lTzBias,tsLocal))
			{
				m_EmailStart.year = tsLocal.year;
				m_EmailStart.month = tsLocal.month;
				m_EmailStart.day = tsLocal.day;
				m_EmailStart.hour = tsLocal.hour;
				m_EmailStart.minute = tsLocal.minute;
				m_EmailStart.second = tsLocal.second;
			}			
		}
		
		GetISAPIData().GetURLString( _T("EMAIL_END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EmailStop );
		
		m_EmailStop.hour = 23;
		m_EmailStop.minute = 59;
		m_EmailStop.second = 59;

		if(lTzBias != 1)
		{
			if(ConvertFromTimeZone(m_EmailStop,lTzBias,tsLocal))
			{
				m_EmailStop.year = tsLocal.year;
				m_EmailStop.month = tsLocal.month;
				m_EmailStop.day = tsLocal.day;
				m_EmailStop.hour = tsLocal.hour;
				m_EmailStop.minute = tsLocal.minute;
				m_EmailStop.second = tsLocal.second;
			}			
		}
	}

	//DebugReporter::Instance().DisplayMessage("CTicketSearch::DecodeForm - Leaving function", DebugReporter::ISAPI, GetCurrentThreadId());
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Perform Query and generate XML
||
||	TODO: Use bulk fetching?
\*--------------------------------------------------------------------------*/
void CTicketSearch::PreformQuery( void )
{
	CEMSString sQuery;
	CEMSString sDate;
	CEMSString sSubject;
	CEMSString sOpenTime;
	dca::String sTicketListDate;
	GetServerParameter(67, sTicketListDate, "0");
	bool bUseDefaultACL = false;

	if(sTicketListDate.ToInt() == 1)
	{
		sOpenTime.Format( _T("CASE WHEN (SELECT TOP 1 DateTime FROM TicketHistory WHERE TicketID=T.TicketID ")
					  _T("AND (TicketActionID=5 OR (TicketActionID=10 AND ID1=0)) ORDER BY TicketHistoryID DESC) > ")
					  _T("OpenTimeStamp THEN (SELECT TOP 1 DateTime FROM TicketHistory WHERE TicketID=T.TicketID ")
					  _T("AND (TicketActionID=5 OR (TicketActionID=10 AND ID1=0)) ORDER BY TicketHistoryID DESC) ELSE OpenTimeStamp ")
					  _T("END AS OpenTimeStamp") );
	}
	else if(sTicketListDate.ToInt() == 2)
	{
		sOpenTime.Format( _T("CASE WHEN (SELECT COUNT(*) FROM InboundMessages WHERE TicketID=T.TicketID) = 1 ")
					  _T("THEN (SELECT TOP 1 EmailDateTime FROM InboundMessages WHERE TicketID=T.TicketID) ")
					  _T("ELSE OpenTimeStamp ")
					  _T("END AS OpenTimeStamp") );
	}
	else
	{
		sOpenTime.Format( _T("OpenTimeStamp"));
	}

	TicketSearchList Ticket;

	//DebugReporter::Instance().DisplayMessage("CTicketSearch::PreformQuery - Entered function.", DebugReporter::ISAPI, GetCurrentThreadId());

	// we must have atleast read access to the ticket
	int nRequiredAccess = EMS_READ_ACCESS;

	if ( !GetIsAdmin() )
	{
		// do we have read access to the default ACL
		// we have to call tihs before GetQuery.Initialize() as it may preform a query
		bUseDefaultACL = GetAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, 0 ) >= nRequiredAccess;
	}
	
	// init the query
	GetQuery().Initialize();

	//DebugReporter::Instance().DisplayMessage("CTicketSearch::PreformQuery - Query initialized.", DebugReporter::ISAPI, GetCurrentThreadId());
	
	if ( m_nODBCTimeout )
		GetQuery().SetCustomTimeout( m_nODBCTimeout );

	/*dca::String t;
	t.Format("CTicketSearch::PreformQuery - Set custom timeout value to %d",  m_nODBCTimeout);
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/

	dca::String budda;

	// bind columns
	BINDCOL_LONG( GetQuery(), Ticket.m_TicketID );
	BINDCOL_LONG( GetQuery(), Ticket.m_TicketBoxID );
	BINDCOL_LONG( GetQuery(), Ticket.m_OwnerID );
	BINDCOL_LONG( GetQuery(), Ticket.m_LockedBy );	
	BINDCOL_LONG( GetQuery(), Ticket.m_PriorityID);
	BINDCOL_LONG( GetQuery(), Ticket.m_TicketStateID );
	BINDCOL_TCHAR( GetQuery(), Ticket.m_Subject );
	BINDCOL_TCHAR( GetQuery(), Ticket.m_Contacts );
	BINDCOL_TIME( GetQuery(), Ticket.m_OpenTimestamp );
	BINDCOL_TIME( GetQuery(), Ticket.m_DateCreated );
	BINDCOL_LONG( GetQuery(), Ticket.m_IsDeleted );
	BINDCOL_BIT( GetQuery(), Ticket.m_UseTickler );
	BINDCOL_LONG( GetQuery(), Ticket.m_TicketCategoryID );
	BINDCOL_LONG_NOLEN( GetQuery(), Ticket.m_MsgCount );
	BINDCOL_LONG_NOLEN( GetQuery(), Ticket.m_NoteCount );
	BINDCOL_LONG_NOLEN( GetQuery(), Ticket.m_LinkCount );
	BINDCOL_LONG_NOLEN( GetQuery(), Ticket.m_AttachCount );

	if( m_nTicketLink > 0)
	{
		BINDCOL_TCHAR( GetQuery(), Ticket.m_LinkDescr );
	}
		
	sQuery.reserve(256);

	if( m_nTicketLink > 0)
	{
		sQuery.Format( _T("SELECT TOP %d T.TicketID, T.TicketBoxID, T.OwnerID, T.LockedBy, ")
			_T("T.PriorityID, T.TicketStateID, T.Subject, T.Contacts, %s, ")
			_T("T.DateCreated, T.IsDeleted, T.UseTickler, T.TicketCategoryID, ")
			_T("(SELECT COUNT(*) FROM InboundMessages WHERE T.TicketID = InboundMessages.TicketID AND InboundMessages.IsDeleted=0) + ")
			_T("(SELECT COUNT(*) FROM OutboundMessages WHERE T.TicketID = OutboundMessages.TicketID AND OutboundMessages.IsDeleted=0) as MsgCount,  ") 
			_T("(SELECT COUNT(*) FROM TicketNotes WHERE T.TicketID = TicketNotes.TicketID) as NoteCount, ")
			_T("(SELECT COUNT(*) FROM TicketLinksTicket WHERE T.TicketID = TicketLinksTicket.TicketID) as LinkCount, ")
			_T("(SELECT COUNT(*) FROM InboundMessageAttachments ima INNER JOIN InboundMessages im ON ima.InboundMessageID=im.InboundMessageID WHERE ")
			_T("im.TicketID = T.TicketID AND im.IsDeleted = 0)+")			
			_T("(SELECT COUNT(*) FROM OutboundMessageAttachments oma INNER JOIN OutboundMessages om ON oma.OutboundMessageID=om.OutboundMessageID WHERE ")
			_T("om.TicketID = T.TicketID AND om.IsDeleted = 0) as AttachCount, ")		
			_T("TL.LinkName ")
			_T("FROM Tickets as T ")
			_T("INNER JOIN TicketLinksTicket as TLT ON T.TicketID=TLT.TicketID ")
			_T("INNER JOIN TicketLinks as TL ON  TLT.TicketLinkID=TL.TicketLinkID "), m_nMaxLines + 1,sOpenTime.c_str());
	}
	else
	{
		sQuery.Format( _T("SELECT TOP %d T.TicketID, T.TicketBoxID, T.OwnerID, T.LockedBy, ")
			_T("T.PriorityID, T.TicketStateID, T.Subject, T.Contacts, %s, ")
			_T("T.DateCreated, T.IsDeleted, T.UseTickler, T.TicketCategoryID, ")
			_T("(SELECT COUNT(*) FROM InboundMessages WHERE T.TicketID = InboundMessages.TicketID AND InboundMessages.IsDeleted=0) + ")
			_T("(SELECT COUNT(*) FROM OutboundMessages WHERE T.TicketID = OutboundMessages.TicketID AND OutboundMessages.IsDeleted=0) as MsgCount,  ") 
			_T("(SELECT COUNT(*) FROM TicketNotes WHERE T.TicketID = TicketNotes.TicketID) as NoteCount, ")
			_T("(SELECT COUNT(*) FROM TicketLinksTicket WHERE T.TicketID = TicketLinksTicket.TicketID) as LinkCount, ")
			_T("(SELECT COUNT(*) FROM InboundMessageAttachments ima INNER JOIN InboundMessages im ON ima.InboundMessageID=im.InboundMessageID WHERE ")
			_T("im.TicketID = T.TicketID AND im.IsDeleted = 0)+")			
			_T("(SELECT COUNT(*) FROM OutboundMessageAttachments oma INNER JOIN OutboundMessages om ON oma.OutboundMessageID=om.OutboundMessageID WHERE ")
			_T("om.TicketID = T.TicketID AND om.IsDeleted = 0) as AttachCount ")			
			_T("FROM Tickets as T "), m_nMaxLines + 1,sOpenTime.c_str());
	}
	

	// check security if the agent is not part
	// of the administrators group...
	//if ( !GetIsAdmin() )
	//{
		sQuery.append( _T("INNER JOIN TicketBoxes ON T.TicketBoxID = TicketBoxes.TicketBoxID ")
						_T("INNER JOIN TicketCategories ON T.TicketCategoryID = TicketCategories.TicketCategoryID ")
					   _T("INNER JOIN Objects ON TicketBoxes.ObjectID = Objects.ObjectID ") );
	//}	
						
	// format the where clause to
	// reflect the search criteria
	{
		dca::String where_clause;
		format_where_clause(where_clause);
		sQuery.append(where_clause.c_str());
	}

	//set the sort column and order
	tstring sSortBy;
	tstring sSortOrder;
	tstring sOrderBy;

	if( m_nTicketLink > 0)
	{
		sOrderBy= "  Order By TL.LinkName,T.TicketID";
	}
	else
	{
		if (m_nSortAscending == 0)
		{
			sSortOrder = "DESC";
		}
		switch(m_nSortBy)
		{
		case 0:
			sSortBy="T.Contacts";
			break;
		case 1:
			sSortBy="T.DateCreated";
			break;
		case 2:
			sSortBy="T.Subject";
			break;
		case 3:
			sSortBy="TicketBoxes.Name";
			break;
		case 4:
			sSortBy="TicketCategories.Description";
			break;
		case 5:
			sSortBy="T.TicketID";
			break;
		case 6:
			sSortBy="T.PriorityID";
			break;
		case 7:
			sSortBy="T.Contacts";
			break;
		default:
			sSortBy="T.DateCreated";
			break;
		}
		sOrderBy= " Order By " + sSortBy + " " + sSortOrder;
	}

	// check security if the agent is not part
	// of the administrators group...
	if ( !GetIsAdmin() )
	{
		sQuery.append( _T(" AND (") );

		// if the agent has rights to the "default ticketbox" the agent has rights
		// to all ticketboxes that use default rights
		if ( bUseDefaultACL )
		{
			sQuery.append( _T("UseDefaultRights=1 OR (UseDefaultRights=0 AND ") );
		}
		else
		{
			sQuery.append( _T("UseDefaultRights=0 AND (") );
		}
	
		BINDPARAM_LONG( GetQuery(), nRequiredAccess );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		
		sQuery.append(	_T("EXISTS (SELECT TOP 1 AccessControlID FROM AccessControl ")
						_T("WHERE ObjectID = TicketBoxes.ObjectID AND AccessLevel >=? AND (AgentID=? OR GroupID IN ")
						_T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=?)))))") );
		sQuery.append(sOrderBy);
		
		budda.assign(sQuery.c_str());
	}
	else{
		
		sQuery.append(sOrderBy);
		budda.assign(sQuery.c_str());
		dca::String::size_type pos = budda.find_last_of('?');
		if(pos != dca::String::npos){
			dca::String a(_T("'"));
			a.append(m_sContactEmail.c_str());
			a.append(_T("'"));
			budda.replace(pos, 1, a.c_str(), a.length());
		}
	}
	
	// execute the query
	//DebugReporter::Instance().DisplayMessage("CTicketSearch::PreformQuery - Preparing to execute query", DebugReporter::ISAPI, GetCurrentThreadId());

	dca::String t;
	t.Format("CTicketSearch::PreformQuery - The Budda Query [ %s ]",  budda.c_str());
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	t.erase();
	t.Format("CTicketSearch::PreformQuery - The sQuery Query [ %s ]",  sQuery.c_str());
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	GetQuery().Execute( sQuery.c_str() );
	
	// fetch the results
	GetXMLGen().IntoElem();
		
	CEMSString sNone(EMS_STRING_LIST_NONE);

	std::list<TicketSearchList> ticketList;
	bool bIsAdded = false;
	int nLines = 0;
	for ( nLines = 0; GetQuery().Fetch() == S_OK; nLines++ )
	{
		if ( nLines < m_nMaxLines )
		{
			if( m_nTicketLink == 1)
			{
				if(Ticket.m_TicketID == nTicketID)
				{
					if(!bIsAdded)
					{
						_tcscpy( Ticket.m_LinkDescr, _T("[Source Ticket]") );
						ticketList.push_back(Ticket);
						bIsAdded = true;
					}
				}
				else
				{
					ticketList.push_back(Ticket);
				}
			}
			else
			{
				ticketList.push_back(Ticket);
			}
		}
	}

	std::list<TicketSearchList>::iterator iterTickets;

	{
		/*dca::String a;
		a.Format("CTicketSearch::PreformQuery - Number of tickets found = %d", ticketList.size());
		DebugReporter::Instance().DisplayMessage(a.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
	}

	bool bLinkNameAdded = false;
	CEMSString sLinkName;

	for(iterTickets = ticketList.begin();
		iterTickets != ticketList.end();
		iterTickets++)
	{
		GetXMLGen().AddChildElem( _T("Item") );
		GetXMLGen().AddChildAttrib( _T("ID"), (*iterTickets).m_TicketID );
		
		AddTicketBoxName( _T("TicketBox"), (*iterTickets).m_TicketBoxID );
		AddAgentName( _T("Owner"), (*iterTickets).m_OwnerID, sNone.c_str() );
		
		sSubject.assign( (*iterTickets).m_SubjectLen ? (*iterTickets).m_Subject : _T("[No Subject]") );
		sSubject.EscapeHTML();
		GetXMLGen().AddChildAttrib( _T("Subject"), sSubject.c_str() );

		GetXMLGen().AddChildAttrib( _T("Contact"), (*iterTickets).m_ContactsLen ? (*iterTickets).m_Contacts : _T("[No Contacts]") );
		
		if( ((*iterTickets).m_UseTickler) && ((*iterTickets).m_TicketStateID == 1))
		{
			GetXMLGen().AddChildAttrib( _T("State"), (*iterTickets).m_TicketStateID + 6 );
			GetXMLGen().AddChildAttrib( _T("StateName"), _T("Closed - Set To Reopen") );
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("State"), (*iterTickets).m_TicketStateID );
			AddTicketStateName( _T("StateName"), (*iterTickets).m_TicketStateID );
		}
		
		GetXMLGen().AddChildAttrib( _T("Priority"), (*iterTickets).m_PriorityID );
		AddPriority( _T("PriorityName"), (*iterTickets).m_PriorityID );

		GetXMLGen().AddChildAttrib( _T("TicketCategory"), (*iterTickets).m_TicketCategoryID );
		AddTicketCategoryName( _T("TicketCategoryName"), (*iterTickets).m_TicketCategoryID );

		GetDateTimeString( (*iterTickets).m_OpenTimestamp, (*iterTickets).m_OpenTimestampLen, sDate );
		GetXMLGen().AddChildAttrib( _T("Date"), sDate.c_str() );

		GetDateTimeString( (*iterTickets).m_DateCreated, (*iterTickets).m_DateCreatedLen, sDate );
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDate.c_str() );

		GetXMLGen().AddChildAttrib( _T("LockedBy"),  (*iterTickets).m_LockedBy);
		AddAgentName( _T("LockedByName"), (*iterTickets).m_LockedBy);
		
		GetXMLGen().AddChildAttrib( _T("MsgCount"), (*iterTickets).m_MsgCount );
		GetXMLGen().AddChildAttrib( _T("NoteCount"), (*iterTickets).m_NoteCount );
		GetXMLGen().AddChildAttrib( _T("LinkCount"), (*iterTickets).m_LinkCount );
		GetXMLGen().AddChildAttrib( _T("AttachCount"), (*iterTickets).m_AttachCount );
		GetXMLGen().AddChildAttrib( _T("IsDeleted"), (*iterTickets).m_IsDeleted );

		int nUnreadMsg = 0;
		int nUnreadNote = 0;
		GetUnReadMessageCountAndGetUnReadNoteCount((*iterTickets).m_TicketID, nUnreadMsg, nUnreadNote);

		if(nUnreadMsg)
			GetXMLGen().AddChildAttrib( _T("hasunread") , 1);
		else
			GetXMLGen().AddChildAttrib( _T("hasunread") , 0);

		GetXMLGen().AddChildAttrib(_T("unreadcount"), nUnreadMsg);

		if(nUnreadNote)
			GetXMLGen().AddChildAttrib( _T("hasunreadnotes") , 1);
		else
			GetXMLGen().AddChildAttrib( _T("hasunreadnotes") , 0);

		GetXMLGen().AddChildAttrib(_T("unreadnotecount"), nUnreadNote);
		
		if( m_nTicketLink > 0)
		{
			CEMSString sLinkDescr;
			sLinkDescr.assign( (*iterTickets).m_LinkDescr );
			sLinkDescr.EscapeHTML();
			GetXMLGen().AddChildAttrib( _T("LinkDescr"), sLinkDescr.c_str() );
			if( m_nTicketLink == 2 && !bLinkNameAdded )
			{
				sLinkName = sLinkDescr;
				bLinkNameAdded = true;
			}
		}		
	}

	GetXMLGen().OutOfElem();

	// do we need to add the more tag
	if ( nLines > m_nMaxLines )
	{
		CEMSString sMoreDesc;
		sMoreDesc.Format( _T("Maxium search result count reached (%d)."), m_nMaxLines );
		GetXMLGen().AddChildElem( _T("More") );
		GetXMLGen().AddChildAttrib( _T("Description"), sMoreDesc.c_str() );
	}
	
	if(bLinkNameAdded)
	{
		GetXMLGen().AddChildElem( _T("LinkTicket") );
		GetXMLGen().AddChildAttrib( _T("LinkName"), sLinkName.c_str() );
		GetXMLGen().AddChildAttrib( _T("LinkID"), nTicketLinkID );
	}
	
	//DebugReporter::Instance().DisplayMessage("CTicketSearch::PreformQuery - XML data was generated.", DebugReporter::ISAPI, GetCurrentThreadId());

	//DebugReporter::Instance().DisplayMessage("CTicketSearch::PreformQuery - Leaving function", DebugReporter::ISAPI, GetCurrentThreadId());
}

//*****************************************************************************                     
// Format the query's where clause to reflect the search criteria.  Also 
// preform necessary binding.

void CTicketSearch::format_where_clause(dca::String& where_clause)
{
	where_clause.reserve(512);

	if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
	{
		where_clause.assign("WHERE T.IsDeleted = 0");
	}
	else
	{
		if(m_nTicketStateID == EMS_TICKETSTATEID_DELETED)
			where_clause.assign("WHERE T.IsDeleted = 1");
	}
	
	if ( m_sTicketID.length() )
	{
		CEMSString sTicketIDs;
		
		switch(m_nTicketIDMatch)
		{
		case 1:
			m_sTicketID.CDLGetNextInt( nTicketID );
			if(!where_clause.length())
			{
				sTicketIDs.Format( _T("WHERE T.TicketID < %d"), nTicketID );
			}
			else
				sTicketIDs.Format( _T(" AND T.TicketID < %d"), nTicketID );
			break;
		case 2:
			m_sTicketID.CDLGetNextInt( nTicketID );
			if(!where_clause.length())
			{
				sTicketIDs.Format( _T("WHERE T.TicketID > %d"), nTicketID );
			}
			else
				sTicketIDs.Format( _T(" AND T.TicketID > %d"), nTicketID );
			break;
		default:
			if( m_nTicketLink == 1)
			{
				m_sTicketID.CDLGetNextInt( nTicketID );
				if(!where_clause.length())
				{
					sTicketIDs.Format( _T("WHERE TL.TicketLinkID IN (SELECT TicketLinkID FROM TicketLinksTicket WHERE TicketID = %d)"), nTicketID );
				}
				else
				{
					sTicketIDs.Format( _T(" AND TL.TicketLinkID IN (SELECT TicketLinkID FROM TicketLinksTicket WHERE TicketID = %d)"), nTicketID );
				}
			}			
			else
			{
				if(!where_clause.length())
				{
					sTicketIDs.Format( _T("WHERE T.TicketID IN(%s)"), m_sTicketID.c_str() );
				}
				else
				{
					sTicketIDs.Format( _T(" AND T.TicketID IN(%s)"), m_sTicketID.c_str() );
				}
			}			
			break;
		}

		where_clause.append( sTicketIDs );
	}

	if( nTicketLinkID > 0)
	{
		CEMSString sTicketIDs;
		if(!where_clause.length())
		{
			sTicketIDs.Format( _T("WHERE TL.TicketLinkID = %d"), nTicketLinkID );
		}
		else
		{
			sTicketIDs.Format( _T(" AND TL.TicketLinkID = %d"), nTicketLinkID );
		}
		where_clause.append( sTicketIDs );
	}
	
	if( m_sSubject.length() > 0 )
	{
		m_sSubject.EscapeSQL();		

		if ( m_ExactSubject )
		{
			BINDPARAM_TCHAR( GetQuery(), (void*) m_sSubject.c_str() );
			if(where_clause.length())
				where_clause.append( _T(" AND T.Subject=?") );
			else
				where_clause.append( _T("WHERE T.Subject=?") );			
		}
		else
		{
			if( m_nFullText == 1 )
			{
				CEMSString sFullText;
				//sFullText.Format( _T(" ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CONTAINS(Subject,'\"%s\"')) )"),m_sMsgSubject.c_str() );
				if(where_clause.length())
				{
					sFullText.Format( _T(" AND (CONTAINS(T.Subject,'\"%s\"')"),m_sSubject.c_str());
				}
				else
				{
					sFullText.Format( _T("WHERE (CONTAINS(T.Subject,'\"%s\"')"),m_sSubject.c_str());
				}
				
				where_clause.append( sFullText );
				
				if( m_sSubject2.length() > 0 )
				{
					m_sSubject2.EscapeSQL();
					
					if ( m_AndOr )
					{
						sFullText.Format( _T(" OR CONTAINS(T.Subject,'\"%s\"'))"),m_sSubject.c_str());
					}
					else
					{
						sFullText.Format( _T(" AND CONTAINS(T.Subject,'\"%s\"'))"),m_sSubject.c_str());
					}
				}
				else
				{
					sFullText.Format( _T(")") );
				}
				where_clause.append( sFullText );
			}
			else
			{
				BINDPARAM_TCHAR( GetQuery(), (void*) m_sSubject.c_str() );			
				if(where_clause.length())
					where_clause.append( _T(" AND (CHARINDEX(?,T.Subject)>0") );
				else
					where_clause.append( _T("WHERE (CHARINDEX(?,T.Subject)>0") );
				
				if( m_sSubject2.length() > 0 )
				{
					m_sSubject2.EscapeSQL();
					
					BINDPARAM_TCHAR( GetQuery(), (void*) m_sSubject2.c_str() );
					
					if ( m_AndOr )
					{
						where_clause.append( _T(" OR CHARINDEX(?,T.Subject)>0)") );
					}
					else
					{
						where_clause.append( _T(" AND CHARINDEX(?,T.Subject)>0)") );
					}
				}
				else
				{
					where_clause.append( _T(")") );
				}
			}
		}
	}

	if ( m_ISAPIData.m_pSession->m_RequireGetOldest )
	{
		dca::String sEnableGetOldestSearch;
		GetServerParameter(93, sEnableGetOldestSearch, "0");
		if(sEnableGetOldestSearch.ToInt() == 0)
		{
			BINDPARAM_LONG( GetQuery(), GetISAPIData().m_pSession->m_AgentID );
			if(where_clause.length())
				where_clause.append( _T(" AND T.OwnerID=?") );
			else
				where_clause.append( _T("WHERE T.OwnerID=?") );
		}		
	}
	else if ( m_nOwnerID > -1 )
	{
		BINDPARAM_LONG( GetQuery(), m_nOwnerID );
		if(where_clause.length())
			where_clause.append( _T(" AND T.OwnerID=?") );
		else
			where_clause.append( _T("WHERE T.OwnerID=?") );
	}
	
	if ( m_nTicketCategoryID > -1 )
	{
		BINDPARAM_LONG( GetQuery(), m_nTicketCategoryID );
		if(where_clause.length())
			where_clause.append( _T(" AND T.TicketCategoryID=?") );
		else
			where_clause.append( _T("WHERE T.TicketCategoryID=?") );
	}
	
	if ( m_nTicketLockStateID > -1 )
	{
		BINDPARAM_LONG( GetQuery(), m_nTicketLockStateID );
		if ( m_nTicketLockStateID == 0 )
		{
			if(where_clause.length())
			{
				where_clause.append( _T(" AND T.LockedBy>0") );
			}
			else
			{
				where_clause.append( _T("WHERE T.LockedBy>0") );
			}
		}
		else
		{
			if(where_clause.length())
			{
				where_clause.append( _T(" AND T.LockedBy=0") );
			}
			else
			{
				where_clause.append( _T("WHERE T.LockedBy=0") );
			}
		}		
	}

	if ( m_nTicketFieldID > 0 )
	{
		BINDPARAM_LONG( GetQuery(), m_nTicketFieldID );
		if ( m_nTicketFieldTypeID == 1 )
		{
			m_sTicketFieldValue.EscapeSQL();

			BINDPARAM_TCHAR( GetQuery(), (void*) m_sTicketFieldValue.c_str() );
			if ( m_ExactTicketField )
			{
				if(where_clause.length())
				{
					where_clause.append( _T(" AND T.TicketID IN (SELECT TicketID FROM TicketFieldsTicket WHERE TicketFieldID=? AND DataValue = ?)") );
				}
				else
				{
					where_clause.append( _T("WHERE T.TicketID IN (SELECT TicketID FROM TicketFieldsTicket WHERE TicketFieldID=? AND DataValue = ?)") );
				}		
			}
			else
			{
				if(where_clause.length())
				{
					where_clause.append( _T(" AND T.TicketID IN (SELECT TicketID FROM TicketFieldsTicket WHERE TicketFieldID=? AND CHARINDEX(?,DataValue)>0)") );
				}
				else
				{
					where_clause.append( _T("WHERE T.TicketID IN (SELECT TicketID FROM TicketFieldsTicket WHERE TicketFieldID=? AND CHARINDEX(?,DataValue)>0)") );
				}
			}
		}
		else if ( m_nTicketFieldTypeID == 2 || m_nTicketFieldTypeID == 3 )
		{
			BINDPARAM_LONG( GetQuery(), m_nTicketFieldValue );
			if(where_clause.length())
			{
				where_clause.append( _T(" AND T.TicketID IN (SELECT TicketID FROM TicketFieldsTicket WHERE TicketFieldID=? AND DataValue = ?)") );
			}
			else
			{
				where_clause.append( _T("WHERE T.TicketID IN (SELECT TicketID FROM TicketFieldsTicket WHERE TicketFieldID=? AND DataValue = ?)") );
			}
		}
	}

	if ( m_nPriorityID > 0 )
	{
		BINDPARAM_LONG( GetQuery(), m_nPriorityID );
		if(where_clause.length())
			where_clause.append( _T(" AND T.PriorityID=?") );
		else
			where_clause.append( _T("WHERE T.PriorityID=?") );
	}
	
	if (m_nTicketBoxID > 0)
	{
		BINDPARAM_LONG( GetQuery(), m_nTicketBoxID);
		if(where_clause.length())
			where_clause.append( _T(" AND T.TicketBoxID=?") );
		else
			where_clause.append( _T("WHERE T.TicketBoxID=?") );
	}
	
	if (m_nTicketStateID > 0)
	{
		if( m_nTicketStateID != EMS_TICKETSTATEID_OPEN)
		{
			if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
			{
				BINDPARAM_LONG( GetQuery(), m_nTicketStateID );
				if(where_clause.length())
					where_clause.append( _T(" AND T.TicketStateID=?") );
				else
					where_clause.append( _T("WHERE T.TicketStateID=?") );
			}
		}
		else
		{	
			// if the state is "open" also display escalated and on-hold items
			if(where_clause.length())
				where_clause.append( _T(" AND T.TicketStateID>1") );
			else
				where_clause.append( _T("WHERE T.TicketStateID>1") );
		}
	}

	if (m_UseCreatedDate)
	{
		BINDPARAM_TIME_NOLEN( GetQuery(), m_CreatedStart);
		BINDPARAM_TIME_NOLEN( GetQuery(), m_CreatedStop);

		if(where_clause.length())
			where_clause.append( _T(" AND T.DateCreated BETWEEN ? AND ?") );
		else
			where_clause.append( _T("WHERE T.DateCreated BETWEEN ? AND ?") );
	}

	if (m_UseOpenedDate)
	{
		BINDPARAM_TIME_NOLEN( GetQuery(), m_OpenedStart);
		BINDPARAM_TIME_NOLEN( GetQuery(), m_OpenedStop);
		
		if(where_clause.length())
			where_clause.append( _T(" AND T.OpenTimestamp BETWEEN ? AND ?") );
		else
			where_clause.append( _T("WHERE T.OpenTimestamp BETWEEN ? AND ?") );
	}

	if ( m_UseEmailDate )
	{
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EmailStart);
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EmailStop);
		
		if(where_clause.length())
			where_clause.append( _T(" AND TicketID IN (SELECT TOP 1 TicketID FROM InboundMessages WHERE TicketID=T.TicketID AND  EmailDateTime BETWEEN ? AND ?)") );
		else
			where_clause.append( _T("WHERE TicketID IN (SELECT TOP 1 TicketID FROM InboundMessages WHERE TicketID=T.TicketID AND  EmailDateTime BETWEEN ? AND ?)") );
		
	}

	if ( m_sContactName.length() > 0 || m_sContactEmail.length() > 0 )
	{
		
		if(where_clause.length())
			where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM TicketContacts WHERE ContactID IN (SELECT PersonalData.ContactID FROM PersonalData") );
		else
			where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM TicketContacts WHERE ContactID IN (SELECT PersonalData.ContactID FROM PersonalData") );
		
		if ( m_sContactName.length() > 0 )
			where_clause.append( _T(" INNER JOIN Contacts on TicketContacts.ContactID = Contacts.ContactID") );
		
		where_clause.append( _T(" WHERE") );
	}
	
	if ( m_sContactName.length() > 0 )
	{
		m_sContactName.EscapeSQL();
		
		where_clause.append( m_ExactContactName ? _T(" Contacts.Name=?") : _T(" Contacts.Name LIKE ?") );
		
		if ( !m_ExactContactName )
		{
			m_sContactName.insert( 0, _T("%") );
			m_sContactName.append( _T("%") );
		}

		BINDPARAM_TCHAR( GetQuery(), (void*) m_sContactName.c_str() );
	}
	
	if ( m_sContactEmail.length() > 0 )
	{
		m_sContactEmail.EscapeSQL();
		
		if ( m_sContactName.length() > 0 )
			where_clause.append( _T(" AND") );
		
		where_clause.append( _T(" PersonalDataTypeID=1 AND DataValue") );
		where_clause.append( m_ExactContactEmail ? _T("=?") : _T(" LIKE ?") );
		
		if ( !m_ExactContactEmail )
		{
			m_sContactEmail.insert( 0, _T("%") );
			m_sContactEmail.append( _T("%") );
		}

		BINDPARAM_TCHAR( GetQuery(), (void*)m_sContactEmail.c_str() );		
	}
	
	if ( m_sContactName.length() > 0 || m_sContactEmail.length() > 0 )
		where_clause.append( _T("))") );

	// search message subject
	if( m_sMsgSubject.length() > 0 )
	{
		m_sMsgSubject.EscapeSQL();

		if ( !m_ExactMsgSubject )
		{
			where_clause.append( where_clause.length() ? _T(" AND") : _T(" WHERE") );

			if( m_nFullText == 1 )
			{
				CEMSString sFullText;
				if ( m_nSearchMsgType == 0 )
				{
					if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
					{
						sFullText.Format( _T(" ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CONTAINS(Subject,'\"%s\"')) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CONTAINS(Subject,'\"%s\"')) )"),m_sMsgSubject.c_str(),m_sMsgSubject.c_str() );
						//where_clause.append( _T(" ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,Subject)>0 ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,Subject)>0) )") );
					}
					else
					{
						sFullText.Format( _T(" ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE CONTAINS(Subject,'\"%s\"')) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CONTAINS(Subject,'\"%s\"')) )"),m_sMsgSubject.c_str(),m_sMsgSubject.c_str() );
						//where_clause.append( _T(" ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE CHARINDEX(?,Subject)>0 ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CHARINDEX(?,Subject)>0) )") );
					}
				}
				else if ( m_nSearchMsgType == 1 )
				{
					if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
					{
						sFullText.Format( _T(" ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CONTAINS(Subject,'\"%s\"')) )"),m_sMsgSubject.c_str() );
						//where_clause.append( _T(" TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,Subject)>0) ") );
					}
					else
					{
						sFullText.Format( _T(" ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE CONTAINS(Subject,'\"%s\"')) )"),m_sMsgSubject.c_str() );
						//where_clause.append( _T(" TicketID IN (SELECT TicketID FROM InboundMessages WHERE CHARINDEX(?,Subject)>0) ") );
					}
				}
				else if ( m_nSearchMsgType == 2 )
				{
					if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
					{
						sFullText.Format( _T(" ( TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CONTAINS(Subject,'\"%s\"')) )"),m_sMsgSubject.c_str() );
						//where_clause.append( _T(" TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,Subject)>0) ") );
					}
					else
					{
						sFullText.Format( _T(" ( TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CONTAINS(Subject,'\"%s\"')) )"),m_sMsgSubject.c_str() );
						//where_clause.append( _T(" TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CHARINDEX(?,Subject)>0) ") );
					}
				}
				where_clause.append( sFullText );
			}
			else
			{
				if ( m_nSearchMsgType == 0 )
				{
					BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgSubject.c_str() );
					BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgSubject.c_str() );

					if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
					{
						where_clause.append( _T(" ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,Subject)>0 ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,Subject)>0) )") );
					}
					else
					{
						where_clause.append( _T(" ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE CHARINDEX(?,Subject)>0 ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CHARINDEX(?,Subject)>0) )") );
					}
				}
				else if ( m_nSearchMsgType == 1 )
				{
					BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgSubject.c_str() );
				
					if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
					{
						where_clause.append( _T(" TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,Subject)>0) ") );
					}
					else
					{
						where_clause.append( _T(" TicketID IN (SELECT TicketID FROM InboundMessages WHERE CHARINDEX(?,Subject)>0) ") );
					}
				}
				else if ( m_nSearchMsgType == 2 )
				{
					BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgSubject.c_str() );
				
					if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
					{
						where_clause.append( _T(" TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,Subject)>0) ") );
					}
					else
					{
						where_clause.append( _T(" TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CHARINDEX(?,Subject)>0) ") );
					}
				}
			}
		}
		else
		{
			if ( m_nSearchMsgType == 0 )
			{
				BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgSubject.c_str() );
				BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgSubject.c_str() );

				if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
				{
					if(where_clause.length())
						where_clause.append( _T(" AND ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND Subject =? ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND Subject =?) )") );
					else
						where_clause.append( _T("WHERE ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND Subject =? ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND Subject =?) )") );
				}
				else
				{
					if(where_clause.length())
						where_clause.append( _T(" AND ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE Subject =? ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE Subject =?) )") );
					else
						where_clause.append( _T("WHERE ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE Subject =? ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE Subject=?) )") );
				}
			}
			else if ( m_nSearchMsgType == 1 )
			{
				BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgSubject.c_str() );
			
				if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
				{
					if(where_clause.length())
						where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND Subject =?) ") );
					else
						where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND Subject =?) ") );
				}
				else
				{
					if(where_clause.length())
						where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM InboundMessages WHERE Subject =?) ") );
					else
						where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM InboundMessages WHERE Subject =?) ") );
				}
			}
			else if ( m_nSearchMsgType == 2 )
			{
				BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgSubject.c_str() );
			
				if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
				{
					if(where_clause.length())
						where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND Subject =?) ") );
					else
						where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND Subject =?) ") );
				}
				else
				{
					if(where_clause.length())
						where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM OutboundMessages WHERE Subject =?) ") );
					else
						where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM OutboundMessages WHERE Subject =?) ") );
				}
			}
		}		
	}

	if ( m_sMsgBody.length() )
	{
		m_sMsgBody.EscapeSQL();
		
		if( m_nFullText == 1 )
		{
			CEMSString sFullText;

			if ( m_nSearchMsgType == 0 )
			{
				if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
				{
					if(where_clause.length())
						sFullText.Format( _T(" AND ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CONTAINS(Body,'\"%s\"')) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CONTAINS(Body,'\"%s\"'))) "),m_sMsgBody.c_str(),m_sMsgBody.c_str() );
					else
						sFullText.Format( _T("WHERE ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CONTAINS(Body,'\"%s\"')) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CONTAINS(Body,'\"%s\"'))) "),m_sMsgBody.c_str(),m_sMsgBody.c_str() );
				}
				else
				{
					if(where_clause.length())
						sFullText.Format( _T(" AND ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE CONTAINS(Body,'\"%s\"')) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CONTAINS(Body,'\"%s\"'))) "),m_sMsgBody.c_str(),m_sMsgBody.c_str() );						
					else
						sFullText.Format( _T("WHERE ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE CONTAINS(Body,'\"%s\"')) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CONTAINS(Body,'\"%s\"'))) "),m_sMsgBody.c_str(),m_sMsgBody.c_str() );
				}
			}
			else if ( m_nSearchMsgType == 1 )
			{
				if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
				{
					if(where_clause.length())
						sFullText.Format( _T(" AND TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CONTAINS(Body,'\"%s\"')) "),m_sMsgBody.c_str() );
					else
						sFullText.Format( _T("WHERE TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CONTAINS(Body,'\"%s\"')) "),m_sMsgBody.c_str() );
				}
				else
				{
					if(where_clause.length())
						sFullText.Format( _T(" AND TicketID IN (SELECT TicketID FROM InboundMessages WHERE CONTAINS(Body,'\"%s\"')) "),m_sMsgBody.c_str() );
					else
						sFullText.Format( _T("WHERE TicketID IN (SELECT TicketID FROM InboundMessages WHERE CONTAINS(Body,'\"%s\"')) "),m_sMsgBody.c_str() );
				}
			}
			else if ( m_nSearchMsgType == 2 )
			{
				if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
				{
					if(where_clause.length())
						sFullText.Format( _T(" AND TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CONTAINS(Body,'\"%s\"')) "),m_sMsgBody.c_str() );
					else
						sFullText.Format( _T("WHERE TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CONTAINS(Body,'\"%s\"')) "),m_sMsgBody.c_str() );
				}
				else
				{
					if(where_clause.length())
						sFullText.Format( _T(" AND TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CONTAINS(Body,'\"%s\"')) "),m_sMsgBody.c_str() );
					else
						sFullText.Format( _T("WHERE TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CONTAINS(Body,'\"%s\"')) "),m_sMsgBody.c_str() );
				}
			}
			where_clause.append( sFullText );
		}
		else
		{
			if ( m_nSearchMsgType == 0 )
			{
				BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgBody.c_str() );
				BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgBody.c_str() );

				if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
				{
					if(where_clause.length())
						where_clause.append( _T(" AND ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0 ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) )") );
					else
						where_clause.append( _T("WHERE ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0 ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) )") );
				}
				else
				{
					if(where_clause.length())
						where_clause.append( _T(" AND ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0 ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) )") );
					else
						where_clause.append( _T("WHERE ( TicketID IN (SELECT TicketID FROM InboundMessages WHERE CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0 ) OR TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) )") );
				}
			}
			else if ( m_nSearchMsgType == 1 )
			{
				BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgBody.c_str() );
			
				if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
				{
					if(where_clause.length())
						where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) ") );
					else
						where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM InboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) ") );
				}
				else
				{
					if(where_clause.length())
						where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM InboundMessages WHERE CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) ") );
					else
						where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM InboundMessages WHERE CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) ") );
				}
			}
			else if ( m_nSearchMsgType == 2 )
			{
				BINDPARAM_TCHAR( GetQuery(), (void*) m_sMsgBody.c_str() );
			
				if(m_nTicketStateID != EMS_TICKETSTATEID_DELETED && m_nTicketStateID != 0)
				{
					if(where_clause.length())
						where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) ") );
					else
						where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM OutboundMessages WHERE IsDeleted=0 AND CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) ") );
				}
				else
				{
					if(where_clause.length())
						where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) ") );
					else
						where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM OutboundMessages WHERE CHARINDEX(?,CAST(Body AS VARCHAR(MAX)))>0) ") );
				}
			}
		}		
	}

	if ( m_sTicketNote.length() )
	{
		m_sTicketNote.EscapeSQL();
		
		BINDPARAM_TCHAR( GetQuery(), (void*) m_sTicketNote.c_str() );

		if(where_clause.length())
			where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM TicketNotes WHERE CHARINDEX(?,CAST(Note AS VARCHAR(MAX)))>0 )") );
		else
			where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM TicketNotes WHERE CHARINDEX(?,CAST(Note AS VARCHAR(MAX)))>0 )") );
	}

	// if we were called from the change ticket window
	// don't show the source ticket
	if ( m_ChangeTicketMsg.MsgID > 0 )
	{
		BINDPARAM_LONG( GetQuery(), m_ChangeTicketMsg.MsgID );
		
		if ( m_ChangeTicketMsg.IsInbound )
		{
			if(where_clause.length())
				where_clause.append( _T(" AND T.TicketID != (SELECT TicketID FROM InboundMessages WHERE InboundMessageID=?)") );
			else
				where_clause.append( _T("WHERE T.TicketID != (SELECT TicketID FROM InboundMessages WHERE InboundMessageID=?)") );
		}
		else
		{
			if(where_clause.length())
				where_clause.append( _T(" AND T.TicketID != (SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID=?)") );
			else
				where_clause.append( _T("WHERE T.TicketID != (SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID=?)") );
		}
	}

	// search for specified inbound or outbound messageID
	if ( m_nMessageID > 0 )
	{
		if ( m_nSearchMsgType == 1 )
		{
			BINDPARAM_LONG( GetQuery(), m_nMessageID );
		
			if(where_clause.length())
					where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM InboundMessages WHERE InboundMessageID=?) ") );
				else
					where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM InboundMessages WHERE InboundMessageID=?) ") );
		}
		else if ( m_nSearchMsgType == 2 )
		{
			BINDPARAM_LONG( GetQuery(), m_nMessageID );
		
			if(where_clause.length())
					where_clause.append( _T(" AND TicketID IN (SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID=?) ") );
				else
					where_clause.append( _T("WHERE TicketID IN (SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID=?) ") );
		}
	}	
}

//*****************************************************************************  

/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves the email address to pre-populate when called
||				from the change ticket window
\*--------------------------------------------------------------------------*/
void CTicketSearch::ChangeTicketDefaultEmailAddr( void )
{
	GetQuery().Initialize();

	TCHAR Address[PERSONALDATA_DATAVALUE_LENGTH];
	Address[0] = _T('\0');
	long AddressLen;

	BINDCOL_TCHAR( GetQuery(), Address );
	BINDPARAM_LONG( GetQuery(), m_ChangeTicketMsg.MsgID );

	if ( m_ChangeTicketMsg.IsInbound )
	{
		GetQuery().Execute( _T("SELECT (SELECT DataValue FROM PersonalData WHERE PersonalDataID = Contacts.DefaultEmailAddressID) ")
							_T("FROM InboundMessages ")
							_T("INNER JOIN Contacts on Contacts.ContactID = InboundMessages.ContactID ")
							_T("WHERE InboundMessageID=?") );
	}
	else
	{
		GetQuery().Execute( _T("SELECT TOP 1 (SELECT DataValue FROM PersonalData WHERE ")
							_T("PersonalDataID = Contacts.DefaultEmailAddressID) ")
							_T("FROM OutboundMessageContacts ")
							_T("INNER JOIN Contacts on Contacts.ContactID = OutboundMessageContacts.ContactID ")
							_T("WHERE OutboundMessageID=?") );
	}
	
	GetQuery().Fetch();

	GetXMLGen().AddChildElem( _T("DEFAULT_DATA") );
	GetXMLGen().AddChildAttrib( _T("CONTACT_EMAIL"), Address );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Delete a ticket
\*--------------------------------------------------------------------------*/
void CTicketSearch::DeleteTicket()
{
	// get the list of tickets	
	CEMSString sTicketIDs;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);
	
	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		Ticket.Delete(true);
	}	

	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
	GetXMLGen().AddChildElem( _T("Refresh") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Takes ownership of a ticket
\*--------------------------------------------------------------------------*/
void CTicketSearch::TakeOwnership( void )
{
	// get the list of tickets	
	CEMSString sTicketIDs;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);

	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		Ticket.Reassign( GetAgentID() );
	}

	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
	GetXMLGen().AddChildElem( _T("Refresh") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Escalate the selected tickets.	              
\*--------------------------------------------------------------------------*/
void CTicketSearch::Escalate( void )
{
	// get the list of tickets	
	CEMSString sTicketIDs;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);

	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		Ticket.Escalate();
	}

	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
	GetXMLGen().AddChildElem( _T("Refresh") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Restore Ticket	              
\*--------------------------------------------------------------------------*/

void CTicketSearch::RestoreTicket(void)
{
	//DEBUGPRINT(_T("CTicketSearch::RestoreTicket"));

	// get the list of tickets	
	CEMSString sTicketIDs;
	CEMSString sQuery;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);

	//DEBUGPRINT(_T("CTicketSearch::RestoreTicket - ticketIDs = %s"),sTicketIDs.c_str());

	GetQuery().Initialize();
	
	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		//DEBUGPRINT(_T("CTicketSearch::RestoreTicket - Ticket.m_TicketID = %d"),Ticket.m_TicketID);

		GetQuery().Reset();
		sQuery.Format( _T("UPDATE Tickets SET IsDeleted=0,TicketStateID=2,LockedBy=0 WHERE TicketID=%d"), Ticket.m_TicketID );
		GetQuery().Execute( sQuery.c_str() );

		// Remove any delete outbound message ticket history entries
		/*GetQuery().Reset();
		sQuery.Format( _T("DELETE from TicketHistory WHERE TicketActionID = 6 AND TicketID=%d AND ID1=0 AND ID2 IN (SELECT OutboundMessageID FROM OutboundMessages WHERE TicketID=%d)"), Ticket.m_TicketID, Ticket.m_TicketID );
		GetQuery().Execute( sQuery.c_str() );*/

		// Remove any delete inbound message ticket history entries
		/*GetQuery().Reset();
		sQuery.Format( _T("DELETE from TicketHistory WHERE TicketActionID = 6 AND TicketID=%d AND ID1=1 AND ID2 IN (SELECT InboundMessageID FROM InboundMessages WHERE TicketID=%d)"), Ticket.m_TicketID, Ticket.m_TicketID );
		GetQuery().Execute( sQuery.c_str() );*/

		GetQuery().Reset();
		sQuery.Format( _T("UPDATE InboundMessages SET IsDeleted=0 WHERE TicketID=%d"), Ticket.m_TicketID );
		GetQuery().Execute( sQuery.c_str() );

		GetQuery().Reset();
		sQuery.Format( _T("UPDATE OutboundMessages SET IsDeleted=0 WHERE TicketID=%d"), Ticket.m_TicketID );
		GetQuery().Execute( sQuery.c_str() );

		GetQuery().Reset();
		
		//Get the current Ticket Data
		Ticket.Query();

		// update the ticket history
		THRestoreTicket( GetQuery(), Ticket.m_TicketID, GetAgentID(), Ticket.m_TicketStateID, Ticket.m_TicketBoxID, Ticket.m_OwnerID, Ticket.m_PriorityID, Ticket.m_TicketCategoryID );

		//Log it to Agent Activity
		if( _ttoi( Ticket.sAal.c_str() ) > 0 )
		{
			if( _ttoi( Ticket.sTrs.c_str() ) > 0 )
			{
				Ticket.LogAgentAction(GetAgentID(),8,Ticket.m_TicketID,0,0,_T(""),_T(""));
			}
		}
	}

	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
	GetXMLGen().AddChildElem( _T("Refresh") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Unlock Ticket	              
\*--------------------------------------------------------------------------*/

void CTicketSearch::UnlockTicket(void)
{
	//DEBUGPRINT(_T("CTicketSearch::UnlockTicket"));

	// get the list of tickets	
	CEMSString sTicketIDs;
	CEMSString sQuery;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);

	//DEBUGPRINT(_T("CTicketSearch::UnlockTicket - ticketIDs = %s"),sTicketIDs.c_str());
	
	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		//DEBUGPRINT(_T("CTicketSearch::UnlockTicket - Ticket.m_TicketID = %d"),Ticket.m_TicketID);

		//Require Admin Rights
		RequireAdmin();
			
		Ticket.AdminUnLock();				
	}

	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
	GetXMLGen().AddChildElem( _T("Refresh") );
}

int CTicketSearch::SetODBCSetting(void)
{
	TServerParameters servParams;
	servParams.m_ServerParameterID = 40;

	int nResult = servParams.Query(GetQuery());

	if(nResult != S_OK)
		lstrcpy(servParams.m_DataValue, "60");

	m_nODBCTimeout = atoi(servParams.m_DataValue);

	return 0;
}

void CTicketSearch::GetUnReadMessageCountAndGetUnReadNoteCount(int nTicketID, int& nMC, int& nNC)
{
	TIMESTAMP_STRUCT ts;
	long tsLen = sizeof(TIMESTAMP_STRUCT);
	ZeroMemory(&ts, sizeof(TIMESTAMP_STRUCT));

	int nCountMsg = 0;
	int nTotalCountMsg = 0;
	int nCountNote = 0;
	int nTotalCountNote = 0;
	long lAgentID = GetISAPIData().m_pSession->m_AgentID;

	if(UseCutoffDate(ts))
	{
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), lAgentID);
		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);

		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);

		BINDPARAM_LONG(GetQuery(), lAgentID);
		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);

		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);

		BINDCOL_LONG_NOLEN(GetQuery(), nCountMsg);
		BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountMsg);
		BINDCOL_LONG_NOLEN(GetQuery(), nCountNote);
		BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountNote);

		CEMSString sQuery;
		sQuery.Format(_T("SELECT (SELECT COUNT(*) FROM InboundMessageRead INNER JOIN InboundMessages ON InboundMessageRead.InboundMessageID = InboundMessages.InboundMessageID INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID WHERE (InboundMessageRead.AgentID = ?) AND (Tickets.TicketID = ?) AND (InboundMessages.DateReceived > ?)), ")
			          _T("(SELECT COUNT(*) FROM InboundMessages WHERE (TicketID = ?) AND (InboundMessages.DateReceived > ?) AND (InboundMessages.IsDeleted=0)), ")
					  _T("(SELECT COUNT(*) FROM TicketNotesRead INNER JOIN TicketNotes ON TicketNotesRead.TicketNoteID = TicketNotes.TicketNoteID INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (TicketNotesRead.AgentID = ?) AND (Tickets.TicketID = ?) AND (TicketNotes.DateCreated > ?)), ")
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

		BINDPARAM_LONG(GetQuery(), lAgentID);
		BINDPARAM_LONG(GetQuery(), nTicketID);

		BINDPARAM_LONG(GetQuery(), nTicketID);

		BINDCOL_LONG_NOLEN(GetQuery(), nCountMsg);
		BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountMsg);
		BINDCOL_LONG_NOLEN(GetQuery(), nCountNote);
		BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountNote);

		CEMSString sQuery;
		sQuery.Format(_T("SELECT (SELECT COUNT(*) FROM InboundMessageRead INNER JOIN InboundMessages ON InboundMessageRead.InboundMessageID = InboundMessages.InboundMessageID INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID WHERE (InboundMessageRead.AgentID = ?) AND (Tickets.TicketID = ?)), ")
			          _T("(SELECT COUNT(*) FROM InboundMessages WHERE (TicketID = ?) AND (IsDeleted=0)), ")
					  _T("(SELECT COUNT(*) FROM TicketNotesRead INNER JOIN TicketNotes ON TicketNotesRead.TicketNoteID = TicketNotes.TicketNoteID INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (TicketNotesRead.AgentID = ?) AND (Tickets.TicketID = ?)), ")
					  _T("(SELECT COUNT(*) FROM TicketNotes WHERE (TicketID = ?))"));

		GetQuery().Execute(sQuery.c_str());
		GetQuery().Fetch();
	}

	nMC = nTotalCountMsg - nCountMsg;
	nNC = nTotalCountNote - nCountNote;
}
