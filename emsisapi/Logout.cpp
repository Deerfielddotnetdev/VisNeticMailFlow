// Logout.cpp: implementation of the CLogout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Logout.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogout::CLogout( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{

}


CLogout::~CLogout()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
//  The main entry point
// 
////////////////////////////////////////////////////////////////////////////////
int CLogout::Run( CURLAction& action )
{
	int nAgentID=GetAgentID();
	tstring sValue;
	tstring sSessionID(GetISAPIData().m_sSessionID);
	XAgentNames name;
		
	GetServerParameter(EMS_SRVPARAM_SESSION_LOGOFF_ALL, sValue, "0");
	GetXMLCache().m_AgentNames.Query( nAgentID, name );	
	
	if(_ttoi(sValue.c_str()) == 0)
	{
		if( GetISAPIData().m_SessionMap.RemoveSession( sSessionID, false, 0 ) )
		{
			GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
												EMSERR_ISAPI_EXTENSION,
												EMS_LOG_AUTHENTICATION,
												E_Logout),
										"%s logged out of all Sessions", 
										name.m_Name );
		}
		else
		{
			GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
												EMSERR_ISAPI_EXTENSION,
												EMS_LOG_AUTHENTICATION,
												E_Logout),
										"%s logged out of Session %s", 
										name.m_Name, sSessionID.c_str());
		}
	}
	else
	{
		if( GetISAPIData().m_SessionMap.RemoveAllSessionsForAgent(nAgentID) )
		{
			GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,
												EMSERR_ISAPI_EXTENSION,
												EMS_LOG_AUTHENTICATION,
												E_Logout),
										"%s logged out of all Sessions", 
										name.m_Name );
		}
	}
	
	return 0;
}
