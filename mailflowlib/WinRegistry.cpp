/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/WinRegistry.cpp,v 1.1.4.1 2006/02/23 20:36:51 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Windows Registry Wrapper
||              
\\*************************************************************************/

#include "stdafx.h"
#include "WinRegistry.h"
#include "malloc.h"
#include <tchar.h>

/*-----------------------------------------------------------\\
|| constructor
\*-----------------------------------------------------------*/
CWinRegistry::CWinRegistry()
{
}

/*-----------------------------------------------------------\\
|| destructor
\*-----------------------------------------------------------*/
CWinRegistry::~CWinRegistry()
{
}

/*-----------------------------------------------------------\\
|| write an int to the registry
\*-----------------------------------------------------------*/
bool CWinRegistry::WriteRegInt(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
	// return value
	bool bRetVal = true;
	HKEY hKey = NULL;

	// create/open reg key handle
	if (RegCreateKeyEx(hRoot, lpszSection, NULL, NULL, NULL, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
	{
		bRetVal = false;
	}
	else
	{
		// set the value
		if (RegSetValueEx(hKey, lpszEntry, 0, REG_DWORD, (BYTE*)&nValue, sizeof(nValue)) != ERROR_SUCCESS)
		{
			bRetVal = false;
		}
	}

	// close the reg key if it exists
	if (hKey)
		RegCloseKey(hKey);

	// it worked
	return bRetVal;
}

/*-----------------------------------------------------------\\
|| writes a string to the registry
\*-----------------------------------------------------------*/
bool CWinRegistry::WriteRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	// return value
	bool bRetVal = true;
	HKEY hKey = NULL;

	// create/open reg key handle
	if (RegCreateKeyEx(hRoot, lpszSection, NULL, NULL, NULL, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
	{
		bRetVal = false;
	}
	else
	{
		// set the value
		if (RegSetValueEx(hKey, lpszEntry, 0, REG_SZ, (BYTE*)lpszValue, ((_tcslen(lpszValue) + 1) * sizeof(TCHAR))) != ERROR_SUCCESS)
		{
			bRetVal = false;
		}
	}

	// close the reg key if it exists
	if (hKey)
		RegCloseKey(hKey);

	// it worked
	return bRetVal;
}

/*-----------------------------------------------------------\\
|| reads a string from the registry
\*-----------------------------------------------------------*/
tstring CWinRegistry::GetRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, TCHAR* lpszDefault)
{	
	// var init
	DWORD pBuffLen = 256 * sizeof(TCHAR);	// default buffer len
	TCHAR* pBuff = new TCHAR[256];			// allocate buffer (can be reallocated below)
	long lRetVal;							// function return value
	HKEY hKey = NULL;							// registry key
	tstring sDest = lpszDefault;			// result string (returned to caller)
	
	// open reg key
	lRetVal = RegOpenKeyEx(hRoot,			// hkey
						   lpszSection,		// sub key
						   0,				// reserved
						   KEY_EXECUTE,		// access permissions
						   &hKey);		// return hkey pointer

	// if we can't open the key, return default
	if (lRetVal != ERROR_SUCCESS)
	{
		goto cleanup;
	}

	// query for the registry key value with default buffer size
	lRetVal = RegQueryValueEx(hKey,					// hkey
							  lpszEntry,				// entry name
							  NULL,						// reserved
							  NULL,						// data type return
							  (BYTE*)pBuff,				// return buffer address
							  &pBuffLen);				// buffer length address

	// if registry key value buffer is too small
	if (lRetVal == ERROR_MORE_DATA)
	{
		// reallocate buffer memory and try again
		pBuff = (TCHAR*)realloc(pBuff, pBuffLen);
		if (pBuff == NULL)	// check the return of realloc
			goto cleanup;			

		// try again with the new, larger buffer
		lRetVal = RegQueryValueEx(hKey, lpszEntry, NULL, NULL, (BYTE*)pBuff, &pBuffLen);
	}

	// if everything fails, return default value
	if (lRetVal != ERROR_SUCCESS)
	{
		goto cleanup;
	}

	// everything worked; copy the result buffer to the dest string
	sDest = pBuff;

cleanup:
	if (pBuff)
		delete pBuff;

	// close the reg key
	if (hKey)
		RegCloseKey(hKey);

	return sDest;
}

/*-----------------------------------------------------------\\
|| reads an int from the registry
\*-----------------------------------------------------------*/
UINT CWinRegistry::GetRegInt(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
	// var init
	UINT pBuff = 0;
	DWORD pBuffLen = sizeof(UINT);
	bool bRetVal = true;
	HKEY hKey = NULL;

	// open reg key
	if (RegOpenKeyEx(hRoot, lpszSection, 0, KEY_EXECUTE, &hKey) != ERROR_SUCCESS)
	{
		bRetVal = false;
	}
	else
	{			
		// query for value
		if (RegQueryValueEx(hKey, lpszEntry, NULL, NULL, (BYTE*)&pBuff, &pBuffLen) != ERROR_SUCCESS)
		{
			bRetVal = false;
		}
	}

	// close the reg key if it was successfully opened
	if (hKey)
		RegCloseKey(hKey);

	// return the buffer if return value is true
	// otherwise, return the default provided
	return (bRetVal) ? pBuff : nDefault;
}

