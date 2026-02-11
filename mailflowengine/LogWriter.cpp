#include "stdafx.h"
#include "LogWriter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogWriter::CLogWriter( CODBCQuery& m_query ) : m_query(m_query)
{
	m_bMapLoaded = FALSE;
	m_FileHandle = INVALID_HANDLE_VALUE;
	m_bDirty = FALSE;
	m_bLogToDb = FALSE;
	m_nServerID = 0;
}

CLogWriter::~CLogWriter()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// UnInitialize
// 
////////////////////////////////////////////////////////////////////////////////
void CLogWriter::UnInitialize(void)
{
	if( m_bMapLoaded )
	{
		// Write out all remaining log entries.
		while( m_Queue.size() > 0 )
		{
			WriteLogEntry( m_Queue.front() );
			m_Queue.pop_front();
		}
	}	

	if( m_FileHandle != INVALID_HANDLE_VALUE )
	{
		CloseLogFile();
	}

	if( m_bMapLoaded )
	{
		m_Map.clear();
		m_bMapLoaded = FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// LoadMap
// 
////////////////////////////////////////////////////////////////////////////////
void CLogWriter::LoadMap(void)
{
	LogEntryTypes_t logentry;
	LogSeverity_t sev;

	m_Map.clear();

	m_query.Initialize();
	
	BINDCOL_LONG( m_query, logentry.m_LogEntryTypeID );
	BINDCOL_WCHAR( m_query, logentry.m_TypeDescrip );
	BINDCOL_LONG( m_query, logentry.m_SeverityLevels );

	m_query.Execute(  L"SELECT LogEntryTypeID,TypeDescrip,SeverityLevels "
					  L"FROM LogEntryTypes "
					  L"ORDER BY LogEntryTypeID" );

	while ( m_query.Fetch() == S_OK )
	{
		m_Map.push_back( logentry );
	}
	
	m_SevMap.clear();

	m_query.Initialize();
	
	BINDCOL_LONG( m_query, sev.m_LogSeverityID );
	BINDCOL_WCHAR( m_query, sev.m_Description );

	m_query.Execute(  L"SELECT LogSeverityID,Description "
					  L"FROM LogSeverity "
					  L"ORDER BY LogSeverityID" );

	while ( m_query.Fetch() == S_OK )
	{
		m_SevMap.push_back( sev );
	}

	//DebugReporter::Instance().DisplayMessage("CLogWriter::LoadMap - Getting ServerID from Global Params", DebugReporter::ENGINE);

	m_nServerID = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_SERVER_ID ) );

	int nLogToDB = _wtoi( g_Object.GetParameter( EMS_SRVPARAM_DATABASE_LOGGING ) );
	
	if ( nLogToDB == 1 )
	{
        m_bLogToDb = true;
	}
	else
	{
		m_bLogToDb = false;
	}

	m_bMapLoaded = TRUE;
	time_t now;
	time( &now );	
	SecondsToTimeStamp(now, m_MapTime);
	DeleteOldLogs();
}


////////////////////////////////////////////////////////////////////////////////
// 
// Log
// 
////////////////////////////////////////////////////////////////////////////////
void CLogWriter::Log( long ErrorCode, wchar_t* wcsText )
{
	CLogEntry logEntry( ErrorCode, wcsText );

	m_Queue.push_back( logEntry );
	
	try
	{
		if(!HeapFree( GetProcessHeap(), 0, wcsText ))
		{
		}
	}
	catch(...){}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Log
// 
//////////////////////////////////////////////////////////////////////////////
void CLogWriter::Log( long ErrorCode, unsigned int StringID )
{
	CEMSString s;	
	s.LoadString( StringID );

	CLogEntry logEntry( ErrorCode, (wchar_t*)s.c_str() );

	m_Queue.push_back( logEntry );
}



////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CLogWriter::Run(void)
{
	if( m_Queue.size() == 0 )
	{
		// Flush the file if we have nothing else to do 
		// (and we haven't done a flush since the last write)
		if( m_bDirty )
		{
			FlushFileBuffers( m_FileHandle );
			m_bDirty = FALSE;
		}

		return S_FALSE;
	}
	
	HRESULT hRet = (m_Queue.size() > 0) ? S_OK : S_FALSE;

	DebugReporter::Instance().DisplayMessage("CLogWriter::Run", DebugReporter::ENGINE);		
	
	while( m_Queue.size() > 0 )
	{
		WriteLogEntry( m_Queue.front() );

		// Remove the item from the queue
		m_Queue.pop_front();
	}

	return hRet;
}

////////////////////////////////////////////////////////////////////////////////
// 
// WriteLogEntry
// 
////////////////////////////////////////////////////////////////////////////////
void CLogWriter::WriteLogEntry( CLogEntry& entry )
{
	UINT LogEntryTypeID;
	UINT LogSeverityID;

	// Make sure the map is loaded
	if ( !m_bMapLoaded )
	{
		LoadMap();
	}

	// extract the LogEntryTypeID from the error code
	LogEntryTypeID = (entry.m_ErrorCode & 0x00ff0000) >> 16;
	
	if( LogEntryTypeID < 1 || LogEntryTypeID > m_Map.size() )
	{
		return;
	}

	LogSeverityID = (entry.m_ErrorCode & 0xf0000000) >> 28;

	if( LogSeverityID < 1 || LogSeverityID > m_SevMap.size() )
	{
		return;
	}

	if( m_Map[ LogEntryTypeID - 1].m_SeverityLevels & (1 << (LogSeverityID-1)) ) 
	{
		wstring::size_type pos;
		
		BOOL bDoDB = m_bLogToDb; 
		
		//Logic to detect Inbound Messaging SQLAllocHandle error and restart mailcomponents.dll
		if(LogEntryTypeID == 2 && LogSeverityID == 3)
		{
			try
			{
				pos = 0;
				dca::String sTemp(entry.m_Text.c_str());
				dca::String::size_type pos2 = sTemp.find("SQLAllocHandle");
				if(pos2 != dca::String::npos)
				{
					int retval;
					dca::String t;

					WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"RestartMailComponents", 5 );

					retval = g_Object.m_MessagingComponents.Shutdown();

					if(retval == 0)
					{
						t.Format("CLogWriter::WriteLogEntry - Successfully unloaded mailcomponents.dll due to SQLAllocHandle error - mail send and receive is disabled.");
						DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
							
						retval = g_Object.m_MessagingComponents.LoadDLL();							

						if( retval )
						{
							CEMSString s;
							s.LoadString( EMS_STRING_ERROR_LOADING_MAILCOMPONENTS );
							_Module.LogEvent( s.c_str(), retval );
							Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, L"Error %d loading mailcomponents.dll - mail send and receive is disabled.", retval );
							t.Format("CLogWriter::WriteLogEntry - Error %d loading mailcomponents.dll - mail send and receive is disabled.", retval);
							DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
							retval = 0;
						}
						else
						{
							t.Format("CLogWriter::WriteLogEntry - Successfully loaded mailcomponents.dll");
							DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);

							retval = g_Object.m_MessagingComponents.Initialize();
							if(retval)
							{
								t.Format("CLogWriter::WriteLogEntry - Error initializing mailcomponents.dll.");
								DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);		
							}
							else
							{
								t.Format("CLogWriter::WriteLogEntry - Successfully initialized mailcomponents.dll.");
								DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);
								WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, L"RestartMailComponents", 0 );
							}
						}
					}
					else
					{
						t.Format("CLogWriter::WriteLogEntry - Failed to unload mailcomponents.dll due to SQLAllocHandle error - mail send and receive may be disabled.");
						DebugReporter::Instance().DisplayMessage(t.c_str(), DebugReporter::ENGINE);							
					}						
				}
			}
			catch(...)
			{
				DebugReporter::Instance().DisplayMessage("Caught exception when shutting down mailcomponents.dll due to SQLAllocHandle error - mail send and receive may be disabled.", DebugReporter::ENGINE);
			}
		}
		
		if ( bDoDB )
		{			
			try
			{
				// Is it time to clear old log entries?
				if( ( (entry.m_TimeStamp.year != m_MapTime.year)
					|| (entry.m_TimeStamp.day != m_MapTime.day)
					|| (entry.m_TimeStamp.month != m_MapTime.month) ) )
				{
					time_t now;
					time( &now );	
					SecondsToTimeStamp(now, m_MapTime);
					DeleteOldLogs();
				}			

				pos = 0;
				// Replace all double quotes with single quotes
				while( (pos = entry.m_Text.find( L'"', pos)) != wstring::npos )
				{
					entry.m_Text[pos] = L'\'';
				}

				pos = 0;
				// Remove any carriage returns
				while( (pos = entry.m_Text.find( L'\r', pos)) != wstring::npos )
				{
					entry.m_Text.erase( pos, 1 );
				}

				pos = 0;
				// Remove any linefeeds
				while( (pos = entry.m_Text.find( L'\n', pos)) != wstring::npos )
				{
					entry.m_Text.erase( pos, 1 );
				}

				// Make sure the text isn't too big
				if( entry.m_Text.size() > 255 )
				{
					entry.m_Text.resize( 252 );
					entry.m_Text += L"...";
				}

				TLog m_log;
				m_log.m_LogTime = entry.m_TimeStamp;
				m_log.m_ServerID = m_nServerID;
				m_log.m_ErrorCode = entry.m_ErrorCode;
				m_log.m_LogSeverityID = LogSeverityID;
				m_log.m_LogEntryTypeID = LogEntryTypeID;
				ZeroMemory(m_log.m_LogText, LOG_TEXT_LENGTH);
				lstrcpyn(m_log.m_LogText, entry.m_Text.c_str(), LOG_TEXT_LENGTH - 1);
				m_log.Insert(m_query);
			}
			catch( ... )
			{
				bDoDB = false;
			}

		}

		if ( !bDoDB )
		{
			char szBuffer[512];
			DWORD dwBytesWritten;
			
			// Is it time to start another log file?
			if( (m_FileHandle != INVALID_HANDLE_VALUE) &&
				( (entry.m_TimeStamp.year != m_LogFileTime.year)
				|| (entry.m_TimeStamp.day != m_LogFileTime.day)
				|| (entry.m_TimeStamp.month != m_LogFileTime.month) ) )
			{
				CloseLogFile();
				DeleteOldLogs();
			}

			m_LogFileTime = entry.m_TimeStamp;

			if( m_FileHandle == INVALID_HANDLE_VALUE)
			{
				OpenLogFile();
			}

			if( m_FileHandle != INVALID_HANDLE_VALUE)
			{

				pos = 0;
				// Replace all double quotes with single quotes
				while( (pos = entry.m_Text.find( L'"', pos)) != wstring::npos )
				{
					entry.m_Text[pos] = L'\'';
				}

				pos = 0;
				// Remove any carriage returns
				while( (pos = entry.m_Text.find( L'\r', pos)) != wstring::npos )
				{
					entry.m_Text.erase( pos, 1 );
				}

				pos = 0;
				// Remove any linefeeds
				while( (pos = entry.m_Text.find( L'\n', pos)) != wstring::npos )
				{
					entry.m_Text.erase( pos, 1 );
				}

				// Make sure the text isn't too big
				if( entry.m_Text.size() > 255 )
				{
					entry.m_Text.resize( 252 );
					entry.m_Text += L"...";
				}

				_snprintf( szBuffer, 511, "\"%02d:%02d:%02d %d/%d/%d\",\"%d\",\"%S\",\"%S\",\"%S\"\r\n",
						entry.m_TimeStamp.hour,
						entry.m_TimeStamp.minute,
						entry.m_TimeStamp.second,
						entry.m_TimeStamp.month,
						entry.m_TimeStamp.day,
						entry.m_TimeStamp.year,
						entry.m_ErrorCode,
						m_SevMap[ LogSeverityID - 1].m_Description,
						m_Map[ LogEntryTypeID - 1].m_TypeDescrip,
						entry.m_Text.c_str() );

				// Null terminate to be safe
				szBuffer[511] = '\0';
				
				WriteFile( m_FileHandle, szBuffer, strlen(szBuffer), &dwBytesWritten, NULL );

				m_bDirty = TRUE;
				
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// OpenLogFile
// 
////////////////////////////////////////////////////////////////////////////////
void CLogWriter::OpenLogFile(void)
{
	wchar_t wcsPath[MAX_PATH];
	tstring tstrPath;
	GetLogFilePath( tstrPath );

	// Create the log file name: LOGYYYYMMDD
	swprintf( wcsPath, L"%sLOG%04d%02d%02d.txt",
		      tstrPath.c_str(),
			  m_LogFileTime.year,
			  m_LogFileTime.month,
			  m_LogFileTime.day );

	// create file if it doesn't already exist
	m_FileHandle = CreateFile( wcsPath, GENERIC_WRITE, 
		                       FILE_SHARE_READ, NULL, 
		                       OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	if( m_FileHandle == INVALID_HANDLE_VALUE )
	{
		Alert( EMS_ALERT_EVENT_CRITICAL_ERROR, 0, _T("Error %d opening log file "), GetLastError() );
	}
	else
	{
		// Move the file pointer to the end
		SetFilePointer( m_FileHandle, 0, 0, FILE_END );
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// CloseLogFile
// 
////////////////////////////////////////////////////////////////////////////////
void CLogWriter::CloseLogFile(void)
{
	// Close log file

	if( m_FileHandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_FileHandle );
		m_FileHandle = INVALID_HANDLE_VALUE;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// DeleteOldLogs
// 
////////////////////////////////////////////////////////////////////////////////
void CLogWriter::DeleteOldLogs(void)
{
	// Delete old logs
	CEMSString  sValue;
	GetServerParameter( EMS_SRVPARAM_DELETE_LOGS, sValue );
	m_nDeleteLogs = _ttoi( sValue.c_str());

	time_t now;
	time( &now ); 
	
	SecondsToTimeStamp(now - (86400*m_nDeleteLogs), m_Time);
	m_Time.hour=0;
	m_Time.minute=0;
	m_Time.second=0;
	m_Time.fraction=0;

	if ( m_nDeleteLogs > 0 )
	{
		m_query.Initialize();
		BINDPARAM_TIME_NOLEN( m_query, m_Time );
		BINDPARAM_LONG( m_query, m_nServerID );
		m_query.Execute( _T("DELETE FROM Log WHERE LogTime<? AND ServerID=?"));		
		DeleteLogFiles();		
	}
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetServerParameter
// 
////////////////////////////////////////////////////////////////////////////////
void CLogWriter::GetServerParameter( int ID, CEMSString& sValue )
{
	TCHAR szDataValue[SERVERPARAMETERS_DATAVALUE_LENGTH];
	long szDataValueLen;

	szDataValue[0] = _T('\0');

	m_query.Initialize();
	BINDPARAM_LONG( m_query, ID );
	BINDCOL_TCHAR( m_query, szDataValue );
	m_query.Execute( _T("SELECT DataValue FROM ServerParameters WHERE ServerParameterID=?"));
	m_query.Fetch();

	sValue = szDataValue;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Search the logging directory for log files
// 
////////////////////////////////////////////////////////////////////////////////
void CLogWriter::DeleteLogFiles(void)
{
	TCHAR szPath[MAX_PATH];
	tstring tstrDir;
	CLogFilesInfo logFileInfo;
	HANDLE hFind;
		
	set<CLogFilesInfo> LogFileList;
	set<CLogFilesInfo>::iterator iter;

	GetLogFilePath( tstrDir );

	_stprintf( szPath, _T("%s*.*"), tstrDir.c_str() );

	hFind = FindFirstFile( szPath, &logFileInfo );
	
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		LogFileList.insert( logFileInfo );
		
		while( FindNextFile( hFind, &logFileInfo ) )
		{
			LogFileList.insert( logFileInfo );
		}

		FindClose( hFind );
	}

	for( iter = LogFileList.begin(); iter != LogFileList.end(); iter++ )
	{
		__int64 ix,iy;
		FILETIME ft;		
		FileTimeToLocalFileTime( &(iter->ftLastWriteTime), &ft );
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft,&st);
		SystemTimeToFileTime( &st, (FILETIME*)&iy );

		SYSTEMTIME sysTime;
		TimeStampToSystemTime(m_Time, sysTime);
		SystemTimeToFileTime( &sysTime, (FILETIME*)&ix );

		tstring sFileName;
		sFileName = iter->cFileName;
		int nFileNameLength = sFileName.length();
			
		if ((iy - ix) < 0 && nFileNameLength > 3)
		{
			DeleteLogFile(sFileName);
		}

	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetLogFilePath
// 
////////////////////////////////////////////////////////////////////////////////
void CLogWriter::GetLogFilePath( tstring& tstrPath )
{
	// Check the registry for a "LogPath" named value
	if ( GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               EMS_LOG_PATH_VALUE, tstrPath ) != ERROR_SUCCESS)
	{
		wchar_t szPath[MAX_PATH];
		wchar_t drive[_MAX_DRIVE];
		wchar_t dir[_MAX_DIR];

		// Get path to this exe
		GetModuleFileName( NULL, szPath, MAX_PATH );

		// split path into components
		_wsplitpath( szPath, drive, dir, NULL, NULL );

		// re-assemble to get path to log file
		_wmakepath( szPath, drive, dir, NULL, NULL );

		tstrPath = szPath;
	}

	// Make sure we have a backslash at the end.
	if( tstrPath.length() > 0 && tstrPath.at( tstrPath.length() - 1 ) != L'\\' )
	{
		tstrPath += L"\\";
	}
}

void CLogWriter::DeleteLogFile( tstring& sFileName )
{
	tstring tstrDir;
	wchar_t szLog[512];
	wmemset(szLog, 0x00, 512);

	GetLogFilePath( tstrDir );
	tstrDir += sFileName;
	DeleteFile( tstrDir.c_str());

	swprintf( szLog, L"Deleted old log file %s", sFileName.c_str());
	Log( E_DBMaintenanceInfo, (wchar_t*) szLog );
}
