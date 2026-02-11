// TempFile.cpp: implementation of the CTempFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TempFile.h"
#include "Utility.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTempFile::CTempFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
}

CTempFile::~CTempFile()
{
	Close();
}

const int CTempFile::CreateTempFile(LPCTSTR szPrefix)
{
	
	// generate & open a temporary file
	TCHAR szTmp[_MAX_PATH];
	DWORD dwRet;

	LINETRACE(_T("* MailComponents::CTempFile::CTempFile - Attempting to generate and open a temporary file."));

	//*************************************************************************
	// Added by Mark Mohr on February 5, 2003.
	// Fixes issue with McAfee Virus Scanner hijacking message from windows temp dir.
	// Will try and use the MailFlow temp directory first.

	HKEY hKey = 0;
	tstring sVMFTempPath;

	LINETRACE(_T("* MailComponents::CTempFile::CTempFile - Opening [ SOFTWARE\\Deerfield.com\\VisNetic MailFlow\\MsgComponent ] registry key."));

	// Open registry key
	long lRetVal = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		                          _T("SOFTWARE\\Deerfield.com\\VisNetic MailFlow\\MsgComponent"),
								  0,
								  KEY_ALL_ACCESS,
								  &hKey);

	// Was it successfull
	if(lRetVal == ERROR_SUCCESS)
	{
		LINETRACE(_T("* MailComponents::CTempFile::CTempFile - Succesfully opened [ SOFTWARE\\Deerfield.com\\VisNetic MailFlow\\MsgComponent ] registry key."));

		// Yes then declare are variables
		TCHAR sFile[_MAX_PATH + 1];
		DWORD dwType;
		DWORD dwSize = _MAX_PATH;

		// Clear buffer
		ZeroMemory(sFile,_MAX_PATH + 1);

		// Get Temp Folder
		lRetVal = ::RegQueryValueEx(hKey,_T("TempFolder"),0,&dwType,(BYTE*)sFile,&dwSize);

		// Was it successfull
		if(lRetVal == ERROR_SUCCESS)
		{
			LINETRACE(_T("* MailComponents::CTempFile::CTempFile - Queried Value TempFolder and got the value [ %s ]"), sFile);

			// If yes then copy it to tstring
			sVMFTempPath = sFile;

			// whackify folder-location if needed
			if (sVMFTempPath.length() > 0 && sVMFTempPath.at(sVMFTempPath.size()-1) != '\\')
				sVMFTempPath += '\\';

			CUtility util;
			util.EnsurePathExists(sVMFTempPath.c_str());
		}
		else
		{
			LINETRACE(_T("* MailComponents::CTempFile::CTempFile - Could not Query Value TempFolder probably does not exist [ %d ]"),lRetVal);

			dwRet = ::GetTempPath(_MAX_PATH, szTmp);
			if (dwRet == 0)
			{
				LINETRACE(_T("GetTempPath call failed: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
				assert(0);
				return ERROR_FILESYSTEM;
			}

			// Else it was not successful user window temp
			sVMFTempPath = szTmp;
		}

		// Close registry key
		::RegCloseKey(hKey);
	}
	else
	{
		dwRet = ::GetTempPath(_MAX_PATH, szTmp);
		if (dwRet == 0)
		{
			LINETRACE(_T("* MailComponents::CTempFile::CTempFile - GetTempPath call failed: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
			assert(0);
			return ERROR_FILESYSTEM;
		}

		// Else it was not successful use windows temp
		sVMFTempPath = szTmp;
	}

	dwRet = ::GetTempFileName(sVMFTempPath.c_str(), szPrefix, 0, szTmp);
	if (dwRet == 0)
	{
		LINETRACE(_T("* MailComponents::CTempFile::CTempFile - GetTempFileName call failed: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
		assert(0);
		return ERROR_FILESYSTEM;
	}

	//*************************************************************************

	// store full temp path
	m_sFullPath = szTmp;

	// open the source file
	m_hFile = CreateFile(	szTmp, 
							GENERIC_WRITE|GENERIC_READ, 
							0, 
							NULL, 
							CREATE_ALWAYS, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		LINETRACE(_T("* MailComponents::CTempFile::CTempFile - CreateFile call failed: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
		assert(0);
		return ERROR_FILESYSTEM;
	}

	return 0;
}


void CTempFile::Close()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

BOOL CTempFile::Delete()
{
	// file must be closed in order to delete
	Close();

	return DeleteFile(m_sFullPath.c_str());
}
