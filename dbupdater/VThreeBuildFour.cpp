#include "prehead.h"
#include "main.h"

int VThreeBuildFour(CDBConn& dbConn,reg::Key& rkMailFlow)
{
	SQLRETURN	retSQL = 0;
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter the Agents table add column OutboxHoldTime") << endl;
	}

	g_logFile.Write(_T("Alter the Agents table add column OutboxHoldTime"));

	// Alter the Agents table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD OutboxHoldTime INT DEFAULT 0 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}