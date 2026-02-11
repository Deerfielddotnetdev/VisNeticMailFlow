////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/MessagingComponents.h,v 1.2 2005/11/29 21:36:28 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////

#pragma once


typedef __declspec( dllimport ) int (*pInitMailComponents)( DWORD dwLogThreadID );
typedef __declspec( dllimport ) int (*pShutdownMailComponents)( void );
typedef __declspec( dllimport ) void (*pCheckMailNow)(void);
typedef __declspec( dllimport ) void (*pSendMailNow)(void);
typedef __declspec( dllimport ) int (*pTestPOP3Auth)( const tstring& sHostName, 
                                                        const UINT nPort, 
                                                        const BOOL bIsAPOP, 
                                                        const tstring& sUserName, 
                                                        const tstring& sPassword,
                                                        const UINT nTimeoutSecs,
														const UINT isSSL );

typedef __declspec( dllimport ) int (*pTestSMTPAuth)( const tstring& sHostName, 
                                                        const UINT nPort, 
                                                        const tstring& sUserName, 
                                                        const tstring& sPassword,
                                                        const UINT nTimeoutSecs,
														const UINT isSSL );
  
typedef __declspec( dllimport ) int (*pRefreshServerParams)(void);

///////////////////////////////////////////////////////////////////////////////////////////
// MailComponents interface return codes
///////////////////////////////////////////////////////////////////////////////////////////

enum ReturnCodes
{
	error_success = 0,						// no error

	// init and trigger functions
	error_winsock_init = 1,					// couldn't init winsock
	error_thread_startup = 2,				// thread startup failure
	error_thread_shutdown = 3,				// thread shutdown timeout

	// auth tests
	error_connect_failed = 4,				// couldn't connect
	error_auth_failed = 5,					// login credentials invalid
	error_auth_method_unsupported = 6		// login method (e.g. APOP, CRAM-MD5, etc.) unsupported
};


class CMessagingComponents  
{
public:
	CMessagingComponents();
	virtual ~CMessagingComponents();

	int LoadDLL(void);
	BOOL IsLoaded(void)			{ return m_bLoaded; }
	int UnloadDLL(void);

	int Initialize(void);
	int Shutdown(void);
	int CheckMailNow(void);
	int SendMailNow(void);
	int TestPOP3Auth( const tstring& sHostName, 
                      const UINT nPort, 
                      const BOOL bIsAPOP, 
                      const tstring& sUserName, 
                      const tstring& sPassword,
                      const UINT nTimeoutSecs,
					  const UINT isSSL );
	int TestSMTPAuth( const tstring& sHostName, 
                      const UINT nPort, 
                      const tstring& sUserName, 
                      const tstring& sPassword,
                      const UINT nTimeoutSecs,
					  const UINT isSSL );
	int RefreshServerParams(void);
protected:
	HINSTANCE				m_pDLL;
	BOOL					m_bLoaded;

	pInitMailComponents		m_InitMailComponents;
	pShutdownMailComponents m_ShutdownMailComponents;
	pCheckMailNow			m_CheckMailNow;
	pSendMailNow			m_SendMailNow;
	pTestPOP3Auth			m_TestPOP3Auth;
	pTestSMTPAuth			m_TestSMTPAuth;
	pRefreshServerParams	m_RefreshServerParams;

};
