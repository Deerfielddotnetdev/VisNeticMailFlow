////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/WorkerThread.cpp,v 1.2.2.4.2.2 2006/09/05 14:16:08 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// WorkerThread.cpp: implementation of the CWorkerThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WorkerThread.h"
#include "Authenticate.h"
#include "RegistryFns.h"
#include ".\MailFlowServer.h"


#define LOGIN_PAGE "login.ems"
#define DEMO_PAGE "demo.ems"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWorkerThread::CWorkerThread(CSharedObjects* pShared) : m_query(m_db), CISAPIThread(pShared)
{
	m_pISAPIData = NULL;
	m_pECB = NULL;
}

CWorkerThread::~CWorkerThread()
{
}


////////////////////////////////////////////////////////////////////////////////
// 
// Initialize
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::Initialize(void)
{
	// Seed the random number generator for creating session IDs
	srand( GetTickCount() + m_nThreadID );

	CoInitialize(NULL);
	
	m_XSLT.CreateComponents();
	
	try
	{
		m_db.Connect();
	}
	catch(...) 
	{ 
		// Close the database connection
		CloseDatabase();
	}

	// create the ISAPI data object
	m_pISAPIData = new CISAPIData( m_query, m_pShared->XMLCache(), m_pShared->SessionMap(), m_pShared->LicenseMgr() );

	GetSourcePaths();

	// Pass a copy of the thread's kill event
	m_pISAPIData->m_hKillEvent = m_hKillEvent;
	m_pISAPIData->m_sURLSubDir = m_szURLSubDir;
	m_pISAPIData->m_pSession = &m_Session;
	m_pISAPIData->m_pBrowserSession = &m_BrowserSession;

	m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_SystemStarting),
					_T("Worker Thread [%d] Initializing"), GetCurrentThreadId() );
}


////////////////////////////////////////////////////////////////////////////////
// 
// UnInitialize
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::UnInitialize(void)
{
	m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_SystemStopping),
					_T("Worker Thread [%d] UnInitializing"), GetCurrentThreadId() );

	if ( m_pISAPIData)
		delete m_pISAPIData;

	CloseDatabase();
	
	m_XSLT.ReleaseComponents();

	CoUninitialize();
}

////////////////////////////////////////////////////////////////////////////////
// 
// OpenDatabase
// 
////////////////////////////////////////////////////////////////////////////////
bool CWorkerThread::OpenDatabase( bool bSendErrorPage )
{
	try
	{
		m_db.Connect();

		return true;
	}
	catch( ODBCError_t error )
	{
		CloseDatabase();

		// log the error
		m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_ISAPI_EXTENSION, E_ODBCError ),
											 _T("Agent: %s, URL: %s?%s, ODBC Exception: %s"),
											 m_Session.m_AgentUserName.c_str(), 
											 (m_pECB) ? m_pECB->lpszPathInfo : _T("N/A"),
											 (m_pECB) ? m_pECB->lpszQueryString : _T(""),
											 (TCHAR*) error.szErrMsg );

		CEMSString sMsg;
		sMsg.Format( _T("Error connecting to database: %s"), error.szErrMsg );

		if ( bSendErrorPage )
		{
			SendErrorPage( sMsg.c_str() );			

			if(!m_bFinishRequest)
			{
				FinishRequest(0);
				m_bFinishRequest = true;
			}
		}
		
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// CloseDatabase
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::CloseDatabase(void)
{
	if( m_db.IsConnected() )
	{
		m_query.Close();
		m_db.Disconnect();
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Run - Main thread function
// 
////////////////////////////////////////////////////////////////////////////////
unsigned int CWorkerThread::Run()
{
	bool bRun = true;
	DWORD dwRet;
	HANDLE hEvents[3] = { m_hKillEvent, m_pShared->Queue().GetNewJobEvent(), m_hReloadEvent };
	CJob Job;

	Initialize();
	
	while ( bRun )
	{
		m_pECB = NULL;

		// check global DB Maintenance flag
		if( m_pShared->m_bDBMaintenanceRunning && m_db.IsConnected() )
		{
			CloseDatabase();
		}
		
		try
		{
			// could wait for other events here as well...
			dwRet = WaitForMultipleObjects(3, hEvents, FALSE, 1000 );

			switch( dwRet )
			{
			case WAIT_TIMEOUT:
				break;

			case WAIT_OBJECT_0:				
				m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_SystemStopping),
					_T("Worker Thread [%d] received Stop Signal"), GetCurrentThreadId() );		
				
				bRun = false;
				break;

			case WAIT_OBJECT_0 + 1:
				try
				{
					// get a job
					if( m_pShared->Queue().GetJob( Job ) == 0 )
					{
						m_pECB = Job.m_pECB;

						m_bFinishRequest = false;
						
						if(m_id < 2147483647)
							++m_id;
						else
							m_id = 1;

						m_nCount = m_id;

						if ( !m_pECB->lpszQueryString ) 
							m_pECB->lpszQueryString = _T("");

	#if defined(_DEBUG)
						if ( m_pECB )
						{
							TCHAR m_szVar[260];
							m_szVar[0] = _T('\0');
							DWORD dwSize = sizeof(m_szVar) - sizeof(TCHAR);
							m_pECB->GetServerVariable( m_pECB->ConnID, "CONTENT_TYPE", m_szVar, &dwSize );
							
							m_szVar[0] = _T('\0');
							dwSize = sizeof(m_szVar) - sizeof(TCHAR);
							m_pECB->GetServerVariable( m_pECB->ConnID, "HTTP_COOKIE", m_szVar, &dwSize );
						}
	#endif

						// set the username in case we get an exception before we get the session information.
						m_Session.m_AgentUserName = _T("Unknown");
						m_Session.m_AgentID = 0;
						m_Session.m_id = m_id;

						// initialize the request
						InitializeRequest();
						m_pISAPIData->Initialize( m_pECB );

						{
							dca::String o;
							o.Format("CWorkerThread::Run - Initialized job for conn id [ %d ] job id [ %d ] in thread [ %d ]", m_pECB->ConnID, m_id, m_nThreadID);
							DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
						}


						// Is the requested page anything other than dbmaintenance?
						if( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("dbmaintenanceresults"),20 ) != 0 )
						{
							// do we need to bail out because database maintenance
							/*if( m_pShared->m_bDBMaintenanceRunning )
							{
								if( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("login"),5 ) != 0 )
								{
									if( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("sessiontickler"),14 ) != 0 )
									{
										SendErrorPage( _T("The Visnetic MailFlow Engine is performing database maintenance."),false, _T("Please Wait") );

										if(!m_bFinishRequest)
										{
											FinishRequest(0);
											m_bFinishRequest = true;
										}

										break;
									}
								}
							}*/

							// do we have a database connection?
							if (!m_db.IsConnected())
							{
								// attempt to connect to the database
								if ( !OpenDatabase(true) )
									break;
							}
						}				

						if( m_db.IsConnected() ) // In case we're doing dbmaintenanceresults
						{				
							// build cache maps				
							if( m_pShared->XMLCache().GetDirty() )
								m_pShared->XMLCache().BuildAllMaps( m_query );

							// reset locks
							if ( m_pShared->SessionMap().GetNeedToResetLocks() )
								m_pShared->SessionMap().ResetLocks( m_query );
						}
						
						DoJob();
					}
				}
				catch(CEMSException EMSException) 
				{ 
					dca::String o;
					o.Format("CWorkerThread::Run - Caught EMS Exception in job id [ %d ] in thread [ %d ]", m_id, m_nThreadID);
					DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					HandleEMSException( EMSException );
				} 
				catch(ODBCError_t error) 
				{ 
					dca::String o;
					o.Format("CWorkerThread::Run - Caught ODBC Error in job id [ %d ] in thread [ %d ]", m_id, m_nThreadID);
					DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					HandleODBCError( error );  
				}
				catch(dca::Exception& e)
				{
					dca::String o;
					o.Format("CWorkerThread::Run - Caught DCA Exception in job id [ %d ] in thread [ %d ]", m_id, m_nThreadID);
					DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					HandleDCAException(e.GetMessage());
				}
				catch(std::exception& e)
				{
					dca::String o;
					o.Format("CWorkerThread::Run - Caught STD Exception in job id [ %d ] in thread [ %d ]", m_id, m_nThreadID);
					DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					dca::WString d(e.what());
					HandleDCAException(d.c_str());
				}
				catch(...)
				{
					dca::String o;
					o.Format("CWorkerThread::Run - Caught Unhandled Exception in job id [ %d ] in thread [ %d ]", m_id, m_nThreadID);
					DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					HandleUnhandledException();
				}
				break;

			case WAIT_OBJECT_0 + 2:
				// Reload Event
				if( m_db.IsConnected() )
				{
					CloseDatabase();
				}
				DebugReporter::Instance().SetInitialized(0);
				ResetEvent( m_hReloadEvent );
				break;

			}
		}
		catch(...)
		{
			HandleUnhandledException();
		}
	}

	UnInitialize();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoJob is called when we have a database connection and have just received
// a job from the Queue
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::DoJob(void)
{
	m_bLoginPage = false;
	TCHAR* p = _tcsrchr( m_pECB->lpszPathInfo, _T('/') );

#if defined(DEMO_VERSION)
	// a cookie is not required to create a demo account...
	if ( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("demo"), 4 ) == 0 )
	{
		m_bLoginPage = true;
		ServePage();
		if(!m_bFinishRequest)
		{
			FinishRequest(0);
			m_bFinishRequest = true;
		}
		return;
	}
#endif

	// a cookie is not required to unsubscribe
	if ( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("unsub"), 5 ) == 0 )
	{
		m_bLoginPage = true;
		ServePage();
		if(!m_bFinishRequest)
		{
			FinishRequest(0);
			m_bFinishRequest = true;
		}
		return;
	}

	GetSessionCookie();

	if( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("sessiontickler"), 14 ) == 0 )
	{
		dca::String o;
		//o.Format("CWorkerThread::DoJob - sessiontickler for Session [ %s ]", m_szSessionID);
		//DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		//DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - sessiontickler", DebugReporter::ISAPI, GetCurrentThreadId());

		int nRet = m_pShared->SessionMap().GetSession( m_szSessionID, m_Session, m_BrowserSession );
		this->m_pISAPIData->m_xmlgen.AddChildElem( _T("Session") );
		this->m_pISAPIData->m_xmlgen.AddChildAttrib( _T("Alive"), (nRet == 0) ? _T("1") : _T("0") );

		o.Format("CWorkerThread::DoJob - sessiontickler for Session [ %s ] returning [%d]", m_szSessionID, nRet);
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
		SendXML( m_pISAPIData->m_xmlgen.GetDoc() );
	}
	else
	{
		// is this the login page?
		m_bLoginPage = ( p && _tcsicmp( p + 1, LOGIN_PAGE ) == 0 );
		
		// if we are GETting the login page, then make sure we set a cookie
		if( m_bLoginPage && _tcsicmp( m_pECB->lpszMethod, "GET" ) == 0 )
		{
			DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - if we are getting the login page, then make sure we set a cookie", DebugReporter::ISAPI, GetCurrentThreadId());
			GetLoginPage();
		}
		else
		{
			int nAgentID = 0;

			m_pISAPIData->GetURLLong( _T("ck_agentid"), nAgentID, true );
			
			dca::String o;
			o.Format("CWorkerThread::DoJob - Request is for page [ %s ] ck_agentid is [%d]", m_pISAPIData->m_sPage.c_str(),nAgentID);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			
			if( !m_bSessionCookieFound && nAgentID != 0 ){
	
				dca::String s;
				s.Format("* CWorkerThread::DoJob - looking for lost coookie for agent id [ %d ]", nAgentID);
				DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

				LookForLostCookie( nAgentID );
			}

			if( m_bSessionCookieFound )
			{
				if( m_bLoginPage && _tcsicmp( m_pECB->lpszMethod, "POST" ) == 0 )
				{
					// Posting the login page - authenticate username and password
					DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - Posting the login page - authenticate username and password", DebugReporter::ISAPI, GetCurrentThreadId());
					Authenticate( true );
				}
				else
				{
					unsigned int id = m_Session.m_id;

					if( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("ticketboxlist"), 13 ) == 0 )
					{
						int nKeepAlive = 0;
						
						if( m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_SESSION_KEEP_ALIVE, srvparam ) )
						{
							nKeepAlive = _ttoi( srvparam.m_DataValue );
						}
						if( nKeepAlive == 1 )
						{
							if( m_pShared->SessionMap().GetSession( m_szSessionID, m_Session, m_BrowserSession, 2 ) != 0 )
							{
								DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - failed to get agent session and browser sesssion from session id", DebugReporter::ISAPI, GetCurrentThreadId());
								m_bSessionCookieFound = FALSE;
							}
						}
						else
						{
							if( m_pShared->SessionMap().GetSession( m_szSessionID, m_Session, m_BrowserSession, 1 ) != 0 )
							{
								DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - failed to get agent session and browser sesssion from session id", DebugReporter::ISAPI, GetCurrentThreadId());
								m_bSessionCookieFound = FALSE;
							}
						}
					}
					else
					{
						if( m_pShared->SessionMap().GetSession( m_szSessionID, m_Session, m_BrowserSession ) != 0 )
						{
							DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - failed to get agent session and browser sesssion from session id", DebugReporter::ISAPI, GetCurrentThreadId());
							m_bSessionCookieFound = FALSE;
						}
					}
					

					if(id != m_Session.m_id)
						m_Session.m_id = id;

					if( nAgentID != 0 && m_Session.m_AgentID != nAgentID ){
						DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - invalid agent id", DebugReporter::ISAPI, GetCurrentThreadId());
						m_bSessionCookieFound = FALSE;
					}

					if( m_bSessionCookieFound == FALSE )
					{
						// Ooops wrong session!
						DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - Ooops wrong session", DebugReporter::ISAPI, GetCurrentThreadId());
						LookForLostCookie( nAgentID );

						if( m_bSessionCookieFound )
						{
							if( m_pShared->SessionMap().GetSession( m_szSessionID, m_Session, m_BrowserSession ) != 0 )
								m_bSessionCookieFound = FALSE;
						}						
					}
					
					if( m_bSessionCookieFound )
					{
						if( m_Session.m_bIsDirty )
						{
							dca::String o;
							o.Format("CWorkerThread::DoJob - Session [%s] is dirty, calling RefreshAgentSession", m_szSessionID);
							DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
							m_pShared->SessionMap().RefreshAgentSession( m_Session, m_query );
						}
						
						if( m_Session.m_IsDeleted )
						{
							RedirectToLoginPage();
						}
						else
						{
							{
								dca::String o;
								o.Format("CWorkerThread::DoJob - We have received a job from the queue for page [ %s ] with URL string [ %s ] by agent [ %s ] with id [ %d ] at ip [ %s ]", m_pISAPIData->m_sPage.c_str(), m_pECB->lpszQueryString, m_pISAPIData->m_pSession->m_AgentUserName.c_str(), m_pISAPIData->m_pSession->m_AgentID, m_pISAPIData->m_pBrowserSession->m_IP.c_str());
								DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
							}

							m_pISAPIData->m_sSessionID = m_szSessionID;

							if( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("dbmaintenanceresults"),20 ) != 0 )
							{
								if( m_pShared->m_bDBMaintenanceRunning )
								{
									if( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("login"),5 ) != 0 )
									{
										if( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("logout"),6 ) != 0 )
										{
											if( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("sessiontickler"),14 ) != 0 )
											{
												SendErrorPage( _T("The Visnetic MailFlow Engine is performing database maintenance."),false, _T("Please Wait") );
											}
										}
									}
								}
							}
							ServePage();
						}
					}
					else
					{
						if( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("ticketboxlist"), 13 ) == 0 )
						{
							RedirectToLoginPage();
						}
						else
						{
							// since we have a session, attempt to authenticate the user
							// as the username and password URL parameters may be present
							DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - since we have a session, attempt to authenticate the user as the username and password URL parameters may be present", DebugReporter::ISAPI, GetCurrentThreadId());
							Authenticate(false);
						}
					}
				}
			}
			else
			{
				DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - no cookie found", DebugReporter::ISAPI, GetCurrentThreadId());
				// no cookie found
				if( m_bLoginPage )
				{
					SendErrorPage( _T("Your browser does not accept cookies. You will have to enable them to login.") );
				}
				else
				{
					DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - create new session", DebugReporter::ISAPI, GetCurrentThreadId());

					// create a session
					m_pShared->SessionMap().CreateNewSessionID( m_szSessionID );
					SetCookieHeader();

					DebugReporter::Instance().DisplayMessage("CWorkerThread::DoJob - attempt to authenticate as the username and password URL parameters may be present", DebugReporter::ISAPI, GetCurrentThreadId());
					// attempt to authenticate as the username and password 
					// URL parameters may be present
					Authenticate(false);
				}
			}
		}
	}

	if(!m_bFinishRequest)
	{
		FinishRequest(0);
		m_bFinishRequest = true;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Redirect the browser to the login page.  If the result
||				of an XML post, return an error.
\*--------------------------------------------------------------------------*/
void CWorkerThread::RedirectToLoginPage(void)
{
	if (!m_pISAPIData->GetXMLPost())
	{
		Redirect( LOGIN_PAGE );		
	}
	else
	{
		m_pISAPIData->SetErrorXML(E_AccessDenied, _T(""), _T("Your session has expired!"));
		SendXML( m_pISAPIData->m_xmlgen.GetDoc() );
	}				
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetLoginPage - verify the request, if the request doesn't match the 
//                URL as specified in the registry, redirect. Otherwise
//				  add the cookie to the response.
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::GetLoginPage(void)
{	
	int nForceHTTPS = 0;
	//tstring sUser;
	//tstring sPass;

	if( m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_FORCE_HTTPS, srvparam ) )
	{
		nForceHTTPS = _ttoi( srvparam.m_DataValue );
	}

	if( nForceHTTPS )
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::GetLoginPage - the force https switch is set.", DebugReporter::ISAPI, GetCurrentThreadId());

		GetServerSecure();

		if( !m_bHTTPS )
		{
			SendErrorPage( _T("Access denied - Unsecured port.") );
			m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_SECURITY_VIOLATION, E_AccessDenied),
												 _T("Attempt to access MailFlow through unsecured port from IP %s"),
												 m_BrowserSession.m_IP.c_str() );
			return;
		}
	}

	m_pShared->SessionMap().CreateNewSessionID( m_szSessionID );
	SetCookieHeader();

	TCHAR m_szUsernameParam[AGENTS_LOGINNAME_LENGTH];
	TCHAR m_szPasswordParam[AGENTS_PASSWORD_LENGTH];

	if(m_pISAPIData->GetURLTCHAR( _T("username"), m_szUsernameParam, 51, true ) 
		&& m_pISAPIData->GetURLTCHAR( _T("password"), m_szPasswordParam, 51, true ) ) 
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::GetLoginPage - username and password in URL, attempt to auth.", DebugReporter::ISAPI, GetCurrentThreadId());
		Authenticate(false);
	}
	else
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::GetLoginPage - Get client IP address and user agent.", DebugReporter::ISAPI, GetCurrentThreadId());
		GetClientIP( m_BrowserSession.m_IP );
		GetUserAgent( m_BrowserSession.m_UserAgent );
		DebugReporter::Instance().DisplayMessage("CWorkerThread::GetLoginPage - username and password not in URL, send to login page.", DebugReporter::ISAPI, GetCurrentThreadId());
		SendLoginPage(false);
	}	
}


////////////////////////////////////////////////////////////////////////////////
// 
// Authenticate
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::Authenticate( bool bPost )
{
	CURLAction action;
	int retval;
	tstring sAgentIP;
	tstring sIPLocking;

	// DebugReporter::Instance().DisplayMessage("CWorkerThread::Authenticate - Get MailFlow registration information.", DebugReporter::ISAPI, GetCurrentThreadId());
	// get registration information
#if !defined(HOSTED_VERSION)	
	//Removed in version 6.5.0.5
	/*m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_LICENSE_KEY, srvparam );
	tstring sKey(srvparam.m_DataValue);
	m_pShared->LicenseMgr().GetKeyInfoFromKey((char*) sKey.c_str() );*/
#endif
	DebugReporter::Instance().DisplayMessage("CWorkerThread::Authenticate - Get client IP address.", DebugReporter::ISAPI, GetCurrentThreadId());
	// Set the client IP address
	GetClientIP( m_BrowserSession.m_IP );
	GetUserAgent( m_BrowserSession.m_UserAgent );
		
	DebugReporter::Instance().DisplayMessage("CWorkerThread::Authenticate - Call auth to check for username and password in URL or Form.", DebugReporter::ISAPI, GetCurrentThreadId());
	
	m_pISAPIData->m_sSessionID = m_szSessionID;
	
	// check user and password and authenticate
	CAuthenticate auth( *m_pISAPIData );
	retval = auth.Run( action );
	
	if( retval == CAuthenticate::Success )
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::Authenticate - Authenticated with username and password from URL or Form.", DebugReporter::ISAPI, GetCurrentThreadId());

#if !defined(HOSTED_VERSION)
		//Removed in version 6.5.0.5
		// Check if we have a valid key
		/*if( m_pShared->LicenseMgr().m_MFKeyPresent == false || 
			( m_pShared->LicenseMgr().m_MFKeyExpired == true && 
			  m_pShared->LicenseMgr().m_MFKeyInfo.KeyType == DCIKEY_KeyType_Evaluation) )
		{
			if( m_Session.m_AgentID != EMS_AGENTID_ADMINISTRATOR )
			{
				m_pISAPIData->m_xmlgen.SetDoc( NULL );
				m_pISAPIData->m_xmlgen.AddElem( _T("root") );
				SendLoginPage( true, _T("Registration is invalid!") );
				m_pISAPIData->m_RoutingEngine.LogIt( EMSERROR( EMS_LOG_SEVERITY_WARNING,
				 									 EMSERR_ISAPI_EXTENSION,
													 EMS_LOG_AUTHENTICATION,
													 E_FailedLogin ),
											_T("Agent %s attempted to login, but the product registration is invalid"),
											m_Session.m_AgentUserName.c_str() );
				return;
			}
		}*/
#endif
		// Check for IP ranges
		auth.GetServerParameter( EMS_SRVPARAM_SESSION_IP_LOCKING, sIPLocking );

		if( _ttoi( sIPLocking.c_str() ) > 0 )
		{
			m_pShared->SessionMap().GetAgentIP( m_Session.m_AgentID, sAgentIP );
			if( sAgentIP.size() > 0 && sAgentIP != m_BrowserSession.m_IP )
			{
				m_pISAPIData->m_xmlgen.SetDoc( NULL );
				m_pISAPIData->m_xmlgen.AddElem( _T("root") );
				SendLoginPage( true, _T("Agent is already logged in!") );
				m_pISAPIData->m_RoutingEngine.LogIt( EMSERROR( EMS_LOG_SEVERITY_WARNING,
				 								 EMSERR_ISAPI_EXTENSION,
												 EMS_LOG_AUTHENTICATION,
												 E_FailedLogin ),
										_T("Agent %s attempted to login from %s, but Agent is already logged in from %s"),
										m_Session.m_AgentUserName.c_str(),
										m_BrowserSession.m_IP.c_str(),
										sAgentIP.c_str() );

				
				return;
			}
		}
		
		dca::String o;
		o.Format("CWorkerThread::Authenticate - Add session [ %s ] to SessionMap", m_szSessionID);
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
		m_pShared->SessionMap().AddSession( m_szSessionID, m_Session, m_BrowserSession );
		
		o.Format("CWorkerThread::Authenticate - Session [ %s ] added to SessionMap", m_szSessionID);
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
		if ( m_bLoginPage )
		{
			o.Format("CWorkerThread::Authenticate - We are coming from the login page so redirect to the main page");
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
			RedirectToMainPage( auth.m_HasLoggedIn );
		}
		else
		{
			tstring sEmail;
			if ( m_pISAPIData->GetFormString( _T("email"), sEmail, true ) &&
				_tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("contact"), 7 ) == 0 )
			{
				m_pShared->SessionMap().SetTempType( m_Session.m_AgentID, 1 );
				m_pShared->SessionMap().SetTempData( m_Session.m_AgentID, sEmail );				
			}
			
			TCHAR* p = _tcsrchr( m_pISAPIData->m_pECB->lpszPathInfo, _T('/') );
			
			if ( p )
			{
				tstring sURL(++p);

				if ( strlen( m_pISAPIData->m_pECB->lpszQueryString ) )
				{
					sURL += _T("?");
					sURL += m_pISAPIData->m_pECB->lpszQueryString;
				}
				
				dca::String sTemp = m_BrowserSession.m_UserAgent.c_str();
				dca::String::size_type pos = sTemp.find('.');
				dca::String sClient = sTemp.substr(0, pos);
						
				if( _tcscmp(sClient.c_str(),"MailFlowClient") && _tcscmp(sClient.c_str(),"3CXPlugIn") )
				{
					tstring sSession = m_szSessionID;
					m_pISAPIData->m_SessionMap.SetRightHandPane( sSession, sURL.c_str() );
					Redirect( _T("mailflow.ems") );
				}
				else
				{
					o.Format("CWorkerThread::Authenticate - We are not coming from the login page so redirect to [%s]", sURL.c_str());
					DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					
					Redirect( sURL.c_str() );
				}
			}
		}
	}
	else
	{	
		DebugReporter::Instance().DisplayMessage("CWorkerThread::Authenticate - Failed to authenticate with username and password from URL or Form, sending Login Page.", DebugReporter::ISAPI, GetCurrentThreadId());

		SendLoginPage( bPost );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// ServePage is called when we have a valid authenticated session
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::ServePage(void)
{
	DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - Entering.", DebugReporter::ISAPI, GetCurrentThreadId());

	CXMLDataClass* pDataObject = NULL;
	CURLAction action;

	m_pShared->URLMap().GetAction( m_pISAPIData, action );

	// create and run an object, if necessary
	if( action.m_pDataObjectCreator )
	{
		try
		{
			// create the object m_pISAPIData->m_sPage.c_str()
			action.m_pDataObjectCreator( pDataObject, *m_pISAPIData );

			if( pDataObject )
			{
				DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - pDataObject Run.", DebugReporter::ISAPI, GetCurrentThreadId());

				try
				{
					dca::String m;
					m.Format("CWorkerThread::ServePage - calling pDataObject->Run with action [Action:%d][AllowXML:%d][RightPane:%d][PageName:%s][PageTitle:%s][FileName:%s]", action.m_nAction,action.m_bAllowXMLOutput,action.m_bRightHandPane,action.m_PageName.c_str(),action.m_sPageTitle.c_str(),action.m_sFileName.c_str());
					DebugReporter::Instance().DisplayMessage(m.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
					pDataObject->Run( action );
				}
				catch (...)
				{

					DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - pDataObject catch - delete pDataObject then throw.", DebugReporter::ISAPI, GetCurrentThreadId());
					
					// release any memory allocated by ISAPI data
					if ( m_pISAPIData->GetClientDataInMemFile())
						m_pISAPIData->CloseMemFile();

					
					// delete it
					delete pDataObject;
					throw;
				}

				DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - pDataObject Run returned - delete pDataObject.", DebugReporter::ISAPI, GetCurrentThreadId());

				// release any memory allocated by ISAPI data
				if ( m_pISAPIData->GetClientDataInMemFile())
					m_pISAPIData->CloseMemFile();

				// delete it				
				delete pDataObject;
			}


			// if this was an XML HTTP post
			if ( m_pISAPIData->GetXMLPost() )
			{
				DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - XML HTTP post.", DebugReporter::ISAPI, GetCurrentThreadId());

				m_pISAPIData->m_xmlgen.SavePos();
				m_pISAPIData->m_xmlgen.ResetPos();


				if ( !m_pISAPIData->m_xmlgen.FindChildElem( _T("postresults") ) )
				{
					DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - XML HTTP postresults.", DebugReporter::ISAPI, GetCurrentThreadId());
					
					// if we made it this far the XML post was sucessful
					m_pISAPIData->m_xmlgen.AddChildElem( _T("postresults") );
					m_pISAPIData->m_xmlgen.IntoElem();
					m_pISAPIData->m_xmlgen.AddChildElem( _T("complete"), _T("true") );
					m_pISAPIData->m_xmlgen.OutOfElem();
				}

				m_pISAPIData->m_xmlgen.RestorePos();
			}
			
		}	
		catch(CEMSException EMSException) 
		{ 
			HandleEMSException( EMSException );
		} 
		catch(ODBCError_t error) 
		{ 
			HandleODBCError( error );  
		}
		catch(dca::Exception& e)
		{
			HandleDCAException(e.GetMessage());
		}
		catch(std::exception& e)
		{
			dca::WString d(e.what());
			HandleDCAException(d.c_str());
		}
		catch(...)
		{
			HandleUnhandledException();
		}
	}

	try
	{
		switch( action.m_nAction )
		{
		case CURLAction::XSL_TRANSLATE:
			{
				DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - XSL_TRANSLATE.", DebugReporter::ISAPI, GetCurrentThreadId());

				TranslateXSL( action );
				if( action.m_bRightHandPane )
				{
					m_pISAPIData->SetRightHandPane();
				}
			}
			break;

		case CURLAction::REDIRECT_URL:
			{
				DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - REDIRECT_URL.", DebugReporter::ISAPI, GetCurrentThreadId());

				Redirect( action.m_PageName.c_str() );
			}
			break;

		case CURLAction::SEND_FILE:
			{
				DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - SEND_FILE.", DebugReporter::ISAPI, GetCurrentThreadId());

				int nRet = SendFile( action.m_PageName.c_str(), action.m_sFileName.c_str(), action.m_sPageTitle.c_str() );
				if(nRet)
				{
					dca::String s;
					s.Format("CWorkerThread::ServePage - failed to send file [%s]", action.m_sFileName.c_str());
					DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				}
				else
				{
					if(lstrcmpi(m_pShared->sWebType.c_str(), "B") && m_pShared->m_nIISVer > 6)
					{
						m_bFinishRequest = true;
					}
				}
			}
			break;

		case CURLAction::SEND_TEMP_FILE:
			{
				DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - SEND_TEMP_FILE.", DebugReporter::ISAPI, GetCurrentThreadId());

				int nRet = SendFile( action.m_PageName.c_str(), action.m_sFileName.c_str(), action.m_sPageTitle.c_str() );
				DeleteFile( action.m_PageName.c_str() );
				if(nRet)
				{
					dca::String s;
					s.Format("CWorkerThread::ServePage - failed to send file [%s]", action.m_sFileName.c_str());
					DebugReporter::Instance().DisplayMessage(s.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				}
				else
				{
					if(lstrcmpi(m_pShared->sWebType.c_str(), "B") && m_pShared->m_nIISVer > 6)
					{
						m_bFinishRequest = true;
					}
				}
			}
			break;		

		case CURLAction::DO_NOTHING:
			{
				DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - DO_NOTHING.", DebugReporter::ISAPI, GetCurrentThreadId());
			}
			break;

		case CURLAction::SEND_XML:
			{
				DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - SEND_XML.", DebugReporter::ISAPI, GetCurrentThreadId());

				SendXML( m_pISAPIData->m_xmlgen.GetDoc() );
			}
			break;
		
		default:
			{
				DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - ReturnFileNotFound.", DebugReporter::ISAPI, GetCurrentThreadId());

				ReturnFileNotFound();
			}
			break;
		}
	}
	catch(CEMSException EMSException) 
	{ 
		HandleEMSException( EMSException );
	} 
	catch(ODBCError_t error) 
	{ 
		HandleODBCError( error );  
	}
	catch(dca::Exception& e)
	{
		HandleDCAException(e.GetMessage());
	}
	catch(std::exception& e)
	{
		dca::WString d(e.what());
		HandleDCAException(d.c_str());
	}
	catch(...)
	{
		HandleUnhandledException();
	}
	DebugReporter::Instance().DisplayMessage("CWorkerThread::ServePage - Leaving.", DebugReporter::ISAPI, GetCurrentThreadId());
}


////////////////////////////////////////////////////////////////////////////////
// 
// TranslateXSL
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::TranslateXSL( CURLAction& action )
{
	ISAPIPARAM param;
	tstring SrcFile;
	tstring SrcPath;
	tstring html;
	HSE_URL_MAPEX_INFO MapInfo;
	TCHAR szSrcPath[MAX_PATH];
	DWORD dwPathLength = MAX_PATH;	
	int nMajor = 0;
	int nMinor = 0;

	CMarkupSTL documentTag;
	documentTag.AddElem( _T("DOCUMENT") );
	documentTag.AddChildElem( _T("LCID"), m_pShared->XMLCache().GetSystemLocalID() );
	documentTag.AddChildElem( _T("TITLE"), action.m_sPageTitle.c_str() );
	documentTag.AddChildElem( _T("CHGNOTSAVEDMSG"), m_pISAPIData->m_sChgNotSavedMsg.c_str() );
	
	dca::String255 sServer;
	memset(sServer, 0x00, 256);
	DWORD dwSize = 255;
	if(!m_pISAPIData->m_pECB->GetServerVariable(m_pISAPIData->m_pECB->ConnID,"SERVER_NAME",sServer,&dwSize))
		SendErrorPage(_T("Failed to get server name from web server when the ISAPI was trying to use API function GetServerVariable."));


	dca::String255 sPort;
	memset(sPort, 0x00, 256);
	dwSize = 255;
	if(!m_pISAPIData->m_pECB->GetServerVariable(m_pISAPIData->m_pECB->ConnID,"SERVER_PORT",sPort,&dwSize))
		SendErrorPage(_T("Failed to get port from web server when the ISAPI was trying to use API function GetServerVariable."));

	dca::String newServer(sServer);

	if( (strcmp(sPort,_T("80"))) && (strcmp(sPort, _T("443"))) )
	{
		newServer.append(_T(":"));
		newServer.append(sPort);
	}

	dca::String sub = m_pISAPIData->m_sURLSubDir.c_str();

	dca::String::size_type pos = sub.find_last_of('/');
	if(pos != dca::String::npos)
		sub.erase(pos,1);

	if(sub.size())
		newServer.append(sub);

	documentTag.AddChildElem(_T("servername"), newServer.c_str());

	dca::String sTemp = m_BrowserSession.m_UserAgent.c_str();
	pos = sTemp.find('.');
	if(pos != dca::String::npos)
	{
		documentTag.AddChildElem(_T("useragent"), _T("MSIE"));
		_stscanf( m_BrowserSession.m_UserAgent.c_str(), "Internet.Explorer.%d.%d", &nMajor, &nMinor );
		if(nMajor > 0)
		{
			documentTag.AddChildElem(_T("msiever"), nMajor);
		}
	}
	else
	{
		documentTag.AddChildElem(_T("useragent"), m_BrowserSession.m_UserAgent.c_str());
	}
	
	tstring sUserOS;
	GetUserOS(sUserOS);
	documentTag.AddChildElem(_T("useros"), sUserOS.c_str());

	m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_SERVER_ID, srvparam);
	documentTag.AddChildElem( _T("serverid"), srvparam.m_DataValue );

	#if defined(DEMO_VERSION)
		documentTag.AddChildElem( _T("DEMO_VERSION") );
	#endif
	#if defined(HOSTED_VERSION)
		documentTag.AddChildElem( _T("HOSTED_VERSION") );
	#endif
	//DcaTrace(m_pISAPIData->m_pECB->lpszPathInfo);

	TCHAR* p = _tcsrchr( m_pISAPIData->m_pECB->lpszPathInfo, _T('/') );
	if( p != NULL )
		documentTag.AddChildElem( _T("FILENAME"), ++p );

	int nForceHTTPS = 0;
	int nVoipIntegration = 0;
	int nUseEmailDateTime = 0;
	int nCloseTicket = 2;
	int nRouteToMe = 2;
	int nUpdateFrequency = 30;
	int nStatusFrequency = 30;
	int nUseVerification = 0;
	int nAllowLogout = 0;
	int nMultipleSpell = 0;
	
	if(m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_FORCE_HTTPS, srvparam))
	{
		nForceHTTPS = _tstoi(srvparam.m_DataValue);
		if(nForceHTTPS)
		{
			documentTag.AddChildElem(_T("usehttps"), nForceHTTPS);
		}
		else
		{
			dca::String255 sHTTPS;
			memset(sHTTPS, 0x00, 256);
			DWORD dwSize = 255;
			if(!m_pISAPIData->m_pECB->GetServerVariable(m_pISAPIData->m_pECB->ConnID,"HTTPS",sHTTPS,&dwSize))
				SendErrorPage(_T("Failed to get information on whether data came from a secure port. ISAPI was trying to use API function GetServerVariable."));

			if(_strcmpi(sHTTPS, "on") == 0)
				nForceHTTPS = 1;

			documentTag.AddChildElem(_T("usehttps"), nForceHTTPS);
		}
	}
	
	if(m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_ALLOW_AGENT_LOGOFF, srvparam))
	{
		nAllowLogout = _tstoi(srvparam.m_DataValue);			
	}
	documentTag.AddChildElem(_T("allowlogout"), nAllowLogout);	

	if(m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_VOIP_INTEGRATION, srvparam))
	{
		nVoipIntegration = _tstoi(srvparam.m_DataValue);			
	}
	documentTag.AddChildElem(_T("usevoip"), nVoipIntegration);	

	if(m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_DEFAULT_CLOSE_TICKET_AFTER_SEND, srvparam))
	{
		nCloseTicket = _tstoi(srvparam.m_DataValue);			
	}
	documentTag.AddChildElem(_T("ticketclose"), nCloseTicket);

	if(m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_DEFAULT_ROUTE_TO_ME, srvparam))
	{
		nRouteToMe = _tstoi(srvparam.m_DataValue);			
	}
	documentTag.AddChildElem(_T("ticketroute"), nRouteToMe);
	if(m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_UPDATE_FREQUENCY, srvparam))
	{
		nUpdateFrequency = _tstoi(srvparam.m_DataValue);			
	}
	documentTag.AddChildElem(_T("updatefreq"), nUpdateFrequency);
	if(m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_STATUS_FREQ, srvparam))
	{
		nStatusFrequency = _tstoi(srvparam.m_DataValue);			
	}
	documentTag.AddChildElem(_T("statusfreq"), nStatusFrequency);
	if(m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_ENABLE_EMAIL_VERIFICATION, srvparam))
	{
		nUseVerification = _tstoi(srvparam.m_DataValue);			
	}
	documentTag.AddChildElem(_T("useverification"), nUseVerification);

	m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_MESSAGE_DATE, srvparam);
	nUseEmailDateTime = _tstoi(srvparam.m_DataValue);			
	documentTag.AddChildElem(_T("usedatetime"), nUseEmailDateTime);	

	m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_DATE_ENTRY_FORMAT, srvparam);
	int nDateEntryFormat = _tstoi(srvparam.m_DataValue);			
	documentTag.AddChildElem(_T("dateformat"), nDateEntryFormat);	

	m_pISAPIData->AddURLParamsToXML( documentTag );
		
	// Make sure we're at the top
	m_pISAPIData->m_xmlgen.ResetPos();
	m_pISAPIData->m_xmlgen.FindElem();

	m_pISAPIData->m_xmlgen.AddChildSubDoc( m_Session.m_AgentXML.c_str() );
	m_pISAPIData->m_xmlgen.AddChildSubDoc( documentTag.GetDoc().c_str() );
			
	if( action.m_bAllowXMLOutput )
	{
		if( m_pISAPIData->FindURLParam( _T("OutputXML"), param ) )
		{
			//DcaTrace(m_pISAPIData->m_xmlgen.GetDoc().c_str());
			SendXML( m_pISAPIData->m_xmlgen.GetDoc() );

			return;
		}
	}

	//DebugReporter::Instance().DisplayMessage("CWorkerThread::TranslateXSL - generate the full path to xsl", DebugReporter::ISAPI, GetCurrentThreadId());
	// generate the full path to the XSL
	_tcscpy( szSrcPath, m_szURLSubDir );
	_tcscat( szSrcPath, action.m_PageName.c_str() );
	ZeroMemory( &MapInfo, sizeof(MapInfo) );
	
	m_pECB->ServerSupportFunction( m_pECB->ConnID, HSE_REQ_MAP_URL_TO_PATH,
						           szSrcPath, &dwPathLength, (DWORD*) &MapInfo );

	SrcPath = szSrcPath;

	//DcaTrace(m_pISAPIData->m_xmlgen.GetDoc().c_str());
	//dca::String f;
	//f.Format("CWorkerThread::TranslateXSL - xmloutput\r\n%s", m_pISAPIData->m_xmlgen.GetDoc().c_str());
	//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	m_XSLT.TranslateXML( m_pISAPIData->m_xmlgen.GetDoc(), SrcPath, html );

	

	//DebugReporter::Instance().DisplayMessage("CWorkerThread::TranslateXSL - append xml to document", DebugReporter::ISAPI, GetCurrentThreadId());
	// append the XML to the document
	html.append( _T("<XML id=\"docXML\"><root>") );

	if ( !m_bLoginPage )
	{
		html.append( m_Session.m_AgentXML.c_str() );
	}

	html.append( documentTag.GetDoc().c_str() );
	html.append( m_pISAPIData->m_xmldoc.GetDoc() );
	html.append( _T("</root></XML>") );

	//f.Format("CWorkerThread::TranslateXSL - html\r\n%s", html.c_str());
	//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	if( _tcsicmp( action.m_PageName.c_str(), _T("newmessageattach.ems") ) == 0)
	{
		if(nMajor > 9)
		{
			SendString(html, true);
		}
		else
		{
			SendString(html, false);
		}
	}
	else
	{
		SendString(html, false);
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// HandleEMSException
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::HandleEMSException( CEMSException& EMSException )
{
	if( m_pECB )
	{
		
		dca::String f;
		f.Format("CWorkerThread::HandleEMSException - %s", EMSException.GetErrorString());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

		SendErrorPage( EMSException.GetErrorString(), false, _T("Visnetic Mailflow Exception") );
	
		if(EMSException.GetLog())
		{
			m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_ISAPI_EXTENSION, E_EMSException ),
												_T("Agent: %s, URL: %s?%s, EMS Exception: %s"),
												m_Session.m_AgentUserName.c_str(), 
												(m_pECB->lpszPathInfo) ? m_pECB->lpszPathInfo : _T("N/A"),
												(m_pECB->lpszQueryString) ? m_pECB->lpszQueryString : _T(""),
												(TCHAR*) EMSException.GetErrorString() );
		}
	}

	/*if(!m_bFinishRequest)
	{
		FinishRequest(0);
		m_bFinishRequest = true;
	}*/
}

////////////////////////////////////////////////////////////////////////////////
// 
// HandleDCAException
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::HandleDCAException( const wchar_t* err )
{
	if( m_pECB )
	{
		dca::String e(err);

		dca::String f;
		f.Format("CWorkerThread::HandleDCAException - %s", e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
		SendErrorPage( e.c_str(), false, _T("Visnetic Mailflow Exception") );
	
		m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_ISAPI_EXTENSION, E_EMSException ),
											_T("Agent: %s, URL: %s?%s, EMS Exception: %s"),
											m_Session.m_AgentUserName.c_str(), 
											(m_pECB->lpszPathInfo) ? m_pECB->lpszPathInfo : _T("N/A"),
											(m_pECB->lpszQueryString) ? m_pECB->lpszQueryString : _T(""),
											(TCHAR*) e.c_str() );
	}

	/*if(!m_bFinishRequest)
	{
		FinishRequest(0);
		m_bFinishRequest = true;
	}*/
}


////////////////////////////////////////////////////////////////////////////////
// 
// HandleODBCError
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::HandleODBCError( ODBCError_t& error )
{
	bool bIsSearchTimeOut = false;
	bool bIsDeadlock = false;

	tstring sTemp = (TCHAR*) error.szErrMsg;	
	tstring::size_type pos;

	pos = sTemp.find(_T("Query timeout expired"),0);
	if(pos != tstring::npos)
	{
		bIsSearchTimeOut = true;
	}
	
	pos = sTemp.find(_T("was deadlocked"),0);
	if(pos != tstring::npos)
	{
		bIsDeadlock = true;
	}

	if ( _tcsnicmp( m_pISAPIData->m_sPage.c_str(), _T("ticketsearchresults"), 19 ) == 0 && bIsSearchTimeOut )
	{
		if( m_pECB )
		{
			dca::String f;
			f.Format("CWorkerThread::HandleODBCError - %s", (TCHAR*) error.szErrMsg);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

			SendErrorPage( _T("Ticket search timeout, please define additional search parameters or increase the timeout value!"), false, _T("Ticket Search Timeout") );
			
			m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_ISAPI_EXTENSION, E_ODBCError ),
												_T("Agent: %s, URL: %s?%s, ODBC Exception: %s"),
												m_Session.m_AgentUserName.c_str(), 
												(m_pECB->lpszPathInfo) ? m_pECB->lpszPathInfo : _T("N/A"),
												(m_pECB->lpszQueryString) ? m_pECB->lpszQueryString : _T(""),
												(TCHAR*) error.szErrMsg );
		}
	}
	else if (bIsDeadlock)
	{
		if( m_pECB )
		{
			dca::String f;
			f.Format("CWorkerThread::HandleODBCError - %s", (TCHAR*) error.szErrMsg);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

			SendErrorPage( (TCHAR*) error.szErrMsg, false, _T("Database (ODBC) Exception") );
			
			m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_ISAPI_EXTENSION, E_ODBCError ),
												_T("Agent: %s, URL: %s?%s, ODBC Exception: %s"),
												m_Session.m_AgentUserName.c_str(), 
												(m_pECB->lpszPathInfo) ? m_pECB->lpszPathInfo : _T("N/A"),
												(m_pECB->lpszQueryString) ? m_pECB->lpszQueryString : _T(""),
												(TCHAR*) error.szErrMsg );
		}
	}
	else
	{
		if( m_pECB )
		{
			dca::String f;
			f.Format("CWorkerThread::HandleODBCError - %s", (TCHAR*) error.szErrMsg);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

			SendErrorPage( (TCHAR*) error.szErrMsg, false, _T("Database (ODBC) Exception") );
			
			m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_ISAPI_EXTENSION, E_ODBCError ),
												_T("Agent: %s, URL: %s?%s, ODBC Exception: %s"),
												m_Session.m_AgentUserName.c_str(), 
												(m_pECB->lpszPathInfo) ? m_pECB->lpszPathInfo : _T("N/A"),
												(m_pECB->lpszQueryString) ? m_pECB->lpszQueryString : _T(""),
												(TCHAR*) error.szErrMsg );
		}

		CloseDatabase();

		//g_ThreadPool.ReLoadAllThreads();
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// HandleUnhandledException
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::HandleUnhandledException( void )
{
	if( m_pECB )
	{
		dca::String f;
		f.Format("CWorkerThread::HandleUnhandledException - Agent: %s, URL: %s?%s, Unhandled exception", m_Session.m_AgentUserName.c_str(), 
			(m_pECB->lpszPathInfo) ? m_pECB->lpszPathInfo : _T("N/A"),
			(m_pECB->lpszQueryString) ? m_pECB->lpszQueryString : _T(""));
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

		SendErrorPage( _T("Unhandled exception") );

		m_pISAPIData->m_RoutingEngine.LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_ISAPI_EXTENSION, E_UnhandledException ),
			_T("Agent: %s, URL: %s?%s, Unhandled exception"), m_Session.m_AgentUserName.c_str(), 
			(m_pECB->lpszPathInfo) ? m_pECB->lpszPathInfo : _T("N/A"),
			(m_pECB->lpszQueryString) ? m_pECB->lpszQueryString : _T("") );
	}

	/*if(!m_bFinishRequest)
	{
		FinishRequest(0);
		m_bFinishRequest = true;
	}*/
}


////////////////////////////////////////////////////////////////////////////////
// 
// SendErrorXML
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::SendErrorPage( LPCTSTR szErrorMsg, bool bForceHTML, LPCTSTR szTitle )
{
	if( bForceHTML && !m_pISAPIData->GetXMLPost() )
	{
		CEMSString sError;
		sError.Format( _T("<HTML><HEAD><TITLE>VisNetic&reg; MailFlow</TITLE><BODY><H4>%s</H4></BODY></HTML>"),
					   szErrorMsg, szErrorMsg );
		if(SendString( sError ))
		{
			
		}
		return;
	}

	m_pISAPIData->m_xmlgen.SetDoc( NULL );
	m_pISAPIData->m_xmlgen.AddElem(_T("root"));
	m_pISAPIData->m_xmlgen.AddChildElem(_T("error"), szErrorMsg);
	
	if ( m_pISAPIData->GetXMLPost() )
	{
		CEMSString sEscError(szErrorMsg);
		sEscError.EscapeJavascript();

		m_pISAPIData->m_xmlgen.AddChildElem(_T("postresults"), szErrorMsg);
		m_pISAPIData->m_xmlgen.IntoElem();
		m_pISAPIData->m_xmlgen.AddChildElem(_T("error"), sEscError.c_str());
		m_pISAPIData->m_xmlgen.OutOfElem();
	}
	else
	{
		CURLAction action;

		if ( m_pISAPIData->GetUseExceptionEMS())
		{
			action.SetXSLTranslate(_T("Exception"), _T("exception.ems"));
		}
		else
		{
			m_pShared->URLMap().GetAction(m_pISAPIData, action);
		}

		m_pISAPIData->m_xmlgen.AddChildElem(_T("title"), szTitle ? szTitle : _T("An exception has occurred!"));		
		/*m_pISAPIData->m_xmlgen.AddChildElem(_T("user"));
		m_pISAPIData->m_xmlgen.IntoElem();
		m_pISAPIData->m_xmlgen.AddChildElem(_T("stylesheetname"), _T("./stylesheets/standard.css"));
		m_pISAPIData->m_xmlgen.AddChildElem(_T("menubackcolor"), _T("#D4D0C8"));
		m_pISAPIData->m_xmlgen.OutOfElem();*/

		try
		{
			TranslateXSL( action );
		}
		catch( ... )
		{
			// if that fails, then format HTML
			try
			{
				CEMSString sError;
				sError.Format( _T("<HTML><HEAD><TITLE>%s</TITLE><BODY><H2>%s</H2></BODY></HTML>"), 
					           szErrorMsg, szErrorMsg );

				if(SendString(sError))
				{
				}
			}
			catch (...)
			{
				// if that fails, give up!
			}
		}
	}

}

////////////////////////////////////////////////////////////////////////////////
// 
// RedirectToMainPage
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::RedirectToMainPage( unsigned char nHasLoggedIn )
{
	CEMSString sURL;
	int nTicketViewID;
	dca::String o;
		
	o.Format("CWorkerThread::RedirectToMainPage - Entered for SessionID [ %s ], getting My Tickets TicketBoxViewID from database", m_szSessionID);
	DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	m_query.Initialize();
	BINDPARAM_LONG( m_query, m_Session.m_AgentID );
	BINDPARAM_LONG( m_query, m_Session.m_AgentID );
	BINDCOL_LONG_NOLEN( m_query, nTicketViewID );
	m_query.Execute( _T("SELECT TicketBoxViewID FROM TicketBoxViews ")
						_T("WHERE AgentID=? AND AgentBoxID=? AND TicketBoxViewTypeID=1") );

	if( m_query.Fetch() == S_OK )
	{
		o.Format("CWorkerThread::RedirectToMainPage - Retrieved My Tickets TicketBoxViewID [ %d ] for SessionID [ %s ], redirecting to My Tickets", nTicketViewID, m_szSessionID);
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
		sURL.Format( _T("mytickets.ems?TicketBoxView=%d"), nTicketViewID );
	}
	else
	{
		o.Format("CWorkerThread::RedirectToMainPage - My Tickets TicketBoxViewID not found for SessionID [ %s ], redirecting to Agent Preferences for AgentID [ %d ]", m_szSessionID, m_Session.m_AgentID);
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
		sURL.Format( _T("apreferences.ems?selectid=%d"), m_Session.m_AgentID );
	}
	
	if( !nHasLoggedIn )
	{
		if ( CAccessControl::GetInstance().GetAgentRightLevel(m_query, m_Session.m_AgentID, EMS_OBJECT_TYPE_AGENT_PREFERENCES,0) == EMS_DELETE_ACCESS )
		{
			o.Format("CWorkerThread::RedirectToMainPage - Agent with SessionID [ %s ] has not previously logged in, redirecting to Agent Preferences for AgentID [ %d ]", m_szSessionID, m_Session.m_AgentID);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			
			sURL.Format( _T("apreferences.ems?selectid=%d"), m_Session.m_AgentID );
		}
		else
		{
			o.Format("CWorkerThread::RedirectToMainPage - Agent with SessionID [ %s ] has not previously logged in, however Agent does not have appropriate rights to Agent Preferences so redirecting to My Tickets", m_szSessionID);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());		
		
			sURL.Format( _T("mytickets.ems?TicketBoxView=%d"), nTicketViewID );
		}

		o.Format("CWorkerThread::RedirectToMainPage - Agent with SessionID [ %s ] has not previously logged in, updating AgentID [ %d ] to reflect successful login", m_szSessionID, m_Session.m_AgentID);
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());		
		
		m_query.Initialize();
		BINDPARAM_LONG( m_query, m_Session.m_AgentID );
		m_query.Execute( _T("UPDATE Agents SET HasLoggedIn=1 WHERE AgentID=?") );
	}

	o.Format("CWorkerThread::RedirectToMainPage - SetRightHandPane to [ %s ] in SessionID [ %s ]", sURL.c_str(), m_szSessionID);
	DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());		
		
	tstring sSession = m_szSessionID;
	m_pISAPIData->m_SessionMap.SetRightHandPane( sSession, sURL.c_str() ); 
	if ( m_bLoginPage )
	{
        dca::String sTemp = m_BrowserSession.m_UserAgent.c_str();
		dca::String::size_type pos = sTemp.find('.');
		dca::String sClient = sTemp.substr(0, pos);
				
		if( _tcscmp(sClient.c_str(),"MailFlowClient") && _tcscmp(sClient.c_str(),"3CXPlugIn") )
		{
			o.Format("CWorkerThread::RedirectToMainPage - Redirecting SessionID [ %s ] to mailflow.ems", sURL.c_str(), m_szSessionID);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());		
			
			Redirect( _T("mailflow.ems") );
		}
		else
		{
			TCHAR szAgentHeader[256];
			LPCTSTR szFormat;
			if ( !_tcscmp(sClient.c_str(),"MailFlowClient") )
			{
				szFormat = _T("MailFlowClientAgent: %d\r\n");
			}
			else if ( !_tcscmp(sClient.c_str(),"3CXPlugIn") )
			{
				szFormat = _T("3CXPlugInAgent: %d\r\n");
			}
			_sntprintf( (TCHAR*) szAgentHeader, 255, 
						szFormat, m_Session.m_AgentID );
			AddHeaders( szAgentHeader );
			Redirect( _T("client.ems") );
		}
			
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetLoginPageXML
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::SendLoginPage( bool bError, LPCTSTR szErrorMsg)
{
	CURLAction action;
	int nMajor = 0;
	int nMinor = 0;
	int nBuild = 0;
	
	// Check User Agent
	//GetUserAgent( m_BrowserSession.m_UserAgent );
	_stscanf( m_BrowserSession.m_UserAgent.c_str(), "Internet.Explorer.%d.%d", &nMajor, &nMinor );

    if( ( nMajor < 5 ) || ( nMajor == 5 && nMinor < 5) )
	{
		dca::String sTemp = m_BrowserSession.m_UserAgent.c_str();
		dca::String::size_type pos = sTemp.find('.');
		dca::String sClient = sTemp.substr(0, pos);
				
		if( _tcscmp(sClient.c_str(),"MailFlowClient") && 
			_tcscmp(sClient.c_str(),"3CXPlugIn") && 
			_tcscmp(sClient.c_str(),"iPad")  && 
			_tcscmp(sClient.c_str(),"iPhone")  && 
			_tcscmp(sClient.c_str(),"iPod")  && 
			_tcscmp(sClient.c_str(),"Android")  && 
			_tcscmp(sClient.c_str(),"Chrome")  && 
			//_tcscmp(sClient.c_str(),"Firefox")  && 
			_tcscmp(sClient.c_str(),"Safari") )
		{
			// If wrong browser - send error page
			CEMSString sError;
			sError.Format( _T("You are accessing VisNetic&reg; MailFlow from an unsupported Browser or OS!<br><br>")
						_T("Your unsupported Browser or OS: %s<br><br>")
						_T("MailFlow has been tested on the following Browser and OS combinations:<br><br>") 
						_T("Browsers")
						_T("<ul><li>Chrome</li>")
						//_T("<li>Firefox</li>")
						_T("<li>Internet Explorer</li>")
						_T("<li>Safari</li>")
						_T("</ul>Operating Systems")
						_T("<ul><li>Android</li>")
						_T("<li>iOS</li>")
						_T("<li>Linux</li>")
						_T("<li>OS X</li>")
						_T("<li>Windows</li>")
						_T("</ul>Utilization of any other Browser or OS may result in undesirable behavior.<br><br>")
						_T("Please visit the MailFlow <a href='http://www.visnetic.com/support'>support pages</a> for additional information regarding support for your browser."), m_BrowserSession.m_UserAgent.c_str() );

			SendErrorPage(sError.c_str(),true);
			return;
		}		
	}
	
	// Added by Mark Mohr on 01/13/2006
	{
		DebugReporter::Instance().DisplayMessage("CWorkerThread::SendLoginPage - Get SERVER_NAME from web server", DebugReporter::ISAPI, GetCurrentThreadId());
		dca::String255 sServerName;
		memset(sServerName, 0x00, 256);
		DWORD dwSize = 255;
		try
		{
			if(m_pECB->GetServerVariable(m_pECB, "SERVER_NAME", sServerName, &dwSize))
			{
				dca::String f;
				f.Format("CWorkerThread::SendLoginPage - Retrieved SERVER_NAME from web server with the value %s and size of %d", sServerName, dwSize);
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

				if(!IsNameRFC1123Compliant(sServerName))
				{
					SendErrorPage(_T("Invalid server name, the only characters that can be used in the server host name are \"A\" to \"Z\", \"a\" to \"z\", \"0\" to \"9\", and the hyphen (\"-\")."));
					return;
				}
			}
			else
			{
				DebugReporter::Instance().DisplayMessage("Failed to get server name from web server when the ISAPI was trying to use API function GetServerVariable.", DebugReporter::ISAPI, GetCurrentThreadId());
				SendErrorPage(_T("Failed to get server name from web server when the ISAPI was trying to use API function GetServerVariable."));
			}
		}
		catch( ... )
		{
			DebugReporter::Instance().DisplayMessage("Exception when the ISAPI was trying to use API function GetServerVariable, reloading all threads.", DebugReporter::ISAPI, GetCurrentThreadId());
			CloseDatabase();			
			g_ThreadPool.ReLoadAllThreads();
			GetLoginPage();			
		}
	}

	tstring sVersion;

	DebugReporter::Instance().DisplayMessage("CWorkerThread::SendLoginPage - Get MailFlow version from registry", DebugReporter::ISAPI, GetCurrentThreadId());
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          EMS_VERSION_VALUE, sVersion );

	m_pISAPIData->m_xmlgen.AddChildElem( _T("Version") );
	m_pISAPIData->m_xmlgen.AddChildAttrib( _T("Number"), sVersion.c_str() );
#if defined(HOSTED_VERSION)
	m_pISAPIData->m_xmlgen.AddChildElem( _T("License"), _T("Hosted Version") );
#else
	m_pISAPIData->m_xmlgen.AddChildElem( _T("License"), _T("Version") );
	//Removed in version 6.5.0.5
	//
	//m_pShared->XMLCache().m_ServerParameters.Query( EMS_SRVPARAM_LICENSE_KEY, srvparam );
	//tstring sKey(srvparam.m_DataValue);
	//m_pShared->LicenseMgr().GetKeyInfoFromKey((char*) sKey.c_str() );
	//if( m_pShared->LicenseMgr().m_MFKeyPresent == true )
	//{
	//	if( m_pShared->LicenseMgr().m_MFKeyInfo.KeyType == DCIKEY_KeyType_Registered )
	//	{		
	//		m_pISAPIData->m_xmlgen.AddChildElem( _T("License"), _T("Registered Version") );
	//	}
	//	else
	//	{
	//		if( m_pShared->LicenseMgr().m_MFKeyExpired == false )
	//		{
	//			m_pISAPIData->m_xmlgen.AddChildElem( _T("License"), _T("Evaluation Version") );
	//		}
	//		else
	//		{
	//			m_pISAPIData->m_xmlgen.AddChildElem( _T("License"), _T("Evaluation Expired") );
	//		}
	//	}
	//}
	//else
	//{
	//	//Let's try to get the key from the DB
	//	m_pShared->LicenseMgr().GetKeyInfo( m_query );
	//	if( m_pShared->LicenseMgr().m_MFKeyPresent == true )
	//	{
	//		WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("IsapiReload"), 0 );
	//		if( m_pShared->LicenseMgr().m_MFKeyInfo.KeyType == DCIKEY_KeyType_Registered )
	//		{		
	//			m_pISAPIData->m_xmlgen.AddChildElem( _T("License"), _T("Registered Version") );
	//		}
	//		else
	//		{
	//			if( m_pShared->LicenseMgr().m_MFKeyExpired == false )
	//			{
	//				m_pISAPIData->m_xmlgen.AddChildElem( _T("License"), _T("Evaluation Version") );
	//			}
	//			else
	//			{
	//				m_pISAPIData->m_xmlgen.AddChildElem( _T("License"), _T("Evaluation Expired") );
	//			}
	//		}
	//	}
	//}
#endif	
	if( bError )
	{
		if(szErrorMsg)
			m_pISAPIData->m_xmlgen.AddChildElem( _T("Error"), szErrorMsg );
		else
			m_pISAPIData->m_xmlgen.AddChildElem( _T("Error"), _T("Login Failed.") );
	}

	DebugReporter::Instance().DisplayMessage("CWorkerThread::SendLoginPage - Set Login Page XSL", DebugReporter::ISAPI, GetCurrentThreadId());
	action.SetXSLTranslate( _T("MailFlow Login"), LOGIN_PAGE );
	
	TranslateXSL( action );
}


////////////////////////////////////////////////////////////////////////////////
// 
// LookForLostCookie - Sometimes IE loses the cookie when opening a new window.
//                     This function checks for an existing session with the same
//                     IP address and if found, sets the cookie.
// 
////////////////////////////////////////////////////////////////////////////////
void CWorkerThread::LookForLostCookie( int nAgentID )
{
	dca::String sClientIP;
	dca::String sSessionID;
	int nRet;

	GetClientIP(sClientIP);

	dca::String f;
	f.Format("CWorkerThread::LookForLostCookie - Looking for lost cookie for AgentID %d at IP Address %s", nAgentID,sClientIP.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	nRet = m_pISAPIData->m_SessionMap.GetSessionIDFromIP(sClientIP, nAgentID, sSessionID );	

	m_bSessionCookieFound = (nRet == 0) ? TRUE : FALSE;

	if( m_bSessionCookieFound )
	{
		// save a copy
		_tcscpy( m_szSessionID, sSessionID.c_str() );

		// Set the cookie in the headers
		SetCookieHeader();
	}
}