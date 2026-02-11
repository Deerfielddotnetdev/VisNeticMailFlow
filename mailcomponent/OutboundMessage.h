// OutboundMessage.h: interface for the COutboundMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OUTBOUNDMESSAGE_H__D5F0D1DD_EB03_4AEB_B29F_B91AE4A677D1__INCLUDED_)
#define AFX_OUTBOUNDMESSAGE_H__D5F0D1DD_EB03_4AEB_B29F_B91AE4A677D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Message.h"

class COutboundMessage : public CMessage  
{
public:
	const int SaveQueued();
	COutboundMessage();
	virtual ~COutboundMessage();

	void SetMessageDestID(const LONG lVal)			{ m_lMessageDestID = lVal; }
	void SetSpoolFileGenerated(const BOOL bVal)		{ m_bSpoolFileGenerated = bVal; }
	void SetOutboundMessageState(const LONG lVal)	{ m_lOutboundMessageState = lVal; }
	void SetLastDeliveryAttempt(const CEmailDate& date) { m_LastDeliveryAttempt = date; }

	const LONG GetMessageDestID()					{ return m_lMessageDestID; }
	const BOOL GetSpoolFileGenerated()				{ return m_bSpoolFileGenerated; }
	const LONG GetOutboundMessageState()			{ return m_lOutboundMessageState; }
	CEmailDate& GetDateSpooled()					{ return m_DateSpooled; }
	CEmailDate& GetLastDeliveryAttempt()			{ return m_LastDeliveryAttempt; }
	
	CEmailText& GetFooter()							{ return m_Footer; }
	CEmailText& GetHeader()							{ return m_Header; }

private:
	LONG m_lMessageDestID;			// from database
	BOOL m_bSpoolFileGenerated;
	LONG m_lOutboundMessageState;	// defined in EMSIDs.h
	CEmailDate m_DateSpooled;
	CEmailDate m_LastDeliveryAttempt;
	CEmailText m_Footer;
	CEmailText m_Header;
};

#endif // !defined(AFX_OUTBOUNDMESSAGE_H__D5F0D1DD_EB03_4AEB_B29F_B91AE4A677D1__INCLUDED_)
