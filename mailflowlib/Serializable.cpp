// Serializable.cpp: implementation of the CSerializable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Serializable.h"
#include "SerialFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerializable::CSerializable()
{

}

CSerializable::~CSerializable()
{
}

// serializes textual data
void CSerializable::PutText(CSerialFile& sfile, const TCHAR *pszText, const long nLength)
{
	////assert(sfile.m_hArcFile != INVALID_HANDLE_VALUE);

	DWORD dwWritten = 0;
	BOOL b = FALSE;

	PutValue(sfile, nLength);

	if (nLength == 0)
		return;

	// write out the text
	b = WriteFile(	sfile.m_hArcFile,
					(LPVOID)pszText,
					nLength,
					&dwWritten,
					NULL);
	if (!b)
	{
		////assert(0);
		throw CSerializeException(GetLastError(), __LINE__, __FILE__);
	}
}

// reads in the next text field from the archive file;
// note that memory is allocated automatically (to be freed by caller);
// function fills caller-supplied long parameter with the size of the allocated field
void CSerializable::GetText(CSerialFile& sfile, TCHAR*& ppszText, long& lLength)
{
	//assert(sfile.m_hArcFile != INVALID_HANDLE_VALUE);

	DWORD dwRead = 0;
	lLength = 0;
	ppszText = NULL;

	GetValue(sfile, lLength);

	// if zero-length, allocate empty string
	if (lLength == 0)
	{
		lLength = sizeof(TCHAR);
		ppszText = (TCHAR*)malloc(lLength);
		ppszText[0] = _T('\0');
		return;
	}

	// allocate the memory
	ppszText = (TCHAR*)calloc(lLength + sizeof(TCHAR), 1);
	//assert(ppszText);
	if (ppszText)
	{
		// read the text into the allocated buffer
		BOOL b = ReadFile(	sfile.m_hArcFile,
							(LPVOID)ppszText,
							lLength,
							&dwRead,
							NULL);
		//assert(dwRead == lLength);
		if (!b)
		{
			//assert(0);
			throw CSerializeException(GetLastError(), __LINE__, __FILE__);
		}
	}
}

// adds a file to an archive (in chunks)
void CSerializable::PutFile(CSerialFile &sfile, const tstring &sFileName)
{
	// open the source file for reading
	HANDLE hFile = CreateFile(	sFileName.c_str(), 
								GENERIC_READ, 
								0, 
								NULL, 
								OPEN_EXISTING, 
								FILE_ATTRIBUTE_NORMAL, 
								NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		// if this fails, mark the file size as 0 and bail
		PutValue(sfile, 0);
		throw CSerializeException(GetLastError(), __LINE__, __FILE__);
	}

	// get the attachment file size
	DWORD dwFileSizeHigh = 0;
	DWORD dwSize = GetFileSize(hFile, &dwFileSizeHigh);
	if ((dwSize == INVALID_FILE_SIZE) || dwFileSizeHigh)
	{
		PutValue(sfile, 0);
		CloseHandle(hFile);
		throw CSerializeException(GetLastError(), __LINE__, __FILE__);
	}

	// write attachment file size to the archive
	try
	{
		PutValue(sfile, dwSize);
	}
	catch (CSerializeException sx)
	{
		CloseHandle(hFile);
		throw sx;
	}	

	// 32k chunks
	const int nChunkSize = (32*1024);
	char buffer[nChunkSize];
	DWORD dwRead, dwWritten;
	while (1)
	{
		// read from original file
		if (!ReadFile(hFile, buffer, nChunkSize, &dwRead, NULL))
		{
			CloseHandle(hFile);
			//assert(0);
			throw CSerializeException(GetLastError(), __LINE__, __FILE__);
		}

		// done yet?
		if (dwRead == 0)
			break;

		// write to the archive file
		if (!WriteFile(sfile.m_hArcFile, buffer, dwRead, &dwWritten, NULL))
		{
			CloseHandle(hFile);
			//assert(0);
			throw CSerializeException(GetLastError(), __LINE__, __FILE__);
		}
	}

	CloseHandle(hFile);
}

void CSerializable::GetFile(CSerialFile &sfile, const tstring &sFileName)
{
	// create the destination file
	HANDLE hFile = CreateFile(	sFileName.c_str(), 
								GENERIC_WRITE, 
								0, 
								NULL, 
								CREATE_ALWAYS, 
								FILE_ATTRIBUTE_NORMAL, 
								NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//assert(0);
		throw CSerializeException(GetLastError(), __LINE__, __FILE__);
	}

	int nFileSize;
	try
	{
		GetValue(sfile, nFileSize);
		//assert(nFileSize >= 0);
	}
	catch (CSerializeException sx)
	{
		CloseHandle(hFile);
		throw sx;
	}
	
	// 32k chunks
	const int nChunkSize = (32*1024);
	char buffer[nChunkSize];
	DWORD dwRead, dwWritten;
	int nLeft = nFileSize;
	while (nLeft > 0)
	{
		int nToRead = (nLeft > nChunkSize) ? nChunkSize : nLeft;

		// read from the archive file
		if (!ReadFile(sfile.m_hArcFile, buffer, nToRead, &dwRead, NULL))
		{
			CloseHandle(hFile);
			//assert(0);
			throw CSerializeException(GetLastError(), __LINE__, __FILE__);
		}

		//assert(nToRead == dwRead);

		// write to the destination file
		if (!WriteFile(hFile, buffer, dwRead, &dwWritten, NULL))
		{
			CloseHandle(hFile);
			//assert(0);
			throw CSerializeException(GetLastError(), __LINE__, __FILE__);
		}

		nLeft -= dwRead;
	}

	CloseHandle(hFile);
}

void CSerializable::PutVarChar(CSerialFile &sfile, const TCHAR *value, int nlen)
{
	DWORD dwWritten = 0;
	BOOL b = WriteFile(	sfile.m_hArcFile,
						(LPVOID)value,
						nlen,
						&dwWritten,
						NULL);
	if (!b)
	{
		//assert(0);
		throw CSerializeException(GetLastError(), __LINE__, __FILE__);
	}
}

void CSerializable::GetVarChar(CSerialFile &sfile, TCHAR *value, int nlen)
{
	DWORD dwRead = 0;
	BOOL b = ReadFile(	sfile.m_hArcFile,
						(LPVOID)value,
						nlen,
						&dwRead,
						NULL);
	if (!b)
	{
		//assert(0);
		throw CSerializeException(GetLastError(), __LINE__, __FILE__);
	}
}
