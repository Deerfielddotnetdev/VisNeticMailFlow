// Column.h: interface for the CColumn class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLUMN_H__9024EF8E_F343_455D_B164_5E7B8426BB17__INCLUDED_)
#define AFX_COLUMN_H__9024EF8E_F343_455D_B164_5E7B8426BB17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Index.h"

class CColumn  
{
public:
	CColumn( const wchar_t* wcsName, int cbName );
	virtual ~CColumn();

	void SetType( const wchar_t* wcsValue, int cbValue );
	void SetNotes( const wchar_t* wcsValue, int cbValue );
	void SetDefault( const wchar_t* wcsValue, int cbValue );

	void SetIdentity(void) { m_bIdentity = true;}
	void SetPrimaryKey(void) { m_bPrimaryKey = true;}
	void SetAllowNulls(void) { m_bAllowNulls = true;}
	void SetImmutable(void) { m_bImmutable = true;}

	enum 
	{ 
		COLUMN_NAME_LENGTH = 127, 
		COLUMN_TYPE_LENGTH = 127,
		NOTES_LENGTH = 1024,
	};


	wchar_t m_wcsName[COLUMN_NAME_LENGTH];
	wchar_t m_wcsType[COLUMN_TYPE_LENGTH];
	wchar_t m_wcsDefault[COLUMN_TYPE_LENGTH];
	wchar_t m_wcsNotes[NOTES_LENGTH];

	bool m_bPrimaryKey;
	bool m_bIdentity;
	bool m_bAllowNulls;
	bool m_bImmutable;

protected:

};

#endif // !defined(AFX_COLUMN_H__9024EF8E_F343_455D_B164_5E7B8426BB17__INCLUDED_)
