/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/AccessControl.cpp,v 1.2.2.1.2.1 2006/07/18 12:55:02 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Global Access Control Object   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "AccessControl.h"
#include "SecurityFns.h"

CAccessControlMutex m_ACMutex;

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a reference to the staic class object.  This
||				is the only way to reference the class.	              
\*--------------------------------------------------------------------------*/
CAccessControl& CAccessControl::GetInstance()
{
	static CAccessControl singleton;
    return singleton;
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Clears the Access Control map	              
\*--------------------------------------------------------------------------*/
void CAccessControl::Invalidate()
{
	if(m_ACMutex.AcquireLock(1000))
	{
		m_AccessMap.clear();
		m_ACMutex.ReleaseLock();					
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a right level given an AgentID, ObjectTypeID 
||				and ActualID.  The results are cached in an internal map.	              
\*--------------------------------------------------------------------------*/
unsigned char CAccessControl::GetAgentRightLevel( CODBCQuery& query, int nAgentID, int nObjectTypeID, int nActualID )
{
	if ( IsAgentAdmin( query, nAgentID ) ) return EMS_DELETE_ACCESS;
	
	unsigned char RightLevel = EMS_NO_ACCESS;
	AccessControlKey MapKey(nAgentID, nObjectTypeID, nActualID);

	// lock the access control map
	if(m_ACMutex.AcquireLock(1000))
	{
		// attempt to find the access level for the combination in the map
		AccessMap::iterator iter = m_AccessMap.find( MapKey );
		
		if( iter != m_AccessMap.end() )
		{
			RightLevel = iter->second;
		}
		else if (nObjectTypeID == EMS_OBJECT_TYPE_TICKET_BOX)
		{
			AddTicketBoxesToMap( query, nAgentID );
			
			iter = m_AccessMap.find( MapKey );
			
			if( iter != m_AccessMap.end() )
			{
				RightLevel = iter->second;
			}
			else
			{
				// if the ticket box was not found.. return the default access level
				AccessControlKey MapKey(nAgentID, EMS_OBJECT_TYPE_TICKET_BOX, 0);
				iter = m_AccessMap.find( MapKey );
				
				if( iter != m_AccessMap.end() )
					RightLevel = iter->second;
			}
		}
		else if (nObjectTypeID == EMS_OBJECT_TYPE_TICKET_CATEGORY)
		{
			AddTicketCategoriesToMap( query, nAgentID );
			
			iter = m_AccessMap.find( MapKey );
			
			if( iter != m_AccessMap.end() )
			{
				RightLevel = iter->second;
			}
			else
			{
				// if the ticket category was not found.. return the default access level
				AccessControlKey MapKey(nAgentID, EMS_OBJECT_TYPE_TICKET_CATEGORY, 0);
				iter = m_AccessMap.find( MapKey );
				
				if( iter != m_AccessMap.end() )
					RightLevel = iter->second;
			}
		}
		else if (nObjectTypeID == EMS_OBJECT_TYPE_AGENT)
		{
			AddAgentsToMap( query, nAgentID );
			
			iter = m_AccessMap.find( MapKey );
			
			if( iter != m_AccessMap.end() )
			{
				RightLevel = iter->second;	
			}
			else
			{
				// if the agent was not found.. return the default access level
				AccessControlKey MapKey(nAgentID, EMS_OBJECT_TYPE_AGENT, 0);
				iter = m_AccessMap.find( MapKey );

				if( iter != m_AccessMap.end() )
					RightLevel = iter->second;
			}
		}
		else if (nObjectTypeID == EMS_OBJECT_TYPE_STD_RESP)
		{
			AddSRCategoriesToMap( query, nAgentID );
			
			iter = m_AccessMap.find( MapKey );
			
			if( iter != m_AccessMap.end() )
			{
				RightLevel = iter->second;	
			}
			else
			{
				// if the agent was not found.. return the default access level
				AccessControlKey MapKey(nAgentID, EMS_OBJECT_TYPE_STD_RESP, 0);
				iter = m_AccessMap.find( MapKey );

				if( iter != m_AccessMap.end() )
					RightLevel = iter->second;
			}
		}
		else
		{
			RightLevel = QueryRightLevel( query,MapKey );
		}
		m_ACMutex.ReleaseLock();					
	}
	
	return RightLevel;	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Queries the database for the access right of the object
||				specifyed by MapKey and adds the result to the security map.	              
\*--------------------------------------------------------------------------*/
unsigned char CAccessControl::QueryRightLevel( CODBCQuery& query, AccessControlKey MapKey )
{
	
	unsigned char AgentAccessLevel = EMS_NO_ACCESS;
	long AgentAccessLevelLen;
	unsigned char UseDefaultRights = FALSE;
	int nObjectID;
	
	// initialize the query
	query.Initialize();

	{
		/*dca::String f;
		f.Format("CAccessControl::QueryRightLevel - get the object information. ObjectTypeId = %d, ActualID = %d", MapKey.m_nObjectTypeID, MapKey.m_nActualID);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
	}
	
	// first get the object information
	BINDPARAM_LONG( query, MapKey.m_nObjectTypeID );
	BINDPARAM_LONG( query, MapKey.m_nActualID );
	BINDCOL_LONG_NOLEN( query, nObjectID );
	BINDCOL_BIT_NOLEN( query, UseDefaultRights );
	
	// are we supposed to use default rights
	query.Execute( _T("SELECT ObjectID, UseDefaultRights ")
		_T("FROM Objects INNER JOIN ObjectTypes ON Objects.ObjectTypeID = ObjectTypes.ObjectTypeID ")
		_T("WHERE Objects.ObjectTypeID = ? AND ActualID = ?") );
	
	// object not found, return no access don't add this to the map
	if( query.Fetch() != S_OK )
	{
		//DebugReporter::Instance().DisplayMessage("CAccessControl::QueryRightLevel - object not found, return no access don't add this to the map", DebugReporter::ISAPI, GetCurrentThreadId());
		return EMS_NO_ACCESS;
	}
	
	// initialize the query
	query.Initialize();

	{
		/*dca::String f;
		f.Format("CAccessControl::QueryRightLevel - use default = %d, objectid = %d", UseDefaultRights, nObjectID);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
	}

	// if we are supposed to use the default rights 
	if( UseDefaultRights )
	{	
		BINDPARAM_LONG( query, MapKey.m_nObjectTypeID );
		BINDPARAM_LONG( query, MapKey.m_nAgentID );
		BINDPARAM_LONG( query, MapKey.m_nAgentID );
		BINDCOL_TINYINT( query, AgentAccessLevel );


		if ( MapKey.m_nObjectTypeID < 6 )
		{
			// query for the default rights
			query.Execute( _T("SELECT MAX(AccessControl.AccessLevel) ")
				_T("FROM AccessControl LEFT OUTER JOIN AgentGroupings ON AccessControl.GroupID = AgentGroupings.GroupID ")
				_T("WHERE AccessControl.ObjectID = ? AND (AccessControl.AgentID=? OR AgentGroupings.AgentID = ?)") );
		}
		else
		{
			// query for the default rights
			query.Execute( _T("SELECT MAX(AccessControl.AccessLevel) ")
				_T("FROM AccessControl LEFT OUTER JOIN AgentGroupings ON AccessControl.GroupID = AgentGroupings.GroupID ")
				_T("INNER JOIN Objects ON Objects.ObjectID=AccessControl.ObjectID ")
				_T("WHERE Objects.ObjectTypeID = ? AND Objects.ActualID=0 AND Objects.BuiltIn=1 AND (AccessControl.AgentID=? OR AgentGroupings.AgentID = ?)") );
		}
	}
	else
	{
		// this object does not use the default rights 
		BINDPARAM_LONG( query, nObjectID );
		BINDPARAM_LONG( query, MapKey.m_nAgentID );
		BINDPARAM_LONG( query, MapKey.m_nAgentID );
		BINDCOL_TINYINT( query, AgentAccessLevel );
		
		// query for the object rights
		query.Execute( _T("SELECT MAX(AccessControl.AccessLevel) ")
			_T("FROM AccessControl LEFT OUTER JOIN AgentGroupings ON AccessControl.GroupID = AgentGroupings.GroupID ")
			_T("WHERE AccessControl.ObjectID = ? AND (AccessControl.AgentID=? OR AgentGroupings.AgentID = ?)") );
	}

	// fetch the access level
	query.Fetch();

	{
		/*dca::String f;
		f.Format("CAccessControl::QueryRightLevel - accesslevel = %d, agentid = %d", AgentAccessLevel, MapKey.m_nAgentID);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
	}
	
	// add it to the map
	m_AccessMap[MapKey] = AgentAccessLevel;

	// return the access level
	return AgentAccessLevel;	
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Adds all ticketboxes to the security map for nAgentID	              
\*--------------------------------------------------------------------------*/
void CAccessControl::AddTicketBoxesToMap(CODBCQuery& query, int nAgentID)
{
	
	AccessControlKey MapKey(nAgentID, EMS_OBJECT_TYPE_TICKET_BOX);
	unsigned char UseDefaultRights;
	unsigned char DefaultAccessLevel = EMS_NO_ACCESS;
	long DefaultAccessLevelLen;
	unsigned char AccessLevel = EMS_NO_ACCESS;
	long AccessLevelLen;
	
	// get the default access level
	query.Initialize();

	BINDPARAM_LONG( query, MapKey.m_nObjectTypeID );
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );

	BINDCOL_TINYINT( query, DefaultAccessLevel );
	
	query.Execute(  _T("SELECT MAX(AccessLevel) FROM AccessControl WHERE AccessControl.ObjectID = ? ")
					_T("AND (AccessControl.AgentID=? OR GroupID IN ")
					_T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=?))") );

	query.Fetch();
	
	// add the default to the map
	m_AccessMap[MapKey] = DefaultAccessLevel;
	
	// query for all the ticketboxes
	// for which the agent has access
	query.Initialize();
	
	BINDCOL_LONG_NOLEN( query, MapKey.m_nActualID );
	BINDCOL_BIT_NOLEN( query, UseDefaultRights );
	BINDCOL_TINYINT( query, AccessLevel );
	
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, MapKey.m_nObjectTypeID );
	
	query.Execute(	_T("SELECT TicketBoxID, UseDefaultRights, (SELECT MAX(AccessLevel) ")
					_T("FROM AccessControl WHERE ObjectID=Objects.ObjectID AND (AgentID=? ")
					_T("OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=?))) ")
					_T("FROM TicketBoxes ") 
					_T("INNER JOIN Objects ON Objects.ObjectID = TicketBoxes.ObjectID ")
					_T("WHERE Objects.ObjectTypeID=? ") );

	
	while (query.Fetch() == S_OK)
	{
		if (UseDefaultRights)
		{
			m_AccessMap[MapKey] = DefaultAccessLevel;
		}
		else
		{
			m_AccessMap[MapKey] = AccessLevel;
		}

		AccessLevel = EMS_NO_ACCESS;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds all ticketbox categories to the security map for nAgentID	              
\*--------------------------------------------------------------------------*/
void CAccessControl::AddTicketCategoriesToMap(CODBCQuery& query, int nAgentID)
{
	
	AccessControlKey MapKey(nAgentID, EMS_OBJECT_TYPE_TICKET_CATEGORY);
	unsigned char UseDefaultRights;
	unsigned char DefaultAccessLevel = EMS_NO_ACCESS;
	long DefaultAccessLevelLen;
	unsigned char AccessLevel = EMS_NO_ACCESS;
	long AccessLevelLen;
	
	// get the default access level
	query.Initialize();

	BINDPARAM_LONG( query, MapKey.m_nObjectTypeID );
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );

	BINDCOL_TINYINT( query, DefaultAccessLevel );
	
	query.Execute(  _T("SELECT MAX(AccessLevel) FROM AccessControl INNER JOIN Objects ON Objects.ObjectID=AccessControl.ObjectID WHERE Objects.ObjectTypeID=? AND Objects.BuiltIn=1 AND Objects.ActualID=0 ")
					_T("AND (AccessControl.AgentID=? OR GroupID IN ")
					_T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=?))") );
	query.Fetch();
	
	// add the default to the map
	m_AccessMap[MapKey] = DefaultAccessLevel;
	
	// query for all the ticket categories
	// for which the agent has access
	query.Initialize();
	
	BINDCOL_LONG_NOLEN( query, MapKey.m_nActualID );
	BINDCOL_BIT_NOLEN( query, UseDefaultRights );
	BINDCOL_TINYINT( query, AccessLevel );
	
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, MapKey.m_nObjectTypeID );
	
	query.Execute(	_T("SELECT TicketCategoryID, UseDefaultRights, (SELECT MAX(AccessLevel) ")
					_T("FROM AccessControl WHERE ObjectID=Objects.ObjectID AND (AgentID=? ")
					_T("OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=?))) ")
					_T("FROM TicketCategories ") 
					_T("INNER JOIN Objects ON Objects.ObjectID = TicketCategories.ObjectID ")
					_T("WHERE Objects.ObjectTypeID=? ") );

	
	while (query.Fetch() == S_OK)
	{
		if (UseDefaultRights)
		{
			m_AccessMap[MapKey] = DefaultAccessLevel;
		}
		else
		{
			m_AccessMap[MapKey] = AccessLevel;
		}

		AccessLevel = EMS_NO_ACCESS;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds all agents to the security map for nAgentID	              
\*--------------------------------------------------------------------------*/
void CAccessControl::AddAgentsToMap(CODBCQuery& query, int nAgentID )
{	

	AccessControlKey MapKey(nAgentID, EMS_OBJECT_TYPE_AGENT);

	unsigned char UseDefaultRights = FALSE;
	unsigned char DefaultAccessLevel = EMS_NO_ACCESS;
	long DefaultAccessLevelLen;
	unsigned char AccessLevel = EMS_NO_ACCESS;
	long AccessLevelLen;

	// get the default access level
	query.Initialize();
	
	BINDPARAM_LONG( query, MapKey.m_nObjectTypeID );
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );
	
	BINDCOL_TINYINT( query, DefaultAccessLevel );
	
	query.Execute(  _T("SELECT MAX(AccessLevel) FROM AccessControl WHERE AccessControl.ObjectID = ? ")
					_T("AND (AccessControl.AgentID=? OR GroupID IN ")
					_T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=?))") );
	
	query.Fetch();

	// add the default to the map
	if(MapKey.m_nActualID != 0)
		m_AccessMap[MapKey] = DefaultAccessLevel;
	else
	   m_AccessMap[MapKey] = EMS_ENUM_ACCESS;
	
	query.Initialize();
	
	BINDCOL_LONG_NOLEN( query, MapKey.m_nActualID );
	BINDCOL_BIT_NOLEN( query, UseDefaultRights );
	BINDCOL_TINYINT( query, AccessLevel );

	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, MapKey.m_nObjectTypeID );
	
	query.Execute(	_T("SELECT AgentID, UseDefaultRights, (SELECT MAX(AccessLevel) ")
		_T("FROM AccessControl WHERE ObjectID=Objects.ObjectID AND (AgentID=? ")
		_T("OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=?))) ")
		_T("FROM Agents ") 
		_T("INNER JOIN Objects ON Objects.ObjectID = Agents.ObjectID ")
		_T("WHERE Agents.IsDeleted=0 AND Objects.ObjectTypeID=? ") );
	
	while (query.Fetch() == S_OK)
	{
		if (UseDefaultRights)
		{
			m_AccessMap[MapKey] = DefaultAccessLevel;
		}
		else
		{
			m_AccessMap[MapKey] = AccessLevel;
		}

		AccessLevel = EMS_NO_ACCESS;
	}
	
	// get the access levels this agent has to other agents via groups
	query.Initialize();
	
	BINDCOL_LONG_NOLEN( query, MapKey.m_nActualID );
	BINDCOL_TINYINT( query, AccessLevel );

	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );
	query.Execute(	_T("SELECT data.AgentID,MAX(data.MaxLevel) ")
		_T("FROM ")
		_T("(SELECT Groups.GroupID, AgentGroupings.AgentID,UseDefaultRights, CASE WHEN UseDefaultRights = 0 THEN (SELECT MAX(AccessLevel) ")
		_T("FROM AccessControl WHERE ObjectID=Objects.ObjectID AND (AgentID=? ")
		_T("OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=?))) ELSE (SELECT MAX(AccessLevel) FROM AccessControl WHERE AccessControl.ObjectID = 4 ")
		_T("AND (AccessControl.AgentID=? OR GroupID IN ")
		_T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=?))) END AS MaxLevel ")
		_T("FROM Groups ") 
		_T("INNER JOIN Objects ON Objects.ObjectID = Groups.ObjectID ")
		_T("INNER JOIN AgentGroupings ON Groups.GroupID=AgentGroupings.GroupID ")
		_T("WHERE Groups.IsDeleted=0 AND Objects.ObjectTypeID=4) data ")
		_T("GROUP BY data.AgentID ")
		_T("ORDER BY data.AgentID ") );
	
		while (query.Fetch() == S_OK)
		{
			if (m_AccessMap.find(MapKey) != m_AccessMap.end())
			{
				if (m_AccessMap[MapKey] < AccessLevel)
				{
					m_AccessMap[MapKey] = AccessLevel;
				}
			}
			else
			{
				m_AccessMap[MapKey] = AccessLevel;
			}

			AccessLevel = EMS_NO_ACCESS;
		}
	// an agent always has edit rights for themself
	MapKey.m_nActualID = nAgentID;
	
	if (m_AccessMap.find(MapKey) != m_AccessMap.end())
	{
		if (m_AccessMap[MapKey] < EMS_EDIT_ACCESS)
			m_AccessMap[MapKey] = EMS_EDIT_ACCESS;
	}
	
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Adds all SR categories to the security map for nAgentID	              
\*--------------------------------------------------------------------------*/
void CAccessControl::AddSRCategoriesToMap(CODBCQuery& query, int nAgentID)
{
	
	AccessControlKey MapKey(nAgentID, EMS_OBJECT_TYPE_STD_RESP);
	unsigned char UseDefaultRights;
	unsigned char DefaultAccessLevel = EMS_NO_ACCESS;
	long DefaultAccessLevelLen;
	unsigned char AccessLevel = EMS_NO_ACCESS;
	long AccessLevelLen;
	
	// get the default access level
	query.Initialize();

	BINDPARAM_LONG( query, MapKey.m_nObjectTypeID );
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );

	BINDCOL_TINYINT( query, DefaultAccessLevel );
	
	query.Execute(  _T("SELECT MAX(AccessLevel) FROM AccessControl INNER JOIN Objects ON Objects.ObjectID=AccessControl.ObjectID WHERE Objects.ObjectTypeID=? AND Objects.BuiltIn=1 AND Objects.ActualID=0 ")
					_T("AND (AccessControl.AgentID=? OR GroupID IN ")
					_T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=?))") );
	query.Fetch();
	
	// add the default to the map
	m_AccessMap[MapKey] = DefaultAccessLevel;
	
	// query for all the SR categories
	// for which the agent has access
	query.Initialize();
	
	BINDCOL_LONG_NOLEN( query, MapKey.m_nActualID );
	BINDCOL_BIT_NOLEN( query, UseDefaultRights );
	BINDCOL_TINYINT( query, AccessLevel );
	
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, MapKey.m_nObjectTypeID );
	
	query.Execute(	_T("SELECT StdResponseCatID, UseDefaultRights, (SELECT MAX(AccessLevel) ")
					_T("FROM AccessControl WHERE ObjectID=Objects.ObjectID AND (AgentID=? ")
					_T("OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=?))) ")
					_T("FROM StdResponseCategories ") 
					_T("INNER JOIN Objects ON Objects.ObjectID = StdResponseCategories.ObjectID ")
					_T("WHERE Objects.ObjectTypeID=? ") );

	
	while (query.Fetch() == S_OK)
	{
		if (UseDefaultRights)
		{
			m_AccessMap[MapKey] = DefaultAccessLevel;
		}
		else
		{
			m_AccessMap[MapKey] = AccessLevel;
		}

		AccessLevel = EMS_NO_ACCESS;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates an XML list of ticket categories for which the agent
||				has atleast the access level specified by MinRightLevel	              
\*--------------------------------------------------------------------------*/
int CAccessControl::ListTicketCategories(CODBCQuery& query, CMarkupSTL& xmlgen, CXMLCache& XMLCache, 
									 int nAgentID, unsigned char MinRightLevel, int nIncludeID )
{
	int nTicketCategories = 0;
	bool bAddedTicketCategories = false;
	
	// lock the access control map
	if(m_ACMutex.AcquireLock(1000))
	{
		query.Initialize();
	
		list<unsigned int>::iterator iter;
		AccessMap::iterator AccessIter;
		
		AccessControlKey MapKey( nAgentID, EMS_OBJECT_TYPE_TICKET_CATEGORY );
		XTicketCategoryNames TCatName;
		
		xmlgen.AddChildElem( _T("TicketCategoryNames") );
		xmlgen.IntoElem();
		
		for( iter = XMLCache.m_TicketCategoryIDs.GetList().begin(); iter != XMLCache.m_TicketCategoryIDs.GetList().end(); iter++ )
		{
			MapKey.m_nActualID = *iter;
			AccessIter = m_AccessMap.find( MapKey );
			
			// if the ticket category wasn't found in the access control map
			// we will try to add all of the ticket categories one time...
			if ( AccessIter == m_AccessMap.end() && *iter != nIncludeID )
			{
				if (bAddedTicketCategories)
					continue;
				
				bAddedTicketCategories = true;
				AddTicketCategoriesToMap(query, nAgentID);
				
				// try to find it again
				if ((AccessIter = m_AccessMap.find( MapKey )) == m_AccessMap.end())
					continue;	
			}
			
			// if the access level is adequate, list the item
			if ( AccessIter->second >= MinRightLevel || *iter == nIncludeID )
			{
				if ( XMLCache.m_TicketCategoryNames.Query( *iter, TCatName ) )
				{
					// always list the unassigned ticket category first
					if ( *iter == 1)
					{
						xmlgen.SavePos();
						xmlgen.ResetChildPos();
						xmlgen.InsertChildElem( _T("TicketCategory") );
					}
					else
					{
						xmlgen.AddChildElem( _T("TicketCategory") );
					}
					
					xmlgen.AddChildAttrib( _T("ID"), *iter );
					xmlgen.AddChildAttrib( _T("Name"), TCatName.m_Name );
					
					if ( *iter == 1)
						xmlgen.RestorePos();
					
					nTicketCategories++;
				}
			}
		}
		
		xmlgen.OutOfElem();	
		m_ACMutex.ReleaseLock();
	}
	return nTicketCategories;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates an XML list of ticketboxes for which the agent
||				has atleast the access level specified by MinRightLevel	              
\*--------------------------------------------------------------------------*/
int CAccessControl::ListTicketBoxes(CODBCQuery& query, CMarkupSTL& xmlgen, CXMLCache& XMLCache, 
									 int nAgentID, unsigned char MinRightLevel, int nIncludeID )
{
	int nTicketBoxes = 0;
	bool bAddedTicketBoxes = false;
	
	// lock the access control map
	if(m_ACMutex.AcquireLock(1000))
	{
		query.Initialize();
	
		list<unsigned int>::iterator iter;
		AccessMap::iterator AccessIter;
		
		AccessControlKey MapKey( nAgentID, EMS_OBJECT_TYPE_TICKET_BOX );
		XTicketBoxNames TBoxName;
		
		xmlgen.AddChildElem( _T("TicketBoxNames") );
		xmlgen.IntoElem();
		
		for( iter = XMLCache.m_TicketBoxIDs.GetList().begin(); iter != XMLCache.m_TicketBoxIDs.GetList().end(); iter++ )
		{
			MapKey.m_nActualID = *iter;
			AccessIter = m_AccessMap.find( MapKey );
			
			// if the ticketbox wasn't found in the access control map
			// we will try to add all of the ticketboxes one time...
			if ( AccessIter == m_AccessMap.end() && *iter != nIncludeID )
			{
				if (bAddedTicketBoxes)
					continue;
				
				bAddedTicketBoxes = true;
				AddTicketBoxesToMap(query, nAgentID);
				
				// try to find it again
				if ((AccessIter = m_AccessMap.find( MapKey )) == m_AccessMap.end())
					continue;	
			}
			
			// if the access level is adequate, list the item
			if ( AccessIter->second >= MinRightLevel || *iter == nIncludeID )
			{
				if ( XMLCache.m_TicketBoxNames.Query( *iter, TBoxName ) )
				{
					// always list the unassigned ticketbox first
					if ( *iter == 1)
					{
						xmlgen.SavePos();
						xmlgen.ResetChildPos();
						xmlgen.InsertChildElem( _T("TicketBox") );
					}
					else
					{
						xmlgen.AddChildElem( _T("TicketBox") );
					}
					
					xmlgen.AddChildAttrib( _T("ID"), *iter );
					xmlgen.AddChildAttrib( _T("Name"), TBoxName.m_Name );
					
					if ( *iter == 1)
						xmlgen.RestorePos();
					
					nTicketBoxes++;
				}
			}
		}
		
		xmlgen.OutOfElem();	
		m_ACMutex.ReleaseLock();
	}

	return nTicketBoxes;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates an XML list of ticketboxes for which the agent
||				has atleast the access level specified by MinRightLevel	              
\*--------------------------------------------------------------------------*/
int CAccessControl::ListTicketBoxesDefaultNames(CODBCQuery& query, CMarkupSTL& xmlgen, CXMLCache& XMLCache, 
									 int nAgentID, unsigned char MinRightLevel, int nIncludeID )
{
	int nTicketBoxes = 0;
	bool bAddedTicketBoxes = false;
	
	// lock the access control map
	if(m_ACMutex.AcquireLock(1000))
	{
		query.Initialize();
	
		list<unsigned int>::iterator iter;
		AccessMap::iterator AccessIter;
		
		AccessControlKey MapKey( nAgentID, EMS_OBJECT_TYPE_TICKET_BOX );
		XTicketBoxDefaultEmailAddressName TEBoxName;
		
		xmlgen.AddChildElem( _T("DefaultEmailAddressNames") );
		xmlgen.IntoElem();
		
		for( iter = XMLCache.m_TicketBoxIDs.GetList().begin(); iter != XMLCache.m_TicketBoxIDs.GetList().end(); iter++ )
		{
			MapKey.m_nActualID = *iter;
			AccessIter = m_AccessMap.find( MapKey );
			
			// if the ticketbox wasn't found in the access control map
			// we will try to add all of the ticketboxes one time...
			if ( AccessIter == m_AccessMap.end() && *iter != nIncludeID )
			{
				if (bAddedTicketBoxes)
					continue;
				
				bAddedTicketBoxes = true;
				AddTicketBoxesToMap(query, nAgentID);
				
				// try to find it again
				if ((AccessIter = m_AccessMap.find( MapKey )) == m_AccessMap.end())
					continue;	
			}
			
			// if the access level is adequate, list the item
			if ( AccessIter->second >= MinRightLevel || *iter == nIncludeID )
			{
				if ( XMLCache.m_DefaultEmailAddressName.Query( *iter, TEBoxName ) )
				{
					// always list the unassigned ticketbox first
					if ( *iter == 1)
					{
						xmlgen.SavePos();
						xmlgen.ResetChildPos();
						xmlgen.InsertChildElem( _T("TicketBox") );
					}
					else
					{
						xmlgen.AddChildElem( _T("TicketBox") );
					}
					
					xmlgen.AddChildAttrib( _T("ID"), *iter );
					if(TEBoxName.m_DefaultEmailAddressNameLen != SQL_NULL_DATA)
						xmlgen.AddChildAttrib( _T("DefaultEmailAddressName"), TEBoxName.m_DefaultEmailAddressName );
					else
						xmlgen.AddChildAttrib( _T("DefaultEmailAddressName"),"");
					
					if ( *iter == 1)
						xmlgen.RestorePos();
					
					nTicketBoxes++;
				}
			}
		}
		
		xmlgen.OutOfElem();
		m_ACMutex.ReleaseLock();
	}

	return nTicketBoxes;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates an XML list of agents for which the agent
||				has atleast the access level specified by MinRightLevel	              
\*--------------------------------------------------------------------------*/
int CAccessControl::ListAgents(CODBCQuery& query, CMarkupSTL& xmlgen, CXMLCache& XMLCache, 
							   int nAgentID, unsigned char MinRightLevel, int nIncludeID )
{
	int nAgents = 0;
	bool bAddedAgents = false;
	
	// lock the access control map
	if(m_ACMutex.AcquireLock(1000))
	{
		query.Initialize();
		list<unsigned int>::iterator iter;
		AccessMap::iterator AccessIter;
		
		AccessControlKey MapKey(nAgentID, EMS_OBJECT_TYPE_AGENT);
		XAgentNames AgentName;
		
		xmlgen.AddChildElem( _T("AgentNames") );
		xmlgen.IntoElem();
		
		for( iter = XMLCache.m_AgentIDs.GetList().begin(); iter != XMLCache.m_AgentIDs.GetList().end(); iter++ )
		{
			MapKey.m_nActualID = *iter;
			AccessIter = m_AccessMap.find( MapKey );
			
			if ( AccessIter == m_AccessMap.end() && *iter != nIncludeID )
			{
				if (bAddedAgents)
					continue;
				
				// if the agent wasn't found in the access control map
				// we will try to add all of the agents one time...
				bAddedAgents = true;
				AddAgentsToMap(query, nAgentID);
				
				// try to find it again
				if ((AccessIter = m_AccessMap.find( MapKey )) == m_AccessMap.end())
					continue;	
			}
			
			if ( AccessIter->second >= MinRightLevel || *iter == nIncludeID  )
			{
				if ( XMLCache.m_AgentNames.Query( *iter, AgentName ) )
				{
					xmlgen.AddChildElem( _T("Agent") );
					xmlgen.AddChildAttrib( _T("ID"), *iter );
					xmlgen.AddChildAttrib( _T("Name"), AgentName.m_Name );
					
					nAgents++;
				}
			}
		}
		
		xmlgen.OutOfElem();
		m_ACMutex.ReleaseLock();
	}
	
	return nAgents;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates an XML list of SR categories for which the agent
||				has atleast the access level specified by MinRightLevel	              
\*--------------------------------------------------------------------------*/
int CAccessControl::ListSRCategories(CODBCQuery& query, CMarkupSTL& xmlgen, CXMLCache& XMLCache, 
									 int nAgentID, unsigned char MinRightLevel, int nIncludeID )
{
	int nSRCategories = 0;
	bool bAddedSRCategories = false;
	
	// lock the access control map
	if(m_ACMutex.AcquireLock(1000))
	{
		query.Initialize();
	
		list<unsigned int>::iterator iter;
		AccessMap::iterator AccessIter;
		
		AccessControlKey MapKey( nAgentID, EMS_OBJECT_TYPE_STD_RESP );
		XSRCategoryNames SRCatName;
		
		xmlgen.AddChildElem( _T("stdrespcategories") );
		xmlgen.IntoElem();
		
		for( iter = XMLCache.m_SRCategoryIDs.GetList().begin(); iter != XMLCache.m_SRCategoryIDs.GetList().end(); iter++ )
		{
			MapKey.m_nActualID = *iter;
			AccessIter = m_AccessMap.find( MapKey );
			
			// if the SR category wasn't found in the access control map
			// we will try to add all of the SR categories one time...
			if ( AccessIter == m_AccessMap.end() && *iter != nIncludeID )
			{
				if (bAddedSRCategories)
					continue;
				
				bAddedSRCategories = true;
				AddSRCategoriesToMap(query, nAgentID);
				
				// try to find it again
				if ((AccessIter = m_AccessMap.find( MapKey )) == m_AccessMap.end())
					continue;	
			}
			
			// if the access level is adequate, list the item
			if ( AccessIter->second >= MinRightLevel || *iter == nIncludeID )
			{
				if ( XMLCache.m_SRCategoryNames.Query( *iter, SRCatName ) )
				{
					if (*iter != 1)
					{
						if (*iter == -4)
						{
							xmlgen.SavePos();
							xmlgen.ResetChildPos();
							xmlgen.InsertChildElem( _T("stdrespcategory") );
						}
						else
						{
							xmlgen.AddChildElem( _T("stdrespcategory") );
						}
						
						xmlgen.IntoElem();
						xmlgen.AddChildElem( _T("id"), *iter );
						xmlgen.AddChildElem( _T("name"), SRCatName.m_Name );
						xmlgen.OutOfElem();

						if ( *iter == -4)
							xmlgen.RestorePos();
						
						nSRCategories++;
					}
				}
			}
		}
		
		xmlgen.OutOfElem();	
		m_ACMutex.ReleaseLock();
	}
	return nSRCategories;
}
