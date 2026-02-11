/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/WinRegistry.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Windows Registry Wrapper
||              
\\*************************************************************************/

#if !defined(AFX_WINREGISTRY_H__428EBC21_2CAA_11D4_A7A1_0000F11129C7__INCLUDED_)
#define AFX_WINREGISTRY_H__428EBC21_2CAA_11D4_A7A1_0000F11129C7__INCLUDED_


#include "stdafx.h"

// CWinRegistry class
class CWinRegistry
{
// Construction
public:
	CWinRegistry();
	virtual ~CWinRegistry();

// Attributes
private:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinRegistry)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	UINT	GetRegInt(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
	tstring GetRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, TCHAR* lpszDefault = _T(""));
	bool	WriteRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
	bool	WriteRegInt(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINREGISTRY_H__428EBC21_2CAA_11D4_A7A1_0000F11129C7__INCLUDED_)
