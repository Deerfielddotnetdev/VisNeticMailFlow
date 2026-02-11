
#include "Reports.h"

class CTicketCountSum : public CReports  
{
public:
	CTicketCountSum( CISAPIData& ISAPIData );
	virtual ~CTicketCountSum() {};

	virtual int Run( CURLAction& action );

protected:
	void DecodeForm(void);

	void DoQuery(void);
	void FormatIDTotalsQuery( CEMSString& sSQL );
	void FormatTotalsQuery( CEMSString& sSQL );
		
	void AddReportHeader( void );
	void AddReportOptions( void );

	bool m_bPost;
	bool m_bTicketBox;
	bool m_bTicketCategory;
	int m_nType;
	int m_nFilter;
	int m_nActionFilter;
	int m_nSortBy;
	int m_nSortDirection;
	int m_nDateRange;
		
	CEMSString idCollection;
	
	CEMSString m_sTitle;
	CEMSString m_sContact;

	TIMESTAMP_STRUCT m_StartDate;
	TIMESTAMP_STRUCT m_EndDate;

	list<int> m_IDs;
	
};
