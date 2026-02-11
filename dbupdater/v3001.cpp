#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;

int V3001(CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	// Bug fix since we forgot to update version 3.0.0.1
	try
	{
		dca::DataConnector dc;
		
		{
			dca::SQLStatement st(dc);
			dca::ServerParametersTable spt(43);
			spt.Query(st);

			if(!lstrcmpi(spt.GetDescription(),_T("Enable MyTickets for Read Update")))
			{
				return 0;
			}
		}
	}
	catch(dca::Exception e)
	{
		_tcout << e.GetMessage() << endl;
	}
	catch(...)
	{
	}

	//Update ServerParameter table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding value to Server Parameter table") << endl;
	}

	g_logFile.Write(_T("Adding value to Server Parameter table"));

	// Adding value to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values(43,'Enable MyTickets for Read Update','1') SET IDENTITY_INSERT ServerParameters OFF SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values(44,'Enable Public Ticket Boxes for Read Update','1') SET IDENTITY_INSERT ServerParameters OFF SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values(45,'Enable AgentViews for Read Update','1') SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}