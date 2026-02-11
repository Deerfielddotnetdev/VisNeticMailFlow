/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/CriticalSection.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	A class based wrapper for critcal sections
||				used in conjunction with CSingleCrit
||              
\\*************************************************************************/

#if !defined(AFX_CRITICALSECTION_H__5324540E_9A2A_435C_82CA_186D46EF0573__INCLUDED_)
#define AFX_CRITICALSECTION_H__5324540E_9A2A_435C_82CA_186D46EF0573__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSingleCrit;

class CCriticalSection  
{
public:
	CCriticalSection();
	virtual ~CCriticalSection();

friend class CSingleCrit;
	
private:
	CRITICAL_SECTION m_crit;

};

#endif // !defined(AFX_CRITICALSECTION_H__5324540E_9A2A_435C_82CA_186D46EF0573__INCLUDED_)
