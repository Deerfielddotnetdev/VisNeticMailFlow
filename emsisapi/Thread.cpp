////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/Thread.cpp,v 1.1 2005/04/18 18:48:45 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
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


CThread::CThread( CSharedObjects* pShared ) : m_pShared(pShared)
{
	// Initialize thread handles
	m_hThread = INVALID_HANDLE_VALUE;
	m_hKillEvent = INVALID_HANDLE_VALUE;
	m_bDestroyKillEvent = false;
	m_hReloadEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
}

CThread::~CThread()
{
	// Close all handles used by thread
	if ( m_hThread != INVALID_HANDLE_VALUE )
		CloseHandle( m_hThread );

	if ( m_bDestroyKillEvent && m_hKillEvent != INVALID_HANDLE_VALUE )
		CloseHandle( m_hKillEvent );

	if( m_hReloadEvent != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hReloadEvent );
	}
}

/////////////////////////// Public member functions //////////////////


void CThread::SetKillEvent( HANDLE hEvent ) 
{ 
	if( hEvent == INVALID_HANDLE_VALUE )
	{
		// Create the kill event
		m_hKillEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
		m_bDestroyKillEvent = true;
	}
	else
	{
		m_hKillEvent = hEvent; 
	}
}


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

