// PopPollThread.h: interface for the CPopPollThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POPPOLLTHREAD_H__66EFD0D2_A1C2_4AD8_8CF8_FD1BECE5B263__INCLUDED_)
#define AFX_POPPOLLTHREAD_H__66EFD0D2_A1C2_4AD8_8CF8_FD1BECE5B263__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <EMSMutex.h>

class CPopAccount;
class CRetrievalSession;

#include <list>
using namespace std;

// CPopPollThread class
class CPopPollThread  
{
public:
	void CheckNow();
	const UINT Stop();
	const UINT Run();
	CPopPollThread();
	virtual ~CPopPollThread();
protected:
	bool IsDatabaseOrDiskFull();
	UINT	m_nServerID;
	CDBMaintenanceMutex _DBMaintenanceMutex;
private:
	bool AssignWorkToSession(CPopAccount* const pPopAccount);
	bool DrainSessionPool();
	bool FillSessionPool();
	const UINT CheckPopAccounts();
	
	enum 
	{ 
		wait_handle_stop,
		check_right_now
	};
	
	HANDLE m_hWaitHandles[2];
	UINT m_nCheckIntervalSecs;
	typedef list<CRetrievalSession*> SESSION_LIST;
	SESSION_LIST m_sessions;

	UINT m_nFailureMark;
	UINT m_nSuccessMark;

	CWinRegistry m_winReg;
	
	BOOL m_bCheckNow;

	bool m_bLoggedNoAccountsWarning;
	int  m_loggedDiskFullError;

};

#endif // !defined(AFX_POPPOLLTHREAD_H__66EFD0D2_A1C2_4AD8_8CF8_FD1BECE5B263__INCLUDED_)
