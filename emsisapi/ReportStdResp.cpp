/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportStdResp.cpp,v 1.2 2005/11/29 21:16:26 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Standard Response Distribution Report   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ReportStdResp.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CReportStdResp::CReportStdResp(CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_bCategory = false;
	m_include_zero = FALSE;
	m_uncategorized = FALSE;
	m_nTotal = 0;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CReportStdResp::Run(CURLAction& action)
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
void CReportStdResp::DecodeForm()
{
	tstring sType;
	GetISAPIData().GetURLString( _T("type"), sType, true );
	m_bCategory = sType.compare( _T("category") ) == 0;

	ISAPIPARAM param;

	if ( GetISAPIData().FindURLParam( _T("graph_type"), param ) )
	{
		m_include_zero = GetISAPIData().FindURLParam( _T("include_zero"), param );
		m_uncategorized = GetISAPIData().FindURLParam( _T("uncategorized"), param );
		m_bPost = true;
	}

	m_bCategory ? m_sTitle.assign( _T("Standard Response Distribution by Category") ) : 
	              m_sTitle.assign( _T("Standard Response Distribution by Author") );
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query the database	              
\*--------------------------------------------------------------------------*/
void CReportStdResp::Query()
{
	// security check
	if ( GetAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0) < EMS_READ_ACCESS )
		return;

	m_bCategory ? QueryCategory() : QueryAuthor();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query for Std Response distribution by Category	              
\*--------------------------------------------------------------------------*/
void CReportStdResp::QueryCategory()
{
	CIDCount IDCount;

	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nID );
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nCount );

	GetQuery().Execute( _T("SELECT StdResponseCatID, (SELECT COUNT(*) FROM StandardResponses ")
						_T("WHERE StdResponseCatID=StdResponseCategories.StdResponseCatID AND ")
						_T("IsApproved=1 AND IsDeleted=0)" )
						_T("FROM StdResponseCategories ORDER BY CategoryName") );
	
	while ( GetQuery().Fetch() == S_OK )
	{
		if ( IDCount.m_nCount > 0 || m_include_zero )
		{
			IDCount.m_nID != EMS_STDRESP_DRAFTS ? m_list.push_back( IDCount ) : m_list.push_front( IDCount );
			m_nTotal += IDCount.m_nCount;
		}
	}
	
	// include the uncategorized responses
	if ( m_uncategorized )
	{
		GetQuery().Reset(false);
		GetQuery().Execute( _T("SELECT 0, COUNT(*) FROM StandardResponses ")
			                _T("WHERE StdResponseCatID=0 AND IsApproved=1 AND IsDeleted=0") );
		
		GetQuery().Fetch();
		
		if ( IDCount.m_nCount > 0 || m_include_zero )
		{
			m_nTotal += IDCount.m_nCount;
			m_list.push_front( IDCount );
		}
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query for Std Response distribution by Author              
\*--------------------------------------------------------------------------*/
void CReportStdResp::QueryAuthor()
{
	CIDCount IDCount;
	
	map<int,int> count_map;
	map<int,int>::iterator map_iter;
	
	list<CIDCount> count_list;
	list<unsigned int>::iterator iter;
	
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nID );
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nCount );
	
	GetQuery().Execute( _T("SELECT AgentID, COUNT(*) FROM StandardResponses GROUP BY AgentID") );
	
	// fetch the results into the map
	while ( GetQuery().Fetch() == S_OK )
	{
		count_map[IDCount.m_nID] = IDCount.m_nCount;
	}

	// iterate through the list of agent IDs from the cache
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

	// include standard responses where the agent is unknown
	if ( m_uncategorized )
	{
		GetQuery().Reset(false);
		GetQuery().Execute( _T("SELECT 0,COUNT(*) FROM StandardResponses ")
							_T("WHERE AgentID NOT IN (SELECT AgentID FROM Agents)") );

		GetQuery().Fetch();
		{
			if ( IDCount.m_nCount > 0 || m_include_zero )
			{
				m_nTotal += IDCount.m_nCount;
				m_list.push_front( IDCount );
			}
		}
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Generate output XML
\*--------------------------------------------------------------------------*/
void CReportStdResp::AddReportHeader()
{
	// define columns
	if ( !m_OutputCDF )
	{
		GetXMLGen().AddChildElem( _T("Column"), m_bCategory ? _T("Category") : _T("Author") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Responses") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
	}
	else
	{
		CEMSString sString;
		sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), m_bCategory ? _T("Category") : _T("Author"), _T("Responses"), _T("Percentage") );
		OutputCDF( sString );
	}

	// add rows
	list<CIDCount>::iterator iter;
	int i=0;

	for ( iter = m_list.begin(); iter != m_list.end(); iter++ )
	{
		CEMSString sName( _T("Unknown") );
		CEMSString sCount, sPercent;

		m_bCategory ? GetStdCategoryName( iter->m_nID, sName ) : GetAgentName( iter->m_nID, sName );
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

			CEMSString sString;
			sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"),  sName.c_str(), sCount.c_str(), sPercent.c_str() );
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
||  Comments:	Add Form Field Options for the report	              
\*--------------------------------------------------------------------------*/
void CReportStdResp::AddReportOptions(void)
{
	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().IntoElem();
	{	
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
		{
			// add the option to include uncategorized responses
			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
			GetXMLGen().AddChildAttrib( _T("NAME"), _T("uncategorized") );
			GetXMLGen().AddChildAttrib( _T("CHECKED"), m_uncategorized );
			GetXMLGen().IntoElem();
			{
				GetXMLGen().AddChildElem( _T("LABLE"), m_bCategory ?
					_T("Include Uncategorized Responses") : _T("Include Responses from Unknown Author") );
			}
			GetXMLGen().OutOfElem();

			// add the option to exclude items with zero records
			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
			GetXMLGen().AddChildAttrib( _T("NAME"), _T("include_zero") );
			GetXMLGen().AddChildAttrib( _T("CHECKED"), m_include_zero );
			GetXMLGen().IntoElem();
			{
				GetXMLGen().AddChildElem( _T("LABLE"), m_bCategory ?
					_T("Include Categories with 0 Responses") : _T("Include Authors with 0 Responses") );
			}
			GetXMLGen().OutOfElem();
		}
		GetXMLGen().OutOfElem();
	}
	GetXMLGen().OutOfElem();

	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/std_distribution.htm") );
}


