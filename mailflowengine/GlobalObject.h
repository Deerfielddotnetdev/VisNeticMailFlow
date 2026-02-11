////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/GlobalObject.h,v 1.1 2005/08/09 16:40:55 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// GlobalObject.h: interface for the CGlobalObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBALOBJECT_H__FF9E7CCE_735B_4682_95A1_7A4729F7F04C__INCLUDED_)
#define AFX_GLOBALOBJECT_H__FF9E7CCE_735B_4682_95A1_7A4729F7F04C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ThreadManager.h"
#include "MessagingComponents.h"
#include "LicenseMgr.h"

class CGlobalObject  
{
public:
	CGlobalObject();
	virtual ~CGlobalObject();
	int Initialize(void);
	void Shutdown(void);	

	HRESULT GetDBParams( CODBCConn& db );
	const wchar_t* GetParameter( int ID );
	void SetParameter( int ID, const wchar_t* szValue );
	void SetServerReload( void );

	CThreadManager m_ThreadManager;
	CMessagingComponents m_MessagingComponents;
	CLicenseMgr m_LicenseMgr;

	CRITICAL_SECTION m_paramsCS;
	map<int,wstring> m_params;

	CODBCConn	m_db;
	CODBCQuery	m_query;
};

#endif // !defined(AFX_GLOBALOBJECT_H__FF9E7CCE_735B_4682_95A1_7A4729F7F04C__INCLUDED_)
