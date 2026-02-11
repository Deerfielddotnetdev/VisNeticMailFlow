/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportInboundMsgs.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Inbound Message Distribution Report   
||              
\\*************************************************************************/
#pragma once

#include "Reports.h"

class CReportInboundMsgs : public CReports  
{
public:
	CReportInboundMsgs( CISAPIData& ISAPIData );
	virtual ~CReportInboundMsgs() {};

	virtual int Run( CURLAction& action );

protected:
	void DecodeForm(void);
	void AddReportHeader(void);
	void AddReportOptions(void);
	void Query(void);

private:
	bool m_bPost;
	int	 m_nType;
	int  m_nFilter;
	int  m_nDateRange;
	unsigned char m_groupby_ticketbox;
		
	CEMSString idCollection;
	list<int> m_IDs;
	bool m_bCustom;

	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;

	CEMSString m_sTitle;
};
