// ProcessingRules.h: interface for the CProcessingRules class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSINGRULES_H__96C3CBEB_E421_449A_B822_AEB99FBC4836__INCLUDED_)
#define AFX_PROCESSINGRULES_H__96C3CBEB_E421_449A_B822_AEB99FBC4836__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "InboundMsg.h"
#include <CkHtmlToText.h>
#include <CkGlobal.h>

//*****************************************************************************
// SubjectCountP class

class SubjectCountP
{
public:
	SubjectCountP()
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

class BodyCountP
{
public:
	BodyCountP()
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

class SubjectBodyCountP
{
public:
	SubjectBodyCountP()
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

class HeaderCountP
{
public:
	HeaderCountP()
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

class AnyCountP
{
public:
	AnyCountP()
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
class CTextMatcherP : public TMatchTextP
{
public:
	enum MATCH_TYPE
	{
		NO_MATCH = 0,
		POS_MATCH,
		NEG_MATCH
	};
public:
	CTextMatcherP() // default constructor
	{
	}
	CTextMatcherP( const CTextMatcherP& i ) // copy constructor
	{
		if (m_MatchTextAllocated > 0) free( m_MatchText );
		memcpy(this,&i,sizeof(MatchTextP_t));
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
	CTextMatcherP& operator=(const CTextMatcherP& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			if (m_MatchTextAllocated > 0) free( m_MatchText );
			memcpy(this,&i,sizeof(MatchTextP_t));
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

	void SelectByProcessingRuleID( CODBCQuery& query, long RuleID )
	{
		query.Initialize();

		BINDPARAM_LONG( query, RuleID );
		BINDCOL_TINYINT( query, m_MatchLocation );
		BINDCOL_BIT( query, m_IsRegEx );

		query.Execute( L"SELECT MatchLocation,IsRegEx,MatchText "
					   L"FROM MatchTextP "
					   L"WHERE ProcessingRuleID=?" );
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
class CProcessingRule : public TProcessingRules
{
public:

	CProcessingRule() : TProcessingRules() // default constructor
	{
		
	}

	CProcessingRule( const CProcessingRule& i ) // copy constructor
	{
		m_text = i.m_text;
		wcscpy( m_RuleDescrip, i.m_RuleDescrip );
		m_ProcessingRuleID = i.m_ProcessingRuleID;
		m_IsEnabled = i.m_IsEnabled;
		m_OrderIndex = i.m_OrderIndex;
		m_HitCount = i.m_HitCount;
		m_ActionType = i.m_ActionType;
		m_ActionID = i.m_ActionID;
		m_PrePost = i.m_PrePost;
	}

	CProcessingRule& operator=(const CProcessingRule& i) // assignment operator
	{
		if (this != &i) // trap for assignments to self
		{
			m_text = i.m_text;
			wcscpy( m_RuleDescrip, i.m_RuleDescrip );
			m_ProcessingRuleID = i.m_ProcessingRuleID;
			m_IsEnabled = i.m_IsEnabled;
			m_OrderIndex = i.m_OrderIndex;
			m_HitCount = i.m_HitCount;
			m_ActionType = i.m_ActionType;
			m_ActionID = i.m_ActionID;
			m_PrePost = i.m_PrePost;
		}
		return *this; // return reference to self
	}	


	void SelectRulesInOrder( CODBCQuery& query );

	void BumpHitCount( CODBCQuery& query )
	{
		query.Initialize();
			
		BINDPARAM_LONG( query, m_ProcessingRuleID );
			
		query.Execute( L"UPDATE ProcessingRules "
					   L"SET HitCount = HitCount + 1 "
					   L"WHERE ProcessingRuleID=?");
	}

	vector<CTextMatcherP> m_text;
		
};

////////////////////////////////////////////////////////////////////////////////
class CProcessingRuleList  
{
public:
	CProcessingRuleList();
	virtual ~CProcessingRuleList();

	void Refresh( CODBCQuery& query );
	
	CProcessingRule* GetRuleMatch( CInboundMsg& msg );
	CProcessingRule* FindRule( unsigned int ProcessingRuleID );

	enum { RefreshRuleIntervalMs = 60000 };	

	long					m_LastRefreshTime;

protected:
	
	vector<CProcessingRule>	m_list;
};



#define IS_WHITE_SPACE(x) ((x)==L' '||(x)==L'\n'||(x)==L'\r'||(x)==L'\t')

#endif
