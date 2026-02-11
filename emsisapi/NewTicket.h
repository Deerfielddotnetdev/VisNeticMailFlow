/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/NewTicket.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "Ticket.h"
#include "XMLDataClass.h"

class CNewTicket : public CTicket
{
public:
	CNewTicket(CISAPIData& ISAPIData);
	virtual ~CNewTicket() {};

	virtual int Run(CURLAction& action);

private:
	void DeleteReservedID( void );
	void DecodeForm( bool bMoveMsg = false );
	void Insert( void );
	void InsertMoveMsg( void );
	void SaveTicket( void );
	bool HandleContact( void );
	int  FindContactsByName( bool bAddToXML );
	void AddDefaultValues( void );	
	
private:
	CEMSString m_sName;
	CEMSString m_sEmail;
	tstring m_Note;
	int m_CloseTicket;
	int m_ContactID;
	bool m_IsPhone;
	bool m_IsCopy;
	
	vector<TTicketFieldsTicket> m_tft;
	vector<TTicketFieldsTicket>::iterator tftIter;

};
