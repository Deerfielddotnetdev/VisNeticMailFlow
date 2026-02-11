#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6603(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	maxObjectID = 0;

	// Check if MessageSourceTypeID of 2 exists
	if(g_showResults != 0)
	{
		_tcout << _T("Check to see if MessageSourceTypeID of 2 exists") << endl;
	}
	g_logFile.Write(_T("Check to see if MessageSourceTypeID of 2 exists"));
	
	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, maxObjectID );
	m_query.Execute( L"SELECT MessageSourceTypeID FROM MessageSourceTypes WHERE MessageSourceTypeID=2" );
	if ( m_query.GetRowCount() > 0 )
	{
		if(g_showResults != 0)
		{
			_tcout << _T("MessageSourceTypeID of 2 exists, updating now") << endl;
		}
		g_logFile.Write(_T("MessageSourceTypeID of 2 exists, updating now"));

		retSQL = dbConn.ExecuteSQL( _T("UPDATE MessageSourceTypes SET Description='IMAP' WHERE MessageSourceTypeID=2") );
		
		if(!SQL_SUCCEEDED(retSQL))
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}
	else
	{
		if(g_showResults != 0)
		{
			_tcout << _T("MessageSourceTypeID of 2 does not exist, adding now") << endl;
		}
		g_logFile.Write(_T("MessageSourceTypeID of 2 does not exist, adding now"));

		retSQL = dbConn.ExecuteSQL( _T("SET IDENTITY_INSERT MessageSourceTypes ON INSERT INTO [MessageSourceTypes]([MessageSourceTypeID],[Description]) VALUES (2,'IMAP') SET IDENTITY_INSERT MessageSourceTypes OFF") );
		
		if(!SQL_SUCCEEDED(retSQL))
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	//Add Column to MessageSources
	if(g_showResults != 0)
	{
		_tcout << _T("Add ErrorCode column to MessageSources table") << endl;
	}

	g_logFile.Write(_T("Add ErrorCode column to MessageSources table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageSources ADD ErrorCode INT DEFAULT 0 NOT NULL") );
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Column to MessageDestinations
	if(g_showResults != 0)
	{
		_tcout << _T("Add ErrorCode column to MessageDestinations table") << endl;
	}

	g_logFile.Write(_T("Add ErrorCode column to MessageDestinations table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE MessageDestinations ADD ErrorCode INT DEFAULT 0 NOT NULL") );
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;
}