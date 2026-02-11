#include "prehead.h"
#include ".\MinMBSizeController.h"

MinMBSizeController::MinMBSizeController(void)
	:_txtMBSize(TXT_MIN_MB)
{
	this->AddControl(_txtMBSize);
}

MinMBSizeController::~MinMBSizeController(void)
{
}

int MinMBSizeController::OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam)
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

int MinMBSizeController::OnInitDialog()
{
	dca::RegistryData mfRD(KEY_EXECUTE);
	dca::WString sMBSize;

	DWORD dwMBSize = mfRD.GetMinimumSpaceMB();

	sMBSize.FromInt(int(dwMBSize));

	_txtMBSize.SetText(sMBSize.c_str());

	return 1;
}

int MinMBSizeController::OnCommand(int nCodeNotify, int nID, HWND hwnd)
{
	switch(nID)
	{
	case CMD_MIN_DISK_CANCEL:
		return OnClose();
	case CMD_MIN_DISK_OK:
		return OnCmdMinDiskOK();
	}

	return 0;
}

int MinMBSizeController::OnCmdMinDiskOK()
{
	dca::WString sMBSize;
	_txtMBSize.GetText(sMBSize);

	int nMBSize = sMBSize.ToInt();
	if(nMBSize < 100)
		nMBSize = 100;

	dca::RegistryData mfRD(KEY_WRITE);
	mfRD.SetMinimumSpaceMB((DWORD)nMBSize);

	return OnClose();
}
