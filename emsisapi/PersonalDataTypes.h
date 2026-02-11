/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/PersonalDataTypes.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing PersonalDataTypes
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CPersonalDataTypes : public CXMLDataClass  
{
public:
	CPersonalDataTypes( CISAPIData& ISAPIData );
	virtual ~CPersonalDataTypes() {};

	virtual int Run( CURLAction& action );

protected:
	void Query(void);
	void QueryOne( CURLAction& action );
	void Update(void);
	void Delete(void);
};
