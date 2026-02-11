#pragma once
#include "XMLDataClass.h"

class UnlockTicketPage : public CXMLDataClass
{
public:
	UnlockTicketPage(CISAPIData& ISAPIData);
	virtual ~UnlockTicketPage(void);

	// methods
	virtual int Run(CURLAction& action);

protected:
	int UnlockTicket(int& nTicketID);
};
