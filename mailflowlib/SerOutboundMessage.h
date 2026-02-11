// OutboundMessageSerializer.h: interface for the CSerOutboundMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OUTBOUNDMESSAGESERIALIZER_H__FF85F3C5_9D12_4537_8D7C_431090EDC7AE__INCLUDED_)
#define AFX_OUTBOUNDMESSAGESERIALIZER_H__FF85F3C5_9D12_4537_8D7C_431090EDC7AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Serializable.h"
#include "SerialFile.h"

class CSerOutboundMessage : public TOutboundMessages, CSerializable
{
public:
	void PurgeRecord(CODBCQuery& query, const int nID);
	void SetArchived(CODBCQuery& query, const int nArcID);
	CSerOutboundMessage();
	virtual ~CSerOutboundMessage();

	void LoadFromDB(const int nMsgID, CODBCQuery& query);
	void SaveToDB(CODBCQuery& query);

	void WriteToFile(CSerialFile& sfile);
	void ReadFromFile(CSerialFile& sfile);

};

#endif // !defined(AFX_OUTBOUNDMESSAGESERIALIZER_H__FF85F3C5_9D12_4537_8D7C_431090EDC7AE__INCLUDED_)
