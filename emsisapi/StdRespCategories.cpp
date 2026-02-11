/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/StdRespCategories.cpp,v 1.2.2.1 2005/12/13 18:11:49 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Class to manage standard response categories  
||              
\\*************************************************************************/

#include "stdafx.h"
#include "StdRespCategories.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStdRespCategories::CStdRespCategories(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_bRefreshRequired = false;
}

CStdRespCategories::~CStdRespCategories()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
int CStdRespCategories::Run( CURLAction& action )
{
	
	//Removed in version 6.5.0.4
	/*if( GetISAPIData().m_sPage.compare( _T("standardresponses") ) == 0)
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0, EMS_READ_ACCESS );
		return 0;
	}*/

	tstring sID;
	tstring sAction;
	
	m_StdResponseCatID = 0;
	
	//Removed in version 6.5.0.4
	//m_access = GetAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0);

	if(!GetISAPIData().GetXMLString( _T("Action"), sAction, true ))
	{
		GetISAPIData().GetFormString(_T("Action"), sAction, true);
	}
	
	if(!GetISAPIData().GetURLLong( _T("selectId"), m_StdResponseCatID, true ))
	{
		GetISAPIData().GetFormLong(_T("selectId"), m_StdResponseCatID, true);
	}
	
	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("editstdrespcategory") ) == 0)
	{
		
		if( sAction.compare( _T("update") ) == 0 )
		{
			if(m_StdResponseCatID > 0)
			{
				TStdResponseCategories::Query(GetQuery());
			}
			DecodeForm();
			return Update( action );
		}
		
		QueryOne();
		return 0;
	}
	
	if( sAction.compare( _T("delete") ) == 0 )
	{
		Delete();
		return 0;
	}	
	
	ListAll();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// ListAll
// 
////////////////////////////////////////////////////////////////////////////////
void CStdRespCategories::ListAll(void)
{
	CEMSString sName;
	int nIndex = 4;	// temporary hack
	CEMSString sURL;
	sURL.Format( _T("%sliststdrespcategories.ems?OutputXML=1"), 
		GetISAPIData().m_sURLSubDir.c_str() );

	// add the access level
	GetXMLGen().AddChildElem(_T("access"));
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(_T("level"), m_access);
	GetXMLGen().OutOfElem();
	
	// add to the XML if a refresh is required
	GetXMLGen().AddChildElem(_T("refresh"));
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(_T("required"), m_bRefreshRequired ? 1 : 0 );
	GetXMLGen().AddChildElem(_T("url"), sURL.c_str() );
	GetXMLGen().OutOfElem();
	
	// list the categories
	//ListStdRespCategories();
	ListStdRespCategories(GetAgentID(),EMS_READ_ACCESS,GetXMLGen());
}


////////////////////////////////////////////////////////////////////////////////
// 
// QueryOne
// 
////////////////////////////////////////////////////////////////////////////////
void CStdRespCategories::QueryOne(void)
{

	RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0, EMS_EDIT_ACCESS );
	
	GetXMLGen().AddChildElem( _T("stdrespcategory") );
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("id"), m_StdResponseCatID );

	if( m_StdResponseCatID == 0 )
	{
		GetXMLGen().AddChildAttrib( _T("name"), _T("") );			
		GetXMLGen().AddChildAttrib( _T("nameraw"), _T("") );			
	}	
	else
	{
		if( TStdResponseCategories::Query( GetQuery() ) != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
		
		CEMSString sString;
		sString.assign( m_CategoryName );
		sString.EscapeHTML();
		GetXMLGen().AddChildAttrib( _T("name"), sString.c_str() );
		sString.assign( m_CategoryName );
		sString.EscapeJavascript();
		GetXMLGen().AddChildAttrib( _T("nameRaw"), sString.c_str() );	
	}

	GetXMLGen().OutOfElem();
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecodeForm
// 
////////////////////////////////////////////////////////////////////////////////
void CStdRespCategories::DecodeForm( void )
{
	GetISAPIData().GetXMLTCHAR( _T("name"), m_CategoryName, 50, false );
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update
// 
////////////////////////////////////////////////////////////////////////////////
int CStdRespCategories::Update(CURLAction& action)
{
	RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0, EMS_EDIT_ACCESS );

	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_StdResponseCatID );
	BINDPARAM_TCHAR( GetQuery(), m_CategoryName );
	GetQuery().Execute( _T("SELECT StdResponseCatID FROM StdResponseCategories ")
						_T("WHERE StdResponseCatID<>? AND CategoryName=? ") );

	if( GetQuery().Fetch() == S_OK )
	{
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("The standard response category name must be unique.") );
	}

	if( m_StdResponseCatID == 0 )
	{
		TStdResponseCategories::Insert( GetQuery() );
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), m_StdResponseCatID);
		GetQuery().Execute( _T("INSERT INTO Objects (ActualID,ObjectTypeID,BuiltIn,UseDefaultRights,DateCreated) VALUES (?,5,0,1,GetDate())") );
		m_ObjectID = GetQuery().GetLastInsertedID();
		TStdResponseCategories::Update( GetQuery() );

	}
	else if ( m_StdResponseCatID == EMS_STDRESPCAT_DRAFTS )
	{
		// can't update the drafts category
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}
	else
	{
		if( TStdResponseCategories::Update( GetQuery() ) == 0 )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
		}
	}

	m_bRefreshRequired = true;
	InvalidateStdRespCategories( true );

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Delete
// 
////////////////////////////////////////////////////////////////////////////////
void CStdRespCategories::Delete(void)
{
	m_access = RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0, EMS_DELETE_ACCESS);

	if( m_StdResponseCatID == 0 )
		GetISAPIData().GetXMLLong( _T("selectId"), m_StdResponseCatID, false );
	
	tstring sOpt;
	if(!GetISAPIData().GetFormString(_T("opt"), sOpt, true))
	{
		GetISAPIData().GetXMLString( _T("opt"), sOpt, false );
	}

	if( m_StdResponseCatID <= EMS_STDRESPCAT_DRAFTS )
	{
		// minimum custom category is 2
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}
	else
	{
		TIMESTAMP_STRUCT now;
		GetTimeStamp( now );

		if( sOpt.compare( _T("delete") ) == 0 )
		{
			// delete all 
			GetQuery().Initialize();
			BINDPARAM_TIME_NOLEN( GetQuery(), now );
			BINDPARAM_LONG( GetQuery(), GetSession().m_AgentID );
			BINDPARAM_LONG( GetQuery(), m_StdResponseCatID );
			GetQuery().Execute( _T("UPDATE StandardResponses ")
								_T("SET IsDeleted=1,")
								_T("StdResponseCatID=-4,")
								_T("DeletedTime=?,")
								_T("DeletedBy=? ")
								_T("WHERE StdResponseCatID=?") );			
		}
		else
		{
			// option -> move to uncategorized
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_StdResponseCatID );
			GetQuery().Execute( _T("UPDATE StandardResponses ")
								_T("SET StdResponseCatID=-4 ")
								_T("WHERE StdResponseCatID=?") );			
		}

		Query(GetQuery());

		if( TStdResponseCategories::Delete( GetQuery() ) == 0 )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		}
		else
		{
			//Delete the Object and Access records
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_ObjectID );
			GetQuery().Execute( _T("DELETE FROM AccessControl WHERE ObjectID=?") );
			GetQuery().Reset(false);
			GetQuery().Execute( _T("DELETE FROM Objects WHERE ObjectID=?") );
		}
		
		m_bRefreshRequired = true;
		InvalidateStdRespCategories( true );	
	}
}
