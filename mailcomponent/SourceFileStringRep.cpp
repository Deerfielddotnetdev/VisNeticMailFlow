// SourceFileStringRep.cpp: implementation of the CSourceFileStringRep class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SourceFileStringRep.h"
#include "MailComponents.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSourceFileStringRep::CSourceFileStringRep()
{
	// A kludge!  mBuffer is allocated by DwStringRep::DwStringRep()
	// and freed by DwStringRep::~DwStringRep().  The fix is to
	// make both this class and DwStringRep derive from a common
    // base class.
    mBufferSave = mBuffer;
    mBuffer = 0;
}

CSourceFileStringRep::~CSourceFileStringRep()
{
	m_mappedFile.UnMap();
	
	// A kludge!  mBuffer is allocated by DwStringRep::DwStringRep()
	// and freed by DwStringRep::~DwStringRep().  The fix is to
	// make both this class and DwStringRep derive from a common
    // base class.
    mBuffer = mBufferSave;
}

const int CSourceFileStringRep::Open(const tstring &sFileName)
{
	assert(sFileName.size() > 0);
	if (sFileName.size() == 0)
		return ERROR_BAD_PARAMETER;

	if (!m_mappedFile.MapFile(sFileName))
	{
		LINETRACE(_T("Failed to map file %s\n"), sFileName.c_str());
		assert(0);
		return ERROR_FILESYSTEM;
	}

	mBuffer	= (char*)m_mappedFile.m_lpData;

	mSize = m_mappedFile.GetLength();

	return 0;
}
