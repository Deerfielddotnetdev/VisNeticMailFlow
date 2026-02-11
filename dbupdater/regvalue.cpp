#include "prehead.h"
#include "RegValue.h"

using namespace reg;

Value::Value() {}

long Value::GetBinary(HKEY hKey,LPCTSTR name,unsigned char* data)
{
	long rc;
	unsigned long dataSize = VAL_SIZE;
	DWORD type = (unsigned long)REG_BINARY;

	rc = RegQueryValueEx(hKey,name,NULL,&type,data,&dataSize);
	if(rc != ERROR_SUCCESS)
	{
		return GetLastError();
	}

	data[dataSize] = '\0';

	return 0;
}

long Value::GetDword(HKEY hKey,LPCTSTR name,unsigned long& data)
{
	long rc;
	unsigned long dataSize = VAL_SIZE;
	DWORD type = (unsigned long)REG_DWORD;
	BYTE* buf = new BYTE[VAL_SIZE];

	rc = RegQueryValueEx(hKey,name,NULL,&type,buf,&dataSize);
	if(rc != ERROR_SUCCESS)
	{
		return GetLastError();
	}

	WORD wd1 = MAKEWORD(buf[0],buf[1]);
	WORD wd2 = MAKEWORD(buf[2],buf[3]);
	data = MAKELONG(wd1,wd2);

	return 0;
}

long Value::GetMultiString(HKEY hKey,LPCTSTR name,unsigned char* data)
{
	long rc;
	unsigned long dataSize = VAL_SIZE;
	DWORD type = (unsigned long)REG_MULTI_SZ;

	rc = RegQueryValueEx(hKey,name,NULL,&type,data,&dataSize);
	if(rc != ERROR_SUCCESS)
	{
		_mbscpy(data,(unsigned char*)"");
		return GetLastError();
	}

	return 0;
}

long Value::GetString(HKEY hKey,LPCTSTR name,LPTSTR data)
{
	long rc;
	unsigned long dataSize = VAL_SIZE;
	DWORD type = (unsigned long)REG_SZ;
	BYTE* buf = new BYTE[VAL_SIZE];

	rc = RegQueryValueEx(hKey,name,NULL,&type,buf,&dataSize);
	if(rc != ERROR_SUCCESS)
	{
		return GetLastError();
	}

	TCHAR* temp = (TCHAR*)buf;
	lstrcpy(data,temp);

	return 0;
}

long Value::SetBinary(HKEY hKey,LPCTSTR name,unsigned char* data)
{
	long rc;
	long dataSize;

	dataSize = _mbslen(data);

	rc = RegSetValueEx(hKey,name,0,REG_BINARY,data,dataSize);
	if(rc != ERROR_SUCCESS)
	{
		return GetLastError();
	}

	return 0;
}

long Value::SetDword(HKEY hKey,LPCTSTR name,unsigned long data)
{
	long rc;
	long dataSize;
	BYTE* buf = new BYTE[VAL_SIZE];

	dataSize = sizeof(data);
	memcpy(buf,&data,dataSize);

	rc = RegSetValueEx(hKey,name,0,REG_DWORD,buf,dataSize);
	if(rc != ERROR_SUCCESS)
	{
		return GetLastError();
	}

	return 0;
}

long Value::SetMultiString(HKEY hKey,LPCTSTR name,unsigned char* data,long dataSize)
{
	long rc;

	rc = RegSetValueEx(hKey,name,0,REG_MULTI_SZ,data,dataSize);
	if(rc != ERROR_SUCCESS)
	{
		return GetLastError();
	}

	return 0;
}

long Value::SetString(HKEY hKey,LPCTSTR name,LPTSTR data)
{
	long rc;
	long dataSize;
	BYTE* buf = new BYTE[VAL_SIZE];
	
#ifdef UNICODE
	char sBuf[VAL_SIZE];
	WideCharToMultiByte(CP_ACP, 0, data, -1, sBuf, VAL_SIZE, NULL, NULL);

	char sName[VAL_SIZE];
	WideCharToMultiByte(CP_ACP, 0, name, -1, sName, VAL_SIZE, NULL, NULL);

	dataSize = strlen(sBuf);
	memcpy(buf,sBuf,dataSize);

	rc = RegSetValueExA(hKey,sName,0,REG_SZ,buf,dataSize);
	if(rc != ERROR_SUCCESS)
	{
		return GetLastError();
	}
#else
	dataSize = lstrlen(data);
	memcpy(buf,data,dataSize);

	rc = RegSetValueEx(hKey,name,0,REG_SZ,buf,dataSize);
	if(rc != ERROR_SUCCESS)
	{
		return GetLastError();
	}
#endif

	return 0;
}
