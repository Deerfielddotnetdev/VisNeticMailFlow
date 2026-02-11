// TicketBoxHeaders.cpp: implementation of the CTicketBoxHeaders class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TicketBoxHeaders.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTicketBoxHeaders::CTicketBoxHeaders(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{
}

CTicketBoxHeaders::~CTicketBoxHeaders()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CTicketBoxHeaders::Run( CURLAction& action )
{
	CEMSString sID;
	tstring sAction;

	// Check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("selectId"), m_HeaderID, true ))
	{
		GetISAPIData().GetXMLString( _T("selectId"), sID, true );
		sID.CDLGetNextInt(m_HeaderID);
	}

	// get the action...
	GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("editticketboxheader") ) == 0)
	{	
		if ( GetISAPIData().GetXMLPost() )
		{
			DISABLE_IN_DEMO();

			if( m_HeaderID == 0 )
			{
				return DoInsert( action );
			}
			else
			{
				return DoUpdate( action );		
			}
		}
		else
		{
			if( m_HeaderID == 0 )
				action.m_sPageTitle.assign( _T("New TicketBox Header") );

			return QueryOne( action );
		}
	}

	if( sAction.compare( _T("delete") ) == 0 )
		return DoDelete( action );
	
	return ListAll( action );
}

////////////////////////////////////////////////////////////////////////////////
// 
// ListAll
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketBoxHeaders::ListAll( CURLAction& action )
{
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_HeaderID );
	BINDCOL_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT HeaderID,Description ")
						_T("FROM TicketBoxHeaders ")
						_T("WHERE IsDeleted=0 ")
						_T("ORDER BY Description") );

	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketBoxHeader") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_HeaderID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// QueryOne
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketBoxHeaders::QueryOne( CURLAction& action )
{
	if( m_HeaderID != 0 )
	{
		if( S_OK != TTicketBoxHeaders::Query( GetQuery() ) )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}

	GenerateXML();
	
	ListTicketBoxHeaders();
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoUpdate
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketBoxHeaders::DoUpdate( CURLAction& action )
{
	DecodeForm();

	unsigned int nOldID = m_HeaderID;

	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nOldID );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT HeaderID FROM TicketBoxHeaders ")
						_T("WHERE HeaderID<>? AND Description=? AND IsDeleted=0 ") );

	if( GetQuery().Fetch() == S_OK )
		THROW_VALIDATION_EXCEPTION( _T("Description"), _T("TicketBox Header names must be unique")  );
	
	// Insert a new record
	TTicketBoxHeaders::Insert( GetQuery() );

	// Now mark the old one as deleted
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), nOldID );
	GetQuery().Execute( _T("UPDATE TicketBoxHeaders ")
						_T("SET IsDeleted=1 ")
						_T("WHERE HeaderID=?") );
	
	// Update any ticket boxes pointing at this header
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_HeaderID );
	BINDPARAM_LONG( GetQuery(), nOldID );
	GetQuery().Execute( _T("UPDATE TicketBoxes ")
						_T("SET HeaderID=? ")
						_T("WHERE HeaderID=?") );	

	GetXMLCache().m_TicketBoxHeaders.Invalidate();
	GetXMLCache().SetDirty(true);
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoInsert
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketBoxHeaders::DoInsert( CURLAction& action )
{
	DecodeForm();

	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT HeaderID FROM TicketBoxHeaders ")
						_T("WHERE Description=? AND IsDeleted=0 ") );

	if( GetQuery().Fetch() == S_OK )
		THROW_VALIDATION_EXCEPTION( _T("Description"), _T("TicketBox Header names must be unique")  );

	TTicketBoxHeaders::Insert( GetQuery() );

	GetXMLCache().m_TicketBoxHeaders.Invalidate();
	GetXMLCache().SetDirty(true);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoDelete
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketBoxHeaders::DoDelete( CURLAction& action )
{
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_HeaderID );
	GetQuery().Execute( _T("UPDATE TicketBoxHeaders ")
						_T("SET IsDeleted=1 ")
						_T("WHERE HeaderID=?") );
	
	if( GetQuery().GetRowCount() == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}

	GetXMLCache().m_TicketBoxHeaders.Invalidate();
	GetXMLCache().SetDirty(true);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecodeForm
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketBoxHeaders::DecodeForm( void )
{
	GetISAPIData().GetXMLLong( _T("ID"), m_HeaderID, false );
	GetISAPIData().GetXMLTCHAR( _T("DESCRIPTION"), m_Description, 125, false );
	GetISAPIData().GetXMLTCHAR( _T("HEADERTEXT"), &m_Header, m_HeaderLen, m_HeaderAllocated, false );
}

////////////////////////////////////////////////////////////////////////////////
// 
// GenerateXML
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketBoxHeaders::GenerateXML( void )
{
	// We must insert a LINEFEED because the <textarea> tag eats the first one
	tstring sHeader = _T("\n");
	sHeader += m_Header;

	GetXMLGen().AddChildElem( _T("TicketBoxHeader") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_HeaderID );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetXMLGen().SetChildData( sHeader.c_str(), 1 );
}