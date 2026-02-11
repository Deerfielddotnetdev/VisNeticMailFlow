#include "prehead.h"
#include "main.h"

int CustomDictionaryFix(CDBConn& dbConn,reg::Key& rkMailFlow)
{
	TCHAR sData[256];

	dbConn.GetData(_T("select @@TRANCOUNT"),sData);

	SQLRETURN	retSQL;

	if(g_showResults != 0)
	{
		_tcout << _T("Recreate table CustomDictionary") << endl;
	}

	g_logFile.Write(_T("Recreate table CustomDictionary"));

	retSQL = dbConn.ExecuteSQL(_T("sp_help CustomDictionary"));

	if(SQL_SUCCEEDED(retSQL))
	{
		retSQL = dbConn.ExecuteSQL(_T("drop table CustomDictionary"));
	}

	retSQL = dbConn.ExecuteSQL(_T("create table CustomDictionary ( CustomDictionaryID int IDENTITY(1,1) PRIMARY KEY not null, AgentID int not null, Word varchar(255) not null)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// create index for custom dictionary
	retSQL = dbConn.ExecuteSQL(_T("create index IX_AgentID on CustomDictionary(AgentID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
				return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	if(!IsTransUp(dbConn,rkMailFlow))
	{
		// Begin transaction
			retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				OutputDebugString(_T("Failed to begin transaction \n"));
				if(g_showResults != 0)
				{
					_tcout << endl << dbConn.GetErrorString() << endl;
				}

				g_logFile.Write(dbConn.GetErrorString());

				dbConn.Disconnect();
				rkMailFlow.Close();
				g_logFile.LineBreak();
				g_logFile.Line();
				return 4;
			}
	}

	// alter table and add constraint
	retSQL = dbConn.ExecuteSQL(_T("alter table CustomDictionary add constraint DF_CustomDictionaryAgentID Default ('0') for AgentID"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}
