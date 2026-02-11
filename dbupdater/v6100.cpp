#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6100(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
		
	//Add Column to TicketFields
	if(g_showResults != 0)
	{
		_tcout << _T("Add TicketFieldTypeID column to TicketFields table") << endl;
	}

	g_logFile.Write(_T("Add TicketFieldTypeID column to TicketFields table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketFields ADD TicketFieldTypeID INT DEFAULT 1 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	
	
	//Add TicketFieldTypes
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketFieldTypes table") << endl;
	}

	g_logFile.Write(_T("Create TicketFieldTypes table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[TicketFieldTypes]( ")
	_T("[TicketFieldTypeID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[Description] [varchar](50) NOT NULL, ")
	_T("CONSTRAINT [PK_TicketFieldTypes] PRIMARY KEY CLUSTERED ([TicketFieldTypeID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Rows to TicketFieldTypes Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to TicketFieldTypes table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to TicketFieldTypes table"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT TicketFieldTypes ON ")
							   _T("INSERT INTO TicketFieldTypes (TicketFieldTypeID,Description) VALUES (1,'Text') ")
							   _T("INSERT INTO TicketFieldTypes (TicketFieldTypeID,Description) VALUES (2,'CheckBox') ")
							   _T("INSERT INTO TicketFieldTypes (TicketFieldTypeID,Description) VALUES (3,'Select') ")
							   _T("SET IDENTITY_INSERT TicketFieldTypes OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add TicketFieldOptions
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketFieldOptions table") << endl;
	}

	g_logFile.Write(_T("Create TicketFieldOptions table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[TicketFieldOptions]( ")
	_T("[TicketFieldOptionID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[TicketFieldID] [int] NOT NULL CONSTRAINT [DF_TicketFieldOptions_TicketFieldID] DEFAULT ((0)), ")
	_T("[OptionValue] [varchar](50) NOT NULL, ")
	_T("[OptionOrder] [int] NOT NULL CONSTRAINT [DF_TicketFieldOptions_OptionOrder] DEFAULT ((0)), ")	
	_T("CONSTRAINT [PK_TicketFieldOptions] PRIMARY KEY CLUSTERED ([TicketFieldOptionID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add columns to TicketBoxes
	if(g_showResults != 0)
	{
		_tcout << _T("Add RequireTC, ReplyToEmailAddress and ReturnPathEmailAddress columns to TicketBoxes table") << endl;
	}

	g_logFile.Write(_T("Add RequireTC, ReplyToEmailAddress and ReturnPathEmailAddress columns to TicketBoxes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxes ADD RequireTC INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE TicketBoxes ADD ReplyToEmailAddress VARCHAR(255) DEFAULT ('') NOT NULL ")
							   _T("ALTER TABLE TicketBoxes ADD ReturnPathEmailAddress VARCHAR(255) DEFAULT ('') NOT NULL") );
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Column to MessageSources
	if(g_showResults != 0)
	{
		_tcout << _T("Add DupMsg and LeaveCopiesDays columns to MessageSources table") << endl;
	}

	g_logFile.Write(_T("Add DupMsg and LeaveCopiesDays columns to MessageSources table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageSources ADD DupMsg INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE MessageSources ADD LeaveCopiesDays INT DEFAULT 0 NOT NULL") );
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add column to UIDLData
	if(g_showResults != 0)
	{
		_tcout << _T("Add UIDLDate column to UIDLData table") << endl;
	}

	g_logFile.Write(_T("Add UIDLDate column to UIDLData table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE UIDLData ADD UIDLDate DATETIME DEFAULT (GetDate()) NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	

	//Add MessageTracking table
	if(g_showResults != 0)
	{
		_tcout << _T("Create MessageTracking table") << endl;
	}

	g_logFile.Write(_T("Create MessageTracking table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[MessageTracking]( ")
	_T("[MessageTrackingID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[MessageID] [int] NOT NULL CONSTRAINT [DF_MessageTracking_MessageID] DEFAULT (0), ")
	_T("[MessageSourceID] [int] NOT NULL CONSTRAINT [DF_MessageTracking_MessageSourceID] DEFAULT (0), ")
	_T("[HeadMsgID] [varchar](255) NOT NULL CONSTRAINT [DF_MessageTracking_HeadMsgID] DEFAULT (''), ")
	_T("[HeadInReplyTo] [varchar](255) NOT NULL CONSTRAINT [DF_MessageTracking_HeadInReplyTo] DEFAULT (''), ")
	_T("[HeadReferences] [varchar](255) NOT NULL CONSTRAINT [DF_MessageTracking_HeadReferences] DEFAULT (''), ")
	_T("CONSTRAINT [PK_MessageTracking] PRIMARY KEY CLUSTERED ([MessageTrackingID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Column to RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add SetOpenOwner column to RoutingRules table") << endl;
	}

	g_logFile.Write(_T("Add SetOpenOwner column to RoutingRules table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD SetOpenOwner INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Row to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Row to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (118,'Default Routing Rule Check Owner','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (119,'Verify Contact Email Frequency','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (120,'Verify Contact Email No MX','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (121,'Verify Contact Email Catch All','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (122,'Verify Contact Email Remove','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (123,'Verify Contact Email Delete','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (124,'Verify Contact Email Decrement','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (125,'Verify Contact Email Increment','2') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (126,'Verify HELO Host Name','') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (127,'Verify Connect Timeout','10') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (128,'Update Frequency','30') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (129,'Agent Activity Change TicketCategory','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (130,'Agent Activity Change TicketField','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (131,'Enable Contact Email Verification','0') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Columns to PersonalData
	if(g_showResults != 0)
	{
		_tcout << _T("Add StatusID and StatusDate columns to PersonalData table") << endl;
	}

	g_logFile.Write(_T("Add StatusID and StatusDate columns to PersonalData table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE PersonalData ADD StatusID INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE PersonalData ADD StatusDate DATETIME DEFAULT (GetDate()) NOT NULL") );
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Bypass table
	if(g_showResults != 0)
	{
		_tcout << _T("Create Bypass table") << endl;
	}

	g_logFile.Write(_T("Create Bypass table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[Bypass]( ")
	_T("[BypassID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[BypassTypeID] [int] NOT NULL CONSTRAINT [DF_Bypass_BypassTypeID] DEFAULT (0), ")
	_T("[BypassValue] [varchar](255) NOT NULL CONSTRAINT [DF_Bypass_BypassValue] DEFAULT (''), ")
	_T("[DateCreated] [datetime] NOT NULL CONSTRAINT [DF_Bypass_DateCreated] DEFAULT (getdate()), ")
	_T("CONSTRAINT [PK_Bypass] PRIMARY KEY CLUSTERED ([BypassID])) "));
	
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
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (30,'Changed TicketCategory') ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (31,'Changed TicketField') ")
							   _T("SET IDENTITY_INSERT AgentActivities OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}