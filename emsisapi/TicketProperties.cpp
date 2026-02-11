
/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketProperties.cpp,v 1.2.2.2.2.1 2006/07/18 12:55:03 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketProperties.h"
#include "StringFns.h"
#include "TicketHistoryFns.h"
#include "TicketNotes.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CTicketProperties::CTicketProperties(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData ), m_Ticket( ISAPIData )
{
	m_bMultipleTickets = false;
	m_bEscalate = false;
	m_bChangeSubject = false;
	m_bChangeIBSubjects = false;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point
\*--------------------------------------------------------------------------*/
int CTicketProperties::Run(CURLAction& action)
{
	CEMSString sTicketID;
		
	// get the TicketID
	GetISAPIData().GetURLString( _T("TicketID"), sTicketID );

	// check for an alertId
	GetISAPIData().GetURLLong( _T("alertId"), nAlertID, true );

	if ( nAlertID > 0 )
	{
		MarkAlertAsRead();
	}

	if ( sTicketID.find( _T(",") ) != CEMSString::npos )
		m_bMultipleTickets = true;
	
	// query the ticket
	if ( !m_bMultipleTickets )
	{
		sTicketID.CDLGetNextInt( m_Ticket.m_TicketID );
		m_Ticket.Query();
	}

	// update
	if ( GetISAPIData().GetXMLPost() )
	{
		UpdateTicket();
	}
	else
	{
		tstring sMethod;

		if ( GetISAPIData().GetURLString( _T("Method"), sMethod, true ) )
		{
			if ( sMethod.compare( _T("owner") ) == 0 )
			{
				action.m_sPageTitle.assign( _T("Reassign Ticket") );
			}
			else if ( sMethod.compare( _T("close") ) == 0  )
			{
				action.m_sPageTitle.assign( _T("Close Ticket") );
			}
		}
			
		// generateXML
		GenerateXML();
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	GenerateXML	
\*--------------------------------------------------------------------------*/
void CTicketProperties::GenerateXML()
{
	if ( !m_bMultipleTickets )
		m_Ticket.GenerateXML();

	ListTicketPriorities();
	//ListTicketCategoryNames( GetXMLGen() );

	TTicketCategories pdt;
	pdt.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("TicketCategoryNames") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketCategory") );
		GetXMLGen().AddChildAttrib( _T("ID"), pdt.m_TicketCategoryID );
		GetXMLGen().AddChildAttrib( _T("Description"), pdt.m_Description );
	}
	GetXMLGen().OutOfElem();

	ListTicketBoxNames( EMS_ENUM_ACCESS, GetXMLGen(), m_Ticket.m_TicketBoxID );
	ListEnabledAgentNames( EMS_ENUM_ACCESS, m_Ticket.m_OwnerID );
	ListMyFolders();
	
	GetXMLGen().AddChildElem( _T("subject") );
	GetXMLGen().AddChildAttrib( _T("text"), m_Ticket.m_Subject);
	GetXMLGen().AddChildElem( _T("TicketStates"));
	GetXMLGen().IntoElem();
	TTicketStates TicketState;
	TicketState.PrepareList( GetQuery() );
	while( GetQuery().Fetch() == S_OK )
	{
		if ( TicketState.m_TicketStateID != EMS_TICKETSTATEID_ESCALATED || 
			 m_Ticket.m_TicketStateID == EMS_TICKETSTATEID_ESCALATED )
		{
			GetXMLGen().AddChildElem( _T("State") );
			GetXMLGen().AddChildAttrib( _T("ID"), TicketState.m_TicketStateID );
			GetXMLGen().AddChildAttrib( _T("Description"), TicketState.m_Description );
		}
	}
	GetXMLGen().OutOfElem();

	// XML for the document
	GetXMLDoc().AddElem( _T("DATA") );
	
	//Get list of TicketFields configured for all TicketBoxes
	TTicketFieldsTicketBox tftb;
	TCHAR Description[50];
	long DescriptionLen;
	int nTicketFieldTypeID;
	GetXMLDoc().AddChildElem( _T("TicketFieldTicketBoxes") );
	GetXMLDoc().IntoElem();
    GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tftb.m_TicketBoxID );
	BINDCOL_LONG( GetQuery(), tftb.m_TicketFieldID );
	BINDCOL_TCHAR( GetQuery(), Description );
	BINDCOL_LONG_NOLEN( GetQuery(), nTicketFieldTypeID );
	BINDCOL_BIT( GetQuery(), tftb.m_IsRequired );
	BINDCOL_BIT( GetQuery(), tftb.m_IsViewed );
	BINDCOL_BIT( GetQuery(), tftb.m_SetDefault );
	BINDCOL_TCHAR( GetQuery(), tftb.m_DefaultValue );
	BINDCOL_LONG( GetQuery(), tftb.m_TicketFieldsTicketBoxID );
	GetQuery().Execute( _T("SELECT tftb.TicketBoxID,tftb.TicketFieldID,tf.Description,tf.TicketFieldTypeID,tftb.IsRequired,tftb.IsViewed,tftb.SetDefault,tftb.DefaultValue,tftb.TicketFieldsTicketBoxID ")
	               _T("FROM TicketFieldsTicketBox tftb INNER JOIN TicketFields tf ON tftb.TicketFieldID=tf.TicketFieldID ")
	               _T("ORDER BY TicketBoxID") );	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLDoc().AddChildElem(_T("TicketFieldTicketBox"));
		GetXMLDoc().AddChildAttrib( _T("TBID"), tftb.m_TicketBoxID );
		GetXMLDoc().AddChildAttrib( _T("TFID"), tftb.m_TicketFieldID );
		GetXMLDoc().AddChildAttrib( _T("TFNAME"), Description );
		GetXMLDoc().AddChildAttrib( _T("TFTYPE"), nTicketFieldTypeID );
		GetXMLDoc().AddChildAttrib( _T("IsRequired"), tftb.m_IsRequired );
		GetXMLDoc().AddChildAttrib( _T("IsViewed"), tftb.m_IsViewed );
		GetXMLDoc().AddChildAttrib( _T("SetDefault"), tftb.m_SetDefault );
		GetXMLDoc().AddChildAttrib( _T("DefaultValue"), tftb.m_DefaultValue );
	}
	GetXMLDoc().OutOfElem();

	//Get Options for type of TicketField select
	TTicketFieldOptions tfo;
	int nTBID;
	GetXMLDoc().AddChildElem( _T("TicketFieldOptions") );
	GetXMLDoc().IntoElem();
    GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tfo.m_TicketFieldOptionID );
	BINDCOL_LONG( GetQuery(), tfo.m_TicketFieldID );
	BINDCOL_TCHAR( GetQuery(), tfo.m_OptionValue );
	BINDCOL_LONG_NOLEN( GetQuery(), nTBID );
	GetQuery().Execute( _T("SELECT tfo.TicketFieldOptionID,tfo.TicketFieldID,tfo.OptionValue,tftb.TicketBoxID ")
	               _T("FROM TicketFieldOptions tfo ")
	               _T("INNER JOIN TicketFieldsTicketBox tftb ON tfo.TicketFieldID=tftb.TicketFieldID ")
	               _T("ORDER BY tftb.TicketBoxID,tfo.TicketFieldID,tfo.OptionOrder") );	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLDoc().AddChildElem(_T("Option"));
		GetXMLDoc().AddChildAttrib( _T("TFOID"), tfo.m_TicketFieldOptionID );
		GetXMLDoc().AddChildAttrib( _T("TFID"), tfo.m_TicketFieldID );
		GetXMLDoc().AddChildAttrib( _T("OPTIONVALUE"), tfo.m_OptionValue );
		GetXMLDoc().AddChildAttrib( _T("TBID"), nTBID );
	}
	GetXMLDoc().OutOfElem();

	if ( !m_bMultipleTickets )
	{
		TTicketFieldsTicket tft;
		GetXMLDoc().AddChildElem( _T("TicketFields") );
		GetXMLDoc().IntoElem();
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), tft.m_TicketFieldID );
		BINDCOL_TCHAR( GetQuery(), tft.m_DataValue );
		BINDCOL_TCHAR( GetQuery(), Description );	
		BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
		GetQuery().Execute( _T("SELECT tft.TicketFieldID, tft.DataValue, tf.Description ")
							_T("FROM TicketFieldsTicket tft INNER JOIN TicketFields tf ON tft.TicketFieldID=tf.TicketFieldID ")
	               			_T("WHERE tft.TicketID=?") );
		while ( GetQuery().Fetch() == S_OK )
		{
			GetXMLDoc().AddChildElem( _T("Field") );
			GetXMLDoc().AddChildAttrib( _T("TFID") , tft.m_TicketFieldID );
			GetXMLDoc().AddChildAttrib( _T("DataValue") , tft.m_DataValue );
			GetXMLDoc().AddChildAttrib( _T("TFNAME"), Description );
		}
		GetXMLDoc().OutOfElem();
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Update the ticket	              
\*--------------------------------------------------------------------------*/
void CTicketProperties::UpdateTicket()
{
	tstring sMethod;
	GetISAPIData().GetURLString( _T("Method"), sMethod );

	// decode the form
	DecodeForm();

	if ( nAlertID > -1 )
	{
        ClearAllAlerts();
	}

	CEMSString sTicketID;
	tstring sNote;
	bool m_bUpdatedTicket = false;

	GetISAPIData().GetURLString( _T("TicketID"), sTicketID );
	GetISAPIData().GetXMLString( _T("NOTE"), sNote, true );

	while ( sTicketID.CDLGetNextInt( m_Ticket.m_TicketID ) )
	{
		if ( sMethod.compare( _T("props") ) == 0 )
		{
			try
			{
				GetQuery().Initialize();
				GetQuery().Execute( _T("BEGIN TRANSACTION") );

				if ( m_Ticket.m_TicketBoxID != -1 )
				{
					m_Ticket.ChangeTicketBox( m_Ticket.m_TicketBoxID );
					m_bUpdatedTicket = true;
				}

				if ( m_Ticket.m_TicketCategoryID != -1 )
				{
					m_Ticket.SetTicketCategory( m_Ticket.m_TicketCategoryID );
					m_bUpdatedTicket = true;
				}

				if ( m_Ticket.m_OwnerID != -1 )
				{
					m_Ticket.Reassign( m_Ticket.m_OwnerID );
					m_bUpdatedTicket = true;
				}

				if ( m_Ticket.m_TicketStateID != -1 && m_Ticket.m_TicketStateID != 4 )
				{
					m_Ticket.SetState( m_Ticket.m_TicketStateID );
					m_bUpdatedTicket = true;
				}

				if ( m_Ticket.m_PriorityID != -1 )
				{
					m_Ticket.ChangePriority( m_Ticket.m_PriorityID );
					m_bUpdatedTicket = true;
				}

				if ( m_Ticket.m_UseTickler != -1 && m_Ticket.m_ReOpenTime.year != 0)
				{
					m_Ticket.SetTickler();
					m_bUpdatedTicket = true;
				}

				if (m_Ticket.m_FolderID != -1)
				{
					m_Ticket.SetFolderID();
					m_bUpdatedTicket = true;
				}

				UpdateSubject(m_bUpdatedTicket);

				GetQuery().Initialize();
				GetQuery().Execute( _T("COMMIT TRANSACTION") );
			}
			catch( ... )
			{
				GetQuery().Initialize();
				GetQuery().Execute( _T("ROLLBACK TRANSACTION") );
				throw;
			}

			if (!m_bUpdatedTicket)
				THROW_VALIDATION_EXCEPTION( _T(""), _T("Select a ticket property to update") ); 
		}
		else if ( sMethod.compare( _T("close") ) == 0 || (sMethod.compare( _T("state") ) == 0 && m_Ticket.m_TicketStateID == 1) )
		{	
			if ( m_Ticket.m_UseTickler != -1 && m_Ticket.m_ReOpenTime.year != 0)
			{
				m_Ticket.SetTickler();
				m_bUpdatedTicket = true;
			}
			
			m_Ticket.Close();
		}
		else if ( sMethod.compare( _T("owner") ) == 0 )
		{
			m_Ticket.Reassign( m_Ticket.m_OwnerID );
			
			if(m_Ticket.m_OwnerID != this->GetAgentID())
			{
				m_Ticket.m_FolderID = 0;
				m_Ticket.SetFolderID();
			}
		}
		else if ( sMethod.compare( _T("state") ) == 0 )
		{
			m_Ticket.SetState( m_Ticket.m_TicketStateID );
		}
		else if ( sMethod.compare( _T("category") ) == 0 )
		{
			m_Ticket.SetTicketCategory( m_Ticket.m_TicketCategoryID );
		}
		else if ( sMethod.compare( _T("priority") ) == 0 )
		{
			m_Ticket.ChangePriority( m_Ticket.m_PriorityID );
		}
		else if ( sMethod.compare( _T("ticketbox") ) == 0 )
		{
			m_Ticket.m_FolderID = 0;
			m_Ticket.ChangeTicketBox( m_Ticket.m_TicketBoxID );
			m_Ticket.SetFolderID();
		}

		if ( sNote.length() )
		{
			TTicketNotes Note;
			PutStringProperty( sNote, &Note.m_Note, &Note.m_NoteAllocated );
			Note.m_TicketID = m_Ticket.m_TicketID;
			Note.m_AgentID = GetAgentID();
			GetTimeStamp( Note.m_DateCreated );
			Note.m_DateCreatedLen = sizeof(TIMESTAMP_STRUCT);
			Note.m_StopTimeLen = SQL_NULL_DATA;
			Note.m_StopTimeLen = SQL_NULL_DATA;
			Note.Insert( GetQuery() );
		
			// Log it in system-generated ticket history
			THAddNote( GetQuery(), m_Ticket.m_TicketID, GetAgentID(), Note.m_TicketNoteID );
			// Mark the note we just added as read			
			CTicketNotes::UpdateTHNoteRead(GetQuery(), Note.m_AgentID, Note.m_TicketID, 1, Note.m_TicketNoteID);			
		}

		// Add/Update any Custom Ticket Fields
		for( tftIter = m_tft.begin(); tftIter != m_tft.end(); tftIter++ )
		{
			m_Ticket.AddTicketFieldTicket(m_Ticket.m_TicketID,tftIter->m_TicketFieldID,tftIter->m_TicketFieldTypeID, tftIter->m_DataValue);
		}
		
		if ( m_bEscalate )
		{
			m_Ticket.Escalate();
		}
	}

	// TODO - Only add this when we really need to!
	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Decode Form	              
\*--------------------------------------------------------------------------*/
void CTicketProperties::DecodeForm()
{
	tstring sAction;
	CEMSString sParam;

	if ( GetISAPIData().GetXMLString( _T("Action"), sAction, true ) )
	{
		if ( sAction.compare( _T("escalate") ) == 0 )
			m_bEscalate = true;
	}

	unsigned char bAllSubjects = 0;

	GetISAPIData().GetXMLLong( _T("TICKETBOXID"), m_Ticket.m_TicketBoxID, true );
	GetISAPIData().GetXMLLong( _T("PRIORITYID"), m_Ticket.m_PriorityID, true );
	GetISAPIData().GetXMLLong( _T("TICKETCATEGORYID"), m_Ticket.m_TicketCategoryID, true );
	GetISAPIData().GetXMLLong( _T("chkUseTickler"), m_Ticket.m_UseTickler, true);
	GetISAPIData().GetXMLTCHAR( _T("txtSubject"), m_NewSubject, 255, true);
	GetISAPIData().GetXMLLong( _T("chkSubject"), bAllSubjects, true);

	CkString str;
	str.setString(m_NewSubject);
	//str.entityDecode();
	_tcscpy( m_NewSubject, str.getString() );
	
	if(_tcscmp(m_NewSubject, m_Ticket.m_Subject) != 0) 
		m_bChangeSubject = true;

	if(bAllSubjects)
		m_bChangeIBSubjects = true;

	if(m_Ticket.m_UseTickler)
	{
		CEMSString sDate;
		if(GetISAPIData().GetXMLString( _T("REOPEN_DATE"), sDate, true))
		{
			sDate.CDLGetTimeStamp(m_Ticket.m_ReOpenTime);
			m_Ticket.m_ReOpenTime.second=0;
			m_Ticket.m_ReOpenTime.fraction=0;

			int nHr=0;
			int nMin=0;
			int nAmPm=0;

			GetISAPIData().GetXMLLong( _T("OpenHr"), nHr, true );
			GetISAPIData().GetXMLLong( _T("OpenMin"), nMin, true );
			GetISAPIData().GetXMLLong( _T("OpenAmPm"), nAmPm, true );
						
			if(nHr == 12 && nAmPm == 1)
			{
				m_Ticket.m_ReOpenTime.hour = 0;
			}
			else if(nAmPm == 2 && nHr == 12)
			{
				m_Ticket.m_ReOpenTime.hour = 12;
			}
			else if (nAmPm == 2)
			{
				m_Ticket.m_ReOpenTime.hour = nHr + 12;
			}
			else
			{
				m_Ticket.m_ReOpenTime.hour = nHr;
			}

			m_Ticket.m_ReOpenTime.minute = nMin;

			long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
			TIMESTAMP_STRUCT tsLocal;
			long tsLocalLen=0;

			if(lTzBias != 1)
			{
				if(ConvertFromTimeZone(m_Ticket.m_ReOpenTime,lTzBias,tsLocal))
				{
					m_Ticket.m_ReOpenTime.year = tsLocal.year;
					m_Ticket.m_ReOpenTime.month = tsLocal.month;
					m_Ticket.m_ReOpenTime.day = tsLocal.day;
					m_Ticket.m_ReOpenTime.hour = tsLocal.hour;
					m_Ticket.m_ReOpenTime.minute = tsLocal.minute;
					m_Ticket.m_ReOpenTime.second = tsLocal.second;
				}			
			}
		}
		else
		{
			ZeroMemory(&m_Ticket.m_ReOpenTime, sizeof(TIMESTAMP_STRUCT));
			m_Ticket.m_ReOpenTimeLen = SQL_NULL_DATA;
			m_Ticket.m_UseTickler = 0;
			m_Ticket.UnSetTickler();
		}

		m_Ticket.m_TicketStateID = EMS_TICKETSTATEID_CLOSED;
		GetISAPIData().GetXMLLong( _T("OWNERID"), m_Ticket.m_OwnerID, true );
		
		return;
	}
	
	if ( !m_bEscalate )
	{
		GetISAPIData().GetXMLLong( _T("STATEID"), m_Ticket.m_TicketStateID, true );
		GetISAPIData().GetXMLLong( _T("OWNERID"), m_Ticket.m_OwnerID, true );
		GetISAPIData().GetXMLLong( _T("FOLDERID"), m_Ticket.m_FolderID, true);
		GetISAPIData().GetXMLLong( _T("TICKETBOXID"), m_Ticket.m_TicketBoxID, true);
		GetISAPIData().GetXMLLong( _T("PRIORITYID"), m_Ticket.m_PriorityID, true);
	}
	else
	{
		m_Ticket.m_TicketStateID = EMS_TICKETSTATEID_ESCALATED;
		m_Ticket.m_FolderID = 0;	
	}

	if ( m_Ticket.m_TicketBoxID != -1 )
	{
		//Get list of TicketFields configured for the selected TicketBox
		m_tft.clear();
		TTicketFields tf;
		TTicketFieldsTicket tft;
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), tf.m_TicketFieldID );
		BINDCOL_LONG( GetQuery(), tf.m_TicketFieldTypeID );
		GetQuery().Execute( _T("SELECT TicketFieldID,TicketFieldTypeID ")
					_T("FROM TicketFields ") );					
		while( GetQuery().Fetch() == S_OK )
		{
			sParam.Format( _T("input%d"), tf.m_TicketFieldID );
			if ( GetISAPIData().GetXMLTCHAR( (TCHAR*) sParam.c_str(), tft.m_DataValue, 255, true) )
			{
				tft.m_TicketFieldID = tf.m_TicketFieldID;
				tft.m_TicketFieldTypeID = tf.m_TicketFieldTypeID;
				m_tft.push_back(tft);
			}			
		}
	}
}

void CTicketProperties::ListMyFolders()
{
	dca::String55 sFolder;
	long sFolderLen = 55;
	long lID = 0;
	ZeroMemory(sFolder, 56);
	long lAgentID = GetISAPIData().m_pSession->m_AgentID;

	GetQuery().Initialize();	
	BINDPARAM_LONG( GetQuery(), lAgentID);
	BINDCOL_LONG_NOLEN( GetQuery(), lID);
	BINDCOL_TCHAR( GetQuery(), sFolder);
	
	GetQuery().Execute( _T("SELECT FolderID, Name FROM Folders WHERE AgentID = ?"));
	
	GetXMLGen().AddChildElem( _T("myfolders") );

	while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("folder") );
		GetXMLGen().AddChildAttrib( _T("id"), lID );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("name"), sFolder );
		GetXMLGen().OutOfElem();
		GetXMLGen().OutOfElem();
	}
}

void CTicketProperties::UpdateSubject(bool& m_bUpdatedTicket)
{
	if(!m_bMultipleTickets)
	{
		if(m_bChangeSubject)
		{
			GetQuery().Initialize();
			BINDPARAM_TCHAR( GetQuery(), m_NewSubject );
			BINDPARAM_LONG( GetQuery(), m_Ticket.m_TicketID );
			GetQuery().Execute( _T("UPDATE Tickets SET Subject = ? WHERE TicketID = ?"));
			m_bUpdatedTicket = true;
		}

		if(m_bChangeIBSubjects)
		{
			{
				GetQuery().Initialize();
				long id1;
				std::list<long> ibIDs;
				BINDPARAM_LONG(GetQuery(), m_Ticket.m_TicketID);
				BINDCOL_LONG_NOLEN( GetQuery(), id1);
				GetQuery().Execute( _T("SELECT InboundMessageID FROM InboundMessages WHERE TicketID = ?"));

				while ( GetQuery().Fetch() == S_OK )
				{
					ibIDs.push_back(id1);
				}

				GetQuery().Initialize();

				std::list<long>::iterator iter;
				for(iter = ibIDs.begin();
					iter != ibIDs.end();
					iter++)
				{
					long id2 = *iter;
					BINDPARAM_TCHAR( GetQuery(), m_NewSubject );
					BINDPARAM_LONG(GetQuery(), id2);
					GetQuery().Execute( _T("UPDATE InboundMessages SET Subject = ? WHERE InboundMessageID = ?"));
				}
			}

			{
				GetQuery().Initialize();
				long id1;
				std::list<long> ibIDs;
				BINDPARAM_LONG(GetQuery(), m_Ticket.m_TicketID);
				BINDCOL_LONG_NOLEN( GetQuery(), id1);
				GetQuery().Execute( _T("SELECT OutboundMessageID FROM OutboundMessages WHERE TicketID = ?"));

				while ( GetQuery().Fetch() == S_OK )
				{
					ibIDs.push_back(id1);
				}

				GetQuery().Initialize();

				std::list<long>::iterator iter;
				for(iter = ibIDs.begin();
					iter != ibIDs.end();
					iter++)
				{
					long id2 = *iter;
					BINDPARAM_TCHAR( GetQuery(), m_NewSubject );
					BINDPARAM_LONG(GetQuery(), id2);
					GetQuery().Execute( _T("UPDATE OutboundMessages SET Subject = ? WHERE OutboundMessageID = ?"));
				}

				GetQuery().Initialize();
			}

			m_bUpdatedTicket = true;
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Marks an AlertMsg as read	              
\*--------------------------------------------------------------------------*/
void CTicketProperties::MarkAlertAsRead()
{
	GetQuery().Initialize();

	BINDPARAM_LONG( GetQuery(), nAlertID );

	GetQuery().Execute( _T("UPDATE AlertMsgs SET Viewed=1 WHERE AlertMsgID=?") );

	/*if ( GetQuery().GetRowCount() != 1 )
	{
		CEMSString sError;
		sError.Format( _T("The AlertID (%d) is invalid"), nAlertID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}*/
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Clear all Alerts	              
\*--------------------------------------------------------------------------*/
void CTicketProperties::ClearAllAlerts()
{
	GetQuery().Initialize();

	BINDPARAM_LONG(GetQuery(), m_Ticket.m_TicketID);

	GetQuery().Execute( _T("DELETE FROM AlertMsgs WHERE TicketID=?") );
}
