/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketBoxOwners.h,v 1.1 2007/09/22 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing TicketBox Owners
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketBoxOwners.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CTicketBoxOwners::CTicketBoxOwners(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CTicketBoxOwners::Run( CURLAction& action )
{
	// Check security
	RequireAdmin();

	if( GetISAPIData().m_sPage.compare( _T("editticketboxowner") ) == 0)
	{
		if( GetISAPIData().GetXMLPost() )
		{
			DISABLE_IN_DEMO();
			Update();
		}
		else
		{
			QueryOne(action);
		}
	
		return 0;
	}

	if ( GetISAPIData().GetXMLPost() )
	{
		DISABLE_IN_DEMO();
		Delete();
	}
	else
	{
		Query();
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query all TicketBoxOwners	              
\*--------------------------------------------------------------------------*/
void CTicketBoxOwners::Query(void)
{
	TTicketBoxOwners tbo;
	tbo.PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("TicketBoxOwners") );
	GetXMLGen().IntoElem();

	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketBoxOwner") );
		GetXMLGen().AddChildAttrib( _T("ID"), tbo.m_TicketBoxOwnerID );
		GetXMLGen().AddChildAttrib( _T("Description"), tbo.m_Description );
	}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query a single TicketBoxOwner
\*--------------------------------------------------------------------------*/
void CTicketBoxOwners::QueryOne( CURLAction& action )
{
	TTicketBoxOwners tbo;
	GetISAPIData().GetURLLong( _T("ID"), tbo.m_TicketBoxOwnerID  );

	if ( tbo.m_TicketBoxOwnerID )
	{
		tbo.Query(GetQuery());
	}
	else
	{
		action.m_sPageTitle = _T("New TicketBox Owner");
	}

	GetXMLGen().AddChildElem( _T("TicketBoxOwner") );
	GetXMLGen().AddChildAttrib( _T("Description"), tbo.m_Description );	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Create a new or update an existing TicketBoxOwner              
\*--------------------------------------------------------------------------*/
void CTicketBoxOwners::Update(void)
{
	TTicketBoxOwners tbo;
	GetISAPIData().GetURLLong( _T("ID"), tbo.m_TicketBoxOwnerID  );
	GetISAPIData().GetXMLTCHAR( _T("Description"), tbo.m_Description, TICKETBOXOWNERS_DESCRIPTION_LENGTH );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), tbo.m_TicketBoxOwnerID );
	BINDPARAM_TCHAR( GetQuery(), tbo.m_Description );
	GetQuery().Execute( _T("SELECT TicketBoxOwnerID FROM TicketBoxOwners ")
						_T("WHERE TicketBoxOwnerID<>? AND Description=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("That TicketBox Owner already exists!")  );

	if ( tbo.m_TicketBoxOwnerID )
	{
		GetQuery().Initialize();
		BINDPARAM_TCHAR( GetQuery(), tbo.m_Description );
		BINDPARAM_LONG( GetQuery(), tbo.m_TicketBoxOwnerID );
		GetQuery().Execute( _T("UPDATE TicketBoxOwners SET Description=? WHERE BuiltIn=0 AND TicketBoxOwnerID=?") );

		if ( GetQuery().GetRowCount() != 1 )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to edit the TicketBox Owner, it may no longer exist.") );
	}
	else
	{
		tbo.Insert( GetQuery() );
	}

	InvalidateTicketBoxOwners();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes one or more TicketBoxOwners	              
\*--------------------------------------------------------------------------*/
void CTicketBoxOwners::Delete(void)
{
	CEMSString sIDs;
	TTicketBoxOwners tbo;

	GetISAPIData().GetXMLString( _T("SELECTID"), sIDs  );
	
	while ( sIDs.CDLGetNextInt( tbo.m_TicketBoxOwnerID ) )
	{
		if ( tbo.m_TicketBoxOwnerID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("This is the default TicketBox Owner and can't be deleted!") );

		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), tbo.m_TicketBoxOwnerID );
		GetQuery().Execute( _T("DELETE FROM TicketBoxOwners WHERE TicketBoxOwnerID=?") );

	}
	
	InvalidateTicketBoxOwners();
}