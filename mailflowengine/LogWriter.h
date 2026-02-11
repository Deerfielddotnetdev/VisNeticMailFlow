// LogWriter.h: interface for the CLogWriter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGWRITER_H__4AA3E444_93D0_44DD_9B29_826DB6AB8513__INCLUDED_)
#define AFX_LOGWRITER_H__4AA3E444_93D0_44DD_9B29_826DB6AB8513__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// CLogEntry is a data class used to store log requests in a deque
class CLogEntry
{
public:
	CLogEntry( long m_ErrorCode, wchar_t* wcsText ) // constructor
		: m_ErrorCode( m_ErrorCode ), m_Text( wcsText ) 
	{
		GetTimeStamp( m_TimeStamp );
	}

	CLogEntry( const CLogEntry& i ) // copy constructor
	{
		m_ErrorCode = i.m_ErrorCode;
		m_TimeStamp = i.m_TimeStamp;
		m_Text = i.m_Text;
	}
	CLogEntry& operator=(const CLogEntry& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			m_ErrorCode = i.m_ErrorCode;
			m_TimeStamp = i.m_TimeStamp;
			m_Text = i.m_Text;
		}
		return *this; // return reference to self
	}

	long m_ErrorCode;
	wstring m_Text;
	TIMESTAMP_STRUCT m_TimeStamp;
};

class CLogFilesInfo : public WIN32_FIND_DATA
{
public:
};

inline bool operator<(const CLogFilesInfo& A,const CLogFilesInfo& B)
{ return (CompareFileTime( &B.ftLastWriteTime, &A.ftLastWriteTime ) == -1); }


class CLogWriter  
{
public:
	CLogWriter( CODBCQuery& query );
	virtual ~CLogWriter();

	void Log( long ErrorCode, wchar_t* wcsText );
	
	void Log( long ErrorCode, unsigned int StringID );


	HRESULT Run(void);
	void UnInitialize(void);
	void OpenLogFile(void);
	void CloseLogFile(void);
	void DeleteOldLogs(void);
	void LoadMap(void);
	void WriteLogEntry( CLogEntry& entry );
	void DeleteLogFiles(void);
	void GetLogFilePath( tstring& tstrPath );
	void DeleteLogFile( tstring& sFileName );

	BOOL					m_bMapLoaded;

protected:
	CODBCQuery&				m_query;
	deque<CLogEntry>		m_Queue;
	vector<LogEntryTypes_t>	m_Map;
	vector<LogSeverity_t>	m_SevMap;
	TIMESTAMP_STRUCT		m_LogFileTime;
	TIMESTAMP_STRUCT		m_Time;
	TIMESTAMP_STRUCT		m_MapTime;	
	HANDLE					m_FileHandle;
	BOOL					m_bDirty;
	BOOL					m_bLogToDb;
	UINT					m_nServerID;
	UINT					m_nDeleteLogs;

	void GetServerParameter( int ID, CEMSString& sValue );
};

#endif // !defined(AFX_LOGWRITER_H__4AA3E444_93D0_44DD_9B29_826DB6AB8513__INCLUDED_)
