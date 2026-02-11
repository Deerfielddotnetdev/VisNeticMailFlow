// RegExTest.cpp: implementation of the CRegExTest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RegExTest.h"
#include <.\boost\regex.hpp>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegExTest::CRegExTest(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CRegExTest::~CRegExTest()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
int CRegExTest::Run( CURLAction& action )
{
	tstring sAction;	

	// get the action...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );
	
	if( sAction.compare( _T("test") ) == 0 )
	{
		DecodeForm();		
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// TestRegEx tests the provided string against the RegEx
// 
////////////////////////////////////////////////////////////////////////////////
void CRegExTest::TestRegEx(  dca::String sRegEx, dca::String sTest )
{
	bool bIsMatch = false;
	
	DebugReporter::Instance().DisplayMessage("CRegExTest::TestRegex - entered.", DebugReporter::ISAPI, GetCurrentThreadId());

	dca::String f;
	f.Format("CRegExTest::TestRegex - String to Test: %s", sTest.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	f.Format("CRegExTest::TestRegex - RegEx: %s", sRegEx.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	try
	{
		boost::regex pattern (sRegEx,boost::regex_constants::perl);
		
		if (boost::regex_search (sTest, pattern, boost::regex_constants::format_perl))
		{
			DebugReporter::Instance().DisplayMessage("CRegExTest::TestRegex - found match", DebugReporter::ISAPI, GetCurrentThreadId());
			bIsMatch = true;
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CRegExTest::TestRegex - match not found", DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
	catch(...)
	{
		THROW_EMS_EXCEPTION( E_InvalidParameters, _T("Invalid Regular Expression!"));
	}
	
	GetXMLGen().AddChildElem( _T("RegExTest") );
	GetXMLGen().AddChildAttrib( _T("RegEx"), sRegEx.c_str() );
	GetXMLGen().AddChildAttrib( _T("TestText"), sTest.c_str() );
	GetXMLGen().AddChildAttrib( _T("Result"), bIsMatch );
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecodeForm
// 
////////////////////////////////////////////////////////////////////////////////
void CRegExTest::DecodeForm(void)
{
	CEMSString sRegEx;
	CEMSString sTest;	
	GetISAPIData().GetFormString( _T("regex"), sRegEx, true );
	GetISAPIData().GetFormString( _T("testtext"), sTest, true );
	
	dca::String sReg(sRegEx);
	dca::String sText(sTest);

	TestRegEx(sReg,sText);
}
