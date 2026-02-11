/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ContactNotes.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Contact Notes
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ContactNotes.h"
#include "AttachFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CContactNotes::Run(CURLAction& action)
{
	m_IsPhone = false;
	
	if ( GetISAPIData().GetURLLong( _T("TICKETBOXVIEW"), m_nViewID, true) == false)
	{		
		// get the ContactID - as either a URL or FORM param		
		if ( GetISAPIData().GetURLLong( _T("CONTACTID"), m_ContactNote.m_ContactID, true) == false)
		{
			if ( GetISAPIData().GetXMLLong( _T("CONTACTID"), m_ContactNote.m_ContactID, true) == false)
			{
				GetISAPIData().GetFormLong( _T("CONTACTID"), m_ContactNote.m_ContactID);
			}
		}				
	}

	GetISAPIData().GetFormLong( _T("AGENTBOXID"), m_nAgentBoxID, true);

	// get the action
	tstring sAction;
	if( GetISAPIData().GetFormString( _T("action"), sAction, true) == false )
	{
		if( GetISAPIData().GetXMLString( _T("action"), sAction, true) == false )
		{
			GetISAPIData().GetURLString( _T("action"), sAction, true);
		}
	}
	
	if ( sAction.compare( _T("view" )) == 0)
	{
		m_sView = _T("View");
		Query();
	}
	else if ( sAction.compare( _T("edit" )) == 0)
	{
		m_sView = _T("Edit");
		m_ContactNote.m_AgentID = GetAgentID();
		Query();
	}
	else if ( sAction.compare( _T("add")) == 0 ) 
	{
		m_sView = _T("New");
		m_ContactNote.m_AgentID = GetAgentID();
		ReserveID();
		GenerateXML();
	} 
	else if ( sAction.compare( _T("addphone")) == 0 ) 
	{
		m_sView = _T("New");
		m_ContactNote.m_AgentID = GetAgentID();
		ReserveID();
		m_IsPhone = true;
		GenerateXML();
	} 
	else if ( sAction.compare( _T("update")) == 0 )
	{
		Update();
	}	
	else if ( sAction.compare( _T("insert")) == 0 )
	{
		Insert();
	}
	else if ( sAction.compare( _T("delete")) == 0 )
	{
		Delete();
	}
	else
	{	
		List();
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists all ticket notes for the ticket	              
\*--------------------------------------------------------------------------*/
void CContactNotes::List( void )
{
	unsigned char access;
	
	if ( m_ContactNote.m_ContactID > 0 )
	{
		// check security
		access = RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS );
	}
	else
	{
		access = 4;
	}
	
	int nContactNoteID;
	int nAgentID;
	int nNumAttach=0;
	TIMESTAMP_STRUCT DateCreated;
	long DateCreatedLen;	

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;
	
	if ( m_nViewID != 0 && m_ContactNote.m_ContactID == 0 )
	{
		GetQuery().Initialize();
        BINDCOL_LONG_NOLEN(GetQuery(), m_nAgentBoxID );
		BINDPARAM_LONG(GetQuery(), m_nViewID );
		GetQuery().Execute( _T("SELECT AgentBoxID FROM TicketBoxViews ")
			_T("WHERE TicketBoxViewID = ? "));
		GetQuery().Fetch();				
	}

	
	GetQuery().Initialize();	
	BINDCOL_LONG_NOLEN(GetQuery(), nContactNoteID );
	BINDCOL_LONG_NOLEN(GetQuery(), nNumAttach );
	BINDCOL_TIME(GetQuery(), DateCreated );
	BINDCOL_LONG_NOLEN(GetQuery(), nAgentID );
	BINDCOL_BIT(GetQuery(), m_ContactNote.m_IsVoipNote );
	BINDCOL_TCHAR(GetQuery(), m_ContactNote.m_Subject );	
	
	if ( m_nViewID != 0 && m_ContactNote.m_ContactID == 0 )
	{
		BINDPARAM_LONG(GetQuery(), m_nAgentBoxID );
		// excute the query
		GetQuery().Execute( _T("SELECT ContactNoteID, (SELECT COUNT(1) FROM NoteAttachments WHERE NoteID=ContactNoteID AND NoteTypeID=2), DateCreated, AgentID, IsVoipNote, Subject ")
			_T("FROM ContactNotes ")
			_T("WHERE ContactID = 0 AND AgentID = ? ")
			_T("ORDER BY DateCreated"));
	}
	else
	{
		BINDPARAM_LONG(GetQuery(), m_ContactNote.m_ContactID );
		// excute the query
		GetQuery().Execute( _T("SELECT ContactNoteID, (SELECT COUNT(1) FROM NoteAttachments WHERE NoteID=ContactNoteID AND NoteTypeID=2),DateCreated, AgentID, IsVoipNote, Subject ")
			_T("FROM ContactNotes ")
			_T("WHERE ContactID = ? ")
			_T("ORDER BY DateCreated"));
	}
	
	
	// add the TicketID to the XML
	GetXMLGen().AddChildElem( _T("Contact") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_ContactNote.m_ContactID );
	GetXMLGen().AddChildAttrib( _T("Access"), access );
	GetXMLGen().AddChildAttrib( _T("Display"), _T("List") );
	if ( m_nViewID != 0 && m_ContactNote.m_ContactID == 0 )
	{
		GetXMLGen().AddChildAttrib( _T("AgentBoxID"), m_nAgentBoxID );
		AddAgentName( _T("ContactName"), m_nAgentBoxID );		
	}
	
	CEMSString sDateCreated;

	while( ( GetQuery().Fetch() ) == S_OK )
	{
		GetXMLGen().AddChildElem( _T("ContactNote") );
		GetXMLGen().AddChildAttrib( _T("ContactNoteID"), nContactNoteID );
		GetXMLGen().AddChildAttrib( _T("NumAttach"), nNumAttach );
		GetXMLGen().AddChildAttrib( _T("IsPhone"), m_ContactNote.m_IsVoipNote );
		GetXMLGen().AddChildAttrib( _T("Subject"), m_ContactNote.m_Subject );
		GetXMLGen().AddChildAttrib( _T("ContactAgentID"), nAgentID );
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(DateCreated,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDateCreated);
			}
			else
			{
				GetDateTimeString( DateCreated, DateCreatedLen, sDateCreated);
			}
		}
		else
		{
			GetDateTimeString( DateCreated, DateCreatedLen, sDateCreated);
		}			
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDateCreated.c_str() );
		AddAgentName( _T("Agent"), nAgentID );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML for a ticket note...	              
\*--------------------------------------------------------------------------*/
void CContactNotes::Query( void )
{
	// get form parameters
	CEMSString sContactNoteID;
	
	if ( GetISAPIData().GetFormString( _T("selectId"), sContactNoteID, true ) )
		sContactNoteID.CDLGetNextInt( m_ContactNote.m_ContactNoteID );

	if ( m_ContactNote.m_ContactID > 0 )
	{
		// check security
		RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS );
	}
	
	if ( m_ContactNote.Query( GetQuery() ) != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("Error - Invalid ContactNoteID %d\n"), m_ContactNote.m_ContactNoteID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError ); 
	}

	GenerateXML();

	// list the attachments
	CAttachment attachment(m_ISAPIData);
	list<CAttachment> AttachmentList;
	list<CAttachment>::iterator iter;

	attachment.ListNoteAttachments( m_ContactNote.m_ContactNoteID, 2, AttachmentList );

	for ( iter = AttachmentList.begin(); iter != AttachmentList.end(); iter++ )
	{
		iter->GenerateXML();
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Insert a new ticket note...	              
\*--------------------------------------------------------------------------*/
void CContactNotes::Insert( void )
{
	try
	{
		// get the contactnoteid if one exists
		CEMSString sContactNoteID;
	
		if ( GetISAPIData().GetURLString( _T("selectId"), sContactNoteID, true) == false)
		{
			if ( GetISAPIData().GetXMLString( _T("selectId"), sContactNoteID, true) == false)
			{
				GetISAPIData().GetFormString( _T("selectId"), sContactNoteID, true);
			}
		}		
		
		sContactNoteID.CDLGetNextInt( m_ContactNote.m_ContactNoteID );	
		
		// get the AgentID
		m_ContactNote.m_AgentID = GetAgentID();

		if ( m_ContactNote.m_ContactID > 0 )
		{
			// check security
			RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS );
		}

		// Is this a phone note?
		if( GetISAPIData().GetFormLong( _T("IsPhone"), m_ContactNote.m_IsVoipNote, true ) == false)
		{
			if( GetISAPIData().GetURLLong( _T("IsPhone"), m_ContactNote.m_IsVoipNote, true ) == false)
			{
				GetISAPIData().GetXMLLong( _T("IsPhone"), m_ContactNote.m_IsVoipNote);	
			}
		}

		if ( m_ContactNote.m_IsVoipNote )
		{
			// TimeStart
			dca::String sStartTime;
			dca::String sTemp;
			GetISAPIData().GetXMLString( _T("TIMESTART"), sStartTime, true);
			if ( sStartTime.length() > 0 )
			{
				TIMESTAMP_STRUCT ts;				
				tstring sValue;
				GetServerParameter( EMS_SRVPARAM_DATE_ENTRY_FORMAT, sValue );
				if(sValue=="0")
				{
					sTemp = sStartTime.substr(0,2);
					ts.month = sTemp.ToInt();
					sTemp = sStartTime.substr(3,2);
					ts.day = sTemp.ToInt();
				}
				else if(sValue=="1")
				{
					sTemp = sStartTime.substr(0,2);
					ts.day = sTemp.ToInt();
					sTemp = sStartTime.substr(3,2);
					ts.month = sTemp.ToInt();					
				}				
				sTemp = sStartTime.substr(6,4);
				ts.year = sTemp.ToInt();
				sTemp = sStartTime.substr(11,2);
				ts.hour = sTemp.ToInt();
				sTemp = sStartTime.substr(14,2);
				ts.minute = sTemp.ToInt();
				sTemp = sStartTime.substr(17,2);
				ts.second = sTemp.ToInt();
				sTemp = sStartTime.substr(20,2);
				if ( sTemp == "PM" && ts.hour != 12 )
				{
					ts.hour = ts.hour + 12;
				}
				else if ( sTemp == "AM" && ts.hour == 12 )
				{
					ts.hour = 0;
				}
				ts.fraction = 0;
				m_ContactNote.m_StartTime = ts;
				m_ContactNote.m_StartTimeLen = sizeof(m_ContactNote.m_StartTime);
			}
			else
			{
				m_ContactNote.m_StartTimeLen = SQL_NULL_DATA;
			}		
								
			// TimeStop
			dca::String sStopTime;
			GetISAPIData().GetXMLString( _T("TIMESTOP"), sStopTime, true);
			if ( sStopTime.length() > 0 )
			{
				TIMESTAMP_STRUCT ts;
				tstring sValue;
				GetServerParameter( EMS_SRVPARAM_DATE_ENTRY_FORMAT, sValue );
				if(sValue=="0")
				{
					sTemp = sStopTime.substr(0,2);
					ts.month = sTemp.ToInt();
					sTemp = sStopTime.substr(3,2);
					ts.day = sTemp.ToInt();
				}
				else if(sValue=="1")
				{
					sTemp = sStopTime.substr(0,2);
					ts.day = sTemp.ToInt();
					sTemp = sStopTime.substr(3,2);
					ts.month = sTemp.ToInt();					
				}
				sTemp = sStopTime.substr(6,4);
				ts.year = sTemp.ToInt();
				sTemp = sStopTime.substr(11,2);
				ts.hour = sTemp.ToInt();
				sTemp = sStopTime.substr(14,2);
				ts.minute = sTemp.ToInt();
				sTemp = sStopTime.substr(17,2);
				ts.second = sTemp.ToInt();
				sTemp = sStopTime.substr(20,2);
				if ( sTemp == "PM" && ts.hour != 12 )
				{
					ts.hour = ts.hour + 12;
				}
				else if ( sTemp == "AM" && ts.hour == 12 )
				{
					ts.hour = 0;
				}
				ts.fraction = 0;
				m_ContactNote.m_StopTime = ts;
				m_ContactNote.m_StopTimeLen = sizeof(m_ContactNote.m_StopTime);
			}
			else
			{
				m_ContactNote.m_StopTimeLen = SQL_NULL_DATA;
			}
			
			int nSeconds = CompareTimeStamps(m_ContactNote.m_StartTime,m_ContactNote.m_StopTime);
			if ( nSeconds > 0 )
			{
				if ( nSeconds < 60 )
				{
					if ( nSeconds < 10 )
					{
						_stprintf( m_ContactNote.m_ElapsedTime, _T("00:00:0%d"),nSeconds);
					}
					else
					{
						_stprintf( m_ContactNote.m_ElapsedTime, _T("00:00:%d"),nSeconds);						
					}				
				}
				else
				{
					int nMinutes = nSeconds/60;
					nSeconds = nSeconds%60;
					if (nMinutes < 60)
					{
						if (nMinutes < 10)
						{
							if ( nSeconds < 10 )
							{
								_stprintf( m_ContactNote.m_ElapsedTime, _T("00:0%d:0%d"),nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ContactNote.m_ElapsedTime, _T("00:0%d:%d"),nMinutes,nSeconds);
							}
						}
						else
						{
							if ( nSeconds < 10 )
							{
								_stprintf( m_ContactNote.m_ElapsedTime, _T("00:%d:0%d"),nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ContactNote.m_ElapsedTime, _T("00:%d:%d"),nMinutes,nSeconds);
							}
						}
					}
					else
					{
						int nHours = nMinutes/60;
						nMinutes = nMinutes%60;
						if (nHours < 10)
						{
							if (nMinutes < 10)
							{
								if ( nSeconds < 10 )
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("0%d:0%d:0%d"),nHours,nMinutes,nSeconds);
								}
								else
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("0%d:0%d:%d"),nHours,nMinutes,nSeconds);
								}
							}
							else
							{
								if ( nSeconds < 10 )
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("0%d:%d:0%d"),nHours,nMinutes,nSeconds);
								}
								else
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("0%d:%d:%d"),nHours,nMinutes,nSeconds);
								}
							}
						}
						else
						{
							if (nMinutes < 10)
							{
								if ( nSeconds < 10 )
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("%d:0%d:0%d"),nHours,nMinutes,nSeconds);
								}
								else
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("%d:0%d:%d"),nHours,nMinutes,nSeconds);
								}
							}
							else
							{
								if ( nSeconds < 10 )
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("%d:%d:0%d"),nHours,nMinutes,nSeconds);
								}
								else
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("%d:%d:%d"),nHours,nMinutes,nSeconds);
								}
							}
						}
					}
				}
			}
			else
			{
				_stprintf( m_ContactNote.m_ElapsedTime, _T("00:00:00"));	
			}
		}
		else
		{
			_stprintf( m_ContactNote.m_ElapsedTime, _T(""));
			m_ContactNote.m_StopTimeLen = SQL_NULL_DATA;
			m_ContactNote.m_StopTimeLen = SQL_NULL_DATA;		
		}
		
		// get the subject
		GetISAPIData().GetXMLTCHAR( _T("subject"), m_ContactNote.m_Subject, 51 );

		// get the actual ticket note
		GetISAPIData().GetXMLTCHAR( _T("NOTE"), &m_ContactNote.m_Note, m_ContactNote.m_NoteLen, m_ContactNote.m_NoteAllocated );			
		
		GetTimeStamp( m_ContactNote.m_DateCreated );
		m_ContactNote.m_DateCreatedLen = sizeof( m_ContactNote.m_DateCreated );
		if( m_ContactNote.m_ContactNoteID > 0 )
		{
			m_ContactNote.Update( GetQuery() );
		}
		else
		{
			m_ContactNote.Insert( GetQuery() );
		}	
		
		List();
	}
	catch(...)
	{
		m_sView = _T("New");
		m_ContactNote.m_ContactNoteID = 0;
		GenerateXML();	
		throw;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Update a ticket note...	              
\*--------------------------------------------------------------------------*/
void CContactNotes::Update( void )
{
	try
	{
		CEMSString sContactNoteID;
	
		if ( GetISAPIData().GetURLString( _T("selectId"), sContactNoteID, true) == false)
		{
			if ( GetISAPIData().GetXMLString( _T("selectId"), sContactNoteID, true) == false)
			{
				GetISAPIData().GetFormString( _T("selectId"), sContactNoteID);
			}
		}		
		
		//GetISAPIData().GetFormString( _T("selectId"), sContactNoteID, true );
		sContactNoteID.CDLGetNextInt( m_ContactNote.m_ContactNoteID );		
		
				
		if ( !m_ContactNote.m_ContactNoteID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("A ContactNoteID must be specified") );

		m_ContactNote.Query( GetQuery() );
		
		// get the AgentID
		m_ContactNote.m_AgentID = GetAgentID();

		if ( m_ContactNote.m_ContactID > 0 )
		{
			// check security
			RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS );
		}

		// Is this a phone note?
		if( GetISAPIData().GetFormLong( _T("IsPhone"), m_ContactNote.m_IsVoipNote, true ) == false)
		{
			if( GetISAPIData().GetURLLong( _T("IsPhone"), m_ContactNote.m_IsVoipNote, true ) == false)
			{
				GetISAPIData().GetXMLLong( _T("IsPhone"), m_ContactNote.m_IsVoipNote);	
			}
		}

		if ( m_ContactNote.m_IsVoipNote )
		{
			// TimeStart
			dca::String sStartTime;
			dca::String sTemp;
			GetISAPIData().GetXMLString( _T("TIMESTART"), sStartTime, true);
			if ( sStartTime.length() > 0 )
			{
				TIMESTAMP_STRUCT ts;
				tstring sValue;
				GetServerParameter( EMS_SRVPARAM_DATE_ENTRY_FORMAT, sValue );
				if(sValue=="0")
				{
					sTemp = sStartTime.substr(0,2);
					ts.month = sTemp.ToInt();
					sTemp = sStartTime.substr(3,2);
					ts.day = sTemp.ToInt();
				}
				else if(sValue=="1")
				{
					sTemp = sStartTime.substr(0,2);
					ts.day = sTemp.ToInt();
					sTemp = sStartTime.substr(3,2);
					ts.month = sTemp.ToInt();					
				}	
				sTemp = sStartTime.substr(6,4);
				ts.year = sTemp.ToInt();
				sTemp = sStartTime.substr(11,2);
				ts.hour = sTemp.ToInt();
				sTemp = sStartTime.substr(14,2);
				ts.minute = sTemp.ToInt();
				sTemp = sStartTime.substr(17,2);
				ts.second = sTemp.ToInt();
				sTemp = sStartTime.substr(20,2);
				if ( sTemp == "PM" && ts.hour != 12 )
				{
					ts.hour = ts.hour + 12;
				}
				else if ( sTemp == "AM" && ts.hour == 12 )
				{
					ts.hour = 0;
				}
				ts.fraction = 0;
				m_ContactNote.m_StartTime = ts;
				m_ContactNote.m_StartTimeLen = sizeof(m_ContactNote.m_StartTime);
			}
			else
			{
				m_ContactNote.m_StartTimeLen = SQL_NULL_DATA;
			}		
								
			// TimeStop
			dca::String sStopTime;
			GetISAPIData().GetXMLString( _T("TIMESTOP"), sStopTime, true);
			if ( sStopTime.length() > 0 )
			{
				TIMESTAMP_STRUCT ts;
				tstring sValue;
				GetServerParameter( EMS_SRVPARAM_DATE_ENTRY_FORMAT, sValue );
				if(sValue=="0")
				{
					sTemp = sStopTime.substr(0,2);
					ts.month = sTemp.ToInt();
					sTemp = sStopTime.substr(3,2);
					ts.day = sTemp.ToInt();
				}
				else if(sValue=="1")
				{
					sTemp = sStopTime.substr(0,2);
					ts.day = sTemp.ToInt();
					sTemp = sStopTime.substr(3,2);
					ts.month = sTemp.ToInt();					
				}
				sTemp = sStopTime.substr(6,4);
				ts.year = sTemp.ToInt();
				sTemp = sStopTime.substr(11,2);
				ts.hour = sTemp.ToInt();
				sTemp = sStopTime.substr(14,2);
				ts.minute = sTemp.ToInt();
				sTemp = sStopTime.substr(17,2);
				ts.second = sTemp.ToInt();
				sTemp = sStopTime.substr(20,2);
				if ( sTemp == "PM" && ts.hour != 12 )
				{
					ts.hour = ts.hour + 12;
				}
				else if ( sTemp == "AM" && ts.hour == 12 )
				{
					ts.hour = 0;
				}
				ts.fraction = 0;
				m_ContactNote.m_StopTime = ts;
				m_ContactNote.m_StopTimeLen = sizeof(m_ContactNote.m_StopTime);
			}
			else
			{
				m_ContactNote.m_StopTimeLen = SQL_NULL_DATA;
			}
			
			int nSeconds = CompareTimeStamps(m_ContactNote.m_StartTime,m_ContactNote.m_StopTime);
			if ( nSeconds > 0 )
			{
				if ( nSeconds < 60 )
				{
					if ( nSeconds < 10 )
					{
						_stprintf( m_ContactNote.m_ElapsedTime, _T("00:00:0%d"),nSeconds);
					}
					else
					{
						_stprintf( m_ContactNote.m_ElapsedTime, _T("00:00:%d"),nSeconds);						
					}				
				}
				else
				{
					int nMinutes = nSeconds/60;
					nSeconds = nSeconds%60;
					if (nMinutes < 60)
					{
						if (nMinutes < 10)
						{
							if ( nSeconds < 10 )
							{
								_stprintf( m_ContactNote.m_ElapsedTime, _T("00:0%d:0%d"),nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ContactNote.m_ElapsedTime, _T("00:0%d:%d"),nMinutes,nSeconds);
							}
						}
						else
						{
							if ( nSeconds < 10 )
							{
								_stprintf( m_ContactNote.m_ElapsedTime, _T("00:%d:0%d"),nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ContactNote.m_ElapsedTime, _T("00:%d:%d"),nMinutes,nSeconds);
							}
						}
					}
					else
					{
						int nHours = nMinutes/60;
						nMinutes = nMinutes%60;
						if (nHours < 10)
						{
							if (nMinutes < 10)
							{
								if ( nSeconds < 10 )
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("0%d:0%d:0%d"),nHours,nMinutes,nSeconds);
								}
								else
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("0%d:0%d:%d"),nHours,nMinutes,nSeconds);
								}
							}
							else
							{
								if ( nSeconds < 10 )
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("0%d:%d:0%d"),nHours,nMinutes,nSeconds);
								}
								else
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("0%d:%d:%d"),nHours,nMinutes,nSeconds);
								}
							}
						}
						else
						{
							if (nMinutes < 10)
							{
								if ( nSeconds < 10 )
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("%d:0%d:0%d"),nHours,nMinutes,nSeconds);
								}
								else
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("%d:0%d:%d"),nHours,nMinutes,nSeconds);
								}
							}
							else
							{
								if ( nSeconds < 10 )
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("%d:%d:0%d"),nHours,nMinutes,nSeconds);
								}
								else
								{
									_stprintf( m_ContactNote.m_ElapsedTime, _T("%d:%d:%d"),nHours,nMinutes,nSeconds);
								}
							}
						}
					}
				}
			}
			else
			{
				_stprintf( m_ContactNote.m_ElapsedTime, _T("00:00:00"));	
			}
		}
		else
		{
			_stprintf( m_ContactNote.m_ElapsedTime, _T(""));
			m_ContactNote.m_StopTimeLen = SQL_NULL_DATA;
			m_ContactNote.m_StopTimeLen = SQL_NULL_DATA;		
		}
		
		// get the subject
		
		if ( GetISAPIData().GetURLTCHAR( _T("subject"), m_ContactNote.m_Subject,51, true) == false)
		{
			if ( GetISAPIData().GetXMLTCHAR( _T("subject"), m_ContactNote.m_Subject,51, true) == false)
			{
				GetISAPIData().GetFormTCHAR( _T("subject"), m_ContactNote.m_Subject, 51 );
			}
		}	

		// get the actual ticket note
		
		
		if ( GetISAPIData().GetURLTCHAR( _T("NOTE"), m_ContactNote.m_Note, m_ContactNote.m_NoteLen, true) == false)
		{
			
				GetISAPIData().GetFormTCHAR( _T("NOTE"), &m_ContactNote.m_Note, m_ContactNote.m_NoteLen, m_ContactNote.m_NoteAllocated );			
			
		}	

		GetTimeStamp( m_ContactNote.m_DateCreated );
		m_ContactNote.m_DateCreatedLen = sizeof( m_ContactNote.m_DateCreated );
		m_ContactNote.Update( GetQuery() );
		
		List();
	}
	catch(...)
	{
		m_sView = _T("Edit");
		m_ContactNote.m_ContactNoteID = 0;
		GenerateXML();	
		throw;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes a contact note...	              
\*--------------------------------------------------------------------------*/
void CContactNotes::Delete( void )
{
	if ( m_ContactNote.m_ContactID > 0 )
	{
		// check security
		RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_DELETE_ACCESS );
	}	
	
	// get form parameters
	CEMSString sTicketNoteID;
	
	// delete each contact note in the comma delimited list
	if ( GetISAPIData().GetFormString( _T("selectId"), sTicketNoteID, true ) )
	{
		while(sTicketNoteID.CDLGetNextInt( m_ContactNote.m_ContactNoteID ))
		{
			DeleteAttachments( m_ContactNote.m_ContactNoteID );
			m_ContactNote.Delete( GetQuery() );
		}
	}

	List();
}

void CContactNotes::DeleteAttachments( int nContactNoteID )
{
	CAttachment attachment(m_ISAPIData);
	list<CAttachment> AttachmentList;
	list<CAttachment>::iterator iter;

	// list the contact note attachments
	attachment.ListNoteAttachments( nContactNoteID, 2, AttachmentList );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nContactNoteID );
	GetQuery().Execute(_T("DELETE FROM NoteAttachments WHERE NoteID=? AND NoteTypeID=2") );

	for ( iter = AttachmentList.begin(); iter != AttachmentList.end(); iter++ )
	{
		if ( GetAttachmentReferenceCount( GetQuery(), iter->m_AttachmentID ) == 0 )
		{
			iter->DeleteNoteAttachment();
		}			
	}	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a new ContactNoteID.       
\*--------------------------------------------------------------------------*/
int CContactNotes::ReserveID(void)
{
	// insert a new contact note
	// check security
	if ( m_ContactNote.m_ContactID > 0 )
	{
		// check security
		RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS );
	}
	
	GetTimeStamp(m_ContactNote.m_DateCreated );
	m_ContactNote.m_DateCreatedLen = sizeof( m_ContactNote.m_DateCreated );
	m_ContactNote.Insert( GetQuery() );

	return m_ContactNote.m_ContactNoteID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML	              
\*--------------------------------------------------------------------------*/
void CContactNotes::GenerateXML( void )
{
	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;
	
	GetXMLGen().AddChildElem( _T("Contact") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_ContactNote.m_ContactID);
	GetXMLGen().AddChildAttrib( _T("Display"), m_sView.c_str() );
	GetXMLGen().AddChildAttrib( _T("AgentBoxID"), m_nAgentBoxID );

	GetXMLGen().AddChildElem( _T("ContactNote") );
	GetXMLGen().AddChildAttrib( _T("ContactNoteID"), m_ContactNote.m_ContactNoteID );
	GetXMLGen().AddChildAttrib( _T("NumAttach"), m_ContactNote.m_NumAttach );		
	GetXMLGen().AddChildAttrib( _T("ContactAgentID"), m_ContactNote.m_AgentID );
	AddAgentName( _T("Agent"), m_ContactNote.m_AgentID );
	if ( m_ContactNote.m_IsVoipNote )
	{
		m_IsPhone = true;
	}
	GetXMLGen().AddChildAttrib( _T("IsPhone"), m_IsPhone );
	GetXMLGen().AddChildAttrib( _T("Subject"), m_ContactNote.m_Subject );
	GetXMLGen().AddChildAttrib( _T("TimeTextArea"), m_ContactNote.m_ElapsedTime );
	if(m_ContactNote.m_StartTimeLen > 0)
	{
		CEMSString sTimeStart;
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(m_ContactNote.m_StartTime,lTzBias,tsLocal))
			{
				GetFullDateTimeString( tsLocal, tsLocalLen, sTimeStart );				
			}
			else
			{
				GetFullDateTimeString( m_ContactNote.m_StartTime, m_ContactNote.m_StartTimeLen, sTimeStart);
			}
		}
		else
		{
			GetFullDateTimeString( m_ContactNote.m_StartTime, m_ContactNote.m_StartTimeLen, sTimeStart);
		}		
		GetXMLGen().AddChildAttrib(_T("TimeStart"), sTimeStart.c_str());
	}
	else
	{
		CEMSString sTimeStart;
		GetXMLGen().AddChildAttrib(_T("TimeStart"), sTimeStart.c_str());
	}
	if(m_ContactNote.m_StopTimeLen > 0)
	{
		CEMSString sTimeStop;
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(m_ContactNote.m_StopTime,lTzBias,tsLocal))
			{
				GetFullDateTimeString( tsLocal, tsLocalLen, sTimeStop);				
			}
			else
			{
				GetFullDateTimeString( m_ContactNote.m_StopTime, m_ContactNote.m_StopTimeLen, sTimeStop);
			}
		}
		else
		{
			GetFullDateTimeString( m_ContactNote.m_StopTime, m_ContactNote.m_StopTimeLen, sTimeStop);
		}		
		GetXMLGen().AddChildAttrib(_T("TimeStop"), sTimeStop.c_str());
	}
	else
	{
		CEMSString sTimeStop;
		GetXMLGen().AddChildAttrib(_T("TimeStop"), sTimeStop.c_str());
	}

	CEMSString sDateCreated;
	if(lTzBias != 1)
	{
		if(ConvertToTimeZone(m_ContactNote.m_DateCreated,lTzBias,tsLocal))
		{
			GetDateTimeString( tsLocal, tsLocalLen, sDateCreated);
		}
		else
		{
			GetDateTimeString( m_ContactNote.m_DateCreated, m_ContactNote.m_DateCreatedLen, sDateCreated);
		}
	}
	else
	{
		GetDateTimeString( m_ContactNote.m_DateCreated, m_ContactNote.m_DateCreatedLen, sDateCreated);
	}
	GetXMLGen().AddChildAttrib(_T("DateCreated"), sDateCreated.c_str());

	GetXMLGen().AddAttrib( _T("Access"), GetAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0 ) );
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("Body") );
	GetXMLGen().SetChildData( m_ContactNote.m_Note, TRUE );
	GetXMLGen().OutOfElem();

	TDictionary dc;
	int nDcEnabled = 0;
	dc.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("Dictionaries") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		if(dc.m_IsEnabled){nDcEnabled++;}
		GetXMLGen().AddChildElem( _T("Dictionary") );
		GetXMLGen().AddChildAttrib( _T("ID"), dc.m_DictionaryID );
		GetXMLGen().AddChildAttrib( _T("Description"), dc.m_Description );
		GetXMLGen().AddChildAttrib( _T("TlxFile"), dc.m_TlxFile );
		GetXMLGen().AddChildAttrib( _T("ClxFile"), dc.m_ClxFile );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), dc.m_IsEnabled );
	}
	GetXMLGen().OutOfElem();
	tstring sDictionaryID;
	GetServerParameter( EMS_SRVPARAM_DEFAULT_DICTIONARY_ID, sDictionaryID, "1" );
	int nDictionaryID = _ttoi( sDictionaryID.c_str() );
	GetXMLGen().AddChildElem(_T("Spell"));
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(_T("Check"));
	GetXMLGen().AddChildAttrib( _T("DcEnabled"), nDcEnabled );
	if(nDcEnabled > 1)
	{
		TAgents a;
		a.m_AgentID = GetAgentID();
		a.Query(GetQuery());
		if(a.m_DictionaryID != nDictionaryID){nDictionaryID=a.m_DictionaryID;}
		GetXMLGen().AddChildAttrib( _T("DictionaryPrompt"), a.m_DictionaryPrompt );
	}	
	GetXMLGen().AddChildAttrib( _T("DictionaryID"), nDictionaryID );		
	GetXMLGen().OutOfElem();
}
