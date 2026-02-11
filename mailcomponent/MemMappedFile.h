// MemMappedFile.h: interface for the CMemMappedFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMMAPPEDFILE_H__82EE7789_59C2_4B3C_8EF9_B109F1C4F9B5__INCLUDED_)
#define AFX_MEMMAPPEDFILE_H__82EE7789_59C2_4B3C_8EF9_B109F1C4F9B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mimepp/mimepp.h>

class CMemMappedFile  
{
public:
	CMemMappedFile();
	virtual ~CMemMappedFile();

public:
	const BOOL MapFileToString(const tstring &sFileName, DwString& dwString);
	const BOOL Flush();
	const BOOL MapFile(const tstring& sFileName);
	void UnMap();
	const DWORD GetLength()								{ return m_dwLength; }
	const HANDLE GetFileMappingHandle()					{ return m_hMapping; }
	const HANDLE GetFileHandle()						{ return m_hFile; }

	LPVOID  m_lpData;

protected:
	const BOOL MapHandle();
	HANDLE  m_hFile;
	HANDLE  m_hMapping;
	tstring m_sMappingName;
	DWORD   m_dwLength;
private:
	const BOOL Init(const tstring& sFileName);
	const BOOL MapViewEntireFile();
};

#endif // !defined(AFX_MEMMAPPEDFILE_H__82EE7789_59C2_4B3C_8EF9_B109F1C4F9B5__INCLUDED_)
