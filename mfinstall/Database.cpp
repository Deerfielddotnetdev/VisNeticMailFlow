// Database.cpp: implementation of the CDatabase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Database.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDatabase::CDatabase(const wchar_t* wcsName, int cbName )
{
	memset( m_wcsName, 0, sizeof( m_wcsName ) );
	memset( m_wcsDSN, 0, sizeof( m_wcsDSN ) );
	memset( m_wcsAction, 0, sizeof( m_wcsAction ) );
	memset( m_wcsType, 0, sizeof( m_wcsType ) );

	wcsncpy( m_wcsName, wcsName, cbName );
}

CDatabase::~CDatabase()
{

}

void CDatabase::SetDSN( const wchar_t* wcsValue, int cbValue )
{
	wcsncpy( m_wcsDSN, wcsValue, cbValue );
}

void CDatabase::SetAction( const wchar_t* wcsValue, int cbValue )
{
	wcsncpy( m_wcsAction, wcsValue, cbValue );
}

void CDatabase::SetType( const wchar_t* wcsValue, int cbValue )
{
	wcsncpy( m_wcsType, wcsValue, cbValue );
}
