/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/ContactFns.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||
||                                         
||  COMMENTS:   Contact Functions
||              
\\*************************************************************************/

#pragma once

#include "ODBCQuery.h"

// struct used to hold contact name and email address
struct EmailAddr_t
{
	EmailAddr_t ()
	{
		m_sName.reserve( CONTACTS_NAME_LENGTH );
		m_sEmailAddr.reserve( PERSONALDATA_DATAVALUE_LENGTH );
	};

	void Clear()
	{
		m_sName.erase();
		m_sEmailAddr.erase();
	}

	CEMSString m_sName;
	CEMSString m_sEmailAddr;
};

// global functions
int FindContactByEmail(CODBCQuery& query, LPCTSTR szEmail);

int FindContactByPhone(CODBCQuery& query, LPCTSTR szPhone);

int CreateContactFromEmail( CODBCQuery& query, LPCTSTR szEmail, LPCTSTR szName );

int GetContactIDFromEmail( CODBCQuery& query, LPCTSTR szEmail, LPCTSTR szName, bool& bIsNew );

void BuildTicketContacts( CODBCQuery& query, int InTicketID );

void RebuildTicketContacts( CODBCQuery& query, int nTicketID );

int AddContactToTicket( CODBCQuery& query, int InContactID, int InTicketID, bool bBuildString = true);

void AddContactToOutboundMsg( CODBCQuery& query, int nContactID, int nMsgID, int nAgentID );

int CommitContact( CODBCQuery& query, int nContactID, int nDefaultEmailID );

int ProcessContactGroups( CODBCQuery& query, LPTSTR sAddrString, LPTSTR& sResultString, 
						    bool bThrowException = true, bool bAlwaysAdd = false );

int ProcessEmailAddrString( LPTSTR sAddrString, list<EmailAddr_t>& ContactList, 
						    bool bThrowException = true, bool bAlwaysAdd = false );

void FormatEmailAddrString( list<EmailAddr_t>& AddrList, TCHAR** pBuffer, long& nLength, long& nAllocated );

void FormatEmailAddrString( TCHAR** pBuffer, long& nLength, long& nAllocated );