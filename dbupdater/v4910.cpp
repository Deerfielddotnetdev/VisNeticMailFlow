#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V4910(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Add column to RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add column ForwardFromEmail to RoutingRules") << endl;
	}

	g_logFile.Write(_T("Add column ForwardFromEmail to RoutingRules"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD ForwardFromEmail VARCHAR(125) DEFAULT '' NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add column to RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add column ForwardFromName to RoutingRules") << endl;
	}

	g_logFile.Write(_T("Add column ForwardFromName to RoutingRules"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD ForwardFromName VARCHAR(125) DEFAULT '' NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;

}