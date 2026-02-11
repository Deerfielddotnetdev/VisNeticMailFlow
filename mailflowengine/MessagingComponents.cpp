// MessagingComponents.cpp: implementation of the CMessagingComponents class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MessagingComponents.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessagingComponents::CMessagingComponents()
{
	m_pDLL = NULL;
	m_bLoaded= false;
}

CMessagingComponents::~CMessagingComponents()
{
	if( m_pDLL )
	{
		FreeLibrary( m_pDLL );
		m_pDLL = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// LoadDLL
// 
////////////////////////////////////////////////////////////////////////////////
int CMessagingComponents::UnloadDLL()
{
	BOOL bRet = true;
	
	if( m_pDLL && m_bLoaded)
	{
		bRet = FreeLibrary( m_pDLL );
		m_pDLL = NULL;
	}

	if(bRet)
	{
		return 0;
	}
	
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// 
// LoadDLL
// 
////////////////////////////////////////////////////////////////////////////////
int CMessagingComponents::LoadDLL()
{
	m_pDLL = LoadLibrary( L"MailComponents.dll" );

	if ( m_pDLL )
	{	

		if ( !(m_InitMailComponents = (pInitMailComponents)	
				GetProcAddress( m_pDLL, "InitMailComponents" )))
		{
			Log( E_ErrorLoadingMsgComponents, EMS_STRING_ERROR_FINDING_MAILCOMPONENTS_EXPORT,
				 L"InitMailComponents" );
			return GetLastError();
		}

		if ( !(m_ShutdownMailComponents = (pShutdownMailComponents)	
				GetProcAddress( m_pDLL, "ShutdownMailComponents" )))
		{
			Log( E_ErrorLoadingMsgComponents, EMS_STRING_ERROR_FINDING_MAILCOMPONENTS_EXPORT,
			     L"ShutdownMailComponents" );
			return GetLastError();
		}

		if( !(m_CheckMailNow = (pCheckMailNow)
			    GetProcAddress( m_pDLL, "CheckMailNow" )))
		{
			Log( E_ErrorLoadingMsgComponents, EMS_STRING_ERROR_FINDING_MAILCOMPONENTS_EXPORT,
				 L"CheckMailNow" );
			return GetLastError();
		}
		
		if( !(m_SendMailNow = (pSendMailNow)
			    GetProcAddress( m_pDLL, "SendMailNow" )))
		{
			Log( E_ErrorLoadingMsgComponents, EMS_STRING_ERROR_FINDING_MAILCOMPONENTS_EXPORT,
				 L"SendMailNow" );
			return GetLastError();
		}

		if( !(m_TestPOP3Auth = (pTestPOP3Auth)
			    GetProcAddress( m_pDLL, "TestPOP3Auth" )))
		{
			Log( E_ErrorLoadingMsgComponents, EMS_STRING_ERROR_FINDING_MAILCOMPONENTS_EXPORT,
				 L"TestPOP3Auth" );
			return GetLastError();
		}

		if( !(m_TestSMTPAuth = (pTestSMTPAuth)
			    GetProcAddress( m_pDLL, "TestSMTPAuth" )))
		{
			Log( E_ErrorLoadingMsgComponents, EMS_STRING_ERROR_FINDING_MAILCOMPONENTS_EXPORT,
				 L"TestSMTPAuth" );
			return GetLastError();
		}

		if( !(m_RefreshServerParams = (pRefreshServerParams)
			    GetProcAddress( m_pDLL, "RefreshServerParams" )))
		{
			Log( E_ErrorLoadingMsgComponents, EMS_STRING_ERROR_FINDING_MAILCOMPONENTS_EXPORT,
				 L"RefreshServerParams" );
			return GetLastError();
		}

		// Set/Reset Registry Key RestartMailComponents
		WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"RestartMailComponents", 0 );

		m_bLoaded = TRUE;

	}
	else
	{
		DWORD dwErr = GetLastError();
		Log( E_ErrorLoadingMsgComponents, EMS_STRING_ERROR_LOADING_MAILCOMPONENTS, dwErr );
		return dwErr;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Initialize
// 
////////////////////////////////////////////////////////////////////////////////
int CMessagingComponents::Initialize()
{
	int nRetval = -1;

	DebugReporter::Instance().DisplayMessage("CMessagingComponents::Initialize - telling MailComponentDLL to to Initialize", DebugReporter::ENGINE);

	if( m_bLoaded ) 
	{
		nRetval = m_InitMailComponents( g_Object.m_ThreadManager.GetThreadID( CThreadManager::Worker ));
	}

	if( nRetval != 0 )
	{
		Log( E_ErrorCallingMsgComponents, EMS_STRING_ERROR_MAILCOMPONENTS_INIT, nRetval );
	}

	return nRetval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Shutdown
// 
////////////////////////////////////////////////////////////////////////////////
int CMessagingComponents::Shutdown()
{
	int nRetval = -1;
	bool bRetval = true;

	DebugReporter::Instance().DisplayMessage("CMessagingComponents::Shutdown - telling MailComponentDLL to to Shutdown", DebugReporter::ENGINE);

	if( m_bLoaded ) 
	{
		nRetval = m_ShutdownMailComponents();
	}
	
	if( nRetval != 0 )
	{
		Log( E_ErrorCallingMsgComponents, EMS_STRING_ERROR_MAILCOMPONENTS_SHUTDOWN, nRetval );
	}
	
	return nRetval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// CheckMailNow
// 
////////////////////////////////////////////////////////////////////////////////
int CMessagingComponents::CheckMailNow()
{
	int nRetval = -1;

	DebugReporter::Instance().DisplayMessage("CMessagingComponents::CheckMailNow - telling MailComponentDLL to to CheckMailNow", DebugReporter::ENGINE);

	if( m_bLoaded ) 
	{
		nRetval = 0;
		m_CheckMailNow();
	}

	if( nRetval != 0 )
	{
		Log( E_ErrorCallingMsgComponents, EMS_STRING_ERROR_MAILCOMPONENTS_CHECKMAILNOW, nRetval );
	}

	return nRetval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SendMailNow
// 
////////////////////////////////////////////////////////////////////////////////
int CMessagingComponents::SendMailNow()
{
	int nRetval = -1;

	DebugReporter::Instance().DisplayMessage("CMessagingComponents::SendMailNow - telling MailComponentDLL to to SendMailNow", DebugReporter::ENGINE);

	if( m_bLoaded ) 
	{
		nRetval = 0;
		m_SendMailNow();
	}

	if( nRetval != 0 )
	{
		Log( E_ErrorCallingMsgComponents, EMS_STRING_ERROR_MAILCOMPONENTS_SENDMAILNOW, nRetval );
	}

	return nRetval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// 
// 
////////////////////////////////////////////////////////////////////////////////
int CMessagingComponents::TestPOP3Auth( const tstring& sHostName, 
										const UINT nPort, 
										const BOOL bIsAPOP, 
										const tstring& sUserName, 
										const tstring& sPassword,
										const UINT nTimeoutSecs,
										const UINT isSSL )
{
	int nRetval = -1;

	DebugReporter::Instance().DisplayMessage("CMessagingComponents::TestPOP3Auth - telling MailComponentDLL to to TestPOP3Auth", DebugReporter::ENGINE);

	if( m_bLoaded ) 
	{
		nRetval = m_TestPOP3Auth( sHostName, nPort, bIsAPOP, sUserName, sPassword, nTimeoutSecs, isSSL );
	}

	if( nRetval != 0 )
	{
		Log( E_ErrorCallingMsgComponents, EMS_STRING_ERROR_MAILCOMPONENTS_TESTPOP3AUTH, nRetval );
	}

	return nRetval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// TestSMTPAuth
// 
////////////////////////////////////////////////////////////////////////////////
int CMessagingComponents::TestSMTPAuth( const tstring& sHostName, 
										const UINT nPort, 
										const tstring& sUserName, 
										const tstring& sPassword,
										const UINT nTimeoutSecs,
										const UINT isSSL )
{
	int nRetval = -1;

	DebugReporter::Instance().DisplayMessage("CMessagingComponents::TestSMTPAuth - telling MailComponentDLL to to TestSMTPAuth", DebugReporter::ENGINE);

	if( m_bLoaded ) 
	{		
		nRetval = m_TestSMTPAuth( sHostName, nPort, sUserName, sPassword, nTimeoutSecs, isSSL );
	}

	if( nRetval != 0 )
	{
		Log( E_ErrorCallingMsgComponents, EMS_STRING_ERROR_MAILCOMPONENTS_TESTSMPTAUTH, nRetval );
	}

	return nRetval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RefreshServerParams
// 
////////////////////////////////////////////////////////////////////////////////
int CMessagingComponents::RefreshServerParams(void)
{
	int nRetval = -1;

	DebugReporter::Instance().DisplayMessage("CMessagingComponents::RefreshServerParams - telling MailComponentDLL to reload", DebugReporter::ENGINE);

	if( m_bLoaded ) 
	{		
		nRetval = m_RefreshServerParams();
	}

	if( nRetval != 0 )
	{
		Log( E_ErrorCallingMsgComponents, EMS_STRING_ERROR_MAILCOMPONENTS_REFRESHSRVPRM, nRetval );
	}

	return nRetval;	
}