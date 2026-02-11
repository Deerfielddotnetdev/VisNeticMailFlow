// ServerParameters.h: interface for the CServerParameters class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERPARAMETERS_H__D5CE4BDA_61B4_46BA_9873_C6C270B45824__INCLUDED_)
#define AFX_SERVERPARAMETERS_H__D5CE4BDA_61B4_46BA_9873_C6C270B45824__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CriticalSection.h"
#include <map>
using namespace std;

class CServerParameters  
{
public:
	const tstring GetParameter(const UINT nID, const tstring& sDefault);
	const int GetParameter(const UINT nID, const int nDefault);
	const BOOL SetParameter(UINT nID, const int nValue);
	const BOOL SetParameter(UINT nID, tstring& sValue);

	// params will be loaded automatically when the singleton instance
	// is accessed, but this can be called directly to force a refresh as desired
	const int Refresh();

	// this is the only way to get an instance of this singleton class
	static CServerParameters& GetInstance();

	virtual ~CServerParameters();

private:
	CServerParameters();
	const int LoadParams();

	typedef map<UINT, tstring> TMAP;
	TMAP m_map;
	bool m_bParamsLoaded;
};

#endif // !defined(AFX_SERVERPARAMETERS_H__D5CE4BDA_61B4_46BA_9873_C6C270B45824__INCLUDED_)
