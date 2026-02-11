#pragma once

class AboutController : public dca::DialogBoxController
{
public:
	AboutController(void);
	~AboutController(void);
	virtual int OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam);
	int OnCommand(int nCodeNotify, int nID, HWND hwnd);
};
