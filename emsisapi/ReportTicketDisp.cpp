/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportTicketDisp.cpp,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ReportTicketDisp.h"
#include <time.h>

/*---------------------------------------------------------------------------\            
||  Comments:	Construction		              
\*--------------------------------------------------------------------------*/
CReportTicketDisp::CReportTicketDisp(CISAPIData& ISAPIData) : CReports( ISAPIData )
{
	m_bGenerateReport = false;
	m_bTicketBox = false;
	m_nFilter = EMS_REPORT_ALL;
	m_ReportInterval = 1;
	m_nDateRange = 0;
	m_include_zero = FALSE;
	m_sTitle = _T("Ticket Activity by TicketBox");

	ZeroMemory( &m_StartDate, sizeof(m_StartDate) );
	ZeroMemory( &m_EndDate, sizeof(m_EndDate) );
	m_EndDate.hour = 23;
	m_EndDate.minute = 59;
	m_EndDate.second = 59;

	m_nCreated = 0;
	m_nMovedTo = 0;
	m_nOpened = 0;
	m_nRestored = 0;
	m_nTotalIn = 0;
	m_nClosed = 0;
	m_nDeleted = 0;
	m_nMovedFrom = 0;
	m_nTotalOut = 0;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CReportTicketDisp::Run( CURLAction& action )
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

		if ( m_bGenerateReport )
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

/*---------------------------------------------------------------------------\            
||  Comments:	Decode form paramaters		              
\*--------------------------------------------------------------------------*/
void CReportTicketDisp::DecodeForm(void)
{	
	tstring sType;
	GetISAPIData().GetURLString( _T("type"), sType, true );
	m_bTicketBox = sType.compare( _T("ticketbox") ) == 0;
	
	if ( GetISAPIData().GetURLLong( _T("TICKETBOX"), m_nFilter, true ) )
	{
		ISAPIPARAM param;
		m_include_zero = GetISAPIData().FindURLParam( _T("include_zero"), param );
		m_bGenerateReport = true;
		
		//GetISAPIData().GetURLLong( _T("SUMMARY_PERIOD_SEL"), m_ReportInterval );
		GetISAPIData().GetURLLong( _T("DATE_RANGE_SEL"), m_nDateRange, true );
		//GetISAPIData().GetURLLong( _T("MAX_LINES"), m_MaxRows );

		CEMSString sDate;
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );

		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EndDate );
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query the database
\*--------------------------------------------------------------------------*/
void CReportTicketDisp::Query(void)
{
	CEMSString sTicketBoxName;
	CEMSString sCreated;
	CEMSString sMovedTo;
	CEMSString sOpened;
	CEMSString sRestored;
	CEMSString sTotalIn;
	CEMSString sClosed;
	CEMSString sDeleted;
	CEMSString sMovedFrom;
	CEMSString sTotalOut;
	CEMSString sTotal;
	CEMSString sCreatedPercent;
	CEMSString sMovedToPercent;
	CEMSString sOpenedPercent;
	CEMSString sRestoredPercent;
	CEMSString sClosedPercent;
	CEMSString sDeletedPercent;
	CEMSString sMovedFromPercent;
	CEMSString sLine;
	int i = 0;
	list<unsigned int>::iterator iter;

	if ( m_nFilter > 0 )
	{ 
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, m_nFilter, EMS_READ_ACCESS );
		m_tbids.push_back(m_nFilter);
	}
	else
	{
		RequireAdmin();
		for( iter = GetXMLCache().m_TicketBoxIDs.GetList().begin(); 
			iter != GetXMLCache().m_TicketBoxIDs.GetList().end(); iter++ )
		{
			m_tbids.push_back(*iter);
		}		
	}

	for( tbIter = m_tbids.begin(); tbIter != m_tbids.end(); tbIter++ )
	{
		GetTicketBoxName( *tbIter, sTicketBoxName );

		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), m_nCreated );
		BINDCOL_LONG_NOLEN( GetQuery(), m_nMovedTo );
		BINDCOL_LONG_NOLEN( GetQuery(), m_nOpened );
		BINDCOL_LONG_NOLEN( GetQuery(), m_nRestored );
		BINDCOL_LONG_NOLEN( GetQuery(), m_nClosed );
		BINDCOL_LONG_NOLEN( GetQuery(), m_nDeleted );
		BINDCOL_LONG_NOLEN( GetQuery(), m_nMovedFrom );
		BINDPARAM_LONG( GetQuery(), *tbIter );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
		BINDPARAM_LONG( GetQuery(), *tbIter );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
		BINDPARAM_LONG( GetQuery(), *tbIter );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
		BINDPARAM_LONG( GetQuery(), *tbIter );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
		BINDPARAM_LONG( GetQuery(), *tbIter );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
		BINDPARAM_LONG( GetQuery(), *tbIter );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
		BINDPARAM_LONG( GetQuery(), *tbIter );
		GetQuery().Execute( _T("SELECT (SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=1), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=2 AND ID1=1), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=2 AND ID1=3 AND ID2=1 AND (TicketStateID=2 OR TicketStateID=3 OR TicketStateID=6)), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=13 AND (TicketStateID=2 OR TicketStateID=3 OR TicketStateID=6)), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=2 AND ID1=3 AND TicketStateID=1), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=7), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE DateTime BETWEEN ? AND ? AND TicketActionID=2 AND ID1=1 AND ID2=?) FROM TicketHistory"));
		// fetch the data and output the results
		if ( GetQuery().Fetch() != S_OK )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		}

		m_nTotalIn = long(m_nCreated) + long(m_nMovedTo) + long(m_nOpened) + long(m_nRestored);
		m_nTotalOut = long(m_nClosed) + long(m_nDeleted) + long(m_nMovedFrom);
		m_nTotal = long(m_nTotalIn) + long(m_nTotalOut);

		sCreatedPercent.Format( _T("%.2f"), m_nTotalIn ? ((float) m_nCreated / (float) m_nTotalIn) * 100 : 0);
		sMovedToPercent.Format( _T("%.2f"), m_nTotalIn ? ((float) m_nMovedTo / (float) m_nTotalIn) * 100 : 0);
		sOpenedPercent.Format( _T("%.2f"), m_nTotalIn ? ((float) m_nOpened / (float) m_nTotalIn) * 100 : 0);
		sRestoredPercent.Format( _T("%.2f"), m_nTotalIn ? ((float) m_nRestored / (float) m_nTotalIn) * 100 : 0);
		sClosedPercent.Format( _T("%.2f"), m_nTotalOut ? ((float) m_nClosed / (float) m_nTotalOut) * 100 : 0);
		sDeletedPercent.Format( _T("%.2f"), m_nTotalOut ? ((float) m_nDeleted / (float) m_nTotalOut) * 100 : 0);
		sMovedFromPercent.Format( _T("%.2f"), m_nTotalOut ? ((float) m_nMovedFrom / (float) m_nTotalOut) * 100 : 0);

		sCreated.Format( _T("%d"), m_nCreated );
		sMovedTo.Format( _T("%d"), m_nMovedTo );
		sOpened.Format( _T("%d"), m_nOpened );
		sRestored.Format( _T("%d"), m_nRestored );
		sTotalIn.Format( _T("%d"), m_nTotalIn );
		sClosed.Format( _T("%d"), m_nClosed );
		sDeleted.Format( _T("%d"), m_nDeleted );
		sMovedFrom.Format( _T("%d"), m_nMovedFrom );
		sTotalOut.Format( _T("%d"), m_nTotalOut );
		sTotal.Format( _T("%d"), m_nTotal );
		
		if ( m_include_zero || m_nTotal > 0 )
		{
			if (! m_OutputCDF )
			{
				GetXMLGen().AddChildElem( _T("Row") );
				GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(i++) );
				
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sTicketBoxName.c_str() );
				sTicketBoxName.EscapeJavascript();
				GetXMLGen().AddChildElem( _T("NameEsc"), sTicketBoxName.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sCreated.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sMovedTo.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sOpened.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sRestored.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sTotalIn.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sClosed.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sDeleted.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sMovedFrom.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sTotalOut.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sTotal.c_str() );
				GetXMLGen().OutOfElem();
			}
			else
			{
				// add line to CSV file
				sLine.Format( "\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"\r\n", 
							sTicketBoxName.c_str(),m_nCreated,m_nMovedTo,m_nOpened,m_nRestored,m_nTotalIn,m_nClosed,m_nDeleted,m_nMovedFrom,m_nTotalOut,m_nTotal );
				OutputCDF( sLine );
			}
		}
	}	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void CReportTicketDisp::AddReportHeader(void)
{
	
		CEMSString sSubTitle;
		CEMSString sString;
		CEMSString sLine;

		// build the sub title
		if( m_bTicketBox )
		{
			sSubTitle.assign( _T("Unknown TicketBox") );
		}
		
		if ( m_nFilter == EMS_REPORT_ALL )
		{
			if( m_bTicketBox )
			{
				sSubTitle.assign( _T("All TicketBoxes") );
			}			
		}
		else
		{
			if ( m_bTicketBox )
			{
				GetTicketBoxName( m_nFilter, sSubTitle );
				sSubTitle.append( _T(" - TicketBox") );
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

		sSubTitle.EscapeHTML();

	if ( !m_OutputCDF )
	{
		GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );

		// define columns
		GetXMLGen().AddChildElem( _T("Column"),_T("TicketBox") );
		GetXMLGen().AddChildElem( _T("Column"), _T("New") );
		//GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Moved To") );
		//GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Opened") );
		//GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Restored") );
		//GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Total In") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Closed") );
		//GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Deleted") );
		//GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Moved From") );
		//GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Total Out") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Total") );		
	}
	else
	{
		sLine.Format( "\"Ticket Activity by TicketBox\"\r\n" );
		OutputCDF( sLine );
		sLine.Format( "\"%s\"\r\n",sSubTitle.c_str() );
		OutputCDF( sLine );
		sLine.Format( "\r\n" );
		OutputCDF( sLine );
		sLine.Format( "\"TicketBox\",\"New\",\"Moved To\",\"Opened\",\"Restored\",\"Total In\",\"Closed\",\"Deleted\",\"Moved From\",\"Total Out\",\"Total\"\r\n" );
		OutputCDF( sLine );
	}

	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add Form Field Options for the report	              
\*--------------------------------------------------------------------------*/
void CReportTicketDisp::AddReportOptions(void)
{
	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Ticket_Activity.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().IntoElem();
	
	// filter drop-down and date range controls
	GetXMLGen().AddChildElem( _T("ROW") );
	GetXMLGen().IntoElem();
	AddTicketBoxes(m_nFilter);
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("include_zero") );
	GetXMLGen().AddChildAttrib( _T("CHECKED"), m_include_zero );
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("LABLE"), _T("Include TicketBoxes with 0 Tickets") );
	GetXMLGen().OutOfElem();
	GetXMLGen().AddChildAttrib( _T("ROWSPAN"), 2 );
	GetXMLGen().OutOfElem();

	GetXMLGen().AddChildElem( _T("ROW") );
	GetXMLGen().IntoElem();
		AddDateRange( m_nDateRange );
		GetXMLGen().AddChildAttrib( _T("ROWSPAN"), 2 );
		GetXMLGen().AddChildAttrib( _T("COLSPAN"), 3 );
	GetXMLGen().OutOfElem();
	
	// summary period drop-down
	/*GetXMLGen().AddChildElem( _T("ROW") );
	GetXMLGen().IntoElem();
		AddDateIntervals( _T("Summary Period:"), m_ReportInterval );
	GetXMLGen().OutOfElem();*/

	// max lines
	/*GetXMLGen().AddChildElem( _T("ROW") );
	GetXMLGen().IntoElem();
		AddMaxLines( m_MaxRows );
	GetXMLGen().OutOfElem();*/		
	
	GetXMLGen().OutOfElem();
}