/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/FileMap.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Memory mapped file class
||              
\\*************************************************************************/

#include "stdafx.h"
#include "FileMap.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFileMap::CFileMap()
:  m_hMap(NULL),
   m_pMap(NULL),
	m_dwSize(0)
{

}

CFileMap::~CFileMap()
{
	Close();
}

void CFileMap::Close()
{
	if(m_pMap != NULL)
	{
		UnmapViewOfFile((LPCVOID) m_pMap);
		m_pMap = NULL;
	}

	if(m_hMap != NULL)
	{
		CloseHandle(m_hMap);
		m_hMap = NULL;
	}

	m_dwSize = 0;
}

PBYTE CFileMap::Map(DWORD dwSize, DWORD dwProtect)
{
	Close();

	m_dwSize = dwSize;

	if ((m_hMap = ::CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, dwProtect, 0, dwSize, NULL)) == NULL)
	{
		CEMSString sError;
		sError.Format( _T("Error (%d) in CreateFileMapping\n"), GetLastError() );
		THROW_EMS_EXCEPTION( E_MemoryError, sError );
	}

	if ((m_pMap = reinterpret_cast<PBYTE>(::MapViewOfFile(m_hMap, FILE_MAP_WRITE, 0, 0, 0))) == NULL)
	{
		Close();
		CEMSString sError;
		sError.Format( _T("Error (%d) in MapViewOfFile\n"), GetLastError() );
		THROW_EMS_EXCEPTION( E_MemoryError, sError );
	}
		 
	return m_pMap;
}

