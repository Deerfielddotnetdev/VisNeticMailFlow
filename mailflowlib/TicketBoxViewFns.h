/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/TicketBoxViewFns.h,v 1.2 2005/11/29 21:30:04 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "QueryClasses.h"

void TBVAddAgent(CODBCQuery &query, int nAgentID, int nAgentBoxID);

void TBVRemoveAgent(CODBCQuery &query, int nAgentID, int nAgentBoxID);

int  DeleteAllAgentViews(CODBCQuery &query, int nAgentID);

int  DeleteAllTicketBoxViews(CODBCQuery &query, int nTicketBoxID);

int	 GetMsgCount( CODBCQuery &query, int nTypeID, int nAgentBoxID );

int  GetTicketCount( CODBCQuery &query, int nTypeID, int nTicketBoxID, int nAgentBoxID, 
					 unsigned char bOwnedItems, unsigned char bClosedItems, int nFolderID = 0 );

int  GetSortField(tstring& sSortField);

TCHAR* GetXMLColumnName(int nSortField);

