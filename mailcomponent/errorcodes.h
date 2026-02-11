#pragma once

#if !defined(AFX_ERR_CODES_INCLUDED)
#define AFX_ERR_CODES_INCLUDED

// success
#define ERROR_NO_ERROR						0x000	// Success; no error was encountered

// mail
#define ERROR_APOP_SERVER_UNSUPPORTED		0x101	// APOP not supported by remote server
#define	ERROR_BAD_POP_ACCOUNT				0x102	// POP account is invalid
#define ERROR_POP_AUTH_FAILURE				0x103	// POP account authentication failure
#define ERROR_NO_MSG_DESTS_DEFINED			0x104	// No message sources are currently defined
#define ERROR_SMTPAUTH_UNSUPPORTED			0x105	// SMTP authentication is not supported by remote server
#define ERROR_SMTP_CRAMMD5_AUTH_FAILURE		0x106	// SMTP authentication (CRAM MD5) failure
#define ERROR_SMTP_LOGIN_AUTH_FAILURE		0x107	// SMTP authentication (LOGIN) failure
#define ERROR_NO_POP_ACCOUNTS_DEFINED		0x108	// No POP accounts are current defined
#define	ERROR_CMD_UNSUPPORTED				0x109	// Command not supported by remote server
#define	ERROR_MALFORMED_RESPONSE			0x10B	// Malformed response received from remote server
#define ERROR_COMMAND_FAILED				0x10C	// Remote server command failed
#define ERROR_NO_VALID_MSG_RECIPIENTS		0x10D	// Message had no valid recipients
#define ERROR_PERMANENT_MSG_DEL_FAILURE		0x10E	// Permanent outbound message delivery failure
#define ERROR_TEMP_MSG_DEL_FAILURE			0x10F	// Temporary outbound message delivery failure
#define ERROR_MSG_DEST_UNRESOLVED			0x110	// Message destination unresolved
#define ERROR_POP3_POLLTHREAD_SHUTDOWN		0x111	// POP polling thread was shut down
#define ERROR_SMTP_POLLTHREAD_SHUTDOWN		0x112	// SMTP polling thread was shut down
#define ERROR_MSG_DATA_REJECTED				0x113	// Message DATA segment was rejected by remote server
#define ERROR_OUTBOUND_MSG_MAXSIZE_EXCEEDED 0x114	// Maximum outbound message size exceeded
#define	ERROR_DEFAULT_MSG_DEST_MISSING		0x115	// Default message destination is missing
#define	ERROR_MSG_TOO_OLD					0x116	// Message is too old

// generic
#define ERROR_DATABASE						0x201	// Database error (generic)
#define ERROR_FILESYSTEM					0x202	// Filesystem error (generic)
#define ERROR_MAILCOMPONENTS_INIT			0x203	// Mail components initialization
#define ERROR_BAD_PARAMETER					0x204	// Bad function parameter
#define ERROR_REGISTRY						0x205	// Windows registry failure
#define ERROR_MEMORY_ALLOCATION				0x206	// Memory allocation failure
#define ERROR_UNKNOWN_ERROR					0x207	// Unknown error
#define ERROR_THREAD_STARTUP				0x208	// Thread startup failure
#define ERROR_COM_INIT						0x209	// COM initialization failure
#define ERROR_DECRYPTING_DATA				0x20A	// Generic decryption error
#define ERROR_DISK_DATA_SIZE				0x20B	// Generic decryption error

// socket
#define	FAILURE_SERVER_CONNECT				0x301	// Socket connection failure
#define	FAILURE_SERVER_LOOKUP				0x302	// Name lookup failure
#define	FAILURE_DATA_TRANSMISSION			0x303	// Socket data transmission failure
#define FAILURE_OPERATION_TIMEOUT			0x304	// Socket operation timeout
#define	FAILURE_CONNECT_REFUSED				0x305	// Socket connection refused
#define	FAILURE_DISCONNECTED				0x306	// Socket disconnected
#define FAILURE_OPERATION_CANCELLED			0x307	// Socket operation cancelled
#define FAILURE_SOCKET_CREATE				0x308	// Socket creation failure
#define FAILURE_SERVER_CONNECT_TIMEOUT		0x309	// Server connection timeout
#define FAILURE_SERVER_CONNECT_REFUSED		0x30A	// Server connection refused
#define ERROR_INIT_WINSOCK					0x30B	// Winsock initialization failure
#define ERROR_READ_LINE						0x30C	// Error reading line from socket

// anti-virus
#define ERROR_AV_SCANNER_INIT_FAILURE		0x401	// Anti-virus scanner initialization failure
#define ERROR_ENACTING_SCAN					0x402	// Error enacting anti-virus scan
#define ERROR_POSTING_SCAN_JOB				0x403	// Error posting the AV scan job to the scanning thread

#endif // !defined(AFX_ERR_CODES_INCLUDED)