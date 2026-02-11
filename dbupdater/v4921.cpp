#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V4921(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	// Alter Agents
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to Agents table") << endl;
	}

	g_logFile.Write(_T("Add column to Agents table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD UseAutoFill BIT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	

	//Add AgentContacts
	if(g_showResults != 0)
	{
		_tcout << _T("Create AgentContacts table") << endl;
	}

	g_logFile.Write(_T("Create AgentContacts table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE AgentContacts ([AgentContactID] [int] IDENTITY (1, 1) NOT NULL ,[AgentID] [int] NOT NULL ,[ContactID] [int] NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AgentContacts ADD CONSTRAINT [PK_AgentContacts] PRIMARY KEY CLUSTERED ([AgentContactID]) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to RoutingRules table") << endl;
	}

	g_logFile.Write(_T("Add column to RoutingRules table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD ForwardInTicket BIT DEFAULT 1 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	
	
	return 0;

}