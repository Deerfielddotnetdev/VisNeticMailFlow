#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V4500(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Drop ContactNotes ContactID FK Constraint
	if(g_showResults != 0)
	{
		_tcout << _T("Drop Constraint FK_ContactNoteContacts") << endl;
	}

	g_logFile.Write(_T("Drop Constraint FK_ContactNoteContacts"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE ContactNotes DROP CONSTRAINT FK_ContactNoteContacts"));
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
							   _T("INSERT INTO TicketBoxViewTypes (TicketBoxViewTypeID,Name) VALUES (11,'My Notes') ")							   
							   _T("SET IDENTITY_INSERT TicketBoxViewTypes OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Row to TicketStates table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding row to TicketStates table") << endl;
	}

	g_logFile.Write(_T("Adding row to TicketStates table"));

	// Adding row to TicketStates table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT TicketStates ON INSERT INTO TicketStates (TicketStateID, Description, HTMLColorName) VALUES (7,'Waiting Response','purple') SET IDENTITY_INSERT TicketStates OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Row to AgentActivities Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to AgentActivities table") << endl;
	}

	g_logFile.Write(_T("Adding Row to AgentActivities table"));

	// Adding rows to AgentActivities table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT AgentActivities ON ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (23,'Copied Message') ")							   
							   _T("SET IDENTITY_INSERT AgentActivities OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Alter ContactNotes
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to ContactNotes table") << endl;
	}

	g_logFile.Write(_T("Add column to ContactNotes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE ContactNotes ADD Subject VARCHAR(50) DEFAULT '' NOT NULL"));							   
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
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (89,'Note Attachment Folder','C:\\Program Files\\Deerfield.com\\VisNetic MailFlow\\NoteAttach') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add NoteAttachments
	if(g_showResults != 0)
	{
		_tcout << _T("Create NoteAttachments table") << endl;
	}

	g_logFile.Write(_T("Create NoteAttachments table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[NoteAttachments]( ")
	_T("[NoteAttachmentID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[AttachmentID] [int] NOT NULL CONSTRAINT [DF_NoteAttachments_AttachmentID]  DEFAULT ((0)), ")
	_T("[NoteID] [int] NOT NULL CONSTRAINT [DF_NoteAttachments_NoteID]  DEFAULT ((0)), ")
	_T("[NoteTypeID] [int] NOT NULL CONSTRAINT [DF_NoteAttachments_NoteTypeID]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_NoteAttachments] PRIMARY KEY CLUSTERED ([NoteAttachmentID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter Archives
	if(g_showResults != 0)
	{
		_tcout << _T("Add column to Archives table") << endl;
	}

	g_logFile.Write(_T("Add column to Archives table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Archives ADD NoteAttRecords INT DEFAULT 0 NOT NULL"));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;

}