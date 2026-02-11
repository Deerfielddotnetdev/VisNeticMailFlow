#pragma once

class CustomSettings : public CXMLDataClass
{
public:
	CustomSettings(CISAPIData& ISAPIData);
	~CustomSettings(void);
	virtual int Run(CURLAction& action);
	int GetSettings(CURLAction& action);
	int BulkSettings(CURLAction& action);
	void UpdateSettings(void);
	void UpdateCTAS(void);
	void UpdateRRTM(void);
	int ApplyAgent(void);
	int ApplyTicketBox(void);
	int ApplyRoutingRule(void);
	int ApplyMessageSource(void);
	int ApplyMessageDestination(void);
	void AddException(void);
	void DeleteException(void);
protected:
	void SetXMLOutput(int nID, LPCTSTR lpDefault, LPCTSTR lpChildElem);
	void SetDatabaseSettings(int nID, std::string& sDataValue);
	bool IsTimeLater(TOfficeHours oh);
	vector<TBypass> m_b;
	vector<TBypass>::iterator mbIter;
	TBypass b;
	int nMaxBypassID;
	int nMaxEmailID;
};
