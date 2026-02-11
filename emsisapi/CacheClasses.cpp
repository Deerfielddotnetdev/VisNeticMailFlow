 /***************************************************************************\
||             
||  $Header: /root/EMSISAPI/CacheClasses.cpp,v 1.2 2005/11/29 21:16:25 markm Exp $
||
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "CacheTemplates.h"
#include "CacheClasses.h"
#include "RegistryFns.h"

void XAgentNames::LoadMap( CODBCQuery& query, map<unsigned int,XAgentNames>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("AgentNames") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TINYINT( query, m_IsDeleted );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute(	_T("SELECT AgentID,IsDeleted,Name ")
		            _T("FROM Agents ORDER BY Name") );		
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_AgentID ] = *this;

		if ( m_IsDeleted == 0 )
		{
			xmlgen.AddChildElem( _T("Agent") );
			xmlgen.AddChildAttrib( _T("ID"), m_AgentID );
			xmlgen.AddChildAttrib( _T("Name"), m_Name );
		}
	}
}

void XAgentNamesWOAdmin::LoadMap( CODBCQuery& query, map<unsigned int,XAgentNamesWOAdmin>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("AgentNames") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute(	_T("SELECT AgentID,Name ")
		_T("FROM Agents ")
		_T("WHERE AgentID > 1 AND IsDeleted = 0 ") 
		_T("ORDER BY Name") );		
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_AgentID ] = *this;
		xmlgen.AddChildElem( _T("Agent") );
		xmlgen.AddChildAttrib( _T("ID"), m_AgentID );
		xmlgen.AddChildAttrib( _T("Name"), m_Name );
	}
}

void XGroupNames::LoadMap( CODBCQuery& query, map<unsigned int,XGroupNames>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("GroupNames") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_TCHAR( query, m_GroupName );
	query.Execute(	_T("SELECT GroupID,GroupName ")
		_T("FROM Groups ")
		_T("WHERE IsDeleted = 0") 
		_T("ORDER BY GroupName") );		
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_GroupID ] = *this;
		xmlgen.AddChildElem( _T("Group") );
		xmlgen.AddChildAttrib( _T("ID"), m_GroupID );
		xmlgen.AddChildAttrib( _T("Name"), m_GroupName );
	}	
}

void XGroupNamesWOAdmin::LoadMap( CODBCQuery& query, map<unsigned int,XGroupNamesWOAdmin>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("GroupNames") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_TCHAR( query, m_GroupName );
	query.Execute(	_T("SELECT GroupID,GroupName ")
		_T("FROM Groups ")
		_T("WHERE IsDeleted = 0 AND GroupID <> 2 ") 
		_T("ORDER BY GroupName") );		
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_GroupID ] = *this;
		xmlgen.AddChildElem( _T("Group") );
		xmlgen.AddChildAttrib( _T("ID"), m_GroupID );
		xmlgen.AddChildAttrib( _T("Name"), m_GroupName );
	}	
}

void XTicketBoxNames::LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxNames>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("TicketBoxNames") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute(	_T("SELECT TicketBoxID,Name ")
		_T("FROM TicketBoxes ")
		_T("ORDER BY Name") );		
	
	while( query.Fetch() == S_OK )
	{			
		Map[ m_TicketBoxID ] = *this;
		
		if ( m_TicketBoxID == 1 ) /*unassigned ticketbox*/
		{
			xmlgen.SavePos();
			xmlgen.ResetChildPos();
			xmlgen.InsertChildElem( _T("TicketBox") );
		}
		else
		{
			xmlgen.AddChildElem( _T("TicketBox") );
		}
	
		xmlgen.AddChildAttrib( _T("ID"), m_TicketBoxID );
		xmlgen.AddChildAttrib( _T("Name"), m_Name );
		
		if ( m_TicketBoxID == 1 ) 
			xmlgen.RestorePos();	
	}
}

void XSRCategoryNames::LoadMap( CODBCQuery& query, map<unsigned int,XSRCategoryNames>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("SRCategoryNames") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_StdResponseCatID );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute(	_T("SELECT StdResponseCatID,CategoryName ")
		_T("FROM StdResponseCategories ")
		_T("ORDER BY CategoryName") );		
	
	while( query.Fetch() == S_OK )
	{			
		Map[ m_StdResponseCatID ] = *this;
		
		if ( m_StdResponseCatID == -4 ) /*uncategorized*/
		{
			xmlgen.SavePos();
			xmlgen.ResetChildPos();
			xmlgen.InsertChildElem( _T("SRCategory") );
		}
		else
		{
			xmlgen.AddChildElem( _T("SRCategory") );
		}
		
		xmlgen.AddChildAttrib( _T("ID"), m_StdResponseCatID );
		xmlgen.AddChildAttrib( _T("Name"), m_Name );
		
		if ( m_StdResponseCatID == -4 ) 
			xmlgen.RestorePos();	
	}
}

void XTicketCategoryNames::LoadMap( CODBCQuery& query, map<unsigned int,XTicketCategoryNames>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("TicketCategoryNames") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_TicketCategoryID );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute(	_T("SELECT TicketCategoryID,Description ")
		_T("FROM TicketCategories ")
		_T("ORDER BY Description") );		
	
	while( query.Fetch() == S_OK )
	{			
		Map[ m_TicketCategoryID ] = *this;
		
		if ( m_TicketCategoryID == 1 ) /*unassigned ticket category*/
		{
			xmlgen.SavePos();
			xmlgen.ResetChildPos();
			xmlgen.InsertChildElem( _T("TicketCategory") );
		}
		else
		{
			xmlgen.AddChildElem( _T("TicketCategory") );
		}
	
		xmlgen.AddChildAttrib( _T("ID"), m_TicketCategoryID );
		xmlgen.AddChildAttrib( _T("Description"), m_Name );
		
		if ( m_TicketCategoryID == 1 ) 
			xmlgen.RestorePos();	
	}
}

void XAutoMessageNames::LoadMap( CODBCQuery& query, map<unsigned int,XAutoMessageNames>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("AutoMessageNames") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_AutoMessageID );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute(	_T("SELECT AutoMessageID,Description ")
		_T("FROM AutoMessages ")
		_T("ORDER BY Description") );		
	
	while( query.Fetch() == S_OK )
	{			
		Map[ m_AutoMessageID ] = *this;
		
		if ( m_AutoMessageID == 0 ) 
		{
			xmlgen.SavePos();
			xmlgen.ResetChildPos();
			xmlgen.InsertChildElem( _T("AutoMessage") );
		}
		else
		{
			xmlgen.AddChildElem( _T("AutoMessage") );
		}
	
		xmlgen.AddChildAttrib( _T("ID"), m_AutoMessageID );
		xmlgen.AddChildAttrib( _T("Description"), m_Name );
		
		if ( m_AutoMessageID == 0 ) 
			xmlgen.RestorePos();	
	}
}

void XTicketFields::LoadMap( CODBCQuery& query, map<unsigned int,XTicketFields>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("TicketFields") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_TicketFieldID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute(	_T("SELECT TicketFieldID,Description ")
		_T("FROM TicketFields ")
		_T("ORDER BY Description") );		
	
	while( query.Fetch() == S_OK )
	{			
		Map[ m_TicketFieldID ] = *this;
		
		if ( m_TicketFieldID == 0 ) 
		{
			xmlgen.SavePos();
			xmlgen.ResetChildPos();
			xmlgen.InsertChildElem( _T("TicketField") );
		}
		else
		{
			xmlgen.AddChildElem( _T("TicketField") );
		}
	
		xmlgen.AddChildAttrib( _T("ID"), m_TicketFieldID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
		
		if ( m_TicketFieldID == 0 ) 
			xmlgen.RestorePos();	
	}
}

void XTicketBoxDefaultEmailAddressName::LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxDefaultEmailAddressName>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("DefaultEmailAddressNames") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_TCHAR( query, m_DefaultEmailAddressName );
	query.Execute(	_T("SELECT TicketBoxID, DefaultEmailAddressName ")
		_T("FROM TicketBoxes ")
		_T("ORDER BY DefaultEmailAddressName") );		
	
	while( query.Fetch() == S_OK )
	{			
		Map[ m_TicketBoxID ] = *this;
		
		if ( m_TicketBoxID == 1 ) /*unassigned ticketbox*/
		{
			xmlgen.SavePos();
			xmlgen.ResetChildPos();
			xmlgen.InsertChildElem( _T("TicketBox") );
		}
		else
		{
			xmlgen.AddChildElem( _T("TicketBox") );
		}
	
		xmlgen.AddChildAttrib( _T("ID"), m_TicketBoxID );
		if(m_DefaultEmailAddressNameLen != SQL_NULL_DATA)
			xmlgen.AddChildAttrib( _T("DefaultEmailAddressName"), m_DefaultEmailAddressName );
		else
			xmlgen.AddChildAttrib( _T("DefaultEmailAddressName"),"");
		
		if ( m_TicketBoxID == 1 ) 
			xmlgen.RestorePos();	
	}
}

void XTicketBoxAddrs::LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxAddrs>& Map, CMarkupSTL& xmlgen )
{	
	xmlgen.AddElem( _T("TicketBoxAddrs") );

	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_TCHAR( query, m_Address );

	query.Execute( _T("SELECT TicketBoxID,DefaultEmailAddress ")
		           _T("FROM TicketBoxes ") );		
	
	while( query.Fetch() == S_OK )
	{	
		if ( m_AddressLen > 0 )
		{
			Map[ m_TicketBoxID ] = *this;
			xmlgen.AddChildElem( _T("TicketBoxAddr") );
			xmlgen.AddChildAttrib( _T("ID"), m_TicketBoxID );
			xmlgen.AddChildAttrib( _T("Address"), m_Address );
		}
	}

	query.Initialize();
	BINDCOL_LONG( query, m_TicketBoxID );
	BINDCOL_TCHAR( query, m_Address );

	query.Execute( _T("SELECT AgentID,DataValue ")
		           _T("FROM PersonalData WHERE AgentID<0 ") );		
	
	while( query.Fetch() == S_OK )
	{	
		if ( m_AddressLen > 0 )
		{
			Map[ abs(m_TicketBoxID) ] = *this;
			xmlgen.AddChildElem( _T("TicketBoxAddr") );
			xmlgen.AddChildAttrib( _T("ID"), abs(m_TicketBoxID) );
			xmlgen.AddChildAttrib( _T("Address"), m_Address );
		}
	}

}

void XMessageDestinationNames::LoadMap( CODBCQuery& query, map<unsigned int,XMessageDestinationNames>& Map, CMarkupSTL& xmlgen )
{
	TCHAR m_ServerAddress[MESSAGEDESTINATIONS_SERVERADDRESS_LENGTH];
	long m_ServerAddressLen;
	TCHAR m_AuthUser[MESSAGEDESTINATIONS_AUTHUSER_LENGTH];
	long m_AuthUserLen;
	int m_SMTPPort;
	long m_SMTPPortLen;
	CEMSString sDispName;

	xmlgen.AddElem( _T("MessageDestinations") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_MsgDestID );
	BINDCOL_TCHAR( query, m_ServerAddress );
	BINDCOL_LONG( query, m_SMTPPort );
	BINDCOL_TCHAR( query, m_AuthUser );
	BINDCOL_TCHAR( query, m_Name );

	query.Execute(	_T("SELECT MessageDestinationID,ServerAddress,SMTPPort,AuthUser,Description ")
		_T("FROM MessageDestinations ")
		_T("ORDER BY Description,ServerAddress,AuthUser") );		
	
	while( query.Fetch() == S_OK )
	{
		if( _tcslen(m_Name) == 0)
		{
			sDispName.Format( _T("%s%s%s, PORT=%d"), m_ServerAddress, m_AuthUserLen ? _T(", USER=") : _T(""),
							  m_AuthUser, m_SMTPPort );
			_tcsncpy( m_Name, sDispName.c_str(), MESSAGESOURCES_REMOTEADDRESS_LENGTH-1 );
		}
		else
		{
			// Escape HTML
			sDispName.assign( m_Name );
			sDispName.EscapeHTML();
			_tcsncpy( m_Name, sDispName.c_str(), MESSAGESOURCES_REMOTEADDRESS_LENGTH-1 );
		}

		
		Map[ m_MsgDestID ] = *this;
		xmlgen.AddChildElem( _T("MessageDestination") );
		xmlgen.AddChildAttrib( _T("ID"), m_MsgDestID );
		xmlgen.AddChildAttrib( _T("Address"), m_Name );
	}
}

void XMessageSourceNames::LoadMap( CODBCQuery& query, map<unsigned int,XMessageSourceNames>& Map, CMarkupSTL& xmlgen )
{
	TCHAR m_RemoteAddress[MESSAGESOURCES_REMOTEADDRESS_LENGTH];
	long m_RemoteAddressLen;
	TCHAR m_AuthUserName[MESSAGESOURCES_AUTHUSERNAME_LENGTH];
	long m_AuthUserNameLen;
	int m_RemotePort;
	long m_RemotePortLen;
	CEMSString sDispName;

	xmlgen.AddElem( _T("MessageSources") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_MsgSourceID );
	BINDCOL_TCHAR( query, m_RemoteAddress );
	BINDCOL_LONG( query, m_RemotePort );
	BINDCOL_TCHAR( query, m_AuthUserName );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute(	_T("SELECT MessageSourceID,RemoteAddress,RemotePort,AuthUserName,Description ")
		_T("FROM MessageSources ")
		_T("ORDER BY Description,RemoteAddress,AuthUserName") );		
	
	while( query.Fetch() == S_OK )
	{
		if( m_NameLen == 0)
		{
			sDispName.Format( _T("%s%s%s, PORT=%d"), m_RemoteAddress, m_AuthUserNameLen ? _T(", USER=") : _T(""),
							  m_AuthUserName, m_RemotePort );
			_tcsncpy( m_Name, sDispName.c_str(), MESSAGESOURCES_REMOTEADDRESS_LENGTH-1 );
		}
		
		Map[ m_MsgSourceID ] = *this;
		xmlgen.AddChildElem( _T("MessageSource") );
		xmlgen.AddChildAttrib( _T("ID"), m_MsgSourceID );
		xmlgen.AddChildAttrib( _T("Address"), m_Name );
	}
}

void XRoutingRuleNames::LoadMap( CODBCQuery& query, map<unsigned int,XRoutingRuleNames>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("RoutingRules") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute(	_T("SELECT RoutingRuleID,RuleDescrip ")
		_T("FROM RoutingRules ")
		_T("ORDER BY OrderIndex") );		
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_RoutingRuleID ] = *this;
		xmlgen.AddChildElem( _T("RoutingRule") );
		xmlgen.AddChildAttrib( _T("ID"), m_RoutingRuleID );
		xmlgen.AddChildAttrib( _T("Description"), m_Name );
	}
}

void XProcessingRuleNames::LoadMap( CODBCQuery& query, map<unsigned int,XProcessingRuleNames>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("ProcessingRules") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_ProcessingRuleID );
	BINDCOL_TCHAR( query, m_Name );
	query.Execute(	_T("SELECT ProcessingRuleID,RuleDescrip ")
		_T("FROM ProcessingRules ")
		_T("ORDER BY OrderIndex") );		
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_ProcessingRuleID ] = *this;
		xmlgen.AddChildElem( _T("ProcessingRule") );
		xmlgen.AddChildAttrib( _T("ID"), m_ProcessingRuleID );
		xmlgen.AddChildAttrib( _T("Description"), m_Name );
	}
}

void XPriorities::LoadMap( CODBCQuery& query, map<unsigned int,XPriorities>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("TicketPriorities") );
	PrepareList( query );
	while( query.Fetch() == S_OK )
	{
		Map[ m_PriorityID ] = *this;
		xmlgen.AddChildElem( _T("Priority") );
		xmlgen.AddChildAttrib( _T("ID"), m_PriorityID );
		xmlgen.AddChildAttrib( _T("Description"), m_Name );
	}
}

void XTicketStates::LoadMap( CODBCQuery& query, map<unsigned int,XTicketStates>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("TicketStates") );
	PrepareList( query );
	while( query.Fetch() == S_OK )
	{
		Map[ m_TicketStateID ] = *this;
		xmlgen.AddChildElem( _T("State") );
		xmlgen.AddChildAttrib( _T("ID"), m_TicketStateID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
		xmlgen.AddChildAttrib( _T("HtmlColor"), m_HTMLColorName );
	}
}

void XOutboundMsgStates::LoadMap( CODBCQuery& query, map<unsigned int,XOutboundMsgStates>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("OutboundMsgStates") );
	PrepareList( query );
	while( query.Fetch() == S_OK )
	{
		Map[ m_OutboundMsgStateID ] = *this;
		xmlgen.AddChildElem( _T("State") );
		xmlgen.AddChildAttrib( _T("ID"), m_OutboundMsgStateID );
		xmlgen.AddChildAttrib( _T("Description"), m_MessageStateName );
	}
}

void XMessageSourceTypes::LoadMap( CODBCQuery& query, map<unsigned int,XMessageSourceTypes>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("MessageSourceTypes") );
	PrepareList( query );
	while( query.Fetch() == S_OK )
	{
		Map[ m_MessageSourceTypeID ] = *this;
		xmlgen.AddChildElem( _T("SourceType") );
		xmlgen.AddChildAttrib( _T("ID"), m_MessageSourceTypeID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
	}
}

void XMessageSourceTypeMatch::LoadMap( CODBCQuery& query, map<unsigned int,XMessageSourceTypeMatch>& Map, CMarkupSTL& xmlgen )
{
	TCHAR m_RemoteHost[MESSAGESOURCES_REMOTEADDRESS_LENGTH];
	long m_RemoteHostLen;
	TCHAR m_AuthUserName[MESSAGESOURCES_AUTHUSERNAME_LENGTH];
	long m_AuthUserNameLen;
	int m_RemotePort;
	long m_RemotePortLen;
	CEMSString sDispName;

	xmlgen.AddElem( _T("MessageSourceTypes") );
	
	// Add the fabricated 0-index item
	m_MessageSourceID = 0;
	_tcscpy( m_RemoteAddress, _T("ANY") );
	Map[0] = *this;
	xmlgen.AddChildElem( _T("SourceType") );
	xmlgen.AddChildAttrib( _T("ID"), m_MessageSourceID );
	xmlgen.AddChildAttrib( _T("Description"), m_RemoteAddress );

	query.Initialize();
	BINDCOL_LONG( query, m_MessageSourceID );
	BINDCOL_TCHAR( query, m_RemoteHost );
	BINDCOL_LONG( query, m_RemotePort );
	BINDCOL_TCHAR( query, m_AuthUserName );
	BINDCOL_TCHAR( query, m_RemoteAddress );
	query.Execute(	_T("SELECT MessageSourceID,RemoteAddress,RemotePort,AuthUserName,Description ")
	               _T("FROM MessageSources ORDER BY Description,RemoteAddress,AuthUserName ") );

	while( query.Fetch() == S_OK )
	{
		if( _tcslen( m_RemoteAddress ) == 0 )
		{
			sDispName.Format( _T("%s, User=%s, Port=%d"), m_RemoteHost, m_AuthUserName, m_RemotePort );
			_tcsncpy( m_RemoteAddress, sDispName.c_str(), MESSAGESOURCES_REMOTEADDRESS_LENGTH-1 );
		}
		Map[ m_MessageSourceID ] = *this;
		xmlgen.AddChildElem( _T("SourceType") );
		xmlgen.AddChildAttrib( _T("ID"), m_MessageSourceID );
		xmlgen.AddChildAttrib( _T("Description"), m_RemoteAddress );
	}
}

void XMatchLocations::LoadMap( CODBCQuery& query, map<unsigned int,XMatchLocations>& Map, CMarkupSTL& xmlgen )
{
	CEMSString s;
	xmlgen.AddElem( _T("MatchLocations") );
	
	m_ID = 0;
	s.LoadString( EMS_STRING_MATCH_LOCATION_SUBJECT );
	_tcscpy( m_Description, s.c_str() );
	Map[ m_ID ] = *this;
	AddXML( xmlgen );
	
	m_ID = 1;
	s.LoadString( EMS_STRING_MATCH_LOCATION_BODY );
	_tcscpy( m_Description, s.c_str() );
	Map[ m_ID ] = *this;
	AddXML( xmlgen );
	
	m_ID = 2;
	s.assign( _T("Subject or Body") );
	_tcscpy( m_Description, s.c_str() );
	Map[ m_ID ] = *this;
	AddXML( xmlgen );
	
	m_ID = 3;
	s.assign( _T("Headers") );
	_tcscpy( m_Description, s.c_str() );
	Map[ m_ID ] = *this;
	AddXML( xmlgen );

	m_ID = 4;
	s.assign( _T("Subject, Body or Headers") );
	_tcscpy( m_Description, s.c_str() );
	Map[ m_ID ] = *this;
	AddXML( xmlgen );
}

void XMatchLocations::AddXML( CMarkupSTL& xmlgen )
{
	xmlgen.AddChildElem( _T("Location") );
	xmlgen.AddChildAttrib( _T("ID"), m_ID );
	xmlgen.AddChildAttrib( _T("Description"), m_Description );
}

void XDeleteOptions::LoadMap( CODBCQuery& query, map<unsigned int,XDeleteOptions>& Map, CMarkupSTL& xmlgen )
{
	CEMSString s;
	xmlgen.AddElem( _T("DeleteOptions") );
	
	//		m_ID = 0;
	//		s.LoadString( EMS_STRING_DO_NOT_DELETE );
	//		_tcscpy( m_Description, s.c_str() );
	//		Map[ m_ID ] = *this;
	//		AddXML( xmlgen );
	
	m_ID = 1;
	s.LoadString( EMS_STRING_DELETE_TO_WASTE_BASKET );
	_tcscpy( m_Description, s.c_str() );
	Map[ m_ID ] = *this;
	AddXML( xmlgen );
	
	m_ID = 2;
	s.LoadString( EMS_STRING_DELETE_PERMANENTLY );
	_tcscpy( m_Description, s.c_str() );
	Map[ m_ID ] = *this;
	AddXML( xmlgen );
}

void XDeleteOptions::AddXML( CMarkupSTL& xmlgen )
{
	xmlgen.AddChildElem( _T("Option") );
	xmlgen.AddChildAttrib( _T("ID"), m_ID );
	xmlgen.AddChildAttrib( _T("Description"), m_Description );
}

void XLogSeverity::LoadMap( CODBCQuery& query, map<unsigned int,XLogSeverity>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("LogSeverities") );
	PrepareList( query );
	while( query.Fetch() == S_OK )
	{
		Map[ m_LogSeverityID ] = *this;
		xmlgen.AddChildElem( _T("LogSeverity") );
		xmlgen.AddChildAttrib( _T("ID"), m_LogSeverityID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
	}
}

void XLogEntryTypes::LoadMap( CODBCQuery& query, map<unsigned int,XLogEntryTypes>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("LogEntryTypes") );
	PrepareList( query );
	while( query.Fetch() == S_OK )
	{
		Map[ m_LogEntryTypeID ] = *this;
		xmlgen.AddChildElem( _T("LogEntryType") );
		xmlgen.AddChildAttrib( _T("ID"), m_LogEntryTypeID );
		xmlgen.AddChildAttrib( _T("Description"), m_TypeDescrip );
	}
}

void XPersonalDataTypes::LoadMap( CODBCQuery& query, map<unsigned int,XPersonalDataTypes>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("PersonalDataTypes") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_PersonalDataTypeID ] = *this;
		xmlgen.AddChildElem( _T("PersonalDataType") );
		xmlgen.AddChildAttrib( _T("ID"), m_PersonalDataTypeID );
		xmlgen.AddChildAttrib( _T("Description"), m_TypeName );
		xmlgen.AddChildAttrib( _T("BuiltIn"), m_BuiltIn );
	}
}

void XAutoActionTypes::LoadMap( CODBCQuery& query, map<unsigned int,XAutoActionTypes>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("AutoActionTypes") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_AutoActionTypeID ] = *this;
		xmlgen.AddChildElem( _T("AutoActionType") );
		xmlgen.AddChildAttrib( _T("ID"), m_AutoActionTypeID );
		xmlgen.AddChildAttrib( _T("Description"), m_TypeName );
		xmlgen.AddChildAttrib( _T("BuiltIn"), m_BuiltIn );
	}
}

void XAutoActionEvents::LoadMap( CODBCQuery& query, map<unsigned int,XAutoActionEvents>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("AutoActionEvents") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_AutoActionEventID ] = *this;
		xmlgen.AddChildElem( _T("AutoActionEvent") );
		xmlgen.AddChildAttrib( _T("ID"), m_AutoActionEventID );
		xmlgen.AddChildAttrib( _T("Description"), m_EventName );
		xmlgen.AddChildAttrib( _T("BuiltIn"), m_BuiltIn );
	}
}

void XTicketCategories::LoadMap( CODBCQuery& query, map<unsigned int,XTicketCategories>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("TicketCategories") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_TicketCategoryID ] = *this;
		xmlgen.AddChildElem( _T("TicketCategory") );
		xmlgen.AddChildAttrib( _T("ID"), m_TicketCategoryID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
		xmlgen.AddChildAttrib( _T("BuiltIn"), m_BuiltIn );
	}
}

void XTicketBoxOwners::LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxOwners>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("TicketBoxOwners") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_TicketBoxOwnerID ] = *this;
		xmlgen.AddChildElem( _T("TicketBoxOwner") );
		xmlgen.AddChildAttrib( _T("ID"), m_TicketBoxOwnerID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
	}
}

void XAutoMessages::LoadMap( CODBCQuery& query, map<unsigned int,XAutoMessages>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("AutoMessages") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_AutoMessageID ] = *this;
		xmlgen.AddChildElem( _T("AutoMessage") );
		xmlgen.AddChildAttrib( _T("ID"), m_AutoMessageID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
	}
}
void XAutoResponses::LoadMap( CODBCQuery& query, map<unsigned int,XAutoResponses>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("AutoResponses") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_AutoResponseID ] = *this;
		xmlgen.AddChildElem( _T("AutoResponse") );
		xmlgen.AddChildAttrib( _T("ID"), m_AutoResponseID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
	}
}
void XAgeAlerts::LoadMap( CODBCQuery& query, map<unsigned int,XAgeAlerts>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("AgeAlerts") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_AgeAlertID ] = *this;
		xmlgen.AddChildElem( _T("AgeAlert") );
		xmlgen.AddChildAttrib( _T("ID"), m_AgeAlertID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
	}
}
void XWaterMarkAlerts::LoadMap( CODBCQuery& query, map<unsigned int,XWaterMarkAlerts>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("WaterMarkAlerts") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_WaterMarkAlertID ] = *this;
		xmlgen.AddChildElem( _T("WaterMarkAlert") );
		xmlgen.AddChildAttrib( _T("ID"), m_WaterMarkAlertID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
	}
}
void XObjectTypes::LoadMap( CODBCQuery& query, map<unsigned int,XObjectTypes>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("ObjectTypes") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_ObjectTypeID ] = *this;
		xmlgen.AddChildElem( _T("ObjectType") );
		xmlgen.AddChildAttrib( _T("ID"), m_ObjectTypeID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
	}
}

void XTicketBoxViewTypes::LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxViewTypes>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("TicketBoxViewTypes") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_TicketBoxViewTypeID ] = *this;
		xmlgen.AddChildElem( _T("TicketBoxViewType") );
		xmlgen.AddChildAttrib( _T("ID"), m_TicketBoxViewTypeID );
		xmlgen.AddChildAttrib( _T("Name"), m_Name );
	}
}

void XAlertEvents::LoadMap( CODBCQuery& query, map<unsigned int,XAlertEvents>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("AlertEvents") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_AlertEventID ] = *this;
		xmlgen.AddChildElem( _T("AlertEvent") );
		xmlgen.AddChildAttrib( _T("ID"), m_AlertEventID );
		xmlgen.AddChildAttrib( _T("Name"), m_Description );
		xmlgen.AddChildAttrib( _T("NeedTicketBoxID"), m_NeedTicketBoxID );
	}
}

void XAlertMethods::LoadMap( CODBCQuery& query, map<unsigned int,XAlertMethods>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("AlertMethods") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_AlertMethodID ] = *this;
		xmlgen.AddChildElem( _T("AlertMethod") );
		xmlgen.AddChildAttrib( _T("ID"), m_AlertMethodID );
		xmlgen.AddChildAttrib( _T("Name"), m_Description );
	}
}

void XStyleSheets::LoadMap( CODBCQuery& query, map<unsigned int,XStyleSheets>& Map, CMarkupSTL& xmlgen)
{
	xmlgen.AddElem( _T("StyleSheets") );
	PrepareList( query );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_StyleSheetID ] = *this;
		xmlgen.AddChildElem( _T("Style") );
		xmlgen.AddChildAttrib( _T("ID"), m_StyleSheetID );
		xmlgen.AddChildAttrib( _T("Name"), m_Name );
	}
}

void XServerParameters::LoadMap( CODBCQuery& query, map<unsigned int,XServerParameters>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("ServerParameters") );
	PrepareList( query );
	while( query.Fetch() == S_OK )
	{			
		Map[ m_ServerParameterID ] = *this;
		xmlgen.AddChildElem( _T("ServerParameter") );
		xmlgen.AddChildAttrib( _T("ID"), m_ServerParameterID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
		xmlgen.SetData( m_DataValue, 1 );
	}

	UINT m_nServerID;
	CEMSString sTemp;
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               EMS_SERVER_ID_VALUE, m_nServerID ) != ERROR_SUCCESS)
	{
		m_nServerID = 1;
	}
	sTemp.Format(_T("%d"),m_nServerID);
	_tcsncpy( m_DataValue, sTemp.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );

	Map[ EMS_SRVPARAM_SERVER_ID ] = *this;
	xmlgen.AddChildElem( _T("ServerParameter") );
	xmlgen.AddChildAttrib( _T("ID"), EMS_SRVPARAM_SERVER_ID );
	xmlgen.AddChildAttrib( _T("Description"), _T("ServerID") );
	xmlgen.SetData( m_DataValue, 1 );
}

void XStdRespCategories::LoadMap( CODBCQuery& query, map<unsigned int,XStdRespCategories>& Map, CMarkupSTL& xmlgen )
{
	CEMSString sName;
	int nIndex = 4;	// temporary hack

	xmlgen.AddElem(_T("stdrespcategories"));

	query.Initialize();

	BINDCOL_LONG(query, m_StdResponseCatID);
	BINDCOL_TCHAR(query, m_CategoryName);

	query.Execute(_T("SELECT StdResponseCatID,CategoryName FROM StdResponseCategories ORDER BY CategoryName"));

	while(query.Fetch() == S_OK)
	{	
		if(m_StdResponseCatID != 1)
		{
			if ( m_StdResponseCatID == -4)
			{
				Map[ m_StdResponseCatID ] = *this;
				xmlgen.SavePos();
				xmlgen.ResetChildPos();
				xmlgen.InsertChildElem( _T("stdrespcategory") );
			}
			else
			{
				Map[ m_StdResponseCatID ] = *this;
				xmlgen.AddChildElem( _T("stdrespcategory") );
			}
						
			xmlgen.IntoElem();

			xmlgen.AddChildElem(_T("rowindex"), nIndex++);
			xmlgen.AddChildElem(_T("id"), m_StdResponseCatID);
			sName.assign( m_CategoryName );
			sName.EscapeHTML();
			xmlgen.AddChildElem(_T("name"),sName.c_str());
			xmlgen.OutOfElem();

			if ( m_StdResponseCatID == -4)
				xmlgen.RestorePos();
		}	
	}
}

void XAgentEmailAddresses::LoadMap( CODBCQuery& query, multimap<unsigned int,XAgentEmailAddresses>& Map )
{
	TCHAR AgentAddress[PERSONALDATA_DATAVALUE_LENGTH];
	long AgentAddressLen;
	TCHAR AgentName[AGENTS_NAME_LENGTH];
	long AgentNameLen;
	int AgentID;
	int DefaultEmailAddrID;

	query.Initialize();
	
	BINDCOL_LONG_NOLEN( query, AgentID );
	BINDCOL_TCHAR( query, AgentName);
	BINDCOL_LONG_NOLEN( query, DefaultEmailAddrID );
	BINDCOL_LONG_NOLEN( query, m_PersonalDataID );
	BINDCOL_TCHAR( query, AgentAddress );
	
	query.Execute(	_T("SELECT Agents.AgentID, Name, DefaultAgentAddressID, PersonalDataID, DataValue ")
					_T("FROM Agents ")
					_T("INNER JOIN PersonalData on PersonalData.AgentID = Agents.AgentID ")
					_T("WHERE PersonalDataTypeID=1 ")
					_T("ORDER BY PersonalDataID") );
	
	while( query.Fetch() == S_OK )
	{	
		if ( AgentAddressLen > 0 )
		{
			m_DefaultEmail = ( m_PersonalDataID == DefaultEmailAddrID );
			
			if ( AgentNameLen > 0)
			{
				m_EmailAddr.Format( _T("\"%s\" <%s>"), AgentName, AgentAddress );
			}
			else
			{
				m_EmailAddr.assign(AgentAddress);
			}

			Map.insert( pair<unsigned int,XAgentEmailAddresses> (AgentID, *this) );
		}
	}
}

bool XAgentEmailAddresses::Query( multimap<unsigned int,XAgentEmailAddresses>& Map, int ID )
{
	multimap<unsigned int,XAgentEmailAddresses>::iterator iter;
	
	for (iter = Map.begin(); iter != Map.end(); iter++ )
	{
		if (iter->second.m_PersonalDataID == ID)
		{
			*this = iter->second;
			return true;
		}
	}
	
	return false;
}

void XAgentEmailAddresses::GetXMLByID( multimap<unsigned int,XAgentEmailAddresses>& Map, int AgentID, CMarkupSTL& xmlgen )
{
	multimap<unsigned int,XAgentEmailAddresses>::iterator start;
	multimap<unsigned int,XAgentEmailAddresses>::iterator end;
	
	start = Map.lower_bound(AgentID);
	end = Map.upper_bound(AgentID);
	
	xmlgen.AddElem( _T("AgentEmailAddresses") );
	
	// for each signature...
	while ((start != end) && (start != Map.end()))
	{
		// the default email is the first item returned
		if ( start->second.m_DefaultEmail)
		{
			xmlgen.SavePos();
			xmlgen.ResetChildPos();
			xmlgen.InsertChildElem( _T("EmailAddress") );
		}
		else
		{
			xmlgen.AddChildElem( _T("EmailAddress") );
		}
		
		xmlgen.AddChildAttrib( _T("ID"), start->second.m_PersonalDataID );
		xmlgen.AddChildAttrib( _T("Default"), start->second.m_DefaultEmail );
		xmlgen.SetChildData(  start->second.m_EmailAddr.c_str(), TRUE );
		
		if ( start->second.m_DefaultEmail)
			xmlgen.RestorePos();
		
		start++;
	}
}

void XSignatures::LoadMap( CODBCQuery& query, map<unsigned int, XSignatures>& Map, CMarkupSTL& xmlgen )
{
	TCHAR szGroupName[GROUPS_GROUPNAME_LENGTH];
	long szGroupNameLen;
	CEMSString sTemp;

	query.Initialize();
	BINDCOL_LONG( query, m_SignatureID );
	BINDCOL_LONG( query, m_GroupID );
	BINDCOL_LONG( query, m_AgentID );
	BINDCOL_LONG( query, m_ObjectTypeID );
	BINDCOL_LONG( query, m_ActualID );
	BINDCOL_TCHAR( query, m_Name );
	BINDCOL_TCHAR( query, szGroupName );

	query.Execute( _T("SELECT SignatureID,Signatures.GroupID,Signatures.AgentID,Signatures.ObjectTypeID,Signatures.ActualID,Name,GroupName,Signature ")
		           _T("FROM Signatures LEFT OUTER JOIN Groups ON Signatures.GroupID = Groups.GroupID ") );
		
	while( query.Fetch() == S_OK )
	{
		GetLongData( query );

		// Add the Group name in parens if this is a group sig
		if( m_GroupID )
		{
			sTemp.Format( _T("%s (%s)"), m_Name, szGroupName );

			if( sTemp.length() >= SIGNATURES_NAME_LENGTH )
				sTemp.resize( SIGNATURES_NAME_LENGTH - 1);

			_tcscpy( m_Name, sTemp.c_str() );
		}

		Map[m_SignatureID] = *this;
		xmlgen.AddChildElem( _T("Signature") );
		xmlgen.AddChildAttrib( _T("ID"), m_SignatureID );
		xmlgen.AddChildAttrib( _T("ObjectTypeID"), m_ObjectTypeID );
		xmlgen.AddChildAttrib( _T("ActualID"), m_ActualID );		
		xmlgen.AddChildAttrib( _T("Name"), m_Name );
		xmlgen.SetChildData( m_Signature , 1 );
	}
}


void CAgentSignatureIDMap::BuildMap( CODBCQuery& query )
{
	if(m_bListValid){return;}

	m_bListValid = true;

	unsigned int nMutexWait = 5000;
	
	GetRegInt(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("MutexWait"), nMutexWait);
	dca::String z;
	z.Format("CAgentSignatureIDMap::BuildMap - MutexWait set to: [%d]", nMutexWait);
	DebugReporter::Instance().DisplayMessage(z.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	unsigned int SignatureID;
	unsigned int AgentID;
	unsigned int AGAgentID;

	long SignatureIDLen;
	long AgentIDLen;
	long AGAgentIDLen;

	if(m_SignatureMapMutex.AcquireLock(nMutexWait))
	{
		m_SignatureIDMap.clear();
		m_DefaultSigIDMap.clear();	

		// Query for the AgentID -> list of SignatureIDs map
		query.Initialize();
		
		BINDCOL_LONG( query, AgentID );
		BINDCOL_LONG( query, SignatureID );
		BINDCOL_LONG( query, AGAgentID );
		
		query.Execute(	_T("SELECT Signatures.AgentID,SignatureID,CASE WHEN AgentGroupings.AgentID IS NULL THEN 0 ELSE AgentGroupings.AgentID END ")
						_T("FROM Signatures LEFT OUTER JOIN AgentGroupings ")
						_T("ON Signatures.GroupID = AgentGroupings.GroupID ") 
						_T("ORDER BY Signatures.Name ") );
		
		while( query.Fetch() == S_OK )
		{		
			if( AgentID != 0 )
			{
				m_SignatureIDMap.insert( pair<unsigned int,unsigned int> (AgentID, SignatureID) );
			}
			else
			{
				m_SignatureIDMap.insert( pair<unsigned int,unsigned int> (AGAgentID, SignatureID) );
			}
		}	

		// Query for the Default SignatureIDs
		query.Initialize();

		BINDCOL_LONG( query, AgentID );
		BINDCOL_LONG( query, SignatureID );	
		
		query.Execute(	_T("SELECT AgentID,DefaultSignatureID ")
						_T("FROM Agents ") );
		
		while( query.Fetch() == S_OK )
		{
			m_DefaultSigIDMap[ AgentID ] = SignatureID;
		}
		m_SignatureMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CAgentSignatureIDMap::BuildMap - Could not aquire SignatureMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CAgentSignatureIDMap::BuildMap - Could not aquire SignatureMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

void CAgentSignatureIDMap::GetSignatureIDList( unsigned int AgentID, list<unsigned int>& IDList,
											   unsigned int& DefaultSignatureID, unsigned int nMutexWait )
{
	if(m_SignatureMapMutex.AcquireLock(nMutexWait))
	{
		multimap<unsigned int,unsigned int>::iterator start;
		multimap<unsigned int,unsigned int>::iterator end;
		
		start = m_SignatureIDMap.lower_bound(AgentID);
		end = m_SignatureIDMap.upper_bound(AgentID);

		while ((start != end) && (start != m_SignatureIDMap.end()))
		{
			IDList.push_back( start->second );
			start++;
		}

		map<unsigned int,unsigned int>::iterator iter = m_DefaultSigIDMap.find( AgentID );

		if( iter == m_DefaultSigIDMap.end() )
		{
			DefaultSignatureID = 0;
		}
		else
		{
			DefaultSignatureID = iter->second;
		}
		m_SignatureMapMutex.ReleaseLock();
	}
	else
	{
		CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
		if(pRoutingEngine)
		{
			pRoutingEngine->LogIt( EMS_ISAPI_LOG_INFO(EMS_LOG_ISAPI_EXTENSION, E_UnhandledException),
				_T("CAgentSignatureIDMap::GetSignatureIDList - Could not aquire SignatureMapMutex.") );
		}
		DebugReporter::Instance().DisplayMessage("CAgentSignatureIDMap::GetSignatureIDList - Could not aquire SignatureMapMutex.", DebugReporter::ISAPI, GetCurrentThreadId());
	}
}

void XSRCategoryIDs::LoadList( CODBCQuery& query, list<unsigned int>& IDlist )
{
	query.Initialize();
	BINDCOL_LONG_NOLEN( query, m_StdResponseCatID );
	
	query.Execute(	_T("SELECT StdResponseCatID FROM StdResponseCategories ")
		_T("ORDER BY CategoryName") );		
	
	while( query.Fetch() == S_OK )
	{
		IDlist.push_back(m_StdResponseCatID);
	}	
}

void XAgentIDs::LoadList( CODBCQuery& query, list<unsigned int>& IDlist )
{
	query.Initialize();
	BINDCOL_LONG_NOLEN( query, m_AgentID );
	
	query.Execute(	_T("SELECT AgentID FROM Agents ")
		_T("WHERE IsDeleted = 0 ") 
		_T("ORDER BY Name") );		
	
	while( query.Fetch() == S_OK )
	{
		IDlist.push_back(m_AgentID);
	}	
}

void XEnabledAgentIDs::LoadList( CODBCQuery& query, list<unsigned int>& IDlist )
{
	query.Initialize();
	BINDCOL_LONG_NOLEN( query, m_AgentID );
	
	query.Execute(	_T("SELECT AgentID FROM Agents ")
		_T("WHERE IsDeleted = 0 AND IsEnabled = 1 ") 
		_T("ORDER BY Name") );		
	
	while( query.Fetch() == S_OK )
	{
		IDlist.push_back(m_AgentID);
	}	
}

void XTicketBoxIDs::LoadList( CODBCQuery& query, list<unsigned int>& IDlist )
{
	query.Initialize();
	BINDCOL_LONG_NOLEN( query, m_TicketBoxID );
	
	query.Execute(	_T("SELECT TicketBoxID FROM TicketBoxes ")
		_T("ORDER BY Name") );		
	
	while( query.Fetch() == S_OK )
	{
		IDlist.push_back(m_TicketBoxID);
	}	
}

void XTicketCategoryIDs::LoadList( CODBCQuery& query, list<unsigned int>& IDlist )
{
	query.Initialize();
	BINDCOL_LONG_NOLEN( query, m_TicketCategoryID );
	
	query.Execute(	_T("SELECT TicketCategoryID FROM TicketCategories ")
		_T("ORDER BY Description") );		
	
	while( query.Fetch() == S_OK )
	{
		IDlist.push_back(m_TicketCategoryID);
	}	
}

void XTicketBoxHeaders::LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxHeaders>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("TicketBoxHeaders") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_HeaderID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute(	_T("SELECT HeaderID,Description ")
		_T("FROM TicketBoxHeaders ")
		_T("WHERE IsDeleted = 0 ") 
		_T("ORDER BY Description") );		
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_HeaderID ] = *this;
		xmlgen.AddChildElem( _T("TicketBoxHeader") );
		xmlgen.AddChildAttrib( _T("ID"), m_HeaderID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
	}	
}

void XTicketBoxFooters::LoadMap( CODBCQuery& query, map<unsigned int,XTicketBoxFooters>& Map, CMarkupSTL& xmlgen )
{
	xmlgen.AddElem( _T("TicketBoxFooters") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_FooterID );
	BINDCOL_TCHAR( query, m_Description );
	query.Execute(	_T("SELECT FooterID,Description ")
		_T("FROM TicketBoxFooters ")
		_T("WHERE IsDeleted = 0 ") 
		_T("ORDER BY Description") );		
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_FooterID ] = *this;
		xmlgen.AddChildElem( _T("TicketBoxFooter") );
		xmlgen.AddChildAttrib( _T("ID"), m_FooterID );
		xmlgen.AddChildAttrib( _T("Description"), m_Description );
	}	
}

void XTrackingBypass::LoadMap( CODBCQuery& query, map<unsigned int,XTrackingBypass>& Map, CMarkupSTL& xmlgen )
{
	int nValue;	
	int nMax = 0;
	query.Initialize();
	BINDCOL_LONG_NOLEN( query, nValue );
	query.Execute( _T("SELECT DataValue FROM ServerParameters WHERE ServerParameterID=143") );
	query.Fetch();
	if(nValue == 1)
	{
		query.Initialize();
		BINDCOL_LONG_NOLEN( query, nMax );
		query.Execute( _T("SELECT CASE WHEN MAX(EmailID) IS NULL THEN 1 ELSE MAX(EmailID) END FROM Email") );
		query.Fetch();		
	}
	
	xmlgen.AddElem( _T("TrackingBypass") );
	
	query.Initialize();
	BINDCOL_LONG( query, m_EmailID );
	BINDCOL_LONG( query, m_EmailTypeID );
	BINDCOL_TCHAR( query, m_EmailValue );
	query.Execute(	_T("SELECT EmailID,EmailTypeID,EmailValue ")
		            _T("FROM Email WHERE EmailTypeID < 2 ORDER BY EmailValue") );
	
	while( query.Fetch() == S_OK )
	{
		Map[ m_EmailID ] = *this;
		xmlgen.AddChildElem( _T("Bypass") );
		xmlgen.AddChildAttrib( _T("ID"), m_EmailID );
		xmlgen.AddChildAttrib( _T("TypeID"), m_EmailTypeID );
		xmlgen.AddChildAttrib( _T("Email"), m_EmailValue );
	}

	if(nValue == 1)
	{
		TCHAR AgentAddress[PERSONALDATA_DATAVALUE_LENGTH];
		long AgentAddressLen;
		int AgentID;

		query.Initialize();
		
		BINDCOL_LONG_NOLEN( query, AgentID );
		BINDCOL_TCHAR( query, AgentAddress );
		
		query.Execute(	_T("SELECT Agents.AgentID, DataValue ")
						_T("FROM Agents ")
						_T("INNER JOIN PersonalData on PersonalData.AgentID = Agents.AgentID ")
						_T("WHERE PersonalDataTypeID=1 AND Agents.IsEnabled=1 AND Agents.IsDeleted=0 ")
						_T("ORDER BY DataValue") );
		
		while( query.Fetch() == S_OK )
		{	
			if ( AgentAddressLen > 0 )
			{
				nMax++;
				XTrackingBypass bypass;
				bypass.m_EmailID = nMax;
				bypass.m_EmailTypeID = 0;
				_tcscpy(bypass.m_EmailValue, AgentAddress);
				
				Map[ nMax ] = bypass;
				xmlgen.AddChildElem( _T("Bypass") );
				xmlgen.AddChildAttrib( _T("ID"), nMax );
				xmlgen.AddChildAttrib( _T("TypeID"), 0 );
				xmlgen.AddChildAttrib( _T("Email"), AgentAddress );
			}
		}
	}
}