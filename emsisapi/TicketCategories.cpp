/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketCategories.h,v 1.1 2007/09/22 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Ticket Categories
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketCategories.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CTicketCategories::CTicketCategories(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CTicketCategories::Run( CURLAction& action )
{
	// Check security
	RequireAdmin();

	if( GetISAPIData().m_sPage.compare( _T("editticketcategory") ) == 0)
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
||  Comments:	Query all TicketCategories	              
\*--------------------------------------------------------------------------*/
void CTicketCategories::Query(void)
{
	TTicketCategories pdt;
	pdt.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("TicketCategoryNames") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketCategory") );
		GetXMLGen().AddChildAttrib( _T("ID"), pdt.m_TicketCategoryID );
		GetXMLGen().AddChildAttrib( _T("Description"), pdt.m_Description );
	}
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query a single TicketCategory
\*--------------------------------------------------------------------------*/
void CTicketCategories::QueryOne( CURLAction& action )
{
	TTicketCategories tc;
	GetISAPIData().GetURLLong( _T("ID"), tc.m_TicketCategoryID  );

	if ( tc.m_TicketCategoryID )
	{
		tc.Query(GetQuery());
	}
	else
	{
		action.m_sPageTitle = _T("New Ticket Category");
	}

	GetXMLGen().AddChildElem( _T("TicketCategory") );
	GetXMLGen().AddChildAttrib( _T("Description"), tc.m_Description );
	GetXMLGen().AddChildAttrib( _T("ID"), tc.m_TicketCategoryID );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Create a new or update an existing TicketCategory              
\*--------------------------------------------------------------------------*/
void CTicketCategories::Update(void)
{
	int ObjectTypeID = EMS_OBJECT_TYPE_TICKET_CATEGORY;
	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );
	TTicketCategories tc;
	GetISAPIData().GetURLLong( _T("ID"), tc.m_TicketCategoryID  );
	GetISAPIData().GetXMLTCHAR( _T("Description"), tc.m_Description, TICKETCATEGORIES_DESCRIPTION_LENGTH );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), tc.m_TicketCategoryID );
	BINDPARAM_TCHAR( GetQuery(), tc.m_Description );
	GetQuery().Execute( _T("SELECT TicketCategoryID FROM TicketCategories ")
						_T("WHERE TicketCategoryID<>? AND Description=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("That ticket category already exists!")  );

	if ( tc.m_TicketCategoryID )
	{
		if ( tc.m_TicketCategoryID == 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("This is the default ticket category and can't be edited!") );
		
		GetQuery().Initialize();
		BINDPARAM_TCHAR( GetQuery(), tc.m_Description );
		BINDPARAM_LONG( GetQuery(), tc.m_TicketCategoryID );
		GetQuery().Execute( _T("UPDATE TicketCategories SET Description=? WHERE BuiltIn=0 AND TicketCategoryID=?") );
		
		
		if ( GetQuery().GetRowCount() != 1 )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to edit the ticket category, it may no longer exist.") );
	}
	else
	{
		
		// Create the object ID
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), ObjectTypeID);
		GetQuery().Execute( _T("INSERT INTO Objects (ObjectTypeID) VALUES (?)") );
		tc.m_ObjectID = GetQuery().GetLastInsertedID();

		tc.Insert( GetQuery() );

		// Update the object row with the actual ID
		GetQuery().Reset( true );

		BINDPARAM_LONG( GetQuery(), tc.m_TicketCategoryID );
		BINDPARAM_TIME_NOLEN( GetQuery(), now );
		BINDPARAM_LONG( GetQuery(), tc.m_ObjectID );

		GetQuery().Execute( _T("UPDATE Objects SET ")
							_T("ActualID = ?, ")
							_T("DateCreated = ? ")
							_T("WHERE ObjectID = ?") );
	}

	InvalidateTicketCategories( true );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes one or more TicketCategories	              
\*--------------------------------------------------------------------------*/
void CTicketCategories::Delete(void)
{
	CEMSString sIDs;
	TTicketCategories tc;
	int ObjectID;
	int NumTickets;

	GetISAPIData().GetXMLString( _T("SELECTID"), sIDs  );
	
	while ( sIDs.CDLGetNextInt( tc.m_TicketCategoryID ) )
	{
		if ( tc.m_TicketCategoryID == 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("This is the default ticket category and can't be deleted!") );

		//Check to see if the Ticket Category is in use by any Tickets
		NumTickets=0;
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), tc.m_TicketCategoryID );
		BINDCOL_LONG_NOLEN( GetQuery(), NumTickets );
		GetQuery().Execute( _T("SELECT COUNT(TicketID) ")
							_T("FROM Tickets ")
							_T("WHERE TicketCategoryID=?") );
		if( GetQuery().Fetch() != S_OK )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
		}
		
		if( NumTickets > 0 )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, _T("This ticket category is assigned to Tickets and can't be deleted!") );
			break;
		}
		
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), tc.m_TicketCategoryID );
		BINDCOL_LONG_NOLEN( GetQuery(), ObjectID );
		GetQuery().Execute( _T("SELECT ObjectID ")
							_T("FROM TicketCategories ")
							_T("WHERE TicketCategoryID=?") );

		if( GetQuery().Fetch() != S_OK )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
		}
				
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), tc.m_TicketCategoryID );
		GetQuery().Execute( _T("DELETE FROM TicketCategories WHERE TicketCategoryID=?") );

		// Delete any access control records
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), ObjectID );
		GetQuery().Execute( _T("DELETE FROM AccessControl ")
							_T("WHERE ObjectID=?") );	

		// Delete the object
		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM Objects ")
							_T("WHERE ObjectID=?") );

	}
	
	InvalidateTicketCategories( true );
}