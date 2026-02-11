#include "stdafx.h"
#include ".\VoipConfig.h"
#include ".\MailFlowServer.h"

VoipConfig::VoipConfig(CISAPIData& ISAPIData)
	:CXMLDataClass(ISAPIData)
{
}

VoipConfig::~VoipConfig(void)
{
}

int VoipConfig::Run(CURLAction& action)
{
	RequireAdmin();

	std::string sAction;
	GetISAPIData().GetFormString("action", sAction, true);

	if(!lstrcmpi(sAction.c_str(),"update"))
	{
		UpdateSettings();
	}

	return GetSettings(action);
}

int VoipConfig::GetSettings(CURLAction& action)
{
	GetXMLGen().AddChildElem(_T("customsettings"));
	SetXMLOutput(68, _T("0"), _T("voipintegration"));

	return 0;
}

void VoipConfig::UpdateSettings(void)
{
	std::string sEnableVoipIntegration;
	if(!GetISAPIData().GetFormString("chkEnableVoipIntegration",sEnableVoipIntegration, true))
		sEnableVoipIntegration.assign(_T("off"));

	if(!sEnableVoipIntegration.compare(_T("off")))
		sEnableVoipIntegration.assign(_T("0"));
	else
		sEnableVoipIntegration.assign(_T("1"));

	SetDatabaseSettings(68, sEnableVoipIntegration);
	
	InvalidateServerParameters(true);
}

void VoipConfig::SetXMLOutput(int nID, LPCTSTR lpDefault, LPCTSTR lpChildElem)
{
	TServerParameters servParams;
	servParams.m_ServerParameterID = nID;

	int nResult = servParams.Query(GetQuery());

	if(nResult != S_OK)
		lstrcpy(servParams.m_DataValue, lpDefault);

	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(lpChildElem, servParams.m_DataValue);
	GetXMLGen().OutOfElem();
}

void VoipConfig::SetDatabaseSettings(int nID, std::string& sDataValue)
{
	int m_ServerParameterID = nID;
	long m_ServerParameterIDLen = 0;
	TCHAR m_DataValue[SERVERPARAMETERS_DATAVALUE_LENGTH];
	long m_DataValueLen = 0;

	ZeroMemory(m_DataValue, SERVERPARAMETERS_DATAVALUE_LENGTH);

	lstrcpyn(m_DataValue, sDataValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1);

	GetQuery().Initialize();
	BINDPARAM_TCHAR(GetQuery(), m_DataValue );
	BINDPARAM_LONG(GetQuery(), m_ServerParameterID );
	GetQuery().Execute( _T("UPDATE ServerParameters ")
	                    _T("SET DataValue=? ")
	                    _T("WHERE ServerParameterID=?") );
}
