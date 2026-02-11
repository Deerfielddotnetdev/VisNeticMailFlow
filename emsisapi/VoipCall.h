#pragma once

#include "XMLDataClass.h"

struct ContactList_t
{
	int nContactID;
	TCHAR Name[CONTACTS_NAME_LENGTH];
	long  NameLen;
	TCHAR Email[PERSONALDATA_DATAVALUE_LENGTH];
};

class CVoipCall : public CXMLDataClass
{
public:
	CVoipCall( CISAPIData& ISAPIData );
	virtual ~CVoipCall();

	virtual int Run( CURLAction& action );

protected:

	int List(void);
	int Query(void);
	
	void DecodeForm(void);
	void MakeCall(void);

	int			m_AgentID;
	int			m_ContactID;
	int			m_TicketID;
	int			m_ExtensionID;
	int			m_PersonalDataID;
	int			m_ServerID;
	CEMSString	m_ServerUrl;	
	int			m_ServerPort;
	int			m_Extension;
	TCHAR		m_Pin[51];
	long		m_PinLen;
	int			m_DialCodeID;
	bool		m_IsTicket;
	TCHAR		m_NumToDial[256];
	long		m_NumToDialLen;

	
private:

	void ListTicketContacts( int nTicketID );
	void ListContact( CURLAction& action, int nContactID );
	void GenMultipleContactXML( list<ContactList_t>& List );
	
};