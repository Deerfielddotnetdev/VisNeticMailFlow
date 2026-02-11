// Index.cpp: implementation of the CIndex class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Index.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIndex::CIndex(const wchar_t* wcsName, int cbName )
{
	memset( m_wcsName, 0, sizeof( m_wcsName ) );
	memset( m_wcsColumn, 0, sizeof( m_wcsColumn ) );

	wcsncpy( m_wcsName, wcsName, cbName );
}

CIndex::~CIndex()
{

}


void CIndex::SetColumn( const wchar_t* wcsValue, int cbValue )
{
	wcsncat( m_wcsColumn, wcsValue, cbValue );	
}
