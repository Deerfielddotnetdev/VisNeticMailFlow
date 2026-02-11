#include "stdafx.h"
#include "DBMonitorThread.h"
#include "VirusScanningThread.h"
#include "EMSEvent.h"
#include "RegistryFns.h"

DBMonitorThread::DBMonitorThread(void): m_query(m_db)
{
	DebugReporter::Instance().DisplayMessage("DBMonitorThread::DBMonitorThread - Constructor", DebugReporter::ENGINE);
	m_pMonitor = new CTicketMonitor( m_query );
	srand ( (unsigned) time(NULL) );
}

DBMonitorThread::~DBMonitorThread(void)
{
	DebugReporter::Instance().DisplayMessage("DBMonitorThread::DBMonitorThread - Destructor", DebugReporter::ENGINE);
	delete m_pMonitor;
}

void DBMonitorThread::Initialize()
{
	try
	{
		DebugReporter::Instance().DisplayMessage("DBMonitorThread::Initialize", DebugReporter::ENGINE);

		if( !m_db.IsConnected() )
		{
			m_db.Connect();
			m_bInitialized = TRUE;
		}

		if( !m_pMonitor )
		{
			dca::String er;			
			er.Format("DBMonitorThread::Initialize - Ticket Monitor not loaded, attempting reload");
			DebugReporter::Instance().DisplayMessage(er.c_str(), DebugReporter::ENGINE);
			m_pMonitor = new CTicketMonitor( m_query );
		}		
	}
	catch(dca::Exception& e)
	{
		try
		{
			dca::String er;
			dca::String x(e.GetMessage());
			er.Format("DBMonitorThread::Initialize - %s", x.c_str());
			DebugReporter::Instance().DisplayMessage(er.c_str(), DebugReporter::ENGINE);
			Log( E_DBMonitorError, L"%s\n", e.GetMessage());
			
			m_db.Disconnect();
		}
		catch(...)
		{}
	}
	catch(...)
	{
		try
		{
			DebugReporter::Instance().DisplayMessage("DCA - Database was not initialized", DebugReporter::ENGINE);
			Log( E_DBMonitorError, L"An Unkown or undefined exception has occurred in the DBMonitor Thread\n");

			m_db.Disconnect();
		}
		catch(...)
		{
		}
	}
}

void DBMonitorThread::Uninitialize()
{
	DebugReporter::Instance().DisplayMessage("DBMonitorThread::Uninitialize - Entered", DebugReporter::ENGINE);
	m_pMonitor->FinishCurrentJob();
	try
	{
		m_db.Disconnect();
		DebugReporter::Instance().DisplayMessage("DBMonitorThread::Uninitialize - Disconnected from DB", DebugReporter::ENGINE);
	}
	catch(...)
	{
	}
}

void DBMonitorThread::Flush()
{
	_event.Set();
}

unsigned int DBMonitorThread::Run()
{
	DWORD dwRet;
	BOOL bRun = TRUE;
	DWORD dwWaitTime;
	
	m_LastAgentTicketCategoryCheck = 0;
	m_LastDBCheckIn = 0;
	m_LastMailCheck = 0;
	m_LastTicketMonitorRun = 0;
	m_LastEmailVerifyRun = 0;
	m_EmailVerifyDays = 0;

	//Wait 5 seconds to allow globals to be loaded
	Sleep(5000);

	SetEvent( m_hReadyEvent );

	// This is the normal loop
	while ( bRun )
	{
		dwWaitTime = 10000;
		dwRet = MsgWaitForMultipleObjects( 1, &m_hStopEvent, FALSE, dwWaitTime, QS_ALLEVENTS );
		
		switch( dwRet )
		{
		case WAIT_TIMEOUT:
			{
				if( m_bInitialized )
				{
					if(!_DBMaintenanceMutex.IsLocked())
					{
						long TickCount = GetTickCount();
						long lDiff = TickCount - m_LastDBCheckIn;

						// DB CheckIn
						if( m_LastDBCheckIn == 0 || ((lDiff > DBCheckInMs) || (lDiff < 0)) )
						{
							if( m_db.IsConnected() )
							{
								DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Checking in to DB", DebugReporter::ENGINE);							
							
								if(CheckInToDB())
								{
									DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - DB CheckIn Failed", DebugReporter::ENGINE);						
								}
								else
								{
									DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - DB CheckIn Succeeded", DebugReporter::ENGINE);						
								}
								
								m_LastDBCheckIn = TickCount;	
							}
							else
							{
								m_db.Connect();
								continue;
							}										
						}	

						lDiff = TickCount - m_LastMailCheck;

						// Mail Check
						if( m_LastMailCheck == 0 || ((lDiff > MailCheckMs) || (lDiff < 0)) )
						{
							DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Running Mail Check", DebugReporter::ENGINE);
							
							if( m_db.IsConnected() )
							{								
								if( MailCheck() == S_OK )
								{
									DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Mail Check successful", DebugReporter::ENGINE);									
								}
								else
								{
									DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Mail Check failed", DebugReporter::ENGINE);						
								}
								m_LastMailCheck = TickCount;	
							}
							else
							{
								m_db.Connect();
							}										
						}

						lDiff = TickCount - m_LastTicketMonitorRun;

						// Ticket Monitor
						if( m_LastTicketMonitorRun == 0 || ((lDiff > TicketMonitorMs) || (lDiff < 0)) )
						{
							DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Running Ticket Monitor", DebugReporter::ENGINE);
							
							if( m_db.IsConnected() )
							{								
								if( m_pMonitor->Run() == S_OK )
								{
									DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Ticket Monitor Events processed", DebugReporter::ENGINE);									
								}
								else
								{
									DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - No Ticket Monitor Events processed", DebugReporter::ENGINE);						
								}
								m_LastTicketMonitorRun = TickCount;	
							}
							else
							{
								m_db.Connect();
							}										
						}

						// AgentTicketCategories
						UINT nUseATC=0;
						if (GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseATC"), nUseATC ) != ERROR_SUCCESS)
						{
							nUseATC = 0;
						}
						if( nUseATC == 1 )
						{
							lDiff = TickCount - m_LastAgentTicketCategoryCheck;
							
							if( m_LastAgentTicketCategoryCheck == 0 || ((lDiff > AgentTicketCategoryCheckMs) || (lDiff < 0)) )
							{
								
								DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Entering Agent TicketCategory check.", DebugReporter::ENGINE);

								try
								{
									if ( _ttoi(g_Object.GetParameter( EMS_SRVPARAM_MASTER_SERVER )) == _ttoi(g_Object.GetParameter( EMS_SRVPARAM_SERVER_ID )) )
									{
										DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Checking Agent TicketCategories", DebugReporter::ENGINE);
										
										if( m_db.IsConnected() )
										{
											if(this->TryAgentTicketCategory())
											{
												DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Agent TicketCategories Failed", DebugReporter::ENGINE);
											}
											
											m_LastAgentTicketCategoryCheck = TickCount;	
										}
										else
										{
											m_db.Connect();
										}
									}
									else
									{
										m_LastAgentTicketCategoryCheck = TickCount;	
									}
								}
								catch(dca::Exception e)
								{
									try
									{
										DcaTrace(_T("DCA - Error occurred in AgentTicketCategory"));
										DcaTrace(e.GetMessage());
										Log( E_DBMonitorError, L"%s\n", e.GetMessage());
										m_db.Disconnect();
									}
									catch(...)
									{
									}
								}
								catch(...)
								{
									try
									{
										DcaTrace(_T("DCA - Error occurred in AgentTicketCategory"));
										Log( E_DBMonitorError, L"An Unkown or undefined exception has occurred in the DBMonitorSystem Thread\n");
										m_db.Disconnect();
									}
									catch(...)
									{
									}
								}
							}
						}
						
						lDiff = TickCount - m_LastEmailVerifyRun;

						// Email Verify
						if(lDiff > CheckContactEmailsMs || lDiff < 0)
						{
							try
							{
								m_EmailVerifyDays = _ttoi(g_Object.GetParameter( EMS_SRVPARAM_VERIFY_CONTACT_EMAIL_FREQUENCY ));
								m_EnableVerification = _ttoi(g_Object.GetParameter( EMS_SRVPARAM_ENABLE_EMAIL_VERIFICATION ));

								if ( m_EmailVerifyDays  > 0 && m_EnableVerification == 1 )
								{
									DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Getting Emails to Verify", DebugReporter::ENGINE);
									
									if( m_db.IsConnected() )
									{
										if(this->GetEmailsToVerify())
										{
											DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Verify Emails Returned True", DebugReporter::ENGINE);
										}
										else
										{
											DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Verify Emails Returned False", DebugReporter::ENGINE);
										}
										if(this->ProcessEmails())
										{
											DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Process Emails Returned True", DebugReporter::ENGINE);
										}
										else
										{
											DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Process Emails Returned False", DebugReporter::ENGINE);
										}
										m_LastEmailVerifyRun = TickCount;	
									}
									else
									{
										m_db.Connect();
									}
								}
								else
								{
									m_LastEmailVerifyRun = TickCount;	
								}
							}
							catch(dca::Exception e)
							{
								try
								{
									DcaTrace(_T("DCA - Error occurred in Email Verify"));
									DcaTrace(e.GetMessage());
									Log( E_DBMonitorError, L"%s\n", e.GetMessage());
									m_db.Disconnect();
								}
								catch(...)
								{
								}
							}
							catch(...)
							{
								try
								{
									DcaTrace(_T("DCA - Error occurred in Email Verify"));
									Log( E_DBMonitorError, L"An Unkown or undefined exception has occurred in the DBMonitorSystem Thread\n");
									m_db.Disconnect();
								}
								catch(...)
								{
								}
							}									
						}
					}
				}
				else
				{
					Initialize();
				}		
			}
			break;
		
		case WAIT_OBJECT_0:			// Stop Event is set.
			{
				bRun = FALSE;
				DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Received stop event", DebugReporter::ENGINE);
			}
			break;
		}
	}
	
	Uninitialize();
	
	return 0;	
}

int DBMonitorThread::CheckInToDB()
{
	tstring sSpoolFolder,sTempFolder;
	TServers ts;
	UINT nServerID;

	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               EMS_SERVER_ID_VALUE, nServerID ) != ERROR_SUCCESS)
	{
		nServerID = 1;
	}

	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );

	try
	{
		ts.m_ServerID = nServerID;
		ts.Query( m_query );
	}
	catch(...)
	{
		try
		{
			DcaTrace(_T("DCA - Error occurred in DB Check In"));
			Log( E_DBMonitorError, L"An Unkown or undefined exception has occurred in DB Check In\n");
			m_db.Disconnect();
			return 1;
		}
		catch(...)
		{
			DcaTrace(_T("DCA - Error occurred in DB Check In"));
			m_db.Disconnect();
			return 1;
		}
	}
	
	
	if ( ts.m_ReloadConfig == 1 )
	{
		GetServerParameters();
		if ( ts.m_SpoolFolderLen == 0 )
		{
			sSpoolFolder.assign(m_sSpoolFolder.c_str());
		}
		else
		{
			sSpoolFolder.assign(ts.m_SpoolFolder);
		}
		if ( ts.m_TempFolderLen == 0 )
		{
			sTempFolder.assign(m_sTempFolder.c_str());
		}
		else
		{
			sTempFolder.assign(ts.m_TempFolder);
		}		
		
		// Update the registry
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_ARCHIVE_PATH_VALUE, m_sArchiveFolder.c_str() );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_BACKUP_PATH_VALUE, m_sBackupFolder.c_str() );
		//WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_LOG_PATH_VALUE, m_sLogFolder.c_str() );
		//WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_REPORT_PATH_VALUE, m_sReportFolder.c_str() );
		WriteRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("InboundAttachFolder"), m_sInboundAttachFolder.c_str() );
		WriteRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("OutboundAttachFolder"), m_sOutboundAttachFolder.c_str() );
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("NoteAttachFolder"), m_sNoteAttachFolder.c_str() );
		//WriteRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("SpoolFolder"), sSpoolFolder.c_str() );
		//WriteRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("TempFolder"), sTempFolder.c_str() );
		//WriteRegString( EMS_ROOT_KEY, REG_KEY_AV, _T("QuarantineFolder"), m_sQuarantineFolder.c_str() );	

		// Create a manual reset event
		HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

		// We duplicate a handle so that we don't have to worry about who
		// closes the event handle.
		HANDLE hEventDuplicate;

		DuplicateHandle( GetCurrentProcess(), hEvent,
						GetCurrentProcess(), &hEventDuplicate, 
						0, FALSE, DUPLICATE_SAME_ACCESS );

		g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::Worker, WM_RELOAD_CONFIG, 
													(WPARAM) hEventDuplicate, EMS_ServerParameters );

		// Reload Virus Scanner config
		//g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::VirusScanner, WM_RELOAD_CONFIG,0, 0 );

		// Reload Messaging Components
		g_Object.m_MessagingComponents.RefreshServerParams();

		// Reload Logging Components


		// tell the ISAPI to reload
		CReloadISAPIEvent reloadEvent;
		SetEvent( reloadEvent.GetHandle() );				

		WaitForSingleObject( hEvent, 5000 );

		CloseHandle( hEvent );

		if ( ProcessDBCheckIn() )
		{
			return 1;
		}
	}

	m_query.Initialize();
	BINDPARAM_TIME_NOLEN( m_query, Now );
	BINDPARAM_LONG( m_query, nServerID );
	m_query.Execute( L"UPDATE Servers SET Status=1,CheckIn=?,ReloadConfig=0 WHERE ServerID=?" );
	
	
	return 0;
}

int DBMonitorThread::ProcessDBCheckIn()
{
	try
	{
		// Create a manual reset event
		HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

		// We duplicate a handle so that we don't have to worry about who
		// closes the event handle.
		HANDLE hEventDuplicate;

		DuplicateHandle( GetCurrentProcess(), hEvent,
						GetCurrentProcess(), &hEventDuplicate, 
						0, FALSE, DUPLICATE_SAME_ACCESS );

		g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::Worker, WM_RELOAD_CONFIG, 
													(WPARAM) hEventDuplicate, EMS_ServerParameters );

		// Reload Virus Scanner config
		//g_Object.m_ThreadManager.PostThreadMessage( CThreadManager::VirusScanner, WM_RELOAD_CONFIG, 0, 0 );

		g_Object.m_MessagingComponents.RefreshServerParams();
		
		CReloadISAPIEvent reloadEvent;
		SetEvent( reloadEvent.GetHandle() );
		

		WaitForSingleObject( hEvent, 5000 );

		CloseHandle( hEvent );

		DebugReporter::Instance().DisplayMessage("DBMonitorThread::Run - Engine Config Reloaded", DebugReporter::ENGINE);
	}
	catch(...)
	{
		try
		{
			DcaTrace(_T("DCA - Error occurred in Process DB Check In"));
			Log( E_DBMonitorError, L"An Unkown or undefined exception has occurred in Process DB Check In\n");
			m_db.Disconnect();
			return 1;
		}
		catch(...)
		{
			DcaTrace(_T("DCA - Error occurred in Process DB Check In"));
			m_db.Disconnect();
			return 1;
		}
	}

	return 0;
}

int DBMonitorThread::MailCheck()
{
	TIMESTAMP_STRUCT Now;
	UINT nMailCheckRestartMin = 10;
	UINT nLastMailCheck = 0;
	UINT nLastMailSend = 0;
	UINT nMailCheckSec = 0;
	UINT nMailSendSec = 0;
	UINT nRestartCount = 0;
	bool doRestart = false;
	dca::String s;

	try
	{
		if(GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,  _T("MailCheckRestartMin"), nMailCheckRestartMin))
		{
			WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("MailCheckRestartMin"), 10 );	
		}

		if(GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,  _T("LastMailSend"), nLastMailSend))
		{
			DebugReporter::Instance().DisplayMessage("DBMonitorThread::MailCheck - LastMailSend registry key not found", DebugReporter::ENGINE);
			return 1;
		}

		if(GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,  _T("LastMailCheck"), nLastMailCheck))
		{
			DebugReporter::Instance().DisplayMessage("DBMonitorThread::MailCheck - LastMailCheck registry key not found", DebugReporter::ENGINE);
			return 1;
		}

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

		UINT nTimeNow = t1;

		// calculate number of seconds since last mail send
		nMailCheckSec = nTimeNow - nLastMailCheck;
		s.Format("DBMonitorThread::MailCheck - Last Mail Check [%d] seconds ago", nMailCheckSec);
		DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);

		// calculate number of seconds since last mail check
		nMailSendSec = nTimeNow - nLastMailSend;
		s.Format("DBMonitorThread::MailCheck - Last Mail Send [%d] seconds ago", nMailSendSec);
		DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);

		if ( nMailCheckSec > (nMailCheckRestartMin * 60))
		{
			s.Format("DBMonitorThread::MailCheck - Last Mail Check [%d] seconds ago exceeds Mail Check Restart [%d] seconds, initiating MailComponents restart", nMailCheckSec, (nMailCheckRestartMin * 60));
			DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);

			doRestart = true;
		}
		else if ( nMailSendSec > (nMailCheckRestartMin * 60))
		{
			s.Format("DBMonitorThread::MailCheck - Last Mail Send [%d] seconds ago exceeds Mail Check Restart [%d] seconds, initiating MailComponents restart", nMailSendSec, (nMailCheckRestartMin * 60));
			DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);

			doRestart = true;
		}

		if ( doRestart )
		{
			int retval=-1;
			
			retval = g_Object.m_MessagingComponents.Shutdown();

			if(retval != 0)
			{
				s.Format("DBMonitorThread::MailCheck - Failed to unload mailcomponents.dll - mail send and receive may be disabled, calling UnloadDLL().");
				DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);

				retval = g_Object.m_MessagingComponents.UnloadDLL();
			}
			
			if(retval == 0)
			{
				s.Format("DBMonitorThread::MailCheck - Successfully unloaded mailcomponents.dll due to request - mail send and receive is disabled.");
				DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);
					
				retval = g_Object.m_MessagingComponents.LoadDLL();							

				if( retval )
				{
					CEMSString d;
					d.LoadString( EMS_STRING_ERROR_LOADING_MAILCOMPONENTS );
					_Module.LogEvent( d.c_str(), retval );
					Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, L"Error %d loading mailcomponents.dll - mail send and receive is disabled.", retval );
					s.Format("DBMonitorThread::MailCheck - Error %d loading mailcomponents.dll - mail send and receive is disabled.", retval);
					DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);					
				}
				else
				{
					s.Format("DBMonitorThread::MailCheck - Successfully loaded mailcomponents.dll");
					DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);

					retval = g_Object.m_MessagingComponents.Initialize();
					if(retval)
					{
						s.Format("DBMonitorThread::MailCheck - Error initializing mailcomponents.dll.");
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);		
					}
					else
					{
						s.Format("DBMonitorThread::MailCheck - Successfully initialized mailcomponents.dll.");
						DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);		
					}
				}
			}
			else
			{
				Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, L"Error %d calling UnloadDLL() - mail send and receive is disabled, attempting to restart the VisNetic MailFlow Engine Service.", retval );
				s.Format("DBMonitorThread::MailCheck - Error %d calling UnloadDLL() - mail send and receive is disabled.", retval);
				DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);				
			}

			if(retval != 0)
			{
				// restart VisNetic MailFlow Engine Service
				UINT nRestartEngine = 0;
				GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,  _T("MailCheckRestartEngine"), nRestartEngine);
				if(nRestartEngine == 0)
				{
					WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("MailCheckRestartEngine"), 1 );
				}
			}
		}
	}
	catch(...)
	{
		try
		{
			DcaTrace(_T("DCA - Error occurred in Mail Check"));
			Log( E_DBMonitorError, L"An Unkown or undefined exception has occurred in Mail Check\n");
			m_db.Disconnect();
			return 1;
		}
		catch(...)
		{
			DcaTrace(_T("DCA - Error occurred in Mail Check"));
			m_db.Disconnect();
			return 1;
		}
	}
	
	return 0;
}

int DBMonitorThread::GetEmailsToVerify()
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	PersonalData_t pd;

	try
	{	
		m_pd.clear();
		
		m_query.Initialize();
		BINDPARAM_TIME_NOLEN( m_query, Now );
		BINDPARAM_LONG( m_query, m_EmailVerifyDays );
		BINDCOL_LONG( m_query, pd.m_PersonalDataID );
		BINDCOL_TCHAR( m_query, pd.m_DataValue );
		BINDCOL_LONG( m_query, pd.m_StatusID );
		m_query.Execute( L"SELECT TOP 100 pd.PersonalDataID,pd.DataValue,pd.StatusID FROM PersonalData pd "
					     L"INNER JOIN Contacts c ON c.ContactID=pd.ContactID "
					     L"WHERE pd.PersonalDataTypeID=1 AND pd.AgentID=0 AND "
						 L"c.IsDeleted=0 AND pd.StatusDate < ? - ? AND pd.DataValue NOT IN "
						 L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 0) AND "
						 L"substring(pd.DataValue , CHARINDEX('@', pd.DataValue )+ 1,len(pd.DataValue)) NOT IN "
						 L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 1)" );

		while( m_query.Fetch() == S_OK )
		{
			m_pd.push_back(pd);
		}

	}
	catch(...)
	{
		try
		{
			DebugReporter::Instance().DisplayMessage("DBMonitorThread::GetEmailsToVerify - Error occurred in GetEmailsToVerify", DebugReporter::ENGINE);
			Log( E_DBMonitorError, L"An Unkown or undefined exception has occurred in GetEmailsToVerify\n");
			return 1;
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("DBMonitorThread::GetEmailsToVerify - Error occurred in GetEmailsToVerify Log Error", DebugReporter::ENGINE);
			return 1;
		}
	}
	
	if(m_pd.size() > 0)
	{
		if ( VerifyEmails() )
		{
			return 1;
		}
	}

	return 0;
}

int DBMonitorThread::VerifyEmails()
{
	bool bDoProcess = false;
	dca::String s;	
	s.Format("DBMonitorThread::VerifyEmails - Entering");
	DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);

	try
	{
		bool success;	
		
		sSystemEmail = g_Object.GetParameter(EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS);
		sHeloHostName = g_Object.GetParameter(EMS_SRVPARAM_VERIFY_HELO_HOST_NAME);
		nNoMx = _ttoi(g_Object.GetParameter( EMS_SRVPARAM_VERIFY_CONTACT_EMAIL_NOMX ));
		nCatchAll = _ttoi(g_Object.GetParameter( EMS_SRVPARAM_VERIFY_CONTACT_EMAIL_CATCHALL ));
		nConnectTimeout = _ttoi(g_Object.GetParameter( EMS_SRVPARAM_VERIFY_CONNECT_TIMEOUT ));
				
		tstring sTemp = (_T("<"));
		sTemp.append(sSystemEmail);
		sTemp.append(_T(">"));
		sSystemEmail.assign(sTemp);
		int nChrSize = WideCharToMultiByte( CP_ACP, 0, sSystemEmail.c_str(), sSystemEmail.size(), NULL, 0, NULL, NULL );
		char* szBuff = new char[nChrSize+1];
		WideCharToMultiByte( CP_ACP, 0, sSystemEmail.c_str(), sSystemEmail.size(), szBuff, nChrSize, NULL, NULL );
		szBuff[nChrSize] = 0;
		const char * sFromEmail = (const char *)szBuff;

		s.Format("DBMonitorThread::VerifyEmails - Set from email address to [%s]", sFromEmail);
		DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);

		for( iter = m_pd.begin(); iter != m_pd.end(); iter++ )
		{
			CkMailMan mailman;
			CkGlobal m_glob;
			// unlock Chilkat
			success = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
			if (success != true)
			{
				DebugReporter::Instance().DisplayMessage("DBMonitorThread::VerifyEmails() - Failed to unlock Global component", DebugReporter::MAIL);
				Log(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
											EMSERR_ROUTING_ENGINE,
											EMS_LOG_DBMONITOR,
											0),
								EMS_STRING_ERROR_INITIALIZE_COMPONENT);

				return 1;
			}
			
			if(sHeloHostName.length() > 0)
			{
				nChrSize = WideCharToMultiByte( CP_ACP, 0, sHeloHostName.c_str(), sHeloHostName.size(), NULL, 0, NULL, NULL );
				szBuff = new char[nChrSize+1];
				WideCharToMultiByte( CP_ACP, 0, sHeloHostName.c_str(), sHeloHostName.size(), szBuff, nChrSize, NULL, NULL );
				szBuff[nChrSize] = 0;
				const char * sHostName = (const char *)szBuff;
				mailman.put_HeloHostname(sHostName);
			}
			
			if(nConnectTimeout < 5)
				nConnectTimeout = 5;
			
			mailman.put_ConnectTimeout(nConnectTimeout);
			
			TIMESTAMP_STRUCT Now;
			GetTimeStamp( Now );

			int nStatusID = 0;

			TCHAR szRandomCode[21];
			ZeroMemory( szRandomCode, (21) * sizeof(TCHAR) );
			for( int i = 0; i < 20; i++ )
			{
				szRandomCode[i] = 'A' + rand() % 26;
			}

			tstring sTemp(iter->m_DataValue);
			tstring sRandom(szRandomCode);
			tstring::size_type pos = sTemp.find_last_of( _T("@") );
			tstring sDomain = sTemp.substr(pos+1,sTemp.length());			
			sRandom.append(_T("@"));
			sRandom.append(sDomain);

			nChrSize = WideCharToMultiByte( CP_ACP, 0, sRandom.c_str(), sRandom.size(), NULL, 0, NULL, NULL );
			szBuff = new char[nChrSize+1];
			WideCharToMultiByte( CP_ACP, 0, sRandom.c_str(), sRandom.size(), szBuff, nChrSize, NULL, NULL );
			szBuff[nChrSize] = 0;
			const char * sRandomEmail = (const char *)szBuff;			
			
			nChrSize = WideCharToMultiByte( CP_ACP, 0, iter->m_DataValue, iter->m_DataValueLen, NULL, 0, NULL, NULL );
			szBuff = new char[nChrSize+1];
			WideCharToMultiByte( CP_ACP, 0, iter->m_DataValue, iter->m_DataValueLen, szBuff, nChrSize, NULL, NULL );
			szBuff[nChrSize] = 0;
			const char * sEmail = (const char *)szBuff;
			
			s.Format("DBMonitorThread::VerifyEmails - Checking email address [%s]", sEmail);
			DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);

			CkString mailServerHostname;
			if(mailman.MxLookup(sEmail,mailServerHostname))
			{
				s.Format("DBMonitorThread::VerifyEmails - MX Lookup returned server [%s] for email address [%s]", mailServerHostname.getString(),sEmail);
				DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);
				
				mailman.put_SmtpHost(mailServerHostname);

				CkEmail email;

				email.put_Subject("This is a test");
				email.put_Body("This is a test");
				email.put_From(sFromEmail);

				email.AddTo("",sEmail);
				email.AddTo("",sRandomEmail);
								
				CkStringArray saBadAddrs;
				success = mailman.VerifyRecips(email,saBadAddrs);
				if (success != true)
				{
					nStatusID = iter->m_StatusID + 1;
					s.Format("DBMonitorThread::VerifyEmails - Verify returned with error [%s]", mailman.lastErrorText());
					DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);
					success = true;
				}
				else
				{
					if (saBadAddrs.get_Count() > 0)
					{
						int i;
						int n;
						bool bBad = false;
						n = saBadAddrs.get_Count();
						for (i = 0; i <= n - 1; i++)
						{
							if( strcmp(saBadAddrs.getString(i),sEmail) == 0 )
							{
								s.Format("DBMonitorThread::VerifyEmails - Verification failed for email address [%s]", saBadAddrs.getString(i));
								DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);							
								nStatusID = iter->m_StatusID + 1;
								bBad = true;
							}
						}

						if(!bBad)
						{
							s.Format("DBMonitorThread::VerifyEmails - Successfully verified email address [%s]", sEmail);
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);	
						}
					}
					else
					{
						// Catch-all detected
						if(nCatchAll)
						{
							dca::String x(sDomain.c_str());
							s.Format("DBMonitorThread::VerifyEmails - Catch-all account detected for email address [%s] at server [%s] add domain [%s] to the bypass", sEmail,mailServerHostname.getString(), x.c_str());
							DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);
							
							TBypass tb;
							tb.m_BypassID = 0;
							tb.m_BypassTypeID = 1;
							_tcscpy(tb.m_BypassValue, sDomain.c_str());

							m_query.Initialize();
							BINDCOL_LONG( m_query, tb.m_BypassID );
							BINDPARAM_LONG( m_query, tb.m_BypassTypeID );
							BINDPARAM_TCHAR( m_query, tb.m_BypassValue );
							m_query.Execute( L"SELECT BypassID FROM Bypass WHERE BypassTypeID=? AND BypassValue=?" );

							if( m_query.Fetch() != S_OK )
							{
								tb.m_DateCreated = Now;
								tb.Insert( m_query );
							}
						}						
					}
				}			
			}
			else
			{
				if(nNoMx)
				{
					nStatusID = iter->m_StatusID + 1;
				}
				
				s.Format("DBMonitorThread::VerifyEmails - MX Lookup failed for email address [%s]", sEmail);
				DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);
			}

			if(success)
			{
				s.Format("DBMonitorThread::VerifyEmails - Updating status for email address [%s] with StatusID [%d] and PersonalDataID [%d]", sEmail,nStatusID,iter->m_PersonalDataID);
				DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);
				
				// Udpate status
				m_query.Initialize();
				BINDPARAM_LONG( m_query, nStatusID );
				BINDPARAM_TIME_NOLEN( m_query, Now );
				BINDPARAM_LONG( m_query, iter->m_PersonalDataID );
				m_query.Execute( L"UPDATE PersonalData SET StatusID=?, StatusDate=? "
								L"WHERE PersonalDataID=?" );

				bDoProcess = true;
			}
		}
	}
	catch(...)
	{
		try
		{
			DebugReporter::Instance().DisplayMessage("DBMonitorThread::VerifyEmails - Error occurred in VerifyEmails", DebugReporter::ENGINE);
			Log( E_DBMonitorError, L"An Unkown or undefined exception has occurred in VerifyEmails\n");
			return 1;
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("DBMonitorThread::VerifyEmails - Error occurred in VerifyEmails Log Error", DebugReporter::ENGINE);
			return 1;
		}
	}	

	return 0;
}

int DBMonitorThread::ProcessEmails()
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	
	PersonalData_t pd;

	try
	{	
		nRemove = _ttoi(g_Object.GetParameter( EMS_SRVPARAM_VERIFY_CONTACT_EMAIL_REMOVE ));
		nDelete = _ttoi(g_Object.GetParameter( EMS_SRVPARAM_VERIFY_CONTACT_EMAIL_DELETE ));
	
		if(nDelete > 0)
		{
			m_query.Initialize();
			BINDPARAM_TIME_NOLEN( m_query, Now );
			BINDPARAM_LONG( m_query, nDelete );
			m_query.Execute( L"UPDATE Contacts SET IsDeleted=1,DeletedTime=?,DeletedBy=0 "
							L"WHERE ContactID IN "
							L"(SELECT pd.ContactID FROM PersonalData pd "
							L"INNER JOIN Contacts c ON c.ContactID=pd.ContactID "
							L"WHERE pd.PersonalDataTypeID=1 AND pd.AgentID=0 AND "
							L"c.IsDeleted=0 AND pd.DataValue NOT IN "
							L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 0) AND "
							L"substring(pd.DataValue , CHARINDEX('@', pd.DataValue )+ 1,len(pd.DataValue)) NOT IN "
							L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 1) AND "
							L"pd.StatusID >= ?)" );

			m_query.Initialize();
			BINDPARAM_TIME_NOLEN( m_query, Now );
			BINDPARAM_LONG( m_query, nDelete );
			m_query.Execute( L"DELETE FROM ContactGrouping "
							L"WHERE ContactID IN "
							L"(SELECT pd.ContactID FROM PersonalData pd "
							L"INNER JOIN Contacts c ON c.ContactID=pd.ContactID "
							L"WHERE pd.PersonalDataTypeID=1 AND pd.AgentID=0 AND "
							L"c.IsDeleted=0 AND pd.DataValue NOT IN "
							L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 0) AND "
							L"substring(pd.DataValue , CHARINDEX('@', pd.DataValue )+ 1,len(pd.DataValue)) NOT IN "
							L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 1) AND "
							L"pd.StatusID >= ?)" );

			m_query.Initialize();
			BINDPARAM_TIME_NOLEN( m_query, Now );
			BINDPARAM_LONG( m_query, nDelete );
			m_query.Execute( L"DELETE FROM AgentContacts "
							L"WHERE ContactID IN "
							L"(SELECT pd.ContactID FROM PersonalData pd "
							L"INNER JOIN Contacts c ON c.ContactID=pd.ContactID "
							L"WHERE pd.PersonalDataTypeID=1 AND pd.AgentID=0 AND "
							L"c.IsDeleted=0 AND pd.DataValue NOT IN "
							L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 0) AND "
							L"substring(pd.DataValue , CHARINDEX('@', pd.DataValue )+ 1,len(pd.DataValue)) NOT IN "
							L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 1) AND "
							L"pd.StatusID >= ?)" );
		}

		if(nRemove > 0 && ((nRemove < nDelete) || nDelete == 0))
		{
			m_query.Initialize();
			BINDPARAM_LONG( m_query, nRemove );
			m_query.Execute( L"DELETE FROM ContactGrouping "
							L"WHERE ContactID IN "
							L"(SELECT pd.ContactID FROM PersonalData pd "
							L"INNER JOIN Contacts c ON c.ContactID=pd.ContactID "
							L"WHERE pd.PersonalDataTypeID=1 AND pd.AgentID=0 AND "
							L"c.IsDeleted=0 AND pd.DataValue NOT IN "
							L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 0) AND "
							L"substring(pd.DataValue , CHARINDEX('@', pd.DataValue )+ 1,len(pd.DataValue)) NOT IN "
							L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 1) AND "
							L"pd.StatusID >= ?)" );

			m_query.Initialize();
			BINDPARAM_LONG( m_query, nRemove );
			m_query.Execute( L"DELETE FROM AgentContacts "
							L"WHERE ContactID IN "
							L"(SELECT pd.ContactID FROM PersonalData pd "
							L"INNER JOIN Contacts c ON c.ContactID=pd.ContactID "
							L"WHERE pd.PersonalDataTypeID=1 AND pd.AgentID=0 AND "
							L"c.IsDeleted=0 AND pd.DataValue NOT IN "
							L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 0) AND "
							L"substring(pd.DataValue , CHARINDEX('@', pd.DataValue )+ 1,len(pd.DataValue)) NOT IN "
							L"(SELECT BypassValue FROM Bypass WHERE BypassTypeID = 1) AND "
							L"pd.StatusID >= ?)" );
		}

	}
	catch(...)
	{
		try
		{
			DebugReporter::Instance().DisplayMessage("DBMonitorThread::ProcessEmails - Error occurred in ProcessEmails", DebugReporter::ENGINE);
			Log( E_DBMonitorError, L"An Unkown or undefined exception has occurred in ProcessEmails\n");
			return 1;
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("DBMonitorThread::ProcessEmails - Error occurred in ProcessEmails Log Error", DebugReporter::ENGINE);
			return 1;
		}
	}
	
	return 0;
}

int DBMonitorThread::TryAgentTicketCategory()
{
	try
	{
		DcaTrace(_T("DCA - Testing db connection.\n"));

		if( m_db.IsConnected() )
		{
			DcaTrace(_T("DCA - Ready to perform AgentTicketCategory.\n"));
			DebugReporter::Instance().DisplayMessage("DBMonitorThread::TryAgentTicketCategory - Ready to perform AgentTicketCategory.", DebugReporter::ENGINE);

			GetAgentTicketCategory();
			return 0;
		}
		else
		{
			m_db.Connect();

			DcaTrace(_T("DCA - Re-Testing db connection.\n"));

			if( m_db.IsConnected() )
			{
				DcaTrace(_T("DCA - Ready to perform AgentTicketCategory.\n"));
				DebugReporter::Instance().DisplayMessage("DBMonitorThread::TryAgentTicketCategory - DB reconnected, ready to perform AgentTicketCategory.", DebugReporter::ENGINE);

				GetAgentTicketCategory();
				return 0;
			}
		}

		return 1;
	}
	catch(dca::Exception e)
	{
		try
		{
			DcaTrace(_T("DCA - Error occurred in AgentTicketCategory"));
			DcaTrace(e.GetMessage());
			Log( E_DBMonitorError, L"%s\n", e.GetMessage());
			m_db.Disconnect();
		}
		catch(...)
		{
		}
	}
	catch(...)
	{
		try
		{
			DcaTrace(_T("DCA - Error occurred in AgentTicketCategory"));
			Log( E_DBMonitorError, L"An Unkown or undefined exception has occurred in the DBMonitorSystem Thread\n");
			m_db.Disconnect();
		}
		catch(...)
		{
		}
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetAgentTicketCategory
// 
////////////////////////////////////////////////////////////////////////////////
void DBMonitorThread::GetAgentTicketCategory( void )
{
	bool bFound = false;
	multimap<unsigned int,unsigned int> m_AgentTCs;
	multimap<unsigned int,unsigned int>::iterator iterTCs;
	multimap<unsigned int,unsigned int> m_TCAgents;
	multimap<unsigned int,unsigned int>::iterator iterAgents;
	vector<int> DeleteIDs;
	vector<int>::iterator iterDel;
	int		nATCID;
	long	nATCIDLen;
	int		nAgentID;
	long	nAgentIDLen;
	int		nTicketCategoryID;
	long	nTicketCategoryIDLen;
	int		ID;

	dca::String s;
	
	DebugReporter::Instance().DisplayMessage("DBMonitorThread::GetAgentTicketCategory - Entering.", DebugReporter::ENGINE);

	m_AgentTCs.clear();

	m_query.Initialize();
	BINDCOL_LONG( m_query, nAgentID );
	BINDCOL_LONG( m_query, nTicketCategoryID );
	m_query.Execute( L"SELECT AG.AgentID,TC.TicketCategoryID FROM Agents AG, TicketCategories TC "
					 L"WHERE AG.IsEnabled=1 AND AG.IsDeleted=0 AND AG.AgentID NOT IN "
					 L"(SELECT AgentID FROM AgentGroupings Where GroupID=2) AND TC.TicketCategoryID IN "
					 L"(SELECT DISTINCT TicketCategories.TicketCategoryID FROM TicketCategories " 
					 L"INNER JOIN Objects ON TicketCategories.ObjectID=Objects.ObjectID WHERE "
					 L"((SELECT MAX(AccessControl.AccessLevel) FROM AccessControl " 
					 L"LEFT OUTER JOIN AgentGroupings ON AccessControl.GroupID = AgentGroupings.GroupID "
					 L"INNER JOIN Objects ON Objects.ObjectID=AccessControl.ObjectID "
					 L"WHERE Objects.ObjectTypeID=6 AND Objects.ActualID=0 AND Objects.BuiltIn=1 AND "
					 L"(AccessControl.AgentID=AG.AgentID OR AgentGroupings.AgentID = AG.AgentID))>1) OR "
					 L"((SELECT MAX(AccessControl.AccessLevel) FROM AccessControl " 
					 L"LEFT OUTER JOIN AgentGroupings ON AccessControl.GroupID = AgentGroupings.GroupID " 
					 L"WHERE AccessControl.ObjectID = TicketCategories.ObjectID AND Objects.UseDefaultRights = 0 AND "
					 L"(AccessControl.AgentID=AG.AgentID OR AgentGroupings.AgentID=AG.AgentID))>1))" );


	while( m_query.Fetch() == S_OK )
	{
		m_AgentTCs.insert( pair<unsigned int,unsigned int> (nAgentID, nTicketCategoryID) );
		
	}
	
	//Clean up any duplicate records
	m_query.Reset( true );
	m_query.Execute( _T("DELETE FROM AgentTicketCategories WHERE AgentTicketCategoryID IN ")
					 _T("(SELECT ATC.AgentTicketCategoryID FROM AgentTicketCategories AS ATC WHERE ")
					 _T("Exists (SELECT AgentID, TicketCategoryID, Count(AgentTicketCategoryID) FROM ")
					 _T("AgentTicketCategories WHERE AgentTicketCategories.AgentID = ATC.AgentID AND ")
					 _T("AgentTicketCategories.TicketCategoryID = ATC.TicketCategoryID GROUP BY ")
					 _T("AgentTicketCategories.AgentID, AgentTicketCategories.TicketCategoryID HAVING ")
					 _T("Count(AgentTicketCategoryID) > 1)) AND AgentTicketCategories.AgentTicketCategoryID ")
					 _T("NOT IN (SELECT Min(AgentTicketCategoryID) FROM AgentTicketCategories AS AT ")
					 _T("WHERE Exists (SELECT AgentID, TicketCategoryID, Count(AgentTicketCategoryID) ")
					 _T("FROM AgentTicketCategories WHERE AgentTicketCategories.AgentID = AT.AgentID ")
					 _T("AND AgentTicketCategories.TicketCategoryID = AT.TicketCategoryID ")
					 _T("GROUP BY AgentTicketCategories.AgentID, AgentTicketCategories.TicketCategoryID ")
					 _T("HAVING Count(AgentTicketCategoryID) > 1)) ") );

	// Get the records from AgentTicketCategories
	m_TCAgents.clear();

	m_query.Initialize();
	BINDCOL_LONG( m_query, nATCID );
	BINDCOL_LONG( m_query, nAgentID );
	BINDCOL_LONG( m_query, nTicketCategoryID );
	m_query.Execute( L"SELECT AgentTicketCategoryID,AgentID,TicketCategoryID FROM AgentTicketCategories" );	
	while( m_query.Fetch() == S_OK )
	{
		m_TCAgents.insert( pair<unsigned int,unsigned int> (nAgentID, nTicketCategoryID) );
		
		bFound = false;
		for( iterTCs = m_AgentTCs.begin(); iterTCs != m_AgentTCs.end(); iterTCs++ )
		{
			if( iterTCs->first == nAgentID && iterTCs->second == nTicketCategoryID )
			{
				bFound = true;
				break;
			}
		}
		
		if( bFound == FALSE )
		{
			DeleteIDs.push_back(nATCID);
		}			
	}

	//Delete the ones from the DB that we didn't find
	for( iterDel = DeleteIDs.begin(); iterDel != DeleteIDs.end(); iterDel++ )
	{
		ID = (*iterDel);
		m_query.Reset( true );
		BINDPARAM_LONG( m_query, ID );
		m_query.Execute( _T("DELETE ")
							_T("FROM AgentTicketCategories ")
							_T("WHERE AgentTicketCategoryID=?") );

		
		s.Format("DBMonitorThread::GetAgentTicketCategory - Successfully deleted AgentTicketCategoryID [%d]", ID);
		DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);
	}
	
	//Add the ones that were not in the DB
	for( iterTCs = m_AgentTCs.begin(); iterTCs != m_AgentTCs.end(); iterTCs++ )
	{
		for( iterAgents = m_TCAgents.begin(); iterAgents != m_TCAgents.end(); iterAgents++ )
		{
			bFound = false;
			if( iterTCs->first == iterAgents->first && iterTCs->second == iterAgents->second )
			{
				bFound = true;
				break;
			}
		}
		if ( bFound == false )
		{
			nAgentID = iterTCs->first;
			nTicketCategoryID = iterTCs->second;
			
			m_query.Reset( true );
			BINDPARAM_LONG( m_query, nAgentID );
			BINDPARAM_LONG( m_query, nTicketCategoryID );
			m_query.Execute( _T("INSERT INTO AgentTicketCategories (AgentID,TicketCategoryID) VALUES (?,?)") );

			s.Format("DBMonitorThread::GetAgentTicketCategory - Successfully added AgentID [%d] for TicketCategoryID [%d]", nAgentID, nTicketCategoryID);
			DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ENGINE);
		}
	}
	
	DebugReporter::Instance().DisplayMessage("DBMonitorThread::GetAgentTicketCategory - Leaving.", DebugReporter::ENGINE);

}

////////////////////////////////////////////////////////////////////////////////
// 
// GetServerParameter
// 
////////////////////////////////////////////////////////////////////////////////
void DBMonitorThread::GetServerParameter( int ID, CEMSString& sValue )
{
	TCHAR szDataValue[SERVERPARAMETERS_DATAVALUE_LENGTH];
	long szDataValueLen;

	szDataValue[0] = _T('\0');

	m_query.Initialize();
	BINDPARAM_LONG( m_query, ID );
	BINDCOL_TCHAR( m_query, szDataValue );
	m_query.Execute( _T("SELECT DataValue FROM ServerParameters WHERE ServerParameterID=?"));
	m_query.Fetch();

	sValue = szDataValue;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetParameters
// 
////////////////////////////////////////////////////////////////////////////////
void DBMonitorThread::GetServerParameters(void)
{
	CEMSString  sValue;

	// Could get these from the cache, but let's get them straight from
	// the database to play it safe.

	GetServerParameter( EMS_SRVPARAM_ARCHIVE_PATH, m_sArchiveFolder );
	GetServerParameter( EMS_SRVPARAM_BACKUP_PATH, m_sBackupFolder );
	GetServerParameter( EMS_SRVPARAM_LOG_PATH, m_sLogFolder );
	GetServerParameter( EMS_SRVPARAM_REPORT_PATH, m_sReportFolder );
	GetServerParameter( EMS_SRVPARAM_INBOUND_ATTACH, m_sInboundAttachFolder );
	GetServerParameter( EMS_SRVPARAM_OUTBOUND_ATTACH, m_sOutboundAttachFolder );
	GetServerParameter( EMS_SRVPARAM_NOTE_ATTACH, m_sNoteAttachFolder );
	GetServerParameter( EMS_SRVPARAM_SPOOL_FOLDER, m_sSpoolFolder );
	GetServerParameter( EMS_SRVPARAM_TEMP_FOLDER, m_sTempFolder );
	GetServerParameter( EMS_SRVPARAM_QUARANTINE_FOLDER, m_sQuarantineFolder );

	GetServerParameter( EMS_SRVPARAM_MAINTENANCE_HIGH, sValue );
	m_nNextMaintHigh = _ttoi( sValue.c_str() );

	GetServerParameter( EMS_SRVPARAM_MAINTENANCE_LOW, sValue );
	m_nNextMaintLow = _ttoi( sValue.c_str() );

}
