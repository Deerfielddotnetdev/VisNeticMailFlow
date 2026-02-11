/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/XMLDataClass.h,v 1.2 2005/11/29 21:16:29 markm Exp $
||
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "ISAPIData.h"
#include "URLMap.h"

class CXMLDataClass  
{
public:
	CXMLDataClass(CISAPIData& ISAPIData);
	virtual ~CXMLDataClass() {};

	virtual int Run( CURLAction& action ) = 0;

	// security 
	unsigned char GetAgentRightLevel( int nObjectTypeID, int nActualID );
	unsigned char RequireAgentRightLevel( int nObjectTypeID, int nActualID, unsigned char RightLevelMin );
	void RequireAdmin(void);

	// these functions add lists from the XML Cache
	int  ListAgentNames(void);
	int  ListAgentNames(unsigned char RightLevelMin, int nIncludeID = 0);
	int  ListEnabledAgentNames(unsigned char RightLevelMin = EMS_NO_ACCESS, int nIncludeID = 0);
	int  ListAgentNamesWOAdmin(void);
	int  ListGroupNames(void);	
	int  ListGroupNamesWOAdmin(void);	
	void ListMessageSourceTypeMatch(void);	// For routing rules
	void ListMessageSourceTypes(void);
	void ListTicketPriorities(void);
	void ListDeleteOptions(void);
	void ListMatchLocations(void);
	int  ListTicketBoxNames( CMarkupSTL& xmlobj );
	int  ListTicketBoxNames(unsigned char RightLevelMin, CMarkupSTL& xmlobj, int nIncludeID = -1 );
	int  ListTicketBoxNames(int nAgentID, unsigned char RightLevelMin, CMarkupSTL& xmlobj, int nIncludeID = -1 );
	int  ListTicketCategoryNames( CMarkupSTL& xmlobj );
	int  ListTicketCategoryNames(unsigned char RightLevelMin, CMarkupSTL& xmlobj, int nIncludeID = -1 );	
	int ListStdRespCategories(int nAgentID, unsigned char RightLevelMin, CMarkupSTL& xmlobj, int nIncludeID = -1 );
	int ListStdRespCategories(void);
	
	int  ListTicketBoxDefaultEmailAddressName( CMarkupSTL& xmlobj );
	int  ListTicketBoxDefaultEmailAddressName(unsigned char RightLevelMin, CMarkupSTL& xmlobj, int nIncludeID = -1 );

	int	 ListTicketBoxAddrs( CMarkupSTL& xmlobj );
	void ListTicketStates(void);
	void ListTicketCategories(void);
	void ListTicketBoxOwners(void);
	void ListLogSeverities(void);
	void ListLogEntryTypes(void);
	void ListSignatures( int AgentID, CMarkupSTL& xmlobj, bool bEscape = false, 
		                 bool bIncludeBody = true, int nDefault = 0 );
	void ListAgentEmails( int AgentID, CMarkupSTL& xmlobj );
	void ListPersonalDataTypes(void);
	void ListAutoActionTypes(void);
	void ListAutoActionEvents(void);
	void ListObjectTypes(void);
	void ListAlertMethods(void);
	void ListAlertEvents(void);
	void ListMessageDestinations(void);
	void ListMessageSources(void);
	void ListRoutingRules(void);
	void ListProcessingRules(void);
	void ListStyleSheetNames(void);
	void ListTicketBoxHeaders(void);
	void ListTicketBoxFooters(void);
	void ListTrackingBypass(void);

	// These functions lookup the ID in the cache and insert a child attribute
	void AddTicketBoxName( TCHAR* szChildAttrib, int TicketBoxID );
	void AddTicketBoxAddr( TCHAR* szChildAttrib, int TicketBoxID );
	void AddTicketStateName( TCHAR* szChildAttrib, int TicketStateID );
	void AddTicketStateHtmlColor( TCHAR* szChildAttrib, int TicketStateID );
	void AddTicketCategoryName( TCHAR* szChildAttrib, int TicketCategoryID );
	void AddOutboundMsgStateName( TCHAR* szChildAttrib, int OutBoundMsgStateID );
	void AddDeleteOption( TCHAR* szChildAttrib, unsigned char DeleteImmediately );
	void AddPriority( TCHAR* szChildAttrib, int PriorityID );
	void AddMessageSourceTypeMatch( TCHAR* szChildAttrib, int MessageSourceTypeID );
	void AddAgentName( TCHAR* szChildAttrib, int AgentID, LPCTSTR szDefault = _T(""));
	void AddGroupName( TCHAR* szChildAttrib, int GroupID );
	void AddMessageSourceType( TCHAR* szChildAttrib, int MessageSourceTypeID );
	void AddObjectType( TCHAR* szChildAttrib, int ObjectTypeID );
	void AddTicketBoxViewTypeName( TCHAR* szChildAttrib, int nTicketBoxViewTypeID );
	void AddAlertMethodName( TCHAR* szChildAttrib, int AlertMethodID );
	void AddAlertEventName( TCHAR* szChildAttrib, int AlertEventID );
	void AddServerParameter( TCHAR* szChildAttrib, int ServerParameterID );
	bool AddStdRespCategory( TCHAR* szChildAttrib, int nCategoryID, bool bEscapeHTML=false );
	void AddStyleSheetFile( TCHAR* szChildAttrib, int StyleSheetID );
	void AddMessageSourceName( TCHAR* szChildAttrib, int MessageSourceID, bool bEscapeHTML=false );
	void AddMessageDestinationName( TCHAR* szChildAttrib, int MessageSourceID, bool bEscapeHTML=false );

	void GetServerParameter( int ServerParameterID, tstring& sValue, LPCTSTR lpDefault=_T(""));
	void SetServerParameter( int ServerParameterID, tstring& sValue );
	int UseCutoffDate(TIMESTAMP_STRUCT& ts);

	// Cache invalidation functions
	void InvalidateAgents( bool bRebuildNow = false );
	void InvalidateGroups( bool bRebuildNow = false );
	void InvalidateTicketBoxes( bool bRebuildNow = false );
	void InvalidateMessageDestinations( bool bRebuildNow = false );
	void InvalidateMessageSources( bool bRebuildNow = false );
	void InvalidateSignatures( bool bRebuildNow = false );
	void InvalidateStdRespCategories( bool bRebuildNow = false );
	void InvalidateRoutingRules( bool bRebuildNow = false );
	void InvalidateProcessingRules( bool bRebuildNow = false );
	void InvalidateServerParameters( bool bRebuildNow = false );
	void InvalidatePersonalDataTypes( bool bRebuildNow = false );
	void InvalidateAutoActionTypes( bool bRebuildNow = false );
	void InvalidateAutoActionEvents( bool bRebuildNow = false );
	void InvalidateTicketCategories( bool bRebuildNow = false );
	void InvalidateTicketBoxOwners( bool bRebuildNow = false );
	void InvalidateAutoMessages( bool bRebuildNow = false );
	void InvalidateAutoResponses( bool bRebuildNow = false );
	void InvalidateAgeAlerts( bool bRebuildNow = false );
	void InvalidateWaterMarkAlerts( bool bRebuildNow = false );
	void InvalidateAgentEmails( bool bRebuildNow = false );
	void InvalidateTracking( bool bRebuildNow = false );

	// Virus scanning function
	int  VirusScanFile( LPCTSTR szActualFile, tstring& sVirusName );	
	int  VirusScanString(LPCTSTR szString, LONG nStringLen, CEMSString& sVirusName, tstring& sCleanedString );
	void VirusScanAttachment( TAttachments Attachment, bool bException=false );
	void GetVirusScanStateIDText( int nID, CEMSString& sDescription );

	// Alerts
	void SendLowDiskSpaceAlert( LPCTSTR szActualFile );
	void SendTicketAssignAlert( long AlertID, LPCTSTR szAlertText );
	void SendOutboundApprovalAlert( long AlertID, LPCTSTR szAlertText );
	void SendOutboundApprovedAlert( long AlertID, LPCTSTR szAlertText );
	void SendOutboundReturnedAlert( long AlertID, LPCTSTR szAlertText );
	void LogSecurityViolation( int nObjectTypeID, int nActualID, unsigned char RightRequested );

	bool GetAgentName( int ID, tstring& sName );
	bool GetOwnerName( int ID, tstring& sName );
	bool GetGroupName( int ID, tstring& sName );
	bool GetTicketBoxName( int ID, tstring& sName );
	bool GetRoutingRuleName( int ID, tstring& sName );
	bool GetProcessingRuleName( int ID, tstring& sName );
	bool GetMsgSourceName( int ID, tstring& sName );

	bool GetTicketPriorityName( int PriorityID, tstring& sPriorityName );
	bool GetTicketStateName( int StateID, tstring& sStateName );
	bool GetTicketFieldName( int ID, tstring& sName );
	bool GetTicketCategoryName( int ID, tstring& sName );
	bool GetStdCategoryName( int nCategoryID, tstring& sStdCatName );

protected:
	CISAPIData& m_ISAPIData;
	
	inline void GenErrorXML(LPCTSTR szMsg);
	
	// accessor helpers
	CISAPIData& GetISAPIData( void )	{ return m_ISAPIData; }
	CODBCQuery& GetQuery( void )		{ return m_ISAPIData.m_query; }
	LPCTSTR	GetRequest( void )			{ return m_ISAPIData.m_sPage.c_str(); }
	int GetAgentID( void )				{ return m_ISAPIData.m_pSession->m_AgentID; }
	LPCTSTR GetAgentName( void)			{ return m_ISAPIData.m_pSession->m_AgentUserName.c_str(); }
	LPCTSTR GetOwnerName( void)			{ return m_ISAPIData.m_pSession->m_AgentUserName.c_str(); }
	int GetMaxRowsPerPage( void )		{ return m_ISAPIData.m_pSession->m_nMaxRowsPerPage; }
	CMarkupSTL& GetXMLGen( void )		{ return m_ISAPIData.m_xmlgen; }
	CMarkupSTL& GetXMLDoc( void )		{ return m_ISAPIData.m_xmldoc; }
	CAgentSession& GetSession( void )		{ return *(m_ISAPIData.m_pSession); }
	CBrowserSession& GetBrowserSession(void){ return *(m_ISAPIData.m_pBrowserSession); }
	CRoutingEngine& GetRoutingEngine(void)	{ return m_ISAPIData.m_RoutingEngine; }	
	CXMLCache&	GetXMLCache(void)			{ return m_ISAPIData.m_XMLCache; }

#if defined(DEMO_VERSION)
	//bool GetIsAdmin(void)					{ return m_ISAPIData.m_pSession->m_AgentID == 1; }
	bool GetIsAdmin(void)					{ return m_ISAPIData.m_pSession->m_IsAdmin; }
#else
	bool GetIsAdmin(void)					{ return m_ISAPIData.m_pSession->m_IsAdmin; }
#endif

	unsigned char GetRequireGetOldest(void)	{ return m_ISAPIData.m_pSession->m_RequireGetOldest; }
	bool GetIsPost(void)					{ return _tcsicmp( m_ISAPIData.m_pECB->lpszMethod, "POST" ) == 0; }
	bool GetIsXMLPost(void)					{ return m_ISAPIData.GetXMLPost(); };
};

