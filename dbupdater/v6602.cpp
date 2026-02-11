#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6602(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	maxObjectID = 0;

	//Add Rows to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to ServerParameters table"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (148,'Time Zone ID','0') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add TimeZoneID column to Agents Table
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add TimeZoneID column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add TimeZoneID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD TimeZoneID INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add TimeZoneID column to TicketBoxes Table
	if(g_showResults != 0)
	{
		_tcout << _T("Alter TicketBoxes Table, add TimeZoneID column") << endl;
	}

	g_logFile.Write(_T("Alter TicketBoxes Table, add TimeZoneID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxes ADD TimeZoneID INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;
}