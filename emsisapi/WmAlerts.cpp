/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/CustomWaterMarkAlerts.h,v 1.1 2007/10/08 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Custom WaterMark Alerts
||              
\\*************************************************************************/

#include "stdafx.h"
#include "WmAlerts.h"
#include "DateFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CWaterMarkAlerts::CWaterMarkAlerts(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CWaterMarkAlerts::~CWaterMarkAlerts()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CWaterMarkAlerts::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_WaterMarkAlertID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_WaterMarkAlertID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_WaterMarkAlertID);
		}
	}

	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );
	
	if( GetISAPIData().m_sPage.compare( _T("editwmalert") ) == 0)
	{
		if ( sAction.compare( _T("resetHitCount") ) == 0 )
		{
			DISABLE_IN_DEMO();
			ResetHitCount();
			return 0;
		}
		else if( GetISAPIData().GetXMLPost() )
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
||  Comments:	Query all Age Alerts	              
\*--------------------------------------------------------------------------*/
void CWaterMarkAlerts::QueryAll(void)
{
	CEMSString sDate;
	PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("WaterMarkAlerts") );
	GetXMLGen().IntoElem();
	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("WaterMarkAlert") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_WaterMarkAlertID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetXMLGen().AddChildAttrib( _T("HitCount"), m_HitCount );
		GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDate );
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDate.c_str() );
		GetDateTimeString( m_DateEdited, m_DateEditedLen, sDate );
		GetXMLGen().AddChildAttrib( _T("DateEdited"), sDate.c_str() );
		GetXMLGen().AddChildAttrib( _T("CreatedByID"), m_CreatedByID );
		GetXMLGen().AddChildAttrib( _T("EditedByID"), m_EditedByID );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	}

	GetXMLGen().OutOfElem();

	ListEnabledAgentNames();
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query a single Age Alert
\*--------------------------------------------------------------------------*/
void CWaterMarkAlerts::QueryOne( CURLAction& action )
{
	CEMSString sDate;
	
	GetISAPIData().GetURLLong( _T("ID"), m_WaterMarkAlertID  );

	if ( m_WaterMarkAlertID )
	{
		Query(GetQuery());
	}
	else
	{
		action.m_sPageTitle = _T("New Watermark Alert");
		//Setup default values for new watermark Alert
		m_IsEnabled = 1;
		m_HitCount = 0;
		m_AlertToTypeID=0;
		m_AlertMethodID=1;
		m_SendLowAlert=1;
		m_LowWaterMark=5;
		m_HighWaterMark=0;
	}

	GetXMLGen().AddChildElem( _T("WaterMarkAlert") );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetXMLGen().AddChildAttrib( _T("HitCount"), m_HitCount );
	GetXMLGen().AddChildAttrib( _T("LowWaterMark"), m_LowWaterMark );
    GetXMLGen().AddChildAttrib( _T("HighWaterMark"), m_HighWaterMark );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	GetXMLGen().AddChildAttrib( _T("SendLowAlert"), m_SendLowAlert );
	GetXMLGen().AddChildAttrib( _T("AlertOnTypeID"), m_AlertOnTypeID );
	GetXMLGen().AddChildAttrib( _T("AlertOnID"), m_AlertOnID );
	GetXMLGen().AddChildAttrib( _T("AlertToTypeID"), m_AlertToTypeID );
	GetXMLGen().AddChildAttrib( _T("AlertToID"), m_AlertToID );
	GetXMLGen().AddChildAttrib( _T("AlertMethodID"), m_AlertMethodID );
	GetXMLGen().AddChildAttrib( _T("EmailAddress"), m_EmailAddress );
	GetXMLGen().AddChildAttrib( _T("FromEmailAddress"), m_FromEmailAddress );
	GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDate );
	GetXMLGen().AddChildAttrib( _T("DateCreated"), sDate.c_str() );
	GetDateTimeString( m_DateEdited, m_DateEditedLen, sDate );
	GetXMLGen().AddChildAttrib( _T("DateEdited"), sDate.c_str() );
	GetXMLGen().AddChildAttrib( _T("CreatedByID"), m_CreatedByID );
	GetXMLGen().AddChildAttrib( _T("EditedByID"), m_EditedByID );
		

	// add in the lists
	ListEnabledAgentNames();
	ListGroupNames();
	ListTicketBoxNames( GetXMLGen() );
	//ListTicketCategoryNames( GetXMLGen() );

	TTicketCategories pdt;
	pdt.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("TicketCategoryNames") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketCategory") );
		GetXMLGen().AddChildAttrib( _T("ID"), pdt.m_TicketCategoryID );
		GetXMLGen().AddChildAttrib( _T("Description"), pdt.m_Description );
	}
	GetXMLGen().OutOfElem();

	ListAlertMethods();
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Create a new or update an existing Auto Message              
\*--------------------------------------------------------------------------*/
void CWaterMarkAlerts::DecodeForm(void)
{
	int nAgentID = GetSession().m_AgentID;
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );

	GetISAPIData().GetURLLong( _T("ID"), m_WaterMarkAlertID  );
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, AUTOMESSAGES_DESCRIPTION_LENGTH );
	GetISAPIData().GetXMLLong( _T("LowWaterMark"), m_LowWaterMark );
	GetISAPIData().GetXMLLong( _T("HighWaterMark"), m_HighWaterMark );
	GetISAPIData().GetXMLLong( _T("IsEnabled"), m_IsEnabled );
	GetISAPIData().GetXMLLong( _T("SendLowAlert"), m_SendLowAlert  );
	GetISAPIData().GetXMLLong( _T("AlertOnTypeID"), m_AlertOnTypeID );
	if ( m_AlertOnTypeID == 0 )
	{
		GetISAPIData().GetXMLLong( _T("AlertOnTbID"), m_AlertOnID );
	}
	else
	{
		GetISAPIData().GetXMLLong( _T("AlertOnTcID"), m_AlertOnID );	
	}
	
	
	GetISAPIData().GetXMLLong( _T("AlertMethod"), m_AlertMethodID );
	if ( m_AlertMethodID == 2 )
	{
		GetISAPIData().GetXMLTCHAR( _T("EmailAddress"), m_EmailAddress, TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH );
		GetISAPIData().GetXMLTCHAR( _T("FromEmailAddress"), m_FromEmailAddress, TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH );	
	}
	else
	{
		GetISAPIData().GetXMLLong( _T("AlertToTypeID"), m_AlertToTypeID );
		if ( m_AlertToTypeID == 0 )
		{
			GetISAPIData().GetXMLLong( _T("Agent"), m_AlertToID );
		}
		else
		{
			GetISAPIData().GetXMLLong( _T("Group"), m_AlertToID );			
		}
	}
	
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_WaterMarkAlertID );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT WaterMarkAlertID FROM WaterMarkAlerts ")
						_T("WHERE WaterMarkAlertID<>? AND Description=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("That age alert already exists!")  );

	if ( m_WaterMarkAlertID )
	{
		//Set edit date and agent
		m_EditedByID = nAgentID;
		m_DateEdited = Now;

		//Update existing
		Update( GetQuery() );

		if ( GetQuery().GetRowCount() != 1 )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to edit the age alert, it may no longer exist.") );
	}
	else
	{
		//Set create and edit agent
		m_CreatedByID = nAgentID;
		m_EditedByID = nAgentID;
		
		//Create new
		Insert( GetQuery() );

		if ( !m_WaterMarkAlertID )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to insert the age alert.") );
	}

	InvalidateWaterMarkAlerts();

}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes one or more WaterMarkAlerts	              
\*--------------------------------------------------------------------------*/
void CWaterMarkAlerts::DeleteAlert(void)
{
	CEMSString sIDs;
	TWaterMarkAlerts aa;

	GetISAPIData().GetXMLString( _T("SELECTID"), sIDs  );
	
	while ( sIDs.CDLGetNextInt( m_WaterMarkAlertID ) )
	{
		if ( m_WaterMarkAlertID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("This is the default watermark alert and can't be deleted!") );

		//Delete the alert
		Delete( GetQuery() );

		if ( GetQuery().GetRowCount() != 1 )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to delete the age alert.") );
	}
	
	InvalidateWaterMarkAlerts();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Resets the hit count of an Age Alert	              
\*--------------------------------------------------------------------------*/
void CWaterMarkAlerts::ResetHitCount(void)
{
	// Reset the hit count for this Auto Message
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_WaterMarkAlertID );
	GetQuery().Execute( _T("UPDATE WaterMarkAlerts SET HitCount=0 WHERE WaterMarkAlertID = ?") );
	

	if ( GetQuery().GetRowCount() != 1 )
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to reset the age alert counts.") );

	
	InvalidateWaterMarkAlerts();
	
}