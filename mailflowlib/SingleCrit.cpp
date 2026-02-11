/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/SingleCrit.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	A class based wrapper for critcal sections
||				used in conjunction with CCriticalSection
||              
\\*************************************************************************/

#include "stdafx.h"
#include "SingleCrit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSingleCrit::CSingleCrit(CCriticalSection* pCritSection, const bool bCreateLocked)
{
	m_pCritSection = pCritSection;

	m_bHasLock = false;

	if (bCreateLocked)
		Lock();
}

CSingleCrit::~CSingleCrit()
{
	if (m_bHasLock)
		Unlock();
}

void CSingleCrit::Lock()
{
	EnterCriticalSection(&m_pCritSection->m_crit);

	m_bHasLock = true;
}

void CSingleCrit::Unlock()
{
	LeaveCriticalSection(&m_pCritSection->m_crit);

	m_bHasLock = false;
}
