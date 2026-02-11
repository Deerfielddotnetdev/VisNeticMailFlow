/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ScheduleReport.cpp,v 1.2.2.1 2005/12/13 18:11:49 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Report Scheduler
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ScheduleReport.h"
#include "DateFns.h"
#include <time.h>
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CScheduleReport::CScheduleReport( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_hCDFFile = INVALID_HANDLE_VALUE;
	m_MaxRows = 250;
	m_isAdminView = false;
	m_OutputCDF = false;
}

CScheduleReport::~CScheduleReport()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CScheduleReport::Run( CURLAction& action )
{
	int nisAdmin = 0;
	
	// check security
	RequireAgentRightLevel( EMS_OBJECT_TYPE_SCHEDULED_REPORTS, 0, EMS_EDIT_ACCESS );
	
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	//RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_ScheduledReportID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_ScheduledReportID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_ScheduledReportID);
		}
	}
	
	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( GetISAPIData().m_sPage.compare( _T("schedulereport") ) == 0 )
	{
		if( sAction.compare( _T("insert") ) == 0 )
		{
			DISABLE_IN_DEMO();
			New();
			return 0;
		}
		else if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();

			if( m_ScheduledReportID == 0 )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			
			Update();
			return 0;
		}
		
		// change title if ID is zero
		if( m_ScheduledReportID == 0 )
		{
			action.m_sPageTitle.assign( "New Scheduled Report" );
		}
		else
		{
			action.m_sPageTitle.assign( "Edit Scheduled Report" );
		}
		
		//Is the request coming from the Admin link
		GetISAPIData().GetURLLong( _T("ISADMIN"), nisAdmin, true );
		if ( nisAdmin == 1 )
		{
			m_isAdminView = true;				
		}
			
		return Query();
	}
	
	if( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();

		if( m_ScheduledReportID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

		Delete();
	}

	if( GetISAPIData().m_sPage.compare( _T("scheduledadminreports") ) == 0 )
	{
		m_isAdminView = true;
		action.m_sPageTitle.assign( "Scheduled Reports" );
		return ListAll();	
	}
	else
	{
		action.m_sPageTitle.assign( "Scheduled Reports" );
		return ListOwnerAll();	
	}
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query all scheduled reports
// 
////////////////////////////////////////////////////////////////////////////////
int CScheduleReport::ListAll(void)
{
	RequireAdmin();

	CEMSString sDate;
	PrepareList( GetQuery() );
	CEMSString nLastResult;

	GetXMLGen().AddChildElem( _T("ScheduledReports") );
	GetXMLGen().IntoElem();
	
    while( GetQuery().Fetch() == S_OK )
	{
		switch( m_LastResultCode )
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

		GetXMLGen().AddChildElem( _T("Report") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_ScheduledReportID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetDateTimeString( m_LastRunTime, m_LastRunTimeLen, sDate );
		if ( sDate == "NULL DATE" )
			sDate.Format( _T("Unknown"));
		GetXMLGen().AddChildAttrib( _T("LastRan"), sDate.c_str() );
		GetXMLGen().AddChildAttrib( _T("LastResult"), nLastResult.c_str() );
		GetDateTimeString( m_NextRunTime, m_NextRunTimeLen, sDate );
		GetXMLGen().AddChildAttrib( _T("NextRun"), sDate.c_str() );
		GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDate );
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDate.c_str() );
		GetDateTimeString( m_DateEdited, m_DateEditedLen, sDate );
		GetXMLGen().AddChildAttrib( _T("DateEdited"), sDate.c_str() );
		GetXMLGen().AddChildAttrib( _T("CreatedByID"), m_CreatedByID );
		GetXMLGen().AddChildAttrib( _T("EditedByID"), m_EditedByID );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
		GetXMLGen().AddChildAttrib( _T("OwnerID"), m_OwnerID );
	}

	GetXMLGen().OutOfElem();

	ListEnabledAgentNames();

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Query all scheduled reports for a particular owner
// 
////////////////////////////////////////////////////////////////////////////////
int CScheduleReport::ListOwnerAll(void)
{
	CEMSString sDate;
	m_OwnerID  = GetAgentID();
	PrepareOwnerList( GetQuery() );
	CEMSString nLastResult;

	

	GetXMLGen().AddChildElem( _T("ScheduledReports") );
	GetXMLGen().IntoElem();
	
    while( GetQuery().Fetch() == S_OK )
	{
		switch( m_LastResultCode )
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

		GetXMLGen().AddChildElem( _T("Report") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_ScheduledReportID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetDateTimeString( m_LastRunTime, m_LastRunTimeLen, sDate );
		if ( sDate == "NULL DATE" )
			sDate.Format( _T("Unknown"));
		GetXMLGen().AddChildAttrib( _T("LastRan"), sDate.c_str() );
		GetXMLGen().AddChildAttrib( _T("LastResult"), nLastResult.c_str() );
		GetDateTimeString( m_NextRunTime, m_NextRunTimeLen, sDate );
		GetXMLGen().AddChildAttrib( _T("NextRun"), sDate.c_str() );
		GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDate );
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDate.c_str() );
		GetDateTimeString( m_DateEdited, m_DateEditedLen, sDate );
		GetXMLGen().AddChildAttrib( _T("DateEdited"), sDate.c_str() );
		GetXMLGen().AddChildAttrib( _T("CreatedByID"), m_CreatedByID );
		GetXMLGen().AddChildAttrib( _T("EditedByID"), m_EditedByID );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	}

	GetXMLGen().OutOfElem();

	ListEnabledAgentNames();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query for a particular scheduled report
// 
////////////////////////////////////////////////////////////////////////////////
int CScheduleReport::Query(void)

{
	GetISAPIData().GetURLLong( _T("ID"), m_ScheduledReportID  );

	if ( m_ScheduledReportID > 0 )
	{
		TScheduleReport::Query(GetQuery());
	}
	else
	{
		//Set defaults for a new report
		m_IsEnabled = 1;
		m_OwnerID = GetAgentID();
		m_RunFreq = 0;
		m_RunOn = 0;
		m_RunSun = 1;
		m_RunMon = 1;
		m_RunTue = 1;
		m_RunWed = 1;
		m_RunThur = 1;
		m_RunFri = 1;
		m_RunSat = 1;
		m_RunAtHour = 0;
		m_RunAtMin = 15;
		m_SendAlertToOwner = 1;
		m_SendResultToEmail = 0;
		m_IncludeResultFile = 0;
		m_SaveResultToFile = 0;
		//m_KeepNumResultFile = 7;
		m_SumPeriod = 1;
		m_MaxResults = 250;
	}
	
	GenerateXML();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update a scheduled report
// 
////////////////////////////////////////////////////////////////////////////////
int CScheduleReport::Update()
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	
	DecodeForm();

	//Check for unique Description
	CheckForDuplicity();

	m_EditedByID = GetAgentID();
	m_DateEdited = Now;
	SetNextRunTime();
	TScheduleReport::Update(GetQuery());
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a scheduled report
// 
////////////////////////////////////////////////////////////////////////////////
int CScheduleReport::Delete()
{
	list<int> rrIDs;
	list<int>::iterator iter;
	int nReportResultID;
	CEMSString sReportFileName;
	CEMSString sTempPath;
	long sTempPathLen = sizeof(sTempPath);
	TReportResults trr;
	
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


	if ( m_ScheduledReportID )
	{
		// Get Report Results for this Scheduled Report
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nReportResultID );
		BINDPARAM_LONG( GetQuery(), m_ScheduledReportID );
		GetQuery().Execute( _T("SELECT ReportResultID ")
							_T("FROM ReportResults ")
							_T("WHERE ScheduledReportID=?") );

		while( GetQuery().Fetch() == S_OK )
		{
			rrIDs.push_back( nReportResultID );
		}
		
		for( iter = rrIDs.begin(); iter != rrIDs.end(); iter++ )
		{
			GetQuery().Reset( true );
			BINDPARAM_LONG( GetQuery(), *iter );
			GetQuery().Execute( _T("DELETE FROM ReportResultRows WHERE ReportResultID=?") );

			//Delete Report Files for this Report Result
			GetQuery().Reset( true );
			BINDPARAM_LONG( GetQuery(), *iter );
			BINDCOL_TCHAR( GetQuery(), trr.m_ResultFile );
			GetQuery().Execute( _T("SELECT ResultFile FROM ReportResults WHERE ReportResultID=?") );
			
			if ( GetQuery().Fetch() == S_OK )
			{
				sReportFileName.Format( _T("%s\\%s"), sTempPath.c_str(), trr.m_ResultFile );
				DeleteFile ( sReportFileName.c_str() );
			}
		}

		GetQuery().Reset( true );
		BINDPARAM_LONG( GetQuery(), m_ScheduledReportID  );
		GetQuery().Execute( _T("DELETE FROM ReportResults WHERE ScheduledReportID =?") );
				
		TScheduleReport::Delete(GetQuery());
	}
	
	return 0;
}

void CScheduleReport::CheckForDuplicity( void )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_ScheduledReportID );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT ScheduledReportID FROM ReportScheduled ")
						_T("WHERE ScheduledReportID<>? AND Description=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("A Scheduled Report with that description already exists, please enter a unique Description!")  );
}



////////////////////////////////////////////////////////////////////////////////
// 
// Create a new, blank scheduled report
// 
////////////////////////////////////////////////////////////////////////////////
int CScheduleReport::New()
{
	DecodeForm();

	//Check for unique Description
	CheckForDuplicity();

	m_CreatedByID = GetAgentID();
	m_EditedByID = GetAgentID();
	TScheduleReport::Insert( GetQuery() );

	return 0;
}

void CScheduleReport::DecodeForm(void)
{
	bool getTb = false;
	bool getAgents = false;
	bool getTc = false;
	bool getSr = false;
	bool getRr = false;
	bool getMs = false;
	bool getTs = false;
	bool getFlag1 = false;
	bool getFlag2 = false;
	string nAmPm;
		
	// Description
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, 256 );
	if ( _tcslen( m_Description ) < 1 )
		THROW_VALIDATION_EXCEPTION( _T("Description"), _T("Please specify a description for the report") );

	// Enabled?
	GetISAPIData().GetXMLLong( _T("IsEnabled"), m_IsEnabled );

	// OwnerID
	GetISAPIData().GetXMLLong( _T("OwnerID"), m_OwnerID, true );

	//Report Type
	GetISAPIData().GetXMLLong( _T("ReportID"), m_ReportID );

	TStandardReport sr;
	sr.m_StandardReportID = m_ReportID;
	sr.Query(GetQuery());
	
	switch( sr.m_ReportTypeID )
	{
	case 1:
		switch( sr.m_ReportObjectID )
		{
		case 1:
				getTb = true;
			break;
		case 2:
				getAgents = true;
			break;
		case 3:
				getTc = true;
			break;
		case 4:
				getRr = true;
			break;
		case 5:
				getMs = true;
			break;
		}
		break;
	case 2: 
		getFlag1 = true;
		switch( sr.m_ReportObjectID )
		{
		case 1:
				getTb = true;					
			break;
		case 2:
				getAgents = true;					
			break;
		case 3:
				getTc = true;
			break;			
		}
		break;
	case 3: 
		switch( sr.m_ReportObjectID )
		{
		case 1:
				getTb = true;					
			break;
		case 2:
				getAgents = true;					
			break;
		case 3:
				getTc = true;
			break;			
		}
		break;
	case 4: 
		getTs = true;
		getFlag1 = true;
		switch( sr.m_ReportObjectID )
		{
		case 3:
				getFlag2 = true;					
			break;
		case 6:
				getFlag2 = true;					
			break;						
		}
		break;
	case 5: 
		getFlag1 = true;
		getFlag2 = true;
		switch( sr.m_ReportObjectID )
		{
		case 7:
				getSr = true;					
			break;			
		}
		break;		
	case 6: 
		getFlag1 = true;
		switch( sr.m_ReportObjectID )
		{
		case 1:
				getTb = true;					
			break;
		case 2:
				getAgents = true;					
			break;
		case 3:
				getTc = true;
			break;			
		}
		break;		
	case 7: 
		getFlag1 = true;
		switch( sr.m_ReportObjectID )
		{
		case 1:
				getTb = true;					
			break;
		case 2:
				getAgents = true;					
			break;
		case 3:
				getTc = true;
			break;			
		}
		break;		
	case 8: 
		getTb = true;
		break;
	case 10:
		getFlag1 = true;
		getTb = true;
		break;
	}		
	
	
	if( getTb )
	{
		GetISAPIData().GetXMLLong( _T("TbTargetID"), m_TargetID, true );
	}
	if( getAgents )
	{
		GetISAPIData().GetXMLLong( _T("AgentsTargetID"), m_TargetID, true );
	}
	if( getTc )
	{
		GetISAPIData().GetXMLLong( _T("TcTargetID"), m_TargetID, true );
	}
	if( getSr )
	{
		GetISAPIData().GetXMLLong( _T("SrTargetID"), m_TargetID, true );
	}
	if( getRr )
	{
		GetISAPIData().GetXMLLong( _T("RrTargetID"), m_TargetID, true );
	}
	if( getMs )
	{
		GetISAPIData().GetXMLLong( _T("MsTargetID"), m_TargetID, true );
	}
	if( getTs )
	{
		GetISAPIData().GetXMLLong( _T("TsTargetID"), m_TicketStateID, true );
	}
	if( getFlag1 )
	{
		int nFlag1;
		GetISAPIData().GetXMLLong( _T("Flag1"), nFlag1, true );
		if(nFlag1==1)
		{
			m_Flag1 = 1;
		}
	}
	if( getFlag2 )
	{
		int nFlag2;
		GetISAPIData().GetXMLLong( _T("Flag2"), nFlag2, true );
		if(nFlag2==1)
		{
			m_Flag2 = 1;
		}
	}
	
	GetISAPIData().GetXMLLong( _T("RunFreq"), m_RunFreq );

	if( m_RunFreq == 0 )
	{
		GetISAPIData().GetXMLLong( _T("runsun"), m_RunSun );
		GetISAPIData().GetXMLLong( _T("runmon"), m_RunMon );
		GetISAPIData().GetXMLLong( _T("runtue"), m_RunTue );
		GetISAPIData().GetXMLLong( _T("runwed"), m_RunWed );
		GetISAPIData().GetXMLLong( _T("runthur"), m_RunThur );
		GetISAPIData().GetXMLLong( _T("runfri"), m_RunFri );
		GetISAPIData().GetXMLLong( _T("runsat"), m_RunSat );
	}
	else
	{
		GetISAPIData().GetXMLLong( _T("RunOn"), m_RunOn );
	}
	
	GetISAPIData().GetXMLString( _T("RunAtAmPm"), nAmPm );
	GetISAPIData().GetXMLLong( _T("RunAtHour"), m_RunAtHour );
	GetISAPIData().GetXMLLong( _T("RunAtMin"), m_RunAtMin );

	if ( nAmPm == "AM" )
	{
		if ( m_RunAtHour == 12 )
		{
			m_RunAtHour = 0;
		}
	}
	else
	{
		if ( m_RunAtHour != 12 )
		{
			m_RunAtHour = m_RunAtHour + 12;
		}
	}

	GetISAPIData().GetXMLLong( _T("SumPeriod"), m_SumPeriod );
	GetISAPIData().GetXMLLong( _T("MaxResults"), m_MaxResults );
	GetISAPIData().GetXMLLong( _T("SendAlertToOwner"), m_SendAlertToOwner );
	GetISAPIData().GetXMLLong( _T("SendResultToEmail"), m_SendResultToEmail );
	GetISAPIData().GetXMLLong( _T("IncludeResultFile"), m_IncludeResultFile );
	GetISAPIData().GetXMLLong( _T("SaveResultToFile"), m_SaveResultToFile );
	GetISAPIData().GetXMLTCHAR( _T("ResultEmailTo"), m_ResultEmailTo, 256 );
	//GetISAPIData().GetXMLLong( _T("KeepNumResultFile"), m_KeepNumResultFile );
	
	SetNextRunTime();
}

void CScheduleReport::GenerateXML(void)
{
	CEMSString sDate;
	CEMSString nLastResult;
	string nAmPm;
	

	if ( m_RunAtHour == 12 )
	{
		nAmPm = "PM";
	}
	else if ( m_RunAtHour == 0 )
	{
		m_RunAtHour = 12;
		nAmPm = "AM";
	}
	else if ( m_RunAtHour > 12 )
	{
		m_RunAtHour = m_RunAtHour - 12;
		nAmPm = "PM";
	}
	else
	{
		nAmPm = "AM";
	}

	GetXMLGen().AddChildElem( _T("Report") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_ScheduledReportID );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetDateTimeString( m_LastRunTime, m_LastRunTimeLen, sDate );
	GetXMLGen().AddChildAttrib( _T("LastRan"), sDate.c_str() );
	GetXMLGen().AddChildAttrib( _T("LastResult"), nLastResult.c_str() );
	GetDateTimeString( m_NextRunTime, m_NextRunTimeLen, sDate );
	GetXMLGen().AddChildAttrib( _T("NextRun"), sDate.c_str() );
	GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDate );
	GetXMLGen().AddChildAttrib( _T("DateCreated"), sDate.c_str() );
	GetDateTimeString( m_DateEdited, m_DateEditedLen, sDate );
	GetXMLGen().AddChildAttrib( _T("DateEdited"), sDate.c_str() );
	GetXMLGen().AddChildAttrib( _T("CreatedByID"), m_CreatedByID );
	GetXMLGen().AddChildAttrib( _T("EditedByID"), m_EditedByID );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	GetXMLGen().AddChildAttrib( _T("OwnerID"), m_OwnerID );
	GetXMLGen().AddChildAttrib( _T("ReportID"), m_ReportID );
	GetXMLGen().AddChildAttrib( _T("RunFreq"), m_RunFreq );
	GetXMLGen().AddChildAttrib( _T("RunOn"), m_RunOn );
	GetXMLGen().AddChildAttrib( _T("RunSun"), m_RunSun );
	GetXMLGen().AddChildAttrib( _T("RunMon"), m_RunMon );
	GetXMLGen().AddChildAttrib( _T("RunTue"), m_RunTue );
	GetXMLGen().AddChildAttrib( _T("RunWed"), m_RunWed );
	GetXMLGen().AddChildAttrib( _T("RunThur"), m_RunThur );
	GetXMLGen().AddChildAttrib( _T("RunFri"), m_RunFri );
	GetXMLGen().AddChildAttrib( _T("RunSat"), m_RunSat );
	GetXMLGen().AddChildAttrib( _T("RunAtHour"), m_RunAtHour );
	GetXMLGen().AddChildAttrib( _T("RunAtMin"), m_RunAtMin );
	GetXMLGen().AddChildAttrib( _T("RunAtAmPm"), nAmPm.c_str() );
	GetXMLGen().AddChildAttrib( _T("SumPeriod"), m_SumPeriod );
	GetXMLGen().AddChildAttrib( _T("MaxResults"), m_MaxResults );
	GetXMLGen().AddChildAttrib( _T("SendAlertToOwner"), m_SendAlertToOwner );
	GetXMLGen().AddChildAttrib( _T("SendResultToEmail"), m_SendResultToEmail );
	GetXMLGen().AddChildAttrib( _T("ResultEmailTo"), m_ResultEmailTo );
	GetXMLGen().AddChildAttrib( _T("IncludeResultFile"), m_IncludeResultFile );
	GetXMLGen().AddChildAttrib( _T("SaveResultToFile"), m_SaveResultToFile );
	//GetXMLGen().AddChildAttrib( _T("KeepNumResultFile"), m_KeepNumResultFile );
	GetXMLGen().AddChildAttrib( _T("TargetID"), m_TargetID );
	GetXMLGen().AddChildAttrib( _T("Flag1"), m_Flag1 );
	GetXMLGen().AddChildAttrib( _T("Flag2"), m_Flag2 );
	GetXMLGen().AddChildAttrib( _T("TicketStateID"), m_TicketStateID );
	GetXMLGen().AddChildAttrib( _T("IsAdminView"), m_isAdminView );
	
	TStandardReport sr;
	GetXMLGen().AddChildElem( _T("StandardReports") );
	GetXMLGen().IntoElem();
	sr.PrepareList(GetQuery());
	while( GetQuery().Fetch() == S_OK )
	{
		if ( sr.m_CanSchedule == 1 )
		{
			GetXMLGen().AddChildElem( _T("Report") );
			GetXMLGen().AddChildAttrib( _T("ID"), sr.m_StandardReportID );
			GetXMLGen().AddChildAttrib( _T("Description"), sr.m_Description );
			GetXMLGen().AddChildAttrib( _T("ReportTypeID"), sr.m_ReportTypeID );
			GetXMLGen().AddChildAttrib( _T("ReportObjectID"), sr.m_ReportObjectID );
		}
	}
	GetXMLGen().OutOfElem();

	// add in the lists
	ListEnabledAgentNames();
	AddTicketStates();
	AddAgents();
	AddTicketBoxes();
	AddRoutingRules();
	AddStdCategories();
	AddMsgSources();
	AddTicketCategories();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a list of
||				ticket states to the report.              
\*--------------------------------------------------------------------------*/
void CScheduleReport::AddTicketStates()
{
	GetXMLGen().AddChildElem( _T("TicketStates") );
	GetXMLGen().IntoElem();
		
	map<unsigned int, XTicketStates> tktstatemap;
	map<unsigned int, XTicketStates>::iterator map_iter;
	
	GetXMLCache().m_TicketStates.GetMap(tktstatemap);

	
	GetXMLGen().AddChildElem( _T("TicketState") );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("-- ALL --" ) );
	GetXMLGen().AddChildAttrib( _T("ID"), EMS_REPORT_ALL );
	
	for ( map_iter = tktstatemap.begin(); map_iter != tktstatemap.end(); map_iter++ )
	{
		GetXMLGen().AddChildElem( _T("TicketState") );
		GetXMLGen().AddChildAttrib( _T("Description"), map_iter->second.m_Description );
		GetXMLGen().AddChildAttrib( _T("ID"), map_iter->second.ID() );		
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a list of
||				Enabled Agents to the report.              
\*--------------------------------------------------------------------------*/
void CScheduleReport::AddAgents()
{
	CEMSString sName;

	GetXMLGen().AddChildElem( _T("Agents") );
	GetXMLGen().IntoElem();
	
	list<unsigned int>::iterator iter;
	XAgentNames AgentName;

	// only members of the administrators group can run 
	// reports on all items
	
	if ( GetIsAdmin() )
	{
		GetXMLGen().AddChildElem( _T("Agent") );
		GetXMLGen().AddChildAttrib( _T("Description"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("ID"), EMS_REPORT_ALL );
	}
	
	for( iter = GetXMLCache().m_AgentIDs.GetList().begin(); 
	     iter != GetXMLCache().m_AgentIDs.GetList().end(); iter++ )
	{
		if ( GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, *iter ) >= EMS_READ_ACCESS )
		{
			GetXMLCache().m_AgentNames.Query( *iter, AgentName );
			sName = AgentName.m_Name;
			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("Agent") );
			GetXMLGen().AddChildAttrib( _T("Description"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("ID"), AgentName.m_AgentID );
		}
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a list of
||				ticket boxes to the report.              
\*--------------------------------------------------------------------------*/
void CScheduleReport::AddTicketBoxes()
{
	CEMSString sName;

	GetXMLGen().AddChildElem( _T("TicketBoxes") );
	GetXMLGen().IntoElem();
	
	list<unsigned int>::iterator iter;
	XTicketBoxNames TicketBoxName;

	// only members of the administrators group can run 
	// reports on all items
	if ( GetIsAdmin() )
	{
		GetXMLGen().AddChildElem( _T("TicketBox") );
		GetXMLGen().AddChildAttrib( _T("Description"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("ID"), EMS_REPORT_ALL );
	}
	
	for( iter = GetXMLCache().m_TicketBoxIDs.GetList().begin(); 
	     iter != GetXMLCache().m_TicketBoxIDs.GetList().end(); iter++ )
	{
		if ( GetAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, *iter ) >= EMS_READ_ACCESS )
		{
			GetXMLCache().m_TicketBoxNames.Query( *iter, TicketBoxName );
			sName = TicketBoxName.m_Name;
			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("TicketBox") );
			GetXMLGen().AddChildAttrib( _T("Description"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("ID"), TicketBoxName.m_TicketBoxID );
		}
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a list of
||				routing rules to the report.              
\*--------------------------------------------------------------------------*/
void CScheduleReport::AddRoutingRules()
{
	TRoutingRules RoutingRule;
	CEMSString sName;
	
	GetXMLGen().AddChildElem( _T("RoutingRules") );
	GetXMLGen().IntoElem();

	
	if ( GetIsAdmin() )
	{
		GetXMLGen().AddChildElem( _T("RoutingRule") );
		GetXMLGen().AddChildAttrib( _T("Description"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("ID"), EMS_REPORT_ALL );
	}
		
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), RoutingRule.m_RoutingRuleID );
		BINDCOL_TCHAR( GetQuery(), RoutingRule.m_RuleDescrip );
		GetQuery().Execute(	_T("SELECT RoutingRuleID,RuleDescrip FROM RoutingRules ORDER BY RuleDescrip") );

		while ( GetQuery().Fetch() == S_OK )
		{
			sName = RoutingRule.m_RuleDescrip;
			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("RoutingRule") );
			GetXMLGen().AddChildAttrib( _T("Description"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("ID"), RoutingRule.m_RoutingRuleID );
		}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a list of
||				standard response categories to the report.              
\*--------------------------------------------------------------------------*/
void CScheduleReport::AddStdCategories()
{
	TStdResponseCategories StdCategory;
	CEMSString sName;
	
	GetXMLGen().AddChildElem( _T("ResponseCategories") );
	GetXMLGen().IntoElem();
	
		
	if ( GetIsAdmin() )
	{
		GetXMLGen().AddChildElem( _T("Category") );
		GetXMLGen().AddChildAttrib( _T("Description"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("ID"), EMS_REPORT_ALL );
	}
		
		GetXMLGen().AddChildElem( _T("Category") );
		GetXMLGen().AddChildAttrib( _T("Description"), _T("Uncategorized")  );
		GetXMLGen().AddChildAttrib( _T("ID"), EMS_STDRESP_UNCATEGORIZED );

		GetXMLGen().AddChildElem( _T("Category") );
		GetXMLGen().AddChildAttrib( _T("Description"), _T("Drafts")  );
		GetXMLGen().AddChildAttrib( _T("ID"), EMS_STDRESP_DRAFTS );
		
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), StdCategory.m_StdResponseCatID );
		BINDCOL_TCHAR( GetQuery(), StdCategory.m_CategoryName );
		GetQuery().Execute(	_T("SELECT StdResponseCatID,CategoryName ")
			                _T("FROM StdResponseCategories WHERE StdResponseCatID>1 ORDER BY CategoryName") );

		while ( GetQuery().Fetch() == S_OK )
		{
			sName = StdCategory.m_CategoryName;
			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("Category") );
			GetXMLGen().AddChildAttrib( _T("Description"), sName.c_str()  );
			GetXMLGen().AddChildAttrib( _T("ID"), StdCategory.m_StdResponseCatID );
		}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a list of
||				message sources to the report.              
\*--------------------------------------------------------------------------*/
void CScheduleReport::AddMsgSources()
{
	TMessageSources MsgSource;
	CEMSString sName;
	
	GetXMLGen().AddChildElem( _T("MessageSources") );
	GetXMLGen().IntoElem();
	if ( GetIsAdmin() )
	{
		GetXMLGen().AddChildElem( _T("MessageSource") );
		GetXMLGen().AddChildAttrib( _T("Description"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("ID"), EMS_REPORT_ALL );
	}
		
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), MsgSource.m_MessageSourceID );
		BINDCOL_TCHAR( GetQuery(), MsgSource.m_Description );
		BINDCOL_TCHAR( GetQuery(), MsgSource.m_RemoteAddress );
		BINDCOL_TCHAR( GetQuery(), MsgSource.m_AuthUserName );
		BINDCOL_LONG( GetQuery(), MsgSource.m_RemotePort );

		GetQuery().Execute(	_T("SELECT MessageSourceID,Description,RemoteAddress,AuthUserName,RemotePort ")
			                _T("FROM MessageSources ORDER BY Description,RemoteAddress,AuthUserName") );

		while ( GetQuery().Fetch() == S_OK )
		{
			if( MsgSource.m_DescriptionLen == 0 )
			{
				sName.Format( _T("%s%s%s, PORT=%d"), MsgSource.m_RemoteAddress, 
			                  MsgSource.m_AuthUserNameLen ? _T(", USER=") : _T(""), 
							  MsgSource.m_AuthUserName, MsgSource.m_RemotePort );
			}
			else
			{
				sName = MsgSource.m_Description;
			}

			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("MessageSource") );
			GetXMLGen().AddChildAttrib( _T("Description"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("ID"), MsgSource.m_MessageSourceID );
		}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a list of
||				ticket categories to the report.              
\*--------------------------------------------------------------------------*/
void CScheduleReport::AddTicketCategories()
{
	TTicketCategories TicketCategory;
	CEMSString sName;
	
	GetXMLGen().AddChildElem( _T("TicketCategories") );
	GetXMLGen().IntoElem();
	if ( GetIsAdmin() )
	{
		GetXMLGen().AddChildElem( _T("TicketCategory") );
		GetXMLGen().AddChildAttrib( _T("Description"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("ID"), EMS_REPORT_ALL );
	}
		
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), TicketCategory.m_TicketCategoryID );
		BINDCOL_TCHAR( GetQuery(), TicketCategory.m_Description );
		
		GetQuery().Execute(	_T("SELECT TicketCategoryID,Description ")
			                _T("FROM TicketCategories ORDER BY Description") );

		while ( GetQuery().Fetch() == S_OK )
		{
			sName = TicketCategory.m_Description;
			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("TicketCategory") );
			GetXMLGen().AddChildAttrib( _T("Description"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("ID"), TicketCategory.m_TicketCategoryID );
		}

	GetXMLGen().OutOfElem();
}
int CScheduleReport::GetDayOfWeek( int month, int day, int year )
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

void CScheduleReport::SetNextRunTime()
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );
	int nDayOfWeek = GetDayOfWeek( Now.month, Now.day, Now.year );
	int xTime = 0;
	time_t now;
	time( &now ); 
	TIMESTAMP_STRUCT m_DateTime;
				
	if(m_RunFreq==0)
	{
		switch(nDayOfWeek)
		{
		case 0:
			if(m_RunSun)
			{
				if(m_RunAtHour<=Now.hour)
				{
					if(m_RunMon){xTime=86400;}
					else if(m_RunTue){xTime=86400*2;}
					else if(m_RunWed){xTime=86400*3;}
					else if(m_RunThur){xTime=86400*4;}
					else if(m_RunFri){xTime=86400*5;}
					else if(m_RunSat){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(m_RunMon){xTime=86400;}
				else if(m_RunTue){xTime=86400*2;}
				else if(m_RunWed){xTime=86400*3;}
				else if(m_RunThur){xTime=86400*4;}
				else if(m_RunFri){xTime=86400*5;}
				else if(m_RunSat){xTime=86400*6;}
				else {xTime=86400*7;}	
			}
			break;
		case 1:
			if(m_RunMon)
			{
				if(m_RunAtHour<=Now.hour)
				{
					if(m_RunTue){xTime=86400;}
					else if(m_RunWed){xTime=86400*2;}
					else if(m_RunThur){xTime=86400*3;}
					else if(m_RunFri){xTime=86400*4;}
					else if(m_RunSat){xTime=86400*5;}
					else if(m_RunSun){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(m_RunTue){xTime=86400;}
				else if(m_RunWed){xTime=86400*2;}
				else if(m_RunThur){xTime=86400*3;}
				else if(m_RunFri){xTime=86400*4;}
				else if(m_RunSat){xTime=86400*5;}
				else if(m_RunSun){xTime=86400*6;}
				else {xTime=86400*7;}
			}
			break;
		case 2:
			if(m_RunTue)
			{
				if(m_RunAtHour<=Now.hour)
				{
					if(m_RunWed){xTime=86400;}
					else if(m_RunThur){xTime=86400*2;}
					else if(m_RunFri){xTime=86400*3;}
					else if(m_RunSat){xTime=86400*4;}
					else if(m_RunSun){xTime=86400*5;}
					else if(m_RunMon){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(m_RunWed){xTime=86400;}
				else if(m_RunThur){xTime=86400*2;}
				else if(m_RunFri){xTime=86400*3;}
				else if(m_RunSat){xTime=86400*4;}
				else if(m_RunSun){xTime=86400*5;}
				else if(m_RunMon){xTime=86400*6;}
				else {xTime=86400*7;}	
			}
			break;
		case 3:
			if(m_RunWed)
			{
				if(m_RunAtHour<=Now.hour)
				{
					if(m_RunThur){xTime=86400;}
					else if(m_RunFri){xTime=86400*2;}
					else if(m_RunSat){xTime=86400*3;}
					else if(m_RunSun){xTime=86400*4;}
					else if(m_RunMon){xTime=86400*5;}
					else if(m_RunTue){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(m_RunThur){xTime=86400;}
				else if(m_RunFri){xTime=86400*2;}
				else if(m_RunSat){xTime=86400*3;}
				else if(m_RunSun){xTime=86400*4;}
				else if(m_RunMon){xTime=86400*5;}
				else if(m_RunTue){xTime=86400*6;}
				else {xTime=86400*7;}	
			}
			break;
		case 4:
			if(m_RunThur)
			{
				if(m_RunAtHour<=Now.hour)
				{
					if(m_RunFri){xTime=86400;}
					else if(m_RunSat){xTime=86400*2;}
					else if(m_RunSun){xTime=86400*3;}
					else if(m_RunMon){xTime=86400*4;}
					else if(m_RunTue){xTime=86400*5;}
					else if(m_RunWed){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(m_RunFri){xTime=86400;}
				else if(m_RunSat){xTime=86400*2;}
				else if(m_RunSun){xTime=86400*3;}
				else if(m_RunMon){xTime=86400*4;}
				else if(m_RunTue){xTime=86400*5;}
				else if(m_RunWed){xTime=86400*6;}
				else {xTime=86400*7;}
			}
			break;
		case 5:
			if(m_RunFri)
			{
				if(m_RunAtHour<=Now.hour)
				{
					if(m_RunSat){xTime=86400;}
					else if(m_RunSun){xTime=86400*2;}
					else if(m_RunMon){xTime=86400*3;}
					else if(m_RunTue){xTime=86400*4;}
					else if(m_RunWed){xTime=86400*5;}
					else if(m_RunThur){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(m_RunSat){xTime=86400;}
				else if(m_RunSun){xTime=86400*2;}
				else if(m_RunMon){xTime=86400*3;}
				else if(m_RunTue){xTime=86400*4;}
				else if(m_RunWed){xTime=86400*5;}
				else if(m_RunThur){xTime=86400*6;}
				else {xTime=86400*7;}	
			}
			break;
		case 6:
			if(m_RunSat)
			{
				if(m_RunAtHour<=Now.hour)
				{
					if(m_RunSun){xTime=86400;}
					else if(m_RunMon){xTime=86400*2;}
					else if(m_RunTue){xTime=86400*3;}
					else if(m_RunWed){xTime=86400*4;}
					else if(m_RunThur){xTime=86400*5;}
					else if(m_RunFri){xTime=86400*6;}
					else {xTime=86400*7;}					 
				}
				else{xTime=0;}
			}
			else
			{
				if(m_RunSun){xTime=86400;}
				else if(m_RunMon){xTime=86400*2;}
				else if(m_RunTue){xTime=86400*3;}
				else if(m_RunWed){xTime=86400*4;}
				else if(m_RunThur){xTime=86400*5;}
				else if(m_RunFri){xTime=86400*6;}
				else {xTime=86400*7;}
			}
			break;
		}
		SecondsToTimeStamp(now + xTime, m_DateTime);
		m_nYear = m_DateTime.year;
		m_nMonth = m_DateTime.month;
		m_nDay = m_DateTime.day;		
	}
	else if ( m_RunFreq == 1 )
	{
		if ( m_RunOn < nDayOfWeek )
		{
			xTime = (7-(nDayOfWeek-m_RunOn))*86400;			
		}
		else if ( m_RunOn > nDayOfWeek )
		{
			xTime = (m_RunOn-nDayOfWeek)*86400;			
		}
		else
		{
			if ( m_RunAtHour <= Now.hour )
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
	else if ( m_RunFreq == 2 )
	{
		m_nYear = Now.year;
		if ( m_RunOn == 0 )
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
		if ( m_RunOn == 0 )
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

	m_NextRunTime.year = m_nYear;
	m_NextRunTime.month = m_nMonth;
	m_NextRunTime.day = m_nDay;
	m_NextRunTime.hour = m_RunAtHour;
	m_NextRunTime.minute = m_RunAtMin;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetWhenSelect
// 
////////////////////////////////////////////////////////////////////////////////
void CScheduleReport::GetWhenSelect( CEMSString& sDatePart, TCHAR* szColumn, bool bBind )
{
	switch( m_ReportInterval )
	{
	case 0:	// Hour
		sDatePart.Format( _T("DATEPART(year,%s),DATEPART(month,%s),DATEPART(day,%s),DATEPART(hour,%s)"), 
			              szColumn, szColumn, szColumn, szColumn );

		if( bBind )
		{
			BINDCOL_LONG_NOLEN( GetQuery(), m_nYear );
			BINDCOL_LONG_NOLEN( GetQuery(), m_nMonth );
			BINDCOL_LONG_NOLEN( GetQuery(), m_nDay );
			BINDCOL_LONG_NOLEN( GetQuery(), m_nHour );
		}
		break;
	
	case 1:	// Day

		if ( m_OutputCDF )
		{
			sDatePart.Format( _T("DATEPART(year,%s),DATEPART(month,%s),DATEPART(day,%s)") , 
							  szColumn, szColumn, szColumn );
		}
		else
		{
			sDatePart.Format( _T("DATEPART(year,%s),DATEPART(month,%s),DATEPART(day,%s),DATEPART(dw,%s)") , 
							  szColumn, szColumn, szColumn, szColumn );
		}

		if( bBind )
		{
			BINDCOL_LONG_NOLEN( GetQuery(), m_nYear );
			BINDCOL_LONG_NOLEN( GetQuery(), m_nMonth );
			BINDCOL_LONG_NOLEN( GetQuery(), m_nDay );

			if ( !m_OutputCDF )
				BINDCOL_LONG_NOLEN( GetQuery(), m_nDayofWeek );
		}
		break;

	case 2: // Week
		sDatePart.Format( _T("DATEPART(year,%s),DATEPART(week,%s)"), 
			              szColumn, szColumn );
		if( bBind )
		{
			BINDCOL_LONG_NOLEN( GetQuery(), m_nYear );
			BINDCOL_LONG_NOLEN( GetQuery(), m_nWeek );
		}
		break;

	case 3: // Month
		sDatePart.Format( _T("DATEPART(year,%s),DATEPART(month,%s)"), 
			              szColumn, szColumn );
		if( bBind )
		{
			BINDCOL_LONG_NOLEN( GetQuery(), m_nYear );
			BINDCOL_LONG_NOLEN( GetQuery(), m_nMonth );
		}
		break;

	case 4: // Year
		sDatePart.Format( _T("DATEPART(year,%s)"), 
			              szColumn );
		if( bBind )
		{
			BINDCOL_LONG_NOLEN( GetQuery(), m_nYear );
		}
		break;
	}
}

void CScheduleReport::GetWeekDayName( tstring& sName, int nDay )
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
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetDateEquality
// 
////////////////////////////////////////////////////////////////////////////////
void CScheduleReport::GetDateEquality( CEMSString& sDatePart, TCHAR* szColumnA, TCHAR* szColumnB )
{
	switch( m_ReportInterval )
	{
	case 0:	// Hour
		sDatePart.Format( _T("DATEPART(year,%s)=DATEPART(year,%s) AND ")
			              _T("DATEPART(month,%s)=DATEPART(month,%s) AND ")
						  _T("DATEPART(day,%s)=DATEPART(day,%s) AND ")
						  _T("DATEPART(hour,%s)=DATEPART(hour,%s) "), 
			              szColumnA, szColumnB, szColumnA, szColumnB,
						  szColumnA, szColumnB, szColumnA, szColumnB );
		break;
	
	case 1:	// Day
		sDatePart.Format( _T("DATEPART(year,%s)=DATEPART(year,%s) AND ")
			              _T("DATEPART(month,%s)=DATEPART(month,%s) AND ")
						  _T("DATEPART(day,%s)=DATEPART(day,%s) "),
			              szColumnA, szColumnB, szColumnA, szColumnB,
						  szColumnA, szColumnB );
		break;

	case 2: // Week
		sDatePart.Format( _T("DATEPART(year,%s)=DATEPART(year,%s) AND ")
			              _T("DATEPART(week,%s)=DATEPART(week,%s) "),
			              szColumnA, szColumnB, szColumnA, szColumnB );
		break;

	case 3: // Month
		sDatePart.Format( _T("DATEPART(year,%s)=DATEPART(year,%s) AND ")
			              _T("DATEPART(month,%s)=DATEPART(month,%s) "),
			              szColumnA, szColumnB, szColumnA, szColumnB );
		break;

	case 4: // Year
		sDatePart.Format( _T("DATEPART(year,%s)=DATEPART(year,%s) "), 
			              szColumnA, szColumnB );
		break;

	default:
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );  
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetWhen
// 
////////////////////////////////////////////////////////////////////////////////
void CScheduleReport::GetWhen( CEMSString& sWhen )
{
	TCHAR* szMonths[] = { _T("January"), _T("February"), _T("March"), _T("April"), _T("May"),
		                  _T("June"), _T("July"), _T("August"), _T("September"), _T("October"),
						  _T("November"), _T("December") };

	TCHAR* szDays[] = { _T("Sunday"), _T("Monday"), _T("Tuesday"), _T("Wednesday"), _T("Thursday"),
		                _T("Friday"), _T("Saturday") };

	switch( m_ReportInterval )
	{
	case 0:
		sWhen.Format( _T("%d/%d/%d %2d:00"), m_nMonth, m_nDay, m_nYear, m_nHour );
		break;

	case 1:	// Day

		if ( m_OutputCDF )
		{
			sWhen.Format( _T("%d/%d/%d"), m_nMonth, m_nDay, m_nYear );
		}
		else
		{
			sWhen.Format( _T("%d/%d/%d - %s"), m_nMonth, m_nDay, m_nYear, szDays[m_nDayofWeek-1] );
		}
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

int CScheduleReport::GetLastDayOfMonth( int month )
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
////////////////////////////////////////////////////////////////////////////////
// 
// OpenCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CScheduleReport::OpenCDF( CURLAction& action )
{
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
void CScheduleReport::OutputCDF( tstring& sLine )
{
	DWORD dwBytes;

	WriteFile( m_hCDFFile, sLine.c_str(), sLine.size(), &dwBytes, NULL );
}

////////////////////////////////////////////////////////////////////////////////
// 
// CloseCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CScheduleReport::CloseCDF(void)
{
	if( m_OutputCDF )
		CloseHandle( m_hCDFFile );
}
