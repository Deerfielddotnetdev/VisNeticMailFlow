#include "prehead.h"
#include "main.h"

typedef TCHAR String[256];
typedef const TCHAR* PCTSTR;

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;

String      sLogBuf;
int			nMaxRecords;

int ConvertData(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	int			retVal = 0;
	nMaxRecords = -1;
	
	
	// Check to see if ConvertData table exists
	wsprintf(sLogBuf,_T("Checking for ConvertData Table."));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);
	
	int nTableID = 0;
	bool tableExists = false;				

	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nTableID );
	m_query.Execute( L"SELECT t.object_id FROM sysobjects o, sys.tables t WHERE o.id=t.object_id and o.name='ConvertData'" );
	if( m_query.Fetch() == S_OK )
	{
		wsprintf(sLogBuf,_T(""));
		_tcout << endl << sLogBuf;
		wsprintf(sLogBuf,_T("The ConvertData table exists? "));
		_tcout << endl << sLogBuf;
		g_logFile.Write(sLogBuf);
		wsprintf(sLogBuf,_T(""));
		_tcout << endl << sLogBuf;		
	
		tableExists = true;
	}

	if ( !tableExists )
	{
		int nTableID = 99;
		wsprintf(sLogBuf,_T(""));
		_tcout << endl << sLogBuf;
		wsprintf(sLogBuf,_T("0: No (Cancel)"));
		_tcout << endl << sLogBuf;
		g_logFile.Write(sLogBuf);
		wsprintf(sLogBuf,_T("1: Yes"));
		_tcout << endl << sLogBuf;
		g_logFile.Write(sLogBuf);
		wsprintf(sLogBuf,_T(""));
		_tcout << endl << sLogBuf;
					
		do
		{
			wsprintf(sLogBuf,_T("The ConvertData table does not exist, would you like to create it? "));
			_tcout << endl << sLogBuf;
			g_logFile.Write(sLogBuf);
			cin >> nTableID;
		}
		while( !cin.fail() && nTableID==99 );
		
		switch(nTableID)
		{
		case 0:
			wsprintf(sLogBuf,_T("No entered, exiting program."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
			break;
		case 1:
			wsprintf(sLogBuf,_T("Yes entered, creating ConvertData table."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);

			retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[ConvertData]( ")
			_T("[ConvertDataID] [int] IDENTITY(1,1) NOT NULL, ")
			_T("[DataTypeID] [int] NOT NULL CONSTRAINT [DF_ConvertData_DataTypeID] DEFAULT ((0)), ")
			_T("[ActualID] [int] NOT NULL CONSTRAINT [DF_ConvertData_ActualID] DEFAULT ((0)), ")
			_T("[DateConverted] [datetime] NOT NULL CONSTRAINT [DF_ConvertData_DateConverted]  DEFAULT (getdate()), ")
			_T("CONSTRAINT [PK_ConvertData] PRIMARY KEY CLUSTERED ([ConvertDataID])) "));

			if(!SQL_SUCCEEDED(retSQL))
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
			break;
		default:
			wsprintf(sLogBuf,_T("Invalid entry, exiting program."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
			break;
		}
	}	
	
	do
	{
		wsprintf(sLogBuf,_T("Enter the maximum number of each data type to convert: (0 = no limit) "));
		_tcout << endl << sLogBuf;
		g_logFile.Write(sLogBuf);
		cin >> nMaxRecords;
	}
	while( !cin.fail() && nMaxRecords==-1 );
	wsprintf(sLogBuf,_T("Max records set to: [%d]."), nMaxRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nDataID = 99;
	wsprintf(sLogBuf,_T(" 0: None (Cancel)"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);
	wsprintf(sLogBuf,_T(" 1: All"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);
	wsprintf(sLogBuf,_T(" 2: Tickets"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);
	wsprintf(sLogBuf,_T(" 3: Inbound Messages"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);
	wsprintf(sLogBuf,_T(" 4: Outbound Messages"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);
	wsprintf(sLogBuf,_T(" 5: Attachments"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);
	wsprintf(sLogBuf,_T(" 6: Contacts"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);
	wsprintf(sLogBuf,_T(" 7: Personal Data"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T(" 8: Agents"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T(" 9: Groups"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("10: Signatures"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("11: Routing Rules"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("12: Processing Rules"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("13: TicketBoxes"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("14: Ticket Categories"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("15: Folders"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("16: Standard Responses"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("17: Standard Response Categories"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("18: Contact Groups"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("19: Ticket Notes"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T("20: Contact Notes"));
	_tcout << endl << sLogBuf;
	g_logFile.Write(sLogBuf);	
	wsprintf(sLogBuf,_T(""));
	_tcout << endl << sLogBuf;
			
	do
	{
		wsprintf(sLogBuf,_T("Which data would you like to convert? "));
		_tcout << endl << sLogBuf;
		g_logFile.Write(sLogBuf);
		cin >> nDataID;
	}
	while( !cin.fail() && nDataID==99 );

	switch(nDataID)
	{
	case 0:
		wsprintf(sLogBuf,_T("None Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		break;
	case 1:
		wsprintf(sLogBuf,_T("All Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertTickets(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Tickets Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertOutboundMessages(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Outbound Messages Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertInboundMessages(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Inbound Messages Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertAttachments(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Attachments Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertContacts(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Contacts Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertPersonalData(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Personal Data Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertAgents(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Agents Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertGroups(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Groups Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertSignatures(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Signatures Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertRoutingRules(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Routing Rules Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertProcessingRules(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Processing Rules Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertTicketBoxes(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("TicketBoxes Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertTicketCategories(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Ticket Categories Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertFolders(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Folders Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertSRs(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Standard Responses Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertSRCats(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Standard Responses Categories Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertContactGroups(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Contact Groups Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertTicketNotes(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Ticket Notes Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		retVal = ConvertContactNotes(m_query);		
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Contact Notes Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 2:
		wsprintf(sLogBuf,_T("Tickets Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertTickets(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Tickets Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 3:
		wsprintf(sLogBuf,_T("Inbound Messages Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertInboundMessages(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Inbound Messages Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 4:
		wsprintf(sLogBuf,_T("Outbound Messages Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertOutboundMessages(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Outbound Messages Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 5:
		wsprintf(sLogBuf,_T("Attachments Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertAttachments(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Attachments Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 6:
		wsprintf(sLogBuf,_T("Contacts Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertContacts(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Contacts Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 7:
		wsprintf(sLogBuf,_T("Personal Data Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertPersonalData(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Personal Data Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 8:
		wsprintf(sLogBuf,_T("Agents Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertAgents(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Agents Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 9:
		wsprintf(sLogBuf,_T("Groups Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertGroups(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Groups Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 10:
		wsprintf(sLogBuf,_T("Signatures Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertSignatures(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Signatures Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 11:
		wsprintf(sLogBuf,_T("Routing Rules Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertRoutingRules(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Routing Rules Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 12:
		wsprintf(sLogBuf,_T("Processing Rules Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertProcessingRules(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Processing Rules Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 13:
		wsprintf(sLogBuf,_T("TicketBoxes Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertTicketBoxes(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Ticket Boxes Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 14:
		wsprintf(sLogBuf,_T("Ticket Categories Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertTicketCategories(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Ticket Categories Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 15:
		wsprintf(sLogBuf,_T("Folders Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertFolders(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Folders Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 16:
		wsprintf(sLogBuf,_T("Standar Responses Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertSRs(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Standard Responses Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 17:
		wsprintf(sLogBuf,_T("Standard Response Categories Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertSRCats(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Standard Response Categories Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 18:
		wsprintf(sLogBuf,_T("Contact Groups Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertContactGroups(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Contact Groups Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 19:
		wsprintf(sLogBuf,_T("Ticket Notes Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertTicketNotes(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Ticket Notes Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	case 20:
		wsprintf(sLogBuf,_T("Contact Notes Selected."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		retVal = ConvertContactNotes(m_query);
		if(retVal != 0)
		{
			wsprintf(sLogBuf,_T("Contact Notes Conversion Failed."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}
		break;
	default:
		wsprintf(sLogBuf,_T("Invalid entry, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
		break;
	}

	return 0;
}

void StringToUTF8(TCHAR* szIn, tstring& szOut)
{
	wstring sData;
	dca::String sTemp(szIn);
	int Len = sTemp.length() + 1;
	
	int nWcsSize = MultiByteToWideChar( CP_ACP, 0, sTemp.c_str(), Len, NULL, 0  );
	sData.resize( nWcsSize, L' ' );
	MultiByteToWideChar( CP_ACP, 0, sTemp.c_str(), Len, (LPWSTR)(sData.c_str()), nWcsSize  );

	int nChrSize = WideCharToMultiByte( CP_UTF8, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
	char* szBuff = new char[nChrSize+1];
	WideCharToMultiByte( CP_UTF8, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
	szBuff[nChrSize] = 0;
	
	USES_CONVERSION;
	
	szOut.assign(A2T(szBuff));

	delete[] szBuff;
}

int ConvertTickets(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertTickets."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM Tickets WHERE TicketID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=1)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Ticket records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Ticket records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Ticket count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	int nOrigMax = nMax;
	int nNumRecordsProcessed = 0;
	
	if(nMax > 1000)
	{
		nMax = 1000;
	}

	vector<TTickets> ticketList;
	vector<TTickets>::iterator iter;
	ticketList.reserve(ticketList.size()+nMax);

	while(nNumRecordsProcessed < nOrigMax)
	{
		ticketList.clear();
		TTickets m_tkt;		
			
		try
		{
			m_query.Initialize();
			CEMSString sQuery;
			sQuery.Format(_T("SELECT TOP %d TicketID,Subject,Contacts FROM Tickets WHERE TicketID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=1)"),nMax);
			BINDCOL_LONG( m_query, m_tkt.m_TicketID );
			BINDCOL_TCHAR( m_query, m_tkt.m_Subject );
			BINDCOL_TCHAR( m_query, m_tkt.m_Contacts );
			m_query.Execute(sQuery.c_str());
			while( m_query.Fetch() == S_OK )
			{
				
				try
				{
					ticketList.push_back(m_tkt);					
				}
				catch(...)
				{
					wsprintf(sLogBuf,_T("Failed to add TicketID: [%d] to ticketList."),m_tkt.m_TicketID);
					_tcout << endl << sLogBuf << endl;
					g_logFile.Write(sLogBuf);
					nNumRecordsProcessed++;
				}
			}
		}
		catch(ODBCError_t error) 
		{ 
			try
			{
				dca::String x(error.szErrMsg);
				wsprintf(sLogBuf,_T("ODBC Error: %s."),x.c_str());
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
			}
			catch(...)
			{
				wsprintf(sLogBuf,_T("Failed to query for Tickets."));
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
			}
			return 1;
		}	
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to query for Tickets."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}	

		for( iter = ticketList.begin(); iter != ticketList.end(); iter++ )
		{	
			try
			{
				tstring sSubject;
				StringToUTF8(iter->m_Subject,sSubject);
				if(sSubject.length() > 255)
				{
					sSubject = sSubject.substr(0,255);
				}

				tstring sContacts;
				StringToUTF8(iter->m_Contacts,sContacts);			
				if(sContacts.length() > 255)
				{
					sContacts = sContacts.substr(0,255);
				}

				m_query.Initialize();
				BINDPARAM_TCHAR_STRING( m_query, sSubject );
				BINDPARAM_TCHAR_STRING( m_query, sContacts );
				BINDPARAM_LONG( m_query, iter->m_TicketID );
			
				m_query.Execute( L"UPDATE Tickets SET Subject=?,Contacts=? WHERE TicketID=?");
				m_query.Reset(true);
				BINDPARAM_LONG( m_query, iter->m_TicketID );
				m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (1,?)");
				wsprintf(sLogBuf,_T("Successfully converted TicketID: [%d]."),iter->m_TicketID);
				_tcout << endl << sLogBuf << endl;
				nNumProcessed++;
				nNumRecordsProcessed++;
			}
			catch(ODBCError_t error) 
			{ 
				try
				{
					dca::String x(error.szErrMsg);
					wsprintf(sLogBuf,_T("ODBC Error: %s."),x.c_str());
					_tcout << endl << sLogBuf << endl;
					g_logFile.Write(sLogBuf);
				}
				catch(...)
				{
					wsprintf(sLogBuf,_T("Failed to convert TicketID: [%d]."),iter->m_TicketID);
					_tcout << endl << sLogBuf << endl;
					g_logFile.Write(sLogBuf);
				}
				return 1;
			}	
			catch(...)
			{
				wsprintf(sLogBuf,_T("Failed to convert TicketID: [%d]."),iter->m_TicketID);
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
				nNumRecordsProcessed++;
			}		
		}			
	}
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Ticket records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertTickets."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	vector<TTickets>().swap(ticketList);
	
	return 0;
}

int ConvertInboundMessages(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertInboundMessages."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM InboundMessages WHERE InboundMessageID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=2)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Inbound Message records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Inbound Message records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Inbound Message count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	int nOrigMax = nMax;
	int nNumRecordsProcessed = 0;
	
	if(nMax > 1000)
	{
		nMax = 1000;
	}

	vector<TInboundMessages> msgList;
	vector<TInboundMessages>::iterator iter;			
	msgList.reserve(msgList.size()+nMax);

	while(nNumRecordsProcessed < nOrigMax)
	{
		msgList.clear();
		TInboundMessages m_msg;			
			
		try
		{
			m_query.Initialize();
			CEMSString sQuery;
			sQuery.Format(_T("SELECT TOP %d InboundMessageID,EmailFromName,Subject,Body,EmailTo,EmailCc,EmailReplyTo FROM InboundMessages WHERE InboundMessageID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=2)"),nMax);
			BINDCOL_LONG( m_query, m_msg.m_InboundMessageID );
			BINDCOL_TCHAR( m_query, m_msg.m_EmailFromName );
			BINDCOL_TCHAR( m_query, m_msg.m_Subject );
			m_query.Execute(sQuery.c_str());
			while( m_query.Fetch() == S_OK )
			{
				GETDATA_TEXT(m_query, m_msg.m_Body);
				GETDATA_TEXT(m_query, m_msg.m_EmailTo);
				GETDATA_TEXT(m_query, m_msg.m_EmailCc);
				GETDATA_TEXT(m_query, m_msg.m_EmailReplyTo);		
				
				try
				{
					msgList.push_back(m_msg);	
				}
				catch(...)
				{
					wsprintf(sLogBuf,_T("Failed to add InboundMessageID: [%d] to msgList."),m_msg.m_InboundMessageID);
					_tcout << endl << sLogBuf << endl;
					g_logFile.Write(sLogBuf);
					nNumRecordsProcessed++;
				}
			}
		}
		catch(ODBCError_t error) 
		{ 
			try
			{
				dca::String x(error.szErrMsg);
				wsprintf(sLogBuf,_T("ODBC Error: %s."),x.c_str());
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
			}
			catch(...)
			{
				wsprintf(sLogBuf,_T("Failed to query for Inbound Messages."));
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
			}
			return 1;
		}	
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to query for Inbound Messages."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}	

		for( iter = msgList.begin(); iter != msgList.end(); iter++ )
		{	
			try
			{
				
				tstring sFromName;
				StringToUTF8(iter->m_EmailFromName,sFromName);
				if(sFromName.length() > 128)
				{
					sFromName = sFromName.substr(0,128);
				}			
				
				tstring sSubject;
				StringToUTF8(iter->m_Subject,sSubject);
				if(sSubject.length() > 255)
				{
					sSubject = sSubject.substr(0,255);
				}			
				
				tstring sBody;
				StringToUTF8(iter->m_Body,sBody);
				
				tstring sEmailTo;
				StringToUTF8(iter->m_EmailTo,sEmailTo);			
				
				tstring sEmailCc;
				StringToUTF8(iter->m_EmailCc,sEmailCc);
							
				tstring sEmailReplyTo;
				StringToUTF8(iter->m_EmailReplyTo,sEmailReplyTo);
				
				m_query.Initialize();
				BINDPARAM_TCHAR_STRING( m_query, sFromName );
				BINDPARAM_TCHAR_STRING( m_query, sSubject );
				BINDPARAM_TEXT_STRING( m_query, sBody );
				BINDPARAM_TEXT_STRING( m_query, sEmailTo );
				BINDPARAM_TEXT_STRING( m_query, sEmailCc );
				BINDPARAM_TEXT_STRING( m_query, sEmailReplyTo );
				BINDPARAM_LONG( m_query, iter->m_InboundMessageID );
			
				m_query.Execute( L"UPDATE InboundMessages SET EmailFromName=?,Subject=?,Body=?,EmailTo=?,EmailCc=?,EmailReplyTo=? WHERE InboundMessageID=?");
				m_query.Reset(true);
				BINDPARAM_LONG( m_query, iter->m_InboundMessageID );
				m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (2,?)");
				wsprintf(sLogBuf,_T("Successfully converted InboundMessageID: [%d]."),iter->m_InboundMessageID);
				_tcout << endl << sLogBuf << endl;
				nNumProcessed++;
				nNumRecordsProcessed++;
			}
			catch(ODBCError_t error) 
			{ 
				try
				{
					dca::String x(error.szErrMsg);
					wsprintf(sLogBuf,_T("ODBC Error: %s."),x.c_str());
					_tcout << endl << sLogBuf << endl;
					g_logFile.Write(sLogBuf);
				}
				catch(...)
				{
					wsprintf(sLogBuf,_T("Failed to convert InboundMessageID: [%d]."),iter->m_InboundMessageID);
					_tcout << endl << sLogBuf << endl;
					g_logFile.Write(sLogBuf);
				}
				return 1;
			}	
			catch(...)
			{
				wsprintf(sLogBuf,_T("Failed to convert InboundMessageID: [%d]."),iter->m_InboundMessageID);
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
				nNumRecordsProcessed++;
			}		
		}			
	}
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Inbound Message records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertInboundMessages."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	vector<TInboundMessages>().swap(msgList);
	
	return 0;
}

int ConvertOutboundMessages(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertOutboundMessages."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM OutboundMessages WHERE OutboundMessageID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=3)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Outbound Message records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Outbound Message records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Outbound Message count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	int nOrigMax = nMax;
	int nNumRecordsProcessed = 0;
	
	if(nMax > 1000)
	{
		nMax = 1000;
	}

	vector<TOutboundMessages> outmsgList;
	vector<TOutboundMessages>::iterator iter;			
	outmsgList.reserve(outmsgList.size()+nMax);

	while(nNumRecordsProcessed < nOrigMax)
	{
		outmsgList.clear();
		TOutboundMessages m_outmsg;
		
		try
		{
			m_query.Initialize();
			CEMSString sQuery;
			sQuery.Format(_T("SELECT TOP %d OutboundMessageID,EmailFrom,EmailPrimaryTo,Subject,Body,EmailTo,EmailCc,EmailBcc,EmailReplyTo FROM OutboundMessages WHERE OutboundMessageID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=3)"),nMax);
			BINDCOL_LONG( m_query, m_outmsg.m_OutboundMessageID );
			BINDCOL_TCHAR( m_query, m_outmsg.m_EmailFrom );
			BINDCOL_TCHAR( m_query, m_outmsg.m_EmailPrimaryTo );
			BINDCOL_TCHAR( m_query, m_outmsg.m_Subject );
			m_query.Execute(sQuery.c_str());
			while( m_query.Fetch() == S_OK )
			{
				GETDATA_TEXT(m_query, m_outmsg.m_Body);
				GETDATA_TEXT(m_query, m_outmsg.m_EmailTo);
				GETDATA_TEXT(m_query, m_outmsg.m_EmailCc);
				GETDATA_TEXT(m_query, m_outmsg.m_EmailBcc);
				GETDATA_TEXT(m_query, m_outmsg.m_EmailReplyTo);		
				
				try
				{
					outmsgList.push_back(m_outmsg);	
				}
				catch(...)
				{
					wsprintf(sLogBuf,_T("Failed to add OutboundMessageID: [%d] to msgList."),m_outmsg.m_OutboundMessageID);
					_tcout << endl << sLogBuf << endl;
					g_logFile.Write(sLogBuf);
					nNumRecordsProcessed++;
				}	
			}
		}
		catch(ODBCError_t error) 
		{ 
			try
			{
				dca::String x(error.szErrMsg);
				wsprintf(sLogBuf,_T("ODBC Error: %s."),x.c_str());
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
			}
			catch(...)
			{
				wsprintf(sLogBuf,_T("Failed to query for Outbound Messages."));
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
			}
			return 1;
		}	
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to query for Outbound Messages."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 1;
		}	

		for( iter = outmsgList.begin(); iter != outmsgList.end(); iter++ )
		{	
			try
			{
				
				tstring sFrom;
				StringToUTF8(iter->m_EmailFrom,sFrom);
				if(sFrom.length() > 255)
				{
					sFrom = sFrom.substr(0,255);
				}
							
				tstring sPrimaryTo;
				StringToUTF8(iter->m_EmailPrimaryTo,sPrimaryTo);
				if(sPrimaryTo.length() > 255)
				{
					sPrimaryTo = sPrimaryTo.substr(0,255);
				}
							
							
				tstring sSubject;
				StringToUTF8(iter->m_Subject,sSubject);
				if(sSubject.length() > 255)
				{
					sSubject = sSubject.substr(0,255);
				}
							
				

				tstring sBody;
				StringToUTF8(iter->m_Body,sBody);
				
				tstring sEmailTo;
				StringToUTF8(iter->m_EmailTo,sEmailTo);			
				
				tstring sEmailCc;
				StringToUTF8(iter->m_EmailCc,sEmailCc);
							
				tstring sEmailBcc;
				StringToUTF8(iter->m_EmailBcc,sEmailBcc);
							
				tstring sEmailReplyTo;
				StringToUTF8(iter->m_EmailReplyTo,sEmailReplyTo);
				
				m_query.Initialize();
				BINDPARAM_TCHAR_STRING( m_query, sFrom );
				BINDPARAM_TCHAR_STRING( m_query, sPrimaryTo );
				BINDPARAM_TCHAR_STRING( m_query, sSubject );
				BINDPARAM_TEXT_STRING( m_query, sBody );
				BINDPARAM_TEXT_STRING( m_query, sEmailTo );
				BINDPARAM_TEXT_STRING( m_query, sEmailCc );
				BINDPARAM_TEXT_STRING( m_query, sEmailBcc );
				BINDPARAM_TEXT_STRING( m_query, sEmailReplyTo );
				BINDPARAM_LONG( m_query, iter->m_OutboundMessageID );
			
				m_query.Execute( L"UPDATE OutboundMessages SET EmailFrom=?,EmailPrimaryTo=?,Subject=?,Body=?,EmailTo=?,EmailCc=?,EmailBcc=?,EmailReplyTo=? WHERE OutboundMessageID=?");
				m_query.Reset(true);
				BINDPARAM_LONG( m_query, iter->m_OutboundMessageID );
				m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (3,?)");
				wsprintf(sLogBuf,_T("Successfully converted OutboundMessageID: [%d]."),iter->m_OutboundMessageID);
				_tcout << endl << sLogBuf << endl;
				nNumProcessed++;
				nNumRecordsProcessed++;
			}
			catch(ODBCError_t error) 
			{ 
				try
				{
					tstring x(error.szErrMsg);
					wsprintf(sLogBuf,_T("Failed to convert OutboundMessageID: [%d] ODBC Error: [%s]."),iter->m_OutboundMessageID, x.c_str());
					_tcout << endl << sLogBuf << endl;
					g_logFile.Write(sLogBuf);
				}
				catch(...)
				{
					wsprintf(sLogBuf,_T("Failed to convert OutboundMessageID: [%d]."),iter->m_OutboundMessageID);
					_tcout << endl << sLogBuf << endl;
					g_logFile.Write(sLogBuf);
				}
				return 1;
			}	
			catch(...)
			{
				wsprintf(sLogBuf,_T("Failed to convert OutboundMessageID: [%d]."),iter->m_OutboundMessageID);
				_tcout << endl << sLogBuf << endl;
				g_logFile.Write(sLogBuf);
				nNumRecordsProcessed++;
			}		
		}			
	}
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Outbound Message records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertOutboundMessages."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	vector<TOutboundMessages>().swap(outmsgList);

	return 0;
}

int ConvertAttachments(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertAttachments."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM Attachments WHERE AttachmentID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=4)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Attachment records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Attachment records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Attachment count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TAttachments> attList;
	vector<TAttachments>::iterator iter;
	TAttachments m_att;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d AttachmentID,FileName FROM Attachments WHERE AttachmentID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=4)"),nMax);
	BINDCOL_LONG( m_query, m_att.m_AttachmentID );
	BINDCOL_TCHAR( m_query, m_att.m_FileName );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		attList.push_back(m_att);	
	}

	for( iter = attList.begin(); iter != attList.end(); iter++ )
	{	
		try
		{
			
			tstring sFileName;
			StringToUTF8(iter->m_FileName,sFileName);
						
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sFileName );
			BINDPARAM_LONG( m_query, iter->m_AttachmentID );
		
			m_query.Execute( L"UPDATE Attachments SET FileName=? WHERE AttachmentID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_AttachmentID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (4,?)");
			wsprintf(sLogBuf,_T("Successfully converted AttachmentID: [%d]."),iter->m_AttachmentID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert AttachmentID: [%d]."),iter->m_AttachmentID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Attachment records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertAttachments."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertContacts(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertContacts."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM Contacts WHERE ContactID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=5)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Contact records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Contact records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Contact count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TContacts> cList;
	vector<TContacts>::iterator iter;
	TContacts m_c;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d ContactID,Name,CompanyName,Title,StreetAddress1,StreetAddress2,City,State,Country FROM Contacts WHERE ContactID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=5)"),nMax);
	BINDCOL_LONG( m_query, m_c.m_ContactID );
	BINDCOL_TCHAR( m_query, m_c.m_Name );	
	BINDCOL_TCHAR( m_query, m_c.m_CompanyName );	
	BINDCOL_TCHAR( m_query, m_c.m_Title );	
	BINDCOL_TCHAR( m_query, m_c.m_StreetAddress1 );	
	BINDCOL_TCHAR( m_query, m_c.m_StreetAddress2 );	
	BINDCOL_TCHAR( m_query, m_c.m_City );	
	BINDCOL_TCHAR( m_query, m_c.m_State );	
	BINDCOL_TCHAR( m_query, m_c.m_Country );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		cList.push_back(m_c);	
	}

	for( iter = cList.begin(); iter != cList.end(); iter++ )
	{	
		try
		{
			
			tstring sName;
			StringToUTF8(iter->m_Name,sName);
			
			tstring sCompanyName;
			StringToUTF8(iter->m_CompanyName,sCompanyName);
			
			tstring sTitle;
			StringToUTF8(iter->m_Title,sTitle);
			
			tstring sStreetAddress1;
			StringToUTF8(iter->m_StreetAddress1,sStreetAddress1);
			
			tstring sStreetAddress2;
			StringToUTF8(iter->m_StreetAddress2,sStreetAddress2);
			
			tstring sCity;
			StringToUTF8(iter->m_City,sCity);
			
			tstring sState;
			StringToUTF8(iter->m_State,sState);
			
			tstring sCountry;
			StringToUTF8(iter->m_Country,sCountry);

			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sName );
			BINDPARAM_TCHAR_STRING( m_query, sCompanyName );
			BINDPARAM_TCHAR_STRING( m_query, sTitle );
			BINDPARAM_TCHAR_STRING( m_query, sStreetAddress1 );
			BINDPARAM_TCHAR_STRING( m_query, sStreetAddress2 );
			BINDPARAM_TCHAR_STRING( m_query, sCity );
			BINDPARAM_TCHAR_STRING( m_query, sState );
			BINDPARAM_TCHAR_STRING( m_query, sCountry );
			BINDPARAM_LONG( m_query, iter->m_ContactID );
		
			m_query.Execute( L"UPDATE Contacts SET Name=?,CompanyName=?,Title=?,StreetAddress1=?,StreetAddress2=?,City=?,State=?,Country=? WHERE ContactID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_ContactID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (5,?)");
			wsprintf(sLogBuf,_T("Successfully converted ContactID: [%d]."),iter->m_ContactID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert ContactID: [%d]."),iter->m_ContactID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Contact records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertContacts."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertPersonalData(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertPersonalData."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM PersonalData WHERE PersonalDataID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=6)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No PersonalData records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] PersonalData records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain PersonalData count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TPersonalData> pdList;
	vector<TPersonalData>::iterator iter;
	TPersonalData m_pd;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d PersonalDataID,DataValue FROM PersonalData WHERE PersonalDataID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=6)"),nMax);
	BINDCOL_LONG( m_query, m_pd.m_PersonalDataID );
	BINDCOL_TCHAR( m_query, m_pd.m_DataValue );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		pdList.push_back(m_pd);	
	}

	for( iter = pdList.begin(); iter != pdList.end(); iter++ )
	{	
		try
		{
			
			tstring sDataValue;
			StringToUTF8(iter->m_DataValue,sDataValue);
						
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sDataValue );
			BINDPARAM_LONG( m_query, iter->m_PersonalDataID );
		
			m_query.Execute( L"UPDATE PersonalData SET DataValue=? WHERE PersonalDataID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_PersonalDataID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (6,?)");
			wsprintf(sLogBuf,_T("Successfully converted PersonalDataID: [%d]."),iter->m_PersonalDataID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert PersonalDataID: [%d]."),iter->m_PersonalDataID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] PersonalData records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertPersonalData."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertAgents(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertAgents."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM Agents WHERE AgentID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=7)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Agent records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Agent records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Agent count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TAgents> dataList;
	vector<TAgents>::iterator iter;
	TAgents data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d AgentID,Name,StatusText,OnlineText,AwayText,NotAvailText,DndText,OfflineText,OooText FROM Agents WHERE AgentID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=7)"),nMax);
	BINDCOL_LONG( m_query, data.m_AgentID );
	BINDCOL_TCHAR( m_query, data.m_Name );	
	BINDCOL_TCHAR( m_query, data.m_StatusText );	
	BINDCOL_TCHAR( m_query, data.m_OnlineText );	
	BINDCOL_TCHAR( m_query, data.m_AwayText );	
	BINDCOL_TCHAR( m_query, data.m_NotAvailText );	
	BINDCOL_TCHAR( m_query, data.m_DndText );	
	BINDCOL_TCHAR( m_query, data.m_OfflineText );	
	BINDCOL_TCHAR( m_query, data.m_OooText );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sName;
			StringToUTF8(iter->m_Name,sName);
			
			tstring sStatusText;
			StringToUTF8(iter->m_StatusText,sStatusText);
			
			tstring sOnlineText;
			StringToUTF8(iter->m_OnlineText,sOnlineText);
			
			tstring sAwayText;
			StringToUTF8(iter->m_AwayText,sAwayText);
			
			tstring sNotAvailText;
			StringToUTF8(iter->m_NotAvailText,sNotAvailText);
			
			tstring sDndText;
			StringToUTF8(iter->m_DndText,sDndText);
			
			tstring sOfflineText;
			StringToUTF8(iter->m_OfflineText,sOfflineText);
			
			tstring sOooText;
			StringToUTF8(iter->m_OooText,sOooText);

			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sName );
			BINDPARAM_TCHAR_STRING( m_query, sStatusText );
			BINDPARAM_TCHAR_STRING( m_query, sOnlineText );
			BINDPARAM_TCHAR_STRING( m_query, sAwayText );
			BINDPARAM_TCHAR_STRING( m_query, sNotAvailText );
			BINDPARAM_TCHAR_STRING( m_query, sDndText );
			BINDPARAM_TCHAR_STRING( m_query, sOfflineText );
			BINDPARAM_TCHAR_STRING( m_query, sOooText );
			BINDPARAM_LONG( m_query, iter->m_AgentID );
		
			m_query.Execute( L"UPDATE Agents SET Name=?,StatusText=?,OnlineText=?,AwayText=?,NotAvailText=?,DndText=?,OfflineText=?,OooText=? WHERE AgentID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_AgentID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (7,?)");
			wsprintf(sLogBuf,_T("Successfully converted AgentID: [%d]."),iter->m_AgentID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert AgentID: [%d]."),iter->m_AgentID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Agent records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertAgents."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertGroups(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertGroups."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM Groups WHERE GroupID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=8)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Group records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Group records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Group count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TGroups> dataList;
	vector<TGroups>::iterator iter;
	TGroups data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d GroupID,GroupName FROM Groups WHERE GroupID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=8)"),nMax);
	BINDCOL_LONG( m_query, data.m_GroupID );
	BINDCOL_TCHAR( m_query, data.m_GroupName );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sGroupName;
			StringToUTF8(iter->m_GroupName,sGroupName);
			
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sGroupName );
			BINDPARAM_LONG( m_query, iter->m_GroupID );
		
			m_query.Execute( L"UPDATE Groups SET GroupName=? WHERE GroupID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_GroupID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (8,?)");
			wsprintf(sLogBuf,_T("Successfully converted GroupID: [%d]."),iter->m_GroupID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert GroupID: [%d]."),iter->m_GroupID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Group records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertGroups."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertSignatures(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertSignatures."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM Signatures WHERE SignatureID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=9)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Signature records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Signature records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Signature count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TSignatures> dataList;
	vector<TSignatures>::iterator iter;
	TSignatures data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d SignatureID,Name,Signature FROM Signatures WHERE SignatureID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=9)"),nMax);
	BINDCOL_LONG( m_query, data.m_SignatureID );
	BINDCOL_TCHAR( m_query, data.m_Name );
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		GETDATA_TEXT(m_query, data.m_Signature);
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sName;
			StringToUTF8(iter->m_Name,sName);
						
			tstring sSig;
			StringToUTF8(iter->m_Signature,sSig);			

			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sName );
			BINDPARAM_TEXT_STRING( m_query, sSig );
			BINDPARAM_LONG( m_query, iter->m_SignatureID );
		
			m_query.Execute( L"UPDATE Signatures SET Name=?,Signature=? WHERE SignatureID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_SignatureID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (9,?)");
			wsprintf(sLogBuf,_T("Successfully converted SignatureID: [%d]."),iter->m_SignatureID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert SignatureID: [%d]."),iter->m_SignatureID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Signature records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertSignatures."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	return 0;
}

int ConvertRoutingRules(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertRoutingRules."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM RoutingRules WHERE RoutingRuleID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=10)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Routing Rule records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Routing Rule records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Routing Rule count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TRoutingRules> dataList;
	vector<TRoutingRules>::iterator iter;
	TRoutingRules data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d RoutingRuleID,RuleDescrip FROM RoutingRules WHERE RoutingRuleID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=10)"),nMax);
	BINDCOL_LONG( m_query, data.m_RoutingRuleID );
	BINDCOL_TCHAR( m_query, data.m_RuleDescrip );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sRuleDescrip;
			StringToUTF8(iter->m_RuleDescrip,sRuleDescrip);
			
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sRuleDescrip );
			BINDPARAM_LONG( m_query, iter->m_RoutingRuleID );
		
			m_query.Execute( L"UPDATE RoutingRules SET RuleDescrip=? WHERE RoutingRuleID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_RoutingRuleID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (10,?)");
			wsprintf(sLogBuf,_T("Successfully converted RoutingRuleID: [%d]."),iter->m_RoutingRuleID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert RoutingRuleID: [%d]."),iter->m_RoutingRuleID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Routing Rule records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertRoutingRules."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertProcessingRules(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertProcessingRules."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM ProcessingRules WHERE ProcessingRuleID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=11)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Processing Rule records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Processing Rule records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Processing Rule count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TProcessingRules> dataList;
	vector<TProcessingRules>::iterator iter;
	TProcessingRules data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d ProcessingRuleID,RuleDescrip FROM ProcessingRules WHERE ProcessingRuleID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=11)"),nMax);
	BINDCOL_LONG( m_query, data.m_ProcessingRuleID );
	BINDCOL_TCHAR( m_query, data.m_RuleDescrip );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sRuleDescrip;
			StringToUTF8(iter->m_RuleDescrip,sRuleDescrip);
			
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sRuleDescrip );
			BINDPARAM_LONG( m_query, iter->m_ProcessingRuleID );
		
			m_query.Execute( L"UPDATE ProcessingRules SET RuleDescrip=? WHERE ProcessingRuleID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_ProcessingRuleID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (11,?)");
			wsprintf(sLogBuf,_T("Successfully converted ProcessingRuleID: [%d]."),iter->m_ProcessingRuleID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert ProcessingRuleID: [%d]."),iter->m_ProcessingRuleID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Processing Rule records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertProcessingRules."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertTicketBoxes(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertTicketBoxes."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM TicketBoxes WHERE TicketBoxID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=12)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No TicketBox records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] TicketBox records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain TicketBox count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TTicketBoxes> dataList;
	vector<TTicketBoxes>::iterator iter;
	TTicketBoxes data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d TicketBoxID,Name,Description,DefaultEmailAddressName FROM TicketBoxes WHERE TicketBoxID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=12)"),nMax);
	BINDCOL_LONG( m_query, data.m_TicketBoxID );
	BINDCOL_TCHAR( m_query, data.m_Name );	
	BINDCOL_TCHAR( m_query, data.m_Description );	
	BINDCOL_TCHAR( m_query, data.m_DefaultEmailAddressName );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sName;
			StringToUTF8(iter->m_Name,sName);
			
			tstring sDescription;
			StringToUTF8(iter->m_Description,sDescription);
			
			tstring sDefaultEmailAddressName;
			StringToUTF8(iter->m_DefaultEmailAddressName,sDefaultEmailAddressName);
			
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sName );
			BINDPARAM_TCHAR_STRING( m_query, sDescription );
			BINDPARAM_TCHAR_STRING( m_query, sDefaultEmailAddressName );
			BINDPARAM_LONG( m_query, iter->m_TicketBoxID );
		
			m_query.Execute( L"UPDATE TicketBoxes SET Name=?,Description=?,DefaultEmailAddressName=? WHERE TicketBoxID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_TicketBoxID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (12,?)");
			wsprintf(sLogBuf,_T("Successfully converted TicketBoxID: [%d]."),iter->m_TicketBoxID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert TicketBoxID: [%d]."),iter->m_TicketBoxID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] TicketBox records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertTicketBoxes."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertTicketCategories(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertTicketCategories."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM TicketCategories WHERE TicketCategoryID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=13)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Ticket Category records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Ticket Category records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Ticket Category count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TTicketCategories> dataList;
	vector<TTicketCategories>::iterator iter;
	TTicketCategories data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d TicketCategoryID,Description FROM TicketCategories WHERE TicketCategoryID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=13)"),nMax);
	BINDCOL_LONG( m_query, data.m_TicketCategoryID );
	BINDCOL_TCHAR( m_query, data.m_Description );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sDescription;
			StringToUTF8(iter->m_Description,sDescription);
			
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sDescription );
			BINDPARAM_LONG( m_query, iter->m_TicketCategoryID );
		
			m_query.Execute( L"UPDATE TicketCategories SET Description=? WHERE TicketCategoryID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_TicketCategoryID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (13,?)");
			wsprintf(sLogBuf,_T("Successfully converted TicketCategoryID: [%d]."),iter->m_TicketCategoryID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert TicketCategoryID: [%d]."),iter->m_TicketCategoryID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Ticket Category records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertTicketCategories."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertFolders(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertFolders."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM Folders WHERE FolderID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=14)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Folder records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Folder records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Folder count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TFolders> dataList;
	vector<TFolders>::iterator iter;
	TFolders data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d FolderID,Name FROM Folders WHERE FolderID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=14)"),nMax);
	BINDCOL_LONG( m_query, data.m_FolderID );
	BINDCOL_TCHAR( m_query, data.m_Name );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sName;
			StringToUTF8(iter->m_Name, sName);
			
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sName );
			BINDPARAM_LONG( m_query, iter->m_FolderID );
		
			m_query.Execute( L"UPDATE Folders SET Name=? WHERE FolderID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_FolderID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (14,?)");
			wsprintf(sLogBuf,_T("Successfully converted FolderID: [%d]."),iter->m_FolderID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert FolderID: [%d]."),iter->m_FolderID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Folder records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertFolders."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertSRs(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertSRs."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM StandardResponses WHERE StandardResponseID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=15)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Standard Response records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Standard Response records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Standard Response count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TStandardResponses> dataList;
	vector<TStandardResponses>::iterator iter;
	TStandardResponses data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d StandardResponseID,Subject,KeyWords,StandardResponse,Note FROM StandardResponses WHERE StandardResponseID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=15)"),nMax);
	BINDCOL_LONG( m_query, data.m_StandardResponseID );
	BINDCOL_TCHAR( m_query, data.m_Subject );
	BINDCOL_TCHAR( m_query, data.m_Keywords );
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		GETDATA_TEXT(m_query, data.m_StandardResponse);
		GETDATA_TEXT(m_query, data.m_Note);
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sSubject;
			StringToUTF8(iter->m_Subject,sSubject);
						
			tstring sKeywords;
			StringToUTF8(iter->m_Keywords,sKeywords);
						
			tstring sSR;
			StringToUTF8(iter->m_StandardResponse,sSR);			

			tstring sNote;
			StringToUTF8(iter->m_Note,sNote);			

			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sSubject );
			BINDPARAM_TCHAR_STRING( m_query, sKeywords );
			BINDPARAM_TEXT_STRING( m_query, sSR );
			BINDPARAM_TEXT_STRING( m_query, sNote );
			BINDPARAM_LONG( m_query, iter->m_StandardResponseID );
		
			m_query.Execute( L"UPDATE StandardResponses SET Subject=?,Keywords=?,StandardResponse=?,Note=? WHERE StandardResponseID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_StandardResponseID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (15,?)");
			wsprintf(sLogBuf,_T("Successfully converted StandardResponseID: [%d]."),iter->m_StandardResponseID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert StandardResponseID: [%d]."),iter->m_StandardResponseID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Standard Response records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertSRs."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	return 0;
}

int ConvertSRCats(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertSRCats."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM StdResponseCategories WHERE StdResponseCatID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=16)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Standard Response Category records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Standard Response Category records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Standard Response Category count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TStdResponseCategories> dataList;
	vector<TStdResponseCategories>::iterator iter;
	TStdResponseCategories data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d StdResponseCatID,CategoryName FROM StdResponseCategories WHERE StdResponseCatID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=16)"),nMax);
	BINDCOL_LONG( m_query, data.m_StdResponseCatID );
	BINDCOL_TCHAR( m_query, data.m_CategoryName );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sName;
			StringToUTF8(iter->m_CategoryName, sName);
			
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sName );
			BINDPARAM_LONG( m_query, iter->m_StdResponseCatID );
		
			m_query.Execute( L"UPDATE StdResponseCategories SET CategoryName=? WHERE StdResponseCatID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_StdResponseCatID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (16,?)");
			wsprintf(sLogBuf,_T("Successfully converted StdResponseCatID: [%d]."),iter->m_StdResponseCatID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert StdResponseCatID: [%d]."),iter->m_StdResponseCatID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Standard Response Category records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertSRCats."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertContactGroups(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertContactGroups."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM ContactGroups WHERE ContactGroupID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=17)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Contact Group records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Contact Group records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Contact Group count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TContactGroups> dataList;
	vector<TContactGroups>::iterator iter;
	TContactGroups data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d ContactGroupID,GroupName FROM ContactGroups WHERE ContactGroupID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=17)"),nMax);
	BINDCOL_LONG( m_query, data.m_ContactGroupID );
	BINDCOL_TCHAR( m_query, data.m_GroupName );	
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sName;
			StringToUTF8(iter->m_GroupName, sName);
			
			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sName );
			BINDPARAM_LONG( m_query, iter->m_ContactGroupID );
		
			m_query.Execute( L"UPDATE ContactGroups SET GroupName=? WHERE ContactGroupID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_ContactGroupID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (17,?)");
			wsprintf(sLogBuf,_T("Successfully converted ContactGroupID: [%d]."),iter->m_ContactGroupID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert ContactGroupID: [%d]."),iter->m_ContactGroupID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Contact Group records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertContactGroups."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	return 0;
}

int ConvertTicketNotes(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertTicketNotes."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM TicketNotes WHERE TicketNoteID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=18)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Ticket Note records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Ticket Note records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Ticket Note count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TTicketNotes> dataList;
	vector<TTicketNotes>::iterator iter;
	TTicketNotes data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d TicketNoteID,Note FROM TicketNotes WHERE TicketNoteID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=18)"),nMax);
	BINDCOL_LONG( m_query, data.m_TicketNoteID );
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		GETDATA_TEXT(m_query, data.m_Note);
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sNote;
			StringToUTF8(iter->m_Note,sNote);
						
			m_query.Initialize();
			BINDPARAM_TEXT_STRING( m_query, sNote );
			BINDPARAM_LONG( m_query, iter->m_TicketNoteID );
		
			m_query.Execute( L"UPDATE TicketNotes SET Note=? WHERE TicketNoteID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_TicketNoteID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (18,?)");
			wsprintf(sLogBuf,_T("Successfully converted TicketNoteID: [%d]."),iter->m_TicketNoteID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert TicketNoteID: [%d]."),iter->m_TicketNoteID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}		
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Ticket Note records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertTicketNotes."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	return 0;
}

int ConvertContactNotes(CODBCQuery& m_query)
{
	wsprintf(sLogBuf,_T("Entered ConvertContactNotes."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	int nMax = 0;
	int nNumRecords = 0;
	int nNumProcessed = 0;
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nNumRecords );
	m_query.Execute( L"SELECT Count(*) FROM ContactNotes WHERE ContactNoteID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=19)" );
	if( m_query.Fetch() == S_OK )
	{
		if(nNumRecords == 0)
		{
			wsprintf(sLogBuf,_T("No Contact Note records to convert."));
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
			return 0;
		}
		if(nMaxRecords <= 0 || nMaxRecords > nNumRecords){nMax = nNumRecords;}else{nMax = nMaxRecords;}
		wsprintf(sLogBuf,_T("Converting [%d] of [%d] Contact Note records."),nMax,nNumRecords);
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
	}
	else
	{	
		wsprintf(sLogBuf,_T("Failed to obtain Contact Note count, exiting program."));
		_tcout << endl << sLogBuf << endl;
		g_logFile.Write(sLogBuf);
		return 1;
	}

	vector<TContactNotes> dataList;
	vector<TContactNotes>::iterator iter;
	TContactNotes data;
	
	m_query.Initialize();
	CEMSString sQuery;
	sQuery.Format(_T("SELECT TOP %d ContactNoteID,Subject,Note FROM ContactNotes WHERE ContactNoteID NOT IN (SELECT ActualID FROM ConvertData WHERE DataTypeID=19)"),nMax);
	BINDCOL_LONG( m_query, data.m_ContactNoteID );
	BINDCOL_TCHAR( m_query, data.m_Subject );
	m_query.Execute(sQuery.c_str());
	while( m_query.Fetch() == S_OK )
	{
		GETDATA_TEXT(m_query, data.m_Note);
		dataList.push_back(data);	
	}

	for( iter = dataList.begin(); iter != dataList.end(); iter++ )
	{	
		try
		{
			
			tstring sSubject;
			StringToUTF8(iter->m_Subject,sSubject);
						
			tstring sNote;
			StringToUTF8(iter->m_Note,sNote);			

			m_query.Initialize();
			BINDPARAM_TCHAR_STRING( m_query, sSubject );
			BINDPARAM_TEXT_STRING( m_query, sNote );
			BINDPARAM_LONG( m_query, iter->m_ContactNoteID );
		
			m_query.Execute( L"UPDATE ContactNotes SET Subject=?,Note=? WHERE ContactNoteID=?");
			m_query.Reset(true);
			BINDPARAM_LONG( m_query, iter->m_ContactNoteID );
			m_query.Execute( L"INSERT INTO ConvertData (DataTypeID,ActualID) VALUES (19,?)");
			wsprintf(sLogBuf,_T("Successfully converted ContactNoteID: [%d]."),iter->m_ContactNoteID);
			_tcout << endl << sLogBuf << endl;
			nNumProcessed++;
		}
		catch(...)
		{
			wsprintf(sLogBuf,_T("Failed to convert ContactNoteID: [%d]."),iter->m_ContactNoteID);
			_tcout << endl << sLogBuf << endl;
			g_logFile.Write(sLogBuf);
		}
	}			
	
	wsprintf(sLogBuf,_T("Converted [%d] of [%d] Contact Note records."),nNumProcessed,nNumRecords);
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);

	wsprintf(sLogBuf,_T("Leaving ConvertContactNotes."));
	_tcout << endl << sLogBuf << endl;
	g_logFile.Write(sLogBuf);
	
	return 0;
}

