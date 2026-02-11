// Serializable.h: interface for the CSerializable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIALIZER_H__D5ED8DBD_5F86_4646_8637_2E366360887D__INCLUDED_)
#define AFX_SERIALIZER_H__D5ED8DBD_5F86_4646_8637_2E366360887D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SerializeException.h"
#include "QueryClasses.h"

class CSerialFile;

class CSerializable  
{
public:
	CSerializable();
	virtual ~CSerializable();

	// get var-length character arrays (no memory allocation) from archive
	void GetVarChar(CSerialFile& sfile, TCHAR* value, int nlen);
	// put var-length character arrays into archive
	void PutVarChar(CSerialFile& sfile, const TCHAR* value, int nlen);

	// get file from the archive
	void GetFile(CSerialFile& sfile, const tstring& sFileName);
	// insert file into the archive
	void PutFile(CSerialFile& sfile, const tstring& sFileName);

	// put var-length TEXT field into the archive
	void PutText(CSerialFile& sfile, const TCHAR* pszText, const long nLength);
	// get var-length TEXT field from the archive (allocates memory)
	void GetText(CSerialFile& sfile, TCHAR*& ppszText, long& lLength);

	// this template function can effectively serialize values
	// when the size can be determined via the sizeof operator;
	// this includes numbers, structs, etc.
	template <class T>
	void PutValue(CSerialFile& sfile, const T value)
	{
		//assert(sfile.m_hArcFile != INVALID_HANDLE_VALUE);

		DWORD dwWritten = 0;
		BOOL b = WriteFile(	sfile.m_hArcFile,
							(LPVOID)&value,
							sizeof(T),
							&dwWritten,
							NULL);
		if (!b)
		{
			assert(0);
			throw CSerializeException(GetLastError(), __LINE__, __FILE__);
		}
	}

	// this template function can effectively serialize values
	// when the size can be determined via the sizeof operator;
	// this includes numbers, structs, etc.
	template <class T>
	void GetValue(CSerialFile& sfile, T& value)
	{
		//assert(sfile.m_hArcFile != INVALID_HANDLE_VALUE);

		DWORD dwRead = 0;
		BOOL b = ReadFile(	sfile.m_hArcFile,
							(LPVOID)&value,
							sizeof(T),
							&dwRead,
							NULL);
		if (!b)
		{
			//assert(0);
			throw CSerializeException(GetLastError(), __LINE__, __FILE__);
		}
	}

};

#endif // !defined(AFX_SERIALIZER_H__D5ED8DBD_5F86_4646_8637_2E366360887D__INCLUDED_)
