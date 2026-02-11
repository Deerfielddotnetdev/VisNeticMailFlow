/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportBusy.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "reports.h"

class CReportBusy : public CReports  
{
public:
	CReportBusy( CISAPIData& ISAPIData );
	virtual ~CReportBusy() {};

	int Run(CURLAction& action);
	
protected:
	void DecodeForm(void);
	void Query(void);
	void GenerateXML(void);
	void AddReportHeader(void);
	void AddReportOptions(void);
	
private:
	list<CIDCount> m_list;

	bool m_bPost;
	bool m_bHour;
	int  m_nTotal;
	int  m_nFilter;
	int  m_nDateRange;
	CEMSString m_sTitle;

	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;
};
