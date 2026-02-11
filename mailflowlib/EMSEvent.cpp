////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMLIBRARY/EMSEvent.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $
//
//  Copyright © 2000 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
// EMSEvent.cpp: implementation of the CEMSEvent class.
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EMSEvent.h"


////////////////////////////////////////////////////////////////////////////////
// 
// CEMSEvent constructor 
// 
////////////////////////////////////////////////////////////////////////////////
CEMSEvent::CEMSEvent( void )
{
	m_hEvent = INVALID_HANDLE_VALUE;
}

////////////////////////////////////////////////////////////////////////////////
// 
// CEMSEvent destructor
// 
////////////////////////////////////////////////////////////////////////////////
CEMSEvent::~CEMSEvent()
{
	if( m_hEvent != INVALID_HANDLE_VALUE && m_hEvent != NULL )
		CloseHandle( m_hEvent );
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create
// 
////////////////////////////////////////////////////////////////////////////////
void CEMSEvent::Create( CEMSString& sName )
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


	m_hEvent = CreateEvent( &sa, TRUE, FALSE, sName.c_str() );

    if(pSD != NULL) 
        LocalFree((HLOCAL) pSD); 

	if( m_hEvent == NULL )
	{
		THROW_EMS_EXCEPTION( E_SystemError, _T("Unable to create event") );
	}
}
