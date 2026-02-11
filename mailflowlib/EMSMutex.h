////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMLIBRARY/EMSMutex.h,v 1.1 2005/05/06 14:56:44 markm Exp $
//
//  Copyright © 2000 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
// EMSMutex.h: interface for the CEMSMutex class.
////////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMSMUTEX_H__92D1088B_8083_4829_8924_83648723321B__INCLUDED_)
#define AFX_EMSMUTEX_H__92D1088B_8083_4829_8924_83648723321B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEMSMutex  
{
public:
	CEMSMutex();											
	virtual ~CEMSMutex();									// Destructor calls ReleaseLock() if necessary

	bool IsLocked(void);									// Test if mutex is owned by anyone
	bool AcquireLock( DWORD dwMilliseconds = INFINITE );	// Block until the lock is obtained
	void ReleaseLock(void);									// Release the mutex

protected:
	void Create( CEMSString& sName );
	HANDLE m_hMutex;
	bool m_bOwned;	
};


class CCreateContactMutex : public CEMSMutex
{
public:
	CCreateContactMutex( void )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.assign( _T("%d_EMS_CONTACT_MUTEX"), nProcessID );
		Create(sMutexName);
	}
};

class CAddContactToTicketMutex : public CEMSMutex
{
public:
	CAddContactToTicketMutex( int nTicketID )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_ACTT_%d"), nProcessID,nTicketID );
		Create( sMutexName );
	}
};

class CTicketMutex : public CEMSMutex
{
public:
	CTicketMutex( int nTicketID )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_TICKET_%d"), nProcessID,nTicketID );
		Create( sMutexName );
	}
};

class COutboundAttachMutex : public CEMSMutex
{
public:
	COutboundAttachMutex( void )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_MUTEX_OUTBOUND_ATTACHMENT"), nProcessID );
		Create( sMutexName );
	}
};


class CDBMaintenanceMutex : public CEMSMutex
{
public:
	CDBMaintenanceMutex( void )
	{
		CEMSString sMutexName;
		sMutexName.Format( _T("EMS_MUTEX_DB_MAINTENANCE") );
		Create( sMutexName );
	}
};

class CJobQueueMutex : public CEMSMutex
{
public:
	CJobQueueMutex( void )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_MUTEX_JOB_QUEUE"), nProcessID );
		Create( sMutexName );
	}
};

class CAccessControlMutex : public CEMSMutex
{
public:
	CAccessControlMutex( void )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_MUTEX_ACCESS_CONTROL"), nProcessID );
		Create( sMutexName );
	}
};

class CUploadMapMutex : public CEMSMutex
{
public:
	CUploadMapMutex( void )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_MUTEX_UPLOAD_MAP"), nProcessID );
		Create( sMutexName );
	}
};

class CAgentMapMutex : public CEMSMutex
{
public:
	CAgentMapMutex( void )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_MUTEX_AGENT_MAP"), nProcessID );
		Create( sMutexName );
	}
};

class CBrowserMapMutex : public CEMSMutex
{
public:
	CBrowserMapMutex( void )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_MUTEX_BROWSER_MAP"), nProcessID );
		Create( sMutexName );
	}
};

class CStatusMapMutex : public CEMSMutex
{
public:
	CStatusMapMutex( void )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_MUTEX_STATUS_MAP"), nProcessID );
		Create( sMutexName );
	}
};

class CResetLocksMutex : public CEMSMutex
{
public:
	CResetLocksMutex( void )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_MUTEX_RESET_LOCKS"), nProcessID );
		Create( sMutexName );
	}
};

class CSignatureMapMutex : public CEMSMutex
{
public:
	CSignatureMapMutex( void )
	{
		CEMSString sMutexName;
		int nProcessID = GetCurrentProcessId();
		sMutexName.Format( _T("%d_EMS_MUTEX_SIGNATURE_MAP"), nProcessID );
		Create( sMutexName );
	}
};
#endif // !defined(AFX_EMSMUTEX_H__92D1088B_8083_4829_8924_83648723321B__INCLUDED_)
