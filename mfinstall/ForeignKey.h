// ForeignKey.h: interface for the CForeignKey class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOREIGNKEY_H__2038AAE8_E774_4984_B5AC_8A8D4D331FD1__INCLUDED_)
#define AFX_FOREIGNKEY_H__2038AAE8_E774_4984_B5AC_8A8D4D331FD1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CForeignKey  
{
public:
	CForeignKey( const wchar_t* wcsName, int cbName );
	virtual ~CForeignKey();

	void SetColumn( const wchar_t* wcsValue, int cbValue );
	void SetForeignTable( const wchar_t* wcsValue, int cbValue );
	void SetForeignColumn( const wchar_t* wcsValue, int cbValue );

	enum 
	{ 
		FOREIGNKEY_NAME_LENGTH = 127, 
		COLUMN_NAME_LENGTH = 127, 
		TABLE_NAME_LENGTH = 127
	};

	wchar_t m_wcsName[FOREIGNKEY_NAME_LENGTH];
	wchar_t m_wcsColumn[COLUMN_NAME_LENGTH];
	wchar_t m_wcsForeignTable[TABLE_NAME_LENGTH];
	wchar_t m_wcsForeignColumn[COLUMN_NAME_LENGTH];

};

#endif // !defined(AFX_FOREIGNKEY_H__2038AAE8_E774_4984_B5AC_8A8D4D331FD1__INCLUDED_)
