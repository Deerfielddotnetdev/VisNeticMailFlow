// DBMaintenance.h: interface for the CDBMaintenance class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "XMLDataClass.h"

class CDBMaintThread : public CThread  
{
public:
	CDBMaintThread( CSessionMap& SessionMap );
	virtual ~CDBMaintThread();
	virtual unsigned int Run( void );

	int m_nJob;
	unsigned int m_AgentID;
	unsigned int m_nArchiveID;
	tstring m_backupfile;
	bool*	m_pDBMaintRunning;

protected:
	CSessionMap& m_SessionMap;
};


class CDBMaintenance : public CXMLDataClass  
{
public:
	CDBMaintenance(CISAPIData& ISAPIData);
	virtual ~CDBMaintenance();

	virtual int Run( CURLAction& action );

	enum DBMaintJobs { NoJob, Job_Database_Maintenance, Job_Restore, Job_Import, 
					   Job_Finished_Successfully, Job_Finished_With_Error };

protected:
	void Update(void);
	void QueryDB(void);
	void DecodeForm(void);
	void GenerateXML(void);
	void ListBackupFiles(void);
	void ListArchives(void);
	void DoNow( CURLAction& action );
	void DoRestore( CURLAction& action );
	void DoImport( CURLAction& action );
	void LaunchDBMaintThread( int nJob, void* pData = NULL );
	void ShowResults( CURLAction& action );
	void VerifyDirectoryExists( tstring& sFolder );
	void GetFileSize( LPCSTR szFileName, tstring& sFileSize );
	void EditDBArchive( CURLAction& action );
	void DeleteArchive(void);
	void DeleteBackup(void);
	int  CheckError(SQLRETURN retcode, char *fn, SQLHANDLE handle, SQLSMALLINT type);
	void ExtractError(char *fn, SQLHANDLE handle, SQLSMALLINT type);
	int  DoSQLQuery(bool bOutputToFile = false);
	void OpenCDF( CURLAction& action );
	void OutputCDF( tstring& sLine );
	void CloseCDF(void);
	
	TCHAR	m_szTempFile[MAX_PATH];
	HANDLE	m_hCDFFile;
	bool m_OutputCDF;
	bool m_bDailyMaintenanceEnabled;
	bool m_bDailyBackupsEnabled;
	bool m_bDailyArchivesEnabled;
	bool m_bArchiveAllowAccess;
	int m_nDailyPurgesEnabled;
	tstring m_sDailyRunTime;
	tstring m_sLastPurgeTime;
	tstring m_sLastArchiveTime;
	tstring m_sLastBackupTime;
	int m_nPurgeCutoffDays;
	int m_nArchiveCutoffDays;
	int m_nArchiveMaxTickets;
	tstring m_sArchiveFolder;
	tstring m_sBackupFolder;
	int m_nArchiveRemoveTickets;
	int m_nDeleteBackups;
	int m_nSun;
	int m_nMon;
	int m_nTue;
	int m_nWed;
	int m_nThu;
	int m_nFri;
	int m_nSat;
	tstring m_sDaysOfWeek;
};
