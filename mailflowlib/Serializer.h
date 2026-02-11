// Serializer.h: interface for the CSerializer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIALIZER_H__E416F633_64E7_46BB_A890_8B9200B22ADC__INCLUDED_)
#define AFX_SERIALIZER_H__E416F633_64E7_46BB_A890_8B9200B22ADC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>
using namespace std;

class CODBCQuery;
class CSerialFile;

class CSerializer  
{
public:
	int RestoreArchive(CODBCQuery& query, const UINT nArcID);
	int ArchivePurge(CODBCQuery& query, const tstring& sDestFile, const int nDays, const BOOL bDoPurge = FALSE);
	CSerializer();
	virtual ~CSerializer();

private:
	void DBCleanup(CODBCQuery& query, const BOOL bDoPurge, long lArcID);
	void RestoreOutboundMsgAttachments(CSerialFile &sfile, const int nCount);
	void RestoreOutboundMsgs(CODBCQuery& query, CSerialFile &sfile, const int nCount);
	void RestoreInboundMsgAttachments(CSerialFile &sfile, const int nCount);
	void RestoreInboundMsgs(CODBCQuery& query, CSerialFile& sfile, const int nCount);
	BOOL InflateArchive(const tstring& sSource, const tstring& sArcPath);
	long AddArcRecord(CODBCQuery& query, const tstring& sArcPath, const BOOL bIsPurged);
	void DeleteAttachFiles();
	void SerializeOutboundMsgAttachments(CODBCQuery& query, CSerialFile& sfile);
	void GetOutboundMessageAttachments(CODBCQuery& query);
	void SerializeOutboundMsgs(CODBCQuery& query, CSerialFile& sfile);
	void GetOutboundMessages(CODBCQuery& query, int nDays);
	const tstring GetTempFile();
	BOOL DeflateArchive(const tstring& sSource, const tstring& sArcPath);
	void SerializeInboundMsgAttachments(CODBCQuery& query, CSerialFile& sfile);
	void GetInboundMessages(CODBCQuery& query, int nDays);
	void GetInboundMessageAttachments(CODBCQuery& query);
	void SerializeInboundMsgs(CODBCQuery& query, CSerialFile& sfile);
	typedef list<UINT> UINT_LIST;
	typedef list<tstring> TSTRING_LIST;
	UINT_LIST m_InboundMsgIDs;
	UINT_LIST m_InboundMsgAttachIDs;
	UINT_LIST m_OutboundMsgIDs;
	UINT_LIST m_OutboundMsgAttachIDs;
	TSTRING_LIST m_AttachPaths;
};

#endif // !defined(AFX_SERIALIZER_H__E416F633_64E7_46BB_A890_8B9200B22ADC__INCLUDED_)
