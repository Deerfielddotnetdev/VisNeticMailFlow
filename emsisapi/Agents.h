// Agents.h: interface for the CAgents class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "XMLDataClass.h"
#include "TBVInfo.h"

class CAgents : public CXMLDataClass, public TAgents
{
public:
	CAgents( CISAPIData& ISAPIData );
	virtual ~CAgents();

	virtual int Run( CURLAction& action );
private:
	CTBVInfo m_TBView;
protected:
	int ListAll( CURLAction& action );
	int QueryOne( CURLAction& action );
	int ListNTAgents( CURLAction& action );
	int Update( CURLAction& action );
	void DecodeForm(void);
	void GenerateXML(void);
	void Delete(void);
	int AgentPreferences( CURLAction& action, tstring sFormAction = "" );
	void QueryPreferences(void);
	void GeneratePreferencesXML(void);
	void DecodePreferencesForm(void);
	void UpdatePreferences(void);
	void GetAgentViews( list<int>& IDList );
	void GetTicketBoxes( list<int>& IDList );
	int GetDefaultEmailAddress(void);
	void CheckLicense(void);
	void EncryptPassword();
	void DecryptPassword();
	int SetStatus(int StatusID, tstring StatusText);
	int LogoutAgent(int AgentID);
	int ChangeSortOrder( CURLAction& action );
		
	TCHAR m_szDefaultEmailAddress[PERSONALDATA_DATAVALUE_LENGTH];
	long m_szDefaultEmailAddressLen;
	set<unsigned int> m_GroupIDs;
	list<TPersonalData> m_Data;
	list<int> m_AgentViews;
	list<int> m_TicketBoxes;
	int m_MaxPersonalDataID;
	int m_nRefreshLeftPane;
	int m_nSignatureID;
};
