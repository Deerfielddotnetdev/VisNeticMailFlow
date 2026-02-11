#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V4810(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	// Alter Agents
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to Agents table") << endl;
	}

	g_logFile.Write(_T("Add column to Agents table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD NewMessageFormat BIT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 2705)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}	
	
	return 0;

}