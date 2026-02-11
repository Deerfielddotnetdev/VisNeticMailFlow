/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportStdResp.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "Reports.h"

class CReportStdResp : public CReports  
{
public:
	CReportStdResp(CISAPIData& ISAPIData);
	virtual ~CReportStdResp() {};
	
private:
	int  Run(CURLAction& action);
	void DecodeForm(void);
	void Query(void);
	void QueryCategory(void);
	void QueryAuthor(void);
	void AddReportOptions(void);
	void AddReportHeader(void);

	bool m_bPost;
	bool m_bCategory;
	list<CIDCount> m_list;
	int m_nTotal;
	unsigned char m_include_zero;
	unsigned char m_uncategorized;
	CEMSString m_sTitle;
};
