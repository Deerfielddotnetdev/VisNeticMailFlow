// Reporter.h: interface for the CReporter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REPORTER_H__5111BC42_B12C_42AF_A0FB_9EBE2D245A85__INCLUDED_)
#define AFX_REPORTER_H__5111BC42_B12C_42AF_A0FB_9EBE2D245A85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CReporter  
{
public:

	CReporter( CODBCQuery& query );
	virtual ~CReporter();

	HRESULT Run(void);
	void UnInitialize(void);

protected:
	
	void CheckEnabled( void );
	bool GetParameters( void );
	bool GetReportToRun( void );
	bool ProcessReport( void );
	void GetWhen( CEMSString& sWhen );
	void GetWhen2( CEMSString& sWhen );
	void GetWhenSelect( CEMSString& sDatePart, TCHAR* szColumn, bool bBind );
	void SetStartEndDate( void );
	int GetDayOfWeek( int month, int day, int year );
	int GetLastDayOfMonth( int month );
	void GetWeekDayName( tstring& sName, int nDay );
	void GetStdCategoryName(  int nID, CEMSString& sName );
	void GetAgentName(  int nID, CEMSString& sName );
	void GetTicketBoxName(  int nID, CEMSString& sName );
	void FormatTimeString( int nWhen, CEMSString& sWhen );
	bool VerifyDirectory( dca::String& sPath );
	void AddResultRows( void );


	void InboundMessageReport( void );
	void OutboundMessageReport( void );
	void OpenTicketAgeReport( void );
	void TicketDistributionReport( void );
	void StandardResponsesReport( void );
	void AverageResponseTimeReport( void );
	void AverageTimeToResolveReport( void );
	void BusiestTimeReport( void );
	void TicketDispositionReport( void );
	
	
	void OpenCDF( void );
	void OutputCDF( tstring& sLine );
	void CloseCDF( void );
	
	void AlertOwner( void );
	void EmailReport( void );
	void SetNextRunTime( void );
	void UpdateReport( void );

	enum { ScheduledReportCheckIntervalMs = 60000 };

	long			m_LastScheduledReportCheck;	
	
	CODBCQuery&			m_query;

	TScheduleReport sr;

	int		m_nYear;
	int		m_nMonth;
	int		m_nWeek;
	int		m_nDay;
	int		m_nHour;
	bool	m_bHour;
	int		m_ReportTypeID;
	long	m_ReportTypeIDLen;
	int		m_ReportObjectID;
	long	m_ReportObjectIDLen;
	int		m_ThisResult;
	long	m_ThisResultLen;
	TCHAR	m_nName[TICKETCATEGORIES_DESCRIPTION_LENGTH];
	long	m_nNameLen;
	TCHAR	m_FromEmailAddress[ALERTS_FROMEMAILADDRESS_LENGTH];
	long	m_FromEmailAddressLen;
	int		m_ReportResultID;
	long	m_ReportResultIDLen;
	long	nPercentage;
	bool	m_reportingEnabled;

	CEMSString	Col1;
	CEMSString	Col2;
	int			Col3;
	long		Col4;
	int			Col5;
	
    
	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;

	CEMSString			m_sReportFolder;
	CEMSString			m_sReportPath;
	CEMSString			m_sReportFile;
	CEMSString			m_sFullReportFile;
	CEMSString			m_sOutboundAttachFolder;
	TCHAR				m_szReportFile[MAX_PATH];
	HANDLE				m_hCDFFile;
	HANDLE				m_hATTACHFile;

	TReportResultRows rrr;
	vector<TReportResultRows> m_rrr;
	vector<TReportResultRows>::iterator rrrIter;
	
};

class CIDCounts
{
public:
	CIDCounts() : m_nID(0), m_nCount(0) {};
	
	int m_nID;
	int m_nCount;
};


#endif