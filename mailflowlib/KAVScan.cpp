// KAVScan.cpp: implementation of the CKAVScan class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KAVScan.h"
#include "MessageIO.h"
#include "RegistryFns.h"

//////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////
CKAVScan::CKAVScan()
{
	m_bInitialized = false;
	m_bUseNew = false;
	ZeroMemory( m_szVirusName, sizeof(m_szVirusName) );
}

////////////////////////////////////////////////////////////////////////////////
// 
// Destructor
// 
////////////////////////////////////////////////////////////////////////////////
CKAVScan::~CKAVScan()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// Initialize - If initialize failed, it will be called again every time a
//              scan is attempted until it succeeds.
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScan::Initialize(void)
{
	int nErr;

	// Try new interface first
	nErr = m_NewScanner.Initialize();

	/*if( nErr == 0 )
	{
		m_bUseNew = true;
	}*/
	if(nErr)
	{
		// Fallback to old interface
		//nErr = m_OldScanner.Initialize();

		//if( nErr )
		//{
			// Fatal - couldn't initialize either plug-in
			return nErr;
		//}

	}

	m_bInitialized = true;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// ScanFile
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScan::GetVersionInfo(dca::String& sVersion, dca::String& sLastUpdate, 
							 dca::String& sRecordCount )
{
	int nErr = Success;

	// Initialize on first call
	if( !m_bInitialized ) 
	{
		if( (nErr = Initialize()) != 0 )
			return nErr;
	}
	
	// Scan on disk
	//if( m_bUseNew )
	//{
		nErr = m_NewScanner.GetVersionInfo( sVersion, sLastUpdate, sRecordCount );
	//}
	//else
	//{
		//nErr = m_OldScanner.GetVersionInfo( sVersion, sLastUpdate, sRecordCount );
	//}

	return nErr;
}


////////////////////////////////////////////////////////////////////////////////
// 
// ScanFile
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScan::ScanFile( LPCSTR szFileName, int nVirusAction, 
					    int nSuspiciousAction, int nNotScannedAction )
{
	int nErr;

	// Initialize on first call
	if( !m_bInitialized ) 
	{
		if( (nErr = Initialize()) != 0 )
			return nErr;
	}
	
	// Clear virus name
	m_szVirusName[0] = '\0';
	m_sQuarantineLoc = "";

	// Scan on disk
	//if( m_bUseNew )
	//{
		nErr = m_NewScanner.ScanFile( szFileName, nVirusAction, m_szVirusName );
	//}
	//else
	//{
		//nErr = m_OldScanner.ScanFile( szFileName, nVirusAction, m_szVirusName );
	//}

	// handle deletes and quarantines


	if( nVirusAction != CKAVScan::RepairOnly )
	{
		if( nErr == Err_Object_Is_Infected )
		{
			if( nVirusAction == CKAVScan::Quarantine )
			{
				return QuarantineFile( szFileName );
			}
			else 
			{
				return CKAVScan::DeleteFile( szFileName );
			}
		}
		else if( nErr == Err_Object_Is_Suspicious )
		{
			if( nSuspiciousAction == CKAVScan::Quarantine )
			{
				return QuarantineFile( szFileName );
			}
			else if( nSuspiciousAction == CKAVScan::Delete )
			{
				return CKAVScan::DeleteFile( szFileName );
			}
		}
		// MJM - perform this action if the file cannot
		// be scanned, regardless of the error code!!
		else if( nErr == Err_Object_Was_Not_Scanned || nErr == Err_Unexpected_Failure )
		{
			if( nNotScannedAction == CKAVScan::Quarantine )
			{
				if ( QuarantineFile( szFileName ) == Err_Deleting_Infected_Object )
				{
					return Err_Deleting_Infected_Object;
				}
			}
			else if( nNotScannedAction == CKAVScan::Delete )
			{
				if ( CKAVScan::DeleteFile( szFileName )== Err_Deleting_Infected_Object )
				{
					return Err_Deleting_Infected_Object;
				}
			}
	
			return Err_Object_Was_Not_Scanned;
		}
	}

	return nErr;
}


////////////////////////////////////////////////////////////////////////////////
// 
// ScanString
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScan::ScanString( LPSTR& szString, int nAction )
{
	int nErr;

	// Initialize on first call
	if( !m_bInitialized ) 
	{
		if( (nErr = Initialize()) != 0 )
			return nErr;
	}

	// Clear the virus name
	m_szVirusName[0] = _T('\0');
	m_sQuarantineLoc = _T("");

	// Scan in memory
	//if( m_bUseNew )
	//{
		nErr = m_NewScanner.ScanString( szString, nAction, m_szVirusName );
	//}
	//else
	//{
		//nErr = m_OldScanner.ScanString( szString, nAction, m_szVirusName );
	//}

	return nErr;
}

/*
////////////////////////////////////////////////////////////////////////////////
// 
// StringToStream - Allocate an IStream using the contents of a char string.
//                  If this function fails, the IStream will be released.
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScan::StringToStream( string& sString, IStream*& pStream )
{
	HRESULT hr;
	LARGE_INTEGER zero;
	zero.LowPart = 0;
	zero.HighPart = 0;

	// create an IStream object from our string buffer
	hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	
	if (!SUCCEEDED(hr))
	{
		return CKAVScan::Err_Creating_IStream;
	}
	
	// write the string text to IStream
	hr = pStream->Write( sString.c_str(), sString.size(), NULL);
	
	if (!SUCCEEDED(hr))
	{
		pStream->Release();
		return CKAVScan::Err_Writing_To_IStream;
	}
	
	// reset the IStream seek point

	hr = pStream->Seek(zero, STREAM_SEEK_SET, NULL);
	
	if (!SUCCEEDED(hr))
	{
		pStream->Release();
		return CKAVScan::Err_Seeking_IStream;
	}
	
	return CKAVScan::Success;
}

////////////////////////////////////////////////////////////////////////////////
// 
// StreamToString - Copy the contents of an IStream to a char string.
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScan::StreamToString( IStream*& pStream, string& sString )
{
	HRESULT hr;
	LARGE_INTEGER zero;
	zero.LowPart = 0;
	zero.HighPart = 0;
	STATSTG stat;
	ULONG bytesRead = 0;

	ZeroMemory( &stat, sizeof(stat) );

	// Get information about the stream
	hr = pStream->Stat( &stat, STATFLAG_NONAME );

	if( stat.cbSize.HighPart )
	{
		// Error! stream too big
		return CKAVScan::Err_Reading_IStream;
	}

	hr = pStream->Seek(zero, STREAM_SEEK_SET, NULL);
	if (!SUCCEEDED(hr))
	{
		return CKAVScan::Err_Seeking_IStream;
	}

	sString.resize( stat.cbSize.LowPart );

	hr = pStream->Read( (void*) sString.c_str(), stat.cbSize.LowPart, &bytesRead );

	if (!SUCCEEDED(hr))
	{
		return CKAVScan::Err_Reading_IStream;
	}

	return CKAVScan::Success;
}
*/

////////////////////////////////////////////////////////////////////////////////
// 
// LPTSTRToStream
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScan::LPSTRToStream( LPSTR& szString, IStream*& pStream )
{
	HRESULT hr;
	LARGE_INTEGER zero;
	zero.LowPart = 0;
	zero.HighPart = 0;

	// create an IStream object from our string buffer
	hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	
	if (!SUCCEEDED(hr))
	{
		return CKAVScan::Err_Creating_IStream;
	}
	
	// write the string text to IStream
	hr = pStream->Write( szString, strlen( szString ), NULL);
	
	if (!SUCCEEDED(hr))
	{
		pStream->Release();
		return CKAVScan::Err_Writing_To_IStream;
	}
	
	// reset the IStream seek point

	hr = pStream->Seek(zero, STREAM_SEEK_SET, NULL);
	
	if (!SUCCEEDED(hr))
	{
		pStream->Release();
		return CKAVScan::Err_Seeking_IStream;
	}
	
	return CKAVScan::Success;
}
////////////////////////////////////////////////////////////////////////////////
// 
// StreamToLPTSTR
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScan::StreamToLPSTR( IStream*& pStream, LPSTR& szString )
{
	HRESULT hr;
	LARGE_INTEGER zero;
	zero.LowPart = 0;
	zero.HighPart = 0;
	STATSTG stat;
	ULONG bytesRead = 0;

	ZeroMemory( &stat, sizeof(stat) );

	// Get information about the stream
	hr = pStream->Stat( &stat, STATFLAG_NONAME );

	if( stat.cbSize.HighPart )
	{
		// Error! stream too big
		return CKAVScan::Err_Reading_IStream;
	}

	hr = pStream->Seek(zero, STREAM_SEEK_SET, NULL);
	if (!SUCCEEDED(hr))
	{
		return CKAVScan::Err_Seeking_IStream;
	}

	GlobalFree( szString );
	szString = (LPSTR) GlobalAlloc( GMEM_FIXED, stat.cbSize.LowPart + 1 );
	szString[stat.cbSize.LowPart] = _T('\0');

	hr = pStream->Read( (void*) szString, stat.cbSize.LowPart, &bytesRead );

	if (!SUCCEEDED(hr))
	{
		return CKAVScan::Err_Reading_IStream;
	}

	return CKAVScan::Success;
}

////////////////////////////////////////////////////////////////////////////////
// 
// QuarantineFile
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScan::QuarantineFile( LPCSTR szFileName )
{
	char dir[_MAX_DIR];
	char drive[_MAX_DRIVE];
	char file[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath( szFileName, NULL, NULL, file, ext);
	dca::String sFileName = (dca::String)file + (dca::String)ext;
	dca::String sQuarantinePath;
	
	dca::WString szFolder;
	dca::WString szQFolder;
	if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("InboundAttachFolder")).c_str(), szFolder))
	{
		dca::WString sCurFile(szFileName);
		dca::WString::size_type pos = sCurFile.find(szFolder);
		if(pos != tstring::npos)
		{
			if(!GetRegString(EMS_ROOT_KEY, REG_KEY_AV, tstring(_T("QuarantineFolder")).c_str(), szQFolder))
			{
				m_sQuarantineLoc = szFileName;
				dca::String qf(szQFolder.c_str());
				m_sQuarantineLoc.replace(0,szFolder.size(), qf);
			}
			else
			{
				CMessageIO msgIO;
#ifdef _UNICODE
				dca::WString f(sFileName.c_str());
				dca::WString q(m_sQuarantineLoc.c_str());
				msgIO.GetQuarantinePath(f, q);
#else
				msgIO.GetQuarantinePath(sFileName, m_sQuarantineLoc);
#endif
			}
		}
		else
		{
			if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("OutboundAttachFolder")).c_str(), szFolder))
			{
				if(!GetRegString(EMS_ROOT_KEY, REG_KEY_AV, tstring(_T("QuarantineFolder")).c_str(), szQFolder))
				{
					m_sQuarantineLoc = szFileName;
					dca::String qf(szQFolder.c_str());
					m_sQuarantineLoc.replace(0,szFolder.size(), qf);
				}
				else
				{
					CMessageIO msgIO;
#ifdef _UNICODE
					dca::WString f(sFileName.c_str());
					dca::WString q(m_sQuarantineLoc.c_str());
					msgIO.GetQuarantinePath(f, q);
#else
					msgIO.GetQuarantinePath(sFileName, m_sQuarantineLoc);
#endif
				}
			}
			else
			{
				CMessageIO msgIO;
#ifdef _UNICODE
				dca::WString f(sFileName.c_str());
				dca::WString q(m_sQuarantineLoc.c_str());
				msgIO.GetQuarantinePath(f, q);
#else
				msgIO.GetQuarantinePath(sFileName, m_sQuarantineLoc);
#endif
			}
		}
	}
	else
	{
		CMessageIO msgIO;

#ifdef _UNICODE
		dca::WString f(sFileName.c_str());
		dca::WString q(m_sQuarantineLoc.c_str());
		msgIO.GetQuarantinePath(f, q);
#else
		msgIO.GetQuarantinePath(sFileName, m_sQuarantineLoc);
#endif
	}
	
	_splitpath( m_sQuarantineLoc.c_str(), drive, dir, NULL, NULL);

	sQuarantinePath = drive;
	sQuarantinePath += dir;

	// Check if	quarantine folder exists
	if( VerifyDirectory( sQuarantinePath ) == false )
	{
		// Could not create quarantine path
		return CKAVScan::DeleteFile( szFileName );
	}

	// move the file to quarantine
	if ( MoveFileA( szFileName, m_sQuarantineLoc.c_str()) )
	{
		return CKAVScan::Success_Object_Has_Been_Quarantined;
	}
	else
	{
		// Quarantining failed, revert to delete
		return CKAVScan::DeleteFile( szFileName );
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// DeleteFile
// 
////////////////////////////////////////////////////////////////////////////////
int CKAVScan::DeleteFile( LPCSTR szFileName )
{
	if ( ::DeleteFileA( szFileName ) )
	{
		return CKAVScan::Success_Object_Has_Been_Deleted;
	}
	else
	{
		return CKAVScan::Err_Deleting_Infected_Object;
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// UnInitialize
// 
////////////////////////////////////////////////////////////////////////////////
void CKAVScan::UnInitialize(void)
{
	if( !m_bInitialized ) 
	{
		return;
	}
	else
	{
		//if( m_bUseNew )
		//{
			m_NewScanner.UnInitialize();
		//}
		//else
		//{
			//m_OldScanner.UnInitialize();
		//}

		m_bInitialized = false;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// VerifyDirectory
// 
////////////////////////////////////////////////////////////////////////////////
bool CKAVScan::VerifyDirectory( dca::String& sPath )
{
	// ensure that the quarantine directory path exists
	DWORD attr;
	int pos;
	bool bRes = true;
	
	// Check for trailing slash:
	pos = sPath.find_last_of("\\");
	if (sPath.length() == pos + 1)	// last character is "\"
	{
		sPath.resize(pos);
	}
	
	// look for existing path part
	attr = GetFileAttributesA(sPath.c_str());
	
	// if it doesn't exist
	if (0xFFFFFFFF == attr)
	{
		pos = sPath.find_last_of("\\");
		if (0 < pos)
		{
			// create parent dirs
			dca::String p = sPath.substr(0, pos);
			bRes = VerifyDirectory(p);
		}
		
		// create note
		dca::WString path(sPath.c_str());
		bRes = bRes && CreateDirectoryW(path.c_str(), NULL);
	}
	else if (!(FILE_ATTRIBUTE_DIRECTORY & attr))
	{	
		bRes = false;
	}
	
	return bRes;
}