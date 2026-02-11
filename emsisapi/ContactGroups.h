// ContactGroups.h: interface for the CContactGroups class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ContactGroupS_H__531BA8AD_217C_4574_94C1_261FE05D3E64__INCLUDED_)
#define AFX_ContactGroupS_H__531BA8AD_217C_4574_94C1_261FE05D3E64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CContactGroups : public CXMLDataClass ,public TContactGroups
{
public:
	CContactGroups( CISAPIData& ISAPIData );
	virtual ~CContactGroups();

	virtual int Run(CURLAction& action);

protected:
	void ListAll( CURLAction& action );
	void QueryOne( CURLAction& action );
	void DoUpdate( CURLAction& action );
	void DoInsert( CURLAction& action );
	void DoDelete( CURLAction& action );
	void SearchList( CURLAction& action );
	void DecodeForm( void );
	void GenerateXML( void );

	list<int> m_IDs;
	int m_nAgentID;
	int m_nOwnerID;
private:
	unsigned char m_AccessLevel;
	bool m_bAgentContactGroups;
};

#endif // !defined(AFX_ContactGroupS_H__531BA8AD_217C_4574_94C1_261FE05D3E64__INCLUDED_)
