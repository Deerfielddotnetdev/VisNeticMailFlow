////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MFINSTALLER/DBConn.cpp,v 1.5 2006/08/07 12:38:07 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// DBConn.cpp: implementation of the CDBConn class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
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
	wmemset( m_szState, 0, SQL_SQLSTATE_SIZE+1 );
	wmemset( m_szErrMsg, 0, SQL_MAX_MESSAGE_LENGTH+1 );
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
SQLRETURN CDBConn::Connect(wchar_t *szConnect)
{
	SQLRETURN ret;
	//wchar_t szConnectInput[1024] = {'\0'};
	wchar_t szConnectOutput[1024];
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

	// Connect to DSN
	ret = SQLDriverConnectW( m_hdbc, NULL, 
		                   szConnect, SQL_NTS, 
						   szConnectOutput, sizeof(szConnectOutput), 
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
SQLRETURN CDBConn::ExecuteSQL( wchar_t* szSQL )
{
	SQLHSTMT	hstmt;
	SQLRETURN	ret;

	ret = SQLAllocHandle( SQL_HANDLE_STMT, m_hdbc, &hstmt );

	if( !SQL_SUCCEEDED( ret ) )
		return ret;

	ret = SQLExecDirectW( hstmt, (SQLWCHAR*)szSQL, SQL_NTS );

	if ( !SQL_SUCCEEDED( ret ) ) 
	{
		SQLGetDiagRecW( SQL_HANDLE_STMT, hstmt, 1, m_szState, &m_nNativeErr, 
			           m_szErrMsg, SQL_MAX_MESSAGE_LENGTH, &m_nErrMsgLength );
	}

	SQLFreeHandle( SQL_HANDLE_STMT, hstmt );
	
	return ret;
}
