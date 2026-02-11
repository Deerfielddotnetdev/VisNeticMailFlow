/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketList.h,v 1.2.2.1 2006/01/25 17:59:20 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "PagedList.h"
#include "listclasses.h"
#include "TBVInfo.h"

class CTicketListItem
{
public:
	
	CTicketListItem() { ZeroMemory( this, sizeof(CTicketListItem));}
	CTicketListItem(const CTicketListItem& tli)
	{
		m_TicketID = tli.m_TicketID;
		m_TicketBoxID = tli.m_TicketBoxID;
		m_PriorityID = tli.m_PriorityID;
		m_TicketStateID = tli.m_TicketStateID;
		m_TicketCategoryID = tli.m_TicketCategoryID;
		m_OwnerID = tli.m_OwnerID;
		m_LockedBy = tli.m_LockedBy;
		m_MessageCount = tli.m_MessageCount;
		m_NoteCount = tli.m_NoteCount;
		m_OpenTimestamp = tli.m_OpenTimestamp;
		m_OpenTimestampLen = tli.m_OpenTimestampLen;
		strncpy(m_Subject, tli.m_Subject, TICKETS_SUBJECT_LENGTH);
		m_SubjectLen = tli.m_SubjectLen;
		strncpy(m_Contacts, tli.m_Contacts, TICKETS_CONTACTS_LENGTH);
		m_ContactsLen = tli.m_ContactsLen;
		m_UseTickler = tli.m_UseTickler;
		m_UnreadMode = tli.m_UnreadMode;
		m_LinkCount = tli.m_LinkCount;
		m_AttachCount = tli.m_AttachCount;
	}
	CTicketListItem& operator=(const CTicketListItem& tli)
	{
		m_TicketID = tli.m_TicketID;
		m_TicketBoxID = tli.m_TicketBoxID;
		m_PriorityID = tli.m_PriorityID;
		m_TicketStateID = tli.m_TicketStateID;
		m_TicketCategoryID = tli.m_TicketCategoryID;
		m_OwnerID = tli.m_OwnerID;
		m_LockedBy = tli.m_LockedBy;
		m_MessageCount = tli.m_MessageCount;
		m_NoteCount = tli.m_NoteCount;
		m_OpenTimestamp = tli.m_OpenTimestamp;
		m_OpenTimestampLen = tli.m_OpenTimestampLen;
		strncpy(m_Subject, tli.m_Subject, TICKETS_SUBJECT_LENGTH);
		m_SubjectLen = tli.m_SubjectLen;
		strncpy(m_Contacts, tli.m_Contacts, TICKETS_CONTACTS_LENGTH);
		m_ContactsLen = tli.m_ContactsLen;
		m_UseTickler = tli.m_UseTickler;
		m_UnreadMode = tli.m_UnreadMode;
		m_LinkCount = tli.m_LinkCount;
		m_AttachCount = tli.m_AttachCount;

		return *this;
	}
	
	int m_TicketID;
	int m_TicketBoxID;
	int m_PriorityID;
	int m_TicketStateID;
	int m_TicketCategoryID;
	int m_OwnerID;
	int m_LockedBy;
	int m_MessageCount;
	int m_NoteCount;
	int m_UseTickler;
	int m_UnreadMode;
	int m_LinkCount;
	int m_AttachCount;

	TIMESTAMP_STRUCT m_OpenTimestamp;
	long m_OpenTimestampLen;

	TCHAR m_Subject[TICKETS_SUBJECT_LENGTH];
	long  m_SubjectLen;
	TCHAR m_Contacts[TICKETS_CONTACTS_LENGTH];
	long  m_ContactsLen;
};

class CTicketList : public CPagedList  
{
public:
	CTicketList(CISAPIData& ISAPIData);
	virtual ~CTicketList();

	virtual int Run( CURLAction& action );

private:	
	CTBVInfo m_TBView;
	TTicketFields tf;
	TTicketFieldsTicket tft;
	int* m_TicketIDArray;
	int m_CustomViewID;

private:
	inline void ListTicketBox(void);
	inline void ListTicketBoxView(CURLAction& action);
	inline void List(void);
	inline void ListQuery(void);
	inline void GenTicketListXML( int* pArray, int nTickets );
	inline void UpdateFilter(void);
	//int GetUnReadMessageCount(int nTicketID);
	//int GetUnReadNoteCount(int nTicketID);
	void GetUnReadMessageCountAndGetUnReadNoteCount(int nTicketID, int nUnreadMode, int& nMC, int& nNC);
	void SetInboundMessagesAsRead(int nTicketID, int nRead);
	void SetTicketNotesAsRead(int nTicketID, int nRead);

	void Query(void);
	
	void FindTicketPage(void);
	void DeleteTicket(void);
	void TakeOwnership(void);
	void Escalate(void);
	void ChangeSortOrder(void);
	void GetOldest(void);

	void GetOrderBy(void);
	LPCTSTR GetXMLSortColumnName(void);

	CEMSString m_sSort;

	double PCFreq;
	__int64 CounterStart;

	void StartCounter()
	{
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		PCFreq = double(li.QuadPart)/1000.0;
		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
	}
	
	double GetCounter()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return double(li.QuadPart-CounterStart)/PCFreq;
	}

};


