/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/VoipDialingCodes.cpp,v 1.2.2.1 2005/12/13 18:11:49 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Voip DialingCodes
||              
\\*************************************************************************/

#include "stdafx.h"
#include "VoipDialingCodes.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CVoipDialingCodes::CVoipDialingCodes( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{	
}

CVoipDialingCodes::~CVoipDialingCodes()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CVoipDialingCodes::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_VoipDialingCodeID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_VoipDialingCodeID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_VoipDialingCodeID);
		}
	}
	
	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( GetISAPIData().m_sPage.compare( _T("voipdialingcode") ) == 0 )
	{
		if( sAction.compare( _T("insert") ) == 0 )
		{
			DISABLE_IN_DEMO();
			New();
			return 0;
		}
		else if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();

			if( m_VoipDialingCodeID == 0 )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			
			Update();
			return 0;
		}
		
		// change title if ID is zero
		if( m_VoipDialingCodeID == 0 )
		{
			action.m_sPageTitle.assign( "New Voip Dial Code" );
		}
		else
		{
			action.m_sPageTitle.assign( "Edit Voip Dial Code" );
		}
		
		return Query();
	}
	
	if( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();

		if( m_VoipDialingCodeID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

		Delete();
	}

	action.m_sPageTitle.assign( "VOIP Dial Codes" );
	return ListAll();	
	
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query all voip DialingCodes
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipDialingCodes::ListAll(void)
{
	RequireAdmin();
	if (!GetISAPIData().GetURLLong( _T("VSID"), m_VoipServerID, true ))
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

	GetXMLGen().AddChildElem( _T("VoipDialingCodes") );
	GetXMLGen().AddChildAttrib( _T("VSID"), m_VoipServerID );
	
	TVoipServers vs;
	vs.m_VoipServerID = m_VoipServerID;
	vs.Query(GetQuery());
	
	GetXMLGen().AddChildAttrib( _T("ServerDescription"), vs.m_Description );		
	
	
	PrepareList( GetQuery() );
	
	GetXMLGen().IntoElem();
	
    while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("DialingCode") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_VoipDialingCodeID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
		GetXMLGen().AddChildAttrib( _T("IsDefault"), m_IsDefault );
		GetXMLGen().AddChildAttrib( _T("DialingCode"), m_DialingCode );				
	}

	GetXMLGen().OutOfElem();	

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query for a particular voip DialingCode
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipDialingCodes::Query(void)

{
	CEMSString sID;
	
	if (!GetISAPIData().GetURLLong( _T("ID"), m_VoipDialingCodeID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_VoipDialingCodeID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_VoipDialingCodeID);
		}
	}

	if (!GetISAPIData().GetURLLong( _T("VSID"), m_VoipServerID, true ))
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

	if ( m_VoipDialingCodeID > 0 )
	{
		TVoipDialingCodes::Query(GetQuery());
	}
	else
	{
		//Set defaults for a new code
		m_IsEnabled = 1;
		m_IsDefault = 0;		
	}
	
	GenerateXML();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update a code
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipDialingCodes::Update()
{
	DecodeForm();

	//Check for unique Description
	CheckForDuplicity();
	if(m_IsDefault)
	{
		ClearDefault();
	}
	else
	{
		SetDefault();
	}

	TVoipDialingCodes::Update(GetQuery());
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a code
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipDialingCodes::Delete()
{
	if ( m_VoipDialingCodeID )
	{
		TVoipDialingCodes::Delete(GetQuery());		
	}
	
	return 0;
}

void CVoipDialingCodes::CheckForDuplicity( void )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_VoipDialingCodeID );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	BINDPARAM_LONG( GetQuery(), m_VoipServerID );
	GetQuery().Execute( _T("SELECT VoipDialingCodeID FROM VoipDialingCodes ")
						_T("WHERE VoipDialingCodeID<>? AND Description=? AND VoipServerID=? ") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("A Voip Dialing Code with that description already exists, please enter a unique Description!")  );
}

void CVoipDialingCodes::ClearDefault( void )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_VoipDialingCodeID );
	BINDPARAM_LONG( GetQuery(), m_VoipServerID );
	GetQuery().Execute( _T("UPDATE VoipDialingCodes SET IsDefault=0 ")
						_T("WHERE VoipDialingCodeID<>? AND VoipServerID=? ") );
	
}

void CVoipDialingCodes::SetDefault( void )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_VoipDialingCodeID );
	BINDPARAM_LONG( GetQuery(), m_VoipServerID );
	GetQuery().Execute( _T("SELECT VoipDialingCodeID FROM VoipDialingCodes ")
						_T("WHERE VoipDialingCodeID<>? AND IsDefault=1 AND VoipServerID=? ") );

	if( GetQuery().GetRowCount() == 0 )
	{
		m_IsDefault = 1;
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create a new code
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipDialingCodes::New()
{
	DecodeForm();

	//Check for unique Description
	CheckForDuplicity();
	if(m_IsDefault)
	{
		ClearDefault();
	}
	else
	{
		SetDefault();
	}

	TVoipDialingCodes::Insert( GetQuery() );

	return 0;
}

void CVoipDialingCodes::DecodeForm(void)
{
	// Description
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, 256 );
	if ( _tcslen( m_Description ) < 1 )
		THROW_VALIDATION_EXCEPTION( _T("Description"), _T("Please specify a description for the dialing code") );

	// ServerID
	GetISAPIData().GetXMLLong( _T("VoipServerID"), m_VoipServerID );

	// Enabled?
	GetISAPIData().GetXMLLong( _T("IsEnabled"), m_IsEnabled );

	// Default?
	GetISAPIData().GetXMLLong( _T("IsDefault"), m_IsDefault );

	// Dialing Code
	GetISAPIData().GetXMLTCHAR( _T("DialingCode"), m_DialingCode, 256 );
	
}

void CVoipDialingCodes::GenerateXML(void)
{
	GetXMLGen().AddChildElem( _T("DialingCode") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_VoipDialingCodeID );
	GetXMLGen().AddChildAttrib( _T("VSID"), m_VoipServerID );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	GetXMLGen().AddChildAttrib( _T("IsDefault"), m_IsDefault );
	GetXMLGen().AddChildAttrib( _T("DialingCode"), m_DialingCode );	
}
