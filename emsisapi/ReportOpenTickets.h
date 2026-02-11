
#include "Reports.h"

class COpenTickets : public CReports  
{
public:
	COpenTickets( CISAPIData& ISAPIData );
	virtual ~COpenTickets() {};

	virtual int Run( CURLAction& action );

protected:
	void DecodeForm(void);

	void DoQuery(void);
	void FormatQuery( CEMSString& sSQL );
	void FormatGroupTotalQuery( CEMSString& sSQL );
	void FormatTotalQuery( CEMSString& sSQL );
	void AddReportHeader( void );
	void AddReportOptions( void );
	void GetSecurityQueryStrings( bool bAll, CEMSString& sJoin, CEMSString& sWhere );

	bool m_bPost;
	bool m_bTicketBox;
	bool m_bTicketCategory;
	bool m_bGroup;
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
