// SMTPConnection.h: interface for the CSMTPConnection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMTPCONNECTION_H__B84FB0A4_DE8C_4482_ABA2_75B17B1832F5__INCLUDED_)
#define AFX_SMTPCONNECTION_H__B84FB0A4_DE8C_4482_ABA2_75B17B1832F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SMTPDest.h"
#include "OutboundMessage.h"
#include <CkString.h>
#include <CKByteData.h>
#include <CkGlobal.h>
#include <CkOAuth2.h>
#include <CkMailMan.h>
#include <CkEmail.h>
#include <CkString.h>
#include <CkStringArray.h>
#include <mimepp/mimepp.h>

// RFC 821 prohibits response lines longer than 512 bytes, 
// and requires that all clients be able to handle 512-byte response lines
#define SMTP_RESPONSE_BUFFER_LENGTH 4096
#define SMTP_COMMAND_BUFFER_LENGTH 512


#include <list>
using namespace std;

class CSMTPConnection  
{
public:
	CSMTPConnection();
	virtual ~CSMTPConnection();

	void Stop();
	void SetMsgDest(CSMTPDest* const pMsgDest);
	const int Authenticate();
	const int AuthenticateOAuth2();
	const int InitConnection();
	const int InitSecureConnection();
	const int SetHello();
	const int DeliverMessages();
	const int Quit();
	const int RefreshToken();

private:
	void BounceMessage(COutboundMessage* const pMsg, CEMSString& errMsg);
	
	void InitTranscript();
	void AppendTranscript(UINT nResID, ...);
	void AppendTranscript(const TCHAR* szText, bool bIsServerComm = false, bool IsServerCommIn = false );

	const int ReopenTicketIfClosed(int nTicketID);
	const int GetWaitingMsgCount(LONG& lCount);
	const int GetSpoolFileSize(COutboundMessage* const pMsg, DWORD& dwSize);
	const int Reset();
	const int SendMsg(COutboundMessage* const pMsg, bool& bSafeCont);
	const int SendMessageSet();
	const int HandleDelFailureAll();
	const int PrepareOutboundMsg(COutboundMessage* const pMsg);
	const int HandleDelSuccess(COutboundMessage* const pMsg);
	const int HandleDelFailure(COutboundMessage* const pMsg);
	const int DelSpoolFile(COutboundMessage* pMsg);
	const int SetLastProcessed();
	const int DeleteMsgFromQueue(COutboundMessage* const pMsg);
	const int SetLastAttemptedDelivery(COutboundMessage* const pMsg);
	const int SendEmail(COutboundMessage* const pMsg);
	const int SendRecipientList(CEmailAddressList& eList, const BOOL bDeliveryConf, CEmailAddressList& rejectList);
	const int UpdateContacts(CEmailAddressList& acceptList, CEmailAddressList& rejectList);
	const int GetCommandResponse(LPCSTR szCmd, const int nLower, const int nUpper, const BOOL bIsSensitive = FALSE);
	const int RcptTo(COutboundMessage* const pMsg);
	const int MailFrom(COutboundMessage* const pMsg, const DWORD dwSpoolFileSize);
	const int UpdateOutboundMsgState(COutboundMessage* const pMsg);
	const int SetSpoolFileGenerated(COutboundMessage* const pMsg, const BOOL bTRUE);
	const int GenerateSpoolFile(COutboundMessage* const pMsg);
	const int ReadAttachmentContent(const tstring& sAttachFile, DwString& sContent);
	const int GetOutboundMsgAttachments();
	const int GetOutboundMsgs(bool& bMoreWaiting);
	const BOOL GetIsResponseGood(LPCSTR szResponse, const UINT nUpper, const UINT nLower);
	const int DeleteMessage(COutboundMessage* const pMsg);
	const int SetErrorCode(LONG nErrorCode);

	void ParseAddress(const wchar_t *addrs, CEmailAddress& eAddr);
	void MergeHeaderFooter(COutboundMessage* pMsg);
	void ClearMessages();	
	void ParseCapabilities(const DwString& sEHLOResponse);
	void ParseAddresses(const wchar_t* addrs, CEmailAddressList& elist);

	void EscapeHTMLAndNewLines( string& str );
	
	CSMTPDest* m_pMsgDest;
	
	CkMailMan m_mailman;
	CkOAuth2 m_oauth2;
	CkGlobal m_glob;

	DwString m_sResponse;
	char m_szCmd[DEFAULT_BUFFER_LENGTH];
	CWinRegistry m_winReg;	

	// session transcript buffer
	tstring m_sSessionTrans;

	typedef list<COutboundMessage*> MSG_CONT;
	MSG_CONT m_OutboundMsgs;

	// data placeholder for determining ESMTP capabilities
	struct _esmtp_capabilities
	{
		BOOL bSmtpAuthMD5;
		BOOL bSmtpAuthLogin;
		BOOL bSize;
		UINT nSize;
		BOOL b8BitMIME;
		BOOL bDsn;

		_esmtp_capabilities::_esmtp_capabilities()
		:	bSmtpAuthMD5(FALSE),			// supports MD5 SMTP auth
			bSmtpAuthLogin(FALSE),			// supports LOGIN SMTP auth
			bSize(FALSE),					// supports SIZE
			nSize(0),						// max message size (if above is true)
			b8BitMIME(FALSE),				// supports 8BITMIME
			bDsn(FALSE)						// supports DSN
			{}
	} m_esmtp_capabilities;
};

#endif // !defined(AFX_SMTPCONNECTION_H__B84FB0A4_DE8C_4482_ABA2_75B17B1832F5__INCLUDED_)
