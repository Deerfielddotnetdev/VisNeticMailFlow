#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V5000(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	
	// Alter RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to RoutingRules table") << endl;
	}

	g_logFile.Write(_T("Add column to RoutingRules table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD AllowRemoteReply BIT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	
	
	// Alter TicketBoxes
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to TicketBoxes table") << endl;
	}

	g_logFile.Write(_T("Add column to TicketBoxes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxes ADD MessageDestinationID INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Alter MessageSources
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to MessageSources table") << endl;
	}

	g_logFile.Write(_T("Add column to MessageSources table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageSources ADD OfficeHours INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	

	// Alter MessageDestinations
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to MessageDestinations table") << endl;
	}

	g_logFile.Write(_T("Add column to MessageDestinations table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageDestinations ADD SSLMode INT DEFAULT 1 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Add column to MessageDestinations table") << endl;
	}

	g_logFile.Write(_T("Add column to MessageDestinations table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageDestinations ADD OfficeHours INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add AutoActionTypes
	if(g_showResults != 0)
	{
		_tcout << _T("Create AutoActionTypes table") << endl;
	}

	g_logFile.Write(_T("Create AutoActionTypes table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[AutoActionTypes]( ")
	_T("[AutoActionTypeID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[TypeName] [varchar](64) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL, ")
	_T("[BuiltIn] [bit] NOT NULL CONSTRAINT [DF_AutoActionTypes_SetDefault]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_AutoActionTypes] PRIMARY KEY CLUSTERED ([AutoActionTypeID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//AutoActionTypes
	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to AutoActionTypes") << endl;
	}

	g_logFile.Write(_T("Add rows to AutoActionTypes"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT AutoActionTypes ON ")
	_T("INSERT INTO [AutoActionTypes]([AutoActionTypeID],[TypeName],[BuiltIn]) VALUES (1,'Change Owner to',1) ")
	_T("INSERT INTO [AutoActionTypes]([AutoActionTypeID],[TypeName],[BuiltIn]) VALUES (2,'Change State to',1) ")
	_T("INSERT INTO [AutoActionTypes]([AutoActionTypeID],[TypeName],[BuiltIn]) VALUES (3,'Delete Ticket',1) ")
	_T("INSERT INTO [AutoActionTypes]([AutoActionTypeID],[TypeName],[BuiltIn]) VALUES (4,'Escalate to',1) ")
	_T("INSERT INTO [AutoActionTypes]([AutoActionTypeID],[TypeName],[BuiltIn]) VALUES (5,'Move to',1) ")	
	_T("SET IDENTITY_INSERT AutoActionTypes OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add AutoActionEvents
	if(g_showResults != 0)
	{
		_tcout << _T("Create AutoActionEvents table") << endl;
	}

	g_logFile.Write(_T("Create AutoActionEvents table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[AutoActionEvents]( ")
	_T("[AutoActionEventID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[EventName] [varchar](64) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL, ")
	_T("[BuiltIn] [bit] NOT NULL CONSTRAINT [DF_AutoActionEvents_SetDefault]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_AutoActionEvents] PRIMARY KEY CLUSTERED ([AutoActionEventID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//AutoActionEvents
	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to AutoActionEvents") << endl;
	}

	g_logFile.Write(_T("Add rows to AutoActionEvents"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT AutoActionEvents ON ")
	_T("INSERT INTO [AutoActionEvents]([AutoActionEventID],[EventName],[BuiltIn]) VALUES (1,'Ticket Created',1) ")
	_T("INSERT INTO [AutoActionEvents]([AutoActionEventID],[EventName],[BuiltIn]) VALUES (2,'Ticket Opened',1) ")
	_T("INSERT INTO [AutoActionEvents]([AutoActionEventID],[EventName],[BuiltIn]) VALUES (3,'Ticket Closed',1) ")
	_T("INSERT INTO [AutoActionEvents]([AutoActionEventID],[EventName],[BuiltIn]) VALUES (4,'Ticket Escalated',1) ")
	_T("INSERT INTO [AutoActionEvents]([AutoActionEventID],[EventName],[BuiltIn]) VALUES (5,'Ticket Moved',1) ")	
	_T("INSERT INTO [AutoActionEvents]([AutoActionEventID],[EventName],[BuiltIn]) VALUES (6,'Ticket Reassigned',1) ")	
	_T("INSERT INTO [AutoActionEvents]([AutoActionEventID],[EventName],[BuiltIn]) VALUES (7,'Inbound Message Added',1) ")	
	_T("INSERT INTO [AutoActionEvents]([AutoActionEventID],[EventName],[BuiltIn]) VALUES (8,'Outbound Message Added',1) ")	
	_T("INSERT INTO [AutoActionEvents]([AutoActionEventID],[EventName],[BuiltIn]) VALUES (9,'Note Added',1) ")	
	_T("SET IDENTITY_INSERT AutoActionEvents OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add AutoActions
	if(g_showResults != 0)
	{
		_tcout << _T("Create AutoActionts table") << endl;
	}

	g_logFile.Write(_T("Create AutoActions table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[AutoActions]( ")
	_T("[AutoActionID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[TicketBoxID] [int] NOT NULL CONSTRAINT [DF_AutoActions_TicketBoxID]  DEFAULT ((0)), ")
	_T("[AutoActionVal] [int] NOT NULL CONSTRAINT [DF_AutoActions_AutoActionVal]  DEFAULT ((0)), ")
	_T("[AutoActionFreq] [int] NOT NULL CONSTRAINT [DF_AutoActions_AutoActionFreq]  DEFAULT ((0)), ")
	_T("[AutoActionEventID] [int] NOT NULL CONSTRAINT [DF_AutoActions_AutoActionEventID]  DEFAULT ((0)), ")
	_T("[AutoActionTypeID] [int] NOT NULL CONSTRAINT [DF_AutoActions_AutoActionTypeID]  DEFAULT ((0)), ")
	_T("[AutoActionTargetID] [int] NOT NULL CONSTRAINT [DF_AutoActions_AutoActionTargetID]  DEFAULT ((0)), ")
	_T("[DateCreated] [datetime] NOT NULL CONSTRAINT [DF_AutoActions_DateCreated]  DEFAULT (getdate()), ")
	_T("CONSTRAINT [PK_AutoActions] PRIMARY KEY CLUSTERED ([AutoActionID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add OfficeHours
	if(g_showResults != 0)
	{
		_tcout << _T("Create OfficeHours table") << endl;
	}

	g_logFile.Write(_T("Create OfficeHours table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[OfficeHours]( ")
	_T("[OfficeHourID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[TypeID] [int] NOT NULL CONSTRAINT [DF_OfficeHours_TypeID]  DEFAULT ((0)), ")
	_T("[ActualID] [int] NOT NULL CONSTRAINT [DF_OfficeHours_ActualID]  DEFAULT ((0)), ")
	_T("[StartHr] [int] NOT NULL CONSTRAINT [DF_OfficeHours_StartHr]  DEFAULT ((0)), ")
	_T("[StartMin] [int] NOT NULL CONSTRAINT [DF_OfficeHours_StartMin]  DEFAULT ((0)), ")
	_T("[StartAmPm] [int] NOT NULL CONSTRAINT [DF_OfficeHours_StartAmPm]  DEFAULT ((0)), ")
	_T("[EndHr] [int] NOT NULL CONSTRAINT [DF_OfficeHours_EndHr]  DEFAULT ((0)), ")
	_T("[EndMin] [int] NOT NULL CONSTRAINT [DF_OfficeHours_EndMin]  DEFAULT ((0)), ")
	_T("[EndAmPm] [int] NOT NULL CONSTRAINT [DF_OfficeHours_EndAmPm]  DEFAULT ((0)), ")
	_T("[DayID] [int] NOT NULL CONSTRAINT [DF_OfficeHours_DayID]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_OfficeHours] PRIMARY KEY CLUSTERED ([OfficeHourID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//OfficeHours
	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to OfficeHours") << endl;
	}

	g_logFile.Write(_T("Add rows to OfficeHours"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT OfficeHours ON ")
	_T("INSERT INTO [OfficeHours]([OfficeHourID],[TypeID],[ActualID],[StartHr],[StartMin],[StartAmPm],[EndHr],[EndMin],[EndAmPm],[DayID]) VALUES (1,0,0,0,60,0,0,60,0,1) ")
	_T("INSERT INTO [OfficeHours]([OfficeHourID],[TypeID],[ActualID],[StartHr],[StartMin],[StartAmPm],[EndHr],[EndMin],[EndAmPm],[DayID]) VALUES (2,0,0,8,0,1,5,0,2,2) ")
	_T("INSERT INTO [OfficeHours]([OfficeHourID],[TypeID],[ActualID],[StartHr],[StartMin],[StartAmPm],[EndHr],[EndMin],[EndAmPm],[DayID]) VALUES (3,0,0,8,0,1,5,0,2,3) ")
	_T("INSERT INTO [OfficeHours]([OfficeHourID],[TypeID],[ActualID],[StartHr],[StartMin],[StartAmPm],[EndHr],[EndMin],[EndAmPm],[DayID]) VALUES (4,0,0,8,0,1,5,0,2,4) ")
	_T("INSERT INTO [OfficeHours]([OfficeHourID],[TypeID],[ActualID],[StartHr],[StartMin],[StartAmPm],[EndHr],[EndMin],[EndAmPm],[DayID]) VALUES (5,0,0,8,0,1,5,0,2,5) ")
	_T("INSERT INTO [OfficeHours]([OfficeHourID],[TypeID],[ActualID],[StartHr],[StartMin],[StartAmPm],[EndHr],[EndMin],[EndAmPm],[DayID]) VALUES (6,0,0,8,0,1,5,0,2,6) ")
	_T("INSERT INTO [OfficeHours]([OfficeHourID],[TypeID],[ActualID],[StartHr],[StartMin],[StartAmPm],[EndHr],[EndMin],[EndAmPm],[DayID]) VALUES (7,0,0,0,60,0,0,60,0,7) ")
	_T("SET IDENTITY_INSERT OfficeHours OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Check if PersonalDataType of "Remote Reply Address" exists
	if(g_showResults != 0)
	{
		_tcout << _T("Check to see if PersonalDataType of Remote Reply Address exists") << endl;
	}
	g_logFile.Write(_T("Check to see if PersonalDataType of Remote Reply Address exists"));
	
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, maxObjectID );
	m_query.Execute( L"SELECT PersonalDataTypeID FROM PersonalDataTypes WHERE TypeName='Remote Reply Address'" );
	if ( m_query.GetRowCount() > 0 )
	{
		if(g_showResults != 0)
		{
			_tcout << _T("PersonalDataType of Remote Reply Address exists") << endl;
		}
		g_logFile.Write(_T("PersonalDataType of Remote Reply Address exists"));
	}
	else
	{
		if(g_showResults != 0)
		{
			_tcout << _T("PersonalDataType of Remote Reply Address does not exist, adding now") << endl;
		}
		g_logFile.Write(_T("PersonalDataType of Remote Reply Address does not exist, adding now"));

		retSQL = dbConn.ExecuteSQL( _T("INSERT INTO [PersonalDataTypes]([TypeName],[BuiltIn]) VALUES ('Remote Reply Address',0)") );
		
		if(!SQL_SUCCEEDED(retSQL))
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}
	
	return 0;
}