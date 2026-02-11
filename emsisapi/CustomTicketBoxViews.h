#pragma once

class CustomTicketBoxViews : public CXMLDataClass
{
public:
	CustomTicketBoxViews(CISAPIData& ISAPIData);
	virtual ~CustomTicketBoxViews(void);

	virtual int Run(CURLAction& action);

protected:
	void AddToFolders(CURLAction& action);
	void ListFolders(CURLAction& action);
	void DeleteFromFolders(CURLAction& action);

protected:
	int _AgentID;
};
