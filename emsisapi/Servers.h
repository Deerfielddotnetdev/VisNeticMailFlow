#if !defined(AFX_SERVERS_H__C1E99A69_5503_4C44_AE93_7D4402430C92__INCLUDED_)
#define AFX_SERVERS_H__C1E99A69_5503_4C44_AE93_7D4402430C92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CServers : public CXMLDataClass, public TServers
{
public:
	CServers( CISAPIData& ISAPIData );
	virtual ~CServers();

	virtual int Run( CURLAction& action );

protected:

	int ListAll(void);
	int Query(void);
	void UpdateSettings();
	void UpdateServerTasks(void);
	int Delete();
	int New();
	
	void StopService(void);
	void StartService(void);
	
	void DecodeForm(void);
	void GenerateXML(void);
	void CheckForDuplicity( void );
	void GenerateRegistrationXML( int nServerId );
	void AddUserSize( long nUserSize );
	void VerifyDirectoryExists( tstring& sFolder );

	int m_nMasterServerID;
	int m_nServerType;
	int nAlertSender;
	int nDBMaint;
	int nDBMon;
	int nMsgCol;
	int nMsgRtr;
	int nMsgSnd;
	int nRpt;
	int nTktMon; 
	bool bIsNew;

	tstring sInboundAttachFolder;
	tstring sOutboundAttachFolder;
	tstring sNoteAttachFolder;
	tstring sBackupFolder;
	tstring sArchiveFolder;
	tstring sDescription;
	tstring sRegistrationKey;
			
};

#endif // !defined(AFX_SERVERS_H__C1E99A69_5503_4C44_AE93_7D4402430C92__INCLUDED_)