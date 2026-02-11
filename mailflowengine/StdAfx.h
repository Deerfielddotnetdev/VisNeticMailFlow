#pragma once

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#pragma warning (disable:4101)
//#define WIN32_LEAN_AND_MEAN

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module

#include <dcabase\dcabase.h>
#include <dcawin\dcawin.h>
#include <dcawinnt\dcawinnt.h>
#include <dcakav\dcakav.h>
#include <dcaodbc\dcaodbc.h>
#include <dcamailflow\dcamailflow.h>
#include <DebugReporter.h>

class CServiceModule : public CComModule
{
public:
	HRESULT RegisterServer(BOOL bRegTypeLib, BOOL bService);
	HRESULT UnregisterServer();
	void Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT nServiceNameID, const GUID* plibid = NULL);
    void Start();
	void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    void Handler(DWORD dwOpcode);
    void Run();
    BOOL IsInstalled();
    BOOL Install();
    BOOL Uninstall();
	LONG Unlock();
	void LogEvent(LPCTSTR pszFormat, ...);
    void SetServiceStatus(DWORD dwState, DWORD dwWin32ExitCode = S_OK );
    void SetupAsLocalServer();

//Implementation
private:
	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI _Handler(DWORD dwOpcode);

// data members
public:
    TCHAR m_szServiceName[256];
    SERVICE_STATUS_HANDLE m_hServiceStatus;
    SERVICE_STATUS m_status;
	DWORD dwThreadID;
	BOOL m_bService;
};

extern CServiceModule _Module;

#include <atlcom.h>

#include <sql.h>
#include <sqlext.h>

// Use the standard C Library
#include <stdio.h>
#include <time.h>

// Use the Standard C++ Library, too
#include <string>
#include <map>
#include <vector>
#include <deque>
#include <list>
#include <set>
using namespace std ;

typedef basic_string<TCHAR> tstring;


// global enumerations
#include "EMSIDs.h"
#include "gendefs.h"
#include "EngineErrorCodes.h"		// MailstreamEngine error codes

// global types
#include "dataclasses.h"	// database table structures
#include "ODBCConn.h"		// ODBC Database connection class
#include "ODBCQuery.h"		// ODBC Database query class
#include "EMSString.h"

// global functions
#include "trace.h"
#include "SecurityFns.h"
#include "RegistryFns.h"
#include "DateFns.h"
#include "LogFns.h"
#include "StringFns.h"

#include "QueryClasses.h"

#include "GlobalObject.h"

extern CGlobalObject g_Object;

#ifdef _UNICODE
#define tmemset	wmemset 
#else
#define tmemset memset
#endif
