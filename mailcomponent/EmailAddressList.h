// EmailAddressList.h: interface for the CEmailAddressList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMAILADDRESSLIST_H__D7AD897A_50B5_45C2_A750_7C4661D60EC1__INCLUDED_)
#define AFX_EMAILADDRESSLIST_H__D7AD897A_50B5_45C2_A750_7C4661D60EC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EmailAddress.h"
#include <vector>
using namespace std;

class CEmailAddressList  
{
public:
	CEmailAddressList();
	virtual ~CEmailAddressList();

	// copy constructor
	CEmailAddressList(const CEmailAddressList&);
	
	// assignment overload
    CEmailAddressList& operator = (const CEmailAddressList&);
	
    void Add(const CEmailAddress& addr);
	
    const int GetCount()					{ return m_addresses.size(); }
    CEmailAddress& Get(int index)			{ return *m_addresses[index];}

private:
	vector<CEmailAddress*> m_addresses;
	
    void DeleteAddresses();
    void CopyAddresses(const CEmailAddressList&);
};

#endif // !defined(AFX_EMAILADDRESSLIST_H__D7AD897A_50B5_45C2_A750_7C4661D60EC1__INCLUDED_)
