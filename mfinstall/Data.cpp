// Data.cpp: implementation of the CData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Data.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CData::CData()
{
	memset( m_wcsColumns, 0, sizeof( m_wcsColumns ) );
	memset( m_wcsValues, 0, sizeof( m_wcsValues ) );
	memset( m_wcsConstantName, 0, sizeof( m_wcsConstantName ) );

}

CData::~CData()
{

}


void CData::SetColumns( const wchar_t* wcsValue, int cbValue )
{
	wcsncat( m_wcsColumns, wcsValue, cbValue );	
}

void CData::SetValues( const wchar_t* wcsValue, int cbValue )
{
	wcsncat( m_wcsValues, wcsValue, cbValue );	
}

void CData::SetConstantName( const wchar_t* wcsValue, int cbValue )
{
	wcsncat( m_wcsConstantName, wcsValue, cbValue );	
}
