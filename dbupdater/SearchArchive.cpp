#include "prehead.h"
#include "main.h"
#include "ArchiveFns.h"
#include "TicketHistoryFns.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

typedef TCHAR String[256];
typedef const TCHAR* PCTSTR;

int SearchArchive(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
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
	
	int numFound = 0;
	int nArcID = 0;
		
	for( arcIter = m_arc.begin(); arcIter != m_arc.end(); arcIter++ )
	{
		numFound++;
		nArcID = arcIter->m_ArchiveID;
		CEMSString sDate;
		GetDateTimeString( arcIter->m_DateCreated, sizeof(arcIter->m_DateCreated), sDate, true, true );
        wsprintf(sLogBuf,_T("Found ArchiveID: %d Created on: %s."),arcIter->m_ArchiveID,sDate.c_str());
		_tcout << sLogBuf << endl;
		g_logFile.Write(sLogBuf);		
	}
	
	int retval = 0;
	if(numFound > 1)
	{
		nArcID = 0;
		do
		{
			wsprintf(sLogBuf,_T("Found multiple Archives, enter the ArchiveID you would like to search:"));
			_tcout << endl << sLogBuf;
			g_logFile.Write(sLogBuf);
			cin >> nArcID;
		}
		while( !cin.fail() && nArcID==0 );
		wsprintf(sLogBuf,_T("ArchiveID: %d Selected."),nArcID);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		
	}
	else if(numFound == 1)
	{
		wsprintf(sLogBuf,_T("ArchiveID: %d Found."),nArcID);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{
		wsprintf(sLogBuf,_T("No Archives Found."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	TIMESTAMP_STRUCT BeginDate;
	BeginDate.year = 2015;
	BeginDate.month = 9;
	BeginDate.day = 15;
	BeginDate.hour = 0;
	BeginDate.minute = 0;
	BeginDate.second = 0;
	BeginDate.fraction = 0;

	TIMESTAMP_STRUCT EndDate;
	EndDate.year = 2015;
	EndDate.month = 10;
	EndDate.day = 6;
	EndDate.hour = 0;
	EndDate.minute = 0;
	EndDate.second = 0;
	EndDate.fraction = 0;

	int nBeg = 0;
	int nEnd = 0;

	list<int> ticketidList;
	tstring sArchive;
	list<InboundMessages_t> msgList;
	list<InboundMessages_t>::iterator iter;
	retval = GetInboundMessageListFromArchive( m_query, nArcID, msgList, sArchive );
	if(retval == 0)
	{
		for( iter = msgList.begin(); iter != msgList.end(); iter++ )
		{
			
			nBeg = OrderTimeStamps(iter->m_DateReceived, BeginDate);
			nEnd = OrderTimeStamps(iter->m_DateReceived, EndDate);

			if(nBeg == 0 && nEnd == 1)
			{
				CEMSString sDate;
				GetDateTimeString( iter->m_DateReceived, sizeof(iter->m_DateReceived), sDate, true, true );
			
				wsprintf(sLogBuf,_T("TicketID: [%d] InboundMessageID: [%d] Received On: [%s] From: [%s]."),iter->m_TicketID,iter->m_InboundMessageID,sDate.c_str(),iter->m_EmailFrom);
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);

				ticketidList.push_back( iter->m_TicketID );
			}
		}
	}

	if(ticketidList.size() > 0)
	{
		ticketidList.sort();
		ticketidList.unique();
	}

	list<int>::iterator tidIter;
	for( tidIter = ticketidList.begin(); tidIter != ticketidList.end(); tidIter++ )
	{
		wsprintf(sLogBuf,_T("TicketID: [%d] will be restored."),*tidIter);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	return 0;
}