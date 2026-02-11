//******************************************************************************
// $Author: markm $
// $Date: 2005/09/28 14:48:08 $
// $Name:  $
// $Revision: 1.8 $
// $Source: c:/cvsrepo/mailflow/vmfadmintool/vmfadmintool/TopController.h,v $
//******************************************************************************

#pragma once

#include <CkGlobal.h>
#include <CkOAuth2.h>
#include <CkString.h>

//******************************************************************************
// TopController class

class TopController : public dca::DialogWinController
{
public:
	TopController(int nWriteAccess);
	~TopController(void);
	virtual int OnMsgHandler(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	int OnInitDialog(void);
	int OnCommand(int nCodeNotify, int nID, HWND hwnd);
	int OnOptSQLAuth();
	int OnOptWinAuth();
	int OnCmdTest();
	int OnSave();
	int OnComboOdbcChange();
	int OnChkSecureChange();
	int OnChkTrustChange();
	int OnTxtServerChange();
	int OnTxtDatabaseChange();
	int OnTxtUserNameChange();
	int OnTxtPasswordWinChange();
	int OnTxtDomainChange();
	int OnTxtLoginNameChange();
	int OnTxtPasswordODBCChange();
	int OnSettingsForceHttpsOption();
	int OnSettingsAdminPassword();
	int OnSettingsMinDisk();
	int OnHelpAbout();
	void ParseDBLogin(LPCTSTR cspValue, dca::WString& sDBLogin, dca::WString& sData, dca::BaseControl& rControl);
	void GetTextBoxValue(dca::BaseControl& rControl, dca::WString& sValue);
	void GetComboValue(dca::ComboBox& rCombo, dca::WString& sValue);
	void SaveDataToRegistry();
	int OnBtnMsgSrc();
	int OnBtnMsgDest();
	int OnBtnRefresh();
protected:
	int			 _nWriteAccess;
	int			 _nWasTested;
	int          _nUseWia;
	dca::TextBox _txtServer;
	dca::TextBox _txtDatabase;
	dca::TextBox _txtUsername;
	dca::TextBox _txtPasswordWin;
	dca::TextBox _txtDomain;
	dca::TextBox _txtLoginName;
	dca::TextBox _txtPasswordODBC;
	dca::ComboBox _comboOdbcDriver;
	dca::Button  _chkSecure;
	dca::Button  _chkTrust;
	dca::Button  _optSQLAuth;
	dca::Button  _optWinAuth;
	dca::Button  _cmdSave;
	dca::WString _sOdbcDriver;
	dca::WString _sServer;
	dca::WString _sDatabase;
	dca::WString _sUsername;
	dca::WString _sPasswordWin;
	dca::WString _sDomain;
	dca::WString _sLoginName;
	dca::WString _sPasswordODBC;
	dca::Button  _btnMsgSrc;
	dca::Button  _btnMsgDest;
	dca::ComboBox _selMsgSrc;
	dca::ComboBox _selMsgDest;
	dca::Button	 _btnRefresh;
	CkGlobal m_glob;
};

//******************************************************************************
