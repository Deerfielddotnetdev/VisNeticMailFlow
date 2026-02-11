/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/Trace.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS: Recreates the behavior of MFC TRACE Macro  
||              
\\*************************************************************************/

// only include this file once
#pragma once

#include <assert.h>

#ifdef _DEBUG

	void Trace(LPCTSTR szMsg, ...);
	void Trace(unsigned int nString, ...);
	void TraceArgList(LPCTSTR szFormat, va_list& va);
	
	void TraceShortFileLine(LPCTSTR szFile, int nLine);
	
	#define ASSERT(f) assert(f)
	#define VERIFY(f) ASSERT(f) 
	
	#ifdef TRACE_FILENAME_ONLY
		#define LINETRACE TraceShortFileLine(_T(__FILE__), __LINE__); Trace
		#define ASSERT_LINETRACE ASSERT(0); TraceShortFileLine(_T(__FILE__), __LINE__); Trace
	#else
		#define LINETRACE Trace(_T("%s(%i) : "), _T(__FILE__), __LINE__); Trace
		#define ASSERT_LINETRACE ASSERT(0); Trace(_T("%s(%i) : "), _T(__FILE__), __LINE__); Trace
	#endif

#else

	#define ASSERT(f) ((void)0)
	#define VERIFY(f) ((void)f)

	// MJM - These macros comment out trace lines for release builds.  
	// this ensures that unused literal strings are not added to the image.  
	// As a limitation the macro is limited to a single line
	#define LINETRACE /##/
	#define ASSERT_LINETRACE /##/

#endif

