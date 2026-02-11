#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6900(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	
	//Add Row to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Row to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (149,'Dictionary ID','1') ")							   
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter Agents Table
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add TimeZoneID column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add TimeZoneID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD TimeZoneID INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add DictionaryID column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add DictionaryID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD DictionaryID INT DEFAULT 1 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Agents Table, add DictionaryPrompt column") << endl;
	}

	g_logFile.Write(_T("Alter Agents Table, add DictionaryPrompt column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Agents ADD DictionaryPrompt BIT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Alter Groups Table
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Groups Table, add TimeZoneID column") << endl;
	}

	g_logFile.Write(_T("Alter Groups Table, add TimeZoneID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Groups ADD TimeZoneID INT DEFAULT 0 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	if(g_showResults != 0)
	{
		_tcout << _T("Alter Groups Table, add DictionaryID column") << endl;
	}

	g_logFile.Write(_T("Alter Groups Table, add DictionaryID column"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Groups ADD DictionaryID INT DEFAULT 1 NOT NULL"));	
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Dictionary Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create Dictionary table") << endl;
	}

	g_logFile.Write(_T("Create Dictionary table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[Dictionary]( ")
	_T("[DictionaryID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[LanguageID] [varchar](2) NOT NULL, ")
	_T("[Description] [varchar](50) NOT NULL, ")
	_T("[TlxFile] [varchar](20) NOT NULL, ")
	_T("[ClxFile] [varchar](20) NOT NULL, ")
	_T("[IsEnabled] [bit] NOT NULL CONSTRAINT [DF_Dictionary_IsEnabled]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_Dictionary] PRIMARY KEY CLUSTERED ([DictionaryID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Rows to Dictionary Table
	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to Dictionary table") << endl;
	}

	g_logFile.Write(_T("Add rows to Dictionary table"));
	
	retSQL = dbConn.ExecuteSQL( _T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('am','American English','ssceam.tlx','ssceam2.clx','1') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('pb','Portuguese (Brazilian)','ssceam_pb.tlx','ssceam2_pb.clx','0') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('da','Danish','ssceam_da.tlx','ssceam2_da.clx','0') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('du','Dutch','ssceam_du.tlx','ssceam2_du.clx','0') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('fi','Finnish','ssceam_fi.tlx','ssceam2_fi.clx','0') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('fr','French','ssceam_fr.tlx','ssceam2_fr.clx','0') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('ge','German','ssceam_ge.tlx','ssceam2_ge.clx','0') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('it','Italian','ssceam_it.tlx','ssceam2_it.clx','0') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('cb','Norwegian (Bokmal)','ssceam_cb.tlx','ssceam2_cb.clx','0') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('po','Portuguese (Iberian)','ssceam_po.tlx','ssceam2_po.clx','0') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('sp','Spanish','ssceam_sp.tlx','ssceam2_sp.clx','0') ")
							   	_T("INSERT INTO Dictionary (LanguageID,Description,TlxFile,ClxFile,IsEnabled) VALUES ('sw','Swedish','ssceam_sw.tlx','ssceam2_sw.clx','0') "));
							  
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}