#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;

int V3000(CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Update AlertEvents table
	if(g_showResults != 0)
	{
		_tcout << _T("Update AlertEvents table") << endl;
	}

	g_logFile.Write(_T("Update AlertEvents table"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE AlertEvents SET Description = 'Disk or Database Full' WHERE Description = 'Disk Full'"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update ServerParameter table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding value to Server Parameter table") << endl;
	}

	g_logFile.Write(_T("Adding value to Server Parameter table"));

	// Adding value to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values(40,'Default Database Timeout','60') SET IDENTITY_INSERT ServerParameters OFF SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values(41,'Use Cutoff Read Date','0') SET IDENTITY_INSERT ServerParameters OFF SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values(42,'Cutoff Read Date','') SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update TicketStates table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding value to TicketStates table") << endl;
	}

	g_logFile.Write(_T("Adding value to TicketStates table"));

	// Adding value to TicketStates table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT TicketStates ON INSERT INTO TicketStates (TicketStateID, Description, HTMLColorName) VALUES (5,'Deleted','green') INSERT INTO TicketStates (TicketStateID, Description, HTMLColorName) VALUES (6,'Reopened','orange') SET IDENTITY_INSERT TicketStates OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter Routing Rules table, Tickets table, StandardResponses and TicketBoxes table") << endl;
	}

	g_logFile.Write(_T("Alter Routing Rules table, Tickets table, StandardResponses and TicketBoxes table"));

	// Alter Tickets table and TicketBoxes table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD AssignUniqueTicketID BIT DEFAULT 0 ALTER TABLE Tickets ADD UseTickler BIT DEFAULT 0 NOT NULL ALTER TABLE Tickets ADD TicklerDateToReopen DATETIME ALTER TABLE Tickets ADD FolderID INTEGER DEFAULT 0 NOT NULL ALTER TABLE TicketBoxes ADD DefaultEmailAddressName VARCHAR(50) ALTER TABLE StandardResponses ADD UseKeywords BIT DEFAULT 0 NOT NULL ALTER TABLE StandardResponses ADD Keywords VARCHAR(255) ALTER TABLE Agents ADD UseMarkAsRead BIT DEFAULT 0 NOT NULL ALTER TABLE Agents ADD MarkAsReadSeconds INT DEFAULT 5 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Update TicketBoxes table") << endl;
	}

	g_logFile.Write(_T("Update TicketBoxes table"));

	//Update TicketBoxes table
	retSQL = dbConn.ExecuteSQL(_T("UPDATE TicketBoxes SET DefaultEmailAddressName = Name"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Create InboundMessageRead table") << endl;
	}

	g_logFile.Write(_T("Create InboundMessageRead table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE InboundMessageRead ( InboundMessageReadID int IDENTITY(1,1) PRIMARY KEY not null, InboundMessageID int not null, AgentID int not null)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketNotesRead table") << endl;
	}

	g_logFile.Write(_T("Create TicketNotesRead table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE TicketNotesRead ( TicketNotesReadID int IDENTITY(1,1) PRIMARY KEY not null, TicketNoteID int not null, AgentID int not null)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Create SRKeywordResults table") << endl;
	}

	g_logFile.Write(_T("Create SRKeywordResults table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE SRKeywordResults ( SRKeywordResultsID int IDENTITY(1,1) PRIMARY KEY not null, InboundMessageID int not null, StandardResponseID int not null, Score int not null)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Create Folder table") << endl;
	}

	g_logFile.Write(_T("Create SRKeywordResults table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE Folders ( FolderID int IDENTITY(1,1) PRIMARY KEY not null, Name varchar(55) not null, AgentID int not null, ParentID int not null default 0)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Added ForeignKey constraints for InboundMessageRead, SRKeywordResults and TicketNotesRead") << endl;
	}

	g_logFile.Write(_T("Added ForeignKey constraints for InboundMessageRead, SRKeywordResults and TicketNotesRead"));
	
	retSQL = dbConn.ExecuteSQL(_T("alter table InboundMessageRead add constraint FK_InboundMessageRead_InboundMessages foreign key (InboundMessageID) references InboundMessages(InboundMessageID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("alter table SRKeywordResults add constraint FK_SRKeywordResults_InboundMessages foreign key (InboundMessageID) references InboundMessages(InboundMessageID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("alter table TicketNotesRead add constraint FK_TicketNotesRead_TicketNotes foreign key (TicketNoteID) references TicketNotes(TicketNoteID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("alter table Folders add constraint FK_Folders_TicketBoxViews foreign key (ParentID) references TicketBoxViews(TicketBoxViewID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}