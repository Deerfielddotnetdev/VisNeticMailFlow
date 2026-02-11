/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketHistory.h,v 1.2 2005/11/29 21:16:28 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

// class to store query results
class CTicketHistoryMsg 
{
public:
	CTicketHistoryMsg() { ZeroMemory( this, sizeof(CTicketHistoryMsg));}
	
	bool m_ReplyToIDIsInbound;
	bool m_IsInbound;

	int m_ID;
	int m_Prority;
	int m_AgentID;
	int m_ReplyToMsgID;
	int m_AttachmentCount;
	int m_ContactID;
	int m_OutboundMsgTypeID;
	int m_OutboundMsgStateID;
	
	TIMESTAMP_STRUCT m_Date;
	long m_DateLen;
	
	TCHAR m_EmailAddr[INBOUNDMESSAGES_EMAILFROM_LENGTH];
	long m_EmailAddrLen;

	TCHAR m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH];
	long m_SubjectLen;
};

// map typedef
typedef map<MsgID_t, CTicketHistoryMsg> MsgMap;
typedef multimap<MsgID_t, MsgID_t> ThreadingMap;

class CTicketHistory : public CXMLDataClass
{
public:
	CTicketHistory(CISAPIData& ISAPIData);
	virtual ~CTicketHistory() {};
	
	virtual int Run( CURLAction& action );
	
protected:
	void ListTicketHistory( int nTicketID );
	void FindChildMessages( MsgID_t Msg );
	void GenMsgXML( CTicketHistoryMsg& msg );
	void GetTicketInfo( int nTicketID );
	void ListTicketNotes( int nTicketID );	
	int SetTicketNoteBeenRead(long lTicketNote);
	bool UseCutoffDate(CTicketHistoryMsg& msg);
	bool UseCutoffDate(TTicketNotes& notes);
	void GetUnReadNoteCount(int nTicketID, int& nNC, int& nUNC);

protected:
	int m_nMaxMsgThreads;
	int m_nMaxNoteThreads;
	unsigned char m_bShowTicketDetails;
	unsigned char m_bDetailedThreading;
	int m_nIsDeleted;
	int nDateTime;
	tstring sDateTime;

	MsgMap m_MsgMap;
	ThreadingMap m_ThreadingMap;
	list<MsgID_t> m_MsgList;
};

