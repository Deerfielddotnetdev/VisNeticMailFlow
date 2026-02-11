// XMLCache.cpp: implementation of the CXMLCache class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLCache.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXMLCache::CXMLCache()
{
	m_bIsDirty = true;

	// get the system local id.... since the system needs to be restarted when
	// this value is changed, it's OK to cache it
	m_SystemLocalID.Format( _T("%d"), GetSystemDefaultLCID() );
}

CXMLCache::~CXMLCache()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
// BuildAllMaps
// 
////////////////////////////////////////////////////////////////////////////////
void CXMLCache::BuildAllMaps( CODBCQuery& query )
{
	if( !m_AgentNames.ListValid() )
		m_AgentNames.BuildMap( query );
	
	if( !m_AgentNamesWOAdmin.ListValid() )
		m_AgentNamesWOAdmin.BuildMap( query );

	if( !m_TicketBoxNames.ListValid() )
		m_TicketBoxNames.BuildMap( query );

	if( !m_TicketCategoryNames.ListValid() )
		m_TicketCategoryNames.BuildMap( query );

	if( !m_AutoMessageNames.ListValid() )
		m_AutoMessageNames.BuildMap( query );

	if( !m_DefaultEmailAddressName.ListValid() )
		m_DefaultEmailAddressName.BuildMap(query);
	
	if( !m_TicketBoxAddrs.ListValid() )
		m_TicketBoxAddrs.BuildMap( query );

	if( !m_TicketStates.ListValid() )
		m_TicketStates.BuildMap( query );
	
	if( !m_TicketFields.ListValid() )
		m_TicketFields.BuildMap( query );
	
	if( !m_TicketCategories.ListValid() )
		m_TicketCategories.BuildMap( query );
	
	if( !m_TicketBoxOwners.ListValid() )
		m_TicketBoxOwners.BuildMap( query );
	
	if( !m_OutboundMsgStates.ListValid() )
		m_OutboundMsgStates.BuildMap( query );

	if( !m_TicketPriorities.ListValid() )
		m_TicketPriorities.BuildMap( query );

	if( !m_MessageSourceTypes.ListValid() )
		m_MessageSourceTypes.BuildMap( query );

	if( !m_MessageSourceTypeMatch.ListValid() )
		m_MessageSourceTypeMatch.BuildMap( query );

	if( !m_MatchLocations.ListValid() )
		m_MatchLocations.BuildMap( query );

	if( !m_DeleteOptions.ListValid() )
		m_DeleteOptions.BuildMap( query );
	
	if( !m_LogSeverity.ListValid() )
		m_LogSeverity.BuildMap( query );

	if( !m_LogEntryTypes.ListValid() )
		m_LogEntryTypes.BuildMap( query );
	
	if ( !m_PersonalDataTypes.ListValid() )
		 m_PersonalDataTypes.BuildMap( query );

	if ( !m_AutoActionTypes.ListValid() )
		 m_AutoActionTypes.BuildMap( query );

	if ( !m_AutoActionEvents.ListValid() )
		 m_AutoActionEvents.BuildMap( query );

	if( !m_Signatures.ListValid() )
		m_Signatures.BuildMap( query );

	if( !m_AgentSignatureIDs.ListValid() )
		m_AgentSignatureIDs.BuildMap( query );

	if( !m_AgentEmails.ListValid() )
		m_AgentEmails.BuildMap( query );

	if( !m_GroupNames.ListValid() )
		m_GroupNames.BuildMap( query );	

	if( !m_GroupNamesWOAdmin.ListValid() )
		m_GroupNamesWOAdmin.BuildMap( query );	

	if( !m_ObjectTypes.ListValid() )
		m_ObjectTypes.BuildMap( query );
	
	if( !m_TicketBoxViewTypes.ListValid() )
		m_TicketBoxViewTypes.BuildMap( query );

	if( !m_AlertMethods.ListValid() )
		m_AlertMethods.BuildMap( query );

	if( !m_AlertEvents.ListValid() )
		m_AlertEvents.BuildMap( query );
	
	if( !m_MsgDestNames.ListValid() )
		m_MsgDestNames.BuildMap( query );
	
	if( !m_ServerParameters.ListValid() )
		m_ServerParameters.BuildMap( query );
	
	if( !m_StdRespCategories.ListValid() )
		m_StdRespCategories.BuildMap( query );

	if( !m_SRCategoryNames.ListValid() )
		m_SRCategoryNames.BuildMap( query );

	if( !m_SRCategoryIDs.ListValid() )
		m_SRCategoryIDs.Build( query );
	
	if( !m_AgentIDs.ListValid() )
		m_AgentIDs.Build( query );

	if( !m_EnabledAgentIDs.ListValid() )
		m_EnabledAgentIDs.Build( query );
	
	if( !m_TicketBoxIDs.ListValid() )
		m_TicketBoxIDs.Build( query );

	if( !m_TicketCategoryIDs.ListValid() )
		m_TicketCategoryIDs.Build( query );

	if( !m_StyleSheets.ListValid() )
		m_StyleSheets.BuildMap( query );
	
	if( !m_TicketBoxHeaders.ListValid() )
		m_TicketBoxHeaders.BuildMap( query );

	if( !m_TicketBoxFooters.ListValid() )
		m_TicketBoxFooters.BuildMap( query );

	if( !m_MsgSourceNames.ListValid() )
		m_MsgSourceNames.BuildMap( query );
	
	if( !m_RoutingRuleNames.ListValid() )
		m_RoutingRuleNames.BuildMap( query );

	if( !m_ProcessingRuleNames.ListValid() )
		m_ProcessingRuleNames.BuildMap( query );

	if( !m_TrackingBypass.ListValid() )
		m_TrackingBypass.BuildMap( query );
	
	m_bIsDirty = false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// InvalidateAllMaps
// 
////////////////////////////////////////////////////////////////////////////////
void CXMLCache::InvalidateAllMaps( void )
{
	m_AgentNames.Invalidate();
	m_AgentNamesWOAdmin.Invalidate();
	m_TicketBoxNames.Invalidate();
	m_DefaultEmailAddressName.Invalidate();
	m_TicketBoxAddrs.Invalidate();
	m_TicketStates.Invalidate();
	m_TicketFields.Invalidate();
	m_TicketCategories.Invalidate();
	m_AutoMessages.Invalidate();
	m_OutboundMsgStates.Invalidate();
	m_TicketPriorities.Invalidate();
	m_MessageSourceTypes.Invalidate();
	m_MessageSourceTypeMatch.Invalidate();
	m_MatchLocations.Invalidate();
	m_DeleteOptions.Invalidate();
	m_LogSeverity.Invalidate();
	m_LogEntryTypes.Invalidate();
	m_PersonalDataTypes.Invalidate();
	m_AutoActionTypes.Invalidate();
	m_AutoActionEvents.Invalidate();
	m_Signatures.Invalidate();
	m_AgentSignatureIDs.Invalidate();
	m_AgentEmails.Invalidate();
	m_GroupNames.Invalidate();
	m_GroupNamesWOAdmin.Invalidate();
	m_ObjectTypes.Invalidate();
	m_TicketBoxViewTypes.Invalidate();
	m_AlertMethods.Invalidate();
	m_AlertEvents.Invalidate();
	m_MsgDestNames.Invalidate();
	m_ServerParameters.Invalidate();
	m_StdRespCategories.Invalidate();
	m_SRCategoryNames.Invalidate();
	m_SRCategoryIDs.Invalidate();
	m_AgentIDs.Invalidate();
	m_EnabledAgentIDs.Invalidate();
	m_TicketBoxIDs.Invalidate();
	m_StyleSheets.Invalidate();
	m_TicketBoxHeaders.Invalidate();
	m_TicketBoxFooters.Invalidate();
	m_MsgSourceNames.Invalidate();
	m_RoutingRuleNames.Invalidate();
	m_TrackingBypass.Invalidate();

	m_bIsDirty = true;
}