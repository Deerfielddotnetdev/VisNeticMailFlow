////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/ThreadPool.cpp,v 1.1 2005/04/18 18:48:45 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// ThreadPool.cpp: implementation of the CThreadPool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ThreadPool.h"
#include "WorkerThread.h"
#include "RegistryFns.h"

////////////////////////////////////////////////////////////////////////////////
// 
//  Initialize all objects and launch worker threads
// 
////////////////////////////////////////////////////////////////////////////////
void CThreadPool::Initialize( HSE_VERSION_INFO * pVer )
{
	DebugReporter::Instance().DisplayMessage("CThreadPool::Initialize - set extension version", DebugReporter::ISAPI, GetCurrentThreadId());

	pVer->dwExtensionVersion = MAKELONG( HSE_VERSION_MINOR, HSE_VERSION_MAJOR );

    strcpy( pVer->lpszExtensionDesc, _T("VisNetic MailFlow EMS ISAPI Extension") );

	DebugReporter::Instance().DisplayMessage("CThreadPool::Initialize - create new CSharedObjects", DebugReporter::ISAPI, GetCurrentThreadId());

	m_pShared = new CSharedObjects;
	
	DebugReporter::Instance().DisplayMessage("CThreadPool::Initialize - initialize the URLMap", DebugReporter::ISAPI, GetCurrentThreadId());

	m_pShared->URLMap().Initialize();
	
	dca::String o;		
	tstring szWebType;
	if(!GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, tstring(_T("webType")).c_str(), szWebType))
	{
		dca::String sTemp(szWebType.c_str());

		o.Format("CThreadPool::Initialize - Setting WebType to %s.", sTemp.c_str());
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			
		m_pShared->sWebType.assign(sTemp);
	}
	UINT nUseATC=0;
	if (GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseATC"), nUseATC) == ERROR_SUCCESS)
	{
		if( nUseATC == 1)
		{
			m_pShared->m_bUseATC = true;
		}
		o.Format("CThreadPool::Initialize - Setting UseATC to %s.", m_pShared->m_bUseATC ? _T("true"):_T("false"));
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	}
	UINT nIISVer=6;
	if (GetRegInt(EMS_ROOT_KEY, _T("Software\\Microsoft\\InetStp"), _T("MajorVersion"), nIISVer) == ERROR_SUCCESS)
	{
		m_pShared->m_nIISVer = nIISVer;
		o.Format("CThreadPool::Initialize - Setting IIS Major Version to %d.", nIISVer);
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	}
	
	DebugReporter::Instance().DisplayMessage("CThreadPool::Initialize - StartAllThreads", DebugReporter::ISAPI, GetCurrentThreadId());

	StartAllThreads();	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Handle an ISAPI request by creating a job and putting it in the queue
// 
////////////////////////////////////////////////////////////////////////////////
int CThreadPool::HandleRequest( EXTENSION_CONTROL_BLOCK * pECB ) 
{ 
	CJob job( pECB );
	
	return m_pShared->Queue().AddJob( job ); 
}

	
////////////////////////////////////////////////////////////////////////////////
// 
// Uninitialize all objects and stop worker threads
// 
////////////////////////////////////////////////////////////////////////////////
void CThreadPool::Terminate( void )
{
	DebugReporter::Instance().DisplayMessage("CThreadPool::Terminate - drain all jobs from queue", DebugReporter::ISAPI, GetCurrentThreadId());

	m_pShared->Queue().DrainJobs();

	DebugReporter::Instance().DisplayMessage("CThreadPool::Terminate - call StopAllThreads", DebugReporter::ISAPI, GetCurrentThreadId());

	StopAllThreads();

	DebugReporter::Instance().DisplayMessage("CThreadPool::Terminate - delete m_pShared", DebugReporter::ISAPI, GetCurrentThreadId());

	delete m_pShared;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Start worker threads
// 
////////////////////////////////////////////////////////////////////////////////
int CThreadPool::StartAllThreads()
{
	CThread* pThread;

	// We'll create 5 worker threads and the session monitor will
	// maintain the thread count once the cache is loaded.

	DebugReporter::Instance().DisplayMessage("CThreadPool::StartAllThreads - create 5 worker threads", DebugReporter::ISAPI, GetCurrentThreadId());

	// Adjust the thread pool up
	while( m_Threads.size() < 5 )
	{
		pThread = new CWorkerThread( m_pShared );
		pThread->SetKillEvent( INVALID_HANDLE_VALUE );
		pThread->StartThread();
		m_Threads.push_back( pThread );	
	}
	
	DebugReporter::Instance().DisplayMessage("CThreadPool::StartAllThreads - create session monitor thread", DebugReporter::ISAPI, GetCurrentThreadId());

	// Create the session monitor thread
	m_pSessionMonitor = new CSessionMonitor( m_pShared, m_Threads );

	DebugReporter::Instance().DisplayMessage("CThreadPool::StartAllThreads - set session monitor thread kill event then start", DebugReporter::ISAPI, GetCurrentThreadId());

	// Start the thread with its own kill object
	m_pSessionMonitor->SetKillEvent( INVALID_HANDLE_VALUE );
	m_pSessionMonitor->StartThread();

	// Create the engine monitor thread
	m_pEngineMonitor = new CEngineMonitor( m_pShared );

	DebugReporter::Instance().DisplayMessage("CThreadPool::StartAllThreads - set engine monitor thread kill event then start", DebugReporter::ISAPI, GetCurrentThreadId());

	// Start the thread with its own kill object
	m_pEngineMonitor->SetKillEvent( INVALID_HANDLE_VALUE );
	m_pEngineMonitor->StartThread();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Stop worker threads
// 
////////////////////////////////////////////////////////////////////////////////
int CThreadPool::StopAllThreads()
{
	list<CThread*>::iterator iter;

	DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - setting worker thread kill events", DebugReporter::ISAPI, GetCurrentThreadId());

	// Set all kill events
	for (iter = m_Threads.begin(); iter != m_Threads.end(); iter++ )
	{
		SetEvent( (*iter)->GetKillEvent() );
	}

	DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - waiting for worker threads to close", DebugReporter::ISAPI, GetCurrentThreadId());

	for (iter = m_Threads.begin(); iter != m_Threads.end(); iter++ )
	{
		if( WAIT_TIMEOUT == WaitForSingleObject( (*iter)->GetHandle(), 2000 ) )
		{
			DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - worker thread failed to close", DebugReporter::ISAPI, GetCurrentThreadId());

			//TerminateThread( (*iter)->GetHandle(), -1 );
		}
	}

	DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - setting session monitor thread kill event", DebugReporter::ISAPI, GetCurrentThreadId());

	SetEvent( m_pSessionMonitor->GetKillEvent() );

	DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - waiting for session monitor thread to close", DebugReporter::ISAPI, GetCurrentThreadId());

	if( WAIT_TIMEOUT == WaitForSingleObject( m_pSessionMonitor->GetHandle(),2000 ))
	{
		DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - session monitor thread failed to close", DebugReporter::ISAPI, GetCurrentThreadId());

		//TerminateThread( m_pSessionMonitor->GetHandle(), -1 );
	}

	DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - setting engine monitor thread kill event", DebugReporter::ISAPI, GetCurrentThreadId());

	SetEvent( m_pEngineMonitor->GetKillEvent() );

	DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - waiting for engine monitor thread to close", DebugReporter::ISAPI, GetCurrentThreadId());

	if( WAIT_TIMEOUT == WaitForSingleObject( m_pEngineMonitor->GetHandle(),2000 ))
	{
		DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - engine monitor thread failed to close", DebugReporter::ISAPI, GetCurrentThreadId());

		//TerminateThread( m_pSessionMonitor->GetHandle(), -1 );
	}
	
	DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - delete worker threads", DebugReporter::ISAPI, GetCurrentThreadId());

	// Delete the thread objects
	for (iter = m_Threads.begin(); iter != m_Threads.end(); iter++ )
	{
		delete (*iter);
	}

	DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - delete session monitor thread", DebugReporter::ISAPI, GetCurrentThreadId());

	delete m_pSessionMonitor;

	DebugReporter::Instance().DisplayMessage("CThreadPool::StopAllThreads - delete engine monitor thread", DebugReporter::ISAPI, GetCurrentThreadId());

	delete m_pEngineMonitor;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// ReloadAllThreads(void)
// 
////////////////////////////////////////////////////////////////////////////////
void CThreadPool::ReLoadAllThreads(void)
{
	DebugReporter::Instance().DisplayMessage("CThreadPool::ReLoadAllThreads - set session monitor reload event", DebugReporter::ISAPI, GetCurrentThreadId());

	SetEvent( m_pSessionMonitor->GetReloadEvent() );
	
	DebugReporter::Instance().DisplayMessage("CThreadPool::ReLoadAllThreads - set engine monitor reload event", DebugReporter::ISAPI, GetCurrentThreadId());

	SetEvent( m_pEngineMonitor->GetReloadEvent() );
	
	list<CThread*>::iterator iter;

	DebugReporter::Instance().DisplayMessage("CThreadPool::ReLoadAllThreads - set worker threads reload event", DebugReporter::ISAPI, GetCurrentThreadId());

	// set all reload events
	for (iter = m_Threads.begin(); iter != m_Threads.end(); iter++ )
	{
		SetEvent( (*iter)->GetReloadEvent() );
	}	
}
