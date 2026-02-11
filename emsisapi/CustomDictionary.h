/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/CustomDictionary.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Supports management of user spellcheck dictionaries 
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CCustomDictionary : public CXMLDataClass  
{
public:
	CCustomDictionary(CISAPIData& ISAPIData);
	virtual ~CCustomDictionary() {};

	virtual int Run(CURLAction& action);

private:
	void ListUserDictionary( CURLAction& action );
	void DeleteFromUserDictionary( CURLAction& action );
	void AddToUserDictionary( CURLAction& action );

private:
	int m_nAgentID;
};
