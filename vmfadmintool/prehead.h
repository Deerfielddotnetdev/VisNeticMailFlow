#pragma once

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <afxwin.h>

#include <dcabase\dcabase.h>
#include <dcawin\dcawin.h>
#include <dcawinnt\dcawinnt.h>
#include <dcaodbc\dcaodbc.h>
#include <dcamailflow\dcamailflow.h>

#include ".\MailStreamEngine.h"
#include "resource.h"

using namespace std;
typedef basic_string<TCHAR> tstring;

#include "dataclasses.h"
#include "ODBCQuery.h"
#include "QueryClasses.h"

#include <atlbase.h>
#include <atlconv.h>
