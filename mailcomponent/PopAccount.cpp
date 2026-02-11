// PopAccount.cpp: implementation of the CPopAccount class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PopAccount.h"
#include "md5class.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPopAccount::CPopAccount()
{
	m_sUsername = _T("");
	m_sPassword = _T("");
	m_bIsAPOP = FALSE;
	m_bIsActive = TRUE;
	m_bLeaveOnServer = FALSE;
	m_lCheckFreqMins = 5;
	m_lConnTimeoutSecs = 30;
	m_lPort = 110;
	m_sServerAddress = _T("");
	m_lDestinationID = -1;
	m_lMessageSourceID = -1;
	m_lMaxInboundMsgSize = DEFAULT_MAX_MSG_SIZE_INBOUND;
	m_lMessageSourceTypeID = 1;
	m_bUseReplyTo = 0;
	m_bIsSSL = FALSE;
	m_lErrorCode = 0;
	m_lOAuthHostID = 0;
}

CPopAccount::CPopAccount(const CPopAccount& account)
	: m_bIsActive(account.m_bIsActive),
	m_bIsAPOP(account.m_bIsAPOP),
	m_bLeaveOnServer(account.m_bLeaveOnServer),
	m_lCheckFreqMins(account.m_lCheckFreqMins),
	m_lConnTimeoutSecs(account.m_lConnTimeoutSecs),
	m_lDestinationID(account.m_lDestinationID),
	m_lMessageSourceID(account.m_lMessageSourceID),
	m_lPort(account.m_lPort),
	m_sPassword(account.m_sPassword),
	m_sServerAddress(account.m_sServerAddress),
	m_sDescription(account.m_sDescription),
	m_stLastChecked(account.m_stLastChecked),
	m_sUsername(account.m_sUsername),
	m_lMaxInboundMsgSize(account.m_lMaxInboundMsgSize),
	m_lMessageSourceTypeID(account.m_lMessageSourceTypeID),
	m_bUseReplyTo(account.m_bUseReplyTo),
	m_bIsSSL(account.m_bIsSSL),
	m_lErrorCode(account.m_lErrorCode),
	m_sAccessToken(account.m_sAccessToken),
	m_stAccessTokenExpire(account.m_stAccessTokenExpire),
	m_sRefreshToken(account.m_sRefreshToken),
	m_stRefreshTokenExpire(account.m_stRefreshTokenExpire),
	m_lOAuthHostID(account.m_lOAuthHostID)
{
}

CPopAccount& CPopAccount::operator=(const CPopAccount& account)
{
	if (this != &account)
	{
		m_bIsActive = account.m_bIsActive;
		m_bIsAPOP = account.m_bIsAPOP;
		m_bLeaveOnServer = account.m_bLeaveOnServer;
		m_lCheckFreqMins = account.m_lCheckFreqMins;
		m_lConnTimeoutSecs = account.m_lConnTimeoutSecs;
		m_lDestinationID = account.m_lDestinationID;
		m_lPort = account.m_lPort;
		m_sPassword = account.m_sPassword;
		m_sServerAddress = account.m_sServerAddress;
		m_sDescription = account.m_sDescription;
		m_stLastChecked = account.m_stLastChecked;
		m_sUsername = account.m_sUsername;
		m_lMaxInboundMsgSize = account.m_lMaxInboundMsgSize;
		m_lMessageSourceTypeID = account.m_lMessageSourceTypeID;
		m_bUseReplyTo = account.m_bUseReplyTo;
		m_bIsSSL = account.m_bIsSSL;
		m_lErrorCode = account.m_lErrorCode;
		m_sAccessToken = account.m_sAccessToken;
		m_stAccessTokenExpire = account.m_stAccessTokenExpire;
		m_sRefreshToken = account.m_sRefreshToken;
		m_stRefreshTokenExpire = account.m_stRefreshTokenExpire;
		m_lOAuthHostID = account.m_lOAuthHostID;
	}

	return *this;
}

CPopAccount::~CPopAccount()
{
}

//===========================================================================//
// Author: Mark R.
// Function:
// Generates an MD5 digest string for APOP;
//===========================================================================//
void CPopAccount::GetAPOPDigest(const char* szTimeStamp, char* szDigest)
{
	USES_CONVERSION;

	assert(szTimeStamp);

	// allocate buffers
	char szString[DEFAULT_BUFFER_LENGTH];

	// generate the hash string
	strcpy(szString, szTimeStamp);
	strcat(szString, T2A(m_sPassword.c_str()));

	// generate and return the digest
	CMD5 md5(szString);
	strcpy(szDigest, md5.getMD5Digest());
}
