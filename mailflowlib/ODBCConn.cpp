/***************************************************************************\
||
||  $Header: /root/MAILSTREAMLIBRARY/ODBCConn.cpp,v 1.2 2005/11/29 21:30:04 markm Exp $
||
||
||  COMMENTS:   ODBC Connection Wrapper
||
\\*************************************************************************/

#include "stdafx.h"
#include "ODBCConn.h"
#include "ODBCQuery.h"
#include "RegistryFns.h"
#include "EMSString.h"
#include "DebugReporter.h"

////////////////////////////////////////////////////////////////////////////////
// 
//  CODBCConn constructor
// 
////////////////////////////////////////////////////////////////////////////////
CODBCConn::CODBCConn()
{
	// Initialization
	m_henv = SQL_NULL_HENV;
	m_hdbc = SQL_NULL_HDBC;
	m_iDBMSType = Unknown;
	m_hThreadToken1 = INVALID_HANDLE_VALUE;
	m_hThreadToken2 = INVALID_HANDLE_VALUE;

}

////////////////////////////////////////////////////////////////////////////////
// 
//  CODBCConn destructor
// 
////////////////////////////////////////////////////////////////////////////////
CODBCConn::~CODBCConn()
{
	if (IsConnected())
		Disconnect();
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetDSN - Get the DSN connection string from the registry
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCConn::GetDSN(CEMSString& tstrDSN)
{
	tstring tstrDBType;

	if (GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		EMS_DATABASE_DSN_VALUE, tstrDSN) != ERROR_SUCCESS)
	{
		ODBCError_t error;
		error.nErrorCode = E_DSNNotInRegistry;

		CEMSString sErrorMsg(EMS_STRING_ERROR_NODSN);
		_tcsncpy((TCHAR*)error.szErrMsg, sErrorMsg.c_str(), 511);

		throw error;
	}

	tstrDSN.Decrypt();

	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		EMS_DATABASE_TYPE_VALUE, tstrDBType);

	if (_tcsicmp(tstrDBType.c_str(), DBTYPE_MSSQL) == 0)
	{
		m_iDBMSType = MSSQL;
	}

	// unknown database type, throw an exception
	// MJM 02/08/2000 - We may wish to change this behavior at 
	// some point in the future.
	if (m_iDBMSType == Unknown)
	{
		ODBCError_t error;
		error.nErrorCode = E_UknownDatabaseType;

		CEMSString sErrorMsg(EMS_STRING_ERROR_UKNOWN_DBTYPE);
		_tcsncpy((TCHAR*)error.szErrMsg, sErrorMsg.c_str(), 511);

		throw error;
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// Connect
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCConn::Connect(LPCTSTR szDSN, LPCTSTR szNTUser,
	LPCTSTR szNTPassword, LPCTSTR szNTDomain)
{
	SQLRETURN ret;
	TCHAR szConnectOutput[1024] = { 0 };
	short nResult = 0;
	CEMSString tstrDSN;
	unsigned int nUseWIA = 0;
	CEMSString sUsername;
	CEMSString sDomain;
	CEMSString sPassword;

	// use function parameters if szDSN is not NULL

	if (szDSN)
	{
		tstrDSN = szDSN;

		if (szNTUser)
		{
			nUseWIA = 1;
			sUsername = szNTUser;
			sDomain = szNTDomain;
			sPassword = szNTPassword;
		}
	}
	else // otherwise, get parameters from registry
	{
		// Get DSN from the registry
		GetDSN(tstrDSN);

		// Get nUseWIA from registry	
		if (GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("DbUseWIA"), nUseWIA) != ERROR_SUCCESS)
		{
			// Missing new registry value, so encrypt current DbLogin and create new value
			GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_DATABASE_DSN_VALUE, tstrDSN);
			tstrDSN.Encrypt();
			WriteRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_DATABASE_DSN_VALUE, tstrDSN.c_str());
			tstrDSN.Decrypt();

			nUseWIA = 0;
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("DbUseWIA"), nUseWIA);
		}

		if (nUseWIA == 1)
		{
			GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("RunAsUser"), sUsername);
			GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("RunAsDomain"), sDomain);
			GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("RunAsPassword"), sPassword);
			sUsername.Decrypt();
			sDomain.Decrypt();
			sPassword.Decrypt();
		}
	}

	if (nUseWIA == 1)
	{
		if (ImpersonateUser(sUsername.c_str(), sPassword.c_str(), sDomain.c_str()) == FALSE)
		{
			/*ODBCError_t error;
			error.nErrorCode = 999;
			error.nNativeErrorCode = 0;
			error.szState[0] = _T('\0');
			_tcscpy( (TCHAR*) error.szErrMsg, _T("NT Logon failed.") );
			throw error;*/
			DebugReporter::Instance().DisplayMessage("CODBCConn::Connect - NT Logon Impersonation failed.", DebugReporter::LIB);
		}
	}

	// Allocate an environment handle
	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv);

	if (!SQL_SUCCEEDED(ret))
	{
		DebugReporter::Instance().DisplayMessage("CODBCConn::Connect - SQLAllocHandle failed.", DebugReporter::LIB);
		ODBCError_t error;
		GetConnDiagnostics(error);
		error.nErrorCode = E_AllocHandle;
		throw error;
	}

	// Set our ODBC version environment variable
	ret = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION,
		(SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);

	if (!SQL_SUCCEEDED(ret))
	{
		DebugReporter::Instance().DisplayMessage("CODBCConn::Connect - SQLSetEnvAttr failed.", DebugReporter::LIB);
		ODBCError_t error;
		GetConnDiagnostics(error);
		error.nErrorCode = E_SetEnvAttrError;

		// free the environment handle
		SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
		m_henv = SQL_NULL_HENV;

		throw error;
	}

	// Allocate a connection handle
	ret = SQLAllocHandle(SQL_HANDLE_DBC, m_henv, &m_hdbc);

	if (!SQL_SUCCEEDED(ret))
	{
		DebugReporter::Instance().DisplayMessage("CODBCConn::Connect - SQLAllocHandle failed.", DebugReporter::LIB);
		ODBCError_t error;
		GetConnDiagnostics(error);
		error.nErrorCode = E_AllocHandle;

		// free the environment handle
		SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
		m_henv = SQL_NULL_HENV;

		throw error;
	}

	// Set read-only connection option
	// Deprecated Function [SQLSetConnectOption(m_hdbc, SQL_ACCESS_MODE, SQL_MODE_READ_ONLY)]
	ret = SQLSetConnectAttr(m_hdbc, SQL_ACCESS_MODE, (SQLPOINTER)SQL_MODE_READ_ONLY, 0);

		if (!SQL_SUCCEEDED(ret))
		{
			DebugReporter::Instance().DisplayMessage("CODBCConn::Connect - SQLSetConnectOption failed.", DebugReporter::LIB);
			ODBCError_t error;
			GetConnDiagnostics(error);
			error.nErrorCode = E_SetConnOptionError;

			// free the connection handle
			SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);
			m_hdbc = SQL_NULL_HDBC;

			// free the environment handle
			SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
			m_henv = SQL_NULL_HENV;

			throw error;
		}

	// Set login timeout to 5 seconds
	ret = SQLSetConnectAttr(m_hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
	if (!SQL_SUCCEEDED(ret))
	{
		DebugReporter::Instance().DisplayMessage("CODBCConn::Connect - SQLSetConnectAttr timeout failed.", DebugReporter::LIB);
	}

	// Connect to DSN
	ret = SQLDriverConnect(m_hdbc, NULL, (SQLTCHAR*)tstrDSN.c_str(), static_cast<SQLSMALLINT>(tstrDSN.length()),
		                    (SQLTCHAR*)szConnectOutput, sizeof(szConnectOutput) / sizeof(TCHAR), &nResult, SQL_DRIVER_NOPROMPT);

	if (!SQL_SUCCEEDED(ret))
	{
		DebugReporter::Instance().DisplayMessage("CODBCConn::Connect - SQLDriverConnect failed, wait 1 second and retry.", DebugReporter::LIB);
		Sleep(1000);

		ret = SQLDriverConnect(m_hdbc, NULL, (SQLTCHAR*)tstrDSN.c_str(), static_cast<SQLSMALLINT>(tstrDSN.length()),
			                    (SQLTCHAR*)szConnectOutput, sizeof(szConnectOutput) / sizeof(TCHAR), &nResult, SQL_DRIVER_NOPROMPT);

		if (!SQL_SUCCEEDED(ret))
		{
			DebugReporter::Instance().DisplayMessage("CODBCConn::Connect - SQLDriverConnect failed, wait 1 second and retry.", DebugReporter::LIB);
			Sleep(1000);

			ret = SQLDriverConnect(m_hdbc, NULL, (SQLTCHAR*)tstrDSN.c_str(), static_cast<SQLSMALLINT>(tstrDSN.length()),
				                    (SQLTCHAR*)szConnectOutput, sizeof(szConnectOutput) / sizeof(TCHAR), &nResult, SQL_DRIVER_NOPROMPT);

			if (!SQL_SUCCEEDED(ret))
			{

				DebugReporter::Instance().DisplayMessage("CODBCConn::Connect - SQLDriverConnect failed, wait 1 second and retry.", DebugReporter::LIB);
				Sleep(1000);

				ret = SQLDriverConnect(m_hdbc, NULL, (SQLTCHAR*)tstrDSN.c_str(), static_cast<SQLSMALLINT>(tstrDSN.length()),
					                    (SQLTCHAR*)szConnectOutput, sizeof(szConnectOutput) / sizeof(TCHAR), &nResult, SQL_DRIVER_NOPROMPT);

				if (!SQL_SUCCEEDED(ret))
				{

					DebugReporter::Instance().DisplayMessage("CODBCConn::Connect - SQLDriverConnect failed.", DebugReporter::LIB);
					ODBCError_t error;
					GetConnDiagnostics(error);
					error.nErrorCode = E_DatabaseConnection;

					// free the connection handle
					SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);
					m_hdbc = SQL_NULL_HDBC;

					// free the environment handle
					SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
					m_henv = SQL_NULL_HENV;

					throw error;

				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Disconnect
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCConn::Disconnect(void)
{
	if (m_hdbc != SQL_NULL_HDBC)
	{
		SQLDisconnect(m_hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);
		m_hdbc = SQL_NULL_HDBC;
	}

	if (m_henv != SQL_NULL_HENV)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
		m_henv = SQL_NULL_HENV;
	}

	if (m_hThreadToken2 != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hThreadToken2);
		m_hThreadToken2 = INVALID_HANDLE_VALUE;
	}

	if (m_hThreadToken1 != INVALID_HANDLE_VALUE)
	{
		m_hThreadToken2 = GetCurrentThread();
		SetThreadToken(&m_hThreadToken2, m_hThreadToken1);
		CloseHandle(m_hThreadToken1);
		m_hThreadToken1 = INVALID_HANDLE_VALUE;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetConnDiagnostics
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCConn::GetConnDiagnostics(ODBCError_t& error)
{
	short nErrMsgLength;

	SQLGetDiagRec(SQL_HANDLE_DBC, m_hdbc, 1, error.szState, &error.nNativeErrorCode,
		error.szErrMsg, SQL_MAX_MESSAGE_LENGTH, &nErrMsgLength);
}

////////////////////////////////////////////////////////////////////////////////
// 
// ImpersonateUser
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CODBCConn::ImpersonateUser(LPCTSTR szNTUser, LPCTSTR szNTPassword, LPCTSTR szNTDomain)
{
	BOOL bRet = TRUE;

	if (OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE, FALSE, &m_hThreadToken1))
	{
		RevertToSelf();
	}

	bRet = LogonUser((TCHAR*)szNTUser, (TCHAR*)szNTDomain, (TCHAR*)szNTPassword,
		LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &m_hThreadToken2);
	if (bRet)
	{
		bRet = ImpersonateLoggedOnUser(m_hThreadToken2);
	}

	return bRet;
}
