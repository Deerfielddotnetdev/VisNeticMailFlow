// Utility.cpp: implementation of the CUtility class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utility.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUtility::CUtility()
{

}

CUtility::~CUtility()
{

}

const bool CUtility::EnsurePathExists(tstring sPath)
{
	DWORD attr;
	int pos;
	bool bRes = true;
	
	// Check for trailing slash:
	pos = sPath.find_last_of(_T("\\"));
	if (sPath.length() == pos + 1)	// last character is "\"
	{
		sPath.resize(pos);
	}
	
	// look for existing path part
	attr = GetFileAttributes(sPath.c_str());
	
	// if it doesn't exist
	if (0xFFFFFFFF == attr)
	{
		pos = sPath.find_last_of(_T("\\"));
		if (0 < pos)
		{
			// create parent dirs
			bRes = EnsurePathExists(sPath.substr(0, pos));
		}
		
		// create note
		bRes = bRes && CreateDirectory(sPath.c_str(), NULL);
	}
	else if (!(FILE_ATTRIBUTE_DIRECTORY & attr))
	{	
		// object already exists, but is not a dir
		SetLastError(ERROR_FILE_EXISTS);
		bRes = false;
	}
	
	return bRes;
}

tstring CUtility::GetErrorString(const DWORD dwErr)
{
	// buffer
	TCHAR* lpszTemp = NULL;
	tstring sTemp = _T("");
	
	// Let the system allocate a string to describe the error code in dwRet
    FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER |	// specifies that the lpBuffer parameter is a pointer to a PVOID pointer, and that the nSize parameter specifies the minimum number of TCHARs to allocate for an output message buffer
					FORMAT_MESSAGE_FROM_SYSTEM |		// specifies that the function should search the system message-table resource(s) for the requested message
					FORMAT_MESSAGE_ARGUMENT_ARRAY,		// specifies that the Arguments parameter is not a va_list structure, but instead is just a pointer to an array of values that represent the arguments
					NULL,								// message source
					dwErr,								// message identifier					
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 	// default language
					(LPTSTR)&lpszTemp,					// message buffer					
					0,									// maximum size of message buffer
					NULL );								// array of message inserts	
	
    if (lpszTemp)
	{
		// copy to our string buff and free memory
		sTemp = lpszTemp;
        LocalFree((HLOCAL)lpszTemp);
	}
	
    return sTemp;
}