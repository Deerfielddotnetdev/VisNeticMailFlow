#pragma once
#include "xmldataclass.h"

class CDemo :
	public CXMLDataClass, public TAgents
{
public:
	CDemo(CISAPIData& ISAPIData );
	virtual ~CDemo(void);

	int virtual Run( CURLAction& action );

private:
	TCHAR m_PasswordConfirm[AGENTS_PASSWORD_LENGTH];
	void DecodeForm(void);
	void GenerateXML(void);
	void CreateAgent(CURLAction& action);
	void AddSupportTicket(void);
	void AddSalesTicket(void);
	void AddInfoTicket(void);
	void GetBodyFromFile( LPCTSTR szFile, TCHAR** ppBuffer, long& nBytesAllocated );
	void GetDemoMsgPath(tstring& sPath );
	void EncryptPassword(void);
	void DecryptPassword(void);
};
