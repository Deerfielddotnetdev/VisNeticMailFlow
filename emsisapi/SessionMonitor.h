// SessionMonitor.h: interface for the CSessionMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESSIONMONITOR_H__F8B2CABE_6E73_4904_B255_5C3E6A7A5A76__INCLUDED_)
#define AFX_SESSIONMONITOR_H__F8B2CABE_6E73_4904_B255_5C3E6A7A5A76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Thread.h"
#include "SharedObjects.h"

struct Queue_t
{
	int nQueueID;
	int nActionID;
	int nTargetID;
};

class CSessionMonitor : public CThread 
{
public:
	CSessionMonitor( CSharedObjects* pShared, list<CThread*>& ThreadList );
	virtual ~CSessionMonitor();

	void SetSharedObject( CSharedObjects* pShared ) { m_pShared = pShared; }
	virtual unsigned int Run( void );

protected:

	void MaintainThreadCount(void);
	void CheckInToDB(int nServerID, bool bIn);
	void CheckMonitorQueue( void );

	list<CThread*>&				m_ThreadList;
	CDBMaintenanceMutex			m_DBMaintenanceMutex;
	bool						m_bDBMaintenanceRunning;
	int							m_nLastCache;
	int							m_nLastRun;
	int							m_nLastSync;
	int							m_nLastStatus;
	int							m_nLastDB;
	CRoutingEngine				m_RoutingEngine;	// COM object for communicating w/ Routing Engine
	CODBCConn	m_db;
	CODBCQuery	m_query;
	
	Queue_t m_Queue;
	vector<Queue_t> vQueue;
	vector<Queue_t>::iterator vIter;
};

#endif // !defined(AFX_SESSIONMONITOR_H__F8B2CABE_6E73_4904_B255_5C3E6A7A5A76__INCLUDED_)
