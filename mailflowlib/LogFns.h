/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/LogFns.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/


#ifndef LOG_FNS_H
#define LOG_FNS_H

#include "ODBCQuery.h"

// Logs an entry if the given log entry type is enabled. The text for the log
// entry is pulled from the database. Optional arguments are merged with the
// string using vwprintf.
void LogIt( CODBCQuery& conn, int LogEntryType, unsigned int nStringID, ... );

// Logs an entry with a hard-coded format string - meant for debug logging only.
void LogIt( CODBCQuery& conn, int LogEntryType, LPCWSTR wcsFormat, ... );
void WriteLogEntry( CODBCQuery& query, int LogEntryType, LPCTSTR wcsBuffer );


#endif
