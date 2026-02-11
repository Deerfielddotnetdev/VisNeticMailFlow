// InboundMessage.cpp: implementation of the CInboundMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InboundMessage.h"
#include "MailComponents.h"
#include "SingleCrit.h"
#include "ODBCConn.h"
#include "ODBCQuery.h"
#include "MessageIO.h"
#include "Utility.h"
#include "FileEncoder.h"
#include "MemMappedFile.h"
#include <mimepp/binhex.h>
#include <mimepp/applfile.h>
#include <mimepp/mimepp.h>
#include "KAVMsgScanner.h"
#include "RegistryFns.h"
#include "AttachFns.h"
#include <fstream>

//#define strcasecmp stricmp

extern dca::Mutex g_csDB;
extern CODBCConn g_odbcConn;
extern dca::Mutex g_csFileIO_1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInboundMessage::CInboundMessage() 
	:m_bPartial(FALSE)
{	
}

CInboundMessage::~CInboundMessage()
{

}

// the message saving process also combines AV scanning, as some
// message elements must be in various states (e.g. attachments must be
// extracted) in order to allow proper AV processing
const UINT CInboundMessage::Save(UINT nMessageSourceID, BOOL bUseReplyTo, UINT nZipAttach, BOOL bMsgIsDup)
{
	DebugReporter::Instance().DisplayMessage("CInboundMessage::Save - entering.", DebugReporter::MAIL);

	int nRet = 0;
	
	CKAVMsgScanner scanner;
	BOOL bDoScan = scanner.GetIsScanningEnabled();
	if (bDoScan)
	{
		nRet = scanner.ScanMsgBody(this);
		if (nRet != 0)
		{
			// logpoint: Error occurred while trying to virus-scan retrieved message (AV error: %d)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										nRet),
							EMS_STRING_MAILCOMP_AV_SCAN_ERROR,
							nRet);
			
			DebugReporter::Instance().DisplayMessage("CInboundMessage::Save - function failed when scanning file.", DebugReporter::MAIL);

			return nRet;
		}
	}

	// delete temp message storage; if this fails, just log it and keep going
	if( VerifyFileExists( m_sMsgFilePath.c_str() ))
	{
		tstring sFileName = m_sMsgFilePath.c_str();
		if (!DeleteFile(sFileName.c_str()))
		{
			// logpoint: Failed to delete temporary file (%s)
			CreateLogEntry(	EMSERROR(EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									ERROR_FILESYSTEM),
									EMS_STRING_MAILCOMP_ERR_DELETING_TEMPFILE,
									sFileName.c_str());

			assert(0);
		}	
	}
	

	m_sMsgFilePath.erase(m_sMsgFilePath.begin(),m_sMsgFilePath.end());

	
	DebugReporter::Instance().DisplayMessage("CInboundMessage::Save - get ready to save attachments.", DebugReporter::MAIL);

	// save attachment files
	nRet = SaveAttachmentFiles();
	if (nRet != 0)
	{
		// logpoint: Failed to save message attachment files (message will be downloaded again)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_MSG_ATTACHMENT_SAVE_FAILURE);

		// try to delete the message database record (to conserve space)
		DeleteMsgDBEntry();

		DebugReporter::Instance().DisplayMessage("CInboundMessage::Save - save attachments failed.", DebugReporter::MAIL);

		return ERROR_FILESYSTEM;
	}

	// resolve message body content attachment dependencies
	//ResolveContentDepends();

	LINETRACE(_T("+ CInboundMessage::Save - save message to database."));

	// save message record to database; note that initially the "IsDeleted"
	// flag is set to 2 (TRUE) so that a failure in a subsequent step
	// won't create an orphan message
	nRet = SaveMessageDB(nMessageSourceID, bUseReplyTo);
	if (nRet != 0)
	{
		// logpoint: Failed to save message to database (message will be downloaded again)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_MSG_DB_SAVE_FAILURE);

		LINETRACE(_T("- CInboundMessage::Save - save message to database."));

		return nRet;
	}

	LINETRACE(_T("+ CInboundMessage::Save - get ready to scan attachments"));

	// scan msg attachments if appropriate
	if (bDoScan)
	{
		nRet = scanner.ScanMsgAttachments(this);
		if (nRet != 0)
		{
			// logpoint: Error occurred while trying to virus-scan retrieved message (AV error: %d)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										nRet),
							EMS_STRING_MAILCOMP_AV_SCAN_ERROR,
							nRet);
			
			if (DeleteMsgDBEntry() != 0)
				assert(0);
			if (DeleteAttachmentFiles() != 0)
				assert(0);

			LINETRACE(_T("- CInboundMessage::Save - scan attachments failed."));

			return nRet;
		}
	}

	// if multiple attachments, add them to a zip
	UINT nNumAttach = m_Attachments.GetCount();
	if( (nNumAttach >= nZipAttach) && (nZipAttach != 0) )
	{
		LINETRACE(_T("+ CInboundMessage::Save - add attachments to zip file."));
		nRet = AddAttachmentsToZip();
		if (nRet != 0)
		{
			// logpoint: Failed to add attachments to zip
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_INBOUND_MESSAGING,
										nRet),
							EMS_STRING_MAILCOMP_MSG_ATTACHMENT_ZIP_FAILURE);

			LINETRACE(_T("+ CInboundMessage::Save - failed to add attachments to zip file."));			
		}
	}	
	
	LINETRACE(_T("+ CInboundMessage::Save - save attachments to database."));

	// create attachment database records
	nRet = SaveAttachmentsDB();
	if (nRet != 0)
	{
		// logpoint: Failed to save attachment database records (message will be downloaded again)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_ATTACHMENT_DB_SAVE_FAILURE);

		// try to delete orphan files and db records
		DeleteMsgDBEntry();
		DeleteAttachmentFiles();

		LINETRACE(_T("+ CInboundMessage::Save - failed save attachments to database."));

		return ERROR_FILESYSTEM;
	}


	if(!bMsgIsDup)
	{
		LINETRACE(_T("+ CInboundMessage::Save - queue the inbound message."));

		// queue the inbound message
		nRet = QueueInboundMsg();
		if (nRet != 0)
		{
			// logpoint: Failed to save message to database (message will be downloaded again)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_INBOUND_MESSAGING,
										nRet),
							EMS_STRING_MAILCOMP_MSG_DB_SAVE_FAILURE);

			LINETRACE(_T("- CInboundMessage::Save - queueing the inbound message failed."));

			return nRet;
		}
	}	

	LINETRACE(_T("+ CInboundMessage::Save - commit the save set the 'IsDeleted' flag to 0."));

	// commit the save (set the "IsDeleted" flag to 0 (FALSE))
	nRet = CommitSave();
	if (nRet != 0)
	{
		assert(0);

		// logpoint: Failed to commit message save in database (message will be downloaded again)
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_INBOUND_MESSAGING,
									nRet),
						EMS_STRING_MAILCOMP_MSG_DB_COMMIT_FAILURE);

		// try to delete orphan files and db records
		if (DeleteMsgDBEntry() != 0)
			assert(0);
		if (DeleteAttachmentFiles() != 0)
			assert(0);

		LINETRACE(_T("- CInboundMessage::Save - failed commit the save."));

		return nRet;
	}

	LINETRACE(_T("+ CInboundMessage::Save - send any virus notification alerts that may need to be sent."));

	// send any virus notification alerts that may need to be sent
	if (bDoScan)
	{
		nRet = scanner.SendInfectionAlert(this);
		if (nRet != 0)
		{
			USES_CONVERSION;

			// logpoint: Failed to send message virus infection notification to message originator (sender: %s)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_INBOUND_MESSAGING,
										nRet),
							EMS_STRING_MAILCOMP_AV_INFECTION_NOTIFICATION_FAILURE,
							A2T(this->GetOriginator().GetInetName().c_str()));

			LINETRACE(_T("+ CInboundMessage::Save - Failed to send message virus infection notification to message originator."));
		}
	}	
		
	DebugReporter::Instance().DisplayMessage("CInboundMessage::Save - leaving.", DebugReporter::MAIL);
	return nRet;
}

const UINT CInboundMessage::SaveMessageDB(UINT nMessageSourceID, BOOL bUseReplyTo, bool bIsError)
{
	USES_CONVERSION;
	int nRet = 0;

	LONG nMsgSourceID = nMessageSourceID;
	
	// get message received time (current time) in SQL time format
	SYSTEMTIME sysTime = m_ReceivedDate.GetSystemTime();
	TIMESTAMP_STRUCT sqlRecTime;
	m_ReceivedDate.SystemTimeToSQLTime(sysTime, sqlRecTime);
	LONG sqlRecTimeLen = 0;

	// this is the date that the message was SENT (by the sender)
	TIMESTAMP_STRUCT& sqlTime = m_Date.GetSQLTime();
	LONG sqlTimeLen = 0;

	// lock access to the database object
	dca::Lock lock(g_csDB);

	LINETRACE(_T("+ CInboundMessage::SaveMessageDB - tracking inbound message TicketID:InboundMessageID [%d:%d]."),GetMessageID(),GetTicketID());

	try
	{
		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		// from and from name
		tstring from;
		tstring fname;

		// Use the ReplyTo if it has only one EmailAddress
		if( bUseReplyTo && (m_ReplyTo.GetCount() == 1) )
		{
			// Assign the ReplyTo.InetName to from tstring
			from.assign(A2T(m_ReplyTo.Get(0).GetInetName().c_str()));

			// Assign the ReplyTo.GetFullname to from name tstring
			// First copy to temp string
			string sFullname(m_ReplyTo.Get(0).GetFullname());

			// Test to see if we have a from name
			if(sFullname.size())
			{
				// Clean up string to remove all the qoutes
				string::size_type pos = sFullname.find('\"');
				while(pos != string::npos)
				{
					sFullname.erase(pos,1);
					pos = sFullname.find('\"');
				}
				
				// Now assign
				fname.assign(A2T(sFullname.c_str()));
			}
			else
			{
				// Assign the Originator.GetFullname to from name tstring
				fname.assign(A2T(m_Originator.GetFullname().c_str()));
			}
		}
		else
		{
			// Assign the Originator.InetName to from tstring
			from.assign(A2T(m_Originator.GetInetName().c_str()));

			// Assign the Originator.GetFullname to from name tstring
			fname.assign(A2T(m_Originator.GetFullname().c_str()));
		}

		from.resize(INBOUNDMESSAGES_EMAILFROM_LENGTH-1, '\0');
		fname.resize(INBOUNDMESSAGES_EMAILFROMNAME_LENGTH-1, '\0');

		// readreceiptto
		tstring readreceiptto = A2T(m_ReadReceiptTo.GetInetName().c_str());
		readreceiptto.resize(INBOUNDMESSAGES_EMAILFROM_LENGTH-1, '\0');		
		int nReadReceipt = 0;
		if(readreceiptto.length() > 0)
			nReadReceipt = 1;

		// primary recipient
		tstring to = A2T(m_ReceivedFor.GetInetName().c_str());
		to.resize(INBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH-1, '\0');

		tstring sub = A2T(m_Subject.GetText().c_str());
		sub.resize(INBOUNDMESSAGES_SUBJECT_LENGTH-1, '\0');
		tstring mtype = A2T(m_MemoText.GetMediaType().c_str());
		mtype.resize(INBOUNDMESSAGES_MEDIATYPE_LENGTH-1, '\0');
		tstring mstype = A2T(m_MemoText.GetMediaSubType().c_str());
		mstype.resize(INBOUNDMESSAGES_MEDIASUBTYPE_LENGTH-1, '\0');

		BINDPARAM_LONG(query, m_nTicketID);
		BINDPARAM_WCHAR_STRING(query, from);
		BINDPARAM_WCHAR_STRING(query, fname);
		BINDPARAM_WCHAR_STRING(query, to);
		BINDPARAM_TIME(query, sqlTime);
		BINDPARAM_TIME(query, sqlRecTime);
		BINDPARAM_WCHAR_STRING(query, sub);
		BINDPARAM_WCHAR_STRING(query, mtype);
		BINDPARAM_WCHAR_STRING(query, mstype);
		BINDPARAM_LONG(query, nMsgSourceID);
		BINDPARAM_LONG(query, m_nMemoVirusScanState);
		BINDPARAM_LONG(query, m_nReplyToMsgID);
		BINDPARAM_TCHAR_STRING(query, m_sVirusName);

		tstring memo;
		memo.resize(m_MemoText.GetText().size());
		
		/*if(m_MemoText.GetCharset().compare("utf-7") == 0)
		{
			MultiByteToWideChar(CP_UTF7,
								0,
								m_MemoText.GetText().c_str(),
								m_MemoText.GetText().size(),
								(TCHAR*)memo.data(), memo.size());
		}
		else if(m_MemoText.GetCharset().compare("utf-8") == 0)
		{
			MultiByteToWideChar(CP_UTF8,
								0,
								m_MemoText.GetText().c_str(),
								m_MemoText.GetText().size(),
								(TCHAR*)memo.data(), memo.size());
		}
		else
		{*/
			MultiByteToWideChar(CP_ACP,
								0,
								m_MemoText.GetText().c_str(),
								m_MemoText.GetText().size(),
								(TCHAR*)memo.data(), memo.size());
		//}
		   
		/*if ( strcasecmp(m_MemoText.GetCharset().c_str(), "iso-8859-1") != 0 )
		{
			UINT nCodepage = CMessage::GetCodePage( m_MemoText.GetCharset() );
			int Len = m_MemoText.GetText().size();
			int nWcsSize = MultiByteToWideChar( nCodepage, 0, m_MemoText.GetText().c_str(), Len, NULL, 0  );
			memo.resize( nWcsSize, L' ' );
			MultiByteToWideChar( nCodepage, 0, m_MemoText.GetText().c_str(), Len, wszNewServer2, sizeof(wszNewServer2) / sizeof(wchar_t)  );
			memo.assign(wszNewServer2,nWcsSize);						
		}
		else
		{
			MultiByteToWideChar(CP_ACP,
								0,
								m_MemoText.GetText().c_str(),
								m_MemoText.GetText().size(),
								(TCHAR*)memo.data(), memo.size());
		}*/

		if( (m_nUuencodeInText != 0) && (m_nUuencodeStartPos != DwString::npos) )
		{
			memo.erase(m_nUuencodeStartPos);
			m_nUuencodeInText = 0;
			m_nUuencodeStartPos = DwString::npos;
		}

		tstring pheaders;
		pheaders.resize(m_PopHeaders.GetText().size());
		MultiByteToWideChar(CP_ACP, 
							0, 
							m_PopHeaders.GetText().c_str(), 
							m_PopHeaders.GetText().size(),
							(TCHAR*)pheaders.data(),
							pheaders.size());
		
		tstring tolist = GetToListString().c_str();
		tstring cclist = GetCcListString().c_str();
		tstring rlist = GetReplyToListString();
		
		BINDPARAM_TEXT_STRING(query, memo);
		BINDPARAM_TEXT_STRING(query, pheaders);
		BINDPARAM_TEXT_STRING(query, tolist);
		BINDPARAM_TEXT_STRING(query, cclist);
		BINDPARAM_TEXT_STRING(query, rlist);
		
		BINDPARAM_LONG(query, m_lPriorityID);
		BINDPARAM_WCHAR_STRING(query, readreceiptto);
		BINDPARAM_TINYINT(query, nReadReceipt);		

		query.Execute(	_T("INSERT INTO InboundMessages ")
						_T("(TicketID, EmailFrom, EmailFromName, EmailPrimaryTo, EmailDateTime, DateReceived, ")
						_T("Subject, MediaType, MediaSubType, IsDeleted, MessageSourceID, VirusScanStateID, ReplyToMsgID, VirusName, ")
						_T("Body, PopHeaders, EmailTo, EmailCc, EmailReplyTo, PriorityID, ReadReceiptTo, ReadReceipt) ")
						_T("VALUES ")
						_T("(?, ?, ?, ?, ?, ?, ?, ?, ?, 2, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

		LINETRACE(_T("+ CInboundMessage::SaveMessageDB - store the generated message id"));

		// store the generated message id
		m_lMessageID = query.GetLastInsertedID();

		// set the inbound message tracking data		
		tstring msgid = A2T(m_MsgID.GetText().c_str());
		msgid.resize(INBOUNDMESSAGES_SUBJECT_LENGTH-1, '\0');
		tstring inreplyto = A2T(m_InReplyTo.GetText().c_str());
		inreplyto.resize(INBOUNDMESSAGES_SUBJECT_LENGTH-1, '\0');
		tstring references = A2T(m_References.GetText().c_str());
		references.resize(INBOUNDMESSAGES_SUBJECT_LENGTH-1, '\0');
		
		query.Initialize();
		BINDPARAM_LONG(query, m_lMessageID);
		BINDPARAM_LONG(query, nMsgSourceID);
		BINDPARAM_WCHAR_STRING(query, msgid);
		BINDPARAM_WCHAR_STRING(query, inreplyto);
		BINDPARAM_WCHAR_STRING(query, references);
		query.Execute(	_T("INSERT INTO MessageTracking ")
						_T("(MessageID, MessageSourceID, HeadMsgID, HeadInReplyTo, HeadReferences) ")
						_T("VALUES ")
						_T("(?, ?, ?, ?, ?)"));

		LINETRACE(_T("+ CInboundMessage::SaveMessageDB - tracking inbound message TicketID:InboundMessageID [%d:%d]."),GetMessageID(),GetTicketID());

		if(bIsError)
		{
			query.Initialize();
			BINDPARAM_LONG(query, m_nTicketID);
			query.Execute( _T("UPDATE Tickets SET TicketStateID = 2 WHERE TicketID = ?"));
		}

	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		nRet = ERROR_DATABASE;
	}

	return nRet;
}


const UINT CInboundMessage::SaveAttachmentsDB()
{
	USES_CONVERSION;
	TCHAR dir[_MAX_DIR];
	TCHAR file[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	// lock access to the database object
	dca::Lock lock(g_csDB);

	// init query object
	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();
		
		// save the attachments to the database
		int nCount = m_Attachments.GetCount();
		for (int i=0; i<nCount; i++)
		{
			CAttachment* pAtt = m_Attachments.GetAttachment(i);
			assert(pAtt);

			

			tstring storedname = pAtt->GetFullpath();			
			storedname.resize(ATTACHMENTS_ATTACHMENTLOCATION_LENGTH-1, '\0');
			_tsplitpath(storedname.c_str(), NULL, dir, file, ext);

			{
				tstring szFolder;

				if(pAtt->GetVirusScanState() == EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_DELETED)
				{
					return 0;
				}
				else if(pAtt->GetVirusScanState() == EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED)
				{
					if(!GetRegString(EMS_ROOT_KEY, REG_KEY_AV, tstring(_T("QuarantineFolder")).c_str(), szFolder))
					{
						// generate an absolute path using the provided filename
						if(szFolder.length() > 0 && szFolder.at(szFolder.size()-1) != '\\')
							szFolder += '\\';

						tstring sDir(dir);
						szFolder = szFolder.erase(0,2);

						storedname = sDir.substr(szFolder.size());

						storedname += (tstring)file + (tstring)ext;
					}
					else
						storedname = (tstring)file + (tstring)ext;
				}
				else
				{
					if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("InboundAttachFolder")).c_str(), szFolder))
					{
						// generate an absolute path using the provided filename
						if(szFolder.length() > 0 && szFolder.at(szFolder.size()-1) != '\\')
							szFolder += '\\';

						//tstring sDir(dir);
						//szFolder = szFolder.erase(0,2);

						//storedname = sDir.substr(szFolder.size());
						storedname = storedname.substr(szFolder.length(),storedname.length());

						//storedname += (tstring)file + (tstring)ext;
					}
					else
						storedname = (tstring)file + (tstring)ext;
				}
			}

			// this is the original attachment filename retrieved with the message
			tstring fname = A2T(pAtt->GetFileName().c_str());
			fname.resize(ATTACHMENTS_FILENAME_LENGTH-1, '\0');
			
			tstring mtype = A2T(pAtt->GetType().c_str());
			mtype.resize(ATTACHMENTS_MEDIATYPE_LENGTH-1, '\0');

			tstring mstype = A2T(pAtt->GetSubType().c_str());
			mstype.resize(ATTACHMENTS_MEDIASUBTYPE_LENGTH-1, '\0');

			tstring cdisp = pAtt->GetContentDisposition();
			cdisp.resize(ATTACHMENTS_CONTENTDISPOSITION_LENGTH-1, '\0');

			tstring cid = A2T(pAtt->GetContentID().c_str());
			cid.resize(ATTACHMENTS_CONTENTID_LENGTH-1, '\0');

			LONG filesize = pAtt->GetSize();
			if(filesize == 0)
			{
				dca::String sFullPath(pAtt->GetFullpath().c_str());
				const char *cstr3 = sFullPath.c_str();				
				std::ifstream in(cstr3, std::ifstream::in | std::ifstream::binary);
				in.seekg(0, std::ifstream::end);
				filesize = in.tellg();		
			}

			LONG nVirusScanStateID = pAtt->GetVirusScanState();

			dca::WString b;
			b.Format(L"CInboundMessage::SaveAttachmentFiles - INSERT INTO Attachments (AttachmentLocation, FileSize, VirusScanStateID, VirusName, MediaType, MediaSubType, ContentDisposition, FileName, ContentID, IsInbound) VALUES (%s, %d, %d, %s, %s, %s, %s, %s, %s, 1)"
				,storedname.c_str()
				,filesize
				,nVirusScanStateID
				,pAtt->GetVirusName().c_str()
				,mtype.c_str()
				,mstype.c_str()
				,cdisp.c_str()
				,fname.c_str()
				,cid.c_str());
			dca::String h(b.c_str());
			DebugReporter::Instance().DisplayMessage(h.c_str(), DebugReporter::MAIL);
			
			BINDPARAM_WCHAR_STRING(query, storedname);
			BINDPARAM_LONG(query, filesize);
			BINDPARAM_LONG(query, nVirusScanStateID);
			BINDPARAM_TCHAR_STRING(query, pAtt->GetVirusName());
			BINDPARAM_WCHAR_STRING(query, mtype);
			BINDPARAM_WCHAR_STRING(query, mstype);
			BINDPARAM_WCHAR_STRING(query, cdisp);
			BINDPARAM_WCHAR_STRING(query, fname);
			BINDPARAM_WCHAR_STRING(query, cid);

			query.Execute(	_T("INSERT INTO Attachments ")
							_T("(AttachmentLocation, FileSize, VirusScanStateID, VirusName, MediaType, MediaSubType, ContentDisposition, FileName, ContentID, IsInbound) ")
							_T("VALUES" )
							_T("(?, ?, ?, ?, ?, ?, ?, ?, ?, 1)"));

			int newID = query.GetLastInsertedID();

			dca::String f;
			f.Format("CInboundMessage::SaveAttachmentFiles - INSERT INTO InboundMessageAttachments (InboundMessageID, AttachmentID) VALUES (%d,%d)",  m_lMessageID, newID);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

			query.Reset(true);
			BINDPARAM_LONG(query, m_lMessageID);
			BINDPARAM_LONG(query, newID);
			query.Execute(	_T("INSERT INTO InboundMessageAttachments ")
							_T("(InboundMessageID, AttachmentID) ")
							_T("VALUES" )
							_T("(?,?)"));

			// store the attachment database id
			pAtt->SetAttachmentID(query.GetLastInsertedID());
			
			query.Reset(true);
				
		}
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	catch(...)
	{
		return ERROR_UNKNOWN_ERROR;
	}

	return 0;
}

const UINT CInboundMessage::SaveAttachmentFiles()
{
	USES_CONVERSION;
	TCHAR dir[_MAX_DIR];
	TCHAR drive[_MAX_DRIVE];

	CMessageIO msgIO;

	dca::Lock lock(g_csFileIO_1);

	
	{
		dca::String f;
		f.Format("CInboundMessage::SaveAttachmentFiles - save attachment files only if count is greater than 0.  Count = %d", m_Attachments.GetCount());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	}

	// save the attachment files
	int nCount = m_Attachments.GetCount();
	for (int i=0; i<nCount; i++)
	{
		// compute and store attachment path
		CAttachment* pAtt = m_Attachments.GetAttachment(i);

		{
			dca::String f;
			f.Format("CInboundMessage::SaveAttachmentFiles - compute and store attachment path for %s", pAtt->GetFileName().c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		}

		// Just the filename at this point
		tstring sAttach(A2T(pAtt->GetFileName().c_str()));
		
		// Make a copy for later
		tstring sAttachName;
		sAttachName.assign(sAttach);

		//Strip the attachment name of any illegal characters

		if (!msgIO.GetInboundAttachPath(sAttach, 1))
		{
			// An error occurred saving attachment (%s) - [Unable to obtain unique filename]
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_INBOUND_MESSAGING,
										ERROR_FILESYSTEM),
							EMS_STRING_ERROR_ATTACH_PATH,
							pAtt->GetFullpath().c_str());

			return ERROR_FILESYSTEM;
		}
		// The full actual path including filename here
		pAtt->SetFullPath(sAttach.c_str());

		// Make a copy for later
		tstring sAttachCopy;
		sAttachCopy.assign(sAttach);

		// ensure that the attachment directory path exists
		_tsplitpath(sAttach.c_str(), drive, dir, NULL, NULL);

		{
			dca::String f;
			dca::String a(sAttach.c_str());
			f.Format("CInboundMessage::SaveAttachmentFiles -  ensure that the attachment directory path exists (1) %s", a.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		}

		if (!CUtility::EnsurePathExists(tstring(drive) + tstring(dir)))
		{
			DebugReporter::Instance().DisplayMessage("CInboundMessage::SaveAttachmentFiles - ensure path exists failed (1)", DebugReporter::MAIL);
			break;
		}
		
		// Make a copy of the full path before we modify it
		tstring sFullPath;
		sFullPath.assign(sAttach);

		// Add \\?\UNC or \\?
		if(sAttach.at(0) == '\\' && sAttach.at(1) == '\\' )
		{
			sAttach = sAttach.erase(0,1);
			sAttach.insert(0,_T("\\\\?\\UNC"));
		}
		else
		{
			sAttach.insert(0,_T("\\\\?\\"));
		}
			
		{
			dca::String f;
			dca::String a(sAttach.c_str());
			f.Format("CInboundMessage::SaveAttachmentFiles - attach filename (1) = %s",a.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		}

		// create the attachment file
		HANDLE hFile = CreateFile(sAttach.c_str(), 
								  GENERIC_WRITE,			// generic write ops
								   0,						// no sharing
								   NULL,					// no security
								   CREATE_NEW,				// fails if file already exists
								   FILE_ATTRIBUTE_NORMAL,	// normal file
								   NULL);					// no attr. template
		
		if (hFile == INVALID_HANDLE_VALUE) 
		{
			{
				dca::String f;
				dca::String a(sAttach.c_str());
				f.Format("CInboundMessage::SaveAttachmentFiles - create file (1) failed for %s",a.c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			}
			
			//Wait a second and try to create the file again
			Sleep(1000);
			hFile = CreateFile(sAttach.c_str(), 
								  GENERIC_WRITE,			// generic write ops
								   0,						// no sharing
								   NULL,					// no security
								   CREATE_NEW,				// opens if file already exists, creates if not
								   FILE_ATTRIBUTE_NORMAL,	// normal file
								   NULL);					// no attr. template
			
			if (hFile == INVALID_HANDLE_VALUE) 
			{
				
				{
					dca::String f;
					dca::String a(sAttach.c_str());
					f.Format("CInboundMessage::SaveAttachmentFiles - create file (2) failed for %s",a.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				}

				// Ok it failed again, let's try to rename the attachment and try a couple more times
				// Just the filename at this point
				tstring sAttach2(A2T(pAtt->GetFileName().c_str()));
				if(sAttach2.length() > 12)
				{
					// Let's grab the last 12 for the name
					sAttach2.erase(0,sAttach2.length() - 12);
				}
				
				// Make a copy for later
				tstring sAttach3;
				sAttach3.assign(sAttach2);
				
				if (!msgIO.GetInboundAttachPath(sAttach2, 1))
				{
					// An error occurred saving attachment (%s) - [Unable to obtain unique filename]
					CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
												EMSERR_MAIL_COMPONENTS,
												EMS_LOG_INBOUND_MESSAGING,
												ERROR_FILESYSTEM),
									EMS_STRING_ERROR_ATTACH_PATH,
									pAtt->GetFullpath().c_str());

					return ERROR_FILESYSTEM;
				}
				
				// ensure that the attachment directory path exists
				_tsplitpath(sAttach2.c_str(), drive, dir, NULL, NULL);

				{
					dca::String f;
					dca::String a(sAttach2.c_str());
					f.Format("CInboundMessage::SaveAttachmentFiles -  ensure that the attachment directory path exists (2) %s", a.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				}

				if (!CUtility::EnsurePathExists(tstring(drive) + tstring(dir)))
				{
					DebugReporter::Instance().DisplayMessage("CInboundMessage::SaveAttachmentFiles - ensure path exists failed (2)", DebugReporter::MAIL);
					break;
				}
				// Make a copy of the full path before we modify it
				tstring sFullPath2;
				sFullPath2.assign(sAttach2);

				// Add \\?\UNC or \\?
				if(sAttach2.at(0) == '\\' && sAttach2.at(1) == '\\' )
				{
					sAttach2 = sAttach2.erase(0,1);
					sAttach2.insert(0,_T("\\\\?\\UNC"));
				}
				else
				{
					sAttach2.insert(0,_T("\\\\?\\"));
				}
					
				{
					dca::String f;
					dca::String a(sAttach2.c_str());
					f.Format("CInboundMessage::SaveAttachmentFiles - attach filename (3) = %s",a.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				}
				
				// create the attachment file
				hFile = CreateFile(sAttach2.c_str(), 
										GENERIC_WRITE,			// generic write ops
										0,						// no sharing
										NULL,					// no security
										CREATE_NEW,				// fails if file already exists
										FILE_ATTRIBUTE_NORMAL,	// normal file
										NULL);					// no attr. template
				
				if (hFile == INVALID_HANDLE_VALUE) 
				{
				
					{
						dca::String f;
						dca::String a(sAttach2.c_str());
						f.Format("CInboundMessage::SaveAttachmentFiles - create file (3) failed for %s",a.c_str());
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					}

				
					//Wait a second and try to create the file again
					Sleep(1000);
					hFile = CreateFile(sAttach2.c_str(), 
										GENERIC_WRITE,			// generic write ops
										0,						// no sharing
										NULL,					// no security
										CREATE_NEW,				// opens if file already exists, creates if not
										FILE_ATTRIBUTE_NORMAL,	// normal file
										NULL);					// no attr. template
					
					if (hFile == INVALID_HANDLE_VALUE) 
					{
						{
							dca::String f;
							dca::String a(sAttach2.c_str());
							f.Format("CInboundMessage::SaveAttachmentFiles - create file (4) failed for %s",a.c_str());
							DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						}
						
						//Wait a second and try to create the file for the final time
						Sleep(1000);
						hFile = CreateFile(sAttach2.c_str(), 
											GENERIC_WRITE,			// generic write ops
											0,						// no sharing
											NULL,					// no security
											CREATE_NEW,				// opens if file already exists, creates if not
											FILE_ATTRIBUTE_NORMAL,	// normal file
											NULL);					// no attr. template
						
						if (hFile == INVALID_HANDLE_VALUE) 
						{
							{
								dca::String f;
								dca::String a(sAttach2.c_str());
								f.Format("CInboundMessage::SaveAttachmentFiles - create file (5) failed for %s",a.c_str());
								DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
							}
							
							tstring sub = A2T(m_Subject.GetText().c_str());
							sub.resize(INBOUNDMESSAGES_SUBJECT_LENGTH-1, '\0');

							tstring sErrorText;
							sErrorText.assign(sAttachName);
							sErrorText.append(L" - Retry as: ");
							sErrorText.append(sAttach3);
							sErrorText.append(L" failed also - Subject: ");
							sErrorText.append(sub);
							
							// An error occurred saving attachment (%s) - [CreateFile() failed error (%d)]
							DWORD dwErr = GetLastError();
							CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
														EMSERR_MAIL_COMPONENTS,
														EMS_LOG_INBOUND_MESSAGING,
														ERROR_FILESYSTEM),
											EMS_STRING_ERROR_ATTACH_CREATEFILE,
											sErrorText.c_str(),
											dwErr);									

							CreateAlert( EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_ERROR_ATTACH_CREATEFILE),sErrorText.c_str(),dwErr);
							return ERROR_FILESYSTEM;

						}
					}
				}
				



				if (hFile != INVALID_HANDLE_VALUE)
				{
					// Convert and set the attachment filename since the rename succeeded
					string sFileName(T2A(sAttach3.c_str()));
					pAtt->SetFileName(sFileName);
					// Set the full path the rename succeeded
					pAtt->SetFullPath(sFullPath2.c_str());

					tstring sub = A2T(m_Subject.GetText().c_str());
					sub.resize(INBOUNDMESSAGES_SUBJECT_LENGTH-1, '\0');

					tstring sErrorText2;
					sErrorText2.assign(sAttachName);
					sErrorText2.append(L" - Retry as: ");
					sErrorText2.append(sAttach3);
					sErrorText2.append(L" was successful - Subject: ");
					sErrorText2.append(sub);

					DWORD dwErr = GetLastError();
					CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
												EMSERR_MAIL_COMPONENTS,
												EMS_LOG_INBOUND_MESSAGING,
												ERROR_FILESYSTEM),
									EMS_STRING_ERROR_ATTACH_CREATEFILE,
									sErrorText2.c_str(),
									dwErr);

					CreateAlert( EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_ERROR_ATTACH_CREATEFILE),sErrorText2.c_str(),dwErr);

				}
			}
		}

		{
			dca::String f;
			dca::String a(sAttach.c_str());
			f.Format("CInboundMessage::SaveAttachmentFiles - create file succeeded for %s",a.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		}

		DWORD dwBytesWritten;
		//write the file
		
		BOOL bRet = WriteFile(hFile, pAtt->GetContent().c_str(), pAtt->GetSize(), &dwBytesWritten, NULL);
		if (!bRet)
		{
			
			{
				dca::String f;
				dca::String a(sAttach.c_str());
				f.Format("CInboundMessage::SaveAttachmentFiles - write file failed (1) for %s",a.c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			}
			
			//Wait a second and try again
			Sleep(1000);
			bRet = WriteFile(hFile, pAtt->GetContent().c_str(), pAtt->GetSize(), &dwBytesWritten, NULL);
			if (!bRet)
			{
				{
					dca::String f;
					dca::String a(sAttach.c_str());
					f.Format("CInboundMessage::SaveAttachmentFiles - write file failed (2) for %s",a.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				}
				
				DWORD dwErr = GetLastError();

				// try to clean up
				CloseHandle(hFile);
				DeleteFile(pAtt->GetFullpath().c_str());

				// An error occurred saving attachment (%s) - (WriteFile() failed error (%d)]
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
											EMSERR_MAIL_COMPONENTS,
											EMS_LOG_INBOUND_MESSAGING,
											ERROR_FILESYSTEM),
								EMS_STRING_ERROR_ATTACH_WRITEFILE,
								pAtt->GetFullpath().c_str(),
								dwErr);

				return ERROR_FILESYSTEM;
			}
		}

		assert(dwBytesWritten == pAtt->GetContent().size());
		
		FlushFileBuffers(hFile);
		CloseHandle(hFile);
	}

	return 0; // success
}

const UINT CInboundMessage::AddAttachmentsToZip()
{
	USES_CONVERSION;
	tstring sSourceZip;		
	dca::String sSaveTarget;
	tstring sTargetZip;
	TCHAR dir[_MAX_DIR];
	TCHAR file[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	try
	{
		int nCount = m_Attachments.GetCount();
		if(nCount > 1)
		{
			CkZip zip;
			bool success;
			tstring szAttachFolder;
			tstring tmpFile;
			tstring tmpFolder;

			CkGlobal m_glob;
			// unlock Chilkat
			success = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
			if (success != true)
			{
				DebugReporter::Instance().DisplayMessage("CInboundMessage::AddAttachmentsToZip() - Failed to unlock Global component", DebugReporter::MAIL);
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
											EMSERR_MAIL_COMPONENTS,
											EMS_LOG_INBOUND_MESSAGING,
											ERROR_NO_ERROR),
								EMS_STRING_ERROR_INITIALIZE_COMPONENT);

				return ERROR_UNKNOWN_ERROR;
			}

			if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("InboundAttachFolder")).c_str(), szAttachFolder))
			{
				if(szAttachFolder.length() > 0 && szAttachFolder.at(szAttachFolder.size()-1) != '\\'){szAttachFolder += '\\';}
			}
			else
			{
				return ERROR_UNKNOWN_ERROR;
			}
			
			CMessageIO msgIO;
			CEMSString sZipName;
			sZipName.Format(_T("attach%d.zip"), m_lMessageID);
			sTargetZip.assign(sZipName.c_str());
			if (!msgIO.GetInboundAttachPath(sTargetZip, 1))
			{
				return 1;
			}
			
			sSaveTarget.Format("attach%d.zip", m_lMessageID);
			
			dca::String sZip(sTargetZip.c_str());
			const char *cstr = sZip.c_str();

			// sTargetZip is tstring full path to target zip
			// sZip is dca::String full path to target zip
			// cstr is const char full path to target zip
			// sSaveTarget is dca::String target zip file name

			for (int i=0; i<nCount; i++)
			{
				CAttachment* pAtt = m_Attachments.GetAttachment(i);
				assert(pAtt);
				
				//get the full filename from the attachment
				tstring filename = pAtt->GetFullpath();			
				filename.resize(ATTACHMENTS_ATTACHMENTLOCATION_LENGTH-1, '\0');					
				_tsplitpath(filename.c_str(), NULL, dir, file, ext);
				tstring storedFileName = (tstring)file + (tstring)ext;
				
				tstring fname = A2T(pAtt->GetFileName().c_str());
				fname.resize(ATTACHMENTS_FILENAME_LENGTH-1, '\0');

				if(i==0)
				{
					success = zip.NewZip(cstr);
				}
				else
				{
					success = zip.NewZip("temp.zip");
				}
				if (success != true)
				{
					return 1;
				}

				dca::String sTemp2(filename.c_str());
				const char *cstr2 = sTemp2.c_str();
				success = zip.AppendOneFileOrDir(cstr2,false);				

				if (success != true)
				{
					return 1;
				}
				else
				{
					//existing name					
					dca::String sTemp4(storedFileName.c_str());
					const char *cstr4 = sTemp4.c_str();

					//new name
					dca::String sTemp3(fname.c_str());
					const char *cstr3 = sTemp3.c_str();

					CkZipEntry *entry = 0;
					entry = zip.GetEntryByName(cstr4);					
					entry->put_FileName(cstr3);
					delete entry;										
				}

				if(i==0)
				{
					success = zip.WriteZipAndClose();
				}
				else
				{
					success = zip.QuickAppend(cstr);
				}
				if (success != true) {
					return 1;
				}						
			}
			
		}

		CAttachment attach;
		attach.SetFileName(sSaveTarget);
		attach.SetFullPath(sTargetZip);
		attach.SetMediaType("application","pdf");
		tstring sDisposition(_T("attachment"));
		attach.SetContentDisposition(sDisposition);

		m_Attachments.AddAttachment(attach);
	}	
	catch(...)
	{
		return ERROR_UNKNOWN_ERROR;
	}

	return 0;
}

// commits the saving of a message by flagging it as no longer
// being 'deleted'
const UINT CInboundMessage::CommitSave()
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();
		BINDPARAM_LONG(query, m_lMessageID);
		query.Execute(	_T("UPDATE InboundMessages SET IsDeleted = 0 WHERE InboundMessageID = ?"));	
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	
	return 0; // success
}

// delete all attachment files relating to this inbound
// message - typically used as part of save transaction rollback
const UINT CInboundMessage::DeleteAttachmentFiles()
{
	int nRet = 0;

	int nCount = m_Attachments.GetCount();
	for (int i=0; i<nCount; i++)
	{
		CAttachment* pAtt = m_Attachments.GetAttachment(i);
		assert(pAtt->GetFullpath().size() > 0);

		if (!DeleteFile(pAtt->GetFullpath().c_str()))
		{
			// our attempt to delete the attachment file has failed; keep trying to
			// delete any others, but return failure at function's endpoint
			LINETRACE(_T("Unable to delete attachment file: %s (code %d)\n"), pAtt->GetFullpath().c_str(), GetLastError());
			assert(0);
			nRet = ERROR_FILESYSTEM;
			continue;
		}
	}

	return nRet;
}

// this function attempts to delete message database records
// relating to a specific message id - typically used as part
// of a message save transaction rollback
const int CInboundMessage::DeleteMsgDBEntry()
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();
		
		BINDPARAM_LONG(query, m_lMessageID);

		query.Execute(	_T("DELETE FROM InboundMessageQueue ")
						_T("WHERE InboundMessageID = ?"));

		query.Reset(false);

		query.Execute(	_T("DELETE FROM InboundMessages ")
						_T("WHERE InboundMessageID = ?"));

	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}

// queuing and inbound message effectively tells the routing engine
// to process the message
const UINT CInboundMessage::QueueInboundMsg()
{
	int nRet = 0;

	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		// now insert a record into the inbound message queue
		query.Reset(true);
		BINDPARAM_LONG(query, m_lMessageID);
		query.Execute(	_T("INSERT INTO InboundMessageQueue ")
						_T("(InboundMessageID) ")
						_T("VALUES ")
						_T("(?)"));
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		nRet = ERROR_DATABASE;
	}

	return nRet;
}
