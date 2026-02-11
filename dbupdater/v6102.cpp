#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6102(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
		
	//Add Column to RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add OfficeHours column to RoutingRules table") << endl;
	}

	g_logFile.Write(_T("Add OfficeHours column to RoutingRules table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD OfficeHours INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter OfficeHours
	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to OfficeHours table") << endl;
	}

	g_logFile.Write(_T("Add columns to OfficeHours table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE OfficeHours ADD Description VARCHAR(255) DEFAULT ('') NOT NULL ")
							   _T("ALTER TABLE OfficeHours ADD TimeStart DATETIME ")
							   _T("ALTER TABLE OfficeHours ADD TimeEnd DATETIME"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}