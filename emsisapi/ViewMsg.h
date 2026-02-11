/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ViewMsg.h,v 1.2 2005/11/29 21:16:28 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"
#include "Ticket.h"
#include "InboundMessage.h"
#include "OutboundMessage.h"

class CViewMsg : public CXMLDataClass  
{
public:
	CViewMsg(CISAPIData& ISAPIData);
	virtual ~CViewMsg() {};
	
	int virtual Run( CURLAction& action );
	
private:
	
	void GetSourceMsgIDAndType(void);
	void GenerateXML(void);
	void ViewMsg(const TCHAR* pAction = NULL);

private:
	
	CTicket				m_Ticket;
	CInboundMessage		m_InboundMessage;
	COutboundMessage	m_OutboundMessage;
	
	bool				m_bIsInbound;
	bool				m_bRefreshRequired;
	int					m_nArchiveID;
	int					m_nUpdateRead;
	int		            m_tResult;
	long				m_lStart;
	long				m_lEnd;
	long				m_lElapsed;
	dca::String			f;
		
};
