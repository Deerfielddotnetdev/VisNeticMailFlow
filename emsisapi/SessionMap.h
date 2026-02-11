#pragma once

#include "MarkupSTL.h"
#include "EMSMutex.h"

class CISAPIData;

#define SESSION_TIMEOUT_MINUTES		20

class CBrowserSession
{
public:
	// Default constructor
	CBrowserSession()
	{
		m_AgentID = 0;
	}
	// Virtual destructor
	~CBrowserSession()
	{
	}

	// Copy constructor
	CBrowserSession( const CBrowserSession& i ) : 
		m_AgentID( i.m_AgentID ), 
		m_LastRequest( i.m_LastRequest ),
		m_IP( i.m_IP ),
		m_UserAgent( i.m_UserAgent ),
		m_RightHandPane( i.m_RightHandPane),
		m_CreateTime ( i.m_CreateTime)
	{
	}

	// Assignment operator
	CBrowserSession& operator=(const CBrowserSession& i) 
	{
		if (this != &i) // trap for assignments to self
		{
			m_AgentID = i.m_AgentID;
			m_LastRequest = i.m_LastRequest;
			m_IP = i.m_IP;
			m_UserAgent = i.m_UserAgent;
			m_RightHandPane = i.m_RightHandPane;
			m_CreateTime = i.m_CreateTime;
		}
		return *this; // return reference to self
	}

	int			 m_AgentID;
	SYSTEMTIME	 m_LastRequest;
	SYSTEMTIME	 m_CreateTime;
	dca::String	 m_IP;
	tstring		 m_UserAgent;
	tstring		 m_RightHandPane;		// URL for right-hand pane
};

typedef map<dca::String,CBrowserSession> BROWSER_SESSION_MAP;

class CAgentSession
{
public:
	// Default constructor
	CAgentSession()
	{
		m_EscalateToAgentID = 0;
		m_RequireGetOldest = 0;
		m_nLoginTestResult = NoTestInProgress;
		m_nMaxRowsPerPage = EMS_DEFAULT_PAGE_ROWS;
		m_bIsDirty = false;
		m_nDBMaintProgress = 0;
		m_IsDeleted = false;
		m_nDBMaintProgress = 0;
		m_id = 0;
		m_TempType = 0;		
	}
	// Virtual destructor
	~CAgentSession()
	{
	}

	// Copy constructor
	CAgentSession( const CAgentSession& i ) : 
		m_CreateTime( i.m_CreateTime ),
		m_LastAutoAway ( i.m_LastAutoAway),
		m_EscalateToAgentID ( i.m_EscalateToAgentID ),
		m_RequireGetOldest ( i.m_RequireGetOldest ),
		m_AgentXML(i.m_AgentXML),
		m_IsAdmin( i.m_IsAdmin ),
		m_UseAutoFill( i.m_UseAutoFill ),
		m_AgentUserName( i.m_AgentUserName ),
		m_nLoginTestResult( i.m_nLoginTestResult),
		m_AgentID( i.m_AgentID ),
		m_nMaxRowsPerPage( i.m_nMaxRowsPerPage ),
		m_bIsDirty( i.m_bIsDirty ),
		m_AgentIP( i.m_AgentIP ),
		m_nDBMaintProgress( i.m_nDBMaintProgress ),
		m_IsDeleted( i.m_IsDeleted ),
		m_id( i.m_id ),
		m_bIsOutboundApprover( i.m_bIsOutboundApprover ),
		m_TempType ( i.m_TempType ),
		m_TempData	( i.m_TempData ),
		m_AgentStatusTypeID	( i.m_AgentStatusTypeID ),
		m_AgentStatusText ( i.m_AgentStatusText ),
		m_AgentAutoStatusTypeID ( i.m_AgentAutoStatusTypeID ),
		m_AgentAutoStatusMin ( i.m_AgentAutoStatusMin ),
		m_LogoutStatusID ( i.m_LogoutStatusID ),
		m_LogoutText ( i.m_LogoutText ),
		m_AutoText ( i.m_AutoText ),
		m_PreAutoID ( i.m_PreAutoID ),
		m_PreAutoText ( i.m_PreAutoText )
	{
	}

	// Assignment operator
	CAgentSession& operator=(const CAgentSession& i) 
	{
		if (this != &i) // trap for assignments to self
		{
			m_AgentID = i.m_AgentID;
			m_CreateTime = i.m_CreateTime;
			m_LastAutoAway = i.m_LastAutoAway;
			m_EscalateToAgentID = i.m_EscalateToAgentID;
			m_RequireGetOldest = i.m_RequireGetOldest;
			m_AgentXML = i.m_AgentXML;
			m_IsAdmin = i.m_IsAdmin;
			m_UseAutoFill = i.m_UseAutoFill;
			m_AgentUserName = i.m_AgentUserName;
			m_nLoginTestResult = i.m_nLoginTestResult;
			m_nMaxRowsPerPage = i.m_nMaxRowsPerPage;
			m_bIsDirty = i.m_bIsDirty;
			m_AgentIP = i.m_AgentIP;
			m_nDBMaintProgress = i.m_nDBMaintProgress;
			m_IsDeleted = i.m_IsDeleted;
			m_id = i.m_id;
			m_bIsOutboundApprover = i.m_bIsOutboundApprover;
			m_TempType = i.m_TempType;
			m_TempData = i.m_TempData;
			m_AgentStatusTypeID = i.m_AgentStatusTypeID;
			m_AgentStatusText = i.m_AgentStatusText;
			m_AgentAutoStatusTypeID = i.m_AgentAutoStatusTypeID;
			m_AgentAutoStatusMin = i.m_AgentAutoStatusMin;
			m_LogoutStatusID = i.m_LogoutStatusID;
			m_LogoutText = i.m_LogoutText;
			m_AutoText = i.m_AutoText;
			m_PreAutoID = i.m_PreAutoID;
			m_PreAutoText = i.m_PreAutoText;
		}
		return *this; // return reference to self
	}

	void Refresh( CODBCQuery& m_query );
	
	enum LoginTestResultsEnum { NoTestInProgress, TestInProgress, TestSuccessful, 
		                        TestFailedConnect, TestFailedAuth, 
								TestFailedAuthNotSupported, TestFailedUnknown };

	int				m_AgentID;				// ID of agent
	SYSTEMTIME		m_CreateTime;			// when this agent session was created
	SYSTEMTIME		m_LastAutoAway;
	int				m_EscalateToAgentID;	// AgentID to escalate tickets to
	unsigned char	m_RequireGetOldest;		// Must this agent use the get oldest button?
	bool			m_IsAdmin;				// Is agent in the adminitrator's group?
	bool			m_UseAutoFill;			// Is agent using email address autofill?
	tstring			m_AgentXML;				// XML for merging with all pages
	tstring			m_AgentUserName;		// Agent login name
	int				m_nLoginTestResult;		// Progress/results of login test
	int				m_nMaxRowsPerPage;		// Max rows per page from agent preferences
	bool			m_bIsDirty;				// Flag indicating that agent session needs to be refreshed
	bool			m_bIsOutboundApprover;	// Flag indicating that agent provides outbound message approvals
	tstring			m_AgentIP;				// IP address of first browser session
	int				m_nDBMaintProgress;		// Used to db maintenance progress/results
	unsigned char	m_IsDeleted;			// Has this agent been deleted after logging in?
	unsigned int    m_id;
	int				m_TempType;				// Temp Data Type
	tstring			m_TempData;				// Temp Data
	int				m_AgentStatusTypeID;	// AgentStatusTypeID
	tstring			m_AgentStatusText;		// Agent Status Text
	int				m_AgentAutoStatusTypeID;// Agent Auto Status Type
	int				m_AgentAutoStatusMin;	// Number of min before setting auto status
	int				m_LogoutStatusID;		// The status to set at agent logout or session timeout
	tstring			m_LogoutText;			// Text to set as status text when agent logs out
	tstring			m_AutoText;				// Text to set as status text when auto status is set
	int				m_PreAutoID;			// The status ID prior to auto status
	tstring			m_PreAutoText;				// The status text prior to auto status
protected:
	TCHAR szLogoutText[AGENTS_STATUS_TEXT_LENGTH];
	long szLogoutTextLen;
	TCHAR szAutoText[AGENTS_STATUS_TEXT_LENGTH];
	long szAutoTextLen;
};

class CAgentStatus
{
public:
	// Default constructor
	CAgentStatus()
	{		
	}
	// Virtual destructor
	~CAgentStatus()
	{
	}

	// Copy constructor
	CAgentStatus( const CAgentStatus& i ) : 
		m_AgentID( i.m_AgentID ),
		m_AgentStatusTypeID	( i.m_AgentStatusTypeID ),
		m_LoggedIn	( i.m_LoggedIn ),
		m_AgentStatusText ( i.m_AgentStatusText )
	{
	}

	// Assignment operator
	CAgentStatus& operator=(const CAgentStatus& i) 
	{
		if (this != &i) // trap for assignments to self
		{
			m_AgentID = i.m_AgentID;			
			m_AgentStatusTypeID = i.m_AgentStatusTypeID;
			m_LoggedIn = i.m_LoggedIn;
			m_AgentStatusText = i.m_AgentStatusText;			
		}
		return *this; // return reference to self
	}

	void Refresh( CODBCQuery& m_query );	

	int				m_AgentID;				// ID of Agent
	int				m_AgentStatusTypeID;	// AgentStatusTypeID
	int				m_LoggedIn;				// Is Agent Logged In
	tstring			m_AgentStatusText;		// Agent Status Text
};

struct MonitorList_t
{
	int nSessionMonitorID;
	int nServerID;
	int nProcessID;
};

class CSessionMap  
{
public:
	CSessionMap();
	virtual ~CSessionMap();

	void AddSession( TCHAR* szSessionID, CAgentSession& session, CBrowserSession& bsession );
	int GetSession( TCHAR* szSessionID, CAgentSession& session, CBrowserSession& bsession, int nTicketBoxList = 0 );
	int GetSessionIDFromIP(dca::String& sClientIP, int nAgentID, dca::String& sSessionID );
	bool RemoveSession( tstring& sSessionID, bool bFromSessionList, int AgentID );
	bool RemoveAllSessionsForAgent( unsigned int nAgentID );
	void CreateNewSessionID( TCHAR* szSessionID );
	void RemoveOldSessions( unsigned int nSessionTimeout );
	bool TimeOutSession( unsigned int AgentID, bool bLockAgent = false );
	void SetLoginTestResults( unsigned int AgentID, int nResults );
	void SetDBMaintResults( unsigned int AgentID, int nResults );
	void GetSessionXML( CMarkupSTL& xmlgen );
	void InvalidateAgentSession( unsigned int AgentID );
	void InvalidateAllAgentSessions( void );
	void QueueSessionMonitors(int nActionID, int nTargetID);
	void RefreshAgentSession( CAgentSession& session, CODBCQuery& m_query, bool bLockAgent = true );
	void GetAgentIP( unsigned int AgentID, tstring& sIP );
	void ResetLocks( CODBCQuery& m_query );
	void UnlockTickets( CODBCQuery& m_query, tstring sAgentIDs = tstring() );
	void FreeReservedTicketIDs( CODBCQuery& m_query, tstring sAgentIDs = tstring() );
	void FreeReservedMsgIDs( CODBCQuery& m_query, tstring sAgentIDs = tstring() );

	bool GetNeedToResetLocks( void ) { return m_bNeedToResetLocks; }
	void SetRightHandPane( tstring& sSessionID, LPCTSTR szURL );
	
	void SetTempType( unsigned int AgentID, int nTempType );
	void SetTempData( unsigned int AgentID, tstring sTempData );
	void GetTempType( unsigned int AgentID, int& nTempType );
	void GetTempData( unsigned int AgentID, tstring& sTempData );

	int GetAgentStatus( unsigned int AgentID, tstring& sStatusText, int& nLoggedIn );
	void SetAgentStatus( unsigned int AgentID, unsigned int nStatusID, tstring sStatusText, bool bLockAgent = true );

	void SaveSessions(void);
	void SyncSessionsWithDB(void);	
	void SaveSessionToDB(const tstring& sessionID, unsigned int AgentID, bool bNewBrowserSession);	
	void LoadStatus(void);

	void DeleteAgentSessionFromDB( unsigned int AgentID );
	void DeleteBrowserSessionFromDB( tstring& sessionID );
	void RemoveOldBrowserSessionsFromDB( void );
	void RemoveOldAgentSessionsFromDB( void );
	void LoadSessionSettings(void);
	void LoadSessionMonitors(void);

	int GetWebSessionsEnabled( void ) { return nWSEnabled; }
	int GetDBSessionsEnabled( void ) { return nDBEnabled; }
	int GetSessionTimeOut( void ) { return nSessionTimeOut; }
	int GetSessionFreq( void ) { return nSessionFreq; }
	int GetStatusFreq( void ) { return nStatusFreq; }
	int GetCacheRefresh( void ) { return nCacheRefresh; }
	int GetDBFreq( void ) { return nDBFreq; }
	int GetServerID( void ) { return nServerID; }
	int GetProcessID( void ) { return nProcessID; }
	int GetQueueFull( void ) { return nQueueFull; }
	int GetQueueSize( void ) { return nQueueSize; }
	int GetCharSet( void ) { return nCharSet; }
	int GetTimeZone( void ) { return nTimeZone; }
	bool GetFullText( void ) { return bFullText; }
	unsigned int GetMutexWait( void ) { return nMutexWait; }

protected:

	void LoadSessions(void);
	void LoadSessionsFromDB(void);
	void LogAgentAction( CODBCQuery& m_query, long nAgentID, long nActionID );
	void CloseDatabase(void);	
	void InsertSessionMonitor(void);
	
	BROWSER_SESSION_MAP m_BrowserMap;
	map<unsigned int,CAgentSession> m_AgentMap;	
	map<unsigned int,CAgentStatus> m_StatusMap;

	list<unsigned int> m_ResetLockList;
	bool m_bNeedToResetLocks;
	
	TCHAR* s_Version;
	CODBCConn	m_db;
	CODBCQuery	m_query;

	int nWSEnabled;
	int nSessionTimeOut;
	int nSessionFreq;
	int nStatusFreq;
	int nDBFreq;
	UINT nCacheRefresh;
	int nDBEnabled;
	UINT nServerID;
	int nProcessID;
	int nQueueFull;
	int nQueueSize;
	int nCharSet;
	int nTimeZone;
	bool bFullText;
	int nSessionExpireAll;
	MonitorList_t m_MonitorList;
	vector<MonitorList_t> vMonList;
	vector<MonitorList_t>::iterator vIter;

	CResetLocksMutex m_ResetLocksMutex;
	CStatusMapMutex m_StatusMapMutex;
	CAgentMapMutex m_AgentMapMutex;
	CBrowserMapMutex m_BrowserMapMutex;
	
	unsigned int nMutexWait;
	
};
