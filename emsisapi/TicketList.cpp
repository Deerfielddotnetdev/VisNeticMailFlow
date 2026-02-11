/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketList.cpp,v 1.2.2.3.2.2 2006/09/05 14:16:08 markm Exp $
||
||
||                                         
||  COMMENTS:	Ticket List    
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketList.h"
#include "EMSString.h"
#include "TicketBoxViewFns.h"
#include "DateFns.h"
#include "ListClasses.h"
#include "Ticket.h"
#include "MessageList.h"
#include "InboundMessage.h"

/*---------------------------------------------------------------------------\             
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CTicketList::CTicketList(CISAPIData& ISAPIData)
	:CPagedList(ISAPIData)
	,m_CustomViewID(0)
{
	m_TicketIDArray = NULL;
	PCFreq = 0.0;
	CounterStart = 0;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Destruction
\*--------------------------------------------------------------------------*/
CTicketList::~CTicketList()
{
	if (m_TicketIDArray)
		delete[] m_TicketIDArray;
}

/*---------------------------------------------------------------------------\                    
||  Comments:	Main Entry Point              
\*--------------------------------------------------------------------------*/
int CTicketList::Run( CURLAction& action )
{	
	//DebugReporter::Instance().DisplayMessage("CTicketList::Run - Entering", DebugReporter::ISAPI, GetCurrentThreadId());
	dca::String f;

	int nTicketRead = 0;

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("ticketread") ) == 0)
	{
		int nTicketID = 0;
		int nRead;

		if(!GetISAPIData().GetXMLLong( _T("ticketID"), nTicketID, true))
			GetISAPIData().GetURLLong( _T("ticketID"), nTicketID, true);

		if(!GetISAPIData().GetXMLLong( _T("action"), nRead, true))
			GetISAPIData().GetURLLong( _T("action"), nRead, true);

		f.Format("CTicketList::Run TicketRead TicketID [%d] Read/Unread [%d]", nTicketID,nRead);
		//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

		if(nTicketID != 0)
		{
			SetInboundMessagesAsRead(nTicketID, nRead);
			SetTicketNotesAsRead(nTicketID, nRead);
		}
	}
	else if(GetISAPIData().GetURLLong( _T("ticketread"), nTicketRead, true))
	{
		//DebugReporter::Instance().DisplayMessage("CTicketList::Run - ticketread", DebugReporter::ISAPI, GetCurrentThreadId());
		
		int nTicketID = 0;
		int nRead;

		if(!GetISAPIData().GetXMLLong( _T("ticketID"), nTicketID, true))
			GetISAPIData().GetURLLong( _T("ticketID"), nTicketID, true);

		if(!GetISAPIData().GetXMLLong( _T("action"), nRead, true))
			GetISAPIData().GetURLLong( _T("action"), nRead, true);

		if(nTicketID != 0)
		{
			SetInboundMessagesAsRead(nTicketID, nRead);
			SetTicketNotesAsRead(nTicketID, nRead);
		}
	}
	else
	{
		if(GetISAPIData().GetURLLong( _T("TicketBoxView"), m_TBView.m_TicketBoxViewID, true))
		{
			//DebugReporter::Instance().DisplayMessage("CTicketList::Run - TicketBoxView", DebugReporter::ISAPI, GetCurrentThreadId());

			tstring sAction;

			if ((GetISAPIData().GetURLString( _T("Action"), sAction, true )) ||
				(GetISAPIData().GetXMLString( _T("Action"), sAction, true )))
			{
				f.Format("CTicketList::Run - Action: %s", sAction.c_str() );
				//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
				if (sAction.compare(_T("delete")) == 0)
					DeleteTicket();
				
				else if (sAction.compare(_T("takeownership")) == 0)
					TakeOwnership();

				else if (sAction.compare(_T("sort")) == 0)
					ChangeSortOrder();

				else if (sAction.compare(_T("getoldest")) == 0)
					GetOldest();

				else if ( sAction.compare( _T("escalate") ) == 0 )
					Escalate();
			}
			f.Format("CTicketList::Run - TicketBoxViewID: %d",m_TBView.m_TicketBoxViewID );
			//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
			m_TBView.m_TicketBoxViewID >= 0 ? ListTicketBoxView(action) : ListTicketBox();
		}
		else if(GetISAPIData().GetURLLong( _T("CustomTicketBoxView"), m_CustomViewID ))
		{
			//DebugReporter::Instance().DisplayMessage("CTicketList::Run - Custom TicketBoxView", DebugReporter::ISAPI, GetCurrentThreadId());

			tstring sAction;

			if ((GetISAPIData().GetURLString( _T("Action"), sAction, true )) ||
				(GetISAPIData().GetXMLString( _T("Action"), sAction, true )))
			{
				if (sAction.compare(_T("delete")) == 0)
					DeleteTicket();
				else if (sAction.compare(_T("sort")) == 0)
					ChangeSortOrder();
			}

			m_CustomViewID >= 0 ? ListTicketBoxView(action) : ListTicketBox();
		}
		else
		{
			//DebugReporter::Instance().DisplayMessage("CTicketList::Run - ListTicketBox", DebugReporter::ISAPI, GetCurrentThreadId());

			ListTicketBox();
		}
	}

	//DebugReporter::Instance().DisplayMessage("CTicketList::Run - Leaving", DebugReporter::ISAPI, GetCurrentThreadId());

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Displays a ticketbox without a view, this is necessary 
||				to display the destination ticket for change ticket.  
||				This functionality could be redesigned.	              
\*--------------------------------------------------------------------------*/
void CTicketList::ListTicketBox(void)
{
	m_TBView.m_ViewTypeID = EMS_PUBLIC;
	m_TBView.m_TicketBoxID = abs(m_TBView.m_TicketBoxViewID );
	m_TBView.m_AgentID = GetAgentID();
	m_TBView.m_SortField = EMS_COLUMN_DATE;
	m_TBView.m_SortAsc = TRUE;
	m_TBView.m_ShowClosedItems = TRUE;
	m_TBView.m_ShowOwnedItems  = TRUE;
	
	List();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Displays the contents of a ticketbox view	              
\*--------------------------------------------------------------------------*/
void CTicketList::ListTicketBoxView( CURLAction& action )
{	
	long m_lStart;
	long m_lEnd;
	long m_lElapsed;
	dca::String f;

	if(m_CustomViewID != 0)
	{
		int nAgentID = 0;
		
		if(!GetISAPIData().GetURLLong( _T("AgentID"), nAgentID, true))
			nAgentID = m_ISAPIData.m_pSession->m_AgentID;

		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_CustomViewID );
		BINDPARAM_LONG( GetQuery(), nAgentID );
		BINDCOL_LONG_NOLEN( GetQuery(), m_TBView.m_TicketBoxViewID );
		GetQuery().Execute( _T("SELECT ParentID FROM Folders WHERE (FolderID = ?) AND (AgentID = ?)"));
		if ( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}

	// if the TicketBoxViewID is 0, display the agent's my tickets
	if ( m_TBView.m_TicketBoxViewID == 0 )
	{
		GetQuery().Initialize();
		
		int nTypeID = EMS_INBOX;
		
		BINDCOL_LONG_NOLEN( GetQuery(), m_TBView.m_TicketBoxViewID );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		BINDPARAM_LONG( GetQuery(), nTypeID );
		
		GetQuery().Execute( _T("SELECT TicketBoxViewID FROM TicketBoxViews WHERE AgentID=? AND AgentBoxID=? ")
			_T("AND TicketBoxViewTypeID=?") );
		
		if ( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}
	
	// get info about the TicketBoxView
	m_TBView.Query( GetQuery() );

	if ( m_TBView.m_UseDefault == 1 )
	{
		//Get default column views
		GetQuery().Initialize();
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowState);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowPriority);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowNumNotes);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowTicketID);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowNumMsgs);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowSubject);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowContact);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowDate);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowCategory);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowOwner);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowTicketBox);
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		GetQuery().Execute( _T("SELECT ShowState,ShowPriority,ShowNumNotes,ShowTicketID,ShowNumMsgs, ")
				   _T("ShowSubject,ShowContact,ShowDate,ShowCategory,ShowOwner,ShowTicketBox ")
		           _T("FROM TicketBoxViews ")
		           _T("WHERE AgentID=? AND TicketBoxViewTypeID=9"));

		if ( GetQuery().Fetch() != S_OK )
		{
			//Set the defaults
			m_TBView.m_ShowState = 1;
			m_TBView.m_ShowPriority = 1;
			m_TBView.m_ShowNumNotes = 1;
			m_TBView.m_ShowTicketID = 1;
			m_TBView.m_ShowNumMsgs = 1;
			m_TBView.m_ShowSubject = 1;
			m_TBView.m_ShowContact = 1;
			m_TBView.m_ShowDate = 1;
			m_TBView.m_ShowCategory = 1;
			m_TBView.m_ShowOwner = 1;
			m_TBView.m_ShowTicketBox = 1;

			//Add the view
			GetQuery().Reset( true );
			BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowState);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowPriority);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowNumNotes);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowTicketID);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowNumMsgs);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowSubject);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowContact);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowDate);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowCategory);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowOwner);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowTicketBox);
			GetQuery().Execute( _T("INSERT INTO TicketBoxViews (AgentID,TicketBoxViewTypeID,ShowState,ShowPriority, ")
								_T("ShowNumNotes,ShowTicketID,ShowNumMsgs,ShowSubject,ShowContact, ")
								_T("ShowDate,ShowCategory,ShowOwner,ShowTicketBox) ")
								_T("VALUES (?,9,?,?,?,?,?,?,?,?,?,?,?)") );	

		}
	}

		
	// does the view type use a ticket list
	switch(m_TBView.m_ViewTypeID) 
	{
	case EMS_INBOX:
	case EMS_PUBLIC:
		break;
		
	default:
		
		// the view is a message list!!
		action.m_PageName = _T("messagelist.ems");
		CMessageList MessageList(m_ISAPIData);
		m_lStart = GetTickCount();	
		MessageList.Run(action);
		m_lEnd = GetTickCount();
		m_lElapsed = m_lEnd-m_lStart;
		f.Format("CTicketList::ListTicketBoxView - MessageList.Run AgentID: %d - TicketBoxViewID: %d - Elapsed: %d", m_ISAPIData.m_pSession->m_AgentID,m_TBView.m_TicketBoxViewID,m_lElapsed );
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		return;
	}

	// update filter
	UpdateFilter();
	
	List();
	
}

/*---------------------------------------------------------------------------\                  
||  Comments:	Lists the tickets in a ticketbox	              
\*--------------------------------------------------------------------------*/
void CTicketList::List(void)
{
	//DebugReporter::Instance().DisplayMessage("CTicketList::List - Entering", DebugReporter::ISAPI, GetCurrentThreadId());
	
	unsigned char Access;
	long m_lStart;
	long m_lEnd;
	long m_lElapsed;
	dca::String f;
	
	if (m_TBView.m_ViewTypeID == EMS_PUBLIC)
	{
		Access= RequireAgentRightLevel(EMS_OBJECT_TYPE_TICKET_BOX, m_TBView.m_TicketBoxID, EMS_READ_ACCESS);
	}
	else
	{
		Access = RequireAgentRightLevel(EMS_OBJECT_TYPE_AGENT, m_TBView.m_AgentBoxID, EMS_READ_ACCESS);
	}
	
	//DebugReporter::Instance().DisplayMessage("CTicketList::List - Access allowed", DebugReporter::ISAPI, GetCurrentThreadId());
	
	unsigned char RequireGetOldest = false;
	int nTicketLink = 0;
	
	if ( m_TBView.m_ViewTypeID != EMS_INBOX )
	{
		RequireGetOldest = GetRequireGetOldest();

		dca::String x;
		x.Format("CTicketList::List - AgentID: [%d] RequireGetOldest for TicketBoxID: [%d] set to: [%d]", m_TBView.m_AgentID, m_TBView.m_TicketBoxID, RequireGetOldest);
		DebugReporter::Instance().DisplayMessage(x.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
		if ( ! RequireGetOldest && m_TBView.m_TicketBoxID > 0 )
		{
			GetQuery().Initialize();
			
			BINDCOL_BIT_NOLEN( GetQuery(), RequireGetOldest );
			BINDCOL_LONG_NOLEN( GetQuery(), nTicketLink );
			BINDPARAM_LONG( GetQuery(), m_TBView.m_TicketBoxID );
			
			GetQuery().Execute( _T("SELECT RequireGetOldest,TicketLink FROM TicketBoxes WHERE TicketBoxID = ?") );
			
			GetQuery().Fetch();

			x.Format("CTicketList::List - RequireGetOldest for TicketBoxID: [%d] set to: [%d]", m_TBView.m_TicketBoxID, RequireGetOldest);
			DebugReporter::Instance().DisplayMessage(x.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
				
		}
	}
	else
	{
		nTicketLink = 1;
	}
	
	//DebugReporter::Instance().DisplayMessage("CTicketList::List - SetItemCount", DebugReporter::ISAPI, GetCurrentThreadId());
	
	m_lStart = GetTickCount();		
	SetItemCount(m_TBView.GetItemCount(GetQuery(), m_CustomViewID));

	if ( RequireGetOldest )
	{
		// if the agent has to use get oldest there is only one page
		//DebugReporter::Instance().DisplayMessage("CTicketList::List - SetPageCount", DebugReporter::ISAPI, GetCurrentThreadId());
		SetPageCount(1);
		//DebugReporter::Instance().DisplayMessage("CTicketList::List - SetCurrentPage", DebugReporter::ISAPI, GetCurrentThreadId());
		SetCurrentPage(1);
	}
	else
	{
		//DebugReporter::Instance().DisplayMessage("CTicketList::List - FindTicketPage", DebugReporter::ISAPI, GetCurrentThreadId());
		FindTicketPage();
	}
	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CTicketList::List - Set Pages TicketBoxViewID: %d - Elapsed: %d", m_TBView.m_TicketBoxViewID,m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	m_lStart = GetTickCount();		
	// add page count information
	AddPageXML();
	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CTicketList::List - AddPageXML TicketBoxViewID: %d - Elapsed: %d", m_TBView.m_TicketBoxViewID,m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	m_lStart = GetTickCount();		
	//Get any custom fields for this TicketBoxView
	GetXMLGen().AddChildElem( _T("CustomColumns") );
	GetXMLGen().IntoElem();

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tf.m_TicketFieldID );
	BINDCOL_TCHAR( GetQuery(), tf.m_Description );
	BINDCOL_LONG( GetQuery(), tf.m_TicketFieldTypeID );
	BINDPARAM_LONG( GetQuery(), m_TBView.m_TicketBoxViewID );
	GetQuery().Execute( _T("SELECT TicketFields.TicketFieldID,TicketFields.Description,TicketFields.TicketFieldTypeID FROM TicketFields ")
						_T("INNER JOIN TicketFieldViews ON TicketFields.TicketFieldID=TicketFieldViews.TicketFieldID ")
						_T("WHERE TicketFieldViews.TicketBoxViewID=? AND TicketFieldViews.ShowField=1") );
		while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Column") );
		GetXMLGen().AddChildAttrib( _T("TFID"), tf.m_TicketFieldID );	
		GetXMLGen().AddChildAttrib( _T("TFTID"), tf.m_TicketFieldTypeID );	
		GetXMLGen().AddChildAttrib( _T("Name"), tf.m_Description );		
	}
	
	GetXMLGen().OutOfElem();
	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CTicketList::List - Add TicketBox Custom Fields TicketBoxViewID: %d - Elapsed: %d", m_TBView.m_TicketBoxViewID,m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	m_lStart = GetTickCount();	
	// add ticketbox information
	GetXMLGen().AddChildElem( _T("TicketBoxItemList") );
	GetXMLGen().AddChildAttrib( _T("TicketBoxView"), m_TBView.m_TicketBoxViewID );
	GetXMLGen().AddChildAttrib( _T("AccessLevel"), Access );
	GetXMLGen().AddChildAttrib( _T("TypeID"), m_TBView.m_ViewTypeID );
	AddTicketBoxViewTypeName( _T("Type"), m_TBView.m_ViewTypeID );
	GetXMLGen().AddChildAttrib( _T("RequireGetOldest"), RequireGetOldest );
	GetXMLGen().AddChildAttrib( _T("TicketLink"), nTicketLink );

	if ( m_TBView.m_TicketBoxViewID > 0 )
	{
		GetXMLGen().AddChildAttrib( _T("SortBy"), GetXMLSortColumnName() );
		GetXMLGen().AddChildAttrib( _T("SortAscending"), m_TBView.m_SortAsc );
	}
	else
	{
		GetXMLGen().AddChildAttrib( _T("SortBy"), _T("-1") );
		GetXMLGen().AddChildAttrib( _T("SortAscending"), _T("-1") );
	}

	GetXMLGen().AddChildAttrib( _T("ShowOwnedItems"), m_TBView.m_ShowOwnedItems);
	GetXMLGen().AddChildAttrib( _T("ShowClosedItems"), m_TBView.m_ShowClosedItems);

	GetXMLGen().AddChildAttrib( _T("ShowState"), m_TBView.m_ShowState);
	GetXMLGen().AddChildAttrib( _T("ShowPriority"), m_TBView.m_ShowPriority);
	GetXMLGen().AddChildAttrib( _T("ShowNumNotes"), m_TBView.m_ShowNumNotes);
	GetXMLGen().AddChildAttrib( _T("ShowTicketID"), m_TBView.m_ShowTicketID);
	GetXMLGen().AddChildAttrib( _T("ShowNumMsgs"), m_TBView.m_ShowNumMsgs);
	GetXMLGen().AddChildAttrib( _T("ShowSubject"), m_TBView.m_ShowSubject);
	GetXMLGen().AddChildAttrib( _T("ShowContact"), m_TBView.m_ShowContact);
	GetXMLGen().AddChildAttrib( _T("ShowDate"), m_TBView.m_ShowDate);
	GetXMLGen().AddChildAttrib( _T("ShowCategory"), m_TBView.m_ShowCategory);
	GetXMLGen().AddChildAttrib( _T("ShowOwner"), m_TBView.m_ShowOwner);
	GetXMLGen().AddChildAttrib( _T("ShowTicketBox"), m_TBView.m_ShowTicketBox);
	
	m_TBView.m_AgentBoxID ? AddAgentName( _T("Agent"), m_TBView.m_AgentBoxID ) : 
	                        GetXMLGen().AddChildAttrib( _T("Agent"), GetAgentName());


	if(m_CustomViewID != 0)
	{
		dca::String55 sName;
		long           sNameLen;
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_CustomViewID);
		BINDCOL_TCHAR( GetQuery(), sName);
		GetQuery().Execute( _T("SELECT Folders.Name FROM Folders WHERE Folders.FolderID = ?"));
		GetQuery().Fetch();
		
		// Fix
		GetXMLGen().AddChildAttrib( _T("TicketBoxName"), sName );
		GetXMLGen().AddChildAttrib( _T("customviewid"), m_CustomViewID );
		GetXMLGen().AddChildAttrib( _T("customagentid"), m_TBView.m_AgentBoxID);
	}
	else
	{
		m_TBView.m_TicketBoxID ? AddTicketBoxName( _T("TicketBoxName"), m_TBView.m_TicketBoxID ) :
	                             AddTicketBoxViewTypeName( _T("TicketBoxName"), m_TBView.m_ViewTypeID );

	    GetXMLGen().AddChildAttrib( _T("TicketBoxID"), m_TBView.m_TicketBoxID );
	}
	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CTicketList::List - Add TicketBox Information TicketBoxViewID: %d - Elapsed: %d", m_TBView.m_TicketBoxViewID,m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	
	// bail out now if the agent must get the oldest item
	if ( RequireGetOldest )
		return;

	// query the database
	Query();
	//DebugReporter::Instance().DisplayMessage("CTicketList::List - Leaving", DebugReporter::ISAPI, GetCurrentThreadId());	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Formats the query string             
\*--------------------------------------------------------------------------*/
void CTicketList::Query(void)
{
	CEMSString sQuery;
	CEMSString sOwnerID;
	CEMSString sFolderID;
	CEMSString sTicketBoxID;
	tstring sTicketStateID;
	tstring sCount;
	dca::String sTicketListDate;
	
	long m_lStart;
	long m_lEnd;
	long m_lElapsed;
	dca::String f;
	
	m_lStart = GetTickCount();	
	// build the ORDER BY string
	GetOrderBy();

	GetServerParameter(67, sTicketListDate, "0");

	// format the query
		
	if ( !m_TBView.m_ShowClosedItems )
		sTicketStateID.assign( _T("AND TicketStateID > 1") );

	if (m_TBView.m_ViewTypeID == EMS_INBOX)
	{
		sOwnerID.Format( _T("AND OwnerID = %d"), m_TBView.m_AgentBoxID );
		sFolderID.Format( _T("AND FolderID = %d"), m_CustomViewID );
	}
	else if (!m_TBView.m_ShowOwnedItems)
	{
		sOwnerID.assign( _T("AND OwnerID = 0") );
	}

	if (m_TBView.m_ViewTypeID == EMS_PUBLIC)
		sTicketBoxID.Format( "AND Tickets.TicketBoxID = %d", m_TBView.m_TicketBoxID );

	

	if ( m_TBView.m_SortField == EMS_COLUMN_COUNT )
	{
		sCount.assign( _T(",(SELECT COUNT(*) FROM InboundMessages WHERE Tickets.TicketID = ")
			_T("InboundMessages.TicketID AND IsDeleted=0) + ")
			_T("(SELECT COUNT(*) FROM OutboundMessages WHERE Tickets.TicketID = ")
			_T("OutboundMessages.TicketID AND IsDeleted=0) as MsgCount ") );
	}
	else if ( m_TBView.m_SortField == EMS_COLUMN_NOTE_COUNT )
	{
		sCount.assign( _T(",(SELECT COUNT(*) FROM TicketNotes WHERE ")
					   _T("Tickets.TicketID = TicketNotes.TicketID) as NoteCount ") );
	}
	else if ( m_TBView.m_SortField == EMS_COLUMN_ATTACH_COUNT )
	{
		sCount.assign( _T(",(SELECT COUNT(*) FROM InboundMessageAttachments ima INNER JOIN InboundMessages im ON ima.InboundMessageID=im.InboundMessageID WHERE Tickets.TicketID = ")
			_T("im.TicketID AND im.IsDeleted=0) + ")
			_T("(SELECT COUNT(*) FROM OutboundMessageAttachments oma INNER JOIN OutboundMessages om ON oma.OutboundMessageID=om.OutboundMessageID WHERE Tickets.TicketID = ")
			_T("om.TicketID AND om.IsDeleted=0) as AttachCount ") );
	}
	else if ( m_TBView.m_SortField == EMS_COLUMN_DATE && sTicketListDate.ToInt() > 0)
	{
		if(sTicketListDate.ToInt() == 1)
		{
			sCount.assign( _T(",(SELECT CASE WHEN (SELECT TOP 1 DateTime FROM TicketHistory WHERE TicketID=Tickets.TicketID ")
						_T("AND (TicketActionID=5 OR (TicketActionID=10 AND ID1=0)) ORDER BY TicketHistoryID DESC) <> ")
						_T("OpenTimeStamp THEN (SELECT TOP 1 DateTime FROM TicketHistory WHERE TicketID=Tickets.TicketID ")
						_T("AND (TicketActionID=5 OR (TicketActionID=10 AND ID1=0)) ORDER BY TicketHistoryID DESC) ELSE OpenTimeStamp ")
						_T("END) AS OpenTimeStamp ") );
		}
		else if(sTicketListDate.ToInt() == 2)
		{
			sCount.assign( _T(",(SELECT CASE WHEN (SELECT TOP 1 DateTime FROM TicketHistory WHERE TicketID=Tickets.TicketID ")
						_T("AND (TicketActionID=5 OR (TicketActionID=10 AND ID1=0)) ORDER BY TicketHistoryID ASC) < ")
						_T("OpenTimeStamp THEN (SELECT TOP 1 DateTime FROM TicketHistory WHERE TicketID=Tickets.TicketID ")
						_T("AND (TicketActionID=5 OR (TicketActionID=10 AND ID1=0)) ORDER BY TicketHistoryID ASC) ELSE OpenTimeStamp ")
						_T("END) AS OpenTimeStamp ") );
			
			/*sCount.assign( _T(",(SELECT CASE WHEN (SELECT COUNT(*) FROM InboundMessages WHERE TicketID=Tickets.TicketID) = 1 ")
						_T("THEN (SELECT TOP 1 EmailDateTime FROM InboundMessages WHERE TicketID=Tickets.TicketID) ")
						_T("ELSE OpenTimeStamp ")
						_T("END) AS OpenTimeStamp ") );*/
		}
	}

	if ( !GetIsAdmin() && g_ThreadPool.GetSharedObjects().m_bUseATC )
	{
		sQuery.Format( _T("SELECT TOP %d TicketID %s FROM Tickets ")
					   _T("INNER JOIN AgentTicketCategories ATC ON ")
					   _T("Tickets.TicketCategoryID=ATC.TicketCategoryID ")
					   _T("WHERE ATC.AgentID=%d AND IsDeleted = 0 ")
					   _T("%s %s %s %s ORDER BY %s "), 
					   GetEndRow(), sCount.c_str(), m_ISAPIData.m_pSession->m_AgentID, sFolderID.c_str(), sTicketStateID.c_str(), sOwnerID.c_str(), 
					   sTicketBoxID.c_str(), m_sSort.c_str() );
	}
	else
	{
		sQuery.Format( _T("SELECT TOP %d TicketID %s FROM Tickets ")
					   _T("WHERE IsDeleted = 0 ")
					   _T("%s %s %s %s ORDER BY %s "), 
					   GetEndRow(), sCount.c_str(), sFolderID.c_str(), sTicketStateID.c_str(), sOwnerID.c_str(), 
					   sTicketBoxID.c_str(), m_sSort.c_str() );
	}
	

	// allocate storage
	m_TicketIDArray = new int[GetMaxRowsPerPage()];

	// preform the query
	GetQuery().Initialize(GetMaxRowsPerPage(), sizeof(int));
	GetQuery().EnableScrollCursor();

	// bind
	BINDCOL_LONG_NOLEN(GetQuery(), m_TicketIDArray[0]);
	
	f.Format("CTicketList::Query TicketBoxViewID: %d - getting ready to execute query [%s]", m_TBView.m_TicketBoxViewID,sQuery.c_str() );
	//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	
	GetQuery().Execute( sQuery.c_str() );
	
	f.Format("CTicketList::Query TicketBoxViewID: %d - fetching rows", m_TBView.m_TicketBoxViewID,sQuery.c_str() );
	//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	
	// fetch the rows
	GetQuery().FetchScroll( SQL_FETCH_ABSOLUTE, GetStartRow() );

	int nTickets = GetQuery().GetRowsFetched();

	GetQuery().Reset();

	f.Format("CTicketList::Query TicketBoxViewID: %d - generating TicketList XML", m_TBView.m_TicketBoxViewID,nTickets );
	//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	
	if ( nTickets )
		GenTicketListXML(m_TicketIDArray,nTickets);

	if ( m_TicketIDArray )
	{
		m_TicketIDArray = NULL;
		delete[] m_TicketIDArray;
	}
	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CTicketList::Query TicketBoxViewID: %d - Elapsed: %d", m_TBView.m_TicketBoxViewID,m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());	
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the XML for the list of tickets
\*--------------------------------------------------------------------------*/
void CTicketList::GenTicketListXML( int* pArray, int nTickets )
{	
	double dTick1;
	double dTick2;
	double dTick3;
	double dTick4;
	double dTick5;
	double dTot = 0;
	dca::String f;
		
	StartCounter();

	CTicketListItem Ticket;
	CEMSString sQuery;
	CEMSString sSubject;
	CEMSString sContacts;
	CEMSString sOpenTime;
	dca::String sTicketListDate;
	GetServerParameter(67, sTicketListDate, "0");
	dca::String sVoipEnabled;
	GetServerParameter(68, sVoipEnabled, "0");
		
	if(sTicketListDate.ToInt() == 1)
	{
		sOpenTime.Format( _T("CASE WHEN (SELECT TOP 1 DateTime FROM TicketHistory WHERE TicketID=Tickets.TicketID ")
					  _T("AND (TicketActionID=5 OR (TicketActionID=10 AND ID1=0)) ORDER BY TicketHistoryID DESC) <> ")
					  _T("OpenTimeStamp THEN (SELECT TOP 1 DateTime FROM TicketHistory WHERE TicketID=Tickets.TicketID ")
					  _T("AND (TicketActionID=5 OR (TicketActionID=10 AND ID1=0)) ORDER BY TicketHistoryID DESC) ELSE OpenTimeStamp ")
					  _T("END AS OpenTimeStamp") );
	}
	else if(sTicketListDate.ToInt() == 2)
	{
		sOpenTime.Format( _T("CASE WHEN (SELECT TOP 1 DateTime FROM TicketHistory WHERE TicketID=Tickets.TicketID ")
					  _T("AND (TicketActionID=5 OR (TicketActionID=10 AND ID1=0)) ORDER BY TicketHistoryID ASC) < ")
					  _T("OpenTimeStamp THEN (SELECT TOP 1 DateTime FROM TicketHistory WHERE TicketID=Tickets.TicketID ")
					  _T("AND (TicketActionID=5 OR (TicketActionID=10 AND ID1=0)) ORDER BY TicketHistoryID ASC) ELSE OpenTimeStamp ")
					  _T("END AS OpenTimeStamp") );
		
		/*sOpenTime.Format( _T("CASE WHEN (SELECT COUNT(*) FROM InboundMessages WHERE TicketID=Tickets.TicketID) = 1 ")
					  _T("THEN (SELECT TOP 1 EmailDateTime FROM InboundMessages WHERE TicketID=Tickets.TicketID) ")
					  _T("ELSE OpenTimeStamp ")
					  _T("END AS OpenTimeStamp") );*/
	}
	else
	{
		sOpenTime.Format( _T("OpenTimeStamp"));
	}

	
	
	tstring sIDString;
	sIDString.reserve( nTickets * 10 );
	
	CEMSString sFormat;
	for(int i = 0; i < nTickets; i++)
	{
		sFormat.Format( "%d", pArray[i]);
		sIDString += sFormat.c_str();
		
		if ( i < (nTickets - 1))
			sIDString += ",";
	}
	
	// add the tickets to the XML
	GetXMLGen().IntoElem();
	
	sQuery.Format(  _T("SELECT TicketID, Subject, Contacts, %s, PriorityID, TicketStateID, ")
					_T("OwnerID, LockedBy, TicketBoxID, UseTickler, TicketCategoryID, ")

					_T("(SELECT COUNT(*) FROM InboundMessages WHERE ")
					_T("Tickets.TicketID = InboundMessages.TicketID AND IsDeleted=0)+")
					
					_T("(SELECT COUNT(*) FROM OutboundMessages WHERE ")
					_T("Tickets.TicketID = OutboundMessages.TicketID AND IsDeleted=0) as MsgCount, ")  
					
					_T("(SELECT COUNT(*) FROM TicketNotes WHERE ")
					_T("Tickets.TicketID = TicketNotes.TicketID) as NoteCount, ")
					
					_T("(SELECT UnreadMode FROM TicketBoxes WHERE ")
					_T("Tickets.TicketBoxID = TicketBoxes.TicketBoxID) as UnreadMode, ")

					_T("(SELECT COUNT(*) FROM TicketLinksTicket WHERE ")
					_T("Tickets.TicketID = TicketLinksTicket.TicketID) as LinkCount, ")
					
					_T("(SELECT COUNT(*) FROM InboundMessageAttachments ima INNER JOIN InboundMessages im ON ima.InboundMessageID=im.InboundMessageID WHERE ")
					_T("im.TicketID = Tickets.TicketID AND im.IsDeleted = 0)+")
					
					_T("(SELECT COUNT(*) FROM OutboundMessageAttachments oma INNER JOIN OutboundMessages om ON oma.OutboundMessageID=om.OutboundMessageID WHERE ")
					_T("om.TicketID = Tickets.TicketID AND om.IsDeleted = 0) as AttachCount ")  
					
					_T("FROM Tickets WHERE TicketID IN (%s) ")
					_T("ORDER BY %s "),

					sOpenTime.c_str(), sIDString.c_str(), m_sSort.c_str() );

	GetQuery().Initialize();

	// bind
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_TicketID);
	BINDCOL_TCHAR(GetQuery(), Ticket.m_Subject);
	BINDCOL_TCHAR(GetQuery(), Ticket.m_Contacts);
	BINDCOL_TIME(GetQuery(), Ticket.m_OpenTimestamp);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_PriorityID);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_TicketStateID);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_OwnerID);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_LockedBy);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_TicketBoxID);
	BINDCOL_BIT_NOLEN(GetQuery(), Ticket.m_UseTickler);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_TicketCategoryID);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_MessageCount);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_NoteCount);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_UnreadMode);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_LinkCount);
	BINDCOL_LONG_NOLEN(GetQuery(), Ticket.m_AttachCount);

	GetQuery().Execute( sQuery.c_str() );

	CEMSString sDateTime;
	std::list<CTicketListItem> ticketList;

	while ( GetQuery().Fetch() == S_OK )
	{
		ticketList.push_back(Ticket);
	}
	
	f.Format("CTicketList::GenTicketListXML - Fetched %d Tickets from DB - TicketBoxViewID: %d - Elapsed: %d", ticketList.size(),m_TBView.m_TicketBoxViewID,(int)GetCounter() );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	std::list<CTicketListItem>::iterator iterTickets;

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;
	
	for(iterTickets = ticketList.begin();
		iterTickets != ticketList.end();
		iterTickets++)
	{
		dTick1 = GetCounter();
		GetXMLGen().AddChildElem(_T("Item"));
		GetXMLGen().AddChildAttrib( _T("ID"), (*iterTickets).m_TicketID);
		
		sSubject.assign( (*iterTickets).m_SubjectLen ? (*iterTickets).m_Subject : _T("[No Subject]") );
		sSubject.EscapeHTML();
		GetXMLGen().AddChildAttrib( _T("Subject"), sSubject.c_str() );
		
		sContacts.assign( (*iterTickets).m_ContactsLen ? (*iterTickets).m_Contacts : _T("[No Contacts]"));
		sContacts.EscapeHTML();
		GetXMLGen().AddChildAttrib( _T("Contact"), sContacts.c_str() );
				
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone((*iterTickets).m_OpenTimestamp,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDateTime );
			}
			else
			{
				GetDateTimeString( (*iterTickets).m_OpenTimestamp, (*iterTickets).m_OpenTimestampLen, sDateTime );
			}
		}
		else
		{
			GetDateTimeString( (*iterTickets).m_OpenTimestamp, (*iterTickets).m_OpenTimestampLen, sDateTime );
		}
		
		GetXMLGen().AddChildAttrib( _T("Date"), sDateTime.c_str() );
		
		if( ((*iterTickets).m_UseTickler) && ((*iterTickets).m_TicketStateID == 1))
		{
			GetXMLGen().AddChildAttrib( _T("State"), 0 );
			GetXMLGen().AddChildAttrib( _T("StateName"), _T("Closed - Set To Reopen") );
			AddTicketStateHtmlColor( _T("HtmlColor"), (*iterTickets).m_TicketStateID );
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("State"), (*iterTickets).m_TicketStateID );
			AddTicketStateName( _T("StateName"), (*iterTickets).m_TicketStateID );
			AddTicketStateHtmlColor( _T("HtmlColor"), (*iterTickets).m_TicketStateID );
		}

		GetXMLGen().AddChildAttrib( _T("TicketCategory"), (*iterTickets).m_TicketCategoryID );
		AddTicketCategoryName( _T("TicketCategoryName"), (*iterTickets).m_TicketCategoryID );
		
		GetXMLGen().AddChildAttrib( _T("Priority"), (*iterTickets).m_PriorityID );
		AddPriority( _T("PriorityName"), (*iterTickets).m_PriorityID );

		if ( m_TBView.m_ViewTypeID != EMS_INBOX )
			AddAgentName( _T("Owner"), (*iterTickets).m_OwnerID, CEMSString(EMS_STRING_LIST_NONE).c_str() );

		if ( m_TBView.m_ViewTypeID != EMS_PUBLIC )
			AddTicketBoxName( _T("TicketBox"), (*iterTickets).m_TicketBoxID );

		if ( (*iterTickets).m_LockedBy == GetAgentID() )
			(*iterTickets).m_LockedBy = 0;

		GetXMLGen().AddChildAttrib( _T("LockedBy"), (*iterTickets).m_LockedBy );
		GetXMLGen().AddChildAttrib( _T("MsgCount") , (*iterTickets).m_MessageCount );
		GetXMLGen().AddChildAttrib( _T("NoteCount") , (*iterTickets).m_NoteCount );
		GetXMLGen().AddChildAttrib( _T("UnreadMode") , (*iterTickets).m_UnreadMode );
		GetXMLGen().AddChildAttrib( _T("LinkCount") , (*iterTickets).m_LinkCount );
		GetXMLGen().AddChildAttrib( _T("AttachCount") , (*iterTickets).m_AttachCount );

		dTick2 = GetCounter();
		dTot = dTot + (dTick2-dTick1);
		
		int nUnreadMsg = 0;
		int nUnreadNote = 0;
		GetUnReadMessageCountAndGetUnReadNoteCount((*iterTickets).m_TicketID, (*iterTickets).m_UnreadMode, nUnreadMsg, nUnreadNote);
		
		dTick3 = GetCounter();
		dTot = dTot + (dTick3-dTick2);

		if(nUnreadMsg > 0)
			GetXMLGen().AddChildAttrib( _T("hasunread") , 1);
		else
			GetXMLGen().AddChildAttrib( _T("hasunread") , 0);

		GetXMLGen().AddChildAttrib(_T("unreadcount"), nUnreadMsg);

		if(nUnreadNote > 0)
			GetXMLGen().AddChildAttrib( _T("hasunreadnotes") , 1);
		else
			GetXMLGen().AddChildAttrib( _T("hasunreadnotes") , 0);

		GetXMLGen().AddChildAttrib(_T("unreadnotecount"), nUnreadNote);

		GetXMLGen().AddChildElem( _T("TicketFields") );
		GetXMLGen().IntoElem();

		dTick4 = GetCounter();
		int nTicketFieldTypeID;

		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), tft.m_TicketFieldID );
		BINDCOL_LONG_NOLEN( GetQuery(), nTicketFieldTypeID );
		BINDCOL_TCHAR( GetQuery(), tft.m_DataValue );
		BINDPARAM_LONG( GetQuery(), (*iterTickets).m_TicketID );
		GetQuery().Execute( _T("SELECT tft.TicketFieldID, tf.TicketFieldTypeID, ")
							_T("CASE tf.TicketFieldTypeID WHEN 3 THEN (SELECT OptionValue COLLATE SQL_Latin1_General_CP1_CI_AI FROM ")							
							_T("TicketFieldOptions WHERE TicketFieldOptionID=CAST(tft.DataValue AS int)) ")							
							_T("ELSE tft.DataValue END ")							
							_T("FROM TicketFieldsTicket tft INNER JOIN 	TicketFields tf ON ")							
							_T("tft.TicketFieldID=tf.TicketFieldID WHERE TicketID=?") );
		
		while ( GetQuery().Fetch() == S_OK )
		{
			GetXMLGen().AddChildElem( _T("Field") );
			GetXMLGen().AddChildAttrib( _T("TID") , (*iterTickets).m_TicketID );
			GetXMLGen().AddChildAttrib( _T("TFID") , tft.m_TicketFieldID );
			GetXMLGen().AddChildAttrib( _T("TFTID") , nTicketFieldTypeID );
			GetXMLGen().AddChildAttrib( _T("DataValue") , tft.m_DataValue );	
		}

		GetXMLGen().OutOfElem();

		dTick5 = GetCounter();

		dTot = dTot + (GetCounter()-dTick3);
		f.Format("CTicketList::GenTicketListXML for TicketID: %d - TicketBoxViewID: %d - Tick2: %d - Tick3: %d - Tick4: %d - Tick5: %d - TickTot: %d", (*iterTickets).m_TicketID,m_TBView.m_TicketBoxViewID,(int)(dTick2-dTick1),(int)(dTick3-dTick2),(int)(dTick4-dTick3),(int)(dTick5-dTick4),(int)(GetCounter()-dTick1));
		//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	}

	GetXMLGen().OutOfElem();

	double lTot = GetCounter();

	f.Format("CTicketList::GenTicketListXML TicketBoxViewID: %d - Elapsed: %d - Tot: %d", m_TBView.m_TicketBoxViewID,(int)lTot,(int)dTot );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Determines which page the specified ticket is located on.
||
||				TODO - Rework with more efficient query
\*--------------------------------------------------------------------------*/
void CTicketList::FindTicketPage(void)
{
	CEMSString sTicketIDs;

	long m_lStart;
	long m_lEnd;
	long m_lElapsed;
	dca::String f;
	
	m_lStart = GetTickCount();

	if ( GetISAPIData().GetURLString( _T("Ticket"), sTicketIDs, true ) ||
	     GetISAPIData().GetURLString( _T("TicketID"), sTicketIDs, true ))
	{
		if ( GetISAPIData().m_URL.find( _T("CurrentPage") ) == GetISAPIData().m_URL.end() )
		{
			int nSrcTicket;

			if ( sTicketIDs.CDLGetNextInt( nSrcTicket ) )
			{
				CEMSString sQuery;
				CEMSString sOwnerID;
				CEMSString sTicketBoxID;
				tstring sTicketStateID;
				tstring sCount;
				int nTicketID;
				int nCount = 0;
				bool bFoundTicket = false;
				
				if ( !m_TBView.m_ShowClosedItems )
					sTicketStateID.assign( _T("AND TicketStateID > 1") );
				
				if (m_TBView.m_ViewTypeID == EMS_INBOX)
				{
					sOwnerID.Format( _T("AND OwnerID = %d"), m_TBView.m_AgentBoxID );
				}
				else if (!m_TBView.m_ShowOwnedItems)
				{
					sOwnerID.assign( _T("AND OwnerID = 0") );
				}

				if (m_TBView.m_ViewTypeID == EMS_PUBLIC)
					sTicketBoxID.Format( "AND Tickets.TicketBoxID = %d", m_TBView.m_TicketBoxID );

				if ( m_TBView.m_SortField == EMS_COLUMN_COUNT )
				{
					sCount.assign( _T(",(SELECT COUNT(*) FROM InboundMessages WHERE Tickets.TicketID = ")
						_T("InboundMessages.TicketID AND IsDeleted=0) + ")
						_T("(SELECT COUNT(*) FROM OutboundMessages WHERE Tickets.TicketID = ")
						_T("OutboundMessages.TicketID AND IsDeleted=0) as MsgCount ") );
				}
				else if ( m_TBView.m_SortField == EMS_COLUMN_NOTE_COUNT )
				{
					sCount.assign( _T(",(SELECT COUNT(*) FROM TicketNotes WHERE ")
								   _T("Tickets.TicketID = TicketNotes.TicketID) as NoteCount ") );
				}
				else if ( m_TBView.m_SortField == EMS_COLUMN_ATTACH_COUNT )
				{
					sCount.assign( _T(",(SELECT COUNT(*) FROM InboundMessageAttachments ima INNER JOIN InboundMessages im ON ima.InboundMessageID=im.InboundMessageID WHERE Tickets.TicketID = ")
						_T("im.TicketID AND im.IsDeleted=0) + ")
						_T("(SELECT COUNT(*) FROM OutboundMessageAttachments oma INNER JOIN OutboundMessages om ON oma.OutboundMessageID=om.OutboundMessageID WHERE Tickets.TicketID = ")
						_T("om.TicketID AND om.IsDeleted=0) as AttachCount ") );
				}

				GetQuery().Initialize();
				
				BINDCOL_LONG_NOLEN( GetQuery(), nTicketID );
				
				GetOrderBy();
				sQuery.Format( _T("SELECT Tickets.TicketID %s FROM Tickets ") 
							   _T("WHERE IsDeleted = 0 %s %s %s ORDER BY %s"), 
							   sCount.c_str(), sTicketStateID.c_str(), sOwnerID.c_str(), sTicketBoxID.c_str(), 
							   m_sSort.c_str() );

				GetQuery().Execute( sQuery.c_str() );

				while ( GetQuery().Fetch() == S_OK )
				{ 
					nCount++;

					if ( nTicketID == nSrcTicket )
					{
						bFoundTicket = true;
						GetQuery().Reset();
						break;
					}
				}
				
				if ( bFoundTicket )
				{
					int nCurrentPage = ( nCount / GetMaxRowsPerPage() );
					
					if ((nCurrentPage == 0) || (nCount % GetMaxRowsPerPage())) 
						nCurrentPage++;
					
					SetCurrentPage(nCurrentPage);
				}
			}
		}
	}
	
	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CTicketList::FindTicketPage TicketBoxViewID: %d - Elapsed: %d", m_TBView.m_TicketBoxViewID,m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Called when the user deletes tickets from the ticket list.
||				Supports deleting multiple tickets.
\*--------------------------------------------------------------------------*/
void CTicketList::DeleteTicket()
{
	// get the list of tickets	
	CEMSString sTicketIDs;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);
	
	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		Ticket.Delete(true);
	}
	
	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Take ownership of the selected tickets.	              
\*--------------------------------------------------------------------------*/
void CTicketList::TakeOwnership( void )
{
	// get the list of tickets	
	CEMSString sTicketIDs;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);

	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		Ticket.Reassign( GetAgentID() );
	}

	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Escalate the selected tickets.	              
\*--------------------------------------------------------------------------*/
void CTicketList::Escalate( void )
{
	// get the list of tickets	
	CEMSString sTicketIDs;
	GetISAPIData().GetXMLString( _T("selectId"), sTicketIDs);
	
	CTicket Ticket(m_ISAPIData);

	// walk through each ticket in the list
	while ( sTicketIDs.CDLGetNextInt(Ticket.m_TicketID) )
	{
		Ticket.Escalate();
	}

	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Changes the sort order of a ticketbox view	              
\*--------------------------------------------------------------------------*/
void CTicketList::ChangeSortOrder()
{	
	if(m_TBView.m_TicketBoxViewID < 1)
	{
		if(m_CustomViewID != 0)
		{
			int nAgentID = 0;
			
			if(!GetISAPIData().GetURLLong( _T("AgentID"), nAgentID, true))
				nAgentID = m_ISAPIData.m_pSession->m_AgentID;

			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_CustomViewID );
			BINDPARAM_LONG( GetQuery(), nAgentID );
			BINDCOL_LONG_NOLEN( GetQuery(), m_TBView.m_TicketBoxViewID );
			GetQuery().Execute( _T("SELECT ParentID FROM Folders WHERE (FolderID = ?) AND (AgentID = ?)"));
			if ( GetQuery().Fetch() != S_OK )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		}
		else
			return;

		if(m_TBView.m_TicketBoxViewID < 1)
			return;
	}

	// get the sort field
	tstring sSortBy;
	GetISAPIData().GetURLString( _T("SortBy"), sSortBy );
	m_TBView.m_SortField = GetSortField(sSortBy);
	
	// get the sort order
	GetISAPIData().GetURLLong( _T("SortAscending"), m_TBView.m_SortAsc );
	
	// change the sort order
	GetQuery().Initialize();
	
	BINDPARAM_LONG(GetQuery(), m_TBView.m_SortField);
	BINDPARAM_BIT(GetQuery(), m_TBView.m_SortAsc);
	BINDPARAM_LONG(GetQuery(), m_TBView.m_TicketBoxViewID);
	
	GetQuery().Execute(_T("UPDATE TicketBoxViews SET SortField = ?, SortAscending = ? WHERE TicketBoxViewID = ?"));
}

/*---------------------------------------------------------------------------\            
||  Comments:	Updates if owned or closed items should be displayed
||				as part of the listing.	              
\*--------------------------------------------------------------------------*/
void CTicketList::UpdateFilter(void)
{
	unsigned char ShowClosedItems;
	unsigned char ShowOwnedItems;

	if ( GetISAPIData().GetURLLong( _T("ShowOwnedItems"), ShowOwnedItems, true ) )
	{
		if ( ShowOwnedItems != m_TBView.m_ShowOwnedItems )
		{
			m_TBView.m_ShowOwnedItems = ShowOwnedItems;

			GetQuery().Initialize();

			BINDPARAM_TINYINT(GetQuery(), m_TBView.m_ShowOwnedItems);
			BINDPARAM_LONG(GetQuery(), m_TBView.m_TicketBoxViewID);

			GetQuery().Execute( _T("UPDATE TicketBoxViews SET ShowOwnedItems=? WHERE TicketBoxViewID=?"));

			if (GetQuery().GetRowCount() < 1)
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( _T("The TicketBox view no longer exists") ));
		}
	}

	if ( GetISAPIData().GetURLLong( _T("ShowClosedItems"), ShowClosedItems, true ) )
	{
		if ( ShowClosedItems != m_TBView.m_ShowClosedItems )
		{
			m_TBView.m_ShowClosedItems = ShowClosedItems;

			GetQuery().Initialize();
			
			BINDPARAM_TINYINT(GetQuery(), m_TBView.m_ShowClosedItems);
			BINDPARAM_LONG(GetQuery(), m_TBView.m_TicketBoxViewID);
			
			GetQuery().Execute( _T("UPDATE TicketBoxViews SET ShowClosedItems=? WHERE TicketBoxViewID=?"));
			
			if (GetQuery().GetRowCount() < 1)
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( _T("The TicketBox view no longer exists") ));
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Get the oldest items in the ticketbox and
||				moves them to the agents "My Tickets"	              
\*--------------------------------------------------------------------------*/
void CTicketList::GetOldest(void)
{
	// get the TicketBoxID
	if ( m_TBView.m_TicketBoxViewID > 0 )
	{
		m_TBView.Query( GetQuery() );
	}
	else
	{
		m_TBView.m_TicketBoxID = abs( m_TBView.m_TicketBoxViewID );
	}

	// check security
	RequireAgentRightLevel(EMS_OBJECT_TYPE_AGENT, GetAgentID(), EMS_ENUM_ACCESS);
	RequireAgentRightLevel(EMS_OBJECT_TYPE_TICKET_BOX, m_TBView.m_TicketBoxID, EMS_EDIT_ACCESS);

	CEMSString sQuery;
	int nTickets;
	int nTicketID;
	CTicket Ticket(m_ISAPIData);
	list<int> TicketIDs;
	list<int>::iterator iter;

	GetISAPIData().GetXMLLong( _T("SelectID"), nTickets );	

	if ( !GetIsAdmin() && g_ThreadPool.GetSharedObjects().m_bUseATC )
	{
		sQuery.Format( _T("SELECT TOP %d TicketID FROM Tickets INNER JOIN AgentTicketCategories ATC ON ")
					   _T("Tickets.TicketCategoryID=ATC.TicketCategoryID WHERE ATC.AgentID=%d AND TicketBoxID=? AND IsDeleted=0 ")
					   _T("AND TicketStateID > 1 AND OwnerID=0 AND LockedBy=0 ORDER BY OpenTimeStamp"), nTickets, m_ISAPIData.m_pSession->m_AgentID );
			
	}
	else
	{
		sQuery.Format( _T("SELECT TOP %d TicketID FROM Tickets WHERE TicketBoxID=? AND IsDeleted=0 ")
					   _T("AND TicketStateID > 1 AND OwnerID=0 AND LockedBy=0 ORDER BY OpenTimeStamp"), nTickets );
	}
	
	GetQuery().Initialize();

	BINDPARAM_LONG( GetQuery(), m_TBView.m_TicketBoxID );
	BINDCOL_LONG_NOLEN(GetQuery(), nTicketID);	

	GetQuery().Execute( sQuery.c_str() );
	while(GetQuery().Fetch() == S_OK)
	{
		TicketIDs.push_back(nTicketID);		
	}

	for( iter = TicketIDs.begin(); iter != TicketIDs.end(); iter++ )
	{
		Ticket.m_TicketID = *iter;
		Ticket.Reassign(m_ISAPIData.m_pSession->m_AgentID);
	}

	GetXMLGen().AddChildElem( _T("UpdateAllCounts") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a string for use in the ORDER BY clause
||				from the sort field ID in the database	              
\*--------------------------------------------------------------------------*/
void CTicketList::GetOrderBy(void)
{	
	switch (m_TBView.m_SortField)
	{
	case EMS_COLUMN_SUBJECT:
		m_sSort.Format( _T("Tickets.Subject %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;
		
	case EMS_COLUMN_CONTACTS:
		m_sSort.Format( _T("Contacts %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;

	case EMS_COLUMN_DATE:
		m_sSort.Format( _T("OpenTimeStamp %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;
	
	case EMS_COLUMN_PRIORITY:
		m_sSort.Format( _T("Tickets.PriorityID %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;

	case EMS_COLUMN_STATE:
		m_sSort.Format( _T("TicketStateID %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;

	case EMS_COLUMN_OWNER:
		m_sSort.Format( _T("OwnerID %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;

	case EMS_COLUMN_TICKETBOX:
		m_sSort.Format( _T("TicketBoxID %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;

	case EMS_COLUMN_COUNT:
		m_sSort.Format( _T("MsgCount %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;
		
	case EMS_COLUMN_ATTACH_COUNT:
		m_sSort.Format( _T("AttachCount %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;
		
	case EMS_COLUMN_NOTE_COUNT:
		m_sSort.Format( _T("NoteCount %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;

	case EMS_COLUMN_TICKETID:
		m_sSort.Format( _T("Tickets.TicketID %s"), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;

	case EMS_COLUMN_TICKETCATEGORY:
		m_sSort.Format( _T("Tickets.TicketCategoryID %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
		break;

	default:
		m_sSort.Format( _T("OpenTimeStamp %s, Tickets.TicketID %s"), 
			SORT_ORDER( m_TBView.m_SortAsc ), SORT_ORDER( m_TBView.m_SortAsc ) );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Verifies that the sort column is valid for this view type and
||				returns the name of the sort column as expected by XML
\*--------------------------------------------------------------------------*/
LPCTSTR CTicketList::GetXMLSortColumnName(void)
{
	switch (m_TBView.m_SortField)
	{
		case EMS_COLUMN_SUBJECT:	
		case EMS_COLUMN_CONTACTS:
		case EMS_COLUMN_DATE:
		case EMS_COLUMN_PRIORITY:	
		case EMS_COLUMN_STATE:
		case EMS_COLUMN_OWNER:
		case EMS_COLUMN_COUNT:
		case EMS_COLUMN_ATTACH_COUNT:
		case EMS_COLUMN_TICKETBOX:
		case EMS_COLUMN_TICKETID:
		case EMS_COLUMN_NOTE_COUNT:
		case EMS_COLUMN_TICKETCATEGORY:
			break;

		case EMS_COLUMN_TO:
			m_TBView.m_SortField = EMS_COLUMN_CONTACTS;
			break;

		// if the sort column isn't valid, reset it to the default
		default:
			m_TBView.m_SortField = EMS_COLUMN_DATE;
			m_TBView.m_SortAsc = TRUE;
	}

	return GetXMLColumnName(m_TBView.m_SortField);
}

void CTicketList::GetUnReadMessageCountAndGetUnReadNoteCount(int nTicketID, int nUnreadMode, int& nMC, int& nNC)
{
	TIMESTAMP_STRUCT ts;
	long tsLen = sizeof(TIMESTAMP_STRUCT);
	ZeroMemory(&ts, sizeof(TIMESTAMP_STRUCT));

	int nCountMsg = 0;
	int nTotalCountMsg = 0;
	int nCountNote = 0;
	int nTotalCountNote = 0;
	long lAgentID = GetISAPIData().m_pSession->m_AgentID;
	
	if( !nUnreadMode )
	{
		if(UseCutoffDate(ts))
		{
			GetQuery().Initialize();
			BINDPARAM_LONG(GetQuery(), lAgentID);
			BINDPARAM_LONG(GetQuery(), nTicketID);
			BINDPARAM_TIME(GetQuery(), ts);

			BINDPARAM_LONG(GetQuery(), nTicketID);
			BINDPARAM_TIME(GetQuery(), ts);

			BINDPARAM_LONG(GetQuery(), lAgentID);
			BINDPARAM_LONG(GetQuery(), nTicketID);
			BINDPARAM_TIME(GetQuery(), ts);

			BINDPARAM_LONG(GetQuery(), nTicketID);
			BINDPARAM_TIME(GetQuery(), ts);

			BINDCOL_LONG_NOLEN(GetQuery(), nCountMsg);
			BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountMsg);
			BINDCOL_LONG_NOLEN(GetQuery(), nCountNote);
			BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountNote);

			CEMSString sQuery;
			sQuery.Format(_T("SELECT (SELECT COUNT(*) FROM InboundMessageRead INNER JOIN InboundMessages ON InboundMessageRead.InboundMessageID = InboundMessages.InboundMessageID INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID WHERE (InboundMessageRead.AgentID = ?) AND (Tickets.TicketID = ?) AND (InboundMessages.DateReceived > ?)), ")
						_T("(SELECT COUNT(*) FROM InboundMessages WHERE (TicketID = ?) AND (InboundMessages.DateReceived > ?)  AND (InboundMessages.IsDeleted=0)), ")
						_T("(SELECT COUNT(*) FROM TicketNotesRead INNER JOIN TicketNotes ON TicketNotesRead.TicketNoteID = TicketNotes.TicketNoteID INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (TicketNotesRead.AgentID = ?) AND (Tickets.TicketID = ?) AND (TicketNotes.DateCreated > ?)), ")
						_T("(SELECT COUNT(*) FROM TicketNotes WHERE (TicketID = ?) AND (TicketNotes.DateCreated > ?))"));

			GetQuery().Execute(sQuery.c_str());
			GetQuery().Fetch();
		}
		else
		{
			GetQuery().Initialize();
			BINDPARAM_LONG(GetQuery(), lAgentID);
			BINDPARAM_LONG(GetQuery(), nTicketID);

			BINDPARAM_LONG(GetQuery(), nTicketID);

			BINDPARAM_LONG(GetQuery(), lAgentID);
			BINDPARAM_LONG(GetQuery(), nTicketID);

			BINDPARAM_LONG(GetQuery(), nTicketID);

			BINDCOL_LONG_NOLEN(GetQuery(), nCountMsg);
			BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountMsg);
			BINDCOL_LONG_NOLEN(GetQuery(), nCountNote);
			BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountNote);

			CEMSString sQuery;
			sQuery.Format(_T("SELECT (SELECT COUNT(*) FROM InboundMessageRead INNER JOIN InboundMessages ON InboundMessageRead.InboundMessageID = InboundMessages.InboundMessageID INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID WHERE (InboundMessageRead.AgentID = ?) AND (Tickets.TicketID = ?)), ")
						_T("(SELECT COUNT(*) FROM InboundMessages WHERE (TicketID = ?) AND (IsDeleted=0)), ")
						_T("(SELECT COUNT(*) FROM TicketNotesRead INNER JOIN TicketNotes ON TicketNotesRead.TicketNoteID = TicketNotes.TicketNoteID INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (TicketNotesRead.AgentID = ?) AND (Tickets.TicketID = ?)), ")
						_T("(SELECT COUNT(*) FROM TicketNotes WHERE (TicketID = ?))"));

			GetQuery().Execute(sQuery.c_str());
			GetQuery().Fetch();
		}
	}
	else
	{
		if(UseCutoffDate(ts))
		{
			GetQuery().Initialize();
			BINDPARAM_LONG(GetQuery(), nTicketID);
			BINDPARAM_TIME(GetQuery(), ts);

			BINDPARAM_LONG(GetQuery(), nTicketID);
			BINDPARAM_TIME(GetQuery(), ts);

			BINDPARAM_LONG(GetQuery(), nTicketID);
			BINDPARAM_TIME(GetQuery(), ts);

			BINDPARAM_LONG(GetQuery(), nTicketID);
			BINDPARAM_TIME(GetQuery(), ts);

			BINDCOL_LONG_NOLEN(GetQuery(), nCountMsg);
			BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountMsg);
			BINDCOL_LONG_NOLEN(GetQuery(), nCountNote);
			BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountNote);

			CEMSString sQuery;
			sQuery.Format(_T("SELECT (SELECT COUNT(*) FROM InboundMessageRead INNER JOIN InboundMessages ON InboundMessageRead.InboundMessageID = InboundMessages.InboundMessageID INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID WHERE (Tickets.TicketID = ?) AND (InboundMessages.DateReceived > ?)), ")
						_T("(SELECT COUNT(*) FROM InboundMessages WHERE (TicketID = ?) AND (InboundMessages.DateReceived > ?)  AND (InboundMessages.IsDeleted=0)), ")
						_T("(SELECT COUNT(*) FROM TicketNotesRead INNER JOIN TicketNotes ON TicketNotesRead.TicketNoteID = TicketNotes.TicketNoteID INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (Tickets.TicketID = ?) AND (TicketNotes.DateCreated > ?)), ")
						_T("(SELECT COUNT(*) FROM TicketNotes WHERE (TicketID = ?) AND (TicketNotes.DateCreated > ?))"));

			GetQuery().Execute(sQuery.c_str());
			GetQuery().Fetch();
		}
		else
		{
			GetQuery().Initialize();
			BINDPARAM_LONG(GetQuery(), nTicketID);

			BINDPARAM_LONG(GetQuery(), nTicketID);

			BINDPARAM_LONG(GetQuery(), nTicketID);

			BINDPARAM_LONG(GetQuery(), nTicketID);

			BINDCOL_LONG_NOLEN(GetQuery(), nCountMsg);
			BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountMsg);
			BINDCOL_LONG_NOLEN(GetQuery(), nCountNote);
			BINDCOL_LONG_NOLEN(GetQuery(), nTotalCountNote);

			CEMSString sQuery;
			sQuery.Format(_T("SELECT (SELECT COUNT(*) FROM InboundMessageRead INNER JOIN InboundMessages ON InboundMessageRead.InboundMessageID = InboundMessages.InboundMessageID INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID WHERE (Tickets.TicketID = ?)), ")
						_T("(SELECT COUNT(*) FROM InboundMessages WHERE (TicketID = ?) AND (IsDeleted=0)), ")
						_T("(SELECT COUNT(*) FROM TicketNotesRead INNER JOIN TicketNotes ON TicketNotesRead.TicketNoteID = TicketNotes.TicketNoteID INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (Tickets.TicketID = ?)), ")
						_T("(SELECT COUNT(*) FROM TicketNotes WHERE (TicketID = ?))"));

			GetQuery().Execute(sQuery.c_str());
			GetQuery().Fetch();
		}
	}

	nMC = nTotalCountMsg - nCountMsg;
	nNC = nTotalCountNote - nCountNote;
}

void CTicketList::SetInboundMessagesAsRead(int nTicketID, int nRead)
{
	dca::String f;
	f.Format("CTicketList::SetInboundMessagesAsRead TicketID [%d] Read/Unread [%d]", nTicketID,nRead);
	//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	TIMESTAMP_STRUCT ts;
	long tsLen = sizeof(TIMESTAMP_STRUCT);
	ZeroMemory(&ts, sizeof(TIMESTAMP_STRUCT));

	long lAgentID = GetISAPIData().m_pSession->m_AgentID;
	std::list<long> ibmList;
	long nID;

	if(UseCutoffDate(ts) && nRead)
	{
		f.Format("CTicketList::SetInboundMessagesAsRead Using CutOffDate [%0d/%0d/%0d]", ts.month,ts.day,ts.year);
		//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);
		BINDCOL_LONG_NOLEN(GetQuery(), nID);
		GetQuery().Execute(_T("SELECT InboundMessages.InboundMessageID FROM InboundMessages INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID WHERE (InboundMessages.IsDeleted = 0) AND (Tickets.TicketID = ?) AND (InboundMessages.DateReceived > ?)"));

		while(GetQuery().Fetch() == S_OK)
		{
			f.Format("CTicketList::SetInboundMessagesAsRead InboundMessageID [%d] added to list", nID);
			//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

			ibmList.push_back(nID);
		}
	}
	else
	{
		f.Format("CTicketList::SetInboundMessagesAsRead Not Using CutOffDate");
		//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDCOL_LONG_NOLEN(GetQuery(), nID);
		GetQuery().Execute(_T("SELECT InboundMessages.InboundMessageID FROM InboundMessages INNER JOIN Tickets ON InboundMessages.TicketID = Tickets.TicketID WHERE (InboundMessages.IsDeleted = 0) AND (Tickets.TicketID = ?)"));

		while(GetQuery().Fetch() == S_OK)
		{
			f.Format("CTicketList::SetInboundMessagesAsRead InboundMessageID [%d] added to list", nID);
			//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

			ibmList.push_back(nID);
		}
	}

	std::list<long>::iterator iterID;

	for(iterID = ibmList.begin();
		iterID != ibmList.end();
		iterID++)
	{
		long nMsgID = (*iterID);
		bool bExist = false;

		dca::String sCmd;
		sCmd.Format(_T("SELECT InboundMessageReadID FROM InboundMessageRead WHERE InboundMessageID = %d AND AgentID = %d"), nMsgID, lAgentID);
		
		f.Format("CTicketList::SetInboundMessagesAsRead Query [%s]", sCmd.c_str());
		//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
		GetQuery().Initialize();
		GetQuery().Execute(sCmd.c_str());
		if(GetQuery().Fetch() == S_OK)
			bExist = true;

		if(nRead)
		{
			if(!bExist)
			{
				f.Format("CTicketList::SetInboundMessagesAsRead Adding InboundMessageRead for InboundMessageID [%d] and AgentID [%d]", nMsgID,lAgentID);
				//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());		
		
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), nMsgID );
				BINDPARAM_LONG( GetQuery(), lAgentID );
				GetQuery().Execute(_T("INSERT INTO InboundMessageRead (InboundMessageID, AgentID) VALUES (?, ?)"));

				long nTicketID;
				long nTicketBoxID;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), nMsgID);
				BINDCOL_LONG_NOLEN( GetQuery(), nTicketID); 
				BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID); 
				GetQuery().Execute(_T("SELECT t.TicketID,t.TicketBoxID FROM InboundMessages im INNER JOIN Tickets t ON im.TicketID=t.TicketID WHERE (InboundMessageID = ?)"));

				if(GetQuery().Fetch() == S_OK)
				{
					CInboundMessage::UpdateTicketHistoryMsgRead(GetQuery(), lAgentID, nTicketID, 1, nMsgID, nTicketBoxID);
				}
			}
		}
		else
		{
			if(bExist)
			{
				f.Format("CTicketList::SetInboundMessagesAsRead Removing InboundMessageRead for InboundMessageID [%d] and AgentID [%d]", nMsgID,lAgentID);
				//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());		
		
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), nMsgID );
				BINDPARAM_LONG( GetQuery(), lAgentID);
				GetQuery().Execute(_T("DELETE FROM InboundMessageRead WHERE InboundMessageID = ? AND AgentID = ?"));

				long nTicketID;
				long nTicketBoxID;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), nMsgID);
				BINDCOL_LONG_NOLEN( GetQuery(), nTicketID); 
				BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID); 
				GetQuery().Execute(_T("SELECT t.TicketID,t.TicketBoxID FROM InboundMessages im INNER JOIN Tickets t ON im.TicketID=t.TicketID WHERE (InboundMessageID = ?)"));

				if(GetQuery().Fetch() == S_OK)
				{
					CInboundMessage::UpdateTicketHistoryMsgRead(GetQuery(), lAgentID, nTicketID, 0, nMsgID, nTicketBoxID);
				}
			}
		}
	}
}

void CTicketList::SetTicketNotesAsRead(int nTicketID, int nRead)
{
	TIMESTAMP_STRUCT ts;
	long tsLen = sizeof(TIMESTAMP_STRUCT);
	ZeroMemory(&ts, sizeof(TIMESTAMP_STRUCT));
	int nCount = 0;
	int nTotalCount = 0;
	long lAgentID = GetISAPIData().m_pSession->m_AgentID;
	std::list<long> ibmList;
	long nID;

	if(UseCutoffDate(ts) && nRead)
	{
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDPARAM_TIME(GetQuery(), ts);
		BINDCOL_LONG_NOLEN(GetQuery(), nID);
		GetQuery().Execute(_T("SELECT TicketNotes.TicketNoteID FROM TicketNotes INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (Tickets.TicketID = ?) AND (TicketNotes.DateCreated > ?)"));

		while(GetQuery().Fetch() == S_OK)
		{
			ibmList.push_back(nID);
		}
	}
	else
	{
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), nTicketID);
		BINDCOL_LONG_NOLEN(GetQuery(), nID);
		GetQuery().Execute(_T("SELECT TicketNotes.TicketNoteID FROM TicketNotes INNER JOIN Tickets ON TicketNotes.TicketID = Tickets.TicketID WHERE (Tickets.TicketID = ?)"));

		while(GetQuery().Fetch() == S_OK)
		{
			ibmList.push_back(nID);
		}
	}

	std::list<long>::iterator iterID;

	for(iterID = ibmList.begin();
		iterID != ibmList.end();
		iterID++)
	{
		long nMsgID = (*iterID);
		bool bExist = false;

		dca::String sCmd;
		sCmd.Format(_T("SELECT TicketNotesReadID FROM TicketNotesRead WHERE TicketNoteID = %d AND AgentID = %d"), nMsgID, lAgentID);
		GetQuery().Initialize();
		GetQuery().Execute(sCmd.c_str());
		if(GetQuery().Fetch() == S_OK)
			bExist = true;

		if(nRead)
		{
			if(!bExist)
			{
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), nMsgID );
				BINDPARAM_LONG( GetQuery(), lAgentID );
				GetQuery().Execute(_T("INSERT INTO TicketNotesRead (TicketNoteID, AgentID) VALUES (?, ?)"));
			}
		}
		else
		{
			if(bExist)
			{
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), nMsgID );
				BINDPARAM_LONG( GetQuery(), lAgentID);
				GetQuery().Execute(_T("DELETE FROM TicketNotesRead WHERE TicketNoteID = ? AND AgentID = ?"));
			}
		}
	}
}
