// RetrievalSession.cpp: implementation of the CRetrievalSession class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MailComponents.h"
#include "RetrievalSession.h"
#include "Pop3Connection.h"
#include "PopAccount.h"
#include "RegistryFns.h"

// thread start-up functions
const DWORD InitRetrievalConn(CRetrievalSession* pSession);
unsigned __stdcall CreatePopConn(PVOID pvoid);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRetrievalSession::CRetrievalSession()
{
	m_bIsBusy = false;
	m_hStartHandle = INVALID_HANDLE_VALUE;
	m_hThreadHandle = INVALID_HANDLE_VALUE;
	m_pPopConn = NULL;
	m_pPopAccount = NULL;
	m_nThreadID = 0;
}

CRetrievalSession::~CRetrievalSession()
{

}

const int CRetrievalSession::StartSession(CPopAccount* const pPopAccount)
{
	assert(pPopAccount);
	if (!pPopAccount)
	{
		DebugReporter::Instance().DisplayMessage("CRetrievalSession::StartSession - return -1", DebugReporter::MAIL);
		return -1;
	}
	m_pPopAccount = pPopAccount;
	UINT nRestartCount = 0;
	
	DWORD dwStartup = InitRetrievalConn(this);
	if (dwStartup != 0)
	{
		dca::String f;
		f.Format("CRetrievalSession::StartSession - Failed to start retrieval session: %d", dwStartup);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

		GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("RestartMailComponents"), nRestartCount );
		if(nRestartCount < 4)
		{
			WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"RestartMailComponents", nRestartCount++ );
		}
		else if (nRestartCount == 4)
		{
			WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"RestartMailComponents", nRestartCount++ );
			CreateAlert( EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_MAILCOMP_RESTART));
		}		

		return dwStartup;
	}

	m_bIsBusy = true;
	return 0;
}

const int CRetrievalSession::StopSession()
{
	if (m_pPopConn)
		m_pPopConn->Stop();

	if (m_hThreadHandle != INVALID_HANDLE_VALUE)
		WaitForSingleObject(m_hThreadHandle, INFINITE);

	return 0;
}

// this function's purpose is to initialize a message retrieval
// connection of the appropriate type (currently pop3, possibly
// also ATRN in the future)
const DWORD InitRetrievalConn(CRetrievalSession* pSession)
{
	// this is our thread start-up event
	pSession->m_hStartHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (pSession->m_hStartHandle == INVALID_HANDLE_VALUE)
		return ERROR_THREAD_STARTUP;
	ResetEvent(pSession->m_hStartHandle);

	// start the pop check thread, which will perform the actual
	// pop3 checking process
	// to do: if ATRN message source, init an ATRN check connection
	pSession->m_hThreadHandle = (HANDLE)_beginthreadex(NULL, 0, &CreatePopConn, pSession, 0, &pSession->m_nThreadID);
	if (pSession->m_hThreadHandle == INVALID_HANDLE_VALUE)
	{
		DebugReporter::Instance().DisplayMessage("InitRetrievalConn - Error - failed to start pop conn thread", DebugReporter::MAIL);
		CloseHandle(pSession->m_hStartHandle);
		return ERROR_THREAD_STARTUP;
	}

	// wait on the thread start-up event before returning (max wait time = 60 secs)
	DWORD dwRet = WaitForSingleObject(pSession->m_hStartHandle, 60000);
	CloseHandle(pSession->m_hStartHandle);
	if (dwRet != WAIT_OBJECT_0)
	{
		DebugReporter::Instance().DisplayMessage("InitRetrievalConn - Error - timed out waiting for pop conn thread to start", DebugReporter::MAIL);
		return ERROR_THREAD_STARTUP;
	}

	return 0;
}

//===========================================================================//
// creates a pop connection thread instance
//===========================================================================//
unsigned __stdcall CreatePopConn(PVOID pvoid)
{
	CRetrievalSession* pSession = (CRetrievalSession*)pvoid;
	assert(pSession);

	// new pop connection object
	pSession->m_pPopConn = new CPOP3Connection();
	assert(pSession->m_pPopConn);

	// ok, we're officially considered "started" at this point
	if (!SetEvent(pSession->m_hStartHandle))
	{
		dca::String f;
		f.Format("CreatePopConn - SetEvent() failed in CreatePopConn() (code %d)", GetLastError());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		//assert(0);
		delete pSession->m_pPopConn;
		return ERROR_THREAD_STARTUP;
	}

	// set the pop account
	pSession->m_pPopConn->SetPopAccount(pSession->m_pPopAccount);

	// check the pop account
	UINT nRet = pSession->m_pPopConn->CheckAccount();
	if (nRet != 0)
	{
		// logpoint: Failed to check POP3 account (server: %s, account: %s)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_CHECKACCOUNT_FAILED,
						pSession->m_pPopAccount->GetDescription().c_str(),
						pSession->m_pPopAccount->GetUserName().c_str(),
						pSession->m_pPopAccount->GetServerAddress().c_str());
	}

	delete pSession->m_pPopConn;
	pSession->m_pPopConn = NULL;

	delete pSession->m_pPopAccount;
	pSession->m_pPopAccount = NULL;

	CloseHandle(pSession->m_hThreadHandle);
	pSession->m_hThreadHandle = INVALID_HANDLE_VALUE;

	pSession->m_bIsBusy = false;
	pSession->m_nThreadID = 0;

	return 0;
}
