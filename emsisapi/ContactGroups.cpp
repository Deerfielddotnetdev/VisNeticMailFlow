// Groups.cpp: implementation of the CContactGroups class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ContactGroups.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CContactGroups::CContactGroups(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nAgentID = 0;
	m_nOwnerID = -1;
	m_bAgentContactGroups = false;
	m_AccessLevel = EMS_NO_ACCESS;
}

CContactGroups::~CContactGroups()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
int CContactGroups::Run(CURLAction& action)
{
	tstring sAction = _T("list");
	
	// get the agentid
	if(!GetISAPIData().GetXMLLong( _T("AgentID"), m_nAgentID, true ))
	{
		if(!GetISAPIData().GetURLLong( _T("AgentID"), m_nAgentID, true ))
		{
			GetISAPIData().GetFormLong( _T("AgentID"), m_nAgentID, true );
		}
	}								  

	if( m_nAgentID == 0)
	{
		// Check security
		// RequireAdmin();
		if(!GetIsAdmin())
		{
			m_AccessLevel = RequireAgentRightLevel(EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS);
			m_bAgentContactGroups = true;
		}
	}
	else
	{
		if((m_nAgentID != GetAgentID()) && !GetIsAdmin())
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
		}
	}

	if(!GetISAPIData().GetXMLString( _T("Action"), sAction, true ))
		if(!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
			GetISAPIData().GetURLString( _T("Action"), sAction, true );

	// get the contactgroupid
	if(!GetISAPIData().GetXMLLong( _T("ID"), m_ContactGroupID, true ))
		if(!GetISAPIData().GetURLLong( _T("ID"), m_ContactGroupID, true ))
			GetISAPIData().GetFormLong( _T("ID"), m_ContactGroupID, true );
		
	
	

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("editcontactgroup") ) == 0)
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
			
			if( m_ContactGroupID == 0 )
			{
				// Change title if ID is zero
				action.m_sPageTitle.assign( _T("New Contact Group") );
			}
			return 0;
		}
	}
	else if ( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();
		DoDelete( action );
	}
	else if ( sAction.compare( _T("searchlist") ) == 0 )
	{
		SearchList( action );
		return 0;
	}

	ListAll( action );
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// ListAll
// 
////////////////////////////////////////////////////////////////////////////////
void CContactGroups::ListAll( CURLAction& action )
{
	int ContactGroupID;
	int ContactID;	
	map<int,tstring> CGMap;
	TCHAR szEmail[255];
	long szEmailLen;
	
	// First, get the membership information and save it in a map
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), ContactGroupID );
	BINDCOL_LONG_NOLEN( GetQuery(), ContactID );
	BINDCOL_TCHAR( GetQuery(), szEmail );
	if(m_nAgentID > 0)
	{
		BINDPARAM_LONG( GetQuery(), m_nAgentID );
		GetQuery().Execute( _T("SELECT CG.ContactGroupID,CG.ContactID,PD.DataValue ")
							_T("FROM ContactGrouping AS CG ") 
							_T("INNER JOIN ContactGroups AS G ON CG.ContactGroupID=G.ContactGroupID ")
							_T("INNER JOIN Contacts AS C ON CG.ContactID=C.ContactID ")
							_T("INNER JOIN PersonalData AS PD ON C.DefaultEmailAddressID=PD.PersonalDataID ")
							_T("WHERE C.IsDeleted=0 AND G.OwnerID = ? ")
							_T("ORDER BY G.GroupName,PD.DataValue ") );
	}
	else if (m_bAgentContactGroups)
	{
		GetQuery().Execute( _T("SELECT CG.ContactGroupID,CG.ContactID,PD.DataValue ")
							_T("FROM ContactGrouping AS CG ") 
							_T("INNER JOIN ContactGroups AS G ON CG.ContactGroupID=G.ContactGroupID ")
							_T("INNER JOIN Contacts AS C ON CG.ContactID=C.ContactID ")
							_T("INNER JOIN PersonalData AS PD ON C.DefaultEmailAddressID=PD.PersonalDataID ")
							_T("WHERE C.IsDeleted=0 AND G.OwnerID=0 ")
							_T("ORDER BY G.GroupName,PD.DataValue ") );
	}
	else
	{
		GetQuery().Execute( _T("SELECT CG.ContactGroupID,CG.ContactID,PD.DataValue ")
							_T("FROM ContactGrouping AS CG ") 
							_T("INNER JOIN ContactGroups AS G ON CG.ContactGroupID=G.ContactGroupID ")
							_T("INNER JOIN Contacts AS C ON CG.ContactID=C.ContactID ")
							_T("INNER JOIN PersonalData AS PD ON C.DefaultEmailAddressID=PD.PersonalDataID ")
							_T("WHERE C.IsDeleted=0")
							_T("ORDER BY G.GroupName,PD.DataValue ") );
	}

	
	while( GetQuery().Fetch() == S_OK )
	{
		if( CGMap.find( ContactGroupID ) == CGMap.end() )
		{
			CGMap[ContactGroupID] = (tstring) szEmail;
		}
		else
		{
			CGMap[ContactGroupID].append( _T(", ") );
			CGMap[ContactGroupID].append( szEmail );
		}
	}

	// Now query for the groups
	TCHAR szOwner[128];
	LONG szOwnerLen;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_ContactGroupID );
	BINDCOL_TCHAR( GetQuery(), m_GroupName );
	if(m_nAgentID > 0)
	{
		_tcscpy( szOwner, _T("") );
		BINDPARAM_LONG( GetQuery(), m_nAgentID );
		GetQuery().Execute( _T("SELECT ContactGroupID,GroupName ")
						_T("FROM ContactGroups ")
						_T("WHERE OwnerID = ? ")
						_T("ORDER BY GroupName") );
	}
	else if (m_bAgentContactGroups)
	{
		BINDCOL_TCHAR( GetQuery(), szOwner );
		GetQuery().Execute( _T("SELECT g.ContactGroupID,g.GroupName,'Global' AS Owner ")
						_T("FROM ContactGroups g WHERE g.OwnerID=0 ")
						_T("ORDER BY Owner,g.GroupName") );
	}
	else
	{
		BINDCOL_TCHAR( GetQuery(), szOwner );
		GetQuery().Execute( _T("SELECT g.ContactGroupID,g.GroupName,CASE WHEN g.OwnerID = 0 THEN 'Global' ELSE a.Name END AS Owner ")
						_T("FROM ContactGroups g ")
						_T("LEFT OUTER JOIN Agents a ON g.OwnerID=a.AgentID ")
						_T("ORDER BY Owner,g.GroupName") );
	}

	GetXMLGen().AddChildElem( _T("Groups") );
	GetXMLGen().IntoElem();
	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Group") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_ContactGroupID );
		GetXMLGen().AddChildAttrib( _T("Name"), m_GroupName );
        GetXMLGen().AddChildAttrib( _T("Owner"), szOwner );

		if( CGMap.find( m_ContactGroupID ) == CGMap.end() )
		{
			GetXMLGen().AddChildAttrib( _T("Members"), _T("") );
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("Members"), CGMap[m_ContactGroupID].c_str() );
		}
	}
	GetXMLGen().OutOfElem();

	if(m_nAgentID == 0  && !m_bAgentContactGroups)
	{
		ListAgentNames();
	}

	if(m_bAgentContactGroups)
	{
		GetXMLGen().AddChildElem( _T("AgentContactGroups") );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SearchList
// 
////////////////////////////////////////////////////////////////////////////////
void CContactGroups::SearchList( CURLAction& action )
{
	int ContactGroupID;
	int ContactID;	
	map<int,tstring> CGMap;
	TCHAR szEmail[255];
	long szEmailLen;
	CEMSString m_sGroupName;
	CEMSString sTemp;
	CEMSString sQuery;
		
	if(!GetISAPIData().GetFormString( _T("GNAME"), m_sGroupName, true ))
		if(!GetISAPIData().GetXMLString( _T("GNAME"), m_sGroupName, true ))
			GetISAPIData().GetURLString( _T("GNAME"), m_sGroupName, true );
	
	if(!GetISAPIData().GetFormLong( _T("OwnerID"), m_nOwnerID, true ))
			if(!GetISAPIData().GetXMLLong( _T("OwnerID"), m_nOwnerID, true ))
				GetISAPIData().GetURLLong( _T("OwnerID"), m_nOwnerID, true );

	CEMSString sWhereClause;
	sWhereClause.reserve(256);
	sWhereClause.assign("C.IsDeleted = 0 ");

	// First, get the membership information and save it in a map
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), ContactGroupID );
	BINDCOL_LONG_NOLEN( GetQuery(), ContactID );
	BINDCOL_TCHAR( GetQuery(), szEmail );

	if ( m_sGroupName.length() > 0 )
	{
		m_sGroupName.EscapeSQL();
		sTemp = m_sGroupName;
		m_sGroupName.insert( 0, _T("%") );
		m_sGroupName.append( _T("%") );
				
		BINDPARAM_TCHAR( GetQuery(), (void*) m_sGroupName.c_str() );

		sWhereClause.append( _T(" AND CG.ContactGroupID IN (SELECT ContactGroupID FROM ContactGroups ")
							_T("WHERE GroupName LIKE ?)") );

	}
	
	if(m_nOwnerID > -1 )
	{
		BINDPARAM_LONG( GetQuery(), m_nOwnerID );

		sWhereClause.append( _T(" AND G.OwnerID = ?") );
	}
		
	if(m_nAgentID > 0)
	{
		BINDPARAM_LONG( GetQuery(), m_nAgentID );
		sQuery.Format( _T("SELECT CG.ContactGroupID,CG.ContactID,PD.DataValue ")
						_T("FROM ContactGrouping AS CG ") 
						_T("INNER JOIN ContactGroups AS G ON CG.ContactGroupID=G.ContactGroupID ")
						_T("INNER JOIN Contacts AS C ON CG.ContactID=C.ContactID ")
						_T("INNER JOIN PersonalData AS PD ON C.DefaultEmailAddressID=PD.PersonalDataID ")
						_T("WHERE %s AND G.OwnerID = ? ")
						_T("ORDER BY G.GroupName,PD.DataValue "), sWhereClause.c_str() );
		
	}
	else
	{
		sQuery.Format( _T("SELECT CG.ContactGroupID,CG.ContactID,PD.DataValue ")
						_T("FROM ContactGrouping AS CG ") 
						_T("INNER JOIN ContactGroups AS G ON CG.ContactGroupID=G.ContactGroupID ")
						_T("INNER JOIN Contacts AS C ON CG.ContactID=C.ContactID ")
						_T("INNER JOIN PersonalData AS PD ON C.DefaultEmailAddressID=PD.PersonalDataID ")
						_T("WHERE %s ")
						_T("ORDER BY G.GroupName,PD.DataValue "), sWhereClause.c_str() );
	}

	GetQuery().Execute(sQuery.c_str());
	while( GetQuery().Fetch() == S_OK )
	{
		if( CGMap.find( ContactGroupID ) == CGMap.end() )
		{
			CGMap[ContactGroupID] = (tstring) szEmail;
		}
		else
		{
			CGMap[ContactGroupID].append( _T(", ") );
			CGMap[ContactGroupID].append( szEmail );
		}
	}

	// Now query for the groups
	TCHAR szOwner[128];
	LONG szOwnerLen;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_ContactGroupID );
	BINDCOL_TCHAR( GetQuery(), m_GroupName );
	if(m_nAgentID > 0)
	{
		_tcscpy( szOwner, _T("") );
		BINDPARAM_LONG( GetQuery(), m_nAgentID );
		GetQuery().Execute( _T("SELECT ContactGroupID,GroupName ")
						_T("FROM ContactGroups ")
						_T("WHERE OwnerID = ? ")
						_T("ORDER BY GroupName") );
	}
	else
	{
		BINDCOL_TCHAR( GetQuery(), szOwner );
		GetQuery().Execute( _T("SELECT g.ContactGroupID,g.GroupName,CASE WHEN g.OwnerID = 0 THEN 'Global' ELSE a.Name END AS Owner ")
						_T("FROM ContactGroups g ")
						_T("LEFT OUTER JOIN Agents a ON g.OwnerID=a.AgentID ")
						_T("ORDER BY Owner,g.GroupName") );
	}

	GetXMLGen().AddChildElem( _T("Groups") );
	GetXMLGen().AddChildAttrib( _T("GroupName"), sTemp.c_str() );
	GetXMLGen().AddChildAttrib( _T("OwnerID"), m_nOwnerID );
	GetXMLGen().IntoElem();
	
	while( GetQuery().Fetch() == S_OK )
	{
		if( CGMap.find( m_ContactGroupID ) != CGMap.end() )
		{
			GetXMLGen().AddChildElem( _T("Group") );
			GetXMLGen().AddChildAttrib( _T("ID"), m_ContactGroupID );
			GetXMLGen().AddChildAttrib( _T("Name"), m_GroupName );
			GetXMLGen().AddChildAttrib( _T("Owner"), szOwner );
			GetXMLGen().AddChildAttrib( _T("Members"), CGMap[m_ContactGroupID].c_str() );
		}
	}
	GetXMLGen().OutOfElem();

	if(m_nAgentID == 0  && !m_bAgentContactGroups)
	{
		ListAgentNames();
	}

	if(m_bAgentContactGroups)
	{
		GetXMLGen().AddChildElem( _T("AgentContactGroups") );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// QueryOne
// 
////////////////////////////////////////////////////////////////////////////////
void CContactGroups::QueryOne( CURLAction& action )
{
	int ContactID;

	if( m_ContactGroupID == 0 )
		return;
	
	GetQuery().Initialize();
	BINDCOL_TCHAR( GetQuery(), m_GroupName );
	BINDPARAM_LONG( GetQuery(), m_ContactGroupID );	
	GetQuery().Execute( _T("SELECT GroupName ")
						_T("FROM ContactGroups ")
						_T("WHERE ContactGroupID=?") );

	if( GetQuery().Fetch() != S_OK )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
	}

	// First, get the membership information and save it in a map
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_ContactGroupID );
	BINDCOL_LONG_NOLEN( GetQuery(), ContactID );
	GetQuery().Execute( _T("SELECT CG.ContactID ")
						_T("FROM ContactGrouping AS CG ") 
						_T("INNER JOIN Contacts AS C ON CG.ContactID=C.ContactID ")
						_T("INNER JOIN PersonalData AS PD ON C.DefaultEmailAddressID=PD.PersonalDataID ")							
						_T("WHERE CG.ContactGroupID=? AND C.IsDeleted=0 ")
						_T("ORDER BY PD.DataValue ") );

	while( GetQuery().Fetch() == S_OK )
	{
		m_IDs.push_back( ContactID );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoUpdate
// 
////////////////////////////////////////////////////////////////////////////////
void CContactGroups::DoUpdate( CURLAction& action )
{
	int ContactID;
	set<int> m_DBIDs;
	set<int>::iterator DBiter;
	list<int>::iterator iter;

	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_ContactGroupID );
	BINDPARAM_TCHAR( GetQuery(), m_GroupName );
	GetQuery().Execute( _T("SELECT ContactGroupID FROM ContactGroups ")
						_T("WHERE ContactGroupID<>? AND GroupName=? ") );

	if( GetQuery().Fetch() == S_OK )
	{
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Contact Group name must be unique")  );
	}

	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_GroupName );
	BINDPARAM_LONG( GetQuery(), m_ContactGroupID );
	GetQuery().Execute( _T("UPDATE ContactGroups ")
						_T("SET GroupName=? ")
						_T("WHERE ContactGroupID=?") );

	if( GetQuery().GetRowCount() == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
	}

	if( m_ContactGroupID > 0 )
	{
		// First, get the membership information and save it in a map
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_ContactGroupID );
		BINDCOL_LONG_NOLEN( GetQuery(), ContactID );
		GetQuery().Execute( _T("SELECT CG.ContactID ")
							_T("FROM ContactGrouping AS CG ") 
							_T("INNER JOIN Contacts AS C ON CG.ContactID=C.ContactID ")
							_T("WHERE CG.ContactGroupID=? AND C.IsDeleted=0 ") );

		while( GetQuery().Fetch() == S_OK )
		{
			m_DBIDs.insert( ContactID );
		}

		for( iter = m_IDs.begin(); iter != m_IDs.end(); iter++ )
		{
			if( m_DBIDs.find(*iter) == m_DBIDs.end() )
			{
				ContactID = *iter;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_ContactGroupID );
				BINDPARAM_LONG( GetQuery(), ContactID );
				GetQuery().Execute( _T("INSERT INTO ContactGrouping ")
									_T("(ContactGroupID,ContactID) ") 
									_T("VALUES ")
									_T("(?,?)") );
			}
		}

		for( DBiter = m_DBIDs.begin(); DBiter != m_DBIDs.end(); DBiter++ )
		{
			bool bFound = false;

			for( iter = m_IDs.begin(); iter != m_IDs.end(); iter++ )
			{
				if( *DBiter == *iter )
				{
					bFound = true;
					break;
				}
			}
			
			if( bFound == false )
			{
				ContactID = *DBiter;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_ContactGroupID );
				BINDPARAM_LONG( GetQuery(), ContactID );
				GetQuery().Execute( _T("DELETE FROM ContactGrouping ")
									_T("WHERE ContactGroupID=? and ContactID=? ") );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoInsert
// 
////////////////////////////////////////////////////////////////////////////////
void CContactGroups::DoInsert( CURLAction& action )
{
	if(!GetIsAdmin() && m_bAgentContactGroups)
	{
		m_AccessLevel = RequireAgentRightLevel(EMS_OBJECT_TYPE_CONTACT, 0, EMS_DELETE_ACCESS);		
	}
	
	int ContactID;
	list<int>::iterator iter;

	if(m_bAgentContactGroups)
	{
		m_OwnerID = 0;
	}
	else if(m_nOwnerID > 0 && m_nAgentID == 0)
	{
		m_OwnerID = m_nOwnerID;
	}
	else
	{
		m_OwnerID = m_nAgentID;
	}
	
	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_GroupName );
	GetQuery().Execute( _T("SELECT ContactGroupID FROM ContactGroups ")
						_T("WHERE GroupName=? ") );

	if( GetQuery().Fetch() == S_OK )
	{
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Group name must be unique")  );
	}

	TContactGroups::Insert( GetQuery() );

	// Update the object row with the actual ID
	GetQuery().Reset( true );

	// Add the members
	for( iter = m_IDs.begin(); iter != m_IDs.end(); iter++ )
	{
		ContactID = *iter;
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_ContactGroupID );
		BINDPARAM_LONG( GetQuery(), ContactID );
		GetQuery().Execute( _T("INSERT INTO ContactGrouping ")
							_T("(ContactGroupID,ContactID) ") 
							_T("VALUES ")
							_T("(?,?)") );
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoDelete
// 
////////////////////////////////////////////////////////////////////////////////
void CContactGroups::DoDelete( CURLAction& action )
{
	if(!GetIsAdmin() && m_bAgentContactGroups)
	{
		m_AccessLevel = RequireAgentRightLevel(EMS_OBJECT_TYPE_CONTACT, 0, EMS_DELETE_ACCESS);		
	}
	
	int nOwnerID;
	CEMSString sIDs;

	GetISAPIData().GetXMLString( _T("SelectID"),sIDs , true );
	
	while ( sIDs.CDLGetNextInt(m_ContactGroupID))
	{


		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_ContactGroupID );
		BINDCOL_LONG_NOLEN( GetQuery(), nOwnerID );
		GetQuery().Execute( _T("SELECT OwnerID ")
							_T("FROM ContactGroups ")
							_T("WHERE ContactGroupID=?") );

		if( GetQuery().Fetch() != S_OK )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
		}

		if((m_nAgentID != nOwnerID) && !GetIsAdmin())
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( _T("You can only delete Contact Groups you own!") ) );
		}

		// commit the group
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_ContactGroupID );
		GetQuery().Execute( _T("DELETE FROM ContactGrouping ")
							_T("WHERE ContactGroupID=?") );

		// Delete the group
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_ContactGroupID );
		GetQuery().Execute( _T("DELETE FROM ContactGroups ")
							_T("WHERE ContactGroupID=?") );
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecodeForm
// 
////////////////////////////////////////////////////////////////////////////////
void CContactGroups::DecodeForm( void )
{
	CEMSString sCollection;
	int ContactID;

	GetISAPIData().GetXMLTCHAR( _T("GroupName"), m_GroupName, 51 );
	GetISAPIData().GetXMLString( _T("ContactIDCollection"), sCollection, false );

	// get the ownerid
	if(!GetISAPIData().GetXMLLong( _T("OwnerID"), m_nOwnerID, true ))
	{
		if(!GetISAPIData().GetURLLong( _T("OwnerID"), m_nOwnerID, true ))
		{
			GetISAPIData().GetFormLong( _T("OwnerID"), m_nOwnerID, true );
		}
	}
	
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( ContactID ) )
	{
		m_IDs.push_back( ContactID );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GenerateXML
// 
////////////////////////////////////////////////////////////////////////////////
void CContactGroups::GenerateXML( void )
{
	TCHAR Email[PERSONALDATA_DATAVALUE_LENGTH] = {0};
	long EmailLen;
	CEMSString sEmail;
	CEMSString sQuery;
	list<int>::iterator iter;
	int ContactID;
	CEMSString m_sContactEmail;
	unsigned char m_ExactContactEmail;
	CEMSString sTemp;
	int nMaxLines = GetSession().m_nMaxRowsPerPage;
	int nExact = 0;
	int nDoGlobal = 0;

	if(!GetISAPIData().GetXMLLong( _T("chkDoGlobal"), nDoGlobal, true ))
		if(!GetISAPIData().GetURLLong( _T("chkDoGlobal"), nDoGlobal, true ))
			GetISAPIData().GetFormLong( _T("chkDoGlobal"), nDoGlobal, true );
	
	GetISAPIData().GetFormString( _T("CONTACT_EMAIL"), m_sContactEmail, true );	
	if ( m_sContactEmail.length() > 0 )
	{
		CEMSString sCollection;
		if(!GetISAPIData().GetURLTCHAR( _T("GroupName"), m_GroupName, 51, true ))
			if(!GetISAPIData().GetFormTCHAR( _T("GroupName"), m_GroupName, 51, true ))
				GetISAPIData().GetXMLTCHAR( _T("GroupName"), m_GroupName, 51, true );

		if(!GetISAPIData().GetURLString( _T("ContactIDCollection"), sCollection, true ))
			if(!GetISAPIData().GetFormString( _T("ContactIDCollection"), sCollection, true ))
				GetISAPIData().GetXMLString( _T("ContactIDCollection"), sCollection, true );

		sCollection.CDLInit();
		while( sCollection.CDLGetNextInt( ContactID ) )
		{
			m_IDs.push_back( ContactID );
		}

		if(!GetISAPIData().GetXMLLong( _T("OwnerID"), m_nOwnerID, true ))
			if(!GetISAPIData().GetURLLong( _T("OwnerID"), m_nOwnerID, true ))
				GetISAPIData().GetFormLong( _T("OwnerID"), m_nOwnerID, true );			
		
	}
	
	GetXMLGen().AddChildElem( _T("ContactGroup") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_ContactGroupID );
	GetXMLGen().AddChildAttrib( _T("AgentID"), m_nAgentID );
	GetXMLGen().AddChildAttrib( _T("Name"), m_GroupName );
	GetXMLGen().AddChildAttrib( _T("OwnerID"), m_nOwnerID );
	GetXMLGen().IntoElem();

	for( iter = m_IDs.begin(); iter != m_IDs.end(); iter++ )
	{
		GetQuery().Initialize();
		BINDCOL_TCHAR( GetQuery(), Email );
		BINDPARAM_LONG( GetQuery(), *iter );		
		GetQuery().Execute( _T("SELECT DataValue FROM PersonalData ")
							_T("INNER JOIN Contacts ON PersonalData.PersonalDataID = Contacts.DefaultEmailAddressID ")
							_T("WHERE Contacts.ContactID=?") );

		if ( GetQuery().Fetch() == S_OK && EmailLen )
		{
			GetXMLGen().AddChildElem( _T("Contact") );
			GetXMLGen().AddChildAttrib( _T("ID"), *iter );
			GetXMLGen().AddChildAttrib( _T("Name"), Email );
		}
	}

	GetXMLGen().OutOfElem();

	GetXMLGen().AddChildElem( _T("MyContacts") );
	GetXMLGen().IntoElem();	
	
	GetISAPIData().GetFormLong( _T("CONTACT_EMAIL_EXACT"), m_ExactContactEmail, true);
	GetISAPIData().GetFormLong( _T("MAXLINES"), nMaxLines, true);
	if(nMaxLines == 0)
	{
		nMaxLines = GetSession().m_nMaxRowsPerPage;
	}

	CEMSString sWhereClause;
	sWhereClause.reserve(256);
	sWhereClause.assign("Contacts.IsDeleted = 0 ");

	GetQuery().Initialize();
	BINDCOL_TCHAR( GetQuery(), Email );
	BINDCOL_LONG_NOLEN( GetQuery(), ContactID );
	
	if ( m_sContactEmail.length() > 0 )
	{
		m_sContactEmail.EscapeSQL();
		sTemp = m_sContactEmail;
		
		BINDPARAM_TCHAR( GetQuery(), (void*) m_sContactEmail.c_str() );

		sWhereClause.append( _T(" AND EXISTS (SELECT PersonalDataID FROM PersonalData ")
							_T("WHERE ContactID = Contacts.ContactID AND PersonalDataTypeID=1 AND DataValue") );

		sWhereClause.append( m_ExactContactEmail ? _T("=?)") : _T(" LIKE ?)") );

		if ( !m_ExactContactEmail )
		{
			nExact = 0;
			m_sContactEmail.insert( 0, _T("%") );
			m_sContactEmail.append( _T("%") );
		}
		else
		{
			nExact = 1;
		}
	}

	if(m_nAgentID == 0 || nDoGlobal == 1)
	{
		sQuery.Format( _T("SELECT TOP %d PersonalData.DataValue,Contacts.ContactID FROM PersonalData ")
							_T("INNER JOIN Contacts ON PersonalData.PersonalDataID = Contacts.DefaultEmailAddressID ")
							_T("WHERE %s ORDER BY PersonalData.DataValue"),nMaxLines, sWhereClause.c_str() );
	}
	else
	{
		BINDPARAM_LONG( GetQuery(), m_nAgentID );
		sQuery.Format( _T("SELECT TOP %d PersonalData.DataValue,Contacts.ContactID FROM PersonalData ")
							_T("INNER JOIN Contacts ON PersonalData.PersonalDataID = Contacts.DefaultEmailAddressID ")
							_T("INNER JOIN AgentContacts ON AgentContacts.ContactID = Contacts.ContactID ")
							_T("WHERE %s AND AgentContacts.AgentID = ? ORDER BY PersonalData.DataValue"),nMaxLines, sWhereClause.c_str() );
	}

	GetQuery().Execute(sQuery.c_str());

	while( GetQuery().Fetch() == S_OK )
	{
		if( EmailLen > 0)
		{
			GetXMLGen().AddChildElem( _T("Contact") );
			GetXMLGen().AddChildAttrib( _T("ID"), ContactID );
			GetXMLGen().AddChildAttrib( _T("Name"), Email );
		}
	}	

	GetXMLGen().OutOfElem();

	if(m_nAgentID == 0  && !m_bAgentContactGroups)
	{
		ListAgentNames();
	}

	// add parameters
	GetXMLGen().AddChildElem( _T("Parameters") );	
	GetXMLGen().AddChildAttrib( _T("MaxRecs"), nMaxLines );
	GetXMLGen().AddChildAttrib( _T("Email"), sTemp.c_str() );
	GetXMLGen().AddChildAttrib( _T("Exact"), nExact );
	GetXMLGen().AddChildAttrib( _T("DoGlobal"), nDoGlobal );
	
}