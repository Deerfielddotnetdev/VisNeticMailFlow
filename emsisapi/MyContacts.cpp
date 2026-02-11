#include "stdafx.h"
#include ".\MyContacts.h"

MyContacts::MyContacts(CISAPIData& ISAPIData)
	:CXMLDataClass(ISAPIData)
	,_AgentID(0)
{
}

MyContacts::~MyContacts(void)
{
}

int MyContacts::Run(CURLAction& action)
{
	GetISAPIData().GetURLLong( _T("AgentID"), _AgentID );

	tstring sAction;
	GetISAPIData().GetXMLString( _T("action"), sAction, true );

	if ( sAction.compare( _T("insert") ) == 0 )
	{
		AddToContacts(action);
	}
	else if ( sAction.compare( _T("delete") ) == 0 )
	{
		DeleteFromContacts(action);
	}
	else
	{
		ListContacts(action);
	}

	return 0;
}

void MyContacts::AddToContacts(CURLAction& action)
{
	// check security
	if(_AgentID)
	{
		RequireAgentRightLevel(EMS_OBJECT_TYPE_AGENT, _AgentID, EMS_EDIT_ACCESS);
	}
	else
	{
		DISABLE_IN_DEMO();
		RequireAdmin();
	}
	
	dca::String255 sEmail;
	ZeroMemory(sEmail, 256);

	GetISAPIData().GetXMLTCHAR( _T("txtEmail"), sEmail, 255);
	int nContactID = 0;

	if(lstrlen(sEmail))
	{
		//Check to see if the contact exists
		GetQuery().Initialize();
		BINDPARAM_TCHAR( GetQuery(), sEmail);
		BINDCOL_LONG_NOLEN( GetQuery(), nContactID);
		GetQuery().Execute( _T("SELECT PersonalData.ContactID FROM Contacts,PersonalData WHERE PersonalData.DataValue=? ")
							_T("AND PersonalData.PersonalDataTypeID=1 AND Contacts.IsDeleted=0 AND PersonalDataID=Contacts.DefaultEmailAddressID") );
		
		if ( GetQuery().Fetch() == S_OK )
		{
			GetQuery().Initialize();	
			BINDPARAM_LONG( GetQuery(), _AgentID);
			BINDPARAM_LONG( GetQuery(), nContactID);
			
			GetQuery().Execute( _T("SELECT AgentContactID FROM AgentContacts WHERE AgentID = ? AND ContactID = ?"));
			
			if ( GetQuery().Fetch() == S_OK )
			{
				THROW_VALIDATION_EXCEPTION( _T("Email Address"), _T("The email address you are attempting to add already exists in your MyContacts!") );
			}
			else
			{
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), _AgentID);
				BINDPARAM_LONG( GetQuery(), nContactID);
				
				GetQuery().Execute( _T("INSERT INTO AgentContacts (AgentID, ContactID) VALUES(?, ?)"));
			}
		}
		else
		{
			THROW_VALIDATION_EXCEPTION( _T("Email Address"), _T("The email address you are attempting to add does not exist in the Contacts database!") );
		}		
	}
}

void MyContacts::ListContacts(CURLAction& action)
{
	// check security
	unsigned char access;

	if(_AgentID)
	{
		access = RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, _AgentID, EMS_READ_ACCESS );
	}
	else
	{
		RequireAdmin();		
	}

	TCHAR Name[PERSONALDATA_DATAVALUE_LENGTH];
	long NameLen;
	TCHAR Email[PERSONALDATA_DATAVALUE_LENGTH];
	long EmailLen;
	int nAgentID = GetAgentID();
	int nAgentContactID;
	int nContactID;
	 
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), _AgentID );
	BINDCOL_LONG_NOLEN( GetQuery(), nAgentContactID );
	BINDCOL_TCHAR( GetQuery(), Name );
	BINDCOL_LONG_NOLEN( GetQuery(), nContactID );
	BINDCOL_TCHAR( GetQuery(), Email );
	
	GetQuery().Execute( _T("SELECT AgentContacts.AgentContactID,Contacts.Name,Contacts.ContactID,(SELECT DataValue FROM PersonalData WHERE PersonalData.PersonalDataID = Contacts.DefaultEmailAddressID ) as DataValue ")
		                _T("FROM AgentContacts INNER JOIN Contacts ON AgentContacts.ContactID = Contacts.ContactID ")
						_T("WHERE AgentContacts.AgentID=? AND Contacts.IsDeleted=0 Order By DataValue") );
	
	
	GetXMLGen().AddChildElem( _T("Contacts") );
	GetXMLGen().IntoElem();
	while ( GetQuery().Fetch() == S_OK )
	{
		CEMSString sName;
		sName.reserve( PERSONALDATA_DATAVALUE_LENGTH );
		sName.assign( Name );
		if(!sName.IsStringValid())
		{
			sName.Format(_T(""));
		}
				
		GetXMLGen().AddChildElem( _T("Contact") );
		GetXMLGen().AddChildAttrib( _T("ID"), nAgentContactID );
		GetXMLGen().AddChildAttrib( _T("CID"), nContactID );
		GetXMLGen().AddChildAttrib( _T("Name"), sName.c_str() );
		GetXMLGen().AddChildAttrib( _T("Email"), Email );
		
	}
	GetXMLGen().OutOfElem();
}

void MyContacts::DeleteFromContacts(CURLAction& action)
{
	// check security
	_AgentID ? RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, _AgentID, EMS_EDIT_ACCESS ) : RequireAdmin();

	CEMSString sQuery;
	CEMSString sSelectID;
	tstring sChunk;

	GetISAPIData().GetXMLString( _T("selectID"), sSelectID );

	while ( sSelectID.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		GetQuery().Initialize();
		sQuery.Format( _T("DELETE FROM AgentContacts WHERE AgentContactID IN (%s)"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );
	}
}