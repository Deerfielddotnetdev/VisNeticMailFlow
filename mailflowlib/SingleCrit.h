/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/SingleCrit.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	A class based wrapper for critcal sections
||				used in conjunction with CCriticalSection
||              
\\*************************************************************************/

#if !defined(AFX_SINGLECRIT_H__7129BB97_509A_4C76_A95C_F1223909A61D__INCLUDED_)
#define AFX_SINGLECRIT_H__7129BB97_509A_4C76_A95C_F1223909A61D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CriticalSection.h"

class CSingleCrit  
{
public:
	CSingleCrit(CCriticalSection* pCritSection, const bool bCreateLocked = true);
	virtual ~CSingleCrit();

	void Unlock();
	void Lock();
	
private:
	CCriticalSection* m_pCritSection;
	bool m_bHasLock;
};

#endif // !defined(AFX_SINGLECRIT_H__7129BB97_509A_4C76_A95C_F1223909A61D__INCLUDED_)
