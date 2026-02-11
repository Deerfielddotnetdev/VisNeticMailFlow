// PopAccount.h: interface for the CPopAccount class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POPACCOUNT_H__956D136E_B934_47F4_9B15_11A469E0140F__INCLUDED_)
#define AFX_POPACCOUNT_H__956D136E_B934_47F4_9B15_11A469E0140F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

using namespace std;

class CPopAccount  
{
public:
	void GetAPOPDigest(const char* szTimeStamp, char* szDigest);
	CPopAccount();
	virtual ~CPopAccount();
	
	CPopAccount(const CPopAccount&);
	CPopAccount& operator = (const CPopAccount&);

	bool operator==(const CPopAccount& c)			{ return c.m_lMessageSourceID == this->m_lMessageSourceID; }

// accessors
public:
	const tstring& GetUserName()	const			{ return m_sUsername; }
	const tstring& GetPassword()	const			{ return m_sPassword; }
	const tstring& GetServerAddress()	const		{ return m_sServerAddress; }
	const tstring& GetDescription()	const			{ return m_sDescription; }
	const BOOL GetIsAPOP()	const					{ return m_bIsAPOP; }
	const BOOL GetLeaveOnServer()	const			{ return m_bLeaveOnServer; }
	const BOOL GetIsActive() const					{ return m_bIsActive; }
	const LONG GetPort() const						{ return m_lPort; }
	const LONG GetCheckFreqMins() const				{ return m_lCheckFreqMins; }
	const LONG GetConnTimeoutSecs() const			{ return m_lConnTimeoutSecs; }
	const SYSTEMTIME& GetLastChecked() const		{ return m_stLastChecked; }
	const LONG GetDestinationID() const				{ return m_lDestinationID; }
	const LONG GetMessageSourceID() const			{ return m_lMessageSourceID; }
	const LONG GetMaxInboundMsgSize() const			{ return m_lMaxInboundMsgSize; }
	const LONG GetMessageSourceTypeID() const		{ return m_lMessageSourceTypeID; }
	const LONG GetUseReplyTo() const                { return m_bUseReplyTo; }
	const BOOL GetIsSSL()	const					{ return m_bIsSSL; }
	const LONG GetZipAttach() const					{ return m_lZipAttach; }
	const LONG GetDupMsg() const					{ return m_lDupMsg; }
	const LONG GetLeaveCopiesDays() const			{ return m_lLeaveCopiesDays; }
	const LONG GetSkipDownloadDays() const			{ return m_lSkipDownloadDays; }
	const LONG GetDateFilters() const				{ return m_lDateFilters; }
	const LONG GetErrorCode() const					{ return m_lErrorCode; }
	const vector<DateFilters_t> GetDFVector() const	{ return m_df; }
	const tstring& GetAccessToken()	const			{ return m_sAccessToken; }
	const SYSTEMTIME& GetAccessTokenExpire() const	{ return m_stAccessTokenExpire; }	
	const tstring& GetRefreshToken()	const		{ return m_sRefreshToken; }
	const SYSTEMTIME& GetRefreshTokenExpire() const	{ return m_stRefreshTokenExpire; }
	const LONG GetOAuthHostID()	const				{ return m_lOAuthHostID; }
		
	void SetUserName(const tstring& sVal)			{ m_sUsername = sVal; }
	void SetPassword(const tstring& sVal)			{ m_sPassword = sVal; }
	void SetServerAddress(const tstring& sVal)		{ m_sServerAddress = sVal; }
	void SetDescription(const tstring& sVal)		{ m_sDescription = sVal; }
	void SetIsAPOP(const BOOL bVal)					{ m_bIsAPOP = bVal; }
	void SetLeaveOnServer(const BOOL bVal)			{ m_bLeaveOnServer = bVal; }
	void SetIsActive(const BOOL bVal)				{ m_bIsActive = bVal; }
	void SetPort(const BOOL nVal)					{ m_lPort = nVal; }
	void SetCheckFreqMins(const LONG lVal)			{ m_lCheckFreqMins = lVal; }
	void SetConnTimeoutSecs(const LONG lVal)		{ m_lConnTimeoutSecs = lVal; }
	void SetLastChecked(const SYSTEMTIME& stVal)	{ m_stLastChecked = stVal; }
	void SetDestinationID(const LONG lVal)			{ m_lDestinationID = lVal; }
	void SetMessageSourceID(const LONG lVal)		{ m_lMessageSourceID = lVal; }
	void SetMaxInboundMsgSize(const LONG lVal)		{ m_lMaxInboundMsgSize = lVal; }
	void SetMessageSourceTypeID(const LONG lVal)	{ m_lMessageSourceTypeID = lVal; }
	void SetUseReplyTo(const BOOL bVal)             { m_bUseReplyTo = bVal; }
	void SetIsSSL(const BOOL bVal)					{ m_bIsSSL = bVal; }
	void SetZipAttach(const LONG lVal)				{ m_lZipAttach = lVal; }
	void SetDupMsg(const LONG lVal)					{ m_lDupMsg = lVal; }
	void SetLeaveCopiesDays(const LONG lVal)		{ m_lLeaveCopiesDays = lVal; }
	void SetSkipDownloadDays(const LONG lVal)		{ m_lSkipDownloadDays = lVal; }
	void SetDateFilters(const LONG lVal)			{ m_lDateFilters = lVal; }
	void SetErrorCode(const LONG lVal)				{ m_lErrorCode = lVal; }
	void SetDFVector(vector<DateFilters_t> vVal)	{ m_df = vVal; }
	void SetAccessToken(const tstring& sVal)		{ m_sAccessToken = sVal; }
	void SetATokenExpire(const SYSTEMTIME& stVal)	{ m_stAccessTokenExpire = stVal; }
	void SetRefreshToken(const tstring& sVal)		{ m_sRefreshToken = sVal; }
	void SetRTokenExpire(const SYSTEMTIME& stVal)	{ m_stRefreshTokenExpire = stVal; }
	void SetOAuthHostID(const LONG lVal)			{ m_lOAuthHostID = lVal; }
	
private:
	tstring m_sUsername;
	tstring m_sPassword;
	tstring m_sServerAddress;
	tstring m_sDescription;
	BOOL m_bIsAPOP;
	BOOL m_bLeaveOnServer;
	BOOL m_bIsActive;
	LONG m_lPort;
	LONG m_lCheckFreqMins;
	LONG m_lConnTimeoutSecs;
	SYSTEMTIME m_stLastChecked;
	LONG m_lMessageSourceID;
	LONG m_lDestinationID;
	LONG m_lMaxInboundMsgSize;
	LONG m_lMessageSourceTypeID;
	LONG m_bUseReplyTo;
	BOOL m_bIsSSL;
	LONG m_lZipAttach;
	LONG m_lDupMsg;
	LONG m_lLeaveCopiesDays;
	LONG m_lSkipDownloadDays;
	LONG m_lDateFilters;
	LONG m_lErrorCode;
	tstring m_sAccessToken;
	SYSTEMTIME m_stAccessTokenExpire;
	tstring m_sRefreshToken;
	SYSTEMTIME m_stRefreshTokenExpire;
	LONG m_lOAuthHostID;
	vector<DateFilters_t> m_df;
};

#endif // !defined(AFX_POPACCOUNT_H__956D136E_B934_47F4_9B15_11A469E0140F__INCLUDED_)
