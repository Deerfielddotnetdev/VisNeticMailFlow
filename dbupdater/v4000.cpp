#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V4000(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
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
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (69,'Database Logging','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (70,'Delete Logs','7') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (71,'Delete Backups','3') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (72,'Master Server','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (73,'Archive Path','C:\\Program Files\\Deerfield.com\\VisNetic MailFlow\\Archive') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (74,'Backup Path','C:\\Program Files\\Deerfield.com\\VisNetic MailFlow\\Backup') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (75,'Log Path','C:\\Program Files\\Deerfield.com\\VisNetic MailFlow\\Logs') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (76,'Next Maintenance High','') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (77,'Next Maintenance Low','') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (78,'Report Path','C:\\Program Files\\Deerfield.com\\VisNetic MailFlow\\Reports') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (79,'Inbound Attachment Folder','C:\\Program Files\\Deerfield.com\\VisNetic MailFlow\\InboundAttach') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (80,'Outbound Attachment Folder','C:\\Program Files\\Deerfield.com\\VisNetic MailFlow\\OutboundAttach') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (81,'Spool Folder','C:\\Program Files\\Deerfield.com\\VisNetic MailFlow\\Spool') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (82,'Temp Folder','C:\\Program Files\\Deerfield.com\\VisNetic MailFlow\\Temp') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (83,'Quarantine Folder','C:\\Program Files\\Deerfield.com\\VisNetic MailFlow\\Quarantine') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (84,'Agent Activity Ticket Message Revoke','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (85,'Agent Activity Ticket Message Release','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (86,'Agent Activity Ticket Message Return','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (87,'3CX PlugIn Version','8.1.9532') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (88,'MailFlow Client Version','0.0.0') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Row to TicketboxViewTypes Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to TicketBoxViewTypes table") << endl;
	}

	g_logFile.Write(_T("Adding Row to TicketBoxViewTypes table"));

	// Adding row to TicketBoxViewTypes table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT TicketBoxViewTypes ON ")
							   _T("INSERT INTO TicketBoxViewTypes (TicketBoxViewTypeID,Name) VALUES (10,'Approval Queue') ")							   
							   _T("SET IDENTITY_INSERT TicketBoxViewTypes OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Row to OutboundMessageStates Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to OutboundMessageStates table") << endl;
	}

	g_logFile.Write(_T("Adding Row to OutboundMessageStates table"));

	// Adding row to OutboundMessageStates table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT OutboundMessageStates ON ")
							   _T("INSERT INTO OutboundMessageStates (OutboundMsgStateID,MessageStateName) VALUES (7,'Waiting Approval') ")							   
							   _T("SET IDENTITY_INSERT OutboundMessageStates OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Rows to ApprovalObjectTypes Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to ApprovalObjectTypes table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to ApprovalObjectTypes table"));

	// Adding rows to ApprovalObjectTypes table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ApprovalObjectTypes ON ")
							   _T("INSERT INTO ApprovalObjectTypes (ApprovalObjectTypeID,Name) VALUES (1,'Outbound Message') ")
							   _T("INSERT INTO ApprovalObjectTypes (ApprovalObjectTypeID,Name) VALUES (2,'Standard Response') ")
							   _T("SET IDENTITY_INSERT ApprovalObjectTypes OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Rows to AlertEvents Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to AlertEvents table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to AlertEvents table"));

	// Adding rows to AlertEvents table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT AlertEvents ON ")
							   _T("INSERT INTO AlertEvents (AlertEventID,Description) VALUES (12,'Inbound Message to Owned Ticket') ")
							   _T("INSERT INTO AlertEvents (AlertEventID,Description) VALUES (13,'Inbound Message from Owned Contact') ")
							   _T("INSERT INTO AlertEvents (AlertEventID,Description) VALUES (14,'Ticket Assigned to You') ")
							   _T("INSERT INTO AlertEvents (AlertEventID,Description) VALUES (15,'Outbound Message Requires Approval') ")
							   _T("INSERT INTO AlertEvents (AlertEventID,Description) VALUES (16,'Outbound Message Approved') ")
							   _T("INSERT INTO AlertEvents (AlertEventID,Description) VALUES (17,'Outbound Message Returned') ")
							   _T("SET IDENTITY_INSERT AlertEvents OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Rows to AgentActivities Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to AgentActivities table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to AgentActivities table"));

	// Adding rows to AgentActivities table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT AgentActivities ON ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (19,'Placed Ticket On-hold') ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (20,'Revoked Message') ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (21,'Released Message') ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (22,'Returned Message') ")
							   _T("SET IDENTITY_INSERT AgentActivities OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Alter Agents Table
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add OutboundApprovalFromID column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add OutboundApprovalFromID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD OutboundApprovalFromID INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter Approvals Table
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Approvals Table, add ActualID column") << endl;
	}

	g_logFile.Write(_T("Alter Approvals Table, add ActualID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Approvals ADD ActualID INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter MessageDestinations Table
	if(g_showResults != 0)
	{
		_tcout << _T("Alter MessageDestinations Table, add ServerID column") << endl;
	}

	g_logFile.Write(_T("Alter MessageDestinations Table, add ServerID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageDestinations ADD ServerID INT DEFAULT 1 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter MessageSources Table
	if(g_showResults != 0)
	{
		_tcout << _T("Alter MessageSources Table, add ServerID column") << endl;
	}

	g_logFile.Write(_T("Alter MessageSources Table, add ServerID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageSources ADD ServerID INT DEFAULT 1 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add Log Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create Log Table") << endl;
	}

	g_logFile.Write(_T("Create Log Table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE Log ( ")
	_T("[LogID] [int] IDENTITY (1, 1) NOT NULL , ")
	_T("[LogTime] [datetime] NOT NULL , ")
	_T("[ServerID] [int] NOT NULL , ")
	_T("[ErrorCode] [int] NOT NULL , ")
	_T("[LogSeverityID] [int] NOT NULL , ")
	_T("[LogEntryTypeID] [int] NOT NULL , ")
	_T("[LogText] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL)"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Log ADD ") 
	_T("CONSTRAINT [DF_Log_LogTime] DEFAULT (getdate()) FOR [LogTime], ")
	_T("CONSTRAINT [DF_Log_ServerID] DEFAULT (0) FOR [ServerID], ")
	_T("CONSTRAINT [DF_Log_ErrorCode] DEFAULT (0) FOR [ErrorCode], ")
	_T("CONSTRAINT [DF_Log_LogSeverityID] DEFAULT (0) FOR [LogSeverityID], ")
	_T("CONSTRAINT [DF_Log_LogEntryTypeID] DEFAULT (0) FOR [LogEntryTypeID], ")
	_T("CONSTRAINT [DF_Log_LogText] DEFAULT ('') FOR [LogText], ")
	_T("CONSTRAINT [PK_Log] PRIMARY KEY  CLUSTERED ([LogID])"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Add Indexes for Log Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create Index IX_LogTime in Log Table") << endl;
	}

	g_logFile.Write(_T("Create Index IX_LogTime in Log Table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE INDEX IX_LogTime on Log(LogTime,ServerID,LogEntryTypeID,LogSeverityID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Create Index IX_ServerID in Log Table") << endl;
	}

	g_logFile.Write(_T("Create Index IX_ServerID in Log Table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE INDEX IX_ServerID on Log(ServerID,LogTime,LogEntryTypeID,LogSeverityID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	// Add Servers Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create Servers Table") << endl;
	}

	g_logFile.Write(_T("Create Servers Table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE Servers ( ")
	_T("[ServerID] [int] IDENTITY (1, 1) NOT NULL , ")
	_T("[Description] [varchar] (50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[RegistrationKey] [varchar] (50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[Status] [int] NOT NULL , ")
	_T("[ReloadConfig] [int] NOT NULL , ")
	_T("[CheckIn] [datetime] NULL , ")
	_T("[SpoolFolder] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL , ")
	_T("[TempFolder] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL , ")
	_T("[WebStatus] [int] NOT NULL , ")
	_T("[WebCheckIn] [datetime] NULL)"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Servers ADD ") 
	_T("CONSTRAINT [DF_Servers_Description] DEFAULT ('') FOR [Description], ")
	_T("CONSTRAINT [DF_Servers_RegistrationKey] DEFAULT ('') FOR [RegistrationKey], ")
	_T("CONSTRAINT [DF_Servers_Status] DEFAULT (0) FOR [Status], ")
	_T("CONSTRAINT [DF_Servers_ReloadConfig] DEFAULT (0) FOR [ReloadConfig], ")
	_T("CONSTRAINT [DF_Servers_WebStatus] DEFAULT (0) FOR [WebStatus], ")	
	_T("CONSTRAINT [PK_Servers] PRIMARY KEY  CLUSTERED ([ServerID])"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	
	
	// Add ServerTaskTypes Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create ServerTaskTypes Table") << endl;
	}

	g_logFile.Write(_T("Create ServerTaskTypes Table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE ServerTaskTypes ( ")
	_T("[Description] [varchar] (50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[ServerTaskTypeID] [int] IDENTITY (1, 1) NOT NULL)"));		
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE ServerTaskTypes ADD ") 
	_T("CONSTRAINT [DF_ServerTaskTypes_Description] DEFAULT ('') FOR [Description], ")
	_T("CONSTRAINT [PK_ServerTaskTypes] PRIMARY KEY  CLUSTERED ([ServerTaskTypeID])"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Rows to ServerTaskTypes Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to ServerTaskTypes table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to ServerTaskTypes table"));

	// Adding rows to ServerTaskTypes table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerTaskTypes ON ")
							   _T("INSERT INTO ServerTaskTypes (ServerTaskTypeID,Description) VALUES (1,'Alert Sender') ")
							   _T("INSERT INTO ServerTaskTypes (ServerTaskTypeID,Description) VALUES (2,'Database Maintenance') ")
							   _T("INSERT INTO ServerTaskTypes (ServerTaskTypeID,Description) VALUES (3,'Database Monitor') ")
							   _T("INSERT INTO ServerTaskTypes (ServerTaskTypeID,Description) VALUES (4,'Message Collection') ")
							   _T("INSERT INTO ServerTaskTypes (ServerTaskTypeID,Description) VALUES (5,'Message Router') ")
							   _T("INSERT INTO ServerTaskTypes (ServerTaskTypeID,Description) VALUES (6,'Message Sending') ")
							   _T("INSERT INTO ServerTaskTypes (ServerTaskTypeID,Description) VALUES (7,'Reporter') ")
							   _T("INSERT INTO ServerTaskTypes (ServerTaskTypeID,Description) VALUES (8,'Ticket Monitor') ")
							   _T("SET IDENTITY_INSERT ServerTaskTypes OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add ServerTasks Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create ServerTasks Table") << endl;
	}

	g_logFile.Write(_T("Create ServerTasks Table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE ServerTasks ( ")
	_T("[ServerTaskID] [int] IDENTITY (1, 1) NOT NULL , ")
	_T("[ServerTaskTypeID] [int] NOT NULL , ")
	_T("[ServerID] [int] NOT NULL)"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE ServerTasks ADD ") 
	_T("CONSTRAINT [DF_ServerTasks_ServerTaskTypeID] DEFAULT (0) FOR [ServerTaskTypeID], ")
	_T("CONSTRAINT [DF_ServerTasks_ServerID] DEFAULT (0) FOR [ServerID], ")
	_T("CONSTRAINT [PK_ServerTasks] PRIMARY KEY  CLUSTERED ([ServerTaskID])"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Rows to ServerTasks Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to ServerTasks table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to ServerTasks table"));

	// Adding rows to ServerTasks table
	retSQL = dbConn.ExecuteSQL(_T("INSERT INTO ServerTasks (ServerTaskTypeID,ServerID) VALUES (1,1) ")
							   _T("INSERT INTO ServerTasks (ServerTaskTypeID,ServerID) VALUES (2,1) ")
							   _T("INSERT INTO ServerTasks (ServerTaskTypeID,ServerID) VALUES (3,1) ")
							   _T("INSERT INTO ServerTasks (ServerTaskTypeID,ServerID) VALUES (4,1) ")
							   _T("INSERT INTO ServerTasks (ServerTaskTypeID,ServerID) VALUES (5,1) ")
							   _T("INSERT INTO ServerTasks (ServerTaskTypeID,ServerID) VALUES (6,1) ")
							   _T("INSERT INTO ServerTasks (ServerTaskTypeID,ServerID) VALUES (7,1) ")
							   _T("INSERT INTO ServerTasks (ServerTaskTypeID,ServerID) VALUES (8,1) "));							   
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;

}