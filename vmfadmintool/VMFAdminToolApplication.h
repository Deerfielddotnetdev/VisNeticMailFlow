//******************************************************************************
// $Author: markm $
// $Date: 2005/09/28 16:06:55 $
// $Name:  $
// $Revision: 1.6 $
// $Source: c:/cvsrepo/mailflow/vmfadmintool/vmfadmintool/VMFAdminToolApplication.h,v $
//******************************************************************************

#pragma once

//******************************************************************************
// Our Application class

class VMFAdminToolApplication : public dca::WinApplication
{
public:
	VMFAdminToolApplication(HINSTANCE hInst);	// Default constructor
	~VMFAdminToolApplication(void);

	virtual int Initialize(int nShow = 1);		// Override initialize function
	void ReloadMailFlow();
protected:
	int DoMainDialog(int nWriteAccess);
	int HaveAdminRights();
	int Authenticate(LPCTSTR sUsername, LPCTSTR sPassword);
};

//******************************************************************************
