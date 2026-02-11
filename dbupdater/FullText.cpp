#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;

int VFullText(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Add Tickets_CAT
	if(g_showResults != 0)
	{
		_tcout << _T("Add Tickets_CAT FullText Catalog") << endl;
	}

	g_logFile.Write(_T("Add Tickets_CAT FullText Catalog"));
	
	retSQL = dbConn.ExecuteSQL( _T("CREATE FULLTEXT CATALOG [Tickets_CAT] WITH ACCENT_SENSITIVITY = ON") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Tickets Full-Text Index
	if(g_showResults != 0)
	{
		_tcout << _T("Add Tickets FullText Index") << endl;
	}

	g_logFile.Write(_T("Add Tickets FullText Index"));
	
	retSQL = dbConn.ExecuteSQL( _T("DECLARE @keyName varchar(max) ")
								_T("set @keyName = (SELECT CONSTRAINT_NAME FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS WHERE TABLE_NAME = 'Tickets' AND CONSTRAINT_TYPE = 'PRIMARY KEY') ")
								_T("DECLARE @query nvarchar(max) ")
								_T("set @query = N'CREATE FULLTEXT INDEX on Tickets (Contacts, Subject) KEY index ' + @keyName + ' ON Tickets_CAT' ")
								_T("exec sp_executesql @query") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add InboundMessages_CAT
	if(g_showResults != 0)
	{
		_tcout << _T("Add InboundMessages_CAT FullText Catalog") << endl;
	}

	g_logFile.Write(_T("Add InboundMessages_CAT FullText Catalog"));
	
	retSQL = dbConn.ExecuteSQL( _T("CREATE FULLTEXT CATALOG [InboundMessages_CAT] WITH ACCENT_SENSITIVITY = ON") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add InboundMessages Full-Text Index
	if(g_showResults != 0)
	{
		_tcout << _T("Add InboundMessages FullText Index") << endl;
	}

	g_logFile.Write(_T("Add InboundMessages FullText Index"));
	
	retSQL = dbConn.ExecuteSQL( _T("DECLARE @keyName varchar(max) ")
								_T("set @keyName = (SELECT CONSTRAINT_NAME FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS WHERE TABLE_NAME = 'InboundMessages' AND CONSTRAINT_TYPE = 'PRIMARY KEY') ")
								_T("DECLARE @query nvarchar(max) ")
								_T("set @query = N'CREATE FULLTEXT INDEX on InboundMessages (Body, Subject) KEY index ' + @keyName + ' ON InboundMessages_CAT' ")
								_T("exec sp_executesql @query") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add OutboundMessages_CAT
	if(g_showResults != 0)
	{
		_tcout << _T("Add OutboundMessages_CAT FullText Catalog") << endl;
	}

	g_logFile.Write(_T("Add OutboundMessages_CAT FullText Catalog"));
	
	retSQL = dbConn.ExecuteSQL( _T("CREATE FULLTEXT CATALOG [OutboundMessages_CAT] WITH ACCENT_SENSITIVITY = ON") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add OutboundMessages Full-Text Index
	if(g_showResults != 0)
	{
		_tcout << _T("Add OutboundMessages FullText Index") << endl;
	}

	g_logFile.Write(_T("Add OutboundMessages FullText Index"));
	
	retSQL = dbConn.ExecuteSQL( _T("DECLARE @keyName varchar(max) ")
								_T("set @keyName = (SELECT CONSTRAINT_NAME FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS WHERE TABLE_NAME = 'OutboundMessages' AND CONSTRAINT_TYPE = 'PRIMARY KEY') ")
								_T("DECLARE @query nvarchar(max) ")
								_T("set @query = N'CREATE FULLTEXT INDEX on OutboundMessages (Body, Subject) KEY index ' + @keyName + ' ON OutboundMessages_CAT' ")
								_T("exec sp_executesql @query") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;
}