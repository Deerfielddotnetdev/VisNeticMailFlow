// VirusScanner.cpp: implementation of the CVirusScanner class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VirusScanner.h"
#include "MailComponents.h"
#include "Utility.h"
#include "MessageIO.h"
#include "CriticalSection.h"
#include "SingleCrit.h"
#include "ServerParameters.h"
#include "AVPKeyStore.h"
#include "DCIKey.h"

extern dca::Mutex g_csFileIO_1;

// ----------------------------------------------------------------------------- //
// constructor
// ----------------------------------------------------------------------------- //
// important: the Initialize() function should always be called next as part 
// of a two-phase contruction of this object;
// ----------------------------------------------------------------------------- //
CVirusScanner::CVirusScanner()
{
	m_pAVPAntivirus = 0;

	m_bIsInitialized = false;
}

// ----------------------------------------------------------------------------- //
// destructor
// ----------------------------------------------------------------------------- //
// note: you can either call UnInitialize() directly, or the destructor
// will do it for you
// ----------------------------------------------------------------------------- //
CVirusScanner::~CVirusScanner()
{
	if (m_bIsInitialized)
		UnInitialize();
}

// ----------------------------------------------------------------------------- //
// Potential result codes from this function:
// ----------------------------------------------------------------------------- //
// TRUE						= success
// FALSE					= failure
// ----------------------------------------------------------------------------- //
const BOOL CVirusScanner::Initialize()
{
	if (m_bIsInitialized)
		return FALSE;

	// get a pointer to antivirus interface
	HRESULT hr = CoCreateInstance(	CLSID_AVPAntivirus,
									NULL,
									CLSCTX_LOCAL_SERVER,
									IID_IAVPAntivirus,
									(void**)&m_pAVPAntivirus );
	if (!SUCCEEDED(hr)) 
		return FALSE;

	hr = m_pAVPAntivirus->Initialize(0, 0, 0);
	if (!SUCCEEDED(hr)) 
	{
		m_pAVPAntivirus->Release();
		return FALSE;
	}

	m_bIsInitialized = true;
	
	return TRUE; // success
}

// ----------------------------------------------------------------------------- //
// Automatically called by destructor; can also be called directly
// ----------------------------------------------------------------------------- //
void CVirusScanner::UnInitialize()
{
	if (!m_bIsInitialized)
		return;

	m_pAVPAntivirus->Uninitialize();

	m_pAVPAntivirus->Release();

	m_bIsInitialized = false;
}

// ----------------------------------------------------------------------------- //
// Potential result codes from this function:
// ----------------------------------------------------------------------------- //
// AV_FILE_IS_CLEAN			- file clean (no infection)
// AV_FILE_IS_INFECTED		- file is infected
// AV_FILE_IS_SUSPICIOUS	- file is suspicious
// AV_FILE_IS_NONSCANNED	- file wasn't scanned
// AV_UNEXPECTED_FAILURE	- other unexpected failure
// ----------------------------------------------------------------------------- //
const UINT CVirusScanner::ScanFile(const tstring& sFilePath, tstring& sVirusName, tstring& sErrorString)
{
	AVRESULT avres;
	HRESULT hr;
	BSTR bstrVirusName = 0;
	BSTR bstrFilePath = SysAllocString(sFilePath.c_str());
	USES_CONVERSION;
	
	hr = m_pAVPAntivirus->ScanFile(	0,					// default scenario
									bstrFilePath,		// file path
									&avres,				// result placeholder
									&bstrVirusName );	// virus name
	if (!SUCCEEDED(hr)) 
	{
		BSTR pbstrErrorInfo = NULL;
		m_pAVPAntivirus->GetErrorInfo(hr, &pbstrErrorInfo);
		LINETRACE(_T("ScanFile() failed: %s\n"), pbstrErrorInfo);
		sErrorString = OLE2T(pbstrErrorInfo);

		SysFreeString(bstrFilePath);
		SysFreeString(pbstrErrorInfo);

		assert(0);

		return AV_UNEXPECTED_FAILURE;
	}

	sVirusName = OLE2T(bstrVirusName);
	SysFreeString(bstrFilePath);

	return avres;
}

// ----------------------------------------------------------------------------- //
// Potential result codes from this function:
// ----------------------------------------------------------------------------- //
// AV_FILE_IS_CLEAN			= file clean (no infection)
// AV_FILE_IS_INFECTED		= file is infected (cleaning failed)
// AV_FILE_IS_CLEANED		= file was infected and cleaned
// AV_FILE_IS_SUSPICIOUS	= file is suspicious (cleaning failed)
// AV_FILE_IS_NONSCANNED	= file wasn't scanned
// AV_UNEXPECTED_FAILURE	= other unexpected failure
// ----------------------------------------------------------------------------- //
const UINT CVirusScanner::ScanFile_Clean_Delete(const tstring &sFilePath, tstring &sVirusName, tstring& sErrorString)
{
	AVRESULT avres;
	HRESULT hr;
	BSTR bstrVirusName = NULL;
	BSTR bstrFilePath = SysAllocString(sFilePath.c_str());
	USES_CONVERSION;
	
	hr = m_pAVPAntivirus->ScanAndCleanFile(	NULL,				// default scenario
											bstrFilePath,		// file path
											bstrFilePath,		// clean file path
											&avres,				// result placeholder
											&bstrVirusName );	// virus name

	// scan process failed entirely
	if (!SUCCEEDED(hr)) 
	{
		BSTR pbstrErrorInfo = NULL;
		m_pAVPAntivirus->GetErrorInfo(hr, &pbstrErrorInfo);
		LINETRACE(_T("Error during ScanAndCleanFile(): %s\n"), pbstrErrorInfo);
		sErrorString = OLE2T(pbstrErrorInfo);
		assert(0);

		SysFreeString(pbstrErrorInfo);
		SysFreeString(bstrFilePath);

		return AV_UNEXPECTED_FAILURE;
	}

	sVirusName = OLE2T(bstrVirusName);
	SysFreeString(bstrVirusName);
	SysFreeString(bstrFilePath);

	// scan succeeded, but file is still infected after cleaning attempt
	if ((avres == AV_FILE_IS_INFECTED) ||
		(avres == AV_FILE_IS_SUSPICIOUS))
	{
		// try to delete the infected file
		UINT nRes = RemoveFile(sFilePath.c_str());
		if (nRes != 0)
		{
			LINETRACE(_T("Failed to remove original file after virus cleaning attempt (code %d)\n"), nRes);
			assert(0);
			// avres will be returned below
		}
		else
		{
			LINETRACE(_T("Infected file (%s) deleted\n"), sFilePath.c_str());	
			return AV_FILE_IS_DELETED;
		}
	}
	
	return avres;	
}

// ----------------------------------------------------------------------------- //
// Potential result codes from this function:
// ----------------------------------------------------------------------------- //
// AV_FILE_IS_CLEAN			= file clean (no infection)
// AV_FILE_IS_INFECTED		= file is infected (quarantine failed)
// AV_FILE_IS_QUARANTINED	= file was infected and quarantined
// AV_FILE_IS_SUSPICIOUS	= file is suspicious (quarantine failed)
// AV_FILE_IS_NONSCANNED	= file wasn't scanned
// AV_UNEXPECTED_FAILURE	= other unexpected failure
// ----------------------------------------------------------------------------- //
const UINT CVirusScanner::ScanFile_Quarantine_Delete(const tstring &sFilePath, tstring &sVirusName, tstring& sQuarantinePath, tstring& sErrorString)
{
	UINT avres = ScanFile(sFilePath, sVirusName, sErrorString);

	if ((avres == AV_FILE_IS_INFECTED) ||
		(avres == AV_FILE_IS_SUSPICIOUS))
	{
		// if the file is either infected or suspicious, we'll quarantine it
		UINT nRet = QuarantineFile(sFilePath, sQuarantinePath);
		if (nRet != 0)
		{
			// quarantine failed
			LINETRACE(_T("Infected file quarantine failed: %d\n"), nRet);
			assert(0);

			// the quarantine moved failed, so try to *delete* the infected file
			UINT nRes = RemoveFile(sFilePath.c_str());
			if (nRes != 0)
			{
				LINETRACE(_T("Failed to remove original file after quarantine failure (code %d)\n"), nRes);
				assert(0);
				// avres is passed to caller
			}
			else
			{
				// delete succeeded
				LINETRACE(_T("Infected file (%s) deleted\n"), sFilePath.c_str());
				return AV_FILE_IS_DELETED;
			}
		}
		else
		{
			// quarantine succeeded
			LINETRACE(_T("Infected file (%s) quarantined to: %s\n"), sFilePath.c_str(), sQuarantinePath.c_str());
			return AV_FILE_IS_QUARANTINED;
		}
	}

	return avres;
}

// ----------------------------------------------------------------------------- //
// Potential result codes from this function:
// ----------------------------------------------------------------------------- //
// 0						= success
// anything else			= failure
// ----------------------------------------------------------------------------- //
const UINT CVirusScanner::QuarantineFile(const tstring &sFilePath, tstring& sQuarantinePath)
{
	TCHAR file[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	_tsplitpath(sFilePath.c_str(), NULL, NULL, file, ext);
	tstring sFileName = (tstring)file + (tstring)ext;

	dca::Lock lock(g_csFileIO_1);

	CMessageIO msgIO;
	BOOL bWorked = msgIO.GetQuarantinePath(sFileName, sQuarantinePath, 1);
	if (!bWorked)
	{
		// An error occurred during message virus scanning (%s) - [Failed to retrieve quarantine path]
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_VIRUS_SCANNING,
									ERROR_FILESYSTEM),
						EMS_STRING_ERROR_AV_QUARANTINE_PATH_RETRIEVAL,
						sFilePath.c_str());

		return ERROR_FILESYSTEM;
	}



	// ensure that the quarantine directory path exists
	TCHAR dir[_MAX_DIR];
	TCHAR drive[_MAX_DRIVE];
	_tsplitpath(sQuarantinePath.c_str(), drive, dir, NULL, NULL);
	if (!CUtility::EnsurePathExists(tstring(drive) + tstring(dir)))
		assert(0); // next file operation call will fail

	// move the file to quarantine
	BOOL bMoved = MoveFile(sFilePath.c_str(), sQuarantinePath.c_str());
	if (!bMoved)
	{
		// An error occurred during message virus scanning (%s) - [MoveFile() failed error (%d)]
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_VIRUS_SCANNING,
									ERROR_FILESYSTEM),
						EMS_STRING_ERROR_AV_MOVEFILE,
						sFilePath.c_str(),
						GetLastError());

		return ERROR_FILESYSTEM;
	}

	return 0; // success
}

// ----------------------------------------------------------------------------- //
// Potential result codes from this function:
// ----------------------------------------------------------------------------- //
// TRUE						= success (string references will be populated)
// FALSE					= failure
// ----------------------------------------------------------------------------- //
const BOOL CVirusScanner::GetVersionInfo(tstring& sEngineVer,		// engine version
										 tstring& sEngineDate,		// engine date
										 tstring& sDatabaseCount,	// virus database count
										 tstring& sDatabaseDate)	// last database update
{
    BSTR bstrEngineVersion		= NULL;
	BSTR bstrEngineDate			= NULL;
	BSTR bstrDataBaseRecCount	= NULL;
	BSTR bstrDataBaseCurDate	= NULL;
	
	HRESULT hr = m_pAVPAntivirus->GetVersions(	&bstrEngineVersion,
												&bstrEngineDate,
												&bstrDataBaseRecCount,
												&bstrDataBaseCurDate );
	if (!SUCCEEDED(hr)) 
	{
		BSTR pbstrErrorInfo = NULL;
		m_pAVPAntivirus->GetErrorInfo(hr, &pbstrErrorInfo);
		LINETRACE(_T("%s\n"), pbstrErrorInfo);
		assert(0);

		SysFreeString(pbstrErrorInfo);

		return FALSE;
	}

	USES_CONVERSION;
	sEngineVer = OLE2T(bstrEngineVersion);
	sEngineDate = OLE2T(bstrEngineDate);
	sDatabaseCount = OLE2T(bstrDataBaseRecCount);
	sDatabaseDate = OLE2T(bstrDataBaseCurDate);

	SysFreeString(bstrEngineVersion);
	SysFreeString(bstrEngineDate);
	SysFreeString(bstrDataBaseRecCount);
	SysFreeString(bstrDataBaseCurDate);

	return TRUE;
}

const BOOL CVirusScanner::GetLicenseInfo(tstring &sExpiration, BOOL &bValid)
{
	BSTR bstrExpiration = NULL;
	HRESULT hr = m_pAVPAntivirus->GetLicenceInfo(&bstrExpiration, &bValid);
	if (!SUCCEEDED(hr)) 
	{
		assert(0);
		return FALSE;
	}

	USES_CONVERSION;
	sExpiration = OLE2T(bstrExpiration);

	SysFreeString(bstrExpiration);
	
	return TRUE;
}

const BOOL CVirusScanner::ReloadDatabases()
{
	HRESULT hr = m_pAVPAntivirus->ReloadDatabases(0);
	if (!SUCCEEDED(hr)) 
	{
		assert(0);
		return FALSE;
	}

	return TRUE;
}

const UINT CVirusScanner::GetUserScanOption()
{
	// note: "deleted infected" is the default if this parameter is missing
	// from the ServerParameters table
	return CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_ANTIVIRUS_ACTION, AV_OPTION_DELETE_INFECTED);
}

const UINT CVirusScanner::ScanFile_Delete(const tstring &sFilePath, tstring &sVirusName, tstring& sErrorString)
{
	UINT avres = ScanFile(sFilePath, sVirusName, sErrorString);
	
	if ((avres == AV_FILE_IS_INFECTED) ||
		(avres == AV_FILE_IS_SUSPICIOUS))
	{
		// if the file is either infected or suspicious, we'll quarantine it
		UINT nRet = RemoveFile(sFilePath);
		if (nRet != 0)
		{
			// file deletion failed
			LINETRACE(_T("Infected file deletion failed: %d\n"), nRet);
			assert(0);
			// avres is returned to caller
		}
		else
		{
			// file deletion successful
			LINETRACE(_T("Infected file (%s) deleted\n"), sFilePath.c_str());
			return AV_FILE_IS_DELETED;
		}
	}
	
	return avres;
}

const UINT CVirusScanner::RemoveFile(const tstring &sFile)
{
	BOOL bRet = DeleteFile(sFile.c_str());
	if (!bRet)
	{
		// An error occurred during message virus scanning (%s) - (DeleteFile() failed error (%d)]
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_VIRUS_SCANNING,
									ERROR_FILESYSTEM),
						EMS_STRING_ERROR_AV_DELETEFILE,
						sFile.c_str(),
						GetLastError());

		return ERROR_FILESYSTEM;
	}

	return 0;
}

const UINT CVirusScanner::ScanString(const string &sString, tstring &sVirusName, tstring& sErrorString)
{
	AVRESULT avres;
	HRESULT hr;
	BSTR bstrVirusName = NULL;

	// create an IStream object from our string buffer
	IStream *stream = NULL;
	hr = CreateStreamOnHGlobal(NULL, TRUE, &stream);
	if (!SUCCEEDED(hr))
	{
		assert(0);		
		return AV_UNEXPECTED_FAILURE;
	}

	// write the string text to IStream
	hr = stream->Write(sString.c_str(), sString.size(), NULL);
	if (!SUCCEEDED(hr))
	{
		stream->Release();
		assert(0);	
		return AV_UNEXPECTED_FAILURE;
	}

	// reset the IStream seek point
	LARGE_INTEGER zero;
	zero.LowPart = 0;
	zero.HighPart = 0;
	hr = stream->Seek(zero, STREAM_SEEK_SET, NULL);
	if (!SUCCEEDED(hr))
	{
		stream->Release();
		assert(0);		
		return AV_UNEXPECTED_FAILURE;
	}

	hr = m_pAVPAntivirus->ScanStream(	NULL,					// default scenario
										stream,					// stream
										&avres,					// result placeholder
										&bstrVirusName);		// virus name	
	if (!SUCCEEDED(hr)) 
	{
		BSTR pbstrErrorInfo = NULL;
		m_pAVPAntivirus->GetErrorInfo(hr, &pbstrErrorInfo);
		LINETRACE(_T("%s\n"), pbstrErrorInfo);
		sErrorString = OLE2T(pbstrErrorInfo);
		stream->Release();
		assert(0);

		SysFreeString(pbstrErrorInfo);
		
		return AV_UNEXPECTED_FAILURE;
	}

	USES_CONVERSION;
	sVirusName = OLE2T(bstrVirusName);
	stream->Release();

	SysFreeString(bstrVirusName);
	
	return avres;
}

const BOOL CVirusScanner::GetIsScanningEnabled()
{
	return (BOOL)CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_ANTIVIRUS_ENABLE, 0);
}

// this function can be called prior to the initialization of the
// class object
int CVirusScanner::CheckRegistration(bool& bKeyExists, bool &bIsEval, bool &bIsExpired)
{
	IAVPKeyStore* pKeyStore = NULL;
	HRESULT hres;
	long nError;
	BSTR str;

	bKeyExists = false;
	bIsEval = false;
	bIsExpired = false;

	hres = CoCreateInstance(CLSID_AVPKeyStore, NULL, CLSCTX_INPROC, 
		                    IID_IAVPKeyStore, (void**) &pKeyStore);

	if (SUCCEEDED(hres))
	{
		// set the app id
		str = SysAllocString(_T("3CI55"));
		hres = pKeyStore->put_AppID(str);
		SysFreeString(str);

		if (SUCCEEDED(hres))
		{
			hres = pKeyStore->CheckKey(&nError);

			// if this fails, there's no reg key
			if (SUCCEEDED(hres) && nError == 0)
			{
				
				// The key is now in str - you can query for the reg details
				// through the COM properties - see AVPKeyStore.h for details
				//pKeyStore->get_Key(&str);

				bKeyExists = true;

				DCIKeyStruct avKeyInfo;
				pKeyStore->get_ExpireDay((long*) &avKeyInfo.ExpireDay);
				pKeyStore->get_ExpireMonth((long*) &avKeyInfo.ExpireMonth);
				pKeyStore->get_ExpireYear((long*) &avKeyInfo.ExpireYear);
				pKeyStore->get_CreateDay((long*) &avKeyInfo.CreateDay);
				pKeyStore->get_CreateMonth((long*) &avKeyInfo.CreateMonth);
				pKeyStore->get_CreateYear((long*) &avKeyInfo.CreateYear);
				pKeyStore->get_KeyType((long*) &avKeyInfo.KeyType);
				pKeyStore->get_UserType((long*) &avKeyInfo.UserType);
				pKeyStore->get_UserSize((long*) &avKeyInfo.UserSize);
				pKeyStore->get_ResellerID((long*) &avKeyInfo.ResellerID);
				pKeyStore->get_SerialNum((long*) &avKeyInfo.SerialNum);

				bIsExpired = (DCIKeyExpired(&avKeyInfo) == DCIKEY_Error_Key_Expired);
				bIsEval = (avKeyInfo.KeyType == DCIKEY_KeyType_Evaluation);			
			}
			else
			{
				// there's no registration key
				pKeyStore->Release();
				return -1;
			}
		}
		else
		{	
			// this shouldn't happen
			assert(0);
			pKeyStore->Release();
			return -1;
		}
	}
	else
	{
		// AV Plug-in is not installed
		return -1;
	}

	pKeyStore->Release();

	return 0;
}
