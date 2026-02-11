/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/PreferenceAgents.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/
#include "stdafx.h"
#include "TicketBoxViewFns.h"
#include "PreferenceAgents.h"
#include "SecurityFns.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CPreferenceAgents::CPreferenceAgents(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nRefreshLeftPane = 0;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Main entry point	              
\*--------------------------------------------------------------------------*/
int CPreferenceAgents::Run( CURLAction& action )
{

	GetISAPIData().GetURLLong( _T("selectID"), m_AgentID, true );

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
void CPreferenceAgents::List(void)
{
	CPreferenceAgentItem Item;
	list<CPreferenceAgentItem> ItemList;

	GetQuery().Initialize();

	BINDPARAM_LONG( GetQuery(), m_AgentID );
	BINDCOL_LONG_NOLEN( GetQuery(), Item.m_AgentBoxID );
	BINDCOL_TCHAR( GetQuery(), Item.m_LoginName );
	BINDCOL_TCHAR( GetQuery(), Item.m_Name );
	BINDCOL_LONG_NOLEN( GetQuery(), Item.m_IsSubscribed );
	BINDCOL_LONG_NOLEN( GetQuery(), Item.m_OpenTickets );

	GetQuery().Execute( 
		_T("SELECT AgentID, LoginName,Name,") 
		_T("(SELECT COUNT(*) FROM Ticketboxviews WHERE TicketBoxID = 0 AND AgentID=? AND AgentBoxID=Agents.AgentID),")
		_T("(SELECT COUNT(*) FROM Tickets WHERE IsDeleted=0 AND TicketStateID > 1 AND OwnerID=Agents.AgentID) ")
		_T("FROM Agents WHERE IsDeleted=0 ORDER BY Name") );


	while ( GetQuery().Fetch() == S_OK )
	{
		if( Item.m_AgentBoxID != m_AgentID && Item.m_AgentBoxID != 0)
			ItemList.push_back(Item);
	}

	list<CPreferenceAgentItem>::iterator iter;

	for ( iter = ItemList.begin(); iter != ItemList.end(); iter++ )
	{
		if ( GetIsAdmin() || iter->m_IsSubscribed || 
			CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID, EMS_OBJECT_TYPE_AGENT, 
                                                              iter->m_AgentBoxID ) >= EMS_READ_ACCESS )
		{
			GetXMLGen().AddChildElem( _T("AgentTicketBox") );
			GetXMLGen().AddChildAttrib( _T("AgentID"), iter->m_AgentBoxID );
			GetXMLGen().AddChildAttrib( _T("OpenItems"), iter->m_OpenTickets );
			GetXMLGen().AddChildAttrib( _T("IsSubscribed"), iter->m_IsSubscribed > 0 ? 1 : 0 );
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("Name"), iter->m_Name );
			GetXMLGen().AddChildElem( _T("Login"), iter->m_LoginName );
			GetXMLGen().OutOfElem();
		}
	}
}



////////////////////////////////////////////////////////////////////////////////
// 
// Update
// 
////////////////////////////////////////////////////////////////////////////////
void CPreferenceAgents::Update(void)
{
	CEMSString sIDCollection;
	int nAgentBoxID;
	list<int> DBAgentViews;
	list<int> AgentViews;
	list<int>::iterator iter;
	list<int>::iterator iter2;
//	int nViewType = EMS_PUBLIC;

	GetISAPIData().GetXMLString( _T("IDCollection"), sIDCollection );

	sIDCollection.CDLInit();
	while( sIDCollection.CDLGetNextInt( nAgentBoxID ) )
	{
		if( nAgentBoxID != 0)
		{
			AgentViews.push_back( nAgentBoxID );
		}
	}

	GetAgentViews( DBAgentViews );

	// Add the ones in the form but not in the database
	for( iter = AgentViews.begin(); iter != AgentViews.end(); iter++ )
	{
		if( *iter != m_AgentID )
		{
			bool bFound = false;

			for( iter2 = DBAgentViews.begin(); iter2 != DBAgentViews.end(); iter2++ )
			{
				if( *iter2 == *iter )
				{
					bFound = true;
					break;
				}
			}
			if( bFound == false )
			{
				TBVAddAgent( GetQuery(), m_AgentID, *iter );
				m_nRefreshLeftPane = 1;
				DBAgentViews.push_back( *iter );
			}
		}
	}
	
	// delete the ones in the database but not in the form
	for( iter = DBAgentViews.begin(); iter != DBAgentViews.end(); iter++ )
	{
		if( *iter != m_AgentID )
		{
			bool bFound = false;

			for( iter2 = AgentViews.begin(); iter2 != AgentViews.end(); iter2++ )
			{
				if( *iter2 == *iter )
				{
					bFound = true;
					break;
				}
			}
			if( bFound == false )
			{
				TBVRemoveAgent( GetQuery(), m_AgentID, *iter );
				m_nRefreshLeftPane = 1;
			}
		}
	}

	GetXMLGen().AddChildElem( _T("RefreshLeftPane") );
	GetXMLGen().AddChildAttrib( _T("Value"), m_nRefreshLeftPane );
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetAgentViews
// 
////////////////////////////////////////////////////////////////////////////////
void CPreferenceAgents::GetAgentViews( list<int>& IDList )
{
	int AgentBoxID;

	// List Agent ticketbox views
	GetQuery().Initialize();
	
	BINDPARAM_LONG(GetQuery(), m_AgentID );
	BINDCOL_LONG_NOLEN(GetQuery(), AgentBoxID );
		
	GetQuery().Execute( _T("SELECT DISTINCT AgentBoxID,Agents.Name ")
						_T("FROM TicketBoxViews INNER JOIN Agents ON TicketBoxViews.AgentBoxID = Agents.AgentID ")
						_T("WHERE TicketBoxViews.AgentID = ? AND AgentBoxID <> 0 ")
						_T("ORDER BY Agents.Name ") );

	while( GetQuery().Fetch() == S_OK )
	{
		// Don't add the agent's own view to the list
		if( AgentBoxID != m_AgentID )
			IDList.push_back( AgentBoxID );
	}
}
