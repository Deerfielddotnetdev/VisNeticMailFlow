// EmailAddress.cpp: implementation of the CEmailAddress class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EmailAddress.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEmailAddress::CEmailAddress()
{
}

CEmailAddress::~CEmailAddress()
{
}

CEmailAddress::CEmailAddress(const CEmailAddress& aAddr)
:	m_sInetName(aAddr.m_sInetName),
	m_sFullName(aAddr.m_sFullName),
	m_sCharset(aAddr.m_sCharset)
{
}

CEmailAddress::CEmailAddress(const string& sInetName, const string& sFullName, const string& sCharset)
:	m_sInetName(sInetName),
	m_sFullName(sFullName),
	m_sCharset(sCharset)
{
}

CEmailAddress& CEmailAddress::operator = (const CEmailAddress& aAddr)
{
    if (this != &aAddr) 
	{
        m_sInetName	= aAddr.m_sInetName;
        m_sFullName	= aAddr.m_sFullName;
        m_sCharset	= aAddr.m_sCharset;
    }
	
    return *this;
}

string CEmailAddress::GetMailboxString()
{
	string s;
	return s;
}
