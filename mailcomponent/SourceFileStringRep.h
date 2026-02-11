// SourceFileStringRep.h: interface for the CSourceFileStringRep class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOURCEFILESTRINGREP_H__307898FB_2453_41D9_B0E4_C7F20116313C__INCLUDED_)
#define AFX_SOURCEFILESTRINGREP_H__307898FB_2453_41D9_B0E4_C7F20116313C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mimepp/mimepp.h>
#include "MemMappedFile.h"

class CSourceFileStringRep : public DwStringRep  
{
public:
	const int Open(const tstring& sFileName);
	CSourceFileStringRep();
	virtual ~CSourceFileStringRep();

private:
	char* mBufferSave;
	CMemMappedFile m_mappedFile;

};

#endif // !defined(AFX_SOURCEFILESTRINGREP_H__307898FB_2453_41D9_B0E4_C7F20116313C__INCLUDED_)
