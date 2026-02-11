////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/Reporter.cpp,v 1.1 2005/08/09 16:40:55 markm Exp $
//
//  Copyright © 2000 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// This object is a memmber of WorkerThread
//
//////////////////////////////////////////////////////////////////////
// Reporter.cpp: implementation of the CReporter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Reporter.h"
#include "RegistryFns.h"
#include "DateFns.h"
#include "AttachFns.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CReporter::CReporter( CODBCQuery& m_query ) : m_query(m_query)
{
	m_LastScheduledReportCheck = 0;
	m_ThisResult = 1;
	Col1.Format( _T(""));
	Col2.Format( _T(""));
	Col3 = 0;
	Col4 = 0;
	Col5 = 0;	
}

CReporter::~CReporter()
{
	
}


////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
HRESULT CReporter::Run(void)
{
	long TickCount = GetTickCount();
	long lDiff = GetTickCount() - m_LastScheduledReportCheck;						
	
	// Scheduled Reports
	if( m_LastScheduledReportCheck == 0 || ((lDiff > ScheduledReportCheckIntervalMs) || (lDiff < 0)) )
	{
		DebugReporter::Instance().DisplayMessage("CReporter::Run", DebugReporter::ENGINE);

		if ( _wtoi(g_Object.GetParameter( EMS_SRVPARAM_ENABLE_SCHEDULED_REPORTS )) )
		{
			DebugReporter::Instance().DisplayMessage("CReporter::Run - Scheduled Reports Enabled", DebugReporter::ENGINE);
			if ( _wtoi(g_Object.GetParameter( EMS_SRVPARAM_REPORTER )) )
			{
				DebugReporter::Instance().DisplayMessage("CReporter::Run - Reporter Server Task Enabled", DebugReporter::ENGINE);
				if( GetReportToRun() )
				{
					DebugReporter::Instance().DisplayMessage("CReporter::Run - Found Report to run", DebugReporter::ENGINE);
					if ( ProcessReport() )
					{
						DebugReporter::Instance().DisplayMessage("CReporter::Run - Successfully processed Report", DebugReporter::ENGINE);
						return S_OK;
					}
					else
					{
						DebugReporter::Instance().DisplayMessage("CReporter::Run - Failed to process Report", DebugReporter::ENGINE);
					}
				}
				else
				{
					DebugReporter::Instance().DisplayMessage("CReporter::Run - No Reports to run", DebugReporter::ENGINE);
				}
			}
			else
			{
				DebugReporter::Instance().DisplayMessage("CReporter::Run - Reporter Server Task Disabled", DebugReporter::ENGINE);
			}
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CReporter::Run - Scheduled Reports Disabled", DebugReporter::ENGINE);
		}
		m_LastScheduledReportCheck = TickCount;
	}
	
	return S_FALSE;	
}

void CReporter::CheckEnabled( void )
{
	TCHAR szDataValue[SERVERPARAMETERS_DATAVALUE_LENGTH];
	CEMSString sValue;
	long szDataValueLen;
	int ID = 47;

	szDataValue[0] = _T('\0');

	m_query.Initialize();
	BINDPARAM_LONG( m_query, ID );
	BINDCOL_TCHAR( m_query, szDataValue );
	m_query.Execute( _T("SELECT DataValue FROM ServerParameters WHERE ServerParameterID=?"));
	m_query.Fetch();

	sValue = szDataValue;
	m_reportingEnabled = (_ttoi( sValue.c_str() ) == 0 ) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Get Report Parameters
// 
////////////////////////////////////////////////////////////////////////////////
bool CReporter::GetParameters( void )
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	int nYear = Now.year;
	int nMonth = Now.month;
	int nDay = Now.day;
	CEMSString sTempPath;
	long sTempPathLen = sizeof(sTempPath);
	long retVal;
		
	//Get the report folder from the registry
	retVal = GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_REPORT_PATH_VALUE, sTempPath );

	if ( retVal != 0 )
	{
		Log( E_Reporter, L"Unable to get report path from MailFlow registry" );	
		return false;
	}

	if( sTempPathLen > 0 ) 
	{
		// Remove all backslashes at end
		while( sTempPath.at( sTempPath.length() - 1 ) == _T('\\') )
		{
			sTempPath.resize( sTempPath.length() - 1);
		}
		m_sReportFolder.Format( _T("%s\\%d\\%d\\%d\\"), sTempPath.c_str(), nYear, nMonth, nDay );
		m_sReportPath.Format( _T("%d\\%d\\%d"), nYear, nMonth, nDay );
	}

	// Get the system email address
	m_query.Initialize();
	int nID = EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS;
	BINDPARAM_LONG( m_query, nID );
	BINDCOL_TCHAR( m_query, m_FromEmailAddress );
	
	m_query.Execute( _T("SELECT DataValue FROM ServerParameters ")
					 _T("WHERE ServerParameterID=?") );
	m_query.Fetch();

	//Get the outbound attachment folder from the registry
	retVal = GetRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("OutboundAttachFolder"), sTempPath );
	
	if ( retVal != 0 )
	{
		Log( E_Reporter, L"Unable to get outbound attach path from MailFlow registry" );	
		return false;
	}

	if( sTempPathLen > 0 ) 
	{
		// Remove all backslashes at end
		while( sTempPath.at( sTempPath.length() - 1 ) == _T('\\') )
		{
			sTempPath.resize( sTempPath.length() - 1);
		}
		m_sOutboundAttachFolder.Format( _T("%s\\%d\\%d\\%d\\"), sTempPath.c_str(), nYear, nMonth, nDay );		
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Get Report To Run
// 
////////////////////////////////////////////////////////////////////////////////
bool CReporter::GetReportToRun( void )

{
	TIMESTAMP_STRUCT Now;
	HRESULT hresult;
	wchar_t szDebug[1024];
	
	GetTimeStamp( Now );
	
	try
	{
				
		m_query.Reset( true );

		BINDPARAM_TIME_NOLEN( m_query, Now );
		BINDCOL_LONG( m_query, m_ReportTypeID );
		BINDCOL_LONG( m_query, m_ReportObjectID );
		BINDCOL_LONG( m_query, sr.m_ScheduledReportID );
		BINDCOL_TCHAR( m_query, sr.m_Description );
		BINDCOL_LONG( m_query, sr.m_ReportID );
		BINDCOL_BIT( m_query, sr.m_IsCustom );
		BINDCOL_LONG( m_query, sr.m_OwnerID );
		BINDCOL_LONG( m_query, sr.m_CreatedByID );
		BINDCOL_TIME( m_query, sr.m_DateCreated );
		BINDCOL_LONG( m_query, sr.m_EditedByID );
		BINDCOL_TIME( m_query, sr.m_DateEdited );
		BINDCOL_TIME( m_query, sr.m_LastRunTime );
		BINDCOL_LONG( m_query, sr.m_LastResultCode );
		BINDCOL_TIME( m_query, sr.m_NextRunTime );
		BINDCOL_LONG( m_query, sr.m_RunFreq );
		BINDCOL_LONG( m_query, sr.m_RunOn );
		BINDCOL_BIT( m_query, sr.m_RunSun );
		BINDCOL_BIT( m_query, sr.m_RunMon );
		BINDCOL_BIT( m_query, sr.m_RunTue );
		BINDCOL_BIT( m_query, sr.m_RunWed );
		BINDCOL_BIT( m_query, sr.m_RunThur );
		BINDCOL_BIT( m_query, sr.m_RunFri );
		BINDCOL_BIT( m_query, sr.m_RunSat );
		BINDCOL_LONG( m_query, sr.m_RunAtHour );
		BINDCOL_LONG( m_query, sr.m_RunAtMin );
		BINDCOL_LONG( m_query, sr.m_SumPeriod );
		BINDCOL_LONG( m_query, sr.m_MaxResults );
		BINDCOL_BIT( m_query, sr.m_SendAlertToOwner );
		BINDCOL_BIT( m_query, sr.m_SendResultToEmail );
		BINDCOL_TCHAR( m_query, sr.m_ResultEmailTo );
		BINDCOL_BIT( m_query, sr.m_IncludeResultFile );
		BINDCOL_BIT( m_query, sr.m_SaveResultToFile );
		BINDCOL_LONG( m_query, sr.m_KeepNumResultFile );
		BINDCOL_BIT( m_query, sr.m_AllowConsolidation );
		BINDCOL_LONG( m_query, sr.m_TargetID );
		BINDCOL_BIT( m_query, sr.m_Flag1 );
		BINDCOL_BIT( m_query, sr.m_Flag2 );
		BINDCOL_LONG( m_query, sr.m_TicketStateID );	
		m_query.Execute( L"SELECT TOP 1 ReportStandard.ReportTypeID,ReportStandard.ReportObjectID,ScheduledReportID,ReportScheduled.Description,ReportID,IsCustom,OwnerID,CreatedBy,DateCreated,EditedBy,DateEdited,LastRunTime,LastResultCode,NextRunTime,RunFreq,RunOn,RunSun,RunMon,RunTue,RunWed,RunThur,RunFri,RunSat,RunAtHour,RunAtMin,SumPeriod,MaxResults,SendAlertToOwner,SendResultToEmail,ResultEmailTo,IncludeResultFile,SaveResultToFile,KeepNumResultFile,AllowConsolidation,TargetID,Flag1,Flag2,TicketStateID "
					L"FROM ReportScheduled INNER JOIN ReportStandard ON ReportScheduled.ReportID=ReportStandard.StandardReportID "
					L"WHERE ReportScheduled.IsEnabled=1 AND ReportScheduled.NextRunTime < ? "
					L"Order By ReportScheduled.NextRunTime ");
					  
		if( (hresult = m_query.Fetch()) == S_OK )
		{
			return true;
		}
	}
	catch( ODBCError_t err )
	{
		swprintf( szDebug, L"Caught ODBCError: %d, %d, %s\n", 
		err.nErrorCode, err.nNativeErrorCode, err.szErrMsg );
	}
	
	return false;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Process Report
// 
////////////////////////////////////////////////////////////////////////////////
bool CReporter::ProcessReport( void )
{
	//Load Reporter Parameters
	if ( !GetParameters() )
	{
		return false;
	}

	//Create the ReportResult Record
	m_query.Initialize();
	BINDPARAM_LONG( m_query, sr.m_ScheduledReportID );
	m_query.Execute( L"INSERT INTO ReportResults (ScheduledReportID) VALUES (?)" );
	m_ReportResultID = m_query.GetLastInsertedID();
	
	//Run the report
	switch ( m_ReportTypeID )
	{
	case 1:
		InboundMessageReport();
		break;
	case 2:
		OutboundMessageReport();
		break;
	case 3:
		OpenTicketAgeReport();
		break;
	case 4:
		TicketDistributionReport();
		break;
	case 5:
		StandardResponsesReport();
		break;
	case 6:
		AverageResponseTimeReport();
		break;
	case 7:
		AverageTimeToResolveReport();
		break;
	case 8:
		if ( m_ReportObjectID == 10 )
			m_bHour = true;
		
		BusiestTimeReport();
		break;
	case 10:
		TicketDispositionReport();
		break;	
	}

	if ( sr.m_SendAlertToOwner )
	{
		AlertOwner();
	}
	if ( sr.m_SendResultToEmail )
	{
		EmailReport();
	}
	
	UpdateReport();

	return true;
	
}

void CReporter::GetWhen2( CEMSString& sWhen )
{
	TCHAR* szMonths[] = { _T("January"), _T("February"), _T("March"), _T("April"), _T("May"),
		                  _T("June"), _T("July"), _T("August"), _T("September"), _T("October"),
						  _T("November"), _T("December") };

	TCHAR* szDays[] = { _T("Sunday"), _T("Monday"), _T("Tuesday"), _T("Wednesday"), _T("Thursday"),
		                _T("Friday"), _T("Saturday") };

	switch( sr.m_SumPeriod )
	{
	case 0:
		sWhen.Format( _T("%d/%d/%d %2d:00"), m_nMonth, m_nDay, m_nYear, m_nHour );
		break;

	case 1:	// Day
		sWhen.Format( _T("%d/%d/%d"), m_nMonth, m_nDay, m_nYear );
		break;

	case 2:	// Week
		sWhen.Format( _T("Week %d of %d"), m_nWeek, m_nYear );
		break;

	case 3:	// Month
		sWhen.Format( _T("%s %d"), szMonths[m_nMonth-1], m_nYear );
		break;

	case 4: // Year
		sWhen.Format( _T("%d"), m_nYear );
		break;
	}
}
void CReporter::GetWhen( CEMSString& sWhen )
{
	switch( sr.m_SumPeriod )
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
	}
}


void CReporter::GetWhenSelect( CEMSString& sDatePart, TCHAR* szColumn, bool bBind )
{
	switch( sr.m_SumPeriod )
	{
	case 0:	// Hour
		sDatePart.Format( _T("DATEPART(year,%s),DATEPART(month,%s),DATEPART(day,%s),DATEPART(hour,%s)"), 
			              szColumn, szColumn, szColumn, szColumn );

		if( bBind )
		{
			BINDCOL_LONG_NOLEN( m_query, m_nYear );
			BINDCOL_LONG_NOLEN( m_query, m_nMonth );
			BINDCOL_LONG_NOLEN( m_query, m_nDay );
			BINDCOL_LONG_NOLEN( m_query, m_nHour );
		}
		break;
	
	case 1:	// Day
		sDatePart.Format( _T("DATEPART(year,%s),DATEPART(month,%s),DATEPART(day,%s)") , 
							  szColumn, szColumn, szColumn );
		
		if( bBind )
		{
			BINDCOL_LONG_NOLEN( m_query, m_nYear );
			BINDCOL_LONG_NOLEN( m_query, m_nMonth );
			BINDCOL_LONG_NOLEN( m_query, m_nDay );
		}
		break;

	case 2: // Week
		sDatePart.Format( _T("DATEPART(year,%s),DATEPART(week,%s)"), 
			              szColumn, szColumn );
		if( bBind )
		{
			BINDCOL_LONG_NOLEN( m_query, m_nYear );
			BINDCOL_LONG_NOLEN( m_query, m_nWeek );
		}
		break;

	case 3: // Month
		sDatePart.Format( _T("DATEPART(year,%s),DATEPART(month,%s)"), 
			              szColumn, szColumn );
		if( bBind )
		{
			BINDCOL_LONG_NOLEN( m_query, m_nYear );
			BINDCOL_LONG_NOLEN( m_query, m_nMonth );
		}
		break;

	case 4: // Year
		sDatePart.Format( _T("DATEPART(year,%s)"), 
			              szColumn );
		if( bBind )
		{
			BINDCOL_LONG_NOLEN( m_query, m_nYear );
		}
		break;
	}
}

void CReporter::SetStartEndDate( void )
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	int nDayOfWeek = GetDayOfWeek( Now.month, Now.day, Now.year );
	int xTime = 0;
	time_t now;
	time( &now ); 
	TIMESTAMP_STRUCT m_DateTime;
	
	switch( sr.m_RunFreq )
	{
	case 0:	// Day
		SecondsToTimeStamp(now - 86400, m_DateTime);
		m_StartDate = m_DateTime;
		m_EndDate = m_DateTime;
		break;
	case 1: // Week
		switch(nDayOfWeek)
		{
		case 0:
			SecondsToTimeStamp(now - (86400*7), m_DateTime);
			m_StartDate = m_DateTime;
			SecondsToTimeStamp(now - 86400, m_DateTime);
			m_EndDate = m_DateTime;
			break;
		case 1:
			SecondsToTimeStamp(now - (86400*8), m_DateTime);
			m_StartDate = m_DateTime;
			SecondsToTimeStamp(now - (86400*2), m_DateTime);
			m_EndDate = m_DateTime;
			break;
		case 2:
			SecondsToTimeStamp(now - (86400*9), m_DateTime);
			m_StartDate = m_DateTime;
			SecondsToTimeStamp(now - (86400*3), m_DateTime);
			m_EndDate = m_DateTime;
			break;
		case 3:
			SecondsToTimeStamp(now - (86400*10), m_DateTime);
			m_StartDate = m_DateTime;
			SecondsToTimeStamp(now - (86400*4), m_DateTime);
			m_EndDate = m_DateTime;
			break;
		case 4:
			SecondsToTimeStamp(now - (86400*11), m_DateTime);
			m_StartDate = m_DateTime;
			SecondsToTimeStamp(now - (86400*5), m_DateTime);
			m_EndDate = m_DateTime;
			break;
		case 5:
			SecondsToTimeStamp(now - (86400*12), m_DateTime);
			m_StartDate = m_DateTime;
			SecondsToTimeStamp(now - (86400*6), m_DateTime);
			m_EndDate = m_DateTime;
			break;
		case 6:
			SecondsToTimeStamp(now - (86400*13), m_DateTime);
			m_StartDate = m_DateTime;
			SecondsToTimeStamp(now - (86400*7), m_DateTime);
			m_EndDate = m_DateTime;
			break;
		}
		break;

	case 2: // Month
		SecondsToTimeStamp(now - 86400*14, m_DateTime);
		m_StartDate = m_DateTime;
		m_StartDate.day = 1;
		m_EndDate = m_DateTime;
		m_EndDate.day = GetLastDayOfMonth(m_DateTime.month);
		break;

	case 3: // Year
		SecondsToTimeStamp(now - 86400*14, m_DateTime);
		m_StartDate.year = m_DateTime.year;
		m_StartDate.day = 1;
		m_StartDate.month = 1;
		m_EndDate.year = m_DateTime.year;
		m_EndDate.day = 31;
		m_EndDate.month = 12;
		break;
	}

	m_StartDate.hour = 0;
	m_StartDate.minute = 0;
	m_StartDate.second = 0;
	m_EndDate.hour = 23;
	m_EndDate.minute = 59;
	m_EndDate.second = 59;
		
}
void CReporter::FormatTimeString( int nWhen, CEMSString& sWhen )
{
	switch( sr.m_RunFreq )
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

void CReporter::AddResultRows( void )
{
	for( rrrIter = m_rrr.begin(); rrrIter != m_rrr.end(); rrrIter++ )
	{
		m_query.Reset( true );
		BINDPARAM_LONG( m_query, m_ReportResultID );
		BINDPARAM_TCHAR( m_query, rrrIter->m_Col1 );
		BINDPARAM_TCHAR( m_query, rrrIter->m_Col2 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col3 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col4 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col5 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col6 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col7 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col8 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col9 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col10 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col11 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col12 );
		BINDPARAM_LONG( m_query, rrrIter->m_Col13 );
		m_query.Execute( L"INSERT INTO ReportResultRows (ReportResultID,Col1,Col2,Col3,Col4,Col5,Col6,Col7,Col8,Col9,Col10,Col11,Col12,Col13) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)" );
	}	
}
void CReporter::InboundMessageReport( void )
{
	CEMSString sQuery;
	CEMSString sFilter;
	CEMSString sDatePart;
	CEMSString sWhere;
	int nCount;

	if ( sr.m_TargetID > 0 )
	{ 
		switch (m_ReportObjectID) 
		{
		case 1:
			sFilter.Format( _T("AND I.OriginalTicketBoxID=%d"), sr.m_TargetID );
			break;
		case 2:
			sFilter.Format( _T("AND I.OriginalOwnerID=%d"), sr.m_TargetID );
			break;
		case 3:
			sFilter.Format( _T("AND I.OriginalTicketCategoryID=%d"), sr.m_TargetID );
			break;
		case 4:
			sFilter.Format( _T("AND I.RoutingRuleID=%d"), sr.m_TargetID );
			break;
		case 5:
			sFilter.Format( _T("AND I.MessageSourceID=%d"), sr.m_TargetID );
			break;		
		}
	}
	
	SetStartEndDate();
    
	m_query.Initialize();
	GetWhenSelect( sDatePart, _T("I.DateReceived"), true ); 
	BINDCOL_LONG_NOLEN( m_query, nCount );
	BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
	BINDPARAM_TIME_NOLEN( m_query, m_EndDate );

	sQuery.Format( _T("SELECT TOP %d %s, COUNT(*) ")
				   _T("FROM InboundMessages as I ")
				   _T("WHERE I.IsDeleted=0 AND I.DateReceived BETWEEN ? AND ? %s ")
				   _T("GROUP BY %s ORDER BY %s"), sr.m_MaxResults + 1, sDatePart.c_str(), sFilter.c_str(), sDatePart.c_str(), sDatePart.c_str() );

	m_query.Execute( sQuery.c_str() );

	// fetch the data and output the results
	CEMSString sDate, sDateEsc, sCount;
	CEMSString sLine;
	OpenCDF();
	OutputCDF( CEMSString( _T("Date, Number of Messages\r\n") ) );
	m_rrr.clear();
	int nRows = 0;
	for ( nRows=0; m_query.Fetch() == S_OK; nRows++ )
	{	
		if ( nRows == sr.m_MaxResults )
		{
			OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
			break;
		}
	
		GetWhen2( sDate );
		sDateEsc.assign( sDate );
		sDateEsc.EscapeJavascript();
		sCount.Format( _T("%d"), nCount );

		sLine.Format( L"\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str() );
				
		OutputCDF( sLine );
		_tcscpy( rrr.m_Col1, sDate.c_str() );
		_tcscpy( rrr.m_Col2, _T("") );
		rrr.m_Col3 = nCount;
		rrr.m_Col4 = 0;
		rrr.m_Col5 = 0;
		rrr.m_Col6 = 0;
		rrr.m_Col7 = 0;
		rrr.m_Col8 = 0;
		rrr.m_Col9 = 0;
		rrr.m_Col10 = 0;
		rrr.m_Col11 = 0;
		rrr.m_Col12 = 0;
		rrr.m_Col13 = 0;
		m_rrr.push_back( rrr );
		
	}
	AddResultRows();
	if ( nRows == 0 )
		OutputCDF( CEMSString( _T("No results returned\r\n") ) );
	
	CloseCDF();
}

void CReporter::OutboundMessageReport( void )
{
	CEMSString sQuery;
	CEMSString sDatePart;
	CEMSString sFilter;
	int nCount;

	if ( sr.m_TargetID > 0 )
	{ 
		switch (m_ReportObjectID) 
		{
		case 1:
			sFilter.Format( _T("AND O.OriginalTicketBoxID=%d"), sr.m_TargetID );
			break;
		case 2:
			sFilter.Format( _T("AND O.AgentID=%d"), sr.m_TargetID );
			break;
		case 3:
			sFilter.Format( _T("AND O.TicketCategoryID=%d"), sr.m_TargetID );
			break;			
		}
	}
	
	if ( !sr.m_Flag1 == 1 )
	{
		sFilter.append( _T(" AND O.AgentID>0"));
	}
	
	SetStartEndDate();
    
	m_query.Initialize();
	GetWhenSelect( sDatePart, _T("O.EmailDateTime"), true ); 
	BINDCOL_LONG_NOLEN( m_query, nCount );
	BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
	BINDPARAM_TIME_NOLEN( m_query, m_EndDate );

	sQuery.Format( _T("SELECT TOP %d %s, COUNT(*) ")
				   _T("FROM OutboundMessages as O ")
				   _T("WHERE O.IsDeleted=0 AND O.EmailDateTime BETWEEN ? AND ? %s ")
				   _T("GROUP BY %s ORDER BY %s"), sr.m_MaxResults + 1, sDatePart.c_str(), sFilter.c_str(), sDatePart.c_str(), sDatePart.c_str() );

	m_query.Execute( sQuery.c_str() );

	// fetch the data and output the results
	CEMSString sDate, sDateEsc, sCount;
	CEMSString sLine;
	m_rrr.clear();
	OpenCDF();
	OutputCDF( CEMSString( _T("Date, Number of Messages\r\n") ) );
	int nRows = 0;
	for ( nRows=0; m_query.Fetch() == S_OK; nRows++ )
	{	
		if ( nRows == sr.m_MaxResults )
		{
			OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
			
			break;
		}
	
		GetWhen2( sDate );
		sDateEsc.assign( sDate );
		sDateEsc.EscapeJavascript();
		sCount.Format( _T("%d"), nCount );

		sLine.Format( L"\"%s\",\"%s\"\r\n", sDate.c_str(), sCount.c_str() );
				
		OutputCDF( sLine );
		_tcscpy( rrr.m_Col1, sDate.c_str() );
		_tcscpy( rrr.m_Col2, _T("") );
		rrr.m_Col3 = nCount;
		rrr.m_Col4 = 0;
		rrr.m_Col5 = 0;
		rrr.m_Col6 = 0;
		rrr.m_Col7 = 0;
		rrr.m_Col8 = 0;
		rrr.m_Col9 = 0;
		rrr.m_Col10 = 0;
		rrr.m_Col11 = 0;
		rrr.m_Col12 = 0;
		rrr.m_Col13 = 0;
		m_rrr.push_back( rrr );
		
	}
	AddResultRows();
	if ( nRows == 0 )
		OutputCDF( CEMSString( _T("No results returned\r\n") ) );
	
	CloseCDF();	
}

void CReporter::OpenTicketAgeReport( void )
{
	CEMSString sWhen, sFilter, sString, sJoin, sWhere;
	int nCount, nWhen, isAdmin;

	//Check to see if report owner is an admin
	isAdmin = 0;
	m_query.Initialize();
	BINDPARAM_LONG( m_query, sr.m_OwnerID );
	BINDCOL_LONG_NOLEN( m_query, isAdmin );
	m_query.Execute( _T("SELECT AgentGroupingID FROM AgentGroupings WHERE GroupID=2 AND AgentID=?") );
	m_query.Fetch();

	if ( m_ReportObjectID == 1 && sr.m_TargetID > 0 )
	{
		sFilter.Format( _T("AND T.TicketBoxID=%d"), sr.m_TargetID );
	}
	else if ( m_ReportObjectID == 3 && sr.m_TargetID > 0 )
	{
		sFilter.Format( _T("AND T.TicketCategoryID=%d"), sr.m_TargetID );
	}
	else if ( sr.m_TargetID > 0 )
	{
		sFilter.Format( _T("AND T.OwnerID=%d"), sr.m_TargetID );
	}
	
	GetWhen( sWhen );
	m_query.Initialize();

	BINDCOL_LONG_NOLEN( m_query, nCount );
	BINDCOL_LONG_NOLEN( m_query, nWhen );
	if ( isAdmin < 1 )
	{
		sString.Format( _T("SELECT TOP %d COUNT(*),%s FROM Tickets as T ")
						_T("INNER JOIN AgentTicketCategories ATC ON ")
					    _T("T.TicketCategoryID=ATC.TicketCategoryID ")
					    _T("WHERE ATC.AgentID=%d AND OpenTimeStamp < getdate() AND TicketStateID>1 AND IsDeleted=0 %s ")
						_T("GROUP BY %s ORDER BY %s "), 
						sr.m_MaxResults + 1, sWhen.c_str(), sr.m_OwnerID, sFilter.c_str(), sWhen.c_str(), sWhen.c_str() );
	}
	else
	{
		sString.Format( _T("SELECT TOP %d COUNT(*),%s FROM Tickets as T ")
						_T("WHERE OpenTimeStamp < getdate() AND TicketStateID>1 AND IsDeleted=0 %s ")
						_T("GROUP BY %s ORDER BY %s "), 
						sr.m_MaxResults + 1, sWhen.c_str(), sFilter.c_str(), sWhen.c_str(), sWhen.c_str() );
	}
	
	m_query.Execute( sString.c_str() );
	m_rrr.clear();
	OpenCDF();
	OutputCDF( CEMSString( _T("Ticket Age, Number of Tickets\r\n") ) );
	int nRows = 0;
	for ( nRows=0; m_query.Fetch() == S_OK; nRows++ )
	{		
		if ( nRows == sr.m_MaxResults )
		{
			OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
			
			break;
		}

		FormatTimeString( nWhen, sString );
		
		// add line to CSV file
		CEMSString sLine;
		sLine.Format( L"\"%s\",\"%d\"\r\n", sString.c_str(), nCount );
		
		OutputCDF( sLine );
		_tcscpy( rrr.m_Col1, sString.c_str() );
		_tcscpy( rrr.m_Col2, _T("") );
		rrr.m_Col3 = nCount;
		rrr.m_Col4 = 0;
		rrr.m_Col5 = 0;
		rrr.m_Col6 = 0;
		rrr.m_Col7 = 0;
		rrr.m_Col8 = 0;
		rrr.m_Col9 = 0;
		rrr.m_Col10 = 0;
		rrr.m_Col11 = 0;
		rrr.m_Col12 = 0;
		rrr.m_Col13 = 0;
		m_rrr.push_back( rrr );
		
	}
	AddResultRows();
	if ( nRows == 0 )
		OutputCDF( CEMSString( _T("No results returned\r\n") ) );
	
	CloseCDF();	
}

void CReporter::TicketDistributionReport( void )
{
	CIDCounts IDCount;
	map<int,int> count_map;
	map<int,int>::iterator map_iter;
	list<CIDCounts> count_list;
	list<unsigned int>::iterator iter;
	int nID, isAdmin;
	long nIDLen;
	int m_nTotal = 0;
	CEMSString sString, sName, sCount, sPercent, sStateID, sQuery;

	//Check to see if report owner is an admin
	isAdmin = 0;
	m_query.Initialize();
	BINDPARAM_LONG( m_query, sr.m_OwnerID );
	BINDCOL_LONG_NOLEN( m_query, isAdmin );
	m_query.Execute( _T("SELECT AgentGroupingID FROM AgentGroupings WHERE GroupID=2 AND AgentID=?") );
	m_query.Fetch();
	
	if( m_ReportObjectID == 1 )
	{
		m_query.Initialize();
		
		BINDCOL_LONG_NOLEN( m_query, IDCount.m_nID );
		BINDCOL_LONG_NOLEN( m_query, IDCount.m_nCount );

		if ( sr.m_TicketStateID != -1 )
		{
			if ( sr.m_TicketStateID != 2 )
			{
				sStateID.Format( _T(" AND TicketStateID=%d"), sr.m_TicketStateID );
			}
			else
			{	
				sStateID = _T(" AND ((TicketStateID > 1 AND TicketStateID < 5) OR TicketStateID = 6) ");
			}
		}

		if(sr.m_TicketStateID != 5 && sr.m_TicketStateID != -1)
		{
			if ( isAdmin < 1 )
			{
				sQuery.Format( _T("SELECT TicketBoxID, COUNT(*) FROM TICKETS ")
							   _T("INNER JOIN AgentTicketCategories ATC ON ")
							   _T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
							   _T("WHERE ATC.AgentID=%d AND IsDeleted=0 %s GROUP BY TicketBoxID"), sStateID.c_str() );
			}
			else
			{
				sQuery.Format( _T("SELECT TicketBoxID, COUNT(*) FROM TICKETS ")
					_T("WHERE IsDeleted=0 %s GROUP BY TicketBoxID"), sStateID.c_str() );
			}
		}
		else
		{
			if(sr.m_TicketStateID != -1)
			{
				sQuery.Format( _T("SELECT TicketBoxID, COUNT(*) FROM TICKETS ")
							_T("WHERE IsDeleted=1 GROUP BY TicketBoxID"), sStateID.c_str() );
			}
			else
			{
				sStateID.Format( _T(" OR TicketStateID >= 0"), sr.m_TicketStateID );

				sQuery.Format( _T("SELECT TicketBoxID, COUNT(*) FROM TICKETS ")
						_T("WHERE IsDeleted=1 %s GROUP BY TicketBoxID"), sStateID.c_str() );
			}
		}

		SetStartEndDate();

		m_query.Execute( sQuery.c_str() );
		
		while ( m_query.Fetch() == S_OK )
		{
			count_map[IDCount.m_nID] = IDCount.m_nCount;
			m_nTotal += IDCount.m_nCount;
		}
		
		//Get TicketBoxID's that the Owner has at least Read rights to
		m_query.Reset( true );
		BINDCOL_LONG_NOLEN( m_query, nID );
		BINDCOL_TCHAR( m_query, m_nName );
		sQuery.Format( _T("Select TicketBoxes.TicketBoxID,TicketBoxes.Name From TicketBoxes ")
					   _T("WHERE TicketBoxes.TicketBoxID IN ")
					   _T("(SELECT DISTINCT TicketBoxes.TicketBoxID FROM TicketBoxes ")
					   _T("INNER JOIN Objects ON TicketBoxes.ObjectID=Objects.ObjectID ")
					   _T("INNER JOIN AccessControl ON Objects.ObjectID=AccessControl.ObjectID ")
					   _T("WHERE Objects.BuiltIn=0 AND Objects.UseDefaultRights=0 AND AccessControl.AccessLevel>1 ")
					   _T("AND (AccessControl.AgentID=%d OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=%d))) ")
					   _T("OR TicketBoxes.TicketBoxID IN ")
					   _T("(SELECT DISTINCT TicketBoxes.TicketBoxID FROM TicketBoxes ")
					   _T("INNER JOIN Objects ON TicketBoxes.ObjectID=Objects.ObjectID ")
					   _T("CROSS JOIN AccessControl WHERE Objects.UseDefaultRights = 1 ")
					   _T("AND ((SELECT MAX(AccessLevel) FROM AccessControl INNER JOIN Objects ON ")
					   _T("Objects.ObjectID=AccessControl.ObjectID WHERE Objects.ObjectTypeID=2 AND Objects.BuiltIn=1 ")
					   _T("AND Objects.ActualID=0 ")
					   _T("AND (AccessControl.AgentID=%d OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=%d)))>0)) "), sr.m_OwnerID, sr.m_OwnerID, sr.m_OwnerID, sr.m_OwnerID);
		
		m_query.Execute( sQuery.c_str() );
		m_rrr.clear();
		OpenCDF();
		OutputCDF( CEMSString( _T("TicketBox, Tickets, Percentage\r\n") ) );
		int nRows = 0;
		for ( nRows=0; m_query.Fetch() == S_OK; nRows++ )
		{		
			if ( nRows == sr.m_MaxResults )
			{
				OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
				
				break;
			}

			IDCount.m_nID = nID;
				
			if ( (map_iter = count_map.find(IDCount.m_nID)) != count_map.end() )
			{
				IDCount.m_nCount = map_iter->second;				
			}
			else
			{
				IDCount.m_nCount = 0;
			}
			
			if ( sr.m_Flag1 || IDCount.m_nCount > 0 )
			{
				sName.Format( _T("%s"), m_nName );
				sName.EscapeCSV();
				sCount.Format( _T("%d"), IDCount.m_nCount );
				sPercent.Format( _T("%.2f"), m_nTotal ? ((float) IDCount.m_nCount / (float) m_nTotal) * 100 : 0);
				sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), sName.c_str(), sCount.c_str(), sPercent.c_str() );
				OutputCDF( sString );				
				
				_tcscpy( rrr.m_Col1, sName.c_str() );
				_tcscpy( rrr.m_Col2, sPercent.c_str() );
				rrr.m_Col3 = IDCount.m_nCount;
				rrr.m_Col4 = 0;
				rrr.m_Col5 = 0;
				rrr.m_Col6 = 0;
				rrr.m_Col7 = 0;
				rrr.m_Col8 = 0;
				rrr.m_Col9 = 0;
				rrr.m_Col10 = 0;
				rrr.m_Col11 = 0;
				rrr.m_Col12 = 0;
				rrr.m_Col13 = 0;
				m_rrr.push_back( rrr );
				
			}
		}
		AddResultRows();		
		if ( nRows == 0 )
		{
			OutputCDF( CEMSString( _T("No results returned\r\n") ) );
		}

		CloseCDF();
	}
	else if( m_ReportObjectID == 3 )
	{
		m_query.Initialize();
	
		BINDCOL_LONG_NOLEN( m_query, IDCount.m_nID );
		BINDCOL_LONG_NOLEN( m_query, IDCount.m_nCount );

		if ( sr.m_TicketStateID != -1 )
		{
			if ( sr.m_TicketStateID != 2 )
			{
				sStateID.Format( _T(" AND TicketStateID=%d"), sr.m_TicketStateID );
			}
			else
			{	
				// if the state is "open" also display escalated, on-hold and re-opened items
				sStateID = _T(" AND ((TicketStateID > 1 AND TicketStateID < 5) OR TicketStateID = 6) ");
			}
		}

		if(sr.m_TicketStateID != 5 && sr.m_TicketStateID != -1)
		{
			if ( isAdmin < 1 )
			{
				sQuery.Format( _T("SELECT TicketCategoryID, COUNT(*) FROM TICKETS ")
							   _T("INNER JOIN AgentTicketCategories ATC ON ")
							   _T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
							   _T("WHERE ATC.AgentID=%d AND IsDeleted=0 %s GROUP BY TicketCategoryID"), sStateID.c_str() );
			}
			else
			{
				sQuery.Format( _T("SELECT TicketCategoryID, COUNT(*) FROM TICKETS ")
				_T("WHERE IsDeleted=0 %s GROUP BY TicketCategoryID"), sStateID.c_str() );
			}
		}
		else
		{
			if(sr.m_TicketStateID != -1)
			{
				if ( isAdmin < 1 )
				{
					sQuery.Format( _T("SELECT TicketCategoryID, COUNT(*) FROM TICKETS ")
								_T("INNER JOIN AgentTicketCategories ATC ON ")
								_T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
								_T("WHERE ATC.AgentID=%d AND IsDeleted=1 GROUP BY TicketCategoryID") );
				}
				else
				{
					sQuery.Format( _T("SELECT TicketCategoryID, COUNT(*) FROM TICKETS ")
							_T("WHERE IsDeleted=1 GROUP BY TicketCategoryID") );
				}
			}
			else
			{
				sStateID.Format( _T(" OR TicketStateID >= 0"), sr.m_TicketStateID );

				if ( isAdmin < 1 )
				{
					sQuery.Format( _T("SELECT TicketCategoryID, COUNT(*) FROM TICKETS ")
								_T("INNER JOIN AgentTicketCategories ATC ON ")
								_T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
								_T("WHERE ATC.AgentID=%d AND IsDeleted=1 %s GROUP BY TicketCategoryID"), sStateID.c_str() );
				}
				else
				{
					sQuery.Format( _T("SELECT TicketCategoryID, COUNT(*) FROM TICKETS ")
						_T("WHERE IsDeleted=1 %s GROUP BY TicketCategoryID"), sStateID.c_str() );
				}				
			}
		}

		m_query.Execute( sQuery.c_str() );
		
		// fetch the results into the map
		while ( m_query.Fetch() == S_OK )
		{
			count_map[IDCount.m_nID] = IDCount.m_nCount;
		}
		
		//Get TicketCategoryID's that the Owner has at least Read rights to
		m_query.Reset( true );
		BINDCOL_LONG_NOLEN( m_query, nID );
		BINDCOL_TCHAR( m_query, m_nName );
		sQuery.Format( _T("Select TicketCategories.TicketCategoryID,TicketCategories.Description From TicketCategories ")
					   _T("WHERE TicketCategories.TicketCategoryID IN ")
					   _T("(SELECT DISTINCT TicketCategories.TicketCategoryID FROM TicketCategories ")
					   _T("INNER JOIN Objects ON TicketCategories.ObjectID=Objects.ObjectID ")
					   _T("INNER JOIN AccessControl ON Objects.ObjectID=AccessControl.ObjectID ")
					   _T("WHERE Objects.BuiltIn=0 AND Objects.UseDefaultRights=0 AND AccessControl.AccessLevel>1 ")
					   _T("AND (AccessControl.AgentID=%d OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=%d))) ")
					   _T("OR TicketCategories.TicketCategoryID IN ")
					   _T("(SELECT DISTINCT TicketCategories.TicketCategoryID FROM TicketCategories ")
					   _T("INNER JOIN Objects ON TicketCategories.ObjectID=Objects.ObjectID ")
					   _T("CROSS JOIN AccessControl WHERE Objects.UseDefaultRights = 1 ")
					   _T("AND ((SELECT MAX(AccessLevel) FROM AccessControl INNER JOIN Objects ON ")
					   _T("Objects.ObjectID=AccessControl.ObjectID WHERE Objects.ObjectTypeID=6 AND Objects.BuiltIn=1 ")
					   _T("AND Objects.ActualID=0 ")
					   _T("AND (AccessControl.AgentID=%d OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=%d)))>0)) "), sr.m_OwnerID, sr.m_OwnerID, sr.m_OwnerID, sr.m_OwnerID);
		
		m_query.Execute( sQuery.c_str() );
		m_rrr.clear();
		OpenCDF();
		OutputCDF( CEMSString( _T("TicketCategory, Tickets, Percentage\r\n") ) );
		int nRows = 0;
		for ( nRows=0; m_query.Fetch() == S_OK; nRows++ )
		{		
			if ( nRows == sr.m_MaxResults )
			{
				OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
				
				break;
			}

			IDCount.m_nID = nID;
						
			if ( IDCount.m_nID != 0 || ( sr.m_Flag2 && IDCount.m_nID == 0 ) )
			{
			
				if ( (map_iter = count_map.find(IDCount.m_nID)) != count_map.end() )
				{
					IDCount.m_nCount = map_iter->second;
					m_nTotal += IDCount.m_nCount;
				}
				else
				{
					IDCount.m_nCount = 0;
				}
							
				if ( sr.m_Flag1 || IDCount.m_nCount > 0 )
				sName.Format( _T("%s"), m_nName );
				sName.EscapeCSV();
				sCount.Format( _T("%d"), IDCount.m_nCount );
				sPercent.Format( _T("%.2f"), m_nTotal ? ((float) IDCount.m_nCount / (float) m_nTotal) * 100 : 0);
				sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), sName.c_str(), sCount.c_str(), sPercent.c_str() );
				OutputCDF( sString );
				_tcscpy( rrr.m_Col1,sName.c_str() );
				_tcscpy( rrr.m_Col2,sPercent.c_str() );
				rrr.m_Col3 = IDCount.m_nCount;
				rrr.m_Col4 = 0;
				rrr.m_Col5 = 0;
				rrr.m_Col6 = 0;
				rrr.m_Col7 = 0;
				rrr.m_Col8 = 0;
				rrr.m_Col9 = 0;
				rrr.m_Col10 = 0;
				rrr.m_Col11 = 0;
				rrr.m_Col12 = 0;
				rrr.m_Col13 = 0;
				m_rrr.push_back( rrr );
				
			}
		}
		AddResultRows();		
		if ( nRows == 0 )
		{
			OutputCDF( CEMSString( _T("No results returned\r\n") ) );
		}

		CloseCDF();
	}
	else
	{
		m_query.Initialize();
	
		BINDCOL_LONG_NOLEN( m_query, IDCount.m_nID );
		BINDCOL_LONG_NOLEN( m_query, IDCount.m_nCount );
		
		if ( sr.m_TicketStateID != -1 )
			sStateID.Format( _T(" AND TicketStateID=%d"), sr.m_TicketStateID );
		
		if(sr.m_TicketStateID != 5 && sr.m_TicketStateID != -1)
		{
			if ( isAdmin < 1 )
			{
				sQuery.Format( _T("SELECT OwnerID, COUNT(*) FROM TICKETS ")
							_T("INNER JOIN AgentTicketCategories ATC ON ")
							_T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
							_T("WHERE ATC.AgentID=%d AND IsDeleted=0 %s GROUP BY OwnerID"), sStateID.c_str() );
			}
			else
			{
				sQuery.Format( _T("SELECT OwnerID, COUNT(*) FROM TICKETS ")
						_T("WHERE IsDeleted=0 %s GROUP BY OwnerID"), sStateID.c_str() );
			}
		}
		else
		{
			if(sr.m_TicketStateID != -1)
			{
				if ( isAdmin < 1 )
				{
					sQuery.Format( _T("SELECT OwnerID, COUNT(*) FROM TICKETS ")
								_T("INNER JOIN AgentTicketCategories ATC ON ")
								_T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
								_T("WHERE ATC.AgentID=%d AND IsDeleted=1 GROUP BY OwnerID") );
				}
				else
				{
					sQuery.Format( _T("SELECT OwnerID, COUNT(*) FROM TICKETS ")
							_T("WHERE IsDeleted=1 GROUP BY OwnerID") );
				}
				
			}
			else
			{
				sStateID.Format( _T(" OR TicketStateID >= 0"), sr.m_TicketStateID );

				if ( isAdmin < 1 )
				{
					sQuery.Format( _T("SELECT OwnerID, COUNT(*) FROM TICKETS ")
								_T("INNER JOIN AgentTicketCategories ATC ON ")
								_T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
								_T("WHERE ATC.AgentID=%d AND IsDeleted=1 %s GROUP BY OwnerID"), sStateID.c_str() );;
				}
				else
				{
					sQuery.Format( _T("SELECT OwnerID, COUNT(*) FROM TICKETS ")
						_T("WHERE IsDeleted=1 %s GROUP BY OwnerID"), sStateID.c_str() );
				}
				
			}
		}
		
		m_query.Execute( sQuery.c_str() );
		
		// fetch the results into the map
		while ( m_query.Fetch() == S_OK )
		{
			count_map[IDCount.m_nID] = IDCount.m_nCount;
		}
		
		//Get AgentID's that the Owner has at least Read rights to
		m_query.Reset( true );
		BINDCOL_LONG_NOLEN( m_query, nID );
		BINDCOL_TCHAR( m_query, m_nName );
		sQuery.Format( _T("Select Agents.AgentID,Agents.Name From Agents ")
					   _T("WHERE Agents.AgentID IN ")
					   _T("(SELECT DISTINCT Agents.AgentID FROM Agents ")
					   _T("INNER JOIN Objects ON Agents.ObjectID=Objects.ObjectID ")
					   _T("INNER JOIN AccessControl ON Objects.ObjectID=AccessControl.ObjectID ")
					   _T("WHERE Objects.BuiltIn=0 AND Objects.UseDefaultRights=0 AND AccessControl.AccessLevel>1 ")
					   _T("AND (AccessControl.AgentID=%d OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=%d))) ")
					   _T("OR Agents.AgentID IN ")
					   _T("(SELECT DISTINCT Agents.AgentID FROM Agents ")
					   _T("INNER JOIN Objects ON Agents.ObjectID=Objects.ObjectID ")
					   _T("CROSS JOIN AccessControl WHERE Objects.UseDefaultRights = 1 ")
					   _T("AND ((SELECT MAX(AccessLevel) FROM AccessControl INNER JOIN Objects ON ")
					   _T("Objects.ObjectID=AccessControl.ObjectID WHERE Objects.ObjectTypeID=1 AND Objects.BuiltIn=1 ")
					   _T("AND Objects.ActualID=0 ")
					   _T("AND (AccessControl.AgentID=%d OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=%d)))>0)) "), sr.m_OwnerID, sr.m_OwnerID, sr.m_OwnerID, sr.m_OwnerID);
		
		m_query.Execute( sQuery.c_str() );
		m_rrr.clear();
		OpenCDF();
		OutputCDF( CEMSString( _T("Agent, Tickets, Percentage\r\n") ) );
		int nRows = 0;
		for ( nRows=0; m_query.Fetch() == S_OK; nRows++ )
		{		
			if ( nRows == sr.m_MaxResults )
			{
				OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
				
				break;
			}

			IDCount.m_nID = nID;
			
			if ( (map_iter = count_map.find(IDCount.m_nID)) != count_map.end() )
			{
				IDCount.m_nCount = map_iter->second;
				m_nTotal += IDCount.m_nCount;
			}
			else
			{
				IDCount.m_nCount = 0;
			}
			
			if ( sr.m_Flag1 || IDCount.m_nCount > 0 )
				sName.Format( _T("%s"), m_nName );
				sName.EscapeCSV();
				sCount.Format( _T("%d"), IDCount.m_nCount );
				sPercent.Format( _T("%.2f"), m_nTotal ? ((float) IDCount.m_nCount / (float) m_nTotal) * 100 : 0);
				sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), sName.c_str(), sCount.c_str(), sPercent.c_str() );
				OutputCDF( sString );
				_tcscpy( rrr.m_Col1,sName.c_str() );
				_tcscpy( rrr.m_Col2,sPercent.c_str() );
				rrr.m_Col3 = IDCount.m_nCount;
				rrr.m_Col4 = 0;
				rrr.m_Col5 = 0;
				rrr.m_Col6 = 0;
				rrr.m_Col7 = 0;
				rrr.m_Col8 = 0;
				rrr.m_Col9 = 0;
				rrr.m_Col10 = 0;
				rrr.m_Col11 = 0;
				rrr.m_Col12 = 0;
				rrr.m_Col13 = 0;
				m_rrr.push_back( rrr );
				
			
		}
			 
		if ( sr.m_Flag2 && (map_iter = count_map.find(0)) != count_map.end() )
		{
			IDCount.m_nID = 0;
			IDCount.m_nCount = map_iter->second;
			m_nTotal += IDCount.m_nCount;
			sName.Format( _T("%s"), m_nName );
			sName.EscapeCSV();
			sCount.Format( _T("%d"), IDCount.m_nCount );
			sPercent.Format( _T("%.2f"), m_nTotal ? ((float) IDCount.m_nCount / (float) m_nTotal) * 100 : 0);
			sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), sName.c_str(), sCount.c_str(), sPercent.c_str() );
			OutputCDF( sString );
			_tcscpy( rrr.m_Col1,sName.c_str() );
			_tcscpy( rrr.m_Col2,sPercent.c_str() );
			rrr.m_Col3 = IDCount.m_nCount;
			rrr.m_Col4 = 0;
			rrr.m_Col5 = 0;
			rrr.m_Col6 = 0;
			rrr.m_Col7 = 0;
			rrr.m_Col8 = 0;
			rrr.m_Col9 = 0;
			rrr.m_Col10 = 0;
			rrr.m_Col11 = 0;
			rrr.m_Col12 = 0;
			rrr.m_Col13 = 0;
			m_rrr.push_back( rrr );
			
		}
		if ( nRows == 0 )
		{
			OutputCDF( CEMSString( _T("No results returned\r\n") ) );
		}
		AddResultRows();
		CloseCDF();
	}	
}

void CReporter::StandardResponsesReport( void )
{
	if ( m_ReportObjectID == 7 )
	{
		//SR by Usage
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
		int m_nTotal;
		long m_nTotalLen;

		if ( sr.m_TargetID > -1 )
			sString.Format( _T("AND S.StdResponseCatID=%d "), sr.m_TargetID );
		
		if ( !sr.m_Flag2 )
			sString += _T("AND U.AgentID != 0");

		SetStartEndDate();

		// query for the total usage
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, m_nTotal );
		BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
		BINDPARAM_TIME_NOLEN( m_query, m_EndDate );

		sSQL.Format( _T("SELECT COUNT(*) FROM StandardResponseUsage as U ") 
					_T("INNER JOIN StandardResponses as S on S.StandardResponseID = U.StandardResponseID ") 
					_T("WHERE U.DateUsed BETWEEN ? AND ? %s"), sString.c_str() );
		
		m_query.Execute( sSQL.c_str() );
		m_query.Fetch();

		// query for the usage by response
		m_query.Initialize();
		BINDCOL_TCHAR_NOLEN( m_query, szName );
		BINDCOL_LONG_NOLEN( m_query, nCategoryID );
		BINDCOL_LONG_NOLEN( m_query, nCount );
		BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
		BINDPARAM_TIME_NOLEN( m_query, m_EndDate );

		if ( ! sr.m_Flag1 )
		{
			sSQL.Format( _T("SELECT S.Subject, StdResponseCatID, COUNT(*) FROM StandardResponseUsage as U ") 
						_T("INNER JOIN StandardResponses as S on S.StandardResponseID = U.StandardResponseID ") 
						_T("WHERE U.DateUsed BETWEEN ? AND ? %s ")
						_T("GROUP BY U.StandardResponseID, S.Subject, StdResponseCatID ORDER BY COUNT(*) DESC"), sString.c_str() );
		}
		else
		{
			CEMSString sCategory;
			
			if ( sr.m_TargetID > -1 )
				sCategory.Format( _T("WHERE S.StdResponseCatID=%d"), sr.m_TargetID );
			
			sString = _T("AND U.AgentID != 0");
			
			sSQL.Format( _T("SELECT Subject,StdResponseCatID, (SELECT Count(*) FROM StandardResponseUsage as U ")
						_T("WHERE StandardResponseID = S.StandardResponseID AND ")
				 		_T("DateUsed BETWEEN ? AND ? %s) ")
				  		_T("FROM StandardResponses as S %s ORDER BY 3 DESC"), sString.c_str(), sCategory.c_str() );
		}


		m_query.Execute( sSQL.c_str() );
		CEMSString sHeader;

		if ( sr.m_TargetID == -1 )
		{
			sHeader.Format( _T("\"%s\",\"%s\",\"%s\",\"%s\"\r\n"), 
				_T("Standard Response"), _T("Times Used"), _T("Percentage"), _T("Category") );
		}
		else
		{
			sHeader.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), 
				_T("Standard Response"), _T("Times Used"), _T("Percentage") );
		}
		
		OpenCDF();
		OutputCDF( sHeader );
		m_rrr.clear();
		// fetch the results and generate XML
		while ( m_query.Fetch() == S_OK )
		{
			sName = szName;
			sCount.Format( _T("%d"), nCount );
			sPercent.Format( _T("%.2f"), m_nTotal ? ((float) nCount / (float) m_nTotal) * 100 : 0);
			GetStdCategoryName( nCategoryID, sCategory );

			sName.EscapeCSV();
			sCategory.EscapeCSV();
			nPercentage = m_nTotal ? ((long) nCount / (long) m_nTotal) * 100 : 0;
			if ( sr.m_TargetID == -1 )
			{
				sString.Format( L"\"%s\",\"%s\",\"%s\",\"%s\"\r\n", sName.c_str(), sCount.c_str(), sPercent.c_str(), sCategory.c_str() );
				_tcscpy( rrr.m_Col1,sName.c_str() );
				_tcscpy( rrr.m_Col2,sCategory.c_str() );
				rrr.m_Col3 = nCount;
				rrr.m_Col4 = nPercentage;
				rrr.m_Col5 = 0;
				rrr.m_Col6 = 0;
				rrr.m_Col7 = 0;
				rrr.m_Col8 = 0;
				rrr.m_Col9 = 0;
				rrr.m_Col10 = 0;
				rrr.m_Col11 = 0;
				rrr.m_Col12 = 0;
				rrr.m_Col13 = 0;
				m_rrr.push_back( rrr );				
			}
			else
			{
				sString.Format( L"\"%s\",\"%s\",\"%s\"\r\n", sName.c_str(), sCount.c_str(), sPercent.c_str() );
				_tcscpy( rrr.m_Col1,sName.c_str() );
				_tcscpy( rrr.m_Col2,_T("") );
				rrr.m_Col3 = nCount;
				rrr.m_Col4 = nPercentage;
				rrr.m_Col5 = 0;
				m_rrr.push_back( rrr );					
			}
			
			OutputCDF( sString );			
		}
		AddResultRows();
		if ( i == 0 )
		{
			sString.Format( _T("No results returned\r\n") );
			OutputCDF( sString );
		}

		CloseCDF();

	}
	else if ( m_ReportObjectID == 8 )
	{
		//SR by Category
		CIDCounts IDCount;
		list<CIDCounts> m_list;
		int m_nTotal = 0;

		m_query.Initialize();
		
		BINDCOL_LONG_NOLEN( m_query, IDCount.m_nID );
		BINDCOL_LONG_NOLEN( m_query, IDCount.m_nCount );

		m_query.Execute( _T("SELECT StdResponseCatID, (SELECT COUNT(*) FROM StandardResponses ")
							_T("WHERE StdResponseCatID=StdResponseCategories.StdResponseCatID AND ")
							_T("IsApproved=1 AND IsDeleted=0)" )
							_T("FROM StdResponseCategories ORDER BY CategoryName") );
		
		while ( m_query.Fetch() == S_OK )
		{
			if ( IDCount.m_nCount > 0 || sr.m_Flag2 )
			{
				IDCount.m_nID != 1 ? m_list.push_back( IDCount ) : m_list.push_front( IDCount );
				m_nTotal += IDCount.m_nCount;
			}
		}
		
		// include the uncategorized responses
		if ( sr.m_Flag1 )
		{
			m_query.Reset(false);
			m_query.Execute( _T("SELECT 0, COUNT(*) FROM StandardResponses ")
								_T("WHERE StdResponseCatID=0 AND IsApproved=1 AND IsDeleted=0") );
			
			m_query.Fetch();
			
			if ( IDCount.m_nCount > 0 || sr.m_Flag2 )
			{
				m_nTotal += IDCount.m_nCount;
				m_list.push_front( IDCount );
			}
		}
		
		OpenCDF();
		CEMSString sString;
		sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), _T("Category") , _T("Responses"), _T("Percentage") );
		OutputCDF( sString );
		m_rrr.clear();
		// add rows
		list<CIDCounts>::iterator iter;
		int i=0;

		for ( iter = m_list.begin(); iter != m_list.end(); iter++ )
		{
			CEMSString sName( _T("Unknown") );
			CEMSString sCount, sPercent;

			GetStdCategoryName( iter->m_nID, sName );
			sCount.Format( _T("%d"), iter->m_nCount );
			sPercent.Format( _T("%.2f"), m_nTotal ? ((float) iter->m_nCount / (float) m_nTotal) * 100 : 0);
			
			sName.EscapeCSV();

			CEMSString sString;
			sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"),  sName.c_str(), sCount.c_str(), sPercent.c_str() );
			OutputCDF( sString );
			nPercentage = m_nTotal ? ((long) iter->m_nCount / (long) m_nTotal) * 100 : 0;
			_tcscpy( rrr.m_Col1, sName.c_str() );
			_tcscpy( rrr.m_Col2, _T("") );
			rrr.m_Col3 = iter->m_nCount;
			rrr.m_Col4 = nPercentage;
			rrr.m_Col5 = 0;
			rrr.m_Col6 = 0;
			rrr.m_Col7 = 0;
			rrr.m_Col8 = 0;
			rrr.m_Col9 = 0;
			rrr.m_Col10 = 0;
			rrr.m_Col11 = 0;
			rrr.m_Col12 = 0;
			rrr.m_Col13 = 0;
			m_rrr.push_back( rrr );				
		}
		
		if ( i == 0 )
		{
			CEMSString sLine;
			sLine.Format( _T("No results returned\r\n") );
			OutputCDF( sLine );
		}
		AddResultRows();
		CloseCDF();
	}
	else if ( m_ReportObjectID == 9 )
	{
		//SR by Author
		CEMSString sQuery;
		CIDCounts IDCount;
		list<CIDCounts> m_list;
		int nID;
		long nIDLen;
		int m_nTotal = 0;
	
		map<int,int> count_map;
		map<int,int>::iterator map_iter;
				
		m_query.Initialize();
		
		BINDCOL_LONG_NOLEN( m_query, IDCount.m_nID );
		BINDCOL_LONG_NOLEN( m_query, IDCount.m_nCount );
		
		m_query.Execute( _T("SELECT AgentID, COUNT(*) FROM StandardResponses GROUP BY AgentID") );
		
		// fetch the results into the map
		while ( m_query.Fetch() == S_OK )
		{
			count_map[IDCount.m_nID] = IDCount.m_nCount;
		}

		//Get AgentID's that the Owner has at least Read rights to
		m_query.Reset( true );
		BINDCOL_LONG_NOLEN( m_query, nID );
		BINDCOL_TCHAR( m_query, m_nName );
		sQuery.Format( _T("Select Agents.AgentID,Agents.Name From Agents ")
					   _T("WHERE Agents.AgentID IN ")
					   _T("(SELECT DISTINCT Agents.AgentID FROM Agents ")
					   _T("INNER JOIN Objects ON Agents.ObjectID=Objects.ObjectID ")
					   _T("INNER JOIN AccessControl ON Objects.ObjectID=AccessControl.ObjectID ")
					   _T("WHERE Objects.BuiltIn=0 AND Objects.UseDefaultRights=0 AND AccessControl.AccessLevel>1 ")
					   _T("AND (AccessControl.AgentID=%d OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=%d))) ")
					   _T("OR Agents.AgentID IN ")
					   _T("(SELECT DISTINCT Agents.AgentID FROM Agents ")
					   _T("INNER JOIN Objects ON Agents.ObjectID=Objects.ObjectID ")
					   _T("CROSS JOIN AccessControl WHERE Objects.UseDefaultRights = 1 ")
					   _T("AND ((SELECT MAX(AccessLevel) FROM AccessControl INNER JOIN Objects ON ")
					   _T("Objects.ObjectID=AccessControl.ObjectID WHERE Objects.ObjectTypeID=1 AND Objects.BuiltIn=1 ")
					   _T("AND Objects.ActualID=0 ")
					   _T("AND (AccessControl.AgentID=%d OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=%d)))>1)) "), sr.m_OwnerID, sr.m_OwnerID, sr.m_OwnerID, sr.m_OwnerID);
		
		m_query.Execute( sQuery.c_str() );
		
		for ( int nRows=0; m_query.Fetch() == S_OK; nRows++ )
		{		
			if ( nRows == sr.m_MaxResults )
			{
				OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
				
				break;
			}

			IDCount.m_nID = nID;
				
			if ( (map_iter = count_map.find(IDCount.m_nID)) != count_map.end() )
			{
				IDCount.m_nCount = map_iter->second;
				m_nTotal += IDCount.m_nCount;
			}
			else
			{
				IDCount.m_nCount = 0;
			}
			
			if ( sr.m_Flag2 || IDCount.m_nCount > 0 )
				m_list.push_back(IDCount);
			
		}

		// include standard responses where the agent is unknown
		if ( sr.m_Flag1 )
		{
			m_query.Reset(false);
			m_query.Execute( _T("SELECT 0,COUNT(*) FROM StandardResponses ")
								_T("WHERE AgentID NOT IN (SELECT AgentID FROM Agents)") );

			m_query.Fetch();
			{
				if ( IDCount.m_nCount > 0 || sr.m_Flag2 )
				{
					m_nTotal += IDCount.m_nCount;
					m_list.push_front( IDCount );
				}
			}
		}
		
		OpenCDF();
		CEMSString sString;
		sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), _T("Author") , _T("Responses"), _T("Percentage") );
		OutputCDF( sString );
		m_rrr.clear();
		// add rows
		list<CIDCounts>::iterator iter;
		int i=0;

		for ( iter = m_list.begin(); iter != m_list.end(); iter++ )
		{
			CEMSString sName( _T("Unknown") );
			CEMSString sCount, sPercent;

			GetAgentName( iter->m_nID, sName );
			sCount.Format( _T("%d"), iter->m_nCount );
			sPercent.Format( _T("%.2f"), m_nTotal ? ((float) iter->m_nCount / (float) m_nTotal) * 100 : 0);
			
			sName.EscapeCSV();

			CEMSString sString;
			sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"),  sName.c_str(), sCount.c_str(), sPercent.c_str() );
			OutputCDF( sString );
			nPercentage = m_nTotal ? ((long) iter->m_nCount / (long) m_nTotal) * 100 : 0;
			_tcscpy( rrr.m_Col1,sName.c_str() );
			_tcscpy( rrr.m_Col2,_T("") );
			rrr.m_Col3 = iter->m_nCount;
			rrr.m_Col4 = nPercentage;
			rrr.m_Col5 = 0;
			rrr.m_Col6 = 0;
			rrr.m_Col7 = 0;
			rrr.m_Col8 = 0;
			rrr.m_Col9 = 0;
			rrr.m_Col10 = 0;
			rrr.m_Col11 = 0;
			rrr.m_Col12 = 0;
			rrr.m_Col13 = 0;
			m_rrr.push_back( rrr );				
		}
		
		if ( i == 0 )
		{
			CEMSString sLine;
			sLine.Format( _T("No results returned\r\n") );
			OutputCDF( sLine );
		}
		AddResultRows();
		CloseCDF();

	}
}

void CReporter::AverageResponseTimeReport( void )
{
	CEMSString sQuery;
	CEMSString sDatePart;
	CEMSString sAutoReply;
	CEMSString sFilter;
	int nReplySeconds;

	if ( m_ReportObjectID == 1 && sr.m_TargetID > 0 )
	{
		sFilter.Format( _T("AND I.OriginalTicketBoxID=%d"), sr.m_TargetID );
	}
	else if ( m_ReportObjectID == 3 && sr.m_TargetID > 0 )
	{
		sFilter.Format( _T("AND I.OriginalTicketCategoryID=%d"), sr.m_TargetID );
	}
	else if ( sr.m_TargetID > 0 )
	{
		// TODO - define if this should be I.OriginalOwnerID or O.AgentID
		sFilter.Format( _T("AND O.AgentID=%d"), sr.m_TargetID );
	}
	
	if ( !sr.m_Flag1 )
		sAutoReply.assign( _T("AND O.AgentID!=0") );

	SetStartEndDate();
	m_query.Initialize();

	GetWhenSelect( sDatePart, _T("I.DateReceived"), true ); 
	BINDCOL_LONG_NOLEN( m_query, nReplySeconds );
	BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
	BINDPARAM_TIME_NOLEN( m_query, m_EndDate );
	
	/*sQuery.Format(  _T("SELECT %s, AVG(DATEDIFF(second,I.DateReceived, O.EmailDateTime)) ")
		_T("FROM InboundMessages as I ")
		_T("INNER JOIN OutboundMessages as O ON I.InboundMessageID = O.ReplyToMsgID ")
		_T("WHERE I.IsDeleted=0 AND O.IsDeleted=0 AND O.ReplyToIDIsInbound=1 ")
		_T("AND I.DateReceived BETWEEN ? AND ? %s %s ")
		_T("AND OutboundMessageStateID!=1 AND OutboundMessageTypeID=3 ")
		_T("AND O.EmailDateTime>I.DateReceived ")
		_T("GROUP BY %s ORDER BY %s"),
		sDatePart.c_str(), sFilter.c_str(), sAutoReply.c_str(), sDatePart.c_str(), sDatePart.c_str() );

	m_query.Execute( sQuery.c_str() );*/

	sQuery.Format(  _T("SET DATEFIRST 7 SELECT %s, AVG(DATEDIFF(second,I.DateReceived, O.EmailDateTime)) ")
		_T("FROM InboundMessages as I ")
		_T("CROSS APPLY (SELECT TOP 1 om.EmailDateTime,om.AgentID FROM OutboundMessages AS om ")
		_T("WHERE om.IsDeleted=0 AND om.ReplyToIDIsInbound=1 AND om.OutboundMessageStateID!=1 AND om.OutboundMessageTypeID=3 AND I.InboundMessageID=om.ReplyToMsgID ")
		_T("ORDER BY om.OutboundMessageID ASC) AS O ")
		_T("WHERE I.IsDeleted=0 AND I.DateReceived BETWEEN ? AND ? %s %s ")
		_T("AND O.EmailDateTime>I.DateReceived ")
		_T("GROUP BY %s ORDER BY %s"),
		sDatePart.c_str(), sFilter.c_str(), sAutoReply.c_str(), sDatePart.c_str(), sDatePart.c_str() );

	m_query.Execute( sQuery.c_str() );

	// define columns
	OpenCDF();
	OutputCDF( CEMSString( _T("Date, Average Response Time (Hours)\r\n") ) );
	// fetch the data and output the results
	CEMSString sDate, sDateEsc, sTime;
	CEMSString sLine;
	m_rrr.clear();
	int nRows = 0;
	for ( nRows=0; m_query.Fetch() == S_OK; nRows++ )
	{	
		if ( nRows == sr.m_MaxResults )
		{
			OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
			break;
		}

		GetWhen2( sDate );
		sDateEsc.assign( sDate );
		sDateEsc.EscapeJavascript();
		sTime.Format( _T("%.2f"), (float) nReplySeconds / (float) 3600 );

		// add line to CSV file
		sLine.Format( L"\"%s\",\"%s\"\r\n", sDate.c_str(), sTime.c_str() );
		OutputCDF( sLine );
		_tcscpy( rrr.m_Col1,sDate.c_str() );
		_tcscpy( rrr.m_Col2,sTime.c_str() );
		rrr.m_Col3 = 0;
		rrr.m_Col4 = 0;
		rrr.m_Col5 = 0;
		rrr.m_Col6 = 0;
		rrr.m_Col7 = 0;
		rrr.m_Col8 = 0;
		rrr.m_Col9 = 0;
		rrr.m_Col10 = 0;
		rrr.m_Col11 = 0;
		rrr.m_Col12 = 0;
		rrr.m_Col13 = 0;
		m_rrr.push_back( rrr );		
	}
	AddResultRows();
	if ( nRows == 0 )
	{
		sLine.Format( _T("No results returned\r\n") );
		OutputCDF( sLine );
	}

	CloseCDF();
}

void CReporter::AverageTimeToResolveReport( void )
{
	CEMSString sQuery;
	CEMSString sDatePart;
	CEMSString sFilter;
	int nMins, isAdmin;
	
	//Check to see if report owner is an admin
	isAdmin = 0;
	m_query.Initialize();
	BINDPARAM_LONG( m_query, sr.m_OwnerID );
	BINDCOL_LONG_NOLEN( m_query, isAdmin );
	m_query.Execute( _T("SELECT AgentGroupingID FROM AgentGroupings WHERE GroupID=2 AND AgentID=?") );
	m_query.Fetch();
		
	if ( m_ReportObjectID == 1 && sr.m_TargetID > 0 )
	{
		sFilter.Format( _T("AND T.TicketBoxID=%d"), sr.m_TargetID );
	}
	else if ( m_ReportObjectID == 3 && sr.m_TargetID > 0 )
	{
		sFilter.Format( _T("AND T.TicketCategoryID=%d"), sr.m_TargetID );
	}
	else if ( sr.m_TargetID > 0 )
	{
		sFilter.Format( _T("AND T.OwnerID=%d"), sr.m_TargetID );
	}
	SetStartEndDate();	

	m_query.Initialize();
	GetWhenSelect( sDatePart, _T("T.DateCreated"), true );
	BINDCOL_LONG_NOLEN( m_query, nMins );
	BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
	BINDPARAM_TIME_NOLEN( m_query, m_EndDate );

	if ( isAdmin < 1 )
	{
		sQuery.Format( _T("SELECT %s, AVG(OpenMins) FROM Tickets as T ")
					   _T("INNER JOIN AgentTicketCategories ATC ON ")
					   _T("T.TicketCategoryID=ATC.TicketCategoryID ")
					   _T("WHERE ATC.AgentID=%d AND T.IsDeleted=0 AND TicketStateID=1 AND T.DateCreated BETWEEN ? AND ? %s AND EXISTS ")
					   _T("(SELECT InboundMessageID FROM InboundMessages as I ")
					   _T("WHERE IsDeleted=0 AND I.TicketID = T.TicketID)") 
					   _T("GROUP BY %s ORDER BY %s"),
					   sDatePart.c_str(), sFilter.c_str(), sDatePart.c_str(), sDatePart.c_str() );	
	}
	else
	{
		sQuery.Format( _T("SELECT %s, AVG(OpenMins) FROM Tickets as T ")
					   _T("WHERE T.IsDeleted=0 AND TicketStateID=1 AND T.DateCreated BETWEEN ? AND ? %s AND EXISTS ")
					   _T("(SELECT InboundMessageID FROM InboundMessages as I ")
					   _T("WHERE IsDeleted=0 AND I.TicketID = T.TicketID)") 
					   _T("GROUP BY %s ORDER BY %s"),
					   sDatePart.c_str(), sFilter.c_str(), sDatePart.c_str(), sDatePart.c_str() );	
	}
	

	m_query.Execute( sQuery.c_str() );

	// define columns
	OpenCDF();
	OutputCDF( CEMSString( _T("Date, Time to Resolve (Hours)\r\n") ) );
	
	// fetch the data and output the results
	CEMSString sDate, sDateEsc, sTime;
	CEMSString sLine;
	m_rrr.clear();
	int nRows = 0;
	for ( nRows=0; m_query.Fetch() == S_OK; nRows++ )
	{	
		if ( nRows == sr.m_MaxResults )
		{
			OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
			break;
		}

		GetWhen2( sDate );
		sDateEsc.assign( sDate );
		sDateEsc.EscapeJavascript();
		sTime.Format( _T("%.2f"), (float) nMins / (float) 60 );

		// add line to CSV file
		sLine.Format( L"\"%s\",\"%s\"\r\n", sDate.c_str(), sTime.c_str() );
		OutputCDF( sLine );
		_tcscpy( rrr.m_Col1,sDate.c_str() );
		_tcscpy( rrr.m_Col2,sTime.c_str() );
		rrr.m_Col3 = 0;
		rrr.m_Col4 = 0;
		rrr.m_Col5 = 0;
		rrr.m_Col6 = 0;
		rrr.m_Col7 = 0;
		rrr.m_Col8 = 0;
		rrr.m_Col9 = 0;
		rrr.m_Col10 = 0;
		rrr.m_Col11 = 0;
		rrr.m_Col12 = 0;
		rrr.m_Col13 = 0;
		m_rrr.push_back( rrr );			
	}

	if ( nRows == 0 )
	{
		sLine.Format( _T("No results returned\r\n") );
		OutputCDF( sLine );
	}
	AddResultRows();
	CloseCDF();
}

void CReporter::BusiestTimeReport( void )
{
	CEMSString sQuery, sFilter, sString, m_sTitle, sName, sCount, sPercent;
	CIDCounts IDCount;
	int m_nTotal = 0;
		
	LPCTSTR szDatePart = m_bHour ? _T("hour") : _T("dw");
	m_sTitle = m_bHour ? _T("Busiest Hour of the Day") : _T("Busiest Day of the Week");

	if ( sr.m_TargetID > 0 )
	{
		sFilter.Format( _T("AND I.OriginalTicketBoxID=%d"), sr.m_TargetID );
	}
	
	SetStartEndDate();
	m_query.Initialize();

	BINDCOL_LONG_NOLEN( m_query, IDCount.m_nID );
	BINDCOL_LONG_NOLEN( m_query, IDCount.m_nCount );
	BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
	BINDPARAM_TIME_NOLEN( m_query, m_EndDate );

	sQuery.Format(	_T("SELECT DATEPART(%s,I.DateReceived), COUNT(*) ")
					_T("FROM InboundMessages as I WHERE DateReceived BETWEEN ? AND ? %s ")
					_T("GROUP BY DATEPART(%s,I.DateReceived) ORDER BY DATEPART(%s,I.DateReceived)"), 
					szDatePart, sFilter.c_str(), szDatePart, szDatePart );

	m_query.Execute( sQuery.c_str() );

	OpenCDF();
	sString.Format( _T("%s\r\n"), m_sTitle.c_str() );
	OutputCDF( sString );
	sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), m_bHour ? _T("Hour") : _T("Day") , _T("Messages Received"), _T("Percentage") );
	OutputCDF( sString );
	m_rrr.clear();
	int nRows = 0;
	for ( nRows=0; m_query.Fetch() == S_OK; nRows++ )
	{		
		if ( nRows == sr.m_MaxResults )
		{
			OutputCDF( CEMSString( _T("Maximum line limit reached\r\n") ) );
			break;
		}

		m_nTotal += IDCount.m_nCount;
		m_bHour ? GetHourString( IDCount.m_nID, sName) : GetWeekDayName( sName, IDCount.m_nID );
	
		sCount.Format( _T("%d"), IDCount.m_nCount );
		sPercent.Format( _T("%.2f"), m_nTotal ? ((float) IDCount.m_nCount / (float) m_nTotal) * 100 : 0);
		
		sName.EscapeCSV();
			
		CEMSString sString;
		sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), sName.c_str(), sCount.c_str(), sPercent.c_str() );
		OutputCDF( sString );
		nPercentage = m_nTotal ? ((long) IDCount.m_nCount / (long) m_nTotal) * 100 : 0;
		_tcscpy( rrr.m_Col1,sName.c_str() );
		_tcscpy( rrr.m_Col2,_T("") );
		rrr.m_Col3 = IDCount.m_nCount;
		rrr.m_Col4 = nPercentage;
		rrr.m_Col5 = 0;
		rrr.m_Col6 = 0;
		rrr.m_Col7 = 0;
		rrr.m_Col8 = 0;
		rrr.m_Col9 = 0;
		rrr.m_Col10 = 0;
		rrr.m_Col11 = 0;
		rrr.m_Col12 = 0;
		rrr.m_Col13 = 0;
		m_rrr.push_back( rrr );		
	}
	AddResultRows();
	if ( nRows == 0 )
		OutputCDF( CEMSString( _T("No results returned\r\n") ) );
	
	CloseCDF();	
}

void CReporter::TicketDispositionReport( void )
{
	list<int> m_tbids;
	list<int>::iterator tbIter;
	int m_nCreated;
	int m_nMovedTo;
	int m_nOpened;
	int m_nRestored;
	int m_nClosed;
	int m_nDeleted;
	int m_nMovedFrom;
	CEMSString sTicketBoxName;
	CEMSString sLine;
	CEMSString sSubTitle;
	CEMSString sString;
	int m_nTotalIn;
	int m_nTotalOut;
	int m_nTotal;
	int nTbID;

	if ( m_ReportObjectID == 1 && sr.m_TargetID > 0 )
	{
		m_tbids.push_back(sr.m_TargetID);
	}
	else
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, nTbID );
		m_query.Execute( L"SELECT TicketBoxID FROM TicketBoxes" );
		for ( int nRows=0; m_query.Fetch() == S_OK; nRows++ )
		{ 
			m_tbids.push_back(nTbID);
		}
	}
	
	SetStartEndDate();
	
	// build the sub title
	sSubTitle.assign( _T("Unknown TicketBox") );
	if ( sr.m_TargetID == -1 )
	{
		sSubTitle.assign( _T("All TicketBoxes") );					
	}
	else
	{
		GetTicketBoxName( sr.m_TargetID, sSubTitle );
		sSubTitle.append( _T(" - TicketBox") );					
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
	
	sLine.Format( _T("\"Ticket Activity by TicketBox\"\r\n") );
	OpenCDF();
	OutputCDF( sLine );
	sLine.Format( _T("\"%s\"\r\n"),sSubTitle.c_str() );
	OutputCDF( sLine );
	sLine.Format( _T("\r\n") );
	OutputCDF( sLine );
	sLine.Format( _T("\"TicketBox\",\"New\",\"Moved To\",\"Opened\",\"Restored\",\"Total In\",\"Closed\",\"Deleted\",\"Moved From\",\"Total Out\",\"Total\"\r\n") );
	OutputCDF( sLine );
	m_rrr.clear();
	for( tbIter = m_tbids.begin(); tbIter != m_tbids.end(); tbIter++ )
	{
		GetTicketBoxName( *tbIter, sTicketBoxName );

		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, m_nCreated );
		BINDCOL_LONG_NOLEN( m_query, m_nMovedTo );
		BINDCOL_LONG_NOLEN( m_query, m_nOpened );
		BINDCOL_LONG_NOLEN( m_query, m_nRestored );
		BINDCOL_LONG_NOLEN( m_query, m_nClosed );
		BINDCOL_LONG_NOLEN( m_query, m_nDeleted );
		BINDCOL_LONG_NOLEN( m_query, m_nMovedFrom );
		BINDPARAM_LONG( m_query, *tbIter );
		BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
		BINDPARAM_TIME_NOLEN( m_query, m_EndDate );
		BINDPARAM_LONG( m_query, *tbIter );
		BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
		BINDPARAM_TIME_NOLEN( m_query, m_EndDate );
		BINDPARAM_LONG( m_query, *tbIter );
		BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
		BINDPARAM_TIME_NOLEN( m_query, m_EndDate );
		BINDPARAM_LONG( m_query, *tbIter );
		BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
		BINDPARAM_TIME_NOLEN( m_query, m_EndDate );
		BINDPARAM_LONG( m_query, *tbIter );
		BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
		BINDPARAM_TIME_NOLEN( m_query, m_EndDate );
		BINDPARAM_LONG( m_query, *tbIter );
		BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
		BINDPARAM_TIME_NOLEN( m_query, m_EndDate );
		BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
		BINDPARAM_TIME_NOLEN( m_query, m_EndDate );
		BINDPARAM_LONG( m_query, *tbIter );
		m_query.Execute( _T("SELECT (SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=1), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=2 AND ID1=1), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=2 AND ID1=3 AND ID2=1 AND (TicketStateID=2 OR TicketStateID=3 OR TicketStateID=6)), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=13 AND (TicketStateID=2 OR TicketStateID=3 OR TicketStateID=6)), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=2 AND ID1=3 AND TicketStateID=1), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE TicketBoxID=? AND DateTime BETWEEN ? AND ? AND TicketActionID=7), ")
							_T("(SELECT COUNT(*) FROM TicketHistory WHERE DateTime BETWEEN ? AND ? AND TicketActionID=2 AND ID1=1 AND ID2=?) FROM TicketHistory"));
		// fetch the data and output the results
		if ( m_query.Fetch() == S_OK )
		{
			m_nTotalIn = long(m_nCreated) + long(m_nMovedTo) + long(m_nOpened) + long(m_nRestored);
			m_nTotalOut = long(m_nClosed) + long(m_nDeleted) + long(m_nMovedFrom);
			m_nTotal = long(m_nTotalIn) + long(m_nTotalOut);
			
			if ( sr.m_Flag1 || m_nTotal > 0 )
			{
				sLine.Format( _T("\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"\r\n"), 
							sTicketBoxName.c_str(),m_nCreated,m_nMovedTo,m_nOpened,m_nRestored,m_nTotalIn,m_nClosed,m_nDeleted,m_nMovedFrom,m_nTotalOut,m_nTotal );
				OutputCDF( sLine );

				_tcscpy( rrr.m_Col1,sTicketBoxName.c_str() );
				_tcscpy( rrr.m_Col2,_T("") );
				rrr.m_Col3 = m_nCreated;
				rrr.m_Col4 = 0;
				rrr.m_Col5 = m_nMovedTo;
				rrr.m_Col6 = m_nOpened;
				rrr.m_Col7 = m_nRestored;
				rrr.m_Col8 = m_nTotalIn;
				rrr.m_Col9 = m_nClosed;
				rrr.m_Col10 = m_nDeleted;
				rrr.m_Col11 = m_nMovedFrom;
				rrr.m_Col12 = m_nTotalOut;
				rrr.m_Col13 = m_nTotal;

				m_rrr.push_back( rrr );	
			}
		}
	}
	
	AddResultRows();
	CloseCDF();	
}

void CReporter::AlertOwner()
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	long NowLen = sizeof(Now);
	CEMSString sDate;
	CEMSString reportDateTime;
	
	GetDateTimeString( Now, NowLen, sDate );

	CustomAlert(EMS_ALERT_EVENT_REPORT_RAN,m_ReportResultID,_T("Scheduled Report [%s] ran at %s"), sr.m_Description, sDate.c_str() );
}

void CReporter::EmailReport()
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	long NowLen = sizeof(Now);
	CEMSString sDate;
	CEMSString reportDateTime;
	GetDateTimeString( Now, NowLen, sDate );
	CEMSString msgSubject;
	CEMSString msgBody;
	CEMSString attachFile;
	
	TOutboundMessages reportmsg;
	TAttachments attach;

	tstring sFilename;
	tstring sType = _T("application");
	tstring sSubType = _T("octet-stream");
	
	// auto-reply to this message

	reportmsg.m_AgentID = 0;		// System is sending this message
	GetTimeStamp( reportmsg.m_EmailDateTime );
	reportmsg.m_EmailDateTimeLen = sizeof( TIMESTAMP_STRUCT );
	reportmsg.m_TicketID = 0;

	msgSubject.Format( _T("Scheduled Report [%s] ran at %s"), sr.m_Description, sDate.c_str());
	// Put the Re: in front of the subject, if there isn't one already
	swprintf( reportmsg.m_Subject, msgSubject.c_str() );

	reportmsg.m_OutboundMessageTypeID = EMS_OUTBOUND_MESSAGE_TYPE_NEW;
	reportmsg.m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_UNTOUCHED;
	reportmsg.m_IsDeleted = 2;
	wcscpy( reportmsg.m_MediaType, _T("TEXT") );
	wcscpy( reportmsg.m_MediaSubType, _T("PLAIN") );

	msgBody.Format(_T("Scheduled Report [%s] ran at %s"), sr.m_Description, sDate.c_str());
	// Copy in the body of the alert
	PutStringPropertyW( (wchar_t*)msgBody.c_str() , &reportmsg.m_Body, &reportmsg.m_BodyAllocated );

	// Set the Email To: Address
	wcscpy( reportmsg.m_EmailPrimaryTo, sr.m_ResultEmailTo );
	PutStringPropertyW( reportmsg.m_EmailPrimaryTo, &reportmsg.m_EmailTo, &reportmsg.m_EmailToAllocated );

	// Set the Email From: Address
	wcscpy( reportmsg.m_EmailFrom,m_FromEmailAddress );

	// Copy the EmailFrom -> EmailReplyTo
	PutStringPropertyW( reportmsg.m_EmailFrom, &reportmsg.m_EmailReplyTo, &reportmsg.m_EmailReplyToAllocated );

	// Create the message record
	reportmsg.Insert( m_query );

	//Add Report as an attachment
	if ( sr.m_SendResultToEmail )
	{
		attachFile.Format(_T("%s%s"),m_sOutboundAttachFolder.c_str(), m_sReportFile.c_str());

		sFilename = attachFile.c_str();
		TCHAR dir[_MAX_DIR];
		TCHAR file[_MAX_FNAME];
		TCHAR ext[_MAX_EXT];
		
		sFilename.resize(ATTACHMENTS_ATTACHMENTLOCATION_LENGTH-1, '\0');
		_tsplitpath(sFilename.c_str(), NULL, dir, file, ext);

		{
			tstring szFolder;
			if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("OutboundAttachFolder")).c_str(), szFolder))
			{
				if(szFolder.length() > 0 && szFolder.at(szFolder.size()-1) != '\\')
					szFolder += '\\';

				sFilename = sFilename.substr(szFolder.length(),sFilename.length());
			}
			else
				sFilename = (tstring)file + (tstring)ext;
		}
		
		attach.m_IsInbound = 0;
		_tcsncpy( attach.m_FileName, m_sReportFile.c_str(), 255);
		_tcsncpy( attach.m_AttachmentLocation, sFilename.c_str(), 255);
		_tcsncpy( attach.m_MediaType, sType.c_str(), 125);
		_tcsncpy( attach.m_MediaSubType, sSubType.c_str(), 125);
		_tcscpy( attach.m_ContentDisposition, _T("attachment") );
		attach.Insert( m_query );

		TOutboundMessageAttachments outbound_msg_attach;
		outbound_msg_attach.m_AttachmentID = attach.m_AttachmentID;
		outbound_msg_attach.m_OutboundMessageID = reportmsg.m_OutboundMessageID;
		outbound_msg_attach.Insert( m_query );

	}

	//	Put In Outbound Queue
	m_query.Reset();
	BINDPARAM_LONG( m_query, reportmsg.m_OutboundMessageID );
	BINDPARAM_TIME( m_query, reportmsg.m_EmailDateTime );
	m_query.Execute( L"INSERT INTO OutboundMessageQueue "
					 L"(OutboundMessageID,DateSpooled,IsApproved) "
					 L"VALUES "
					 L"(?,?,1)" );

	// undelete the message
	m_query.Initialize();
	BINDPARAM_LONG( m_query, reportmsg.m_OutboundMessageID );
	m_query.Execute( L"UPDATE OutboundMessages "
					 L"SET IsDeleted=0 "
					 L"WHERE OutboundMessageID=?");
}
void CReporter::UpdateReport()
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	CEMSString sTemp;
	
	SetNextRunTime();

	sr.m_LastRunTime = Now;
	sr.m_LastResultCode = m_ThisResult;

	//Update the Report
	m_query.Reset( true );
	BINDPARAM_TIME_NOLEN( m_query, sr.m_NextRunTime );
	BINDPARAM_TIME_NOLEN( m_query, sr.m_LastRunTime );
	BINDPARAM_LONG( m_query, sr.m_LastResultCode );
	BINDPARAM_LONG( m_query, sr.m_ScheduledReportID );
	m_query.Execute( L"UPDATE ReportScheduled "
						 L"SET NextRunTime=?,LastRunTime=?,LastResultCode=? "
						 L"WHERE ScheduledReportID = ?" );

	//Update the Report Result Entry
	sTemp.Format( _T("%s\\%s"), m_sReportPath.c_str(), m_sReportFile.c_str() );

	m_query.Reset( true );
	BINDPARAM_LONG( m_query, sr.m_LastResultCode );
	BINDPARAM_TCHAR_STRING( m_query, sTemp );
	BINDPARAM_TIME_NOLEN( m_query, m_StartDate );
	BINDPARAM_TIME_NOLEN( m_query, m_EndDate );
	BINDPARAM_LONG( m_query, m_ReportResultID );
	m_query.Execute( L"UPDATE ReportResults "
						 L"SET ResultCode=?,ResultFile=?,DateFrom=?,DateTo=? "
						 L"WHERE ReportResultID = ?" );

	if ( sr.m_SaveResultToFile == 0 )
	{
		//Remove the temp file from the report folder
		DeleteFile( m_sFullReportFile.c_str() );
	}
		
}




////////////////////////////////////////////////////////////////////////////////
// 
// Set Next Run Time
// 
////////////////////////////////////////////////////////////////////////////////
void CReporter::SetNextRunTime( void )
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	int nDayOfWeek = GetDayOfWeek( Now.month, Now.day, Now.year );
	int xTime = 0;
	time_t now;
	time( &now ); 
	TIMESTAMP_STRUCT m_DateTime;
				
	if(sr.m_RunFreq==0)
	{
		switch(nDayOfWeek)
		{
		case 0:
			if(sr.m_RunSun)
			{
				if(sr.m_RunAtHour<=Now.hour)
				{
					if(sr.m_RunMon){xTime=86400;}
					else if(sr.m_RunTue){xTime=86400*2;}
					else if(sr.m_RunWed){xTime=86400*3;}
					else if(sr.m_RunThur){xTime=86400*4;}
					else if(sr.m_RunFri){xTime=86400*5;}
					else if(sr.m_RunSat){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(sr.m_RunMon){xTime=86400;}
				else if(sr.m_RunTue){xTime=86400*2;}
				else if(sr.m_RunWed){xTime=86400*3;}
				else if(sr.m_RunThur){xTime=86400*4;}
				else if(sr.m_RunFri){xTime=86400*5;}
				else if(sr.m_RunSat){xTime=86400*6;}
				else {xTime=86400*7;}	
			}
			break;
		case 1:
			if(sr.m_RunMon)
			{
				if(sr.m_RunAtHour<=Now.hour)
				{
					if(sr.m_RunTue){xTime=86400;}
					else if(sr.m_RunWed){xTime=86400*2;}
					else if(sr.m_RunThur){xTime=86400*3;}
					else if(sr.m_RunFri){xTime=86400*4;}
					else if(sr.m_RunSat){xTime=86400*5;}
					else if(sr.m_RunSun){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(sr.m_RunTue){xTime=86400;}
				else if(sr.m_RunWed){xTime=86400*2;}
				else if(sr.m_RunThur){xTime=86400*3;}
				else if(sr.m_RunFri){xTime=86400*4;}
				else if(sr.m_RunSat){xTime=86400*5;}
				else if(sr.m_RunSun){xTime=86400*6;}
				else {xTime=86400*7;}
			}
			break;
		case 2:
			if(sr.m_RunTue)
			{
				if(sr.m_RunAtHour<=Now.hour)
				{
					if(sr.m_RunWed){xTime=86400;}
					else if(sr.m_RunThur){xTime=86400*2;}
					else if(sr.m_RunFri){xTime=86400*3;}
					else if(sr.m_RunSat){xTime=86400*4;}
					else if(sr.m_RunSun){xTime=86400*5;}
					else if(sr.m_RunMon){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(sr.m_RunWed){xTime=86400;}
				else if(sr.m_RunThur){xTime=86400*2;}
				else if(sr.m_RunFri){xTime=86400*3;}
				else if(sr.m_RunSat){xTime=86400*4;}
				else if(sr.m_RunSun){xTime=86400*5;}
				else if(sr.m_RunMon){xTime=86400*6;}
				else {xTime=86400*7;}	
			}
			break;
		case 3:
			if(sr.m_RunWed)
			{
				if(sr.m_RunAtHour<=Now.hour)
				{
					if(sr.m_RunThur){xTime=86400;}
					else if(sr.m_RunFri){xTime=86400*2;}
					else if(sr.m_RunSat){xTime=86400*3;}
					else if(sr.m_RunSun){xTime=86400*4;}
					else if(sr.m_RunMon){xTime=86400*5;}
					else if(sr.m_RunTue){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(sr.m_RunThur){xTime=86400;}
				else if(sr.m_RunFri){xTime=86400*2;}
				else if(sr.m_RunSat){xTime=86400*3;}
				else if(sr.m_RunSun){xTime=86400*4;}
				else if(sr.m_RunMon){xTime=86400*5;}
				else if(sr.m_RunTue){xTime=86400*6;}
				else {xTime=86400*7;}	
			}
			break;
		case 4:
			if(sr.m_RunThur)
			{
				if(sr.m_RunAtHour<=Now.hour)
				{
					if(sr.m_RunFri){xTime=86400;}
					else if(sr.m_RunSat){xTime=86400*2;}
					else if(sr.m_RunSun){xTime=86400*3;}
					else if(sr.m_RunMon){xTime=86400*4;}
					else if(sr.m_RunTue){xTime=86400*5;}
					else if(sr.m_RunWed){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(sr.m_RunFri){xTime=86400;}
				else if(sr.m_RunSat){xTime=86400*2;}
				else if(sr.m_RunSun){xTime=86400*3;}
				else if(sr.m_RunMon){xTime=86400*4;}
				else if(sr.m_RunTue){xTime=86400*5;}
				else if(sr.m_RunWed){xTime=86400*6;}
				else {xTime=86400*7;}
			}
			break;
		case 5:
			if(sr.m_RunFri)
			{
				if(sr.m_RunAtHour<=Now.hour)
				{
					if(sr.m_RunSat){xTime=86400;}
					else if(sr.m_RunSun){xTime=86400*2;}
					else if(sr.m_RunMon){xTime=86400*3;}
					else if(sr.m_RunTue){xTime=86400*4;}
					else if(sr.m_RunWed){xTime=86400*5;}
					else if(sr.m_RunThur){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(sr.m_RunSat){xTime=86400;}
				else if(sr.m_RunSun){xTime=86400*2;}
				else if(sr.m_RunMon){xTime=86400*3;}
				else if(sr.m_RunTue){xTime=86400*4;}
				else if(sr.m_RunWed){xTime=86400*5;}
				else if(sr.m_RunThur){xTime=86400*6;}
				else {xTime=86400*7;}	
			}
			break;
		case 6:
			if(sr.m_RunSat)
			{
				if(sr.m_RunAtHour<=Now.hour)
				{
					if(sr.m_RunSun){xTime=86400;}
					else if(sr.m_RunMon){xTime=86400*2;}
					else if(sr.m_RunTue){xTime=86400*3;}
					else if(sr.m_RunWed){xTime=86400*4;}
					else if(sr.m_RunThur){xTime=86400*5;}
					else if(sr.m_RunFri){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(sr.m_RunSun){xTime=86400;}
				else if(sr.m_RunMon){xTime=86400*2;}
				else if(sr.m_RunTue){xTime=86400*3;}
				else if(sr.m_RunWed){xTime=86400*4;}
				else if(sr.m_RunThur){xTime=86400*5;}
				else if(sr.m_RunFri){xTime=86400*6;}
				else {xTime=86400*7;}
			}
			break;
		}
		SecondsToTimeStamp(now + xTime, m_DateTime);
		m_nYear = m_DateTime.year;
		m_nMonth = m_DateTime.month;
		m_nDay = m_DateTime.day;		
	}
	else if ( sr.m_RunFreq == 1 )
	{
		if ( sr.m_RunOn < nDayOfWeek )
		{
			xTime = (7-(nDayOfWeek-sr.m_RunOn))*86400;			
		}
		else if ( sr.m_RunOn > nDayOfWeek )
		{
			xTime = (sr.m_RunOn-nDayOfWeek)*86400;			
		}
		else
		{
			if ( sr.m_RunAtHour <= Now.hour )
			{
				xTime = 86400;
			}
			else
			{
				xTime = 0;
			}
		}
		SecondsToTimeStamp(now + xTime, m_DateTime);
		m_nYear = m_DateTime.year;
		m_nMonth = m_DateTime.month;
		m_nDay = m_DateTime.day;
	}
	else if ( sr.m_RunFreq == 2 )
	{
		m_nYear = Now.year;
		if ( sr.m_RunOn == 0 )
		{
			m_nDay = 1;
			m_nMonth = Now.month + 1;
			if(m_nMonth == 13)
			{
				m_nMonth = 1;
				m_nYear = m_nYear +1;
			}
		}
		else
		{
			m_nMonth = Now.month;
			m_nDay = GetLastDayOfMonth( Now.month );			
		}		
	}
	else
	{
		if ( sr.m_RunOn == 0 )
		{
			m_nDay = 1;
			m_nMonth = 1;
			m_nYear = Now.year + 1;
		}
		else
		{
			m_nDay = 31;
			m_nMonth = 12;
			m_nYear = Now.year;
		}		
	}

	sr.m_NextRunTime.year = m_nYear;
	sr.m_NextRunTime.month = m_nMonth;
	sr.m_NextRunTime.day = m_nDay;
	sr.m_NextRunTime.hour = sr.m_RunAtHour;
	sr.m_NextRunTime.minute = sr.m_RunAtMin;
}

void CReporter::GetStdCategoryName( int nID, CEMSString& sName )
{
	m_query.Initialize();
	BINDCOL_TCHAR( m_query, m_nName );
	BINDPARAM_LONG( m_query, nID );
	m_query.Execute( _T("SELECT CategoryName ")
	               _T("FROM StdResponseCategories ")
	               _T("WHERE StdResponseCatID=?") );
	
	m_query.Fetch();
	sName.Format( _T("%s"), m_nName );

}

void CReporter::GetAgentName( int nID, CEMSString& sName )
{
	m_query.Initialize();
	BINDCOL_TCHAR( m_query, m_nName );
	BINDPARAM_LONG( m_query, nID );
	m_query.Execute( _T("SELECT Name ")
	               _T("FROM Agents ")
	               _T("WHERE AgentID=?") );
	
	m_query.Fetch();
	sName.Format( _T("%s"), m_nName );

}

void CReporter::GetTicketBoxName( int nID, CEMSString& sName )
{
	m_query.Initialize();
	BINDCOL_TCHAR( m_query, m_nName );
	BINDPARAM_LONG( m_query, nID );
	m_query.Execute( _T("SELECT Name ")
	               _T("FROM TicketBoxes ")
	               _T("WHERE TicketBoxID=?") );
	
	m_query.Fetch();
	sName.Format( _T("%s"), m_nName );
}

int CReporter::GetDayOfWeek( int month, int day, int year )
{
	int weekDay;

	if ( month < 3 )
	{
		month += 12;
		year -= 1;
	}
	
	weekDay = (day + 2*month + 3*(month+1)/5 + year + year/4 - year/100 + year/400 + 1) % 7;

	if ( month > 12 )
	{
		month -= 12;
		year += 1;
	}

	return weekDay;
}

int CReporter::GetLastDayOfMonth( int month )
{
	switch ( month )
	{
	case 1:
		return 31;
	case 2:
		return 28;
	case 3:
		return 31;
	case 4:
		return 30;
	case 5:
		return 31;
	case 6:
		return 30;
	case 7:
		return 31;
	case 8:
		return 31;
	case 9:
		return 30;
	case 10:
		return 31;
	case 11:
		return 30;
	case 12:
		return 31;
	default:
		return 30;
	}
}

void CReporter::GetWeekDayName( tstring& sName, int nDay )
{
	switch ( nDay )
	{
		case 1:
			sName.assign( _T("Sunday") );
			break;
		case 2:
			sName.assign( _T("Monday") );
			break;
		case 3:
			sName.assign( _T("Tuesday") );
			break;
		case 4:
			sName.assign( _T("Wednesday") );
			break;
		case 5:
			sName.assign( _T("Thursday") );
			break;
		case 6:
			sName.assign( _T("Friday") );
			break;
		case 7:
			sName.assign( _T("Saturday") );
			break;

		default:
			sName.assign( _T("Unknown") );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// OpenCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CReporter::OpenCDF( void )
{
	CEMSString sReportFile;
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	dca::String sPath = m_sReportFolder.c_str();
	dca::String aPath = m_sOutboundAttachFolder.c_str();
	
	if ( VerifyDirectory(sPath))
	{
		sReportFile.Format( _T("%s%s-%d%d%d.csv"), m_sReportFolder.c_str(), sr.m_Description, Now.hour, Now.minute, Now.second);
		m_sReportFile.Format( _T("%s-%d%d%d.csv"),sr.m_Description, Now.hour, Now.minute, Now.second );
		m_sFullReportFile.Format(_T("%s"), sReportFile.c_str());
		m_hCDFFile = CreateFile( sReportFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
								FILE_ATTRIBUTE_NORMAL, NULL );

		if( m_hCDFFile == INVALID_HANDLE_VALUE )
		{
			Log( E_Reporter, EMS_STRING_REPORTER_CREATEFILE, sReportFile.c_str(), GetLastError() );			
		}
	}
	else
	{
		Log( E_Reporter, EMS_STRING_REPORTER_CREATEFILE, sReportFile.c_str(), GetLastError() );
	}

	if ( sr.m_IncludeResultFile )
	{
		if ( VerifyDirectory(aPath))
		{
			sReportFile.Format( _T("%s%s"), m_sOutboundAttachFolder.c_str(), m_sReportFile.c_str());
			m_hATTACHFile = CreateFile( sReportFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
									FILE_ATTRIBUTE_NORMAL, NULL );

			if( m_hATTACHFile == INVALID_HANDLE_VALUE )
			{
				Log( E_Reporter, EMS_STRING_REPORTER_CREATEFILE, sReportFile.c_str(), GetLastError() );			
			}
		}
		else
		{
			Log( E_Reporter, EMS_STRING_REPORTER_CREATEFILE, sReportFile.c_str(), GetLastError() );
		}
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// OutputCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CReporter::OutputCDF( tstring& sLine )
{
	char szBuffer[512];
	DWORD dwBytesWritten;
	wstring::size_type pos;
	
	pos = 0;
	// Replace all double quotes with single quotes
	while( (pos = sLine.find( L'"', pos)) != wstring::npos )
	{
		sLine[pos] = L'\'';
	}

	pos = 0;
	// Remove any carriage returns
	while( (pos = sLine.find( L'\r', pos)) != wstring::npos )
	{
		sLine.erase( pos, 1 );
	}

	pos = 0;
	// Remove any linefeeds
	while( (pos = sLine.find( L'\n', pos)) != wstring::npos )
	{
		sLine.erase( pos, 1 );
	}

	_snprintf( szBuffer, 511, "%S\r\n", sLine.c_str() );
				

	// Null terminate to be safe
	szBuffer[511] = '\0';

	WriteFile( m_hCDFFile, szBuffer, strlen(szBuffer), &dwBytesWritten, NULL );
	if ( sr.m_IncludeResultFile )
	{
		WriteFile( m_hATTACHFile, szBuffer, strlen(szBuffer), &dwBytesWritten, NULL );
	}
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// CloseCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CReporter::CloseCDF(void)
{
	CloseHandle( m_hCDFFile );
	if ( sr.m_IncludeResultFile )
	{
		CloseHandle( m_hATTACHFile );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// VerifyDirectory
// 
////////////////////////////////////////////////////////////////////////////////
bool CReporter::VerifyDirectory( dca::String& sPath )
{
	// ensure that the path exists
	DWORD attr;
	int pos;
	bool bRes = true;
	
	// Check for trailing slash:
	pos = sPath.find_last_of("\\");
	if (sPath.length() == pos + 1)	// last character is "\"
	{
		sPath.resize(pos);
	}
	
	// look for existing path part
	attr = GetFileAttributesA(sPath.c_str());
	
	// if it doesn't exist
	if (0xFFFFFFFF == attr)
	{
		pos = sPath.find_last_of("\\");
		if (0 < pos)
		{
			// create parent dirs
			dca::String p = sPath.substr(0, pos);
			bRes = VerifyDirectory(p);
		}
		
		// create note
		dca::WString path(sPath.c_str());
		bRes = bRes && CreateDirectoryW(path.c_str(), NULL);
	}
	else if (!(FILE_ATTRIBUTE_DIRECTORY & attr))
	{	
		bRes = false;
	}
	
	return bRes;
}

////////////////////////////////////////////////////////////////////////////////
// 
// UnInitialize
// 
////////////////////////////////////////////////////////////////////////////////
void CReporter::UnInitialize(void)
{
	// Handle all remaining alerts
	/*while( m_Queue.size() > 0 )
	{
		HandleAlert( m_Queue.front() );
		m_Queue.pop_front();
	}*/
}

