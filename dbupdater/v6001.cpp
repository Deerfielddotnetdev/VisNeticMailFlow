#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6001(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
		
	//Add Row to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Row to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (117,'Date Entry Format','0') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Column to TicketBoxes
	if(g_showResults != 0)
	{
		_tcout << _T("Add MultiMail column to TicketBoxes table") << endl;
	}

	g_logFile.Write(_T("Add MultiMail column to TicketBoxes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxes ADD MultiMail INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Column to RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add MultiMail column to RoutingRules table") << endl;
	}

	g_logFile.Write(_T("Add MultiMail column to RoutingRules table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD MultiMail INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Column to OutboundMessages
	if(g_showResults != 0)
	{
		_tcout << _T("Add MultiMail column to OutboundMessages table") << endl;
	}

	g_logFile.Write(_T("Add MultiMail column to OutboundMessages table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE OutboundMessages ADD MultiMail INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;
}