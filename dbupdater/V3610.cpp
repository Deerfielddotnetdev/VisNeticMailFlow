#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;

int V3610(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Add FooterLocation Column to TicketBoxes, AutoResponses and OutboundMessages

	if(g_showResults != 0)
	{
		_tcout << _T("Add FooterLocation column to TicketBoxes table") << endl;
	}

	g_logFile.Write(_T("Add FooterLocation column to TicketBoxes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxes ADD FooterLocation tinyint default '0' not null") );
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Add FooterLocation column to AutoResponses table") << endl;
	}

	g_logFile.Write(_T("Add FooterLocation column to AutoResponses table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AutoResponses ADD FooterLocation tinyint default '0' not null") );
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Add FooterLocation column to OutboundMessages table") << endl;
	}

	g_logFile.Write(_T("Add FooterLocation column to OutboundMessages table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE OutboundMessages ADD FooterLocation tinyint default '0' not null") );
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	return 0;

}