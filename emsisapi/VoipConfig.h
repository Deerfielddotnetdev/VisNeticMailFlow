#pragma once

class VoipConfig : public CXMLDataClass
{
public:
	VoipConfig(CISAPIData& ISAPIData);
	~VoipConfig(void);
	virtual int Run(CURLAction& action);
	int GetSettings(CURLAction& action);
	void UpdateSettings(void);
protected:
	void SetXMLOutput(int nID, LPCTSTR lpDefault, LPCTSTR lpChildElem);
	void SetDatabaseSettings(int nID, std::string& sDataValue);
};
