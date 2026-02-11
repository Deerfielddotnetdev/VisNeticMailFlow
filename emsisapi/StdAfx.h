////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/StdAfx.h,v 1.2.2.1.2.1 2006/07/18 12:55:03 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

// only include this file once
#pragma once

#define _SCL_SECURE_NO_WARNINGS

#define SESSIONID_LENGTH		16
#define VC_EXTRALEAN
#define WIN32_EXTRA_LEAN
//#define DEMO_VERSION

#include <dcabase\dcabase.h>
#include <dcawin\dcawin.h>
#include <dcaodbc\dcaodbc.h>
#include <dcakav\dcakav.h>

#include <windows.h>
#include <httpext.h>
#include <msxml.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <sql.h>
#include <sqlext.h>
#include <assert.h>

#pragma warning( disable : 4786 ) 
#pragma warning( disable : 4503 )  

#include <deque>
#include <map>
#include <set>
#include <list>

using namespace std ;

typedef basic_string<TCHAR> tstring;

#include "EMSIDs.h"
#include "gendefs.h"
#include "ErrorCodes.h"

#include "ODBCQuery.h"
#include "dataclasses.h"
#include "queryclasses.h"

#include <DebugReporter.h>

#include "EMSString.h"
#include "EMSException.h"
#include "Trace.h"
#include "ISAPIData.h"
#include "CacheClasses.h"
#include "CacheTemplates.h"
#include "XMLDataClass.h"
#include "ListClasses.h"
#include "Threadpool.h"

// globals
extern CThreadPool g_ThreadPool;

// support for demo version
#if defined(DEMO_VERSION)
	#define DISABLE_IN_DEMO() if (GetAgentID() != 1) {THROW_EMS_EXCEPTION_NOLOG( E_AccessDenied, _T("Feature has been disabled for this demonstration") );}
#else
	#define DISABLE_IN_DEMO()
#endif
