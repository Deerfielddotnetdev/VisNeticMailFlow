#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6910(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Modify English Dictionary CLX Entry
	
	if(g_showResults != 0)
	{
		_tcout << _T("Modify English Dictionary CLX Entry") << endl;
	}

	g_logFile.Write(_T("Modify English Dictionary CLX Entry"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE Dictionary SET ClxFile='ssceam2.clx' WHERE DictionaryID=1"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	// Add columns to MessageSources
	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to MessageSources table") << endl;
	}

	g_logFile.Write(_T("Add columns to MessageSources table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageSources ADD AccessToken TEXT DEFAULT '' NOT NULL ")
							   _T("ALTER TABLE MessageSources ADD AccessTokenExpire DATETIME ")
							   _T("ALTER TABLE MessageSources ADD RefreshToken TEXT DEFAULT '' NOT NULL ")
							   _T("ALTER TABLE MessageSources ADD RefreshTokenExpire DATETIME ")
							   _T("ALTER TABLE MessageSources ADD OAuthHostID INT DEFAULT 0 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add columns to MessageDestinations
	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to MessageDestinations table") << endl;
	}

	g_logFile.Write(_T("Add columns to MessageDestinations table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageDestinations ADD AccessToken TEXT DEFAULT '' NOT NULL ")
							   _T("ALTER TABLE MessageDestinations ADD AccessTokenExpire DATETIME ")
							   _T("ALTER TABLE MessageDestinations ADD RefreshToken TEXT DEFAULT '' NOT NULL ")
							   _T("ALTER TABLE MessageDestinations ADD RefreshTokenExpire DATETIME ")
							   _T("ALTER TABLE MessageDestinations ADD OAuthHostID INT DEFAULT 0 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add OAuthHosts Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create OAuthHosts table") << endl;
	}

	g_logFile.Write(_T("Create OAuthHosts table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[OAuthHosts]( ")
	_T("[OAuthHostID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[Description] [varchar](255) NOT NULL, ")
	_T("[AuthEndPoint] [varchar](255) NOT NULL, ")
	_T("[TokenEndPoint] [varchar](255) NOT NULL, ")
	_T("[ClientID] [varchar](255) NOT NULL, ")
	_T("[ClientSecret] [varchar](255) NOT NULL, ")
	_T("[Scope] [varchar] (512) NOT NULL , ")
	_T("CONSTRAINT [PK_OAuthHosts] PRIMARY KEY CLUSTERED ([OAuthHostID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add row to OAuthHosts Table
	if(g_showResults != 0)
	{
		_tcout << _T("Add row to OAuthHosts table") << endl;
	}

	g_logFile.Write(_T("Add rows to OAuthHosts table"));
	
	retSQL = dbConn.ExecuteSQL( _T("INSERT INTO OAuthHosts (Description,AuthEndPoint,TokenEndPoint,ClientID,ClientSecret,Scope) VALUES ('Microsoft 365','https://login.microsoftonline.com/common/oauth2/v2.0/authorize','https://login.microsoftonline.com/common/oauth2/v2.0/token','a4f1a665-9df0-42e5-ab77-435d90bace89','gZG3-H~~KIgwowbM4XYx1Ic.ohA2K2kBUm','openid profile offline_access https://outlook.office365.com/SMTP.Send https://outlook.office365.com/POP.AccessAsUser.All https://outlook.office365.com/IMAP.AccessAsUser.All') "));
							  
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}