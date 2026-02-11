// WorkerThread.h: interface for the CWorkerThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TICKETMONITORTHREAD_H__3DDE7A3F_379D_48A9_B11A_2B80D2EB49F4__INCLUDED_)
#define AFX_TICKETMONITORTHREAD_H__3DDE7A3F_379D_48A9_B11A_2B80D2EB49F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Thread.h"
#include "MsgRouter.h"
//#include "TicketMonitor.h"
#include "LogWriter.h"
#include "AlertSender.h"
#include "DBMaintenance.h"
#include "Reporter.h"

class CWorkerThread : public CThread  
{
public:
	CWorkerThread();
	virtual ~CWorkerThread();

	virtual unsigned int Run( void );

	void Initialize(void);
	void Uninitialize(void);

protected:
	BOOL DoWork(void);
	bool CheckRegistration(void);
	bool CheckEngine(void);	
	void SendRegAlert( wchar_t* szProduct, int ExpireDay, int ExpireMonth, int ExpireYear,
					   int KeyType, int SrvParamID );
	void FinishJobsInProgress(void);
	void HandleMsg( MSG* msg );
	void HandleODBCError( ODBCError_t* pErr );
	void HandleException( wchar_t* wcsDescription );

	CODBCConn	m_db;
	CODBCQuery	m_query;
	BOOL		m_bInitialized;			// Have we gotten the global parameters?
	BOOL		m_bNeverConnectedToDB;	// Have we ever connected to DB?
	BOOL		m_bRun;					// Keep on running?
	int			m_nConnectToDBFailures;

	CMsgRouter* m_pRouter;
	//CTicketMonitor* m_pMonitor;
	CLogWriter* m_pLogger;
	CAlertSender* m_pAlerter;
	CDBMaintenance* m_pDBMaint;
	CReporter* m_pReporter;	

	DWORD		m_dwLastRegCheck;
	long		m_LastDBCheckIn;	
};


void Log( long ErrorCode, wchar_t* wcsText, ... );
void Log( long ErrorCode, unsigned int StringID, ... );

void Alert( long AlertEventID, long TicketBoxID, wchar_t* wcsText, ... );
void Alert( long AlertEventID, long TicketBoxID, unsigned int StringID, ... );
void CustomAlert( long AlertEventID, long AlertID, wchar_t* wcsText, ... );

#endif // !defined(AFX_TICKETMONITORTHREAD_H__3DDE7A3F_379D_48A9_B11A_2B80D2EB49F4__INCLUDED_)
