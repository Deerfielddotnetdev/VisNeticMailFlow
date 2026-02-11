/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/EMSException.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	COM Exception Object.  This object allows the throwing of an
||				exception containing diagnostic information from a COM
||				component.   
||              
\\*************************************************************************/

#if !defined(AFX_COMEXCEPTION_H__A110607E_9AD8_4FED_A7F4_1A535FE16795__INCLUDED_)
#define AFX_COMEXCEPTION_H__A110607E_9AD8_4FED_A7F4_1A535FE16795__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EMSString.h"

class CEMSException  
{
public:
	
	CEMSException(LPCSTR szFile, int nLine, int nErrorCode, CEMSString& sErrorString, bool bLog = true, LPCTSTR szField = _T(""));
	CEMSException(LPCSTR szFile, int nLine, int nErrorCode, LPCTSTR szErrorString, bool bLog = true, LPCTSTR szField = _T(""));

private:
	
	int m_nErrorCode;
	int m_nLine;
	CHAR m_szFileName[MAX_PATH];
	CEMSString m_sErrorString;
	CEMSString m_sFieldName;
	bool m_bLog;
	
public:

	bool		GetLog()			{return m_bLog;}
	int		    GetErrorCode()		{return m_nErrorCode;}
	int		    GetLine()			{return m_nLine;}
	LPCSTR	    GetFileName()		{return m_szFileName;}
	LPCTSTR     GetErrorString()	{return m_sErrorString.c_str();}
	LPCTSTR     GetFieldName()		{return m_sFieldName.c_str();}
};

// helper macros
#define THROW_EMS_EXCEPTION(a,b) throw CEMSException( __FILE__, __LINE__, a, b);
#define THROW_EMS_EXCEPTION_NOLOG(a,b) throw CEMSException( __FILE__, __LINE__, a, b, false );
#define THROW_VALIDATION_EXCEPTION(a,b) throw CEMSException( __FILE__, __LINE__, E_FormValidation, b, false, a );

#endif // !defined(AFX_COMEXCEPTION_H__A110607E_9AD8_4FED_A7F4_1A535FE16795__INCLUDED_)
