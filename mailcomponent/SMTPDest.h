// SMTPDest.h: interface for the CSMTPDest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMTPDEST_H__6699EABF_A032_4057_90D7_AEFD6CDB4B40__INCLUDED_)
#define AFX_SMTPDEST_H__6699EABF_A032_4057_90D7_AEFD6CDB4B40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSMTPDest  
{
public:
	CSMTPDest();
	virtual ~CSMTPDest();

	CSMTPDest(const CSMTPDest&);
	CSMTPDest& operator = (const CSMTPDest&);

	bool operator==(const CSMTPDest& c)				{ return c.m_lMessageDestID == this->m_lMessageDestID; }

	void SetServerAddress(const tstring& sVal)		{ m_sServerAddress = sVal; }
	void SetDescription(const tstring& sVal)		{ m_sDescription = sVal; }
	void SetUsesSMTPAuth(const BOOL bVal)			{ m_bUsesSMTPAuth = bVal; }
	void SetAuthUser(const tstring& sVal)			{ m_sAuthUser = sVal; }
	void SetAuthPass(const tstring& sVal)			{ m_sAuthPass = sVal; }
	void SetSMTPPort(const UINT nVal)				{ m_nSMTPPort = nVal; }
	void SetConnTimeoutSecs(const UINT nVal)		{ m_nConnTimeoutSecs = nVal; }
	void SetLastProcessed(const SYSTEMTIME& stVal)	{ m_stLastProcessed = stVal; }
	void SetProcessFreqMins(const UINT nVal)		{ m_nProcessFreqMins = nVal; }
	void SetMessageDestID(const LONG lVal)			{ m_lMessageDestID = lVal; }
	void SetMaxOutboundMsgSize(const LONG lVal)		{ m_lMaxOutboundMsgSize = lVal; }
	void SetMaxSendRetryHours(const LONG lVal)		{ m_lMaxSendRetryHours = lVal; }
	void SetIsSSL(const BOOL bVal)					{ m_bIsSSL = bVal; }
	void SetSslMode(const LONG lVal)				{ m_lSslMode = lVal; }
	void SetErrorCode(const LONG lVal)				{ m_lErrorCode = lVal; }
	void SetAccessToken(const tstring& sVal)		{ m_sAccessToken = sVal; }
	void SetATokenExpire(const SYSTEMTIME& stVal)	{ m_stAccessTokenExpire = stVal; }
	void SetRefreshToken(const tstring& sVal)		{ m_sRefreshToken = sVal; }
	void SetRTokenExpire(const SYSTEMTIME& stVal)	{ m_stRefreshTokenExpire = stVal; }
	void SetOAuthHostID(const LONG lVal)			{ m_lOAuthHostID = lVal; }

	const tstring& GetServerAddress()				{ return m_sServerAddress; }
	const tstring& GetDescription()					{ return m_sDescription; }
	const BOOL GetUsesSMTPAuth()					{ return m_bUsesSMTPAuth; }
	const tstring& GetAuthUser()					{ return m_sAuthUser; }
	const tstring& GetAuthPass()					{ return m_sAuthPass; }
	const UINT GetSMTPPort()						{ return m_nSMTPPort; }
	const UINT GetConnTimeoutSecs()					{ return m_nConnTimeoutSecs; }
	const SYSTEMTIME& GetLastProcessed()			{ return m_stLastProcessed; }
	const UINT GetProcessFreqMins()					{ return m_nProcessFreqMins; }
	const LONG GetMessageDestID()					{ return m_lMessageDestID; }
	const LONG GetMaxOutboundMsgSize()				{ return m_lMaxOutboundMsgSize; }
	const LONG GetMaxSendRetryHours()				{ return m_lMaxSendRetryHours; }
	const BOOL GetIsSSL()							{ return m_bIsSSL; }
	const LONG GetSslMode()							{ return m_lSslMode; }
	const LONG GetErrorCode()						{ return m_lErrorCode; }
	const tstring& GetAccessToken()	const			{ return m_sAccessToken; }
	const SYSTEMTIME& GetAccessTokenExpire() const	{ return m_stAccessTokenExpire; }	
	const tstring& GetRefreshToken()	const		{ return m_sRefreshToken; }
	const SYSTEMTIME& GetRefreshTokenExpire() const	{ return m_stRefreshTokenExpire; }
	const LONG GetOAuthHostID()	const				{ return m_lOAuthHostID; }

private:
	tstring m_sServerAddress;
	tstring m_sDescription;
	BOOL m_bUsesSMTPAuth;
	tstring m_sAuthUser;
	tstring m_sAuthPass;
	UINT m_nSMTPPort;
	UINT m_nConnTimeoutSecs;
	SYSTEMTIME m_stLastProcessed;
	UINT m_nProcessFreqMins;
	LONG m_lMessageDestID;		// from database
	LONG m_lMaxOutboundMsgSize;
	LONG m_lMaxSendRetryHours;
	BOOL m_bIsSSL;
	LONG m_lSslMode;
	LONG m_lErrorCode;
	tstring m_sAccessToken;
	SYSTEMTIME m_stAccessTokenExpire;
	tstring m_sRefreshToken;
	SYSTEMTIME m_stRefreshTokenExpire;
	LONG m_lOAuthHostID;
};

#endif // !defined(AFX_SMTPDEST_H__6699EABF_A032_4057_90D7_AEFD6CDB4B40__INCLUDED_)
