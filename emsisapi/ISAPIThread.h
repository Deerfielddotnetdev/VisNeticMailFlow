// ISAPIThread.h: interface for the CISAPIThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISAPITHREAD_H__D2CC35FD_5DD4_45B9_AF0D_3DA90E50770E__INCLUDED_)
#define AFX_ISAPITHREAD_H__D2CC35FD_5DD4_45B9_AF0D_3DA90E50770E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Thread.h"
#include "XMLDataClass.h"

//-------------------------------------------------------------------------
// packet to assist in transmitting a file
//-------------------------------------------------------------------------

class CFilePacket
{
public:
	enum{MAX_HEADERS = 1024};

	CFilePacket():
	m_hFile(INVALID_HANDLE_VALUE)
	{}
	~CFilePacket()
	{
		if(INVALID_HANDLE_VALUE != m_hFile)
		::CloseHandle(m_hFile);
	}

	HANDLE	m_hFile;
	TCHAR	m_headers[MAX_HEADERS];
};

class CISAPIThread : public CThread  
{
public:
	CISAPIThread( CSharedObjects* pShared );
	virtual ~CISAPIThread();

	void SetSharedObject( CSharedObjects* pShared ) { m_pShared = pShared; }
	
	// one-time initialization
	int GetSourcePaths(void);

	void InitializeRequest( void );
	void FinishRequest(unsigned int id);
	void GetServerSecure(void);
	void GetSessionCookie(void);
	void SetCookieHeader(void);
	void SendHeaders( TCHAR* szContentType, DWORD dwBytes, bool bIsAttach = false );
	int SendString( tstring& html, bool bIsAttach = false );
	int SendXML( tstring& xml );
	void Redirect( const TCHAR* szPage );
	void ReturnFileNotFound( void );
	void GetClientIP(dca::String& IP );
	void GetUserAgent( tstring& sUserAgent );
	void GetUserOS( tstring& sUserOS );
	int SendFile( const TCHAR* szFileOnDisk, const TCHAR* szFileName, 
		          const TCHAR* szMediaType );
	bool SyncWrite(const char* lpResponse, DWORD& dwResponse);

	unsigned int m_id;

protected:

	void AddHeaders( TCHAR* szHeaders );
	void FormatHeaders( TCHAR* szContentType, DWORD dwContentLength, bool bIsAttach = false );
	static void CALLBACK FileComplete(EXTENSION_CONTROL_BLOCK* pECB,PVOID pContext, DWORD count, DWORD error);
	
	EXTENSION_CONTROL_BLOCK*	m_pECB;
	TCHAR						m_szURL[MAX_PATH];
	TCHAR						m_szServerName[MAX_PATH];
	TCHAR						m_szURLSubDir[MAX_PATH];
	TCHAR						m_szHeaderStatus[64];
	CEMSString 					m_szHeader;
	TCHAR						m_szSessionID[SESSIONID_LENGTH+1];

	BYTE						m_buffer[8192];
	BOOL						m_bSessionCookieFound;
	BOOL						m_bHeaderSet;
	TCHAR						m_szServerPort[16];
	bool						m_bHTTPS;
};

#define COOKIE_NAME "EMSSessionID="

#endif // !defined(AFX_ISAPITHREAD_H__D2CC35FD_5DD4_45B9_AF0D_3DA90E50770E__INCLUDED_)
