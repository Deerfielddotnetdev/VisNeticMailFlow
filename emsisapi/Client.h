// Client.h: interface for the CClient class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "AlertList.h"

class CClient :  public CPagedList  
{
public:
	CClient(CISAPIData& ISAPIData);
	virtual ~CClient() {};

	int Run(CURLAction& action);
	
private:
	void List( void );
	void GenerateXML( TAlertMsgs* pAlertArray);
	void FormatQueryString(CEMSString& sQuery);
	void DeleteAlerts( void );
	void DeleteAllAlerts( void );
	void ContactSearch( void );
	void ContactDetails( void );
	void GenContactXML( void );
	void TicketSearch( CURLAction& action );

private:
	unsigned m_Access;
	int m_nUnreadCount;
	int m_nTotalCount;
	int m_nMaxAlertMsgID;
	int m_nMaxID;
	int m_nAgentID;
	int m_nContactID;
	int m_nTicketID;
	CEMSString m_sPersonalData;

	TCHAR m_szUsernameParam[AGENTS_LOGINNAME_LENGTH];
	TCHAR m_szPasswordParam[AGENTS_PASSWORD_LENGTH];

	unsigned char m_AccessLevel;
	int m_nMaxPersonalDataID;
	int m_nMaxFormPersonalDataID;

	TContacts m_Contact;
	CPersonalDataList m_DefaultEmail;
	map<int, CPersonalDataList> m_PersonalDataMap;

};
