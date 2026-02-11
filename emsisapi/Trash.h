/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Trash.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Administrative Trash Can    
||              
\\*************************************************************************/

#pragma once

#include "PagedList.h"

typedef struct DeletedItem
{
	long ID;
	TCHAR Name[256];
	long NameLen; 
	long DeletedBy;
	TIMESTAMP_STRUCT DeletedTime;
	long DeletedTimeLen;
	// NEW: support for displaying DateCreated
	TIMESTAMP_STRUCT DateCreated;
	long DateCreatedLen;

} DeletedItem;


class CTrash : public CPagedList  
{
public:
	CTrash(CISAPIData& ISAPIData);
	virtual ~CTrash() {};

	int Run(CURLAction& action);
	
private:

	LPCTSTR GetItemTypeName( int nViewID );
	LPCTSTR GetItemTypeTableName(int nViewID );

	void DecodeForm( void );
	void BuildSummaryInfo( void );
	int  CountAllDeletedItems( int nItemType );
	void CountDeletedItemsByAgent( void );
	void ListAgents( void );		
	void ListItems( void );
	void GenerateXML( DeletedItem* pItemList, int nItems );

	void Restore( void );
	void UndeleteTickets( void );
	void UndeleteStandardResponses ( void );
	void UndeleteContacts( void );
	void UndeleteInboundMsgs( void );
	void UndeleteOutboundMsgs( void );

	void Purge( void );
	void PurgeTickets( void );
	void PurgeInboundMsgs( void );
	void PurgeOutboundMsgs( void );
	void PurgeContacts( void );
	void PurgeStdResponses( void );

	void PurgeAll( void );
	void PurgeAllInboundMsgs( void );
	void PurgeAllOutboundMsgs( void );
	void PurgeAllTickets( void );
	void PurgeAllContacts( void );
	void PurgeAllStdResponses( void );
	
private:

	int m_nAgentFilter;
	int m_nItemType;

	map<int, int> m_DeletedByCounts;
};
