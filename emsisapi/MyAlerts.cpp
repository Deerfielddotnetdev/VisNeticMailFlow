
#include "stdafx.h"
#include "MyAlerts.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CMyAlerts::CMyAlerts(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CMyAlerts::~CMyAlerts()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CMyAlerts::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	//RequireAdmin();

	//alert.m_AgentID = GetAgentID();
	GetISAPIData().GetURLLong( _T("agentid"), alert.m_AgentID );

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), alert.m_AlertID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), alert.m_AlertID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(alert.m_AlertID);
		}
	}

	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );
	
	if( GetISAPIData().m_sPage.compare( _T("editmyalert") ) == 0)
	{
		if( GetISAPIData().GetXMLPost() )
		{
			DISABLE_IN_DEMO();
			DecodeForm();
		}
		else
		{
			QueryOne(action);
		}
	
		return 0;
	}

	if ( GetISAPIData().GetXMLPost() )
	{
		DISABLE_IN_DEMO();
		DeleteAlert();
	}
	else
	{
		QueryAll();
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query all Alerts	              
\*--------------------------------------------------------------------------*/
void CMyAlerts::QueryAll(void)
{
	//PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("MyAlerts") );
	GetXMLGen().IntoElem();

	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), alert.m_AlertID );
	BINDCOL_LONG_NOLEN( GetQuery(), alert.m_AlertEventID );
	BINDCOL_LONG_NOLEN( GetQuery(), alert.m_AlertMethodID );
	BINDCOL_TCHAR( GetQuery(), alert.m_EmailAddress );
	BINDPARAM_LONG (GetQuery(), alert.m_AgentID );	
	GetQuery().Execute( _T("SELECT AlertID,AlertEventID,AlertMethodID,EmailAddress ")
						_T("FROM Alerts WHERE AlertEventID IN (12,13,14,15,16,17) AND AgentID=? ")
						_T("ORDER BY AlertEventID,AlertMethodID,EmailAddress") );

	while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("MyAlert") );
		GetXMLGen().AddChildAttrib( _T("ID"), alert.m_AlertID );
		GetXMLGen().AddChildAttrib( _T("AEID"), alert.m_AlertEventID );
		GetXMLGen().AddChildAttrib( _T("AMID"), alert.m_AlertMethodID );
		GetXMLGen().AddChildAttrib( _T("EmailAddress"), alert.m_EmailAddress );
	}

	GetXMLGen().OutOfElem();

	ListAlertEvents();
	ListAlertMethods();	
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query a single Alert
\*--------------------------------------------------------------------------*/
void CMyAlerts::QueryOne( CURLAction& action )
{
	GetISAPIData().GetURLLong( _T("ID"), alert.m_AlertID  );
	if ( alert.m_AlertID )
	{
		if ( alert.Query( GetQuery() ) != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("The specified alert (%d) no longer exists in the database"), alert.m_AlertID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError.c_str() );
		}
		else
		{
			action.m_sPageTitle = _T("Edit Alert");
		}		
	}
	else
	{
		action.m_sPageTitle = _T("New Alert");
		//Setup default values for new Age Alert		
		alert.m_AlertMethodID=1;
		alert.m_AlertEventID=12;		
		_tcscpy( alert.m_EmailAddress, _T("") );		
	}

	GetXMLGen().AddChildElem( _T("MyAlert") );
	GetXMLGen().AddChildAttrib( _T("ID"), alert.m_AlertID );
	GetXMLGen().AddChildAttrib( _T("AMID"), alert.m_AlertMethodID );
	GetXMLGen().AddChildAttrib( _T("AEID"), alert.m_AlertEventID );
	GetXMLGen().AddChildAttrib( _T("EmailAddress"), alert.m_EmailAddress );
	
	// add in the lists
	ListAlertMethods();
	ListAlertEvents();	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Create a new or update an existing Alert           
\*--------------------------------------------------------------------------*/
void CMyAlerts::DecodeForm(void)
{
	GetISAPIData().GetURLLong( _T("ID"), alert.m_AlertID  );
	GetISAPIData().GetXMLLong( _T("AlertMethod"), alert.m_AlertMethodID );
	GetISAPIData().GetXMLLong( _T("AlertEvent"), alert.m_AlertEventID );
	if ( alert.m_AlertMethodID == 2 )
	{
		GetISAPIData().GetXMLTCHAR( _T("EmailAddress"), alert.m_EmailAddress, TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH );			
	}
		
	//Check to see if this alert already exists
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), alert.m_AlertID );
	BINDPARAM_LONG( GetQuery(), alert.m_AgentID );
	BINDPARAM_LONG( GetQuery(), alert.m_AlertEventID );
	BINDPARAM_LONG( GetQuery(), alert.m_AlertMethodID );
	BINDPARAM_TCHAR( GetQuery(), alert.m_EmailAddress );
	GetQuery().Execute( _T("SELECT AlertID FROM Alerts ")
						_T("WHERE AlertID<>? AND AgentID=? AND AlertEventID=? AND AlertMethodID=? AND EmailAddress=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("This alert already exists and cannot be duplicated!")  );

	if ( alert.m_AlertID )
	{
		//Update existing
		alert.Update( GetQuery() );

		if ( GetQuery().GetRowCount() != 1 )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to edit the alert, it may no longer exist.") );
	}
	else
	{	
		//Create new
		alert.Insert( GetQuery() );

		if ( !alert.m_AlertID )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to insert the alert.") );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes one or more Alerts	              
\*--------------------------------------------------------------------------*/
void CMyAlerts::DeleteAlert(void)
{
	CEMSString sIDs;
	
	GetISAPIData().GetXMLString( _T("SELECTID"), sIDs  );
	
	while ( sIDs.CDLGetNextInt( alert.m_AlertID ) )
	{
		if ( alert.m_AlertID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("Invalid ID selected for deletion!") );
		
		alert.Delete(GetQuery());

		if ( GetQuery().GetRowCount() != 1 )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to delete the alert.") );
	}
}