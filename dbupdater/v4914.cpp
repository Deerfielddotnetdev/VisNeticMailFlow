#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V4914(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	// Alter MessageSources
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to MessageSources table") << endl;
	}

	g_logFile.Write(_T("Add column to MessageSources table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageSources ADD IsSSL BIT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	

	// Alter MessageDestinations
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to MessageDestinations table") << endl;
	}

	g_logFile.Write(_T("Add column to MessageDestinations table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageDestinations ADD IsSSL BIT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	

	return 0;

}