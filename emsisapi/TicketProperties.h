/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketProperties.h,v 1.2 2005/11/29 21:16:28 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

// only includ eonce
#pragma once

#include "XMLDataClass.h"
#include "Ticket.h"
#include <CkString.h>

class CTicketProperties : public CXMLDataClass  
{
public:
	CTicketProperties(CISAPIData& ISAPIData );
	virtual ~CTicketProperties() {};

	virtual int Run(CURLAction& action);

private:
	void GenerateXML();
	void UpdateTicket();
	void DecodeForm();
	void ListMyFolders();
	void UpdateSubject(bool& m_bUpdatedTicket);
	void MarkAlertAsRead();
	void ClearAllAlerts();	

private:
	CTicket m_Ticket;
	bool m_bMultipleTickets;
	bool m_bEscalate;
	bool m_bChangeSubject;
	bool m_bChangeIBSubjects;
	TCHAR m_NewSubject[TICKETS_SUBJECT_LENGTH];	
	int nAlertID;
	vector<TTicketFieldsTicket> m_tft;
	vector<TTicketFieldsTicket>::iterator tftIter;
};
