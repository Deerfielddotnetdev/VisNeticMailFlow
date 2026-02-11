#include "prehead.h"
#include "main.h"

int V2007(CDBConn& dbConn,reg::Key& rkMailFlow)
{
	SQLRETURN	retSQL = 0;
	
	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to OutboundMessages [2.0.0.7]") << endl;
	}

	g_logFile.Write(_T("Add columns to OutboundMessages [2.0.0.7]"));

	// Alter the Agents table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE OutboundMessages ADD DraftCloseTicket TINYINT NOT NULL CONSTRAINT DF_OutboundMessages_DraftCloseTicket DEFAULT 0 ALTER TABLE OutboundMessages ADD DraftRouteToMe TINYINT NOT NULL CONSTRAINT DF_OutboundMessages_DraftRouteToMe DEFAULT 0"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Change in RoutingRules [2.0.0.7]") << endl;
	}

	g_logFile.Write(_T("Change in RoutingRules [2.0.0.7]"));

	// Alter the Agents table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules DROP CONSTRAINT DF_RoutingRulesAssignToAgentAlg ALTER TABLE RoutingRules ADD CONSTRAINT DF_RoutingRulesAssignToAgentAlg DEFAULT ('4') FOR AssignToAgentAlg"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Drop Constraint in StandardResponseUsage [2.0.0.7]") << endl;
	}

	g_logFile.Write(_T("Drop Constraint in StandardResponseUsage [2.0.0.7]"));

	// Alter the Agents table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE StandardResponseUsage DROP CONSTRAINT FK_StandardResponseUsageAgents"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Resize column in PersonalDataTypes [2.0.0.7]") << endl;
	}

	g_logFile.Write(_T("Resize column in PersonalDataTypes [2.0.0.7]"));

	// Alter the Agents table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE PersonalDataTypes ALTER COLUMN TypeName VARCHAR(64) NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}
