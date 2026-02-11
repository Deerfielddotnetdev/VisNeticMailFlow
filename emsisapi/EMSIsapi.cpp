////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/EMSIsapi.cpp,v 1.1.6.1 2006/07/18 12:55:03 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// EMSIsapi.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ThreadPool.h"
#include "RegistryFns.h"

// globals
HINSTANCE	g_hInstance = NULL;
CThreadPool g_ThreadPool;
int nFull = 0;
int nAlertFull = 0;

/*---------------------------------------------------------------------------\                     
||  Comments:	DLL Main Entry Point	              
\*--------------------------------------------------------------------------*/
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  dwReason, 
                       LPVOID lpReserved
					 )
{
	switch( dwReason ) 
	{
    case DLL_PROCESS_ATTACH:
		g_hInstance = (HINSTANCE)hModule;
        break;
		
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

BOOL WINAPI GetExtensionVersion( OUT HSE_VERSION_INFO * pVer )
/*++

Purpose:

    The first function called after IIS successfully 
    loads the DLL.  The function should use the 
    version structure provided by IIS to set the ISAPI
    architectural version number of this extension.

    A simple text-string is also set so that 
    administrators can identify the DLL.

    Note that HSE_VERSION_MINOR and HSE_VERSION_MAJOR
    are constants defined in httpext.h.

Arguments: 

    pVer - points to extension version structure

Return Value:

    TRUE if successful; FALSE otherwise.    

--*/
{
 	DebugReporter::Instance().DisplayMessage("EMIsapi::GetExtensionVersion", DebugReporter::ISAPI, GetCurrentThreadId());
	
	g_ThreadPool.Initialize( pVer );

	int nQueueFull=g_ThreadPool.GetSharedObjects().SessionMap().GetQueueFull();

	dca::String o;
	o.Format("GetExtensionVersion - Retrieved QueueFull value of %d from server parameters.", nQueueFull);
	DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
	if(nQueueFull == 0 || nQueueFull < 256)
	{
		nAlertFull = nQueueFull;
		o.Format("GetExtensionVersion - Setting nAlertFull to %d.", nAlertFull);
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
	}	
    return TRUE;
}


DWORD WINAPI HttpExtensionProc( IN EXTENSION_CONTROL_BLOCK * pECB )
/*++

Purpose:    

    Function called by the IIS Server when a request 
    for the ISAPI dll arrives.  The HttpExtensionProc                  
    function processes the request and outputs the
    appropriate response to the web client using
    WriteClient().

Argument:

    pECB - pointer to extention control block.

Return Value:

    HSE_STATUS_SUCCESS

--*/
{
	if( g_ThreadPool.HandleRequest( pECB ) == 0)
	{
		nFull = 0;
		return HSE_STATUS_PENDING;
	}
	else
	{
		nFull++;
		dca::String o;
			
		if(nFull == nAlertFull && nAlertFull != 0)
		{
			o.Format("HttpExtensionProc - Queue Full %d consecutive requests, asking for recycle.", nAlertFull);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
			pECB->ServerSupportFunction(pECB->ConnID,HSE_REQ_REPORT_UNHEALTHY,"Queue Full",NULL,NULL);
			nFull = 0;			
		}
		else
		{
			o.Format("HttpExtensionProc - Queue Full %d consecutive requests.", nFull);
			DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		}	
		
		return HSE_STATUS_ERROR;
	}
}


BOOL WINAPI TerminateExtension( IN DWORD dwFlags )
/*++

Routine Description:

    This function is called when the WWW service is shutdown

Arguments:

    dwFlags - HSE_TERM_ADVISORY_UNLOAD or HSE_TERM_MUST_UNLOAD

Return Value:

    TRUE if extension is ready to be unloaded,
    FALSE otherwise

--*/
{
    // Note: We must not agree to be unloaded if we have
    // any pending requests.
	DebugReporter::Instance().DisplayMessage("EMIsapi::TerminateExtension - terminating worker and session monitor threads", DebugReporter::ISAPI, GetCurrentThreadId());

	g_ThreadPool.Terminate();

	DebugReporter::Instance().DisplayMessage("EMIsapi::TerminateExtension - worker and session monitor threads terminated", DebugReporter::ISAPI, GetCurrentThreadId());

    return TRUE;
}