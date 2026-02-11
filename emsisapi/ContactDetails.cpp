/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ContactDetails.cpp,v 1.1.4.1 2005/12/13 18:11:50 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "listclasses.h"
#include "DateFns.h"
#include "ContactFns.h"
#include "ContactDetails.h"
#include "ContactGroups.h"
#include "EMSMutex.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CContactDetails::CContactDetails(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{
	// init vars
	m_AccessLevel = EMS_NO_ACCESS;
	m_nMaxPersonalDataID = 0;
	m_nMaxFormPersonalDataID = 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CContactDetails::Run( CURLAction& action )
{
	tstring sAction;
	tstring sID;	
	
	GetISAPIData().GetURLLong( _T("contact"), m_Contact.m_ContactID, true );
	if ( GetISAPIData().GetFormTCHAR( _T("email"), m_Contact.m_Name,256, true) == false)
		GetISAPIData().GetURLTCHAR( _T("email"), m_Contact.m_Name,256, true);
	
	if (_tcslen( m_Contact.m_Name ) == 0)
	{
		if (m_ISAPIData.m_pSession->m_TempType == 1)
		{
			_tcscpy( m_Contact.m_Name,  m_ISAPIData.m_pSession->m_TempData.c_str() );
			GetISAPIData().m_SessionMap.SetTempType( m_ISAPIData.m_pSession->m_AgentID, 1 );
			GetISAPIData().m_SessionMap.SetTempData( m_ISAPIData.m_pSession->m_AgentID, "" );			
		}
	}

	GetISAPIData().GetXMLPost() ? GetISAPIData().GetXMLString( _T("Action"), sAction, true ) :
	                              GetISAPIData().GetFormString( _T("Action"), sAction, true );

	if (sAction.compare(_T("merge")) == 0)
	{
		Merge();
	}
	else if( m_Contact.m_ContactID == 0 ) 
	{
		if (_tcslen( m_Contact.m_Name ) > 0)
		{
			if ((m_Contact.m_ContactID = FindContactByEmail( GetQuery(), m_Contact.m_Name)) == 0 )
			{
				//CEMSString sError;
				//sError.Format( _T("Error - Email address [%s] not found!\n"), m_Contact.m_Name );
				//THROW_EMS_EXCEPTION( E_InvalidID, sError );
				m_AccessLevel = RequireAgentRightLevel(EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS);
				action.m_sPageTitle.assign( _T("New Contact") );
				GenNewContactXML();
			}
			else
			{
				Query();
			}
			return 0;
		}
		else if (sAction.compare(_T("insert")) == 0)
		{
			Insert();	
		}
		else
		{
			m_AccessLevel = RequireAgentRightLevel(EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS);
		}

		action.m_sPageTitle.assign( _T("New Contact") );

		GenNewContactXML();
	}
	else 
	{
		if (sAction.compare(_T("update")) == 0)
		{
			Update();
		}
		else
		{
			Query();
		}
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates the database for an existing contact              
\*--------------------------------------------------------------------------*/
void CContactDetails::Query()
{
	// check security
	m_AccessLevel = RequireAgentRightLevel(EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS);	
	
	// query for the contact details
	if ( m_Contact.Query(GetQuery()) != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Error - Invalid ContactID %d\n"), m_Contact.m_ContactID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError ); 
	}

	// list the personal data
	CPersonalDataList DataItem;	
	DataItem.ListContact( GetQuery(), m_Contact.m_ContactID );

	m_nMaxPersonalDataID = 0;

	while( ( GetQuery().Fetch() ) == S_OK )
	{
		if (DataItem.m_PersonalDataID != m_Contact.m_DefaultEmailAddressID)
		{
			m_PersonalDataMap[DataItem.m_PersonalDataID] = DataItem;

			if (DataItem.m_PersonalDataID > m_nMaxPersonalDataID)
				m_nMaxPersonalDataID = DataItem.m_PersonalDataID + 1;
		}
		else
		{
			m_DefaultEmail = DataItem;
		}
	}

	// Now query for the groups
	TContactGroups cGroup;	
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), cGroup.m_ContactGroupID );
	BINDCOL_TCHAR( GetQuery(), cGroup.m_GroupName );
	BINDPARAM_LONG( GetQuery(), m_Contact.m_ContactID );
	GetQuery().Execute( _T("SELECT g.ContactGroupID,g.GroupName ")
						_T("FROM ContactGroups g ")
						_T("INNER JOIN ContactGrouping cg on g.ContactGroupID=cg.ContactGroupID ")
						_T("INNER JOIN Contacts c on cg.ContactID=c.ContactID ")
						_T("WHERE c.ContactID =? ")
						_T("ORDER BY g.GroupName") );
	
	GetXMLGen().AddChildElem( _T("Groups") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Group") );
		GetXMLGen().AddChildAttrib( _T("ID"), cGroup.m_ContactGroupID );
		GetXMLGen().AddChildAttrib( _T("Name"), cGroup.m_GroupName );
	}
	GetXMLGen().OutOfElem();
	
	// generate the XML
	GenContactXML();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates XML for the contact	              
\*--------------------------------------------------------------------------*/
void CContactDetails::GenContactXML()
{
	CEMSString sDateCreated;
	int NoteCount = 0;

	GetDateTimeString( m_Contact.m_DateCreated, m_Contact.m_DateCreatedLen, sDateCreated);	
	
	GetXMLGen().AddChildElem(_T("Contact"));
	GetXMLGen().AddChildAttrib( _T("AccessLevel"), m_AccessLevel);
	GetXMLGen().AddChildAttrib(_T("ID"), m_Contact.m_ContactID);
	GetXMLGen().AddChildAttrib(_T("DateCreated"), sDateCreated.c_str());
	GetXMLGen().AddChildAttrib(_T("IsDeleted"), m_Contact.m_IsDeleted);
	GetXMLGen().AddChildAttrib(_T("Owner"), m_Contact.m_OwnerID);
	
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), NoteCount );
	BINDPARAM_LONG( GetQuery(), m_Contact.m_ContactID );
	GetQuery().Execute( _T("SELECT COUNT(*) FROM ContactNotes WHERE ContactID=?") );
	GetQuery().Fetch();

	GetXMLGen().AddChildAttrib( _T("NoteCount"), NoteCount );
	

	GetXMLGen().IntoElem();
	
	GetXMLGen().AddChildElem(_T("Title"), m_Contact.m_Title);
	GetXMLGen().AddChildElem(_T("Name"), m_Contact.m_Name);
	GetXMLGen().AddChildElem(_T("Company"), m_Contact.m_CompanyName);
	GetXMLGen().AddChildElem(_T("StreetAddress1"), m_Contact.m_StreetAddress1);
	GetXMLGen().AddChildElem(_T("StreetAddress2"), m_Contact.m_StreetAddress2);
	GetXMLGen().AddChildElem(_T("City"), m_Contact.m_City);
	GetXMLGen().AddChildElem(_T("State"), m_Contact.m_State);
	GetXMLGen().AddChildElem(_T("Zip"), m_Contact.m_ZipCode);
	GetXMLGen().AddChildElem(_T("Country"), m_Contact.m_Country);
	GetXMLGen().AddChildElem(_T("Website"), m_Contact.m_WebPageAddress);
	
	GetXMLGen().AddChildElem( _T("DefaultEmailAddress"), m_DefaultEmail.m_DataValue );
	GetXMLGen().AddAttrib( _T("PersonalDataMaxID"), m_nMaxPersonalDataID);

	map<int, CPersonalDataList>::iterator DataIter;
	
	for ( DataIter = m_PersonalDataMap.begin(); DataIter != m_PersonalDataMap.end(); DataIter++)
	{
		GetXMLGen().AddChildElem( _T("PersonalData") );
		GetXMLGen().AddChildAttrib( _T("ID"), DataIter->second.m_PersonalDataID );
		GetXMLGen().AddChildAttrib( _T("TypeID"), DataIter->second.m_PersonalDataTypeID);
		GetXMLGen().AddChildAttrib( _T("Value"), DataIter->second.m_DataValue);
		GetXMLGen().AddChildAttrib( _T("Note"), DataIter->second.m_Note);
	}
		
	GetXMLGen().OutOfElem();

	// always list the personal data types
	ListPersonalDataTypes();
	
	// always list the agent names
	ListEnabledAgentNames( EMS_EDIT_ACCESS );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates the necessary XML for the new contacts dialog	              
\*--------------------------------------------------------------------------*/
void CContactDetails::GenNewContactXML()
{
	GetXMLGen().AddChildElem(_T("Contact"));
	GetXMLGen().AddChildAttrib( _T("AccessLevel"), m_AccessLevel);
	GetXMLGen().AddChildAttrib(_T("ID"), _T("0"));
	if (_tcslen( m_Contact.m_Name ) > 0)
	{
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("DefaultEmailAddress"), m_Contact.m_Name );
		GetXMLGen().OutOfElem();
	}
	// always list the personal data types
	ListPersonalDataTypes();
	
	// always list the agent names
	ListEnabledAgentNames( EMS_EDIT_ACCESS );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Creates a new contact	              
\*--------------------------------------------------------------------------*/
void CContactDetails::Insert()
{
	try
	{
		// check security
		m_AccessLevel = RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS );
		
		// read form paramaters
		DecodeForm();

		// lock contacts
		CCreateContactMutex lock;
		
		if (!lock.AcquireLock( 1000 ))
		{
			// throw an exception, we couldn't get the lock
			THROW_EMS_EXCEPTION( E_CreateContact, _T("Unable to aquire contact lock"));
		}

		// make sure that none of the email addresses are already in use
		VerifyUniqueEmailAddr();
		
		// create a deleted contact record
		GetTimeStamp(m_Contact.m_DateCreated);
		m_Contact.m_DateCreatedLen = sizeof(m_Contact.m_DateCreated);
		m_Contact.m_IsDeleted = EMS_DELETE_OPTION_DELETE_PERMANENTLY;
		m_Contact.Insert(GetQuery());
		
		// create the default email address record
		m_DefaultEmail.m_ContactID = m_Contact.m_ContactID;
		m_DefaultEmail.Insert( GetQuery() );
		int nDefaultEmailID = m_DefaultEmail.m_PersonalDataID;
		
		// add additional personal data records
		map<int, CPersonalDataList>::iterator DataIter;
		
		for ( DataIter = m_PersonalDataMap.begin(); DataIter != m_PersonalDataMap.end(); DataIter++)
		{
			DataIter->second.m_ContactID = m_Contact.m_ContactID;
			DataIter->second.Insert(GetQuery());
			
			// do we need to bump the MaxPersonalDataID
			if (m_nMaxPersonalDataID <= DataIter->second.m_PersonalDataID)
				m_nMaxPersonalDataID = DataIter->second.m_PersonalDataID + 1;
		}
		
		// update the contacts record with the ID of the default
		// email addresss, remove the IsDeleted flag.
		if ( CommitContact( GetQuery(), m_Contact.m_ContactID, nDefaultEmailID ) != 1)
		{
			CEMSString sError;
			sError.Format( _T("Unable to commit contact %d"), m_Contact.m_ContactID );
			THROW_EMS_EXCEPTION( E_CreateContact, sError ); 
		}
		
	}
	catch(...)
	{
		// an error occured, return the form params as XML
		m_Contact.m_ContactID = 0;
		m_nMaxPersonalDataID = m_nMaxFormPersonalDataID;
		GenContactXML();
		throw;
	}
	
	// dump back limited details... requested by Geoff
	GetXMLGen().AddChildElem(_T("NewContact"));
	GetXMLGen().AddChildAttrib(_T("ID"), m_Contact.m_ContactID);
	GetXMLGen().AddChildAttrib(_T("Name"), m_Contact.m_Name);
	GetXMLGen().AddChildAttrib(_T("DefaultEmailAddress"), m_DefaultEmail.m_DataValue);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Updates an existing contact	              
\*--------------------------------------------------------------------------*/
void CContactDetails::Update()
{
	try
	{
		// check security
		m_AccessLevel = RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS );
		
		// storage
		set<int> DBIDs;
		list<int> DeleteIDs;
		
		// decode the form
		DecodeForm();
		
		// lock contacts
		CCreateContactMutex lock;
		
		if (!lock.AcquireLock( 1000 ))
		{
			// throw an exception, we couldn't get the lock
			THROW_EMS_EXCEPTION( E_CreateContact, _T("Unable to aquire contact lock"));
		}
		
		// make sure that none of the email addresses already exsist
		VerifyUniqueEmailAddr( m_Contact.m_ContactID );

		// get the default email address
		m_Contact.m_DefaultEmailAddressID = GetDefaultEmailAddress();

		// update the contact
		if (m_Contact.Update(GetQuery()) != 1)
		{
			CEMSString sError;
			sError.Format( _T("Invalid ContactID %d\n"), m_Contact.m_ContactID );
			THROW_EMS_EXCEPTION( E_UpdateContact, sError ); 
		}

		// update the default email address
		m_DefaultEmail.m_ContactID = m_Contact.m_ContactID;
		m_DefaultEmail.m_PersonalDataID = m_Contact.m_DefaultEmailAddressID;
		
		if ( m_DefaultEmail.Update( GetQuery() ) != 1)
		{
			CEMSString sError;
			sError.Format( _T("Unable to update default email address %d\n"), m_DefaultEmail.m_PersonalDataID );
			THROW_EMS_EXCEPTION( E_UpdateContact, sError ); 
		}
	
		// get a list of the personal data items associated
		// with this contact that are currently in the database
		CPersonalDataList DataItem;	
		DataItem.ListContact( GetQuery(), m_Contact.m_ContactID );
		
		// determine which ID's we need to delete
		while(GetQuery().Fetch() == S_OK)
		{
			DBIDs.insert(DataItem.m_PersonalDataID);

			if ( DataItem.m_PersonalDataID != m_Contact.m_DefaultEmailAddressID )
			{
				// do we need to bump the MaxPersonalDataID
				if (m_nMaxFormPersonalDataID <= DataItem.m_PersonalDataID)
					m_nMaxFormPersonalDataID = DataItem.m_PersonalDataID + 1;

				// mark items that are in the database, however not in the map for deletion
				if  ( m_PersonalDataMap.find(DataItem.m_PersonalDataID) == m_PersonalDataMap.end() )
					DeleteIDs.push_back(DataItem.m_PersonalDataID);
			}
		}

		// delete the items in the database that we couldn't find in our list
		list<int>::iterator iterDel;
		
		for( iterDel = DeleteIDs.begin(); iterDel != DeleteIDs.end(); iterDel++ )
		{	
			DataItem.m_PersonalDataID = *iterDel;

			// TODO - we could delete all the records at once with an IN statement
		
			if ( DataItem.Delete( GetQuery() ) != 1)
			{
				CEMSString sError;
				sError.Format(_T("Unable to delete personal data record: %d\n"), DataItem.m_PersonalDataID);
				THROW_EMS_EXCEPTION( E_UpdateContact, sError );
			}
		}

		// now either update or add each item
		map<int, CPersonalDataList>::iterator DataIter;
		
		for ( DataIter = m_PersonalDataMap.begin(); DataIter != m_PersonalDataMap.end(); DataIter++)
		{
			if( DataIter->first < m_nMaxPersonalDataID && DBIDs.find( DataIter->first) != DBIDs.end())
			{
				if ( DataIter->second.Update( GetQuery() ) != 1 )
				{
					CEMSString sError;
					sError.Format( _T("Unable to update personal data record: %d\n"), DataIter->second.m_PersonalDataID );
					THROW_EMS_EXCEPTION( E_UpdateContact, sError ); 
				}
			}
			else
			{
				DataIter->second.Insert( GetQuery() );
				
				// do we need to bump the MaxPersonalDataID
				if (m_nMaxPersonalDataID <= DataIter->second.m_PersonalDataID)
					m_nMaxPersonalDataID = DataIter->second.m_PersonalDataID + 1;
			}
		}
		
		// rebuild the contact string for tickets which include this contact
		list<int> TicketIDs;
		int nTicketID;
		
		GetQuery().Initialize();
		
		BINDCOL_LONG_NOLEN( GetQuery(), nTicketID ); 
		BINDPARAM_LONG( GetQuery(), m_Contact.m_ContactID );
		
		GetQuery().Execute( _T("SELECT TicketContacts.TicketID FROM TicketContacts ")
			                _T("INNER JOIN Tickets ON TicketContacts.TicketID = Tickets.TicketID ")
			                _T("WHERE Tickets.IsDeleted = 0 AND ContactID = ?") );
		
		while ( GetQuery().Fetch() == S_OK )
		{
			TicketIDs.push_back(nTicketID);
		}
		
		list<int>::iterator iter;
		
		for ( iter = TicketIDs.begin(); iter != TicketIDs.end(); iter++ )
		{
			BuildTicketContacts( GetQuery(), *iter );
		}
	}
	catch(...)
	{
		// an error occured, return the form params as XML
		m_nMaxPersonalDataID = m_nMaxFormPersonalDataID;
		GenContactXML();	
		throw;
	}

	// success - dump out the XML
	// this may not be needed if contact details is
	// only displayrd in a modal window.
	GenContactXML();

	// success - dump back limited details... requested by Geoff
	GetXMLGen().AddChildElem(_T("NewContact"));
	GetXMLGen().AddChildAttrib(_T("ID"), m_Contact.m_ContactID);
	GetXMLGen().AddChildAttrib(_T("Name"), m_Contact.m_Name);
	GetXMLGen().AddChildAttrib(_T("DefaultEmailAddress"), m_DefaultEmail.m_DataValue);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Decodes the form parameters	              
\*--------------------------------------------------------------------------*/
void CContactDetails::DecodeForm()
{
	GetISAPIData().GetXMLTCHAR(_T("ADDRESS1"), m_Contact.m_StreetAddress1, 76);
	GetISAPIData().GetXMLTCHAR(_T("ADDRESS2"), m_Contact.m_StreetAddress2, 76);
	GetISAPIData().GetXMLTCHAR(_T("CITY"), m_Contact.m_City, 36);
	GetISAPIData().GetXMLTCHAR(_T("STATE"), m_Contact.m_State, 31);
	GetISAPIData().GetXMLTCHAR(_T("ZIP"), m_Contact.m_ZipCode, 21);
	GetISAPIData().GetXMLTCHAR(_T("COUNTRY"), m_Contact.m_Country, 56);
	GetISAPIData().GetXMLTCHAR(_T("FULLNAME"), m_Contact.m_Name, 256);
	GetISAPIData().GetXMLTCHAR(_T("TITLE"), m_Contact.m_Title, 21);
	GetISAPIData().GetXMLTCHAR(_T("COMPANY"), m_Contact.m_CompanyName, 51);
	GetISAPIData().GetXMLTCHAR(_T("WEBSITE"), m_Contact.m_WebPageAddress, 256);
	GetISAPIData().GetXMLLong(_T("OWNERID"), m_Contact.m_OwnerID);
	
	GetISAPIData().GetXMLTCHAR(_T("DEFAULTEMAIL"), m_DefaultEmail.m_DataValue, 256);
	m_DefaultEmail.m_PersonalDataTypeID = EMS_PERSONAL_DATA_EMAIL;
	
	GetISAPIData().GetXMLLong( _T("PERSONALDATAMAXID"), m_nMaxPersonalDataID );
	
	// validate the default email address
	if ( _tcslen( m_DefaultEmail.m_DataValue ) != 0 )
	{
		CEMSString sString;
		sString.assign(m_DefaultEmail.m_DataValue);
		
		if (!sString.ValidateEmailAddr())
		{
			CEMSString sError;
			sError.Format( _T("The email address specified [%s] is invalid"), sString.c_str() );
			THROW_VALIDATION_EXCEPTION( _T("DefaultEmail"), sError );
		}
	}
	else if ( _tcslen( m_Contact.m_Name ) == 0 )
	{
		THROW_VALIDATION_EXCEPTION( _T("DefaultEmail"), CEMSString( _T("The default email address or full name must be specified") ) );
	}
	
	// build a map containing the personal data fields
	CEMSString sParam;
	CEMSString sCollection;
	int nID;
	
	CPersonalDataList Data;
	Data.m_AgentID = 0;
	Data.m_ContactID = m_Contact.m_ContactID;
	
	GetISAPIData().GetXMLString( _T("PERSONALDATAIDCOLLECTION"), sCollection );
	
	while ( sCollection.CDLGetNextInt( nID ) )
	{
		// decode form paramaters
		sParam.Format( _T("PERSONALDATATYPE%d"), nID);
		GetISAPIData().GetXMLLong( sParam.c_str(), Data.m_PersonalDataTypeID );
		
		sParam.Format( _T("PERSONALDATA%d"), nID);
		GetISAPIData().GetXMLTCHAR( sParam.c_str(), Data.m_DataValue, sizeof(Data.m_DataValue) );
		
		// save the personal data item in the map
		if ( _tcslen( Data.m_DataValue ) > 0 )
		{
			// validate that the syntax of any addition email addresses is valid
			if ( Data.m_PersonalDataTypeID == EMS_PERSONAL_DATA_EMAIL )
			{
				CEMSString sString;
				sString.assign(Data.m_DataValue);

				if (!sString.ValidateEmailAddr())
				{
					CEMSString sField, sError;
					sField.Format( _T("PersonalData%d"), nID );
					sError.Format( _T("The email address specified [%s] is invalid"), sString.c_str() );
					THROW_VALIDATION_EXCEPTION( sField.c_str(), sError );
				}
			}

			Data.m_PersonalDataID = nID;
			m_PersonalDataMap[nID] = Data;
		}
		
		// calculate MaxFormPersonalDataID
		if (nID >= m_nMaxFormPersonalDataID)
			m_nMaxFormPersonalDataID = nID + 1;		
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Verifies that all of the email addresses posted 
||				are unique.      
\*--------------------------------------------------------------------------*/
void CContactDetails::VerifyUniqueEmailAddr( int nContactID /* =0 */)
{	
	map<int, CPersonalDataList>::iterator DataIter;
	int nID;
	
	// make sure that the same email addresses hasn't been specified more 
	// then once for the contact...
	for ( DataIter = m_PersonalDataMap.begin(); DataIter != m_PersonalDataMap.end(); DataIter++)
	{
		// make sure that it doesn't match the default email address
		if ( _tcscmp(  m_DefaultEmail.m_DataValue, DataIter->second.m_DataValue ) == 0 )
		{
			CEMSString sField, sError;
			sField.Format( _T("PersonalData%d"), DataIter->second.m_PersonalDataID );
			sError.Format( _T("The email address %s is already in use"), DataIter->second.m_DataValue );
			THROW_VALIDATION_EXCEPTION( sField.c_str(), sError );
		}
		
		// make sure that it doesn't match any other "additional information" email addresses
		if ( DataIter->second.m_PersonalDataTypeID == EMS_PERSONAL_DATA_EMAIL )
		{
			map<int, CPersonalDataList>::iterator Iter;

			for ( Iter = m_PersonalDataMap.begin(); Iter != m_PersonalDataMap.end(); Iter++)
			{
				if ( Iter != DataIter && Iter->second.m_PersonalDataTypeID == EMS_PERSONAL_DATA_EMAIL )
				{	
					if ( _tcscmp( Iter->second.m_DataValue, DataIter->second.m_DataValue ) == 0 )
					{
						CEMSString sField, sError;
						sField.Format( _T("PersonalData%d"), DataIter->second.m_PersonalDataID );
						sError.Format( _T("The email address %s is already in use"), DataIter->second.m_DataValue );
						THROW_VALIDATION_EXCEPTION( sField.c_str(), sError );
					}
				}
			}
		}
	}

	// check the default email address
	if ((nID = FindContactByEmail( GetQuery(), m_DefaultEmail.m_DataValue)) != 0)
	{
		if ((nContactID == 0) || (nID != nContactID))
		{
			CEMSString sError;
			sError.Format( _T("The email address %s is already in use by another contact"), m_DefaultEmail.m_DataValue );
			THROW_VALIDATION_EXCEPTION( _T("DefaultEmail"), sError );
		}
	}
	
	// check the rest of the email addresses
	for ( DataIter = m_PersonalDataMap.begin(); DataIter != m_PersonalDataMap.end(); DataIter++)
	{
		if (DataIter->second.m_PersonalDataTypeID == EMS_PERSONAL_DATA_EMAIL)
		{
			if ((nID = FindContactByEmail( GetQuery(), DataIter->second.m_DataValue)) != 0 )
			{
				if ((nContactID == 0) || (nID != nContactID))
				{
					CEMSString sField, sError;
					sField.Format( _T("PersonalData%d"), DataIter->second.m_PersonalDataID );
					sError.Format( _T("The email address %s is already in use by another contact"), DataIter->second.m_DataValue );
					THROW_VALIDATION_EXCEPTION( sField.c_str(), sError );
				}
			}
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the DefaultEmailAddressID for the contact	              
\*--------------------------------------------------------------------------*/
int CContactDetails::GetDefaultEmailAddress(void)
{
	int nDefaultEmailAddressID;

	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), nDefaultEmailAddressID );
	BINDPARAM_LONG( GetQuery(), m_Contact.m_ContactID );

	GetQuery().Execute( _T("SELECT DefaultEmailAddressID ")
		                _T("FROM Contacts ")
		                _T("WHERE ContactID = ?") );

	if (GetQuery().Fetch() != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("Error - Invalid ContactID %d\n"), m_Contact.m_ContactID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	return nDefaultEmailAddressID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Merges two contacts	              
\*--------------------------------------------------------------------------*/
void CContactDetails::Merge( void )
{
	// edit access is required to merge contacts
	RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS );

	CEMSString sSelectID;
	int nSourceID;
	int nDestID;

	GetISAPIData().GetFormString( _T("SELECTID"), sSelectID );
	
	if (! sSelectID.CDLGetNextInt( nSourceID ) )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("A Source ContactID must be specified") );

	if (! sSelectID.CDLGetNextInt( nDestID) )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("A Destination ContactID must be specified") );


	// build a list of TicketIDs that reference the source contact 
	list<int> TicketIDs;
	int nTicketID;

	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), nTicketID ); 
	BINDPARAM_LONG( GetQuery(), nSourceID );
	
	GetQuery().Execute( _T("SELECT TicketContacts.TicketID FROM TicketContacts ")
		                _T("INNER JOIN Tickets ON TicketContacts.TicketID = Tickets.TicketID ")
		                _T("WHERE Tickets.IsDeleted = 0 AND ContactID = ?") );
	
	while ( GetQuery().Fetch() == S_OK )
	{
		TicketIDs.push_back(nTicketID);
	}
	
	// update TicketContacts
	GetQuery().Initialize();
	
	BINDPARAM_LONG( GetQuery(), nDestID );
	BINDPARAM_LONG( GetQuery(), nSourceID );
	BINDPARAM_LONG( GetQuery(), nDestID );
	
	// prevents the contact from being added to a ticket which the contact is already a part of...
	GetQuery().Execute( _T("UPDATE TicketContacts SET ContactID = ? WHERE ContactID = ? ")
		                _T("AND TicketID NOT IN (SELECT TicketID FROM TicketContacts WHERE ContactID = ?)" ) );

	// delete any records which are left over ( where the destination contact was already part of the ticket )
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nSourceID );
	GetQuery().Execute( _T("DELETE FROM TicketContacts WHERE ContactID = ? ") );
	
	// update outbound message contacts
	GetQuery().Initialize();
	
	BINDPARAM_LONG( GetQuery(), nDestID );
	BINDPARAM_LONG( GetQuery(), nSourceID );
	BINDPARAM_LONG( GetQuery(), nDestID );
	
	// prevents the contact from being added to an outbound message which the contact is already a part of...
	GetQuery().Execute( _T("UPDATE OutboundMessageContacts SET ContactID = ? WHERE ContactID = ? " )
		                _T("AND OutboundMessageID NOT IN (SELECT OutboundMessageID FROM OutboundMessageContacts WHERE ContactID = ?)") );
	
	// delete any records which are left over ( where the destination contact was already part of the outbound message )
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nSourceID );
	GetQuery().Execute( _T("DELETE FROM OutboundMessageContacts WHERE ContactID = ? ") );


	// rebuild the contact display string for each ticket that
	// referenced the source contact
	list<int>::iterator iter;
	
	for ( iter = TicketIDs.begin(); iter != TicketIDs.end(); iter++ )
	{
		BuildTicketContacts( GetQuery(), *iter );
	}
	
	// update email personal data records
	GetQuery().Initialize();
	
	int nPersonalDataType = EMS_PERSONAL_DATA_EMAIL;
	
	BINDPARAM_LONG( GetQuery(), nDestID );
	BINDPARAM_LONG( GetQuery(), nSourceID );
	BINDPARAM_LONG( GetQuery(), nPersonalDataType );
	
	GetQuery().Execute( _T("Update PersonalData SET ContactID = ? ")
		                _T("WHERE ContactID = ? AND PersonalDataTypeID = ?" ) );
	
	// delete the source contact
	GetQuery().Initialize();

	TIMESTAMP_STRUCT time;
	GetTimeStamp(time);

	BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
	BINDPARAM_TIME_NOLEN( GetQuery(), time );
	BINDPARAM_LONG( GetQuery(), nSourceID );
	
	GetQuery().Execute( _T("Update Contacts SET IsDeleted=1,DeletedBy=?,DeletedTime=? WHERE ContactID = ?") );

	// add XML with merge details
	GetXMLGen().AddChildElem( _T("MERGE") );
	GetXMLGen().AddChildAttrib( _T("SOURCE"), nSourceID );
	GetXMLGen().AddChildAttrib( _T("DESTINATION"), nDestID );

	// view the destination contact
	m_Contact.m_ContactID = nDestID;

	Query();
}