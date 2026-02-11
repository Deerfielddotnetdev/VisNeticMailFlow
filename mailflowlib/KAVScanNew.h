// KAVScanNew.h: interface for the CKAVScanNew class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CKAVScanNew  
{
public:
	CKAVScanNew();
	virtual ~CKAVScanNew();

	int Initialize(void);
	int GetVersionInfo(dca::String& sVersion, dca::String& sLastUpdate, dca::String& sRecordCount );
	int ScanFile( LPCSTR szFileName, int nAction, LPSTR szVirusName );
	int ScanString( LPSTR& szString, int nAction, LPSTR szVirusName );
	void UnInitialize(void);

protected:

	int ConvertErrorCode( int avres );

	bool m_bInitialized;
	
	dca::KasperskyAV m_kavScanner;
};
