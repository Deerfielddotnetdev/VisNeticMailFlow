// Logout.h: interface for the CLogout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGOUT_H__78B00091_C98B_48B8_AD32_A8790B737224__INCLUDED_)
#define AFX_LOGOUT_H__78B00091_C98B_48B8_AD32_A8790B737224__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CLogout : public CXMLDataClass  
{
public:
	CLogout( CISAPIData& ISAPIData );

	// this is the only way to get an instance of this singleton class
	static CLogout& GetInstance();

	virtual ~CLogout();

	virtual int Run(CURLAction& action);
	
protected:	

};

#endif // !defined(AFX_LOGOUT_H__78B00091_C98B_48B8_AD32_A8790B737224__INCLUDED_)
