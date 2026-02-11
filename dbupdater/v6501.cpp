#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6501(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
		
	//Add Column to MessageSources
	if(g_showResults != 0)
	{
		_tcout << _T("Add DateFilters column to MessageSources table") << endl;
	}

	g_logFile.Write(_T("Add DateFilters column to MessageSources table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageSources ADD DateFilters INT DEFAULT 0 NOT NULL") );
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Create Email Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create Email table") << endl;
	}

	g_logFile.Write(_T("Create Email table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[Email]( ")
	_T("[EmailID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[EmailTypeID] [int] NOT NULL CONSTRAINT [DF_Email_EmailTypeID] DEFAULT ((0)), ")
	_T("[EmailValue] [varchar](255) NOT NULL CONSTRAINT [DF_Email_EmailValue] DEFAULT (''), ")
	_T("[DateCreated] [datetime] NOT NULL CONSTRAINT [DF_Email_DateCreated]  DEFAULT (getdate()), ")
	_T("CONSTRAINT [PK_Email] PRIMARY KEY CLUSTERED ([EmailID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Rows to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (139,'Allow Access During Archive','0') ")							  
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (140,'Archive Max Tickets','0') ")							  
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (141,'Character Set','0') ")							  
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (142,'Message Date','0') ")							  
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}