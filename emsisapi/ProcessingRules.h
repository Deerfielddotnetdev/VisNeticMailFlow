// ProcessingRules.h: interface for the CProcessingRules class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSINGRULES_H__72AE82FA_2B31_44E3_98A5_36E7D6469030__INCLUDED_)
#define AFX_PROCESSINGRULES_H__72AE82FA_2B31_44E3_98A5_36E7D6469030__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CProcessingRules : public CXMLDataClass, public TProcessingRules  
{
public:
	CProcessingRules( CISAPIData& ISAPIData );
	virtual ~CProcessingRules();

	virtual int Run( CURLAction& action );

protected:
	int ListAll(void);
	int Query(void);
	int Update();
	int Delete();
	int New();
	int ReOrder( CURLAction& action );
	int ResetOrder(void);
	bool TestRegEx(dca::String sRegEx);

	void DecodeForm(void);
	void ResetHitCount(void);
	void GenerateXML(void);
	
	vector<TMatchTextP> m_mt;

	TMatchTextP mt;
	
	vector<TMatchTextP>::iterator mtIter;
	
	int m_DBMatchTextMaxID;
	int m_MaxID;
	int m_SortDir;
	int m_SortBy;
};

#endif // !defined(AFX_PROCESSINGRULES_H__72AE82FA_2B31_44E3_98A5_36E7D6469030__INCLUDED_)
