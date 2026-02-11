/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/MultipleContacts.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

// only include once
#pragma once

#include "XMLDataClass.h"

struct MultipleContactList_t
{
	int nContactID;
	TCHAR Name[CONTACTS_NAME_LENGTH];
	long  NameLen;
	TCHAR Email[PERSONALDATA_DATAVALUE_LENGTH];
};

class CMultipleContacts : public CXMLDataClass  
{
public:
	
	// construction
	CMultipleContacts(CISAPIData& ISAPIData) : CXMLDataClass( ISAPIData ) {};
	virtual ~CMultipleContacts() {};

	int Run(CURLAction& action);

private:

	void ListTicketContacts( CURLAction& action, int nTicketID );
	void ListOutboundMsgContacts( CURLAction& action, int nMsgID );
	void GenMultipleContactXML( list<MultipleContactList_t>& List );
};
