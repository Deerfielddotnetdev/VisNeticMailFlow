// POP3Connection.cpp: implementation of the CPOP3Connection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "POP3Connection.h"
#include "MailComponents.h"
#include "ODBCConn.h"
#include "ODBCQuery.h"
#include "SingleCrit.h"
#include "MemMappedFile.h"
#include "SourceFileStringRep.h"
#include "Utility.h"
#include "DateFns.h"
#include "TempFile.h"
#include "ServerParameters.h"
#include "QueryClasses.h"
#include "EmailDate.h"
#include <vector>
#include <mimepp/mimepp.h>

extern dca::Mutex g_csDB;
extern CODBCConn g_odbcConn;

const int nLongTimeoutSecs = (60 * 5); // long timeout is 5 mins

/////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPOP3Connection::CPOP3Connection()
{
	m_pPopAccount = NULL;
	m_bServerSupportsUIDL = true;
	popBundle = 0;
}

CPOP3Connection::~CPOP3Connection()
{
	ClearMsgInfo();
	if (m_mailman.get_IsPop3Connected())
	{
		m_mailman.Pop3EndSession();
	}
	if (m_imap.IsConnected())
	{
		m_imap.Disconnect();
	}
	m_imap.dispose();
	m_mailman.dispose();
	dca::String f;
	f.Format("CPOP3Connection::~CPOP3Connection() - Disposing POP3Conn for MessageSourceID: %d", m_pPopAccount->GetMessageSourceID());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
}

const int CPOP3Connection::CheckAccount()
{
	if (!m_pPopAccount)
		return ERROR_BAD_POP_ACCOUNT;

	int nRet = 0;
	int nType = 0;
	int nCount = 0;
	int i = 0;
	CkStringArray* uidlList;
	bool bRet;
	long maxWaitMillisec = m_pPopAccount->GetConnTimeoutSecs() * 1000;
	USES_CONVERSION;

	{
		dca::String f;
		dca::String d(m_pPopAccount->GetDescription().c_str());
		f.Format("CPOP3Connection::CheckAccount - Checking POP3 account (message source: %s)", d.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	}
	// logpoint: Checking POP3 account (server: %s, account: %s)
	CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_INFORMATIONAL,
		EMSERR_MAIL_COMPONENTS,
		EMS_LOG_INBOUND_MESSAGING,
		ERROR_NO_ERROR),
		EMS_STRING_MAILCOMP_CHECKING_POP3,
		m_pPopAccount->GetDescription().c_str());

	// set the "lastchecked" field in the database first
	nRet = SetLastChecked();
	if (nRet != 0)
	{
		dca::String f;
		dca::String d(m_pPopAccount->GetDescription().c_str());
		f.Format("CPOP3Connection::CheckAccount - Failed to set last-processed value for (message source: %s)", d.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		// logpoint: Failed to set last-processed value for message source (server: %s, account: %s)
		CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
			EMSERR_MAIL_COMPONENTS,
			EMS_LOG_INBOUND_MESSAGING,
			nRet),
			EMS_STRING_MAILCOMP_SETLASTCHECKED_FAILURE,
			m_pPopAccount->GetDescription().c_str());

		CreateAlert(EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_MAILCOMP_SETLASTCHECKED_FAILURE), m_pPopAccount->GetDescription().c_str());

		goto cleanup;
	}

	// get the account type POP3=1 IMAP=2
	nType = m_pPopAccount->GetMessageSourceTypeID();

	// Unlock Chilkat
	bRet = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
	if (bRet != true)
	{
		DebugReporter::Instance().DisplayMessage("Incorrect Chilkat License / Version", DebugReporter::MAIL);
		CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
			EMSERR_MAIL_COMPONENTS,
			EMS_LOG_INBOUND_MESSAGING,
			nRet),
			EMS_STRING_ERROR_INITIALIZE_COMPONENT);

		if (m_pPopAccount->GetErrorCode() == 0)
		{
			CreateAlert(EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_ERROR_INITIALIZE_COMPONENT));
			SetErrorCode(1);
		}
		nRet = 1;
		goto cleanup;
	}

	if (nType == 1)
	{
		m_mailman.put_ImmediateDelete(false);

		if (m_pPopAccount->GetIsSSL() == TRUE)
		{
			nRet = InitSecureConnection();
		}
		else
		{
			nRet = InitConnection();
		}

		if (nRet != 0)
		{
			dca::String f;
			f.Format("CPOP3Connection::CheckAccount - Failed to initialize connection to POP3 server (server: %s)", m_pPopAccount->GetDescription().c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_FAILED_SERVER_CONNECT,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetPort());

			if (m_pPopAccount->GetErrorCode() == 0)
			{
				CreateAlert(EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_MAILCOMP_FAILED_SERVER_CONNECT),
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetPort());

				SetErrorCode(1);
			}
			goto cleanup;
		}
	}
	else if (nType == 2)
	{
		// connect to IMAP server

		const char* sslServerHost = T2A(m_pPopAccount->GetServerAddress().c_str());

		bool bSsl = false;
		if (m_pPopAccount->GetIsSSL() == TRUE)
		{
			bSsl = true;
			//m_imap.put_SslProtocol("TLS 1.0 or higher");
			//m_imap.put_SslAllowedCiphers("best-practices");
		}
		int nPort = m_pPopAccount->GetPort();
		m_imap.put_Ssl(bool(bSsl));
		m_imap.put_StartTls(false);
		m_imap.put_Port(nPort);
		m_imap.put_ConnectTimeout(maxWaitMillisec);

		bRet = m_imap.Connect(sslServerHost);
		if (bRet != true)
		{
			CkString sError;
			m_imap.get_LastErrorText(sError);
			dca::String r(sError.getString());
			dca::String f;
			f.Format("CPOP3Connection::CheckAccount - Failed to initialize connection to IMAP server (server: %s) with error (%s)", sslServerHost, r.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_FAILED_SERVER_CONNECT,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetPort());

			if (m_pPopAccount->GetErrorCode() == 0)
			{
				CreateAlert(EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_MAILCOMP_FAILED_SERVER_CONNECT),
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetPort());

				SetErrorCode(1);
			}
			nRet = 1;
			goto cleanup;
		}
	}
	else
	{
		dca::String f;
		dca::String s(m_pPopAccount->GetServerAddress().c_str());
		f.Format("CPOP3Connection::CheckAccount - Invalid Message Source Type (server: %s)", s.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
			EMSERR_MAIL_COMPONENTS,
			EMS_LOG_INBOUND_MESSAGING,
			nRet),
			EMS_STRING_MAILCOMP_INVALID_TYPE,
			m_pPopAccount->GetDescription().c_str());

		if (m_pPopAccount->GetErrorCode() == 0)
		{
			CreateAlert(EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_MAILCOMP_INVALID_TYPE),
				m_pPopAccount->GetDescription().c_str());

			SetErrorCode(1);
		}
		nRet = 1;
		goto cleanup;
	}

	if (nType == 1)
	{
		if (m_pPopAccount->GetOAuthHostID() > 0)
		{
			nRet = AuthenticateOAuth2POP3();
		}
		else
		{
			nRet = Authenticate();
		}

		if (nRet != 0)
		{
			CkString sError;
			m_mailman.get_LastErrorText(sError);
			dca::String f;
			dca::String s(m_pPopAccount->GetServerAddress().c_str());
			dca::String n(m_pPopAccount->GetUserName().c_str());
			dca::String r(sError.getString());
			f.Format("CPOP3Connection::CheckAccount - POP3 account authentication failed, likely incorrect login details (server: %s, account: %s, response: %s)", s.c_str(), n.c_str(), r.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			// logpoint: POP3 account authentication failed (server: %s, account: %s, response: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_POP3_AUTH_FAILURE,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str(),
				A2T(sError.getString()));

			if (m_pPopAccount->GetErrorCode() == 0)
			{
				CreateAlert(EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_MAILCOMP_POP3_AUTH_FAILURE),
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str(),
					A2T(sError.getString()));

				SetErrorCode(1);
			}

			goto quitandcleanup;
		}
	}
	else
	{
		// authenticate to the IMAP server
		if (m_pPopAccount->GetOAuthHostID() > 0)
		{
			int nRet = AuthenticateOAuth2IMAP();
			if (nRet == 0)
			{
				bRet = true;
			}
			else
			{
				bRet = false;
			}
		}
		else
		{
			const char* sUserName = T2A(m_pPopAccount->GetUserName().c_str());
			const char* sPassword = T2A(m_pPopAccount->GetPassword().c_str());

			bRet = m_imap.Login(sUserName, sPassword);
		}

		if (bRet != true)
		{
			dca::String f;
			dca::String s(m_pPopAccount->GetServerAddress().c_str());
			dca::String n(m_pPopAccount->GetUserName().c_str());
			f.Format("CPOP3Connection::CheckAccount - IMAP account authentication failed, likely incorrect login details (server: %s, account: %s)", s.c_str(), n.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			// logpoint: POP3 account authentication failed (server: %s, account: %s, response: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_POP3_AUTH_FAILURE,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str(),
				_T("Invalid Username or Password"));

			if (m_pPopAccount->GetErrorCode() == 0)
			{
				CreateAlert(EMS_ALERT_EVENT_INBOUND_DELIVERY, (EMS_STRING_MAILCOMP_POP3_AUTH_FAILURE),
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str(),
					_T("Invalid Username or Password"));

				SetErrorCode(1);
			}
			nRet = 1;
			goto quitandcleanup;
		}

		bRet = m_imap.SelectMailbox("Inbox");
		if (bRet != true)
		{
			dca::String f;
			dca::String s(m_pPopAccount->GetServerAddress().c_str());
			dca::String n(m_pPopAccount->GetUserName().c_str());
			f.Format("CPOP3Connection::CheckAccount - IMAP select Inbox failed (server: %s, account: %s)", s.c_str(), n.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_POP3_AUTH_FAILURE,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str(),
				_T("Select Inbox failed"));

			nRet = 1;
			goto quitandcleanup;
		}
	}

	if (m_pPopAccount->GetErrorCode() > 0)
	{
		SetErrorCode(0);
	}

	DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - get the msg listing (the msg sizes are stored here as well)", DebugReporter::MAIL);

	// get the msg listing (the msg sizes are stored here as well)
	if (nType == 1)
	{
		try
		{
			uidlList = m_mailman.GetUidls();
			// Bug Fix 3/23/2024 checking for null on uidList
			if (uidlList != nullptr)
			{
				nCount = uidlList->get_Count();
			}
			auto lastVal = m_mailman.get_LastMethodSuccess();
			if (lastVal != true)
			{
				dca::String f;
				dca::String s(m_pPopAccount->GetServerAddress().c_str());
				dca::String n(m_pPopAccount->GetUserName().c_str());
				f.Format("CPOP3Connection::CheckAccount - Failed to retrieve UIDL list for POP3 account, check Mailbox for possible corrupt messages (server: %s, account: %s)", s.c_str(), n.c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					nRet),
					EMS_STRING_MAILCOMP_POP3_LIST_FAILURE,
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str());

				nRet = 1;
				// Bug Fix 3/23/2024 checking for null on uidList
				if (uidlList != nullptr)
				{
					delete uidlList;
				}
				goto quitandcleanup;
			}
		}
		catch (...)
		{
			DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - exception occurred in GetUidls, set nRet=1 then quitandcleanup", DebugReporter::MAIL);
			nRet = 1;
			delete uidlList;
			goto quitandcleanup;
		}

		try
		{
			//Loop through the uidlList and create a msgMap
			for (i = 0; i <= nCount - 1; i++)
			{
				const char* sUidl = uidlList->getString(i);
				_msg_info* msginfo = new _msg_info;
				assert(msginfo);
				if (!msginfo)
				{
					DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - uidlList loop failed to create new msginfo, gotocleanup and return 1", DebugReporter::MAIL);
					//return ERROR_MEMORY_ALLOCATION;
					nRet = 1;
					goto quitandcleanup;
				}

				msginfo->sUIDLString.resize(strlen(sUidl));
				MultiByteToWideChar(CP_ACP,
					0,
					sUidl,
					strlen(sUidl),
					(TCHAR*)msginfo->sUIDLString.data(),
					msginfo->sUIDLString.size());
				m_mapMsgInfo[i + 1] = msginfo;

			}
		}
		catch (...)
		{
			DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - exception occurred in uidlList create msgMap, set nRet=1 then quitandcleanup", DebugReporter::MAIL);
			nRet = 1;
			delete uidlList;
			goto quitandcleanup;
		}

		try
		{
			if (m_bServerSupportsUIDL)
			{
				nRet = PruneUIDLListing();
				if (nRet != 0)
				{
					dca::String f;
					dca::String s(m_pPopAccount->GetServerAddress().c_str());
					dca::String n(m_pPopAccount->GetUserName().c_str());
					f.Format("CPOP3Connection::CheckAccount -  Error occured while processing UIDL listing (server: %s, account: %s)", s.c_str(), n.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					// logpoint: Error occured while processing UIDL listing (server: %s, account: %s)
					CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
						EMSERR_MAIL_COMPONENTS,
						EMS_LOG_INBOUND_MESSAGING,
						nRet),
						EMS_STRING_MAILCOMP_UIDL_PRUNING_FAILED,
						m_pPopAccount->GetDescription().c_str(),
						m_pPopAccount->GetServerAddress().c_str(),
						m_pPopAccount->GetUserName().c_str());

					nRet = 1;
					delete uidlList;
					goto quitandcleanup;
				}
			}
		}
		catch (...)
		{
			DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - exception occurred in PruneUIDLListing, set nRet=1 then quitandcleanup", DebugReporter::MAIL);
			nRet = 1;
			delete uidlList;
			goto quitandcleanup;
		}

		try
		{
			//Get Msg Headers of messages to download then set size
			CkStringArray saUidl;

			MAP_MSG_INFO::iterator iter = m_mapMsgInfo.begin();
			const MAP_MSG_INFO::iterator iter_end = m_mapMsgInfo.end();
			for (; iter != iter_end; ++iter)
			{
				_msg_info* pMsgInfo = (_msg_info*)iter->second;
				saUidl.Append(T2A(pMsgInfo->sUIDLString.c_str()));
				delete pMsgInfo;
			}
			m_mapMsgInfo.clear();
			if (uidlList->get_Count() > 0)
			{
				popBundle = m_mailman.FetchMultipleHeaders(saUidl, 0);
				if (m_mailman.get_LastMethodSuccess() != true)
				{
					dca::String f;
					dca::String s(m_pPopAccount->GetServerAddress().c_str());
					dca::String n(m_pPopAccount->GetUserName().c_str());
					f.Format("CPOP3Connection::CheckAccount - Failed to retrieve headers for POP3 account (server: %s, account: %s)", s.c_str(), n.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
						EMSERR_MAIL_COMPONENTS,
						EMS_LOG_INBOUND_MESSAGING,
						nRet),
						EMS_STRING_MAILCOMP_POP3_LIST_FAILURE,
						m_pPopAccount->GetDescription().c_str(),
						m_pPopAccount->GetServerAddress().c_str(),
						m_pPopAccount->GetUserName().c_str());

					nRet = 1;
					delete uidlList;
					goto quitandcleanup;
				}
			}
		}
		catch (...)
		{
			DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - exception occurred in get message headers and set size, set nRet=1 then quitandcleanup", DebugReporter::MAIL);
			nRet = 1;
			delete uidlList;
			goto quitandcleanup;
		}

		CkEmail* email = 0;

		try
		{
			i = 0;
			if (uidlList->get_Count() > 0)
			{
				for (i = 0; i <= popBundle->get_MessageCount() - 1; i++)
				{
					email = popBundle->GetEmail(i);
					int nSize = email->get_Size();
					const char* uidl = email->uidl();
					CkString sDate;
					email->get_LocalDateStr(sDate);
					CkDateTime dateTime;
					dateTime.SetFromRfc822(sDate.getString());
					bool bIsOld = dateTime.OlderThan(m_pPopAccount->GetSkipDownloadDays(), "days");

					_msg_info* msginfo = new _msg_info;
					assert(msginfo);
					if (!msginfo)
					{
						DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - popBundle loop failed to create new msginfo, delete email object, set nRet=1 then quitandcleanup", DebugReporter::MAIL);
						//return ERROR_MEMORY_ALLOCATION;
						delete email;
						delete uidlList;
						nRet = 1;
						goto quitandcleanup;
					}
					msginfo->nMsgSizeBytes = nSize;
					msginfo->bIsOld = bIsOld;

					msginfo->sUIDLString.resize(strlen(uidl));
					MultiByteToWideChar(CP_ACP,
						0,
						uidl,
						strlen(uidl),
						(TCHAR*)msginfo->sUIDLString.data(),
						msginfo->sUIDLString.size());
					m_mapMsgInfo[i + 1] = msginfo;

					delete email;
				}
			}
		}
		catch (...)
		{
			DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - exception occurred in popBundle loop create new msginfo, delete email object, et nRet=1 then quitandcleanup", DebugReporter::MAIL);
			delete email;
			delete uidlList;
			nRet = 1;
			goto quitandcleanup;
		}
		delete uidlList;
	}
	else
	{
		m_imap.put_PeekMode(true);
		CkMessageSet* messageSet = 0;
		bool fetchUids = true;
		messageSet = m_imap.Search("ALL", fetchUids);
		//messageSet = m_imap.GetAllUids();
		if (m_imap.get_LastMethodSuccess() != true)
		{
			dca::String f;
			dca::String s(m_pPopAccount->GetServerAddress().c_str());
			dca::String n(m_pPopAccount->GetUserName().c_str());
			f.Format("CPOP3Connection::CheckAccount - Failed to retrieve message list for IMAP account (server: %s, account: %s) Errror: %s", s.c_str(), n.c_str(), m_imap.lastErrorText());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_POP3_LIST_FAILURE,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			nRet = 1;
			goto quitandcleanup;
		}
		if (messageSet == 0)
		{
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_INFORMATIONAL,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				ERROR_NO_ERROR),
				EMS_STRING_MAILCOMP_MSGS_WAITING,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetUserName().c_str(),
				0);

			delete messageSet;
			goto quitandcleanup;
		}
		CkEmailBundle* bundle = 0;
		bundle = m_imap.FetchHeaders(*messageSet);
		if (m_imap.get_LastMethodSuccess() != true)
		{
			dca::String f;
			dca::String s(m_pPopAccount->GetServerAddress().c_str());
			dca::String n(m_pPopAccount->GetUserName().c_str());
			f.Format("CPOP3Connection::CheckAccount - Failed to retrieve message list for IMAP account (server: %s, account: %s)", s.c_str(), n.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_POP3_LIST_FAILURE,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			nRet = 1;
			goto quitandcleanup;
		}

		if (bundle->get_MessageCount() == 0)
		{
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_INFORMATIONAL,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				ERROR_NO_ERROR),
				EMS_STRING_MAILCOMP_MSGS_WAITING,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetUserName().c_str(),
				0);

			nRet = 0;
			delete messageSet;
			delete bundle;
			goto quitandcleanup;
		}

		int i;
		for (i = 0; i <= bundle->get_MessageCount() - 1; i++)
		{
			CkEmail* email = 0;
			email = bundle->GetEmail(i);
			int nSize = email->get_Size();
			CkString sDate;
			email->get_LocalDateStr(sDate);
			CkDateTime dateTime;
			dateTime.SetFromRfc822(sDate.getString());
			bool bIsOld = dateTime.OlderThan(m_pPopAccount->GetSkipDownloadDays(), "days");
			CkString sSubject;
			email->get_Subject(sSubject);
			dca::String dsSubject(sSubject.getString());

			char buffer[33];
			dca::String sID(_itoa(messageSet->GetId(i), buffer, 10));

			_msg_info* msginfo = new _msg_info;
			assert(msginfo);
			if (!msginfo)
			{
				DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - bundle loop failed to create new msginfo, gotocleanup and return 1", DebugReporter::MAIL);
				//return ERROR_MEMORY_ALLOCATION;
				nRet = 1;
				delete messageSet;
				delete bundle;
				goto quitandcleanup;
			}
			msginfo->nMsgSizeBytes = nSize;
			msginfo->bIsOld = bIsOld;

			msginfo->sUIDLString.resize(sID.size());
			MultiByteToWideChar(CP_ACP,
				0,
				sID.c_str(),
				sID.size(),
				(TCHAR*)msginfo->sUIDLString.data(),
				msginfo->sUIDLString.size());
			m_mapMsgInfo[i + 1] = msginfo;

			delete email;
		}

		nRet = PruneIMAPUIDLListing(bundle);
		if (nRet != 0)
		{
			dca::String f;
			dca::String s(m_pPopAccount->GetServerAddress().c_str());
			dca::String n(m_pPopAccount->GetUserName().c_str());
			f.Format("CPOP3Connection::CheckAccount -  Error occured while processing UIDL listing (server: %s, account: %s)", s.c_str(), n.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_UIDL_PRUNING_FAILED,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			goto quitandcleanup;
		}

		// retrieve the messages waiting for this user
		nRet = GetWaitingIMAPMsgs(bundle);
		if (nRet != 0)
		{
			dca::String f;
			dca::String s(m_pPopAccount->GetServerAddress().c_str());
			dca::String n(m_pPopAccount->GetUserName().c_str());
			f.Format("CPOP3Connection::CheckAccount - Failed to retrieve waiting messages for account (server: %s, account: %s)", s.c_str(), n.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			// logpoint: Failed to retrieve waiting messages for account (server: %s, account: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MESSAGE_RETRIEVAL_FAILURE,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			goto quitandcleanup;
		}
		m_imap.ExpungeAndClose();
		bool success = m_imap.Logout();
		if (success)
		{
			dca::String f;
			dca::String a(m_pPopAccount->GetServerAddress().c_str());
			f.Format("CPOP3Connection::CheckAccount() - Successfully issued IMAP LOGOUT command to %s", a.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		}
		if (m_imap.IsConnected())
		{
			bool success = m_imap.Disconnect();
			if (success)
			{
				dca::String f;
				dca::String a(m_pPopAccount->GetServerAddress().c_str());
				f.Format("CPOP3Connection::CheckAccount() - Successfully disconnected from IMAP server %s", a.c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			}
		}
		delete messageSet;
		delete bundle;
	}

	if (nType == 1)
	{
		// retrieve the messages waiting for this user
		nRet = GetWaitingMsgs();
		if (nRet != 0)
		{
			dca::String f;
			dca::String s(m_pPopAccount->GetServerAddress().c_str());
			dca::String n(m_pPopAccount->GetUserName().c_str());
			f.Format("CPOP3Connection::CheckAccount - Failed to retrieve waiting messages for account (server: %s, account: %s)", s.c_str(), n.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			// logpoint: Failed to retrieve waiting messages for account (server: %s, account: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MESSAGE_RETRIEVAL_FAILURE,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			goto quitandcleanup;
		}
	}

quitandcleanup:

	if (Quit() != 0)
	{
		if (nType == 1)
		{
			DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - POP3 QUIT command failed", DebugReporter::MAIL);
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - IMAP LOGOUT command failed", DebugReporter::MAIL);
		}
	}

cleanup:

	dca::String f;
	dca::String d(m_pPopAccount->GetDescription().c_str());
	f.Format("CPOP3Connection::CheckAccount - Message retrieval complete (message source: %s)", d.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	// logpoint: POP3 message retrieval complete (server: %s, account: %s)
	CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_INFORMATIONAL,
		EMSERR_MAIL_COMPONENTS,
		EMS_LOG_INBOUND_MESSAGING,
		ERROR_NO_ERROR),
		EMS_STRING_MAILCOMP_POP3_CHECK_COMPLETE,
		m_pPopAccount->GetDescription().c_str());

	/*if (m_pPopAccount->GetIsSSL())
	{
		m_socket.Close(20000);
	}
	else
	{
		m_sockCore.Close();
	}*/

	delete popBundle;
	// delete m_pScanner;

	//CoUninitialize();

	return nRet;
}

const int CPOP3Connection::GetGreetingTimestamp(LPCSTR szGreeting, char* szTimeStamp)
{
	char* pFind = (char*)strstr(szGreeting, "<");
	if (pFind == NULL)
		return ERROR_MALFORMED_RESPONSE;

	const int nLoc = (pFind - szGreeting);

	char* pFindEnd = (char*)strstr(szGreeting + nLoc, ">");
	if (pFindEnd == NULL)
		return ERROR_MALFORMED_RESPONSE;

	const int nLocEnd = (pFindEnd - szGreeting) + 1;

	strncpy(szTimeStamp, (szGreeting + nLoc), nLocEnd - nLoc);
	memset(szTimeStamp + (nLocEnd - nLoc), '\0', 1);

	USES_CONVERSION;
	dca::String f;
	dca::String s(m_pPopAccount->GetServerAddress().c_str());
	f.Format("Retrieved greeting timestamp (%s) from %s", szGreeting, s.c_str());
	DebugReporter::Instance().DisplayMessage("CPOP3Connection::CheckAccount - POP3 QUIT command failed", DebugReporter::MAIL);

	return 0; // success
}

const int CPOP3Connection::SetPopAccount(CPopAccount* pPopAccount)
{
	if (!pPopAccount)
		return ERROR_BAD_PARAMETER;

	m_pPopAccount = pPopAccount;

	return 0; // success
}

const BOOL CPOP3Connection::GetIsResponseGood(LPCSTR m_szResponse)
{
	if (m_szResponse[0] == '+')
		return TRUE;

	return FALSE;
}

const int CPOP3Connection::Authenticate()
{
	bool bRet = false;
	USES_CONVERSION;

	const char* sUserName = T2A(m_pPopAccount->GetUserName().c_str());
	const char* sPassword = T2A(m_pPopAccount->GetPassword().c_str());

	if (m_pPopAccount->GetIsAPOP())
	{
		m_mailman.put_UseApop(true);
	}

	m_mailman.put_PopUsername(sUserName);
	m_mailman.put_PopPassword(sPassword);

	//bRet = m_mailman.Pop3Authenticate();
	bRet = true;

	if (!bRet)
	{
		return 1;
	}
	else
	{
		dca::String f;
		dca::String u(m_pPopAccount->GetUserName().c_str());
		dca::String s(m_pPopAccount->GetServerAddress().c_str());
		f.Format("CPOP3Connection::Authenticate() - Successfully authenticated user %s to %s", u.c_str(), s.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	}

	return 0;
}

const int CPOP3Connection::AuthenticateOAuth2POP3()
{
	USES_CONVERSION;
	int nRet = 0;
	bool success = false;
	try
	{
		// Check the expiration of the AccessToken		
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		const SYSTEMTIME ateTime = m_pPopAccount->GetAccessTokenExpire();
		TIMESTAMP_STRUCT tsSys;
		long tsSysLen = 0;
		TIMESTAMP_STRUCT tsAte;
		long tsAteLen = 0;
		SystemTimeToTimeStamp(sysTime, tsSys);
		SystemTimeToTimeStamp(ateTime, tsAte);
		nRet = OrderTimeStamps(tsSys, tsAte);

		if (nRet != 1)
		{
			// AccessToken expired, get a new one using the RefreshToken
			nRet = RefreshToken();
			if (nRet != 0)
			{
				//refreshing the token failed
				DebugReporter::Instance().DisplayMessage("CPOP3Connection::AuthenticateOAuth2POP3() - Refreshing the token failed", DebugReporter::MAIL);
				return nRet;
			}
		}
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	catch (CEMSException eex)
	{
		dca::String f;
		dca::String e(eex.GetErrorString());
		f.Format("CPOP3Connection::AuthenticateOAuth2POP3() - EMS Exception occured: %d, %s", eex.GetErrorCode(), e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return eex.GetErrorCode();
	}

	const char* accessToken = T2A(m_pPopAccount->GetAccessToken().c_str());
	m_oauth2.put_AccessToken(accessToken);

	const char* userName = T2A(m_pPopAccount->GetUserName().c_str());

	m_mailman.put_PopUsername(userName);
	m_mailman.put_PopPassword("");
	m_mailman.put_OAuth2AccessToken(m_oauth2.accessToken());

	//success = m_mailman.Pop3Authenticate();
	success = true;
	if (!success)
	{
		//refresh the accesstoken and try again
		nRet = RefreshToken();
		if (nRet != 0)
		{
			//refreshing the token failed
			DebugReporter::Instance().DisplayMessage("CPOP3Connection::AuthenticateOAuth2POP3() - Refreshing the token failed", DebugReporter::MAIL);
			return nRet;
		}

		m_mailman.put_OAuth2AccessToken(m_oauth2.accessToken());
		success = m_mailman.Pop3Authenticate();
		if (!success)
		{
			return 1;
		}
	}

	return 0;
}

const int CPOP3Connection::AuthenticateOAuth2IMAP()
{
	USES_CONVERSION;
	int nRet = 0;
	bool success = false;
	try
	{
		// Check the expiration of the AccessToken		
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		const SYSTEMTIME ateTime = m_pPopAccount->GetAccessTokenExpire();
		TIMESTAMP_STRUCT tsSys;
		long tsSysLen = 0;
		TIMESTAMP_STRUCT tsAte;
		long tsAteLen = 0;
		SystemTimeToTimeStamp(sysTime, tsSys);
		SystemTimeToTimeStamp(ateTime, tsAte);
		nRet = OrderTimeStamps(tsSys, tsAte);

		if (nRet != 1)
		{
			// AccessToken expired, get a new one using the RefreshToken
			nRet = RefreshToken();
			if (nRet != 0)
			{
				//refreshing the token failed
				DebugReporter::Instance().DisplayMessage("CPOP3Connection::AuthenticateOAuth2IMAP() - Refreshing the token failed", DebugReporter::MAIL);
				return nRet;
			}
		}
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	catch (CEMSException eex)
	{
		dca::String f;
		dca::String e(eex.GetErrorString());
		f.Format("CPOP3Connection::AuthenticateOAuth2POP3() - EMS Exception occured: %d, %s", eex.GetErrorCode(), e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return eex.GetErrorCode();
	}

	const char* accessToken = T2A(m_pPopAccount->GetAccessToken().c_str());
	m_oauth2.put_AccessToken(accessToken);

	const char* userName = T2A(m_pPopAccount->GetUserName().c_str());
	m_imap.put_AuthMethod("XOAUTH2");

	success = m_imap.Login(userName, m_oauth2.accessToken());
	if (!success)
	{
		//refresh the accesstoken and try again
		nRet = RefreshToken();
		if (nRet != 0)
		{
			//refreshing the token failed
			DebugReporter::Instance().DisplayMessage("CPOP3Connection::AuthenticateOAuth2IMAP() - Refreshing the token failed", DebugReporter::MAIL);
			return nRet;
		}

		success = m_imap.Login(userName, m_oauth2.accessToken());
		if (!success)
		{
			return 1;
		}
	}

	return 0;
}

//const int CPOP3Connection::GetUIDLListing()
//{
//	int nRet=0;
//	int nRead=0;
//
//	strcpy(m_szCmd, "UIDL\r\n");
//
//	nRet = GetCommandResponse(m_szCmd, FALSE, TRUE);
//	if (nRet != 0)
//		return nRet;
//
//	// keep reading lines until we hit a period by itself
//	while (1)
//	{
//		if (m_pPopAccount->GetIsSSL())
//		{
//			const char * receivedMsg;
//			receivedMsg = m_socket.receiveToCRLF();
//			if (receivedMsg == 0 ) {
//				nRet=1;		
//			}
//			else
//			{
//				strcpy(m_szResponse, receivedMsg);			
//			}		
//		}
//		else
//		{
//			nRet = m_sockCore.ReadLine(m_szResponse, LARGE_BUFFER_LENGTH, nRead, m_pPopAccount->GetConnTimeoutSecs());
//		}
//		
//		if (nRet != 0)
//			return nRet;
//
//		// if this is the multi-line terminator, then break
//		int len = strlen(m_szResponse);
//		if (len >= 3 
//			&& m_szResponse[0] == '.' 
//			&& (m_szResponse[1] == '\r' && m_szResponse[2] == '\n')) 
//		{
//            break;
//        }
//
//		{
//			//dca::String f;
//			//f.Format("CPOP3Connection::GetUIDLListing() - GetUIDLListing() response: length = %d, string = %s ", nRead, m_szResponse);
//			//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
//		}
//
//		DwString sResponse(m_szResponse);
//		size_t nPos = sResponse.find(' ', 0);
//		if (nPos == DwString::npos)
//		{
//			//DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetUIDLListing() - Failed to find whitespace in GetUIDLListing() response", DebugReporter::MAIL);
//			// DrainResponse();
//			return ERROR_MALFORMED_RESPONSE;
//			// break;
//		}		
//
//		// here's the msg index
//		int nIndex = atoi(sResponse.c_str());
//
//		// get the UIDL string
//		sResponse.erase(0, nPos);
//		sResponse.Trim();
//
//		// find the map entry and set it's msg size value
//		MAP_MSG_INFO::iterator iter = m_mapMsgInfo.find(nIndex);
//		if (iter != m_mapMsgInfo.end())		// found a match
//		{
//			// set the size
//			_msg_info* pMsgInfo = (_msg_info*)iter->second;
//
//			pMsgInfo->sUIDLString.resize(sResponse.size());
//			MultiByteToWideChar(CP_ACP, 
//								0, 
//								sResponse.c_str(), 
//								sResponse.size(),
//								(TCHAR*)pMsgInfo->sUIDLString.data(),
//								pMsgInfo->sUIDLString.size());
//			
//
//			//pMsgInfo->sUIDLString = A2T(sResponse.c_str());
//
//		}
//	}
//
//	dca::String f;
//	dca::String a(m_pPopAccount->GetServerAddress().c_str());
//	f.Format("CPOP3Connection::GetUIDLListing() - Successfully retrieved UIDL listing from %s", a.c_str());
//	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
//
//	return 0;	// success
//}

const int CPOP3Connection::Quit()
{
	if (m_pPopAccount->GetMessageSourceTypeID() == 1)
	{
		if (m_mailman.get_IsPop3Connected())
		{
			bool success = m_mailman.Pop3EndSession();
			if (!success)
			{
				return 1;
			}
			dca::String f;
			dca::String a(m_pPopAccount->GetServerAddress().c_str());
			f.Format("CPOP3Connection::Quit() - Closing Connection. Successfully issued QUIT command to %s", a.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		}
	}
	else
	{
		if (m_imap.IsConnected())
		{
			bool success = m_imap.Logout();
			if (!success)
			{
				return 1;
			}
			dca::String f;
			dca::String a(m_pPopAccount->GetServerAddress().c_str());
			f.Format("CPOP3Connection::Quit() - Closing Connection. Successfully issued LOGOUT command to %s", a.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		}
	}

	return 0; // success
}

void CPOP3Connection::Stop()
{
	// stop any socket-level activity
	if (m_mailman.get_IsPop3Connected())
	{
		m_mailman.Pop3EndSession();
	}
}

const int CPOP3Connection::GetWaitingMsgs()
{
	int nRet = 0;
	tstring sFileName;
	USES_CONVERSION;
	DwString* pMsg = NULL;

	// logpoint: POP3 account has %d messages waiting (server: %s, account: %s)
	CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_INFORMATIONAL,
		EMSERR_MAIL_COMPONENTS,
		EMS_LOG_INBOUND_MESSAGING,
		ERROR_NO_ERROR),
		EMS_STRING_MAILCOMP_MSGS_WAITING,
		m_pPopAccount->GetDescription().c_str(),
		m_pPopAccount->GetUserName().c_str(),
		m_mapMsgInfo.size());

	// iterate over the UIDL list
	MAP_MSG_INFO::iterator iter = m_mapMsgInfo.begin();
	const MAP_MSG_INFO::iterator iter_end = m_mapMsgInfo.end();
	for (; iter != iter_end; ++iter)
	{
		bool bDupMsg = false;
		DwString sMsg;
		CInboundMessage message;

		// cast iterators
		int nMsgIndex = (int)iter->first;
		_msg_info* pMsgInfo = (_msg_info*)iter->second;
		if (!pMsgInfo)
			return ERROR_UNKNOWN_ERROR;

		dca::String f;
		f.Format("CPOP3Connection::GetWaitingMsgs - Getting ready to retrieve message index no. %d.", nMsgIndex);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

		// get this specific message
		nRet = GetMsg(sMsg, message, *pMsgInfo, nMsgIndex);
		if (nRet != 0)
		{
			if (nRet == FAILURE_OPERATION_TIMEOUT)
			{
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					nRet),
					EMS_STRING_MAILCOMP_MSG_RETRIEVAL_FAILURE,
					m_pPopAccount->GetDescription().c_str(),
					nMsgIndex,
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str());

				message.SetPartial(TRUE);
			}
			else if (nRet != ERROR_MSG_TOO_OLD)
			{
				// logpoint: Unable to retrieve message index no. %d (server: %s, account: %s) - retrieval will be re-attempted
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					nRet),
					EMS_STRING_MAILCOMP_MSG_RETRIEVAL_FAILURE,
					m_pPopAccount->GetDescription().c_str(),
					nMsgIndex,
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str());

				// if the is problem related to a command failure or socket timeout, we'll just keep
				// on truckin', otherwise it's safest to halt POP processing
				if (nRet == ERROR_COMMAND_FAILED)
				{
					nRet = ERROR_NO_ERROR;
					continue;
				}
				else
					break;
			}
		}

		if (nRet == ERROR_MSG_TOO_OLD)
		{
			bDupMsg = true;
			// logpoint: [%s] message not routed because it is older than [%d] days (server: %s account: %s subject: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_WARNING,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				ERROR_NO_ERROR),
				EMS_STRING_MAILCOMP_MSG_SKIPPED,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetSkipDownloadDays(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str(),
				A2T(message.GetSubject().GetText().c_str()));

			nRet = ERROR_NO_ERROR;
		}
		else if (m_pPopAccount->GetDupMsg() > 0)
		{
			// if message source dup enabled, let's check for dup
			bDupMsg = IsMsgDup(A2T(message.GetMsgID().GetText().c_str()), m_pPopAccount->GetMessageSourceID());
			if (bDupMsg)
			{
				USES_CONVERSION;

				// logpoint: [%s] a duplicate message was detected and will not be routed (server: %s account: %s subject: %s)
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_WARNING,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_DUP_MSG,
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str(),
					A2T(message.GetSubject().GetText().c_str()));
			}
		}

		if (nRet != FAILURE_OPERATION_TIMEOUT)
		{
			// save the message (to db and filesystem)
			// note: virus scanning will also occur during the msg save process
			nRet = message.Save(m_pPopAccount->GetMessageSourceID(), m_pPopAccount->GetUseReplyTo(), m_pPopAccount->GetZipAttach(), bDupMsg);
			if (nRet != 0)
			{
				// logpoint: Unable to save message index no. %d (server: %s, account: %s)
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					nRet),
					EMS_STRING_MAILCOMP_MSG_SAVE_FAILED,
					nMsgIndex,
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str());

				// error evaluation
				switch (nRet)
				{
					// errors that will not abort the msg download loop
				case ERROR_FILESYSTEM:
					message.SetPartial(TRUE);
					nRet = ERROR_NO_ERROR;
					break;
				case ERROR_ENACTING_SCAN:
					nRet = ERROR_NO_ERROR;
					break;
					// everything else will abort the msg download loop
				default:
					return nRet;
				}
			}
		}

		// as long as the mail server supports UIDL, we'll save UIDL information; this allows
		// us to avoid duplicate message downloads even if the next step (message deletion)
		// fails for some reason
		if (m_bServerSupportsUIDL)
		{
			int nWasTimeOut = nRet;
			// record this UIDL
			nRet = SaveUIDL(pMsgInfo->sUIDLString.c_str(), message.GetPartial());
			if (nRet != 0)
			{
				// logpoint: Failed to save UIDL information for message index no. %d (server: %s, account: %s)
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					nRet),
					EMS_STRING_MAILCOMP_UIDL_SAVE_FAILURE,
					nMsgIndex,
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str());

				break;
			}

			if (nWasTimeOut == FAILURE_OPERATION_TIMEOUT)
				break;
		}

		// delete if the user doesn't want to leave mail on the server, or UIDL isn't supported.
		// note: we also never want to delete a "partially" downloaded message (one that was
		// not completely downloaded due to its large size)
		if ((!m_pPopAccount->GetLeaveOnServer() || !m_bServerSupportsUIDL)
			&& !message.GetPartial())
		{
			USES_CONVERSION;
			nRet = 1;
			bool success = m_mailman.DeleteByUidl(T2A(pMsgInfo->sUIDLString.c_str()));
			if (success)
			{
				nRet = 0;
			}
			if (nRet != 0)
			{
				// logpoint: Failed to delete (from mail server) message index no. %d (server: %s, account: %s)
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					nRet),
					EMS_STRING_MAILCOMP_SERVER_MSG_DELETE_FAILURE,
					nMsgIndex,
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str());

				// continue processing messages; this one won't be downloaded again
				// as long as UIDL is supported on the server, otherwise it will be
			}
		}
	}

	return nRet;
}

const int CPOP3Connection::GetWaitingIMAPMsgs(CkEmailBundle* bundle)
{
	int nRet = 0;
	tstring sFileName;
	USES_CONVERSION;
	DwString* pMsg = NULL;

	// logpoint: POP3 account has %d messages waiting (server: %s, account: %s)
	CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_INFORMATIONAL,
		EMSERR_MAIL_COMPONENTS,
		EMS_LOG_INBOUND_MESSAGING,
		ERROR_NO_ERROR),
		EMS_STRING_MAILCOMP_MSGS_WAITING,
		m_pPopAccount->GetDescription().c_str(),
		m_pPopAccount->GetUserName().c_str(),
		m_mapMsgInfo.size());

	// iterate over the UIDL list
	MAP_MSG_INFO::iterator iter = m_mapMsgInfo.begin();
	const MAP_MSG_INFO::iterator iter_end = m_mapMsgInfo.end();
	for (; iter != iter_end; ++iter)
	{
		bool bDupMsg = false;
		DwString sMsg;
		CInboundMessage message;

		// cast iterators
		int nMsgIndex = (int)iter->first;
		_msg_info* pMsgInfo = (_msg_info*)iter->second;
		if (!pMsgInfo)
			return ERROR_UNKNOWN_ERROR;

		dca::String f;
		f.Format("CPOP3Connection::GetWaitingIMAPMsgs - Getting ready to retrieve message index no. %d.", nMsgIndex);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

		// get this specific message
		nRet = GetIMAPMsg(sMsg, message, *pMsgInfo, nMsgIndex);
		if (nRet != 0)
		{
			if (nRet == FAILURE_OPERATION_TIMEOUT)
			{
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					nRet),
					EMS_STRING_MAILCOMP_MSG_RETRIEVAL_FAILURE,
					m_pPopAccount->GetDescription().c_str(),
					nMsgIndex,
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str());

				message.SetPartial(TRUE);
			}
			else if (nRet != ERROR_MSG_TOO_OLD)
			{
				// logpoint: Unable to retrieve message index no. %d (server: %s, account: %s) - retrieval will be re-attempted
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					nRet),
					EMS_STRING_MAILCOMP_MSG_RETRIEVAL_FAILURE,
					m_pPopAccount->GetDescription().c_str(),
					nMsgIndex,
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str());

				// if the is problem related to a command failure or socket timeout, we'll just keep
				// on truckin', otherwise it's safest to halt POP processing
				if (nRet == ERROR_COMMAND_FAILED)
				{
					nRet = ERROR_NO_ERROR;
					continue;
				}
				else
					break;
			}
		}

		if (nRet == ERROR_MSG_TOO_OLD)
		{
			bDupMsg = true;

			// logpoint: [%s] message not routed because it is older than [%d] days (server: %s account: %s subject: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_WARNING,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				ERROR_NO_ERROR),
				EMS_STRING_MAILCOMP_MSG_SKIPPED,
				m_pPopAccount->GetDescription().c_str(),
				m_pPopAccount->GetSkipDownloadDays(),
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str(),
				A2T(message.GetSubject().GetText().c_str()));

			nRet = ERROR_NO_ERROR;
		}
		else if (m_pPopAccount->GetDupMsg() > 0)
		{
			// if message source dup enabled, let's check for dup
			bDupMsg = IsMsgDup(A2T(message.GetMsgID().GetText().c_str()), m_pPopAccount->GetMessageSourceID());
			if (bDupMsg)
			{
				USES_CONVERSION;

				// logpoint: [%s] a duplicate message was detected and will not be routed (server: %s account: %s subject: %s)
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_WARNING,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					ERROR_NO_ERROR),
					EMS_STRING_MAILCOMP_DUP_MSG,
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str(),
					A2T(message.GetSubject().GetText().c_str()));
			}
		}

		if (nRet != FAILURE_OPERATION_TIMEOUT)
		{
			// save the message (to db and filesystem)
			// note: virus scanning will also occur during the msg save process
			nRet = message.Save(m_pPopAccount->GetMessageSourceID(), m_pPopAccount->GetUseReplyTo(), m_pPopAccount->GetZipAttach(), bDupMsg);
			if (nRet != 0)
			{
				// logpoint: Unable to save message index no. %d (server: %s, account: %s)
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					nRet),
					EMS_STRING_MAILCOMP_MSG_SAVE_FAILED,
					nMsgIndex,
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str());

				// error evaluation
				switch (nRet)
				{
					// errors that will not abort the msg download loop
				case ERROR_FILESYSTEM:
					message.SetPartial(TRUE);
					nRet = ERROR_NO_ERROR;
					break;
				case ERROR_ENACTING_SCAN:
					nRet = ERROR_NO_ERROR;
					break;
					// everything else will abort the msg download loop
				default:
					return nRet;
				}
			}
		}

		// as long as the mail server supports UIDL, we'll save UIDL information; this allows
		// us to avoid duplicate message downloads even if the next step (message deletion)
		// fails for some reason
		if (m_bServerSupportsUIDL)
		{
			int nWasTimeOut = nRet;
			// record this UIDL
			nRet = SaveUIDL(pMsgInfo->sUIDLString.c_str(), message.GetPartial());
			if (nRet != 0)
			{
				// logpoint: Failed to save UIDL information for message index no. %d (server: %s, account: %s)
				CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
					EMSERR_MAIL_COMPONENTS,
					EMS_LOG_INBOUND_MESSAGING,
					nRet),
					EMS_STRING_MAILCOMP_UIDL_SAVE_FAILURE,
					nMsgIndex,
					m_pPopAccount->GetDescription().c_str(),
					m_pPopAccount->GetServerAddress().c_str(),
					m_pPopAccount->GetUserName().c_str());

				break;
			}

			if (nWasTimeOut == FAILURE_OPERATION_TIMEOUT)
				break;
		}

		// delete if the user doesn't want to leave mail on the server, or UIDL isn't supported.
		// note: we also never want to delete a "partially" downloaded message (one that was
		// not completely downloaded due to its large size)
		if ((!m_pPopAccount->GetLeaveOnServer() || !m_bServerSupportsUIDL)
			&& !message.GetPartial())
		{
			bool success;
			int i;
			for (i = 0; i <= bundle->get_MessageCount() - 1; i++)
			{
				CkEmail* email = 0;
				email = bundle->GetEmail(i);
				success = m_imap.SetMailFlag(*email, "Deleted", 1);
				if (success != true)
				{
					// logpoint: Failed to delete (from mail server) message index no. %d (server: %s, account: %s)
					CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
						EMSERR_MAIL_COMPONENTS,
						EMS_LOG_INBOUND_MESSAGING,
						nRet),
						EMS_STRING_MAILCOMP_SERVER_MSG_DELETE_FAILURE,
						nMsgIndex,
						m_pPopAccount->GetDescription().c_str(),
						m_pPopAccount->GetServerAddress().c_str(),
						m_pPopAccount->GetUserName().c_str());

					// continue processing messages; this one won't be downloaded again
					// as long as UIDL is supported on the server, otherwise it will be
				}
				delete email;
			}
		}
	}

	return nRet;
}

const int CPOP3Connection::GetMultiLineResponse(DwString& sMultiLine)
{
	return 0; // success
}

const int CPOP3Connection::RetrMsg(DwString& sMsg, const UINT nMsgID)
{
	int nRet;

	DebugReporter::Instance().DisplayMessage(" + CPOP3Connection::RetrMsg - Send RETR Command", DebugReporter::MAIL);

	// send RETR command; check the initial response line before
	// reading the message body
	_snprintf(m_szCmd, DEFAULT_BUFFER_LENGTH, "RETR %d\r\n", nMsgID);
	nRet = GetCommandResponse(m_szCmd);
	if (nRet != 0)
		return nRet;

	// read the msg body
	return GetMultiLineResponse(sMsg);
}


const int CPOP3Connection::RetrMsgToFile(const char* sUidl, tstring& sFileName)
{
	int nRet = 0;
	bool bDone = false;
	string sMsgBuffer;
	DWORD dwWritten;
	USES_CONVERSION;

	const short nStopBuf = 30;

	DebugReporter::Instance().DisplayMessage("CPOP3Connection::RetrMsgToFile - Get a temporary file", DebugReporter::MAIL);

	// get a temporary file
	CTempFile temp;
	nRet = temp.CreateTempFile(_T("VMF"));
	if (nRet != 0)
	{
		DebugReporter::Instance().DisplayMessage("CPOP3Connection::RetrMsgToFile - leaving function with error when creating temp file.", DebugReporter::MAIL);

		return nRet;
	}
	HANDLE hFileHandle = temp.GetFileHandle();
	sFileName = temp.GetFilePath(); // returned in caller-provided variable

	DebugReporter::Instance().DisplayMessage("CPOP3Connection::RetrMsgToFile - Send RETR command", DebugReporter::MAIL);

	// send RETR command; confirm positive initial response line before
	// reading entire message body
	CkEmail* email = m_mailman.FetchEmail(sUidl);
	if (m_mailman.get_LastMethodSuccess() == false)
	{
		DebugReporter::Instance().DisplayMessage("CPOP3Connection::RetrMsgToFile - leaving function with error when getting response from mailserver.", DebugReporter::MAIL);
		nRet = 1;
		delete email;
		return nRet;
	}

	sMsgBuffer = email->getMime();

	BOOL bWrite = WriteFile(hFileHandle,
		sMsgBuffer.c_str(),
		sMsgBuffer.size(),
		&dwWritten,
		NULL);
	if (!bWrite)
	{
		dca::String f;
		dca::String u(CUtility::GetErrorString(GetLastError()).c_str());
		f.Format("CPOP3Connection::RetrMsgToFile - Error writing message file: %d (%s)", GetLastError(), u.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		delete email;
		return ERROR_FILESYSTEM;
	}


	DebugReporter::Instance().DisplayMessage("CPOP3Connection::RetrMsgToFile - leaving function with success", DebugReporter::MAIL);

	delete email;
	return 0;
}

const int CPOP3Connection::RetrIMAPMsgToFile(const UINT nMsgID, tstring& sFileName)
{
	int nRet = 0;
	bool bDone = false;
	string sMsgBuffer;
	DWORD dwWritten;
	USES_CONVERSION;

	const short nStopBuf = 30;

	DebugReporter::Instance().DisplayMessage("CPOP3Connection::RetrMsgToFile - Get a temporary file", DebugReporter::MAIL);

	// get a temporary file
	CTempFile temp;
	nRet = temp.CreateTempFile(_T("VMF"));
	if (nRet != 0)
	{
		DebugReporter::Instance().DisplayMessage("CPOP3Connection::RetrMsgToFile - leaving function with error when creating temp file.", DebugReporter::MAIL);

		return nRet;
	}
	HANDLE hFileHandle = temp.GetFileHandle();
	sFileName = temp.GetFilePath();

	const char* msgMime = 0;
	msgMime = m_imap.fetchSingleAsMime(nMsgID, false);
	if (m_imap.get_LastMethodSuccess() != true)
	{
		dca::String f;
		dca::String s(m_pPopAccount->GetServerAddress().c_str());
		dca::String n(m_pPopAccount->GetUserName().c_str());
		f.Format("CPOP3Connection::CheckAccount - Failed to retrieve IMAP Message Index: %d (server: %s, account: %s)", nMsgID, s.c_str(), n.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

		return ERROR_COMMAND_FAILED;
	}
	sMsgBuffer.assign(msgMime);
	BOOL bWrite = WriteFile(hFileHandle,
		sMsgBuffer.c_str(),
		sMsgBuffer.size(),
		&dwWritten,
		NULL);
	if (!bWrite)
	{
		dca::String f;
		dca::String u(CUtility::GetErrorString(GetLastError()).c_str());
		f.Format("CPOP3Connection::RetrMsgToFile - Error writing message file: %d (%s)", GetLastError(), u.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return ERROR_FILESYSTEM;
	}

	DebugReporter::Instance().DisplayMessage("CPOP3Connection::RetrIMAPMsgToFile - leaving function with success", DebugReporter::MAIL);

	return 0;
}

const int CPOP3Connection::DeleteMsgByID(const UINT nMsgIndexNum)
{
	int nRet;

	// formulate DEL command
	_snprintf(m_szCmd, DEFAULT_BUFFER_LENGTH, "DELE %d\r\n", nMsgIndexNum);
	nRet = GetCommandResponse(m_szCmd);
	if (nRet != 0)
		return nRet;

	dca::String f;
	dca::String s(m_pPopAccount->GetServerAddress().c_str());
	f.Format("CPOP3Connection::DeleteMsgByID - Successfully deleted message UIDL #%d from %s", nMsgIndexNum, s.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

	return 0; // success
}

const int CPOP3Connection::DeleteIMAPMsgByID(const UINT nMsgIndexNum)
{
	int nRet;

	// formulate DEL command
	_snprintf(m_szCmd, DEFAULT_BUFFER_LENGTH, "DELE %d\r\n", nMsgIndexNum);
	nRet = GetCommandResponse(m_szCmd);
	if (nRet != 0)
		return nRet;

	dca::String f;
	dca::String s(m_pPopAccount->GetServerAddress().c_str());
	f.Format("CPOP3Connection::DeleteMsgByID - Successfully deleted message UIDL #%d from %s", nMsgIndexNum, s.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

	return 0; // success
}

const int CPOP3Connection::SaveUIDL(LPCTSTR sMsgID, BOOL bIsPartial)
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		LONG lMessageSourceID = m_pPopAccount->GetMessageSourceID();

		BINDPARAM_LONG(query, lMessageSourceID);
		BINDPARAM_TCHAR(query, (LPTSTR)sMsgID);
		BINDPARAM_LONG(query, bIsPartial);

		query.Execute(_T("INSERT INTO UIDLData ")
			_T("(MessageSourceID, Identifier, IsPartial) ")
			_T("VALUES ")
			_T("(?,?,?)"));
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}

bool CPOP3Connection::IsMsgDup(TCHAR* szMsgID, int nMessageSourceID)
{
	if (_tcscmp(szMsgID, _T("")) == 0)
	{
		return false;
	}

	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		int nInboundMessageID = 0;

		BINDPARAM_TCHAR(query, szMsgID);
		BINDPARAM_LONG(query, nMessageSourceID);
		BINDCOL_LONG_NOLEN(query, nInboundMessageID);

		query.Execute(_T("SELECT TOP 1 MessageID ")
			_T("FROM MessageTracking ")
			_T("WHERE HeadMsgID=? AND MessageSourceId=?"));

		query.Fetch();

		if (nInboundMessageID > 0)
			return true;
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return false;
	}

	return false;
}

const int CPOP3Connection::SetLastChecked()
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		// bind the lastchecked param
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		TIMESTAMP_STRUCT sqlTime;
		if (CEmailDate::SystemTimeToSQLTime(sysTime, sqlTime))
		{
			LONG sqlTimeLen = 0;
			BINDPARAM_TIME(query, sqlTime);
		}

		// bind the message id param
		int nMsgID = m_pPopAccount->GetMessageSourceID();
		LONG nMsgIDLen = 0;
		BINDPARAM_LONG(query, nMsgID);

		// execute the query
		query.Execute(_T("UPDATE	MessageSources ")
			_T("SET		LastChecked = ? ")
			_T("WHERE	MessageSourceID = ?"));

	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	catch (CEMSException eex)
	{
		dca::String f;
		dca::String e(eex.GetErrorString());
		f.Format("CPOP3Connection::SetLastChecked - EMS Exception occured: %d, %s", eex.GetErrorCode(), e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return eex.GetErrorCode();
	}

	return 0; // success
}


const int CPOP3Connection::SetErrorCode(LONG nErrorCode)
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		// bind the errorcode param
		LONG nErrorCodeLen = 0;
		BINDPARAM_LONG(query, nErrorCode);

		// bind the message source id param
		int nMsgSrcID = m_pPopAccount->GetMessageSourceID();
		LONG nMsgSrcIDLen = 0;
		BINDPARAM_LONG(query, nMsgSrcID);

		// execute the query
		query.Execute(_T("UPDATE	MessageSources ")
			_T("SET		ErrorCode = ? ")
			_T("WHERE	MessageSourceID = ?"));

	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	catch (CEMSException eex)
	{
		dca::String f;
		dca::String e(eex.GetErrorString());
		f.Format("CPOP3Connection::SetLastChecked - EMS Exception occured: %d, %s", eex.GetErrorCode(), e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return eex.GetErrorCode();
	}

	return 0; // success
}

const int CPOP3Connection::RetrTop(DwString& sMsg, const char* sUidl)
{
	int nRet = 1;
	sMsg.clear();
	int i = 0;
	for (i = 0; i <= popBundle->get_MessageCount() - 1; i++)
	{
		CkEmail* email = 0;
		email = popBundle->GetEmail(i);
		const char* uidl = email->uidl();
		if (strcmp(sUidl, uidl) == 0)
		{
			sMsg.append(email->header());
			nRet = 0;
		}
		delete email;
	}

	return nRet;
}


const int CPOP3Connection::GetMsg(DwString& sMsg, CInboundMessage& message, const _msg_info& msginfo, const UINT nMsgIndex)
{
	DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - Get and test for max size", DebugReporter::MAIL);

	int nRet;
	DwString tmpMsg;
	USES_CONVERSION;

	// read the maximum message size
	UINT nSizeLimitK = m_pPopAccount->GetMaxInboundMsgSize();
	UINT nSizeK = (msginfo.nMsgSizeBytes / 1024);
	UINT nSizeLimitKcorrected = nSizeLimitK * (UINT)1.37;
	UINT nSizeKcorrected = nSizeK / (UINT)1.37;
	const char* sUidl = T2A(msginfo.sUIDLString.c_str());
	if (nSizeK == 0) nSizeK = 1;

	if ((nSizeLimitK != 0) && (nSizeK > nSizeLimitKcorrected))
	{
		message.SetPartial(TRUE);

		// logpoint: Inbound message index no. %d size (%dk) exceeds defined limit of %dk (server: %s, account: %s)
		CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_WARNING,
			EMSERR_MAIL_COMPONENTS,
			EMS_LOG_INBOUND_MESSAGING,
			ERROR_NO_ERROR),
			EMS_STRING_MAILCOMP_MSG_SIZE_WARNING,
			m_pPopAccount->GetDescription().c_str(),
			nMsgIndex,
			nSizeKcorrected,
			nSizeLimitK,
			m_pPopAccount->GetServerAddress().c_str(),
			m_pPopAccount->GetUserName().c_str());

		// get msg headers
		nRet = RetrTop(sMsg, sUidl);
		if (nRet != 0)
		{
			// logpoint: Unable to retrieve TOP of message no. %d (server: %s, account: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_TOP_FAILED,
				m_pPopAccount->GetDescription().c_str(),
				nMsgIndex,
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			return ERROR_COMMAND_FAILED;
		}

		// parse the message headers
		nRet = message.ParseFrom(sMsg, false, false);
		if (nRet != 0)
		{
			assert(0);
			return nRet;
		}

		// set the memo text with our standard "message too large" string (contained
		// in the resource dll)
		USES_CONVERSION;

		try
		{
			CEMSString eString;
			eString.Format(EMS_STRING_MAILCOMP_MSG_TOO_LARGE, nSizeKcorrected, nSizeLimitK);
			message.GetMemoText().SetText(T2A(eString.c_str()), "ISO-8859-1");
		}
		catch (CEMSException EMSException)
		{
			dca::String f;
			dca::String e(EMSException.GetErrorString());
			f.Format("CPOP3Connection::GetMsg - Error formatting 'msg too large' string (%s)", e.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			return EMSException.GetErrorCode();
		}
	}
	else
	{
		tstring sFileName;

		DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - pull message to temp file", DebugReporter::MAIL);

		//get headers in case getting the full message errors
		nRet = RetrTop(tmpMsg, sUidl);
		if (nRet != 0)
		{
			// logpoint: Unable to retrieve TOP of message no. %d (server: %s, account: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_TOP_FAILED,
				m_pPopAccount->GetDescription().c_str(),
				nMsgIndex,
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			return ERROR_COMMAND_FAILED;
		}

		// pull the message to a temp file
		nRet = RetrMsgToFile(/*in*/T2A(msginfo.sUIDLString.c_str()), /*out*/sFileName);
		if (nRet != 0)
		{
			// logpoint: Error retrieving msg no. %d to file (server: %s, account: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_RETR_MSG_TO_FILE_ERROR,
				m_pPopAccount->GetDescription().c_str(),
				nMsgIndex,
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			// delete temp message file
			if (!DeleteFile(sFileName.c_str()))
				assert(0);

			DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - function failed, error when retreiving message", DebugReporter::MAIL);

			if (nRet == ERROR_READ_LINE)
			{
				message.SetPartial(TRUE);

				// parse the message headers
				nRet = message.ParseFrom(tmpMsg, false, false);
				if (nRet != 0)
				{
					assert(0);
					return nRet;
				}

				USES_CONVERSION;

				try
				{
					CEMSString eString;
					eString.Format(_T("Error occurred retrieving this message to file, message was left on server."));
					message.GetMemoText().SetText(T2A(eString.c_str()), "ISO-8859-1");
				}
				catch (CEMSException EMSException)
				{
					dca::String f;
					dca::String e(EMSException.GetErrorString());
					f.Format("CPOP3Connection::GetMsg - Error formatting 'msg to file' string (%s)", e.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					return EMSException.GetErrorCode();
				}
			}

			return nRet;
		}

		message.SetMsgFilePath(sFileName.c_str());

		DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - map the file to a string in chunks to help avoid memory spikes", DebugReporter::MAIL);

		// map the file to a string in chunks to help avoid memory spikes
		CMemMappedFile memfile;
		const BOOL bRes = memfile.MapFileToString(sFileName, sMsg);
		if (!bRes)
		{
			// logpoint: Error mapping message no. %d file to string (server: %s, account: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				ERROR_FILESYSTEM),
				EMS_STRING_MAILCOMP_ERR_MAPPING_FILE_TO_STRING,
				m_pPopAccount->GetDescription().c_str(),
				nMsgIndex,
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			assert(0);

			// delete temp message file
			if (!DeleteFile(sFileName.c_str()))
				assert(0);

			DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - function failed, error when mapping message", DebugReporter::MAIL);

			return ERROR_FILESYSTEM;
		}

		DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - unmapping file.", DebugReporter::MAIL);

		// unmap the file
		memfile.UnMap();

		DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - parse it.", DebugReporter::MAIL);

		// parse it
		if (m_pPopAccount->GetDateFilters() == 1)
		{
			nRet = message.ParseFrom(sMsg, true, true, m_pPopAccount->GetDFVector());
		}
		else
		{
			nRet = message.ParseFrom(sMsg, true, false);
		}

		if (nRet != 0)
		{
			assert(0);

			DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - function failed parsing from.", DebugReporter::MAIL);

			return nRet;
		}
	}

	DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - function succeeded.", DebugReporter::MAIL);

	if (m_pPopAccount->GetSkipDownloadDays() > 0 && msginfo.bIsOld)
	{
		return ERROR_MSG_TOO_OLD;
	}

	return 0;
}

const int CPOP3Connection::GetIMAPMsg(DwString& sMsg, CInboundMessage& message, const _msg_info& msginfo, const UINT nMsgIndex)
{
	DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - Get and test for max size", DebugReporter::MAIL);

	int nRet;
	DwString tmpMsg;

	// read the maximum message size
	UINT nSizeLimitK = m_pPopAccount->GetMaxInboundMsgSize();
	UINT nSizeK = (msginfo.nMsgSizeBytes / 1024);
	UINT nSizeLimitKcorrected = nSizeLimitK * (UINT)1.37;
	UINT nSizeKcorrected = nSizeK / (UINT)1.37;

	if (nSizeK == 0) nSizeK = 1;

	if ((nSizeLimitK != 0) && (nSizeK > nSizeLimitKcorrected))
	{
		message.SetPartial(TRUE);

		// logpoint: Inbound message index no. %d size (%dk) exceeds defined limit of %dk (server: %s, account: %s)
		CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_WARNING,
			EMSERR_MAIL_COMPONENTS,
			EMS_LOG_INBOUND_MESSAGING,
			ERROR_NO_ERROR),
			EMS_STRING_MAILCOMP_MSG_SIZE_WARNING,
			m_pPopAccount->GetDescription().c_str(),
			nMsgIndex,
			nSizeKcorrected,
			nSizeLimitK,
			m_pPopAccount->GetServerAddress().c_str(),
			m_pPopAccount->GetUserName().c_str());

		// get msg headers
		const char* hdrMime = 0;
		dca::String sTemp;
		int nUID = _ttoi(msginfo.sUIDLString.c_str());
		hdrMime = m_imap.fetchSingleHeaderAsMime(nUID, true);
		if (m_imap.get_LastMethodSuccess() != true)
		{
			dca::String f;
			dca::String s(m_pPopAccount->GetServerAddress().c_str());
			dca::String n(m_pPopAccount->GetUserName().c_str());
			f.Format("CPOP3Connection::CheckAccount - Failed to retrieve IMAP message header for Message UID: %d (server: %s, account: %s)", nUID, s.c_str(), n.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				0),
				EMS_STRING_MAILCOMP_TOP_FAILED,
				m_pPopAccount->GetDescription().c_str(),
				nMsgIndex,
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			return ERROR_COMMAND_FAILED;
		}
		sMsg = hdrMime;

		// parse the message headers
		nRet = message.ParseFrom(sMsg, false, false);
		if (nRet != 0)
		{
			assert(0);
			return nRet;
		}

		// set the memo text with our standard "message too large" string (contained
		// in the resource dll)
		USES_CONVERSION;

		try
		{
			CEMSString eString;
			eString.Format(EMS_STRING_MAILCOMP_MSG_TOO_LARGE, nSizeKcorrected, nSizeLimitK);
			message.GetMemoText().SetText(T2A(eString.c_str()), "ISO-8859-1");
		}
		catch (CEMSException EMSException)
		{
			dca::String f;
			dca::String e(EMSException.GetErrorString());
			f.Format("CPOP3Connection::GetMsg - Error formatting 'msg too large' string (%s)", e.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			return EMSException.GetErrorCode();
		}
	}
	else
	{
		tstring sFileName;

		DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - pull message to temp file", DebugReporter::MAIL);

		//get headers in case getting the full message errors
		const char* hdrMime = 0;
		hdrMime = m_imap.fetchSingleHeaderAsMime(nMsgIndex, false);
		if (m_imap.get_LastMethodSuccess() != true)
		{
			dca::String f;
			dca::String s(m_pPopAccount->GetServerAddress().c_str());
			dca::String n(m_pPopAccount->GetUserName().c_str());
			f.Format("CPOP3Connection::CheckAccount - Failed to retrieve IMAP message header for Message Index: %d (server: %s, account: %s)", nMsgIndex, s.c_str(), n.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				0),
				EMS_STRING_MAILCOMP_TOP_FAILED,
				m_pPopAccount->GetDescription().c_str(),
				nMsgIndex,
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			return ERROR_COMMAND_FAILED;
		}
		tmpMsg = hdrMime;

		// pull the message to a temp file
		nRet = RetrIMAPMsgToFile(/*in*/nMsgIndex, /*out*/sFileName);
		if (nRet != 0)
		{
			// logpoint: Error retrieving msg no. %d to file (server: %s, account: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				nRet),
				EMS_STRING_MAILCOMP_RETR_MSG_TO_FILE_ERROR,
				m_pPopAccount->GetDescription().c_str(),
				nMsgIndex,
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			// delete temp message file
			if (!DeleteFile(sFileName.c_str()))
				assert(0);

			DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - function failed, error when retreiving message", DebugReporter::MAIL);

			if (nRet == ERROR_READ_LINE)
			{
				message.SetPartial(TRUE);

				// parse the message headers
				nRet = message.ParseFrom(tmpMsg, false, false);
				if (nRet != 0)
				{
					assert(0);
					return nRet;
				}

				USES_CONVERSION;

				try
				{
					CEMSString eString;
					eString.Format(_T("Error occurred retrieving this message to file, message was left on server."));
					message.GetMemoText().SetText(T2A(eString.c_str()), "ISO-8859-1");
				}
				catch (CEMSException EMSException)
				{
					dca::String f;
					dca::String e(EMSException.GetErrorString());
					f.Format("CPOP3Connection::GetMsg - Error formatting 'msg to file' string (%s)", e.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					return EMSException.GetErrorCode();
				}
			}

			return nRet;
		}

		message.SetMsgFilePath(sFileName.c_str());

		DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - map the file to a string in chunks to help avoid memory spikes", DebugReporter::MAIL);

		// map the file to a string in chunks to help avoid memory spikes
		CMemMappedFile memfile;
		const BOOL bRes = memfile.MapFileToString(sFileName, sMsg);
		if (!bRes)
		{
			// logpoint: Error mapping message no. %d file to string (server: %s, account: %s)
			CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_ERROR,
				EMSERR_MAIL_COMPONENTS,
				EMS_LOG_INBOUND_MESSAGING,
				ERROR_FILESYSTEM),
				EMS_STRING_MAILCOMP_ERR_MAPPING_FILE_TO_STRING,
				m_pPopAccount->GetDescription().c_str(),
				nMsgIndex,
				m_pPopAccount->GetServerAddress().c_str(),
				m_pPopAccount->GetUserName().c_str());

			assert(0);

			// delete temp message file
			if (!DeleteFile(sFileName.c_str()))
				assert(0);

			DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - function failed, error when mapping message", DebugReporter::MAIL);

			return ERROR_FILESYSTEM;
		}

		DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - unmapping file.", DebugReporter::MAIL);

		// unmap the file
		memfile.UnMap();

		DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - parse it.", DebugReporter::MAIL);

		// parse it
		if (m_pPopAccount->GetDateFilters() == 1)
		{
			nRet = message.ParseFrom(sMsg, true, true, m_pPopAccount->GetDFVector());
		}
		else
		{
			nRet = message.ParseFrom(sMsg, true, false);
		}

		if (nRet != 0)
		{
			assert(0);

			DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - function failed parsing from.", DebugReporter::MAIL);

			return nRet;
		}
	}

	DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsg - function succeeded.", DebugReporter::MAIL);

	if (m_pPopAccount->GetSkipDownloadDays() > 0 && msginfo.bIsOld)
	{
		return ERROR_MSG_TOO_OLD;
	}

	return 0;
}

const int CPOP3Connection::GetMsgList()
{
	return 0;	// success
}

const int CPOP3Connection::GetCommandResponse(LPCSTR szCmd, const BOOL bIsSensitive, const BOOL bLongRespTimeout)
{
	return 0;
}

const int CPOP3Connection::InitConnection()
{
	bool bRet = false;
	long maxWaitMillisec = m_pPopAccount->GetConnTimeoutSecs() * 1000;
	USES_CONVERSION;

	const char* sslServerHost = T2A(m_pPopAccount->GetServerAddress().c_str());

	m_mailman.put_MailHost(sslServerHost);
	m_mailman.put_PopSsl(false);
	m_mailman.put_Pop3Stls(false);
	m_mailman.put_MailPort(m_pPopAccount->GetPort());
	m_mailman.put_ConnectTimeout(maxWaitMillisec);

	//bRet = m_mailman.Pop3Connect();
	bRet = true;

	if (bRet != true)
	{
		return 1;
	}

	return 0;
}

const int CPOP3Connection::InitSecureConnection()
{
	bool bRet = false;
	long maxWaitMillisec = m_pPopAccount->GetConnTimeoutSecs() * 1000;
	USES_CONVERSION;

	const char* sslServerHost = T2A(m_pPopAccount->GetServerAddress().c_str());

	//m_mailman.put_SslProtocol("TLS 1.0 or higher");
//m_mailman.put_SslAllowedCiphers("best-practices");
	m_mailman.put_MailHost(sslServerHost);
	m_mailman.put_PopSsl(true);
	m_mailman.put_MailPort(m_pPopAccount->GetPort());
	m_mailman.put_ConnectTimeout(maxWaitMillisec);

	//bRet = m_mailman.Pop3Connect();
	bRet = true;

	if (bRet != true)
	{
		return 1;
	}

	return 0;
}

void CPOP3Connection::ClearMsgInfo()
{
	MAP_MSG_INFO::iterator i = m_mapMsgInfo.begin();
	while (i != m_mapMsgInfo.end())
	{
		_msg_info* pInfo = (_msg_info*)i->second;
		if (pInfo)
			delete pInfo;

		i++;
	}

	m_mapMsgInfo.clear();
}

const int CPOP3Connection::PruneUIDLListing()
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		vector<UINT> vecToDelete;

		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		LONG lUIDLID;
		LONG lMessageSourceID = m_pPopAccount->GetMessageSourceID();
		TCHAR szString[255];
		LONG lIsPartial;
		LONG lKeepDays = m_pPopAccount->GetLeaveCopiesDays();
		int nIsOld;

		BINDCOL_LONG_NOLEN(query, lUIDLID);
		BINDCOL_TCHAR_NOLEN(query, szString);
		BINDCOL_LONG_NOLEN(query, lIsPartial);
		BINDCOL_LONG_NOLEN(query, nIsOld);
		if (lKeepDays > 0)
		{
			BINDPARAM_LONG(query, lKeepDays);
			BINDPARAM_LONG(query, lMessageSourceID);
			query.Execute(_T("SELECT	UIDLID, Identifier, IsPartial, ")
				_T("CASE WHEN UIDLDate < GETDATE() - ? THEN 1 ELSE 0 END AS IsOld ")
				_T("FROM	UIDLData ")
				_T("WHERE	MessageSourceID = ?"));
		}
		else
		{
			BINDPARAM_LONG(query, lMessageSourceID);
			query.Execute(_T("SELECT	UIDLID, Identifier, IsPartial, 0 ")
				_T("FROM	UIDLData ")
				_T("WHERE	MessageSourceID = ?"));
		}
		USES_CONVERSION;
		// iterate over query results and prune map
		while (query.Fetch() == S_OK)
		{
			BOOL bIsMsgPartial = (BOOL)lIsPartial;
			bool bMatchFound = false;

			// iterate over the UIDL map and remove any entries that
			// have already been downloaded
			MAP_MSG_INFO::iterator iter = m_mapMsgInfo.begin();
			while (iter != m_mapMsgInfo.end())
			{
				_msg_info* pMsgInfo = (_msg_info*)iter->second;
				if (pMsgInfo->sUIDLString == (tstring)szString)
				{
					if ((!bIsMsgPartial && !m_pPopAccount->GetLeaveOnServer()) || nIsOld)
					{
						bool success = m_mailman.DeleteByUidl(T2A(pMsgInfo->sUIDLString.c_str()));
						if (success == false)
						{
							dca::String f;
							f.Format("CPOP3Connection::PruneUIDLListing - Failed to delete old message #%d", iter->first);
							DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						}
						else
						{
							// logpoint: Deleted old message ID (%d) from server (server: %s, account: %s)
							CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_INBOUND_MESSAGING,
								ERROR_NO_ERROR),
								EMS_STRING_MAILCOMP_DELETED_OLD_MSG,
								m_pPopAccount->GetDescription().c_str(),
								iter->first,
								m_pPopAccount->GetServerAddress().c_str(),
								m_pPopAccount->GetUserName().c_str());

							if (nIsOld)
							{
								vecToDelete.push_back(lUIDLID);
							}
						}
					}

					// if we found a match, remove it from map and continue query fetch
					delete pMsgInfo;
					m_mapMsgInfo.erase(iter);

					bMatchFound = true;

					break;
				}
				else
					iter++;	// continue looping through map

			} // map iteration

			// this UIDL string was nowhere in the map, which means that the record no
			// longer exists on the server (which means it can be deleted)
			if (!bMatchFound)
			{
				dca::String f;
				dca::String s(szString);
				f.Format("CPOP3Connection::PruneUIDLListing - UIDL marker %s not found on server (will be deleted)", s.c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				vecToDelete.push_back(lUIDLID);
			}

		} // uidl database record iteration

		// clean out our UIDL data structure
		CleanUIDLData(vecToDelete, query);

	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}

const int CPOP3Connection::PruneIMAPUIDLListing(CkEmailBundle* bundle)
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		vector<UINT> vecToDelete;

		// init query object
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		LONG lUIDLID;
		LONG lMessageSourceID = m_pPopAccount->GetMessageSourceID();
		TCHAR szString[255];
		LONG lIsPartial;
		LONG lKeepDays = m_pPopAccount->GetLeaveCopiesDays();
		int nIsOld;

		BINDCOL_LONG_NOLEN(query, lUIDLID);
		BINDCOL_TCHAR_NOLEN(query, szString);
		BINDCOL_LONG_NOLEN(query, lIsPartial);
		BINDCOL_LONG_NOLEN(query, nIsOld);
		if (lKeepDays > 0)
		{
			BINDPARAM_LONG(query, lKeepDays);
			BINDPARAM_LONG(query, lMessageSourceID);
			query.Execute(_T("SELECT	UIDLID, Identifier, IsPartial, ")
				_T("CASE WHEN UIDLDate < GETDATE() - ? THEN 1 ELSE 0 END AS IsOld ")
				_T("FROM	UIDLData ")
				_T("WHERE	MessageSourceID = ?"));
		}
		else
		{
			BINDPARAM_LONG(query, lMessageSourceID);
			query.Execute(_T("SELECT	UIDLID, Identifier, IsPartial, 0 ")
				_T("FROM	UIDLData ")
				_T("WHERE	MessageSourceID = ?"));
		}

		// iterate over query results and prune map
		while (query.Fetch() == S_OK)
		{
			BOOL bIsMsgPartial = (BOOL)lIsPartial;
			bool bMatchFound = false;

			MAP_MSG_INFO::iterator iter = m_mapMsgInfo.begin();
			while (iter != m_mapMsgInfo.end())
			{
				_msg_info* pMsgInfo = (_msg_info*)iter->second;
				if (pMsgInfo->sUIDLString == (tstring)szString)
				{
					if ((!bIsMsgPartial && !m_pPopAccount->GetLeaveOnServer()) || nIsOld)
					{
						bool success;

						CkEmail* email = 0;
						email = bundle->GetEmail(iter->first - 1);
						success = m_imap.SetMailFlag(*email, "Deleted", 1);
						if (success != true)
						{
							dca::String f;
							f.Format("CPOP3Connection::PruneUIDLListing - Failed to delete old message #%d", iter->first);
							DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
						}
						else
						{
							CreateLogEntry(EMSERROR(EMS_LOG_SEVERITY_INFORMATIONAL,
								EMSERR_MAIL_COMPONENTS,
								EMS_LOG_INBOUND_MESSAGING,
								ERROR_NO_ERROR),
								EMS_STRING_MAILCOMP_DELETED_OLD_MSG,
								m_pPopAccount->GetDescription().c_str(),
								iter->first,
								m_pPopAccount->GetServerAddress().c_str(),
								m_pPopAccount->GetUserName().c_str());

							if (nIsOld)
							{
								vecToDelete.push_back(lUIDLID);
							}
						}
						delete email;
					}

					// if we found a match, remove it from map and continue query fetch
					delete pMsgInfo;
					m_mapMsgInfo.erase(iter);

					bMatchFound = true;

					break;
				}
				else
					iter++;	// continue looping through map

			} // map iteration

			// this UIDL string was nowhere in the map, which means that the record no
			// longer exists on the server (which means it can be deleted)
			if (!bMatchFound)
			{
				dca::String f;
				dca::String s(szString);
				f.Format("CPOP3Connection::PruneUIDLListing - UIDL marker %s not found on server (will be deleted)", s.c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				vecToDelete.push_back(lUIDLID);
			}

		} // uidl database record iteration

		// clean out our UIDL data structure
		CleanUIDLData(vecToDelete, query);

	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return 0;
}

const int CPOP3Connection::CleanUIDLData(vector<UINT>& vecToDelete, CODBCQuery& query)
{
	if (vecToDelete.size() == 0)
		return 0;

	dca::String f;
	f.Format("CPOP3Connection::CleanUIDLData - Cleaning up %d old UIDL records...", vecToDelete.size());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

	TCHAR buf[33];
	tstring sSQL(_T("DELETE from UIDLData where UIDLID IN ("));
	int n = 0;
	for (vector<UINT>::iterator i = vecToDelete.begin(); i < vecToDelete.end(); i++)
	{
		if (n != 0) sSQL += _T(",");

		_itot(*i, buf, 10);
		sSQL += buf;

		// we don't want to formulate an IN clause with more than 254 items at once;
		// this is safer across DB platforms
		n++;
		if ((n >= 254) || (i == (vecToDelete.end() - 1)))
		{
			sSQL.append(_T(")"));
			query.Reset(TRUE);
			query.Execute(sSQL.c_str());

			n = 0;
			sSQL = _T("DELETE from UIDLData where UIDLID IN (");
		}
	}

	return 0;
}

const BOOL CPOP3Connection::ContainsTermSequence(const char* szBuffer)
{
	assert(szBuffer);

	// if this is the multi-line terminator
	if (strlen(szBuffer) >= 3
		&& szBuffer[0] == '.'
		&& (szBuffer[1] == '\r' && szBuffer[2] == '\n'))
	{
		return TRUE;
	}

	return FALSE;
}

const int CPOP3Connection::ExtractMsgSize(const char* szBuffer)
{
	string sRes(szBuffer);
	int nMsgSize = -1;
	size_t pos = sRes.find(" ", 0);
	if (pos != string::npos)
	{
		sRes.erase(0, pos);
		nMsgSize = atoi(sRes.c_str());
		if (nMsgSize == 0)
			nMsgSize = -1;
	}

	return nMsgSize;
}

const int CPOP3Connection::GetMsgSizeByIndex(const int nIndex, int& nMsgSize)
{
	int nRet;

	_snprintf(m_szCmd, DEFAULT_BUFFER_LENGTH, "LIST %d\r\n", nIndex);

	nRet = GetCommandResponse(m_szCmd);
	if (nRet != 0)
		return nRet;

	DwString sResponse(m_szResponse);
	size_t nPos = sResponse.find(' ', 0);
	if (nPos == DwString::npos)
	{
		DebugReporter::Instance().DisplayMessage("CPOP3Connection::GetMsgSizeByIndex - Failed to find whitespace in GetMsgSizeByIndex() response", DebugReporter::MAIL);
		assert(0);
		return ERROR_MALFORMED_RESPONSE;
	}

	// get the message size, place it in caller-provided storage
	sResponse.erase(0, nPos);
	nMsgSize = atoi(sResponse.c_str());

	return 0;
}

const int CPOP3Connection::RefreshToken()
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	USES_CONVERSION;
	tstring aToken;
	tstring rToken;
	tstring sError;
	bool success = false;
	TOAuthHosts oh;
	try
	{
		CODBCQuery query(g_odbcConn);

		oh.m_OAuthHostID = m_pPopAccount->GetOAuthHostID();
		oh.Query(query);

		const char* tokenEndPoint = T2A(oh.m_TokenEndPoint);
		m_oauth2.put_TokenEndpoint(tokenEndPoint);

		const char* clientID = T2A(oh.m_ClientID);
		m_oauth2.put_ClientId(clientID);

		const char* clientSecret = T2A(oh.m_ClientSecret);
		m_oauth2.put_ClientSecret(clientSecret);

		const char* refreshToken = T2A(m_pPopAccount->GetRefreshToken().c_str());
		m_oauth2.put_RefreshToken(refreshToken);

		success = m_oauth2.RefreshAccessToken();
		if (success != true) {
			const char* lastError = m_oauth2.lastErrorText();
			dca::String f;
			dca::String e(lastError);
			f.Format("CPOP3Connection::RefreshToken() - Error occurred during RefreshToken(): %s", e.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			return 1;
		}

		aToken.assign(A2T(m_oauth2.accessToken()));
		rToken.assign(A2T(m_oauth2.refreshToken()));

		m_pPopAccount->SetAccessToken(aToken);
		m_pPopAccount->SetRefreshToken(rToken);

		time_t now;
		time(&now);
		TIMESTAMP_STRUCT tsaTokenExpire;
		long tsaTokenExpireLen = 0;
		SecondsToTimeStamp(now + 3599, tsaTokenExpire);

		LONG lMessageSourceID = m_pPopAccount->GetMessageSourceID();

		query.Initialize();
		BINDPARAM_TEXT_STRING(query, aToken);
		BINDPARAM_TEXT_STRING(query, rToken);
		BINDPARAM_TIME(query, tsaTokenExpire);
		BINDPARAM_LONG(query, lMessageSourceID);
		query.Execute(_T("UPDATE MessageSources SET AccessToken=?,RefreshToken=?,AccessTokenExpire=? WHERE MessageSourceID=?"));
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}
	catch (CEMSException eex)
	{
		dca::String f;
		dca::String e(eex.GetErrorString());
		f.Format("CPOP3Connection::RefreshToken() - EMS Exception occured: %d, %s", eex.GetErrorCode(), e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return eex.GetErrorCode();
	}

	return 0;
}