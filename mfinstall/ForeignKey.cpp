// ForeignKey.cpp: implementation of the CForeignKey class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ForeignKey.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CForeignKey::CForeignKey( const wchar_t* wcsName, int cbName )
{
	memset( m_wcsName, 0, sizeof( m_wcsName ) );
	memset( m_wcsColumn, 0, sizeof( m_wcsColumn ) );
	memset( m_wcsForeignTable, 0, sizeof( m_wcsForeignTable ) );
	memset( m_wcsForeignColumn, 0, sizeof( m_wcsForeignColumn ) );

	wcsncpy( m_wcsName, wcsName, cbName );
}


CForeignKey::~CForeignKey()
{

}


void CForeignKey::SetColumn( const wchar_t* wcsValue, int cbValue )
{
	wcsncpy( m_wcsColumn, wcsValue, cbValue );	
}

void CForeignKey::SetForeignTable( const wchar_t* wcsValue, int cbValue )
{
	wcsncpy( m_wcsForeignTable, wcsValue, cbValue );	
}

void CForeignKey::SetForeignColumn( const wchar_t* wcsValue, int cbValue )
{
	wcsncpy( m_wcsForeignColumn, wcsValue, cbValue );	
}
