/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/PersonalDataTypes.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing PersonalDataTypes
||              
\\*************************************************************************/

#include "stdafx.h"
#include "PersonalDataTypes.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CPersonalDataTypes::CPersonalDataTypes(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CPersonalDataTypes::Run( CURLAction& action )
{
	// Check security
	RequireAdmin();

	if( GetISAPIData().m_sPage.compare( _T("editpersonaldatatype") ) == 0)
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
||  Comments:	Query all PersonalDataTypes	              
\*--------------------------------------------------------------------------*/
void CPersonalDataTypes::Query(void)
{
	TPersonalDataTypes pdt;
	pdt.PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("PersonalDataTypes") );
	GetXMLGen().IntoElem();

	while( GetQuery().Fetch() == S_OK )
	{
		if ( pdt.m_BuiltIn == 0 )
		{
			GetXMLGen().AddChildElem( _T("PersonalDataType") );
			GetXMLGen().AddChildAttrib( _T("ID"), pdt.m_PersonalDataTypeID );
			GetXMLGen().AddChildAttrib( _T("Description"), pdt.m_TypeName );
		}
	}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query a single PersonalDataType
\*--------------------------------------------------------------------------*/
void CPersonalDataTypes::QueryOne( CURLAction& action )
{
	TPersonalDataTypes DataType;
	GetISAPIData().GetURLLong( _T("ID"), DataType.m_PersonalDataTypeID  );

	if ( DataType.m_PersonalDataTypeID )
	{
		DataType.Query(GetQuery());
	}
	else
	{
		action.m_sPageTitle = _T("New Property");
	}

	GetXMLGen().AddChildElem( _T("PersonalDataType") );
	GetXMLGen().AddChildAttrib( _T("Description"), DataType.m_TypeName );	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Create a new or update an existing PersonalDataType              
\*--------------------------------------------------------------------------*/
void CPersonalDataTypes::Update(void)
{
	TPersonalDataTypes DataType;
	GetISAPIData().GetURLLong( _T("ID"), DataType.m_PersonalDataTypeID  );
	GetISAPIData().GetXMLTCHAR( _T("Description"), DataType.m_TypeName, PERSONALDATATYPES_TYPENAME_LENGTH );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), DataType.m_PersonalDataTypeID );
	BINDPARAM_TCHAR( GetQuery(), DataType.m_TypeName );
	GetQuery().Execute( _T("SELECT PersonalDataTypeID FROM PersonalDataTypes ")
						_T("WHERE PersonalDataTypeID<>? AND TypeName=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("The property description must be unique.")  );

	if ( DataType.m_PersonalDataTypeID )
	{
		GetQuery().Initialize();
		BINDPARAM_TCHAR( GetQuery(), DataType.m_TypeName );
		BINDPARAM_LONG( GetQuery(), DataType.m_PersonalDataTypeID );
		GetQuery().Execute( _T("UPDATE PersonalDataTypes SET TypeName=? WHERE BuiltIn=0 AND PersonalDataTypeID=?") );

		if ( GetQuery().GetRowCount() != 1 )
			THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Unable to edit property, the property may no longer exist.") );
	}
	else
	{
		DataType.Insert( GetQuery() );
	}

	InvalidatePersonalDataTypes();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes one or more PersonalDataTypes	              
\*--------------------------------------------------------------------------*/
void CPersonalDataTypes::Delete(void)
{
	CEMSString sIDs;
	TPersonalDataTypes DataType;

	GetISAPIData().GetXMLString( _T("SELECTID"), sIDs  );
	
	while ( sIDs.CDLGetNextInt( DataType.m_PersonalDataTypeID ) )
	{
		if ( DataType.m_PersonalDataTypeID < 1 )
			THROW_EMS_EXCEPTION( E_InvalidID, _T("A property ID must be specified") );

		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), DataType.m_PersonalDataTypeID );
		GetQuery().Execute( _T("DELETE FROM PersonalData WHERE PersonalDataTypeID=?") );

		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM PersonalDataTypes WHERE PersonalDataTypeID=?") );
	}
	
	InvalidatePersonalDataTypes();
}