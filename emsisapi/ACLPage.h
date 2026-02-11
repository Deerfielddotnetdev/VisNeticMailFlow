// ACLPage.h: interface for the CACLPage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACLPAGE_H__6206B0B6_BC96_4FDD_85A5_BF4EE65BE9D3__INCLUDED_)
#define AFX_ACLPAGE_H__6206B0B6_BC96_4FDD_85A5_BF4EE65BE9D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CACLPage : public CXMLDataClass  
{
public:
	CACLPage( CISAPIData& ISAPIData );
	virtual ~CACLPage();

	virtual int Run( CURLAction& action );

protected:
	int AccessRights( CURLAction& action );
	int AccessControl( CURLAction& action );
	void AddObjectName( int ObjectTypeID, int& ActualID );
	int UpdateAccessRights( CURLAction& action );
	void ShowDefaultACL( int ObjectTypeID );
};

#endif // !defined(AFX_ACLPAGE_H__6206B0B6_BC96_4FDD_85A5_BF4EE65BE9D3__INCLUDED_)
