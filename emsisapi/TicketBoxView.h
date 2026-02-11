#pragma once

// base class
#include "XMLDataClass.h"
#include "SessionMap.h"

struct CUSTOMTICKETBOXVIEW_STRUCT
{
	dca::String55  _sName;
	long           _sNameLen;
	int            _nID;
	int			   _nAgentBoxID;
	int			   _nTypeID;
	unsigned char  _ucShowClosedItems;

	CUSTOMTICKETBOXVIEW_STRUCT()
	{
		ZeroMemory(this, sizeof(CUSTOMTICKETBOXVIEW_STRUCT));
	}

	CUSTOMTICKETBOXVIEW_STRUCT(const CUSTOMTICKETBOXVIEW_STRUCT& ctbvs)
	{
		lstrcpyn(_sName, ctbvs._sName, 55);
		_sNameLen = ctbvs._sNameLen;
		_nID = ctbvs._nID;
		_nAgentBoxID = ctbvs._nAgentBoxID;
		_nTypeID = ctbvs._nTypeID;
		_ucShowClosedItems = ctbvs._ucShowClosedItems;
	}

	CUSTOMTICKETBOXVIEW_STRUCT operator=(const CUSTOMTICKETBOXVIEW_STRUCT& ctbvs)
	{
		lstrcpyn(_sName, ctbvs._sName, 55);
		_sNameLen = ctbvs._sNameLen;
		_nID = ctbvs._nID;
		_nAgentBoxID = ctbvs._nAgentBoxID;
		_nTypeID = ctbvs._nTypeID;
		_ucShowClosedItems = ctbvs._ucShowClosedItems;

		return (*this);
	}
};

class CTicketBoxView : public CXMLDataClass
{
public:
	
	CTicketBoxView(CISAPIData& ISAPIData)
	:CXMLDataClass(ISAPIData)
	,_sEnableMyTickets("1")
	,_sEnablePublicTicketBoxes("1")
	,_sEnableAgentViews("1")
	{};
	virtual ~CTicketBoxView() {};

public:
	virtual int Run( CURLAction& action );

private:
	void List();
	void ListPrivateViews();
	void ListPublicViews();
	void ListAgentStatus();
protected:
	int GetUnreadMessageCountPrivate(int nAgentID, unsigned int nShowClosed, int nFolderID = 0);
	int GetUnreadMessageCountPublic(int nTicketBoxID, unsigned int nShowClosed, unsigned int nShowOwned, unsigned int nUnreadMode);
	void ListMyFolders(int nAgentID);
protected:
	dca::String _sEnableMyTickets;
	dca::String _sEnablePublicTicketBoxes;
	dca::String _sEnableAgentViews;	
};
