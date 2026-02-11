// MainFrame.cpp: implementation of the CMainFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainFrame.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainFrame::CMainFrame( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{

}

CMainFrame::~CMainFrame()
{

}


int CMainFrame::Run( CURLAction& action )
{
	tstring sPane;

	if( GetISAPIData().m_sPage.compare( _T("main") ) == 0)
	{
		if( GetISAPIData().GetURLString( _T("rightpane"), sPane, true ) && sPane.size() > 0 )
		{
			Translate( sPane );
			GetXMLGen().AddChildElem(_T("rightpane"));
			GetXMLGen().SetChildData( sPane.c_str(), 1 );
		}
		else
		{
			GetXMLGen().AddChildElem(_T("rightpane"));
			GetXMLGen().SetChildData( GetBrowserSession().m_RightHandPane.c_str(), 1 );
		}

	}
	else if( GetISAPIData().m_sPage.compare( _T("mailflow") ) == 0)
	{
		if( GetISAPIData().GetURLString( _T("rightpane"), sPane, true ) && sPane.size() > 0 )
		{
			GetXMLGen().AddChildElem(_T("rightpane"));
			GetXMLGen().SetChildData( sPane.c_str(), 1 );
		}
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Translate
// 
////////////////////////////////////////////////////////////////////////////////
void CMainFrame::Translate( tstring& sURL )
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
