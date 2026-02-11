/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/CustomWaterMarkAlerts.h,v 1.1 2007/10/08 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Custom WaterMark Alerts
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CWaterMarkAlerts : public CXMLDataClass, public TWaterMarkAlerts 
{
public:
	CWaterMarkAlerts( CISAPIData& ISAPIData );
	virtual ~CWaterMarkAlerts();

	virtual int Run( CURLAction& action );

protected:
	void QueryAll(void);
	void QueryOne( CURLAction& action );
	void DecodeForm(void);
	void DeleteAlert(void);
	void ResetHitCount(void);
};