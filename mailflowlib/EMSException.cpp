/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/EMSException.cpp,v 1.2 2005/11/29 21:30:03 markm Exp $
||
||                                         
||  COMMENTS:	Mailstream exception object
||              
\\*************************************************************************/

#include "stdafx.h"
#include "EMSException.h"

/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||				
||  Comments:	Constructs a exception object.    
\*--------------------------------------------------------------------------*/
CEMSException::CEMSException(LPCSTR szFile, int nLine, int nErrorCode, CEMSString& sErrorString, bool bLog, LPCTSTR szField )
{
	// get a pointer the start of the filename
	// we are not interested in the directory
	const char* pPtr = NULL;
	
	if ((pPtr = strrchr(szFile, _T('\\'))) != NULL)
	{
		// advance the pointer past the back slash
		pPtr++;
	}
	else
	{
		pPtr = "Unknown File";
	}
	
	strncpy(m_szFileName, pPtr, MAX_PATH - 1);
	m_nLine = nLine;
	m_nErrorCode = nErrorCode;
	m_sErrorString.assign( sErrorString );
	m_sFieldName.assign( szField );
	m_bLog = bLog;
	
}


CEMSException::CEMSException(LPCSTR szFile, int nLine, int nErrorCode, LPCTSTR szErrorString, bool bLog, LPCTSTR szField )
{
	// get a pointer the start of the filename
	// we are not interested in the directory
	const char* pPtr = NULL;

	if ((pPtr = strrchr(szFile, _T('\\'))) != NULL)
	{
		// advance the pointer past the back slash
		pPtr++;
	}
	else
	{
		pPtr = "Unknown File";
	}

	strncpy(m_szFileName, pPtr, MAX_PATH - 1);
	m_nLine = nLine;
	m_nErrorCode = nErrorCode;
	m_sErrorString.assign( szErrorString );
	m_sFieldName.assign( szField );
	m_bLog = bLog;
}
