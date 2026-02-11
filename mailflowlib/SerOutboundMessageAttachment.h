// SerOutboundMessageAttachment.h: interface for the CSerOutboundMessageAttachment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEROUTBOUNDMESSAGEATTACHMENT_H__ADB74A58_6338_4D4A_BD06_6FB7EC1DB908__INCLUDED_)
#define AFX_SEROUTBOUNDMESSAGEATTACHMENT_H__ADB74A58_6338_4D4A_BD06_6FB7EC1DB908__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Serializable.h"
#include "SerialFile.h"

class CSerOutboundMessageAttachment : public TOutboundMessageAttachments, public CSerializable  
{
public:
	void PurgeRecord(CODBCQuery& query, const int nID);
	CSerOutboundMessageAttachment();
	virtual ~CSerOutboundMessageAttachment();

	void LoadFromDB(const int nMsgAttachID, CODBCQuery& query);

	void WriteToFile(CSerialFile& sfile, tstring& sAttachPath);
	void Restore(CSerialFile& sfile);
};

#endif // !defined(AFX_SEROUTBOUNDMESSAGEATTACHMENT_H__ADB74A58_6338_4D4A_BD06_6FB7EC1DB908__INCLUDED_)
