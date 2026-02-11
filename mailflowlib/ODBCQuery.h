/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/ODBCQuery.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   ODBC Query Wrapper.  The default constructor will create
||				an ODBCConnection.
||              
\\*************************************************************************/

#ifndef ODBCQUERY_H
#define ODBCQUERY_H

#include "ODBCConn.h"
#include "ErrorCodes.h"

////////////////////////////////////////////////////////////////////////////////
// STB - Macros for binding paramaters
// A = Query Object, B = The variable to bind to
		
// allows a paramater of SQL_NO_DATA.  The application must specify a variable with LEN
// appeneded to the name.  This variable must be set to the length of the data or SQL_NULL_DATA
#define BINDPARAM_TIME(a,b)  a.BindParam(SQL_C_TIMESTAMP,SQL_TYPE_TIMESTAMP,19,3,&b,0,&b##Len)

// does not allow for the paramater to be SQL_NULL_DATA does not require a Len variable
#define BINDPARAM_TIME_NOLEN(a,b)  a.BindParam(SQL_C_TIMESTAMP,SQL_TYPE_TIMESTAMP,19,3,&b,0,NULL)

#define BINDPARAM_TCHAR(a,b) a.BindParam(SQL_C_TCHAR,SQL_VARCHAR,256,0,b,0,NULL)
#define BINDPARAM_WCHAR(a,b) a.BindParam(SQL_C_WCHAR,SQL_VARCHAR,256,0,b,0,NULL)
#define BINDPARAM_CHAR(a,b)  a.BindParam(SQL_C_CHAR, SQL_VARCHAR,256,0,b,0, NULL)

#define BINDPARAM_TEXT(a,b)  { b##Len = (sizeof(TCHAR) * _tcslen(b)); \
							   if (b##Len == 0) b##Len = 1; \
							   a.BindParam(SQL_C_TCHAR,SQL_LONGVARCHAR,b##Len,0,b,0,NULL); }

#define BINDPARAM_TCHAR_STRING(a,b)  a.BindParam(SQL_C_TCHAR, SQL_VARCHAR, 256, 0, \
                                     (SQLPOINTER) b.c_str(), 0, NULL);

#define BINDPARAM_WCHAR_STRING(a,b)  a.BindParam(SQL_C_WCHAR, SQL_VARCHAR, 256, 0, \
                                     (SQLPOINTER) b.c_str(), 0, NULL);

#define BINDPARAM_TEXT_STRING(a,b)  a.BindParam(SQL_C_TCHAR, SQL_LONGVARCHAR, (b.length() > 0) ? b.length() : 1, \
									0, (SQLPOINTER) b.c_str(), 0, NULL);

#define BINDPARAM_LONG(a,b)    a.BindParam(SQL_C_LONG,SQL_INTEGER,10,0,&b,0,NULL)
#define BINDPARAM_BIT(a,b)     a.BindParam(SQL_C_BIT,SQL_BIT,1,0,&b,0,NULL)
#define BINDPARAM_TINYINT(a,b) a.BindParam(SQL_C_TINYINT,SQL_TINYINT,3,0,&b,0,NULL)


////////////////////////////////////////////////////////////////////////////////
// MJM - Macros for binding to the result set
// A = Query Object, B = The variable to bind to		
#define BINDCOL_LONG(a,b)			a.BindCol(SQL_C_LONG, &b, sizeof(b), &b##Len)
#define BINDCOL_LONG_NOLEN(a,b)		a.BindCol(SQL_C_LONG, &b, sizeof(b), NULL)
#define BINDCOL_TIME(a,b)			a.BindCol(SQL_C_TIMESTAMP, &b, sizeof(b), &b##Len)
#define BINDCOL_BIT(a,b)			a.BindCol(SQL_C_BIT, &b, sizeof(b), &b##Len)
#define BINDCOL_BIT_NOLEN(a,b)		a.BindCol(SQL_C_BIT, &b, sizeof(b), NULL)
#define BINDCOL_TINYINT(a,b)		a.BindCol(SQL_C_TINYINT, &b, sizeof(b), &b##Len)
#define BINDCOL_TINYINT_NOLEN(a,b)	a.BindCol(SQL_C_TINYINT, &b, sizeof(b), NULL)

#define BINDCOL_CHAR(a,b)			a.BindCol(SQL_C_CHAR, b, sizeof(b), &b##Len)
#define BINDCOL_CHAR_NOLEN(a,b)		a.BindCol(SQL_C_CHAR, b, sizeof(b), NULL)

#define BINDCOL_WCHAR(a,b)			a.BindCol(SQL_C_WCHAR, b, sizeof(b), &b##Len)
#define BINDCOL_WCHAR_NOLEN(a,b)	a.BindCol(SQL_C_WCHAR, b, sizeof(b), NULL)

#define BINDCOL_TCHAR(a,b)			a.BindCol(SQL_C_TCHAR, b, sizeof(b), &b##Len)
#define BINDCOL_TCHAR_NOLEN(a,b)	a.BindCol(SQL_C_TCHAR, b, sizeof(b), NULL)

////////////////////////////////////////////////////////////////////////////////
// STB - Macros for getting long data
// A = Query Object, B = The variable retrieve data for
// Since this function allocates a buffer, it is important to do the following:
//  1. Initialize the member variables to zero in object constructor
//  2. Delete the dynamic buffer if allocated in the object destructor
#define GETDATA_TEXT(a,b) a.GetData( SQL_C_TCHAR, &b, &b##Allocated, &b##Len)  	

// forward declaration
class CODBCConn;

class CODBCQuery  
{
public:

	enum { LONG_DATA_CHUNK = 1024 };

	// construction
	CODBCQuery();
	CODBCQuery( CODBCConn& db );
	
	virtual ~CODBCQuery();

	// operations
	SQLHSTMT GetStatementHandle(void) { return m_hstmt; }

	SQLUINTEGER GetRowsFetched(void) { return m_RowsFetched; }

	void Initialize( SQLUINTEGER FetchRows = 1, SQLUINTEGER FetchRowSize = 0);

	void BindParam( SQLSMALLINT CType, SQLSMALLINT SQLType, 
					   SQLUINTEGER ColumnSize, SQLSMALLINT DecimalDigits,
					   SQLPOINTER pParam, SQLINTEGER ParamLen, 
					   SQLINTEGER* pLenOrInd );

	void BindCol( SQLSMALLINT CType, SQLPOINTER pParam, 
					 SQLINTEGER ParamLen, SQLINTEGER* pLenOrInd );

	void Execute( LPCTSTR wcsSQL );

	HRESULT Fetch( void );
	
	HRESULT FetchScroll( SQLSMALLINT FetchOrientation, SQLINTEGER FetchOffset = NULL);

	void GetData( short type, TCHAR** ppBuffer, long* pAllocated, long* pLength );

	void GetFixedLenTextData( LPCTSTR pBuffer, long nBuffLen );

	void Reset(bool bResetBindings = true );

	void EnableScrollCursor(void);

	// Use GetRowCount only for INSERT, UPDATE, and DELETE(s).
	long GetRowCount( void );
	
	// use to get the ID of inserted records
	long GetLastInsertedID( void );
	
	void Close();
	
	void GetQueryDiagnostics(ODBCError_t& error);
	
	bool GetMoreResults(void);
	
	void SetCustomTimeout( int nSeconds );

	// accessor functions
	CODBCConn* GetODBCConn()			{return m_pDB;}
	bool GetAllocatedConn()				{return m_bAllocatedConn;}	

// properties
protected:

	void SetFetchType( SQLUINTEGER FetchRows, SQLUINTEGER FetchRowSize );

	CODBCConn*		m_pDB;				// The database connection
	bool			m_bAllocatedConn;	// Flag if we allocated the connection object
	bool			m_bScrollCursor;	// Is the cursor currently scrollable
	SQLHSTMT		m_hstmt;			// The statement handle
	SQLUSMALLINT    m_ParamIndex;		// Used by BindParam
	SQLUSMALLINT    m_ColIndex;			// Used by BindCol
	SQLUINTEGER		m_RowsFetched;		// Used in bulk fetches
	SQLUSMALLINT	m_LongDataCol;		// Used by GetLongDataColumn
	SQLUINTEGER		m_FetchRows;		// From last call to Initalize()
	SQLUINTEGER		m_FetchRowSize;		// From last call to Initalize()
	bool			m_bCustomTimeout;	// Set if the timeout was changed
};

#endif // ODBCQUERY_H