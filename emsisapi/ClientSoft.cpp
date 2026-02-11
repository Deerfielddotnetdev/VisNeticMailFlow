/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ClientSoft.cpp
||
||  Copyright © 2009 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ClientSoft.h"
#include "UploadMap.h"
#include "AttachFns.h"
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CClientSoft::CClientSoft(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	// handle exceptions in the local EMS file
	GetISAPIData().SetUseExceptionEMS(false);
}

CClientSoft::~CClientSoft()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point
\*--------------------------------------------------------------------------*/
int CClientSoft::Run(CURLAction& action)
{
	tstring sAction;
	GetISAPIData().GetFormString( _T("ACTION"), sAction, true );
	
	try
	{
		if ( sAction.compare( _T("add") ) == 0 )
		{
			OutputDebugString("CClientSoft::Run - Before AddSoft\n");
			AddSoft();
		}
		else if ( sAction.compare( _T("delete") ) == 0 )
		{
			OutputDebugString("CClientSoft::Run - Before DeleteSoft\n");
			DeleteSoft();
		}
	}
	catch(...)
	{
		// update the progress indicator that the upload is complete
		CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );

		ListSoft();
		throw;
	}

	// update the progress indicator that the upload is complete
	CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );

	ListSoft();
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists files in the plug-ins folder	              
\*--------------------------------------------------------------------------*/
void CClientSoft::ListSoft( void )
{
	tstring sValue;
	GetServerParameter( EMS_SRVPARAM_3CX_PLUGIN_VERSION, sValue );
	GetXMLGen().AddChildElem( _T("PlugIn3cx") );
	GetXMLGen().AddChildAttrib( _T("Version"), sValue.c_str() );	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds a file to the plug-ins folder	              
\*--------------------------------------------------------------------------*/
void CClientSoft::AddSoft( void )
{
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Removes a file from the plug-ins folder              
\*--------------------------------------------------------------------------*/
void CClientSoft::DeleteSoft( void )
{
	
}
