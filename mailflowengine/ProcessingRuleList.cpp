// ProcessingRules.cpp: implementation of the CProcessingRuleList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProcessingRuleList.h"
#include <.\boost\regex.hpp>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessingRuleList::CProcessingRuleList()
{
	m_LastRefreshTime = 0;	// Never
}

CProcessingRuleList::~CProcessingRuleList()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// SelectRulesInOrder - Used by Processing Engine
// 
////////////////////////////////////////////////////////////////////////////////
void CProcessingRule::SelectRulesInOrder( CODBCQuery& query )
{
	query.Initialize();

	BINDCOL_LONG( query, m_ProcessingRuleID );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_ActionID );
	BINDCOL_LONG( query, m_ActionType );
	BINDCOL_BIT( query, m_PrePost );
	BINDCOL_WCHAR( query, m_RuleDescrip );
		
	query.Execute( L"SELECT ProcessingRuleID,HitCount,ActionID,ActionType,PrePost,RuleDescrip "
				   L"FROM ProcessingRules AS P "
				   L"WHERE P.IsEnabled=1 "
				   L"ORDER BY OrderIndex" );

	int x = 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Refresh - Refreshes the list of Processing rules from the DB
// 
////////////////////////////////////////////////////////////////////////////////
void CProcessingRuleList::Refresh( CODBCQuery& query )
{
	CProcessingRule rule;
	vector<CProcessingRule>::iterator iter;
	CTextMatcherP mtext;
		
	// If the rules are fresh enough, return immediately
	if( m_LastRefreshTime != 0 )
	{
		if ( GetTickCount() - m_LastRefreshTime < RefreshRuleIntervalMs )
		{
			return;
		}
	}

	try
	{
		rule.SelectRulesInOrder( query );

		// Clear the list
		m_list.clear();

		while( query.Fetch() == S_OK )
		{
			m_list.push_back( rule );
		}

		for(iter = m_list.begin(); iter != m_list.end(); ++iter)
		{
			mtext.SelectByProcessingRuleID( query, iter->m_ProcessingRuleID );
		
			while( query.Fetch() == S_OK )
			{
				GETDATA_TEXT( query, mtext.m_MatchText );
				mtext.BuildWordLists();
				iter->m_text.push_back( mtext );
			}

		}

		// Set the last refresh time
		m_LastRefreshTime = GetTickCount();
	}
	catch( ... )
	{
		// We could have a partially populated list, so we had better
		// bail and try again later.
		m_list.clear();

		throw;
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
//  GetRuleMatch walks the list of Processing rules and returns a pointer
//  to the matching rule or NULL if no rule matches.
// 
////////////////////////////////////////////////////////////////////////////////
CProcessingRule* CProcessingRuleList::GetRuleMatch( CInboundMsg& msg )
{
	dca::String e;
	e.Format("CProcessingRuleList::GetRuleMatch - entered, rule list size = %d", m_list.size());
	//DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

	vector<CProcessingRule>::iterator RuleIter;
	vector<CTextMatcherP>::iterator TextIter;
	bool bTextMatch;

	for(RuleIter = m_list.begin(); RuleIter != m_list.end(); ++RuleIter)
	{
		
		// Does the subject and/or body match?
		bTextMatch = (RuleIter->m_text.size() == 0);
		
		SubjectCountP     sc;
		BodyCountP        bc;
		SubjectBodyCountP sbc;
		HeaderCountP      hc;
		AnyCountP         ac;

		CTextMatcherP::MATCH_TYPE bMatch = CTextMatcherP::NO_MATCH;

		if( !bTextMatch )
		{
			e.Format("CProcessingRuleList::GetRuleMatch - Text size is %d", RuleIter->m_text.size());
			//DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

			for(TextIter = RuleIter->m_text.begin(); TextIter != RuleIter->m_text.end(); ++TextIter)
			{
				dca::String text1(TextIter->m_MatchText);
				e.Format("CProcessingRuleList::GetRuleMatch - Ready to match %s with messag id %d", text1.c_str(), msg.m_InboundMessageID);
				//DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

				// Do we have a match?
				bMatch = TextIter->Match(msg);

				e.Format("CProcessingRuleList::GetRuleMatch - have match = %d and location = %d", bMatch, TextIter->m_MatchLocation);
				//DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

				// What location were we looking at
				switch(TextIter->m_MatchLocation)
				{
				case EMS_MATCH_LOCATION_SUBJECT:
					{
						// Did we have a match
						if(bMatch == CTextMatcherP::POS_MATCH)
							sc.IncrementSubject();	// Yes then increment number of subjects had a match
						else if(bMatch == CTextMatcherP::NO_MATCH)
						{
							if(TextIter->MatchAndsInSubject(msg))
								sc.IncrementSubject();
						}

						sc.IncrementTotalSubject();	// Increment number of subjects we had both positive and negative
					}
					break;
				case EMS_MATCH_LOCATION_BODY:
					{
						if(bMatch == CTextMatcherP::POS_MATCH)
							bc.IncrementBody();
						else if(bMatch == CTextMatcherP::NO_MATCH)
						{
							if(TextIter->MatchAndsInBody(msg))
								bc.IncrementBody();
						}

						bc.IncrementTotalBody();
					}
					break;
				case EMS_MATCH_LOCATION_SUBJECT_OR_BODY:
					if(bMatch == CTextMatcherP::POS_MATCH)
						sbc.IncrementSubjectBody();
					else if(bMatch == CTextMatcherP::NO_MATCH)
					{
						if(TextIter->MatchAndsInSubject(msg) || TextIter->MatchAndsInBody(msg))
							sbc.IncrementSubjectBody();
					}

					sbc.IncrementTotalSubjectBody();
					break;
				case EMS_MATCH_LOCATION_HEADERS:
					if(bMatch == CTextMatcherP::POS_MATCH)
						hc.IncrementHeader();
					else if(bMatch == CTextMatcherP::NO_MATCH)
					{
						if(TextIter->MatchAndsInHeader(msg))
							hc.IncrementHeader();
					}
					hc.IncrementTotalHeader();
					break;
				case EMS_MATCH_LOCATION_ANY:
					if(bMatch == CTextMatcherP::POS_MATCH)
						ac.IncrementAny();
					else if(bMatch == CTextMatcherP::NO_MATCH)
					{
						if(TextIter->MatchAndsInSubject(msg) || TextIter->MatchAndsInBody(msg) || TextIter->MatchAndsInHeader(msg) )
							ac.IncrementAny();
					}

					ac.IncrementTotalAny();
					break;
				}
			}

			//DebugReporter::Instance().DisplayMessage("CProcessingRuleList::GetRuleMatch - Ready to test matches", DebugReporter::ENGINE);

			// Were there subjects to test for
			if(sc.HasSubjects())
			{
				// Were there bodies to test for
				if(bc.HasBodys())
				{
					if(sbc.HasSubjectBodys())  // Was there any subject bodys
					{
						// SUBJECT BODY SUBJECTBODYS

						if(hc.HasHeaders()) // Was there any headers
						{
							// SUBJECT BODY SUBJECTBODYS HEADERS

							if(ac.HasAnys()) // Was there anys
							{
								// SUBJECT BODY SUBJECTBODYS HEADERS ANYS
								if(sc.MatchedSubjects() && bc.MatchedBodys() && sbc.MatchedSubjectBodys() && hc.MatchedHeaders() && ac.MatchedAnys())
									bTextMatch = true;
								else
									bTextMatch = false;
							}
							else
							{
								// SUBJECT BODY SUBJECTBODYS HEADERS
								if(sc.MatchedSubjects() && bc.MatchedBodys() && sbc.MatchedSubjectBodys() && hc.MatchedHeaders())
									bTextMatch = true;
								else
									bTextMatch = false;
							}
						}
						else
						{
							// SUBJECT BODY SUBJECTBODYS
							if(ac.HasAnys())
							{
								// SUBJECT BODY SUBJECTBODYS ANYS
								if(sc.MatchedSubjects() && bc.MatchedBodys() && sbc.MatchedSubjectBodys() && ac.MatchedAnys())
									bTextMatch = true;
								else
									bTextMatch = false;
							}
							else
							{
								// SUBJECT BODY SUBJECTBODYS
								if(sc.MatchedSubjects() && bc.MatchedBodys() && sbc.MatchedSubjectBodys())
									bTextMatch = true;
								else
									bTextMatch = false;
							}
						}
					}
					else
					{
						// SUBJECT BODY
						if(hc.HasHeaders())
						{
							// SUBJECT BODY HEADER
							if(ac.HasAnys())
							{
								// SUBJECT BODY HEADER ANY
								if(sc.MatchedSubjects() && bc.MatchedBodys() && hc.MatchedHeaders() && ac.MatchedAnys())
									bTextMatch = true;
								else
									bTextMatch = false;
							}
							else
							{
								// SUBJECT BODY HEADER
								if(sc.MatchedSubjects() && bc.MatchedBodys() && hc.MatchedHeaders())
									bTextMatch = true;
								else
									bTextMatch = false;
							}
						}
						else
						{
							// SUBJECT BODY
							if(ac.HasAnys())
							{
								// SUBJECT BODY ANY
								if(sc.MatchedSubjects() && bc.MatchedBodys() && ac.MatchedAnys())
									bTextMatch = true;
								else
									bTextMatch = false;
							}
							else
							{
								// SUBJECT BODY
								if(sc.MatchedSubjects() && bc.MatchedBodys())
									bTextMatch = true;
								else
									bTextMatch = false;
							}
						}
					}
				}
				else if(sbc.HasSubjectBodys()) // Was there subject bodys
				{
					// SUBJECT SUBJECTBODYS

					if(hc.HasHeaders()) // Was there headers
					{
						// SUBJECT SUBJECTBODYS HEADERS
						if(ac.HasAnys()) // Was there anys
						{
							// SUBJECT SUBJECTBODYS HEADERS ANY
							if(sc.MatchedSubjects() && sbc.MatchedSubjectBodys() && hc.MatchedHeaders() && ac.MatchedAnys())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
						else
						{
							// SUBJECT SUBJECTBODYS HEADERS
							if(sc.MatchedSubjects() && sbc.MatchedSubjectBodys() && hc.MatchedHeaders())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
					}
					else
					{
						// SUBJECT SUBJECTBODYS
						if(ac.HasAnys()) // Was there anys
						{
							// SUBJECT SUBJECTBODYS ANY
							if(sc.MatchedSubjects() && sbc.MatchedSubjectBodys() && ac.MatchedAnys())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
						else
						{
							// SUBJECT SUBJECTBODYS

							if(sc.MatchedSubjects() && sbc.MatchedSubjectBodys())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
					}
				}
				else if(hc.HasHeaders())	// Was there headers
				{
					// SUBJECT HEADERS

					if(ac.HasAnys()) // Was there anys
					{
						// SUBJECT HEADERS ANY
						if(sc.MatchedSubjects() && hc.MatchedHeaders() && ac.MatchedAnys())
							bTextMatch = true;
						else
							bTextMatch = false;
					}
					else
					{
						// SUBJECT HEADERS
						if(sc.MatchedSubjects() && hc.MatchedHeaders())
							bTextMatch = true;
						else
							bTextMatch = false;
					}
				}
				else if(ac.HasAnys()) // Was there anys
				{
					// SUBJECT ANYS
					if(sc.MatchedSubjects() && ac.MatchedAnys())
						bTextMatch = true;
					else
						bTextMatch = false;
				}
				else
				{
					if(sc.MatchedSubjects())
						bTextMatch = true;
					else
						bTextMatch = false;
				}
			}
			else if(bc.HasBodys()) // No Subjects is there bodys
			{
				// BODY
				if(sbc.HasSubjectBodys())  // Was there any subject bodys
				{
					// BODY SUBJECTBODYS

					if(hc.HasHeaders()) // Was there any headers
					{
						//BODY SUBJECTBODYS HEADERS

						if(ac.HasAnys()) // Was there anys
						{
							//BODY SUBJECTBODYS HEADERS ANYS
							if(bc.MatchedBodys() && sbc.MatchedSubjectBodys() && hc.MatchedHeaders() && ac.MatchedAnys())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
						else
						{
							// BODY SUBJECTBODYS HEADERS
							if(bc.MatchedBodys() && sbc.MatchedSubjectBodys() && hc.MatchedHeaders())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
					}
					else
					{
						//BODY SUBJECTBODYS
						if(ac.HasAnys())
						{
							//BODY SUBJECTBODYS ANYS
							if(bc.MatchedBodys() && sbc.MatchedSubjectBodys() && ac.MatchedAnys())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
						else
						{
							//BODY SUBJECTBODYS
							if(bc.MatchedBodys() && sbc.MatchedSubjectBodys())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
					}
				}
				else
				{
					//BODY
					if(hc.HasHeaders())
					{
						//BODY HEADER
						if(ac.HasAnys())
						{
							//BODY HEADER ANY
							if(bc.MatchedBodys() && hc.MatchedHeaders() && ac.MatchedAnys())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
						else
						{
							//BODY HEADER
							if(bc.MatchedBodys() && hc.MatchedHeaders())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
					}
					else
					{
						// BODY
						if(ac.HasAnys())
						{
							//BODY ANY
							if(bc.MatchedBodys() && ac.MatchedAnys())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
						else
						{
							//BODY
							if(bc.MatchedBodys())
								bTextMatch = true;
							else
								bTextMatch = false;
						}
					}
				}
			}
			else if(sbc.HasSubjectBodys())  // No Subjects, Bodys is there subjectbodys
			{
				//SUBJECTBODYS
				if(hc.HasHeaders()) // Was there headers
				{
					//SUBJECTBODYS HEADERS
					if(ac.HasAnys()) // Was there anys
					{
						//SUBJECTBODYS HEADERS ANY
						if(sbc.MatchedSubjectBodys() && hc.MatchedHeaders() && ac.MatchedAnys())
							bTextMatch = true;
						else
							bTextMatch = false;
					}
					else
					{
						//SUBJECTBODYS HEADERS
						if(sbc.MatchedSubjectBodys() && hc.MatchedHeaders())
							bTextMatch = true;
						else
							bTextMatch = false;
					}
				}
				else
				{
					//SUBJECTBODYS
					if(ac.HasAnys()) // Was there anys
					{
						//SUBJECTBODYS ANY
						if(sbc.MatchedSubjectBodys() && ac.MatchedAnys())
							bTextMatch = true;
						else
							bTextMatch = false;
					}
					else
					{
						//SUBJECTBODYS

						if(sbc.MatchedSubjectBodys())
							bTextMatch = true;
						else
							bTextMatch = false;
					}
				}
			}
			else if(hc.HasHeaders())	// No Subjects, Bodys, Subject bodys, is there headers
			{
				//HEADERS
				if(ac.HasAnys()) // Was there anys
				{
					//HEADERS ANY
					if(hc.MatchedHeaders() && ac.MatchedAnys())
						bTextMatch = true;
					else
						bTextMatch = false;
				}
				else
				{
					//HEADERS
					if(hc.MatchedHeaders())
						bTextMatch = true;
					else
						bTextMatch = false;
				}
			}
			else if(ac.HasAnys()) // No Subjects, Bodys, Subject bodys, Headers is there anys
			{
				//ANYS
				if(ac.MatchedAnys())
					bTextMatch = true;
				else
					bTextMatch = false;
			}
		}
		dca::String x(RuleIter->m_RuleDescrip);
		e.Format("CProcessingRuleList::GetRuleMatch - Rule [ %s ] match text = %d", x.c_str(), bTextMatch);
		//DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
		
		// Must pass text match
		if( bTextMatch )
		{
			e.Format("CProcessingRuleList::GetRuleMatch - Rule [ %s ] matched text", x.c_str());
			//DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

			return &(*RuleIter);
		}
		
	}

	return NULL;	
}




////////////////////////////////////////////////////////////////////////////////
// 
// FindRule
// 
////////////////////////////////////////////////////////////////////////////////
CProcessingRule* CProcessingRuleList::FindRule( unsigned int ProcessingRuleID )
{
	vector<CProcessingRule>::iterator RuleIter;

	for(RuleIter = m_list.begin(); RuleIter != m_list.end(); ++RuleIter)
	{
		if( RuleIter->m_ProcessingRuleID == ProcessingRuleID )
			return &(*RuleIter);
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Parse the match text into positive and negative word lists.
// 
////////////////////////////////////////////////////////////////////////////////
void CTextMatcherP::BuildWordLists(void)
{
	wchar_t wcsWordBuffer[MaxMatchTextWordLength+1];
	wchar_t* p = m_MatchText;
	wchar_t* end = p + wcslen( m_MatchText );
	wchar_t* w;
	BOOL bInWord = FALSE;
	BOOL bQuotedWord = FALSE;
	BOOL bPositiveMatch = FALSE;
	BOOL bAndPosMatch = FALSE;
	BOOL bAndNegMatch = FALSE;

	m_pos.clear();
	m_neg.clear();
	m_ands.clear();

	if(m_IsRegEx)
	{
		m_pos.push_back( wstring(m_MatchText) );
	}
	else
	{
		while( p <= end )
		{
			if( bInWord )
			{
				// This condition handles the forms +/- modifiers on quoted phrases
				if( (bQuotedWord == FALSE) && (w == wcsWordBuffer) && (*p == L'"'))
				{
					bQuotedWord = TRUE;
				}
				// This checks for the end of a word or phrase.
				else if( ((bQuotedWord == FALSE) && (IS_WHITE_SPACE(*p)||p==end)) 
					|| ((w - wcsWordBuffer ) > MaxMatchTextWordLength)
					|| ((bQuotedWord == TRUE) && (*p == L'"')) )
				{
					bInWord = FALSE;
					*w = 0;
					if( bPositiveMatch && !bAndPosMatch)
					{
						m_pos.push_back( wstring(wcsWordBuffer) );
					}
					else if( bPositiveMatch && bAndPosMatch)
					{
						m_ands.push_back( wstring(wcsWordBuffer) );
					}
					else if( !bPositiveMatch && bAndNegMatch)
					{
						m_negands.push_back( wstring(wcsWordBuffer) );
					}
					else
					{
						m_neg.push_back( wstring(wcsWordBuffer) );					
					}
				}
				else
				{
					// Add another character to the word buffer
					*w++ = *p;
				}
			}
			else
			{
				if( IS_WHITE_SPACE(*p) )
				{
					// ignore
				}
				else if( *p == L'"' )
				{
					bPositiveMatch = TRUE;
					bQuotedWord = TRUE;
					bInWord = TRUE;
					bAndPosMatch = FALSE;
					bAndNegMatch = FALSE;
					w = wcsWordBuffer;
				}
				else if ( *p == L'+' )
				{
					bPositiveMatch = TRUE;
					bQuotedWord = FALSE;
					bInWord = TRUE;
					bAndPosMatch = FALSE;
					bAndNegMatch = FALSE;
					w = wcsWordBuffer;
				}
				else if ( *p == L'-' )
				{
					bPositiveMatch = FALSE;
					bQuotedWord = FALSE;
					bInWord = TRUE;
					bAndPosMatch = FALSE;
					bAndNegMatch = FALSE;
					w = wcsWordBuffer;
				}
				else if ( *p == L'&' )
				{
					bPositiveMatch = TRUE;
					bQuotedWord = FALSE;
					bInWord = TRUE;
					bAndPosMatch = TRUE;
					bAndNegMatch = FALSE;
					w = wcsWordBuffer;
				}
				else if( * p == L'^' )
				{
					bPositiveMatch = FALSE;
					bQuotedWord = FALSE;
					bInWord = TRUE;
					bAndPosMatch = FALSE;
					bAndNegMatch = TRUE;
					w = wcsWordBuffer;
				}
				else
				{
					bPositiveMatch = TRUE;
					bQuotedWord = FALSE;
					bInWord = TRUE;
					w = wcsWordBuffer;
					*w++ = *p;
				}
			}
			p++;
		}
	}	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Run through the wordlists for this message and return true on match.
// 
////////////////////////////////////////////////////////////////////////////////
CTextMatcherP::MATCH_TYPE CTextMatcherP::Match( CInboundMsg& msg )
{
	//DebugReporter::Instance().DisplayMessage("CTextMatcherP::Match - entered", DebugReporter::ENGINE);

	vector<wstring>::iterator iter;
	MATCH_TYPE bPosMatch = NO_MATCH;
	
	if(m_pos.size() > 0)
	{
		for(iter = m_pos.begin(); iter != m_pos.end(); ++iter)
		{
			bHtmlToText = false;
			if( EMS_MATCH_SUBJECT( m_MatchLocation ) )
			{
				
				if ( m_IsRegEx )
				{
					// Test subject against RegEx
					if( TestRegEx( iter->c_str(), msg.m_Subject ) )
					{
						bPosMatch = POS_MATCH;
						break;
					}
				}
				else
				{
					// Search subject for positive match
					if( FindString( iter->c_str(), msg.m_Subject ) )
					{
						bPosMatch = POS_MATCH;
						break;
					}
				}				
			}
			
			if( EMS_MATCH_BODY( m_MatchLocation ) )
			{
				if ( m_IsRegEx )
				{
					// Test body against RegEx
					bHtmlToText = true;
					if( TestRegEx( iter->c_str(), msg.m_Body ) )
					{
						bPosMatch = POS_MATCH;
						break;
					}
				}
				else
				{
					// Search body for positive match
					if( FindString( iter->c_str(), msg.m_Body ) )
					{
						bPosMatch = POS_MATCH;
						break;
					}
				}				
			}
			
			if( EMS_MATCH_HEADERS( m_MatchLocation ) )
			{
				if ( m_IsRegEx )
				{
					// Test headers against RegEx
					if( TestRegEx( iter->c_str(), msg.m_PopHeaders ) )
					{
						bPosMatch = POS_MATCH;
						break;
					}
				}
				else
				{
					// Search headers for positive match
					if( FindString( iter->c_str(), msg.m_PopHeaders ) )
					{
						bPosMatch = POS_MATCH;
						break;
					}
				}				
			}

		}

		if(!bPosMatch)
		{
			return bPosMatch;
		}
	}	

	//DebugReporter::Instance().DisplayMessage("CTextMatcherP::Match - testing for negative matches", DebugReporter::ENGINE);
	
	for(iter = m_neg.begin(); iter != m_neg.end(); ++iter)
	{
		if( EMS_MATCH_SUBJECT( m_MatchLocation ) )
		{
			// Search subject for negative match
			if( FindString( iter->c_str(), msg.m_Subject ) )
			{
				return NEG_MATCH;
			}
		}
		
		if( EMS_MATCH_BODY( m_MatchLocation ) )
		{
			// Search body for negative match
			if( FindString( iter->c_str(), msg.m_Body ) )
			{
				return NEG_MATCH;
			}
		}

		if( EMS_MATCH_HEADERS( m_MatchLocation ) )
		{
			// Search body for negative match
			if( FindString( iter->c_str(), msg.m_PopHeaders ) )
			{
				return NEG_MATCH;
			}
		}
	}

	//DebugReporter::Instance().DisplayMessage("CTextMatcherP::Match - returning", DebugReporter::ENGINE);

	return bPosMatch;
}

bool CTextMatcherP::MatchAndsInBody( CInboundMsg& msg )
{
	vector<wstring>::iterator iter;
	bool bPosMatch = false;
	size_t nFound = 0;

	if(m_ands.size() > 0)
	{
		for(iter = m_ands.begin(); iter != m_ands.end(); ++iter)
		{
			if(EMS_MATCH_BODY( m_MatchLocation ))
			{
				// Search body for positive match
				if( FindString( iter->c_str(), msg.m_Body ) )
					++nFound;
			}
		}

		if(m_ands.size() == nFound)
			bPosMatch = true;
	}

	nFound = 0;

	if(m_negands.size() > 0)
	{
		for(iter = m_negands.begin(); iter != m_negands.end(); ++iter)
		{
			if(EMS_MATCH_BODY( m_MatchLocation ))
			{
				// Search body for negative match
				if( FindString( iter->c_str(), msg.m_Body ) )
					++nFound;
			}
		}

		if(m_negands.size() == nFound)
			bPosMatch = false;
	}

	return bPosMatch;
}

bool CTextMatcherP::MatchAndsInSubject( CInboundMsg& msg )
{
	vector<wstring>::iterator iter;
	bool bPosMatch = false;
	size_t nFound = 0;

	if(m_ands.size() > 0)
	{
		for(iter = m_ands.begin(); iter != m_ands.end(); ++iter)
		{
			if(EMS_MATCH_SUBJECT( m_MatchLocation ))
			{
				// Search body for positive match
				if( FindString( iter->c_str(), msg.m_Subject ) )
					++nFound;
			}
		}

		if(m_ands.size() == nFound)
			bPosMatch = true;
	}

	nFound = 0;

	if(m_negands.size() > 0)
	{
		for(iter = m_negands.begin(); iter != m_negands.end(); ++iter)
		{
			if(EMS_MATCH_SUBJECT( m_MatchLocation ))
			{
				// Search body for positive match
				if( FindString( iter->c_str(), msg.m_Subject ) )
					++nFound;
			}
		}

		if(m_negands.size() == nFound)
			bPosMatch = false;
	}

	return bPosMatch;
}

bool CTextMatcherP::MatchAndsInHeader( CInboundMsg& msg )
{
	vector<wstring>::iterator iter;
	bool bPosMatch = false;
	size_t nFound = 0;

	if(m_ands.size() > 0)
	{
		for(iter = m_ands.begin(); iter != m_ands.end(); ++iter)
		{
			if(EMS_MATCH_HEADERS( m_MatchLocation ))
			{
				// Search body for positive match
				if( FindString( iter->c_str(), msg.m_PopHeaders  ) )
					++nFound;
			}
		}

		if(m_ands.size() == nFound)
			bPosMatch = true;
	}

	nFound = 0;

	if(m_negands.size() > 0)
	{
		for(iter = m_negands.begin(); iter != m_negands.end(); ++iter)
		{
			if(EMS_MATCH_HEADERS( m_MatchLocation ))
			{
				// Search body for positive match
				if( FindString( iter->c_str(), msg.m_PopHeaders  ) )
					++nFound;
			}
		}

		if(m_negands.size() == nFound)
			bPosMatch = false;
	}

	return bPosMatch;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FindString does a case-insensitive wcsstr()
// 
////////////////////////////////////////////////////////////////////////////////
bool CTextMatcherP::FindString(  const wchar_t* szString, wchar_t* szBuffer )
{
	//DebugReporter::Instance().DisplayMessage("CTextMatcherP::FindString - entered.", DebugReporter::ENGINE);

	dca::String m(szBuffer);
	dca::String f;
	f.Format("CTextMatcherP::FindString - buffer to match %s", m.c_str());
	//DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);

	tstring sBuffer;
	long StringLen = wcslen( szString );
	long BufLen;
	long i = 0;
	size_t pos = 0;
	TCHAR szSpaceChars[] = {0x0009, 0x000D, 0x000A};  // Tab, CR, LF
	
	// prepend and append a space character to the buffer so that
	// exact word matches (e.g. " word ") will match when the word
	// is at the beginning or end of a buffer
	sBuffer.assign( _T(" ") );
	sBuffer.append( szBuffer );
	sBuffer.append( _T(" ") );

	//DebugReporter::Instance().DisplayMessage("CTextMatcherP::FindString - start converting characters", DebugReporter::ENGINE);

	// now convert non-space characters to spaces in the buffer
	pos = sBuffer.find_first_of(szSpaceChars, 0);
	while( pos != tstring::npos )
	{
		//DebugReporter::Instance().DisplayMessage("CTextMatcherP::FindString - start converting multiple space characters", DebugReporter::ENGINE);

		// convert multiple space characters to one
		while( (pos + 1) < sBuffer.size() && wcschr( szSpaceChars, sBuffer[pos + 1]) )
		{
			sBuffer = sBuffer.erase(pos + 1, 1);
		}

		//DebugReporter::Instance().DisplayMessage("CTextMatcherP::FindString - end converting multiple space characters", DebugReporter::ENGINE);

		sBuffer[pos] = 0x0020; // Space

		pos++;

		pos = sBuffer.find_first_of(szSpaceChars, pos);
	}

	//DebugReporter::Instance().DisplayMessage("CTextMatcherP::FindString - done converting characters", DebugReporter::ENGINE);

	BufLen = sBuffer.size() - StringLen + 1;

	//DebugReporter::Instance().DisplayMessage("CTextMatcherP::FindString - start comparing characters", DebugReporter::ENGINE);

	while ( i < BufLen )
	{
		if( wcsnicmp( szString, sBuffer.c_str() + i, StringLen ) == 0 )
		{
			//DebugReporter::Instance().DisplayMessage("CTextMatcherP::FindString - returning true", DebugReporter::ENGINE);
			return true;
		}

		i++;
	}

	//DebugReporter::Instance().DisplayMessage("CTextMatcherP::FindString - end comparing characters", DebugReporter::ENGINE);

	//DebugReporter::Instance().DisplayMessage("CTextMatcherP::FindString - returning false", DebugReporter::ENGINE);

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// TestRegEx tests the message part against the RegEx
// 
////////////////////////////////////////////////////////////////////////////////
bool CTextMatcherP::TestRegEx(  const wchar_t* szString, wchar_t* szBuffer )
{
	// szString is the RegEx - szBuffer is the message part

	DebugReporter::Instance().DisplayMessage("CTextMatcherP::TestRegex - entered.", DebugReporter::ENGINE);

	dca::String m;
	tstring sM(szBuffer);

	CkGlobal m_glob;
	// unlock Chilkat
	bool success = m_glob.UnlockBundle("DEERFL.CB1122026_MEQCIEmYvJKZHXPMP+mW32ewkexb1stbg9+mp2kG+Ewh1XXYAiAwgg6XL+3vWAVa1r7eZrMCvInwy4Qil2j/u1pgQNuD8A==");
	if (success != true)
	{
		DebugReporter::Instance().DisplayMessage("CTextMatcherP::TestRegEx() - Failed to unlock Global component", DebugReporter::MAIL);
		Log(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_ROUTING_ENGINE,
									EMS_LOG_INBOUND_MESSAGING,
									0),
						EMS_STRING_ERROR_INITIALIZE_COMPONENT);

		return false;
	}

	int nChrSize = WideCharToMultiByte( CP_ACP, 0, sM.c_str(), sM.size(), NULL, 0, NULL, NULL );
	char* html = new char[nChrSize+1];
	WideCharToMultiByte( CP_ACP, 0, sM.c_str(), sM.size(), html, nChrSize, NULL, NULL );
	html[nChrSize] = 0;

	const char * plainText;

	plainText = h2t.toText(html);

	if(bHtmlToText)
	{
		m.assign(plainText);
		bHtmlToText = false;
	}
	else
	{
		m.assign(html);
	}
	
	dca::String f;
	f.Format("CTextMatcherP::TestRegEx - message part to test: %s", m.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);

	dca::String s(szString);	
	f.Format("CTextMatcherP::TestRegEx - RegEx: %s", s.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);
	
	try
	{
		boost::regex pattern (s,boost::regex_constants::perl);
		
		if (boost::regex_search (m, pattern, boost::regex_constants::format_perl))
		{
			DebugReporter::Instance().DisplayMessage("CTextMatcherP::TestRegEx - found match", DebugReporter::ENGINE);
			return true;
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CTextMatcherP::TestRegEx - match not found", DebugReporter::ENGINE);
			return false;
		}
	}
	catch(...)
	{
		DebugReporter::Instance().DisplayMessage("CTextMatcherP::TestRegEx - invalid RegEx", DebugReporter::ENGINE);
		return false;
	}
}