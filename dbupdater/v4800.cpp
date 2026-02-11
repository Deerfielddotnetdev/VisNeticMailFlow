#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V4800(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Add Row to AgentActivities Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to AgentActivities table") << endl;
	}

	g_logFile.Write(_T("Adding Row to AgentActivities table"));

	// Adding row to AgentActivities table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT AgentActivities ON ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (24,'Merged Ticket') ")							   
							   _T("SET IDENTITY_INSERT AgentActivities OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter Signatures
	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to Signatures table") << endl;
	}

	g_logFile.Write(_T("Add columns to Signatures table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Signatures ADD ObjectTypeID INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Signatures ADD ActualID INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Signatures ADD MediaTypeID INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;

}