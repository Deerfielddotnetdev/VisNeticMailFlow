/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/PurgeFns.cpp,v 1.2.4.1 2006/05/01 15:44:11 markm Exp $
||
||                                         
||  COMMENTS:	Purge Helper Functions    
||              
\\*************************************************************************/

#include "stdafx.h"
#include <tchar.h>
#include "PurgeFns.h"
#include "QueryClasses.h"
#include "AttachFns.h"
#include "TicketHistoryFns.h"
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges a comma delimited list of tickets            
\*--------------------------------------------------------------------------*/
void PurgeTickets( CEMSString& sIDs, CODBCQuery& query, int nNumTickets, bool bKeepTicketSummary )
{
	CEMSString sStatus;
	CEMSString sQuery;
	CEMSString sChunk;
	tstring sFullAttachPath;
	TAttachments attach;
	list<unsigned int> AttachmentIDList;
	list<unsigned int>::iterator iter;
	
	
	sIDs.CDLInit();

	sStatus.Format(_T("Purging [%d] Tickets."), nNumTickets);
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );

	int nNumChunks = 1;
	int nNumTicketsPurged = 0;
	while ( sIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		// TODO - wrap everything in a transaction?

		query.Initialize();
		BINDCOL_LONG_NOLEN( query, attach.m_AttachmentID );
		sQuery.Format( _T("SELECT A.AttachmentID FROM Attachments as A ")
			           _T("INNER JOIN InboundMessageAttachments as I ON A.AttachmentID = I.AttachmentID ")
			           _T("WHERE InboundMessageID IN (SELECT InboundMessageID ")
			           _T("FROM InboundMessages WHERE TicketID IN (%s))")
			           _T("UNION ")
					   _T("SELECT A.AttachmentID FROM Attachments as A ")
					   _T("INNER JOIN OutboundMessageAttachments as O ON A.AttachmentID = O.AttachmentID ")
					   _T("WHERE OutboundMessageID IN (SELECT OutboundMessageID ")
					   _T("FROM OutboundMessages WHERE TicketID IN (%s))")
					   _T("UNION ")
					   _T("SELECT A.AttachmentID FROM Attachments as A ")
					   _T("INNER JOIN NoteAttachments as N ON A.AttachmentID = N.AttachmentID ")
					   _T("WHERE NoteID IN (SELECT TicketNoteID ")
					   _T("FROM TicketNotes WHERE TicketID IN (%s)AND NoteTypeID=1)"), sChunk.c_str(), sChunk.c_str(), sChunk.c_str() );
	

		query.Execute( sQuery.c_str() );
		
		while( query.Fetch() == S_OK )
		{
			AttachmentIDList.push_back( attach.m_AttachmentID );
		}
		
		// delete OutboundMessageAttachment records
		query.Reset();

		sQuery.Format( _T("DELETE FROM OutboundMessageAttachments ")
			_T("WHERE OutboundMessageID IN (SELECT OutboundMessageID ")
			_T("FROM OutboundMessages WHERE TicketID IN (%s))"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );

		// delete InboundMessageAttachment records
		query.Reset();
		
		sQuery.Format( _T("DELETE FROM InboundMessageAttachments ")
			_T("WHERE InboundMessageID IN (SELECT InboundMessageID ")
			_T("FROM InboundMessages WHERE TicketID IN (%s))"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );

		// delete NoteAttachment records
		query.Reset();

		sQuery.Format( _T("DELETE FROM NoteAttachments ")
			_T("WHERE NoteID IN (SELECT TicketNoteID ")
			_T("FROM TicketNotes WHERE TicketID IN (%s) AND NoteTypeID=1)"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );		

		// for each attachment ID
		for ( iter = AttachmentIDList.begin(); iter != AttachmentIDList.end(); iter++ )
		{
			// if the attachment is no longer referenced, delete it from
			// the disk and remove the record from the Attachments table
			if ( GetAttachmentReferenceCount( query, *iter ) == 0 )
			{
				attach.m_AttachmentID = *iter;
				attach.Query( query );
				
				if ( attach.m_VirusScanStateID != EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED )
				{
					attach.m_IsInbound ? GetFullInboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath ) :
										 GetFullOutboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath );
				}
				else
				{
					GetFullQuarantinePath( attach.m_AttachmentLocation, sFullAttachPath );
				}

				DeleteFile( sFullAttachPath.c_str() );
				
				attach.Delete( query );
			}
		}

		AttachmentIDList.clear();

		if( bKeepTicketSummary )
		{
			// Log the undeleted messages being removed
			multimap<unsigned int,unsigned int> TktMsgMap;
			multimap<unsigned int,unsigned int>::iterator TktMsgMapIter;
			unsigned int nTicketID, nMsgID;

			query.Reset();
			BINDCOL_LONG_NOLEN( query, nTicketID );
			BINDCOL_LONG_NOLEN( query, nMsgID );
			sQuery.Format( _T("SELECT TicketID,InboundMessageID ")
				           _T("FROM InboundMessages ")
						   _T("WHERE TicketID IN (%s) AND IsDeleted=0"), sChunk.c_str() );

			query.Execute( sQuery.c_str() );

			while( query.Fetch() == S_OK )
			{
				TktMsgMap.insert( pair<unsigned int,unsigned int> (nTicketID,nMsgID) );
			}
	
			for( TktMsgMapIter = TktMsgMap.begin(); TktMsgMapIter != TktMsgMap.end(); TktMsgMapIter++ )
			{
				THDelInboundMsg( query, TktMsgMapIter->first, 0, TktMsgMapIter->second, 0 );
			}	
			
			TktMsgMap.clear();

			query.Reset();
			BINDCOL_LONG_NOLEN( query, nTicketID );
			BINDCOL_LONG_NOLEN( query, nMsgID );
			sQuery.Format( _T("SELECT TicketID,OutboundMessageID ")
				           _T("FROM OutboundMessages ")
						   _T("WHERE TicketID IN (%s) AND IsDeleted=0"), sChunk.c_str() );

			query.Execute( sQuery.c_str() );

			while( query.Fetch() == S_OK )
			{
				TktMsgMap.insert( pair<unsigned int,unsigned int> (nTicketID,nMsgID) );
			}
	
			for( TktMsgMapIter = TktMsgMap.begin(); TktMsgMapIter != TktMsgMap.end(); TktMsgMapIter++ )
			{
				THDelOutboundMsg( query, TktMsgMapIter->first, 0, TktMsgMapIter->second, 0 );
			}	
		}

		query.Reset();

		// delete inboundmessageread
		sQuery.Format( _T("DELETE FROM InboundMessageRead ")
			_T("WHERE InboundMessageID IN (SELECT InboundMessageID ")
			_T("FROM InboundMessages WHERE TicketID IN (%s))"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );

		query.Reset();

		// delete inboundmessagequeue
		sQuery.Format( _T("DELETE FROM InboundMessageQueue ")
			_T("WHERE InboundMessageID IN (SELECT InboundMessageID ")
			_T("FROM InboundMessages WHERE TicketID IN (%s))"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );

		query.Reset();

		// delete srkeywords
		sQuery.Format( _T("DELETE FROM SRKeywordResults ")
			_T("WHERE InboundMessageID IN (SELECT InboundMessageID ")
			_T("FROM InboundMessages WHERE TicketID IN (%s))"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );

		query.Reset();

		// delete ticketnotesread
		sQuery.Format( _T("DELETE FROM TicketNotesRead ")
			_T("WHERE TicketNoteID IN (SELECT TicketNoteID ")
			_T("FROM TicketNotes WHERE TicketID IN (%s))"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );

		query.Reset();

		// delete inbound message records
		sQuery.Format( _T("DELETE FROM InboundMessages ")
			_T("WHERE TicketID IN (%s)"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );

		query.Reset();

		// delete outbound message contacts
		sQuery.Format( _T("DELETE FROM OutboundMessageContacts ")
			_T("WHERE OutboundMessageID IN (SELECT OutboundMessageID ")
			_T("FROM OutboundMessages WHERE TicketID IN (%s))"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );

		query.Reset();
	
		// delete outbound message queue records
		sQuery.Format( _T("DELETE FROM OutboundMessageQueue ")
			_T("WHERE OutboundMessageID IN (SELECT OutboundMessageID ")
			_T("FROM OutboundMessages WHERE TicketID IN (%s))"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );

		query.Reset();
	
		// delete outbound message records
		sQuery.Format( _T("DELETE FROM OutboundMessages ")
			_T("WHERE TicketID IN (%s)"), sChunk.c_str() );
		
		query.Execute( sQuery.c_str() );		
		
		if( bKeepTicketSummary == false )
		{
			query.Reset();

			// delete ticket notes
			sQuery.Format( _T("DELETE FROM TicketNotes ")
				_T("WHERE TicketID IN (%s)"), sChunk.c_str() );
			
			query.Execute( sQuery.c_str() );
			
			query.Reset();

			// delete custom fields
			sQuery.Format( _T("DELETE FROM TicketFieldsTicket ")
				_T("WHERE TicketID IN (%s)"), sChunk.c_str() );
			
			query.Execute( sQuery.c_str() );
			
			query.Reset();

			// delete ticket links
			sQuery.Format( _T("DELETE FROM TicketLinksTicket ")
				_T("WHERE TicketID IN (%s)"), sChunk.c_str() );
			
			query.Execute( sQuery.c_str() );
			
			query.Reset();

			// delete ticket contact records
			sQuery.Format( _T("DELETE FROM TicketContacts ")
				_T("WHERE TicketID IN (%s)"), sChunk.c_str() );
			
			query.Execute( sQuery.c_str() );
			
			query.Reset();

			// delete ticket history records
			sQuery.Format( _T("DELETE FROM TicketHistory ")
				_T("WHERE TicketID IN (%s)"), sChunk.c_str() );
			
			query.Execute( sQuery.c_str() );
	
			query.Reset();
			
			// delete the ticket record
			sQuery.Format( _T("DELETE FROM Tickets ")
				_T("WHERE TicketID IN (%s)"), sChunk.c_str() );
			
			query.Execute( sQuery.c_str() );
		}

		if(25*nNumChunks > nNumTickets)
		{
			nNumTicketsPurged = nNumTickets;
		}
		else
		{
			nNumTicketsPurged = 25*nNumChunks;
		}
		sStatus.Format(_T("Purged [%d] of [%d] Tickets."), nNumTicketsPurged, nNumTickets);
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
		nNumChunks++;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges a comma delimited list of InboundMessages            
\*--------------------------------------------------------------------------*/
void PurgeInboundMessages( CEMSString& sIDs, CODBCQuery& query )
{
	CEMSString sQuery;
	CEMSString sChunk;
	TAttachments attach;
	tstring sFullAttachPath;
	list<unsigned int> AttachmentIDList;
	list<unsigned int>::iterator iter;

	sIDs.CDLInit();

	while ( sIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		/*---------------------------------------------------------------------------\                     
		||  Comments:	Get a list of associated AttachmentIDs	              
		\*--------------------------------------------------------------------------*/
		query.Initialize();
		BINDCOL_LONG_NOLEN( query, attach.m_AttachmentID );
		sQuery.Format( _T("SELECT A.AttachmentID FROM Attachments as A ")
			           _T("INNER JOIN InboundMessageAttachments as I on A.AttachmentID = I.AttachmentID ")
			           _T("WHERE InboundMessageID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );

		while( query.Fetch() == S_OK )
		{
			AttachmentIDList.push_back( attach.m_AttachmentID );
		}
		
		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from InboundMessageAttachments              
		\*--------------------------------------------------------------------------*/
		sQuery.Format( _T("DELETE FROM InboundMessageAttachments ")
			_T("WHERE InboundMessageID IN (%s)"), sChunk.c_str() );
		
		query.Reset();
		query.Execute( sQuery.c_str() );

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Check to see if each attachment is still referenced.  If it 
		||				is not, delete the file from the disk and remove the record
		\*--------------------------------------------------------------------------*/
		for ( iter = AttachmentIDList.begin(); iter != AttachmentIDList.end(); iter++ )
		{
			// if the file is no longer referenced, delete it from the disk
			// and remove the record in the Attachments table
			if ( GetAttachmentReferenceCount( query, *iter ) == 0 )
			{
				attach.m_AttachmentID = *iter;
				attach.Query( query );
				
				if ( attach.m_VirusScanStateID != EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED )
				{
					attach.m_IsInbound ? GetFullInboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath ) :
										 GetFullOutboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath );
				}
				else
				{
					GetFullQuarantinePath( attach.m_AttachmentLocation, sFullAttachPath );
				}
				
				DeleteFile( sFullAttachPath.c_str() );
				
				attach.Delete( query );
			}
		}

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from InboundMessagesRead             
		\*--------------------------------------------------------------------------*/
		query.Reset();
		sQuery.Format( _T("DELETE FROM InboundMessageRead WHERE InboundMessageID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from InboundMessagesRead             
		\*--------------------------------------------------------------------------*/
		query.Reset();
		sQuery.Format( _T("DELETE FROM SRKeywordResults WHERE InboundMessageID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from InboundMessages              
		\*--------------------------------------------------------------------------*/
		query.Reset();
		sQuery.Format( _T("DELETE FROM InboundMessages WHERE InboundMessageID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Purges a comma delimited list of OutboundMessages            
\*--------------------------------------------------------------------------*/
void PurgeOutboundMessages( CEMSString& sIDs, CODBCQuery& query )
{
	CEMSString sQuery;
	CEMSString sChunk;
	TAttachments attach;
	tstring sFullAttachPath;
	list<unsigned int> AttachmentIDList;
	list<unsigned int>::iterator iter;
		
	sIDs.CDLInit();

	while ( sIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from OutboundMessageQueue	              
		\*--------------------------------------------------------------------------*/
		query.Initialize();
		sQuery.Format( _T("DELETE FROM OutboundMessageQueue WHERE OutboundMessageID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );
		
		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from OutboundMessageContacts	              
		\*--------------------------------------------------------------------------*/
		query.Reset();
		sQuery.Format( _T("DELETE FROM OutboundMessageContacts WHERE OutboundMessageID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Get a list of associated AttachmentIDs	              
		\*--------------------------------------------------------------------------*/
		query.Reset();
		BINDCOL_LONG_NOLEN( query, attach.m_AttachmentID );
		sQuery.Format( _T("SELECT A.AttachmentID FROM Attachments as A ")
			           _T("INNER JOIN OutboundMessageAttachments as O on A.AttachmentID = O.AttachmentID ")
			           _T("WHERE OutboundMessageID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );
		
		while( query.Fetch() == S_OK )
		{
			AttachmentIDList.push_back( attach.m_AttachmentID );
		}

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from OutboundMessageAttachments              
		\*--------------------------------------------------------------------------*/
		query.Reset();
		sQuery.Format( _T("DELETE FROM OutboundMessageAttachments WHERE OutboundMessageID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Check to see if each attachment is still referenced.  If it 
		||				is not, delete the file from the disk and remove the record
		\*--------------------------------------------------------------------------*/
		for ( iter = AttachmentIDList.begin(); iter != AttachmentIDList.end(); iter++ )
		{
			// if the file is no longer referenced, delete it from the disk
			// and remove the record in the Attachments table
			if ( GetAttachmentReferenceCount( query, *iter ) == 0 )
			{
				attach.m_AttachmentID = *iter;
				attach.Query( query );
				
				if ( attach.m_VirusScanStateID != EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED )
				{
					attach.m_IsInbound ? GetFullInboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath ) :
										 GetFullOutboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath );
				}
				else
				{
					GetFullQuarantinePath( attach.m_AttachmentLocation, sFullAttachPath );
				}
				
				DeleteFile( sFullAttachPath.c_str() );
				
				attach.Delete( query );
			}
		}

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from OutboundMessages              
		\*--------------------------------------------------------------------------*/
		query.Reset();
		sQuery.Format( _T("DELETE FROM OutboundMessages WHERE OutboundMessageID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );
	}
}



////////////////////////////////////////////////////////////////////////////////
// 
// PurgeContacts
// 
////////////////////////////////////////////////////////////////////////////////
void PurgeContacts( CEMSString& sIDs, CODBCQuery& query )
{
	CEMSString sQuery;
	tstring sChunk;

	sIDs.CDLInit();

	while ( sIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		try
		{
			query.Initialize();
			query.Execute( _T("BEGIN TRANSACTION") );

			query.Reset(true);			
			sQuery.Format( _T("DELETE FROM OutboundMessageContacts WHERE ContactID IN (%s)"), sChunk.c_str() );
			query.Execute( sQuery.c_str() );

			query.Reset(true);			
			sQuery.Format( _T("DELETE FROM TicketContacts WHERE ContactID IN (%s)"), sChunk.c_str() );
			query.Execute( sQuery.c_str() );

			query.Reset(true);			
			sQuery.Format( _T("DELETE FROM ContactNotes WHERE ContactID IN (%s)"), sChunk.c_str() );
			query.Execute( sQuery.c_str() );

			query.Reset(true);			
			sQuery.Format( _T("DELETE FROM ContactGrouping WHERE ContactID IN (%s)"), sChunk.c_str() );
			query.Execute( sQuery.c_str() );

			query.Reset(true);			
			sQuery.Format( _T("DELETE FROM AgentContacts WHERE ContactID IN (%s)"), sChunk.c_str() );
			query.Execute( sQuery.c_str() );

			query.Reset(true);			
			sQuery.Format( _T("DELETE FROM PersonalData WHERE ContactID IN (%s)"), sChunk.c_str() );
			query.Execute( sQuery.c_str() );

			query.Reset(true);			
			sQuery.Format( _T("DELETE FROM Contacts WHERE ContactID IN (%s)"), sChunk.c_str() );
			query.Execute( sQuery.c_str() );
			
			query.Reset(true);
			query.Execute( _T("COMMIT TRANSACTION") );
		}
		catch( ... )
		{
			query.Initialize();
			query.Execute( _T("ROLLBACK TRANSACTION") );
			throw;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// PurgeStandardResponses
// 
////////////////////////////////////////////////////////////////////////////////
void PurgeStandardResponses( CEMSString& sIDs, CODBCQuery& query )
{
	TAttachments attach;
	tstring sFullAttachPath;
	list<unsigned int> AttachmentIDList;
	list<unsigned int>::iterator iter;
	CEMSString sQuery;
	CEMSString sChunk;

	sIDs.CDLInit();

	while ( sIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from standard response usage	              
		\*--------------------------------------------------------------------------*/
		query.Initialize();
		sQuery.Format( _T("DELETE FROM StandardResponseUsage WHERE StandardResponseID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from standard response favorites              
		\*--------------------------------------------------------------------------*/
		query.Reset();
		sQuery.Format( _T("DELETE FROM StdResponseFavorites WHERE StandardResponseID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Get	a list of associated AttachmentIDs     
		\*--------------------------------------------------------------------------*/
		query.Reset();
		BINDCOL_LONG_NOLEN( query, attach.m_AttachmentID );
		sQuery.Format( _T("SELECT A.AttachmentID FROM Attachments as A ")
			           _T("INNER JOIN StdResponseAttachments as S on A.AttachmentID = S.AttachmentID ")
			           _T("WHERE StandardResponseID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );
		
		while( query.Fetch() == S_OK )
		{
			AttachmentIDList.push_back( attach.m_AttachmentID );
		}
	
		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from StdResponseAttachments      
		\*--------------------------------------------------------------------------*/
		query.Reset();
		sQuery.Format( _T("DELETE FROM StdResponseAttachments WHERE StandardResponseID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );

		/*---------------------------------------------------------------------------\                     
		||  Comments:	Check to see if each attachment is still referenced.  If it 
		||				is not, delete the file from the disk and remove the record
		\*--------------------------------------------------------------------------*/
		for ( iter = AttachmentIDList.begin(); iter != AttachmentIDList.end(); iter++ )
		{
			// if the file is no longer referenced, delete it from the disk
			// and remove the record in the Attachments table
			if ( GetAttachmentReferenceCount( query, *iter ) == 0 )
			{
				attach.m_AttachmentID = *iter;
				attach.Query( query );
				
				if ( attach.m_VirusScanStateID != EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED )
				{
					attach.m_IsInbound ? GetFullInboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath ) :
										 GetFullOutboundAttachPath( attach.m_AttachmentLocation, sFullAttachPath );
				}
				else
				{
					GetFullQuarantinePath( attach.m_AttachmentLocation, sFullAttachPath );
				}
				
				DeleteFile( sFullAttachPath.c_str() );
				
				attach.Delete( query );
			}
		}
		
		/*---------------------------------------------------------------------------\                     
		||  Comments:	Delete from StandardResponses      
		\*--------------------------------------------------------------------------*/
		query.Reset();
		sQuery.Format( _T("DELETE FROM StandardResponses WHERE StandardResponseID IN (%s)"), sChunk.c_str() );
		query.Execute( sQuery.c_str() );
	}
}