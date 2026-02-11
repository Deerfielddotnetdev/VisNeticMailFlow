/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/VoipExtensions.cpp,v 1.2.2.1 2005/12/13 18:11:49 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Voip Extensions
||              
\\*************************************************************************/

#include "stdafx.h"
#include "VoipExtensions.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CVoipExtensions::CVoipExtensions( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{	
}

CVoipExtensions::~CVoipExtensions()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CVoipExtensions::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	//RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_VoipExtensionID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_VoipExtensionID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_VoipExtensionID);
		}
	}
	
	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( GetISAPIData().m_sPage.compare( _T("voipextension") ) == 0 )
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

			if( m_VoipExtensionID == 0 )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			
			Update();
			return 0;
		}
		
		// change title if ID is zero
		if( m_VoipExtensionID == 0 )
		{
			action.m_sPageTitle.assign( "New Voip Extension" );
		}
		else
		{
			action.m_sPageTitle.assign( "Edit Voip Extension" );
		}
		
		return Query();
	}
	
	if( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();

		if( m_VoipExtensionID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

		Delete();
	}

	action.m_sPageTitle.assign( "VOIP Extensions" );
	return ListAll();	
	
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query all voip Extensions
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipExtensions::ListAll(void)
{
	//RequireAdmin();

	//m_AgentID = GetAgentID();
	GetISAPIData().GetURLLong( _T("AgentID"), m_AgentID );
	
	PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("VoipExtensions") );
	GetXMLGen().IntoElem();
	
    while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Extension") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_VoipExtensionID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
		GetXMLGen().AddChildAttrib( _T("ServerID"), m_VoipServerID );
		GetXMLGen().AddChildAttrib( _T("IsDefault"), m_IsDefault );
		GetXMLGen().AddChildAttrib( _T("Extension"), m_Extension );				
	}

	GetXMLGen().OutOfElem();

	TVoipServers vs;
	GetXMLGen().AddChildElem( _T("VoipServers") );
	GetXMLGen().IntoElem();
	vs.PrepareList(GetQuery());
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Server") );
		GetXMLGen().AddChildAttrib( _T("VSID"), vs.m_VoipServerID );
		GetXMLGen().AddChildAttrib( _T("Description"), vs.m_Description );			
	}
	GetXMLGen().OutOfElem();	

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query for a particular voip Extension
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipExtensions::Query(void)

{
	CEMSString sID;
	
	if (!GetISAPIData().GetURLLong( _T("ID"), m_VoipExtensionID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_VoipExtensionID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_VoipExtensionID);
		}
	}

	if ( m_VoipExtensionID > 0 )
	{
		TVoipExtensions::Query(GetQuery());
	}
	else
	{
		//Set defaults for a new Extension
		m_IsEnabled = 1;
		m_IsDefault = 0;		
	}
	
	GenerateXML();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update a scheduled report
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipExtensions::Update()
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

	TVoipExtensions::Update(GetQuery());
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a Extension
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipExtensions::Delete()
{
	if ( m_VoipExtensionID )
	{
		TVoipExtensions::Delete(GetQuery());		
	}
	
	return 0;
}

void CVoipExtensions::CheckForDuplicity( void )
{
	//int nAgentID = GetAgentID();
	int nAgentID=0;
	GetISAPIData().GetURLLong( _T("AgentID"), nAgentID );
	
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_VoipExtensionID );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	BINDPARAM_LONG( GetQuery(), nAgentID );
	GetQuery().Execute( _T("SELECT VoipExtensionID FROM VoipExtensions ")
						_T("WHERE VoipExtensionID<>? AND Description=? AND AgentID=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("A Voip Extension with that description already exists, please enter a unique Description!")  );
}

void CVoipExtensions::ClearDefault( void )
{
	//int nAgentID = GetAgentID();
	int nAgentID=0;
	GetISAPIData().GetURLLong( _T("AgentID"), nAgentID );
	
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_VoipExtensionID );
	BINDPARAM_LONG( GetQuery(), nAgentID );
	GetQuery().Execute( _T("UPDATE VoipExtensions SET IsDefault=0 ")
						_T("WHERE VoipExtensionID<>? AND AgentID=? ") );
	
}

void CVoipExtensions::SetDefault( void )
{
	//int nAgentID = GetAgentID();
	int nAgentID=0;
	GetISAPIData().GetURLLong( _T("AgentID"), nAgentID );
	
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_VoipExtensionID );
	BINDPARAM_LONG( GetQuery(), nAgentID );
	GetQuery().Execute( _T("SELECT VoipExtensionID FROM VoipExtensions ")
						_T("WHERE VoipExtensionID<>? AND IsDefault=1 AND AgentID=? ") );

	if( GetQuery().GetRowCount() == 0 )
	{
		m_IsDefault = 1;
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create a new extension
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipExtensions::New()
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

	TVoipExtensions::Insert( GetQuery() );

	return 0;
}

void CVoipExtensions::DecodeForm(void)
{
	//m_AgentID = GetAgentID();
	GetISAPIData().GetURLLong( _T("AgentID"), m_AgentID );
	
	// Description
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, 256 );
	if ( _tcslen( m_Description ) < 1 )
		THROW_VALIDATION_EXCEPTION( _T("Description"), _T("Please specify a description for the voip Extension") );

	// ServerID
	GetISAPIData().GetXMLLong( _T("VoipServerID"), m_VoipServerID );

	// Enabled?
	GetISAPIData().GetXMLLong( _T("IsEnabled"), m_IsEnabled );

	// Default?
	GetISAPIData().GetXMLLong( _T("IsDefault"), m_IsDefault );

	// Extension
	GetISAPIData().GetXMLLong( _T("Extension"), m_Extension, true );
	if ( m_Extension < 1 )
		THROW_VALIDATION_EXCEPTION( _T("Extension"), _T("Please specify an Extension for the voip extension") );

	// Pin
	GetISAPIData().GetXMLTCHAR( _T("Pin"), m_Pin, 50, true );	

}

void CVoipExtensions::GenerateXML(void)
{
	GetXMLGen().AddChildElem( _T("Extension") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_VoipExtensionID );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	GetXMLGen().AddChildAttrib( _T("ServerID"), m_VoipServerID );
	GetXMLGen().AddChildAttrib( _T("IsDefault"), m_IsDefault );
	GetXMLGen().AddChildAttrib( _T("Extension"), m_Extension );
	GetXMLGen().AddChildAttrib( _T("Pin"), m_Pin );

    TVoipServers vs;
	GetXMLGen().AddChildElem( _T("VoipServers") );
	GetXMLGen().IntoElem();
	vs.PrepareList(GetQuery());
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Server") );
		GetXMLGen().AddChildAttrib( _T("VSID"), vs.m_VoipServerID );
		GetXMLGen().AddChildAttrib( _T("Description"), vs.m_Description );			
	}
	GetXMLGen().OutOfElem();	
}
