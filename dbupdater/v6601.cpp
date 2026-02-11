#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6601(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	maxObjectID = 0;

	//Add Rows to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (144,'Search Require Date','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (145,'Sessions Logoff All','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (146,'Sessions Expire All','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (147,'Allow Agent LogOff','0') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;
}