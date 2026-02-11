#pragma once

#include "XMLDataClass.h"

class CTicketPrint : public CXMLDataClass  
{
public:
	CTicketPrint( CISAPIData& ISAPIData );
	virtual ~CTicketPrint() {};

	virtual int Run( CURLAction& action );

protected:
	void DecodeForm(void);
	void GenerateXML(void);
	void GenerateTicketXML(void);
	void GenerateIMXML(void);
	void GenerateOMXML(void);
	void GenerateTNXML(void);
	void GenerateTEXML(void);

private:
	int m_TicketID;
	int m_TicketSummary;
	int m_InboundMessages;
	int m_OutboundMessages;
	int m_TicketNotes;
	int m_TicketEvents;	
};
