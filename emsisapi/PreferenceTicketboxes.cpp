/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/PreferenceTicketboxes.cpp,v 1.2 2005/11/29 21:16:26 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/
#include "stdafx.h"
#include "PreferenceTicketboxes.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CPreferenceTicketboxes::CPreferenceTicketboxes(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nRefreshLeftPane = 0;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Main entry point	              
\*--------------------------------------------------------------------------*/
int CPreferenceTicketboxes::Run( CURLAction& action )
{

	GetISAPIData().GetURLLong( _T("selectID"), m_AgentID );

	if( GetISAPIData().GetXMLPost() )
	{
		Update();
	}
	else
	{
		List();
	}

	return 0;	
}

/*---------------------------------------------------------------------------\            
||  Comments:	Generate XML that contains ticketbox subscription details
\*--------------------------------------------------------------------------*/
void CPreferenceTicketboxes::List(void)
{
	CPreferenceTicketboxItem Item;
	list<CPreferenceTicketboxItem> ItemList;

	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), Item.m_nTicketBoxID );
	BINDCOL_TCHAR_NOLEN( GetQuery(), Item.m_szTicketBoxName );
	BINDCOL_TCHAR_NOLEN( GetQuery(), Item.m_szTicketBoxDescripton );
	BINDCOL_BIT_NOLEN( GetQuery(), Item.m_IsSubscribed );
	BINDCOL_LONG_NOLEN( GetQuery(), Item.m_nOpenItems );

	BINDPARAM_LONG( GetQuery(), m_AgentID );
	
	GetQuery().Execute(_T("SELECT TicketBoxID, Name, Description,(SELECT COUNT(*) FROM Ticketboxviews WHERE TicketBoxID = TicketBoxes.TicketBoxID AND AgentID=?),(SELECT COUNT(*) FROM Tickets WHERE IsDeleted=0 AND TicketStateID > 1 AND OwnerID=0 AND TicketBoxID = TicketBoxes.TicketBoxID) FROM TicketBoxes ORDER BY Name"));

	while ( GetQuery().Fetch() == S_OK )
	{
		ItemList.push_back(Item);
	}

	list<CPreferenceTicketboxItem>::iterator iter;

	for ( iter = ItemList.begin(); iter != ItemList.end(); iter++ )
	{
		CPreferenceTicketboxItem prefTBItem = *iter;

		if ( GetIsAdmin() || iter->m_IsSubscribed || 
			CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID, EMS_OBJECT_TYPE_TICKET_BOX, 
			                                                  iter->m_nTicketBoxID ) >= EMS_READ_ACCESS )
		{
			GetXMLGen().AddChildElem( _T("PublicTicketBox") );
			GetXMLGen().AddChildAttrib( _T("TicketBoxID"), iter->m_nTicketBoxID );
			GetXMLGen().AddChildAttrib( _T("OpenItems"), iter->m_nOpenItems );
			GetXMLGen().AddChildAttrib( _T("IsSubscribed"), iter->m_IsSubscribed );
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("Name"), iter->m_szTicketBoxName );
			GetXMLGen().AddChildElem( _T("Description"), iter->m_szTicketBoxDescripton );
			GetXMLGen().OutOfElem();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update
// 
////////////////////////////////////////////////////////////////////////////////
void CPreferenceTicketboxes::Update(void)
{
	CEMSString sIDCollection;
	int nTicketBoxID;
	list<int> DBTicketBoxes;
	list<int> TicketBoxes;
	list<int>::iterator iter;
	list<int>::iterator iter2;
	int nViewType = EMS_PUBLIC;
	int nSortField = EMS_COLUMN_DATE;

	GetTicketBoxes( DBTicketBoxes );

	GetISAPIData().GetXMLString( _T("IDCollection"), sIDCollection );

	sIDCollection.CDLInit();
	while( sIDCollection.CDLGetNextInt( nTicketBoxID ) )
	{
		if( nTicketBoxID != 0 )
		{
			TicketBoxes.push_back( nTicketBoxID );
		}
	}

	// Add the ones in the form but not in the database
	for( iter = TicketBoxes.begin(); iter != TicketBoxes.end(); iter++ )
	{
		bool bFound = false;

		for( iter2 = DBTicketBoxes.begin(); iter2 != DBTicketBoxes.end(); iter2++ )
		{
			if( *iter2 == *iter )
			{
				bFound = true;
				break;
			}
		}
		if( bFound == false )
		{
			nTicketBoxID = *iter;
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), m_AgentID );
			BINDPARAM_LONG( GetQuery(), nTicketBoxID );
			BINDPARAM_LONG( GetQuery(), nViewType );
			BINDPARAM_LONG( GetQuery(), nSortField );
			GetQuery().Execute( _T("INSERT INTO TicketBoxViews ")
								_T("(AgentID,TicketBoxID,TicketBoxViewTypeID,SortField,SortAscending) ")
								_T("VALUES (?,?,?,?,1)") );	
			m_nRefreshLeftPane = 1;
			DBTicketBoxes.push_back( *iter );
		}
	}
		
	// delete the ones in the database but not in the form
	for( iter = DBTicketBoxes.begin(); iter != DBTicketBoxes.end(); iter++ )
	{
		bool bFound = false;

		for( iter2 = TicketBoxes.begin(); iter2 != TicketBoxes.end(); iter2++ )
		{
			if( *iter2 == *iter )
			{
				bFound = true;
				break;
			}
		}
		if( bFound == false )
		{
			nTicketBoxID = *iter;
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), m_AgentID );
			BINDPARAM_LONG( GetQuery(), nTicketBoxID );
			GetQuery().Execute( _T("DELETE FROM TicketBoxViews ")
								_T("WHERE AgentID=? AND TicketBoxID=?") );
			m_nRefreshLeftPane = 1;
		}
	}

	GetXMLGen().AddChildElem( _T("RefreshLeftPane") );
	GetXMLGen().AddChildAttrib( _T("Value"), m_nRefreshLeftPane );
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetTicketBoxes
// 
////////////////////////////////////////////////////////////////////////////////
void CPreferenceTicketboxes::GetTicketBoxes( list<int>& IDList )
{
	int TicketBoxID;

	// List Public ticketboxviews
	GetQuery().Initialize();

	BINDPARAM_LONG(GetQuery(), m_AgentID );
	BINDCOL_LONG_NOLEN(GetQuery(), TicketBoxID );

	GetQuery().Execute(	_T("SELECT DISTINCT TicketBoxViews.TicketBoxID, TicketBoxes.Name ")
						_T("FROM TicketBoxViews INNER JOIN TicketBoxes ON TicketBoxViews.TicketBoxID = TicketBoxes.TicketBoxID ")
						_T("WHERE TicketBoxViews.AgentID = ? AND TicketBoxViews.TicketBoxID <> 0 ") 
						_T("ORDER BY TicketBoxes.Name ") );
	
	while( GetQuery().Fetch() == S_OK )
	{
		IDList.push_back( TicketBoxID );
	}

}
