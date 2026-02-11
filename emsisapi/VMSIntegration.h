// VMSIntegration.h: interface for the CVMSIntegration class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VMSINTEGRATION_H__44275691_83D8_4693_8CA7_09CA97F6C207__INCLUDED_)
#define AFX_VMSINTEGRATION_H__44275691_83D8_4693_8CA7_09CA97F6C207__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CVMSIntegration : public CXMLDataClass  
{
public:
	CVMSIntegration(CISAPIData& ISAPIData);
	virtual ~CVMSIntegration();

	int virtual Run( CURLAction& action );

protected:

	inline void UpdateConfig(void);
	inline void GenConfigXML(void);
	
	int VMSAgents(void);
};

#endif // !defined(AFX_VMSINTEGRATION_H__44275691_83D8_4693_8CA7_09CA97F6C207__INCLUDED_)
