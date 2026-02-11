////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMLIBRARY/KAVScan.h,v 1.2.4.1 2006/05/01 15:44:11 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// KAVScan.h: interface for the CKAVScan class.
//
//////////////////////////////////////////////////////////////////////
//
// This class encapsulates virus scanning. It makes the decision
// which API to use (pre-4.3 CKAVScanOld) or (4.3 - CKAVScanNew ) 
// and hands the virus scan calls off to the appropriate object.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KAVSCAN_H__B1FCA041_829F_4C83_A109_D38DFBCB7A43__INCLUDED_)
#define AFX_KAVSCAN_H__B1FCA041_829F_4C83_A109_D38DFBCB7A43__INCLUDED_

////////////////////////////////////////////////////////////////////////////////
// 
// Please note that COM needs to be initialized before using this class and
// the object must be destroyed (or UnInitialized) before COM is uninitialized.
// 
////////////////////////////////////////////////////////////////////////////////

#include "KAVScanNew.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct VirusScanInfo
{
	char szFile[MAX_PATH+1];			// filename 
	char* szString;						// string - must be allocated with GlobalAlloc();
	bool bFile;							// if true, scan file, else string
	HANDLE hEvent;						// Set when scan complete
	int nRet;							// Return code from CKAVScan::Errors enumeration
	char szVirusName[MAX_PATH+1];		// Name of virus, if found
	char szQuarantineLoc[MAX_PATH+1];	// Location of quarantined file, 
										//  if nRet=Success_Object_Has_Been_Quarantined
	int nOptions;						// 0=standard, 1=repair only
} VirusScanInfo;


class CKAVScan  
{
public:
	CKAVScan();
	virtual ~CKAVScan();
	
	enum StringSizes
	{
		VirusNameLen = 256,
	};

	enum Errors 
	{
		Success,
		Success_Object_Has_Been_Cleaned,
		Success_Object_Has_Been_Deleted,
		Success_Object_Has_Been_Quarantined,
		//Err_Initializing_Old_Plugin,
		//Err_Calling_Old_Plugin,
		Err_Object_Is_Infected,				
		Err_Object_Is_Suspicious,
		Err_Object_Was_Not_Scanned,
		Err_Unexpected_Failure,
		Err_Creating_IStream,
		Err_Reading_IStream,
		Err_Writing_To_IStream,
		Err_Seeking_IStream,
		Err_Initializing_New_Plugin,
		Err_Product_Not_Registered,
		Err_License_Expired,					// This is from Kaspersky
		Err_License_Limited,					// This is from Kaspersky
		Err_Calling_New_Plugin,
		Err_Deleting_Infected_Object,
	};
	
	enum Actions  { Repair = 0,			// Repair if infected, if unreparable, delete
					Delete = 1,			// Delete if infected
					Quarantine = 2,		// Quarantine if infected (do not attempt repair)
					Allow = 3,			// Pass through (for unscannable and suspicious)
					RepairOnly = 4 };	// Repair only (do not delete on failure)

	int ScanFile( LPCSTR szFileName, int nVirusAction, int nSuspiciousAction = Allow, int nNotScannedAction = Allow );

	int ScanString( LPSTR& szString, int nAction );

	// Call this after virus has been found
	char* GetVirusName(void) { return m_szVirusName; }
	dca::String& GetQuarantineLoc(void) { return m_sQuarantineLoc; }


//	static int StringToStream( string& sString, IStream*& pStream );
//	static int StreamToString( IStream*& pStream, string& sString );

	static int LPSTRToStream( LPSTR& szString, IStream*& pStream );
	static int StreamToLPSTR( IStream*& pStream, LPSTR& szString );


	int QuarantineFile( LPCSTR szFileName );
	int DeleteFile( LPCSTR szFileName );
	bool VerifyDirectory( dca::String& sQuarantinePath );

	int GetVersionInfo(dca::String& sVersion, dca::String& sLastUpdate, dca::String& sRecordCount );

	void UnInitialize(void);

protected:

	int Initialize(void);

	
	//CKAVScanOld m_OldScanner;
	CKAVScanNew m_NewScanner;

	bool m_bInitialized;
	bool m_bUseNew;

	char	     m_szVirusName[VirusNameLen + 1];
	dca::String  m_sQuarantineLoc;

};

#endif // !defined(AFX_KAVSCAN_H__B1FCA041_829F_4C83_A109_D38DFBCB7A43__INCLUDED_)
