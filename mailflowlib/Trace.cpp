/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/Trace.cpp,v 1.1.4.1 2006/02/23 20:36:51 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:  Recreates the behavior of MFC TRACE Macro 
||              
\\*************************************************************************/

// trace functions are only
// called from debug mode
#include "stdafx.h"
#ifdef _DEBUG
#include "trace.h"
#include <assert.h>
#include "EMSString.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Used to write a literal string to debug output	              
\*--------------------------------------------------------------------------*/
void Trace(LPCTSTR szFormat, ...)
{
	va_list va;
	va_start( va, szFormat );
	
	TraceArgList(szFormat, va);
	
	va_end(va);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Used to write a string from the string table to debug output	              
\*--------------------------------------------------------------------------*/
void Trace(unsigned int nString, ...)
{
	try
	{
		CEMSString sFormat;
		sFormat.LoadString(nString);

		va_list va;
		va_start( va, nString );
		
		TraceArgList(sFormat.c_str(), va);
		
		va_end(va);
	}
	catch (...) 
	{
		Trace(_T("An error occured loading the string (%d)\n"), nString);
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Base Trace function.  Formats the output and writes it to
||				debug output.	              
\*--------------------------------------------------------------------------*/
void TraceArgList(LPCTSTR szFormat, va_list& va)
{
	try
	{	
		CEMSString sOutput;
		sOutput.FormatArgList(szFormat, va);
		
		// add the linebreak if it's not there...
		if (sOutput.length() > 0)
		{
			if (sOutput[sOutput.length() - 1] != _T('\n'))
				sOutput.append(_T("\n"));
		}
	
		OutputDebugString(sOutput.c_str());	
	}
	catch(...)
	{
		OutputDebugString(_T("Error - Trace() failed!!\n"));
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Writes only the filename (not the path) and the line number
||				to debug output.
\*--------------------------------------------------------------------------*/
void TraceShortFileLine(LPCTSTR szFile, int nLine)
{
	CEMSString sMsg;
	LPCTSTR szFormat = _T("%s(%i) : ");
		
	TCHAR* pPtr = NULL;

	// get a pointer the start of the filename
	// we are not interested in the directory
	if ((pPtr = (TCHAR*)_tcsrchr(szFile, _T('\\'))) != NULL)
	{
		// advance the pointer past the back slash
		pPtr++;
	}
	else
	{
		pPtr = _T("Unknown File");
	}
	
	try
	{
		// format the output
		sMsg.Format(szFormat, pPtr, nLine);
	}
	catch(...)
	{
		OutputDebugString(_T("Error - TraceShortFileLine() failed!!\n"));
	}

	OutputDebugString(sMsg.c_str());
}

#endif