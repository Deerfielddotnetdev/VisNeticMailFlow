// VirusScanningThread.h: interface for the CVirusScanningThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "Thread.h"
#include "KAVScan.h"


class CVirusScanningThread : public CThread  
{
public:
	CVirusScanningThread();
	virtual ~CVirusScanningThread();

	virtual unsigned int Run( void );

	static dca::String m_sVersion;
	static dca::String m_sRecordCount;
	static dca::String m_sLastUpdate;
	
protected:
	CKAVScan	m_scanner;

	BOOL		m_bInitialized;

	void Scan( VirusScanInfo* pVSInfo );
	BOOL Initialize(void);
	void UnInitialize(void);
	void Log( int Code, LPCWSTR szMsg );

};
