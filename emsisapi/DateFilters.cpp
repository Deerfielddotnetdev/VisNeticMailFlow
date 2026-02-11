// ProcessingRules.cpp: implementation of the CProcessingRules class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DateFilters.h"
#include "StringFns.h"
#include <.\boost\regex.hpp>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDateFilters::CDateFilters(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_MaxID = 0;
	m_DateFilterID = 0;
}

CDateFilters::~CDateFilters()
{
}

int CDateFilters::Run( CURLAction& action )
{  
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_DateFilterID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_DateFilterID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_DateFilterID);
		}
	}

	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
	{
		if (!GetISAPIData().GetXMLString( _T("Action"), sAction, true ))
			GetISAPIData().GetURLString( _T("Action"), sAction, true );
	}

	if( GetISAPIData().m_sPage.compare( _T("datefilter") ) == 0 )
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

			if( m_DateFilterID == 0 )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			
			Update();
			return 0;
		}
		
		// change title if ID is zero
		if( m_DateFilterID == 0 )
		{
			action.m_sPageTitle.assign( "New Date Filter" );
			GenerateXML();
		}
		else
		{
			return Query();
		}
	}
	
	if( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();

		if( m_DateFilterID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

		Delete();
	}
		
	return ListAll();
}

int CDateFilters::ListAll(void)
{
	RequireAdmin();

	PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("DateFilters") );
	GetXMLGen().IntoElem();
	
	while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem(_T("DateFilter"));
		GetXMLGen().AddChildAttrib( _T("ID"), m_DateFilterID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetXMLGen().AddChildAttrib( _T("WhereToCheck"), m_WhereToCheck );
		GetXMLGen().AddChildAttrib( _T("RegEx"), m_RegEx );
		GetXMLGen().AddChildAttrib( _T("Header"), m_Header );
		GetXMLGen().AddChildAttrib( _T("HeaderValueTypeID"), m_HeaderValueTypeID );
		GetXMLGen().AddChildAttrib( _T("HeaderValue"), m_HeaderValue );
	}

	GetXMLGen().OutOfElem();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query for a particular date filter
// 
////////////////////////////////////////////////////////////////////////////////
int CDateFilters::Query(void)
{
	RequireAdmin();	

	if( m_DateFilterID != 0 )
	{
		int nRet = TDateFilters::Query(GetQuery());

		if ( nRet != 0 )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		}
	}
	else
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}

	GenerateXML();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update a date filter
// 
////////////////////////////////////////////////////////////////////////////////
int CDateFilters::Update()
{
	DecodeForm();

	tstring sReg(m_RegEx);
	if(!TestRegEx(sReg.c_str()))
	{
		THROW_EMS_EXCEPTION( E_InvalidParameters, _T("Invalid Regular Expression!"));
	}

	if(m_DateFilterID > 0)
	{
		int nRet = TDateFilters::Update(GetQuery());
		if(nRet != 1)
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
		}
	}
	else
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a date filter
// 
////////////////////////////////////////////////////////////////////////////////
int CDateFilters::Delete()
{
	RequireAdmin();	

	if( m_DateFilterID != 0 )
	{
		int nRet = TDateFilters::Delete(GetQuery());

		if ( nRet != 1 )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		}
	}
	else
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}

	return 0;
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create a new date filter
// 
////////////////////////////////////////////////////////////////////////////////
int CDateFilters::New()
{
	RequireAdmin();

	DecodeForm();
	
	TDateFilters::Insert(GetQuery());

	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_DateFilters );	
	
	return 0;
}

void CDateFilters::DecodeForm(void)
{
	// rule description
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, 125 );
	 
	if ( _tcslen( m_Description ) < 1 )
		THROW_VALIDATION_EXCEPTION( _T("Description"), _T("Please specify a description for the date filter") );

	GetISAPIData().GetXMLLong( _T("WhereToCheck"), m_WhereToCheck );
	GetISAPIData().GetXMLTCHAR( _T("RegEx"), m_RegEx, REGULAR_EXPRESSION_LENGTH );
	GetISAPIData().GetXMLTCHAR( _T("Header"), m_Header, HEADER_LENGTH );
	GetISAPIData().GetXMLLong( _T("HeaderValueTypeID"), m_HeaderValueTypeID, true );
	GetISAPIData().GetXMLTCHAR( _T("HeaderValue"), m_HeaderValue, HEADER_LENGTH );	
}

void CDateFilters::GenerateXML(void)
{
	GetXMLGen().AddChildElem(_T("DateFilter"));
	GetXMLGen().AddChildAttrib( _T("ID"), m_DateFilterID );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetXMLGen().AddChildAttrib( _T("WhereToCheck"), m_WhereToCheck );
	GetXMLGen().AddChildAttrib( _T("RegEx"), m_RegEx );
	GetXMLGen().AddChildAttrib( _T("Header"), m_Header );
	GetXMLGen().AddChildAttrib( _T("HeaderValueTypeID"), m_HeaderValueTypeID );
	GetXMLGen().AddChildAttrib( _T("HeaderValue"), m_HeaderValue );
		
	ListMatchLocations();
}

bool CDateFilters::TestRegEx(dca::String sRegEx)
{
	try
	{
		boost::regex pattern (sRegEx,boost::regex_constants::perl);		
	}
	catch(...)
	{
		return false;
	}
	return true;	
}