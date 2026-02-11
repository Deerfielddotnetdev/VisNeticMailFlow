#pragma once

class OfficeHours : public CXMLDataClass
{
public:
	OfficeHours(CISAPIData& ISAPIData);
	~OfficeHours(void);
	
	virtual int Run(CURLAction& action);	
protected:
	int GetSettings(CURLAction& action);
	void UpdateSettings(void);	
	bool IsTimeLater(TOfficeHours oh);

	int nTypeID;
	int nActualID;
	int nUseDefault;
};
