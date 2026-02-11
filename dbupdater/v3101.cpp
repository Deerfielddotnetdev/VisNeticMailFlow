
#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;

int V3101(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDns)
{
	SQLRETURN	retSQL = 0;

	//ReportObjects
	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to ReportObjects") << endl;
	}

	g_logFile.Write(_T("Add rows to ReportObjects"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ReportObjects ON ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (2,'Agent',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (3,'Ticket Category',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (4,'Routing Rule',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (5,'Message Source',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (6,'Owner',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (7,'SR Usage',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (8,'SR Category',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (9,'SR Author',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (10,'By Hour',0) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (11,'By Day of Week',0) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (12,'Contact',1) ")
	_T("SET IDENTITY_INSERT ReportObjects OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//ReportTypes
	if(g_showResults != 0)
	{
		_tcout << _T("Add ReportTypes") << endl;
	}

	g_logFile.Write(_T("Add rows to ReportTypes"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ReportTypes ON ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (2,'Outbound Messages',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (3,'Open Ticket Age',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (4,'Ticket Distribution',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (5,'Standard Responses',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (6,'Avg Response Time',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (7,'Avg Time to Resolve',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (8,'Busiest Time',0) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (9,'History',1) ")
	_T("SET IDENTITY_INSERT ReportTypes OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;

}