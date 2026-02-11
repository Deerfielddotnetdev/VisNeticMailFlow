#pragma once

#include <CkSocket.h>
#include <CkString.h>

class CBSession
{
public:
	CBSession()
	{
	}
	
	~CBSession()
	{
	}

	CBSession( const CBSession& i ) : 
		m_LastRequest( i.m_LastRequest ),
		m_AgentID( i.m_AgentID )
	{
	}
	
	CBSession& operator=(const CBSession& i) 
	{
		if (this != &i) 
		{
			m_LastRequest = i.m_LastRequest;
			m_AgentID = i.m_AgentID;
		}
		return *this; 
	}

	SYSTEMTIME	m_LastRequest;
	int			m_AgentID;
};

class CASession
{
public:
	CASession()
	{			
	}
	
	~CASession()
	{
	}

	CASession( const CASession& i ) : 
		m_AgentID( i.m_AgentID ),
		m_LastAuto( i.m_LastAuto ),
		m_StatusID	( i.m_StatusID ),
		m_StatusText ( i.m_StatusText ),		
		m_PreAutoID ( i.m_PreAutoID ),
		m_PreAutoText ( i.m_PreAutoText )
	{
	}

	CASession& operator=(const CASession& i) 
	{
		if (this != &i) 
		{
			m_AgentID = i.m_AgentID;
			m_LastAuto = i.m_LastAuto;
			m_StatusID = i.m_StatusID;
			m_StatusText = i.m_StatusText;			
			m_PreAutoID = i.m_PreAutoID;
			m_PreAutoText = i.m_PreAutoText;
		}
		return *this; 
	}

	int				m_AgentID;				
	SYSTEMTIME		m_LastAuto;				
	int				m_StatusID;	
	tstring			m_StatusText;			
	int				m_PreAutoID;			
	tstring			m_PreAutoText;
};

class WebSessionThread : public CThread
{
public:
	WebSessionThread(void);
	~WebSessionThread(void);

	virtual unsigned int Run( void );

protected:
	virtual void Initialize();
	virtual void Uninitialize();
	void GenAgentXML( void );
	void GenBrowserXML( void );
	void LoadSessionsFromDB(void);
	void AddBrowserSession( dca::String sSessionID, int nAgentID, SYSTEMTIME lastRequest );
	void AddAgentSession( int nAgentID );
	void UpdateBrowserSession( dca::String sSessionID, int nAgentID, SYSTEMTIME lastRequest);
	void UpdateAgentSession( int nAgentID, SYSTEMTIME lastAuto, int nStatusID, tstring sStatusText, int nPreAutoID, tstring sPreAutoText );
    void HandleMsg( MSG* msg );
	void HandleODBCError( ODBCError_t* pErr );
	
	CODBCConn	m_db;
	CODBCQuery	m_query;

	bool		m_bInitialized;

	map<dca::String,CBSession> m_bMap;
	map<unsigned int,CASession> m_aMap;
	map<dca::String,CBSession>::iterator bIter;
	map<unsigned int,CASession>::iterator aIter;
	
	dca::String bXML;
	dca::String aXML;

	int nCommand;
	dca::String sKey;
	dca::String sData;
	int nEnabled;
	int nMasterID;
	UINT nServerID;
	CkSocket listenSocket;
	CkSocket *connectedSocket;
	int nPort;

};