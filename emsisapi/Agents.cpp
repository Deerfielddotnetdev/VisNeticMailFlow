 // Agents.cpp: implementation of the CAgents class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Agents.h"
#include "TicketBoxViewFns.h"
#include <lm.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAgents::CAgents( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nRefreshLeftPane = 0;
}

CAgents::~CAgents()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
//  Run - the main entry point
// 
////////////////////////////////////////////////////////////////////////////////
int CAgents::Run( CURLAction& action )
{
	CEMSString sID;
	tstring sAction;
	
	// get the action one way or another...
	if (!GetISAPIData().GetXMLString( _T("Action"), sAction, true ))
		GetISAPIData().GetFormString( _T("Action"), sAction, true );


	

			if ((GetISAPIData().GetXMLString( _T("Action"), sAction, true )) ||
				(GetISAPIData().GetFormString( _T("Action"), sAction, true )) ||
				(GetISAPIData().GetURLString( _T("Action"), sAction, true )))
			{
				if (sAction.compare(_T("sort")) == 0)
				{
					return ChangeSortOrder( action );					
				}
			}

	// get the selectId one way or the another...
	if (!GetISAPIData().GetXMLLong( _T("selectId"), m_AgentID, true ))
	{
		if (!GetISAPIData().GetURLLong( _T("selectId"), m_AgentID, true ))
		{
			if (GetISAPIData().GetFormString( _T("selectId"), sID, true ) )
				sID.CDLGetNextInt(m_AgentID);
		}
	}

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("editagent") ) == 0)
	{
		// Check security
		RequireAdmin();

		if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();
			return Update( action );
		}
		else
		{
			return QueryOne( action );
		}
	} 
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("ntagents") ) == 0)
	{
		DISABLE_IN_DEMO();
		RequireAdmin();
		return ListNTAgents( action );
	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("agentpreferences") ) == 0
			  || _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("apreferences") ) == 0 
			  || _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("preferencegeneral") ) == 0  
			  || _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("preferencesignatures") ) == 0 )
	{
		return AgentPreferences( action, sAction);
	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("setstatus") ) == 0 )
	{
		if( sAction.compare( _T("update") ) == 0 )
		{
			if (!GetISAPIData().GetXMLLong( _T("statusid"), m_StatusID, true ))
			{
				if (!GetISAPIData().GetURLLong( _T("statusid"), m_StatusID, true ))
				{
					GetISAPIData().GetFormLong( _T("statusid"), m_StatusID);
				}
			}
			tstring sStatus;
			if (!GetISAPIData().GetXMLString( _T("statustext"), sStatus, true ))
			{
				if(!GetISAPIData().GetFormString( _T("statustext"), sStatus, true ))
				{
					GetISAPIData().GetURLString( _T("statustext"), sStatus);
				}
			}				
			return SetStatus( m_StatusID, sStatus);
		}
		else if( sAction.compare( _T("logout") ) == 0 )
		{
			int nAgentID = 0;
			if (!GetISAPIData().GetXMLLong( _T("agentid"), nAgentID, true ))
			{
				if (!GetISAPIData().GetURLLong( _T("agentid"), nAgentID, true ))
				{
					GetISAPIData().GetFormLong( _T("agentid"), nAgentID);
				}
			}
			if(nAgentID > 0)
			{
				return LogoutAgent(nAgentID);
			}
			else
			{
				return 0;
			}
		}
		else
		{
			int nAgentID = 0;
			if (!GetISAPIData().GetXMLLong( _T("agentid"), nAgentID, true ))
			{
				if (!GetISAPIData().GetURLLong( _T("agentid"), nAgentID, true ))
				{
					GetISAPIData().GetFormLong( _T("agentid"), nAgentID, true);
				}
			}
			if(nAgentID > 0)
			{
				tstring sValue;
	
				if(m_AgentID == 0)
					m_AgentID = GetAgentID();

				GetServerParameter(EMS_SRVPARAM_ALLOW_AGENT_LOGOFF, sValue, "0");

				if(_ttoi(sValue.c_str()) == 0)
				{
					THROW_EMS_EXCEPTION_NOLOG( E_AccessDenied, _T("Logout option has been disabled by the Administrator!") );
				}

				RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, nAgentID, EMS_DELETE_ACCESS );
				tstring sStatusText;
				int nLoggedIn = 0;
				int nStateID = m_ISAPIData.m_SessionMap.GetAgentStatus(nAgentID,sStatusText,nLoggedIn);

				GetXMLGen().AddChildElem( _T("Agent") );
				GetXMLGen().AddChildAttrib( _T("ID"), nAgentID );
				GetXMLGen().AddChildAttrib( _T("StatusID"), nStateID );
				GetXMLGen().AddChildAttrib( _T("LoggedIn"), nLoggedIn );
			}
			return 0;
		}		
	}
	else
	{
		// check security
		RequireAdmin();
	}
	
	if( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();
		Delete();
	}
	
	return ListAll( action );
}


////////////////////////////////////////////////////////////////////////////////
// 
// ListAll
// 
////////////////////////////////////////////////////////////////////////////////
int CAgents::ListAll( CURLAction& action )
{
	int nAgentCount = 0;
	int nEnabledAgentCount = 0;
	int nAlertCount = 0;

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_AgentID );
	BINDCOL_TCHAR( GetQuery(), m_LoginName );
	BINDCOL_TCHAR( GetQuery(), m_Name );
	BINDCOL_BIT( GetQuery(), m_IsEnabled );
	BINDCOL_TCHAR( GetQuery(), m_NTDomain );
	BINDCOL_LONG( GetQuery(), m_IsNTUser );
	BINDCOL_TCHAR( GetQuery(), m_szDefaultEmailAddress );
	BINDCOL_LONG_NOLEN( GetQuery(), nAlertCount );
	BINDCOL_LONG( GetQuery(), m_StatusID );
	BINDCOL_TCHAR( GetQuery(), m_StatusText );	

	GetQuery().Execute( _T("SELECT Agents.AgentID,LoginName,Name,IsEnabled,NTDomain,IsNTUser,DataValue, ")
						_T("(SELECT COUNT(*) FROM Alerts WHERE Alerts.AgentID=Agents.AgentID),Agents.StatusID,StatusText ")
						_T("FROM Agents LEFT OUTER JOIN PersonalData ")
						_T("ON Agents.DefaultAgentAddressID = PersonalData.PersonalDataID ")
						_T("WHERE IsDeleted=0 ")
						_T("ORDER BY Name") );

	while( GetQuery().Fetch() == S_OK )
	{
		DecryptPassword();

		nAgentCount++;

		if( m_IsEnabled )
			nEnabledAgentCount++;

		GetLongData( GetQuery() );
		GetXMLGen().AddChildElem( _T("Agent") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_AgentID );
		GetXMLGen().AddChildAttrib( _T("Name"), m_Name );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
		GetXMLGen().AddChildAttrib( _T("LoginName"), m_LoginName );
		GetXMLGen().AddChildAttrib( _T("IsNTUser"), m_IsNTUser );
		GetXMLGen().AddChildAttrib( _T("NTDomain"), m_IsNTUser && m_NTDomainLen ? m_NTDomain : _T("") );
		GetXMLGen().AddChildAttrib( _T("StatusID"), m_StatusID );
		GetXMLGen().AddChildAttrib( _T("StatusText"), m_StatusText );
		
		if( m_szDefaultEmailAddressLen != SQL_NULL_DATA )
		{
			GetXMLGen().AddChildAttrib( _T("DefaultEmailAddress"), m_szDefaultEmailAddress );
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("DefaultEmailAddress"), _T("") );
		}

		GetXMLGen().AddChildAttrib( _T("AlertCount"), nAlertCount );	
	}
#if defined(HOSTED_VERSION)
	// Add XML
	GetXMLGen().AddChildElem( _T("LicenseInfo") );
	GetXMLGen().AddChildAttrib( _T("EnabledAgents"), GetISAPIData().m_LicenseMgr.GetNumAgents( GetQuery()) );
	GetXMLGen().AddChildAttrib( _T("TotalAgents"), nAgentCount );
	GetXMLGen().AddChildAttrib( _T("LicensedAgents"), 
		                        GetISAPIData().m_LicenseMgr.GetMaxAgents( GetQuery()) );
#else
	GetISAPIData().m_LicenseMgr.GetKeyInfo( GetQuery() );

	// Add XML
	GetXMLGen().AddChildElem( _T("LicenseInfo") );
	GetXMLGen().AddChildAttrib( _T("EnabledAgents"), nEnabledAgentCount );
	GetXMLGen().AddChildAttrib( _T("TotalAgents"), nAgentCount );
	GetXMLGen().AddChildAttrib( _T("LicensedAgents"), 
		                        GetISAPIData().m_LicenseMgr.m_MFKeyAgentCount );
#endif
	GetXMLGen().AddChildElem( _T("RefreshLeftPane") );
	GetXMLGen().AddChildAttrib( _T("Value"), m_nRefreshLeftPane );
	
	GetXMLGen().AddChildElem( _T("Sort") );
	GetXMLGen().AddChildAttrib( _T("SortBy"), 3 );
	GetXMLGen().AddChildAttrib( _T("SortAscending"), 1 );
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// ListAll
// 
////////////////////////////////////////////////////////////////////////////////
int CAgents::ChangeSortOrder( CURLAction& action )
{
	int nAgentCount = 0;
	int nEnabledAgentCount = 0;
	int nAlertCount = 0;
	tstring sSortBy;
	tstring sSortOrder;
	tstring sOrderBy;
	int nSortBy = 3;
	CEMSString sQuery;

	// get the sort field
	GetISAPIData().GetURLLong( _T("SortBy"), nSortBy );
			
	// get the sort order
	int nSortAsc = 0;
	GetISAPIData().GetURLLong( _T("SortAscending"), nSortAsc );
	
	if (nSortAsc == 0)
	{
		sSortOrder = "DESC";
	}
	switch(nSortBy)
	{
	case 1:
		sSortBy="IsEnabled";
		break;
	case 2:
		sSortBy="Agents.StatusID";
		break;
	case 3:
		sSortBy="Name";
		break;
	case 4:
		sSortBy="DataValue";
		break;
	case 5:
		sSortBy="LoginName";
		break;
	case 6:
		sSortBy="IsNTUser";
		break;
	default:
		sSortBy="Name";
		break;
	}
	sOrderBy= " Order By " + sSortBy + " " + sSortOrder;

	sQuery.Format( _T("SELECT Agents.AgentID,LoginName,Name,IsEnabled,NTDomain,IsNTUser,DataValue, ")
						_T("(SELECT COUNT(*) FROM Alerts WHERE Alerts.AgentID=Agents.AgentID),Agents.StatusID,StatusText ")
						_T("FROM Agents LEFT OUTER JOIN PersonalData ")
						_T("ON Agents.DefaultAgentAddressID = PersonalData.PersonalDataID ")
						_T("WHERE IsDeleted=0 ") );
	sQuery.append(sOrderBy);

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_AgentID );
	BINDCOL_TCHAR( GetQuery(), m_LoginName );
	BINDCOL_TCHAR( GetQuery(), m_Name );
	BINDCOL_BIT( GetQuery(), m_IsEnabled );
	BINDCOL_TCHAR( GetQuery(), m_NTDomain );
	BINDCOL_LONG( GetQuery(), m_IsNTUser );
	BINDCOL_TCHAR( GetQuery(), m_szDefaultEmailAddress );
	BINDCOL_LONG_NOLEN( GetQuery(), nAlertCount );
	BINDCOL_LONG( GetQuery(), m_StatusID );
	BINDCOL_TCHAR( GetQuery(), m_StatusText );	

	GetQuery().Execute(sQuery.c_str());

	while( GetQuery().Fetch() == S_OK )
	{
		DecryptPassword();

		nAgentCount++;

		if( m_IsEnabled )
			nEnabledAgentCount++;

		GetLongData( GetQuery() );
		GetXMLGen().AddChildElem( _T("Agent") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_AgentID );
		GetXMLGen().AddChildAttrib( _T("Name"), m_Name );
		GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
		GetXMLGen().AddChildAttrib( _T("LoginName"), m_LoginName );
		GetXMLGen().AddChildAttrib( _T("IsNTUser"), m_IsNTUser );
		GetXMLGen().AddChildAttrib( _T("NTDomain"), m_IsNTUser && m_NTDomainLen ? m_NTDomain : _T("") );
		GetXMLGen().AddChildAttrib( _T("StatusID"), m_StatusID );
		GetXMLGen().AddChildAttrib( _T("StatusText"), m_StatusText );
		
		if( m_szDefaultEmailAddressLen != SQL_NULL_DATA )
		{
			GetXMLGen().AddChildAttrib( _T("DefaultEmailAddress"), m_szDefaultEmailAddress );
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("DefaultEmailAddress"), _T("") );
		}

		GetXMLGen().AddChildAttrib( _T("AlertCount"), nAlertCount );	
	}

#if defined(HOSTED_VERSION)
	// Add XML
	GetXMLGen().AddChildElem( _T("LicenseInfo") );
	GetXMLGen().AddChildAttrib( _T("EnabledAgents"), GetISAPIData().m_LicenseMgr.GetNumAgents( GetQuery()) );
	GetXMLGen().AddChildAttrib( _T("TotalAgents"), nAgentCount );
	GetXMLGen().AddChildAttrib( _T("LicensedAgents"), 
		                        GetISAPIData().m_LicenseMgr.GetMaxAgents( GetQuery()) );
#else
	GetISAPIData().m_LicenseMgr.GetKeyInfo( GetQuery() );

	// Add XML
	GetXMLGen().AddChildElem( _T("LicenseInfo") );
	GetXMLGen().AddChildAttrib( _T("EnabledAgents"), nEnabledAgentCount );
	GetXMLGen().AddChildAttrib( _T("TotalAgents"), nAgentCount );
	GetXMLGen().AddChildAttrib( _T("LicensedAgents"), 
		                        GetISAPIData().m_LicenseMgr.m_MFKeyAgentCount );
#endif
	
	GetXMLGen().AddChildElem( _T("RefreshLeftPane") );
	GetXMLGen().AddChildAttrib( _T("Value"), m_nRefreshLeftPane );

	GetXMLGen().AddChildElem( _T("Sort") );
	GetXMLGen().AddChildAttrib( _T("SortBy"), nSortBy );
	GetXMLGen().AddChildAttrib( _T("SortAscending"), nSortAsc );
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// QueryOne
// 
////////////////////////////////////////////////////////////////////////////////
int CAgents::QueryOne( CURLAction& action )
{
	unsigned int GroupID;
	long GroupIDLen;
	TPersonalData personaldata;
	int DefaultAddrID;

	m_szDefaultEmailAddressLen = 4;
	m_szDefaultEmailAddress[0] = _T('\0');
	m_MaxPersonalDataID = 0;

	if( m_AgentID != 0 )
	{
		GetQuery().Initialize();
		BINDCOL_TCHAR( GetQuery(), m_LoginName );
		BINDCOL_TCHAR( GetQuery(), m_Name );
		BINDCOL_BIT( GetQuery(), m_IsEnabled );
		BINDCOL_LONG( GetQuery(), m_MaxReportRowsPerPage );
		BINDCOL_BIT( GetQuery(), m_StdResponseApprovalsRequired );
		BINDCOL_BIT( GetQuery(), m_MessageApprovalsRequired );
		BINDCOL_BIT( GetQuery(), m_QuoteMsgInReply );
		BINDCOL_TCHAR( GetQuery(), m_ReplyQuotedPrefix );
		BINDCOL_TCHAR( GetQuery(), m_NTDomain );
		BINDCOL_LONG( GetQuery(), m_IsNTUser );
		BINDCOL_BIT( GetQuery(), m_UseIM );
		BINDCOL_BIT( GetQuery(), m_NewMessageFormat );
		BINDCOL_LONG( GetQuery(), m_EscalateToAgentID );
		BINDCOL_LONG_NOLEN( GetQuery(), DefaultAddrID );
		BINDCOL_LONG( GetQuery(), m_RequireGetOldest );
		BINDCOL_LONG( GetQuery(), m_OutboundApprovalFromID );
		BINDCOL_BIT( GetQuery(), m_UseAutoFill );
		BINDCOL_LONG( GetQuery(), m_StatusID );
		BINDPARAM_LONG( GetQuery(), m_AgentID );

		GetQuery().Execute( _T("SELECT LoginName,Name,IsEnabled,MaxReportRowsPerPage,")
							_T("StdResponseApprovalsRequired,MessageApprovalsRequired,")
							_T("QuoteMsgInReply,ReplyQuotedPrefix,NTDomain,IsNTUser,UseIM,NewMessageFormat,")
							_T("EscalateToAgentID,DefaultAgentAddressID,RequireGetOldest,OutboundApprovalFromID,UseAutoFill,StatusID ")
							_T("FROM Agents ")
							_T("WHERE AgentID=? AND IsDeleted=0 ") );

		if( S_OK != GetQuery().Fetch() )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}


	if( m_AgentID == 0 )
	{
		action.m_sPageTitle.assign( _T("New Agent") );

		enum _NETSETUP_JOIN_STATUS status;
		LPWSTR Domain = NULL;
		DWORD res;
		
		res = NetGetJoinInformation( NULL, &Domain, &status );
		
		if( res == NERR_Success )
		{
			WideCharToMultiByte( CP_ACP, 0, Domain, wcslen(Domain)+1,
								 m_NTDomain, AGENTS_NTDOMAIN_LENGTH, 0, 0 );
		}
		
		m_GroupIDs.insert( EMS_GROUPID_EVERYONE );
	}
	else
	{
		// Group Membership
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		BINDCOL_LONG( GetQuery(), GroupID );
		GetQuery().Execute( _T("SELECT GroupID ")
							_T("FROM AgentGroupings ")
							_T("WHERE AgentID=?") );

		while( S_OK == GetQuery().Fetch() )
		{
			m_GroupIDs.insert( GroupID );
		}

		// List personal data
		GetQuery().Initialize();

		BINDPARAM_LONG(GetQuery(), m_AgentID);
		BINDCOL_LONG(GetQuery(), personaldata.m_PersonalDataID );
		BINDCOL_LONG(GetQuery(), personaldata.m_PersonalDataTypeID );
		BINDCOL_TCHAR(GetQuery(), personaldata.m_DataValue );
		BINDCOL_TCHAR(GetQuery(), personaldata.m_Note );

		GetQuery().Execute(	_T("SELECT PersonalDataID,PersonalDataTypeID,DataValue,Note ")
							_T("FROM PersonalData ")
							_T("WHERE AgentID = ? ") );
		
		while( GetQuery().Fetch() == S_OK )
		{
			if( personaldata.m_PersonalDataID == DefaultAddrID )
			{
				_tcscpy( m_szDefaultEmailAddress, personaldata.m_DataValue );
			}
			else
			{
				m_Data.push_back( personaldata );
				if( personaldata.m_PersonalDataID >= m_MaxPersonalDataID )
				{
					m_MaxPersonalDataID = personaldata.m_PersonalDataID + 1;
				}
			}
		}
	}

	GenerateXML();

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// ListNTAgents
// 
////////////////////////////////////////////////////////////////////////////////
int CAgents::ListNTAgents( CURLAction& action )
{
	PNET_DISPLAY_USER pBuff, p;
	DWORD res, dwRec, i = 0;
	LPWSTR PDCName = NULL;
	LPWSTR Domain = NULL;
	enum _NETSETUP_JOIN_STATUS status;
#ifndef _UNICODE
	TCHAR szNameBuffer[512];
#endif
	tstring sDomain;

	if( GetISAPIData().GetURLString( _T("Domain"), sDomain, true ) )
	{
		// We were provided a "domain" URL parameter
#ifndef _UNICODE
		wchar_t szDomain[256];

		MultiByteToWideChar( CP_ACP, 0, sDomain.c_str(), sDomain.size()+1,
							 szDomain, 256 );
		res = NetGetDCName( NULL, szDomain, (LPBYTE*) &PDCName );
#else
		res = NetGetDCName( NULL, sDomain.c_str(), (LPBYTE*) &PDCName );
#endif

		if( res == NERR_Success )
		{		
			GetXMLGen().AddChildElem( _T("PDC") );

			if( sDomain.size() < 1 )
			{
				res = NetGetJoinInformation( PDCName, &Domain, &status );
				
				if( res == NERR_Success )
				{
#ifndef _UNICODE
					WideCharToMultiByte( CP_ACP, 0, Domain, wcslen(Domain)+1,
						szNameBuffer, sizeof(szNameBuffer), 0, 0 );
					GetXMLGen().AddChildAttrib( _T("Domain"), szNameBuffer );
#else
					GetXMLGen().AddChildAttrib( _T("Domain"), p->usri1_name );
#endif
				}
				else
				{
					GetXMLGen().AddChildAttrib( _T("Domain"), _T("") );
				}		
			}
			else
			{
				GetXMLGen().AddChildAttrib( _T("Domain"), sDomain.c_str() );
			}

#ifndef _UNICODE
			WideCharToMultiByte( CP_ACP, 0, PDCName, wcslen(PDCName)+1,
								 szNameBuffer, 512, 0, 0 );
			GetXMLGen().AddChildAttrib( _T("Name"), szNameBuffer );
#else
			GetXMLGen().AddChildAttrib( _T("Name"), PDCName );
#endif
		}
		else
		{
			return 0; // domain not found
		}
	}
	else
	{
		// We were not provided a "domain" URL parameter - use the system default
		res = NetGetDCName( NULL, NULL, (LPBYTE*) &PDCName );

		if( res == NERR_Success )
		{
#ifndef _UNICODE
			WideCharToMultiByte( CP_ACP, 0, PDCName, wcslen(PDCName)+1,
								 szNameBuffer, sizeof(szNameBuffer), 0, 0 );
			GetXMLGen().AddChildAttrib( _T("Name"), szNameBuffer );
#else
			GetXMLGen().AddChildAttrib( _T("Name"), PDCName );
#endif
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("Name"), _T("") );
		}

		res = NetGetJoinInformation( PDCName, &Domain, &status );
		
		if( res == NERR_Success )
		{
#ifndef _UNICODE
			WideCharToMultiByte( CP_ACP, 0, Domain, wcslen(Domain)+1,
								 szNameBuffer, sizeof(szNameBuffer), 0, 0 );
			GetXMLGen().AddChildAttrib( _T("Domain"), szNameBuffer );
#else
			GetXMLGen().AddChildAttrib( _T("Domain"), p->usri1_name );
#endif
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("Domain"), _T("") );
		}
	}

	do // begin do
	{ 
		//
		// Call the NetQueryDisplayInformation function;
		//   specify information level 3 (group account information).
		//
		res = NetQueryDisplayInformation( PDCName, 1, i, 1000, 0xFFFFFFFF, &dwRec, (void**)&pBuff);
		//
		// If the call succeeds,
		//
		if((res==ERROR_SUCCESS) || (res==ERROR_MORE_DATA))
		{
			p = pBuff;
			for(;dwRec>0;dwRec--)
			{
				if( (p->usri1_flags & (UF_ACCOUNTDISABLE|UF_LOCKOUT) ) == 0 )
				{
					GetXMLGen().AddChildElem( _T("NTUser") );

#ifndef _UNICODE
					WideCharToMultiByte( CP_ACP, 0, p->usri1_name, wcslen(p->usri1_name)+1,
						                 szNameBuffer, sizeof(szNameBuffer), 0, 0 );
					GetXMLGen().AddChildAttrib( _T("username"), szNameBuffer );
#else
					GetXMLGen().AddChildAttrib( _T("username"), p->usri1_name );
#endif

#ifndef _UNICODE
					WideCharToMultiByte( CP_ACP, 0, p->usri1_full_name, wcslen(p->usri1_full_name)+1,
						                 szNameBuffer, sizeof(szNameBuffer), 0, 0 );
					GetXMLGen().AddChildAttrib( _T("fullname"), szNameBuffer );
#else
					GetXMLGen().AddChildAttrib( _T("fullname"), p->usri1_full_name );
#endif

#ifndef _UNICODE
					WideCharToMultiByte( CP_ACP, 0, p->usri1_comment, wcslen(p->usri1_comment)+1,
						                 szNameBuffer, sizeof(szNameBuffer), 0, 0 );
					GetXMLGen().AddChildAttrib( _T("comment"), szNameBuffer );
#else
					GetXMLGen().AddChildAttrib( _T("comment"), p->usri1_comment );
#endif

				}

				i = p->usri1_next_index;
				p++;
			}
			//
			// Free the allocated memory.
			//
			NetApiBufferFree(pBuff);
		}
		//
		// Continue while there is more data.
		//
	} while (res==ERROR_MORE_DATA); // end do


	if( PDCName )
		NetApiBufferFree( PDCName );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update
// 
////////////////////////////////////////////////////////////////////////////////
int CAgents::Update( CURLAction& action )
{
	set<unsigned int>::iterator iter;
	set<unsigned int> DBGroupIDS;
	set<unsigned int> DBPDList;
	int nGroupID;
	TPersonalData pd;
	int PersonalDataID;
	int DefaultAddrID;
	tstring sValue;

	DecodeForm();

	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_AgentID );
	BINDPARAM_TCHAR( GetQuery(), m_LoginName );
	BINDPARAM_TCHAR( GetQuery(), m_Name );
	GetQuery().Execute( _T("SELECT AgentID FROM Agents ")
						_T("WHERE AgentID<>? AND IsDeleted=0 AND (LoginName=? OR Name=?) ") );

	if( GetQuery().Fetch() == S_OK )
	{
		THROW_EMS_EXCEPTION( E_Duplicate_Name, CEMSString(_T("Agent names must be unique"))  );
	}

	if ( m_AgentID == 0 )
	{
		int ObjectTypeID = EMS_OBJECT_TYPE_AGENT;
		TIMESTAMP_STRUCT now;
		GetTimeStamp( now );
		
		if ( m_IsEnabled )
		{
			CheckLicense();
		}

		// Create the object ID
		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), ObjectTypeID);
		GetQuery().Execute( _T("INSERT INTO Objects (ObjectTypeID) VALUES (?)") );
		m_ObjectID = GetQuery().GetLastInsertedID();

		// Create the new agent as deleted
		m_IsDeleted = 1;

		_tcscpy( m_ReplyQuotedPrefix, _T(">") );
		
		EncryptPassword();
		
		// Set m_CloseTicket
		GetServerParameter( EMS_SRVPARAM_DEFAULT_CLOSE_TICKET_AFTER_SEND, sValue );
		if(sValue=="0")
		{
			m_CloseTicket = 0;
		}
		else if(sValue=="1")
		{
			m_CloseTicket = 1;
		}
		else
		{
			m_CloseTicket = 2;
		}
				
		// Set m_RouteToInbox
		GetServerParameter( EMS_SRVPARAM_DEFAULT_ROUTE_TO_ME, sValue );
		if(sValue=="0")
		{
			m_RouteToInbox = 0;
		}
		else if(sValue=="1")
		{
			m_RouteToInbox = 1;
		}
		else
		{
			m_RouteToInbox = 2;
		}
		
		_tcscpy( m_StatusText, _T("Offline") );
		_tcscpy( m_OnlineText, _T("Online") );
		_tcscpy( m_AwayText, _T("Away") );
		_tcscpy( m_NotAvailText, _T("Not Available") );
		_tcscpy( m_DndText, _T("Do Not Disturb") );
		_tcscpy( m_OfflineText, _T("Offline") );
		_tcscpy( m_OooText, _T("Out of Office") );
		m_AutoStatusTypeID = 2;

		TAgents::Insert( GetQuery() );
		DecryptPassword();

		// Update the object row with the actual ID
		GetQuery().Reset( true );

		BINDPARAM_LONG( GetQuery(), m_AgentID );
		BINDPARAM_TIME_NOLEN( GetQuery(), now );
		BINDPARAM_LONG( GetQuery(), m_ObjectID );
	
		GetQuery().Execute( _T("UPDATE Objects SET ")
							_T("ActualID = ?, ")
							_T("DateCreated = ? ")
							_T("WHERE ObjectID = ?") );

		// Add the default ticket box views
		TBVAddAgent( GetQuery(), m_AgentID, m_AgentID );

		// Put the agent in the requested groups
		for( iter = m_GroupIDs.begin(); iter != m_GroupIDs.end(); iter++ )
		{	
			nGroupID = (*iter);
			GetQuery().Reset( true );
			BINDPARAM_LONG( GetQuery(), m_AgentID );
			BINDPARAM_LONG( GetQuery(), nGroupID );
			GetQuery().Execute( _T("INSERT INTO AgentGroupings ")
								_T("(AgentID,GroupID) ")
								_T("VALUES ")
								_T("(?,?)") );
		}

		list<TPersonalData>::iterator PDiter;
		
		for ( PDiter = m_Data.begin(); PDiter != m_Data.end(); PDiter++)
		{
			PDiter->Insert(GetQuery());
			if( PDiter->m_PersonalDataID >= m_MaxPersonalDataID )
			{
				m_MaxPersonalDataID = PDiter->m_PersonalDataID + 1;
			}
		}

		pd.m_AgentID = m_AgentID;
		pd.m_ContactID = 0;
		pd.m_PersonalDataTypeID = EMS_PERSONAL_DATA_EMAIL;
		_tcscpy( pd.m_DataValue, m_szDefaultEmailAddress );
		pd.Insert( GetQuery() );

		// Create an access control item so that the new agent has edit access to himself
		// in case the agent is configured to use a custom access control list
		GetQuery().Reset( true );
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		BINDPARAM_LONG( GetQuery(), m_ObjectID );
		GetQuery().Execute( _T("INSERT INTO AccessControl ")
							_T("(AgentID,GroupID,AccessLevel,ObjectID) ")
							_T("VALUES ")
							_T("(?,0,3,?)") );

		// Last of all, undelete the agent.
		GetQuery().Reset( true );
		BINDPARAM_LONG( GetQuery(), pd.m_PersonalDataID );
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		GetQuery().Execute( _T("UPDATE Agents SET IsDeleted=0, DefaultAgentAddressID=? WHERE AgentID=?") );
	}
	else
	{
		if ( m_IsEnabled )
		{
			// Check to see if we are enabling this Agent
			long nIsEnabled = 0;
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_AgentID );
			BINDCOL_LONG_NOLEN( GetQuery(), nIsEnabled );
			GetQuery().Execute( _T("SELECT IsEnabled FROM Agents WHERE AgentID=?") );
			GetQuery().Fetch();
			if ( nIsEnabled == 0)
			{
				CheckLicense();
			}			
		}
		
		pd.m_AgentID = m_AgentID;
		pd.m_ContactID = 0;
		pd.m_PersonalDataTypeID = EMS_PERSONAL_DATA_EMAIL;
		_tcscpy( pd.m_DataValue, m_szDefaultEmailAddress );
		pd.m_PersonalDataID = GetDefaultEmailAddress();
		pd.Update( GetQuery() );

		if( GetQuery().GetRowCount() == 0 )
		{
			pd.Insert( GetQuery() );

			GetQuery().Reset( true );
			BINDPARAM_LONG( GetQuery(), pd.m_PersonalDataID );
			BINDPARAM_LONG( GetQuery(), m_AgentID );
			GetQuery().Execute( _T("UPDATE Agents SET DefaultAgentAddressID=? WHERE AgentID=?") );
		}
		DefaultAddrID = pd.m_PersonalDataID;

		// Can't disable the admin
		if( m_AgentID == EMS_AGENTID_ADMINISTRATOR )
			m_IsEnabled = 1;
	
		// Get the Status Text
		GetQuery().Initialize();
		BINDCOL_TCHAR( GetQuery(), m_StatusText );
		BINDPARAM_LONG( GetQuery(), m_StatusID );
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		GetQuery().Execute( _T("SELECT CASE ? WHEN 1 THEN OnlineText WHEN 2 THEN AwayText WHEN 3 THEN NotAvailText WHEN 4 THEN DndText WHEN 5 THEN OfflineText WHEN 6 THEN OooText END ")
							_T("FROM Agents WHERE AgentID=? ") );
		GetQuery().Fetch();

		GetQuery().Initialize();		
		BINDPARAM_TCHAR( GetQuery(), m_LoginName );
		BINDPARAM_TCHAR( GetQuery(), m_Name );
		BINDPARAM_BIT( GetQuery(), m_IsEnabled );

		if( m_IsNTUser == EMS_USER_TYPE_NT )
		{
			BINDPARAM_TCHAR( GetQuery(), m_NTDomain );
		}
		else if( m_IsNTUser == EMS_USER_TYPE_LOCAL && _tcscmp( m_Password, _T("********") ) )
		{
			BINDPARAM_TCHAR( GetQuery(), m_Password );
		}

		BINDPARAM_LONG( GetQuery(), m_IsNTUser );
		BINDPARAM_LONG( GetQuery(), m_EscalateToAgentID );
		BINDPARAM_LONG( GetQuery(), m_OutboundApprovalFromID );
		BINDPARAM_BIT( GetQuery(), m_RequireGetOldest );
		BINDPARAM_LONG( GetQuery(), m_StatusID );
		BINDPARAM_TCHAR( GetQuery(), m_StatusText );
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		
		if( m_IsNTUser == EMS_USER_TYPE_NT )
		{
			GetQuery().Execute( _T("UPDATE Agents ")
								_T("SET LoginName=?,Name=?,IsEnabled=?,NTDomain=?,IsNTUser=?,EscalateToAgentID=?,OutboundApprovalFromID=?,RequireGetOldest=?,StatusID=?,StatusText=? ")
								_T("WHERE AgentID=?") );
		}
		else if( m_IsNTUser == EMS_USER_TYPE_LOCAL )
		{
			if( _tcscmp( m_Password, _T("********") ) )
			{
				EncryptPassword();
				GetQuery().Execute( _T("UPDATE Agents ")
									_T("SET LoginName=?,Name=?,IsEnabled=?,Password=?,IsNTUser=?,EscalateToAgentID=?,OutboundApprovalFromID=?,RequireGetOldest=?,StatusID=?,StatusText=? ")
									_T("WHERE AgentID=?") );
			}
			else
			{
				GetQuery().Execute( _T("UPDATE Agents ")
									_T("SET LoginName=?,Name=?,IsEnabled=?,IsNTUser=?,EscalateToAgentID=?,OutboundApprovalFromID=?,RequireGetOldest=?,StatusID=?,StatusText=? ")
									_T("WHERE AgentID=?") );
			}
		}
		else // EMS_USER_TYPE_VMS
		{
			GetQuery().Execute( _T("UPDATE Agents ")
								_T("SET LoginName=?,Name=?,IsEnabled=?,IsNTUser=?,EscalateToAgentID=?,OutboundApprovalFromID=?,RequireGetOldest=?,StatusID=?,StatusText=? ")
								_T("WHERE AgentID=?") );
		}

		if( GetQuery().GetRowCount() == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	
		// Update Agent Status
		tstring sStatusText;
		sStatusText.assign(m_StatusText);
		GetISAPIData().m_SessionMap.SetAgentStatus(m_AgentID,m_StatusID,sStatusText);

		// Group Membership
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		BINDCOL_LONG_NOLEN( GetQuery(), nGroupID );
		GetQuery().Execute( _T("SELECT GroupID ")
							_T("FROM AgentGroupings ")
							_T("WHERE AgentID=?") );

		while( S_OK == GetQuery().Fetch() )
		{
			DBGroupIDS.insert( nGroupID );
		}

		for( iter = m_GroupIDs.begin(); iter != m_GroupIDs.end(); iter++ )
		{	
			nGroupID = (*iter);

			if( DBGroupIDS.find( nGroupID ) == DBGroupIDS.end() )
			{
				GetQuery().Reset( true );
				BINDPARAM_LONG( GetQuery(), m_AgentID );
				BINDPARAM_LONG( GetQuery(), nGroupID );
				GetQuery().Execute( _T("INSERT INTO AgentGroupings ")
									_T("(AgentID,GroupID) ")
									_T("VALUES ")
									_T("(?,?)") );		
			}
		}

		for( iter = DBGroupIDS.begin(); iter != DBGroupIDS.end(); iter++ )
		{	
			nGroupID = (*iter);

			if( ( nGroupID == EMS_GROUPID_ADMINISTRATORS && m_AgentID == EMS_AGENTID_ADMINISTRATOR)
				|| ( nGroupID == EMS_GROUPID_EVERYONE) )
			{
				// Can't remove membership!
			}
			else
			{
				if( m_GroupIDs.find( nGroupID ) == m_GroupIDs.end() )
				{
					GetQuery().Reset( true );
					BINDPARAM_LONG( GetQuery(), m_AgentID );
					BINDPARAM_LONG( GetQuery(), nGroupID );
					GetQuery().Execute( _T("DELETE FROM AgentGroupings ")
										_T("WHERE AgentID=? AND GroupID=? ") );
				}
			}	
		}

		
		GetQuery().Reset(true);
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		BINDCOL_LONG_NOLEN( GetQuery(), PersonalDataID );
		GetQuery().Execute( _T("SELECT PersonalDataID FROM PersonalData WHERE AgentID=?") );
		

		while( GetQuery().Fetch() == S_OK )
		{
			if( PersonalDataID != DefaultAddrID)
			{
				DBPDList.insert( PersonalDataID );
			}
		}
		
		list<TPersonalData>::iterator PDiter;
		
		for ( PDiter = m_Data.begin(); PDiter != m_Data.end(); PDiter++)
		{
			if( DBPDList.find( PDiter->m_PersonalDataID ) == DBPDList.end() )
			{
				PDiter->Insert(GetQuery());
				if( PDiter->m_PersonalDataID >= m_MaxPersonalDataID )
				{
					m_MaxPersonalDataID = PDiter->m_PersonalDataID + 1;
				}
			}
			else
			{
				PDiter->Update(GetQuery());
			}
		}

		set<unsigned int>::iterator DBPDiter;

		for( DBPDiter = DBPDList.begin(); DBPDiter != DBPDList.end(); DBPDiter++ )
		{
			bool bFound = false;
			for ( PDiter = m_Data.begin(); PDiter != m_Data.end(); PDiter++)
			{	
				if( *DBPDiter == PDiter->m_PersonalDataID )
				{
					bFound = true;
					break;
				}
			}
			if( bFound == false ) 
			{
				int nPDID = *DBPDiter;
				GetQuery().Reset(true);
				BINDPARAM_LONG( GetQuery(), nPDID );
				GetQuery().Execute( _T("DELETE FROM PersonalData WHERE PersonalDataID=?") );		
			}
		}
	}
	
	int nAppGroupID = 0;	
	
	// reset queued outbound messages, if any
	GetQuery().Initialize();	
	if ( m_OutboundApprovalFromID == 0 )
	{
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		GetQuery().Execute( _T("DELETE FROM Approvals WHERE RequestAgentID=? AND ApprovalObjectTypeID=1 ") );
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE OutboundMessageQueue SET IsApproved=1 WHERE OutboundMessageID IN (SELECT OutboundMessageID FROM OutboundMessages WHERE OutboundMessageStateID=7 AND AgentID=?) ") );
		GetQuery().Reset(false);
		GetQuery().Execute( _T("UPDATE OutboundMessages SET OutboundMessageStateID=3 WHERE OutboundMessageStateID=7 AND AgentID=? ") );	
	}
	else if ( m_OutboundApprovalFromID < 0 )
	{
		nAppGroupID = abs(m_OutboundApprovalFromID);
		BINDPARAM_LONG( GetQuery(), nAppGroupID );
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		GetQuery().Execute( _T("UPDATE Approvals SET ApproverGroupID=?,ApproverAgentID=0 WHERE RequestAgentID=? AND ApprovalObjectTypeID=1 ") );
	}
	else
	{
		BINDPARAM_LONG( GetQuery(), m_OutboundApprovalFromID );
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		GetQuery().Execute( _T("UPDATE Approvals SET ApproverGroupID=0,ApproverAgentID=? WHERE RequestAgentID=? AND ApprovalObjectTypeID=1 ") );
	}

#if defined(HOSTED_VERSION)
	GetISAPIData().m_LicenseMgr.UpdateNumAgents(GetQuery());
#endif
	
	// invalidate cache objects
	//InvalidateAgentEmails();
	InvalidateAgents();

	GetISAPIData().m_SessionMap.InvalidateAllAgentSessions(); 
	GetISAPIData().m_SessionMap.QueueSessionMonitors(1,0);

	if( m_AgentID == GetAgentID() )
		GetISAPIData().m_SessionMap.RefreshAgentSession( GetSession(), GetQuery() );

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Delete
// 
////////////////////////////////////////////////////////////////////////////////
void CAgents::Delete(void)
{
	int ObjectID;
	tstring sValue;

	if( m_AgentID <= EMS_AGENTID_ADMINISTRATOR )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}

	// Get the agent objectID
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_AgentID );
	BINDCOL_LONG_NOLEN( GetQuery(), ObjectID );
	GetQuery().Execute( _T("SELECT ObjectID ")
		                _T("FROM Agents ")
						_T("WHERE AgentID=?") );

	if( GetQuery().Fetch() != S_OK )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}

	m_nRefreshLeftPane = 1;

	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM AgentGroupings ")
						_T("WHERE AgentID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM AccessControl ")
						_T("WHERE AgentID=?") );

	// Point all alerts to administrator
	GetQuery().Reset( false );
	GetQuery().Execute( _T("UPDATE Alerts ")
						_T("SET AgentID=1 ")
						_T("WHERE AgentID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM IPRanges ")
						_T("WHERE AgentID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM PersonalData ")
						_T("WHERE AgentID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM Signatures ")
						_T("WHERE AgentID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM TaskList ")
						_T("WHERE AgentID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM StandardResponseUsage ")
						_T("WHERE AgentID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM StdResponseFavorites ")
						_T("WHERE AgentID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("UPDATE Tickets SET FolderID=0 ")
						_T("WHERE OwnerID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM Folders ")
						_T("WHERE AgentID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("DELETE FROM AgentContacts ")
						_T("WHERE AgentID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("UPDATE Tickets SET OwnerID=0 ")
						_T("WHERE OwnerID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("UPDATE Tickets SET LockedBy=0 ")
 						_T("WHERE LockedBy=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("UPDATE Contacts SET OwnerID=0 ")
						_T("WHERE OwnerID=?") );

	GetQuery().Reset( false );
	GetQuery().Execute( _T("UPDATE RoutingRules SET AssignToAgent=0 ")
						_T("WHERE AssignToAgent=?") );
	
	GetQuery().Reset( false );
	GetQuery().Execute( _T("UPDATE RoutingRules SET ForwardEnable=0 ")
						_T("WHERE ForwardFromAgent=?") );

	DeleteAllAgentViews( GetQuery(), m_AgentID );

	GetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_AGENTID, sValue );

	// are we the default owner of all tickets?
	if( _ttoi( sValue.c_str()) == m_AgentID )
	{
		sValue.assign( _T("0") );
		SetServerParameter( EMS_SRVPARAM_DEF_RR_MATCH_AGENTID, sValue );
		InvalidateServerParameters( true );
		GetRoutingEngine().ReloadConfig( EMS_RoutingRules );	
	}

	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_AgentID );
	GetQuery().Execute( _T("UPDATE Agents SET IsEnabled=0,IsDeleted=1 ")
						_T("WHERE AgentID=?") );
				
	if( GetQuery().GetRowCount() == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}		

	// delete access control items
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), ObjectID );
	GetQuery().Execute( _T("DELETE FROM AccessControl ")
						_T("WHERE ObjectID=?") );

	// update the object to use default rights... if not the agent will
	// be listed under "custom access control"
	GetQuery().Reset(false);
	GetQuery().Execute( _T("UPDATE Objects SET UseDefaultRights=1 WHERE ObjectID=?") );

#if defined(HOSTED_VERSION)
	GetISAPIData().m_LicenseMgr.UpdateNumAgents(GetQuery());
#endif

	// end the agent's session
	m_ISAPIData.m_SessionMap.InvalidateAgentSession( m_AgentID );

	InvalidateAgents();
}


////////////////////////////////////////////////////////////////////////////////
// 
// DecodeForm
// 
////////////////////////////////////////////////////////////////////////////////
void CAgents::DecodeForm(void)
{
	int nEnabled = 1;
	int nUserType = 0;
	CEMSString sCollection;
	CEMSString sField;
	int GroupID;
	int iSelectBox;
	int iSelectBoxApp;
	tstring sAgentGroup;
	tstring sAppAgentGroup;
	TPersonalData pd;
	bool bEveryoneFlag = false;
	list<TPersonalData>::iterator iter;

	GetISAPIData().GetXMLLong( _T("AgentID"), m_AgentID );
	GetISAPIData().GetXMLLong( _T("Enabled"), nEnabled );
	m_IsEnabled = (nEnabled == 0) ? 0 : 1;
	GetISAPIData().GetXMLTCHAR( _T("FirstName"), m_Name, 129 );

	GetISAPIData().GetXMLLong( _T("StatusID"), m_StatusID );

	//GetISAPIData().GetXMLLong( _T("EscalateToAgentID"), m_EscalateToAgentID );
	GetISAPIData().GetXMLString( _T("AgentOrGroup"), sAgentGroup );
	if( sAgentGroup == "Agent" )
	{
		GetISAPIData().GetXMLLong( _T("Agent"), iSelectBox );
		m_EscalateToAgentID = iSelectBox;
	}
	else
	{
		GetISAPIData().GetXMLLong( _T("Group"), iSelectBox );
		if ( iSelectBox != 0 )
		{
			m_EscalateToAgentID = -(iSelectBox);
		}
		else
		{
			m_EscalateToAgentID = 0;
		}		
	}

	GetISAPIData().GetXMLString( _T("AppAgentOrGroup"), sAppAgentGroup );
	if( sAppAgentGroup == "Agent" )
	{
		GetISAPIData().GetXMLLong( _T("AgentApp"), iSelectBoxApp );
		m_OutboundApprovalFromID = iSelectBoxApp;
	}
	else
	{
		GetISAPIData().GetXMLLong( _T("GroupApp"), iSelectBoxApp );
		if ( iSelectBoxApp != 0 )
		{
			m_OutboundApprovalFromID = -(iSelectBoxApp);
		}
		else
		{
			m_OutboundApprovalFromID = 0;
		}		
	}

	GetISAPIData().GetXMLLong( _T("UserType"), nUserType );
	m_IsNTUser = nUserType;

	if( m_IsNTUser == EMS_USER_TYPE_NT )
	{
		GetISAPIData().GetXMLTCHAR( _T("Domain"), m_NTDomain, 126 );
	}
	else if( m_IsNTUser == EMS_USER_TYPE_LOCAL )
	{
		GetISAPIData().GetXMLTCHAR( _T("Password"), m_Password, 51 );
	}

	GetISAPIData().GetXMLTCHAR( _T("Username"), m_LoginName, 51 );

	// Get group membership
	GetISAPIData().GetXMLString( _T("GroupIDCollection"), sCollection, false );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( GroupID ) )
	{
		m_GroupIDs.insert( GroupID );
		if( GroupID == EMS_GROUPID_EVERYONE )
		{
			bEveryoneFlag = true;
		}
	}

	if( !bEveryoneFlag )
	{
		m_GroupIDs.insert( EMS_GROUPID_EVERYONE );
	}

	// Get personaldata items (like contact editor)
	GetISAPIData().GetXMLLong( _T("PersonalDataMaxID"), m_MaxPersonalDataID, false );
	GetISAPIData().GetXMLString( _T("PersonalDataIDCollection"), sCollection, false );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( pd.m_PersonalDataID ) )
	{
		pd.m_AgentID = m_AgentID;
		pd.m_ContactID = 0;
		
		sField.Format( _T("PersonalDataType%d"), pd.m_PersonalDataID );
		GetISAPIData().GetXMLLong( sField.c_str(), pd.m_PersonalDataTypeID );

		sField.Format( _T("PersonalData%d"), pd.m_PersonalDataID );
		GetISAPIData().GetXMLTCHAR( sField.c_str(), pd.m_DataValue, 256 );

		m_Data.push_back( pd );
	}

	// default email address
	GetISAPIData().GetXMLTCHAR( _T("DefaultEmail"), m_szDefaultEmailAddress, 256, false );
	GetISAPIData().GetXMLLong( _T("RequireGetOldest"), m_RequireGetOldest );
	
	// validate the agent's default email address
	CEMSString sString(m_szDefaultEmailAddress);

	if (sString.size() > 0 && !sString.ValidateEmailAddr())
	{
		CEMSString sError;
		sError.Format( _T("The email address specified [%s] is invalid"), sString.c_str() );
		THROW_VALIDATION_EXCEPTION( _T("DefaultEmail"), sError );
	}

	for( iter = m_Data.begin(); iter != m_Data.end(); iter++ )
	{
		if( iter->m_PersonalDataTypeID == 1 )
		{
			sString.assign( iter->m_DataValue );

			if( !sString.ValidateEmailAddr() )
			{
				CEMSString sError;
				sField.Format( _T("PersonalData%d"), iter->m_PersonalDataID );
				sError.Format( _T("The email address specified [%s] is invalid"), sString.c_str() );
				THROW_VALIDATION_EXCEPTION( sField.c_str(), sError );
			}
		}
	}

}

////////////////////////////////////////////////////////////////////////////////
// 
// GenerateXML
// 
////////////////////////////////////////////////////////////////////////////////
void CAgents::GenerateXML(void)
{
	set<unsigned int>::iterator iter;
	list<TPersonalData>::iterator iter2;
	tstring sValue;
	int nGroupID;
		
	GetServerParameter( EMS_SRVPARAM_VMS_INTEGRATION, sValue );
	GetXMLGen().AddChildElem( _T("VMSIntegration") );
	GetXMLGen().AddChildAttrib( _T("Enabled"), sValue.c_str() );

	GetXMLGen().AddChildElem( _T("Agent") );
	GetXMLGen().AddChildAttrib( _T("ID"), m_AgentID );
	GetXMLGen().AddChildAttrib( _T("Name"), m_Name );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_IsEnabled );
	GetXMLGen().AddChildAttrib( _T("LoginName"), m_LoginName );
	GetXMLGen().AddChildAttrib( _T("IsNTUser"), m_IsNTUser );
	GetXMLGen().AddChildAttrib( _T("NTDomain"), m_NTDomain );
	GetXMLGen().AddChildAttrib( _T("Password"), (m_AgentID) ? _T("********") : _T("") );
	GetXMLGen().AddChildAttrib( _T("MaxReportRowsPerPage"), m_MaxReportRowsPerPage );
	GetXMLGen().AddChildAttrib( _T("StdResponseApprovalsRequired"), m_StdResponseApprovalsRequired );
	GetXMLGen().AddChildAttrib( _T("MessageApprovalsRequired"), m_MessageApprovalsRequired );
	GetXMLGen().AddChildAttrib( _T("QuoteMsgInReply"), m_QuoteMsgInReply );
	GetXMLGen().AddChildAttrib( _T("ReplyQuotedPrefix"), m_ReplyQuotedPrefix );
	GetXMLGen().AddChildAttrib( _T("UseIM"), m_UseIM );
	GetXMLGen().AddChildAttrib( _T("NewMessageFormat"), m_NewMessageFormat );
	GetXMLGen().AddChildAttrib( _T("UseAutoFill"), m_UseAutoFill );
	GetXMLGen().AddChildAttrib( _T("MaxPersonalDataID"), m_MaxPersonalDataID );
	GetXMLGen().AddChildAttrib( _T("RequireGetOldest"), m_RequireGetOldest );
	GetXMLGen().AddChildAttrib( _T("StatusID"), m_StatusID );
		
	if( m_EscalateToAgentID >= 0 )
	{
		GetXMLGen().AddChildAttrib( _T("EscalateToAgentID"), m_EscalateToAgentID );
	}
	else
	{
		m_EscalateToAgentID = abs(m_EscalateToAgentID);
		GetXMLGen().AddChildAttrib( _T("EscalateToGroupID"), m_EscalateToAgentID );
	}

	if( m_OutboundApprovalFromID >= 0 )
	{
		GetXMLGen().AddChildAttrib( _T("OutboundApprovalFromAgentID"), m_OutboundApprovalFromID );
	}
	else
	{
		m_OutboundApprovalFromID = abs(m_OutboundApprovalFromID);
		GetXMLGen().AddChildAttrib( _T("OutboundApprovalFromGroupID"), m_OutboundApprovalFromID );
	}
	
	if( m_szDefaultEmailAddressLen != SQL_NULL_DATA )
	{
		GetXMLGen().AddChildAttrib( _T("DefaultEmailAddress"), m_szDefaultEmailAddress );
	}
	else
	{
		GetXMLGen().AddChildAttrib( _T("DefaultEmailAddress"), _T("") );
	}

	GetXMLGen().IntoElem();

	
	for( iter = m_GroupIDs.begin(); iter != m_GroupIDs.end(); iter++ )
	{
		GetXMLGen().AddChildElem( _T("Group") );
		GetXMLGen().AddChildAttrib( _T("ID"), *iter );
		AddGroupName( _T("GroupName"), *iter );
	}

	for( iter2 = m_Data.begin(); iter2 != m_Data.end(); iter2++ )
	{
		GetXMLGen().AddChildElem( _T("PersonalData") );
		GetXMLGen().AddChildAttrib( _T("ID"), iter2->m_PersonalDataID );
		GetXMLGen().AddChildAttrib( _T("TypeID"), iter2->m_PersonalDataTypeID );
		GetXMLGen().AddChildAttrib( _T("Value"), iter2->m_DataValue );
		GetXMLGen().AddChildAttrib( _T("Note"), iter2->m_Note );
	}

	GetXMLGen().OutOfElem();
	
	//Get Escalation Groups
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), nGroupID );
	BINDCOL_TCHAR( GetQuery(), m_szDefaultEmailAddress );
	GetQuery().Execute( _T("SELECT GroupID,GroupName ")
						_T("FROM Groups ")
						_T("WHERE IsDeleted=0 AND IsEscalationGroup=1 ")
						_T("ORDER BY GroupName") );

	GetXMLGen().AddChildElem( _T("EscGroups") );
	GetXMLGen().IntoElem();
	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Group") );
		GetXMLGen().AddChildAttrib( _T("ID"), nGroupID );
		GetXMLGen().AddChildAttrib( _T("GroupName"), m_szDefaultEmailAddress );		
	}
	GetXMLGen().OutOfElem();

	//Get Groups with ID's
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), nGroupID );
	BINDCOL_TCHAR( GetQuery(), m_szDefaultEmailAddress );
	GetQuery().Execute( _T("SELECT GroupID,GroupName ")
						_T("FROM Groups ")
						_T("WHERE IsDeleted=0 ")
						_T("ORDER BY GroupName") );

	GetXMLGen().AddChildElem( _T("AppGroups") );
	GetXMLGen().IntoElem();
	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Group") );
		GetXMLGen().AddChildAttrib( _T("ID"), nGroupID );
		GetXMLGen().AddChildAttrib( _T("GroupName"), m_szDefaultEmailAddress );		
	}
	GetXMLGen().OutOfElem();

	ListEnabledAgentNames();
	ListGroupNames();
	ListPersonalDataTypes();
	
	
}

////////////////////////////////////////////////////////////////////////////////
// 
// AgentPreferences
// 
////////////////////////////////////////////////////////////////////////////////
int CAgents::AgentPreferences( CURLAction& action, tstring sFormAction )
{
	int groupid = 0;
	int agentid = 0;
	int sigid = 0;

	// Test for signature page
	if(_tcsicmp( GetISAPIData().m_sPage.c_str(), _T("preferencesignatures") ) == 0){

		if( sFormAction.compare( _T("makedefault") ) == 0 || sFormAction.compare( _T("delete") ) == 0)
		{
			if(m_AgentID < 0)
			{
				sigid = abs(m_AgentID);
			}
			else
			{
				sigid = m_AgentID;
			}

			// Lets query the signature id since we are changing default or deleting
			GetQuery().Initialize();
			BINDCOL_LONG_NOLEN( GetQuery(), groupid );
			BINDCOL_LONG_NOLEN( GetQuery(), agentid );
			BINDPARAM_LONG( GetQuery(), sigid );

			GetQuery().Execute("SELECT GroupID, AgentID FROM Signatures WHERE SignatureID = ?");
			
			if( GetQuery().Fetch() != S_OK )
			{
				// This should not happen but just in case
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
			}
			else
			{
				if( sFormAction.compare( _T("makedefault") ) == 0 )
				{
					// Signature assigned to group so test access rights for group
					if(groupid != 0){
						RequireAgentRightLevel
							( EMS_OBJECT_TYPE_GROUP, groupid, EMS_EDIT_ACCESS );
					} // Signature assigned to agent so test against agent
					else if(agentid !=0)
					{
						RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, agentid, EMS_EDIT_ACCESS );
					}
					else
					{
						RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_EDIT_ACCESS );
					}
				}
				else
				{
					// Signature assigned to group so test access rights for group
					if(groupid != 0)
					{
						RequireAgentRightLevel( EMS_OBJECT_TYPE_GROUP, groupid, EMS_DELETE_ACCESS );
					} // Signature assigned to agent so test against agent
					else if(agentid !=0)
					{
						RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, agentid, EMS_EDIT_ACCESS );
					}
					else
					{
						RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_EDIT_ACCESS );
					}
				}				
			}
		}
		else
		{
			RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_EDIT_ACCESS );
		}
	}
	else
	{
		// Check security
		RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_EDIT_ACCESS );
	}

	if( m_ISAPIData.GetXMLPost() )
	{
		DecodePreferencesForm();	
		UpdatePreferences();
		return 0;
	}

	QueryPreferences();
	GeneratePreferencesXML();

	CEMSString sTitle;
	sTitle.Format( _T("Preferences for %s"), m_Name );
	action.m_sPageTitle.assign( sTitle );

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// QueryPreferences
// 
////////////////////////////////////////////////////////////////////////////////
void CAgents::QueryPreferences( void )
{

	if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("preferencegeneral") ) == 0 )
	{
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), m_MaxReportRowsPerPage );
		BINDCOL_BIT( GetQuery(), m_QuoteMsgInReply );
		BINDCOL_TCHAR( GetQuery(), m_ReplyQuotedPrefix );
		BINDCOL_BIT( GetQuery(), m_UseIM );
		BINDCOL_BIT( GetQuery(), m_NewMessageFormat );
		BINDCOL_LONG( GetQuery(), m_StyleSheetID );
		BINDCOL_LONG( GetQuery(), m_DictionaryID );
		BINDCOL_BIT( GetQuery(), m_DictionaryPrompt );
		BINDCOL_BIT( GetQuery(), m_ForceSpellCheck );
		BINDCOL_BIT( GetQuery(), m_SignatureTopReply );
		BINDCOL_BIT( GetQuery(), m_UsePreviewPane );
		BINDCOL_BIT( GetQuery(), m_ShowMessagesInbound );
		BINDCOL_TINYINT( GetQuery(), m_CloseTicket );
		BINDCOL_TINYINT( GetQuery(), m_RouteToInbox );
		BINDCOL_LONG( GetQuery(), m_OutboxHoldTime );
		BINDCOL_BIT( GetQuery(), m_UseMarkAsRead );
		BINDCOL_LONG( GetQuery(), m_MarkAsReadSeconds );
		BINDCOL_BIT( GetQuery(), m_UseAutoFill );
		BINDCOL_LONG( GetQuery(), m_DefaultTicketBoxID );
		BINDCOL_LONG( GetQuery(), m_DefaultTicketDblClick );
		BINDCOL_LONG( GetQuery(), m_ReadReceipt );
		BINDCOL_LONG( GetQuery(), m_DeliveryConfirmation );
		BINDCOL_LONG( GetQuery(), m_LoginStatusID );
		BINDCOL_LONG( GetQuery(), m_LogoutStatusID );
		BINDCOL_LONG( GetQuery(), m_AutoStatusTypeID );
		BINDCOL_LONG( GetQuery(), m_AutoStatusMin );
		BINDPARAM_LONG( GetQuery(), m_AgentID );

		GetQuery().Execute( _T("SELECT MaxReportRowsPerPage,QuoteMsgInReply,ReplyQuotedPrefix,UseIM,NewMessageFormat,StyleSheetID,DictionaryID,DictionaryPrompt,")
							_T("ForceSpellCheck,SignatureTopReply,UsePreviewPane,ShowMessagesInbound,CloseTicket,")
							_T("RouteToInbox,OutboxHoldTime, UseMarkAsRead, MarkAsReadSeconds,UseAutoFill,DefaultTicketBoxID,DefaultTicketDblClick,ReadReceipt,DeliveryConfirmation,LoginStatusID,LogoutStatusID,AutoStatusTypeID,AutoStatusMin ")
							_T("FROM Agents ")
							_T("WHERE Agents.AgentID=? AND IsDeleted=0 ") );
		
		if( GetQuery().Fetch() != S_OK )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		}

		//Get default column views
		GetQuery().Initialize();
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowState);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowPriority);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowNumNotes);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowTicketID);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowNumMsgs);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowSubject);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowContact);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowDate);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowCategory);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowOwner);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowTicketBox);
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		GetQuery().Execute( _T("SELECT ShowState,ShowPriority,ShowNumNotes,ShowTicketID,ShowNumMsgs, ")
				   _T("ShowSubject,ShowContact,ShowDate,ShowCategory,ShowOwner,ShowTicketBox ")
		           _T("FROM TicketBoxViews ")
		           _T("WHERE AgentID=? AND TicketBoxViewTypeID=9"));
	
		if (GetQuery().Fetch() != S_OK)
		{
			//Set the defaults
			m_TBView.m_ShowState = 1;
			m_TBView.m_ShowPriority = 1;
			m_TBView.m_ShowNumNotes = 1;
			m_TBView.m_ShowTicketID = 1;
			m_TBView.m_ShowNumMsgs = 1;
			m_TBView.m_ShowSubject = 1;
			m_TBView.m_ShowContact = 1;
			m_TBView.m_ShowDate = 1;
			m_TBView.m_ShowCategory = 1;
			m_TBView.m_ShowOwner = 1;
			m_TBView.m_ShowTicketBox = 1;

			//Add the view
			GetQuery().Reset( true );
			BINDPARAM_LONG( GetQuery(), m_AgentID );
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowState);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowPriority);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowNumNotes);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowTicketID);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowNumMsgs);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowSubject);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowContact);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowDate);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowCategory);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowOwner);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowTicketBox);
			GetQuery().Execute( _T("INSERT INTO TicketBoxViews (AgentID,TicketBoxViewTypeID,ShowState,ShowPriority, ")
								_T("ShowNumNotes,ShowTicketID,ShowNumMsgs,ShowSubject,ShowContact, ")
								_T("ShowDate,ShowCategory,ShowOwner,ShowTicketBox) ")
								_T("VALUES (?,9,?,?,?,?,?,?,?,?,?,?,?)") );	

		}
		


	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("apreferences") ) == 0 )
	{
		GetQuery().Initialize();
		BINDCOL_TCHAR( GetQuery(), m_Name );
		BINDPARAM_LONG( GetQuery(), m_AgentID );

		GetQuery().Execute( _T("SELECT Name ")
							_T("FROM Agents ")
							_T("WHERE Agents.AgentID=? AND IsDeleted=0 ") );
		
		if( GetQuery().Fetch() != S_OK )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		}		
	}
}


void CAgents::GetAgentViews( list<int>& IDList )
{
	int AgentBoxID;

	// List Agent ticketbox views
	GetQuery().Initialize();
	
	BINDPARAM_LONG(GetQuery(), m_AgentID );
	BINDCOL_LONG_NOLEN(GetQuery(), AgentBoxID );
		
	GetQuery().Execute( _T("SELECT DISTINCT AgentBoxID,Agents.Name ")
						_T("FROM TicketBoxViews INNER JOIN Agents ON TicketBoxViews.AgentBoxID = Agents.AgentID ")
						_T("WHERE TicketBoxViews.AgentID = ? AND AgentBoxID <> 0 ")
						_T("ORDER BY Agents.Name ") );

	while( GetQuery().Fetch() == S_OK )
	{
		// Don't add the agent's own view to the list
		if( AgentBoxID != m_AgentID )
			IDList.push_back( AgentBoxID );
	}

}

void CAgents::GetTicketBoxes( list<int>& IDList )
{
	int TicketBoxID;

	// List Public ticketboxviews
	GetQuery().Initialize();

	BINDPARAM_LONG(GetQuery(), m_AgentID);
	BINDCOL_LONG_NOLEN(GetQuery(), TicketBoxID );

	GetQuery().Execute(	_T("SELECT DISTINCT TicketBoxViews.TicketBoxID, TicketBoxes.Name ")
						_T("FROM TicketBoxViews INNER JOIN TicketBoxes ON TicketBoxViews.TicketBoxID = TicketBoxes.TicketBoxID ")
						_T("WHERE TicketBoxViews.AgentID = ? AND TicketBoxViews.TicketBoxID <> 0 ") 
						_T("ORDER BY TicketBoxes.Name ") );
	
	while( GetQuery().Fetch() == S_OK )
	{
		IDList.push_back( TicketBoxID );
	}

}

////////////////////////////////////////////////////////////////////////////////
// 
// GeneratePreferencesXML
// 
////////////////////////////////////////////////////////////////////////////////
void CAgents::GeneratePreferencesXML(void)
{
	list<int>::iterator iter;

	if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("preferencegeneral") ) == 0 )
	{
		TDictionary dc;
		int nDcEnabled = 0;
		dc.PrepareList( GetQuery() );
		GetXMLGen().AddChildElem( _T("Dictionaries") );
		GetXMLGen().IntoElem();
		while( GetQuery().Fetch() == S_OK )
		{
			if(dc.m_IsEnabled){nDcEnabled++;}
			GetXMLGen().AddChildElem( _T("Dictionary") );
			GetXMLGen().AddChildAttrib( _T("ID"), dc.m_DictionaryID );
			GetXMLGen().AddChildAttrib( _T("Description"), dc.m_Description );
			GetXMLGen().AddChildAttrib( _T("TlxFile"), dc.m_TlxFile );
			GetXMLGen().AddChildAttrib( _T("ClxFile"), dc.m_ClxFile );
			GetXMLGen().AddChildAttrib( _T("IsEnabled"), dc.m_IsEnabled );
		}
		GetXMLGen().OutOfElem();
				
		GetXMLGen().AddChildElem( _T("Agent") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_AgentID );
		GetXMLGen().AddChildAttrib( _T("MaxReportRowsPerPage"), m_MaxReportRowsPerPage );
		GetXMLGen().AddChildAttrib( _T("QuoteMsgInReply"), m_QuoteMsgInReply );
		GetXMLGen().AddChildAttrib( _T("ReplyQuotedPrefix"), m_ReplyQuotedPrefix );
		GetXMLGen().AddChildAttrib( _T("StyleSheetID"), m_StyleSheetID );
		GetXMLGen().AddChildAttrib( _T("DictionaryID"), m_DictionaryID );
		GetXMLGen().AddChildAttrib( _T("DictionaryPrompt"), m_DictionaryPrompt );
		GetXMLGen().AddChildAttrib( _T("ForceSpellCheck"), m_ForceSpellCheck );
		GetXMLGen().AddChildAttrib( _T("SignatureTopReply"), m_SignatureTopReply );
		GetXMLGen().AddChildAttrib( _T("UseIM"), m_UseIM );
		GetXMLGen().AddChildAttrib( _T("NewMessageFormat"), m_NewMessageFormat );
		GetXMLGen().AddChildAttrib( _T("UseAutoFill"), m_UseAutoFill );
		GetXMLGen().AddChildAttrib( _T("UsePreviewPane"), m_UsePreviewPane );
		GetXMLGen().AddChildAttrib( _T("ShowMessages"), m_ShowMessagesInbound );
		GetXMLGen().AddChildAttrib( _T("CloseTicketAfterSend"), m_CloseTicket );
		GetXMLGen().AddChildAttrib( _T("AutoRouteToInbox"), m_RouteToInbox );
		GetXMLGen().AddChildAttrib( _T("OutboxHoldTime"), m_OutboxHoldTime );
		GetXMLGen().AddChildAttrib( _T("usemarkasread"), m_UseMarkAsRead );
		GetXMLGen().AddChildAttrib( _T("markasreadseconds"), m_MarkAsReadSeconds );
		GetXMLGen().AddChildAttrib(_T("DefaultTicketBoxID"), m_DefaultTicketBoxID);	
		GetXMLGen().AddChildAttrib(_T("TicketDblClick"), m_DefaultTicketDblClick);
		GetXMLGen().AddChildAttrib(_T("ReadReceipt"), m_ReadReceipt);	
		GetXMLGen().AddChildAttrib(_T("DeliveryConfirmation"), m_DeliveryConfirmation);	
		GetXMLGen().AddChildAttrib(_T("LoginStatusID"), m_LoginStatusID);	
		GetXMLGen().AddChildAttrib(_T("LogoutStatusID"), m_LogoutStatusID);
		GetXMLGen().AddChildAttrib(_T("AutoStatusTypeID"), m_AutoStatusTypeID);	
		GetXMLGen().AddChildAttrib(_T("AutoStatusMin"), m_AutoStatusMin);	
		GetXMLGen().AddChildAttrib( _T("ShowDictionary"), nDcEnabled);	
				
		GetXMLGen().AddChildAttrib( _T("ShowState"), m_TBView.m_ShowState);
		GetXMLGen().AddChildAttrib( _T("ShowPriority"), m_TBView.m_ShowPriority);
		GetXMLGen().AddChildAttrib( _T("ShowNumNotes"), m_TBView.m_ShowNumNotes);
		GetXMLGen().AddChildAttrib( _T("ShowTicketID"), m_TBView.m_ShowTicketID);
		GetXMLGen().AddChildAttrib( _T("ShowNumMsgs"), m_TBView.m_ShowNumMsgs);
		GetXMLGen().AddChildAttrib( _T("ShowSubject"), m_TBView.m_ShowSubject);
		GetXMLGen().AddChildAttrib( _T("ShowContact"), m_TBView.m_ShowContact);
		GetXMLGen().AddChildAttrib( _T("ShowDate"), m_TBView.m_ShowDate);
		GetXMLGen().AddChildAttrib( _T("ShowCategory"), m_TBView.m_ShowCategory);
		GetXMLGen().AddChildAttrib( _T("ShowOwner"), m_TBView.m_ShowOwner);
		GetXMLGen().AddChildAttrib( _T("ShowTicketBox"), m_TBView.m_ShowTicketBox);

		GetXMLDoc().AddElem( _T("Agent") );
		GetXMLDoc().AddAttrib( _T("ID"), m_AgentID );
		GetXMLDoc().AddAttrib( _T("MaxReportRowsPerPage"), m_MaxReportRowsPerPage );
		GetXMLDoc().AddAttrib( _T("QuoteMsgInReply"), m_QuoteMsgInReply );
		GetXMLDoc().AddAttrib( _T("ReplyQuotedPrefix"), m_ReplyQuotedPrefix );
		GetXMLDoc().AddAttrib( _T("StyleSheetID"), m_StyleSheetID );
		GetXMLDoc().AddAttrib( _T("ForceSpellCheck"), m_ForceSpellCheck );
		GetXMLDoc().AddAttrib( _T("SignatureTopReply"), m_SignatureTopReply );
		GetXMLDoc().AddAttrib( _T("UseIM"), m_UseIM );
		GetXMLDoc().AddAttrib( _T("NewMessageFormat"), m_NewMessageFormat );
		GetXMLDoc().AddAttrib( _T("UseAutoFill"), m_UseAutoFill );
		GetXMLDoc().AddAttrib( _T("RefreshLeftPane"), m_nRefreshLeftPane );
		GetXMLDoc().AddAttrib( _T("UsePreviewPane"), m_UsePreviewPane );
		GetXMLDoc().AddAttrib( _T("ShowMessages"), m_ShowMessagesInbound );
		GetXMLDoc().AddAttrib( _T("CloseTicketAfterSend"), m_CloseTicket );
		GetXMLDoc().AddAttrib( _T("AutoRouteToInbox"), m_RouteToInbox );
		GetXMLDoc().AddAttrib( _T("OutboxHoldTime"), m_OutboxHoldTime );
		GetXMLDoc().AddAttrib( _T("usemarkasread"), m_UseMarkAsRead );
		GetXMLDoc().AddAttrib( _T("markasreadseconds"), m_MarkAsReadSeconds );
		GetXMLDoc().AddAttrib(_T("DefaultTicketBoxID"), m_DefaultTicketBoxID);	
		GetXMLDoc().AddAttrib(_T("TicketDblClick"), m_DefaultTicketDblClick);
		GetXMLDoc().AddAttrib(_T("ReadReceipt"), m_ReadReceipt);	
		GetXMLDoc().AddAttrib(_T("DeliveryConfirmation"), m_DeliveryConfirmation);
		GetXMLDoc().AddAttrib( _T("ShowDictionary"), nDcEnabled);

		GetXMLDoc().AddAttrib( _T("ShowState"), m_TBView.m_ShowState);
		GetXMLDoc().AddAttrib( _T("ShowPriority"), m_TBView.m_ShowPriority);
		GetXMLDoc().AddAttrib( _T("ShowNumNotes"), m_TBView.m_ShowNumNotes);
		GetXMLDoc().AddAttrib( _T("ShowTicketID"), m_TBView.m_ShowTicketID);
		GetXMLDoc().AddAttrib( _T("ShowNumMsgs"), m_TBView.m_ShowNumMsgs);
		GetXMLDoc().AddAttrib( _T("ShowSubject"), m_TBView.m_ShowSubject);
		GetXMLDoc().AddAttrib( _T("ShowContact"), m_TBView.m_ShowContact);
		GetXMLDoc().AddAttrib( _T("ShowDate"), m_TBView.m_ShowDate);
		GetXMLDoc().AddAttrib( _T("ShowCategory"), m_TBView.m_ShowCategory);
		GetXMLDoc().AddAttrib( _T("ShowOwner"), m_TBView.m_ShowOwner);
		GetXMLDoc().AddAttrib( _T("ShowTicketBox"), m_TBView.m_ShowTicketBox);

		ListStyleSheetNames();
		ListTicketBoxNames(m_AgentID, EMS_EDIT_ACCESS, GetXMLGen() );		
	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("preferencesignatures") ) == 0 )
	{
		ListSignatures( m_AgentID, GetXMLGen(), false, false, 0 );
		ListTicketBoxNames(m_AgentID, EMS_EDIT_ACCESS, GetXMLGen() );
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
	else
	{
		GetXMLGen().AddChildElem( _T("Agent") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_AgentID );
		GetXMLGen().AddChildAttrib( _T("Name"), m_Name );
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// GeneratePreferencesXML
// 
////////////////////////////////////////////////////////////////////////////////
void CAgents::DecodePreferencesForm(void)
{
	CEMSString sCollection;
	
	if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("preferencegeneral") ) == 0 )
	{
		GetISAPIData().GetXMLLong( _T("MaxReportRowsPerPage"), m_MaxReportRowsPerPage );
		GetISAPIData().GetXMLLong( _T("QuoteMsgInReply"), m_QuoteMsgInReply );
		GetISAPIData().GetXMLLong( _T("StyleSheetID"), m_StyleSheetID );
		GetISAPIData().GetXMLTCHAR( _T("ReplyQuotedPrefix"), m_ReplyQuotedPrefix, 3 );
		GetISAPIData().GetXMLLong( _T("defaultSignatureID"), m_DefaultSignatureID, true );	
		GetISAPIData().GetXMLLong( _T("NewMessageFormat"), m_NewMessageFormat );
		GetISAPIData().GetXMLLong( _T("UseAutoFill"), m_UseAutoFill );
		GetISAPIData().GetXMLLong( _T("ForceSpellCheck"), m_ForceSpellCheck );
		GetISAPIData().GetXMLLong( _T("DictionaryID"), m_DictionaryID, true );
		GetISAPIData().GetXMLLong( _T("DictionaryPrompt"), m_DictionaryPrompt, true );
		GetISAPIData().GetXMLLong( _T("SignatureTopReply"), m_SignatureTopReply );
		GetISAPIData().GetXMLLong( _T("UsePreviewPane"), m_UsePreviewPane );
		GetISAPIData().GetXMLLong( _T("ShowMessages"), m_ShowMessagesInbound );
		GetISAPIData().GetXMLLong( _T("AutoRouteToInbox"), m_RouteToInbox );
		GetISAPIData().GetXMLLong( _T("CloseTicketAfterSend"), m_CloseTicket );
		GetISAPIData().GetXMLLong( _T("OutboxHoldTime"), m_OutboxHoldTime );
		GetISAPIData().GetXMLLong( _T("chkMarkMsgRead"), m_UseMarkAsRead );
		GetISAPIData().GetXMLLong( _T("txtMarkMsgRead"), *(int*)&m_MarkAsReadSeconds );
		GetISAPIData().GetXMLLong( _T("DefaultTicketBoxID"), m_DefaultTicketBoxID );
		GetISAPIData().GetXMLLong( _T("TicketDblClick"), m_DefaultTicketDblClick );
		GetISAPIData().GetXMLLong( _T("ReadReceipt"), m_ReadReceipt );
		GetISAPIData().GetXMLLong( _T("DeliveryConfirmation"), m_DeliveryConfirmation );

		GetISAPIData().GetXMLLong( _T("DefaultOnlineStatus"), m_LoginStatusID );
		GetISAPIData().GetXMLLong( _T("DefaultOfflineStatus"), m_LogoutStatusID );
		GetISAPIData().GetXMLLong( _T("AutoStatusTypeID"), m_AutoStatusTypeID );
		GetISAPIData().GetXMLLong( _T("AutoStatusMin"), m_AutoStatusMin );
		GetISAPIData().GetXMLTCHAR( _T("OnlineText"), m_OnlineText, AGENTS_STATUS_TEXT_LENGTH );
		GetISAPIData().GetXMLTCHAR( _T("AwayText"), m_AwayText, AGENTS_STATUS_TEXT_LENGTH );
		GetISAPIData().GetXMLTCHAR( _T("NotAvailText"), m_NotAvailText, AGENTS_STATUS_TEXT_LENGTH );
		GetISAPIData().GetXMLTCHAR( _T("OfflineText"), m_OfflineText, AGENTS_STATUS_TEXT_LENGTH );
		GetISAPIData().GetXMLTCHAR( _T("OooText"), m_OooText, AGENTS_STATUS_TEXT_LENGTH );
		
		GetISAPIData().GetXMLLong( _T("ShowState"), m_TBView.m_ShowState);
		GetISAPIData().GetXMLLong( _T("ShowPriority"), m_TBView.m_ShowPriority);
		GetISAPIData().GetXMLLong( _T("ShowNumNotes"), m_TBView.m_ShowNumNotes);
		GetISAPIData().GetXMLLong( _T("ShowTicketID"), m_TBView.m_ShowTicketID);
		GetISAPIData().GetXMLLong( _T("ShowNumMsgs"), m_TBView.m_ShowNumMsgs);
		GetISAPIData().GetXMLLong( _T("ShowSubject"), m_TBView.m_ShowSubject);
		GetISAPIData().GetXMLLong( _T("ShowContact"), m_TBView.m_ShowContact);
		GetISAPIData().GetXMLLong( _T("ShowDate"), m_TBView.m_ShowDate);
		GetISAPIData().GetXMLLong( _T("ShowCategory"), m_TBView.m_ShowCategory);
		GetISAPIData().GetXMLLong( _T("ShowOwner"), m_TBView.m_ShowOwner);
		GetISAPIData().GetXMLLong( _T("ShowTicketBox"), m_TBView.m_ShowTicketBox);
	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("preferencesignatures") ) == 0 )
	{
		tstring strAction;
		m_nSignatureID = 0;

		if( GetISAPIData().GetXMLString( _T("Action"), strAction ) ) 
		{
			GetISAPIData().GetURLLong( _T("SelectID"), m_AgentID );
			GetISAPIData().GetXMLLong( _T("SelectID"), m_nSignatureID );
		}	
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// UpdatePreferences
// 
////////////////////////////////////////////////////////////////////////////////
void CAgents::UpdatePreferences(void)
{
	int nStyleSheetID;


	if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("preferencegeneral") ) == 0 )
	{	
		GetQuery().Initialize();
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowState );
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowPriority );
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowNumNotes );
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowTicketID );
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowNumMsgs );
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowSubject );
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowContact );
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowDate );
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowCategory );
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowOwner );
		BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowTicketBox );
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		GetQuery().Execute( _T("UPDATE TicketBoxViews ")
							_T("SET ShowState=?,ShowPriority=?,ShowNumNotes=?,ShowTicketID=?,ShowNumMsgs=?,ShowSubject=?,ShowContact=?,ShowDate=?,ShowCategory=?,ShowOwner=?,ShowTicketBox=? ")
							_T("WHERE AgentID=? AND TicketBoxViewTypeID=9") );		

		if( GetQuery().GetRowCount() == 0 )
		{
			// bail out if m_TicketBoxViewID was invalid
			THROW_EMS_EXCEPTION( E_InvalidID,  _T("Invalid TicketBoxView ID") ); 
		}
		
		GetQuery().Initialize();		
		BINDCOL_LONG_NOLEN( GetQuery(), nStyleSheetID );
		BINDPARAM_LONG( GetQuery(), m_AgentID );
		GetQuery().Execute( _T("SELECT StyleSheetID FROM Agents WHERE AgentID=?") );
		if( GetQuery().Fetch() != S_OK )
		{
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		}

		if( (nStyleSheetID != m_StyleSheetID) && ( m_AgentID == GetAgentID() ) )
			GetXMLGen().AddChildElem( _T("Refresh") );
		
		GetQuery().Initialize();		
		BINDPARAM_LONG( GetQuery(), m_MaxReportRowsPerPage );
		BINDPARAM_BIT( GetQuery(), m_QuoteMsgInReply );
		BINDPARAM_LONG( GetQuery(), m_StyleSheetID );
		BINDPARAM_TCHAR( GetQuery(), m_ReplyQuotedPrefix );
		BINDPARAM_BIT( GetQuery(), m_NewMessageFormat );
		BINDPARAM_BIT( GetQuery(), m_ForceSpellCheck );
		BINDPARAM_LONG( GetQuery(), m_DictionaryID );
		BINDPARAM_BIT( GetQuery(), m_DictionaryPrompt );
		BINDPARAM_BIT( GetQuery(), m_SignatureTopReply );
		BINDPARAM_BIT( GetQuery(), m_UsePreviewPane );
		BINDPARAM_BIT( GetQuery(), m_ShowMessagesInbound );
		BINDPARAM_TINYINT( GetQuery(), m_CloseTicket );
		BINDPARAM_TINYINT( GetQuery(), m_RouteToInbox );
		BINDPARAM_LONG( GetQuery(), m_OutboxHoldTime );
		BINDPARAM_BIT( GetQuery(), m_UseMarkAsRead );
		BINDPARAM_LONG( GetQuery(), m_MarkAsReadSeconds );
		BINDPARAM_BIT( GetQuery(), m_UseAutoFill );
		BINDPARAM_LONG( GetQuery(), m_DefaultTicketBoxID );
		BINDPARAM_LONG( GetQuery(), m_DefaultTicketDblClick );
		BINDPARAM_LONG( GetQuery(), m_ReadReceipt );
		BINDPARAM_LONG( GetQuery(), m_DeliveryConfirmation );
		BINDPARAM_LONG( GetQuery(), m_AutoStatusTypeID );
		BINDPARAM_LONG( GetQuery(), m_AutoStatusMin );
		BINDPARAM_LONG( GetQuery(), m_LoginStatusID );
		BINDPARAM_LONG( GetQuery(), m_LogoutStatusID );
		BINDPARAM_TCHAR( GetQuery(), m_OnlineText );	
		BINDPARAM_TCHAR( GetQuery(), m_AwayText );	
		BINDPARAM_TCHAR( GetQuery(), m_NotAvailText );	
		BINDPARAM_TCHAR( GetQuery(), m_OfflineText );	
		BINDPARAM_TCHAR( GetQuery(), m_OooText );
		BINDPARAM_LONG( GetQuery(), m_AgentID );

		GetQuery().Execute( _T("UPDATE Agents ")
							_T("SET MaxReportRowsPerPage=?,QuoteMsgInReply=?,StyleSheetID=?,ReplyQuotedPrefix=?,NewMessageFormat=?,")
							_T("ForceSpellCheck=?,DictionaryID=?,DictionaryPrompt=?,SignatureTopReply=?,UsePreviewPane=?,ShowMessagesInbound=?,")
							_T("CloseTicket=?,RouteToInbox=?,OutboxHoldTime=?,UseMarkAsRead=?,MarkAsReadSeconds=?,UseAutoFill=?,DefaultTicketBoxID=?,DefaultTicketDblClick=?,ReadReceipt=?,DeliveryConfirmation=?, ")
							_T("AutoStatusTypeID=?,AutoStatusMin=?,LoginStatusID=?,LogoutStatusID=?,OnlineText=?,AwayText=?,NotAvailText=?,OfflineText=?,OooText=? ")
							_T("WHERE AgentID=?") );
		
		if( GetQuery().GetRowCount() == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		
		GetISAPIData().m_SessionMap.InvalidateAgentSession( m_AgentID );
		GetISAPIData().m_SessionMap.QueueSessionMonitors(1,m_AgentID);
		
		if( m_AgentID == GetAgentID() )
			GetISAPIData().m_SessionMap.RefreshAgentSession( GetSession(), GetQuery() );
	}
	else if ( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("preferencesignatures") ) == 0 )
	{
		tstring strAction;
		GetISAPIData().GetXMLString( _T("Action"), strAction );
		
		if ( strAction.compare( _T("makedefault") ) == 0 )
		{

			int nDefaultSignatureID;

			// check security
			RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, m_AgentID, EMS_EDIT_ACCESS );

			// first query the current default signature ID for the agent
			GetQuery().Initialize();

			BINDCOL_LONG_NOLEN( GetQuery(), nDefaultSignatureID );
			BINDPARAM_LONG( GetQuery(), m_AgentID );

			GetQuery().Execute( _T("SELECT DefaultSignatureID FROM Agents WHERE AgentID=?") );
			
			if ( GetQuery().Fetch() != S_OK )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
			
			// if the selected signature is already the default
			if ( nDefaultSignatureID == m_nSignatureID )
				m_nSignatureID = 0;

			GetQuery().Initialize();		
		
			BINDPARAM_LONG( GetQuery(), m_nSignatureID );
			BINDPARAM_LONG( GetQuery(), m_AgentID );

			GetQuery().Execute( _T("UPDATE Agents SET DefaultSignatureID=? WHERE AgentID=?") );
			
			if( GetQuery().GetRowCount() == 0 )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

			InvalidateSignatures( true );
		}
		else if ( strAction.compare( _T("delete") ) == 0 )
		{
			XSignatures sig;

			if( m_nSignatureID < 1 )
				return;
			
			// check permissions
			if( GetXMLCache().m_Signatures.Query( m_nSignatureID, sig ) )
			{
				// MJM - edit access is required per DevTrack item 681
				RequireAgentRightLevel( sig.m_AgentID ? EMS_OBJECT_TYPE_AGENT : EMS_OBJECT_TYPE_GROUP, 
										sig.m_AgentID, EMS_EDIT_ACCESS );
			}
			else
			{
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			}

			TSignatures sig2;
			sig2.m_SignatureID = m_nSignatureID;

			if( sig2.Delete( GetQuery() ) == 0 )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		
			InvalidateSignatures( true );
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the DefaultEmailAddressID for the contact	              
\*--------------------------------------------------------------------------*/
int CAgents::GetDefaultEmailAddress(void)
{
	int nDefaultEmailAddressID;

	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), nDefaultEmailAddressID );
	BINDPARAM_LONG( GetQuery(), m_AgentID );

	GetQuery().Execute( _T("SELECT DefaultAgentAddressID ")
		                _T("FROM Agents ")
		                _T("WHERE AgentID = ?") );

	if (GetQuery().Fetch() != S_OK)
	{
		CEMSString sError;
		sError.Format( _T("Error - Invalid AgentID %d\n"), m_AgentID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	return nDefaultEmailAddressID;
}

////////////////////////////////////////////////////////////////////////////////
// 
// CheckLicense
// 
////////////////////////////////////////////////////////////////////////////////
void CAgents::CheckLicense(void)
{
	long nAgents = 0;
	long nMaxAgents = 0;
	
#if defined(HOSTED_VERSION)
	nAgents = GetISAPIData().m_LicenseMgr.GetNumAgents(GetQuery());
	nMaxAgents = GetISAPIData().m_LicenseMgr.GetMaxAgents(GetQuery());	
#else
	GetISAPIData().m_LicenseMgr.GetKeyInfo( GetQuery() );
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), nAgents );
	GetQuery().Execute( _T("SELECT COUNT(*) FROM Agents WHERE IsEnabled=1 AND IsDeleted=0 AND AgentID<>1") );
	GetQuery().Fetch();
	nMaxAgents = GetISAPIData().m_LicenseMgr.m_MFKeyAgentCount;
#endif

	if( nAgents >= nMaxAgents )
	{	
		THROW_EMS_EXCEPTION( E_SystemError, _T("Registration does not allow more agents to be added or enabled.") );
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// EncryptPassword
// 
////////////////////////////////////////////////////////////////////////////////
void CAgents::EncryptPassword()
{
	CEMSString sPassword;

	sPassword.assign( m_Password );
	sPassword.Encrypt();
	strncpy( m_Password, sPassword.c_str(), AGENTS_PASSWORD_LENGTH-1 );	
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecryptPassword
// 
////////////////////////////////////////////////////////////////////////////////
void CAgents::DecryptPassword()
{
	CEMSString sPassword;

	sPassword.assign( m_Password );
	sPassword.Decrypt();
	strncpy( m_Password, sPassword.c_str(), AGENTS_PASSWORD_LENGTH-1 );
}

////////////////////////////////////////////////////////////////////////////////
// 
// SetStatus
// 
////////////////////////////////////////////////////////////////////////////////
int CAgents::SetStatus(int StatusID, tstring StatusText)
{
	if(m_AgentID == 0)
		m_AgentID = GetAgentID();

	int nSaveDefault = 0;
	GetISAPIData().GetXMLLong( _T("savedefault"), nSaveDefault, true);
	
	if(StatusID < 0 || StatusID > 6)
	{
		DebugReporter::Instance().DisplayMessage("CAgents::SetStatus - StatusID < 0 or > 6", DebugReporter::ISAPI, GetCurrentThreadId());	
	}
	
	if(nSaveDefault)
	{
		//Save the status text as the default for the statusid
		GetQuery().Initialize();
		BINDPARAM_TCHAR_STRING( GetQuery(), StatusText );
		BINDPARAM_LONG( GetQuery(), m_AgentID );

		switch( StatusID )
		{
		case 1:
			GetQuery().Execute(_T("UPDATE Agents SET OnlineText=? WHERE AgentID=?"));
			break;
		case 2:
			GetQuery().Execute(_T("UPDATE Agents SET AwayText=? WHERE AgentID=?"));
			break;
		case 3:
			GetQuery().Execute(_T("UPDATE Agents SET NotAvailText=? WHERE AgentID=?"));
			break;
		case 5:
			GetQuery().Execute(_T("UPDATE Agents SET OfflineText=? WHERE AgentID=?"));
			break;
		case 6:
			GetQuery().Execute(_T("UPDATE Agents SET OooText=? WHERE AgentID=?"));
			break;
		}
	}
	
	GetISAPIData().m_SessionMap.SetAgentStatus(m_AgentID,StatusID,StatusText);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// LogoutAgent
// 
////////////////////////////////////////////////////////////////////////////////
int CAgents::LogoutAgent(int nAgentID)
{
	tstring sValue;
	
	if(m_AgentID == 0)
		m_AgentID = GetAgentID();

	GetServerParameter(EMS_SRVPARAM_ALLOW_AGENT_LOGOFF, sValue, "0");

	if(_ttoi(sValue.c_str()) == 0)
	{
		THROW_EMS_EXCEPTION_NOLOG( E_AccessDenied, _T("Logout option has been disabled by the Administrator!") );
	}

	RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, nAgentID, EMS_DELETE_ACCESS );

	int nDoLogout = 0;
	GetISAPIData().GetXMLLong( _T("chkLogout"), nDoLogout, true);
	
	if(nDoLogout == 1)
	{
		if(GetISAPIData().m_SessionMap.RemoveAllSessionsForAgent(nAgentID))
		{
			XAgentNames name;
			XAgentNames name2;
			GetXMLCache().m_AgentNames.Query( nAgentID, name );
			GetXMLCache().m_AgentNames.Query( m_AgentID, name2 );

			GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
												EMSERR_ISAPI_EXTENSION,
												EMS_LOG_AUTHENTICATION,
												E_Logout),
										"%s was logged out of all sessions by %s", 
										name.m_Name,name2.m_Name );
		}
	}
	else
	{
		int nStatusID = 0;
		GetISAPIData().GetXMLLong( _T("statusid"), nStatusID, true);
		if(nStatusID > 0 && nStatusID < 7)
		{
			tstring sStatusText;
			int nLoggedIn = 0;
			int nStateID = m_ISAPIData.m_SessionMap.GetAgentStatus(nAgentID,sStatusText,nLoggedIn);
			if(nStatusID != nStateID)
			{
				TAgents agent;
				GetQuery().Initialize();
				BINDCOL_TCHAR_NOLEN( GetQuery(), agent.m_StatusText );
				BINDPARAM_LONG( GetQuery(), nAgentID );

				switch( nStatusID )
				{
				case 1:
					GetQuery().Execute(_T("SELECT OnlineText FROM Agents WHERE AgentID=?"));
					break;
				case 2:
					GetQuery().Execute(_T("SELECT AwayText FROM Agents WHERE AgentID=?"));
					break;
				case 3:
					GetQuery().Execute(_T("SELECT NotAvailText FROM Agents WHERE AgentID=?"));
					break;
				case 5:
					GetQuery().Execute(_T("SELECT OfflineText FROM Agents WHERE AgentID=?"));
					break;
				case 6:
					GetQuery().Execute(_T("SELECT OooText FROM Agents WHERE AgentID=?"));
					break;
				}
				GetQuery().Fetch();
				m_ISAPIData.m_SessionMap.SetAgentStatus(nAgentID,nStatusID,agent.m_StatusText,true);
			}
		}
	}
	
	return 0;
}
