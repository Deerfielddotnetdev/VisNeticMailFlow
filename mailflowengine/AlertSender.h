// AlertSender.h: interface for the CAlertSender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALERTSENDER_H__5FAFBC42_B12C_42AF_A0FB_9EBE2D245A85__INCLUDED_)
#define AFX_ALERTSENDER_H__5FAFBC42_B12C_42AF_A0FB_9EBE2D245A85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CAlertRecipient
{
public:

	CAlertRecipient()
	{
		m_bSent = FALSE;
	}

	unsigned int m_AlertMethodID;
	long m_AlertMethodIDLen;

	unsigned int m_AgentID;
	long m_AgentIDLen;

	unsigned int m_GroupID;
	long m_GroupIDLen;

	TCHAR m_AlertDescription[ALERTEVENTS_DESCRIPTION_LENGTH];
	long m_AlertDescriptionLen;

	TCHAR m_TicketBoxName[TICKETBOXES_DESCRIPTION_LENGTH];
	long m_TicketBoxNameLen;

	TCHAR m_EmailAddress[ALERTS_EMAILADDRESS_LENGTH];
	long m_EmailAddressLen;
	
	TCHAR m_FromEmailAddress[ALERTS_FROMEMAILADDRESS_LENGTH];
	long m_FromEmailAddressLen;

	int m_AlertEventID;
	
	BOOL m_bSent;
};

class CAlert
{
public:

	CAlert()
	{
		m_AlertEventID = 0;
		m_pAlertInfo = 0;
	}

	CAlert( unsigned int AlertEventID, AlertInfo* pAlertInfo ) 
		: m_AlertEventID(AlertEventID), m_pAlertInfo( pAlertInfo )
	{
	}

	~CAlert()
	{
		if( m_pAlertInfo )
		{
			if( !HeapFree( GetProcessHeap(), 0, m_pAlertInfo) )
			{
			}
		}
	}

	// Copy constructor
	CAlert( const CAlert& i ) : 
		m_AlertEventID( i.m_AlertEventID )
	{
		if( i.m_pAlertInfo )
		{
			int nAlertSize = sizeof(AlertInfo) + ( 2 * wcslen(i.m_pAlertInfo->wcsText) );
			m_pAlertInfo = (AlertInfo*) HeapAlloc ( GetProcessHeap (), 0, nAlertSize );
			memcpy( m_pAlertInfo, i.m_pAlertInfo, nAlertSize );
		}
		else
		{
			m_pAlertInfo = NULL;
		}
		m_Recipients = i.m_Recipients;
	}

	// Assignment operator
	CAlert& operator=(const CAlert& i) 
	{
		if (this != &i) // trap for assignments to self
		{
			m_AlertEventID =  i.m_AlertEventID; 
	
			if( m_pAlertInfo )
			{
				if(!HeapFree( GetProcessHeap(), 0, m_pAlertInfo) )
				{
				}
			}

			if( i.m_pAlertInfo )
			{
				int nAlertSize = sizeof(AlertInfo) + ( 2 * wcslen(i.m_pAlertInfo->wcsText) );
				m_pAlertInfo = (AlertInfo*) HeapAlloc (GetProcessHeap (), 0, nAlertSize );
				memcpy( m_pAlertInfo, i.m_pAlertInfo, nAlertSize );
			}
			else
			{
				m_pAlertInfo = NULL;
			}
			m_Recipients = i.m_Recipients;
		}
		return *this; // return reference to self
	}

	unsigned int m_AlertEventID;
	
	AlertInfo* m_pAlertInfo;
	vector<CAlertRecipient> m_Recipients;
};


class CAlertSender  
{
public:
	CAlertSender( CODBCQuery& query );
	virtual ~CAlertSender();

	void SendAlert( long AlertEventID, AlertInfo* pAlertInfo );
	HRESULT Run(void);
	void HandleAlert( CAlert& alert );
	void DispatchAlert( CAlert& alert, CAlertRecipient& recipient );
	void SendInstantMsg( CAlert& alert, CAlertRecipient& recipient );
	void SendInternalEmail( CAlert& alert, CAlertRecipient& recipient );
	void SendExternalEmail( CAlert& alert, CAlertRecipient& recipient );
	void UnInitialize(void);

protected:
	CODBCQuery&			m_query;
	deque<CAlert>		m_Queue;
};

#endif // !defined(AFX_ALERTSENDER_H__5FAFBC42_B12C_42AF_A0FB_9EBE2D245A85__INCLUDED_)
