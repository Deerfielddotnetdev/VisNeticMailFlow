#include "prehead.h"
#include ".\AboutController.h"

AboutController::AboutController(void)
{
}

AboutController::~AboutController(void)
{
}

int AboutController::OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam)
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

int AboutController::OnCommand(int nCodeNotify, int nID, HWND hwnd)
{
	switch(nID)
	{
	case IDOK:
		return OnClose();
	}

	return 0;
}
