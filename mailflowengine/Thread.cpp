////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/Thread.cpp,v 1.1 2005/08/09 16:40:55 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <process.h>

#include "Thread.h"


////////////////////// Static Member Functions //////////////////////////
// Static thread function calls member function, since _beginthreadex
// can't call a member function directly, it passes the object pointer
// to this static function
unsigned int _stdcall CThread::ThreadFunc( void* pArg )
{
	CThread* pThread = (CThread*) pArg;

	return pThread->Run();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CThread::CThread( void )
{
	// Initialize thread handle
	m_hThread = INVALID_HANDLE_VALUE;
	m_hReadyEvent = CreateEvent( NULL, TRUE, FALSE, NULL );	
	m_hStopEvent = INVALID_HANDLE_VALUE;
}

CThread::~CThread()
{
	// Close all handles used by thread
	if ( m_hThread != INVALID_HANDLE_VALUE )
		CloseHandle( m_hThread );

	if ( m_hReadyEvent != INVALID_HANDLE_VALUE )
		CloseHandle( m_hReadyEvent );
}

/////////////////////////// Public member functions //////////////////

// Thread startup
// 
DWORD CThread::StartThread( void )
{
	// Create the thread
	m_hThread = (HANDLE)_beginthreadex( NULL, 0, ThreadFunc, 
		                                this, 0, &m_nThreadID ); 

	if ( m_hThread == INVALID_HANDLE_VALUE )
	{
		// Error creating thread
		return -1;
	}
	
	// Success
	return 0;
}
