// Signatures.h: interface for the CSignatures class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "XMLDataClass.h"

class CRegExTest : public CXMLDataClass
{
public:
	CRegExTest( CISAPIData& ISAPIData );
	virtual ~CRegExTest();

	virtual int Run(CURLAction& action);

	void TestRegEx( dca::String sRegEx, dca::String sTest );
	void DecodeForm( void );
	
};
