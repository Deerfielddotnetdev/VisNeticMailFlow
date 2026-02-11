// POP3Connection.h: interface for the CPOP3Connection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POP3CONNECTION_H__3F537584_BDB2_40A6_A367_5FA431E7EA74__INCLUDED_)
#define AFX_POP3CONNECTION_H__3F537584_BDB2_40A6_A367_5FA431E7EA74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PopAccount.h"
#include <mimepp/mimepp.h>
#include "InboundMessage.h"
#include "WinRegistry.h"
#include <CkGlobal.h>
#include <CkOAuth2.h>
#include <CkImap.h>
#include <CkMessageSet.h>
#include <CkEmailBundle.h>
#include <CkEmail.h>
#include <CkString.h>
#include <CkStringArray.h>
#include <CkMailMan.h>
#include <CkDateTime.h>

using namespace std;
#include <map>

class CPOP3Connection  
{
public:
	CPOP3Connection();
	virtual ~CPOP3Connection();

	void Stop();
	const int InitConnection();
	const int Authenticate();
	const int AuthenticateOAuth2POP3();
	const int AuthenticateOAuth2IMAP();	
	const int SetPopAccount(CPopAccount* pPopAccount);
	const int CheckAccount();
	const int Quit();
	const int InitSecureConnection();
	const int RefreshToken();
	
private:
	const int CleanUIDLData(vector<UINT>& todelete, CODBCQuery& query);
	const int GetMsgSizeByIndex(const int nIndex, int& nMsgSize);
	const int ExtractMsgSize(const char* szBuffer);
	const BOOL ContainsTermSequence(const char* szBuffer);
	void ClearMsgInfo();
	const int PruneUIDLListing();
	const int PruneIMAPUIDLListing(CkEmailBundle* bundle);
	const int GetCommandResponse(LPCSTR szCmd, const BOOL bIsSensitive = FALSE, const BOOL bLongRespTimeout = FALSE);
	const int GetMsgList();
	struct _msg_info
	{
		tstring sUIDLString;
		int nMsgSizeBytes;
		bool bIsOld;

		_msg_info::_msg_info()
		{
			sUIDLString = _T("");
			nMsgSizeBytes = 0;
			bIsOld = false;
		}

		bool operator== (const _msg_info &rhs) const { return nMsgSizeBytes == rhs.nMsgSizeBytes; }
		bool operator< (const _msg_info &rhs) const { return nMsgSizeBytes < rhs.nMsgSizeBytes; }
	};

	bool m_bServerSupportsUIDL;

	const int GetMsg(DwString& sMsg, CInboundMessage& message, const _msg_info& msginfo, const UINT nMsgIndex);
	const int GetIMAPMsg(DwString& sMsg, CInboundMessage& message, const _msg_info& msginfo, const UINT nMsgIndex);
	const int RetrTop(DwString& sMsg, const char* sUidl);

	const int RetrMsgToFile(const char* sUidl, tstring& sFileName);
	const int RetrIMAPMsgToFile(const UINT nMsgID, tstring& sFileName);
	const int SetLastChecked();
	const int SetErrorCode(LONG nErrorCode);
	const int SaveUIDL(LPCTSTR sMsgID, BOOL bIsPartial);
	const int DeleteMsgByID(const UINT nMsgIndexNum);
	const int DeleteIMAPMsgByID(const UINT nMsgIndexNum);
	const int RetrMsg(DwString& sMsg, const UINT nMsgID);
	const int GetMultiLineResponse(DwString& sMultiLine);
	const int GetWaitingMsgs();
	const int GetWaitingIMAPMsgs(CkEmailBundle* bundle);
	//const int GetUIDLListing();
	const BOOL GetIsResponseGood(LPCSTR szResponse);
	bool IsMsgDup(TCHAR* szMsgID, int nMessageSourceID);
	const int GetGreetingTimestamp(LPCSTR szGreeting, char* szTimeStamp);
	CPopAccount* m_pPopAccount;
	CkImap m_imap;
	CkMailMan m_mailman;
	CkOAuth2 m_oauth2;
	CkGlobal m_glob;
	CkEmailBundle* popBundle;
	char m_szResponse[LARGE_BUFFER_LENGTH];
	char m_szCmd[DEFAULT_BUFFER_LENGTH];
	char m_szOAuth2Cmd[LARGE_BUFFER_LENGTH];
	char m_szOAuth2Response[LARGE_BUFFER_LENGTH];
	
	string m_sInitialServerResponse;
	
	typedef map<int, _msg_info*> MAP_MSG_INFO;
	MAP_MSG_INFO m_mapMsgInfo;

	CWinRegistry m_winReg;
};

#endif // !defined(AFX_POP3CONNECTION_H__3F537584_BDB2_40A6_A367_5FA431E7EA74__INCLUDED_)
