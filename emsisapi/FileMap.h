// UploadParser.h: interface for the CUploadParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPLOADPARSER_H__9C45566C_B9D3_43AD_B3A3_A44522986FDA__INCLUDED_)
#define AFX_UPLOADPARSER_H__9C45566C_B9D3_43AD_B3A3_A44522986FDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFilePart
{
public:
	TCHAR m_szFieldName[MAX_PATH];
	TCHAR m_szFileName[MAX_PATH];
	TCHAR m_szContentType[MAX_PATH];
	TCHAR m_szContentDisposition[MAX_PATH];
	unsigned int m_nOffset;
};


class CFileMap  
{
public:
	CFileMap();
	~CFileMap();

	PBYTE		Map(DWORD dwSize, DWORD dwProtect = PAGE_READWRITE);
	void		Close();

	PBYTE		GetBuffer()		{ return m_pMap; }
	DWORD		GetLength()		{ return m_dwSize; }

protected:

	HANDLE				m_hMap;
	PBYTE				m_pMap;
	DWORD				m_dwSize;
};


#endif // !defined(AFX_UPLOADPARSER_H__9C45566C_B9D3_43AD_B3A3_A44522986FDA__INCLUDED_)
