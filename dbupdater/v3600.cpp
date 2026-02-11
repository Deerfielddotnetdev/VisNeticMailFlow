#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V3600(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Add Columns to TicketBoxViews


	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to TicketBoxViews table") << endl;
	}

	g_logFile.Write(_T("Add columns to TicketBoxViews table"));

	// Add columns to TicketBoxViews table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxViews ADD ShowState BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD ShowPriority BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD ShowNumNotes BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD ShowTicketID BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD ShowNumMsgs BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD ShowSubject BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD ShowContact BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD ShowDate BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD ShowCategory BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD ShowOwner BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD ShowTicketBox BIT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE TicketBoxViews ADD UseDefault BIT DEFAULT 1 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Drop TicketBoxViews TicketBoxViewTypeID FK Constraint
	if(g_showResults != 0)
	{
		_tcout << _T("Drop Constraint FK_TicketBoxViewTypes") << endl;
	}

	g_logFile.Write(_T("Drop Constraint FK_TicketBoxViewTypes"));

	// Alter the TicketBoxViews table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxViews DROP CONSTRAINT FK_TicketBoxViewTypes"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Add row to TicketBoxViewTypes
	if(g_showResults != 0)
	{
		_tcout << _T("Add Column Defaults row to TicketBoxViewTypes") << endl;
	}

	g_logFile.Write(_T("Add Column Defaults row to TicketBoxViewTypes"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT TicketBoxViewTypes ON INSERT INTO TicketBoxViewTypes (TicketBoxViewTypeID,Name) VALUES (9,'Column Defaults') SET IDENTITY_INSERT TicketBoxViewTypes OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add rows to TicketActions table
	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to TicketActions") << endl;
	}

	g_logFile.Write(_T("Add rows to TicketActions"));
	
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT TicketActions ON INSERT INTO TicketActions (TicketActionID,Description) VALUES (10,'Added note to ticket') INSERT INTO TicketActions (TicketActionID,Description) VALUES (11,'Removed note from ticket') INSERT INTO TicketActions (TicketActionID,Description) VALUES (12,'Note read') INSERT INTO TicketActions (TicketActionID,Description) VALUES (13,'Ticket restored') INSERT INTO TicketActions (TicketActionID,Description) VALUES (14,'Ticket moved') SET IDENTITY_INSERT TicketActions OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add New Tables
	/*
	TicketFields
	TicketFieldsTicketBox
	TicketFieldsTicket
	TicketFieldViews
	*/

	//Add TicketFields
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketFields table") << endl;
	}

	g_logFile.Write(_T("Create TicketFields table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[TicketFields]( ")
	_T("[TicketFieldID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_TicketFields_Description]  DEFAULT (''), ")
	_T("CONSTRAINT [PK_TicketFields] PRIMARY KEY CLUSTERED ([TicketFieldID])) "));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add TicketFieldsTicketBox
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketFieldsTicketBox table") << endl;
	}

	g_logFile.Write(_T("Create TicketFieldsTicketBox table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[TicketFieldsTicketBox]( ")
	_T("[TicketFieldsTicketBoxID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[TicketBoxID] [int] NOT NULL CONSTRAINT [DF_TicketFieldsTicketBox_TicketBoxID]  DEFAULT ((0)), ")
	_T("[TicketFieldID] [int] NOT NULL CONSTRAINT [DF_TicketFieldsTicketBox_TicketFieldID]  DEFAULT ((0)), ")
	_T("[IsRequired] [bit] NOT NULL CONSTRAINT [DF_TicketFieldsTicketBox_IsRequired]  DEFAULT ((0)), ")
	_T("[IsViewed] [bit] NOT NULL CONSTRAINT [DF_TicketFieldsTicketBox_IsViewed]  DEFAULT ((1)), ")
	_T("[SetDefault] [bit] NOT NULL CONSTRAINT [DF_TicketFieldsTicketBox_SetDefault]  DEFAULT ((0)), ")
	_T("[DefaultValue] [varchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_TicketFieldsTicketBox_DefaultValue]  DEFAULT (''), ")
	_T("CONSTRAINT [PK_TicketFieldsTicketBox] PRIMARY KEY CLUSTERED ([TicketFieldsTicketBoxID])) "));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add TicketFieldsTicket
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketFieldsTicket table") << endl;
	}

	g_logFile.Write(_T("Create TicketFieldsTicket table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[TicketFieldsTicket]( ")
	_T("[TicketFieldsTicketID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[TicketID] [int] NOT NULL CONSTRAINT [DF_TicketFieldsTicket_TicketID]  DEFAULT ((0)), ")
	_T("[TicketFieldID] [int] NOT NULL CONSTRAINT [DF_TicketFieldsTicket_TicketFieldID]  DEFAULT ((0)), ")
	_T("[DataValue] [varchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_TicketFieldsTicket_DataValue]  DEFAULT (''), ")
	_T("CONSTRAINT [PK_TicketFieldsTicket] PRIMARY KEY CLUSTERED ([TicketFieldsTicketID])) "));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add TicketFieldViews
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketFieldViews table") << endl;
	}

	g_logFile.Write(_T("Create TicketFieldViews table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[TicketFieldViews]( ")
	_T("[TicketFieldViewID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[TicketBoxViewID] [int] NOT NULL CONSTRAINT [DF_TicketFieldViews_TicketBoxViewID]  DEFAULT ((0)), ")
	_T("[TicketFieldID] [int] NOT NULL CONSTRAINT [DF_TicketFieldViews_TicketFieldID]  DEFAULT ((0)), ")
	_T("[ShowField] [bit] NOT NULL CONSTRAINT [DF_TicketFieldViews_ShowField]  DEFAULT ((1)), ")
	_T("CONSTRAINT [PK_TicketFieldViews] PRIMARY KEY CLUSTERED ([TicketFieldViewID])) "));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update ObjectTypes table
	//Add Auto Ticket Open to ObjectTypes as ID 9.
	//Add Agent Audit to ObjectTypes as ID 10.

	if(g_showResults != 0)
	{
		_tcout << _T("Add Auto Ticket Open and Agent Audit rows to ObjectTypes") << endl;
	}

	g_logFile.Write(_T("Add Auto Ticket Open and Agent Audit rows to ObjectTypes"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ObjectTypes ON INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (9,'Auto Ticket Open') INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (10,'Agent Audit') SET IDENTITY_INSERT ObjectTypes OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	TCHAR buf[1024];
	int newObjectID = 0;
	int newObjectID2 = 0;
		
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
			newObjectID2 = newObjectID + 1;
			if(g_showResults != 0)
			{
				wsprintf(buf,_T("Received MAX ObjectID: %d incrementing to %d"),maxObjectID,newObjectID);
				_tcout << buf << endl;
			}
			maxObjectID = newObjectID2;
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
		newObjectID2 = newObjectID + 1;
		if(g_showResults != 0)
		{
			wsprintf(buf,_T("Received MAX ObjectID: %d incrementing to %d"),maxObjectID,newObjectID);
			_tcout << buf << endl;
		}
		maxObjectID = newObjectID2;
	}
	
	//Update Objects table
	//Add Auto Ticket Open to Objects.
	//Add Agent Audit to Objects

	if(g_showResults != 0)
	{
		_tcout << _T("Add Auto Ticket Open and Agent Audit rows to Objects") << endl;
	}

	g_logFile.Write(_T("Add Auto Ticket Open and Agent Audit rows to Objects"));

	wsprintf(buf,_T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights) VALUES (%d,0,9,1,0) INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights) VALUES (%d,0,10,1,0) SET IDENTITY_INSERT Objects OFF"), newObjectID, newObjectID2 );
	retSQL = dbConn.ExecuteSQL(buf);if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Update AccessControl table
	//Add Entry to AccessControl with GroupID=1, AccessLevel=4 and ObjectID = Auto Ticket Open ID from Objects Table.

	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to AccessControl") << endl;
	}

	g_logFile.Write(_T("Add rows to AccessControl"));
	wsprintf(buf,_T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d) INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,0,%d)"), newObjectID, newObjectID2 );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add AgentActivityLog
	if(g_showResults != 0)
	{
		_tcout << _T("Create AgentActivityLog table") << endl;
	}

	g_logFile.Write(_T("Create AgentActivityLog table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[AgentActivityLog]( ")
	_T("[AgentActivityID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[AgentID] [int] NOT NULL CONSTRAINT [DF_AgentActivityLog_AgentID]  DEFAULT ((0)), ")
	_T("[ActivityID] [int] NOT NULL CONSTRAINT [DF_AgentActivityLog_AuthActionID]  DEFAULT ((0)), ")
	_T("[ActivityDate] [datetime] NOT NULL CONSTRAINT [DF_AgentActivityLog_ActivityDate]  DEFAULT (getdate()), ")
	_T("[ID1] [int] NOT NULL CONSTRAINT [DF_AgentActivityLog_ID1]  DEFAULT ((0)), ")
	_T("[ID2] [int] NOT NULL CONSTRAINT [DF_AgentActivityLog_ID2]  DEFAULT ((0)), ")
	_T("[ID3] [int] NOT NULL CONSTRAINT [DF_AgentActivityLog_ID3]  DEFAULT ((0)), ")
	_T("[Data1] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_AgentActivityLog_Data1]  DEFAULT (''), ")
	_T("[Data2] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_AgentActivityLog_Data2]  DEFAULT (''), ")
	_T("CONSTRAINT [PK_AgentActivityLog] PRIMARY KEY CLUSTERED ([AgentActivityID])) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add AgentActivities
	if(g_showResults != 0)
	{
		_tcout << _T("Create AgentActivities table") << endl;
	}

	g_logFile.Write(_T("Create AgentActivities table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[AgentActivities]( ")
	_T("[AgentActivityID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_AgentActivities_Description]  DEFAULT (''), ")
	_T("CONSTRAINT [PK_AgentActivities] PRIMARY KEY CLUSTERED ([AgentActivityID])) "));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Rows to AgentActivities Table
	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to AgentActivities") << endl;
	}

	g_logFile.Write(_T("Add rows to AgentActivities"));
	
	retSQL = dbConn.ExecuteSQL(_T("INSERT INTO AgentActivities (Description) VALUES ('Logged In') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Logged Out') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Deleted Ticket') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Created Ticket') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Moved Ticket') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Escalated Ticket') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Reassigned Ticket') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Restored Ticket') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Opened Ticket') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Closed Ticket') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Added Message') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Deleted Message') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Read Message') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Added Note') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Deleted Note') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Read Note') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Read Alert') ")
							   _T("INSERT INTO AgentActivities (Description) VALUES ('Deleted Alert') "));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Rows to Server Parameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to Server Parameter table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to Server Parameter table"));

	// Adding value to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (48,'Agent Activity Log','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (49,'Agent Activity Authentication','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (50,'Agent Activity Ticket Delete','1') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (51,'Agent Activity Ticket Create','1') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (52,'Agent Activity Ticket Move','1') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (53,'Agent Activity Ticket Escalate','1') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (54,'Agent Activity Ticket Reassign','1') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (55,'Agent Activity Ticket Restore','1') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (56,'Agent Activity Ticket Open','1') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (57,'Agent Activity Ticket Close','1') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (58,'Agent Activity Ticket Message Add','1') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (59,'Agent Activity Ticket Message Delete','1') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (60,'Agent Activity Ticket Message Read','0') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (61,'Agent Activity Ticket Note Add','0') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (62,'Agent Activity Ticket Note Delete','0') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (63,'Agent Activity Ticket Note Read','0') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (64,'Agent Activity Alert Read','0') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (65,'Agent Activity Alert Delete','0') ")
                               _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (66,'Max TicketLock Time','60') ")
                               _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update Tickets table
	//Add LockedTime column

	if(g_showResults != 0)
	{
		_tcout << _T("Add LockedTime column to Tickets") << endl;
	}

	g_logFile.Write(_T("Add LockedTime column to Tickets"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Tickets ADD LockedTime DATETIME"));
							   
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
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (10,'Ticket Activity',1) ")
	_T("SET IDENTITY_INSERT ReportTypes OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//ReportStandard
	if(g_showResults != 0)
	{
		_tcout << _T("Add row to ReportStandard") << endl;
	}

	g_logFile.Write(_T("Add row to ReportStandard"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ReportStandard ON ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (30,'Ticket Activity By TicketBox',1,10,1) ")
	_T("SET IDENTITY_INSERT ReportStandard OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Columns to ReportResultRows
	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to ReportResultRows") << endl;
	}

	g_logFile.Write(_T("Add columns to ReportResultRows"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE ReportResultRows ADD Col6 INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE ReportResultRows ADD Col7 INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE ReportResultRows ADD Col8 INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE ReportResultRows ADD Col9 INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE ReportResultRows ADD Col10 INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE ReportResultRows ADD Col11 INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE ReportResultRows ADD Col12 INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE ReportResultRows ADD Col13 INT DEFAULT 0 NOT NULL") );
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Columns to TicketHistory


	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to TicketHistory table") << endl;
	}

	g_logFile.Write(_T("Add columns to TicketHistory table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketHistory ADD TicketStateID INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE TicketHistory ADD TicketBoxID INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE TicketHistory ADD OwnerID INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE TicketHistory ADD PriorityID INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE TicketHistory ADD TicketCategoryID INT DEFAULT 0 NOT NULL") );
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	
	return 0;

}