
#pragma warning( disable : 4786 )

// Use the standard C Library
#include <cstdio>

#include <tchar.h>
#include <dcabase\dcabase.h>
#include <dcawin\dcawin.h>
#include <dcaodbc\dcaodbc.h>
#include <dcakav\dcakav.h>

// windows headers
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <tchar.h>
#include <assert.h>

// STL Support
#include <deque>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <list>
using namespace std;

// support for tstrings
typedef basic_string<TCHAR> tstring;

// other includes
#include "gendefs.h"
#include "EMSIDs.h"
#include "ErrorCodes.h"
#include "trace.h"
#include "dataclasses.h"
#include "EMSString.h"
#include "ODBCQuery.h"
#include "EMSException.h"

