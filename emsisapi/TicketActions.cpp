// TicketActions.cpp: implementation of the CTicketActions class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TicketActions.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTicketActions::CTicketActions(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{

}

CTicketActions::~CTicketActions()
{

}


/*---------------------------------------------------------------------------\                     
||  Comments:	              
\*--------------------------------------------------------------------------*/
int CTicketActions::Run( CURLAction& action )
{
	int nID;

	if (GetISAPIData().GetURLLong( _T("ID"), nID ))
		GetHistory( nID );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetHistory
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::GetHistory( int nID )
{
	list<TTicketHistory> history;
	list<TTicketHistory>::iterator iter;
	TTicketHistory th;
	TTickets tkt;

	tkt.m_TicketID = nID;	
	if(	tkt.Query( GetQuery() ) != S_OK )
	{
		return;	// No ticketID in DB
	}

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nID );
	BINDCOL_TIME( GetQuery(), th.m_DateTime );
	BINDCOL_LONG( GetQuery(), th.m_AgentID );
	BINDCOL_LONG( GetQuery(), th.m_TicketActionID );
	BINDCOL_LONG( GetQuery(), th.m_ID1 );
	BINDCOL_LONG( GetQuery(), th.m_ID2 );
	BINDCOL_LONG( GetQuery(), th.m_TicketStateID );
	BINDCOL_LONG( GetQuery(), th.m_TicketBoxID );
	BINDCOL_LONG( GetQuery(), th.m_OwnerID );
	BINDCOL_LONG( GetQuery(), th.m_PriorityID );
	BINDCOL_LONG( GetQuery(), th.m_TicketCategoryID );
	BINDCOL_TCHAR( GetQuery(), th.m_DataValue );

	GetQuery().Execute( _T("SELECT DateTime,AgentID,TicketActionID,ID1,ID2,TicketStateID,TicketBoxID,OwnerID,PriorityID,TicketCategoryID,DataValue ")
						_T("FROM TicketHistory ")
						_T("WHERE TicketID=? ORDER BY TicketHistoryID DESC") );

	while( GetQuery().Fetch() == S_OK )
	{
		if ( th.m_TicketActionID != EMS_TICKETACTIONID_MOVED )
		{
            history.push_back( th );
		}
	}

	// Generate XML

	GetXMLGen().AddChildElem( _T("TicketHistory") );
	GetXMLGen().AddChildAttrib( _T("ID"), nID );
	GetXMLGen().IntoElem();
	

	if( tkt.m_IsDeleted )
	{
		TicketDeleted( tkt );
	}

	int i=0;
	for( iter = history.begin(); iter != history.end(); iter++ )
	{
		if ( i==0 && tkt.m_IsDeleted )
		{
			if ( iter->m_TicketActionID != EMS_TICKETACTIONID_DELETED )
			{
				GetAction( tkt, *iter );
			}
		}
		else
		{
			GetAction( tkt, *iter );
		}
		
		i++;
	}

	GetXMLGen().OutOfElem();
}


////////////////////////////////////////////////////////////////////////////////
// 
// TicketDeleted
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketDeleted( TTickets& tkt )
{
	CEMSString sDate;
	CEMSString sAgentName;
	CEMSString sDesc;

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;

	if(lTzBias != 1)
	{
		if(ConvertToTimeZone(tkt.m_DeletedTime,lTzBias,tsLocal))
		{
			GetDateTimeString( tsLocal, tsLocalLen, sDate );
		}
		else
		{
			GetDateTimeString( tkt.m_DeletedTime, tkt.m_DeletedTimeLen, sDate );
		}
	}
	else
	{
		GetDateTimeString( tkt.m_DeletedTime, tkt.m_DeletedTimeLen, sDate );
	}

	GetAgentName( tkt.m_DeletedBy, sAgentName );

	GetXMLGen().AddChildElem( _T("Action") );
	GetXMLGen().AddChildAttrib( _T("When"), sDate.c_str() );
	GetXMLGen().AddChildAttrib( _T("ID"), EMS_TICKETACTIONID_DELETED );

	sDesc.Format( _T("%s deleted ticket"), sAgentName.c_str() );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}	

////////////////////////////////////////////////////////////////////////////////
// 
// GetAction
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::GetAction( TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDate;
	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;

	if(lTzBias != 1)
	{
		if(ConvertToTimeZone(th.m_DateTime,lTzBias,tsLocal))
		{
			GetDateTimeString( tsLocal, tsLocalLen, sDate );
		}
		else
		{
			GetDateTimeString( th.m_DateTime, th.m_DateTimeLen, sDate );
		}
	}
	else
	{
		GetDateTimeString( th.m_DateTime, th.m_DateTimeLen, sDate );
	}

	GetXMLGen().AddChildElem( _T("Action") );
	GetXMLGen().AddChildAttrib( _T("When"), sDate.c_str() );
	GetXMLGen().AddChildAttrib( _T("ID"), th.m_TicketActionID );

	switch( th.m_TicketActionID )
	{
	case EMS_TICKETACTIONID_CREATED:
		TicketCreated( tkt, th );
		break;

	case EMS_TICKETACTIONID_MODIFIED:
		TicketModified( tkt, th );
		break;

	case EMS_TICKETACTIONID_AGE_ALERTED:
		TicketAgeAlerted( tkt, th );
		break;

	case EMS_TICKETACTIONID_ESCALATED:
		TicketEscalated( tkt, th );
		break;

	case EMS_TICKETACTIONID_ADD_MSG:
		TicketAddMsg( tkt, th );
		break;

	case EMS_TICKETACTIONID_DEL_MSG:
		TicketDelMsg( tkt, th );
		break;

	case EMS_TICKETACTIONID_DELETED:
		TicketWasDeleted( tkt, th );
		break;

	case EMS_TICKETACTIONID_ARC_MSG:
		TicketArchiveMsg( tkt, th );
		break;

	case EMS_TICKETACTIONID_READ_MSG:
		TicketReadMsg(tkt, th);
		break;

	case EMS_TICKETACTIONID_ADD_NOTE:
		TicketAddNote( tkt, th );
		break;

	case EMS_TICKETACTIONID_DEL_NOTE:
		TicketDelNote( tkt, th );
		break;

	case EMS_TICKETACTIONID_READ_NOTE:
		TicketReadNote(tkt, th);
		break;

	case EMS_TICKETACTIONID_RESTORED:
		TicketRestored(tkt, th);
		break;

	case EMS_TICKETACTIONID_REVOKE_MSG:
		MsgRevoked(tkt, th);
		break;

	case EMS_TICKETACTIONID_RELEASE_MSG:
		MsgReleased(tkt, th);
		break;

	case EMS_TICKETACTIONID_RETURN_MSG:
		MsgReturned(tkt, th);
		break;

	case EMS_TICKETACTIONID_MERGED:
		TicketMerged(tkt, th);
		break;

	case EMS_TICKETACTIONID_SAVED_MSG:
		TicketSaveMsg(tkt, th);
		break;

	case EMS_TICKETACTIONID_LINKED:
		TicketLinked(tkt, th);
		break;

	case EMS_TICKETACTIONID_UNLINKED:
		TicketUnlinked(tkt, th);
		break;
	
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// TicketCreated
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketCreated( TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	CEMSString sCreator;
	tstring sAgentName;
	tstring sTicketBox;
	tstring sPriority;
	tstring sState;
	tstring sOwner = _T("None");

	if( th.m_AgentID == 0 )
	{
		TCHAR RuleDescrip[ROUTINGRULES_RULEDESCRIP_LENGTH];	
		long RuleDescripLen;

		if( th.m_ID1 == 0 )
		{
			_tcscpy( RuleDescrip, _T("Default") );
		}
		else
		{
			GetQuery().Reset();

			BINDPARAM_LONG( GetQuery(), th.m_ID1 );
			BINDCOL_TCHAR( GetQuery(), RuleDescrip );
			
			GetQuery().Execute( _T("SELECT RuleDescrip FROM RoutingRules WHERE RoutingRuleID=?") );

			if( GetQuery().Fetch() != S_OK )
			{
				_tcscpy( RuleDescrip, _T("Unknown") );
			}
		}

		sCreator.Format( _T("Routing rule: %s"), RuleDescrip );
	}
	else
	{
		GetAgentName( th.m_AgentID, sAgentName );
		sCreator.assign( sAgentName );
	}
	
	if(th.m_TicketBoxID>0){tkt.m_TicketBoxID=th.m_TicketBoxID;}
	GetTicketBoxName( tkt.m_TicketBoxID, sTicketBox );
	
	if(th.m_OwnerID>0){tkt.m_OwnerID=th.m_OwnerID;}
	GetOwnerName( tkt.m_OwnerID, sOwner );

	if(th.m_TicketStateID>0){tkt.m_TicketStateID=th.m_TicketStateID;}
	GetTicketStateName( tkt.m_TicketStateID, sState );

	if(th.m_PriorityID>0){tkt.m_PriorityID=th.m_PriorityID;}
	GetTicketPriorityName( tkt.m_PriorityID, sPriority );

	sDesc.Format( _T("%s created ticket, TicketBox: %s, Owner: %s, State: %s, Priority: %s"),
		          sCreator.c_str(), sTicketBox.c_str(), sOwner.c_str(), sState.c_str(),
				  sPriority.c_str() );
	
	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );

}


////////////////////////////////////////////////////////////////////////////////
// 
// TicketModified
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketModified( TTickets& tkt, TTicketHistory& th )
{
	tstring sAgentName;
	CEMSString sDesc;
	CEMSString sString;
	GetInstigator( th.m_AgentID, sAgentName );

	switch( th.m_ID1 )
	{
	case 1:	// TicketBox
		if(th.m_TicketBoxID>0){tkt.m_TicketBoxID=th.m_TicketBoxID;}	
		GetTicketBoxName( tkt.m_TicketBoxID, sString );
		sDesc.Format( _T("%s moved ticket to TicketBox: %s"), 
					  sAgentName.c_str(), sString.c_str() );
		tkt.m_TicketBoxID = th.m_ID2;
		break;

	case 2:	// Owner 
		if(th.m_OwnerID>0){tkt.m_OwnerID=th.m_OwnerID;}
		if( !GetAgentName( tkt.m_OwnerID, sString ) )
		{
			sString = _T("None");
		}
		if(tkt.m_OwnerID == 0)
		{
			sString = _T("None");
		}
		sDesc.Format( _T("%s changed owner to: %s"), 
			          sAgentName.c_str(), sString.c_str() );
		tkt.m_OwnerID = th.m_ID2;
		break;

	case 3: // TicketState
		if(th.m_TicketStateID>0){tkt.m_TicketStateID=th.m_TicketStateID;}
		GetTicketStateName( tkt.m_TicketStateID, sString );
		sDesc.Format( _T("%s changed state to: %s"), 
			          sAgentName.c_str(), sString.c_str() );
		tkt.m_TicketStateID = th.m_ID2;
		break;

	case 4: // Priority
		if(th.m_PriorityID>0){tkt.m_PriorityID=th.m_PriorityID;}
		GetTicketPriorityName( tkt.m_PriorityID, sString );
		sDesc.Format( _T("%s changed priority to: %s"), 
			          sAgentName.c_str(), sString.c_str() );
		tkt.m_PriorityID = th.m_ID2;
		break;
	
	case 5: // Category
		if(th.m_TicketCategoryID>0){tkt.m_TicketCategoryID=th.m_TicketCategoryID;}
		GetTicketCategoryName( tkt.m_TicketCategoryID, sString );
		sDesc.Format( _T("%s changed Ticket Category to: %s"), 
			          sAgentName.c_str(), sString.c_str() );
		tkt.m_TicketCategoryID = th.m_ID2;
		break;

	case 6: // TicketField
		GetTicketFieldName( th.m_ID2, sString );
		sDesc.Format( _T("%s changed Ticket Field [%s] to: %s"), 
			          sAgentName.c_str(), sString.c_str(), th.m_DataValue );
		break;
	}

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

////////////////////////////////////////////////////////////////////////////////
// 
// TicketAgeAlerted
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketAgeAlerted( TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sPriority;

	if(th.m_PriorityID>0){tkt.m_PriorityID=th.m_PriorityID;}
	GetTicketPriorityName( tkt.m_PriorityID, sPriority );
	
	sDesc.Format( _T("System age-alerted ticket, priority changed to: %s"), sPriority.c_str() );

	tkt.m_PriorityID = th.m_ID1;

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

////////////////////////////////////////////////////////////////////////////////
// 
// TicketEscalated
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketEscalated( TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;	
	tstring sOwnerName = _T("None");	

	GetInstigator( th.m_AgentID, sAgentName );
	if( !GetAgentName( tkt.m_OwnerID, sOwnerName ) )
	{
		sOwnerName = _T("None");
	}

	sDesc.Format( _T("%s escalated ticket, owner changed to %s"), 
		          sAgentName.c_str(), sOwnerName.c_str() );	

	tkt.m_TicketStateID = th.m_ID1;
	tkt.m_OwnerID = th.m_ID2;

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );	
}

////////////////////////////////////////////////////////////////////////////////
// 
// TicketAddMsg
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketAddMsg( TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	tstring sMsgDesc;

	GetInstigator( th.m_AgentID, sAgentName );

	if( th.m_ID1 == 0 )
	{
		sMsgDesc.assign( _T("outbound") );
	}
	else
	{
		sMsgDesc.assign( _T("inbound") );
	}
	sDesc.Format( _T("%s added %s message %d"), sAgentName.c_str(), sMsgDesc.c_str(), th.m_ID2 );
	
	GetXMLGen().AddChildAttrib( _T("IsInbound"), th.m_ID1  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );	
}

////////////////////////////////////////////////////////////////////////////////
// 
// TicketDelMsg
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketDelMsg( TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	tstring sMsgDesc;

	GetInstigator( th.m_AgentID, sAgentName );

	if( th.m_ID1 == 0 )
	{
		sMsgDesc.assign( _T("outbound") );
	}
	else
	{
		sMsgDesc.assign( _T("inbound") );
	}
	sDesc.Format( _T("%s removed %s message %d"), sAgentName.c_str(), sMsgDesc.c_str(), th.m_ID2 );
	
	GetXMLGen().AddChildAttrib( _T("IsInbound"), th.m_ID1  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );	
}


////////////////////////////////////////////////////////////////////////////////
// 
// TicketAddNote
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketAddNote( TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	tstring sMsgDesc;

	GetInstigator( th.m_AgentID, sAgentName );

	sDesc.Format( _T("%s added note %d"), sAgentName.c_str(), th.m_ID2 );
	
	GetXMLGen().AddChildAttrib( _T("IsInbound"), th.m_ID1  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );	
}

////////////////////////////////////////////////////////////////////////////////
// 
// TicketDelNote
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketDelNote( TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	tstring sMsgDesc;

	GetInstigator( th.m_AgentID, sAgentName );

	sDesc.Format( _T("%s removed note %d"), sAgentName.c_str(), th.m_ID2 );
	
	GetXMLGen().AddChildAttrib( _T("IsInbound"), th.m_ID1  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );	
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetAgentName
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::GetInstigator( int AgentID, tstring& sAgentName )
{
	if( AgentID == 0 )
	{
		sAgentName.assign( _T("System") );
	}
	else
	{
		GetAgentName( AgentID, sAgentName );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// TicketArchiveMsg
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketArchiveMsg( TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sMsgDesc;

	if( th.m_ID1 == 0 )
	{
		sMsgDesc.assign( _T("outbound") );
	}
	else
	{
		sMsgDesc.assign( _T("inbound") );
	}

	sDesc.Format( _T("System archived %s message %d"), sMsgDesc.c_str(), th.m_ID2 );
	
	GetXMLGen().AddChildAttrib( _T("IsInbound"), th.m_ID1  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );
	GetXMLGen().AddChildAttrib( _T("ArchiveID"), th.m_AgentID );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );	
}

void CTicketActions::TicketReadMsg(TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sMsgDesc;
	tstring sAgentName;

	GetInstigator( th.m_AgentID, sAgentName );

	if( th.m_ID1 == 0 )
	{
		sMsgDesc.assign( _T("unread") );
	}
	else
	{
		sMsgDesc.assign( _T("read") );
	}

	sDesc.Format( _T("%s marked inbound message %d as %s"), sAgentName.c_str(), th.m_ID2, sMsgDesc.c_str() );
	
	GetXMLGen().AddChildAttrib( _T("wasread"), th.m_ID1  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

void CTicketActions::TicketReadNote(TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sMsgDesc;
	tstring sAgentName;

	GetInstigator( th.m_AgentID, sAgentName );

	if( th.m_ID1 == 0 )
	{
		sMsgDesc.assign( _T("unread") );
	}
	else
	{
		sMsgDesc.assign( _T("read") );
	}

	sDesc.Format( _T("%s marked note %d as %s"), sAgentName.c_str(), th.m_ID2, sMsgDesc.c_str() );
	
	GetXMLGen().AddChildAttrib( _T("wasread"), th.m_ID1  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

void CTicketActions::TicketRestored(TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sMsgDesc;
	tstring sAgentName;

	GetInstigator( th.m_AgentID, sAgentName );

	sDesc.Format( _T("%s restored ticket"), sAgentName.c_str() );
	
	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

void CTicketActions::MsgRevoked(TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	tstring sMsgDesc;

	GetInstigator( th.m_AgentID, sAgentName );

	sMsgDesc.assign( _T("outbound") );	
	sDesc.Format( _T("%s revoked %s message %d"), sAgentName.c_str(), sMsgDesc.c_str(), th.m_ID2 );
	
	GetXMLGen().AddChildAttrib( _T("IsInbound"), 0  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

void CTicketActions::MsgReleased(TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	tstring sMsgDesc;

	GetInstigator( th.m_AgentID, sAgentName );

	sMsgDesc.assign( _T("outbound") );	
	sDesc.Format( _T("%s released %s message %d"), sAgentName.c_str(), sMsgDesc.c_str(), th.m_ID2 );
	
	GetXMLGen().AddChildAttrib( _T("IsInbound"), 0  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

void CTicketActions::MsgReturned(TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	tstring sMsgDesc;

	GetInstigator( th.m_AgentID, sAgentName );

	sMsgDesc.assign( _T("outbound") );	
	sDesc.Format( _T("%s returned %s message %d"), sAgentName.c_str(), sMsgDesc.c_str(), th.m_ID2 );
	
	GetXMLGen().AddChildAttrib( _T("IsInbound"), 0  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

void CTicketActions::TicketWasDeleted(TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sMsgDesc;
	tstring sAgentName;

	GetInstigator( th.m_AgentID, sAgentName );

	sDesc.Format( _T("%s deleted ticket"), sAgentName.c_str() );
	
	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

void CTicketActions::TicketMerged(TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	
	GetInstigator( th.m_AgentID, sAgentName );

	sDesc.Format( _T("%s merged Ticket %d"), sAgentName.c_str(), th.m_ID1 );
	
	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

void CTicketActions::TicketLinked(TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	
	GetInstigator( th.m_AgentID, sAgentName );

	sDesc.Format( _T("%s linked ticket to Ticket Link %s"), sAgentName.c_str(), th.m_DataValue );
	
	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

void CTicketActions::TicketUnlinked(TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	
	GetInstigator( th.m_AgentID, sAgentName );

	sDesc.Format( _T("%s unlinked ticket from Ticket Link %s"), sAgentName.c_str(), th.m_DataValue );
	
	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );
}

////////////////////////////////////////////////////////////////////////////////
// 
// TicketSaveMsg
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketActions::TicketSaveMsg( TTickets& tkt, TTicketHistory& th )
{
	CEMSString sDesc;
	tstring sAgentName;
	tstring sMsgDesc;

	GetInstigator( th.m_AgentID, sAgentName );

	if( th.m_ID1 == 0 )
	{
		sMsgDesc.assign( _T("outbound") );
	}
	else
	{
		sMsgDesc.assign( _T("inbound") );
	}
	sDesc.Format( _T("%s saved %s message %d"), sAgentName.c_str(), sMsgDesc.c_str(), th.m_ID2 );
	
	GetXMLGen().AddChildAttrib( _T("IsInbound"), th.m_ID1  );
	GetXMLGen().AddChildAttrib( _T("MsgID"), th.m_ID2 );

	GetXMLGen().SetChildData( sDesc.c_str(), TRUE );	
}

