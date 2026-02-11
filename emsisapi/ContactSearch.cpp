/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ContactSearch.cpp,v 1.2 2005/11/29 21:16:25 markm Exp $
||
||
||                                         
||  COMMENTS:  Contact Search 
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ContactSearch.h"
#include "DateFns.h"
#include "ContactFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CContactSearch::CContactSearch(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nMaxLines = GetSession().m_nMaxRowsPerPage;
	m_nMergeContactID = 0;
	m_ExactContactEmail = 0;
	m_ExactContactName = 0;
	m_nOwnerID = 0;
	m_nState = 0;
	m_ExactCompanyName = 0;
	m_ExactWebsite = 0;
	m_ExactAddress;
	m_ExactCity = 0;
	m_ExactState = 0;
	m_ExactCountry = 0;
	m_ExactZipCode = 0;
	m_nPersonalDataType = 0;
	m_ExactPersonalData = 0;
	m_nSearchType = 0;
	m_nChkGroups = 0;
}

CContactSearch::~CContactSearch()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CContactSearch::Run(CURLAction& action)
{
	tstring sAction;
	
	// get the action
	GetISAPIData().GetXMLPost() ? GetISAPIData().GetXMLString( _T("Action"), sAction, true ) :
								  GetISAPIData().GetURLString( _T("Action"), sAction, true );

	// attempt to get the merge source ID 
	if (!GetISAPIData().GetURLLong( _T("MERGE"), m_nMergeContactID, true ))
		GetISAPIData().GetURLLong( _T("MERGESOURCEID"), m_nMergeContactID, true );

	// attempt to get the agentid
	if (GetISAPIData().GetURLLong( _T("AgentContactID"), m_nAgentID, true ) && m_nAgentID > 0)
	{
		m_nSearchType = 2;
	}
	else if (GetISAPIData().GetURLLong( _T("AGENTID"), m_nAgentID, true) && m_nAgentID > 0)
	{
		m_nSearchType = 1;
	}
	else if (GetISAPIData().GetXMLLong( _T("AgentContactID"), m_nAgentID, true) && m_nAgentID > 0)
	{
		m_nSearchType = 0;
	}
	
	if(m_nAgentID > 0 && ((m_nAgentID ==  m_ISAPIData.m_pSession->m_AgentID) || GetIsAdmin()))
	{
		GetXMLGen().AddChildElem( _T("ISAGENTCONTACTS") );
		GetXMLGen().AddChildAttrib( _T("AgentID"), m_nAgentID );
	}
				

	if ( m_nMergeContactID )
	{
		TContacts Contact;
		Contact.m_ContactID = m_nMergeContactID;

		if ( Contact.Query( GetQuery() ) != S_OK )
		{
			CEMSString sError;
			sError.Format(_T("Invalid Merge Source ContactID (%d)"), m_nMergeContactID);
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
		
		GetXMLGen().AddChildElem( _T("Merge") );
		GetXMLGen().AddChildAttrib( _T("SourceID"), m_nMergeContactID );
		GetXMLGen().AddChildAttrib( _T("SourceName"), Contact.m_Name );
	}
	
	if ( sAction.compare( _T("search") ) == 0 )
	{
		PreformQuery();
	}
	else if ( sAction.compare( _T("delete") ) == 0 )
	{
		Delete();
	}
	else if ( sAction.compare( _T("remove") ) == 0 )
	{
		Remove();
	}
	else if ( sAction.compare( _T("addmycontact") ) == 0 )
	{
		AddMyContact();
	}
	else if ( sAction.compare( _T("reset") ) == 0 )
	{
		Reset();
	}
	else
	{
		// add max rows
		GetXMLGen().AddChildElem( _T("Parameters") );
		GetXMLGen().AddChildAttrib( _T("MaxRecs"), m_nMaxLines );

		ListAgentNames();
		ListPersonalDataTypes();
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Read form parameters
\*--------------------------------------------------------------------------*/
void CContactSearch::DecodeForm( void )
{
	GetISAPIData().GetURLString( _T("CONTACT_NAME"), m_sContactName );
	GetISAPIData().GetURLLong( _T("CONTACT_NAME_EXACT"), m_ExactContactName );
	
	GetISAPIData().GetURLString( _T("CONTACT_EMAIL"), m_sContactEmail );
	GetISAPIData().GetURLLong( _T("CONTACT_EMAIL_EXACT"), m_ExactContactEmail);

	GetISAPIData().GetURLLong( _T("CONTACT_OWNER"), m_nOwnerID );	
	GetISAPIData().GetURLLong( _T("VERIFICATION_STATUS"), m_nState, true );	

	GetISAPIData().GetURLString( _T("CONTACT_COMPANY_NAME"), m_sCompanyName );
	GetISAPIData().GetURLLong( _T("CONTACT_COMPANY_NAME_EXACT"), m_ExactCompanyName );
	
	GetISAPIData().GetURLString( _T("CONTACT_WEBSITE"), m_sWebsite );
	GetISAPIData().GetURLLong( _T("CONTACT_WEBSITE_EXACT"), m_ExactWebsite );

	GetISAPIData().GetURLString( _T("CONTACT_ADDRESS"), m_sAddress );
	GetISAPIData().GetURLLong( _T("CONTACT_ADDRESS_EXACT"), m_ExactAddress );

	GetISAPIData().GetURLString( _T("CONTACT_CITY"), m_sCity );
	GetISAPIData().GetURLLong( _T("CONTACT_CITY_EXACT"), m_ExactCity );

	GetISAPIData().GetURLString( _T("CONTACT_STATE"), m_sState );
	GetISAPIData().GetURLLong( _T("CONTACT_STATE_EXACT"), m_ExactState );
	
	GetISAPIData().GetURLString( _T("CONTACT_COUNTRY"), m_sCountry );
	GetISAPIData().GetURLLong( _T("CONTACT_COUNTRY_EXACT"), m_ExactCountry );
	
	GetISAPIData().GetURLString( _T("CONTACT_ZIP"), m_sZipCode );
	GetISAPIData().GetURLLong( _T("CONTACT_ZIP_EXACT"), m_ExactZipCode );

	GetISAPIData().GetURLLong( _T("CONTACT_PERSONALDATA_TYPE"), m_nPersonalDataType);
	GetISAPIData().GetURLLong( _T("CONTACT_PERSONALDATA_EXACT"), m_ExactPersonalData);
	GetISAPIData().GetURLString( _T("CONTACT_PERSONALDATA_VALUE"), m_sPersonalData );
	
	GetISAPIData().GetURLLong( _T("MAXLINES"), m_nMaxLines );
	if(GetISAPIData().GetURLLong( _T("CHKGROUPS"), m_nChkGroups, true ))
		m_nChkGroups = 1;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Perform Query and generate XML
\*--------------------------------------------------------------------------*/
void CContactSearch::PreformQuery( void )
{
	CEMSString sQuery;
	CEMSString sWhereClause;
	int nContactID;
	int nNoteCount;
	int nAddressState;
	TCHAR Name[CONTACTS_NAME_LENGTH];
	long  NameLen;
	TCHAR EmailAddr[CONTACTS_NAME_LENGTH];
	long  EmailAddrLen;

	if(((m_nAgentID ==  m_ISAPIData.m_pSession->m_AgentID) || GetIsAdmin()) && m_nSearchType == 1)
	{
		GetQuery().Initialize();
		
		BINDCOL_LONG_NOLEN( GetQuery(), nContactID );
		BINDCOL_TCHAR( GetQuery(), Name );
		BINDCOL_TCHAR( GetQuery(), EmailAddr );
		BINDCOL_LONG_NOLEN( GetQuery(), nAddressState );		
		BINDCOL_LONG_NOLEN( GetQuery(), nNoteCount );
		
		sQuery.Format( _T("SELECT TOP %d Contacts.ContactID, Contacts.Name, ")
					_T("PersonalData.DataValue as Email, ")
					_T("PersonalData.StatusID as Status, ")
					_T("(SELECT COUNT(*) FROM ContactNotes WHERE ContactID = Contacts.ContactID) ")
					_T("FROM Contacts INNER JOIN AgentContacts ON Contacts.ContactID=AgentContacts.ContactID ")
					_T("INNER JOIN PersonalData ON Contacts.DefaultEmailAddressID=PersonalData.PersonalDataID ")
					_T("WHERE AgentContacts.AgentID = %d ")
					_T("ORDER BY Contacts.Name, Email"), m_nMaxLines + 1, m_nAgentID );
		
		GetQuery().Execute( sQuery.c_str() );
	}
	else if ( ((m_nAgentID ==  m_ISAPIData.m_pSession->m_AgentID) || GetIsAdmin()) && m_nSearchType == 2)
	{
		DecodeForm();
		
		GetQuery().Initialize();
		
		FormatWhereClause( sWhereClause );
		
		BINDCOL_LONG_NOLEN( GetQuery(), nContactID );
		BINDCOL_TCHAR( GetQuery(), Name );
		BINDCOL_TCHAR( GetQuery(), EmailAddr );
		BINDCOL_LONG_NOLEN( GetQuery(), nAddressState );		
		BINDCOL_LONG_NOLEN( GetQuery(), nNoteCount );
		
		sQuery.Format( _T("SELECT TOP %d Contacts.ContactID, Contacts.Name, ")
					_T("PersonalData.DataValue as Email, ")
					_T("PersonalData.StatusID as Status, ")
					_T("(SELECT COUNT(*) FROM ContactNotes WHERE ContactID = Contacts.ContactID) ")
					_T("FROM Contacts INNER JOIN AgentContacts ON Contacts.ContactID=AgentContacts.ContactID ")
					_T("INNER JOIN PersonalData ON Contacts.DefaultEmailAddressID=PersonalData.PersonalDataID ")
					_T("WHERE AgentContacts.AgentID = %d AND %s ")
					_T("ORDER BY Contacts.Name, Email"), m_nMaxLines + 1, m_nAgentID, sWhereClause.c_str() );
		
		GetQuery().Execute( sQuery.c_str() );
		
	}
	else
	{
		// require read access to contacts
		RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS );
		
		DecodeForm();
		
		GetQuery().Initialize();
		
		FormatWhereClause( sWhereClause );
		
		BINDCOL_LONG_NOLEN( GetQuery(), nContactID );
		BINDCOL_TCHAR( GetQuery(), Name );
		BINDCOL_TCHAR( GetQuery(), EmailAddr );
		BINDCOL_LONG_NOLEN( GetQuery(), nAddressState );		
		BINDCOL_LONG_NOLEN( GetQuery(), nNoteCount );
		
		if(m_nChkGroups > 0)
		{
			CEMSString sWhere;
			sWhere.Format(_T("(g.OwnerID=0 OR g.OwnerID = %d) "), m_ISAPIData.m_pSession->m_AgentID );

			if ( m_sContactName.length() > 0 )
			{
				BINDPARAM_TCHAR( GetQuery(), (void*) m_sContactName.c_str() );

				sWhere.append( _T(" AND g.GroupName") );
				sWhere.append( m_ExactContactName ? _T("=?") : _T(" LIKE ?") );
			}

			sQuery.Format( _T("SET ROWCOUNT %d SELECT Contacts.ContactID, Contacts.Name as Name, ")
						   _T("PersonalData.DataValue as Email, ")
						   _T("PersonalData.StatusID as Status, ")
						   _T("(SELECT COUNT(*) FROM ContactNotes WHERE ContactID = Contacts.ContactID) ")
						   _T("FROM Contacts INNER JOIN PersonalData ON Contacts.DefaultEmailAddressID=PersonalData.PersonalDataID ")
						   _T("WHERE %s UNION ")
						   _T("SELECT g.ContactGroupID, '[Contact Group]' as Name, ")
						   _T("g.GroupName as Email, -1, 0 ")
						   _T("FROM ContactGroups g ")
						   _T("WHERE %s ")
						   _T("ORDER BY Name, Email"), m_nMaxLines + 1, sWhereClause.c_str(), sWhere.c_str()  );
		}
		else
		{
			sQuery.Format( _T("SELECT TOP %d Contacts.ContactID, Contacts.Name, ")
						   _T("PersonalData.DataValue as Email, ")
						   _T("PersonalData.StatusID as Status, ")
						   _T("(SELECT COUNT(*) FROM ContactNotes WHERE ContactID = Contacts.ContactID) ")
						   _T("FROM Contacts INNER JOIN PersonalData ON Contacts.DefaultEmailAddressID=PersonalData.PersonalDataID ")
						   _T("WHERE %s ")
						   _T("ORDER BY Contacts.Name, Email"), m_nMaxLines + 1, sWhereClause.c_str() );
		}
				
		GetQuery().Execute( sQuery.c_str() );
	}
	
	bool bHasEmail, bHasName;

	int nLines = 0;
	for(; GetQuery().Fetch() == S_OK; nLines++ )
	{
		if ( nLines < m_nMaxLines )
		{
			bHasName  = NameLen > 0; 
			bHasEmail = EmailAddrLen > 0 && EmailAddrLen != SQL_NULL_DATA;
			
			GetXMLGen().AddChildElem( _T("Item") );
			GetXMLGen().AddChildAttrib( _T("ID"), nContactID );
			GetXMLGen().AddChildAttrib( _T("NoteCount"), nNoteCount );
			
			// used by the web interface so "[No name defined]" is 
			// not copied when a contact is selected for a new ticket
			GetXMLGen().AddChildAttrib( _T("HasName"), bHasName );
			GetXMLGen().AddChildAttrib( _T("HasEmail"), bHasEmail );

			if(bHasEmail)
			{
				GetXMLGen().AddChildAttrib( _T("AddressState"), nAddressState );
			}
			else
			{
				GetXMLGen().AddChildAttrib( _T("AddressState"), -1 );
			}

			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("Name") );
			GetXMLGen().SetChildData( bHasName ? Name : _T("[No name defined]"), TRUE );
			GetXMLGen().OutOfElem();

			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("DefaultEmail") );
			GetXMLGen().SetChildData( bHasEmail ? EmailAddr : _T("[No default email address defined]"), TRUE );
			GetXMLGen().OutOfElem();			
		}
	}
	
	// do we need to add the more tag
	if ( nLines > m_nMaxLines )
	{
		CEMSString sMoreDesc;
		sMoreDesc.Format( _T("Maxium search result count reached (%d)."), m_nMaxLines );
		GetXMLGen().AddChildElem( _T("More") );
		GetXMLGen().AddChildAttrib( _T("Description"), sMoreDesc.c_str() );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Format the query's where clause	              
\*--------------------------------------------------------------------------*/
void CContactSearch::FormatWhereClause( CEMSString& sWhereClause )
{
	sWhereClause.reserve(256);
	sWhereClause.assign("Contacts.IsDeleted = 0 ");
	
	// don't return the source contact when merging contacts
	if ( m_nMergeContactID > 0 )
	{
		BINDPARAM_LONG( GetQuery(), m_nMergeContactID );
		
		sWhereClause.append( _T(" AND Contacts.ContactID != ?") );
	}
	
	if ( m_nOwnerID > 0 )
	{
		BINDPARAM_LONG( GetQuery(), m_nOwnerID );
		
		sWhereClause.append( _T(" AND OwnerID = ?") );
	}

	if ( m_nState == 1 )
	{
		sWhereClause.append( _T(" AND EXISTS (SELECT PersonalDataID FROM PersonalData ")
							 _T("WHERE ContactID = Contacts.ContactID AND PersonalDataTypeID=1 AND StatusID=0)") );
	}

	if ( m_nState == 2 )
	{
		sWhereClause.append( _T(" AND EXISTS (SELECT PersonalDataID FROM PersonalData ")
							 _T("WHERE ContactID = Contacts.ContactID AND PersonalDataTypeID=1 AND StatusID>0)") );
	}

	if ( m_sContactName.length() > 0 )
	{
		m_sContactName.EscapeSQL();
		
		BINDPARAM_TCHAR( GetQuery(), (void*) m_sContactName.c_str() );

		sWhereClause.append( _T(" AND Name") );
		sWhereClause.append( m_ExactContactName ? _T("=?") : _T(" LIKE ?") );
		
		if ( !m_ExactContactName )
		{
			m_sContactName.insert( 0, _T("%") );
			m_sContactName.append( _T("%") );
		}
	}

	if ( m_sContactEmail.length() > 0 )
	{
		m_sContactEmail.EscapeSQL();
		
		BINDPARAM_TCHAR( GetQuery(), (void*) m_sContactEmail.c_str() );

		sWhereClause.append( _T(" AND EXISTS (SELECT PersonalDataID FROM PersonalData ")
							 _T("WHERE ContactID = Contacts.ContactID AND PersonalDataTypeID=1 AND DataValue") );

		sWhereClause.append( m_ExactContactEmail ? _T("=?)") : _T(" LIKE ?)") );

		if ( !m_ExactContactEmail )
		{
			m_sContactEmail.insert( 0, _T("%") );
			m_sContactEmail.append( _T("%") );
		}
	}
	
	if ( m_sCompanyName.length() > 0 )
	{
		m_sCompanyName.EscapeSQL();

		BINDPARAM_TCHAR( GetQuery(), (void*) m_sCompanyName.c_str() );
		
		sWhereClause.append( _T(" AND CompanyName") );
		sWhereClause.append( m_ExactCompanyName ? _T("=?") : _T(" LIKE ?") );

		if ( !m_ExactCompanyName )
		{
			m_sCompanyName.insert( 0, _T("%") );
			m_sCompanyName.append( _T("%") );
		}
	}

	if ( m_sWebsite.length() > 0 )
	{
		m_sWebsite.EscapeSQL();

		BINDPARAM_TCHAR( GetQuery(), (void*) m_sWebsite.c_str() );
		
		sWhereClause.append( _T(" AND WebPageAddress") );
		sWhereClause.append( m_ExactWebsite ? _T("=?") : _T(" LIKE ?") );

		if ( !m_ExactWebsite )
		{
			m_sWebsite.insert( 0, _T("%") );
			m_sWebsite.append( _T("%") );
		}
	}

	if ( m_sAddress.length() > 0 )
	{
		m_sAddress.EscapeSQL();

		BINDPARAM_TCHAR( GetQuery(), (void*) m_sAddress.c_str() );
		BINDPARAM_TCHAR( GetQuery(), (void*) m_sAddress.c_str() );

		sWhereClause.append( _T(" AND (StreetAddress1") );
		sWhereClause.append( m_ExactAddress ? _T("=?") : _T(" LIKE ?") );
		
		sWhereClause.append( _T(" OR StreetAddress2") );
		sWhereClause.append( m_ExactAddress ? _T("=?") : _T(" LIKE ?") );

		if ( !m_ExactAddress )
		{
			m_sAddress.insert( 0, _T("%") );
			m_sAddress.append( _T("%") );
		}

		sWhereClause.append( _T(")") );
	}

	if ( m_sCity.length() > 0 )
	{
		m_sCity.EscapeSQL();

		BINDPARAM_TCHAR( GetQuery(), (void*) m_sCity.c_str() );

		sWhereClause.append( _T(" AND City") );
		sWhereClause.append( m_ExactCity ? _T("=?") : _T(" LIKE ?") );
	
		if ( !m_ExactCity )
		{
			m_sCity.insert( 0, _T("%") );
			m_sCity.append( _T("%") );
		}
	}

	if ( m_sState.length() > 0 )
	{
		m_sState.EscapeSQL();

		BINDPARAM_TCHAR( GetQuery(), (void*) m_sState.c_str() );

		sWhereClause.append( _T(" AND State ") );
		sWhereClause.append( m_ExactState ? _T("=?") : _T(" LIKE ?") );
		
		if ( !m_ExactState )
		{
			m_sState.insert( 0, _T("%") );
			m_sState.append( _T("%") );
		}
	}
	
	if ( m_sCountry.length() > 0 )
	{
		m_sCountry.EscapeSQL();

		BINDPARAM_TCHAR( GetQuery(), (void*) m_sCountry.c_str() );
		
		sWhereClause.append( _T(" AND Country ") );
		sWhereClause.append( m_ExactCountry ? _T("=?") : _T(" LIKE ?") );
		
		if ( !m_ExactCountry )
		{
			m_sCountry.insert( 0, _T("%") );
			m_sCountry.append( _T("%") );
		}
	}

	if ( m_sZipCode.length() > 0 )
	{
		m_sZipCode.EscapeSQL();

		BINDPARAM_TCHAR( GetQuery(), (void*) m_sZipCode.c_str() );

		sWhereClause.append( _T(" AND ZipCode ") );
		sWhereClause.append( m_ExactZipCode ? _T("=?") : _T(" LIKE ?") );

		if ( !m_ExactZipCode )
		{
			m_sZipCode.insert( 0, _T("%") );
			m_sZipCode.append( _T("%") );
		}
	}

	if ( m_sPersonalData.length() > 0 )
	{
		m_sPersonalData.EscapeSQL();

		sWhereClause.append( _T(" AND EXISTS (SELECT PersonalDataID FROM PersonalData ")
			                 _T(" WHERE ContactID = Contacts.ContactID") );

		if ( m_nPersonalDataType > 0)
		{
			BINDPARAM_LONG( GetQuery(), m_nPersonalDataType );
			sWhereClause.append( _T(" AND PersonalDataTypeID=?") );	
		}
		
		BINDPARAM_TCHAR( GetQuery(), (void*) m_sPersonalData.c_str() );
		
		sWhereClause.append( _T(" AND PersonalData.DataValue") );
		sWhereClause.append( m_ExactPersonalData ? _T("=?") : _T(" LIKE ?") );
		sWhereClause.append( _T(" )") );
	
		if ( !m_ExactPersonalData )
		{
			m_sPersonalData.insert( 0, _T("%") );
			m_sPersonalData.append( _T("%") );
		}
	}	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Delete contacts, supports deleting multiple contacts
\*--------------------------------------------------------------------------*/
void CContactSearch::Delete()
{
	bool bRefresh = false;
	CEMSString sContactIDs;
	int nContactID;
	TIMESTAMP_STRUCT DeletedTime;

	GetTimeStamp(DeletedTime);
	
	// check security
	RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_DELETE_ACCESS );

	GetISAPIData().GetXMLString( _T("SelectID"), sContactIDs );


	while ( sContactIDs.CDLGetNextInt( nContactID ) )
	{
		GetQuery().Initialize();
		
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		BINDPARAM_TIME_NOLEN( GetQuery(), DeletedTime );
		BINDPARAM_LONG( GetQuery(), nContactID );
		
		GetQuery().Execute( _T("UPDATE Contacts SET IsDeleted=1, DeletedBy=?, DeletedTime=? WHERE ContactID=?") );
		
		if ( GetQuery().GetRowCount() != 1 )
		{
			CEMSString sError;
			sError.Format( _T("Unable to delete contact (%d). The contact does not exist."), nContactID );
			THROW_EMS_EXCEPTION( E_UpdateContact, sError );
		}
		
		GetQuery().Initialize();		
		BINDPARAM_LONG( GetQuery(), nContactID );
		GetQuery().Execute( _T("DELETE FROM AgentContacts ")
						_T("WHERE ContactID=?") );

		if ( !bRefresh )
		{
			// if we were able to delete a ticket
			// add the flag to refresh the page
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
		
		// now update the ticket contacts
		int  nTicketID;
		list<int> TicketIDList;
		
		GetQuery().Initialize();
		
		BINDCOL_LONG_NOLEN( GetQuery(), nTicketID );
		BINDPARAM_LONG( GetQuery(), nContactID );
		
		GetQuery().Execute( _T("SELECT TicketID FROM TicketContacts WHERE ContactID=?") );
		
		while ( GetQuery().Fetch() == S_OK )
		{
			TicketIDList.push_back(nTicketID);
		}
		
		list<int>::iterator iter;
		
		for ( iter = TicketIDList.begin(); iter != TicketIDList.end(); iter++ )
		{
			BuildTicketContacts( GetQuery(), *iter );
		}

		// skip the next two items... this is a hack because selectID is posted
		// as ContactID, Name, Email...
		sContactIDs.CDLGetNextInt( nContactID );
		sContactIDs.CDLGetNextInt( nContactID );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Remove contacts from MyContacts, supports removing multiple contacts
\*--------------------------------------------------------------------------*/
void CContactSearch::Remove()
{
	CEMSString sQuery;
	CEMSString sContactIDs;
	int nContactID;
	bool bRefresh = false;
		
	// check security
	m_nAgentID ? RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_nAgentID, EMS_EDIT_ACCESS ) : RequireAdmin();

	GetISAPIData().GetXMLString( _T("selectID"), sContactIDs );

	while ( sContactIDs.CDLGetNextInt( nContactID ) )
	{
		GetQuery().Initialize();
		sQuery.Format( _T("DELETE FROM AgentContacts WHERE ContactID = %d AND AgentID = %d"), nContactID, m_nAgentID );
		GetQuery().Execute( sQuery.c_str() );

		if ( !bRefresh )
		{
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Reset contact status, supports resetting multiple contacts
\*--------------------------------------------------------------------------*/
void CContactSearch::Reset()
{
	CEMSString sQuery;
	CEMSString sContactIDs;
	int nContactID;
	bool bRefresh = false;
		
	// check security
	m_nAgentID ? RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_nAgentID, EMS_EDIT_ACCESS ) : RequireAdmin();

	GetISAPIData().GetXMLString( _T("selectID"), sContactIDs );

	while ( sContactIDs.CDLGetNextInt( nContactID ) )
	{
		GetQuery().Initialize();
		sQuery.Format( _T("UPDATE PersonalData SET StatusID=0 WHERE ContactID = %d AND PersonalDataTypeID=1"), nContactID );
		GetQuery().Execute( sQuery.c_str() );

		if ( !bRefresh )
		{
			bRefresh = true;
			GetXMLGen().AddChildElem( _T("Refresh") );
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add contacts to MyContacts
\*--------------------------------------------------------------------------*/
void CContactSearch::AddMyContact()
{
	CEMSString sQuery;
	CEMSString sContactIDs;
	int nContactID;
	bool bRefresh = false;
	
	// check security
	m_nAgentID ? RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_nAgentID, EMS_EDIT_ACCESS ) : RequireAdmin();

	GetISAPIData().GetXMLString( _T("selectID"), sContactIDs );

	while ( sContactIDs.CDLGetNextInt( nContactID ) )
	{
		if(nContactID > 0)
		{
			GetQuery().Initialize();	
			BINDPARAM_LONG( GetQuery(), m_nAgentID);
			BINDPARAM_LONG( GetQuery(), nContactID);		
			GetQuery().Execute( _T("SELECT AgentContactID FROM AgentContacts WHERE AgentID = ? AND ContactID = ?"));		
			if ( GetQuery().Fetch() == S_OK )
			{
				// The contact is already in MyContacts
			}
			else
			{
				GetQuery().Reset(false);
				GetQuery().Execute( _T("INSERT INTO AgentContacts (AgentID, ContactID) VALUES(?, ?)"));
				if ( !bRefresh )
				{
					bRefresh = true;
					GetXMLGen().AddChildElem( _T("Refresh") );
				}
			}
		}
	}
}