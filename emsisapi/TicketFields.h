/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketFields.h,v 1.1 2007/09/22 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Ticket Fields
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CTicketFields : public CXMLDataClass, public TTicketFields 
{
public:
	CTicketFields( CISAPIData& ISAPIData );
	virtual ~CTicketFields() {};

	virtual int Run( CURLAction& action );

protected:
	void QueryAll(void);
	void QueryOne( CURLAction& action );
	void Update(void);
	void Delete(void);
	void AddTicketFields(void);

	vector<TTicketFieldsTicketBox> m_tftb;
	vector<TTicketFieldsTicketBox>::iterator mtftbIter;
	TTicketFieldsTicketBox tftb;
		
	vector<TTicketFieldOptions> m_tfo;
	vector<TTicketFieldOptions>::iterator mtfoIter;
	TTicketFieldOptions tfo;	
	int m_MaxOptionID;	
};
