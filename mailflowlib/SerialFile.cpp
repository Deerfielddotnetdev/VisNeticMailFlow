// SerialFile.cpp: implementation of the CSerialFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "VMFDBMaint.h"
#include "SerialFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerialFile::CSerialFile()
:	m_hArcFile(INVALID_HANDLE_VALUE)
{

}

CSerialFile::~CSerialFile()
{
	CloseArchive();
}

int CSerialFile::OpenArchive(const tstring &sArcFileName, const int nDir)
{
	//assert(sArcFileName.size() != 0);
	if (sArcFileName.size() == 0)
		return -1;

	m_sArcFileName = sArcFileName;

	m_hArcFile = CreateFile(sArcFileName.c_str(),
							(nDir == serializer_dir_in) ? GENERIC_READ : GENERIC_WRITE,
							0,						// no sharing
							NULL,					// no security
							(nDir == serializer_dir_in) ? OPEN_EXISTING : CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,	// normal file
							NULL);

	if (m_hArcFile == INVALID_HANDLE_VALUE)
	{
		//assert(0);
		return -1;
	}

	return 0; // success
}

void CSerialFile::CloseArchive()
{
	if (m_hArcFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hArcFile);
		m_hArcFile = INVALID_HANDLE_VALUE;
	}
}
