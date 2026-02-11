//******************************************************************************
// $Author: markm $
// $Date: 2005/10/28 17:43:30 $
// $Name:  $
// $Revision: 1.11 $
// $Source: c:/cvsrepo/mailflow/vmfadmintool/vmfadmintool/TopController.cpp,v $
//******************************************************************************

#include "prehead.h"
#include ".\TopController.h"
#include ".\ForceHTTPSController.h"
#include ".\AdminPwdController.h"
#include ".\MinMBSizeController.h"
#include ".\AboutController.h"
#include ".\VMFAdminToolApplication.h"
#include <shellapi.h>
#include <time.h>
#include "DateFns.h"

extern VMFAdminToolApplication* pApp;

TopController::TopController(int nWriteAccess)
	:_nWriteAccess(nWriteAccess)
	,_nWasTested(1)
	,_nUseWia(0)
	,_comboOdbcDriver(COMBO_ODBC)
	,_txtServer(TXT_SERVER)
	,_txtDatabase(TXT_DATABASE)
	,_txtUsername(TXT_USERNAME)
	,_txtPasswordWin(TXT_PASSWORD_WIN)
	,_txtDomain(TXT_DOMAIN)
	,_txtLoginName(TXT_LOGIN_NAME)
	,_txtPasswordODBC(TXT_PASSWORD_ODBC)
	,_optSQLAuth(OPT_SQL_AUTH)
	,_optWinAuth(OPT_WIN_AUTH)
	,_cmdSave(CMD_SAVE)
	,_chkSecure(CHK_SECURE)
	,_chkTrust(CHK_TRUST)
	,_selMsgSrc(SEL_MSG_SRC)
	,_selMsgDest(SEL_MSG_DEST)
	,_btnMsgSrc(BTN_MSG_SRC)
	,_btnMsgDest(BTN_MSG_DEST)
	,_btnRefresh(BTN_REFRESH)
{
	this->AddControl(_comboOdbcDriver);
	this->AddControl(_txtServer);
	this->AddControl(_txtDatabase);
	this->AddControl(_txtUsername);
	this->AddControl(_txtPasswordWin);
	this->AddControl(_txtDomain);
	this->AddControl(_txtLoginName);
	this->AddControl(_txtPasswordODBC);
	this->AddControl(_optSQLAuth);
	this->AddControl(_optWinAuth);
	this->AddControl(_cmdSave);
	this->AddControl(_chkSecure);
	this->AddControl(_chkTrust);
	this->AddControl(_selMsgSrc);
	this->AddControl(_selMsgDest);
	this->AddControl(_btnMsgSrc);
	this->AddControl(_btnMsgDest);
	this->AddControl(_btnRefresh);
}

TopController::~TopController(void)
{
}

int TopController::OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(dca::DialogWinController::OnMsgHandler(msg, wParam, lParam))
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			return OnInitDialog();
		case WM_COMMAND:
			return OnCommand(HIWORD(wParam), LOWORD(wParam), HWND(lParam));
		}
		
		return 1;
	}

	return 0;
}

int TopController::OnInitDialog(void)
{
	bool bRet = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
	if (bRet != true)
	{
		dca::MsgBox(_T("Unlocking Global Components failed.\r\n\r\nCertain features may not be available!")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONWARNING
						,_hwnd);
	}	
	
	// SetIcons
	HICON mainIcon = (HICON)LoadImage(_hInstance, MAKEINTRESOURCE(ICO_VMF), IMAGE_ICON, 16, 16, LR_VGACOLOR );
	SendMsg(WM_SETICON,	(WPARAM)ICON_BIG,	(LPARAM)mainIcon);
	SendMsg(WM_SETICON,	(WPARAM)ICON_SMALL, (LPARAM)mainIcon);

	dca::RegistryData mfRD(KEY_EXECUTE);

	_nUseWia = mfRD.GetUseWIA();

	mfRD.GetRunAsUser(_sUsername);
	_txtUsername.SetText(_sUsername.c_str());

	mfRD.GetRunAsPassword(_sPasswordWin);
	_txtPasswordWin.SetText(_sPasswordWin.c_str());

	mfRD.GetRunAsDomain(_sDomain);
	_txtDomain.SetText(_sDomain.c_str());

	dca::WString sDBLogin;
	mfRD.GetDBLogin(sDBLogin);

	// TODO mfRD.GetOdbcDriver(_sOdbcDriver);

	dca::WString isInstalled(L"");
	try{
		dca::LocalMachineRegKey lmrk(L"SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers", dca::RegKey::OPEN, KEY_READ);
		lmrk.QueryStringValue(L"ODBC Driver 17 for SQL Server", isInstalled);
	}
	catch(...){
	}
	
	_comboOdbcDriver.InsertString(L"SQL Server",0);
	
	if(!isInstalled.compare(L"Installed"))
	{
		_comboOdbcDriver.InsertString(L"ODBC Driver 17 for SQL Server",1);
		if(!_sOdbcDriver.compare(L"ODBC Driver 17 for SQL Server"))
		{
			_comboOdbcDriver.SetCurrentSelected(1);
		}
		else
		{
			_comboOdbcDriver.SetCurrentSelected(0);
		}
	}
	else
	{
		_comboOdbcDriver.SetCurrentSelected(0);		
	}

	//get secure
	DWORD nSecure=0;
	try{
		dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_READ);
		lmrk.QueryDWORDValue(L"OdbcSecure", nSecure);
	}
	catch(...){
	}

	int nOdbcSelected = _comboOdbcDriver.GetCurrentSelected();

	if(nOdbcSelected == 1)
	{
		if(nSecure)
		{
			_chkSecure.SetStateToChecked();
			_chkTrust.Enable();
			DWORD nTrust=0;
			try{
				dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_READ);
				lmrk.QueryDWORDValue(L"OdbcTrust", nTrust);
			}
			catch(...){
			}
			if(nTrust)
			{
				_chkTrust.SetStateToChecked();
			}
		}
		else
		{
			_chkSecure.SetStateToUnChecked();
			_chkTrust.Disable();
			_chkTrust.SetStateToUnChecked();
		}
	}
	else
	{
		_chkSecure.Disable();
		_chkSecure.SetStateToUnChecked();
		_chkTrust.Disable();
		_chkTrust.SetStateToUnChecked();
	}
	
	

	if(sDBLogin.size())
	{
		ParseDBLogin(_T("SERVER"), sDBLogin, _sServer, _txtServer);
		ParseDBLogin(_T("UID"), sDBLogin, _sLoginName, _txtLoginName);
		ParseDBLogin(_T("PWD"), sDBLogin, _sPasswordODBC, _txtPasswordODBC);
		ParseDBLogin(_T("DATABASE"), sDBLogin, _sDatabase, _txtDatabase);		
	}
	else
	{
		mfRD.GetSQLServer(_sServer);
		_txtServer.SetText(_sServer.c_str());

		mfRD.GetSQLSaUser(_sLoginName);
		_txtLoginName.SetText(_sLoginName.c_str());

		mfRD.GetSQLSaPassword(_sPasswordODBC);
		_txtPasswordODBC.SetText(_sPasswordODBC.c_str());

		mfRD.GetDBName(_sDatabase);
		_txtDatabase.SetText(_sDatabase.c_str());
	}

	if(_nUseWia)
	{
		mfRD.GetSQLSaUser(_sLoginName);
		_txtLoginName.SetText(_sLoginName.c_str());

		mfRD.GetSQLSaPassword(_sPasswordODBC);
		_txtPasswordODBC.SetText(_sPasswordODBC.c_str());

		_txtLoginName.Disable();
		_txtPasswordODBC.Disable();

		_optSQLAuth.SetStateToUnChecked();
		_optWinAuth.SetStateToChecked();

	}
	else
	{
		_txtUsername.Disable();
		_txtPasswordWin.Disable();
		_txtDomain.Disable();

		_optSQLAuth.SetStateToChecked();
		_optWinAuth.SetStateToUnChecked();
	}

	if(!_nWriteAccess)
	{
		_txtServer.Disable();
		_txtDatabase.Disable();
		_txtUsername.Disable();
		_txtPasswordWin.Disable();
		_txtDomain.Disable();
		_txtLoginName.Disable();
		_txtPasswordODBC.Disable();
		_optSQLAuth.Disable();
		_optWinAuth.Disable();
		_cmdSave.Disable();
		_comboOdbcDriver.Disable();
		_chkSecure.Disable();
		_chkTrust.Disable();
	}

	_cmdSave.Disable();

	try
	{
		CODBCQuery	m_query;	

		// get Message Sources from Database
		TMessageSources tms;
		tms.PrepareList(m_query);
		while(m_query.Fetch() == S_OK)
		{
			if(tms.m_OAuthHostID > 0)
			{
				_selMsgSrc.InsertString(tms.m_Description);
			}
		}

		// get Message Destinations from Database
		TMessageDestinations tmd;
		tmd.PrepareList(m_query);
		while(m_query.Fetch() == S_OK)
		{
			if(tmd.m_OAuthHostID > 0)
			{
				_selMsgDest.InsertString(tmd.m_Description);
			}
		}
	}
	catch(...)
	{
		::MessageBox(0,_T("Unable to get Message Sources and Message Destinations from database!"),_T("VisNetic MailFlow Admin Tool"),MB_OK);
	}

	return 1;
}

int TopController::OnSave()
{
	if(!_nWasTested)
	{
		if(OnCmdTest())
		{
			dca::MsgBox msgBox(_T("The ODBC Connection test failed proceed anyway?")
				               ,_T("VisNetic MailFlow Admin Tool")
						       ,MB_YESNO | MB_ICONQUESTION);

			if(msgBox.GetReturn() == IDYES)
			{
				SaveDataToRegistry();
			}
			else
			{
				dca::MsgBox(_T("Your changes have not been saved.")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONINFORMATION
						,_hwnd);
				_cmdSave.Enable();
				return 0;
			}
		}
		else
			SaveDataToRegistry();
	}
	else
		SaveDataToRegistry();

	_cmdSave.Disable();

	dca::MsgBox(_T("Your changes have been saved.\r\n\r\nPlease restart the World Wide Web Publishing\r\nand Visnetic MailFlow Engine services.")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONINFORMATION
						,_hwnd);

	return 1;
}

int TopController::OnCommand(int nCodeNotify, int nID, HWND hwnd)
{
	switch(nID)
	{
	case CMD_EXIT:
		return OnClose();
	case OPT_SQL_AUTH:
		return OnOptSQLAuth();
	case OPT_WIN_AUTH:
		return OnOptWinAuth();
	case BTN_MSG_SRC:
		return OnBtnMsgSrc();
	case BTN_MSG_DEST:
		return OnBtnMsgDest();
	case BTN_REFRESH:
		return OnBtnRefresh();
	case CMD_TEST:
		return OnCmdTest();
	case CMD_SAVE:
		return OnSave();
	case CMD_SETTINGS_FORCEHTTPSOPTION:
		return OnSettingsForceHttpsOption();
	case CMD_SETTINGS_ADMINISTRATORAGENTPASSWORD:
		return OnSettingsAdminPassword();
	case CMD_SETTINGS_DISK:
		return OnSettingsMinDisk();
	case CMD_HELP_ABOUT:
		return OnHelpAbout();
	case COMBO_ODBC:
		switch(nCodeNotify)
		{
		case CBN_SELCHANGE:
			return OnComboOdbcChange();
		}
	case CHK_SECURE:
		return OnChkSecureChange();
	case CHK_TRUST:
		return OnChkTrustChange();
	case TXT_SERVER:
		switch(nCodeNotify)
		{
		case EN_CHANGE:
			return OnTxtServerChange();
		}
	case TXT_DATABASE:
		switch(nCodeNotify)
		{
		case EN_CHANGE:
			return OnTxtDatabaseChange();
		}
	case TXT_USERNAME:
		switch(nCodeNotify)
		{
		case EN_CHANGE:
			return OnTxtUserNameChange();
		}
	case TXT_PASSWORD_WIN:
		switch(nCodeNotify)
		{
		case EN_CHANGE:
			return OnTxtPasswordWinChange();
		}
	case TXT_DOMAIN:
		switch(nCodeNotify)
		{
		case EN_CHANGE:
			return OnTxtDomainChange();
		}
	case TXT_LOGIN_NAME:
		switch(nCodeNotify)
		{
		case EN_CHANGE:
			return OnTxtLoginNameChange();
		}
	case TXT_PASSWORD_ODBC:
		switch(nCodeNotify)
		{
		case EN_CHANGE:
			return OnTxtPasswordODBCChange();
		}
	}

	return 0;
}

void TopController::ParseDBLogin(LPCTSTR cspValue, dca::WString& sDBLogin, dca::WString& sData, dca::BaseControl& rControl)
{
	dca::WString::size_type nPos = sDBLogin.find(cspValue);

	if(nPos != dca::WString::npos)
	{
		nPos += lstrlen(cspValue) + 1;
		dca::WString sTemp = sDBLogin.substr(nPos);

		nPos = sTemp.find(';');

		sData = sTemp.substr(0,nPos);

		rControl.SetText(sData.c_str());
	}
}

int TopController::OnOptSQLAuth()
{
	_txtLoginName.Enable();
	_txtPasswordODBC.Enable();

	_txtUsername.Disable();
	_txtPasswordWin.Disable();
	_txtDomain.Disable();

	_nUseWia = 0;

	_cmdSave.Enable();
	_nWasTested = 0;

	return 0;
}

int TopController::OnOptWinAuth()
{
	_txtLoginName.Disable();
	_txtPasswordODBC.Disable();

	_txtUsername.Enable();
	_txtPasswordWin.Enable();
	_txtDomain.Enable();

	_nUseWia = 1;

	_cmdSave.Enable();
	_nWasTested = 0;

	return 0;
}

int TopController::OnCmdTest()
{
	GetComboValue(_comboOdbcDriver, _sOdbcDriver);
	dca::RegistryData mfRD(KEY_WRITE);
	// TODO mfRD.SetOdbcDriver(_sOdbcDriver);

	UINT nSecure = _chkSecure.GetCheckedState();
	UINT nTrust = _chkTrust.GetCheckedState();

	try{
		dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_WRITE);
		lmrk.SetDWORDValue(L"OdbcSecure", nSecure);
	}
	catch(...){
	}


	try{
		dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_WRITE);
		lmrk.SetDWORDValue(L"OdbcTrust", nTrust);
	}
	catch(...){
	}
	
	_nWasTested = 1;

	try
	{
		if(_nUseWia)
		{
			if(_sUsername.length() == 0)
			{
				dca::MsgBox(_T("You must specify a username.")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONINFORMATION
						,_hwnd);
				return 1;
			}
			else if(_sPasswordWin.length() == 0)
			{
				dca::MsgBox(_T("You must specify a password.")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONINFORMATION
						,_hwnd);
				return 1;
			}
		}
		else
		{
			if(_sLoginName.length() == 0)
			{
				dca::MsgBox(_T("You must specify a username.")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONINFORMATION
						,_hwnd);
				return 1;
			}
			else if(_sPasswordODBC.length() == 0)
			{
				dca::MsgBox(_T("You must specify a password.")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONINFORMATION
						,_hwnd);
				return 1;
			}
		}
		
		dca::SmartComObject comObjMFAdmin(CLSID_RoutingEngineAdmin);
		dca::SmartComInterface<IRoutingEngineAdmin, IID_IRoutingEngineAdmin> iMFAdmin(comObjMFAdmin);

		dca::BString bServer(_sServer.c_str());
		dca::BString bDatabase(_sDatabase.c_str());
		dca::BString bName((_nUseWia) ? _sUsername.c_str() : _sLoginName.c_str());
		dca::BString bPassword((_nUseWia) ? _sPasswordWin.c_str() : _sPasswordODBC.c_str());
		dca::BString bDomain(_sDomain.c_str());
		dca::BString bError("success");

		long lResult= 0;

		iMFAdmin->TestODBC(bServer
						,_nUseWia
						,bName
						,bPassword
						,bDomain
						,bDatabase
						,bError.GetRef()
						,&lResult);

		
		
		if(lResult == 0)
		{
			dca::WString sResult(_T("The ODBC Connection test failed:\n\n"));
			sResult.append((LPCTSTR)(BSTR)bError);
			dca::MsgBox(sResult.c_str()
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONSTOP);
			_nWasTested = 0;
			_cmdSave.Disable();
			return 1;
		}
		else
		{
			dca::MsgBox(_T("The ODBC Connection test passed.")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONINFORMATION
						,_hwnd);
		}
	}
	catch(dca::Exception e)  // Catch any DCA library exceptions
	{
		::MessageBox(0,e.GetMessage(),_T("VisNetic MailFlow Admin Tool"),MB_OK  | MB_ICONERROR);
		_nWasTested = 0;
		return 1;
	}
	catch(...)
	{
		dca::MsgBox(_T("The ODBC Connection test failed with an unknown error.")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONERROR
						,_hwnd);
	}

	return 0;
}

int TopController::OnComboOdbcChange()
{
	dca::WString sTemp;
	GetComboValue(_comboOdbcDriver, sTemp);
	if(_sOdbcDriver.compare(sTemp))
	{
		_sOdbcDriver = sTemp;
		_nWasTested = 0;
		_cmdSave.Enable();

		int nOdbcSelected = _comboOdbcDriver.GetCurrentSelected();
		if(nOdbcSelected == 1)
		{
			DWORD nSecure=0;
			try{
				dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_READ);
				lmrk.QueryDWORDValue(L"OdbcSecure", nSecure);
			}
			catch(...){
			}
			
			if(nSecure)
			{
				_chkSecure.Enable();
				_chkSecure.SetStateToChecked();
				_chkTrust.Enable();
				DWORD nTrust=0;
				try{
					dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_READ);
					lmrk.QueryDWORDValue(L"OdbcTrust", nTrust);
				}
				catch(...){
				}
				if(nTrust)
				{
					_chkTrust.SetStateToChecked();
				}
			}
			else
			{
				_chkSecure.Enable();
				_chkSecure.SetStateToUnChecked();
				_chkTrust.Disable();
				_chkTrust.SetStateToUnChecked();
			}
		}
		else
		{
			_chkSecure.Disable();
			_chkSecure.SetStateToUnChecked();
			_chkTrust.Disable();
			_chkTrust.SetStateToUnChecked();
		}
	}

	return 0;
}

int TopController::OnChkSecureChange()
{
	UINT nSecure = _chkSecure.GetCheckedState();

	if(nSecure)
	{
		_chkTrust.Enable();
		_chkTrust.SetStateToChecked();
	}
	else
	{
		_chkTrust.Disable();
		_chkTrust.SetStateToUnChecked();
	}
	
	_cmdSave.Enable();
	_nWasTested = 0;

	return 0;
}

int TopController::OnChkTrustChange()
{
	_cmdSave.Enable();
	_nWasTested = 0;

	return 0;
}

int TopController::OnTxtServerChange()
{
	dca::WString sTemp;
	GetTextBoxValue(_txtServer, sTemp);
	if(_sServer.compare(sTemp))
	{
		_sServer = sTemp;
		_nWasTested = 0;
		_cmdSave.Enable();
	}

	return 0;
}

int TopController::OnTxtDatabaseChange()
{
	dca::WString sTemp;
	GetTextBoxValue(_txtDatabase, sTemp);
	if(_sDatabase.compare(sTemp))
	{
		_sDatabase = sTemp;
		_nWasTested = 0;
		_cmdSave.Enable();
	}

	return 0;
}

int TopController::OnTxtUserNameChange()
{
	dca::WString sTemp;
	GetTextBoxValue(_txtUsername, sTemp);
	if(_sUsername.compare(sTemp))
	{
		_sUsername = sTemp;
		_nWasTested = 0;
		_cmdSave.Enable();
	}

	return 0;
}

int TopController::OnTxtPasswordWinChange()
{
	dca::WString sTemp;
	GetTextBoxValue(_txtPasswordWin, sTemp);
	if(_sPasswordWin.compare(sTemp))
	{
		_sPasswordWin = sTemp;
		_nWasTested = 0;
		_cmdSave.Enable();
	}

	return 0;
}

int TopController::OnTxtDomainChange()
{
	dca::WString sTemp;
	GetTextBoxValue(_txtDomain, sTemp);
	if(_sDomain.compare(sTemp))
	{
		_sDomain = sTemp;
		_nWasTested = 0;
		_cmdSave.Enable();
	}

	return 0;
}

int TopController::OnTxtLoginNameChange()
{
	dca::WString sTemp;
	GetTextBoxValue(_txtLoginName, sTemp);
	if(_sLoginName.compare(sTemp))
	{
		_sLoginName = sTemp;
		_nWasTested = 0;
		_cmdSave.Enable();
	}

	return 0;
}

int TopController::OnTxtPasswordODBCChange()
{
	dca::WString sTemp;
	GetTextBoxValue(_txtPasswordODBC, sTemp);
	if(_sPasswordODBC.compare(sTemp))
	{
		_sPasswordODBC = sTemp;
		_nWasTested = 0;
		_cmdSave.Enable();
	}

	return 0;
}

void TopController::GetTextBoxValue(dca::BaseControl& rControl, dca::WString& sValue)
{
	dca::WString sTemp;
	rControl.GetText(sTemp);

	if(sValue.compare(sTemp))
	{
		sValue = sTemp;		
	}
}

void TopController::GetComboValue(dca::ComboBox& rCombo, dca::WString& sValue)
{
	dca::WString sTemp;
	int nSelected = rCombo.GetCurrentSelected();
	rCombo.GetListBoxText(nSelected, sTemp);
	
	if(sValue.compare(sTemp))
	{
		sValue = sTemp;		
	}
}

void TopController::SaveDataToRegistry()
{
	dca::RegistryData mfRD(KEY_WRITE);

	// TODO mfRD.SetOdbcDriver(_sOdbcDriver);
	mfRD.SetSQLServer(_sServer);
	mfRD.SetDBName(_sDatabase);
	mfRD.SetSQLSaUser(_sLoginName);
	mfRD.SetSQLSaPassword(_sPasswordODBC);
	mfRD.SetRunAsUser(_sUsername);
	mfRD.SetRunAsPassword(_sPasswordWin);
	mfRD.SetRunAsDomain(_sDomain);
	mfRD.SetUseWIA(_nUseWia);

	UINT nSecure = _chkSecure.GetCheckedState();
	UINT nTrust = _chkTrust.GetCheckedState();

	try{
		dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_WRITE);
		lmrk.SetDWORDValue(L"OdbcSecure", nSecure);
	}
	catch(...){
	}


	try{
		dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_WRITE);
		lmrk.SetDWORDValue(L"OdbcTrust", nTrust);
	}
	catch(...){
	}

	dca::WString dbLogin;

	if(_nUseWia)
	{
		if(nSecure && nTrust)
		{
			dbLogin.Format(_T("DRIVER={%s};SERVER=%s;DATABASE=%s;Trusted_Connection=yes;Encrypt=yes;TrustServerCertificate=yes;")
			           ,_sOdbcDriver.c_str()
					   ,_sServer.c_str()
					   ,_sDatabase.c_str());
		}
		else if(nSecure)
		{
			dbLogin.Format(_T("DRIVER={%s};SERVER=%s;DATABASE=%s;Trusted_Connection=yes;Encrypt=yes;")
			           ,_sOdbcDriver.c_str()
					   ,_sServer.c_str()
					   ,_sDatabase.c_str());

		}
		else
		{
			dbLogin.Format(_T("DRIVER={%s};SERVER=%s;DATABASE=%s;Trusted_Connection=yes;")
			           ,_sOdbcDriver.c_str()
					   ,_sServer.c_str()
					   ,_sDatabase.c_str());
		}
	}
	else
	{
		if(nSecure && nTrust)
		{
			dbLogin.Format(_T("DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;Encrypt=yes;TrustServerCertificate=yes;")
					,_sOdbcDriver.c_str()
					,_sServer.c_str()
					,_sLoginName.c_str()
					,_sPasswordODBC.c_str()
					,_sDatabase.c_str());
		}
		else if(nSecure)
		{
			dbLogin.Format(_T("DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;Encrypt=yes;")
					,_sOdbcDriver.c_str()
					,_sServer.c_str()
					,_sLoginName.c_str()
					,_sPasswordODBC.c_str()
					,_sDatabase.c_str());

		}
		else
		{
			dbLogin.Format(_T("DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;")
					,_sOdbcDriver.c_str()
					,_sServer.c_str()
					,_sLoginName.c_str()
					,_sPasswordODBC.c_str()
					,_sDatabase.c_str());
		}
	}

	mfRD.SetDBLogin(dbLogin);

	pApp->ReloadMailFlow();
}

int TopController::OnSettingsForceHttpsOption()
{
	int nUseHttps = 0;

	{
		dca::SmartComObject comObjMFAdmin(CLSID_RoutingEngineAdmin);
		dca::SmartComInterface<IRoutingEngineAdmin, IID_IRoutingEngineAdmin> iMFAdmin(comObjMFAdmin);

		dca::BString bReturn;
		long lReturn;

		iMFAdmin->GetServerParameter(25, bReturn.GetRef(), &lReturn);

		nUseHttps = _wtoi((BSTR)bReturn);
	}

	{
		ForceHTTPSController theController(nUseHttps);
		dca::ModalDialog dlgForceHttps;

		dlgForceHttps.Create(_hInstance,
							 DLG_FORCEHTTPS,
							&theController,
							_hwnd);

		if(!dlgForceHttps.GetReturn())
			return 1;

		nUseHttps = theController.GetForceHttps();
	}

	{
		dca::SmartComObject comObjMFAdmin(CLSID_RoutingEngineAdmin);
		dca::SmartComInterface<IRoutingEngineAdmin, IID_IRoutingEngineAdmin> iMFAdmin(comObjMFAdmin);

		wchar_t temp[80];
		_itow(nUseHttps, temp, 10);

		dca::BString bReturn(temp);
		long lReturn;
		iMFAdmin->SetServerParameter(25, bReturn, &lReturn);
	}

	return 0;
}

int TopController::OnSettingsAdminPassword()
{
	AdminPwdController theController;
	dca::ModalDialog dlgAdminPwd;

	dlgAdminPwd.Create(_hInstance,
		               DLG_ADMIN_PASSWD,
					   &theController,
					   _hwnd);

	if(!dlgAdminPwd.GetReturn())
			return 1;

	{
		dca::SmartComObject comObjMFAdmin(CLSID_RoutingEngineAdmin);
		dca::SmartComInterface<IRoutingEngineAdmin, IID_IRoutingEngineAdmin> iMFAdmin(comObjMFAdmin);

		dca::BString bUser(_T("Administrator"));
		dca::BString bPassword(theController.GetNewPassword());
		long lReturn;

		iMFAdmin->SetPassword(bUser, bPassword, &lReturn);

		if(lReturn)
			dca::MsgBox(_T("Error setting administrator agent password")
			            ,_T("VisNetic MailFlow Admin Tool"));
		else
			dca::MsgBox(_T("Adminstrator password changed")
			            ,_T("VisNetic MailFlow Admin Tool"));
	}

	return 0;
}

int TopController::OnSettingsMinDisk()
{
	MinMBSizeController theController;
	dca::ModalDialog dlgMinSize;

	dlgMinSize.Create(_hInstance,
		               DLG_MIN_MB_SIZE,
					   &theController,
					  _hwnd);

	return 0;
}

int TopController::OnHelpAbout()
{
	AboutController dbCtrl;
	dca::ModalDialog dlgAbout;

	dlgAbout.Create(_hInstance,
		            DLG_ABOUT,
					&dbCtrl,
					_hwnd);

	return 0;
}

int TopController::OnBtnMsgSrc()
{
	USES_CONVERSION;
	dca::WString sMsgSrc;
	_selMsgSrc.GetText(sMsgSrc);

	int status = m_glob.get_UnlockStatus();
    if (status != 2)
	{
        dca::MsgBox(_T("Unlocking Global Components failed.\r\n\r\nThis feature is not available!")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONWARNING
						,_hwnd);
		return 1;
    }
    
	CODBCQuery	m_query;	

	int nMsgSrcID;
	TMessageSources tms;
	tms.PrepareList(m_query);
	while(m_query.Fetch() == S_OK)
	{
		if(tms.m_OAuthHostID > 0)
		{
			if ( _tcscmp( tms.m_Description, sMsgSrc.c_str() ) == 0 )
			{
				nMsgSrcID = tms.m_MessageSourceID;
				break;
			}
		}
	}

	tms.m_MessageSourceID = nMsgSrcID;
	tms.Query(m_query);
	TOAuthHosts toh;
	toh.m_OAuthHostID = tms.m_OAuthHostID;
	toh.Query(m_query);

	CEMSString sMsg;
	sMsg.Format(_T("A browswer window will open to the OAuth2 provider login screen.\r\n\r\nLogin with the username [%s] then authorize access."),tms.m_AuthUserName);
	dca::MsgBox(sMsg.c_str()
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONINFORMATION
						,_hwnd);

	CkOAuth2 oauth2;    
    oauth2.put_ListenPort(49546);
    oauth2.put_AuthorizationEndpoint(T2A(toh.m_AuthEndPoint));
    oauth2.put_TokenEndpoint(T2A(toh.m_TokenEndPoint));
    oauth2.put_ClientId(T2A(toh.m_ClientID));
    oauth2.put_ClientSecret(T2A(toh.m_ClientSecret));
    oauth2.put_CodeChallenge(false);
	if(tms.m_MessageSourceTypeID == 1)
	{
		oauth2.put_Scope("openid profile offline_access https://outlook.office365.com/POP.AccessAsUser.All");
	}
	else
	{
		oauth2.put_Scope("openid profile offline_access https://outlook.office365.com/IMAP.AccessAsUser.All");
	}
    oauth2.put_RedirectAllowHtml("OAuth2 Token request was successful, you can close this browser window.");
	oauth2.put_RedirectDenyHtml("Auth2 Token request was denied, you can close this browser window.");

    const char *url = oauth2.startAuth();
    if (oauth2.get_LastMethodSuccess() != true)
	{
        dca::MsgBox(_T("An error occurred during Token request!")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONWARNING
						,_hwnd);
		return 1;
    }

	ShellExecuteA(0, 0, url, 0, 0 , SW_SHOW );

	int numMsWaited = 0;
    while ((numMsWaited < 120000) && (oauth2.get_AuthFlowState() < 3)) {
        oauth2.SleepMs(100);
        numMsWaited = numMsWaited + 100;
    }

    if (oauth2.get_AuthFlowState() < 3) {
        oauth2.Cancel();
		dca::MsgBox(_T("No response from the browser!")
							,_T("VisNetic MailFlow Admin Tool")
							,MB_OK | MB_ICONWARNING
							,_hwnd);
		return 1;
    }

    if (oauth2.get_AuthFlowState() == 5) {
        dca::MsgBox(_T("OAuth2 failed to complete!")
							,_T("VisNetic MailFlow Admin Tool")
							,MB_OK | MB_ICONWARNING
							,_hwnd);
        return 1;
    }

    if (oauth2.get_AuthFlowState() == 4) {
        dca::MsgBox(_T("OAuth2 authorization was denied!")
							,_T("VisNetic MailFlow Admin Tool")
							,MB_OK | MB_ICONWARNING
							,_hwnd);
        return 1;
    }

    if (oauth2.get_AuthFlowState() != 3) {
        dca::MsgBox(_T("Unexpected error occurred!")
							,_T("VisNetic MailFlow Admin Tool")
							,MB_OK | MB_ICONWARNING
							,_hwnd);
        return 1;
    }

	// Update the AccessToken and RefreshToken
	tstring aToken(A2T(oauth2.accessToken()));	
	tstring rToken(A2T(oauth2.refreshToken()));
			
	time_t now;
	time( &now ); 
	TIMESTAMP_STRUCT tsaTokenExpire;
	long tsaTokenExpireLen=0;
	SecondsToTimeStamp(now + 3599, tsaTokenExpire);

	m_query.Initialize();
	BINDPARAM_TEXT_STRING(m_query, aToken);
	BINDPARAM_TEXT_STRING(m_query, rToken);
	BINDPARAM_TIME (m_query, tsaTokenExpire);
	BINDPARAM_LONG(m_query, tms.m_MessageSourceID);
	m_query.Execute(	_T("UPDATE MessageSources SET AccessToken=?,RefreshToken=?,AccessTokenExpire=? WHERE MessageSourceID=?"));	
	
	dca::MsgBox(_T("OAuth2 authorization was successful!")
							,_T("VisNetic MailFlow Admin Tool")
							,MB_OK | MB_ICONINFORMATION
							,_hwnd);

	return 0;
}


int TopController::OnBtnMsgDest()
{
	USES_CONVERSION;
	dca::WString sMsgDest;
	_selMsgDest.GetText(sMsgDest);

	int status = m_glob.get_UnlockStatus();
    if (status != 2)
	{
        dca::MsgBox(_T("Unlocking Global Components failed.\r\n\r\nThis feature is not available!")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONWARNING
						,_hwnd);
		return 1;
    }
    
	CODBCQuery	m_query;	

	int nMsgDestID;
	TMessageDestinations tmd;
	tmd.PrepareList(m_query);
	while(m_query.Fetch() == S_OK)
	{
		if(tmd.m_OAuthHostID > 0)
		{
			if ( _tcscmp( tmd.m_Description, sMsgDest.c_str() ) == 0 )
			{
				nMsgDestID = tmd.m_MessageDestinationID;
				break;
			}
		}
	}

	tmd.m_MessageDestinationID = nMsgDestID;
	tmd.Query(m_query);
	TOAuthHosts toh;
	toh.m_OAuthHostID = tmd.m_OAuthHostID;
	toh.Query(m_query);

	CEMSString sMsg;
	sMsg.Format(_T("A browswer window will open to the OAuth2 provider login screen.\r\n\r\nLogin with the username [%s] then authorize access."),tmd.m_AuthUser);
	dca::MsgBox(sMsg.c_str()
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONINFORMATION
						,_hwnd);

	CkOAuth2 oauth2;    
    oauth2.put_ListenPort(49546);
    oauth2.put_AuthorizationEndpoint(T2A(toh.m_AuthEndPoint));
    oauth2.put_TokenEndpoint(T2A(toh.m_TokenEndPoint));
    oauth2.put_ClientId(T2A(toh.m_ClientID));
    oauth2.put_ClientSecret(T2A(toh.m_ClientSecret));
    oauth2.put_CodeChallenge(false);
    oauth2.put_Scope("openid profile offline_access https://outlook.office365.com/SMTP.Send");
	oauth2.put_RedirectAllowHtml("OAuth2 Token request was successful, you can close this browser window.");
	oauth2.put_RedirectDenyHtml("Auth2 Token request was denied, you can close this browser window.");

    const char *url = oauth2.startAuth();
    if (oauth2.get_LastMethodSuccess() != true)
	{
        dca::MsgBox(_T("An error occurred during Token request!")
						,_T("VisNetic MailFlow Admin Tool")
						,MB_OK | MB_ICONWARNING
						,_hwnd);
		return 1;
    }

	ShellExecuteA(0, 0, url, 0, 0 , SW_SHOW );

	int numMsWaited = 0;
    while ((numMsWaited < 120000) && (oauth2.get_AuthFlowState() < 3)) {
        oauth2.SleepMs(100);
        numMsWaited = numMsWaited + 100;
    }

    if (oauth2.get_AuthFlowState() < 3) {
        oauth2.Cancel();
		dca::MsgBox(_T("No response from the browser!")
							,_T("VisNetic MailFlow Admin Tool")
							,MB_OK | MB_ICONWARNING
							,_hwnd);
		return 1;
    }

    if (oauth2.get_AuthFlowState() == 5) {
        dca::MsgBox(_T("OAuth2 failed to complete!")
							,_T("VisNetic MailFlow Admin Tool")
							,MB_OK | MB_ICONWARNING
							,_hwnd);
        return 1;
    }

    if (oauth2.get_AuthFlowState() == 4) {
        dca::MsgBox(_T("OAuth2 authorization was denied!")
							,_T("VisNetic MailFlow Admin Tool")
							,MB_OK | MB_ICONWARNING
							,_hwnd);
        return 1;
    }

    if (oauth2.get_AuthFlowState() != 3) {
        dca::MsgBox(_T("Unexpected error occurred!")
							,_T("VisNetic MailFlow Admin Tool")
							,MB_OK | MB_ICONWARNING
							,_hwnd);
        return 1;
    }

	// Update the AccessToken and RefreshToken
	tstring aToken;	
	tstring rToken;
	
	aToken.assign(A2T(oauth2.accessToken()));
	rToken.assign(A2T(oauth2.refreshToken()));
	
	time_t now;
	time( &now ); 
	TIMESTAMP_STRUCT tsaTokenExpire;
	long tsaTokenExpireLen=0;
	SecondsToTimeStamp(now + 3599, tsaTokenExpire);

	m_query.Initialize();
	BINDPARAM_TEXT_STRING(m_query, aToken);
	BINDPARAM_TEXT_STRING(m_query, rToken);
	BINDPARAM_TIME (m_query, tsaTokenExpire);
	BINDPARAM_LONG(m_query, tmd.m_MessageDestinationID);
	m_query.Execute(	_T("UPDATE MessageDestinations SET AccessToken=?,RefreshToken=?,AccessTokenExpire=? WHERE MessageDestinationID=?"));	
	
	dca::MsgBox(_T("OAuth2 authorization was successful!")
							,_T("VisNetic MailFlow Admin Tool")
							,MB_OK | MB_ICONINFORMATION
							,_hwnd);

	return 0;
}

int TopController::OnBtnRefresh()
{
	CODBCQuery	m_query;	

	// get Message Sources from Database
	TMessageSources tms;
	tms.PrepareList(m_query);
	_selMsgSrc.SendMsg(CB_RESETCONTENT);
	
	while(m_query.Fetch() == S_OK)
	{
		if(tms.m_OAuthHostID > 0)
		{
			_selMsgSrc.InsertString(tms.m_Description);
		}
	}

	// get Message Destinations from Database
	TMessageDestinations tmd;
	tmd.PrepareList(m_query);
	_selMsgDest.SendMsg(CB_RESETCONTENT);
	while(m_query.Fetch() == S_OK)
	{
		if(tmd.m_OAuthHostID > 0)
		{
			_selMsgDest.InsertString(tmd.m_Description);
		}
	}

	return 0;
}