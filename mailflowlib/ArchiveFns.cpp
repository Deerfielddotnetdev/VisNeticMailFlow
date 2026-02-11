/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/ArchiveFns.cpp,v 1.2.2.2 2006/02/15 15:02:08 markm Exp $
||
||
||                                         
||  COMMENTS:	Archive Functions
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ArchiveFns.h"
#include "AttachFns.h"
#include "PurgeFns.h"
#include "QueryClasses.h"
#include "DateFns.h"
#include "DebugReporter.h"
#include "RegistryFns.h"
#include "EMSMutex.h"

#define RETONERR(x)   if( (nRet= x ) != Arc_Success ) return nRet


////////////////////////////////////////////////////////////////////////////////
// 
// ExportArchive
// 
////////////////////////////////////////////////////////////////////////////////
int ExportArchive( CODBCQuery& query, tstring& sDestFile, int nDays, 
				   bool bRemoveTickets, bool bKeepTicketSummary, CEMSString& sErr, CEMSString& sTicketIDs, int& nNumTickets, bool& bDoMore, int m_nArchiveMaxTickets, bool m_bArchiveAllowAccess )
{
	CArchiveFile arc(query);
	int retval = 0;
	
	arc.OpCode() = Arc_Op_Export;

	try
	{
		query.Initialize();
		query.Execute( _T("BEGIN TRANSACTION") );

		retval = arc.ExportArchive( sDestFile, nDays, bRemoveTickets, bKeepTicketSummary, sTicketIDs, nNumTickets, bDoMore, m_nArchiveMaxTickets, m_bArchiveAllowAccess );

		query.Initialize();
		if( retval == Arc_Success )
		{
			query.Execute( _T("COMMIT TRANSACTION") );			
		}
		else
		{
			bDoMore = false;
			query.Execute( _T("ROLLBACK TRANSACTION") );
		}
	}
	catch( ODBCError_t err )
	{
		sErr.Format( _T("ExportArchive: Caught ODBC err: %s"), err.szErrMsg );
		query.Initialize();
		query.Execute( _T("ROLLBACK TRANSACTION") );
		retval = Arc_ErrODBC;
	}
	catch( CEMSException e )
	{
		sErr.Format( _T("ExportArchive:: Caught EMS exception: %s"), e.GetErrorString() );
		query.Initialize();
		query.Execute( _T("ROLLBACK TRANSACTION") );
		retval = Arc_ErrException;
	}
	catch( ... )
	{
		sErr.assign( _T("ExportArchive:: Caught unhandled exception") );
		query.Initialize();
		query.Execute( _T("ROLLBACK TRANSACTION") );
		retval = Arc_ErrException;
	}

	// If the operation failed, delete the archive file
	if( retval != Arc_Success )
	{
		if( arc.IsOpen() )
		{
			arc.Close();
		}
		DeleteFile( sDestFile.c_str() );
	}
	//else
	//{
		// If it succeeded, do a shrink database
		//arc.ShrinkDatabase();
	//}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// ImportArchive
// 
////////////////////////////////////////////////////////////////////////////////
int ImportArchive( CODBCQuery& query, tstring& sDestFile, 
				   bool bReplaceDuplicates, CEMSString& sErr )
{
	CArchiveFile arc(query);
	int retval = 0;

	arc.OpCode() = Arc_Op_Import;

	arc.Merge() = !bReplaceDuplicates;

	try
	{
		query.Initialize();
		query.Execute( _T("BEGIN TRANSACTION") );

		/*query.Initialize();
		query.Execute( _T("SET IDENTITY_INSERT Tickets ON") );*/

		retval = arc.ImportArchive( sDestFile );

		query.Initialize();
		if( retval == Arc_Success )
		{
			query.Execute( _T("COMMIT TRANSACTION") );
		}
		else
		{
			query.Execute( _T("ROLLBACK TRANSACTION") );
		}
	}
	catch( ODBCError_t err )
	{
		sErr.Format( _T("ImportArchive:: Caught ODBC err: %s"), err.szErrMsg );
		query.Initialize();
		query.Execute( _T("ROLLBACK TRANSACTION") );
		retval = Arc_ErrODBC;
	}
	catch( CEMSException e )
	{
		sErr.Format( _T("ImportArchive:: Caught EMS exception: %s"), e.GetErrorString() );
		query.Initialize();
		query.Execute( _T("ROLLBACK TRANSACTION") );
		retval = Arc_ErrException;
	}
	catch( ... )
	{
		sErr.assign( _T("ImportArchive:: Caught unhandled exception") );
		query.Initialize();
		query.Execute( _T("ROLLBACK TRANSACTION") );
		retval = Arc_ErrException;
	}

	query.Initialize();
	query.Execute( _T("SET IDENTITY_INSERT Tickets OFF") );

	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Backup
// 
////////////////////////////////////////////////////////////////////////////////
int Backup( CODBCQuery& query, tstring& sDestFile, CEMSString& sErr )
{
	CArchiveFile arc(query);
	int retval = 0;

	arc.OpCode() = Arc_Op_Backup;

	try
	{
		query.Initialize();
		query.Execute( _T("BEGIN TRANSACTION") );

		retval = arc.Backup( sDestFile );

		query.Initialize();
		if( retval == Arc_Success )
		{
			query.Execute( _T("COMMIT TRANSACTION") );
		}
		else
		{
			query.Execute( _T("ROLLBACK TRANSACTION") );
		}	
	}
	catch( ODBCError_t err )
	{
		sErr.Format( _T("Backup:: Caught ODBC err: %s"), err.szErrMsg );

		try
		{
			query.Initialize();
			query.Execute( _T("ROLLBACK TRANSACTION") );
		}
		catch(...)
		{
		}

		retval = Arc_ErrODBC;
	}
	catch( CEMSException e )
	{
		sErr.Format( _T("Backup:: Caught EMS exception: %s"), e.GetErrorString() );
		
		try
		{
			query.Initialize();
			query.Execute( _T("ROLLBACK TRANSACTION") );
		}
		catch(...)
		{
		}

		retval = Arc_ErrException;
	}
	catch( ... )
	{
		sErr.assign( _T("Backup:: Caught unhandled exception") );
		
		try
		{
			query.Initialize();
			query.Execute( _T("ROLLBACK TRANSACTION") );
		}
		catch(...)
		{
		}

		retval = Arc_ErrException;
	}

	if( retval != Arc_Success )
	{
		if( arc.IsOpen() )
		{
			arc.Close();
			DeleteFile( sDestFile.c_str() );
		}
	}
	//else
	//{
		//arc.ShrinkDatabase();
	//}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Restore
// 
////////////////////////////////////////////////////////////////////////////////
int Restore( CODBCQuery& query, tstring& sSrcFile, bool bEraseFirst, 
			 bool bReplaceDuplicates, bool bReplaceConfig, CEMSString& sErr )
{
	CArchiveFile arc(query);
	int retval = 0;

	arc.OpCode() = Arc_Op_Restore;

	arc.Merge() = !bReplaceDuplicates;
	arc.ReplaceConfig() = bReplaceConfig;

	try
	{
		query.Initialize();
		query.Execute( _T("BEGIN TRANSACTION") );
				
		retval = arc.Restore( sSrcFile, true );

		query.Initialize();
		if( retval == Arc_Success )
		{
			query.Execute( _T("COMMIT TRANSACTION") );
		}
		else
		{
			query.Execute( _T("ROLLBACK TRANSACTION") );
		}	
	}
	catch( ODBCError_t err )
	{
		//DEBUGPRINT(_T("- MaiStreamEngine.Restore %d, %d, %s, %s.\n"),err.nErrorCode,err.nNativeErrorCode,err.szErrMsg,err.szState);
		sErr.Format( _T("Restore:: Caught ODBC err: %s"), err.szErrMsg );
		query.Initialize();
		query.Execute( _T("ROLLBACK TRANSACTION") );
		retval = Arc_ErrODBC;
	}
	catch( CEMSException e )
	{
		sErr.Format( _T("Restore:: Caught EMS exception: %s"), e.GetErrorString() );
		query.Initialize();
		query.Execute( _T("ROLLBACK TRANSACTION") );
		retval = Arc_ErrException;
	}
	catch( ... )
	{
		sErr.assign( _T("Restore:: Caught unhandled exception") );
		query.Initialize();
		query.Execute( _T("ROLLBACK TRANSACTION") );
		retval = Arc_ErrException;
	}

	//if( retval == Arc_Success )
		//arc.ShrinkDatabase();

	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// CArchiveFile constructor
// 
////////////////////////////////////////////////////////////////////////////////
CArchiveFile::CArchiveFile( CODBCQuery& query ) : m_query(query)
{
	// Initialize the array of lookup tables
	ZeroMemory( m_LookupTables, sizeof(m_LookupTables) );
	for( int i = 0; i < TABLEID_COUNT; i++ )
	{
		m_LookupTables[i].TableID = i;
	}

	// Allocate buffers
	m_InBuffer = new unsigned char[ARCHIVE_BUFFER_SEED];
	m_InBuffLen = ARCHIVE_BUFFER_SEED;
	m_InBuffUsed = 0;

	m_OutBuffer = new unsigned char[ARCHIVE_BUFFER_SEED];
	ZeroMemory(m_OutBuffer, ARCHIVE_BUFFER_SEED);
	m_OutBuffLen = ARCHIVE_BUFFER_SEED;
	m_OutBuffUsed = 0;

	
	zalloc = 0;  // z_stream member
    zfree = 0;   // z_stream member
    opaque = 0;  // z_stream member

	m_hFile = INVALID_HANDLE_VALUE;

	// Default is to merge
	m_bMerge = true;
	m_bReplaceConfig = false;
}


////////////////////////////////////////////////////////////////////////////////
// 
// CArchiveFile destructor
// 
////////////////////////////////////////////////////////////////////////////////
CArchiveFile::~CArchiveFile()
{
	if( m_InBuffer )
	{
		delete[] m_InBuffer;
	}
	if( m_OutBuffer )
	{
		delete[] m_OutBuffer;
	}

	Close();
}


////////////////////////////////////////////////////////////////////////////////
// 
// ExportArchive
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::ExportArchive( tstring& sDestFile, int nDays, 
								 bool bRemoveTickets, bool bKeepTicketSummary, CEMSString& sTicketIDs, int& nNumTickets, bool& bDoMore, int m_nArchiveMaxTickets, bool m_bArchiveAllowAccess )
{
	CEMSString sStatus;
	list<ArchiveIndexEntry> TicketIDs;
	list<ArchiveIndexEntry> InMsgIDs;
	list<ArchiveIndexEntry> OutMsgIDs;
	list<ArchiveIndexEntry> InAttachIDs;
	list<ArchiveIndexEntry> OutAttachIDs;
	list<ArchiveIndexEntry> AttachIDs;
	list<ArchiveIndexEntry> TicketBoxIDs;
	list<ArchiveIndexEntry> TicketBoxHdrIDs;
	list<ArchiveIndexEntry> TicketBoxFtrIDs;
	list<ArchiveIndexEntry> TicketContactIDs;
	list<ArchiveIndexEntry> OutMsgContactIDs;
	list<ArchiveIndexEntry> ContactIDs;
	list<ArchiveIndexEntry> PersonalDataIDs;
	list<ArchiveIndexEntry> TicketNoteIDs;
	list<ArchiveIndexEntry> NoteAttachIDs;
	list<ArchiveIndexEntry> TicketHistoryIDs;
	multimap<unsigned int,unsigned int> TicketOutMsgMap;
	multimap<unsigned int,unsigned int> TicketInMsgMap;
	multimap<unsigned int,unsigned int>::iterator TicketMsgMapiter;
	TTicketHistory tktHistory;

	ArchiveIndexEntry identry;
	ArchiveIndexEntry identry2;
	//CEMSString sTicketIDs;
	CEMSString sTicketBoxIDs;
	CEMSString sContactIDs;
	CEMSString sID;
	CEMSString sChunk;
	CEMSString sQuery;
	set<unsigned int> ContactIDSet;
	set<unsigned int> TicketBoxHdrSet;
	set<unsigned int> TicketBoxFtrSet;
	set<unsigned int> TicketBoxSet;
	long TicketBoxID = 0;
	int nRet = 0;
	int TicketBoxHdrID;
	int TicketBoxFtrID;
	TArchives arch;
	int TicketID;
	bDoMore = false;

	_tcscpy( arch.m_ArcFilePath, sDestFile.c_str() );
	GetTimeStamp( arch.m_DateCreated );
	arch.m_DateCreatedLen = sizeof(TIMESTAMP_STRUCT);
	arch.m_Purged = (bRemoveTickets) ? 1 : 0;

	// Insert to get the ArchiveID
	arch.Insert( m_query );

	// Initialization
	ZeroMemory( &identry, sizeof(identry) );
	ZeroMemory( &identry2, sizeof(identry2) );

	RETONERR( Open( sDestFile, false ) );
	
	sStatus.Format(_T("Getting list of Tickets to Archive."));
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );

	// Tickets
	m_query.Initialize();
	BINDPARAM_LONG( m_query, nDays );
	BINDPARAM_LONG( m_query, nDays );
	BINDPARAM_LONG( m_query, nDays );
	BINDCOL_LONG_NOLEN( m_query, identry.ID );
	BINDCOL_LONG_NOLEN( m_query, TicketBoxID );

	if( bRemoveTickets && !bKeepTicketSummary )
	{
		// removing tickets - select all old tickets
		m_query.Execute( _T("SELECT TicketID,TicketBoxID FROM Tickets AS T ")
					     _T("WHERE DATEDIFF(day,OpenTimeStamp,getdate()) >= ? ")
						 _T("AND ( (SELECT COUNT(*) FROM InboundMessages AS I WHERE T.TicketID=I.TicketID) = 0 ")
						 _T("OR DATEDIFF(day,(SELECT MAX(I.DateReceived)FROM InboundMessages AS I WHERE T.TicketID=I.TicketID),getdate()) >= ? ) ")
						 _T("AND ( (SELECT COUNT(*) FROM OutboundMessages AS O WHERE T.TicketID=O.TicketID) = 0 ")
						 _T("OR DATEDIFF(day,(SELECT MAX(O.EmailDateTime)FROM OutboundMessages AS O WHERE T.TicketID=O.TicketID),getdate()) >= ? ) ")
						 _T("AND (SELECT COUNT(*) FROM OutboundMessages AS O WHERE T.TicketID=O.TicketID AND O.OutboundMessageStateID = 1) = 0 ")
						 _T("ORDER BY TicketID") );
	}
	else
	{
		// Not removing tickets - only select old tickets with messages.
		m_query.Execute( _T("SELECT TicketID,TicketBoxID FROM Tickets AS T ")
					     _T("WHERE DATEDIFF(day,OpenTimeStamp,getdate()) >= ? ")
						 _T("AND ( (SELECT COUNT(*) FROM InboundMessages AS I WHERE T.TicketID=I.TicketID) = 0 ")
						 _T("OR DATEDIFF(day,(SELECT MAX(I.DateReceived)FROM InboundMessages AS I WHERE T.TicketID=I.TicketID),getdate()) >= ? ) ")
						 _T("AND ( (SELECT COUNT(*) FROM OutboundMessages AS O WHERE T.TicketID=O.TicketID) = 0 ")
						 _T("OR DATEDIFF(day,(SELECT MAX(O.EmailDateTime)FROM OutboundMessages AS O WHERE T.TicketID=O.TicketID),getdate()) >= ? ) ")
						 _T("AND (SELECT COUNT(*) FROM InboundMessages AS I WHERE T.TicketID=I.TicketID) + ")
						 _T("(SELECT COUNT(*) FROM OutboundMessages AS O WHERE T.TicketID=O.TicketID AND OutboundMessageStateID <> 1) > 0")
						 _T("ORDER BY TicketID") );
	}
	
	while( m_query.Fetch() == S_OK )
	{
		if( identry.ID != 0 )
		{
			TicketIDs.push_back( identry );
			sID.Format( _T("%d,"), identry.ID );
			sTicketIDs.append( sID );

			if( TicketBoxID != 0 && TicketBoxSet.find( TicketBoxID ) == TicketBoxSet.end() )
			{
				TicketBoxSet.insert( TicketBoxID );
				identry.ID = TicketBoxID;
				TicketBoxIDs.push_back( identry );
				sID.Format( _T("%d,"), TicketBoxID );
				sTicketBoxIDs.append( sID );
			}
		}
		if(m_nArchiveMaxTickets > 0 && bRemoveTickets)
		{
			nNumTickets = TicketIDs.size();
			if(nNumTickets >= m_nArchiveMaxTickets)
			{
				bDoMore = true;
				break;
			}
		}
	}

	if( TicketIDs.size() == 0 )
	{
		return Arc_NoTicketsFound;
	}

	// Now let's build the list of everything else based on these ticketIDs

	sStatus.Format(_T("Found [%d] Tickets to Archive, getting other objects to Archive based on these Tickets."),TicketIDs.size());
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );

	// Inbound Messages
	sTicketIDs.CDLInit();
	while ( sTicketIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		BINDCOL_LONG_NOLEN( m_query, TicketID );
		sQuery.Format( _T("SELECT InboundMessageID,TicketID ")
					   _T("FROM InboundMessages WHERE TicketID IN (%s) ")
					   _T("ORDER BY InboundMessageID"), sChunk.c_str() );

		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				InMsgIDs.push_back( identry );

				arch.m_InMsgRecords++;

				TicketInMsgMap.insert( pair<unsigned int,unsigned int> (TicketID,identry.ID) );
			}
		}		
	}
	
	// Outbound Messages
	sTicketIDs.CDLInit();
	while ( sTicketIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		BINDCOL_LONG_NOLEN( m_query, TicketID );
		sQuery.Format( _T("SELECT OutboundMessageID,TicketID ")
					   _T("FROM OutboundMessages WHERE TicketID IN (%s) ")
					   _T("AND OutboundMessageStateID <> 1 ")
					   _T("ORDER BY OutboundMessageID"), sChunk.c_str() );

		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				OutMsgIDs.push_back( identry );

				arch.m_OutMsgRecords++;

				TicketOutMsgMap.insert( pair<unsigned int,unsigned int> (TicketID,identry.ID) );
			}
		}		
	}

	// Inbound Message Attachments
	sTicketIDs.CDLInit();
	while ( sTicketIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		BINDCOL_LONG_NOLEN( m_query, identry2.ID );
		sQuery.Format( _T("SELECT InboundMessageAttachmentID,AttachmentID ")
					   _T("FROM InboundMessageAttachments ")
					   _T("WHERE InboundMessageID IN (SELECT InboundMessageID ")
					   _T("FROM InboundMessages WHERE TicketID IN (%s)) ")
					   _T("ORDER BY InboundMessageAttachmentID"), sChunk.c_str() );

		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				InAttachIDs.push_back( identry );
			}
			if( identry2.ID != 0 )
			{
				AttachIDs.push_back( identry2 );
				arch.m_InAttRecords++;
			}
		}		
	}
	
	// Outbound Message Attachments
	sTicketIDs.CDLInit();
	while ( sTicketIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		BINDCOL_LONG_NOLEN( m_query, identry2.ID );
		sQuery.Format( _T("SELECT OutboundMessageAttachmentID,AttachmentID ")
					   _T("FROM OutboundMessageAttachments ")
					   _T("WHERE OutboundMessageID IN (SELECT OutboundMessageID ")
					   _T("FROM OutboundMessages WHERE TicketID IN (%s) AND OutboundMessageStateID <> 1) ")
					   _T("ORDER BY OutboundMessageAttachmentID"), sChunk.c_str() );

		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				OutAttachIDs.push_back( identry );
			}
			if( identry2.ID != 0 )
			{
				arch.m_OutAttRecords++;

				// Check if this ID has already been added to the list
				list<ArchiveIndexEntry>::iterator iter = AttachIDs.begin();
				bool bFound = false;

				while( iter != AttachIDs.end() )
				{
					if( identry2.ID == iter->ID )
					{
						bFound = true;
						break;
					}
					iter++;
				}
				if( !bFound )
				{
					AttachIDs.push_back( identry2 );
				}
			}
		}		
	}

	// Outbound Message Contacts
	sTicketIDs.CDLInit();
	while ( sTicketIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		sQuery.Format( _T("SELECT OutboundMessageContactID ")
					   _T("FROM OutboundMessageContacts ")
					   _T("WHERE OutboundMessageID IN (SELECT OutboundMessageID ")
					   _T("FROM OutboundMessages WHERE TicketID IN (%s) AND OutboundMessageStateID <> 1) ")
					   _T("ORDER BY OutboundMessageContactID"), sChunk.c_str() );

		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				OutMsgContactIDs.push_back( identry );
			}
		}		
	}

	// TicketContacts
	sTicketIDs.CDLInit();
	while ( sTicketIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		sQuery.Format( _T("SELECT TicketContactID ")
					   _T("FROM TicketContacts ")
					   _T("WHERE TicketID IN (%s) "), sChunk.c_str() );

		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				TicketContactIDs.push_back( identry );
			}
		}		
	}

	// TicketNotes
	sTicketIDs.CDLInit();
	while ( sTicketIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		sQuery.Format( _T("SELECT TicketNoteID ")
					   _T("FROM TicketNotes WHERE TicketID IN (%s) ")
					   _T("ORDER BY TicketNoteID"), sChunk.c_str() );
		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				TicketNoteIDs.push_back( identry );
			}
		}
	}

	// TicketNote Attachments
	sTicketIDs.CDLInit();
	while ( sTicketIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		BINDCOL_LONG_NOLEN( m_query, identry2.ID );
		sQuery.Format( _T("SELECT NoteAttachmentID,AttachmentID ")
					   _T("FROM NoteAttachments ")
					   _T("WHERE NoteID IN (SELECT TicketNoteID ")
					   _T("FROM TicketNotes WHERE TicketID IN (%s)) AND NoteTypeID=1 ")
					   _T("ORDER BY NoteAttachmentID"), sChunk.c_str() );

		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				NoteAttachIDs.push_back( identry );
			}
			if( identry2.ID != 0 )
			{
				arch.m_NoteAttRecords++;

				// Check if this ID has already been added to the list
				list<ArchiveIndexEntry>::iterator iter = AttachIDs.begin();
				bool bFound = false;

				while( iter != AttachIDs.end() )
				{
					if( identry2.ID == iter->ID )
					{
						bFound = true;
						break;
					}
					iter++;
				}
				if( !bFound )
				{
					AttachIDs.push_back( identry2 );
				}
			}
		}		
	}	
	
	// Contacts
	sTicketIDs.CDLInit();
	while ( sTicketIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		sQuery.Format( _T("SELECT DISTINCT ContactID ")
					   _T("FROM TicketContacts WHERE TicketID IN (%s)")
					   _T("ORDER BY ContactID"), sChunk.c_str() );
		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 && ContactIDSet.find( identry.ID ) == ContactIDSet.end() )
			{
				ContactIDSet.insert( identry.ID );
				ContactIDs.push_back( identry );
				sID.Format( _T("%d,"), identry.ID );
				sContactIDs.append( sID );
			}
		}		
	}

	sContactIDs.CDLInit();
	while ( sContactIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		sQuery.Format( _T("SELECT PersonalDataID ")
					   _T("FROM PersonalData ")
					   _T("WHERE ContactID IN (%s) ")
					   _T("ORDER BY PersonalDataID"),
						sChunk.c_str() );
		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				PersonalDataIDs.push_back( identry );
			}
		}
	}
	
	// Put the ticket box header and footer IDs in a map to remove redundant IDs
	// and sort them in numerical order.
	sTicketBoxIDs.CDLInit();
	while ( sTicketBoxIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, TicketBoxHdrID );
		BINDCOL_LONG_NOLEN( m_query, TicketBoxFtrID );

		sQuery.Format( _T("SELECT HeaderID,FooterID ")
					   _T("FROM TicketBoxes WHERE TicketBoxID IN (%s)"), sChunk.c_str() );

		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( TicketBoxHdrID != 0 && TicketBoxHdrSet.find( TicketBoxHdrID ) == TicketBoxHdrSet.end() )
			{
				TicketBoxHdrSet.insert( TicketBoxHdrID );
				identry.ID = TicketBoxHdrID;
				TicketBoxHdrIDs.push_back( identry );
			}
			if( TicketBoxFtrID != 0 && TicketBoxFtrSet.find( TicketBoxFtrID ) == TicketBoxFtrSet.end() )
			{
				TicketBoxFtrSet.insert( TicketBoxFtrID );
				identry.ID = TicketBoxFtrID;
				TicketBoxFtrIDs.push_back( identry );
			}
		}		
	}

	// Create TicketHistory records
	tktHistory.m_AgentID = arch.m_ArchiveID;
	GetTimeStamp( tktHistory.m_DateTime );
	tktHistory.m_DateTimeLen = sizeof(TIMESTAMP_STRUCT);
	tktHistory.m_TicketActionID = EMS_TICKETACTIONID_ARC_MSG;
	tktHistory.m_ID1 = 0; // outbound

	for( TicketMsgMapiter = TicketOutMsgMap.begin(); TicketMsgMapiter != TicketOutMsgMap.end(); TicketMsgMapiter++ )
	{
		tktHistory.m_TicketID = TicketMsgMapiter->first;
		tktHistory.m_ID2 = TicketMsgMapiter->second;
		tktHistory.Insert( m_query );
	}
	
	tktHistory.m_ID1 = 1; // inbound

	for( TicketMsgMapiter = TicketInMsgMap.begin(); TicketMsgMapiter != TicketInMsgMap.end(); TicketMsgMapiter++ )
	{
		tktHistory.m_TicketID = TicketMsgMapiter->first;
		tktHistory.m_ID2 = TicketMsgMapiter->second;
		tktHistory.Insert( m_query );
	}

	// TicketHistory
	sTicketIDs.CDLInit();
	while ( sTicketIDs.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );
		sQuery.Format( _T("SELECT TicketHistoryID ")
					   _T("FROM TicketHistory WHERE TicketID IN (%s) ")
					   _T("ORDER BY TicketHistoryID"), sChunk.c_str() );
		m_query.Execute( sQuery.c_str() );
	
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				TicketHistoryIDs.push_back( identry );
			}
		}
	}

	RETONERR( Serialize( TABLEID_Tickets, TicketIDs ) );
	RETONERR( Serialize( TABLEID_InboundMessages, InMsgIDs ) );
	RETONERR( Serialize( TABLEID_OutboundMessages, OutMsgIDs ) );
	RETONERR( Serialize( TABLEID_InboundMessageAttachments, InAttachIDs ) );
	RETONERR( Serialize( TABLEID_OutboundMessageAttachments, OutAttachIDs ) );
	RETONERR( Serialize( TABLEID_NoteAttachments, NoteAttachIDs ) );
	RETONERR( Serialize( TABLEID_Attachments, AttachIDs ) );
	RETONERR( Serialize( TABLEID_TicketBoxes, TicketBoxIDs ) );
	RETONERR( Serialize( TABLEID_TicketBoxHeaders, TicketBoxHdrIDs ) );
	RETONERR( Serialize( TABLEID_TicketBoxFooters, TicketBoxFtrIDs ) );
	RETONERR( Serialize( TABLEID_OutboundMessageContacts, OutMsgContactIDs ) );
	RETONERR( Serialize( TABLEID_Contacts, ContactIDs ) );
	RETONERR( Serialize( TABLEID_TicketContacts, TicketContactIDs ) );
	RETONERR( Serialize( TABLEID_PersonalData, PersonalDataIDs ) );
	RETONERR( Serialize( TABLEID_TicketNotes, TicketNoteIDs ) );
	RETONERR( Serialize( TABLEID_TicketHistory, TicketHistoryIDs ) );

	sStatus.Format(_T("Finished writing data to Archive File, updating Lookup Tables."));
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );

	UpdateLookupTables();

	Close();
	
	if( bRemoveTickets )
	{
		sStatus.Format(_T("Purging Tickets."));
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );		
		if(m_bArchiveAllowAccess && m_nArchiveMaxTickets == 0)
		{
			nNumTickets = TicketIDs.size();
		}
		else
		{
			PurgeTickets( sTicketIDs, m_query, TicketIDs.size(), bKeepTicketSummary );
		}		
	}
	
	sStatus.Format(_T("Updating Archive entry in database."));
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );

	// Update Archive Entry w/ counts
	arch.Update( m_query );

	return nRet;
}



////////////////////////////////////////////////////////////////////////////////
// 
// ImportArchive
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::ImportArchive( tstring& sSrcFile )
{
	int nRet;
	TArchives arc; 
	int nCnt;

	RETONERR( Open( sSrcFile, true ) );

	// TODO: Create Archives entry for this archive.
	
	RETONERR( DeSerialize( TABLEID_TicketBoxFooters, nCnt ) );
	RETONERR( DeSerialize( TABLEID_TicketBoxHeaders, nCnt ) );
	RETONERR( DeSerialize( TABLEID_TicketBoxes, nCnt ) );
	RETONERR( DeSerialize( TABLEID_Tickets, nCnt ) );
	

	// TODO: Clear TicketArchive records, if any exist for this archive
	RETONERR( DeSerialize( TABLEID_OutboundMessages, arc.m_OutMsgRecords ) );
	RETONERR( DeSerialize( TABLEID_InboundMessages, arc.m_InMsgRecords ) );

	RETONERR( DeSerialize( TABLEID_Attachments, nCnt ) );

	RETONERR( DeSerialize( TABLEID_OutboundMessageAttachments, arc.m_OutAttRecords ) );
	RETONERR( DeSerialize( TABLEID_OutboundMessageContacts, nCnt ) );
	RETONERR( DeSerialize( TABLEID_InboundMessageAttachments, arc.m_InAttRecords ) );

	RETONERR( DeSerialize( TABLEID_Contacts, nCnt ) );
	RETONERR( DeSerialize( TABLEID_PersonalData, nCnt ) );
	RETONERR( DeSerialize( TABLEID_TicketContacts, nCnt ) );
	RETONERR( DeSerialize( TABLEID_TicketNotes, nCnt ) );
	RETONERR( DeSerialize( TABLEID_NoteAttachments, arc.m_NoteAttRecords ) );
	RETONERR( DeSerialize( TABLEID_TicketHistory, nCnt ) );

	RETONERR( FixupCircularDependencies() );

	// Update the archives entry
	m_query.Initialize();
	BINDPARAM_LONG( m_query, arc.m_InMsgRecords );
	BINDPARAM_LONG( m_query, arc.m_OutMsgRecords );
	BINDPARAM_LONG( m_query, arc.m_InAttRecords );
	BINDPARAM_LONG( m_query, arc.m_OutAttRecords );
	BINDPARAM_LONG( m_query, arc.m_NoteAttRecords );
	BINDPARAM_TCHAR_STRING( m_query, sSrcFile );
	m_query.Execute( _T("UPDATE Archives SET InMsgRecords=?,OutMsgRecords=?, ")
		             _T("InAttRecords=?,OutAttRecords=?,NoteAttRecords=? ")
					 _T("WHERE ArcFilePath=? ") );
	
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Backup
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::Backup( tstring& sDestFile )
{
	int nRet;

	RETONERR( Open( sDestFile, false ) );

	// Backup all tables
	for( int i = 0; i < TABLEID_COUNT; i++ )
	{
		RETONERR( Serialize( g_ArcTblOrder[i] ) );
	}

	RETONERR( UpdateLookupTables() );

	return Arc_Success;
}


	
////////////////////////////////////////////////////////////////////////////////
// 
// Restore
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::Restore( tstring& sSrcFile, bool bEraseFirst )
{
	CEMSString sStatus;
	CEMSString sPath;
	sPath.Format(_T("%s"),sSrcFile.c_str());
	CEMSString sQuery;
	int nRet = Arc_Success;
	int i;
	int nCnt;
	dca::String f;
	dca::String t(sSrcFile.c_str());
	sStatus.Format(_T("Restoring file [%s]"),sPath.c_str());
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
	f.Format("CArchiveFile::Restore - File to restore:[%s]", t.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
	RETONERR( Open( sSrcFile, true ) );

	
	if( bEraseFirst )
	{
		// Remove all data in tables
		sStatus.Format(_T("Removing table data."));
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
	
		DebugReporter::Instance().DisplayMessage("CArchiveFile::Restore - Removing table data", DebugReporter::LIB);
		for( i = TABLEID_COUNT-1; i >= 0; i-- )
		{
			if( g_ArcTblOrder[i] != TABLEID_Registry )
			{
				sStatus.Format(_T("Removing data from table [%s]"),g_ArcTblInfo[g_ArcTblOrder[i]].szTableName);
				WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
				m_query.Initialize();
				sQuery.Format( _T("DELETE FROM %s WHERE %s > %d"), 
							   g_ArcTblInfo[g_ArcTblOrder[i]].szTableName,
							   g_ArcTblInfo[g_ArcTblOrder[i]].szPrimaryKey,
							   g_ArcTblInfo[g_ArcTblOrder[i]].ReservedID );
				m_query.Execute( sQuery.c_str() );
			}
		}

		
		// Delete all files in attachment directories
		tstring sFullPath;
		GetInboundAttachPath( sFullPath);
		sPath.Format(_T("%s"),sFullPath.c_str());
		dca::String x(sFullPath.c_str());
		f.Format("CArchiveFile::Restore - Clearing InboundAttach:[%s]", x.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
		sStatus.Format(_T("Removing attachments from InboundAttach folder [%s]"),sPath.c_str());
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
				
		//RETONERR( Open( sSrcFile, true ) );
		DeleteDirectory( sFullPath.c_str() );
		/*WIN32_FIND_DATA findData;
		HANDLE hFind;
		tstring sFullPath;

		GetFullInboundAttachPath( _T("*.*"), sFullPath );
		hFind = FindFirstFile( sFullPath.c_str(), &findData );
		if (hFind != INVALID_HANDLE_VALUE) 
		{

			if( !( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				GetFullInboundAttachPath( findData.cFileName, sFullPath );			
				DeleteFile( sFullPath.c_str() );
			}	
			while( FindNextFile( hFind, &findData ) )
			{
				if( !( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
				{
					GetFullInboundAttachPath( findData.cFileName, sFullPath );			
					DeleteFile( sFullPath.c_str() );
				}
			}

			FindClose( hFind );
		}*/
		GetOutboundAttachPath( sFullPath);
		sPath.Format(_T("%s"),sFullPath.c_str());
		dca::String y(sFullPath.c_str());
		f.Format("CArchiveFile::Restore - Clearing OutboundAttach:[%s]", y.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
		sStatus.Format(_T("Removing attachments from OutboundAttach folder [%s]"),sPath.c_str());
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
		
		DeleteDirectory( sFullPath.c_str() );
		/*GetFullOutboundAttachPath( _T("*.*"), sFullPath );
		hFind = FindFirstFile( sFullPath.c_str(), &findData );
		if (hFind != INVALID_HANDLE_VALUE) 
		{

			if( !( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				GetFullOutboundAttachPath( findData.cFileName, sFullPath );			
				DeleteFile( sFullPath.c_str() );
			}	
			while( FindNextFile( hFind, &findData ) )
			{
				if( !( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
				{
					GetFullOutboundAttachPath( findData.cFileName, sFullPath );			
					DeleteFile( sFullPath.c_str() );
				}	
			}

			FindClose( hFind );
		}*/
		GetNoteAttachPath( sFullPath);
		sPath.Format(_T("%s"),sFullPath.c_str());
		dca::String z(sFullPath.c_str());
		f.Format("CArchiveFile::Restore - Clearing NoteAttach:[%s]", z.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
		sStatus.Format(_T("Removing attachments from NoteAttach folder [%s]"),sPath.c_str());
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
		
		DeleteDirectory( sFullPath.c_str() );
	}
	
	// Extract all tables from backup
	for( i = 0; i < TABLEID_COUNT; i++ )
	{
		DeSerialize(g_ArcTblOrder[i], nCnt);
	}

	FixupCircularDependencies();
	
	return nRet;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Open
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::Open( tstring& FileName, bool bReadOnly )
{
	DWORD dwBytes = 0;
	dca::String f;		

	if (FileName.size() == 0)
		return -1;

	m_FileName = FileName;

	// Add \\?\UNC or \\?
	if(FileName.at(0) == '\\' && FileName.at(1) == '\\' )
	{
		FileName = FileName.erase(0,1);
		FileName.insert(0,_T("\\\\?\\UNC"));
	}

	m_hFile = CreateFile(	FileName.c_str(),
							(bReadOnly) ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ,
							0,						// no sharing
							NULL,					// no security
							(bReadOnly) ? OPEN_EXISTING : CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,	// normal file
							NULL);

	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		f.Format("CArchiveFile::Open Failed to open Archive File [%s], will retry 4 more times.",FileName.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
		
		m_hFile = CreateFile(	FileName.c_str(),
							(bReadOnly) ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ,
							0,						// no sharing
							NULL,					// no security
							(bReadOnly) ? OPEN_EXISTING : CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,	// normal file
							NULL);

		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			f.Format("CArchiveFile::Open Failed to open Archive File [%s], will retry 3 more times.",FileName.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
			
			m_hFile = CreateFile(	FileName.c_str(),
							(bReadOnly) ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ,
							0,						// no sharing
							NULL,					// no security
							(bReadOnly) ? OPEN_EXISTING : CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,	// normal file
							NULL);

			if (m_hFile == INVALID_HANDLE_VALUE)
			{
				f.Format("CArchiveFile::Open Failed to open Archive File [%s], will retry 2 more times.",FileName.c_str());
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
				
				m_hFile = CreateFile(	FileName.c_str(),
							(bReadOnly) ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ,
							0,						// no sharing
							NULL,					// no security
							(bReadOnly) ? OPEN_EXISTING : CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,	// normal file
							NULL);

				if (m_hFile == INVALID_HANDLE_VALUE)
				{
					f.Format("CArchiveFile::Open Failed to open Archive File [%s], will retry 1 more time.",FileName.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
					
					m_hFile = CreateFile(	FileName.c_str(),
							(bReadOnly) ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ,
							0,						// no sharing
							NULL,					// no security
							(bReadOnly) ? OPEN_EXISTING : CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,	// normal file
							NULL);

					if (m_hFile == INVALID_HANDLE_VALUE)
					{
						f.Format("CArchiveFile::Open Failed to open Archive File [%s], 4 retries failed.",FileName.c_str());
						DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);

						//assert(0);
						return Arc_ErrOpenFile;
					}
				}
			}
		}		
	}
		
	m_dwFileSize.dwHighDateTime  = 0;
	m_dwFileSize.dwLowDateTime = GetFileSize((HANDLE)m_hFile, &m_dwFileSize.dwHighDateTime );

	if( bReadOnly )
	{
		// Verify the header block and read in the lookup tables.		
		if( !ReadFile( m_hFile, &m_hdr, sizeof(m_hdr), &dwBytes, NULL ) )
			return Arc_ErrReadingFile;
		
		if( m_hdr.MagicWord != 0xDFAC001 )
			return Arc_ErrNotArchive;
		
		if( !ReadFile( m_hFile, m_LookupTables, sizeof(ArchiveLookupTable) * m_hdr.LookupTableCount, &dwBytes, NULL ) )
			return Arc_ErrReadingFile;
	}
	else
	{
		// Write out the header block and lookup tables
		m_hdr.MagicWord = 0xDFAC001;
		m_hdr.Version = 0x05010000;
		m_hdr.LookupTableCount = TABLEID_COUNT;

		if( !WriteFile( m_hFile, &m_hdr, sizeof(m_hdr), &dwBytes, NULL ) )
			return Arc_ErrWritingToFile;

		if( !WriteFile( m_hFile, m_LookupTables, sizeof(m_LookupTables), &dwBytes, NULL ) )
			return Arc_ErrWritingToFile;
	}


	return Arc_Success;
}

////////////////////////////////////////////////////////////////////////////////
// 
// PastEndOfFile - since SetFilePointer will allow you to set the pointer 
//                 beyond end of file, we have to validate offsets ourself.
// 
////////////////////////////////////////////////////////////////////////////////
bool CArchiveFile::PastEndOfFile( DWORD dwHigh, DWORD dwLow, int length )
{
	FILETIME ft;
	__int64* pEnd;
	__int64* pPos;
	
	ft.dwHighDateTime = dwHigh;
	ft.dwLowDateTime = dwLow;

	pEnd = (__int64*)(&m_dwFileSize);

	pPos = (__int64*)(&ft);

	return ((*pPos) + (_int64)length) > (*pEnd);
}


////////////////////////////////////////////////////////////////////////////////
// 
// OpenWithID
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::OpenWithID( int nArchiveID, tstring& sArchive )
{

	TCHAR szArchive[ARCHIVES_ARCFILEPATH_LENGTH];
	long szArchiveLen;

	m_query.Initialize();
	BINDPARAM_LONG( m_query, nArchiveID );
	BINDCOL_TCHAR( m_query, szArchive );
	m_query.Execute( _T("SELECT ArcFilePath FROM Archives ")
					 _T("WHERE ArchiveID=?") );

	if( m_query.Fetch() == S_OK )
	{
		sArchive = szArchive;
		return Open( sArchive, true );
	}
	else
	{
		return Arc_ErrFileNotFound;
	}
}



////////////////////////////////////////////////////////////////////////////////
// 
// Close
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::Close(void)
{
	
	if( m_hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hFile );
		m_hFile = INVALID_HANDLE_VALUE;
	}
	
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// UpdateLookupTables
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::UpdateLookupTables(void)
{
	DWORD dwBytes = 0;

	// re-wind the file pointer
	SetFilePointer( m_hFile, 0, 0, FILE_BEGIN );

	if( !WriteFile( m_hFile, &m_hdr, sizeof(m_hdr), &dwBytes, NULL ) )
		return Arc_ErrWritingToFile;

	if( !WriteFile( m_hFile, m_LookupTables, sizeof(m_LookupTables), &dwBytes, NULL ) )
		return Arc_ErrWritingToFile;

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Serialize - Put all records from a table into the archive
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::Serialize( long TableID )
{
	list<ArchiveIndexEntry> IDs;
	ArchiveIndexEntry identry;
	CEMSString sQuery;

	// Initialization
	ZeroMemory( &identry, sizeof(identry) );

	if( TableID == TABLEID_Registry )
	{
		// Only one record
		identry.ID = 1;
		IDs.push_back( identry );
	}
	else
	{
		// Select all IDs from the database
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, identry.ID );

		sQuery.Format( _T("SELECT %s FROM %s ORDER BY %s"),
			           g_ArcTblInfo[TableID].szPrimaryKey,
			           g_ArcTblInfo[TableID].szTableName,
			           g_ArcTblInfo[TableID].szPrimaryKey );

		// Execute the query
		m_query.Execute( sQuery.c_str() );
		
		// Collect the IDs
		while( m_query.Fetch() == S_OK )
		{
			if( identry.ID != 0 )
			{
				IDs.push_back( identry );
			}
		}
	}

	// Call the second form of Serialize
	return Serialize( TableID, IDs );
}



////////////////////////////////////////////////////////////////////////////////
// 
// Serialize - Put selected records from table in archive
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::Serialize( long TableID, list<ArchiveIndexEntry>& IDs )
{
	CEMSString sStatus;
	dca::String sName = g_ArcTblInfo[TableID].szTableName;
	dca::String f;
	dca::String sOp;
	tstring sTemp;

	if( m_OpCode == Arc_Op_Export ){sOp = "Archiving";sTemp.assign(_T("Archiving"));}else{sOp = "Backing up";sTemp.assign(_T("Backing up"));}
	
	f.Format("CArchiveFile::Serialize %s table [%s]",sOp.c_str(),sName.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
	sStatus.Format(_T("%s table [%s]"),sTemp.c_str(),g_ArcTblInfo[TableID].szTableName);
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
	
	ArchiveLookupTable* pLookupTable = m_LookupTables;
	list<ArchiveIndexEntry>::iterator iter;
	DWORD dwBytes = 0;
	int retval = 0;
	int i;
	int nRecords = 0;
	
	// Save the table ID 
	m_TableID = TableID;

	for( i = 0; i < TABLEID_COUNT; i++ )
	{
		if( pLookupTable->TableID == TableID )
		{
			break;
		}
		pLookupTable++;
	}
	
	if( i == TABLEID_COUNT )
	{
		// Error! - ID not found in lookup table
		return Arc_ErrIDNotFound;
	}

	// Serialize all the IDs
	for( iter = IDs.begin(); iter != IDs.end(); iter++ )
	{
		UINT nCancelMaint;
		if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("CancelMaintenance"), nCancelMaint) == ERROR_SUCCESS)
		{
			if(nCancelMaint == 1)
			{
				return Arc_ErrCanceled;
			}
		}
		
		retval = SerializeRecord( *iter );
		if( retval != Arc_Success && retval != Arc_ErrIDNotFound )
		{
			return retval;
		}
		nRecords++;

		sStatus.Format(_T("%s table [%s] record [%d] of [%d]"),sTemp.c_str(),g_ArcTblInfo[TableID].szTableName,nRecords,IDs.size());
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );

	}

	// Update the lookup table
	pLookupTable->RecordCount = IDs.size();
	pLookupTable->OffsetHigh = 0;
	pLookupTable->OffsetLow = SetFilePointer( m_hFile, 0, &pLookupTable->OffsetHigh, FILE_CURRENT );


	// Write out the index table
	for( iter = IDs.begin(); iter != IDs.end(); iter++ )
	{	
		if( !WriteFile( m_hFile, &(iter->ID), sizeof(ArchiveIndexEntry), &dwBytes, NULL ) )
			return Arc_ErrWritingToFile;
	}

	f.Format("CArchiveFile::Serialize Finished %s table [%s], %d records written",sOp.c_str(),sName.c_str(),nRecords);
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
	sStatus.Format(_T("Finished %s table [%s], %d records written."),sTemp.c_str(),g_ArcTblInfo[TableID].szTableName,nRecords);
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
	
	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// SerializeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::SerializeRecord( ArchiveIndexEntry& identry )
{
	DWORD dwBytes = 0;
	int retval = 0;
	
	retval = RecordToBuffer( identry.ID );

	if( retval == Arc_ErrODBC )
	{
		// Try again in case it was a timeout error
		retval = RecordToBuffer( identry.ID );
	}

	if( retval != Arc_Success && retval != Arc_ErrIDNotFound )
		return retval;

	retval = CompressBuffer();
	if( retval != Arc_Success )
		return retval;

	identry.OffsetHigh = 0;
	identry.OffsetLow = SetFilePointer( m_hFile, 0, &identry.OffsetHigh, FILE_CURRENT );
	identry.Length = m_OutBuffUsed; // Length of compressed record

	if( !WriteFile( m_hFile, m_OutBuffer, m_OutBuffUsed, &dwBytes, NULL ) )
		return Arc_ErrWritingToFile;

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// DeSerialize
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::DeSerialize( long TableID, int& nRecords )
{
	CEMSString sStatus;
	dca::String sName = g_ArcTblInfo[TableID].szTableName;
	dca::String f;
	f.Format("CArchiveFile::DeSerialize Restoring table [%s]",sName.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
	sStatus.Format(_T("Restoring table [%s]"),g_ArcTblInfo[TableID].szTableName);
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
	ArchiveLookupTable* pLookupTable = NULL;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int i;
	int retval = 0;

	nRecords = 0;

	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TableID )
		{
			pLookupTable = &m_LookupTables[i];
			break;
		}
	}

	if( pLookupTable == NULL )
		return Arc_ErrIDNotFound;

	m_TableID = TableID;

	Offset.LowPart = pLookupTable->OffsetLow;
	Offset.HighPart = pLookupTable->OffsetHigh;

	for( i = 0; i < pLookupTable->RecordCount; i++ )
	{
		UINT nCancelMaint;
		if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("CancelMaintenance"), nCancelMaint) == ERROR_SUCCESS)
		{
			if(nCancelMaint == 1)
			{
				return Arc_ErrCanceled;
			}
		}		
		
		// Make a copy since SetFilePointer may overwrite the 3rd parameter.
		long dwOffSetHigh = Offset.HighPart;


		if( !PastEndOfFile( dwOffSetHigh, Offset.LowPart, sizeof(identry) ) )
		{
			dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
				
			if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
			{
				return Arc_ErrFileSeek;
			}

			if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				return Arc_ErrReadingFile;


			ZeroMemory(m_OutBuffer, m_OutBuffLen);
			retval = DeSerializeRecord( identry );

			if( retval != Arc_Success && retval != Arc_ErrFileSeek )
				return retval;
		}

		Offset.QuadPart += sizeof(ArchiveIndexEntry);
		nRecords++;
		
		sStatus.Format(_T("Restoring table [%s] record [%d] of [%d]"),g_ArcTblInfo[TableID].szTableName,nRecords,pLookupTable->RecordCount);
		WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
	
	}

	f.Format("CArchiveFile::Finished restoring table [%s], %d records written",sName.c_str(),nRecords);
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
	sStatus.Format(_T("Finished restoring table [%s], %d records written."),g_ArcTblInfo[TableID].szTableName,nRecords);
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("LastMaintStatus"), sStatus.c_str() );
	return Arc_Success;
}

////////////////////////////////////////////////////////////////////////////////
// 
// ArchiveToBuffer
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::ArchiveToBuffer( ArchiveIndexEntry& identry )
{
	DWORD dwTotalRead = 0;
	DWORD dwBytes;
	DWORD dwRet;
	DWORD dwError;
	int retval = 0;

	if( PastEndOfFile( identry.OffsetHigh, identry.OffsetLow, identry.Length ) )
	{
		return Arc_ErrFileSeek;
	}


	// Seek to the record position
	dwRet = SetFilePointer( m_hFile, identry.OffsetLow, &identry.OffsetHigh, FILE_BEGIN );
		
	if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
	{
		return Arc_ErrFileSeek;
	}


	// Make sure our buffer is big enough
	retval = RequireInBufferSize( identry.Length );
	if( retval != Arc_Success )
		return retval;

	// Loop until all data read for the record
	while( dwTotalRead < (DWORD)identry.Length )
	{
		// Read the record into the in buffer
		if( !ReadFile( m_hFile, m_InBuffer + dwTotalRead, 
			           identry.Length - dwTotalRead, &dwBytes, NULL ) )
		{
			return Arc_ErrReadingFile;
		}

		dwTotalRead += dwBytes;
	}


	m_InBuffUsed = identry.Length;

	retval = DecompressBuffer();

	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// DeSerializeRecord
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::DeSerializeRecord( ArchiveIndexEntry& identry )
{
	int retval = Arc_Success;

	if( identry.Length > 0 )
	{
		retval = ArchiveToBuffer( identry );

		if( retval != Arc_Success )
			return retval;

		retval = BufferToRecord( identry.ID );

		if( retval == Arc_ErrODBC )
		{
			// Try again in case it was a timeout error
			retval = BufferToRecord( identry.ID );
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RequireBufferSize - Verify that buffer is appropriate size and re-allocate
//                     if necessary. Returns 0 if buffer is ok, -1 if 
//					   allocation failed.
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RequireBufferSize( unsigned char*& Buffer, long& BuffLen, long nNewSize )
{
	if( nNewSize > BuffLen )
	{
		unsigned char* NewBuffer = new unsigned char[nNewSize];
		ZeroMemory(NewBuffer, nNewSize);

		if( NewBuffer )
		{
			// re-allocate
			if( Buffer )
			{
				CopyMemory( NewBuffer, Buffer, BuffLen );
				delete[] Buffer;
			}
			Buffer = NewBuffer;
			BuffLen = nNewSize;
		}
		else
		{
			return Arc_ErrMemAlloc;
		}
	}

	return ( Buffer == NULL) ? Arc_ErrMemAlloc : Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// CompressBuffer
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::CompressBuffer( void )
{
	int err;

	err = Z_OK;
    avail_in = m_InBuffUsed;
	next_in = m_InBuffer;
    avail_out = m_OutBuffLen;
    next_out = m_OutBuffer;
	total_out = 0;

	// Initialize the compression algorithm
	err = deflateInit( this, Z_DEFAULT_COMPRESSION );
	
	if( err != Z_OK )
	{
		return Arc_ErrDeflate;
	}

    while( 1 )
	{
        err = deflate( this, Z_FINISH );

		if( err == Z_STREAM_END )
		{
			break;
		}
		else if( avail_out == 0 )
		{
			// Double the output buffer size
			long NewBufSize = total_out * 2;

			// Use the input buffer size if larger than the new buffer size
			if( NewBufSize < m_InBuffUsed )
			{
				NewBufSize = m_InBuffUsed;
			}

			// double the size of the output buffer
			if( RequireOutBufferSize( NewBufSize ) )
			{
				deflateEnd( this );
				return Arc_ErrDeflate;
			}

			avail_out = NewBufSize - total_out;
			next_out = m_OutBuffer + total_out;
		}
		else if ( err != S_OK )
		{
			return Arc_ErrDeflate;
		}
    }
	
    err = deflateEnd( this );
	
	if( err != Z_OK )
	{
		return Arc_ErrDeflate;
	}

	// Calculate the size of the compressed record
	m_OutBuffUsed = total_out;

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// DecompressBuffer
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::DecompressBuffer( void )
{
	int err;

	err = Z_OK;
    avail_in = m_InBuffUsed;
	next_in = m_InBuffer;
    avail_out = m_OutBuffLen;
    next_out = m_OutBuffer;
	total_out = 0;

	// Initialize the compression algorithm
	err = inflateInit( this );
	
	if( err != Z_OK )
	{
		return Arc_ErrInflate;
	}

    while( 1 )
	{
        err = inflate( this, Z_FINISH );

		if( err == Z_STREAM_END )
		{
			break;
		}
		else if( avail_out == 0 )
		{
			// Double the output buffer size
			long NewBufSize = total_out * 2;

			// Use the input buffer size if larger than the new buffer size
			if( NewBufSize < m_InBuffUsed )
			{
				NewBufSize = m_InBuffUsed;
			}

			if( RequireOutBufferSize( NewBufSize ) )
			{
				inflateEnd( this );
				return Arc_ErrInflate;
			}

			avail_out = NewBufSize - total_out;
			next_out = m_OutBuffer + total_out;
		}
		else if ( err != S_OK )
		{
			return Arc_ErrInflate;
		}
    }
	
    err = inflateEnd( this );
	
	if( err != Z_OK )
	{
		return Arc_ErrInflate;
	}

	// Calculate the size of the compressed record
	m_OutBuffUsed = total_out;

	return Arc_Success;
}



////////////////////////////////////////////////////////////////////////////////
// 
// RecordToBuffer - must set m_InBuffUsed to length of uncompressed record
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RecordToBuffer( long ID )
{
	int nRet;

	m_InBuffUsed = 0;

//	try
//	{
		nRet = g_ArcTblInfo[m_TableID].StuffFn( *this, ID );
//	}
//	catch( ODBCError_t  )
//	{
//		return Arc_ErrODBC;
//	}
//	catch( CEMSException e )
//	{
//		return Arc_ErrException;
//	}
//	catch( ... )
//	{
//		return Arc_ErrException;
//	}
	return nRet;
}


////////////////////////////////////////////////////////////////////////////////
// 
// AddByteField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::PutByteField( unsigned char value )
{
	// Make sure the buffer has enough room
	if( RequireInBufferSize( m_InBuffUsed + sizeof(unsigned char) ) != Arc_Success )
		return Arc_ErrMemAlloc;

	CopyMemory( m_InBuffer + m_InBuffUsed, &value, sizeof(unsigned char) );
	m_InBuffUsed += sizeof(unsigned char);

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// AddLongField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::PutLongField( int value )
{
	// Make sure the buffer has enough room
	if( RequireInBufferSize( m_InBuffUsed + sizeof(long) ) != Arc_Success )
		return Arc_ErrMemAlloc;

	CopyMemory( m_InBuffer + m_InBuffUsed, &value, sizeof(long) );
	m_InBuffUsed += sizeof(long);

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// PutDateField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::PutDateField( TIMESTAMP_STRUCT& value, long valueLen )
{
	// Make sure the buffer has enough room
	if( RequireInBufferSize( m_InBuffUsed + sizeof(TIMESTAMP_STRUCT) ) != Arc_Success )
		return Arc_ErrMemAlloc;

	if( valueLen == SQL_NULL_DATA )
	{
		ZeroMemory( &value, sizeof(TIMESTAMP_STRUCT) );
	}

	CopyMemory( m_InBuffer + m_InBuffUsed, &value, sizeof(TIMESTAMP_STRUCT) );
	m_InBuffUsed += sizeof(TIMESTAMP_STRUCT);

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// PutCharField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::PutCharField( TCHAR* value )
{
	unsigned int nStrLen = (unsigned int) _tcslen( value ) * sizeof(TCHAR);
	// Make sure the buffer has enough room
	if( RequireInBufferSize( m_InBuffUsed + nStrLen + sizeof(unsigned int) ) != Arc_Success )
		return Arc_ErrMemAlloc;

	// Copy the string length as a 1-byte quantity 
	// (Note that SQL Strings can't be longer than 255)
	CopyMemory( m_InBuffer + m_InBuffUsed, &nStrLen, sizeof(unsigned int) );
	m_InBuffUsed += sizeof(unsigned int);

	// Copy the string itself
	CopyMemory( m_InBuffer + m_InBuffUsed, value, nStrLen );
	m_InBuffUsed += nStrLen;

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// PutTextField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::PutTextField( TCHAR* value, long valueLen )
{
//	valueLen *= sizeof(TCHAR);

	if( valueLen == SQL_NULL_DATA )
	{
		_tcscpy( value, _T("") );
		valueLen = 0;
	}
	
	// Make sure the buffer has enough room
	if( RequireInBufferSize( m_InBuffUsed + valueLen + sizeof(long) ) != Arc_Success )
		return Arc_ErrMemAlloc;

	// Copy the text length as a 4-byte quantity 
	CopyMemory( m_InBuffer + m_InBuffUsed, &valueLen, sizeof(long) );
	m_InBuffUsed += sizeof(long);

	// Copy the text itself
	CopyMemory( m_InBuffer + m_InBuffUsed, value, valueLen );
	m_InBuffUsed += valueLen;

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// PutFileField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::PutFileField( TCHAR* szFilename, bool bIsInbound, bool bIsNote )
{
	HANDLE hFile;
	DWORD dwFileSizeHigh = 0;
	DWORD dwSize = 0;
	DWORD dwRead = 0;
	DWORD dwTotalRead = 0;
	tstring sFullPath;

	// Make sure there is enough room in the buffer for the file length
	if( RequireInBufferSize( m_InBuffUsed + sizeof(DWORD) ) != Arc_Success )
		return Arc_ErrMemAlloc;

	// Generate the full path
	if( bIsNote )
	{
		GetFullNoteAttachPath( szFilename, sFullPath );
	}
	else if( bIsInbound )
	{
		GetFullInboundAttachPath( szFilename, sFullPath );
	}
	else
	{
		GetFullOutboundAttachPath( szFilename, sFullPath );
	}

	// Open the file
	hFile = CreateFile( sFullPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// Write a zero-byte file
		CopyMemory( m_InBuffer + m_InBuffUsed, &dwSize, sizeof(DWORD) );
		m_InBuffUsed += sizeof(DWORD);

		//return Arc_ErrFileNotFound;
		return Arc_Success;
	}

	// get the attachment file size
	dwSize = GetFileSize(hFile, &dwFileSizeHigh);
	if ((dwSize == INVALID_FILE_SIZE) || dwFileSizeHigh)
	{
		// Write a zero-byte file
		CopyMemory( m_InBuffer + m_InBuffUsed, &dwSize, sizeof(DWORD) );
		m_InBuffUsed += sizeof(DWORD);
		CloseHandle( hFile );
		return Arc_ErrFileNotFound;
	}

	// Write the file size to the buffer
	CopyMemory( m_InBuffer + m_InBuffUsed, &dwSize, sizeof(DWORD) );
	m_InBuffUsed += sizeof(DWORD);

	// Make sure the buffer is big enough for the file
	if( RequireInBufferSize( m_InBuffUsed + dwSize ) != Arc_Success )
		return Arc_ErrMemAlloc;

	// Loop until we read all of the file
	while ( dwTotalRead < dwSize )
	{
		// read from original file
		if ( !ReadFile( hFile, m_InBuffer + m_InBuffUsed + dwTotalRead, 
			            dwSize - dwTotalRead, &dwRead, NULL ) )
		{

			CloseHandle(hFile);

			// update the size of the file to zero in the buffer
			dwSize = 0;
			CopyMemory( m_InBuffer + m_InBuffUsed - sizeof(DWORD), &dwSize, sizeof(DWORD) );

			return Arc_ErrReadingFile;
		}

		dwTotalRead += dwRead;
	}

	CloseHandle(hFile);

	// Update the buffer pointer and return
	m_InBuffUsed += dwSize;

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// BufferToRecord - Size of uncompressed record is in m_OutBuffUsed
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::BufferToRecord( long ID )
{
	int nRet;
	CEMSString sErr;

	m_OutBuffPos = 0;

	try
	{
		nRet =  g_ArcTblInfo[m_TableID].RetrieveFn( *this, ID );
	}
	catch( ODBCError_t err )
	{
		sErr.Format( _T("ExportArchive: Caught ODBC err: %s"), err.szErrMsg );
		return Arc_ErrODBC;
	}
	catch( CEMSException e )
	{
		return Arc_ErrException;
	}
	catch( ... )
	{
		return Arc_ErrException;
	}

	return nRet;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetByteField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetByteField( unsigned char& value )
{
	if( m_OutBuffLen < m_OutBuffPos + (long)sizeof(unsigned char) )
	{
		while( m_OutBuffLen < m_OutBuffPos + (long)sizeof(unsigned char) )
		{
			long NewBufSize = m_OutBuffLen * 2;
			if( RequireOutBufferSize( NewBufSize ) )
			{
				return Arc_ErrBufferTooSmall;
			}			
		}
	}

	CopyMemory( &value, m_OutBuffer + m_OutBuffPos, sizeof(unsigned char) );

	m_OutBuffPos += sizeof(unsigned char);

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetLongField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetDecimalField( long& value )
{
	if( m_OutBuffLen < m_OutBuffPos + (long)sizeof(long) )
	{
		while( m_OutBuffLen < m_OutBuffPos + (long)sizeof(long) )
		{
			long NewBufSize = m_OutBuffLen * 2;
			if( RequireOutBufferSize( NewBufSize ) )
			{
				return Arc_ErrBufferTooSmall;
			}
		}
	}
	
	CopyMemory( &value, m_OutBuffer + m_OutBuffPos, sizeof(long) );

	m_OutBuffPos += sizeof(long);

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetLongField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetLongField( int& value )
{
	if( m_OutBuffLen < m_OutBuffPos + (long)sizeof(long) )
	{
		while( m_OutBuffLen < m_OutBuffPos + (long)sizeof(long) )
		{
			long NewBufSize = m_OutBuffLen * 2;
			if( RequireOutBufferSize( NewBufSize ) )
			{
				return Arc_ErrBufferTooSmall;
			}
		}
	}
	
	CopyMemory( &value, m_OutBuffer + m_OutBuffPos, sizeof(long) );

	m_OutBuffPos += sizeof(long);

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetDateField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetDateField( TIMESTAMP_STRUCT& value, long& valueLen )
{
	if( m_OutBuffLen < m_OutBuffPos + (long)sizeof(TIMESTAMP_STRUCT) )
	{
		while( m_OutBuffLen < m_OutBuffPos + (long)sizeof(TIMESTAMP_STRUCT) )
		{
			long NewBufSize = m_OutBuffLen * 2;
			if( RequireOutBufferSize( NewBufSize ) )
			{
				return Arc_ErrBufferTooSmall;
			}
		}
	}
	
	CopyMemory( &value, m_OutBuffer + m_OutBuffPos, sizeof(TIMESTAMP_STRUCT) );
	
	if( value.year == 0 && value.month == 0 && value.day == 0 )
	{
		valueLen = SQL_NULL_DATA;
	}
	else
	{
		valueLen = sizeof(TIMESTAMP_STRUCT);
	}

	m_OutBuffPos += sizeof(TIMESTAMP_STRUCT);

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetCharField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetCharField( TCHAR* value, ULONG maxlen )
{
	DebugReporter::Instance().DisplayMessage("CArchiveFile::GetCharField - Entering", DebugReporter::LIB);
	


	if(m_hdr.Version == 0x05010000)
	{
		int nStrLen = 0;

		if( m_OutBuffLen < m_OutBuffPos + (long)sizeof(int) )
		{
			while( m_OutBuffLen < m_OutBuffPos + (long)sizeof(int) )
			{
				long NewBufSize = m_OutBuffLen * 2;
				if( RequireOutBufferSize( NewBufSize ) )
				{
					return Arc_ErrBufferTooSmall;
				}
			}
		}
		
		// Copy the 1-byte length of the string
		CopyMemory( &nStrLen, m_OutBuffer + m_OutBuffPos, sizeof(int) );
		m_OutBuffPos += sizeof(unsigned int);

		if( m_OutBuffLen < m_OutBuffPos + nStrLen )
		{
			dca::String f;
			f.Format("CArchiveFile::GetCharField - m_OutBuffLen:[%d] m_OutBuffPos:[%d] nStrLen:[%d]", m_OutBuffLen,m_OutBuffPos,nStrLen);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
			DebugReporter::Instance().DisplayMessage("CArchiveFile::GetCharField - m_OutBuffLen is less than m_OutBuffPos - 2", DebugReporter::LIB);
		
			while( m_OutBuffLen < m_OutBuffPos + nStrLen )
			{
				long NewBufSize = m_OutBuffLen * 2;
				if( RequireOutBufferSize( NewBufSize ) )
				{
					DebugReporter::Instance().DisplayMessage("CArchiveFile::GetCharField - BufferTooSmall - 2", DebugReporter::LIB);
		
					return Arc_ErrBufferTooSmall;
				}
			}
		}
		
		// Check the max string length
		if( nStrLen == 0 )
		{
			// Empty string
			value[0] = _T('\0');
		}
		else if( (nStrLen / sizeof(TCHAR)) < maxlen )
		{
			DebugReporter::Instance().DisplayMessage("CArchiveFile::GetCharField - Copy Full String", DebugReporter::LIB);
		
			// Copy the full string 
#ifdef UNICODE
			CopyMemory( value, m_OutBuffer + m_OutBuffPos, nStrLen );
#else
			WideCharToMultiByte( CP_ACP, 0, (LPWSTR)(m_OutBuffer + m_OutBuffPos), nStrLen / 2,
								value, nStrLen / 2, NULL, NULL );
#endif

			value[nStrLen/sizeof(TCHAR)] = _T('\0');
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CArchiveFile::GetCharField - Copy Part Of String", DebugReporter::LIB);
		
			// Copy part of the string 
#ifdef UNICODE
			CopyMemory( value, m_OutBuffer + m_OutBuffPos, maxlen - 1 );
#else
			WideCharToMultiByte( CP_ACP, 0, (LPWSTR)(m_OutBuffer + m_OutBuffPos), maxlen-1,
								value, maxlen-1, NULL, NULL );
#endif
			value[(maxlen-1)/sizeof(TCHAR)] = _T('\0');
		}
		
		m_OutBuffPos += nStrLen;
	}
	else
	{
		unsigned char nStrLen = 0;

		if( m_OutBuffLen < m_OutBuffPos + (long)sizeof(unsigned char) )
			return Arc_ErrBufferTooSmall;

		// Copy the 1-byte length of the string
		CopyMemory( &nStrLen, m_OutBuffer + m_OutBuffPos, sizeof(unsigned char) );
		m_OutBuffPos += sizeof(unsigned char);

		if( m_OutBuffLen < m_OutBuffPos + nStrLen )
		{
			dca::String f;
			f.Format("CArchiveFile::GetCharField - m_OutBuffLen:[%d] m_OutBuffPos:[%d] nStrLen:[%d]", m_OutBuffLen,m_OutBuffPos,nStrLen);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
			DebugReporter::Instance().DisplayMessage("CArchiveFile::GetCharField - m_OutBuffLen is less than m_OutBuffPos - 2", DebugReporter::LIB);
		
			while( m_OutBuffLen < m_OutBuffPos + nStrLen )
			{
				long NewBufSize = m_OutBuffLen * 2;
				if( RequireOutBufferSize( NewBufSize ) )
				{
					DebugReporter::Instance().DisplayMessage("CArchiveFile::GetCharField - BufferTooSmall - 2", DebugReporter::LIB);
		
					return Arc_ErrBufferTooSmall;
				}
			}
		}
		
		// Check the max string length
		if( nStrLen == 0 )
		{
			// Empty string
			value[0] = _T('\0');
		}
		else if( (nStrLen / sizeof(TCHAR)) < maxlen )
		{
			DebugReporter::Instance().DisplayMessage("CArchiveFile::GetCharField - Copy Full String", DebugReporter::LIB);
		
			// Copy the full string 
#ifdef UNICODE
			CopyMemory( value, m_OutBuffer + m_OutBuffPos, nStrLen );
#else
			WideCharToMultiByte( CP_ACP, 0, (LPWSTR)(m_OutBuffer + m_OutBuffPos), nStrLen / 2,
								value, nStrLen / 2, NULL, NULL );
#endif

			value[nStrLen/sizeof(TCHAR)] = _T('\0');
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CArchiveFile::GetCharField - Copy Part Of String", DebugReporter::LIB);
		
			// Copy part of the string 
#ifdef UNICODE
			CopyMemory( value, m_OutBuffer + m_OutBuffPos, maxlen - 1 );
#else
			WideCharToMultiByte( CP_ACP, 0, (LPWSTR)(m_OutBuffer + m_OutBuffPos), maxlen-1,
								value, maxlen-1, NULL, NULL );
#endif
			value[(maxlen-1)/sizeof(TCHAR)] = _T('\0');
		}
		
		m_OutBuffPos += nStrLen;
	}

	DebugReporter::Instance().DisplayMessage("CArchiveFile::GetCharField - Leaving", DebugReporter::LIB);
	
	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetTextField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetTextField( TCHAR*& value, long& allocated, long& len )
{
	long nStrLen = 0;

	if( m_OutBuffLen < m_OutBuffPos + (long)sizeof(long))
	{
		while( m_OutBuffLen < m_OutBuffPos + (long)sizeof(long) )
		{
			long NewBufSize = m_OutBuffLen * 2;
			if( RequireOutBufferSize( NewBufSize ) )
			{
				return Arc_ErrBufferTooSmall;
			}
		}
	}
	
	// Copy the 4-byte length of the text
	CopyMemory( &nStrLen, m_OutBuffer + m_OutBuffPos, sizeof(long) );
	m_OutBuffPos += sizeof(long);
	
	if (allocated > 0) free( value );
	allocated = nStrLen + sizeof(TCHAR);
	value = (TCHAR*)calloc( allocated, 1 );
	len = nStrLen;

#ifdef UNICODE
	CopyMemory( value, m_OutBuffer + m_OutBuffPos, nStrLen );
#else
	WideCharToMultiByte( CP_ACP, 0, (LPWSTR)(m_OutBuffer + m_OutBuffPos), nStrLen / 2,
		                 value, nStrLen / 2, NULL, NULL );
#endif

	// Zero Terminate
	value[nStrLen/sizeof(TCHAR)] = _T('\0');

	m_OutBuffPos += nStrLen;
	
	return Arc_Success;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetFileField
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetFileField( TCHAR* szFilename, bool bIsInbound,
							    long szFilenameLen, TCHAR* szOrigFilename, bool bIsNote )
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwSize = 0;
	int nNameSeq = 1;
	tstring sFullPath;
	DWORD dwWritten;
	DWORD dwTotalWritten = 0;
	tstring sOrigFileName = szOrigFilename;
	

	// Remove bad chars from original filename
	const TCHAR* badchars = _T("\\/:*?\"<>|");
	if (_tcspbrk(sOrigFileName.c_str(), badchars) != NULL)
	{
		// weed out the bad characters
		for (int n = 0; n < (int)sOrigFileName.size(); n++)
		{
			for (int i=0; i < (int)_tcslen(badchars); i++)
			{
				if (sOrigFileName[n] == badchars[i])
					sOrigFileName.erase(n, 1);
			}
		}		
	}	

	// Read out the file length
	if( m_OutBuffLen < m_OutBuffPos + (long)sizeof(DWORD) )
	{
		while( m_OutBuffLen < m_OutBuffPos + (long)sizeof(DWORD) )
		{
			long NewBufSize = m_OutBuffLen * 2;
			if( RequireOutBufferSize( NewBufSize ) )
			{
				return Arc_ErrBufferTooSmall;
			}
		}
	}
	
	CopyMemory( &dwSize, m_OutBuffer + m_OutBuffPos, sizeof(DWORD) );
	m_OutBuffPos += sizeof(DWORD);

	if( dwSize == 0 )
	{
		// No file in buffer (or zero-byte)
		return Arc_Success;
	}
	
	while( hFile == INVALID_HANDLE_VALUE )
	{
		// Generate the full path
		
		if( bIsNote )
		{
			GetFullNoteAttachPath( szFilename, sFullPath );
		}
		else if( bIsInbound )
		{
			GetFullInboundAttachPath( szFilename, sFullPath );
		}
		else
		{
			GetFullOutboundAttachPath( szFilename, sFullPath );
		}
		
		//Strip filename and verify directory exists
		dca::String sPath = sFullPath.c_str();
		int pos = sPath.find_last_of("\\");
		sPath.resize(pos);
		
		if( VerifyDirectory( sPath ) )
		{
			hFile = CreateFile( sFullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, 
								FILE_ATTRIBUTE_NORMAL, NULL );
			
			if( hFile == INVALID_HANDLE_VALUE )
			{
				DWORD dwErr = GetLastError();

				if( dwErr != ERROR_ALREADY_EXISTS && dwErr != ERROR_FILE_EXISTS )
				{
					m_OutBuffPos += dwSize;
					return Arc_ErrCreatingFile;
				}

				_sntprintf(szFilename, szFilenameLen-1, _T("%s(%d)"), sOrigFileName.c_str(), nNameSeq++);
			}
		}
	}

	while( dwTotalWritten < dwSize )
	{
		if( !WriteFile( hFile, m_OutBuffer + m_OutBuffPos + dwTotalWritten, 
			            dwSize - dwTotalWritten, &dwWritten, NULL ) )
		{
			CloseHandle( hFile );
			m_OutBuffPos += dwSize;
			return Arc_ErrWritingToFile;
		}

		dwTotalWritten += dwWritten;
	}

	
	// Close the file and update the buffer pointer
	CloseHandle( hFile );
	m_OutBuffPos += dwSize;

	return Arc_Success;
}




////////////////////////////////////////////////////////////////////////////////
// 
// TranslateID -- If an entry in the map exists for this ID, return it,
//                otherwise return the original ID
// 
////////////////////////////////////////////////////////////////////////////////
void CArchiveFile::TranslateID( int TableID, int& ID )
{
	IDMapIter iter;

	// IDs of zero are ignored
	if( ID == 0 )
		return;

	iter = m_IDMap[TableID].find( ID );

	if( iter != m_IDMap[TableID].end() )
	{
		ID = iter->second;
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// AddIDMapping
// 
////////////////////////////////////////////////////////////////////////////////
void CArchiveFile::AddIDMapping( int TableID, int OriginalID, int NewID )
{
	m_IDMap[TableID][OriginalID] = NewID;
}


////////////////////////////////////////////////////////////////////////////////
// 
// AddActualObjectID
// 
////////////////////////////////////////////////////////////////////////////////
void CArchiveFile::AddActualObjectID( int ObjectID, int ActualID )
{
	m_ActualObjectIDs[ObjectID] = ActualID;
}


////////////////////////////////////////////////////////////////////////////////
// 
// FixupCircularDependencies - There are certain cases where 2 tables each 
// contain a foreign key to the other table. Another case is when a table
// contains an ID of another record in the same table. In these cases, a
// final pass is needed to resolve these IDs.
//
// 	 DefaultSignatureID in Agents
//	 DefaultAgentAddressID in Agents
//	 DefaultEmailAddressID in Contacts
//	 ReplyToMsgID in OutboundMessages AND InboundMessages
//   ActualID in Objects
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::FixupCircularDependencies(void)
{

	IDMapIter iter;
	int ID;
	int ReplyToMsgID,NewReplyToMsgID;
	int DefaultAgentAddressID, NewDefaultAgentAddressID;
	int DefaultSignatureID, NewDefaultSignatureID;
	int StyleSheetID, NewStyleSheetID;
	int DefaultEmailAddressID, NewDefaultEmailAddressID;
	int ActualID;

	unsigned char ReplyToIDIsInbound;

	// Fix up ReplyToMsgID in OutboundMessages
	for( iter = m_IDMap[TABLEID_OutboundMessages].begin(); 
	     iter != m_IDMap[TABLEID_OutboundMessages].end(); iter++ )
	{
		ID = iter->second;

		m_query.Initialize();
		BINDPARAM_LONG( m_query, ID );
		BINDCOL_LONG_NOLEN( m_query, ReplyToMsgID );
		BINDCOL_BIT_NOLEN( m_query, ReplyToIDIsInbound );
		m_query.Execute( _T("SELECT ReplyToMsgID,ReplyToIDIsInbound ") 
			             _T("FROM OutboundMessages ")
						 _T("WHERE OutboundMessageID=?") );

		if( m_query.Fetch() == S_OK )
		{
			if( ReplyToIDIsInbound )
			{
				NewReplyToMsgID = ReplyToMsgID;
				TranslateID( TABLEID_InboundMessages, NewReplyToMsgID );
			}
			else
			{
				NewReplyToMsgID = ReplyToMsgID;
				TranslateID( TABLEID_OutboundMessages, NewReplyToMsgID );
			}
			if( NewReplyToMsgID != ReplyToMsgID )
			{
				m_query.Initialize();
				BINDPARAM_LONG( m_query, NewReplyToMsgID );
				BINDPARAM_LONG( m_query, ID );
				m_query.Execute( _T("UPDATE OutboundMessages ")
					             _T("SET ReplyToMsgID=? ")
								 _T("WHERE OutboundMessageID=?") );
			}
		}
	}

	// Fix up ReplyToMsgID in InboundMessages
	for( iter = m_IDMap[TABLEID_InboundMessages].begin(); 
	     iter != m_IDMap[TABLEID_InboundMessages].end(); iter++ )
	{
		ID = iter->second;

		m_query.Initialize();
		BINDPARAM_LONG( m_query, ID );
		BINDCOL_LONG_NOLEN( m_query, ReplyToMsgID );
		BINDCOL_BIT_NOLEN( m_query, ReplyToIDIsInbound );
		m_query.Execute( _T("SELECT ReplyToMsgID,ReplyToIDIsInbound ") 
			             _T("FROM InboundMessages ")
						 _T("WHERE InboundMessageID=?") );

		if( m_query.Fetch() == S_OK )
		{
			if( ReplyToIDIsInbound )
			{
				NewReplyToMsgID = ReplyToMsgID;
				TranslateID( TABLEID_InboundMessages, NewReplyToMsgID );
			}
			else
			{
				NewReplyToMsgID = ReplyToMsgID;
				TranslateID( TABLEID_OutboundMessages, NewReplyToMsgID );
			}
			if( NewReplyToMsgID != ReplyToMsgID )
			{
				m_query.Initialize();
				BINDPARAM_LONG( m_query, NewReplyToMsgID );
				BINDPARAM_LONG( m_query, ID );
				m_query.Execute( _T("UPDATE InboundMessages ")
					             _T("SET ReplyToMsgID=? ")
								 _T("WHERE InboundMessageID=?") );
			}
		}			 

	}


	// fix up agents table
	for( iter = m_IDMap[TABLEID_Agents].begin(); 
	     iter != m_IDMap[TABLEID_Agents].end(); iter++ )
	{
		ID = iter->second;

		m_query.Initialize();

		BINDPARAM_LONG( m_query, ID );
		BINDCOL_LONG_NOLEN( m_query, DefaultAgentAddressID );
		BINDCOL_LONG_NOLEN( m_query, DefaultSignatureID );
		BINDCOL_LONG_NOLEN( m_query, StyleSheetID );
		m_query.Execute( _T("SELECT DefaultAgentAddressID,DefaultSignatureID,StyleSheetID ") 
			             _T("FROM Agents ")
						 _T("WHERE AgentID=?") );

		if( m_query.Fetch() == S_OK )
		{
			NewDefaultAgentAddressID = DefaultAgentAddressID;
			TranslateID( TABLEID_PersonalData, NewDefaultAgentAddressID );

			NewDefaultSignatureID = DefaultSignatureID;
			TranslateID( TABLEID_Signatures, NewDefaultSignatureID );

			NewStyleSheetID = StyleSheetID;
			TranslateID( TABLEID_StyleSheets, NewStyleSheetID );

			if( ( NewDefaultAgentAddressID != DefaultAgentAddressID )
				|| ( NewDefaultSignatureID != DefaultSignatureID )
				|| ( NewStyleSheetID != StyleSheetID ))
			{
				m_query.Initialize();
				BINDPARAM_LONG( m_query, NewDefaultAgentAddressID );
				BINDPARAM_LONG( m_query, NewDefaultSignatureID );
				BINDPARAM_LONG( m_query, NewStyleSheetID );
				BINDPARAM_LONG( m_query, ID );

				m_query.Execute( _T("UPDATE Agents ")
					             _T("SET DefaultAgentAddressID=?, ")
								 _T("DefaultSignatureID=?, StyleSheetID=? ")
								 _T("WHERE AgentID=?") );
			}
		}
	}

	// Fix up DefaultEmailAddressID in Contacts
	for( iter = m_IDMap[TABLEID_Contacts].begin(); 
	     iter != m_IDMap[TABLEID_Contacts].end(); iter++ )
	{
		ID = iter->second;

		m_query.Initialize();

		BINDPARAM_LONG( m_query, ID );
		BINDCOL_LONG_NOLEN( m_query, DefaultEmailAddressID );
		m_query.Execute( _T("SELECT DefaultEmailAddressID ") 
			             _T("FROM Contacts ")
						 _T("WHERE ContactID=?") );

		if( m_query.Fetch() == S_OK )
		{
			NewDefaultEmailAddressID = DefaultEmailAddressID;
			TranslateID( TABLEID_PersonalData, NewDefaultEmailAddressID );

			if( NewDefaultEmailAddressID != DefaultEmailAddressID )
			{
				m_query.Initialize();
				BINDPARAM_LONG( m_query, NewDefaultEmailAddressID );
				BINDPARAM_LONG( m_query, ID );
				m_query.Execute( _T("UPDATE Contacts ")
					             _T("SET DefaultEmailAddressID=? ")
								 _T("WHERE ContactID=?") );
			}
		}
	}

	// ActualID in Objects
	for( iter = m_ActualObjectIDs.begin(); 
	     iter != m_ActualObjectIDs.end(); iter++ )
	{
		ActualID = iter->second;	// The new Actual ID 
		ID = iter->first;			// The new Object ID
		
		m_query.Initialize();
		BINDPARAM_LONG( m_query, ActualID );
		BINDPARAM_LONG( m_query, ID );
		m_query.Execute( _T("UPDATE Objects ") 
			             _T("SET ActualID=? ")
						 _T("WHERE ObjectID=?") );
	}

	// Fix up ServerParameters
	CEMSString sID;
	TServerParameters SrvParam;

	// default message destination ID
	SrvParam.m_ServerParameterID = EMS_SRVPARAM_DEFAULT_MSGDEST_ID;
	SrvParam.Query( m_query );
	ID = _ttoi(SrvParam.m_DataValue);
	TranslateID( TABLEID_MessageDestinations, ID );
	sID.Format( _T("%d"), ID );
	_tcscpy( SrvParam.m_DataValue, sID.c_str() );
	SrvParam.Update( m_query );

	// default routing rule - match AgentID
	SrvParam.m_ServerParameterID = EMS_SRVPARAM_DEF_RR_MATCH_AGENTID;
	SrvParam.Query( m_query );
	ID = _ttoi(SrvParam.m_DataValue);
	TranslateID( TABLEID_Agents, ID );
	sID.Format( _T("%d"), ID );
	_tcscpy( SrvParam.m_DataValue, sID.c_str() );
	SrvParam.Update( m_query );

	// default routing rule - match TicketBoxID
	SrvParam.m_ServerParameterID = EMS_SRVPARAM_DEF_RR_MATCH_TICKETBOXID;
	SrvParam.Query( m_query );
	ID = _ttoi(SrvParam.m_DataValue);
	TranslateID( TABLEID_TicketBoxes, ID );
	sID.Format( _T("%d"), ID );
	_tcscpy( SrvParam.m_DataValue, sID.c_str() );
	SrvParam.Update( m_query );

	return Arc_Success;
}


////////////////////////////////////////////////////////////////////////////////
// 
// PrintArchiveContents
// 
////////////////////////////////////////////////////////////////////////////////
void PrintArchiveContents( CODBCQuery& query, tstring& sSrcFile, CEMSString& sErr )
{
	CArchiveFile arc(query);

	try
	{
		arc.PrintArchiveContents( sSrcFile );
	}
	catch( ODBCError_t err )
	{
		sErr.Format( _T("PrintArchiveContents:: Caught ODBC err: %s"), err.szErrMsg );
	}
	catch( CEMSException e )
	{
		sErr.Format( _T("PrintArchiveContents:: Caught EMS exception: %s"), e.GetErrorString() );
	}
	catch( ... )
	{
		sErr.assign( _T("PrintArchiveContents:: Caught unhandled exception") );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// PrintArchiveContents
// 
////////////////////////////////////////////////////////////////////////////////
void CArchiveFile::PrintArchiveContents( tstring& sSrcFile )
{
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	int nRet;
	int i,j;
	int totalSize;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;

	_tprintf( _T("Archive File: %s\n"), sSrcFile.c_str() );


	nRet = Open( sSrcFile, true );

	if( nRet != Arc_Success )
	{
		_tprintf( _T("Error opening file\n") );
		return;
	}

	_tprintf( _T("Version: %d.%d.%d.%d\n"), (m_hdr.Version & 0xff000000) >> 24,
											(m_hdr.Version & 0xff0000) >> 16,
											(m_hdr.Version & 0xff00) >> 8,
											(m_hdr.Version & 0xff) );

	_tprintf( _T("Number of tables: %d\n"), m_hdr.LookupTableCount );
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{

		Offset.LowPart = m_LookupTables[i].OffsetLow;
		Offset.HighPart = m_LookupTables[i].OffsetHigh;

		totalSize = 0;

		for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
		{
			// Make a copy since SetFilePointer may overwrite the 3rd parameter.
			long dwOffSetHigh = Offset.HighPart;

			dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
				
			if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
			{
				_tprintf( _T("Error moving file pointer\n") );
				return;
			}

			if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
			{
				_tprintf( _T("Error reading from file\n") );
				return;
			}
			
			Offset.QuadPart += sizeof(ArchiveIndexEntry);

			totalSize += identry.Length;

			_tprintf( _T("\t\tID=%d, Length=%d\n"), identry.ID, identry.Length );
		}

		if( m_LookupTables[i].RecordCount )
		{
			_tprintf( _T("%d %s  #: %d  Size: %d\n"),
				      i, g_ArcTblInfo[m_LookupTables[i].TableID].szTableName,
					  m_LookupTables[i].RecordCount, totalSize );
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// ShrinkDatabase
// 
////////////////////////////////////////////////////////////////////////////////
void CArchiveFile::ShrinkDatabase(void)
{
	TCHAR szDatabaseName[256];
	long szDatabaseNameLen = 0;
	CEMSString sSQL;
	
	m_query.Initialize();
	BINDCOL_TCHAR( m_query, szDatabaseName );
	m_query.Execute( _T("SELECT DB_Name()") );
	m_query.Fetch();

	sSQL.Format( _T("DBCC SHRINKDATABASE([%s])"), szDatabaseName );

	m_query.Initialize();
	m_query.Execute( sSQL.c_str() );

	do
	{
		ODBCError_t err;
		m_query.GetQueryDiagnostics( err );
	} 
	while ( m_query.GetMoreResults() );

}

////////////////////////////////////////////////////////////////////////////////
// 
// GetInboundMessage
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetInboundMessage( InboundMessages_t& msg )
{
	DebugReporter::Instance().DisplayMessage("CArchiveFile::GetInboundMessage - Entering", DebugReporter::LIB);
	
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;

	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_InboundMessages )
		{
			DebugReporter::Instance().DisplayMessage("CArchiveFile::GetInboundMessage - Found InboundMessages Table", DebugReporter::LIB);
	
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				if( identry.ID == msg.m_InboundMessageID )
				{
					DebugReporter::Instance().DisplayMessage("CArchiveFile::GetInboundMessage - Found InboundMessageID, ArchiveToBuffer", DebugReporter::LIB);
	
					retval = ArchiveToBuffer( identry );

					if( retval == Arc_Success )
					{
						DebugReporter::Instance().DisplayMessage("CArchiveFile::GetInboundMessage - RetrieveInboundMessageToStruct", DebugReporter::LIB);
	
						m_OutBuffPos = 0;
						return RetrieveInboundMessageToStruct( *this, msg );
					}
					else
					{
						DebugReporter::Instance().DisplayMessage("CArchiveFile::GetInboundMessage - ArchiveToBuffer Failed", DebugReporter::LIB);
					}
					
					return retval;				
				}
			}
		}
	}

	return Arc_ErrIDNotFound;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetOutboundMessage
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetOutboundMessage( OutboundMessages_t& msg )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;

	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_OutboundMessages )
		{
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				if( identry.ID == msg.m_OutboundMessageID )
				{
					retval = ArchiveToBuffer( identry );

					if( retval == Arc_Success )
					{
						m_OutBuffPos = 0;
						return RetrieveOutboundMessageToStruct( *this, msg );
					}
					
					return retval;				
				}
			}
		}
	}

	return Arc_ErrIDNotFound;
}



////////////////////////////////////////////////////////////////////////////////
// 
// RestoreOutboundMessages
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreOutboundMessages( int TicketID, list<int>& outList )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	OutboundMessages_t om;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_OutboundMessages )
		{
			m_TableID = TABLEID_OutboundMessages;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					om.m_OutboundMessageID = identry.ID;
					RetrieveOutboundMessageToStruct( *this, om );

					if( om.m_TicketID == TicketID)
					{
						retval = DeSerializeRecord( identry );
						if( retval == Arc_Success )
						{
							outList.push_back(om.m_OutboundMessageID);
						}
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreOutboundMessageAttachments
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreOutboundMessageAttachments( int msgID )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	OutboundMessageAttachments_t oma;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_OutboundMessageAttachments )
		{
			m_TableID = TABLEID_OutboundMessageAttachments;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					oma.m_OutboundMessageAttachmentID = identry.ID;
					GetLongField( oma.m_OutboundMessageID );
					
					if( oma.m_OutboundMessageID == msgID)
					{
						retval = DeSerializeRecord( identry );						
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreAttachment
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreAttachment( int attachID )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	Attachments_t ta;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_Attachments )
		{
			m_TableID = TABLEID_Attachments;
			m_bMerge = false;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					ta.m_AttachmentID = identry.ID;
					RetrieveAttachmentToStruct( *this, ta );
					
					if( ta.m_AttachmentID == attachID)
					{
						retval = DeSerializeRecord( identry );
						bool bIsNote = false;
						if ( _tcscmp( ta.m_ContentDisposition, _T("noteattach")) == 0 )
						{
							bIsNote = true;
						}
						retval = GetFileField( ta.m_AttachmentLocation, (ta.m_IsInbound != 0 ? true : false), ATTACHMENTS_ATTACHMENTLOCATION_LENGTH, ta.m_FileName, bIsNote );
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreOutboundMessageContacts
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreOutboundMessageContacts( int msgID )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	OutboundMessageContacts_t omc;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_OutboundMessageContacts )
		{
			m_TableID = TABLEID_OutboundMessageContacts;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					omc.m_OutboundMessageContactID = identry.ID;
					GetLongField( omc.m_ContactID );
					GetLongField( omc.m_OutboundMessageID );					

					if( omc.m_OutboundMessageID == msgID)
					{
						retval = DeSerializeRecord( identry );										
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreInboundMessages
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreInboundMessages( int TicketID, list<int>& inList )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	InboundMessages_t im;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_InboundMessages )
		{
			m_TableID = TABLEID_InboundMessages;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					im.m_InboundMessageID = identry.ID;
					RetrieveInboundMessageToStruct(*this,im);

					if( im.m_TicketID == TicketID)
					{
						retval = DeSerializeRecord( identry );
						if( retval == Arc_Success )
						{
							inList.push_back(im.m_InboundMessageID);
						}
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreInboundMessageAttachments
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreInboundMessageAttachments( int msgID )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	InboundMessageAttachments_t ima;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_InboundMessageAttachments )
		{
			m_TableID = TABLEID_InboundMessageAttachments;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					ima.m_InboundMessageAttachmentID = identry.ID;
					GetLongField( ima.m_InboundMessageID );
					GetLongField( ima.m_AttachmentID );

					if( ima.m_InboundMessageID == msgID)
					{
						retval = DeSerializeRecord( identry );											
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreTicketNotes
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreTicketNotes( int TicketID, list<int>& noteList )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	TicketNotes_t tn;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_TicketNotes )
		{
			m_TableID = TABLEID_TicketNotes;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					tn.m_TicketNoteID = identry.ID;
					GetLongField( tn.m_TicketID );

					if( tn.m_TicketID == TicketID)
					{
						retval = DeSerializeRecord( identry );
						if( retval == Arc_Success )
						{
							noteList.push_back(tn.m_TicketNoteID);
						}
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreNoteAttachments
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreNoteAttachments( int noteID )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	NoteAttachments_t na;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_NoteAttachments )
		{
			m_TableID = TABLEID_NoteAttachments;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					na.m_NoteAttachmentID = identry.ID;
					GetLongField( na.m_NoteID );
					GetLongField( na.m_AttachmentID );

					if( na.m_NoteID == noteID)
					{
						retval = DeSerializeRecord( identry );	
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetNoteAttachments
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetNoteAttachments( int noteID, list<Attachments_t>& attachList )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	TNoteAttachments na;
	Attachments_t att;

	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_NoteAttachments )
		{
			m_TableID = TABLEID_NoteAttachments;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					na.m_NoteAttachmentID = identry.ID;
					GetLongField( na.m_NoteID );
					GetLongField( na.m_AttachmentID );

					if( na.m_NoteID == noteID)
					{
						att.m_AttachmentID = na.m_AttachmentID;
						attachList.push_back( att );
					}
				}
			}
		}
	}

	return GetAttachments( attachList );
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetTicketContacts
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetTicketContacts( int TicketID, list<int>& contactList )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	TicketContacts_t tc;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_TicketContacts )
		{
			m_TableID = TABLEID_TicketContacts;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					tc.m_TicketContactID = identry.ID;
					GetLongField( tc.m_ContactID );
					GetLongField( tc.m_TicketID );
					
					if( tc.m_TicketID == TicketID)
					{
						contactList.push_back(tc.m_ContactID);						
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetOutboundContacts
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetOutboundContacts( int msgID, list<int>& contactList )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	OutboundMessageContacts_t omc;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_OutboundMessageContacts )
		{
			m_TableID = TABLEID_OutboundMessageContacts;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					omc.m_OutboundMessageContactID = identry.ID;
					GetLongField( omc.m_ContactID );
					GetLongField( omc.m_OutboundMessageID );
					
					if( omc.m_OutboundMessageID == msgID)
					{
						contactList.push_back( omc.m_ContactID );									
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreTicketContacts
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreTicketContacts( int TicketID )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	TicketContacts_t tc;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_TicketContacts )
		{
			m_TableID = TABLEID_TicketContacts;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					tc.m_TicketContactID = identry.ID;
					GetLongField( tc.m_TicketID );
					
					if( tc.m_TicketID == TicketID)
					{
						retval = DeSerializeRecord( identry );										
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreContact
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreContact( int contactID )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	Contacts_t tc;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_Contacts )
		{
			m_TableID = TABLEID_Contacts;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					tc.m_ContactID = identry.ID;
										
					if( tc.m_ContactID == contactID)
					{
						retval = DeSerializeRecord( identry );											
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestorePersonalData
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestorePersonalData( int contactID )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	PersonalData_t pd;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_PersonalData )
		{
			m_TableID = TABLEID_PersonalData;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					pd.m_PersonalDataID = identry.ID;
					GetLongField( pd.m_ContactID );
										
					if( pd.m_ContactID == contactID)
					{
						retval = DeSerializeRecord( identry );										
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreTicketHistory
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::RestoreTicketHistory( int TicketID )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	TicketHistory_t th;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_TicketHistory )
		{
			m_TableID = TABLEID_TicketHistory;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					th.m_TicketHistoryID = identry.ID;
					GetLongField( th.m_TicketID );
										
					if( th.m_TicketID == TicketID)
					{
						retval = DeSerializeRecord( identry );										
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetTicket
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetTicket( Tickets_t& tkt )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	//dca::String f;
				
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_Tickets )
		{
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				//f.Format("CArchiveFile::GetTicket - Found TicketID:[%d]", identry.ID);
				//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
				
				if( identry.ID == tkt.m_TicketID )
				{
					retval = ArchiveToBuffer( identry );

					if( retval == Arc_Success )
					{
						m_OutBuffPos = 0;
						return RetrieveTicketToStruct( *this, tkt );
					}
					
					return retval;				
				}
			}
		}
	}

	return Arc_ErrIDNotFound;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetOutboundAttachments
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetOutboundAttachments( int msgID, list<Attachments_t>& attachList )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	TOutboundMessageAttachments oma;
	Attachments_t att;

	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_OutboundMessageAttachments )
		{
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					GetLongField( oma.m_OutboundMessageID );
					GetLongField( oma.m_AttachmentID );

					if( oma.m_OutboundMessageID == msgID)
					{
						att.m_AttachmentID = oma.m_AttachmentID;
						attachList.push_back( att );
					}
				}
			}
		}
	}

	return GetAttachments( attachList );
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetInboundAttachments
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetInboundAttachments( int msgID, list<Attachments_t>& attachList )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	TInboundMessageAttachments ima;
	Attachments_t att;

	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_InboundMessageAttachments )
		{
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					GetLongField( ima.m_InboundMessageID );
					GetLongField( ima.m_AttachmentID );

					if( ima.m_InboundMessageID == msgID)
					{
						att.m_AttachmentID = ima.m_AttachmentID;
						attachList.push_back( att );
					}
				}
			}
		}
	}

	return GetAttachments( attachList );
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetAttachments
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetAttachments( list<Attachments_t>& attachList )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	list<Attachments_t>::iterator iter;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_Attachments )
		{
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);

				for( iter = attachList.begin(); iter != attachList.end(); iter++ )
				{
					if( identry.ID == iter->m_AttachmentID )
					{
						retval = ArchiveToBuffer( identry );

						if( retval == Arc_Success )
						{
							m_OutBuffPos = 0;
							RetrieveAttachmentToStruct( *this, *iter ); 
						}
					}
				}
			}
		}
	}

	return Arc_Success;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetAttachmentToTempFile
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetAttachmentToTempFile( Attachments_t& att, tstring& sTempFile, 
										   int nMatchType, int nIsInbound, 
										   tstring& sContentID, tstring& sFileName )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	bool bMatch;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_Attachments )
		{
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);
				
				// We have enough info for nMatchType=0
				bMatch = ( nMatchType == 0 ) ? (identry.ID == att.m_AttachmentID) : true;

				if( bMatch )
				{
					retval = ArchiveToBuffer( identry );

					if( retval == Arc_Success )
					{
						m_OutBuffPos = 0;
						RetrieveAttachmentToStruct( *this, att ); 
					
						if(    ( nMatchType == 0 ) 
							|| (( nMatchType == 1 ) && (sContentID.compare(att.m_ContentID) == 0))
							|| (( nMatchType == 2 ) && (sFileName.compare(att.m_FileName) == 0)) )
						{
							
							bool bIsNote = false;
							if ( _tcscmp( att.m_ContentDisposition, _T("noteattach")) == 0 )
							{
								bIsNote = true;
							}
								
							retval = GetFileField( att.m_AttachmentLocation, (att.m_IsInbound != 0 ? true: false),
												   ATTACHMENTS_ATTACHMENTLOCATION_LENGTH, att.m_FileName, bIsNote );

							if( retval == Arc_Success )
							{
								
								if ( bIsNote )
								{
									GetFullNoteAttachPath( att.m_AttachmentLocation, sTempFile );
								}
								else if( att.m_IsInbound > 0 )
								{
									GetFullInboundAttachPath( att.m_AttachmentLocation, sTempFile );
								}
								else
								{
									GetFullOutboundAttachPath( att.m_AttachmentLocation, sTempFile );
								}
							}

							return retval;
						}
					}
					else
					{
						return retval;
					}
				}
			}
		}
	}

	return Arc_ErrIDNotFound;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetInboundMessages
// 
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::GetInboundMessages( list<InboundMessages_t>& msgList )
{
	int i,j;
	ArchiveIndexEntry identry;
	LARGE_INTEGER Offset;
	DWORD dwRet;
	DWORD dwError;
	DWORD dwBytes;
	int retval = 0;
	InboundMessages_t msg;
	
	for( i = 0; i < m_hdr.LookupTableCount; i++ )
	{
		if( m_LookupTables[i].TableID == TABLEID_InboundMessages )
		{
			m_TableID = TABLEID_InboundMessages;
			Offset.LowPart = m_LookupTables[i].OffsetLow;
			Offset.HighPart = m_LookupTables[i].OffsetHigh;

			for( j = 0; j < m_LookupTables[i].RecordCount; j++ )
			{
				// Make a copy since SetFilePointer may overwrite the 3rd parameter.
				long dwOffSetHigh = Offset.HighPart;

				dwRet = SetFilePointer( m_hFile, Offset.LowPart, &dwOffSetHigh, FILE_BEGIN );
					
				if( dwRet == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != NO_ERROR )
				{
					return Arc_ErrFileSeek;
				}

				if( !ReadFile( m_hFile, &identry, sizeof(identry), &dwBytes, NULL ) )
				{
					return Arc_ErrFileSeek;
				}
				
				Offset.QuadPart += sizeof(ArchiveIndexEntry);
				
				msg.m_InboundMessageID = identry.ID;
				retval = ArchiveToBuffer( identry );

				if( retval == Arc_Success )
				{
					m_OutBuffPos = 0;
					retval = RetrieveInboundMessageToStruct( *this, msg );
					
					if( retval == Arc_Success)
					{
						//msg.m_EmailFrom
						//msg.m_EmailTo
						//msg.m_EmailCc
						//msg.m_Subject
						//msg.m_Body
						//msg.m_PopHeaders
						
						
						if(_tcscmp( msg.m_EmailFrom, _T("email@domain.com") ) == 0 )
						{
							msgList.push_back( msg );									
						}
					}
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetInboundMessageFromArchive
// 
////////////////////////////////////////////////////////////////////////////////
int GetInboundMessageFromArchive( CODBCQuery& query, int nArchiveID,
								  InboundMessages_t& msg, Tickets_t& tkt,
								  list<Attachments_t>& attachList, tstring& sArchive )
{
	CArchiveFile arc(query);
	int retval = 0;
	
	DebugReporter::Instance().DisplayMessage("GetInboundMessageFromArchive - Open Archive File", DebugReporter::LIB);
	
	retval = arc.OpenWithID( nArchiveID, sArchive );

	if( retval == Arc_Success )
	{
		DebugReporter::Instance().DisplayMessage("GetInboundMessageFromArchive - Get InboundMessage From Archive", DebugReporter::LIB);
	
		retval = arc.GetInboundMessage( msg );

		if( retval == Arc_Success )
		{
			tkt.m_TicketID = msg.m_TicketID;

			DebugReporter::Instance().DisplayMessage("GetInboundMessageFromArchive - Get Ticket From Archive", DebugReporter::LIB);
	
			retval = arc.GetTicket( tkt );

			if( retval == Arc_Success )
			{
				DebugReporter::Instance().DisplayMessage("GetInboundMessageFromArchive - Get Inbound Message Attachments", DebugReporter::LIB);
	
				retval = arc.GetInboundAttachments( msg.m_InboundMessageID, attachList );

				if(retval != Arc_Success)
				{
					DebugReporter::Instance().DisplayMessage("GetInboundMessageFromArchive - Get Inbound Message Attachments Failed", DebugReporter::LIB);
				}
			}
			else
			{
				DebugReporter::Instance().DisplayMessage("GetInboundMessageFromArchive - Get Ticket From Archive Failed", DebugReporter::LIB);
			}
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("GetInboundMessageFromArchive - Get InboundMessage From Archive Failed", DebugReporter::LIB);
		}
	}
	else
	{
		DebugReporter::Instance().DisplayMessage("GetInboundMessageFromArchive - Open Archive File Failed", DebugReporter::LIB);
	}
	
	msg.m_SubjectLen = _tcslen( msg.m_Subject );
	tkt.m_SubjectLen = _tcslen( tkt.m_Subject );
	tkt.m_ContactsLen = _tcslen( tkt.m_Contacts );

	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetOutboundMessageFromArchive
// 
////////////////////////////////////////////////////////////////////////////////
int GetOutboundMessageFromArchive( CODBCQuery& query, int nArchiveID, 
								   OutboundMessages_t& msg, Tickets_t& tkt,
								   list<Attachments_t>& attachList, tstring& sArchive )
{
	CArchiveFile arc(query);
	int retval = 0;

	retval = arc.OpenWithID( nArchiveID, sArchive );

	if( retval == Arc_Success )
	{
		retval = arc.GetOutboundMessage( msg );

		if( retval == Arc_Success )
		{
			tkt.m_TicketID = msg.m_TicketID;

			retval = arc.GetTicket( tkt );

			arc.GetOutboundAttachments( msg.m_OutboundMessageID, attachList );
		}
	}

	msg.m_SubjectLen = _tcslen( msg.m_Subject );
	tkt.m_SubjectLen = _tcslen( tkt.m_Subject );
	tkt.m_ContactsLen = _tcslen( tkt.m_Contacts );
	
	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetAttachmentFromArchive
//
//  nMatchType = 0 -> Use AttachmentID
//  nMatchType = 1 -> Use ContentID
//  nMatchType = 2 -> Use FileName
// 
////////////////////////////////////////////////////////////////////////////////
int GetAttachmentFromArchive( CODBCQuery& query, int nArchiveID, int nMatchType,
						      int AttachmentID, int nIsInbound, tstring& sContentID,
							  tstring& sTempFile, tstring& sFileName, 
							  tstring& sMediaType, tstring& sArchive )
{
	CArchiveFile arc(query);
	Attachments_t att;
	int retval = 0;

	retval = arc.OpenWithID( nArchiveID, sArchive );

	if( retval == Arc_Success )
	{
		att.m_AttachmentID = AttachmentID;

		retval = arc.GetAttachmentToTempFile( att, sTempFile, nMatchType, 
			                                  nIsInbound, sContentID, sFileName );

		
		if( retval == Arc_Success )
		{
			sFileName.assign( att.m_FileName );
			sMediaType.assign( att.m_MediaType );
			sMediaType.append( _T("/") );
			sMediaType.append( att.m_MediaSubType );
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetTicketFromArchive
// 
////////////////////////////////////////////////////////////////////////////////
int GetTicketFromArchive( CODBCQuery& query, int nArchiveID, Tickets_t& tkt, tstring& sArchive )
{
	CArchiveFile arc(query);
	int retval = 0;

	retval = arc.OpenWithID( nArchiveID, sArchive );

	if( retval == Arc_Success )
	{
		retval = arc.GetTicket( tkt );
	}
	
	tkt.m_SubjectLen = _tcslen( tkt.m_Subject );
	tkt.m_ContactsLen = _tcslen( tkt.m_Contacts );

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetInboundMessageListFromArchive
// 
////////////////////////////////////////////////////////////////////////////////
int GetInboundMessageListFromArchive( CODBCQuery& query, int nArchiveID, 
								  list<InboundMessages_t>& msgList, tstring& sArchive )
{
	CArchiveFile arc(query);
	int retval = 0;

	retval = arc.OpenWithID( nArchiveID, sArchive );

	if( retval == Arc_Success )
	{
		retval = arc.GetInboundMessages(msgList);
	}	

	return retval;
}


////////////////////////////////////////////////////////////////////////////////
// 
// RestoreAttachmentFromArchive
// 
////////////////////////////////////////////////////////////////////////////////
int RestoreAttachmentFromArchive( CODBCQuery& query, int nArchiveID, int nMatchType,
								  int& AttachmentID, tstring& sContentID )
{
	CArchiveFile arc(query);
	Attachments_t att;
	int retval = 0;
	tstring sArchive;
	tstring sTempFile;
	int nIsInbound = 0;
	tstring sFileName;

	retval = arc.OpenWithID( nArchiveID, sArchive );

	if( retval == Arc_Success )
	{
		att.m_AttachmentID = AttachmentID;

		retval = arc.GetAttachmentToTempFile( att, sTempFile, nMatchType, 
			                                  nIsInbound, sContentID, sFileName );

		
		if( retval == Arc_Success )
		{
			query.Initialize();
			BINDPARAM_TCHAR( query, att.m_AttachmentLocation );
			BINDPARAM_TCHAR( query, att.m_MediaType );
			BINDPARAM_TCHAR( query, att.m_MediaSubType );
			BINDPARAM_TCHAR( query, att.m_ContentDisposition );
			BINDPARAM_TCHAR( query, att.m_FileName );
			BINDPARAM_LONG( query, att.m_FileSize );
			BINDPARAM_LONG( query, att.m_VirusScanStateID );
			BINDPARAM_TCHAR( query, att.m_VirusName );
			BINDPARAM_TCHAR( query, att.m_ContentID );
			BINDPARAM_BIT( query, att.m_IsInbound );
			query.Execute( _T("INSERT INTO Attachments ")
						   _T("(AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound) ")
						   _T("VALUES")
						   _T("(?,?,?,?,?,?,?,?,?,?)") );

			AttachmentID = query.GetLastInsertedID();
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreTicketObjectsFromArchive
// 
////////////////////////////////////////////////////////////////////////////////
int RestoreTicketObjectsFromArchive( CODBCQuery& query, int nArchiveID, int nTicketID )
{
	CArchiveFile arc(query);
	int retval = 0;
	tstring sArchive;

	list<int> outList;	
	list<int> inList;
	list<int> noteList;

	list<Attachments_t> attList;
	
	list<int> contactList;
			
	list<int> ticketHistoryIDs;

	list<int>::iterator iter;
	list<int>::iterator iter2;
	list<Attachments_t>::iterator iter3;

	retval = arc.OpenWithID( nArchiveID, sArchive );

	arc.GetTicketContacts( nTicketID, contactList );
	for( iter2 = contactList.begin(); iter2 != contactList.end(); iter2++ )
	{
		arc.RestoreContact( *iter2 );
		arc.RestorePersonalData( *iter2 );
	}
	arc.RestoreTicketContacts(nTicketID);	
	if( retval == Arc_Success )
	{
		retval = arc.RestoreOutboundMessages(nTicketID, outList);
		if( retval == Arc_Success )
		{
			for( iter = outList.begin(); iter != outList.end(); iter++ )
			{
				attList.clear();
				arc.GetOutboundAttachments( *iter, attList );
				for( iter3 = attList.begin(); iter3 != attList.end(); iter3++ )
				{
					arc.RestoreAttachment( iter3->m_AttachmentID );
				}
				arc.RestoreOutboundMessageAttachments( *iter );
				
				contactList.clear();
				arc.GetOutboundContacts(*iter, contactList);
				for( iter2 = contactList.begin(); iter2 != contactList.end(); iter2++ )
				{
					arc.RestoreContact( *iter2 );
					arc.RestorePersonalData( *iter2 );
				}
				arc.RestoreOutboundMessageContacts(*iter);				
			}			
		}
		retval = arc.RestoreInboundMessages(nTicketID, inList);
		if( retval == Arc_Success )
		{
			for( iter = inList.begin(); iter != inList.end(); iter++ )
			{
				attList.clear();
				arc.GetInboundAttachments( *iter, attList );
				for( iter3 = attList.begin(); iter3 != attList.end(); iter3++ )
				{
					arc.RestoreAttachment( iter3->m_AttachmentID );
				}
				arc.RestoreInboundMessageAttachments( *iter);
			}
		}
		retval = arc.RestoreTicketNotes(nTicketID, noteList);
		if( retval == Arc_Success )
		{
			for( iter = noteList.begin(); iter != noteList.end(); iter++ )
			{
				attList.clear();
				arc.GetNoteAttachments( *iter, attList );
				for( iter3 = attList.begin(); iter3 != attList.end(); iter3++ )
				{
					arc.RestoreAttachment( iter3->m_AttachmentID );
				}
				arc.RestoreNoteAttachments( *iter );
			}
		}
		retval = arc.RestoreTicketHistory(nTicketID);
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////////////
// 
// VerifyDirectory
// 
////////////////////////////////////////////////////////////////////////////////
bool VerifyDirectory( dca::String& sPath )
{
	// ensure that the path exists
	DWORD attr;
	int pos;
	bool bRes = true;
	
	// Check for trailing slash:
	pos = sPath.find_last_of("\\");
	if (sPath.length() == pos + 1)	// last character is "\"
	{
		sPath.resize(pos);
	}
	
	// look for existing path part
	attr = GetFileAttributesA(sPath.c_str());
	
	// if it doesn't exist
	if (0xFFFFFFFF == attr)
	{
		pos = sPath.find_last_of("\\");
		if (0 < pos)
		{
			// create parent dirs
			dca::String p = sPath.substr(0, pos);
			bRes = VerifyDirectory(p);
		}
		
		// create note
		dca::WString path(sPath.c_str());
		bRes = bRes && CreateDirectoryW(path.c_str(), NULL);
	}
	else if (!(FILE_ATTRIBUTE_DIRECTORY & attr))
	{	
		bRes = false;
	}
	
	return bRes;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DeleteDirectory
// 
////////////////////////////////////////////////////////////////////////////////
bool DeleteDirectory(const TCHAR* sPath)
{
    HANDLE hFind;  // file handle

    WIN32_FIND_DATA FindFileData;

    TCHAR DirPath[MAX_PATH];
    TCHAR FileName[MAX_PATH];

    _tcscpy(DirPath,sPath);
    _tcscat(DirPath,_T("\\*"));    // searching all files

    _tcscpy(FileName,sPath);
    _tcscat(FileName,_T("\\"));

    hFind = FindFirstFile(DirPath,&FindFileData); // find the first file
    if(hFind == INVALID_HANDLE_VALUE) return FALSE;
    _tcscpy(DirPath,FileName);
        
    bool bSearch = true;
    while(bSearch) { // until we finds an entry
        if(FindNextFile(hFind,&FindFileData)) {
            if(IsDots(FindFileData.cFileName)) continue;
            _tcscat(FileName,FindFileData.cFileName);
            if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                // we have found a directory, recurse
                if(!DeleteDirectory(FileName)) { 
                    FindClose(hFind); 
                    return FALSE; // directory couldn't be deleted
                }
                RemoveDirectory(FileName); // remove the empty directory
                _tcscpy(FileName,DirPath);
            }
            else {
                //if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                    //_chmod(FileName, _S_IWRITE); // change read-only file mode
                if(!DeleteFile(FileName)) {  // delete the file
                    FindClose(hFind); 
                    return FALSE; 
                }                 
                _tcscpy(FileName,DirPath);
            }
        }
        else {
            if(GetLastError() == ERROR_NO_MORE_FILES) // no more files there
            bSearch = false;
            else {
                // some error occured, close the handle and return FALSE
                FindClose(hFind); 
                return FALSE;
            }

        }

    }
    FindClose(hFind);  // closing file handle
 
	return TRUE;
    //return RemoveDirectory(sPath); // remove the empty directory

}


bool IsDots(const TCHAR* str)
{
    if(_tcscmp(str,_T(".")) && _tcscmp(str,_T(".."))) return FALSE;
    return TRUE;
}