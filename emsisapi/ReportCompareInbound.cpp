/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportCompareInbound.cpp,v 1.2 2005/11/29 21:16:26 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ReportCompareInbound.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CReportCompare::CReportCompare(CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_bTicketBox = false;
	m_nTotal = 0;
	m_nDateRange = 0;
	m_include_zero = 0;

	ZeroMemory( &m_StartDate, sizeof(m_StartDate) );
	ZeroMemory( &m_EndDate, sizeof(m_EndDate) );
	m_EndDate.hour = 23;
	m_EndDate.minute = 59;
	m_EndDate.second = 59;
}

CReportCompare::~CReportCompare()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main entry point	              
\*--------------------------------------------------------------------------*/
int CReportCompare::Run( CURLAction& action )
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
			Query();
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
void CReportCompare::DecodeForm()
{
	tstring sType;
	GetISAPIData().GetURLString( _T("type"), sType, true );
	m_bTicketBox = sType.compare( _T("ticketbox") ) == 0;
	
	ISAPIPARAM param;
	
	if ( GetISAPIData().FindURLParam( _T("START_DATE"), param ) )
	{
		m_bPost = true;
		m_include_zero = GetISAPIData().FindURLParam( _T("include_zero"), param );
		
		CEMSString sDate;
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );
		
		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EndDate );
	}
	
	m_bTicketBox ? m_sTitle.assign( _T("TicketBox Comparison - Outbound Messages Sent") ) : 
	               m_sTitle.assign( _T("Agent Comparison - Outbound Messages Sent") );
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query the database
\*--------------------------------------------------------------------------*/
void CReportCompare::Query(void)
{
	m_bTicketBox ? QueryTicketBox() : QueryAgent();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query for ticket distribution by TicketBox	              
\*--------------------------------------------------------------------------*/
void CReportCompare::QueryTicketBox(void)
{
	CIDCount IDCount;
	map<int,int> count_map;
	map<int,int>::iterator map_iter;
	list<CIDCount> count_list;
	list<unsigned int>::iterator iter;
	CEMSString sStateID;
	CEMSString sQuery;

	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nID );
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nCount );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
	
	sQuery = _T("SELECT OriginalTicketBoxID, COUNT(*) FROM OutboundMessages ")
		_T("WHERE IsDeleted=0 AND OutboundMessageStateID>1 AND ")
		_T("EmailDateTime BETWEEN ? AND ? GROUP BY OriginalTicketBoxID");

	GetQuery().Execute( sQuery.c_str() );
	
	// fetch the results into the map
	while ( GetQuery().Fetch() == S_OK )
	{
		count_map[IDCount.m_nID] = IDCount.m_nCount;
	}
	
	// for each ticketbox
	for( iter = GetXMLCache().m_TicketBoxIDs.GetList().begin(); 
		 iter != GetXMLCache().m_TicketBoxIDs.GetList().end(); iter++ )
	{
		// check security
		if ( GetAgentRightLevel(EMS_OBJECT_TYPE_TICKET_BOX, *iter) >= EMS_READ_ACCESS )
		{
			IDCount.m_nID = *iter;
			
			if ( (map_iter = count_map.find(IDCount.m_nID)) != count_map.end() )
			{
				IDCount.m_nCount = map_iter->second;
				m_nTotal += IDCount.m_nCount;
			}
			else
			{
				IDCount.m_nCount = 0;
			}
			
			if ( m_include_zero || IDCount.m_nCount > 0 )
				m_list.push_back(IDCount);
		}
	}

	// generate the XML
	GenerateXML();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query for ticket distribution by Owner	              
\*--------------------------------------------------------------------------*/
void CReportCompare::QueryAgent(void)
{
	CIDCount IDCount;
	map<int,int> count_map;
	map<int,int>::iterator map_iter;
	list<CIDCount> count_list;
	list<unsigned int>::iterator iter;
	CEMSString sQuery;
	
	// query the database
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nID );
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nCount );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
	
	sQuery=  _T("SELECT AgentID, COUNT(*) FROM OutboundMessages ")
		_T("WHERE IsDeleted=0 AND OutboundMessageStateID>1 AND ")
		_T("EmailDateTime BETWEEN ? AND ? GROUP BY AgentID");
	
	GetQuery().Execute( sQuery.c_str() );
	
	// fetch the results into the map
	while ( GetQuery().Fetch() == S_OK )
	{
		count_map[IDCount.m_nID] = IDCount.m_nCount;
	}
	
	// for each agent ID stored in the cache
	for( iter = GetXMLCache().m_AgentIDs.GetList().begin(); 
	     iter != GetXMLCache().m_AgentIDs.GetList().end(); iter++ )
	{
		// check security
		if ( GetAgentRightLevel(EMS_OBJECT_TYPE_AGENT, *iter) >= EMS_READ_ACCESS )
		{
			IDCount.m_nID = *iter;
			
			if ( (map_iter = count_map.find(IDCount.m_nID)) != count_map.end() )
			{
				IDCount.m_nCount = map_iter->second;
				m_nTotal += IDCount.m_nCount;
			}
			else
			{
				IDCount.m_nCount = 0;
			}

			if ( m_include_zero || IDCount.m_nCount > 0 )
				m_list.push_back(IDCount);
		}
	}
		 
	// generate the XML
	GenerateXML();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Generate Output XML
\*--------------------------------------------------------------------------*/
void CReportCompare::GenerateXML(void)
{
	CEMSString sString;
	
	if ( !m_OutputCDF )
	{
		// define columns
		GetXMLGen().AddChildElem( _T("Column"), m_bTicketBox ? _T("TicketBox") : _T("Agent") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Messages") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
	}
	else
	{
		sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), 
			            m_bTicketBox ? _T("TicketBox") : _T("Agent"), _T("Messages"), _T("Percentage") );
		OutputCDF( sString );
	}

	// add rows
	list<CIDCount>::iterator iter;
	int i=0;

	for ( iter = m_list.begin(); iter != m_list.end(); iter++ )
	{
		CEMSString sName( _T("Not Owned") );
		CEMSString sCount, sPercent;

		m_bTicketBox ? GetTicketBoxName( iter->m_nID, sName ) : GetAgentName( iter->m_nID, sName );
		sCount.Format( _T("%d"), iter->m_nCount );
		sPercent.Format( _T("%.2f"), m_nTotal ? ((float) iter->m_nCount / (float) m_nTotal) * 100 : 0);
		
		if ( !m_OutputCDF )
		{
			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(i++) );
			
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("Cell"), sName.c_str() );
			sName.EscapeJavascript();
			GetXMLGen().AddChildElem( _T("NameEsc"), sName.c_str() );
			GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
			GetXMLGen().AddChildElem( _T("Cell"), sPercent.c_str() );
			GetXMLGen().OutOfElem();
		}
		else
		{
			sName.EscapeCSV();

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

	
	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void CReportCompare::AddReportHeader()
{
	if ( !m_OutputCDF )
	{
	}

	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add Form Field Options for the report	              
\*--------------------------------------------------------------------------*/
void CReportCompare::AddReportOptions(void)
{
	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/compare.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();

			// add the option to exclude items with zero records
			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
			GetXMLGen().AddChildAttrib( _T("NAME"), _T("include_zero") );
			GetXMLGen().AddChildAttrib( _T("CHECKED"), m_include_zero );
			GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("LABLE"), m_bTicketBox ? 
				_T("Include TicketBoxes with 0 Messages") : _T("Include Agents with 0 Messages") );
			GetXMLGen().OutOfElem();

			AddDateRange( m_nDateRange );
		GetXMLGen().OutOfElem();
	GetXMLGen().OutOfElem();
}
