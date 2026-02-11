
#include "Reports.h"

class CTicketBoxDetails : public CReports  
{
public:
	CTicketBoxDetails( CISAPIData& ISAPIData );
	virtual ~CTicketBoxDetails() {};

	virtual int Run( CURLAction& action );

protected:
	void DecodeForm(void);

	void DoQuery(void);
	void FormatTbQuery( CEMSString& sSQL );
	void FormatTbTotalsQuery( CEMSString& sSQL );
	void FormatTotalsQuery( CEMSString& sSQL );
		
	void AddReportHeader( void );
	void AddReportOptions( void );

	bool m_bPost;
	int m_nType;
	int m_nFilter;
	int m_nActionFilter;
	int m_nSortBy;
	int m_nSortDirection;
	int m_nDateRange;

	CEMSString tbCollection;
	
	CEMSString m_sTitle;
	CEMSString m_sContact;

	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;

	list<int> m_tbIDs;
	
};
