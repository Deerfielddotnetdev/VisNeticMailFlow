// DBMaintenance.cpp: implementation of the CDBMaintenance class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DBMaintenance.h"
#include "RegistryFns.h"
#include "LogConfig.h"
#include "EMSMutex.h"

#define MAX_COL_NAME_LEN  256
#define MAX_COLS 256
const int MAX_CHAR = 1024;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDBMaintenance::CDBMaintenance(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{
	m_nArchiveMaxTickets = 0;
	m_OutputCDF = false;
}

CDBMaintenance::~CDBMaintenance()
{
}

////////////////////////////////////////////////////////////////////////////////
//
// Run
//
////////////////////////////////////////////////////////////////////////////////
int CDBMaintenance::Run(CURLAction& action)
{
	tstring sAction = _T("list");

	// Check security
	RequireAdmin();

	if (_tcsicmp(GetISAPIData().m_sPage.c_str(), _T("dbmaintenanceresults")) == 0)
	{
		if (GetISAPIData().GetXMLPost())
		{
			GetISAPIData().GetXMLString(_T("Action"), sAction);

			if (sAction.compare(_T("cancel")) == 0)
			{
				WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("CancelMaintenance"), 1);
			}
		}
		else
		{
			ShowResults(action);
		}

		return 0;
	}

	if (g_ThreadPool.GetSharedObjects().m_bDBMaintenanceRunning)
	{
		GetXMLGen().AddChildElem(_T("DBMaintenance"));
		GetXMLGen().AddChildAttrib(_T("Running"), "1");
		return 0;
	}

	if (_tcsicmp(GetISAPIData().m_sPage.c_str(), _T("dbmaintenance")) == 0)
	{
		if (GetISAPIData().GetXMLPost())
		{
			DISABLE_IN_DEMO();

			GetISAPIData().GetXMLString(_T("Action"), sAction);

			// KF only decode+persist when explicitly updating settings
			if (sAction.compare(_T("update")) == 0)
			{
				DecodeForm();   // KF
				Update();       // KF
				QueryDB();      // KF
			}
			else if (sAction.compare(_T("doNow")) == 0)
			{
				// Use already-persisted settings; do NOT DecodeForm/Update here
				WriteRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
					_T("LastMaintStatus"),
					_T("Initializing Maintenance Task, please wait..."));

				// KF refresh our in-memory view from persisted params
				QueryDB(); // KF

				// KF ensure previous cancels don't short-circuit a run
				WriteRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("CancelMaintenance"), 0); // KF

				// KF force routing engine to reload server parameters
				m_ISAPIData.m_RoutingEngine.ReloadConfig(EMS_ServerParameters); // KF

				// KF rebuild cached param set on ISAPI side as well
				InvalidateServerParameters(true); // KF

				DoNow(action);
				GetXMLGen().AddChildElem(_T("DBMaintenance"));
				GetXMLGen().AddChildAttrib(_T("Running"), "1");
				GetXMLGen().AddChildAttrib(_T("NextURL"), "dbmaintenance.ems");
				return 0;
			}
		}
		else
		{
			QueryDB();
			GenerateXML();
		}
	}
	else if (_tcsicmp(GetISAPIData().m_sPage.c_str(), _T("dbarchives")) == 0)
	{
		if (GetISAPIData().GetXMLPost())
		{
			DISABLE_IN_DEMO();

			GetISAPIData().GetXMLString(_T("Action"), sAction);

			if (sAction.compare(_T("import")) == 0)
			{
				CEMSString sURL;
				int nCurrentPage = 1;

				DoImport(action);
				GetXMLGen().AddChildElem(_T("DBMaintenance"));
				GetXMLGen().AddChildAttrib(_T("Running"), "1");
				GetISAPIData().GetURLLong(_T("CurrentPage"), nCurrentPage, true);
				sURL.Format(_T("dbarchives.ems?CurrentPage=%d"), nCurrentPage);
				GetXMLGen().AddChildAttrib(_T("NextURL"), sURL.c_str());
				return 0;
			}
			else if (sAction.compare(_T("delete")) == 0)
			{
				DeleteArchive();
			}
		}

		ListArchives();
	}
	else if (_tcsicmp(GetISAPIData().m_sPage.c_str(), _T("dbbackups")) == 0)
	{
		if (GetISAPIData().GetXMLPost())
		{
			DISABLE_IN_DEMO();

			GetISAPIData().GetXMLString(_T("Action"), sAction);

			if (sAction.compare(_T("restore")) == 0)
			{
				CEMSString sURL;
				int nCurrentPage = 1;

				DoRestore(action);
				GetXMLGen().AddChildElem(_T("DBMaintenance"));
				GetXMLGen().AddChildAttrib(_T("Running"), "1");
				GetISAPIData().GetURLLong(_T("CurrentPage"), nCurrentPage, true);
				sURL.Format(_T("dbbackups.ems?CurrentPage=%d"), nCurrentPage);
				GetXMLGen().AddChildAttrib(_T("NextURL"), sURL.c_str());

				return 0;
			}
			else if (sAction.compare(_T("delete")) == 0)
			{
				DeleteBackup();
			}
		}

		ListBackupFiles();
	}
	else if (_tcsicmp(GetISAPIData().m_sPage.c_str(), _T("editdbarchive")) == 0)
	{
		EditDBArchive(action);
	}
	else if (_tcsicmp(GetISAPIData().m_sPage.c_str(), _T("sqlquery")) == 0)
	{

		DISABLE_IN_DEMO();

		tstring sServer;
		tstring sDatabase;

		GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("SQLServer"), sServer);
		GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("DBNAME"), sDatabase);

		GetXMLGen().AddChildElem(_T("SqlQuery"));
		GetXMLGen().AddChildAttrib(_T("Server"), sServer.c_str());
		GetXMLGen().AddChildAttrib(_T("Database"), sDatabase.c_str());
	}
	else if (_tcsicmp(GetISAPIData().m_sPage.c_str(), _T("sqlqueryresults")) == 0)
	{
		DISABLE_IN_DEMO();

		GetISAPIData().GetFormString(_T("Action"), sAction, true);

		if (sAction.compare(_T("doquery")) == 0)
		{
			DoSQLQuery(false);
		}
		else if (sAction.compare(_T("outputfile")) == 0)
		{
			OpenCDF(action);
			DoSQLQuery(true);
		}
		else
		{
			GetXMLGen().AddChildElem(_T("SqlQuery"));
			GetXMLGen().AddChildAttrib(_T("NumRows"), 0);
		}
	}

	return 0;
}
int CDBMaintenance::CheckError(SQLRETURN retcode, char* fn, SQLHANDLE handle, SQLSMALLINT type)
{
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	{
		ExtractError(fn, handle, type);
		return 1;
	}
	return 0;
}

void CDBMaintenance::ExtractError(char* fn, SQLHANDLE handle, SQLSMALLINT type)
{
	SQLSMALLINT i = 0;
	SQLINTEGER NativeError;
	SQLCHAR SQLState[7];
	SQLCHAR MessageText[256];
	SQLSMALLINT TextLength;
	SQLRETURN ret;

	CEMSString sTemp;
	GetXMLGen().AddChildElem(_T("SQLError"));
	GetXMLGen().IntoElem();
	do
	{
		ret = SQLGetDiagRec(type, handle, ++i, SQLState, &NativeError,
			MessageText, sizeof(MessageText), &TextLength);
		if (SQL_SUCCEEDED(ret))
		{
			sTemp.Format(_T("%s:%s:%ld:%ld:%s"),
				fn, SQLState, (long)i, (long)NativeError, MessageText);
			GetXMLGen().AddChildElem(_T("Error"));
			GetXMLGen().AddChildAttrib(_T("Message"), sTemp.c_str());
		}
	} while (ret == SQL_SUCCESS);
	GetXMLGen().OutOfElem();
}

int CDBMaintenance::DoSQLQuery(bool bOutputToFile)
{
	SQLHENV   henv = SQL_NULL_HENV;   // Environment
	SQLHDBC   hdbc = SQL_NULL_HDBC;   // Connection handle
	SQLHSTMT  hstmt = SQL_NULL_HSTMT;  // Statement handle
	SQLRETURN retcode, retcode2;

	SQLCHAR* ColumnName[MAX_COLS];
	SQLSMALLINT    ColumnNameLen[MAX_COLS];
	SQLSMALLINT    ColumnDataType[MAX_COLS];
	SQLULEN        ColumnDataSize[MAX_COLS];
	SQLSMALLINT    ColumnDataDigits[MAX_COLS];
	SQLSMALLINT    ColumnDataNullable[MAX_COLS];
	SQLCHAR* ColumnData[MAX_COLS];
	SQLLEN         ColumnDataLen[MAX_COLS];
	TIMESTAMP_STRUCT ts[MAX_COLS];
	LONG			 tsLen[MAX_COLS];
	SQLSMALLINT    i, j;
	TCHAR szConnectOutput[1024] = { 0 };
	SQLSMALLINT nResult = 0;
	CEMSString sColumn;
	tstring sServer;
	tstring sDatabase;
	tstring sUser;
	tstring sPassword;
	tstring sQuery;
	tstring sDriver;
	char* buffer = new char[MAX_CHAR];
	SQLINTEGER length = MAX_CHAR - 1;
	std::string sData;
	int numRows = 0;
	SQLSMALLINT numCols;
	CEMSString sTemp;
	tstring sLine;
	SQLINTEGER numRowsAffected = 0;
	UINT nSecure = 0;
	UINT nTrust = 0;

	GetISAPIData().GetFormString(_T("user"), sUser);
	GetISAPIData().GetFormString(_T("password"), sPassword);
	GetISAPIData().GetFormString(_T("querytext"), sQuery);

	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("SQLServer"), sServer);
	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("DBNAME"), sDatabase);
	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("OdbcDriver"), sDriver);
	GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("OdbcSecure"), nSecure);
	GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("OdbcTrust"), nTrust);

	CEMSString dSN;

	if (nSecure && nTrust)
	{
		dSN.Format(_T("DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;Encrypt=yes;TrustServerCertificate=yes;")
			, sDriver.c_str()
			, sServer.c_str()
			, sUser.c_str()
			, sPassword.c_str()
			, sDatabase.c_str());
	}
	else if (nSecure)
	{
		dSN.Format(_T("DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;Encrypt=yes;")
			, sDriver.c_str()
			, sServer.c_str()
			, sUser.c_str()
			, sPassword.c_str()
			, sDatabase.c_str());
	}
	else
	{
		dSN.Format(_T("DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;")
			, sDriver.c_str()
			, sServer.c_str()
			, sUser.c_str()
			, sPassword.c_str()
			, sDatabase.c_str());
	}

	tstring sDSN(dSN.c_str());

	try
	{
		// Initialise buffers
		for (i = 0; i < MAX_COLS; i++) {
			ColumnName[i] = NULL;
			ColumnData[i] = NULL;
		}

		// Allocate environment handle
		retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
		if (CheckError(retcode, "SQLAllocHandle(SQL_HANDLE_ENV)",
			henv, SQL_HANDLE_ENV))
		{
			return 1;
		}

		// Set the ODBC version environment attribute
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,
			(SQLPOINTER*)SQL_OV_ODBC3, 0);
		if (CheckError(retcode, "SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION)",
			henv, SQL_HANDLE_ENV))
		{
			return 1;
		}

		// Allocate connection handle
		retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
		if (CheckError(retcode, "SQLAllocHandle(SQL_HANDLE_DBC)",
			hdbc, SQL_HANDLE_DBC))
		{
			return 1;
		}

		// Set login timeout to 5 seconds
		retcode = SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT,
			(SQLPOINTER)5, 0);
		if (CheckError(retcode, "SQLSetConnectAttr(SQL_LOGIN_TIMEOUT)",
			hdbc, SQL_HANDLE_DBC))
		{
			return 1;
		}

		// Connect to data source
		retcode = SQLDriverConnect(hdbc, NULL,
			(SQLTCHAR*)sDSN.c_str(), static_cast<SQLSMALLINT>(sDSN.length()),
			(SQLTCHAR*)szConnectOutput, sizeof(szConnectOutput) / sizeof(TCHAR),
			&nResult, SQL_DRIVER_NOPROMPT);

		if (CheckError(retcode, "SQLConnect(DSN)", hdbc, SQL_HANDLE_DBC))
		{
			return 1;
		}

		// Allocate statement handle
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
		if (CheckError(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)",
			hstmt, SQL_HANDLE_STMT))
		{
			return 1;
		}

		// Prepare Statement (my change to free format input by user)
		retcode = SQLPrepare(hstmt, (SQLTCHAR*)sQuery.c_str(), sQuery.length());
		if (CheckError(retcode, "SQLPrepare(SQL_HANDLE_ENV)",
			hstmt, SQL_HANDLE_STMT))
		{
			return 1;
		}

		// Retrieve number of columns
		retcode = SQLNumResultCols(hstmt, &numCols);
		if (CheckError(retcode, "SQLNumResultCols()", hstmt,
			SQL_HANDLE_STMT))
		{
			return 1;
		}

		if (numCols > 0)
		{
			// Loop round number of columns using SQLDescribeCol to get info about
			// the column, followed by SQLBindCol to bind the column to a data area
			sTemp.clear();
			if (!m_OutputCDF)
			{
				GetXMLGen().AddChildElem(_T("Columns"));
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem(_T("Column"));
				GetXMLGen().AddChildAttrib(_T("Description"), _T(""));
			}
			for (i = 0; i < numCols; i++) {
				ColumnName[i] = (SQLCHAR*)malloc(MAX_COL_NAME_LEN);
				retcode = SQLDescribeCol(
					hstmt,                    // Select Statement (Prepared)
					i + 1,                      // Columnn Number
					ColumnName[i],            // Column Name (returned)
					MAX_COL_NAME_LEN,         // size of Column Name buffer
					&ColumnNameLen[i],        // Actual size of column name
					&ColumnDataType[i],       // SQL Data type of column
					&ColumnDataSize[i],       // Data size of column in table
					&ColumnDataDigits[i],     // Number of decimal digits
					&ColumnDataNullable[i]);  // Whether column nullable
				if (CheckError(retcode, "SQLDescribeCol()", hstmt, SQL_HANDLE_STMT))
				{
					return 1;
				}

				if (m_OutputCDF)
				{
					sTemp.Format(_T("%s"), (const char*)ColumnName[i]);
					sTemp.EscapeCSV();
					if (sLine.length() == 0)
					{
						sLine.append(_T("\""));
						sLine.append(sTemp.c_str());
						sLine.append(_T("\""));
					}
					else
					{
						sLine.append(_T(",\""));
						sLine.append(sTemp.c_str());
						sLine.append(_T("\""));
					}
				}
				else
				{
					GetXMLGen().AddChildElem(_T("Column"));
					GetXMLGen().AddChildAttrib(_T("Description"), (const char*)ColumnName[i]);
				}

				// Bind column, changing SQL data type to C data type
				// (assumes INT and VARCHAR for now)
				ColumnData[i] = (SQLCHAR*)malloc(ColumnDataSize[i] + 1);
				switch (ColumnDataType[i]) {
				case SQL_INTEGER:
					ColumnDataType[i] = SQL_C_CHAR;
					break;
				case SQL_VARCHAR:
					ColumnDataType[i] = SQL_C_CHAR;
					break;
				case SQL_TYPE_TIMESTAMP:
					ColumnDataType[i] = SQL_C_TIMESTAMP;
					break;
				case -6:
					ColumnDataType[i] = SQL_C_TINYINT;
					break;
				case -7:
					ColumnDataType[i] = SQL_C_BIT;
					break;
				}

				if (ColumnDataType[i] == SQL_TIMESTAMP)
				{
					retcode = SQLBindCol(hstmt, i + 1, SQL_C_TIMESTAMP, &ts[i],
						sizeof(ts), &tsLen[i]);
					if (CheckError(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT))
					{
						return 1;
					}
				}
				else if (ColumnDataType[i] == SQL_LONGVARCHAR)
				{
					retcode = SQLBindCol(hstmt,            // Statement handle
						i + 1,                    // Column number
						SQL_C_CHAR,                // C Type
						(SQLPOINTER)NULL,            // Column value Pointer
						(SQLINTEGER)SQL_DATA_AT_EXEC,      // Size of Data Buffer
						&ColumnDataLen[i]); // Size of data returned
					if (CheckError(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT))
					{
						return 1;
					}
				}
				else
				{
					retcode = SQLBindCol(hstmt,                  // Statement handle
						i + 1,                    // Column number
						ColumnDataType[i],      // C Data Type
						ColumnData[i],          // Data buffer
						ColumnDataSize[i],      // Size of Data Buffer
						&ColumnDataLen[i]); // Size of data returned
					if (CheckError(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT))
					{
						return 1;
					}
				}


			}
			if (m_OutputCDF)
			{
				sLine.append(_T("\r\n"));
				OutputCDF(sLine);
				sLine.clear();
			}
			else
			{
				GetXMLGen().OutOfElem();
			}
		}

		retcode = SQLExecute(hstmt);
		if (CheckError(retcode, "SQLExecute()", hstmt, SQL_HANDLE_STMT))
		{
			return 1;
		}
		if (numCols > 0)
		{
			if (!m_OutputCDF)
			{
				GetXMLGen().AddChildElem(_T("Rows"));
				GetXMLGen().IntoElem();
			}

			for (i = 0; ; i++)
			{
				retcode = SQLFetch(hstmt);

				if (retcode == SQL_NO_DATA) {
					break;
				}

				if (CheckError(retcode, "SQLFetch()", hstmt, SQL_HANDLE_STMT))
				{
					return 1;
				}

				if (!m_OutputCDF)
				{
					GetXMLGen().AddChildElem(_T("Row"));
					GetXMLGen().IntoElem();
					GetXMLGen().AddChildElem(_T("Cell"), i + 1);
				}

				for (j = 0; j < numCols; j++)
				{
					if (ColumnDataType[j] == SQL_INTEGER)
					{
						if (m_OutputCDF)
						{
							sTemp.Format(_T("%s"), (const char*)ColumnData[j]);
							sTemp.EscapeCSV();
							if (sLine.length() == 0)
							{
								sLine.append(_T("\""));
								sLine.append(sTemp.c_str());
								sLine.append(_T("\""));
							}
							else
							{
								sLine.append(_T(",\""));
								sLine.append(sTemp.c_str());
								sLine.append(_T("\""));
							}
						}
						else
						{
							GetXMLGen().AddChildElem(_T("Cell"), (const char*)ColumnData[j]);
						}
					}
					else if (ColumnDataType[j] == SQL_TIMESTAMP)
					{
						GetDateTimeString(ts[j], sizeof(ts[j]), sColumn, true, true);
						if (m_OutputCDF)
						{
							if (sLine.length() == 0)
							{
								sLine.append(_T("\""));
								sLine.append(sColumn.c_str());
								sLine.append(_T("\""));
							}
							else
							{
								sLine.append(_T(",\""));
								sLine.append(sColumn.c_str());
								sLine.append(_T("\""));
							}
						}
						else
						{
							GetXMLGen().AddChildElem(_T("Cell"), sColumn.c_str());
						}
					}
					else if (ColumnDataType[j] == SQL_TINYINT || ColumnDataType[j] == SQL_BIT)
					{
						if (m_OutputCDF)
						{
							unsigned int nTemp = (int)*ColumnData[j];
							sTemp.Format(_T("%d"), nTemp);
							if (sLine.length() == 0)
							{
								sLine.append(_T("\""));
								sLine.append(sTemp.c_str());
								sLine.append(_T("\""));
							}
							else
							{
								sLine.append(_T(",\""));
								sLine.append(sTemp.c_str());
								sLine.append(_T("\""));
							}
						}
						else
						{
							GetXMLGen().AddChildElem(_T("Cell"), (int)*ColumnData[j]);
						}
					}
					else if (ColumnDataType[j] == SQL_LONGVARCHAR)
					{
						SQLRETURN retcode2_local;
						do {
							retcode2_local = SQLGetData(
								hstmt,
								j + 1,
								SQL_CHAR,
								buffer,
								MAX_CHAR,
								&length);

							sData.append(buffer);
							buffer[0] = 0;
						} while (retcode2_local == SQL_SUCCESS_WITH_INFO);
						if (CheckError(retcode2_local, "SQLGetData()", hstmt, SQL_HANDLE_STMT))
						{
							return 1;
						}

						if (m_OutputCDF)
						{
							sTemp.Format(_T("%s"), sData.c_str());
							sTemp.EscapeCSV();
							if (sLine.length() == 0)
							{
								sLine.append(_T("\""));
								sLine.append(sTemp.c_str());
								sLine.append(_T("\""));
							}
							else
							{
								sLine.append(_T(",\""));
								sLine.append(sTemp.c_str());
								sLine.append(_T("\""));
							}
						}
						else
						{
							sTemp.Format(_T("%s"), sData.c_str());
							GetXMLGen().AddChildElem(_T("Cell"), (const char*)sTemp.c_str());
						}
						sData.clear();
					}
					else
					{
						if (m_OutputCDF)
						{
							sTemp.Format(_T("%s"), (const char*)ColumnData[j]);
							CEMSString sTemp2;
							sTemp2.Format(_T("%s"), (const char*)ColumnName[j]);
							if (sTemp2.find(_T("pass")) != CEMSString::npos || sTemp2.find(_T("Pass")) != CEMSString::npos)
							{
								sTemp.Format(_T("********"));
							}
							else
							{
								sTemp.EscapeCSV();
							}
							if (sLine.length() == 0)
							{
								sLine.append(_T("\""));
								sLine.append(sTemp.c_str());
								sLine.append(_T("\""));
							}
							else
							{
								sLine.append(_T(",\""));
								sLine.append(sTemp.c_str());
								sLine.append(_T("\""));
							}
						}
						else
						{
							sTemp.Format(_T("%s"), (const char*)ColumnData[j]);
							CEMSString sTemp2;
							sTemp2.Format(_T("%s"), (const char*)ColumnName[j]);
							if (sTemp2.find(_T("pass")) != CEMSString::npos || sTemp2.find(_T("Pass")) != CEMSString::npos)
							{
								sTemp.Format(_T("********"));
							}
							GetXMLGen().AddChildElem(_T("Cell"), (const char*)sTemp.c_str());
						}
					}
				}
				if (m_OutputCDF)
				{
					sLine.append(_T("\r\n"));
					OutputCDF(sLine);
					sLine.clear();
				}
				else
				{
					GetXMLGen().OutOfElem();
				}
				numRows = i + 1;
			}
			if (!m_OutputCDF)
			{
				GetXMLGen().OutOfElem();
			}

			for (i = 0; i < numCols; i++) {
				if (ColumnName[i] != NULL) free(ColumnName[i]);
				if (ColumnData[i] != NULL) free(ColumnData[i]);
			}
		}
		else
		{
			retcode = SQLRowCount(hstmt, &numRowsAffected);
			if (CheckError(retcode, "SQLRowCount()", hstmt, SQL_HANDLE_STMT))
			{
				return 1;
			}
		}

		// Free handles
		// Statement
		if (hstmt != SQL_NULL_HSTMT)
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

		// Connection
		if (hdbc != SQL_NULL_HDBC) {
			SQLDisconnect(hdbc);
			SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		}

		// Environment
		if (henv != SQL_NULL_HENV)
			SQLFreeHandle(SQL_HANDLE_ENV, henv);

	}
	catch (ODBCError_t error)
	{
		GetRoutingEngine().LogIt(EMSERROR(EMS_LOG_SEVERITY_ERROR,
			EMSERR_ISAPI_EXTENSION,
			EMS_LOG_DATABASE_MAINTENANCE,
			E_ODBCError),
			"ODBC Error running SQL Query: %s",
			error.szErrMsg);

		if (m_OutputCDF)
		{
			CloseCDF();
			DeleteFile(m_szTempFile);
		}
		return 1;
	}
	catch (...)
	{
		GetRoutingEngine().LogIt(EMSERROR(EMS_LOG_SEVERITY_ERROR,
			EMSERR_ISAPI_EXTENSION,
			EMS_LOG_DATABASE_MAINTENANCE,
			E_ODBCError),
			"Unhandled exception running SQL Query");

		if (m_OutputCDF)
		{
			CloseCDF();
			DeleteFile(m_szTempFile);
		}
		return 1;
	}

	if (m_OutputCDF)
	{
		CloseCDF();
	}
	else
	{
		GetXMLGen().AddChildElem(_T("SqlQuery"));
		GetXMLGen().AddChildAttrib(_T("Server"), sServer.c_str());
		GetXMLGen().AddChildAttrib(_T("Database"), sDatabase.c_str());
		GetXMLGen().AddChildAttrib(_T("User"), sUser.c_str());
		GetXMLGen().AddChildAttrib(_T("Password"), sPassword.c_str());
		GetXMLGen().AddChildAttrib(_T("QueryText"), sQuery.c_str());
		GetXMLGen().AddChildAttrib(_T("NumRows"), numRows);
		GetXMLGen().AddChildAttrib(_T("NumRowsAffected"), numRowsAffected);
	}
	return 0;
}

void CDBMaintenance::VerifyDirectoryExists(tstring& sFolder)
{
	CEMSString sFilename;
	SYSTEMTIME now;
	int n = 99;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	if (sFolder.size() < 1)
	{
		CEMSString sError;
		sError.Format(_T("Please provide a valid directory."), sFolder.c_str());
		THROW_EMS_EXCEPTION(E_SystemError, sError);
	}

	GetLocalTime(&now);

	sFilename.Format(_T("%s\\VMF-%d-%d-%d-%d.TST"), sFolder.c_str(),
		now.wYear, now.wMonth, now.wDay, n);

	hFile = CreateFile(sFilename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwErr = GetLastError();

		if (dwErr != ERROR_ALREADY_EXISTS && dwErr != ERROR_FILE_EXISTS)
		{
			CEMSString sError;
			sError.Format(_T("%s is not a valid directory or the permissions are incorrect."), sFolder.c_str());
			THROW_EMS_EXCEPTION(E_SystemError, sError);
		}
	}
	else
	{
		CloseHandle(hFile);
		DeleteFile(sFilename.c_str());
	}
}

void CDBMaintenance::Update(void)
{
	DebugReporter::Instance().DisplayMessage("Entering CDBMaintenance::Update function", DebugReporter::ISAPI, GetCurrentThreadId());

	try
	{
		CEMSString sValue;
		tstring sOne = _T("1");
		tstring sZero = _T("0");

		SetServerParameter(EMS_SRVPARAM_ENABLE_DAILY_MAINTENANCE,
			m_sDaysOfWeek.length() ? m_sDaysOfWeek : sZero);

		SetServerParameter(EMS_SRVPARAM_ENABLE_DAILY_BACKUPS,
			m_bDailyBackupsEnabled ? sOne : sZero);

		SetServerParameter(EMS_SRVPARAM_ENABLE_DAILY_ARCHIVES,
			m_bDailyArchivesEnabled ? sOne : sZero);

		sValue.Format(_T("%d"), m_nDailyPurgesEnabled);
		SetServerParameter(EMS_SRVPARAM_ENABLE_DAILY_PURGES, sValue);

		if (m_bDailyMaintenanceEnabled)
		{
			SetServerParameter(EMS_SRVPARAM_DAILY_MAINT_RUN_TIME, m_sDailyRunTime);
		}

		sValue.Format(_T("%d"), m_nPurgeCutoffDays);
		SetServerParameter(EMS_SRVPARAM_PURGE_CUTOFF, sValue);

		dca::WString location(EMS_LOCATION_IN_REGISTRY);
		dca::LocalMachineRegKey lmrk(location.c_str(), dca::RegKey::CREATE, KEY_WRITE);

		if (m_bDailyArchivesEnabled)
		{
			VerifyDirectoryExists(m_sArchiveFolder);

			sValue.Format(_T("%d"), m_nArchiveCutoffDays);
			SetServerParameter(EMS_SRVPARAM_ARCHIVE_CUTOFF, sValue);

			sValue.Format(_T("%d"), m_nArchiveMaxTickets);
			SetServerParameter(EMS_SRVPARAM_ARCHIVE_MAX, sValue);

			{
				dca::WString arcPath(EMS_ARCHIVE_PATH_VALUE);
				dca::WString value(m_sArchiveFolder.c_str());

				lmrk.SetStringValue(arcPath.c_str(), value);
				SetServerParameter(EMS_SRVPARAM_ARCHIVE_PATH, m_sArchiveFolder);
			}

			sValue.Format(_T("%d"), m_nArchiveRemoveTickets);
			SetServerParameter(EMS_SRVPARAM_ARCHIVE_REMOVE_TICKETS, sValue);

			SetServerParameter(EMS_SRVPARAM_ARCHIVE_ACCESS,
				m_bArchiveAllowAccess ? sOne : sZero);
		}

		if (m_bDailyBackupsEnabled)
		{
			VerifyDirectoryExists(m_sBackupFolder);

			{
				dca::WString backupPath(EMS_BACKUP_PATH_VALUE);
				dca::WString value(m_sBackupFolder.c_str());

				lmrk.SetStringValue(backupPath.c_str(), value);
				SetServerParameter(EMS_SRVPARAM_BACKUP_PATH, m_sBackupFolder);
			}

			sValue.Format(_T("%d"), m_nDeleteBackups);
			SetServerParameter(EMS_SRVPARAM_DELETE_BACKUPS, sValue);
		}

		m_ISAPIData.m_RoutingEngine.ReloadConfig(EMS_ServerParameters);
		InvalidateServerParameters(true);

		{
			dca::WString maintLow(L"NextMaintenanceLow");
			lmrk.SetDWORDValue(maintLow.c_str(), 0);
		}


		{
			dca::WString maintHigh(L"NextMaintenanceHigh");
			lmrk.SetDWORDValue(maintHigh.c_str(), 0);
		}
	}
	catch (dca::Exception& e)
	{
		dca::String o;
		dca::String err(e.GetMessage());
		o.Format("CDBMaintenance::Update - %s", err.c_str());
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

void CDBMaintenance::QueryDB(void)
{
	tstring sValue;

	// Rebuild the server parameters (in case the last run time has changed)
	InvalidateServerParameters(true);

	GetServerParameter(EMS_SRVPARAM_ENABLE_DAILY_MAINTENANCE, m_sDaysOfWeek);
	m_bDailyMaintenanceEnabled = (_tcsicmp(m_sDaysOfWeek.c_str(), _T("0")) == 0) ? false : true;

	GetServerParameter(EMS_SRVPARAM_ENABLE_DAILY_BACKUPS, sValue);
	m_bDailyBackupsEnabled = (_ttoi(sValue.c_str()) == 0) ? false : true;

	GetServerParameter(EMS_SRVPARAM_ENABLE_DAILY_ARCHIVES, sValue);
	m_bDailyArchivesEnabled = (_ttoi(sValue.c_str()) == 0) ? false : true;

	GetServerParameter(EMS_SRVPARAM_ENABLE_DAILY_PURGES, sValue);
	m_nDailyPurgesEnabled = _ttoi(sValue.c_str());

	GetServerParameter(EMS_SRVPARAM_DAILY_MAINT_RUN_TIME, m_sDailyRunTime);
	GetServerParameter(EMS_SRVPARAM_PURGE_LASTRUN, m_sLastPurgeTime);
	GetServerParameter(EMS_SRVPARAM_ARCHIVE_LASTRUN, m_sLastArchiveTime);
	GetServerParameter(EMS_SRVPARAM_BACKUP_LASTRUN, m_sLastBackupTime);

	GetServerParameter(EMS_SRVPARAM_PURGE_CUTOFF, sValue);
	m_nPurgeCutoffDays = _ttoi(sValue.c_str());

	GetServerParameter(EMS_SRVPARAM_ARCHIVE_CUTOFF, sValue);
	m_nArchiveCutoffDays = _ttoi(sValue.c_str());

	GetServerParameter(EMS_SRVPARAM_ARCHIVE_MAX, sValue);
	m_nArchiveMaxTickets = _ttoi(sValue.c_str());

	GetServerParameter(EMS_SRVPARAM_ARCHIVE_ACCESS, sValue);
	m_bArchiveAllowAccess = (_ttoi(sValue.c_str()) == 0) ? false : true;

	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_ARCHIVE_PATH_VALUE, m_sArchiveFolder);
	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_BACKUP_PATH_VALUE, m_sBackupFolder);

	GetServerParameter(EMS_SRVPARAM_ARCHIVE_REMOVE_TICKETS, sValue);
	m_nArchiveRemoveTickets = _ttoi(sValue.c_str());

	GetServerParameter(EMS_SRVPARAM_DELETE_BACKUPS, sValue);
	m_nDeleteBackups = _ttoi(sValue.c_str());
}

void CDBMaintenance::DecodeForm(void)
{
	unsigned char Bit = 0;
	tstring sAmPm;
	tstring sValue;

	GetISAPIData().GetXMLLong(_T("Enabled"), Bit);
	m_bDailyMaintenanceEnabled = (Bit == 0) ? false : true;

	GetISAPIData().GetXMLLong(_T("DoArchive"), Bit);
	m_bDailyArchivesEnabled = (Bit == 0) ? false : true;

	m_nDailyPurgesEnabled = 0;

	GetISAPIData().GetXMLLong(_T("PurgeTickets"), Bit);
	m_nDailyPurgesEnabled += Bit * EMS_PURGE_TICKETS;

	GetISAPIData().GetXMLLong(_T("PurgeTrash"), Bit);
	m_nDailyPurgesEnabled += Bit * EMS_PURGE_TRASH;

	GetISAPIData().GetXMLLong(_T("PurgeStdRspUsage"), Bit);
	m_nDailyPurgesEnabled += Bit * EMS_PURGE_STDRSPUSAGE;

	GetISAPIData().GetXMLLong(_T("PurgeAlerts"), Bit);
	m_nDailyPurgesEnabled += Bit * EMS_PURGE_ALERTS;

	GetISAPIData().GetXMLLong(_T("PurgeContacts"), Bit);
	m_nDailyPurgesEnabled += Bit * EMS_PURGE_CONTACTS;

	GetISAPIData().GetXMLLong(_T("DoBackup"), Bit);
	m_bDailyBackupsEnabled = (Bit == 0) ? false : true;

	if (m_bDailyArchivesEnabled)
	{
		GetISAPIData().GetXMLString(_T("ArchivePath"), m_sArchiveFolder);
		GetISAPIData().GetXMLLong(_T("ArchiveCutoff"), m_nArchiveCutoffDays);
		GetISAPIData().GetXMLLong(_T("ArchiveMax"), m_nArchiveMaxTickets, true);
		GetISAPIData().GetXMLLong(_T("archiveRemove"), m_nArchiveRemoveTickets);
		GetISAPIData().GetXMLLong(_T("archiveAccess"), Bit, true);
		m_bArchiveAllowAccess = (Bit == 0) ? false : true;
	}
	else
	{
		GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_ARCHIVE_PATH_VALUE, m_sArchiveFolder);
		GetServerParameter(EMS_SRVPARAM_ARCHIVE_CUTOFF, sValue);
		m_nArchiveCutoffDays = _ttoi(sValue.c_str());
	}

	GetISAPIData().GetXMLLong(_T("PurgeCutoff"), m_nPurgeCutoffDays);

	if (m_bDailyBackupsEnabled)
	{
		GetISAPIData().GetXMLString(_T("BackupPath"), m_sBackupFolder);
		GetISAPIData().GetXMLLong(_T("DeleteBackups"), m_nDeleteBackups);
	}
	else
	{
		GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_BACKUP_PATH_VALUE, m_sBackupFolder);
	}

	if (m_bDailyMaintenanceEnabled)
	{
		GetISAPIData().GetXMLLong(_T("Sun"), m_nSun);
		GetISAPIData().GetXMLLong(_T("Mon"), m_nMon);
		GetISAPIData().GetXMLLong(_T("Tue"), m_nTue);
		GetISAPIData().GetXMLLong(_T("Wed"), m_nWed);
		GetISAPIData().GetXMLLong(_T("Thu"), m_nThu);
		GetISAPIData().GetXMLLong(_T("Fri"), m_nFri);
		GetISAPIData().GetXMLLong(_T("Sat"), m_nSat);

		if (m_nSun)
		{
			m_sDaysOfWeek.assign(_T("SU"));
		}
		if (m_nMon)
		{
			if (m_sDaysOfWeek.length()) { m_sDaysOfWeek.append(_T(":")); }
			m_sDaysOfWeek.append(_T("MO"));
		}
		if (m_nTue)
		{
			if (m_sDaysOfWeek.length()) { m_sDaysOfWeek.append(_T(":")); }
			m_sDaysOfWeek.append(_T("TU"));
		}
		if (m_nWed)
		{
			if (m_sDaysOfWeek.length()) { m_sDaysOfWeek.append(_T(":")); }
			m_sDaysOfWeek.append(_T("WE"));
		}
		if (m_nThu)
		{
			if (m_sDaysOfWeek.length()) { m_sDaysOfWeek.append(_T(":")); }
			m_sDaysOfWeek.append(_T("TH"));
		}
		if (m_nFri)
		{
			if (m_sDaysOfWeek.length()) { m_sDaysOfWeek.append(_T(":")); }
			m_sDaysOfWeek.append(_T("FR"));
		}
		if (m_nSat)
		{
			if (m_sDaysOfWeek.length()) { m_sDaysOfWeek.append(_T(":")); }
			m_sDaysOfWeek.append(_T("SA"));
		}

		GetISAPIData().GetXMLString(_T("Time"), m_sDailyRunTime);
		GetISAPIData().GetXMLString(_T("AMPM"), sAmPm);

		m_sDailyRunTime += _T(" ");
		m_sDailyRunTime += sAmPm;
	}
	else
	{
		GetServerParameter(EMS_SRVPARAM_DAILY_MAINT_RUN_TIME, m_sDailyRunTime);
	}
}

void CDBMaintenance::GenerateXML(void)
{
	GetXMLGen().AddChildElem(_T("DailyOptions"));
	GetXMLGen().AddChildAttrib(_T("Enabled"), m_bDailyMaintenanceEnabled);
	if (m_bDailyMaintenanceEnabled)
	{
		tstring::size_type pos;
		pos = m_sDaysOfWeek.find(_T("SU"), 0);
		if (pos != tstring::npos)
		{
			GetXMLGen().AddChildAttrib(_T("Sun"), 1);
		}
		pos = m_sDaysOfWeek.find(_T("MO"), 0);
		if (pos != tstring::npos)
		{
			GetXMLGen().AddChildAttrib(_T("Mon"), 1);
		}
		pos = m_sDaysOfWeek.find(_T("TU"), 0);
		if (pos != tstring::npos)
		{
			GetXMLGen().AddChildAttrib(_T("Tue"), 1);
		}
		pos = m_sDaysOfWeek.find(_T("WE"), 0);
		if (pos != tstring::npos)
		{
			GetXMLGen().AddChildAttrib(_T("Wed"), 1);
		}
		pos = m_sDaysOfWeek.find(_T("TH"), 0);
		if (pos != tstring::npos)
		{
			GetXMLGen().AddChildAttrib(_T("Thu"), 1);
		}
		pos = m_sDaysOfWeek.find(_T("FR"), 0);
		if (pos != tstring::npos)
		{
			GetXMLGen().AddChildAttrib(_T("Fri"), 1);
		}
		pos = m_sDaysOfWeek.find(_T("SA"), 0);
		if (pos != tstring::npos)
		{
			GetXMLGen().AddChildAttrib(_T("Sat"), 1);
		}
	}
	GetXMLGen().AddChildAttrib(_T("DoBackup"), m_bDailyBackupsEnabled);
	GetXMLGen().AddChildAttrib(_T("DoArchive"), m_bDailyArchivesEnabled);

	GetXMLGen().AddChildAttrib(_T("PurgeTickets"),
		(m_nDailyPurgesEnabled & EMS_PURGE_TICKETS) ? 1 : 0);
	GetXMLGen().AddChildAttrib(_T("PurgeTrash"),
		(m_nDailyPurgesEnabled & EMS_PURGE_TRASH) ? 1 : 0);
	GetXMLGen().AddChildAttrib(_T("PurgeStdRspUsage"),
		(m_nDailyPurgesEnabled & EMS_PURGE_STDRSPUSAGE) ? 1 : 0);
	GetXMLGen().AddChildAttrib(_T("PurgeAlerts"),
		(m_nDailyPurgesEnabled & EMS_PURGE_ALERTS) ? 1 : 0);
	GetXMLGen().AddChildAttrib(_T("PurgeContacts"),
		(m_nDailyPurgesEnabled & EMS_PURGE_CONTACTS) ? 1 : 0);

	GetXMLGen().AddChildAttrib(_T("Time"), m_sDailyRunTime.c_str());

	GetXMLGen().AddChildElem(_T("PurgeOptions"));
	GetXMLGen().AddChildAttrib(_T("CutoffDays"), m_nPurgeCutoffDays);
	GetXMLGen().AddChildAttrib(_T("LastRun"), m_sLastPurgeTime.c_str());

	GetXMLGen().AddChildElem(_T("ArchiveOptions"));
	GetXMLGen().AddChildAttrib(_T("CutoffDays"), m_nArchiveCutoffDays);
	GetXMLGen().AddChildAttrib(_T("ArchiveMax"), m_nArchiveMaxTickets);
	GetXMLGen().AddChildAttrib(_T("RemoveTickets"), m_nArchiveRemoveTickets);
	GetXMLGen().AddChildAttrib(_T("Path"), m_sArchiveFolder.c_str());
	GetXMLGen().AddChildAttrib(_T("LastRun"), m_sLastArchiveTime.c_str());
	GetXMLGen().AddChildAttrib(_T("ArchiveAccess"), m_bArchiveAllowAccess);

	GetXMLGen().AddChildElem(_T("BackupOptions"));
	GetXMLGen().AddChildAttrib(_T("Path"), m_sBackupFolder.c_str());
	GetXMLGen().AddChildAttrib(_T("LastRun"), m_sLastBackupTime.c_str());
	GetXMLGen().AddChildAttrib(_T("DeleteBackups"), m_nDeleteBackups);
}

////////////////////////////////////////////////////////////////////////////////
//
// Search the backukp directory for backup files and return XML
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::ListBackupFiles(void)
{
	TCHAR szPath[MAX_PATH];
	tstring tstrDir;
	CLogFileInfo logFileInfo;
	HANDLE hFind;
	FILETIME ft;
	SYSTEMTIME st;
	CEMSString sDateTime;
	set<CLogFileInfo> LogFileList;
	set<CLogFileInfo>::iterator iter;
	int nPage = 1;
	int nLine = 0;
	int nCurrentPage = 1;
	CEMSString sBytes;

	GetISAPIData().GetURLLong(_T("CurrentPage"), nCurrentPage, true);

	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_BACKUP_PATH_VALUE, m_sBackupFolder);
	GetXMLGen().AddChildElem(_T("BackupOptions"));
	GetXMLGen().AddChildAttrib(_T("Path"), m_sBackupFolder.c_str());

	_stprintf(szPath, _T("%s\\*.BAK"), m_sBackupFolder.c_str());

	hFind = FindFirstFile(szPath, &logFileInfo);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		LogFileList.insert(logFileInfo);

		while (FindNextFile(hFind, &logFileInfo))
		{
			LogFileList.insert(logFileInfo);
		}

		FindClose(hFind);
	}

	for (iter = LogFileList.begin(); iter != LogFileList.end(); iter++)
	{
		nLine++;

		if (nLine >= GetSession().m_nMaxRowsPerPage)
		{
			nLine = 0;
			nPage++;
		}

		if (nPage == nCurrentPage)
		{
			GetXMLGen().AddChildElem(_T("Backup"));
			GetXMLGen().AddChildAttrib(_T("Filename"), iter->cFileName);

			FileTimeToLocalFileTime(&(iter->ftLastWriteTime), &ft);
			FileTimeToSystemTime(&ft, &st);

			GetDateTimeString(st, sDateTime);

			GetXMLGen().AddChildAttrib(_T("Date"), sDateTime.c_str());

			sBytes.FormatBytes(iter->nFileSizeLow, iter->nFileSizeHigh);
			GetXMLGen().AddChildAttrib(_T("Bytes"), sBytes.c_str());
		}
	}

	GetXMLGen().AddChildElem(_T("Page"));
	GetXMLGen().AddChildAttrib(_T("Current"), nCurrentPage);
	GetXMLGen().AddChildAttrib(_T("Count"), nPage);

}


////////////////////////////////////////////////////////////////////////////////
//
// Search the Archives table and return the results in XML
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::ListArchives(void)
{
	CEMSString sDate;
	TArchives arc;
	int nPage = 1;
	int nLine = 0;
	int nCurrentPage = 1;
	tstring sFileSize;
	CLogFileInfo logFileInfo;
	list<CLogFileInfo> LogFileList;
	list<CLogFileInfo>::iterator iter;
	HANDLE hFind;
	TCHAR szPath[MAX_PATH];
	FILETIME ft;
	SYSTEMTIME st;
	tstring sFileName;

	GetISAPIData().GetURLLong(_T("CurrentPage"), nCurrentPage, true);

	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_ARCHIVE_PATH_VALUE, m_sArchiveFolder);
	GetXMLGen().AddChildElem(_T("ArchiveOptions"));
	GetXMLGen().AddChildAttrib(_T("Path"), m_sArchiveFolder.c_str());

	// See what's on the filesystem
	_stprintf(szPath, _T("%s\\*.ARC"), m_sArchiveFolder.c_str());

	hFind = FindFirstFile(szPath, &logFileInfo);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		logFileInfo.dwReserved0 = 0;
		LogFileList.push_back(logFileInfo);

		while (FindNextFile(hFind, &logFileInfo))
		{
			logFileInfo.dwReserved0 = 0;
			LogFileList.push_back(logFileInfo);
		}

		FindClose(hFind);
	}


	GetQuery().Initialize();
	BINDCOL_LONG(GetQuery(), arc.m_ArchiveID);
	BINDCOL_TIME(GetQuery(), arc.m_DateCreated);
	BINDCOL_TCHAR(GetQuery(), arc.m_ArcFilePath);
	BINDCOL_LONG(GetQuery(), arc.m_InMsgRecords);
	BINDCOL_LONG(GetQuery(), arc.m_InAttRecords);
	BINDCOL_LONG(GetQuery(), arc.m_OutMsgRecords);
	BINDCOL_LONG(GetQuery(), arc.m_OutAttRecords);

	GetQuery().Execute(_T("SELECT ArchiveID,DateCreated,ArcFilePath,InMsgRecords,InAttRecords,OutMsgRecords,OutAttRecords ")
		_T("FROM Archives ")
		_T("ORDER BY DateCreated DESC"));


	while (GetQuery().Fetch() == S_OK)
	{
		nLine++;

		// Strip leading path if the same as ArchiveFolder
		if (_tcsnicmp(arc.m_ArcFilePath, m_sArchiveFolder.c_str(), m_sArchiveFolder.size()) == 0)
		{
			// Strip the "\" character
			if (arc.m_ArcFilePath[m_sArchiveFolder.size()] == _T('\\'))
			{
				sFileName = arc.m_ArcFilePath + m_sArchiveFolder.size() + 1;
			}
			else
			{
				sFileName = arc.m_ArcFilePath + m_sArchiveFolder.size();
			}
		}
		else
		{
			sFileName.assign(arc.m_ArcFilePath);
		}


		for (iter = LogFileList.begin(); iter != LogFileList.end(); iter++)
		{
			if (_tcsicmp(sFileName.c_str(), iter->cFileName) == 0
				|| _tcsicmp(arc.m_ArcFilePath, iter->cFileName) == 0)
			{
				iter->dwReserved0 = 1;
			}
		}

		if (nLine >= GetSession().m_nMaxRowsPerPage)
		{
			nLine = 0;
			nPage++;
		}

		if (nPage == nCurrentPage)
		{
			GetXMLGen().AddChildElem(_T("Archive"));
			GetXMLGen().AddChildAttrib(_T("Filename"), sFileName.c_str());

			GetDateTimeString(arc.m_DateCreated, arc.m_DateCreatedLen, sDate);
			GetXMLGen().AddChildAttrib(_T("Date"), sDate.c_str());
			GetXMLGen().AddChildAttrib(_T("ID"), arc.m_ArchiveID);
			GetXMLGen().AddChildAttrib(_T("InMsg"), arc.m_InMsgRecords);
			GetXMLGen().AddChildAttrib(_T("InAtt"), arc.m_InAttRecords);
			GetXMLGen().AddChildAttrib(_T("OutMsg"), arc.m_OutMsgRecords);
			GetXMLGen().AddChildAttrib(_T("OutAtt"), arc.m_OutAttRecords);
			GetFileSize(arc.m_ArcFilePath, sFileSize);
			GetXMLGen().AddChildAttrib(_T("Size"), sFileSize.c_str());
		}
	}

	// Create any archives that were found in the archive path, but are
	// not in the archives table
	for (iter = LogFileList.begin(); iter != LogFileList.end(); iter++)
	{
		if (iter->dwReserved0 == 0)
		{

			_tcscpy(arc.m_ArcFilePath, m_sArchiveFolder.c_str());
			if (m_sArchiveFolder.size() > 0 && m_sArchiveFolder.at(m_sArchiveFolder.size() - 1) != _T('\\'))
			{
				_tcscat(arc.m_ArcFilePath, "\\");
			}
			_tcscat(arc.m_ArcFilePath, iter->cFileName);

			FileTimeToLocalFileTime(&(iter->ftLastWriteTime), &ft);
			FileTimeToSystemTime(&ft, &st);

			SystemTimeToTimeStamp(st, arc.m_DateCreated);
			arc.m_DateCreatedLen = sizeof(TIMESTAMP_STRUCT);

			arc.m_InAttRecords = 0;
			arc.m_OutAttRecords = 0;
			arc.m_InMsgRecords = 0;
			arc.m_OutMsgRecords = 0;
			arc.m_Purged = 0;

			arc.Insert(GetQuery());

			nLine++;

			if (nLine >= GetSession().m_nMaxRowsPerPage)
			{
				nLine = 0;
				nPage++;
			}

			if (nPage == nCurrentPage)
			{
				GetXMLGen().AddChildElem(_T("Archive"));
				GetXMLGen().AddChildAttrib(_T("Filename"), iter->cFileName);
				GetDateTimeString(arc.m_DateCreated, arc.m_DateCreatedLen, sDate);
				GetXMLGen().AddChildAttrib(_T("Date"), sDate.c_str());
				GetXMLGen().AddChildAttrib(_T("ID"), arc.m_ArchiveID);
				GetXMLGen().AddChildAttrib(_T("InMsg"), 0);
				GetXMLGen().AddChildAttrib(_T("InAtt"), 0);
				GetXMLGen().AddChildAttrib(_T("OutMsg"), 0);
				GetXMLGen().AddChildAttrib(_T("OutAtt"), 0);
				GetFileSize(arc.m_ArcFilePath, sFileSize);
				GetXMLGen().AddChildAttrib(_T("Size"), sFileSize.c_str());
			}
		}
	}


	GetXMLGen().AddChildElem(_T("Page"));
	GetXMLGen().AddChildAttrib(_T("Current"), nCurrentPage);
	GetXMLGen().AddChildAttrib(_T("Count"), nPage);
}


////////////////////////////////////////////////////////////////////////////////
//
// GetFileSize
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::GetFileSize(LPCSTR szFileName, tstring& sFileSize)
{
	tstring sFullPath;
	HANDLE hFile;
	FILETIME ft;
	CEMSString sSize;

	ZeroMemory(&ft, sizeof(ft));

	hFile = CreateFile(szFileName, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		ft.dwLowDateTime = ::GetFileSize(hFile, &ft.dwHighDateTime);

		sSize.FormatBytes(ft.dwLowDateTime, ft.dwHighDateTime);

		sFileSize.assign(sSize);

		CloseHandle(hFile);
	}
	else
	{
		sFileSize = "file not found";
	}
}



////////////////////////////////////////////////////////////////////////////////
//
// DoNow
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::DoNow(CURLAction& action)
{
	LaunchDBMaintThread(Job_Database_Maintenance);
}

////////////////////////////////////////////////////////////////////////////////
//
// DoRestore
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::DoRestore(CURLAction& action)
{
	CEMSString sBackupFiles;
	tstring sbackupfile;

	GetISAPIData().GetXMLString(_T("SELECTID"), sbackupfile);

	// we only want the first file in the list
	GetISAPIData().GetXMLString(_T("SELECTID"), sBackupFiles);
	sBackupFiles.CDLGetNextString(sbackupfile);

	if (sbackupfile.compare(_T("0")) == 0)
		return;

	GetXMLCache().InvalidateAllMaps();
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3, 0);

	LaunchDBMaintThread(Job_Restore, (void*)sbackupfile.c_str());
}

////////////////////////////////////////////////////////////////////////////////
//
// DoImport
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::DoImport(CURLAction& action)
{
	int archiveid = 0;

	GetISAPIData().GetXMLLong(_T("SELECTID"), archiveid);

	if (archiveid == 0)
		return;

	GetXMLCache().InvalidateAllMaps();
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3, 0);

	LaunchDBMaintThread(Job_Import, (void*)archiveid);
}

////////////////////////////////////////////////////////////////////////////////
//
// LaunchDBMaintThread
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::LaunchDBMaintThread(int nJob, void* pData)
{
	//DEBUGPRINT(_T("EMSIsapi::CDBMaintenance::LaunchDBMaintThread - entered"));

	CDBMaintThread* pThread = new CDBMaintThread(GetISAPIData().m_SessionMap);

	pThread->m_nJob = nJob;
	pThread->m_AgentID = GetSession().m_AgentID;
	pThread->SetKillEvent(GetISAPIData().m_hKillEvent);
	GetISAPIData().m_SessionMap.SetDBMaintResults(pThread->m_AgentID, nJob);

	pThread->m_pDBMaintRunning = &(g_ThreadPool.GetSharedObjects().m_bDBMaintenanceRunning);

	switch (nJob)
	{
	case Job_Database_Maintenance:
		break;

	case Job_Import:
		pThread->m_nArchiveID = (unsigned int)pData;
		break;

	case Job_Restore:
		pThread->m_backupfile = (TCHAR*)pData;
		break;
	}

	// Start the thread
	pThread->StartThread();

}


#define	E_DBMaintErrorUnhandled EMSERROR(EMS_LOG_SEVERITY_ERROR,EMSERR_ROUTING_ENGINE,EMS_LOG_DATABASE_MAINTENANCE,13)
#define	E_DBMaintErrorWritingToFile	EMSERROR(EMS_LOG_SEVERITY_ERROR,EMSERR_ROUTING_ENGINE,EMS_LOG_DATABASE_MAINTENANCE,14)
#define	E_DBMaintErrorODBC EMSERROR(EMS_LOG_SEVERITY_ERROR,EMSERR_ROUTING_ENGINE,EMS_LOG_DATABASE_MAINTENANCE,15)
#define E_DBMaintErrorArchive EMSERROR(EMS_LOG_SEVERITY_ERROR,EMSERR_ROUTING_ENGINE,EMS_LOG_DATABASE_MAINTENANCE,16)
#define E_DBMaintErrorRestore EMSERROR(EMS_LOG_SEVERITY_ERROR,EMSERR_ROUTING_ENGINE,EMS_LOG_DATABASE_MAINTENANCE,17)
#define E_DBMaintErrorImport EMSERROR(EMS_LOG_SEVERITY_ERROR,EMSERR_ROUTING_ENGINE,EMS_LOG_DATABASE_MAINTENANCE,18)



////////////////////////////////////////////////////////////////////////////////
//
// ShowResults
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::ShowResults(CURLAction& action)
{
	tstring sRegValue;
	GetXMLGen().AddChildElem(_T("Results"));

	//int nMaintProg = GetSession().m_nDBMaintProgress;
	int nMaintProg;
	UINT nCode;
	if (GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("MaintenanceProg"), nCode) == ERROR_SUCCESS)
	{
		nMaintProg = nCode;
		GetXMLGen().AddChildAttrib(_T("Code"), nMaintProg);
	}

	if (nMaintProg < 0 || nMaintProg > Job_Finished_Successfully)
	{
		GetXMLGen().AddChildAttrib(_T("Code"), Job_Finished_With_Error);
	}
	else
	{
		GetXMLGen().AddChildAttrib(_T("Code"), nMaintProg);
	}


	GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sRegValue);

	switch (nMaintProg)
	{
	case Job_Database_Maintenance:
		GetXMLGen().AddChildAttrib(_T("Description"), sRegValue.c_str());
		break;

	case Job_Restore:
		GetXMLGen().AddChildAttrib(_T("Description"), sRegValue.c_str());
		break;

	case Job_Import:
		GetXMLGen().AddChildAttrib(_T("Description"), sRegValue.c_str());
		break;

	case Job_Finished_Successfully:
		GetXMLGen().AddChildAttrib(_T("Description"), "Operation completed successfully");
		break;

	case E_DBMaintErrorUnhandled:
		GetXMLGen().AddChildAttrib(_T("Description"), "Unhandled exception, see log for details");
		break;

	case E_DBMaintErrorWritingToFile:
		GetXMLGen().AddChildAttrib(_T("Description"), "Error writing to file, see log for details");
		break;

	case E_DBMaintErrorODBC:
		GetXMLGen().AddChildAttrib(_T("Description"), "ODBC Error, see log for details");
		break;

	case E_DBMaintErrorArchive:
		GetXMLGen().AddChildAttrib(_T("Description"), "Error during archive, see log for details");
		break;

	case E_DBMaintErrorRestore:
		GetXMLGen().AddChildAttrib(_T("Description"), "Error during restore, see log for details");
		break;

	case E_DBMaintErrorImport:
		GetXMLGen().AddChildAttrib(_T("Description"), "Error during import, see log for details");
		break;

	default:
		GetXMLGen().AddChildAttrib(_T("Description"), "Please wait...");
		break;
	}
}


////////////////////////////////////////////////////////////////////////////////
//
// EditDBArchive
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::EditDBArchive(CURLAction& action)
{
	CEMSString sSelectID;
	int nArchiveID;
	TArchives arc;
	tstring sArcPath;
	CEMSString sDate;
	tstring sFileSize;

	GetISAPIData().GetURLString(_T("SELECTID"), sSelectID);

	if (!sSelectID.CDLGetNextInt(nArchiveID))
		THROW_EMS_EXCEPTION(E_InvalidID, _T("An archiveID must be specified"));

	arc.m_ArchiveID = nArchiveID;

	if (arc.Query(GetQuery()) != S_OK)
		THROW_EMS_EXCEPTION(E_InvalidID, CEMSString(EMS_STRING_INVALID_ID));

	if (GetISAPIData().GetFormString(_T("ArchivePath"), sArcPath, true))
	{
		DISABLE_IN_DEMO();

		int nID;

		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), nArchiveID);
		BINDPARAM_TCHAR_STRING(GetQuery(), sArcPath);
		BINDCOL_LONG_NOLEN(GetQuery(), nID);
		GetQuery().Execute(_T("SELECT ArchiveID FROM Archives WHERE ArchiveID<>? AND ArcFilePath=?"));

		if (GetQuery().Fetch() == S_OK)
			THROW_EMS_EXCEPTION(E_Duplicate_Name, _T("Archive paths must be unique"));

		GetQuery().Reset(true);
		BINDPARAM_TCHAR_STRING(GetQuery(), sArcPath);
		BINDPARAM_LONG(GetQuery(), nArchiveID);
		GetQuery().Execute(_T("UPDATE Archives SET ArcFilePath=? WHERE ArchiveID=?"));

		// TODO: Get the current page and pop it on
		action.SetRedirectURL(_T("dbarchives.ems"));

	}
	else
	{
		GetXMLGen().AddChildElem(_T("Archive"));
		GetDateTimeString(arc.m_DateCreated, arc.m_DateCreatedLen, sDate);
		GetXMLGen().AddChildAttrib(_T("Date"), sDate.c_str());
		GetXMLGen().AddChildAttrib(_T("ID"), arc.m_ArchiveID);
		GetXMLGen().AddChildAttrib(_T("InMsg"), arc.m_InMsgRecords);
		GetXMLGen().AddChildAttrib(_T("InAtt"), arc.m_InAttRecords);
		GetXMLGen().AddChildAttrib(_T("OutMsg"), arc.m_OutMsgRecords);
		GetXMLGen().AddChildAttrib(_T("OutAtt"), arc.m_OutAttRecords);
		GetFileSize(arc.m_ArcFilePath, sFileSize);
		GetXMLGen().AddChildAttrib(_T("Size"), sFileSize.c_str());
		GetXMLGen().SetChildData(arc.m_ArcFilePath, 1);
	}
}


void CDBMaintenance::DeleteArchive(void)
{
	DebugReporter::Instance().DisplayMessage("Entered CDBMaintenance::DeleteArchive", DebugReporter::ISAPI, GetCurrentThreadId());

	CEMSString sSelectID;
	int nArchiveID;
	TCHAR szArcPath[MAX_PATH];
	long szArcPathLen;

	ZeroMemory(szArcPath, sizeof(szArcPath));

	GetISAPIData().GetXMLString(_T("SELECTID"), sSelectID);

	while (sSelectID.CDLGetNextInt(nArchiveID))
	{
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), nArchiveID);
		BINDCOL_TCHAR(GetQuery(), szArcPath);
		GetQuery().Execute(_T("SELECT ArcFilePath FROM Archives WHERE ArchiveID=?"));

		if (GetQuery().Fetch() != S_OK)
		{
			THROW_EMS_EXCEPTION(E_InvalidID, CEMSString(EMS_STRING_INVALID_ID));
		}

		GetQuery().Reset(true);
		BINDPARAM_LONG(GetQuery(), nArchiveID);
		GetQuery().Execute(_T("DELETE FROM Archives WHERE ArchiveID=?"));

		// Delete from TicketHistory
		GetQuery().Reset(false);
		GetQuery().Execute(_T("DELETE FROM TicketHistory WHERE TicketActionID=8 AND AgentID=?"));

		if (!DeleteFile(szArcPath))
		{
			wchar_t mess[1025];
			wmemset(mess, 0x00, 1025);
			DWORD e = ::GetLastError();

			dca::Exception::FormatMsg(e, mess, 1024);

			dca::String o;
			dca::String m(mess);
			o.Format("CDBMaintenance::DeleteArchive - [ %d ] %s", e, m.c_str());
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
}

void CDBMaintenance::DeleteBackup(void)
{
	DebugReporter::Instance().DisplayMessage("Entered CDBMaintenance::DeleteBackup", DebugReporter::ISAPI, GetCurrentThreadId());

	CEMSString sBackupFiles;
	tstring sFileToDelete;
	tstring sChunk;

	GetISAPIData().GetXMLString(_T("SELECTID"), sBackupFiles);
	sBackupFiles.CDLInit();

	while (sBackupFiles.CDLGetNextString(sChunk))
	{
		GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_BACKUP_PATH_VALUE, sFileToDelete);

		if (sFileToDelete.size() > 0 && sFileToDelete.at(sFileToDelete.size() - 1) != _T('\\'))
		{
			sFileToDelete += _T("\\");
		}

		sFileToDelete += sChunk;

		if (!DeleteFile(sFileToDelete.c_str()))
		{
			wchar_t mess[1025];
			wmemset(mess, 0x00, 1025);
			DWORD e = ::GetLastError();

			dca::Exception::FormatMsg(e, mess, 1024);

			dca::String o;
			dca::String m(mess);
			o.Format("CDBMaintenance::DeleteBackup - [ %d ] %s", e, m.c_str());
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// OpenCDF
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::OpenCDF(CURLAction& action)
{
	dca::String f;
	DebugReporter::Instance().DisplayMessage("CReports::OpenCDF - Entering", DebugReporter::ISAPI, GetCurrentThreadId());

	// check security
	RequireAgentRightLevel(EMS_OBJECT_TYPE_STANDARD_REPORTS, 0, EMS_EDIT_ACCESS);

	TCHAR szTempPath[MAX_PATH];

	GetTempPath(MAX_PATH, szTempPath);
	GetTempFileName(szTempPath, _T("ems"), 0, m_szTempFile);

	f.Format("CReports::OpenCDF - Temp file set to [%s]", m_szTempFile);
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	m_hCDFFile = CreateFile(m_szTempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_hCDFFile == INVALID_HANDLE_VALUE)
	{
		DebugReporter::Instance().DisplayMessage("CReports::OpenCDF - Error creating temp file", DebugReporter::ISAPI, GetCurrentThreadId());
		THROW_EMS_EXCEPTION(E_SystemError, _T("Error creating temp file"));
	}

	action.SetSendTempFile(m_szTempFile, _T("queryresults.csv"));

	m_OutputCDF = true;

	DebugReporter::Instance().DisplayMessage("CReports::OpenCDF - Leaving", DebugReporter::ISAPI, GetCurrentThreadId());
}

////////////////////////////////////////////////////////////////////////////////
//
// OutputCDF
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::OutputCDF(tstring& sLine)
{
	DWORD dwBytes;

	WriteFile(m_hCDFFile, sLine.c_str(), sLine.size(), &dwBytes, NULL);
}

////////////////////////////////////////////////////////////////////////////////
//
// CloseCDF
//
////////////////////////////////////////////////////////////////////////////////
void CDBMaintenance::CloseCDF(void)
{
	if (m_OutputCDF)
		CloseHandle(m_hCDFFile);
}

////////////////////////////////////////////////////////////////////////////////
//
// CDBMaintThread constructor
//
////////////////////////////////////////////////////////////////////////////////
CDBMaintThread::CDBMaintThread(CSessionMap& SessionMap)
	: m_SessionMap(SessionMap), CThread(NULL)
{

}

////////////////////////////////////////////////////////////////////////////////
//
// CDBMaintThread destructor - deletes itself
//
////////////////////////////////////////////////////////////////////////////////
CDBMaintThread::~CDBMaintThread()
{
	delete this;
}


////////////////////////////////////////////////////////////////////////////////
//
// Run
//
////////////////////////////////////////////////////////////////////////////////
unsigned int CDBMaintThread::Run()
{
	BOOL bRet = FALSE;
	long nResult = -1;

	CoInitialize(NULL);

	CRoutingEngine* pRoutingEngine = new CRoutingEngine;

	switch (m_nJob)
	{
	case CDBMaintenance::Job_Database_Maintenance:
		if (pRoutingEngine)
		{
			bRet = pRoutingEngine->DoDBMaintenance(&nResult);
		}
		break;

	case CDBMaintenance::Job_Import:
		bRet = pRoutingEngine->RestoreArchive(m_nArchiveID, &nResult);
		break;

	case CDBMaintenance::Job_Restore:
	{
		wchar_t* pBackupFile = new wchar_t[m_backupfile.size() + 1];
		MultiByteToWideChar(CP_ACP, 0, m_backupfile.c_str(), m_backupfile.size() + 1, pBackupFile, m_backupfile.size() + 1);
		BSTR bstr = SysAllocString(pBackupFile);

		bRet = pRoutingEngine->RestoreBackup(bstr, &nResult);

		delete[] pBackupFile;
		SysFreeString(bstr);
		break;
	}
	}


	delete pRoutingEngine;

	CoUninitialize();

	if (WaitForSingleObject(m_hKillEvent, 0) == WAIT_OBJECT_0)
	{
		// If kill event is set, exit gracefully
		return 0;
	}

	if (bRet && nResult == 0)
	{
		m_nJob = CDBMaintenance::Job_Finished_Successfully;
	}
	else
	{
		m_nJob = nResult;
	}

	// Wait for the global flag to clear before posting results.
	while (*m_pDBMaintRunning)
	{
		if (WaitForSingleObject(m_hKillEvent, 100) == WAIT_OBJECT_0)
		{
			// If kill event is set, exit gracefully
			return 0;
		}
	}

	m_SessionMap.SetDBMaintResults(m_AgentID, m_nJob);

	return 0;
}
