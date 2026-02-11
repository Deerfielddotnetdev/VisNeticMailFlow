// Groups.h: interface for the CGroups class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GROUPS_H__531BA8AD_217C_4574_94C1_261FE05D3E64__INCLUDED_)
#define AFX_GROUPS_H__531BA8AD_217C_4574_94C1_261FE05D3E64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CGroups : public CXMLDataClass ,public TGroups
{
public:
	CGroups( CISAPIData& ISAPIData );
	virtual ~CGroups();

	virtual int Run(CURLAction& action);

protected:
	void ListAll( CURLAction& action );
	void QueryOne( CURLAction& action );
	void DoUpdate( CURLAction& action );
	void DoInsert( CURLAction& action );
	void DoDelete( CURLAction& action );
	void DecodeForm( void );
	void GenerateXML( void );

	list<int> m_IDs;
};

#endif // !defined(AFX_GROUPS_H__531BA8AD_217C_4574_94C1_261FE05D3E64__INCLUDED_)
