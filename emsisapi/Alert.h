/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Alert.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

// only include once
#pragma once

#include "XMLDataClass.h"

class CAlert : public CXMLDataClass, public TAlertMsgs 
{
public:
	CAlert(CISAPIData& ISAPIData);
	virtual ~CAlert() {};

	int Run(CURLAction& action);

private:
	inline void Query( void );
	inline void Delete( void );
	inline void MarkAsRead( void );
	inline void GenerateXML( void );

private:
	bool m_bRefreshRequired;
};
