// OLEBASE.H

#ifndef OLEBASE_H
#define OLEBASE_H

#include <objbase.h>

class UseOle
{
public:
	UseOle()
	{
		HRESULT hr;

		hr = OleInitialize(0);
		if(hr != S_OK && hr != S_FALSE)
		{
			throw "Failed to intialize OLE";
		}
	}
	~UseOle()
	{
		OleUninitialize();
	}
};

class BString
{
	friend class CString;
public:
	BString() : _str(0)
	{}
	BString(VARIANT& var)
	{
		if(var.vt != VT_BSTR)
		{
			throw "Variant type is not a BSTR";
		}
		_str = var.bstrVal;
	}
	BString(WCHAR* str) : _str(::SysAllocString(str))
	{}
	~BString()
	{
		::SysFreeString(_str);
	}
	BSTR* GetPointer() { return &_str; }
	unsigned int GetLength() { return SysStringLen(_str); }
	operator BSTR() { return _str; }
protected:
	BSTR _str;
};

class CString
{
public:
	CString(BString& bstr) : _len(::SysStringLen(bstr._str) + 1)
	{
		_str = new char[_len];
		size_t charsRet = 0;
		wcstombs_s(&charsRet, _str, _len, bstr._str, bstr.GetLength());
		_str[_len] = '\0';
	}
	~CString()
	{
		delete [] _str;
	}
	operator char const* () const { return _str; }
	int Len() { return _len; }
	bool IsEqual(char const* str)
	{
		return strcmp(str,_str) == 0;
	}
protected:
	char*	_str;
	int		_len;
};

#endif // OLEBASE_H