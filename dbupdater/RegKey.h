#ifndef REGKEY_H
#define REGKEY_H

//#ifdef _UNICODE
//#define UNICODE
//#endif

#include <windows.h>
#include <TCHAR.H>

/* Pass in one of the HKEYS
HKEY_CLASSES_ROOT
HKEY_CURRENT_CONFIG
HKEY_CURRENT_USER
HKEY_LOCAL_MACHINE
HKEY_USERS
Windows NT: HKEY_PERFORMANCE_DATA 
Windows 95 and Windows 98: HKEY_DYN_DATA*/


namespace reg
{
	class Key
	{
	public:
		Key();
		operator HKEY () { return _hKey; }
		BOOL Create(HKEY hKey,TCHAR* subKey);
		BOOL Open(HKEY hKey,TCHAR* subKey);
		BOOL Close();
	private:
		HKEY	_hKey;
	};

}

#endif // REGKEY_H