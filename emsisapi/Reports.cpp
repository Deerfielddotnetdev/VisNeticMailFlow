/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Reports.cpp,v 1.2.2.1 2005/12/13 18:11:49 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Reporting Base Class  
||              
\\*************************************************************************/

#include "stdafx.h"
#include "Reports.h"
#include "DateFns.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CReports::CReports( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	// check security
	RequireAgentRightLevel( EMS_OBJECT_TYPE_STANDARD_REPORTS, 0, EMS_READ_ACCESS );

	m_hCDFFile = INVALID_HANDLE_VALUE;
	m_MaxRows = 250;

	m_OutputCDF = false;
}

CReports::~CReports()
{
}

/*---------------------------------------------------------------------------\            
||  Comments:	Returns the color that corrosponds to the graph for
||				the row index.
\*--------------------------------------------------------------------------*/
LPCTSTR CReports::GetGraphColor(int nRowIndex )
{
	LPTSTR ColorArray[]= {"#FF9966", "#C2A00E", "#71A214", "#3399CC", 
						  "#CC99FF", "#FF9999", "#00CC99", "#A00000", 
						  "#0000A0", "#FFFF00", "#00FF00", "#FF0000", 
						  "#0000FF", "#800080", "#FFA000", "#00FFFF", 
						  "#008080", "#FF00A0", "#CCCCCC"};
	
	return ColorArray[nRowIndex % 19];
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				summary periods to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddDateIntervals( LPCTSTR szLable, int nSelected /*=0*/ )
{
	LPTSTR SummaryPeroids[]= { _T("Hour"),
							   _T("Day"),
							   _T("Week"),
							   _T("Month"),
							 //_T("Quarter"),
							   _T("Year"),
							   NULL };

	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("SUMMARY_PERIOD_SEL") );
	
	GetXMLGen().IntoElem();
	
	GetXMLGen().AddChildElem( _T("LABLE"), _T(szLable) );
	
	for ( int i=0; SummaryPeroids[i] != NULL; i++ )
	{
		GetXMLGen().AddChildElem( _T("OPTION"), SummaryPeroids[i] );
		GetXMLGen().AddChildAttrib( _T("VALUE"), i );
		
		if ( i == nSelected )
			GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
	}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a date range control to the report, which allows the 
||				agent to select from a predefined date range or specify a 
||				custom date range.        
\*--------------------------------------------------------------------------*/
void CReports::AddDateRange( int nSelected /*=0*/ )
{
	LPTSTR DateRange[]= { _T("Last 30 Days"),
						  _T("Last 14 Days"),
						  _T("Last 7 Days"),
						  _T("Today"),
						  _T("Yesterday"),
						  _T("This Week"),
						  _T("Last Week"),
						  _T("This Month"),
						  _T("Last Month"),
						  _T("This Quarter"),
						  _T("Last Quarter"),
						  _T("This Year"),
						  _T("Last Year"),
						  _T("All Dates"),
						  NULL };
	
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("DATE_RANGE") );
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("LABLE"), _T("Date Range") );
	
	for ( int i=0; DateRange[i] != NULL; i++ )
	{
		GetXMLGen().AddChildElem( _T("OPTION"), DateRange[i] );
		GetXMLGen().AddChildAttrib( _T("VALUE"), i );
		
		if ( i == nSelected )
			GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				message types to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddMsgTypes( int nSelected /*=0*/ )
{
	LPTSTR MsgTypes[]= { _T("-- ALL --"),
						 _T("Inbound"),
						 _T("Outbound"),
						 NULL };
	
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("MSG_FILTER") );
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("LABLE"), _T("Message Type:") );
	
	for ( int i=0; MsgTypes[i] != NULL; i++ )
	{
		GetXMLGen().AddChildElem( _T("OPTION"), MsgTypes[i] );
		GetXMLGen().AddChildAttrib( _T("VALUE"), i );
		
		if ( i == nSelected )
			GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				audit action types to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddActionTypes( int nSelected /*=0*/ )
{
	LPTSTR ActionTypes[]= { _T("-- ALL --"),
							_T("Logged In"),
							_T("Logged Out"),
							_T("Deleted Ticket"),
							_T("Created Ticket"),
							_T("Moved Ticket"),
							_T("Escalated Ticket"),
							_T("Reassigned Ticket"),
							_T("Restored Ticket"),
							_T("Opened Ticket"),
							_T("Closed Ticket"),
							_T("Added Message"),
							_T("Deleted Message"),
							_T("NoShow"),
							_T("NoShow"),
							_T("NoShow"),
							_T("NoShow"),
							_T("NoShow"),
							_T("NoShow"),
							_T("NoShow"),
							_T("Revoked Message"),
							_T("Released Message"),
							_T("Returned Message"),
							_T("NoShow"),
							_T("NoShow"),
							_T("NoShow"),
							_T("Created Link"),
							_T("Deleted Link"),
							_T("Linked Ticket"),
							_T("Unlinked Ticket"),							
							NULL };
	
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("ACTION_FILTER") );
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("LABLE"), _T("Action Type:") );
	
	for ( int i=0; ActionTypes[i] != NULL; i++ )
	{
		if(ActionTypes[i] != "NoShow")
		{
			GetXMLGen().AddChildElem( _T("OPTION"), ActionTypes[i] );
			GetXMLGen().AddChildAttrib( _T("VALUE"), i );
		
			if ( i == nSelected )
				GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
		}
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				audit action types to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddSortBy( int nSelected /*=0*/ )
{
	LPTSTR SortBy[]= { _T("Date"),
							_T("Agent"),
							_T("Activity"),							
							NULL };
	
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("SORT_BY") );
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("LABLE"), _T("Sort Results By:") );
	
	for ( int i=0; SortBy[i] != NULL; i++ )
	{
		GetXMLGen().AddChildElem( _T("OPTION"), SortBy[i] );
		GetXMLGen().AddChildAttrib( _T("VALUE"), i );
		
		if ( i == nSelected )
			GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				audit action types to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddSortDirection( int nSelected /*=0*/ )
{
	LPTSTR SortDirection[]= { _T("Ascending"),
							_T("Descending"),							
							NULL };
	
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("SORT_DIRECTION") );
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("LABLE"), _T("Sort Direction:") );
	
	for ( int i=0; SortDirection[i] != NULL; i++ )
	{
		GetXMLGen().AddChildElem( _T("OPTION"), SortDirection[i] );
		GetXMLGen().AddChildAttrib( _T("VALUE"), i );
		
		if ( i == nSelected )
			GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
	}
	
	GetXMLGen().OutOfElem();
}


/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				ticket states to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddTicketStates( int nSelected /*=0*/, bool bSubmitOnChange /*=false*/ )
{
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("TICKET_STATE") );
	
	GetXMLGen().IntoElem();
	
	GetXMLGen().AddChildElem( _T("LABLE"), _T("Ticket State:") );
	
	if ( bSubmitOnChange )
	{
		GetXMLGen().AddChildElem( _T("EVENT"), _T("document.tabularForm.submit()")  );
		GetXMLGen().AddChildAttrib( _T("TYPE"), _T("onchange") );
	}

	GetXMLGen().AddChildElem( _T("OPTION"), _T("-- ALL --") );
	GetXMLGen().AddChildAttrib( _T("VALUE"), EMS_REPORT_ALL );
	
	map<unsigned int, XTicketStates> tktstatemap;
	map<unsigned int, XTicketStates>::iterator map_iter;
	
	GetXMLCache().m_TicketStates.GetMap(tktstatemap);
	
	for ( map_iter = tktstatemap.begin(); map_iter != tktstatemap.end(); map_iter++ )
	{
		GetXMLGen().AddChildElem( _T("OPTION"), map_iter->second.m_Description );
		GetXMLGen().AddChildAttrib( _T("VALUE"), map_iter->second.ID() );
		
		if ( map_iter->second.ID() == nSelected )
			GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				Agents to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddAgents( int nSelected /*=0*/ )
{
	CEMSString sName;

	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("AGENT") );
	
	GetXMLGen().IntoElem();
	
	GetXMLGen().AddChildElem( _T("LABLE"), _T("Agent:") );
	
	list<unsigned int>::iterator iter;
	XAgentNames AgentName;

	// only members of the administrators group can run 
	// reports on all items
	if ( GetIsAdmin() )
	{
		GetXMLGen().AddChildElem( _T("OPTION"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("VALUE"), EMS_REPORT_ALL );
	}
	
	for( iter = GetXMLCache().m_AgentIDs.GetList().begin(); 
	     iter != GetXMLCache().m_AgentIDs.GetList().end(); iter++ )
	{
		if ( GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, *iter ) >= EMS_READ_ACCESS )
		{
			GetXMLCache().m_AgentNames.Query( *iter, AgentName );
			sName = AgentName.m_Name;
			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("OPTION"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("VALUE"), AgentName.m_AgentID );

			if ( AgentName.m_AgentID == nSelected )
				GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
		}
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				Groups to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddGroups( int nSelected, bool bAddAll )
{
	TGroups Group;
	CEMSString sName;
	
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("Group") );

	GetXMLGen().IntoElem();
	
		GetXMLGen().AddChildElem( _T("LABLE"), _T("Group:") );

		if(bAddAll)
		{
			if ( GetIsAdmin() )
			{
				GetXMLGen().AddChildElem( _T("OPTION"), _T("-- ALL --" ) );
				GetXMLGen().AddChildAttrib( _T("VALUE"), EMS_REPORT_ALL );
			}
		}

		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(),Group.m_GroupID );
		BINDCOL_TCHAR( GetQuery(), Group.m_GroupName );
		
		GetQuery().Execute(	_T("SELECT GroupID,GroupName ")
			                _T("FROM Groups ORDER BY GroupName") );

		while ( GetQuery().Fetch() == S_OK )
		{
			sName = Group.m_GroupName;
			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("OPTION"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("VALUE"), Group.m_GroupID );

			if ( Group.m_GroupID  == nSelected )
				GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
		}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				ticket boxes to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddTicketBoxes( int nSelected, bool bAllowMulti )
{
	CEMSString sName;

	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("TICKETBOX") );
	
	GetXMLGen().IntoElem();
	
	GetXMLGen().AddChildElem( _T("LABLE"), _T("TicketBox:") );

	if ( bAllowMulti )
	{
		GetXMLGen().AddChildElem( _T("EVENT"), _T("true")  );
		GetXMLGen().AddChildAttrib( _T("TYPE"), _T("multiple") );
		GetXMLGen().AddChildElem( _T("EVENT"), _T("3")  );
		GetXMLGen().AddChildAttrib( _T("TYPE"), _T("size") );
	}
	
	list<unsigned int>::iterator iter;
	XTicketBoxNames TicketBoxName;

	// only members of the administrators group can run 
	// reports on all items
	if ( GetIsAdmin() )
	{
		GetXMLGen().AddChildElem( _T("OPTION"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("VALUE"), EMS_REPORT_ALL );
	}
	
	for( iter = GetXMLCache().m_TicketBoxIDs.GetList().begin(); 
	     iter != GetXMLCache().m_TicketBoxIDs.GetList().end(); iter++ )
	{
		if ( GetAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, *iter ) >= EMS_READ_ACCESS )
		{
			GetXMLCache().m_TicketBoxNames.Query( *iter, TicketBoxName );
			sName = TicketBoxName.m_Name;
			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("OPTION"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("VALUE"), TicketBoxName.m_TicketBoxID );

			if ( TicketBoxName.m_TicketBoxID == nSelected )
				GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
		}
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				routing rules to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddRoutingRules( int nSelected /*=0*/ )
{
	TRoutingRules RoutingRule;
	CEMSString sName;
	
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("ROUTING_RULE") );

	GetXMLGen().IntoElem();
	
		GetXMLGen().AddChildElem( _T("LABLE"), _T("Routing Rule:") );

		GetXMLGen().AddChildElem( _T("OPTION"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("VALUE"), EMS_REPORT_ALL );
		
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), RoutingRule.m_RoutingRuleID );
		BINDCOL_TCHAR( GetQuery(), RoutingRule.m_RuleDescrip );
		GetQuery().Execute(	_T("SELECT RoutingRuleID,RuleDescrip FROM RoutingRules ORDER BY RuleDescrip") );

		while ( GetQuery().Fetch() == S_OK )
		{
			sName = RoutingRule.m_RuleDescrip;
			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("OPTION"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("VALUE"), RoutingRule.m_RoutingRuleID );

			if ( RoutingRule.m_RoutingRuleID  == nSelected )
				GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
		}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				standard response categories to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddStdCategories( int nSelected )
{
	TStdResponseCategories StdCategory;
	CEMSString sName;
	
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("STDCAT") );

	GetXMLGen().IntoElem();
	
		GetXMLGen().AddChildElem( _T("LABLE"), _T("Category:") );

		GetXMLGen().AddChildElem( _T("OPTION"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("VALUE"), EMS_REPORT_ALL );
		
		GetXMLGen().AddChildElem( _T("OPTION"), _T("Uncategorized")  );
		GetXMLGen().AddChildAttrib( _T("VALUE"), EMS_STDRESP_UNCATEGORIZED );

		if ( nSelected == EMS_STDRESP_UNCATEGORIZED )
			GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );

		GetXMLGen().AddChildElem( _T("OPTION"), _T("Drafts")  );
		GetXMLGen().AddChildAttrib( _T("VALUE"), EMS_STDRESP_DRAFTS );
		
		if ( nSelected == EMS_STDRESP_DRAFTS )
			GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );

		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), StdCategory.m_StdResponseCatID );
		BINDCOL_TCHAR( GetQuery(), StdCategory.m_CategoryName );
		GetQuery().Execute(	_T("SELECT StdResponseCatID,CategoryName ")
			                _T("FROM StdResponseCategories WHERE StdResponseCatID>1 ORDER BY CategoryName") );

		while ( GetQuery().Fetch() == S_OK )
		{
			sName = StdCategory.m_CategoryName;
			sName.EscapeHTML();

			GetXMLGen().AddChildElem( _T("OPTION"), sName.c_str()  );
			GetXMLGen().AddChildAttrib( _T("VALUE"), StdCategory.m_StdResponseCatID );

			if ( StdCategory.m_StdResponseCatID == nSelected )
				GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
		}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				message sources to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddMsgSources( int nSelected /*=0*/ )
{
	TMessageSources MsgSource;
	CEMSString sName;
	
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("MSG_SOURCE") );

	GetXMLGen().IntoElem();
	
		GetXMLGen().AddChildElem( _T("LABLE"), _T("Message Source:") );

		GetXMLGen().AddChildElem( _T("OPTION"), _T("-- ALL --" ) );
		GetXMLGen().AddChildAttrib( _T("VALUE"), EMS_REPORT_ALL );
		
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

			GetXMLGen().AddChildElem( _T("OPTION"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("VALUE"), MsgSource.m_MessageSourceID );

			if ( MsgSource.m_MessageSourceID  == nSelected )
				GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
		}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds a drop down box containing a list of
||				ticket categories to the report.              
\*--------------------------------------------------------------------------*/
void CReports::AddTicketCategories( int nSelected /*=0*/ )
{
	TTicketCategories TicketCategory;
	CEMSString sName;
	
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("SELECT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("TICKET_CATEGORY") );

	GetXMLGen().IntoElem();
	
		GetXMLGen().AddChildElem( _T("LABLE"), _T("Ticket Category:") );

		if ( GetIsAdmin() )
		{
			GetXMLGen().AddChildElem( _T("OPTION"), _T("-- ALL --" ) );
			GetXMLGen().AddChildAttrib( _T("VALUE"), EMS_REPORT_ALL );
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

			GetXMLGen().AddChildElem( _T("OPTION"), sName.c_str() );
			GetXMLGen().AddChildAttrib( _T("VALUE"), TicketCategory.m_TicketCategoryID );

			if ( TicketCategory.m_TicketCategoryID  == nSelected )
				GetXMLGen().AddChildAttrib( _T("SELECTED"), TRUE );
		}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add max lines text box to the report	              
\*--------------------------------------------------------------------------*/
void CReports::AddMaxLines( int nMaxLines )
{
	CEMSString sLines;
	sLines.Format( _T("%d"), nMaxLines );
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("INPUT") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("MAX_LINES") );
	GetXMLGen().AddChildAttrib( _T("SIZE"), _T("4") );
	GetXMLGen().AddChildAttrib( _T("MAXLENGTH"), _T("4") );

	GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("LABLE"), _T("Max Lines:") );
		GetXMLGen().AddChildElem( _T("VALUE"), sLines.c_str() );
	GetXMLGen().OutOfElem();
}
	
////////////////////////////////////////////////////////////////////////////////
// 
// GetWhenSelect
// 
////////////////////////////////////////////////////////////////////////////////
void CReports::GetWhenSelect( CEMSString& sDatePart, TCHAR* szColumn, bool bBind )
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

void CReports::GetWeekDayName( tstring& sName, int nDay )
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
void CReports::GetDateEquality( CEMSString& sDatePart, TCHAR* szColumnA, TCHAR* szColumnB )
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
void CReports::GetWhen( CEMSString& sWhen )
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


////////////////////////////////////////////////////////////////////////////////
// 
// OpenCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CReports::OpenCDF( CURLAction& action )
{
	dca::String f;	
	DebugReporter::Instance().DisplayMessage("CReports::OpenCDF - Entering", DebugReporter::ISAPI, GetCurrentThreadId());
	
	// check security
	RequireAgentRightLevel( EMS_OBJECT_TYPE_STANDARD_REPORTS, 0, EMS_EDIT_ACCESS );
	
	TCHAR szTempPath[MAX_PATH];
	
	GetTempPath( MAX_PATH, szTempPath );
	GetTempFileName( szTempPath, _T("ems"), 0, m_szTempFile );
	
	f.Format("CReports::OpenCDF - Temp file set to [%s]", m_szTempFile);
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	m_hCDFFile = CreateFile( m_szTempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
			                 FILE_ATTRIBUTE_NORMAL, NULL );

	if( m_hCDFFile == INVALID_HANDLE_VALUE )
	{
		DebugReporter::Instance().DisplayMessage("CReports::OpenCDF - Error creating temp file", DebugReporter::ISAPI, GetCurrentThreadId());
		THROW_EMS_EXCEPTION( E_SystemError, _T("Error creating temp file") );
	}
	
	action.SetSendTempFile( m_szTempFile, _T("report.csv") );

	m_OutputCDF = true;

	DebugReporter::Instance().DisplayMessage("CReports::OpenCDF - Leaving", DebugReporter::ISAPI, GetCurrentThreadId());
}

////////////////////////////////////////////////////////////////////////////////
// 
// OutputCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CReports::OutputCDF( tstring& sLine )
{
	DWORD dwBytes;
	dca::String f;
	
	f.Format("CReports::OutputCDF - Write to file %s ", sLine.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	WriteFile( m_hCDFFile, sLine.c_str(), sLine.size(), &dwBytes, NULL );
}

////////////////////////////////////////////////////////////////////////////////
// 
// CloseCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CReports::CloseCDF(void)
{
	DebugReporter::Instance().DisplayMessage("CReports::CloseCDF - Entering", DebugReporter::ISAPI, GetCurrentThreadId());

	if( m_OutputCDF )
		CloseHandle( m_hCDFFile );

	DebugReporter::Instance().DisplayMessage("CReports::CloseCDF - Leaving", DebugReporter::ISAPI, GetCurrentThreadId());

}

CPrintReport::CPrintReport( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

int CPrintReport::Run( CURLAction& action )
{
	SYSTEMTIME st;
	CEMSString sTime;

	GetLocalTime( &st );
	GetDateTimeString( st, sTime );

	GetXMLGen().AddChildElem( _T("PrintReport") );
	GetXMLGen().AddChildAttrib( _T("Date"), sTime.c_str() );

	return 0;
}

