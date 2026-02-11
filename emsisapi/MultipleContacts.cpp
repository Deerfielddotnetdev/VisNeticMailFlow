/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/MultipleContacts.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "MultipleContacts.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point              
\*--------------------------------------------------------------------------*/
int CMultipleContacts::Run(CURLAction& action)
{
	CEMSString sID;
	int nID;

	// check security
	RequireAgentRightLevel(EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS);
	
	if (GetISAPIData().GetURLString( _T("TicketID"), sID, true ))
	{
		sID.CDLGetNextInt( nID );
		ListTicketContacts( action, nID );
	}
	else if (GetISAPIData().GetURLString( _T("OutboundMsgID"), sID, true ))
	{
		sID.CDLGetNextInt( nID );
		ListOutboundMsgContacts( action, nID );
	}
	else
	{
		// a TicketID or OutboundMsgID is required
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Error - A TicketID or OutboundMsgID URL parameter is required\n")); 
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the contacts for a ticket.           
\*--------------------------------------------------------------------------*/
void CMultipleContacts::ListTicketContacts( CURLAction& action, int nTicketID )
{
	GetQuery().Initialize();
	
	MultipleContactList_t ListItem;
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
	
	list<MultipleContactList_t> List;

	while (GetQuery().Fetch() == S_OK)
	{
		List.push_back( ListItem );
	}
	
	GenMultipleContactXML( List );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the contacts for an outbound message. 	              
\*--------------------------------------------------------------------------*/
void CMultipleContacts::ListOutboundMsgContacts( CURLAction& action, int nMsgID )
{
	GetQuery().Initialize();
	
	MultipleContactList_t ListItem;
	BINDCOL_LONG_NOLEN( GetQuery(), ListItem.nContactID );
	BINDCOL_TCHAR( GetQuery(), ListItem.Name );
	BINDCOL_TCHAR_NOLEN( GetQuery(), ListItem.Email );
	
	BINDPARAM_LONG( GetQuery(), nMsgID );
	
	GetQuery().Execute( _T("SELECT DISTINCT OutboundMessageContacts.ContactID, Contacts.Name, DataValue ") 
		_T("FROM OutboundMessageContacts ")
		_T("INNER JOIN Contacts ON OutboundMessageContacts.ContactID = Contacts.ContactID ")
		_T("INNER JOIN PersonalData ON Contacts.DefaultEmailAddressID = PersonalData.PersonalDataID ")
		_T("WHERE OutboundMessageContacts.OutboundMessageID = ? AND Contacts.IsDeleted=0 ")
		_T("ORDER BY 1,2 ") );
	
	list<MultipleContactList_t> List;
	
	while (GetQuery().Fetch() == S_OK)
	{
		List.push_back( ListItem );
	}
	
	GenMultipleContactXML( List );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates the XML for a list of multiple contacts	              
\*--------------------------------------------------------------------------*/
void CMultipleContacts::GenMultipleContactXML( list<MultipleContactList_t>& List )
{
	int nTicketBoxView = 0;
	if ( GetISAPIData().GetURLLong( _T("TicketBoxView"), nTicketBoxView, true ) )
	{
		GetXMLGen().AddChildElem( _T("TicketBoxView") );
		GetXMLGen().AddChildAttrib( _T("ID"), nTicketBoxView );
	}

	list<MultipleContactList_t>::iterator iter;

	for ( iter = List.begin(); iter != List.end(); iter++ )
	{
		GetXMLGen().AddChildElem(_T("Contact"));
		GetXMLGen().AddChildAttrib(_T("ID"), iter->nContactID );
		GetXMLGen().AddChildAttrib(_T("Name"), iter->NameLen ? iter->Name : iter->Email );
	}
}

