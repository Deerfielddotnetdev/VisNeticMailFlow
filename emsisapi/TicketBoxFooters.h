// TicketBoxFooters.h: interface for the CTicketBoxFooters class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "XMLDataClass.h"

class CTicketBoxFooters : public CXMLDataClass, public TTicketBoxFooters  
{
public:
	CTicketBoxFooters(CISAPIData& ISAPIData);
	virtual ~CTicketBoxFooters();

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
