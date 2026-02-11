//*****************************************************************************
//
// dbUpdate.exe - (main.cpp)
// Utility to update VisNetic MailFlow Database
// Created on : 6/13/2002
//
//*****************************************************************************

//*****************************************************************************

// Include headers
#include "prehead.h"
#include "main.h"

//*****************************************************************************

//*****************************************************************************

// Declarations
typedef TCHAR String[256];
typedef const TCHAR* PCTSTR;

BOOL      bIsTrans = FALSE;
BOOL      bIsDBConn = FALSE;
LogFile   g_logFile;
int	      g_showResults = 0;
HINSTANCE g_hInst = 0;
int       maxObjectID = 0;
//*****************************************************************************

//*****************************************************************************

// Function Declarations
void Banner();
int TestVersion(int h,int l,int i,int b,const TCHAR* ver);
BOOL IsOldVersion(TCHAR* ver);

//*****************************************************************************


//*****************************************************************************

// Main function
int _tmain(int argc,TCHAR* argv[])
{
	try
	{

		// Variables
		int			retVal;
		int			g_exportXML = 0;
		int			g_doInstall = 0;
		int			g_doFullText = 0;
		int			g_doRestoreTicket = 0;
		int			g_doSearchArchive = 0;
		int			g_doConvertData = 0;
		int			nTicketID = 0;
		reg::Key	rkMailFlow;
		reg::Value	rvDBVersion;
		reg::Value  rvDSN;
		CDBConn		dbConn;
		String		dbVersion;
		String		dsn;
		SQLRETURN	retSQL;
		TCHAR		wDsn[256];
		String      sLogBuf;
		
		g_hInst = ::GetModuleHandle(NULL);
	
		ResString sDbVer(g_hInst,IDS_UPDATE_DBVER);

		g_logFile.Line();
		g_logFile.Write(_T("VisNetic MailFlow Database Update Log v6.9.5.0"));
		g_logFile.Date();		

		if(argc <= 1)
		{
			Banner();
			g_showResults = 1;
		}
		else if(argc > 2)
		{
			Banner();
			_tcout << endl << _T("This program only takes one argument. Type '-h' for help.") << endl;
			g_logFile.Write(_T("This program only takes one argument. Type '-h' for help."));

			g_logFile.LineBreak();
			g_logFile.Line();
			return 1;
		}
		else
		{
			if(!_tcscmp(_T("-s"),argv[1]))
			{
				g_showResults = 0;
			}
			else if(!_tcscmp(_T("-h"),argv[1]))
			{
				Banner();
				_tcout << endl << _T("Valid Command Line parameters:") << endl;
				g_logFile.Write(_T("Valid Command Line parameters:"));

				/*_tcout << _T("Switch '-a' : Search an Archive.") << endl;
				g_logFile.Write(_T("Switch '-a' : Search an Archive."));*/
				
				_tcout << _T("Switch '-c' : Convert data to UTF-8.") << endl;
				g_logFile.Write(_T("Switch '-c' : Convert data to UTF-8."));
				
				_tcout << _T("Switch '-e' : Exports database structure to XML.") << endl;
				g_logFile.Write(_T("Switch '-e' : Exports database structure to XML."));
				
				_tcout << _T("Switch '-f' : Sets up FullText search.") << endl;
				g_logFile.Write(_T("Switch '-f' : Sets up full text Ticket Search."));

				_tcout << _T("Switch '-h' : Display's help.") << endl;
				g_logFile.Write(_T("Switch '-h' : Display's help."));

				_tcout << _T("Switch '-i' : Sets up new install getdate() defaults.") << endl;
				g_logFile.Write(_T("Switch '-i' : Sets up new install getdate() defaults."));

				_tcout << _T("Switch '-r' : Restore a Ticket from Archive.") << endl;
				g_logFile.Write(_T("Switch '-r' : Restore a Ticket from Archive."));

				_tcout << _T("Switch '-s' : Silent update. Does not display results to the screen.") << endl;
				g_logFile.Write(_T("Switch '-s' : Silent update. Does not display results to the screen."));

				g_logFile.LineBreak();
				g_logFile.Line();
				return 1;
			}
			else if(!_tcscmp(_T("-e"),argv[1]))
			{
				Banner();
				_tcout << endl << _T("Export database structure to XML") << endl;
				g_logFile.Write(_T("Export database structure to XML"));

				g_logFile.LineBreak();
				g_logFile.Line();

				g_exportXML = 1;
				g_showResults = 1;
			}
			else if(!_tcscmp(_T("-i"),argv[1]))
			{
				Banner();
				_tcout << endl << _T("Setup new install getdate() defaults") << endl;
				g_logFile.Write(_T("Setup new install getdate() defaults"));

				g_logFile.LineBreak();
				g_logFile.Line();

				g_doInstall = 1;
				g_showResults = 1;
			}
			else if(!_tcscmp(_T("-f"),argv[1]))
			{
				Banner();
				_tcout << endl << _T("Setup FullText Search") << endl;
				g_logFile.Write(_T("Setup FullText Search"));

				g_logFile.LineBreak();
				g_logFile.Line();

				g_doFullText = 1;
				g_showResults = 1;
			}
			else if(!_tcscmp(_T("-r"),argv[1]))
			{
				Banner();
				_tcout << endl << _T("Restore a Ticket from Archive") << endl;
				g_logFile.Write(_T("Restore a Ticket from Archive"));

				g_logFile.LineBreak();
				g_logFile.Line();

				_tcout << endl << _T("Enter the TicketID to Restore:");
				cin >> nTicketID;

				wsprintf(sLogBuf,_T("You entered TicketID: %d"),nTicketID);

				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);

				if(nTicketID < 1)
				{
					_tcout << endl << _T("The TicketID must be greater than zero (0):") << endl;
					cin >> nTicketID;
					wsprintf(sLogBuf,_T("You entered TicketID: %d"),nTicketID);
					_tcout << endl << sLogBuf << endl;
					g_logFile.Write(sLogBuf);
					if(nTicketID < 1)
					{
						_tcout << endl << _T("TicketID was less than one (1)") << endl;
						g_logFile.Write(_T("TicketID was less than one (1)"));
						return 1;
					}
				}
				g_doRestoreTicket = 1;
				g_showResults = 1;
			}
			/*else if(!_tcscmp(_T("-a"),argv[1]))
			{
				Banner();
				_tcout << endl << _T("Search an Archive") << endl;
				g_logFile.Write(_T("Search an Archive"));

				g_logFile.LineBreak();
				g_logFile.Line();
				
				g_doSearchArchive = 1;
				g_showResults = 1;
			}*/
			else if(!_tcscmp(_T("-c"),argv[1]))
			{
				Banner();
				_tcout << endl << _T("Convert data to UTF-8") << endl;
				g_logFile.Write(_T("Convert data to UTF-8"));

				g_logFile.LineBreak();
				g_logFile.Line();
				
				g_doConvertData = 1;
				g_showResults = 1;
			}
			else
			{
				Banner();
				_tcout << endl << _T("Invalid argument. Type '-h' for help.") << endl;
				g_logFile.Write(_T("Invalid argument. Type '-h' for help."));
				g_logFile.LineBreak();
				g_logFile.Line();
				return 1; 
			}
		}
		
		// Connect to registry key
		if(g_showResults != 0)
		{
			_tcout << endl << _T("Connecting to the MailFlow Registry") << endl;		
		}		
		g_logFile.Write(_T("Connecting to the MailFlow Registry"));
		retVal = rkMailFlow.Open(HKEY_LOCAL_MACHINE,_T("Software\\Deerfield.com\\VisNetic MailFlow"));
		if(!retVal)
		{
			_tcout << endl << _T("Failed to access registry key VisNetic MailFlow") << endl;
			g_logFile.Write(_T("Failed to access registry key VisNetic MailFlow"));
			g_logFile.LineBreak();
			g_logFile.Line();
			return 2;
		}

		// Get DB Version
		if(g_showResults != 0)
		{
			_tcout << endl << _T("Getting current DB version from the MailFlow Registry") << endl;		
		}
		g_logFile.Write(_T("Getting current DB version from the MailFlow Registry"));
		retVal = rvDBVersion.GetString(rkMailFlow,_T("dbVersion"),dbVersion);
		if(retVal != 0)
		{
			_tcout << endl << _T("Failed to access registry value dbVersion") << endl;
			g_logFile.Write(_T("Failed to access registry value dbVersion"));
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 2;
		}

		// Display results
		wsprintf(sLogBuf,_T("Found Database version : %s"),dbVersion);

		if(g_showResults != 0)
		{
			_tcout << endl << sLogBuf << endl;
		}

		g_logFile.Write(sLogBuf);

		// Is Pre 2.0 version
		if(!IsOldVersion(dbVersion))
		{
			_tcscat(dbVersion,_T(".0"));
		}

		CODBCQuery	m_query;
		TCHAR sData[256];

		// Get DSN value from registry
		_tcout << endl << _T("Getting DB connection string from the MailFlow Registry") << endl;
		g_logFile.Write(_T("Getting DB connection string from the MailFlow Registry"));
		
		retVal = rvDSN.GetString(rkMailFlow,_T("dbLogin"),dsn);
		if(retVal != 0)
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Failed to access registry value dbLogin") << endl;
			}

			g_logFile.Write(_T("Failed to access registry value dbLogin"));
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 2;
		}

		int tv = TestVersion(2,0,0,0,dbVersion);
		if(tv <= 0)
		{
			dca::EString eDSN(dsn);
			eDSN.Decrypt();
			_tcscpy(dsn, eDSN.c_str());
		}
		
		if(g_doRestoreTicket == 0 && g_doSearchArchive == 0 && g_doConvertData == 0)
		{
			wsprintf(sLogBuf,_T("Found DSN : %s"),dsn);
			if(g_showResults != 0)
			{
				_tcout << endl << sLogBuf << endl;
			}
		}
		
		// Connect to Database
		_tcout << endl << _T("Attempting to connect to the MailFlow DB") << endl;
		g_logFile.Write(_T("Attempting to connect to the MailFlow DB"));
		
		lstrcpy(wDsn,dsn);
		retSQL = dbConn.Connect(wDsn);
		if(!SQL_SUCCEEDED(retSQL))
		{
			dca::EString EStr(dsn);
			EStr.Decrypt();
			_tcscpy(dsn,EStr.c_str());

			_tcout << endl << _T("First DB connection attempt failed... trying again") << endl;
			g_logFile.Write(_T("First DB connection attempt failed... trying again"));
		
			lstrcpy(wDsn,dsn);
			retSQL = dbConn.Connect(wDsn);
			if(!SQL_SUCCEEDED(retSQL))
			{
				if(g_showResults != 0)
				{
					_tcout << endl << dbConn.GetErrorString() << endl;
				}

				g_logFile.Write(dbConn.GetErrorString());

				rkMailFlow.Close();
				g_logFile.LineBreak();
				g_logFile.Line();
				return 3;
			}
			else
			{
				bIsDBConn = TRUE;
				_tcout << endl << _T("Successfully connected to the MailFlow DB") << endl;
				g_logFile.Write(_T("Successfully connected to the MailFlow DB"));
			}
		}
		else
		{
			bIsDBConn = TRUE;
			_tcout << endl << _T("Successfully connected to the MailFlow DB") << endl;
			g_logFile.Write(_T("Successfully connected to the MailFlow DB"));
		}
		
		//Get dbVersion from the MailFlow DB
		_tcout << endl << _T("Getting DB Version from the MailFlow DB") << endl;
		g_logFile.Write(_T("Getting DB Version from the MailFlow DB"));

		TCHAR szDataValue[SERVERPARAMETERS_DATAVALUE_LENGTH];
		long szDataValueLen;
		szDataValue[0] = _T('\0');
		m_query.Initialize();
		BINDCOL_TCHAR( m_query, szDataValue );
		m_query.Execute( _T("SELECT DataValue FROM ServerParameters WHERE ServerParameterID=24"));
		m_query.Fetch();

		// Display results
		wsprintf(sLogBuf,_T("Found DB Version : %s in the MailFlow DB"),szDataValue);
		if(g_showResults != 0)
		{
			_tcout << endl << sLogBuf << endl;
		}
		g_logFile.Write(sLogBuf);

		_tcout << endl << _T("Setting Registry dbVersion to Database DB Version") << endl;
		g_logFile.Write(_T("Setting Registry dbVersion to Database DB Version"));

		wsprintf(dbVersion,_T("%s"),szDataValue);
		retVal = rvDBVersion.SetString(rkMailFlow,_T("dbVersion"),szDataValue);
		if(retVal != 0)
		{
			_tcout << endl << _T("Failed to update registry value dbVersion") << endl;
			g_logFile.Write(_T("Failed to update registry value dbVersion"));
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 2;
		}

		// Test dbVersion
		if(g_doInstall == 0 && g_exportXML == 0 && g_doFullText == 0 && g_doRestoreTicket == 0 && g_doSearchArchive == 0 && g_doConvertData == 0)
		{
			retVal = TestVersion(6,9,4,5,dbVersion);
			if(retVal < 0)
			{
				try
				{
					ResString err(g_hInst,IDS_101);
					
					wsprintf(sLogBuf,_T("Failure - %s"),(PCTSTR)err);

					if(g_showResults != 0)
					{
						_tcout << endl << sLogBuf << endl;
					}

					g_logFile.Write(sLogBuf);
				}
				catch(const TCHAR* except)
				{
					if(g_showResults != 0)
					{
						_tcout << except << endl;
					}

					g_logFile.Write(except);
				}

				rkMailFlow.Close();
				g_logFile.LineBreak();
				g_logFile.Line();
				return 101;
			}
			else if(retVal > 0)
			{
				try
				{
					ResString err(g_hInst,IDS_200);

					if(g_showResults != 0)
					{
						_tcout << endl << (PCTSTR)err << endl;
					}

					g_logFile.Write((PCTSTR)err);
				}
				catch(const TCHAR* except)
				{
					if(g_showResults != 0)
					{
						_tcout << except << endl;
					}

					g_logFile.Write(except);

					return 1;
				}
			}
			else if(retVal == 0)
			{
				try
				{
					ResString err(g_hInst,IDS_100);

					wsprintf(sLogBuf,_T("Failure - %s"),(PCTSTR)err);

					if(g_showResults != 0)
					{
						_tcout << endl << sLogBuf << endl;
					}

					g_logFile.Write(sLogBuf);
				}
				catch(const TCHAR* except)
				{
					if(g_showResults != 0)
					{
						_tcout << except << endl;
					}

					g_logFile.Write(except);
				}

				rkMailFlow.Close();
				g_logFile.LineBreak();
				g_logFile.Line();
				return 100;
			}
		}

		if(g_doInstall == 1)
		{
			int rc = VInstall(m_query,dbConn,rkMailFlow,wDsn);
			return rc;
		}
		
		if(g_doFullText == 1)
		{
			int rc = VFullText(m_query,dbConn,rkMailFlow,wDsn);
			return rc;
		}
		
		if(g_exportXML == 1)
		{
			int rc = VExportXML(m_query,dbConn,rkMailFlow,wDsn);
			return 1;
		}
		
		if(g_doRestoreTicket == 1)
		{
			int rc = RestoreTicket(m_query,dbConn,rkMailFlow,wDsn,nTicketID);
			return rc;
		}

		/*if(g_doSearchArchive == 1)
		{
			int rc = SearchArchive(m_query,dbConn,rkMailFlow,wDsn);
			return rc;
		}*/

		if(g_doConvertData == 1)
		{
			int retVal = TestVersion(6,5,0,0,dbVersion);
			if(retVal == 1)
			{
				retVal = ConvertData(m_query,dbConn,rkMailFlow,wDsn);				
			}
			else
			{
				_tcout << endl << _T("Data can only be converted if database version is less than 6.5.0.0.") << endl;
				g_logFile.Write(_T("Data can only be converted if database version is less than 6.5.0.0"));
				retVal = 0;
			}
			return retVal;
		}

		if(g_showResults != 0)
		{
			_tcout << endl << _T("Beginning SQL script") << endl;
		}

		g_logFile.Write(_T("Beginning SQL script"));

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
		else
		{
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			bIsTrans = TRUE;
		}

		//*************************************************************************
		// 
		// Version 1.0.0 to 1.0.1
		//
		//*************************************************************************
		
		//if(!strcmp("1.0.0",dbVersion))
		if(!TestVersion(1,0,0,0,dbVersion))
		{
			// Must be version 1.0.0
			
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.0.1.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.0.1.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Modifying AlertMethods Table") << endl;
			}

			g_logFile.Write(_T("Modifying AlertMethods Table"));

			// Update External Email to Temp1
			retSQL = dbConn.ExecuteSQL(_T("update AlertMethods set Description = 'Temp1' where Description = 'External Email'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Change Internal Email to External Email
			retSQL = dbConn.ExecuteSQL(_T("update AlertMethods set Description = 'External Email' where Description = 'Internal Email'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Change Instant Message to Internal Email
			retSQL = dbConn.ExecuteSQL(_T("update AlertMethods set Description = 'Internal Email' where Description = 'Instant Message'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Modifying Alerts Table") << endl;
			}

			g_logFile.Write(_T("Modifying Alerts Table"));

			// Change AlertMethodId from 2 to 1
			retSQL = dbConn.ExecuteSQL(_T("update Alerts set AlertMethodId = '1' where AlertMethodId = '2'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Change AlertMethodId to 3 to 2
			retSQL = dbConn.ExecuteSQL(_T("update Alerts set AlertMethodId = '2' where AlertMethodId = '3'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Renaming Inbox to My Tickets") << endl;
			}

			g_logFile.Write(_T("Renaming Inbox to My Tickets"));

			// Update Inbox name to My Tickets
			retSQL = dbConn.ExecuteSQL(_T("update TicketBoxViewTypes set Name = 'My Tickets' where Name = 'Inbox'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Renaming TicketPriorities to Priorities") << endl;
			}

			g_logFile.Write(_T("Renaming TicketPriorities to Priorities"));


			// Rename TicketPriorities to Priorities
			retSQL = dbConn.ExecuteSQL(_T("exec sp_rename 'TicketPriorities', 'Priorities'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add column PriorityID to OutboundMessages") << endl;
			}

			g_logFile.Write(_T("Add column PriorityID to OutboundMessages"));

			// Add column PriorityID to Outbound Message
			retSQL = dbConn.ExecuteSQL(_T("alter table OutboundMessages add PriorityID int default '3' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add column PriorityID to InboundMessages") << endl;
			}

			g_logFile.Write(_T("Add column PriorityID to InboundMessages"));

			// Add column PriorityID to InboundMessages
			retSQL = dbConn.ExecuteSQL(_T("alter table InboundMessages add PriorityID int default '3' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Altering the table Tickets foreign keys") << endl;
			}

			g_logFile.Write(_T("Altering the table Tickets foreign keys"));

			// drop constraint FK_TicketPriorities
			retSQL = dbConn.ExecuteSQL(_T("alter table Tickets drop constraint FK_TicketPriorities"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// add constraint FK_Priorities
			retSQL = dbConn.ExecuteSQL(_T("alter table Tickets add constraint FK_Priorities foreign key (PriorityID) references Priorities(PriorityID)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Altering the table Agents") << endl;
			}

			g_logFile.Write(_T("Altering the table Agents"));

			// Add RequireGetOldest column
			retSQL = dbConn.ExecuteSQL(_T("alter table Agents add RequireGetOldest bit default '0' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Add HasLoggedIn column
			retSQL = dbConn.ExecuteSQL(_T("alter table Agents add HasLoggedIn bit default '0' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add column WaterMarkStatus to TicketBoxes") << endl;
			}

			g_logFile.Write(_T("Add column WaterMarkStatus to TicketBoxes"));

			// Add column PriorityID to InboundMessages
			retSQL = dbConn.ExecuteSQL(_T("alter table TicketBoxes add WaterMarkStatus tinyint default '0' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Updating AlertMethods table") << endl;
			}

			g_logFile.Write(_T("Updating AlertMethods table"));

			// Deleted Temp1
			retSQL = dbConn.ExecuteSQL(_T("delete from AlertMethods where Description = 'Temp1'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("1.0.1.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.0.1.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 1.0.1 to 1.0.2
		//
		//*************************************************************************

		//if(!strcmp("1.0.1",dbVersion))
		if(!TestVersion(1,0,1,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.0.2.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.0.2.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Create Table Archives") << endl;
			}

			g_logFile.Write(_T("Create Table Archives"));

			// Create Table Archives
			TCHAR* crTable = _T("CREATE TABLE Archives ( ArchiveID integer IDENTITY(1,1) PRIMARY KEY, DateCreated datetime, Purged tinyint default(0) not null,ArcFilePath varchar(255) not null,InMsgRecords integer default(0) not null,InAttRecords integer default(0) not null,OutMsgRecords integer default (0) not null,OutAttRecords integer default (0) not null)");
			retSQL = dbConn.ExecuteSQL(crTable);
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add column ArchiveID to OutboundMessages") << endl;
			}

			g_logFile.Write(_T("Add column ArchiveID to OutboundMessages"));

			// Add column ArchiveID to Outbound Messages
			retSQL = dbConn.ExecuteSQL(_T("alter table OutboundMessages add ArchiveID integer default '0' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add column ArchiveID to InBoundMessages") << endl;
			}

			g_logFile.Write(_T("Add column ArchiveID to InBoundMessages"));

			// Add column ArchiveID to Inbound Messages
			retSQL = dbConn.ExecuteSQL(_T("alter table InboundMessages add ArchiveID integer default '0' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("1.0.2.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.0.2.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 1.0.2 to 1.0.3
		//
		//*************************************************************************

		//if(!strcmp("1.0.2",dbVersion))
		if(!TestVersion(1,0,2,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.0.3.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.0.3.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Create Table AlertMsgs") << endl;
			}

			g_logFile.Write(_T("Create Table AlertMsgs"));

			// Create Table AlertMsgs
			TCHAR* crTable = _T("CREATE TABLE AlertMsgs ( AlertMsgID integer IDENTITY(1,1) PRIMARY KEY, AlertEventID integer not null, AgentID integer not null, Viewed bit default(0) not null, DateCreated datetime, Body text not null)");
			retSQL = dbConn.ExecuteSQL(crTable);
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Changed description of Internal Email to Internal") << endl;
			}

			g_logFile.Write(_T("Changed description of Internal Email to Internal"));

			// Changed description of Internal Email to Internal
			retSQL = dbConn.ExecuteSQL(_T("update AlertMethods set Description = 'Internal' where Description = 'Internal Email'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add signatureID to OutboundMessages") << endl;
			}

			g_logFile.Write(_T("Add signatureID to OutboundMessages"));

			// Add signatureID to OutboundMessages
			retSQL = dbConn.ExecuteSQL(_T("alter table OutboundMessages add SignatureID integer default '0' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add rows to ServerParameters") << endl;
			}

			g_logFile.Write(_T("Add rows to ServerParameters"));

			// Add Database Version to ServerParameters
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values (24,'Database Version','1.0.3') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Add Force HTTPS to ServerParameters
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values (25,'Force HTTPS','0') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add row to TicketBoxViewTypes") << endl;
			}

			g_logFile.Write(_T("Add row to TicketBoxViewTypes"));

			// Add row to TicketBoxViewTypes
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT TicketBoxViewTypes ON insert into TicketBoxViewTypes (TicketBoxViewTypeID,Name) values (8,'Alerts') SET IDENTITY_INSERT TicketBoxViewTypes OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add Agents to TicketBoxViews") << endl;
			}

			g_logFile.Write(_T("Add Agents to TicketBoxViews"));

			TCHAR test[256];
			TCHAR buf[256];
			int num_rows = 0;
			
			// Add Agents to TicketBoxViews
			retSQL = dbConn.GetData(_T("select count(*) from agents"),test);
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
			num_rows = _ttoi(test);
			num_rows++;

			retSQL = dbConn.ExecuteSQL(_T("declare i_cursor cursor for select agentid from agents"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			retSQL = dbConn.ExecuteSQL(_T("open i_cursor"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			for(int i = 1; i < num_rows; i++)
			{
				dbConn.GetData(_T("fetch next from i_cursor"),test);
				wsprintf(buf,_T("insert into TicketBoxViews (AgentId,AgentBoxID,SortField,TicketBoxViewTypeID) values (%s,%s,1,8)"),test,test);
				dbConn.ExecuteSQL(buf);
			}

			retSQL = dbConn.ExecuteSQL(_T("close i_cursor"));
			retSQL = dbConn.ExecuteSQL(_T("deallocate i_cursor"));
		
			lstrcpy(dbVersion,_T("1.0.3.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.0.3.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

	
		//*************************************************************************
		// 
		// Version 1.0.3 to 1.0.4
		//
		//*************************************************************************

		//if(!strcmp("1.0.3",dbVersion))
		if(!TestVersion(1,0,3,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.0.4.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.0.4.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Update Database Version") << endl;
			}
			
			g_logFile.Write(_T("Update Database Version"));

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '1.0.4' where Description = 'Database Version'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("1.0.4.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.0.4.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 1.0.4 to 1.0.5
		//
		//*************************************************************************

		//if(!strcmp("1.0.4",dbVersion))
		if(!TestVersion(1,0,4,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.0.5.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.0.5.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Modify Server parameters") << endl;
			}

			g_logFile.Write(_T("Modify Server parameters"));

			// Modify Server parameters
			// Default message ID
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set Description = 'Default message destination' where ServerParameterId = 10"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// VisNetic AntiVirus
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set Description = 'VisNetic AntiVirus Scanning' where ServerParameterId = 12"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add new parameters to ServerParameters") << endl;
			}

			g_logFile.Write(_T("Add new parameters to ServerParameters"));

			// Add new parameters to Server
			// VMS Integration
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values (26,'VMS Integration','0') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// VMS WebMail URL
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values (27,'VMS WebMail URL','') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// VMS DB Connection String
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values (28,'VMS Database Connection String','') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
			
			// Max ISAPI threads
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values (29,'Maximum ISAPI extension threads','5') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
			
			// Last VMF reg alert
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values (30,'Last MailFlow registration alert','Never') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Last AntiVirus reg alert
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values (31,'Last AntiVirus registration alert','Never') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Change default email") << endl;
			}

			g_logFile.Write(_T("Change default email"));

			// Change default email
			retSQL = dbConn.ExecuteSQL(_T("update TicketBoxes set DefaultEmailAddress = 'unassigned@mailflow' where Name = 'Unassigned'"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Change builtin data types") << endl;
			}

			g_logFile.Write(_T("Change builtin data types"));
			
			// Change builtin data types
			retSQL = dbConn.ExecuteSQL(_T("update PersonalDataTypes set BuiltIn = 0 where PersonalDataTypeID <> 1"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add default signature ID to Agents") << endl;
			}

			g_logFile.Write(_T("Add default signature ID to Agents"));

			// Add default signature ID to Agents
			retSQL = dbConn.ExecuteSQL(_T("alter table Agents add DefaultSignatureID int default '0' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Update Database Version") << endl;
			}
			
			g_logFile.Write(_T("Update Database Version"));

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '1.0.5' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("1.0.5.0"));
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.0.5.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 1.0.5 to 1.0.6
		//
		//*************************************************************************

		//if(!strcmp("1.0.5",dbVersion))
		if(!TestVersion(1,0,5,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.0.6.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.0.6.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Add column to OutboundMessageAttachments") << endl;
			}

			g_logFile.Write(_T("Add column to OutboundMessageAttachments"));

			// Add column to OutBoundMessageAttachments
			retSQL = dbConn.ExecuteSQL(_T("alter table OutboundMessageAttachments add FileSize int default '0' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add column to StdResponseAttachments") << endl;
			}

			g_logFile.Write(_T("Add column to StdResponseAttachments"));
			
			// Add column to StdResponseAttachments
			retSQL = dbConn.ExecuteSQL(_T("alter table StdResponseAttachments add FileSize int default '0' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Add rows to Server Parameter") << endl;
			}

			g_logFile.Write(_T("Add rows to Server Parameter"));

			// Add rows to Server Parameter
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values (32,'Match agent address in default routing rule','1') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
			
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values (33,'Match TicketBox address in default routing rule','1') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Modify description it ObjectType table") << endl;
			}

			g_logFile.Write(_T("Modify description it ObjectType table"));

			// Modify description it ObjectType table
			retSQL = dbConn.ExecuteSQL(_T("update ObjectTypes set Description = 'TicketBox' where ObjectTypeID = 2"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Update Database Version") << endl;
			}
			
			g_logFile.Write(_T("Update Database Version"));

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '1.0.6' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("1.0.6.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.0.6.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 1.0.6 to 1.0.10
		//
		//*************************************************************************

		//if(!strcmp("1.0.6",dbVersion))
		if(!TestVersion(1,0,6,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.0.10.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.0.10.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Removed key constraint from MessageSource") << endl;
			}

			g_logFile.Write(_T("Removed key constraint from MessageSource"));
			
			// Modify description it ObjectType table
			retSQL = dbConn.ExecuteSQL(_T("alter table MessageSources drop constraint FK_MsgSourcesMsgDestinations"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Update Database Version") << endl;
			}
			
			g_logFile.Write(_T("Update Database Version"));

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '1.0.10' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("1.0.10.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.0.10.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 1.0.10 to 1.0.11
		//
		//*************************************************************************

		if(!TestVersion(1,0,10,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.0.11.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.0.11.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Change Low WaterMark in AlertEvents table") << endl;
			}

			g_logFile.Write(_T("Change Low WaterMark in AlertEvents table"));

			// Change Low WaterMark in AlertEvents table
			retSQL = dbConn.ExecuteSQL(_T("update AlertEvents set Description = 'Low WaterMark' where AlertEventID = 1"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Change High WaterMark in AlertEvents table") << endl;
			}

			g_logFile.Write(_T("Change High WaterMark in AlertEvents table"));

			// Change High WaterMark in AlertEvents table
			retSQL = dbConn.ExecuteSQL(_T("update AlertEvents set Description = 'High WaterMark' where AlertEventID = 2"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Change Ticket Age in AlertEvents table") << endl;
			}

			g_logFile.Write(_T("Change Ticket Age in AlertEvents table"));

			// Change Ticket Age in AlertEvents table
			retSQL = dbConn.ExecuteSQL(_T("update AlertEvents set Description = 'Ticket Age' where AlertEventID = 3"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Change Product Registration in AlertEvents table") << endl;
			}

			g_logFile.Write(_T("Change Product Registration in AlertEvents table"));

			// Change Product Registration in AlertEvents table
			retSQL = dbConn.ExecuteSQL(_T("update AlertEvents set Description = 'Product Registration' where AlertEventID = 4"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Change Low Disk Space in AlertEvents table") << endl;
			}

			g_logFile.Write(_T("Change Low Disk Space in AlertEvents table"));

			// Change Low Disk Space in AlertEvents table
			retSQL = dbConn.ExecuteSQL(_T("update AlertEvents set Description = 'Low Disk Space' where AlertEventID = 5"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Change Inbound Delivery in AlertEvents table") << endl;
			}

			g_logFile.Write(_T("Change Inbound Delivery in AlertEvents table"));

			// Change Inbound Delivery in AlertEvents table
			retSQL = dbConn.ExecuteSQL(_T("update AlertEvents set Description = 'Inbound Delivery' where AlertEventID = 6"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Change Outbound Delivery in AlertEvents table") << endl;
			}

			g_logFile.Write(_T("Change Outbound Delivery in AlertEvents table"));

			// Change Outbound Delivery in AlertEvents table
			retSQL = dbConn.ExecuteSQL(_T("update AlertEvents set Description = 'Outbound Delivery' where AlertEventID = 7"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Change Critical Error in AlertEvents table") << endl;
			}

			g_logFile.Write(_T("Change Critical Error in AlertEvents table"));

			// Change Critical Error in AlertEvents table
			retSQL = dbConn.ExecuteSQL(_T("update AlertEvents set Description = 'Critical Error' where AlertEventID = 8"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Change ISAPI extension in LogEntryTypes table") << endl;
			}

			g_logFile.Write(_T("Change ISAPI extension in LogEntryTypes table"));

			// Change ISAPI extension in LogEntryTypes table
			retSQL = dbConn.ExecuteSQL(_T("update LogEntryTypes set TypeDescrip = 'ISAPI extension' where LogEntryTypeId = 5"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Modify column index IX_TicketID in table TicketContacts") << endl;
			}

			g_logFile.Write(_T("Modify column index IX_TicketID in table TicketContacts"));

			// Modify column index IX_TicketID in table TicketContacts
			retSQL = dbConn.ExecuteSQL(_T("drop index TicketContacts.IX_TicketID create index IX_TicketID on TicketContacts(TicketID,ContactID)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Modify column index IX_ContactID in table TicketContacts") << endl;
			}

			g_logFile.Write(_T("Modify column index IX_ContactID in table TicketContacts"));

			// Modify column index IX_ContactID in table TicketContacts
			retSQL = dbConn.ExecuteSQL(_T("drop index TicketContacts.IX_ContactID create index IX_ContactID on TicketContacts(ContactID,TicketID)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Modify column index IX_ContactID in table PersonalData") << endl;
			}

			g_logFile.Write(_T("Modify column index IX_ContactID in table PersonalData"));

			// Modify column index IX_ContactID in table PersonalData
			retSQL = dbConn.ExecuteSQL(_T("drop index PersonalData.IX_ContactID create index IX_ContactID on PersonalData(DataValue,PersonalDataTypeID,ContactID)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Update Database Version") << endl;
			}
			
			g_logFile.Write(_T("Update Database Version"));

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '1.0.11' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("1.0.11.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.0.11.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 1.0.11 to 1.0.17
		//
		//*************************************************************************

		if(!TestVersion(1,0,11,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.0.17.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.0.17.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Modify Table Alerts") << endl;
			}

			g_logFile.Write(_T("Modify Table Alerts"));

			// add column
			retSQL = dbConn.ExecuteSQL(_T("alter table Alerts add FromEmailAddress varchar(255) default 'Alert System' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// close database and commit changes.
			dbConn.ExecuteSQL(_T("commit transaction"));
			dbConn.Disconnect();
			bIsTrans = FALSE;
			bIsDBConn = FALSE;
		}

		if(!bIsDBConn)
		{
			// re-connect to Database
			retSQL = dbConn.Connect(wDsn);
			if(!SQL_SUCCEEDED(retSQL))
			{
				if(g_showResults != 0)
				{
					_tcout << endl << dbConn.GetErrorString() << endl;
				}

				g_logFile.Write(dbConn.GetErrorString());

				rkMailFlow.Close();
				g_logFile.LineBreak();
				g_logFile.Line();
				return 3;
			}
		}

		if(!bIsTrans)
		{
			OutputDebugString(_T("Begin Transaction Line 2550\n"));
			// Begin transaction
			retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
			if(!SQL_SUCCEEDED(retSQL))
			{
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
			else
			{
				bIsTrans = TRUE;
			}
		}

		if(!TestVersion(1,0,11,0,dbVersion))
		{
			// set to blanks
			retSQL = dbConn.ExecuteSQL(_T("update Alerts set FromEmailAddress = ''"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Update Database Version") << endl;
			}
		
			g_logFile.Write(_T("Update Database Version"));
			
			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '1.0.17' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("1.0.17.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.0.17.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 1.0.17 to 1.1.0
		//
		//*************************************************************************

		if(!TestVersion(1,0,17,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.1.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.1.0.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Adding  column ForceSpellCheck to Agents Table") << endl;
			}

			g_logFile.Write(_T("Adding  column ForceSpellCheck to Agents Table"));

			// Adding  column ForceSpellCheck to Agents Table
			retSQL = dbConn.ExecuteSQL(_T("alter table Agents add ForceSpellCheck bit default '1' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Adding  column SignatureTopReply to Agents Table") << endl;
			}

			g_logFile.Write(_T("Adding  column SignatureTopReply to Agents Table"));

			// Adding  column SignatureTopReply to Agents Table
			retSQL = dbConn.ExecuteSQL(_T("alter table Agents add SignatureTopReply bit default '1' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
			
			if(g_showResults != 0)
			{
				_tcout << _T("Updating Database version in ServerParameters table") << endl;
			}

			g_logFile.Write(_T("Updating Database version in ServerParameters table"));

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '1.1.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("1.1.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.1.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 1.1.0 to 1.2.2
		//
		//*************************************************************************
	
		if(!TestVersion(1,1,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v1.2.2.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v1.2.2.0"));
			
			if(g_showResults != 0)
			{
				_tcout << _T("Inserting 'File could not be scanned' into table VirusScanStates") << endl;
			}

			g_logFile.Write(_T("Inserting 'File could not be scanned' into table VirusScanStates"));

			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT VirusScanStates ON insert into VirusScanStates (VirusScanStateID,Description) Values (7,'File could not be scanned') SET IDENTITY_INSERT VirusScanStates OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Inserting 'File is suspicous' into table VirusScanStates") << endl;
			}

			g_logFile.Write(_T("Inserting 'File is suspicous' into table VirusScanStates"));

			// VirusScanStates (Description) VALUES ('File is suspicous')
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT VirusScanStates ON insert into VirusScanStates (VirusScanStateID,Description) Values (8,'File is suspicous') SET IDENTITY_INSERT VirusScanStates OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
			
			if(g_showResults != 0)
			{
				_tcout << _T("Modifying RoutingRules Table, adding column AutoReplyFrom") << endl;
			}

			g_logFile.Write(_T("Modifying RoutingRules Table, adding column AutoReplyFrom"));

			retSQL = dbConn.ExecuteSQL(_T("alter table RoutingRules add AutoReplyFrom VARCHAR(255)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Create table CustomDictionary") << endl;
			}

			g_logFile.Write(_T("Create table CustomDictionary"));

			// Create table custom dictionary
			retSQL = dbConn.ExecuteSQL(_T("create table CustomDictionary ( CustomDictionaryID int IDENTITY(1,1) PRIMARY KEY not null, AgentID int not null, Word varchar(255) not null)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}


			// create index for custom dictionary
			retSQL = dbConn.ExecuteSQL(_T("create index IX_AgentID on CustomDictionary(AgentID)"));
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

			// alter table and add constraint
			retSQL = dbConn.ExecuteSQL(_T("alter table CustomDictionary add constraint DF_CustomDictionaryAgentID Default ('0') for AgentID"));
			if(!SQL_SUCCEEDED(retSQL))
			{
			return ErrorOpt(dbConn,rkMailFlow);
			}

			if(g_showResults != 0)
			{
				_tcout << _T("Updating Database version in ServerParameters table") << endl;
			}

			g_logFile.Write(_T("Updating Database version in ServerParameters table"));

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '1.2.2' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("1.2.2.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v1.2.2.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 1.2.2 to 2.0.0.3
		//
		//*************************************************************************
		
		if(!TestVersion(1,2,2,0,dbVersion))
		{
			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);
			OutputDebugString(sData);
			OutputDebugString(_T("\n"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v2.0.0.3") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v2.0.0.3"));
			
			// #1
			if(g_showResults != 0)
			{
				_tcout << _T("Adding new indexes to the Tickets Table") << endl;
			}

			g_logFile.Write(_T("Adding new indexes to the Tickets Table"));

			// create column index IX_Date in table Tickets
			retSQL = dbConn.ExecuteSQL(_T("create index IX_Date on Tickets(OpenTimeStamp,TicketID,TicketStateID,OwnerID,IsDeleted)"));
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

			// #2
			if(g_showResults != 0)
			{
				_tcout << _T("Update table AlertMsgs") << endl;
			}

			g_logFile.Write(_T("Update table AlertMsgs"));

			// drop constraint
			retSQL = dbConn.ExecuteSQL(_T("alter table AlertMsgs drop constraint DF_AlertMsgsViewed"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);
			OutputDebugString(sData);
			OutputDebugString(_T("\n"));

			// alter column and set to 0
			TCHAR* crTable = _T("alter table AlertMsgs  alter column Viewed tinyint not null alter table AlertMsgs  add constraint DF_AlertMsgsViewed default('0') for Viewed");
			retSQL = dbConn.ExecuteSQL(crTable);
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);
			OutputDebugString(sData);
			OutputDebugString(_T("\n"));

			// create column index IX_AgentID in table AlertMsgs
			retSQL = dbConn.ExecuteSQL(_T("create index IX_AgentID on AlertMsgs(AgentID,Viewed,AlertMsgID)"));
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

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);
			OutputDebugString(sData);
			OutputDebugString(_T("\n"));

			// QuarantinePage
			int rc = QuarantinePage(dbConn,rkMailFlow);
			if(rc != 0)
				return rc;

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			// Ticket Notes
			rc = TicketNotes(dbConn,rkMailFlow);
			if(rc != 0)
				return rc;

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			rc = TicketBoxAlg(dbConn,rkMailFlow);
			if(rc != 0)
				return rc;
			
			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			if(g_showResults != 0)
			{
				_tcout << _T("Alter Agents table and InboundMessages table") << endl;
			}

			g_logFile.Write(_T("Alter Agents table and InboundMessages table"));

			// Alter Agents table and Inbound Messages
			retSQL = dbConn.ExecuteSQL(_T("alter table Agents add ShowMessagesInbound bit default 1 not null alter table Agents add UsePreviewPane bit default 1 not null alter table Agents add CloseTicket tinyint default 1 not null alter table Agents add RouteToInbox tinyint default 1 not null alter table InboundMessages add DateReceived datetime"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			if(g_showResults != 0)
			{
				_tcout << _T("Update InboundMessages table") << endl;
			}

			g_logFile.Write(_T("Update InboundMessages table"));

			// Update InboundMessages table
			retSQL = dbConn.ExecuteSQL(_T("update InboundMessages set DateReceived=(select EMailDateTime from InboundMessages as I where I.InboundMessageID = InboundMessages.InboundMessageID)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
		
			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			if(g_showResults != 0)
			{
				_tcout << _T("Adding value to Server Parameter table") << endl;
			}

			g_logFile.Write(_T("Adding value to Server Parameter table"));

			// Adding value to Server Parameter table
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values(34,'System Email Address','systememail@mailflow.deerfield.net') insert into ServerParameters (ServerParameterID,Description,DataValue) values(35,'AntiVirus action for files that can not be scanned','0') insert into ServerParameters (ServerParameterID,Description,DataValue) values(36,'AntiVirus action for suspicious files','0') insert into ServerParameters (ServerParameterID,Description,DataValue) values(37,'Default routing rule agent ID','0') insert into ServerParameters (ServerParameterID,Description,DataValue) values(38,'Default routing rule TicketBoxID','0') insert into ServerParameters (ServerParameterID,Description,DataValue) values(39,'Remove tickets after archive','0') SET IDENTITY_INSERT ServerParameters OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			if(g_showResults != 0)
			{
				_tcout << _T("Updating MessageDestinations and MessageSources") << endl;
			}

			g_logFile.Write(_T("Updating MessageDestinations and MessageSources"));

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("alter table MessageDestinations add Description varchar(255) default '' not null alter table MessageSources add Description varchar(255) default '' not null"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			rc = EncryptAgentsPassword(dbConn,rkMailFlow);
			if(rc != 0)
				return rc;

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			rc = EncryptMessageSrcPasswords(dbConn,rkMailFlow);
			if(rc != 0)
				return rc;

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			rc = EncryptMessageDestPasswords(dbConn,rkMailFlow);
			if(rc != 0)
				return rc;

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			rc = CustomDictionaryFix(dbConn,rkMailFlow);
			if(rc != 0)
				return rc;

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			if(g_showResults != 0)
			{
				_tcout << _T("Updating StyleSheets table") << endl;
			}

			g_logFile.Write(_T("Updating StyleSheets table"));

			// Updating StyleSheets
			retSQL = dbConn.ExecuteSQL(_T("truncate table stylesheets insert into stylesheets(Name,Filename) values ('Standard','./stylesheets/standard.css') insert into stylesheets (Name,Filename) values ('Hot Dog','./stylesheets/hotdog.css') insert into StyleSheets (Name,Filename) values ('Earth','./stylesheets/earth.css') update Agents set StyleSheetID = StyleSheetID - 1 where StyleSheetID > 1"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
		
			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			if(g_showResults != 0)
			{
				_tcout << _T("Add entry 'InProgress' to OutboundMessageStates table") << endl;
			}

			g_logFile.Write(_T("Add entry 'InProgress' to OutboundMessageStates table"));

			// Updating Outbound Messages States
			retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT OutboundMessageStates ON INSERT INTO OutboundMessageStates (OutboundMsgStateID,MessageStateName) VALUES (6,'InProgress') SET IDENTITY_INSERT OutboundMessageStates OFF"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Debug
			dbConn.GetData(_T("select @@TRANCOUNT"),sData);

			rc = VTwoBuildThree(dbConn,rkMailFlow);
			if(rc != 0)
				return rc;

			rc = VThreeBuildFour(dbConn,rkMailFlow);
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '2.0.0.3' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("2.0.0.3"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v2.0.0.3") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);

			dbConn.GetData(_T("select @@TRANCOUNT"),sData);
		}

		//*************************************************************************
		// 
		// Version 2.0.0.3 to 2.0.0.4
		//
		//*************************************************************************
		
		if(!TestVersion(2,0,0,3,dbVersion))
		{

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v2.0.0.4") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v2.0.0.4"));
			
			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '2.0.0.4' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("2.0.0.4"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v2.0.0.4") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 2.0.0.4 to 2.0.0.7
		//
		//*************************************************************************
		
		if(!TestVersion(2,0,0,4,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v2.0.0.7") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v2.0.0.7"));
			
			int rc = V2007(dbConn,rkMailFlow);
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '2.0.0.7' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("2.0.0.7"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v2.0.0.7") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 2.0.0.7 to 3.0.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(2,0,0,7,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v3.0.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v3.0.0.0"));
			
			int rc = V3000(dbConn,rkMailFlow, wDsn);
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '3.0.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("3.0.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v3.0.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 3.0.0.0 to 3.0.0.1
		//
		//*************************************************************************
		
		if(!TestVersion(3,0,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v3.0.0.1") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v3.0.0.1"));
			
			int rc = V3001(dbConn,rkMailFlow, wDsn);
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '3.0.0.1' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("3.0.0.1"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v3.0.0.1") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 3.0.0.1 to 3.0.0.2
		//
		//*************************************************************************
		
		if(!TestVersion(3,0,0,1,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v3.0.0.2") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v3.0.0.2"));
			
			int rc = V3002(dbConn,rkMailFlow, wDsn);
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '3.0.0.2' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("3.0.0.2"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v3.0.0.2") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 3.0.0.2 to 3.1.0.1
		//
		//*************************************************************************
		
		if(!TestVersion(3,0,0,2,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v3.1.0.1") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v3.1.0.1"));
			
			//It says to run V3100, but it's really V3101
			int rc = V3100(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '3.1.0.1' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("3.1.0.1"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v3.1.0.1") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 3.1.0.1 to 3.5.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(3,1,0,1,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v3.5.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v3.5.0.0"));
			
			int rc = V3500(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
			{
				return rc;
			}
			
			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '3.5.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("3.5.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v3.5.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 3.5.0.0 to 3.6.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(3,5,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v3.6.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v3.6.0.0"));
			
			int rc = V3600(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '3.6.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("3.6.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v3.6.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 3.6.0.0 to 3.6.1.0
		//
		//*************************************************************************
		
		if(!TestVersion(3,6,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v3.6.1.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v3.6.1.0"));
			
			int rc = V3610(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '3.6.1.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("3.6.1.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v3.6.1.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 3.6.1.0 to 3.6.1.1
		//
		//*************************************************************************
		
		if(!TestVersion(3,6,1,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v3.6.1.1") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v3.6.1.1"));
			
			int rc = V3611(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '3.6.1.1' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("3.6.1.1"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v3.6.1.1") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 3.6.1.1 to 3.7.0.1
		//
		//*************************************************************************
		
		if(!TestVersion(3,6,1,1,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v3.7.0.1") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v3.7.0.1"));
			
			if(bIsTrans)
			{
				// commit changes.
				
				dbConn.GetData(_T("select @@TRANCOUNT"),sData);
				dbConn.ExecuteSQL(_T("commit transaction"));
				bIsTrans = FALSE;				
			}

			// Rename TicketFieldsTicket.TicketFieldTicketID to TicketFieldsTicketID
			if(g_showResults != 0)
			{
				_tcout << _T("Checking DB for column TicketFieldsTicket.TicketFieldTicketID") << endl;
			}

			g_logFile.Write(_T("Checking DB for column TicketFieldsTicket.TicketFieldTicketID"));
			
			int nColumnID=0;
			bool columnExists = false;				

			m_query.Initialize();
			BINDCOL_LONG_NOLEN( m_query, nColumnID );
			m_query.Execute( L"SELECT ORDINAL_POSITION FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME = 'TicketFieldsTicket' AND COLUMN_NAME = 'TicketFieldTicketID'" );
			if( m_query.Fetch() == S_OK )
			{
				g_logFile.Write(_T("Column TicketFieldTicketID in table TicketFieldsTicket exists"));
				columnExists = true;

				if(g_showResults != 0)
				{
					_tcout << _T("Column TicketFieldTicketID in table TicketFieldsTicket exists") << endl;
				}
			}

			if ( columnExists )
			{
				//rename the column
				g_logFile.Write(_T("Rename TicketFieldsTicket.TicketFieldTicketID to TicketFieldsTicketID"));
				if(g_showResults != 0)
				{
					_tcout << _T("Rename TicketFieldsTicket.TicketFieldTicketID to TicketFieldsTicketID") << endl;
				}				
				retSQL = dbConn.ExecuteSQL(_T("EXEC sp_rename 'TicketFieldsTicket.TicketFieldTicketID','TicketFieldsTicketID','COLUMN'"));
				if(!SQL_SUCCEEDED(retSQL))
				{
					return ErrorOpt(dbConn,rkMailFlow);
				}
			}
			else
			{
				g_logFile.Write(_T("Column TicketFieldTicketID in table TicketFieldsTicket does not exist"));
				
				if(g_showResults != 0)
				{
					_tcout << _T("Column TicketFieldTicketID in table TicketFieldsTicket does not exist") << endl;
				}
			}
			
			// Rename TicketFieldsTicketBox.TicketFieldTicketBoxID to TicketFieldsTicketBoxID
			if(g_showResults != 0)
			{
				_tcout << _T("Checking DB for column TicketFieldsTicketBox.TicketFieldTicketBoxID") << endl;
			}

			g_logFile.Write(_T("Checking DB for column TicketFieldsTicketBox.TicketFieldTicketBoxID"));
			
			nColumnID=0;
			columnExists = false;				

			m_query.Initialize();
			BINDCOL_LONG_NOLEN( m_query, nColumnID );
			m_query.Execute( L"SELECT ORDINAL_POSITION FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME = 'TicketFieldsTicketBox' AND COLUMN_NAME = 'TicketFieldTicketBoxID'" );
			if( m_query.Fetch() == S_OK )
			{
				g_logFile.Write(_T("Column TicketFieldTicketBoxID in table TicketFieldsTicketBox exists"));
				columnExists = true;

				if(g_showResults != 0)
				{
					_tcout << _T("Column TicketFieldTicketBoxID in table TicketFieldsTicketBox exists") << endl;
				}
			}

			if ( columnExists )
			{
				//rename the column
				g_logFile.Write(_T("Rename TicketFieldsTicketBox.TicketFieldTicketBoxID to TicketFieldsTicketBoxID"));
				if(g_showResults != 0)
				{
					_tcout << _T("Rename TicketFieldsTicketBox.TicketFieldTicketBoxID to TicketFieldsTicketBoxID") << endl;
				}				
				retSQL = dbConn.ExecuteSQL(_T("EXEC sp_rename 'TicketFieldsTicketBox.TicketFieldTicketBoxID','TicketFieldsTicketBoxID','COLUMN'"));
				if(!SQL_SUCCEEDED(retSQL))
				{
					return ErrorOpt(dbConn,rkMailFlow);
				}
			}
			else
			{
				g_logFile.Write(_T("Column TicketFieldTicketBoxID in table TicketFieldsTicketBox does not exist"));
				columnExists = true;

				if(g_showResults != 0)
				{
					_tcout << _T("Column TicketFieldTicketBoxID in table TicketFieldsTicketBox does not exist") << endl;
				}	
			}
            
			if(!bIsTrans)
			{
				OutputDebugString(_T("Begin Transaction"));
				// Begin transaction
				retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
				if(!SQL_SUCCEEDED(retSQL))
				{
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
				else
				{
					bIsTrans = TRUE;
				}
			}			
			
			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '3.7.0.1' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("3.7.0.1"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v3.7.0.1") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 3.7.0.1 to 3.8.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(3,7,0,1,dbVersion))
		{
			int nIndexID = 0;
			bool indexExists = false;
			
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v3.8.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v3.8.0.0"));

			//Indexes			
			
			if(bIsTrans)
			{
				// commit changes.
				dbConn.GetData(_T("select @@TRANCOUNT"),sData);
				dbConn.ExecuteSQL(_T("commit transaction"));
				bIsTrans = FALSE;				
			}
		
			// Modify column index IX_TicketBoxID in table Tickets
			if(g_showResults != 0)
			{
				_tcout << _T("Modify column index IX_TicketBoxID in table Tickets") << endl;
			}

			g_logFile.Write(_T("Modify column index IX_TicketBoxID in table Tickets"));
			
			m_query.Initialize();
			BINDCOL_LONG_NOLEN( m_query, nIndexID );
			m_query.Execute( L"SELECT i.id FROM sysobjects o, sysindexes i WHERE o.id=i.id and o.xtype='u' and o.name='Tickets' and i.name='IX_TicketBoxID'" );
			if( m_query.Fetch() == S_OK )
			{
				g_logFile.Write(_T("Index IX_TicketBoxID in table Tickets exists"));
				indexExists = true;
			}

			if ( indexExists )
			{
				//drop the index
				g_logFile.Write(_T("Drop index IX_TicketBoxID in table Tickets"));
				retSQL = dbConn.ExecuteSQL(_T("drop index Tickets.IX_TicketBoxID"));
				if(!SQL_SUCCEEDED(retSQL))
				{
					return ErrorOpt(dbConn,rkMailFlow);
				}
			}
			
			//create the index
			g_logFile.Write(_T("Create index IX_TicketBoxID in table Tickets"));
			retSQL = dbConn.ExecuteSQL(_T("create index IX_TicketBoxID on Tickets(TicketBoxID,TicketStateID,OwnerID,IsDeleted,TicketCategoryID,FolderID,TicketID)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
			

			// Modify column index IX_OwnerID in table Tickets
			if(g_showResults != 0)
			{
				_tcout << _T("Modify column index IX_OwnerID in table Tickets") << endl;
			}

			g_logFile.Write(_T("Modify column index IX_OwnerID in table Tickets"));
			
			nIndexID=0;
			indexExists = false;				

			m_query.Initialize();
			BINDCOL_LONG_NOLEN( m_query, nIndexID );
			m_query.Execute( L"SELECT i.id FROM sysobjects o, sysindexes i WHERE o.id=i.id and o.xtype='u' and o.name='Tickets' and i.name='IX_OwnerID'" );
			if( m_query.Fetch() == S_OK )
			{
				g_logFile.Write(_T("Index IX_OwnerID in table Tickets exists"));
				indexExists = true;
			}

			if ( indexExists )
			{
				//drop the index
				g_logFile.Write(_T("Drop index IX_OwnerID in table Tickets"));
				retSQL = dbConn.ExecuteSQL(_T("drop index Tickets.IX_OwnerID"));
				if(!SQL_SUCCEEDED(retSQL))
				{
					return ErrorOpt(dbConn,rkMailFlow);
				}
			}
			
			nIndexID=0;
			indexExists = false;				

			m_query.Initialize();
			BINDCOL_LONG_NOLEN( m_query, nIndexID );
			m_query.Execute( L"SELECT i.id FROM sysobjects o, sysindexes i WHERE o.id=i.id and o.xtype='u' and o.name='Tickets' and i.name='IX_Owner'" );
			if( m_query.Fetch() == S_OK )
			{
				g_logFile.Write(_T("Index IX_Owner in table Tickets exists"));
				indexExists = true;
			}

			if ( indexExists )
			{
				//drop the index
				g_logFile.Write(_T("Drop index IX_Owner in table Tickets"));
				retSQL = dbConn.ExecuteSQL(_T("drop index Tickets.IX_Owner"));
				if(!SQL_SUCCEEDED(retSQL))
				{
					return ErrorOpt(dbConn,rkMailFlow);
				}
			}

			//create the index
			g_logFile.Write(_T("Create index IX_OwnerID in table Tickets"));
			retSQL = dbConn.ExecuteSQL(_T("create index IX_OwnerID on Tickets(OwnerID, FolderID, IsDeleted, TicketStateID)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
			
			// Add column index IX_TicketNoteID in table TicketNotesRead
			if(g_showResults != 0)
			{
				_tcout << _T("Modify column index IX_TicketNoteID in table TicketNotesRead") << endl;
			}

			g_logFile.Write(_T("Modify column index IX_TicketNoteID in table TicketNotesRead"));
			
			nIndexID=0;
			indexExists = false;				

			m_query.Initialize();
			BINDCOL_LONG_NOLEN( m_query, nIndexID );
			m_query.Execute( L"SELECT i.id FROM sysobjects o, sysindexes i WHERE o.id=i.id and o.xtype='u' and o.name='TicketNotesRead' and i.name='IX_TicketNoteID'" );
			if( m_query.Fetch() == S_OK )
			{
				g_logFile.Write(_T("Index IX_TicketNoteID in table TicketNotesRead exists"));
				indexExists = true;
			}

			if ( indexExists )
			{
				//drop the index
				g_logFile.Write(_T("Drop index IX_TicketNoteID in table TicketNotesRead"));
				retSQL = dbConn.ExecuteSQL(_T("drop index TicketNotesRead.IX_TicketNoteID"));
				if(!SQL_SUCCEEDED(retSQL))
				{
					return ErrorOpt(dbConn,rkMailFlow);
				}
			}
			
			//create the index
			g_logFile.Write(_T("Create index IX_TicketNoteID in table TicketNotesRead"));
			retSQL = dbConn.ExecuteSQL(_T("create index IX_TicketNoteID on TicketNotesRead(TicketNoteID,AgentID)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			// Add column index IX_InboundMessageID in table InboundMessageRead
			if(g_showResults != 0)
			{
				_tcout << _T("Modify column index IX_InboundMessageID in table InboundMessageRead") << endl;
			}

			g_logFile.Write(_T("Modify column index IX_InboundMessageID in table InboundMessageRead"));
			
			nIndexID=0;
			indexExists = false;				

			m_query.Initialize();
			BINDCOL_LONG_NOLEN( m_query, nIndexID );
			m_query.Execute( L"SELECT i.id FROM sysobjects o, sysindexes i WHERE o.id=i.id and o.xtype='u' and o.name='InboundMessageRead' and i.name='IX_InboundMessageID'" );
			if( m_query.Fetch() == S_OK )
			{
				g_logFile.Write(_T("Index IX_InboundMessageID in table InboundMessageRead exists"));
				indexExists = true;
			}

			if ( indexExists )
			{
				//drop the index
				g_logFile.Write(_T("Drop index IX_InboundMessageID in table InboundMessageRead"));
				retSQL = dbConn.ExecuteSQL(_T("drop index InboundMessageRead.IX_InboundMessageID"));
				if(!SQL_SUCCEEDED(retSQL))
				{
					return ErrorOpt(dbConn,rkMailFlow);
				}
			}
			
			//create the index
			g_logFile.Write(_T("Create index IX_InboundMessageID in table InboundMessageRead"));
			retSQL = dbConn.ExecuteSQL(_T("create index IX_InboundMessageID on InboundMessageRead(InboundMessageID,AgentID)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}			

			// Add column index IX_AgentID in table AgentTicketCategories
			if(g_showResults != 0)
			{
				_tcout << _T("Modify column index IX_AgentID in table AgentTicketCategories") << endl;
			}

			g_logFile.Write(_T("Modify column index IX_AgentID in table AgentTicketCategories"));
			
			nIndexID=0;
			indexExists = false;				

			m_query.Initialize();
			BINDCOL_LONG_NOLEN( m_query, nIndexID );
			m_query.Execute( L"SELECT i.id FROM sysobjects o, sysindexes i WHERE o.id=i.id and o.xtype='u' and o.name='AgentTicketCategories' and i.name='IX_AgentID'" );
			if( m_query.Fetch() == S_OK )
			{
				g_logFile.Write(_T("Index IX_AgentID in table AgentTicketCategories exists"));
				indexExists = true;
			}

			if ( indexExists )
			{
				//drop the index
				g_logFile.Write(_T("Drop index IX_AgentID in table AgentTicketCategories"));
				retSQL = dbConn.ExecuteSQL(_T("drop index AgentTicketCategories.IX_AgentID"));
				if(!SQL_SUCCEEDED(retSQL))
				{
					return ErrorOpt(dbConn,rkMailFlow);
				}
			}
			
			//create the index
			g_logFile.Write(_T("Create index IX_AgentID in table AgentTicketCategories"));
			retSQL = dbConn.ExecuteSQL(_T("create index IX_AgentID on AgentTicketCategories(AgentID,TicketCategoryID)"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			if(!bIsTrans)
			{
				OutputDebugString(_T("Begin Transaction"));
				// Begin transaction
				retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
				if(!SQL_SUCCEEDED(retSQL))
				{
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
				else
				{
					bIsTrans = TRUE;
				}
			}
			
			int rc = V3800(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '3.8.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("3.8.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v3.8.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 3.8.0.0 to 4.0.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(3,8,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.0.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.0.0.0"));
			
			int rc = V4000(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.0.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.0.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.0.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.0.0.0 to 4.5.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(4,0,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.5.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.5.0.0"));
			
			int rc = V4500(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.5.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.5.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.5.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.5.0.0 to 4.8.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(4,5,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.8.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.8.0.0"));
			
			int rc = V4800(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.8.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.8.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.8.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.8.0.0 to 4.8.1.0
		//
		//*************************************************************************
		
		if(!TestVersion(4,8,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.8.1.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.8.1.0"));
			
			int rc = V4810(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.8.1.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.8.1.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.8.1.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.8.1.0 to 4.9.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(4,8,1,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.9.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.9.0.0"));
			
			int rc = V4900(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.9.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.9.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.9.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.9.0.0 to 4.9.1.0
		//
		//*************************************************************************
		
		if(!TestVersion(4,9,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.9.1.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.9.1.0"));
			
			int rc = V4910(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.9.1.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.9.1.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.9.1.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.9.1.0 to 4.9.1.2
		//
		//*************************************************************************
		
		if(!TestVersion(4,9,1,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.9.1.2") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.9.1.2"));
			
			int rc = V4912(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.9.1.2' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.9.1.2"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.9.1.2") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.9.1.2 to 4.9.1.4
		//
		//*************************************************************************
		
		if(!TestVersion(4,9,1,2,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.9.1.4") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.9.1.4"));
			
			int rc = V4914(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.9.1.4' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.9.1.4"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.9.1.4") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.9.1.4 to 4.9.2.0
		//
		//*************************************************************************
		
		if(!TestVersion(4,9,1,4,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.9.2.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.9.2.0"));
			
			int rc = V4920(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.9.2.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.9.2.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.9.2.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.9.2.0 to 4.9.2.1
		//
		//*************************************************************************
		
		if(!TestVersion(4,9,2,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.9.2.1") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.9.2.1"));
			
			int rc = V4921(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.9.2.1' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.9.2.1"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.9.2.1") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.9.2.1 to 4.9.2.2
		//
		//*************************************************************************
		
		if(!TestVersion(4,9,2,1,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v4.9.2.2") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v4.9.2.2"));
			
			int rc = V4922(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '4.9.2.2' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("4.9.2.2"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v4.9.2.2") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 4.9.2.2 to 5.0.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(4,9,2,2,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v5.0.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v5.0.0.0"));
			
			int rc = V5000(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '5.0.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("5.0.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v5.0.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 5.0.0.0 to 5.0.3.0
		//
		//*************************************************************************
		
		if(!TestVersion(5,0,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v5.0.3.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v5.0.3.0"));
			
			int rc = V5030(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '5.0.3.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("5.0.3.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v5.0.3.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 5.0.3.0 to 5.1.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(5,0,3,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v5.1.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v5.1.0.0"));
			
			int rc = V5100(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '5.1.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("5.1.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v5.1.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 5.1.0.0 to 6.0.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(5,1,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.0.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.0.0.0"));
			
			int rc = V6000(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.0.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.0.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.0.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.0.0.0 to 6.0.0.1
		//
		//*************************************************************************
		
		if(!TestVersion(6,0,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.0.0.1") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.0.0.1"));
			
			int rc = V6001(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.0.0.1' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.0.0.1"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.0.0.1") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.0.0.1 to 6.1.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(6,0,0,1,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.1.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.1.0.0"));
			
			int rc = V6100(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.1.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.1.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.1.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.1.0.0 to 6.1.0.2
		//
		//*************************************************************************
		
		if(!TestVersion(6,1,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.1.0.2") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.1.0.2"));
			
			int rc = V6102(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.1.0.2' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.1.0.2"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.1.0.2") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.1.0.2 to 6.5.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(6,1,0,2,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.5.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.5.0.0"));
			
			int rc = V6500(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.5.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.5.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.5.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.5.0.0 to 6.5.0.1
		//
		//*************************************************************************
		
		if(!TestVersion(6,5,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.5.0.1") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.5.0.1"));
			
			int rc = V6501(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.5.0.1' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.5.0.1"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.5.0.1") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.5.0.1 to 6.5.0.4
		//
		//*************************************************************************
		
		if(!TestVersion(6,5,0,1,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.5.0.4") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.5.0.4"));
			
			int rc = V6504(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.5.0.4' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.5.0.4"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.5.0.4") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.5.0.4 to 6.5.0.5
		//
		//*************************************************************************
		
		if(!TestVersion(6,5,0,4,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.5.0.5") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.5.0.5"));
			
			int rc = V6505(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.5.0.5' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.5.0.5"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.5.0.5") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.5.0.5 to 6.6.0.1
		//
		//*************************************************************************
		
		if(!TestVersion(6,5,0,5,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.6.0.1") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.6.0.1"));
			
			int rc = V6601(m_query,dbConn,rkMailFlow,wDsn);
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.6.0.1' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.6.0.1"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.6.0.1") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.6.0.1 to 6.6.0.3
		//
		//*************************************************************************
		
		if(!TestVersion(6,6,0,1,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.6.0.3") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.6.0.3"));
			
			int rc = V6603(m_query,dbConn,rkMailFlow,wDsn);			
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.6.0.3' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.6.0.3"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.6.0.3") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.6.0.3 to 6.6.1.0
		//
		//*************************************************************************
		
		if(!TestVersion(6,6,0,3,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.6.1.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.6.1.0"));
			
			int rc = V6610(m_query,dbConn,rkMailFlow,wDsn);			
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.6.1.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.6.1.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.6.1.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.6.1.0 to 6.9.0.0
		//
		//*************************************************************************
		
		if(!TestVersion(6,6,1,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.9.0.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.9.0.0"));
			
			int rc = V6900(m_query,dbConn,rkMailFlow,wDsn);			
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.9.0.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.9.0.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.9.0.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.9.0.0 to 6.9.1.0
		//
		//*************************************************************************
		
		if(!TestVersion(6,9,0,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.9.1.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.9.1.0"));
			
			int rc = V6910(m_query,dbConn,rkMailFlow,wDsn);			
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.9.1.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.9.1.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.9.1.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.9.1.0 to 6.9.2.0
		//
		//*************************************************************************
		
		if(!TestVersion(6,9,1,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.9.2.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.9.2.0"));
			
			int rc = V6920(m_query,dbConn,rkMailFlow,wDsn);			
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.9.2.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.9.2.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.9.2.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}
		
		//*************************************************************************
		// 
		// Version 6.9.2.0 to 6.9.3.0
		//
		//*************************************************************************
		
		if(!TestVersion(6,9,2,0,dbVersion))
		{
			if(g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.9.3.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.9.3.0"));
			
			int rc = V6930(m_query,dbConn,rkMailFlow,wDsn);			
			
			if(rc != 0)
				return rc;

			if(g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.9.3.0' where ServerParameterId = 24"));
			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}

			lstrcpy(dbVersion,_T("6.9.3.0"));

			if(g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.9.3.0") << endl;
			}
			wsprintf(sLogBuf,_T("Database updated to version - %s"),dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// Version 6.9.3.0 to 6.9.4.0
		//
		//*************************************************************************

		if (!TestVersion(6, 9, 3, 0, dbVersion))
		{
			if (g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.9.4.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.9.4.0"));

			//int rc = V6930(m_query, dbConn, rkMailFlow, wDsn);

			//if (rc != 0)
				//return rc;

			if (g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}

			g_logFile.Write(sDbVer);

			// Update Database Version
			retSQL = dbConn.ExecuteSQL(_T("update ServerParameters set DataValue = '6.9.4.0' where ServerParameterId = 24"));
			if (!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn, rkMailFlow);
			}

			lstrcpy(dbVersion, _T("6.9.4.0"));

			if (g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.9.4.0") << endl;
			}
			wsprintf(sLogBuf, _T("Database updated to version - %s"), dbVersion);
			g_logFile.Write(sLogBuf);
		}
		//*************************************************************************
		// 
		// Version 6.9.4.0 to 6.9.5.0
		//
		//*************************************************************************

		//if (!TestVersion(6, 9, 4, 5, dbVersion)) - OLD LOGIC
		if (_tcsicmp(dbVersion, _T("6.9.5.0")) != 0) // NEW LOGIC - KF

		{
			if (g_showResults != 0)
			{
				_tcout << endl << _T("Begin updating DB to v6.9.5.0") << endl;
			}
			g_logFile.Write(_T("Begin updating DB to v6.9.5.0"));

			int rc = V6950(m_query, dbConn, rkMailFlow, wDsn);
			if (rc != 0)
				return rc;

			if (g_showResults != 0)
			{
				_tcout << (PCTSTR)sDbVer << endl;
			}
			g_logFile.Write(sDbVer);

			// Only update version here
			retSQL = dbConn.ExecuteSQL(_T("UPDATE ServerParameters SET DataValue = '6.9.5.0' WHERE ServerParameterId = 24"));
			if (!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn, rkMailFlow);
			}

			lstrcpy(dbVersion, _T("6.9.5.0"));

			if (g_showResults != 0)
			{
				_tcout << endl << _T("Finished updating DB to v6.9.5.0") << endl;
			}

			wsprintf(sLogBuf, _T("Database updated to version - %s"), dbVersion);
			g_logFile.Write(sLogBuf);
		}

		//*************************************************************************
		// 
		// We made it so close everything down and clean it up.
		// Make sure you change all the version info and .exe names.
		//
		//*************************************************************************

		if(g_showResults != 0)
		{
			_tcout << _T("Update Successful") << endl;
		}

		g_logFile.Write(_T("Update Successful"));
		
		retVal = rvDBVersion.SetString(rkMailFlow,_T("dbVersion"),_T("6.9.5.0"));
		if(retVal != 0)
		{
			_tcout << endl << _T("Failed to update registry value dbVersion") << endl;
			g_logFile.Write(_T("Failed to update registry value dbVersion"));
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 2;
		}

		dbConn.GetData(_T("select @@TRANCOUNT"),sData);

		dbConn.ExecuteSQL(_T("commit transaction"));

		//Check to make sure the TicketBoxes table was updated
		dbConn.ExecuteSQL(_T("IF COL_LENGTH('TicketBoxes','UnreadMode') IS NULL BEGIN ALTER TABLE TicketBoxes ADD UnreadMode INT DEFAULT 0 NOT NULL END"));
		dbConn.ExecuteSQL(_T("IF COL_LENGTH('TicketBoxes','FromFormat') IS NULL BEGIN ALTER TABLE TicketBoxes ADD FromFormat INT DEFAULT 0 NOT NULL END"));
		
		dbConn.Disconnect();
		rkMailFlow.Close();

		g_logFile.LineBreak();
		g_logFile.Line();

		return 0;
	}
	catch(const TCHAR* except)
	{
		if(g_showResults != 0)
		{
			_tcout << except << endl;
		}
	
		g_logFile.Write(except);
		return 1;
	}
	catch(...)
	{
		if(g_showResults != 0)
		{
			_tcout << _T("Unknown Exception") << endl;
		}
	
		g_logFile.Write(_T("Unknown Exception"));
		return 1;
	}
}

//*****************************************************************************


//*****************************************************************************

// Banner Function
void Banner()
{
	_tcout << _T("VisNetic MailFlow v6.9.5") << endl
		 << _T("Database Update Utility") << endl;
}

//*****************************************************************************

//*****************************************************************************

// TestVersion Function
int TestVersion(int h,int l,int i,int b,const TCHAR* ver)
{
	
	TCHAR string[256];  // buffer to hold our string value;
	_TINT dot = '.';	// our dot character

	int result;
	int majorVer = 0;
	int minorVer = 0;
	int macroVer = 0;
	int buildVer = 0;
	int j = 0;

	// Get Major Version
	TCHAR* pdest = (TCHAR*)_tcschr(ver,dot);
	if(pdest == NULL)
		throw _T("Invalid Version Number");

	result = pdest - ver;
	for(j;j < result;j++)
	{
		string[j] = ver[j];
	}
	string[j] = '\0';
	majorVer = _ttoi(string);

	j = 0;
	pdest++;

	// Get Minor Version
	TCHAR* temp1 = _tcschr(pdest,dot);
	if(temp1 == NULL)
		throw "Invalid Version Number";

	result = temp1 - pdest;
	for(j;j < result;j++)
	{
		string[j] = pdest[j];
	}
	string[j] = '\0';
	minorVer = _ttoi(string);

	j = 0;
	temp1++;

	TCHAR* temp2 = _tcschr(temp1,dot);
	if(temp2 == NULL)
		throw _T("Invalid Version Number");

	result = temp2 - temp1;
	for(j;j < result;j++)
	{
		string[j] = temp1[j];
	}
	string[j] = '\0';
	macroVer = _ttoi(string);

	temp2++;
	buildVer = _ttoi(temp2);


	if(majorVer == h)
	{
		if(minorVer == l)
		{
			if(macroVer == i)
			{
				if(buildVer == b)
				{
					return 0;
				}
				else if(buildVer > b)
				{
					return -1;
				}
				else
				{
					return 1;
				}
			}
			else if(macroVer > i)
			{
				return -1;
			}
			else
			{
				return 1;
			}
		}
		else if(minorVer > l)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	else if(majorVer > h)
	{
		return -1;
	}
	else
	{
		return 1;
	}

	return 0;	
}

//*****************************************************************************

BOOL IsOldVersion(TCHAR* ver)
{
	int dots = 0;
	int len = _tcslen(ver);

	for(int i = 0;i < len; i++)
	{
		if(ver[i] == '.')
			dots++;
	}

	if(dots < 3)
		return FALSE;

	return TRUE;
}
