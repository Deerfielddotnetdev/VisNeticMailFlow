/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportCompareInbound.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "Reports.h"

class CReportCompare : public CReports  
{
public:
	CReportCompare(CISAPIData& ISAPIData );
	virtual ~CReportCompare();

	virtual int Run( CURLAction& action );
private:
	void DecodeForm(void);
	void Query(void);
	void QueryTicketBox(void);
	void QueryAgent(void);
	void GenerateXML(void);
	void AddReportHeader( void );
	void AddReportOptions( void );

private:
	CEMSString m_sTitle;
	list<CIDCount> m_list;
	bool m_bPost;
	bool m_bTicketBox;
	int m_nTotal;
	int m_nDateRange;
	unsigned char m_include_zero;

	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;
};
