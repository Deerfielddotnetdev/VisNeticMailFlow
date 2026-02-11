/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportStdRespUsage.cpp,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Standard Response Usage Report   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ReportStdRespUsage.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CReportStdRespUsage::CReportStdRespUsage( CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_nFilter = EMS_REPORT_ALL;
	m_nTotal = 0;
	m_nDateRange = 0;
	m_include_zero = FALSE;
	m_include_auto = FALSE;
	m_sTitle = _T("Standard Response Usage");
	
	ZeroMemory( &m_StartDate, sizeof(m_StartDate) );
	ZeroMemory( &m_EndDate, sizeof(m_EndDate) );
	m_EndDate.hour = 23;
	m_EndDate.minute = 59;
	m_EndDate.second = 59;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CReportStdRespUsage::Run(CURLAction& action)
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
void CReportStdRespUsage::DecodeForm()
{
	if (GetISAPIData().GetURLLong( _T("STDCAT"), m_nFilter, true ))
	{
		ISAPIPARAM param;
		CEMSString sDate;

		m_bPost = true;
		
		m_include_zero = GetISAPIData().FindURLParam( _T("include_zero"), param );
		m_include_auto = GetISAPIData().FindURLParam( _T("include_auto"), param );
		
		GetISAPIData().GetURLLong( _T("DATE_RANGE_SEL"), m_nDateRange, true );
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );
		
		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EndDate );
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query the database	              
\*--------------------------------------------------------------------------*/
void CReportStdRespUsage::Query()
{
	CEMSString sSQL;
	CEMSString sString;
	CEMSString sName; 
	CEMSString sCategory;
	CEMSString sCount; 
	CEMSString sPercent;

	TCHAR szName[STANDARDRESPONSES_SUBJECT_LENGTH];
	int nCategoryID;
	int nCount;
	int i=0;

	if ( m_nFilter > EMS_REPORT_ALL )
		sString.Format( _T("AND S.StdResponseCatID=%d "), m_nFilter );
	
	if ( !m_include_auto )
		sString += _T("AND U.AgentID != 0");

	// query for the total usage
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), m_nTotal );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );

	sSQL.Format( _T("SELECT COUNT(*) FROM StandardResponseUsage as U ") 
				 _T("INNER JOIN StandardResponses as S on S.StandardResponseID = U.StandardResponseID ") 
				 _T("WHERE U.DateUsed BETWEEN ? AND ? %s"), sString.c_str() );
	
	GetQuery().Execute( sSQL.c_str() );
	GetQuery().Fetch();

	// query for the usage by response
	GetQuery().Initialize();
	BINDCOL_TCHAR_NOLEN( GetQuery(), szName );
	BINDCOL_LONG_NOLEN( GetQuery(), nCategoryID );
	BINDCOL_LONG_NOLEN( GetQuery(), nCount );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );

	if ( ! m_include_zero )
	{
		sSQL.Format( _T("SELECT S.Subject, StdResponseCatID, COUNT(*) FROM StandardResponseUsage as U ") 
					 _T("INNER JOIN StandardResponses as S on S.StandardResponseID = U.StandardResponseID ") 
					 _T("WHERE U.DateUsed BETWEEN ? AND ? %s ")
					 _T("GROUP BY U.StandardResponseID, S.Subject, StdResponseCatID ORDER BY COUNT(*) DESC"), sString.c_str() );
	}
	else
	{
		CEMSString sCategory;
		
		if ( m_nFilter > EMS_REPORT_ALL )
			sCategory.Format( _T("WHERE S.StdResponseCatID=%d"), m_nFilter );
		
		sString = _T("AND U.AgentID != 0");
		
		sSQL.Format( _T("SELECT Subject,StdResponseCatID, (SELECT Count(*) FROM StandardResponseUsage as U ")
					 _T("WHERE StandardResponseID = S.StandardResponseID AND ")
				 	 _T("DateUsed BETWEEN ? AND ? %s) ")
				  	 _T("FROM StandardResponses as S %s ORDER BY 3 DESC"), sString.c_str(), sCategory.c_str() );
	}


	GetQuery().Execute( sSQL.c_str() );

	// fetch the results and generate XML
	while ( GetQuery().Fetch() == S_OK )
	{
		sName = szName;
		sCount.Format( _T("%d"), nCount );
		sPercent.Format( _T("%.2f"), m_nTotal ? ((float) nCount / (float) m_nTotal) * 100 : 0);
		GetStdCategoryName( nCategoryID, sCategory );

		if ( !m_OutputCDF )
		{
			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("Color"), GetGraphColor(i++) );
			
			GetXMLGen().IntoElem();

				sName.EscapeHTML();
				GetXMLGen().AddChildElem( _T("Cell"), sName.c_str()  );

				sName = szName;
				sName.EscapeJavascript();
				GetXMLGen().AddChildElem( _T("NameEsc"), sName.c_str() );

				GetXMLGen().AddChildElem( _T("Cell"), sCount.c_str() );
				GetXMLGen().AddChildElem( _T("Cell"), sPercent.c_str() );

				if ( m_nFilter == EMS_REPORT_ALL )
				{
					sCategory.EscapeHTML();
					GetXMLGen().AddChildElem( _T("Cell"), sCategory.c_str() );
				}

			GetXMLGen().OutOfElem();
		}
		else
		{
			sName.EscapeCSV();
			sCategory.EscapeCSV();

			if ( m_nFilter == EMS_REPORT_ALL )
			{
				sString.Format( "\"%s\",\"%s\",\"%s\",\"%s\"\r\n", 
					sName.c_str(), sCount.c_str(), sPercent.c_str(), sCategory.c_str() );
			}
			else
			{
				sString.Format( "\"%s\",\"%s\",\"%s\"\r\n", 
					sName.c_str(), sCount.c_str(), sPercent.c_str() );
			}
			
			OutputCDF( sString );
		}
	}
	
	if ( m_OutputCDF && i == 0 )
	{
		sString.Format( _T("No results returned\r\n") );
		OutputCDF( sString );
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Generate output XML
\*--------------------------------------------------------------------------*/
void CReportStdRespUsage::AddReportHeader()
{
	CEMSString sString;

	if ( !m_OutputCDF )
	{
		CEMSString sSubTitle;

		// build the sub title
		if ( m_nFilter == EMS_REPORT_ALL )
		{
			sSubTitle.assign( _T("All Categories") );
		}
		else
		{
			GetStdCategoryName( m_nFilter, sSubTitle );
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

		if ( m_nFilter == EMS_REPORT_ALL )
			GetXMLGen().AddChildElem( _T("Column"), _T("Category") );
	}
	else
	{
		CEMSString sString;

		if ( m_nFilter == EMS_REPORT_ALL )
		{
			sString.Format( _T("\"%s\",\"%s\",\"%s\",\"%s\"\r\n"), 
				_T("Standard Response"), _T("Times Used"), _T("Percentage"), _T("Category") );
		}
		else
		{
			sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), 
				_T("Standard Response"), _T("Times Used"), _T("Percentage") );
		}

		OutputCDF( sString );
	}

	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add options (fields) for the report	              
\*--------------------------------------------------------------------------*/
void CReportStdRespUsage::AddReportOptions(void)
{
	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().IntoElem();
	
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddStdCategories( m_nFilter );
			AddDateRange( m_nDateRange );
			GetXMLGen().AddChildAttrib( _T("ROWSPAN"), 4 );
		GetXMLGen().OutOfElem();

		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("COLSPAN"), 2 );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
			GetXMLGen().AddChildAttrib( _T("NAME"), _T("include_zero") );
			GetXMLGen().AddChildAttrib( _T("CHECKED"), m_include_zero );
			GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("LABLE"), _T("Include Unused Responses") );
			GetXMLGen().OutOfElem();
		GetXMLGen().OutOfElem();

		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("COLSPAN"), 2 );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
			GetXMLGen().AddChildAttrib( _T("NAME"), _T("include_auto") );
			GetXMLGen().AddChildAttrib( _T("CHECKED"), m_include_auto );
			GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("LABLE"), _T("Include Auto Replies") );
			GetXMLGen().OutOfElem();
		GetXMLGen().OutOfElem();

		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().AddChildAttrib( _T("HEIGHT"), _T("100%") );
		GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("NONE") );
		GetXMLGen().OutOfElem();

	GetXMLGen().OutOfElem();

	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/std_usage.htm") );
}



