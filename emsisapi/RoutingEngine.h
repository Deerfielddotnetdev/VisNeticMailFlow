// RoutingEngine.h: interface for the CRoutingEngine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROUTINGENGINE_H__1BC5FBD0_6C28_4614_8149_D592E54F9D9C__INCLUDED_)
#define AFX_ROUTINGENGINE_H__1BC5FBD0_6C28_4614_8149_D592E54F9D9C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <MailStreamEngine.h>


class CRoutingEngine  
{
public:
	CRoutingEngine();
	virtual ~CRoutingEngine();

	BOOL LogIt( int ErrorCode, TCHAR* wcsFormat, ... );
	BOOL LogIt( int ErrorCode, unsigned int nStringID, ... );
	BOOL ReloadConfig( long nConfigItem );

	BOOL ProcessOutboundQueue(void);
	BOOL ProcessInboundQueue(void);

	BOOL TestPOP3Auth( BSTR HostName, long nPort, VARIANT_BOOL IsAPOP, BSTR UserName,
		               BSTR Password, long TimeoutSecs, long isSSL, long* pError );
	BOOL TestSMTPAuth( BSTR HostName, long nPort, BSTR UserName,
		               BSTR Password, long TimeoutSecs, long isSSL, long* pError );
	
	BOOL SendAlert(long AlertEventID, long TicketBoxID, BSTR Text);


	BOOL DoDBMaintenance( long* pError );
	BOOL RestoreArchive( long ArchiveID, long* pError );
	BOOL RestoreBackup( BSTR BackupFile, long* pError );

	BOOL VirusScanFile( LPCTSTR szFile, long Options, tstring& sVirusName, 
		                tstring& sQuarantineLoc, long* pError );
	BOOL VirusScanString( LPCTSTR szString, tstring& sVirusName, 
		                  tstring& sCleanedString, long* pError );

	BOOL GetAVInfo( tstring& sVersion, tstring& sRecordCount, 
				    tstring& sLastUpdate, long* pError );
	
protected:

	BOOL InterfacePointerGood(void);
	BOOL HandleCOMError( HRESULT hres );

	BOOL WriteLogEntry( int ErrorCode, tstring& text );

	


	IRoutingEngineComm* m_pRoutingEngine;

};

#endif // !defined(AFX_ROUTINGENGINE_H__1BC5FBD0_6C28_4614_8149_D592E54F9D9C__INCLUDED_)
