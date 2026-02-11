// IPRanges.cpp: implementation of the CIPRanges class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IPRanges.h"
#include "StringFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIPRanges::CIPRanges( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CIPRanges::~CIPRanges()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
// Run - main entry point
// 
////////////////////////////////////////////////////////////////////////////////
int CIPRanges::Run( CURLAction& action )
{
	tstring sAction;

	// Check security
	RequireAdmin();

	GetISAPIData().GetXMLString( _T("action"), sAction, true);
	
	if( GetISAPIData().m_sPage.compare( _T("ipranges") )  == 0 )
	{
		if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();
			return UpdateIPRanges( action );
		}
	
		return IPRanges( action );
	}
	
	return IPAccess( action );
}

////////////////////////////////////////////////////////////////////////////////
// 
// IPAccess is a summary page
// 
////////////////////////////////////////////////////////////////////////////////
int CIPRanges::IPAccess( CURLAction& action )
{
	TCHAR szIP[16];

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_IPRangeID );
	BINDCOL_LONG( GetQuery(), m_AgentID );
	BINDCOL_LONG( GetQuery(), m_GroupID );
	BINDCOL_LONG( GetQuery(), m_LowerRange );
	BINDCOL_LONG( GetQuery(), m_UpperRange );

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("agent_ipaccess") ) == 0 )
	{
		GetQuery().Execute( _T("SELECT IPRangeID,IPRanges.AgentID,GroupID,LowerRange,UpperRange ")
							_T("FROM IPRanges ")
							_T("INNER JOIN Agents on IPRanges.AgentID = Agents.AgentID ")
							_T("WHERE GroupID=0 AND Agents.IsDeleted=0 ")
							_T("ORDER BY Agents.Name") );

		int nAgentID = 0;
		GetXMLGen().SavePos();

		while( GetQuery().Fetch() == S_OK )
		{
			if ( nAgentID != m_AgentID )
			{
				if ( nAgentID != 0 )
					GetXMLGen().OutOfElem();

				GetXMLGen().AddChildElem( _T("Agent") );
				GetXMLGen().AddChildAttrib( _T("ID"), m_AgentID );
				AddAgentName( _T("Name"), m_AgentID );
				GetXMLGen().IntoElem();
			}

			GetXMLGen().AddChildElem( _T("IPRange") );
			GetXMLGen().AddChildAttrib( _T("ID"), m_IPRangeID );
		
			InetNToA( m_LowerRange, szIP );
			GetXMLGen().AddChildAttrib( _T("LowerRange"), szIP );

			InetNToA( m_UpperRange, szIP );
			GetXMLGen().AddChildAttrib( _T("UpperRange"), szIP );

			if ( nAgentID != m_AgentID )
				nAgentID = m_AgentID;
		}
	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("group_ipaccess") ) == 0 )
	{	
		GetQuery().Execute( _T("SELECT IPRangeID,AgentID,IPRanges.GroupID,LowerRange,UpperRange ")
							_T("FROM IPRanges ")
							_T("INNER JOIN Groups ON IPRanges.GroupID=Groups.GroupID ")
							_T("WHERE AgentID=0 AND Groups.IsDeleted=0 ")
							_T("ORDER BY Groups.GroupName") );

		int nGroupID = 0;
		GetXMLGen().SavePos();

		while( GetQuery().Fetch() == S_OK )
		{
			if ( nGroupID != m_GroupID )
			{
				if ( nGroupID != 0 )
					GetXMLGen().OutOfElem();

				GetXMLGen().AddChildElem( _T("Group") );
				GetXMLGen().AddChildAttrib( _T("ID"), m_GroupID );
				AddGroupName( _T("Name"), m_GroupID );
				GetXMLGen().IntoElem();
			}

			GetXMLGen().AddChildElem( _T("IPRange") );
			GetXMLGen().AddChildAttrib( _T("ID"), m_IPRangeID );

			InetNToA( m_LowerRange, szIP );
			GetXMLGen().AddChildAttrib( _T("LowerRange"), szIP );

			InetNToA( m_UpperRange, szIP );
			GetXMLGen().AddChildAttrib( _T("UpperRange"), szIP );

			if ( nGroupID != m_GroupID )
				nGroupID = m_GroupID;
		}
	}

	GetXMLGen().RestorePos();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// IPRanges is the detail page
// 
////////////////////////////////////////////////////////////////////////////////
int CIPRanges::IPRanges( CURLAction& action )
{
	TCHAR szIP[16];
	int AgentID = 0;
	int GroupID = 0;
	int nMaxID = 0;

	GetISAPIData().GetURLLong( _T("agentID"), AgentID, true );
	GetISAPIData().GetURLLong( _T("groupID"), GroupID, true );

	if( GroupID == 0 && AgentID != 0 )
	{
		GetXMLGen().AddChildElem( _T("IPRanges") );
		GetXMLGen().AddChildAttrib( _T("Type"), _T("Agent") );
		GetXMLGen().AddChildAttrib( _T("ID"), AgentID );
		AddAgentName( _T("Name"), AgentID );
		GetXMLGen().IntoElem();

		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), AgentID );
		BINDCOL_LONG( GetQuery(), m_IPRangeID );
		BINDCOL_LONG( GetQuery(), m_AgentID );
		BINDCOL_LONG( GetQuery(), m_GroupID );
		BINDCOL_LONG( GetQuery(), m_LowerRange );
		BINDCOL_LONG( GetQuery(), m_UpperRange );
		GetQuery().Execute( _T("SELECT IPRangeID,AgentID,GroupID,LowerRange,UpperRange ")
							_T("FROM IPRanges ")
							_T("WHERE GroupID=0 AND AgentID=? ")
							_T("ORDER BY AgentID") );

		while( GetQuery().Fetch() == S_OK )
		{
			GetXMLGen().AddChildElem( _T("IPRange") );
			GetXMLGen().AddChildAttrib( _T("ID"), m_IPRangeID );

			InetNToA( m_LowerRange, szIP );
			GetXMLGen().AddChildAttrib( _T("LowerRange"), szIP );

			InetNToA( m_UpperRange, szIP );
			GetXMLGen().AddChildAttrib( _T("UpperRange"), szIP );

			if( m_IPRangeID >= nMaxID )
				nMaxID = m_IPRangeID + 1;		
		}

		GetXMLGen().OutOfElem();
	}
	else if ( AgentID == 0 && GroupID != 0 )
	{
		GetXMLGen().AddChildElem( _T("IPRanges") );
		GetXMLGen().AddChildAttrib( _T("Type"), _T("Group") );
		GetXMLGen().AddChildAttrib( _T("ID"), GroupID );
		AddGroupName( _T("Name"), GroupID );
		GetXMLGen().IntoElem();

		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), GroupID );
		BINDCOL_LONG( GetQuery(), m_IPRangeID );
		BINDCOL_LONG( GetQuery(), m_AgentID );
		BINDCOL_LONG( GetQuery(), m_GroupID );
		BINDCOL_LONG( GetQuery(), m_LowerRange );
		BINDCOL_LONG( GetQuery(), m_UpperRange );
		GetQuery().Execute( _T("SELECT IPRangeID,AgentID,GroupID,LowerRange,UpperRange ")
							_T("FROM IPRanges ")
							_T("WHERE AgentID=0 AND GroupID=? ")
							_T("ORDER BY GroupID") );

		while( GetQuery().Fetch() == S_OK )
		{
			GetXMLGen().AddChildElem( _T("IPRange") );
			GetXMLGen().AddChildAttrib( _T("ID"), m_IPRangeID );
			InetNToA( m_LowerRange, szIP );
			GetXMLGen().AddChildAttrib( _T("LowerRange"), szIP );

			InetNToA( m_UpperRange, szIP );
			GetXMLGen().AddChildAttrib( _T("UpperRange"), szIP );

			if( m_IPRangeID >= nMaxID )
				nMaxID = m_IPRangeID + 1;		
		}

		GetXMLGen().OutOfElem();
	}
	else
	{
		ListGroupNames();
		ListAgentNames();
	}
	
	GetXMLGen().AddChildAttrib( _T("MaxID"), nMaxID );

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// UpdateIPRanges
// 
////////////////////////////////////////////////////////////////////////////////
int CIPRanges::UpdateIPRanges( CURLAction& action )
{
	int ID;
	int MaxID;
	tstring sType;
	CEMSString sCollection;
	TIPRanges ipr;
	CEMSString sParam;
	vector<TIPRanges> List;
	vector<TIPRanges>::iterator iter;
	set<int> DeleteIDs;
	set<int> DBIDs;
	set<int>::iterator iterDel;
	bool bFound;
	tstring sUpper;
	tstring sLower;
	bool bGotUpper, bGotLower;

	ipr.m_AgentID = 0;
	ipr.m_GroupID = 0;

	// MJM - get the agent or group ID one way or another... the front end 
	// needs to be cleaned up to use a single method
	if ( !GetISAPIData().GetXMLLong( _T("AgentID"), ipr.m_AgentID, true ) )
	{
		if ( !GetISAPIData().GetXMLLong( _T("GroupID"), ipr.m_GroupID, true ) )
		{
			GetISAPIData().GetXMLLong( _T("ID"), ID, false );
			GetISAPIData().GetXMLLong( _T("MaxID"), MaxID, false );
			GetISAPIData().GetXMLString( _T("Type"), sType, false );

			if( sType.compare( _T("Agent") ) == 0 )
			{
				ipr.m_AgentID = ID;
			}
			else if( sType.compare( _T("Group") ) == 0 )
			{
				ipr.m_GroupID = ID;
			}
			else
			{
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			}
		}
	}
	
	GetISAPIData().GetXMLString( _T("IDCollection"), sCollection );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( ipr.m_IPRangeID ) )
	{
		sParam.Format( _T("LowerRange%d"), ipr.m_IPRangeID );
		bGotLower = GetISAPIData().GetXMLString( (TCHAR*) sParam.c_str(), sLower, true );

		sParam.Format( _T("UpperRange%d"), ipr.m_IPRangeID );
		bGotUpper = GetISAPIData().GetXMLString( (TCHAR*) sParam.c_str(), sUpper, true );

		if( bGotUpper && bGotLower )
		{
			ipr.m_LowerRange = InetAddr( sLower );
			ipr.m_UpperRange = InetAddr( sUpper );
			List.push_back( ipr );
		}
	}

	// Pull all IDs from the database for this objectID
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), ipr.m_AgentID );
	BINDPARAM_LONG( GetQuery(), ipr.m_GroupID );
	BINDCOL_LONG_NOLEN( GetQuery(), ID );
	GetQuery().Execute( _T("SELECT IPRangeID ")
						_T("FROM IPRanges ")
						_T("WHERE AgentID=? AND GroupID=?") );

	while( GetQuery().Fetch() == S_OK )
	{
		DBIDs.insert( ID );
		bFound = false;
		for ( iter = List.begin(); iter != List.end(); iter++ )
		{
			if( iter->m_IPRangeID == ID )
			{
				bFound = true;
				break;
			}
		}
		if( bFound == FALSE )
		{
			// Store the IDs
			DeleteIDs.insert(ID);
		}
	}

	// Delete the ones in the database that we couldn't find in our list
	for( iterDel = DeleteIDs.begin(); iterDel != DeleteIDs.end(); iterDel++ )
	{
		ID = (*iterDel);
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), ID );
		GetQuery().Execute( _T("DELETE ")
							_T("FROM IPRanges ")
							_T("WHERE IPRangeID=?") );		
	}

	// now either update or add.
	for ( iter = List.begin(); iter != List.end(); iter++ )
	{
		if( DBIDs.find( iter->m_IPRangeID ) != DBIDs.end() 
			&& iter->m_IPRangeID < MaxID )
		{
			iter->Update( GetQuery() );
		}
		else
		{
			iter->Insert( GetQuery() );
		}
	}	
	
	return 0;
}
