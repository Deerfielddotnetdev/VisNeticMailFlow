/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/CustomDictionary.cpp,v 1.2.2.1 2005/12/13 18:11:50 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Supports management of user spellcheck dictionaries   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "CustomDictionary.h"

/*---------------------------------------------------------------------------\                       
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CCustomDictionary::CCustomDictionary(CISAPIData& ISAPIData) : CXMLDataClass( ISAPIData )
{
	m_nAgentID = 0;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CCustomDictionary::Run( CURLAction& action )
{
	GetISAPIData().GetURLLong( _T("AgentID"), m_nAgentID );

	tstring sAction;
	GetISAPIData().GetXMLString( _T("action"), sAction, true );

	if ( sAction.compare( _T("insert") ) == 0 )
	{
		AddToUserDictionary( action );
	}
	else if ( sAction.compare( _T("delete") ) == 0 )
	{
		DeleteFromUserDictionary( action );
	}
	else
	{
		ListUserDictionary( action );
	}

	return 0;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Lists an agents custom dictionary
\*--------------------------------------------------------------------------*/
void CCustomDictionary::ListUserDictionary( CURLAction& action )
{
	// check security
	unsigned char access;

	if ( m_nAgentID )
	{
		access = RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_nAgentID, EMS_READ_ACCESS );
	}
	else
	{
		RequireAdmin();
		access = EMS_DELETE_ACCESS;
	}

	GetXMLGen().AddChildElem( _T("Access") );
	GetXMLGen().AddChildAttrib( _T("Level"), access );

	TCustomDictionary CustomEntry;

	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), CustomEntry.m_CustomDictionaryID );
	BINDCOL_TCHAR_NOLEN( GetQuery(), CustomEntry.m_Word );
	BINDPARAM_LONG( GetQuery(), m_nAgentID );
	
	GetQuery().Execute( _T("SELECT CustomDictionaryID, Word FROM CustomDictionary WHERE AgentID=?") );
	
	while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("CUSTOM_ENTRY") );
		GetXMLGen().AddChildAttrib( _T("ID"), CustomEntry.m_CustomDictionaryID );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("WORD"), CustomEntry.m_Word );
		GetXMLGen().OutOfElem();
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Deletes entries from an agent's custom dictionary.
||				Supports the deletion of multiple entries.
\*--------------------------------------------------------------------------*/
void CCustomDictionary::DeleteFromUserDictionary( CURLAction& action )
{
	// check security
	m_nAgentID ? RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_nAgentID, EMS_EDIT_ACCESS ) : RequireAdmin();

	CEMSString sQuery;
	CEMSString sSelectID;
	tstring sChunk;

	GetISAPIData().GetXMLString( _T("selectID"), sSelectID );

	while ( sSelectID.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		GetQuery().Initialize();
		
		BINDPARAM_LONG( GetQuery(), m_nAgentID );

		sQuery.Format( _T("DELETE FROM CustomDictionary WHERE CustomDictionaryID IN (%s) AND AgentID=?"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );
	}
}

/*---------------------------------------------------------------------------\            
||  Comments:	Adds an entry to an agent's custom dictionary
\*--------------------------------------------------------------------------*/
void CCustomDictionary::AddToUserDictionary( CURLAction& action )
{
	// check security
	if ( m_nAgentID )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_nAgentID, EMS_EDIT_ACCESS );
	}
	else
	{
		DISABLE_IN_DEMO();
		RequireAdmin();
	}

	TCustomDictionary CustomEntry;
	CustomEntry.m_AgentID = m_nAgentID;
	GetISAPIData().GetXMLTCHAR( _T("WORD"), CustomEntry.m_Word, 256 );
	
	// check that the word isn't already there
	GetQuery().Initialize();
	
	BINDPARAM_LONG( GetQuery(), CustomEntry.m_AgentID );
	BINDPARAM_TCHAR( GetQuery(), CustomEntry.m_Word );
	
	GetQuery().Execute( _T("SELECT CustomDictionaryID FROM CustomDictionary WHERE AgentID=? AND Word=?") );
	
	if ( GetQuery().Fetch() == S_OK )
		THROW_VALIDATION_EXCEPTION( _T("WORD"), _T("The specified word already exists in the custom dictionary") );

	// add the word to the dictionary	
	CustomEntry.Insert( GetQuery() );
}