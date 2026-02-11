////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MFDBUPDATE/dbconn.h,v 1.2 2006/04/03 14:06:32 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// DBConn.h: interface for the CDBConn class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(DBCONN_H)
#define DBCONN_H

//#ifdef _UNICODE
//#define UNICODE
//#endif

#include <windows.h>
#include <TCHAR.H>
#include <sql.h>
#include <sqlext.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDBConn  
{
public:
	CDBConn();
	virtual ~CDBConn();
	SQLRETURN Disconnect(void);
	SQLRETURN Connect( TCHAR* szConnect);
	SQLRETURN ExecuteSQL( TCHAR* szSQL );
	SQLRETURN GetData( TCHAR* szSQL, TCHAR* data );

	long GetNativeErrorCode(void)	{ return m_nNativeErr; }
	TCHAR* GetErrorString(void)		{ return m_szErrMsg; }
	HDBC GetDBC(void)				{ return m_hdbc; }

	void GetDiagnostics(void)
	{
		SQLGetDiagRec( SQL_HANDLE_DBC, m_hdbc, 1, (_TUCHAR*)m_szState, &m_nNativeErr, 
			           (_TUCHAR*)m_szErrMsg, SQL_MAX_MESSAGE_LENGTH, &m_nErrMsgLength );
	}

	TCHAR m_szState[SQL_SQLSTATE_SIZE+1];
	TCHAR m_szErrMsg[SQL_MAX_MESSAGE_LENGTH+1];
	long m_nNativeErr;
	short m_nErrMsgLength;

protected:
	HDBC m_hdbc;
	HENV m_henv;

};

#endif // !defined(DBCONN_H)
