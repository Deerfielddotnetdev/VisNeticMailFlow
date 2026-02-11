// Authenticate.cpp: implementation of the CAuthenticate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SecurityFns.h"
#include "Authenticate.h"
#include "StringFns.h"
#include <CkHttp.h>
#include <CkHttpRequest.h>
#include <CkHttpResponse.h>
#include <CkGlobal.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAuthenticate::CAuthenticate(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{

}

CAuthenticate::~CAuthenticate()
{

}


int CAuthenticate::Run( CURLAction& action )
{
	//ISAPIPARAM param;
	
	if(    !m_ISAPIData.GetFormTCHAR( _T("username"), m_szUsernameParam, 51, true ) 
		&& !m_ISAPIData.GetURLTCHAR( _T("username"), m_szUsernameParam, 51, true ) ) 
	{
		// error - field not found
		return Failed_BadForm;
	}

	if(    !m_ISAPIData.GetFormTCHAR( _T("password"), m_szPasswordParam, 51, true )
		&& !m_ISAPIData.GetURLTCHAR( _T("password"), m_szPasswordParam, 51, true ) ) 
	{
		// error - field not found
		return Failed_BadForm;
	}

	return Log( AuthenticateUser() );
}


void CAuthenticate::LogAgentAction( long nAgentID, long nActionID, CEMSString agentIP, CEMSString agentClient  )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nAgentID );
	BINDPARAM_LONG( GetQuery(), nActionID );
	BINDPARAM_TCHAR_STRING( GetQuery(), agentIP );
	BINDPARAM_TCHAR_STRING( GetQuery(), agentClient );
	GetQuery().Execute(_T("INSERT INTO AgentActivityLog (AgentID,ActivityID,Data1,Data2) VALUES (?,?,?,?)"));	
}

int CAuthenticate::Log( int retval )
{
	// Log the results
	tstring sAal;
	tstring sAaa;
	XAgentNames name;

	if(m_AgentID > 0)
	{
		GetXMLCache().m_AgentNames.Query( m_AgentID, name );
	}
		
	if ( retval == 0 )
	{
		GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_LOG, sAal );
		GetServerParameter( EMS_SRVPARAM_AGENT_ACTIVITY_AUTHENTICATION, sAaa );
		if( _ttoi( sAal.c_str() ) > 0 )
		{
			if( _ttoi( sAaa.c_str() ) > 0 )
			{
				LogAgentAction(m_AgentID,1,GetBrowserSession().m_IP.c_str(),GetBrowserSession().m_UserAgent.c_str());
			}
		}
	}

	switch( retval )
	{
	case CAuthenticate::Success:
		GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
											EMSERR_ISAPI_EXTENSION,
											EMS_LOG_AUTHENTICATION,
			                                E_SuccessfulLogin),
								  EMS_STRING_LOGON_SUCCESSFUL, 
								  name.m_Name,GetISAPIData().m_sSessionID.c_str(),
								  GetBrowserSession().m_IP.c_str() );
		break;

	case CAuthenticate::Failed_IPRange:
		GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_WARNING,
											EMSERR_ISAPI_EXTENSION,
											EMS_LOG_AUTHENTICATION,
											E_FailedLogin),
								  EMS_STRING_LOGON_FAILED_IPRANGE,
								  m_szUsernameParam,
								  GetBrowserSession().m_IP.c_str() );
		break;

	case CAuthenticate::Failed_BadPassword:
		GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_WARNING,
											EMSERR_ISAPI_EXTENSION,
											EMS_LOG_AUTHENTICATION,
											E_FailedLogin),
								  EMS_STRING_LOGON_FAILED_BAD_PASSWORD,
								  m_szUsernameParam,
								  GetBrowserSession().m_IP.c_str() );
		break;

	case CAuthenticate::Failed_NoSuchUser:
		GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_WARNING,
											EMSERR_ISAPI_EXTENSION,
											EMS_LOG_AUTHENTICATION,
											E_FailedLogin),
								  EMS_STRING_LOGON_FAILED_NO_SUCH_USER,
								  m_szUsernameParam,
								  GetBrowserSession().m_IP.c_str() );
		break;

	case CAuthenticate::Failed_BadForm:
		GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_WARNING,
											EMSERR_ISAPI_EXTENSION,
											EMS_LOG_AUTHENTICATION,
											E_FailedLogin),
								  EMS_STRING_LOGON_FAILED_NO_SUCH_USER,
								  m_szUsernameParam,
								  GetBrowserSession().m_IP.c_str() );
		break;
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// AuthenticateUser - Authenticates a user given a login name, password and
//                    IP address.
// 
////////////////////////////////////////////////////////////////////////////////
int CAuthenticate::AuthenticateUser(void )
{
	long nEscalateAgentValid = 0;
	long nApproverCount = 0;
	long nApproverGroupCount = 0;	

	GetQuery().Initialize();

	BINDPARAM_TCHAR( m_ISAPIData.m_query, m_szUsernameParam );

	BINDCOL_LONG( GetQuery(), m_AgentID );
	BINDCOL_LONG( GetQuery(), m_IsNTUser ); 
	BINDCOL_TCHAR( GetQuery(), m_Password ); 
	BINDCOL_TCHAR( GetQuery(), m_Name );
	BINDCOL_TCHAR( GetQuery(), m_NTDomain );
	BINDCOL_LONG( GetQuery(), m_StyleSheetID );
	BINDCOL_BIT( GetQuery(), m_UseIM );
	BINDCOL_BIT( GetQuery(), m_NewMessageFormat );
	BINDCOL_LONG( GetQuery(), m_EscalateToAgentID );
	BINDCOL_BIT( GetQuery(), m_QuoteMsgInReply );
	BINDCOL_TCHAR( GetQuery(), m_ReplyQuotedPrefix );
	BINDCOL_LONG( GetQuery(), m_MaxReportRowsPerPage );
	BINDCOL_BIT( GetQuery(), m_RequireGetOldest );
	BINDCOL_BIT( GetQuery(), m_HasLoggedIn );
	BINDCOL_BIT( GetQuery(), m_ForceSpellCheck );
	BINDCOL_BIT( GetQuery(), m_SignatureTopReply );
	BINDCOL_BIT( GetQuery(), m_ShowMessagesInbound );
	BINDCOL_BIT( GetQuery(), m_UsePreviewPane );
	BINDCOL_TINYINT( GetQuery(), m_CloseTicket );
	BINDCOL_TINYINT( GetQuery(), m_RouteToInbox );
	BINDCOL_BIT( GetQuery(), m_UseMarkAsRead );
	BINDCOL_LONG( GetQuery(), m_MarkAsReadSeconds );
	BINDCOL_LONG( GetQuery(), m_OutboundApprovalFromID );
	BINDCOL_BIT( GetQuery(), m_UseAutoFill );
	BINDCOL_LONG( GetQuery(), m_DefaultTicketBoxID );
	BINDCOL_LONG( GetQuery(), m_DefaultTicketDblClick );
	BINDCOL_LONG( GetQuery(), m_ReadReceipt );
	BINDCOL_LONG( GetQuery(), m_DeliveryConfirmation );
	BINDCOL_LONG( GetQuery(), m_LoginStatusID );
	BINDCOL_LONG( GetQuery(), m_LogoutStatusID );
	BINDCOL_TCHAR( GetQuery(), m_OnlineText );
	BINDCOL_TCHAR( GetQuery(), m_AwayText );
	BINDCOL_TCHAR( GetQuery(), m_NotAvailText );
	BINDCOL_TCHAR( GetQuery(), m_DndText );
	BINDCOL_TCHAR( GetQuery(), m_OfflineText );
	BINDCOL_TCHAR( GetQuery(), m_OooText );
	BINDCOL_LONG( GetQuery(), m_AutoStatusTypeID );
	BINDCOL_LONG( GetQuery(), m_AutoStatusMin );	
	BINDCOL_TCHAR( GetQuery(), szLogoutText );
	BINDCOL_TCHAR( GetQuery(), szAutoText );
	BINDCOL_LONG_NOLEN( GetQuery(), nEscalateAgentValid );	
	BINDCOL_LONG_NOLEN( GetQuery(), nApproverCount );
	BINDCOL_LONG_NOLEN( GetQuery(), nApproverGroupCount );
#if defined(HOSTED_VERSION)
	BINDCOL_TIME( GetQuery(), m_DateSet );
	GetQuery().Execute( _T("SELECT TOP 1 AgentID,IsNTUser,Password,Name,NTDomain,StyleSheetID,")
						_T("UseIM,NewMessageFormat,EscalateToAgentID,QuoteMsgInReply,ReplyQuotedPrefix,")
						_T("MaxReportRowsPerPage,RequireGetOldest,HasLoggedIn, ForceSpellCheck,")
						_T("SignatureTopReply,ShowMessagesInbound,UsePreviewPane,")
						_T("CloseTicket,RouteToInbox,UseMarkAsRead,MarkAsReadSeconds,OutboundApprovalFromID,UseAutoFill, ")
						_T("DefaultTicketBoxID,DefaultTicketDblClick,ReadReceipt,DeliveryConfirmation,LoginStatusID,LogoutStatusID, ")
						_T("OnlineText,AwayText,NotAvailText,DndText,OfflineText,OooText,AutoStatusTypeID,AutoStatusMin, ")
						_T("(SELECT CASE LogoutStatusID WHEN 5 THEN OfflineText WHEN 6 THEN OooText END), ")
						_T("(SELECT CASE AutoStatusTypeID WHEN 1 THEN OnlineText WHEN 2 THEN AwayText WHEN 3 THEN NotAvailText WHEN 4 THEN DndText WHEN 5 THEN OfflineText WHEN 6 THEN OooText END), ")
						_T("(SELECT COUNT(*) FROM Agents WHERE AgentID=A.EscalateToAgentID AND IsEnabled=1 AND IsDeleted=0), ")
						_T("(SELECT COUNT(*) FROM Agents WHERE A.AgentID=OutboundApprovalFromID AND IsEnabled=1 AND IsDeleted=0), ")
						_T("(SELECT COUNT(*) FROM Agents WHERE A.AgentID IN ")
						_T("(SELECT AgentGroupings.AgentID FROM AgentGroupings WHERE AgentGroupings.GroupID IN (SELECT ")
						_T("Abs(OutboundApprovalFromID) FROM Agents WHERE OutboundApprovalFromID < 0 ))AND AgentID=A.AgentID), ")
						_T("DateSet ")
						_T("FROM Agents AS A ")
						_T("WHERE LoginName=? AND IsEnabled = 1 AND IsDeleted = 0") );
#else
	GetQuery().Execute( _T("SELECT TOP 1 AgentID,IsNTUser,Password,Name,NTDomain,StyleSheetID,")
						_T("UseIM,NewMessageFormat,EscalateToAgentID,QuoteMsgInReply,ReplyQuotedPrefix,")
						_T("MaxReportRowsPerPage,RequireGetOldest,HasLoggedIn, ForceSpellCheck,")
						_T("SignatureTopReply,ShowMessagesInbound,UsePreviewPane,")
						_T("CloseTicket,RouteToInbox,UseMarkAsRead,MarkAsReadSeconds,OutboundApprovalFromID,UseAutoFill, ")
						_T("DefaultTicketBoxID,DefaultTicketDblClick,ReadReceipt,DeliveryConfirmation,LoginStatusID,LogoutStatusID, ")
						_T("OnlineText,AwayText,NotAvailText,DndText,OfflineText,OooText,AutoStatusTypeID,AutoStatusMin, ")
						_T("(SELECT CASE LogoutStatusID WHEN 5 THEN OfflineText WHEN 6 THEN OooText END), ")
						_T("(SELECT CASE AutoStatusTypeID WHEN 1 THEN OnlineText WHEN 2 THEN AwayText WHEN 3 THEN NotAvailText WHEN 4 THEN DndText WHEN 5 THEN OfflineText WHEN 6 THEN OooText END), ")
						_T("(SELECT COUNT(*) FROM Agents WHERE AgentID=A.EscalateToAgentID AND IsEnabled=1 AND IsDeleted=0), ")
						_T("(SELECT COUNT(*) FROM Agents WHERE A.AgentID=OutboundApprovalFromID AND IsEnabled=1 AND IsDeleted=0), ")
						_T("(SELECT COUNT(*) FROM Agents WHERE A.AgentID IN ")
						_T("(SELECT AgentGroupings.AgentID FROM AgentGroupings WHERE AgentGroupings.GroupID IN (SELECT ")
						_T("Abs(OutboundApprovalFromID) FROM Agents WHERE OutboundApprovalFromID < 0 ))AND AgentID=A.AgentID) ")
						_T("FROM Agents AS A ")
						_T("WHERE LoginName=? AND IsEnabled = 1 AND IsDeleted = 0") );
#endif

	// Fetch the data
	if( GetQuery().Fetch() == S_FALSE )
	{
		return Failed_NoSuchUser;
	}
	else
	{
		if ( nApproverCount > 0 || nApproverGroupCount > 0 )
			m_bIsOutboundApprover = true;
		else
			m_bIsOutboundApprover = false;
		
		if( nEscalateAgentValid == 0 )
		{
			if ( m_EscalateToAgentID < 0 )
			{
				int nGroupID;
				long nGroupIDLen = 4;
				int nEscalateToID = abs(m_EscalateToAgentID);
				
				GetQuery().Initialize();

				BINDPARAM_LONG( GetQuery(), nEscalateToID );

				BINDCOL_LONG( GetQuery(), nGroupID );
				
				GetQuery().Execute( _T("SELECT GroupID ")
										_T("FROM Groups ")
										_T("WHERE GroupID=? AND IsEscalationGroup = 1 AND IsDeleted = 0") );
				

				// Fetch the data
				if( GetQuery().Fetch() == S_FALSE )
				{
					m_EscalateToAgentID = 0;
				}
			}
			else
			{
				m_EscalateToAgentID = 0;
			}
		}

		// verify the password
		return AuthenticatePassword();
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// Authenticate - Compares the user's password and returns success or
//                failure in bAuthed
// 
////////////////////////////////////////////////////////////////////////////////
int CAuthenticate::AuthenticatePassword( void )
{
	int retval = Failed_BadPassword;
	BOOL bAuthed = FALSE;

#if defined(HOSTED_VERSION)
	if( m_IsNTUser == EMS_USER_TYPE_NT )
	{
		// Authenticate against an VMF Auth Server
		if( VMFServerAuth() )
		{
			bAuthed = TRUE;
		}
		else
		{
			DWORD dwErr = GetLastError();

			// ignore logon failures.
			if( dwErr < ERROR_LOGON_FAILURE || dwErr > ERROR_ACCOUNT_DISABLED )
			{
			}
		}
	}
	else if( m_IsNTUser == EMS_USER_TYPE_LOCAL )
	{
		// Password in database
		CEMSString sPassword;
		sPassword.assign( m_Password );
		sPassword.Decrypt();

		bAuthed = ( _tcscmp( m_szPasswordParam, sPassword.c_str() ) == 0 );
	}	
#else
	HANDLE hToken = INVALID_HANDLE_VALUE;

	if( m_IsNTUser == EMS_USER_TYPE_NT )
	{
		BOOL bThreadToken = FALSE;
		HANDLE hThreadToken1, hThreadToken2;

		// In order for the following bit 'o code to work,
		// The component must be running in-process. This means
		// Low (IIS Process) Application Protection in the Home Directory
		// tab of the web site settings.

		//Save the current thread token
		if( OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE, FALSE, &hThreadToken1) )
		{
		   RevertToSelf();
		   bThreadToken = TRUE;
		}

		// Authenticate against an NT domain controller
		if( LogonUser( m_szUsernameParam, m_NTDomain, m_szPasswordParam, 
			           LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &hToken ) )
		{
			bAuthed = TRUE;
		}
		else
		{
			DWORD dwErr = GetLastError();

			// ignore logon failures.
			if( dwErr < ERROR_LOGON_FAILURE || dwErr > ERROR_ACCOUNT_DISABLED )
			{
			}
		}

		//Restore the original thread token
		if( bThreadToken )
		{
		   hThreadToken2 = GetCurrentThread();
		   SetThreadToken( &hThreadToken2, hThreadToken1 );
		   CloseHandle( hThreadToken1 );
		}

		if( bAuthed )
		{
			CloseHandle( hToken );
		}
		
		// Just in case the LogonUser took an unusually long time
		// and we were asked to exit during the call...
		if( WaitForSingleObject( m_ISAPIData.m_hKillEvent, 0 ) == WAIT_OBJECT_0)
		{
			THROW_EMS_EXCEPTION( E_SystemStopping, CEMSString(EMS_STRING_SYSTEM_STOPPING) ); 
		}
	}
	else if( m_IsNTUser == EMS_USER_TYPE_LOCAL )
	{
		// Password in database
		CEMSString sPassword;
		sPassword.assign( m_Password );
		sPassword.Decrypt();

		bAuthed = ( _tcscmp( m_szPasswordParam, sPassword.c_str() ) == 0 );
	}
	else if( m_IsNTUser == EMS_USER_TYPE_VMS )
	{
		bAuthed = VMSAuth();
	}
#endif
	

	if( bAuthed )
	{
		// Authenticate the IP Address
		retval = AuthenticateIP();
		
		if( retval == Success )
		{
			SetSessionData();
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// VMSAuth
// 
////////////////////////////////////////////////////////////////////////////////
bool CAuthenticate::VMSAuth(void)
{
	CODBCConn dbconn;
	CODBCQuery query( dbconn );
	tstring sDSN;
	TCHAR szPassword[256];
	long szPasswordLen;
	bool bRet = false;

	GetServerParameter( EMS_SRVPARAM_VMS_DSN, sDSN );

	try
	{
		dbconn.Connect( sDSN.c_str() );

		query.Initialize();
		BINDPARAM_TCHAR( query, m_szUsernameParam );
		BINDCOL_TCHAR( query, szPassword );

		query.Execute( _T("SELECT U_Password FROM Users WHERE U_Mailbox=? AND U_AccountDisabled=0") );

		if( query.Fetch() == S_OK )
		{
			bRet = ( _tcscmp( m_szPasswordParam, szPassword ) == 0 );
		}
	}
	catch(ODBCError_t error) 
	{ 
		GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_ERROR,
											EMSERR_ISAPI_EXTENSION,
											EMS_LOG_AUTHENTICATION,
			                                E_FailedLogin),
								  "ODBC Error authenticating VMS user %s: %s", 
								  m_LoginName,
								  error.szErrMsg );
	}	
	catch( ... )
	{
		GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_ERROR,
											EMSERR_ISAPI_EXTENSION,
											EMS_LOG_AUTHENTICATION,
			                                E_FailedLogin),
								  "Unhandled exception authenticating VMS user %s",
								  m_LoginName );		
	}	

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////
// 
// VMFServerAuth
// 
////////////////////////////////////////////////////////////////////////////////
bool CAuthenticate::VMFServerAuth(void)
{
	bool bRet = false;
	CEMSString sPassword;			
	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );		
	
	if(m_DateSetLen != SQL_NULL_DATA)
	{
		int nSeconds = CompareTimeStamps(m_DateSet,now);
		if(nSeconds < 604800)
		{
			sPassword.assign( m_Password );
			sPassword.Decrypt();

			bRet = ( _tcscmp( m_szPasswordParam, sPassword.c_str() ) == 0 );
		}
	}	

	if(!bRet)
	{
		CkHttp http;
		CkGlobal m_glob;
		
		bRet = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
		if (bRet != true)
		{
			DebugReporter::Instance().DisplayMessage("CAuthenticate::VMFServerAuth() - Failed to unlock Global component", DebugReporter::MAIL);
			GetRoutingEngine().LogIt(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_ISAPI_EXTENSION,
										EMS_LOG_ISAPI_EXTENSION,
										0),
							EMS_STRING_ERROR_INITIALIZE_COMPONENT);

			return bRet;
		}		

		CkHttpRequest request;
		//  Add the form parameters to the HTTP request:
		request.AddParam("domain",m_NTDomain);
		request.AddParam("user",m_szUsernameParam);
		request.AddParam("pass",m_szPasswordParam);
	    
		CkHttpResponse *response = 0;

		try
		{
			tstring sUrl;
			GetServerParameter( EMS_SRVPARAM_VMS_WEBMAIL_URL, sUrl );
			response = http.PostUrlEncoded(sUrl.c_str(),request);
			if (response == 0 )
			{
				return bRet;
			}

			dca::String sResponse(response->bodyStr());
			dca::String::size_type pos = sResponse.find("authenticated");

			if(pos != dca::String::npos)
			{
				bRet = true;
				sPassword.assign( m_szPasswordParam );
				sPassword.Encrypt();
				strncpy( m_Password, sPassword.c_str(), AGENTS_PASSWORD_LENGTH-1 );
				GetQuery().Initialize();
				BINDPARAM_TCHAR( GetQuery(), m_Password );
				BINDPARAM_TIME_NOLEN( GetQuery(), now );
				BINDPARAM_LONG( GetQuery(), m_AgentID );
				GetQuery().Execute(_T("UPDATE Agents SET Password=?,DateSet=? WHERE AgentID=?"));
			}

			delete response;

		}
		catch( ... )
		{
				
		}	
	}

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////
// 
// AuthenticateIP
// 
////////////////////////////////////////////////////////////////////////////////
int CAuthenticate::AuthenticateIP( void )
{
	int retval = Failed_IPRange;
	BOOL bGotData = FALSE;
	dca::String sIP = GetBrowserSession().m_IP;
	
	int nIp6 = sIP.find_first_of(":");
	if ( nIp6 > -1 )
	{
		return retval = Success;
	}

	unsigned int nIP = InetAddr( GetBrowserSession().m_IP );

	GetQuery().Initialize();

	BINDPARAM_LONG( GetQuery(), m_AgentID );
	BINDPARAM_LONG( GetQuery(), m_AgentID );

	BINDCOL_LONG( GetQuery(), m_LowerRange ); 
	BINDCOL_LONG( GetQuery(), m_UpperRange ); 

	GetQuery().Execute( _T("SELECT LowerRange,UpperRange ")
						_T("FROM IPRanges LEFT OUTER JOIN AgentGroupings ON IPRanges.GroupID = AgentGroupings.GroupID ")
						_T("WHERE IPRanges.AgentID=? OR AgentGroupings.AgentID = ?") );

	while ( retval == Failed_IPRange ) 
	{
		// Fetch the data
		if( GetQuery().Fetch() == S_FALSE )
			break;

		bGotData = TRUE;

		if ( (m_LowerRange <= nIP) && (nIP <= m_UpperRange) )
		{
			retval = Success;
		}
	}

	// If no IP Ranges returned, then auth was successful
	if( bGotData == FALSE )
	{
		retval = Success;
	}

	if( retval == Failed_IPRange && m_AgentID == EMS_AGENTID_ADMINISTRATOR )
	{
		// Always allow the administrator to login from 127.0.0.1
		if( nIP == 0x7F000001 )
		{
			retval = Success;
		}
	}
	
	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetSessionData - Set the session data from the authentication query
// 
////////////////////////////////////////////////////////////////////////////////
void CAuthenticate::SetSessionData(void )
{
	bool  bIsAdmin = IsAgentAdmin( GetQuery(), m_AgentID );
	int m_nOutbound = 0;
	if ( m_OutboundApprovalFromID != 0 )
		m_nOutbound = 1;

	GetXMLGen().SetDoc(NULL);
	GetXMLGen().AddElem(_T("user"));
	GetXMLGen().AddChildElem(_T("id"), m_AgentID);
	GetXMLGen().AddChildElem(_T("name"), m_Name);
	GetXMLGen().AddChildElem(_T("useim"), m_UseIM);
	GetXMLGen().AddChildElem(_T("newmessageformat"), m_NewMessageFormat);
	GetXMLGen().AddChildElem(_T("useautofill"), m_UseAutoFill);
	GetXMLGen().AddChildElem(_T("outboundapproval"), m_nOutbound);
	GetXMLGen().AddChildElem(_T("outboundapprover"), m_bIsOutboundApprover);	
	AddStyleSheetFile(_T("stylesheetname"), m_StyleSheetID);
	GetXMLGen().AddChildElem(_T("isadmin"), bIsAdmin);
	GetXMLGen().AddChildElem(_T("escalatetoagentid"), m_EscalateToAgentID);	
	GetXMLGen().AddChildElem(_T("quotemsginreply"), m_QuoteMsgInReply);
	GetXMLGen().AddChildElem(_T("replyquotedprefix"), m_ReplyQuotedPrefix);
	GetXMLGen().AddChildElem(_T("forcespellcheck"), m_ForceSpellCheck);
	GetXMLGen().AddChildElem(_T("signaturetopreply"), m_SignatureTopReply);
	GetXMLGen().AddChildElem(_T("showmessagesinbound"), m_ShowMessagesInbound);
	GetXMLGen().AddChildElem(_T("usepreviewpane"), m_UsePreviewPane);
	GetXMLGen().AddChildElem(_T("closeticketaftersend"), m_CloseTicket);
	GetXMLGen().AddChildElem(_T("autoroutetoinbox"), m_RouteToInbox);
	GetXMLGen().AddChildElem(_T("usemarkasread"), m_UseMarkAsRead);
	GetXMLGen().AddChildElem(_T("markasreadseconds"), m_MarkAsReadSeconds);
	GetXMLGen().AddChildElem(_T("defaultticketboxid"), m_DefaultTicketBoxID);	
	GetXMLGen().AddChildElem(_T("defaultticketdblclick"), m_DefaultTicketDblClick);
	GetXMLGen().AddChildElem(_T("readreceipt"), m_ReadReceipt);	
	GetXMLGen().AddChildElem(_T("deliveryconfirmation"), m_DeliveryConfirmation);

	if( m_LoginStatusID < 0){m_LoginStatusID = 1;}

	switch( m_LoginStatusID )
	{
	case 1:
		_tcscpy( m_StatusText, m_OnlineText );		
		break;
		
	case 2:
		_tcscpy( m_StatusText, m_AwayText );
		break;

	case 3:
		_tcscpy( m_StatusText, m_NotAvailText);
		break;
		
	case 4:
		_tcscpy( m_StatusText, m_DndText);
		break;
		
	default:
		_tcscpy( m_StatusText, _T("Online") );
		
	}
	
	GetXMLGen().AddChildElem(_T("statusid"), m_LoginStatusID);
	GetXMLGen().AddChildElem(_T("statustext"), m_StatusText);
	GetXMLGen().AddChildElem(_T("onlinetext"),m_OnlineText);
	GetXMLGen().AddChildElem(_T("awaytext"), m_AwayText);
	GetXMLGen().AddChildElem(_T("notavailtext"), m_NotAvailText);
	GetXMLGen().AddChildElem(_T("dndtext"), m_DndText);
	GetXMLGen().AddChildElem(_T("offlinetext"), m_OfflineText);
	GetXMLGen().AddChildElem(_T("oootext"), m_OooText);
	GetXMLGen().AddChildElem(_T("autostatustypeid"), m_AutoStatusTypeID );
	GetXMLGen().AddChildElem(_T("autostatusmin"), m_AutoStatusMin );
	GetXMLGen().AddChildElem(_T("logoutstatusid"), m_LogoutStatusID );
	GetXMLGen().AddChildElem(_T("logouttext"), szLogoutText );
	GetXMLGen().AddChildElem(_T("autotext"), szAutoText );

	GetXMLGen().AddChildElem(_T("access"));
	GetXMLGen().IntoElem();

#if defined(DEMO_VERSION)
	GetXMLGen().AddChildElem( _T("contact"), CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID, EMS_OBJECT_TYPE_CONTACT, 0) );
	GetXMLGen().AddChildElem( _T("stdResp"), CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_STD_RESP, 0) );
#else
	GetXMLGen().AddChildElem( _T("contact"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID, EMS_OBJECT_TYPE_CONTACT, 0) );

	GetXMLGen().AddChildElem( _T("stdResp"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_STD_RESP, 0) );

	GetXMLGen().AddChildElem( _T("stdreports"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_STANDARD_REPORTS, 0) );

	GetXMLGen().AddChildElem( _T("schedreports"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_SCHEDULED_REPORTS, 0) );

	GetXMLGen().AddChildElem( _T("autoopen"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_AUTO_TICKET_OPEN, 0) );

	GetXMLGen().AddChildElem( _T("agentaudit"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_AGENT_AUDIT, 0) );

	GetXMLGen().AddChildElem( _T("voip"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_VOIP, 0) );

	GetXMLGen().AddChildElem( _T("pref"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_AGENT_PREFERENCES, 0) );

	GetXMLGen().AddChildElem( _T("prefgen"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_GENERAL, 0) );

	GetXMLGen().AddChildElem( _T("preftb"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_TICKETBOX, 0) );

	GetXMLGen().AddChildElem( _T("prefav"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_AGENT_VIEWS, 0) );

	GetXMLGen().AddChildElem( _T("prefsig"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_SIGNATURES, 0) );

	GetXMLGen().AddChildElem( _T("prefmydict"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_MY_DICTIONARY, 0) );

	GetXMLGen().AddChildElem( _T("prefmyfold"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_MY_FOLDERS, 0) );

	GetXMLGen().AddChildElem( _T("prefmyal"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_MY_ALERTS, 0) );

	GetXMLGen().AddChildElem( _T("prefmycont"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_PREFERENCES_MY_CONTACTS, 0) );

	GetXMLGen().AddChildElem( _T("rrtm"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_ROUTE_REPLIES, 0) );

	GetXMLGen().AddChildElem( _T("ctas"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_CLOSE_TICKET, 0) );

	GetXMLGen().AddChildElem( _T("rrr"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_REQUEST_READ, 0) );

	GetXMLGen().AddChildElem( _T("rdc"), bIsAdmin ? EMS_DELETE_ACCESS : 
	CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID,EMS_OBJECT_TYPE_REQUEST_DELIVERY, 0) );	

#endif

	GetXMLGen().OutOfElem();

	GetSession().m_AgentID = m_AgentID;
	GetSession().m_AgentXML = GetXMLGen().GetDoc();
	GetSession().m_IsAdmin = bIsAdmin;
	GetSession().m_UseAutoFill = (m_UseAutoFill!=0);
	GetSession().m_bIsOutboundApprover =  m_bIsOutboundApprover;
	GetSession().m_AgentUserName = m_Name;
	GetSession().m_EscalateToAgentID = m_EscalateToAgentID;
	GetSession().m_RequireGetOldest = m_RequireGetOldest;
	GetSession().m_nMaxRowsPerPage = (m_MaxReportRowsPerPage > 0) ? m_MaxReportRowsPerPage : EMS_DEFAULT_PAGE_ROWS;
	GetSession().m_AgentIP = GetBrowserSession().m_IP;
	GetSession().m_AgentStatusTypeID = m_LoginStatusID;
	GetSession().m_LogoutStatusID = m_LogoutStatusID;
	GetSession().m_AgentStatusText = m_StatusText;
	GetSession().m_AgentAutoStatusTypeID = m_AutoStatusTypeID;
	GetSession().m_AgentAutoStatusMin = m_AutoStatusMin;
	GetSession().m_LogoutText = szLogoutText;
	GetSession().m_AutoText = szAutoText;

	GetBrowserSession().m_AgentID = m_AgentID;

	if(m_LoginStatusID < 0)
	{
		DebugReporter::Instance().DisplayMessage("CAuthenticate::SetSessionData - m_LoginStatusID < 0", DebugReporter::ISAPI, GetCurrentThreadId());	
	}
	//Update the Agents table
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_LoginStatusID );
	BINDPARAM_TCHAR( GetQuery(), m_StatusText );
	BINDPARAM_LONG( GetQuery(), m_AgentID );	
	GetQuery().Execute(_T("UPDATE Agents SET StatusID=?,StatusText=? WHERE AgentID=?"));

}