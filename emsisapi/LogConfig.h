// LogConfig.h: interface for the CLogConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGCONFIG_H__A46D40E5_BC49_4FAF_A0D9_C254A7339E1D__INCLUDED_)
#define AFX_LOGCONFIG_H__A46D40E5_BC49_4FAF_A0D9_C254A7339E1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

struct CLogLine
{
	tstring sDate;
	unsigned int nErrorCode;
	tstring sSeverity;
	tstring sLogEntryType;
	tstring sDesc;
};

class CLogConfig : public CXMLDataClass  
{
public:
	CLogConfig( CISAPIData& ISAPIData );
	virtual ~CLogConfig();

	virtual int Run(CURLAction& action);

protected:
	int ListAll(void);
	int Update(CURLAction& action);
	void GetLogFilePath( tstring& tstrPath );
	int ListDates(void);
	int ListDBDates(void);
	int ListDebugLogs(void);
	void GetLogFileInfo( WIN32_FIND_DATA* pFindData );
	int ShowLogs( tstring& sFileName, CURLAction& action );
	int ShowDBLogs( CURLAction& action );
	int SaveDBLogs( CURLAction& action );
	void Delete( tstring& sFileName );
	void DeleteDBLogs();
	void VerifyDirectoryExists( tstring& sFolder );
	void CreateLogFile( void );
	void OpenLogFile( void );
	void CloseLogFile( void );
	void ProcessBuffer( TCHAR* szBuffer, DWORD dwBytes );
	void ComputeStartLine(void);
	void OutputLines(void);
	bool StartTimePassed(void);
	void GetStartTime(void);
	void GetDBStartTime(void);

	bool m_bContinue;
	TCHAR szFieldBuffer[512];
	TCHAR* f;
	BOOL bInField;
	long FieldNdx;
	tstring sDate;
	unsigned int nErrorCode;
	tstring sSeverity;
	tstring sLogEntryType;
	tstring sLogEntryTypeFilter;
	int nComponentFilter;
	int nMaxRecs;
	int m_nLineCount;
	BOOL m_bCountLines;
	int m_nPage;
	int m_nStartLine;
	int m_nLinesOut;
	int m_nCurrentLine;
	CLogLine* m_pLines;
	unsigned char SeverityMask[17];
	tstring m_sTime;
	int m_nServerID;
	TCHAR m_Description[SERVER_DESCRIPTION_LENGTH];
	long m_DescriptionLen;
	TCHAR m_Severity[55];
	long m_SeverityLen;
	TCHAR m_EntryType[125];
	long m_EntryTypeLen;
	int m_nNumRows;
	TIMESTAMP_STRUCT m_LogDate;
	long m_LogDateLen;
	int m_nServerLogID;
	tstring m_sLogDate;
	int m_nLogEntryTypeID;
	tstring sStartAmPm;
	dca::String sStartHr;
	dca::String sStartMin;
	tstring sStopAmPm;
	dca::String sStopHr;
	dca::String sStopMin;

	HANDLE	m_FileHandle;
	
};

class CLogFileInfo : public WIN32_FIND_DATA
{
public:
};

inline bool operator<(const CLogFileInfo& A,const CLogFileInfo& B)
{ return (CompareFileTime( &B.ftLastWriteTime, &A.ftLastWriteTime ) == -1); }


#endif // !defined(AFX_LOGCONFIG_H__A46D40E5_BC49_4FAF_A0D9_C254A7339E1D__INCLUDED_)
