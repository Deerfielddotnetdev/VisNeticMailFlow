// DownloadSVG.cpp: implementation of the CDownloadSVG class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DownloadSVG.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDownloadSVG::CDownloadSVG(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{

}

CDownloadSVG::~CDownloadSVG()
{

}


int CDownloadSVG::Run( CURLAction& action )
{
	HSE_URL_MAPEX_INFO MapInfo;
	TCHAR szSrcPath[MAX_PATH];
	DWORD dwPathLength = MAX_PATH;
	HANDLE hFile;
	bool bExists = false;
	TCHAR* Path = _T("plug-ins/SVGView.exe");
	tstring sValue;

	_tcscpy( szSrcPath, GetISAPIData().m_sURLSubDir.c_str() );
	_tcscat( szSrcPath, Path );
	ZeroMemory( &MapInfo, sizeof(MapInfo) );
	
	GetISAPIData().m_pECB->ServerSupportFunction( GetISAPIData().m_pECB->ConnID, 
		                                          HSE_REQ_MAP_URL_TO_PATH,
						                          szSrcPath, &dwPathLength, 
												  (DWORD*) &MapInfo );


	hFile = CreateFile( szSrcPath, GENERIC_READ, 
		                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
		                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if( hFile != INVALID_HANDLE_VALUE )
	{
		bExists = true;
		CloseHandle( hFile );

		if( GetISAPIData().GetURLString( _T("Download"), sValue, true ) )
		{
			action.m_sPageTitle.assign( _T("application/octet-stream") );
			action.SetSendFile( szSrcPath, _T("SVGView.exe") );
			return 0;
		}
	}

	GetXMLGen().AddChildElem( _T("SVG") );
	GetXMLGen().AddChildAttrib( _T("Exists"), bExists ? _T("1") : _T("0") );

	return 0;
}