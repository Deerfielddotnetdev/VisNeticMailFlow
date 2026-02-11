#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V3611(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Add Row to Server Parameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to Server Parameter table") << endl;
	}

	g_logFile.Write(_T("Adding Row to Server Parameter table"));

	// Adding value to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (67,'Ticket List Date','0') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;

}