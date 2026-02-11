// InboundMessage.h: interface for the CInboundMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INBOUNDMESSAGE_H__E81DBD5B_D3EB_4692_9FAE_40092F905EE5__INCLUDED_)
#define AFX_INBOUNDMESSAGE_H__E81DBD5B_D3EB_4692_9FAE_40092F905EE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Message.h"
#include "EmailAddress.h"
#include "CriticalSection.h"
#include <CkZip.h>
#include <CkZipEntry.h>
#include <CkGlobal.h>

class CInboundMessage : public CMessage  
{
public:
	CInboundMessage();
	virtual ~CInboundMessage();

	const UINT Save(UINT nMessageSourceID, BOOL bUseReplyTo, UINT nZipAttach, BOOL bMsgIsDup);
	const BOOL GetPartial()				{ return m_bPartial; }
	void SetPartial(const BOOL bVal)	{ m_bPartial = bVal; }
public:
	const UINT QueueInboundMsg();
	const UINT SaveMessageDB(UINT nMessageSourceID, BOOL bUseReplyTo, bool bIsError = false);
	const UINT CommitSave();

private:
	BOOL m_bPartial;		// indicates that message was only partially downloaded (probably due to large size)
	const int DeleteMsgDBEntry();
	const UINT DeleteAttachmentFiles();
	const UINT SaveAttachmentsDB();
	const UINT SaveAttachmentFiles();
	const UINT AddAttachmentsToZip();	
};

#endif // !defined(AFX_INBOUNDMESSAGE_H__E81DBD5B_D3EB_4692_9FAE_40092F905EE5__INCLUDED_)
