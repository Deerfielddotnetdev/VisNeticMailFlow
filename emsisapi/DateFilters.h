// DateFilters.h: interface for the CDateFilters class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "XMLDataClass.h"

class CDateFilters : public CXMLDataClass, public TDateFilters  
{
public:
	CDateFilters( CISAPIData& ISAPIData );
	virtual ~CDateFilters();

	virtual int Run( CURLAction& action );

protected:
	int ListAll(void);
	int Query(void);
	int Update();
	int Delete();
	int New();
	bool TestRegEx(dca::String sRegEx);

	void DecodeForm(void);
	void GenerateXML(void);

	int m_MaxID;
	
};
