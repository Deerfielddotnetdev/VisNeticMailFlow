#pragma once

class MyContacts : public CXMLDataClass
{
public:
	MyContacts(CISAPIData& ISAPIData);
	virtual ~MyContacts(void);

	virtual int Run(CURLAction& action);

protected:
	void AddToContacts(CURLAction& action);
	void ListContacts(CURLAction& action);
	void DeleteFromContacts(CURLAction& action);

protected:
	int _AgentID;
};
