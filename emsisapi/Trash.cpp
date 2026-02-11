/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Trash.cpp,v 1.2 2005/11/29 21:16:28 markm Exp $
||
||
||                                         
||  COMMENTS:	Administrative Trash Can   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "Trash.h"
#include "ContactFns.h"
#include "PurgeFns.h"
#include "AttachFns.h"
#include "ArchiveFns.h"
#include "Attachment.h"	
#include "TicketHistoryFns.h"
#include "Ticket.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CTrash::CTrash(CISAPIData& ISAPIData) : CPagedList( ISAPIData ) 
{
	m_nAgentFilter		= -1;
	m_nItemType			= EMS_DELETED_TICKETS;
};

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CTrash::Run(CURLAction& action)
{
	// require admin access
	RequireAdmin();

	// decode the form
	DecodeForm();

	// preform the requested action
	tstring sAction;
	GetISAPIData().GetFormString( _T("ACTION"), sAction, true);
	
	if ( sAction.compare( _T("destroy_all") ) == 0 )
	{
		DISABLE_IN_DEMO();
		PurgeAll();
	}
	else if ( sAction.compare( _T("destroy_selected") ) == 0 )
	{
		DISABLE_IN_DEMO();
		Purge();
	}
	else if ( sAction.compare( _T("restore_selected") ) == 0 )
	{
		DISABLE_IN_DEMO();
		Restore();
	}

	// build the summary information
	BuildSummaryInfo();

	// list the items
	ListItems();
	
	// list agent names
	ListAgents();
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Reads form parameters	              
\*--------------------------------------------------------------------------*/
void CTrash::DecodeForm( void )
{
	// decode the form
	GetISAPIData().GetURLLong( _T("VIEW"), m_nItemType, true );
	GetISAPIData().GetFormLong( _T("VIEW"), m_nItemType, true );

	GetISAPIData().GetFormLong( _T("AGENTFILTER"), m_nAgentFilter, true );
	GetISAPIData().GetURLLong( _T("AGENTFILTER"), m_nAgentFilter, true );
		
	// sanity checks
	if (( m_nItemType < 1) || ( m_nItemType >= EMS_DELETED_END))
	{
		CEMSString sError;
		sError.Format( _T("The ItemType (%d) is invalid"), m_nItemType );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds summary XML information	              
\*--------------------------------------------------------------------------*/
void CTrash::BuildSummaryInfo( void )
{
	GetXMLGen().AddChildElem( _T("Summary") );
	GetXMLGen().AddChildAttrib( _T("View"), m_nItemType );
	GetXMLGen().AddChildAttrib( _T("ViewName"), GetItemTypeName(m_nItemType) );
	GetXMLGen().AddChildAttrib( _T("AgentFilter"), m_nAgentFilter );
	
	for ( int i = 1; i < EMS_DELETED_END; i++ )
	{
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("Item") );
		GetXMLGen().AddChildAttrib( _T("ID"), i );
		GetXMLGen().AddChildAttrib( _T("Name"), GetItemTypeName( i ) );
		GetXMLGen().AddChildAttrib( _T("Count"), CountAllDeletedItems( i ) );
		GetXMLGen().OutOfElem();
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the name for a type of deleted item	              
\*--------------------------------------------------------------------------*/
LPCTSTR CTrash::GetItemTypeName( int nViewID )
{
	// TODO - LOAD ALL STRING FROM THE STRING TABLE
	switch (nViewID)
	{
	case EMS_DELETED_CONTACTS:
		return _T("Contacts");
		
	case EMS_DELETED_STDRESPONSES:
		return _T("Standard Responses");
		
	case EMS_DELETED_INBOUND_MSG:
		return _T("Inbound Messages");
		
	case EMS_DELETED_OUTBOUND_MSG:
		return _T("Outbound Messages");
		
	default:
		return _T("Tickets");
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the name for a type of deleted item	              
\*--------------------------------------------------------------------------*/
LPCTSTR CTrash::GetItemTypeTableName( int nViewID )
{
	switch (nViewID)
	{
	case EMS_DELETED_CONTACTS:
		return _T("Contacts");
		
	case EMS_DELETED_STDRESPONSES:
		return _T("StandardResponses");
	
	case EMS_DELETED_INBOUND_MSG:
		return _T("InboundMessages");
		
	case EMS_DELETED_OUTBOUND_MSG:
		return _T("OutboundMessages");
		
	default:
		return _T("Tickets");
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Count Deleted Items	              
\*--------------------------------------------------------------------------*/
int CTrash::CountAllDeletedItems( int nItemType )
{	
	CEMSString sQuery;
	int nCount;

	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), nCount );
	
	sQuery.Format( _T("SELECT COUNT(*) FROM %s WHERE IsDeleted = 1"), GetItemTypeTableName(nItemType) );
	
	GetQuery().Execute( sQuery.c_str() );

	GetQuery().Fetch();

	return nCount;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Count Deleted Tickets Per Agent	              
\*--------------------------------------------------------------------------*/
void CTrash::CountDeletedItemsByAgent( )
{
	CEMSString sQuery;
	int nCount;
	int nDeletedBy;
	
	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), nCount );
	BINDCOL_LONG_NOLEN( GetQuery(), nDeletedBy );
	
	sQuery.Format( _T("SELECT COUNT(*), DeletedBy FROM %s WHERE IsDeleted = 1 GROUP BY DeletedBy"), 
		GetItemTypeTableName(m_nItemType) );
	
	GetQuery().Execute( sQuery.c_str() );
	
	while ( GetQuery().Fetch() == S_OK )
	{
		m_DeletedByCounts[nDeletedBy] = nCount;
	}
	
	// add "all" to the map
	m_DeletedByCounts[-1] = CountAllDeletedItems( m_nItemType );
	
	// calculate item count
	map<int, int>::iterator iter;
	iter = m_DeletedByCounts.find(m_nAgentFilter);
	
	SetItemCount( iter != m_DeletedByCounts.end() ? iter->second : 0 );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates an XML list of agents with an item count 
||              for the current view type.	              
\*--------------------------------------------------------------------------*/
void CTrash::ListAgents( void )
{
	// TODO - pull the string for SYSTEM and ALL from the string
	// table....
	
	map<int, int>::iterator map_iter;
	int nCount;
	
	GetXMLGen().AddChildElem( _T("AgentNames") );
	GetXMLGen().IntoElem();
	
	// add all...	
	map_iter = m_DeletedByCounts.find(-1);
	nCount = map_iter != m_DeletedByCounts.end() ? map_iter->second : 0;
	GetXMLGen().AddChildElem( _T("Agent") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("-1") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("-- All --") );
	GetXMLGen().AddChildAttrib( _T("Count"), nCount );	
	
	// add system...
	map_iter = m_DeletedByCounts.find(0);
	nCount = map_iter != m_DeletedByCounts.end() ? map_iter->second : 0;
	
	GetXMLGen().AddChildElem( _T("Agent") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("0") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("-- System --") );
	GetXMLGen().AddChildAttrib( _T("Count"), nCount );	
	
	// add agents...
	XAgentNames AgentName;
	list<unsigned int>::iterator iter;

	for( iter = GetXMLCache().m_AgentIDs.GetList().begin(); iter != GetXMLCache().m_AgentIDs.GetList().end(); iter++ )
	{
		if ( GetXMLCache().m_AgentNames.Query( *iter, AgentName ) )
		{
			map_iter = m_DeletedByCounts.find( *iter );
			nCount = map_iter != m_DeletedByCounts.end() ? map_iter->second : 0;
			
			GetXMLGen().AddChildElem( _T("Agent") );
			GetXMLGen().AddChildAttrib( _T("ID"), *iter );
			GetXMLGen().AddChildAttrib( _T("Name"), AgentName.m_Name );
			GetXMLGen().AddChildAttrib( _T("Count"), nCount );
		}
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists deleted items
\*--------------------------------------------------------------------------*/
void CTrash::ListItems()
{
	CEMSString sAgentFilter;
	CEMSString sQuery;
	
	// get the item counts for each agent
	// and calculate the page count
	CountDeletedItemsByAgent();
	
	// init the query
	GetQuery().Initialize( m_ISAPIData.m_pSession->m_nMaxRowsPerPage, sizeof(DeletedItem) );
	GetQuery().EnableScrollCursor();
	
	// allocate storage
	DeletedItem* pItemList = NULL;

	if ((pItemList = new DeletedItem[GetMaxRowsPerPage()]) == NULL)
		THROW_EMS_EXCEPTION( E_MemoryError, CEMSString(EMS_STRING_ERROR_MEMORY) );
	
	try
	{

		BINDCOL_LONG_NOLEN( GetQuery(), pItemList[0].ID );
		BINDCOL_TCHAR( GetQuery(), pItemList[0].Name );
		BINDCOL_LONG_NOLEN( GetQuery(), pItemList[0].DeletedBy );
		BINDCOL_TIME( GetQuery(), pItemList[0].DeletedTime );
		BINDCOL_TIME(GetQuery(), pItemList[0].DateCreated);
		
		// agent filter...
		if (m_nAgentFilter != -1)
			sAgentFilter.Format( _T("AND DeletedBy = %d"), m_nAgentFilter);
		
		// format the query
		switch ( m_nItemType )
		{

		case EMS_DELETED_TICKETS:

			sQuery.Format( _T("SELECT DISTINCT TOP %d TicketID, Subject, DeletedBy, DeletedTime, DateCreated ")
						   _T("FROM Tickets ")
						   _T("WHERE Tickets.IsDeleted = 1 %s ")
						   _T("ORDER BY DeletedTime DESC "), GetEndRow(), sAgentFilter.c_str() );
			break;

		case EMS_DELETED_INBOUND_MSG:

			sQuery.Format( _T("SELECT TOP %d InboundMessageID, Subject, DeletedBy, DeletedTime, DateCreated ")
						   _T("FROM InboundMessages ")
						   _T("WHERE IsDeleted = 1 %s ")
						   _T("ORDER BY DeletedTime DESC "), GetEndRow(), sAgentFilter.c_str() );
			break;

		case EMS_DELETED_OUTBOUND_MSG:
			
			sQuery.Format( _T("SELECT TOP %d OutboundMessageID, Subject, DeletedBy, DeletedTime, DateCreated ")
						   _T("FROM OutboundMessages ")
						   _T("WHERE IsDeleted = 1 %s ")
						   _T("ORDER BY DeletedTime DESC "), GetEndRow(), sAgentFilter.c_str() );
			break;

		case EMS_DELETED_STDRESPONSES:

			sQuery.Format( _T( "SELECT TOP %d StandardResponseID, Subject, DeletedBy, DeletedTime, DateCreated " )
						   _T( "FROM StandardResponses ")
						   _T( "WHERE IsDeleted = 1 %s")
						   _T( "ORDER BY DeletedTime DESC "), GetEndRow(), sAgentFilter.c_str() );
			break;

		case EMS_DELETED_CONTACTS:

			sQuery.Format( _T( "SELECT TOP %d ContactID, Name, DeletedBy, DeletedTime, DateCreated " )
						   _T( "FROM Contacts ")
						   _T( "WHERE IsDeleted = 1 %s") 
						   _T( "ORDER BY DeletedTime DESC "), GetEndRow(), sAgentFilter.c_str()  );
			break;
		}
		
		// execute the query
		GetQuery().Execute( sQuery.c_str() );
		
		// fetch the rows
		GetQuery().FetchScroll( SQL_FETCH_ABSOLUTE, GetStartRow() );
		
		// generate the XML
		GenerateXML( pItemList, GetQuery().GetRowsFetched() );
		
		// close the cursor
		GetQuery().Reset();
	}
	catch(...)
	{
		// free memory
		if (pItemList)
		{
			delete[] pItemList;
			pItemList = NULL;
		}
		
		throw;
	}

	// free memory
	if (pItemList)
	{
		delete[] pItemList;
		pItemList = NULL;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML for the list of deleted items	              
\*--------------------------------------------------------------------------*/
void CTrash::GenerateXML( DeletedItem* pItemList, int nItems )
{
	CEMSString sDateTime;
	CEMSString sName;
	
	// add page information
	AddPageXML();
	
	for(int i = 0; i < nItems; i++)
	{
		GetXMLGen().AddChildElem(_T("Item"));
		GetXMLGen().AddChildAttrib( _T("ID"), pItemList[i].ID );
		
		sName.assign( pItemList[i].NameLen ? pItemList[i].Name : _T("[No Description]") );
		sName.EscapeHTML();
		GetXMLGen().AddChildAttrib( _T("Name"), sName.c_str() );
	
		if ( pItemList[i].DeletedBy > 0)
		{
			AddAgentName( _T("DeletedBy"), pItemList[i].DeletedBy, _T("Not Avaliable"));
		}
		else
		{
			// TO DO - need to load the string from the string table...
			GetXMLGen().AddChildAttrib( _T("DeletedBy"), _T("SYSTEM") );
		}
		
		GetDateTimeString( pItemList[i].DeletedTime, pItemList[i].DeletedTimeLen, sDateTime );
		GetXMLGen().AddChildAttrib( _T("DeletedTime"), sDateTime.c_str() );

		// NEW: Add the DateCreated field
		GetDateTimeString(pItemList[i].DateCreated, pItemList[i].DateCreatedLen, sDateTime);
		GetXMLGen().AddChildAttrib(_T("DateCreated"), sDateTime.c_str());
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Restore Selected Items	              
\*--------------------------------------------------------------------------*/
void CTrash::Restore( void )
{
	switch(m_nItemType)
	{
	case EMS_DELETED_TICKETS:
		UndeleteTickets();
		break;
	case EMS_DELETED_INBOUND_MSG:
		UndeleteInboundMsgs();
		break;
	case EMS_DELETED_OUTBOUND_MSG:
		UndeleteOutboundMsgs();
		break;
	case EMS_DELETED_CONTACTS:
		UndeleteContacts();
		break;
	case EMS_DELETED_STDRESPONSES:
		UndeleteStandardResponses();
		break;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purge Selected Items	              
\*--------------------------------------------------------------------------*/
void CTrash::Purge( void )
{
	switch(m_nItemType)
	{
	case EMS_DELETED_TICKETS:
		PurgeTickets();
		break;
	case EMS_DELETED_INBOUND_MSG:
		PurgeInboundMsgs();
		break;
	case EMS_DELETED_OUTBOUND_MSG:
		PurgeOutboundMsgs();
		break;
	case EMS_DELETED_CONTACTS:
		PurgeContacts();
		break;
	case EMS_DELETED_STDRESPONSES:
		PurgeStdResponses();
		break;
	}

	/*
	// shrink the database
	CArchiveFile archive( GetQuery() );
	archive.ShrinkDatabase();
	*/
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purge All Items	              
\*--------------------------------------------------------------------------*/
void CTrash::PurgeAll( void )
{
	switch(m_nItemType)
	{
	case EMS_DELETED_TICKETS:
		PurgeAllTickets();
		break;
	case EMS_DELETED_INBOUND_MSG:
		PurgeAllInboundMsgs();
		break;
	case EMS_DELETED_OUTBOUND_MSG:
		PurgeAllOutboundMsgs();
		break;
	case EMS_DELETED_CONTACTS:
		PurgeAllContacts();
		break;
	case EMS_DELETED_STDRESPONSES:
		PurgeAllStdResponses();
		break;
	}

	/*
	// shrink the database
	CArchiveFile archive( GetQuery() );
	archive.ShrinkDatabase();
	*/
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Restores a comma delimited list of tickets	              
\*--------------------------------------------------------------------------*/
void CTrash::UndeleteTickets( void )
{
	CEMSString sQuery;
	CEMSString sIDs;
	CEMSString sChunk;
	
	CTicket Ticket(m_ISAPIData);

	tstring sSingleID;

	GetQuery().Initialize();
	
	GetISAPIData().GetFormString( _T("IDCOLLECTION"), sIDs );
	
	CEMSString sTicketIDs = sIDs;
	
	while ( sIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		sQuery.Format( _T("UPDATE Tickets SET IsDeleted=0,TicketStateID=2,LockedBy=0 WHERE TicketID IN (%s)"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );

		// Remove any delete outbound message ticket history entries
		/*GetQuery().Reset();
		sQuery.Format( _T("DELETE from TicketHistory ")
			           _T("WHERE TicketActionID = 6 ")
					   _T("AND TicketID IN (%s) ")
					   _T("AND ID1=0 ")
					   _T("AND ID2 IN (SELECT OutboundMessageID FROM OutboundMessages WHERE TicketID IN (%s)) "), sChunk.c_str(), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );*/

		// Remove any delete inbound message ticket history entries
		/*GetQuery().Reset();
		sQuery.Format( _T("DELETE from TicketHistory ")
			           _T("WHERE TicketActionID = 6 ")
					   _T("AND TicketID IN (%s) ")
					   _T("AND ID1=1 ")
					   _T("AND ID2 IN (SELECT InboundMessageID FROM InboundMessages WHERE TicketID IN (%s)) "), sChunk.c_str(), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );*/


		sQuery.Format( _T("UPDATE InboundMessages SET IsDeleted=0 WHERE TicketID IN (%s)"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );

		sQuery.Format( _T("UPDATE OutboundMessages SET IsDeleted=0 WHERE TicketID IN (%s)"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );
		
	}

	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
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
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Restores a comma delimited list of InboundMessages	              
\*--------------------------------------------------------------------------*/
void CTrash::UndeleteInboundMsgs( void )
{
	CEMSString sQuery;
	CEMSString sIDs;
	CEMSString sChunk;

	int nMsgID;
	int nTicketID;
	
	GetISAPIData().GetFormString( _T("IDCOLLECTION"), sIDs );
	
	while ( sIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{	
		// undelete the messages
		GetQuery().Initialize();
		sQuery.Format( _T("UPDATE InboundMessages SET IsDeleted = 0 WHERE InboundMessageID IN (%s)"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );

		// undelete the tickets
		GetQuery().Reset();
		sQuery.Format( _T("UPDATE Tickets SET IsDeleted = 0 WHERE TicketID IN ")
			           _T("(SELECT TicketID From InboundMessages WHERE InboundMessageID IN (%s) )"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );

		// Remove any delete inbound message ticket history entries
		GetQuery().Reset();
		sQuery.Format( _T("DELETE from TicketHistory ")
			           _T("WHERE TicketActionID = 6 ")
					   _T("AND TicketID IN (SELECT TicketID From InboundMessages WHERE InboundMessageID IN (%s)) ")
					   _T("AND ID1=1 ")
					   _T("AND ID2 IN (%s)"), sChunk.c_str(), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );

		// update the ticket contacts
		sChunk.CDLInit();

		while ( sChunk.CDLGetNextInt( nMsgID ) )
		{
			GetQuery().Reset();
			BINDCOL_LONG_NOLEN( GetQuery(), nTicketID );
			BINDPARAM_LONG( GetQuery(), nMsgID );
			GetQuery().Execute( _T("SELECT TicketID From InboundMessages WHERE InboundMessageID = ?") );
			
			if ( GetQuery().Fetch() == S_OK )
				RebuildTicketContacts( GetQuery(), nTicketID );
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Restores a comma delimited list of OutboundMessages	              
\*--------------------------------------------------------------------------*/
void CTrash::UndeleteOutboundMsgs( void )
{
	CEMSString sQuery;
	CEMSString sIDs;
	CEMSString sChunk;

	int nMsgID;
	int nTicketID;
	
	GetISAPIData().GetFormString( _T("IDCOLLECTION"), sIDs );
	
	while ( sIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		GetQuery().Initialize();
		sQuery.Format( _T("UPDATE OutboundMessages SET IsDeleted = 0 ")
			           _T("WHERE OutboundMessageID IN (%s)"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );

		GetQuery().Reset();
		sQuery.Format( _T("UPDATE Tickets SET IsDeleted = 0 WHERE TicketID IN ")
					   _T("(SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID IN (%s) )"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );

		// Remove any delete outbound message ticket history entries
		GetQuery().Reset();
		sQuery.Format( _T("DELETE from TicketHistory ")
			           _T("WHERE TicketActionID = 6 ")
					   _T("AND TicketID IN (SELECT TicketID From OutboundMessages WHERE OutboundMessageID IN (%s)) ")
					   _T("AND ID1=0 ")
					   _T("AND ID2 IN (%s)"), sChunk.c_str(), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );

		// update the ticket contacts
		sChunk.CDLInit();

		while ( sChunk.CDLGetNextInt( nMsgID ) )
		{
			GetQuery().Reset();
			BINDCOL_LONG_NOLEN( GetQuery(), nTicketID );
			BINDPARAM_LONG( GetQuery(), nMsgID );
			GetQuery().Execute( _T("SELECT TicketID From OutboundMessages WHERE OutboundMessageID=?") );

			if ( GetQuery().Fetch() == S_OK )
				RebuildTicketContacts( GetQuery(), nTicketID );
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Restores a comma delimited list of standard responses	              
\*--------------------------------------------------------------------------*/
void CTrash::UndeleteStandardResponses ( void )
{
	CEMSString sQuery;
	CEMSString sIDs;
	CEMSString sChunk;
	
	GetISAPIData().GetFormString( _T("IDCOLLECTION"), sIDs );
	
	while ( sIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		GetQuery().Initialize();

		sQuery.Format( _T("UPDATE StandardResponses SET IsDeleted = 0")
			           _T("WHERE StandardResponseID IN (%s)"), sChunk.c_str() );		
		
		GetQuery().Execute( sQuery.c_str() );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Restores a comma delimited list of contacts           
\*--------------------------------------------------------------------------*/
void CTrash::UndeleteContacts( void )
{
	CEMSString sQuery;
	CEMSString sIDs;
	CEMSString sChunk;
	
	GetISAPIData().GetFormString( _T("IDCOLLECTION"), sIDs );
	
	while ( sIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		GetQuery().Initialize();
		sQuery.Format( _T("UPDATE Contacts SET IsDeleted = 0 WHERE ContactID IN (%s)"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );
		
		GetQuery().Reset();		
		sQuery.Format( _T("DELETE FROM AgentContacts WHERE ContactID IN (%s)"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );		

		// now update the ticket contacts
		int  nTicketID;
		list<int> TicketIDList;
		list<int>::iterator iter;
		
		GetQuery().Reset();
		BINDCOL_LONG_NOLEN( GetQuery(), nTicketID );
		sQuery.Format( _T("SELECT DISTINCT TicketID FROM TicketContacts WHERE ContactID IN (%s)"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );
		
		while ( GetQuery().Fetch() == S_OK )
		{
			TicketIDList.push_back(nTicketID);
		}
		
		for ( iter = TicketIDList.begin(); iter != TicketIDList.end(); iter++ )
		{
			BuildTicketContacts( GetQuery(), *iter );
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges a comma delimited list of tickets            
\*--------------------------------------------------------------------------*/
void CTrash::PurgeTickets( void )
{
	int nTicketID;
	int nTicketCount = 0;
	CEMSString sIDs;
	GetISAPIData().GetFormString( _T("IDCOLLECTION"), sIDs );

	sIDs.CDLInit();
	while ( sIDs.CDLGetNextInt( nTicketID ) )
	{
		nTicketCount++;								
	}
	
	// call the library function
	::PurgeTickets( sIDs, GetQuery(), nTicketCount ); 
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges a comma delimited list of InboundMessages            
\*--------------------------------------------------------------------------*/
void CTrash::PurgeInboundMsgs( void )
{
	CEMSString sIDs;
	GetISAPIData().GetFormString( _T("IDCOLLECTION"), sIDs );
	
	// call the library function
	::PurgeInboundMessages( sIDs, GetQuery() ); 
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges a comma delimited list of OutboundMessages            
\*--------------------------------------------------------------------------*/
void CTrash::PurgeOutboundMsgs( void )
{
	CEMSString sIDs;
	GetISAPIData().GetFormString( _T("IDCOLLECTION"), sIDs );
	
	// call the library function
	::PurgeOutboundMessages( sIDs, GetQuery() ); 
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges a comma delimited list of contacts     
\*--------------------------------------------------------------------------*/
void CTrash::PurgeContacts( void )
{
	CEMSString sIDs;
		
	GetISAPIData().GetFormString( _T("IDCOLLECTION"), sIDs );
	
	::PurgeContacts( sIDs, GetQuery() );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges a comma delimited list of standard responses             
\*--------------------------------------------------------------------------*/
void CTrash::PurgeStdResponses( void )
{
	CEMSString sIDs;
	
	GetISAPIData().GetFormString( _T("IDCOLLECTION"), sIDs );
	
	PurgeStandardResponses( sIDs, GetQuery() );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges all tickets marked as deleted            
\*--------------------------------------------------------------------------*/
void CTrash::PurgeAllTickets( void )
{
	CEMSString sQuery;
	CEMSString sAgentFilter;

	if (m_nAgentFilter != -1)
		sAgentFilter.Format( _T("AND DeletedBy = %d"), m_nAgentFilter);
	
	PurgeAllInboundMsgs();	
	PurgeAllOutboundMsgs();

	// delete ticketnotesread
	GetQuery().Initialize();
	sQuery.Format( _T("DELETE FROM TicketNotesRead ")
		           _T("WHERE TicketNoteID IN ")
				   _T("(SELECT TicketNoteID FROM TicketNotes WHERE TicketID IN ")
				   _T("(SELECT TicketID FROM Tickets WHERE IsDeleted = 1 %s))"), sAgentFilter.c_str());
	GetQuery().Execute( sQuery.c_str() );

	GetQuery().Initialize();
	sQuery.Format( _T("DELETE FROM TicketNotes ")
				   _T("WHERE TicketID IN (SELECT TicketID FROM Tickets WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
	
	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM TicketContacts ")
		           _T("WHERE TicketID IN (SELECT TicketID FROM Tickets WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
	
	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM TicketHistory ")
		           _T("WHERE TicketID IN (SELECT TicketID FROM Tickets WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
	
	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM TicketFieldsTicket ")
				   _T("WHERE TicketID IN (SELECT TicketID FROM Tickets WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );

	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM TicketLinksTicket ")
				   _T("WHERE TicketID IN (SELECT TicketID FROM Tickets WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );

	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM Tickets WHERE Tickets.IsDeleted = 1 %s"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges all contacts marked as deleted	  	              
\*--------------------------------------------------------------------------*/
void CTrash::PurgeAllInboundMsgs( void )
{
	CEMSString sQuery;
	CEMSString sAgentFilter;
	CAttachment attach(m_ISAPIData);
	tstring sFullAttachPath;
	list<unsigned int> AttachmentIDList;
	list<unsigned int>::iterator iter;
	
	if (m_nAgentFilter != -1)
		sAgentFilter.Format( _T("AND DeletedBy = %d"), m_nAgentFilter);

	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), attach.m_AttachmentID );
	sQuery.Format( _T("SELECT A.AttachmentID FROM Attachments as A ")
				   _T("INNER JOIN InboundMessageAttachments as I on A.AttachmentID = I.AttachmentID ")
				   _T("WHERE InboundMessageID IN (SELECT InboundMessageID ")
				   _T("FROM InboundMessages WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
	
	while( GetQuery().Fetch() == S_OK )
	{
		AttachmentIDList.push_back( attach.m_AttachmentID );
	}
	
	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM InboundMessageAttachments ")
		           _T("WHERE InboundMessageID IN (SELECT InboundMessageID ")
		           _T("FROM InboundMessages WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );

	for ( iter = AttachmentIDList.begin(); iter != AttachmentIDList.end(); iter++ )
	{
		// if the attachment is no longer referenced, delete it 
		if ( GetAttachmentReferenceCount( GetQuery(), *iter ) == 0 )
		{
			attach.m_AttachmentID = *iter;
			attach.Delete();
		}
	}

	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM InboundMessageRead ")
				   _T("WHERE InboundMessageID IN (SELECT InboundMessageID ")
				   _T("FROM InboundMessages WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );

	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM SRKeywordResults ")
				   _T("WHERE InboundMessageID IN (SELECT InboundMessageID ")
				   _T("FROM InboundMessages WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
	
	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM InboundMessages WHERE InboundMessages.IsDeleted = 1 %s"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges all contacts marked as deleted	  	              
\*--------------------------------------------------------------------------*/
void CTrash::PurgeAllOutboundMsgs( void )
{
	CEMSString sQuery;
	CEMSString sAgentFilter;
	CAttachment attach(m_ISAPIData);
	tstring sFullAttachPath;
	list<unsigned int> AttachmentIDList;
	list<unsigned int>::iterator iter;
	
	if (m_nAgentFilter != -1)
		sAgentFilter.Format( _T("AND DeletedBy = %d"), m_nAgentFilter);

	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), attach.m_AttachmentID );
	sQuery.Format( _T("SELECT A.AttachmentID FROM Attachments as A ")
		           _T("INNER JOIN OutboundMessageAttachments as O on A.AttachmentID = O.AttachmentID ")
		           _T("WHERE OutboundMessageID IN (SELECT OutboundMessageID ")
		           _T("FROM OutboundMessages WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
	
	while( GetQuery().Fetch() == S_OK )
	{
		AttachmentIDList.push_back( attach.m_AttachmentID );
	}
	
	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM OutboundMessageAttachments ")
		_T("WHERE OutboundMessageID IN (SELECT OutboundMessageID ")
		_T("FROM OutboundMessages WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );

	for ( iter = AttachmentIDList.begin(); iter != AttachmentIDList.end(); iter++ )
	{
		// if the attachment is no longer referenced, delete it
		if ( GetAttachmentReferenceCount( GetQuery(), *iter ) == 0 )
		{
			attach.m_AttachmentID = *iter;
			attach.Delete( );
		}
	}

	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM OutboundMessageContacts ")
		_T("WHERE OutboundMessageID IN (SELECT OutboundMessageID ")
		_T("FROM OutboundMessages WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
	
	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM OutboundMessageQueue ")
		_T("WHERE OutboundMessageID IN (SELECT OutboundMessageID ")
		_T("FROM OutboundMessages WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
	
	GetQuery().Reset();
	sQuery.Format( _T("DELETE FROM OutboundMessages WHERE IsDeleted = 1 %s"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges all contacts marked as deleted	  	              
\*--------------------------------------------------------------------------*/
void CTrash::PurgeAllContacts( void )
{
	CEMSString sQuery;
	CEMSString sAgentFilter;
	
	if (m_nAgentFilter != -1)
		sAgentFilter.Format( _T("AND DeletedBy = %d"), m_nAgentFilter);

	try
	{
		GetQuery().Initialize();
		GetQuery().Execute( _T("BEGIN TRANSACTION") );	

		// delete from OutboundMessageContacts
		GetQuery().Reset(true);
		sQuery.Format( _T("DELETE FROM OutboundMessageContacts WHERE ContactID IN ")
			_T("(SELECT ContactID FROM Contacts WHERE Contacts.IsDeleted = 1 %s)"), sAgentFilter.c_str() );
		GetQuery().Execute( sQuery.c_str() );
		
		// delete from ticket contacts
		GetQuery().Reset(true);
		sQuery.Format( _T("DELETE FROM TicketContacts WHERE ContactID IN ")
			_T("(SELECT ContactID FROM Contacts WHERE Contacts.IsDeleted = 1 %s)"), sAgentFilter.c_str() );
		GetQuery().Execute( sQuery.c_str() );

		// delete from contact notes
		GetQuery().Reset(true);
		sQuery.Format( _T("DELETE FROM ContactNotes WHERE ContactID IN ")
			_T("(SELECT ContactID FROM Contacts WHERE Contacts.IsDeleted = 1 %s)"), sAgentFilter.c_str() );
		GetQuery().Execute( sQuery.c_str() );
		
		// delete from contact groups
		GetQuery().Reset(true);
		sQuery.Format( _T("DELETE FROM ContactGrouping WHERE ContactID IN ")
			_T("(SELECT ContactID FROM Contacts WHERE Contacts.IsDeleted = 1 %s)"), sAgentFilter.c_str() );
		GetQuery().Execute( sQuery.c_str() );
		
		// delete from agent contacts
		GetQuery().Reset(true);
		sQuery.Format( _T("DELETE FROM AgentContacts WHERE ContactID IN ")
			_T("(SELECT ContactID FROM Contacts WHERE Contacts.IsDeleted = 1 %s)"), sAgentFilter.c_str() );
		GetQuery().Execute( sQuery.c_str() );
		
		// delete from personal data
		GetQuery().Reset(true);
		sQuery.Format( _T("DELETE FROM PersonalData WHERE ContactID IN ")
			_T("(SELECT ContactID FROM Contacts WHERE Contacts.IsDeleted = 1 %s)"), sAgentFilter.c_str() );
		GetQuery().Execute( sQuery.c_str() );
		
		// delete from contacts
		GetQuery().Reset(true);
		sQuery.Format( _T("DELETE FROM Contacts WHERE Contacts.IsDeleted = 1 %s"), sAgentFilter.c_str() );
		GetQuery().Execute( sQuery.c_str() );

		GetQuery().Reset(true);
		GetQuery().Execute( _T("COMMIT TRANSACTION") );
	}
	catch( ... )
	{
		GetQuery().Initialize();
		GetQuery().Execute( _T("ROLLBACK TRANSACTION") );
		throw;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges all standard responses marked as deleted	  	              
\*--------------------------------------------------------------------------*/
void CTrash::PurgeAllStdResponses( void )
{
	CEMSString sQuery;
	CEMSString sAgentFilter;
	CAttachment attach(m_ISAPIData);
	tstring sFullAttachPath;
	list<unsigned int> AttachmentIDList;
	list<unsigned int>::iterator iter;
	
	if (m_nAgentFilter != -1)
		sAgentFilter.Format( _T("AND DeletedBy = %d"), m_nAgentFilter);

	// delete from standard response usage
	GetQuery().Initialize();
	sQuery.Format( _T("DELETE FROM StandardResponseUsage ")
		_T("WHERE StandardResponseID IN (SELECT StandardResponseID FROM StandardResponses WHERE IsDeleted = 1 %s)"),
		sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );

	// delete from standard response favorites
	GetQuery().Reset(true);
	sQuery.Format( _T("DELETE FROM StdResponseFavorites ")
		_T("WHERE StandardResponseID IN (SELECT StandardResponseID FROM StandardResponses WHERE IsDeleted = 1 %s)"),
		sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );

	// delete attachments
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), attach.m_AttachmentID );
	sQuery.Format( _T("SELECT A.AttachmentID FROM Attachments as A ")
		           _T("INNER JOIN StdResponseAttachments as O on A.AttachmentID = O.AttachmentID ")
		           _T("WHERE StandardResponseID IN (SELECT StandardResponseID ")
		           _T("FROM StandardResponses WHERE IsDeleted = 1 %s)"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
	
	while( GetQuery().Fetch() == S_OK )
	{
		AttachmentIDList.push_back( attach.m_AttachmentID );
	}

	// delete from standard response attachments table
	GetQuery().Reset(true);
	sQuery.Format( _T("DELETE FROM StdResponseAttachments ")
		_T("WHERE StandardResponseID IN (SELECT StandardResponseID FROM StandardResponses WHERE IsDeleted = 1 %s)"),
		sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );

	for ( iter = AttachmentIDList.begin(); iter != AttachmentIDList.end(); iter++ )
	{
		// if the attachment is no longer referenced, delete it 
		if ( GetAttachmentReferenceCount( GetQuery(), *iter ) == 0 )
		{
			attach.m_AttachmentID = *iter;
			attach.Delete();
		}
	}

	// delete standard responses
	GetQuery().Reset(true);
	sQuery.Format( _T("DELETE FROM StandardResponses WHERE IsDeleted = 1 %s"), sAgentFilter.c_str() );
	GetQuery().Execute( sQuery.c_str() );
}