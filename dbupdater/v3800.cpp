#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V3800(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	// Alter TicketNotes
	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to TicketNotes table") << endl;
	}

	g_logFile.Write(_T("Add columns to TicketNotes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketNotes ADD IsVoipNote BIT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE TicketNotes ADD StartTime DATETIME ")
							   _T("ALTER TABLE TicketNotes ADD StopTime DATETIME ")
							   _T("ALTER TABLE TicketNotes ADD ElapsedTime VARCHAR(50) DEFAULT '' NOT NULL ")
							   _T("ALTER TABLE TicketNotes ADD ContactID INT DEFAULT 0 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Alter ContactNotes
	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to ContactNotes table") << endl;
	}

	g_logFile.Write(_T("Add columns to ContactNotes table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE ContactNotes ADD IsVoipNote BIT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE ContactNotes ADD StartTime DATETIME ")
							   _T("ALTER TABLE ContactNotes ADD StopTime DATETIME ")
							   _T("ALTER TABLE ContactNotes ADD ElapsedTime VARCHAR(50) DEFAULT '' NOT NULL"));							   
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Add VOIP Integration Row to Server Parameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding VOIP Integration Row to Server Parameter table") << endl;
	}

	g_logFile.Write(_T("Adding VOIP Integration Row to Server Parameter table"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (68,'VOIP Integration','0') ")
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add VOIP Tables

	// Add VoipDialingCodes
	if(g_showResults != 0)
	{
		_tcout << _T("Create VoipDialingCodes table") << endl;
	}

	g_logFile.Write(_T("Create VoipDialingCodes table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[VoipDialingCodes]( ")
	_T("[VoipDialingCodeID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[VoipServerID] [int] NOT NULL CONSTRAINT [DF_VoipDialingCodes_VoipServerID]  DEFAULT ((0)), ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_VoipDialingCodes_Description]  DEFAULT (''), ")
	_T("[DialingCode] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_VoipDialingCodes_DialingCode]  DEFAULT (''), ")
	_T("[IsEnabled] [bit] NOT NULL CONSTRAINT [DF_VoipDialingCodes_IsEnabled]  DEFAULT ((1)), ")
	_T("[IsDefault] [bit] NOT NULL CONSTRAINT [DF_VoipDialingCodes_IsDefault]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_VoipDialingCodes] PRIMARY KEY CLUSTERED ([VoipDialingCodeID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add VoipExtensions
	if(g_showResults != 0)
	{
		_tcout << _T("Create VoipExtensions table") << endl;
	}

	g_logFile.Write(_T("Create VoipExtensions table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[VoipExtensions]( ")
	_T("[VoipExtensionID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[AgentID] [int] NOT NULL CONSTRAINT [DF_VoipExtensions_AgentID]  DEFAULT ((0)), ")
	_T("[VoipServerID] [int] NOT NULL CONSTRAINT [DF_VoipExtensions_VoipServerID]  DEFAULT ((0)), ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_VoipExtensions_Description]  DEFAULT (''), ")
	_T("[Extension] [int] NOT NULL CONSTRAINT [DF_VoipExtensions_Extension]  DEFAULT ((0)), ")
	_T("[Pin] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_VoipExtensions_Pin]  DEFAULT (''), ")
	_T("[IsEnabled] [bit] NOT NULL CONSTRAINT [DF_VoipExtensions_IsEnabled]  DEFAULT ((1)), ")
	_T("[IsDefault] [bit] NOT NULL CONSTRAINT [DF_VoipExtensions_IsDefault]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_VoipExtensions] PRIMARY KEY CLUSTERED ([VoipExtensionID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add VoipServers
	if(g_showResults != 0)
	{
		_tcout << _T("Create VoipServers table") << endl;
	}

	g_logFile.Write(_T("Create VoipServers table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[VoipServers]( ")
	_T("[VoipServerID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[VoipServerTypeID] [int] NOT NULL CONSTRAINT [DF_VoipServers_VoipServerTypeID]  DEFAULT ((0)), ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_VoipServers_Description]  DEFAULT (''), ")
	_T("[Url] [varchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_VoipServers_Url]  DEFAULT (''), ")
	_T("[IsEnabled] [bit] NOT NULL CONSTRAINT [DF_VoipServers_IsEnabled]  DEFAULT ((1)), ")
	_T("[IsDefault] [bit] NOT NULL CONSTRAINT [DF_VoipServers_IsDefault]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_VoipServers] PRIMARY KEY CLUSTERED ([VoipServerID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add VoipServerTypes
	if(g_showResults != 0)
	{
		_tcout << _T("Create VoipServerTypes table") << endl;
	}

	g_logFile.Write(_T("Create VoipServerTypes table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[VoipServerTypes]( ")
	_T("[VoipServerTypeID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_VoipServerTypes_Description]  DEFAULT (''), ")
	_T("CONSTRAINT [PK_VoipServerTypes] PRIMARY KEY CLUSTERED ([VoipServerTypeID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	if(g_showResults != 0)
	{
		_tcout << _T("Add row to VoipServerTypes") << endl;
	}

	g_logFile.Write(_T("Add row to VoipServerTypes"));

	retSQL = dbConn.ExecuteSQL(_T("INSERT INTO [VoipServerTypes]([Description]) VALUES ('3CX PBX')"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update ObjectTypes table
	//Add VOIP to ObjectTypes as ID 11.
	
	if(g_showResults != 0)
	{
		_tcout << _T("Add VOIP row to ObjectTypes") << endl;
	}

	g_logFile.Write(_T("Add VOIP row to ObjectTypes"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ObjectTypes ON INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (11,'VOIP') SET IDENTITY_INSERT ObjectTypes OFF"));
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
			maxObjectID = newObjectID;
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
		maxObjectID = newObjectID;
	}
	
	//Update Objects table
	//Add VOIP to Objects.
	
	if(g_showResults != 0)
	{
		_tcout << _T("Add VOIP row to Objects") << endl;
	}

	g_logFile.Write(_T("Add VOIP row to Objects"));

	wsprintf(buf,_T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights) VALUES (%d,0,11,1,0) SET IDENTITY_INSERT Objects OFF"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Update AccessControl table
	//Add Entry to AccessControl with GroupID=1, AccessLevel=0 and ObjectID = VOIP ID from Objects Table.

	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to AccessControl") << endl;
	}

	g_logFile.Write(_T("Add rows to AccessControl"));
	wsprintf(buf,_T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,0,%d)"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	return 0;

}