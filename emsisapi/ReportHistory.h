/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportHistory.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "Reports.h"

class CReportHistory : public CReports  
{
public:
	CReportHistory( CISAPIData& ISAPIData );
	virtual ~CReportHistory() {};

	virtual int Run( CURLAction& action );

protected:
	void DecodeForm(void);

	void Query(void);
	void FormatAgentQuery( CEMSString& sSQL );
	void FormatTicketBoxQuery( CEMSString& sSQL );
	void FormatTicketCategoryQuery( CEMSString& sSQL );
	void FormatContactQuery( CEMSString& sSQL );
	
	void AddReportHeader( void );
	void AddReportOptions( void );

	void GetSecurityQueryStrings( CEMSString& sJoin, CEMSString& sWhere );

	bool m_bPost;
	int m_nType;
	int m_nFilter;
	int m_nMsgFilter;
	int m_nDateRange;

	CEMSString m_sTitle;
	CEMSString m_sContact;

	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;
};
