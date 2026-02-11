#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6000(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	
	// Alter RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add QuoteOriginal column to RoutingRules table") << endl;
	}

	g_logFile.Write(_T("Add QuoteOriginal column to RoutingRules table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD QuoteOriginal BIT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	
	
	if(g_showResults != 0)
	{
		_tcout << _T("Add DoNotAssign column to RoutingRules table") << endl;
	}

	g_logFile.Write(_T("Add DoNotAssign column to RoutingRules table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD DoNotAssign INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Alter MessageSources Table
	if(g_showResults != 0)
	{
		_tcout << _T("Add ZipAttach column to MessageSources table") << endl;
	}

	g_logFile.Write(_T("Add ZipAttach column to MessageSources table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageSources ADD ZipAttach INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Alter Agents Table
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add StatusID column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add StatusID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD StatusID INT DEFAULT 5 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add AutoStatusTypeID column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add AutoStatusTypeID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD AutoStatusTypeID INT DEFAULT 2 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add AutoStatusMin column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add AutoStatusMin column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD AutoStatusMin INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add LoginStatusID column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add LoginStatusID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD LoginStatusID INT DEFAULT 1 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add LogoutStatusID column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add LogoutStatusID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD LogoutStatusID INT DEFAULT 5 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add StatusText column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add StatusText column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD StatusText VARCHAR(125) DEFAULT 'Offline' NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add OnlineText column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add OnlineText column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD OnlineText VARCHAR(125) DEFAULT 'Online' NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add AwayText column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add AwayText column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD AwayText VARCHAR(125) DEFAULT 'Away' NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add NotAvailText column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add NotAvailText column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD NotAvailText VARCHAR(125) DEFAULT 'Not Available' NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add DndText column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add DndText column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD DndText VARCHAR(125) DEFAULT 'Do Not Disturb' NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add OfflineText column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add OfflineText column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD OfflineText VARCHAR(125) DEFAULT 'Offline' NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add OooText column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add OooText column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD OooText VARCHAR(125) DEFAULT 'Out of Office' NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Rows to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (96,'Session Timeout','30') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (97,'Session Keep Alive','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (98,'Deny Ticket Assignment Online','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (99,'Deny Ticket Assignment Away','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (100,'Deny Ticket Assignment Not Avail','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (101,'Deny Ticket Assignment Offline','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (102,'Deny Ticket Assignment Out of Office','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (103,'Allow Ticket Assignment Administrator','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (104,'Allow Ticket Ownership Agent','1') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (105,'Default Routing Rule Deny Online','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (106,'Default Routing Rule Deny Away','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (107,'Default Routing Rule Deny Not Avail','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (108,'Default Routing Rule Deny Offline','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (109,'Default Routing Rule Deny Out of Office','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (110,'Agent Activity Ticket Message Copied','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (111,'Agent Activity Ticket Merged','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (112,'Agent Activity Ticket Message Saved','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (113,'Agent Activity Ticket Link Created','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (114,'Agent Activity Ticket Link Deleted','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (115,'Agent Activity Ticket Linked','0') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (116,'Agent Activity Ticket Unlinked','0') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add TicketLinks
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketLinks table") << endl;
	}

	g_logFile.Write(_T("Create TicketLinks table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[TicketLinks]( ")
	_T("[TicketLinkID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[LinkName] [varchar](50) NOT NULL, ")
	_T("[DateCreated] [datetime] NOT NULL CONSTRAINT [DF_TicketLinks_DateCreated]  DEFAULT (getdate()), ")
	_T("[OwnerID] [int] NOT NULL CONSTRAINT [DF_TicketLinks_OwnerID] DEFAULT ((0)), ")
	_T("[IsDeleted] [int] NOT NULL CONSTRAINT [DF_TicketLinks_IsDeleted] DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_TicketLinks] PRIMARY KEY CLUSTERED ([TicketLinkID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add TicketLinksTicket
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketLinksTicket table") << endl;
	}

	g_logFile.Write(_T("Create TicketLinksTicket table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[TicketLinksTicket]( ")
	_T("[TicketLinksTicketID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[TicketLinkID] [int] NOT NULL CONSTRAINT [DF_TicketLinksTicket_TicketLinkID] DEFAULT ((0)), ")
	_T("[TicketID] [int] NOT NULL CONSTRAINT [DF_TicketLinksTicket_TicketID] DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_TicketLinksTicket] PRIMARY KEY CLUSTERED ([TicketLinksTicketID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Columns to TicketBoxes
	if(g_showResults != 0)
	{
		_tcout << _T("Add TicketLink column to TicketBoxes table") << endl;
	}

	g_logFile.Write(_T("Add TicketLink column to TicketBoxes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxes ADD TicketLink INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
		
	//Add ContactGroups Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create ContactGroups table") << endl;
	}

	g_logFile.Write(_T("Create ContactGroups table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[ContactGroups]( ")
	_T("[ContactGroupID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[GroupName] [varchar](50) NOT NULL, ")
	_T("[OwnerID] [int] NOT NULL CONSTRAINT [DF_ContactGroups_OwnerID] DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_ContactGroups] PRIMARY KEY CLUSTERED ([ContactGroupID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add ContactGrouping Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create ContactGrouping table") << endl;
	}

	g_logFile.Write(_T("Create ContactGrouping table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[ContactGrouping]( ")
	_T("[ContactGroupingID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[ContactGroupID] [int] NOT NULL CONSTRAINT [DF_ContactGrouping_ContactGroupID] DEFAULT ((0)), ")
	_T("[ContactID] [int] NOT NULL CONSTRAINT [DF_ContactGrouping_ContactID] DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_ContactGrouping] PRIMARY KEY CLUSTERED ([ContactGroupingID])) "));
	
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
	
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT TicketActions ON INSERT INTO TicketActions (TicketActionID,Description) VALUES (20,'Ticket linked') INSERT INTO TicketActions (TicketActionID,Description) VALUES (21,'Ticket unlinked') SET IDENTITY_INSERT TicketActions OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Alter TicketHistory add DataValue
	if(g_showResults != 0)
	{
		_tcout << _T("Alter TicketHistory Table, add DataValue column") << endl;
	}

	g_logFile.Write(_T("Alter TicketHistory Table, add DataValue column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketHistory ADD DataValue VARCHAR(50) DEFAULT '' NOT NULL"));
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
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (26,'Created Link') ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (27,'Deleted Link') ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (28,'Ticket Linked') ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (29,'Ticket Unlinked') ")
							   _T("SET IDENTITY_INSERT AgentActivities OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	
	
	return 0;
}