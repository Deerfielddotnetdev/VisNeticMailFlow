// EngineMonitor.h: interface for the CEngineMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESSIONMONITOR_H__F8B2CABE_6E73_4604_B255_5C3E6A7A5A76__INCLUDED_)
#define AFX_SESSIONMONITOR_H__F8B2CABE_6E73_4604_B255_5C3E6A7A5A76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Thread.h"
#include "SharedObjects.h"

class CEngineMonitor : public CThread 
{
public:
	
	CEngineMonitor(CSharedObjects* pShared);
	virtual ~CEngineMonitor();
	virtual unsigned int Run( void );

protected:

	int CheckEngine( void );
	BOOL GetProcessList( void );
	BOOL TerminateMyProcess(DWORD dwProcessId, UINT uExitCode);

	CDBMaintenanceMutex			m_DBMaintenanceMutex;
	bool						m_bDBMaintenanceRunning;
	int							m_nLastRun;
	CRoutingEngine				m_RoutingEngine;
	CODBCConn	m_db;
	CODBCQuery	m_query;	
};

#endif // !defined(AFX_SESSIONMONITOR_H__F8B2CABE_6E73_4604_B255_5C3E6A7A5A76__INCLUDED_)
