#pragma once

class MinMBSizeController : public dca::DialogBoxController
{
public:
	MinMBSizeController(void);
	~MinMBSizeController(void);
	virtual int OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	int OnInitDialog();
	int OnCommand(int nCodeNotify, int nID, HWND hwnd);
	int OnCmdMinDiskOK();
protected:
	dca::TextBox _txtMBSize;
};
