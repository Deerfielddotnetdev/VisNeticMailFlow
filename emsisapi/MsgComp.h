/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/MsgComp.h,v 1.2.2.1 2006/02/23 20:37:12 markm Exp $
||
||
||                                         
||  COMMENTS:	Message Composition  
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"
#include "Ticket.h"
#include "InboundMessage.h"
#include "OutboundMessage.h"
#include "Attachment.h"
#include "ContactFns.h"

class CMsgComp : public CXMLDataClass  
{
public:
	
	CMsgComp(CISAPIData& ISAPIData);
	virtual ~CMsgComp() {};
	
	virtual int Run(CURLAction& action);

private:
	
	void ReleaseLock(void);

	void GetSourceMsgIDAndType(void);
	void SetPageType(void);
	void AddPageTypeXML(void);
	
	void GenerateXML(void);
	void DecodeForm(void);
	
	void QuerySourceMsg(void);
	void AddSourceMsgAttachments( int nMsgID );
	template<class T> void ProcessSrcMsg( T& Message );
	
	template<class T> void ReplyToAll( T& Message );
	void MangleSubject( LPTSTR szMsgSubject );

	void SaveMessage(void);
	void CreateMessage(void);
	void SendMessage(void);
	
	void UpdateTicket(void);
	void UpdateMessage(void);

	void ProcessContacts( list<EmailAddr_t>& ContactList, list<int>& ContactIDs );

	void AddContactEmailAddr( int nContactID );
	void GetTicketBoxHeaderFooter( void );
	void BuildTicketEmailAddresses( CEMSString& sAddresses );
	void BuildContactEmailAddresses( void );
	void BuildContactGroups( void );
	void ListStdResps( void );
	
	void RefreshParentView( void );
	void GetAgentEmail(dca::String& agentEmail);
	void GetTicketBoxEmail(dca::String& ticketboxEmail);
	void CheckAgentAlerts( int nAgentID, TCHAR szSubject[TICKETS_SUBJECT_LENGTH] );	

private:

	CTicket m_Ticket;
	CInboundMessage		m_InboundMessage;
	COutboundMessage	m_OutboundMessage;

	tstring m_sNote;
	unsigned char 		m_UseHTML;
	long				m_originalInboundMessageID;
	
	int m_nPageType;
	MsgID_t m_SrcMsg;
	bool m_bDraft;
	bool m_bReplyToAll;
	bool m_bCreatedTicket;
	bool m_bOutboundApproval;
	int  m_nApprovalID;

	tstring m_sOutboundMessageSubject;

	vector<TTicketFieldsTicket> m_tft;
	vector<TTicketFieldsTicket>::iterator tftIter;

	int m_nArchiveID;	
	
	enum 
	{ 
		EMS_NewMessage		= EMS_OUTBOUND_MESSAGE_TYPE_NEW,
		EMS_ForwardMessage	= EMS_OUTBOUND_MESSAGE_TYPE_FORWARD,
		EMS_ReplyToMessage	= EMS_OUTBOUND_MESSAGE_TYPE_REPLY,
		EMS_ViewMessage		= 4,
		EMS_SendAgain		= 5,
		EMS_SendDraft		= 6
	} PageTypeEnum;	
};
