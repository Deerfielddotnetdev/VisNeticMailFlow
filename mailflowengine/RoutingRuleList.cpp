// RoutingRules.cpp: implementation of the CRoutingRuleList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RoutingRuleList.h"
#include <.\boost\regex.hpp>
#include "DateFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRoutingRuleList::CRoutingRuleList()
{
	m_LastRefreshTime = 0;	// Never	
}

CRoutingRuleList::~CRoutingRuleList()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// SelectRulesInOrder - Used by Routing Engine
// 
////////////////////////////////////////////////////////////////////////////////
void CRoutingRule::SelectRulesInOrder( CODBCQuery& query )
{
	query.Initialize();

	BINDCOL_LONG( query, m_RoutingRuleID );
	BINDCOL_LONG( query, m_AutoReplyWithStdResponse );
	BINDCOL_BIT( query, m_AutoReplyQuoteMsg );
	BINDCOL_BIT( query, m_AutoReplyCloseTicket );
	BINDCOL_LONG( query, m_ForwardFromAgent );
	BINDCOL_BIT( query, m_ForwardFromContact );
	BINDCOL_LONG( query, m_AssignToTicketBox );
	BINDCOL_TINYINT( query, m_DeleteImmediately );
	BINDCOL_LONG( query, m_HitCount );
	BINDCOL_LONG( query, m_PriorityID );
	BINDCOL_WCHAR( query, m_RuleDescrip );
	BINDCOL_LONG( query, m_AssignToAgent );
	BINDCOL_BIT( query, m_AutoReplyEnable );
	BINDCOL_BIT( query, m_ForwardEnable );
	BINDCOL_WCHAR( query, m_AgentName ); // Add to SELECT list
	BINDCOL_WCHAR( query, m_AgentAddress );
	BINDCOL_LONG( query, m_MessageSourceTypeID );
	BINDCOL_BIT( query, m_AssignToAgentEnable );
	BINDCOL_BIT( query, m_DeleteImmediatelyEnable );
	BINDCOL_WCHAR( query, m_TicketboxName );
	BINDCOL_WCHAR( query, m_AutoReplyFrom );
	BINDCOL_LONG( query, m_AssignToAgentAlg );
	BINDCOL_LONG( query, m_AssignToTicketBoxAlg );
	BINDCOL_BIT( query, m_AssignUniqueTicketID );
	BINDCOL_LONG( query, m_AssignToTicketCategory );
	BINDCOL_BIT( query, m_AlertEnable );
	BINDCOL_BIT( query, m_AlertIncludeSubject );
	BINDCOL_LONG( query, m_AlertToAgentID );
	BINDCOL_WCHAR( query, m_AlertText );
	BINDCOL_BIT( query, m_ToOrFrom );
	BINDCOL_BIT( query, m_ConsiderAllOwned );
	BINDCOL_BIT( query, m_DoProcessingRules );
	BINDCOL_LONG( query, m_LastOwnerID );
	BINDCOL_LONG( query, m_MatchMethod );
	BINDCOL_WCHAR( query, m_ForwardFromEmail );
	BINDCOL_WCHAR( query, m_ForwardFromName );
	BINDCOL_BIT( query, m_ForwardInTicket );
	BINDCOL_BIT( query, m_AutoReplyInTicket );
	BINDCOL_BIT( query, m_AllowRemoteReply );
	BINDCOL_LONG( query, m_DoNotAssign );
	BINDCOL_BIT( query, m_QuoteOriginal );
	BINDCOL_LONG( query, m_MultiMail );
	BINDCOL_LONG( query, m_SetOpenOwner );
	BINDCOL_LONG( query, m_OfficeHours );
	BINDCOL_LONG( query, m_IgnoreTracking );
	query.Execute( L"SELECT RoutingRuleID,R.AutoReplyWithStdResponse," 
				   L"R.AutoReplyQuoteMsg,R.AutoReplyCloseTicket,"
				   L"ForwardFromAgent,ForwardFromContact,"
				   L"AssignToTicketBox,DeleteImmediately,"
				   L"HitCount,PriorityID,RuleDescrip,AssignToAgent,"
				   L"R.AutoReplyEnable,ForwardEnable,A.Name,DataValue,MessageSourceTypeID,"
				   L"AssignToAgentEnable,DeleteImmediatelyEnable,T.Name,AutoReplyFrom, "
				   L"AssignToAgentAlg,AssignToTicketBoxAlg,R.AssignUniqueTicketID,R.AssignToTicketCategory, "
				   L"R.AlertEnable,R.AlertIncludeSubject,R.AlertToAgentID,R.AlertText,R.ToOrFrom,R.ConsiderAllOwned,R.DoProcessingRules,R.LastOwnerID,R.MatchMethod,R.ForwardFromEmail,R.ForwardFromName,R.ForwardInTicket,R.AutoReplyInTicket,R.AllowRemoteReply,R.DoNotAssign,R.QuoteOriginal,R.MultiMail,R.SetOpenOwner,R.OfficeHours,R.IgnoreTracking "
				   L"FROM RoutingRules AS R "
				   L"LEFT OUTER JOIN TicketBoxes AS T ON R.AssignToTicketBox = T.TicketBoxID "
				   L"LEFT OUTER JOIN Agents AS A ON R.ForwardFromAgent = A.AgentID "
				   L"LEFT OUTER JOIN PersonalData AS P ON A.DefaultAgentAddressID = P.PersonalDataID "
	  			   L"WHERE R.IsEnabled=1  "
				   L"ORDER BY OrderIndex" );

	int x = 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Refresh - Refreshes the list of routing rules from the DB
// 
////////////////////////////////////////////////////////////////////////////////
void CRoutingRuleList::Refresh( CODBCQuery& query )
{
	CRoutingRule rule;
	vector<CRoutingRule>::iterator iter;
	CAddressToMatcher mt;
	CAddressFromMatcher mf;
	CTextMatcher mtext;
	CForwardToAddr fta;
	CForwardCCAddr fca;

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
		SYSTEMTIME curTime;
		ZeroMemory(&curTime, sizeof(SYSTEMTIME));
		GetLocalTime(&curTime);
		bool bIsInOfficeHours = false;
		bool bOfficeHoursSet = false;
		bool bExceptionSet = false;
		int nDayOfWeek = curTime.wDayOfWeek + 1;

		TIMESTAMP_STRUCT Now;
		GetTimeStamp( Now );

		vector<OfficeHours_t> too;
		vector<OfficeHours_t>::iterator tooIter;

		query.Initialize();					
		OfficeHours_t oh;
		BINDCOL_LONG( query, oh.m_TypeID );
		BINDCOL_LONG( query, oh.m_StartHr );
		BINDCOL_LONG( query, oh.m_StartMin );
		BINDCOL_LONG( query, oh.m_StartAmPm );
		BINDCOL_LONG( query, oh.m_EndHr );
		BINDCOL_LONG( query, oh.m_EndMin );
		BINDCOL_LONG( query, oh.m_EndAmPm );
		BINDCOL_LONG( query, oh.m_ActualID );		
		BINDPARAM_TIME_NOLEN( query, Now );
		BINDPARAM_LONG( query, nDayOfWeek );					
		BINDPARAM_LONG( query, nDayOfWeek );					
		query.Execute(	_T("SELECT TypeID,StartHr,StartMin,StartAmPm,EndHr,EndMin,EndAmPm,ActualID ")
						_T("FROM OfficeHours ")
						_T("WHERE (? BETWEEN TimeStart AND TimeEnd AND TypeID IN (1,2)) OR (TypeID=0 AND ActualID=0 AND DayID=?) OR (TypeID=9 AND ActualID<>0 AND DayID=?)")
						_T("ORDER BY OfficeHourID DESC"));
		while( query.Fetch() == S_OK )
		{
			if(oh.m_TypeID == 1)
			{
				bIsInOfficeHours = false;
				bExceptionSet = true;
				break;
			}
			else if(oh.m_TypeID == 2)
			{
				bIsInOfficeHours = true;
				bExceptionSet = true;
				break;
			}
			else
			{
				too.push_back(oh);
			}
		}
		
		rule.SelectRulesInOrder( query );

		// Clear the list
		m_list.clear();

		while( query.Fetch() == S_OK )
		{
			rule.nOoo= 0;
			rule.nOffline= 0;
			rule.nDnd= 0;
			rule.nNotAvail= 0;
			rule.nAway= 0;
			rule.nOnline= 0;
			bool bAddRule = true;
			bOfficeHoursSet = false;

			if(rule.m_DoNotAssign%2!=0){rule.nOoo=1;}
			int nVal;
			CEMSString sNums;
			sNums.Format(_T("2,3,6,7,10,11,14,15,18,19,22,23,26,27,30,31,34,35,38,39,42,43,46,47,50,51,54,55,58,59,62,63"));
			sNums.CDLInit();
			while ( sNums.CDLGetNextInt( nVal ) )
			{
				if ( nVal == rule.m_DoNotAssign ){rule.nOffline=1;break;}
			}
			sNums.Format(_T("4,5,6,7,12,13,14,15,20,21,23,24,28,29,30,31,36,37,38,39,44,45,46,47,52,53,54,55,60,61,62,63"));
			sNums.CDLInit();
			while ( sNums.CDLGetNextInt( nVal ) )
			{
				if ( nVal == rule.m_DoNotAssign ){rule.nDnd=1;break;}
			}
			sNums.Format(_T("8,9,10,11,12,13,14,15,24,25,26,27,28,29,30,31,40,41,42,43,44,45,46,47,56,57,58,59,60,61,62,63"));
			sNums.CDLInit();
			while ( sNums.CDLGetNextInt( nVal ) )
			{
				if ( nVal == rule.m_DoNotAssign ){rule.nNotAvail=1;break;}
			}
			if((rule.m_DoNotAssign>15 && rule.m_DoNotAssign<32)||(rule.m_DoNotAssign > 47)){rule.nAway=1;}
			if(rule.m_DoNotAssign>31){rule.nOnline=1;}

			// Does this Routing Rule use Office Hours?
			if ( rule.m_OfficeHours > 0)
			{
				if(!bExceptionSet)
				{
					for( tooIter = too.begin(); tooIter != too.end(); tooIter++ )
					{
						if(tooIter->m_ActualID == rule.m_RoutingRuleID && !bOfficeHoursSet)
						{
							bIsInOfficeHours = InOfficeHours(tooIter->m_StartHr,tooIter->m_StartMin,tooIter->m_StartAmPm,tooIter->m_EndHr,tooIter->m_EndMin,tooIter->m_EndAmPm);
							bOfficeHoursSet = true;
						}
						else if(tooIter->m_ActualID == 0 && !bOfficeHoursSet)
						{
							bIsInOfficeHours = InOfficeHours(tooIter->m_StartHr,tooIter->m_StartMin,tooIter->m_StartAmPm,tooIter->m_EndHr,tooIter->m_EndMin,tooIter->m_EndAmPm);
							bOfficeHoursSet = true;
						}
					}
					if(!bOfficeHoursSet)
					{
						bIsInOfficeHours = true;
						bOfficeHoursSet = true;
					}
				}
				if( !bIsInOfficeHours && rule.m_OfficeHours == 1)
				{
					dca::String e;
					e.Format("CRoutingRuleList::Refresh - Routing Rule ID: %d not processed outside Office Hours!", rule.m_RoutingRuleID);
					DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
					bAddRule = false; 
				}
				else if( bIsInOfficeHours && rule.m_OfficeHours == 2)
				{
					dca::String e;
					e.Format("CRoutingRuleList::Refresh - Routing Rule ID: %d not processed during Office Hours!", rule.m_RoutingRuleID);
					DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
					bAddRule = false; 
				}
			}		
			if(bAddRule)
			{
				m_list.push_back( rule );			
			}
		}

		for(iter = m_list.begin(); iter != m_list.end(); ++iter)
		{
			mt.SelectByRoutingRuleID( query, iter->m_RoutingRuleID );

			while( query.Fetch() == S_OK )
			{
				PreprocessPattern( mt.m_EmailAddress );
				iter->m_to.push_back( mt );
			}

			mf.SelectByRoutingRuleID( query, iter->m_RoutingRuleID );

			while( query.Fetch() == S_OK )
			{
				PreprocessPattern( mf.m_EmailAddress );
				iter->m_from.push_back( mf );
			}

			mtext.SelectByRoutingRuleID( query, iter->m_RoutingRuleID );

			while( query.Fetch() == S_OK )
			{
				GETDATA_TEXT( query, mtext.m_MatchText );
				mtext.BuildWordLists();
				iter->m_text.push_back( mtext );
			}

			fta.SelectByRoutingRuleID( query, iter->m_RoutingRuleID );

			while( query.Fetch() == S_OK )
				iter->m_fta.push_back( fta );

			fca.SelectByRoutingRuleID( query, iter->m_RoutingRuleID );

			while( query.Fetch() == S_OK )
				iter->m_fca.push_back( fca );

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
// PreprocessPattern
// 
////////////////////////////////////////////////////////////////////////////////
void CRoutingRuleList::PreprocessPattern( wchar_t* szEmailAddress )
{
	int nLen = wcslen(szEmailAddress);
	
	// Remove a "*" at beginning or end
	if( wcsncmp( szEmailAddress, L"*@", 2 ) == 0 )
	{
		for( int i = 0; i < nLen; i++ )
		{
			szEmailAddress[i] = towupper(szEmailAddress[i+1]);
		}

		return;
	}
	else if ( nLen > 2 && wcsncmp( szEmailAddress + nLen - 2, L"@*", 2 ) == 0 )
	{
		szEmailAddress[nLen-1] = 0;
		nLen--;
	}

	// convert pattern to upper case
	for( int i = 0; i < nLen; i++ )
	{
		szEmailAddress[i] = towupper(szEmailAddress[i]);
	}

}



////////////////////////////////////////////////////////////////////////////////
// 
//  GetRuleMatch walks the list of routing rules and returns a pointer
//  to the matching rule or NULL if no rule matches.
// 
////////////////////////////////////////////////////////////////////////////////
CRoutingRule* CRoutingRuleList::GetRuleMatch( CInboundMsg& msg )
{
	dca::String e;
	e.Format("CRoutingRuleList::GetRuleMatch - entered, rule list size = %d", m_list.size());
	DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

	vector<CRoutingRule>::iterator RuleIter;
	vector<CAddressToMatcher>::iterator ToIter;
	vector<CAddressFromMatcher>::iterator FromIter;
	vector<CTextMatcher>::iterator TextIter;
	bool bSourceMatch,bToMatch,bFromMatch,bTextMatch;

	for(RuleIter = m_list.begin(); RuleIter != m_list.end(); ++RuleIter)
	{
		dca::String x(RuleIter->m_RuleDescrip);

		// MessageSourceType Matching
		bSourceMatch = (RuleIter->m_MessageSourceTypeID == 0);
		if( !bSourceMatch )
		{
			bSourceMatch = (RuleIter->m_MessageSourceTypeID == msg.m_MessageSourceID );
			if( bSourceMatch )
			{
				e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] Message Source matched ID: [%d]", x.c_str(), msg.m_MessageSourceID);
				DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
			}
		}
		else
		{
			e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] Message Source set to All", x.c_str());
			DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
		}
		
		if( bSourceMatch )
		{
			// Does the To: field match?
			bToMatch = (RuleIter->m_to.size() == 0);
			if( !bToMatch )
			{
				for(ToIter = RuleIter->m_to.begin(); ToIter != RuleIter->m_to.end(); ++ToIter)
				{
					if( ToIter->Match( msg ) )
					{
						dca::String sToMatch(ToIter->m_EmailAddress);
						e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] matched To: email address [%s]", x.c_str(), sToMatch.c_str());
						DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
						bToMatch = true;
						break;
					}							
				}
				if(!bToMatch)
				{
					e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] To: resulted in no match", x.c_str());
					DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
				}
			}
			else
			{
				e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] To: is empty", x.c_str());
				DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
			}

			// Does the From: field match?
			bFromMatch = (RuleIter->m_from.size() == 0);
			if( !bFromMatch )
			{
				for(FromIter = RuleIter->m_from.begin(); FromIter != RuleIter->m_from.end(); ++FromIter)
				{
					if( FromIter->Match( msg ) )
					{
						dca::String sFromMatch(FromIter->m_EmailAddress);
						e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] matched From: email address [%s]", x.c_str(), sFromMatch.c_str());
						DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
						bFromMatch = true;
						break;
					}		
				}
				if(!bFromMatch)
				{
					e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] From: resulted in no match", x.c_str());
					DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
				}
			}
			else
			{
				e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] From: is empty", x.c_str());
				DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
			}

			if ( RuleIter->m_ToOrFrom )
			{
				if( (bToMatch || bFromMatch) )
				{
					e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] To: OR From: matched one or both", x.c_str());
					DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);					
				}
				else
				{
					e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] To: OR From: resulted in no matches", x.c_str());
					DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
					continue;
				}
			}
			else
			{
				if( bToMatch && bFromMatch)
				{
					e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] To: AND From: matched both", x.c_str());
					DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
				}
				else
				{
					e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] To: AND From: resulted in one or no matches", x.c_str());
					DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
					continue;
				}
			}

			// Does the subject and/or body match?
			bTextMatch = (RuleIter->m_text.size() == 0);
			
			SubjectCount     sc;
			BodyCount        bc;
			SubjectBodyCount sbc;
			HeaderCount      hc;
			AnyCount         ac;

			CTextMatcher::MATCH_TYPE bMatch = CTextMatcher::NO_MATCH;

			if( !bTextMatch )
			{
				e.Format("CRoutingRuleList::GetRuleMatch - Text size is %d", RuleIter->m_text.size());
				DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

				for(TextIter = RuleIter->m_text.begin(); TextIter != RuleIter->m_text.end(); ++TextIter)
				{
					dca::String text1(TextIter->m_MatchText);
					e.Format("CRoutingRuleList::GetRuleMatch - Ready to match [%s] with InboundMessageID [%d]", text1.c_str(), msg.m_InboundMessageID);
					DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

					// Do we have a match?
					bMatch = TextIter->Match(msg);

					e.Format("CRoutingRuleList::GetRuleMatch - have match = %d and location = %d", bMatch, TextIter->m_MatchLocation);
					DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

					// What location were we looking at
					switch(TextIter->m_MatchLocation)
					{
					case EMS_MATCH_LOCATION_SUBJECT:
						{
							// Did we have a match
							if(bMatch == CTextMatcher::POS_MATCH)
								sc.IncrementSubject();	// Yes then increment number of subjects had a match
							else if(bMatch == CTextMatcher::NO_MATCH)
							{
								if(TextIter->MatchAndsInSubject(msg))
									sc.IncrementSubject();
							}

							sc.IncrementTotalSubject();	// Increment number of subjects we had both positive and negative
						}
						break;
					case EMS_MATCH_LOCATION_BODY:
						{
							if(bMatch == CTextMatcher::POS_MATCH)
								bc.IncrementBody();
							else if(bMatch == CTextMatcher::NO_MATCH)
							{
								if(TextIter->MatchAndsInBody(msg))
									bc.IncrementBody();
							}

							bc.IncrementTotalBody();
						}
						break;
					case EMS_MATCH_LOCATION_SUBJECT_OR_BODY:
						if(bMatch == CTextMatcher::POS_MATCH)
							sbc.IncrementSubjectBody();
						else if(bMatch == CTextMatcher::NO_MATCH)
						{
							if(TextIter->MatchAndsInSubject(msg) || TextIter->MatchAndsInBody(msg))
								sbc.IncrementSubjectBody();
						}

						sbc.IncrementTotalSubjectBody();
						break;
					case EMS_MATCH_LOCATION_HEADERS:
						if(bMatch == CTextMatcher::POS_MATCH)
							hc.IncrementHeader();
						else if(bMatch == CTextMatcher::NO_MATCH)
						{
							if(TextIter->MatchAndsInHeader(msg))
								hc.IncrementHeader();
						}
						hc.IncrementTotalHeader();
						break;
					case EMS_MATCH_LOCATION_ANY:
						if(bMatch == CTextMatcher::POS_MATCH)
							ac.IncrementAny();
						else if(bMatch == CTextMatcher::NO_MATCH)
						{
							if(TextIter->MatchAndsInSubject(msg) || TextIter->MatchAndsInBody(msg) || TextIter->MatchAndsInHeader(msg) )
								ac.IncrementAny();
						}

						ac.IncrementTotalAny();
						break;
					}
				}

				DebugReporter::Instance().DisplayMessage("CRoutingRuleList::GetRuleMatch - Ready to test matches", DebugReporter::ENGINE);

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

			e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] match text = %d", x.c_str(), bTextMatch);
			DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);		

		}
		else
		{
			e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] Message Source set to ID: [%d] did not match message collected by ID: [%d]", x.c_str(), RuleIter->m_MessageSourceTypeID, msg.m_MessageSourceID);
			DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);
			continue;
		}
		
		if ( (*RuleIter).m_ToOrFrom )
		{
			// Must pass message source To or From address and match text
			if( bSourceMatch && (bToMatch || bFromMatch) && bTextMatch)
			{
				e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] matched all", x.c_str());
				DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

				unsigned char t = (*RuleIter).m_AssignUniqueTicketID;

				return &(*RuleIter);
			}
		}
		else
		{
			// Must pass all 4 tests to get a match
			if( bSourceMatch && bToMatch && bFromMatch && bTextMatch)
			{
				e.Format("CRoutingRuleList::GetRuleMatch - Rule [%s] matched all", x.c_str());
				DebugReporter::Instance().DisplayMessage(e.c_str(), DebugReporter::ENGINE);

				unsigned char t = (*RuleIter).m_AssignUniqueTicketID;

				return &(*RuleIter);
			}
		}
	}

	return NULL;	
}


////////////////////////////////////////////////////////////////////////////////
// 
// FindRule
// 
////////////////////////////////////////////////////////////////////////////////
CRoutingRule* CRoutingRuleList::FindRule( unsigned int RoutingRuleID )
{
	vector<CRoutingRule>::iterator RuleIter;

	for(RuleIter = m_list.begin(); RuleIter != m_list.end(); ++RuleIter)
	{
		if( RuleIter->m_RoutingRuleID == RoutingRuleID )
			return &(*RuleIter);
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// 
// MatchEmailAddress
// 
////////////////////////////////////////////////////////////////////////////////
inline bool MatchEmailAddress( wchar_t* szAddress, wchar_t* szPattern )
{
	if( wcsstr( szAddress, szPattern ) != NULL )
	{
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// CAddressToMatcher::Match
// 
////////////////////////////////////////////////////////////////////////////////
bool CAddressToMatcher::Match( CInboundMsg& msg )
{
	if( MatchEmailAddress( msg.szMatchEmailTo, m_EmailAddress ) )
		return true;

	if( MatchEmailAddress( msg.szMatchEmailCc, m_EmailAddress ) )
		return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// CAddressFromMatcher::Match
// 
////////////////////////////////////////////////////////////////////////////////
bool CAddressFromMatcher::Match( CInboundMsg& msg )
{
	if( MatchEmailAddress( msg.szMatchEmailFrom, m_EmailAddress ) )
		return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Parse the match text into positive and negative word lists.
// 
////////////////////////////////////////////////////////////////////////////////
void CTextMatcher::BuildWordLists(void)
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
CTextMatcher::MATCH_TYPE CTextMatcher::Match( CInboundMsg& msg )
{
	//DebugReporter::Instance().DisplayMessage("CTextMatcher::Match - entered", DebugReporter::ENGINE);

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

	//DebugReporter::Instance().DisplayMessage("CTextMatcher::Match - testing for negative matches", DebugReporter::ENGINE);
	
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

	//DebugReporter::Instance().DisplayMessage("CTextMatcher::Match - returning", DebugReporter::ENGINE);

	return bPosMatch;
}

bool CTextMatcher::MatchAndsInBody( CInboundMsg& msg )
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

bool CTextMatcher::MatchAndsInSubject( CInboundMsg& msg )
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

bool CTextMatcher::MatchAndsInHeader( CInboundMsg& msg )
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
bool CTextMatcher::FindString(  const wchar_t* szString, wchar_t* szBuffer )
{
	DebugReporter::Instance().DisplayMessage("CTextMatcher::FindString - entered.", DebugReporter::ENGINE);

	dca::String y(szString);
	dca::String f;
	f.Format("CTextMatcher::FindString - String to find [%s]", y.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);

	dca::String m(szBuffer);
	f.Format("CTextMatcher::FindString - String to search [%s]", m.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);
	
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

	//DebugReporter::Instance().DisplayMessage("CTextMatcher::FindString - start converting characters", DebugReporter::ENGINE);

	// now convert non-space characters to spaces in the buffer
	pos = sBuffer.find_first_of(szSpaceChars, 0);
	while( pos != tstring::npos )
	{
		//DebugReporter::Instance().DisplayMessage("CTextMatcher::FindString - start converting multiple space characters", DebugReporter::ENGINE);

		// convert multiple space characters to one
		while( (pos + 1) < sBuffer.size() && wcschr( szSpaceChars, sBuffer[pos + 1]) )
		{
			sBuffer = sBuffer.erase(pos + 1, 1);
		}

		//DebugReporter::Instance().DisplayMessage("CTextMatcher::FindString - end converting multiple space characters", DebugReporter::ENGINE);

		sBuffer[pos] = 0x0020; // Space

		pos++;

		pos = sBuffer.find_first_of(szSpaceChars, pos);
	}

	//DebugReporter::Instance().DisplayMessage("CTextMatcher::FindString - done converting characters", DebugReporter::ENGINE);

	BufLen = sBuffer.size() - StringLen + 1;

	//DebugReporter::Instance().DisplayMessage("CTextMatcher::FindString - start comparing characters", DebugReporter::ENGINE);

	while ( i < BufLen )
	{
		if( wcsnicmp( szString, sBuffer.c_str() + i, StringLen ) == 0 )
		{
			DebugReporter::Instance().DisplayMessage("CTextMatcher::FindString - returning true", DebugReporter::ENGINE);
			return true;
		}

		i++;
	}

	//DebugReporter::Instance().DisplayMessage("CTextMatcher::FindString - end comparing characters", DebugReporter::ENGINE);

	DebugReporter::Instance().DisplayMessage("CTextMatcher::FindString - returning false", DebugReporter::ENGINE);

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// TestRegEx tests the message part against the RegEx
// 
////////////////////////////////////////////////////////////////////////////////
bool CTextMatcher::TestRegEx(  const wchar_t* szString, wchar_t* szBuffer )
{
	// szString is the RegEx - szBuffer is the message part

	DebugReporter::Instance().DisplayMessage("CTextMatcher::TestRegex - entered.", DebugReporter::ENGINE);

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
	f.Format("CTextMatcher::TestRegEx - message part to test: %s", m.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);

	dca::String s(szString);	
	f.Format("CTextMatcher::TestRegEx - RegEx: %s", s.c_str());
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ENGINE);
	
	try
	{
		boost::regex pattern (s,boost::regex_constants::perl);
		
		if (boost::regex_search (m, pattern, boost::regex_constants::format_perl))
		{
			DebugReporter::Instance().DisplayMessage("CTextMatcher::TestRegEx - found match", DebugReporter::ENGINE);
			return true;
		}
		else
		{
			DebugReporter::Instance().DisplayMessage("CTextMatcher::TestRegEx - match not found", DebugReporter::ENGINE);
			return false;
		}
	}
	catch(...)
	{
		DebugReporter::Instance().DisplayMessage("CTextMatcher::TestRegEx - invalid RegEx", DebugReporter::ENGINE);
		return false;
	}
}