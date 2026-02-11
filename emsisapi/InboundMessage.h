/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/InboundMessage.h,v 1.2 2005/11/29 21:16:26 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

// only include this file once
#pragma once

#include "XMLDataClass.h"
#include "Attachment.h"

class CInboundMessage : public TInboundMessages, public CXMLDataClass  
{
public:
	CInboundMessage(CISAPIData& ISAPIData);
	virtual ~CInboundMessage();

	virtual int Run(CURLAction& action);

	void GenerateXML( bool bEscapeBody = false, int nArchiveID = 0, bool bForEdit = false );
	void Query( void );
	void QueryMinimal( void );
	void Delete( void );

	list<CAttachment>& GetAttachmentList( void ) { return m_AttachList; }
	void SetID( int nID )	{ m_InboundMessageID = nID; }
	int GetBeenRead() { return m_BeenRead; }
	void static UpdateTicketHistoryMsgRead(CODBCQuery& query, int nAgentID, int nTicketID, int nRead, int nMsgID, int nTicketBoxID);
	
protected:

	TIMESTAMP_STRUCT m_RepliedToWhen;
	long			 m_RepliedToWhenLen;
	int			     m_BeenRead;
	int              m_UseCutoffDate;
	
	list<CAttachment> m_AttachList;

private:
	void VirusScanBody(void);
	bool UseCutoffDate();
};
