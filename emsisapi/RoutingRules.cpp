// RoutingRules.cpp: implementation of the CRoutingRules class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RoutingRules.h"
#include "StringFns.h"
#include <.\boost\regex.hpp>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRoutingRules::CRoutingRules(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_StdRespName[0] = _T('\0');
	m_DBMatchTextMaxID = 0;
	m_MaxID = 0;
	m_RoutingRuleID = 0;
	m_ForwardInTicket = 1;
	m_AutoReplyInTicket = 1;
	m_AllowRemoteReply = 0;
	m_SortBy = 0;
	m_SortDir = 1;
	m_MultiMail = 0;
	m_SetOpenOwner = 0;
	m_OfficeHours = 0;
	m_IgnoreTracking = 0;
}

CRoutingRules::~CRoutingRules()
{
}

int CRoutingRules::Run( CURLAction& action )
{  
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_RoutingRuleID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_RoutingRuleID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_RoutingRuleID);
		}
	}

	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
	{
		if (!GetISAPIData().GetXMLString( _T("Action"), sAction, true ))
			GetISAPIData().GetURLString( _T("Action"), sAction, true );
	}
		

	if( GetISAPIData().m_sPage.compare( _T("rroptions") ) == 0 )
	{
		return Options( sAction );
	}
	else if( GetISAPIData().m_sPage.compare( _T("routingrule") ) == 0 )
	{
		if( sAction.compare( _T("insert") ) == 0 )
		{
			DISABLE_IN_DEMO();
			New();
			return 0;
		}
		else if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();

			if( m_RoutingRuleID == 0 )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			
			Update();
			return 0;
		}
		else if ( sAction.compare( _T("resetHitCount") ) == 0 )
		{
			DISABLE_IN_DEMO();
			ResetHitCount();
		}

		// change title if ID is zero
		if( m_RoutingRuleID == 0 )
			action.m_sPageTitle.assign( "New Routing Rule" );

		return Query();
	}
	
	if( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();

		if( m_RoutingRuleID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

		Delete();
		ResetOrder();
	}
	else if( sAction.compare( _T("reorder") ) == 0 )
	{
		DISABLE_IN_DEMO();
		ReOrder( action );
	}
	else if( sAction.compare( _T("sort") ) == 0 )
	{
		GetISAPIData().GetURLLong( _T("SortBy"), m_SortBy, true );
		GetISAPIData().GetURLLong( _T("SortAscending"), m_SortDir, true );
	}

	return ListAll();
}

void CRoutingRules::ResetHitCount(void)
{
	GetQuery().Initialize();	
	BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
	GetQuery().Execute( _T("UPDATE RoutingRules Set HitCount=0 WHERE RoutingRuleID=?") );
		
	if( GetQuery().GetRowCount() == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
}

int CRoutingRules::ListAll(void)
{
	CEMSString sQuery;
	CEMSString sSort;

	// Set the sort order
	switch (m_SortBy)
	{
	case 0:
		sSort.Format( _T("OrderIndex %s"), 
			SORT_ORDER( m_SortDir ) );
		break;
		
	case 1:
		sSort.Format( _T("RuleDescrip %s"), 
			SORT_ORDER( m_SortDir ) );
		break;

	case 2:
		sSort.Format( _T("IsEnabled %s"), 
			SORT_ORDER( m_SortDir ) );
		break;
	
	case 3:
		sSort.Format( _T("ms.Description %s"), 
			SORT_ORDER( m_SortDir ) );
		break;

	case 4:
		sSort.Format( _T("tb.Name %s"), 
			SORT_ORDER( m_SortDir ) );
		break;

	case 5:
		sSort.Format( _T("a.Name %s"), 
			SORT_ORDER( m_SortDir ) );
		break;

	case 6:
		sSort.Format( _T("tc.Description %s"), 
			SORT_ORDER( m_SortDir ) );
		break;

	case 7:
		sSort.Format( _T("HitCount %s"), 
			SORT_ORDER( m_SortDir ) );
		break;	

	case 8:
		sSort.Format( _T("OfficeHours %s"), 
			SORT_ORDER( m_SortDir ) );
		break;	

	default:
		sSort.Format( _T("OrderIndex ASC"));
	}

	// Format the Query
	sQuery.Format( _T("SELECT RoutingRuleID,HitCount,RuleDescrip, ")
						_T("rr.IsEnabled,rr.OrderIndex,rr.DeleteImmediately,rr.MessageSourceTypeID, ")
						_T("rr.AssignToTicketBox,rr.PriorityID,rr.AssignToTicketCategory,rr.AssignToAgent,rr.OfficeHours, ")
						_T("ms.Description,tb.Name,a.Name,tc.Description ")
						_T("FROM RoutingRules rr ")
						_T("LEFT OUTER JOIN MessageSources ms ON rr.MessageSourceTypeID=ms.MessageSourceID ")
						_T("LEFT OUTER JOIN TicketBoxes tb ON rr.AssignToTicketBox=tb.TicketBoxID ")
						_T("LEFT OUTER JOIN Agents a ON rr.AssignToAgent=a.AgentID ")
						_T("LEFT OUTER JOIN TicketCategories tc ON rr.AssignToTicketCategory=tc.TicketCategoryID ")
						_T("ORDER BY %s"), sSort.c_str() );

	// Do the query
	GetQuery().Initialize();	

	BINDCOL_LONG( GetQuery(), m_RoutingRuleID );
	BINDCOL_LONG( GetQuery(), m_HitCount );
	BINDCOL_TCHAR( GetQuery(), m_RuleDescrip );
	BINDCOL_BIT( GetQuery(), m_IsEnabled );
	BINDCOL_LONG( GetQuery(), m_OrderIndex );
	BINDCOL_TINYINT( GetQuery(), m_DeleteImmediately );
	BINDCOL_LONG( GetQuery(), m_MessageSourceTypeID );
	BINDCOL_LONG( GetQuery(), m_AssignToTicketBox );
	BINDCOL_LONG( GetQuery(), m_PriorityID );
	BINDCOL_LONG( GetQuery(), m_AssignToTicketCategory );
	BINDCOL_LONG( GetQuery(), m_AssignToAgent );
	BINDCOL_LONG( GetQuery(), m_OfficeHours );
	
	GetQuery().Execute( sQuery.c_str() );

	while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem(_T("RoutingRule"));
		GetXMLGen().AddChildAttrib( _T("ID"), m_RoutingRuleID );
		AddTicketBoxName( _T("AssignToTicketBox"), m_AssignToTicketBox );
		AddDeleteOption( _T("DeleteImmediately"), m_DeleteImmediately );
		GetXMLGen().AddChildAttrib( _T("HitCount"), m_HitCount );
		AddPriority(  _T("Priority"), m_PriorityID );
		GetXMLGen().AddChildAttrib( _T("RuleDescrip"), m_RuleDescrip );
		GetXMLGen().AddChildAttrib( _T("OrderIndex"), m_OrderIndex+1 );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
		AddMessageSourceTypeMatch( _T("MessageSourceType"), m_MessageSourceTypeID );
		AddAgentName( _T("AssignToAgent"), m_AssignToAgent );
		AddTicketCategoryName( _T("AssignToTicketCategory"), m_AssignToTicketCategory );
		GetXMLGen().AddChildAttrib( _T("OfficeHours"), m_OfficeHours );		
	}

	// Add default routing rule options
	CEMSString sValue;
	GetXMLGen().AddChildElem( _T("Options") );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_TICKETBOX, sValue );
	GetXMLGen().AddChildAttrib( _T("UseMatchTicketBox"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_AGENT, sValue );
	GetXMLGen().AddChildAttrib( _T("UseMatchAgentAddress"), sValue.c_str() );

	// add sort information
	GetXMLGen().AddChildElem( _T("RoutingRules") );
	GetXMLGen().AddChildAttrib( _T("SortBy"), m_SortBy);
	GetXMLGen().AddChildAttrib( _T("SortAscending"), m_SortDir);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query for a particular routing rule
// 
////////////////////////////////////////////////////////////////////////////////
int CRoutingRules::Query(void)
{
	
	if( m_RoutingRuleID != 0 )
	{
		// Do the query
		GetQuery().Initialize();
		
		BINDCOL_LONG( GetQuery(), m_RoutingRuleID );
		BINDCOL_LONG( GetQuery(), m_AutoReplyWithStdResponse );
		BINDCOL_BIT( GetQuery(), m_AutoReplyQuoteMsg );
		BINDCOL_BIT( GetQuery(), m_AutoReplyCloseTicket );
		BINDCOL_LONG( GetQuery(), m_ForwardFromAgent );
		BINDCOL_BIT( GetQuery(), m_ForwardFromContact );
		BINDCOL_LONG( GetQuery(), m_AssignToTicketBox );
		BINDCOL_TINYINT( GetQuery(), m_DeleteImmediately );
		BINDCOL_LONG( GetQuery(), m_HitCount );
		BINDCOL_LONG( GetQuery(), m_PriorityID );
		BINDCOL_TCHAR( GetQuery(), m_RuleDescrip );
		BINDCOL_LONG( GetQuery(), m_AssignToAgent );
		BINDCOL_BIT( GetQuery(), m_AutoReplyEnable );
		BINDCOL_BIT( GetQuery(), m_ForwardEnable );
		BINDCOL_BIT( GetQuery(), m_IsEnabled );
		BINDCOL_LONG( GetQuery(), m_MessageSourceTypeID );
		BINDCOL_LONG( GetQuery(), m_OrderIndex );
		BINDCOL_BIT( GetQuery(), m_AssignToAgentEnable );
		BINDCOL_BIT( GetQuery(), m_DeleteImmediatelyEnable );
		BINDCOL_TCHAR( GetQuery(), m_AutoReplyFrom );
		BINDCOL_LONG( GetQuery(), m_AssignToAgentAlg );
		BINDCOL_LONG( GetQuery(), m_AssignToTicketBoxAlg );
		BINDCOL_BIT( GetQuery(), m_AssignUniqueTicketID);
		BINDCOL_LONG( GetQuery(), m_AssignToTicketCategory );
		BINDCOL_BIT( GetQuery(), m_AlertEnable);
		BINDCOL_BIT( GetQuery(), m_AlertIncludeSubject);
		BINDCOL_LONG( GetQuery(), m_AlertToAgentID );
		BINDCOL_TCHAR( GetQuery(), m_AlertText );
		BINDCOL_BIT( GetQuery(), m_ToOrFrom );
		BINDCOL_BIT( GetQuery(), m_ConsiderAllOwned );
		BINDCOL_BIT( GetQuery(), m_DoProcessingRules );
		BINDCOL_LONG( GetQuery(), m_LastOwnerID );
		BINDCOL_LONG( GetQuery(), m_MatchMethod );
		BINDCOL_TCHAR( GetQuery(), m_ForwardFromEmail );
		BINDCOL_TCHAR( GetQuery(), m_ForwardFromName );
		BINDCOL_BIT( GetQuery(), m_ForwardInTicket );
		BINDCOL_BIT( GetQuery(), m_AutoReplyInTicket );
		BINDCOL_BIT( GetQuery(), m_AllowRemoteReply );
		BINDCOL_LONG( GetQuery(), m_DoNotAssign );
		BINDCOL_BIT( GetQuery(), m_QuoteOriginal );
		BINDCOL_LONG( GetQuery(), m_MultiMail );
		BINDCOL_LONG( GetQuery(), m_SetOpenOwner );
		BINDCOL_LONG( GetQuery(), m_OfficeHours );
		BINDCOL_LONG( GetQuery(), m_IgnoreTracking );
		BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );

		GetQuery().Execute( _T("SELECT RoutingRuleID,AutoReplyWithStdResponse,")
							_T("AutoReplyQuoteMsg,AutoReplyCloseTicket,")
							_T("ForwardFromAgent,ForwardFromContact,")
							_T("AssignToTicketBox,DeleteImmediately,")
							_T("HitCount,PriorityID,RuleDescrip,AssignToAgent,")
							_T("AutoReplyEnable,ForwardEnable,")
							_T("IsEnabled,MessageSourceTypeID,OrderIndex,")
							_T("AssignToAgentEnable,DeleteImmediatelyEnable,AutoReplyFrom, ")
							_T("AssignToAgentAlg,AssignToTicketBoxAlg,AssignUniqueTicketID,AssignToTicketCategory, ")
							_T("AlertEnable,AlertIncludeSubject,AlertToAgentID,AlertText,ToOrFrom,ConsiderAllOwned,DoProcessingRules,LastOwnerID,MatchMethod,ForwardFromEmail,ForwardFromName,ForwardInTicket,AutoReplyInTicket,AllowRemoteReply,DoNotAssign,QuoteOriginal,MultiMail,SetOpenOwner,OfficeHours,IgnoreTracking ")
							_T("FROM RoutingRules ")
	  						_T("WHERE RoutingRuleID=? ") );

		if ( GetQuery().Fetch() == S_OK )
		{
			/*************************************************/
			// Get Match To Addresses

			GetQuery().Initialize();

			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			BINDCOL_LONG( GetQuery(), mta.m_MatchID );
			BINDCOL_TCHAR( GetQuery(), mta.m_EmailAddress );

			GetQuery().Execute( _T("SELECT MatchID,EmailAddress ")
								_T("FROM MatchToAddresses ")
  								_T("WHERE RoutingRuleID=?") );


			while( GetQuery().Fetch() == S_OK )
			{
				m_mta.push_back( mta );
			}
			
			/*************************************************/
			// Get Assign to Agents

			GetQuery().Initialize();

			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			BINDCOL_LONG( GetQuery(), ata.m_AgentRoutingID );
			BINDCOL_LONG( GetQuery(), ata.m_AgentID );
			
			GetQuery().Execute( _T("SELECT AgentRoutingID,AgentID ")
								_T("FROM AgentRouting ")
  								_T("WHERE RoutingRuleID=?") );


			while( GetQuery().Fetch() == S_OK )
			{
				m_ata.push_back( ata );
			}

			/*************************************************/
			// Get Assign to TicketBoxes

			GetQuery().Initialize();

			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			BINDCOL_LONG( GetQuery(), attb.m_TicketBoxRoutingID );
			BINDCOL_LONG( GetQuery(), attb.m_TicketBoxID );
			BINDCOL_LONG( GetQuery(), attb.m_Percentage );

			GetQuery().Execute( _T("SELECT TicketBoxRoutingID,TicketBoxID,Percentage ")
								_T("FROM TicketBoxRouting ")
  								_T("WHERE RoutingRuleID=?") );


			while( GetQuery().Fetch() == S_OK )
			{
				m_attb.push_back( attb );
			}

			/*************************************************/
			// Get Match From Addresses

			GetQuery().Initialize();

			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			BINDCOL_LONG( GetQuery(), mfa.m_MatchID );
			BINDCOL_TCHAR( GetQuery(), mfa.m_EmailAddress );

			GetQuery().Execute( _T("SELECT MatchID,EmailAddress ")
								_T("FROM MatchFromAddresses ")
  								_T("WHERE RoutingRuleID=?") );

			while( GetQuery().Fetch() == S_OK )
			{
				m_mfa.push_back( mfa );
			}

			/*************************************************/
			// Get Match Text

			GetQuery().Initialize();

			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			BINDCOL_LONG( GetQuery(), mt.m_MatchID );
			BINDCOL_TINYINT( GetQuery(), mt.m_MatchLocation );
			BINDCOL_BIT( GetQuery(), mt.m_IsRegEx );

			GetQuery().Execute( _T("SELECT MatchID,MatchLocation,IsRegEX,MatchText ")
								_T("FROM MatchText ")
								_T("WHERE RoutingRuleID=?") );

			while( GetQuery().Fetch() == S_OK )
			{
				GETDATA_TEXT( GetQuery(), mt.m_MatchText );
				m_mt.push_back( mt );
				if( mt.m_MatchID >= m_MaxID)
					m_MaxID = mt.m_MatchID + 1;
			}

			m_DBMatchTextMaxID = m_MaxID;

			/*************************************************/
			// Forward To Addresses

			GetQuery().Initialize();

			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			BINDCOL_LONG( GetQuery(), fta.m_AddressID );
			BINDCOL_TCHAR( GetQuery(), fta.m_EmailAddress );

			GetQuery().Execute( _T("SELECT AddressID,EmailAddress ")
								_T("FROM ForwardToAddresses ")
								_T("WHERE RoutingRuleID=?") );

			while( GetQuery().Fetch() == S_OK )
			{
				m_fta.push_back( fta );
			}

			/*************************************************/
			// Forward CC Addresses

			GetQuery().Initialize();

			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			BINDCOL_LONG( GetQuery(), fca.m_AddressID );
			BINDCOL_TCHAR( GetQuery(), fca.m_EmailAddress );

			GetQuery().Execute( _T("SELECT AddressID,EmailAddress ")
								_T("FROM ForwardCCAddresses ")
								_T("WHERE RoutingRuleID=?") );

			while( GetQuery().Fetch() == S_OK )
			{
				m_fca.push_back( fca );
			}

			/*************************************************/

			if( m_AutoReplyWithStdResponse != 0 )
			{
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_AutoReplyWithStdResponse );
				BINDCOL_TCHAR( GetQuery(), m_StdRespName );
				GetQuery().Execute( _T("SELECT Subject ")
									_T("FROM StandardResponses ")
									_T("WHERE StandardResponseID=?") );
				GetQuery().Fetch();
			}
		}
		else
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
		}
	}

	GenerateXML();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update a routing rule
// 
////////////////////////////////////////////////////////////////////////////////
int CRoutingRules::Update()
{
	bool bFound;
	vector<int> DeleteIDs;
	map<int,int> DBIDs;
	vector<int>::iterator iterDel;
	int ID;
	int MaxID;

	DecodeForm();

	// Update all the required fields

	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_RuleDescrip );
	BINDPARAM_BIT( GetQuery(), m_IsEnabled );
	BINDPARAM_BIT( GetQuery(), m_AutoReplyEnable );
	BINDPARAM_BIT( GetQuery(), m_ForwardEnable );
	BINDPARAM_LONG( GetQuery(), m_AssignToTicketBoxAlg );
	BINDPARAM_LONG( GetQuery(), m_AssignToAgentAlg);
	BINDPARAM_BIT( GetQuery(), m_DeleteImmediatelyEnable );
	BINDPARAM_LONG( GetQuery(), m_PriorityID );
	BINDPARAM_LONG( GetQuery(), m_MessageSourceTypeID );
	BINDPARAM_LONG( GetQuery(), m_AssignToAgent );
	BINDPARAM_LONG( GetQuery(), m_AssignToTicketBox );
	BINDPARAM_BIT( GetQuery(), m_AssignUniqueTicketID );
	BINDPARAM_LONG( GetQuery(), m_AssignToTicketCategory );
	BINDPARAM_BIT( GetQuery(), m_AlertEnable );
	BINDPARAM_BIT( GetQuery(), m_AlertIncludeSubject );
	BINDPARAM_LONG( GetQuery(), m_AlertToAgentID );
	BINDPARAM_TCHAR( GetQuery(), m_AlertText );
	BINDPARAM_BIT( GetQuery(), m_ToOrFrom );
	BINDPARAM_BIT( GetQuery(), m_ConsiderAllOwned );
	BINDPARAM_BIT( GetQuery(), m_DoProcessingRules );
	BINDPARAM_LONG( GetQuery(), m_LastOwnerID );
	BINDPARAM_LONG( GetQuery(), m_MatchMethod );
	BINDPARAM_TCHAR( GetQuery(), m_ForwardFromEmail );
	BINDPARAM_TCHAR( GetQuery(), m_ForwardFromName );
	BINDPARAM_BIT( GetQuery(), m_ForwardInTicket );
	BINDPARAM_BIT( GetQuery(), m_AutoReplyInTicket );
	BINDPARAM_BIT( GetQuery(), m_AllowRemoteReply );
	BINDPARAM_LONG( GetQuery(), m_DoNotAssign );
	BINDPARAM_BIT( GetQuery(), m_QuoteOriginal );
	BINDPARAM_LONG( GetQuery(), m_MultiMail );
	BINDPARAM_LONG( GetQuery(), m_SetOpenOwner );
	BINDPARAM_LONG( GetQuery(), m_OfficeHours );
	BINDPARAM_LONG( GetQuery(), m_IgnoreTracking );
	BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
	GetQuery().Execute( _T("UPDATE RoutingRules ")
						_T("SET RuleDescrip=?,IsEnabled=?,AutoReplyEnable=?,ForwardEnable=?,")
						_T("AssignToTicketBoxAlg=?,AssignToAgentAlg=?,DeleteImmediatelyEnable=?,PriorityID=?,")
						_T("MessageSourceTypeID=?,AssignToAgent=?,AssignToTicketBox=?,AssignUniqueTicketID=?, AssignToTicketCategory=?,")
						_T("AlertEnable=?,AlertIncludeSubject=?,AlertToAgentID=?,AlertText=?,ToOrFrom=?,ConsiderAllOwned=?,DoProcessingRules=?,LastOwnerID=?,MatchMethod=?,ForwardFromEmail=?,ForwardFromName=?,ForwardInTicket=?,AutoReplyInTicket=?,AllowRemoteReply=?,DoNotAssign=?,QuoteOriginal=?,MultiMail=?,SetOpenOwner=?,OfficeHours=?,IgnoreTracking=? ")
						_T("WHERE RoutingRuleID=?") );		

	if( GetQuery().GetRowCount() == 0 )
	{
		// bail out if m_RoutingRuleID was invalid
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}
	else
	{
		if( m_DeleteImmediatelyEnable )
		{
			GetQuery().Reset();
			BINDPARAM_TINYINT( GetQuery(), m_DeleteImmediately );
			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			GetQuery().Execute( _T("UPDATE RoutingRules ")
								_T("SET DeleteImmediately=? ")
								_T("WHERE RoutingRuleID=?") );		

			if( GetQuery().GetRowCount() == 0 )
			{
				// bail out if m_RoutingRuleID was invalid
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			}
		}

		if( m_AutoReplyEnable )
		{
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), m_AutoReplyWithStdResponse );
			BINDPARAM_BIT( GetQuery(), m_AutoReplyQuoteMsg );
			BINDPARAM_BIT( GetQuery(), m_AutoReplyCloseTicket );
			BINDPARAM_TCHAR( GetQuery(), m_AutoReplyFrom );
			BINDPARAM_BIT( GetQuery(), m_AutoReplyInTicket );
			BINDPARAM_BIT( GetQuery(), m_AllowRemoteReply );
			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			GetQuery().Execute( _T("UPDATE RoutingRules ")
								_T("SET AutoReplyWithStdResponse=?,AutoReplyQuoteMsg=?,")
								_T("AutoReplyCloseTicket=?,AutoReplyFrom=?,AutoReplyInTicket=?,AllowRemoteReply=? ")
								_T("WHERE RoutingRuleID=?") );		

			if( GetQuery().GetRowCount() == 0 )
			{
				// bail out if m_RoutingRuleID was invalid
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			}
		}
	
		// MatchToAddresses

		// Walk the database records and compare to our list
		DeleteIDs.clear();
		GetQuery().Reset();
		BINDCOL_LONG( GetQuery(), mta.m_MatchID );
		BINDCOL_TCHAR( GetQuery(), mta.m_EmailAddress );
		BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
		GetQuery().Execute( _T("SELECT MatchID,EmailAddress ")
				            _T("FROM MatchToAddresses ")
							_T("WHERE RoutingRuleID=?") );
		while( GetQuery().Fetch() == S_OK )
		{
			bFound = false;
			for ( mtaIter = m_mta.begin(); mtaIter != m_mta.end(); mtaIter++)
			{
				if( mtaIter->m_MatchID == 0 &&
					_tcscmp( mta.m_EmailAddress, mtaIter->m_EmailAddress ) == 0 )
				{
					mtaIter->m_MatchID = mta.m_MatchID;
					bFound = true;
					break;
				}
			}
			if( bFound == FALSE )
			{
				DeleteIDs.push_back(mta.m_MatchID);
			}
		}
		// Delete the ones in the database that we couldn't find in our list
		for( iterDel = DeleteIDs.begin(); iterDel != DeleteIDs.end(); iterDel++ )
		{
			ID = (*iterDel);
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), ID );
			GetQuery().Execute( _T("DELETE ")
								_T("FROM MatchToAddresses ")
								_T("WHERE MatchID=?") );		
		}
		// Add the ones from our list that we couldn't find in the database
		for ( mtaIter = m_mta.begin(); mtaIter != m_mta.end(); mtaIter++)
		{
			if( mtaIter->m_MatchID == 0 )
			{
				mtaIter->Insert( GetQuery() );
			}
		}
	
		// Agent Routing

		// Delete existing records for this Routing Rule
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
		GetQuery().Execute( _T("DELETE FROM AgentRouting ")
							_T("WHERE RoutingRuleID=?") );
		
		// Add the ones from our list 
		for ( ataIter = m_ata.begin(); ataIter != m_ata.end(); ataIter++)
		{
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), ataIter->m_AgentID );
			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			GetQuery().Execute( _T("INSERT INTO AgentRouting ")
						_T("(AgentID,RoutingRuleID) ")
						_T("VALUES")
						_T("(?,?)") );
		}
	
		// TicketBox Routing

		// Delete existing records for this Routing Rule
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
		GetQuery().Execute( _T("DELETE FROM TicketBoxRouting ")
							_T("WHERE RoutingRuleID=?") );
		
		// Add the ones from our list
		for ( attbIter = m_attb.begin(); attbIter != m_attb.end(); attbIter++)
		{
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), attbIter->m_Percentage );
			BINDPARAM_LONG( GetQuery(), attbIter->m_TicketBoxID );
			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			GetQuery().Execute( _T("INSERT INTO TicketBoxRouting ")
						_T("(Percentage,TicketBoxID,RoutingRuleID) ")
						_T("VALUES")
						_T("(?,?,?)") );
		}
	
		// MatchFromAddresses

		// Walk the database records and compare to our list
		DeleteIDs.clear();
		GetQuery().Reset();
		BINDCOL_LONG( GetQuery(), mfa.m_MatchID );
		BINDCOL_TCHAR( GetQuery(), mfa.m_EmailAddress );
		BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
		GetQuery().Execute( _T("SELECT MatchID,EmailAddress ")
				            _T("FROM MatchFromAddresses ")
							_T("WHERE RoutingRuleID=?") );
		while( GetQuery().Fetch() == S_OK )
		{
			bFound = false;
			for ( mfaIter = m_mfa.begin(); mfaIter != m_mfa.end(); mfaIter++)
			{
				if( mfaIter->m_MatchID == 0
					&& _tcscmp( mfa.m_EmailAddress, mfaIter->m_EmailAddress ) == 0 )
				{
					mfaIter->m_MatchID = mfa.m_MatchID;
					bFound = true;
					break;
				}
			}
			if( bFound == FALSE )
			{
				DeleteIDs.push_back(mfa.m_MatchID);
			}
		}
		// Delete the ones in the database that we couldn't find in our list
		for( iterDel = DeleteIDs.begin(); iterDel != DeleteIDs.end(); iterDel++ )
		{
			ID = (*iterDel);
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), ID );
			GetQuery().Execute( _T("DELETE ")
								_T("FROM MatchFromAddresses ")
								_T("WHERE MatchID=?") );		
		}
		// Add the ones from our list that we couldn't find in the database
		for ( mfaIter = m_mfa.begin(); mfaIter != m_mfa.end(); mfaIter++)
		{
			if( mfaIter->m_MatchID == 0 )
			{
				mfaIter->Insert( GetQuery() );
			}
		}

		// Forward options
		if( m_ForwardEnable )
		{
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), m_ForwardFromAgent );
			BINDPARAM_BIT( GetQuery(), m_ForwardFromContact );
			BINDPARAM_BIT( GetQuery(), m_ForwardInTicket );
			BINDPARAM_BIT( GetQuery(), m_QuoteOriginal );
			BINDPARAM_LONG( GetQuery(), m_MultiMail );
			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			GetQuery().Execute( _T("UPDATE RoutingRules ")
								_T("SET ForwardFromAgent=?,ForwardFromContact=?,ForwardInTicket=?,QuoteOriginal=?,MultiMail=? ")
								_T("WHERE RoutingRuleID=?") );		

			if( GetQuery().GetRowCount() == 0 )
			{
				// bail out if m_RoutingRuleID was invalid
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			}
			
			// ForwardToAddresses

			// Walk the database records and compare to our list
			DeleteIDs.clear();
			GetQuery().Reset();
			BINDCOL_LONG( GetQuery(), fta.m_AddressID );
			BINDCOL_TCHAR( GetQuery(), fta.m_EmailAddress );
			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			GetQuery().Execute( _T("SELECT AddressID,EmailAddress ")
								_T("FROM ForwardToAddresses ")
								_T("WHERE RoutingRuleID=?") );
			while( GetQuery().Fetch() == S_OK )
			{
				bFound = false;
				for ( ftaIter = m_fta.begin(); ftaIter != m_fta.end(); ftaIter++)
				{
					if( ftaIter->m_AddressID == 0
						&& _tcscmp( fta.m_EmailAddress, ftaIter->m_EmailAddress ) == 0 )
					{
						ftaIter->m_AddressID = fta.m_AddressID;
						bFound = true;
						break;
					}
				}
				if( bFound == FALSE )
				{
					DeleteIDs.push_back(fta.m_AddressID);
				}
			}
			// Delete the ones in the database that we couldn't find in our list
			for( iterDel = DeleteIDs.begin(); iterDel != DeleteIDs.end(); iterDel++ )
			{
				ID = (*iterDel);
				GetQuery().Reset();
				BINDPARAM_LONG( GetQuery(), ID );
				GetQuery().Execute( _T("DELETE ")
									_T("FROM ForwardToAddresses ")
									_T("WHERE AddressID=?") );		
			}
			// Add the ones from our list that we couldn't find in the database
			for ( ftaIter = m_fta.begin(); ftaIter != m_fta.end(); ftaIter++)
			{
				if( ftaIter->m_AddressID == 0 )
				{
					ftaIter->Insert( GetQuery() );
				}
			}

			// ForwardCcAddressses

			// Walk the database records and compare to our list
			DeleteIDs.clear();
			GetQuery().Reset();
			BINDCOL_LONG( GetQuery(), fca.m_AddressID );
			BINDCOL_TCHAR( GetQuery(), fca.m_EmailAddress );
			BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
			GetQuery().Execute( _T("SELECT AddressID,EmailAddress ")
								_T("FROM ForwardCCAddresses ")
								_T("WHERE RoutingRuleID=?") );
			while( GetQuery().Fetch() == S_OK )
			{
				bFound = false;
				for ( fcaIter = m_fca.begin(); fcaIter != m_fca.end(); fcaIter++)
				{
					if( fcaIter->m_AddressID == 0 
						&& _tcscmp( fca.m_EmailAddress, fcaIter->m_EmailAddress ) == 0 )
					{
						fcaIter->m_AddressID = fca.m_AddressID;
						bFound = true;
						break;
					}
				}
				if( bFound == FALSE )
				{
					DeleteIDs.push_back(fca.m_AddressID);
				}
			}
			// Delete the ones in the database that we couldn't find in our list
			for( iterDel = DeleteIDs.begin(); iterDel != DeleteIDs.end(); iterDel++ )
			{
				ID = (*iterDel);
				GetQuery().Reset();
				BINDPARAM_LONG( GetQuery(), ID );
				GetQuery().Execute( _T("DELETE ")
									_T("FROM ForwardCCAddresses ")
									_T("WHERE AddressID=?") );		
			}
			// Add the ones from our list that we couldn't find in the database
			for ( fcaIter = m_fca.begin(); fcaIter != m_fca.end(); fcaIter++)
			{
				if( fcaIter->m_AddressID == 0 )
				{
					fcaIter->Insert( GetQuery() );
				}
			}
		}

		// Match text
		GetISAPIData().GetXMLLong( _T("DBMatchTextMaxID"), MaxID );

		DeleteIDs.clear();
		GetQuery().Reset();
		BINDCOL_LONG( GetQuery(), mt.m_MatchID );
		BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
		GetQuery().Execute( _T("SELECT MatchID ")
				            _T("FROM MatchText ")
							_T("WHERE RoutingRuleID=?") );
		while( GetQuery().Fetch() == S_OK )
		{
			DBIDs[ mt.m_MatchID ] = 1;
			bFound = false;
			for ( mtIter = m_mt.begin(); mtIter != m_mt.end(); mtIter++)
			{
				if( mtIter->m_MatchID == mt.m_MatchID )
				{
					bFound = true;
					break;
				}
			}
			if( bFound == FALSE )
			{
				DeleteIDs.push_back(mt.m_MatchID);
			}
		}
		// Delete the ones in the database that we couldn't find in our list
		for( iterDel = DeleteIDs.begin(); iterDel != DeleteIDs.end(); iterDel++ )
		{
			ID = (*iterDel);
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), ID );
			GetQuery().Execute( _T("DELETE ")
								_T("FROM MatchText ")
								_T("WHERE MatchID=?") );		
		}
		// now either update or add.
		for ( mtIter = m_mt.begin(); mtIter != m_mt.end(); mtIter++)
		{
			if( DBIDs.find( mtIter->m_MatchID ) != DBIDs.end() && mtIter->m_MatchID < MaxID )
			{
				mtIter->Update( GetQuery() );
			}
			else
			{
				mtIter->Insert( GetQuery() );
			}
		}

		m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_RoutingRules );	
		InvalidateRoutingRules();
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a routing rule
// 
////////////////////////////////////////////////////////////////////////////////
int CRoutingRules::Delete()
{
	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
// TicketBox Routing

	// Delete existing records for this Routing Rule
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
	GetQuery().Execute( _T("DELETE FROM AgentRouting WHERE RoutingRuleID = ?") );
	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM TicketBoxRouting WHERE RoutingRuleID = ?") );
	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM MatchToAddresses WHERE RoutingRuleID = ?") );
	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM MatchFromAddresses WHERE RoutingRuleID = ?") );
	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM ForwardToAddresses WHERE RoutingRuleID = ?") );
	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM ForwardCCAddresses WHERE RoutingRuleID = ?") );
	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM MatchText WHERE RoutingRuleID = ?") );
	
	if( TRoutingRules::Delete( GetQuery() ) == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}	
		
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create a new, blank routing rule
// 
////////////////////////////////////////////////////////////////////////////////
int CRoutingRules::New()
{
	DecodeForm();
	
	// Get the maximum OrderIndex from the database and add 1
	// This insures that the new routing rule will appear at the bottom of the list
	m_OrderIndex = 0;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_OrderIndex );
	GetQuery().Execute( _T("SELECT MAX(OrderIndex) FROM RoutingRules") );
	GetQuery().Fetch();
	m_OrderIndex++;

	TRoutingRules::Insert( GetQuery() );

	// Make sure our IDs are all correct and add new rows as necessary

	for ( mtaIter = m_mta.begin(); mtaIter != m_mta.end(); mtaIter++)
	{
		mtaIter->m_RoutingRuleID = m_RoutingRuleID;
		mtaIter->Insert( GetQuery() );
	}
	for ( mfaIter = m_mfa.begin(); mfaIter != m_mfa.end(); mfaIter++)
	{
		mfaIter->m_RoutingRuleID = m_RoutingRuleID;
		mfaIter->Insert( GetQuery() );
	}
	for ( ftaIter = m_fta.begin(); ftaIter != m_fta.end(); ftaIter++)
	{
		ftaIter->m_RoutingRuleID = m_RoutingRuleID;
		ftaIter->Insert( GetQuery() );
	}
	for ( fcaIter = m_fca.begin(); fcaIter != m_fca.end(); fcaIter++)
	{
		fcaIter->m_RoutingRuleID = m_RoutingRuleID;
		fcaIter->Insert( GetQuery() );
	}
	for ( mtIter = m_mt.begin(); mtIter != m_mt.end(); mtIter++)
	{
		mtIter->m_RoutingRuleID = m_RoutingRuleID;
		mtIter->Insert( GetQuery() );
	}

	// Agent Routing
	// Add the ones from our list 
	for ( ataIter = m_ata.begin(); ataIter != m_ata.end(); ataIter++)
	{
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), ataIter->m_AgentID );
		BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
		GetQuery().Execute( _T("INSERT INTO AgentRouting ")
					_T("(AgentID,RoutingRuleID) ")
					_T("VALUES")
					_T("(?,?)") );
	}
	
		// TicketBox Routing
	// Add the ones from our list
	for ( attbIter = m_attb.begin(); attbIter != m_attb.end(); attbIter++)
	{
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), attbIter->m_Percentage );
		BINDPARAM_LONG( GetQuery(), attbIter->m_TicketBoxID );
		BINDPARAM_LONG( GetQuery(), m_RoutingRuleID );
		GetQuery().Execute( _T("INSERT INTO TicketBoxRouting ")
					_T("(Percentage,TicketBoxID,RoutingRuleID) ")
					_T("VALUES")
					_T("(?,?,?)") );
	}
	
	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_RoutingRules );	
	InvalidateRoutingRules();

	return 0;
}

void CRoutingRules::DecodeForm(void)
{
	CEMSString sCollection;
	CEMSString sParam;
	CEMSString sValue;
	tstring sChunk;
	tstring sAgentGroup;
	int iSelectBox;
	int ID;
	int location;
	int isregex;
	
	m_mta.clear();
	m_mfa.clear();
	m_fta.clear();
	m_fca.clear();
	m_mt.clear();

	// rule description
	GetISAPIData().GetXMLTCHAR( _T("RuleDescrip"), m_RuleDescrip, 125 );
	 
	if ( _tcslen( m_RuleDescrip ) < 1 )
		THROW_VALIDATION_EXCEPTION( _T("RuleDescrip"), _T("Please specify a description for the routing rule") );

	// rule enabled?
	GetISAPIData().GetXMLLong( _T("RuleEnabled"), m_IsEnabled );

	// Auto-Reply
	GetISAPIData().GetXMLLong( _T("AutoReplyEnabled"), m_AutoReplyEnable );	
	if( m_AutoReplyEnable )
	{
		GetISAPIData().GetXMLLong( _T("AutoReplyStandardResponseId"), m_AutoReplyWithStdResponse );
	}
	else
	{
		m_AutoReplyWithStdResponse = 0;
	}

	// Forward
	GetISAPIData().GetXMLLong( _T("ForwardCopyEnabled"), m_ForwardEnable );
	if( m_ForwardEnable )
	{
		GetISAPIData().GetXMLLong( _T("ForwardFromId"), m_ForwardFromAgent );
		GetISAPIData().GetXMLLong( _T("ForwardInTicket"), m_ForwardInTicket );
		GetISAPIData().GetXMLLong( _T("QuoteOriginal"), m_QuoteOriginal );
		if(m_ForwardFromAgent == -1)
		{
			GetISAPIData().GetXMLTCHAR( _T("ForwardFromEmail"), m_ForwardFromEmail, 125, true );
			GetISAPIData().GetXMLTCHAR( _T("ForwardFromName"), m_ForwardFromName, 125, true );
		}
		GetISAPIData().GetXMLLong( _T("chkMultiMail"), m_MultiMail, true);		
	}
	else
	{
		m_ForwardFromAgent = 0;
	}

	// ForwardFromContact is derived from the value of ForwardFromID
	m_ForwardFromContact = ( m_ForwardFromAgent == 0 ) ? 1 : 0;

	// Assign to agent
	GetISAPIData().GetXMLLong( _T("AssignOwnership"), m_AssignToAgentAlg );
	GetISAPIData().GetXMLLong( _T("AssignToAgentId"), m_AssignToAgent );
	GetISAPIData().GetXMLLong( _T("chkOpenOwner"), m_SetOpenOwner );
	GetISAPIData().GetXMLLong( _T("OfficeHours"), m_OfficeHours );
	GetISAPIData().GetXMLLong( _T("chkIgnoreTracking"), m_IgnoreTracking );
	
	// Delete options
	GetISAPIData().GetXMLLong( _T("ShredEnabled"), m_DeleteImmediatelyEnable );

	if( m_DeleteImmediatelyEnable )
		GetISAPIData().GetXMLLong( _T("DeleteOptionId"), m_DeleteImmediately );

	GetISAPIData().GetXMLLong( _T("AssignTicketBox"), m_AssignToTicketBoxAlg, true );
	GetISAPIData().GetXMLLong( _T("AssignToTicketBoxId"), m_AssignToTicketBox, true );
	GetISAPIData().GetXMLLong( _T("MessageSourceTypeId"), m_MessageSourceTypeID ); 		
	GetISAPIData().GetXMLLong( _T("PriorityID"), m_PriorityID );
	GetISAPIData().GetXMLLong( _T("AutoReplyQuoteMsg"), m_AutoReplyQuoteMsg );
	GetISAPIData().GetXMLLong( _T("AutoReplyCloseTicket"), m_AutoReplyCloseTicket );
	GetISAPIData().GetXMLLong( _T("AutoReplyInTicket"), m_AutoReplyInTicket );		
	GetISAPIData().GetXMLLong( _T("AllowRemoteReply"), m_AllowRemoteReply );		
	GetISAPIData().GetXMLTCHAR( _T("AutoReplyEmailAddress"), m_AutoReplyFrom, 255, true );
	GetISAPIData().GetXMLLong( _T("DBMatchTextMaxID"), m_DBMatchTextMaxID );
	GetISAPIData().GetXMLLong( _T("MatchTextMaxID"), m_MaxID );
	GetISAPIData().GetXMLLong( _T("chkCreateNewTickets"), m_AssignUniqueTicketID );
	GetISAPIData().GetXMLLong( _T("MatchMethod"), m_MatchMethod );
	GetISAPIData().GetXMLLong( _T("AssignToTicketCategoryId"), m_AssignToTicketCategory );
	GetISAPIData().GetXMLLong( _T("AlertEnable"), m_AlertEnable );
	GetISAPIData().GetXMLLong( _T("AlertIncludeSubject"), m_AlertIncludeSubject );
	GetISAPIData().GetXMLLong( _T("ToOrFrom"), m_ToOrFrom );
	GetISAPIData().GetXMLLong( _T("ConsiderAllOwned"), m_ConsiderAllOwned );
	GetISAPIData().GetXMLLong( _T("DoProcessingRules"), m_DoProcessingRules );
	GetISAPIData().GetXMLString( _T("AgentOrGroup"), sAgentGroup );
	if( sAgentGroup == "Agent" )
	{
		GetISAPIData().GetXMLLong( _T("Agent"), iSelectBox );
		m_AlertToAgentID = iSelectBox;
	}
	else
	{
		GetISAPIData().GetXMLLong( _T("Group"), iSelectBox );
		m_AlertToAgentID = -(iSelectBox);
	}
	GetISAPIData().GetXMLTCHAR( _T("AlertText"), m_AlertText, 255, true );
	
	GetISAPIData().GetXMLString( _T("MatchToAddressCollection"), sCollection );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextChunk( 1, sChunk ) )
	{
		_tcscpy( mta.m_EmailAddress, sChunk.c_str() );
		mta.m_RoutingRuleID = m_RoutingRuleID;
		mta.m_MatchID = 0;
		m_mta.push_back( mta );
	}
	
	GetISAPIData().GetXMLString( _T("MatchFromAddressCollection"), sCollection );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextChunk( 1, sChunk ) )
	{
		_tcscpy( mfa.m_EmailAddress, sChunk.c_str() );
		mfa.m_RoutingRuleID = m_RoutingRuleID;
		mfa.m_MatchID = 0;
		m_mfa.push_back( mfa );
	}

	// Match Text
	GetISAPIData().GetXMLString( _T("MatchTextCollection"), sCollection );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( ID ) )
	{
		mt.m_MatchID = ID;
		mt.m_RoutingRuleID = m_RoutingRuleID;
		
		sParam.Format( _T("MatchText%d"), ID );
		GetISAPIData().GetXMLString( (TCHAR*) sParam.c_str(), sValue );
		PutStringProperty( sValue, &mt.m_MatchText, &mt.m_MatchTextAllocated );

		sParam.Format( _T("MatchLocation%d"), ID );
		GetISAPIData().GetXMLLong( (TCHAR*) sParam.c_str(), location );
		mt.m_MatchLocation = (unsigned char)location;

		sParam.Format( _T("IsRegEX%d"), ID );
		GetISAPIData().GetXMLLong( (TCHAR*) sParam.c_str(), isregex );
		mt.m_IsRegEx = (unsigned char)isregex;

		if(isregex == 1 && !TestRegEx(sValue.c_str()))
		{
			THROW_EMS_EXCEPTION( E_InvalidParameters, _T("Invalid Regular Expression in Match Text!"));
		}
		
		m_mt.push_back( mt );

		if( mt.m_MatchID >= m_MaxID)
			m_MaxID = mt.m_MatchID + 1;
	}

	// Agent Routing
	GetISAPIData().GetXMLString( _T("AGRouteCollection"), sCollection );	
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( ID ) )
	{
		ata.m_AgentID = ID;
		ata.m_RoutingRuleID = m_RoutingRuleID;
		if(ata.m_AgentID>0)
		{
			m_ata.push_back( ata );
		}
	}
	GetISAPIData().GetXMLLong( _T("DoNotAssign"), m_DoNotAssign );

	// TicketBox Routing
	GetISAPIData().GetXMLString( _T("TBRouteCollection"), sCollection );
	int i=0;
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( ID ) )
	{
		if ( i==0 )
		{
			attb.m_TicketBoxID = ID;
			i=1;
		}
		else
		{
			attb.m_Percentage = ID;
			attb.m_RoutingRuleID = m_RoutingRuleID;
			m_attb.push_back( attb );
			i=0;
		}
	}

	GetISAPIData().GetXMLString( _T("ForwardToAddressCollection"), sCollection );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextChunk( 1, sChunk ) )
	{
		_tcscpy( fta.m_EmailAddress, sChunk.c_str() );
		fta.m_RoutingRuleID = m_RoutingRuleID;
		fta.m_AddressID = 0;
		m_fta.push_back( fta );
	}

	GetISAPIData().GetXMLString( _T("ForwardCcAddressCollection"), sCollection );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextChunk( 1, sChunk ) )
	{
		_tcscpy( fca.m_EmailAddress, sChunk.c_str() );
		fca.m_RoutingRuleID = m_RoutingRuleID;
		fca.m_AddressID = 0;
		m_fca.push_back( fca );
	}

	if( m_AutoReplyEnable )
	{
		sValue.assign( m_AutoReplyFrom );

		if (!sValue.ValidateEmailAddr())
		{
			CEMSString sError;
			sError.Format( _T("The email address specified [%s] is invalid"), sValue.c_str() );
			THROW_VALIDATION_EXCEPTION( _T("AutoReplyEmailAddress"), sError );
		}
	}
}

int CRoutingRules::ReOrder( CURLAction& action )
{
	CEMSString s;
	int Order = 0;
	int ID;

	while( true )
	{
		s.Format( _T("ruleorder%d"), Order );
	
		if( GetISAPIData().GetXMLLong( s.c_str(), ID, true ) == false )
		{
			break;
		}

		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), Order );
		BINDPARAM_LONG( GetQuery(), ID );
		GetQuery().Execute( _T("UPDATE RoutingRules ")
							_T("SET OrderIndex=? ")
							_T("WHERE RoutingRuleID=? ") );

		Order++;
	} 
	
	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_RoutingRules );

	return 0;
}

void CRoutingRules::GenerateXML(void)
{
	CEMSString sStdRespName;
	
	GetXMLGen().AddChildElem(_T("RoutingRule"));
	GetXMLGen().AddChildAttrib( _T("ID"), m_RoutingRuleID );
	GetXMLGen().AddChildAttrib( _T("RuleDescrip"), m_RuleDescrip );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	GetXMLGen().AddChildAttrib( _T("AutoReplyEnable"), m_AutoReplyEnable );
	GetXMLGen().AddChildAttrib( _T("AutoReplyWithStdResponseID"), m_AutoReplyWithStdResponse );
	GetXMLGen().AddChildAttrib( _T("AlertEnable"), m_AlertEnable );
	GetXMLGen().AddChildAttrib( _T("AlertIncludeSubject"), m_AlertIncludeSubject );
	if( m_AlertToAgentID >= 0 )
	{
		GetXMLGen().AddChildAttrib( _T("AlertToAgentID"), m_AlertToAgentID );
	}
	else
	{
		m_AlertToAgentID = abs(m_AlertToAgentID);
		GetXMLGen().AddChildAttrib( _T("AlertToGroupID"), m_AlertToAgentID );
	}
		
	GetXMLGen().AddChildAttrib( _T("AlertText"), m_AlertText );
	
	GetXMLGen().AddChildAttrib( _T("ForwardEnable"), m_ForwardEnable );
	GetXMLGen().AddChildAttrib( _T("ForwardFromEmail"), m_ForwardFromEmail );
	GetXMLGen().AddChildAttrib( _T("ForwardFromName"), m_ForwardFromName );
	GetXMLGen().AddChildAttrib( _T("ForwardInTicket"), m_ForwardInTicket );
	GetXMLGen().AddChildAttrib( _T("QuoteOriginal"), m_QuoteOriginal );
	GetXMLGen().AddChildAttrib( _T("MultiMail"), m_MultiMail );
	
	if( m_ForwardFromContact )
		GetXMLGen().AddChildAttrib( _T("ForwardFromID"), 0 );
	else
		GetXMLGen().AddChildAttrib( _T("ForwardFromID"), m_ForwardFromAgent );

	GetXMLGen().AddChildAttrib( _T("AssignToAgentID"), m_AssignToAgent );
	GetXMLGen().AddChildAttrib( _T("chkOpenOwner"), m_SetOpenOwner );
	GetXMLGen().AddChildAttrib( _T("OfficeHours"), m_OfficeHours );
	GetXMLGen().AddChildAttrib( _T("chkIgnoreTracking"), m_IgnoreTracking );
	GetXMLGen().AddChildAttrib( _T("DeleteImmediately"), m_DeleteImmediately );
	GetXMLGen().AddChildAttrib( _T("DeleteImmediatelyEnable"), m_DeleteImmediatelyEnable );
	GetXMLGen().AddChildAttrib( _T("AssignToTicketBoxID"), m_AssignToTicketBox );
	GetXMLGen().AddChildAttrib( _T("AssignToTicketCategoryID"), m_AssignToTicketCategory );
	GetXMLGen().AddChildAttrib( _T("MessageSourceTypeID"), m_MessageSourceTypeID );
	GetXMLGen().AddChildAttrib( _T("PriorityID"), m_PriorityID );
	GetXMLGen().AddChildAttrib( _T("AutoReplyQuoteMsg"), m_AutoReplyQuoteMsg );
	GetXMLGen().AddChildAttrib( _T("AutoReplyInTicket"), m_AutoReplyInTicket );
	GetXMLGen().AddChildAttrib( _T("AllowRemoteReply"), m_AllowRemoteReply );
	GetXMLGen().AddChildAttrib( _T("AutoReplyCloseTicket"), m_AutoReplyCloseTicket );
	GetXMLGen().AddChildAttrib( _T("HitCount"), m_HitCount );
	GetXMLGen().AddChildAttrib( _T("OrderIndex"), m_OrderIndex );
	GetXMLGen().AddChildAttrib( _T("DBMatchTextMaxID"), m_DBMatchTextMaxID );
	GetXMLGen().AddChildAttrib( _T("MatchTextMaxID"), m_MaxID );
	GetXMLGen().AddChildAttrib( _T("AssignToTicketCategory"), m_AssignToTicketCategory );
	GetXMLGen().AddChildAttrib( _T("ToOrFrom"), m_ToOrFrom );
	GetXMLGen().AddChildAttrib( _T("ConsiderAllOwned"), m_ConsiderAllOwned );
	GetXMLGen().AddChildAttrib( _T("DoProcessingRules"), m_DoProcessingRules );
	
	int nOoo=0; //1
	int nOffline=0; //2
	int nDnd=0; //4
	int nNotAvail=0; //8
	int nAway=0; //16
	int nOnline=0; //32
	
	if(m_DoNotAssign%2!=0){nOoo=1;}
	GetXMLGen().AddChildAttrib( _T("chkOoo"), nOoo );
	int nVal;
	CEMSString sNums;
	sNums.Format(_T("2,3,6,7,10,11,14,15,18,19,22,23,26,27,30,31,34,35,38,39,42,43,46,47,50,51,54,55,58,59,62,63"));
	sNums.CDLInit();
	while ( sNums.CDLGetNextInt( nVal ) )
	{
		if ( nVal == m_DoNotAssign ){nOffline=1;break;}
	}
	GetXMLGen().AddChildAttrib( _T("chkOffline"), nOffline );	
	sNums.Format(_T("4,5,6,7,12,13,14,15,20,21,23,24,28,29,30,31,36,37,38,39,44,45,46,47,52,53,54,55,60,61,62,63"));
	sNums.CDLInit();
	while ( sNums.CDLGetNextInt( nVal ) )
	{
		if ( nVal == m_DoNotAssign ){nDnd=1;break;}
	}
	GetXMLGen().AddChildAttrib( _T("chkDnd"), nDnd );
	sNums.Format(_T("8,9,10,11,12,13,14,15,24,25,26,27,28,29,30,31,40,41,42,43,44,45,46,47,56,57,58,59,60,61,62,63"));
	sNums.CDLInit();
	while ( sNums.CDLGetNextInt( nVal ) )
	{
		if ( nVal == m_DoNotAssign ){nNotAvail=1;break;}
	}
	GetXMLGen().AddChildAttrib( _T("chkNotAvail"), nNotAvail );
	if((m_DoNotAssign>15 && m_DoNotAssign<32)||(m_DoNotAssign > 47)){nAway=1;}
	GetXMLGen().AddChildAttrib( _T("chkAway"), nAway );
	if(m_DoNotAssign>31){nOnline=1;}
	GetXMLGen().AddChildAttrib( _T("chkOnline"), nOnline );

	sStdRespName.assign( m_StdRespName );
	sStdRespName.EscapeJavascript();
	GetXMLGen().AddChildAttrib( _T("AutoReplyStdResponseName"), sStdRespName.c_str() );
	GetXMLGen().AddChildAttrib( _T("AutoReplyEmailAddress"), m_AutoReplyFrom );
	GetXMLGen().AddChildAttrib( _T("AssignOwnership"), m_AssignToAgentAlg );
	GetXMLGen().AddChildAttrib( _T("AssignTicketBox"), m_AssignToTicketBoxAlg );
	GetXMLGen().AddChildAttrib( _T("assignuniqueticketid"), m_AssignUniqueTicketID );
	
	if (m_AssignUniqueTicketID)
		m_MatchMethod = 2;

	GetXMLGen().AddChildAttrib( _T("MatchMethod"), m_MatchMethod );
	GetXMLGen().AddChildAttrib( _T("AssignToTicketCategoryID"), m_AssignToTicketCategory );
	
	GetXMLGen().IntoElem();

	for( mtaIter = m_mta.begin(); mtaIter != m_mta.end(); mtaIter++ )
	{
		GetXMLGen().AddChildElem(_T("MatchToAddress"));
		GetXMLGen().AddChildAttrib( _T("EmailAddress"), mtaIter->m_EmailAddress );
	}

	for( mfaIter = m_mfa.begin(); mfaIter != m_mfa.end(); mfaIter++ )
	{
		GetXMLGen().AddChildElem(_T("MatchFromAddress"));
		GetXMLGen().AddChildAttrib( _T("EmailAddress"), mfaIter->m_EmailAddress );
	}

	for( mtIter = m_mt.begin(); mtIter != m_mt.end(); mtIter++ )
	{
		GetXMLGen().AddChildElem(_T("MatchText"));
		GetXMLGen().AddChildAttrib( _T("ID"), mtIter->m_MatchID );
		GetXMLGen().AddChildAttrib( _T("MatchLocation"), mtIter->m_MatchLocation );
		GetXMLGen().AddChildAttrib( _T("IsRegEx"), mtIter->m_IsRegEx );
		GetXMLGen().SetChildData( mtIter->m_MatchText, 1 );
	}

	for( ataIter = m_ata.begin(); ataIter != m_ata.end(); ataIter++ )
	{
		GetXMLGen().AddChildElem(_T("AgentMembers"));
		GetXMLGen().AddChildAttrib( _T("AGID"), ataIter->m_AgentID );			
	}

	for( attbIter = m_attb.begin(); attbIter != m_attb.end(); attbIter++ )
	{
		GetXMLGen().AddChildElem(_T("TicketBoxMembers"));
		GetXMLGen().AddChildAttrib( _T("TBID"), attbIter->m_TicketBoxID );
		GetXMLGen().AddChildAttrib( _T("PERC"), attbIter->m_Percentage );		
	}

	for( ftaIter = m_fta.begin(); ftaIter != m_fta.end(); ftaIter++ )
	{
		GetXMLGen().AddChildElem(_T("ForwardToAddress"));
		GetXMLGen().AddChildAttrib( _T("EmailAddress"), ftaIter->m_EmailAddress );
	}

	for( fcaIter = m_fca.begin(); fcaIter != m_fca.end(); fcaIter++ )
	{
		GetXMLGen().AddChildElem(_T("ForwardCCAddress"));
		GetXMLGen().AddChildAttrib( _T("EmailAddress"), fcaIter->m_EmailAddress );
	}

	GetXMLGen().OutOfElem();

	// add in the lists
	ListMessageSourceTypeMatch();
	ListTicketPriorities();
	ListDeleteOptions();
	ListEnabledAgentNames();
	ListMatchLocations();
	ListTicketBoxNames( GetXMLGen() );
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

	ListGroupNames();
}

////////////////////////////////////////////////////////////////////////////////
// 
// Options
// 
////////////////////////////////////////////////////////////////////////////////
int CRoutingRules::Options( tstring& sAction )
{
	CEMSString sValue;

	if( sAction.compare( _T("update") ) == 0 )
	{
		DISABLE_IN_DEMO();

		GetISAPIData().GetXMLLong( _T("AssignTicketBox"), m_AssignToTicketBoxAlg );
		GetISAPIData().GetXMLLong( _T("AssignToTicketBoxId"), m_AssignToTicketBox );
		GetISAPIData().GetXMLLong( _T("AssignOwnership"), m_AssignToAgentAlg );
		GetISAPIData().GetXMLLong( _T("AssignToAgentId"), m_AssignToAgent );
		GetISAPIData().GetXMLLong( _T("AssignToTicketCategoryId"), m_AssignToTicketCategory );
		GetISAPIData().GetXMLLong( _T("CreateNewTicket"), m_AssignUniqueTicketID );
		GetISAPIData().GetXMLLong( _T("MatchMethod"), m_MatchMethod );
		
		std::string sOnline;
		if(!GetISAPIData().GetXMLString("chkOnline",sOnline, true))
			sOnline.assign(_T("0"));

		std::string sAway;
		if(!GetISAPIData().GetXMLString("chkAway",sAway, true))
			sAway.assign(_T("0"));

		std::string sNotAvail;
		if(!GetISAPIData().GetXMLString("chkNotAvail",sNotAvail, true))
			sNotAvail.assign(_T("0"));

		std::string sOffline;
		if(!GetISAPIData().GetXMLString("chkOffline",sOffline, true))
			sOffline.assign(_T("0"));

		std::string sOoo;
		if(!GetISAPIData().GetXMLString("chkOoo",sOoo, true))
			sOoo.assign(_T("0"));

		std::string sOo;
		if(!GetISAPIData().GetXMLString("chkOpenOwner",sOo, true))
			sOo.assign(_T("0"));
	
		sValue.Format( _T("%d"), m_AssignToTicketBoxAlg );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_TICKETBOX,  sValue );
		sValue.Format( _T("%d"), m_AssignToAgentAlg );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_AGENT,  sValue );
		sValue.Format( _T("%d"), m_AssignToTicketBox );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_TICKETBOXID,  sValue );
		sValue.Format( _T("%d"), m_AssignToAgent );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_AGENTID,  sValue );
		sValue.Format( _T("%d"), m_AssignToTicketCategory );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_TICKETCATEGORYID,  sValue );
		sValue.Format( _T("%d"), m_AssignUniqueTicketID );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_NEW_TICKET,  sValue );
		sValue.Format( _T("%d"), m_MatchMethod );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_METHOD,  sValue );
		sValue.Format( _T("%s"), sOnline.c_str() );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_ONLINE,  sValue );
		sValue.Format( _T("%s"), sAway.c_str() );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_AWAY,  sValue );
		sValue.Format( _T("%s"), sNotAvail.c_str() );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_NOTAVAIL,  sValue );
		sValue.Format( _T("%s"), sOffline.c_str() );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_OFFLINE,  sValue );
		sValue.Format( _T("%s"), sOoo.c_str() );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_OOO,  sValue );
		sValue.Format( _T("%s"), sOo.c_str() );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_CHECK_OPEN_OWNER,  sValue );

		InvalidateServerParameters( true );
		GetRoutingEngine().ReloadConfig( EMS_RoutingRules );
	}

	GetXMLGen().AddChildElem( _T("RoutingRule") );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_TICKETBOX, sValue );
	GetXMLGen().AddChildAttrib( _T("AssignTicketBox"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_AGENT, sValue );
	GetXMLGen().AddChildAttrib( _T("AssignOwnership"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_AGENTID, sValue );
	GetXMLGen().AddChildAttrib( _T("AssignToAgentID"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_TICKETBOXID, sValue );
	GetXMLGen().AddChildAttrib( _T("AssignToTicketBoxID"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_TICKETCATEGORYID, sValue );
	GetXMLGen().AddChildAttrib( _T("AssignToTicketCategory"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_NEW_TICKET, sValue );
	GetXMLGen().AddChildAttrib( _T("CreateNewTicket"), sValue.c_str() );

	if ( sValue.c_str() == "1" )
		GetXMLGen().AddChildAttrib( _T("MatchMethod"), "2" );
	else
        GetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_METHOD, sValue );
		GetXMLGen().AddChildAttrib( _T("MatchMethod"), sValue.c_str() );

	GetServerParameter( EMS_SRVPARAM_DEF_RR_ONLINE, sValue );
	GetXMLGen().AddChildAttrib( _T("chkOnline"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_AWAY, sValue );
	GetXMLGen().AddChildAttrib( _T("chkAway"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_NOTAVAIL, sValue );
	GetXMLGen().AddChildAttrib( _T("chkNotAvail"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_OFFLINE, sValue );
	GetXMLGen().AddChildAttrib( _T("chkOffline"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_OOO, sValue );
	GetXMLGen().AddChildAttrib( _T("chkOoo"), sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_DEF_RR_CHECK_OPEN_OWNER, sValue );
	GetXMLGen().AddChildAttrib( _T("chkOpenOwner"), sValue.c_str() );

	ListEnabledAgentNames();
	ListTicketBoxNames( GetXMLGen() );
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

	return 0;
}

int CRoutingRules::ResetOrder(void)
{
	std::list<long> rrIDs;
	std::list<long>::iterator iter;
	int Order = 0;	

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_RoutingRuleID );	
	GetQuery().Execute( _T("SELECT RoutingRuleID FROM RoutingRules ORDER BY OrderIndex") );
	while ( GetQuery().Fetch() == S_OK )
	{
		rrIDs.push_back(m_RoutingRuleID);
	}
	
	for(iter = rrIDs.begin(); iter != rrIDs.end(); iter++)
	{
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), Order );
		BINDPARAM_LONG( GetQuery(), *iter );
		GetQuery().Execute( _T("UPDATE RoutingRules ")
							_T("SET OrderIndex=? ")
							_T("WHERE RoutingRuleID=? ") );

		Order++;
	}		
	
	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_RoutingRules );
	InvalidateRoutingRules();

	return 0;
}

bool CRoutingRules::TestRegEx(dca::String sRegEx)
{
	bool bIsValid = true;
	try
	{
		boost::regex pattern (sRegEx,boost::regex_constants::perl);		
	}
	catch(...)
	{
		bIsValid = false;
	}
	return bIsValid;	
}