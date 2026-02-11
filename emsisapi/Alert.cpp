/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Alert.cpp,v 1.2 2005/11/29 21:16:25 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Support for viewing AlertMsgs   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "Alert.h"
#include "DateFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construciton	              
\*--------------------------------------------------------------------------*/
CAlert::CAlert(CISAPIData& ISAPIData) : CXMLDataClass( ISAPIData ) 
{
	m_bRefreshRequired = false;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CAlert::Run(CURLAction& action)
{
	CEMSString sID;
	tstring sAction;

	GetISAPIData().GetURLString( _T("ID"), sID );

	// Grab the first ID from the list
	sID.CDLInit();
	sID.CDLGetNextInt( m_AlertMsgID );

	if (GetISAPIData().GetFormString( _T("ACTION"), sAction, true ))
	{
		if ( sAction.compare( _T("delete") ) == 0 )
			Delete();

		// generate blank XML
		GenerateXML();
	}
	else
	{
		Query();
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Queries an Alert and generates the required XML 	              
\*--------------------------------------------------------------------------*/
void CAlert::Query( void )
{
	// query the AlertMsg
	if ( TAlertMsgs::Query( GetQuery() ) != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("The AlertID (%d) is invalid"), m_AlertMsgID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	// check security
	RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_READ_ACCESS );

	// mark the alert msg as read
	if (GetAgentID() == m_AgentID)
		MarkAsRead();
	
	// generate XML
	GenerateXML();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes an Alert	             
\*--------------------------------------------------------------------------*/
void CAlert::Delete( void )
{
	// check security
	RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_DELETE_ACCESS );

	if ( TAlertMsgs::Delete( GetQuery() ) != 1 )
	{
		CEMSString sError;
		sError.Format( _T("The AlertID (%d) is invalid"), m_AlertMsgID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	m_bRefreshRequired = true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Marks an AlertMsg as read	              
\*--------------------------------------------------------------------------*/
void CAlert::MarkAsRead( void )
{
	GetQuery().Initialize();

	BINDPARAM_LONG( GetQuery(), m_AlertMsgID );

	GetQuery().Execute( _T("UPDATE AlertMsgs SET Viewed=1 WHERE AlertMsgID=?") );

	if ( GetQuery().GetRowCount() != 1 )
	{
		CEMSString sError;
		sError.Format( _T("The AlertID (%d) is invalid"), m_AlertMsgID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
			
}

/*---------------------------------------------------------------------------\                     
||  Comments:	GeneratesXML for an AlertMsg	              
\*--------------------------------------------------------------------------*/
void CAlert::GenerateXML( void )
{
	CEMSString sDate;
	GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDate );

	CEMSString myTicketID;
	myTicketID.Format( _T("%d"), m_TicketID );
	
	GetXMLGen().AddChildElem( _T("RefreshList") );
	GetXMLGen().AddChildAttrib( _T("Required"), m_bRefreshRequired ? 1 : 0 );

	GetXMLGen().AddChildElem( _T("AlertMsg") );
	GetXMLGen().AddChildAttrib(_T("alerteventid"), m_AlertEventID);
	GetXMLGen().AddChildAttrib( _T("Viewed"), m_Viewed );
	GetXMLGen().AddChildAttrib( _T("AgentID"), m_AgentID );
	GetXMLGen().AddChildAttrib( _T("DateCreated"), sDate.c_str() );
	AddAlertEventName( _T("Subject"), m_AlertEventID );
		
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("Body") );

	if( m_AlertEventID == EMS_ALERT_EVENT_AGE_ALERT ||
		m_AlertEventID == EMS_ALERT_EVENT_TICKET_CREATED ||
		m_AlertEventID == EMS_ALERT_EVENT_TICKET_ESCALATED_TO_GROUP ||
		m_AlertEventID == EMS_ALERT_EVENT_INBOUND_OWNED_TICKET ||
		m_AlertEventID == EMS_ALERT_EVENT_INBOUND_OWNED_CONTACT ||
		m_AlertEventID == EMS_ALERT_EVENT_TICKET_ASSIGNED)
	{
		tstring sBody = m_Body;
		tstring sTicketIDs;
		CEMSString sChunk;
			
		bool bFirstRow = true;
		tstring sLine;
		int oldPos = 0;
		int newPos = 0;

		// Find where the TicketIDs begin
		int pos = sBody.find( "\n\n" ) + 2;
		sTicketIDs.assign( sBody.c_str() + pos );
		
		sBody.reserve( sBody.size()*2 );
		sBody.resize( pos );

		// Parse ticketIDs using the commas
		while( (newPos = sTicketIDs.find( _T("\r\n"), oldPos )) != tstring::npos )
		{
			sChunk.assign( sTicketIDs.substr( oldPos, newPos - oldPos ) );
			oldPos = newPos + 2;
	
			int pos = sChunk.find( _T('#') );
			if( pos != tstring::npos )
			{
				sLine.assign( sChunk.c_str() + pos );
				sLine.erase(0, 1);
				pos = sLine.find( _T(' ') );

				if( pos != tstring::npos )
				{
					sLine.resize( pos );

					sBody += _T("<BR><a href=\"javascript:ViewTicketHistory('");
					sBody += sLine;	
					sBody += _T("');\">");
					sChunk.EscapeHTML();
					sBody += sChunk;
					sBody += _T("</a>");
				}
			}
		}

		GetXMLGen().SetChildData( sBody.c_str(), TRUE );
	}
	else if( m_AlertEventID == EMS_ALERT_EVENT_REPORT_RAN )
	{
		tstring sBody = m_Body;
		CEMSString sChunk;
		sChunk.Format( _T("View Report Results"));
		
		sBody += _T("<BR><BR><a href=\"javascript:ViewReportDetails(");
		sBody += myTicketID;	
		sBody += _T(");\">");
		sChunk.EscapeHTML();
		sBody += sChunk;
		sBody += _T("</a>");
		
		GetXMLGen().SetChildData( sBody.c_str(), TRUE );
	}
	else
	{
		GetXMLGen().SetChildData( m_Body, TRUE );
	}

	GetXMLGen().OutOfElem();
}
