#include "stdafx.h"

WebSessionThread::WebSessionThread(void): m_query(m_db)
{
	DebugReporter::Instance().DisplayMessage("WebSessionThread::WebSessionThread - Constructor", DebugReporter::ENGINE);
	m_bInitialized = false;
	nEnabled = 0;
	//srand ( (unsigned) time(NULL) );
}

WebSessionThread::~WebSessionThread(void)
{
	DebugReporter::Instance().DisplayMessage("WebSessionThread::WebSessionThread - Destructor", DebugReporter::ENGINE);	
}

void WebSessionThread::Initialize()
{
	try
	{
		DebugReporter::Instance().DisplayMessage("WebSessionThread::Initialize", DebugReporter::ENGINE);		
		
		//Wait 5 seconds to allow globals to be loaded
		Sleep(5000);
		
		bool success = true;
		if( !m_db.IsConnected() )
		{
			m_db.Connect();			
		}
		
		nEnabled = _wtoi(g_Object.GetParameter( EMS_SRVPARAM_MASTER_WEBSESSIONS ));
		nMasterID = _wtoi(g_Object.GetParameter( EMS_SRVPARAM_MASTER_SERVER ));
		GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_SERVER_ID_VALUE, nServerID );

		if(nEnabled == 1 && (nMasterID == nServerID))
		{
			TIMESTAMP_STRUCT now;
			GetTimeStamp( now );
			m_query.Initialize();
			BINDPARAM_TIME_NOLEN( m_query, now );
			BINDPARAM_TIME_NOLEN( m_query, now );
			m_query.Execute(_T("DELETE FROM SessionMonitorQueue WHERE ProcessID NOT IN (SELECT ProcessID FROM SessionMonitors) ")
							_T("DELETE FROM SessionMonitorQueue WHERE ProcessID IN (SELECT ProcessID FROM SessionMonitors WHERE CheckIn < DATEADD(minute,-10,?)) ")
							_T("DELETE FROM SessionMonitors WHERE CheckIn < DATEADD(minute,-10,?)"));
			/*success = listenSocket.UnlockComponent("DEERFISocket_D3OcgxH7EFpL");
			if (success != true)
			{
				DebugReporter::Instance().DisplayMessage("WebSessionThread::Initialize - Failed to unlock CkSocket component", DebugReporter::ENGINE);
				Uninitialize();			
			}
			else
			{
				static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";        
				char *s = new char[SESSIONID_LENGTH];

				for( int i = 0; i < SESSIONID_LENGTH; i++ )
				{
					s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
				}	
				
				s[SESSIONID_LENGTH]=0;

				dca::String sTemp(s);
				s[SESSIONID_LENGTH] = '\0';
				delete[] s;
				sKey.assign(sTemp);

				USES_CONVERSION;
				tstring sRandom;
				sRandom.assign(A2T(sTemp.c_str()));
								
				m_query.Initialize();
				BINDPARAM_WCHAR( m_query, (wchar_t*) sRandom.c_str() );
				m_query.Execute( L"UPDATE ServerParameters SET DataValue=? WHERE ServerParameterID=133" );

				LoadSessionsFromDB();

				m_query.Initialize();
				BINDCOL_LONG_NOLEN( m_query, nPort );
				m_query.Execute( _T("SELECT DataValue FROM ServerParameters WHERE ServerParameterID=135") );
				if ( m_query.Fetch() != S_OK )
				{
					nPort = 1227;
				}	

				success = listenSocket.BindAndListen(nPort,5);
				if (success != true)
				{
					DebugReporter::Instance().DisplayMessage("WebSessionThread::Initialize() - Failed to bind to port", DebugReporter::MAIL);
					Uninitialize();					
				}
				connectedSocket = 0;
			}*/
		}
		else
		{
			//try{listenSocket.Close(1000);}catch(...){}
		}
		
		if(success)
		{
			m_bInitialized = true;
		}
	}
	catch( ODBCError_t error )
	{
		HandleODBCError( &error );
	}		
	catch(dca::Exception& e)
	{
		try
		{
			dca::String er;
			dca::String x(e.GetMessage());
			er.Format("WebSessionThread::Initialize - %s", x.c_str());
			DebugReporter::Instance().DisplayMessage(er.c_str(), DebugReporter::ENGINE);
			Log( E_WebSessionError, L"%s\n", e.GetMessage());
			
			m_db.Disconnect();
		}
		catch(...)
		{}
	}
	catch(...)
	{
		try
		{
			DebugReporter::Instance().DisplayMessage("Database was not initialized in the WebSession Thread", DebugReporter::ENGINE);
			Log( E_WebSessionError, L"Database was not initialized in the WebSession Thread\n");

			m_db.Disconnect();
		}
		catch(...)
		{
		}
	}
}

void WebSessionThread::Uninitialize()
{
	DebugReporter::Instance().DisplayMessage("WebSessionThread::Uninitialize - Entered", DebugReporter::ENGINE);
	try
	{
		m_db.Disconnect();
		//try{connectedSocket->Close(1000);if(connectedSocket){delete connectedSocket;}}catch(...){}
		//try{listenSocket.dispose();}catch(...){}
		DebugReporter::Instance().DisplayMessage("WebSessionThread::Uninitialize - Disconnected from DB", DebugReporter::ENGINE);
	}
	catch(...)
	{
	}
}

unsigned int WebSessionThread::Run()
{
	DWORD dwRet;
	MSG msg;
	BOOL bRun = TRUE;
	DWORD dwWaitTime;	
	bool success;
	int nCount = 0;
	
	PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );	
	SetEvent( m_hReadyEvent );
						
	while ( bRun )
	{
		dwWaitTime = 1000;
		dwRet = MsgWaitForMultipleObjects( 1, &m_hStopEvent, FALSE, dwWaitTime, QS_ALLEVENTS );
		int nRet;

		switch( dwRet )
		{
		case WAIT_TIMEOUT:
		{
			if( m_bInitialized )
			{
				if(nEnabled == 1)
				{
					nCount++;
					if(nCount > 300)
					{
						nCount = 0;
						TIMESTAMP_STRUCT now;
						GetTimeStamp( now );
						m_query.Initialize();
						BINDPARAM_TIME_NOLEN( m_query, now );
						BINDPARAM_TIME_NOLEN( m_query, now );
						m_query.Execute(_T("DELETE FROM SessionMonitorQueue WHERE ProcessID NOT IN (SELECT ProcessID FROM SessionMonitors) ")
										_T("DELETE FROM SessionMonitorQueue WHERE ProcessID IN (SELECT ProcessID FROM SessionMonitors WHERE CheckIn < DATEADD(minute,-10,?)) ")
										_T("DELETE FROM SessionMonitors WHERE CheckIn < DATEADD(minute,-10,?)"));
					}
					//dca::String sMsg;
					//dca::String sTemp;
					//long lPos=0;

					//connectedSocket = listenSocket.AcceptNextConnection(5000);
					//if (connectedSocket == 0 )
					//{
					//	DebugReporter::Instance().DisplayMessage("WebSessionThread::Run() - No incoming connection, looping", DebugReporter::MAIL);
					//	try{connectedSocket->Close(1000);}catch(...){}
					//	continue;
					//}

					////  Set maximum timeouts for reading and writing (in millisec)
					//connectedSocket->put_MaxReadIdleMs(5000);
					//connectedSocket->put_MaxSendIdleMs(5000);

					////  The client will send a command followed by -EOM-"
					//const char *receivedMsg = 0;
					//receivedMsg = connectedSocket->receiveUntilMatch("-EOM-");
					//if (receivedMsg == 0 )
					//{
					//	try{connectedSocket->Close(1000);}catch(...){}
					//	continue;
					//}
					//
					////Parse the command
					//try
					//{
					//	sMsg.assign(receivedMsg);
					//	sMsg = sMsg.substr(sMsg.find("key:"),sMsg.length()-sMsg.find("key:"));
					//	lPos = sMsg.find("-EOM-");
					//	sMsg = sMsg.substr(0,lPos);					
					//	lPos = sMsg.find("cmd:");					
					//	sTemp = sMsg.substr(4,lPos-4);
					//	if(strcmp(sTemp.c_str(),sKey.c_str()) != 0)
					//	{
					//		//Invalid key submitted
					//		success = connectedSocket->SendString("1");
					//		try{connectedSocket->Close(5000);}catch(...){}
					//		continue;
					//	}
					//}
					//catch(...)
					//{
					//	//Parse Error
					//	success = connectedSocket->SendString("1");
					//	try{connectedSocket->Close(5000);}catch(...){}
					//	continue;
					//}
					//
					//try
					//{
					//	sTemp = sMsg.substr(sMsg.find("cmd:")+4,1);
					//	nCommand = atoi(sTemp.c_str());
					//	if(nCommand<1 || nCommand>7)
					//	{
					//		//Invalid command
					//		success = connectedSocket->SendString("2");
					//		try{connectedSocket->Close(5000);}catch(...){}
					//		continue;
					//	}
					//}
					//catch(...)
					//{
					//	//Invalid command
					//	success = connectedSocket->SendString("2");
					//	try{connectedSocket->Close(5000);}catch(...){}
					//	continue;
					//}
					//
					//switch(nCommand)
					//{
					//case 1:
					//	//Send AgentXML
					//	GenAgentXML();
					//	success = connectedSocket->SendString(aXML.c_str());
					//	if (success != true) {
					//		DebugReporter::Instance().DisplayMessage("WebSessionThread::Run() - Failed to accept connection", DebugReporter::MAIL);
					//		connectedSocket->Close(1000);
					//		continue;
					//	}
					//	break;
					//case 2:
					//	//Send BrowserXML
					//						
					//	break;
					//case 3:

					//	break;
					//case 4:
					//	try
					//	{
					//		int nAgentID;
					//		int nStatusID;
					//		tstring sStatusText;
					//		SYSTEMTIME lastAuto;
					//		lastAuto.wYear = 0;
					//		lastAuto.wMonth = 0;
					//		lastAuto.wDay = 0;
					//		
					//		lPos = sMsg.find("statusid:");					
					//		sTemp = sMsg.substr(sMsg.find("agentid:")+8,lPos-sMsg.find("agentid:")+8);
					//		nAgentID = atoi(sTemp.c_str());
					//		sTemp = sMsg.substr(sMsg.find("statusid:")+9,1);
					//		nStatusID = atoi(sTemp.c_str());
					//		lPos = sMsg.find("statustext:")+11;
					//		sTemp = sMsg.substr(lPos,sMsg.length()-lPos);
					//		USES_CONVERSION;
					//		sStatusText = A2T(sTemp.c_str());
					//		if(nAgentID>0 && nStatusID>0 && nStatusID<7 && sStatusText.length()>0)
					//		{
					//			UpdateAgentSession( nAgentID,lastAuto,nStatusID,sStatusText,0,_T(""));
					//		}
					//		else
					//		{
					//			//Invalid data
					//			success = connectedSocket->SendString("3");
					//			try{connectedSocket->Close(1000);}catch(...){}
					//			continue;
					//		}
					//	}
					//	catch(...)
					//	{
					//		//Invalid data
					//		success = connectedSocket->SendString("3");
					//		try{connectedSocket->Close(1000);}catch(...){}
					//		continue;
					//	}
					//	break;
					//}
					//					
					////  Close the connection with the client.
					//try{connectedSocket->Close(1000);}catch(...){}
				}
			}
			else
			{
				Initialize();
			}		
		}
		break;
		
		case WAIT_OBJECT_0 + 1:
		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{				
			HandleMsg( &msg );				
		}
		break;

		case WAIT_OBJECT_0:
			{
				bRun = FALSE;
				DebugReporter::Instance().DisplayMessage("WebSessionThread::Run - Received stop event", DebugReporter::ENGINE);
			}
			break;
		}
	}
	
	Uninitialize();
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// HandleMsg
// 
////////////////////////////////////////////////////////////////////////////////
void WebSessionThread::HandleMsg( MSG* msg )
{
	HANDLE hEvent;

	switch( msg->message )
	{
	case WM_RELOAD_CONFIG:
		{
			Initialize();

			hEvent = (HANDLE)msg->wParam;

			SetEvent( hEvent );	
			CloseHandle( hEvent );
		}

		break;

	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// HandleODBCError
// 
////////////////////////////////////////////////////////////////////////////////
void WebSessionThread::HandleODBCError( ODBCError_t* pErr )
{
	wchar_t szDebug[1024];


	if( pErr->nErrorCode == E_DSNNotInRegistry )
	{
		CEMSString s;
		s.LoadString( EMS_STRING_ERROR_NODSN );
		wcscpy( pErr->szErrMsg, s.c_str() );
	}

	swprintf( szDebug, L"Caught ODBCError: %d, %s\n", 
			  pErr->nErrorCode, pErr->szErrMsg );

	Log( E_WebSessionError, szDebug );

	// Close the database connection
	if( m_db.IsConnected() )
	{
		m_query.Close();
		m_db.Disconnect();
	}
}

void WebSessionThread::GenAgentXML( void )
{
	FILETIME ft;
	aXML.clear();
	aXML.append( "<XML id=\"AXML\"><root>" );
	for ( aIter = m_aMap.begin(); aIter != m_aMap.end(); aIter++ )
	{
		char buffer [33];
		aXML.append( "<ASession>" );
		aXML.append( "<AgentID>" );
		itoa(aIter->first,buffer,10);
		aXML.append( buffer );
		aXML.append( "</AgentID>" );
		SystemTimeToFileTime( &(aIter->second.m_LastAuto), &ft);
		aXML.append( "<LastAutoHigh>" );
		itoa(ft.dwHighDateTime,buffer,10);
		aXML.append( buffer );
		aXML.append( "</LastAutoHigh>" );
		aXML.append( "<LastAutoLow>" );
		itoa(ft.dwLowDateTime,buffer,10);
		aXML.append( buffer );
		aXML.append( "</LastAutoLow>" );
		aXML.append( "<StatusID>" );
		itoa(aIter->second.m_StatusID,buffer,10);
		aXML.append( buffer );
		aXML.append( "</StatusID>" );
		aXML.append( "<StatusText>" );
		dca::String sTemp(aIter->second.m_StatusText.c_str());
		aXML.append( sTemp  );
		aXML.append( "</StatusText>" );
		aXML.append( "<PreAutoID>" );
		itoa(aIter->second.m_PreAutoID,buffer,10);
		aXML.append( buffer );
		aXML.append( "</PreAutoID>" );
		aXML.append( "<PreAutoText>" );
		dca::String sTemp2(aIter->second.m_PreAutoText.c_str());
		aXML.append( sTemp2  );
		aXML.append( "</PreAutoText>" );
		aXML.append( "</ASession>"  );
	}
	aXML.append( "</root></XML>" );
}

void WebSessionThread::GenBrowserXML( void )
{
	FILETIME ft;
	bXML.clear();
	bXML.append( "<XML id=\"BXML\"><root>" );
	for ( bIter = m_bMap.begin(); bIter != m_bMap.end(); bIter++ )
	{
		char buffer [33];
		bXML.append( "<BSession>" );
		bXML.append( "<SessionID>" );
		bXML.append( bIter->first  );
		bXML.append( "</SessionID>" );
		bXML.append( "<AgentID>" );
		itoa(bIter->second.m_AgentID,buffer,10);
		bXML.append( buffer );
		bXML.append( "</AgentID>" );
		SystemTimeToFileTime( &(bIter->second.m_LastRequest), &ft);
		bXML.append( "<LastRequestHigh>" );
		itoa(ft.dwHighDateTime,buffer,10);
		bXML.append( buffer );
		bXML.append( "</LastRequestHigh>" );
		bXML.append( "<LastRequestLow>" );
		itoa(ft.dwLowDateTime,buffer,10);
		bXML.append( buffer );
		bXML.append( "</LastRequestLow>" );		
		bXML.append( "</BSession>" );
	}
	bXML.append( "</root></XML>" );
}
void WebSessionThread::LoadSessionsFromDB(void)
{
	if(!m_db.IsConnected()){try{m_db.Connect();}catch(...){}}	
	
	TCHAR szSessionID[51] = {0};
	TCHAR szIP[51] = {0};
	TCHAR szUserAgent[51] = {0};
	TCHAR szPreAutoText[126] = {0};
	TCHAR szStatusText[126] = {0};
	int nAgentID;
	int nPreAutoID;
	int nStatusID;
	TIMESTAMP_STRUCT lr;
	long lrLen;
	
	m_query.Initialize();
	
	BINDCOL_TCHAR_NOLEN( m_query, szSessionID );
	BINDCOL_LONG_NOLEN( m_query, nAgentID );
	BINDCOL_TIME( m_query, lr );
	m_query.Execute(  _T("SELECT SessionID,AgentID,LastRequest FROM BrowserSessions") );

	while( m_query.Fetch() == S_OK )
	{
		CBSession bsession;

		bsession.m_AgentID = nAgentID;	
		TimeStampToSystemTime(lr, bsession.m_LastRequest);
		m_bMap[ szSessionID ] = bsession;
	}

	m_query.Reset(true);
	
	BINDCOL_LONG_NOLEN( m_query, nAgentID );
	BINDCOL_TIME( m_query, lr );
	BINDCOL_LONG_NOLEN( m_query, nPreAutoID );
	BINDCOL_TCHAR_NOLEN( m_query, szPreAutoText );
	BINDCOL_LONG_NOLEN( m_query, nStatusID );
	BINDCOL_TCHAR_NOLEN( m_query, szStatusText );
	m_query.Execute(  _T("SELECT s.AgentID,s.LastAuto,s.PreAutoID,s.PreAutoText, ")
					  _T("a.StatusID,a.StatusText ")
					  _T("FROM AgentSessions s INNER JOIN Agents a ON s.AgentID=a.AgentID") );
	
	while( m_query.Fetch() == S_OK )
	{
		CASession asession;

		asession.m_AgentID = nAgentID;	
		TimeStampToSystemTime(lr, asession.m_LastAuto);
		asession.m_PreAutoID = nPreAutoID;
		asession.m_PreAutoText.assign(szPreAutoText);
		asession.m_StatusID = nStatusID;
		asession.m_StatusText.assign(szStatusText);
		m_aMap[ nAgentID ] = asession;
	}	
}

void WebSessionThread::AddBrowserSession( dca::String sSessionID, int nAgentID, SYSTEMTIME lastRequest )
{
	bIter = m_bMap.find( sSessionID );
	if( bIter == m_bMap.end() )
	{
		CBSession bsession;

		bsession.m_AgentID = nAgentID;	
		bsession.m_LastRequest = lastRequest;
		m_bMap[ sSessionID ] = bsession;
	}
}

void WebSessionThread::AddAgentSession( int nAgentID )
{
	aIter = m_aMap.find( nAgentID );
	if( aIter == m_aMap.end() )
	{
		if(!m_db.IsConnected()){try{m_db.Connect();}catch(...){}}	
	
		TCHAR szPreAutoText[126] = {0};
		TCHAR szStatusText[126] = {0};		
		int nPreAutoID;
		int nStatusID;
		TIMESTAMP_STRUCT lr;
			long lrLen;

		m_query.Initialize();
	
		BINDPARAM_LONG( m_query, nAgentID );
		BINDCOL_TIME( m_query, lr );
		BINDCOL_LONG_NOLEN( m_query, nPreAutoID );
		BINDCOL_TCHAR_NOLEN( m_query, szPreAutoText );
		BINDCOL_LONG_NOLEN( m_query, nStatusID );
		BINDCOL_TCHAR_NOLEN( m_query, szStatusText );
		m_query.Execute(  _T("SELECT s.LastAuto,s.PreAutoID,s.PreAutoText ")
						_T("a.StatusID,a.StatusText ")
						_T("FROM AgentSessions s INNER JOIN Agents a ON s.AgentID=a.AgentID WHERE s.AgentID=?") );
		
		if( m_query.Fetch() == S_OK )
		{
			CASession asession;

			asession.m_AgentID = nAgentID;	
			TimeStampToSystemTime(lr, asession.m_LastAuto);
			asession.m_PreAutoID = nPreAutoID;
			asession.m_PreAutoText.assign(szPreAutoText);
			asession.m_StatusID = nStatusID;
			asession.m_StatusText.assign(szStatusText);
			m_aMap[ nAgentID ] = asession;
		}	
	}
}
	
void WebSessionThread::UpdateBrowserSession( dca::String sSessionID, int nAgentID, SYSTEMTIME lastRequest)
{
	bIter = m_bMap.find( sSessionID );
	if( bIter != m_bMap.end() )
	{
		bIter->second.m_LastRequest = lastRequest;		
	}
	else
	{
		AddBrowserSession(sSessionID,nAgentID,lastRequest);
	}
}
	
void WebSessionThread::UpdateAgentSession( int nAgentID, SYSTEMTIME lastAuto, int nStatusID, tstring sStatusText, int nPreAutoID, tstring sPreAutoText )
{
	aIter = m_aMap.find( nAgentID );
	if( aIter != m_aMap.end() )
	{
		if(lastAuto.wYear != 0 && lastAuto.wMonth !=0 && lastAuto.wDay != 0)
		{
			aIter->second.m_LastAuto = lastAuto;
		}
		if(nPreAutoID > -1 && nPreAutoID < 7)
		{
			aIter->second.m_PreAutoID = nPreAutoID;
			aIter->second.m_PreAutoText.assign(sPreAutoText);
		}
		if(nStatusID > -1 && nStatusID < 7)
		{
			aIter->second.m_StatusID = nStatusID;
			aIter->second.m_StatusText.assign(sStatusText);
		}
	}
	else
	{
		AddAgentSession(nAgentID);
	}
}
