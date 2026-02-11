#include "prehead.h"
#include "main.h"

int TicketBoxAlg(CDBConn& dbConn,reg::Key& rkMailFlow)
{
	SQLRETURN	retSQL;

	if(g_showResults != 0)
	{
		_tcout << _T("Implementing TicketBox algorithm changes") << endl;
	}

	g_logFile.Write(_T("Implementing TicketBox algorithm changes"));

	// Update ServerParameters
	retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set Description = 'Default routing rule ticket owner algorithm' where ServerParameterID = 32 update ServerParameters set Description = 'Default routing rule ticketbox algorithm' where ServerParameterID = 33"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}


	// Alter routing rule table add columns
	retSQL = dbConn.ExecuteSQL(_T("alter table RoutingRules add AssignToAgentAlg int alter table RoutingRules add AssignToTicketBoxAlg int"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add constraints to routing rule table
	retSQL = dbConn.ExecuteSQL(_T("alter table RoutingRules add constraint DF_RoutingRulesAssignToAgentAlg default ('1') for AssignToAgentAlg alter table RoutingRules add constraint DF_RoutingRulesAssignToTicketBoxAlg default ('1') for AssignToTicketBoxAlg"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Configure routing rule data
	retSQL = dbConn.ExecuteSQL(_T("update RoutingRules set AssignToTicketBoxAlg = 0 update RoutingRules set AssignToAgentAlg = 2 where AssignToAgent < 0 and AssignToAgentEnable != 0 update RoutingRules set AssignToAgentAlg = 1 where AssignToAgent = 0 and AssignToAgentEnable != 0 update RoutingRules set AssignToAgentAlg = 0 where AssignToAgent > 0 and AssignToAgentEnable != 0 update RoutingRules set  AssignToAgent = 0 where AssignToAgent < 0 and AssignToAgentEnable != 0 update RoutingRules set AssignToAgentAlg = 0 where AssignToAgentEnable = 0 update RoutingRules set AssignToAgent = 0 where AssignToAgentEnable = 0"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("alter table RoutingRules ALTER COLUMN AssignToAgentAlg int NOT NULL alter table RoutingRules ALTER COLUMN AssignToTicketBoxAlg int NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}