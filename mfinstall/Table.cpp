// Table.cpp: implementation of the CTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Table.h"


// For sorting tables by name
bool operator<(const CTable& x, const CTable& y)
{
    return wcscmp( x.m_wcsName, y.m_wcsName ) < 0;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTable::CTable( const wchar_t* wcsName, int cbName )
{
	memset( m_wcsName, 0, sizeof( m_wcsName ) );
	memset( m_wcsNotes, 0, sizeof( m_wcsNotes ) );

	wcsncpy( m_wcsName, wcsName, cbName );
}

CTable::~CTable()
{

}

void CTable::AddColumn( CColumn& column )
{
	m_Columns.push_back( column );
}

void CTable::AddIndex( CIndex& index )
{	
	m_Indices.push_back( index );
}

void CTable::AddForeignKey( CForeignKey& foreignkey )
{
	m_ForeignKeys.push_back( foreignkey );
}

void CTable::AddData( CData& data )
{
	m_Data.push_back( data );
}

void CTable::SetNotes( wchar_t* wcsValue, int cbValue )
{
	wcsncat( m_wcsNotes, wcsValue, cbValue );	
}