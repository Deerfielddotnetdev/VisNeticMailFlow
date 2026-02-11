/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/CustomAgeAlerts.h,v 1.1 2007/10/08 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Custom Age Alerts
||              
\\*************************************************************************/

#include "stdafx.h"
#include "AgeAlerts.h"
#include "DateFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CAgeAlerts::CAgeAlerts(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CAgeAlerts::~CAgeAlerts()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CAgeAlerts::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_AgeAlertID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_AgeAlertID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_AgeAlertID);
		}
	}

	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );
	
	if( GetISAPIData().m_sPage.compare( _T("editagealert") ) == 0)
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
void CAgeAlerts::QueryAll(void)
{
	CEMSString sDate;
	PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("AgeAlerts") );
	GetXMLGen().IntoElem();
	
    while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("AgeAlert") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_AgeAlertID );
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
void CAgeAlerts::QueryOne( CURLAction& action )
{
	CEMSString sDate;
	
	GetISAPIData().GetURLLong( _T("ID"), m_AgeAlertID  );

	if ( m_AgeAlertID )
	{
		Query(GetQuery());
	}
	else
	{
		action.m_sPageTitle = _T("New Age Alert");
		//Setup default values for new Age Alert
		m_IsEnabled = 1;
		m_HitCount = 0;
		m_AlertToTypeID=0;
		m_AlertMethodID=1;
		m_ThresholdMins=5;
		m_ThresholdFreq=0;
	}

	GetXMLGen().AddChildElem( _T("AgeAlert") );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetXMLGen().AddChildAttrib( _T("HitCount"), m_HitCount );
	GetXMLGen().AddChildAttrib( _T("ThresholdFreq"), m_ThresholdFreq );
    if (  m_ThresholdFreq == 1 )
	{
		m_ThresholdMins = m_ThresholdMins/60;
	}
	else if (  m_ThresholdFreq == 2 )
	{
		m_ThresholdMins = m_ThresholdMins/1440;
	}
	GetXMLGen().AddChildAttrib( _T("ThresholdMins"), m_ThresholdMins );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
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
void CAgeAlerts::DecodeForm(void)
{
	int nAgentID = GetSession().m_AgentID;
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );

	GetISAPIData().GetURLLong( _T("ID"), m_AgeAlertID  );
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, AUTOMESSAGES_DESCRIPTION_LENGTH );
	GetISAPIData().GetXMLLong( _T("ThresholdMins"), m_ThresholdMins );
	GetISAPIData().GetXMLLong( _T("ThresholdFreq"), m_ThresholdFreq );
	if ( m_ThresholdFreq == 1 )
	{
		m_ThresholdMins = m_ThresholdMins*60;
	}
	else if ( m_ThresholdFreq == 2 )
	{
		m_ThresholdMins = m_ThresholdMins*1440;
	}
	GetISAPIData().GetXMLLong( _T("IsEnabled"), m_IsEnabled );
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
	BINDPARAM_LONG( GetQuery(), m_AgeAlertID );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT AgeAlertID FROM AgeAlerts ")
						_T("WHERE AgeAlertID<>? AND Description=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("That age alert already exists!")  );

	if ( m_AgeAlertID )
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

		if ( !m_AgeAlertID )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to insert the age alert.") );
	}

	InvalidateAgeAlerts();

}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes one or more AgeAlerts	              
\*--------------------------------------------------------------------------*/
void CAgeAlerts::DeleteAlert(void)
{
	CEMSString sIDs;
	TAgeAlerts aa;

	GetISAPIData().GetXMLString( _T("SELECTID"), sIDs  );
	
	while ( sIDs.CDLGetNextInt( m_AgeAlertID ) )
	{
		if ( m_AgeAlertID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("This is the default age alert and can't be deleted!") );

		// Delete existing records for this age alert
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_AgeAlertID );
		GetQuery().Execute( _T("DELETE FROM AgeAlertsSent WHERE AgeAlertID = ?") );
					
		//Delete Message
		Delete( GetQuery() );

		if ( GetQuery().GetRowCount() != 1 )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to delete the age alert.") );
	}
	
	InvalidateAgeAlerts();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Resets the hit count of an Age Alert	              
\*--------------------------------------------------------------------------*/
void CAgeAlerts::ResetHitCount(void)
{
	// Reset the hit count for this Auto Message
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_AgeAlertID );
	GetQuery().Execute( _T("UPDATE AgeAlerts SET HitCount=0 WHERE AgeAlertID = ?") );
	

	if ( GetQuery().GetRowCount() != 1 )
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to reset the age alert counts.") );

	
	InvalidateAgeAlerts();
	
}