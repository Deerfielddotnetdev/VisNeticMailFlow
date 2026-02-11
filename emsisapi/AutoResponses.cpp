/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/AutoResponses.h,v 1.1 2007/10/08 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Auto Responses
||              
\\*************************************************************************/

#include "stdafx.h"
#include "AutoResponses.h"
#include "DateFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CAutoResponses::CAutoResponses(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CAutoResponses::~CAutoResponses()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CAutoResponses::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_AutoResponseID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_AutoResponseID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_AutoResponseID);
		}
	}

	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );
	
	if( GetISAPIData().m_sPage.compare( _T("editautoresponse") ) == 0)
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
		DeleteAutoResponse();
	}
	else
	{
		QueryAll();
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query all AutoResponses	              
\*--------------------------------------------------------------------------*/
void CAutoResponses::QueryAll(void)
{
	CEMSString sDate;
	PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("AutoResponses") );
	GetXMLGen().IntoElem();
	
    while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("AutoResponse") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_AutoResponseID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetXMLGen().AddChildAttrib( _T("HitCount"), m_HitCount );
		GetXMLGen().AddChildAttrib( _T("FailCount"), m_FailCount );
		GetXMLGen().AddChildAttrib( _T("TotalCount"), m_TotalCount );
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
||  Comments:	Query a single AutoResponse
\*--------------------------------------------------------------------------*/
void CAutoResponses::QueryOne( CURLAction& action )
{
	CEMSString sDate;
	TCHAR nStdRespName[101];
	long nStdRespNameLen;
	double nSentPercent = 0;

	GetISAPIData().GetURLLong( _T("ID"), m_AutoResponseID  );

	if ( m_AutoResponseID )
	{
		Query(GetQuery());
	}
	else
	{
		action.m_sPageTitle = _T("New Auto Response");
		//Setup default values for new Auto Response
		m_IsEnabled = 1;
		m_SendToPercent = 100;
		m_WhenToSendVal = 2;
		m_WhenToSendFreq = 2;
		m_WhenToSendTypeID = 1;
		m_SendFromTypeID = 0;
		m_HitCount = 0;
		m_FailCount = 0;
		m_TotalCount = 0;
		m_AutoReplyQuoteMsg = 1;
		m_AutoReplyCloseTicket = 0;
		m_AutoReplyInTicket = 1;
		m_OmitTracking = 0;
	}

	GetXMLGen().AddChildElem( _T("AutoResponse") );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetXMLGen().AddChildAttrib( _T("HitCount"), m_HitCount );
	GetXMLGen().AddChildAttrib( _T("FailCount"), m_FailCount );
	GetXMLGen().AddChildAttrib( _T("TotalCount"), m_TotalCount );
	nSentPercent = ((double)m_HitCount / m_TotalCount)*100;
	GetXMLGen().AddChildAttrib( _T("SentPercent"), (int)nSentPercent );
	GetXMLGen().AddChildAttrib( _T("SendToTypeID"), m_SendToTypeID );
	GetXMLGen().AddChildAttrib( _T("SendToID"), m_SendToID );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	GetXMLGen().AddChildAttrib( _T("WhenToSendFreq"), m_WhenToSendFreq );
	if ( m_WhenToSendFreq == 1 )
	{
		m_WhenToSendVal = m_WhenToSendVal/60;
	}
	else if ( m_WhenToSendFreq == 2 )
	{
		m_WhenToSendVal = m_WhenToSendVal/1440;
	}
	GetXMLGen().AddChildAttrib( _T("WhenToSendVal"), m_WhenToSendVal );
	GetXMLGen().AddChildAttrib( _T("WhenToSendTypeID"), m_WhenToSendTypeID );
	if ( m_SrToSendID == 0 )
	{
		GetXMLGen().AddChildAttrib( _T("SrToSend"), " " );
	}
	else
	{
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_SrToSendID );
		BINDCOL_TCHAR( GetQuery(), nStdRespName );
		GetQuery().Execute( _T("SELECT Subject ")
							_T("FROM StandardResponses ")
							_T("WHERE StandardResponseID=?") );
		GetQuery().Fetch();
		GetXMLGen().AddChildAttrib( _T("SrToSend"), nStdRespName );
	}
	GetXMLGen().AddChildAttrib( _T("SrToSendID"), m_SrToSendID );
	GetXMLGen().AddChildAttrib( _T("SendFromTypeID"), m_SendFromTypeID );
	GetXMLGen().AddChildAttrib( _T("ReplyToAddress"), m_ReplyToAddress );
	GetXMLGen().AddChildAttrib( _T("ReplyToName"), m_ReplyToName );
	GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDate );
	GetXMLGen().AddChildAttrib( _T("DateCreated"), sDate.c_str() );
	GetDateTimeString( m_DateEdited, m_DateEditedLen, sDate );
	GetXMLGen().AddChildAttrib( _T("DateEdited"), sDate.c_str() );
	GetXMLGen().AddChildAttrib( _T("CreatedByID"), m_CreatedByID );
	GetXMLGen().AddChildAttrib( _T("EditedByID"), m_EditedByID );
	GetXMLGen().AddChildAttrib( _T("SendToPercent"), m_SendToPercent );
	GetXMLGen().AddChildAttrib( _T("OmitTracking"), m_OmitTracking );
	GetXMLGen().AddChildAttrib( _T("AutoReplyQuoteMsg"), m_AutoReplyQuoteMsg );
	GetXMLGen().AddChildAttrib( _T("AutoReplyCloseTicket"), m_AutoReplyCloseTicket );
	GetXMLGen().AddChildAttrib( _T("AutoReplyInTicket"), m_AutoReplyInTicket );
	GetXMLGen().AddChildAttrib( _T("HeaderID"), m_HeaderID );
	GetXMLGen().AddChildAttrib( _T("FooterID"), m_FooterID );
	GetXMLGen().AddChildAttrib( _T("FooterLocation"), m_FooterLocation );
	

	// add in the lists
	ListEnabledAgentNames();
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

	ListTicketBoxHeaders();
	ListTicketBoxFooters();
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Create a new or update an existing Auto Response           
\*--------------------------------------------------------------------------*/
void CAutoResponses::DecodeForm(void)
{
	int nAgentID = GetSession().m_AgentID;
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );

	GetISAPIData().GetURLLong( _T("ID"), m_AutoResponseID  );
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, AUTOMESSAGES_DESCRIPTION_LENGTH );
	GetISAPIData().GetXMLLong( _T("SendToTypeID"), m_SendToTypeID );
	if ( m_SendToTypeID == 0 )
	{
		GetISAPIData().GetXMLLong( _T("SendToTbID"), m_SendToID );
	}
	else
	{
		GetISAPIData().GetXMLLong( _T("SendToTcID"), m_SendToID );	
	}
	GetISAPIData().GetXMLLong( _T("IsEnabled"), m_IsEnabled );
	GetISAPIData().GetXMLLong( _T("WhenToSendVal"), m_WhenToSendVal );
	GetISAPIData().GetXMLLong( _T("WhenToSendFreq"), m_WhenToSendFreq );
	if ( m_WhenToSendFreq == 1 )
	{
		m_WhenToSendVal = m_WhenToSendVal*60;
	}
	else if ( m_WhenToSendFreq == 2 )
	{
		m_WhenToSendVal = m_WhenToSendVal*1440;
	}
	GetISAPIData().GetXMLLong( _T("WhenToSendTypeID"), m_WhenToSendTypeID );
	GetISAPIData().GetXMLLong( _T("SrToSendID"), m_SrToSendID );
	GetISAPIData().GetXMLLong( _T("AutoReplyQuoteMsg"), m_AutoReplyQuoteMsg );
	GetISAPIData().GetXMLLong( _T("AutoReplyCloseTicket"), m_AutoReplyCloseTicket );
	GetISAPIData().GetXMLLong( _T("AutoReplyInTicket"), m_AutoReplyInTicket );
	GetISAPIData().GetXMLLong( _T("HeaderID"), m_HeaderID );
	GetISAPIData().GetXMLLong( _T("FooterID"), m_FooterID );
	GetISAPIData().GetXMLLong( _T("FooterLocation"), m_FooterLocation );
	GetISAPIData().GetXMLLong( _T("SendFromTypeID"), m_SendFromTypeID );
	if ( m_SendFromTypeID == 2 )
	{
		GetISAPIData().GetXMLTCHAR( _T("ReplyToAddress"), m_ReplyToAddress, TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH );
		GetISAPIData().GetXMLTCHAR( _T("ReplyToName"), m_ReplyToName, TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH );	
	}
	GetISAPIData().GetXMLLong( _T("SendToPercent"), m_SendToPercent );
	GetISAPIData().GetXMLLong( _T("OmitTracking"), m_OmitTracking );
	
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_AutoResponseID );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT AutoResponseID FROM AutoResponses ")
						_T("WHERE AutoResponseID<>? AND Description=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("That auto response already exists!")  );

	if ( m_AutoResponseID )
	{
		//Set edit date and agent
		m_EditedByID = nAgentID;
		m_DateEdited = Now;

		//Update existing
		Update( GetQuery() );

		if ( GetQuery().GetRowCount() != 1 )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to edit the auto response, it may no longer exist.") );
	}
	else
	{
		//Set create and edit agent
		m_CreatedByID = nAgentID;
		m_EditedByID = nAgentID;
		
		//Create new
		Insert( GetQuery() );

		if ( !m_AutoResponseID )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to insert the auto response.") );
	}

	InvalidateAutoResponses();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes one or more AutoResponses	              
\*--------------------------------------------------------------------------*/
void CAutoResponses::DeleteAutoResponse(void)
{
	CEMSString sIDs;
	TAutoResponses cs;

	GetISAPIData().GetXMLString( _T("SELECTID"), sIDs  );
	
	while ( sIDs.CDLGetNextInt( m_AutoResponseID ) )
	{
		if ( m_AutoResponseID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("This is the default auto response and can't be deleted!") );

		// Delete existing records for this Auto Response
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_AutoResponseID );
		GetQuery().Execute( _T("DELETE FROM AutoResponsesSent WHERE AutoResponseID = ?") );
					
		//Delete Auto Response
		Delete( GetQuery() );

		if ( GetQuery().GetRowCount() != 1 )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to delete the auto response.") );
	}
	
	InvalidateAutoResponses();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Resets the hit count of a Auto Response	              
\*--------------------------------------------------------------------------*/
void CAutoResponses::ResetHitCount( void )
{
	// Reset the hit count for this Auto Response
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_AutoResponseID );
	GetQuery().Execute( _T("UPDATE AutoResponses SET HitCount=0,FailCount=0,TotalCount=0 WHERE AutoResponseID = ?") );
	

	if ( GetQuery().GetRowCount() != 1 )
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to reset the auto response counts.") );

	
	InvalidateAutoResponses();
	
}