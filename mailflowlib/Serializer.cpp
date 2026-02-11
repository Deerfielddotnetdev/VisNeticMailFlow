// Serializer.cpp: implementation of the CSerializer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Serializer.h"
#include "SerializeException.h"
#include "SerInboundMessage.h"
#include "SerOutboundMessage.h"
#include "SerialFile.h"
#include "SerInboundMessageAttachment.h"
#include "SerOutboundMessageAttachment.h"
#include "zipper.h"
#include "EMSException.h"
#include "ArchiveEntry.h"
#include "DateFns.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerializer::CSerializer()
{
}

CSerializer::~CSerializer()
{
}

// ticket criteria:
//	- ticket status closed and not locked
//	- associated messages older than the cutoff date (nDays ago)
int CSerializer::ArchivePurge(CODBCQuery& query, const tstring &sDestFile, const int nDays, const BOOL bDoPurge)
{
	int nRet = 0;
	bool bDelSerFileOnErr = true;

	// if not purging, we must have an archive destination
	if (!bDoPurge)
	{
		//assert(sDestFile.size() != 0);
		if (sDestFile.size() == 0)
			return ERROR_INVALID_PARAMETER;
	}

	// tmp file location
	const tstring sTmp = GetTempFile();
	if (sTmp.size() == 0)
		return ERROR_PATH_NOT_FOUND;

	// the temp file location becomes our serialization file
	CSerialFile serFile;
	int nRetArc = serFile.OpenArchive(sTmp, CSerialFile::serializer_dir_out);
	if (nRetArc != 0)
		return nRetArc;

	try
	{
		query.Initialize();

		// the first two steps of this process are only executed
		// if we're performing an archive (rather than a purge)
		if (!bDoPurge)
		{
			/////////////////////////////////////////
			// step 1 - serialize inbound messages
			/////////////////////////////////////////


			// get inbound messages
			GetInboundMessages(query, nDays);

			// serialize inbound messages
			SerializeInboundMsgs(query, serFile);

			// get inbound attachments
			GetInboundMessageAttachments(query);

			// serialize inbound attachments
			SerializeInboundMsgAttachments(query, serFile);

			/////////////////////////////////////////
			// step 2 - serialize outbound messages
			/////////////////////////////////////////


			// get outbound messages
			GetOutboundMessages(query, nDays);

			// serialize outbound messages
			SerializeOutboundMsgs(query, serFile);

			// get outbound attachments
			GetOutboundMessageAttachments(query);

			// serialize outbound message attachments
			SerializeOutboundMsgAttachments(query, serFile);

		} // if (!bDoPurge)

		// if anything goes wrong prior to step 3, we can 
		// safely delete the temporary serialization file in our 
		// exception handlers; but from here out, we'll leave it
		bDelSerFileOnErr = false;

		/////////////////////////////////////////
		// step 3 - finalization
		/////////////////////////////////////////


		// close archive file
		serFile.CloseArchive();


		// deflate the archive file into its final location
		if (!DeflateArchive(sTmp, sDestFile))
			return -1;


		// create entry in Archives table
		long lArcID = AddArcRecord(query, sDestFile, bDoPurge);


		// delete attachment files
		DeleteAttachFiles();


		// cleanup DB records
		DBCleanup(query, bDoPurge, lArcID);

	}
#pragma warning (disable:4101)
	catch (CSerializeException sx)
	{
		if (bDelSerFileOnErr)
		{
			serFile.CloseArchive();
			DeleteFile(sTmp.c_str());
		}

		nRet = sx.GetErrCode();
	}
	catch(...)
	{
		if (bDelSerFileOnErr)
		{
			serFile.CloseArchive();
			DeleteFile(sTmp.c_str());
		}

		nRet = -1;
	}
#pragma warning (default:4101)

	// delete the temp archive file (if created)
	if (nRetArc == 0)
		DeleteFile(sTmp.c_str());

	return nRet; // success
}

void CSerializer::SerializeInboundMsgs(CODBCQuery &query, CSerialFile& sfile)
{
	query.Reset(true);

	// iterate and read/write
	for (	UINT_LIST::iterator cur(m_InboundMsgIDs.begin());
			cur != m_InboundMsgIDs.end();
			++cur)
	{
		// load our serializable message object
		CSerInboundMessage serInMsg;
		serInMsg.LoadFromDB(*cur, query);
		serInMsg.WriteToFile(sfile);
	}
}

void CSerializer::GetInboundMessageAttachments(CODBCQuery &query)
{
	query.Reset(true);

	LONG lInboundMsgAttID;

	// iterate and read/write
	for (	UINT_LIST::iterator cur(m_InboundMsgIDs.begin());
			cur != m_InboundMsgIDs.end();
			++cur)
	{
		BINDCOL_LONG_NOLEN(query, lInboundMsgAttID);
		BINDPARAM_LONG(query, *cur);
		query.Execute(	_T("select	MessageAttachmentID ")
						_T("from	InboundMessageAttachments ")
						_T("where	InboundMessageID = ?"));

		while (query.Fetch() == S_OK)
			m_InboundMsgAttachIDs.push_back(lInboundMsgAttID);

		query.Reset(true);
	}
}

void CSerializer::GetInboundMessages(CODBCQuery &query, int nDays)
{
	query.Reset(true);

	LONG lInboundMsgID;
	BINDCOL_LONG_NOLEN(query, lInboundMsgID);
	BINDPARAM_LONG(query, nDays);
	query.Execute(	_T("select 	ib.InboundMessageID ")
					_T("from	InboundMessages as ib, Tickets as ti ")
					_T("where	ib.TicketID = ti.TicketID ")
					_T("and		ti.LockedBy = 0" )
					_T("and		ti.TicketStateID = 1 ")
					_T("and		ib.ArchiveID = 0 ")
					_T("and		DATEDIFF(day, ib.EmailDateTime, getdate()) > ?"));
	while (query.Fetch() == S_OK)
	{
		// store off the message id for later use
		m_InboundMsgIDs.push_back(lInboundMsgID);
	}
}

void CSerializer::SerializeInboundMsgAttachments(CODBCQuery &query, CSerialFile &sfile)
{
	tstring sFilePath;

	// iterate and read/write
	for (	UINT_LIST::iterator cur(m_InboundMsgAttachIDs.begin());
			cur != m_InboundMsgAttachIDs.end();
			++cur)
	{
		CSerInboundMessageAttachment ina;
		ina.LoadFromDB(*cur, query);
		ina.WriteToFile(sfile, sFilePath);

		m_AttachPaths.push_back(sFilePath);
	}
}

BOOL CSerializer::DeflateArchive(const tstring& sSource, const tstring &sArcPath)
{
	// to do: better error checking
	CZipper zipper;
	int nRet = zipper.CompressFile(sSource.c_str(), sArcPath.c_str(), 6);
	if (nRet == Z_OK)
		return TRUE; // success

	return FALSE;
}

const tstring CSerializer::GetTempFile()
{
	// find a temp path for our compressed file
	TCHAR szTmp[_MAX_PATH] = {'\0'};
	DWORD dwRet = GetTempPath(_MAX_PATH, szTmp);
	if (dwRet != 0)
		dwRet = GetTempFileName(szTmp, _T("mfz"), 0, szTmp);	

	return tstring(szTmp);
}

void CSerializer::GetOutboundMessages(CODBCQuery &query, int nDays)
{
	query.Reset(true);

	// TO DO: TEST QUERY - does not compare dates yet!
	LONG lInboundMsgID;
	BINDCOL_LONG_NOLEN(query, lInboundMsgID);
	BINDPARAM_LONG(query, nDays);
	query.Execute(	_T("select 	ob.OutboundMessageID ")
					_T("from	OutboundMessages as ob, Tickets as ti ")
					_T("where	ob.TicketID = ti.TicketID ")
					_T("and		ti.LockedBy = 0 ")
					_T("and		ti.TicketStateID = 1 ")
					_T("and		ob.ArchiveID = 0" )
					_T("and		DATEDIFF(day, ob.EmailDateTime, getdate()) > ?"));
	while (query.Fetch() == S_OK)
	{
		// store off the message id for later use
		m_OutboundMsgIDs.push_back(lInboundMsgID);
	}
}

void CSerializer::SerializeOutboundMsgs(CODBCQuery &query, CSerialFile &sfile)
{
	query.Reset(true);

	// iterate and read/write
	for (	UINT_LIST::iterator cur(m_OutboundMsgIDs.begin());
			cur != m_OutboundMsgIDs.end();
			++cur)
	{
		// load our serializable message object
		CSerOutboundMessage serOutMsg;
		serOutMsg.LoadFromDB(*cur, query);
		serOutMsg.WriteToFile(sfile);
	}
}

void CSerializer::GetOutboundMessageAttachments(CODBCQuery &query)
{
	query.Reset(true);

	LONG lMsgAttID;

	// iterate and read/write
	for (	UINT_LIST::iterator cur(m_OutboundMsgIDs.begin());
			cur != m_OutboundMsgIDs.end();
			++cur)
	{
		BINDCOL_LONG_NOLEN(query, lMsgAttID);
		BINDPARAM_LONG(query, *cur);
		query.Execute(	_T("select	AttachmentID ")
						_T("from	OutboundMessageAttachments ")
						_T("where	OutboundMessageID = ?"));

		while (query.Fetch() == S_OK)
			m_OutboundMsgAttachIDs.push_back(lMsgAttID);

		query.Reset(true);
	}
}

void CSerializer::SerializeOutboundMsgAttachments(CODBCQuery &query, CSerialFile &sfile)
{
	tstring sFilePath;

	// iterate and read/write
	for (	UINT_LIST::iterator cur(m_OutboundMsgAttachIDs.begin());
			cur != m_OutboundMsgAttachIDs.end();
			++cur)
	{
		CSerOutboundMessageAttachment outa;
		outa.LoadFromDB(*cur, query);
		outa.WriteToFile(sfile, sFilePath);

		m_AttachPaths.push_back(sFilePath);
	}
}

int CSerializer::RestoreArchive(CODBCQuery& query, const UINT nArcID)
{
	//assert(nArcID > 0);

	int nRet = 0;
	tstring sTmp;
	int nRetArc = -1;

	try
	{
		query.Initialize();

		// get archive information
		CArchiveEntry arcEntry;
		arcEntry.LoadFromDB(query, nArcID);

		// inflate the archive file to temp location
		sTmp = GetTempFile();
		if (sTmp.size() == 0)
			return ERROR_PATH_NOT_FOUND;
		
		if (!InflateArchive(arcEntry.m_ArcFilePath, sTmp))
		{
			return -1;
		}

		// attach to the archive file
		CSerialFile serFile;
		nRetArc = serFile.OpenArchive(sTmp, CSerialFile::serializer_dir_in);
		if (nRetArc != 0)
			return nRetArc;
		
		RestoreInboundMsgs(query, serFile, arcEntry.m_InMsgRecords);

		RestoreInboundMsgAttachments(serFile, arcEntry.m_InAttRecords);

		RestoreOutboundMsgs(query, serFile, arcEntry.m_OutMsgRecords);

		RestoreOutboundMsgAttachments(serFile, arcEntry.m_OutAttRecords);
		
	}
#pragma warning (disable:4101)
	catch (CSerializeException sx)
	{
		//assert(0);
		nRet = sx.GetErrCode();
	}
	catch (CEMSException ex)
	{
		//assert(0);
		nRet = -1;
	}
	catch(...)
	{
		//assert(0);
		nRet = -1;
	}
#pragma warning (default:4101)

	if (nRetArc == 0)
		DeleteFile(sTmp.c_str());
	
	return nRet;
}

void CSerializer::DeleteAttachFiles()
{
	// iterate and delete
	for (	TSTRING_LIST::iterator cur(m_AttachPaths.begin());
			cur != m_AttachPaths.end();
			++cur)
	{
		DeleteFile((*cur).c_str());
	}
}

long CSerializer::AddArcRecord(CODBCQuery &query, const tstring& sArcPath, const BOOL bIsPurged)
{
	CArchiveEntry arcEntry;
	arcEntry.m_Purged = bIsPurged;
	GetTimeStamp(arcEntry.m_DateCreated);
	arcEntry.m_DateCreatedLen = sizeof(TIMESTAMP_STRUCT);
	_tcscpy(arcEntry.m_ArcFilePath, sArcPath.c_str());
	arcEntry.m_InAttRecords = m_InboundMsgAttachIDs.size();
	arcEntry.m_InMsgRecords = m_InboundMsgIDs.size();
	arcEntry.m_OutAttRecords = m_OutboundMsgAttachIDs.size();
	arcEntry.m_OutMsgRecords = m_OutboundMsgIDs.size();

	arcEntry.Insert(query);
	
	return query.GetLastInsertedID();
}

BOOL CSerializer::InflateArchive(const tstring &sSource, const tstring &sArcPath)
{
	// to do: better error checking
	CZipper zipper;
	int nRet = zipper.DecompressFile(sSource.c_str(), sArcPath.c_str());
	if (nRet == Z_OK)
		return TRUE; // success


	return FALSE;
}

void CSerializer::RestoreInboundMsgs(CODBCQuery& query, CSerialFile &sfile, const int nCount)
{
	for (int i=0; i<nCount; i++)
	{
		CSerInboundMessage inb;
		inb.ReadFromFile(sfile);
		inb.SaveToDB(query);
	}
}

void CSerializer::RestoreInboundMsgAttachments(CSerialFile &sfile, const int nCount)
{
	for (int i=0; i<nCount; i++)
	{
		CSerInboundMessageAttachment inba;
		inba.Restore(sfile);
	}
}

void CSerializer::RestoreOutboundMsgs(CODBCQuery &query, CSerialFile &sfile, const int nCount)
{
	for (int i=0; i<nCount; i++)
	{
		CSerOutboundMessage outb;
		outb.ReadFromFile(sfile);
		outb.SaveToDB(query);
	}
}

void CSerializer::RestoreOutboundMsgAttachments(CSerialFile &sfile, const int nCount)
{
	for (int i=0; i<nCount; i++)
	{
		CSerOutboundMessageAttachment outba;
		outba.Restore(sfile);
	}
}

// database records could be truncated or purged as they're exported into the
// archive file, but this approach is more "transactional"; only if the previous
// steps succeed will this routine be called
void CSerializer::DBCleanup(CODBCQuery &query, const BOOL bDoPurge, long lArcID)
{
	query.Reset(true);

	UINT_LIST::iterator cur;

	// clean up inbound msgs
	for (cur = m_InboundMsgIDs.begin(); cur != m_InboundMsgIDs.end(); ++cur)
	{
		// load our serializable message object
		CSerInboundMessage serInMsg;
		
		if (!bDoPurge)
		{
			serInMsg.LoadFromDB(*cur, query);
			serInMsg.SetArchived(query, lArcID);
		}
		else
			serInMsg.PurgeRecord(query, *cur);
	}

	// clean up inbound msg attachments
	for (cur = m_InboundMsgAttachIDs.begin(); cur != m_InboundMsgAttachIDs.end(); ++cur)
	{
		if (bDoPurge)
		{
			CSerInboundMessageAttachment serInAtt;
			serInAtt.PurgeRecord(query, *cur);
		}
	}

	// clean up outbound msgs
	for (cur = m_OutboundMsgIDs.begin(); cur != m_OutboundMsgIDs.end();	++cur)
	{
		// load our serializable message object
		CSerOutboundMessage serOutMsg;
		
		if (!bDoPurge)
		{
			serOutMsg.LoadFromDB(*cur, query);
			serOutMsg.SetArchived(query, lArcID);
		}
		else
			serOutMsg.PurgeRecord(query, *cur);
	}

	// clean up outbound msg attachments
	for (cur = m_OutboundMsgAttachIDs.begin(); cur != m_OutboundMsgAttachIDs.end(); ++cur)
	{
		if (bDoPurge)
		{
			CSerOutboundMessageAttachment outa;
			outa.PurgeRecord(query, *cur);
		}
	}
}
