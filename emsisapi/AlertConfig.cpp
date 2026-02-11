// AlertConfig.cpp: implementation of the CAlertConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AlertConfig.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAlertConfig::CAlertConfig(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CAlertConfig::~CAlertConfig()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
int CAlertConfig::Run( CURLAction& action )
{
	// check security
	RequireAdmin();

	if ( GetISAPIData().GetXMLPost() )
	{
		DISABLE_IN_DEMO();

		if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("alertconfiglist") ) == 0 )
		{
			return DoDelete();
		}
		else
		{
			TAlerts alert;
			DecodeForm(alert);
			return DoUpdate(alert);	
		}
	}
	else if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("alertconfiglist") ) == 0 )
	{
		ListAll( m_Alerts );
		GenerateListXML();
	}
	else
	{
		TAlerts alert;
		QueryOne( alert );
		GenerateXML( alert );
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// QueryOne
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertConfig::QueryOne( TAlerts& alert )
{
	CEMSString sIDs;

	if ( GetISAPIData().GetURLString( _T("SelectID"), sIDs, true ) )
	{
		sIDs.CDLGetNextInt( alert.m_AlertID );

		if ( alert.Query( GetQuery() ) != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("The specified alert (%d) no longer exists in the database"), alert.m_AlertID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError.c_str() );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// ListAll
// 
////////////////////////////////////////////////////////////////////////////////
int CAlertConfig::ListAll( list<TAlerts>& alerts )
{
	TAlerts alert;
	alerts.clear();

	GetQuery().Initialize();
	alert.PrepareList( GetQuery() );

	while( GetQuery().Fetch() == S_OK )
	{
		alerts.push_back( alert );
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoUpdate
// 
////////////////////////////////////////////////////////////////////////////////
int CAlertConfig::DoUpdate( TAlerts& alert )
{
	CEMSString sIDs;
	GetISAPIData().GetURLString( _T("SelectID"), sIDs, true );
	sIDs.CDLGetNextInt( alert.m_AlertID );

	//Check to see if this alert already exists
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), alert.m_AlertID );
	BINDPARAM_LONG( GetQuery(), alert.m_AgentID );
	BINDPARAM_LONG( GetQuery(), alert.m_AlertEventID );
	BINDPARAM_LONG( GetQuery(), alert.m_AlertMethodID );
	BINDPARAM_TCHAR( GetQuery(), alert.m_EmailAddress );
	BINDPARAM_LONG( GetQuery(), alert.m_TicketBoxID );
	BINDPARAM_LONG( GetQuery(), alert.m_GroupID );
	GetQuery().Execute( _T("SELECT AlertID FROM Alerts ")
						_T("WHERE AlertID<>? AND AlertID>0 AND AgentID=? AND AlertEventID=? AND AlertMethodID=? AND EmailAddress=? AND TicketBoxID=? AND GroupID=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("This alert already exists and cannot be duplicated!")  );


	if ( alert.m_AlertID )
	{
		if ( alert.Update( GetQuery() ) != 1 )
		{
			CEMSString sError;
			sError.Format( _T("Unable to save alert"), alert.m_AlertID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError.c_str() );
		}
	}
	else
	{	
		alert.Insert( GetQuery() );
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoDelete
// 
////////////////////////////////////////////////////////////////////////////////
int CAlertConfig::DoDelete( void )
{
	TAlerts alert;
	CEMSString sIDs;

	GetISAPIData().GetXMLString( _T("SelectID"), sIDs );

	while ( sIDs.CDLGetNextInt( alert.m_AlertID ) )
	{
		alert.Delete( GetQuery() );
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecodeForm
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertConfig::DecodeForm( TAlerts& alert )
{
	tstring sType;

	GetISAPIData().GetXMLLong( _T("ALERTMETHOD"), alert.m_AlertMethodID );
	GetISAPIData().GetXMLLong( _T("ALERTEVENT"), alert.m_AlertEventID );
	GetISAPIData().GetXMLLong( _T("TICKETBOX"), alert.m_TicketBoxID );

	if ( GetISAPIData().GetXMLString( _T("AgentOrGroup"), sType, true ) )
	{
		if ( sType.compare( _T("Agent") ) == 0 )
		{
			GetISAPIData().GetXMLLong( _T("AGENT"), alert.m_AgentID, true );
		}
		else
		{
			GetISAPIData().GetXMLLong( _T("GROUP"), alert.m_GroupID, true );
		}
	}
	else
	{
		GetISAPIData().GetXMLTCHAR( _T("EMAILADDRESS"), alert.m_EmailAddress, 
			256, true );

		GetISAPIData().GetXMLTCHAR( _T("FROMEMAILADDRESS"), alert.m_FromEmailAddress, 
			256, true );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GenerateXML
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertConfig::GenerateXML( TAlerts& alert )
{
	if ( alert.m_AlertID )
	{
		XAlertEvents alertevent;

		GetXMLGen().AddChildElem( _T("Alert") );
		GetXMLGen().AddChildAttrib( _T("ID"), alert.m_AlertID );
		GetXMLGen().AddChildAttrib( _T("AgentID"), alert.m_AgentID );
		GetXMLGen().AddChildAttrib( _T("GroupID"), alert.m_GroupID );
		GetXMLGen().AddChildAttrib( _T("AlertEventID"), alert.m_AlertEventID );
		GetXMLGen().AddChildAttrib( _T("AlertMethodID"), alert.m_AlertMethodID );
		GetXMLGen().AddChildAttrib( _T("EmailAddress"), alert.m_EmailAddress );
		GetXMLGen().AddChildAttrib( _T("TicketBoxID"), alert.m_TicketBoxID );

		if( GetXMLCache().m_AlertEvents.Query( alert.m_AlertEventID, alertevent ) )
		{
			GetXMLGen().AddChildAttrib( _T("NeedTicketBoxID"), alertevent.m_NeedTicketBoxID );
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("NeedTicketBoxID"), 0 );
		}
	}
	
	ListAlertMethods();
	ListAlertEvents();
	ListEnabledAgentNames();
	ListGroupNames();

	GetXMLGen().SavePos();
	ListTicketBoxNames( GetXMLGen() );
	GetXMLGen().RestorePos();
	GetXMLGen().FindChildElem( _T("TicketBoxNames") );
	GetXMLGen().IntoElem();
	GetXMLGen().InsertChildElem( _T("TicketBox") );
	GetXMLGen().AddChildAttrib( _T("ID"), 0 );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Any TicketBox") );
	GetXMLGen().OutOfElem();

	tstring sValue;
	GetServerParameter( EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS, sValue );
	GetXMLGen().AddChildElem( _T("SystemEmailAddress") );
	GetXMLGen().SetChildData( sValue.c_str(), 1 );
}

////////////////////////////////////////////////////////////////////////////////
// 
// GenerateListXML
// 
////////////////////////////////////////////////////////////////////////////////
void CAlertConfig::GenerateListXML( void )
{
	CEMSString sRecipient;
	list<TAlerts>::iterator iter;

	for( iter= m_Alerts.begin(); iter != m_Alerts.end(); iter++ )
	{
		GetXMLGen().AddChildElem( _T("Alert") );
		GetXMLGen().AddChildAttrib( _T("ID"), iter->m_AlertID );

		if ( iter->m_AgentID )
		{
			GetAgentName( iter->m_AgentID, sRecipient );
		}
		else if ( iter->m_GroupID )
		{
			GetGroupName( iter->m_GroupID, sRecipient );
			sRecipient += _T(" (Group)");
		}
		else 
		{
			sRecipient = iter->m_EmailAddress;
		}
		
		AddAlertEventName( _T("AlertEventName"), iter->m_AlertEventID );
		AddAlertMethodName( _T("AlertMethodName"), iter->m_AlertMethodID );
		GetXMLGen().AddChildAttrib( _T("Recipient"), sRecipient.c_str() );
		GetXMLGen().AddChildAttrib( _T("FromEmailAddress"), iter->m_FromEmailAddress );
		
		if ( iter->m_TicketBoxID )
		{
			AddTicketBoxName( _T("TicketBoxName"), iter->m_TicketBoxID );
		}
		else
		{
			if ( iter->m_AlertEventID > 11 && iter->m_AlertEventID < 15 )
			{
				GetXMLGen().AddChildAttrib( _T("TicketBoxName"), _T("") );
			}
			else
			{
				GetXMLGen().AddChildAttrib( _T("TicketBoxName"), _T("Any TicketBox") );
			}
		}
	}
}
