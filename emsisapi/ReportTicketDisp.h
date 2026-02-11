/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportTicketDisp.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "reports.h"

class CReportTicketDisp : public CReports  
{
public:
	CReportTicketDisp(CISAPIData& ISAPIData);
	virtual ~CReportTicketDisp() {};

	int Run(CURLAction& action);

private:
	void DecodeForm();
	void Query();
	void AddReportHeader();
	void AddReportOptions();
	
private:
	bool m_bGenerateReport;
	bool m_bTicketBox;
	int m_nFilter;
	int m_nDateRange;
	CEMSString m_sTitle;

	int m_nCreated;
	int m_nMovedTo;
	int m_nOpened;
	int m_nRestored;
	int m_nTotalIn;

	int m_nClosed;
	int m_nDeleted;
	int m_nMovedFrom;
	int m_nTotalOut;

	int m_nTotal;

	unsigned char m_include_zero;
	
	list<int> m_tbids;
	list<int>::iterator tbIter;

	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;
};	