#include "prehead.h"
#include ".\AdminPwdController.h"

AdminPwdController::AdminPwdController()
	:_txtPassword(TXT_NEW_AGENT_PASSWD)
	,_txtConfirmPassword(TXT_CONFIRM_AGENT_PASSWD)
{
	this->AddControl(_txtPassword);
	this->AddControl(_txtConfirmPassword);
}

AdminPwdController::~AdminPwdController(void)
{
}

int AdminPwdController::OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(dca::DialogBoxController::OnMsgHandler(msg, wParam, lParam))
	{
		switch(msg)
		{
		case WM_COMMAND:
			return OnCommand(HIWORD(wParam), LOWORD(wParam), HWND(lParam));
		}
	}
	
	return 1;
}

int AdminPwdController::OnCommand(int nCodeNotify, int nID, HWND hwnd)
{
	switch(nID)
	{
	case CMD_ADMIN_PASSWD_CANCEL:
		return OnClose();
	case CMD_ADMIN_PASSWD_OK:
		return OnAdmimPasswdOK();
	}

	return 0;
}

int AdminPwdController::OnAdmimPasswdOK()
{
	dca::WString sTemp;

	_txtPassword.GetText(_sPassword);
	_txtConfirmPassword.GetText(sTemp);

	if(_sPassword.compare(sTemp))
	{
		dca::MsgBox(_T("Passwords don't match!"), _T("VisNetic MailFlow Admin Tool"));
	}
	else
	{
		_nReturn = 1;

		return OnClose();
	}

	return 0;
}
