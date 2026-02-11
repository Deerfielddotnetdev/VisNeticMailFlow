#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int VInstall(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

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

	//Set AgeAlerts Defaults
	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AgeAlerts DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for AgeAlerts DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AgeAlerts ADD CONSTRAINT [DF_AgeAlerts_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AgeAlerts DateEdited") << endl;
	}

	g_logFile.Write(_T("Modify default value for AgeAlerts DateEdited"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AgeAlerts ADD CONSTRAINT [DF_AgeAlerts_DateEdited] DEFAULT (getdate()) FOR [DateEdited]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Set AutoMessages Defaults
	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AutoMessages DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for AutoMessages DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AutoMessages ADD CONSTRAINT [DF_AutoMessages_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AutoMessages DateEdited") << endl;
	}

	g_logFile.Write(_T("Modify default value for AutoMessages DateEdited"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AutoMessages ADD CONSTRAINT [DF_AutoMessages_DateEdited] DEFAULT (getdate()) FOR [DateEdited]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Set AutoResponses Defaults
	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AutoResponses DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for AutoResponses DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AutoResponses ADD CONSTRAINT [DF_AutoResponses_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AutoResponses DateEdited") << endl;
	}

	g_logFile.Write(_T("Modify default value for AutoResponses DateEdited"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AutoResponses ADD CONSTRAINT [DF_AutoResponses_DateEdited] DEFAULT (getdate()) FOR [DateEdited]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Set ReportCustom Defaults
	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for ReportCustom DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for ReportCustom DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE ReportCustom ADD CONSTRAINT [DF_ReportCustom_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for ReportCustom DateEdited") << endl;
	}

	g_logFile.Write(_T("Modify default value for ReportCustom DateEdited"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE ReportCustom ADD CONSTRAINT [DF_ReportCustom_DateEdited] DEFAULT (getdate()) FOR [DateEdited]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Set ReportResults Defaults
	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for ReportResults DateRan") << endl;
	}

	g_logFile.Write(_T("Modify default value for ReportResults DateRan"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE ReportResults ADD CONSTRAINT [DF_ReportResults_DateRan] DEFAULT (getdate()) FOR [DateRan]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Set ReportScheduled Defaults
	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for ReportScheduled DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for ReportScheduled DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE ReportScheduled ADD CONSTRAINT [DF_ReportScheduled_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for ReportScheduled DateEdited") << endl;
	}

	g_logFile.Write(_T("Modify default value for ReportScheduled DateEdited"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE ReportScheduled ADD CONSTRAINT [DF_ReportScheduled_DateEdited] DEFAULT (getdate()) FOR [DateEdited]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Set WaterMarkAlerts Defaults
	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for WaterMarkAlerts DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for WaterMarkAlerts DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE WaterMarkAlerts ADD CONSTRAINT [DF_WaterMarkAlerts_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for WaterMarkAlerts DateEdited") << endl;
	}

	g_logFile.Write(_T("Modify default value for WaterMarkAlerts DateEdited"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE WaterMarkAlerts ADD CONSTRAINT [DF_WaterMarkAlerts_DateEdited] DEFAULT (getdate()) FOR [DateEdited]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AgentActivityLog ActivityDate") << endl;
	}

	g_logFile.Write(_T("Modify default value for AgentActivityLog ActivityDate"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AgentActivityLog ADD CONSTRAINT [DF_AgentActivityLog_ActivityDate] DEFAULT (getdate()) FOR [ActivityDate]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for Log LogTime") << endl;
	}

	g_logFile.Write(_T("Modify default value for Log LogTime"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE Log ADD CONSTRAINT [DF_Log_LogTime] DEFAULT (getdate()) FOR [LogTime]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AutoActions DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for AutoActions DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AutoActions ADD CONSTRAINT [DF_AutoActions_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for TicketLinks DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for TicketLinks DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE TicketLinks ADD CONSTRAINT [DF_TicketLinks_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for UIDLData UIDLDate") << endl;
	}

	g_logFile.Write(_T("Modify default value for UIDLData UIDLDate"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE UIDLData ADD CONSTRAINT [DF_UIDLData_UIDLDate] DEFAULT (getdate()) FOR [UIDLDate]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for Bypass DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for Bypass DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE Bypass ADD CONSTRAINT [DF_Bypass_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AgentSessions CreateTime") << endl;
	}

	g_logFile.Write(_T("Modify default value for AgentSessions CreateTime"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AgentSessions ADD CONSTRAINT [DF_AgentSessions_CreateTime] DEFAULT (getdate()) FOR [CreateTime]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for AgentSessions LastAuto") << endl;
	}

	g_logFile.Write(_T("Modify default value for AgentSessions LastAuto"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE AgentSessions ADD CONSTRAINT [DF_AgentSessions_LastAuto] DEFAULT (getdate()) FOR [LastAuto]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for BrowserSessions LastRequest") << endl;
	}

	g_logFile.Write(_T("Modify default value for BrowserSessions LastRequest"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE BrowserSessions ADD CONSTRAINT [DF_BrowserSessions_LastRequest] DEFAULT (getdate()) FOR [LastRequest]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for BrowserSessions CreateTime") << endl;
	}

	g_logFile.Write(_T("Modify default value for BrowserSessions CreateTime"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE BrowserSessions ADD CONSTRAINT [DF_BrowserSessions_CreateTime] DEFAULT (getdate()) FOR [CreateTime]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for SessionMonitors CheckIn") << endl;
	}

	g_logFile.Write(_T("Modify default value for SessionMonitors CheckIn"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE SessionMonitors ADD CONSTRAINT [DF_SessionMonitors_CheckIn] DEFAULT (getdate()) FOR [CheckIn]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Modify default value for Email DateCreated") << endl;
	}

	g_logFile.Write(_T("Modify default value for Email DateCreated"));
	
	retSQL = dbConn.ExecuteSQL( _T("ALTER TABLE Email ADD CONSTRAINT [DF_Email_DateCreated] DEFAULT (getdate()) FOR [DateCreated]") );
		
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Create IX_IsDeleted Index on InboundMessages
	if(g_showResults != 0)
	{
		_tcout << _T("Create IX_IsDeleted Index on InboundMessages") << endl;
	}

	g_logFile.Write(_T("Create IX_IsDeleted Index on InboundMessages"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE INDEX IX_IsDeleted ON InboundMessages(IsDeleted,DateReceived)"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	//Create IX_TicketStateID Index on Tickets
	if(g_showResults != 0)
	{
		_tcout << _T("Create IX_TicketStateID Index on Tickets") << endl;
	}

	g_logFile.Write(_T("Create IX_TicketStateID Index on Tickets"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE INDEX IX_TicketStateID ON Tickets(TicketStateID,IsDeleted,TicketCategoryID)"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

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

	retSQL = dbConn.ExecuteSQL(_T("CREATE INDEX IX_TicketBoxID2 ON TicketHistory(TicketActionID,ID1,ID2)"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	//Add Row to StdResponseCategories Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to StdResponseCategories Table") << endl;
	}

	g_logFile.Write(_T("Adding Row to StdResponseCategories Table"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT StdResponseCategories ON ")
							   _T("INSERT INTO StdResponseCategories (StdResponseCatID,CategoryName,ObjectID,BuiltIn) VALUES (-4,'Uncategorized',0,1) ")							  
							   _T("SET IDENTITY_INSERT StdResponseCategories OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	TCHAR buf[1024];
	int newObjectID = 0;
	if ( maxObjectID == 0 )
	{
		// Get the MAX ObjectID from the  Objects table
		if(g_showResults != 0)
		{
			_tcout << _T("Get the MAX ObjectID from the Objects table and add 1") << endl;
		}
		
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, maxObjectID );
		m_query.Execute( L"SELECT MAX(ObjectID) FROM Objects" );
		if( !m_query.Fetch() == S_OK )
		{
			_tcout << endl << _T("Failed to get maxObjectID from MailFlow DB") << endl;
			g_logFile.Write(_T("Failed to get maxObjectID from MailFlow DB"));
			g_logFile.LineBreak();
			g_logFile.Line();
			return 99;
		}

		if ( maxObjectID != 0 )
		{
			newObjectID = maxObjectID + 1;			
			if(g_showResults != 0)
			{
				wsprintf(buf,_T("Received MAX ObjectID: %d incrementing to %d"),maxObjectID,newObjectID);
				_tcout << buf << endl;
			}			
		}
		else
		{
			_tcout << endl << _T("Failed to get maxObjectID from MailFlow DB") << endl;
			g_logFile.Write(_T("Failed to get maxObjectID from MailFlow DB"));
			g_logFile.LineBreak();
			g_logFile.Line();
			return 2;
		}
	}
	else
	{
		newObjectID = maxObjectID + 1;		
		if(g_showResults != 0)
		{
			wsprintf(buf,_T("Received MAX ObjectID: %d incrementing to %d"),maxObjectID,newObjectID);
			_tcout << buf << endl;
		}		
	}
	maxObjectID++;

	//Add Row to Objects table	
	if(g_showResults != 0)
	{
		_tcout << _T("Add row to Objects table") << endl;
	}

	g_logFile.Write(_T("Add row to Objects table"));
	
	wsprintf(buf, _T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,-4,5,1,1,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Update StdResponseCategories table	
	if(g_showResults != 0)
	{
		_tcout << _T("Update StdResponseCategories table") << endl;
	}

	g_logFile.Write(_T("Update StdResponseCategories table"));
	
	wsprintf(buf, _T("UPDATE StdResponseCategories SET ObjectID=%d WHERE StdResponseCatID=-4"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;

}