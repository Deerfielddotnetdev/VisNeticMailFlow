// AboutPage.h: interface for the AboutPage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ABOUTPAGE_H__1D97B468_D706_4978_992E_2C57C9753899__INCLUDED_)
#define AFX_ABOUTPAGE_H__1D97B468_D706_4978_992E_2C57C9753899__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CAboutPage : public CXMLDataClass  
{
public:
	CAboutPage( CISAPIData& ISAPIData );
	virtual ~CAboutPage();

	virtual int Run( CURLAction& action );

protected:
	
};

#endif // !defined(AFX_ABOUTPAGE_H__1D97B468_D706_4978_992E_2C57C9753899__INCLUDED_)
