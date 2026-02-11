/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Reports.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Reporting Base Class
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CReports : public CXMLDataClass
{
public:
	CReports( CISAPIData& ISAPIData );
	virtual ~CReports();

protected:

	void GetWhenSelect( CEMSString& sDatePart, TCHAR* szColumn, bool bBind = true );
	void GetDateEquality( CEMSString& sDateEquality, TCHAR* szColumnA, TCHAR* szColumnB );
	void GetWeekDayName( tstring& sName, int nDay );
	void GetWhen( CEMSString& sDatePart );
	void OpenCDF( CURLAction& action );
	void OutputCDF( tstring& sLine );
	void CloseCDF(void);
	
	LPCTSTR GetGraphColor( int nRowIndex );
	void AddTicketStates( int nSelected = 0, bool bSubmitOnChange = false );
	void AddTicketBoxes( int nSelected = 0, bool bAllowMulti = false );
	void AddAgents( int nSelected = 0 );
	void AddRoutingRules( int nSelected = 0 );
	void AddMsgSources( int nSelected = 0 );
	void AddTicketCategories( int nSelected = 0 );
	void AddStdCategories( int nSelected );
	void AddDateIntervals( LPCTSTR szLable, int nSelected = 0 );
	void AddDateRange( int nSelected = 0 );
	void AddMsgTypes( int nSelected = 0 );
	void AddMaxLines( int nMaxLines = 0 );
	void AddActionTypes( int nSelected = 0 );
	void AddSortBy( int nSelected = 0 );
	void AddSortDirection( int nSelected = 0 );
	void AddGroups( int nSelected = 0, bool bAddAll = true );

	TIMESTAMP_STRUCT	m_Start;
	TIMESTAMP_STRUCT	m_Stop;
	int					m_ReportInterval;
	int					m_MaxRows;
	long				m_nYear;
	long				m_nMonth;
	long				m_nWeek;
	long				m_nDay;
	long				m_nHour;
	long				m_nDayofWeek;

	bool				m_OutputCDF;
	TCHAR				m_szTempFile[MAX_PATH];
	HANDLE				m_hCDFFile;
};

class CIDCount
{
public:
	CIDCount() : m_nID(0), m_nCount(0) {};
	
	int m_nID;
	int m_nCount;
};

class CPrintReport : public CXMLDataClass  
{
public:
	CPrintReport( CISAPIData& ISAPIData );
	virtual ~CPrintReport() {};

	virtual int Run( CURLAction& action );

protected:
};

// constants
#define EMS_REPORT_ALL  -1
#define EMS_REPORT_FULL_REPORT -2