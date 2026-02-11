// Registration.h: interface for the CRegistration class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRATION_H__389797CC_D6A5_475A_9330_EB57685374F7__INCLUDED_)
#define AFX_REGISTRATION_H__389797CC_D6A5_475A_9330_EB57685374F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CRegistration : public CXMLDataClass  
{
public:
	CRegistration( CISAPIData& ISAPIData );
	virtual ~CRegistration();

	virtual int Run( CURLAction& action );

protected:
	int ListAll(void);
	int Update(CURLAction& action);
	void GenerateXML(void);
	void AddUserSize( long nUserSize );
	tstring m_sKey;
};

#endif // !defined(AFX_REGISTRATION_H__389797CC_D6A5_475A_9330_EB57685374F7__INCLUDED_)
