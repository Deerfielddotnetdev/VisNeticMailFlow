/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/MessageIO.cpp,v 1.2.4.1 2006/05/01 15:44:11 markm Exp $
||
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "AttachFns.h"
#include "MessageIO.h"
#include "RegistryFns.h"

#include <assert.h>

// file parsing delimiters
static const tstring sPopHeadersStart	= _T("<?mailstream-parsepoint: popheaders>");
static const tstring sPopHeadersEnd		= _T("<?mailstream-parsepoint: /popheaders>");
static const tstring sCcStart			= _T("<?mailstream-parsepoint: cc>");
static const tstring sCcEnd				= _T("<?mailstream-parsepoint: /cc>");
static const tstring sBccStart			= _T("<?mailstream-parsepoint: bcc>");
static const tstring sBccEnd			= _T("<?mailstream-parsepoint: /bcc>");
static const tstring sToStart			= _T("<?mailstream-parsepoint: to>");
static const tstring sToEnd				= _T("<?mailstream-parsepoint: /to>");
static const tstring sReplyToStart		= _T("<?mailstream-parsepoint: reply-to>");
static const tstring sReplyToEnd		= _T("<?mailstream-parsepoint: /reply-to>");
static const tstring sMsgHeaderStart	= _T("<?mailstream-parsepoint: msgheader>");
static const tstring sMsgHeaderEnd		= _T("<?mailstream-parsepoint: /msgheader>");
static const tstring sBodyStart			= _T("<?mailstream-parsepoint: body>");
static const tstring sBodyEnd			= _T("<?mailstream-parsepoint: /body>");
static const tstring sFooterStart		= _T("<?mailstream-parsepoint: footer>");
static const tstring sFooterEnd			= _T("<?mailstream-parsepoint: /footer>");


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageIO::CMessageIO()
{
}

CMessageIO::~CMessageIO()
{
}

///////////////////////////////////////////////////////////////////////////
// writes a format-compliant message file consisting of provided footer,
// header, and message body string buffers. the caller must also provide
// a fully qualified file path;
// returns 0 upon success, otherwise a custom error code (see header file)
///////////////////////////////////////////////////////////////////////////
const UINT CMessageIO::WriteMsgFile(const tstring& sFileLoc, const _msg_file& msgfile)
{
	// validate msg file location
	assert(sFileLoc.size() != 0);
	if (sFileLoc.size() == 0)
		return E_InvalidParameters;


	// create the file; note that in DEBUG mode, the file is always
	// created (e.g. over-written if necessary); in release mode, this
	// call fails if the file already exists
	HANDLE hFile = CreateFile(	sFileLoc.c_str(), 
								GENERIC_WRITE,			// generic write ops
								0,						// no sharing
								NULL,					// no security
#ifdef _DEBUG
								CREATE_ALWAYS,			// always create file (for debugging)
#else
								CREATE_NEW,				// fails if file already exists
#endif
								FILE_ATTRIBUTE_NORMAL,	// normal file
								NULL);					// no attr. template

	
	if (hFile == INVALID_HANDLE_VALUE) 
	{ 		
		return E_WritingMsgFile;
	}

	// build a single buffer that contains all file elements
	tstring sBuffer;
	// pop headers
	sBuffer += sPopHeadersStart;
	sBuffer += msgfile.sPopHeaders;
	sBuffer += sPopHeadersEnd;
	// to
	sBuffer += sToStart;
	sBuffer += msgfile.sTo;
	sBuffer += sToEnd;
	// cc
	sBuffer += sCcStart;
	sBuffer += msgfile.sCc;
	sBuffer += sCcEnd;
	// bcc
	sBuffer += sBccStart;
	sBuffer += msgfile.sBcc;
	sBuffer += sBccEnd;
	// reply-to
	sBuffer += sReplyToStart;
	sBuffer += msgfile.sReplyTo;
	sBuffer += sReplyToEnd;
	// msg header
	sBuffer += sMsgHeaderStart;
	sBuffer += msgfile.sMsgHeader;
	sBuffer += sMsgHeaderEnd;
	// body
	sBuffer += sBodyStart;
	sBuffer += msgfile.sBody;
	sBuffer += sBodyEnd;
	// msg footer
	sBuffer += sFooterStart;
	sBuffer += msgfile.sFooter;
	sBuffer += sFooterEnd;

	// write the header
	DWORD dwBytesWritten;
	BOOL bRet;
	bRet = WriteFile(hFile, sBuffer.c_str(), sBuffer.size() * sizeof(TCHAR), &dwBytesWritten, NULL);
	if (!bRet)
	{
		CloseHandle(hFile);
		return E_WritingMsgFile;
	}

	CloseHandle(hFile);
	
	return 0; // success
}

///////////////////////////////////////////////////////////////////////////
// reads a format-compliant message file into the footer, header,
// and body string placeholders provided by the caller. caller must also
// provide a fully qualified path to the file;
// returns 0 upon success, otherwise a custom error code (see header file)
///////////////////////////////////////////////////////////////////////////
const UINT CMessageIO::ReadMsgFile(const tstring &sFileLoc, _msg_file& msgfile)
{
	// file path must exist
	if (sFileLoc.size() < 1)
	{
		return E_InvalidParameters;
	}

	// open the file for reading and deny further writes
	HANDLE hFile = CreateFile(	sFileLoc.c_str(), 
								GENERIC_READ,			// generic write ops
								FILE_SHARE_READ,		// share reading
								NULL,					// no security
								OPEN_EXISTING,			// always create file
								FILE_ATTRIBUTE_NORMAL,	// normal file
								NULL);					// no attr. template

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return E_OpeningMsgFile;
	}

	// get the file size
	DWORD dwFileSize;
	dwFileSize = GetFileSize((HANDLE)hFile, NULL);

	if (dwFileSize == INVALID_FILE_SIZE)
	{
		CloseHandle((HANDLE)hFile);
		assert(0);
		return E_MsgFileTooLarge;
	}

	// don't read any message files larger than 3MB
	// to do: replace this arbitrary limit with something more intelligent
	DWORD dwSizeLimit = (1024 * 1024) * 3;
	if (dwFileSize > dwSizeLimit)
	{
		CloseHandle((HANDLE)hFile);
		assert(0);
		return E_MsgFileTooLarge;
	}

	// allocate a memory buffer
	TCHAR* pszFileBuff = new TCHAR[dwFileSize];
	assert(pszFileBuff);
	if (!pszFileBuff)
	{
		CloseHandle((HANDLE)hFile);
		return E_MemoryError;
	}

	// read the entire file into memory (most of it will be the msg body)
	DWORD dwRead;
	BOOL bRet = ReadFile((HANDLE)hFile, pszFileBuff, dwFileSize, &dwRead, NULL);
	if (!bRet)
	{
		CloseHandle((HANDLE)hFile);
		delete pszFileBuff;
		assert(0);
		return E_ReadingMsgFile;
	}

	// close the file
	CloseHandle((HANDLE) hFile);

	// do the parsing; note that any malformation of the file in question will
	// result in an error - all start/end tags must be present, regardless of whether
	// they contain data
	int nRet = 0;
	tstring sBuff(pszFileBuff);
	delete pszFileBuff;

	// pop headers
	nRet = TagExtract(sBuff, sPopHeadersStart, sPopHeadersEnd, msgfile.sPopHeaders);
	if (nRet != 0)
		return nRet;
	// cc
	nRet = TagExtract(sBuff, sCcStart, sCcEnd, msgfile.sCc);
	if (nRet != 0)
		return nRet;
	// bcc
	nRet = TagExtract(sBuff, sBccStart, sBccEnd, msgfile.sBcc);
	if (nRet != 0)
		return nRet;
	// to
	nRet = TagExtract(sBuff, sToStart, sToEnd, msgfile.sTo);
	if (nRet != 0)
		return nRet;
	// reply-to
	nRet = TagExtract(sBuff, sReplyToStart, sReplyToEnd, msgfile.sReplyTo);
	if (nRet != 0)
		return nRet;
	// msg header
	nRet = TagExtract(sBuff, sMsgHeaderStart, sMsgHeaderEnd, msgfile.sMsgHeader);
	if (nRet != 0)
		return nRet;
	// msg body
	nRet = TagExtract(sBuff, sBodyStart, sBodyEnd, msgfile.sBody);
	if (nRet != 0)
		return nRet;
	// msg footer
	nRet = TagExtract(sBuff, sFooterStart, sFooterEnd, msgfile.sFooter);
	if (nRet != 0)
		return nRet;

	return 0; // success
}

const UINT CMessageIO::TagExtract(const tstring& sFile, const tstring& sStart, const tstring& sEnd, tstring& sResult)
{
	int nStartPos = std::string::npos;
	int nStopPos = std::string::npos;

	// tag the start and end tag positions
	nStartPos = sFile.find(sStart.c_str(), 0);
	nStopPos = sFile.find(sEnd.c_str(), 0);

	// if either start or stop tag are missing, return
	// invalid msg file error
	if (nStartPos == std::string::npos ||
		nStopPos == std::string::npos)
		return E_InvalidMsgFile;

	// otherwise extract the data between the two tags
	sResult = sFile.substr(nStartPos + sStart.size(), nStopPos - nStartPos - sStart.size());

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// provide a string object placeholder and a valid message id;
// if successful, the function will return TRUE and will have copied
// a valid path into the string placeholder
///////////////////////////////////////////////////////////////////////////
const BOOL CMessageIO::GetInboundMsgPath(tstring &sFileLoc, const UINT nMsgID)
{
	return GenMsgPath(sFileLoc, nMsgID, tstring(_T("InboundMsgFolder")));
}

///////////////////////////////////////////////////////////////////////////
// provide a string object placeholder and a valid message id;
// if successful, the function will return TRUE and will have copied
// a valid path into the string placeholder
///////////////////////////////////////////////////////////////////////////
const BOOL CMessageIO::GetOutboundMsgPath(tstring &sFileLoc, const UINT nMsgID)
{
	return GenMsgPath(sFileLoc, nMsgID, tstring(_T("OutboundMsgFolder")));
}

///////////////////////////////////////////////////////////////////////////
// provide a string object reference that contains the fully
// qualified path to the _current_ attachment file location;
// this path will be changed to reflect the new path/filename
// for the attachment
///////////////////////////////////////////////////////////////////////////
const BOOL CMessageIO::GetInboundAttachPath(tstring &sFileLoc,  int nVerThree)
{
	return GenAttachPath(sFileLoc, EMS_REG_LOC_MAIL_BASE, tstring(_T("InboundAttachFolder")), nVerThree);
}

///////////////////////////////////////////////////////////////////////////
// provide a string object reference that contains the fully
// qualified path to the _current_ attachment file location;
// this path will be changed to reflect the new path/filename
// for the attachment
///////////////////////////////////////////////////////////////////////////
const BOOL CMessageIO::GetNoteAttachPath(tstring &sFileLoc, int nVerThree)
{
	return GenAttachPath(sFileLoc, EMS_LOCATION_IN_REGISTRY, tstring(_T("NoteAttachFolder")), nVerThree);
}

///////////////////////////////////////////////////////////////////////////
// provide a string object reference that contains the fully
// qualified path to the _current_ attachment file location;
// this path will be changed to reflect the new path/filename
// for the attachment
///////////////////////////////////////////////////////////////////////////
const BOOL CMessageIO::GetOutboundAttachPath(tstring &sFileLoc, int nVerThree)
{
	return GenAttachPath(sFileLoc, EMS_REG_LOC_MAIL_BASE, tstring(_T("OutboundAttachFolder")), nVerThree);
}

///////////////////////////////////////////////////////////////////////////
// this over-ride allows you to provide only an attachment filename
// and an empty string object placeholder; if successful, the function
// will return TRUE and populate the string placeholder with the fully
// qualified attachment path; useful if you only have an attachment
// filename (as with incoming mail) and require a storage location
///////////////////////////////////////////////////////////////////////////
const BOOL CMessageIO::GetOutboundAttachPath(const tstring& sFileName, tstring &sFileLoc, int nVerThree)
{
	// filename required for this over-ride
	if (sFileName.size() < 1)
	{
		return E_InvalidParameters;
	}

	sFileLoc = sFileName;

	return GenAttachPath(sFileLoc, EMS_REG_LOC_MAIL_BASE, tstring(_T("OutboundAttachFolder")), nVerThree);
}

///////////////////////////////////////////////////////////////////////////
// private attachment path generator
///////////////////////////////////////////////////////////////////////////
const BOOL CMessageIO::GenAttachPath(tstring &sFileLoc, const tstring& sRegSection, const tstring &RegFolderLoc,  const int nVerThree)
{
#ifdef _UNICODE
	dca::WString szFolder;
#else
	dca::String szFolder;
#endif


	OutputDebugString(_T("CMessageIO::GenAttachPath - sFileLoc contain at least a filename\n"));
	// sFileLoc contain at least a filename (could be the entire
	// original attachment path)
	if (sFileLoc.size() < 1)
	{
		return FALSE;
	}

	OutputDebugString(_T("CMessageIO::GenAttachPath - get the attachment path from the registry\n"));
	// get the attachment path from the registry
	if( ERROR_SUCCESS != GetRegString(EMS_ROOT_KEY, sRegSection.c_str(), RegFolderLoc.c_str(), szFolder))
	{
		return FALSE;
	}
	
	OutputDebugString(szFolder.c_str());
	OutputDebugString(_T("CMessageIO::GenAttachPath - generate an absolute path using the provided filename\n"));
	// generate an absolute path using the provided filename
	if ( szFolder.length() > 0 && szFolder.at(szFolder.size()-1) != '\\')
		szFolder += '\\';

	OutputDebugString(_T("CMessageIO::GenAttachPath - test for version three"));
	if(nVerThree)
	{
		SYSTEMTIME st;
		ZeroMemory(&st, sizeof(SYSTEMTIME));
		::GetLocalTime(&st);

#ifdef _UNICODE
		dca::WString temp;
#else
		dca::String temp;
#endif 
		temp.FromInt(st.wYear);
		szFolder += temp;
		szFolder += _T("\\");

		temp.FromInt(st.wMonth);
		szFolder += temp;
		szFolder += _T("\\");

		temp.FromInt(st.wDay);
		szFolder += temp;
		szFolder += _T("\\");
	}
	else
	{
		szFolder += _T("Signatures\\");
	}
	
	OutputDebugString(_T("CMessageIO::GenAttachPath - check for filename conflicts; rename filename if necessary\n"));
	// check for filename conflicts; rename filename if necessary
	UINT nNameSeq = 2;

	TCHAR szFName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];	
	_tsplitpath(sFileLoc.c_str(), NULL, NULL, szFName, szExt);
	
	sFileLoc = szFolder + tstring(szFName) + tstring(szExt);
	
	while ( VerifyFileExists( sFileLoc.c_str() ) )
	{
		TCHAR szBuff[_MAX_PATH];
		
		if ( _sntprintf(szBuff, _MAX_PATH - 1, _T("%s(%d)"), szFName, nNameSeq++) < 0 )
		{
			return FALSE;
		}
		
		sFileLoc = (szFolder + szBuff) + szExt;
	}

	return TRUE; 
}


///////////////////////////////////////////////////////////////////////////
// private message path generator
///////////////////////////////////////////////////////////////////////////
const BOOL CMessageIO::GenMsgPath(tstring &sFileLoc, const UINT nMsgID, tstring &sRegLoc)
{
	tstring szFolder;
	
	// caller should already have generated a row in the OutboundMessages table
	// and must provide that message ID to this function
	assert(nMsgID >= 0);
	
	// get folder location from reigstry
	if( ERROR_SUCCESS != GetRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, sRegLoc.c_str(), szFolder) )
	{
		return FALSE;
	}
	
	// whackify folder-location if needed
	if (szFolder.length() > 0 && szFolder.at(szFolder.size()-1) != '\\')
		szFolder += '\\';
	
	// generate a zero-padded filename from the message id (facilitates sorting)
	TCHAR szBuff[_MAX_PATH];
	int nRet;
	
	if ((nRet = _sntprintf(szBuff, _MAX_PATH - 1, _T("%s%09d.msg"), szFolder.c_str(), nMsgID)) < 0)
	{
		return FALSE;
	}

	// copy path into return buffer
	sFileLoc = szBuff;
		
	return TRUE; // success
}

const BOOL CMessageIO::GetOutboundSpoolPath(tstring &sFileLoc, const UINT nMsgID)
{
	return GenMsgPath(sFileLoc, nMsgID, tstring(_T("SpoolFolder")));
}

const BOOL CMessageIO::GetQuarantinePath(const tstring &sFileName, tstring &sFileLoc, int nVerThree)
{
	// filename required for this over-ride
	if (sFileName.size() < 1)
	{
		return E_InvalidParameters;
	}

	sFileLoc = sFileName;

	return GenAttachPath(sFileLoc, REG_KEY_AV, tstring(_T("QuarantineFolder")), nVerThree);
}

