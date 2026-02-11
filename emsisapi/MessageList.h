/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/MessageList.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "PagedList.h"
#include "listclasses.h"
#include "TBVInfo.h"

class CMessageListItem
{
public:
	
	CMessageListItem() { ZeroMemory( this, sizeof(CMessageListItem));}
	
	int m_MsgStateID;
	int m_MessageID;
	int m_TicketID;
	int m_Prority;
	int m_AttachmentCount;
	int m_AgentID;
	int m_TicketBoxID;
	int m_TicketNoteCount;
	
	TIMESTAMP_STRUCT m_EmailDateTime;
	long m_EmailDateTimeLen;
	
	TCHAR m_Subject[OUTBOUNDMESSAGES_SUBJECT_LENGTH];
	long m_SubjectLen;
	TCHAR m_EmailAddr[OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH];
	TCHAR m_EmailAddrFrom[OUTBOUNDMESSAGES_EMAILFROM_LENGTH];	
};

class CMessageList : public CPagedList  
{
public:
	int Run(CURLAction& action);
	CMessageList(CISAPIData& ISAPIData);
	virtual ~CMessageList() {};

private:
	CTBVInfo m_TBView;

private:
	
	int List(void);
	int ListQueue( int nInOut );
	void PrepareOutboundQuery(CEMSString& sQuery);
	void FormatQueryString(CEMSString& sQuery);
	void GenMessageListXML(CMessageListItem* pArray);
	
	int ChangeSortOrder(void);
	int DeleteMessage(void);
	int RevokeMessage(void);
	int ReleaseMessage(void);
	int ReturnMessage(void);
	int RemoveMessage( int nInOut );
	int RevokeFromQueue(void);
	int PutInQueue(void);
	int DeleteOrphan(void);
	int DeleteOutboundOrphan(void);
	int DeleteAllOrphans(void);
	int DeleteAllOutboundOrphans(void);

	LPCTSTR GetOrderBy(void);
	LPCTSTR GetXMLSortColumnName(void);
};
