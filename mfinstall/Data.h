// Data.h: interface for the CData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATA_H__A3B0C6D4_1B29_4EF4_A681_B43030A48D50__INCLUDED_)
#define AFX_DATA_H__A3B0C6D4_1B29_4EF4_A681_B43030A48D50__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CData  
{
public:
	CData();
	virtual ~CData();

	void SetColumns( const wchar_t* wcsValue, int cbValue );
	void SetValues( const wchar_t* wcsValue, int cbValue );
	void SetConstantName( const wchar_t* wcsValue, int cbValue );


	enum 
	{ 
		DATA_COLUMNS_LENGTH = 255, 
		DATA_VALUES_LENGTH = 255,
		DATA_CONSTANTNAMES_LENGTH = 255,
	};

	wchar_t m_wcsColumns[DATA_COLUMNS_LENGTH];
	wchar_t m_wcsValues[DATA_VALUES_LENGTH];
	wchar_t m_wcsConstantName[DATA_CONSTANTNAMES_LENGTH];

};

#endif // !defined(AFX_DATA_H__A3B0C6D4_1B29_4EF4_A681_B43030A48D50__INCLUDED_)
