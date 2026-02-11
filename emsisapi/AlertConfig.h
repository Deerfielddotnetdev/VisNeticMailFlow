// AlertConfig.h: interface for the CAlertConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALERTCONFIG_H__6871A8EB_6923_4AB7_8ECE_FEAD0A7D7F2B__INCLUDED_)
#define AFX_ALERTCONFIG_H__6871A8EB_6923_4AB7_8ECE_FEAD0A7D7F2B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CAlertConfig : public CXMLDataClass  
{
public:
	CAlertConfig( CISAPIData& ISAPIData );
	virtual ~CAlertConfig();

	virtual int Run( CURLAction& action );

protected:
	void QueryOne( TAlerts& alert );
	int ListAll( list<TAlerts>& alerts );
	int DoUpdate( TAlerts& alert );
	int DoDelete( void );
	void DecodeForm( TAlerts& alert );
	void GenerateXML( TAlerts& alert );
	void GenerateListXML( void );

	list<TAlerts> m_Alerts;
};

#endif // !defined(AFX_ALERTCONFIG_H__6871A8EB_6923_4AB7_8ECE_FEAD0A7D7F2B__INCLUDED_)
