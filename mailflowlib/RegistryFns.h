/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/RegistryFns.h,v 1.1.6.1 2006/05/01 15:44:11 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Registry Helper Functions   
||              
\\*************************************************************************/

LONG WriteRegInt(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
LONG WriteRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
LONG WriteEncryptedRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
LONG GetEncryptedRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, tstring& sValue);
LONG GetRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, tstring& sValue);
LONG GetRegString(HKEY hRoot, LPCWSTR lpszSection, LPCWSTR lpszEntry, dca::WString& sValue);
LONG GetRegString(HKEY hRoot, LPCSTR lpszSection, LPCSTR lpszEntry, dca::WString& sValue);
LONG GetRegInt(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, UINT& nValue );

