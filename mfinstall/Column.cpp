// Column.cpp: implementation of the CColumn class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Column.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CColumn::CColumn( const wchar_t* wcsName, int cbName )
{
	memset( m_wcsName, 0, sizeof( m_wcsName ) );
	memset( m_wcsType, 0, sizeof( m_wcsType ) );
	memset( m_wcsNotes, 0, sizeof( m_wcsNotes ) );
	memset( m_wcsDefault, 0, sizeof( m_wcsDefault ) );

	wcsncpy( m_wcsName, wcsName, cbName );

	m_bIdentity = false;
	m_bPrimaryKey = false;
	m_bAllowNulls = false;
	m_bImmutable = false;
}


CColumn::~CColumn()
{

}


void CColumn::SetType( const wchar_t* wcsValue, int cbValue )
{
	wcsncpy( m_wcsType, wcsValue, cbValue );	
}


void CColumn::SetNotes( const wchar_t* wcsValue, int cbValue )
{
	wcsncat( m_wcsNotes, wcsValue, cbValue );	
}

void CColumn::SetDefault( const wchar_t* wcsValue, int cbValue )
{
	wcsncat( m_wcsDefault, wcsValue, cbValue );	
}