// InboundMessageSerializer.h: interface for the CSerInboundMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INBOUNDMESSAGESERIALIZER_H__207547AA_9C63_41B2_B608_7F8CF782E4EB__INCLUDED_)
#define AFX_INBOUNDMESSAGESERIALIZER_H__207547AA_9C63_41B2_B608_7F8CF782E4EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Serializable.h"
#include "SerialFile.h"

class CSerInboundMessage : public TInboundMessages, CSerializable
{
public:
	void PurgeRecord(CODBCQuery& query, const int nMsgID);
	void SetArchived(CODBCQuery& query, const int nArcID);
	CSerInboundMessage();
	virtual ~CSerInboundMessage();

	void LoadFromDB(const int nMsgID, CODBCQuery& query);
	void SaveToDB(CODBCQuery& query);

	void WriteToFile(CSerialFile& sfile);
	void ReadFromFile(CSerialFile& sfile);

};

#endif // !defined(AFX_INBOUNDMESSAGESERIALIZER_H__207547AA_9C63_41B2_B608_7F8CF782E4EB__INCLUDED_)
