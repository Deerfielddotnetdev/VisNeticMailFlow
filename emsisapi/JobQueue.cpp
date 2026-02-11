////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/JobQueue.cpp,v 1.2 2005/11/29 21:16:26 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// JobQueue.cpp: implementation of the CJobQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JobQueue.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJobQueue::CJobQueue()
{
	m_hNewJobEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	MAX_QUEUE_LENGTH = 32;

	dca::String o;
	o.Format("CJobQueue - Initializing and setting Queue Size to 32.", MAX_QUEUE_LENGTH);
	DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());	
}

CJobQueue::~CJobQueue()
{
	CloseHandle( m_hNewJobEvent );
}


int CJobQueue::AddJob( CJob&  job )
{
	int nRet = 0;
	
	if(m_JQMutex.AcquireLock(1000))
	{
		if( m_Queue.size() < MAX_QUEUE_LENGTH )
		{
			m_Queue.push_back( job );
			m_JQMutex.ReleaseLock();
			SetEvent( m_hNewJobEvent );
		}
		else
		{
			m_JQMutex.ReleaseLock();			
			nRet = -1;
		}	
	}
	else
	{
		nRet = -1;
	}
	return nRet;
}


int CJobQueue::GetJob( CJob& job )
{
	int nRet = 0;

	if(m_JQMutex.AcquireLock(1000))
	{
		if( m_Queue.size() > 0 )
		{
			job = m_Queue.front();
			m_Queue.pop_front();
			m_JQMutex.ReleaseLock();			
		}
		else
		{
			m_JQMutex.ReleaseLock();			
			nRet = -1;
		}
	}
	else
	{
		nRet = -1;
	}

	if( m_Queue.size() == 0 )
	{
		ResetEvent( m_hNewJobEvent );
	}

	return nRet;
}


void CJobQueue::DrainJobs( void )
{
	HSE_SEND_HEADER_EX_INFO HeaderExInfo;
	int nRet = 0;
	CJob job(NULL);

	if(m_JQMutex.AcquireLock(1000))
	{
		while( m_Queue.size() > 0 )
		{
			job = m_Queue.front();
			m_Queue.pop_front();

			HeaderExInfo.pszStatus = "503 OK";
			HeaderExInfo.pszHeader = "Content-type: text/html\r\n\r\n";
			HeaderExInfo.cchStatus = strlen( HeaderExInfo.pszStatus );
			HeaderExInfo.cchHeader = strlen( HeaderExInfo.pszHeader );
			HeaderExInfo.fKeepConn = FALSE;

			if ( job.m_pECB )
			{	
				// send headers using IIS-provided callback
				// (note - if we needed to keep connection open,
				//  we would set fKeepConn to TRUE *and* we would
				//  need to provide correct Content-Length: header)

				job.m_pECB->ServerSupportFunction( job.m_pECB->ConnID,
												HSE_REQ_SEND_RESPONSE_HEADER_EX,
												&HeaderExInfo,
												NULL, NULL );
				//
				// Inform server that the request has been satisfied, 
				// and the connection may now be dropped
				//

				job.m_pECB->ServerSupportFunction( job.m_pECB->ConnID, 
												HSE_REQ_DONE_WITH_SESSION, 
												NULL, NULL, NULL );
			}
		}
		m_JQMutex.ReleaseLock();			
	}	
}
