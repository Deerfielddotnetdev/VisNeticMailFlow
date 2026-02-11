// ThreadManager.h: interface for the CThreadManager class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "WorkerThread.h"
#include ".\TicklerSystemThread.h"
#include ".\DBMonitorThread.h"
#include ".\WebSessionThread.h"
#include ".\DBMaintThread.h"

class CThreadManager  
{

public:

	enum Threads { Worker, Tickler, DBMonitor, WebSession, DBMaint, ThreadCount };

	CThreadManager();
	virtual ~CThreadManager();

	int StopAllThreads(void);
	int StartAllThreads(void);

	int PostThreadMessage( int nThread, UINT Msg, WPARAM wParam, LPARAM lParam );
	int RestartThread( int nThread );

	DWORD GetThreadID( int nThread );

	void SetStopEvent(void);
	
protected:
	CThread* m_pThreads[ThreadCount];
	HANDLE m_hKillEvent;
	HANDLE m_hStopEvent;

	int m_nThreadsStarted;
};
