// SockUtil.h: interface for the CSockUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKUTIL_H__64A0BCFA_AA39_4F33_900D_A0AB55481DFE__INCLUDED_)
#define AFX_SOCKUTIL_H__64A0BCFA_AA39_4F33_900D_A0AB55481DFE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSockUtil  
{
public:
	CSockUtil();
	virtual ~CSockUtil();

	void GetIPString(const DWORD dwIP, LPTSTR szBuffer);
};

#endif // !defined(AFX_SOCKUTIL_H__64A0BCFA_AA39_4F33_900D_A0AB55481DFE__INCLUDED_)
