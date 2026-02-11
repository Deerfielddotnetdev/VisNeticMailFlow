/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/PreferenceTicketboxes.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CPreferenceTicketboxItem 
{
public:
	CPreferenceTicketboxItem() { ZeroMemory( this, sizeof(CPreferenceTicketboxItem) ); }
	virtual ~CPreferenceTicketboxItem() {};

	int m_nTicketBoxID;
	unsigned char m_IsSubscribed;
	TCHAR m_szTicketBoxName[TICKETBOXES_NAME_LENGTH];
	TCHAR m_szTicketBoxDescripton[TICKETBOXES_DESCRIPTION_LENGTH];
	int m_nOpenItems;
};


class CPreferenceTicketboxes : public CXMLDataClass
{
public:
	CPreferenceTicketboxes(CISAPIData& ISAPIData );
	virtual ~CPreferenceTicketboxes() {};

	virtual int Run(CURLAction& action);

private:
	void List(void);
	void Update(void);

	void GetTicketBoxes( list<int>& IDList );


	int m_AgentID;
	int m_nRefreshLeftPane;

};
