//*****************************************************************************
// Deerfield Computer Associates Library (DCALIB)
// Written by Mark Mohr
// 10/11/2004
//*****************************************************************************

#pragma once

typedef TCHAR TString255[256];
typedef TCHAR TString511[512];

class TString : public std::basic_string<TCHAR>
{
public:
	TString(void);
	~TString(void);
	TString(const TCHAR* str);
	TString(size_type size, const TCHAR* str, ...);
	TString(const _Myt& str);
	TString(HINSTANCE hInstance, int nID);

	const TString& tolower();
	const TString& toupper();
	int toInt();
	const TString& trim(TCHAR c = 0);
	const TString& trimStart(TCHAR c = 0);
	const TString& trimEnd(TCHAR c = 0);
};

typedef std::vector<TString> TStringVector;
