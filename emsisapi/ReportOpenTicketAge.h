/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportOpenTicketAge.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/
#pragma once

#include "Reports.h"

class CReportOpenTicketAge : public CReports  
{
public:
	CReportOpenTicketAge( CISAPIData& ISAPIData );
	virtual ~CReportOpenTicketAge() {};

	virtual int Run( CURLAction& action );

protected:
	void DecodeForm(void);
	void Query(void);
	void AddReportHeader(void);
	void AddReportOptions(void);
	void FormatTimeString( int nWhen, CEMSString& sWhen );
	void GetWhen( CEMSString& sWhen );

private:
	bool m_bPost;
	bool m_bTicketBox;
	bool m_bTicketCategory;
	bool m_bGroup;
	int m_nFilter;
	CEMSString m_sTitle;
};
