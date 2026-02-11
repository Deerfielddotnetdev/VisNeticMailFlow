/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ViewResults.cpp,v 1.1 2007/09/22 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for viewing Report Results
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ViewResults.h"
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CViewResults::CViewResults(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_hCDFFile = INVALID_HANDLE_VALUE;
	m_MaxRows = 250;

	m_OutputCDF = false;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CViewResults::Run( CURLAction& action )
{
	RequireAgentRightLevel( EMS_OBJECT_TYPE_SCHEDULED_REPORTS, 0, EMS_READ_ACCESS );
	
	try
	{
		tstring sAction;

		if( GetISAPIData().GetURLString( _T("Action"), sAction, true ) )
		{
			if( sAction.compare( _T("download") ) == 0 )
				OpenCDF(action);				
		}

		if ( GetISAPIData().GetXMLPost() )
		{
			DISABLE_IN_DEMO();
			Delete();
		}
		else
		{
			if( m_ISAPIData.m_sPage.compare( _T("viewresults") ) == 0)
			{
				Query();
			}
			else if ( m_ISAPIData.m_sPage.compare( _T("viewreportdetails") ) == 0)
			{
				ShowResults( action );
			}
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


void CViewResults::ShowResults( CURLAction& action )
{
	CEMSString sDate;
	CEMSString sTitle;
	
	GetISAPIData().GetURLLong( _T("ID"), trr.m_ReportResultID );
	if ( !trr.m_ReportResultID )
		GetISAPIData().GetXMLLong( _T("ID"), trr.m_ReportResultID );
	if ( trr.m_ReportResultID > 0 )
	{
		//Get report details
		trr.QueryOne( GetQuery() );
		
		tsr.m_ScheduledReportID = trr.m_ScheduledReportID;
		tsr.Query( GetQuery() );
		
		tstdr.m_StandardReportID = tsr.m_ReportID;
		tstdr.Query( GetQuery() );
		
		GetDateTimeString( trr.m_DateRan, trr.m_DateRanLen, sDate );
		sTitle.Format(  _T("%s :: %s"), tsr.m_Description, sDate.c_str() );
		action.m_sPageTitle.assign( sTitle.c_str() );

		rrr.m_ReportResultID = trr.m_ReportResultID;
		rrr.Query( GetQuery() );
		m_rrr.clear();
		while( GetQuery().Fetch() == S_OK )
		{
			m_rrr.push_back( rrr );
		}
		
		AddReportHeader();
		OutputResults();

	}
	else
	{
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Invalid Report ID!") );
	}
}

void CViewResults::OutputResults(void)
{
	int nRows = 0;
	CEMSString sDate, sDateEsc, sCount, sLine, sPercent, sCategory;
	
	switch ( tstdr.m_ReportTypeID )
	{
	case 1:
		//Add the rows
		for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
		{
			sDate.Format( _T("%s"), rrrIter->m_Col1 );
            sDateEsc.assign( sDate );
			sDateEsc.EscapeJavascript();
			sCount.Format( _T("%d"), rrrIter->m_Col3 );
			
			if (! m_OutputCDF )
			{
				GetXMLGen().AddChildElem( _T("Row") );
				GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
				GetXMLGen().OutOfElem();	
			}
			else
			{
				// add line to CSV file
				sLine.Format( "\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str() );
				OutputCDF( sLine );
			}
			nRows++;
		}	
		break;
	case 2:
		//Add the rows
		for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
		{
			sDate.Format( _T("%s"), rrrIter->m_Col1 );
            sDateEsc.assign( sDate );
			sDateEsc.EscapeJavascript();
			sCount.Format( _T("%d"), rrrIter->m_Col3 );
			
			if (! m_OutputCDF )
			{
				GetXMLGen().AddChildElem( _T("Row") );
				GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
				GetXMLGen().OutOfElem();	
			}
			else
			{
				// add line to CSV file
				sLine.Format( "\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str() );
				OutputCDF( sLine );
			}
			nRows++;
		}	
		break;
	case 3:
		//Add the rows
		for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
		{
			sDate.Format( _T("%s"), rrrIter->m_Col1 );
            sDateEsc.assign( sDate );
			sDateEsc.EscapeJavascript();
			sCount.Format( _T("%d"), rrrIter->m_Col3 );
			
			if (! m_OutputCDF )
			{
				GetXMLGen().AddChildElem( _T("Row") );
				GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
				GetXMLGen().OutOfElem();	
			}
			else
			{
				// add line to CSV file
				sLine.Format( "\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str() );
				OutputCDF( sLine );
			}
			nRows++;
		}		
		break;
	case 4:
		//Add the rows
		for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
		{
			sDate.Format( _T("%s"), rrrIter->m_Col1 );
            sDateEsc.assign( sDate );
			sDateEsc.EscapeJavascript();
			sCount.Format( _T("%d"), rrrIter->m_Col3 );
			sPercent.Format( _T("%s"), rrrIter->m_Col2 );
			
			if (! m_OutputCDF )
			{
				GetXMLGen().AddChildElem( _T("Row") );
				GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sPercent.c_str() );
				GetXMLGen().OutOfElem();	
			}
			else
			{
				sDate.EscapeCSV();

				sLine.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), sDate.c_str(), sCount.c_str(), sPercent.c_str() );
				OutputCDF( sLine );				
			}
			nRows++;
		}		
		break;
	case 5:
		//Add the rows
		if ( tstdr.m_ReportObjectID == 7 ) //Usage
		{	
			sDate.Format( _T("%s"), rrrIter->m_Col1 );
            sDateEsc.assign( sDate );
			sDateEsc.EscapeJavascript();
			sCount.Format( _T("%d"), rrrIter->m_Col3 );
			sPercent.Format( _T("%d"), rrrIter->m_Col4 );
			GetStdCategoryName( tsr.m_TargetID, sCategory );

			for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
			{
				if (! m_OutputCDF )
				{
					GetXMLGen().AddChildElem( _T("Row") );
					GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
					GetXMLGen().IntoElem();
					GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
					GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
					GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
					GetXMLGen().AddChildElem( _T("Cell"), sPercent.c_str() );
					GetXMLGen().OutOfElem();	
				}
				else
				{
					sDate.EscapeCSV();
					sCategory.EscapeCSV();
					if ( tsr.m_TargetID == EMS_REPORT_ALL )
					{
						sLine.Format( "\"%s\",\"%s\",\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str(), sPercent.c_str(), sCategory.c_str() );
					}
					else
					{
						sLine.Format( "\"%s\",\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str(), sPercent.c_str() );
					}
					OutputCDF( sLine );
				}
				nRows++;
			}
		}
		else //Category or Author
		{
			sDate.Format( _T("%s"), rrrIter->m_Col1 );
            sDateEsc.assign( sDate );
			sDateEsc.EscapeJavascript();
			sCount.Format( _T("%d"), rrrIter->m_Col3 );
			sPercent.Format( _T("%s"), rrrIter->m_Col4 );
			GetStdCategoryName( tsr.m_TargetID, sCategory );

			for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
			{
				if (! m_OutputCDF )
				{
					GetXMLGen().AddChildElem( _T("Row") );
					GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
					GetXMLGen().IntoElem();
					GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
					GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
					GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
					GetXMLGen().AddChildElem( _T("Cell"), sPercent.c_str() );
					GetXMLGen().OutOfElem();	
				}
				else
				{
					sDate.EscapeCSV();
					sLine.Format( "\"%s\",\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str(), sPercent.c_str() );
					OutputCDF( sLine );
				}
				nRows++;
			}
		}
		break;
	case 6:
		//Add the rows
		for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
		{
			sDate.Format( _T("%s"), rrrIter->m_Col1 );
            sDateEsc.assign( sDate );
			sDateEsc.EscapeJavascript();
			
			char *endPtr;
			double x = strtod( rrrIter->m_Col2, &endPtr );			
			sCount.Format( _T("%.2f"), x);

			if (! m_OutputCDF )
			{
				GetXMLGen().AddChildElem( _T("Row") );
				GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
				GetXMLGen().OutOfElem();	
			}
			else
			{
				// add line to CSV file
				sLine.Format( "\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str() );
				OutputCDF( sLine );
			}
			nRows++;
		}	
		break;
	case 7:
		//Add the rows
		for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
		{
			sDate.Format( _T("%s"), rrrIter->m_Col1 );
            sDateEsc.assign( sDate );
			sDateEsc.EscapeJavascript();
			
			char *endPtr;
			double x = strtod( rrrIter->m_Col2, &endPtr );			
			sCount.Format( _T("%.2f"), x);
			
			if (! m_OutputCDF )
			{
				GetXMLGen().AddChildElem( _T("Row") );
				GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
				GetXMLGen().OutOfElem();	
			}
			else
			{
				// add line to CSV file
				sLine.Format( "\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str() );
				OutputCDF( sLine );
			}
			nRows++;
		}	
		break;
	case 8:
		//Add the rows
		for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
		{
			sDate.Format( _T("%s"), rrrIter->m_Col1 );
            sDateEsc.assign( sDate );
			sDateEsc.EscapeJavascript();
			sCount.Format( _T("%d"), rrrIter->m_Col3 );
			sPercent.Format( _T("%s"), rrrIter->m_Col2 );
			
			if (! m_OutputCDF )
			{
				GetXMLGen().AddChildElem( _T("Row") );
				GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sPercent.c_str() );
				GetXMLGen().OutOfElem();	
			}
			else
			{
				sDate.EscapeCSV();

				sLine.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), sDate.c_str(), sCount.c_str(), sPercent.c_str() );
				OutputCDF( sLine );				
			}
			nRows++;
		}		
		break;
	case 10:
		//Add the rows
		for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
		{
			sDate.Format( _T("%s"), rrrIter->m_Col1 );
            sDateEsc.assign( sDate );
			sDateEsc.EscapeJavascript();
			sCount.Format( _T("%d"), rrrIter->m_Col3 );
			
			if (! m_OutputCDF )
			{
				GetXMLGen().AddChildElem( _T("Row") );
				GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), (_T("%d"),rrrIter->m_Col3) );
				GetXMLGen().AddChildElem( _T("Cell"), (_T("%d"),rrrIter->m_Col5) );
				GetXMLGen().AddChildElem( _T("Cell"), (_T("%d"),rrrIter->m_Col6) );
				GetXMLGen().AddChildElem( _T("Cell"), (_T("%d"),rrrIter->m_Col7) );
				GetXMLGen().AddChildElem( _T("Cell"), (_T("%d"),rrrIter->m_Col8) );
				GetXMLGen().AddChildElem( _T("Cell"), (_T("%d"),rrrIter->m_Col9) );
				GetXMLGen().AddChildElem( _T("Cell"), (_T("%d"),rrrIter->m_Col10) );
				GetXMLGen().AddChildElem( _T("Cell"), (_T("%d"),rrrIter->m_Col11) );
				GetXMLGen().AddChildElem( _T("Cell"), (_T("%d"),rrrIter->m_Col12) );
				GetXMLGen().AddChildElem( _T("Cell"), (_T("%d"),rrrIter->m_Col13) );
				GetXMLGen().OutOfElem();
			}
			else
			{
				// add line to CSV file
				sLine.Format( "\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"\r\n", 
							sDate.c_str(),rrrIter->m_Col3,rrrIter->m_Col5,rrrIter->m_Col6,rrrIter->m_Col7,rrrIter->m_Col8,rrrIter->m_Col9,rrrIter->m_Col10,rrrIter->m_Col11,rrrIter->m_Col12,rrrIter->m_Col13 );
				OutputCDF( sLine );
			}
			nRows++;
		}	
		break;	
	}
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void CViewResults::AddReportHeader(void)
{
	CEMSString sSubTitle;
	CEMSString sString;
	CEMSString sTemp;
	
	switch ( tstdr.m_ReportObjectID )
	{
	case 1:
		sSubTitle.assign( _T("Unknown TicketBox") );
		break;
	case 2:
		sSubTitle.assign( _T("Unknown Agent") );
		break;
	case 3:
		sSubTitle.assign( _T("Unknown Ticket Category") );
		break;
	case 4:
		sSubTitle.assign( _T("Unknown Routing Rule") );
		break;
	case 5:
		sSubTitle.assign( _T("Unknown Message Source") );
		break;
	}	
	
	GetXMLGen().AddChildElem( _T("RRID"), trr.m_ReportResultID );

	switch ( tstdr.m_ReportTypeID )
	{
	case 1:
		if ( !m_OutputCDF )
		{
			switch ( tstdr.m_ReportObjectID )
			{
			case 1:
				m_sTitle.assign( _T("Inbound Messages by TicketBox") );
				break;
			case 2:
				m_sTitle.assign( _T("Inbound Messages by Agent") );
				break;
			case 4:
				m_sTitle.assign( _T("Inbound Messages by Routing Rule") );
				break;
			case 5:
				m_sTitle.assign( _T("Inbound Messages by Message Source") );
				break;
			case 3:
				m_sTitle.assign( _T("Inbound Messages by Ticket Category") );
				break;
			}

			switch ( tsr.m_SumPeriod )
			{
			case 0: //Hour
				m_sSumPeriod.assign( _T("Hour") );
				break;
			case 1: //Day
				m_sSumPeriod.assign( _T("Day") );
				break;
			case 2: //Week
				m_sSumPeriod.assign( _T("Week") );
				break;
			case 3: //Month
				m_sSumPeriod.assign( _T("Month") );
				break;
			case 4: //Year
				m_sSumPeriod.assign( _T("Year") );
				break;
			}

			if ( tsr.m_TargetID == EMS_REPORT_ALL )
			{
				switch ( tstdr.m_ReportObjectID )
				{
				case 1:
					sSubTitle.assign( _T("All TicketBoxes") );
					break;
				case 2:
					sSubTitle.assign( _T("All Agents") );
					break;
				case 4:
					sSubTitle.assign( _T("All Routing Rules") );
					break;
				case 5:
					sSubTitle.assign( _T("All Message Sources") );
					break;
				case 3:
					sSubTitle.assign( _T("All Ticket Categories") );
					break;
				}
			}
			else
			{	
				switch ( tstdr.m_ReportObjectID )
				{
				case 1:
					sSubTitle.assign( _T("TicketBox - ") );
					GetTicketBoxName( tsr.m_TargetID, sString );
					break;
				case 2:
					sSubTitle.assign( _T("Agent - ") );
					GetAgentName( tsr.m_TargetID, sString );
					break;
				case 4:
					sSubTitle.assign( _T("Routing Rule - ") );
					GetRoutingRuleName( tsr.m_TargetID, sString );
					break;
				case 5:
					sSubTitle.assign( _T("Message Source - ") );
					GetMsgSourceName( tsr.m_TargetID, sString );
					break;
				case 3:
					sSubTitle.assign( _T("Ticket Category - ") );
					GetTicketCategoryName( tsr.m_TargetID, sString );
					break;
				}

				sSubTitle += sString;
			}

			sSubTitle += _T(" [");
			GetDateTimeString( trr.m_DateFrom, trr.m_DateFromLen, sString, true, false );
			sSubTitle += sString.c_str();
			sSubTitle += _T(" - ");
			GetDateTimeString( trr.m_DateTo, trr.m_DateToLen, sString, true, false );
			sSubTitle += sString.c_str();
			sSubTitle += _T("]");

			sSubTitle.EscapeHTML();
			GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
			GetXMLGen().AddChildElem( _T("SumPeriod"), m_sSumPeriod.c_str() );
			GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
			GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Inbound_Message.htm") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Date") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Number of Messages") );
			GetXMLGen().AddChildElem( _T("Results") );
		}
		else
		{
			OutputCDF( tstring("Date, Number of Messages\r\n") );
		}
		break;
	case 2:
		if ( !m_OutputCDF )
		{
			switch ( tstdr.m_ReportObjectID )
			{
			case 1:
				m_sTitle.assign( _T("Outbound Messages by TicketBox") );
				break;
			case 2:
				m_sTitle.assign( _T("Outbound Messages by Agent") );
				break;
			case 3:
				m_sTitle.assign( _T("Outbound Messages by Ticket Category") );
				break;
			}

			switch ( tsr.m_SumPeriod )
			{
			case 0: //Hour
				m_sSumPeriod.assign( _T("Hour") );
				break;
			case 1: //Day
				m_sSumPeriod.assign( _T("Day") );
				break;
			case 2: //Week
				m_sSumPeriod.assign( _T("Week") );
				break;
			case 3: //Month
				m_sSumPeriod.assign( _T("Month") );
				break;
			case 4: //Year
				m_sSumPeriod.assign( _T("Year") );
				break;
			}

			if ( tsr.m_TargetID == EMS_REPORT_ALL )
			{
				switch ( tstdr.m_ReportObjectID )
				{
				case 1:
					sSubTitle.assign( _T("All TicketBoxes") );
					break;
				case 2:
					sSubTitle.assign( _T("All Agents") );
					break;
				case 3:
					sSubTitle.assign( _T("All Ticket Categories") );
					break;				
				}
			}
			else
			{	
				switch ( tstdr.m_ReportObjectID )
				{
				case 1:
					sSubTitle.assign( _T("TicketBox - ") );
					GetTicketBoxName( tsr.m_TargetID, sString );
					break;
				case 2:
					sSubTitle.assign( _T("Agent - ") );
					GetAgentName( tsr.m_TargetID, sString );
					break;
				case 3:
					sSubTitle.assign( _T("Ticket Category - ") );
					GetTicketCategoryName( tsr.m_TargetID, sString );
					break;
				}

				sSubTitle += sString;
			}

			sSubTitle += _T(" [");
			GetDateTimeString( trr.m_DateFrom, trr.m_DateFromLen, sString, true, false );
			sSubTitle += sString.c_str();
			sSubTitle += _T(" - ");
			GetDateTimeString( trr.m_DateTo, trr.m_DateToLen, sString, true, false );
			sSubTitle += sString.c_str();
			sSubTitle += _T("]");

			sSubTitle.EscapeHTML();
			GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
			GetXMLGen().AddChildElem( _T("SumPeriod"), m_sSumPeriod.c_str() );
			GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
			GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Outbound_Message.htm") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Date") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Number of Messages") );
			GetXMLGen().AddChildElem( _T("Results") );
		}
		else
		{
			OutputCDF( tstring("Date, Number of Messages\r\n") );
		}
		break;
	case 3:
		if (! m_OutputCDF )
		{
			m_sTitle.assign( _T("Open Ticket Age") );
			
			if ( tsr.m_TargetID == EMS_REPORT_ALL )
			{
				switch ( tstdr.m_ReportObjectID )
				{
				case 1:
					sSubTitle.assign( _T("All TicketBoxes") );
					break;
				case 2:
					sSubTitle.assign( _T("All Agents") );
					break;
				case 3:
					sSubTitle.assign( _T("All Ticket Categories") );
					break;				
				}
			}
			else
			{	
				switch ( tstdr.m_ReportObjectID )
				{
				case 1:
					sSubTitle.assign( _T("TicketBox - ") );
					GetTicketBoxName( tsr.m_TargetID, sString );
					break;
				case 2:
					sSubTitle.assign( _T("Agent - ") );
					GetAgentName( tsr.m_TargetID, sString );
					break;
				case 3:
					sSubTitle.assign( _T("Ticket Category - ") );
					GetTicketCategoryName( tsr.m_TargetID, sString );
					break;
				}

				sSubTitle += sString;
			}


			// define columns
			sSubTitle.EscapeHTML();
			GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
			GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
			GetXMLGen().AddChildElem( _T("Column"), _T("Ticket Age") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Number of Tickets") );
			GetXMLGen().AddChildElem( _T("Results") );
		}
		else
		{
			OutputCDF( tstring("Ticket Age, Number of Tickets\r\n") );
		}
		break;
	case 4:
		if ( !m_OutputCDF )
		{
			switch ( tstdr.m_ReportObjectID )
			{
			case 1:
				m_sTitle.assign( _T("Ticket Distribution by TicketBox") );
				break;
			case 2:
				m_sTitle.assign( _T("Ticket Distribution by Agent") );
				break;
			case 3:
				m_sTitle.assign( _T("Ticket Distribution by Ticket Category") );
				break;
			}

			
			// build the sub-title
			CEMSString sSubTitle( _T("All") );
			GetTicketStateName( tsr.m_TicketStateID, sSubTitle );
			sSubTitle.append( _T(" Tickets") );
			sSubTitle.EscapeHTML();
			GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
			
			// define columns
			switch ( tstdr.m_ReportObjectID )
			{
			case 1:
				GetXMLGen().AddChildElem( _T("Column"), _T("TicketBox") );
				break;
			case 2:
				GetXMLGen().AddChildElem( _T("Column"), _T("Ticket Category") );
				break;
			case 3:
				GetXMLGen().AddChildElem( _T("Column"), _T("Agent") );
				break;
			}
			
			GetXMLGen().AddChildElem( _T("Column"), _T("Tickets") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
			GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Ticket_Distribution.htm") );
			GetXMLGen().AddChildElem( _T("Results") );
		}
		else
		{
			switch ( tstdr.m_ReportObjectID )
			{
			case 1:
				sTemp.assign( _T("TicketBox") );
				break;
			case 2:
				sTemp.assign( _T("Ticket Category") );
				break;
			case 3:
				sTemp.assign( _T("Agent") );
				break;
			}
					
			sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), sTemp.c_str(), _T("Tickets"), _T("Percentage") );
			OutputCDF( sString );
		}
		break;
	case 5:
		if ( tstdr.m_ReportObjectID == 7 ) //Usage
		{
			if (! m_OutputCDF )
			{
				m_sTitle.assign( _T("Standard Response Usage") );				
				GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );

				if ( tsr.m_TargetID == EMS_REPORT_ALL )
				{
					sSubTitle.assign( _T("All Categories") );
				}
				else
				{	
					GetStdCategoryName( tsr.m_TargetID, sSubTitle );					
				}

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
				GetXMLGen().AddChildElem( _T("Column"), _T("Standard Response") );
				GetXMLGen().AddChildElem( _T("Column"), _T("Times Used") );
				GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
				
				if ( tsr.m_TargetID == EMS_REPORT_ALL )
					GetXMLGen().AddChildElem( _T("Column"), _T("Category") );

				GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/std_distribution.htm") );
				GetXMLGen().AddChildElem( _T("Results") );
			}
			else
			{
				if ( tsr.m_TargetID == EMS_REPORT_ALL )
				{
					sString.Format( _T("\"%s\",\"%s\",\"%s\",\"%s\"\r\n"), _T("Standard Response"), _T("Times Used"), _T("Percentage"), _T("Category") );
				}
				else
				{
					sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), _T("Standard Response"), _T("Times Used"), _T("Percentage") );
				}

				OutputCDF( sString );
			}
		}
		else //Category or Author
		{
			if (! m_OutputCDF )
			{
				if ( tstdr.m_ReportObjectID == 8 )
				{
					m_sTitle.assign( _T("Standard Response Distribution by Category") );
				}
				else
				{
					m_sTitle.assign( _T("Standard Response Distribution by Author") );
				}
											
				GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );

				// define columns
				if ( tstdr.m_ReportObjectID == 8 )
				{
					GetXMLGen().AddChildElem( _T("Column"), _T("Category") );
				}
				else
				{
					GetXMLGen().AddChildElem( _T("Column"), _T("Author") );
				}
				
				GetXMLGen().AddChildElem( _T("Column"), _T("Responses") );
				GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
				GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/std_distribution.htm") );
				GetXMLGen().AddChildElem( _T("Results") );
			}
			else
			{
				if ( tstdr.m_ReportObjectID == 8 )
				{
					sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), _T("Category"), _T("Responses"), _T("Percentage") );
				}
				else
				{
					sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), _T("Author"), _T("Responses"), _T("Percentage") );
				}

				OutputCDF( sString );				
			}
		}
		break;
	case 6:
		if ( !m_OutputCDF )
		{
			m_sTitle = _T("Average Response Time (Hours)");
			
			if ( tsr.m_TargetID == EMS_REPORT_ALL )
			{
				switch ( tstdr.m_ReportObjectID )
				{
				case 1:
					sSubTitle.assign( _T("All TicketBoxes") );
					break;
				case 2:
					sSubTitle.assign( _T("All Agents") );
					break;
				case 3:
					sSubTitle.assign( _T("All Ticket Categories") );
					break;				
				}
			}
			else
			{	
				switch ( tstdr.m_ReportObjectID )
				{
				case 1:
					sSubTitle.assign( _T("TicketBox - ") );
					GetTicketBoxName( tsr.m_TargetID, sString );
					break;
				case 2:
					sSubTitle.assign( _T("Agent - ") );
					GetAgentName( tsr.m_TargetID, sString );
					break;
				case 3:
					sSubTitle.assign( _T("Ticket Category - ") );
					GetTicketCategoryName( tsr.m_TargetID, sString );
					break;
				}

				sSubTitle += sString;
			}

			sSubTitle += _T(" [");
			GetDateTimeString( trr.m_DateFrom, trr.m_DateFromLen, sString, true, false );
			sSubTitle += sString.c_str();
			sSubTitle += _T(" - ");
			GetDateTimeString( trr.m_DateTo, trr.m_DateToLen, sString, true, false );
			sSubTitle += sString.c_str();
			sSubTitle += _T("]");

			sSubTitle.EscapeHTML();
			GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
			GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
			GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Avg_Response_Time.htm") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Date") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Response Time (Hours)") );
			GetXMLGen().AddChildElem( _T("Results") );			
		}
		else
		{
			OutputCDF( tstring("Date, Response Time (Hours)\r\n") );
		}
		break;
	case 7:
		if ( !m_OutputCDF )
		{
			m_sTitle = _T("Average Time to Resolve Ticket (Hours)");
			
			if ( tsr.m_TargetID == EMS_REPORT_ALL )
			{
				switch ( tstdr.m_ReportObjectID )
				{
				case 1:
					sSubTitle.assign( _T("All TicketBoxes") );
					break;
				case 2:
					sSubTitle.assign( _T("All Agents") );
					break;
				case 3:
					sSubTitle.assign( _T("All Ticket Categories") );
					break;				
				}
			}
			else
			{	
				switch ( tstdr.m_ReportObjectID )
				{
				case 1:
					sSubTitle.assign( _T("TicketBox - ") );
					GetTicketBoxName( tsr.m_TargetID, sString );
					break;
				case 2:
					sSubTitle.assign( _T("Agent - ") );
					GetAgentName( tsr.m_TargetID, sString );
					break;
				case 3:
					sSubTitle.assign( _T("Ticket Category - ") );
					GetTicketCategoryName( tsr.m_TargetID, sString );
					break;
				}

				sSubTitle += sString;
			}

			sSubTitle += _T(" [");
			GetDateTimeString( trr.m_DateFrom, trr.m_DateFromLen, sString, true, false );
			sSubTitle += sString.c_str();
			sSubTitle += _T(" - ");
			GetDateTimeString( trr.m_DateTo, trr.m_DateToLen, sString, true, false );
			sSubTitle += sString.c_str();
			sSubTitle += _T("]");

			sSubTitle.EscapeHTML();
			GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
			GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
			GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Avg_Time_to_Resolve.htm") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Date") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Time to Resolve (Hours)") );
			GetXMLGen().AddChildElem( _T("Results") );
		}
		else
		{
			OutputCDF( tstring("Date, Time to Resolve (Hours)\r\n") );
		}
		break;
	case 8:
		if ( !m_OutputCDF )
		{
			if ( tstdr.m_ReportObjectID == 10 )
			{
				m_sTitle = _T("Busiest Hour of the Day");
			}
			else
			{
				m_sTitle = _T("Busiest Day of the Week");
			}

			sSubTitle = _T("Unknown TicketBox");

			if ( tsr.m_TargetID == EMS_REPORT_ALL )
			{
				sSubTitle = _T("All TicketBoxes");
			}
			else
			{
				sSubTitle = _T("TicketBox - ");
				GetTicketBoxName( tsr.m_TargetID, sString );
			}

			sSubTitle += sString;

			sSubTitle += _T(" [");
			GetDateTimeString( trr.m_DateFrom, trr.m_DateFromLen, sString, true, false );
			sSubTitle += sString.c_str();
			sSubTitle += _T(" - ");
			GetDateTimeString( trr.m_DateTo, trr.m_DateToLen, sString, true, false );
			sSubTitle += sString.c_str();
			sSubTitle += _T("]");

			sSubTitle.EscapeHTML();
			GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
			GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
			GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Busiest_Time.htm") );
			if ( tstdr.m_ReportObjectID == 10 )
			{
				GetXMLGen().AddChildElem( _T("Column"), _T("Hour") );
			}
			else
			{
				GetXMLGen().AddChildElem( _T("Column"), _T("Day") );
			}
			GetXMLGen().AddChildElem( _T("Column"), _T("Messages Received") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
			GetXMLGen().AddChildElem( _T("Results") );	
		}
		else
		{
			sString.Format( _T("%s\r\n"), m_sTitle.c_str() );
			OutputCDF( sString );

			if ( tstdr.m_ReportObjectID == 10 )
			{
				sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"),	_T("Hour"), _T("Messages Received"), _T("Percentage") );
			}
			else
			{
				sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), _T("Day"), _T("Messages Received"), _T("Percentage") );
			}
						
			OutputCDF( sString );			
		}
		break;
	case 10:
		m_sTitle = _T("Ticket Activity by TicketBox");
		
		if ( tsr.m_TargetID == EMS_REPORT_ALL )
		{
			switch ( tstdr.m_ReportObjectID )
			{
			case 1:
				sSubTitle.assign( _T("All TicketBoxes") );
				break;							
			}
		}
		else
		{	
			switch ( tstdr.m_ReportObjectID )
			{
			case 1:
				sSubTitle.assign( _T("TicketBox - ") );
				GetTicketBoxName( tsr.m_TargetID, sString );
				break;				
			}

			sSubTitle += sString;
		}

		sSubTitle += _T(" [");
		GetDateTimeString( trr.m_DateFrom, trr.m_DateFromLen, sString, true, false );
		sSubTitle += sString.c_str();
		sSubTitle += _T(" - ");
		GetDateTimeString( trr.m_DateTo, trr.m_DateToLen, sString, true, false );
		sSubTitle += sString.c_str();
		sSubTitle += _T("]");

		sSubTitle.EscapeHTML();
		if ( !m_OutputCDF )
		{
			GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
			GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
			GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Ticket_Activity.htm") );
			// define columns
			GetXMLGen().AddChildElem( _T("Column"),_T("TicketBox") );
			GetXMLGen().AddChildElem( _T("Column"), _T("New") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Moved To") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Opened") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Restored") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Total In") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Closed") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Deleted") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Moved From") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Total Out") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Total") );		
			GetXMLGen().AddChildElem( _T("Results") );
		}
		else
		{
			CEMSString sLine;
			OutputCDF( tstring("Ticket Activity by TicketBox\r\n") );
			sLine.Format( _T("\"%s\"\r\n"),sSubTitle.c_str() );
			OutputCDF( sLine );
			sLine.Format( _T("\r\n") );
			OutputCDF( sLine );
			sLine.Format( _T("\"TicketBox\",\"New\",\"Moved To\",\"Opened\",\"Restored\",\"Total In\",\"Closed\",\"Deleted\",\"Moved From\",\"Total Out\",\"Total\"\r\n") );
			OutputCDF( sLine );			
		}
		break;	
	}	
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Query all Results	              
\*--------------------------------------------------------------------------*/
void CViewResults::Query(void)
{
	CEMSString sDate;
	CEMSString nLastResult;
	
	GetISAPIData().GetURLLong( _T("ID"), trr.m_ScheduledReportID  );
	if ( trr.m_ScheduledReportID > 0 )
	{
		trr.Query( GetQuery() );
	
		GetXMLGen().AddChildElem( _T("ReportResults") );
		GetXMLGen().AddChildAttrib( _T("SRID"), trr.m_ScheduledReportID );
		GetXMLGen().IntoElem();

		while( GetQuery().Fetch() == S_OK )
			{
			switch( trr.m_ResultCode )
			{
			case 0:	// Unknown
				nLastResult.Format(_T("Unknown"));
				break;	
			case 1:	// Success
				nLastResult.Format(_T("Success"));
				break;
			case 2: //Fail
				nLastResult.Format(_T("Fail"));
				break;
			}
			
			GetXMLGen().AddChildElem( _T("Result") );
			GetXMLGen().AddChildAttrib( _T("ID"), trr.m_ReportResultID );
			GetXMLGen().AddChildAttrib( _T("ResultCode"), nLastResult.c_str() );
			GetDateTimeString( trr.m_DateRan, trr.m_DateRanLen, sDate );
			if ( sDate == "NULL DATE" )
				sDate.Format( _T("Unknown"));
			GetXMLGen().AddChildAttrib( _T("DateRan"), sDate.c_str() );
			GetDateTimeString( trr.m_DateFrom, trr.m_DateFromLen, sDate );
			if ( sDate == "NULL DATE" )
				sDate.Format( _T("Unknown"));
			GetXMLGen().AddChildAttrib( _T("DateFrom"), sDate.c_str() );
			GetDateTimeString( trr.m_DateTo, trr.m_DateToLen, sDate );
			if ( sDate == "NULL DATE" )
				sDate.Format( _T("Unknown"));
			GetXMLGen().AddChildAttrib( _T("DateTo"), sDate.c_str() );
			GetXMLGen().AddChildAttrib( _T("ResultFile"), trr.m_ResultFile );
		}

		GetXMLGen().OutOfElem();
	}
	else
	{
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Invalid Report ID!") );
	}
	
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes one or more ViewResults	              
\*--------------------------------------------------------------------------*/
void CViewResults::Delete(void)
{
	CEMSString sIDs;
	CEMSString sReportFileName;
	CEMSString sTempPath;
	long sTempPathLen = sizeof(sTempPath);
	
	//Get the report folder from the registry
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_REPORT_PATH_VALUE, sTempPath );

	if( sTempPathLen > 0 ) 
	{
		// Remove all backslashes at end
		while( sTempPath.at( sTempPath.length() - 1 ) == _T('\\') )
		{
			sTempPath.resize( sTempPath.length() - 1);
		}		
	}
	
	GetISAPIData().GetXMLString( _T("SELECTID"), sIDs  );
	
	while ( sIDs.CDLGetNextInt( trr.m_ReportResultID ) )
	{
		if ( trr.m_ReportResultID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("Invalid Result ID!") );

		//Delete Row Results for this Report Result
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), trr.m_ReportResultID );
		GetQuery().Execute( _T("DELETE FROM ReportResultRows WHERE ReportResultID=?") );

		//Delete Report Files for this Report Result
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), trr.m_ReportResultID );
		BINDCOL_TCHAR( GetQuery(), trr.m_ResultFile );
		GetQuery().Execute( _T("SELECT ResultFile FROM ReportResults WHERE ReportResultID=?") );
		
		if ( GetQuery().Fetch() == S_OK )
		{
			sReportFileName.Format( _T("%s\\%s"), sTempPath.c_str(), trr.m_ResultFile );
			DeleteFile ( sReportFileName.c_str() );
		}
				
		trr.Delete( GetQuery() );
		//DeleteFile
	}	
}

LPCTSTR CViewResults::GetGraphColor(int nRowIndex )
{
	LPTSTR ColorArray[]= {"#FF9966", "#C2A00E", "#71A214", "#3399CC", 
						  "#CC99FF", "#FF9999", "#00CC99", "#A00000", 
						  "#0000A0", "#FFFF00", "#00FF00", "#FF0000", 
						  "#0000FF", "#800080", "#FFA000", "#00FFFF", 
						  "#008080", "#FF00A0", "#CCCCCC"};
	
	return ColorArray[nRowIndex % 19];
}


////////////////////////////////////////////////////////////////////////////////
// 
// OpenCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CViewResults::OpenCDF( CURLAction& action )
{
	// check security
	//RequireAgentRightLevel( EMS_OBJECT_TYPE_STANDARD_REPORTS, 0, EMS_EDIT_ACCESS );
	
	TCHAR szTempPath[MAX_PATH];
	
	GetTempPath( MAX_PATH, szTempPath );
	GetTempFileName( szTempPath, _T("ems"), 0, m_szTempFile );
	
	m_hCDFFile = CreateFile( m_szTempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
			                 FILE_ATTRIBUTE_NORMAL, NULL );

	if( m_hCDFFile == INVALID_HANDLE_VALUE )
		THROW_EMS_EXCEPTION( E_SystemError, _T("Error creating temp file") );
	
	action.SetSendTempFile( m_szTempFile, _T("report.csv") );

	m_OutputCDF = true;
}

////////////////////////////////////////////////////////////////////////////////
// 
// OutputCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CViewResults::OutputCDF( tstring& sLine )
{
	DWORD dwBytes;

	WriteFile( m_hCDFFile, sLine.c_str(), sLine.size(), &dwBytes, NULL );
}

////////////////////////////////////////////////////////////////////////////////
// 
// CloseCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CViewResults::CloseCDF(void)
{
	if( m_OutputCDF )
		CloseHandle( m_hCDFFile );
}

CPrintReportV::CPrintReportV( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

int CPrintReportV::Run( CURLAction& action )
{
	SYSTEMTIME st;
	CEMSString sTime;

	GetLocalTime( &st );
	GetDateTimeString( st, sTime );

	GetXMLGen().AddChildElem( _T("PrintReport") );
	GetXMLGen().AddChildAttrib( _T("Date"), sTime.c_str() );

	return 0;
}