/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/UploadProgress.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "UploadProgress.h"
#include "UploadMap.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CUploadProgress::CUploadProgress(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{

}

CUploadProgress::~CUploadProgress()
{

}

int CUploadProgress::Run(CURLAction& action)
{
	int nUploadID;
	int Percent;
	int nDoCancel;

	GetISAPIData().GetURLLong( _T("ID"), nUploadID );
	GetISAPIData().GetURLLong( _T("docancel"), nDoCancel, true );

	if (nDoCancel == 1)
	{
		CUploadMap::GetInstance().UpdateProgress( nUploadID, 101 );
	}

	Percent = CUploadMap::GetInstance().GetProgress( nUploadID );

	GetXMLGen().AddChildElem( _T("Upload") );
	GetXMLGen().AddChildAttrib( _T("ID"), nUploadID );
	GetXMLGen().AddChildAttrib( _T("Percent"), Percent );
	
	return 0;
}
