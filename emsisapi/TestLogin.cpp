// TestLogin.cpp: implementation of the CTestLogin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestLogin.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTestLogin::CTestLogin(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{

}

CTestLogin::~CTestLogin()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// Run - main entry point
// 
// testlogin.ems URL parameters:
// qu= 'ms' - message source || 'md' - messagedestination
// ho= 'hostname'
// pt= 'port'
// un= 'user name'
// pw= 'pass word'
// ct= 'connection timeout'
// ap= '1' - true || '0' - false (isAPOP) : only for message source, otherwise do not expect
////////////////////////////////////////////////////////////////////////////////
int CTestLogin::Run( CURLAction& action )
{
	// Check security
	RequireAdmin();

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("testlogin") ) == 0)
	{
		return StartTest( action );
	}

	GetXMLGen().AddChildElem( _T("LoginTestResults") );
	GetXMLGen().AddChildAttrib( _T("Code"), GetISAPIData().m_pSession->m_nLoginTestResult );

	switch( GetISAPIData().m_pSession->m_nLoginTestResult )
	{
	case CAgentSession::NoTestInProgress:
		GetXMLGen().AddChildAttrib( _T("Description"), _T("No test in progress") );
		break;

	case CAgentSession::TestInProgress:
		GetXMLGen().AddChildAttrib( _T("Description"), _T("Test in progress") );
		break;

	case CAgentSession::TestSuccessful:
		GetXMLGen().AddChildAttrib( _T("Description"), _T("Test successful") );
		break;

	case CAgentSession::TestFailedConnect:
		GetXMLGen().AddChildAttrib( _T("Description"), _T("Error connecting to host") );
		break;

	case CAgentSession::TestFailedAuth:
		GetXMLGen().AddChildAttrib( _T("Description"), _T("Authentication failed") );
		break;

	case CAgentSession::TestFailedAuthNotSupported:
		GetXMLGen().AddChildAttrib( _T("Description"), _T("Authentication not supported") );
		break;

	case CAgentSession::TestFailedUnknown:
		GetXMLGen().AddChildAttrib( _T("Description"), _T("Test failed for an unknown reason") );
		break;
		
	}

	return 0;
}


int CTestLogin::StartTest( CURLAction& action )
{
	tstring sType;
	tstring sHostName;
	tstring sUserName;
	tstring sPassword;
	CEMSString sURL;
	CTestLoginThread* pThread = new CTestLoginThread( GetISAPIData().m_SessionMap );
	wchar_t* pHostName;
	wchar_t* pUserName;
	wchar_t* pPassword;
	int sslMode = 0;
	int isSSL = 0;

	GetISAPIData().GetURLString( _T("qu"), sType, false );
	GetISAPIData().GetURLLong( _T("pt"), pThread->nPort, false );
	GetISAPIData().GetURLString( _T("ho"), sHostName, false );
	GetISAPIData().GetURLString( _T("un"), sUserName, false );
	GetISAPIData().GetURLString( _T("pw"), sPassword, false );
	GetISAPIData().GetURLLong( _T("ct"), pThread->nTimeout, false );
	GetISAPIData().GetURLLong( _T("sl"), isSSL, false );
	GetISAPIData().GetURLLong( _T("sm"), sslMode, true );

	if ((sslMode == 1 || sslMode == 2) && isSSL == 1)
	{
		pThread->isSSL = sslMode;
	}
	else
	{
		pThread->isSSL = isSSL;
	}

	if( sPassword.compare( _T("%%AP%%") ) == 0 && sType.compare( _T("ms") ) == 0)
	{
		TMessageSources ms;
		GetQuery().Initialize();
		BINDPARAM_TCHAR_STRING( GetQuery(), sUserName );
		BINDCOL_TCHAR( GetQuery(), ms.m_AuthPassword );
		GetQuery().Execute( _T("SELECT Password FROM Agents WHERE LoginName=? AND IsEnabled=1 AND IsDeleted=0"));	
		GetQuery().Fetch();

		CEMSString sDecrypt;
		sDecrypt.assign(ms.m_AuthPassword);
		sDecrypt.Decrypt();
		sPassword.assign(sDecrypt.c_str());
	}

	sURL.Format( _T("%stestloginresults.ems?OutputXML=1"),
					GetISAPIData().m_sURLSubDir.c_str() );
	
	GetXMLGen().AddChildElem( _T("Test") );
	GetXMLGen().AddChildAttrib( _T("URL"), sURL.c_str() );
	GetXMLGen().AddChildAttrib( _T("qu"), sType.c_str() );
	GetXMLGen().AddChildAttrib( _T("pt"), pThread->nPort );
	GetXMLGen().AddChildAttrib( _T("ho"), sHostName.c_str() );
	GetXMLGen().AddChildAttrib( _T("un"), sUserName.c_str() );
	GetXMLGen().AddChildAttrib( _T("pw"), sPassword.c_str() );
	GetXMLGen().AddChildAttrib( _T("ct"), pThread->nTimeout );
	GetXMLGen().AddChildAttrib( _T("sl"), isSSL );
	GetXMLGen().AddChildAttrib( _T("sm"), sslMode );

	if( sType.compare( _T("ms") ) == 0 )
	{
		// Message Source
		GetISAPIData().GetURLLong( _T("ap"), pThread->isAPOP, false );
		GetXMLGen().AddChildAttrib( _T("ap"), pThread->isAPOP );
		pThread->nType = 0;

	}
	else if ( sType.compare( _T("md") ) == 0 )
	{
		// Message destination
		pThread->nType = 1;
	}
	else
	{
		// Invalid type
		THROW_EMS_EXCEPTION( E_InvalidRequest, _T("qu contains invalid type") );
	}

	pHostName = new wchar_t[sHostName.size()+1];
	pUserName = new wchar_t[sUserName.size()+1];
	pPassword = new wchar_t[sPassword.size()+1];

	MultiByteToWideChar( CP_ACP, 0, sHostName.c_str(), sHostName.size()+1, pHostName, sHostName.size()+1 );
	MultiByteToWideChar( CP_ACP, 0, sUserName.c_str(), sUserName.size()+1, pUserName, sUserName.size()+1 );
	MultiByteToWideChar( CP_ACP, 0, sPassword.c_str(), sPassword.size()+1, pPassword, sPassword.size()+1 );
	
	pThread->HostName = SysAllocString( pHostName );
	pThread->UserName = SysAllocString( pUserName );
	pThread->Password = SysAllocString( pPassword );

	delete[] pHostName;
	delete[] pUserName;
	delete[] pPassword;

	pThread->m_AgentID = GetSession().m_AgentID;

	pThread->SetKillEvent( GetISAPIData().m_hKillEvent );

	GetISAPIData().m_SessionMap.SetLoginTestResults( pThread->m_AgentID, CAgentSession::TestInProgress );

	// Start the thread
	pThread->StartThread();

	return 0;
}

/*
////////////////////////////////////////////////////////////////////////////////
// 
// SendFirstChunk
// 
////////////////////////////////////////////////////////////////////////////////
void CTestLogin::SendFirstChunk(void)
{
	HSE_SEND_HEADER_EX_INFO HeaderExInfo;

	HeaderExInfo.pszStatus = _T("200 OK");
    HeaderExInfo.pszHeader = _T("Content-type: text/html\r\n\r\n");
    HeaderExInfo.cchStatus = strlen( HeaderExInfo.pszStatus );
    HeaderExInfo.cchHeader = strlen( HeaderExInfo.pszHeader );
    HeaderExInfo.fKeepConn = FALSE;

 
	GetISAPIData().m_pECB->ServerSupportFunction( GetISAPIData().m_pECB->ConnID,
												  HSE_REQ_SEND_RESPONSE_HEADER_EX,
												  &HeaderExInfo,
												  NULL, NULL );

	SendChunk( "<html><head><title>Login Test</title></head><body>Please Wait...\r\n" );
}



////////////////////////////////////////////////////////////////////////////////
// 
// SendRepetitiveChunk
// 
////////////////////////////////////////////////////////////////////////////////
void CTestLogin::SendRepetitiveChunk(void)
{
	SendChunk( ".\r\n" );
}


////////////////////////////////////////////////////////////////////////////////
// 
// SendLastChunk
// 
////////////////////////////////////////////////////////////////////////////////
void CTestLogin::SendLastChunk(void)
{
	switch( nResult )
	{

	case 0:
		SendChunk( "<P>Authentication Succeeded</body></html>" );
		break;

	case 4: 
		SendChunk( "<P>Connect Failed</body></html>");
		break;

	case 5: 
		SendChunk( "<P>Authentication Failed</body></html>");
		break;

	case 6: 
		SendChunk( "<P>Authentication Method Unsupported</body></html>");
		break;

	default:
		SendChunk( "<P>Unknown Error Occurred</body></html>" );
		break;
	}
}

*/


CTestLoginThread::CTestLoginThread( CSessionMap& SessionMap ) 
 : m_SessionMap( SessionMap ), CThread(NULL)
{
	HostName = NULL;
	UserName = NULL;
	Password = NULL;
}

CTestLoginThread::~CTestLoginThread()
{
	if( HostName )
		SysFreeString( HostName );

	if( UserName )
		SysFreeString( UserName );

	if( Password )
		SysFreeString( Password );

	delete this;
}


unsigned int CTestLoginThread::Run()
{
	BOOL bRet;
	long nResult = -1;

	CoInitialize(NULL);

	CRoutingEngine*	pRoutingEngine = new CRoutingEngine;

	if( nType == 0 )
	{
		bRet = pRoutingEngine->TestPOP3Auth( HostName, nPort, (isAPOP == 0) ? 0 : 0xffff, 
			                                     UserName, Password, nTimeout, isSSL, &nResult );
	}
	else	
	{
		bRet = pRoutingEngine->TestSMTPAuth( HostName, nPort, UserName, Password, nTimeout, isSSL, &nResult );
	}

	delete pRoutingEngine;

	CoUninitialize();

	if( WaitForSingleObject( m_hKillEvent, 0 ) == WAIT_OBJECT_0 )
	{
		// If kill event is set, exit gracefully
		return 0;
	}

	switch( nResult )
	{
	case 0: 
		nResult = CAgentSession::TestSuccessful;
		break;

	case 4:
		nResult = CAgentSession::TestFailedConnect;
		break;
		
	case 5:
		nResult = CAgentSession::TestFailedAuth;
		break;

	case 6:
		nResult = CAgentSession::TestFailedAuthNotSupported;
		break;

	default:
		nResult = CAgentSession::TestFailedUnknown;
		break;
	}

	m_SessionMap.SetLoginTestResults( m_AgentID, nResult );

	return 0;
}