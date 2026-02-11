/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/AgentAudit.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "Reports.h"

class CAgentAudit : public CReports  
{
public:
	CAgentAudit( CISAPIData& ISAPIData );
	virtual ~CAgentAudit() {};

	virtual int Run( CURLAction& action );

protected:
	void DecodeForm(void);

	void DoQuery(void);
	void FormatAgentQuery( CEMSString& sSQL );
		
	void AddReportHeader( void );
	void AddReportOptions( void );

	bool m_bPost;
	int m_nType;
	int m_nFilter;
	int m_nActionFilter;
	int m_nSortBy;
	int m_nSortDirection;
	int m_nDateRange;

	CEMSString agCollection;
	CEMSString acCollection;

	CEMSString m_sTitle;
	CEMSString m_sContact;

	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;

	list<int> m_agIDs;
	list<int> m_acIDs;
};
