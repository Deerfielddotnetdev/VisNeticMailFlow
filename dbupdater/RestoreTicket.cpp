#include "prehead.h"
#include "main.h"
#include "ArchiveFns.h"
#include "TicketHistoryFns.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

typedef TCHAR String[256];
typedef const TCHAR* PCTSTR;

int RestoreTicket(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn, int TicketID)
{
	SQLRETURN	retSQL = 0;
	reg::Value	rvArchivePath;
	String		sArchivePath;
	String      sLogBuf;
	int			retVal;
	maxObjectID = 0;
		
	// Get Archive Path
	_tcout << endl << _T("Getting Archive Path from the MailFlow Registry") << endl;
	g_logFile.Write(_T("Getting Archive Path from the MailFlow Registry"));
	retVal = rvArchivePath.GetString(rkMailFlow,_T("ArchivePath"),sArchivePath);
	if(retVal != 0)
	{
		_tcout << endl << _T("Failed to access registry value ArchivePath") << endl;
		g_logFile.Write(_T("Failed to access registry value ArchivePath"));
		rkMailFlow.Close();
		g_logFile.LineBreak();
		g_logFile.Line();
		return 2;
	}

	// Display results and test value
	wsprintf(sLogBuf,_T("Found Archive Path: %s"),sArchivePath);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	//Check to see if Ticket exists in Tickets table
	bool bFoundTicketDB = false;	
	
	m_query.Initialize();
	BINDPARAM_LONG( m_query, TicketID );
	BINDCOL_LONG_NOLEN( m_query, maxObjectID );
	m_query.Execute( L"SELECT TicketID FROM Tickets WHERE TicketID=?" );
	if( m_query.Fetch() == S_OK )
	{
		//If Ticket exists, prompt to continue
		if(maxObjectID == TicketID)
		{
			char sRetVal;
			do
			{
				wsprintf(sLogBuf,_T("TicketID: %d exists in the MailFlow database, would you still like to restore it from Archive (y/n)? "),TicketID);
				_tcout << endl << sLogBuf;
				g_logFile.Write(sLogBuf);
				cin >> sRetVal;
			}
			while( !cin.fail() && sRetVal!='y' && sRetVal!='n' );
			if(sRetVal == 'n')
			{
				return 1;
			}
			else
			{
				bFoundTicketDB = true;
			}
		}
	}

	TTickets m_Ticket;
	m_Ticket.m_TicketID = TicketID;
	
	if(bFoundTicketDB)
	{
		m_Ticket.Query(m_query);		
	}

	//Get list of archive files from Database in Descending order (newest first)
	TArchives arc;
	vector<Archives_t> m_arc;
	vector<Archives_t>::iterator arcIter;

	m_query.Initialize();
	BINDCOL_LONG( m_query, arc.m_ArchiveID );
	BINDCOL_TIME( m_query, arc.m_DateCreated );
	BINDCOL_TCHAR( m_query, arc.m_ArcFilePath );
	BINDCOL_LONG( m_query, arc.m_InMsgRecords );
	BINDCOL_LONG( m_query, arc.m_InAttRecords );
	BINDCOL_LONG( m_query, arc.m_OutMsgRecords );
	BINDCOL_LONG( m_query, arc.m_OutAttRecords );
	m_query.Execute( _T("SELECT ArchiveID,DateCreated,ArcFilePath,InMsgRecords,InAttRecords,OutMsgRecords,OutAttRecords ")
						_T("FROM Archives ")
						_T("ORDER BY ArchiveID DESC") );	
	
	m_arc.clear();

	while( m_query.Fetch() == S_OK )
	{
		m_arc.push_back( arc );			
	}
	
	int nTicketArcID = 0;
	int numFound = 0;
	tstring sArchive;
	bool bFoundTicket = false;

	//Walk through archive files to find first instance of TicketID
	for( arcIter = m_arc.begin(); arcIter != m_arc.end(); arcIter++ )
	{
		int retval = 0;		
		CEMSString sDate;
		GetDateTimeString( arcIter->m_DateCreated, sizeof(arcIter->m_DateCreated), sDate, true, true );
        retval = GetTicketFromArchive( m_query, arcIter->m_ArchiveID, m_Ticket, sArchive );
		if( retval == Arc_Success )
		{
			numFound++;
			nTicketArcID = arcIter->m_ArchiveID;		
			bFoundTicket = true;			
			wsprintf(sLogBuf,_T("TicketID: %d found in ArchiveID: %d Created on: %s."),TicketID,nTicketArcID,sDate.c_str());
			_tcout << sLogBuf << endl;
			g_logFile.Write(sLogBuf);			
		}
		else
		{
			wsprintf(sLogBuf,_T("Get TicketID: %d from ArchiveID: %d Created on: %s returned: %d."),TicketID,arcIter->m_ArchiveID,sDate.c_str(),retval);
			_tcout << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}
	}
	
	if(!bFoundTicket)
	{
		wsprintf(sLogBuf,_T("TicketID: %d not found in an Archive file."),TicketID);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;		
	}
	else
	{
		int retval = 0;			
		if(numFound > 1)
		{
			int nRetVal;
			do
			{
				wsprintf(sLogBuf,_T("TicketID: %d found in multiple Archives, enter the ArchiveID you would like to restore from:"),TicketID);
				_tcout << endl << sLogBuf;
				g_logFile.Write(sLogBuf);
				cin >> nRetVal;
			}
			while( !cin.fail() && nRetVal==0 );
			retval = GetTicketFromArchive( m_query, nRetVal, m_Ticket, sArchive );
			if( retval != Arc_Success )
			{
				wsprintf(sLogBuf,_T("TicketID: %d not found in ArchiveID: %d."),TicketID,nRetVal);
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
				return 1;
			}
			else
			{
				nTicketArcID = nRetVal;
			}
		}
		else
		{
			retval = GetTicketFromArchive( m_query, nTicketArcID, m_Ticket, sArchive );
			if( retval != Arc_Success )
			{
				wsprintf(sLogBuf,_T("TicketID: %d not found in ArchiveID: %d."),TicketID,nTicketArcID);
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
				return 1;
			}
		}
	}

	//Check to see if TicketBoxID exists
	maxObjectID = 0;
	m_query.Initialize();
	BINDPARAM_LONG( m_query, m_Ticket.m_TicketBoxID );
	BINDCOL_LONG_NOLEN( m_query, maxObjectID );
	m_query.Execute( L"SELECT TicketBoxID FROM TicketBoxes WHERE TicketBoxID=?" );
	if( m_query.Fetch() == S_OK )
	{
		//If TicketBox does not exist, put in Unassigned
		if(maxObjectID != m_Ticket.m_TicketBoxID)
		{
			wsprintf(sLogBuf,_T("The TicketBox that TicketID: %d was previously in no longer exists, the Ticket will be restored to the Unassigned TicketBox."),TicketID);
			_tcout << endl << sLogBuf << endl;
			m_Ticket.m_TicketBoxID = 1;
		}
	}
		
	if(m_Ticket.m_OwnerID > 0)
	{
		//Check to see if Ticket Owner exists
		maxObjectID = 0;
		m_query.Initialize();
		BINDPARAM_LONG( m_query, m_Ticket.m_OwnerID );
		BINDCOL_LONG_NOLEN( m_query, maxObjectID );
		m_query.Execute( L"SELECT AgentID FROM Agents WHERE AgentID=? AND IsDeleted=0 AND IsEnabled=1" );
		if( m_query.Fetch() == S_OK )
		{
			//If TicketBox does not exist, put in Unassigned
			if(maxObjectID != m_Ticket.m_OwnerID)
			{
				wsprintf(sLogBuf,_T("The Owner that TicketID: %d was previously assigned to longer exists, the Ticket Owner will be set to NONE."),TicketID);
				_tcout << endl << sLogBuf << endl;
				m_Ticket.m_OwnerID = 0;
			}
		}
	}

	wsprintf(sLogBuf,_T("Restoring TicketID: %d from ArchiveID: %d."),TicketID,nTicketArcID);
	_tcout << endl << sLogBuf << endl;

	try
	{
		//Restore Ticket
		if(bFoundTicketDB)
		{
			m_Ticket.Update(m_query);
		}
		else
		{
			m_query.Initialize();
			BINDPARAM_LONG( m_query, m_Ticket.m_TicketID );
			BINDPARAM_LONG( m_query, m_Ticket.m_TicketStateID );
			BINDPARAM_LONG( m_query, m_Ticket.m_TicketBoxID );
			BINDPARAM_LONG( m_query, m_Ticket.m_OwnerID );
			BINDPARAM_TCHAR( m_query, m_Ticket.m_Subject );
			BINDPARAM_TCHAR( m_query, m_Ticket.m_Contacts );
			BINDPARAM_LONG( m_query, m_Ticket.m_PriorityID );
			BINDPARAM_TIME( m_query, m_Ticket.m_DateCreated );
			BINDPARAM_TIME( m_query, m_Ticket.m_OpenTimestamp );
			BINDPARAM_LONG( m_query, m_Ticket.m_OpenMins );
			BINDPARAM_BIT( m_query, m_Ticket.m_AutoReplied );
			BINDPARAM_BIT( m_query, m_Ticket.m_AgeAlerted );
			BINDPARAM_TINYINT( m_query, m_Ticket.m_IsDeleted );
			BINDPARAM_TIME( m_query, m_Ticket.m_DeletedTime );
			BINDPARAM_LONG( m_query, m_Ticket.m_DeletedBy );
			BINDPARAM_LONG( m_query, m_Ticket.m_LockedBy );
			BINDPARAM_LONG( m_query, m_Ticket.m_UseTickler );
			BINDPARAM_TIME( m_query, m_Ticket.m_ReOpenTime );
			BINDPARAM_LONG( m_query, m_Ticket.m_FolderID );
			BINDPARAM_LONG( m_query, m_Ticket.m_TicketCategoryID );
			BINDPARAM_TIME( m_query, m_Ticket.m_LockedTime );		

			m_query.Execute( _T("SET IDENTITY_INSERT Tickets ON"));
			m_query.Execute( _T("INSERT INTO Tickets ")
									_T("(TicketID,TicketStateID,TicketBoxID,OwnerID,Subject,Contacts,PriorityID,DateCreated,OpenTimestamp,OpenMins,AutoReplied,AgeAlerted,IsDeleted,DeletedTime,DeletedBy,LockedBy,UseTickler,TicklerDateToReopen,FolderID,TicketCategoryID,LockedTime) ")
									_T("VALUES")
									_T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
			m_query.Execute( _T("SET IDENTITY_INSERT Tickets OFF"));
		}
	}
	catch(...)
	{
		_tcout << endl << _T("Failed to restore Ticket from Archive") << endl;
		g_logFile.Write(_T("Failed to restore Ticket from Archive"));
		g_logFile.LineBreak();
		g_logFile.Line();
		return 2;
	}

	wsprintf(sLogBuf,_T("Successfully restored TicketID: %d."),TicketID);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	THRestoreTicket( m_query, m_Ticket.m_TicketID, 0, m_Ticket.m_TicketStateID, m_Ticket.m_TicketBoxID, m_Ticket.m_OwnerID, m_Ticket.m_PriorityID, m_Ticket.m_TicketCategoryID );

	wsprintf(sLogBuf,_T("Restoring Ticket Objects for TicketID: %d."),TicketID);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	retVal = RestoreTicketObjectsFromArchive( m_query, nTicketArcID, TicketID );

	if(retVal != 0)
	{
		_tcout << endl << _T("Failed to restore Ticket Objects from Archive") << endl;
		g_logFile.Write(_T("Failed to restore Ticket Objects from Archive"));
		g_logFile.LineBreak();
		g_logFile.Line();
		return 2;
	}
	else
	{
		_tcout << endl << _T("Successfully restored Ticket Objects from Archive.") << endl;
		g_logFile.Write(_T("Successfully restored Ticket Objects from Archive."));
		g_logFile.LineBreak();
		g_logFile.Line();

	}

	return 0;
}