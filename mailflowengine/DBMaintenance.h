////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/DBMaintenance.h,v 1.1.6.1 2006/07/18 12:50:09 markm Exp $
//
//  Copyright © 2000 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
// 
// DBMaintenance handles database backups, archives, and purges
// 
////////////////////////////////////////////////////////////////////////////////
//
// DBMaintenance.h: interface for the CDBMaintenance class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBMAINTENANCE_H__766A7FB3_8CBE_4115_9A83_B513567AD655__INCLUDED_)
#define AFX_DBMAINTENANCE_H__766A7FB3_8CBE_4115_9A83_B513567AD655__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDBMaintenance  
{
public:
	CDBMaintenance( CODBCQuery& query, CODBCConn& db );
	virtual ~CDBMaintenance();

	HRESULT Run(void);
	void Initialize(void);
	void SetNextRun(void);

	HRESULT RunMaintenance( bool bForceRun );

	HRESULT RestoreArchive( long nID );
	HRESULT RestoreBackup( wchar_t* wcsBackupFile );
	
	enum { RunIntervalMs = 60000 };	

	void SetKillEvent( HANDLE hKillEvent ) { m_hKillEvent = hKillEvent; }

protected:

	int LaunchDBUpdater(void);
	
	void GetParameters(void);
	void GetServerParameter( int ID, CEMSString& sValue );
	void SetServerParameter( int ID, CEMSString& sValue );

	bool DoBackup(void);
	bool DoArchive(CEMSString& sTicketIDs, int& nNumTickets, bool& bDoMore);
	bool DoPurge(void);

	void DeleteBackupFiles(void);
	void DeleteBackupFile( tstring& sFileName );
	
	DWORD GetFileName( CEMSString & sFolder, CEMSString & sFilename, bool bIsArchive );
	void LogDBMaintError( int nLogErr, int nArchErr, CEMSString& sErr );

	CODBCQuery&			m_query;
	BOOL				m_bInitialized;
	FILETIME			m_NextRun;

	bool m_bDailyMaintenanceEnabled;
	bool m_bDailyBackupsEnabled;
	bool m_bDailyArchivesEnabled;
	bool m_bArchiveAllowAccess;
	int m_nDailyPurgesEnabled;
	int m_nPurgeCutoffDays;
	int m_nArchiveCutoffDays;
	int m_nArchiveMaxTickets;
	int m_nArchiveRemoveTickets;
	int m_nDeleteBackups;
	TIMESTAMP_STRUCT m_Time;
	CEMSString  m_sArchiveFolder;
	CEMSString  m_sBackupFolder;
	DWORD m_LastRun;
	HANDLE m_hKillEvent;
	CODBCConn&	m_db;

};

class CBackupFilesInfo : public WIN32_FIND_DATA
{
public:
};

inline bool operator<(const CBackupFilesInfo& A,const CBackupFilesInfo& B)
{ return (CompareFileTime( &B.ftLastWriteTime, &A.ftLastWriteTime ) == -1); }



#endif // !defined(AFX_DBMAINTENANCE_H__766A7FB3_8CBE_4115_9A83_B513567AD655__INCLUDED_)
