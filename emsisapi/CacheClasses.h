/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/CacheClasses.h,v 1.2 2005/11/29 21:16:25 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:		
||              
\\*************************************************************************/

#pragma once

////////////////////////////////////////////////////////////////////////////////
// XAgentNames - maps AgentIDs to Names
////////////////////////////////////////////////////////////////////////////////
class XAgentNames 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XAgentNames>& Map, CMarkupSTL& xmlgen );
	
	int m_AgentID;	
	long m_AgentIDLen;

	TCHAR m_Name[AGENTS_NAME_LENGTH];
	long m_NameLen;
	unsigned char m_IsDeleted;
	long m_IsDeletedLen;
};

////////////////////////////////////////////////////////////////////////////////
// XAgentNamesWOAdmin - maps AgentIDs to Names (excluding the administrator)
////////////////////////////////////////////////////////////////////////////////
class XAgentNamesWOAdmin
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XAgentNamesWOAdmin>& Map, CMarkupSTL& xmlgen );

	int m_AgentID;	
	long m_AgentIDLen;

	TCHAR m_Name[AGENTS_NAME_LENGTH];
	long m_NameLen;
};

////////////////////////////////////////////////////////////////////////////////
// XGroupNames - maps GroupIDs to Names
////////////////////////////////////////////////////////////////////////////////
class XGroupNames 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XGroupNames>& Map, CMarkupSTL& xmlgen );
	
	int m_GroupID;	
	long m_GroupIDLen;

	TCHAR m_GroupName[GROUPS_GROUPNAME_LENGTH];
	long m_GroupNameLen;
};

////////////////////////////////////////////////////////////////////////////////
// XGroupNamesWOAdmin - maps GroupIDs to Names (excluding administrators group)
////////////////////////////////////////////////////////////////////////////////
class XGroupNamesWOAdmin 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XGroupNamesWOAdmin>& Map, CMarkupSTL& xmlgen );

	int m_GroupID;	
	long m_GroupIDLen;

	TCHAR m_GroupName[GROUPS_GROUPNAME_LENGTH];
	long m_GroupNameLen;
};

////////////////////////////////////////////////////////////////////////////////
// XTicketBoxNames - maps TicketBoxID to Names
////////////////////////////////////////////////////////////////////////////////
class XTicketBoxNames 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxNames>& Map, CMarkupSTL& xmlgen );

	int m_TicketBoxID;	
	long m_TicketBoxIDLen;

	TCHAR m_Name[TICKETBOXES_NAME_LENGTH];
	long m_NameLen;
};

////////////////////////////////////////////////////////////////////////////////
// XTicketCategoryNames - maps TicketCategoryID to Names
////////////////////////////////////////////////////////////////////////////////
class XTicketCategoryNames 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTicketCategoryNames>& Map, CMarkupSTL& xmlgen );

	int m_TicketCategoryID;	
	long m_TicketCategoryIDLen;

	TCHAR m_Name[TICKETCATEGORIES_DESCRIPTION_LENGTH];
	long m_NameLen;
};

////////////////////////////////////////////////////////////////////////////////
// XSRCategoryNames - maps StdResponseCatIDs to Names
////////////////////////////////////////////////////////////////////////////////
class XSRCategoryNames 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XSRCategoryNames>& Map, CMarkupSTL& xmlgen );

	int m_StdResponseCatID;	
	long m_StdResponseCatIDLen;

	TCHAR m_Name[STDRESPONSECATEGORIES_CATEGORYNAME_LENGTH];
	long m_NameLen;
};

////////////////////////////////////////////////////////////////////////////////
// XAutoMessageNames - maps AutoMessageID to Names
////////////////////////////////////////////////////////////////////////////////
class XAutoMessageNames 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XAutoMessageNames>& Map, CMarkupSTL& xmlgen );

	int m_AutoMessageID;	
	long m_AutoMessageIDLen;

	TCHAR m_Name[AUTOMESSAGES_DESCRIPTION_LENGTH];
	long m_NameLen;
};

////////////////////////////////////////////////////////////////////////////////
// XTicketFields - maps TicketFieldID to Description
////////////////////////////////////////////////////////////////////////////////
class XTicketFields 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTicketFields>& Map, CMarkupSTL& xmlgen );

	int m_TicketFieldID;	
	long m_TicketFieldIDLen;

	TCHAR m_Description[TICKETFIELDS_DESCRIPTION_LENGTH];
	long m_DescriptionLen;
};

////////////////////////////////////////////////////////////////////////////////
// XTicketBoxDefaultEmailAddressName - maps TicketBoxID to DefaultEmailAddressName
////////////////////////////////////////////////////////////////////////////////
class XTicketBoxDefaultEmailAddressName
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int, XTicketBoxDefaultEmailAddressName>& Map, CMarkupSTL& xmlgen );

	int m_TicketBoxID;	
	long m_TicketBoxIDLen;

	TCHAR m_DefaultEmailAddressName[TICKETBOXES_NAME_LENGTH];
	long m_DefaultEmailAddressNameLen;
};

class XTicketBoxAddrs
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxAddrs>& Map, CMarkupSTL& xmlgen );

	int m_TicketBoxID;	
	long m_TicketBoxIDLen;
	
	TCHAR m_Address[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];
	long m_AddressLen;
};


////////////////////////////////////////////////////////////////////////////////
// XMessageDestinationNames - maps MessageDestinationIDs to Names
////////////////////////////////////////////////////////////////////////////////
class XMessageDestinationNames 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XMessageDestinationNames>& Map, CMarkupSTL& xmlgen );

	int m_MsgDestID;	
	long m_MsgDestIDLen;

	TCHAR m_Name[MESSAGEDESTINATIONS_SERVERADDRESS_LENGTH];
	long m_NameLen;
};

////////////////////////////////////////////////////////////////////////////////
// XMessageSourceNames - maps MessageSourceIDs to Names
////////////////////////////////////////////////////////////////////////////////
class XMessageSourceNames 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XMessageSourceNames>& Map, CMarkupSTL& xmlgen );

	int m_MsgSourceID;	
	long m_MsgSourceIDLen;

	TCHAR m_Name[MESSAGESOURCES_REMOTEADDRESS_LENGTH];
	long m_NameLen;
};

////////////////////////////////////////////////////////////////////////////////
// XRoutingRuleNames - maps RoutingRuleIDs to Names
////////////////////////////////////////////////////////////////////////////////
class XRoutingRuleNames
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XRoutingRuleNames>& Map, CMarkupSTL& xmlgen );

	int m_RoutingRuleID;	
	long m_RoutingRuleIDLen;

	TCHAR m_Name[ROUTINGRULES_RULEDESCRIP_LENGTH];
	long m_NameLen;
};

////////////////////////////////////////////////////////////////////////////////
// XProcessingRuleNames - maps ProcessingRuleIDs to Names
////////////////////////////////////////////////////////////////////////////////
class XProcessingRuleNames
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XProcessingRuleNames>& Map, CMarkupSTL& xmlgen );

	int m_ProcessingRuleID;	
	long m_ProcessingRuleIDLen;

	TCHAR m_Name[PROCESSINGRULES_RULEDESCRIP_LENGTH];
	long m_NameLen;
};

class XTicketStates : public TTicketStates 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTicketStates>& Map, CMarkupSTL& xmlgen );
};

class XOutboundMsgStates : public TOutboundMessageStates 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XOutboundMsgStates>& Map, CMarkupSTL& xmlgen );
};

class XPriorities : public TPriorities 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XPriorities>& Map, CMarkupSTL& xmlgen );
};

// This is useful for MessageSources
class XMessageSourceTypes : public TMessageSourceTypes
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XMessageSourceTypes>& Map, CMarkupSTL& xmlgen );
};

// This class is for Routing Rules
class XMessageSourceTypeMatch 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XMessageSourceTypeMatch>& Map, CMarkupSTL& xmlgen );

	int m_MessageSourceID;	/* 
			Uniquely identifies message sources.
		 IDENTITY PRIMARY KEY SQL Type: INTEGER */
	long m_MessageSourceIDLen;

	TCHAR m_RemoteAddress[MESSAGESOURCES_REMOTEADDRESS_LENGTH];	/* 
			Remote server address (ip or host name).
		 SQL Type: VARCHAR(255) */
	long m_RemoteAddressLen;

};

class XMatchLocations 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XMatchLocations>& Map, CMarkupSTL& xmlgen );
	void AddXML( CMarkupSTL& xmlgen );

	int m_ID;
	TCHAR m_Description[64];
};

class XDeleteOptions 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XDeleteOptions>& Map, CMarkupSTL& xmlgen );
	void AddXML( CMarkupSTL& xmlgen );
	int m_ID;
	TCHAR m_Description[64];
};


class XLogSeverity : public TLogSeverity 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XLogSeverity>& Map, CMarkupSTL& xmlgen );
};


class XLogEntryTypes : public TLogEntryTypes 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XLogEntryTypes>& Map, CMarkupSTL& xmlgen );
};

class XPersonalDataTypes : public TPersonalDataTypes
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XPersonalDataTypes>& Map, CMarkupSTL& xmlgen);
};

class XAutoActionTypes : public TAutoActionTypes
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XAutoActionTypes>& Map, CMarkupSTL& xmlgen);
};

class XAutoActionEvents : public TAutoActionEvents
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XAutoActionEvents>& Map, CMarkupSTL& xmlgen);
};

class XTicketCategories : public TTicketCategories
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTicketCategories>& Map, CMarkupSTL& xmlgen);
};

class XTicketBoxOwners : public TTicketBoxOwners
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxOwners>& Map, CMarkupSTL& xmlgen);
};

class XAutoMessages : public TAutoMessages
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XAutoMessages>& Map, CMarkupSTL& xmlgen);
};

class XAutoResponses : public TAutoResponses
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XAutoResponses>& Map, CMarkupSTL& xmlgen);
};

class XAgeAlerts : public TAgeAlerts
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XAgeAlerts>& Map, CMarkupSTL& xmlgen);
};

class XWaterMarkAlerts : public TWaterMarkAlerts
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XWaterMarkAlerts>& Map, CMarkupSTL& xmlgen);
};

class XObjectTypes : public TObjectTypes
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XObjectTypes>& Map, CMarkupSTL& xmlgen);
};

class XTicketBoxViewTypes : public TTicketBoxViewTypes
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxViewTypes>& Map, CMarkupSTL& xmlgen);
};


class XAlertEvents : public TAlertEvents
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XAlertEvents>& Map, CMarkupSTL& xmlgen);
};

class XAlertMethods : public TAlertMethods
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XAlertMethods>& Map, CMarkupSTL& xmlgen);
};

class XStyleSheets : public TStyleSheets
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XStyleSheets>& Map, CMarkupSTL& xmlgen);
};

class XServerParameters : public TServerParameters
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XServerParameters>& Map, CMarkupSTL& xmlgen );
};

class XStdRespCategories : public TStdResponseCategories
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XStdRespCategories>& Map, CMarkupSTL& xmlgen );
};


class XAgentEmailAddresses
{
public:
	void LoadMap( CODBCQuery& query, multimap<unsigned int,XAgentEmailAddresses>& Map );
	bool Query( multimap<unsigned int,XAgentEmailAddresses>& Map, int ID );
	void GetXMLByID( multimap<unsigned int,XAgentEmailAddresses>& Map, int AgentID, CMarkupSTL& xmlgen );

	CEMSString m_EmailAddr;

private:
	int m_PersonalDataID;
	unsigned char m_DefaultEmail;

};

class XSignatures : public TSignatures 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int, XSignatures>& Map, CMarkupSTL& xmlgen );
};

class CAgentSignatureIDMap
{
public:
	CAgentSignatureIDMap() { m_bListValid = false; }
	void BuildMap( CODBCQuery& query );

	void Invalidate(void) { m_bListValid = false; }
	bool ListValid(void) { return m_bListValid; }
	void GetSignatureIDList( unsigned int AgentID, list<unsigned int>& IDList,
		                     unsigned int& DefaultSignatureID, unsigned int nMutexWait );

protected:
	multimap<unsigned int,unsigned int>		m_SignatureIDMap;
	map<unsigned int,unsigned int>			m_DefaultSigIDMap;

	CSignatureMapMutex m_SignatureMapMutex;
	bool					m_bListValid;

};


/*---------------------------------------------------------------------------\                     
||  Comments:	Stores a list of AgentIDs ordered by name
\*--------------------------------------------------------------------------*/
class XAgentIDs
{
public:
	void LoadList( CODBCQuery& query, list<unsigned int>& IDlist );

private:
	int m_AgentID;	
};

/*---------------------------------------------------------------------------\                     
||  Comments:	Stores a list of enabled AgentIDs ordered by name
\*--------------------------------------------------------------------------*/
class XEnabledAgentIDs
{
public:
	void LoadList( CODBCQuery& query, list<unsigned int>& IDlist );

private:
	int m_AgentID;	
};

/*---------------------------------------------------------------------------\                     
||  Comments:	Stores a list of TicketCategoryIDs ordered by name
\*--------------------------------------------------------------------------*/
class XTicketCategoryIDs
{
public:
	void LoadList( CODBCQuery& query, list<unsigned int>& IDlist );

private:
	int m_TicketCategoryID;	
};

/*---------------------------------------------------------------------------\                     
||  Comments:	Stores a list of SRCategoryIDs ordered by name
\*--------------------------------------------------------------------------*/
class XSRCategoryIDs
{
public:
	void LoadList( CODBCQuery& query, list<unsigned int>& IDlist );

private:
	int m_StdResponseCatID;	
};

/*---------------------------------------------------------------------------\                     
||  Comments:	Stores a list of TicketBoxIDs ordered by name
\*--------------------------------------------------------------------------*/
class XTicketBoxIDs
{
public:
	void LoadList( CODBCQuery& query, list<unsigned int>& IDlist );

private:
	int m_TicketBoxID;	
};

////////////////////////////////////////////////////////////////////////////////
// 
// XTicketBoxHeaders - maps TicketHeaderIDs to Names
// 
////////////////////////////////////////////////////////////////////////////////
class XTicketBoxHeaders 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxHeaders>& Map, CMarkupSTL& xmlgen );

	int m_HeaderID;	
	long m_HeaderIDLen;

	TCHAR m_Description[TICKETBOXHEADERS_DESCRIPTION_LENGTH];
	long m_DescriptionLen;

};

////////////////////////////////////////////////////////////////////////////////
// 
// XTicketBoxFooters - maps TicketHeaderIDs to Names
// 
////////////////////////////////////////////////////////////////////////////////
class XTicketBoxFooters 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxFooters>& Map, CMarkupSTL& xmlgen );

	int m_FooterID;	
	long m_FooterIDLen;

	TCHAR m_Description[TICKETBOXFOOTERS_DESCRIPTION_LENGTH];
	long m_DescriptionLen;
};

////////////////////////////////////////////////////////////////////////////////
// 
// XTrackingBypass - maps Bypass items to Values
// 
////////////////////////////////////////////////////////////////////////////////
class XTrackingBypass 
{
public:
	void LoadMap( CODBCQuery& query, map<unsigned int,XTrackingBypass>& Map, CMarkupSTL& xmlgen );

	int m_EmailID;	
	long m_EmailIDLen;
	
	int m_EmailTypeID;
	long m_EmailTypeIDLen;
	TCHAR m_EmailValue[PERSONALDATA_DATAVALUE_LENGTH];
	long m_EmailValueLen;
};
