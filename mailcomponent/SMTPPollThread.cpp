// SMTPPollThread.cpp: implementation of the CSMTPPollThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4244)

#include "stdafx.h"
#include "SMTPPollThread.h"
#include "MailComponents.h"
#include "ODBCConn.h"
#include "ODBCQuery.h"
#include "MessageIO.h"
#include "SingleCrit.h"
#include <map>
#include "SMTPSession.h"
#include "ServerParameters.h"
#include "EmailDate.h"
#include "QueryClasses.h"
#include "DateFns.h"
#include "RegistryFns.h"

dca::Mutex g_sessionLockSMTP_1;

extern dca::Mutex g_csDB;
extern CODBCConn g_odbcConn;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSMTPPollThread::CSMTPPollThread()
{
	m_hWaitHandles[wait_handle_stop] = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWaitHandles[send_right_now] = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_nFailureMark = 0;
	m_nSuccessMark = 0;
	
	m_nSendIntervalSecs = 15;

	m_bDefMsgDestMissing = false;
	m_bLoggedNoAccountsWarning = false;
}

CSMTPPollThread::~CSMTPPollThread()
{
	if (m_hWaitHandles[wait_handle_stop])
		CloseHandle(m_hWaitHandles[wait_handle_stop]);
	
	if (m_hWaitHandles[send_right_now])
		CloseHandle(m_hWaitHandles[send_right_now]);

	if (!DrainSessionPool())
		assert(0);
}

const UINT CSMTPPollThread::Run()
{
	DWORD dwRet;
	bool bRun = true;
	int nRet = 0;
	TIMESTAMP_STRUCT Now;
	
	// set LastMailSend registry key	
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
	WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMailSend"), t1 );	
	
	DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - Fill the SMTP session pool", DebugReporter::MAIL);
	
	// fill the SMTP session pool
	if (!FillSessionPool())
	{
		DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - Failed to fill the SMTP session pool", DebugReporter::MAIL);
		assert(0);
	}
	
	DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - Use waitformultiple objects here to run ResolveMsgDests at 15 second intervals", DebugReporter::MAIL);
	
	while (bRun)
	{
		try
		{
			DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - ResetEvent()", DebugReporter::MAIL);
					
			ResetEvent(m_hWaitHandles[send_right_now]);
			m_bSendNow = FALSE;
			
			DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - WaitForMultipleObjects()", DebugReporter::MAIL);
					
			dwRet = WaitForMultipleObjects(2, m_hWaitHandles, FALSE, m_nSendIntervalSecs * 1000);
			
			switch (dwRet)
			{
				// stop processing
				case (WAIT_OBJECT_0 + wait_handle_stop):
					DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - wait_handle_stop", DebugReporter::MAIL);
					bRun = false;
					break;
					
					// new processing interval
				case (WAIT_OBJECT_0 + send_right_now):
				case WAIT_TIMEOUT:
					
					DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - Send now or wait timeout", DebugReporter::MAIL);
					if(!_DBMaintenanceMutex.IsLocked())
					{
						// if db connection isn't open
						if (!g_odbcConn.IsConnected())
						{
							DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - OpenDBConn()", DebugReporter::MAIL);
					
							// try to open it now; if it won't open,
							// wait until the next processing interval
							UINT nRet = OpenDBConn();
							if (nRet != 0)
							{
								// logpoint: Unable to establish connection to database
								CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
															EMSERR_MAIL_COMPONENTS,
															EMS_LOG_OUTBOUND_MESSAGING,
															nRet),
												EMS_STRING_MAILCOMP_DB_CONN_FAILURE);

								break;
							}
						}

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
						WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMailSend"), t1 );

						DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - Ready to resolve message destinations", DebugReporter::MAIL);
						// resolve server destinations for messages that don't already know
						// where they're going
						nRet = ResolveMsgDests();
						if (nRet != 0)
						{
							if (nRet != ERROR_DEFAULT_MSG_DEST_MISSING)
							{
								// probably the result of a database failure, so log and bail
								// logpoint: Failed to resolve destinations for queued outbound messages
								CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
															EMSERR_MAIL_COMPONENTS,
															EMS_LOG_OUTBOUND_MESSAGING,
															nRet),
												EMS_STRING_MAILCOMP_MSG_DESTINATION_RESOLUTION_FAILURE);
							}

							break;
						}

						DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - Ready to spawn connections", DebugReporter::MAIL);
						
						// otherwise it's time to make the donuts
						nRet = SpawnConnections();
						if (nRet != ERROR_NO_ERROR)
						{
							if (nRet == ERROR_NO_MSG_DESTS_DEFINED)
							{
								if( !m_bLoggedNoAccountsWarning )
								{
									// logpoint: There are currently no outbound message destinations defined
									CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
															EMSERR_MAIL_COMPONENTS,
															EMS_LOG_OUTBOUND_MESSAGING,
															ERROR_NO_MSG_DESTS_DEFINED),
												EMS_STRING_MAILCOMP_NO_MSG_DESTS_DEFINED);

									CreateAlert( EMS_ALERT_EVENT_OUTBOUND_DELIVERY, (EMS_STRING_MAILCOMP_NO_MSG_DESTS_DEFINED));

									m_bLoggedNoAccountsWarning = true;
								}
							}
							else
							{
								// logpoint: Failed to initialize SMTP connections
								CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
															EMSERR_MAIL_COMPONENTS,
															EMS_LOG_OUTBOUND_MESSAGING,
															nRet),
												EMS_STRING_MAILCOMP_SMTP_INIT_SMTP_CONNS_FAILURE);
							}

							break;
						}
						else
						{
							m_bLoggedNoAccountsWarning = false;
						}
					}
					else
					{
						DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - DBMaintenance Running", DebugReporter::MAIL);
					}
					DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - Send now or wait timeout leaving", DebugReporter::MAIL);
					break;
				
				// anything else is an error
				default:
					DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - Unexpected error", DebugReporter::MAIL);
					LINETRACE(_T("Unexpected error in CSMTPPollThread::Run()\n"));
					assert(0);
					break;
			}
		}
		catch(dca::Exception e)
		{
			dca::String f(e.GetMessage());
			f.insert(0, "CSMTPPollThread::Run() - ");
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);			
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run() - An unknown or undefined exception has occurred", DebugReporter::MAIL);
		}
	}
	
	LINETRACE(_T("SMTP polling thread has terminated normally\n"));
	DebugReporter::Instance().DisplayMessage("CSMTPPollThread::Run - SMTP polling thread has terminated normally", DebugReporter::MAIL);

	return 0; // success
}

const int CSMTPPollThread::Stop()
{
	dca::Lock lock(g_sessionLockSMTP_1);

	LINETRACE(_T("Stopping SMTP session pool sessions...\n"));
	for (	SESSION_LIST::iterator cur(m_sessions.begin());
			cur != m_sessions.end();
			++cur)
	{
		((CSMTPSession*)*cur)->StopSession();
	}

	// stop this thread
	SetEvent(m_hWaitHandles[wait_handle_stop]);
	
	// stop this thread
	if (!SetEvent(m_hWaitHandles[wait_handle_stop]))
		assert(0);

	return 0;
}

void CSMTPPollThread::SendNow()
{
	// this will trigger immediate delivery of all
	// waiting msgs
	if (!SetEvent(m_hWaitHandles[send_right_now]))
		assert(0);

	m_bSendNow = TRUE;
}

const int CSMTPPollThread::SpawnConnections()
{	
	// lock access to the database object
	dca::Lock lock(g_csDB);

	bool bAccountsFound = false;
	SYSTEMTIME curTime;
	ZeroMemory(&curTime, sizeof(SYSTEMTIME));
	GetLocalTime(&curTime);
	bool bIsInOfficeHours = false;
	bool bOfficeHoursSet = false;
	int nDayOfWeek = curTime.wDayOfWeek + 1;
		
	try
	{
		m_nServerID = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_SERVER_ID, 1);
		
		dca::String f;
		f.Format("CSMTPPollThread::SpawnConnections - ServerID: %d", m_nServerID );
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

		CODBCQuery query(g_odbcConn);
		query.Initialize();

		//MessageDestinations_t msgdest;
		//ZeroMemory(&msgdest, sizeof(MessageDestinations_t));

		MessageDestinations_t md;
		vector<MessageDestinations_t> m_md;
		vector<MessageDestinations_t>::iterator mdIter;

		BINDCOL_LONG(query, md.m_MessageDestinationID);
		BINDCOL_WCHAR(query, md.m_ServerAddress);
		BINDCOL_WCHAR(query, md.m_Description);
		BINDCOL_LONG(query, md.m_UseSMTPAuth);
		BINDCOL_WCHAR(query, md.m_AuthUser);
		BINDCOL_WCHAR(query, md.m_AuthPass);
		BINDCOL_LONG(query, md.m_SMTPPort);
		BINDCOL_LONG(query, md.m_ConnTimeoutSecs);
		BINDCOL_LONG(query, md.m_MaxOutboundMsgSize);
		BINDCOL_LONG(query, md.m_MaxSendRetryHours);
		BINDCOL_TIME(query, md.m_LastProcessInterval);
		BINDCOL_LONG(query, md.m_ProcessFreqMins);
		BINDCOL_LONG(query, md.m_IsSSL);
		BINDCOL_LONG(query, md.m_SSLMode);
		BINDCOL_LONG(query, md.m_OfficeHours);
		BINDCOL_LONG(query, md.m_ErrorCode);
		BINDCOL_TIME(query, md.m_AccessTokenExpire);
		BINDCOL_LONG(query, md.m_OAuthHostID);
		BINDPARAM_LONG(query, m_nServerID );
		BINDPARAM_LONG(query, m_nFailureMark);

		query.Execute(	_T("select MessageDestinationID, ServerAddress, Description, UseSMTPAuth, ")
						_T("AuthUser, AuthPass, SMTPPort, ConnTimeoutSecs, MaxOutboundMsgSize, ")
						_T("MaxSendRetryHours, LastProcessInterval, ProcessFreqMins, IsSSL, SSLMode, OfficeHours, ErrorCode, AccessTokenExpire, OAuthHostID, AccessToken, RefreshToken ")
						_T("FROM MessageDestinations ")
						_T("WHERE IsActive = 1 and ServerID=? and ServerID IN (SELECT ServerID FROM ServerTasks st WHERE st.ServerTaskTypeID=6 AND st.ServerID=ServerID) ")
						_T("and MessageDestinationID >= ?"));

		m_md.clear();

		while( query.Fetch() == S_OK )
		{
			GETDATA_TEXT( query, md.m_AccessToken );
			GETDATA_TEXT( query, md.m_RefreshToken );
			m_md.push_back( md );			
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
						_T("WHERE (? BETWEEN TimeStart AND TimeEnd AND TypeID IN (1,2)) OR (TypeID=0 AND ActualID=0 AND DayID=?) OR (TypeID=6 AND ActualID<>0 AND DayID=?)")
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
					
		// iterate over the sql fetch
		for( mdIter = m_md.begin(); mdIter != m_md.end(); mdIter++ )
		{
			bAccountsFound = true;

			// if any session from the worker pool is actively processing this message
			// destination, then we want to skip over it
			bool b = false;
			for (SESSION_LIST::iterator cur = m_sessions.begin(); cur != m_sessions.end(); ++cur)
			{
				CSMTPSession* pSession = (CSMTPSession*)*cur;
				if (pSession->m_pSMTPDest && 
					(pSession->m_pSMTPDest->GetMessageDestID() == mdIter->m_MessageDestinationID) && pSession->m_bIsBusy)
				{
					dca::String f;
					f.Format("CSMTPPollThread::SpawnConnections() - Msg destination %d not processed, already being processed in another session with threadID [%d].", pSession->m_pSMTPDest->GetMessageDestID(),pSession->m_nThreadID );
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					b = true;
					break;
				}
			}
			if (b) continue;
			
			// create new SMTP destination
			CSMTPDest* pDest = new CSMTPDest();
			assert(pDest);
			if (pDest)
			{
				pDest->SetMessageDestID(mdIter->m_MessageDestinationID);
				pDest->SetAuthUser(mdIter->m_AuthUser);
				pDest->SetServerAddress(mdIter->m_ServerAddress);
				pDest->SetDescription(mdIter->m_Description);

				// MER 12/5/02 - now expecting password string to be encrypted
			
				try
				{
					CEMSString sDecrypt;
					sDecrypt.assign(mdIter->m_AuthPass);
					sDecrypt.Decrypt();
					pDest->SetAuthPass(sDecrypt.c_str());
				}
				catch (CEMSException EMSException)
				{
					// An error occured while attempting to decrypt message destination password (server: %s, username: %s).
					CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
												EMSERR_MAIL_COMPONENTS,
												EMS_LOG_INBOUND_MESSAGING,
												ERROR_DECRYPTING_DATA),
									EMS_STRING_MAILCOMP_ERROR_DECRYPTING_SMTP_PASSWORD,
									mdIter->m_ServerAddress,
									mdIter->m_AuthUser);

					// can't see the decryption process failing under normal circumstances,
					// should be safe just to log this event and skip the account in this case
					continue;
				}
				
				
				pDest->SetConnTimeoutSecs(mdIter->m_ConnTimeoutSecs);
				pDest->SetProcessFreqMins(mdIter->m_ProcessFreqMins);
				pDest->SetSMTPPort(mdIter->m_SMTPPort);
				pDest->SetUsesSMTPAuth(mdIter->m_UseSMTPAuth);
				pDest->SetIsSSL(mdIter->m_IsSSL);
				pDest->SetMaxOutboundMsgSize(mdIter->m_MaxOutboundMsgSize);
				pDest->SetSslMode(mdIter->m_SSLMode);
				pDest->SetErrorCode(mdIter->m_ErrorCode);
				if(mdIter->m_OAuthHostID > 0)
				{
					pDest->SetAccessToken(mdIter->m_AccessToken);
					pDest->SetRefreshToken(mdIter->m_RefreshToken);				
					pDest->SetOAuthHostID(mdIter->m_OAuthHostID);
					SYSTEMTIME ateTime;
					if ((mdIter->m_AccessTokenExpireLen != -1) &&
						(CEmailDate::SQLTimeToSystemTime(mdIter->m_AccessTokenExpire, ateTime)))
						pDest->SetATokenExpire(ateTime);
					else
					{
						DebugReporter::Instance().DisplayMessage("CSMTPPollThread::CheckPopAccounts - the value for access token expire in the database is invalid (probably NULL), the account needs to be reauthorized.", DebugReporter::MAIL);
					}
				}

				SYSTEMTIME syTime;
				if ((mdIter->m_LastProcessIntervalLen != -1) && 
					(CEmailDate::SQLTimeToSystemTime(mdIter->m_LastProcessInterval, syTime)))
					pDest->SetLastProcessed(syTime);
				else
				{
					// the value in the database is invalid (probably NULL), so let's
					// force an immediate send, which will update the lastprocessed date/time
					GetLocalTime(&syTime);
					pDest->SetLastProcessed(syTime);
					m_bSendNow = TRUE;
				}
				
				// check the "LastProcessInterval" setting to see if we actually need to check this account!
				// note: if we were triggered by "SendNow()", then the answer is always 'yes'
				UINT nDiffSecs = 0;
				const UINT nFreq = pDest->GetProcessFreqMins();
				const SYSTEMTIME lastProc = pDest->GetLastProcessed();
				if (!CEmailDate::GetDiffSecs(lastProc, curTime, nDiffSecs))
				{
					LINETRACE(_T("GetDiffSecs failed\n"));
					assert(0);
				}

				// if the number of minutes elapsed is less than the processing interval,
				// then move on to the next one
				// note: if SendNow() triggered this, we always want to send!
				UINT nMinsEl = nDiffSecs / 60;
				LINETRACE(_T("%d mins have elapsed since msg destination %d was last triggered (check freq: %d mins)\n"), nMinsEl, pDest->GetMessageDestID(), nFreq);
				f.Format("CSMTPPollThread::SpawnConnections - %d mins have elapsed since msg destination %d was last triggered (check freq: %d mins)", nMinsEl, pDest->GetMessageDestID(), nFreq );
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				if ((nMinsEl < nFreq) && !m_bSendNow)
				{
					delete pDest;
					continue; // next msg destination
				}

				// Does this MessageDestination use Office Hours?
				if ( mdIter->m_OfficeHours > 0 && !m_bSendNow)
				{
					if(!bOfficeHoursSet)
					{
						for( tooIter = too.begin(); tooIter != too.end(); tooIter++ )
						{
							if(tooIter->m_ActualID == mdIter->m_MessageDestinationID && !bOfficeHoursSet)
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
					
					if( !bIsInOfficeHours && mdIter->m_OfficeHours == 1)
					{
						DebugReporter::Instance().DisplayMessage("CSMTPPollThread::SpawnConnections - Message Destination not processed outside Office Hours!", DebugReporter::MAIL);
						delete pDest;
						continue; 
					}
					else if( bIsInOfficeHours && mdIter->m_OfficeHours == 2)
					{
						DebugReporter::Instance().DisplayMessage("CSMTPPollThread::SpawnConnections - Message Destination not processed during Office Hours!", DebugReporter::MAIL);
						delete pDest;
						continue; 
					}
				}			

				f.Format("CSMTPPollThread::SpawnConnections - Assigning MessageDestinationID: %d to work session", pDest->GetMessageDestID() );
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				
				// assign this account to a session
				bool bAssigned = AssignWorkToSession(pDest);
				if (!bAssigned)
				{
					LINETRACE(_T("Failed to find available session for MessageDestinationID %d\n"), pDest->GetMessageDestID());
					f.Format("CSMTPPollThread::SpawnConnections - Failed to find available session for MessageDestinationID %d", pDest->GetMessageDestID() );
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					delete pDest;
					break;
				}	
			}
		}
		for (SESSION_LIST::iterator cur = m_sessions.begin(); cur != m_sessions.end(); ++cur)
		{
			CSMTPSession* pSession = (CSMTPSession*)*cur;
			if (pSession->m_pSMTPDest)
			{
				int nBusy = 0;
				if(pSession->m_bIsBusy){nBusy = 1;}
				dca::String f;
				f.Format("CSMTPPollThread::SpawnConnections() - Session with MessageDestinationID [%d] threadID [%d] with isBusy [%d].", pSession->m_pSMTPDest->GetMessageDestID(),pSession->m_nThreadID, nBusy );
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);				
			}
			else
			{
				int nBusy = 0;
				if(pSession->m_bIsBusy){nBusy = 1;}
				dca::String f;
				f.Format("CSMTPPollThread::SpawnConnections() - Session without m_pSMTPDest threadID [%d] with isBusy [%d].", pSession->m_nThreadID, nBusy );
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			}
		}
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);

		// logpoint: Failed to retrieve outbound message destinations
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_OUTBOUND_MESSAGING,
									ERROR_DATABASE),
						EMS_STRING_MAILCOMP_MSG_DEST_RETRIEVAL_FAILURE);

		return ERROR_DATABASE;
	}

	if (m_nFailureMark <= m_nSuccessMark)
		m_nFailureMark = m_nSuccessMark = 0;

	// if no accounts found
	if (!bAccountsFound)
	{
		if ( CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_MSG_SENDER, 0) == 1 )
		{
			return ERROR_NO_MSG_DESTS_DEFINED;
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CSMTPPollThread::SpawnConnections - SMTP sending disabled for this server", DebugReporter::MAIL);
		}
	}

	return 0;
}


const int CSMTPPollThread::ResolveMsgDests()
{
	std::map<LONG, LONG> destmap;

	// get the default message destination ID
	UINT lDefDest = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_DEFAULT_MSGDEST_ID, 0);

	dca::String f;
	f.Format("CSMTPPollThread::ResolveMsgDests - Default MessageDestination ID: %d", lDefDest );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					
	// if a default message destination is not defined, we'll generate an admin alert (and log
	// entry and halt outbound message processing until it is
	if (lDefDest == 0)
	{
		if (!m_bDefMsgDestMissing)
		{
			m_bDefMsgDestMissing = true;

			// logpoint: Outbound delivery halted because no default message destination defined (will resume when corrected)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_OUTBOUND_MESSAGING,
										ERROR_DEFAULT_MSG_DEST_MISSING),
							IDS_STRING_MAILCOMP_NOMSGDESTDEF_ALERT);
			
			// administrative alert: Outbound delivery halted because no default message destination defined (will resume when corrected)
			CreateAlert(EMS_ALERT_EVENT_OUTBOUND_DELIVERY,
						IDS_STRING_MAILCOMP_NOMSGDESTDEF_ALERT);
		}

		return ERROR_DEFAULT_MSG_DEST_MISSING;
	}
	else
	{
		if (m_bDefMsgDestMissing)
			m_bDefMsgDestMissing = false;
	}

	// lock access to the database object
	dca::Lock lock(g_csDB);

	// init query object
	try
	{
		CODBCQuery query(g_odbcConn);

		std::vector<SMTPMessageDestination>           destDescriptionList;
		std::vector<SMTPMessageDestination>::iterator idest;

		// Get Message desinations
		// Updated by Mark Mohr on 11/22/2006
		{
			TMessageDestinations messageDest;
			messageDest.PrepareList(query);

			while(query.Fetch() != S_FALSE)
			{
				if ( messageDest.m_IsActive )
				{
					SMTPMessageDestination temp1(messageDest.m_MessageDestinationID
					                        ,messageDest.m_Description);
				
					destDescriptionList.push_back(temp1);
				}
			}
		}

		query.Initialize();

		LONG lOutboundMsgID = 0;
		LONG lOutboundMsgIDLen = 0;
		LONG lOutboundMsgType = 0;
		LONG lOutboundMsgTypeLen = 0;
		LONG lCurDestID = 0;
		LONG lCurDestIDLen = 0;
		LONG lNewDestID = 0;
		LONG lNewDestIDLen = 0;
		LONG lTBDestID = 0;
		LONG lTBDestIDLen = 0;
		LONG lReplyToIDIsInbound = 0;
		LONG lReplyToIDIsInboundLen = 0;
		TCHAR EmailFromTxt[255];
		LONG EmailFromTxtLen = 255;
		CEmailDate curDate;
		

		BINDCOL_LONG(query, lOutboundMsgID);		// outbound message id
		BINDCOL_LONG(query, lOutboundMsgType);		// outbound msg type (new, forward, or reply)
		BINDCOL_LONG(query, lCurDestID);			// msg destination in OutboundMessageQueue
		BINDCOL_LONG(query, lNewDestID);			// msg destination in MessageSources
		BINDCOL_LONG(query, lTBDestID);				// msg destination in TicketBoxes
		BINDCOL_LONG(query, lReplyToIDIsInbound);	// are we replying to a message id that's inbound?
		BINDCOL_TCHAR(query, EmailFromTxt);
		BINDPARAM_TIME_NOLEN(query, curDate.GetSQLTime());

		// first we want to know about all the outbound messages that require
		// message destination resolution
		query.Execute(	_T("select 	OutboundMessages.OutboundMessageID, ")
						_T("		OutboundMessages.OutboundMessageTypeID, ")
						_T("		OutboundMessageQueue.MessageDestinationID, ")
						_T("		MessageSources.MessageDestinationID, ")
						_T("		TicketBoxes.MessageDestinationID, ")
						_T("		OutboundMessages.ReplyToIDIsInbound, ")
						_T("        OutboundMessages.EmailFrom ")
						_T("from	OutboundMessageQueue, InboundMessages ")
						_T("left outer join MessageSources ON MessageSources.MessageSourceID = InboundMessages.MessagesourceID ")
						_T("right outer join OutboundMessages ON InboundMessages.InboundMessageID = OutboundMessages.ReplyToMsgID ")
						_T("left outer join Agents ON OutboundMessages.AgentID = Agents.AgentID ")
						_T("left outer join Tickets ON OutboundMessages.TicketID = Tickets.TicketID ")
						_T("right outer join TicketBoxes on TicketBoxes.TicketBoxID = Tickets.TicketBoxID ")
						_T("where	OutboundMessageQueue.MessageDestinationID = 0 ")
						_T("		and OutboundMessageQueue.OutboundMessageID = OutboundMessages.OutboundMessageID ")
						_T("		and OutboundMessages.IsDeleted = 0 ")
						_T("		and OutboundMessages.ArchiveID = 0 ")
						_T("		and OutboundMessageQueue.IsApproved = 1")
						_T("		and (Agents.OutboxHoldTime IS NULL OR DATEADD(minute, Agents.OutboxHoldTime, OutboundMessages.EmailDateTime ) < ?) "));
						
		// iterate over the sql fetch
		while (query.Fetch() == S_OK)
		{
			// by default, assume that we'll want to use the default msg destination
			LONG lDestID = lDefDest;

			wstring fromTest(EmailFromTxt);
			wstring::size_type pos = fromTest.find_last_of('@');
			if(pos != wstring::npos){
				
				fromTest = fromTest.substr(pos);

				pos = fromTest.find_last_of('>');

				if(pos != wstring::npos){
					fromTest.erase(pos);
				}
			}

			bool found = false;

			std::vector<std::wstring>::iterator istring;
			for(idest = destDescriptionList.begin();
				idest != destDescriptionList.end();
				++idest){

					for(istring = (*idest).GetDescriptionList().begin();
						istring != (*idest).GetDescriptionList().end();
						++istring){

							if( !(*istring).compare(fromTest)){
								lDestID = (*idest).GetId();
								found = true;
								break;
						}
					}

					if(found)
						break;
			}

			// if the outbound message resides in a Ticket that is in a TicketBox
			// that has a MessageDestination set, use it
			if(lDestID == lDefDest && (!found) ){
				// if the TicketBox source destination ID isn't zero, then we'll use it
				if ((lTBDestIDLen != SQL_NULL_DATA) && 
					(lTBDestID != 0))
				{
					// assign the new destination
					lDestID = lTBDestID;
					found = true;
				}
			}				
				
			// if this is a reply to an inbound message, then we want to use
			// the inbound message source's corresponding message destination
			if(lDestID == lDefDest && (!found) ){
				if (lOutboundMsgType == EMS_OUTBOUND_MESSAGE_TYPE_REPLY)
				{
					// if the inbound message source destination ID isn't zero, then we'll use it
					if ((lNewDestIDLen != SQL_NULL_DATA) && 
						(lNewDestID != 0) && 
						(lReplyToIDIsInbound == 1))
					{
						// assign the new destination
						lDestID = lNewDestID;
						found = true;
					}
				}
			}

			f.Format("CSMTPPollThread::ResolveMsgDests - Using MessageDestination ID: %d for OutboundMessageID: %d", lDefDest, lOutboundMsgID );
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			// default dest ID should never be zero here, but we'll handle the case
			// just to be safe
			assert(lDestID != 0);
			if (lDestID == 0)
			{
				// logpoint: Failed to resolve destination for queued outbound message no. %d (does a default message destination exist?)
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
											EMSERR_MAIL_COMPONENTS,
											EMS_LOG_OUTBOUND_MESSAGING,
											ERROR_MSG_DEST_UNRESOLVED),
								EMS_STRING_MAILCOMP_MSG_DEST_UNRESOLVED,
								lOutboundMsgID);
			}
			else
			{
				// map the new message destination id to the outbound message
				pair<LONG, LONG> pair(lOutboundMsgID, lDestID);
				destmap.insert(pair);
			}
		}

		query.Reset(true);

		BINDCOL_LONG(query, lOutboundMsgID);		// outbound message id
		BINDCOL_LONG(query, lOutboundMsgType);		// outbound msg type (new, forward, or reply)
		BINDCOL_LONG(query, lCurDestID);			// msg destination in OutboundMessageQueue
		BINDCOL_LONG(query, lNewDestID);			// msg destination in MessageSources
		BINDCOL_LONG(query, lReplyToIDIsInbound);	// are we replying to a message id that's inbound?
		BINDCOL_TCHAR(query, EmailFromTxt);
		BINDPARAM_TIME_NOLEN(query, curDate.GetSQLTime());

		// first we want to know about all the outbound messages that require
		// message destination resolution
		query.Execute(	_T("select 	OutboundMessages.OutboundMessageID, ")
						_T("		OutboundMessages.OutboundMessageTypeID, ")
						_T("		OutboundMessageQueue.MessageDestinationID, ")
						_T("		MessageSources.MessageDestinationID, ")
						_T("		OutboundMessages.ReplyToIDIsInbound, ")
						_T("        OutboundMessages.EmailFrom ")
						_T("from	OutboundMessageQueue, InboundMessages ")
						_T("left outer join MessageSources ON MessageSources.MessageSourceID = InboundMessages.MessagesourceID ")
						_T("right outer join OutboundMessages ON InboundMessages.InboundMessageID = OutboundMessages.ReplyToMsgID ")
						_T("left outer join Agents ON OutboundMessages.AgentID = Agents.AgentID ")
						_T("where	OutboundMessageQueue.MessageDestinationID = 0 ")
						_T("		and OutboundMessageQueue.OutboundMessageID = OutboundMessages.OutboundMessageID ")
						_T("		and OutboundMessages.IsDeleted = 0 ")
						_T("		and OutboundMessages.TicketID = 0 ")
						_T("		and OutboundMessages.ArchiveID = 0 ")
						_T("		and OutboundMessageQueue.IsApproved = 1")
						_T("		and (Agents.OutboxHoldTime IS NULL OR DATEADD(minute, Agents.OutboxHoldTime, OutboundMessages.EmailDateTime ) < ?) "));

		// iterate over the sql fetch
		while (query.Fetch() == S_OK)
		{
			// by default, assume that we'll want to use the default msg destination
			LONG lDestID = lDefDest;

			wstring fromTest(EmailFromTxt);
			wstring::size_type pos = fromTest.find_last_of('@');
			if(pos != wstring::npos){
				
				fromTest = fromTest.substr(pos);

				pos = fromTest.find_last_of('>');

				if(pos != wstring::npos){
					fromTest.erase(pos);
				}
			}

			bool found = false;

			std::vector<std::wstring>::iterator istring;
			for(idest = destDescriptionList.begin();
				idest != destDescriptionList.end();
				++idest){

					for(istring = (*idest).GetDescriptionList().begin();
						istring != (*idest).GetDescriptionList().end();
						++istring){

							if( !(*istring).compare(fromTest)){
								lDestID = (*idest).GetId();
								found = true;
								break;
						}
					}

					if(found)
						break;
			}

			// by default, assume that we'll want to use the default msg destination
			//LONG lDestID = lDefDest;

			// if this is a reply to an inbound message, then we want to use
			// the inbound message source's corresponding message destination
			if(lDestID == lDefDest && (!found) ){
				if (lOutboundMsgType == EMS_OUTBOUND_MESSAGE_TYPE_REPLY)
				{
					// if the inbound message source destination ID isn't zero, then we'll use it
					if ((lNewDestIDLen != SQL_NULL_DATA) && 
						(lNewDestID != 0) && 
						(lReplyToIDIsInbound == 1))
					{
						// assign the new destination
						lDestID = lNewDestID;
					}
				}
			}

			f.Format("CSMTPPollThread::ResolveMsgDests - Using MessageDestination ID: %d for OutboundMessageID: %d", lDefDest, lOutboundMsgID );
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			// default dest ID should never be zero here, but we'll handle the case
			// just to be safe
			assert(lDestID != 0);
			if (lDestID == 0)
			{
				// logpoint: Failed to resolve destination for queued outbound message no. %d (does a default message destination exist?)
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
											EMSERR_MAIL_COMPONENTS,
											EMS_LOG_OUTBOUND_MESSAGING,
											ERROR_MSG_DEST_UNRESOLVED),
								EMS_STRING_MAILCOMP_MSG_DEST_UNRESOLVED,
								lOutboundMsgID);
			}
			else
			{
				// map the new message destination id to the outbound message
				pair<LONG, LONG> pair(lOutboundMsgID, lDestID);
				destmap.insert(pair);
			}
		}

		query.Reset(true);


		// iterate over our LONG map and update the database
		std::map<LONG, LONG>::iterator iter = destmap.begin();
		std::map<LONG, LONG>::iterator iter_end = destmap.end();
		for (; iter != iter_end; ++iter)
		{
			LONG lMsgID = iter->first;
			LONG lDestID = iter->second;			

			BINDPARAM_LONG(query, lDestID);
			BINDPARAM_LONG(query, lMsgID);			

			query.Execute(	_T("UPDATE OutboundMessageQueue ")
							_T("SET MessageDestinationID = ? ")
							_T("WHERE OutboundMessageID = ? "));

			query.Reset(true);
		}
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}

bool CSMTPPollThread::FillSessionPool()
{
	// pool size is user-definable; the pool size effectively limits
	// how many threads of this type can run concurrently
	dca::Lock lock(g_sessionLockSMTP_1);

	LINETRACE(_T("Filling SMTP session pool...\n"));
	int nPoolSize = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_MAX_CON_SMTP_THREADS, 5);
	for (int i=0; i<nPoolSize; i++)
	{
		CSMTPSession* pSession = new CSMTPSession();
		assert(pSession);
		if (pSession)
			m_sessions.push_back(pSession);
	}

	LINETRACE(_T("SMTP session pool has been filled\n"));

	return true;
}

bool CSMTPPollThread::DrainSessionPool()
{
	for(	SESSION_LIST::iterator cur(m_sessions.begin());
			cur != m_sessions.end();
			++cur)
	{
		delete *cur;
	}

	m_sessions.clear();

	LINETRACE(_T("SMTP session pool has been drained\n"));

	return true;
}

bool CSMTPPollThread::AssignWorkToSession(CSMTPDest *const pDest)
{
	for	(	SESSION_LIST::iterator cur(m_sessions.begin());
			cur != m_sessions.end();
			++cur)
	{
		CSMTPSession* pSession = (CSMTPSession*)*cur;

		if (!pSession->m_bIsBusy)
		{
			if (pSession->StartSession(pDest) != 0)
				break;
			
			dca::String f;
			f.Format("CSMTPPollThread::AssignWorkToSession - MessageDestinationID: %d assigned to ThreadID: %d", pDest->GetMessageDestID(), pSession->m_nThreadID );
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						
			m_nSuccessMark = pDest->GetMessageDestID();
			return true;
		}
	}

	m_nFailureMark = pDest->GetMessageDestID();

	return false;
}
