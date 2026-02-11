#include "prehead.h"
#include "regkey.h"
#include <tchar.h>

using namespace reg;

Key::Key() : _hKey(0) {}

BOOL Key::Create(HKEY hKey,TCHAR* subKey)
{
		long rc;
		DWORD disp;

		rc = RegCreateKeyEx(hKey,subKey,0,_T(""),REG_OPTION_NON_VOLATILE,
			                KEY_ALL_ACCESS,NULL,&_hKey,&disp);
		if(rc != ERROR_SUCCESS)
		{
			return FALSE;
		}

		return TRUE;
}

BOOL Key::Open(HKEY hKey,TCHAR* subKey)
{
	long rc;

	rc = RegOpenKeyEx(hKey,subKey,0,KEY_ALL_ACCESS,&_hKey);
	
	if(rc != ERROR_SUCCESS)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL Key::Close()
{
	long rc;

	rc = RegCloseKey(_hKey);
	if(rc != ERROR_SUCCESS)
	{
		return FALSE;
	}

	return TRUE;
}