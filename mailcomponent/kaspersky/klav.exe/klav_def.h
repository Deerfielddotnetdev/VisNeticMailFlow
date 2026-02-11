// klav_def.h ////////////////////////////////////////////////////////////////

// Kaspersky Anti-Virus Interface Defines
// Copyright (C) 2001 Kaspersky Lab. All rights reserved.

// Dmitry A. Gnusaryov [dima@kaspersky.com]

//////////////////////////////////////////////////////////////////////////////

#ifndef KLAV_DEF_H
#define KLAV_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////

#define AVOPTIONS_MODE_COMPOUND_PACKED			0x00000001
#define AVOPTIONS_MODE_COMPOUND_ARCHIVES		0x00000002
#define AVOPTIONS_MODE_COMPOUND_MAIL_DATABASES	0x00000004
#define AVOPTIONS_MODE_COMPOUND_PLAIN_MAIL		0x00000008
#define AVOPTIONS_MODE_COMPOUND_LIMIT_SIZE		0x00000010

#define AVOPTIONS_MODE_CODE_ANALYSER			0x00000020

#define AVOPTIONS_MODE_REDUNDANT				0x00000040
#define AVOPTIONS_MODE_REDUNDANT_LIMIT_SIZE		0x00000080

//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif // KLAV_DEF_H

// eof ///////////////////////////////////////////////////////////////////////