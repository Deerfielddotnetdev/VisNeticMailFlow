#pragma once

#include <EMSMutex.h>
#include "TicketMonitor.h"
#include <CkMailMan.h>
#include <CkEmail.h>
#include <CkStringArray.h>
#include <CkGlobal.h>

class DBMonitorThread : public CThread
{
public:
	DBMonitorThread(void);
	~DBMonitorThread(void);

	virtual unsigned int Run( void );

protected:
	virtual void Initialize();
	virtual void Flush();
	virtual void Uninitialize();
	int TryAgentTicketCategory();
	void GetAgentTicketCategory();
	int CheckInToDB();
	int ProcessDBCheckIn();
	int MailCheck();
	int GetEmailsToVerify();
	int VerifyEmails();
	int ProcessEmails();
	void GetServerParameters(void);
	void GetServerParameter( int ID, CEMSString& sValue );
	
	CTicketMonitor* m_pMonitor;

	enum { AgentTicketCategoryCheckMs = 180000, DBCheckInMs = 10000, MailCheckMs = 60000, TicketMonitorMs = 20000, CheckContactEmailsMs = 60000 };	
	long	m_LastAgentTicketCategoryCheck;
	long	m_LastDBCheckIn;
	long	m_LastMailCheck;
	long	m_LastTicketMonitorRun;
	long	m_LastEmailVerifyRun;
	int		m_EmailVerifyDays;
	int		m_EnableVerification;

	vector<PersonalData_t> m_pd;
	vector<PersonalData_t>::iterator iter;

	tstring sSystemEmail;
	tstring sHeloHostName;
	int nNoMx;
	int nCatchAll;
	int nRemove;
	int nDelete;
	int nConnectTimeout;

	dca::Event	        _event;
	CDBMaintenanceMutex _DBMaintenanceMutex;

	CODBCConn	m_db;
	CODBCQuery	m_query;

	CEMSString  m_sArchiveFolder;
	CEMSString  m_sBackupFolder;
	CEMSString  m_sLogFolder;
	CEMSString  m_sReportFolder;
	CEMSString  m_sInboundAttachFolder;
	CEMSString  m_sOutboundAttachFolder;
	CEMSString  m_sNoteAttachFolder;
	CEMSString  m_sSpoolFolder;
	CEMSString  m_sTempFolder;
	CEMSString  m_sQuarantineFolder;
	int			m_nNextMaintHigh;
	int			m_nNextMaintLow;

	BOOL		m_bInitialized;			// Have we connected to the DB yet?

};
