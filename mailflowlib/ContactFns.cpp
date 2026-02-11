 /***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/ContactFns.cpp,v 1.2 2005/11/29 21:30:03 markm Exp $
||
||
||                                         
||  COMMENTS:   Contact Functions
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ContactFns.h"
#include "DateFns.h"
#include "StringFns.h"
#include "QueryClasses.h"
#include "EMSMutex.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a ContactID from an email address. If necessary
||				Creates a new contact with a name of szName and an email 
||				address of szEmail.
||
||				Return: ContactID of created contact, 0 if an error occured	              
\*--------------------------------------------------------------------------*/
int GetContactIDFromEmail( CODBCQuery& query, LPCTSTR szEmail, LPCTSTR szName, bool& bIsNew )
{
	CCreateContactMutex lock;
	bIsNew = false;

	if( lock.AcquireLock( 1000 ) == false )
	{
		// error - could not obtain mutex
		return 0;
	}
	
	int nContactID = 0;
	if ((nContactID = FindContactByEmail(query, szEmail)) != 0)
		return nContactID;

	// we need to create the contact
	bIsNew = true;
	
	return CreateContactFromEmail( query, szEmail, szName );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Creates a contact given an email address and name.				             
\*--------------------------------------------------------------------------*/
int CreateContactFromEmail( CODBCQuery& query, LPCTSTR szEmail, LPCTSTR szName )
{
	TContacts Contact;

	GetTimeStamp(Contact.m_DateCreated);
	Contact.m_DateCreatedLen = sizeof(Contact.m_DateCreated);
	Contact.m_IsDeleted = EMS_DELETE_OPTION_DELETE_PERMANENTLY;
	
	// copy the name
	_tcsncpy(Contact.m_Name, szName, CONTACTS_NAME_LENGTH - 1);

	// create the deleted contact
	Contact.Insert(query);
	
	
	// add the default email address
	TPersonalData PersonalData;
	PersonalData.m_ContactID = Contact.m_ContactID;
	PersonalData.m_PersonalDataTypeID = EMS_PERSONAL_DATA_EMAIL;
	_tcsncpy(PersonalData.m_DataValue, szEmail, PERSONALDATA_DATAVALUE_LENGTH - 1);
	
	PersonalData.Insert( query );
	
	
	// commit the contact
	if (CommitContact( query, Contact.m_ContactID, PersonalData.m_PersonalDataID ) != 1)
		return 0;
	
	
	// return the ID of the created
	return Contact.m_ContactID;
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Searches for a contact that has an email address
||				of szEmail.  
||
||				Return: ContactID of the first match, 0 if no match is found	              
\*--------------------------------------------------------------------------*/
int FindContactByEmail(CODBCQuery& query, LPCTSTR szEmail)
{
	// bail out if the string is empty
	if ( _tcslen( szEmail ) == 0 )
		return 0;

	int nContactID;

	query.Initialize();
	BINDPARAM_TCHAR( query, (TCHAR*) szEmail );
	BINDCOL_LONG_NOLEN( query, nContactID );
	
	query.Execute( _T("SELECT PersonalData.ContactID ")
		_T("FROM PersonalData INNER JOIN Contacts ON PersonalData.ContactID = Contacts.ContactID ")
		_T("WHERE PersonalData.PersonalDataTypeID=1 ")
		_T("AND Contacts.IsDeleted = 0 ")
		_T("AND PersonalData.DataValue = ?"));
	
	if ( query.Fetch() != S_OK )
	{
		return 0;
	}
		
	return nContactID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Find contact by phone number	              
\*--------------------------------------------------------------------------*/
int FindContactByPhone(CODBCQuery& query, LPCTSTR szPhone)
{
	// bail out if the string is empty
	if ( _tcslen( szPhone ) == 0 )
		return 0;

	dca::String sPhone(szPhone);
	
	int nContactID = 0;

	std::string sTemp = "LIKE '";
	sTemp.append(sPhone.c_str());
	sTemp.append("%'");

	std::string sTemp2 = "'%phone%'";

	CEMSString sSql;
	sSql.Format( _T("SELECT TOP 1 pd.ContactID FROM PersonalData pd ")
				_T("INNER JOIN PersonalDataTypes pdt ON pd.PersonalDataTypeID=pdt.PersonalDataTypeID ")
				_T("WHERE (REPLACE( REPLACE( REPLACE( REPLACE( REPLACE( pd.datavalue, '-', '' ) ")
				_T(", '.', '' ), ' ', '' ), '(', ''), ')', '')  %s ) AND pdt.TypeName LIKE %s ORDER BY pd.ContactID"),sTemp.c_str(),sTemp2.c_str() );
	
	query.Initialize();
	BINDCOL_LONG_NOLEN( query, nContactID );
	query.Execute( sSql.c_str() );
	
	if ( query.Fetch() != S_OK )
	{
		return 0;
	}
		
	return nContactID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Sets the DefaultEmailAddressID and removes the IsDeleted
||				flag from a contact record.	              
\*--------------------------------------------------------------------------*/
int CommitContact( CODBCQuery& query, int nContactID, int nDefaultEmailID )
{
	query.Initialize();

	BINDPARAM_LONG( query, nDefaultEmailID );
	BINDPARAM_LONG( query, nContactID );
 
	query.Execute( _T("UPDATE Contacts ")
				   _T("SET DefaultEmailAddressID = ?, IsDeleted = 0 ")
				   _T("WHERE ContactID = ?"));

	return query.GetRowCount();
}


////////////////////////////////////////////////////////////////////////////////
// 
// Adds a Contact to a Ticket
// 
////////////////////////////////////////////////////////////////////////////////
int AddContactToTicket( CODBCQuery& query, int nContactID, int nTicketID, bool bRebuldString )
{
	unsigned int TicketContactID;
	long TicketContactIDLen;

	CAddContactToTicketMutex lock( nTicketID );

	if( lock.AcquireLock( 1000 ) == false )
	{
		// Error - could not obtain mutex
		return -1;
	}

	// check if the TicketContacts record exists
	query.Initialize();
	BINDPARAM_LONG( query, nTicketID );
	BINDPARAM_LONG( query, nContactID );
	BINDCOL_LONG( query, TicketContactID );
	query.Execute( _T("SELECT TicketContactID from TicketContacts ")
		           _T("WHERE TicketID=? AND ContactID=?") );

	if( query.Fetch() == S_OK )
	{
		// Contact is already part of the ticket
		return 0;
	}

	// Add the TicketContacts record
	query.Reset();
	BINDPARAM_LONG( query, nTicketID );
	BINDPARAM_LONG( query, nContactID );
	query.Execute( _T("INSERT INTO TicketContacts ")
				   _T("(TicketID,ContactID) ")
				   _T("VALUES (?,?)") );	

	lock.ReleaseLock();
	
	if ( bRebuldString )
	{
		// Rebuild the ticket contacts field.
		BuildTicketContacts( query, nTicketID );
	}
	
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Updates the specified ticket contacts to reflect
||				the contacts of the messages which it contains.	              
\*--------------------------------------------------------------------------*/
void RebuildTicketContacts( CODBCQuery& query, int nTicketID )
{
	bool bRebuildString = false;
	
	int nContactID;
	set<int> DBIDs;
	
	// don't let another thread add a contact to a ticket...
	CAddContactToTicketMutex lock( nTicketID );
	
	if( lock.AcquireLock( 1000 ) == false )
		return;
	
	// query the database for the contacts currently
	// associated with the ticket
	query.Initialize();
	
	BINDCOL_LONG_NOLEN( query, nContactID );
	BINDPARAM_LONG( query, nTicketID );
	
	query.Execute( _T("SELECT ContactID FROM TicketContacts WHERE TicketID=?") );
	
	while( query.Fetch() == S_OK )
	{
		DBIDs.insert( nContactID );
	}
	
	// build a list of contacts for the ticket from its messages 
	set<int> ActualIDs;
	
	query.Initialize();
	
	BINDCOL_LONG_NOLEN( query, nContactID );
	BINDPARAM_LONG( query, nTicketID );
	BINDPARAM_LONG( query, nTicketID );
	
	query.Execute( _T("SELECT DISTINCT ContactID FROM InboundMessages ") 
		_T("WHERE TicketID=? AND IsDeleted=0 ")
		_T("UNION ")
		_T("SELECT DISTINCT OutboundMessageContacts.ContactID ") 
		_T("FROM OutboundMessages ")
		_T("INNER JOIN OutboundMessageContacts ON ")
		_T("OutboundMessages.OutboundMessageID = OutboundMessageContacts.OutboundMessageID ")
		_T("WHERE OutboundMessages.TicketID=? AND IsDeleted=0 ") );
	
	while( query.Fetch() == S_OK )
	{
		ActualIDs.insert( nContactID );
	}
	
	// create contacts which aren't in the database
	set<int>::iterator iter;
	int nTemp = 0;
	
	for ( iter = ActualIDs.begin(); iter != ActualIDs.end(); iter++ )
	{
		if ( DBIDs.find( *iter) == DBIDs.end() )
		{
			query.Initialize();
			nTemp = *iter;
			
			BINDPARAM_LONG( query, nTicketID );
			BINDPARAM_LONG( query, nTemp );
			
			query.Execute( _T("INSERT INTO TicketContacts (TicketID, ContactID) VALUES(?,?)") );

			bRebuildString = true;
		}
	}
	
	// remove contacts from the database that aren't in actualIDs
	for ( iter = DBIDs.begin(); iter != DBIDs.end(); iter++ )
	{
		if ( ActualIDs.find( *iter) == ActualIDs.end() )
		{
			query.Initialize();
			nTemp = *iter;
			
			BINDPARAM_LONG( query, nTicketID );
			BINDPARAM_LONG( query, nTemp );
			
			query.Execute( _T("DELETE FROM TicketContacts WHERE TicketID=? AND ContactID=?") );
			
			if ( query.GetRowCount() != 1)
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

			bRebuildString = true;
		}
	}
	
	if ( bRebuildString )
		BuildTicketContacts( query, nTicketID );
}

////////////////////////////////////////////////////////////////////////////////
// 
// AddNameToContactsString
// 
////////////////////////////////////////////////////////////////////////////////
static void AddNameToContactsString( TCHAR* szName, tstring& sContactNames )
{
	if( sContactNames.size() == 0 )
	{
		sContactNames = szName;			// No comma for first contact
	}
	else if ( sContactNames.size() < TICKETS_CONTACTS_LENGTH )
	{
		sContactNames += _T(", ");			// additional contacts
		sContactNames += szName;
		// truncat the string if too long
		if ( sContactNames.size() > TICKETS_CONTACTS_LENGTH-1 )
		{
			sContactNames.resize(TICKETS_CONTACTS_LENGTH-1);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// BuildTicketContacts
// 
////////////////////////////////////////////////////////////////////////////////
void BuildTicketContacts( CODBCQuery& query, int nTicketID )
{
	CEMSString sContactNames;
	TCHAR Name[CONTACTS_NAME_LENGTH];
	long NameLen;
	TCHAR Email[PERSONALDATA_DATAVALUE_LENGTH];
	long EmailLen;

	query.Initialize();
	BINDPARAM_LONG( query, nTicketID );
	BINDCOL_TCHAR( query, Name );
	BINDCOL_TCHAR( query, Email );
	
	query.Execute( _T("SELECT Contacts.Name, ")
		_T("(SELECT DataValue FROM PersonalData WHERE PersonalDataID = Contacts.DefaultEmailAddressID ) ")
		_T("FROM TicketContacts INNER JOIN Contacts ")
		_T("ON TicketContacts.ContactID = Contacts.ContactID ")
		_T("WHERE TicketID=? AND Contacts.IsDeleted=0 ")
		_T("ORDER BY TicketContactID") );
	
	while ( query.Fetch() == S_OK )
	{
		if ( NameLen && NameLen != SQL_NULL_DATA )
		{
			AddNameToContactsString( Name, sContactNames );
		}
		else if ( EmailLen && EmailLen != SQL_NULL_DATA )
		{
			AddNameToContactsString( Email, sContactNames );
		}
	}
	
	// handle the case where there are no contacts
	if ( sContactNames.length() == 0 )
		sContactNames.assign(_T("[No Contacts]"));
	
	query.Reset();
	BINDPARAM_TCHAR_STRING( query, sContactNames );
	BINDPARAM_LONG( query, nTicketID );
	query.Execute( _T("UPDATE Tickets SET Contacts=? WHERE TicketID=?") );
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Adds a contact to an OutboundMessage	              
\*--------------------------------------------------------------------------*/
void AddContactToOutboundMsg( CODBCQuery& query, int nContactID, int nMsgID, int nAgentID )
{
	unsigned int OutboundMsgContactID;
	unsigned int AgentContactID;	

	// check if the OutboundMsg record exists
	query.Initialize();
	BINDPARAM_LONG( query, nMsgID );
	BINDPARAM_LONG( query, nContactID );
	BINDCOL_LONG_NOLEN( query, OutboundMsgContactID );
	query.Execute( _T("SELECT OutboundMessageContactID from OutboundMessageContacts  ")
		           _T("WHERE OutboundMessageID=? AND ContactID=?") );
	
	if( query.Fetch() != S_OK )
	{
		// add the OutboundMessageContacts record
		query.Reset();
		BINDPARAM_LONG( query, nMsgID );
		BINDPARAM_LONG( query, nContactID );
		query.Execute( _T("INSERT INTO OutboundMessageContacts ")
			_T("(OutboundMessageID,ContactID) ")
			_T("VALUES ")
			_T("(?,?)") );	
	}
	
	

	// check if the AgentContacts record exists
	query.Initialize();
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nContactID );
	BINDCOL_LONG_NOLEN( query, AgentContactID );
	query.Execute( _T("SELECT ContactID from AgentContacts  ")
		           _T("WHERE AgentID=? AND ContactID=?") );
	
	if( query.Fetch() == S_OK )
	{
		// Contact is already part of the message
		return;
	}
	
	// add the AgentContacts record
	query.Reset();
	BINDPARAM_LONG( query, nAgentID );
	BINDPARAM_LONG( query, nContactID );
	query.Execute( _T("INSERT INTO AgentContacts ")
		_T("(AgentID,ContactID) ")
		_T("VALUES ")
		_T("(?,?)") );

	return;
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Verifies an email address and adds it to the list	              
\*--------------------------------------------------------------------------*/
bool VerifyEmailAddr( EmailAddr_t& EmailAddr, list<EmailAddr_t>& EmailAddrList, bool bThrowException, bool bAlwaysAdd )
{
	// trim off any white space
	EmailAddr.m_sEmailAddr.TrimWhiteSpace();
	EmailAddr.m_sName.TrimWhiteSpace();

	// make sure the email address isn't empty
	if ( EmailAddr.m_sEmailAddr.length() < 1 )
		return false;

	if ( !bAlwaysAdd )
	{
		if ( EmailAddr.m_sEmailAddr.length() > (PERSONALDATA_DATAVALUE_LENGTH - 1) )
		{
			if( bThrowException )
			{
				THROW_EMS_EXCEPTION_NOLOG( E_ParameterTooLarge, _T("Email address exceeded maximum length") );
			}
			else
			{
				return false;
			}
		}
		
		if ( EmailAddr.m_sName.length() > (CONTACTS_NAME_LENGTH - 1))
		{
			if( bThrowException )
			{
				THROW_EMS_EXCEPTION_NOLOG( E_ParameterTooLarge, _T("Email address real name exceeded maximum length") );
			}
			else
			{
				return false;
			}
		}
		
		// is the email address valid?
		if ( !EmailAddr.m_sEmailAddr.ValidateEmailAddr() )
		{
			if( bThrowException )
			{
				CEMSString sError;
				sError.Format( _T("The email address (%s) is invalid"), EmailAddr.m_sEmailAddr.c_str() );
				THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, sError );
			}
			else
			{
				return false;
			}
		}
	}
	
	// add this contact to the list
	EmailAddrList.push_back(EmailAddr);

	return true;
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Parses an email address string and creates a list of 
||				ContactNameAddr structs.  szString MUST be NULL terminated.
||				Returns the number or email addresses parsed.
||
||				TODO - Support email addresses delimited by a space??
||				TODO - Trim whitespace from source string (szString)
\*--------------------------------------------------------------------------*/
int ProcessEmailAddrString( LPTSTR szString, list<EmailAddr_t>& EmailAddrList, bool bThrowException, bool bAlwaysAdd )
{
	EmailAddr_t EmailAddr;

	bool bInBracket = false;
	bool bFoundBracket = false;
	bool bInQuotes = false;
	bool bInSquareBracket = false;
	bool bFoundSquareBracket = false;
	int nContacts = 0;
	bool bMailboxUsePhrase = false;

	TCHAR* p = szString;
	TCHAR* q = szString;

	// loop through the string
	for(int i = 0; p[i] != 0x00; i++)
	{
		// look for quotes
		if (!bInBracket && (p[i] == _T('\"')))                                 
		{
			bInQuotes = !bInQuotes;
			continue;
		}

		// look for brackets
		if (!bInQuotes )
		{
			if (!bInBracket)
			{
				// look for the start of brackets
				if(p[i] == _T('<') || p[i] == _T('(') || p[i] == _T('[') )
				{
					bFoundBracket = true;
					bInBracket = true;

					// force it to be a bracket in the source string
					if(p[i] != _T('['))
					{
						p[i] = _T('<');
						// clear the email address
						// we want to use what's in the brackets
						EmailAddr.m_sEmailAddr.erase();
					}
					else
					{
						// square bracket, may be a contact group
						bInSquareBracket = true;
						EmailAddr.m_sEmailAddr.append(1, p[i]);
					}
					continue;
				}
				else
				{
					if(p[i] != _T(' ') && p[i] != _T(';') && p[i] != _T(','))
						if(bMailboxUsePhrase)
							return -1;
				}
			}
			else if(p[i] == _T('>') || p[i] == _T(')') ||  p[i] == _T(']') )
			{					
				// fource it to be a bracket in the source string
				if(p[i] != _T(']'))
				{
					p[i] = _T('>');
				}
				else
				{
					bFoundBracket = false;
					bFoundSquareBracket = true;
					EmailAddr.m_sEmailAddr.append(1, p[i]);
				}
				
				bInBracket = false;
				bInSquareBracket = false;
				continue;
			}
		}

		// are we at the end of this email address
		if(!bInBracket && !bInQuotes && (p[i] == _T(';') || p[i] == _T(',')))
		{
			// if we never found a bracket... there isn't a name
			if (!bFoundBracket)
			{
				EmailAddr.m_sName.erase();
			}
			else
			{
				EmailAddr.m_sName.DoubleQuoteRealName();
			}

			// verify the email address and add it to the list
			if(bFoundSquareBracket)
			{
				VerifyEmailAddr( EmailAddr, EmailAddrList, bThrowException, true );
				p[i] = _T(';');
				nContacts++;
			}
			else if (VerifyEmailAddr( EmailAddr, EmailAddrList, bThrowException, bAlwaysAdd ))
			{
				p[i] = _T(';');
				nContacts++;
			}
				
			// reset vars
			bInBracket = false;
			bFoundBracket = false;
			bInSquareBracket = false;
			bFoundSquareBracket = false;
			bInQuotes = false;
			bMailboxUsePhrase = false;
			EmailAddr.Clear();
			continue;
		}	
		
		// add character to the email address
		if ( bInBracket || (!bInQuotes && !bFoundBracket) )
			EmailAddr.m_sEmailAddr.append(1, p[i]);
			
		// add character to the name
		if(!bInBracket && !bFoundBracket)
		{
			if(bInQuotes)
				if(!bMailboxUsePhrase)
					bMailboxUsePhrase = true;

			EmailAddr.m_sName.append(1, p[i]);
		}
	}	
	
	// if we never found a bracket... there isn't a name
	if (!bFoundBracket)
	{
		EmailAddr.m_sName.erase();
	}
	else
	{
		EmailAddr.m_sName.DoubleQuoteRealName();
	}
	
	if(bInBracket)
	{
		if(bMailboxUsePhrase)
			return -1;
	}

	// add the last item...
	if(bFoundSquareBracket)
	{
		VerifyEmailAddr( EmailAddr, EmailAddrList, bThrowException, true );
		nContacts ++;
	}
	else if ( VerifyEmailAddr( EmailAddr, EmailAddrList, bThrowException, bAlwaysAdd ) )
	{
		nContacts ++;
	}
		
	
	return nContacts;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Parses an email address string and creates a list of 
||				ContactNameAddr structs.  szString MUST be NULL terminated.
||				Returns the number or email addresses parsed.
||
||				TODO - Support email addresses delimited by a space??
||				TODO - Trim whitespace from source string (szString)
\*--------------------------------------------------------------------------*/
int ProcessContactGroups( CODBCQuery& query, LPTSTR szString, LPTSTR& szResultString, bool bThrowException, bool bAlwaysAdd )
{
	tstring sResult;

	bool bInBracket = false;
	bool bFoundBracket = false;
	bool bInQuotes = false;
	bool bInSquareBracket = false;
	bool bFoundSquareBracket = false;
	int nContacts = 0;
	bool bMailboxUsePhrase = false;

	TCHAR* p = szString;
	TCHAR* q = szString;
	tstring sTemp;
	tstring sName;

	// loop through the string
	int i = 0;
	for(; p[i] != 0x00; i++)
	{
		// look for quotes
		if (!bInBracket && (p[i] == _T('\"')))                                 
		{
			bInQuotes = !bInQuotes;
			continue;
		}

		// look for brackets
		if (!bInQuotes )
		{
			if (!bInBracket)
			{
				// look for the start of brackets
				if(p[i] == _T('<') || p[i] == _T('(') || p[i] == _T('[') )
				{
					bFoundBracket = true;
					bInBracket = true;

					// force it to be a bracket in the source string
					if(p[i] != _T('['))
					{
						p[i] = _T('<');
						// clear the email address
						// we want to use what's in the brackets
						sTemp = _T("");
					}
					else
					{
						// square bracket, may be a contact group
						bInSquareBracket = true;
						sTemp = sTemp + p[i];
					}
					continue;
				}
				else
				{
					if(p[i] != _T(' ') && p[i] != _T(';') && p[i] != _T(','))
						if(bMailboxUsePhrase)
							return -1;
				}
			}
			else if(p[i] == _T('>') || p[i] == _T(')') ||  p[i] == _T(']') )
			{					
				// fource it to be a bracket in the source string
				if(p[i] != _T(']'))
				{
					p[i] = _T('>');
				}
				else
				{
					bFoundBracket = false;
					bFoundSquareBracket = true;
					sTemp = sTemp + p[i];
				}
				
				bInBracket = false;
				bInSquareBracket = false;
				continue;
			}
		}

		// are we at the end of this email address
		if(!bInBracket && !bInQuotes && (p[i] == _T(';') || p[i] == _T(',')))
		{
			// if we never found a bracket... there isn't a name
			if (!bFoundBracket)
				sName = _T("");
			
			// verify the email address and add it to the list
			if(bFoundSquareBracket)
			{
				tstring sT = sTemp;
				if(sT.at(0) == '[')
				{
					sT = sT.substr(1,sT.length());
				}
				if(sT.at(sT.length()-1) == ']')
				{
					sT = sT.substr(0,sT.length()-1);
				}
				
				int nCount = 0;				
				query.Initialize();
				BINDPARAM_TCHAR_STRING( query, sT );
				BINDCOL_LONG_NOLEN( query, nCount );
				query.Execute( _T("SELECT COUNT(*) FROM ContactGroups WHERE GroupName = ?"));
				query.Fetch();

				if(nCount == 1)
				{
					// Append sTemp to result
					sResult = sResult + sTemp + _T(";");					
					p[i] = _T(';');
					nContacts++;
				}
			}
			else
			{
				// Append sTemp to result
				if(sName.length())
				{
					tstring sT = sName;
					if(sT.at(0) == ' ')
					{
						sT = sT.substr(1,sT.length());
					}
					if(sT.at(sT.length()-1) == ' ')
					{
						sT = sT.substr(0,sT.length()-1);
					}
					sTemp = _T("\"") + sT + _T("\"<") + sTemp + _T(">");
				}
				sResult = sResult + sTemp + _T(";");
				p[i] = _T(';');
				nContacts++;
			}
				
			// reset vars
			bInBracket = false;
			bFoundBracket = false;
			bInSquareBracket = false;
			bFoundSquareBracket = false;
			bInQuotes = false;
			bMailboxUsePhrase = false;
			sTemp = _T("");
			continue;
		}	
		
		// add character to the email address
		if ( bInBracket || (!bInQuotes && !bFoundBracket) )
			sTemp = sTemp + p[i];

		// add character to the name
		if(!bInBracket && !bFoundBracket)
		{
			if(bInQuotes)
				if(!bMailboxUsePhrase)
					bMailboxUsePhrase = true;

			sName = sName + p[i];
		}
	}	
	
	// if we never found a bracket... there isn't a name
	if (!bFoundBracket)
	{
		sName = _T("");
	}
	
	if(bInBracket)
	{
		if(bMailboxUsePhrase)
			return -1;
	}

	// add the last item...
	if(bFoundSquareBracket)
	{
		tstring sT = sTemp;
		if(sT.at(0) == '[')
		{
			sT = sT.substr(1,sT.length());
		}
		if(sT.at(sT.length()-1) == ']')
		{
			sT = sT.substr(0,sT.length()-1);
		}
		
		int nCount = 0;				
		query.Initialize();
		BINDPARAM_TCHAR_STRING( query, sT );
		BINDCOL_LONG_NOLEN( query, nCount );
		query.Execute( _T("SELECT COUNT(*) FROM ContactGroups WHERE GroupName = ?"));
		query.Fetch();

		if(nCount == 1)
		{
			// Append sTemp to result
			sResult = sResult + sTemp;

			p[i] = _T(';');
			nContacts++;
		}
	}
	else
	{
		// Append sTemp to result
		tstring sT = sName;
		if(sT.at(0) == ' ')
		{
			sT = sT.substr(1,sT.length());
		}
		if(sT.at(sT.length()-1) == ' ')
		{
			sT = sT.substr(0,sT.length()-1);
		}
		sTemp = _T("\"") + sT + _T("\"<") + sTemp + _T(">");
				
		sResult = sResult + sTemp;
		nContacts ++;
	}
		
	// Copy the result
	_tcscpy(szResultString, sResult.c_str());
	return nContacts;
}

void FormatEmailAddrString( TCHAR** szBuffer, long& nLength, long& nAllocated )
{
	list<EmailAddr_t> AddrList;
	
	ProcessEmailAddrString( *szBuffer, AddrList, false, true );

	FormatEmailAddrString( AddrList, szBuffer, nLength, nAllocated );
}

void FormatEmailAddrString( list<EmailAddr_t>& AddrList, TCHAR** szBuffer, long& nLength, long& nAllocated )
{	
	tstring sAddr;
	list<EmailAddr_t>::iterator iter;
	
	for ( iter = AddrList.begin(); iter != AddrList.end(); iter++ )
	{
		if ( iter != AddrList.begin() )
			sAddr += _T("; ");
		
		if ( iter->m_sName.size() > 0 )
		{
			tstring sName;
			sName.assign(iter->m_sName.c_str());
			if(sName.at(0) == '\"')
			{
				sName = sName.erase(0,1);						
			}
			if(sName.at(sName.size()-1) == '\"')
			{
				sName = sName.erase(sName.size()-1,1);						
			}
			
			sAddr += _T("\"");
			sAddr += sName;
			sAddr += _T("\" <");
		}
		
		sAddr += iter->m_sEmailAddr;
		
		if ( iter->m_sName.size() > 0 )
			sAddr += _T(">");
	}
	
	PutStringProperty( sAddr, szBuffer, &nAllocated );
	nLength = sAddr.length();
}
