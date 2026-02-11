// OutboundMsg.h: interface for the COutboundMsg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OUTBOUNDMSG_H__FCF37771_515B_44DB_91F0_06E1577F0973__INCLUDED_)
#define AFX_OUTBOUNDMSG_H__FCF37771_515B_44DB_91F0_06E1577F0973__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "InboundMsg.h"
#include <CkHtmlToText.h>
#include <CkGlobal.h>

class COutboundMsg : public TOutboundMessages  
{
public:
	COutboundMsg( CODBCQuery& query );
	virtual ~COutboundMsg();

	int PutInOutboundQueue( void );
	int UnDeleteMsg( void );

	void Forward( CInboundMsg& msg, vector<CForwardToAddr> fta, vector<CForwardCCAddr> fca, bool bInTicket = true, bool bIsRemoteReply = false, bool bAllowRemoteReply = false, bool bQuoteOriginal = false, unsigned int nOmitTracking = 0 );
	int ForwardAttachments( CInboundMsg& msg );

	bool AutoReply( int StdRespID, int AutoReplyQuoteMsg, CInboundMsg& msg, TCHAR* szAutoReplyFrom, bool bInTicket = true, unsigned int nOmitTracking = 0 );
	bool AutoMessage( int StdRespID, CInboundMsg& msg, TCHAR* szAutoReplyFrom, int m_CreateNewTicket, unsigned int nOmitTracking = 0 );
	bool InsertStdRespBody( int StdRespID, int AutoReplyQuoteMsg, CInboundMsg& msg );
	int AddStdRespAttachments( int StdRespID );
	void CheckForVariables( void );

protected:
	void FindOrInsert( tstring::size_type& pos, wchar_t* szTag, tstring& sBody, bool bAppend = false );


	CODBCQuery&			m_query;
	CkHtmlToText		h2t;

};

#endif // !defined(AFX_OUTBOUNDMSG_H__FCF37771_515B_44DB_91F0_06E1577F0973__INCLUDED_)
