////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/Thread.h,v 1.1 2005/04/18 18:48:45 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_RASTHREAD_H__E1F9CC76_6971_4DE3_9355_01A06B4E05E6__INCLUDED_)
#define AFX_RASTHREAD_H__E1F9CC76_6971_4DE3_9355_01A06B4E05E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SharedObjects.h"

class CThread  
{
public:

	// Constructor
	CThread( CSharedObjects* pShared );
	virtual ~CThread();

	// The static thread starter function
	static unsigned int _stdcall ThreadFunc( void* pArg );

	// Do not override these
	void SetKillEvent( HANDLE hEvent );
	HANDLE GetKillEvent() { return m_hKillEvent; }

	DWORD StartThread( void );
	HANDLE GetHandle( void ) { return m_hThread; }
	unsigned int GetID( void ) { return m_nThreadID; }
	unsigned int GetCount( void ) { return m_nCount; }

	HANDLE GetReloadEvent() { return m_hReloadEvent; }

	// This is the one to override
	virtual unsigned int Run( void ) = 0;

	unsigned int m_nThreadID;
	unsigned int m_nCount;

protected:
	CSharedObjects*		m_pShared;
	HANDLE				m_hThread;
	HANDLE				m_hKillEvent;	// service_main signals this to drain queue and exit
	bool				m_bDestroyKillEvent;
	HANDLE				m_hReloadEvent;

};

#endif // !defined(AFX_RASTHREAD_H__E1F9CC76_6971_4DE3_9355_01A06B4E05E6__INCLUDED_)
