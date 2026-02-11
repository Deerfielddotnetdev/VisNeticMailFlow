// TicketActions.h: interface for the CTicketActions class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "XMLDataClass.h"

class CTicketActions : public CXMLDataClass  
{
public:
	CTicketActions(CISAPIData& ISAPIData);
	virtual ~CTicketActions();

	// methods
	virtual int Run( CURLAction& action );
	void GetHistory( int nID );
	
protected:
	
	void GetAction( TTickets& tkt, TTicketHistory& th );

	void TicketDeleted( TTickets& tkt );
	void TicketCreated( TTickets& tkt, TTicketHistory& th );
	void TicketModified( TTickets& tkt, TTicketHistory& th );
	void TicketAgeAlerted( TTickets& tkt, TTicketHistory& th );
	void TicketEscalated( TTickets& tkt, TTicketHistory& th );
	void TicketAddMsg( TTickets& tkt, TTicketHistory& th );
	void TicketDelMsg( TTickets& tkt, TTicketHistory& th );
	void TicketArchiveMsg( TTickets& tkt, TTicketHistory& th );
	void TicketReadMsg(TTickets& tkt, TTicketHistory& th );
	void TicketAddNote( TTickets& tkt, TTicketHistory& th );
	void TicketDelNote( TTickets& tkt, TTicketHistory& th );
	void TicketReadNote(TTickets& tkt, TTicketHistory& th );
	void TicketRestored(TTickets& tkt, TTicketHistory& th );
	void TicketWasDeleted(TTickets& tkt, TTicketHistory& th );
	void MsgRevoked(TTickets& tkt, TTicketHistory& th );
	void MsgReleased(TTickets& tkt, TTicketHistory& th );
	void MsgReturned(TTickets& tkt, TTicketHistory& th );
	void TicketMerged(TTickets& tkt, TTicketHistory& th );
	void TicketSaveMsg( TTickets& tkt, TTicketHistory& th );
	void TicketLinked(TTickets& tkt, TTicketHistory& th );
	void TicketUnlinked(TTickets& tkt, TTicketHistory& th );	
	
	void GetInstigator( int AgentID, tstring& sAgentName );

};
