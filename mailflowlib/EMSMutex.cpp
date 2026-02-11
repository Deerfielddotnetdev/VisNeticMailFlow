////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMLIBRARY/EMSMutex.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $
//
//  Copyright © 2000 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
// EMSMutex.cpp: implementation of the CEMSMutex class.
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EMSMutex.h"


////////////////////////////////////////////////////////////////////////////////
// 
// CEMSMutex constructor 
// 
////////////////////////////////////////////////////////////////////////////////
CEMSMutex::CEMSMutex( void )
{
	m_hMutex = INVALID_HANDLE_VALUE;
	m_bOwned = false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// CEMSMutex destructor
// 
////////////////////////////////////////////////////////////////////////////////
CEMSMutex::~CEMSMutex()
{
	if( m_bOwned )
		ReleaseLock();

	if( m_hMutex != INVALID_HANDLE_VALUE && m_hMutex != NULL )
		CloseHandle( m_hMutex );
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSMutex::Create( CEMSString& sName )
{
	PSECURITY_DESCRIPTOR pSD; 
	SECURITY_ATTRIBUTES sa;	

	// Make sure the name isn't too long.
	if( sName.size() >= MAX_PATH )
	{
		sName.resize(MAX_PATH-1);
	}
	
	// Initialize a security descriptor.  
	pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
			SECURITY_DESCRIPTOR_MIN_LENGTH);   // defined in Winnt.h  
 
	InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);
 
	// Add a NULL descriptor ACL to the security descriptor.  
	SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE);

	// Initialize a security attributes structure.
	sa.nLength = sizeof (SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;


	m_hMutex = CreateMutex( &sa, FALSE, sName.c_str() );

    if(pSD != NULL) 
        LocalFree((HLOCAL) pSD); 

	if( m_hMutex == NULL )
	{
		THROW_EMS_EXCEPTION( E_SystemError, _T("Unable to create mutex") );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// IsLocked returns true if the mutex is owned by another thread.
// 
////////////////////////////////////////////////////////////////////////////////
bool CEMSMutex::IsLocked(void)		// Test if mutex is owned by anyone
{
	DWORD dwRet = WaitForSingleObject( m_hMutex, 100 );

	switch( dwRet )
	{
	case WAIT_TIMEOUT:
		return true;

	case WAIT_ABANDONED:
		// no break
	case WAIT_OBJECT_0:
		ReleaseMutex( m_hMutex );
		return false;

	case WAIT_FAILED:		// Error - problem with handle
		return false;
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////
// 
// AcquireLock waits for the lock. On success it returns true
// 
////////////////////////////////////////////////////////////////////////////////
bool CEMSMutex::AcquireLock( DWORD dwMilliseconds )	
{
	DWORD dwRet = WaitForSingleObject( m_hMutex, dwMilliseconds );

	switch( dwRet )
	{
	case WAIT_TIMEOUT:
		return false;

	case WAIT_ABANDONED:
		// no break
	case WAIT_OBJECT_0:
		m_bOwned = true;
		return m_bOwned;

	case WAIT_FAILED:		// Error - problem with handle
		return false;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// ReleaseLock
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSMutex::ReleaseLock(void)	// Release the mutex
{
	ReleaseMutex( m_hMutex );
	m_bOwned = false;
}
