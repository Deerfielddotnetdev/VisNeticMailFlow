/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Columns.h,v 1.1 2007/09/22 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Columns
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"
#include "TBVInfo.h"


class CColumns : public CXMLDataClass  
{
public:
	CColumns( CISAPIData& ISAPIData );
	virtual ~CColumns() {};

	virtual int Run( CURLAction& action );

protected:
	void Query(void);
	void Update(void);
	void DecodeForm(void);
	void GenerateXML(void);

private:	
	CTBVInfo m_TBView;
	TTicketFields tf;
	TTicketFieldViews tfv;
	vector<TTicketFieldViews> m_tfv;
	vector<TTicketFieldViews>::iterator tfvIter;
};
