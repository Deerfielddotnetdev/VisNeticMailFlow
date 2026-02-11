////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/GlobalObject.cpp,v 1.1.6.1 2006/07/18 12:50:09 markm Exp $
//
//  Copyright © 2001 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// GlobalObject.cpp: implementation of the CGlobalObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GlobalObject.h"
#include "winsock.h"

CGlobalObject g_Object;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGlobalObject::CGlobalObject(): m_query(m_db)
{
	InitializeCriticalSection( &m_paramsCS );
}

CGlobalObject::~CGlobalObject()
{
	DeleteCriticalSection( &m_paramsCS );
}

int CGlobalObject::Initialize()
{
	int retval = 0;
	UINT nDbServerID=0;
	UINT nRegServerID=0;
	int nRet;
	TCHAR chrComputerName[MAX_COMPUTERNAME_LENGTH + 1]; 
	DWORD dwBufferSize = MAX_COMPUTERNAME_LENGTH + 1;

	//Validate ServerID

	if( !m_db.IsConnected() )
	{
		m_db.Connect();
	}	

	//DebugReporter::Instance().DisplayMessage("CGlobalObject::Initialize - Get Computer Name", DebugReporter::ENGINE);
	//Get the computer name
	GetComputerName(chrComputerName,&dwBufferSize);

	dca::String f;
	dca::String sTemp(chrComputerName);
	f.Format("CGlobalObject::Initialize - Got Computer Name [%s]", sTemp.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);
	
	// Ok, we have the name, let's see if it's setup in the DB
	m_query.Initialize();
	BINDPARAM_TCHAR( m_query, chrComputerName );
	BINDCOL_LONG_NOLEN( m_query, nDbServerID );
	m_query.Execute( L"SELECT ServerID FROM Servers WHERE [Description]=?" );	
	m_query.Fetch();

	f.Format("CGlobalObject::Initialize - Got ServerID [%d] from DB", nDbServerID);
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);

	GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_SERVER_ID_VALUE, nRegServerID );

	f.Format("CGlobalObject::Initialize - Got ServerID [%d] from Registry", nRegServerID);
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);

	
	if ( nDbServerID > 0 &&  nRegServerID != nDbServerID )
	{
		// Server is in DB but not registry, let's add it to Registry
		DebugReporter::Instance().DisplayMessage("CGlobalObject::Initialize - Server is in DB but not registry, let's add it to Registry", DebugReporter::ENGINE);
		WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"ServerID", nDbServerID );
	}
	else if (nDbServerID == 0 || nRegServerID == 0 || nRegServerID != nDbServerID )
	{
		// ServerID is out of synch, let's sync it up
		DebugReporter::Instance().DisplayMessage("CGlobalObject::Initialize - Server ID is out of sync", DebugReporter::ENGINE);
			
		m_query.Initialize();
		BINDPARAM_TCHAR( m_query, chrComputerName );				
		m_query.Execute( L"INSERT INTO Servers([Description],ReloadConfig) VALUES (?,1)" );
		m_query.Initialize();
		BINDPARAM_TCHAR( m_query, chrComputerName );
		BINDCOL_LONG_NOLEN( m_query, nDbServerID );
		m_query.Execute( L"SELECT ServerID FROM Servers WHERE [Description]=?" );	
		if (  m_query.Fetch() == S_OK )
		{
			if ( nDbServerID > 0 )
			{
				// Ok we got the ServerID in the nDbServerID variable, let's put it in the registry
				DebugReporter::Instance().DisplayMessage("CGlobalObject::Initialize - Ok we got the ServerID in the nDbServerID variable, let's put it in the registry", DebugReporter::ENGINE);		
				WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"ServerID", nDbServerID );
			}
		}
	}
	
	// Start All Threads
	retval = m_ThreadManager.StartAllThreads();

	// Load the MessagingComponents
	
	retval = m_MessagingComponents.LoadDLL();
	dca::String t;

	if( retval )
	{
		CEMSString s;
		s.LoadString( EMS_STRING_ERROR_LOADING_MAILCOMPONENTS );
		_Module.LogEvent( s.c_str(), retval );
		Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, L"Error %d loading mailcomponents.dll - mail send and receive is disabled.", retval );
		// Note if we return an error here, we must set the status
		//_Module.m_status.dwWin32ExitCode = ERROR_SERVICE_DEPENDENCY_FAIL;
		t.Format("CGlobalObject::Initialize - Error %d loading mailcomponents.dll - mail send and receive is disabled.", retval);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		retval = 0;
    }
	else
	{
		t.Format("CGlobalObject::Initialize - Successfully loaded mailcomponents.dll");
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
	}

	// Initalize the Messaging Components DLL
	m_MessagingComponents.Initialize();

	return retval;
}

void CGlobalObject::Shutdown()
{
	// Tell all processing to stop
	m_ThreadManager.SetStopEvent();

	// Shutdown the Messaging Components DLL
	if( m_MessagingComponents.IsLoaded() )
	{

		m_MessagingComponents.Shutdown();

	}

	// Stop All Threads
	m_ThreadManager.StopAllThreads();
}

HRESULT CGlobalObject::GetDBParams(CODBCConn& db)
{
	DebugReporter::Instance().DisplayMessage("CGlobalObject::GetDBParams - Entering GetDBParams()", DebugReporter::ENGINE);

	if( !db.IsConnected() )
	{
		db.Connect();
	}

	UINT m_nServerID;
	UINT m_nServerTaskTypeID;
	UINT m_nNumberServers;
	CEMSString sTemp;
	dca::String t;
	
	ServerParameters_t srvparam;

	EnterCriticalSection( &m_paramsCS );

	DebugReporter::Instance().DisplayMessage("CGlobalObject::GetDBParams - Initializing m_query", DebugReporter::ENGINE);

	CODBCQuery m_query( db );
	m_query.Initialize();

	DebugReporter::Instance().DisplayMessage("CGlobalObject::GetDBParams - m_query initialized - binding columns", DebugReporter::ENGINE);

	BINDCOL_LONG( m_query, srvparam.m_ServerParameterID );
	BINDCOL_WCHAR( m_query, srvparam.m_Description );
	BINDCOL_WCHAR( m_query, srvparam.m_DataValue );

	DebugReporter::Instance().DisplayMessage("CGlobalObject::GetDBParams - Columns bound - executing query", DebugReporter::ENGINE);

	m_query.Execute( L"SELECT ServerParameterID, [Description], DataValue FROM ServerParameters" );
	
	DebugReporter::Instance().DisplayMessage("CGlobalObject::GetDBParams - Query executed - entering while fetch", DebugReporter::ENGINE);

	while (  m_query.Fetch() == S_OK )
	{
		dca::String t2(srvparam.m_Description);
		dca::String t3(srvparam.m_DataValue);
		t.Format("CGlobalObject::GetDBParams - Loaded DB parameter: [%d]-[%s] = %s\n", srvparam.m_ServerParameterID,t2.c_str(),t3.c_str());
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		m_params[ srvparam.m_ServerParameterID ] = srvparam.m_DataValue;
	}

	DebugReporter::Instance().DisplayMessage("CGlobalObject::GetDBParams - DB ServerParams complete - getting Server tasks Params ", DebugReporter::ENGINE);

	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               EMS_SERVER_ID_VALUE, m_nServerID ) != ERROR_SUCCESS)
	{
		m_nServerID = 1;
	}

	t.Format("CGlobalObject::GetDBParams - Getting tasks for ServerID: %d", m_nServerID );
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

	sTemp.Format(_T("%d"),m_nServerID);
	wcscpy( srvparam.m_DataValue, sTemp.c_str() );

	m_params[ EMS_SRVPARAM_SERVER_ID ] = srvparam.m_DataValue;
		
	m_params[ EMS_SRVPARAM_ALERT_SENDER ] = _T("0");
	m_params[ EMS_SRVPARAM_DB_MAINT ] = _T("0");
	m_params[ EMS_SRVPARAM_DB_MONITOR ] = _T("0");
	m_params[ EMS_SRVPARAM_MSG_COLLECTOR ] = _T("0");
	m_params[ EMS_SRVPARAM_MSG_ROUTER ] = _T("0");
	m_params[ EMS_SRVPARAM_MSG_SENDER ] = _T("0");
	m_params[ EMS_SRVPARAM_REPORTER ] = _T("0");
	m_params[ EMS_SRVPARAM_TICKET_MONITOR ] = _T("0");
	m_params[ EMS_SRVPARAM_NUMBER_SERVERS ] = _T("1");
	
	m_query.Initialize();
	BINDPARAM_LONG( m_query, m_nServerID );
	BINDCOL_LONG_NOLEN( m_query, m_nServerTaskTypeID );	
	m_query.Execute( L"SELECT ServerTaskTypeID FROM ServerTasks WHERE ServerID=?" );	

	DebugReporter::Instance().DisplayMessage("CGlobalObject::GetDBParams - Query executed - entering Server Tasks while fetch", DebugReporter::ENGINE);

	while (  m_query.Fetch() == S_OK )
	{
		
		t.Format("CGlobalObject::GetDBParams - Setting task ID: %d ", m_nServerTaskTypeID );
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
		
		switch ( m_nServerTaskTypeID )
		{			
			case 1:			
				m_params[ EMS_SRVPARAM_ALERT_SENDER ] = _T("1");
				break;
			case 2:
				m_params[ EMS_SRVPARAM_DB_MAINT ] = _T("1");
				break;
			case 3:
				m_params[ EMS_SRVPARAM_DB_MONITOR ] = _T("1");
				break;
			case 4:
				m_params[ EMS_SRVPARAM_MSG_COLLECTOR ] = _T("1");
				break;
			case 5:
				m_params[ EMS_SRVPARAM_MSG_ROUTER ] = _T("1");
				break;
			case 6:
				m_params[ EMS_SRVPARAM_MSG_SENDER ] = _T("1");
				break;
			case 7:
				m_params[ EMS_SRVPARAM_REPORTER ] = _T("1");
				break;
			case 8:
				m_params[ EMS_SRVPARAM_TICKET_MONITOR ] = _T("1");
				break;
		}		
	}

	DebugReporter::Instance().DisplayMessage("CGlobalObject::GetDBParams - Getting number of servers from DB", DebugReporter::ENGINE);

	m_query.Initialize();
	BINDCOL_LONG_NOLEN( m_query, m_nNumberServers );
	m_query.Execute( L"SELECT COUNT(*) FROM Servers" );	
	if (  m_query.Fetch() == S_OK )
	{
		CEMSString sValue;
		sValue.Format(L"%d",m_nNumberServers);
		m_params[ EMS_SRVPARAM_NUMBER_SERVERS ] = sValue.c_str();
	}
	
	t.Format("CGlobalObject::GetDBParams - %d Server(s) Configured", m_nNumberServers );
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

	LeaveCriticalSection( &m_paramsCS );
	
	dca::String sParam;

	for ( int i = 1000; i < 1010; i++ )
	{
		switch ( i )
		{			
			case 1000:			
				sParam.Format("Server ID");
				break;
			case 1001:
				sParam.Format("Alert Server");
				break;
			case 1002:
				sParam.Format("DB Maint Server");
				break;
			case 1003:
				sParam.Format("DB Monitor Server");
				break;
			case 1004:
				sParam.Format("Msg Collect Server");
				break;
			case 1005:
				sParam.Format("Msg Router Server");
				break;
			case 1006:
				sParam.Format("Msg Sender Server");
				break;
			case 1007:
				sParam.Format("Reporter Server");
				break;
			case 1008:
				sParam.Format("Ticket Monitor Server");
				break;
			case 1009:
				sParam.Format("Number Servers");
				break;
		}		
		
		const wchar_t* szValue = g_Object.GetParameter( i );
		t.Format("CGlobalObject::GetDBParams - Loaded DB parameter: [%d]-[%s] = %s\n", i, sParam.c_str(), szValue);
		DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

	}
	return S_OK;
}


const wchar_t* CGlobalObject::GetParameter( int ID )
{
	/*dca::String t;
	t.Format("CGlobalObject::GetParameter - Value for parameter %d requested", ID );
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);*/
	
	wchar_t* szRet = NULL;

	EnterCriticalSection( &m_paramsCS );

	map<int,wstring>::iterator iter = m_params.find( ID );

	if( iter != m_params.end() )
	{
		szRet = (wchar_t*) iter->second.c_str();
	}

	LeaveCriticalSection( &m_paramsCS );

	/*t.Format("CGlobalObject::GetParameter - Parameter: [%d]-[%s] returned", ID, szRet);
	DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);*/

	return szRet;
}

void CGlobalObject::SetParameter( int ID, const wchar_t* szValue )
{
	EnterCriticalSection( &m_paramsCS );

	g_Object.m_params[ID] = szValue;

	LeaveCriticalSection( &m_paramsCS );
}

void CGlobalObject::SetServerReload()
{
	UINT nServerID;
	
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               EMS_SERVER_ID_VALUE, nServerID ) != ERROR_SUCCESS)
	{
		nServerID = 1;
	}
	if( !m_db.IsConnected() )
	{
		m_db.Connect();
	}	
	m_query.Initialize();
	BINDPARAM_LONG( m_query, nServerID );
	m_query.Execute( L"UPDATE Servers SET ReloadConfig=1 WHERE ServerID<>?" );
}
