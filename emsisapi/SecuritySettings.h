// SecuritySettings.h: interface for the CSecuritySettings class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SECURITYSETTINGS_H__D58FB0C2_AE81_4067_BBD1_B534A3DA83DB__INCLUDED_)
#define AFX_SECURITYSETTINGS_H__D58FB0C2_AE81_4067_BBD1_B534A3DA83DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CSecuritySettings : public CXMLDataClass  
{
public:
	CSecuritySettings(CISAPIData& ISAPIData );
	virtual ~CSecuritySettings();


	virtual int Run(CURLAction& action);

protected:
	
};

#endif // !defined(AFX_SECURITYSETTINGS_H__D58FB0C2_AE81_4067_BBD1_B534A3DA83DB__INCLUDED_)
