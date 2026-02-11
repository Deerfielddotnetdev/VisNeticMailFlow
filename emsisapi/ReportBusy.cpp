/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportBusy.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ReportBusy.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CReportBusy::CReportBusy( CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_bHour = false;
	m_nTotal = 0;
	m_nFilter = EMS_REPORT_ALL;
	m_nDateRange = 0;

	ZeroMemory( &m_StartDate, sizeof(m_StartDate) );
	ZeroMemory( &m_EndDate, sizeof(m_EndDate) );
	m_EndDate.hour = 23;
	m_EndDate.minute = 59;
	m_EndDate.second = 59;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point
\*--------------------------------------------------------------------------*/
int CReportBusy::Run( CURLAction& action )

{
	try
	{
		tstring sAction = "";
		
		if( GetISAPIData().GetURLString( _T("Action"), sAction, true ) )
		{
			if( sAction.compare( _T("download") ) == 0 )
			{
				OpenCDF(action);
			}
		}
		
		DecodeForm();

		if ( !m_OutputCDF )
			AddReportOptions();

		if ( m_bPost )
		{
			AddReportHeader();
			Query();
			GenerateXML();
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
void CReportBusy::DecodeForm()
{
	tstring sType;
	GetISAPIData().GetURLString( _T("type"), sType, true );
	m_bHour = sType.compare( _T("hour") ) == 0;
		
	if ( GetISAPIData().GetURLLong( _T("TICKETBOX"), m_nFilter, true ) )
	{
		CEMSString sDate;
		m_bPost = true;
		
		GetISAPIData().GetURLLong( _T("DATE_RANGE_SEL"), m_nDateRange, true );
		
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );
		
		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EndDate );
	}

	m_sTitle = m_bHour ? _T("Busiest Hour of the Day") : _T("Busiest Day of the Week");
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query the database	              
\*--------------------------------------------------------------------------*/
void CReportBusy::Query()
{
	CEMSString sQuery;
	CEMSString sFilter;
	CIDCount IDCount;
	
	LPCTSTR szDatePart = m_bHour ? _T("hour") : _T("dw");

	if ( m_nFilter > 0 )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, m_nFilter, EMS_READ_ACCESS );
		sFilter.Format( _T("AND I.OriginalTicketBoxID=%d"), m_nFilter );
	}
	else
	{
		RequireAdmin();
	}
	
	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nID );
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nCount );
	
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );

	sQuery.Format(	_T("SELECT DATEPART(%s,I.DateReceived), COUNT(*) ")
					_T("FROM InboundMessages as I WHERE DateReceived BETWEEN ? AND ? %s ")
					_T("GROUP BY DATEPART(%s,I.DateReceived) ORDER BY DATEPART(%s,I.DateReceived)"), 
					szDatePart, sFilter.c_str(), szDatePart, szDatePart );

	GetQuery().Execute( sQuery.c_str() );

	while ( GetQuery().Fetch() == S_OK )
	{
		m_nTotal += IDCount.m_nCount;
		m_list.push_back(IDCount);
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Generate the report results
\*--------------------------------------------------------------------------*/
void CReportBusy::GenerateXML()
{
	list<CIDCount>::iterator iter;
	int i=0;
	
	for ( iter = m_list.begin(); iter != m_list.end(); iter++ )
	{
		CEMSString sName;
		CEMSString sCount, sPercent;
		
		m_bHour ? GetHourString( iter->m_nID, sName) : GetWeekDayName( sName, iter->m_nID );
	
		sCount.Format( _T("%d"), iter->m_nCount );
		sPercent.Format( _T("%.2f"), m_nTotal ? ((float) iter->m_nCount / (float) m_nTotal) * 100 : 0);
		i++;
		if ( !m_OutputCDF )
		{
			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(i++) );
			
			GetXMLGen().IntoElem();

			GetXMLGen().AddChildElem( _T("Cell"), sName.c_str() );
			GetXMLGen().AddChildElem( _T("NameEsc"), sName.c_str() );
			GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
			GetXMLGen().AddChildElem( _T("Cell"), sPercent.c_str() );
			GetXMLGen().OutOfElem();
		}
		else
		{
			sName.EscapeCSV();
			
			CEMSString sString;
			sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), sName.c_str(), sCount.c_str(), sPercent.c_str() );
			OutputCDF( sString );
		}
	}
	
	if ( m_OutputCDF && i == 0 )
	{
		CEMSString sLine;
		sLine.Format( _T("No results returned\r\n") );
		OutputCDF( sLine );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header	              
\*--------------------------------------------------------------------------*/
void CReportBusy::AddReportHeader(void)
{
	CEMSString sString;

	if ( !m_OutputCDF )
	{
		CEMSString sSubTitle;
		
		// build the sub title
		sSubTitle = _T("Unknown TicketBox");

		if ( m_nFilter == EMS_REPORT_ALL )
		{
			sSubTitle = _T("All TicketBoxes");
		}
		else
		{
			sSubTitle = _T("TicketBox - ");
			GetTicketBoxName( m_nFilter, sString );
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
		GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
		
		// define columns
		GetXMLGen().AddChildElem( _T("Column"), m_bHour ? _T("Hour") : _T("Day") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Messages Received") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
	}
	else
	{
		sString.Format( _T("%s\r\n"), m_sTitle.c_str() );
		OutputCDF( sString );
		
		sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), 
			m_bHour ? _T("Hour") : _T("Day") , _T("Messages Received"), _T("Percentage") );
		OutputCDF( sString );
	}

	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add options (fields) to the report	              
\*--------------------------------------------------------------------------*/
void CReportBusy::AddReportOptions(void)
{
	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Busiest_Time.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().AddChildAttrib( _T("FULL_LABELS"), 1 );
	GetXMLGen().IntoElem();

		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddTicketBoxes( m_nFilter );
			AddDateRange( m_nDateRange );
			GetXMLGen().AddChildAttrib( _T("ROWSPAN"), 2 );
		GetXMLGen().OutOfElem();
		
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().AddChildAttrib( _T("HEIGHT"), _T("65") );
		GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("NONE") );
		GetXMLGen().OutOfElem();
	
	GetXMLGen().OutOfElem();
}