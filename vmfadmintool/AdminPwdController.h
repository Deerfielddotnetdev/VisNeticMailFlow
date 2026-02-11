#pragma once

class AdminPwdController : public dca::DialogBoxController
{
public:
	AdminPwdController(void);
	~AdminPwdController(void);
	virtual int OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam);
	LPCTSTR GetNewPassword() const { return _sPassword.c_str(); }
protected:
	int OnInitDialog();
	int OnCommand(int nCodeNotify, int nID, HWND hwnd);
	int OnAdmimPasswdOK();
protected:
	dca::TextBox _txtPassword;
	dca::TextBox _txtConfirmPassword;
	dca::WString _sPassword;
};
