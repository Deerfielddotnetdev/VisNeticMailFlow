/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportResolveTime.cpp,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Average Time to resolve ticket report   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ReportResolveTime.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CReportResolveTime::CReportResolveTime(CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_bTicketBox = false;
	m_bTicketCategory = false;
	m_nFilter = EMS_REPORT_ALL;
	m_include_auto = FALSE;
	m_ReportInterval = 1;
	m_nDateRange = 0;
	m_sTitle = _T("Average Time to Resolve Ticket (Hours)");

	ZeroMemory( &m_StartDate, sizeof(m_StartDate) );
	ZeroMemory( &m_EndDate, sizeof(m_EndDate) );
	m_EndDate.hour = 23;
	m_EndDate.minute = 59;
	m_EndDate.second = 59;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point
\*--------------------------------------------------------------------------*/
int CReportResolveTime::Run( CURLAction& action )
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
			AddReportHeader();
			Query();
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

void CReportResolveTime::DecodeForm( void )
{
	//DEBUGPRINT(_T("* EMSIsapi.CReportResloveTime.DecodeForm"));

	tstring sType;
	GetISAPIData().GetURLString( _T("type"), sType, true );
	m_bTicketBox = sType.compare( _T("ticketbox") ) == 0;
	m_bTicketCategory = sType.compare( _T("ticketcategory") ) == 0;

	if ( GetISAPIData().GetURLLong( _T("TICKETBOX"), m_nFilter, true ) ||
		GetISAPIData().GetURLLong( _T("AGENT"), m_nFilter, true ) ||
		GetISAPIData().GetURLLong( _T("TICKET_CATEGORY"), m_nFilter, true ) )
	{
		ISAPIPARAM param;

		m_bPost = true;
		m_include_auto = GetISAPIData().FindURLParam( _T("INCLUDE_AUTO"), param );

		GetISAPIData().GetURLLong( _T("SUMMARY_PERIOD_SEL"), m_ReportInterval );
		GetISAPIData().GetURLLong( _T("DATE_RANGE_SEL"), m_nDateRange, true );
		GetISAPIData().GetURLLong( _T("MAX_LINES"), m_MaxRows );

		CEMSString sDate;
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );

		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		//DEBUGPRINT(_T("EMSIsapi.CReportResolveTime.DecodeForm - sDate = [ %s ]"),sDate.c_str());
		sDate.CDLGetTimeStamp( m_EndDate );
		//DEBUGPRINT(_T("EMSIsapi.CReportResolveTime.DecodeForm - sEndDate = [ %d/%d/%d ]"),m_EndDate.month,m_EndDate.day,m_EndDate.year);
	}
}

void CReportResolveTime::Query( void )
{
	CEMSString sQuery;
	CEMSString sDatePart;
	CEMSString sFilter;
	int nMins;

	if ( m_bTicketBox && m_nFilter > 0 )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, m_nFilter, EMS_READ_ACCESS );
		sFilter.Format( _T("AND T.TicketBoxID=%d"), m_nFilter );
	}
	else if ( m_bTicketCategory && m_nFilter > 0 )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_CATEGORY, m_nFilter, EMS_READ_ACCESS );
		sFilter.Format( _T("AND T.TicketCategoryID=%d"), m_nFilter );
	}
	else if ( m_nFilter > 0 )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_nFilter, EMS_READ_ACCESS );
		sFilter.Format( _T("AND T.OwnerID=%d"), m_nFilter );
	}
	else
	{
		RequireAdmin();
	}

	GetQuery().Initialize();
	GetWhenSelect( sDatePart, _T("T.DateCreated") ); 
	BINDCOL_LONG_NOLEN( GetQuery(), nMins );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );

	sQuery.Format( _T("SET DATEFIRST 7 SELECT %s, AVG(OpenMins) FROM Tickets as T ")
		_T("WHERE T.IsDeleted=0 AND TicketStateID=1 AND T.DateCreated BETWEEN ? AND ? %s AND EXISTS ")
		_T("(SELECT InboundMessageID FROM InboundMessages as I ")
		_T("WHERE IsDeleted=0 AND I.TicketID = T.TicketID)") 
		_T("GROUP BY %s ORDER BY %s"),
		sDatePart.c_str(), sFilter.c_str(), sDatePart.c_str(), sDatePart.c_str() );	

	GetQuery().Execute( sQuery.c_str() );

	// define columns
	if (! m_OutputCDF )
	{
		GetXMLGen().AddChildElem( _T("Column"), _T("Date") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Time to Resolve (Hours)") );
	}
	else
	{
		OutputCDF( tstring("Date, Time to Resolve (Hours)\r\n") );
	}

	// fetch the data and output the results
	CEMSString sDate, sDateEsc, sTime;
	CEMSString sLine;
	int nRows = 0;
	for ( nRows=0; GetQuery().Fetch() == S_OK; nRows++ )
	{	
		if ( nRows == m_MaxRows )
		{
			if( m_OutputCDF )
			{
				OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
			}
			else
			{
				GetXMLGen().AddChildElem( _T("Information"), _T("Maximum line limit reached") );
			}
			break;
		}

		GetWhen( sDate );
		sDateEsc.assign( sDate );
		sDateEsc.EscapeJavascript();
		sTime.Format( _T("%.2f"), (float) nMins / (float) 60 );

		if (! m_OutputCDF )
		{
			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );

			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
			GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
			GetXMLGen().AddChildElem( _T("Cell"), sTime.c_str() );
			GetXMLGen().OutOfElem();
		}
		else
		{
			// add line to CSV file
			sLine.Format( "\"%s\",\"%s\"\r\n", sDate.c_str(), sTime.c_str() );
			OutputCDF( sLine );
		}
	}

	if ( m_OutputCDF && nRows == 0 )
	{
		sLine.Format( _T("No results returned\r\n") );
		OutputCDF( sLine );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void CReportResolveTime::AddReportHeader(void)
{
	if ( !m_OutputCDF )
	{
		CEMSString sSubTitle;
		CEMSString sString;

		// build the sub title
		if( m_bTicketBox )
		{
			sSubTitle.assign( _T("Unknown TicketBox") );
		}
		else if( m_bTicketCategory )
		{
			sSubTitle.assign( _T("Unknown Ticket Category") );
		}
		else
		{
			sSubTitle.assign( _T("Unknown Agent") );
		}

		if ( m_nFilter == EMS_REPORT_ALL )
		{
			if( m_bTicketBox )
			{
				sSubTitle.assign( _T("All TicketBoxes") );
			}
			else if( m_bTicketCategory )
			{
				sSubTitle.assign( _T("All Ticket Categories") );
			}
			else
			{
				sSubTitle.assign( _T("All Agents") );
			}
		}
		else
		{
			if ( m_bTicketBox )
			{
				GetTicketBoxName( m_nFilter, sSubTitle );
				sSubTitle.append( _T(" TicketBox - ") );
			}
			else if( m_bTicketCategory )
			{
				GetTicketCategoryName( m_nFilter, sSubTitle );
				sSubTitle.append( _T(" Ticket Category - ") );
			}
			else
			{
				GetAgentName( m_nFilter, sSubTitle );
				sSubTitle.append( _T(" Agent - ") );
			}
		}

		sSubTitle += sString;

		sSubTitle += _T(" [");
		GetDateTimeString( m_StartDate, sizeof(m_StartDate), sString, true, false );
		sSubTitle += sString.c_str();
		sSubTitle += _T(" - ");
		GetDateTimeString( m_EndDate, sizeof(m_EndDate), sString, true, false );
		sSubTitle += sString.c_str();
		sSubTitle += _T("]");

		// add report paramaters
		sSubTitle.EscapeHTML();
		GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
	}

	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add Form Field Options for the report	              
\*--------------------------------------------------------------------------*/
void CReportResolveTime::AddReportOptions(void)
{
	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Avg_Time_to_Resolve.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().IntoElem();

		// filter drop-down and date range controls
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			if( m_bTicketBox )
			{
				AddTicketBoxes(m_nFilter);
			}
			else if( m_bTicketCategory )
			{
				AddTicketCategories(m_nFilter);
			}
			else
			{
				AddAgents(m_nFilter);
			}
			AddDateRange( m_nDateRange );
			GetXMLGen().AddChildAttrib( _T("ROWSPAN"), 4 );
		GetXMLGen().OutOfElem();
		
		// summary period drop-down
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddDateIntervals( _T("Summary Period:"), m_ReportInterval );
		GetXMLGen().OutOfElem();

		// max lines
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddMaxLines( m_MaxRows );
		GetXMLGen().OutOfElem();

		// include auto responses
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();

			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("COLSPAN"), 2 );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
			GetXMLGen().AddChildAttrib( _T("NAME"), _T("include_auto") );
			GetXMLGen().AddChildAttrib( _T("CHECKED"), m_include_auto );

			GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("LABLE"), _T("Include Auto Responses") );
			GetXMLGen().OutOfElem();

		GetXMLGen().OutOfElem();

	GetXMLGen().OutOfElem();
}