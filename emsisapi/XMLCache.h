// XMLCache.h: interface for the CXMLCache class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLCACHE_H__014CA7DC_F304_4971_A4DF_071BF17BAF02__INCLUDED_)
#define AFX_XMLCACHE_H__014CA7DC_F304_4971_A4DF_071BF17BAF02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CacheClasses.h"
#include "CacheTemplates.h"

class CXMLCache  
{
public:
	CXMLCache();
	virtual ~CXMLCache();

	void BuildAllMaps( CODBCQuery& query );

	CMapCache<XAgentNames> m_AgentNames;
	CMapCache<XAgentNamesWOAdmin> m_AgentNamesWOAdmin;
	CMapCache<XTicketBoxNames> m_TicketBoxNames;
	CMapCache<XTicketCategoryNames> m_TicketCategoryNames;
	CMapCache<XSRCategoryNames> m_SRCategoryNames;
	CMapCache<XAutoMessageNames> m_AutoMessageNames;
	CMapCache<XTicketBoxDefaultEmailAddressName> m_DefaultEmailAddressName;
	CMapCache<XTicketBoxAddrs> m_TicketBoxAddrs;
	CMapCache<XTicketStates> m_TicketStates;
	CMapCache<XTicketFields> m_TicketFields;
	CMapCache<XOutboundMsgStates> m_OutboundMsgStates;
	CMapCache<XPriorities> m_TicketPriorities;
	CMapCache<XMessageSourceTypes> m_MessageSourceTypes;
	CMapCache<XMessageSourceTypeMatch> m_MessageSourceTypeMatch;
	CMapCache<XMatchLocations> m_MatchLocations;
	CMapCache<XDeleteOptions> m_DeleteOptions;
	CMapCache<XLogSeverity> m_LogSeverity;
	CMapCache<XLogEntryTypes> m_LogEntryTypes;
	CMapCache<XSignatures> m_Signatures;
	CAgentSignatureIDMap m_AgentSignatureIDs;
	CIDMapCache<XAgentEmailAddresses> m_AgentEmails;
	CMapCache<XPersonalDataTypes> m_PersonalDataTypes;
	CMapCache<XAutoActionTypes> m_AutoActionTypes;
	CMapCache<XAutoActionEvents> m_AutoActionEvents;
	CMapCache<XTicketCategories> m_TicketCategories;
	CMapCache<XTicketBoxOwners> m_TicketBoxOwners;
	CMapCache<XAutoMessages> m_AutoMessages;
	CMapCache<XAutoResponses> m_AutoResponses;
	CMapCache<XAgeAlerts> m_AgeAlerts;
	CMapCache<XWaterMarkAlerts> m_WaterMarkAlerts;
	CMapCache<XGroupNames> m_GroupNames;
	CMapCache<XGroupNamesWOAdmin> m_GroupNamesWOAdmin;
	CMapCache<XObjectTypes> m_ObjectTypes;
	CMapCache<XTicketBoxViewTypes> m_TicketBoxViewTypes;
	CMapCache<XAlertMethods> m_AlertMethods;
	CMapCache<XAlertEvents> m_AlertEvents;
	CMapCache<XMessageDestinationNames> m_MsgDestNames;
	CMapCache<XMessageSourceNames> m_MsgSourceNames;	
	CMapCache<XRoutingRuleNames> m_RoutingRuleNames;	
	CMapCache<XProcessingRuleNames> m_ProcessingRuleNames;	
	CMapCache<XServerParameters> m_ServerParameters;
	CMapCache<XStdRespCategories> m_StdRespCategories;
	COrderedListCache<XAgentIDs> m_AgentIDs;
	COrderedListCache<XEnabledAgentIDs> m_EnabledAgentIDs;
	COrderedListCache<XTicketBoxIDs> m_TicketBoxIDs;
	COrderedListCache<XTicketCategoryIDs> m_TicketCategoryIDs;
	COrderedListCache<XSRCategoryIDs> m_SRCategoryIDs;
	CMapCache<XStyleSheets>	m_StyleSheets;
	CMapCache<XTicketBoxHeaders> m_TicketBoxHeaders;
	CMapCache<XTicketBoxFooters> m_TicketBoxFooters;
	CMapCache<XTrackingBypass> m_TrackingBypass;

	void SetDirty( bool bIsDirty ) { m_bIsDirty = bIsDirty; }
	bool GetDirty( void ) { return m_bIsDirty; }
	
	LPCTSTR GetSystemLocalID( void ) { return m_SystemLocalID.c_str(); }

	void InvalidateAllMaps( void );	
	
protected:
	bool m_bIsDirty;
	CEMSString m_SystemLocalID;
};

#endif // !defined(AFX_XMLCACHE_H__014CA7DC_F304_4971_A4DF_071BF17BAF02__INCLUDED_)
