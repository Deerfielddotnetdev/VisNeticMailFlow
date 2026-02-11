
#include "stdafx.h"
#include "ReportTicketCountSum.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CTicketCountSum::CTicketCountSum( CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_bTicketBox = false;
	m_bTicketCategory = false;
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
int CTicketCountSum::Run( CURLAction& action )
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
			DoQuery();
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
void CTicketCountSum::DecodeForm()
{
	int tbID;
	
	tstring sType;
	GetISAPIData().GetURLString( _T("type"), sType, true );
	m_bTicketBox = sType.compare( _T("ticketbox") ) == 0;
	m_bTicketCategory = sType.compare( _T("ticketcategory") ) == 0;
		
	ISAPIPARAM param;
	
	if ( GetISAPIData().FindURLParam( _T("MAX_LINES"), param ) )
	{
		GetISAPIData().GetURLLong( _T("TICKETBOX"), m_nFilter, true );

		GetISAPIData().GetURLString( _T("IDCollection"), idCollection, false );
		
		GetISAPIData().GetURLLong( _T("DATE_RANGE_SEL"), m_nDateRange, true );
		GetISAPIData().GetURLLong( _T("MAX_LINES"), m_MaxRows );
				
		CEMSString sDate;
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );
		
		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EndDate );

		idCollection.CDLInit();
		while( idCollection.CDLGetNextInt( tbID ) )
		{
			if ( tbID == -1 )
			{
				m_nFilter = -1;
			}
			else
			{
				m_nFilter = 0;
				m_IDs.push_back( tbID );
			}			
		}
		
		if ( m_IDs.size() > 0 || m_nFilter == -1 )
		{
			m_bPost = true;
		}
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query the database	              
\*--------------------------------------------------------------------------*/
void CTicketCountSum::DoQuery()
{
	CEMSString sSQL;
	CEMSString sIDTotal;
	CEMSString sTotals;
	CEMSString sTemp; 
	CEMSString sOwner; 
	CEMSString sLine;
	TBReports_t tbd;
	vector<TBReports_t> m_idt;
	vector<TBReports_t>::iterator midtIter;
	vector<TBReports_t> m_t;
	vector<TBReports_t>::iterator mtIter;
		
	// add column names for CDF
	if ( m_OutputCDF )
	{
		if( m_bTicketBox )
		{
			CEMSString sLine( _T("TicketBox,Open,Closed,Total,Percent Closed\r\n") );
		}
		else if( m_bTicketCategory )
		{
			CEMSString sLine( _T("Ticket Category,Open,Closed,Total,Percent Closed\r\n") );
		}
		else
		{
			CEMSString sLine( _T("Owner,Open,Closed,Total,Percent Closed\r\n") );
		}			
		
		OutputCDF( sLine );
	}

	// get the SQL query string
	FormatIDTotalsQuery( sSQL );
		
	m_idt.clear();
	
	// init the query
	GetQuery().Initialize();
	
	try
	{
		// bind to database columns
		BINDCOL_TCHAR( GetQuery(), tbd.Name );		
		BINDCOL_TCHAR( GetQuery(), tbd.Owner );		
		BINDCOL_LONG_NOLEN( GetQuery(), tbd.nOpen );
		BINDCOL_LONG_NOLEN( GetQuery(), tbd.nClosed );
		BINDCOL_LONG_NOLEN( GetQuery(), tbd.nTotal );
		BINDCOL_LONG_NOLEN( GetQuery(), tbd.nPctClosed );

		// bind query parameters
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );

		GetQuery().Execute( sSQL.c_str() );
		while( GetQuery().Fetch() == S_OK )
		{
			m_idt.push_back(tbd);
		}	
		 
		sSQL.Format( _T("") );
		FormatTotalsQuery( sSQL );
		m_t.clear();
	
		GetQuery().Reset( false );
	
		GetQuery().Execute( sSQL.c_str() );
		while( GetQuery().Fetch() == S_OK )
		{
			m_t.push_back(tbd);
		}

		int nRows=0;
		
		for( midtIter = m_idt.begin(); midtIter != m_idt.end(); midtIter++ )
		{
			if ( nRows == m_MaxRows )
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

			if ( midtIter->NameLen != SQL_NULL_DATA )
			{
				sIDTotal.Format(_T("%s"),midtIter->Name);
			}
			else
			{
				sIDTotal.Format(_T("None"));
			}			

			if( m_OutputCDF )
			{
				// escape any commas
				sIDTotal.EscapeCSV();
											
				// format the line
				sLine.Format( _T("\"%s\",%d,%d,%d,%d\r\n"), 
							sIDTotal.c_str(), midtIter->nOpen, midtIter->nClosed, midtIter->nTotal, midtIter->nPctClosed );

				// add the line to the comma delimited file
				OutputCDF( sLine );
			}
			else
			{
				GetXMLGen().AddChildElem( _T("Row") );
						
				GetXMLGen().IntoElem();
				{
					GetXMLGen().AddChildElem( _T("Name"), sIDTotal.c_str() );
					GetXMLGen().AddChildElem( _T("Open"), midtIter->nOpen );
					GetXMLGen().AddChildElem( _T("Closed"), midtIter->nClosed );
					GetXMLGen().AddChildElem( _T("Total"), midtIter->nTotal );
					GetXMLGen().AddChildElem( _T("PctClosed"), midtIter->nPctClosed );
					GetXMLGen().AddChildElem( _T("IsTBTotal"), "0" );
				}
				GetXMLGen().OutOfElem();
			}
			
			nRows++;
		}
		
		if (nRows > 0 && !m_OutputCDF)
		{
			for( mtIter = m_t.begin(); mtIter != m_t.end(); mtIter++ )
			{
				sTemp.Format(_T("%s"),mtIter->Name);
				if( m_OutputCDF )
				{
					// escape any commas
					sTemp.EscapeCSV();

					// format the line
					sLine.Format( _T("\"%s\",%d,%d,%d,%d\r\n"), 
								sTemp.c_str(), mtIter->nOpen, mtIter->nClosed, mtIter->nTotal, mtIter->nPctClosed );

					// add the line to the comma delimited file
					OutputCDF( sLine );
				}
				else
				{
					GetXMLGen().AddChildElem( _T("Row") );
							
					GetXMLGen().IntoElem();
					{
						GetXMLGen().AddChildElem( _T("Name"), sTemp.c_str() );
						GetXMLGen().AddChildElem( _T("Open"), mtIter->nOpen );
						GetXMLGen().AddChildElem( _T("Closed"), mtIter->nClosed );
						GetXMLGen().AddChildElem( _T("Total"), mtIter->nTotal );
						GetXMLGen().AddChildElem( _T("PctClosed"), mtIter->nPctClosed );
						GetXMLGen().AddChildElem( _T("IsTBTotal"), "2" );
					}
					GetXMLGen().OutOfElem();
				}
			}
		}

		if ( m_OutputCDF && nRows == 0 )
		OutputCDF( CEMSString( _T("No results returned\r\n") ) );
	}
	catch(...)
	{
		throw;
	}
		
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Build the tb query string	              
\*--------------------------------------------------------------------------*/
void CTicketCountSum::FormatIDTotalsQuery( CEMSString& sSQL )
{
	CEMSString sTemp;
	list<int>::iterator idIter;

	if ( m_nFilter > -1 )
	{
		for( idIter = m_IDs.begin(); idIter != m_IDs.end(); idIter++ )
		{
			if( m_bTicketBox )
			{
				RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, *idIter, EMS_READ_ACCESS );
			}
			else if( m_bTicketCategory )
			{
				RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_CATEGORY, *idIter, EMS_READ_ACCESS );
			}
			else
			{
				RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, *idIter, EMS_READ_ACCESS );
			}				
		}		
	}
	else
	{
		RequireAdmin();
	}
	
	if ( m_nFilter == -1 )
	{
		if( m_bTicketBox )
		{
			sTemp.Format( _T("SELECT tb.Name , '', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 ")
					  _T("GROUP BY tb.Name ORDER BY tb.Name ") );						
		}
		else if( m_bTicketCategory )
		{
			sTemp.Format( _T("SELECT tc.Description , '', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 ")
					  _T("GROUP BY tc.Description ORDER BY tc.Description ") );	
		}
		else
		{
			sTemp.Format( _T("SELECT a.Name , '', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("LEFT OUTER JOIN Agents a ON t.OwnerID=a.AgentID ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 ")
					  _T("GROUP BY a.Name ORDER BY a.Name ") );	
		}		
	}
	else 
	{
		if( m_bTicketBox )
		{
			sTemp.Format( _T("SELECT tb.Name , '', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 AND t.TicketBoxID IN (%s) ")
					  _T("GROUP BY tb.Name ORDER BY tb.Name "), idCollection.c_str() );
		}
		else if( m_bTicketCategory )
		{
			sTemp.Format( _T("SELECT tc.Description , '', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 AND t.TicketCategoryID IN (%s) ")
					  _T("GROUP BY tc.Description ORDER BY tc.Description "), idCollection.c_str() );
		}
		else
		{
			sTemp.Format( _T("SELECT a.Name , '', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("LEFT OUTER JOIN Agents a ON t.OwnerID=a.AgentID ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 AND t.OwnerID IN (%s) ")
					  _T("GROUP BY a.Name ORDER BY a.Name "), idCollection.c_str() );
		}		
	}
		
	sSQL += sTemp;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Build the tb query string	              
\*--------------------------------------------------------------------------*/
void CTicketCountSum::FormatTotalsQuery( CEMSString& sSQL )
{
	CEMSString sTemp;
	
	if ( m_nFilter == -1 )
	{
		sTemp.Format( _T("SELECT 'Grand Total', ' ', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0") );		
	}
	else 
	{
		if( m_bTicketBox )
		{
			sTemp.Format( _T("SELECT 'Grand Total', ' ', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 AND t.TicketBoxID IN (%s)"), idCollection.c_str() );
		}
		else if( m_bTicketCategory )
		{
			sTemp.Format( _T("SELECT 'Grand Total', ' ', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 AND t.TicketCategoryID IN (%s)"), idCollection.c_str() );
		}
		else
		{
			sTemp.Format( _T("SELECT 'Grand Total', ' ', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 AND t.OwnerID IN (%s)"), idCollection.c_str() );
		}		
	}
		
	sSQL += sTemp;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void CTicketCountSum::AddReportHeader()
{
	if ( !m_OutputCDF )
	{
		CEMSString sSubTitle;
		CEMSString sString;
		list<int>::iterator idIter;
		
		switch ( m_nFilter )
		{
			case -1:
				if( m_bTicketBox )
				{
					sSubTitle = _T("All TicketBoxes");
				}
				else if( m_bTicketCategory )
				{
					sSubTitle = _T("All Ticket Categories");
				}
				else
				{
					sSubTitle = _T("All Owners");
				}				
				break;
			default:
				if ( m_IDs.size() == 1 )
				{
					for( idIter = m_IDs.begin(); idIter != m_IDs.end(); idIter++ )
					{
						if( m_bTicketBox )
						{
							GetTicketBoxName( *idIter, sSubTitle );
						}
						else if( m_bTicketCategory )
						{
							GetTicketCategoryName( *idIter, sSubTitle );
						}
						else
						{
							GetAgentName( *idIter, sSubTitle );
						}						
					}			
				}
				else
				{
					if( m_bTicketBox )
					{
						sSubTitle = _T("Multiple TicketBoxes");
					}
					else if( m_bTicketCategory )
					{
						sSubTitle = _T("Multiple Ticket Categories");
					}
					else
					{
						sSubTitle = _T("Multiple Owners");
					}					
				}				
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
	}

	GetXMLGen().AddChildElem( _T("Results") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add Form Field Options for the report	              
\*--------------------------------------------------------------------------*/
void CTicketCountSum::AddReportOptions(void)
{
	// set the title
	if( m_bTicketBox )
	{
		m_sTitle.assign("Ticket Count Summary by TicketBox");
	}
	else if( m_bTicketCategory )
	{
		m_sTitle.assign("Ticket Count Summary by Ticket Category");
	}
	else
	{
		m_sTitle.assign("Ticket Count Summary by Owner");
	}
	

	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Ticket_Count_Summary.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().AddChildAttrib( _T("FULL_LABELS"), 1 );
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
		else
		{
			AddAgents(m_nFilter);
		}
		
		AddDateRange( m_nDateRange );
		GetXMLGen().AddChildAttrib( _T("ROWSPAN"), 5 );

		GetXMLGen().OutOfElem();
		
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
			AddMaxLines(m_MaxRows);
		GetXMLGen().OutOfElem();

		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().AddChildAttrib( _T("HEIGHT"), _T("100%") );
		GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("FORM_FIELD") );
			GetXMLGen().AddChildAttrib( _T("TYPE"), _T("NONE") );
		GetXMLGen().OutOfElem();
	
	GetXMLGen().OutOfElem();
}

