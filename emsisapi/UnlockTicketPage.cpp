#include "stdafx.h"
#include ".\UnlockTicketPage.h"
#include ".\Ticket.h"

UnlockTicketPage::UnlockTicketPage(CISAPIData& ISAPIData)
	:CXMLDataClass(ISAPIData)
{
}

UnlockTicketPage::~UnlockTicketPage(void)
{
}

int UnlockTicketPage::Run(CURLAction& action)
{
	// Check security
	RequireAdmin();

	std::string sAction;
	GetISAPIData().GetFormString("action", sAction, true);

	if(!lstrcmpi(sAction.c_str(),"unlock"))
	{
		int nTicketID = 0;
		GetISAPIData().GetFormLong(_T("txtTicketID"), nTicketID, true);

		return this->UnlockTicket(nTicketID);
	}

	return 0;
}

int UnlockTicketPage::UnlockTicket(int& nTicketID)
{
	CTicket theTicket(this->GetISAPIData(), nTicketID);
	if(theTicket.AdminUnLock())
	{
		GetXMLGen().AddChildElem(_T("unlockticket"));
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem(_T("result"), _T("Failed to unlock ticket!"));
		GetXMLGen().OutOfElem();
	}
	else
	{
		GetXMLGen().AddChildElem(_T("unlockticket"));
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem(_T("result"), _T("Successfully unlocked ticket."));
		GetXMLGen().OutOfElem();
	}

	return 0;
}
