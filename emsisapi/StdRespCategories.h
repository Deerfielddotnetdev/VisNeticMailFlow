/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/StdRespCategories.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Class to manage standard response categories  
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CStdRespCategories : public CXMLDataClass, public TStdResponseCategories
{
public:
	CStdRespCategories( CISAPIData& ISAPIData );
	virtual ~CStdRespCategories();

	virtual int Run( CURLAction& action );

protected:
	void ListAll(void);
	void QueryOne(void);
	void DecodeForm( void );
	int Update(CURLAction& action);
	void Delete(void);
	bool m_bRefreshRequired;
	unsigned char m_access;	
};

