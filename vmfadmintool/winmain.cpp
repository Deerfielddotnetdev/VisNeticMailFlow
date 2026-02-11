//******************************************************************************
// $Author: markm $
// $Date: 2005/09/28 16:06:55 $
// $Name:  $
// $Revision: 1.5 $
// $Source: c:/cvsrepo/mailflow/vmfadmintool/vmfadmintool/winmain.cpp,v $
//******************************************************************************

// Precompiled header
#include "prehead.h"

// Local header files
#include ".\VMFAdminToolApplication.h"
VMFAdminToolApplication* pApp = NULL;

//******************************************************************************
// initial entry point for a Windows-based application

int WINAPI WinMain(HINSTANCE hInst
				   ,HINSTANCE hPrevInst
				   ,char* lpCommand
				   ,int nShow)
{
	int nRetVal = 0; // Return value from Initialize

	{
		// Create our Application object
		VMFAdminToolApplication theApp(hInst);

		pApp = &theApp;

		// Initialize Application object
		nRetVal = theApp.Initialize();
	}

	pApp = NULL;

	return nRetVal;
}

//******************************************************************************
