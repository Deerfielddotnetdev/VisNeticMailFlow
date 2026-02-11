/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportResolveTime.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "reports.h"

class CReportResolveTime : public CReports  
{
public:
	CReportResolveTime(CISAPIData& ISAPIData );
	virtual ~CReportResolveTime() {};

	virtual int Run( CURLAction& action );

protected:
	void DecodeForm(void);
	void Query();
	void AddReportHeader(void);
	void AddReportOptions(void);

private:
	bool m_bPost;
	bool m_bTicketBox;
	bool m_bTicketCategory;
	unsigned char m_include_auto;
	int m_nFilter;
	int m_nDateRange;
	CEMSString m_sTitle;
	
	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;
};

