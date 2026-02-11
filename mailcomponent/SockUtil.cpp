// SockUtil.cpp: implementation of the CSockUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SockUtil.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSockUtil::CSockUtil()
{

}

CSockUtil::~CSockUtil()
{

}

/*---------------------------------------------------------------------\\
|| provides a formatted IP address string given an IP address
\*---------------------------------------------------------------------*/
void CSockUtil::GetIPString(const DWORD dwIP, LPTSTR szBuffer)
{
	_stprintf(	szBuffer, 
				_T("%d.%d.%d.%d"), 
				(BYTE)(dwIP >> 0),
				(BYTE)(dwIP >> 8),
				(BYTE)(dwIP >> 16),
				(BYTE)(dwIP >> 24));	
}