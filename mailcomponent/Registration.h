// Registration.h: interface for the CRegistration class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRATION_H__8945FC83_43FB_4395_A110_3CC035E152B1__INCLUDED_)
#define AFX_REGISTRATION_H__8945FC83_43FB_4395_A110_3CC035E152B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRegistration  
{
public:
	static int GetRegIsEval(const tstring& sRegKey, bool& bIsEval, bool& bIsExpired);
	CRegistration();
	virtual ~CRegistration();

};

#endif // !defined(AFX_REGISTRATION_H__8945FC83_43FB_4395_A110_3CC035E152B1__INCLUDED_)
