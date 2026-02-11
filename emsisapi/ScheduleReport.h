/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ScheduleReport.h,v 1.1 2005/04/18 18:48:45 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Report Scheduler
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CScheduleReport : public CXMLDataClass, public TScheduleReport
{
public:
	CScheduleReport( CISAPIData& ISAPIData );
	virtual ~CScheduleReport();

	virtual int Run( CURLAction& action );

protected:

	int ListAll(void);
	int ListOwnerAll(void);
	int Query(void);
	int Update();
	int Delete();
	int New();
	
	void DecodeForm(void);
	void GenerateXML(void);

	void GetWhenSelect( CEMSString& sDatePart, TCHAR* szColumn, bool bBind = true );
	void GetDateEquality( CEMSString& sDateEquality, TCHAR* szColumnA, TCHAR* szColumnB );
	void GetWeekDayName( tstring& sName, int nDay );
	void GetWhen( CEMSString& sDatePart );
	void OpenCDF( CURLAction& action );
	void OutputCDF( tstring& sLine );
	void CloseCDF(void);
	void CheckForDuplicity( void );
	
	void AddTicketStates( void );
	void AddTicketBoxes( void );
	void AddAgents( void );
	void AddRoutingRules( void );
	void AddMsgSources( void );
	void AddTicketCategories( void );
	void AddStdCategories( void );

	int  GetLastDayOfMonth( int month );
	int  GetDayOfWeek( int month, int day, int year );
	void SetNextRunTime( void );
	
	TIMESTAMP_STRUCT	m_Start;
	TIMESTAMP_STRUCT	m_Stop;
	int					m_ReportInterval;
	int					m_MaxRows;
	int					m_nYear;
	int					m_nMonth;
	int					m_nWeek;
	int					m_nDay;
	int					m_nHour;
	int					m_nDayofWeek;
	bool				m_isAdminView;
	bool				m_OutputCDF;
	TCHAR				m_szTempFile[MAX_PATH];
	HANDLE				m_hCDFFile;
};

// constants
#define EMS_REPORT_ALL  -1
#define EMS_REPORT_FULL_REPORT -2