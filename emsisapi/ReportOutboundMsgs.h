/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportOutboundMsgs.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Outbound Message Distribution Report   
||              
\\*************************************************************************/
#pragma once

#include "Reports.h"

class CReportOutboundMsgs : public CReports  
{
public:
	CReportOutboundMsgs( CISAPIData& ISAPIData );
	virtual ~CReportOutboundMsgs() {};

	virtual int Run( CURLAction& action );

protected:
	void DecodeForm(void);
	void AddReportHeader(void);
	void AddReportOptions(void);
	void Query(void);

	bool m_bPost;
	bool m_bTicketBox;
	bool m_bTicketCategory;
	bool m_bGroup;
	unsigned char m_include_auto;
	unsigned char m_groupby_ticketbox;
	int  m_nFilter;
	int  m_nDateRange;
	int  m_nRows;
	
	CEMSString idCollection;
	list<int> m_IDs;
	bool m_bCustom;

	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;

	CEMSString m_sTitle;
};
