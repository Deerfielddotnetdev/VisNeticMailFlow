// AlertList.h: interface for the CAlertList class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PagedList.h"
#include "TBVInfo.h"

class CAlertList :  public CPagedList 
{
public:
	CAlertList(CISAPIData& ISAPIData);
	virtual ~CAlertList() {};

	int Run(CURLAction& action);
	
private:
	void List( void );
	void GenerateXML( TAlertMsgs* pAlertArray);
	void FormatQueryString(CEMSString& sQuery);
	void DeleteAlerts( void );
	void DeleteAllAlerts( void );

private:
	unsigned m_Access;
	int m_nUnreadCount;
	
	CTBVInfo m_TBView;
};
