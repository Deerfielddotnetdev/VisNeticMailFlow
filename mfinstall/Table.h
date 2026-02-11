// Table.h: interface for the CTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TABLE_H__A3C298B7_0AFB_4EE4_A730_B6ECA4EC361D__INCLUDED_)
#define AFX_TABLE_H__A3C298B7_0AFB_4EE4_A730_B6ECA4EC361D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Column.h"
#include "Index.h"
#include "ForeignKey.h"
#include "Data.h"

class CTable  
{
public:
	CTable( const wchar_t* wcsName, int cbName );
	virtual ~CTable();

	void AddColumn( CColumn& column );
	void AddIndex( CIndex& index );
	void AddForeignKey( CForeignKey& foreignkey );
	void AddData( CData& data );

	void SetNotes( wchar_t* wcsValue, int cbValue );

	enum 
	{ 
		TABLE_NAME_LENGTH = 255,
		NOTES_LENGTH = 1024,
	};


	wchar_t m_wcsName[TABLE_NAME_LENGTH];
	wchar_t m_wcsNotes[NOTES_LENGTH];

	list<CColumn> m_Columns;
	list<CIndex> m_Indices;
	list<CForeignKey> m_ForeignKeys;
	list<CData> m_Data;

protected:

};


bool operator<(const CTable& x, const CTable& y);

#endif // !defined(AFX_TABLE_H__A3C298B7_0AFB_4EE4_A730_B6ECA4EC361D__INCLUDED_)
