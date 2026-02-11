// TicketBoxFooters.cpp: implementation of the CTicketBoxFooters class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TicketBoxFooters.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTicketBoxFooters::CTicketBoxFooters(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{
}

CTicketBoxFooters::~CTicketBoxFooters()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	              
\*--------------------------------------------------------------------------*/
int CTicketBoxFooters::Run( CURLAction& action )
{
	CEMSString sID;
	tstring sAction;

	// Check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("selectId"), m_FooterID, true ))
	{
		GetISAPIData().GetXMLString( _T("selectId"), sID, true );
		sID.CDLGetNextInt(m_FooterID);
	}

	// get the action...
	GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("editticketboxfooter") ) == 0)
	{	
		if ( GetISAPIData().GetXMLPost() )
		{
			DISABLE_IN_DEMO();

			if( m_FooterID == 0 )
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
			if( m_FooterID == 0 )
				action.m_sPageTitle.assign( _T("New TicketBox Footer") );

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
int CTicketBoxFooters::ListAll( CURLAction& action )
{
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_FooterID );
	BINDCOL_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT FooterID,Description ")
						_T("FROM TicketBoxFooters ")
						_T("WHERE IsDeleted=0 ")
						_T("ORDER BY Description") );

	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketBoxFooter") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_FooterID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// QueryOne
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketBoxFooters::QueryOne( CURLAction& action )
{
	if( m_FooterID != 0 )
	{
		if( S_OK != TTicketBoxFooters::Query( GetQuery() ) )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}

	GenerateXML();

	ListTicketBoxFooters();
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoUpdate
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketBoxFooters::DoUpdate( CURLAction& action )
{
	DecodeForm();

	unsigned int nOldID = m_FooterID;

	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nOldID );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT FooterID FROM TicketBoxFooters ")
						_T("WHERE FooterID<>? AND Description=? AND IsDeleted=0 ") );

	if( GetQuery().Fetch() == S_OK )
		THROW_VALIDATION_EXCEPTION( _T("Description"), _T("TicketBox Footer names must be unique")  );
	
	// Insert a new record
	TTicketBoxFooters::Insert( GetQuery() );

	// Now mark the old one as deleted
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), nOldID );
	GetQuery().Execute( _T("UPDATE TicketBoxFooters ")
						_T("SET IsDeleted=1 ")
						_T("WHERE FooterID=?") );
	
	// Update any ticket boxes pointing at this footer
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_FooterID );
	BINDPARAM_LONG( GetQuery(), nOldID );
	GetQuery().Execute( _T("UPDATE TicketBoxes ")
						_T("SET FooterID=? ")
						_T("WHERE FooterID=?") );

	GetXMLCache().m_TicketBoxFooters.Invalidate();
	GetXMLCache().SetDirty(true);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoInsert
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketBoxFooters::DoInsert( CURLAction& action )
{
	DecodeForm();

	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT FooterID FROM TicketBoxFooters ")
						_T("WHERE Description=? AND IsDeleted=0 ") );

	if( GetQuery().Fetch() == S_OK )
		THROW_VALIDATION_EXCEPTION( _T("Description"), _T("TicketBox Footer names must be unique")  );

	TTicketBoxFooters::Insert( GetQuery() );

	GetXMLCache().m_TicketBoxFooters.Invalidate();
	GetXMLCache().SetDirty(true);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoDelete
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketBoxFooters::DoDelete( CURLAction& action )
{
	
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_FooterID );
	GetQuery().Execute( _T("UPDATE TicketBoxFooters ")
						_T("SET IsDeleted=1 ")
						_T("WHERE FooterID=?") );
	
	if( GetQuery().GetRowCount() == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 

	GetXMLCache().m_TicketBoxFooters.Invalidate();
	GetXMLCache().SetDirty(true);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecodeForm
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketBoxFooters::DecodeForm( void )
{
	GetISAPIData().GetXMLLong( _T("ID"), m_FooterID, false );
	GetISAPIData().GetXMLTCHAR( _T("DESCRIPTION"), m_Description, 125, false );
	GetISAPIData().GetXMLTCHAR( _T("FOOTERTEXT"), &m_Footer, m_FooterLen, m_FooterAllocated, false );
}

////////////////////////////////////////////////////////////////////////////////
// 
// GenerateXML
// 
////////////////////////////////////////////////////////////////////////////////
void CTicketBoxFooters::GenerateXML( void )
{
	// We must insert a LINEFEED because the <textarea> tag eats the first one
	tstring sFooter = _T("\n");
	sFooter += m_Footer;

	GetXMLGen().AddChildElem( _T("TicketBoxFooter") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_FooterID );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetXMLGen().SetChildData( sFooter.c_str(), 1 );
}