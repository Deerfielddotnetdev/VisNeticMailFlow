/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ReportTicketDist.cpp,v 1.2 2005/11/29 21:16:26 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ReportTicketDist.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CReportTicketDist::CReportTicketDist(CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_bTicketBox = false;
	m_bTicketCategory = false;
	m_nTicketState = EMS_TICKETSTATEID_OPEN;
	m_include_zero = FALSE;
	m_include_unowned = FALSE;
	m_include_notassigned = FALSE;
	m_nTotal = 0;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Main entry point
\*--------------------------------------------------------------------------*/
int CReportTicketDist::Run(CURLAction& action)
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
||  Comments:	Decode Form Parameters
\*--------------------------------------------------------------------------*/
void CReportTicketDist::DecodeForm(void)
{
	tstring sType;
	GetISAPIData().GetURLString( _T("type"), sType, true );
	m_bTicketBox = sType.compare( _T("ticketbox") ) == 0;
	m_bTicketCategory = sType.compare( _T("ticketcategory") ) == 0;
	
	if ( GetISAPIData().GetURLLong( _T("ticket_state"), m_nTicketState, true ) )
	{
		ISAPIPARAM param;
		m_include_zero = GetISAPIData().FindURLParam( _T("include_zero"), param );
		m_include_unowned = GetISAPIData().FindURLParam( _T("include_unowned"), param );
		m_include_notassigned = GetISAPIData().FindURLParam( _T("include_notassigned"), param );
		m_bPost = true;
	}

	if( m_bTicketBox )
	{
		m_sTitle.assign("Ticket Distribution by TicketBox");
	}
	else if( m_bTicketCategory )
	{
		m_sTitle.assign("Ticket Distribution by Ticket Category");
	}
	else
	{
		m_sTitle.assign("Ticket Distribution by Owner");
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query the database
\*--------------------------------------------------------------------------*/
void CReportTicketDist::Query(void)
{
	if( m_bTicketBox )
	{
		QueryTicketBox();
	}
	else if( m_bTicketCategory )
	{
		QueryTicketCategory();
	}
	else
	{
		QueryOwner();
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query for ticket distribution by TicketBox	              
\*--------------------------------------------------------------------------*/
void CReportTicketDist::QueryTicketBox(void)
{
	CIDCount IDCount;
	map<int,int> count_map;
	map<int,int>::iterator map_iter;
	list<CIDCount> count_list;
	list<unsigned int>::iterator iter;
	CEMSString sStateID;
	CEMSString sQuery;
	CEMSString sTicketCategoryID;

	sTicketCategoryID.Format(_T("AND (Tickets.TicketCategoryID IN (SELECT DISTINCT TicketCategories.TicketCategoryID FROM TicketCategories ")
									_T("INNER JOIN Objects ON TicketCategories.ObjectID=Objects.ObjectID ")
									_T("INNER JOIN AccessControl ON Objects.ObjectID=AccessControl.ObjectID ")
									_T("WHERE Objects.BuiltIn=0 AND Objects.UseDefaultRights=0 AND AccessControl.AccessLevel>1 ")
									_T("AND (AccessControl.AgentID=%d OR GroupID IN (SELECT GroupID FROM AgentGroupings WHERE AgentID=%d))) ")
									_T("OR Tickets.TicketCategoryID IN(SELECT DISTINCT TicketCategories.TicketCategoryID FROM TicketCategories ")
									_T("INNER JOIN Objects ON TicketCategories.ObjectID = Objects.ObjectID ")
									_T("CROSS JOIN AccessControl ")
									_T("WHERE Objects.UseDefaultRights = 1 ")
									_T("AND ((SELECT MAX(AccessLevel) FROM AccessControl INNER JOIN Objects ON ")
									_T("Objects.ObjectID=AccessControl.ObjectID WHERE Objects.ObjectTypeID=6 AND Objects.BuiltIn=1 ")
									_T("AND Objects.ActualID=0 AND (AccessControl.AgentID=%d OR GroupID IN (SELECT GroupID FROM AgentGroupings ")
									_T("WHERE AgentID=%d)))>1))) "),m_ISAPIData.m_pSession->m_AgentID,m_ISAPIData.m_pSession->m_AgentID,m_ISAPIData.m_pSession->m_AgentID,m_ISAPIData.m_pSession->m_AgentID);
	


	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nID );
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nCount );

	// format the ticket state filter clause
	if ( m_nTicketState != EMS_REPORT_ALL )
	{
		if ( m_nTicketState != EMS_TICKETSTATEID_OPEN )
		{
			sStateID.Format( _T(" AND TicketStateID=%d"), m_nTicketState );
		}
		else
		{	
			// if the state is "open" also display escalated, on-hold and re-opened items
			sStateID = _T(" AND ((TicketStateID > 1 AND TicketStateID < 5) OR TicketStateID = 6) ");
		}
	}

	if(m_nTicketState != EMS_TICKETSTATEID_DELETED && m_nTicketState != EMS_REPORT_ALL)
	{
		if ( !GetIsAdmin() && g_ThreadPool.GetSharedObjects().m_bUseATC )
		{
			sQuery.Format( _T("SELECT TicketBoxID, COUNT(*) FROM TICKETS ")
						   _T("INNER JOIN AgentTicketCategories ATC ON ")
						   _T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
						   _T("WHERE ATC.AgentID=%d AND IsDeleted=0 %s GROUP BY TicketBoxID"), GetAgentID(), sStateID.c_str() );
		}
		else
		{
			sQuery.Format( _T("SELECT TicketBoxID, COUNT(*) FROM TICKETS ")
						   _T("WHERE IsDeleted=0 %s GROUP BY TicketBoxID"), sStateID.c_str() );
		}
	}
	else
	{
		if ( !GetIsAdmin() && g_ThreadPool.GetSharedObjects().m_bUseATC )
		{
			if(m_nTicketState != EMS_REPORT_ALL)
			{
				sQuery.Format( _T("SELECT TicketBoxID, COUNT(*) FROM TICKETS ")
							   _T("INNER JOIN AgentTicketCategories ATC ON ")
							   _T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
							   _T("WHERE ATC.AgentID=%d AND IsDeleted=1 GROUP BY TicketBoxID") );
			}
			else
			{
				sStateID.Format( _T(" OR TicketStateID >= 0"), m_nTicketState );

				sQuery.Format( _T("SELECT TicketBoxID, COUNT(*) FROM TICKETS ")
							   _T("INNER JOIN AgentTicketCategories ATC ON ")
							   _T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
							   _T("WHERE ATC.AgentID=%d AND IsDeleted=1 %s GROUP BY TicketBoxID"), sStateID.c_str() );
			}   
		}
		else
		{
			if(m_nTicketState != EMS_REPORT_ALL)
			{
				sQuery.Format( _T("SELECT TicketBoxID, COUNT(*) FROM TICKETS ")
							_T("WHERE IsDeleted=1 GROUP BY TicketBoxID") );
			}
			else
			{
				sStateID.Format( _T(" OR TicketStateID >= 0"), m_nTicketState );

				sQuery.Format( _T("SELECT TicketBoxID, COUNT(*) FROM TICKETS ")
						_T("WHERE IsDeleted=1 %s GROUP BY TicketBoxID"), sStateID.c_str() );
			}
		}
	}

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
||  Comments:	Query for ticket distribution by Ticket Category              
\*--------------------------------------------------------------------------*/
void CReportTicketDist::QueryTicketCategory(void)
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

	// format the ticket state filter clause
	if ( m_nTicketState != EMS_REPORT_ALL )
	{
		if ( m_nTicketState != EMS_TICKETSTATEID_OPEN )
		{
			sStateID.Format( _T(" AND TicketStateID=%d"), m_nTicketState );
		}
		else
		{	
			// if the state is "open" also display escalated, on-hold and re-opened items
			sStateID = _T(" AND TicketStateID > 1 AND TicketStateID < 5 OR TicketStateID = 6");
		}
	}

	if(m_nTicketState != EMS_TICKETSTATEID_DELETED && m_nTicketState != EMS_REPORT_ALL)
	{
		sQuery.Format( _T("SELECT TicketCategoryID, COUNT(*) FROM TICKETS ")
			_T("WHERE IsDeleted=0 %s GROUP BY TicketCategoryID"), sStateID.c_str() );
	}
	else
	{
		if(m_nTicketState != EMS_REPORT_ALL)
		{
			sQuery.Format( _T("SELECT TicketCategoryID, COUNT(*) FROM TICKETS ")
						 _T("WHERE IsDeleted=1 GROUP BY TicketCategoryID"), sStateID.c_str() );
		}
		else
		{
			sStateID.Format( _T(" OR TicketStateID >= 0"), m_nTicketState );

			sQuery.Format( _T("SELECT TicketCategoryID, COUNT(*) FROM TICKETS ")
					_T("WHERE IsDeleted=1 %s GROUP BY TicketCategoryID"), sStateID.c_str() );
		}
	}

	GetQuery().Execute( sQuery.c_str() );
	
	// fetch the results into the map
	while ( GetQuery().Fetch() == S_OK )
	{
		count_map[IDCount.m_nID] = IDCount.m_nCount;
	}
	
	// for each ticket category
	for( iter = GetXMLCache().m_TicketCategoryIDs.GetList().begin(); 
		 iter != GetXMLCache().m_TicketCategoryIDs.GetList().end(); iter++ )
	{
		//check security
		if ( GetAgentRightLevel(EMS_OBJECT_TYPE_TICKET_CATEGORY, *iter) >= EMS_READ_ACCESS )
		{
			IDCount.m_nID = *iter;
			
			if ( IDCount.m_nID != 0 || ( m_include_notassigned && IDCount.m_nID == 0 ) )
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
							
				if ( m_include_zero || IDCount.m_nCount > 0 )
				m_list.push_back(IDCount);
			
			}
		}
	}

	// generate the XML
	GenerateXML();
}


/*---------------------------------------------------------------------------\            
||  Comments:	Query for ticket distribution by Owner	              
\*--------------------------------------------------------------------------*/
void CReportTicketDist::QueryOwner(void)
{
	CIDCount IDCount;
	map<int,int> count_map;
	map<int,int>::iterator map_iter;
	list<CIDCount> count_list;
	list<unsigned int>::iterator iter;
	CEMSString sStateID;
	CEMSString sQuery;
	
	// query the database
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nID );
	BINDCOL_LONG_NOLEN( GetQuery(), IDCount.m_nCount );
	
	if ( m_nTicketState != EMS_REPORT_ALL )
		sStateID.Format( _T(" AND TicketStateID=%d"), m_nTicketState );
	
	if(m_nTicketState != EMS_TICKETSTATEID_DELETED && m_nTicketState != EMS_REPORT_ALL)
	{
		sQuery.Format( _T("SELECT OwnerID, COUNT(*) FROM TICKETS ")
					_T("WHERE IsDeleted=0 %s GROUP BY OwnerID"), sStateID.c_str() );
	}
	else
	{
		if(m_nTicketState != EMS_REPORT_ALL)
		{
			sQuery.Format( _T("SELECT OwnerID, COUNT(*) FROM TICKETS ")
						_T("WHERE IsDeleted=1 GROUP BY OwnerID"), sStateID.c_str() );
		}
		else
		{
			sStateID.Format( _T(" OR TicketStateID >= 0"), m_nTicketState );

			sQuery.Format( _T("SELECT OwnerID, COUNT(*) FROM TICKETS ")
					_T("WHERE IsDeleted=1 %s GROUP BY OwnerID"), sStateID.c_str() );
		}
	}
	
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
		 
	if ( m_include_unowned && (map_iter = count_map.find(0)) != count_map.end() )
	{
		 IDCount.m_nID = 0;
		 IDCount.m_nCount = map_iter->second;
		 m_nTotal += IDCount.m_nCount;
		 m_list.push_back(IDCount);
	}

	// generate the XML
	GenerateXML();
}

/*---------------------------------------------------------------------------\            
||  Comments:	Generate Output XML
\*--------------------------------------------------------------------------*/
void CReportTicketDist::GenerateXML(void)
{
	CEMSString sString;
	
	if ( !m_OutputCDF )
	{
		// build the sub-title
		CEMSString sSubTitle( _T("All") );
		GetTicketStateName( m_nTicketState, sSubTitle );
		sSubTitle.append( _T(" Tickets") );
		sSubTitle.EscapeHTML();
		GetXMLGen().AddChildElem( _T("SubTitle"), sSubTitle.c_str() );
		
		// define columns
		GetXMLGen().AddChildElem( _T("Column"), m_bTicketBox ? _T("TicketBox") : m_bTicketCategory ? _T("Ticket Category") : _T("Agent") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Tickets") );
		GetXMLGen().AddChildElem( _T("Column"), _T("Percentage") );
	}
	else
	{
		sString.Format( _T("\"%s\",\"%s\",\"%s\"\r\n"), 
			            m_bTicketBox ? _T("TicketBox") : m_bTicketCategory ? _T("Ticket Category") : _T("Agent"), _T("Tickets"), _T("Percentage") );
		OutputCDF( sString );
	}

	// add rows
	list<CIDCount>::iterator iter;
	int i=0;

	for ( iter = m_list.begin(); iter != m_list.end(); iter++ )
	{
		CEMSString sName( _T("Not Owned") );
		CEMSString sCount, sPercent;

		m_bTicketBox ? GetTicketBoxName( iter->m_nID, sName ) : m_bTicketCategory ? GetTicketCategoryName( iter->m_nID, sName ) : GetAgentName( iter->m_nID, sName );
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
			i++;
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
void CReportTicketDist::AddReportOptions(void)
{
	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Ticket_Distribution.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("ROW") );
	GetXMLGen().IntoElem();
	
	AddTicketStates( m_nTicketState, false );
	
	// add the option to exclude items with zero records
	GetXMLGen().AddChildElem( _T("FORM_FIELD") );
	GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
	GetXMLGen().AddChildAttrib( _T("NAME"), _T("include_zero") );
	GetXMLGen().AddChildAttrib( _T("CHECKED"), m_include_zero );
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("LABLE"), m_bTicketBox ? 
		_T("Include TicketBoxes with 0 Tickets") : m_bTicketCategory ? _T("Include Ticket Categories with 0 Tickets") : _T("Include Agents with 0 Tickets") );
	GetXMLGen().OutOfElem();
	
	// add the option to include tickets that are not owned
	if ( !m_bTicketBox && !m_bTicketCategory )
	{
		GetXMLGen().AddChildElem( _T("FORM_FIELD") );
		GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
		GetXMLGen().AddChildAttrib( _T("NAME"), _T("include_unowned") );
		GetXMLGen().AddChildAttrib( _T("CHECKED"), m_include_unowned );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("LABLE"), _T("Include tickets that are not owned") );
		GetXMLGen().OutOfElem();
	}
	
	// add the option to include the Unassigned ticket category
	if ( m_bTicketCategory )
	{
		GetXMLGen().AddChildElem( _T("FORM_FIELD") );
		GetXMLGen().AddChildAttrib( _T("TYPE"), _T("CHECK") );
		GetXMLGen().AddChildAttrib( _T("NAME"), _T("include_notassigned") );
		GetXMLGen().AddChildAttrib( _T("CHECKED"), m_include_notassigned );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("LABLE"), _T("Include the Unassigned Ticket Category") );
		GetXMLGen().OutOfElem();
	}
	
	GetXMLGen().OutOfElem();
	GetXMLGen().OutOfElem();
}