////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/MAILSTREAMLIBRARY/EMSEvent.h,v 1.1 2005/05/06 14:56:44 markm Exp $
//
//  Copyright © 2000 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
// EMSEvent.h: interface for the CEMSEvent class.
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEMSEvent
{
public:
	CEMSEvent();											
	virtual ~CEMSEvent();									// Destructor calls ReleaseLock() if necessary

	HANDLE GetHandle() { return m_hEvent; }
	
protected:
	void Create( CEMSString& sName );
	HANDLE m_hEvent;
};


class CReloadISAPIEvent : public CEMSEvent
{
public:
	CReloadISAPIEvent( void )
	{
		int nProcessID = GetCurrentProcessId();
		CEMSString sEventName;
		sEventName.assign( _T("%d_EMS_RELOAD_ISAPI"), nProcessID );
		Create(sEventName);
	}
};

