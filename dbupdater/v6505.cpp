#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6505(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	maxObjectID = 0;

	//Create IX_IsDeleted Index on Tickets
	if(g_showResults != 0)
	{
		_tcout << _T("Create IX_IsDeleted Index on Tickets") << endl;
	}

	g_logFile.Write(_T("Create IX_IsDeleted Index on Tickets"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE INDEX IX_IsDeleted ON Tickets(IsDeleted,TicketStateID)"));
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	//Create IX_TicketBoxID2 Index on Tickets
	if(g_showResults != 0)
	{
		_tcout << _T("Create IX_TicketBoxID2 Index on Tickets") << endl;
	}

	g_logFile.Write(_T("Create IX_TicketBoxID2 Index on Tickets"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE INDEX IX_TicketBoxID2 ON Tickets(TicketBoxID,IsDeleted,TicketStateID,DateCreated)"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	//Create IX_TicketActionID Index on TicketHistory
	if(g_showResults != 0)
	{
		_tcout << _T("Create IX_TicketActionID Index on TicketHistory") << endl;
	}

	g_logFile.Write(_T("Create IX_TicketActionID Index on TicketHistory"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE INDEX IX_TicketActionID ON TicketHistory(TicketActionID,ID1,ID2)"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	return 0;
}