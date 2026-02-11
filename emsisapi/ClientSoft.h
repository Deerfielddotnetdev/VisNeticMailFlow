#pragma once

#include "XMLDataClass.h"

class CClientSoft : public CXMLDataClass  
{
public:
	CClientSoft(CISAPIData& ISAPIData);
	virtual ~CClientSoft();

	int Run(CURLAction& action);
	
	void ListSoft(void);
	void AddSoft(void);
	void DeleteSoft(void);

private:
	
};
