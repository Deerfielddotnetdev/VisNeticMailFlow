/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketBoxOwners.h,v 1.1 2007/09/22 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing TicketBox Owners
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CTicketBoxOwners : public CXMLDataClass  
{
public:
	CTicketBoxOwners( CISAPIData& ISAPIData );
	virtual ~CTicketBoxOwners() {};

	virtual int Run( CURLAction& action );

protected:
	void Query(void);
	void QueryOne( CURLAction& action );
	void Update(void);
	void Delete(void);
};
