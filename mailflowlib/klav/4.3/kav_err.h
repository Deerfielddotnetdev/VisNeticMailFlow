// kav_err.h /////////////////////////////////////////////////////////////////

// Kaspersky Anti-Virus Interface Errors
// Copyright (C) 1999-2002 Kaspersky Lab. All rights reserved.

// Dmitry A. Gnusaryov [dima@kaspersky.com]

//////////////////////////////////////////////////////////////////////////////

#ifndef KAV_ERR_H
#define KAV_ERR_H

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////

#define KAV_E_QUEUE_OVERFLOW	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 600)
#define KAV_E_QUEUE_EMPTY		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 601)
#define KAV_E_TIMEOUT			MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 602)
#define KAV_E_BUSY				MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 603)

#define KAV_E_PRODUCT_NOT_REGISTERED  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 604)
#define KAV_E_LICENCE_EXPIRED   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 605)
#define KAV_E_LICENCE_LIMITED   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 606)

//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif // KAV_ERR_H

// eof ///////////////////////////////////////////////////////////////////////