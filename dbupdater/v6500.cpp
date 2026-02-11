#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6500(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
		
	//Add Rows to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (132,'Enable Shared Agent Sessions','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (133,'Session Sync Frequency','30') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (134,'Agent Status Sync Frequency','30') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (135,'Save Sessions To DB Frequency','60') ")							  
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (136,'Enable Database Agent Sessions','0') ")							  
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (137,'Queue Full','0') ")							  
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (138,'Queue Size','32') ")							  
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Create AgentSessions Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create AgentSessions table") << endl;
	}

	g_logFile.Write(_T("Create AgentSessions table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[AgentSessions]( ")
	_T("[AgentSessionID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[AgentID] [int] NOT NULL CONSTRAINT [DF_AgentSessions_AgentID] DEFAULT ((0)), ")
	_T("[CreateTime] [datetime] NOT NULL CONSTRAINT [DF_AgentSessions_CreateTime]  DEFAULT (getdate()), ")
	_T("[LastAuto] [datetime] NOT NULL CONSTRAINT [DF_AgentSessions_LastAuto]  DEFAULT (getdate()), ")
	_T("[PreAutoID] [int] NOT NULL CONSTRAINT [DF_AgentSessions_PreAutoID] DEFAULT ((0)), ")
	_T("[PreAutoText] [varchar](125) NOT NULL CONSTRAINT [DF_AgentSessions_PreAutoText] DEFAULT (''), ")
	_T("CONSTRAINT [PK_AgentSessions] PRIMARY KEY CLUSTERED ([AgentSessionID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Create BrowserSessions Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create BrowserSessions table") << endl;
	}

	g_logFile.Write(_T("Create BrowserSessions table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[BrowserSessions]( ")
	_T("[BrowserSessionID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[SessionID] [varchar](50) NOT NULL CONSTRAINT [DF_BrowserSessions_SessionID] DEFAULT (''), ")
	_T("[BrowserIP] [varchar](50) NOT NULL CONSTRAINT [DF_BrowserSessions_BrowserIP] DEFAULT (''), ")
	_T("[AgentID] [int] NOT NULL CONSTRAINT [DF_BrowserSessions_AgentID] DEFAULT ((0)), ")
	_T("[LastRequest] [datetime] NOT NULL CONSTRAINT [DF_BrowserSessions_LastRequest]  DEFAULT (getdate()), ")
	_T("[CreateTime] [datetime] NOT NULL CONSTRAINT [DF_BrowserSessions_CreateTime]  DEFAULT (getdate()), ")
	_T("[UserAgent] [varchar](50) NOT NULL CONSTRAINT [DF_BrowserSessions_UserAgent] DEFAULT (''), ")
	_T("CONSTRAINT [PK_BrowserSessions] PRIMARY KEY CLUSTERED ([BrowserSessionID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Create SessionMonitors Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create SessionMonitors table") << endl;
	}

	g_logFile.Write(_T("Create SessionMonitors table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[SessionMonitors]( ")
	_T("[SessionMonitorID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[ServerID] [int] NOT NULL CONSTRAINT [DF_SessionMonitors_ServerID] DEFAULT ((0)), ")
	_T("[ProcessID] [int] NOT NULL CONSTRAINT [DF_SessionMonitors_ProcessID] DEFAULT ((0)), ")
	_T("[CheckIn] [datetime] NOT NULL CONSTRAINT [DF_SessionMonitors_CheckIn]  DEFAULT (getdate()), ")
	_T("CONSTRAINT [PK_SessionMonitors] PRIMARY KEY CLUSTERED ([SessionMonitorID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Create SessionMonitorQueue Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create SessionMonitorQueue table") << endl;
	}

	g_logFile.Write(_T("Create SessionMonitorQueue table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[SessionMonitorQueue]( ")
	_T("[QueueID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[ServerID] [int] NOT NULL CONSTRAINT [DF_SessionMonitorQueue_ServerID] DEFAULT ((0)), ")
	_T("[ProcessID] [int] NOT NULL CONSTRAINT [DF_SessionMonitorQueue_ProcessID] DEFAULT ((0)), ")
	_T("[ActionID] [int] NOT NULL CONSTRAINT [DF_SessionMonitorQueue_ActionID] DEFAULT ((0)), ")
	_T("[TargetID] [int] NOT NULL CONSTRAINT [DF_SessionMonitorQueue_TargetID] DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_SessionMonitorQueue] PRIMARY KEY CLUSTERED ([QueueID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update ObjectTypes Table
	if(g_showResults != 0)
	{
		_tcout << _T("Updating ObjectTypes Table") << endl;
	}

	g_logFile.Write(_T("Updating ObjectTypes Table"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE ObjectTypes SET Description='Draft Message' WHERE ObjectTypeID=11"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Allow Delete Drafts for Group Everyone
	if(g_showResults != 0)
	{
		_tcout << _T("Updating Object Security") << endl;
	}

	g_logFile.Write(_T("Updating Object Security"));

	dbConn.ExecuteSQL(_T("UPDATE AccessControl SET AccessLevel=4 WHERE ObjectID=(SELECT TOP 1 ObjectID FROM Objects WHERE ObjectTypeID=11 AND ActualID=0) AND GroupID=1"));
	
	//Convert DB Maint Setting
	if(g_showResults != 0)
	{
		_tcout << _T("Convert DB Maintenance Setting") << endl;
	}

	g_logFile.Write(_T("Convert DB Maintenance Setting"));

	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, maxObjectID );
	m_query.Execute( L"SELECT DataValue FROM ServerParameters WHERE ServerParameterID=1" );
	if( m_query.Fetch() == S_OK )
	{
		if(maxObjectID==1)
		{
			retSQL = dbConn.ExecuteSQL(_T("UPDATE ServerParameters SET DataValue='SU:MO:TU:WE:TH:FR:SA' WHERE ServerParameterID=1"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
		}
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

	//Crete IX_TicketStateID Index on Tickets
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
	
	return 0;
}