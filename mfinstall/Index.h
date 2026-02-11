// Index.h: interface for the CIndex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INDEX_H__31140912_7F47_4367_90CC_F0EA0E8B1FA8__INCLUDED_)
#define AFX_INDEX_H__31140912_7F47_4367_90CC_F0EA0E8B1FA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIndex  
{
public:
	CIndex(const wchar_t* wcsName, int cbName );
	virtual ~CIndex();

	void SetColumn( const wchar_t* wcsValue, int cbValue );

	enum 
	{ 
		INDEX_NAME_LENGTH = 127, 
		COLUMN_NAME_LENGTH = 127, 
	};

	wchar_t m_wcsName[INDEX_NAME_LENGTH];
	wchar_t m_wcsColumn[COLUMN_NAME_LENGTH];

};

#endif // !defined(AFX_INDEX_H__31140912_7F47_4367_90CC_F0EA0E8B1FA8__INCLUDED_)
