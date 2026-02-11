// Groups.cpp: implementation of the CGroups class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Groups.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGroups::CGroups(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CGroups::~CGroups()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
int CGroups::Run(CURLAction& action)
{
	tstring sAction = _T("list");

	// Check security
	RequireAdmin();

	GetISAPIData().GetXMLString( _T("Action"), sAction, true );
	GetISAPIData().GetURLLong( _T("ID"), m_GroupID, true );

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("editgroup") ) == 0)
	{
		if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();
			DecodeForm();
			DoUpdate( action );
			return 0;
		}
		else if( sAction.compare( _T("insert") ) == 0 )
		{
			DISABLE_IN_DEMO();
			DecodeForm();
			DoInsert( action );
			return 0;
		}
		else
		{
			QueryOne( action );
			GenerateXML();	
			
			if( m_GroupID == 0 )
			{
				// Change title if ID is zero
				action.m_sPageTitle.assign( _T("New Group") );
			}
			return 0;
		}
	}
	else if ( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();
		DoDelete( action );
	}

	ListAll( action );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// ListAll
// 
////////////////////////////////////////////////////////////////////////////////
void CGroups::ListAll( CURLAction& action )
{
	int GroupID;
	int AgentID;
	map<int,tstring> AGMap;
	XAgentNames agentname;

	// First, get the membership information and save it in a map
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), GroupID );
	BINDCOL_LONG_NOLEN( GetQuery(), AgentID );
	GetQuery().Execute( _T("SELECT AG.GroupID,AG.AgentID ")
						_T("FROM AgentGroupings AS AG ") 
						_T("INNER JOIN Groups AS G ON AG.GroupID=G.GroupID ")
						_T("INNER JOIN Agents AS A ON AG.AgentID=A.AgentID ")
						_T("WHERE G.IsDeleted=0 AND A.IsDeleted=0 ")
						_T("ORDER BY G.GroupName,A.Name ") );

	while( GetQuery().Fetch() == S_OK )
	{
		// Get the agent name from the cache
		if( GetXMLCache().m_AgentNames.Query( AgentID, agentname ) ) 
		{
			if( AGMap.find( GroupID ) == AGMap.end() )
			{
				AGMap[GroupID] = (tstring) agentname.m_Name;
			}
			else
			{
				AGMap[GroupID].append( _T(", ") );
				AGMap[GroupID].append( agentname.m_Name );
			}
		}
	}

	// Now query for the groups
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_GroupID );
	BINDCOL_TCHAR( GetQuery(), m_GroupName );
	BINDCOL_BIT( GetQuery(), m_IsEscGroup );
	GetQuery().Execute( _T("SELECT GroupID,GroupName,IsEscalationGroup ")
						_T("FROM Groups ")
						_T("WHERE IsDeleted=0 ")
						_T("ORDER BY GroupName") );

	GetXMLGen().AddChildElem( _T("Groups") );
	GetXMLGen().IntoElem();
	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Group") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_GroupID );
		GetXMLGen().AddChildAttrib( _T("Name"), m_GroupName );
		GetXMLGen().AddChildAttrib( _T("isEscGroup"), m_IsEscGroup );
		GetXMLGen().AddChildAttrib( _T("BuiltIn"), m_GroupID <= EMS_GROUPID_ADMINISTRATORS ? 1 : 0 );

		if( AGMap.find( m_GroupID ) == AGMap.end() )
		{
			GetXMLGen().AddChildAttrib( _T("Members"), _T("") );
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("Members"), AGMap[m_GroupID].c_str() );
		}
	}
	GetXMLGen().OutOfElem();

}

////////////////////////////////////////////////////////////////////////////////
// 
// QueryOne
// 
////////////////////////////////////////////////////////////////////////////////
void CGroups::QueryOne( CURLAction& action )
{
	TSignatures sig;
	int AgentID;

	if( m_GroupID == 0 )
		return;
	
	GetQuery().Initialize();
	BINDCOL_TCHAR( GetQuery(), m_GroupName );
	BINDPARAM_LONG( GetQuery(), m_GroupID );
	BINDCOL_BIT( GetQuery(), m_IsEscGroup );
	BINDCOL_BIT( GetQuery(), m_UseEscTicketBox );
	BINDCOL_LONG( GetQuery(), m_AssignToTicketBoxID );
	GetQuery().Execute( _T("SELECT GroupName,IsEscalationGroup,UseEscTicketBox,AssignToTicketBoxID ")
						_T("FROM Groups ")
						_T("WHERE GroupID=?") );

	if( GetQuery().Fetch() != S_OK )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
	}

	// First, get the membership information and save it in a map
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_GroupID );
	BINDCOL_LONG_NOLEN( GetQuery(), AgentID );
	GetQuery().Execute( _T("SELECT AG.AgentID ")
						_T("FROM AgentGroupings AS AG ") 
						_T("INNER JOIN Agents AS A ON AG.AgentID=A.AgentID ")
						_T("WHERE AG.GroupID=? AND A.IsDeleted=0 ")
						_T("ORDER BY A.Name ") );

	while( GetQuery().Fetch() == S_OK )
	{
		m_IDs.push_back( AgentID );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoUpdate
// 
////////////////////////////////////////////////////////////////////////////////
void CGroups::DoUpdate( CURLAction& action )
{
	int AgentID;
	set<int> m_DBIDs;
	set<int>::iterator DBiter;
	list<int>::iterator iter;

	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_GroupID );
	BINDPARAM_TCHAR( GetQuery(), m_GroupName );
	GetQuery().Execute( _T("SELECT GroupID FROM Groups ")
						_T("WHERE GroupID<>? AND GroupName=? ") );

	if( GetQuery().Fetch() == S_OK )
	{
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Group name must be unique")  );
	}

	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_GroupName );
	BINDPARAM_BIT( GetQuery(), m_IsEscGroup );
	BINDPARAM_BIT( GetQuery(), m_UseEscTicketBox );
	BINDPARAM_LONG( GetQuery(), m_AssignToTicketBoxID );
	BINDPARAM_LONG( GetQuery(), m_GroupID );
	GetQuery().Execute( _T("UPDATE Groups ")
						_T("SET GroupName=?,UseEscTicketBox=?,IsEscalationGroup=?,AssignToTicketBoxID=? ")
						_T("WHERE GroupID=?") );

	if( GetQuery().GetRowCount() == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
	}

	if( m_GroupID > EMS_GROUPID_EVERYONE )
	{
		// First, get the membership information and save it in a map
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_GroupID );
		BINDCOL_LONG_NOLEN( GetQuery(), AgentID );
		GetQuery().Execute( _T("SELECT AG.AgentID ")
							_T("FROM AgentGroupings AS AG ") 
							_T("INNER JOIN Agents AS A ON AG.AgentID=A.AgentID ")
							_T("WHERE AG.GroupID=? AND A.IsDeleted=0 ") );

		while( GetQuery().Fetch() == S_OK )
		{
			m_DBIDs.insert( AgentID );
		}

		for( iter = m_IDs.begin(); iter != m_IDs.end(); iter++ )
		{
			if( m_DBIDs.find(*iter) == m_DBIDs.end() )
			{
				AgentID = *iter;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_GroupID );
				BINDPARAM_LONG( GetQuery(), AgentID );
				GetQuery().Execute( _T("INSERT INTO AgentGroupings ")
									_T("(GroupID,AgentID) ") 
									_T("VALUES ")
									_T("(?,?)") );
			}
		}

		for( DBiter = m_DBIDs.begin(); DBiter != m_DBIDs.end(); DBiter++ )
		{
			bool bFound = false;

			if( *DBiter == EMS_AGENTID_ADMINISTRATOR && m_GroupID == EMS_GROUPID_ADMINISTRATORS )
			{
				bFound = true;
			}
			else
			{
				for( iter = m_IDs.begin(); iter != m_IDs.end(); iter++ )
				{
					if( *DBiter == *iter )
					{
						bFound = true;
						break;
					}
				}
			}
			if( bFound == false )
			{
				AgentID = *DBiter;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_GroupID );
				BINDPARAM_LONG( GetQuery(), AgentID );
				GetQuery().Execute( _T("DELETE FROM AgentGroupings ")
									_T("WHERE GroupID=? and AgentID=? ") );
			}
		}
	}

	// invalidate group cache
	InvalidateGroups();

	// invalidate access control cache
	CAccessControl::GetInstance().Invalidate();
	
	// invalidate sessions
	GetISAPIData().m_SessionMap.RefreshAgentSession( GetSession(), GetQuery() );
	GetISAPIData().m_SessionMap.InvalidateAllAgentSessions();
	GetISAPIData().m_SessionMap.QueueSessionMonitors(1,0);
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoInsert
// 
////////////////////////////////////////////////////////////////////////////////
void CGroups::DoInsert( CURLAction& action )
{
	int AgentID;
	list<int>::iterator iter;
	int ObjectTypeID = EMS_OBJECT_TYPE_GROUP;
	TIMESTAMP_STRUCT now;


	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_GroupName );
	GetQuery().Execute( _T("SELECT GroupID FROM Groups ")
						_T("WHERE GroupName=? ") );

	if( GetQuery().Fetch() == S_OK )
	{
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Group name must be unique")  );
	}

	GetTimeStamp( now );

	// Create the object ID
	GetQuery().Initialize();
	BINDPARAM_LONG(GetQuery(), ObjectTypeID);
	GetQuery().Execute( _T("INSERT INTO Objects (ObjectTypeID) VALUES (?)") );
	m_ObjectID = GetQuery().GetLastInsertedID();

	// Create the new group as deleted
	m_IsDeleted = 1;

	TGroups::Insert( GetQuery() );

	// Update the object row with the actual ID
	GetQuery().Reset( true );

	BINDPARAM_LONG( GetQuery(), m_GroupID );
	BINDPARAM_TIME_NOLEN( GetQuery(), now );
	BINDPARAM_LONG( GetQuery(), m_ObjectID );

	GetQuery().Execute( _T("UPDATE Objects SET ")
						_T("ActualID = ?, ")
						_T("DateCreated = ? ")
						_T("WHERE ObjectID = ?") );

	// Add the members
	for( iter = m_IDs.begin(); iter != m_IDs.end(); iter++ )
	{
		AgentID = *iter;
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_GroupID );
		BINDPARAM_LONG( GetQuery(), AgentID );
		GetQuery().Execute( _T("INSERT INTO AgentGroupings ")
							_T("(GroupID,AgentID) ") 
							_T("VALUES ")
							_T("(?,?)") );
	}

	// commit the group
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_GroupID );
	GetQuery().Execute( _T("UPDATE Groups ")
						_T("SET IsDeleted=0 ")
						_T("WHERE GroupID=?") );

	InvalidateGroups();	
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoDelete
// 
////////////////////////////////////////////////////////////////////////////////
void CGroups::DoDelete( CURLAction& action )
{
	int ObjectID;

	GetISAPIData().GetXMLLong( _T("SelectID"), m_GroupID, true );
	
	if( m_GroupID <= EMS_GROUPID_ADMINISTRATORS )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}
	
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_GroupID );
	BINDCOL_LONG_NOLEN( GetQuery(), ObjectID );
	GetQuery().Execute( _T("SELECT ObjectID ")
		                _T("FROM Groups ")
						_T("WHERE GroupID=?") );

	if( GetQuery().Fetch() != S_OK )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}

	// commit the group
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_GroupID );
	GetQuery().Execute( _T("DELETE FROM AgentGroupings ")
						_T("WHERE GroupID=?") );

	GetQuery().Reset(false);
	GetQuery().Execute( _T("DELETE FROM Alerts ")
						_T("WHERE GroupID=?") );

	GetQuery().Reset(false);
	GetQuery().Execute( _T("DELETE FROM IPRanges ")
						_T("WHERE GroupID=?") );
	
	GetQuery().Reset(false);
	GetQuery().Execute( _T("DELETE FROM Signatures ")
						_T("WHERE GroupID=?") );

	// Delete the group
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_GroupID );
	GetQuery().Execute( _T("DELETE FROM Groups ")
						_T("WHERE GroupID=?") );
	
	// Delete any access control records
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), ObjectID );
	GetQuery().Execute( _T("DELETE FROM AccessControl ")
						_T("WHERE ObjectID=?") );	

	// Delete the object
	GetQuery().Reset(false);
	GetQuery().Execute( _T("DELETE FROM Objects ")
						_T("WHERE ObjectID=?") );

	// invalidate group cache
	InvalidateGroups();

	// invalidate access control cache
	CAccessControl::GetInstance().Invalidate();
	
	// invalidate sessions
	GetISAPIData().m_SessionMap.RefreshAgentSession( GetSession(), GetQuery() );
	GetISAPIData().m_SessionMap.InvalidateAllAgentSessions();
	GetISAPIData().m_SessionMap.QueueSessionMonitors(1,0);	
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecodeForm
// 
////////////////////////////////////////////////////////////////////////////////
void CGroups::DecodeForm( void )
{
	CEMSString sCollection;
	int AgentID;

	GetISAPIData().GetXMLTCHAR( _T("GroupName"), m_GroupName, 51 );
	GetISAPIData().GetXMLString( _T("AgentIDCollection"), sCollection, false );
	GetISAPIData().GetXMLLong( _T("AssignToTicketBoxId"), m_AssignToTicketBoxID, true );
	GetISAPIData().GetXMLLong( _T("isEscGroup"), m_IsEscGroup );
	GetISAPIData().GetXMLLong( _T("useEscTicketBox"), m_UseEscTicketBox );
	
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( AgentID ) )
	{
		m_IDs.push_back( AgentID );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GenerateXML
// 
////////////////////////////////////////////////////////////////////////////////
void CGroups::GenerateXML( void )
{
	list<int>::iterator iter;

	GetXMLGen().AddChildElem( _T("Group") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_GroupID );
	GetXMLGen().AddChildAttrib( _T("Name"), m_GroupName );
	GetXMLGen().AddChildAttrib( _T("isEscGroup"), m_IsEscGroup );
	GetXMLGen().AddChildAttrib( _T("useEscTicketBox"), m_UseEscTicketBox );
	GetXMLGen().AddChildAttrib( _T("AssignToTicketBoxId"), m_AssignToTicketBoxID );
	GetXMLGen().IntoElem();

	for( iter = m_IDs.begin(); iter != m_IDs.end(); iter++ )
	{
		GetXMLGen().AddChildElem( _T("Agent") );
		GetXMLGen().AddChildAttrib( _T("ID"), *iter );
		AddAgentName( _T("Name"), *iter );
	}

	GetXMLGen().OutOfElem();

	ListAgentNames();
	ListTicketBoxNames( GetXMLGen() );
}