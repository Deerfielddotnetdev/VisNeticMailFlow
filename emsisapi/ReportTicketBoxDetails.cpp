
#include "stdafx.h"
#include "ReportTicketBoxDetails.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CTicketBoxDetails::CTicketBoxDetails( CISAPIData& ISAPIData ) : CReports( ISAPIData )
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
int CTicketBoxDetails::Run( CURLAction& action )
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
void CTicketBoxDetails::DecodeForm()
{
	int tbID;
	
	GetISAPIData().GetURLLong( _T("type"), m_nType );
		
	ISAPIPARAM param;
	
	if ( GetISAPIData().FindURLParam( _T("MAX_LINES"), param ) )
	{
		GetISAPIData().GetURLLong( _T("TICKETBOX"), m_nFilter, true );

		GetISAPIData().GetURLString( _T("TBIDCollection"), tbCollection, false );
		
		GetISAPIData().GetURLLong( _T("DATE_RANGE_SEL"), m_nDateRange, true );
		GetISAPIData().GetURLLong( _T("MAX_LINES"), m_MaxRows );
				
		CEMSString sDate;
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );
		
		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EndDate );

		tbCollection.CDLInit();
		while( tbCollection.CDLGetNextInt( tbID ) )
		{
			if ( tbID == -1 )
			{
				m_nFilter = -1;
			}
			else
			{
				m_nFilter = 0;
				m_tbIDs.push_back( tbID );
			}			
		}
		
		if ( m_tbIDs.size() > 0 || m_nFilter == -1 )
		{
			m_bPost = true;
		}
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Query the database	              
\*--------------------------------------------------------------------------*/
void CTicketBoxDetails::DoQuery()
{
	CEMSString sSQL;
	CEMSString sTicketBox;
	CEMSString sTicketBoxTotal;
	CEMSString sTotals;
	CEMSString sTemp; 
	CEMSString sOwner; 
	CEMSString sLine;
	TBReports_t tbd;
	vector<TBReports_t> m_tbd;
	vector<TBReports_t>::iterator mtbdIter;
	vector<TBReports_t> m_tbt;
	vector<TBReports_t>::iterator mtbtIter;
	vector<TBReports_t> m_t;
	vector<TBReports_t>::iterator mtIter;
		
	// add column names for CDF
	if ( m_OutputCDF )
	{
		CEMSString sLine( _T("TicketBox,Owner,Open,Closed,Total,Percent Closed\r\n") );
		OutputCDF( sLine );
	}

	// get the SQL query string
	FormatTbQuery( sSQL );
		
	m_tbd.clear();
	
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
			m_tbd.push_back(tbd);
		}	
		 
		sSQL.Format( _T("") );
		FormatTbTotalsQuery( sSQL );
		m_tbt.clear();
	
		GetQuery().Reset( false );
	
		GetQuery().Execute( sSQL.c_str() );
		while( GetQuery().Fetch() == S_OK )
		{
			m_tbt.push_back(tbd);
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
		
		for( mtbdIter = m_tbd.begin(); mtbdIter != m_tbd.end(); mtbdIter++ )
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

			sTicketBox.Format(_T("%s"),mtbdIter->Name);

			if ( sTemp != _T("") )
			{
				if ( sTemp != sTicketBox )
				{
					for( mtbtIter = m_tbt.begin(); mtbtIter != m_tbt.end(); mtbtIter++ )
					{
						sTicketBoxTotal.Format(_T("%s"),mtbtIter->Name);
						if ( sTemp == sTicketBoxTotal )
						{
							if( m_OutputCDF )
							{
								// escape any commas
								sTicketBoxTotal.Format(_T("%s Total"),mtbtIter->Name);
								sTicketBoxTotal.EscapeCSV();
																					
								// format the line
								sLine.Format( _T("\"%s\",\"Totals\",%d,%d,%d,%d\r\n"), 
											sTicketBoxTotal.c_str(), mtbtIter->nOpen, mtbtIter->nClosed, mtbtIter->nTotal, mtbtIter->nPctClosed );

								// add the line to the comma delimited file
								OutputCDF( sLine );
							}
							else
							{
								GetXMLGen().AddChildElem( _T("Row") );
										
								GetXMLGen().IntoElem();
								{
									sTicketBoxTotal.Format(_T("%s Total"),mtbtIter->Name);
									GetXMLGen().AddChildElem( _T("TicketBox"), sTicketBoxTotal.c_str() );
									GetXMLGen().AddChildElem( _T("Owner"), "" );
									GetXMLGen().AddChildElem( _T("Open"), mtbtIter->nOpen );
									GetXMLGen().AddChildElem( _T("Closed"), mtbtIter->nClosed );
									GetXMLGen().AddChildElem( _T("Total"), mtbtIter->nTotal );
									GetXMLGen().AddChildElem( _T("PctClosed"), mtbtIter->nPctClosed );
									GetXMLGen().AddChildElem( _T("IsTBTotal"), "1" );									
								}
								GetXMLGen().OutOfElem();
							}
						}
					}
				}
			}
			
			if ( mtbdIter->OwnerLen != SQL_NULL_DATA )
			{
				sOwner.Format(_T("%s"),mtbdIter->Owner);
			}
			else
			{
				sOwner.Format(_T("None"));
			}			

			if( m_OutputCDF )
			{
				// escape any commas
				sTicketBox.EscapeCSV();
				sOwner.EscapeCSV();
											
				// format the line
				sLine.Format( _T("\"%s\",\"%s\",%d,%d,%d,%d\r\n"), 
							sTicketBox.c_str(), sOwner.c_str(), mtbdIter->nOpen, mtbdIter->nClosed, mtbdIter->nTotal, mtbdIter->nPctClosed );

				// add the line to the comma delimited file
				OutputCDF( sLine );
			}
			else
			{
				GetXMLGen().AddChildElem( _T("Row") );
						
				GetXMLGen().IntoElem();
				{
					GetXMLGen().AddChildElem( _T("TicketBox"), sTicketBox.c_str() );
					GetXMLGen().AddChildElem( _T("Owner"), sOwner.c_str() );
					GetXMLGen().AddChildElem( _T("Open"), mtbdIter->nOpen );
					GetXMLGen().AddChildElem( _T("Closed"), mtbdIter->nClosed );
					GetXMLGen().AddChildElem( _T("Total"), mtbdIter->nTotal );
					GetXMLGen().AddChildElem( _T("PctClosed"), mtbdIter->nPctClosed );
					GetXMLGen().AddChildElem( _T("IsTBTotal"), "0" );
				}
				GetXMLGen().OutOfElem();
			}
			sTemp.Format(_T("%s"),mtbdIter->Name);
			nRows++;
		}
		
		if (nRows > 0)
		{
			for( mtbtIter = m_tbt.begin(); mtbtIter != m_tbt.end(); mtbtIter++ )
			{
				sTicketBoxTotal.Format(_T("%s"),mtbtIter->Name);
				if ( sTemp == sTicketBoxTotal )
				{
					if( m_OutputCDF )
					{
						// escape any commas
						sTicketBoxTotal.Format(_T("%s Total"),mtbtIter->Name);
						sTicketBoxTotal.EscapeCSV();
																			
						// format the line
						sLine.Format( _T("\"%s\",\"Totals\",%d,%d,%d,%d\r\n"), 
									sTicketBoxTotal.c_str(), mtbtIter->nOpen, mtbtIter->nClosed, mtbtIter->nTotal, mtbtIter->nPctClosed );

						// add the line to the comma delimited file
						OutputCDF( sLine );
					}
					else
					{
						GetXMLGen().AddChildElem( _T("Row") );
								
						GetXMLGen().IntoElem();
						{
							sTicketBoxTotal.Format(_T("%s Total"),mtbtIter->Name);
							GetXMLGen().AddChildElem( _T("TicketBox"), sTicketBoxTotal.c_str() );
							GetXMLGen().AddChildElem( _T("Owner"), "" );
							GetXMLGen().AddChildElem( _T("Open"), mtbtIter->nOpen );
							GetXMLGen().AddChildElem( _T("Closed"), mtbtIter->nClosed );
							GetXMLGen().AddChildElem( _T("Total"), mtbtIter->nTotal );
							GetXMLGen().AddChildElem( _T("PctClosed"), mtbtIter->nPctClosed );	
							GetXMLGen().AddChildElem( _T("IsTBTotal"), "1" );
						}
						GetXMLGen().OutOfElem();
					}

					for( mtIter = m_t.begin(); mtIter != m_t.end(); mtIter++ )
					{
						sTemp.Format(_T("%s"),mtIter->Name);
						if( m_OutputCDF )
						{
							// format the line
							sLine.Format( _T("\"%s\",\"\",%d,%d,%d,%d\r\n"), 
										sTemp.c_str(), mtIter->nOpen, mtIter->nClosed, mtIter->nTotal, mtIter->nPctClosed );

							// add the line to the comma delimited file
							OutputCDF( sLine );
						}
						else
						{
							GetXMLGen().AddChildElem( _T("Row") );
									
							GetXMLGen().IntoElem();
							{
								GetXMLGen().AddChildElem( _T("TicketBox"), sTemp.c_str() );
								GetXMLGen().AddChildElem( _T("Owner"), "" );
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
void CTicketBoxDetails::FormatTbQuery( CEMSString& sSQL )
{
	CEMSString sTemp;
	list<int>::iterator tbIter;

	// check security
		
	if ( m_nFilter > -1 )
	{
		for( tbIter = m_tbIDs.begin(); tbIter != m_tbIDs.end(); tbIter++ )
		{
			RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, *tbIter, EMS_READ_ACCESS );
		}		
	}
	else
	{
		RequireAdmin();
	}

	sSQL.Format( _T("SET ROWCOUNT %d "), m_MaxRows + 1 );

	if ( m_nFilter == -1 )
	{
		sTemp.Format( _T("SELECT tb.Name , a.Name, ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
					  _T("LEFT OUTER JOIN Agents a ON t.OwnerID=a.AgentID ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 ")
					  _T("GROUP BY tb.Name,a.Name ORDER BY tb.Name,a.Name") );
	}
	else 
	{
		sTemp.Format( _T("SELECT tb.Name , a.Name, ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
					  _T("LEFT OUTER JOIN Agents a ON t.OwnerID=a.AgentID ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 AND t.TicketBoxID IN (%s) ")
					  _T("GROUP BY tb.Name,a.Name ORDER BY tb.Name,a.Name"), tbCollection.c_str() );
	}
		
	sSQL += sTemp;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Build the tb query string	              
\*--------------------------------------------------------------------------*/
void CTicketBoxDetails::FormatTbTotalsQuery( CEMSString& sSQL )
{
	CEMSString sTemp;
	
	if ( m_nFilter == -1 )
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
	else 
	{
		sTemp.Format( _T("SELECT tb.Name , '', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 AND t.TicketBoxID IN (%s) ")
					  _T("GROUP BY tb.Name ORDER BY tb.Name "), tbCollection.c_str() );
	}
		
	sSQL += sTemp;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Build the tb query string	              
\*--------------------------------------------------------------------------*/
void CTicketBoxDetails::FormatTotalsQuery( CEMSString& sSQL )
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
		sTemp.Format( _T("SELECT 'Grand Total', ' ', ")
					  _T("COUNT(CASE WHEN t.TicketStateID > 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(CASE WHEN t.TicketStateID = 1 THEN 1 ELSE NULL END), ")
					  _T("COUNT(1), ")
					  _T("CASE WHEN COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)=0 OR COUNT(1)=0 THEN 0 ELSE COUNT(CASE WHEN t.TicketStateID=1 THEN 1 ELSE NULL END)*100/COUNT(1) END ") 
					  _T("FROM Tickets t ")
					  _T("WHERE DateCreated BETWEEN ? AND ? AND t.IsDeleted = 0 AND t.TicketBoxID IN (%s)"), tbCollection.c_str() );
	}
		
	sSQL += sTemp;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void CTicketBoxDetails::AddReportHeader()
{
	if ( !m_OutputCDF )
	{
		CEMSString sSubTitle;
		CEMSString sString;
		list<int>::iterator tbIter;
		
		switch ( m_nFilter )
		{
			case -1:
				sSubTitle = _T("All TicketBoxes");
				break;
			default:
				if ( m_tbIDs.size() == 1 )
				{
					for( tbIter = m_tbIDs.begin(); tbIter != m_tbIDs.end(); tbIter++ )
					{
						GetTicketBoxName( *tbIter, sSubTitle );
					}			
				}
				else
				{
					sSubTitle = _T("Multiple TicketBoxes");
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
void CTicketBoxDetails::AddReportOptions(void)
{
	// set the title
	m_sTitle = _T("TicketBox Details");
	

	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/TicketBox_Details.htm") );

	GetXMLGen().AddChildElem( _T("REPORT_OPTIONS") );
	GetXMLGen().AddChildAttrib( _T("FULL_LABELS"), 1 );
	GetXMLGen().IntoElem();
	
		GetXMLGen().AddChildElem( _T("ROW") );
		GetXMLGen().IntoElem();
		
		AddTicketBoxes(m_nFilter);
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

