////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MFINSTALLER/DBConn.h,v 1.3 2006/08/04 20:23:18 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// DBConn.h: interface for the CDBConn class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CDBConn  
{
public:
	CDBConn();
	virtual ~CDBConn();
	SQLRETURN Disconnect(void);
	SQLRETURN Connect( wchar_t* szConnect);
	SQLRETURN ExecuteSQL( wchar_t* szSQL );
	SQLRETURN GetData( wchar_t* szSQL, wchar_t* data, int nNum = 1)
	{
		SQLHSTMT	hstmt;
		SQLRETURN	ret;
		SQLWCHAR   szName[80];
		SQLINTEGER  cbName;

		ret = SQLAllocHandle( SQL_HANDLE_STMT, m_hdbc, &hstmt );

		if( !SQL_SUCCEEDED( ret ) )
		return ret;

		ret = SQLExecDirectW( hstmt, (SQLWCHAR*)szSQL, SQL_NTS );

		if ( !SQL_SUCCEEDED( ret ) ) 
		{
			SQLGetDiagRecW( SQL_HANDLE_STMT, hstmt, 1, m_szState, &m_nNativeErr, 
			           m_szErrMsg, SQL_MAX_MESSAGE_LENGTH, &m_nErrMsgLength );
		}

		ret = SQLFetch(hstmt);
		if (ret == SQL_ERROR || ret == SQL_SUCCESS_WITH_INFO)
		{
			SQLGetDiagRecW( SQL_HANDLE_STMT, hstmt, 1, m_szState, &m_nNativeErr, 
		           m_szErrMsg, SQL_MAX_MESSAGE_LENGTH, &m_nErrMsgLength );
		}

		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		{
			SQLGetData(hstmt,nNum, SQL_C_WCHAR, szName, 80, &cbName);
			wcscpy(data,(const wchar_t*)szName);
		}
		else
		{
			SQLGetDiagRecW( SQL_HANDLE_STMT, hstmt, 1, m_szState, &m_nNativeErr, 
		           m_szErrMsg, SQL_MAX_MESSAGE_LENGTH, &m_nErrMsgLength );
		}
		

		SQLFreeHandle( SQL_HANDLE_STMT, hstmt );
	
		return ret;
	}

	long GetNativeErrorCode(void)	{ return m_nNativeErr; }
	const wchar_t* GetErrorString(void)	const	{ return m_szErrMsg; }
	HDBC GetDBC(void)				{ return m_hdbc; }

	void GetDiagnostics(void)
	{
		SQLGetDiagRecW( SQL_HANDLE_DBC, m_hdbc, 1, m_szState, &m_nNativeErr, 
			           m_szErrMsg, SQL_MAX_MESSAGE_LENGTH, &m_nErrMsgLength );
	}

	wchar_t m_szState[SQL_SQLSTATE_SIZE+1];
	wchar_t m_szErrMsg[SQL_MAX_MESSAGE_LENGTH+1];
	long m_nNativeErr;
	short m_nErrMsgLength;

protected:
	HDBC m_hdbc;
	HENV m_henv;

};

