////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/JobQueue.h,v 1.2 2005/11/29 21:16:26 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// JobQueue.h: interface for the CJobQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JOBQUEUE_H__825A56E8_19C8_4342_B6B7_EE87DCD0E499__INCLUDED_)
#define AFX_JOBQUEUE_H__825A56E8_19C8_4342_B6B7_EE87DCD0E499__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EMSMutex.h"

class CJob
{
public:
	CJob() : m_pECB(NULL) {}
	CJob( EXTENSION_CONTROL_BLOCK* pECB ) : m_pECB(pECB) {}
	EXTENSION_CONTROL_BLOCK* m_pECB;
};

class CJobQueue  
{
public:
	CJobQueue();
	virtual ~CJobQueue();

	int AddJob( CJob& job );
	int GetJob( CJob& job );
	void DrainJobs( void );

	HANDLE GetNewJobEvent(void) { return m_hNewJobEvent; }
	void SetQueueSize(const int nVal){ MAX_QUEUE_LENGTH = nVal; }

protected:
	deque<CJob> m_Queue;
	HANDLE m_hNewJobEvent;
	UINT MAX_QUEUE_LENGTH;
	CJobQueueMutex m_JQMutex;
};

#endif // !defined(AFX_JOBQUEUE_H__825A56E8_19C8_4342_B6B7_EE87DCD0E499__INCLUDED_)
