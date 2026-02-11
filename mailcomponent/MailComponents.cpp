// MailComponents.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "MailComponents.h"
#include "PopPollThread.h"
#include "SMTPPollThread.h"
#include "ODBCConn.h"
#include "CriticalSection.h"
#include "SingleCrit.h"
#include <stdio.h>
#include <stdarg.h>
#include "PopAccount.h"
#include "Pop3Connection.h"
#include "SMTPDest.h"
#include "SMTPConnection.h"
#include "ServerParameters.h"
#include "QueryClasses.h"
#include "DateFNS.h"

#include <CkImap.h>
#include <CkGlobal.h>
#include <CkOAuth2.h>

static CPopPollThread* g_pPopPollThread;		// pop3 polling thread
static CSMTPPollThread* g_pSMTPPollThread;		// smtp polling thread
HANDLE g_hPopPollthread;						// pop3 poll thread handle
HANDLE g_hSMTPPollthread;						// smtp poll thread handle?
static CWinRegistry g_winReg;					// global registry object
dca::Mutex g_csDB;								// global database critical section
dca::Mutex g_csFileIO_1;						// file IO lock
CODBCConn g_odbcConn;							// global database connection
DWORD g_dwLogThreadID;							// global logging thread id
DWORD g_dwScanThreadID;							// global AV scanning thread id

const DWORD InitPopPollThread();
const DWORD InitSMTPPollThread();
unsigned __stdcall PopPollThread(PVOID pvoid);
unsigned __stdcall SMTPPollThread(PVOID pvoid);

//===========================================================================//
// Author: Mark R.
// Function:
// DLL entry point
//===========================================================================//
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			// note: instance handle can be stored here in a global
			// if needed at any point
			DwInitialize();
			break;
			
		case DLL_THREAD_ATTACH:
			break;
			
		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			DwFinalize();
			break;
    }
    return TRUE;
}

//===========================================================================//
// Author: Mark R.
// Function:
// exported function to init the mail gateway
//===========================================================================//
MAILCOMPONENTS_API int InitMailComponents(const DWORD dwLogThreadID, const DWORD dwScanThreadID)
{
	// store off these routing engine thread ids for later use
	g_dwLogThreadID = dwLogThreadID;
	g_dwScanThreadID = dwScanThreadID;

	// logpoint: Initializing mail components (LogThreadID = %d)
	CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_MESSAGE_PROCESSING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_INITIALIZING,
					dwLogThreadID);

	int nRet = 0;

	// init windows sockets
	//WSADATA wsaData;
	//if (WSAStartup(MAKEWORD(1,1),&wsaData) == SOCKET_ERROR)
	//{
	//	// logpoint: Winsock layer failed to initialize
	//	// note: critical errors will automatically fire an alert
	//	CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_CRITICAL_ERROR,
	//								EMSERR_MAIL_COMPONENTS,
	//								EMS_LOG_MESSAGE_PROCESSING,
	//								ERROR_INIT_WINSOCK),
	//					EMS_STRING_MAILCOMP_WINSOCK_INIT_FAILURE);

	//	return error_winsock_init;
	//}

	// initialize our pop3 polling thread
	nRet = InitPopPollThread();
	if (nRet != 0)
	{
		// logpoint: Failed to initialize the POP polling thread
		// note: critical errors will automatically fire an alert
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_CRITICAL_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_MESSAGE_PROCESSING,
									nRet),
						EMS_STRING_MAILCOMP_POP3_POLLTHREAD_INIT_FAILURE);

		return error_thread_startup;
	}

	// initialize our SMTP polling thread
	nRet = InitSMTPPollThread();
	if (nRet != 0)
	{
		// logpoint: Failed to initialize the SMTP polling thread
		// note: critical errors will automatically fire an alert
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_CRITICAL_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_MESSAGE_PROCESSING,
									nRet),
						EMS_STRING_MAILCOMP_SMTP_POLLTHREAD_INIT_FAILURE);

		return error_thread_startup;
	}

	return 0; // success
}

//===========================================================================//
// Author: Mark R.
// Function:
// exported function to force immediate email account checks
//===========================================================================//
MAILCOMPONENTS_API void CheckMailNow(void)
{
	// logpoint: Forcing an immediate POP3 mail check
	CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_INBOUND_MESSAGING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_POP3_CHECK_FORCED);

	// check all accounts now
	g_pPopPollThread->CheckNow();
}

MAILCOMPONENTS_API void SendMailNow(void)
{
	// logpoint: Forcing immediate SMTP mail delivery
	CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_OUTBOUND_MESSAGING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_SMTP_FORCED);

	// check all accounts now
	g_pSMTPPollThread->SendNow();
}

void CreateAlert(const UINT nAlertEventID, const CEMSString& emsString)
{
	DebugReporter::Instance().DisplayMessage("CreateAlert() - Posting administrative alert",  DebugReporter::MAIL);
	LINETRACE(_T("Posted administrative alert (alert event %d, text: %s)"), nAlertEventID, emsString.c_str());

	if (g_dwLogThreadID != 0)
	{
		// IMPORTANT: the routing engine frees this memory via HeapFree; the allocation
		// method used below must not be changed without the appropriate changes to the engine
		AlertInfo* pAi = (AlertInfo*)HeapAlloc(GetProcessHeap(), 0, sizeof(AlertInfo) + (emsString.size())*sizeof(TCHAR));
		if (!pAi) return;
		pAi->TicketBoxID = 0;
		pAi->AlertID = 0;
		wcscpy(pAi->wcsText, emsString.c_str());

		// note: PostThreadMessage does not block
		if (!PostThreadMessage(	g_dwLogThreadID,
								WM_SEND_ALERT,
								(WPARAM)nAlertEventID,
								(LPARAM)pAi))
		{
			// originally thought that I'd create a log entry here, but if
			// we can't send an alert, what's the chance that we can send a
			// log message? probably zilch, so just debug output it
			DebugReporter::Instance().DisplayMessage("CreateAlert() - Failed to post administrative alert",  DebugReporter::MAIL);
			LINETRACE(_T("Failed to create administrative alert: %d (g_dwLogThreadID = %d)\n"), GetLastError(), g_dwLogThreadID);

			// oh, and free our memory, since the routing engine didn't get the msg
			HeapFree(GetProcessHeap(), 0, pAi);
		}
	}
}

// public version of CreateAlert must be called with a string resource
// id, which helps to enforce localization potential
void CreateAlert(const UINT nAlertEventID, const UINT nStringID, ...)
{
	//DebugReporter::Instance().DisplayMessage("CreateAlert(with ID) - Posting administrative alert",  DebugReporter::MAIL);
	
	try
	{
		// get our formatted string
		va_list va;
		va_start(va, nStringID);
		CEMSString emsString;
		emsString.FormatArgList(nStringID, va);
		va_end(va);

		CreateAlert(nAlertEventID, emsString);
	}
	catch (CEMSException EMSException)
	{
		DebugReporter::Instance().DisplayMessage("CreateAlert(with ID) - Error during string formatting",  DebugReporter::MAIL);
	
		LINETRACE(_T("Error during string formatting (%s)\n"), EMSException.GetErrorString());
		assert(0);
	}
	catch (...)
	{
		DebugReporter::Instance().DisplayMessage("CreateAlert(with ID) - Unknown exception during CreateAlert(with ID)",  DebugReporter::MAIL);
	
		LINETRACE(_T("Unknown exception during CreateAlert\n"));
		assert(0);
	}
}

void CreateLogEntry(const UINT nErrorCode, LPCTSTR szString, ...)
{
	//DebugReporter::Instance().DisplayMessage("CreateLogEntry(with LP) - Entering",  DebugReporter::MAIL);
	
	CEMSString emsString;

	try
	{
		va_list va;
		va_start(va, szString);	
		emsString.FormatArgList(szString, va);
		va_end(va);
	}
	catch (CEMSException EMSException)
	{
		DebugReporter::Instance().DisplayMessage("CreateLogEntry(with LP) - Error formatting log string",  DebugReporter::MAIL);
	
		emsString = _T("Error formatting log string: ");
		emsString += szString;
	}

	CreateLogEntry( nErrorCode, emsString );
}

void CreateLogEntry(const UINT nErrorCode, const UINT nStringID, ... )
{
	//DebugReporter::Instance().DisplayMessage("CreateLogEntry(with ID) - Entering",  DebugReporter::MAIL);
	
	CEMSString emsString;

	try
	{
		va_list va;
		va_start(va, nStringID);	
		emsString.FormatArgList(nStringID, va);
		va_end(va);

		CreateLogEntry( nErrorCode, emsString );
	}
	catch (CEMSException EMSException)
	{
		DebugReporter::Instance().DisplayMessage("CreateLogEntry(with ID) - Error formatting log string",  DebugReporter::MAIL);
	
		emsString = _T("Error formatting log string ID=");
		emsString += nStringID;
	}
}

void CreateLogEntry(const UINT nErrorCode, CEMSString& emsString )
{
	//DebugReporter::Instance().DisplayMessage("CreateLogEntry(with EMS) - Entering",  DebugReporter::MAIL);
	
	// extract error code and severity
	int nEC = (nErrorCode & 0xffff);
	int nSev = ((nErrorCode & 0xf0000000) >> 28);

	// format the text
	try
	{
		assert(emsString.size() > 0);

		// send the log message to debug console
		LINETRACE(_T("%s (code: %d)\n"), emsString.c_str(), nEC);

		// if this is a critical error, send an administrative alert
		if (nSev == EMS_LOG_SEVERITY_CRITICAL_ERROR)
		{
			CreateAlert(	EMS_ALERT_EVENT_CRITICAL_ERROR,
							emsString);
		}

		// post this message to the routing engine logger thread
		if (g_dwLogThreadID != 0)
		{
			// allocate a copy of the formatted string on the heap;
			// the routing engine will free the memory after logging the event
			// IMPORTANT: the routing engine frees this memory via HeapFree; the allocation
			// method used below must not be changed without the appropriate changes to the engine
			TCHAR* pszString = (TCHAR*)HeapAlloc(GetProcessHeap(), 0, (emsString.size() * sizeof(TCHAR)) + sizeof(TCHAR));
			assert(pszString);
			if (pszString)
			{
				_tcscpy(pszString, emsString.c_str());

				// note: PostThreadMessage does not block
				if (!PostThreadMessage(	g_dwLogThreadID,
										WM_WRITE_LOG,
										(WPARAM)nErrorCode,
										(LPARAM)pszString))
				{
					DebugReporter::Instance().DisplayMessage("CreateLogEntry(with EMS) - Failed to create log entry",  DebugReporter::MAIL);
	
					LINETRACE(_T("Failed to create log entry: %d (g_dwLogThreadID = %d)\n"), GetLastError(), g_dwLogThreadID);
					
					// oh, and free our memory, since the routing engine didn't get the msg
					HeapFree(GetProcessHeap(), 0, pszString);
				}
			}
		}
	}
	catch (...)
	{
		DebugReporter::Instance().DisplayMessage("CreateLogEntry(with EMS) - Unknown exception",  DebugReporter::MAIL);
	
		LINETRACE(_T("Unknown exception during CreateLogEntry\n"));
		assert(0);
	}
}
 
const UINT OpenDBConn()
{
	DebugReporter::Instance().DisplayMessage("OpenDBConn() - Entering",  DebugReporter::MAIL);
	
	// lock access to the database object
	dca::Lock lock(g_csDB);
	
	// if we're already connected, then proceed no further
	if (g_odbcConn.IsConnected())
	{
		DebugReporter::Instance().DisplayMessage("OpenDBConn() - DB already connected",  DebugReporter::MAIL);
	
		return 0;
	}

	try
	{
		DebugReporter::Instance().DisplayMessage("OpenDBConn() - Attempting to connect DB",  DebugReporter::MAIL);
		g_odbcConn.Connect();
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);

		// logpoint: Unable to establish connection to application database (code: %d, msg: %s)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_MESSAGE_PROCESSING,
									ERROR_DATABASE),
						EMS_STRING_MAILCOMP_DBCONN_FAILURE,
						oerr.nErrorCode,
						oerr.szErrMsg);

		return ERROR_DATABASE;
	}

	DebugReporter::Instance().DisplayMessage("OpenDBConn() - DB connected successfully",  DebugReporter::MAIL);
	
	return 0;
}

void HandleDBError(ODBCError_t* pErr)
{
	dca::String e;
	dca::String h(pErr->szErrMsg);
	e.Format("HandleDBError - Caught database exception (%d): %s", pErr->nErrorCode, h.c_str());
	DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::MAIL);

	// "close" the connection so that it will be re-opened
	// on the next database access attempt
	dca::Lock lock(g_csDB);
	
	try
	{
		if (g_odbcConn.IsConnected())
		{
			DebugReporter::Instance().DisplayMessage("HandleDBError - Disconnecting DB connection",  DebugReporter::MAIL);
			// break the database connection, as we can no longer trust it
			g_odbcConn.Disconnect();

			// attempt to re-connect immedaitely;
			// warning! do NOT call OpenDBConn() here - doing so could
			// cause a recursion problem
			DebugReporter::Instance().DisplayMessage("HandleDBError - Connecting DB connection",  DebugReporter::MAIL);
			g_odbcConn.Connect();
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("HandleDBError - DB was already disconnected, reconnecting",  DebugReporter::MAIL);
			g_odbcConn.Connect();
		}
	}
	catch (ODBCError_t oerr)
	{
		// we essentially eat this error, which should only occur
		// if we fail to re-connect to the database; the connection
		// will be re-attempted later;
		// note that other threads waiting on the global db connection
		// lock will also fail if this occurs
		int nCode = oerr.nErrorCode;
		e.Format("HandleDBError - Failed to reconnect to database after disconnecting (connection will be re-attempted): %d", nCode);
		DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::MAIL);
	}

	// here we log any errors that aren't related to the database connection
	// (the MailFlow engine already logs these)
	if (pErr->nErrorCode != E_DatabaseConnection)
	{
		// logpoint: Encountered database exception (code: %d, msg: %s)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_MESSAGE_PROCESSING,
									ERROR_DATABASE),
						IDS_STRING_MAILCOMP_DB_EXCEPTION_CAUGHT,
						pErr->nErrorCode,
						pErr->szErrMsg);
	}
}


//===========================================================================//
// Author: Mark R.
// Function:
// exported function to stop the mail gateway
//===========================================================================// 
MAILCOMPONENTS_API int ShutdownMailComponents(void)
{
	DebugReporter::Instance().DisplayMessage("ShutdownMailComponents() - Entering",  DebugReporter::MAIL);
	
	int nRet = 0;
	DWORD dwRet;

	const UINT nTermTimeout = 30000;

	// stop the POP3 polling thread
	g_pPopPollThread->Stop();
	dwRet = WaitForSingleObject(g_hPopPollthread, nTermTimeout);
	if (dwRet == WAIT_TIMEOUT)
	{
		// logpoint: Timed out waiting for POP3 polling thread to stop (waited %d milliseconds)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									ERROR_POP3_POLLTHREAD_SHUTDOWN),
						EMS_STRING_MAILCOMP_POP3_POLLTHREAD_STOP_TIMEOUT,
						nTermTimeout);

		assert(0);
		nRet = error_thread_shutdown;
		goto cleanup;
	}

	DebugReporter::Instance().DisplayMessage("ShutdownMailComponents() - PopPollThread successfully terminated",  DebugReporter::MAIL);

	// stop the SMTP polling thread
	g_pSMTPPollThread->Stop();
	dwRet = WaitForSingleObject(g_hSMTPPollthread, nTermTimeout);
	if (dwRet == WAIT_TIMEOUT)
	{
		// logpoint: Timed out waiting for SMTP polling thread to stop (waited %d milliseconds)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									ERROR_SMTP_POLLTHREAD_SHUTDOWN),
						EMS_STRING_MAILCOMP_SMTP_POLLTHREAD_TIMEOUT,
						nTermTimeout);

		assert(0);
		nRet = error_thread_shutdown;
		goto cleanup;
	}

	DebugReporter::Instance().DisplayMessage("ShutdownMailComponents() - SMTPPollThread successfully terminated",  DebugReporter::MAIL);

cleanup:

	// close the database connection
	g_odbcConn.Disconnect();
	DebugReporter::Instance().DisplayMessage("ShutdownMailComponents() - Database connection closed",  DebugReporter::MAIL);

	WSACleanup();
	delete g_pPopPollThread;
	CloseHandle(g_hPopPollthread);
	delete g_pSMTPPollThread;
	CloseHandle(g_hSMTPPollthread);

	// logpoint: Mail components DLL has been terminated
	CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_MESSAGE_PROCESSING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_SHUTDOWN);

	return nRet;
}

//===========================================================================//
// Author: Mark R.
// Function:
// Creates the pop polling object in unique thread
//===========================================================================//
const DWORD InitPopPollThread()
{
	UINT tid;
	g_hPopPollthread = (HANDLE)_beginthreadex(NULL, 0, &PopPollThread, NULL, 0, &tid);
	if (g_hPopPollthread == INVALID_HANDLE_VALUE)
		return ERROR_MAILCOMPONENTS_INIT;

	return 0;
}

const DWORD InitSMTPPollThread()
{
	UINT tid;
	g_hSMTPPollthread = (HANDLE)_beginthreadex(NULL, 0, &SMTPPollThread, NULL, 0, &tid);
	if (g_hSMTPPollthread == INVALID_HANDLE_VALUE)
		return ERROR_MAILCOMPONENTS_INIT;
	
	return 0;
}

//===========================================================================//
// Author: Mark R.
// Function:
// Creates an instance of a pop polling thread
//===========================================================================//
unsigned __stdcall PopPollThread(PVOID pvoid)
{
	g_pPopPollThread = new CPopPollThread();
	assert(g_pPopPollThread);
	
	if (g_pPopPollThread)
		g_pPopPollThread->Run();

	return 0;
}

unsigned __stdcall SMTPPollThread(PVOID pvoid)
{
	g_pSMTPPollThread = new CSMTPPollThread();
	assert(g_pSMTPPollThread);
	
	if (g_pSMTPPollThread)
		g_pSMTPPollThread->Run();
	
	return 0;
}

// force a server parameter refresh right now (accesses
// server parameters singleton object)
MAILCOMPONENTS_API int RefreshServerParams(void)
{
	DebugReporter::Instance().DisplayMessage("RefreshServerParams() - Sending a server refresh signal to CServerParameters", DebugReporter::MAIL);

	if (CServerParameters::GetInstance().Refresh() != 0)
	{
		DebugReporter::Instance().DisplayMessage("RefreshServerParams() - Failed to refresh server parameters", DebugReporter::MAIL);

		LINETRACE(_T("Failed to refresh server parameters\n"));
		assert(0);
		return error_params_refresh_failed;
	}

	LINETRACE(_T("RefreshServerParams() was called successfully\n"));
	DebugReporter::Instance().SetInitialized(0);
	DebugReporter::Instance().DisplayMessage("RefreshServerParams() - Called successfully", DebugReporter::MAIL);
	

	return error_success;
}

// test pop3 auth credentials
MAILCOMPONENTS_API int TestPOP3Auth(const tstring& sHostName, 
									const UINT nPort, 
									const BOOL bIsAPOP, 
									const tstring& sUserName, 
									const tstring& sPassword,
									const UINT nTimeoutSecs,
									const UINT isSSL )
{
	
	int nType = 1;
	int nSSL = isSSL;
	CkGlobal m_glob;
	USES_CONVERSION;
	
	if (isSSL == 2 || isSSL == 3 || isSSL == 5)
	{
		// logpoint: IMAP authentication test was initiated (server = %s, port = %d, username = %s)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_INBOUND_MESSAGING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_IMAP_AUTHTEST_STARTED,
					sHostName.c_str(),
					nPort,
					sUserName.c_str());
		
		nType = 2;
		if(isSSL == 2)
		{
			nSSL = 0;
		}
		else if(isSSL == 3)
		{
			nSSL = 1;
		}
		else
		{
			nSSL = 1;
			nType = 4;
		}
	}
	else
	{
		// logpoint: POP3 authentication test was initiated (server = %s, port = %d, username = %s)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_INBOUND_MESSAGING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_POP3_AUTHTEST_STARTED,
					sHostName.c_str(),
					nPort,
					sUserName.c_str());

		if(isSSL == 2)
		{
			nSSL = 1;
		}
		else if(isSSL == 4)
		{
			nSSL = 1;
			nType = 3;
		}
	}
	
	int nRet = 0;
	bool bRet = false;

	// unlock Chilkat
	bRet = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
	if (bRet != true)
	{
		nRet = 8;
		DebugReporter::Instance().DisplayMessage("TestPOP3Auth() - Failed to unlock Global component", DebugReporter::MAIL);
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									nRet),
						EMS_STRING_ERROR_INITIALIZE_COMPONENT);

		return error_init_components_failed;
	}
		
	if(nType == 1 || nType == 3)
	{
		tstring sDesc(_T("TestPOP3Auth"));
		CPopAccount acc;
		acc.SetDescription(sDesc);
		acc.SetConnTimeoutSecs(nTimeoutSecs);
		acc.SetIsActive(TRUE);
		acc.SetIsAPOP(bIsAPOP);
		acc.SetLeaveOnServer(TRUE);
		acc.SetPassword(sPassword);
		acc.SetUserName(sUserName);
		acc.SetPort(nPort);
		acc.SetServerAddress(sHostName);
		acc.SetIsSSL(nSSL);

		if(nType == 3)
		{
			SYSTEMTIME sysTime;
			int nID = atoi(T2A(sPassword.c_str()));
			TMessageSources ms;
			ms.m_MessageSourceID = nID;
			CODBCQuery query(g_odbcConn);
			ms.Query(query);
			acc.SetOAuthHostID(ms.m_OAuthHostID);
			acc.SetAccessToken(ms.m_AccessToken);			
			TimeStampToSystemTime(ms.m_AccessTokenExpire, sysTime);
			acc.SetATokenExpire(sysTime);
			acc.SetRefreshToken(ms.m_RefreshToken);
			TimeStampToSystemTime(ms.m_RefreshTokenExpire, sysTime);
			acc.SetRTokenExpire(sysTime);
		}
		
		CPOP3Connection conn;
		conn.SetPopAccount(&acc);
		if ( nSSL == 1 )
		{
			nRet = conn.InitSecureConnection();
		}
		else
		{
			nRet = conn.InitConnection();
		}
		
		if (nRet != 0){
			dca::String e;
			dca::String h(sHostName.c_str());
			e.Format("TestPOP3Auth - Error Connect Failed for host %s", h.c_str());
			DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::MAIL);
			return error_connect_failed;
		}

		if(nType == 3)
		{
			nRet = conn.AuthenticateOAuth2POP3();
		}
		else
		{
			nRet = conn.Authenticate();
		}
		
		if (nRet != 0)
		{
			if (nRet == ERROR_APOP_SERVER_UNSUPPORTED){
				dca::String e;
				dca::String h(sHostName.c_str());
				e.Format("TestPOP3Auth - Error Authorization Method Unsupported for host %s", h.c_str());
				DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::MAIL);
				return error_auth_method_unsupported;
			}
			else{
				dca::String e;
				dca::String h(sHostName.c_str());
				e.Format("TestPOP3Auth - Error Authorization Failed for host %s", h.c_str());
				DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::MAIL);
				return error_auth_failed;
			}
		}
		else
		{
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_INBOUND_MESSAGING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_POP3_AUTHTEST_SUCCESS);
		}
	}
	else
	{
		if(nType == 2 || nType == 4)
		{
			// connect to IMAP server
			USES_CONVERSION;
			long maxWaitMillisec = nTimeoutSecs*1000;
			CkImap m_imap;
			// unlock Chilkat
			bRet = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
			if (bRet != true)
			{
				nRet = 8;
				DebugReporter::Instance().DisplayMessage("TestIMAPAuth() - Failed to unlock Global component", DebugReporter::MAIL);
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
											EMSERR_MAIL_COMPONENTS,
											EMS_LOG_INBOUND_MESSAGING,
											ERROR_NO_ERROR),
								EMS_STRING_ERROR_INITIALIZE_COMPONENT);

				return error_init_components_failed;
			}
				
			const char * sslServerHost = T2A(sHostName.c_str());
		    
			bool bSsl = false;
			if(nSSL == 1)
			{
				bSsl = true;
			}
			m_imap.put_Ssl(bool(bSsl));
			m_imap.put_StartTls(false);
			m_imap.put_Port(nPort);
			m_imap.put_ConnectTimeout(maxWaitMillisec);
			
			bRet = m_imap.Connect(sslServerHost);
			if (bRet != true)
			{
				dca::String f;
				f.Format("TestIMAPAuth - Failed to initialize connection to IMAP server (server: %s)", sHostName.c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									ERROR_NO_ERROR),
						EMS_STRING_MAILCOMP_IMAP_AUTHTEST_FAILED_CONNECT,
						sHostName.c_str(),
						nPort,
						sUserName.c_str());

				return error_connect_failed;
			}

            const char * sUser = T2A(sUserName.c_str());
			if(nType == 2)
			{
				const char * sPass = T2A(sPassword.c_str());

				bRet = m_imap.Login(sUser,sPass);
			}
			else
			{
				USES_CONVERSION;
				CODBCQuery query(g_odbcConn);
				TMessageSources ms;
				TOAuthHosts oh;
				int nRet=0;
				tstring aToken;
				tstring rToken;
				tstring sError;
				CkOAuth2 m_oauth2;

				try
				{					
					int nID = atoi(T2A(sPassword.c_str()));
					ms.m_MessageSourceID = nID;
					ms.Query(query);				
					oh.m_OAuthHostID = ms.m_OAuthHostID;
					oh.Query(query);

					// Check the expiration of the AccessToken		
					SYSTEMTIME sysTime;
					GetLocalTime(&sysTime);
					SYSTEMTIME ateTime;
					TimeStampToSystemTime(ms.m_AccessTokenExpire,ateTime);
					TIMESTAMP_STRUCT tsSys;
					long tsSysLen=0;		
					TIMESTAMP_STRUCT tsAte;
					long tsAteLen=0;
					SystemTimeToTimeStamp(sysTime,tsSys);
					SystemTimeToTimeStamp(ateTime,tsAte);
					nRet = OrderTimeStamps( tsSys, tsAte );

					if(nRet != 1)
					{
						const char * tokenEndPoint = T2A(oh.m_TokenEndPoint);
						m_oauth2.put_TokenEndpoint(tokenEndPoint);

						const char * clientID = T2A(oh.m_ClientID);
						m_oauth2.put_ClientId(clientID);
						
						const char * clientSecret = T2A(oh.m_ClientSecret);			
						m_oauth2.put_ClientSecret(clientSecret);

						const char * refreshToken = T2A(ms.m_RefreshToken);					
						m_oauth2.put_RefreshToken(refreshToken);

						// Send the HTTP POST to refresh the access token..
						bRet = m_oauth2.RefreshAccessToken();
						if (bRet != true) {
							const char * lastError = m_oauth2.lastErrorText();
							return 1;
						}
						
						// Update the AccessToken and RefreshToken
						aToken.assign(A2T(m_oauth2.accessToken()));
						rToken.assign(A2T(m_oauth2.refreshToken()));

						time_t now;
						time( &now ); 
						TIMESTAMP_STRUCT tsaTokenExpire;
						long tsaTokenExpireLen=0;
						SecondsToTimeStamp(now + 3599, tsaTokenExpire);

						query.Initialize();
						BINDPARAM_TEXT_STRING(query, aToken);
						BINDPARAM_TEXT_STRING(query, rToken);
						BINDPARAM_TIME (query, tsaTokenExpire);
						BINDPARAM_LONG(query, ms.m_MessageSourceID);
						query.Execute(	_T("UPDATE MessageSources SET AccessToken=?,RefreshToken=?,AccessTokenExpire=? WHERE MessageSourceID=?"));

					}
					else
					{
						const char * accessToken = T2A( ms.m_AccessToken);					
						m_oauth2.put_AccessToken(accessToken);
					}

				}
				catch (ODBCError_t oerr)
				{
					HandleDBError(&oerr);
					return ERROR_DATABASE;
				}
				catch (CEMSException eex)
				{
					dca::String f;
					dca::String e(eex.GetErrorString());
					f.Format("TestIMAPAuth() - EMS Exception occured: %d, %s", eex.GetErrorCode(), e.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					return eex.GetErrorCode();
				}

				const char * userName = T2A(ms.m_AuthUserName);
				m_imap.put_AuthMethod("XOAUTH2");

				bRet = m_imap.Login(userName,m_oauth2.accessToken());
			
			}
			if (bRet != true)
			{
				dca::String f;
				dca::String s(sHostName.c_str());
				dca::String n(sUserName.c_str());			
				f.Format("TestIMAPAuth - IMAP account authentication failed (server: %s, account: %s)", s.c_str(), n.c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									ERROR_NO_ERROR),
						EMS_STRING_MAILCOMP_IMAP_AUTHTEST_FAILED_AUTH,
						sHostName.c_str(),
						nPort,
						sUserName.c_str());
				m_imap.Logout();
				m_imap.dispose();
				return error_auth_failed;
			}
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									ERROR_NO_ERROR),
						EMS_STRING_MAILCOMP_IMAP_AUTHTEST_SUCCESS);
			m_imap.Logout();
			m_imap.dispose();
		}
	}
	return error_success;
}

// test smtp auth credentials
MAILCOMPONENTS_API int TestSMTPAuth(const tstring& sHostName, 
									const UINT nPort, 
									const tstring& sUserName, 
									const tstring& sPassword,
									const UINT nTimeoutSecs,
									const UINT isSSL)
{
	// logpoint: SMTP authentication test was initiated (server = %s, port = %d, username = %s)
	CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_OUTBOUND_MESSAGING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_SMTP_AUTH_TEST_STARTED,
					sHostName.c_str(),
					nPort,
					sUserName.c_str());

	int nRet = 0;
	bool bRet = false;
	CkGlobal m_glob;
	USES_CONVERSION;

	// unlock Chilkat
	bRet = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
	if (bRet != true)
	{
		nRet = 8;
		DebugReporter::Instance().DisplayMessage("TestSMTPAuth() - Failed to unlock Global component", DebugReporter::MAIL);
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									nRet),
						EMS_STRING_ERROR_INITIALIZE_COMPONENT);

		return error_init_components_failed;
	}	

	tstring sDesc(_T("TestSMTPAuth"));
	CSMTPDest dest;
	dest.SetDescription(sDesc);
	dest.SetAuthPass(sPassword);
	dest.SetUsesSMTPAuth(TRUE);
	dest.SetAuthUser(sUserName);
	dest.SetServerAddress(sHostName);
	dest.SetSMTPPort(nPort);
	dest.SetConnTimeoutSecs(nTimeoutSecs);
	dest.SetIsSSL(isSSL > 0 ? 1:0);	
	if(isSSL == 3 || isSSL == 4)
	{
		SYSTEMTIME sysTime;
		int nID = atoi(T2A(sPassword.c_str()));
		TMessageDestinations md;
		md.m_MessageDestinationID = nID;
		CODBCQuery query(g_odbcConn);
		md.Query(query);
		dest.SetOAuthHostID(md.m_OAuthHostID);
		dest.SetAccessToken(md.m_AccessToken);
		TimeStampToSystemTime(md.m_AccessTokenExpire, sysTime);
		dest.SetATokenExpire(sysTime);
		dest.SetRefreshToken(md.m_RefreshToken);
		TimeStampToSystemTime(md.m_RefreshTokenExpire, sysTime);
		dest.SetRTokenExpire(sysTime);
		if(isSSL == 3)
		{
			dest.SetSslMode(1);
		}
		else
		{
			dest.SetSslMode(2);
		}
	}

	CSMTPConnection conn;
	conn.SetMsgDest(&dest);

	nRet = conn.SetHello();
	if (nRet != 0)
		return error_auth_failed;

	if ( isSSL > 0)
	{
		nRet = conn.InitSecureConnection();
	}
	else
	{
		nRet = conn.InitConnection();
	}
	
	if (nRet != 0)
		return error_connect_failed;
	if(isSSL == 3 || isSSL == 4)
	{
		nRet = conn.AuthenticateOAuth2();
	}
	else
	{
		nRet = conn.Authenticate();
	}
	
	if (nRet != 0)
	{
		if (nRet == ERROR_SMTPAUTH_UNSUPPORTED)
			return error_auth_method_unsupported;
		else
			return error_auth_failed;
	}

	CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_OUTBOUND_MESSAGING,
								ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_SMTP_AUTH_TEST_SUCCESS);

	return error_success;
}

// this is an installer-friendly wrapper around the pop3
// auth credential test
MAILCOMPONENTS_API int TestPOP3Auth2(const char* sHostName, 
									 const UINT nPort, 
									 const bool bIsAPOP, 
								  	 const char* sUserName, 
									 const char* sPassword,
									 const UINT nTimeoutSecs,
									 const UINT isSSL)
{
	USES_CONVERSION;
	return TestPOP3Auth(A2T(sHostName),
						nPort,
						bIsAPOP,
						A2T(sUserName),
						A2T(sPassword),
						nTimeoutSecs,
						isSSL);
}

// this is an installer-friendly wrapper around the smtp
// auth credential test
MAILCOMPONENTS_API int TestSMTPAuth2(const char* sHostName, 
									 const UINT nPort, 
									 const char* sUserName, 
									 const char* sPassword,
									 const UINT nTimeoutSecs,
									 const UINT isSSL)
{
	USES_CONVERSION;
	return TestSMTPAuth(A2T(sHostName),
						nPort,
						A2T(sUserName),
						A2T(sPassword),
						nTimeoutSecs,
						isSSL);
}