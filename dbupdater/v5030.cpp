#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;

int V5030(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	
	// Update ReportStandard
	if(g_showResults != 0)
	{
		_tcout << _T("Update ReportStandard table") << endl;
	}

	g_logFile.Write(_T("Update ReportStandard table"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE ReportStandard SET Description='Outbound Messages By Agent' WHERE StandardReportID=7"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	
	
	retSQL = dbConn.ExecuteSQL(_T("UPDATE ReportStandard SET Description='Outbound Messages By Ticket Category' WHERE StandardReportID=8"));
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
		
	return 0;
}