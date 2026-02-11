/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/gendefs.h,v 1.2.2.2 2006/02/23 20:36:51 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   General Defenitions
||              
\\*************************************************************************/

#pragma once
// resource file string constants
#include "..\\EMSResource\\resource.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	General Purpose Macros	              
\*--------------------------------------------------------------------------*/
//#define ARRAYSIZE(a)	 (sizeof(a)/sizeof(a[0]))
#define CHAR_ARRAYSIZE(a) ((sizeof(a)/sizeof(a[0])) - 1)
#define SORT_ORDER(a) a ? _T("ASC") : _T("DESC")	

/*---------------------------------------------------------------------------\                     
||  Comments:	Constants              
\*--------------------------------------------------------------------------*/
#define SZSERVICENAME _T("VisNetic MailFlow Service")
#define AVSCANSTATE VirusScanStatesEnum
#define SESSIONID_LENGTH 16
#define EMS_CDL_CHUNK_SIZE 25
#define EMS_DEFAULT_PAGE_ROWS 25

/*---------------------------------------------------------------------------\                     
||  Comments:	Registry Constants	              
\*--------------------------------------------------------------------------*/
#define EMS_ROOT_KEY				HKEY_LOCAL_MACHINE
#define EMS_LOCATION_IN_REGISTRY	_T("Software\\Deerfield.com\\VisNetic MailFlow")
#define	EMS_REG_LOC_MAIL_BASE		_T("Software\\Deerfield.com\\VisNetic MailFlow\\MsgComponent")
#define	REG_LOC_UIDL				_T("Software\\Deerfield.com\\VisNetic MailFlow\\MsgComponent\\UIDL")
#define	REG_KEY_AV					_T("Software\\Deerfield.com\\VisNetic MailFlow\\VirusScanner")
#define EMS_DATABASE_DSN_VALUE		_T("DBLogin")
#define EMS_DATABASE_TYPE_VALUE		_T("DBType")
#define EMS_LOG_PATH_VALUE			_T("LogPath")
#define EMS_INSTALL_PATH_VALUE		_T("InstallPath")
#define EMS_REG_RESOURCE_PATH		_T("ResourceDLL")
#define EMS_ARCHIVE_PATH_VALUE		_T("ArchivePath")
#define EMS_BACKUP_PATH_VALUE		_T("BackupPath")
#define EMS_REPORT_PATH_VALUE		_T("ReportPath")
#define EMS_URL_BASE_VALUE			_T("URLBase")
#define EMS_URL_SUBDIR_VALUE		_T("URLSubDir")
#define EMS_VERSION_VALUE			_T("Version")
#define EMS_DB_VERSION_VALUE		_T("dbVersion")
#define EMS_SERVER_ID_VALUE			_T("ServerID")


/*---------------------------------------------------------------------------\                     
||  Comments:	Structures	              
\*--------------------------------------------------------------------------*/

// struct to represent a unique message
typedef struct MsgID_t
{
	MsgID_t() 
	{
		MsgID = 0;
		IsInbound = 0;
	}

	int MsgID;
	bool IsInbound;
	
} MsgID_t;

inline bool operator<(const MsgID_t& A,const MsgID_t& B)
{
	if (A.MsgID < B.MsgID) return true;
	if ((A.MsgID == B.MsgID) && (A.IsInbound < B.IsInbound)) return true;
	return false;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	COM ASSESORS	              
\*--------------------------------------------------------------------------*/
#define PROPPUT_TIME( a )	if (newVal == NULL) { a##Len = SQL_NULL_DATA; } \
							else { a##Len = sizeof(TIMESTAMP_STRUCT); VarDateToTimeStamp(newVal, a); }

#define PROPGET_TIME( a )	if (a##Len == SQL_NULL_DATA) { *pVal = NULL; } \
							else { TimeStampToVarDate(a, *pVal); }

#define PROPPUT_WCHAR( a )  wcsncpy( a, newVal, (sizeof(a)/sizeof(wchar_t)) - 1 )

#define PROPGET_WCHAR( a )  *pVal = SysAllocString( a ); \
	if ((*pVal == NULL) && (wcslen(a) > 0)) \
							{\
							THROW_EMS_EXCEPTION(E_MemoryError, CEMSString(EMS_STRING_ERROR_MEMORY));\
							}\

  
#define PROPPUT_CHAR( a )	WideCharToMultiByte( CP_ACP, 0, newVal, -1, a, sizeof(a), NULL, NULL)

#define PROPGET_CHAR( a )	wchar_t temp[sizeof(a)]; \
	MultiByteToWideChar( CP_ACP, 0, a, -1, temp, sizeof(a) ); \
	*pVal = SysAllocString( temp ); \
	if ((*pVal == NULL) && (wcslen(a) > 0)) \
							{\
							THROW_EMS_EXCEPTION(E_MemoryError, CEMSString(EMS_STRING_ERROR_MEMORY);\
							}\

#define PROPPUT_TEXT( a )	PutStringProperty( newVal, &a, &a##Allocated )
#define PROPGET_TEXT( a )	PROPGET_WCHAR( a )

// In VB-land true is 0xffff and false is 0, and no other values are allowed for the data
// type VARIANT_BOOL. We convert these to 0 and 1 for C.
#define PROPPUT_BOOL( a )  a = (newVal) ? 1 : 0
#define PROPGET_BOOL( a )  *pVal = ( a ) ? 0xffff : 0


/*---------------------------------------------------------------------------\                     
||  Comments:	Enumerations          
\*--------------------------------------------------------------------------*/

// Built-In Standard Response Categories
typedef enum DeerfieldEMSStdCategories
{
	EMS_STDRESP_MY_FAVORITES = -2,
	EMS_STDRESP_FREQUENTLY_USED = -1,
	EMS_STDRESP_SEARCH_RESULTS = -3,
	EMS_STDRESP_UNCATEGORIZED = -4,
	EMS_STDRESP_DRAFTS	= 1

} DeerfieldEMSStdCategories;

// Deleted Items Types
typedef enum DeerfieldEMSDeletedItemTypes
{
	EMS_DELETED_TICKETS  = 1,
	EMS_DELETED_INBOUND_MSG = 2,
	EMS_DELETED_OUTBOUND_MSG = 3,
	EMS_DELETED_CONTACTS = 4,
	EMS_DELETED_STDRESPONSES  = 5,
	EMS_DELETED_END = 6
	
} DeerfieldEMSDeletedItemTypes;

// TicketBoxView columns
typedef enum DeerfieldEMSTBVColumns
{
	EMS_COLUMN_SUBJECT, 
	EMS_COLUMN_CONTACTS,
	EMS_COLUMN_DATE,
	EMS_COLUMN_PRIORITY,
	EMS_COLUMN_STATE,
	EMS_COLUMN_TO,
	EMS_COLUMN_OWNER,
	EMS_COLUMN_COUNT,
	EMS_COLUMN_ATTACH_COUNT,
	EMS_COLUMN_TICKETBOX,
	EMS_COLUMN_TICKETID,
	EMS_COLUMN_NOTE_COUNT,
	EMS_COLUMN_TICKETCATEGORY,
	EMS_COLUMN_AGENT,
	EMS_COLUMN_FROM,
	EMS_COLUMN_END,
} DeerfieldEMSTBVColumns;

// Access Control Levels
typedef enum DeerfieldEMSAccessLevels
{
	EMS_NO_ACCESS       = 0,
	EMS_ENUM_ACCESS		= 1,
	EMS_READ_ACCESS		= 2,
	EMS_EDIT_ACCESS	    = 3,
	EMS_DELETE_ACCESS	= 4,
		
} DeerfieldEMSAccessLevels;

// Ticket Box Status
typedef enum DeerfieldEMSTicketBoxStatusIDs
{
	EMS_TICKETBOX_NORMAL		= 0,
	EMS_TICKETBOX_ACCESSDENIED	= 1,
	EMS_TICKETBOX_LOWWATERMARK	= 2,
	EMS_TICKETBOX_HIGHWATERMARK = 3,
		
} DeerfieldEMSTicketBoxStatusIDs;


typedef enum DeerfieldEMSMatchLocations
{
	EMS_MATCH_LOCATION_SUBJECT			= 0,
	EMS_MATCH_LOCATION_BODY				= 1,	
	EMS_MATCH_LOCATION_SUBJECT_OR_BODY	= 2,
	EMS_MATCH_LOCATION_HEADERS			= 3,
	EMS_MATCH_LOCATION_ANY				= 4
} DeerfieldEMSMatchLocations;

typedef enum DeerfieldEMSPersonalDataTypes
{
	EMS_PERSONAL_DATA_EMAIL 			= 1,
	EMS_PERSONAL_DATA_BUSSINESS_PHONE	= 2,
	EMS_PERSONAL_DATA_HOME_PHONE		= 3,
	EMS_PERSONAL_DATA_MOBLE_PHONE		= 4,
	EMS_PERSONAL_DATA_PAGER				= 5,

} DeerfieldEMSPersonalDataTypes;

typedef enum DeerfieldAutoActionTypes
{
	EMS_AUTO_ACTION_CHANGE_OWNER		= 1,
	EMS_AUTO_ACTION_CHANGE_STATE		= 2,
	EMS_AUTO_ACTION_DELETE				= 3,
	EMS_AUTO_ACTION_ESCALATE			= 4,
	EMS_AUTO_ACTION_MOVE				= 5,

} DeerfieldAutoActionTypes;

typedef enum DeerfieldAutoActionEvents
{
	EMS_AUTO_ACTION_TICKET_CREATED		= 1,
	EMS_AUTO_ACTION_TICKET_OPENED		= 2,
	EMS_AUTO_ACTION_TICKET_CLOSED		= 3,
	EMS_AUTO_ACTION_TICKET_ESCALATED	= 4,
	EMS_AUTO_ACTION_TICKET_MOVED		= 5,
	EMS_AUTO_ACTION_TICKET_REASSIGNED	= 6,
	EMS_AUTO_ACTION_INBOUND_ADDED		= 7,
	EMS_AUTO_ACTION_OUTBOUND_ADDED		= 8,
	EMS_AUTO_ACTION_NOTE_ADDED			= 9,

} DeerfieldAutoActionEvents;

typedef enum OfficeHourTypes
{
	EMS_OFFICE_HOUR_AGENT				= 1,
	EMS_OFFICE_HOUR_TICKETBOX			= 2,
	EMS_OFFICE_HOUR_ROUTINGRULE			= 3,
	EMS_OFFICE_HOUR_GROUP				= 4,
	EMS_OFFICE_HOUR_MESSAGESOURCE		= 5,
	EMS_OFFICE_HOUR_MESSAGEDESTINATION	= 6,
	EMS_OFFICE_HOUR_PROCESSRULE			= 7,
	EMS_OFFICE_HOUR_REPORTSTANDARD		= 8,
	EMS_OFFICE_HOUR_REPORTSCHEDULED		= 9,
	EMS_OFFICE_HOUR_REPORTCUSTOM		= 10,
	EMS_OFFICE_HOUR_AUTOMESSAGE			= 11,
	EMS_OFFICE_HOUR_AUTORESPONSE		= 12,
	EMS_OFFICE_HOUR_ALERTINTEGRATED		= 13,
	EMS_OFFICE_HOUR_ALERTAGE			= 14,
	EMS_OFFICE_HOUR_ALERTWATERMARK		= 15,

} OfficeHourTypes;

#define EMS_MATCH_BODY(x)	((x) == EMS_MATCH_LOCATION_BODY || (x) == EMS_MATCH_LOCATION_SUBJECT_OR_BODY || (x) == EMS_MATCH_LOCATION_ANY)
#define EMS_MATCH_SUBJECT(x)	((x) == EMS_MATCH_LOCATION_SUBJECT || (x) == EMS_MATCH_LOCATION_SUBJECT_OR_BODY || (x) == EMS_MATCH_LOCATION_ANY)
#define EMS_MATCH_HEADERS(x)	((x) == EMS_MATCH_LOCATION_HEADERS || (x) == EMS_MATCH_LOCATION_ANY)

typedef enum DeerfieldEMSDeleteOptions
{
	EMS_DELETE_OPTION_DO_NOT_DELETE				= 0,
	EMS_DELETE_OPTION_DELETE_TO_WASTE_BASKET	= 1,
	EMS_DELETE_OPTION_DELETE_PERMANENTLY		= 2,
	EMS_DELETE_OPTION_RESERVED					= 3,

} DeerfieldEMSDeleteOptions;


typedef enum DeerfieldEMSUserTypes
{
	EMS_USER_TYPE_LOCAL							= 0,
	EMS_USER_TYPE_NT							= 1,
	EMS_USER_TYPE_VMS							= 2,
} DeerfieldEMSUserTypes;

////////////////////////////////////////////////////////////////////////////////
// 
//  These messages are posted to the worker thread to perform the operations
//  provided by the COM interface. (The COM interface implementation posts 
//  these messages.) It is also the primary interface to the routing engine
//  from the MailComponents DLL.
// 
////////////////////////////////////////////////////////////////////////////////
#define WM_RELOAD_CONFIG	(WM_USER+1)  
// WPARAM = An event handle to signal when the reload is complete - You
// should use DuplicateHandle() as the worker thread will close this handle
// after signalling the event.
// LPARAM is one of the following items:
typedef enum ConfigurationItems 
{ 
	EMS_RoutingRules,
	EMS_MessageSources, 
	EMS_MessageDestinations,
	EMS_LogConfig,
	EMS_ServerParameters,
	EMS_Registration,
	EMS_ISAPI,
	EMS_ProcessingRules,
	EMS_AgentTicketCategories,
	EMS_DateFilters,
} ConfigurationItems;

#define WM_PROCESS_OUTBOUND	(WM_USER+2)
// WPARAM and LPARAM are not used
#define WM_PROCESS_INBOUND	(WM_USER+3)
// WPARAM and LPARAM are not used
#define WM_RETRY_OUTBOUND	(WM_USER+4)
// WPARAM and LPARAM are not used
#define WM_WRITE_LOG		(WM_USER+5)
// WPARAM is an error code constructed with the macro EMSERROR below
// LPARAM is a pointer to a wchar_t string that has been allocated with new[]
//        The worker thread will delete[] this buffer.
#define WM_SEND_ALERT		(WM_USER+6)
// WPARAM is a AlertEventID
// LPARAM is a pointer to a AlertInfo structure that has been allocated
//        on the heap using new
#define WM_DB_MAINTENANCE_NOW	(WM_USER+7)
// WPARAM and LPARAM are not used
#define WM_RESTORE_ARCHIVE		(WM_USER+8)
// WPARAM is the Archive ID. LPARAM is not used
#define WM_RESTORE_BACKUP		(WM_USER+9)
// WPARAM is a pointer to a wchar_t string that has been allocated with new[]
//        The worker thread will delete[] this buffer. LPARAM is not used

#define WM_VIRUS_SCAN			(WM_USER+10)


//#define ALERT_TEXT_LENGTH	256
// Note that this is a variable-length structure
typedef struct AlertInfo
{
	long TicketBoxID;						// TicketBoxID (if req'd, set to zero if not req'd)
	long AlertID;							// Custom Alert ID (if req'd, set to zero if not req'd)
	wchar_t wcsText[1];						// Null-terminated text description of alert
} AlertInfo;

// These constants are to be used as the first parameter to the EMSERROR macro
typedef enum DeerfieldEMSErrorComponents
{
	EMSERR_ROUTING_ENGINE	= 1,
	EMSERR_MAIL_COMPONENTS	= 2,
	EMSERR_ISAPI_EXTENSION	= 3

} DeerfieldEMSErrorComponents;

////////////////////////////////////////////////////////////////////////////////
// Error Codes are 32-bit values containing 4 numbers:
//  a LogSeverityID from the LogSeverities table, 
//  a component ID from the DeerfieldEMSErrorComponents enumeration above, 
//  a LogEntryTypeID from the LogEntryTypes table, 
//  and a 16-bit errorcode
//    MSB------------++----------------++---------------------------LSB
//    |4 bits||4 bits||    8 bits      ||            16 bits          |
//    | Sev. || Comp.|| LogEntryTypeID ||         Error Code          |
//    +------++------++----------------++-----------------------------+
////////////////////////////////////////////////////////////////////////////////
#define EMSERROR( Severity, Component, LogEntryType, ErrorCode ) \
        (((Severity)&0xf)<<28)|(((Component)&0xf)<<24)|(((LogEntryType)&0xff)<<16)|((ErrorCode)&0xffff)

#define EMS_ISAPI_LOG_INFO(a,b) EMSERROR(EMS_LOG_SEVERITY_INFORMATIONAL, EMSERR_ISAPI_EXTENSION, a, b)
#define EMS_ISAPI_LOG_WARNING(a,b) EMSERROR(EMS_LOG_SEVERITY_WARNING, EMSERR_ISAPI_EXTENSION, a, b)
#define EMS_ISAPI_LOG_ERROR(a,b) EMSERROR(EMS_LOG_SEVERITY_ERROR, EMSERR_ISAPI_EXTENSION, a, b)

#define EMS_IS_INFECTED(x) ((   ((x) == EMS_VIRUS_SCAN_STATE_VIRUS_FOUND) \
							 || ((x) == EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_DELETED) \
							 || ((x) == EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED) ) ? 1 : 0 )


// Bit fields for Database Maintenance Purge Options

#define	EMS_PURGE_TICKETS		0x00000001
#define	EMS_PURGE_TRASH			0x00000002
#define EMS_PURGE_STDRSPUSAGE	0x00000004
#define EMS_PURGE_ALERTS		0x00000008
#define EMS_PURGE_CONTACTS		0x00000010

