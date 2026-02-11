/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketNotes.cpp,v 1.2.2.1.2.1 2006/07/18 12:55:03 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Ticket Notes
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketNotes.h"
#include "Ticket.h"
#include "TicketHistoryFns.h"
#include "AttachFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CTicketNotes::Run(CURLAction& action)
{
	CEMSString sString;
	CEMSString sType;
	m_IsPhone = false;

	// get the TicketID		
	if ( GetISAPIData().GetURLString( _T("TICKETID"), sString, true ) ||
		 GetISAPIData().GetURLString( _T("TICKET"), sString, true ) ||
		 GetISAPIData().GetXMLString( _T("TICKETID"), sString, true ))
	{
		sString.CDLGetNextInt( m_TicketID );
	}

	// get the Ticket Note ID
	if (( GetISAPIData().GetXMLString( _T("SELECTID"), sString, true )) || 
		( GetISAPIData().GetURLString( _T("SELECTID"), sString, true )))
	{
		sString.CDLInit();
		sString.CDLGetNextInt( m_TicketNoteID );
	}

	if( GetISAPIData().m_sPage.compare( _T("updateticketnotes") ) == 0)
		sString.assign(_T("updateread"));
	
	// get the action
	GetISAPIData().GetXMLString( _T("action"), sString, true);

	// is this a phone note?
	if (( GetISAPIData().GetXMLString( _T("type"), sType, true )) || 
		( GetISAPIData().GetURLString( _T("type"), sType, true )))
	{
		if ( sType.compare( _T("phone")) == 0 )
		{
			m_IsPhone = true;
		}
	}

	action.m_sPageTitle.assign( "Ticket Note" );

	if ( sString.compare( _T("insert")) == 0 )
	{
		Insert();
		SetBeenRead();
	}
	else if ( sString.compare( _T("update")) == 0 )
	{
		Update();
	}
	else if ( sString.compare( _T("updateread")) == 0)
	{
		UpdateRead();
	}
	else if ( sString.compare( _T("delete")) == 0 )
	{
		Delete();
	}
	else if ( m_TicketNoteID > 0 )
	{
		Query();
	}
	else if ( sString.compare( _T("0")) == 0 )
	{
		m_sView = _T("New");
		m_AgentID = GetAgentID();

		action.m_sPageTitle.assign( "New Ticket Note" );

		ReserveID();
		SetBeenRead();

		GenerateXML();
	}
	else
	{		
		action.m_sPageTitle.assign( "Ticket Notes" );
		ListTicketContacts( m_TicketID );
		List();
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists all ticket notes for the ticket	              
\*--------------------------------------------------------------------------*/
void CTicketNotes::List( void )
{
	
	CEMSString sString;
	if ( !m_TicketID )
	{
		// see if omsg is defined... if so, get the ticketid using the outboundmessageid
		int nOutMsgID;
		
		if ( GetISAPIData().GetURLString( _T("OMSG"), sString, true ) ||
		 GetISAPIData().GetURLString( _T("OMSG"), sString, true ) ||
		 GetISAPIData().GetXMLString( _T("OMSG"), sString, true ))
		{
			sString.CDLGetNextInt( nOutMsgID );
			GetQuery().Initialize();	
			BINDCOL_LONG(GetQuery(), m_TicketID );			
			BINDPARAM_LONG(GetQuery(), nOutMsgID );			
			GetQuery().Execute( _T("SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID = ?"));
			if(( GetQuery().Fetch() ) != S_OK )
			{
				THROW_EMS_EXCEPTION( E_InvalidID, _T("A TicketID must be specified") );
			}
		}
		else
		{
			THROW_EMS_EXCEPTION( E_InvalidID, _T("A TicketID must be specified") );
		}
	}
		

	CTicket Ticket( m_ISAPIData, m_TicketID );
	unsigned char access;
	CEMSString sDateCreated;

	std::list<TTicketNotes> noteList;	

	// require read access to the ticket
	access = Ticket.RequireRead();

	// set the view type
	m_sView = _T("List");

	GetQuery().Initialize();
	
	BINDCOL_LONG(GetQuery(), m_TicketNoteID );
	BINDCOL_LONG(GetQuery(), m_NumAttach );
	BINDCOL_TIME(GetQuery(), m_DateCreated );
	BINDCOL_LONG(GetQuery(), m_AgentID );
	BINDCOL_BIT(GetQuery(), m_IsVoipNote );
	BINDPARAM_LONG(GetQuery(), m_TicketID );
	
	// excute the query
	GetQuery().Execute( _T("SELECT TicketNoteID, (SELECT COUNT(1) FROM NoteAttachments WHERE NoteID=TicketNoteID AND NoteTypeID=1), DateCreated, AgentID, IsVoipNote, Note ")
		                _T("FROM TicketNotes WHERE TicketID = ? ORDER BY DateCreated DESC"));
	
	// add the TicketID to the XML
	GetXMLGen().AddChildElem( _T("Ticket") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_TicketID );
	GetXMLGen().AddChildAttrib( _T("Display"), _T("List") );
	GetXMLGen().AddChildAttrib( _T("Access"), access );
	
	while( ( GetQuery().Fetch() ) == S_OK )
	{		
		// get the note text data
		GETDATA_TEXT( GetQuery(), m_Note );

		noteList.push_back(*this);
	}

	std::list<TTicketNotes>::iterator iNote;

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;
	
	for(iNote = noteList.begin();
		iNote != noteList.end();
		iNote++)
	{
		// if the note is too long, truncate it
		if( m_NoteLen > 64 )
		{
			// add ellipsis
			m_Note[64] = _T('\0');
			m_Note[63] = _T('.');
			m_Note[62] = _T('.');
			m_Note[61] = _T('.');
		}
		
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone((*iNote).m_DateCreated,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDateCreated );
			}
			else
			{
				GetDateTimeString( (*iNote).m_DateCreated, (*iNote).m_DateCreatedLen, sDateCreated );
			}
		}
		else
		{
			GetDateTimeString( (*iNote).m_DateCreated, (*iNote).m_DateCreatedLen, sDateCreated );
		}
				
		m_TicketNoteID = (*iNote).m_TicketNoteID;
		SetBeenRead();
		
		GetXMLGen().AddChildElem( _T("TicketNote") );		
		GetXMLGen().AddChildAttrib( _T("IsVoip"), (*iNote).m_IsVoipNote );
		GetXMLGen().AddChildAttrib( _T("TicketNoteID"), m_TicketNoteID );
		GetXMLGen().AddChildAttrib( _T("NumAttach"), (*iNote).m_NumAttach );
		GetXMLGen().AddChildAttrib( _T("usecuttoffdate"), m_UseCutoffDate );
		GetXMLGen().AddChildAttrib( _T("beenread"), m_BeenRead);
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDateCreated.c_str() );
		AddAgentName( _T("Agent"), (*iNote).m_AgentID, _T("Unknown") );
		
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("Preview") );
		GetXMLGen().SetChildData( (*iNote).m_Note, TRUE );
		GetXMLGen().OutOfElem();
	}	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML for a ticket note...	              
\*--------------------------------------------------------------------------*/
void CTicketNotes::Query( void )
{
	if ( !m_TicketNoteID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("A TicketNoteID must be specified") );

	unsigned char bEdit = 0;
	GetISAPIData().GetURLLong( _T("EDIT"), bEdit, true );

	m_sView = bEdit ?  _T("Edit") : _T("View");
	
	// query the ticket note
	if ( TTicketNotes::Query( GetQuery() ) != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("Error - Invalid TicketNoteID %d\n"), m_TicketNoteID ); 
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	SetBeenRead();
	
	// check security - do this here to ensure that we have the ID
	CTicket Ticket( m_ISAPIData, m_TicketID );
	Ticket.RequireRead();
		
	GenerateXML();

	// list the attachments
	CAttachment attachment(m_ISAPIData);
	list<CAttachment> AttachmentList;
	list<CAttachment>::iterator iter;

	attachment.ListNoteAttachments( m_TicketNoteID, 1, AttachmentList );

	for ( iter = AttachmentList.begin(); iter != AttachmentList.end(); iter++ )
	{
		iter->GenerateXML();
	}
} 
		 

/*---------------------------------------------------------------------------\                     
||  Comments:	Insert a new ticket note...	              
\*--------------------------------------------------------------------------*/
void CTicketNotes::Insert( void )
{
	// sanity check
	if ( !m_TicketID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("A TicketID must be specified") );

	// get the note from the form
	GetISAPIData().GetXMLTCHAR( _T("NOTE"), &m_Note, m_NoteLen, m_NoteAllocated );
	
	// Is this a VOIP note?
	GetISAPIData().GetXMLLong( _T("IsVoipNote"), m_IsVoipNote );
	
	if ( m_IsVoipNote )
	{
		// TimeStart
		dca::String sStartTime;
		dca::String sTemp;
		GetISAPIData().GetXMLString( _T("timestart"), sStartTime, true);
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
			m_StartTime = ts;
			m_StartTimeLen = sizeof(m_StartTime);
		}
		else
		{
			m_StartTimeLen = SQL_NULL_DATA;
		}		
							
		// TimeStop
		dca::String sStopTime;
		GetISAPIData().GetXMLString( _T("timestop"), sStopTime, true);
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
			m_StopTime = ts;
			m_StopTimeLen = sizeof(m_StopTime);
		}
		else
		{
			m_StopTimeLen = SQL_NULL_DATA;
		}

		int nSeconds = CompareTimeStamps(m_StartTime,m_StopTime);
		if ( nSeconds > 0 )
		{
			if ( nSeconds < 60 )
			{
				if ( nSeconds < 10 )
				{
					_stprintf( m_ElapsedTime, _T("00:00:0%d"),nSeconds);
				}
				else
				{
					_stprintf( m_ElapsedTime, _T("00:00:%d"),nSeconds);						
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
							_stprintf( m_ElapsedTime, _T("00:0%d:0%d"),nMinutes,nSeconds);
						}
						else
						{
							_stprintf( m_ElapsedTime, _T("00:0%d:%d"),nMinutes,nSeconds);
						}
					}
					else
					{
						if ( nSeconds < 10 )
						{
							_stprintf( m_ElapsedTime, _T("00:%d:0%d"),nMinutes,nSeconds);
						}
						else
						{
							_stprintf( m_ElapsedTime, _T("00:%d:%d"),nMinutes,nSeconds);
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
								_stprintf( m_ElapsedTime, _T("0%d:0%d:0%d"),nHours,nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ElapsedTime, _T("0%d:0%d:%d"),nHours,nMinutes,nSeconds);
							}
						}
						else
						{
							if ( nSeconds < 10 )
							{
								_stprintf( m_ElapsedTime, _T("0%d:%d:0%d"),nHours,nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ElapsedTime, _T("0%d:%d:%d"),nHours,nMinutes,nSeconds);
							}
						}
					}
					else
					{
						if (nMinutes < 10)
						{
							if ( nSeconds < 10 )
							{
								_stprintf( m_ElapsedTime, _T("%d:0%d:0%d"),nHours,nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ElapsedTime, _T("%d:0%d:%d"),nHours,nMinutes,nSeconds);
							}
						}
						else
						{
							if ( nSeconds < 10 )
							{
								_stprintf( m_ElapsedTime, _T("%d:%d:0%d"),nHours,nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ElapsedTime, _T("%d:%d:%d"),nHours,nMinutes,nSeconds);
							}
						}
					}
				}
			}
		}
		else
		{
			_stprintf( m_ElapsedTime, _T("00:00:00"));	
		}
		
		// ContactID
		GetISAPIData().GetXMLLong( _T("ContactID"), m_ContactID, true );
		
		if ( m_ContactID < 1 )
			m_ContactID = 0;
	}
	else
	{
		_stprintf( m_ElapsedTime, _T(""));	
		m_ContactID = 0;
		m_StopTimeLen = SQL_NULL_DATA;
		m_StopTimeLen = SQL_NULL_DATA;		
	}	

	// check security
	CTicket Ticket( m_ISAPIData, m_TicketID );
	Ticket.RequireEdit();
	
	m_AgentID = GetAgentID();
	GetTimeStamp(m_DateCreated );
	m_DateCreatedLen = sizeof( m_DateCreated );
	TTicketNotes::Insert( GetQuery() );

	// Log it in system-generated ticket history
	THAddNote( GetQuery(), m_TicketID, GetAgentID(), m_TicketNoteID );

	//Mark the note we just added as read
	UpdateRead();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Insert a new ticket note...	              
\*--------------------------------------------------------------------------*/
void CTicketNotes::Update( void )
{
	if ( !m_TicketNoteID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("A TicketNoteID must be specified") );

	TTicketNotes::Query( GetQuery() );

	// check security
	if ( m_AgentID != GetAgentID() )
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_EDIT_ACCESS );

	// get the note from the form
	GetISAPIData().GetXMLTCHAR( _T("NOTE"), &m_Note, m_NoteLen, m_NoteAllocated );

	// Is this a VOIP note?
	GetISAPIData().GetXMLLong( _T("IsVoipNote"), m_IsVoipNote );
	
	if ( m_IsVoipNote )
	{
		// TimeStart
		dca::String sStartTime;
		dca::String sTemp;
		GetISAPIData().GetXMLString( _T("timestart"), sStartTime, true);
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
			m_StartTime = ts;
			m_StartTimeLen = sizeof(m_StartTime);
		}
		else
		{
			m_StartTimeLen = SQL_NULL_DATA;
		}		
							
		// TimeStart
		dca::String sStopTime;
		GetISAPIData().GetXMLString( _T("timestop"), sStopTime, true);
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
			m_StopTime = ts;
			m_StopTimeLen = sizeof(m_StopTime);
		}
		else
		{
			m_StopTimeLen = SQL_NULL_DATA;
		}

		int nSeconds = CompareTimeStamps(m_StartTime,m_StopTime);
		if ( nSeconds > 0 )
		{
			if ( nSeconds < 60 )
			{
				if ( nSeconds < 10 )
				{
					_stprintf( m_ElapsedTime, _T("00:00:0%d"),nSeconds);
				}
				else
				{
					_stprintf( m_ElapsedTime, _T("00:00:%d"),nSeconds);						
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
							_stprintf( m_ElapsedTime, _T("00:0%d:0%d"),nMinutes,nSeconds);
						}
						else
						{
							_stprintf( m_ElapsedTime, _T("00:0%d:%d"),nMinutes,nSeconds);
						}
					}
					else
					{
						if ( nSeconds < 10 )
						{
							_stprintf( m_ElapsedTime, _T("00:%d:0%d"),nMinutes,nSeconds);
						}
						else
						{
							_stprintf( m_ElapsedTime, _T("00:%d:%d"),nMinutes,nSeconds);
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
								_stprintf( m_ElapsedTime, _T("0%d:0%d:0%d"),nHours,nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ElapsedTime, _T("0%d:0%d:%d"),nHours,nMinutes,nSeconds);
							}
						}
						else
						{
							if ( nSeconds < 10 )
							{
								_stprintf( m_ElapsedTime, _T("0%d:%d:0%d"),nHours,nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ElapsedTime, _T("0%d:%d:%d"),nHours,nMinutes,nSeconds);
							}
						}
					}
					else
					{
						if (nMinutes < 10)
						{
							if ( nSeconds < 10 )
							{
								_stprintf( m_ElapsedTime, _T("%d:0%d:0%d"),nHours,nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ElapsedTime, _T("%d:0%d:%d"),nHours,nMinutes,nSeconds);
							}
						}
						else
						{
							if ( nSeconds < 10 )
							{
								_stprintf( m_ElapsedTime, _T("%d:%d:0%d"),nHours,nMinutes,nSeconds);
							}
							else
							{
								_stprintf( m_ElapsedTime, _T("%d:%d:%d"),nHours,nMinutes,nSeconds);
							}
						}
					}
				}
			}
		}
		else
		{
			_stprintf( m_ElapsedTime, _T("00:00:00"));	
		}
	}
	else
	{
		_stprintf( m_ElapsedTime, _T(""));		
		m_StopTimeLen = SQL_NULL_DATA;
		m_StopTimeLen = SQL_NULL_DATA;		
	}	
	
	// set other fields
	m_AgentID = GetAgentID();
	GetTimeStamp( m_DateCreated );
	m_DateCreatedLen = sizeof( m_DateCreated );
	TTicketNotes::Update( GetQuery() );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes a ticket note...	              
\*--------------------------------------------------------------------------*/
void CTicketNotes::Delete( void )
{
	// get form parameters
	CEMSString sTicketNoteID;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketNoteID );

	while ( sTicketNoteID.CDLGetNextInt( m_TicketNoteID ) )
	{
		// require delete rights for the author of the ticket note
		TTicketNotes::Query( GetQuery() );

		if ( m_AgentID != GetAgentID() )
			RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_DELETE_ACCESS );

		dca::String sCmd;
		sCmd.Format(_T("DELETE FROM TicketNotesRead WHERE TicketNoteID = %d"), m_TicketNoteID);
		GetQuery().Initialize();
		GetQuery().Execute(sCmd.c_str());

		// delete any ticketnote attachments

		TTicketNotes::Delete( GetQuery() );

		// Log it in system-generated ticket history
		THDelNote( GetQuery(), m_TicketID, GetAgentID(), m_TicketNoteID );
	}
}

void CTicketNotes::DeleteAttachments( int nTicketNoteID )
{
	CAttachment attachment(m_ISAPIData);
	list<CAttachment> AttachmentList;
	list<CAttachment>::iterator iter;

	// list the ticket note attachments
	attachment.ListNoteAttachments( nTicketNoteID, 1, AttachmentList );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nTicketNoteID );
	GetQuery().Execute(_T("DELETE FROM NoteAttachments WHERE NoteID=? AND NoteTypeID=1") );

	for ( iter = AttachmentList.begin(); iter != AttachmentList.end(); iter++ )
	{
		if ( GetAttachmentReferenceCount( GetQuery(), iter->m_AttachmentID ) == 0 )
		{
			iter->DeleteNoteAttachment();
		}			
	}	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a new TicketNoteID.       
\*--------------------------------------------------------------------------*/
int CTicketNotes::ReserveID(void)
{
	// insert a new ticket note
	// check security
	CTicket Ticket( m_ISAPIData, m_TicketID );
	Ticket.RequireEdit();
	
	m_AgentID = GetAgentID();
	GetTimeStamp(m_DateCreated );
	m_DateCreatedLen = sizeof( m_DateCreated );
	TTicketNotes::Insert( GetQuery() );

	// Log it in system-generated ticket history
	THAddNote( GetQuery(), m_TicketID, GetAgentID(), m_TicketNoteID );

	return m_TicketNoteID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML	              
\*--------------------------------------------------------------------------*/
void CTicketNotes::GenerateXML( void )
{
	unsigned char access;

	if ( m_AgentID != GetAgentID() )
	{
		access = max( GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID ), EMS_READ_ACCESS );
	}
	else
	{
		access = EMS_DELETE_ACCESS;
	}

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;
	
	GetXMLGen().AddChildElem( _T("Ticket") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_TicketID);
	GetXMLGen().AddChildAttrib( _T("Display"), m_sView.c_str() );

	GetXMLGen().AddChildElem( _T("TicketNote") );
	if(m_IsVoipNote)
		m_IsPhone = true;
	GetXMLGen().AddChildAttrib( _T("IsVoip"), m_IsPhone );
	// ContactID
	GetISAPIData().GetURLLong( _T("ContactID"), m_ContactID, true );	
	GetXMLGen().AddChildAttrib( _T("ContactID"), m_ContactID );
	GetXMLGen().AddChildAttrib( _T("TimeTextArea"), m_ElapsedTime );
	if(m_StartTimeLen > 0)
	{
		CEMSString sTimeStart;

		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(m_StartTime,lTzBias,tsLocal))
			{
				GetFullDateTimeString( tsLocal, tsLocalLen, sTimeStart );				
			}
			else
			{
				GetFullDateTimeString( m_StartTime, m_StartTimeLen, sTimeStart);
			}
		}
		else
		{
			GetFullDateTimeString( m_StartTime, m_StartTimeLen, sTimeStart);
		}		
		GetXMLGen().AddChildAttrib(_T("TimeStart"), sTimeStart.c_str());
	}
	else
	{
		CEMSString sTimeStart;
		GetXMLGen().AddChildAttrib(_T("TimeStart"), sTimeStart.c_str());
	}
	if(m_StopTimeLen > 0)
	{
		CEMSString sTimeStop;
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(m_StopTime,lTzBias,tsLocal))
			{
				GetFullDateTimeString( tsLocal, tsLocalLen, sTimeStop);				
			}
			else
			{
				GetFullDateTimeString( m_StopTime, m_StopTimeLen, sTimeStop);
			}
		}
		else
		{
			GetFullDateTimeString( m_StopTime, m_StopTimeLen, sTimeStop);
		}
		GetXMLGen().AddChildAttrib(_T("TimeStop"), sTimeStop.c_str());
	}
	else
	{
		CEMSString sTimeStop;
		GetXMLGen().AddChildAttrib(_T("TimeStop"), sTimeStop.c_str());
	}
	GetXMLGen().AddChildAttrib( _T("TicketNoteID"), m_TicketNoteID );
	GetXMLGen().AddChildAttrib( _T("NumAttach"), m_NumAttach );
	GetXMLGen().AddChildAttrib( _T("usecuttoffdate"), m_UseCutoffDate );
	GetXMLGen().AddChildAttrib(_T("beenread"), m_BeenRead );
	GetXMLGen().AddChildAttrib( _T("Access"), access );
	
	AddAgentName( _T("Agent"), m_AgentID );

	CEMSString sDateCreated;
	if(lTzBias != 1)
	{
		if(ConvertToTimeZone(m_DateCreated,lTzBias,tsLocal))
		{
			GetDateTimeString( tsLocal, tsLocalLen, sDateCreated);
		}
		else
		{
			GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDateCreated);
		}
	}
	else
	{
		GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDateCreated);
	}
	GetXMLGen().AddChildAttrib(_T("DateCreated"), sDateCreated.c_str());
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("Body") );
	GetXMLGen().SetChildData( m_Note, TRUE );
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

void CTicketNotes::SetBeenRead()
{
	if(!UseCutoffDate())
	{
		// has this message been read by agent
		long lAgentID = GetISAPIData().m_pSession->m_AgentID;
		long lBeenReadID;
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(),m_TicketNoteID );
		BINDPARAM_LONG( GetQuery(), lAgentID );
		BINDCOL_LONG_NOLEN( GetQuery(), lBeenReadID );
		GetQuery().Execute(_T("SELECT TicketNotesReadID FROM TicketNotesRead WHERE TicketNoteID = ? AND AgentID = ?"));
		if(GetQuery().Fetch() == S_OK)
			m_BeenRead = 1;
	}
	else
	{
		m_BeenRead = 1;
		m_UseCutoffDate = 1;
	}
}

void CTicketNotes::UpdateRead()
{
	if(!UseCutoffDate())
	{
		long lBeenRead = 0;

		if ( GetISAPIData().GetXMLLong(_T("beenread"), *(int*)&lBeenRead, true ) == false )
		{
			lBeenRead = 1;
		}
		
		
		if ( !m_TicketNoteID )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("A TicketNoteID must be specified") );

		long lAgentID = GetISAPIData().m_pSession->m_AgentID;
		long lBeenReadID;
		bool bExist = false;
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(),m_TicketNoteID );
		BINDPARAM_LONG( GetQuery(), lAgentID );
		BINDCOL_LONG_NOLEN( GetQuery(), lBeenReadID );
		GetQuery().Execute(_T("SELECT TicketNotesReadID FROM TicketNotesRead WHERE TicketNoteID = ? AND AgentID = ?"));
		if(GetQuery().Fetch() == S_OK)
			bExist = true;

		if(lBeenRead)
		{
			if(!bExist)
			{
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(),m_TicketNoteID );
				BINDPARAM_LONG( GetQuery(), lAgentID );
				GetQuery().Execute(_T("INSERT INTO TicketNotesRead (TicketNoteID, AgentID) VALUES (?, ?)"));
				
				long nTicketID;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_TicketNoteID);
				BINDCOL_LONG_NOLEN( GetQuery(), nTicketID); 
				GetQuery().Execute(_T("SELECT TicketID FROM TicketNotes WHERE (TicketNoteID = ?)"));
				if(GetQuery().Fetch() == S_OK)
				{
					UpdateTHNoteRead(GetQuery(), lAgentID, nTicketID, 1, m_TicketNoteID);
				}
			}
		}
		else
		{
			if(bExist)
			{
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(),m_TicketNoteID );
				BINDPARAM_LONG( GetQuery(), lAgentID );
				GetQuery().Execute(_T("DELETE TicketNotesRead WHERE TicketNoteID = ? AND AgentID = ?"));

				long nTicketID;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_TicketNoteID);
				BINDCOL_LONG_NOLEN( GetQuery(), nTicketID); 
				GetQuery().Execute(_T("SELECT TicketID FROM TicketNotes WHERE (TicketNoteID = ?)"));
				if(GetQuery().Fetch() == S_OK)
				{
					UpdateTHNoteRead(GetQuery(), lAgentID, nTicketID, 0, m_TicketNoteID);
				}
			}
		}
	}
}

void CTicketNotes::UpdateTHNoteRead(CODBCQuery& query, int nAgentID, int nTicketID, int nRead, int nNoteID)
{
	SYSTEMTIME curTime;
	::GetLocalTime(&curTime);

	TTicketHistory tth;
	tth.m_AgentID = nAgentID;
	tth.m_TicketID = nTicketID;
	tth.m_TicketActionID = EMS_TICKETACTIONID_READ_NOTE;
	tth.m_ID1 = nRead;
	tth.m_ID2 = nNoteID;
	tth.m_DateTime.year = curTime.wYear;
	tth.m_DateTime.month = curTime.wMonth;
	tth.m_DateTime.day = curTime.wDay;
	tth.m_DateTime.hour = curTime.wHour;
	tth.m_DateTime.minute = curTime.wMinute;
	tth.m_DateTime.second = curTime.wSecond;
	tth.m_DateTimeLen = sizeof(TIMESTAMP_STRUCT);

	tth.Insert(query);
}

bool CTicketNotes::UseCutoffDate()
{
	TServerParameters servParams;
	servParams.m_ServerParameterID = 41;

	int nResult = servParams.Query(GetQuery());

	if(nResult != S_OK)
		return false;

	if(!lstrcmp(servParams.m_DataValue,_T("1")))
	{
		servParams.m_ServerParameterID = 42;

		nResult = servParams.Query(GetQuery());

		if((nResult != S_OK) || (lstrlen(servParams.m_DataValue) == 0))
			return false;

		SYSTEMTIME recvTime;
		ZeroMemory(&recvTime, sizeof(SYSTEMTIME));

		recvTime.wYear = m_DateCreated.year;
		recvTime.wDay = m_DateCreated.day;
		recvTime.wMonth = m_DateCreated.month;
		recvTime.wHour = m_DateCreated.hour;
		recvTime.wMinute = m_DateCreated.minute;
		recvTime.wSecond = m_DateCreated.second;

		SYSTEMTIME cutoffTime;
		ZeroMemory(&cutoffTime, sizeof(SYSTEMTIME));

		dca::String sDate(servParams.m_DataValue);

		dca::String::size_type pos = dca::String::npos;

		int nDatePos = 0;
		do
		{
			pos = sDate.find('/');

			if(pos != dca::String::npos)
			{
				dca::String temp = sDate.substr(0, pos);
				sDate = sDate.erase(0, pos + 1);
				switch(nDatePos)
				{
				case 0:
					{
						cutoffTime.wMonth = temp.ToInt();
						nDatePos++;
					}
					break;
				case 1:
					{
						cutoffTime.wDay = temp.ToInt();
						nDatePos++;
					}
					break;
				}
			}
			else
			{
				if(nDatePos == 2)
				{
					cutoffTime.wYear = sDate.ToInt();
				}
			}
		}while(pos != dca::String::npos);

		cutoffTime.wHour = 23;
		cutoffTime.wMinute = 59;
		cutoffTime.wSecond = 59;

		FILETIME ftRecv;
		FILETIME ftCutoff;

		SystemTimeToFileTime(&recvTime, &ftRecv);
		SystemTimeToFileTime(&cutoffTime, &ftCutoff);

		if(CompareFileTime(&ftCutoff, &ftRecv) == 1)
			return true;

		return false;
	}

	return false;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the contacts for a ticket.           
\*--------------------------------------------------------------------------*/
void CTicketNotes::ListTicketContacts( int nTicketID )
{
	GetQuery().Initialize();
	TicketContactList_t ListItem;
	BINDCOL_LONG_NOLEN( GetQuery(), ListItem.nContactID );
	BINDCOL_TCHAR( GetQuery(), ListItem.Name );
	BINDCOL_TCHAR_NOLEN( GetQuery(), ListItem.Email );
	BINDPARAM_LONG( GetQuery(), nTicketID );
	
	GetQuery().Execute( _T("SELECT DISTINCT TicketContacts.ContactID, Contacts.Name, DataValue ") 
		_T("FROM TicketContacts ")
		_T("INNER JOIN Contacts ON TicketContacts.ContactID = Contacts.ContactID ")
		_T("INNER JOIN PersonalData ON Contacts.DefaultEmailAddressID = PersonalData.PersonalDataID ")
		_T("WHERE TicketContacts.TicketID = ? AND Contacts.IsDeleted=0 ")
		_T("ORDER BY 1,2 ") );
	
	list<TicketContactList_t> List;

	while (GetQuery().Fetch() == S_OK)
	{
		List.push_back( ListItem );
	}
	
	GenMultipleContactXML( List );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates the XML for a list of multiple contacts	              
\*--------------------------------------------------------------------------*/
void CTicketNotes::GenMultipleContactXML( list<TicketContactList_t>& List )
{
	list<TicketContactList_t>::iterator iter;
	
	GetXMLGen().AddChildElem(_T("Contacts"));
	GetXMLGen().AddChildAttrib(_T("Count"), List.size() );
	
	for ( iter = List.begin(); iter != List.end(); iter++ )
	{
		GetXMLGen().AddChildElem(_T("Contact"));
		GetXMLGen().AddChildAttrib(_T("ID"), iter->nContactID );
		GetXMLGen().AddChildAttrib(_T("Name"), iter->NameLen ? iter->Name : iter->Email );		
	}
	
	TPersonalData pd;

	for ( iter = List.begin(); iter != List.end(); iter++ )
	{		
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), pd.m_ContactID );
		BINDCOL_TCHAR( GetQuery(), pd.m_Note );
		BINDCOL_TCHAR( GetQuery(), pd.m_DataValue );		
		BINDCOL_LONG( GetQuery(), pd.m_PersonalDataID );
		BINDPARAM_LONG( GetQuery(), iter->nContactID );
		
		GetQuery().Execute( _T("SELECT pd.contactid,pdt.typename,pd.datavalue,pd.personaldataid ") 
							_T("FROM personaldatatypes pdt ")
							_T("INNER JOIN personaldata pd ON pdt.personaldatatypeid=pd.personaldatatypeid ")
							_T("WHERE pd.personaldatatypeid in (2,3,4) AND pd.contactid=? ") );		

		while (GetQuery().Fetch() == S_OK)
		{
			GetXMLGen().AddChildElem(_T("Number"));
			GetXMLGen().AddChildAttrib(_T("CID"), pd.m_ContactID );
			GetXMLGen().AddChildAttrib(_T("NumberType"), pd.m_Note );
			GetXMLGen().AddChildAttrib(_T("Number"), pd.m_DataValue );
			GetXMLGen().AddChildAttrib(_T("PDID"), pd.m_PersonalDataID );
		}		
	}
}
