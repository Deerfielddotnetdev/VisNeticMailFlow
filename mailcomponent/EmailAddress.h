// EmailAddress.h: interface for the CEmailAddress class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMAILADDRESS_H__D113F5D3_8FBD_4AE4_9529_2CD8645E2244__INCLUDED_)
#define AFX_EMAILADDRESS_H__D113F5D3_8FBD_4AE4_9529_2CD8645E2244__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
using namespace std;

class CEmailAddress  
{
public:
	string GetMailboxString();
	CEmailAddress();
	virtual ~CEmailAddress();

	// copy constructor
	CEmailAddress(const CEmailAddress&);
	
	// parameter constructor
    CEmailAddress(const string& inetName, const string& personalName, const string& charset);

	// assignment overload
    CEmailAddress& operator = (const CEmailAddress&);
	
    void Set(const string& sInetName, const string& sFullName, const string& sCharset)
															{
																m_sInetName = sInetName;
																m_sFullName = sFullName;
																m_sCharset = sCharset;
															}

	
	
    const string& GetInetName()	const						{ return m_sInetName; }
    const string& GetFullname()	const						{ return m_sFullName; }
    const string& GetCharset()	const						{ return m_sCharset; }

private:
	string m_sInetName;		// internet name (<jspratt@xyz.org>)
    string m_sFullName;		// full name (e.g. Jack A. Spratt)
    string m_sCharset;		// char set

};

#endif // !defined(AFX_EMAILADDRESS_H__D113F5D3_8FBD_4AE4_9529_2CD8645E2244__INCLUDED_)
