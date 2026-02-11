// IPRanges.h: interface for the CIPRanges class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "XMLDataClass.h"

class CIPRanges : public CXMLDataClass, public TIPRanges 
{
public:
	CIPRanges( CISAPIData& ISAPIData );
	virtual ~CIPRanges();

	virtual int Run(CURLAction& action);

protected:

	int IPAccess( CURLAction& action );
	int IPRanges( CURLAction& action );
	int UpdateIPRanges( CURLAction& action );
};
