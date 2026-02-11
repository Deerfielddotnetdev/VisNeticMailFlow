#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V5100(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	
	// Update AgentActivityLog
	if(g_showResults != 0)
	{
		_tcout << _T("Update AgentActivityLog table ActivityID 15 to 20") << endl;
	}

	g_logFile.Write(_T("Update AgentActivityLog table ActivityID 15 to 20"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE AgentActivityLog SET ActivityID=20 WHERE ActivityID=15"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	
	
	if(g_showResults != 0)
	{
		_tcout << _T("Update AgentActivityLog table ActivityID 16 to 21") << endl;
	}

	g_logFile.Write(_T("Update AgentActivityLog table ActivityID 16 to 21"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE AgentActivityLog SET ActivityID=21 WHERE ActivityID=16"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Update AgentActivityLog table ActivityID 17 to 22") << endl;
	}

	g_logFile.Write(_T("Update AgentActivityLog table ActivityID 17 to 22"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE AgentActivityLog SET ActivityID=22 WHERE ActivityID=17"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Alter TicketBoxes
	if(g_showResults != 0)
	{
		_tcout << _T("Add column UnreadMode to TicketBoxes table") << endl;
	}

	g_logFile.Write(_T("Add column UnreadMode to TicketBoxes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxes ADD UnreadMode INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Add column FromFormat to TicketBoxes table") << endl;
	}

	g_logFile.Write(_T("Add column FromFormat to TicketBoxes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxes ADD FromFormat INT DEFAULT 0 NOT NULL"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update newly added columns with value from MaxRecordSize
	if(g_showResults != 0)
	{
		_tcout << _T("Update TicketBoxes UnreadMode") << endl;
	}

	g_logFile.Write(_T("Update TicketBoxes UnreadMode"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE TicketBoxes SET UnreadMode = (CASE WHEN MaxRecordsSize IN (0,10,20) THEN 0 WHEN MaxRecordsSize IN (1,11,21) THEN 1 END)"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Update TicketBoxes FromFormat") << endl;
	}

	g_logFile.Write(_T("Update TicketBoxes FromFormat"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE TicketBoxes SET FromFormat = (CASE WHEN MaxRecordsSize IN (0,1) THEN 0 WHEN MaxRecordsSize IN (10,11) THEN 1 WHEN MaxRecordsSize IN (20,21) THEN 2 END)"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Drop TicketFieldsTicket IX_TicketID if exists then recreate
	if(g_showResults != 0)
	{
		_tcout << _T("Drop Index IX_TicketID from TicketFieldsTicket Table") << endl;
	}

	g_logFile.Write(_T("Drop Index IX_TicketID from TicketFieldsTicket Table"));

	retSQL = dbConn.ExecuteSQL(_T("IF EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[TicketFieldsTicket]') AND name = N'IX_TicketID') DROP INDEX [IX_TicketID] ON [TicketFieldsTicket] WITH ( ONLINE = OFF )"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(g_showResults != 0)
		{
			_tcout << _T("Error occurred during Drop Index IX_TicketID from TicketFieldsTicket Table") << endl;
		}

		g_logFile.Write(_T("Error occurred during Drop Index IX_TicketID from TicketFieldsTicket Table"));
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Create Index IX_TicketID in TicketFieldsTicket Table") << endl;
	}

	g_logFile.Write(_T("Create Index IX_TicketID in TicketFieldsTicket Table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE INDEX IX_TicketID on TicketFieldsTicket(TicketID)"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	// Alter Agents Table
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add DefaultTicketBoxID column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add DefaultTicketBoxID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD DefaultTicketBoxID INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add DefaultTicketDblClick column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add DefaultTicketDblClick column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD DefaultTicketDblClick INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add ReadReceipt column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add ReadReceipt column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD ReadReceipt INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add DeliveryConfirmation column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add DeliveryConfirmation column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD DeliveryConfirmation INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Row to AgentActivities Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Saved Message Row to AgentActivities table") << endl;
	}

	g_logFile.Write(_T("Adding Saved Message Row to AgentActivities table"));

	// Adding row to AgentActivities table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT AgentActivities ON ")
							   _T("INSERT INTO AgentActivities (AgentActivityID,Description) VALUES (25,'Saved Message') ")							   
							   _T("SET IDENTITY_INSERT AgentActivities OFF"));
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
	
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT TicketActions ON INSERT INTO TicketActions (TicketActionID,Description) VALUES (15,'Revoked message') INSERT INTO TicketActions (TicketActionID,Description) VALUES (16,'Released message') INSERT INTO TicketActions (TicketActionID,Description) VALUES (17,'Returned message') INSERT INTO TicketActions (TicketActionID,Description) VALUES (18,'Ticket merged') INSERT INTO TicketActions (TicketActionID,Description) VALUES (19,'Saved message') SET IDENTITY_INSERT TicketActions OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter OutboundMessages Table
	if(g_showResults != 0)
	{
		_tcout << _T("Add ReadReceipt column to OutboundMessages table") << endl;
	}

	g_logFile.Write(_T("Add ReadReceipt column to OutboundMessages table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE OutboundMessages ADD ReadReceipt tinyint default '0' not null") );
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Add DeliveryConfirmation column to OutboundMessages table") << endl;
	}

	g_logFile.Write(_T("Add DeliveryConfirmation column to OutboundMessages table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE OutboundMessages ADD DeliveryConfirmation tinyint default '0' not null") );
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add columns to InboundMessages
	if(g_showResults != 0)
	{
		_tcout << _T("Add ReadReceipt column to InboundMessages table") << endl;
	}

	g_logFile.Write(_T("Add ReadReceipt column to InboundMessages table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE InboundMessages ADD ReadReceipt tinyint default '0' not null") );
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Add ReadReceiptTo column to InboundMessages table") << endl;
	}

	g_logFile.Write(_T("Add ReadReceiptTo column to InboundMessages table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE InboundMessages ADD ReadReceiptTo VARCHAR(255) DEFAULT ('') NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Add new rows to ObjectTypes") << endl;
	}

	g_logFile.Write(_T("Add new rows to ObjectTypes"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ObjectTypes ON ")
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (12,'Agent Preferences') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (13,'Preferences General') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (14,'Preferences TicketBoxes') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (15,'Preferences Agent Views') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (16,'Preferences Signatures') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (17,'Preferences My Dictionary') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (18,'Preferences My Folders') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (19,'Preferences My Alerts') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (20,'Preferences My Contacts') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (21,'Route Replies to Me') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (22,'Close Ticket after Sending') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (23,'Request Read Receipt') ")							   
							   _T("INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (24,'Request Delivery Confirmation') ")							   
							   _T("SET IDENTITY_INSERT ObjectTypes OFF"));

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

	//Update Objects table	
	
	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to Objects") << endl;
	}

	g_logFile.Write(_T("Add rows to Objects"));

	wsprintf(buf, _T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,12,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,13,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;

	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,14,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,15,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,16,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,17,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,18,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,19,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,20,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,21,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,22,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,23,1,0,GetDate())"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,0,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	newObjectID++;
	maxObjectID++;
	
	wsprintf(buf, _T("INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,0,24,1,0,GetDate()) SET IDENTITY_INSERT Objects OFF"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	wsprintf(buf, _T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,0,%d)"), newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	maxObjectID++;

	//Add Rows to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Rows to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Rows to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (94,'Default Close Ticket after Send','2') ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (95,'Default Route Replies to Me','2') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;
}