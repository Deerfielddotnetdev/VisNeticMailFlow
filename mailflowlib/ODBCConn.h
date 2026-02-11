/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/ODBCConn.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   ODBC Connection Wrapper
||              
\\*************************************************************************/

#ifndef ODBCCONN_H
#define ODBCCONN_H

#include "EMSString.h"
#include "ErrorCodes.h"

// Database type codes
#define DBTYPE_MSSQL				_T("MSSQL")
#define DBTYPE_MYSQL				_T("MySQL")
#define DBTYPE_ACCESS				_T("Access")

// forward declaration
class CODBCQuery;

// class definition
class CODBCConn  
{

// properties
protected:
	int			m_iDBMSType;			// Database type
	HENV		m_henv;					// Environment handle
	HDBC		m_hdbc;					// Database connection handle

	// For Integrated Windows Authentication
	HANDLE m_hThreadToken1, m_hThreadToken2;

// operations
public:
	CODBCConn();
	virtual ~CODBCConn();

	void Connect( LPCTSTR szDSN = NULL, LPCTSTR szNTUser = NULL, 
		          LPCTSTR szNTPasword = NULL, LPCTSTR szNTDomain = NULL );
	void Disconnect( void );
	void GetConnDiagnostics(ODBCError_t& error);

	// Member access functions
	bool IsConnected(void)			{ return (m_hdbc != SQL_NULL_HDBC); }
	HDBC GetDBC(void)				{ return m_hdbc; }
	int  GetDBMSType(void)			{ return m_iDBMSType; }

// enumerations
enum DBMSType { MSSQL, MySQL, Unknown };

protected:

	void GetDSN( CEMSString& tstrDSN );
	BOOL ImpersonateUser( LPCTSTR szNTUser, LPCTSTR szNTPassword, LPCTSTR szNTDomain );

};

#endif // ODBCCONN_H