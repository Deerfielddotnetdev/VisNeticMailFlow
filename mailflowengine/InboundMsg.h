////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMENGINE/InboundMsg.h,v 1.1 2005/08/09 16:40:55 markm Exp $
//
//  Copyright © 2000 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// InboundMsg.h: interface for the CInboundMsg class.
//
//////////////////////////////////////////////////////////////////////

#ifndef INBOUND_MSG_H
#define INBOUND_MSG_H


class CInboundMsg : public TInboundMessages
{
public:
	CInboundMsg( CODBCQuery& query );
	virtual ~CInboundMsg();

	HRESULT GetNewMessage( tstring& sLockedMsgIDs );
	int SetRouteInfo( long TicketID, long RoutingRuleID, long ContactID, 
		              long TicketBoxID, long OwnerID, long TicketCategoryID );
	int RemoveFromInboundQueue( void );
	int Delete( bool bToWasteBasket );
	bool GetFirstMessageInTicket( int TicketID, int& TicketBoxID );

	CODBCQuery&			m_query;
	int m_MessageSourceTypeID;
	
	void AllocBuffer( wchar_t* szSource, wchar_t** szCopy );
	void FreeMatchBuffers(void);
	void AllocMatchBuffers(void);

	wchar_t* szMatchEmailTo;	// These are upper-case versions copies used for matching
	wchar_t* szMatchEmailCc;
	wchar_t* szMatchEmailFrom;
};

#endif // INBOUND_MSG_H