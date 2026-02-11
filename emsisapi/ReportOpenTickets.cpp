#include "stdafx.h"
#include "ReportOpenTickets.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
COpenTickets::COpenTickets( CISAPIData& ISAPIData ) : CReports( ISAPIData )
{
	m_bPost = false;
	m_bTicketBox = false;
	m_bTicketCategory = false;
	m_bGroup = false;
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
int COpenTickets::Run( CURLAction& action )
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
void COpenTickets::DecodeForm()
{
	int nID;
	
	tstring sType;
	GetISAPIData().GetURLString( _T("type"), sType, true );
	m_bTicketBox = sType.compare( _T("ticketbox") ) == 0;
	m_bTicketCategory = sType.compare( _T("ticketcategory") ) == 0;
	m_bGroup = sType.compare( _T("group") ) == 0;
		
	ISAPIPARAM param;
	
	if ( GetISAPIData().FindURLParam( _T("MAX_LINES"), param ) )
	{
		GetISAPIData().GetURLString( _T("IDCollection"), idCollection, false );
		
		GetISAPIData().GetURLLong( _T("DATE_RANGE_SEL"), m_nDateRange, true );
		GetISAPIData().GetURLLong( _T("MAX_LINES"), m_MaxRows );
				
		CEMSString sDate;
		GetISAPIData().GetURLString( _T("START_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_StartDate );
		
		GetISAPIData().GetURLString( _T("END_DATE"), sDate );
		sDate.CDLGetTimeStamp( m_EndDate );

		idCollection.CDLInit();
		while( idCollection.CDLGetNextInt( nID ) )
		{
			if ( nID == -1 )
			{
				m_nFilter = -1;
			}
			else
			{
				m_nFilter = 0;
				m_IDs.push_back( nID );
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
void COpenTickets::DoQuery()
{
	CEMSString sSQL;
	CEMSString sTemp;
	CEMSString sName;
	CEMSString sTicketBox;
	CEMSString sTicketCategory;
	CEMSString sGroup;
	CEMSString sOwner;
	CEMSString sSubject;
	CEMSString sNotes;
	CEMSString sLine;
	CEMSString sDateCreated;
	CEMSString sDateUpdated;
	OpenTicketReports_t otr;
	TBReports_t tbd;
	vector<TBReports_t> m_tb;
	vector<TBReports_t>::iterator mtbIter; 
	int nRows=0;
				
	// add column names for CDF
	if ( m_OutputCDF )
	{
		if( m_bTicketBox )
		{
			sLine.Format( _T("TicketBox,ID,Subject,Created,State,Notes,Updated,Category,Owner,Days Created,Days Updated\r\n") );
		}
		else if( m_bTicketCategory )
		{
			sLine.Format( _T("Category,ID,Subject,Created,State,Notes,Updated,TicketBox,Owner,Days Created,Days Updated\r\n") );
		}
		else if( m_bGroup )
		{
			sLine.Format( _T("Owner,ID,Subject,Created,State,Notes,Updated,TicketBox,Category,Days Created,Days Updated\r\n") );
		}
		else
		{
			sLine.Format( _T("Owner,ID,Subject,Created,State,Notes,Updated,TicketBox,Category,Days Created,Days Updated\r\n") );
		}			
		
		OutputCDF( sLine );
	}

	// get the SQL query string
	FormatGroupTotalQuery( sSQL );
		
	m_tb.clear();
	
	// init the query
	GetQuery().Initialize();
	
	try
	{
		// bind to database columns
		BINDCOL_TCHAR( GetQuery(), tbd.Owner );		
		BINDCOL_LONG_NOLEN( GetQuery(), tbd.nOpen );
		
		// bind query parameters
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
		
		GetQuery().Execute( sSQL.c_str() );
		while( GetQuery().Fetch() == S_OK )
		{
			m_tb.push_back(tbd);
		}		 
	}
	catch(...)
	{
		throw;
	}

	sSQL.Format( _T("") );
	// get the SQL query string
	FormatQuery( sSQL );
		
	// init the query
	GetQuery().Initialize();
	
	try
	{
		// bind to database columns
		if( m_bTicketBox )
		{
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketBox );		
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_TicketID );		
			BINDCOL_TCHAR( GetQuery(), otr.m_Subject );
			BINDCOL_TIME( GetQuery(), otr.m_DateCreated );
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketState );
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_NumNotes );
			BINDCOL_TIME( GetQuery(), otr.m_LastUpdate );
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketCategory );		
			BINDCOL_TCHAR( GetQuery(), otr.m_Owner );		
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_DaysCreated );
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_DaysUpdated );
		}
		else if( m_bTicketCategory )
		{
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketCategory );		
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_TicketID );		
			BINDCOL_TCHAR( GetQuery(), otr.m_Subject );
			BINDCOL_TIME( GetQuery(), otr.m_DateCreated );
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketState );
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_NumNotes );
			BINDCOL_TIME( GetQuery(), otr.m_LastUpdate );
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketBox );		
			BINDCOL_TCHAR( GetQuery(), otr.m_Owner );		
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_DaysCreated );
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_DaysUpdated );
		}
		else if( m_bGroup )
		{
			BINDCOL_TCHAR( GetQuery(), otr.m_Group );		
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_TicketID );		
			BINDCOL_TCHAR( GetQuery(), otr.m_Subject );
			BINDCOL_TIME( GetQuery(), otr.m_DateCreated );
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketState );
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_NumNotes );
			BINDCOL_TIME( GetQuery(), otr.m_LastUpdate );
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketBox );		
			BINDCOL_TCHAR( GetQuery(), otr.m_Owner );
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketCategory );		
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_DaysCreated );
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_DaysUpdated );
		}
		else
		{
			BINDCOL_TCHAR( GetQuery(), otr.m_Owner );		
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_TicketID );		
			BINDCOL_TCHAR( GetQuery(), otr.m_Subject );
			BINDCOL_TIME( GetQuery(), otr.m_DateCreated );
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketState );
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_NumNotes );
			BINDCOL_TIME( GetQuery(), otr.m_LastUpdate );
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketBox );		
			BINDCOL_TCHAR( GetQuery(), otr.m_TicketCategory );		
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_DaysCreated );
			BINDCOL_LONG_NOLEN( GetQuery(), otr.m_DaysUpdated );
		}	
		

		// bind query parameters
		BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
		BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
		
		GetQuery().Execute( sSQL.c_str() );
		while( GetQuery().Fetch() == S_OK )
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

			if ( otr.m_OwnerLen != SQL_NULL_DATA )
			{
				sOwner.Format(_T("%s"),otr.m_Owner);
			}
			else
			{
				sOwner.Format(_T("None"));
			}			

			sTicketBox.Format(_T("%s"),otr.m_TicketBox);
			sTicketCategory.Format(_T("%s"),otr.m_TicketCategory);
			sGroup.Format(_T("%s"),otr.m_Group);
			sSubject.Format(_T("%s"),otr.m_Subject);
			GetDateTimeString( otr.m_DateCreated, otr.m_DateCreatedLen, sDateCreated );
			GetDateTimeString( otr.m_LastUpdate, otr.m_LastUpdateLen, sDateUpdated );

			if ( sTemp != _T("") )
			{
				if( m_bTicketBox )
				{
					if ( sTemp != sTicketBox )
					{
						for( mtbIter = m_tb.begin(); mtbIter != m_tb.end(); mtbIter++ )
						{
							sName.Format(_T("%s"),mtbIter->Owner);
							if ( sTemp == sName )
							{
								sName.Format(_T("%s Total"),mtbIter->Owner);
								if( m_OutputCDF )
								{
									// escape any commas
									sName.EscapeCSV();
																						
									// format the line
									sLine.Format( _T("\"%s\",%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\r\n"), 
												sName.c_str(), mtbIter->nOpen );

									// add the line to the comma delimited file
									OutputCDF( sLine );
								}
								else
								{
									GetXMLGen().AddChildElem( _T("Row") );
									GetXMLGen().IntoElem();
									{
										GetXMLGen().AddChildElem( _T("TicketBox"), sName.c_str() );
										GetXMLGen().AddChildElem( _T("TicketID"), mtbIter->nOpen );
										GetXMLGen().AddChildElem( _T("Subject"), "" );
										GetXMLGen().AddChildElem( _T("DateCreated"), "" );
										GetXMLGen().AddChildElem( _T("State"), "" );
										GetXMLGen().AddChildElem( _T("Notes"), "" );
										GetXMLGen().AddChildElem( _T("DateUpdated"), "" );
										GetXMLGen().AddChildElem( _T("Category"), "" );
										GetXMLGen().AddChildElem( _T("Owner"), "" );
										GetXMLGen().AddChildElem( _T("DaysCreated"), "" );
										GetXMLGen().AddChildElem( _T("DaysUpdated"), "" );
										GetXMLGen().AddChildElem( _T("IsTBTotal"), "1" );
									}
									GetXMLGen().OutOfElem();
								}
							}
						}
					}
				}
				else if( m_bTicketCategory )
				{
					if ( sTemp != sTicketCategory )
					{
						for( mtbIter = m_tb.begin(); mtbIter != m_tb.end(); mtbIter++ )
						{
							sName.Format(_T("%s"),mtbIter->Owner);
							if ( sTemp == sName )
							{
								sName.Format(_T("%s Total"),mtbIter->Owner);									
								if( m_OutputCDF )
								{
									// escape any commas
									sName.EscapeCSV();
																						
									// format the line
									sLine.Format( _T("\"%s\",%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\r\n"),  
												sName.c_str(), mtbIter->nOpen );

									// add the line to the comma delimited file
									OutputCDF( sLine );
								}
								else
								{
									GetXMLGen().AddChildElem( _T("Row") );
									GetXMLGen().IntoElem();
									{
										GetXMLGen().AddChildElem( _T("Category"), sName.c_str() );
										GetXMLGen().AddChildElem( _T("TicketID"),  mtbIter->nOpen );
										GetXMLGen().AddChildElem( _T("Subject"), "" );
										GetXMLGen().AddChildElem( _T("DateCreated"), "" );
										GetXMLGen().AddChildElem( _T("State"), "" );
										GetXMLGen().AddChildElem( _T("Notes"), "" );
										GetXMLGen().AddChildElem( _T("DateUpdated"), "" );
										GetXMLGen().AddChildElem( _T("TicketBox"), "" );
										GetXMLGen().AddChildElem( _T("Owner"), "" );
										GetXMLGen().AddChildElem( _T("DaysCreated"), "" );
										GetXMLGen().AddChildElem( _T("DaysUpdated"),"" );
										GetXMLGen().AddChildElem( _T("IsTBTotal"), "1" );
									}
									GetXMLGen().OutOfElem();
								}
							}
						}
					}
				}
				else if( m_bGroup )
				{
					if ( sTemp != sGroup )
					{
						for( mtbIter = m_tb.begin(); mtbIter != m_tb.end(); mtbIter++ )
						{
							sName.Format(_T("%s"),mtbIter->Owner);
							if ( sTemp == sName )
							{
								sName.Format(_T("%s Total"),mtbIter->Owner);									
								if( m_OutputCDF )
								{
									// escape any commas
									sName.EscapeCSV();
																						
									// format the line
									sLine.Format( _T("\"%s\",%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\r\n"),  
												sName.c_str(), mtbIter->nOpen );

									// add the line to the comma delimited file
									OutputCDF( sLine );
								}
								else
								{
									GetXMLGen().AddChildElem( _T("Row") );
									GetXMLGen().IntoElem();
									{
										GetXMLGen().AddChildElem( _T("Group"), sName.c_str() );
										GetXMLGen().AddChildElem( _T("TicketID"),  mtbIter->nOpen );
										GetXMLGen().AddChildElem( _T("Subject"), "" );
										GetXMLGen().AddChildElem( _T("DateCreated"), "" );
										GetXMLGen().AddChildElem( _T("State"), "" );
										GetXMLGen().AddChildElem( _T("Notes"), "" );
										GetXMLGen().AddChildElem( _T("DateUpdated"), "" );
										GetXMLGen().AddChildElem( _T("TicketBox"), "" );
										GetXMLGen().AddChildElem( _T("Owner"), "" );
										GetXMLGen().AddChildElem( _T("DaysCreated"), "" );
										GetXMLGen().AddChildElem( _T("DaysUpdated"),"" );
										GetXMLGen().AddChildElem( _T("IsTBTotal"), "1" );
									}
									GetXMLGen().OutOfElem();
								}
							}
						}
					}
				}
				else
				{
					if ( sTemp != sOwner )
					{
						for( mtbIter = m_tb.begin(); mtbIter != m_tb.end(); mtbIter++ )
						{
							if ( mtbIter->OwnerLen != SQL_NULL_DATA )
							{
								sName.Format(_T("%s"),mtbIter->Owner);								
							}
							else
							{
								sName.Format(_T("None"));
							}							
							if ( sTemp == sName )
							{
								sName.Format(_T("%s Total"),sName.c_str());									
								if( m_OutputCDF )
								{
									// escape any commas
									sName.EscapeCSV();
																						
									// format the line
									sLine.Format( _T("\"%s\",%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\r\n"), 
												sName.c_str(), mtbIter->nOpen );

									// add the line to the comma delimited file
									OutputCDF( sLine );
								}
								else
								{
									GetXMLGen().AddChildElem( _T("Row") );
									GetXMLGen().IntoElem();
									{
										GetXMLGen().AddChildElem( _T("Owner"), sName.c_str() );
										GetXMLGen().AddChildElem( _T("TicketID"), mtbIter->nOpen );
										GetXMLGen().AddChildElem( _T("Subject"), "" );
										GetXMLGen().AddChildElem( _T("DateCreated"), "" );
										GetXMLGen().AddChildElem( _T("State"), "" );
										GetXMLGen().AddChildElem( _T("Notes"), "" );
										GetXMLGen().AddChildElem( _T("DateUpdated"), "" );
										GetXMLGen().AddChildElem( _T("TicketBox"), "" );
										GetXMLGen().AddChildElem( _T("Category"), "" );
										GetXMLGen().AddChildElem( _T("DaysCreated"), "" );
										GetXMLGen().AddChildElem( _T("DaysUpdated"), "" );
										GetXMLGen().AddChildElem( _T("IsTBTotal"), "1" );
									}
									GetXMLGen().OutOfElem();
								}
							}
						}
					}
				}				
			}
			if ( otr.m_NumNotes > 0 )
			{
				sNotes.Format(_T("Yes"));
			}
			else
			{
				sNotes.Format(_T("No"));
			}

			if( m_OutputCDF )
			{
				// escape any commas
				sOwner.EscapeCSV();
				sTicketBox.EscapeCSV();
				sTicketCategory.EscapeCSV();
				sGroup.EscapeCSV();
				sSubject.EscapeCSV();				
				
				// format the line
				if( m_bTicketBox )
				{
					sLine.Format( _T("\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d\r\n"), 
							sTicketBox.c_str(),otr.m_TicketID,sSubject.c_str(),sDateCreated.c_str(),otr.m_TicketState, 
							sNotes.c_str(),sDateUpdated.c_str(),sTicketCategory.c_str(),sOwner.c_str(), 
							otr.m_DaysCreated,otr.m_DaysUpdated );
				}
				else if( m_bTicketCategory )
				{
					sLine.Format( _T("\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d\r\n"), 
							sTicketCategory.c_str(),otr.m_TicketID,sSubject.c_str(),sDateCreated.c_str(),otr.m_TicketState, 
							sNotes.c_str(),sDateUpdated.c_str(),sTicketBox.c_str(),sOwner.c_str(), 
							otr.m_DaysCreated,otr.m_DaysUpdated );
				}
				else if( m_bGroup )
				{
					sLine.Format( _T("\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d\r\n"), 
							sOwner.c_str(),otr.m_TicketID,sSubject.c_str(),sDateCreated.c_str(),otr.m_TicketState, 
							sNotes.c_str(),sDateUpdated.c_str(),sTicketBox.c_str(),sTicketCategory.c_str(), 
							otr.m_DaysCreated,otr.m_DaysUpdated );
				}
				else
				{
					sLine.Format( _T("\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d\r\n"), 
							sOwner.c_str(),otr.m_TicketID,sSubject.c_str(),sDateCreated.c_str(),otr.m_TicketState, 
							sNotes.c_str(),sDateUpdated.c_str(),sTicketBox.c_str(),sTicketCategory.c_str(), 
							otr.m_DaysCreated,otr.m_DaysUpdated );
				}									
				
				// add the line to the comma delimited file
				OutputCDF( sLine );
			}
			else
			{
				GetXMLGen().AddChildElem( _T("Row") );
						
				GetXMLGen().IntoElem();
				{
					GetXMLGen().AddChildElem( _T("TicketBox"), sTicketBox.c_str() );
					GetXMLGen().AddChildElem( _T("TicketID"), otr.m_TicketID );
					GetXMLGen().AddChildElem( _T("Subject"), sSubject.c_str() );
					GetXMLGen().AddChildElem( _T("DateCreated"), sDateCreated.c_str() );
					GetXMLGen().AddChildElem( _T("State"), otr.m_TicketState );
					GetXMLGen().AddChildElem( _T("Notes"), sNotes.c_str() );
					GetXMLGen().AddChildElem( _T("DateUpdated"), sDateUpdated.c_str() );
					GetXMLGen().AddChildElem( _T("Category"), sTicketCategory.c_str() );
					GetXMLGen().AddChildElem( _T("Owner"), sOwner.c_str() );
					GetXMLGen().AddChildElem( _T("DaysCreated"), otr.m_DaysCreated );
					GetXMLGen().AddChildElem( _T("DaysUpdated"), otr.m_DaysUpdated );
				}
				GetXMLGen().OutOfElem();
			}
			
			if( m_bTicketBox )
			{
				sTemp.Format(_T("%s"),otr.m_TicketBox);
			}
			else if( m_bTicketCategory )
			{
				sTemp.Format(_T("%s"),otr.m_TicketCategory);
			}
			else if( m_bGroup )
			{
				sTemp.Format(_T("%s"),otr.m_Group);
			}
			else
			{
				if ( otr.m_OwnerLen != SQL_NULL_DATA )
				{
					sTemp.Format(_T("%s"),otr.m_Owner);								
				}
				else
				{
					sTemp.Format(_T("None"));
				}				
			}		
			
			nRows++;
		}	
		
		if (nRows > 0)
		{
			if( m_bTicketBox )
			{
				for( mtbIter = m_tb.begin(); mtbIter != m_tb.end(); mtbIter++ )
				{
					sName.Format(_T("%s"),mtbIter->Owner);
					if ( sTemp == sName )
					{
						sName.Format(_T("%s Total"),mtbIter->Owner);
						if( m_OutputCDF )
						{
							// escape any commas
							sName.EscapeCSV();
																				
							// format the line
							sLine.Format( _T("\"%s\",%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\r\n"), 
										sName.c_str(), mtbIter->nOpen );

							// add the line to the comma delimited file
							OutputCDF( sLine );
						}
						else
						{
							GetXMLGen().AddChildElem( _T("Row") );
							GetXMLGen().IntoElem();
							{
								GetXMLGen().AddChildElem( _T("TicketBox"), sName.c_str() );
								GetXMLGen().AddChildElem( _T("TicketID"), mtbIter->nOpen );
								GetXMLGen().AddChildElem( _T("Subject"), "" );
								GetXMLGen().AddChildElem( _T("DateCreated"), "" );
								GetXMLGen().AddChildElem( _T("State"), "" );
								GetXMLGen().AddChildElem( _T("Notes"), "" );
								GetXMLGen().AddChildElem( _T("DateUpdated"), "" );
								GetXMLGen().AddChildElem( _T("Category"), "" );
								GetXMLGen().AddChildElem( _T("Owner"), "" );
								GetXMLGen().AddChildElem( _T("DaysCreated"), "" );
								GetXMLGen().AddChildElem( _T("DaysUpdated"), "" );
								GetXMLGen().AddChildElem( _T("IsTBTotal"), "1" );
							}
							GetXMLGen().OutOfElem();
						}
					}					
				}
			}
			else if( m_bTicketCategory )
			{
				for( mtbIter = m_tb.begin(); mtbIter != m_tb.end(); mtbIter++ )
				{
					sName.Format(_T("%s"),mtbIter->Owner);
					if ( sTemp == sName )
					{
						sName.Format(_T("%s Total"),mtbIter->Owner);									
						if( m_OutputCDF )
						{
							// escape any commas
							sName.EscapeCSV();
																				
							// format the line
							sLine.Format( _T("\"%s\",%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\r\n"),  
										sName.c_str(), mtbIter->nOpen );

							// add the line to the comma delimited file
							OutputCDF( sLine );
						}
						else
						{
							GetXMLGen().AddChildElem( _T("Row") );
							GetXMLGen().IntoElem();
							{
								GetXMLGen().AddChildElem( _T("Category"), sName.c_str() );
								GetXMLGen().AddChildElem( _T("TicketID"),  mtbIter->nOpen );
								GetXMLGen().AddChildElem( _T("Subject"), "" );
								GetXMLGen().AddChildElem( _T("DateCreated"), "" );
								GetXMLGen().AddChildElem( _T("State"), "" );
								GetXMLGen().AddChildElem( _T("Notes"), "" );
								GetXMLGen().AddChildElem( _T("DateUpdated"), "" );
								GetXMLGen().AddChildElem( _T("TicketBox"), "" );
								GetXMLGen().AddChildElem( _T("Owner"), "" );
								GetXMLGen().AddChildElem( _T("DaysCreated"), "" );
								GetXMLGen().AddChildElem( _T("DaysUpdated"),"" );
								GetXMLGen().AddChildElem( _T("IsTBTotal"), "1" );
							}
							GetXMLGen().OutOfElem();
						}
					}					
				}
			}
			else if( m_bGroup )
			{
				for( mtbIter = m_tb.begin(); mtbIter != m_tb.end(); mtbIter++ )
				{
					sName.Format(_T("%s"),mtbIter->Owner);
					if ( sTemp == sName )
					{
						sName.Format(_T("%s Total"),mtbIter->Owner);									
						if( m_OutputCDF )
						{
							// escape any commas
							sName.EscapeCSV();
																				
							// format the line
							sLine.Format( _T("\"%s\",%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\r\n"),  
										sName.c_str(), mtbIter->nOpen );

							// add the line to the comma delimited file
							OutputCDF( sLine );
						}
						else
						{
							GetXMLGen().AddChildElem( _T("Row") );
							GetXMLGen().IntoElem();
							{
								GetXMLGen().AddChildElem( _T("Group"), sName.c_str() );
								GetXMLGen().AddChildElem( _T("TicketID"),  mtbIter->nOpen );
								GetXMLGen().AddChildElem( _T("Subject"), "" );
								GetXMLGen().AddChildElem( _T("DateCreated"), "" );
								GetXMLGen().AddChildElem( _T("State"), "" );
								GetXMLGen().AddChildElem( _T("Notes"), "" );
								GetXMLGen().AddChildElem( _T("DateUpdated"), "" );
								GetXMLGen().AddChildElem( _T("TicketBox"), "" );
								GetXMLGen().AddChildElem( _T("Owner"), "" );
								GetXMLGen().AddChildElem( _T("DaysCreated"), "" );
								GetXMLGen().AddChildElem( _T("DaysUpdated"),"" );
								GetXMLGen().AddChildElem( _T("IsTBTotal"), "1" );
							}
							GetXMLGen().OutOfElem();
						}
					}					
				}
			}
			else
			{
				for( mtbIter = m_tb.begin(); mtbIter != m_tb.end(); mtbIter++ )
				{
					if ( mtbIter->OwnerLen != SQL_NULL_DATA )
					{
						sName.Format(_T("%s"),mtbIter->Owner);								
					}
					else
					{
						sName.Format(_T("None"));
					}							
					if ( sTemp == sName )
					{
						sName.Format(_T("%s Total"),sName.c_str());										
						if( m_OutputCDF )
						{
							// escape any commas
							sName.EscapeCSV();
																				
							// format the line
							sLine.Format( _T("\"%s\",%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\r\n"), 
										sName.c_str(), mtbIter->nOpen );

							// add the line to the comma delimited file
							OutputCDF( sLine );
						}
						else
						{
							GetXMLGen().AddChildElem( _T("Row") );
							GetXMLGen().IntoElem();
							{
								GetXMLGen().AddChildElem( _T("Owner"), sName.c_str() );
								GetXMLGen().AddChildElem( _T("TicketID"), mtbIter->nOpen );
								GetXMLGen().AddChildElem( _T("Subject"), "" );
								GetXMLGen().AddChildElem( _T("DateCreated"), "" );
								GetXMLGen().AddChildElem( _T("State"), "" );
								GetXMLGen().AddChildElem( _T("Notes"), "" );
								GetXMLGen().AddChildElem( _T("DateUpdated"), "" );
								GetXMLGen().AddChildElem( _T("TicketBox"), "" );
								GetXMLGen().AddChildElem( _T("Category"), "" );
								GetXMLGen().AddChildElem( _T("DaysCreated"), "" );
								GetXMLGen().AddChildElem( _T("DaysUpdated"), "" );
								GetXMLGen().AddChildElem( _T("IsTBTotal"), "1" );
							}
							GetXMLGen().OutOfElem();
						}
					}
				}				
			}

			sSQL.Format( _T("") );
			// get the SQL query string
			FormatTotalQuery( sSQL );
				
			m_tb.clear();
			
			// init the query
			GetQuery().Initialize();
			
			try
			{
				// bind to database columns
				BINDCOL_LONG_NOLEN( GetQuery(), tbd.nTotal );
				
				// bind query parameters
				BINDPARAM_TIME_NOLEN( GetQuery(), m_StartDate );
				BINDPARAM_TIME_NOLEN( GetQuery(), m_EndDate );
				
				GetQuery().Execute( sSQL.c_str() );
				if( GetQuery().Fetch() == S_OK )
				{
					sName.Format(_T("Grand Total"));
					if( m_OutputCDF )
					{
						// format the line
						sLine.Format( _T("\"%s\",%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\r\n"), 
									sName.c_str(), tbd.nTotal );

						// add the line to the comma delimited file
						OutputCDF( sLine );
					}
					else
					{
						GetXMLGen().AddChildElem( _T("Row") );
						GetXMLGen().IntoElem();
						{
							if( m_bTicketBox )
							{
								GetXMLGen().AddChildElem( _T("Owner"), "" );
								GetXMLGen().AddChildElem( _T("TicketBox"), sName.c_str() );
								GetXMLGen().AddChildElem( _T("Category"), "" );
								GetXMLGen().AddChildElem( _T("Group"), "" );
							}
							else if( m_bTicketCategory )
							{
								GetXMLGen().AddChildElem( _T("Owner"), "" );
								GetXMLGen().AddChildElem( _T("TicketBox"), "" );
								GetXMLGen().AddChildElem( _T("Category"), sName.c_str() );
								GetXMLGen().AddChildElem( _T("Group"), "" );
							}
							else if( m_bGroup )
							{
								GetXMLGen().AddChildElem( _T("Owner"), "" );
								GetXMLGen().AddChildElem( _T("TicketBox"), "" );
								GetXMLGen().AddChildElem( _T("Category"), "" );
								GetXMLGen().AddChildElem( _T("Group"), sName.c_str() );
							}
							else
							{
								GetXMLGen().AddChildElem( _T("Owner"), sName.c_str() );
								GetXMLGen().AddChildElem( _T("TicketBox"), "" );
								GetXMLGen().AddChildElem( _T("Category"), "" );
								GetXMLGen().AddChildElem( _T("Group"), "" );
							}							
							GetXMLGen().AddChildElem( _T("TicketID"), tbd.nTotal );
							GetXMLGen().AddChildElem( _T("Subject"), "" );
							GetXMLGen().AddChildElem( _T("DateCreated"), "" );
							GetXMLGen().AddChildElem( _T("State"), "" );
							GetXMLGen().AddChildElem( _T("Notes"), "" );
							GetXMLGen().AddChildElem( _T("DateUpdated"), "" );
							GetXMLGen().AddChildElem( _T("DaysCreated"), "" );
							GetXMLGen().AddChildElem( _T("DaysUpdated"), "" );
							GetXMLGen().AddChildElem( _T("IsTBTotal"), "2" );
						}
						GetXMLGen().OutOfElem();
					}
				}		 
			}
			catch(...)
			{
				throw;
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
void COpenTickets::FormatQuery( CEMSString& sSQL )
{
	CEMSString sTemp;
	CEMSString sJoin;
	CEMSString sWhere;
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
			else if( m_bGroup )
			{
				RequireAgentRightLevel( EMS_OBJECT_TYPE_GROUP, *idIter, EMS_READ_ACCESS );
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
	
	GetSecurityQueryStrings( false, sJoin, sWhere );

	sSQL.Format( _T("SET ROWCOUNT %d "), m_MaxRows + 1 );
	
	if ( m_nFilter == -1 )
	{
		if( m_bTicketBox )
		{
			sTemp.Format( _T("SELECT tb.Name, t.TicketID, t.Subject, t.DateCreated, ts.Description, ")
						  _T("(SELECT COUNT(1) FROM TicketNotes WHERE TicketID=t.TicketID), ")
                          _T("(SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), ")
						  _T("tc.Description, a.Name, ")
						  _T("(SELECT DATEDIFF(day, t.DateCreated, getdate())), ")
						  _T("(SELECT DATEDIFF(day, (SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), getdate())) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
						  _T("INNER JOIN TicketStates ts ON t.TicketStateID=ts.TicketStateID ")
						  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID ")
						  _T("LEFT OUTER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 ")
						  _T("ORDER BY tb.Name,t.TicketID ") );
		}
		else if( m_bTicketCategory )
		{
			sTemp.Format( _T("SELECT tc.Description, t.TicketID, t.Subject, t.DateCreated, ts.Description, ")
						  _T("(SELECT COUNT(1) FROM TicketNotes WHERE TicketID=t.TicketID), ")
                          _T("(SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), ")
						  _T("tb.Name, a.Name, ")
						  _T("(SELECT DATEDIFF(day, t.DateCreated, getdate())), ")
						  _T("(SELECT DATEDIFF(day, (SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), getdate())) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
						  _T("INNER JOIN TicketStates ts ON t.TicketStateID=ts.TicketStateID ")
						  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID %s ")
						  _T("LEFT OUTER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 %s ")
						  _T("ORDER BY tc.Description,t.TicketID "),sJoin.c_str(),sWhere.c_str() );
		}
		else if( m_bGroup )
		{
			sTemp.Format( _T("SELECT g.GroupName, t.TicketID, t.Subject, t.DateCreated, ts.Description, ")
						  _T("(SELECT COUNT(1) FROM TicketNotes WHERE TicketID=t.TicketID), ")
                          _T("(SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), ")
						  _T("tb.Name, a.Name, tc.Description, ")
						  _T("(SELECT DATEDIFF(day, t.DateCreated, getdate())), ")
						  _T("(SELECT DATEDIFF(day, (SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), getdate())) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
						  _T("INNER JOIN TicketStates ts ON t.TicketStateID=ts.TicketStateID ")
						  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID %s ")
						  _T("INNER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("INNER JOIN AgentGroupings ag ON t.OwnerID = ag.AgentID ")
						  _T("INNER JOIN Groups g ON ag.GroupID = g.GroupID ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.OwnerID > 0 AND t.TicketStateID > 1 %s ")
						  _T("ORDER BY g.GroupName,t.TicketID "),sJoin.c_str(),sWhere.c_str() );
		}
		else
		{
			sTemp.Format( _T("SELECT a.Name, t.TicketID, t.Subject, t.DateCreated, ts.Description, ")
						  _T("(SELECT COUNT(1) FROM TicketNotes WHERE TicketID=t.TicketID), ")
                          _T("(SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), ")
						  _T("tb.Name, tc.Description, ")
						  _T("(SELECT DATEDIFF(day, t.DateCreated, getdate())), ")
						  _T("(SELECT DATEDIFF(day, (SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), getdate())) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
						  _T("INNER JOIN TicketStates ts ON t.TicketStateID=ts.TicketStateID ")
						  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID %s ")
						  _T("LEFT OUTER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 %s ")
						  _T("ORDER BY a.Name,t.TicketID "),sJoin.c_str(),sWhere.c_str() );
		}		
	}
	else 
	{
		if( m_bTicketBox )
		{
			sTemp.Format( _T("SELECT tb.Name, t.TicketID, t.Subject, t.DateCreated, ts.Description, ")
						  _T("(SELECT COUNT(1) FROM TicketNotes WHERE TicketID=t.TicketID), ")
                          _T("(SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), ")
						  _T("tc.Description, a.Name, ")
						  _T("(SELECT DATEDIFF(day, t.DateCreated, getdate())), ")
						  _T("(SELECT DATEDIFF(day, (SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), getdate())) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
						  _T("INNER JOIN TicketStates ts ON t.TicketStateID=ts.TicketStateID ")
						  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID ")
						  _T("LEFT OUTER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 AND t.TicketBoxID IN (%s) ")
						  _T("ORDER BY tb.Name,t.TicketID "),idCollection.c_str() );
			
		}
		else if( m_bTicketCategory )
		{
			sTemp.Format( _T("SELECT tc.Description, t.TicketID, t.Subject, t.DateCreated, ts.Description, ")
						  _T("(SELECT COUNT(1) FROM TicketNotes WHERE TicketID=t.TicketID), ")
                          _T("(SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), ")
						  _T("tb.Name, a.Name, ")
						  _T("(SELECT DATEDIFF(day, t.DateCreated, getdate())), ")
						  _T("(SELECT DATEDIFF(day, (SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), getdate())) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
						  _T("INNER JOIN TicketStates ts ON t.TicketStateID=ts.TicketStateID ")
						  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID %s ")
						  _T("LEFT OUTER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 AND t.TicketCategoryID IN (%s) %s ")
						  _T("ORDER BY tc.Description,t.TicketID "),sJoin.c_str(),idCollection.c_str(),sWhere.c_str() );
		}
		else if( m_bGroup )
		{
			sTemp.Format( _T("SELECT g.GroupName, t.TicketID, t.Subject, t.DateCreated, ts.Description, ")
						  _T("(SELECT COUNT(1) FROM TicketNotes WHERE TicketID=t.TicketID), ")
                          _T("(SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), ")
						  _T("tb.Name, a.Name, tc.Description, ")
						  _T("(SELECT DATEDIFF(day, t.DateCreated, getdate())), ")
						  _T("(SELECT DATEDIFF(day, (SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), getdate())) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
						  _T("INNER JOIN TicketStates ts ON t.TicketStateID=ts.TicketStateID ")
						  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID %s ")
						  _T("INNER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("INNER JOIN AgentGroupings ag ON t.OwnerID = ag.AgentID ")
						  _T("INNER JOIN Groups g ON ag.GroupID = g.GroupID ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.OwnerID > 0 AND t.TicketStateID > 1 AND g.GroupID IN (%s) %s ")
						  _T("ORDER BY g.GroupName,t.TicketID "),sJoin.c_str(),idCollection.c_str(),sWhere.c_str() );			
		}
		else
		{
			sTemp.Format( _T("SELECT a.Name, t.TicketID, t.Subject, t.DateCreated, ts.Description, ")
						  _T("(SELECT COUNT(1) FROM TicketNotes WHERE TicketID=t.TicketID), ")
                          _T("(SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), ")
						  _T("tb.Name, tc.Description, ")
						  _T("(SELECT DATEDIFF(day, t.DateCreated, getdate())), ")
						  _T("(SELECT DATEDIFF(day, (SELECT TOP 1 (DateTime) FROM TicketHistory WHERE TicketID=t.TicketID AND TicketActionID <> 9 AND TicketActionID <> 12 ORDER BY DateTime DESC), getdate())) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
						  _T("INNER JOIN TicketStates ts ON t.TicketStateID=ts.TicketStateID ")
						  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID %s ")
						  _T("LEFT OUTER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 AND t.OwnerID IN (%s) %s ")
						  _T("ORDER BY a.Name,t.TicketID "),sJoin.c_str(),idCollection.c_str(),sWhere.c_str() );
		}		
	}
		
	sSQL += sTemp;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Build the tb query string	              
\*--------------------------------------------------------------------------*/
void COpenTickets::FormatGroupTotalQuery( CEMSString& sSQL )
{
	CEMSString sTemp;
	CEMSString sJoin;
	CEMSString sWhere;

	GetSecurityQueryStrings( true, sJoin, sWhere );
		
	if ( m_nFilter == -1 )
	{
		if( m_bTicketBox )
		{
			sTemp.Format( _T("SELECT tb.Name, COUNT(1) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 ")
						  _T("GROUP BY tb.Name ORDER BY tb.Name ") );
		}
		else if( m_bTicketCategory )
		{
			sTemp.Format( _T("SELECT tc.Description, COUNT(1) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID %s ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 %s ")
						  _T("GROUP BY tc.Description ORDER BY tc.Description "),sJoin.c_str(),sWhere.c_str() );
		}
		else if( m_bGroup )
		{
			sTemp.Format( _T("SELECT g.GroupName, COUNT(1) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("INNER JOIN AgentGroupings ag ON t.OwnerID = ag.AgentID ")
						  _T("INNER JOIN Groups g ON ag.GroupID = g.GroupID %s ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.OwnerID > 0 AND t.TicketStateID > 1 %s ")
						  _T("GROUP BY g.GroupName ORDER BY g.GroupName "),sJoin.c_str(),sWhere.c_str() );
		}
		else
		{
			sTemp.Format( _T("SELECT a.Name, COUNT(1) ")
						  _T("FROM Tickets t ")
						  _T("LEFT OUTER JOIN Agents a ON t.OwnerID = a.AgentID %s ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 %s ")
						  _T("GROUP BY a.Name ORDER BY a.Name "),sJoin.c_str(),sWhere.c_str() );
		}		
	}
	else 
	{
		if( m_bTicketBox )
		{
			sTemp.Format( _T("SELECT tb.Name, COUNT(1) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 AND t.TicketBoxID IN (%s) ")
						  _T("GROUP BY tb.Name ORDER BY tb.Name "), idCollection.c_str() );
		}
		else if( m_bTicketCategory )
		{
			sTemp.Format( _T("SELECT tc.Description, COUNT(1) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN TicketCategories tc ON t.TicketCategoryID=tc.TicketCategoryID %s ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 AND t.TicketCategoryID IN (%s) %s ")
						  _T("GROUP BY tc.Description ORDER BY tc.Description "),sJoin.c_str(),idCollection.c_str(),sWhere.c_str() );
		}
		else if( m_bGroup )
		{
			sTemp.Format( _T("SELECT g.GroupName, COUNT(1) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("INNER JOIN AgentGroupings ag ON t.OwnerID = ag.AgentID ")
						  _T("INNER JOIN Groups g ON ag.GroupID = g.GroupID %s ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.OwnerID > 0 AND t.TicketStateID > 1 AND g.GroupID IN (%s) %s ")
						  _T("GROUP BY g.GroupName ORDER BY g.GroupName "),sJoin.c_str(),idCollection.c_str(),sWhere.c_str() );
		}
		else
		{
			sTemp.Format( _T("SELECT a.Name, COUNT(1) ")
						  _T("FROM Tickets t ")
						  _T("LEFT OUTER JOIN Agents a ON t.OwnerID = a.AgentID %s ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 AND t.OwnerID IN (%s) %s ")
						  _T("GROUP BY a.Name ORDER BY a.Name "),sJoin.c_str(),idCollection.c_str(),sWhere.c_str() );
		}		
	}
		
	sSQL = sTemp;
}
/*---------------------------------------------------------------------------\                     
||  Comments:	Build the tb query string	              
\*--------------------------------------------------------------------------*/
void COpenTickets::FormatTotalQuery( CEMSString& sSQL )
{
	CEMSString sTemp;
	CEMSString sJoin;
	CEMSString sWhere;

	GetSecurityQueryStrings( true, sJoin, sWhere );
		
	if ( m_nFilter == -1 )
	{
		sTemp.Format( _T("SELECT COUNT(1) FROM Tickets t ")
				  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
				  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 ") );
	}
	else 
	{
		if( m_bTicketBox )
		{
			sTemp.Format( _T("SELECT COUNT(1) FROM Tickets t ")
				  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
				  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 AND t.TicketBoxID IN (%s) "), idCollection.c_str() );			
		}
		else if( m_bTicketCategory )
		{
			sTemp.Format( _T("SELECT COUNT(1) FROM Tickets t %s ")
				  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
				  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 AND t.TicketCategoryID IN (%s) %s "),sJoin.c_str(),idCollection.c_str(),sWhere.c_str() );
		}
		else if( m_bGroup )
		{
			sTemp.Format( _T("SELECT COUNT(1) ")
						  _T("FROM Tickets t ")
						  _T("INNER JOIN Agents a ON t.OwnerID = a.AgentID ")
						  _T("INNER JOIN AgentGroupings ag ON t.OwnerID = ag.AgentID ")
						  _T("INNER JOIN Groups g ON ag.GroupID = g.GroupID %s ")
						  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
						  _T("AND t.IsDeleted = 0 AND t.OwnerID > 0 AND t.TicketStateID > 1 AND g.GroupID IN (%s) %s "),sJoin.c_str(),idCollection.c_str(),sWhere.c_str() );
		}
		else
		{
			sTemp.Format( _T("SELECT COUNT(1) FROM Tickets t %s ")
				  _T("WHERE t.DateCreated BETWEEN ? AND ? ")
				  _T("AND t.IsDeleted = 0 AND t.TicketStateID > 1 AND t.OwnerID IN (%s) %s "),sJoin.c_str(),idCollection.c_str(),sWhere.c_str() );
		}		
	}

	sSQL = sTemp;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the report header
\*--------------------------------------------------------------------------*/
void COpenTickets::AddReportHeader()
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
				else if( m_bGroup )
				{
					sSubTitle = _T("All Groups");
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
						else if( m_bGroup )
						{
							GetGroupName( *idIter, sSubTitle );
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
					else if( m_bGroup )
					{
						sSubTitle = _T("Multiple Groups");
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
void COpenTickets::AddReportOptions(void)
{
	// set the title
	if( m_bTicketBox )
	{
		m_sTitle.assign("Open Tickets by TicketBox");
	}
	else if( m_bTicketCategory )
	{
		m_sTitle.assign("Open Tickets by Ticket Category");
	}
	else if( m_bGroup )
	{
		m_sTitle.assign("Open Tickets by Group");
	}
	else
	{
		m_sTitle.assign("Open Tickets by Owner");
	}
	

	GetXMLGen().AddChildElem( _T("Title"), m_sTitle.c_str() );
	GetXMLGen().AddChildElem( _T("HelpPath"), _T("Reporting/Open_Tickets.htm") );

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
		else if( m_bGroup )
		{
			AddGroups(m_nFilter, false);
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

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the SQL query string that is needed to only
||				include the tickets that the logged in user has access to.
\*--------------------------------------------------------------------------*/
void COpenTickets::GetSecurityQueryStrings( bool bAll, CEMSString& sJoin, CEMSString& sWhere )
{
	// if the agent is an admin, then they have
	// access to everything
	if ( GetIsAdmin() )
		return;

	int nDefaultAccessLevel = GetAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, 0 );
	
	if ( bAll )
	{
        sJoin = _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
    			_T("INNER JOIN Objects o ON tb.ObjectID=o.ObjectID ");
	}
	else
	{
		sJoin = _T("INNER JOIN Objects o ON tb.ObjectID=o.ObjectID ");
	}
	
	// Modified on 7/11/2007 via Mike C. on the phone (Mark Mohr).  This
	// Fixed an issue when viewing history report, Mike C. has more information about this.
	if( nDefaultAccessLevel >= EMS_READ_ACCESS )
	{
		sWhere.Format( _T("AND (UseDefaultRights=1 OR (UseDefaultRights=0 AND ")
			           _T("EXISTS (SELECT TOP 1 AccessControlID FROM AccessControl ")
			           _T("WHERE ObjectID = tb.ObjectID AND AccessLevel >=2 AND (AgentID=%d OR GroupID IN ")
			           _T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=%d)))))"),
			           GetAgentID(), GetAgentID() );
	}
	else
	{
		sWhere.Format( _T("AND (UseDefaultRights=0 AND EXISTS (SELECT TOP 1 AccessControlID FROM AccessControl ")
			           _T("WHERE ObjectID = tb.ObjectID AND AccessLevel >=2 AND (AgentID=%d OR GroupID IN ")
			           _T("(SELECT GroupID FROM AgentGroupings WHERE AgentID=%d))))"),
			            GetAgentID(), GetAgentID() );
	}
}

