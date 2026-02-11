// KAVScanNew.cpp: implementation of the CKAVScanNew class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RegistryFns.h"
#include "KAVScan.h"

#include "klav/4.3/kav_err.h"
#include "klav/4.3/version.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKAVScanNew::CKAVScanNew() :
	m_bInitialized(false)	
{
}

CKAVScanNew::~CKAVScanNew()
{
	UnInitialize();
}


////////////////////////////////////////////////////////////////////////////////
// 
// Initialize
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScanNew::Initialize(void)
{
	try
	{
		m_kavScanner.Initialize();
	}
	catch(...)
	{
		switch(m_kavScanner.GetLastHresult())
		{
		case KAV_E_PRODUCT_NOT_REGISTERED:
			return CKAVScan::Err_Product_Not_Registered;

		case KAV_E_LICENCE_EXPIRED:
			return CKAVScan::Err_License_Expired;

		case KAV_E_LICENCE_LIMITED:
			return CKAVScan::Err_License_Limited;
		}

		return CKAVScan::Err_Initializing_New_Plugin;
	}

	m_bInitialized = true;

	return CKAVScan::Success;
}

////////////////////////////////////////////////////////////////////////////////
// 
// GetVersionInfo
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScanNew::GetVersionInfo(dca::String& sVersion, dca::String& sLastUpdate, 
								dca::String& sRecordCount )
{
	KAV_DATABASES_INFO info;

	if(!m_bInitialized)
	{
		return CKAVScan::Err_Initializing_New_Plugin;
	}

	sVersion.assign(m_kavScanner.GetVersion());

	m_kavScanner.GetDatabaseInfo(&info);

	SYSTEMTIME sysTime;

	ZeroMemory( &sysTime, sizeof(sysTime) );

#pragma warning (disable:4244)
	sysTime.wYear = info.dwLastUpdateYear;
	sysTime.wMonth = info.dwLastUpdateMonth;
	sysTime.wDay = info.dwLastUpdateDay;
#pragma warning (default:4244)

	char szBuffer[64];
	// get the size of the date string
	GetDateFormatA(LOCALE_USER_DEFAULT, DATE_LONGDATE, &sysTime, NULL, 
		          szBuffer, 63 );

	sLastUpdate.assign(szBuffer);

	sprintf( szBuffer, "%d", info.dwNumberOfViruses );

	sRecordCount = szBuffer;

	return CKAVScan::Success;
}



////////////////////////////////////////////////////////////////////////////////
// 
// ScanFile
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScanNew::ScanFile( LPCSTR szFileName, int nAction, LPSTR szVirusName  )
{
	KAV_OPTIONS Option;
	KAV_RESULT Result;
	HRESULT hr = S_OK;

	if(!m_bInitialized)
	{
		return CKAVScan::Err_Initializing_New_Plugin;
	}

	if( nAction == CKAVScan::Repair || nAction == CKAVScan::RepairOnly)
	{
		if(!m_kavScanner.ScanAndCleanFile(szFileName, &Option, &Result))
			hr = m_kavScanner.GetLastHresult();
	}
	else
	{
		if(!m_kavScanner.ScanFile(szFileName, &Option, &Result))
			hr = m_kavScanner.GetLastHresult();
	}

	if( !SUCCEEDED(hr) )
	{
		return CKAVScan::Err_Calling_New_Plugin;
	}

	strncpy(szVirusName,Result.szVirusName,CKAVScan::VirusNameLen );

	return ConvertErrorCode( Result.enScanResult );
}


////////////////////////////////////////////////////////////////////////////////
// 
// ScanString
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScanNew::ScanString( LPSTR& szString, int nAction, LPSTR szVirusName  )
{
	IStream* pStream = NULL;
	IStream* pCleanStream = NULL;
	KAV_OPTIONS Option;
	KAV_RESULT Result;
	HRESULT hr = S_OK;
	int nErr;

	if( !m_bInitialized)
	{
		return CKAVScan::Err_Initializing_New_Plugin;
	}

	// Convert the string to a stream object
	nErr = CKAVScan::LPSTRToStream( szString, pStream );

	if( nErr != CKAVScan::Success )
	{
		return nErr;
	}
	
	// Scan it
	if( nAction == CKAVScan::Repair || nAction == CKAVScan::RepairOnly)
	{
		// create an IStream object from our string buffer
		hr = CreateStreamOnHGlobal(NULL, TRUE, &pCleanStream);
		
		if (!SUCCEEDED(hr))
		{
			pStream->Release();
			return CKAVScan::Err_Creating_IStream;
		}		

		if(!m_kavScanner.ScanAndCleanStream(pStream, pCleanStream, &Option, &Result))
			hr = m_kavScanner.GetLastHresult();
	}
	else
	{
		if(!m_kavScanner.ScanStream(pStream, &Option, &Result))
			hr = m_kavScanner.GetLastHresult();
	}

	if( !SUCCEEDED(hr) )
	{
		pStream->Release();
		if( pCleanStream )
			pCleanStream->Release();

		return CKAVScan::Err_Calling_New_Plugin;
	}

	strncpy( szVirusName, Result.szVirusName, CKAVScan::VirusNameLen );

	// Convert "cleaned stream" to string
	if( (nAction == CKAVScan::Repair || nAction == CKAVScan::RepairOnly )
		 && Result.enScanResult == KAV_S_R_DISINFECTED )
	{
		nErr = CKAVScan::StreamToLPSTR( pCleanStream, szString );

		if( nErr != CKAVScan::Success )
		{
			pStream->Release();
			if( pCleanStream )
				pCleanStream->Release();
			return nErr;
		}
	}

	pStream->Release();
	if( pCleanStream )
		pCleanStream->Release();

	return ConvertErrorCode( Result.enScanResult );
}


////////////////////////////////////////////////////////////////////////////////
// 
// ConvertErrorCode
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScanNew::ConvertErrorCode( int avres )
{
	switch( avres )
	{
	case KAV_S_R_CLEAN:
		return CKAVScan::Success;
		
	case KAV_S_R_INFECTED:
		return CKAVScan::Err_Object_Is_Infected;

	case KAV_S_R_DISINFECTED:
		return CKAVScan::Success_Object_Has_Been_Cleaned;
		
	case KAV_S_R_SUSPICIOUS:
		return CKAVScan::Err_Object_Is_Suspicious;

	case KAV_S_R_NONSCANNED:
		return CKAVScan::Err_Object_Was_Not_Scanned;	// TODO: Add an option for these files.

	case KAV_S_R_FAILURE:
		return CKAVScan::Err_Unexpected_Failure;
	}

	return CKAVScan::Err_Unexpected_Failure;
}

////////////////////////////////////////////////////////////////////////////////
// 
// UnInitialize
// 
////////////////////////////////////////////////////////////////////////////////
void CKAVScanNew::UnInitialize(void)
{
	if(m_bInitialized )
	{
		m_kavScanner.Uninitialize();
		m_bInitialized = false;
	}	
}