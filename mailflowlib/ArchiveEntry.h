// ArchiveEntry.h: interface for the CArchiveEntry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEENTRY_H__3E9DA150_83FB_4625_BBB0_F11B3D78B0FB__INCLUDED_)
#define AFX_ARCHIVEENTRY_H__3E9DA150_83FB_4625_BBB0_F11B3D78B0FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "QueryClasses.h"

class CArchiveEntry : public TArchives  
{
public:
	void LoadFromDB(CODBCQuery& query, const int nID);
	CArchiveEntry();
	virtual ~CArchiveEntry();

};

#endif // !defined(AFX_ARCHIVEENTRY_H__3E9DA150_83FB_4625_BBB0_F11B3D78B0FB__INCLUDED_)
