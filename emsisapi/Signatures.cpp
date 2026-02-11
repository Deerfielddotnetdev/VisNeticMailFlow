// Signatures.cpp: implementation of the CSignatures class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Signatures.h"
#include "StringFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSignatures::CSignatures(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_refid = 0;
}

CSignatures::~CSignatures()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
int CSignatures::Run( CURLAction& action )
{
	CEMSString sID;
	tstring sAction;

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("selectId"), m_SignatureID, true ))
	{
		GetISAPIData().GetXMLString( _T("selectId"), sID, true );
		sID.CDLGetNextInt(m_SignatureID);
	}

	// get the action...
	GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("editsignaturepref") ) == 0 )
	{
		if( GetISAPIData().GetXMLPost() )
		{
			if( m_SignatureID == 0 )
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
			if( m_SignatureID == 0 )
				action.m_sPageTitle.assign( _T("New Signature") );
		
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
int CSignatures::ListAll( CURLAction& action )
{
	int GroupID = 0;
	int AgentID = 0;

	GetISAPIData().GetURLLong( _T("AgentID"), AgentID, true );	
	GetISAPIData().GetURLLong( _T("GroupID"), GroupID, true );	

	GetXMLGen().AddChildElem( _T("Parameters") );
	GetXMLGen().AddChildAttrib( _T("GroupID"), GroupID );
	AddGroupName( _T("GroupName"), GroupID );
	GetXMLGen().AddChildAttrib( _T("AgentID"), AgentID );
	AddAgentName( _T("AgentName"), AgentID );

	if( m_refid )
	{
		GetXMLGen().AddChildAttrib( _T("refid"), m_refid );
	}

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_SignatureID );
	BINDCOL_LONG( GetQuery(), m_AgentID );
	BINDCOL_LONG( GetQuery(), m_GroupID );
	BINDCOL_LONG( GetQuery(), m_ObjectTypeID );
	BINDCOL_LONG( GetQuery(), m_ActualID );
	BINDCOL_TCHAR( GetQuery(), m_Name );

	if( GroupID )
	{
		// List signatures for an group - requires read access
		RequireAgentRightLevel( EMS_OBJECT_TYPE_GROUP, GroupID, EMS_READ_ACCESS );

		BINDPARAM_LONG( GetQuery(), GroupID );
		GetQuery().Execute( _T("SELECT SignatureID,AgentID,GroupID,ObjectTypeID,ActualID,Name,Signature ")
							_T("FROM Signatures ")
							_T("WHERE GroupID=? ")
							_T("ORDER BY Name") );
	}
	else if ( AgentID )
	{
		// List signatures for an agent - requires read access
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, AgentID, EMS_READ_ACCESS );

		BINDPARAM_LONG( GetQuery(), AgentID );
		BINDPARAM_LONG( GetQuery(), AgentID );
		GetQuery().Execute( _T("SELECT SignatureID,Signatures.AgentID,Signatures.GroupID,Signatures.ObjectTypeID,Signatures.ActualID,Name,Signature ")
							_T("FROM Signatures LEFT OUTER JOIN AgentGroupings ")
							_T("ON Signatures.GroupID = AgentGroupings.GroupID ")
							_T("WHERE Signatures.AgentID=? OR AgentGroupings.AgentID=? ")
							_T("ORDER BY Signatures.GroupID,Signatures.AgentID,Name") );
	}
	else
	{
		// List all signatures - agent must be administrator
		RequireAdmin();	

		GetQuery().Execute( _T("SELECT SignatureID,Signatures.AgentID,Signatures.GroupID,Signatures.ObjectTypeID,Signatures.ActualID,Signatures.Name,Signature ")
							_T("FROM Signatures ")
							_T("LEFT OUTER JOIN Groups ON Signatures.GroupID = Groups.GroupID ")
							_T("LEFT OUTER JOIN Agents ON Signatures.AgentID = Agents.AgentID ")
							_T("ORDER BY Agents.Name,Groups.GroupName,Signatures.Name") );
	}

	while( GetQuery().Fetch() == S_OK )
	{
		GetLongData( GetQuery() );
		GenerateXMLList();
	}

	ListTicketBoxNames( GetXMLGen() );
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
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// QueryOne
// 
////////////////////////////////////////////////////////////////////////////////
int CSignatures::QueryOne( CURLAction& action )
{
	if( m_SignatureID == 0 )
	{
		GetISAPIData().GetURLLong( _T("AgentID"), m_AgentID, true );
		GetISAPIData().GetURLLong( _T("GroupID"), m_GroupID, true );
	}
	else
	{
		if( TSignatures::Query( GetQuery() ) != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

		if( m_AgentID )
		{
			RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_EDIT_ACCESS );
		}
		else
		{
			RequireAgentRightLevel( EMS_OBJECT_TYPE_GROUP, m_GroupID, EMS_EDIT_ACCESS );
		}
	}

	GenerateXML();
	
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Update a message source
// 
////////////////////////////////////////////////////////////////////////////////
int CSignatures::DoUpdate( CURLAction& action )
{
	if( m_SignatureID == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 

	DecodeForm();

	CheckForDuplicates();

	if( m_AgentID )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_EDIT_ACCESS );
	}
	else
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_GROUP, m_GroupID, EMS_EDIT_ACCESS );
	}

	if( TSignatures::Update( GetQuery() ) == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}
	else
	{
		InvalidateSignatures();
	}

	return 0;
}

void CSignatures::CheckForDuplicates(void)
{
	int nSignatureID;

	// Add a list of other signature names for duplicate prevention
	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_Name );
	BINDPARAM_LONG( GetQuery(), m_SignatureID );
	BINDCOL_LONG_NOLEN( GetQuery(), nSignatureID );
	if( m_AgentID )
	{
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		GetQuery().Execute( _T("SELECT SignatureID FROM Signatures WHERE Name=? AND SignatureID<>? AND AgentID=?") );
	}
	else
	{
		BINDPARAM_LONG( GetQuery(), m_GroupID );
		GetQuery().Execute( _T("SELECT SignatureID FROM Signatures WHERE Name=? AND SignatureID<>? AND GroupID=?") );
	}

	if( GetQuery().Fetch() == S_OK )
	{
		THROW_VALIDATION_EXCEPTION( _T("Name"), _T("Signature names must be unique") );
	}

	if ( m_ObjectTypeID > 0 && m_ActualID > 0 )
	{
		// Add a list of other signature names for duplicate prevention
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_SignatureID );
		BINDPARAM_LONG( GetQuery(), m_ObjectTypeID );
		BINDPARAM_LONG( GetQuery(), m_ActualID );
		BINDCOL_LONG_NOLEN( GetQuery(), nSignatureID );
		if( m_AgentID )
		{
			BINDPARAM_LONG( GetQuery(), m_AgentID );
			GetQuery().Execute( _T("SELECT SignatureID FROM Signatures WHERE SignatureID<>? AND ObjectTypeID=? AND ActualID=? AND AgentID=?") );
		}
		else
		{
			BINDPARAM_LONG( GetQuery(), m_GroupID );
			GetQuery().Execute( _T("SELECT SignatureID FROM Signatures WHERE SignatureID<>? AND ObjectTypeID=? AND ActualID=? AND GroupID=?") );
		}

		if( GetQuery().Fetch() == S_OK )
		{
			THROW_VALIDATION_EXCEPTION( _T("Object"), _T("Duplicate default signature detected") );
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// Delete a message source
// 
////////////////////////////////////////////////////////////////////////////////
int CSignatures::DoDelete( CURLAction& action )
{
	XSignatures sig;
	
	// Check Permissions
	if( GetXMLCache().m_Signatures.Query( m_SignatureID, sig ) )
	{
		// MJM - Edit access is required per DevTrack item 681
		if( sig.m_AgentID )
		{
			RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, sig.m_AgentID, EMS_EDIT_ACCESS );
		}
		else
		{
			RequireAgentRightLevel( EMS_OBJECT_TYPE_GROUP, sig.m_GroupID, EMS_EDIT_ACCESS );
		}	
	}
	else
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}

	if( TSignatures::Delete( GetQuery() ) == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

	InvalidateSignatures();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create a new, blank message source
// 
////////////////////////////////////////////////////////////////////////////////
int CSignatures::DoInsert( CURLAction& action )
{
	CEMSString sRedirectURL;

	DecodeForm();

	if( m_AgentID == 0 && m_GroupID == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	
	CheckForDuplicates();
	
	if( m_AgentID )
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_EDIT_ACCESS );
	}
	else
	{
		RequireAgentRightLevel( EMS_OBJECT_TYPE_GROUP, m_GroupID, EMS_EDIT_ACCESS );
	}

	TSignatures::Insert( GetQuery() );

	InvalidateSignatures();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecodeForm
// 
////////////////////////////////////////////////////////////////////////////////
void CSignatures::DecodeForm(void)
{
	tstring sType;
	tstring sTbOrTc;
	int iSelectBox;

	GetISAPIData().GetXMLTCHAR( _T("SignatureText"), &m_Signature,  m_SignatureLen, m_SignatureAllocated, false );

	CEMSString sBody;
	sBody.assign( m_Signature );	
	sBody.FixInlineTags(true);
	//sBody.FixBRTag();
	PutStringProperty( sBody, &m_Signature, &m_SignatureAllocated );	

	GetISAPIData().GetXMLTCHAR( _T("Name"), m_Name, 64, false );

	GetISAPIData().GetXMLString( _T("Type"), sType, false );

	if( sType.compare( _T("Group") ) == 0 )
	{
		GetISAPIData().GetXMLLong( _T("TypeID"), m_GroupID, false );
	}
	else if( sType.compare( _T("Agent") ) == 0 )
	{
		GetISAPIData().GetXMLLong( _T("TypeID"), m_AgentID, false );	
	}
	else
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}

	GetISAPIData().GetXMLLong( _T("ID"), m_SignatureID, false );

	GetISAPIData().GetXMLString( _T("TbOrTc"), sTbOrTc);
	if( sTbOrTc == "Tb" )
	{
		GetISAPIData().GetXMLLong( _T("Tb"), iSelectBox );
		m_ObjectTypeID = 2;			
	}
	else
	{
		GetISAPIData().GetXMLLong( _T("Tc"), iSelectBox );
		m_ObjectTypeID = 6;		
	}
	
	if ( iSelectBox == 1 )
	{
		THROW_VALIDATION_EXCEPTION( _T("Unassigned Objects"), _T("Default Signatures cannot be associated with Unassigned Objects") );
	}
	else if ( iSelectBox == 0 )
	{
		m_ActualID = 0;
		m_ObjectTypeID = 0;
	}
	else
	{
		m_ActualID = iSelectBox;	
	}
	
}


////////////////////////////////////////////////////////////////////////////////
// 
// GenerateXML
// 
////////////////////////////////////////////////////////////////////////////////
void CSignatures::GenerateXML( void )
{
	int nGroup = 0;
	GetISAPIData().GetURLLong( _T("Group"), nGroup, true );
	CEMSString sString;

	GetXMLGen().AddChildElem( _T("Signatures") );

	if( m_GroupID || nGroup )
	{
		GetXMLGen().AddChildAttrib( _T("Type"), _T("Group") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_GroupID );
		AddGroupName( _T("Name"), m_GroupID );
	}
	else
	{
		GetXMLGen().AddChildAttrib( _T("Type"), _T("Agent") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_AgentID );
		AddAgentName( _T("Name"), m_AgentID );
	}

	GetXMLGen().IntoElem();

	GetXMLGen().AddChildElem( _T("Signature") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_SignatureID );
	GetXMLGen().AddChildAttrib( _T("Name"), m_Name );
	GetXMLGen().AddChildAttrib( _T("ActualID"), m_ActualID );
	GetXMLGen().AddChildAttrib( _T("ObjectTypeID"), m_ObjectTypeID );
	
	tstring sUserAgent(GetBrowserSession().m_UserAgent);
	dca::String sTemp = sUserAgent.c_str();
	dca::String sTemp2 = sTemp.substr(0,17);
	if ( sTemp2 == "Internet.Explorer" )
	{
		bool bIsOldSig = true;
		tstring sSig;
		tstring::size_type posTagOpen;
		tstring::size_type posTagClose;
		sSig = m_Signature;
		posTagOpen = sSig.find(_T("<"));
		posTagClose = sSig.find(_T(">"));
		
		if( posTagOpen != tstring::npos )
		{
			if ( posTagClose != tstring::npos && posTagClose > posTagOpen )
				bIsOldSig = false;
		}
		
		if ( bIsOldSig )
		{
			sString.assign( m_Signature );
			sString.EscapeHTMLAndNewLines();
			GetXMLGen().SetChildData( sString.c_str(), TRUE );		
		}
		else
		{
			GetXMLGen().SetChildData( m_Signature, 1 );
		}		
	}
	else
	{
		GetXMLGen().SetChildData( m_Signature, 1 );
	}
	
	GetXMLGen().OutOfElem();

	if ( !m_AgentID && !m_GroupID )
		nGroup ? ListGroupNames() : ListAgentNames();

	ListTicketBoxNames( GetXMLGen() );
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

////////////////////////////////////////////////////////////////////////////////
// 
// GenerateXML
// 
////////////////////////////////////////////////////////////////////////////////
void CSignatures::GenerateXMLList( void )
{
	// what to name it?
	m_GroupID ? GetXMLGen().AddChildElem( _T("GroupSignature") ) :
				GetXMLGen().AddChildElem( _T("AgentSignature") );
	
	GetXMLGen().AddChildAttrib( _T("ID"), m_SignatureID );

	// Agent
	GetXMLGen().AddChildAttrib( _T("AgentID"), m_AgentID );
	AddAgentName( _T("AgentName"), m_AgentID );

	// Group
	GetXMLGen().AddChildAttrib( _T("GroupID"), m_GroupID );
	AddGroupName( _T("GroupName"), m_GroupID );

	GetXMLGen().AddChildAttrib( _T("Name"), m_Name );
	GetXMLGen().AddChildAttrib( _T("ActualID"), m_ActualID );
	GetXMLGen().AddChildAttrib( _T("ObjectTypeID"), m_ObjectTypeID );	

	GetXMLGen().SetChildData( m_Signature, 1 );
}
