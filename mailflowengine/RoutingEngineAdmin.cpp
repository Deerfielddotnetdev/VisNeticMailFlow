// RoutingEngineAdmin.cpp : Implementation of CRoutingEngineAdmin
#include "stdafx.h"
#include "MailStreamEngine.h"
#include "RoutingEngineAdmin.h"

/////////////////////////////////////////////////////////////////////////////
// CRoutingEngineAdmin


////////////////////////////////////////////////////////////////////////////////
// 
// Authenticate
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineAdmin::Authenticate( BSTR bstrLogonName, BSTR bstrPassword, long* pResult )
{
	TCHAR Password[AGENTS_PASSWORD_LENGTH];
	long PasswordLen;
	TCHAR NTDomain[AGENTS_NTDOMAIN_LENGTH];
	long NTDomainLen;
	int IsNTUser;
	long IsNTUserLen;
	int AdminGroup = EMS_GROUPID_ADMINISTRATORS;

	try
	{
		*pResult = FALSE;	// failed

		ZeroMemory( Password, sizeof(Password) );

		m_db.Connect();

		m_query.Initialize();
		BINDPARAM_WCHAR( m_query, bstrLogonName );
		BINDPARAM_LONG( m_query, AdminGroup );
		BINDCOL_TCHAR( m_query, Password );
		BINDCOL_LONG( m_query, IsNTUser ); 
		BINDCOL_TCHAR( m_query, NTDomain );

		m_query.Execute( _T("SELECT Password,IsNTUser,NTDomain ")
						 _T("FROM Agents INNER JOIN AgentGroupings ON Agents.AgentID = AgentGroupings.AgentID ")
						 _T("WHERE LoginName=? AND GroupID=? ")
			             _T("AND isDeleted=0 AND IsEnabled=1") );

		if( m_query.Fetch() == S_OK )
		{
			switch( IsNTUser )
			{
			case EMS_USER_TYPE_NT:
				*pResult = NTUserAuth( bstrLogonName, bstrPassword, NTDomain );
				break;

			case EMS_USER_TYPE_LOCAL:
				*pResult = LocalUserAuth( bstrPassword, Password );
				break;

			case EMS_USER_TYPE_VMS:
				*pResult = VMSUserAuth( bstrLogonName, bstrPassword );
				break;

			default:
				break;
			}
		}
		else
		{
		}

		m_query.Close();
		m_db.Disconnect();
	}
	catch( ODBCError_t e )
	{
		*pResult = 2;
	}			

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 
// NTUserAuth
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngineAdmin::NTUserAuth( BSTR LogonName, BSTR Password, LPTSTR NTDomain )
{
	HANDLE hToken = INVALID_HANDLE_VALUE;
	BOOL bAuthed = FALSE;
	BOOL bThreadToken = FALSE;

	// Authenticate against an NT domain controller
	if( LogonUser( LogonName, NTDomain, Password, 
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

	if( bAuthed )
	{
		CloseHandle( hToken );
	}

	return bAuthed;
}

////////////////////////////////////////////////////////////////////////////////
// 
// LocalUserAuth
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngineAdmin::LocalUserAuth( BSTR Password, LPTSTR szDBPassword )
{
	CEMSString sPassword;
	sPassword.assign( szDBPassword );
	sPassword.Decrypt();

	return ( wcscmp( Password, sPassword.c_str() ) == 0 ) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// 
// VMSUserAuth
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CRoutingEngineAdmin::VMSUserAuth( BSTR LogonName, BSTR Password )
{
	CODBCConn dbconn;
	CODBCQuery query( dbconn );
	tstring sDSN;
	TCHAR szVMSDSN[256];
	WCHAR szVMSPassword[256];
	long szVMSPasswordLen;
	int nSrvParamID = EMS_SRVPARAM_VMS_DSN;

	m_query.Initialize();
	BINDPARAM_LONG( m_query, nSrvParamID );
	BINDCOL_TCHAR_NOLEN( m_query, szVMSDSN );
	m_query.Execute( _T("SELECT DataValue FROM ServerParameters ")
		             _T("WHERE ServerParameterID=? " ) );

	if( m_query.Fetch() != S_OK )
	{
		return FALSE;
	}

	try
	{
		dbconn.Connect( szVMSDSN );

		query.Initialize();
		BINDPARAM_WCHAR( query, LogonName );
		BINDCOL_WCHAR( query, szVMSPassword );

		query.Execute( _T("SELECT U_Password FROM Users WHERE U_Mailbox=?") );

		if( query.Fetch() == S_OK )
		{
			if( wcscmp( szVMSPassword, Password ) == 0 )
			{
				return TRUE;
			}
		}
	}
	catch(ODBCError_t) 
	{ 

	}	
	catch( ... )
	{

	}	

	return FALSE;
}



////////////////////////////////////////////////////////////////////////////////
// 
// TestODBC
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineAdmin::TestODBC( BSTR Server, long UseWIA, BSTR LogonName, BSTR Password,
											BSTR Domain, BSTR Database, BSTR* pErrMsg, long* pResult )
{
	CEMSString sDSN;
	CODBCConn db;
	CODBCQuery query(db);
	long nCount;
	dca::String e;

	dca::String svr(Server);
	dca::String lname(LogonName);
	dca::String pwd(Password);
	dca::String dm(Domain);
	dca::String dbase(Database);
	tstring sDriver;
	UINT nSecure;
	UINT nTrust;

	try
	{
		// Initialize output parameters
		*pResult = FALSE;
		*pErrMsg = NULL;

		GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("OdbcDriver"), sDriver );
		GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("OdbcSecure"), nSecure );
		GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("OdbcTrust"), nTrust );

		if( UseWIA )
		{
			if(nSecure && nTrust)
			{
				sDSN.Format( _T("DRIVER={%s};SERVER=%s;DATABASE=%s;Trusted_Connection=yes;Encrypt=yes;TrustServerCertificate=yes;"), 
				         sDriver.c_str(),Server, Database );
			}
			else if(nSecure)
			{
				sDSN.Format( _T("DRIVER={%s};SERVER=%s;DATABASE=%s;Trusted_Connection=yes;Encrypt=yes;"), 
				         sDriver.c_str(),Server, Database );
			}
			else
			{
				sDSN.Format( _T("DRIVER={%s};SERVER=%s;DATABASE=%s;Trusted_Connection=yes;"), 
				         sDriver.c_str(),Server, Database );
			}
			
			dca::String dsn(sDSN.c_str());
			e.Format("CRoutingEngineAdmin::TestODBC - DSN:%s LogonName:%s Password:%s Domain:%s", dsn.c_str(),lname.c_str(),pwd.c_str(),dm.c_str());
			DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);			
			
			db.Connect( sDSN.c_str(), LogonName, Password, Domain );			
		}
		else
		{
			if(nSecure && nTrust)
			{
				sDSN.Format( _T("DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;Encrypt=yes;TrustServerCertificate=yes;"), 
						 sDriver.c_str(),Server, LogonName, Password, Database );
			}
			else if(nSecure)
			{
				sDSN.Format( _T("DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;Encrypt=yes;"), 
						 sDriver.c_str(),Server, LogonName, Password, Database );
			}
			else
			{
				sDSN.Format( _T("DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;"), 
						 sDriver.c_str(),Server, LogonName, Password, Database );
			}

			dca::String dsn(sDSN.c_str());
			e.Format("CRoutingEngineAdmin::TestODBC - DSN:%s", dsn.c_str());
			DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

			db.Connect( sDSN.c_str() );
		}

		// Verify a few tables
		query.Initialize();
		BINDCOL_LONG_NOLEN( query, nCount );
		query.Execute( _T("SELECT COUNT(*) FROM ServerParameters") );
		query.Fetch();

		query.Reset( false );
		query.Execute( _T("SELECT COUNT(*) FROM MessageSources") );
		query.Fetch();

		query.Reset( false );
		query.Execute( _T("SELECT COUNT(*) FROM MessageDestinations") );
		query.Fetch();

		query.Close();
		db.Disconnect();

		DebugReporter::Instance().DisplayMessage("CRoutingEngineAdmin::TestODBC - Successful connection to database", DebugReporter::ENGINE);
		*pResult = TRUE;
	}
	catch( ODBCError_t err )
	{
		*pErrMsg = SysAllocString( err.szErrMsg );
		dca::String x(err.szErrMsg);
		e.Format("CRoutingEngineAdmin::TestODBC - ODBC Error: %s", x.c_str());
		DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);		
	}
	catch( ... )
	{
		DebugReporter::Instance().DisplayMessage("CRoutingEngineAdmin::TestODBC - Caught exception", DebugReporter::ENGINE);
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetForceHTTPS
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineAdmin::GetServerParameter( long nID, BSTR* bstrValue, long* pResult )
{
	WCHAR szValue[SERVERPARAMETERS_DATAVALUE_LENGTH];
	long szValueLen;

	try
	{
		*pResult = 1;
		*bstrValue = NULL;
		
		m_db.Connect();

		m_query.Initialize();
		BINDPARAM_LONG( m_query, nID );
		BINDCOL_WCHAR( m_query, szValue );
		
		m_query.Execute( _T("SELECT DataValue ")
						 _T("FROM ServerParameters ")
						 _T("WHERE ServerParameterID=?") );

		if( m_query.Fetch() == S_OK )
		{
			*pResult = 0;
			*bstrValue = SysAllocString( szValue );
		}

		m_query.Close();
		m_db.Disconnect();	
	}
	catch( ODBCError_t e )
	{
		*pResult = 2;
	}		
	catch( ... )
	{
		*pResult = 3;
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 
// SetForceHTTPS
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineAdmin::SetServerParameter( long nID, BSTR bstrValue, long* pResult )
{
	try
	{
		*pResult = 1;

		m_db.Connect();

		m_query.Initialize();
		BINDPARAM_WCHAR( m_query, bstrValue );
		BINDPARAM_LONG( m_query, nID );
		
		m_query.Execute( _T("UPDATE ServerParameters ")
						 _T("SET DataValue=? ")
						 _T("WHERE ServerParameterID=?") );

		if( m_query.GetRowCount() == 1 )
		{
			*pResult = 0;
		}

		m_query.Close();
		m_db.Disconnect();	
	}
	catch( ODBCError_t e )
	{
		*pResult = 2;
	}		
	catch( ... )
	{
		*pResult = 3;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetPassword
// 
////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CRoutingEngineAdmin::SetPassword( BSTR User, BSTR Password, long* pResult )
{
	try
	{
		*pResult = 1;

		m_db.Connect();

		if( wcscmp( L"Administrator", User ) == 0 )
		{
			CEMSString sPwd = Password;

			sPwd.Encrypt();
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sPwd );		
			m_query.Execute( _T("UPDATE Agents ")
							 _T("SET LoginName='Admin',Password=?,IsNTUser=0 ")
							 _T("WHERE AgentID=1") );

			if( m_query.GetRowCount() == 1 )
			{
				*pResult = 0;
			}
		}
		else if( wcscmp( L"VMFAdmin", User ) == 0 )
		{
			m_query.Initialize();
			BINDPARAM_WCHAR( m_query, Password );		
			m_query.Execute( _T("EXEC sp_password @new=?,@loginame='VMFAdmin'") );
			*pResult = 0;
		}
		else if( wcsstr( L"1:", User ) == 0 )
		{
			
			//Encrypt the Agent Password
			CEMSString sPwd = Password;
			sPwd.Encrypt();

			//Get the LoginName from the User string
			tstring sTemp(User);
			sTemp = sTemp.substr(2,sTemp.length());
			sTemp.resize(sTemp.length());

			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sPwd );
			BINDPARAM_TCHAR_STRING( m_query, sTemp );
			m_query.Execute( _T("UPDATE Agents ")
				_T("SET Password=? WHERE LoginName=?") );

			if( m_query.GetRowCount() == 1 )
			{
				*pResult = 0;
			}
		}
		else if( wcsstr( L"2:", User ) == 0 )
		{
			//Encrypt the MessageSource Password
			CEMSString sPwd = Password;
			sPwd.Encrypt();

			//Get the AuthUserName from the User string
			tstring sTemp(User);
			sTemp = sTemp.substr(2,sTemp.length());
			sTemp.resize(sTemp.length());

			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sPwd );
			BINDPARAM_TCHAR_STRING( m_query, sTemp );
			m_query.Execute( _T("UPDATE MessageSources ")
				_T("SET AuthPassword=? WHERE AuthUserName=?") );

			if( m_query.GetRowCount() == 1 )
			{
				*pResult = 0;
			}
		}
		else if( wcsstr( L"3:", User ) == 0 )
		{
			//Encrypt the MessageDestination Password
			CEMSString sPwd = Password;
			sPwd.Encrypt();

			//Get the AuthUser from the User string
			tstring sTemp(User);
			sTemp = sTemp.substr(2,sTemp.length());
			sTemp.resize(sTemp.length());

			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sPwd );
			BINDPARAM_TCHAR_STRING( m_query, sTemp );
			m_query.Execute( _T("UPDATE MessageDestinations ")
				_T("SET AuthPass=? WHERE AuthUser=?") );

			if( m_query.GetRowCount() == 1 )
			{
				*pResult = 0;
			}
		}

		m_query.Close();
		m_db.Disconnect();	
	}
	catch( ODBCError_t e )
	{
		*pResult = 2;
	}		
	catch( ... )
	{
		*pResult = 3;
	}

	return S_OK;
}
