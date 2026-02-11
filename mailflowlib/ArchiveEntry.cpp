// ArchiveEntry.cpp: implementation of the CArchiveEntry class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveEntry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArchiveEntry::CArchiveEntry()
{

}

CArchiveEntry::~CArchiveEntry()
{

}

void CArchiveEntry::LoadFromDB(CODBCQuery &query, const int nID)
{
	//assert(nID > 0);

	m_ArchiveID = nID;

	Query(query);
}
