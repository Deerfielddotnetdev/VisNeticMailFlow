// SMTPSession.h: interface for the CSMTPSession class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMTPSESSION_H__0DEEB3FF_AE4E_41F1_A332_0D49B5C75163__INCLUDED_)
#define AFX_SMTPSESSION_H__0DEEB3FF_AE4E_41F1_A332_0D49B5C75163__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSMTPDest;
class CSMTPConnection;

class CSMTPSession  
{
public:
	const int StopSession();
	const int StartSession(CSMTPDest* const pDest);
	CSMTPSession();
	virtual ~CSMTPSession();

public:
	CSMTPDest* m_pSMTPDest;
	CSMTPConnection* m_pSMTPConn;

	HANDLE m_hThreadHandle;
	HANDLE m_hStartHandle;
	UINT m_nThreadID;

	BOOL m_bIsBusy;

};

#endif // !defined(AFX_SMTPSESSION_H__0DEEB3FF_AE4E_41F1_A332_0D49B5C75163__INCLUDED_)
