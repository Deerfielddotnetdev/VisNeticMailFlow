//******************************************************************************
// $Author: markm $
// $Date: 2005/09/26 20:09:11 $
// $Name:  $
// $Revision: 1.5 $
// $Source: c:/cvsrepo/mailflow/vmfadmintool/vmfadmintool/ForceHTTPSController.cpp,v $
//******************************************************************************

#include "prehead.h"
#include ".\ForceHTTPSController.h"

//******************************************************************************
// Constructor

ForceHTTPSController::ForceHTTPSController(int nForceHttps)
	:_nForceHttps(nForceHttps)
	,_chkForceHttps(CHK_FORCE_HTTPS)
{
	this->AddControl(_chkForceHttps);
}

//******************************************************************************

//******************************************************************************
// Destructor

ForceHTTPSController::~ForceHTTPSController(void)
{
}

//******************************************************************************

//******************************************************************************
// OnMsgHandler - Handles messages

int ForceHTTPSController::OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(dca::DialogBoxController::OnMsgHandler(msg, wParam, lParam))
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			return OnInitDialog();
		case WM_COMMAND:
			return OnCommand(HIWORD(wParam), LOWORD(wParam), HWND(lParam));
		}
	}
	
	return 1;
}

//******************************************************************************

//******************************************************************************

int ForceHTTPSController::OnInitDialog()
{
	if(_nForceHttps)
		_chkForceHttps.SetStateToChecked();
	else
		_chkForceHttps.SetStateToUnChecked();

	return 1;
}

//******************************************************************************

//******************************************************************************

int ForceHTTPSController::OnCommand(int nCodeNotify, int nID, HWND hwnd)
{
	switch(nID)
	{
	case CMD_FORCE_HTTPS_CANCEL:
		return OnClose();
	case CMD_FORCE_HTTPS_OK:
		return OnCmdForceHttps();
	}

	return 0;
}

//******************************************************************************

//******************************************************************************

int ForceHTTPSController::OnCmdForceHttps()
{
	UINT nState = _chkForceHttps.GetCheckedState();

	if(nState == BST_CHECKED)
		_nForceHttps = 1;
	else
		_nForceHttps = 0;

	_nReturn = 1;

	return OnClose();
}

//******************************************************************************
