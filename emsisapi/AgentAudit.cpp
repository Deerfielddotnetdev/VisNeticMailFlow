/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/AgentAudit.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "AgentAudit.h"
#include "ContactFNS.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CAgentAudit::CAgentAudit( CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_nType = 0;
	m_nFilter = 0;
	m_nActionFilter = 0;
	m_nDateRange = 0;

	ZeroMemory( &m_StartDate, sizeof(m_StartDate) );
	ZeroMemory( &m_EndDate, sizeof(m_EndDate) );
	m_EndDate.hour = 23;
	m_EndDate.minute = 59;
	m_EndDate.second = 59;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main entry point	              
\*--------------------------------------------------------------------------*/
int CAgentAudit::Run( CURLAction& action )
{
	RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT_AUDIT, 0, EMS_DELETE_ACCESS );
	
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
			DoQuery();
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
void CAgentAudit::DecodeForm()
{
	int agID;
	int acID;

	GetISAPIData().GetURLLong( _T("type"), m_nType );
		
	ISAPIPARAM param;
	
	if ( GetISAPIData().FindURLParam( _T("MAX_LINES"), param ) )
	{
		GetISAPIData().GetURLLong( _T("AGENT"), m_nFilter, true );

		GetISAPIData().GetURLString( _T("AgentIDCollection"), agCollection, false );
		GetISAPIData().GetURLString( _T("ActionIDCollection"), acCollection, false );
		
		GetISAPIData().GetURLLong( _T("ACTION_FILTER"), m_nActionFilter );
		GetISAPIData().GetURLLong( _T("DATE_RANGE_SEL"), m_nDateRange, true );
		GetISAPIData().GetURLLong( _T("MAX_LINES"), m_MaxRows );
		GetISAPIData().GetURLLong( _T("SORT_BY"), m_nSortBy );
		GetISAPIData().GetURLLong( _T("SORT_DIRECTION"), m_nSortDirection );
		
		long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
		TIMESTAMP_STRUCT tsLocal;
		long tsLocalLen=0;

		CEMSString sDate;
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );

		if(lTzBias != 1)
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
		}
		
		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EndDate );

		if(lTzBias != 1)
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
		}

		agCollection.CDLInit();
		while( agCollection.CDLGetNextInt( agID ) )
		{
			if ( agID == -1 )
			{
				m_nFilter = -1;
			}
			else
			{
				m_nFilter = 0;
				m_agIDs.push_back( agID );
			}			
		}
		
		acCollection.CDLInit();
		while( acCollection.CDLGetNextInt( acID ) )
		{
			if ( acID == 0 )
			{
				m_nActionFilter = 0;
			}
			else
			{
				m_nActionFilter = -1;
				m_acIDs.push_back( acID );
			}			
		}

		if ( m_agIDs.size() > 0 || m_nFilter == -1 )
		{
			m_bPost = true;
		}
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query the database	              
\*--------------------------------------------------------------------------*/
void CAgentAudit::DoQuery()
{
	TAgentActivity taa;
	
	CEMSString sSQL;
	CEMSString sDate; 
	CEMSString sTime;
	CEMSString sLine;
	CEMSString sData1;
	CEMSString sData2;
	tstring sName;
	tstring sFromName;
	tstring sToName;
	tstring sActivityName;
	tstring sTBFromName;
	tstring sTBToName;
	tstring sTCFromName;
	tstring sTCToName;	
	tstring sTFName;
	
	// add column names for CDF
	if ( m_OutputCDF )
	{
		CEMSString sLine( _T("Date,Agent,Activity\r\n") );
		OutputCDF( sLine );
	}

	// get the SQL query string
	switch ( m_nType )
	{
	case 0:
		FormatAgentQuery( sSQL );
		break;

	default:
		FormatAgentQuery( sSQL );
	}
	
	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;

	GetQuery().Initialize();

	// bind to database columns
	BINDCOL_LONG(  GetQuery(), taa.m_AgentActivityID );
	BINDCOL_LONG(  GetQuery(), taa.m_AgentID );
	BINDCOL_LONG(  GetQuery(), taa.m_ActivityID );
	BINDCOL_TIME(  GetQuery(), taa.m_ActivityDate );
	BINDCOL_LONG( GetQuery(), taa.m_ID1 );
	BINDCOL_LONG( GetQuery(), taa.m_ID2 );
	BINDCOL_LONG( GetQuery(), taa.m_ID3 );
	BINDCOL_TCHAR( GetQuery(), taa.m_Data1 );
	BINDCOL_TCHAR( GetQuery(), taa.m_Data2 );
	
	// bind query parameters
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
	BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );

	GetQuery().Execute( sSQL.c_str() );
	int nRows = 0;
	for ( nRows=0; GetQuery().Fetch() == S_OK; nRows++ )
	{
		if( nRows == m_MaxRows )
		{
			if( m_OutputCDF )
			{
				CEMSString sString;
				sString.Format( _T("Max row limit reached\r\n") );
				OutputCDF( sString );
			}
			else
			{
				GetXMLGen().AddChildElem( _T("Information"),  _T("Max line limit reached") );
			}

			break;
		}

		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(taa.m_ActivityDate,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDate, true, false );
				GetDateTimeString( tsLocal, tsLocalLen, sTime, false, true );
			}
			else
			{
				GetDateTimeString( taa.m_ActivityDate, taa.m_ActivityDateLen, sDate, true, false );
				GetDateTimeString( taa.m_ActivityDate, taa.m_ActivityDateLen, sTime, false, true );
			}
		}
		else
		{
			GetDateTimeString( taa.m_ActivityDate, taa.m_ActivityDateLen, sDate, true, false );
			GetDateTimeString( taa.m_ActivityDate, taa.m_ActivityDateLen, sTime, false, true );
		}
		
		GetAgentName( taa.m_AgentID, sName );

		if ( taa.m_ActivityID == 5 )
		{
			GetTicketBoxName( taa.m_ID2, sTBFromName );
			GetTicketBoxName( taa.m_ID3, sTBToName );						
		}
		
		if ( taa.m_ActivityID == 30 )
		{
			GetTicketCategoryName( taa.m_ID2, sTCFromName );
			GetTicketCategoryName( taa.m_ID3, sTCToName );						
		}		
		
		if ( taa.m_ActivityID == 31 )
		{
			GetTicketFieldName( taa.m_ID2, sTFName );									
		}

		if ( taa.m_ActivityID == 7 )
		{
			if ( taa.m_ID2 == 0 )
			{
				sFromName = _T("None");
			}
			else
			{
				GetAgentName( taa.m_ID2, sFromName );
			}
			if ( taa.m_ID3 == 0 )
			{
				sToName = _T("None");
			}
			else
			{
				GetAgentName( taa.m_ID3, sToName );
			}			
		}
	
		switch ( taa.m_ActivityID )
		{
			case 1:
				sActivityName = _T("Logged In");
				break;
			case 2:
				sActivityName = _T("Logged Out");
				break;
			case 3:
				sActivityName = _T("Deleted Ticket");
				break;
			case 4:
				sActivityName = _T("Created Ticket");
				break;
			case 5:
				sActivityName = _T("Moved Ticket");
				break;
			case 6:
				sActivityName = _T("Escalated Ticket");
				break;
			case 7:
				sActivityName = _T("Reassigned Ticket");
				break;
			case 8:
				sActivityName = _T("Restored Ticket");
				break;
			case 9:
				sActivityName = _T("Changed State of Ticket");
				break;
			case 10:
				sActivityName = _T("Changed State of Ticket");
				break;
			case 11:
				sActivityName = _T("Added Message");
				break;
			case 12:
				sActivityName = _T("Deleted Message");
				break;
			case 13:
				sActivityName = _T("Read Message");
				break;
			case 14:
				sActivityName = _T("Added Note");
				break;
			case 15:
				sActivityName = _T("Deleted Note");
				break;
			case 16:
				sActivityName = _T("Read Note");
				break;
			case 17:
				sActivityName = _T("Read Alert");
				break;
			case 18:
				sActivityName = _T("Deleted Alert");
				break;
			case 19:
				sActivityName = _T("Placed Ticket On-Hold");
				break;
			case 20:
				sActivityName = _T("Revoked Message");
				break;
			case 21:
				sActivityName = _T("Released Message");
				break;
			case 22:
				sActivityName = _T("Returned Message");
				break;
			case 23:
				sActivityName = _T("Copied Message");
				break;
			case 24:
				sActivityName = _T("Merged Ticket");
				break;
			case 25:
				sActivityName = _T("Saved Message");
				break;
			case 26:
				sActivityName = _T("Created Ticket Link");
				break;
			case 27:
				sActivityName = _T("Deleted Ticket Link");
				break;
			case 28:
				sActivityName = _T("Linked Ticket");
				break;
			case 29:
				sActivityName = _T("Unlinked Ticket");
				break;
			case 30:
				sActivityName = _T("Changed TicketCategory of Ticket");
				break;
			case 31:
				sActivityName = _T("Changed TicketField");
				break;
			default:
				sActivityName = _T("");
		}


		sData1 = taa.m_Data1;
		sData2 = taa.m_Data2;
		
		if( m_OutputCDF )
		{
			// escape any commas
			sDate.EscapeCSV();
			sTime.EscapeCSV();
			sData1.EscapeCSV();
			sData2.EscapeCSV();
			
			CEMSString sActivity;
			
			if ( taa.m_ActivityID == 1 )
			{
				sActivity.Format( _T("%s from IP: %s with %s"),sActivityName.c_str(), taa.m_Data1, taa.m_Data2 );
			}
			else if ( taa.m_ActivityID == 2 )
			{
				sActivity.Format( _T("%s"),sActivityName.c_str() );
			}
			else if ( taa.m_ActivityID > 2 && taa.m_ActivityID < 11 )
			{
				if ( taa.m_ActivityID == 5 )
				{
					sActivity.Format( _T("%s ID: %d from %s to %s"),sActivityName.c_str(), taa.m_ID1, sTBFromName.c_str(), sTBToName.c_str() );
				}
				else if ( taa.m_ActivityID == 7 )
				{
					sActivity.Format( _T("%s ID: %d from %s to %s"),sActivityName.c_str(), taa.m_ID1, sFromName.c_str(), sToName.c_str() );
				}
				else if ( taa.m_ActivityID == 9 )
				{
					if ( taa.m_ID2 == 1 )
					{
						sActivity.Format( _T("%s ID: %d from from Closed to Open"),sActivityName.c_str(), taa.m_ID1 );
					}
					else if ( taa.m_ID2 == 3 )
					{
						sActivity.Format( _T("%s ID: %d from from On-hold to Open"),sActivityName.c_str(), taa.m_ID1 );
					}					
				}
				else if ( taa.m_ActivityID == 10 )
				{
					if ( taa.m_ID2 == 2 )
					{
						sActivity.Format( _T("%s ID: %d from from Open to Closed"),sActivityName.c_str(), taa.m_ID1 );
					}
					else if ( taa.m_ID2 == 3 )
					{
						sActivity.Format( _T("%s ID: %d from from On-hold to Closed"),sActivityName.c_str(), taa.m_ID1 );
					}	
				}
				else
				{
					sActivity.Format( _T("%s ID: %d"),sActivityName.c_str(), taa.m_ID1 );
				}
				
			}
			else if ( taa.m_ActivityID == 11 )
			{
				sActivity.Format( _T("%s ID: %d to Ticket ID: %d"),sActivityName.c_str(), taa.m_ID1, taa.m_ID2 );
			}
			else if ( taa.m_ActivityID == 23 )
			{
				sActivity.Format( _T("%s ID: %d from Ticket ID: %d to Ticket ID %d"),sActivityName.c_str(), taa.m_ID1, taa.m_ID2, taa.m_ID3 );
			}
			else if ( taa.m_ActivityID == 30 )
			{
				sActivity.Format( _T("%s ID: %d from %s to %s"),sActivityName.c_str(), taa.m_ID1, sTCFromName.c_str(), sTCToName.c_str() );
			}
			else if ( taa.m_ActivityID == 31 )
			{
				sActivity.Format( _T("%s [%s] in Ticket ID: %d to %s"),sActivityName.c_str(), sTFName.c_str(), taa.m_ID1, taa.m_Data1 );
			}
			else
			{
				sActivity.Format( _T("%s ID: %d from Ticket ID: %d"),sActivityName.c_str(), taa.m_ID1, taa.m_ID2 );
			}
					
						
			// format the line
			sLine.Format( _T("\"%s %s\",\"%s\",\"%s\"\r\n"), 
				          sDate.c_str(), sTime.c_str(), sName.c_str(), sActivity.c_str() );

			// add the line to the comma delimited file
			OutputCDF( sLine );
		}
		else
		{
			GetXMLGen().AddChildElem( _T("Row") );
					
			GetXMLGen().IntoElem();
			{
				GetXMLGen().AddChildElem( _T("AgentActivityID"), taa.m_AgentActivityID );
				GetXMLGen().AddChildElem( _T("AgentID"), taa.m_AgentID );
				GetXMLGen().AddChildElem( _T("AgentName"), sName.c_str() );
				GetXMLGen().AddChildElem( _T("ActivityID"), taa.m_ActivityID );
				GetXMLGen().AddChildElem( _T("ActivityName"), sActivityName.c_str() );
				GetXMLGen().AddChildElem( _T("Date"), sDate.c_str() );
				GetXMLGen().AddChildElem( _T("Time"), sTime.c_str() );
				GetXMLGen().AddChildElem( _T("ID1"), taa.m_ID1 );
				GetXMLGen().AddChildElem( _T("ID2"), taa.m_ID2 );
				GetXMLGen().AddChildElem( _T("ID3"), taa.m_ID3 );
				GetXMLGen().AddChildElem( _T("Data1"), sData1.c_str() );
				GetXMLGen().AddChildElem( _T("Data2"), sData2.c_str() );
				if ( taa.m_ActivityID == 5 )
				{
					GetXMLGen().AddChildElem( _T("TbFromName"), sTBFromName.c_str() );
					GetXMLGen().AddChildElem( _T("TbToName"), sTBToName.c_str() );
				}
				if ( taa.m_ActivityID == 7 )
				{
					GetXMLGen().AddChildElem( _T("FromName"), sFromName.c_str() );
					GetXMLGen().AddChildElem( _T("ToName"), sToName.c_str() );
				}
				if ( taa.m_ActivityID == 30 )
				{
					GetXMLGen().AddChildElem( _T("TcFromName"), sTCFromName.c_str() );
					GetXMLGen().AddChildElem( _T("TcToName"), sTCToName.c_str() );
				}
				if ( taa.m_ActivityID == 31 )
				{
					GetXMLGen().AddChildElem( _T("TFName"), sTFName.c_str() );
				}
			}
			GetXMLGen().OutOfElem();
		}
	}

	if ( m_OutputCDF && nRows == 0 )
		OutputCDF( CEMSString( _T("No results returned\r\n") ) );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Build the agent history query string	              
\*--------------------------------------------------------------------------*/
void CAgentAudit::FormatAgentQuery( CEMSString& sSQL )
{
	CEMSString sTemp;
	CEMSString sSortBy;
	CEMSString sSortDirection;
	list<int>::iterator agIter;

	// check security
		
	if ( m_nFilter > -1 )
	{
		for( agIter = m_agIDs.begin(); agIter != m_agIDs.end(); agIter++ )
		{
			RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, *agIter, EMS_READ_ACCESS );
		}		
	}
	else
	{
		RequireAdmin();
	}

	switch(m_nSortBy)
	{
	case 0:
		sSortBy.Format( _T("aal.ActivityDate") );
		break;
	case 1:
		sSortBy.Format( _T("ag.Name") );
		break;
	case 2:
		sSortBy.Format( _T("aa.Description") );
		break;
	default:
		sSortBy.Format( _T("aal.ActivityDate") );
	}

	switch(m_nSortDirection)
	{
	case 0:
		sSortDirection.Format( _T("ASC") );
		break;
	case 1:
		sSortDirection.Format( _T("DESC") );
		break;	
	default:
		sSortDirection.Format( _T("ASC") );
	}

	sSQL.Format( _T("SET ROWCOUNT %d "), m_MaxRows + 1 );

	if ( m_nFilter == -1 && m_nActionFilter == 0 )
	{
		sTemp.Format( _T("SELECT aal.AgentActivityID,aal.AgentID,aal.ActivityID,aal.ActivityDate,aal.ID1,aal.ID2,aal.ID3,aal.Data1,aal.Data2 ")
					_T("FROM AgentActivityLog aal ")
					_T("INNER JOIN AgentActivities aa ON aal.ActivityID=aa.AgentActivityID ")
					_T("INNER JOIN Agents ag ON aal.AgentID=ag.AgentID ")
					_T("WHERE aal.ActivityDate BETWEEN ? AND ? ORDER BY %s %s"), sSortBy.c_str(), sSortDirection.c_str() );
	}
	else if ( m_nFilter == -1 && m_nActionFilter == -1 )
	{
		sTemp.Format( _T("SELECT aal.AgentActivityID,aal.AgentID,aal.ActivityID,aal.ActivityDate,aal.ID1,aal.ID2,aal.ID3,aal.Data1,aal.Data2 ")
					_T("FROM AgentActivityLog aal ")
					_T("INNER JOIN AgentActivities aa ON aal.ActivityID=aa.AgentActivityID ")
					_T("INNER JOIN Agents ag ON aal.AgentID=ag.AgentID ")
					_T("WHERE aal.ActivityID IN (%s) AND aal.ActivityDate BETWEEN ? AND ? ORDER BY %s %s"), 
					acCollection.c_str(), sSortBy.c_str(), sSortDirection.c_str() );
	}
	else if ( m_nFilter == 0 && m_nActionFilter == -1 )
	{
		sTemp.Format( _T("SELECT aal.AgentActivityID,aal.AgentID,aal.ActivityID,aal.ActivityDate,aal.ID1,aal.ID2,aal.ID3,aal.Data1,aal.Data2 ")
					_T("FROM AgentActivityLog aal ")
					_T("INNER JOIN AgentActivities aa ON aal.ActivityID=aa.AgentActivityID ")
					_T("INNER JOIN Agents ag ON aal.AgentID=ag.AgentID ")
					_T("WHERE aal.AgentID IN (%s) AND aal.ActivityID IN (%s) AND aal.ActivityDate BETWEEN ? AND ? ORDER BY %s %s"), 
					agCollection.c_str(), acCollection.c_str(), sSortBy.c_str(), sSortDirection.c_str() );
	}
	else
	{
		sTemp.Format( _T("SELECT aal.AgentActivityID,aal.AgentID,aal.ActivityID,aal.ActivityDate,aal.ID1,aal.ID2,aal.ID3,aal.Data1,aal.Data2 ")
					_T("FROM AgentActivityLog aal ")
					_T("INNER JOIN AgentActivities aa ON aal.ActivityID=aa.AgentActivityID ")
					_T("INNER JOIN Agents ag ON aal.AgentID=ag.AgentID ")
					_T("WHERE aal.AgentID IN (%s) AND aal.ActivityDate BETWEEN ? AND ? ORDER BY %s %s"), 
					agCollection.c_str(), sSortBy.c_str(), sSortDirection.c_str() );
	}
	
	sSQL += sTemp;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void CAgentAudit::AddReportHeader()
{
	if ( !m_OutputCDF )
	{
		CEMSString sSubTitle;
		CEMSString sString;
		list<int>::iterator agIter;
		
		switch ( m_nFilter )
		{
			case -1:
				sSubTitle = _T("All Agents");
				break;
			default:
				if ( m_agIDs.size() == 1 )
				{
					for( agIter = m_agIDs.begin(); agIter != m_agIDs.end(); agIter++ )
					{
						GetAgentName( *agIter, sSubTitle );
					}			
				}
				else
				{
					sSubTitle = _T("Multiple Agents");
				}				
		}
		
		sSubTitle += _T(" [");

		long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
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
		}
		
		sSubTitle += sString.c_str();
		sSubTitle += _T(" - ");

		if(lTzBias != 1)
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
		}
		
		sSubTitle += sString.c_str();
		sSubTitle += _T("]");
		
		sSubTitle.EscapeHTML();
		GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
	}

	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add Form Field Options for the report	              
\*--------------------------------------------------------------------------*/
void CAgentAudit::AddReportOptions(void)
{
	list<int>::iterator acIter;

	// set the title
	switch ( m_nType )
	{
		case 0:
			m_sTitle = _T("Agent Audit");
			break;
		default:
			m_sTitle = _T("Agent Audit");
	}

	if ( m_bPost )
	{
		switch ( m_nActionFilter )
		{
			case 0:
				m_sTitle += _T(" - All Actions");
				break;
			default:
				if ( m_acIDs.size() == 1 )
				{
					for( acIter = m_acIDs.begin(); acIter != m_acIDs.end(); acIter++ )
					{
						switch ( *acIter )
						{
							case 1:
								m_sTitle += _T(" - Agent Login");
								break;
							case 2:
								m_sTitle += _T(" - Agent Logout");
								break;
							case 3:
								m_sTitle += _T(" - Ticket Delete");
								break;
							case 4:
								m_sTitle += _T(" - Ticket Create");
								break;
							case 5:
								m_sTitle += _T(" - Ticket Move");
								break;
							case 6:
								m_sTitle += _T(" - Ticket Escalate");
								break;
							case 7:
								m_sTitle += _T(" - Ticket Reassign");
								break;
							case 8:
								m_sTitle += _T(" - Ticket Restore");
								break;
							case 9:
								m_sTitle += _T(" - Ticket Open");
								break;
							case 10:
								m_sTitle += _T(" - Ticket Close");
								break;
							case 11:
								m_sTitle += _T(" - Ticket Linked");
								break;
							case 12:
								m_sTitle += _T(" - Ticket Unlinked");
								break;
							case 13:
								m_sTitle += _T(" - Message Add");
								break;
							case 14:
								m_sTitle += _T(" - Message Delete");
								break;
							case 15:
								m_sTitle += _T(" - Message Revoke");
								break;
							case 16:
								m_sTitle += _T(" - Message Release");
								break;
							case 17:
								m_sTitle += _T(" - Message Return");
								break;
							case 18:
								m_sTitle += _T(" - Link Create");
								break;
							case 19:
								m_sTitle += _T(" - Link Delete");
								break;
							default:
								m_sTitle += _T(" - Multiple Actions");
						}
					}
				}
				else
				{
					m_sTitle += _T(" - Multiple Actions");
				}				
		}
	}

	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/History_Reports.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().AddChildAttrib( _T("FULL_LABELS"), 1 );
	GetXMLGen().IntoElem();
	
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
		
		AddAgents( m_nFilter );
		AddDateRange( m_nDateRange );
		GetXMLGen().AddChildAttrib( _T("ROWSPAN"), 5 );

		GetXMLGen().OutOfElem();
		
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddActionTypes( m_nActionFilter );
		GetXMLGen().OutOfElem();
		
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddMaxLines(m_MaxRows);
		GetXMLGen().OutOfElem();

		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddSortBy( m_nSortBy );
		GetXMLGen().OutOfElem();
		
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddSortDirection( m_nSortDirection );
		GetXMLGen().OutOfElem();
		
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().AddChildAttrib( _T("HEIGHT"), _T("100%") );
		GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("NONE") );
		GetXMLGen().OutOfElem();
	
	GetXMLGen().OutOfElem();
}

