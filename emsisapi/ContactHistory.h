/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ContactHistory.h,v 1.2 2005/11/29 21:16:25 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CContactHistory : public CXMLDataClass  
{
public:
	CContactHistory(CISAPIData& ISAPIData);
	virtual ~CContactHistory();

	virtual int Run( CURLAction& action );

private:
	void ListContactHistory( int nContactID );
	void GetContactInfo( int nContactID );
	void DeleteTicket(void);
	void TakeOwnership(void);
	void Escalate(void);	
public:
	void GetUnReadMessageCountAndGetUnReadNoteCount(int nTicketID, int& nMC, int& nNC);
};
