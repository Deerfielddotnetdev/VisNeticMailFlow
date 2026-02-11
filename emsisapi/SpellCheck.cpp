/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/SpellCheck.cpp,v 1.2.2.1 2005/12/13 18:11:49 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "SpellCheck.h"
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CSpellCheck::CSpellCheck(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{	
	SSCE_S16 result;	
	tstring sPath;
	
	// set the registration key
	SSCE_SetKey(0x3E59C65E);
	
	// open a session
	m_SessionID = SSCE_OpenSession();

	if ( m_SessionID < 0)
	{
		CEMSString sError;
		sError.Format( _T("Error (%d) opening spellcheck session"), m_SessionID );
		THROW_EMS_EXCEPTION( E_SpellCheckFailed, sError );
	}
	
	// load the user dictionary
	LoadUserDictionary();

	GetLexiconPath( sPath );

	tstring sTlx(_T("ssceam.tlx"));
	tstring sClx(_T("ssceam2.clx"));

	int nDID = 1;
	GetISAPIData().GetFormLong( _T("did"), nDID, true);
	dca::String f;
	f.Format("CSpellCheck::CSpellCheck - Spell check DictionaryID [%d]", nDID);
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	
	if(nDID > 1)
	{
		f.Format("CSpellCheck::CSpellCheck - Spell check DictionaryID [%d] greater than 1",nDID);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			
		TDictionary dc;
		GetQuery().Initialize();
		BINDCOL_TCHAR( GetQuery(), dc.m_TlxFile );
		BINDCOL_TCHAR( GetQuery(), dc.m_ClxFile );
		BINDPARAM_LONG( GetQuery(), nDID );
		GetQuery().Execute( _T("SELECT TlxFile,ClxFile ")
					_T("FROM Dictionary ")
					_T("WHERE DictionaryID=?") );	
		if(GetQuery().Fetch() == S_OK )
		{
			f.Format("CSpellCheck::CSpellCheck - Spell check TLX File [%s] for DictionaryID [%d]", dc.m_TlxFile,nDID);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			sTlx.assign(dc.m_TlxFile);
			f.Format("CSpellCheck::CSpellCheck - Spell check CLX File [%s] for DictionaryID [%d]", dc.m_ClxFile,nDID);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			sClx.assign(dc.m_ClxFile);
		}
	}
	
	// open the text lexicon file... this file will be searched first 
	// it contains common words and can be searched the quickest
	tstring sFile = sPath + sTlx;

	f.Format("CSpellCheck::CSpellCheck - Spell check TLX File [%s]", sFile.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	if ( (result = SSCE_OpenLex( m_SessionID, sFile.c_str(), -1 )) < 0 )
	{
		CEMSString sError;
		sError.Format( _T("Error (%d) opening dictionary file %s"), result, sFile.c_str() );
		THROW_EMS_EXCEPTION( E_SpellCheckFailed, sError );
	}
	
	// open the main lexicon file...
	// if we purchased the source SDK we could open this file once and share it 
	// between threads...

	sFile = sPath + sClx;
	
	f.Format("CSpellCheck::CSpellCheck - Spell check CLX File [%s]", sFile.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	if ( (result = SSCE_OpenLex( m_SessionID, sFile.c_str(), -1 )) < 0 )
	{
		CEMSString sError;
		sError.Format( _T("Error (%d) opening dictionary file %s"), result, sFile.c_str() );
		THROW_EMS_EXCEPTION( E_SpellCheckFailed, sError );
	}
	
	SSCE_SetOption( m_SessionID, SSCE_IGNORE_DOMAIN_NAMES_OPT, TRUE);
	SSCE_SetOption( m_SessionID, SSCE_SUGGEST_PHONETIC_OPT, TRUE);
	SSCE_SetOption( m_SessionID, SSCE_SUGGEST_TYPOGRAPHICAL_OPT, TRUE);
	SSCE_SetOption( m_SessionID, SSCE_SUGGEST_SPLIT_WORDS_OPT, TRUE);	
	SSCE_SetOption( m_SessionID, SSCE_REPORT_DOUBLED_WORD_OPT, TRUE);
	// Added for French Dictionary
	SSCE_SetOption( m_SessionID, SSCE_SPLIT_CONTRACTED_WORDS_OPT, TRUE);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Destruction	              
\*--------------------------------------------------------------------------*/
CSpellCheck::~CSpellCheck()
{
	//DEBUGPRINT(_T("* EMSIsapi.~CSpellCheck - Closing session for Sentry Spelling Checker Engine."));

	SSCE_CloseSession( m_SessionID );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Perform the appropriate action	              
\*--------------------------------------------------------------------------*/
int CSpellCheck::Run( CURLAction& action )
{
	CEMSString sBodyText;
	int nCheckOriginal = 0;
	int nIsHtml = 0;

	GetISAPIData().GetFormString( _T("BODYTEXT"), sBodyText, false, true);
	sBodyText.ToAscii();
	GetISAPIData().GetFormLong( _T("ishtml"), nIsHtml, true);

	// set spell check options
	if(nIsHtml)
		SSCE_SetOption( m_SessionID, SSCE_IGNORE_HTML_MARKUPS_OPT, 1);
	else
		SSCE_SetOption( m_SessionID, SSCE_IGNORE_HTML_MARKUPS_OPT, 0);
	
	// perform the spell-check
	CheckString( sBodyText.c_str() );

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Checks a string for misspelled words	              
\*--------------------------------------------------------------------------*/
void CSpellCheck::CheckString( LPCTSTR szString )
{
	SSCE_CHAR szBadWord[SSCE_MAX_WORD_SZ];
	SSCE_CHAR szAltWord[SSCE_MAX_WORD_SZ];
	SSCE_S32 pos = 0;
	SSCE_S16 result;
	
	CEMSString sBadWord;
	CEMSString sBadWordEsc;
	sBadWord.reserve(SSCE_MAX_WORD_SZ);
	sBadWordEsc.reserve(SSCE_MAX_WORD_SZ);

	//DEBUGPRINT(_T("* EMSIsapi.CSpellCheck.CheckString - Checking the string."));

	// check the string...
	while ( true )
	{
		result = SSCE_CheckString( m_SessionID, (SSCE_CHAR*) szString, &pos, szBadWord, 80, 
			szAltWord, 80 );

		if ( result < 0 )
		{
			CEMSString sError;
			sError.Format( _T("Error (%d) during spellcheck"), result );
			THROW_EMS_EXCEPTION( E_SpellCheckFailed, sError );
		}
			
		if ( result & SSCE_END_OF_BLOCK_RSLT )
			break;
	
		if ( result & SSCE_MISSPELLED_WORD_RSLT )
		{
			// save off the misspelled word
			sBadWord = (char*) szBadWord;

			// MJM - this is a hack to make the custom dictionary case-insensitive
			// convert the misspelled word to lower-case
			SSCE_CHAR* p = szBadWord;
			while (*p != _T('\0'))
			{
				*p = tolower(*p);
				p++;
			}

			// attempt to find the lower-case version of the misspelled word
			// using only the custom dictionary lexicon
			result = SSCE_FindLexWord( m_SessionID, m_UserLexID, szBadWord, szAltWord, 80 );
			
			// if it wasn't found
			if ( result != SSCE_IGNORE_ACTION )
			{
				//DEBUGPRINT(_T("* EMSIsapi.CSpellCheck.CheckString - Word was not found."));

				sBadWordEsc = sBadWord;
				sBadWordEsc.EscapeJavascript();
				sBadWordEsc.ToUTF8();

				// add it to the XML
				GetXMLGen().AddChildElem( _T("misspelled_word") );
				GetXMLGen().SetChildAttrib( _T("word"), sBadWordEsc.c_str() );
				Suggest( sBadWord );
			}
		}
		else if ( result & SSCE_DOUBLED_WORD_RSLT )
		{
			//DEBUGPRINT(_T("* EMSIsapi.CSpellCheck.CheckString - Word was found twice."));

			sBadWordEsc = (char*) szBadWord;
			sBadWordEsc.EscapeJavascript();
			sBadWordEsc.ToUTF8();

			// add it to the XML
			GetXMLGen().AddChildElem( _T("double_word") );
			GetXMLGen().SetChildAttrib( _T("word"), sBadWordEsc.c_str() );				
		}
	
		pos += _tcslen((char*) szBadWord);
	}

	// add summary information
	SSCE_S32 nWordCount = SSCE_CountStringWords( m_SessionID, (SSCE_CHAR*) szString);
	
	GetXMLGen().AddChildElem( _T("Summary") );
	GetXMLGen().SetChildAttrib( _T("WordCount"), nWordCount );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists suggestions for a misspelled word	   
||
||				The suggestions for each word are added to a map this prevents
||				duplicate calls to SSCE_SUGGEST which are quite slow.  Ideally
||				the XSL template would support a single suggestion structure for
||				each unique misspelled word.
\*--------------------------------------------------------------------------*/
void CSpellCheck::Suggest(CEMSString& sString)
{	
	map< string, list<CEMSString> >::iterator map_iter;


	// try to find the suggestions for this word in the map...
	if ( (map_iter = m_suggestion_map.find(sString)) == m_suggestion_map.end() )
	{

		// get the suggestions from the spelling library if they are not in the map...
		list<CEMSString> suggestion_list;

		// list suggestions...
		SSCE_S16 result;
		SSCE_CHAR szSuggestions[(SSCE_MAX_WORD_SZ * EMS_SPELL_MAX_SUGGESTIONS) + 1];
		SSCE_S16 scores[EMS_SPELL_MAX_SUGGESTIONS];
	
		result = SSCE_Suggest( m_SessionID, (SSCE_CHAR*) sString.c_str(), EMS_SPELL_SUGGEST_LEVEL, szSuggestions, 
							   405, scores, 4 );

		// check for error
		if ( result < 0 )
		{
			CEMSString sError;
			sError.Format( _T("Error (%d) generating suggestions for %s"), result, sString.c_str());
			THROW_EMS_EXCEPTION( E_SpellCheckFailed, sError );
		}

		// build the list of suggestions
		CEMSString sSuggestion;
		sSuggestion.reserve(SSCE_MAX_WORD_SZ);
	
		for (SSCE_CHAR* p = szSuggestions, i = 0; *p != '\0'; p += (strlen( (char*) p) + 1), i++) 
		{
			// only return words with a high enough score
			if (scores[i] >= EMS_SPELL_SCORE)
			{
				sSuggestion = (char*) p;
				sSuggestion.EscapeJavascript();
				sSuggestion.ToUTF8();
				suggestion_list.push_back( sSuggestion );
			}
		}

		// add the list of suggestions to the map
		map_iter = m_suggestion_map.insert( pair<tstring, list<CEMSString> > ( sString, suggestion_list) ).first;
	}

	// loop through the list of suggestions
	list<CEMSString>::iterator list_iter;
		
	GetXMLGen().IntoElem();

	for ( list_iter = map_iter->second.begin(); list_iter != map_iter->second.end(); list_iter++ )
	{
		GetXMLGen().AddChildElem( _T("suggestion") );
		GetXMLGen().SetChildAttrib( _T("word"), list_iter->c_str() );	
	}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the path in which the lexicon files are stored	              
\*--------------------------------------------------------------------------*/
void CSpellCheck::GetLexiconPath( tstring& tstrPath )
{
	// get the installation path from the registry
	if ( GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_INSTALL_PATH_VALUE, tstrPath ) != ERROR_SUCCESS)
		THROW_EMS_EXCEPTION( E_SpellCheckFailed, _T("Error reading InstallPath key from registry"));
	
	// make sure it has a backslash at the end.
	if( tstrPath.length() > 0 && tstrPath.at( tstrPath.length() - 1 ) != _T('\\') )
		tstrPath += _T("\\");

	// append the rest of the path
	tstrPath += _T("bin\\spellcheck\\");
}


/*---------------------------------------------------------------------------\            
||  Comments:	Loads the agent's custom dictionary.  Global entries
||				are loaded as well.	              
\*--------------------------------------------------------------------------*/
void CSpellCheck::LoadUserDictionary( void )
{
	TCHAR szWord[EMS_SPELL_MAX_CUSTOM_LENGTH];
	CEMSString sWord;
	// create the user lexicon
	if ( (m_UserLexID = SSCE_CreateLex( m_SessionID, NULL, SSCE_ANY_LANG ) < 0) )
		THROW_EMS_EXCEPTION( E_SpellCheckFailed, _T("Unable to create user lexicon"));

	// query the database and build the user lexicon
	GetQuery().Initialize();
	
	BINDCOL_TCHAR_NOLEN( GetQuery(), szWord );
	BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
	
	GetQuery().Execute( _T("SELECT Word FROM CustomDictionary WHERE AgentID IN (0,?)") );
	
	while ( GetQuery().Fetch() == S_OK )
	{
		sWord.Format(_T("%s"),szWord);
		sWord.ToAscii();		
		_tcsncpy( szWord, sWord.c_str(), EMS_SPELL_MAX_CUSTOM_LENGTH );
		SSCE_AddToLex( m_SessionID, m_UserLexID, (const SSCE_CHAR*) szWord, SSCE_IGNORE_ACTION, NULL );
	}
}