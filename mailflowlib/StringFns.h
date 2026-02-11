/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/StringFns.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   String Functions
||              
\\*************************************************************************/

#ifndef STRING_FNS_H
#define STRING_FNS_H

#include "EMSString.h"

void PutStringProperty( TCHAR* wcsBuffer, long wcsBufLen, TCHAR** ppBuffer, long* pAllocated );
void PutStringProperty( tstring& string, TCHAR** ppBuffer,  long* pAllocated );

// The following functions are only available for UNICODE compilation
#ifdef _UNICODE
void PutStringPropertyW( wchar_t* wcsString, wchar_t** ppBuffer,  long* pAllocated );
void PutStringProperty( BSTR newVal, wchar_t** ppBuffer, long* pAllocated );
#endif // _UNICODE 

// Ticket/MsgIDs and the Subject line
void RemoveTicketIDFromSubject( tstring& strSubject );
void PutTicketIDInSubject( int TicketID, int MsgID, TCHAR* szSubject, int nTicketTracking );
bool GetTicketIDFromSubject( TCHAR* szSubject, int& TicketID, int& MsgID );

// Ticket/MsgIDs and the Body
void RemoveTicketIDFromBody( tstring& strBody );
bool GetTicketIDFromBody( TCHAR* szBody, int& TicketID, int& MsgID );

// Internet address <-> strings
unsigned int InetAddr( tstring& IP );
void InetNToA( unsigned int n, TCHAR* szIP );

#endif
