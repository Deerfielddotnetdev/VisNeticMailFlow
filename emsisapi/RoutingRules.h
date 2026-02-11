// RoutingRules.h: interface for the CRoutingRules class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROUTINGRULES_H__72AE82FA_2B31_44E3_98A5_36E7D6469030__INCLUDED_)
#define AFX_ROUTINGRULES_H__72AE82FA_2B31_44E3_98A5_36E7D6469030__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CRoutingRules : public CXMLDataClass, public TRoutingRules  
{
public:
	CRoutingRules( CISAPIData& ISAPIData );
	virtual ~CRoutingRules();

	virtual int Run( CURLAction& action );

protected:
	int ListAll(void);
	int Query(void);
	int Update();
	int Delete();
	int New();
	int ReOrder( CURLAction& action );
	int Options( tstring& sAction );
	int ResetOrder(void);
	bool TestRegEx(dca::String sRegEx);

	void DecodeForm(void);
	void ResetHitCount(void);
	void GenerateXML(void);
	
	vector<TMatchToAddresses> m_mta;
	vector<TMatchFromAddresses> m_mfa;
	vector<TForwardToAddresses> m_fta;
	vector<TForwardCCAddresses> m_fca;
	vector<TAssignToAgents> m_ata;
	vector<TAssignToTicketBoxes> m_attb;
	vector<TMatchText> m_mt;

	TMatchText mt;
	TMatchToAddresses mta;
	TMatchFromAddresses mfa;
	TForwardToAddresses fta;
	TForwardCCAddresses fca;
	TAssignToTicketBoxes attb;
	TAssignToAgents ata;

	vector<TMatchToAddresses>::iterator mtaIter;
	vector<TMatchFromAddresses>::iterator mfaIter;
	vector<TForwardToAddresses>::iterator ftaIter;
	vector<TForwardCCAddresses>::iterator fcaIter;
	vector<TAssignToAgents>::iterator ataIter;
	vector<TAssignToTicketBoxes>::iterator attbIter;
	vector<TMatchText>::iterator mtIter;
	
	int m_DBMatchTextMaxID;
	int m_MaxID;
	TCHAR m_StdRespName[STANDARDRESPONSES_SUBJECT_LENGTH];
	long m_StdRespNameLen;
	int m_SortDir;
	int m_SortBy;
};

#endif // !defined(AFX_ROUTINGRULES_H__72AE82FA_2B31_44E3_98A5_36E7D6469030__INCLUDED_)
