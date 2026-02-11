#ifndef DBMAINTTHREAD_HEADER
#define DBMAINTTHREAD_HEADER

#include "DBMaintenance.h"

class DBMaintThread : public CThread  
{
public:
	DBMaintThread();
	virtual ~DBMaintThread();

	virtual unsigned int Run( void );

	void Initialize(void);
	void Uninitialize(void);

protected:
	BOOL DoWork(void);
	void HandleMsg( MSG* msg );
	void HandleODBCError( ODBCError_t* pErr );
	void HandleException( wchar_t* wcsDescription );

	CODBCConn	m_db;
	CODBCQuery	m_query;
	BOOL		m_bInitialized;			// Have we gotten the global parameters?
	BOOL		m_bNeverConnectedToDB;	// Have we ever connected to DB?
	BOOL		m_bRun;					// Keep on running?
	int			m_nConnectToDBFailures;

	CDBMaintenance* m_pDBMaint;		
};
#endif