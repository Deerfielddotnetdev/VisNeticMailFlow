/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/ErrorCodes.h,v 1.1.6.1 2006/07/18 12:47:08 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	This file defines the global error codes used in ISupportErrorInfo   
||              
\\*************************************************************************/

#ifndef DEERFIELD_EMS_ERROR_CODES_H
#define DEERFIELD_EMS_ERROR_CODES_H


////////////////////////////////////////////////////////////////////////////////
// 
// DeerfieldEMSErrorsCodes defines the base 16-bits of the complete error
// code which also contains a component ID, severity ID, and log entry type ID
// 
////////////////////////////////////////////////////////////////////////////////
typedef enum DeerfieldEMSErrorCodes 
{
    E_DatabaseConnection		=	0x200,	// Database connection error
    E_DatabaseQuery				=	0x201,	// Database query error
	E_DSNNotInRegistry			=   0x202,	// No DSN in registry
	E_AllocHandle				=   0x203,	// Error allocating handle
	E_SetEnvAttrError			=   0x204,	// Error setting environment attributes
	E_SetConnOptionError		=   0x205,	// Error setting connection option
	E_SetStmtAttrError			=   0x206,	// SetStmtAttr error
	E_GetStmtAttrError			=	0x207,	// GetStmtAttr error
	E_SQLGetDataError			=   0x208,	// SQLGetData error
	E_SQLFreeStmt				=	0x209,	// SQLFreeStmt error
	E_StringNotFound			=   0x20a,	// String not found in Strings table
	E_SQLRowCount				=   0x20b,	// SQLRowCount error
	E_SQLGetDiagField			=   0x20c,	// SQLGetDiagField error
	E_SQLUnknown				=	0x20d,	// An unknown ODBC error occured
	E_UknownDatabaseType		=	0x20e,	// Unsupported database platform
	
    E_NoOnStartPage				=	0x300,	// OnStartPage wasn't called
	E_SystemError				=   0x301,	// System error
	E_InvalidParameters			=	0x302,	// One or more parameters are invalid
	E_InvalidID					=   0x303,	// The object ID wasn't found in DB
	E_UnhandledException		=   0x304,	// catch( ... ) 
	E_MemoryError				=	0x305,	// memory allocation failed
	E_UnableToLoadResourceDLL	=   0x306,	// unable to load the resource dll
	E_SystemStopping			=   0x307,	// system is shutting down.
	E_ODBCError					=   0x308,	// ODBC Exception
	E_EMSException				=   0x309,  // EMS Exception
	E_SPUnhandledException		=   0x310,	// Unhandled exception in ServePage()
	E_SPODBCError				=   0x31A,	// ODBC Exception in ServePage()
	E_SPEMSException			=   0x31B,  // EMS Exception in ServerPage()
	E_XSLTranslation			=   0x31C,	// Error in XSL Translation
	E_DCAException              =   0x31D,  // DCA Exceptions
	E_SystemStarting			=   0x31E,	// system is starting.

	E_MsgFileTooLarge			=	0x400,	// the message file was too large
	E_OpeningMsgFile			=	0x401,	// an error occured opening message file
	E_ReadingMsgFile			=	0x402,	// an error occured reading the message file
	E_WritingMsgFile			=	0x403,	// an error occured writing the message file
	E_InvalidMsgFile			=	0x404,	// the specified message file was invalid
	E_InfectedBody				=	0x405,
	E_InfectedFile				=   0x406,	// file is infected with a virus
	E_VirusScanFailed			=   0x407,	// unspecified error in virus scan
	E_WritingAttachment			=	0x408,	// an error occured writing an atachment file
	E_AttachPathNotInReg		=	0x409,	// error reading the attachment path
	E_AttachNotFound			=	0x40a,	// attachment not found on disk
	E_DiskFull					=   0x40b,	// disk full error
	E_CleanedBody				=	0x40c,	// the body had a virus which was cleaned
	E_SuspiciousFile			=	0x40d,	// the file was suspicious
	E_FileNotScanned			=	0x40e,	// the file could not be scanned
	E_SuspiciousBody			=	0x40f,	// the body is suspicious
	E_BodyNotScanned			=	0x410,	// the body could not be scanned
	E_MsgAlreadySent			=	0x411,	// the message cannot be revoked as it as already been sent
	
	E_SuccessfulLogin			=	0x501,	
	E_FailedLogin				=	0x502,	
	E_AccessDenied				=   0x503,	// Agent doesn't have correct rights
	E_Logout					=	0x504,	
	E_ResetLock					=	0x505,	// Reset agent ticket lock
	

	E_URLVarNotFound			=	0x600,  // a required URL variable was not found
	E_FormFieldNotFound			=	0x601,  // a required form field was not found
	E_InvalidRequest			=   0x602,  // invalid http request
	E_ParameterTooLarge			=	0x603,	// a form / URL paramter was larger then the supplied buffer
	E_Duplicate_Name			=   0x604,	// an object with this name already exists in DB
	E_FormValidation			=	0x605,	// an error occured validating form data

	E_CreateContact				=	0x700,	// system was unable to create the contact
	E_UpdateContact				=	0x701,	// an error occured updating a contact
	E_ContactEmailInUse			=	0x702,	// the email address is already used by another contact
	E_TicketLocked				=	0x703,	// the ticket is locked by another user
	E_SpellCheckFailed			=	0x704,	// an error occured during spell-check
	E_ReadingArchive			=   0x705,	// an error occured retrieving a message from an archive

} DeerfieldEMSErrorCodes;


typedef struct ODBCError_t
{
	 // default constructor
	ODBCError_t()	{ ZeroMemory(this, sizeof(ODBCError_t)); }

	long nErrorCode;
	long nNativeErrorCode;
	SQLTCHAR szState[SQL_SQLSTATE_SIZE+1];
	SQLTCHAR szErrMsg[SQL_MAX_MESSAGE_LENGTH];
	
} ODBCError_t;

#endif