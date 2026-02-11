/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/LogFns.cpp,v 1.2 2005/11/29 21:30:04 markm Exp $
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/


#include "stdafx.h"

#include "LogFns.h"
#include "StringFns.h"
#include "DateFns.h"

#include ".\MailStreamEngine.h"
#include ".\MailStreamEngine_i.c"


#ifdef _UNICODE

////////////////////////////////////////////////////////////////////////////////
// 
// WriteLogEntry
// 
////////////////////////////////////////////////////////////////////////////////
void WriteLogEntry( CODBCQuery& query, int LogEntryType, LPCTSTR wcsBuffer )
{
	IRoutingEngineComm* pRoutingEngine;
	HRESULT hres;

	hres = CoCreateInstance( CLSID_RoutingEngineComm, NULL, CLSCTX_LOCAL_SERVER, 
		                     IID_IRoutingEngineComm, (void**) &pRoutingEngine );

	if ( SUCCEEDED( hres ) )
	{
		BSTR bstr = SysAllocString( wcsBuffer );
		pRoutingEngine->Log( LogEntryType, bstr );
		pRoutingEngine->Release();
		SysFreeString( bstr );
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Logs an entry if the given log entry type is enabled. The text for the log
// entry is pulled from the database. Optional arguments are merged with the
// string using vwprintf.
//
////////////////////////////////////////////////////////////////////////////////
void LogIt( CODBCQuery& query, int LogEntryType, unsigned int nStringID, ... )
{
	CEMSString szLogString;
	va_list va;
	
	va_start( va, nStringID );
	szLogString.Format(nStringID, va);
	va_end( va );	
	
	WriteLogEntry( query, LogEntryType, szLogString.c_str() );
}


////////////////////////////////////////////////////////////////////////////////
// 
// Same as above, but uses hard-coded format string.
// 
////////////////////////////////////////////////////////////////////////////////
void LogIt( CODBCQuery& query, int LogEntryType, TCHAR* wcsFormat, ... )
{
	CEMSString szLogString;
	va_list va;
	
	va_start( va, wcsFormat );
	szLogString.Format(wcsFormat, va);
	va_end( va );	
	
	WriteLogEntry( query, LogEntryType, szLogString.c_str() );
}


#endif // #ifdef _UNICODE
