/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/SpellCheck.h,v 1.1 2005/04/18 18:48:45 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#if !defined(AFX_SPELLCHECK_H__749548FF_396D_4CE5_BDBC_46DF17294499__INCLUDED_)
#define AFX_SPELLCHECK_H__749548FF_396D_4CE5_BDBC_46DF17294499__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define EMS_SPELL_SCORE				75
#define EMS_SPELL_SUGGEST_LEVEL		50
#define EMS_SPELL_MAX_SUGGESTIONS	5
#define EMS_SPELL_MAX_CUSTOM_LENGTH 256

#include "XMLDataClass.h"
#include "ssce.h"

class CSpellCheck : public CXMLDataClass
{
public:
	CSpellCheck(CISAPIData& ISAPIData);
	virtual ~CSpellCheck();

	int Run( CURLAction& action );

private:
	void CheckString(LPCTSTR szString);
	void Suggest(CEMSString& sString);
	void GetLexiconPath( tstring& tstrPath );
	void LoadUserDictionary(void);

	SSCE_S16 m_SessionID;
	SSCE_S16 m_UserLexID;
	
	map< string, list<CEMSString> > m_suggestion_map;
};

#endif // !defined(AFX_SPELLCHECK_H__749548FF_396D_4CE5_BDBC_46DF17294499__INCLUDED_)
