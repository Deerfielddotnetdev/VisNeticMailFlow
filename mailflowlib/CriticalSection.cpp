/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/CriticalSection.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||                                         
||  COMMENTS:	A class based wrapper for critcal sections
||				used in conjunction with CSingleCrit
||              
\\*************************************************************************/

#include "stdafx.h"
#include "CriticalSection.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCriticalSection::CCriticalSection()
{
	InitializeCriticalSection(&m_crit);
}

CCriticalSection::~CCriticalSection()
{
	DeleteCriticalSection(&m_crit);
}
