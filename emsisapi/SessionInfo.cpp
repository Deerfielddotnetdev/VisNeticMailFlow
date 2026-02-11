// SessionInfo.cpp: implementation of the CSessionInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SessionInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSessionInfo::CSessionInfo(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CSessionInfo::~CSessionInfo()
{

}


int CSessionInfo::Run( CURLAction& action )
{
	// Check security
	RequireAdmin();
	tstring sSessionID;

	if( GetISAPIData().GetURLString( _T("ClearSession"), sSessionID, true ) )
	{
		GetISAPIData().m_SessionMap.RemoveSession( sSessionID, true, GetAgentID() );	
	}

	GetISAPIData().m_SessionMap.GetSessionXML( GetXMLGen() );

	return 0;
}