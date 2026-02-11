// MemMappedFile.cpp: implementation of the CMemMappedFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemMappedFile.h"
#include "MailComponents.h"
#include "Utility.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemMappedFile::CMemMappedFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_hMapping = NULL;
	m_lpData = NULL;
	m_dwLength = 0;
}

CMemMappedFile::~CMemMappedFile()
{
	UnMap();
}

const BOOL CMemMappedFile::MapFile(const tstring &sFileName)
{
	if (!Init(sFileName))
		return FALSE;

	if (!MapHandle())
		return FALSE;

	return MapViewEntireFile();
}

const BOOL CMemMappedFile::MapHandle()
{
	// create the file mapping
	m_hMapping = ::CreateFileMapping(	m_hFile,			// handle to file
										NULL,				// security
										PAGE_READONLY,		// protection
										0,					// high-order DWORD of size
										m_dwLength,			// low-order DWORD of size
										NULL);				// object name
	if (m_hMapping == NULL)	
	{
		CreateLogEntry(	EMSERROR( EMS_LOG_SEVERITY_ERROR,EMSERR_MAIL_COMPONENTS,EMS_LOG_INBOUND_MESSAGING,ERROR_FILESYSTEM), 
			_T("CMemMappedFile::MapHandle() - Failed in call to CreateFileMapping, GetLastError returned %d (%s)"), 
			GetLastError(), CUtility::GetErrorString(GetLastError()).c_str() );
		UnMap();
		return FALSE;
	}

	return TRUE;
}

void CMemMappedFile::UnMap()
{
	// free the data pointer
	if (m_lpData != NULL)
	{
		FlushViewOfFile(m_lpData, 0);
		UnmapViewOfFile(m_lpData);
		m_lpData = NULL;
	}
	
	// remove the file mapping
	if (m_hMapping != NULL)
	{
		CloseHandle(m_hMapping);
		m_hMapping = NULL;
	}
	
	// close the file system file if its open
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	m_dwLength = 0;
}

const BOOL CMemMappedFile::Flush()
{
	if (m_lpData == NULL)
	{
		assert(0);
		return FALSE;
	}

	return FlushViewOfFile(m_lpData, 0);
}

const BOOL CMemMappedFile::MapViewEntireFile()
{
	// map a view of the file; this is where we get our
	// data pointer
	m_lpData = MapViewOfFile(	m_hMapping,					// handle to file-mapping object
								FILE_MAP_READ,				// access mode
								0,							// high-order DWORD of offset						
								0,							// low-order DWORD of offset
								0);							// number of bytes to map

	if (m_lpData == NULL)
		return FALSE;

	return TRUE;
}

const BOOL CMemMappedFile::Init(const tstring& sFileName)
{
	// open the source file
	m_hFile = CreateFile(	sFileName.c_str(), 
							GENERIC_READ, 
							0, 
							NULL, 
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		CreateLogEntry(	EMSERROR( EMS_LOG_SEVERITY_ERROR,EMSERR_MAIL_COMPONENTS,EMS_LOG_INBOUND_MESSAGING,ERROR_FILESYSTEM), 
			_T("CMemMappedFile::Init() - Failed in call to CreateFile, GetLastError returned %d (%s)"), 
			GetLastError(), CUtility::GetErrorString(GetLastError()).c_str() );
		UnMap();
		return FALSE;
	}
	
	// get the size of the file we are mapping
	DWORD dwFileSizeHigh = 0;
	m_dwLength = GetFileSize(m_hFile, &dwFileSizeHigh);
	if (m_dwLength == 0xFFFFFFFF)
	{
		CreateLogEntry(	EMSERROR( EMS_LOG_SEVERITY_ERROR,EMSERR_MAIL_COMPONENTS,EMS_LOG_INBOUND_MESSAGING,ERROR_FILESYSTEM), 
			_T("CMemMappedFile::Init() - Failed in call to GetFileSize, GetLastError returned %d (%s)"), 
			GetLastError(), CUtility::GetErrorString(GetLastError()).c_str() );
		UnMap();
		return FALSE;
	}
	
	// fail if file is greater than 4GB in size
	if (dwFileSizeHigh)
	{
		CreateLogEntry(	EMSERROR( EMS_LOG_SEVERITY_ERROR,EMSERR_MAIL_COMPONENTS,EMS_LOG_INBOUND_MESSAGING,ERROR_FILESYSTEM), 
			_T("CMemMappedFile::Init() -  File size is greater than 4GB") );
		UnMap();
		return FALSE;
	}
	
	// fail if file is 0 length in size
	if (dwFileSizeHigh == 0 && m_dwLength == 0)
	{
		CreateLogEntry(	EMSERROR( EMS_LOG_SEVERITY_ERROR,EMSERR_MAIL_COMPONENTS,EMS_LOG_INBOUND_MESSAGING,ERROR_FILESYSTEM), 
			_T("CMemMappedFile::Init() - File size is 0, not attempting to memory map the file") );
		UnMap();
		return FALSE;
	}

	return TRUE;
}

const BOOL CMemMappedFile::MapFileToString(const tstring &sFileName, DwString& dwString)
{
	if (!Init(sFileName))
		return FALSE;
	
	if (!MapHandle())
		return FALSE;

	// get allocation granularity
    SYSTEM_INFO systemInfo;
	ZeroMemory(&systemInfo, sizeof(SYSTEM_INFO));
    ::GetSystemInfo(&systemInfo);
	DWORD dw_allocationSize = (systemInfo.dwAllocationGranularity * 4);

	if (dw_allocationSize > m_dwLength)
		dw_allocationSize = 0;
	
	LARGE_INTEGER liPosition;
    liPosition.QuadPart = 0;
	DWORD dwRemaining = m_dwLength;
	while(m_lpData = MapViewOfFile(	m_hMapping,
									FILE_MAP_READ,	
									liPosition.HighPart,		
									liPosition.LowPart,	
									dw_allocationSize))
    {
		// append map segment to string object
		dwString.append((const char*)m_lpData, (dw_allocationSize == 0) ? dwRemaining : dw_allocationSize);

		// increment map section
        liPosition.QuadPart += dw_allocationSize;

		// unmap view
        if(!UnmapViewOfFile(m_lpData))
        {
			CreateLogEntry(	EMSERROR( EMS_LOG_SEVERITY_ERROR,EMSERR_MAIL_COMPONENTS,EMS_LOG_INBOUND_MESSAGING,ERROR_FILESYSTEM), 
				_T("CMemMappedFile::MapFileToString() - Failed in call to UnmapViewOfFile, GetLastError returned %d (%s)"), 
				GetLastError(), CUtility::GetErrorString(GetLastError()).c_str() );
            return FALSE;
        }

		if (dw_allocationSize == 0)
			break;

		dwRemaining -= dw_allocationSize;

		if (dw_allocationSize > dwRemaining)
			dw_allocationSize = 0;
    }

	m_lpData = NULL;

	return TRUE;
}
