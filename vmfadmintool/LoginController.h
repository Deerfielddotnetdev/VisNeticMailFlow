#pragma once

class LoginController : public dca::DialogBoxController
{
public:
	LoginController(void);
	~LoginController(void);
	virtual int OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam);
	int OnCommand(int nCodeNotify, int nID, HWND hwnd);
	int OnLogin();
	LPCTSTR GetUsername() const { return _sUsername.c_str(); }
	LPCTSTR GetPassword() const { return _sPassword.c_str(); }
protected:
	dca::TextBox _txtUsername;
	dca::TextBox _txtPassword;
	dca::WString _sUsername;
	dca::WString _sPassword;
};
