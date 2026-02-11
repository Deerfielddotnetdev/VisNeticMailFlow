#include "prehead.h"
#include ".\LoginController.h"

LoginController::LoginController(void)
	:_txtUsername(TXT_LOGIN_USERNAME)
	,_txtPassword(TXT_LOGIN_PASSWORD)
{
	this->AddControl(_txtUsername);
	this->AddControl(_txtPassword);
}

LoginController::~LoginController(void)
{
}

int LoginController::OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam)
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

int LoginController::OnCommand(int nCodeNotify, int nID, HWND hwnd)
{
	switch(nID)
	{
	case CMD_LOGIN:
		return OnLogin();
	}

	return 0;
}

int LoginController::OnLogin()
{
	_txtUsername.GetText(_sUsername);
	_txtPassword.GetText(_sPassword);

	_nReturn = 1;

	return OnClose();;
}
