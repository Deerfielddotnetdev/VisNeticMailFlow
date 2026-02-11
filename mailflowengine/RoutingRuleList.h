// RoutingRules.h: interface for the CRoutingRules class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROUTINGRULES_H__96C4DFEB_E421_449A_B822_AEB99FBC4836__INCLUDED_)
#define AFX_ROUTINGRULES_H__96C4DFEB_E421_449A_B822_AEB99FBC4836__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "InboundMsg.h"
#include <CkHtmlToText.h>
#include <CkGlobal.h>

//*****************************************************************************
// SubjectCount class

class SubjectCount
{
public:
	SubjectCount()
		:_subjects(0)
		,_totalSubjects(0)
	{
	}
	UINT MatchedSubjects() { return (_subjects > 0) ? 1 : 0; }
	UINT HasSubjects() { return (_totalSubjects > 0) ? 1 : 0; }
	void IncrementSubject() { ++_subjects; }
	void IncrementTotalSubject() { ++_totalSubjects; }
protected:
	int _subjects;
	int _totalSubjects;
};

//*****************************************************************************

//*****************************************************************************
// BodyCount class

class BodyCount
{
public:
	BodyCount()
		:_bodys(0)
		,_totalBodys(0)
	{
	}
	UINT MatchedBodys() { return (_bodys > 0) ? 1 : 0; }
	UINT HasBodys() { return (_totalBodys > 0) ? 1 : 0; }
	void IncrementBody() { ++_bodys; }
	void IncrementTotalBody() { ++_totalBodys; }
protected:
	int _bodys;
	int _totalBodys;
};

//*****************************************************************************

//*****************************************************************************
// SubjectBodyCount class

class SubjectBodyCount
{
public:
	SubjectBodyCount()
		:_subjectBodys(0)
		,_totalSubjectBodys(0)
	{
	}
	UINT MatchedSubjectBodys() { return (_subjectBodys > 0) ? 1 : 0; }
	UINT HasSubjectBodys() { return (_totalSubjectBodys > 0) ? 1 : 0; }
	void IncrementSubjectBody() { ++_subjectBodys; }
	void IncrementTotalSubjectBody() { ++_totalSubjectBodys; }
protected:
	int _subjectBodys;
	int _totalSubjectBodys;
};

//*****************************************************************************

//*****************************************************************************
// HeaderCount class

class HeaderCount
{
public:
	HeaderCount()
		:_headers(0)
		,_totalHeaders(0)
	{
	}
	UINT MatchedHeaders() { return (_headers > 0) ? 1 : 0; }
	UINT HasHeaders() { return (_totalHeaders > 0) ? 1 : 0; }
	void IncrementHeader() { ++_headers; }
	void IncrementTotalHeader() { ++_totalHeaders; }
protected:
	int _headers;
	int _totalHeaders;
};

//*****************************************************************************

//*****************************************************************************
// AnyCount class

class AnyCount
{
public:
	AnyCount()
		:_anys(0)
		,_totalAnys(0)
	{
	}
	UINT MatchedAnys() { return (_anys > 0) ? 1 : 0; }
	UINT HasAnys() { return (_totalAnys > 0) ? 1 : 0; }
	void IncrementAny() { ++_anys; }
	void IncrementTotalAny() { ++_totalAnys; }
protected:
	int _anys;
	int _totalAnys;
};

//*****************************************************************************


////////////////////////////////////////////////////////////////////////////////
class CTextMatcher : public TMatchText
{
public:
	enum MATCH_TYPE
	{
		NO_MATCH = 0,
		POS_MATCH,
		NEG_MATCH
	};
public:
	CTextMatcher() // default constructor
	{
	}
	CTextMatcher( const CTextMatcher& i ) // copy constructor
	{
		if (m_MatchTextAllocated > 0) free( m_MatchText );
		memcpy(this,&i,sizeof(MatchText_t));
		if (m_MatchTextAllocated > 0)
		{
			m_MatchText = (wchar_t*) calloc( m_MatchTextAllocated, 1 );
			memcpy(m_MatchText,i.m_MatchText,m_MatchTextAllocated);
		}
		m_pos = i.m_pos;
		m_neg = i.m_neg;
		m_ands = i.m_ands;
		m_negands = i.m_negands;
	}
	CTextMatcher& operator=(const CTextMatcher& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_MatchTextAllocated > 0) free( m_MatchText );
			memcpy(this,&i,sizeof(MatchText_t));
			if (m_MatchTextAllocated > 0)
			{
				m_MatchText = (wchar_t*) calloc( m_MatchTextAllocated, 1 );
				memcpy(m_MatchText,i.m_MatchText,m_MatchTextAllocated);
			}
			m_pos = i.m_pos;
			m_neg = i.m_neg;
			m_ands = i.m_ands;
			m_negands = i.m_negands;
		}
		return *this; // return reference to self
	}
	
	enum { MaxMatchTextWordLength = 1024 };	

	void BuildWordLists(void);
	MATCH_TYPE Match( CInboundMsg& msg );
	bool MatchAndsInBody( CInboundMsg& msg );
	bool MatchAndsInSubject( CInboundMsg& msg );
	bool MatchAndsInHeader( CInboundMsg& msg );
	bool FindString( const wchar_t* szString, wchar_t* szBuffer );
	bool TestRegEx( const wchar_t* szString, wchar_t* szBuffer );

	void SelectByRoutingRuleID( CODBCQuery& query, long RuleID )
	{
		query.Initialize();

		BINDPARAM_LONG( query, RuleID );
		BINDCOL_TINYINT( query, m_MatchLocation );
		BINDCOL_BIT( query, m_IsRegEx );

		query.Execute( L"SELECT MatchLocation,IsRegEx,MatchText "
					   L"FROM MatchText "
					   L"WHERE RoutingRuleID=?" );
	}

protected:
	vector<wstring> m_pos;
	vector<wstring> m_neg;
	vector<wstring> m_ands;
	vector<wstring> m_negands;
	CkHtmlToText	h2t;
	bool			bHtmlToText;
};


////////////////////////////////////////////////////////////////////////////////
class CAddressToMatcher : public TMatchToAddresses
{
public:
	bool Match( CInboundMsg& msg );
	
	void SelectByRoutingRuleID( CODBCQuery& query, long RuleID )
	{
		query.Initialize();

		BINDPARAM_LONG( query, RuleID );
		BINDCOL_WCHAR( query, m_EmailAddress );

		query.Execute( L"SELECT EmailAddress "
					   L"FROM MatchToAddresses "
  					   L"WHERE RoutingRuleID=?" );
	}
};


////////////////////////////////////////////////////////////////////////////////
class CAddressFromMatcher : public TMatchFromAddresses
{
public:
	bool Match( CInboundMsg& msg );

	void SelectByRoutingRuleID( CODBCQuery& query, long RuleID )
	{
		query.Initialize();

		BINDPARAM_LONG( query, RuleID );
		BINDCOL_WCHAR( query, m_EmailAddress );

		query.Execute( L"SELECT EmailAddress "
					   L"FROM MatchFromAddresses "
  					   L"WHERE RoutingRuleID=?" );
	}
};


////////////////////////////////////////////////////////////////////////////////
class CForwardToAddr : public TForwardToAddresses
{
public:
	void SelectByRoutingRuleID( CODBCQuery& query, long RuleID )
	{
		query.Initialize();

		BINDPARAM_LONG( query, RuleID );
		BINDCOL_WCHAR( query, m_EmailAddress );

		query.Execute( L"SELECT EmailAddress "
					   L"FROM ForwardToAddresses "
  					   L"WHERE RoutingRuleID=?" );
	}
};

////////////////////////////////////////////////////////////////////////////////
class CForwardCCAddr : public TForwardCCAddresses
{
public:
	void SelectByRoutingRuleID( CODBCQuery& query, long RuleID )
	{
		query.Initialize();

		BINDPARAM_LONG( query, RuleID );
		BINDCOL_WCHAR( query, m_EmailAddress );

		query.Execute( L"SELECT EmailAddress "
					   L"FROM ForwardCCAddresses "
  					   L"WHERE RoutingRuleID=?" );
	}
};

////////////////////////////////////////////////////////////////////////////////
class CRoutingRule : public TRoutingRules
{
public:

	CRoutingRule() : TRoutingRules() // default constructor
	{
		m_AgentName[0] = _T('\0');
		m_AgentAddress[0] = _T('\0');
		m_TicketboxName[0] = _T('\0');		
	}

	CRoutingRule( const CRoutingRule& i ) // copy constructor
	{
		m_to = i.m_to;
		m_from = i.m_from;
		m_text = i.m_text;
		m_fta = i.m_fta;
		m_fca = i.m_fca;
		wcscpy( m_AgentName, i.m_AgentName );
		wcscpy( m_AgentAddress, i.m_AgentAddress );
		wcscpy( m_TicketboxName, i.m_TicketboxName );
		wcscpy( m_RuleDescrip, i.m_RuleDescrip );
		wcscpy( m_ForwardFromEmail, i.m_ForwardFromEmail );
		wcscpy( m_ForwardFromName, i.m_ForwardFromName );
		m_RoutingRuleID = i.m_RoutingRuleID;
		m_IsEnabled = i.m_IsEnabled;
		m_OrderIndex = i.m_OrderIndex;
		m_AutoReplyEnable = i.m_AutoReplyEnable;
		m_AutoReplyWithStdResponse = i.m_AutoReplyWithStdResponse;
		m_AutoReplyQuoteMsg = i.m_AutoReplyQuoteMsg;
		m_AutoReplyCloseTicket = i.m_AutoReplyCloseTicket;
		m_ForwardEnable = i.m_ForwardEnable;
		m_ForwardFromAgent = i.m_ForwardFromAgent;
		m_ForwardFromContact = i.m_ForwardFromContact;
		m_ForwardInTicket = i.m_ForwardInTicket;
		m_QuoteOriginal = i.m_QuoteOriginal;
		m_MultiMail = i.m_MultiMail;
		m_SetOpenOwner = i.m_SetOpenOwner;
		m_OfficeHours = i.m_OfficeHours;
		m_IgnoreTracking = i.m_IgnoreTracking;
		m_DoNotAssign = i.m_DoNotAssign;
		m_AutoReplyInTicket = i.m_AutoReplyInTicket;
		m_AllowRemoteReply = i.m_AllowRemoteReply;
		m_AssignToTicketBox = i.m_AssignToTicketBox;
		m_AssignToAgentEnable = i.m_AssignToAgentEnable;
		m_AssignToAgent = i.m_AssignToAgent;
		m_DeleteImmediatelyEnable = i.m_DeleteImmediatelyEnable;
		m_DeleteImmediately = i.m_DeleteImmediately;
		m_HitCount = i.m_HitCount;
		m_PriorityID = i.m_PriorityID;
		m_MessageSourceTypeID = i.m_MessageSourceTypeID;
		wcscpy( m_AutoReplyFrom, i.m_AutoReplyFrom );
		m_AssignToAgentAlg = i.m_AssignToAgentAlg;
		m_AssignToTicketBoxAlg = i.m_AssignToTicketBoxAlg;
		m_AssignUniqueTicketID = i.m_AssignUniqueTicketID;
		m_AssignToTicketCategory = i.m_AssignToTicketCategory;
		m_AlertEnable = i.m_AlertEnable;
		m_AlertIncludeSubject = i.m_AlertIncludeSubject;
		m_AlertToAgentID = i.m_AlertToAgentID;
		m_ToOrFrom = i.m_ToOrFrom;
		m_ConsiderAllOwned = i.m_ConsiderAllOwned;
		m_DoProcessingRules = i.m_DoProcessingRules;
		m_LastOwnerID = i.m_LastOwnerID;
		m_MatchMethod = i.m_MatchMethod;		
		wcscpy( m_AlertText, i.m_AlertText );
		nOoo = i.nOoo;
		nOffline = i.nOffline;
		nDnd = i.nDnd;
		nNotAvail = i.nNotAvail;
		nOffline = i.nOffline;
		nAway = i.nAway;
		nOnline = i.nOnline;
	}

	CRoutingRule& operator=(const CRoutingRule& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			m_to = i.m_to;
			m_from = i.m_from;
			m_text = i.m_text;
			m_fta = i.m_fta;
			m_fca = i.m_fca;
			wcscpy( m_AgentName, i.m_AgentName );
			wcscpy( m_AgentAddress, i.m_AgentAddress );
			wcscpy( m_TicketboxName, i.m_TicketboxName );
			wcscpy( m_RuleDescrip, i.m_RuleDescrip );
			wcscpy( m_ForwardFromEmail, i.m_ForwardFromEmail );
			wcscpy( m_ForwardFromName, i.m_ForwardFromName );
			m_RoutingRuleID = i.m_RoutingRuleID;
			m_IsEnabled = i.m_IsEnabled;
			m_OrderIndex = i.m_OrderIndex;
			m_AutoReplyEnable = i.m_AutoReplyEnable;
			m_AutoReplyWithStdResponse = i.m_AutoReplyWithStdResponse;
			m_AutoReplyQuoteMsg = i.m_AutoReplyQuoteMsg;
			m_AutoReplyCloseTicket = i.m_AutoReplyCloseTicket;
			m_ForwardEnable = i.m_ForwardEnable;
			m_ForwardFromAgent = i.m_ForwardFromAgent;
			m_ForwardFromContact = i.m_ForwardFromContact;
			m_ForwardInTicket = i.m_ForwardInTicket;
			m_QuoteOriginal = i.m_QuoteOriginal;
			m_MultiMail = i.m_MultiMail;
			m_SetOpenOwner = i.m_SetOpenOwner;
			m_OfficeHours = i.m_OfficeHours;
			m_IgnoreTracking = i.m_IgnoreTracking;
			m_DoNotAssign = i.m_DoNotAssign;
			m_AutoReplyInTicket = i.m_AutoReplyInTicket;
			m_AllowRemoteReply = i.m_AllowRemoteReply;
			m_AssignToTicketBox = i.m_AssignToTicketBox;
			m_AssignToAgentEnable = i.m_AssignToAgentEnable;
			m_AssignToAgent = i.m_AssignToAgent;
			m_DeleteImmediatelyEnable = i.m_DeleteImmediatelyEnable;
			m_DeleteImmediately = i.m_DeleteImmediately;
			m_HitCount = i.m_HitCount;
			m_PriorityID = i.m_PriorityID;
			m_MessageSourceTypeID = i.m_MessageSourceTypeID;
			wcscpy( m_AutoReplyFrom, i.m_AutoReplyFrom );
			m_AssignToAgentAlg = i.m_AssignToAgentAlg;
			m_AssignToTicketBoxAlg = i.m_AssignToTicketBoxAlg;
			m_AssignUniqueTicketID = i.m_AssignUniqueTicketID;
			m_AssignToTicketCategory = i.m_AssignToTicketCategory;
			m_AlertEnable = i.m_AlertEnable;
			m_AlertIncludeSubject = i.m_AlertIncludeSubject;
			m_AlertToAgentID = i.m_AlertToAgentID;
			m_ToOrFrom = i.m_ToOrFrom;
			m_ConsiderAllOwned = i.m_ConsiderAllOwned;
			m_DoProcessingRules = i.m_DoProcessingRules;
			m_LastOwnerID = i.m_LastOwnerID;
			m_MatchMethod = i.m_MatchMethod;
			wcscpy( m_AlertText, i.m_AlertText );
			nOoo = i.nOoo;
			nOffline = i.nOffline;
			nDnd = i.nDnd;
			nNotAvail = i.nNotAvail;
			nOffline = i.nOffline;
			nAway = i.nAway;
			nOnline = i.nOnline;
		}
		return *this; // return reference to self
	}	


	void SelectRulesInOrder( CODBCQuery& query );

	void BumpHitCount( CODBCQuery& query )
	{
		query.Initialize();
			
		BINDPARAM_LONG( query, m_RoutingRuleID );
			
		query.Execute( L"UPDATE RoutingRules "
					   L"SET HitCount = HitCount + 1 "
					   L"WHERE RoutingRuleID=?");
	}

	vector<CAddressToMatcher> m_to;
	vector<CAddressFromMatcher> m_from;
	vector<CTextMatcher> m_text;
	vector<CForwardToAddr> m_fta;
	vector<CForwardCCAddr> m_fca;
	
	wchar_t m_AgentName[AGENTS_NAME_LENGTH];
	long m_AgentNameLen;
	wchar_t m_AgentAddress[PERSONALDATA_DATAVALUE_LENGTH];
	long m_AgentAddressLen;
	wchar_t m_TicketboxName[TICKETBOXES_NAME_LENGTH];
	long m_TicketboxNameLen;

	int nOoo; //1
	int nOffline; //2
	int nDnd; //4
	int nNotAvail; //8
	int nAway; //16
	int nOnline; //32
};

////////////////////////////////////////////////////////////////////////////////
class CRoutingRuleList  
{
public:
	CRoutingRuleList();
	virtual ~CRoutingRuleList();

	void Refresh( CODBCQuery& query );
	void PreprocessPattern( wchar_t* szEmailAddress );

	CRoutingRule* GetRuleMatch( CInboundMsg& msg );
	CRoutingRule* FindRule( unsigned int RoutingRuleID );

	enum { RefreshRuleIntervalMs = 60000 };	

	long					m_LastRefreshTime;

protected:
	
	vector<CRoutingRule>	m_list;
};



#define IS_WHITE_SPACE(x) ((x)==L' '||(x)==L'\n'||(x)==L'\r'||(x)==L'\t')

#endif // !defined(AFX_ROUTINGRULES_H__96C4DFEB_E421_449A_B822_AEB99FBC4836__INCLUDED_)
