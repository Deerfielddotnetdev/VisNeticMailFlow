// SerInboundMessageAttachment.cpp: implementation of the CSerInboundMessageAttachment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerInboundMessageAttachment.h"
#include "AttachFns.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerInboundMessageAttachment::CSerInboundMessageAttachment()
{

}

CSerInboundMessageAttachment::~CSerInboundMessageAttachment()
{

}

void CSerInboundMessageAttachment::LoadFromDB(const int nMsgAttachID, CODBCQuery& query)
{
	//assert(nMsgAttachID >= 0);

//	m_MessageAttachmentID = nMsgAttachID;

	Query(query);
}

void CSerInboundMessageAttachment::WriteToFile(CSerialFile &sfile, tstring& sAttachPath)
{
/*
	PutValue(sfile, m_MessageAttachmentID);
	PutVarChar(sfile, m_AttachmentLocation, INBOUNDMESSAGEATTACHMENTS_ATTACHMENTLOCATION_LENGTH * sizeof(TCHAR));

	GetFullInboundAttachPath(m_AttachmentLocation, sAttachPath);

	try
	{
		PutFile(sfile, sAttachPath);
	}
	catch (CSerializeException ex)
	{
		// if the attachment file is missing, skip it
		if (ex.GetErrCode() == ERROR_FILE_NOT_FOUND)
		{
		}
		else
		{
			// else pass exception on, which will stop processing
			throw ex;
		}
*/
}

void CSerInboundMessageAttachment::Restore(CSerialFile &sfile)
{
/*
	GetValue(sfile, m_MessageAttachmentID);
	GetVarChar(sfile, m_AttachmentLocation, INBOUNDMESSAGEATTACHMENTS_ATTACHMENTLOCATION_LENGTH * sizeof(TCHAR));

	// to do: any potential for filename conflicts when this
	// attachment file is restored?
	tstring sFullPath;
	GetFullInboundAttachPath(m_AttachmentLocation, sFullPath);
	GetFile(sfile, sFullPath);
*/
}


void CSerInboundMessageAttachment::PurgeRecord(CODBCQuery &query, const int nID)
{
/*
	m_MessageAttachmentID = nID;
	
	Delete(query);
*/
}
