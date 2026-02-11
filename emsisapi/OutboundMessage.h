/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/OutboundMessage.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

// only include this file once
#pragma once

#include "XMLDataClass.h"
#include "Attachment.h"

class COutboundMessage : public TOutboundMessages, public CXMLDataClass  
{
public:
	COutboundMessage(CISAPIData& ISAPIData);
	virtual ~COutboundMessage();

	int Run(CURLAction& action) { return 0; }

	void GenerateXML( bool bEscapeBody = false, int nArchiveID = 0, bool bForEdit = false );
	void Query( void );
	void Delete( void );
	void DeleteOrphan( void );

	int  ReserveID( int nTicketID );
	void Revoke( void );
	void Release( void );
	void Return( void );
	void CheckAgentAlerts( int nAgentID, int nAlertEventID );

	void GetOutboundMessageType( int nOutboundMsgID );
	void GetReplyToIDAndType( int nOutboundMsgID );

	list<CAttachment>& GetAttachmentList( void ) { return m_AttachList; }
	void SetID( int nID )	{ m_OutboundMessageID = nID; }

	tstring sAal;
	tstring sTmrev;
	tstring sTmrel;
	tstring sTmret;
	
protected:
	TIMESTAMP_STRUCT m_RepliedToWhen;
	long			 m_RepliedToWhenLen;
	
	list<CAttachment> m_AttachList;

	void LogAgentAction( long nAgentID,long nActionID,long nId1,long nId2,long nId3,CEMSString sData1,CEMSString sData2 );

};
