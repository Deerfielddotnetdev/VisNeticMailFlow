/***************************************************************************\
||             
||  $Header: /root/EMSRESOURCE/EMSResource.cpp,v 1.1 2005/08/09 16:28:22 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Mailstream Shared Resource DLL   
||              
\\*************************************************************************/

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
