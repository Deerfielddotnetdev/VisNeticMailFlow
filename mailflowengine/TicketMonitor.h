// TicketMonitor.h: interface for the CTicketMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TICKETMONITOR_H__15A7EC07_AB18_46FE_BE2B_03CB3533FC98__INCLUDED_)
#define AFX_TICKETMONITOR_H__15A7EC07_AB18_46FE_BE2B_03CB3533FC98__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTicketMonitor  
{
public:
	CTicketMonitor( CODBCQuery& query );
	virtual ~CTicketMonitor();

	HRESULT Run(void);
	int FinishCurrentJob(void);

protected:

	bool GetAutoReplyTicket( void );
	bool GetCustomAutoReplyTicket( int nGetType );
	void SendAutoReply( void );
	void SendCustomAutoReply( void );
	bool GetAgedTicket( map<unsigned int,tstring>& AlertMsgBody );
	void HandleAgeAlert( map<unsigned int,tstring>& AlertMsgBody );
	bool CheckWaterMarks( void );
	bool GetCustomAgedTicket( int nGetType, map<unsigned int,tstring>& AlertMsgBody );
	void HandleCustomAgeAlert( map<unsigned int,tstring>& AlertMsgBody );
	bool CheckCustomWaterMarks( int nCheckType );
	void SendWaterMarkAlert( TTicketBoxes* pInfo, int Count, int AlertEventID );
	void SendCustomWaterMarkAlert( int WaterMarkAlertID, int Count, int nCheckType, int AlertEventID );
	void SetWaterMarkStatus( int WaterMarkAlertID, int Status );
	void SetWaterMarkHitCount( int WaterMarkAlertID, int HitCount );
	void SetTicketBoxStatus( int TicketBoxID, int Status );
	void SendAgeAlert( int TicketBoxID, int nTickets, tstring& msgBody );
	void SendCustomAgeAlert( int nAlertOnTypeID, int nAgeAlertID, int TicketBoxID, int nTickets, tstring& msgBody );
	bool GetAutoMessage( int nGetType );
	void SendAutoMessage( void );
	bool GetContactSendMessage( int nContactID );
	bool CheckLockedTickets( void );
	void FreeReservedMsgIDs( int TicketID );
	void UnlockTicket( int TicketID );
	void DeleteTicket( int TicketID );
	void ProcessAutoActions( void );
	void EscalateTicket( int nTicketID, int nTicketBoxID, int nEscalateToID );

    
	enum { AutoReplyCheckIntervalMs = 60000,
		   AutoReplyCheckTbClosedIntervalMs = 60000,
		   AutoReplyCheckTbCreatedIntervalMs = 60000,
		   AutoReplyCheckTcClosedIntervalMs = 60000,
		   AutoReplyCheckTcCreatedIntervalMs = 60000,
		   AgeAlertCheckIntervalMs = 60000,
		   WaterMarkCheckIntervalMs = 60000,
		   CustomWaterMarkTbCheckIntervalMs = 60000,
		   CustomWaterMarkTcCheckIntervalMs = 60000,
		   CustomAgeAlertTbCheckIntervalMs = 60000,
		   CustomAgeAlertTcCheckIntervalMs = 60000,
		   AutoMessageTbClosedCheckIntervalMs = 60000,
		   AutoMessageTcClosedCheckIntervalMs = 60000,
		   AutoMessageTbCreatedCheckIntervalMs = 60000,
		   AutoMessageTcCreatedCheckIntervalMs = 60000,
		   LockedTicketCheckIntervalMs = 60000,
		   AutoActionCheckIntervalMs = 60000 };	

	long			m_LastAutoReplyCheck;	
	long			m_LastTbClosedAutoReplyCheck;	
	long			m_LastTbCreatedAutoReplyCheck;	
	long			m_LastTcClosedAutoReplyCheck;	
	long			m_LastTcCreatedAutoReplyCheck;	
	long			m_LastAgeAlertCheck;	
	long			m_LastAgeAlertTbCheck;	
	long			m_LastAgeAlertTcCheck;	
	long			m_LastWaterMarkCheck;
	long			m_LastCustomWaterMarkTbCheck;
	long			m_LastCustomWaterMarkTcCheck;
	long			m_LastAutoMessageTbClosedCheck;
	long			m_LastAutoMessageTbCreatedCheck;
	long			m_LastAutoMessageTcClosedCheck;
	long			m_LastAutoMessageTcCreatedCheck;
	long			m_LastLockedTicketCheck;
	long			m_LastAutoActionCheck;
	
	CODBCQuery&			m_query;

	typedef struct Auto_t
	{
		Auto_t()
		{
			ZeroMemory( this, sizeof(Auto_t) );
		}
		unsigned int		m_TicketID;
		long				m_TicketIDLen;
		unsigned int		m_AgeAlertID;
		long				m_AgeAlertIDLen;
		unsigned int		m_TicketBoxID;
		long				m_TicketBoxIDLen;
		unsigned int		m_AutoReplyStdRespID;
		long				m_AutoReplyStdRespIDLen;
		unsigned char		m_AutoReplyQuoteMsg;	
		long				m_AutoReplyQuoteMsgLen;
		unsigned char		m_AutoReplyCloseTicket;
		long				m_AutoReplyCloseTicketLen;
		unsigned char		m_AutoReplyInTicket;
		long				m_AutoReplyInTicketLen;
		TCHAR				m_AutoReplyFrom[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];
		long				m_AutoReplyFromLen;
		TCHAR				m_AutoReplyFromName[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];
		long				m_AutoReplyFromNameLen;
		TCHAR				m_TicketBoxName[TICKETBOXES_NAME_LENGTH];
		long				m_TicketBoxNameLen;
		TCHAR				m_TicketCategoryName[TICKETCATEGORIES_DESCRIPTION_LENGTH];
		long				m_TicketCategoryNameLen;
		long				m_TicketBoxHeaderID;
		long				m_TicketBoxFooterID;
		unsigned char		m_FooterLocation;
		unsigned int		m_SendFromTypeID;
		long				m_SendFromTypeIDLen;
		unsigned int		m_CreateNewTicket;
		long				m_CreateNewTicketLen;
		TCHAR				m_NewReplyToAddress[TICKETBOXES_DEFAULTEMAILADDRESS_LENGTH];
		long				m_NewReplyToAddressLen;
		TCHAR				m_NewReplyToName[TICKETBOXES_NAME_LENGTH];
		long				m_NewReplyToNameLen;	
		TCHAR				m_Subject[TICKETS_SUBJECT_LENGTH];
		long				m_SubjectLen;	
		unsigned int		m_NewOwnerID;
		long				m_NewOwnerIDLen;
		unsigned int		m_NewTicketBoxID;
		long				m_NewTicketBoxIDLen;
		unsigned int		m_NewTicketCategoryID;
		long				m_NewTicketCategoryIDLen;
		unsigned int		m_SendToPercent;	
		long				m_SendToPercentLen;
		unsigned int		m_OmitTracking;	
		long				m_OmitTrackingLen;
		unsigned int		m_AutoMessageID;	
		long				m_AutoMessageIDLen;
		unsigned int		m_AutoResponseID;	
		long				m_AutoResponseIDLen;
		unsigned int		m_HitCount;	
		long				m_HitCountLen;
		unsigned int		m_FailCount;	
		long				m_FailCountLen;
		unsigned int		m_TotalCount;	
		long				m_TotalCountLen;

	} Auto_t;

	Auto_t m_Auto_t;	

	wchar_t				m_szSendMessage[PERSONALDATA_DATAVALUE_LENGTH];
	
	TTickets newTicket;
	
	multimap<unsigned int,unsigned int> m_AgeAlertedTickets;
	
	TCustomAgeAlerts caa;
	vector<TCustomAgeAlerts> m_caa;
	vector<TCustomAgeAlerts>::iterator caaIter;

	TAutoActions aa;
	vector<TAutoActions> m_aa;
	vector<TAutoActions>::iterator aaIter;

	vector<Auto_t> m_a;
	vector<Auto_t>::iterator aIter;
};

#endif // !defined(AFX_TICKETMONITOR_H__15A7EC07_AB18_46FE_BE2B_03CB3533FC98__INCLUDED_)
