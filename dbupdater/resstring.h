#ifndef RESSTRING_H
#define RESSTRING_H

//#ifdef _UNICODE
//#define UNICODE
//#endif

#include <windows.h>
#include <TCHAR.H>
#include <string>

class ResString
{
    enum { MAX_RESSTRING = 256 };
public:
    ResString (HINSTANCE hInst, int resId)
	{
		wchar_t buf[MAX_RESSTRING];

		if (!::LoadStringW(hInst, resId, buf, MAX_RESSTRING))
		{
			throw _T("Load String failed");
		}

		_tStr = buf;
	}
    operator LPCTSTR() { return _tStr.c_str(); }
private:
	std::wstring _tStr;
};

#endif // RESSTRING_H
