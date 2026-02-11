// OutboundMessage.cpp: implementation of the COutboundMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MailComponents.h"
#include "OutboundMessage.h"
#include "SingleCrit.h"
#include "ODBCConn.h"
#include "ODBCQuery.h"

extern dca::Mutex g_csDB;
extern CODBCConn g_odbcConn;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COutboundMessage::COutboundMessage()
{
	m_lMessageDestID = 0;
	m_bSpoolFileGenerated = FALSE;
	m_lOutboundMessageState = 0;
}

COutboundMessage::~COutboundMessage()
{

}

const int COutboundMessage::SaveQueued()
{
	int nRet = 0;
	USES_CONVERSION;

	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		LONG lDeletedBy = 0;
		LONG lIsDeleted = 0;
		LONG lAgentID = 0;
		LONG lReplyToMsgID = 0;
		LONG lMsgType = EMS_OUTBOUND_MESSAGE_TYPE_NEW;
		LONG LMsgState = EMS_OUTBOUND_MESSAGE_STATE_UNTOUCHED;

		tstring sBody;
		sBody.resize(GetMemoText().GetText().size());
		MultiByteToWideChar(CP_ACP, 
							0, 
							GetMemoText().GetText().c_str(), 
							GetMemoText().GetText().size(),
							(TCHAR*)sBody.data(),
							sBody.size());

		tstring sTo = GetAddressListString(GetToRecipients());
		tstring sCC = GetAddressListString(GetCCRecipients());
		tstring sBCC = GetAddressListString(GetBCCRecipients());
		tstring sReplyTo = GetAddressListString(GetReplyToList());

		BINDPARAM_LONG(query, m_nTicketID);		
		BINDPARAM_TCHAR(query, A2T(this->GetOriginator().GetInetName().c_str()));
		BINDPARAM_TCHAR(query, A2T(this->GetToRecipients().Get(0).GetInetName().c_str()));
		BINDPARAM_TIME_NOLEN(query, this->GetDate().GetSQLTime());
		BINDPARAM_TCHAR(query, A2T(this->GetSubject().GetText().c_str()));
		BINDPARAM_TCHAR(query, A2T(this->GetMemoText().GetMediaType().c_str()));
		BINDPARAM_TCHAR(query, A2T(this->GetMemoText().GetMediaSubType().c_str()));
		BINDPARAM_LONG(query, lDeletedBy);
		BINDPARAM_LONG(query, lIsDeleted);
		BINDPARAM_LONG(query, lAgentID);
		BINDPARAM_LONG(query, lReplyToMsgID);
		BINDPARAM_LONG(query, lMsgType);
		BINDPARAM_LONG(query, LMsgState);
		BINDPARAM_TEXT_STRING(query, sBody);
		BINDPARAM_TEXT_STRING(query, sTo);
		BINDPARAM_TEXT_STRING(query, sCC);
		BINDPARAM_TEXT_STRING(query, sBCC);
		BINDPARAM_TEXT_STRING(query, sReplyTo);

		query.Execute( _T("INSERT INTO OutboundMessages ")
					   _T("(TicketID,EmailFrom,EmailPrimaryTo,EmailDateTime,Subject,MediaType,MediaSubType,DeletedBy,IsDeleted,AgentID,ReplyToMsgID,OutboundMessageTypeID,OutboundMessageStateID,Body,EmailTo, EmailCc, EmailBcc,EmailReplyTo) ")
					   _T("VALUES ")
					   _T("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));

		// insert into outbound message queue
		LONG lOutMsgID = query.GetLastInsertedID();
		query.Reset(true);

		CEmailDate curDate;
		BINDPARAM_LONG(query, lOutMsgID);
		BINDPARAM_TIME_NOLEN(query, curDate.GetSQLTime());

		query.Execute(	_T("INSERT INTO OutboundMessageQueue ")
						_T("(OutboundMessageID, MessageDestinationID, SpoolFileGenerated, IsApproved, DateSpooled) ")
						_T("VALUES ")
						_T("(?,0,0,1,?)"));
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		nRet = ERROR_DATABASE;
	}

	return 0;
}
