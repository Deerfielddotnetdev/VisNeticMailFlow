#pragma once

#include "XMLDataClass.h"

class CTicketCategories : public CXMLDataClass  
{
public:
	CTicketCategories( CISAPIData& ISAPIData );
	virtual ~CTicketCategories() {};

	virtual int Run( CURLAction& action );

protected:
	void Query(void);
	void QueryOne( CURLAction& action );
	void Update(void);
	void Delete(void);
};
