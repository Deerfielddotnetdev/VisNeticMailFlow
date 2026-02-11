#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6504(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	maxObjectID = 0;

	//Get StandardResponse CatIDs
	if(g_showResults != 0)
	{
		_tcout << _T("Get StandardResponse CatIDs") << endl;
	}

	g_logFile.Write(_T("Get StandardResponse CatIDs"));

	int nCatID;
	list<int> catList;
	list<int>::iterator iter;

	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, nCatID );
	m_query.Execute( L"SELECT StdResponseCatID FROM StdResponseCategories WHERE StdResponseCatID>1" );
	while( m_query.Fetch() == S_OK )
	{
		catList.push_back( nCatID );			
	}
	
	//Update ObjectTypes Table
	if(g_showResults != 0)
	{
		_tcout << _T("Updating ObjectTypes Table") << endl;
	}

	g_logFile.Write(_T("Updating ObjectTypes Table"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE ObjectTypes SET Description='Standard Response Category' WHERE ObjectTypeID=5"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Columns to StdResponseCategories
	if(g_showResults != 0)
	{
		_tcout << _T("Add ObjectID and BuiltIn columns to StdResponseCategories table") << endl;
	}

	g_logFile.Write(_T("Add ObjectID and BuiltIn columns to StdResponseCategories table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE StdResponseCategories ADD ObjectID INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE StdResponseCategories ADD BuiltIn BIT DEFAULT 0 NOT NULL") );
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update Objects Table
	if(g_showResults != 0)
	{
		_tcout << _T("Updating Objects Table") << endl;
	}

	g_logFile.Write(_T("Updating Objects Table"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE Objects SET UseDefaultRights=0 WHERE ObjectTypeID=5 AND BuiltIn=1 AND ActualID=0"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
		
	//Update StdResponseCategories Table
	if(g_showResults != 0)
	{
		_tcout << _T("Updating StdResponseCategories Table") << endl;
	}

	g_logFile.Write(_T("Updating StdResponseCategories Table"));

	retSQL = dbConn.ExecuteSQL(_T("UPDATE StdResponseCategories SET ObjectID=5,BuiltIn=1 WHERE StdResponseCatID=1"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Row to StdResponseCategories Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to StdResponseCategories Table") << endl;
	}

	g_logFile.Write(_T("Adding Row to StdResponseCategories Table"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT StdResponseCategories ON ")
							   _T("INSERT INTO StdResponseCategories (StdResponseCatID,CategoryName,ObjectID,BuiltIn) VALUES (-4,'Uncategorized',0,1) ")							  
							   _T("SET IDENTITY_INSERT StdResponseCategories OFF"));
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

	//Add Row to Objects table	
	if(g_showResults != 0)
	{
		_tcout << _T("Add row to Objects table") << endl;
	}

	g_logFile.Write(_T("Add row to Objects table"));
	
	wsprintf(buf, _T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,-4,5,1,1,GetDate()) SET IDENTITY_INSERT Objects OFF"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Update StdResponseCategories table	
	if(g_showResults != 0)
	{
		_tcout << _T("Update StdResponseCategories table") << endl;
	}

	g_logFile.Write(_T("Update StdResponseCategories table"));
	
	wsprintf(buf, _T("UPDATE StdResponseCategories SET ObjectID=%d WHERE StdResponseCatID=-4"), newObjectID );
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	newObjectID++;
	maxObjectID++;

	//Update StandardResponses table	
	if(g_showResults != 0)
	{
		_tcout << _T("Update StdResponseCatID in StandardResponses table") << endl;
	}

	g_logFile.Write(_T("Update StdResponseCatID in StandardResponses table"));
	
	retSQL = dbConn.ExecuteSQL(_T("UPDATE StandardResponses SET StdResponseCatID=-4 WHERE StdResponseCatID=0"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	for( iter = catList.begin(); iter != catList.end(); iter++ )
	{
		//Add Row Objects table	
		if(g_showResults != 0)
		{
			_tcout << _T("Add row to Objects table") << endl;
		}

		g_logFile.Write(_T("Add row to Objects table"));
		
		wsprintf(buf, _T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (%d,%d,5,0,1,GetDate()) SET IDENTITY_INSERT Objects OFF"), newObjectID, *iter );
		retSQL = dbConn.ExecuteSQL(buf);
		if(!SQL_SUCCEEDED(retSQL))
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
		
		//Update StdResponseCategories table	
		if(g_showResults != 0)
		{
			_tcout << _T("Update StdResponseCategories table") << endl;
		}

		g_logFile.Write(_T("Update StdResponseCategories table"));
		
		wsprintf(buf, _T("UPDATE StdResponseCategories SET ObjectID=%d WHERE StdResponseCatID=%d"), newObjectID, *iter );
		retSQL = dbConn.ExecuteSQL(buf);
		if(!SQL_SUCCEEDED(retSQL))
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
		newObjectID++;
		maxObjectID++;
	}
		
	//Add Column to RoutingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Add IgnoreTracking column to RoutingRules table") << endl;
	}

	g_logFile.Write(_T("Add IgnoreTracking column to RoutingRules table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD IgnoreTracking INT DEFAULT 0 NOT NULL") );
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Row to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Row to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (143,'Agent Email Bypass','0') ")							  
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Column to AutoMessages
	if(g_showResults != 0)
	{
		_tcout << _T("Add OmitTracking column to AutoMessages table") << endl;
	}

	g_logFile.Write(_T("Add OmitTracking column to AutoMessages table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AutoMessages ADD OmitTracking INT DEFAULT 0 NOT NULL") );
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Column to AutoResponses
	if(g_showResults != 0)
	{
		_tcout << _T("Add OmitTracking column to AutoResponses table") << endl;
	}

	g_logFile.Write(_T("Add OmitTracking column to AutoResponses table"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AutoResponses ADD OmitTracking INT DEFAULT 0 NOT NULL") );
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}