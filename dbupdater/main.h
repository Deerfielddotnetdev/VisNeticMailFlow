#if !defined MAIN_H
#define MAIN_H

using namespace std;

//*****************************************************************************
// Definitions
#ifdef _UNICODE
#define _tcout wcout
#else
#define _tcout cout
#endif
//*****************************************************************************

class XmlFile
{
public:
	XmlFile() 
	{ 
		char tempPath[255];

		//GetTempPath(255,tempPath);
		strcpy(tempPath,"MailFlow-schema.xml");
		_xmlFile.open(tempPath,ios::out | ios::app);
	}
	~XmlFile() { _xmlFile.close(); }
	void Date()
	{
		SYSTEMTIME sysTime;
		TCHAR buf[256];

		GetLocalTime(&sysTime);

		wsprintf(buf, _T("%02d/%02d/%d - %02d:%02d:%02d"),sysTime.wMonth,sysTime.wDay,sysTime.wYear,
		         sysTime.wHour,sysTime.wMinute,sysTime.wSecond);

		dca::String c(buf);
	
		_xmlFile << c.c_str() << endl;
	}
	void Write(const TCHAR* txt)
	{
		dca::String c(txt);

		_xmlFile << c.c_str() << endl;
	}
	void LineBreak()
	{
		_xmlFile << endl;
	}
	void Line()
	{
		for(int i = 0;i < 50;i++)
		{
			_xmlFile << "*";
		}
		_xmlFile << endl << endl;
	}
private:
	ofstream _xmlFile;
};

class LogFile
{
public:
	LogFile() 
	{ 
		char tempPath[255];

		//GetTempPath(255,tempPath);
		strcpy(tempPath,"dbupdater.log");
		_logFile.open(tempPath,ios::out | ios::app);
	}
	~LogFile() { _logFile.close(); }
	void Date()
	{
		SYSTEMTIME sysTime;
		TCHAR buf[256];

		GetLocalTime(&sysTime);

		wsprintf(buf, _T("%02d/%02d/%d - %02d:%02d:%02d"),sysTime.wMonth,sysTime.wDay,sysTime.wYear,
		         sysTime.wHour,sysTime.wMinute,sysTime.wSecond);

		dca::String c(buf);
	
		_logFile << c.c_str() << endl;
	}
	void Write(const TCHAR* txt)
	{
		dca::String c(txt);

		_logFile << c.c_str() << endl;
	}
	void LineBreak()
	{
		_logFile << endl;
	}
	void Line()
	{
		for(int i = 0;i < 50;i++)
		{
			_logFile << "*";
		}
		_logFile << endl << endl;
	}
private:
	ofstream _logFile;
};

extern LogFile	g_logFile;
extern int	    g_showResults;

inline int ErrorOpt(CDBConn& dbConn,reg::Key& rkMailFlow)
{
	if(g_showResults != 0)
	{
			_tcout << endl << dbConn.GetErrorString() << endl;
	}

	g_logFile.Write(dbConn.GetErrorString());

	dbConn.ExecuteSQL(_T("rollback transaction"));
	
	dbConn.Disconnect();
	rkMailFlow.Close();
	g_logFile.LineBreak();
	g_logFile.Line();
	return 4;
}

inline BOOL IsTransUp(CDBConn& dbConn,reg::Key& rkMailFlow)
{
	TCHAR sData[256];

	// Debug
	dbConn.GetData(_T("select @@TRANCOUNT"),sData);

	int x = _tstoi(sData);
	if(x == 0)
	{
		return FALSE;
	}

	return TRUE;
}

int QuarantinePage(CDBConn& dbConn,reg::Key& rkMailFlow);
int TicketNotes(CDBConn& dbConn,reg::Key& rkMailFlow);
int TicketBoxAlg(CDBConn& dbConn,reg::Key& rkMailFlow);
int EncryptAgentsPassword(CDBConn& dbConn,reg::Key& rkMailFlow);
int EncryptMessageSrcPasswords(CDBConn& dbConn,reg::Key& rkMailFlow);
int EncryptMessageDestPasswords(CDBConn& dbConn,reg::Key& rkMailFlow);
int CustomDictionaryFix(CDBConn& dbConn,reg::Key& rkMailFlow);
int VTwoBuildThree(CDBConn& dbConn,reg::Key& rkMailFlow);
int VThreeBuildFour(CDBConn& dbConn,reg::Key& rkMailFlow);
int V2007(CDBConn& dbConn,reg::Key& rkMailFlow);
int V3000(CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn);
int V3001(CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn);
int V3002(CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn);
int V3100(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V3500(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V3600(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V3610(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V3611(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V3800(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4000(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4500(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4800(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4810(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4900(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4910(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4912(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4914(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4920(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4921(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V4922(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V5000(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V5030(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V5100(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6000(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6001(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6100(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6102(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6500(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6501(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6504(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6505(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6601(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6603(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6610(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6900(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6910(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6920(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6930(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int V6950(CODBCQuery& m_query, CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn);
int VInstall(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int VExportXML(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int VFullText(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int RestoreTicket(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn,int nTicketID);
int SearchArchive(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
int ConvertData(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow,TCHAR* wDsn);
void StringToUTF8(TCHAR* szIn, tstring& szOut);
int ConvertTickets(CODBCQuery& m_query);
int ConvertInboundMessages(CODBCQuery& m_query);
int ConvertOutboundMessages(CODBCQuery& m_query);
int ConvertAttachments(CODBCQuery& m_query);
int ConvertContacts(CODBCQuery& m_query);
int ConvertPersonalData(CODBCQuery& m_query);
int ConvertAgents(CODBCQuery& m_query);
int ConvertGroups(CODBCQuery& m_query);
int ConvertSignatures(CODBCQuery& m_query);
int ConvertRoutingRules(CODBCQuery& m_query);
int ConvertProcessingRules(CODBCQuery& m_query);
int ConvertTicketBoxes(CODBCQuery& m_query);
int ConvertTicketCategories(CODBCQuery& m_query);
int ConvertFolders(CODBCQuery& m_query);
int ConvertSRs(CODBCQuery& m_query);
int ConvertSRCats(CODBCQuery& m_query);
int ConvertContactGroups(CODBCQuery& m_query);
int ConvertTicketNotes(CODBCQuery& m_query);
int ConvertContactNotes(CODBCQuery& m_query);
#endif // MAIN_H