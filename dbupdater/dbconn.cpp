////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MFDBUPDATE/dbconn.cpp,v 1.2 2005/09/22 12:43:06 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// DBConn.cpp: implementation of the CDBConn class.
//
//////////////////////////////////////////////////////////////////////

#include "prehead.h"
#include "DBConn.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// 
//  CDBConn constructor
// 
////////////////////////////////////////////////////////////////////////////////
CDBConn::CDBConn()
{
	// Initialization
	m_henv = SQL_NULL_HENV;
	m_hdbc = SQL_NULL_HDBC;
	memset( m_szState, 0, SQL_SQLSTATE_SIZE+1 );
	memset( m_szErrMsg, 0, SQL_MAX_MESSAGE_LENGTH+1 );
	m_nNativeErr = 0;
	m_nErrMsgLength = 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
//  CDBConn destructor
// 
////////////////////////////////////////////////////////////////////////////////
CDBConn::~CDBConn()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
//  Connect Method
// 
////////////////////////////////////////////////////////////////////////////////
SQLRETURN CDBConn::Connect(TCHAR *szConnect)
{
	SQLRETURN ret;
	TCHAR szConnectInput[1024] = {'\0'};
	TCHAR szConnectOutput[1024];
	short nResult = 0;


	// Allocate an environment handle
	ret = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv );

	if ( !SQL_SUCCEEDED( ret ) )
	{
		GetDiagnostics();
		return ret;
	}

	// Set our ODBC version environment variable
	ret = SQLSetEnvAttr( m_henv, SQL_ATTR_ODBC_VERSION, 
		                 (SQLPOINTER) SQL_OV_ODBC2, SQL_IS_INTEGER );

	if ( !SQL_SUCCEEDED( ret ) )
	{
		GetDiagnostics();
		return ret;
	}

	// Allocate a connection handle
	ret = SQLAllocHandle( SQL_HANDLE_DBC, m_henv, &m_hdbc );

	if ( !SQL_SUCCEEDED( ret ) )
	{
		GetDiagnostics();
		return ret;
	}

	// Set read-only connection option
	ret = SQLSetConnectOption( m_hdbc, SQL_ACCESS_MODE, SQL_MODE_READ_ONLY);

	if ( !SQL_SUCCEEDED( ret ) )
	{
		GetDiagnostics();
		return ret;
	}

	lstrcpy((TCHAR*)szConnectInput,szConnect);

	//WideCharToMultiByte( CP_ACP, 0, szConnect, wcslen( szConnect ), 
		                 //szConnectInput, sizeof(szConnectInput), NULL, NULL );

	

	// Connect to DSN
	ret = SQLDriverConnect( m_hdbc, NULL, 
		                   (_TUCHAR*)szConnectInput, SQL_NTS, 
						   (_TUCHAR*)szConnectOutput, sizeof(szConnectOutput), 
						   &nResult, SQL_DRIVER_NOPROMPT );

	if ( !SQL_SUCCEEDED( ret ) ) 
	{
		GetDiagnostics();
		return ret;
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// 
//  Disconnect Method
// 
////////////////////////////////////////////////////////////////////////////////
SQLRETURN CDBConn::Disconnect()
{
	SQLRETURN ret = SQL_SUCCESS;

	if( m_hdbc != SQL_NULL_HDBC )
	{
		ret = SQLDisconnect( m_hdbc );
		ret = SQLFreeHandle( SQL_HANDLE_DBC, m_hdbc );
		m_hdbc = SQL_NULL_HDBC;
	}

	if( m_henv != SQL_NULL_HENV )
	{
		ret = SQLFreeHandle( SQL_HANDLE_ENV, m_henv );
		m_henv = SQL_NULL_HENV;
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// 
//  ExecuteSQL
// 
////////////////////////////////////////////////////////////////////////////////
SQLRETURN CDBConn::ExecuteSQL( TCHAR* szSQL )
{
	SQLHSTMT	hstmt;
	SQLRETURN	ret;

	ret = SQLAllocHandle( SQL_HANDLE_STMT, m_hdbc, &hstmt );

	if( !SQL_SUCCEEDED( ret ) )
		return ret;

	ret = SQLExecDirect( hstmt, (_TUCHAR*)szSQL, SQL_NTS );

	if ( !SQL_SUCCEEDED( ret ) ) 
	{
		SQLGetDiagRec( SQL_HANDLE_STMT, hstmt, 1, (_TUCHAR*)m_szState, &m_nNativeErr, 
			           (_TUCHAR*)m_szErrMsg, SQL_MAX_MESSAGE_LENGTH, &m_nErrMsgLength );
	}

	SQLFreeHandle( SQL_HANDLE_STMT, hstmt );
	
	return ret;
}

SQLRETURN CDBConn::GetData( TCHAR* szSQL, TCHAR* data)
{
	SQLHSTMT	hstmt;
	SQLRETURN	ret;
	TCHAR   szName[80];
	SQLINTEGER  cbName;

	ret = SQLAllocHandle( SQL_HANDLE_STMT, m_hdbc, &hstmt );

	if( !SQL_SUCCEEDED( ret ) )
	return ret;

	ret = SQLExecDirect( hstmt, (_TUCHAR*)szSQL, SQL_NTS );

	if ( !SQL_SUCCEEDED( ret ) ) 
	{
		SQLGetDiagRec( SQL_HANDLE_STMT, hstmt, 1, (_TUCHAR*)m_szState, &m_nNativeErr, 
		           (_TUCHAR*)m_szErrMsg, SQL_MAX_MESSAGE_LENGTH, &m_nErrMsgLength );
	}

	ret = SQLFetch(hstmt);
	if (ret == SQL_ERROR || ret == SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec( SQL_HANDLE_STMT, hstmt, 1, (_TUCHAR*)m_szState, &m_nNativeErr, 
	           (_TUCHAR*)m_szErrMsg, SQL_MAX_MESSAGE_LENGTH, &m_nErrMsgLength );
	}

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
	{
		SQLGetData(hstmt,1, SQL_C_CHAR, szName, 80, &cbName);
		lstrcpy(data,szName);
	}
	else
	{
		SQLGetDiagRec( SQL_HANDLE_STMT, hstmt, 1, (_TUCHAR*)m_szState, &m_nNativeErr, 
	           (_TUCHAR*)m_szErrMsg, SQL_MAX_MESSAGE_LENGTH, &m_nErrMsgLength );
	}
		
	SQLFreeHandle( SQL_HANDLE_STMT, hstmt );
	
	return ret;
}
