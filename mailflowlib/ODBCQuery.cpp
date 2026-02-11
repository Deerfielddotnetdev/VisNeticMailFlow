/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/ODBCQuery.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||                                         
||  COMMENTS:   ODBC Query Wrapper.  The default constructor will create
||				an ODBCConnection.
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ODBCConn.h"
#include "ODBCQuery.h"
#include "ErrorCodes.h"
#include "DebugReporter.h"

//////////////////////////////////////////////////////////////////////
//
// Construction  -- Creates a database connection
//
//////////////////////////////////////////////////////////////////////
CODBCQuery::CODBCQuery() 
{
	// initialize
	m_bAllocatedConn = true;
	m_hstmt = SQL_NULL_HSTMT;
	m_ParamIndex = 1;
	m_ColIndex = 1;
	m_RowsFetched = 0;
	m_bScrollCursor = false;
	m_bCustomTimeout = false;
	
	// allocate the database connection
	m_pDB = new CODBCConn;

	// if the allocation failed, throw an exception
	if (m_pDB == NULL)
	{
		m_bAllocatedConn = false;
		
		ODBCError_t error;
		m_pDB->GetConnDiagnostics(error);
		error.nErrorCode = E_AllocHandle;
		throw error;
	}
	
	// connect to the database
	m_pDB->Connect();
}


//////////////////////////////////////////////////////////////////////
//
// Construction  -- Uses an existing database connection
//
//////////////////////////////////////////////////////////////////////

CODBCQuery::CODBCQuery( CODBCConn& db ) 
{
	// initialize
	m_bAllocatedConn = false;
	m_hstmt = SQL_NULL_HSTMT;
	m_ParamIndex = 1;
	m_ColIndex = 1;
	m_RowsFetched = 0;
	m_bScrollCursor = false;
	m_bCustomTimeout = false;

	// use the database connection that was passed in
	m_pDB = &db;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Destruction
// 
///////////////////////////////////////////////////////////////////////////////
CODBCQuery::~CODBCQuery()
{
	// close the database connection
	Close();

	// free the database connection
	// if we allocated it
	if (m_bAllocatedConn)
		delete m_pDB;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Close
// 
///////////////////////////////////////////////////////////////////////////////
void CODBCQuery::Close(void)
{
	try
	{
		if( m_hstmt != SQL_NULL_HSTMT )
		{
			SQLFreeHandle( SQL_HANDLE_STMT, m_hstmt );
			m_hstmt = SQL_NULL_HSTMT;		
		}
		m_ColIndex = 1;
		m_ParamIndex = 1;
		m_RowsFetched = 0;
		m_bScrollCursor = false;
	}
	catch(...)
	{
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Initialize
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCQuery::Initialize( SQLUINTEGER FetchRows, SQLUINTEGER FetchRowSize)
{ 
	SQLRETURN ret;
	
	// Allocate a handle if we need one
	if( m_hstmt == SQL_NULL_HSTMT )
	{
		if ( !m_pDB->IsConnected() )
		{
			m_pDB->Connect();
		}
		
		ret = SQLAllocHandle( SQL_HANDLE_STMT, m_pDB->GetDBC(), &m_hstmt ); 
		
		// Handle the error, if appropriate
		if( !SQL_SUCCEEDED( ret ) )
		{
			ODBCError_t error;
			m_pDB->GetConnDiagnostics(error);
			error.nErrorCode = E_AllocHandle;
			throw error;	
		}
		
		SetFetchType( FetchRows, FetchRowSize );
	}
	else 
	{
		if( m_FetchRows != FetchRows )
			SetFetchType( FetchRows, FetchRowSize );
	
		// perform a reset
		Reset( true );
	}

	// do we need to disable scrollable cursors?
	if (m_bScrollCursor)
	{
		// disable scrollable cursors
		ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_FORWARD_ONLY, 0);

		if( !SQL_SUCCEEDED( ret ) )
		{
			ODBCError_t error;
			GetQueryDiagnostics(error);
			error.nErrorCode = E_SetStmtAttrError;
			throw error;
		}

		m_bScrollCursor = false;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetFetchType
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCQuery::SetFetchType( SQLUINTEGER FetchRows, SQLUINTEGER FetchRowSize )
{
	SQLRETURN ret;

	// Set a pointer to the variable used to return row counts for bulk fetching
	ret = SQLSetStmtAttr( m_hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)FetchRows, 0 );

	if( !SQL_SUCCEEDED( ret ) )
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_SetStmtAttrError;
		throw error;
	}
	
	// if we wish to fetch multiple rows
	if( FetchRows > 1 )
	{
		ret = SQLSetStmtAttr( m_hstmt, SQL_ATTR_ROW_BIND_TYPE, 
			(SQLPOINTER)FetchRowSize, 0 );
		
		if( !SQL_SUCCEEDED( ret ) )
		{
			ODBCError_t error;
			GetQueryDiagnostics(error);
			error.nErrorCode = E_SetStmtAttrError;
			throw error;
		}
		
		ret = SQLSetStmtAttr( m_hstmt, SQL_ATTR_ROWS_FETCHED_PTR, 
			(SQLPOINTER)&m_RowsFetched, 0 );
		
		if( !SQL_SUCCEEDED( ret ) )
		{
			ODBCError_t error;
			GetQueryDiagnostics(error);
			error.nErrorCode = E_SetStmtAttrError;
			throw error;
		}
	}

	m_FetchRows = FetchRows;
	m_FetchRowSize = FetchRowSize;
}


////////////////////////////////////////////////////////////////////////////////
// 
// BindParam
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCQuery::BindParam( SQLSMALLINT CType, SQLSMALLINT SQLType, 
							SQLUINTEGER ColumnSize, SQLSMALLINT DecimalDigits,
							SQLPOINTER pParam, SQLINTEGER ParamLen, 
							SQLINTEGER* pLenOrInd )
{
	SQLRETURN ret;
	
	ret = SQLBindParameter( m_hstmt, m_ParamIndex++, SQL_PARAM_INPUT, CType,
		SQLType, ColumnSize, DecimalDigits, pParam,  
		ParamLen, pLenOrInd );
	
	// Check the results
	if( ! SQL_SUCCEEDED( ret ) )
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_DatabaseQuery;
		throw error;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// BindCol
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCQuery::BindCol( SQLSMALLINT CType, SQLPOINTER pParam, 
						  SQLINTEGER ParamLen, SQLINTEGER* pLenOrInd )
{
	SQLRETURN ret;
	
	ret = SQLBindCol( m_hstmt, m_ColIndex++, CType,
		pParam, ParamLen, pLenOrInd );
	
	// Check the results
	if( ! SQL_SUCCEEDED( ret ) )
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_DatabaseQuery;
		throw error;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Execute
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCQuery::Execute( LPCTSTR szSQL )
{
	SQLRETURN ret;

	ret = SQLExecDirect( m_hstmt, (SQLTCHAR*) szSQL, _tcslen(szSQL) );
	
	// Check the results
	if( ! ( SQL_NO_DATA == ret || SQL_SUCCEEDED( ret ) ) )
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_DatabaseQuery;
		throw error;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Fetch
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CODBCQuery::Fetch( void )
{
	SQLRETURN ret;
	
	ret = SQLFetch( m_hstmt );
	
	// Check the results
	if( ret == SQL_NO_DATA )
	{
		m_RowsFetched = 0;
		return S_FALSE;
	}
	else if(! SQL_SUCCEEDED( ret ))
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_DatabaseQuery;
		throw error;
	}
	
	// Success! - reset the column index for GetLongDataColumn
	m_LongDataCol = m_ColIndex;

	return S_OK;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	              
\*--------------------------------------------------------------------------*/
HRESULT CODBCQuery::FetchScroll( SQLSMALLINT FetchOrientation, SQLINTEGER FetchOffset)
{
	SQLRETURN ret;

	// scrollable cursors must be enabled
	if (!m_bScrollCursor)
	{
		m_RowsFetched = 0;
		
		ODBCError_t error;
		_tcscpy((TCHAR*) error.szErrMsg, _T("Error -- Scrollable cursors must be enabled to call FetchScroll\n"));
		error.nErrorCode = E_DatabaseQuery;
		throw error;
	}
	
	ret = SQLFetchScroll(m_hstmt, FetchOrientation, FetchOffset );
	
	// Check the results
	if( ret == SQL_NO_DATA )
	{
		m_RowsFetched = 0;
		return S_FALSE;
	}
	else if(! SQL_SUCCEEDED( ret ))
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_DatabaseQuery;
		throw error;
	}
	
	// Success! - reset the column index for GetLongDataColumn
	m_LongDataCol = m_ColIndex;
	
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetLongDataColumn - This works with MSSQL, but needs to be tested on other
//                     DBMS
// 
// type			- SQL_C_TYPE 
// *ppBuffer	- pointer to dynamic wchar buffer
// *pAllocated	- number of wchars allocated
// *pLength		- number of wchars in string
//
////////////////////////////////////////////////////////////////////////////////
void CODBCQuery::GetData( short type, TCHAR** ppBuffer, 
						  long* pAllocated, long* pLength )
{
	SQLRETURN ret;
	int nNewSize;
	long nOffset;

	// Allocate initial buffer if necessary
	if( *ppBuffer == NULL )
	{
		*ppBuffer = (TCHAR*) calloc( LONG_DATA_CHUNK, 1 );
		*pAllocated = LONG_DATA_CHUNK;
	}
	
	ret = SQLGetData( m_hstmt, m_LongDataCol, type, *ppBuffer, *pAllocated, pLength ); 
	
	if( !SQL_SUCCEEDED( ret ) )
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_SQLGetDataError;
		throw error;
	}
	
	if( *pLength == SQL_NO_TOTAL )
	{
		while( ret == SQL_SUCCESS_WITH_INFO )	// more data out there
		{
			// Calculate size of new buffer
			nNewSize = *pAllocated + LONG_DATA_CHUNK;
			
			// Allocate the new buffer
			TCHAR* szNewBuffer = (TCHAR*) calloc( nNewSize, 1 );
			
			// Copy over the data we already have.
			memcpy( szNewBuffer, *ppBuffer, *pAllocated );
			
			// Free the old buffer
			free( *ppBuffer );
			
			// Set the old pointer to the new buffer
			*ppBuffer = szNewBuffer;
			
			// calculate the offset to get the next chunk at
			nOffset = (*pAllocated / sizeof(TCHAR)) - 1;

			// Get the next chunk
			ret = SQLGetData( m_hstmt, m_LongDataCol, type, *ppBuffer + nOffset, 
							  nNewSize - nOffset * sizeof(TCHAR), pLength ); 
						
			// Re-calculate the size of the buffer allocated.
			*pAllocated = nNewSize;
			
			if( !SQL_SUCCEEDED( ret ) )
			{
				ODBCError_t error;
				GetQueryDiagnostics(error);
				error.nErrorCode = E_SQLGetDataError;
				throw error;
			}
		}
	}
	else if ( ret == SQL_SUCCESS_WITH_INFO  )
	{
		// we have the total length of the string
		nNewSize = *pLength + sizeof(TCHAR);
		
		// Allocate the new buffer
		TCHAR* szNewBuffer = (TCHAR*) calloc( nNewSize, 1 );
		
		// Copy over the data we already have.
		memcpy( szNewBuffer, *ppBuffer, *pAllocated );
		
		// Free the old buffer
		free( *ppBuffer );
		
		// Set the old pointer to the new buffer
		*ppBuffer = szNewBuffer;
		
		// calculate the offset to get the next chunk at
		nOffset = (*pAllocated / sizeof(TCHAR)) - 1;

		// Get the next chunk
		ret = SQLGetData( m_hstmt, m_LongDataCol, type, *ppBuffer + nOffset, 
			              nNewSize - nOffset * sizeof(TCHAR), pLength ); 
		
		// Re-calculate the full length of the string.
		*pLength += nOffset * sizeof(TCHAR);
		
		// Re-calculate the size of the buffer allocated.
		*pAllocated = nNewSize;
		
		if( !SQL_SUCCEEDED( ret ) )
		{
			ODBCError_t error;
			GetQueryDiagnostics(error);
			error.nErrorCode = E_SQLGetDataError;
			throw error;
		}
	}
	
	// Bump the column index for the next call
	m_LongDataCol++;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	              
\*--------------------------------------------------------------------------*/
void CODBCQuery::GetFixedLenTextData( LPCTSTR pBuffer, long nBuffLen )
{
	SQLRETURN ret;
	long nLength;

	ret = SQLGetData( m_hstmt, m_LongDataCol, SQL_C_TCHAR, (SQLPOINTER) pBuffer, nBuffLen, &nLength ); 
	
	if( !SQL_SUCCEEDED( ret ) )
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_SQLGetDataError;
		throw error;
	}
}


/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Closes the result set cursor:
||
||				bool bResetBindings - Resets column and paramater bindings	              
\*--------------------------------------------------------------------------*/
void CODBCQuery::Reset(bool bResetBindings /* = true */)
{
	SQLRETURN ret;
	
	// reset the number of rows fetched
	m_RowsFetched = 0;

	// close the cursor
	ret = SQLFreeStmt( m_hstmt,	SQL_CLOSE);
	
	// Handle the error, if appropriate
	if( !SQL_SUCCEEDED( ret ) )
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_SQLFreeStmt;
		throw error;
	}
	
	// If the timeout was changed, set it back to default (no timeout)
	if( m_bCustomTimeout )
	{
		SQLRETURN ret;

		ret = SQLSetStmtAttr( m_hstmt, SQL_ATTR_QUERY_TIMEOUT, 
							  (SQLPOINTER)0, SQL_IS_UINTEGER );

		if( ! SQL_SUCCEEDED( ret ) )
		{
			ODBCError_t error;
			GetQueryDiagnostics(error);
			error.nErrorCode = E_SetConnOptionError;
			throw error;
		}
		m_bCustomTimeout = false;
	}

	// are we supposed to reset the bindings?
	if (bResetBindings) 
	{
		// reset colunm bindings 
		if (m_ColIndex > 1)
		{
			ret = SQLFreeStmt( m_hstmt,	SQL_UNBIND);
			
			// Handle the error, if appropriate
			if( !SQL_SUCCEEDED( ret ) )
			{
				ODBCError_t error;
				GetQueryDiagnostics(error);
				error.nErrorCode = E_SQLFreeStmt;
				throw error;
			}
			
			// reset the index
			m_ColIndex = 1;
		}
		
		//Reset paramater bindings
		if (m_ParamIndex > 1)
		{
			ret = SQLFreeStmt( m_hstmt,	SQL_RESET_PARAMS);
			
			// Handle the error, if appropriate
			if( !SQL_SUCCEEDED( ret ) )
			{
				ODBCError_t error;
				GetQueryDiagnostics(error);
				error.nErrorCode = E_SQLFreeStmt;
				throw error;
			}
			
			// reset the index
			m_ParamIndex = 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetRowCount - Use GetRowCount only for INSERT, UPDATE, and DELETE(s).
// 
////////////////////////////////////////////////////////////////////////////////
long CODBCQuery::GetRowCount( void )
{
	SQLRETURN ret;
	long RowCount = 0;
	
	ret = SQLRowCount( m_hstmt, &RowCount );
	
	// Handle the error, if appropriate
	if( !SQL_SUCCEEDED( ret ) )
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_SQLRowCount;
		throw error;
	}
	
	return RowCount;
}

/*---------------------------------------------------------------------------\                    
||  Comments:	Use to get the ID of inserted records              
\*--------------------------------------------------------------------------*/
long CODBCQuery::GetLastInsertedID( void )
{
	long nID = 0;
	
	// initialize ourself
	Initialize();
	
	// bind to the result set
	BindCol(SQL_C_LONG, &nID, sizeof(nID), NULL);
	
	// execute the correct query for the database
	// platform
	if( m_pDB->GetDBMSType() == CODBCConn::MySQL )
	{
		Execute( _T("SELECT LAST_INSERT_ID()") );
	}
	else
	{
		Execute( _T("SELECT @@identity") );
	}
	
	// fetch the ID
	if (Fetch() != S_OK)
	{
		// throw an exception if we can't fetch the ID
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_DatabaseQuery;
		throw error;
	}
	
	return nID;
}

/*---------------------------------------------------------------------------\                       
||  Comment:	Changes the statement to use scrollable cursors
||				This will be in effect untill Initialize is called again.	              
\*--------------------------------------------------------------------------*/
void CODBCQuery::EnableScrollCursor()
{
	SQLRETURN ret;

	// enable scrollable cursors
	ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_STATIC, 0);
	
	// Handle the error, if appropriate
	if (ret != SQL_SUCCESS)
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_SetStmtAttrError;
		throw error;
	}
	
	m_bScrollCursor = true;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetQueryDiagnostics
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCQuery::GetQueryDiagnostics(ODBCError_t& error)
{
	short nErrMsgLength;
	
	SQLGetDiagRec( SQL_HANDLE_STMT, m_hstmt, 1, error.szState, &error.nNativeErrorCode, 
		error.szErrMsg, SQL_MAX_MESSAGE_LENGTH, &nErrMsgLength );
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetMoreResults
// 
////////////////////////////////////////////////////////////////////////////////
bool CODBCQuery::GetMoreResults(void)
{
	SQLRETURN ret;

	ret = SQLMoreResults( m_hstmt );

	return SQL_SUCCEEDED( ret );
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetCustomTimeout
// 
////////////////////////////////////////////////////////////////////////////////
void CODBCQuery::SetCustomTimeout( int nSeconds )
{
	SQLRETURN ret;
	
	ret = SQLSetStmtAttr( m_hstmt, SQL_ATTR_QUERY_TIMEOUT, 
						  (SQLPOINTER) nSeconds, SQL_IS_UINTEGER );


	if( ! SQL_SUCCEEDED( ret ) )
	{
		ODBCError_t error;
		GetQueryDiagnostics(error);
		error.nErrorCode = E_SetConnOptionError;
		throw error;
	}

	m_bCustomTimeout = true;
}

