
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MAILCOMPONENTS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MAILCOMPONENTS_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef MAILCOMPONENT_EXPORTS
#define MAILCOMPONENTS_API extern "C" __declspec(dllexport)
#else
#define MAILCOMPONENTS_API extern "C" __declspec(dllimport)
#endif

///////////////////////////////////////////////////////////////////////////////////////////
// MailComponents interface return codes
///////////////////////////////////////////////////////////////////////////////////////////

enum ReturnCodes
{
	error_success = 0,						// no error

	// init and trigger functions
	error_winsock_init = 1,					// couldn't init winsock
	error_thread_startup = 2,				// thread startup failure
	error_thread_shutdown = 3,				// thread shutdown timeout

	// auth tests
	error_connect_failed = 4,				// couldn't connect
	error_auth_failed = 5,					// login credentials invalid
	error_auth_method_unsupported = 6,		// login method (e.g. APOP, CRAM-MD5, etc.) unsupported

	error_params_refresh_failed = 7,
	error_init_components_failed = 8
};


///////////////////////////////////////////////////////////////////////////////////////////
// MailComponents init and trigger functions
///////////////////////////////////////////////////////////////////////////////////////////

MAILCOMPONENTS_API int InitMailComponents(const DWORD dwLogThreadID, const DWORD dwScanThreadID);
MAILCOMPONENTS_API int ShutdownMailComponents(void);
MAILCOMPONENTS_API void CheckMailNow(void);
MAILCOMPONENTS_API void SendMailNow(void);
MAILCOMPONENTS_API int RefreshServerParams(void);


///////////////////////////////////////////////////////////////////////////////////////////
// SMTP & POP authentication test functions
///////////////////////////////////////////////////////////////////////////////////////////

// return codes:
// successful connection and auth	= error_success
// failed to connect to server		= error_connect_failed
// connected ok, failed to auth		= error_auth_failed
MAILCOMPONENTS_API int TestPOP3Auth(const tstring& sHostName, 
									const UINT nPort, 
									const BOOL bIsAPOP, 
									const tstring& sUserName, 
									const tstring& sPassword,
									const UINT nTimeoutSecs,
									const UINT isSSL);

// return codes:
// successful connection and auth	= error_success
// failed to connect to server		= error_connect_failed
// connected ok, failed to auth		= error_auth_failed
MAILCOMPONENTS_API int TestSMTPAuth(const tstring& sHostName, 
									const UINT nPort, 
									const tstring& sUserName, 
									const tstring& sPassword,
									const UINT nTimeoutSecs,
									const UINT isSSL);

// the two calls below are the same as above, but use simple data types
// so they can be called directly from the installer
MAILCOMPONENTS_API int TestPOP3Auth2(const char* sHostName,
									const UINT nPort,
									const bool bIsAPOP,
									const char* sUserName,
									const char* sPassword,
									const UINT nTimeoutSecs,
									const UINT isSSL);

MAILCOMPONENTS_API int TestSMTPAuth2(const char* sHostName, 
									const UINT nPort, 
									const char* sUserName, 
									const char* sPassword,
									const UINT nTimeoutSecs,
									const UINT isSSL);


///////////////////////////////////////////////////////////////////////////////////////////
// Non-exported global functions
///////////////////////////////////////////////////////////////////////////////////////////
struct ODBCError_t;
const UINT OpenDBConn();

void CreateLogEntry(const UINT nErrorCode, const UINT nStringID, ...);
void CreateLogEntry(const UINT nErrorCode, LPCTSTR szString, ...);
void CreateLogEntry(const UINT nErrorCode, CEMSString& emsString );

void CreateAlert(const UINT nAlertEventID, const UINT nStringID, ...);
void HandleDBError(ODBCError_t* pErr);
