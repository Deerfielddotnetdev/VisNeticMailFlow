// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__F1759EE8_E0BA_4DFB_B807_07158C774CC0__INCLUDED_)
#define AFX_STDAFX_H__F1759EE8_E0BA_4DFB_B807_07158C774CC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable : 4786)

// Insert your headers here
#include <dcabase\dcabase.h>
#include <dcawin\dcawin.h>
#include <dcawinnt\dcawinnt.h>
#include <dcaodbc\dcaodbc.h>
#include <dcamailflow\dcamailflow.h>

#include "resource.h"
#include <assert.h>				// assertions

#include <WTYPES.H>

#include <process.h>			// threading

#include "gendefs.h"			// application-defined (internal) defs
#include "errorcodes.h"			// application-defined error codes

#include "WinRegistry.h"		// registry access
extern CWinRegistry g_winReg;
#include <DebugReporter.h>

#include <atlbase.h>
#include <atlconv.h>

typedef std::basic_string<TCHAR> tstring;

#ifdef _UNICODE
#define tmemset	wmemset 
#else
#define tmemset memset
#endif

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__F1759EE8_E0BA_4DFB_B807_07158C774CC0__INCLUDED_)
