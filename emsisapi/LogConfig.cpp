// LogConfig.cpp: implementation of the CLogConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogConfig.h"
#include "RegistryFns.h"
#include "DateFns.h"

extern HINSTANCE g_hInstance;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogConfig::CLogConfig( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nServerID = 0;
	m_nLogEntryTypeID = 0;
	sStartHr = "00";
	sStartMin = "00";
	sStartAmPm = "AM";
	sStopHr = "11";
	sStopMin = "59";
	sStopAmPm = "PM";
	m_FileHandle = INVALID_HANDLE_VALUE;
}

CLogConfig::~CLogConfig()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
//  The main entry point
// 
////////////////////////////////////////////////////////////////////////////////
int CLogConfig::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	tstring sFileName;
	
	// Check security
	RequireAdmin();
	
	if( !GetISAPIData().GetURLString( _T("Action"), sAction, true ) )
		GetISAPIData().GetFormString( _T("Action"), sAction, true );
	
	
	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("showlogs") ) == 0 
		|| _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("showlogdetails") ) == 0
		|| _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("showdblogdetails") ) == 0)
	{			
		if( GetISAPIData().GetURLString( _T("selectId"), sFileName, true ) )
		{
			
			if( GetISAPIData().GetURLLong( _T("LogServerId"), m_nServerID, true ) )
			{
				if( m_nServerID > 0 )
				{
					if( GetISAPIData().GetURLString( _T("LogDate"), m_sLogDate, true ) )
					{
						if ( m_sLogDate != "" )
						{
							if( sAction.compare( _T("delete") ) == 0 )
							{
								DeleteDBLogs();
								action.SetRedirectURL( _T("showdblogs.ems") );
								return 0;
							}
							else if( sAction.compare( _T("savedblog") ) == 0 )
							{
								SaveDBLogs( action );
								action.SetRedirectURL( _T("showlogs.ems") );
								return 0;
							}
							else
							{
								return ShowDBLogs( action );
							}
						}
						else
						{
							return ListDBDates();
						}
					}
					else
					{
						return ListDBDates();
					}
				}
				else
				{
					return ListDBDates();
				}
			}			
						
			if( sFileName.length() > 0 )
			{
				// Hack off the comma at the end
				if( sFileName.length() > 0 && sFileName.at( sFileName.length() - 1 ) == _T(',') )
					sFileName.resize( sFileName.length() - 1 );
				
				if( sAction.compare( _T("delete") ) == 0 )
				{
					DISABLE_IN_DEMO();
					CEMSString sTemp(sFileName.c_str());
					while ( sTemp.CDLGetNextString( sFileName ) )
					{
						Delete( sFileName );
					}					
					action.SetRedirectURL( _T("showlogs.ems") );
					return 0;
				}
				else if( sAction.compare( _T("deletedebuglog") ) == 0 )
				{
					DISABLE_IN_DEMO();
					CEMSString sTemp(sFileName.c_str());
					while ( sTemp.CDLGetNextString( sFileName ) )
					{
						Delete( sFileName );
					}					
					action.SetRedirectURL( _T("showdebuglogs.ems") );
					return 0;
				}
				else 
				{
					return ShowLogs( sFileName, action );
				}
			}
			else
			{
				return ListDates();
			}
		}
		else if( GetISAPIData().GetURLString( _T("LogFile"), sFileName, true ) )
		{
			return ShowLogs( sFileName, action );
		}
		
		return ListDates();
	}
	else if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("showdblogs") ) == 0 )
	{
		return ListDBDates();
	}
	else if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("showdebuglogs") ) == 0 )
	{
		return ListDebugLogs();
	}
	
	if( sAction.compare( _T("update") ) == 0 )
	{
		DISABLE_IN_DEMO();
		Update( action );
	}
	
	ListAll();
	
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// List the logging configuration
// 
////////////////////////////////////////////////////////////////////////////////
int CLogConfig::ListAll(void)
{
	TLogEntryTypes logentry;
	tstring sLogPath;
	tstring sValue;

	GetQuery().Initialize();

	logentry.PrepareList( GetQuery() );

	while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem(_T("LogEntryType"));
		GetXMLGen().SetChildAttrib( _T("ID"), logentry.m_LogEntryTypeID );
		GetXMLGen().SetChildAttrib( _T("Description"), logentry.m_TypeDescrip );
		GetXMLGen().SetChildAttrib( _T("Informational"), (logentry.m_SeverityLevels & 0x1) );		
		GetXMLGen().SetChildAttrib( _T("Warning"), (logentry.m_SeverityLevels & 0x2) >> 1 );		
		GetXMLGen().SetChildAttrib( _T("Error"), (logentry.m_SeverityLevels & 0x4) >> 2 );		
		GetXMLGen().SetChildAttrib( _T("CriticalError"), (logentry.m_SeverityLevels & 0x8) >> 3 );		
	}

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_LOG_PATH_VALUE, sLogPath );
	
	UINT nDebugLogValue;
	CEMSString sVal;

	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               _T("DebugLogEngine"), nDebugLogValue ) == ERROR_SUCCESS)
	{
		sVal.Format(_T("%d"),nDebugLogValue);		
		GetXMLGen().AddChildElem( _T("DebugLogEngine") );
		GetXMLGen().SetChildData( sVal.c_str(), 1 );
	}
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               _T("DebugLogMail"), nDebugLogValue ) == ERROR_SUCCESS)
	{
		sVal.Format(_T("%d"),nDebugLogValue);
		GetXMLGen().AddChildElem( _T("DebugLogMail") );
		GetXMLGen().SetChildData( sVal.c_str(), 1 );
	}
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               _T("DebugLogWeb"), nDebugLogValue ) == ERROR_SUCCESS)
	{
		sVal.Format(_T("%d"),nDebugLogValue);
		GetXMLGen().AddChildElem( _T("DebugLogWeb") );
		GetXMLGen().SetChildData( sVal.c_str(), 1 );
	}
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               _T("DebugLogLib"), nDebugLogValue ) == ERROR_SUCCESS)
	{
		sVal.Format(_T("%d"),nDebugLogValue);
		GetXMLGen().AddChildElem( _T("DebugLogLib") );
		GetXMLGen().SetChildData( sVal.c_str(), 1 );
	}

	GetXMLGen().AddChildElem( _T("LogPath") );
	GetXMLGen().SetChildData( sLogPath.c_str(), 1 );

	TServerParameters servParams;
	servParams.m_ServerParameterID = EMS_SRVPARAM_DATABASE_LOGGING;

	int nResult = servParams.Query(GetQuery());

	if(nResult != S_OK)
		lstrcpy(servParams.m_DataValue, _T("0"));

	GetXMLGen().AddChildElem( _T("LogToDb") );
	GetXMLGen().SetChildData( servParams.m_DataValue, 1 );

	servParams.m_ServerParameterID = EMS_SRVPARAM_DELETE_LOGS;

	nResult = servParams.Query(GetQuery());

	if(nResult != S_OK)
		lstrcpy(servParams.m_DataValue, _T("0"));

	GetXMLGen().AddChildElem( _T("DeleteLogs") );
	GetXMLGen().SetChildData( servParams.m_DataValue, 1 );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
//  Update the logging configuration
// 
////////////////////////////////////////////////////////////////////////////////
int CLogConfig::Update(CURLAction& action)
{
	CEMSString s;
	int nLogSeverityIDCnt = GetXMLCache().m_LogSeverity.GetListSize();
	int nLogEntryIDCnt = GetXMLCache().m_LogEntryTypes.GetListSize();
	int i,j;
	unsigned char nValue;
	int LogEntryID;
	int SeverityLevels;
	tstring sLogPath;

	std::string sLogToDb;
	if(!GetISAPIData().GetFormString("logtodb",sLogToDb, true))
		sLogToDb.assign(_T("off"));

	if(!sLogToDb.compare(_T("off")))
		sLogToDb.assign(_T("0"));
	else
		sLogToDb.assign(_T("1"));

	int nDebugLogEngine;
	if(!GetISAPIData().GetFormLong("debuglogengine",nDebugLogEngine, true))
		nDebugLogEngine = 0;

	WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("DebugLogEngine"), nDebugLogEngine );

	int nDebugLogMail;
	if(!GetISAPIData().GetFormLong("debuglogmail",nDebugLogMail, true))
		nDebugLogMail = 0;

	WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("DebugLogMail"), nDebugLogMail );

	int nDebugLogWeb;
	if(!GetISAPIData().GetFormLong("debuglogweb",nDebugLogWeb, true))
		nDebugLogWeb = 0;

	WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("DebugLogWeb"), nDebugLogWeb );

	int nDebugLogLib;
	if(!GetISAPIData().GetFormLong("debugloglib",nDebugLogLib, true))
		nDebugLogLib = 0;

	WriteRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("DebugLogLib"), nDebugLogLib );

	GetISAPIData().GetFormString( _T("logPath"), sLogPath );
	VerifyDirectoryExists( sLogPath );
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_LOG_PATH_VALUE, 
		            sLogPath.c_str() );
	//SetServerParameter( EMS_SRVPARAM_LOG_PATH, sLogPath );	
	
	int m_ServerParameterID = EMS_SRVPARAM_DATABASE_LOGGING;
	long m_ServerParameterIDLen = 0;
	TCHAR m_DataValue[SERVERPARAMETERS_DATAVALUE_LENGTH];
	long m_DataValueLen = 0;

	ZeroMemory(m_DataValue, SERVERPARAMETERS_DATAVALUE_LENGTH);

	lstrcpyn(m_DataValue, sLogToDb.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1);

	GetQuery().Initialize();
	BINDPARAM_TCHAR(GetQuery(), m_DataValue );
	BINDPARAM_LONG(GetQuery(), m_ServerParameterID );
	GetQuery().Execute( _T("UPDATE ServerParameters ")
						_T("SET DataValue=? ")
						_T("WHERE ServerParameterID=?") );

	std::string sDeleteLogs;
	if(!GetISAPIData().GetFormString("deletelogs",sDeleteLogs, true))
		sDeleteLogs.assign(_T("0"));

	m_ServerParameterID = EMS_SRVPARAM_DELETE_LOGS;
	m_ServerParameterIDLen = 0;
	m_DataValueLen = 0;

	ZeroMemory(m_DataValue, SERVERPARAMETERS_DATAVALUE_LENGTH);

	lstrcpyn(m_DataValue, sDeleteLogs.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1);

	GetQuery().Initialize();
	BINDPARAM_TCHAR(GetQuery(), m_DataValue );
	BINDPARAM_LONG(GetQuery(), m_ServerParameterID );
	GetQuery().Execute( _T("UPDATE ServerParameters ")
						_T("SET DataValue=? ")
						_T("WHERE ServerParameterID=?") );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), SeverityLevels );
	BINDPARAM_LONG( GetQuery(), LogEntryID );

	for( i = 0; i < nLogEntryIDCnt; i++ )
	{
		SeverityLevels = 0;
		for( j = 0; j < nLogSeverityIDCnt; j++ )
		{	
			s.Format( _T("%d.%d"), i+1, j+1 );
			GetISAPIData().GetFormBit( s.c_str(), nValue );

			if( nValue )
			{
				SeverityLevels |= (1 << j);
			}
		}
		LogEntryID = i+1;

		GetQuery().Execute( _T("UPDATE LogEntryTypes ")
						   _T("SET SeverityLevels=? ")
						   _T("WHERE LogEntryTypeID=?") );

		GetQuery().Reset( false );
	}

	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_LogConfig );
	g_ThreadPool.ReLoadAllThreads();
	InvalidateServerParameters(true);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetLogFilePath
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::GetLogFilePath( tstring& tstrPath )
{
	// Check the registry for a "LogPath" named value
	if ( GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               EMS_LOG_PATH_VALUE, tstrPath ) != ERROR_SUCCESS)
	{
		TCHAR szPath[MAX_PATH];
		TCHAR drive[_MAX_DRIVE];
		TCHAR dir[_MAX_DIR];

		// Get path to this exe
		GetModuleFileName( g_hInstance, szPath, MAX_PATH );

		// split path into components
		_tsplitpath( szPath, drive, dir, NULL, NULL );

		// re-assemble to get path to log file
		_tmakepath( szPath, drive, dir, NULL, NULL );

		tstrPath = szPath;
	}

	// Make sure we have a backslash at the end.
	if( tstrPath.length() > 0 && tstrPath.at( tstrPath.length() - 1 ) != _T('\\') )
	{
		tstrPath += _T("\\");
	}
}

/*//////////////////////////////////////////////////////////////////////////////
// 
// Get information about the log file and add to XML
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::GetLogFileInfo( WIN32_FIND_DATA* pFindData )
{
	FILETIME ft;
	SYSTEMTIME st;
	CEMSString sDateTime;
	CEMSString sBytes;

	GetXMLGen().AddChildElem(_T("LogFile"));
	GetXMLGen().AddChildAttrib( _T("FileName"), pFindData->cFileName );

	FileTimeToLocalFileTime( &(pFindData->ftLastWriteTime), &ft );
	FileTimeToSystemTime( &ft, &st);
	
	GetDateTimeString( st, sDateTime );

	GetXMLGen().AddChildAttrib( _T("Date"), sDateTime.c_str() );

	sBytes.FormatBytes( pFindData->nFileSizeLow );
	GetXMLGen().AddChildAttrib( _T("Bytes"), sBytes.c_str() ); 
} */

////////////////////////////////////////////////////////////////////////////////
// 
// Search the logging directory for log files and return XML 
// 
////////////////////////////////////////////////////////////////////////////////
int CLogConfig::ListDates(void)
{
	TCHAR szPath[MAX_PATH];
	tstring tstrDir;
	CLogFileInfo logFileInfo;
	HANDLE hFind;
	FILETIME ft;
	SYSTEMTIME st;
	CEMSString sDateTime;
	CEMSString sBytes;
	
	set<CLogFileInfo> LogFileList;
	set<CLogFileInfo>::iterator iter;

	GetLogFilePath( tstrDir );

	_stprintf( szPath, _T("%sLOG*.txt"), tstrDir.c_str() );

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
		GetXMLGen().AddChildElem(_T("LogFile"));
		GetXMLGen().AddChildAttrib( _T("FileName"), iter->cFileName );

		FileTimeToLocalFileTime( &(iter->ftLastWriteTime), &ft );
		FileTimeToSystemTime( &ft, &st);
		
		GetDateTimeString( st, sDateTime );

		GetXMLGen().AddChildAttrib( _T("Date"), sDateTime.c_str() );

		sBytes.FormatBytes( iter->nFileSizeLow, iter->nFileSizeHigh );
		GetXMLGen().AddChildAttrib( _T("Bytes"), sBytes.c_str() ); 
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Search the logging directory for debug log files and return XML 
// 
////////////////////////////////////////////////////////////////////////////////
int CLogConfig::ListDebugLogs(void)
{
	TCHAR szPath[MAX_PATH];
	tstring tstrDir;
	CLogFileInfo logFileInfo;
	HANDLE hFind;
	FILETIME ft;
	SYSTEMTIME st;
	CEMSString sDateTime;
	CEMSString sBytes;
	
	set<CLogFileInfo> LogFileList;
	set<CLogFileInfo>::iterator iter;

	GetLogFilePath( tstrDir );

	_stprintf( szPath, _T("%s*.log"), tstrDir.c_str() );

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
		GetXMLGen().AddChildElem(_T("LogFile"));
		GetXMLGen().AddChildAttrib( _T("FileName"), iter->cFileName );

		FileTimeToLocalFileTime( &(iter->ftLastWriteTime), &ft );
		FileTimeToSystemTime( &ft, &st);
		
		GetDateTimeString( st, sDateTime );

		GetXMLGen().AddChildAttrib( _T("Date"), sDateTime.c_str() );

		sBytes.FormatBytes( iter->nFileSizeLow, iter->nFileSizeHigh );
		GetXMLGen().AddChildAttrib( _T("Bytes"), sBytes.c_str() ); 
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Search the logging directory for log files and return XML 
// 
////////////////////////////////////////////////////////////////////////////////
int CLogConfig::ListDBDates(void)
{
	int nMonth;
	int nDay;
	int nYear;
	CEMSString sMonth;
	CEMSString sDay;
		
	
	GetISAPIData().GetURLLong( _T("ServerId"), m_nServerID, true );
	    
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN(GetQuery(), nMonth);
	BINDCOL_LONG_NOLEN(GetQuery(), nDay);
	BINDCOL_LONG_NOLEN(GetQuery(), nYear);
	BINDCOL_TCHAR(GetQuery(), m_Description);
	BINDCOL_LONG_NOLEN(GetQuery(), m_nServerLogID);
	BINDCOL_LONG_NOLEN(GetQuery(), m_nNumRows );
	if ( m_nServerID > 0 )
	{
		BINDPARAM_LONG(GetQuery(), m_nServerID );
		GetQuery().Execute( _T("SELECT MONTH(l.LogTime) as Month, DAY(l.LogTime) as 'Day', ")
						_T("YEAR(l.LogTime) as 'Year', s.Description, l.ServerID, COUNT(*) As 'NumRows' ")
						_T("FROM Log l INNER JOIN Servers s ON l.ServerID=s.ServerID WHERE l.ServerID=? ")
						_T("GROUP BY DAY(l.LogTime), MONTH(l.LogTime), YEAR(l.LogTime), s.Description, l.ServerID ")
						_T("ORDER BY YEAR(l.LogTime) DESC, MONTH(l.LogTime) DESC, DAY(l.LogTime) DESC") );
	}
	else
	{
		GetQuery().Execute( _T("SELECT MONTH(l.LogTime) as Month, DAY(l.LogTime) as 'Day', ")
						_T("YEAR(l.LogTime) as 'Year', s.Description, l.ServerID, COUNT(*) As 'NumRows' ")
						_T("FROM Log l INNER JOIN Servers s ON l.ServerID=s.ServerID ")
						_T("GROUP BY DAY(l.LogTime), MONTH(l.LogTime), YEAR(l.LogTime), s.Description, l.ServerID ")
						_T("ORDER BY YEAR(l.LogTime) DESC, MONTH(l.LogTime) DESC, DAY(l.LogTime) DESC") );
	}
	
	int x=1;
    while( GetQuery().Fetch() == S_OK )
	{
		if(nMonth<10)
		{
			sMonth.Format(_T("0%d"),nMonth);
		}
		else
		{
			sMonth.Format(_T("%d"),nMonth);
		}
		if(nDay<10)
		{
			sDay.Format(_T("0%d"),nDay);
		}
		else
		{
			sDay.Format(_T("%d"),nDay);
		}
		
		GetXMLGen().AddChildElem( _T("DBLog") );
		GetXMLGen().AddChildAttrib( _T("ROWID"), x );
		GetXMLGen().AddChildAttrib( _T("Month"), sMonth.c_str() );
		GetXMLGen().AddChildAttrib( _T("Day"), sDay.c_str() );
		GetXMLGen().AddChildAttrib( _T("Year"), nYear );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetXMLGen().AddChildAttrib( _T("ServerID"), m_nServerLogID );
		GetXMLGen().AddChildAttrib( _T("NumRows"), m_nNumRows );		

		x++;
	}

	GetQuery().Initialize();
	BINDCOL_TCHAR(GetQuery(), m_Description);
	BINDCOL_LONG_NOLEN(GetQuery(), m_nServerLogID);	
	GetQuery().Execute( _T("SELECT Description,ServerID FROM Servers") );
		
	GetXMLGen().AddChildElem( _T("Servers") );
	GetXMLGen().IntoElem();
	
    while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Server") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_nServerLogID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	}

	GetXMLGen().OutOfElem();

	GetXMLGen().AddChildAttrib( _T("ServerID"), m_nServerID );
	return 0;
}

void CLogConfig::Delete( tstring& sFileName )
{
	tstring tstrDir;
	CEMSString sError;

	GetLogFilePath( tstrDir );

	tstrDir += sFileName;

	if( DeleteFile( tstrDir.c_str() ) == FALSE )
	{
		TCHAR* lpszTemp = NULL;
	
		// Let the system allocate a string to describe the error code in dwRet
		FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER |	// specifies that the lpBuffer parameter is a pointer to a PVOID pointer, and that the nSize parameter specifies the minimum number of TCHARs to allocate for an output message buffer
						FORMAT_MESSAGE_FROM_SYSTEM |		// specifies that the function should search the system message-table resource(s) for the requested message
						FORMAT_MESSAGE_ARGUMENT_ARRAY,		// specifies that the Arguments parameter is not a va_list structure, but instead is just a pointer to an array of values that represent the arguments
						NULL,								// message source
						GetLastError(),								// message identifier					
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 	// default language
						(LPTSTR)&lpszTemp,					// message buffer					
						0,									// maximum size of message buffer
						NULL );								// array of message inserts	
	
		sError.assign( _T("Error deleting log file:") );

		if (lpszTemp)
		{
			sError.append( lpszTemp );
			// copy to our string buff and free memory
			LocalFree((HLOCAL)lpszTemp);
		}

		THROW_EMS_EXCEPTION( E_SystemError, sError.c_str() );
	}	
}




////////////////////////////////////////////////////////////////////////////////
// 
// ShowLogs for a particular log file with optional filters
// 
////////////////////////////////////////////////////////////////////////////////
int CLogConfig::ShowLogs( tstring& sFileName, CURLAction& action )
{
	tstring tstrDir;
	HANDLE hFile;
	TCHAR szBuffer[8192];
	DWORD dwBytes = 0;
	tstring sValue;
	CEMSString sPageTitle;

	ZeroMemory( SeverityMask, sizeof(SeverityMask) );

	// TODO: Store log report options in session

	sPageTitle.Format( _T("Contents of: %s"), sFileName.c_str() );
	action.m_sPageTitle.assign( sPageTitle );

	m_bContinue = true;
	f = szFieldBuffer;
	bInField = FALSE;
	FieldNdx = 0;

	m_nCurrentLine = 0;
	m_nLineCount = 0;
	m_nPage = 1;
	m_nStartLine = 0;
	m_nLinesOut = 0;
	nComponentFilter = 0;

	nMaxRecs = GetSession().m_nMaxRowsPerPage;

	if( GetISAPIData().GetURLString( _T("LogEntryType"), sLogEntryTypeFilter, true ) == false )
	{
		SeverityMask[1] = SeverityMask[2] = SeverityMask[3] = SeverityMask[4] = 1;
	}

	if( GetISAPIData().GetURLString( _T("SevInfo"), sValue, true ) )
	{
		SeverityMask[1] = 1;
	}
		
	if( GetISAPIData().GetURLString( _T("SevWarning"), sValue, true ) )
	{
		SeverityMask[2] = 1;
	}

	if( GetISAPIData().GetURLString( _T("SevError"), sValue, true ) )
	{
		 SeverityMask[3] = 1;
	}

	if( GetISAPIData().GetURLString( _T("SevCritical"), sValue, true ) )
	{
		SeverityMask[4] = 1;
	}
	
	GetISAPIData().GetURLLong( _T("Component"), nComponentFilter, true );
	GetISAPIData().GetURLLong( _T("LineCount"), m_nLineCount, true );
	GetISAPIData().GetURLLong( _T("CurrentPage"), m_nPage, true );

	GetStartTime();

	m_bCountLines = ( m_nLineCount == 0 ) ? TRUE : FALSE;


	if( m_bCountLines == FALSE )
	{
		ComputeStartLine();
	}
	
	GetLogFilePath( tstrDir );

	tstrDir += sFileName;

	hFile = CreateFile( tstrDir.c_str(), GENERIC_READ, 
		                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
		                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if( hFile != NULL && hFile != INVALID_HANDLE_VALUE )
	{
		try
		{
			m_pLines = new CLogLine[nMaxRecs];

			while( m_bContinue && ReadFile( hFile, szBuffer, 8192, 
										  &dwBytes, NULL ) && (dwBytes > 0) )
			{
				ProcessBuffer( szBuffer, dwBytes );
			}

			if( m_bCountLines && m_nCurrentLine > 0 )
			{
				m_bContinue = true;
				m_bCountLines = FALSE;
				m_nLineCount = m_nCurrentLine;
				m_nCurrentLine = 0;
				f = szFieldBuffer;
				bInField = FALSE;
				FieldNdx = 0;

				ComputeStartLine();
				SetFilePointer( hFile, 0, 0, FILE_BEGIN );

				while( m_bContinue && ReadFile( hFile, szBuffer, 8192, 
											  &dwBytes, NULL ) && (dwBytes > 0) )
				{
					ProcessBuffer( szBuffer, dwBytes );
				}
			}

			OutputLines();

			delete[] m_pLines;
		}
		catch(...)
		{
			CloseHandle( hFile );
			delete[] m_pLines;
			throw;
		}	

		CloseHandle( hFile );
	}


	
	// Spit back the options
	GetXMLGen().AddChildElem( _T("Options") );
	GetXMLGen().AddChildAttrib( _T("selectId"), sFileName.c_str() );
	GetXMLGen().AddChildAttrib( _T("SevInfo"), SeverityMask[1] );
	GetXMLGen().AddChildAttrib( _T("SevWarning"), SeverityMask[2] );
	GetXMLGen().AddChildAttrib( _T("SevError"), SeverityMask[3] );
	GetXMLGen().AddChildAttrib( _T("SevCritical"), SeverityMask[4] );

	GetXMLGen().AddChildAttrib( _T("LogEntryType"), sLogEntryTypeFilter.c_str() );
	GetXMLGen().AddChildAttrib( _T("Component"), nComponentFilter );
	GetXMLGen().AddChildAttrib( _T("Time"), m_sTime.c_str() );

	GetXMLGen().AddChildElem( _T("Page") );
	GetXMLGen().AddChildAttrib( _T("LineCount"), m_nLineCount );
	GetXMLGen().AddChildAttrib( _T("Current"), m_nPage );
	GetXMLGen().AddChildAttrib( _T("Count"), (m_nLineCount == 0) ? 1 : (m_nLineCount + nMaxRecs-1) / nMaxRecs );

	ListLogSeverities();
	ListLogEntryTypes();	

	GetXMLGen().AddChildElem( _T("Components") );
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("Component") );
	GetXMLGen().AddChildAttrib( _T("ID"), 0 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("All") );
	GetXMLGen().AddChildElem( _T("Component") );
	GetXMLGen().AddChildAttrib( _T("ID"), 1 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Routing Engine") );
	GetXMLGen().AddChildElem( _T("Component") );
	GetXMLGen().AddChildAttrib( _T("ID"), 2 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Messaging Components") );
	GetXMLGen().AddChildElem( _T("Component") );
	GetXMLGen().AddChildAttrib( _T("ID"), 3 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("ISAPI Extension") );
	GetXMLGen().OutOfElem();


	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Save DB Log for a particular server and date
// 
////////////////////////////////////////////////////////////////////////////////
int CLogConfig::SaveDBLogs( CURLAction& action )
{
	char szBuffer[512];
	DWORD dwBytesWritten;

	CreateLogFile();	

	if( m_FileHandle == INVALID_HANDLE_VALUE)
	{
		OpenLogFile();
	}

	if( m_FileHandle != INVALID_HANDLE_VALUE)
	{
		TLog dbLog;

		CEMSString sLogDateStart;
		CEMSString sLogDateStop;
		
		sLogDateStart.Format(_T("%s 00:00:00 AM"), m_sLogDate.c_str(),sStartHr.c_str(),sStartMin.c_str(),sStartAmPm.c_str());
		sLogDateStop.Format(_T("%s 23:59:59 PM"), m_sLogDate.c_str(),sStopHr.c_str(),sStopMin.c_str(),sStopAmPm.c_str());
		
		CEMSString sQuery;
		
		sQuery.Format(_T("SELECT L.LogID,L.LogTime,L.ServerID,L.ErrorCode,L.LogSeverityID,L.LogEntryTypeID,S.Description,E.TypeDescrip,L.LogText ")
					_T("FROM Log L INNER JOIN LogSeverity S ON L.LogSeverityID=S.LogSeverityID INNER JOIN LogEntryTypes E ")
					_T("ON L.LogEntryTypeID=E.LogEntryTypeID ")_T("WHERE L.LogTime BETWEEN '%s' AND '%s' AND L.ServerID=%d"),sLogDateStart.c_str(),sLogDateStop.c_str(),m_nServerID);
		
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), dbLog.m_LogID );
		BINDCOL_TIME( GetQuery(), dbLog.m_LogTime );
		BINDCOL_LONG( GetQuery(), dbLog.m_ServerID );
		BINDCOL_LONG( GetQuery(), dbLog.m_ErrorCode );
		BINDCOL_LONG( GetQuery(), dbLog.m_LogSeverityID );
		BINDCOL_LONG( GetQuery(), dbLog.m_LogEntryTypeID );
		BINDCOL_TCHAR( GetQuery(), m_Severity );
		BINDCOL_TCHAR( GetQuery(), m_EntryType );
		BINDCOL_TCHAR( GetQuery(), dbLog.m_LogText );
		
		GetQuery().Execute( sQuery.c_str() );

		while ( GetQuery().Fetch() == S_OK )
		{
			_snprintf( szBuffer, 511, "\"%02d:%02d:%02d %d/%d/%d\",\"%d\",\"%s\",\"%s\",\"%s\"\r\n",
						dbLog.m_LogTime.hour,
						dbLog.m_LogTime.minute,
						dbLog.m_LogTime.second,
						dbLog.m_LogTime.month,
						dbLog.m_LogTime.day,
						dbLog.m_LogTime.year,
						dbLog.m_ErrorCode,
						m_Severity,
						m_EntryType,						
						dbLog.m_LogText);

				// Null terminate to be safe
				szBuffer[511] = '\0';
				
				WriteFile( m_FileHandle, szBuffer, strlen(szBuffer), &dwBytesWritten, NULL );			
					
		}

		CloseLogFile();
		
	}


	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Show DB Logs for a particular server and date
// 
////////////////////////////////////////////////////////////////////////////////
int CLogConfig::ShowDBLogs( CURLAction& action )
{
	CEMSString sPageTitle;
	tstring sValue;
	

	ZeroMemory( SeverityMask, sizeof(SeverityMask) );

	// TODO: Store log report options in session

	// Get the Server Description
	GetQuery().Initialize();
	BINDCOL_TCHAR(GetQuery(), m_Description);
	BINDPARAM_LONG(GetQuery(), m_nServerID );
	GetQuery().Execute( _T("SELECT Description FROM Servers WHERE ServerID=?") );
	if ( GetQuery().Fetch() == S_OK )
	{
		sPageTitle.Format( _T("Database Log for Server: %s Date: %s"), m_Description, m_sLogDate.c_str() );
		action.m_sPageTitle.assign( sPageTitle );
	}

	m_bContinue = true;
	f = szFieldBuffer;
	bInField = FALSE;
	FieldNdx = 0;

	m_nCurrentLine = 1;
	m_nPage = 1;
	m_nStartLine = 1;
	m_nLinesOut = 0;
	nComponentFilter = 0;

	nMaxRecs = GetSession().m_nMaxRowsPerPage;
	m_pLines = new CLogLine[nMaxRecs];

	tstring sSeverity;

	if( GetISAPIData().GetURLString( _T("LogEntryType"), sLogEntryTypeFilter, true ) == false )
	{
		SeverityMask[1] = SeverityMask[2] = SeverityMask[3] = SeverityMask[4] = 1;
		sSeverity = "1,2,3,4";
	}
	else
	{
		// Get the log entry type ID
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN(GetQuery(), m_nLogEntryTypeID);
		BINDPARAM_TCHAR_STRING(GetQuery(), sLogEntryTypeFilter );
		GetQuery().Execute( _T("SELECT LogEntryTypeID FROM LogEntryTypes WHERE TypeDescrip=?") );
		GetQuery().Fetch();
	}

	if( GetISAPIData().GetURLString( _T("SevInfo"), sValue, true ) )
	{
		SeverityMask[1] = 1;
		sSeverity = "1";
	}
		
	if( GetISAPIData().GetURLString( _T("SevWarning"), sValue, true ) )
	{
		SeverityMask[2] = 1;
		if(sSeverity.length() > 0)
		{
			sSeverity += ",2";
		}
		else
		{
			sSeverity = "2";
		}
	}

	if( GetISAPIData().GetURLString( _T("SevError"), sValue, true ) )
	{
		 SeverityMask[3] = 1;
		 if(sSeverity.length() > 0)
		{
			sSeverity += ",3";
		}
		else
		{
			sSeverity = "3";
		}
	}

	if( GetISAPIData().GetURLString( _T("SevCritical"), sValue, true ) )
	{
		SeverityMask[4] = 1;
		if(sSeverity.length() > 0)
		{
			sSeverity += ",4";
		}
		else
		{
			sSeverity = "4";
		}
	}
	
	CEMSString sSeverityIDs;
	sSeverityIDs.Format(_T("%s"),sSeverity.c_str());
	
	GetISAPIData().GetURLLong( _T("Component"), nComponentFilter, true );

	tstring sComp;
	if ( nComponentFilter == 0 )
	{
		sComp = "1,2,3,4,5,6,7,8,9,10,11";
	}
	else if ( nComponentFilter == 1 )
	{
		sComp = "3,4,6,9,11";
	}
	else if ( nComponentFilter == 2 )
	{
		sComp = "1,2,10";
	}
	else
	{
		sComp = "5,7,8";
	}

	GetISAPIData().GetURLLong( _T("LineCount"), m_nLineCount, true );
	GetISAPIData().GetURLLong( _T("CurrentPage"), m_nPage, true );

	GetDBStartTime();

	m_bCountLines = ( m_nLineCount == 0 ) ? TRUE : FALSE;

	TLog dbLog;

	CEMSString sLogDateStart;
	CEMSString sLogDateStop;
	
	sLogDateStart.Format(_T("%s %s:%s:00 %s"), m_sLogDate.c_str(),sStartHr.c_str(),sStartMin.c_str(),sStartAmPm.c_str());
	sLogDateStop.Format(_T("%s %s:%s:59 %s"), m_sLogDate.c_str(),sStopHr.c_str(),sStopMin.c_str(),sStopAmPm.c_str());
	
	CEMSString sQuery;
	if ( m_nLogEntryTypeID > 0 )
	{
		sQuery.Format(_T("SELECT COUNT(*) FROM Log ")
				  _T("WHERE LogTime BETWEEN '%s' AND '%s' AND ServerID=%d AND ")
				  _T("LogSeverityID IN (%s) AND LogEntryTypeID=%d"),sLogDateStart.c_str(),sLogDateStop.c_str(),m_nServerID,sSeverityIDs.c_str(),m_nLogEntryTypeID);
	}
	else
	{
		sQuery.Format(_T("SELECT COUNT(*) FROM Log ")
				  _T("WHERE LogTime BETWEEN '%s' AND '%s' AND ServerID=%d AND ")
				  _T("LogSeverityID IN (%s) AND LogEntryTypeID IN (%s)"),sLogDateStart.c_str(),sLogDateStop.c_str(),m_nServerID,sSeverityIDs.c_str(),sComp.c_str());
	}


	int nTotalRows;
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), nTotalRows );
	GetQuery().Execute( sQuery.c_str() );

	GetQuery().Fetch();
	m_nLineCount = nTotalRows;

	if( m_bCountLines == FALSE )
	{
		ComputeStartLine();
	}

	if ( m_nLogEntryTypeID > 0 )
	{
		sQuery.Format(_T("SELECT L.LogID,L.LogTime,L.ServerID,L.ErrorCode,L.LogSeverityID,L.LogEntryTypeID,S.Description,E.TypeDescrip,L.LogText ")
	              _T("FROM Log L INNER JOIN LogSeverity S ON L.LogSeverityID=S.LogSeverityID INNER JOIN LogEntryTypes E ")
	              _T("ON L.LogEntryTypeID=E.LogEntryTypeID ")_T("WHERE L.LogTime BETWEEN '%s' AND '%s' AND L.ServerID=%d AND ")
				  _T("L.LogSeverityID IN (%s) AND L.LogEntryTypeID=%d"),sLogDateStart.c_str(),sLogDateStop.c_str(),m_nServerID,sSeverityIDs.c_str(),m_nLogEntryTypeID);
	}
	else
	{
		sQuery.Format(_T("SELECT L.LogID,L.LogTime,L.ServerID,L.ErrorCode,L.LogSeverityID,L.LogEntryTypeID,S.Description,E.TypeDescrip,L.LogText ")
	              _T("FROM Log L INNER JOIN LogSeverity S ON L.LogSeverityID=S.LogSeverityID INNER JOIN LogEntryTypes E ")
	              _T("ON L.LogEntryTypeID=E.LogEntryTypeID ")_T("WHERE L.LogTime BETWEEN '%s' AND '%s' AND L.ServerID=%d AND ")
				  _T("L.LogSeverityID IN (%s) AND L.LogEntryTypeID IN (%s)"),sLogDateStart.c_str(),sLogDateStop.c_str(),m_nServerID,sSeverityIDs.c_str(),sComp.c_str());
	}
	
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), dbLog.m_LogID );
	BINDCOL_TIME( GetQuery(), dbLog.m_LogTime );
	BINDCOL_LONG( GetQuery(), dbLog.m_ServerID );
	BINDCOL_LONG( GetQuery(), dbLog.m_ErrorCode );
	BINDCOL_LONG( GetQuery(), dbLog.m_LogSeverityID );
	BINDCOL_LONG( GetQuery(), dbLog.m_LogEntryTypeID );
	BINDCOL_TCHAR( GetQuery(), m_Severity );
	BINDCOL_TCHAR( GetQuery(), m_EntryType );
	BINDCOL_TCHAR( GetQuery(), dbLog.m_LogText );
	
	GetQuery().Execute( sQuery.c_str() );

	while ( GetQuery().Fetch() == S_OK )
	{
		CEMSString sDateTime,sLogEntryTypeID;
		GetDateTimeString(dbLog.m_LogTime, dbLog.m_LogTimeLen, sDateTime );
		sLogEntryTypeID.Format(_T("%d"), dbLog.m_LogEntryTypeID);
				
		sDate = sDateTime.c_str();
		nErrorCode = dbLog.m_ErrorCode;
		sSeverity= m_Severity;
		sLogEntryType = m_EntryType;
		
		if(  SeverityMask[ (nErrorCode & 0xf0000000) >> 28]					
			&& (sLogEntryTypeFilter.size() == 0 || (sLogEntryTypeFilter.compare(sLogEntryType) == 0))
			&& (nComponentFilter == 0 || nComponentFilter == ((nErrorCode & 0x0f000000)>>24) ) )
		{
			
			if( m_bCountLines && m_nCurrentLine > 0 )
			{
				m_bContinue = true;
				m_bCountLines = FALSE;
				m_nLineCount = m_nCurrentLine;
				m_nCurrentLine = 0;				
			}			
			
			
			if( m_bCountLines == FALSE )
			{
				if ( m_nLinesOut >= nMaxRecs )
				{
					m_bContinue = false;
				}
				else if( m_nCurrentLine >= m_nStartLine )
				{
					// Add to output 
					m_pLines[m_nLinesOut].sDate = sDate;
					m_pLines[m_nLinesOut].nErrorCode = nErrorCode;
					m_pLines[m_nLinesOut].sLogEntryType = m_EntryType;
					m_pLines[m_nLinesOut].sSeverity = sSeverity;
					m_pLines[m_nLinesOut].sDesc = dbLog.m_LogText;
					m_nLinesOut++;					
				}
			}
			else if( StartTimePassed() )
			{
				m_bContinue = false;
			}
			
			m_nCurrentLine++;
		}
				
	}

	OutputLines();

			
	// Spit back the options
	GetXMLGen().AddChildElem( _T("Options") );
	//GetXMLGen().AddChildAttrib( _T("selectId"), sFileName.c_str() );
	GetXMLGen().AddChildAttrib( _T("SevInfo"), SeverityMask[1] );
	GetXMLGen().AddChildAttrib( _T("SevWarning"), SeverityMask[2] );
	GetXMLGen().AddChildAttrib( _T("SevError"), SeverityMask[3] );
	GetXMLGen().AddChildAttrib( _T("SevCritical"), SeverityMask[4] );

	GetXMLGen().AddChildAttrib( _T("LogDate"), m_sLogDate.c_str() );
	GetXMLGen().AddChildAttrib( _T("ServerID"), m_nServerID );
	GetXMLGen().AddChildAttrib( _T("LogEntryType"), sLogEntryTypeFilter.c_str() );
	GetXMLGen().AddChildAttrib( _T("Component"), nComponentFilter );
	GetXMLGen().AddChildAttrib( _T("Time"), m_sTime.c_str() );
	GetXMLGen().AddChildAttrib( _T("StartHr"), sStartHr.c_str() );
	GetXMLGen().AddChildAttrib( _T("StartMin"), sStartMin.c_str() );
	GetXMLGen().AddChildAttrib( _T("StartAmPm"), sStartAmPm.c_str() );
	GetXMLGen().AddChildAttrib( _T("StopHr"), sStopHr.c_str() );
	GetXMLGen().AddChildAttrib( _T("StopMin"), sStopMin.c_str() );
	GetXMLGen().AddChildAttrib( _T("StopAmPm"), sStopAmPm.c_str() );


	GetXMLGen().AddChildElem( _T("Page") );
	GetXMLGen().AddChildAttrib( _T("LineCount"), m_nLineCount );
	GetXMLGen().AddChildAttrib( _T("Current"), m_nPage );
	GetXMLGen().AddChildAttrib( _T("Count"), (m_nLineCount == 0) ? 1 : (m_nLineCount + nMaxRecs-1) / nMaxRecs );

	ListLogSeverities();
	ListLogEntryTypes();	

	GetXMLGen().AddChildElem( _T("Components") );
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("Component") );
	GetXMLGen().AddChildAttrib( _T("ID"), 0 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("All") );
	GetXMLGen().AddChildElem( _T("Component") );
	GetXMLGen().AddChildAttrib( _T("ID"), 1 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Routing Engine") );
	GetXMLGen().AddChildElem( _T("Component") );
	GetXMLGen().AddChildAttrib( _T("ID"), 2 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Messaging Components") );
	GetXMLGen().AddChildElem( _T("Component") );
	GetXMLGen().AddChildAttrib( _T("ID"), 3 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("ISAPI Extension") );
	GetXMLGen().OutOfElem();


	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete DB Logs for a particular server and date
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::DeleteDBLogs()
{
	CEMSString sLogDateStart;
	CEMSString sLogDateStop;
	sLogDateStart.Format(_T("%s 00:00:00 AM"), m_sLogDate.c_str());
	sLogDateStop.Format(_T("%s 11:59:59 PM"), m_sLogDate.c_str());
	
	GetQuery().Initialize();
	BINDPARAM_TCHAR_STRING(GetQuery(), sLogDateStart );
	BINDPARAM_TCHAR_STRING(GetQuery(), sLogDateStop );
	BINDPARAM_LONG(GetQuery(), m_nServerID );
	GetQuery().Execute( _T("DELETE FROM Log WHERE LogTime BETWEEN ? AND ? AND ServerID=?") );
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// ProcessBuffer
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::ProcessBuffer( TCHAR* szBuffer, DWORD dwBytes )
{
	TCHAR *p;

	p = szBuffer;

	while( m_bContinue && (p < szBuffer + dwBytes) )
	{
		if( bInField )
		{
			if( *p == _T('"') || f - szFieldBuffer == 511 )
			{
				*f = 0;	// Null terminate

				switch( FieldNdx )
				{
				case 0:
					sDate = szFieldBuffer;
					break;

				case 1:
					nErrorCode = _ttoi(szFieldBuffer);
					break;

				case 2:
					sSeverity = szFieldBuffer;
					break;

				case 3:
					sLogEntryType = szFieldBuffer;
					break;

				case 4:
					if(  SeverityMask[ (nErrorCode & 0xf0000000) >> 28]					
						&& (sLogEntryTypeFilter.size() == 0 || (sLogEntryTypeFilter.compare(sLogEntryType) == 0))
						&& (nComponentFilter == 0 || nComponentFilter == ((nErrorCode & 0x0f000000)>>24) ) )
					{
						if( m_bCountLines == FALSE )
						{
							if ( m_nLinesOut >= nMaxRecs )
							{
								m_bContinue = false;
							}
							else if( m_nCurrentLine >= m_nStartLine )
							{
								// Add to output 
								m_pLines[m_nLinesOut].sDate = sDate;
								m_pLines[m_nLinesOut].nErrorCode = nErrorCode;
								m_pLines[m_nLinesOut].sLogEntryType = sLogEntryType;
								m_pLines[m_nLinesOut].sSeverity = sSeverity;
								m_pLines[m_nLinesOut].sDesc = szFieldBuffer;
								m_nLinesOut++;
							}
						}
						else if( StartTimePassed() )
						{
							m_bContinue = false;
						}
						
						m_nCurrentLine++;
					}
					break;
				}

				f = szFieldBuffer;
				FieldNdx = (FieldNdx + 1) % 5;
				bInField = FALSE;
			}
			else
			{
				*f++ = *p;
			}
		}
		else
		{
			if( *p == _T('"') )
			{
				bInField = TRUE;
			}
		}

		p++;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetStartTime
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::GetStartTime(void)
{
	GetISAPIData().GetURLString( _T("Time"), m_sTime, true );

	// Trim off any leading spaces
	while( m_sTime.size() > 0 && !isdigit( m_sTime[0] ) )
	{
		m_sTime.erase(0,1);
	}

	// Add a leading 0 if necessary
	if(m_sTime.size() > 0)
	{
		if( m_sTime[1] == _T(':') )
		{
			m_sTime.insert( 0, _T("0") );
		}
	}

	// Shorten if ncessary
	if( m_sTime.size() > 5 )
	{
		m_sTime.resize(5);
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetStartTime
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::GetDBStartTime(void)
{
	GetISAPIData().GetURLString( _T("StartHr"), sStartHr, true );
	if ( sStartHr != "" )
	{
		if (sStartHr.ToInt() < 0 || sStartHr.ToInt() > 12 )
		{
			THROW_VALIDATION_EXCEPTION( _T("Start Hour"), _T("The start hour must be an integer between 0 and 12") );
		}
		else
		{
			if(sStartHr.length() == 1)
			{
				sStartHr.insert( 0, _T("0") );
			}
		}
	}
	else
	{
		sStartHr = "00";
	}
	
	GetISAPIData().GetURLString( _T("StartMin"), sStartMin, true );
	if ( sStartMin != "" )
	{
		if (sStartMin.ToInt() < 0 || sStartMin.ToInt() > 59)
		{
			THROW_VALIDATION_EXCEPTION( _T("Start Minute"), _T("The start minute must be an integer between 0 and 59") );
		}
		else
		{
			if(sStartMin.length() == 1)
			{
				sStartMin.insert( 0, _T("0") );
			}			
		}
	}
	else
	{
		sStartMin = "00";
	}

	GetISAPIData().GetURLString( _T("STARTAMPM"), sStartAmPm, true );

	if ( sStartAmPm == "" )
	{
		sStartAmPm = "AM";
	}

	GetISAPIData().GetURLString( _T("StopHr"), sStopHr, true );
	if ( sStopHr != "" )
	{
		if (sStopHr.ToInt() < 0 || sStopHr.ToInt() > 12 )
		{
			THROW_VALIDATION_EXCEPTION( _T("Stop Hour"), _T("The stop hour must be an integer between 0 and 12") );					
		}
		else
		{
			if(sStopHr.length() == 1)
			{
				sStopHr.insert( 0, _T("0") );
			}
		}
	}
	else
	{
		sStopHr = "11";
	}
	
	GetISAPIData().GetURLString( _T("StopMin"), sStopMin, true );
	if ( sStopMin != "" )
	{
		if (sStopMin.ToInt() < 0 || sStopMin.ToInt() > 59)
		{
			THROW_VALIDATION_EXCEPTION( _T("Stop Minute"), _T("The stop minute must be an integer between 0 and 59") );
		}
		else
		{
			if(sStopMin.length() == 1)
			{
				sStopMin.insert( 0, _T("0") );
			}			
		}
	}
	else
	{
		sStopMin = "59";
	}

	
	GetISAPIData().GetURLString( _T("STOPAMPM"), sStopAmPm, true );

	if ( sStopAmPm == "" )
	{
		sStopAmPm = "PM";
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// StartTimePassed
// 
////////////////////////////////////////////////////////////////////////////////
bool CLogConfig::StartTimePassed(void)
{
	if( m_sTime.size() > 0 )
	{
		if( _tcsicmp( sDate.c_str(), m_sTime.c_str() ) > 0 )
		{
			return true;
		}
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////
// 
// ComputeStartLine
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::ComputeStartLine(void)
{
	// We have the total number of lines in m_nLineCount
	// And the desired page number in m_nPage
	// And the lines per page in nMaxRecs

	m_nStartLine = (m_nLineCount - m_nPage * nMaxRecs);

	if( m_nStartLine < 0 )
	{
		m_nStartLine = 0;
		m_nPage = (m_nLineCount == 0) ? 1 : (m_nLineCount + nMaxRecs-1) / nMaxRecs ;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// OutputLines
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::OutputLines(void)
{
	int i;
	
	if( m_nLinesOut == 0 )
	{
		GetXMLGen().AddChildElem( _T("None") );
		GetXMLGen().AddChildAttrib( _T("Description"), _T("No log entries match criteria") );
	}
	else
	{
		for( i = m_nLinesOut - 1; i >= 0; i-- )
		{
			if( !m_pLines[i].sDate.empty() )
			{
				// This code automatically sets the current time to the first one in the list
				//if( i == m_nLinesOut - 1 && m_sTime.size() == 0 )
				//{
				//	m_sTime = m_pLines[i].sDate.substr(0,5);
				//}
				
				int nComponent = (m_pLines[i].nErrorCode & 0x0f000000) >> 24;
				GetXMLGen().AddChildElem(_T("Log"));
				GetXMLGen().AddChildAttrib( _T("Date"), m_pLines[i].sDate.c_str() );
				GetXMLGen().AddChildAttrib( _T("ErrorCode"), m_pLines[i].nErrorCode );
				GetXMLGen().AddChildAttrib( _T("SeverityDesc"), m_pLines[i].sSeverity.c_str() );
				GetXMLGen().AddChildAttrib( _T("Severity"), (( m_pLines[i].nErrorCode & 0xf0000000)>>28) );
				GetXMLGen().AddChildAttrib( _T("Component"), nComponent );

				switch( nComponent )
				{
				case 1:
					GetXMLGen().AddChildAttrib( _T("ComponentDesc"), _T("Routing Engine") );
					break;

				case 2:
					GetXMLGen().AddChildAttrib( _T("ComponentDesc"), _T("Messaging Components") );
					break;

				case 3:
					GetXMLGen().AddChildAttrib( _T("ComponentDesc"), _T("ISAPI Extension") );
					break;
				}

				GetXMLGen().AddChildAttrib( _T("LogEntryType"), m_pLines[i].sLogEntryType.c_str() );
				
				tstring sTemp = m_pLines[i].sDesc;								
				if ( sTemp.length() > INBOUNDMESSAGES_VIRUSNAME_LENGTH-1)
				{	
					sTemp = sTemp.substr(0,INBOUNDMESSAGES_VIRUSNAME_LENGTH-4);
					sTemp += _T("...");					
				}
				GetXMLGen().AddChildAttrib( _T("ShortDescription"), sTemp.c_str() );
				GetXMLGen().SetChildData( m_pLines[i].sDesc.c_str(), 1 );
			}
		}	
	}
}



////////////////////////////////////////////////////////////////////////////////
// 
// VerifyDirectoryExists
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::VerifyDirectoryExists( tstring& sFolder )
{
	CEMSString sFilename;
	SYSTEMTIME now;
	int n = 99;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	if( sFolder.size() < 1 )
	{
		CEMSString sError;
		sError.Format( _T("Please provide a valid directory."), sFolder.c_str() );
		THROW_EMS_EXCEPTION( E_SystemError, sError );
	}

	GetLocalTime( &now );

	sFilename.Format( _T("%s\\VMF-%d-%d-%d-%d.TST"), sFolder.c_str(),
			  now.wYear, now.wMonth, now.wDay, n );


	hFile = CreateFile( sFilename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, 
			            FILE_ATTRIBUTE_NORMAL, NULL );
	
	if( hFile == INVALID_HANDLE_VALUE )
	{
		DWORD dwErr = GetLastError();

		if( dwErr != ERROR_ALREADY_EXISTS && dwErr != ERROR_FILE_EXISTS )
		{
			CEMSString sError;
			sError.Format( _T("%s is not a valid directory or the permissions are incorrect. Error: %d"), 
				sFolder.c_str(), dwErr);
			THROW_EMS_EXCEPTION( E_SystemError, sError );
		}		
	}
	else
	{
		CloseHandle( hFile );
		DeleteFile( sFilename.c_str() );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create Log File
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::CreateLogFile()
{
	tstring tstrDir;
	tstring sYr;
	tstring sMo;
	tstring sDa;
	CEMSString sFilename;	
	HANDLE hFile = INVALID_HANDLE_VALUE;

	GetLogFilePath( tstrDir );

	if( tstrDir.size() < 1 )
	{
		CEMSString sError;
		sError.Format( _T("Please provide a valid directory."), tstrDir.c_str() );
		THROW_EMS_EXCEPTION( E_SystemError, sError );
	}

	// Get the Server Description
	GetQuery().Initialize();
	BINDCOL_TCHAR(GetQuery(), m_Description);
	BINDPARAM_LONG(GetQuery(), m_nServerID );
	GetQuery().Execute( _T("SELECT Description FROM Servers WHERE ServerID=?") );
	GetQuery().Fetch();
	
	sYr = m_sLogDate.substr(6,4);
	sMo = m_sLogDate.substr(0,2);
	sDa = m_sLogDate.substr(3,2);

	sFilename.Format( _T("%s\\LOG%s%s%s-%s.txt"), tstrDir.c_str(), sYr.c_str(), sMo.c_str(), sDa.c_str(), m_Description);

	hFile = CreateFile( sFilename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
			            FILE_ATTRIBUTE_NORMAL, NULL );
	
	if( hFile == INVALID_HANDLE_VALUE )
	{
		DWORD dwErr = GetLastError();

		if( dwErr != ERROR_ALREADY_EXISTS && dwErr != ERROR_FILE_EXISTS )
		{
			CEMSString sError;
			sError.Format( _T("Cold not create log file. Error: %d"), 
				dwErr);
			THROW_EMS_EXCEPTION( E_SystemError, sError );
		}		
	}
	else
	{
		CloseHandle( hFile );		
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// OpenLogFile
// 
////////////////////////////////////////////////////////////////////////////////
void CLogConfig::OpenLogFile(void)
{
	tstring tstrDir;
	tstring sYr;
	tstring sMo;
	tstring sDa;
	CEMSString sFilename;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	GetLogFilePath( tstrDir );

	if( tstrDir.size() < 1 )
	{
		CEMSString sError;
		sError.Format( _T("Please provide a valid directory."), tstrDir.c_str() );
		THROW_EMS_EXCEPTION( E_SystemError, sError );
	}

	// Get the Server Description
	GetQuery().Initialize();
	BINDCOL_TCHAR(GetQuery(), m_Description);
	BINDPARAM_LONG(GetQuery(), m_nServerID );
	GetQuery().Execute( _T("SELECT Description FROM Servers WHERE ServerID=?") );
	GetQuery().Fetch();
	
	sYr = m_sLogDate.substr(6,4);
	sMo = m_sLogDate.substr(0,2);
	sDa = m_sLogDate.substr(3,2);

	sFilename.Format( _T("%s\\LOG%s%s%s-%s.txt"), tstrDir.c_str(), sYr.c_str(), sMo.c_str(), sDa.c_str(), m_Description);

	// create file if it doesn't already exist
	m_FileHandle = CreateFile( sFilename.c_str(), GENERIC_WRITE, 
		                       FILE_SHARE_READ, NULL, 
		                       OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	if( m_FileHandle == INVALID_HANDLE_VALUE )
	{
		DWORD dwErr = GetLastError();
		
		CEMSString sError;
			sError.Format( _T("Cold not open log file. Error: %d"), 
				dwErr);
			THROW_EMS_EXCEPTION( E_SystemError, sError );		
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
void CLogConfig::CloseLogFile(void)
{
	// Close log file

	if( m_FileHandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_FileHandle );
		m_FileHandle = INVALID_HANDLE_VALUE;
	}
}