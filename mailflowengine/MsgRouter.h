// MsgRouter.h: interface for the CMsgRouter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGROUTER_H__A845773D_8EBF_4070_996A_50E3B8B12D70__INCLUDED_)
#define AFX_MSGROUTER_H__A845773D_8EBF_4070_996A_50E3B8B12D70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RoutingRuleList.h"
#include "ProcessingRuleList.h"


struct SRIDWITHKEYWORD
{
	long			_lStandardResponseID;
	long            _lStandardResponseIDLen;
	dca::WString255 _sKeywords;
	long            _sKeywordsLen;

	SRIDWITHKEYWORD()
	{
		ZeroMemory(this, sizeof(SRIDWITHKEYWORD));
	}

	SRIDWITHKEYWORD(const SRIDWITHKEYWORD& srid)
	{
		_lStandardResponseID = srid._lStandardResponseID;
		_lStandardResponseIDLen = srid._lStandardResponseIDLen;
		wcsncpy(_sKeywords, srid._sKeywords, 255);
		_sKeywordsLen = srid._sKeywordsLen;
	}

	SRIDWITHKEYWORD& operator=(const SRIDWITHKEYWORD& srid)
	{
		_lStandardResponseID = srid._lStandardResponseID;
		_lStandardResponseIDLen = srid._lStandardResponseIDLen;
		wcsncpy(_sKeywords, srid._sKeywords, 255);
		_sKeywordsLen = srid._sKeywordsLen;
	}
};

class CMsgRouter  
{
public:
	CMsgRouter( CODBCQuery& query );
	virtual ~CMsgRouter();


	void Initialize(void);
	HRESULT Run(void);
	int FinishCurrentJob(void);
	void UnInitialize(void);
	
protected:

	enum { MRState_UnInitialized,
		   MRState_RefreshRules,
		   MRState_GetMessage, 
		   MRState_CheckContact, 
		   MRState_FindRuleMatch, 
		   MRState_BumpHitCount,
		   MRState_Forward,
		   MRState_AutoReply,
		   MRState_PutInTicketBox,
		   MRState_RemoveFromQueue,
		   MRState_CreateAutoReplies,
		   MRState_Finished };

	void RefreshRules(void);
	void GetNewMessage( void );
	void CheckContact( void );
	void ImplicitRoutingRule(void);
	void FindRuleMatch( void );
	void BumpHitCount( void );
	void ShredRule( void );
	void ForwardCopy( void );
	void AutoReply( void );	
	void PutInTicketBox( void );
	void CheckAgentAlerts( int nOwnerID, bool bIsContactOwner );
	void CreateAutoReplies();
	unsigned int CreateNewTicket( void );
	int UnDeleteTicket( void );
	int SetTicketState(  long TicketStateID );  // changes ticket state
	void FindTicketByTicketBoxAndContact( void );
	void CreateNewTicketForContact( void );
	void RemoveFromQueue(void);
	bool GetTicketInfo( int nTicketID, int nMsgID );
	bool GetPreRoutedTicketInfo(void);
	void GetContactName(void);
	void ValidateOwnerID( set<long> AgentIDs );
	void CheckOwnerStatus( void );
	bool GetTicketIDFromHeader( int InboundMessageID, int& TicketID, int& MsgID );

	unsigned int GetRegDword( wchar_t* szName );
	void SetRegDword( wchar_t* szName, DWORD dwValue );
	void WalkLockedMessages( tstring& sMsgIDs );
	bool AssignToAgentWithMatchingAddress(void);
	bool AssignToAgentWhoOwnsContact(void);
	bool AssignToAgentByRoundRobin(int DenyOnline, int DenyAway, int DenyNotAvail, int DenyOffline, int DenyOoo);
	void AssignToAgentAlgorithm( int AlgID, int AgentID, int DenyOnline, int DenyAway, int DenyNotAvail, int DenyOffline, int DenyOoo );
	bool AssignToTicketBoxWithMatchingAddress(void);
	bool AssignToTicketBoxByPercent(void);
	void AssignToTicketBoxAlgorithm( int AlgID, int TicketBoxID );

	TAssignToTicketBoxes attb;
	vector<TAssignToTicketBoxes> m_attb;
	vector<TAssignToTicketBoxes>::iterator attbIter;	
	
	int					m_MRState;
	bool				m_bGotNewMsg;
	unsigned int		m_RoutingRuleID;
	unsigned int		m_ProcessingRuleID;
	int					m_bNewTicket;
	bool				m_bNewContact;
	int					m_ActionType;
	int					m_ActionID;
	
	CODBCQuery&			m_query;
	CRoutingRuleList	m_rules;
	CProcessingRuleList m_rulesP;
	wchar_t				m_szContactName[CONTACTS_NAME_LENGTH];
	wchar_t				m_szContactEmail[PERSONALDATA_DATAVALUE_LENGTH];
	CInboundMsg			m_msg;
	TTickets			m_tkt;
	CRoutingRule*		m_pRule;
	CProcessingRule*	m_pRuleP;
	bool				m_AutoReplyCloseTicketFlag;
	wchar_t				m_szTicketBoxName[TICKETBOXES_NAME_LENGTH];
	map<int,TMessageSources>	m_MsgSrcs;
	map<unsigned int,unsigned int>	m_AgentStatusMap;
	map<int,TBypass>	m_Bypass;

	int					m_nLastState;
	int					m_nFailedCnt;

	enum { RefreshRuleIntervalMs = 60000 };	

	long				m_LastRefreshTime;
	map<unsigned int,unsigned int>	m_LockedTicketMap;
	
	bool SetEscapeCharacter(TCHAR* pEscChar, dca::WString& rSrchStr);
	bool UpdateSubjectWithEscape(dca::WString& rSubject, TCHAR tChar, TCHAR* pEscChar);
	bool CleanupReplyInSubject(dca::WString& rSubject);
	bool CleanupForwardInSubject(dca::WString& rSubject);
	void CleanupSubject(dca::WString& rSubject);
	void AddTicketFieldTicket( long TicketID, long TicketFieldID, TCHAR* DataValue );

	vector<TTicketFieldsTicket> m_tft;
	vector<TTicketFieldsTicket>::iterator tftIter;
};

#endif // !defined(AFX_MSGROUTER_H__A845773D_8EBF_4070_996A_50E3B8B12D70__INCLUDED_)
