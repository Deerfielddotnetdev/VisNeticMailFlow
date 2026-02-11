#ifndef REGVALUE_H
#define REGVALUE_H

//#ifdef _UNICODE
//#define UNICODE
//#endif

#include <windows.h>
#include <TCHAR.H>
#include <mbstring.h>

namespace reg
{
	class Value
	{
		enum{VAL_SIZE = 2048};
	public:
		Value();
		long GetBinary(HKEY hKey,LPCTSTR name,unsigned char* data);
		long GetDword(HKEY hKey,LPCTSTR name,unsigned long& data);
		long GetMultiString(HKEY hKey,LPCTSTR name,unsigned char* data);
		long GetString(HKEY hKey,LPCTSTR name,LPTSTR data);
		long SetBinary(HKEY hKey,LPCTSTR name,unsigned char* data);
		long SetDword(HKEY hKey,LPCTSTR name,unsigned long data);
		long SetMultiString(HKEY hKey,LPCTSTR name,unsigned char* data,long dataSize);
		long SetString(HKEY hKey,LPCTSTR name,LPTSTR data);
	private:
	};
}

#endif //REGVALUE_H