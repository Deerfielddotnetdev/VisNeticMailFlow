// SharedObjects.h: interface for the CSharedObjects class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHAREDOBJECTS_H__4737003E_2C7F_45B9_8B29_91D2C94B2161__INCLUDED_)
#define AFX_SHAREDOBJECTS_H__4737003E_2C7F_45B9_8B29_91D2C94B2161__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "JobQueue.h"
#include "URLMap.h"
#include "SessionMap.h"
#include "XMLCache.h"
#include "AccessControl.h"
#include "LicenseMgr.h"

class CSharedObjects  
{
public:
	CSharedObjects();
	virtual ~CSharedObjects();

	CJobQueue& Queue(void) { return m_Queue; }
	CURLMap& URLMap(void) { return m_URLMap; }
	CSessionMap& SessionMap(void) { return m_SessionMap; }
	CXMLCache& XMLCache(void) { return m_XMLCache; }
	CLicenseMgr& LicenseMgr(void) { return m_LicenseMgr; }	
	
	bool				m_bDBMaintenanceRunning;
	dca::String			sWebType;
	bool				m_bUseATC;
	bool				m_bFullText;
	UINT				m_nIISVer;
	UINT				m_nCharSet;
	UINT				m_nTimeZone;

protected:
	CJobQueue			m_Queue;
	CURLMap				m_URLMap;
	CSessionMap			m_SessionMap;
	CXMLCache			m_XMLCache;
	CLicenseMgr			m_LicenseMgr;
};

#endif // !defined(AFX_SHAREDOBJECTS_H__4737003E_2C7F_45B9_8B29_91D2C94B2161__INCLUDED_)
