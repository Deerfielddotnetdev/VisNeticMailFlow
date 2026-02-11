/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/RegistryFns.cpp,v 1.1.6.1 2006/05/01 15:44:11 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Registry Helper Functions    
||              
\\*************************************************************************/

#include "stdafx.h"
#include "malloc.h"
#include <tchar.h>

/*-----------------------------------------------------------\\
|| write an int to the registry
\*-----------------------------------------------------------*/
LONG WriteRegInt(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
	LONG lRet = ERROR_SUCCESS;
	HKEY hKey;

	// create/open reg key handle
	if ((lRet = RegCreateKeyEx(hRoot, lpszSection, NULL, NULL, NULL, KEY_WRITE, NULL, &hKey, NULL)) != ERROR_SUCCESS)
		return lRet;
	
	// set the value
	lRet = RegSetValueEx(hKey, lpszEntry, NULL, REG_DWORD, (BYTE*) &nValue, sizeof(nValue));
	
	// close the registry key
	RegCloseKey(hKey);
	
	return lRet;
}

/*-----------------------------------------------------------\\
|| writes a string to the registry
\*-----------------------------------------------------------*/
LONG WriteRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	LONG lRet = ERROR_SUCCESS;
	HKEY hKey;
	
	// create/open reg key handle
	if ((lRet = RegCreateKeyEx(hRoot, lpszSection, NULL, NULL, NULL, KEY_WRITE, NULL, &hKey, NULL)) != ERROR_SUCCESS)
		return lRet;
	
	// set the value
	lRet = RegSetValueEx(hKey, lpszEntry, 0, REG_SZ, (BYTE*) lpszValue, ((_tcsclen(lpszValue) + 1) * sizeof(TCHAR)));

	// close the registry key
	RegCloseKey(hKey);
	
	return lRet;
}

/*-----------------------------------------------------------\\
|| reads a string from the registry
\*-----------------------------------------------------------*/
LONG GetRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, tstring& sValue)
{	
	LONG lRet = ERROR_SUCCESS;
	HKEY hKey;

	TCHAR* pBuff = new TCHAR[256];				// allocate buffer (can be reallocated below)
	DWORD dwBuffLen = (256 * sizeof(TCHAR));	// default buffer len
	DWORD dwType;
	
	// did allocation succeed?
	if( !pBuff )
		return -1;

	// open reg key
	if ((lRet = RegOpenKeyEx(hRoot, lpszSection, NULL, KEY_EXECUTE, &hKey)) != ERROR_SUCCESS)		
		return lRet;

	// query for the registry key value with default buffer size
	lRet = RegQueryValueEx(hKey, lpszEntry, NULL, &dwType, (BYTE*) pBuff, &dwBuffLen);

	// if registry key value buffer is too small
	if (lRet == ERROR_MORE_DATA)
	{
		delete[] pBuff;

		// reallocate buffer memory and try again
		pBuff = new TCHAR[dwBuffLen];
		
		// allocation error occured
		if (!pBuff)
		{
			RegCloseKey(hKey);
			return -1;
		}

		// try again with the new, larger buffer
		lRet = RegQueryValueEx(hKey, lpszEntry, NULL, &dwType, (BYTE*) pBuff, &dwBuffLen);
	}

	// close the registry key
	RegCloseKey(hKey);

	if (lRet == ERROR_SUCCESS)
	{
		// if the type wasn't a string
		if (dwType != REG_SZ)
		{
			lRet = -1;
		}
		else
		{
			// everything worked; copy the result buffer to the dest string
			sValue = pBuff;
		}
	}

	delete[] pBuff;
	
	return lRet;
}

/*-----------------------------------------------------------\\
|| reads a string from the registry
\*-----------------------------------------------------------*/
LONG GetRegString(HKEY hRoot, LPCWSTR lpszSection, LPCWSTR lpszEntry, dca::WString& sValue)
{	
	LONG lRet = ERROR_SUCCESS;
	HKEY hKey;

	wchar_t* pBuff = new wchar_t[256];				// allocate buffer (can be reallocated below)
	DWORD dwBuffLen = (256 * sizeof(wchar_t));	// default buffer len
	DWORD dwType;
	
	// did allocation succeed?
	if( !pBuff )
		return -1;

	// open reg key
	if ((lRet = RegOpenKeyExW(hRoot, lpszSection, NULL, KEY_EXECUTE, &hKey)) != ERROR_SUCCESS)		
		return lRet;

	// query for the registry key value with default buffer size
	lRet = RegQueryValueExW(hKey, lpszEntry, NULL, &dwType, (BYTE*) pBuff, &dwBuffLen);

	// if registry key value buffer is too small
	if (lRet == ERROR_MORE_DATA)
	{
		delete[] pBuff;

		// reallocate buffer memory and try again
		pBuff = new wchar_t[dwBuffLen];
		
		// allocation error occured
		if (!pBuff)
		{
			RegCloseKey(hKey);
			return -1;
		}

		// try again with the new, larger buffer
		lRet = RegQueryValueExW(hKey, lpszEntry, NULL, &dwType, (BYTE*) pBuff, &dwBuffLen);
	}

	// close the registry key
	RegCloseKey(hKey);

	if (lRet == ERROR_SUCCESS)
	{
		// if the type wasn't a string
		if (dwType != REG_SZ)
		{
			lRet = -1;
		}
		else
		{
			// everything worked; copy the result buffer to the dest string
			sValue = pBuff;
		}
	}

	delete[] pBuff;
	
	return lRet;
}

LONG GetRegString(HKEY hRoot, LPCSTR lpszSection, LPCSTR lpszEntry, dca::WString& sValue)
{	
	LONG lRet = ERROR_SUCCESS;
	HKEY hKey;

	wchar_t* pBuff = new wchar_t[256];				// allocate buffer (can be reallocated below)
	DWORD dwBuffLen = (256 * sizeof(wchar_t));	// default buffer len
	DWORD dwType;
	
	// did allocation succeed?
	if( !pBuff )
		return -1;

	// open reg key
	dca::WString sec(lpszSection);
	if ((lRet = RegOpenKeyExW(hRoot, sec.c_str(), NULL, KEY_EXECUTE, &hKey)) != ERROR_SUCCESS)		
		return lRet;

	// query for the registry key value with default buffer size
	dca::WString ent(lpszEntry);
	lRet = RegQueryValueExW(hKey, ent.c_str(), NULL, &dwType, (BYTE*) pBuff, &dwBuffLen);

	// if registry key value buffer is too small
	if (lRet == ERROR_MORE_DATA)
	{
		delete[] pBuff;

		// reallocate buffer memory and try again
		pBuff = new wchar_t[dwBuffLen];
		
		// allocation error occured
		if (!pBuff)
		{
			RegCloseKey(hKey);
			return -1;
		}

		// try again with the new, larger buffer
		lRet = RegQueryValueExW(hKey, ent.c_str(), NULL, &dwType, (BYTE*) pBuff, &dwBuffLen);
	}

	// close the registry key
	RegCloseKey(hKey);

	if (lRet == ERROR_SUCCESS)
	{
		// if the type wasn't a string
		if (dwType != REG_SZ)
		{
			lRet = -1;
		}
		else
		{
			// everything worked; copy the result buffer to the dest string
			sValue = pBuff;
		}
	}

	delete[] pBuff;
	
	return lRet;
}


/*-----------------------------------------------------------\\
|| reads an int from the registry
\*-----------------------------------------------------------*/
LONG GetRegInt(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, UINT& nValue )
{
	LONG lRet = ERROR_SUCCESS;
	HKEY hKey;

	DWORD dwBuffLen = sizeof(UINT);
	DWORD dwType;
	
	// open reg key
	if ((lRet = RegOpenKeyEx(hRoot, lpszSection, NULL, KEY_EXECUTE, &hKey)) != ERROR_SUCCESS)		
		return lRet;

	// query for value
	lRet = RegQueryValueEx(hKey, lpszEntry, NULL, &dwType, (BYTE*) &nValue, &dwBuffLen);

	// if the type was wrong
	if (dwType != REG_DWORD)
		lRet = -1;

	// close the registry key
	RegCloseKey(hKey);
	
	return lRet;
}

// Encrypt all three strings into a one obfuscated string
//DWORD Encrypt( CString& szUsername, CString& szPassword, CString& szDomain, 
//			   LPTSTR* szObfuscated )

LONG WriteEncryptedRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	CEMSString String;
	int nRet;

	String.assign( lpszValue );

	String.Encrypt();

	nRet = WriteRegString( hRoot, lpszSection, lpszEntry, String.c_str() );

	return nRet;
}



// Recover all three strings from one obfuscated string
//DWORD Decrypt( LPCTSTR szObfuscated, 
//			   CString& szUsername, CString& szPassword, CString& szDomain )

LONG GetEncryptedRegString(HKEY hRoot, LPCTSTR lpszSection, LPCTSTR lpszEntry, tstring& sValue)
{
	CEMSString String;
	LONG nRet;

	// Get the encrypted string
	nRet = GetRegString( hRoot, lpszSection, lpszEntry, String );
	if( nRet != ERROR_SUCCESS )
	{
		return nRet;
	}

	String.Decrypt();

	sValue.assign( String );

	return ERROR_SUCCESS;
}

