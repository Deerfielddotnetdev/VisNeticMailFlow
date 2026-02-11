/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportTicketDist.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "Reports.h"

class CReportTicketDist : public CReports  
{
public:
	CReportTicketDist(CISAPIData& ISAPIData);
	virtual ~CReportTicketDist() {};

	virtual int Run(CURLAction& action);

protected:
	void DecodeForm();
	void Query();
	void QueryTicketBox();
	void QueryTicketCategory();
	void QueryOwner();
	void AddReportOptions();
	void GenerateXML();

private:
	CEMSString m_sTitle;
	list<CIDCount> m_list;
	bool m_bPost;
	bool m_bTicketBox;
	bool m_bTicketCategory;
	int m_nTicketState;
	int m_nTotal;
	unsigned char m_include_zero;
	unsigned char m_include_unowned;
	unsigned char m_include_notassigned;
};
