////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/ThreadPool.h,v 1.1 2005/04/18 18:48:45 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// ThreadPool.h: interface for the CThreadPool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREADPOOL_H__826D2D30_4740_45EE_BD09_41AFB631AA60__INCLUDED_)
#define AFX_THREADPOOL_H__826D2D30_4740_45EE_BD09_41AFB631AA60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WorkerThread.h"
#include "SharedObjects.h"
#include "SessionMonitor.h"
#include "EngineMonitor.h"

class CThreadPool  
{
public:
	CThreadPool() {};
	virtual ~CThreadPool() {};

	void Initialize( HSE_VERSION_INFO * pVer );
	int HandleRequest( EXTENSION_CONTROL_BLOCK * pECB );
	void Terminate( void );
	void ReLoadAllThreads(void);
	
	CSharedObjects& GetSharedObjects(void) { return *m_pShared; }

protected:

	int StopAllThreads(void);
	int StartAllThreads(void);

	list<CThread*> m_Threads;

	CSharedObjects* m_pShared;
	CSessionMonitor* m_pSessionMonitor;
	CEngineMonitor* m_pEngineMonitor;
};

#endif // !defined(AFX_THREADPOOL_H__826D2D30_4740_45EE_BD09_41AFB631AA60__INCLUDED_)
