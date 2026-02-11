#pragma once

#include "XMLDataClass.h"

class CMyAlerts : public CXMLDataClass
{
public:
	CMyAlerts( CISAPIData& ISAPIData );
	virtual ~CMyAlerts();

	virtual int Run( CURLAction& action );

protected:
	void QueryAll(void);
	void QueryOne( CURLAction& action );
	void DecodeForm(void);
	void DeleteAlert(void);	

private:
	TAlerts alert;	
};