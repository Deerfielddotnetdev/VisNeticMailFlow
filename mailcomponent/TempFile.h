// TempFile.h: interface for the CTempFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEMPFILE_H__3494DA2C_5CE9_4C65_8BAA_4D569FB7FA3A__INCLUDED_)
#define AFX_TEMPFILE_H__3494DA2C_5CE9_4C65_8BAA_4D569FB7FA3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTempFile  
{
public:
	BOOL Delete();
	void Close();
	const int CreateTempFile(LPCTSTR szPrefix);
	CTempFile();
	virtual ~CTempFile();

	tstring& GetFilePath()			{ return m_sFullPath; }
	HANDLE GetFileHandle()			{ return m_hFile; }

private:
	HANDLE m_hFile;
	tstring m_sFullPath;

};

#endif // !defined(AFX_TEMPFILE_H__3494DA2C_5CE9_4C65_8BAA_4D569FB7FA3A__INCLUDED_)
