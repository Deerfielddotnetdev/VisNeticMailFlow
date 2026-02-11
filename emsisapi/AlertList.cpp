/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/AlertList.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "AlertList.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CAlertList::CAlertList(CISAPIData& ISAPIData) : CPagedList(ISAPIData)
{
	m_Access = EMS_NO_ACCESS;
	m_nUnreadCount = 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CAlertList::Run(CURLAction& action)
{
	tstring sAction;
	
	// get the ID
	GetISAPIData().GetURLLong( _T("TicketBoxView"), m_TBView.m_TicketBoxViewID );
		
	// get info about the TicketBoxView
	m_TBView.Query( GetQuery() );
		
	if ( m_TBView.m_ViewTypeID != EMS_ALERTS ) 
		THROW_EMS_EXCEPTION( E_InvalidID, _T("The specified TicketBoxView cannot use the alert display"));
		
	if( m_TBView.m_AgentBoxID == GetSession().m_AgentID )
	{
		action.m_sPageTitle.assign( _T("Alerts") );
	}
	else
	{
		CEMSString sPageName;
		tstring sAgentName;
		GetAgentName( m_TBView.m_AgentBoxID, sAgentName );
		sPageName.Format( _T("Alerts for %s"), sAgentName.c_str() );
		action.m_sPageTitle.assign( sPageName );
	}
	
	// read the action
	if (GetISAPIData().GetFormString( _T("Action"), sAction, true))
	{	
		if (sAction.compare(_T("delete")) == 0)
			DeleteAlerts();
		
		else if (sAction.compare(_T("delete_all")) == 0)
			DeleteAllAlerts();
	}
			
	List();

	return 0;
}
	
/*---------------------------------------------------------------------------\                     
||  Comments:	Lists alerts for an agent	              
\*--------------------------------------------------------------------------*/
void CAlertList::List( void )
{
	// must have delete rights for the agent
	m_Access = RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_TBView.m_AgentBoxID, EMS_READ_ACCESS );

	// update the page count
	SetItemCount( m_TBView.GetItemCount( GetQuery() ) );
	
	// get the number of unread alerts
	if ( m_TBView.m_AgentBoxID == GetAgentID() )
	{
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), m_nUnreadCount );
		BINDPARAM_LONG( GetQuery(), m_TBView.m_AgentBoxID );
		GetQuery().Execute( _T("SELECT COUNT(*) FROM AlertMsgs WHERE AgentID=? AND Viewed=0") );
		GetQuery().Fetch();
	}
	
	// preform the query
	GetQuery().Initialize(GetMaxRowsPerPage(), sizeof(TAlertMsgs));
	GetQuery().EnableScrollCursor();
	
	// allocate storage
	TAlertMsgs* pAlertArray = NULL;
	
	if ((pAlertArray = new TAlertMsgs[GetMaxRowsPerPage()]) == NULL)
		THROW_EMS_EXCEPTION( E_MemoryError, CEMSString(EMS_STRING_ERROR_MEMORY) );
	
	try
	{
		// bind
		BINDCOL_LONG(GetQuery(), pAlertArray[0].m_AlertMsgID);
		BINDCOL_LONG(GetQuery(), pAlertArray[0].m_AlertEventID);
		BINDCOL_LONG(GetQuery(), pAlertArray[0].m_TicketID);
		BINDCOL_TIME(GetQuery(), pAlertArray[0].m_DateCreated);
		BINDCOL_BIT(GetQuery(), pAlertArray[0].m_Viewed);
		BINDCOL_TCHAR(GetQuery(), pAlertArray[0].m_Subject );
		BINDCOL_TCHAR(GetQuery(), pAlertArray[0].m_AlertName );
        BINDPARAM_LONG(GetQuery(), m_TBView.m_AgentBoxID);

		// format the query
		CEMSString sQuery;
		sQuery.Format( _T("SELECT TOP %d AlertMsgs.AlertMsgID, AlertMsgs.AlertEventID, AlertMsgs.TicketID, AlertMsgs.DateCreated, AlertMsgs.Viewed, AlertMsgs.Subject, AlertEvents.Description ")
					   _T("FROM AlertMsgs INNER JOIN AlertEvents ON AlertMsgs.AlertEventID=AlertEvents.AlertEventID ")
					   _T("WHERE AgentID=? ORDER BY DateCreated DESC"), GetEndRow() );

		// execute the query
		GetQuery().Execute( sQuery.c_str() );

		// fetch the data
		GetQuery().FetchScroll( SQL_FETCH_ABSOLUTE, GetStartRow() );
		
		// generate the XML
		GenerateXML(pAlertArray);
		
		// close the cursor
		GetQuery().Reset();
	}
	catch(...)
	{
		if (pAlertArray)
		{
			delete[] pAlertArray;
			pAlertArray = NULL;
		}
		
		throw;
	}
	
	if (pAlertArray)
	{
		delete[] pAlertArray;
		pAlertArray = NULL;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML	              
\*--------------------------------------------------------------------------*/
void CAlertList::GenerateXML( TAlertMsgs* pAlertArray)
{
	CEMSString sDateTime;
	tstring sEmptyString = "";
	CEMSString strAlertSubject;

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;

	// add the access level
	GetXMLGen().AddChildElem( _T("Access") );
	GetXMLGen().AddChildAttrib( _T("Level"), m_Access );

	// add page count information
	AddPageXML();
	GetXMLGen().AddChildAttrib( _T("Unread"), m_nUnreadCount );
	
	// add ticketbox information
	GetXMLGen().AddChildElem( _T("AlertList") );
	GetXMLGen().AddChildAttrib( _T("TicketBoxView"), m_TBView.m_TicketBoxViewID );
	GetXMLGen().AddChildAttrib( _T("SortBy"), _T("-1") );
	
	GetXMLGen().AddChildAttrib( _T("AgentID"), m_TBView.m_AgentBoxID );
	AddAgentName( _T("Agent"), m_TBView.m_AgentBoxID );
	AddTicketBoxViewTypeName( _T("TicketBoxName"), m_TBView.m_ViewTypeID );
	
	GetXMLGen().IntoElem();
	
	for(UINT i = 0; i < GetQuery().GetRowsFetched(); i++)
	{
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(pAlertArray[i].m_DateCreated,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDateTime );
			}
			else
			{
				GetDateTimeString(pAlertArray[i].m_DateCreated, pAlertArray[i].m_DateCreatedLen, sDateTime );
			}
		}
		else
		{
			GetDateTimeString(pAlertArray[i].m_DateCreated, pAlertArray[i].m_DateCreatedLen, sDateTime );
		}
				
		GetXMLGen().AddChildElem(_T("Item"));
		GetXMLGen().AddChildAttrib( _T("ID"), pAlertArray[i].m_AlertMsgID );
		GetXMLGen().AddChildAttrib( _T("Viewed"), m_TBView.m_AgentBoxID == GetAgentID() ? pAlertArray[i].m_Viewed : 1 );
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDateTime.c_str() );
		GetXMLGen().AddChildAttrib( _T("AlertEventID"), pAlertArray[i].m_AlertEventID );
		GetXMLGen().AddChildAttrib( _T("TicketID"), pAlertArray[i].m_TicketID );
		if( ( pAlertArray[i].m_AlertEventID == 9 || pAlertArray[i].m_AlertEventID == 10 ||
			pAlertArray[i].m_AlertEventID == 12 || pAlertArray[i].m_AlertEventID == 13 ||
			pAlertArray[i].m_AlertEventID == 14) && pAlertArray[i].m_Subject != sEmptyString )
		{
			
			GetXMLGen().AddChildAttrib( _T("Subject"), pAlertArray[i].m_Subject );
		}
		else if ( pAlertArray[i].m_Subject != sEmptyString )
		{
            strAlertSubject.Format( _T("%s :: %s"), pAlertArray[i].m_AlertName, pAlertArray[i].m_Subject );
			GetXMLGen().AddChildAttrib( _T("Subject"), strAlertSubject.c_str() );
		}
		else
		{
            AddAlertEventName( _T("Subject"), pAlertArray[i].m_AlertEventID );
		}
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes a comma delimited list of alerts	              
\*--------------------------------------------------------------------------*/
void CAlertList::DeleteAlerts( void )
{
	CEMSString sQuery;
	CEMSString sAlertIDs;
	tstring sChunk;

	// must have delete rights for the agent
	m_Access = RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_TBView.m_AgentBoxID, EMS_DELETE_ACCESS );

	GetISAPIData().GetFormString( _T("selectId"), sAlertIDs);
	
	GetQuery().Initialize();

	BINDPARAM_LONG( GetQuery(), m_TBView.m_AgentBoxID );

	while ( sAlertIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		sQuery.Format( _T("DELETE FROM AlertMsgs WHERE AlertMsgID IN (%s) AND AgentID=?"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes all alerts for the agent	              
\*--------------------------------------------------------------------------*/
void CAlertList::DeleteAllAlerts( void )
{
	// must have delete rights for the agent
	m_Access = RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_TBView.m_AgentBoxID, EMS_DELETE_ACCESS );

	GetQuery().Initialize();

	BINDPARAM_LONG( GetQuery(), m_TBView.m_AgentBoxID );
	GetQuery().Execute( _T("DELETE FROM AlertMsgs WHERE AgentID=?") );
}
