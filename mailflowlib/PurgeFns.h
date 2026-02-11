/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/PurgeFns.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/


#ifndef PURGE_FNS_H
#define PURGE_FNS_H

#include "ODBCQuery.h"

void PurgeTickets( CEMSString& sIDs, CODBCQuery& query, int nNumTickets, bool bKeepTicketSummary = false );
void PurgeInboundMessages( CEMSString& sIDs, CODBCQuery& query );
void PurgeOutboundMessages( CEMSString& sIDs, CODBCQuery& query );
void PurgeContacts( CEMSString& sIDs, CODBCQuery& query );
void PurgeStandardResponses( CEMSString& sIDs, CODBCQuery& query );



#endif