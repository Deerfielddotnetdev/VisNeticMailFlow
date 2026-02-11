// OutboundMessageSerializer.cpp: implementation of the CSerOutboundMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerOutboundMessage.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerOutboundMessage::CSerOutboundMessage()
{

}

CSerOutboundMessage::~CSerOutboundMessage()
{

}


void CSerOutboundMessage::LoadFromDB(const int nMsgID, CODBCQuery& query)
{
	//assert(nMsgID >= 0);

	m_OutboundMessageID = nMsgID;

	Query(query);
}

void CSerOutboundMessage::WriteToFile(CSerialFile &sfile)
{
	PutValue(sfile, m_OutboundMessageID);
	PutVarChar(sfile, m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH * sizeof(TCHAR));
	PutVarChar(sfile, m_MediaType, OUTBOUNDMESSAGES_MEDIATYPE_LENGTH * sizeof(TCHAR));
	PutVarChar(sfile, m_MediaSubType, OUTBOUNDMESSAGES_MEDIASUBTYPE_LENGTH * sizeof(TCHAR));
	PutText(sfile, m_Body, m_BodyLen);
	PutText(sfile, m_EmailTo, m_EmailToLen);
	PutText(sfile, m_EmailCc, m_EmailCcLen);
	PutText(sfile, m_EmailBcc, m_EmailBccLen);
	PutText(sfile, m_EmailReplyTo, m_EmailReplyToLen);
}

void CSerOutboundMessage::ReadFromFile(CSerialFile &sfile)
{
	GetValue(sfile, m_OutboundMessageID);
	GetVarChar(sfile, m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH * sizeof(TCHAR));
	GetVarChar(sfile, m_MediaType, OUTBOUNDMESSAGES_MEDIATYPE_LENGTH * sizeof(TCHAR));
	GetVarChar(sfile, m_MediaSubType, OUTBOUNDMESSAGES_MEDIASUBTYPE_LENGTH * sizeof(TCHAR));

	if (m_BodyAllocated) free(m_Body);
	GetText(sfile, m_Body, m_BodyLen);
	m_BodyAllocated = m_BodyLen;

	if (m_EmailToAllocated) free(m_EmailTo);
	GetText(sfile, m_EmailTo, m_EmailToLen);
	m_EmailToAllocated = m_EmailToLen;

	if (m_EmailCcAllocated) free(m_EmailCc);
	GetText(sfile, m_EmailCc, m_EmailCcLen);
	m_EmailCcAllocated = m_EmailCcLen;

	if (m_EmailBccAllocated) free(m_EmailBcc);
	GetText(sfile, m_EmailBcc, m_EmailBccLen);
	m_EmailBccAllocated = m_EmailBccLen;

	if (m_EmailReplyToAllocated) free(m_EmailReplyTo);
	GetText(sfile, m_EmailReplyTo, m_EmailReplyToLen);
	m_EmailReplyToAllocated = m_EmailReplyToLen;
}

void CSerOutboundMessage::SaveToDB(CODBCQuery& query)
{
	// this record is no longer archived
	m_ArchiveID = 0;
	
	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailFrom );
	BINDPARAM_TCHAR( query, m_MediaType );
	BINDPARAM_TCHAR( query, m_MediaSubType );
	BINDPARAM_TEXT( query, m_Body );
	BINDPARAM_TEXT( query, m_EmailTo );
	BINDPARAM_TEXT( query, m_EmailCc );
	BINDPARAM_TEXT( query, m_EmailBcc );
	BINDPARAM_TEXT( query, m_EmailReplyTo );
	BINDPARAM_LONG( query, m_ArchiveID);
	BINDPARAM_LONG( query, m_OutboundMessageID );
	query.Execute( _T("UPDATE OutboundMessages ")
	               _T("SET EmailFrom=?,MediaType=?,MediaSubType=?,Body=?,EmailTo=?,EmailCc=?,EmailBcc=?,EmailReplyTo=?,ArchiveID=? ")
	               _T("WHERE OutboundMessageID=?") );
}

void CSerOutboundMessage::SetArchived(CODBCQuery &query, const int nArcID)
{
	// set text fields to empty string
	m_EmailFrom[0] = _T('\0');
	m_MediaType[0] = _T('\0');
	m_MediaSubType[0] = _T('\0');
	m_Body[0] = _T('\0');
	m_EmailTo[0] = _T('\0');
	m_EmailCc[0] = _T('\0');
	m_EmailBcc[0] = _T('\0');
	m_EmailReplyTo[0] = _T('\0');
	
	// set archive ID
	m_ArchiveID = nArcID;

	// update record
	Update(query);
}

void CSerOutboundMessage::PurgeRecord(CODBCQuery &query, const int nID)
{
	m_OutboundMessageID = nID;
	
	Delete(query);
}
