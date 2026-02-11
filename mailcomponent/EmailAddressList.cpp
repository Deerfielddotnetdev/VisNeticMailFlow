// EmailAddressList.cpp: implementation of the CEmailAddressList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EmailAddressList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEmailAddressList::CEmailAddressList()
{

}

CEmailAddressList::~CEmailAddressList()
{
	DeleteAddresses();
}

CEmailAddressList::CEmailAddressList(const CEmailAddressList& aList)
{
    CopyAddresses(aList);
}

CEmailAddressList& CEmailAddressList::operator = (const CEmailAddressList& aList)
{
    if (this != &aList) 
	{
        DeleteAddresses();
        CopyAddresses(aList);
    }
	
    return *this;
}

void CEmailAddressList::Add(const CEmailAddress& aAddr)
{
    CEmailAddress* addr = new CEmailAddress(aAddr);
	assert(addr);
	if (addr)	
		m_addresses.push_back(addr);
}

void CEmailAddressList::DeleteAddresses()
{
    vector<CEmailAddress*>::iterator ppaddr;
    for (ppaddr = m_addresses.begin(); ppaddr != m_addresses.end(); ++ppaddr) 
	{
        if (*ppaddr)
			delete *ppaddr;

        *ppaddr = NULL;
    }

    m_addresses.clear();
}


void CEmailAddressList::CopyAddresses(const CEmailAddressList& aList)
{
	const vector<CEmailAddress*>& addresses = aList.m_addresses;
	vector<CEmailAddress*>::iterator iter;

    for (iter = m_addresses.begin(); iter != m_addresses.end(); ++iter) 
	{
        CEmailAddress* addr = new CEmailAddress(**iter);
		if (addr)
			m_addresses.push_back(addr);
    }
}
