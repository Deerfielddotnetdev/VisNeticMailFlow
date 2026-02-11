#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V4900(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	// Alter MatchText
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to MatchText table") << endl;
	}

	g_logFile.Write(_T("Add column to MatchText table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MatchText ADD IsRegEx BIT DEFAULT 0 NOT NULL"));

	// Alter MatchTextP
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to MatchTextP table") << endl;
	}

	g_logFile.Write(_T("Add column to MatchTextP table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MatchTextP ADD IsRegEx BIT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add AgentRouting
	if(g_showResults != 0)
	{
		_tcout << _T("Create AgentRouting table") << endl;
	}

	g_logFile.Write(_T("Create AgentRouting table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE AgentRouting ([AgentRoutingID] [int] IDENTITY (1, 1) NOT NULL ,[RoutingRuleID] [int] NOT NULL ,[AgentID] [int] NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AgentRouting ADD CONSTRAINT [PK_AgentRouting] PRIMARY KEY CLUSTERED ([AgentRoutingID]) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	
	
	//Add column to RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add column LastOwnerID to RoutingRules") << endl;
	}

	g_logFile.Write(_T("Add column LastOwnerID to RoutingRules"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD LastOwnerID INT DEFAULT 0 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add column to RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add column MatchMethod to RoutingRules") << endl;
	}

	g_logFile.Write(_T("Add column MatchMethod to RoutingRules"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD MatchMethod INT DEFAULT 0 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Row to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Row to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (90,'Ticket Tracking','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (91,'Default Routing Rule Create Ticket','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (92,'Default Routing Rule Match Method','0') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;

}