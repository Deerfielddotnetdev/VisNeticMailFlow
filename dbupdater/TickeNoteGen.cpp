#include "prehead.h"
#include "main.h"

int TicketNotes(CDBConn& dbConn,reg::Key& rkMailFlow)
{
	SQLRETURN	retSQL;

	//*************************************************************************
	
	if(g_showResults != 0)
	{
		_tcout << _T("Adding table for TicketNotes support") << endl;
	}

	g_logFile.Write(_T("Adding table for TicketNotes support"));

	// Create table for TicketHistory
	retSQL = dbConn.ExecuteSQL(_T("create table TicketHistory (TicketHistoryID int IDENTITY(1,1) PRIMARY KEY not null, TicketID int not null, DateTime datetime, AgentID int not null, TicketActionID int not null, ID1 int not null, ID2 int not null)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Create table for TicketHistory
	retSQL = dbConn.ExecuteSQL(_T("create table TicketActions ( TicketActionID int IDENTITY(1,1) PRIMARY KEY not null, Description VARCHAR(255) not null)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("insert into TicketActions(Description) values('Ticket created') insert into TicketActions(Description) values('Ticket modified') insert into TicketActions(Description) values('Ticket age alerted') insert into TicketActions(Description) values('Ticket escalated') insert into TicketActions(Description) values('Added message to ticket') insert into TicketActions(Description) values('Removed messages from ticket') insert into TicketActions(Description) values('Ticket deleted')"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}