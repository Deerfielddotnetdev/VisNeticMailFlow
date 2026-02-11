// PopPollThread.cpp: implementation of the CPopPollThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PopPollThread.h"
#include "MailComponents.h"
#include "MessageIO.h"
#include "ODBCConn.h"
#include "ODBCQuery.h"
#include "SingleCrit.h"
#include "CriticalSection.h"
#include "PopAccount.h"
#include "RetrievalSession.h"
#include "EmailDate.h"
#include "ServerParameters.h"
#include "DateFns.h"
#include "RegistryFns.h"

extern dca::Mutex g_csDB;
extern CODBCConn g_odbcConn;

dca::Mutex g_sessionLock_1;

const TCHAR sConnString[] = _T("DRIVER={%s};SERVER=%s;DATABASE=%s;UID=%s;PWD=%s;");
const TCHAR sConnStringSecure[] = _T("DRIVER={%s};SERVER=%s;DATABASE=%s;UID=%s;PWD=%s;Encrypt=yes;");
const TCHAR sConnStringSecureTrust[] = _T("DRIVER={%s};SERVER=%s;DATABASE=%s;UID=%s;PWD=%s;Encrypt=yes;TrustServerCertificate=yes;");

const TCHAR sConnStringWia[] = _T("DRIVER={%s};SERVER=%s;DATABASE=%s;Trusted_Connection=yes;");
const TCHAR sConnStringWiaSecure[] = _T("DRIVER={%s};SERVER=%s;DATABASE=%s;Trusted_Connection=yes;Encrypt=yes;");
const TCHAR sConnStringWiaSecureTrust[] = _T("DRIVER={%s};SERVER=%s;DATABASE=%s;Trusted_Connection=yes;Encrypt=yes;TrustServerCertificate=yes;");

// thread start-up functions
const DWORD InitPopCheckConn(CPopAccount* pPopAccount);
unsigned __stdcall PopCheckConn(PVOID pvoid);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPopPollThread::CPopPollThread():
	m_loggedDiskFullError(0)
{
	m_hWaitHandles[wait_handle_stop] = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWaitHandles[check_right_now] = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_nCheckIntervalSecs = 15;

	m_nFailureMark = 0;
	m_nSuccessMark = 0;
	
	m_bLoggedNoAccountsWarning = false;

	m_nServerID = 0;
}

CPopPollThread::~CPopPollThread()
{
	if (m_hWaitHandles[wait_handle_stop])
		CloseHandle(m_hWaitHandles[wait_handle_stop]);

	if (m_hWaitHandles[check_right_now])
		CloseHandle(m_hWaitHandles[check_right_now]);

	if (!DrainSessionPool())
		assert(0);
}

const UINT CPopPollThread::Run()
{
	DWORD dwRet;
	UINT nCheck;
	bool bRun = true;
	TIMESTAMP_STRUCT Now;	
	
	// set LastMailCheck registry key	
	GetTimeStamp( Now );
	tm t;
	ZeroMemory(&t, sizeof(t));
	t.tm_hour = Now.hour;
	t.tm_mday = Now.day;
	t.tm_min = Now.minute;
	t.tm_sec = Now.second;
	t.tm_year = Now.year - 1900;
	t.tm_mon = Now.month - 1;
	t.tm_wday = 0;
	time_t t1 = mktime(&t);	
	WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMailCheck"), t1 );
	

	DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Fill the POP3 session pool", DebugReporter::MAIL);
	// fill the POP3 session pool
	if (!FillSessionPool())
	{
		DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Failed to fill POP3 session pool", DebugReporter::MAIL);
	}

	DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Use waitformultiple objects here to run CheckPopAccounts at 15 second intervals", DebugReporter::MAIL);
	// use waitformultiple objects here to run CheckPopAccounts at 1-min intervals
	while (bRun)
	{
		try
		{
			DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - while (bRun) ResetEvent", DebugReporter::MAIL);
			
			ResetEvent(m_hWaitHandles[check_right_now]);
			m_bCheckNow = FALSE;
			
			DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - WaitForMultipleObjects", DebugReporter::MAIL);
			
			dwRet = WaitForMultipleObjects(2, m_hWaitHandles, FALSE, m_nCheckIntervalSecs * 1000);

			DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Switch on dwRet", DebugReporter::MAIL);
			
			switch (dwRet)
			{
				// stop processing
				case (WAIT_OBJECT_0 + wait_handle_stop):
					{
						DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - stop processing", DebugReporter::MAIL);
						bRun = false;
					}
					break;

				// new processing interval
				case (WAIT_OBJECT_0 + check_right_now):
				case WAIT_TIMEOUT:

					DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Check now or wait timeout", DebugReporter::MAIL);
					
					if(!_DBMaintenanceMutex.IsLocked())
					{
						// if db connection isn't open
						if (!g_odbcConn.IsConnected())
						{
							DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - db connection is not open. Will try to open", DebugReporter::MAIL);
							// try to open it now; if it won't open,
							// wait until the next processing interval
							UINT nRet = OpenDBConn();
							if (nRet != 0)
							{						
								DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Unable to establish connection to database", DebugReporter::MAIL);
								// logpoint: Unable to establish connection to database
								CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
															EMSERR_MAIL_COMPONENTS,
															EMS_LOG_INBOUND_MESSAGING,
															nRet),
												EMS_STRING_MAILCOMP_DB_CONN_FAILURE);

								break;
							}
						}

						DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Ready to check pop accounts", DebugReporter::MAIL);
						
						// set LastMailCheck registry key
						GetTimeStamp( Now );
						ZeroMemory(&t, sizeof(t));
						t.tm_hour = Now.hour;
						t.tm_mday = Now.day;
						t.tm_min = Now.minute;
						t.tm_sec = Now.second;
						t.tm_year = Now.year - 1900;
						t.tm_mon = Now.month - 1;
						t.tm_wday = 0;
						time_t t1 = mktime(&t);
						WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMailCheck"), t1 );

						// otherwise it's time to check the mail
						nCheck = CheckPopAccounts();

						if (nCheck == ERROR_NO_ERROR)
						{
							DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - CheckPopAccounts returned 0", DebugReporter::MAIL);
						
							m_bLoggedNoAccountsWarning = false;
						}
						else
						{
							if (nCheck == ERROR_NO_POP_ACCOUNTS_DEFINED)
							{
								if( !m_bLoggedNoAccountsWarning )
								{
									// logpoint: There are currently no POP3 accounts to check (none defined)
									CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
																EMSERR_MAIL_COMPONENTS,
																EMS_LOG_INBOUND_MESSAGING,
																ERROR_NO_POP_ACCOUNTS_DEFINED),
													EMS_STRING_MAILCOMP_NO_POP_ACCOUNTS_DEFINED);
									
									CreateAlert( EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_MAILCOMP_NO_POP_ACCOUNTS_DEFINED));

									m_bLoggedNoAccountsWarning = true;

								}
							}
							else if(nCheck == ERROR_DISK_DATA_SIZE)
							{
								if(!m_loggedDiskFullError)
								{
									DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Unable to get mail not enough room in database or drive", DebugReporter::MAIL);
									// logpoint: Unable to get mail not enough room in database or drive
									CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
																EMSERR_MAIL_COMPONENTS,
																EMS_LOG_INBOUND_MESSAGING,
																nCheck),
																EMS_STRING_MAILCOMP_DISKDATASIZE);
									
									CreateAlert(EMS_ALERT_EVENT_LOW_DISK_SPACE, EMS_STRING_MAILCOMP_DISKDATASIZE);

									m_loggedDiskFullError = 1;
								}
							}
							else
							{
								DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Unable to get mail not enough room in database or drive", DebugReporter::MAIL);
								// logpoint: Unable to check POP3 email accounts
								CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
															EMSERR_MAIL_COMPONENTS,
															EMS_LOG_INBOUND_MESSAGING,
															nCheck),
															EMS_STRING_MAILCOMP_POP3_ACCOUNTS_CHECK_FAILURE);
							}
						}
					}
					else
					{
						DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - DBMaintenance Running", DebugReporter::MAIL);
					}
					DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Check now or wait timeout leaving", DebugReporter::MAIL);
					
					break;

				// anything else is an error
				default:
					DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - Unexpected error", DebugReporter::MAIL);
					break;
			}

			DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - while (bRun) looping", DebugReporter::MAIL);
		}
		catch(dca::Exception e)
		{
			dca::String f(e.GetMessage());
			f.insert(0, "CPopPollThread::Run() - ");
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);			
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("CPopPollThread::Run() - An unknown or undefined exception has occurred", DebugReporter::MAIL);
		}
	}

	DebugReporter::Instance().DisplayMessage("CPopPollThread::Run - POP3 polling thread has terminated normally", DebugReporter::MAIL);

	return 0; // success
}

// stop all active POP sessions
const UINT CPopPollThread::Stop()
{
	dca::Lock lock(g_sessionLock_1);

	DebugReporter::Instance().DisplayMessage("CPopPollThread::Stop - Stopping POP3 session pool sessions...", DebugReporter::MAIL);
	for (	SESSION_LIST::iterator cur(m_sessions.begin());
			cur != m_sessions.end();
			++cur)
	{
		CRetrievalSession* pSession = (CRetrievalSession*)*cur;
		if (pSession)
			pSession->StopSession();
	}

	// stop this thread
	DebugReporter::Instance().DisplayMessage("CPopPollThread::Stop - Setting POP3 stop event handle", DebugReporter::MAIL);
	SetEvent(m_hWaitHandles[wait_handle_stop]);

	return 0;
}

// generates a container of CPopAccount objects based upon database
// records, then immediately closes the database connection
const UINT CPopPollThread::CheckPopAccounts()
{
	DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - entering", DebugReporter::MAIL);

	// Check disk size....
	if(IsDatabaseOrDiskFull())
	{
		return ERROR_DISK_DATA_SIZE;
	}

	m_loggedDiskFullError = 0;	
	bool bAccountsFound = false;
	bool bIsInOfficeHours = false;
	bool bOfficeHoursSet = false;
	bool bUseDateFilters = false;
	SYSTEMTIME curTime;
	ZeroMemory(&curTime, sizeof(SYSTEMTIME));
	GetLocalTime(&curTime);
	int nDayOfWeek = curTime.wDayOfWeek + 1;
	
	dca::Lock lockSession(g_sessionLock_1);

	// lock access to the database object
	dca::Lock lockDB(g_csDB);

	try
	{
		m_nServerID = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_SERVER_ID, 1);
		
		CODBCQuery query(g_odbcConn);
		query.Initialize();
		
		MessageSources_t ms;
		vector<MessageSources_t> m_ms;
		vector<MessageSources_t>::iterator msIter;

		BINDCOL_LONG(query, ms.m_MessageSourceID);
		BINDCOL_WCHAR(query, ms.m_RemoteAddress);
		BINDCOL_WCHAR(query, ms.m_Description);
		BINDCOL_LONG(query, ms.m_RemotePort);
		BINDCOL_WCHAR(query, ms.m_AuthUserName);
		BINDCOL_WCHAR(query, ms.m_AuthPassword);
		BINDCOL_LONG(query, ms.m_IsAPOP);
		BINDCOL_LONG(query, ms.m_LeaveCopiesOnServer);
		BINDCOL_LONG(query, ms.m_CheckFreqMins);
		BINDCOL_LONG(query, ms.m_MaxInboundMsgSize);
		BINDCOL_LONG(query, ms.m_ConnTimeoutSecs);
		BINDCOL_TIME(query, ms.m_LastChecked);
		BINDCOL_LONG(query, ms.m_MessageDestinationID);
		BINDCOL_LONG(query, ms.m_MessageSourceTypeID);
		BINDCOL_LONG(query, ms.m_UseReplyTo);					
		BINDCOL_LONG(query, ms.m_IsSSL);
		BINDCOL_LONG(query, ms.m_OfficeHours);
		BINDCOL_LONG(query, ms.m_ZipAttach);		
		BINDCOL_LONG(query, ms.m_DupMsg);
		BINDCOL_LONG(query, ms.m_LeaveCopiesDays);
		BINDCOL_LONG(query, ms.m_SkipDownloadDays);
		BINDCOL_LONG(query, ms.m_DateFilters);
		BINDCOL_LONG(query, ms.m_ErrorCode);
		BINDCOL_TIME(query, ms.m_AccessTokenExpire);
		BINDCOL_LONG(query, ms.m_OAuthHostID);
		BINDPARAM_LONG(query, m_nServerID );
		BINDPARAM_LONG(query, m_nFailureMark);

		// execute the query
		query.Execute(	_T("select	MessageSourceID, RemoteAddress, Description, RemotePort, AuthUserName, ")
						_T("CASE WHEN AuthPassword = 'xëÉT.;}kÚÅ' THEN (SELECT Password FROM Agents WHERE LoginName=AuthUserName AND IsEnabled=1 AND IsDeleted=0) ELSE AuthPassword END, IsAPOP, LeaveCopiesOnServer, CheckFreqMins, MaxInboundMsgSize, ")
						_T("ConnTimeoutSecs, LastChecked, MessageDestinationID, MessageSourceTypeID, UseReplyTo, IsSSL, OfficeHours, ZipAttach, DupMsg, LeaveCopiesDays, SkipDownloadDays, DateFilters, ErrorCode, AccessTokenExpire, OAuthHostID, AccessToken, RefreshToken ")
						_T("from MessageSources ")
						_T("where IsActive = 1 and ServerID=? and ServerID IN (SELECT ServerID FROM ServerTasks st WHERE st.ServerTaskTypeID=4 AND st.ServerID=ServerID) and MessageSourceID >= ?"));

		m_ms.clear();

		while( query.Fetch() == S_OK )
		{
			GETDATA_TEXT( query, ms.m_AccessToken );
			GETDATA_TEXT( query, ms.m_RefreshToken );
		
			if(ms.m_DateFilters == 1)
			{
				bUseDateFilters = true;
			}
			m_ms.push_back( ms );			
		}
		
		TIMESTAMP_STRUCT Now;
		GetTimeStamp( Now );

		vector<OfficeHours_t> too;
		vector<OfficeHours_t>::iterator tooIter;

		query.Initialize();					
		OfficeHours_t oh;
		BINDCOL_LONG( query, oh.m_TypeID );
		BINDCOL_LONG( query, oh.m_StartHr );
		BINDCOL_LONG( query, oh.m_StartMin );
		BINDCOL_LONG( query, oh.m_StartAmPm );
		BINDCOL_LONG( query, oh.m_EndHr );
		BINDCOL_LONG( query, oh.m_EndMin );
		BINDCOL_LONG( query, oh.m_EndAmPm );
		BINDCOL_LONG( query, oh.m_ActualID );		
		BINDPARAM_TIME_NOLEN( query, Now );
		BINDPARAM_LONG( query, nDayOfWeek );					
		BINDPARAM_LONG( query, nDayOfWeek );					
		query.Execute(	_T("SELECT TypeID,StartHr,StartMin,StartAmPm,EndHr,EndMin,EndAmPm,ActualID ")
						_T("FROM OfficeHours ")
						_T("WHERE (? BETWEEN TimeStart AND TimeEnd AND TypeID IN (1,2)) OR (TypeID=0 AND ActualID=0 AND DayID=?) OR (TypeID=3 AND ActualID<>0 AND DayID=?)")
						_T("ORDER BY OfficeHourID DESC"));
		while( query.Fetch() == S_OK )
		{
			if(oh.m_TypeID == 1)
			{
				bIsInOfficeHours = false;
				bOfficeHoursSet = true;
				break;
			}
			else if(oh.m_TypeID == 2)
			{
				bIsInOfficeHours = true;
				bOfficeHoursSet = true;
				break;
			}
			else
			{
				too.push_back(oh);
			}
		}
		
		vector<DateFilters_t> m_df;
		
		if(bUseDateFilters)
		{
			DateFilters_t df;				

			query.Initialize();	
			BINDCOL_LONG(query, df.m_DateFilterID);
			BINDCOL_WCHAR(query, df.m_Description);
			BINDCOL_LONG(query, df.m_WhereToCheck);
			BINDCOL_WCHAR(query, df.m_RegEx);
			BINDCOL_WCHAR(query, df.m_Header);
			BINDCOL_LONG(query, df.m_HeaderValueTypeID);
			BINDCOL_WCHAR(query, df.m_HeaderValue);
			query.Execute(	_T("select DateFilterID, Description, WhereToCheck, RegEx, Header, HeaderValueTypeID, HeaderValue ")
							_T("from DateFilters ORDER BY Description"));

			while( query.Fetch() == S_OK )
			{
				m_df.push_back( df );			
			}
		}

		for( msIter = m_ms.begin(); msIter != m_ms.end(); msIter++ )
		{
			bAccountsFound = true;

			// if any session from the worker pool is actively processing this message
			// source, then we want to skip over it
			bool b = false;
			for (SESSION_LIST::iterator cur = m_sessions.begin(); cur != m_sessions.end(); ++cur)
			{
				CRetrievalSession* pSession = (CRetrievalSession*)*cur;
				if (pSession->m_pPopAccount && 
					(pSession->m_pPopAccount->GetMessageSourceID() == msIter->m_MessageSourceID) && pSession->m_bIsBusy)
				{
					dca::String f;
					f.Format("CPopPollThread::CheckPopAccounts - Msg source %d not checked, already being checked in another session with threadID [%d].", pSession->m_pPopAccount->GetMessageSourceID(),pSession->m_nThreadID );
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					b = true;
					break;
				}
			}
			if (b) continue;

			DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - create a pop account object on the heap", DebugReporter::MAIL);
			// create a pop account object on the heap
			CPopAccount* pPop = new CPopAccount();
			if (pPop)
			{
				// set pop account parameters
				pPop->SetMessageSourceID(msIter->m_MessageSourceID);
				pPop->SetUserName(msIter->m_AuthUserName);
				pPop->SetServerAddress(msIter->m_RemoteAddress);
				pPop->SetDescription(msIter->m_Description);

				// MER 12/5/02 - now expecting password string to be encrypted
				try
				{
					CEMSString sDecrypt;
					sDecrypt.assign(msIter->m_AuthPassword);
					sDecrypt.Decrypt();
					pPop->SetPassword(sDecrypt.c_str());
				}
				catch (CEMSException EMSException)
				{
					dca::String f;
					dca::String r(msIter->m_RemoteAddress);
					dca::String a(msIter->m_AuthUserName);
					f.Format("CPopPollThread::CheckPopAccounts - An error occured while attempting to decrypt POP3 account password (server: %s, username: %s). Account cannot be checked", r.c_str(), a.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					// An error occured while attempting to decrypt POP3 account password (server: %s, username: %s). Account cannot be checked.
					CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
												EMSERR_MAIL_COMPONENTS,
												EMS_LOG_INBOUND_MESSAGING,
												ERROR_DECRYPTING_DATA),
									EMS_STRING_MAILCOMP_ERROR_DECRYPTING_POP3_PASSWORD,
									msIter->m_RemoteAddress,
									msIter->m_AuthUserName);

					// can't see the decryption process failing under normal circumstances,
					// should be safe just to log this event and skip the account in this case
					continue;
				}
			
				pPop->SetLeaveOnServer((BOOL)msIter->m_LeaveCopiesOnServer);
				pPop->SetIsAPOP((BOOL)msIter->m_IsAPOP);
				pPop->SetCheckFreqMins(msIter->m_CheckFreqMins);
				pPop->SetConnTimeoutSecs(msIter->m_ConnTimeoutSecs);
				pPop->SetPort(msIter->m_RemotePort);
				pPop->SetDestinationID(msIter->m_MessageDestinationID);
				pPop->SetMaxInboundMsgSize(msIter->m_MaxInboundMsgSize);
				pPop->SetMessageSourceTypeID(msIter->m_MessageSourceTypeID);
				pPop->SetUseReplyTo(msIter->m_UseReplyTo);
				pPop->SetIsSSL((BOOL)msIter->m_IsSSL);				
				pPop->SetZipAttach(msIter->m_ZipAttach);
				pPop->SetDupMsg(msIter->m_DupMsg);
				pPop->SetLeaveCopiesDays(msIter->m_LeaveCopiesDays);
				pPop->SetSkipDownloadDays(msIter->m_SkipDownloadDays);
				pPop->SetDateFilters(msIter->m_DateFilters);
				pPop->SetErrorCode(msIter->m_ErrorCode);
				if(msIter->m_OAuthHostID > 0)
				{
					pPop->SetAccessToken(msIter->m_AccessToken);
					pPop->SetRefreshToken(msIter->m_RefreshToken);				
					pPop->SetOAuthHostID(msIter->m_OAuthHostID);
					SYSTEMTIME ateTime;
					if ((msIter->m_AccessTokenExpireLen != -1) &&
						(CEmailDate::SQLTimeToSystemTime(msIter->m_AccessTokenExpire, ateTime)))
						pPop->SetATokenExpire(ateTime);
					else
					{
						DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - the value for access token expire in the database is invalid (probably NULL), the account needs to be reauthorized.", DebugReporter::MAIL);
					}
				}

				if(msIter->m_DateFilters == 1)
				{
					pPop->SetDFVector(m_df);
				}
				
				SYSTEMTIME syTime;
				if ((msIter->m_LastCheckedLen != -1) &&
					(CEmailDate::SQLTimeToSystemTime(msIter->m_LastChecked, syTime)))
					pPop->SetLastChecked(syTime);
				else
				{
					DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - the value in the database is invalid (probably NULL), so let's force an immediate check, which will update the lastchecked date/time", DebugReporter::MAIL);
					// the value in the database is invalid (probably NULL), so let's
					// force an immediate check, which will update the lastchecked date/time
					GetLocalTime(&syTime);
					pPop->SetLastChecked(syTime);
					m_bCheckNow = TRUE;
				}

				// check the "LastChecked" setting to see if we actually need to check this account!
				// note: if we were triggered by "CheckNow()", then the answer is always 'yes'
				UINT nDiffSecs = 0;
				const UINT nFreq = pPop->GetCheckFreqMins();
				const SYSTEMTIME lastProc = pPop->GetLastChecked();
				
				if (!CEmailDate::GetDiffSecs(lastProc, curTime, nDiffSecs))
				{
					DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - GetDiffSecs failed in CPopPollThread::CheckPopAccounts()", DebugReporter::MAIL);
					assert(0);
				}

				// if the number of minutes elapsed is less than the processing interval,
				// then move on to the next one (it's not time yet)
				// note: if CheckNow() triggered this, we always want to send!
				UINT nMinsEl = nDiffSecs / 60;
				
				{
					dca::String f;
					f.Format("CPopPollThread::CheckPopAccounts - %d mins have elapsed since msg source %d was last triggered (check freq: %d mins)", nMinsEl, pPop->GetMessageSourceID(), nFreq);
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				}

				if ((nMinsEl < nFreq) && !m_bCheckNow)
				{
					dca::String f;
					f.Format("CPopPollThread::CheckPopAccounts - Msg source %d not checked, elapsed mins (%d) less than check freq (%d)", pPop->GetMessageSourceID(), nMinsEl, nFreq);
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					delete pPop;
					continue; // next msg source
				}
				
				// Does this MessageSource use Office Hours?
				if ( msIter->m_OfficeHours > 0 && !m_bCheckNow)
				{
					if(!bOfficeHoursSet)
					{
						for( tooIter = too.begin(); tooIter != too.end(); tooIter++ )
						{
							if(tooIter->m_ActualID == msIter->m_MessageSourceID && !bOfficeHoursSet)
							{
								bIsInOfficeHours = InOfficeHours(tooIter->m_StartHr,tooIter->m_StartMin,tooIter->m_StartAmPm,tooIter->m_EndHr,tooIter->m_EndMin,tooIter->m_EndAmPm);
								bOfficeHoursSet = true;
							}
							else if(tooIter->m_ActualID == 0 && !bOfficeHoursSet)
							{
								bIsInOfficeHours = InOfficeHours(tooIter->m_StartHr,tooIter->m_StartMin,tooIter->m_StartAmPm,tooIter->m_EndHr,tooIter->m_EndMin,tooIter->m_EndAmPm);
								bOfficeHoursSet = true;
							}
						}
						if(!bOfficeHoursSet)
						{
							bIsInOfficeHours = true;
						}
					}
					
					int nIsInOfficeHours = 0;
					if(bIsInOfficeHours)
						nIsInOfficeHours = 1;

					dca::String f;				

					switch( msIter->m_OfficeHours )
					{
					case 0:
						f.Format("CPopPollThread::CheckPopAccounts - Msg source %d set to always check - office hours: %d", pPop->GetMessageSourceID(),nIsInOfficeHours);
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						break;

					case 1:
						f.Format("CPopPollThread::CheckPopAccounts - Msg source %d set to check during office hours - office hours: %d", pPop->GetMessageSourceID(),nIsInOfficeHours);
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						break;

					case 2:
						f.Format("CPopPollThread::CheckPopAccounts - Msg source %d set to check outside office hours - office hours: %d", pPop->GetMessageSourceID(),nIsInOfficeHours);
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						break;				
					}
										
					if( !bIsInOfficeHours && msIter->m_OfficeHours == 1)
					{
						DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - Message source not checked outside Office Hours!", DebugReporter::MAIL);
						delete pPop;
						continue; 
					}
					else if( bIsInOfficeHours && msIter->m_OfficeHours == 2)
					{
						DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - Message source not checked during Office Hours!", DebugReporter::MAIL);
						delete pPop;
						continue; 
					}
				}
			
				DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - Message source is being checked!", DebugReporter::MAIL);

				// assign this account to a session
				bool bAssigned = AssignWorkToSession(pPop);
				if (!bAssigned)
				{
					dca::String f;
					f.Format("CPopPollThread::CheckPopAccounts() - Failed to find available session for pop account #%d", pPop->GetMessageSourceID());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					delete pPop;
					break;
				}
			}
		}
		for (SESSION_LIST::iterator cur = m_sessions.begin(); cur != m_sessions.end(); ++cur)
		{
			CRetrievalSession* pSession = (CRetrievalSession*)*cur;
			if (pSession->m_pPopAccount)
			{
				int nBusy = 0;
				if(pSession->m_bIsBusy){nBusy = 1;}
				dca::String f;
				f.Format("CPopPollThread::CheckPopAccounts() - Session with MessageSourceID [%d] threadID [%d] with isBusy [%d].", pSession->m_pPopAccount->GetMessageSourceID(),pSession->m_nThreadID, nBusy );
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			}
			else
			{
				int nBusy = 0;
				if(pSession->m_bIsBusy){nBusy = 1;}
				dca::String f;
				f.Format("CPopPollThread::CheckPopAccounts() - Session without m_pPopAccount threadID [%d] with isBusy [%d].", pSession->m_nThreadID, nBusy );
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			}
		}
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);

		DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - Failed to retrieve POP3 account information from database", DebugReporter::MAIL);
		// logpoint: Failed to retrieve POP3 account information from database
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									ERROR_DATABASE),
						EMS_STRING_MAILCOMP_POPACCOUNT_DB_READ_FAILURE);

		return ERROR_DATABASE;
	}

	if (m_nFailureMark <= m_nSuccessMark)
		m_nFailureMark = m_nSuccessMark = 0;

	// if there are no POP accounts defined
	if (!bAccountsFound)
	{
		if ( CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_MSG_COLLECTOR, 0) == 1 )
		{
			DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - returning no pop accounts found", DebugReporter::MAIL);
	
			return ERROR_NO_POP_ACCOUNTS_DEFINED;
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckNow - POP3 polling disabled for this server", DebugReporter::MAIL);
		}
	}
	
	DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckPopAccounts - returning 0", DebugReporter::MAIL);
	
	return 0; // success
}

void CPopPollThread::CheckNow()
{
	DebugReporter::Instance().DisplayMessage("CPopPollThread::CheckNow - POP3 polling thread received CheckNow() signal", DebugReporter::MAIL);

	// this will trigger immediate checking of all
	// defined POP3 accounts
	if (!SetEvent(m_hWaitHandles[check_right_now]))
		assert(0);

	m_bCheckNow = TRUE;
}

bool CPopPollThread::FillSessionPool()
{
	dca::Lock lock(g_sessionLock_1);

	// pool size is user-definable; the pool size effectively limits
	// how many threads of this type can run concurrently
	DebugReporter::Instance().DisplayMessage("CPopPollThread::FillSessionPool - Filling POP3 session pool...", DebugReporter::MAIL);
	int nPoolSize = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_MAX_CON_POP3_THREADS, 5);
	for (int i=0; i<nPoolSize; i++)
	{
		CRetrievalSession* pSession = new CRetrievalSession();
		assert(pSession);
		if (pSession)
			m_sessions.push_back(pSession);
	}

	DebugReporter::Instance().DisplayMessage("CPopPollThread::FillSessionPool - POP3 session pool has been filled", DebugReporter::MAIL);

	return true;
}

bool CPopPollThread::DrainSessionPool()
{
	DebugReporter::Instance().DisplayMessage("CPopPollThread::DrainSessionPool - Draining POP3 session pool...", DebugReporter::MAIL);
	for(	SESSION_LIST::iterator cur(m_sessions.begin());
			cur != m_sessions.end();
			++cur)
	{
		delete *cur;
	}

	m_sessions.clear();

	DebugReporter::Instance().DisplayMessage("CPopPollThread::DrainSessionPool - POP3 session pool has been drained", DebugReporter::MAIL);

	return true;
}

bool CPopPollThread::AssignWorkToSession(CPopAccount *const pPopAccount)
{
	// now try to find a session in the pool that isn't busy
	for	(	SESSION_LIST::iterator cur = m_sessions.begin(); 
			cur != m_sessions.end(); 
			++cur)
	{
		CRetrievalSession* pSession = (CRetrievalSession*)*cur;
		if (!pSession->m_bIsBusy)
		{
			dca::String f;
			dca::String n(pPopAccount->GetUserName().c_str());
			dca::String a(pPopAccount->GetServerAddress().c_str());
			f.Format("CPopPollThread::AssignWorkToSession - Assigned POP account (%s, %s) to worker session", n.c_str(), a.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			
			if (pSession->StartSession(pPopAccount) != 0)
				break;

			m_nSuccessMark = pPopAccount->GetMessageSourceID();
			DebugReporter::Instance().DisplayMessage("CPopPollThread::AssignWorkToSession returning true", DebugReporter::MAIL);
			return true;
		}
	}

	m_nFailureMark = pPopAccount->GetMessageSourceID();
	DebugReporter::Instance().DisplayMessage("CPopPollThread::AssignWorkToSession returning false", DebugReporter::MAIL);			
	return false;
}

bool CPopPollThread::IsDatabaseOrDiskFull()
{
	try
	{
		DebugReporter::Instance().DisplayMessage("* Entered CPopPollThread::IsDatabaseOrDiskFull function", DebugReporter::MAIL);

		DWORD   maxSpaceMB;			// maximum Disk or Database space
									// Anything under this will generate an error

		// Create connection buffer and zero it out
		wchar_t sConnBuffer[1024];
		wmemset(sConnBuffer, 0x00, 1024);
		dca::WString sDBName;
		dca::WString sPath;			// Added on July 13, 2006 by Mark Mohr
		dca::RegistryData mfReg;

		try
		{
			mfReg.GetDBName(sDBName);
			
			// Get install path for Mailflow and parse out drive
			mfReg.GetInstallPath(sPath);
			wchar_t sDrive[_MAX_DRIVE];
			wmemset(sDrive, 0x00, _MAX_DRIVE);
			_wsplitpath(sPath.c_str(), sDrive,0,0,0);

			// Assign drive letter to path
			sPath.assign(sDrive);
		}
		catch(...)
		{
			sDBName.assign(_T("VisNetic MailFlow"));
		}

		dca::String x(sPath.c_str());
		DebugReporter::Instance().DisplayMessage(x.c_str(), DebugReporter::MAIL);
		x.AssignUnicode(sDBName.c_str());
		DebugReporter::Instance().DisplayMessage(x.c_str(), DebugReporter::MAIL);

		dca::WString sqlServer;		// The sqlServer
		mfReg.GetSQLServer(sqlServer);

		// Get Info from registry
		if(mfReg.GetUseWIA())
		{
			dca::WString sRunAsUser;
			dca::WString sRunAsPassword;
			dca::WString sRunAsDomain;
			dca::WString sDriver("SQL Server");
			UINT nSecure = 0;
			UINT nTrust = 0;

			mfReg.GetRunAsUser(sRunAsUser);
			mfReg.GetRunAsPassword(sRunAsPassword);
			mfReg.GetRunAsDomain(sRunAsDomain);
			
			if(GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,  tstring(_T("OdbcDriver")).c_str(), sDriver))
			{
				DebugReporter::Instance().DisplayMessage("CPopPollThread::IsDatabaseOrDiskFull Get ODBC Driver from registry failed.", DebugReporter::MAIL);
			}
			if(GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("OdbcSecure"), nSecure ))
			{
				DebugReporter::Instance().DisplayMessage("CPopPollThread::IsDatabaseOrDiskFull Get ODBC Secure from registry failed.", DebugReporter::MAIL);
			}
			if(GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("OdbcTrust"), nTrust ))
			{
				DebugReporter::Instance().DisplayMessage("CPopPollThread::IsDatabaseOrDiskFull Get ODBC Trust from registry failed.", DebugReporter::MAIL);
			}

			if(nSecure && nTrust)
			{
				_sntprintf(sConnBuffer, 1023, sConnStringWiaSecureTrust, sDriver.c_str(), sqlServer.c_str(), sDBName.c_str());
			}
			else if(nSecure)
			{
				_sntprintf(sConnBuffer, 1023, sConnStringWiaSecure, sDriver.c_str(), sqlServer.c_str(), sDBName.c_str());
			}
			else
			{
				_sntprintf(sConnBuffer, 1023, sConnStringWia, sDriver.c_str(), sqlServer.c_str(), sDBName.c_str());
			}
		}
		else
		{
			dca::WString saPass;		// The sa users password
			dca::WString saUser;		// The sa user
			dca::WString sDriver("SQL Server");
			UINT nSecure = 0;
			UINT nTrust = 0;

			mfReg.GetSQLSaUser(saUser);
			mfReg.GetSQLSaPassword(saPass);

			if(GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,  tstring(_T("OdbcDriver")).c_str(), sDriver))
			{
				DebugReporter::Instance().DisplayMessage("CPopPollThread::IsDatabaseOrDiskFull Get ODBC Driver from registry failed.", DebugReporter::MAIL);
			}
			if(GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("OdbcSecure"), nSecure ))
			{
				DebugReporter::Instance().DisplayMessage("CPopPollThread::IsDatabaseOrDiskFull Get ODBC Secure from registry failed.", DebugReporter::MAIL);
			}
			if(GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("OdbcTrust"), nTrust ))
			{
				DebugReporter::Instance().DisplayMessage("CPopPollThread::IsDatabaseOrDiskFull Get ODBC Trust from registry failed.", DebugReporter::MAIL);
			}

			if(nSecure && nTrust)
			{
				_sntprintf(sConnBuffer, 1023, sConnStringSecureTrust, sDriver.c_str(), sqlServer.c_str(), sDBName.c_str(), saUser.c_str(), saPass.c_str());
			}
			else if(nSecure)
			{
				_sntprintf(sConnBuffer, 1023, sConnStringSecure, sDriver.c_str(), sqlServer.c_str(), sDBName.c_str(), saUser.c_str(), saPass.c_str());
			}
			else
			{
				_sntprintf(sConnBuffer, 1023, sConnString, sDriver.c_str(), sqlServer.c_str(), sDBName.c_str(), saUser.c_str(), saPass.c_str());
			}
		}

		maxSpaceMB = mfReg.GetMinimumSpaceMB();	// default is 100 MB

		dca::String f;
		f.Format("MinimumSpace %d", maxSpaceMB);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

		// Create SQL Enviroment and SQL connection Objects
		dca::DataConnector dc(0);
		dc.SetDSN(sConnBuffer);

		LONG bufLen = lstrlen(sDBName.c_str());		// Store len of Database Name string

		// Connect to sql server with connection string
		dc.Initialize();

		if(dc.IsConnected())
		{
			DebugReporter::Instance().DisplayMessage("Connected to datatabase", DebugReporter::MAIL);

			// Made good connection construct sql statement object
			dca::SQLStatement sqlStmt(dc);
			
			// Bind the dbname to the statement for store procedure
			SQLINTEGER si = SQL_NTS;
			sqlStmt.BindStringParameter(sDBName.c_str(), bufLen);
			
			// Was able to store dbname so execute store procedure
			sqlStmt.ExecuteDirect(_T("{call sp_helpfile}"));

			// Execution of store procedure worked 
			dca::WString tDBSize;			// Holds current database size
			dca::WString tDBMaxSize;			// Holds maximum database size

			// Fetch first record set
			if(sqlStmt.Fetch())
			{
				/*if(sqlStmt.MoreResults())
				{
					if(sqlStmt.Fetch())
					{*/
						dca::WString255 sDBSize;			// buffer to hold dbsize
						dca::WString255 sDBMaxSize;		// buffer to hold maxsize
						
						ZeroMemory(sDBSize, 255 * sizeof(wchar_t));
						ZeroMemory(sDBMaxSize, 255 * sizeof(wchar_t));
						
						sqlStmt.GetStringData(sDBSize, 255, 5);
						tDBSize.assign(sDBSize);

						sqlStmt.GetStringData(sDBMaxSize, 255, 6);
						tDBMaxSize.assign(sDBMaxSize);
					/*}
				}*/
			}

			// Is database maxSize unlimited
			if(lstrcmpi(tDBMaxSize.c_str(),_T("UNLIMITED")))
			{
				dca::WString::size_type pos;
				do
				{
					pos = tDBSize.find(0x0020);
					if(pos != dca::WString::npos)
						tDBSize = tDBSize.erase(pos);
				}while(pos != dca::WString::npos);

				// Get difference between max size and dbsize
				UINT nSize = (tDBMaxSize.ToInt() - tDBSize.ToInt()) / 1024;

				f.Format("DB MinimumSpace %d", nSize);
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

				if(nSize < maxSpaceMB)
				{
					return true;
				}
			}
			else
			{
				DebugReporter::Instance().DisplayMessage("Database size is unlimited", DebugReporter::MAIL);
			}

			// Make sure disk space is good also.
			// See above comments for sPath.  Use to send null string this was causing problems
			// When VMF was installed on another drive but VMF was checking the size of C
			__int64 nFreeSpace = (dca::System::GetTotalNumberOfFreeBytes(sPath.c_str()) / 1024) / 1024;

			f.Format("FreeSpace %d", nFreeSpace);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

			if(nFreeSpace < __int64(maxSpaceMB))
				return true;
		}
	}
	catch(dca::Exception e)
	{
		dca::String f(e.GetMessage());
		f.insert(0, "CPopPollThread::IsDatabaseOrDiskFull() - ");
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

		CreateLogEntry(	EMSERROR(EMS_LOG_SEVERITY_ERROR,
								 EMSERR_MAIL_COMPONENTS,
								 EMS_LOG_INBOUND_MESSAGING,
								 ERROR_DATABASE),
								 e.GetMessage());
	}
	catch(...)
	{
		DebugReporter::Instance().DisplayMessage("CPopPollThread::IsDatabaseOrDiskFull() - An unknown or undefined exception has occurred", DebugReporter::MAIL);
	}

	DebugReporter::Instance().DisplayMessage("* Leaving CPopPollThread::IsDatabaseOrDiskFull function with false result", DebugReporter::MAIL);
	return false;
}
