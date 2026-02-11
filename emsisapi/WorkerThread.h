////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/WorkerThread.h,v 1.1.4.1.2.1 2006/07/18 12:55:03 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// WorkerThread.h: interface for the CWorkerThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORKERTHREAD_H__52A2740A_F742_4C27_BC2E_DF9B275AAEBF__INCLUDED_)
#define AFX_WORKERTHREAD_H__52A2740A_F742_4C27_BC2E_DF9B275AAEBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ISAPIThread.h"
#include "ISAPIData.h"
#include "XSLTranslator.h"
#include "EMSException.h"
#include "EMSMutex.h"

class CWorkerThread : public CISAPIThread  
{
public:
	CWorkerThread( CSharedObjects* pShared );
	virtual ~CWorkerThread();

	void SetSharedObject( CSharedObjects* pShared ) { m_pShared = pShared; }
	virtual unsigned int Run( void );
protected:

	void Initialize(void);
	bool OpenDatabase(bool bSendErrorPage = false);
	void CloseDatabase(void);
	void UnInitialize(void);

	void DoJob(void);
	void ServePage(void);
	void Authenticate( bool bPost );
	void HandleEMSException( CEMSException& EMSException );
	void HandleODBCError( ODBCError_t& error );
	void HandleUnhandledException( void );
	void HandleDCAException( const wchar_t* err );
	void TranslateXSL( CURLAction& action );
	void SendErrorPage( LPCTSTR szErrorMsg, bool bForceHTML = false, LPCTSTR szTitle = NULL );
	void GetLoginPage(void);
	void RedirectToLoginPage(void);
	void RedirectToMainPage( unsigned char nHasLoggedIn );
	void SendLoginPage( bool bError, LPCTSTR szErrorMsg = NULL);
	void LookForLostCookie( int nAgentID );
	
	CODBCConn	m_db;
	CODBCQuery	m_query;

	BOOL		m_bLoginPage;
	BOOL		m_bFinishRequest;
		
	CAgentSession	m_Session;
	CBrowserSession	m_BrowserSession;
	CISAPIData*		m_pISAPIData;
	CXSLTranslator	m_XSLT;
	XServerParameters srvparam;

	// Added by Mark Mohr on 01/13/2006
	BOOL IsNameRFC1123Compliant(LPCSTR lpServerName)
	{
		// Get Length
		int nLen = lstrlen(lpServerName);
		for(int i = 0; i < nLen; i++)
		{
			if((lpServerName[i] >= 0x00 && lpServerName[i] < 0x2D)    /* NULL to , */
				|| (lpServerName[i] > 0x2E && lpServerName[i] < 0x30) /* / */
				|| (lpServerName[i] > 0x3A && lpServerName[i] < 0x41) /* ; to *@ */
				|| (lpServerName[i] > 0x5A && lpServerName[i] < 0x61) /* [ to ' */
				|| (lpServerName[i] > 0x7A))                          /* { to ~ */
				return 0;
		}

		// Must have had all these characters
		// - . 0 to 9 A to Z a to z
		return 1;
	}
};

#endif // !defined(AFX_WORKERTHREAD_H__52A2740A_F742_4C27_BC2E_DF9B275AAEBF__INCLUDED_)
