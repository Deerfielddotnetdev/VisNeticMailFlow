#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;

int V3500(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Add New Tables
	/*
	AgentTicketCategories
	*/

	//Add AgentTicketCategories
	if(g_showResults != 0)
	{
		_tcout << _T("Create AgentTicketCategories table") << endl;
	}

	g_logFile.Write(_T("Create AgentTicketCategories table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[AgentTicketCategories]( ")
	_T("[AgentTicketCategoryID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[AgentID] [int] NOT NULL CONSTRAINT [DF_AgentTicketCategories_AgentID]  DEFAULT ((0)), ")
	_T("[TicketCategoryID] [int] NOT NULL CONSTRAINT [DF_AgentTicketCategories_TicketCategoryID]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_AgentTicketCategories] PRIMARY KEY CLUSTERED ([AgentTicketCategoryID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Set AlertMsgs Defaults
	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AlertMsgs DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for AlertMsgs DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AlertMsgs ADD CONSTRAINT [DF_AlertMsgs_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	

	return 0;

}