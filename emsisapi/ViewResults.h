/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ViewResults.h,v 1.1 2007/09/22 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Report Results
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CViewResults : public CXMLDataClass  
{
public:
	CViewResults( CISAPIData& ISAPIData );
	virtual ~CViewResults() {};

	virtual int Run( CURLAction& action );

protected:
	void Query(void);
	void Delete(void);
	void ShowResults( CURLAction& action );
	void OutputResults( void );
	void AddReportHeader(void);

	void OpenCDF( CURLAction& action );
	void OutputCDF( tstring& sLine );
	void CloseCDF(void);


	TReportResults			trr;
	TScheduleReport			tsr;
	TStandardReport			tstdr;
	
	TReportResultRows rrr;
	vector<TReportResultRows> m_rrr;
	vector<TReportResultRows>::iterator rrrIter;

	bool				m_OutputCDF;
	TCHAR				m_szTempFile[MAX_PATH];
	HANDLE				m_hCDFFile;

	int					m_MaxRows;

	LPCTSTR GetGraphColor( int nRowIndex );
		

private:
	bool				m_bPost;
	int					m_nType;
	int					m_nFilter;
	int					m_nDateRange;

	TIMESTAMP_STRUCT	m_StartDate;
	TIMESTAMP_STRUCT	m_EndDate;

	CEMSString			m_sTitle;
	CEMSString			m_sSumPeriod;
	
};

class CPrintReportV : public CXMLDataClass  
{
public:
	CPrintReportV( CISAPIData& ISAPIData );
	virtual ~CPrintReportV() {};

	virtual int Run( CURLAction& action );

protected:
};

// constants
#define EMS_REPORT_ALL  -1
#define EMS_REPORT_FULL_REPORT -2