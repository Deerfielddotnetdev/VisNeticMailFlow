// SerInboundMessageAttachment.h: interface for the CSerInboundMessageAttachment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERINBOUNDMESSAGEATTACHMENT_H__172DEE7B_EA73_4227_B927_DECBB60E5D65__INCLUDED_)
#define AFX_SERINBOUNDMESSAGEATTACHMENT_H__172DEE7B_EA73_4227_B927_DECBB60E5D65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Serializable.h"
#include "SerialFile.h"

class CSerInboundMessageAttachment : public TInboundMessageAttachments, CSerializable
{
public:
	void PurgeRecord(CODBCQuery& query, const int nID);
	CSerInboundMessageAttachment();
	virtual ~CSerInboundMessageAttachment();

	void LoadFromDB(const int nMsgAttachID, CODBCQuery& query);

	void WriteToFile(CSerialFile& sfile, tstring& sAttachPath);
	void Restore(CSerialFile& sfile);
};

#endif // !defined(AFX_SERINBOUNDMESSAGEATTACHMENT_H__172DEE7B_EA73_4227_B927_DECBB60E5D65__INCLUDED_)
