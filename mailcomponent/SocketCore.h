#if !defined(AFX_SOCKETCORE_H__CD5DF715_0CF7_40DE_80B4_B78249078FBF__INCLUDED_)
#define AFX_SOCKETCORE_H__CD5DF715_0CF7_40DE_80B4_B78249078FBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SocketCore.h : header file

/////////////////////////////////////////////////////////////////////////////
// CSocketCore window

class CSocketCore
{
// Construction
public:
	CSocketCore();
	virtual	~CSocketCore();

// Attributes
protected:
	int				m_nRet;					// generic return value (re-usable)
	SOCKET			m_sockActive;			// active socket
	SOCKADDR_IN		m_sockaddrActive;		// socket address
	HANDLE			m_hSockHandles[2];		// socket handles

	// socket handle events
	enum local_events
	{
		local_event_stop,
		local_event_socket,
		local_gethostbyname
	};

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSocketCore)
	//}}AFX_VIRTUAL

// Implementation
public:
	int				ReadLine(char* buff, const int bufLen, int& nRead, const UINT nTimeout = 20);
	virtual void	Close();
	virtual int		Recv(char* buff, const int nBuffLen, const UINT nRecvTimeout = 20);
	virtual int		Send(char* buff, const int nBuffLen, const UINT nTransmitTimeout = 20);			// loops and calls SendBytes
	virtual int		Connect(LPCTSTR sHost, const int nPort, const UINT nTimeoutSecs);
	virtual int		CreateSocket();
	virtual	void	Stop();							// Aborts any blocking wait in progress

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOCKETCORE_H__CD5DF715_0CF7_40DE_80B4_B78249078FBF__INCLUDED_)
