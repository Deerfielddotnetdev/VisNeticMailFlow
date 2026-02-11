/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/AutoResponses.h,v 1.1 2007/10/08 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Auto Responses
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CAutoResponses : public CXMLDataClass, public TAutoResponses 
{
public:
	CAutoResponses( CISAPIData& ISAPIData );
	virtual ~CAutoResponses();

	virtual int Run( CURLAction& action );

protected:
	void QueryAll(void);
	void QueryOne( CURLAction& action );
	void DecodeForm(void);
	void DeleteAutoResponse(void);
	void ResetHitCount(void);
};