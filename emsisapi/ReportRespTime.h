/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportRespTime.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "reports.h"

class CReportRespTime : public CReports  
{
public:
	CReportRespTime(CISAPIData& ISAPIData);
	virtual ~CReportRespTime() {};

	int Run(CURLAction& action);

private:
	void DecodeForm();
	void Query();
	void AddReportHeader();
	void AddReportOptions();
	
private:
	bool m_bGenerateReport;
	bool m_bTicketBox;
	bool m_bTicketCategory;
	unsigned char m_include_auto;
	int m_nFilter;
	int m_nDateRange;
	CEMSString m_sTitle;

	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;
};	