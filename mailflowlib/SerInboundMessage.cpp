// InboundMessageSerializer.cpp: implementation of the CSerInboundMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerInboundMessage.h"
#include "SerializeException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerInboundMessage::CSerInboundMessage()
{

}

CSerInboundMessage::~CSerInboundMessage()
{

}

void CSerInboundMessage::LoadFromDB(const int nMsgID, CODBCQuery& query)
{
	//assert(nMsgID >= 0);

	m_InboundMessageID = nMsgID;

	Query(query);
}

void CSerInboundMessage::WriteToFile(CSerialFile &sfile)
{
	PutValue(sfile, m_InboundMessageID);
	PutVarChar(sfile, m_EmailFrom, INBOUNDMESSAGES_EMAILFROM_LENGTH * sizeof(TCHAR));
	PutVarChar(sfile, m_EmailFromName, INBOUNDMESSAGES_EMAILFROMNAME_LENGTH * sizeof(TCHAR));
	PutVarChar(sfile, m_EmailPrimaryTo, INBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH * sizeof(TCHAR));
	PutVarChar(sfile, m_MediaType, INBOUNDMESSAGES_MEDIATYPE_LENGTH * sizeof(TCHAR));
	PutVarChar(sfile, m_MediaSubType, INBOUNDMESSAGES_MEDIASUBTYPE_LENGTH * sizeof(TCHAR));
	PutVarChar(sfile, m_VirusName, INBOUNDMESSAGES_VIRUSNAME_LENGTH * sizeof(TCHAR));
	PutText(sfile, m_Body, m_BodyLen);
	PutText(sfile, m_PopHeaders, m_PopHeadersLen);
	PutText(sfile, m_EmailTo, m_EmailToLen);
	PutText(sfile, m_EmailCc, m_EmailCcLen);
	PutText(sfile, m_EmailReplyTo, m_EmailReplyToLen);
}

void CSerInboundMessage::ReadFromFile(CSerialFile &sfile)
{
	GetValue(sfile, m_InboundMessageID);
	GetVarChar(sfile, m_EmailFrom, INBOUNDMESSAGES_EMAILFROM_LENGTH * sizeof(TCHAR));
	GetVarChar(sfile, m_EmailFromName, INBOUNDMESSAGES_EMAILFROMNAME_LENGTH * sizeof(TCHAR));
	GetVarChar(sfile, m_EmailPrimaryTo, INBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH * sizeof(TCHAR));
	GetVarChar(sfile, m_MediaType, INBOUNDMESSAGES_MEDIATYPE_LENGTH * sizeof(TCHAR));
	GetVarChar(sfile, m_MediaSubType, INBOUNDMESSAGES_MEDIASUBTYPE_LENGTH * sizeof(TCHAR));
	GetVarChar(sfile, m_VirusName, INBOUNDMESSAGES_VIRUSNAME_LENGTH * sizeof(TCHAR));

	if (m_BodyAllocated) free(m_Body);
	GetText(sfile, m_Body, m_BodyLen);
	m_BodyAllocated = m_BodyLen;

	if (m_PopHeadersAllocated) free(m_PopHeaders);
	GetText(sfile, m_PopHeaders, m_PopHeadersLen);
	m_PopHeadersAllocated = m_PopHeadersLen;

	if (m_EmailToAllocated) free(m_EmailTo);
	GetText(sfile, m_EmailTo, m_EmailToLen);
	m_EmailToAllocated = m_EmailToLen;

	if (m_EmailCcAllocated) free(m_EmailCc);
	GetText(sfile, m_EmailCc, m_EmailCcLen);
	m_EmailCcAllocated = m_EmailCcLen;

	if (m_EmailReplyToAllocated) free(m_EmailReplyTo);
	GetText(sfile, m_EmailReplyTo, m_EmailReplyToLen);
	m_EmailReplyToAllocated = m_EmailReplyToLen;
}

void CSerInboundMessage::SaveToDB(CODBCQuery& query)
{
	// this record is no longer archived
	m_ArchiveID = 0;

	query.Initialize();
	BINDPARAM_TCHAR( query, m_EmailFrom );
	BINDPARAM_TCHAR( query, m_EmailFromName );
	BINDPARAM_TCHAR( query, m_EmailPrimaryTo );
	BINDPARAM_TCHAR( query, m_MediaType );
	BINDPARAM_TCHAR( query, m_MediaSubType );
	BINDPARAM_TCHAR( query, m_VirusName );
	BINDPARAM_TEXT( query, m_Body );
	BINDPARAM_TEXT( query, m_PopHeaders );
	BINDPARAM_TEXT( query, m_EmailTo );
	BINDPARAM_TEXT( query, m_EmailCc );
	BINDPARAM_TEXT( query, m_EmailReplyTo );
	BINDPARAM_LONG( query, m_ArchiveID);
	BINDPARAM_LONG( query, m_InboundMessageID );
	query.Execute( _T("UPDATE InboundMessages ")
	               _T("SET EmailFrom=?,EmailFromName=?,EmailPrimaryTo=?,MediaType=?,MediaSubType=?,VirusName=?,Body=?,PopHeaders=?,EmailTo=?,EmailCc=?,EmailReplyTo=?,ArchiveID=? ")
	               _T("WHERE InboundMessageID=?"));
}

void CSerInboundMessage::SetArchived(CODBCQuery &query, const int nArcID)
{
	// set text fields to empty string
	m_EmailFrom[0] = _T('\0');
	m_EmailFromName[0] = _T('\0');
	m_EmailPrimaryTo[0] = _T('\0');
	m_MediaType[0] = _T('\0');
	m_MediaSubType[0] = _T('\0');
	m_VirusName[0] = _T('\0');
	m_Body[0] = _T('\0');
	m_PopHeaders[0] = _T('\0');
	m_EmailTo[0] = _T('\0');
	m_EmailCc[0] = _T('\0');
	m_EmailReplyTo[0] = _T('\0');

	// set archive ID
	m_ArchiveID = nArcID;

	// update record
	Update(query);
}

void CSerInboundMessage::PurgeRecord(CODBCQuery &query, const int nMsgID)
{
	m_InboundMessageID = nMsgID;

	// delete record
	Delete(query);
}
