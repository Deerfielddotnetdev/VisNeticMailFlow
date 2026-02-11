////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/DBMaintenance.cpp,v 1.2.4.1 2006/07/18 12:50:09 markm Exp $
//
//  Copyright © 2000 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
// 
// DBMaintenance handles database backups, archives, and purges
// 
////////////////////////////////////////////////////////////////////////////////
//
// DBMaintenance.cpp: implementation of the CDBMaintenance class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DBMaintenance.h"
#include "RegistryFns.h"
#include "EMSMutex.h"
#include "DateFns.h"
#include "PurgeFns.h"
#include "AttachFns.h"
#include "ArchiveFns.h"

long g_dwDBMaintResults = 0;

//KF: helpers to tidy CSV lists and safely call purge
static inline void KF_TrimTrailingComma(CEMSString& s) {         //KF
	if (s.length() && s.at(s.length() - 1) == _T(','))             //KF
		s.resize(s.length() - 1);                                   //KF
}                                                                 //KF
static inline bool KF_HasIDs(const CEMSString& s) {               //KF
	return s.length() > 0;                                        //KF
}                                                                 //KF

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDBMaintenance::CDBMaintenance(CODBCQuery& m_query, CODBCConn& db)
	: m_query(m_query), m_db(db)
{
	m_bInitialized = FALSE;
	m_LastRun = 0;
}

CDBMaintenance::~CDBMaintenance()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// Initialize
// 
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::Initialize(void)
{

	m_NextRun.dwLowDateTime = 0;
	m_NextRun.dwHighDateTime = 0;

	GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		L"NextMaintenanceLow", (unsigned int&)m_NextRun.dwLowDateTime);
	GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		L"NextMaintenanceHigh", (unsigned int&)m_NextRun.dwHighDateTime);

	// No registry value found, so write it out
	if (m_NextRun.dwLowDateTime == 0 && m_NextRun.dwHighDateTime == 0)
	{
		SetNextRun();
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetNextRun
// 
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::SetNextRun(void)
{
	const wchar_t* pRunTime = g_Object.GetParameter(EMS_SRVPARAM_DAILY_MAINT_RUN_TIME);

	// If the parameter is found, then set the next run time.
	if (pRunTime)
	{
		SYSTEMTIME st;
		long Hour = 0;
		long Minute = 0;
		bool bBumpDayFlag = false;

		swscanf(pRunTime, L"%d:%d", &Hour, &Minute);

		if (Hour == 12)
		{
			Hour = 0;
		}

		if (Hour < 12 && wcsstr(pRunTime, L"PM"))
		{
			Hour += 12;
		}

		GetLocalTime(&st);

		// Calculate how many days till we should run
		int nDayOfWeek = 0;
		bool bRunToday = false;
		bool bRunTomorrow = false;
		tstring sValue = g_Object.GetParameter(EMS_SRVPARAM_ENABLE_DAILY_MAINTENANCE);
		if (_tcsicmp(sValue.c_str(), _T("0")) != 0)
		{
			nDayOfWeek = st.wDayOfWeek;
			int nEnabledDays[7];
			tstring::size_type pos;
			pos = sValue.find(_T("SU"), 0);
			nEnabledDays[0] = pos != tstring::npos ? 1 : 0;
			pos = sValue.find(_T("MO"), 0);
			nEnabledDays[1] = pos != tstring::npos ? 1 : 0;
			pos = sValue.find(_T("TU"), 0);
			nEnabledDays[2] = pos != tstring::npos ? 1 : 0;
			pos = sValue.find(_T("WE"), 0);
			nEnabledDays[3] = pos != tstring::npos ? 1 : 0;
			pos = sValue.find(_T("TH"), 0);
			nEnabledDays[4] = pos != tstring::npos ? 1 : 0;
			pos = sValue.find(_T("FR"), 0);
			nEnabledDays[5] = pos != tstring::npos ? 1 : 0;
			pos = sValue.find(_T("SA"), 0);
			nEnabledDays[6] = pos != tstring::npos ? 1 : 0;

			if (nEnabledDays[nDayOfWeek] == 1)
			{
				bRunToday = true;
			}
			if (nDayOfWeek == 6)
			{
				nDayOfWeek = 0;
			}
			else
			{
				nDayOfWeek++;
			}
			if (nEnabledDays[nDayOfWeek] == 1)
			{
				bRunTomorrow = true;
			}
		}
		// Advance to next day if the time has passed
		if ((st.wHour > Hour) || (st.wHour == Hour && st.wMinute >= Minute))
		{
			bBumpDayFlag = true;
		}

		st.wHour = (WORD)Hour;
		st.wMinute = (WORD)Minute;

		SystemTimeToFileTime(&st, &m_NextRun);

		if (bBumpDayFlag)
		{
			__int64* pTime = (__int64*)&m_NextRun;
			*pTime = *pTime + __int64(10000000) * __int64(60 * 60 * 24);

			FileTimeToSystemTime(&m_NextRun, &st);
		}

		if ((bRunToday && !bBumpDayFlag) || (bRunTomorrow && bBumpDayFlag))
		{
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
				L"NextMaintenanceLow", m_NextRun.dwLowDateTime);

			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
				L"NextMaintenanceHigh", m_NextRun.dwHighDateTime);
		}
		else
		{
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
				L"NextMaintenanceLow", 0);

			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
				L"NextMaintenanceHigh", 0);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetServerParameter
// 
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::GetServerParameter(int ID, CEMSString& sValue)
{
	TCHAR szDataValue[SERVERPARAMETERS_DATAVALUE_LENGTH];
	long szDataValueLen;

	szDataValue[0] = _T('\0');

	m_query.Initialize();
	BINDPARAM_LONG(m_query, ID);
	BINDCOL_TCHAR(m_query, szDataValue);
	m_query.Execute(_T("SELECT DataValue FROM ServerParameters WHERE ServerParameterID=?"));
	m_query.Fetch();

	sValue = szDataValue;
}


////////////////////////////////////////////////////////////////////////////////
// 
// SetServerParameter
// 
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::SetServerParameter(int ID, CEMSString& sValue)
{
	//KF: upsert logic — create row if missing
	long nCount = 0;                                              //KF
	m_query.Initialize();                                         //KF
	BINDPARAM_LONG(m_query, ID);                                  //KF
	BINDCOL_LONG_NOLEN(m_query, nCount);                          //KF
	m_query.Execute(_T("SELECT COUNT(*) FROM ServerParameters ")   //KF
		_T("WHERE ServerParameterID=?"));             //KF
	m_query.Fetch();                                              //KF

	if (nCount > 0) {                                             //KF
		m_query.Initialize();
		BINDPARAM_TCHAR(m_query, (TCHAR*)sValue.c_str());
		BINDPARAM_LONG(m_query, ID);
		m_query.Execute(_T("UPDATE ServerParameters ")
			_T("SET DataValue=? ")
			_T("WHERE ServerParameterID=?"));
	}
	else {                                                      //KF
		m_query.Initialize();                                     //KF
		BINDPARAM_LONG(m_query, ID);                           //KF
		BINDPARAM_TCHAR(m_query, (TCHAR*)sValue.c_str());      //KF
		m_query.Execute(_T("INSERT INTO ServerParameters ")
			_T("(ServerParameterID, DataValue) ")
			_T("VALUES (?, ?)"));                   //KF
	}                                                             //KF
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetParameters
// 
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::GetParameters(void)
{
	CEMSString  sValue;

	// Could get these from the cache, but let's get them straight from
	// the database to play it safe.

	//GetServerParameter( EMS_SRVPARAM_ENABLE_DAILY_MAINTENANCE, sValue );
	sValue = g_Object.GetParameter(EMS_SRVPARAM_ENABLE_DAILY_MAINTENANCE);
	m_bDailyMaintenanceEnabled = (_tcsicmp(sValue.c_str(), _T("0")) == 0) ? false : true;

	//GetServerParameter( EMS_SRVPARAM_ENABLE_DAILY_BACKUPS, sValue );
	m_bDailyBackupsEnabled = (_ttoi(g_Object.GetParameter(EMS_SRVPARAM_ENABLE_DAILY_BACKUPS)) == 0) ? false : true;

	//GetServerParameter( EMS_SRVPARAM_ENABLE_DAILY_ARCHIVES, sValue );
	m_bDailyArchivesEnabled = (_ttoi(g_Object.GetParameter(EMS_SRVPARAM_ENABLE_DAILY_ARCHIVES)) == 0) ? false : true;

	//GetServerParameter( EMS_SRVPARAM_ARCHIVE_ACCESS, sValue );
	m_bArchiveAllowAccess = (_ttoi(g_Object.GetParameter(EMS_SRVPARAM_ARCHIVE_ACCESS)) == 0) ? false : true;

	//GetServerParameter( EMS_SRVPARAM_ENABLE_DAILY_PURGES, sValue );
	m_nDailyPurgesEnabled = _ttoi(g_Object.GetParameter(EMS_SRVPARAM_ENABLE_DAILY_PURGES));

	//GetServerParameter( EMS_SRVPARAM_PURGE_CUTOFF, sValue );
	m_nPurgeCutoffDays = _ttoi(g_Object.GetParameter(EMS_SRVPARAM_PURGE_CUTOFF));

	//GetServerParameter( EMS_SRVPARAM_ARCHIVE_CUTOFF, sValue );
	m_nArchiveCutoffDays = _ttoi(g_Object.GetParameter(EMS_SRVPARAM_ARCHIVE_CUTOFF));

	//GetServerParameter( EMS_SRVPARAM_ARCHIVE_MAX, sValue );
	m_nArchiveMaxTickets = _ttoi(g_Object.GetParameter(EMS_SRVPARAM_ARCHIVE_MAX));

	//GetServerParameter( EMS_SRVPARAM_ARCHIVE_REMOVE_TICKETS, sValue );
	m_nArchiveRemoveTickets = _ttoi(g_Object.GetParameter(EMS_SRVPARAM_ARCHIVE_REMOVE_TICKETS));


	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_ARCHIVE_PATH_VALUE, m_sArchiveFolder);

	if (m_sArchiveFolder.length() > 0)
	{
		// Remove all backslashes at end
		while (m_sArchiveFolder.at(m_sArchiveFolder.length() - 1) == _T('\\'))
		{
			m_sArchiveFolder.resize(m_sArchiveFolder.length() - 1);
		}
	}

	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_BACKUP_PATH_VALUE, m_sBackupFolder);

	if (m_sBackupFolder.length() > 0)
	{
		// Remove all backslashes at end
		while (m_sBackupFolder.at(m_sBackupFolder.length() - 1) == _T('\\'))
		{
			m_sBackupFolder.resize(m_sBackupFolder.length() - 1);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CDBMaintenance::Run(void)
{
	HRESULT hres;
	SYSTEMTIME st;
	FILETIME now;
	long TickCount = GetTickCount();

	if (m_LastRun == 0 || TickCount - m_LastRun > RunIntervalMs)
	{
		DebugReporter::Instance().DisplayMessage("CDBMaintenance::Run", DebugReporter::ENGINE);

		if (_wtoi(g_Object.GetParameter(EMS_SRVPARAM_DB_MAINT)))
		{
			Initialize();
			m_LastRun = TickCount;
		}
		else
		{
			m_LastRun = TickCount;
			return S_FALSE;
		}
	}
	else
	{
		return S_FALSE;
	}

	GetLocalTime(&st);
	SystemTimeToFileTime(&st, (FILETIME*)&now);

	if (CompareFileTime(&now, &m_NextRun) > 0)
	{
		try
		{
			hres = RunMaintenance(false);
		}
		catch (...)
		{
			SetNextRun();
			throw;
		}

		if (SUCCEEDED(hres))
		{
			SetNextRun();
		}

		return hres;
	}

	return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RunMaintenance
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CDBMaintenance::RunMaintenance(bool bForceRun)
{
	GetParameters();

	if (m_bDailyMaintenanceEnabled || bForceRun)
	{
		CDBMaintenanceMutex mutex;
		SYSTEMTIME now;
		CEMSString sNow;
		bool bNoAccessSet = false;

		try
		{
			Log(E_DBMaintenanceInfo, L"Database maintenance started");

			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"CancelMaintenance", 0);

			g_dwDBMaintResults = 0;
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 1);

			if (m_bDailyBackupsEnabled)
			{
				Log(E_DBMaintenanceInfo, L"Database backup started");
				try
				{
					if (mutex.AcquireLock(0))
					{
						g_Object.m_MessagingComponents.Shutdown();

						// Give the ISAPI threads a few seconds to relinquish their connections.
						WaitForSingleObject(m_hKillEvent, 3000);

						bNoAccessSet = true;

						if (DoBackup())
						{
							GetLocalTime(&now);
							GetDateString(now.wYear, now.wMonth, now.wDay, sNow);
							SetServerParameter(EMS_SRVPARAM_BACKUP_LASTRUN, sNow);
							DeleteBackupFiles();
						}
						if (m_bArchiveAllowAccess || (!m_bDailyArchivesEnabled && !m_nDailyPurgesEnabled))
						{
							mutex.ReleaseLock();
							g_Object.m_MessagingComponents.Initialize();
							bNoAccessSet = false;
						}
					}
					else
					{
						g_dwDBMaintResults = E_DBMaintErrorUnhandled;
						WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
						Log(E_DBMaintErrorUnhandled, L"Could not obtain mutex for Database Maintenance Backup");
					}
				}
				catch (int nErrorCode)
				{
					if (bNoAccessSet)
					{
						mutex.ReleaseLock();
						g_Object.m_MessagingComponents.Initialize();
						bNoAccessSet = false;
					}
					g_dwDBMaintResults = nErrorCode;
					WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
					return nErrorCode;
				}
				catch (...)
				{
					if (m_bArchiveAllowAccess || (!m_bDailyArchivesEnabled && !m_nDailyPurgesEnabled))
					{
						mutex.ReleaseLock();
						g_Object.m_MessagingComponents.Initialize();
						bNoAccessSet = false;
					}
				}

			}

			if (m_bDailyArchivesEnabled)
			{
				Log(E_DBMaintenanceInfo, L"Database archive started");
				CEMSString sTicketIDs;
				int nNumTickets;
				bool bDoMore = true;

				try
				{
					if (!m_bArchiveAllowAccess && !bNoAccessSet)
					{
						if (mutex.AcquireLock(0))
						{
							g_Object.m_MessagingComponents.Shutdown();

							// Give the ISAPI threads a few seconds to relinquish their connections.
							WaitForSingleObject(m_hKillEvent, 3000);
							bNoAccessSet = true;
						}
						else
						{
							g_dwDBMaintResults = E_DBMaintErrorUnhandled;
							WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
							Log(E_DBMaintErrorUnhandled, L"Could not obtain mutex for Database Maintenance Archive");
						}
					}

					if (m_nArchiveMaxTickets > 0 && m_nArchiveRemoveTickets > 0)
					{
						while (bDoMore)
						{
							if (DoArchive(sTicketIDs, nNumTickets, bDoMore))
							{
								GetLocalTime(&now);
								GetDateString(now.wYear, now.wMonth, now.wDay, sNow);
								SetServerParameter(EMS_SRVPARAM_ARCHIVE_LASTRUN, sNow);
								bool bKeepTicketSummary = (m_nArchiveRemoveTickets < 2);
								PurgeTickets(sTicketIDs, m_query, nNumTickets, bKeepTicketSummary);
							}
							else
							{
								bDoMore = false;
							}

							UINT nCancelMaint;
							if (GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("CancelMaintenance"), nCancelMaint) == ERROR_SUCCESS)
							{
								if (nCancelMaint == 1)
								{
									bDoMore = false;
								}
							}
						}
					}
					else
					{
						if (DoArchive(sTicketIDs, nNumTickets, bDoMore))
						{
							GetLocalTime(&now);
							GetDateString(now.wYear, now.wMonth, now.wDay, sNow);
							SetServerParameter(EMS_SRVPARAM_ARCHIVE_LASTRUN, sNow);
							bool bKeepTicketSummary = (m_nArchiveRemoveTickets < 2);
							PurgeTickets(sTicketIDs, m_query, nNumTickets, bKeepTicketSummary);
						}
					}

					if (bNoAccessSet && !m_nDailyPurgesEnabled)
					{
						mutex.ReleaseLock();
						g_Object.m_MessagingComponents.Initialize();
						bNoAccessSet = false;
					}
				}
				catch (int nErrorCode)
				{
					if (bNoAccessSet)
					{
						mutex.ReleaseLock();
						g_Object.m_MessagingComponents.Initialize();
						bNoAccessSet = false;
					}
					g_dwDBMaintResults = nErrorCode;
					WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
					return nErrorCode;
				}
				catch (...)
				{
					if (bNoAccessSet && !m_nDailyPurgesEnabled)
					{
						mutex.ReleaseLock();
						g_Object.m_MessagingComponents.Initialize();
						bNoAccessSet = false;
					}
				}
			}
			if (m_nDailyPurgesEnabled > 0)
			{
				Log(E_DBMaintenanceInfo, L"Database purge started");
				try
				{
					if (!m_bArchiveAllowAccess && !bNoAccessSet)
					{
						if (mutex.AcquireLock(0))
						{
							g_Object.m_MessagingComponents.Shutdown();

							// Give the ISAPI threads a few seconds to relinquish their connections.
							WaitForSingleObject(m_hKillEvent, 3000);
							bNoAccessSet = true;
						}
						else
						{
							g_dwDBMaintResults = E_DBMaintErrorUnhandled;
							WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
							Log(E_DBMaintErrorUnhandled, L"Could not obtain mutex for Database Maintenance Purge");
						}
					}

					if (DoPurge())
					{
						GetLocalTime(&now);
						GetDateString(now.wYear, now.wMonth, now.wDay, sNow);
						SetServerParameter(EMS_SRVPARAM_PURGE_LASTRUN, sNow);
					}

					if (bNoAccessSet)
					{
						mutex.ReleaseLock();
						g_Object.m_MessagingComponents.Initialize();
						bNoAccessSet = false;
					}
				}
				catch (...)
				{
					if (bNoAccessSet)
					{
						mutex.ReleaseLock();
						g_Object.m_MessagingComponents.Initialize();
						bNoAccessSet = false;
					}
				}
			}

			if (bNoAccessSet)
			{
				mutex.ReleaseLock();
				g_Object.m_MessagingComponents.Initialize();
				bNoAccessSet = false;
			}

			Log(E_DBMaintenanceInfo, L"Database maintenance finished");
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 4);
		}
		catch (int nErrorCode)
		{
			if (bNoAccessSet)
			{
				mutex.ReleaseLock();
				g_Object.m_MessagingComponents.Initialize();
				bNoAccessSet = false;
			}
			// We've already logged it
			g_dwDBMaintResults = nErrorCode;
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
		}
		catch (ODBCError_t err)
		{
			if (bNoAccessSet)
			{
				mutex.ReleaseLock();
				g_Object.m_MessagingComponents.Initialize();
				bNoAccessSet = false;
			}
			Log(E_DBMaintErrorODBC, L"ODBC Error in Database Maintenance: %s\n",
				err.szErrMsg);
			g_dwDBMaintResults = E_DBMaintErrorODBC;
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
			throw;
		}
		catch (...)
		{
			if (bNoAccessSet)
			{
				mutex.ReleaseLock();
				g_Object.m_MessagingComponents.Initialize();
				bNoAccessSet = false;
			}
			g_dwDBMaintResults = E_DBMaintErrorUnhandled;
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
			Log(E_DBMaintErrorUnhandled, L"Unhandled Exception in Database Maintenance");
			throw;
		}
	}


	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoBackup
// 
////////////////////////////////////////////////////////////////////////////////
bool CDBMaintenance::DoBackup(void)
{
	CEMSString  sBackupFile;
	DWORD dwErr;
	CEMSString sErr;

	dwErr = GetFileName(m_sBackupFolder, sBackupFile, false);

	if (dwErr)
	{
		Log(E_DBMaintErrorWritingToFile, L"Backup Failed, Error %d writing to file: %s",
			dwErr, sBackupFile.c_str());
		throw (int)E_DBMaintErrorWritingToFile;
	}

	WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"CancelMaintenance", 0);

	dwErr = Backup(m_query, sBackupFile, sErr);

	if (dwErr == Arc_Success)
	{
		Log(E_DBMaintenanceInfo, L"Database successfully backed up to: %s", sBackupFile.c_str());
	}
	else
	{
		LogDBMaintError(E_DBMaintErrorBackup, dwErr, sErr);
		throw (int)dwErr;
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
// 
// DoArchive
// 
////////////////////////////////////////////////////////////////////////////////
bool CDBMaintenance::DoArchive(CEMSString& sTicketIDs, int& nNumTickets, bool& bDoMore)
{
	CEMSString  sDestFile;
	int nRetval;
	DWORD dwErr;
	CEMSString sErr;

	dwErr = GetFileName(m_sArchiveFolder, sDestFile, true);

	if (dwErr)
	{
		Log(E_DBMaintErrorWritingToFile, L"Archive Failed, Error %d writing to file: %s",
			dwErr, sDestFile.c_str());
		throw (int)E_DBMaintErrorWritingToFile;
	}

	bool bRemoveTickets = (m_nArchiveRemoveTickets > 0);
	bool bKeepTicketSummary = (m_nArchiveRemoveTickets < 2);

	WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"CancelMaintenance", 0);

	nRetval = ExportArchive(m_query, sDestFile, m_nArchiveCutoffDays,
		bRemoveTickets, bKeepTicketSummary, sErr, sTicketIDs, nNumTickets, bDoMore, m_nArchiveMaxTickets, m_bArchiveAllowAccess);
	if (nRetval == 0)
	{
		Log(E_DBMaintenanceInfo, L"Database archived to: %s", sDestFile.c_str());
	}
	else if (nRetval == Arc_NoTicketsFound)
	{
		Log(E_DBMaintenanceInfo, L"No tickets to archive");
	}
	else
	{
		bDoMore = false;
		LogDBMaintError(E_DBMaintErrorArchive, nRetval, sErr);
		throw (int)nRetval;
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
// 
// DoPurge
// 
////////////////////////////////////////////////////////////////////////////////
bool CDBMaintenance::DoPurge(void)
{
	CEMSString sActualFilename;
	CEMSString sIDs;
	CEMSString sID;
	int ID;

	WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"CancelMaintenance", 0);

	//
	// Always purge hard-deleted (>1) records
	//
	m_query.Reset(false);
	m_query.Execute(_T("DELETE FROM StandardResponses WHERE IsDeleted>1"));
	m_query.Reset(false);
	m_query.Execute(_T("DELETE FROM TicketBoxFooters WHERE IsDeleted>1"));
	m_query.Reset(false);
	m_query.Execute(_T("DELETE FROM TicketBoxHeaders WHERE IsDeleted>1"));
	m_query.Reset(false);
	m_query.Execute(_T("DELETE FROM Groups WHERE IsDeleted>1"));

	//
	// Also clear tickets that are fully marked >1
	//
	m_query.Reset(false);
	BINDCOL_LONG_NOLEN(m_query, ID);
	m_query.Execute(_T("SELECT TicketID FROM Tickets WHERE IsDeleted>1"));

	int nNumTickets = 0;
	while (m_query.Fetch() == S_OK)
	{
		nNumTickets++;
		sID.Format(_T("%d,"), ID);
		sIDs.append(sID);
	}

	KF_TrimTrailingComma(sIDs);
	if (KF_HasIDs(sIDs) && nNumTickets > 0)
		PurgeTickets(sIDs, m_query, nNumTickets);

	//
	// Purge inactive open tickets
	//
	if (m_nDailyPurgesEnabled & EMS_PURGE_TICKETS)
	{
		sIDs = _T("");
		m_query.Reset(false);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		BINDCOL_LONG_NOLEN(m_query, ID);
		m_query.Execute(_T("SELECT TicketID FROM Tickets AS T ")
			_T("WHERE DATEDIFF(day,OpenTimeStamp,GETDATE()) >= ? ")
			_T("AND ( (SELECT COUNT(*) FROM InboundMessages AS I WHERE T.TicketID=I.TicketID) = 0 ")
			_T("OR DATEDIFF(day,(SELECT MAX(I.DateReceived) FROM InboundMessages AS I WHERE T.TicketID=I.TicketID),GETDATE()) >= ? ) ")
			_T("AND ( (SELECT COUNT(*) FROM OutboundMessages AS O WHERE T.TicketID=O.TicketID) = 0 ")
			_T("OR DATEDIFF(day,(SELECT MAX(O.EmailDateTime) FROM OutboundMessages AS O WHERE T.TicketID=O.TicketID),GETDATE()) >= ? ) ")
			_T("AND (SELECT COUNT(*) FROM OutboundMessages AS O WHERE T.TicketID=O.TicketID AND O.OutboundMessageStateID = 1) = 0 "));

		nNumTickets = 0;
		while (m_query.Fetch() == S_OK)
		{
			nNumTickets++;
			sID.Format(_T("%d,"), ID);
			sIDs.append(sID);
		}

		KF_TrimTrailingComma(sIDs);
		if (KF_HasIDs(sIDs) && nNumTickets > 0)
			PurgeTickets(sIDs, m_query, nNumTickets);
	}

	//
	// Purge deleted (trash) items
	//
	if (m_nDailyPurgesEnabled & EMS_PURGE_TRASH)
	{
		// --- Deleted Tickets ---
		sIDs = _T("");
		m_query.Reset(false);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		BINDCOL_LONG_NOLEN(m_query, ID);
		m_query.Execute(_T("SELECT TicketID FROM Tickets ")
			_T("WHERE IsDeleted=1 AND DATEDIFF(day,DeletedTime,GETDATE()) > ?"));

		nNumTickets = 0;
		while (m_query.Fetch() == S_OK)
		{
			nNumTickets++;
			sID.Format(_T("%d,"), ID);
			sIDs.append(sID);
		}
		KF_TrimTrailingComma(sIDs);
		if (KF_HasIDs(sIDs) && nNumTickets > 0)
			PurgeTickets(sIDs, m_query, nNumTickets);

		// --- Deleted Inbound Messages ---
		sIDs = _T("");
		m_query.Reset(false);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		BINDCOL_LONG_NOLEN(m_query, ID);
		m_query.Execute(_T("SELECT InboundMessageID FROM InboundMessages ")
			_T("WHERE IsDeleted=1 AND DATEDIFF(day,DeletedTime,GETDATE()) > ?"));

		while (m_query.Fetch() == S_OK)
		{
			sID.Format(_T("%d,"), ID);
			sIDs.append(sID);
		}
		KF_TrimTrailingComma(sIDs);
		if (KF_HasIDs(sIDs))
			PurgeInboundMessages(sIDs, m_query);

		// --- Deleted Outbound Messages ---
		sIDs = _T("");
		m_query.Reset(false);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		BINDCOL_LONG_NOLEN(m_query, ID);
		m_query.Execute(_T("SELECT OutboundMessageID FROM OutboundMessages ")
			_T("WHERE IsDeleted=1 AND DATEDIFF(day,DeletedTime,GETDATE()) > ?"));

		while (m_query.Fetch() == S_OK)
		{
			sID.Format(_T("%d,"), ID);
			sIDs.append(sID);
		}
		KF_TrimTrailingComma(sIDs);
		if (KF_HasIDs(sIDs))
			PurgeOutboundMessages(sIDs, m_query);

		// --- Deleted Contacts ---
		sIDs = _T("");
		m_query.Reset(false);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		BINDCOL_LONG_NOLEN(m_query, ID);
		m_query.Execute(_T("SELECT ContactID FROM Contacts ")
			_T("WHERE IsDeleted=1 AND DATEDIFF(day,DeletedTime,GETDATE()) > ?"));

		while (m_query.Fetch() == S_OK)
		{
			sID.Format(_T("%d,"), ID);
			sIDs.append(sID);
		}
		KF_TrimTrailingComma(sIDs);
		if (KF_HasIDs(sIDs))
			PurgeContacts(sIDs, m_query);

		// --- Deleted Standard Responses ---
		sIDs = _T("");
		m_query.Reset(false);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		BINDCOL_LONG_NOLEN(m_query, ID);
		m_query.Execute(_T("SELECT StandardResponseID FROM StandardResponses ")
			_T("WHERE IsDeleted=1 AND DATEDIFF(day,DeletedTime,GETDATE()) > ?"));

		while (m_query.Fetch() == S_OK)
		{
			sID.Format(_T("%d,"), ID);
			sIDs.append(sID);
		}
		KF_TrimTrailingComma(sIDs);
		if (KF_HasIDs(sIDs))
			PurgeStandardResponses(sIDs, m_query);
	}

	//
	// Other maintenance purges
	//
	if (m_nDailyPurgesEnabled & EMS_PURGE_STDRSPUSAGE)
	{
		m_query.Reset(true);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		m_query.Execute(_T("DELETE FROM StandardResponseUsage WHERE DATEDIFF(day,DateUsed,GETDATE()) > ?"));
	}

	if (m_nDailyPurgesEnabled & EMS_PURGE_ALERTS)
	{
		m_query.Reset(true);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		m_query.Execute(_T("DELETE FROM AlertMsgs WHERE DATEDIFF(day,DateCreated,GETDATE()) > ?"));
	}

	if (m_nDailyPurgesEnabled & EMS_PURGE_CONTACTS)
	{
		sIDs = _T("");
		m_query.Reset(false);
		BINDPARAM_LONG(m_query, m_nPurgeCutoffDays);
		BINDCOL_LONG_NOLEN(m_query, ID);
		m_query.Execute(_T("SELECT ContactID FROM Contacts ")
			_T("WHERE DATEDIFF(day,DateCreated,GETDATE()) > ? ")
			_T("AND ContactID NOT IN (SELECT TicketContacts.ContactID FROM TicketContacts ")
			_T("WHERE TicketContacts.ContactID = Contacts.ContactID)"));
		while (m_query.Fetch() == S_OK)
		{
			sID.Format(_T("%d,"), ID);
			sIDs.append(sID);
		}
		KF_TrimTrailingComma(sIDs);
		if (KF_HasIDs(sIDs))
			PurgeContacts(sIDs, m_query);
	}

	Log(E_DBMaintenanceInfo, L"Database purge complete.");
	return true;
}



////////////////////////////////////////////////////////////////////////////////
// 
// RestoreArchive
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CDBMaintenance::RestoreArchive(long nID)
{
	CDBMaintenanceMutex mutex;
	wchar_t szArchive[ARCHIVES_ARCFILEPATH_LENGTH];
	long szArchiveLen;
	CEMSString  sArcPath;
	CEMSString sErr;

	WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"CancelMaintenance", 0);

	if (mutex.AcquireLock(0))
	{
		g_dwDBMaintResults = 0;
		WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 3);
		m_query.Initialize();
		BINDPARAM_LONG(m_query, nID);
		BINDCOL_WCHAR(m_query, szArchive);

		m_query.Execute(_T("SELECT ArcFilePath FROM Archives ")
			_T("WHERE ArchiveID=?"));

		if (m_query.Fetch() == S_OK)
		{
			int nRetval;

			sArcPath = szArchive;

			nRetval = ImportArchive(m_query, sArcPath, false, sErr);

			if (nRetval == 0)
			{
				Log(E_DBMaintenanceInfo, L"Archive %s restored successfully.", szArchive);
				WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 4);
			}
			else
			{
				LogDBMaintError(E_DBMaintErrorImport, nRetval, sErr);
				g_dwDBMaintResults = E_DBMaintErrorImport;
				WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
				return S_OK;
			}
		}
		else
		{
			Log(E_DBMaintErrorImport, L"Archive ID %d not found in database.", nID);
			g_dwDBMaintResults = E_DBMaintErrorImport;
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
		}
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreBackup
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CDBMaintenance::RestoreBackup(wchar_t* wcsBackupFile)
{
	CDBMaintenanceMutex mutex;
	CEMSString  sBackupFile;
	int nErr;
	CEMSString sErr;

	WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"CancelMaintenance", 0);

	if (mutex.AcquireLock(0))
	{
		g_dwDBMaintResults = 0;
		WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 2);
		GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_BACKUP_PATH_VALUE, m_sBackupFolder);

		if (m_sBackupFolder.length() > 0)
		{
			// Remove all backslashes at end
			while (m_sBackupFolder.at(m_sBackupFolder.length() - 1) == _T('\\'))
			{
				m_sBackupFolder.resize(m_sBackupFolder.length() - 1);
			}
		}

		sBackupFile.Format(_T("%s\\%s"), m_sBackupFolder.c_str(), wcsBackupFile);

		delete[] wcsBackupFile;

		nErr = Restore(m_query, sBackupFile, true, false, false, sErr);

		if (nErr == 0)
		{
			Log(E_DBMaintenanceInfo, L"Database restored from %s", sBackupFile.c_str());
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 4);
		}
		else if (nErr != Arc_ErrNotArchive)
		{
			LogDBMaintError(E_DBMaintErrorRestore, nErr, sErr);
			g_dwDBMaintResults = E_DBMaintErrorRestore;
			WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
			return S_OK;
		}
		else
		{
			wchar_t szDebug[1024];
			CEMSString sSQL;
			int nAttempts = 0;
			wchar_t szDatabaseName[256];
			long szDatabaseNameLen = 0;

			m_query.Initialize();
			BINDCOL_TCHAR(m_query, szDatabaseName);
			m_query.Execute(L"SELECT DB_Name()");
			m_query.Fetch();

			sSQL.Format(L"RESTORE DATABASE [%s] FROM DISK='%s'",
				szDatabaseName, sBackupFile.c_str());
			try
			{
				while (nAttempts++ < 10)
				{
					try
					{
						m_query.Initialize();
						m_query.Execute(L"USE master");
						m_query.Initialize();
						m_query.Execute(sSQL.c_str());

						do
						{
							ODBCError_t err;
							m_query.GetQueryDiagnostics(err);
						} while (m_query.GetMoreResults());

						sSQL.Format(_T("USE [%s]"), szDatabaseName);
						m_query.Initialize();
						m_query.Execute(sSQL.c_str());

						if (LaunchDBUpdater() != 0)
						{
							g_dwDBMaintResults = E_DBMaintErrorRestore;
						}

						break;
					}
					catch (ODBCError_t err)
					{
						swprintf(szDebug, L"Caught ODBCError: %d, %d, %s\n",
							err.nErrorCode, err.nNativeErrorCode, err.szErrMsg);

						if (m_db.IsConnected())
						{
							m_query.Close();
							m_db.Disconnect();
						}
						m_db.Connect();

						if (err.nNativeErrorCode != 3101)
						{
							Log(E_DBMaintErrorRestore, L"ODBC Error %s restoring database from: %s",
								err.szErrMsg, sBackupFile.c_str());
							g_dwDBMaintResults = E_DBMaintErrorRestore;
							return S_OK;
						}

					}
					WaitForSingleObject(m_hKillEvent, 10000);
				}
			}
			catch (...)
			{
				Log(E_DBMaintErrorRestore, L"Exception thrown restoring database from: %s", sBackupFile.c_str());
				throw;
			}

			if (nAttempts >= 10)
			{
				Log(E_DBMaintErrorRestore, L"Could not restore database from: %s because database in use. Please close all DB connections and try again",
					sBackupFile.c_str());
				g_dwDBMaintResults = E_DBMaintErrorRestore;
				WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 5);
			}
			else
			{
				Log(E_DBMaintenanceInfo, L"Database restored from: %s", sBackupFile.c_str());
				WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"MaintenanceProg", 4);
			}

			sSQL.Format(_T("USE [%s]"), szDatabaseName);
			m_query.Initialize();
			m_query.Execute(sSQL.c_str());
		}

	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 
// LogDBMaintError
// 
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::LogDBMaintError(int nLogErr, int nArchErr, CEMSString& sErr)
{
	switch (nArchErr)
	{
	case Arc_ErrWritingToFile:
		Log(nLogErr, L"Error writing to file in database maintenance operation.");
		break;

	case Arc_ErrReadingFile:
		Log(nLogErr, L"Error reading from file in database maintenance operation.");
		break;

	case Arc_ErrMemAlloc:
		Log(nLogErr, L"Error allocating memory in database maintenance operation.");
		break;

	case Arc_ErrDeflate:
		Log(nLogErr, L"Compression error in database maintenance operation.");
		break;

	case Arc_ErrInflate:
		Log(nLogErr, L"De-compression error in database maintenance operation.");
		break;

	case Arc_ErrIDNotFound:
		Log(nLogErr, L"ID not found error in database maintenance operation.");
		break;

	case Arc_ErrNotArchive:
		Log(nLogErr, L"Error - source file is not an archive.");
		break;

	case Arc_ErrOpenFile:
		Log(nLogErr, L"Error opening file in database maintenance operation.");
		break;

	case Arc_ErrFileSeek:
		Log(nLogErr, L"Error moving file pointer in database maintenance operation.");
		break;

	case Arc_ErrBufferTooSmall:
		Log(nLogErr, L"Buffer too small error in database maintenance operation.");
		break;

	case Arc_ErrFileNotFound:
		Log(nLogErr, L"File not found error in database maintenance operation.");
		break;

	case Arc_ErrCreatingFile:
		Log(nLogErr, L"Error creating file in database maintenance operation.");
		break;

	case Arc_NoTicketsFound:
		Log(nLogErr, L"No tickets found older than cutoff interval.");
		break;

	case Arc_ErrODBC:
		Log(nLogErr, L"ODBC error in database maintenance operation: %s", sErr.c_str());
		break;

	case Arc_ErrException:
		Log(nLogErr, L"Exception in database maintenance operation: %s", sErr.c_str());
		break;

	case Arc_ErrTruncation:
		Log(nLogErr, L"One of the Routing Rule's MatchText had an error when restoring.");
		break;

	case Arc_ErrCanceled:
		Log(nLogErr, L"Database Maintenace canceled by an Administrator.");
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// LaunchDBUpdater
// 
////////////////////////////////////////////////////////////////////////////////
int CDBMaintenance::LaunchDBUpdater(void)
{
	int retval = 0;
	CEMSString  tstrPath;
	CEMSString  regDBVersion;
	TCHAR szDBVersion[256];
	long szDBVersionLen;
	long nIDDBVersion = EMS_SRVPARAM_DATABASE_VERSION;

	m_query.Initialize();
	BINDPARAM_LONG(m_query, nIDDBVersion);
	BINDCOL_TCHAR(m_query, szDBVersion);
	m_query.Execute(_T("SELECT DataValue FROM ServerParameters WHERE ServerParameterID=?"));
	m_query.Fetch();

	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_DB_VERSION_VALUE, regDBVersion);

	if (regDBVersion.compare(szDBVersion) == 0)
	{
		// Versions match, no update necessary
		return retval;
	}

	// Write the DB version to the registry so that dbup.exe sees the right version
	WriteRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_DB_VERSION_VALUE, szDBVersion);

	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_INSTALL_PATH_VALUE, tstrPath);

	if (tstrPath.length() > 0 && tstrPath.at(tstrPath.length() - 1) != _T('\\'))
	{
		tstrPath += _T("\\");
	}

	tstrPath.append(_T("bin\\dbup.exe -s"));

	// STARTUPINFO init
	STARTUPINFO sui;
	ZeroMemory(&sui, sizeof(STARTUPINFO));
	sui.cb = sizeof(STARTUPINFO);
	sui.dwFlags = STARTF_USESHOWWINDOW;
	sui.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	// execute the wingate installer
	if (CreateProcess(NULL,
		(LPTSTR)tstrPath.c_str(),
		NULL,
		NULL,
		TRUE,
		NORMAL_PRIORITY_CLASS,
		NULL,
		NULL,
		&sui,
		&pi))
	{
		// wait for the process to exit
		if (WaitForSingleObject(pi.hProcess, 180000) == WAIT_TIMEOUT)
		{
			Log(E_DBMaintErrorRestore, _T("DB updater took longer than expected to complete. ")
				_T("The system will not function properly until it finishes."));
			retval = -2;
		}


		// check the exit code
		DWORD dwRet;
		GetExitCodeProcess(pi.hProcess, &dwRet);

		if (dwRet != 0 && dwRet != 100 && dwRet != 101)
		{
			if (dwRet == 2)
			{
				Log(E_DBMaintErrorRestore, _T("The DB update procedure failed to get information from the registry. ")
					_T("If this backup is from a previous version, you must execute dbup.exe."));
			}
			else if (dwRet == 3)
			{
				Log(E_DBMaintErrorRestore, _T("The DB update procedure failed to connect to SQL Server. ")
					_T("If this backup is from a previous version, you must execute dbup.exe."));
			}
			else if (dwRet == 4)
			{
				Log(E_DBMaintErrorRestore, _T("The DB update procedure failed to execute an SQL statement. ")
					_T("If this backup is from a previous version, you must execute dbup.exe."));
			}
			else
			{
				Log(E_DBMaintErrorRestore, _T("The DB update procedure failed. ")
					_T("If this backup is from a previous version, you must execute dbup.exe."));
			}

			retval = -3;
		}

		// close process information handles
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else
	{
		Log(E_DBMaintErrorRestore, _T("Could not execute DB updater. ")
			_T("If this backup is from a previous version, you must execute dbup.exe."));
		retval = -4;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetFileName
// 
////////////////////////////////////////////////////////////////////////////////
DWORD CDBMaintenance::GetFileName(CEMSString& sFolder, CEMSString& sFilename, bool bIsArchive)
{
	SYSTEMTIME now;
	TCHAR* szExt;
	int n = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	szExt = (bIsArchive) ? _T("ARC") : _T("BAK");

	GetLocalTime(&now);

	while (hFile == INVALID_HANDLE_VALUE)
	{
		n++;

		sFilename.Format(_T("%s\\VMF-%d-%d-%d-%d.%s"), sFolder.c_str(),
			now.wYear, now.wMonth, now.wDay, n, szExt);

		hFile = CreateFile(sFilename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwErr = GetLastError();

			if (dwErr != ERROR_ALREADY_EXISTS && dwErr != ERROR_FILE_EXISTS)
			{
				return dwErr;
			}
		}
	}

	CloseHandle(hFile);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete old backup files
// 
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::DeleteBackupFiles(void)
{
	CEMSString  sValue;
	GetServerParameter(EMS_SRVPARAM_DELETE_BACKUPS, sValue);
	m_nDeleteBackups = _ttoi(sValue.c_str());

	if (m_nDeleteBackups > 0)
	{
		time_t now;
		time(&now);

		SecondsToTimeStamp(now - (86400 * m_nDeleteBackups), m_Time);
		m_Time.hour = 0;
		m_Time.minute = 0;
		m_Time.second = 0;
		m_Time.fraction = 0;

		TCHAR szPath[MAX_PATH];
		tstring tstrDir;
		CBackupFilesInfo backupFileInfo;
		HANDLE hFind;

		set<CBackupFilesInfo> BackupFileList;
		set<CBackupFilesInfo>::iterator iter;

		tstrDir.assign(m_sBackupFolder.c_str());
		// Make sure we have a backslash at the end.
		if (tstrDir.length() > 0 && tstrDir.at(tstrDir.length() - 1) != L'\\')
		{
			tstrDir += L"\\";
		}

		_stprintf(szPath, _T("%sVMF*.BAK"), tstrDir.c_str());

		hFind = FindFirstFile(szPath, &backupFileInfo);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			BackupFileList.insert(backupFileInfo);

			while (FindNextFile(hFind, &backupFileInfo))
			{
				BackupFileList.insert(backupFileInfo);
			}

			FindClose(hFind);
		}

		for (iter = BackupFileList.begin(); iter != BackupFileList.end(); iter++)
		{
			__int64 ix, iy;
			FILETIME ft;
			FileTimeToLocalFileTime(&(iter->ftLastWriteTime), &ft);
			SYSTEMTIME st;
			FileTimeToSystemTime(&ft, &st);
			SystemTimeToFileTime(&st, (FILETIME*)&iy);

			SYSTEMTIME sysTime;
			TimeStampToSystemTime(m_Time, sysTime);
			SystemTimeToFileTime(&sysTime, (FILETIME*)&ix);

			if ((iy - ix) < 0)
			{
				tstring sFileName;
				sFileName = iter->cFileName;
				DeleteBackupFile(sFileName);
			}
		}
	}
}

void CDBMaintenance::DeleteBackupFile(tstring& sFileName)
{
	tstring tstrDir;
	tstrDir.assign(m_sBackupFolder.c_str());
	// Make sure we have a backslash at the end.
	if (tstrDir.length() > 0 && tstrDir.at(tstrDir.length() - 1) != L'\\')
	{
		tstrDir += L"\\";
	}
	tstrDir += sFileName;
	DeleteFile(tstrDir.c_str());
	Log(E_DBMaintenanceInfo, L"Deleted old backup file: %s", sFileName.c_str());
}
