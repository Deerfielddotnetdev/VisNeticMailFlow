/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/VoipCall.cpp,v 1.2.2.1 2005/12/13 18:11:49 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Voip Extensions
||              
\\*************************************************************************/

#include "stdafx.h"
#include "VoipCall.h"
#include "Socket.h"
#include <iostream>

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CVoipCall::CVoipCall( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{	
}

CVoipCall::~CVoipCall()
{
	m_IsTicket = true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CVoipCall::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	//RequireAdmin();

	// Are we doing a Ticket or Contact?
	if (!GetISAPIData().GetURLLong( _T("ticket"), m_TicketID, true ))
	{
		if (!GetISAPIData().GetURLLong( _T("contact"), m_ContactID, true ))
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		}
		else
		{
			m_IsTicket = false;
		}
	}
		
	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( sAction.compare( _T("dial") ) == 0 )
	{
		DISABLE_IN_DEMO();
		MakeCall();
		return 0;
	}

	if ( m_IsTicket )
	{
		ListTicketContacts( m_TicketID );
		action.m_sPageTitle.assign( "VOIP Call for Ticket" );
	}
	else
	{
		ListContact( action, m_ContactID );
		action.m_sPageTitle.assign( "VOIP Call for Contact" );
	}
	
	return List();		
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query all voip Extensions
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipCall::List(void)
{
	//RequireAdmin();

	m_AgentID = GetAgentID();

	TVoipExtensions ve;
	ve.m_AgentID = m_AgentID;
	ve.PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("VoipCall") );
	GetXMLGen().AddChildAttrib( _T("IsTicket"), m_IsTicket );
	if ( m_IsTicket )
	{
		GetXMLGen().AddChildAttrib( _T("TicketID"), m_TicketID );
	}
	
	GetXMLGen().AddChildElem( _T("VoipExtensions") );
	GetXMLGen().IntoElem();
	
    while( GetQuery().Fetch() == S_OK )
	{
		if(ve.m_IsEnabled)
		{
			GetXMLGen().AddChildElem( _T("Extension") );
			GetXMLGen().AddChildAttrib( _T("ID"), ve.m_VoipExtensionID );
			GetXMLGen().AddChildAttrib( _T("Description"), ve.m_Description );
			GetXMLGen().AddChildAttrib( _T("ServerID"), ve.m_VoipServerID );
			GetXMLGen().AddChildAttrib( _T("IsDefault"), ve.m_IsDefault );
			GetXMLGen().AddChildAttrib( _T("Extension"), ve.m_Extension );
			GetXMLGen().AddChildAttrib( _T("Pin"), ve.m_Pin );
		}
	}

	GetXMLGen().OutOfElem();

	TVoipServers vs;
	GetXMLGen().AddChildElem( _T("VoipServers") );
	GetXMLGen().IntoElem();
	vs.PrepareList(GetQuery());
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Server") );
		GetXMLGen().AddChildAttrib( _T("VSID"), vs.m_VoipServerID );
		GetXMLGen().AddChildAttrib( _T("Description"), vs.m_Description );			
	}
	GetXMLGen().OutOfElem();	

	GetXMLGen().AddChildElem( _T("VoipDialingCodes") );		
	GetQuery().Initialize();
	TVoipDialingCodes vdc;
	BINDCOL_LONG( GetQuery(), vdc.m_VoipDialingCodeID );
	BINDCOL_LONG( GetQuery(), vdc.m_VoipServerID );
	BINDCOL_TCHAR( GetQuery(), vdc.m_Description );
	BINDCOL_TCHAR( GetQuery(), vdc.m_DialingCode );
	BINDCOL_BIT( GetQuery(), vdc.m_IsDefault );
	GetQuery().Execute( _T("SELECT VoipDialingCodeID,VoipServerID,Description,DialingCode,IsDefault ") 
		_T("FROM VoipDialingCodes ")
		_T("WHERE IsEnabled=1 ")
		_T("ORDER BY VoipServerID,Description ") );

	GetXMLGen().IntoElem();	
    while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("DialingCode") );
		GetXMLGen().AddChildAttrib( _T("ID"), vdc.m_VoipDialingCodeID );
		GetXMLGen().AddChildAttrib( _T("VSID"), vdc.m_VoipServerID );
		GetXMLGen().AddChildAttrib( _T("Description"), vdc.m_Description );
		GetXMLGen().AddChildAttrib( _T("DialingCode"), vdc.m_DialingCode );
		GetXMLGen().AddChildAttrib( _T("IsDefault"), vdc.m_IsDefault );						
	}
	GetXMLGen().OutOfElem();	

	return 0;
}

void CVoipCall::MakeCall(void)
{
	CEMSString sUrl;
	TVoipServers vs;
	TVoipExtensions ve;
	TVoipDialingCodes vdc;
	
	DecodeForm();
	
	GetQuery().Initialize();
	ve.m_VoipExtensionID = m_ExtensionID;
	ve.Query(GetQuery());
	vs.m_VoipServerID = ve.m_VoipServerID;
	m_Extension = ve.m_Extension;
	
	GetQuery().Reset(true);
	vs.Query(GetQuery());
	m_ServerUrl.Format( _T("%s"),vs.m_Url);
	m_ServerPort = vs.m_Port;
	
	GetQuery().Reset(true);
	vdc.m_VoipDialingCodeID = m_DialCodeID;
	vdc.Query(GetQuery());
	
	// Strip all spaces and non numeric characters from the number
	string x;
	CEMSString sNumber;
	sNumber.Format( _T("%s"), m_NumToDial );
	x.assign(sNumber.c_str());
				
    basic_string<char, std::char_traits<char> >::iterator i = x.begin();
    while(i != x.end())
    {
        if(!isdigit(*i)) i = x.erase(i);
        else i++;
    }

	if ( x.length() < 1 )
	{
		THROW_VALIDATION_EXCEPTION( _T("Number"), _T("An invalid Number was specified") );
	}
	
	sUrl.Format( _T("GET /PbxAPI.aspx?func=make_call&from=%d&pin=%s&to=%s%s HTTP/1.0"),m_Extension,m_Pin,vdc.m_DialingCode,x.c_str() );	
	CEMSString sHost;
	sHost.Format( _T("Host: %s"),m_ServerUrl.c_str());
		
	string sTemp = "";
	bool bRedirected = false;
	try
	{
		SocketClient s(m_ServerUrl.c_str(), m_ServerPort);
		s.SendLine(sUrl.c_str());		
		s.SendLine(sHost.c_str());
		s.SendLine("");	
		
		size_t pos;
		size_t pos2;
		string sNewUrl;		

		while (1)
		{
			string l = s.ReceiveLine();
			pos = l.find("302 Found",0);
			if (pos != string::npos)
				bRedirected = true;

			if(bRedirected)
			{
				pos2 = l.find("Location:",0);
				if (pos2 != string::npos)
				{
					sNewUrl = l.substr(pos2+10, l.length());
				}
			}

			sTemp += l;
			if (l.empty())
				break;
		}

		if(bRedirected && sNewUrl.length()>0)
		{
			sUrl.Format( _T("GET %s HTTP/1.0"),sNewUrl.c_str() );

			SocketClient s2(m_ServerUrl.c_str(), m_ServerPort);
			s2.SendLine(sUrl.c_str());
			s2.SendLine(sHost.c_str());
			s2.SendLine("");

			while (1)
			{
				string l = s2.ReceiveLine();
				sTemp += l;
				if (l.empty())
					break;
			}
		}

	} 
	catch (...)
	{
		CEMSString sError;
		sError.Format( _T("Unable to complete VOIP call. Could not connect\n\nto VOIP Server: %s on Port: %d"), m_ServerUrl.c_str(), m_ServerPort );
		THROW_VALIDATION_EXCEPTION( _T("Voip Call Failed"), sError );
	}

	if ( m_PersonalDataID < 0 )
	{
		// Add the number for the contact
		TPersonalData pd;
		pd.m_AgentID = 0;
		pd.m_ContactID = m_ContactID;
		_tcscpy(pd.m_DataValue, m_NumToDial);
		if ( m_PersonalDataID == -2 )
		{
			pd.m_PersonalDataTypeID = 2;
		}
		else if ( m_PersonalDataID == -3 )
		{		
			pd.m_PersonalDataTypeID = 3;
		}
		else
		{
			pd.m_PersonalDataTypeID = 4;
		}
		pd.Insert(GetQuery());
	}
	else
	{
		// Update the number for the contact
		GetQuery().Reset(true);
		BINDPARAM_TCHAR( GetQuery(), m_NumToDial );
		BINDPARAM_LONG( GetQuery(), m_PersonalDataID );
		GetQuery().Execute( _T("UPDATE PersonalData SET DataValue=? WHERE PersonalDataID=?") );			
	}
}

void CVoipCall::DecodeForm(void)
{
	// Extension
	GetISAPIData().GetXMLLong( _T("Extension"), m_ExtensionID );
	if ( m_ExtensionID < 1 )
		THROW_VALIDATION_EXCEPTION( _T("Extension"), _T("An invalid Extension was specified") );

	// NumberType or PersonalDataID
	GetISAPIData().GetXMLLong( _T("Number"), m_PersonalDataID );
	
	// Pin
	if(!(GetISAPIData().GetXMLTCHAR( _T("selectedPin"), m_Pin, 51, true )))
		GetISAPIData().GetFormTCHAR( _T("selectedPin"), m_Pin, 51 );
	
	// DialCodeID
	GetISAPIData().GetXMLLong( _T("DialingCode"), m_DialCodeID );

	// ContactID
	GetISAPIData().GetXMLLong( _T("Contact"), m_ContactID );

	// Number to Dial
	GetISAPIData().GetXMLTCHAR( _T("numToDial"), m_NumToDial, 256, true );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the contacts for a ticket.           
\*--------------------------------------------------------------------------*/
void CVoipCall::ListTicketContacts( int nTicketID )
{
	GetQuery().Initialize();
	ContactList_t ListItem;
	BINDCOL_LONG_NOLEN( GetQuery(), ListItem.nContactID );
	BINDCOL_TCHAR( GetQuery(), ListItem.Name );
	BINDCOL_TCHAR_NOLEN( GetQuery(), ListItem.Email );
	
	BINDPARAM_LONG( GetQuery(), nTicketID );
	
	GetQuery().Execute( _T("SELECT DISTINCT TicketContacts.ContactID, Contacts.Name, DataValue ") 
		_T("FROM TicketContacts ")
		_T("INNER JOIN Contacts ON TicketContacts.ContactID = Contacts.ContactID ")
		_T("INNER JOIN PersonalData ON Contacts.DefaultEmailAddressID = PersonalData.PersonalDataID ")
		_T("WHERE TicketContacts.TicketID = ? AND Contacts.IsDeleted=0 ")
		_T("ORDER BY 1,2 ") );
	
	list<ContactList_t> List;

	while (GetQuery().Fetch() == S_OK)
	{
		List.push_back( ListItem );
	}
	
	GenMultipleContactXML( List );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the contacts for a ticket.           
\*--------------------------------------------------------------------------*/
void CVoipCall::ListContact( CURLAction& action, int nContactID )
{
	GetQuery().Initialize();
	ContactList_t ListItem;
	BINDCOL_LONG_NOLEN( GetQuery(), ListItem.nContactID );
	BINDCOL_TCHAR( GetQuery(), ListItem.Name );
	BINDCOL_TCHAR_NOLEN( GetQuery(), ListItem.Email );
	
	BINDPARAM_LONG( GetQuery(), nContactID );
	
	GetQuery().Execute( _T("SELECT DISTINCT Contacts.ContactID, Contacts.Name, PersonalData.DataValue ") 
		_T("FROM Contacts  ")
		_T("INNER JOIN PersonalData ON Contacts.DefaultEmailAddressID = PersonalData.PersonalDataID ")
		_T("WHERE Contacts.ContactID = ? AND Contacts.IsDeleted=0 ") );
			
	list<ContactList_t> List;

	while (GetQuery().Fetch() == S_OK)
	{
		List.push_back( ListItem );
	}
	
	GenMultipleContactXML( List );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates the XML for a list of multiple contacts	              
\*--------------------------------------------------------------------------*/
void CVoipCall::GenMultipleContactXML( list<ContactList_t>& List )
{
	list<ContactList_t>::iterator iter;
	
	for ( iter = List.begin(); iter != List.end(); iter++ )
	{
		GetXMLGen().AddChildElem(_T("Contact"));
		GetXMLGen().AddChildAttrib(_T("ID"), iter->nContactID );
		GetXMLGen().AddChildAttrib(_T("Name"), iter->NameLen ? iter->Name : iter->Email );		
	}
	
	TPersonalData pd;

	for ( iter = List.begin(); iter != List.end(); iter++ )
	{		
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), pd.m_ContactID );
		BINDCOL_TCHAR( GetQuery(), pd.m_Note );
		BINDCOL_TCHAR( GetQuery(), pd.m_DataValue );		
		BINDCOL_LONG( GetQuery(), pd.m_PersonalDataID );
		BINDPARAM_LONG( GetQuery(), iter->nContactID );
		
		GetQuery().Execute( _T("SELECT pd.contactid,pdt.typename,pd.datavalue,pd.personaldataid ") 
							_T("FROM personaldatatypes pdt ")
							_T("INNER JOIN personaldata pd ON pdt.personaldatatypeid=pd.personaldatatypeid ")
							_T("WHERE pd.personaldatatypeid in (2,3,4) AND pd.contactid=? ") );		

		while (GetQuery().Fetch() == S_OK)
		{
			GetXMLGen().AddChildElem(_T("Number"));
			GetXMLGen().AddChildAttrib(_T("CID"), pd.m_ContactID );
			GetXMLGen().AddChildAttrib(_T("NumberType"), pd.m_Note );
			GetXMLGen().AddChildAttrib(_T("Number"), pd.m_DataValue );
			GetXMLGen().AddChildAttrib(_T("PDID"), pd.m_PersonalDataID );
		}		
	}
}
