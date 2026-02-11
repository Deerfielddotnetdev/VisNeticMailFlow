/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportOpenTicketAge.cpp,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ReportOpenTicketAge.h"

/*---------------------------------------------------------------------------\                     
||  Comments: Construction
\*--------------------------------------------------------------------------*/
CReportOpenTicketAge::CReportOpenTicketAge(CISAPIData& ISAPIData) : CReports( ISAPIData )
{
	m_bPost = false;
	m_bTicketBox = false;
	m_bTicketCategory = false;
	m_bGroup = false;
	m_nFilter = EMS_REPORT_ALL;
	m_ReportInterval = 1;
	m_sTitle = _T("Open Ticket Age");
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point
\*--------------------------------------------------------------------------*/
int CReportOpenTicketAge::Run( CURLAction& action )
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

/*---------------------------------------------------------------------------\                     
||  Comments:	Get request paramaters	              
\*--------------------------------------------------------------------------*/
void CReportOpenTicketAge::DecodeForm( void )
{
	tstring sType;
	GetISAPIData().GetURLString( _T("type"), sType, true );
	m_bTicketBox = sType.compare( _T("ticketbox") ) == 0;
	m_bTicketCategory = sType.compare( _T("ticketcategory") ) == 0;
	m_bGroup = sType.compare( _T("group") ) == 0;

	if ( GetISAPIData().GetURLLong( _T("TICKETBOX"), m_nFilter, true ) ||
		 GetISAPIData().GetURLLong( _T("AGENT"), m_nFilter, true ) ||
		 GetISAPIData().GetURLLong( _T("GROUP"), m_nFilter, true ) ||
		 GetISAPIData().GetURLLong( _T("TICKET_CATEGORY"), m_nFilter, true ) )
	{
		m_bPost = true;

		GetISAPIData().GetURLLong( _T("SUMMARY_PERIOD_SEL"), m_ReportInterval );
		GetISAPIData().GetURLLong( _T("MAX_LINES"), m_MaxRows );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query the database and output the results as XML	              
\*--------------------------------------------------------------------------*/
void CReportOpenTicketAge::Query( void )
{
	CEMSString sWhen;
	CEMSString sFilter;
	CEMSString sString;
	CEMSString sJoin;
	CEMSString sWhere;	
	int nCount;
	int nWhen;

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
	else if ( m_bGroup && m_nFilter > 0 )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_GROUP, m_nFilter, EMS_READ_ACCESS );
		sFilter.Format( _T("AND T.OwnerID IN (SELECT AgentID FROM AgentGroupings WHERE GroupID=%d)"), m_nFilter );
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
	
	GetWhen( sWhen );
	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), nCount );
	BINDCOL_LONG_NOLEN( GetQuery(), nWhen );

	if ( !GetIsAdmin() && g_ThreadPool.GetSharedObjects().m_bUseATC )
	{
		sString.Format( _T("SELECT TOP %d COUNT(*),%s FROM Tickets as T ")
						_T("INNER JOIN AgentTicketCategories ATC ON ")
					    _T("T.TicketCategoryID=ATC.TicketCategoryID ")
					    _T("WHERE ATC.AgentID=%d AND OpenTimeStamp < getdate() AND TicketStateID>1 AND IsDeleted=0 %s ")
					    _T("GROUP BY %s ORDER BY %s "), 
					    m_MaxRows + 1, sWhen.c_str(), GetAgentID(), sFilter.c_str(), sWhen.c_str(), sWhen.c_str() );
	}
	else
	{
		sString.Format( _T("SELECT TOP %d COUNT(*),%s FROM Tickets as T ")
					    _T("WHERE OpenTimeStamp < getdate() AND TicketStateID>1 AND IsDeleted=0 %s ")
					    _T("GROUP BY %s ORDER BY %s "), 
					    m_MaxRows + 1, sWhen.c_str(), sFilter.c_str(), sWhen.c_str(), sWhen.c_str() );
	}

	GetQuery().Execute( sString.c_str() );
	int nRows = 0;
	for ( nRows = 0; GetQuery().Fetch() == S_OK; nRows++ )
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

		FormatTimeString( nWhen, sString );

		if (! m_OutputCDF )
		{
			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
		
			GetXMLGen().IntoElem();
			{			
				GetXMLGen().AddChildElem( _T("Cell"), sString.c_str() );

				sWhen.EscapeJavascript();
				GetXMLGen().AddChildElem( _T("NameEsc"), sString.c_str() );

				sString.Format( _T("%d"), nCount );
				GetXMLGen().AddChildElem( _T("Cell"), sString.c_str()  );

				GetXMLGen().OutOfElem();
			}
		}
		else
		{
			// add line to CSV file
			CEMSString sLine;
			sLine.Format( "\"%s\",\"%d\"\r\n", sString.c_str(), nCount );
			OutputCDF( sLine );
		}
	}
	
	if ( nRows == 0 )
		OutputCDF( CEMSString( _T("No results returned\r\n") ) );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void CReportOpenTicketAge::AddReportHeader(void)
{
	if (! m_OutputCDF )
	{
		// build the sub-title
		CEMSString sSubTitle;
		
		if( m_bTicketBox )
		{
			sSubTitle.assign( _T("Unknown TicketBox") );
		}
		else if( m_bTicketCategory )
		{
			sSubTitle.assign( _T("Unknown Ticket Category") );
		}
		else if( m_bGroup )
		{
			sSubTitle.assign( _T("Unknown Group") );
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
			else if( m_bGroup )
			{
				sSubTitle.assign( _T("All Groups") );
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
				sSubTitle.append( _T(" - TicketBox") );
			}
			else if( m_bTicketCategory )
			{
				GetTicketCategoryName( m_nFilter, sSubTitle );
				sSubTitle.append( _T(" - Ticket Category") );
			}
			else if( m_bGroup )
			{
				GetGroupName( m_nFilter, sSubTitle );
				sSubTitle.append( _T(" - Group") );
			}
			else
			{
				GetAgentName( m_nFilter, sSubTitle );
				sSubTitle.append( _T(" - Agent") );
			}
		}

		// define columns
		sSubTitle.EscapeHTML();
		GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
		GetXMLGen().AddChildElem( _T("Column"), _T("Ticket Age") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Number of Tickets") );
	}
	else
	{
		OutputCDF( tstring("Ticket Age, Number of Tickets\r\n") );
	}

	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add Form Field Options for the report	              
\*--------------------------------------------------------------------------*/
void CReportOpenTicketAge::AddReportOptions(void)
{
	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Open_Ticket_Totals.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().IntoElem();
		
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
			else if( m_bGroup )
			{
				AddGroups(m_nFilter);
			}
			else
			{
				AddAgents(m_nFilter);
			}
			AddDateIntervals( _T("Report By:"), m_ReportInterval );
			AddMaxLines( m_MaxRows );
		GetXMLGen().OutOfElem();
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	SQL syntax to determine the age of a ticket
||				TODO: This logic needs to be revisited as
||				the length of a month is assumed to be 30 days
||				and a year is assumed to be 365 days.	              
\*--------------------------------------------------------------------------*/
void CReportOpenTicketAge::GetWhen( CEMSString& sWhen )
{
	switch( m_ReportInterval )
	{
	case 0:	// Hour
		sWhen.assign( _T("DATEDIFF(mi,OpenTimeStamp,getdate()) / 60") );
		break;
	
	case 1:	// Day
		sWhen.assign( _T("DATEDIFF(mi,OpenTimeStamp,getdate()) / 1440") );
		break;

	case 2: // Week
		sWhen.assign( _T("DATEDIFF(mi,OpenTimeStamp,getdate()) / 10080") );
		break;

	case 3: // Month
		sWhen.assign( _T("DATEDIFF(mi,OpenTimeStamp,getdate()) / 43200") );
		break;

	case 4: // Year
		sWhen.assign( _T("DATEDIFF(mi,OpenTimeStamp,getdate()) / 525600") );
		break;

	default:
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}
}

void CReportOpenTicketAge::FormatTimeString( int nWhen, CEMSString& sWhen )
{
	switch( m_ReportInterval )
	{
	case 0:	// Hour
		if( nWhen == 0 )
			sWhen.assign( _T("less than an hour") );
		else if (nWhen == 1)
			sWhen.assign( _T("1 hour") );
		else
			sWhen.Format( _T("%d hours"), nWhen );
		break;
	
	case 1:	// Day
		if( nWhen == 0 )
			sWhen.assign( _T("less than a day") );
		else if (nWhen == 1)
			sWhen.assign( _T("1 day") );
		else
			sWhen.Format( _T("%d days"), nWhen );
		break;

	case 2: // Week
		if( nWhen == 0 )
			sWhen.assign( _T("less than a week") );
		else if (nWhen == 1)
			sWhen.assign( _T("1 week") );
		else
			sWhen.Format( _T("%d weeks"), nWhen );
		break;

	case 3: // Month
		if( nWhen == 0 )
			sWhen.assign( _T("less than a month") );
		else if (nWhen == 1)
			sWhen.assign( _T("1 month") );
		else
			sWhen.Format( _T("%d months"), nWhen );
		break;

	case 4: // Year
		if( nWhen == 0 )
			sWhen.assign( _T("less than a year") );
		else if (nWhen == 1)
			sWhen.assign( _T("1 year") );
		else
			sWhen.Format( _T("%d years"), nWhen );
		break;
	}
}