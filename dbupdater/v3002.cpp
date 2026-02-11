#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;

int V3002(CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Update MessageSources table
	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to MessageSources") << endl;
	}

	g_logFile.Write(_T("Add columns to MessageSources"));

	// Add columns to MessageSources
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageSources ADD UseReplyTo BIT DEFAULT 0"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update LogEntryTypes table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding value to LogEntryTypes table") << endl;
	}

	g_logFile.Write(_T("Adding value to LogEntryTypes table"));

	// Adding value to LogEntryTypes table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT LogEntryTypes ON insert into LogEntryTypes (LogEntryTypeID,TypeDescrip,SeverityLevels) values(11,'Tickler processing','14') SET IDENTITY_INSERT LogEntryTypes OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}