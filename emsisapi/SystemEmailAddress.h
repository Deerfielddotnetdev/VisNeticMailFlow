// SystemEmailAddress.h: interface for the CSystemEmailAddress class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYSTEMEMAILADDRESS_H__8B9DAA65_D58C_460B_9369_D89BC21B69E9__INCLUDED_)
#define AFX_SYSTEMEMAILADDRESS_H__8B9DAA65_D58C_460B_9369_D89BC21B69E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CSystemEmailAddress : public CXMLDataClass  
{
public:
	CSystemEmailAddress(CISAPIData& ISAPIData);
	virtual ~CSystemEmailAddress();

	virtual int Run(CURLAction& action);

};

#endif // !defined(AFX_SYSTEMEMAILADDRESS_H__8B9DAA65_D58C_460B_9369_D89BC21B69E9__INCLUDED_)
