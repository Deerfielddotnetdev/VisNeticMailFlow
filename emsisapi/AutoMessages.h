/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/AutoMessages.h,v 1.1 2007/10/08 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Auto Messages
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CAutoMessages : public CXMLDataClass, public TAutoMessages 
{
public:
	CAutoMessages( CISAPIData& ISAPIData );
	virtual ~CAutoMessages();

	virtual int Run( CURLAction& action );

protected:
	void QueryAll(void);
	void QueryOne( CURLAction& action );
	void DecodeForm(void);
	void DeleteMessage(void);
	void ResetHitCount(void);
};