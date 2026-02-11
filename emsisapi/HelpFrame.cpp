// HelpFrame.cpp: implementation of the CHelpFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HelpFrame.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHelpFrame::CHelpFrame( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{

}

CHelpFrame::~CHelpFrame()
{

}


int CHelpFrame::Run( CURLAction& action )
{
	tstring sPane;

	if( GetISAPIData().m_sPage.compare( _T("mainhelp") ) == 0)
	{
		if( GetISAPIData().GetURLString( _T("righthelp"), sPane, true ) && sPane.size() > 0 )
		{
			Translate( sPane );
			GetXMLGen().AddChildElem(_T("righthelp"));
			GetXMLGen().SetChildData( sPane.c_str(), 1 );			
		}
		else
		{
			GetXMLGen().AddChildElem(_T("righthelp"));
			if (m_ISAPIData.m_pSession->m_TempType == 2)
			{
				GetXMLGen().SetChildData( m_ISAPIData.m_pSession->m_TempData.c_str(), 1 );				
			}
			else
			{
				GetXMLGen().SetChildData( _T("help/introduction.htm"), 1 );
			}
		}

	}
	else if( GetISAPIData().m_sPage.compare( _T("help") ) == 0)
	{
		if( GetISAPIData().GetURLString( _T("righthelp"), sPane, true ) && sPane.size() > 0 )
		{
			GetISAPIData().m_SessionMap.SetTempType( m_ISAPIData.m_pSession->m_AgentID, 2 );
			GetISAPIData().m_SessionMap.SetTempData( m_ISAPIData.m_pSession->m_AgentID, sPane );
			action.SetRedirectURL(_T("help.ems"));
		}
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Translate
// 
////////////////////////////////////////////////////////////////////////////////
void CHelpFrame::Translate( tstring& sURL )
{
	tstring::size_type pos;

	pos = sURL.find( _T('$') );
	if ( pos != tstring::npos )
	{
		sURL.replace( pos, 1, 1, _T('?') );

		while ( (pos = sURL.find( _T('$'), pos)) != tstring::npos )
		{
			sURL.replace( pos, 1, 1, _T('&') );		
		}
	}
}
