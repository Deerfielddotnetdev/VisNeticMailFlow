//******************************************************************************
// $Author: markm $
// $Date: 2005/09/26 20:09:11 $
// $Name:  $
// $Revision: 1.5 $
// $Source: c:/cvsrepo/mailflow/vmfadmintool/vmfadmintool/ForceHTTPSController.h,v $
//******************************************************************************

#pragma once

//******************************************************************************
// TopController class

class ForceHTTPSController : public dca::DialogBoxController
{
public:
	ForceHTTPSController(int nForceHttps = 0);
	~ForceHTTPSController(void);
	virtual int OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam);
	int GetForceHttps() { return _nForceHttps; }
protected:
	int OnInitDialog();
	int OnCommand(int nCodeNotify, int nID, HWND hwnd);
	int OnCmdForceHttps();
protected:
	int         _nForceHttps;
	dca::Button _chkForceHttps;
};

//******************************************************************************
