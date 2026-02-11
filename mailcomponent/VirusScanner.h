// VirusScanner.h: interface for the CVirusScanner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIRUSSCANNER_H__6807387D_8983_4B26_A4E2_A03D7FF74F2A__INCLUDED_)
#define AFX_VIRUSSCANNER_H__6807387D_8983_4B26_A4E2_A03D7FF74F2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WinRegistry.h"
#include "Kaspersky\klav.exe\klav.h"
#include "Kaspersky\klav.exe\klav_i.c"
#include "Kaspersky\klav.exe\klav_def.h"

// these codes extend the AVRESULT structure
#define AV_FILE_IS_QUARANTINED		50
#define AV_FILE_IS_DELETED			51

class CVirusScanner  
{
public:
	int CheckRegistration(bool& bKeyExists, bool& bIsEval, bool& bIsExpired);
	const BOOL GetIsScanningEnabled();
	const UINT ScanString(const string& sString, tstring& sVirusName, tstring& sErrorString);
	const UINT RemoveFile(const tstring& sFile);
	const UINT ScanFile_Delete(const tstring& sFilePath, tstring& sVirusName, tstring& sErrorString);
	const UINT GetUserScanOption();
	const BOOL ReloadDatabases();
	const BOOL GetLicenseInfo(tstring& sExpiration, BOOL& bValid);
	const BOOL GetVersionInfo(tstring& sEngineVer, tstring& sEngineDate, tstring& sDatabaseCount, tstring& sDatabaseDate);
	const UINT ScanFile_Quarantine_Delete(const tstring& sFilePath, tstring& sVirusName, tstring& sQuarantinePath, tstring& sErrorString);
	const UINT ScanFile_Clean_Delete(const tstring& sFilePath, tstring& sVirusName, tstring& sErrorString);
	const UINT ScanFile(const tstring& sFilePath, tstring& sVirusName, tstring& sErrorString);
	const UINT QuarantineFile(const tstring& sFilePath, tstring& sQuarantinePath);
	const BOOL Initialize();
	void UnInitialize();
	CVirusScanner();
	virtual ~CVirusScanner();

	// these options correlate to the "AVAction" value in the ServerParameters table
	enum av_scanner_option
	{
		AV_OPTION_CLEAN_INFECTED = 0,
		AV_OPTION_DELETE_INFECTED = 1,
		AV_OPTION_QUARANTINE_INFECTED = 2
	};

protected:
	CWinRegistry					m_winReg;
	IAVPAntivirus*					m_pAVPAntivirus;
	bool							m_bIsInitialized;
	DWORD							m_dwScenarioID;
};

#endif // !defined(AFX_VIRUSSCANNER_H__6807387D_8983_4B26_A4E2_A03D7FF74F2A__INCLUDED_)
