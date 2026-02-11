/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/ArchiveFns.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Archive Functions
||              
\\*************************************************************************/

// only include once
#pragma once


#include "ArchiveTables.h"

// The functions exported from this module:

int ExportArchive( CODBCQuery& query, tstring& sDestFile, int nDays, 
				   bool bRemoveTickets, bool bKeepTicketSummary, CEMSString& sErr, CEMSString& sTicketIDs, int& nNumTickets, bool& bDoMore, int m_nArchiveMaxTickets, bool m_bArchiveAllowAccess = false );

int ImportArchive( CODBCQuery& query, tstring& sSrcFile, 
				   bool bReplaceDuplicates, CEMSString& sErr );

int Backup( CODBCQuery& query, tstring& sDestFile, CEMSString& sErr );

int Restore( CODBCQuery& query, tstring& sSrcFile, bool bEraseFirst, 
			  bool bReplaceDuplicates, bool bReplaceConfig, CEMSString& sErr );

void PrintArchiveContents( CODBCQuery& query, tstring& sSrcFile, CEMSString& sErr );

int GetInboundMessageFromArchive( CODBCQuery& query, int nArchiveID, 
								  InboundMessages_t& msg, Tickets_t& tkt,
								  list<Attachments_t>& attachList,
								  tstring& sArchive );

int GetOutboundMessageFromArchive( CODBCQuery& query, int nArchiveID, 
								   OutboundMessages_t& msg, Tickets_t& tkt,
								   list<Attachments_t>& attachList,
								   tstring& sArchive );


int GetAttachmentFromArchive( CODBCQuery& query, int nArchiveID, int nMatchType,
							  int AttachmentID, int nIsInbound, tstring& sContentID,
							  tstring& sTempFile, tstring& sFileName, 
							  tstring& sMediaType, tstring& sArchive );

int GetTicketFromArchive( CODBCQuery& query, int nArchiveID, 
								  Tickets_t& tkt, tstring& sArchive );

int GetInboundMessageListFromArchive( CODBCQuery& query, int nArchiveID, 
								  list<InboundMessages_t>& msgList, tstring& sArchive );

int RestoreAttachmentFromArchive( CODBCQuery& query, int nArchiveID, int nMatchType,
								  int& AttachmentID, tstring& sContentID );

int RestoreTicketObjectsFromArchive( CODBCQuery& query, int nArchiveID, int nTicketID );

bool VerifyDirectory( dca::String& sPath );
bool DeleteDirectory(const TCHAR* sPath);
bool IsDots(const TCHAR* str);

// this is required for proper linking with the zlib C modules
extern "C" 
{
	#include "zlib/zlib.h"
}

// NOTE! This code is in development and not compiled in the library yet!

// The archive begins with an ArchiveHeader
typedef struct ArchiveHeader
{
	long MagicWord;					// 0xDFAC001  
	long Version;					// Version of archive format
	long LookupTableCount;			// Number of ArchiveLookupTables following this block
} ArchiveHeader;

// The ArchiveHeader is followed by an array of ArchiveLookupTables
typedef struct ArchiveLookupTable
{
	long TableID;					// Identifies the table
	long OffsetHigh;					// Offset to first ArchiveIndexEntry
	long OffsetLow;
	long RecordCount;				// Number of ArchiveIndexEntries in index
} ArchiveLookupTable;

// The Archive Indexes are found from the ArchiveLookupTable
typedef struct ArchiveIndexEntry
{
	long ID;							// ID of record in DB
	long OffsetHigh;					// Offset to start of compresssed record
	long OffsetLow;
	long Length;						// Length of compressed record
} IDIndexEntry;

enum ArchiveErrorCodes		// Error codes
{ 
	Arc_Success,			// 0
	Arc_ErrWritingToFile,	// 1
	Arc_ErrReadingFile,		// 2
	Arc_ErrMemAlloc,		// 3
	Arc_ErrDeflate,			// 4
	Arc_ErrInflate,			// 5
	Arc_ErrIDNotFound,		// 6
	Arc_ErrNotArchive,		// 7
	Arc_ErrOpenFile,		// 8
	Arc_ErrFileSeek,		// 9
	Arc_ErrBufferTooSmall,	// 10
	Arc_ErrFileNotFound,	// 11
	Arc_ErrCreatingFile,	// 12
	Arc_NoTicketsFound,		// 13 -  No tickets to archive
	Arc_ErrODBC,			// 14
	Arc_ErrException,		// 15
	Arc_ErrTruncation,		// 16
	Arc_ErrCanceled			// 17
};

enum ArchiveOperation		// Op codes
{
	Arc_Op_Import,
	Arc_Op_Export,
	Arc_Op_Backup,
	Arc_Op_Restore
};


typedef map<int,int> IDMap;
typedef map<int,int>::iterator IDMapIter;

////////////////////////////////////////////////////////////////////////////////
// 
// CArchiveFile class definition
// 
////////////////////////////////////////////////////////////////////////////////
class CArchiveFile : public z_stream 
{
  public:
	CArchiveFile(CODBCQuery& query);
	~CArchiveFile();
	
	int ImportArchive( tstring& sSrcFile );
	int ExportArchive( tstring& sDestFile, int nDays, 
		               bool bRemoveTickets, bool bKeepTicketSummary, CEMSString& sTicketIDs, int& nNumTickets, bool& bDoMore, int m_nArchiveMaxTickets, bool m_bArchiveAllowAccess = false );
	int Backup( tstring& sDestFile );
	int Restore( tstring& sSrcFile, bool bEraseFirst );

	void PrintArchiveContents( tstring& sSrcFile );

	int OpenWithID( int nArchiveID, tstring& sArchive );
	int GetInboundMessage( InboundMessages_t& msg );
	int GetOutboundMessage( OutboundMessages_t& msg );
	int	RestoreAttachment( int attachID );
	int RestoreOutboundMessages( int TicketID, list<int>& outList );
	int RestoreOutboundMessageAttachments( int msgID );
	int RestoreOutboundMessageContacts( int msgID );
	int RestoreInboundMessages( int TicketID, list<int>& inList );
	int RestoreInboundMessageAttachments( int msgID );
	int RestoreTicketNotes( int TicketID, list<int>& noteList );
	int RestoreNoteAttachments( int noteID );
	int RestoreTicketContacts( int TicketID );
	int RestoreContact( int contactID );
	int RestorePersonalData( int contactID );
	int RestoreTicketHistory( int TicketID );
	int GetTicket( Tickets_t& tkt );
	int GetTicketContacts( int TicketID, list<int>& contactList );
	int GetOutboundContacts( int msgID, list<int>& contactList );
	int GetOutboundAttachments( int msgID, list<Attachments_t>& attachList );
	int GetInboundAttachments( int msgID, list<Attachments_t>& attachList );
	int GetNoteAttachments( int noteID, list<Attachments_t>& attachList );
	int GetAttachments( list<Attachments_t>& attachList );
	int GetInboundMessages( list<InboundMessages_t>& msgList );
	int GetAttachmentToTempFile( Attachments_t& att, tstring& sTempFile, int nMatchType,
		                         int nIsInbound, tstring& sContentID, tstring& sFileName );

	CODBCQuery&	GetQuery(void) { return m_query; }

	// Generic		Database -> Buffer conversion functions
	int PutByteField( unsigned char value );
	int PutLongField( int value );
	int PutDateField( TIMESTAMP_STRUCT& value, long valueLen );
	int PutCharField( TCHAR* value );
	int PutTextField( TCHAR* value, long valueLen );
	int PutFileField( TCHAR* szFilename, bool bIsInbound, bool bIsNote );

	// Generic		Buffer -> Database conversion functions
	int GetByteField( unsigned char& value );
	int GetDecimalField( long& value );
	int GetLongField( int& value );
	int GetDateField( TIMESTAMP_STRUCT& value, long &valueLen );
	int GetCharField( TCHAR* value, ULONG maxlen );
	int GetTextField( TCHAR*& value, long& allocated, long& len );
	int GetFileField( TCHAR* szFilename, bool bIsInbound, long szFilenameLen, TCHAR* szOrigFilename, bool bIsNote );

	// ID translation functions
	void AddIDMapping( int TableID, int OriginalID, int NewID );
	void TranslateID( int TableID, int& ID );

	void AddActualObjectID( int ObjectID, int ActualID );

	bool& Merge(void) { return m_bMerge; }
	bool Overwrite(void) { return !m_bMerge; }

	bool& ReplaceConfig(void) { return m_bReplaceConfig; }

	bool IsOpen(void) { return m_hFile != INVALID_HANDLE_VALUE;	}
	int Close(void);

	void ShrinkDatabase(void);

	int& OpCode(void) { return m_OpCode; }

	enum { ARCHIVE_BUFFER_SEED = 16384 };
	
	ArchiveHeader& GetHdr(void) { return m_hdr;	}

 protected:

	int Open( tstring& FileName, bool bReadOnly );
	int UpdateLookupTables(void);

	// Serialize a select list of IDs
	int Serialize( long TableID, list<ArchiveIndexEntry>& IDs );

	// Serialize all records from this table found in the database
	int Serialize( long TableID );

	// DeSerialize all records from this table found in the archive
	int DeSerialize( long TableID, int& nRecords );

	int FixupCircularDependencies(void);

	int SerializeRecord( ArchiveIndexEntry& identry );
	int DeSerializeRecord( ArchiveIndexEntry& identry );

	int RecordToBuffer( long ID );
	int CompressBuffer( void );

	int BufferToRecord( long ID );
  	int DecompressBuffer( void );
	
	int RequireOutBufferSize( long nBytes ) 
		{ return RequireBufferSize( m_OutBuffer, m_OutBuffLen, nBytes ); }

 	int RequireInBufferSize( long nBytes )
		{ return RequireBufferSize( m_InBuffer, m_InBuffLen, nBytes ); } 

	int RequireBufferSize( unsigned char*& Buffer, long& BuffLen, long nNewSize );
	
	int ArchiveToBuffer( ArchiveIndexEntry& identry );

	bool PastEndOfFile( DWORD dwHigh, DWORD dwLow, int length );

	// Data members
	
	ArchiveHeader		m_hdr;								// File Header
	ArchiveLookupTable	m_LookupTables[TABLEID_COUNT];		// Lookup tables

	// 2 memory buffers are needed for the compresssion and decompression
	unsigned char*		m_InBuffer;							// Input Memory Buffer
	long				m_InBuffLen;						// Length of Memory Buffer
	long				m_InBuffUsed;						// Length of record in Memory Buffer

	unsigned char*		m_OutBuffer;						// Output Memory Buffer
	long				m_OutBuffLen;						// Length of Memory Buffer
	long				m_OutBuffUsed;						// Length of record in Memory Buffer
	long				m_OutBuffPos;						// Retrieve pointer position 

	tstring				m_FileName;							// File name
	HANDLE				m_hFile;							// File handle
	CODBCQuery&			m_query;							// ODBC Query object reference
	long				m_TableID;							// Type of table to (de)serialize
	
	// The maps are used for translating IDs when inserting records
	IDMap				m_IDMap[TABLEID_COUNT];
	
	IDMap				m_ActualObjectIDs;					// Mappings for Objects.ActualID

	bool				m_bMerge;							
	// If a record already exists in the database, then if
	// m_bMerge = true, leave database record unchanged
	// m_bMerge = false, update database record from archive

	bool				m_bReplaceConfig;

	int					m_OpCode;							// Archive operation
	
	FILETIME			m_dwFileSize;						// Size of archive (when reading)

};

typedef int (*pArchiveTableFunction )( CArchiveFile& arc, long ID );

typedef struct ArchiveTableInfo
{
	pArchiveTableFunction	StuffFn;
	pArchiveTableFunction   RetrieveFn;
	TCHAR*					szTableName;
	TCHAR*					szPrimaryKey;
	int						ReservedID;
} ArchiveTableInfo;

extern ArchiveTableInfo g_ArcTblInfo[TABLEID_COUNT];
extern int g_ArcTblOrder[TABLEID_COUNT];


int RetrieveInboundMessageToStruct( CArchiveFile& arc, InboundMessages_t& msg );
int RetrieveOutboundMessageToStruct( CArchiveFile& arc, OutboundMessages_t& msg );
int RetrieveTicketToStruct( CArchiveFile& arc, Tickets_t& tkt );
int RetrieveAttachmentToStruct(  CArchiveFile& arc, Attachments_t& att );
