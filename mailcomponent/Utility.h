// Utility.h: interface for the CUtility class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTILITY_H__9086AFA8_D31D_424E_98F0_A4443DB9D0B8__INCLUDED_)
#define AFX_UTILITY_H__9086AFA8_D31D_424E_98F0_A4443DB9D0B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUtility  
{
public:
	CUtility();
	virtual ~CUtility();

public:
	static const bool EnsurePathExists(tstring sPath);
	static tstring GetErrorString(const DWORD dwErr);

};

#endif // !defined(AFX_UTILITY_H__9086AFA8_D31D_424E_98F0_A4443DB9D0B8__INCLUDED_)
