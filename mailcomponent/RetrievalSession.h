// RetrievalSession.h: interface for the CRetrievalSession class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POPSESSION_H__8DA319CB_E871_4F11_9E91_933F3892075F__INCLUDED_)
#define AFX_POPSESSION_H__8DA319CB_E871_4F11_9E91_933F3892075F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPopAccount;
class CPOP3Connection;

class CRetrievalSession  
{
public:
	const int StopSession();
	const int StartSession(CPopAccount* const pPopAccount);
	CRetrievalSession();
	virtual ~CRetrievalSession();

public:
	CPopAccount* m_pPopAccount;
	CPOP3Connection* m_pPopConn;
	HANDLE m_hThreadHandle;
	HANDLE m_hStartHandle;
	UINT m_nThreadID;
	BOOL m_bIsBusy;
};

#endif // !defined(AFX_POPSESSION_H__8DA319CB_E871_4F11_9E91_933F3892075F__INCLUDED_)
