/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/NewTicket.cpp,v 1.2.2.2 2006/02/23 20:37:12 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Creates a new ticket and associates a note with
||				the created ticket.  
||              
\\*************************************************************************/

#include "stdafx.h"
#include "NewTicket.h"
#include "StringFns.h"
#include "ContactFns.h"
#include "TicketHistoryFns.h"
#include "ChangeTicket.h"
#include "TicketNotes.h"

/*---------------------------------------------------------------------------\                     
||  Construction/Destruction	              
\*--------------------------------------------------------------------------*/
CNewTicket::CNewTicket(CISAPIData& ISAPIData) : CTicket(ISAPIData)
{
	m_CloseTicket = 0;
	m_ContactID = 0;
	m_IsPhone = false;
    m_IsCopy = false;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CNewTicket::Run(CURLAction& action)
{
	CEMSString sType;
	int nCopy=0;
	
	if( GetISAPIData().m_sPage.compare( _T("selectcontact") ) == 0)
	{
		GetISAPIData().GetURLString( _T("Name"), m_sName );
		FindContactsByName(true);
	}
	else
	{
		if ( m_ISAPIData.GetXMLPost() )
		{
			if ( GetISAPIData().m_URL.find( _T("MESSAGEID") ) != GetISAPIData().m_URL.end() )
			{
				InsertMoveMsg();
			}
			else
			{
				Insert();
			}
		}
		else
		{
			// is this a new ticket with phone note?
			if (( GetISAPIData().GetXMLString( _T("type"), sType, true )) || 
				( GetISAPIData().GetURLString( _T("type"), sType, true )))
			{
				if ( sType.compare( _T("phone")) == 0 )
				{
					m_IsPhone = true;
				}
			}

			// are we copying a message to a new ticket?
			if (( GetISAPIData().GetXMLLong( _T("copy"), nCopy, true )) || 
				( GetISAPIData().GetURLLong( _T("copy"), nCopy, true )))
			{
				if ( nCopy == 1 )
				{
					m_IsCopy = true;
				}
			}

			
			// add default values for GUI
			AddDefaultValues();
			
			// add additional XML
			ListEnabledAgentNames( EMS_ENUM_ACCESS );
			
			// add -- NONE -- to the list of agent names
			GetXMLGen().SavePos();
			GetXMLGen().ResetChildPos();
			GetXMLGen().FindChildElem(_T("AgentNames"));
			GetXMLGen().IntoElem();
			GetXMLGen().InsertChildElem( _T("Agent") );
			GetXMLGen().AddChildAttrib( _T("ID"), 0 );
			GetXMLGen().AddChildAttrib( _T("Name"), CEMSString(EMS_STRING_LIST_NONE).c_str());
			GetXMLGen().RestorePos();
			
			ListTicketBoxNames( EMS_ENUM_ACCESS, GetXMLGen() );
			ListTicketPriorities();

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

			// reserve a new ticket ID
			ReserveID();

			GetXMLGen().AddChildElem( _T("Ticket") );
			GetXMLGen().AddChildAttrib( _T("ID"), m_TicketID );
			GetXMLGen().AddChildAttrib( _T("IsPhone"), m_IsPhone );
			GetXMLGen().AddChildAttrib( _T("IsCopy"), m_IsCopy );
		}
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Decode form paramaters	              
\*--------------------------------------------------------------------------*/
void CNewTicket::DecodeForm( bool bMoveMsg /*=false*/ )
{
	CEMSString sParam;

	GetISAPIData().GetXMLLong( _T("TicketID"), m_TicketID );

	GetISAPIData().GetXMLString( _T("NAME"), m_sName, true );
	GetISAPIData().GetXMLString( _T("EMAIL"), m_sEmail, true );
	GetISAPIData().GetXMLString( _T("NOTE"), m_Note, true );
	
	GetISAPIData().GetXMLTCHAR( _T("SUBJECT"), m_Subject, 255);

	GetISAPIData().GetXMLLong( _T("TICKETBOX"), m_TicketBoxID );
	GetISAPIData().GetXMLLong( _T("TICKETCATEGORY"), m_TicketCategoryID );
	GetISAPIData().GetXMLLong( _T("OWNER"), m_OwnerID );
	GetISAPIData().GetXMLLong( _T("PRIORITY"), m_PriorityID );
	GetISAPIData().GetXMLLong( _T("CLOSETICKET"), m_CloseTicket );
	GetISAPIData().GetXMLLong( _T("chkUseTickler"), m_UseTickler, true);
	
	GetISAPIData().GetXMLLong( _T("CONTACTID"), m_ContactID, true );
	
	// validation
	if ( _tcslen( m_Subject ) < 1 )
		THROW_VALIDATION_EXCEPTION( _T("subject"), CEMSString(EMS_STRING_NEW_TICKET_NO_SUBJECT) );
	
	if ( !bMoveMsg )
	{
		if ( m_sName.length() < 1 && m_sEmail.length() < 1 )
			THROW_VALIDATION_EXCEPTION( _T("name"), CEMSString(EMS_STRING_NEW_TICKET_NO_CONTACT) );
		
		if ( m_sName.length() > CONTACTS_NAME_LENGTH - 1 )
			THROW_VALIDATION_EXCEPTION( _T("name"), CEMSString(EMS_STRING_NEW_TICKET_NAME_LENGTH) );

		if ( m_sEmail.length() > CONTACTS_NAME_LENGTH - 1 )
			THROW_VALIDATION_EXCEPTION( _T("email"), CEMSString(EMS_STRING_NEW_TICKET_EMAIL_LENGTH) );
	
		if ( m_sEmail.length() && !m_sEmail.ValidateEmailAddr() )
			THROW_VALIDATION_EXCEPTION( _T("email"), CEMSString(EMS_STRING_NEW_TICKET_INVALID_EMAIL) );
	}

	// Is this a VOIP note?
	int nIsPhone = 0;
	GetISAPIData().GetXMLLong( _T("IsPhoneNote"), nIsPhone );
	if ( nIsPhone == 1 )
		m_IsPhone = true;
	
	// Is this a copy?
	int nIsCopy = 0;
	GetISAPIData().GetXMLLong( _T("IsCopy"), nIsCopy );
	if ( nIsCopy == 1 )
		m_IsCopy = true;

	if(m_UseTickler)
	{
		CEMSString sDate;
		GetISAPIData().GetXMLString( _T("REOPEN_DATE"), sDate);
		sDate.CDLGetTimeStamp(m_ReOpenTime);
		m_ReOpenTime.second=0;
		m_ReOpenTime.fraction=0;

		int nHr=0;
		int nMin=0;
		int nAmPm=0;

		GetISAPIData().GetXMLLong( _T("OpenHr"), nHr, true );
		GetISAPIData().GetXMLLong( _T("OpenMin"), nMin, true );
		GetISAPIData().GetXMLLong( _T("OpenAmPm"), nAmPm, true );
					
		if(nHr == 12 && nAmPm == 1)
		{
			m_ReOpenTime.hour = 0;
		}
		else if(nAmPm == 2 && nHr == 12)
		{
			m_ReOpenTime.hour = 12;
		}
		else if (nAmPm == 2)
		{
			m_ReOpenTime.hour = nHr + 12;
		}
		else
		{
			m_ReOpenTime.hour = nHr;
		}

		m_ReOpenTime.minute = nMin;
		
		long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
		TIMESTAMP_STRUCT tsLocal;
		long tsLocalLen=0;

		if(lTzBias != 1)
		{
			if(ConvertFromTimeZone(m_ReOpenTime,lTzBias,tsLocal))
			{
				m_ReOpenTime.year = tsLocal.year;
				m_ReOpenTime.month = tsLocal.month;
				m_ReOpenTime.day = tsLocal.day;
				m_ReOpenTime.hour = tsLocal.hour;
				m_ReOpenTime.minute = tsLocal.minute;
				m_ReOpenTime.second = tsLocal.second;
			}			
		}

		m_CloseTicket = EMS_TICKETSTATEID_CLOSED;
	}

	//Get list of TicketFields configured for this TicketBox
	m_tft.clear();
	TTicketFieldsTicketBox tftb;
	TTicketFieldsTicket tft;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tftb.m_TicketFieldID );
	BINDCOL_LONG( GetQuery(), tft.m_TicketFieldTypeID );
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("SELECT tftb.TicketFieldID,tf.TicketFieldTypeID ")
	               _T("FROM TicketFieldsTicketBox tftb INNER JOIN TicketFields tf ON tftb.TicketFieldID=tf.TicketFieldID ")
	               _T("WHERE tftb.TicketBoxID=? ORDER BY TicketBoxID") );	
	while( GetQuery().Fetch() == S_OK )
	{
		sParam.Format( _T("input%d"), tftb.m_TicketFieldID );
		GetISAPIData().GetXMLTCHAR( (TCHAR*) sParam.c_str(), tft.m_DataValue, 255, true);
		tft.m_TicketID = m_TicketID;
		tft.m_TicketFieldID = tftb.m_TicketFieldID;		
		m_tft.push_back(tft);
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add the default property values to the XML 
||				for the new ticket	              
\*--------------------------------------------------------------------------*/
void CNewTicket::AddDefaultValues( void )
{
	MsgID_t Msg;
		
	if ( GetISAPIData().GetURLLong( _T("MessageID"), Msg.MsgID, true ) )
	{
		tstring sMsgType;
		GetISAPIData().GetURLString( _T("MessageType"), sMsgType, true );
		Msg.IsInbound = sMsgType.compare( _T("inbound") ) == 0;

		GetQuery().Initialize();

		BINDCOL_LONG_NOLEN( GetQuery(), m_TicketID );
		BINDPARAM_LONG( GetQuery(), Msg.MsgID );
		
		Msg.IsInbound ? GetQuery().Execute( _T("SELECT TicketID FROM InboundMessages WHERE InboundMessageID=?") ) :
			            GetQuery().Execute( _T("SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID=?") );
		
		if ( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("The message no longer exists") );

		Query();

		GetXMLGen().AddChildElem( _T("Default") );
		GetXMLGen().AddChildAttrib( _T("TicketBox"), m_TicketBoxID );
		GetXMLGen().AddChildAttrib( _T("TicketCategory"), m_TicketCategoryID );
		GetXMLGen().AddChildAttrib( _T("Owner"), m_OwnerID );
		GetXMLGen().AddChildAttrib( _T("Priority"), m_PriorityID );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("Subject"), m_Subject );
		GetXMLGen().OutOfElem();
	}
	else
	{
		TTicketBoxViews TicketBoxView;
		GetISAPIData().GetURLLong( _T("TicketBoxView"), TicketBoxView.m_TicketBoxViewID, true );
		
		if ( TicketBoxView.m_TicketBoxViewID > 0 )
		{
			// if an error occurs, let it through
			TicketBoxView.Query( GetQuery() );
		}
		GetXMLGen().AddChildElem( _T("Default") );
		GetXMLGen().AddChildAttrib( _T("TicketBox"), TicketBoxView.m_TicketBoxID );
		GetXMLGen().AddChildAttrib( _T("Owner"), TicketBoxView.m_AgentBoxID );
		GetXMLGen().AddChildAttrib( _T("Priority"), EMS_PRIORITY_NORMAL );
	}

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
		if(nTicketFieldTypeID == 1)
		{
			tstring sVal;
			sVal.assign( tftb.m_DefaultValue );

			//$$DateTimeNow$$
			tstring::size_type pos = sVal.find( _T("$$DateTimeNow$$") );
			if( pos != tstring::npos )
			{
				TIMESTAMP_STRUCT Now;
				GetTimeStamp( Now );
				long NowLen = sizeof(Now);
				CEMSString sDate;
				GetDateTimeString( Now, NowLen, sDate );
				sVal.replace( pos, 15, sDate.c_str() );
				_sntprintf( tftb.m_DefaultValue, TICKETFIELDS_VALUE_LENGTH - 1, sVal.c_str() );
			}
		}
		
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
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Creates a new ticket	              
\*--------------------------------------------------------------------------*/
void CNewTicket::Insert( void )
{
	DecodeForm();

	// check security
	RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, m_TicketBoxID, EMS_EDIT_ACCESS );
	RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_CATEGORY, m_TicketCategoryID, EMS_EDIT_ACCESS );
	
	if ( m_OwnerID )
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_OwnerID, EMS_ENUM_ACCESS );

	// process the contact
	if ( !m_ContactID && !HandleContact() )
		return;
	
	// add the contact to the ticket
	AddContactToTicket( GetQuery(), m_ContactID, m_TicketID, true );

	SaveTicket();

	//Log the creation of the Ticket in TicketHistory
	THCreateTicket( GetQuery(), m_TicketID, GetSession().m_AgentID, 0, m_TicketStateID, m_TicketBoxID, m_OwnerID, m_PriorityID, m_TicketCategoryID );

	//Log it to Agent Activity
	if( _ttoi( sAal.c_str() ) == 1 && _ttoi( sTc.c_str() ) == 1 )
	{
		LogAgentAction(GetAgentID(),4,m_TicketID,m_TicketBoxID,0,_T(""),_T(""));		
	}

	// Do we need to add a phone note?
	if ( m_IsPhone )
	{
		TTicketNotes Note;
		Note.m_TicketID = m_TicketID;
		Note.m_AgentID  = GetAgentID();
		Note.m_DateCreated = m_DateCreated;
		Note.m_DateCreatedLen = sizeof(Note.m_DateCreated);
		Note.m_IsVoipNote = 1;
		Note.m_ContactID = m_ContactID;
		PutStringProperty( m_Note, &Note.m_Note, &Note.m_NoteAllocated );
				
		// ElapsedTime
		GetISAPIData().GetXMLTCHAR( _T("timetextarea"), Note.m_ElapsedTime, 50, true );
		
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
			if ( sTemp == "PM" && ts.hour != 12 )
			{
				ts.hour = ts.hour + 12;
			}
			else if ( sTemp == "AM" && ts.hour == 12 )
			{
				ts.hour = 0;
			}
			ts.second = 0;
			ts.fraction = 0;
			Note.m_StartTime = ts;
			Note.m_StartTimeLen = sizeof(Note.m_StartTime);
		}
		else
		{
			Note.m_StartTimeLen = SQL_NULL_DATA;
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
			if ( sTemp == "PM" && ts.hour != 12 )
			{
				ts.hour = ts.hour + 12;
			}
			else if ( sTemp == "AM" && ts.hour == 12 )
			{
				ts.hour = 0;
			}
			ts.second = 0;
			ts.fraction = 0;
			Note.m_StopTime = ts;
			Note.m_StopTimeLen = sizeof(Note.m_StopTime);
		}
		else
		{
			Note.m_StopTimeLen = SQL_NULL_DATA;
		}

		Note.Insert( GetQuery() );

		//Log the addition of the note in TicketHistory
		THAddNote( GetQuery(), m_TicketID, GetAgentID(), Note.m_TicketNoteID );

		bool bExist = false;

		GetQuery().Initialize();
		dca::String sCmd;
		sCmd.Format(_T("SELECT TicketNotesReadID FROM TicketNotesRead WHERE TicketNoteID = %d AND AgentID = %d"), Note.m_TicketNoteID, Note.m_AgentID);
		GetQuery().Execute(sCmd.c_str());
		if(GetQuery().Fetch() == S_OK)
			bExist = true;

		if(!bExist)
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), Note.m_TicketNoteID );
			BINDPARAM_LONG( GetQuery(), Note.m_AgentID);
			GetQuery().Execute(_T("INSERT INTO TicketNotesRead (TicketNoteID, AgentID) VALUES (?, ?)"));
			
			// Mark the note we just added as read
			CTicketNotes::UpdateTHNoteRead(GetQuery(), Note.m_AgentID, Note.m_TicketID, 1, Note.m_TicketNoteID);
		}
	}
	else if ( m_Note.length() )
	{
		// we need to add a ticket note
		TTicketNotes Note;
		Note.m_TicketID = m_TicketID;
		Note.m_AgentID  = GetAgentID();
		Note.m_DateCreated = m_DateCreated;
		Note.m_DateCreatedLen = sizeof(Note.m_DateCreated);
		PutStringProperty( m_Note, &Note.m_Note, &Note.m_NoteAllocated );
		
		Note.m_ContactID = 0;
		Note.m_StopTimeLen = SQL_NULL_DATA;
		Note.m_StopTimeLen = SQL_NULL_DATA;

		Note.Insert( GetQuery() );

		//Log the addition of the note in TicketHistory
		THAddNote( GetQuery(), m_TicketID, GetAgentID(), Note.m_TicketNoteID );

		bool bExist = false;

		GetQuery().Initialize();
		dca::String sCmd;
		sCmd.Format(_T("SELECT TicketNotesReadID FROM TicketNotesRead WHERE TicketNoteID = %d AND AgentID = %d"), Note.m_TicketNoteID, Note.m_AgentID);
		GetQuery().Execute(sCmd.c_str());
		if(GetQuery().Fetch() == S_OK)
			bExist = true;

		if(!bExist)
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), Note.m_TicketNoteID );
			BINDPARAM_LONG( GetQuery(), Note.m_AgentID);
			GetQuery().Execute(_T("INSERT INTO TicketNotesRead (TicketNoteID, AgentID) VALUES (?, ?)"));
			
			// Mark the note we just added as read
			CTicketNotes::UpdateTHNoteRead(GetQuery(), Note.m_AgentID, Note.m_TicketID, 1, Note.m_TicketNoteID);
		}
	}
	
	// do we need to refresh the parent ticketlist?
	bool bRefreshParent = false;

	TTicketBoxViews TBoxView;
	GetISAPIData().GetURLLong( _T("TicketBoxView"), TBoxView.m_TicketBoxViewID, true );
	
	// query the ticketbox view we need this information to 
	// determine if the ticketlist needs refreshed.
	if ( TBoxView.m_TicketBoxViewID > 0 )
	{		
		if (TBoxView.Query( GetQuery() ) != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("Invalid TicketboxViewID (%d)"), TBoxView.m_TicketBoxViewID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
	}
	else
	{
		TBoxView.m_TicketBoxID = (TBoxView.m_TicketBoxID * -1);
		TBoxView.m_AgentBoxID = 0;
		TBoxView.m_ShowClosedItems = TRUE;
		TBoxView.m_ShowOwnedItems = TRUE;
	}

	if ( TBoxView.m_AgentBoxID > 0 )
	{
		if ( m_OwnerID == TBoxView.m_AgentBoxID && (TBoxView.m_ShowClosedItems || !m_CloseTicket) )
			bRefreshParent = true;
	}
	else
	{
		if ( m_TicketBoxID == TBoxView.m_TicketBoxID && (TBoxView.m_ShowOwnedItems || !m_OwnerID) &&
		   (TBoxView.m_ShowClosedItems || !m_CloseTicket))
		{
			bRefreshParent = true;
		}
	}

	if (bRefreshParent)
		GetXMLGen().AddChildElem( _T("Refresh") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Creates a new ticket and moves an existing 
||				message thread into it.
\*--------------------------------------------------------------------------*/
void CNewTicket::InsertMoveMsg( void )
{
	unsigned char fromAccess;
	unsigned char toAccess;	
	MsgID_t Msg;
	GetISAPIData().GetURLLong( _T("MessageID"), Msg.MsgID );
	
	tstring sMsgType;
	GetISAPIData().GetURLString( _T("MessageType"), sMsgType, true );
	Msg.IsInbound = sMsgType.compare( _T("inbound") ) == 0;

	// get the Ticket ID of the source ticket
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), m_TicketID );
	BINDPARAM_LONG( GetQuery(), Msg.MsgID );
	
	Msg.IsInbound ? GetQuery().Execute( _T("SELECT TicketID FROM InboundMessages WHERE InboundMessageID=?") ) :
			        GetQuery().Execute( _T("SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID=?") );
	
	if ( GetQuery().Fetch() != S_OK )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("The source message no longer exists") );

	Query();
	
	int nSourceTicketiD = m_TicketID;
	int nSourceTicketBoxID = m_TicketBoxID;

	// now decode the form
	DecodeForm(true);

	// check security
	RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, m_TicketBoxID, EMS_EDIT_ACCESS );
	if(m_TicketBoxID != nSourceTicketBoxID)
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_TICKET_BOX, nSourceTicketBoxID, EMS_EDIT_ACCESS );
		toAccess = GetAgentRightLevel(EMS_OBJECT_TYPE_TICKET_BOX, m_TicketBoxID);
		fromAccess = GetAgentRightLevel(EMS_OBJECT_TYPE_TICKET_BOX, nSourceTicketBoxID);

		if (fromAccess < toAccess)
		{
			CEMSString sError;
			sError.Format(_T("Insufficient rights to the source or destination TicketBox!"));
			THROW_EMS_EXCEPTION(E_InvalidID, sError);
		}
	}
	
	if ( m_OwnerID )
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_OwnerID, EMS_ENUM_ACCESS );

	/*if(!m_IsCopy)
	{
		RequireDelete();
	}*/
	
	Lock( false );

	THCreateTicket( GetQuery(), m_TicketID, GetSession().m_AgentID, 0, m_TicketStateID, m_TicketBoxID, m_OwnerID, m_PriorityID, m_TicketCategoryID );

	//Log it to Agent Activity
	if( _ttoi( sAal.c_str() ) == 1 && _ttoi( sTc.c_str() ) == 1 )
	{
		LogAgentAction(GetAgentID(),4,m_TicketID,m_TicketBoxID,0,_T(""),_T(""));		
	}

	// move the messages
	CChangeTicket ChngTicket(m_ISAPIData);
	ChngTicket.m_SourceTicket.m_TicketID = nSourceTicketiD;
	ChngTicket.m_nDestTicketID = m_TicketID;
	ChngTicket.m_SourceMsg = Msg;
	if(m_IsCopy)
	{
        ChngTicket.CopyMessage();
	}
	else
	{
		ChngTicket.MoveMessage();
	}

	// update the contacts of both tickets
	RebuildTicketContacts( GetQuery(), nSourceTicketiD );
	RebuildTicketContacts( GetQuery(), m_TicketID );
		
	SaveTicket();

	// do we need to add a ticket note?
	if ( m_Note.length() )
	{
		TTicketNotes Note;
		Note.m_TicketID = m_TicketID;
		Note.m_AgentID  = GetAgentID();
		Note.m_DateCreated = m_DateCreated;
		Note.m_DateCreatedLen = sizeof(Note.m_DateCreated);
		PutStringProperty( m_Note, &Note.m_Note, &Note.m_NoteAllocated );
		
		Note.Insert( GetQuery() );
	}

	int nTicketBoxView;
	tstring sShowDest;
	
	GetISAPIData().GetURLLong( _T("TicketBoxView"), nTicketBoxView );
	GetISAPIData().GetURLString( _T("ShowDest"), sShowDest );

	ChngTicket.GetTicketBoxView( m_TicketID, nTicketBoxView, sShowDest.compare( _T("true") ) == 0 );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Updates the ticket in the database and marks it as
||				undeleted.
\*-------------------------------------------------------------------------*/
void CNewTicket::SaveTicket()
{
	// should the ticket be created as closed?
	m_TicketStateID = m_CloseTicket ? EMS_TICKETSTATEID_CLOSED : EMS_TICKETSTATEID_OPEN;
	m_IsDeleted = 0;
		
	GetQuery().Initialize();
	
	BINDPARAM_LONG( GetQuery(), m_TicketStateID );
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	BINDPARAM_LONG( GetQuery(), m_TicketCategoryID );
	BINDPARAM_LONG( GetQuery(), m_OwnerID );
	BINDPARAM_TCHAR( GetQuery(), m_Subject );
	BINDPARAM_LONG( GetQuery(), m_PriorityID );
	BINDPARAM_TIME( GetQuery(), m_DateCreated );
	BINDPARAM_TIME( GetQuery(), m_OpenTimestamp );
	BINDPARAM_TINYINT( GetQuery(), m_IsDeleted );
	BINDPARAM_LONG( GetQuery(), m_LockedBy );
	BINDPARAM_LONG( GetQuery(), m_TicketID );
	
	GetQuery().Execute( _T("UPDATE Tickets ")
						_T("SET TicketStateID=?,TicketBoxID=?,TicketCategoryID=?,OwnerID=?,Subject=?,PriorityID=?,DateCreated=?,")
						_T("OpenTimestamp=?,IsDeleted=?,LockedBy=? WHERE TicketID=?") );

	if ( GetQuery().GetRowCount() == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Unable to update ticket, the ticket does not exist") );

	if(m_UseTickler != -1 && m_ReOpenTime.year != 0)
	{
		SetTickler();
	}

	// Add/Update any Custom Ticket Fields
	for( tftIter = m_tft.begin(); tftIter != m_tft.end(); tftIter++ )
	{
		AddTicketFieldTicket(tftIter->m_TicketID,tftIter->m_TicketFieldID,tftIter->m_TicketFieldTypeID,tftIter->m_DataValue);
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Handle the case where only a contact name was entered	              
\*--------------------------------------------------------------------------*/
bool CNewTicket::HandleContact( void )
{
	CCreateContactMutex lock;
	
	// security check - the agent must have atleast read access to contacts
	unsigned char access = RequireAgentRightLevel( EMS_OBJECT_TYPE_CONTACT, 0, EMS_READ_ACCESS );
	
	// obtain the contact lock		
	if( lock.AcquireLock( 1000 ) == false )
	{
		THROW_EMS_EXCEPTION( E_CreateContact, CEMSString(EMS_STRING_ERROR_CREATE_CONTACT) );
	}

	if ( m_sEmail.length() > 0 )
	{
		if ( ( m_ContactID = FindContactByEmail( GetQuery(), m_sEmail.c_str() ) ) <= 0)
		{
			// didn't find the contact, if we don't have edit rights, throw an error
			if ( access < EMS_EDIT_ACCESS )
				LogSecurityViolation( EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS );
	
			// create the contact
			if ((m_ContactID = CreateContactFromEmail( GetQuery(), m_sEmail.c_str(), m_sName.c_str() )) < 1)
				THROW_EMS_EXCEPTION( E_CreateContact, CEMSString(EMS_STRING_ERROR_CREATE_CONTACT) );
		}
	}
	else
	{
		if ( FindContactsByName(false) == 0 )
		{
			// didn't find the contact, if we don't have edit rights, throw an error
			if ( access < EMS_EDIT_ACCESS )
				LogSecurityViolation( EMS_OBJECT_TYPE_CONTACT, 0, EMS_EDIT_ACCESS );
			
			// create the contact
			if ((m_ContactID = CreateContactFromEmail( GetQuery(), m_sEmail.c_str(), m_sName.c_str() )) < 1)
				THROW_EMS_EXCEPTION( E_CreateContact, CEMSString(EMS_STRING_ERROR_CREATE_CONTACT) );
		}
		else
		{
			GetXMLGen().AddChildElem( _T("postresults") );
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("complete"), _T("SelectContact") );
			GetXMLGen().OutOfElem();
			return false;
		}
	}
	
	return true;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Returns the number of contacts with the exact same name
||				as m_sName	              
\*--------------------------------------------------------------------------*/
int CNewTicket::FindContactsByName( bool bAddToXML )
{
	int nContacts;
	TCHAR szEmail[PERSONALDATA_DATAVALUE_LENGTH];
	long szEmailLen;
	
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), m_ContactID );
	BINDCOL_TCHAR( GetQuery(), szEmail );
	
	BINDPARAM_TCHAR( GetQuery(), (TCHAR*) m_sName.c_str() );
	
	GetQuery().Execute( _T("SELECT ContactID, ")
		_T("(SELECT DataValue FROM PersonalData WHERE PersonalDataID = Contacts.DefaultEmailAddressID) ") 
		_T("FROM Contacts WHERE Name = ?") );
	
	bool bHasEmail;

	for ( nContacts = 0; GetQuery().Fetch() == S_OK; nContacts++ )
	{
		if ( bAddToXML )
		{
			bHasEmail = szEmailLen > 0 && szEmailLen != SQL_NULL_DATA;
				
			GetXMLGen().AddChildElem( _T("Contact") );
			GetXMLGen().AddChildAttrib( _T("ID"), m_ContactID );
			
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("Name") );
			GetXMLGen().SetChildData( m_sName.c_str(), true );
			GetXMLGen().OutOfElem();
			
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("Email") );
			GetXMLGen().SetChildData( bHasEmail ? szEmail : _T("[No default email address defined]"), true );
			GetXMLGen().OutOfElem();
		}
	}

	return nContacts;
}