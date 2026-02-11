#include "prehead.h"
#include "main.h"

int VTwoBuildThree(CDBConn& dbConn,reg::Key& rkMailFlow)
{
	SQLRETURN	retSQL = 0;
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter the Agents table") << endl;
	}

	g_logFile.Write(_T("Alter the Agents table"));

	// Alter the Agents table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE InboundMessages ADD OriginalOwnerID int NOT NULL CONSTRAINT DF_InboundMessagesOriginalOwnerID DEFAULT 0"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter the InboundMessages table") << endl;
	}

	g_logFile.Write(_T("Alter the InboundMessages table"));

	// Alter the InboundMessages table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE InboundMessages ADD OriginalTicketBoxID int NOT NULL CONSTRAINT DF_InboundMessagesOriginalTicketBoxID  DEFAULT 0 CREATE INDEX IX_Date on InboundMessages(DateReceived, IsDeleted) CREATE INDEX IX_OriginalTicketBoxID on InboundMessages(OriginalTicketBoxID) CREATE INDEX IX_OriginalOwnerID on InboundMessages(OriginalOwnerID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	if(!IsTransUp(dbConn,rkMailFlow))
	{
		// Begin transaction
		retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
		if(!SQL_SUCCEEDED(retSQL))
		{
			OutputDebugString(_T("Failed to begin transaction \n"));
			if(g_showResults != 0)
			{
				_tcout << endl << dbConn.GetErrorString() << endl;
			}

			g_logFile.Write(dbConn.GetErrorString());

			dbConn.Disconnect();
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 4;
		}
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter the OutboundMessages table") << endl;
	}

	g_logFile.Write(_T("Alter the OutboundMessages table"));

	// Alter the InboundMessages table
	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE OutboundMessages ADD OriginalTicketBoxID int NOT NULL CONSTRAINT DF_OutboundMessagesOriginalTicketBoxID  DEFAULT 0 CREATE INDEX IX_Date on OutboundMessages(EmailDateTime, IsDeleted) CREATE INDEX IX_OriginalTicketBoxID on OutboundMessages(OriginalTicketBoxID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	if(!IsTransUp(dbConn,rkMailFlow))
	{
		// Begin transaction
		retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
		if(!SQL_SUCCEEDED(retSQL))
		{
			OutputDebugString(_T("Failed to begin transaction \n"));
			if(g_showResults != 0)
			{
				_tcout << endl << dbConn.GetErrorString() << endl;
			}

			g_logFile.Write(dbConn.GetErrorString());
			
			dbConn.Disconnect();
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 4;
		}
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Update AlertEvents table") << endl;
	}

	g_logFile.Write(_T("Update AlertEvents table"));

	//Update AlertEvents table
	retSQL = dbConn.ExecuteSQL(_T("UPDATE AlertEvents SET Description='Disk Full' where AlertEventID=5"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Update OutboundMessages table") << endl;
	}

	g_logFile.Write(_T("Update OutboundMessages table"));

	//Update OutboundMessages table
	retSQL = dbConn.ExecuteSQL(_T("UPDATE OutboundMessages SET OriginalTicketBoxID = (SELECT TicketBoxID FROM Tickets WHERE TicketID = OutboundMessages.TicketID ) WHERE EXISTS (SELECT TicketID FROM Tickets WHERE TicketID = OutboundMessages.TicketID )"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Update InboundMessages table") << endl;
	}

	g_logFile.Write(_T("Update InboundMessages table"));

	//Update InboundMessages table
	retSQL = dbConn.ExecuteSQL(_T("UPDATE InboundMessages SET OriginalTicketBoxID = (SELECT TicketBoxID FROM Tickets WHERE TicketID = InboundMessages.TicketID ), OriginalOwnerID = (SELECT OwnerID FROM Tickets WHERE TicketID = InboundMessages.TicketID ) WHERE EXISTS (SELECT TicketID FROM Tickets WHERE TicketID = InboundMessages.TicketID )"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	
	if(g_showResults != 0)
	{
		_tcout << _T("Create index in TicketHistory table") << endl;
	}

	g_logFile.Write(_T("Create index in TicketHistory table"));

	//"Create index in TicketHistory table"
	retSQL = dbConn.ExecuteSQL(_T("CREATE INDEX IX_TicketID on TicketHistory(TicketID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	if(!IsTransUp(dbConn,rkMailFlow))
	{
		// Begin transaction
		retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
		if(!SQL_SUCCEEDED(retSQL))
		{
			OutputDebugString(_T("Failed to begin transaction \n"));
			if(g_showResults != 0)
			{
				_tcout << endl << dbConn.GetErrorString() << endl;
			}

			g_logFile.Write(dbConn.GetErrorString());
			
			dbConn.Disconnect();
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 4;
		}
	}

	return 0;
}