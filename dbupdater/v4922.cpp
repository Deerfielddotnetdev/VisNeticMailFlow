#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V4922(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	// Alter RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to RoutingRules table") << endl;
	}

	g_logFile.Write(_T("Add column to RoutingRules table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD AutoReplyInTicket BIT DEFAULT 1 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	
	
	// Alter TicketBoxes
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to TicketBoxes table") << endl;
	}

	g_logFile.Write(_T("Add column to TicketBoxes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxes ADD AutoReplyInTicket BIT DEFAULT 1 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter AutoResponses
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to AutoResponses table") << endl;
	}

	g_logFile.Write(_T("Add column to AutoResponses table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AutoResponses ADD AutoReplyInTicket BIT DEFAULT 1 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;

}