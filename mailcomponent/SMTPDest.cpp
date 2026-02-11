// SMTPDest.cpp: implementation of the CSMTPDest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SMTPDest.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSMTPDest::CSMTPDest()
{
	m_bUsesSMTPAuth = FALSE;
	m_bIsSSL = FALSE;
	m_nSMTPPort = 25;
	m_nConnTimeoutSecs = 25;
	m_nProcessFreqMins = 20;
	m_lMessageDestID = 0;
	m_lMaxOutboundMsgSize = DEFAULT_MAX_MSG_SIZE_OUTBOUND;
	m_lMaxSendRetryHours = DEFAULT_MAX_SEND_RETRY_HOURS;
	m_lSslMode = 1;
	m_lErrorCode = 0;
	m_lOAuthHostID = 0;
}


CSMTPDest::CSMTPDest(const CSMTPDest& dest)
:	m_bUsesSMTPAuth(dest.m_bUsesSMTPAuth),
	m_bIsSSL(dest.m_bIsSSL),
	m_lMessageDestID(dest.m_lMessageDestID),
	m_nConnTimeoutSecs(dest.m_nConnTimeoutSecs),
	m_nProcessFreqMins(dest.m_nProcessFreqMins),
	m_nSMTPPort(dest.m_nSMTPPort),
	m_sAuthPass(dest.m_sAuthPass),
	m_sAuthUser(dest.m_sAuthUser),
	m_sServerAddress(dest.m_sServerAddress),
	m_sDescription(dest.m_sDescription),
	m_stLastProcessed(dest.m_stLastProcessed),
	m_lMaxSendRetryHours(dest.m_lMaxSendRetryHours),
	m_lMaxOutboundMsgSize(dest.m_lMaxOutboundMsgSize),
	m_lSslMode(dest.m_lSslMode),
	m_lErrorCode(dest.m_lErrorCode),
	m_sAccessToken(dest.m_sAccessToken),
	m_stAccessTokenExpire(dest.m_stAccessTokenExpire),
	m_sRefreshToken(dest.m_sRefreshToken),
	m_stRefreshTokenExpire(dest.m_stRefreshTokenExpire),
	m_lOAuthHostID(dest.m_lOAuthHostID)
{
}

CSMTPDest& CSMTPDest::operator=(const CSMTPDest& dest)
{
    if (this != &dest)
	{
        m_bUsesSMTPAuth		= dest.m_bUsesSMTPAuth;
        m_bIsSSL			= dest.m_bIsSSL;
        m_lMessageDestID	= dest.m_lMessageDestID;
        m_nConnTimeoutSecs	= dest.m_nConnTimeoutSecs;
		m_nProcessFreqMins	= dest.m_nProcessFreqMins;
        m_nSMTPPort			= dest.m_nSMTPPort;
        m_sAuthPass			= dest.m_sAuthPass;
		m_sAuthUser			= dest.m_sAuthUser;
		m_sServerAddress	= dest.m_sServerAddress;
		m_sDescription		= dest.m_sDescription;
		m_stLastProcessed	= dest.m_stLastProcessed;
		m_lMaxSendRetryHours = dest.m_lMaxSendRetryHours;
		m_lMaxOutboundMsgSize = dest.m_lMaxOutboundMsgSize;
		m_lSslMode			= dest.m_lSslMode;
		m_lErrorCode		= dest.m_lErrorCode;
		m_sAccessToken		= dest.m_sAccessToken;
		m_stAccessTokenExpire = dest.m_stAccessTokenExpire;
		m_sRefreshToken		= dest.m_sRefreshToken;
		m_stRefreshTokenExpire = dest.m_stRefreshTokenExpire;
		m_lOAuthHostID = dest.m_lOAuthHostID;
    }
	
    return *this;
}


CSMTPDest::~CSMTPDest()
{

}
