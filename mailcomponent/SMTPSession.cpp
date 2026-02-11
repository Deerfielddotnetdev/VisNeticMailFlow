// SMTPSession.cpp: implementation of the CSMTPSession class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SMTPSession.h"
#include "MailComponents.h"
#include "SMTPConnection.h"
#include "SMTPDest.h"
#include "RegistryFns.h"

// thread start-up functions
const DWORD InitSMTPConn(CSMTPSession* pSession);
unsigned __stdcall SMTPConn(PVOID pvoid);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSMTPSession::CSMTPSession()
{
	m_bIsBusy = false;
	m_hStartHandle = INVALID_HANDLE_VALUE;
	m_hThreadHandle = INVALID_HANDLE_VALUE;
	m_pSMTPConn = NULL;
	m_pSMTPDest = NULL;
	m_nThreadID = 0;
}

CSMTPSession::~CSMTPSession()
{

}

const int CSMTPSession::StartSession(CSMTPDest *const pDest)
{
	assert(pDest);
	if (!pDest)
	{
		assert(0);
		return -1;
	}
	m_pSMTPDest = pDest;
	UINT nRestartCount = 0;

	DWORD dwStartup = InitSMTPConn(this);
	if (dwStartup != 0)
	{
		dca::String f;
		f.Format("CSMTPSession::StartSession - Failed to InitSMTPConn for MessageDestinationID: %d", pDest->GetMessageDestID());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

		GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("RestartMailComponents"), nRestartCount );
		if(nRestartCount < 4)
		{
			WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"RestartMailComponents", nRestartCount++ );
		}
		else if (nRestartCount == 4)
		{
			WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"RestartMailComponents", nRestartCount++ );
			CreateAlert( EMS_ALERT_EVENT_OUTBOUND_DELIVERY, (EMS_STRING_MAILCOMP_RESTART));
		}		

		return dwStartup;
	}

	m_bIsBusy = true;

	return 0;
}

const int CSMTPSession::StopSession()
{
	if (m_pSMTPConn)
		m_pSMTPConn->Stop();

	if (m_hThreadHandle != INVALID_HANDLE_VALUE)
		WaitForSingleObject(m_hThreadHandle, INFINITE);

	return 0;
}


//===========================================================================//
// initializes an SMTP delivery connection thread
//===========================================================================//
const DWORD InitSMTPConn(CSMTPSession* pSession)
{
	// this is our thread start-up event
	pSession->m_hStartHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (pSession->m_hStartHandle == INVALID_HANDLE_VALUE)
	{
		assert(0);
		return ERROR_THREAD_STARTUP;
	}
	ResetEvent(pSession->m_hStartHandle);

	// start the smtp thread, which will perform the actual
	// sending process
	pSession->m_hThreadHandle = (HANDLE)_beginthreadex(NULL, 0, &SMTPConn, pSession, 0, &pSession->m_nThreadID);
	if (pSession->m_hThreadHandle == INVALID_HANDLE_VALUE)
	{
		dca::String f;
		f.Format("InitSMTPConn - Failed to start SMTPConn thread for MessageDestinationID: %d", pSession->m_pSMTPDest->GetMessageDestID());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		
		CloseHandle(pSession->m_hStartHandle);
		return ERROR_THREAD_STARTUP;
	}

	// wait on the thread start-up event before returning (max wait time = 60 secs)
	DWORD dwRet = WaitForSingleObject(pSession->m_hStartHandle, 60000);
	CloseHandle(pSession->m_hStartHandle);
	if (dwRet != WAIT_OBJECT_0)
	{
		dca::String f;
		f.Format("InitSMTPConn - Timed out waiting for SMTPConn thread to start for MessageDestinationID: %d", pSession->m_pSMTPDest->GetMessageDestID());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

		return ERROR_THREAD_STARTUP;
	}
	
	return 0;	// success (pCheck will be freed by the caller)
}

//===========================================================================//
// creates a smtp connection thread instance
//===========================================================================//
unsigned __stdcall SMTPConn(PVOID pvoid)
{
	DebugReporter::Instance().DisplayMessage("SMTPSession - Entering SMTPConn", DebugReporter::MAIL);
	CSMTPSession* pSession = (CSMTPSession*)pvoid;
	assert(pSession);

	// new smtp connection object
	pSession->m_pSMTPConn = new CSMTPConnection();
	assert(pSession->m_pSMTPConn);

	DebugReporter::Instance().DisplayMessage("SMTPSession::SMTPConn - New CSMTPConnection successfully created", DebugReporter::MAIL);
	// ok, we're officially considered "started" at this point
	if (!SetEvent(pSession->m_hStartHandle))
	{
		dca::String f;
		f.Format("SMTPConn - SetEvent() failed in SMTPConn() for MessageDestinationID %d Code: %d", pSession->m_pSMTPDest->GetMessageDestID(),GetLastError());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		
		//assert(0);
		delete pSession->m_pSMTPConn;
		return ERROR_THREAD_STARTUP;
	}

	DebugReporter::Instance().DisplayMessage("SMTPSession::SMTPConn - SetEvent() succeeded", DebugReporter::MAIL);
	// set the smtp account
	pSession->m_pSMTPConn->SetMsgDest(pSession->m_pSMTPDest);

	DebugReporter::Instance().DisplayMessage("SMTPSession::SMTPConn - Calling DeliverMessages()", DebugReporter::MAIL);
	// check the smtp account
	UINT nRet = pSession->m_pSMTPConn->DeliverMessages();
	if (nRet != 0)
	{
		// logpoint: Not all outbound messages could be delivered (server: %s, port: %d)
		/*
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRes),
						EMS_STRING_MAILCOMP_CHECKACCOUNT_FAILED,
						pSMTP->pMsgDest->GetServerAddress().c_str(),
						pSMTP->pMsgDest->GetSMTPPort());
		*/
	}

	delete pSession->m_pSMTPConn;
	pSession->m_pSMTPConn = NULL;

	delete pSession->m_pSMTPDest;
	pSession->m_pSMTPDest = NULL;

	CloseHandle(pSession->m_hThreadHandle);
	pSession->m_hThreadHandle = INVALID_HANDLE_VALUE;

	pSession->m_bIsBusy = false;
	pSession->m_nThreadID = 0;
	
	DebugReporter::Instance().DisplayMessage("SMTPSession - Leaving SMTPConn", DebugReporter::MAIL);
	
	return 0;
}