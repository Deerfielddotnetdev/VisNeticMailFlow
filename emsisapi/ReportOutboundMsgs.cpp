/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportOutboundMsgs.cpp,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Outbound Message Distribution Report   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ReportOutBoundMsgs.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CReportOutboundMsgs::CReportOutboundMsgs(CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_bCustom = false;
	m_bTicketBox = false;
	m_bTicketCategory = false;
	m_bGroup = false;
	m_include_auto = FALSE;
	m_groupby_ticketbox = FALSE;
	m_nFilter = EMS_REPORT_ALL;
	m_ReportInterval = 1;
	m_nDateRange	 = 0;
	m_nRows = 0;

	ZeroMemory( &m_StartDate, sizeof(m_StartDate) );
	ZeroMemory( &m_EndDate, sizeof(m_EndDate) );
	m_EndDate.hour = 23;
	m_EndDate.minute = 59;
	m_EndDate.second = 59;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CReportOutboundMsgs::Run( CURLAction& action )
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
||  Comments:	Decode URL Parameters	              
\*--------------------------------------------------------------------------*/
void CReportOutboundMsgs::DecodeForm(void)
{	
	int nID;
	tstring sType;
	GetISAPIData().GetURLString( _T("type"), sType, true );
	m_bTicketBox = sType.compare( _T("ticketbox") ) == 0;
	m_bTicketCategory = sType.compare( _T("ticketcategory") ) == 0;
	m_bGroup = sType.compare( _T("group") ) == 0;
	
	if(m_bTicketBox)
	{
		if(GetISAPIData().GetURLString( _T("IDCollection"), idCollection, true ))
		{
			m_bPost = true;
		}
	}
	else if( GetISAPIData().GetURLLong( _T("AGENT"), m_nFilter, true ) ||
		 GetISAPIData().GetURLLong( _T("GROUP"), m_nFilter, true ) ||
		 GetISAPIData().GetURLLong( _T("TICKET_CATEGORY"), m_nFilter, true ) )
	{
		m_bPost = true;
	}
	
	if(m_bPost)
	{
		ISAPIPARAM param;
		
		m_include_auto = GetISAPIData().FindURLParam( _T("INCLUDE_AUTO"), param );
		
		m_groupby_ticketbox = GetISAPIData().FindURLParam( _T("GROUPBY_TICKETBOX"), param );
		if(m_groupby_ticketbox)
			m_bCustom = true;

		GetISAPIData().GetURLLong( _T("SUMMARY_PERIOD_SEL"), m_ReportInterval );
		GetISAPIData().GetURLLong( _T("DATE_RANGE_SEL"), m_nDateRange, true );
		GetISAPIData().GetURLLong( _T("MAX_LINES"), m_MaxRows );

		/*long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
		TIMESTAMP_STRUCT tsLocal;
		long tsLocalLen=0;*/

		CEMSString sDate;
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );

		/*if(lTzBias != 1)
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
		}*/
		
		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EndDate );

		/*if(lTzBias != 1)
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
		}*/

		idCollection.CDLInit();
		while( idCollection.CDLGetNextInt( nID ) )
		{
			if ( nID == -1 )
			{
				m_nFilter = -1;
			}
			else
			{
				m_nFilter = 0;
				m_IDs.push_back( nID );
			}			
		}
	}

	if( m_bTicketBox )
	{
		m_sTitle.assign("Outbound Messages by TicketBox");
	}
	else if( m_bTicketCategory )
	{
		m_sTitle.assign("Outbound Messages by Ticket Category");
	}
	else if( m_bGroup )
	{
		m_sTitle.assign("Outbound Messages by Group");
	}
	else
	{
		m_sTitle.assign("Outbound Messages by Agent");
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void CReportOutboundMsgs::AddReportHeader(void)
{
	if ( !m_OutputCDF )
	{
		CEMSString sSubTitle;
		CEMSString sString;

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
				if ( m_IDs.size() == 1 )
				{
					list<int>::iterator idIter;
					for( idIter = m_IDs.begin(); idIter != m_IDs.end(); idIter++ )
					{
						
						if(!GetTicketBoxName( *idIter, sSubTitle ))
							sSubTitle.assign(_T("Deleted TicketBox"));
						
						sSubTitle.append( _T(" - TicketBox") );
						m_nFilter = *idIter;
					}			
				}
				else
				{
					sSubTitle = _T("Multiple TicketBoxes");				
				}
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
		
		sSubTitle += _T(" [");
		
		/*long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
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
		}*/
		
		GetDateTimeString( m_StartDate, sizeof(m_StartDate), sString, true, false );
		sSubTitle += sString.c_str();
		sSubTitle += _T(" - ");
		
		/*if(lTzBias != 1)
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
		}*/
		
		GetDateTimeString( m_EndDate, sizeof(m_EndDate), sString, true, false );
		sSubTitle += sString.c_str();
		sSubTitle += _T("]");

		// add report paramaters
		sSubTitle.EscapeHTML();
		GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );

		// define columns
		if(m_bTicketBox && m_bCustom)
		{
			GetXMLGen().AddChildElem( _T("Column"), _T("TicketBox") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Number of Messages") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Date") );
		}
		else
		{
			GetXMLGen().AddChildElem( _T("Column"), _T("Date") );
			GetXMLGen().AddChildElem( _T("Column"), _T("Number of Messages") );
		}
	}
	else
	{
		if(m_bTicketBox && m_bCustom)
		{
			OutputCDF( tstring("TicketBox,Number of Messages,Date\r\n") );
		}
		else
		{
			OutputCDF( tstring("Date, Number of Messages\r\n") );
		}
	}

	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add Form Field Options for the report	              
\*--------------------------------------------------------------------------*/
void CReportOutboundMsgs::AddReportOptions(void)
{
	int nReportObjectID;

	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Outbound_Message.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().IntoElem();

	// filter drop-down and date range controls
	GetXMLGen().AddChildElem( _T("ROW") );
	GetXMLGen().IntoElem();
	if( m_bTicketBox )
	{
		AddTicketBoxes(m_nFilter, true);
		nReportObjectID=1;				
	}
	else if( m_bTicketCategory )
	{
		AddTicketCategories(m_nFilter);
		nReportObjectID=3;
	}
	else if( m_bGroup )
	{
		AddGroups(m_nFilter);
		nReportObjectID=6;
	}
	else
	{
		AddAgents(m_nFilter);
		nReportObjectID=2;
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

	if ( m_bTicketBox )
	{
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();

		GetXMLGen().AddChildElem( _T("FORM_FIELD") );
		GetXMLGen().AddChildAttrib( _T("COLSPAN"), 2 );
		GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
		GetXMLGen().AddChildAttrib( _T("NAME"), _T("groupby_ticketbox") );
		GetXMLGen().AddChildAttrib( _T("CHECKED"), m_groupby_ticketbox );

		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("LABLE"), _T("Group Results by TicketBox") );
		GetXMLGen().OutOfElem();

		GetXMLGen().OutOfElem();
	}

	GetXMLGen().OutOfElem();

	GetXMLGen().AddChildElem( _T("ReportTypeID"), _T("2") );
	GetXMLGen().AddChildElem( _T("ReportObjectID"), nReportObjectID );
	if(m_bCustom)
	{
		GetXMLGen().AddChildElem( _T("GroupByTb"), _T("1") );
	}
	else
	{
		GetXMLGen().AddChildElem( _T("GroupByTb"), _T("0") );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query the database	              
\*--------------------------------------------------------------------------*/
void CReportOutboundMsgs::Query(void)
{
	CEMSString sQuery;
	CEMSString sDatePart;
	CEMSString sFilter;
	int nCount;
	int nTicketBoxID=0;
	int nTbCount=0;
	int nTotalCount=0;
	int nSaveTbID = -99;
	int nSubCount = 0;
	bool bDoSubTotal = false;

	if ( m_nFilter == EMS_REPORT_ALL )
	{	
		RequireAdmin();
	}
	else if ( m_bTicketBox )
	{
		if(m_IDs.size() > 0)
		{
			list<int>::iterator idIter;
			for( idIter = m_IDs.begin(); idIter != m_IDs.end(); idIter++ )
			{
				RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, *idIter, EMS_READ_ACCESS );	
			}
			sFilter.Format( _T("AND O.OriginalTicketBoxID IN (%s)"), idCollection.c_str() );
		}
		else
		{
			RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, m_nFilter, EMS_READ_ACCESS );
			sFilter.Format( _T("AND O.OriginalTicketBoxID=%d"), m_nFilter );
		}
	}
	else if ( m_bTicketCategory )
	{
		sFilter.Format( _T("AND O.TicketCategoryID=%d"), m_nFilter );
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_CATEGORY, m_nFilter, EMS_READ_ACCESS );
	}
	else if ( m_bGroup )
	{
		sFilter.Format( _T("AND O.AgentID IN (SELECT AgentID FROM AgentGroupings WHERE GroupID=%d)"), m_nFilter );
		RequireAgentRightLevel( EMS_OBJECT_TYPE_GROUP, m_nFilter, EMS_READ_ACCESS );
			
	}
	else
	{
		sFilter.Format( _T("AND O.AgentID=%d"), m_nFilter );
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_nFilter, EMS_READ_ACCESS );
	}

	if ( !m_include_auto )
		sFilter.append(" AND O.AgentID>0");

	GetQuery().Initialize();

	if(m_bTicketBox && m_bCustom)
	{
		BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID );
	}
	
	GetWhenSelect( sDatePart, _T("O.EmailDateTime") ); 

	BINDCOL_LONG_NOLEN( GetQuery(), nCount );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );

	if(m_bTicketBox && m_bCustom)
	{
		sQuery.Format( _T("SET DATEFIRST 7 SELECT TOP %d OriginalTicketBoxID,%s, COUNT(*) ")
				   _T("FROM OutboundMessages as O ")
				   _T("WHERE O.IsDeleted=0 AND O.EmailDateTime BETWEEN ? AND ? %s ")
				   _T("GROUP BY OriginalTicketBoxID,%s ORDER BY OriginalTicketBoxID,%s"),
				   m_MaxRows + 1, sDatePart.c_str(), sFilter.c_str(), sDatePart.c_str(), sDatePart.c_str() );
	}
	else
	{
		sQuery.Format( _T("SET DATEFIRST 7 SELECT TOP %d %s, COUNT(*) ")
				   _T("FROM OutboundMessages as O ")
				   _T("WHERE O.IsDeleted=0 AND O.EmailDateTime BETWEEN ? AND ? %s ")
				   _T("GROUP BY %s ORDER BY %s"),
				   m_MaxRows + 1, sDatePart.c_str(), sFilter.c_str(), sDatePart.c_str(), sDatePart.c_str() );
	
	
	}
	GetQuery().Execute( sQuery.c_str() );

	// fetch the data and output the results
	CEMSString sSaveTicketBox,sTicketBox, sDate, sDateEsc, sCount, sTbCount;
	CEMSString sLine;
	int nRows;

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
	
		if(nTicketBoxID > 0)
		{
			if(!GetTicketBoxName( nTicketBoxID, sTicketBox ))
				sTicketBox.Format(_T("Deleted TicketBox"));							
		}
		else
		{
			sTicketBox.Format(_T("Unknown"));
		}
		
		if(m_bTicketBox && m_bCustom)
		{
			nTotalCount = nTotalCount + nCount;

			if(nSaveTbID == nTicketBoxID)
			{
				nTbCount = nTbCount + nCount;
			}
			else
			{
				if(nSaveTbID != -99)
				{
					if(nSaveTbID > 0)
					{
						CEMSString sTempTicketBox;
						if(!GetTicketBoxName( nSaveTbID, sTempTicketBox ))
						{
							sSaveTicketBox.Format(_T("Deleted TicketBox Total"));
						}
						else
						{
							sSaveTicketBox.Format(_T("%s Total"),sTempTicketBox.c_str());
						}
					}
					else
					{
						sSaveTicketBox.Format(_T("Unknown Total"));
					}
					bDoSubTotal = true;
					nSubCount = nTbCount;
					nTbCount = nCount;
				}
				else
				{
					nTbCount = nCount;
				}
				nSaveTbID = nTicketBoxID;
			}
		}
		GetWhen( sDate );
		sDateEsc.assign( sDate );
		sDateEsc.EscapeJavascript();
		sCount.Format( _T("%d"), nCount );

		if (! m_OutputCDF )
		{
			if(bDoSubTotal)
			{
				GetXMLGen().AddChildElem( _T("Row") );
				GetXMLGen().AddChildAttrib( _T("RowType"), _T("1") );		
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sSaveTicketBox.c_str() );
				sTbCount.Format( _T("%d"), nSubCount );
				GetXMLGen().AddChildElem( _T("Cell"), sTbCount.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), _T("") );
				GetXMLGen().OutOfElem();
				bDoSubTotal = false;
				nSubCount = 0;
				sSaveTicketBox.Format(_T(""));
			}

			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(nRows) );
			GetXMLGen().AddChildAttrib( _T("RowType"), _T("0") );
		
			if(m_bTicketBox && m_bCustom)
			{
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sTicketBox.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );				
				GetXMLGen().OutOfElem();
			}
			else
			{
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("Cell"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("NameEsc"), sDateEsc.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
				GetXMLGen().OutOfElem();
			}
		}
		else
		{
			// add line to CSV file
			if(m_bTicketBox && m_bCustom)
			{
				if(bDoSubTotal)
				{
					sTbCount.Format( _T("%d"), nSubCount );
					sLine.Format( "\"%s\",\"%s\",\"%s\"\r\n", sSaveTicketBox.c_str(), sTbCount.c_str(), _T("") );
					OutputCDF( sLine );
					bDoSubTotal = false;
					nSubCount = 0;
					sSaveTicketBox.Format(_T(""));
				}
				sLine.Format( "\"%s\",\"%s\",\"%s\"\r\n", sTicketBox.c_str(), sCount.c_str(), sDate.c_str() );
			}
			else
			{
				sLine.Format( "\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str() );
			}
			OutputCDF( sLine );
		}
	}
	
	if(m_bTicketBox && m_bCustom)
	{
		if (! m_OutputCDF )
		{
			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("RowType"), _T("1") );		
			GetXMLGen().IntoElem();
			sSaveTicketBox.Format(_T("%s Total"),sTicketBox.c_str());
			GetXMLGen().AddChildElem( _T("Cell"), sSaveTicketBox.c_str() );
			sTbCount.Format( _T("%d"), nTbCount );
			GetXMLGen().AddChildElem( _T("Cell"), sTbCount.c_str() );
			GetXMLGen().AddChildElem( _T("Cell"), _T("") );
			GetXMLGen().OutOfElem();

			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("RowType"), _T("2") );		
			GetXMLGen().IntoElem();
			sTicketBox.Format(_T("Grand Total"));
			GetXMLGen().AddChildElem( _T("Cell"), sTicketBox.c_str() );
			sTbCount.Format( _T("%d"), nTotalCount );
			GetXMLGen().AddChildElem( _T("Cell"), sTbCount.c_str() );
			GetXMLGen().AddChildElem( _T("Cell"), _T("") );
			GetXMLGen().OutOfElem();
		}
		else
		{
			sSaveTicketBox.Format(_T("%s Total"),sTicketBox.c_str());
			sTbCount.Format( _T("%d"), nTbCount );
			sLine.Format( "\"%s\",\"%s\",\"%s\"\r\n", sSaveTicketBox.c_str(), sTbCount.c_str(), sDate.c_str() );
			OutputCDF( sLine );

			sTicketBox.Format(_T("Grand Total"));
			sTbCount.Format( _T("%d"), nTotalCount );
			sLine.Format( "\"%s\",\"%s\",\"%s\"\r\n", sTicketBox.c_str(), sTbCount.c_str(), sDate.c_str() );
			OutputCDF( sLine );
		}
		
	}
	
	if ( m_OutputCDF && nRows == 0 )
		OutputCDF( CEMSString( _T("No results returned\r\n") ) );
}
