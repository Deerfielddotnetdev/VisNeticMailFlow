// TestLogin.h: interface for the CTestLogin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTLOGIN_H__B776D820_6D8E_4D55_8E06_2B6E0186C5EF__INCLUDED_)
#define AFX_TESTLOGIN_H__B776D820_6D8E_4D55_8E06_2B6E0186C5EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"
#include "Thread.h"

class CTestLoginThread : public CThread  
{
public:
	CTestLoginThread( CSessionMap& SessionMap );
	virtual ~CTestLoginThread();
	virtual unsigned int Run( void );

	BSTR HostName;
	BSTR UserName;
	BSTR Password;
	int nPort;
	int nTimeout;
	int isAPOP;
	int isSSL;
	int nType;		// 0 = message source, 1 = message destination
	unsigned int m_AgentID;

protected:
	CSessionMap& m_SessionMap;
};

class CTestLogin : public CXMLDataClass  
{
public:
	CTestLogin( CISAPIData& ISAPIData );
	virtual ~CTestLogin();
	
	virtual int Run(CURLAction& action);
	int StartTest( CURLAction& action );
};

#endif // !defined(AFX_TESTLOGIN_H__B776D820_6D8E_4D55_8E06_2B6E0186C5EF__INCLUDED_)
