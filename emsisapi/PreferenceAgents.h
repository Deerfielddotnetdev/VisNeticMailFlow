/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/PreferenceAgents.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"


class CPreferenceAgentItem 
{
public:
	CPreferenceAgentItem() { ZeroMemory( this, sizeof(CPreferenceAgentItem) ); }
	virtual ~CPreferenceAgentItem() {};

	long m_OpenTickets;
	TCHAR m_LoginName[AGENTS_LOGINNAME_LENGTH];	
	long m_LoginNameLen;
	TCHAR m_Name[AGENTS_NAME_LENGTH];
	long m_NameLen;
	int m_AgentBoxID;
	int m_IsSubscribed;
};


class CPreferenceAgents : public CXMLDataClass
{
public:
	CPreferenceAgents(CISAPIData& ISAPIData );
	virtual ~CPreferenceAgents() {};

	virtual int Run(CURLAction& action);

private:
	void List(void);
	void Update(void);
	void GetAgentViews( list<int>& IDList );

	int m_AgentID;
	int m_nRefreshLeftPane;
	
};
