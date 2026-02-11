/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/StringFns.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   String Functions
||              
\\*************************************************************************/

#include "stdafx.h"
#include "StringFns.h"
#include "RegistryFns.h"


////////////////////////////////////////////////////////////////////////////////
// 
// PutStringProperty - Helper function for handling TEXT properties
// 
////////////////////////////////////////////////////////////////////////////////
static void PutStringProperty( TCHAR* wcsBuffer, long wcsBufLen, 
					           TCHAR** ppBuffer, long* pAllocated )
{
	if( wcsBufLen == 0 )
	{
		if( *pAllocated > 0)
		{
			// NULL terminate the buffer
			(*ppBuffer)[0] = 0;
		}
	}
	else
	{
		long ByteLen = (wcsBufLen+1) * sizeof(TCHAR);

		if( *pAllocated < ByteLen )
		{
			// Allocate the new buffer
			TCHAR* wcsNewBuffer = (TCHAR*)calloc( ByteLen, 1 );
		
			if( *pAllocated > 0 )
			{
				// Free the old buffer
				free( *ppBuffer );
			}
		
			// Set the old pointer to the new buffer
			*ppBuffer = wcsNewBuffer;
			*pAllocated = ByteLen;
		}

		// copy the data and NULL terminate
		_tcsncpy( *ppBuffer, wcsBuffer, wcsBufLen+1 );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// PutStringProperty - Helper function for handling TEXT properties
// 
////////////////////////////////////////////////////////////////////////////////
void PutStringProperty( tstring& string, TCHAR** ppBuffer,  long* pAllocated )
{
	PutStringProperty( (TCHAR*)string.c_str(), string.size(), ppBuffer, pAllocated );
}


// The following functions are only available for UNICODE compilation
#ifdef _UNICODE
////////////////////////////////////////////////////////////////////////////////
// 
// PutStringProperty - Helper function for handling TEXT properties
// 
////////////////////////////////////////////////////////////////////////////////
void PutStringProperty( BSTR newVal, TCHAR** ppBuffer,  long* pAllocated )
{
	PutStringProperty( newVal, SysStringLen(newVal), ppBuffer, pAllocated );
}

////////////////////////////////////////////////////////////////////////////////
// 
// PutStringProperty - Helper function for handling TEXT properties
// 
////////////////////////////////////////////////////////////////////////////////
void PutStringPropertyW( wchar_t* wcsString, wchar_t** ppBuffer,  long* pAllocated )
{
	PutStringProperty( wcsString, wcslen( wcsString ), ppBuffer, pAllocated );
}

#endif // _UNICODE 


////////////////////////////////////////////////////////////////////////////////
// 
// RemoveTicketIDFromSubject - Removes a TicketID and OutboundMessageID from the string
//
////////////////////////////////////////////////////////////////////////////////
void RemoveTicketIDFromSubject( tstring& strSubject )
{
	int nStart = 0;
	int nStop = 0;
	bool nColon;
	bool bContinue;
	size_t nOffset = 0;
	UINT nUsePar=0;
	UINT nUseTicketNo=0;

	GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseParInSubject"), nUsePar );
	GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseTicketNoInSubject"), nUseTicketNo );

	if(nUsePar == 0)
	{
		// remove anything of the form [#:#]
		nOffset = 0;
		while ( nOffset < strSubject.size() )
		{
			if( strSubject[nOffset] == _T('[') )
			{
				int nStartBracket = nOffset;
				nStart = nOffset++;
				nColon = false;
				bContinue = true;
				while ( bContinue && nOffset < strSubject.size() )
				{
					if( strSubject[nOffset] == _T(':') )
					{
						nColon = true;
					}
					else if( strSubject[nOffset] == _T(']') )
					{
						if( nColon )
						{
							strSubject.erase( nStart, nOffset + 1 - nStart);
							nOffset = nStart - 1;
						}
						bContinue = false;
					}
					else if( (_istdigit(strSubject[nOffset]) == 0) && ( nUseTicketNo == 0 ))
					{
						// non-digit character, break out of the loop
						bContinue = false;
					}
					else if( nOffset - nStartBracket > 128 )
					{
						bContinue = false;
					}
					
					if( bContinue) 
					{
						nOffset++;
					}
				}
			}
			nOffset++;
		}
	}
	
	if(nUsePar == 1)
	{
		// remove anything of the form (#:#)
		nOffset = 0;
		while ( nOffset < strSubject.size() )
		{
			if( strSubject[nOffset] == _T('(') )
			{
				int nStartBracket = nOffset;
				nStart = nOffset++;
				nColon = false;
				bContinue = true;
				while ( bContinue && nOffset < strSubject.size() )
				{
					if( strSubject[nOffset] == _T(':') )
					{
						nColon = true;
					}
					else if( strSubject[nOffset] == _T(')') )
					{
						if( nColon )
						{
							strSubject.erase( nStart, nOffset + 1 - nStart);
							nOffset = nStart - 1;
						}
						bContinue = false;
					}
					else if( _istdigit(strSubject[nOffset]) == 0 )
					{
						// non-digit character, break out of the loop
						bContinue = false;
					}
					else if( nOffset - nStartBracket > 128 )
					{
						bContinue = false;
					}
					
					if( bContinue) 
					{
						nOffset++;
					}
				}
			}
			nOffset++;
		}
	}
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// PutTicketIDInSubject - inserts a TicketID and OutboundMessageID into 
//                        szSubject, which must be of length:
//                        INBOUNDMESSAGES_SUBJECT_LENGTH. If the resulting
//                        subject line is too long, it is terminated with
//						  ellipsis...
////////////////////////////////////////////////////////////////////////////////
void PutTicketIDInSubject( int TicketID, int MsgID, TCHAR* szSubject, int nTicketTracking )
{
	tstring strSubject = szSubject;	// need a copy for sntprintf
	int ret;
	UINT nUsePar=0;
	UINT nUseTicketNo=0;
	
	GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseParInSubject"), nUsePar );
	GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseTicketNoInSubject"), nUseTicketNo );

	// Remove the ticketID and OutboundMessageID, if present
	RemoveTicketIDFromSubject( strSubject );

	if( nTicketTracking == 0 )
	{
		if ( nUseTicketNo == 1 )
		{
			if( strSubject.size() > INBOUNDMESSAGES_SUBJECT_LENGTH - 37 )
			{
				strSubject.resize( INBOUNDMESSAGES_SUBJECT_LENGTH - 40 );
				strSubject.append( _T("...") );
			}
		}
		else
		{
			if( strSubject.size() > INBOUNDMESSAGES_SUBJECT_LENGTH - 26 )
			{
				strSubject.resize( INBOUNDMESSAGES_SUBJECT_LENGTH - 29 );
				strSubject.append( _T("...") );
			}
		}
		

		// format the new subject line
		if( nUsePar == 1 )
		{
			ret = _sntprintf( szSubject, INBOUNDMESSAGES_SUBJECT_LENGTH - 1, _T("%s (%d:%d)"),
							strSubject.c_str(), TicketID, MsgID );
		}
		else
		{
			if ( nUseTicketNo == 1 )
			{
				ret = _sntprintf( szSubject, INBOUNDMESSAGES_SUBJECT_LENGTH - 1, _T("%s [Ticket No. %d:%d]"),
							strSubject.c_str(), TicketID, MsgID );
			}
			else
			{
				ret = _sntprintf( szSubject, INBOUNDMESSAGES_SUBJECT_LENGTH - 1, _T("%s [%d:%d]"),
							strSubject.c_str(), TicketID, MsgID );
			}		
		}
	}
	else
	{
		ret = _sntprintf( szSubject, INBOUNDMESSAGES_SUBJECT_LENGTH - 1, _T("%s"), strSubject.c_str() );
	}

	// zero terminate to be safe
	szSubject[INBOUNDMESSAGES_SUBJECT_LENGTH - 1] = _T('\0');
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetTicketIDFromSubject
// 
////////////////////////////////////////////////////////////////////////////////
bool GetTicketIDFromSubject( TCHAR* szSubject, int& TicketID, int& MsgID )
{
	const int TicketBufferLength = 43;
	TCHAR szTicketID[TicketBufferLength];
	TCHAR* p = szSubject;
	TCHAR* e = p + INBOUNDMESSAGES_SUBJECT_LENGTH;
	TCHAR* q = szTicketID;
	TCHAR* f = q + TicketBufferLength - 1;
	bool bInBrackets = false;
	UINT nUsePar=0;
	UINT nUseTicketNo=0;

	GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseParInSubject"), nUsePar );
	GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseTicketNoInSubject"), nUseTicketNo );

	if(nUsePar == 0)
	{
		while( *p && p < e )
		{
			if( bInBrackets )
			{
				if( *p == _T(']') )
				{
					// we're out of the brackets now
					bInBrackets = false;

					// zero terminate szTicketID
					*q = 0;

					// strip Ticket No. from szTicketID
					if ( nUseTicketNo == 1 )
					{
						tstring sTicketID = szTicketID;
						if ( sTicketID.substr(0,10) == _T("Ticket No.") )
						{
							sTicketID.erase( 0, 11);
							_tcscpy( szTicketID, sTicketID.c_str() );
						}										
					}

					// scan for 2 integers separated by a colon
					if( _stscanf( szTicketID, _T("%d:%d"), &TicketID, &MsgID ) == 2 )
					{
						// we got it!
						return true;
					}
					else
					{
						// nothing doing, try again
						q = szTicketID;
					}
				}
				else
				{
					// copy to our TicketID buffer
					if( q < f )
					{
						*q++ = *p;
					}
				}
			}
			else if( *p == _T('[') )
			{
				bInBrackets = true;
			}
			p++;
		}
		
		bInBrackets = false;
		p = szSubject;
		e = p + INBOUNDMESSAGES_SUBJECT_LENGTH;
		while( *p && p < e )
		{
			if( bInBrackets )
			{
				if( *p == _T(')') )
				{
					// we're out of the brackets now
					bInBrackets = false;

					// zero terminate szTicketID
					*q = 0;

					// strip Ticket No. from szTicketID
					if ( nUseTicketNo == 1 )
					{
						tstring sTicketID = szTicketID;
						if ( sTicketID.substr(0,10) == _T("Ticket No.") )
						{
							sTicketID.erase( 0, 11);
							_tcscpy( szTicketID, sTicketID.c_str() );
						}										
					}

					// scan for 2 integers separated by a colon
					if( _stscanf( szTicketID, _T("%d:%d"), &TicketID, &MsgID ) == 2 )
					{
						// we got it!
						return true;
					}
					else
					{
						// nothing doing, try again
						q = szTicketID;
					}
				}
				else
				{
					// copy to our TicketID buffer
					if( q < f )
					{
						*q++ = *p;
					}
				}
			}
			else if( *p == _T('(') )
			{
				bInBrackets = true;
			}
			p++;
		}

	}	

	if(nUsePar == 1)
	{
		bInBrackets = false;
		p = szSubject;
		e = p + INBOUNDMESSAGES_SUBJECT_LENGTH;
		while( *p && p < e )
		{
			if( bInBrackets )
			{
				if( *p == _T(')') )
				{
					// we're out of the brackets now
					bInBrackets = false;

					// zero terminate szTicketID
					*q = 0;

					// strip Ticket No. from szTicketID
					if ( nUseTicketNo == 1 )
					{
						tstring sTicketID = szTicketID;
						if ( sTicketID.substr(0,10) == _T("Ticket No.") )
						{
							sTicketID.erase( 0, 11);
							_tcscpy( szTicketID, sTicketID.c_str() );
						}										
					}

					// scan for 2 integers separated by a colon
					if( _stscanf( szTicketID, _T("%d:%d"), &TicketID, &MsgID ) == 2 )
					{
						// we got it!
						return true;
					}
					else
					{
						// nothing doing, try again
						q = szTicketID;
					}
				}
				else
				{
					// copy to our TicketID buffer
					if( q < f )
					{
						*q++ = *p;
					}
				}
			}
			else if( *p == _T('(') )
			{
				bInBrackets = true;
			}
			p++;
		}

		bInBrackets = false;
		p = szSubject;
		e = p + INBOUNDMESSAGES_SUBJECT_LENGTH;		
		while( *p && p < e )
		{
			if( bInBrackets )
			{
				if( *p == _T(']') )
				{
					// we're out of the brackets now
					bInBrackets = false;

					// zero terminate szTicketID
					*q = 0;

					// strip Ticket No. from szTicketID
					if ( nUseTicketNo == 1 )
					{
						tstring sTicketID = szTicketID;
						if ( sTicketID.substr(0,10) == _T("Ticket No.") )
						{
							sTicketID.erase( 0, 11);
							_tcscpy( szTicketID, sTicketID.c_str() );
						}										
					}

					// scan for 2 integers separated by a colon
					if( _stscanf( szTicketID, _T("%d:%d"), &TicketID, &MsgID ) == 2 )
					{
						// we got it!
						return true;
					}
					else
					{
						// nothing doing, try again
						q = szTicketID;
					}
				}
				else
				{
					// copy to our TicketID buffer
					if( q < f )
					{
						*q++ = *p;
					}
				}
			}
			else if( *p == _T('[') )
			{
				bInBrackets = true;
			}
			p++;
		}
	}
	
	// Nothing found
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RemoveTicketIDFromBody - Removes a TicketID and OutboundMessageID from the body
//
////////////////////////////////////////////////////////////////////////////////
void RemoveTicketIDFromBody( tstring& strBody )
{
	int nStart = 0;
	int nStop = 0;
	bool nColon;
	bool bContinue;
	size_t nOffset = 0;
	UINT nUsePar=0;
	UINT nUseTicketNo=0;
	tstring sTicketID;
	int TicketID=0;
	int MsgID=0;

	GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseParInSubject"), nUsePar );
	GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseTicketNoInSubject"), nUseTicketNo );

	// remove anything of the form [#:#]
	nOffset = 0;
	while ( nOffset < strBody.size() )
	{
		if( strBody[nOffset] == _T('[') )
		{
			int nStartBracket = nOffset;
			sTicketID.clear();
			nStart = nOffset++;
			nColon = false;
			bContinue = true;
			while ( bContinue && nOffset < strBody.size() )
			{
				if( strBody[nOffset] == _T(':') )
				{
					nColon = true;
				}
				else if( strBody[nOffset] == _T(']') )
				{
					TCHAR szTicketID[128];
					_tcscpy( szTicketID, sTicketID.c_str() );
					if( nColon )
					{
						if ( _stscanf( szTicketID, _T("Ticket tracking data, please do not remove. %d:%d"), &TicketID, &MsgID ) == 2 )
						{
							strBody.erase( nStart, nOffset + 1 - nStart);
							nOffset = nStart - 1;
						}
					}
					bContinue = false;
				}
				else if( nOffset - nStartBracket > 128 )
				{
					bContinue = false;
				}
								
				if( bContinue) 
				{
					sTicketID.append(1,strBody[nOffset]);
					nOffset++;
				}
			}
		}
		
		nOffset++;
	}
	
	if( nUsePar == 1 )
	{
		// remove anything of the form (#:#)
		nOffset = 0;
		while ( nOffset < strBody.size() )
		{
			if( strBody[nOffset] == _T('(') )
			{
				int nStartBracket = nOffset;
				sTicketID.clear();
				nStart = nOffset++;
				nColon = false;
				bContinue = true;
				while ( bContinue && nOffset < strBody.size() )
				{
					if( strBody[nOffset] == _T(':') )
					{
						nColon = true;
					}
					else if( strBody[nOffset] == _T(')') )
					{
						TCHAR szTicketID[128];
						_tcscpy( szTicketID, sTicketID.c_str() );
						if( nColon )
						{
							if ( _stscanf( szTicketID, _T("Ticket tracking data, please do not remove. %d:%d"), &TicketID, &MsgID ) == 2 )
							{
								strBody.erase( nStart, nOffset + 1 - nStart);
								nOffset = nStart - 1;
							}
						}
						bContinue = false;
					}
					else if( nOffset - nStartBracket > 128 )
					{
						bContinue = false;
					}
									
					if( bContinue) 
					{
						sTicketID.append(1,strBody[nOffset]);
						nOffset++;
					}
				}
			}
			
			nOffset++;
		}
	}
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetTicketIDFromBody
// 
////////////////////////////////////////////////////////////////////////////////
bool GetTicketIDFromBody( TCHAR* szBody, int& TicketID, int& MsgID )
{
	tstring strBody = szBody;
	const int TicketBufferLength = 128;
	TCHAR szTicketID[TicketBufferLength];
	TCHAR* p = szBody;
	TCHAR* e = p + strBody.length();
	TCHAR* q = szTicketID;
	TCHAR* f = q + TicketBufferLength - 1;
	bool bInBrackets = false;
	UINT nUsePar=0;	

	GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("UseParInSubject"), nUsePar );	

	// loop through the body
	while( *p && p < e )
	{
		if( bInBrackets )
		{
			if( *p == _T(']') )
			{
				// we're out of the brackets now
				bInBrackets = false;

				// zero terminate szTicketID
				*q = 0;				

				// scan for 2 integers separated by a colon
				if( _stscanf( szTicketID, _T("Ticket tracking data, please do not remove. %d:%d"), &TicketID, &MsgID ) == 2 )
				{
					// we got it!
					return true;
				}
				else
				{
					// nothing doing, try again
					q = szTicketID;
				}
			}
			else
			{
				// copy to our TicketID buffer
				if( q < f )
				{
					*q++ = *p;
				}
			}
		}
		else if( *p == _T('[') )
		{
			bInBrackets = true;
		}
		p++;
	}

	if( nUsePar == 1 )
	{
		// loop through the body
		bInBrackets = false;
		p = szBody;
		e = p + strBody.length();
		while( *p && p < e )
		{
			if( bInBrackets )
			{
				if( *p == _T(')') )
				{
					// we're out of the brackets now
					bInBrackets = false;

					// zero terminate szTicketID
					*q = 0;

					// scan for 2 integers separated by a colon
					if( _stscanf( szTicketID, _T("Ticket tracking data, please do not remove. %d:%d"), &TicketID, &MsgID ) == 2 )
					{
						// we got it!
						return true;
					}
					else
					{
						// nothing doing, try again
						q = szTicketID;
					}
				}
				else
				{
					// copy to our TicketID buffer
					if( q < f )
					{
						*q++ = *p;
					}
				}
			}
			else if( *p == _T('(') )
			{
				bInBrackets = true;
			}
			p++;
		}
	}
	
	// Nothing found
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// InetNToA - TCHAR version of inet_ntoa - szIP must be at least 16 TCHARs! 
// 
////////////////////////////////////////////////////////////////////////////////
void InetNToA( unsigned int n, TCHAR* szIP )
{
	_stprintf( szIP, _T("%d.%d.%d.%d"),
		       (n >> 24) & 0xff, (n >> 16) & 0xff, (n >> 8 ) & 0xff, (n & 0xff) );
}

////////////////////////////////////////////////////////////////////////////////
// 
// InetAddr - wchar_t version of inet_addr 
// 
////////////////////////////////////////////////////////////////////////////////
unsigned int InetAddr( tstring& IP )
{
	unsigned int ip = 0;
	unsigned int n;
	TCHAR szTemp[16];
	TCHAR* p = (TCHAR*)(IP.c_str());
	TCHAR* q;
	int i;

	for ( i = 3; i >= 0; i-- )
	{
		// copy the first octet to the temp buffer
		q = szTemp;
		while ( (*p != _T('.')) && (*p != 0) && (q - szTemp < 15))
		{
			*q = *p;
			p++;
			q++;
		}

		// zero terminate the temp buffer
		*q = '\0';

		// advance to the next octet
		if ( *p != 0 )
			p++;

		n = _ttoi( szTemp );

		if( n > 255 )
		{
			THROW_EMS_EXCEPTION( E_InvalidParameters, _T("Invalid IP Address") ); 
		}

		// shift the byte into the DWORD
		ip += n << (i*8);
	}
	
	// voila!
	return ip;
}
