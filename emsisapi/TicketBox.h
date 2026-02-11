// TicketBox.h: interface for the CTicketBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TICKETBOX_H__C1E99A69_3055_4C44_AE93_7D4402430C92__INCLUDED_)
#define AFX_TICKETBOX_H__C1E99A69_3055_4C44_AE93_7D4402430C92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"
#include "Ticket.h"

class CTicketBox : public CXMLDataClass, public TTicketBoxes
{
public:
	CTicketBox(CISAPIData& ISAPIData);
	virtual ~CTicketBox();

	// methods
	virtual int Run( CURLAction& action );

protected:
	int ListAll(void);
	int Query(void);
	int Update(void);
	int Delete(void);
	int New(void);
	void DecodeForm(void);
	bool IsDuplicate(void);
	void GenerateXML(void);
	void MoveTickets(void);
	void DeleteTickets(void);
	void ModifyTickets(void);
	void AddTicketFields(void);
	void AddAutoActions(void);
	void AssignDefaultValues(void);
	void AddTicketFieldTicket( long TicketID, long TicketFieldID, TCHAR* DataValue );
	void UpdateTicketBoxViews(void);

	TCHAR m_StdRespName[STANDARDRESPONSES_SUBJECT_LENGTH];
	long m_StdRespNameLen;
	bool m_bRefreshLeftPane;
	bool m_assignDefaultValues;

	vector<TTicketFieldsTicketBox> m_tftb;
	TTicketFieldsTicketBox tftb;
	vector<TTicketFieldsTicketBox>::iterator mtftbIter;

	list<int> m_tid;
	list<int>::iterator mtidIter;

	list<TPersonalData> m_Data;
	list<int> m_IDs;
	int m_MaxPersonalDataID;

	vector<TAutoActions> m_aa;
	TAutoActions aa;
	vector<TAutoActions>::iterator aaIter;	
	int m_MaxAutoActionID;
	
	CTicket m_Ticket;
};

#endif // !defined(AFX_TICKETBOX_H__C1E99A69_3055_4C44_AE93_7D4402430C92__INCLUDED_)
