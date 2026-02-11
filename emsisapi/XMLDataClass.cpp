/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/XMLDataClass.cpp,v 1.2.2.3 2006/02/23 20:37:12 markm Exp $
||
||
||                                         
||  COMMENTS:	Base class for all XML generation objects   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "XMLDataClass.h"
#include "RegistryFns.h"
#include "MessageIO.h"
#include "KAVScan.h"
#include "AttachFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CXMLDataClass::CXMLDataClass(CISAPIData& ISAPIData) : m_ISAPIData(ISAPIData)
{	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the access level that the agent has for 
||				the object specified...	              
\*--------------------------------------------------------------------------*/
unsigned char CXMLDataClass::GetAgentRightLevel( int nObjectTypeID, int nActualID )
{
	// admins have full control
	if ( GetIsAdmin() ) return EMS_DELETE_ACCESS;

	// if the agent is not an admin look up the access level
	unsigned char RightLevel;
	RightLevel = CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), GetAgentID(), nObjectTypeID, nActualID );
	
	return RightLevel;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Requires that the agent has the access level specified	              
\*--------------------------------------------------------------------------*/
unsigned char CXMLDataClass::RequireAgentRightLevel( int nObjectTypeID, int nActualID, unsigned char RightLevelMin )
{
	unsigned char access = EMS_NO_ACCESS;
	
	if ( (access = GetAgentRightLevel( nObjectTypeID, nActualID )) < RightLevelMin )
	{
		LogSecurityViolation( nObjectTypeID, nActualID, RightLevelMin );
	}
	
	return access;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Logs any security violation	              
\*--------------------------------------------------------------------------*/
void CXMLDataClass::LogSecurityViolation( int nObjectTypeID, int nActualID, unsigned char RightRequested )
{
	CEMSString sRightRequested;
	CEMSString sObject;
	CEMSString sExObject;
	
	switch (RightRequested)
	{
	case EMS_ENUM_ACCESS:
		sRightRequested.LoadString(EMS_STRING_ENUM_ACCESS);
		break;
	case EMS_READ_ACCESS:
		sRightRequested.LoadString(EMS_STRING_READ_ACCESS);
		break;
	case EMS_EDIT_ACCESS:
		sRightRequested.LoadString(EMS_STRING_EDIT_ACCESS);
		break;
	case EMS_DELETE_ACCESS:
		sRightRequested.LoadString(EMS_STRING_DELETE_ACCESS);
		break;
	}
	
	XObjectTypes objecttype;
	GetXMLCache().m_ObjectTypes.Query( nObjectTypeID, objecttype );
	sObject.assign(objecttype.m_Description);
	sExObject.assign(objecttype.m_Description);
	
	XAgentNames agentname;
	GetXMLCache().m_AgentNames.Query( GetAgentID(), agentname );
	
	if (nObjectTypeID == EMS_OBJECT_TYPE_TICKET_BOX)
	{
		XTicketBoxNames name;
		GetXMLCache().m_TicketBoxNames.Query( nActualID, name );
		
		sObject += _T(" [");
		sObject += name.m_Name;
		sObject += _T("]");
	}
	else if (nObjectTypeID == EMS_OBJECT_TYPE_AGENT)
	{
		XAgentNames name;
		GetXMLCache().m_AgentNames.Query( nActualID, name );
		
		sObject += _T(" [");
		sObject += name.m_Name;
		sObject += _T("]");
	}
	
	// log the security violation
	GetRoutingEngine().LogIt( EMS_ISAPI_LOG_WARNING( EMS_LOG_SECURITY_VIOLATION, E_AccessDenied),
		EMS_ISAPI_ERROR_SECURITY_VIOLATION, agentname.m_Name, sRightRequested.c_str(), 
		sObject.c_str() );

	CEMSString sError;
	sError.Format( EMS_STRING_ERROR_ACCESS_DENIED, sExObject.c_str(), sRightRequested.c_str() );
	THROW_EMS_EXCEPTION_NOLOG( E_AccessDenied, sError );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Require Administrative Access	              
\*--------------------------------------------------------------------------*/
void CXMLDataClass::RequireAdmin(void)
{
	if( GetSession().m_IsAdmin == false )
	{
		GetRoutingEngine().LogIt(	EMS_ISAPI_LOG_ERROR( EMS_LOG_SECURITY_VIOLATION, E_AccessDenied),
			EMS_ISAPI_ERROR_SECURITY_VIOLATION_ADMIN_ONLY,
			GetSession().m_AgentUserName.c_str(),
			GetISAPIData().m_sPage.c_str() );
			
		THROW_EMS_EXCEPTION_NOLOG( E_AccessDenied, CEMSString( EMS_STRING_ERROR_ACCESS_DENIED_ADMIN ) ); 
	}
}

int CXMLDataClass::ListAgentNames(void)
{
	tstring xml;
	int nNames = GetXMLCache().m_AgentNames.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
	return nNames;
}

int CXMLDataClass::ListAgentNamesWOAdmin(void)
{
	tstring xml;
	int nNames = GetXMLCache().m_AgentNamesWOAdmin.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
	return nNames;
}

int CXMLDataClass::ListAgentNames(unsigned char RightLevelMin, int nIncludeID /*=0*/)
{
	if (GetIsAdmin())
		return ListAgentNames();
	
	return CAccessControl::GetInstance().ListAgents( GetQuery(), GetXMLGen(), GetXMLCache(), 
													 GetAgentID(), RightLevelMin, nIncludeID );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add a list of enabled agents to the XML	              
\*--------------------------------------------------------------------------*/
int CXMLDataClass::ListEnabledAgentNames(unsigned char RightLevelMin /*=EMS_NO_ACCESS*/, int nIncludeID /*=0*/)
{	
	int nAgents = 0;
	bool bAddedIncludedAgent = !nIncludeID;
	list<unsigned int>::iterator iter;
	XAgentNames AgentName;

	GetXMLGen().AddChildElem( _T("AgentNames") );
	GetXMLGen().IntoElem();

	// loop through our list of enabled agents and add the ones
	// the we have access rights to...
	for( iter = GetXMLCache().m_EnabledAgentIDs.GetList().begin(); 
	     iter != GetXMLCache().m_EnabledAgentIDs.GetList().end(); iter++ )
	{
		if ( GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, *iter ) >= RightLevelMin || *iter == nIncludeID )
		{
			if ( GetXMLCache().m_AgentNames.Query( *iter, AgentName ) )
			{
				GetXMLGen().AddChildElem( _T("Agent") );
				GetXMLGen().AddChildAttrib( _T("ID"), *iter );
				GetXMLGen().AddChildAttrib( _T("Name"), AgentName.m_Name );
			}

			if ( *iter == nIncludeID )
				bAddedIncludedAgent = true;

			nAgents++;
		}
	}
	
	// if we have not done so already, add the agent that we always need to include
	if ( !bAddedIncludedAgent && GetXMLCache().m_AgentNames.Query( nIncludeID, AgentName ) )
	{
		GetXMLGen().AddChildElem( _T("Agent") );
		GetXMLGen().AddChildAttrib( _T("ID"), nIncludeID );
		GetXMLGen().AddChildAttrib( _T("Name"), AgentName.m_Name );
		nAgents++;
	}

	GetXMLGen().OutOfElem();

	return nAgents;
}

int CXMLDataClass::ListGroupNames(void)
{
	tstring xml;
	int nGroups = GetXMLCache().m_GroupNames.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
	return nGroups;
}

int CXMLDataClass::ListGroupNamesWOAdmin(void)
{
	tstring xml;
	int nGroups = GetXMLCache().m_GroupNamesWOAdmin.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
	return nGroups;
}

int CXMLDataClass::ListTicketBoxNames( CMarkupSTL& xmlobj )
{
	tstring xml;
	int nTicketBoxes = GetXMLCache().m_TicketBoxNames.GetXML( xml );
	xmlobj.AddChildSubDoc( xml.c_str() );
	return nTicketBoxes;
}

int CXMLDataClass::ListTicketBoxNames(unsigned char RightLevelMin, CMarkupSTL& xmlobj, int nIncludeID /*=-1*/  )
{
	if (GetIsAdmin())
		return ListTicketBoxNames( xmlobj );

	return CAccessControl::GetInstance().ListTicketBoxes( GetQuery(), xmlobj, GetXMLCache(), GetAgentID(), RightLevelMin, nIncludeID );
}

int CXMLDataClass::ListTicketBoxNames(int nAgentID, unsigned char RightLevelMin, CMarkupSTL& xmlobj, int nIncludeID /*=-1*/  )
{
	if (GetIsAdmin())
		return ListTicketBoxNames( xmlobj );

	return CAccessControl::GetInstance().ListTicketBoxes( GetQuery(), xmlobj, GetXMLCache(), nAgentID, RightLevelMin, nIncludeID );
}

int CXMLDataClass::ListTicketCategoryNames( CMarkupSTL& xmlobj )
{
	tstring xml;
	int nTicketCategories = GetXMLCache().m_TicketCategoryNames.GetXML( xml );
	xmlobj.AddChildSubDoc( xml.c_str() );
	return nTicketCategories;
}


int CXMLDataClass::ListTicketCategoryNames(unsigned char RightLevelMin, CMarkupSTL& xmlobj, int nIncludeID /*=-1*/  )
{
	if (GetIsAdmin())
		return ListTicketCategoryNames( xmlobj );

	return CAccessControl::GetInstance().ListTicketCategories( GetQuery(), xmlobj, GetXMLCache(), 
		                                                  GetAgentID(), RightLevelMin, nIncludeID );
}

int  CXMLDataClass::ListTicketBoxDefaultEmailAddressName( CMarkupSTL& xmlobj )
{
	tstring xml;
	int nTicketBoxes = GetXMLCache().m_DefaultEmailAddressName.GetXML( xml );
	xmlobj.AddChildSubDoc( xml.c_str() );
	return nTicketBoxes;
}

int  CXMLDataClass::ListTicketBoxDefaultEmailAddressName(unsigned char RightLevelMin, CMarkupSTL& xmlobj, int nIncludeID /*-1*/ )
{
	if (GetIsAdmin())
		return ListTicketBoxDefaultEmailAddressName( xmlobj );

	return CAccessControl::GetInstance().ListTicketBoxesDefaultNames( GetQuery(), xmlobj, GetXMLCache(), 
		                                                  GetAgentID(), RightLevelMin, nIncludeID );
}


int CXMLDataClass::ListTicketBoxAddrs( CMarkupSTL& xmlobj  )
{
	tstring xml;
	int nTicketAddrs = GetXMLCache().m_TicketBoxAddrs.GetXML( xml );
	xmlobj.AddChildSubDoc( xml.c_str() );
	return nTicketAddrs;
}

void CXMLDataClass::ListMessageSourceTypeMatch(void)
{
	tstring xml;
	GetXMLCache().m_MessageSourceTypeMatch.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListMessageSourceTypes(void)
{
	tstring xml;
	GetXMLCache().m_MessageSourceTypes.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListTicketPriorities(void)
{
	tstring xml;
	GetXMLGen().AddChildSubDoc( _T("<TicketPriorities>")
								_T("<Priority ID=\"1\" Description=\"High\" />") 
		                        _T("<Priority ID=\"3\" Description=\"Normal\" />")
								_T("<Priority ID=\"5\" Description=\"Low\" />")
								_T("</TicketPriorities>") );
}

void CXMLDataClass::ListDeleteOptions(void)
{
	tstring xml;
	GetXMLCache().m_DeleteOptions.GetXML( xml ) ;
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListMatchLocations(void)
{
	tstring xml;
	GetXMLCache().m_MatchLocations.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListTicketStates(void)
{
	tstring xml;
	GetXMLCache().m_TicketStates.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListTicketCategories(void)
{
	tstring xml;
	GetXMLCache().m_TicketCategories.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListTicketBoxOwners(void)
{
	tstring xml;
	GetXMLCache().m_TicketBoxOwners.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListLogSeverities(void)
{
	tstring xml;
	GetXMLCache().m_LogSeverity.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListLogEntryTypes(void)
{
	tstring xml;
	GetXMLCache().m_LogEntryTypes.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListSignatures( int AgentID,
								    CMarkupSTL& xmlobj, /*=GetXMLGen()*/
									bool bEscape, /*= false*/
									bool bIncludeBody, /*true*/
									int nDefault /*= 0*/ )
{
	list<unsigned int>::iterator iter;	
	list<unsigned int> IDList;	
	tstring xml;
	unsigned int DefaultID;
	unsigned int nMutexWait = GetISAPIData().m_SessionMap.GetMutexWait();
	GetXMLCache().m_AgentSignatureIDs.GetSignatureIDList( AgentID, IDList, DefaultID, nMutexWait );

	if ( nDefault > 0 )
		DefaultID = nDefault;

	xmlobj.AddChildElem( _T("Signatures") );
	xmlobj.IntoElem();

	for( iter = IDList.begin(); iter != IDList.end(); iter++ )
	{
		XSignatures sig;
		
		if( GetXMLCache().m_Signatures.Query( *iter, sig) )
		{
			xmlobj.AddChildElem( _T("Signature") );
			xmlobj.AddChildAttrib( _T("ID"), sig.m_SignatureID );

			if ( bEscape )
			{
				CEMSString sTemp( sig.m_Name );
				sTemp.EscapeHTML();
				sTemp.EscapeJavascript();
				xmlobj.AddChildAttrib( _T("Name"), sTemp.c_str() );
			}
			else
			{
				xmlobj.AddChildAttrib( _T("Name"), sig.m_Name );
			}

			xmlobj.AddChildAttrib( _T("IsDefault"), (DefaultID == sig.m_SignatureID) ? 1 : 0 );
			xmlobj.AddChildAttrib( _T("IsGroup"), (sig.m_GroupID != 0) ? 1 : 0 );
			xmlobj.AddChildAttrib( _T("ObjectTypeID"), (sig.m_ObjectTypeID ));
			xmlobj.AddChildAttrib( _T("ActualID"), (sig.m_ActualID ));
			
			if ( bIncludeBody )
			{
				CEMSString sTemp(sig.m_Signature);
				if ( bEscape )
				{
					sTemp.EscapeHTML();
					sTemp.EscapeJavascript();
				}
				xmlobj.SetChildData( sTemp.c_str(), 1 );
			}
		}
	}
	
	xmlobj.OutOfElem();
}

void CXMLDataClass::ListAgentEmails( int AgentID, CMarkupSTL& xmlobj  )
{
	tstring xml;
	GetXMLCache().m_AgentEmails.GetXML( xml, AgentID );
	xmlobj.AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListPersonalDataTypes(void)
{
	tstring xml;
	GetXMLCache().m_PersonalDataTypes.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListAutoActionTypes(void)
{
	tstring xml;
	GetXMLCache().m_AutoActionTypes.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListAutoActionEvents(void)
{
	tstring xml;
	GetXMLCache().m_AutoActionEvents.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListObjectTypes(void)
{
	tstring xml;
	GetXMLCache().m_ObjectTypes.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListAlertMethods(void)
{
	tstring xml;
	GetXMLCache().m_AlertMethods.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListAlertEvents(void)
{
	tstring xml;
	GetXMLCache().m_AlertEvents.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListMessageDestinations(void)
{
	tstring xml;
	GetXMLCache().m_MsgDestNames.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListMessageSources(void)
{
	tstring xml;
	GetXMLCache().m_MsgSourceNames.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListRoutingRules(void)
{
	tstring xml;
	GetXMLCache().m_RoutingRuleNames.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

int CXMLDataClass::ListStdRespCategories(void)
{
	tstring xml;
	GetXMLCache().m_StdRespCategories.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
	return 1;
}

int CXMLDataClass::ListStdRespCategories(int nAgentID, unsigned char RightLevelMin, CMarkupSTL& xmlobj, int nIncludeID /*=-1*/  )
{
	if (GetIsAdmin())
		return ListStdRespCategories();

	return CAccessControl::GetInstance().ListSRCategories( GetQuery(), xmlobj, GetXMLCache(), nAgentID, RightLevelMin, nIncludeID );
}

void CXMLDataClass::ListStyleSheetNames(void)
{
	tstring xml;
	GetXMLCache().m_StyleSheets.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListTicketBoxHeaders(void)
{
	tstring xml;
	GetXMLCache().m_TicketBoxHeaders.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListTicketBoxFooters(void)
{
	tstring xml;
	GetXMLCache().m_TicketBoxFooters.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}

void CXMLDataClass::ListTrackingBypass(void)
{
	tstring xml;
	GetXMLCache().m_TrackingBypass.GetXML( xml );
	GetXMLGen().AddChildSubDoc( xml.c_str() );
}	

void CXMLDataClass::AddTicketBoxName( TCHAR* szChildAttrib, int TicketBoxID )
{
	XTicketBoxNames ticketboxname;
	
	if( GetXMLCache().m_TicketBoxNames.Query( TicketBoxID, ticketboxname ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, ticketboxname.m_Name );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unknown") );
	}
}

void CXMLDataClass::AddTicketBoxAddr( TCHAR* szChildAttrib, int TicketBoxID )
{
	XTicketBoxAddrs ticketboxaddr;
	
	if( GetXMLCache().m_TicketBoxAddrs.Query( TicketBoxID, ticketboxaddr ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, ticketboxaddr.m_Address );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("") );
	}
}

void CXMLDataClass::AddDeleteOption( TCHAR* szChildAttrib, unsigned char DeleteImmediately )
{
	XDeleteOptions delopts;
	
	if( GetXMLCache().m_DeleteOptions.Query( DeleteImmediately, delopts ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, delopts.m_Description );
	}
	else
	{
		CEMSString s;
		s.LoadString( EMS_STRING_DO_NOT_DELETE );
		GetXMLGen().AddChildAttrib( szChildAttrib, s.c_str() );
	}
}

void CXMLDataClass::AddPriority( TCHAR* szChildAttrib, int PriorityID )
{
	XPriorities priority;
	
	if( GetXMLCache().m_TicketPriorities.Query( PriorityID, priority ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, priority.m_Name );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unknown") );
	}
}

void CXMLDataClass::AddMessageSourceTypeMatch( TCHAR* szChildAttrib, int MessageSourceTypeID )
{
	XMessageSourceTypeMatch msgsrctype;
	
	if( GetXMLCache().m_MessageSourceTypeMatch.Query( MessageSourceTypeID, msgsrctype ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, msgsrctype.m_RemoteAddress );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("ANY") );
	}
}

void CXMLDataClass::AddMessageSourceType( TCHAR* szChildAttrib, int MessageSourceTypeID )
{
	XMessageSourceTypes msgsrctype;
	
	if( GetXMLCache().m_MessageSourceTypes.Query( MessageSourceTypeID, msgsrctype ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, msgsrctype.m_Description );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unknown") );
	}
}

void CXMLDataClass::AddOutboundMsgStateName( TCHAR* szChildAttrib, int OutBoundMsgStateID )
{
	XOutboundMsgStates msg_state;
	
	if( GetXMLCache().m_OutboundMsgStates.Query( OutBoundMsgStateID, msg_state ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, msg_state.m_MessageStateName );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unknown") );
	}
}

void CXMLDataClass::AddTicketStateName( TCHAR* szChildAttrib, int TicketStateID )
{
	XTicketStates tktstate;
	
	if( GetXMLCache().m_TicketStates.Query( TicketStateID, tktstate ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, tktstate.m_Description );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unknown") );
	}
}

void CXMLDataClass::AddTicketStateHtmlColor( TCHAR* szChildAttrib, int TicketStateID )
{
	XTicketStates tktstate;
	
	if( GetXMLCache().m_TicketStates.Query( TicketStateID, tktstate ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, tktstate.m_HTMLColorName );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unknown") );
	}
}

void CXMLDataClass::AddTicketCategoryName( TCHAR* szChildAttrib, int TicketCategoryID )
{
	XTicketCategories tktcategory;
	
	if( GetXMLCache().m_TicketCategories.Query( TicketCategoryID, tktcategory ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, tktcategory.m_Description );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unassigned") );
	}
}

void CXMLDataClass::AddAgentName( TCHAR* szChildAttrib, int AgentID, LPCTSTR szDefault )
{
	XAgentNames agentname;
	
	if( AgentID == 0 )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, "-- NONE --" );
	}
	else
	{
		if( GetXMLCache().m_AgentNames.Query( AgentID, agentname ) )
		{
			GetXMLGen().AddChildAttrib( szChildAttrib, agentname.m_Name );
		}
		else
		{
			GetXMLGen().AddChildAttrib( szChildAttrib, szDefault );
		}
	}
}

void CXMLDataClass::AddMessageSourceName( TCHAR* szChildAttrib, int MessageSourceID, bool bEscapeHTML )
{
	XMessageSourceNames msgsrcname;

	if( GetXMLCache().m_MsgSourceNames.Query( MessageSourceID, msgsrcname ) )
	{
		if (bEscapeHTML)
		{
			CEMSString sTemp(msgsrcname.m_Name);
			sTemp.EscapeHTML();
			GetXMLGen().AddChildAttrib( szChildAttrib, sTemp.c_str() );
		}
		else
		{
			GetXMLGen().AddChildAttrib( szChildAttrib, msgsrcname.m_Name );
		}
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, "Unknown" );
	}
}

void CXMLDataClass::AddMessageDestinationName( TCHAR* szChildAttrib, int MessageDestinationID, bool bEscapeHTML )
{
	XMessageDestinationNames msgdestname;

	if( GetXMLCache().m_MsgDestNames.Query( MessageDestinationID, msgdestname ) )
	{
		if (bEscapeHTML)
		{
			CEMSString sTemp(msgdestname.m_Name);
			sTemp.EscapeHTML();
			GetXMLGen().AddChildAttrib( szChildAttrib, sTemp.c_str() );
		}
		else
		{
			GetXMLGen().AddChildAttrib( szChildAttrib, msgdestname.m_Name );
		}
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, "Unknown" );
	}
}

bool CXMLDataClass::GetAgentName( int AgentID, tstring& sName )
{
	XAgentNames agentname;
	
	if( GetXMLCache().m_AgentNames.Query( AgentID, agentname ) )
	{
		sName = agentname.m_Name;
		return true;
	}
	else if( AgentID == 0 )
	{
		sName = _T("System");
		return true;
	}

	return false;
}

bool CXMLDataClass::GetOwnerName( int AgentID, tstring& sName )
{
	XAgentNames agentname;
	
	if( GetXMLCache().m_AgentNames.Query( AgentID, agentname ) )
	{
		sName = agentname.m_Name;
		return true;
	}
	else if( AgentID == 0 )
	{
		sName = _T("None");
		return true;
	}

	return false;
}

bool CXMLDataClass::GetGroupName( int GroupID, tstring& sName )
{
	XGroupNames groupname;

	if( GetXMLCache().m_GroupNames.Query( GroupID, groupname ) )
	{
		sName = groupname.m_GroupName;
		return true;
	}

	return false;
}

bool CXMLDataClass::GetTicketBoxName( int ID, tstring& sName )
{
	XTicketBoxNames tbname;
	
	if( GetXMLCache().m_TicketBoxNames.Query( ID, tbname ) )
	{
		sName = tbname.m_Name;
		return true;
	}

	return false;
}

bool CXMLDataClass::GetTicketCategoryName( int ID, tstring& sName )
{
	XTicketCategoryNames tcname;
	
	if( GetXMLCache().m_TicketCategoryNames.Query( ID, tcname ) )
	{
		sName = tcname.m_Name;
		return true;
	}
	else
	{
		sName.assign(_T("Unassigned"));
	}

	return false;
}

bool CXMLDataClass::GetRoutingRuleName( int ID, tstring& sName )
{
	XRoutingRuleNames name;
	
	if( GetXMLCache().m_RoutingRuleNames.Query( ID, name ) )
	{
		sName = name.m_Name;
		return true;
	}

	return false;
}

bool CXMLDataClass::GetMsgSourceName( int ID, tstring& sName )
{
	XMessageSourceNames name;
	
	if( GetXMLCache().m_MsgSourceNames.Query( ID, name ) )
	{
		sName = name.m_Name;
		return true;
	}

	return false;
}

bool CXMLDataClass::GetTicketPriorityName( int PriorityID, tstring& sPriorityName )
{
	XPriorities pname;
	
	if( GetXMLCache().m_TicketPriorities.Query( PriorityID, pname ) )
	{
		sPriorityName = pname.m_Name;
		return true;
	}

	return false;
}

bool CXMLDataClass::GetTicketStateName( int StateID, tstring& sStateName )
{
	XTicketStates sname;
	
	if( GetXMLCache().m_TicketStates.Query( StateID, sname ) )
	{
		sStateName = sname.m_Description;
		return true;
	}	

	return false;
}

bool CXMLDataClass::GetTicketFieldName( int FieldID, tstring& sFieldName )
{
	XTicketFields sname;
	
	if( GetXMLCache().m_TicketFields.Query( FieldID, sname ) )
	{
		sFieldName = sname.m_Description;
		return true;
	}	

	return false;
}

bool CXMLDataClass::GetStdCategoryName( int nCategoryID, tstring& sStdCatName )
{
	bool bRet = true;

	if ( nCategoryID == EMS_STDRESP_UNCATEGORIZED )
	{
		sStdCatName.assign( _T("Uncategorized") );
	}
	else if ( nCategoryID == EMS_STDRESP_DRAFTS )
	{
		sStdCatName.assign( _T("Drafts") );
	}
	else
	{
		XStdRespCategories stdrespcat;
		
		if ( GetXMLCache().m_StdRespCategories.Query( nCategoryID, stdrespcat ) )
		{
			sStdCatName.assign( stdrespcat.m_CategoryName );
			bRet = false;
		}
	}

	return bRet;
}

void CXMLDataClass::AddGroupName( TCHAR* szChildAttrib, int GroupID )
{
	XGroupNames groupname;
	
	if( GetXMLCache().m_GroupNames.Query( GroupID, groupname ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, groupname.m_GroupName );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("") );
	}
}

void CXMLDataClass::AddObjectType( TCHAR* szChildAttrib, int ObjectTypeID )
{
	XObjectTypes objecttype;
	
	if( GetXMLCache().m_ObjectTypes.Query( ObjectTypeID, objecttype ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, objecttype.m_Description );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unknown") );
	}
}

void CXMLDataClass::AddTicketBoxViewTypeName( TCHAR* szChildAttrib, int nTicketBoxViewTypeID )
{
	XTicketBoxViewTypes TicketBoxViewType;
	
	if( GetXMLCache().m_TicketBoxViewTypes.Query( nTicketBoxViewTypeID, TicketBoxViewType ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, TicketBoxViewType.m_Name );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unknown") );
	}
}

void CXMLDataClass::AddAlertMethodName( TCHAR* szChildAttrib, int AlertMethodID )
{
	XAlertMethods alertmethod;
	
	if( GetXMLCache().m_AlertMethods.Query( AlertMethodID, alertmethod ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, alertmethod.m_Description );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unknown") );
	}
}

void CXMLDataClass::AddAlertEventName( TCHAR* szChildAttrib, int AlertEventID )
{
	XAlertEvents alertevent;
	
	if( GetXMLCache().m_AlertEvents.Query( AlertEventID, alertevent ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, alertevent.m_Description );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Unknown") );
	}
}


void CXMLDataClass::AddServerParameter( TCHAR* szChildAttrib, int ServerParameterID )
{
	XServerParameters srvparam;
	
	if( GetXMLCache().m_ServerParameters.Query( ServerParameterID, srvparam ) )
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, srvparam.m_DataValue );
	}
	else
	{
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("") );
	}
}


bool CXMLDataClass::AddStdRespCategory( TCHAR* szChildAttrib, int nCategoryID, bool bEscapeHTML )
{
	XStdRespCategories StdRespCategory;
	
	// TODO - move this to the cache object
	switch ( nCategoryID )
	{
	case EMS_STDRESP_MY_FAVORITES:
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("My Favorites") );
		return true;

	case EMS_STDRESP_FREQUENTLY_USED:
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("My Most Frequently Used") );
		return true;

	case EMS_STDRESP_SEARCH_RESULTS:
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Search Results"));
		return true;

	case EMS_STDRESP_UNCATEGORIZED:
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Uncategorized") );
		return true;

	case EMS_STDRESP_DRAFTS:
		GetXMLGen().AddChildAttrib( szChildAttrib, _T("Drafts") );
		return true;
	}

	if ( GetXMLCache().m_StdRespCategories.Query( nCategoryID, StdRespCategory) )
	{
		if( bEscapeHTML )
		{
			CEMSString sString = StdRespCategory.m_CategoryName;
			sString.EscapeHTML();
			GetXMLGen().AddChildAttrib( szChildAttrib, sString.c_str() );
		}
		else
		{
			GetXMLGen().AddChildAttrib( szChildAttrib, StdRespCategory.m_CategoryName );
		}
		return true;
	}
	
	return false;
}


void CXMLDataClass::AddStyleSheetFile( TCHAR* szChildAttrib, int StyleSheetID )
{
	XStyleSheets css;
	
	if( GetXMLCache().m_StyleSheets.Query( StyleSheetID, css ) )
	{
		GetXMLGen().AddChildElem( szChildAttrib, css.m_Filename );
	}
	else
	{
		GetXMLGen().AddChildElem( szChildAttrib, _T("./stylesheets/standard.css") );
	}
	
	// Add the menu background color
	if ( _tcscmp( css.m_Filename, _T("./stylesheets/earth.css") ) == 0 )
	{
		GetXMLGen().AddChildElem( _T("menubackcolor"), "#90C0E8" );
	}
	else if ( _tcscmp( css.m_Filename, _T("./stylesheets/hotdog.css") ) == 0 )
	{
		GetXMLGen().AddChildElem( _T("menubackcolor"), "#FEC51D" );
	}
	else
	{
		GetXMLGen().AddChildElem( _T("menubackcolor"), _T("#D4D0C8") );
	}
}

void CXMLDataClass::GetServerParameter( int ServerParameterID, tstring& sValue, LPCTSTR lpDefault )
{
	XServerParameters srvparam;
	
	if( GetXMLCache().m_ServerParameters.Query( ServerParameterID, srvparam ) )
	{
		sValue = srvparam.m_DataValue;
	}
	else
	{
		sValue = lpDefault;
	}
}

void CXMLDataClass::SetServerParameter( int ServerParameterID, tstring& sValue )
{
	TServerParameters srvparam;
	
	srvparam.m_ServerParameterID = ServerParameterID;
	_tcsncpy( srvparam.m_DataValue, sValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH );
	
	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), srvparam.m_DataValue );
	BINDPARAM_LONG( GetQuery(), srvparam.m_ServerParameterID );
	GetQuery().Execute( _T("UPDATE ServerParameters SET DataValue=? WHERE ServerParameterID=?") );
	
	GetXMLCache().SetDirty(true);
	GetXMLCache().m_ServerParameters.Invalidate();
}


void CXMLDataClass::InvalidateAgents( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_AgentEmails.BuildMap( GetQuery() );
		GetXMLCache().m_AgentNames.BuildMap( GetQuery() );
		GetXMLCache().m_AgentNamesWOAdmin.BuildMap( GetQuery() );
		GetXMLCache().m_AgentIDs.Build( GetQuery() );
		GetXMLCache().m_EnabledAgentIDs.Build( GetQuery() );
		GetXMLCache().m_AgentSignatureIDs.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_AgentEmails.Invalidate();
		GetXMLCache().m_AgentNames.Invalidate();
		GetXMLCache().m_AgentNamesWOAdmin.Invalidate();
		GetXMLCache().m_AgentIDs.Invalidate();
		GetXMLCache().m_EnabledAgentIDs.Invalidate();
		GetXMLCache().m_AgentSignatureIDs.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,1);
}

void CXMLDataClass::InvalidateGroups( bool bRebuildNow )
{
	CAccessControl::GetInstance().Invalidate();

	if( bRebuildNow )
	{
		GetXMLCache().m_GroupNames.BuildMap( GetQuery() );
		GetXMLCache().m_GroupNamesWOAdmin.BuildMap( GetQuery() );
		GetXMLCache().m_AgentSignatureIDs.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_GroupNames.Invalidate();
		GetXMLCache().m_GroupNamesWOAdmin.Invalidate();
		GetXMLCache().m_AgentSignatureIDs.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,2);
}

void CXMLDataClass::InvalidateTicketBoxes( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_TicketBoxAddrs.BuildMap( GetQuery() );
		GetXMLCache().m_TicketBoxNames.BuildMap( GetQuery() );
		GetXMLCache().m_TicketBoxIDs.Build( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_TicketBoxAddrs.Invalidate();
		GetXMLCache().m_TicketBoxNames.Invalidate();
		GetXMLCache().m_TicketBoxIDs.Invalidate();
		GetXMLCache().m_DefaultEmailAddressName.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,3);
}

void CXMLDataClass::InvalidateMessageDestinations( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_MsgDestNames.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_MsgDestNames.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,4);
}


void CXMLDataClass::InvalidateMessageSources( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_MessageSourceTypeMatch.BuildMap( GetQuery() );
		GetXMLCache().m_MsgSourceNames.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_MsgSourceNames.Invalidate();
		GetXMLCache().m_MessageSourceTypeMatch.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,5);
}

void CXMLDataClass::InvalidateRoutingRules( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_RoutingRuleNames.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_RoutingRuleNames.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,6);
}

void CXMLDataClass::InvalidateProcessingRules( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_ProcessingRuleNames.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_ProcessingRuleNames.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,7);
}

void CXMLDataClass::InvalidateSignatures( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_Signatures.BuildMap( GetQuery() );
		GetXMLCache().m_AgentSignatureIDs.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_Signatures.Invalidate();
		GetXMLCache().m_AgentSignatureIDs.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,8);
}

void CXMLDataClass::InvalidateStdRespCategories( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_StdRespCategories.BuildMap( GetQuery() );
		GetXMLCache().m_SRCategoryIDs.Build( GetQuery() );
		GetXMLCache().m_SRCategoryNames.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_StdRespCategories.Invalidate();
		GetXMLCache().m_SRCategoryIDs.Invalidate();
		GetXMLCache().m_SRCategoryNames.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,9);
}


void CXMLDataClass::InvalidateServerParameters( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_ServerParameters.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_ServerParameters.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,10);
}

void CXMLDataClass::InvalidatePersonalDataTypes( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_PersonalDataTypes.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_PersonalDataTypes.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,11);
}

void CXMLDataClass::InvalidateAutoActionTypes( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_AutoActionTypes.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_AutoActionTypes.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,12);
}

void CXMLDataClass::InvalidateAutoActionEvents( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_AutoActionEvents.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_AutoActionEvents.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,13);
}

void CXMLDataClass::InvalidateTicketCategories( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_TicketCategories.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_TicketCategories.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,14);
}

void CXMLDataClass::InvalidateTicketBoxOwners( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_TicketBoxOwners.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_TicketBoxOwners.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,15);
}

void CXMLDataClass::InvalidateAutoMessages( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_AutoMessages.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_AutoMessages.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,16);
}

void CXMLDataClass::InvalidateAutoResponses( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_AutoResponses.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_AutoResponses.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,17);
}

void CXMLDataClass::InvalidateAgeAlerts( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_AgeAlerts.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_AgeAlerts.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,18);
}

void CXMLDataClass::InvalidateWaterMarkAlerts( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_WaterMarkAlerts.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_WaterMarkAlerts.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,19);
}

void CXMLDataClass::InvalidateAgentEmails( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_AgentEmails.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_AgentEmails.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,20);
}

void CXMLDataClass::InvalidateTracking( bool bRebuildNow )
{
	if( bRebuildNow )
	{
		GetXMLCache().m_AgentEmails.BuildMap( GetQuery() );
	}
	else
	{
		GetXMLCache().SetDirty(true);
		GetXMLCache().m_AgentEmails.Invalidate();
	}
	GetISAPIData().m_SessionMap.QueueSessionMonitors(3,20);
}

////////////////////////////////////////////////////////////////////////////////
// 
// VirusScanFile - Scan the file and return the new VirusScanStateID.
//                 Note that the quarantine/delete logic is automatically
//				   handled by the engine.
// 
////////////////////////////////////////////////////////////////////////////////
int CXMLDataClass::VirusScanFile( LPCTSTR szFile, tstring& sVirusName )
{
	tstring sValue;
	long nErr;
	tstring sQuarantineLoc;
	int nVirusScanStateID = EMS_VIRUS_SCAN_STATE_NOT_SCANNED;
	
	GetServerParameter( EMS_SRVPARAM_ANTIVIRUS_ENABLE, sValue );
	
	if( sValue.compare( _T("0") ) == 0 )
		return nVirusScanStateID;
	
	GetRoutingEngine().LogIt( EMS_ISAPI_LOG_INFO( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
							_T("Attempting to virus scan file %s" ), szFile );

	GetRoutingEngine().VirusScanFile( szFile, 0, sVirusName, sQuarantineLoc, &nErr );

	switch( nErr )
	{
		case CKAVScan::Success:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_INFO( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("No virus found in file %s"), szFile );
			nVirusScanStateID = EMS_VIRUS_SCAN_STATE_VIRUS_FREE;
			break;

		case CKAVScan::Success_Object_Has_Been_Cleaned:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_INFO( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("Virus %s found in file %s and repaired. Agent=%s"),
									  sVirusName.c_str(), szFile, GetAgentName() );
			nVirusScanStateID = EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_REPAIRED;
			break;

		case CKAVScan::Success_Object_Has_Been_Deleted:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_WARNING( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("Virus %s found in file %s which has been deleted. Agent=%s"), 
				sVirusName.c_str(), szFile, GetAgentName() ); 
			nVirusScanStateID = EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_DELETED;
			break;

		case CKAVScan::Success_Object_Has_Been_Quarantined:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_WARNING( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("Virus %s found in file %s which has been quarantined to %s Agent=%s"), 
				 sVirusName.c_str(), szFile, sQuarantineLoc.c_str(), GetAgentName() ); 
			nVirusScanStateID = EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED;
			break;

		case CKAVScan::Err_Object_Is_Infected:			
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_WARNING( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("Virus %s found in file %s. Agent=%s"), 
				sVirusName.c_str(), szFile, GetAgentName() ); 
			nVirusScanStateID = EMS_VIRUS_SCAN_STATE_VIRUS_FOUND;
			break;
		
		case CKAVScan::Err_Object_Is_Suspicious:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_WARNING( EMS_LOG_VIRUS_SCANNING, E_SuspiciousFile ),
				_T("File %s is suspicious. Agent=%s"), 
				szFile, GetAgentName() ); 
			nVirusScanStateID = EMS_VIRUS_SCAN_STATE_SUSPICIOUS;
			break;

		case CKAVScan::Err_Object_Was_Not_Scanned:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_WARNING( EMS_LOG_VIRUS_SCANNING, E_FileNotScanned ),
				_T("File %s could not be scanned. Agent=%s"), 
				szFile, GetAgentName() ); 
			nVirusScanStateID = EMS_VIRUS_SCAN_STATE_UNSCANNABLE;
			break;

		case CKAVScan::Err_Deleting_Infected_Object:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("Infected file (%s) containing virus %s could not be deleted. Agent=%s" ),
				szFile, sVirusName.c_str(), GetAgentName() );
			nVirusScanStateID = EMS_VIRUS_SCAN_STATE_VIRUS_FOUND;
			break;			

		default:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("Unable to initialize virus scanning component, error code=%d" ), nErr );
			CEMSString sError;
			sError.Format( _T("Unable to initialize virus scanning component, error code=%d" ), nErr );
			THROW_EMS_EXCEPTION_NOLOG( E_VirusScanFailed, sError );
			break;
	}

	return nVirusScanStateID;
}


/*---------------------------------------------------------------------------\                     
||  Comments: Virus scans a string....	              
\*--------------------------------------------------------------------------*/
int CXMLDataClass::VirusScanString( LPCTSTR szString, LONG nStringLen, 
								    CEMSString& sVirusName, tstring& sCleanedString )
{
	// is virus scanning enabled?
	tstring sValue;
	long nErr;
	int nRet = 0;
	
	GetServerParameter( EMS_SRVPARAM_ANTIVIRUS_ENABLE, sValue );
	
	if( sValue.compare( _T("0") ) == 0 )
		return 0;
	
	GetRoutingEngine().VirusScanString( szString, sVirusName, sCleanedString, &nErr );

	switch( nErr )
	{
		case CKAVScan::Success:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_INFO( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("No virus found in message body") );
			break;

		case CKAVScan::Success_Object_Has_Been_Cleaned:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_WARNING( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("Virus %s found in message body and repaired. Agent=%s"),
									  sVirusName.c_str(), GetAgentName() );
			nRet = E_CleanedBody;
			break;

		case CKAVScan::Err_Object_Is_Infected:			
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("Virus %s found in message body. Agent=%s"),
									  sVirusName.c_str(), GetAgentName() );
			nRet = E_InfectedBody;
			break;

		case CKAVScan::Err_Object_Is_Suspicious:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_WARNING( EMS_LOG_VIRUS_SCANNING, E_SuspiciousBody ),
				_T("Message body is suspicious and may contain a virus. Agent=%s"),
				GetAgentName() );
			nRet = E_SuspiciousBody;
			break;

		case CKAVScan::Err_Object_Was_Not_Scanned:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_WARNING( EMS_LOG_VIRUS_SCANNING, E_BodyNotScanned ),
				_T("Message body could not be scanned. Agent=%s"),
				 GetAgentName() );
			nRet = E_BodyNotScanned;
			break;

		default:
			GetRoutingEngine().LogIt( EMS_ISAPI_LOG_ERROR( EMS_LOG_VIRUS_SCANNING, E_VirusScanFailed ),
				_T("Unable to initialize scanning component, error code=%d" ), nErr );

			CEMSString sError;
			sError.Format(_T("Unable to initialize virus scanning component, error code=%d" ), nErr);
			THROW_EMS_EXCEPTION_NOLOG( E_VirusScanFailed, sError );

			break;
	}

	return nRet;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Scans an attachment and updates the virus scan state ID	
||				Option to throw an exception if the attachment is infected.
||				TAttachments::Query() must be called before this funciton.            
\*--------------------------------------------------------------------------*/
void CXMLDataClass::VirusScanAttachment( TAttachments Attachment, bool bException /*=false*/ )
{
	int nNewVirusScanStateID;
	tstring sAttachLocation;
	tstring sVirusName;
	
	// Allow restored files to be downloaded
	if( Attachment.m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_RESTORED ||
		Attachment.m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_REPAIRED )
	{
		return;
	}

	Attachment.m_IsInbound ? GetFullInboundAttachPath( Attachment.m_AttachmentLocation, sAttachLocation ) :
	                         GetFullOutboundAttachPath( Attachment.m_AttachmentLocation, sAttachLocation );
	
	// virus scan the file
	nNewVirusScanStateID = VirusScanFile( sAttachLocation.c_str(), sVirusName );
	
	// Has the virus scan state changed?
	if( nNewVirusScanStateID != Attachment.m_VirusScanStateID )
	{
		// Perform an update
		Attachment.m_VirusScanStateID = nNewVirusScanStateID;
		
		_tcscpy( Attachment.m_VirusName, sVirusName.c_str() );
		
		Attachment.Update( GetQuery() );
	}
	
	if( bException && EMS_IS_INFECTED( nNewVirusScanStateID ) )
	{
		CEMSString sMsg;
		sMsg.Format( EMS_STRING_ERROR_FILE_VIRUS, sAttachLocation.c_str(), sVirusName.c_str() );
		THROW_EMS_EXCEPTION_NOLOG( E_InfectedFile, sMsg.c_str() );		
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Retrieves a textual description for a virus scan state ID	              
\*--------------------------------------------------------------------------*/
void CXMLDataClass::GetVirusScanStateIDText( int nID, CEMSString& sDescription )
{
	switch ( nID )
	{
	case EMS_VIRUS_SCAN_STATE_NOT_SCANNED:
		sDescription.assign( _T("Has not been virus scanned") );
		break;
	case EMS_VIRUS_SCAN_STATE_VIRUS_FREE:
		sDescription.assign( _T("No viruses detected") );
		break;
	case EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_DELETED:
		sDescription.assign( _T("Deleted by virus scanner") );
		break;
	case EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED:
		sDescription.assign( _T("Quarantined by virus scanner") );
		break;
	case EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_REPAIRED:
		sDescription.assign( _T("Repaired by virus scanner") );
		break;
	case EMS_VIRUS_SCAN_STATE_VIRUS_FOUND:
		sDescription.assign( _T("Virus detected") );
		break;
	case EMS_VIRUS_SCAN_STATE_UNSCANNABLE:
		sDescription.assign( _T("Could not be virus scanned") );
		break;
	case EMS_VIRUS_SCAN_STATE_SUSPICIOUS:
		sDescription.assign( _T("May be virus infected (Suspicious)") );
		break;
	case EMS_VIRUS_SCAN_STATE_RESTORED:
		sDescription.assign( _T("Restored from quarantine") );
		break;

	default:
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments: Sends an alert when a disk full error received writing to file	              
\*--------------------------------------------------------------------------*/
void CXMLDataClass::SendLowDiskSpaceAlert( LPCTSTR szActualFile )
{
	BSTR bstr;
	wchar_t wcsText[MAX_PATH+100];

	swprintf( wcsText, L"Disk full error writing to file: %S", szActualFile );

	bstr = SysAllocString( wcsText );

	GetRoutingEngine().SendAlert( EMS_ALERT_EVENT_LOW_DISK_SPACE, 0, bstr );

	SysFreeString( bstr );
}

/*---------------------------------------------------------------------------\                     
||  Comments: Sends an alert when a ticket is reassigned	              
\*--------------------------------------------------------------------------*/
void CXMLDataClass::SendTicketAssignAlert( long AlertID, LPCTSTR szAlertText )
{
	BSTR bstr;
	wchar_t wcsText[MAX_PATH+100];

	swprintf( wcsText, L"%S", szAlertText );

	bstr = SysAllocString( wcsText );

	GetRoutingEngine().SendAlert( EMS_ALERT_EVENT_TICKET_ASSIGNED, AlertID, bstr );

	SysFreeString( bstr );
}

/*---------------------------------------------------------------------------\                     
||  Comments: Sends an alert when an outbound message requires approval	              
\*--------------------------------------------------------------------------*/
void CXMLDataClass::SendOutboundApprovalAlert( long AlertID, LPCTSTR szAlertText )
{
	BSTR bstr;
	wchar_t wcsText[MAX_PATH+100];

	swprintf( wcsText, L"%S", szAlertText );

	bstr = SysAllocString( wcsText );

	GetRoutingEngine().SendAlert( EMS_ALERT_EVENT_OUTBOUND_APPROVAL_REQUIRED, AlertID, bstr );

	SysFreeString( bstr );
}

/*---------------------------------------------------------------------------\                     
||  Comments: Sends an alert when an outbound message is approved	              
\*--------------------------------------------------------------------------*/
void CXMLDataClass::SendOutboundApprovedAlert( long AlertID, LPCTSTR szAlertText )
{
	BSTR bstr;
	wchar_t wcsText[MAX_PATH+100];

	swprintf( wcsText, L"%S", szAlertText );

	bstr = SysAllocString( wcsText );

	GetRoutingEngine().SendAlert( EMS_ALERT_EVENT_OUTBOUND_APPROVED, AlertID, bstr );

	SysFreeString( bstr );
}

/*---------------------------------------------------------------------------\                     
||  Comments: Sends an alert when an outbound message is returned	              
\*--------------------------------------------------------------------------*/
void CXMLDataClass::SendOutboundReturnedAlert( long AlertID, LPCTSTR szAlertText )
{
	BSTR bstr;
	wchar_t wcsText[MAX_PATH+100];

	swprintf( wcsText, L"%S", szAlertText );

	bstr = SysAllocString( wcsText );

	GetRoutingEngine().SendAlert( EMS_ALERT_EVENT_OUTBOUND_RETURNED, AlertID, bstr );

	SysFreeString( bstr );
}

int CXMLDataClass::UseCutoffDate(TIMESTAMP_STRUCT& ts)
{
	dca::String sUseCuttoffDate("0");

	GetServerParameter(41, sUseCuttoffDate, "0");

	if(sUseCuttoffDate.ToInt() != 0)
	{
		dca::String sDate;

		GetServerParameter(42, sDate);

		if(sDate.size() == 0)
			return 0;

		dca::String::size_type pos = dca::String::npos;

		int nDatePos = 0;
		do
		{
			pos = sDate.find('/');

			if(pos != dca::String::npos)
			{
				dca::String temp = sDate.substr(0, pos);
				sDate = sDate.erase(0, pos + 1);
				switch(nDatePos)
				{
				case 0:
					{
						ts.month = temp.ToInt();
						nDatePos++;
					}
					break;
				case 1:
					{
						ts.day = temp.ToInt();
						nDatePos++;
					}
					break;
				}
			}
			else
			{
				if(nDatePos == 2)
				{
					ts.year = sDate.ToInt();
				}
			}
		}while(pos != dca::String::npos);

		ts.hour = 23;
		ts.minute = 59;
		ts.second = 59;

		return 1;
	}

	return 0;
}
