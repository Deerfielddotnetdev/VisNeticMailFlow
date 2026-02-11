////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/Thread.h,v 1.1 2005/08/09 16:40:55 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_RASTHREAD_H__E1F9CC76_6971_4DE3_9355_01A06B4E05E6__INCLUDED_)
#define AFX_RASTHREAD_H__E1F9CC76_6971_4DE3_9355_01A06B4E05E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CThread  
{
public:

	// Constructor
	CThread();
	virtual ~CThread();

	// The static thread starter function
	static unsigned int _stdcall ThreadFunc( void* pArg );

	// Do not override these
	virtual void SetKillEvent( HANDLE hEvent ) { m_hKillEvent = hEvent; }

	virtual void SetStopEvent( HANDLE hEvent ) { m_hStopEvent = hEvent; }

	virtual DWORD StartThread( void );
	virtual HANDLE GetHandle( void ) { return m_hThread; }
	virtual unsigned int GetID( void ) { return m_nThreadID; }

	// This is the one to override
	virtual unsigned int Run( void ) = 0;

	unsigned int m_nThreadID;

	HANDLE m_hReadyEvent;

protected:
	HANDLE m_hThread;
	HANDLE m_hKillEvent;		// service_main signals this to drain queue and exit
	HANDLE m_hStopEvent;
	
};

#endif // !defined(AFX_RASTHREAD_H__E1F9CC76_6971_4DE3_9355_01A06B4E05E6__INCLUDED_)
