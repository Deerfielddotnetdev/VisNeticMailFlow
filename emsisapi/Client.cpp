/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Client.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "Client.h"
#include "ContactFns.h"

//Note: Remember to set sRequiredClientVersion in ISAPIThread.cpp!

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CClient::CClient(CISAPIData& ISAPIData) : CPagedList(ISAPIData)
{
	m_nUnreadCount = 0;
	m_nTotalCount = 0;
	m_nMaxAlertMsgID = 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CClient::Run(CURLAction& action)
{
	tstring sAction;
	
	// read the action
	if (GetISAPIData().GetURLString( _T("Action"), sAction, true))
	{	
		if (sAction.compare(_T("alerts_delete")) == 0)
		{
			DeleteAlerts();
		}
		else if (sAction.compare(_T("alerts_delete_all")) == 0)
		{
			DeleteAllAlerts();
		}
		else if (sAction.compare(_T("alerts_list")) == 0)
		{
			List();
		}
		else if (sAction.compare(_T("contact_search")) == 0)
		{
			ContactSearch();
		}
		else if (sAction.compare(_T("contact_details")) == 0)
		{
			ContactDetails();
		}
		else if (sAction.compare(_T("ticket_search")) == 0)
		{
			TicketSearch( action );
		}
	}	

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Search for ticket              
\*--------------------------------------------------------------------------*/
void CClient::TicketSearch( CURLAction& action )
{
	GetISAPIData().GetURLLong( _T("ticketid"), m_nTicketID );
	CEMSString sURL;
	sURL.Format( _T("mailflow.ems?rightpane=ticketsearchresults.ems$action=search$hideleft=1$ticketid=%d$MAXLINES=100$nocancel=1&ck_agentid=0"), m_nTicketID);
	action.SetRedirectURL( (TCHAR*) sURL.c_str() );
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Search for contact	              
\*--------------------------------------------------------------------------*/
void CClient::ContactSearch( void )
{
	if ( GetAgentRightLevel(EMS_OBJECT_TYPE_CONTACT, 0) >= EMS_READ_ACCESS )
	{
		GetISAPIData().GetURLString( _T("phone"), m_sPersonalData, true );
		m_nContactID = 0;

		if ( m_sPersonalData.length() > 0 )
		{
			
			std::string sTemp = "LIKE '";
			sTemp.append(m_sPersonalData.c_str());
			sTemp.append("%'");

			std::string sTemp2 = "'%phone%'";

			CEMSString sSql;
			sSql.Format( _T("SELECT TOP 1 pd.ContactID FROM PersonalData pd ")
						_T("INNER JOIN PersonalDataTypes pdt ON pd.PersonalDataTypeID=pdt.PersonalDataTypeID ")
						_T("WHERE (REVERSE(REPLACE( REPLACE( REPLACE( REPLACE( REPLACE( pd.datavalue, '-', '' ) ")
						_T(", '.', '' ), ' ', '' ), '(', ''), ')', ''))  %s ) AND pdt.TypeName LIKE %s ORDER BY pd.ContactID"),sTemp.c_str(),sTemp2.c_str() );
			
			
			
			GetQuery().Initialize();
			BINDCOL_LONG_NOLEN( GetQuery(), m_nContactID );
			GetQuery().Execute( sSql.c_str() );
			GetQuery().Fetch();				
		}

		
	}
	else
	{
		m_nContactID = -1;
	}
		
	GetXMLGen().AddChildElem( _T("Contact") );			
	GetXMLGen().AddChildAttrib( _T("ContactID"), m_nContactID );	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Get contact details	              
\*--------------------------------------------------------------------------*/
void CClient::ContactDetails( void )
{
	GetISAPIData().GetURLLong( _T("contact"), m_Contact.m_ContactID, true );

	// check security
	m_AccessLevel = RequireAgentRightLevel(EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS);

	m_Contact.Query(GetQuery());

	// list the personal data
	CPersonalDataList DataItem;	
	DataItem.ListContact( GetQuery(), m_Contact.m_ContactID );

	m_nMaxPersonalDataID = 0;

	while( ( GetQuery().Fetch() ) == S_OK )
	{
		if (DataItem.m_PersonalDataID == m_Contact.m_DefaultEmailAddressID)
		{
			m_DefaultEmail = DataItem;
		}
	}

	// generate the XML
	GenContactXML();

}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates XML for the contact	              
\*--------------------------------------------------------------------------*/
void CClient::GenContactXML( void )
{
	GetXMLGen().AddChildElem(_T("Contact"));
	GetXMLGen().AddChildAttrib(_T("ID"), m_Contact.m_ContactID);	
	GetXMLGen().AddChildAttrib(_T("Title"), m_Contact.m_Title);
	GetXMLGen().AddChildAttrib(_T("Name"), m_Contact.m_Name);
	GetXMLGen().AddChildAttrib(_T("Company"), m_Contact.m_CompanyName);
	GetXMLGen().AddChildAttrib(_T("StreetAddress1"), m_Contact.m_StreetAddress1);
	GetXMLGen().AddChildAttrib(_T("StreetAddress2"), m_Contact.m_StreetAddress2);
	GetXMLGen().AddChildAttrib(_T("City"), m_Contact.m_City);
	GetXMLGen().AddChildAttrib(_T("State"), m_Contact.m_State);
	GetXMLGen().AddChildAttrib(_T("Zip"), m_Contact.m_ZipCode);
	GetXMLGen().AddChildAttrib(_T("Country"), m_Contact.m_Country);
	GetXMLGen().AddChildAttrib(_T("Website"), m_Contact.m_WebPageAddress);
	GetXMLGen().AddChildAttrib( _T("DefaultEmailAddress"), m_DefaultEmail.m_DataValue );	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists alerts for an agent	              
\*--------------------------------------------------------------------------*/
void CClient::List( void )
{
	m_nAgentID = GetSession().m_AgentID;
	m_nMaxID = 0;
	GetISAPIData().GetURLLong( _T("maxid"), m_nMaxID, true );

	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), m_nTotalCount );
	BINDCOL_LONG_NOLEN( GetQuery(), m_nUnreadCount );
	BINDCOL_LONG_NOLEN( GetQuery(), m_nMaxAlertMsgID );
	BINDPARAM_LONG( GetQuery(), m_nAgentID );
	BINDPARAM_LONG( GetQuery(), m_nMaxID );
	BINDPARAM_LONG( GetQuery(), m_nAgentID );
	BINDPARAM_LONG( GetQuery(), m_nAgentID );
	BINDPARAM_LONG( GetQuery(), m_nMaxID );
	GetQuery().Execute( _T("SELECT COUNT(*),(SELECT Count(*) FROM AlertMsgs WHERE AgentID=? AND Viewed=0 AND AlertMsgID>?),(SELECT MAX(AlertMsgID) FROM AlertMsgs WHERE AgentID=?) FROM AlertMsgs WHERE AgentID=? AND AlertMsgID>?") );
	GetQuery().Fetch();

	// allocate storage
	TAlertMsgs* pAlertArray = NULL;
	
	if ((pAlertArray = new TAlertMsgs[m_nTotalCount]) == NULL)
		THROW_EMS_EXCEPTION( E_MemoryError, CEMSString(EMS_STRING_ERROR_MEMORY) );
		
	if ( m_nTotalCount )
	{
		// preform the query
		GetQuery().Initialize(m_nTotalCount, sizeof(TAlertMsgs));
		GetQuery().EnableScrollCursor();
		
		
		
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
			// format the query
			CEMSString sQuery;
			sQuery.Format( _T("SELECT TOP %d AlertMsgs.AlertMsgID, AlertMsgs.AlertEventID, AlertMsgs.TicketID, AlertMsgs.DateCreated, AlertMsgs.Viewed, AlertMsgs.Subject, AlertEvents.Description ")
						_T("FROM AlertMsgs INNER JOIN AlertEvents ON AlertMsgs.AlertEventID=AlertEvents.AlertEventID ")
						_T("WHERE AgentID=%d AND AlertMsgID>%d ORDER BY DateCreated DESC"), m_nTotalCount, GetSession().m_AgentID, m_nMaxID );

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
	else
	{
		/*GetXMLGen().AddChildElem( _T("Alerts") );
		GetXMLGen().AddChildAttrib( _T("AgentID"), m_nAgentID );
		GetXMLGen().AddChildAttrib( _T("Total"), "0" );
		GetXMLGen().AddChildAttrib( _T("Unread"), "0" );
		GetXMLGen().AddChildAttrib( _T("MaxID"), "0" );*/
		GenerateXML(pAlertArray);
		if (pAlertArray)
		{
			delete[] pAlertArray;
			pAlertArray = NULL;
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML	              
\*--------------------------------------------------------------------------*/
void CClient::GenerateXML( TAlertMsgs* pAlertArray)
{
	CEMSString sDateTime;
	tstring sEmptyString = "";
	CEMSString strAlertSubject;

	GetXMLGen().AddChildElem( _T("Alerts") );
	GetXMLGen().AddChildAttrib( _T("AgentID"), GetSession().m_AgentID );
	GetXMLGen().AddChildAttrib( _T("Total"), m_nTotalCount );
	GetXMLGen().AddChildAttrib( _T("Unread"), m_nUnreadCount );
	GetXMLGen().AddChildAttrib( _T("MaxID"), m_nMaxAlertMsgID );
	GetXMLGen().IntoElem();
	
	for(UINT i = 0; i < GetQuery().GetRowsFetched(); i++)
	{
		GetDateTimeString(pAlertArray[i].m_DateCreated, pAlertArray[i].m_DateCreatedLen, sDateTime );
		
		GetXMLGen().AddChildElem(_T("Alert"));
		GetXMLGen().AddChildAttrib( _T("ID"), pAlertArray[i].m_AlertMsgID );
		GetXMLGen().AddChildAttrib( _T("Viewed"), GetSession().m_AgentID ? pAlertArray[i].m_Viewed : 1 );
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDateTime.c_str() );
		GetXMLGen().AddChildAttrib( _T("AlertEventID"), pAlertArray[i].m_AlertEventID );
		GetXMLGen().AddChildAttrib( _T("TicketID"), pAlertArray[i].m_TicketID );
		GetXMLGen().AddChildAttrib( _T("AlertEventName"), pAlertArray[i].m_AlertName );
		if( ( pAlertArray[i].m_AlertEventID == 9 || pAlertArray[i].m_AlertEventID == 10 ) && pAlertArray[i].m_Subject != sEmptyString )
		{
			GetXMLGen().AddChildAttrib( _T("Subject"), pAlertArray[i].m_Subject );
		}
		/*else if(  pAlertArray[i].m_AlertEventID == 12 || pAlertArray[i].m_AlertEventID == 13|| pAlertArray[i].m_AlertEventID == 14 )
		{
			strAlertSubject.Format( _T("%s :: [%d]"), pAlertArray[i].m_AlertName, pAlertArray[i].m_TicketID );
			GetXMLGen().AddChildAttrib( _T("Subject"), strAlertSubject.c_str() );
		}
		*/else if ( pAlertArray[i].m_Subject != sEmptyString )
		{
            strAlertSubject.Format( _T("%s :: %s"), pAlertArray[i].m_AlertName, pAlertArray[i].m_Subject );
			GetXMLGen().AddChildAttrib( _T("Subject"), strAlertSubject.c_str() );
		}
		else
		{
            GetXMLGen().AddChildAttrib( _T("Subject"), pAlertArray[i].m_AlertName );
		}
	}
	
	GetXMLGen().OutOfElem();	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes a comma delimited list of alerts	              
\*--------------------------------------------------------------------------*/
void CClient::DeleteAlerts( void )
{
	CEMSString sQuery;
	CEMSString sAlertIDs;
	tstring sChunk;

	// must have delete rights for the agent
	m_Access = RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, GetSession().m_AgentID, EMS_DELETE_ACCESS );

	GetISAPIData().GetFormString( _T("selectId"), sAlertIDs);
	
	GetQuery().Initialize();

	BINDPARAM_LONG( GetQuery(), GetSession().m_AgentID );

	while ( sAlertIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		sQuery.Format( _T("DELETE FROM AlertMsgs WHERE AlertMsgID IN (%s) AND AgentID=?"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes all alerts for the agent	              
\*--------------------------------------------------------------------------*/
void CClient::DeleteAllAlerts( void )
{
	// must have delete rights for the agent
	m_Access = RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, GetSession().m_AgentID, EMS_DELETE_ACCESS );

	GetQuery().Initialize();

	BINDPARAM_LONG( GetQuery(), GetSession().m_AgentID );
	GetQuery().Execute( _T("DELETE FROM AlertMsgs WHERE AgentID=?") );
}
