// ProcessingRules.cpp: implementation of the CProcessingRules class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProcessingRules.h"
#include "StringFns.h"
#include <.\boost\regex.hpp>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessingRules::CProcessingRules(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_DBMatchTextMaxID = 0;
	m_MaxID = 0;
	m_ProcessingRuleID = 0;
	m_SortBy = 0;
	m_SortDir = 1;
}

CProcessingRules::~CProcessingRules()
{
}

int CProcessingRules::Run( CURLAction& action )
{  
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_ProcessingRuleID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_ProcessingRuleID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_ProcessingRuleID);
		}
	}

	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
	{
		if (!GetISAPIData().GetXMLString( _T("Action"), sAction, true ))
			GetISAPIData().GetURLString( _T("Action"), sAction, true );
	}

	if( GetISAPIData().m_sPage.compare( _T("processingrule") ) == 0 )
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

			if( m_ProcessingRuleID == 0 )
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
		if( m_ProcessingRuleID == 0 )
			action.m_sPageTitle.assign( "New Processing Rule" );

		return Query();
	}
	
	if( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();

		if( m_ProcessingRuleID == 0 )
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

void CProcessingRules::ResetHitCount(void)
{
	GetQuery().Initialize();	
	BINDPARAM_LONG( GetQuery(), m_ProcessingRuleID );
	GetQuery().Execute( _T("UPDATE ProcessingRules Set HitCount=0 WHERE ProcessingRuleID=?") );
		
	if( GetQuery().GetRowCount() == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
}

int CProcessingRules::ListAll(void)
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
		sSort.Format( _T("HitCount %s"), 
			SORT_ORDER( m_SortDir ) );
		break;	

	default:
		sSort.Format( _T("OrderIndex ASC"));
	}

	// Format the Query
	sQuery.Format( _T("SELECT ProcessingRuleID,HitCount,RuleDescrip,")
						_T("IsEnabled,OrderIndex,PrePost ")
						_T("FROM ProcessingRules ")
						_T("ORDER BY %s"), sSort.c_str() );	

	// Do the query
	GetQuery().Initialize();	

	BINDCOL_LONG( GetQuery(), m_ProcessingRuleID );
	BINDCOL_LONG( GetQuery(), m_HitCount );
	BINDCOL_TCHAR( GetQuery(), m_RuleDescrip );
	BINDCOL_BIT( GetQuery(), m_IsEnabled );
	BINDCOL_LONG( GetQuery(), m_OrderIndex );
	BINDCOL_BIT( GetQuery(), m_PrePost );
	
	GetQuery().Execute( sQuery.c_str() );

	while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem(_T("ProcessingRule"));
		GetXMLGen().AddChildAttrib( _T("ID"), m_ProcessingRuleID );
		GetXMLGen().AddChildAttrib( _T("HitCount"), m_HitCount );
		GetXMLGen().AddChildAttrib( _T("RuleDescrip"), m_RuleDescrip );
		GetXMLGen().AddChildAttrib( _T("OrderIndex"), m_OrderIndex+1 );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
		GetXMLGen().AddChildAttrib( _T("PrePost"), m_PrePost );
	}

	// add sort information
	GetXMLGen().AddChildElem( _T("ProcessingRules") );
	GetXMLGen().AddChildAttrib( _T("SortBy"), m_SortBy);
	GetXMLGen().AddChildAttrib( _T("SortAscending"), m_SortDir);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query for a particular processing rule
// 
////////////////////////////////////////////////////////////////////////////////
int CProcessingRules::Query(void)
{
	
	if( m_ProcessingRuleID != 0 )
	{
		// Do the query
		GetQuery().Initialize();
		
		BINDCOL_LONG( GetQuery(), m_HitCount );
		BINDCOL_TCHAR( GetQuery(), m_RuleDescrip );
		BINDCOL_BIT( GetQuery(), m_IsEnabled );
		BINDCOL_LONG( GetQuery(), m_OrderIndex );
		BINDCOL_BIT( GetQuery(), m_PrePost );
		BINDCOL_LONG( GetQuery(), m_ActionID );
		BINDCOL_LONG( GetQuery(), m_ActionType );
		BINDPARAM_LONG( GetQuery(), m_ProcessingRuleID );

		GetQuery().Execute( _T("SELECT HitCount,RuleDescrip,IsEnabled,OrderIndex,PrePost,ActionID,ActionType ")
							_T("FROM ProcessingRules ")
	  						_T("WHERE ProcessingRuleID=? ") );

		if ( GetQuery().Fetch() == S_OK )
		{
						
			/*************************************************/
			// Get Match Text

			GetQuery().Initialize();

			BINDPARAM_LONG( GetQuery(), m_ProcessingRuleID );
			BINDCOL_LONG( GetQuery(), mt.m_MatchID );
			BINDCOL_TINYINT( GetQuery(), mt.m_MatchLocation );
			BINDCOL_BIT( GetQuery(), mt.m_IsRegEx );
			GetQuery().Execute( _T("SELECT MatchID,MatchLocation,IsRegEx,MatchText ")
								_T("FROM MatchTextP ")
								_T("WHERE ProcessingRuleID=?") );

			while( GetQuery().Fetch() == S_OK )
			{
				GETDATA_TEXT( GetQuery(), mt.m_MatchText );
				m_mt.push_back( mt );
				if( mt.m_MatchID >= m_MaxID)
					m_MaxID = mt.m_MatchID + 1;
			}

			m_DBMatchTextMaxID = m_MaxID;			
			
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
// Update a processing rule
// 
////////////////////////////////////////////////////////////////////////////////
int CProcessingRules::Update()
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
	BINDPARAM_BIT( GetQuery(), m_PrePost );
	BINDPARAM_LONG( GetQuery(), m_ActionType );
	BINDPARAM_LONG( GetQuery(), m_ActionID );
	BINDPARAM_LONG( GetQuery(), m_ProcessingRuleID );
	
	GetQuery().Execute( _T("UPDATE ProcessingRules ")
						_T("SET RuleDescrip=?,IsEnabled=?,PrePost=?,ActionType=?,ActionID=? ")
						_T("WHERE ProcessingRuleID=?") );		

	if( GetQuery().GetRowCount() == 0 )
	{
		// bail out if m_ProcessingRuleID was invalid
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}
	else
	{
		// Match text
		GetISAPIData().GetXMLLong( _T("DBMatchTextMaxID"), MaxID );

		DeleteIDs.clear();
		GetQuery().Reset();
		BINDCOL_LONG( GetQuery(), mt.m_MatchID );
		BINDPARAM_LONG( GetQuery(), m_ProcessingRuleID );
		GetQuery().Execute( _T("SELECT MatchID ")
				            _T("FROM MatchTextP ")
							_T("WHERE ProcessingRuleID=?") );
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
								_T("FROM MatchTextP ")
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

		m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_ProcessingRules );	
		InvalidateProcessingRules();
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a processing rule
// 
////////////////////////////////////////////////////////////////////////////////
int CProcessingRules::Delete()
{
	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_ProcessingRuleID );

	// Delete existing records for this Processing Rule
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_ProcessingRuleID );
	GetQuery().Execute( _T("DELETE FROM MatchTextP WHERE ProcessingRuleID = ?") );
	
	if( TProcessingRules::Delete( GetQuery() ) == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}	
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create a new, blank processing rule
// 
////////////////////////////////////////////////////////////////////////////////
int CProcessingRules::New()
{
	DecodeForm();
	
	// Get the maximum OrderIndex from the database and add 1
	// This insures that the new processing rule will appear at the bottom of the list
	m_OrderIndex = 0;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_OrderIndex );
	GetQuery().Execute( _T("SELECT MAX(OrderIndex) FROM ProcessingRules") );
	GetQuery().Fetch();
	m_OrderIndex++;

	TProcessingRules::Insert( GetQuery() );

	// Make sure our IDs are all correct and add new rows as necessary

	for ( mtIter = m_mt.begin(); mtIter != m_mt.end(); mtIter++)
	{
		mtIter->m_ProcessingRuleID = m_ProcessingRuleID;
		mtIter->Insert( GetQuery() );
	}

	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_ProcessingRules );	
	InvalidateProcessingRules();

	return 0;
}

void CProcessingRules::DecodeForm(void)
{
	CEMSString sCollection;
	CEMSString sParam;
	CEMSString sValue;
	tstring sChunk;
	tstring sAgentGroup;
	int ID;
	int location;
	int isregex;
		
	m_mt.clear();

	// rule description
	GetISAPIData().GetXMLTCHAR( _T("RuleDescrip"), m_RuleDescrip, 125 );
	 
	if ( _tcslen( m_RuleDescrip ) < 1 )
		THROW_VALIDATION_EXCEPTION( _T("RuleDescrip"), _T("Please specify a description for the processing rule") );

	// rule enabled?
	GetISAPIData().GetXMLLong( _T("RuleEnabled"), m_IsEnabled );
	GetISAPIData().GetXMLLong( _T("PrePost"), m_PrePost );
	GetISAPIData().GetXMLLong( _T("ActionType"), m_ActionType );
	GetISAPIData().GetXMLLong( _T("ActionID"), m_ActionID );

	GetISAPIData().GetXMLLong( _T("DBMatchTextMaxID"), m_DBMatchTextMaxID );
	GetISAPIData().GetXMLLong( _T("MatchTextMaxID"), m_MaxID );
	
	// Match Text
	GetISAPIData().GetXMLString( _T("MatchTextCollection"), sCollection );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( ID ) )
	{
		mt.m_MatchID = ID;
		mt.m_ProcessingRuleID = m_ProcessingRuleID;
				
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
}

int CProcessingRules::ReOrder( CURLAction& action )
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
		GetQuery().Execute( _T("UPDATE ProcessingRules ")
							_T("SET OrderIndex=? ")
							_T("WHERE ProcessingRuleID=? ") );

		Order++;
	} 
	
	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_ProcessingRules );

	return 0;
}

void CProcessingRules::GenerateXML(void)
{
	CEMSString sStdRespName;
	
	GetXMLGen().AddChildElem(_T("ProcessingRule"));
	GetXMLGen().AddChildAttrib( _T("ID"), m_ProcessingRuleID );
	GetXMLGen().AddChildAttrib( _T("RuleDescrip"), m_RuleDescrip );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	GetXMLGen().AddChildAttrib( _T("PrePost"), m_PrePost );
	GetXMLGen().AddChildAttrib( _T("ActionType"), m_ActionType );
	GetXMLGen().AddChildAttrib( _T("ActionID"), m_ActionID );
	GetXMLGen().AddChildAttrib( _T("HitCount"), m_HitCount );
	GetXMLGen().AddChildAttrib( _T("OrderIndex"), m_OrderIndex );
	GetXMLGen().AddChildAttrib( _T("DBMatchTextMaxID"), m_DBMatchTextMaxID );
	GetXMLGen().AddChildAttrib( _T("MatchTextMaxID"), m_MaxID );
		
	GetXMLGen().IntoElem();

	for( mtIter = m_mt.begin(); mtIter != m_mt.end(); mtIter++ )
	{
		GetXMLGen().AddChildElem(_T("MatchText"));
		GetXMLGen().AddChildAttrib( _T("ID"), mtIter->m_MatchID );
		GetXMLGen().AddChildAttrib( _T("MatchLocation"), mtIter->m_MatchLocation );
		GetXMLGen().AddChildAttrib( _T("IsRegEx"), mtIter->m_IsRegEx );
		GetXMLGen().SetChildData( mtIter->m_MatchText, 1 );
	}

	GetXMLGen().OutOfElem();

	// add in the lists
	ListMatchLocations();
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

}

int CProcessingRules::ResetOrder(void)
{
	std::list<long> prIDs;
	std::list<long>::iterator iter;
	int Order = 0;	

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_ProcessingRuleID );	
	GetQuery().Execute( _T("SELECT ProcessingRuleID FROM ProcessingRules ORDER BY OrderIndex") );
	while ( GetQuery().Fetch() == S_OK )
	{
		prIDs.push_back(m_ProcessingRuleID);
	}
	
	for(iter = prIDs.begin(); iter != prIDs.end(); iter++)
	{
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), Order );
		BINDPARAM_LONG( GetQuery(), *iter );
		GetQuery().Execute( _T("UPDATE ProcessingRules ")
							_T("SET OrderIndex=? ")
							_T("WHERE ProcessingRuleID=? ") );

		Order++;
	}		
	
	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_ProcessingRules );
	InvalidateProcessingRules();

	return 0;
}

bool CProcessingRules::TestRegEx(dca::String sRegEx)
{
	try
	{
		boost::regex pattern (sRegEx,boost::regex_constants::perl);		
	}
	catch(...)
	{
		return false;
	}
	return true;	
}