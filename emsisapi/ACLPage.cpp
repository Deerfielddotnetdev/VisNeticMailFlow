// ACLPage.cpp: implementation of the CACLPage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ACLPage.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CACLPage::CACLPage(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{

}

CACLPage::~CACLPage()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
//  Run - the main entry point
// 
////////////////////////////////////////////////////////////////////////////////
int CACLPage::Run( CURLAction& action )

{
	tstring sAction;

	// Check security
	RequireAdmin();

	GetISAPIData().GetXMLString( _T("Action"), sAction, true );	

	// Get the page name (everything past the last '/')
	if( m_ISAPIData.m_sPage.compare( _T("accessrights") ) == 0 || m_ISAPIData.m_sPage.compare( _T("objectrights") ) == 0)
	{
		if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();
			return UpdateAccessRights( action );
		}

		return AccessRights( action );
	}

	return AccessControl( action );
}

////////////////////////////////////////////////////////////////////////////////
// 
// UpdateAccessRights
// 
////////////////////////////////////////////////////////////////////////////////
int CACLPage::UpdateAccessRights( CURLAction& action )
{
	int ObjectID = 0;
	int nUseDefaultRights = 0;
	CEMSString sCollection;
	CEMSString sParam;
	CEMSString sAgentOrGroup;
	int nAccessControlMaxID;
	vector<TAccessControl> ACL;
	vector<TAccessControl>::iterator iter1;
	vector<TAccessControl>::iterator iter2;
	set<int> DeleteIDs;
	set<int> DBIDs;
	set<int>::iterator iterDel;
	TAccessControl ac;
	int nAccessLevel;
	int ID;
	bool bFound;
    
	int ObjectTypeID = 0;
	int ActualID = 0;	
	
	GetISAPIData().GetXMLLong( _T("ObjectID"), ObjectID, true );
	GetISAPIData().GetXMLLong( _T("ObjectTypeID"), ObjectTypeID, true );
	GetISAPIData().GetXMLLong( _T("ID"), ActualID, true );

	if ( !ObjectID )
	{
		// first get the object information
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), ObjectTypeID );
		BINDPARAM_LONG( GetQuery(), ActualID );
		BINDCOL_LONG_NOLEN( GetQuery(), ObjectID );
		
		GetQuery().Execute( _T("SELECT Objects.ObjectID ")
							_T("FROM Objects ")
							_T("WHERE Objects.ObjectTypeID=? AND ActualID=?") );

		// must have an object ID
		if( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("Unable to obtain ObjectID") );
	}
	
	if (!GetISAPIData().GetXMLLong( _T("UseDefaultRights"), nUseDefaultRights, true ))
	{
		nUseDefaultRights = 0;
	}

	// initialize the query
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nUseDefaultRights );
	BINDPARAM_LONG( GetQuery(), ObjectID );
	GetQuery().Execute( _T("UPDATE Objects ")
						_T("SET UseDefaultRights=? ")
						_T("WHERE ObjectID=?") );

	if( GetQuery().GetRowCount() == 0 )
		THROW_EMS_EXCEPTION(E_InvalidID, _T("Unable to query object") ); 

	if( nUseDefaultRights == 0)
	{
		// Access Control Items
		GetISAPIData().GetXMLLong( _T("AccessControlMaxID"), nAccessControlMaxID );
		GetISAPIData().GetXMLString( _T("ACLCollection"), sCollection );
		sCollection.CDLInit();
		while( sCollection.CDLGetNextInt( ac.m_AccessControlID ) )
		{
			ac.m_ObjectID = ObjectID;

			sParam.Format( _T("AccessLevel%d"), ac.m_AccessControlID );
			GetISAPIData().GetXMLLong( (TCHAR*) sParam.c_str(), nAccessLevel );
			ac.m_AccessLevel = (unsigned char) nAccessLevel;

			sParam.Format( _T("AgentOrGroup%d"), ac.m_AccessControlID );
			GetISAPIData().GetXMLString( (TCHAR*) sParam.c_str(), sAgentOrGroup );

			if( sAgentOrGroup.compare( _T("Agent") ) == 0 )
			{
				// Agent Access Control Item
				sParam.Format( _T("Agent%d"), ac.m_AccessControlID );
				GetISAPIData().GetXMLLong( (TCHAR*) sParam.c_str(), ac.m_AgentID );
				ac.m_GroupID = 0;
			}
			else
			{
				// Group Access Control Item
				sParam.Format( _T("Group%d"), ac.m_AccessControlID );
				GetISAPIData().GetXMLLong( (TCHAR*) sParam.c_str(), ac.m_GroupID );
				ac.m_AgentID = 0;
			}

			ACL.push_back( ac );
		}

RemoveRedundantACLs:
		// Remove redundant ACLs.
		for( iter1 = ACL.begin(); iter1 != ACL.end(); iter1++ )
		{
			for( iter2 = ACL.begin(); iter2 != ACL.end(); iter2++ )
			{
				if( iter1 != iter2 )
				{
					if( ( iter1->m_AgentID == iter2->m_AgentID ) &&
						( iter1->m_GroupID == iter2->m_GroupID ) )
					{
						if( iter2->m_AccessLevel <= iter1->m_AccessLevel )
						{
							// Remove the item at this iterator
							ACL.erase( iter2 );

							// Start over as this invalidates the iterators
							goto RemoveRedundantACLs;
						}
					}
				}
			}
		}

		// Pull all IDs from the database for this objectID
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), ObjectID );
		BINDCOL_LONG_NOLEN( GetQuery(), ID );
		GetQuery().Execute( _T("SELECT AccessControlID ")
							_T("FROM AccessControl ")
							_T("WHERE ObjectID=?") );

		while( GetQuery().Fetch() == S_OK )
		{
			DBIDs.insert( ID );
			bFound = false;
			for ( iter1 = ACL.begin(); iter1 != ACL.end(); iter1++ )
			{
				if( iter1->m_AccessControlID == ID )
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
								_T("FROM AccessControl ")
								_T("WHERE AccessControlID=?") );		
		}

		// now either update or add.
		for ( iter1 = ACL.begin(); iter1 != ACL.end(); iter1++ )
		{
			if( DBIDs.find( iter1->m_AccessControlID ) != DBIDs.end() 
				&& iter1->m_AccessControlID < nAccessControlMaxID )
			{
				iter1->Update( GetQuery() );
			}
			else
			{
				iter1->Insert( GetQuery() );
			}
		}	
	}
	else
	{
		//We're set to usedefault rights so delete any entries in AccessControl for this ObjectID
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), ObjectID );
		GetQuery().Execute( _T("DELETE FROM AccessControl WHERE ObjectID=?") );
	}

	// invalidate the access map
	CAccessControl::GetInstance().Invalidate();

	// invalidate sessions
	GetISAPIData().m_SessionMap.RefreshAgentSession( GetSession(), GetQuery() );
	GetISAPIData().m_SessionMap.InvalidateAllAgentSessions();
	GetISAPIData().m_SessionMap.QueueSessionMonitors(1,0);
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// AccessRights
// 
////////////////////////////////////////////////////////////////////////////////
int CACLPage::AccessRights( CURLAction& action )
{
	int ObjectTypeID = 0;
	int ActualID = 0;
	TAccessControl acl;
	unsigned char UseDefaultRights;
	int AccessControlMaxID = 0;
	
	GetISAPIData().GetURLLong( _T("ObjectTypeID"), ObjectTypeID, true );
	GetISAPIData().GetURLLong( _T("ID"), ActualID, true );

	if ( ObjectTypeID > 0 )
	{
		GetXMLGen().AddChildElem( _T("AccessControlList") );
		AddObjectType( _T("ObjectType"), ObjectTypeID );

		AddObjectName( ObjectTypeID, ActualID );

		// initialize the query
		GetQuery().Initialize();
		
		// first get the object information
		BINDPARAM_LONG( GetQuery(), ObjectTypeID );
		BINDPARAM_LONG( GetQuery(), ActualID );
		BINDCOL_LONG( GetQuery(), acl.m_ObjectID );
		BINDCOL_BIT_NOLEN( GetQuery(), UseDefaultRights );
		
		GetQuery().Execute( _T("SELECT Objects.ObjectID,UseDefaultRights ")
							_T("FROM Objects INNER JOIN ObjectTypes ")
							_T("ON Objects.ObjectTypeID = ObjectTypes.ObjectTypeID ")
							_T("WHERE Objects.ObjectTypeID=? AND ActualID=?") );

		// Must have an object ID
		if( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 


		GetXMLGen().AddChildAttrib( _T("ID"), ActualID );
		GetXMLGen().AddChildAttrib( _T("ObjectID"), acl.m_ObjectID );
		GetXMLGen().AddChildAttrib( _T("UseDefaultRights"), UseDefaultRights );
		GetXMLGen().AddChildAttrib( _T("DefaultObject"), (ActualID==0) ? 1 : 0 );

		GetXMLGen().IntoElem();

		// initialize the query
		GetQuery().Reset();
		
		// first get the object information
		BINDPARAM_LONG( GetQuery(), acl.m_ObjectID );
		BINDCOL_LONG( GetQuery(), acl.m_AccessControlID );
		BINDCOL_LONG( GetQuery(), acl.m_AgentID );
		BINDCOL_LONG( GetQuery(), acl.m_GroupID );
		BINDCOL_TINYINT( GetQuery(), acl.m_AccessLevel );
		
		GetQuery().Execute( _T("SELECT AccessControlID,AgentID,AccessControl.GroupID,AccessLevel ")
							_T("FROM AccessControl INNER JOIN Groups ")
							_T("ON AccessControl.GroupID = Groups.GroupID ")
							_T("WHERE AccessControl.ObjectID=? ") 
							_T("ORDER BY GroupName") );

		// Must have an object ID
		while( GetQuery().Fetch() == S_OK )
		{
			GetXMLGen().AddChildElem( _T("AccessControlItem") );
			GetXMLGen().AddChildAttrib( _T("ID"), acl.m_AccessControlID );
			GetXMLGen().AddChildAttrib( _T("AgentID"), acl.m_AgentID );
			GetXMLGen().AddChildAttrib( _T("GroupID"), acl.m_GroupID );
			GetXMLGen().AddChildAttrib( _T("AccessLevel"), acl.m_AccessLevel );

			if( acl.m_AccessControlID >= AccessControlMaxID )
				AccessControlMaxID = acl.m_AccessControlID + 1;
		}

		GetQuery().Reset( false );
		GetQuery().Execute( _T("SELECT AccessControlID,AccessControl.AgentID,GroupID,AccessLevel ")
							_T("FROM AccessControl INNER JOIN Agents ")
							_T("ON AccessControl.AgentID = Agents.AgentID ")
							_T("WHERE AccessControl.ObjectID=? ") 
							_T("ORDER BY Name") );

		// Must have an object ID
		while( GetQuery().Fetch() == S_OK )
		{
			GetXMLGen().AddChildElem( _T("AccessControlItem") );
			GetXMLGen().AddChildAttrib( _T("ID"), acl.m_AccessControlID );
			GetXMLGen().AddChildAttrib( _T("AgentID"), acl.m_AgentID );
			GetXMLGen().AddChildAttrib( _T("GroupID"), acl.m_GroupID );
			GetXMLGen().AddChildAttrib( _T("AccessLevel"), acl.m_AccessLevel );

			if( acl.m_AccessControlID >= AccessControlMaxID )
				AccessControlMaxID = acl.m_AccessControlID + 1;
		}

		GetXMLGen().OutOfElem();
	}
	
	GetXMLGen().AddChildAttrib( _T("AccessControlMaxID"), AccessControlMaxID );

	ShowDefaultACL( ObjectTypeID );

	XObjectTypes objecttype;
	TObjects object;
	int ObjectID = 0;
	object.m_ActualID = 0;

	GetXMLGen().AddChildElem( _T("DefaultACLs") );
	GetXMLGen().IntoElem();

	// initialize the query
	GetQuery().Initialize();
	
	// first get the object information
	BINDCOL_LONG( GetQuery(), object.m_ObjectID );
	BINDCOL_LONG( GetQuery(), object.m_ObjectTypeID );
	BINDCOL_LONG( GetQuery(), acl.m_AccessControlID );
	BINDCOL_LONG( GetQuery(), acl.m_AgentID );
	BINDCOL_LONG( GetQuery(), acl.m_GroupID );
	BINDCOL_TINYINT( GetQuery(), acl.m_AccessLevel );
	
	GetQuery().Execute( _T("SELECT Objects.ObjectID,ObjectTypeID,AccessControlID,AgentID,GroupID,AccessLevel ")
						_T("FROM Objects LEFT OUTER JOIN AccessControl ")
						_T("ON Objects.ObjectID = AccessControl.ObjectID ")
						_T("WHERE Objects.ActualID=0 ")
						_T("ORDER BY ObjectTypeID") );

	while ( GetQuery().Fetch() == S_OK )
	{
		if( object.m_ObjectID != ObjectID )
		{
			if( ObjectID != 0)
				GetXMLGen().OutOfElem();

			ObjectID = object.m_ObjectID;
			GetXMLGen().AddChildElem( _T("AccessControlList") );
			AddObjectType( _T("ObjectType"), object.m_ObjectTypeID );
			AddObjectName( object.m_ObjectTypeID, object.m_ActualID );
			GetXMLGen().AddChildAttrib( _T("ID"), object.m_ActualID );
			GetXMLGen().AddChildAttrib( _T("ObjectTypeID"), object.m_ObjectTypeID );
			GetXMLGen().IntoElem();
		}

		if( acl.m_AccessControlIDLen != SQL_NULL_DATA )
		{
			GetXMLGen().AddChildElem( _T("AccessControlItem") );
			GetXMLGen().AddChildAttrib( _T("ID"), acl.m_AccessControlID );
			GetXMLGen().AddChildAttrib( _T("AgentID"), acl.m_AgentID );
			AddAgentName( _T("AgentName"), acl.m_AgentID );
			GetXMLGen().AddChildAttrib( _T("GroupID"), acl.m_GroupID );
			AddGroupName( _T("GroupName"), acl.m_GroupID );
			GetXMLGen().AddChildAttrib( _T("AccessLevel"), acl.m_AccessLevel );
		}
	}

	if( ObjectID != 0)
		GetXMLGen().OutOfElem();
	
	GetXMLGen().OutOfElem();
	
	ListAgentNames();
	ListGroupNames();
	ListTicketBoxNames( GetXMLGen() );
	ListStdRespCategories();	

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

////////////////////////////////////////////////////////////////////////////////
// 
// AccessControl
// 
////////////////////////////////////////////////////////////////////////////////
int CACLPage::AccessControl( CURLAction& action )
{
	XObjectTypes objecttype;
	TAccessControl acl;
	TObjects object;
	int ObjectID = 0;
	object.m_ActualID = 0;
	
	GetXMLGen().AddChildElem( _T("DefaultACLs") );
	GetXMLGen().IntoElem();

	// initialize the query
	GetQuery().Initialize();
	
	// first get the object information
	BINDCOL_LONG( GetQuery(), object.m_ObjectID );
	BINDCOL_LONG( GetQuery(), object.m_ObjectTypeID );
	BINDCOL_LONG( GetQuery(), acl.m_AccessControlID );
	BINDCOL_LONG( GetQuery(), acl.m_AgentID );
	BINDCOL_LONG( GetQuery(), acl.m_GroupID );
	BINDCOL_TINYINT( GetQuery(), acl.m_AccessLevel );
	
	GetQuery().Execute( _T("SELECT Objects.ObjectID,ObjectTypeID,AccessControlID,AgentID,GroupID,AccessLevel ")
						_T("FROM Objects LEFT OUTER JOIN AccessControl ")
						_T("ON Objects.ObjectID = AccessControl.ObjectID ")
						_T("WHERE Objects.ActualID=0 ")
						_T("ORDER BY ObjectTypeID") );

	while ( GetQuery().Fetch() == S_OK )
	{
		if( object.m_ObjectID != ObjectID )
		{
			if( ObjectID != 0)
				GetXMLGen().OutOfElem();

			ObjectID = object.m_ObjectID;
			GetXMLGen().AddChildElem( _T("AccessControlList") );
			AddObjectType( _T("ObjectType"), object.m_ObjectTypeID );
			AddObjectName( object.m_ObjectTypeID, object.m_ActualID );
			GetXMLGen().AddChildAttrib( _T("ID"), object.m_ActualID );
			GetXMLGen().AddChildAttrib( _T("ObjectTypeID"), object.m_ObjectTypeID );
			GetXMLGen().IntoElem();
		}

		if( acl.m_AccessControlIDLen != SQL_NULL_DATA )
		{
			GetXMLGen().AddChildElem( _T("AccessControlItem") );
			GetXMLGen().AddChildAttrib( _T("ID"), acl.m_AccessControlID );
			GetXMLGen().AddChildAttrib( _T("AgentID"), acl.m_AgentID );
			AddAgentName( _T("AgentName"), acl.m_AgentID );
			GetXMLGen().AddChildAttrib( _T("GroupID"), acl.m_GroupID );
			AddGroupName( _T("GroupName"), acl.m_GroupID );
			GetXMLGen().AddChildAttrib( _T("AccessLevel"), acl.m_AccessLevel );
		}
	}

	if( ObjectID != 0 )
		GetXMLGen().OutOfElem();

	GetXMLGen().OutOfElem();


	ObjectID = 0;

	GetXMLGen().AddChildElem( _T("CustomACL") );
	GetXMLGen().IntoElem();

	// initialize the query
	GetQuery().Initialize();
	
	// first get the object information
	BINDCOL_LONG( GetQuery(), object.m_ObjectID );
	BINDCOL_LONG( GetQuery(), object.m_ActualID );
	BINDCOL_LONG( GetQuery(), object.m_ObjectTypeID );
	BINDCOL_LONG( GetQuery(), acl.m_AccessControlID );
	BINDCOL_LONG( GetQuery(), acl.m_AgentID );
	BINDCOL_LONG( GetQuery(), acl.m_GroupID );
	BINDCOL_TINYINT( GetQuery(), acl.m_AccessLevel );
	
	GetQuery().Execute( _T("SELECT Objects.ObjectID,ActualID,ObjectTypeID,AccessControlID,AgentID,GroupID,AccessLevel ")
						_T("FROM Objects LEFT OUTER JOIN AccessControl ")
						_T("ON Objects.ObjectID = AccessControl.ObjectID ")
						_T("WHERE UseDefaultRights=0 AND ActualID <> 0 ")
						_T("ORDER BY Objects.ObjectID") );

	while ( GetQuery().Fetch() == S_OK )
	{
		if( object.m_ObjectID != ObjectID )
		{
			if( ObjectID != 0)
				GetXMLGen().OutOfElem();

			ObjectID = object.m_ObjectID;
			GetXMLGen().AddChildElem( _T("AccessControlList") );
			AddObjectType( _T("ObjectType"), object.m_ObjectTypeID );
			AddObjectName( object.m_ObjectTypeID, object.m_ActualID );

			GetXMLGen().AddChildAttrib( _T("ID"), object.m_ActualID );
			GetXMLGen().AddChildAttrib( _T("ObjectTypeID"), object.m_ObjectTypeID );
			GetXMLGen().IntoElem();
		}

		if( acl.m_AccessControlIDLen != SQL_NULL_DATA )
		{
			GetXMLGen().AddChildElem( _T("AccessControlItem") );
			GetXMLGen().AddChildAttrib( _T("ID"), acl.m_AccessControlID );
			GetXMLGen().AddChildAttrib( _T("AgentID"), acl.m_AgentID );
			AddAgentName( _T("AgentName"), acl.m_AgentID );
			GetXMLGen().AddChildAttrib( _T("GroupID"), acl.m_GroupID );
			AddGroupName( _T("GroupName"), acl.m_GroupID );
			GetXMLGen().AddChildAttrib( _T("AccessLevel"), acl.m_AccessLevel );
		}
	}

	if( ObjectID != 0 )
		GetXMLGen().OutOfElem();

	GetXMLGen().OutOfElem();

	ListAgentNames();
	ListTicketBoxNames( GetXMLGen() );
	ListGroupNames();
	ListStdRespCategories();
	
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

////////////////////////////////////////////////////////////////////////////////
// 
// AddObjectName
// 
////////////////////////////////////////////////////////////////////////////////
void CACLPage::AddObjectName( int ObjectTypeID, int& ActualID )
{
	switch( ObjectTypeID )
	{
	case EMS_OBJECT_TYPE_AGENT:
		if( ActualID == 0 )
		{
			GetXMLGen().AddChildAttrib( _T("Name"), _T("Agents") );
		}
		else
		{
			AddAgentName( _T("Name"), ActualID );
		}
			
		break;
		
	case EMS_OBJECT_TYPE_TICKET_BOX:
		if( ActualID == 0 )
		{
			GetXMLGen().AddChildAttrib( _T("Name"), _T("TicketBoxes") );
		}
		else		
		{
			AddTicketBoxName( _T("Name"), ActualID );
		}
		break;
				
	case EMS_OBJECT_TYPE_TICKET_CATEGORY:
		if( ActualID == 0 )
		{
			GetXMLGen().AddChildAttrib( _T("Name"), _T("Ticket Categories") );
		}
		else		
		{
			AddTicketCategoryName( _T("Name"), ActualID );
		}
		break;
				
	case EMS_OBJECT_TYPE_GROUP:
		if( ActualID == 0 )
		{
			GetXMLGen().AddChildAttrib( _T("Name"), _T("Groups") );
		}
		else		
		{
			AddGroupName( _T("Name"), ActualID );
		}
		break;
		
	case EMS_OBJECT_TYPE_STD_RESP:
		if( ActualID == 0 )
		{
			GetXMLGen().AddChildAttrib( _T("Name"), _T("Standard Response Categories") );
		}
		else		
		{
			AddStdRespCategory( _T("Name"), ActualID );
		}
		break;

	case EMS_OBJECT_TYPE_CONTACT:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Contacts") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_STANDARD_REPORTS:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Standard Reports") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_SCHEDULED_REPORTS:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Scheduled Reports") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_AUTO_TICKET_OPEN:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Auto Ticket Open") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_AGENT_AUDIT:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Agent Audit Reports") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_VOIP:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Delete Drafts") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_AGENT_PREFERENCES:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Agent Preferences") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_PREFERENCES_GENERAL:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Preferences General") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_PREFERENCES_TICKETBOX:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Preferences TicketBoxes") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_PREFERENCES_AGENT_VIEWS:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Preferences Agent Views") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_PREFERENCES_SIGNATURES:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Preferences Signatures") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_PREFERENCES_MY_DICTIONARY:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Preferences My Dictionary") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_PREFERENCES_MY_FOLDERS:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Preferences My Folders") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_PREFERENCES_MY_ALERTS:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Preferences My Alerts") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_PREFERENCES_MY_CONTACTS:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Preferences My Contacts") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_ROUTE_REPLIES:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Route Replies To Me") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_CLOSE_TICKET:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Close Ticket After Send") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_REQUEST_READ:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Request Read Receipt") );
		ActualID = 0;
		break;

	case EMS_OBJECT_TYPE_REQUEST_DELIVERY:
		GetXMLGen().AddChildAttrib( _T("Name"), _T("Request Delivery Confirmation") );
		ActualID = 0;
		break;

	default:
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// ShowDefaultACL
// 
////////////////////////////////////////////////////////////////////////////////
void CACLPage::ShowDefaultACL( int ObjectTypeID )
{
	XObjectTypes objecttype;
	TAccessControl acl;
	TObjects object;
	object.m_ActualID = 0;

	GetXMLGen().AddChildElem( _T("DefaultACL") );
	GetXMLGen().IntoElem();

	// initialize the query
	GetQuery().Initialize();
	
	// first get the object information
	BINDPARAM_LONG( GetQuery(), ObjectTypeID );
	BINDCOL_LONG( GetQuery(), object.m_ObjectID );
	BINDCOL_LONG( GetQuery(), object.m_ObjectTypeID );
	BINDCOL_LONG( GetQuery(), acl.m_AccessControlID );
	BINDCOL_LONG( GetQuery(), acl.m_AgentID );
	BINDCOL_LONG( GetQuery(), acl.m_GroupID );
	BINDCOL_TINYINT( GetQuery(), acl.m_AccessLevel );
	
	GetQuery().Execute( _T("SELECT Objects.ObjectID,ObjectTypeID,AccessControlID,AgentID,GroupID,AccessLevel ")
						_T("FROM Objects LEFT OUTER JOIN AccessControl ")
						_T("ON Objects.ObjectID = AccessControl.ObjectID ")
						_T("WHERE Objects.ActualID=0 ")
						_T("AND ObjectTypeID=?") );

	while ( GetQuery().Fetch() == S_OK )
	{
		if( acl.m_AccessControlIDLen != SQL_NULL_DATA )
		{
			GetXMLGen().AddChildElem( _T("AccessControlItem") );
			GetXMLGen().AddChildAttrib( _T("ID"), acl.m_AccessControlID );
			GetXMLGen().AddChildAttrib( _T("AgentID"), acl.m_AgentID );
			AddAgentName( _T("AgentName"), acl.m_AgentID );
			GetXMLGen().AddChildAttrib( _T("GroupID"), acl.m_GroupID );
			AddGroupName( _T("GroupName"), acl.m_GroupID );
			GetXMLGen().AddChildAttrib( _T("AccessLevel"), acl.m_AccessLevel );
		}
	}

	GetXMLGen().OutOfElem();
}
