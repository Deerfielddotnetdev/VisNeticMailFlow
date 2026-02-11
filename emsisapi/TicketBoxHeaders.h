// TicketBoxHeaders.h: interface for the CTicketBoxHeaders class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "XMLDataClass.h"

class CTicketBoxHeaders : public CXMLDataClass, public TTicketBoxHeaders
{
public:
	CTicketBoxHeaders(CISAPIData& ISAPIData);
	virtual ~CTicketBoxHeaders();

	// methods
	virtual int Run( CURLAction& action );

protected:
	
	int ListAll( CURLAction& action );
	int QueryOne( CURLAction& action );
	int DoUpdate( CURLAction& action );
	int DoInsert( CURLAction& action );
	int DoDelete( CURLAction& action );
	void DecodeForm( void );
	void GenerateXML( void );
};
