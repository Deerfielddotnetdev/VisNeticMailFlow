// Authenticate.h: interface for the CAuthenticate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTHENTICATE_H__DACAA84D_A403_4369_AEFC_736F14A73FB4__INCLUDED_)
#define AFX_AUTHENTICATE_H__DACAA84D_A403_4369_AEFC_736F14A73FB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAuthenticate : public CXMLDataClass, public TAgents
{
public:
	CAuthenticate( CISAPIData& ISAPIData );
	virtual ~CAuthenticate();

	virtual int Run( CURLAction& action );

	void SetSessionData( void );

	void LogAgentAction( long nAgentID, long nActionID, CEMSString agentIP, CEMSString agentClient );

	enum ReturnCodes { Success, Failed_IPRange, Failed_BadPassword, 
		               Failed_NoSuchUser, Failed_BadForm, Failed_NoKey };
	
protected:
	int AuthenticateUser(void );
	int AuthenticatePassword( void );
	int AuthenticateIP( void );
	int Log( int retval );
	bool VMSAuth(void);
	bool m_bIsOutboundApprover;
	bool VMFServerAuth(void);
	
	TCHAR m_szUsernameParam[AGENTS_LOGINNAME_LENGTH];
	TCHAR m_szPasswordParam[AGENTS_PASSWORD_LENGTH];

	UINT m_LowerRange;
	long m_LowerRangeLen;

	UINT m_UpperRange;
	long m_UpperRangeLen;

	TCHAR szLogoutText[AGENTS_STATUS_TEXT_LENGTH];
	long szLogoutTextLen;
	TCHAR szAutoText[AGENTS_STATUS_TEXT_LENGTH];
	long szAutoTextLen;
	
	TIMESTAMP_STRUCT m_DateSet;			
	long m_DateSetLen;
};

#endif // !defined(AFX_AUTHENTICATE_H__DACAA84D_A403_4369_AEFC_736F14A73FB4__INCLUDED_)
