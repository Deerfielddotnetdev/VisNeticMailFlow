/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ChangeTicket.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Change Ticket
||              
\\*************************************************************************/

#include "XMLDataClass.h"
#include "Ticket.h"
#include "InboundMessage.h"
#include "OutboundMessage.h"


// structure to hold child message query results
typedef struct TicketMsg_t
{
	TicketMsg_t() 
	{ 
		ZeroMemory( this, sizeof(TicketMsg_t) );
	}
	
	int MsgID;
	bool IsInbound;
	bool bUsed;	
	
} TicketMsg_t;


typedef multimap<MsgID_t, TicketMsg_t> CReplyToMap;


class CChangeTicket : public CXMLDataClass  
{
public:
	CChangeTicket(CISAPIData& ISAPIData);
	virtual ~CChangeTicket() {};

	int Run(CURLAction& action);
	void MoveMessage( void );
	void CopyMessage( void );
	void GetTicketBoxView( int nTicketID, int nSrcViewID, bool bShowDest );

private:
	void GetSourceTicketID( void );
	void FindChildMessages( void );
	void AddChildMessages( int nMsgID, bool bIsInbound, CReplyToMap& ReplyMap );
	int  GetDestTicketBoxID( void );

	CInboundMessage		m_InboundMessage;
	COutboundMessage	m_OutboundMessage;

public:
	CTicket m_SourceTicket;
	MsgID_t m_SourceMsg;
	int m_nDestTicketID;
	int m_nTicketLink;
	int m_nLinkCount;

	list<TicketMsg_t> m_ChildMessageList;
};