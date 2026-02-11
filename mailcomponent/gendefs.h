#pragma once

#if !defined(AFX_GEN_DEFS_INCLUDED)
#define AFX_GEN_DEFS_INCLUDED

#define DEFAULT_BUFFER_LENGTH			1024
#define LARGE_BUFFER_LENGTH				4096
#define EXTRA_LARGE_BUFFER_LENGTH		8192

#define X_MAILER_HEADER_VALUE			"VisNetic MailFlow 6.9.4.1"
#define X_MAILER_HEADER_VALUE_EVAL		"VisNetic MailFlow 6.9.4.1 - Evaluation"

// arraysize macro
//#define ARRAYSIZE(a)					(sizeof(a)/sizeof(a[0]))

#define DEFAULT_MAX_MSG_SIZE_OUTBOUND	5000
#define DEFAULT_MAX_MSG_SIZE_INBOUND	5000
#define DEFAULT_MAX_SEND_RETRY_HOURS	48

#endif // !defined(AFX_GEN_DEFS_INCLUDED)