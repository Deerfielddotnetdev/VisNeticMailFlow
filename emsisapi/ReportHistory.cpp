/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportHistory.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ReportHistory.h"
#include "ContactFNS.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CReportHistory::CReportHistory( CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_nType = 0;
	m_nFilter = 0;
	m_nMsgFilter = 0;
	m_nDateRange = 0;

	ZeroMemory( &m_StartDate, sizeof(m_StartDate) );
	ZeroMemory( &m_EndDate, sizeof(m_EndDate) );
	m_EndDate.hour = 23;
	m_EndDate.minute = 59;
	m_EndDate.second = 59;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main entry point	              
\*--------------------------------------------------------------------------*/
int CReportHistory::Run( CURLAction& action )
{
	try
	{
		tstring sAction;
		
		if( GetISAPIData().GetURLString( _T("Action"), sAction, true ) )
		{
			if( sAction.compare( _T("download") ) == 0 )
				OpenCDF(action);
		}
		
		DecodeForm();

		if ( !m_OutputCDF )
			AddReportOptions();

		if ( m_bPost )
		{
			Query();
			AddReportHeader();
		}
	}
	catch(...)
	{
		if ( m_OutputCDF )
		{
			CloseCDF();
			DeleteFile( m_szTempFile );
		}

		throw;
	}

	CloseCDF();
	return 0;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Decode URL Parameters	              
\*--------------------------------------------------------------------------*/
void CReportHistory::DecodeForm()
{
	GetISAPIData().GetURLLong( _T("type"), m_nType );
	
	ISAPIPARAM param;
	
	if ( GetISAPIData().FindURLParam( _T("MAX_LINES"), param ) )
	{
		GetISAPIData().GetURLLong( _T("TICKETBOX"), m_nFilter, true );
		GetISAPIData().GetURLLong( _T("AGENT"), m_nFilter, true );
		GetISAPIData().GetURLString( _T("CONTACT"), m_sContact, true );
		GetISAPIData().GetURLLong( _T("TICKET_CATEGORY"), m_nFilter, true );
		

		if ( m_nFilter || m_nType == 2 || m_nType == 3 )
			m_bPost = true;

		GetISAPIData().GetURLLong( _T("MSG_FILTER"), m_nMsgFilter );
		GetISAPIData().GetURLLong( _T("DATE_RANGE_SEL"), m_nDateRange, true );
		GetISAPIData().GetURLLong( _T("MAX_LINES"), m_MaxRows );
		
		long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
		TIMESTAMP_STRUCT tsLocal;
		long tsLocalLen=0;

		CEMSString sDate;
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );

		if(lTzBias != 1)
		{
			if(ConvertFromTimeZone(m_StartDate,lTzBias,tsLocal))
			{
				m_StartDate.year = tsLocal.year;
				m_StartDate.month = tsLocal.month;
				m_StartDate.day = tsLocal.day;
				m_StartDate.hour = tsLocal.hour;
				m_StartDate.minute = tsLocal.minute;
				m_StartDate.second = tsLocal.second;
			}			
		}
		
		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EndDate );

		if(lTzBias != 1)
		{
			if(ConvertFromTimeZone(m_EndDate,lTzBias,tsLocal))
			{
				m_EndDate.year = tsLocal.year;
				m_EndDate.month = tsLocal.month;
				m_EndDate.day = tsLocal.day;
				m_EndDate.hour = tsLocal.hour;
				m_EndDate.minute = tsLocal.minute;
				m_EndDate.second = tsLocal.second;
			}			
		}
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query the database	              
\*--------------------------------------------------------------------------*/
void CReportHistory::Query()
{
	TInboundMessages msg;
	CEMSString sSQL;
	CEMSString sDate; 
	CEMSString sTime;
	CEMSString sLine;
	CEMSString sSubject;
	CEMSString sFrom;
	CEMSString sTo;
	int nIsInbound;

	// add column names for CDF
	if ( m_OutputCDF )
	{
		CEMSString sLine( _T("Message Type,Date,Subject,From,To,TicketID\r\n") );
		OutputCDF( sLine );
	}

	// get the SQL query string
	switch ( m_nType )
	{
	case 0:
		FormatAgentQuery( sSQL );
		break;
	case 1:
		FormatTicketBoxQuery( sSQL );
		break;
	case 2:
		FormatContactQuery( sSQL );
		break;
	case 3:
		FormatTicketCategoryQuery( sSQL );
		break;
	}
	
	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;

	GetQuery().Initialize();

	// bind to database columns
	BINDCOL_LONG(  GetQuery(), msg.m_InboundMessageID );
	BINDCOL_LONG(  GetQuery(), msg.m_TicketID );
	BINDCOL_TCHAR( GetQuery(), msg.m_Subject );
	BINDCOL_TIME(  GetQuery(), msg.m_EmailDateTime );
	BINDCOL_TCHAR( GetQuery(), msg.m_EmailFrom );
	BINDCOL_LONG( GetQuery(), msg.m_ContactID );
	BINDCOL_LONG_NOLEN( GetQuery(), nIsInbound );

	// bind query parameters
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );

	GetQuery().Execute( sSQL.c_str() );
	int nRows = 0;
	for ( nRows=0; GetQuery().Fetch() == S_OK; nRows++ )
	{
		if( nRows == m_MaxRows )
		{
			if( m_OutputCDF )
			{
				CEMSString sString;
				sString.Format( _T("Max row limit reached\r\n") );
				OutputCDF( sString );
			}
			else
			{
				GetXMLGen().AddChildElem( _T("Information"),  _T("Max line limit reached") );
			}

			break;
		}

		GETDATA_TEXT( GetQuery(), msg.m_EmailTo );

		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(msg.m_EmailDateTime,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDate, true, false );
				GetDateTimeString( tsLocal, tsLocalLen, sTime, false, true );
			}
			else
			{
				GetDateTimeString( msg.m_EmailDateTime, msg.m_EmailDateTimeLen, sDate, true, false );
				GetDateTimeString( msg.m_EmailDateTime, msg.m_EmailDateTimeLen, sTime, false, true );
			}
		}
		else
		{
			GetDateTimeString( msg.m_EmailDateTime, msg.m_EmailDateTimeLen, sDate, true, false );
			GetDateTimeString( msg.m_EmailDateTime, msg.m_EmailDateTimeLen, sTime, false, true );
		}		

		sSubject = msg.m_SubjectLen ? msg.m_Subject : _T("[No Subject]");
		sFrom = msg.m_EmailFrom;
		sTo = msg.m_EmailTo;

		if( m_OutputCDF )
		{
			// escape any commas
			sDate.EscapeCSV();
			sTime.EscapeCSV();
			sSubject.EscapeCSV();
			sFrom.EscapeCSV();
			sTo.EscapeCSV();
			
			// format the line
			sLine.Format( _T("\"%s\",\"%s %s\",\"%s\",\"%s\",\"%s\",\"%d\"\r\n"), 
				          nIsInbound ? _T("Inbound") : _T("Outbound"), sDate.c_str(), sTime.c_str(), 
						  sSubject.c_str(), sFrom.c_str(), sTo.c_str(), msg.m_TicketID );

			// add the line to the comma delimited file
			OutputCDF( sLine );
		}
		else
		{
			sSubject.EscapeHTML();

			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("MsgID"), msg.m_InboundMessageID );
			GetXMLGen().AddChildAttrib( _T("IsInbound"), nIsInbound );
			GetXMLGen().AddChildAttrib( _T("TicketID"), msg.m_TicketID );
			
			if ( nIsInbound )
				GetXMLGen().AddChildAttrib( _T("ContactID"), msg.m_ContactID );

			GetXMLGen().IntoElem();
			{
				GetXMLGen().AddChildElem( _T("Date"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("Time"), sTime.c_str() );
				GetXMLGen().AddChildElem( _T("Subject"), sSubject.c_str() );
				GetXMLGen().AddChildElem( _T("From"), sFrom.c_str() );
				FormatEmailAddrString( &msg.m_EmailTo, msg.m_EmailToLen, msg.m_EmailToAllocated);
				GetXMLGen().AddChildElem( _T("To"), msg.m_EmailTo );
			}
			GetXMLGen().OutOfElem();
		}
	}

	if ( m_OutputCDF && nRows == 0 )
		OutputCDF( CEMSString( _T("No results returned\r\n") ) );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Build the agent history query string	              
\*--------------------------------------------------------------------------*/
void CReportHistory::FormatAgentQuery( CEMSString& sSQL )
{
	CEMSString sJoin;
	CEMSString sWhere;
	CEMSString sFilter;
	CEMSString sTemp;

	// check security
	if ( m_nFilter > 0 )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_nFilter, EMS_READ_ACCESS );
	}
	else
	{
		RequireAdmin();
	}

	GetSecurityQueryStrings( sJoin, sWhere );

	sSQL.Format( _T("SET ROWCOUNT %d "), m_MaxRows + 1 );

	if ( m_nMsgFilter == 0 || m_nMsgFilter == 1 )
	{
		if ( m_nFilter > 0 )
		{
			// only include messages that were in the ticketbox when there were created
			sFilter.Format( "AND OriginalOwnerID=%d", m_nFilter );
		}
	
		sTemp.Format( _T("SELECT InboundMessageID,M.TicketID,M.Subject,DateReceived,EmailFrom,ContactID,1,EmailTo ")
					  _T("FROM InboundMessages as M %s ")
					  _T("WHERE M.IsDeleted = 0 AND DateReceived BETWEEN ? AND ? %s %s "), 
						sJoin.c_str(), sFilter.c_str(), sWhere.c_str() );

		sSQL += sTemp;

		if ( m_nMsgFilter == 1 )
			sSQL += _T("ORDER BY 4");
	}

	if ( m_nMsgFilter == 0 )
		sSQL += _T(" UNION ALL ");
	
	if ( m_nMsgFilter == 0 || m_nMsgFilter == 2 )
	{
		if ( m_nFilter > 0 )
			sFilter.Format( _T("AND M.AgentID = %d"), m_nFilter );

		sTemp.Format( _T("SELECT OutboundMessageID,M.TicketID,M.Subject,EmailDateTime,EmailFrom,0,0,EmailTo ")
					  _T("FROM OutboundMessages as M %s ")
					  _T("WHERE M.IsDeleted = 0 AND OutboundMessageStateID>1 ")
					  _T("AND EmailDateTime BETWEEN ? AND ? %s %s ")
					  _T("ORDER BY 4"), sJoin.c_str(), sFilter.c_str(), sWhere.c_str() );

		sSQL += sTemp;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Build the TicketBox history query string	              
\*--------------------------------------------------------------------------*/
void CReportHistory::FormatTicketBoxQuery( CEMSString& sSQL )
{
	CEMSString sString;
	CEMSString sFilter;
	CEMSString sJoin;
	CEMSString sWhere;
	
	// check security
	if ( m_nFilter > 0 )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, m_nFilter, EMS_READ_ACCESS );
	}
	else
	{
		RequireAdmin();
	}

	GetSecurityQueryStrings( sJoin, sWhere );

	sSQL.Format( _T("SET ROWCOUNT %d "), m_MaxRows + 1 );

	// query for the inbound message
	if ( m_nMsgFilter == 0 || m_nMsgFilter == 1 )
	{
		// only include messages that were in the TicketBox when they were created
		if ( m_nFilter > 0 )
		{
			// only include messages that were in the ticketbox when there were created
			sFilter.Format( "AND OriginalTicketBoxID=%d", m_nFilter );
		}

		sString.Format( _T("SELECT InboundMessageID,M.TicketID,M.Subject,DateReceived,EmailFrom,ContactID,1,EmailTo ")
					    _T("FROM InboundMessages as M %s ")
					    _T("WHERE M.IsDeleted = 0 AND DateReceived BETWEEN ? AND ? %s %s "), 
						sJoin.c_str(), sFilter.c_str(), sWhere.c_str() );

		sSQL += sString;

		if ( m_nMsgFilter == 1 )
			sSQL += _T("ORDER BY 4");
	}

	if ( m_nMsgFilter == 0 )
		sSQL += _T(" UNION ALL ");
	
	// include outbound messages
	if ( m_nMsgFilter == 0 || m_nMsgFilter == 2 )
	{
		// only include messages that were in the TicketBox when there were createdcreated
		if ( m_nFilter > 0 )
		{
			// only include messages that were in the ticketbox when there were created
			sFilter.Format( "AND OriginalTicketBoxID=%d", m_nFilter );
		}

		sString.Format( _T("SELECT OutboundMessageID,M.TicketID,M.Subject,EmailDateTime,EmailFrom,0,0,EmailTo ")
						_T("FROM OutboundMessages as M %s ")
						_T("WHERE M.IsDeleted=0  AND OutboundMessageStateID>1 ")
						_T("AND EmailDateTime BETWEEN ? AND ? %s %s ORDER BY 4"), 
						sJoin.c_str(), sFilter.c_str(), sWhere.c_str() );

		sSQL += sString;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Build the Ticket Category history query string	              
\*--------------------------------------------------------------------------*/
void CReportHistory::FormatTicketCategoryQuery( CEMSString& sSQL )
{
	CEMSString sString;
	CEMSString sFilter;
	CEMSString sJoin;
	CEMSString sWhere;
	
	// check security
	if ( m_nFilter > 0 )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_CATEGORY, m_nFilter, EMS_READ_ACCESS );
	}
	else
	{
		RequireAdmin();
	}

	GetSecurityQueryStrings( sJoin, sWhere );

	sSQL.Format( _T("SET ROWCOUNT %d "), m_MaxRows + 1 );

	// query for the inbound message
	if ( m_nMsgFilter == 0 || m_nMsgFilter == 1 )
	{
		// only include messages that were in the TicketBox when they were created
		if ( m_nFilter > 0 )
		{
			// only include messages that were in the ticket category when they were created
			sFilter.Format( "AND OriginalTicketCategoryID=%d", m_nFilter );
		}

		sString.Format( _T("SELECT InboundMessageID,M.TicketID,M.Subject,DateReceived,EmailFrom,ContactID,1,EmailTo ")
					    _T("FROM InboundMessages as M %s ")
					    _T("WHERE M.IsDeleted = 0 AND DateReceived BETWEEN ? AND ? %s %s "), 
						sJoin.c_str(), sFilter.c_str(), sWhere.c_str() );

		sSQL += sString;

		if ( m_nMsgFilter == 1 )
			sSQL += _T("ORDER BY 4");
	}

	if ( m_nMsgFilter == 0 )
		sSQL += _T(" UNION ALL ");
	
	// include outbound messages
	if ( m_nMsgFilter == 0 || m_nMsgFilter == 2 )
	{
		// only include messages that were in the TicketBox when there were createdcreated
		if ( m_nFilter > 0 )
		{
			// only include messages that were in the ticket category when they were created
			sFilter.Format( "AND M.TicketCategoryID=%d", m_nFilter );
		}

		sString.Format( _T("SELECT OutboundMessageID,M.TicketID,M.Subject,EmailDateTime,EmailFrom,0,0,EmailTo ")
						_T("FROM OutboundMessages as M %s ")
						_T("WHERE M.IsDeleted=0  AND OutboundMessageStateID>1 ")
						_T("AND EmailDateTime BETWEEN ? AND ? %s %s ORDER BY 4"), 
						sJoin.c_str(), sFilter.c_str(), sWhere.c_str() );

		sSQL += sString;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Build the contact history query string		              
\*--------------------------------------------------------------------------*/
void CReportHistory::FormatContactQuery( CEMSString& sSQL )
{
	CEMSString sString;
	CEMSString sFilter;
	CEMSString sJoin;
	CEMSString sWhere;
	int nContactID = -1;
	
	// check security
	RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS );
	GetSecurityQueryStrings( sJoin, sWhere );

	sSQL.Format( _T("SET ROWCOUNT %d "), m_MaxRows + 1 );

	// query for the inbound messages
	if ( m_nMsgFilter == 0 || m_nMsgFilter == 1 )
	{
		// only include messages that were in the TicketBox when they were created
		nContactID = FindContactByEmail( GetQuery(), m_sContact.c_str() );
		sFilter.Format( _T("AND M.ContactID=%d"), nContactID );
			
		sString.Format( _T("SELECT InboundMessageID,M.TicketID,M.Subject,DateReceived,")
						_T("EmailFrom,M.ContactID,1,EmailTo ")
					    _T("FROM InboundMessages M INNER JOIN Contacts C on M.ContactID=C.ContactID %s ")
					    _T("WHERE M.IsDeleted=0 AND C.IsDeleted=0 ")
						_T("AND DateReceived BETWEEN ? AND ? %s %s "), 
						sJoin.c_str(), sFilter.c_str(), sWhere.c_str() );

		sSQL += sString;

		if ( m_nMsgFilter == 1 )
			sSQL += _T("ORDER BY 4");
	}

	if ( m_nMsgFilter == 0 )
		sSQL += _T(" UNION ALL ");
	
	// include outbound messages
	if ( m_nMsgFilter == 0 || m_nMsgFilter == 2 )
	{
		if ( nContactID == -1 )
			nContactID = FindContactByEmail( GetQuery(), m_sContact.c_str() );
			
		sFilter.Format( _T("AND OutboundMessageID IN (SELECT OutboundMessageID FROM ")
				_T("OutboundMessageContacts OC INNER JOIN Contacts C on OC.ContactID = C.ContactID ")
				_T("WHERE C.IsDeleted=0 AND OC.ContactID=%d) "), nContactID );
		
		sString.Format( _T("SELECT OutboundMessageID,M.TicketID,M.Subject,EmailDateTime,EmailFrom,0,0,EmailTo ")
						_T("FROM OutboundMessages as M %s ")
						_T("WHERE M.IsDeleted = 0  AND OutboundMessageStateID>1 ")
						_T("AND EmailDateTime BETWEEN ? AND ? %s %s ORDER BY 4"), 
						sJoin.c_str(), sFilter.c_str(), sWhere.c_str() );

		sSQL += sString;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void CReportHistory::AddReportHeader()
{
	if ( !m_OutputCDF )
	{
		CEMSString sSubTitle;
		CEMSString sString;
		
		switch ( m_nType )
		{
			case 0:
				m_nFilter == EMS_REPORT_ALL ? sSubTitle = _T("All Agents") : 
														  GetAgentName( m_nFilter, sSubTitle );
				break;

			case 1:
				m_nFilter == EMS_REPORT_ALL ? sSubTitle = _T("All TicketBoxes") : 
														  GetTicketBoxName( m_nFilter, sSubTitle );
				break;

			case 3:
				m_nFilter == EMS_REPORT_ALL ? sSubTitle = _T("All Ticket Categories") : 
														  GetTicketCategoryName( m_nFilter, sSubTitle );
				break;

			default:
				sSubTitle = m_sContact;
		}
		
		sSubTitle += _T(" [");
		
		long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
		TIMESTAMP_STRUCT tsLocal;
		long tsLocalLen=0;

		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(m_StartDate,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sString );
			}
			else
			{
				GetDateTimeString( m_StartDate, sizeof(m_StartDate), sString, true, false );
			}
		}
		else
		{
			GetDateTimeString( m_StartDate, sizeof(m_StartDate), sString, true, false );
		}
		
		sSubTitle += sString.c_str();
		sSubTitle += _T(" - ");
		
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(m_EndDate,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sString );
			}
			else
			{
				GetDateTimeString( m_EndDate, sizeof(m_EndDate), sString, true, false );
			}
		}
		else
		{
			GetDateTimeString( m_EndDate, sizeof(m_EndDate), sString, true, false );
		}
		
		sSubTitle += sString.c_str();
		sSubTitle += _T("]");
		
		sSubTitle.EscapeHTML();
		GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
	}

	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add Form Field Options for the report	              
\*--------------------------------------------------------------------------*/
void CReportHistory::AddReportOptions(void)
{
	// set the title
	switch ( m_nType )
	{
		case 0:
			m_sTitle = _T("Agent History");
			break;
		case 1:
			m_sTitle = _T("TicketBox History");
			break;
		case 3:
			m_sTitle = _T("Ticket Category History");
			break;
		default:
			m_sTitle = _T("Contact History");
	}

	if ( m_bPost )
	{
		switch ( m_nMsgFilter )
		{
			case 0:
				m_sTitle += _T(" - All Messages");
				break;
			case 1:
				m_sTitle += _T(" - Inbound Messages");
				break;
			default:
				m_sTitle += _T(" - Outbound Messages");
		}
	}

	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/History_Reports.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().AddChildAttrib( _T("FULL_LABELS"), 1 );
	GetXMLGen().IntoElem();
	
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();

			if ( m_nType == 0 )
			{
				AddAgents( m_nFilter );
			}
			else if ( m_nType == 1 )
			{
				AddTicketBoxes( m_nFilter );
			}
			else if ( m_nType == 3 )
			{
				if ( m_bPost )
				{
					AddTicketCategories( m_nFilter );
				}
				else
				{
					AddTicketCategories( -1 );
				}
			}
			else
			{
				GetXMLGen().AddChildElem( _T("FORM_FIELD") );
				GetXMLGen().AddChildAttrib( _T("TYPE"), _T("INPUT") );
				GetXMLGen().AddChildAttrib( _T("NAME"), _T("CONTACT") );
				GetXMLGen().AddChildAttrib( _T("SIZE"), _T("30") );
				GetXMLGen().AddChildAttrib( _T("MAXLENGTH"), _T("255") );

				GetXMLGen().IntoElem();
					GetXMLGen().AddChildElem( _T("LABLE"), _T("Contact Email:") );
					GetXMLGen().AddChildElem( _T("VALUE"), m_sContact.c_str() );
				GetXMLGen().OutOfElem();

				GetXMLGen().AddChildElem( _T("FORM_FIELD") );
				GetXMLGen().AddChildAttrib( _T("TYPE"), _T("BUTTON") );
				GetXMLGen().AddChildAttrib( _T("NAME"), _T("SEARCH") );
			
				GetXMLGen().IntoElem();
					GetXMLGen().AddChildElem( _T("LABLE"), _T("Search") );
					GetXMLGen().AddChildElem( _T("EVENT"), _T("ContactSearch()")  );
					GetXMLGen().AddChildAttrib( _T("TYPE"), _T("onclick") );
				GetXMLGen().OutOfElem();
			}

			AddDateRange( m_nDateRange );
			GetXMLGen().AddChildAttrib( _T("ROWSPAN"), 4 );

		GetXMLGen().OutOfElem();
		
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddMsgTypes( m_nMsgFilter );
		GetXMLGen().OutOfElem();
		
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddMaxLines(m_MaxRows);
		GetXMLGen().OutOfElem();
		
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().AddChildAttrib( _T("HEIGHT"), _T("100%") );
		GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("NONE") );
		GetXMLGen().OutOfElem();
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the SQL query string that is needed to only
||				include the tickets that the logged in user has access to.
\*--------------------------------------------------------------------------*/
void CReportHistory::GetSecurityQueryStrings( CEMSString& sJoin, CEMSString& sWhere )
{
	// if the agent is an admin, then they have
	// access to everything
	if ( GetIsAdmin() )
		return;

	int nDefaultAccessLevel = GetAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, 0 );
	
	sJoin = _T("INNER JOIN Tickets as T ON M.TicketID=T.TicketID ")
		    _T("INNER JOIN TicketBoxes as TB ON T.TicketBoxID=TB.TicketBoxID ")
    		_T("INNER JOIN Objects ON TB.ObjectID=Objects.ObjectID ");
	
	// Modified on 7/11/2007 via Mike C. on the phone (Mark Mohr).  This
	// Fixed an issue when viewing history report, Mike C. has more information about this.
	if( nDefaultAccessLevel >= EMS_READ_ACCESS )
	{
		sWhere.Format( _T("AND (UseDefaultRights=1 OR (UseDefaultRights=0 AND ")
			           _T("EXISTS (SELECT TOP 1 AccessControlID FROM AccessControl ")
			           _T("WHERE ObjectID = TB.ObjectID AND AccessLevel >=2 AND (AgentID=%d OR GroupID IN ")
			           _T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=%d)))))"),
			           GetAgentID(), GetAgentID() );
	}
	else
	{
		sWhere.Format( _T("AND (UseDefaultRights=0 AND EXISTS (SELECT TOP 1 AccessControlID FROM AccessControl ")
			           _T("WHERE ObjectID = TB.ObjectID AND AccessLevel >=2 AND (AgentID=%d OR GroupID IN ")
			           _T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=%d))))"),
			            GetAgentID(), GetAgentID() );
	}
}