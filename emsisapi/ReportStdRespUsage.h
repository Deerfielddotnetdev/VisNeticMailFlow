/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportStdRespUsage.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "Reports.h"

class CReportStdRespUsage : public CReports  
{
public:
	CReportStdRespUsage(CISAPIData& ISAPIData);
	virtual ~CReportStdRespUsage() {};

	int Run( CURLAction& action );

private:
	void DecodeForm(void);
	void Query(void);
	void AddReportOptions(void);
	void AddReportHeader(void);

private:
	bool m_bPost;
	CEMSString m_sTitle;
	int m_nFilter;
	int m_nTotal;
	int m_nDateRange;
	unsigned char m_include_zero;
	unsigned char m_include_auto;
	
	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;
};
