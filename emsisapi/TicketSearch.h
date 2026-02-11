/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketSearch.h,v 1.2 2005/11/29 21:16:28 markm Exp $
||
||
||                                         
||  COMMENTS:	Ticket Search   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class TicketSearchList : public Tickets_t
{
public:
	TicketSearchList()
		:Tickets_t()
		,m_MsgCount(0)
		,m_NoteCount(0)
		,m_LinkCount(0)
		,m_AttachCount(0)
	{
	}
	long m_MsgCount;
	long m_NoteCount;
	long m_LinkCount;
	long m_AttachCount;
};

class CTicketSearch : public CXMLDataClass  
{
public:
	CTicketSearch(CISAPIData& ISAPIData );
	virtual ~CTicketSearch();
	
	int Run(CURLAction& action);

private:

	void DecodeForm( void );
	void PreformQuery( void );
	// Mark Mohr changed this
	void format_where_clause(dca::String& where_clause);
	void ChangeTicketDefaultEmailAddr( void );
	void DeleteTicket( void );
	void TakeOwnership( void );
	void Escalate( void );
	void RestoreTicket(void);
	void UnlockTicket(void);
	int SetODBCSetting(void);
	void GetUnReadMessageCountAndGetUnReadNoteCount(int nTicketID, int& nMC, int& nNC);
	
private:
	
	// used when searching from the
	// change ticket dialog
	MsgID_t m_ChangeTicketMsg;

	// used when merging tickets
	CEMSString m_sMergeID;
	CEMSString m_sLinkID;

	// form parameters
	int m_nMaxLines;
	int m_nSortBy;
	int m_nSortAscending;

	CEMSString m_sSubject;
	CEMSString m_sSubjectBegin;
	CEMSString m_sSubjectEnd;
	CEMSString m_sSubjectWild;
	unsigned char m_ExactSubject;
	CEMSString m_sSubject2;
	CEMSString m_sSubject2Begin;
	CEMSString m_sSubject2End;
	CEMSString m_sSubject2Wild;
	unsigned char m_AndOr;

	CEMSString m_sMsgSubject;
	unsigned char m_ExactMsgSubject;

	CEMSString m_sContactEmail;
	unsigned char m_ExactContactEmail;

	CEMSString m_sContactName;
	unsigned char m_ExactContactName;

	CEMSString m_sMsgBody;
	CEMSString m_sTicketNote;
	
	int m_nSearchMsgType;
	int m_nODBCTimeout;

	CEMSString m_sTicketID;
	int m_nTicketBoxID;
	int m_nOwnerID;
	int m_nPriorityID;
	int m_nMessageID;
	
	int m_nTicketStateID;
	int m_nTicketCategoryID;
	int m_nTicketIDMatch;
	int m_nTicketLockStateID;

	int m_nTicketLink;
	int nTicketID;
	int nTicketLinkID;

	TIMESTAMP_STRUCT m_CreatedStart;
	TIMESTAMP_STRUCT m_CreatedStop;
	unsigned char m_UseCreatedDate;

	TIMESTAMP_STRUCT m_OpenedStart;
	TIMESTAMP_STRUCT m_OpenedStop;
	unsigned char m_UseOpenedDate;

	TIMESTAMP_STRUCT m_EmailStart;
	TIMESTAMP_STRUCT m_EmailStop;
	unsigned char m_UseEmailDate;

	CEMSString m_sTicketFieldID;
	int m_nTicketFieldID;
	int m_nTicketFieldTypeID;
	CEMSString m_sTicketFieldValue;
	unsigned char m_ExactTicketField;
	int m_nTicketFieldValue;

	int m_nFullText;
};
