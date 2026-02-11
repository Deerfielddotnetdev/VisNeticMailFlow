/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/VoipServers.cpp,v 1.2.2.1 2005/12/13 18:11:49 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Voip Servers
||              
\\*************************************************************************/

#include "stdafx.h"
#include "VoipServers.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CVoipServers::CVoipServers( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CVoipServers::~CVoipServers()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CVoipServers::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_VoipServerID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_VoipServerID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_VoipServerID);
		}
	}
	
	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( GetISAPIData().m_sPage.compare( _T("voipserver") ) == 0 )
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

			if( m_VoipServerID == 0 )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			
			Update();
			return 0;
		}
		
		// change title if ID is zero
		if( m_VoipServerID == 0 )
		{
			action.m_sPageTitle.assign( "New Voip Server" );
		}
		else
		{
			action.m_sPageTitle.assign( "Edit Voip Server" );
		}
		
		return Query();
	}
	
	if( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();

		if( m_VoipServerID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

		Delete();
	}

	action.m_sPageTitle.assign( "VOIP Servers" );
	return ListAll();	
	
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query all voip servers
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipServers::ListAll(void)
{
	RequireAdmin();

	PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("VoipServers") );
	GetXMLGen().IntoElem();
	
    while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Server") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_VoipServerID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
		GetXMLGen().AddChildAttrib( _T("TypeID"), m_VoipServerTypeID );
		GetXMLGen().AddChildAttrib( _T("IsDefault"), m_IsDefault );
		GetXMLGen().AddChildAttrib( _T("Url"), m_Url );
		GetXMLGen().AddChildAttrib( _T("Port"), m_Port );		
	}

	GetXMLGen().OutOfElem();

	TVoipServerTypes vst;
	GetXMLGen().AddChildElem( _T("VoipServerTypes") );
	GetXMLGen().IntoElem();
	vst.PrepareList(GetQuery());
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Type") );
		GetXMLGen().AddChildAttrib( _T("VSTID"), vst.m_VoipServerTypeID );
		GetXMLGen().AddChildAttrib( _T("Description"), vst.m_Description );			
	}
	GetXMLGen().OutOfElem();	

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query for a particular voip server
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipServers::Query(void)

{
	CEMSString sID;
	
	if (!GetISAPIData().GetURLLong( _T("ID"), m_VoipServerID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_VoipServerID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_VoipServerID);
		}
	}

	if ( m_VoipServerID > 0 )
	{
		TVoipServers::Query(GetQuery());
	}
	else
	{
		//Set defaults for a new server
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
int CVoipServers::Update()
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

	TVoipServers::Update(GetQuery());
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a server
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipServers::Delete()
{
	if ( m_VoipServerID )
	{
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_VoipServerID );		
		GetQuery().Execute( _T("DELETE FROM VoipDialingCodes ")
						_T("WHERE VoipServerID=? ") );

		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM VoipExtensions ")
						_T("WHERE VoipServerID=? ") );
		
		TVoipServers::Delete(GetQuery());		
	}
	
	return 0;
}

void CVoipServers::CheckForDuplicity( void )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_VoipServerID );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT VoipServerID FROM VoipServers ")
						_T("WHERE VoipServerID<>? AND Description=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("A Voip Server with that description already exists, please enter a unique Description!")  );
}

void CVoipServers::ClearDefault( void )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_VoipServerID );
	GetQuery().Execute( _T("UPDATE VoipServers SET IsDefault=0 ")
						_T("WHERE VoipServerID<>? ") );
	
}

void CVoipServers::SetDefault( void )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_VoipServerID );
	GetQuery().Execute( _T("SELECT VoipServerID FROM VoipServers ")
						_T("WHERE VoipServerID<>? AND IsDefault=1 ") );

	if( GetQuery().GetRowCount() == 0 )
	{
		m_IsDefault = 1;
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create a new, blank scheduled report
// 
////////////////////////////////////////////////////////////////////////////////
int CVoipServers::New()
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

	TVoipServers::Insert( GetQuery() );

	return 0;
}

void CVoipServers::DecodeForm(void)
{
	// Description
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, 256 );
	if ( _tcslen( m_Description ) < 1 )
		THROW_VALIDATION_EXCEPTION( _T("Description"), _T("Please specify a description for the voip server") );

	// Enabled?
	GetISAPIData().GetXMLLong( _T("IsEnabled"), m_IsEnabled );

	// Default?
	GetISAPIData().GetXMLLong( _T("IsDefault"), m_IsDefault );

	// VoipServerTypeID
	GetISAPIData().GetXMLLong( _T("VoipServerTypeID"), m_VoipServerTypeID, true );

	// URL
	GetISAPIData().GetXMLTCHAR( _T("Url"), m_Url, 256 );
	if ( _tcslen( m_Url ) < 1 )
		THROW_VALIDATION_EXCEPTION( _T("Url"), _T("Please specify a Url for the voip server") );

	// Port
	GetISAPIData().GetXMLLong( _T("Port"), m_Port );	
}

void CVoipServers::GenerateXML(void)
{
	GetXMLGen().AddChildElem( _T("Server") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_VoipServerID );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	GetXMLGen().AddChildAttrib( _T("TypeID"), m_VoipServerTypeID );
	GetXMLGen().AddChildAttrib( _T("IsDefault"), m_IsDefault );
	GetXMLGen().AddChildAttrib( _T("Url"), m_Url );
	GetXMLGen().AddChildAttrib( _T("Port"), m_Port );

    TVoipServerTypes vst;
	GetXMLGen().AddChildElem( _T("VoipServerTypes") );
	GetXMLGen().IntoElem();
	vst.PrepareList(GetQuery());
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Type") );
		GetXMLGen().AddChildAttrib( _T("VSTID"), vst.m_VoipServerTypeID );
		GetXMLGen().AddChildAttrib( _T("Description"), vst.m_Description );			
	}
	GetXMLGen().OutOfElem();		
}
