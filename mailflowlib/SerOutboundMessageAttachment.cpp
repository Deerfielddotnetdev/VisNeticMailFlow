// SerOutboundMessageAttachment.cpp: implementation of the CSerOutboundMessageAttachment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerOutboundMessageAttachment.h"
#include "AttachFns.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerOutboundMessageAttachment::CSerOutboundMessageAttachment()
{

}

CSerOutboundMessageAttachment::~CSerOutboundMessageAttachment()
{

}

void CSerOutboundMessageAttachment::LoadFromDB(const int nMsgAttachID, CODBCQuery& query)
{
	//assert(nMsgAttachID >= 0);

	m_AttachmentID = nMsgAttachID;

	Query(query);
}

void CSerOutboundMessageAttachment::WriteToFile(CSerialFile &sfile, tstring& sAttachPath)
{
/*
	PutValue(sfile, m_OutboundMessageID);
	PutVarChar(sfile, m_AttachmentLocation, OUTBOUNDMESSAGEATTACHMENTS_ATTACHMENTLOCATION_LENGTH * sizeof(TCHAR));

	GetFullOutboundAttachPath(m_AttachmentLocation, sAttachPath);

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
	}
*/
}

void CSerOutboundMessageAttachment::Restore(CSerialFile &sfile)
{
/*
	GetValue(sfile, m_OutboundMessageID);
	GetVarChar(sfile, m_AttachmentLocation, OUTBOUNDMESSAGEATTACHMENTS_ATTACHMENTLOCATION_LENGTH * sizeof(TCHAR));

	// to do: any potential for filename conflicts when this
	// attachment file is restored?
	tstring sFullPath;
	GetFullOutboundAttachPath(m_AttachmentLocation, sFullPath);
	GetFile(sfile, sFullPath);
*/
}

void CSerOutboundMessageAttachment::PurgeRecord(CODBCQuery &query, const int nID)
{
	m_AttachmentID = nID;

	Delete(query);
}
