// Message.cpp: implementation of the CMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Message.h"
#include "MailComponents.h"
#include <mimepp/binhex.h>
#include <mimepp/applfile.h>
#include "TempFile.h"
#include "FileEncoder.h"
#include "MemMappedFile.h"
#include "Registration.h"
#include "ServerParameters.h"
#include "InboundMessage.h"
#include "stringfns.h"
#include "decode2047.c"
#include "RegistryFns.h"
#include <.\boost\regex.hpp>
#include "DateFns.h"
#include <sstream>

//#define _IN_HOUSE

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define strcasecmp stricmp
#define strncasecmp strnicmp

static void		_ParseMemoText_Text(const DwEntity& aEntity, CEmailText& aText, CEmailText m_Mailer);
static void		ParseMemoText_MessagePartial(const DwEntity& aEntity, CEmailText& aText);
static void		ParseMemoText_MessageExternalBody(const DwEntity& aEntity, CEmailText& aText);
static void		ParseMemoText_Multipart_Recursive(const DwEntity& aEntity, CEmailText& aText, CEmailText m_Mailer);
static int		_GetAttachment(const DwEntity& aEntity, CAttachment& aAttachment);
static DwBool	_PartContainsText(const DwEntity& aEntity);
static void		_SetSubject(DwMessage& aMessage, const char* aText, const char* aCharset, int aEncoding);
static int		_RecommendEncoding(const CEmailText& aText);
static void		UnDotStuff(DwString& sBuff);
static void		DoDotStuffing(DwString& sBuf);
static void		FixBareNoBreak(DwString& sBuf);
static void		DecodeField(DwText& afield, string& sDecoded);
static void		DecodeField(DwString& afield, DwString& aField);
static void		_WrapOneLongLine(int aMaxLineLen, string& aLine);

CMessage::CMessage()
{
	m_nMemoVirusScanState = EMS_VIRUS_SCAN_STATE_NOT_SCANNED;
	m_lPriorityID = 3; // medium priority is default
	m_nTicketID = 0;
	m_lMessageID = 0;
	m_nReplyToMsgID = 0;
	m_nUuencodeInText = 0;
	m_nUuencodeStartPos = DwString::npos;
}

CMessage::~CMessage()
{

}

int CMessage::ParseFrom(const DwString& aMessageString, const bool bParseAttachments, const bool bDateFilters, vector<DateFilters_t> m_df)
{
	// build it
    DwMessage message(aMessageString);
    message.Parse();
    DwHeaders& headers = message.Headers();	
	
	if (bDateFilters)
	{
		try
		{
			if(m_df.size() > 0)
			{
				USES_CONVERSION;
				CEmailText tmpSub;
				CEmailText tmpBody;
				bool bHeaderAdded = false;
				
				ParseSubject(message, tmpSub);
				ParseMemoText(message, tmpBody);
				bool bMatch;
				
				//Get the filters
				vector<DateFilters_t>::iterator dfIter;
				for( dfIter = m_df.begin(); dfIter != m_df.end(); dfIter++ )
				{
					bMatch = false;
					int nWhere = dfIter->m_WhereToCheck; //subject=0 body=1 both=2
					tstring sRegEx(dfIter->m_RegEx);
					string sExpression(T2A(sRegEx.c_str()));
					//boost::smatch results;
					boost::match_results<std::string::const_iterator> what;
					boost::regex pattern (sExpression,boost::regex_constants::perl);
					string sTest;
						
					if(nWhere == 0 || nWhere == 2)
					{
						//subject
						sTest.assign(tmpSub.GetText());						
						bMatch = boost::regex_search (sTest, what, pattern);
					}
					
					if((nWhere == 2 && !bMatch) || nWhere == 1)
					{
						//if both and not in subject or if body test body
						sTest.assign(tmpBody.GetText());
						bMatch = boost::regex_search (sTest, what, pattern);
					}			
					if(bMatch)
					{
						dca::String sResults(what[0]);
						bHeaderAdded = true;
						bool bAuto = false;
						tstring sTemp;
						
						sTemp.assign(dfIter->m_HeaderValue);
						string sValue(T2A(sTemp.c_str()));
						if (sValue.find ("$$") != string::npos)
						{
							//Attempt to calculate the number of days from now										
							int nRet = GetNumberDaysFromNow(sResults,sValue);
							dca::String sVal;
							sVal.FromInt(nRet);
							sValue.assign(sVal.c_str());
						}

						sTemp.assign(dfIter->m_Header);
						string sHeader = "X-MailFlow-";
						sHeader.append(T2A(sTemp.c_str()));				
						DwFieldBody& fbody = headers.FieldBody(sHeader.c_str());												
						fbody.FromString(sValue.c_str());
					}
				}
				if(bHeaderAdded)
				{
					message.Assemble();
				}
			}
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("CMessage::ParseFrom - error occured when parsing message for date filters.", DebugReporter::MAIL);
		}
	}

	// store the headers untouched
	m_PopHeaders.SetText(headers.AsString().c_str(), "ISO-8859-1");

	// priority
	ParsePriority(message, m_lPriorityID);
    
	// originator
	ParseOriginator(message, m_Originator);
	
	// readreceiptto
	ParseReadReceiptTo(message, m_ReadReceiptTo);

	// recipients
    ParseRecipients(message, "to", m_ToRecipients);
    ParseRecipients(message, "cc", m_CcRecipients);
	ParseRecipients(message, "reply-to", m_ReplyTo);

	// who's it for?
	ParseReceivedFor(message, m_ReceivedFor);

	// parse the date sent (and convert from arbitrary UTC to local time)
	ParseDate(message, m_Date);

	// subject and message body
    ParseSubject(message, m_Subject);
	ParseMailer(message, m_Mailer);
	ParseInReplyTo(message, m_InReplyTo);
	ParseReferences(message, m_References);
	ParseMsgID(message, m_MsgID);
    ParseMemoText(message, m_MemoText);

	// attachments
	if (bParseAttachments)
	{
		int nRet = ParseAttachments(message, m_Attachments);
		if (nRet != 0)
			return nRet;
	}

	return 0;
}

int CMessage::GetNumberDaysFromNow( dca::String sResults, string sValue )
{
	int nNumberDays = 0;

	try
	{
		SYSTEMTIME st;
		GetLocalTime( &st );
		int nDayOfWeek = st.wDayOfWeek;		
			
		if (sValue.find ("Next") != string::npos)
		{
			if (sValue.find ("Sunday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 14;
					break;
				case 1:
					nNumberDays = 13;
					break;
				case 2:
					nNumberDays = 12;
					break;			
				case 3:
					nNumberDays = 11;
					break;			
				case 4:
					nNumberDays = 10;
					break;			
				case 5:
					nNumberDays = 9;
					break;			
				case 6:
					nNumberDays = 8;
					break;			
				}
			}
			else if (sValue.find ("Monday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 8;
					break;
				case 1:
					nNumberDays = 14;
					break;
				case 2:
					nNumberDays = 13;
					break;			
				case 3:
					nNumberDays = 12;
					break;			
				case 4:
					nNumberDays = 11;
					break;			
				case 5:
					nNumberDays = 10;
					break;			
				case 6:
					nNumberDays = 9;
					break;			
				}
			}
			else if (sValue.find ("Tuesday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 9;
					break;
				case 1:
					nNumberDays = 8;
					break;
				case 2:
					nNumberDays = 14;
					break;			
				case 3:
					nNumberDays = 13;
					break;			
				case 4:
					nNumberDays = 12;
					break;			
				case 5:
					nNumberDays = 11;
					break;			
				case 6:
					nNumberDays = 10;
					break;			
				}
			}
			else if (sValue.find ("Wednesday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 10;
					break;
				case 1:
					nNumberDays = 9;
					break;
				case 2:
					nNumberDays = 8;
					break;			
				case 3:
					nNumberDays = 14;
					break;			
				case 4:
					nNumberDays = 13;
					break;			
				case 5:
					nNumberDays = 12;
					break;			
				case 6:
					nNumberDays = 11;
					break;			
				}
			}
			else if (sValue.find ("Thursday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 11;
					break;
				case 1:
					nNumberDays = 10;
					break;
				case 2:
					nNumberDays = 9;
					break;			
				case 3:
					nNumberDays = 8;
					break;			
				case 4:
					nNumberDays = 14;
					break;			
				case 5:
					nNumberDays = 13;
					break;			
				case 6:
					nNumberDays = 12;
					break;			
				}
			}
			else if (sValue.find ("Friday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 12;
					break;
				case 1:
					nNumberDays = 11;
					break;
				case 2:
					nNumberDays = 10;
					break;			
				case 3:
					nNumberDays = 9;
					break;			
				case 4:
					nNumberDays = 8;
					break;			
				case 5:
					nNumberDays = 14;
					break;			
				case 6:
					nNumberDays = 13;
					break;			
				}
			}
			else if (sValue.find ("Saturday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 13;
					break;
				case 1:
					nNumberDays = 12;
					break;
				case 2:
					nNumberDays = 11;
					break;			
				case 3:
					nNumberDays = 10;
					break;			
				case 4:
					nNumberDays = 9;
					break;			
				case 5:
					nNumberDays = 8;
					break;			
				case 6:
					nNumberDays = 14;
					break;			
				}
			}
		}
		else if (sValue.find ("This") != string::npos)
		{
			if (sValue.find ("Sunday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 7;
					break;
				case 1:
					nNumberDays = 6;
					break;
				case 2:
					nNumberDays = 5;
					break;			
				case 3:
					nNumberDays = 4;
					break;			
				case 4:
					nNumberDays = 3;
					break;			
				case 5:
					nNumberDays = 2;
					break;			
				case 6:
					nNumberDays = 1;
					break;			
				}
			}
			else if (sValue.find ("Monday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 1;
					break;
				case 1:
					nNumberDays = 7;
					break;
				case 2:
					nNumberDays = 6;
					break;			
				case 3:
					nNumberDays = 5;
					break;			
				case 4:
					nNumberDays = 4;
					break;			
				case 5:
					nNumberDays = 3;
					break;			
				case 6:
					nNumberDays = 2;
					break;			
				}
			}
			else if (sValue.find ("Tuesday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 2;
					break;
				case 1:
					nNumberDays = 1;
					break;
				case 2:
					nNumberDays = 7;
					break;			
				case 3:
					nNumberDays = 6;
					break;			
				case 4:
					nNumberDays = 5;
					break;			
				case 5:
					nNumberDays = 4;
					break;			
				case 6:
					nNumberDays = 3;
					break;			
				}
			}
			else if (sValue.find ("Wednesday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 3;
					break;
				case 1:
					nNumberDays = 2;
					break;
				case 2:
					nNumberDays = 1;
					break;			
				case 3:
					nNumberDays = 7;
					break;			
				case 4:
					nNumberDays = 6;
					break;			
				case 5:
					nNumberDays = 5;
					break;			
				case 6:
					nNumberDays = 4;
					break;			
				}
			}
			else if (sValue.find ("Thursday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 4;
					break;
				case 1:
					nNumberDays = 3;
					break;
				case 2:
					nNumberDays = 2;
					break;			
				case 3:
					nNumberDays = 1;
					break;			
				case 4:
					nNumberDays = 7;
					break;			
				case 5:
					nNumberDays = 6;
					break;			
				case 6:
					nNumberDays = 5;
					break;			
				}
			}
			else if (sValue.find ("Friday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 5;
					break;
				case 1:
					nNumberDays = 4;
					break;
				case 2:
					nNumberDays = 3;
					break;			
				case 3:
					nNumberDays = 2;
					break;			
				case 4:
					nNumberDays = 1;
					break;			
				case 5:
					nNumberDays = 7;
					break;			
				case 6:
					nNumberDays = 6;
					break;			
				}
			}
			else if (sValue.find ("Saturday") != string::npos)
			{
				switch (nDayOfWeek) 
				{
				case 0:
					nNumberDays = 6;
					break;
				case 1:
					nNumberDays = 5;
					break;
				case 2:
					nNumberDays = 4;
					break;			
				case 3:
					nNumberDays = 3;
					break;			
				case 4:
					nNumberDays = 2;
					break;			
				case 5:
					nNumberDays = 1;
					break;			
				case 6:
					nNumberDays = 7;
					break;			
				}
			}
		}
		else if (sValue.find ("$$Date") != string::npos)
		{
			dca::String mo;
			dca::String da;
			dca::String yr;
			string delim;
			string sRes(sResults.c_str());
			if(sRes.find("-") != string::npos)
			{
				delim = "-";
			}
			else if(sRes.find(".") != string::npos)
			{
				delim = ".";
			}
			else
			{
				delim = "/";
			}
			int nDelim = sRes.find(delim);                
			if (sValue.find ("MDY") != string::npos)
			{
				//Found date is in mm/dd/yyyy or mm-dd-yyyy format				
				mo.assign(sRes.substr(0, nDelim));
				dca::String sTemp = sRes.substr(nDelim+1,sRes.length());
				da.assign(sTemp.substr(0,sTemp.find(delim)));
				yr.assign(sRes.substr(sRes.find_last_of(delim)+1,sRes.length()));
			}
			else if (sValue.find ("DMY")  != string::npos)
			{
				//Found date is in dd/mm/yyyy or dd-mm-yyyy format				
				da.assign(sRes.substr(0, nDelim));
				dca::String sTemp = sRes.substr(nDelim+1,sRes.length());
				mo.assign(sTemp.substr(0,sTemp.find(delim)));
				yr.assign(sRes.substr(sRes.find_last_of(delim)+1,sRes.length()));
			}
			else if (sValue.find ("YMD")  != string::npos)
			{
				//Found date is in yyyy/mm/dd or yyyy-mm-dd format		
				yr.assign(sRes.substr(0, nDelim));
				dca::String sTemp = sRes.substr(nDelim+1,sRes.length());
				mo.assign(sTemp.substr(0,sTemp.find(delim)));
				da.assign(sRes.substr(sRes.find_last_of(delim)+1,sRes.length()));
			}
			else
			{
				DebugReporter::Instance().DisplayMessage("CMessage::GetNumberDaysFromNow - invalid date result format.", DebugReporter::MAIL);
				return 0;
			}

			//convert the found date then compare to today
			TIMESTAMP_STRUCT tsRes;
			tsRes.month = mo.ToInt();
			tsRes.day = da.ToInt();
			if(yr.length() == 2)
			{
				dca::String sNum = "20";
				sNum.append(yr);
				yr.assign(sNum);
			}
			tsRes.year = yr.ToInt();
			tsRes.hour = st.wHour;
			tsRes.minute = st.wMinute;
			tsRes.second = st.wSecond;
			tsRes.fraction = 0;

			TIMESTAMP_STRUCT Now;
			SystemTimeToTimeStamp(st,Now);

			if(tsRes.year < Now.year)
			{
				nNumberDays = -1;
			}
			else
			{
				if(tsRes.month < Now.month)
				{
					nNumberDays = -1;
				}
				else
				{
					if(tsRes.day < Now.day)
					{
						nNumberDays = -1;
					}
				}
			}

			if(nNumberDays == 0)
			{
				int nSeconds = CompareTimeStamps(Now,tsRes);
				
				if(nSeconds == 0)
				{
					nNumberDays = 0;
				}
				else
				{
					nNumberDays = nSeconds / 86400;
				}
			}
		}
	}
	catch(...)
	{
		DebugReporter::Instance().DisplayMessage("CMessage::GetNumberDaysFromNow - error occured when getting number of days.", DebugReporter::MAIL);
		return 0;
	}
	
	return nNumberDays;
}

int CMessage::ParsePriority(DwMessage& message, LONG& lPriorityID)
{
	DwHeaders& headers = message.Headers();

	lPriorityID = EMS_PRIORITY_NORMAL;

	int numFields = headers.NumFields();
    for (int i=0; i < numFields; ++i) 
	{
        const DwField& field = headers.FieldAt(i);

		if (field.FieldBody()->ClassId() == DwMessageComponent::kCidText)
		{
			DwString sf = field.FieldNameStr();
			sf.ConvertToLowerCase();
		
			// check to see if this a known priority field
			if (sf == "x-priority" ||
				sf == "importance" ||
				sf == "x-msmail-priority")
			{
				DwString sfb = field.FieldBody()->AsString();
				sfb.ConvertToLowerCase();

				// x-priority field has the priority level in numerical form
				if (sf == "x-priority")
				{
					lPriorityID = atoi(sfb.c_str());
					assert(lPriorityID != 0);
					if (lPriorityID == 0)
					{
						// numerical form not present for some reason,
						// so reset and look for an alternate priority field
						lPriorityID = EMS_PRIORITY_NORMAL;
						continue;
					}

					break;	// if we find x-priority, we're done!
				}

				// otherwise we have an importance or x-msmail-priority field,
				// so we'll parse by string
				if (sfb == "highest")
				{
					lPriorityID = EMS_PRIORITY_HIGHEST;
					break;
				}
				else if (sfb == "high")
				{
					lPriorityID = min(lPriorityID, EMS_PRIORITY_HIGHEST);
					// looping continues (value can be over-ridden)
				}
				else if (sfb == "normal")
				{
					lPriorityID = EMS_PRIORITY_NORMAL;
					break;
				}
				else if (sfb == "low")
				{
					lPriorityID = max(lPriorityID, EMS_PRIORITY_LOWEST);
					// looping continues (value can be over-ridden)
				}
				else if (sfb == "lowest")
				{
					lPriorityID = EMS_PRIORITY_LOWEST;
					break;
				}				
			}
		}
    }

	if(lPriorityID < 1 || lPriorityID > 5)
	{
		lPriorityID = EMS_PRIORITY_NORMAL;
	}
	return 0;
}


void CMessage::ParseReceivedFor(DwMessage& message, CEmailAddress& address)
{
	DwHeaders& headers = message.Headers();
	
	address.Set("", "", "ISO-8859-1");

	// iterate through headers
	int numFields = headers.NumFields();
	for (int i=0; i < numFields; ++i) 
	{
		const DwField& field = headers.FieldAt(i);

		// check text headers
		if (field.FieldBody()->ClassId() == DwMessageComponent::kCidText)
		{
			// if any of the following X-headers exist, they will take precedence
			DwString sf = field.FieldNameStr();
			sf.ConvertToLowerCase();
			DwString sfb = field.FieldBodyStr();

			// stop at the first special "x-for" header we find
			if (sf == "x-mdrcpt-to" ||
				sf == "x-envelope-to" ||
				sf == "x-orig-to")
			{
				// isolate the internet name
				DwMailbox addr;
				addr.FromString(sfb.c_str());
				addr.Parse();
				DwString inetName = addr.LocalPart();
				inetName += "@";
				inetName += addr.Domain();

				address.Set(inetName.c_str(), "", "ISO-8859-1");
				break;
			}
		}

		// check stamp headers
		if (field.FieldBody()->ClassId() == DwMessageComponent::kCidStamp) 
		{
			DwStamp& stamp = (DwStamp&)*field.FieldBody();

			// we want the oldest "received for" stamp that we can find, so we'll
			// iterate and replace until we run out of received headers
			if (stamp.For().length() != 0)
			{
				// to do: could this address consist of encoded words?
				DwString sFor = stamp.For();

				// isolate the internet name
				DwMailbox addr;
				addr.FromString(sFor.c_str());
				addr.Parse();
				DwString inetName = addr.LocalPart();
				inetName += "@";
				inetName += addr.Domain();

				// store address
				address.Set(inetName.c_str(), "", "ISO-8859-1");

				// break;
			}
		}
	}

	// the routing engine really needs a valid email address in the PrimaryEmailTo
	// field in the database; so if we can't extract a "received for" from the received
	// headers, then we'll populate this field with the first address in the to recipient
	// list
	/*
	if (address.GetInetName().size() == 0)
	{
		address = m_ToRecipients.Get(0);
		assert(address.GetInetName().size() != 0);
	}
	*/
}

void CMessage::SerializeToString(DwString& aMessageString, BOOL b8bitmime)
{
	USES_CONVERSION;
	
    DwMessage message;
    DwHeaders& headers = message.Headers();
    DwBody& body = message.Body();

	// add an "X-Mailer" field
	DwFieldBody& fbody = headers.FieldBody("X-Mailer");
	bool bIsEval, bIsExpired;
	int nRet = CRegistration::GetRegIsEval(CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_LICENSE_KEY, _T("")), bIsEval, bIsExpired);
	if(!nRet)
	{
		if(bIsEval)
			fbody.FromString(X_MAILER_HEADER_VALUE_EVAL);
		else
			fbody.FromString(X_MAILER_HEADER_VALUE);
	}
	else
		fbody.FromString(X_MAILER_HEADER_VALUE);

	if (m_ReadReceipt == 1)
	{
		// add an "Disposition-Notification-To" field
		DwFieldBody& fbody = headers.FieldBody("Disposition-Notification-To");
		fbody.FromString(m_Originator.GetInetName().c_str());
	}

	if (m_DeliveryConfirmation == 1)
	{
		// add an "Return-Receipt-To" field
		DwFieldBody& fbody = headers.FieldBody("Return-Receipt-To");
		fbody.FromString(m_Originator.GetInetName().c_str());
	}	
	
	tstring sCustomHeader;
	if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("CustomHeader")).c_str(), sCustomHeader))
	{
		tstring sCustomHeaderValue;
		if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("CustomHeaderValue")).c_str(), sCustomHeaderValue))
		{
			DwFieldBody& fbody = headers.FieldBody(T2A(sCustomHeader.c_str()));
			CEMSString sTicketID;
			sTicketID.Format( _T("%d"), m_nTicketID );
			tstring::size_type pos = sCustomHeaderValue.find( _T("$$TicketID$$") );
			if( pos != tstring::npos )
			{
				sCustomHeaderValue.replace( pos, 12, sTicketID.c_str() );
			}
			CEMSString sOutboundMessageID;
			sOutboundMessageID.Format( _T("%d"), m_lMessageID );
			pos = sCustomHeaderValue.find( _T("$$OutboundMessageID$$") );
			if( pos != tstring::npos )
			{
				sCustomHeaderValue.replace( pos, 21, sOutboundMessageID.c_str() );
			}
			fbody.FromString(T2A(sCustomHeaderValue.c_str()));				
		}
	}

	// from
    DwMailbox* mailbox = new DwMailbox;
    mailbox->FromString(m_Originator.GetInetName().c_str());
    mailbox->Parse();
    if (m_Originator.GetFullname().length() > 0) 
	{
		DwString from(m_Originator.GetFullname().c_str());
		// remove all double quotes
		std::string::size_type pos = 0;
		while((pos = from.find('"', 0)) != DwString::npos) 
		{
			from.erase(pos, 1);			
		}		
		
		// add double quotes to beginning and end
		from.insert(0, 1, '"');
		from.append(1, '"');		
		mailbox->SetFullName(from);
    }
    headers.From().AddMailbox(mailbox);	

    // to
    int i;
    for (i=0; i < m_ToRecipients.GetCount(); ++i) 
	{
        const CEmailAddress& addr = m_ToRecipients.Get(i);
        DwMailbox* mailbox = new DwMailbox;
        mailbox->FromString(addr.GetInetName().c_str());
        mailbox->Parse();
        if (addr.GetFullname().length() > 0) 
		{
			DwString to(addr.GetFullname().c_str());
			// remove all double quotes
			std::string::size_type pos = 0;
			while((pos = to.find('"', 0)) != DwString::npos) 
			{
				to.erase(pos, 1);				
			}		
			
			// add double quotes to beginning and end
			to.insert(0, 1, '"');
			to.append(1, '"');			
			mailbox->SetFullName(to);
        }
        headers.To().AddAddress(mailbox);
    }

    // cc
    for (i=0; i < m_CcRecipients.GetCount(); ++i) 
	{
        const CEmailAddress& addr = m_CcRecipients.Get(i);
        DwMailbox* mailbox = new DwMailbox;
        mailbox->FromString(addr.GetInetName().c_str());
        mailbox->Parse();
        if (addr.GetFullname().length() > 0) 
		{
			DwString cc(addr.GetFullname().c_str());
			// remove all double quotes
			std::string::size_type pos = 0;
			while((pos = cc.find('"', 0)) != DwString::npos) 
			{
				cc.erase(pos, 1);				
			}		
			
			// add double quotes to beginning and end
			cc.insert(0, 1, '"');
			cc.append(1, '"');
			mailbox->SetFullName(cc);
        }
        headers.Cc().AddAddress(mailbox);
    }

	// if no To: and no Cc: then add a To: Undisclosed Recipients header
	if(m_ToRecipients.GetCount() == 0 && m_CcRecipients.GetCount() == 0)
	{
		DwFieldBody& fbody = headers.FieldBody("To");
		fbody.FromString("Undisclosed Recipients");
	}
	
	// reply-to
    for (i=0; i < m_ReplyTo.GetCount(); ++i) 
	{
        const CEmailAddress& addr = m_ReplyTo.Get(i);
        DwMailbox* mailbox = new DwMailbox;
        mailbox->FromString(addr.GetInetName().c_str());
        mailbox->Parse();
        if (addr.GetFullname().length() > 0) 
		{
			DwString replyto(addr.GetFullname().c_str());
			if (addr.GetFullname()[0] != '"')
			{
				if (addr.GetFullname()[0] != '=' && addr.GetFullname()[1] != '?')
				{
					replyto.insert(0, 1, '"');
					replyto.append(1, '"');
				}
			}
			mailbox->SetFullName(replyto);
        }
        headers.ReplyTo().AddAddress(mailbox);
    }

	if(m_ReturnPath.GetCount() == 1)
	{
		// add an "Return-Path" field
		const CEmailAddress& addr = m_ReturnPath.Get(0);        
		DwFieldBody& fbody = headers.FieldBody("Return-Path");
		fbody.FromString(addr.GetInetName().c_str());
	}
	
	// inreplyto
	if(m_InReplyTo.GetText().length() > 0)
	{
		message.Headers().InReplyTo().FromString(m_InReplyTo.GetText().c_str());
	}
	
	// references
	if(m_References.GetText().length() > 0)
	{
		message.Headers().References().FromString(m_References.GetText().c_str());
	}

    // subject
	message.Headers().Subject().FromString(m_Subject.GetText().c_str());		

    // date (uses current date)
    headers.Date().SetValuesLiteral(m_Date.GetYear(), m_Date.GetMonth(), m_Date.GetDay(),
									m_Date.GetHour(), m_Date.GetMinute(), m_Date.GetSecond(), 
									m_Date.GetZone());

    // message-id
	tstring sDomain;
	tstring sHeloHostName = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_VERIFY_HELO_HOST_NAME, _T(""));
	headers.MessageId().CreateDefault();
    if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("MessageIdDomain")).c_str(), sDomain))
	{
		if(sDomain.length() > 5)
		{
			size_t origsize = wcslen(sDomain.c_str()) + 1;
			const size_t newsize = 100;
			size_t convertedChars = 0;
			char nstring[newsize];
			wcstombs(nstring, sDomain.c_str(), origsize);			
			headers.MessageId().SetDomain(nstring);
			
		}
	}
	else if(sHeloHostName.length() > 5)
	{
		size_t origsize = wcslen(sHeloHostName.c_str()) + 1;
		const size_t newsize = 100;
		size_t convertedChars = 0;
		char nstring[newsize];
		wcstombs(nstring, sHeloHostName.c_str(), origsize);			
		headers.MessageId().SetDomain(nstring);
	}
	
	DwString sVMF("VisNetic.MailFlow.");	
	dca::String sIDs;
	sIDs.Format("%d.%d",m_nTicketID,m_lMessageID);
	sVMF.append(sIDs.c_str());
	headers.MessageId().SetLocalPart(sVMF.c_str());
	
	// message priority
	DwString sXPriority("3 (Normal)");
	DwString sImportance("Normal");
	DwString sMSMailPriority("Normal");
	switch (m_lPriorityID)
	{
		case EMS_PRIORITY_NORMAL:
			break;
		case EMS_PRIORITY_HIGHEST:
		case EMS_PRIORITY_HIGH:
			sXPriority = "2 (High)";
			sImportance = "High";
			sMSMailPriority = "High";
			break;
		case EMS_PRIORITY_LOWEST:
		case EMS_PRIORITY_LOW:
			sXPriority = "4 (Low)";
			sImportance = "Low";
			sMSMailPriority = "Low";
			break;
	}
	// if the priority is "normal", then we won't add a priority
	// header to the message. this is done specifically to avoid
	// adding the "X-MSMail-Priority" header to messages that don't 
	// specifically require it, as it's suspicious to some spam catchers
	if (m_lPriorityID != EMS_PRIORITY_NORMAL)
	{
		DwFieldBody& fxp = headers.FieldBody("X-Priority");
		DwFieldBody& fmsmp = headers.FieldBody("X-MSMail-Priority");
		DwFieldBody& fimp = headers.FieldBody("Importance");
		fxp.FromString(sXPriority.c_str());
		fmsmp.FromString(sMSMailPriority.c_str());
		fimp.FromString(sImportance.c_str());
	}

    // MIME-version
    headers.MimeVersion().FromString("1.0");

    // content-type
    if (m_Attachments.GetCount() == 0) 
	{
        DwString text = m_MemoText.GetText().c_str();
		headers.ContentType().SetTypeStr(m_MemoText.GetMediaType().c_str());
		headers.ContentType().SetSubtypeStr(m_MemoText.GetMediaSubType().c_str());
		DwParameter* parameter = new DwParameter;
        parameter->SetAttribute("charset");
        parameter->SetValue("UTF-8");
        headers.ContentType().AddParameter(parameter);

		// MARKR - the best, and easiest, thing to do here is to just QP encode the
		// message body, which will result in a natural shortening of the line lengths
		// for SMTP transport. This makes everybody happy, and is universally supported.

		// HOWEVER, Deerfield.com's product support forums don't support QP encoding
		// for inbound forum thread messages, so it's necessary to maintain a private
		// internal build with a different strategy. In this case, we forcibly wrap very
		// long lines to make them compliant with SMTP line lengths, and then do NOT
		// QP encode the body (as long as we have 8bitmime support)

#ifdef _IN_HOUSE
#pragma message("In House build for Deerfield.com")
		if (!b8bitmime)
		{
			headers.ContentTransferEncoding().FromEnum(DwMime::kCteQuotedPrintable);
			DwEncodeQuotedPrintable(text, text);
		}
		else
		{
			// VMF will do line wraps only if it's forced to (e.g. it looks like
			// we'll violate RFC 821 if we don't
			WrapLongLines(900, text);
			DoDotStuffing(text);		// if no encoding, then dot-stuff
		}
#else
		headers.ContentTransferEncoding().FromEnum(DwMime::kCteBase64);
		DwEncodeBase64(text, text);
#endif

        body.FromString(text);
    }
    else /* if (mAttachments.size() > 0) */ 
	{
        headers.ContentType().SetType(DwMime::kTypeMultipart);
        headers.ContentType().SetSubtype(DwMime::kSubtypeMixed);
        headers.ContentType().CreateBoundary();

        // memo text part
        {
            DwBodyPart* part = new DwBodyPart;
            DwString text = m_MemoText.GetText().c_str();
			DwHeaders& partHeaders = part->Headers();

			partHeaders.ContentTransferEncoding().FromEnum(DwMime::kCteBase64);
			DwEncodeBase64(text, text);
			/*
			if (!b8bitmime)
			{
				partHeaders.ContentTransferEncoding().FromEnum(DwMime::kCteQuotedPrintable);
				DwEncodeQuotedPrintable(text, text);
			}
			else
			{
				// VMF will do line wraps only if it's forced to (e.g. it looks like
				// we'll violate RFC 821 if we don't
				WrapLongLines(900, text);		
				DoDotStuffing(text);		// if no encoding, then dot-stuff
			}
			*/

			partHeaders.ContentType().SetTypeStr(m_MemoText.GetMediaType().c_str());
			partHeaders.ContentType().SetSubtypeStr(m_MemoText.GetMediaSubType().c_str());
            DwParameter* parameter = new DwParameter;
            parameter->SetAttribute("charset");
            parameter->SetValue("UTF-8");
            partHeaders.ContentType().AddParameter(parameter);
            part->Body().FromString(text);
            body.AddBodyPart(part);
        }

        // attachments
        for (int i=0; i < m_Attachments.GetCount(); ++i) 
		{
            const CAttachment& attach = *m_Attachments.GetAttachment(i);
            DwBodyPart* part = new DwBodyPart;
            DwHeaders& partHeaders = part->Headers();
            partHeaders.ContentTransferEncoding().FromEnum(DwMime::kCteBase64);
            partHeaders.ContentType().SetTypeStr(attach.GetType().c_str());
            partHeaders.ContentType().SetSubtypeStr(attach.GetSubType().c_str());
            partHeaders.ContentType().SetName(attach.GetFileName().c_str());
			partHeaders.ContentDisposition().SetDispositionTypeStr(T2A(attach.GetContentDisposition().c_str()));
            partHeaders.ContentDisposition().SetFilename(attach.GetFileName().c_str());
			if( attach.GetContentID().size() > 0 )
			{
				partHeaders.ContentId() = DwMsgId( attach.GetContentID().c_str() );
			}
			// note: encoding is no longer done here (in-line), but rather as the spool
			// file is written out
            // DwString content;
            // DwEncodeBase64(attach.GetContent(), content);
            // part->Body().FromString(content);
			part->Body().FromString(attach.GetContent());
            body.AddBodyPart(part);
        }
    }

	// assemble the complete message text
    message.Assemble();
    aMessageString = message.AsString();
}


//
// Wraps a single long line by inserting '\n' characters
//
static void _WrapOneLongLine(int aMaxLineLen, DwString& aLine)
{
	assert(aMaxLineLen > 0);
	UINT count = 0;
	DwString wrappedLine;
	while (aLine.length() > (size_t)aMaxLineLen) {
		string::size_type pos = aMaxLineLen;
		char ch = aLine[pos];
		while (pos > 0 && ch != ' ' && ch != '\t') {
			--pos;
			ch = aLine[pos];
		}
		while (pos > 0 && (ch == ' ' || ch == '\t')) {
			--pos;
			ch = aLine[pos];
		}
		if (pos > 0) {
			++pos;
			if (count > 0) {
				wrappedLine += "\n";
			}
			++count;
			wrappedLine += aLine.substr(0, pos);
			ch = aLine[pos];
			while (ch == ' ' || ch == '\t') {
				++pos;
				if (pos < aLine.length()) {
					ch = aLine[pos];
				}
				else {
					ch = 0;
				}
			}
			aLine = aLine.substr(pos);
		}
		else /* if (pos == 0) */ {
			pos = 0;
			ch = aLine[pos];
			while (ch != ' ' && ch != '\t') {
				++pos;
				if (pos < aLine.length()) {
					ch = aLine[pos];
				}
				else {
					ch = ' ';
				}
			}
			if (count > 0) {
				wrappedLine += "\n";
			}
			++count;
			wrappedLine += aLine.substr(0, pos);
			if (pos < aLine.length()) {
				ch = aLine[pos];
			}
			else {
				ch = 0;
			}
			while (ch == ' ' || ch == '\t') {
				++pos;
				if (pos < aLine.length()) {
					ch = aLine[pos];
				}
				else {
					ch = 0;
				}
			}
			aLine = aLine.substr(pos);
		}
	}
	if (aLine.length() > 0) {
		if (count > 0) {
			wrappedLine += "\n";
		}
		wrappedLine += aLine;
	}
	aLine = wrappedLine;
}

//
// Wraps all lines in aStr that are longer than aMaxLineLen
// NOTE THIS FUNCTION IS CURRENTLY BROKEN - IT CREATES BARE LINEFEEDS!
void CMessage::WrapLongLines(int aMaxLineLen, DwString& aStr)
{
	const DwString& src = aStr;
	string::size_type srcLen = src.length();
	DwString dst;
	dst.reserve((size_t)(1.2*srcLen));

	// Get first line
	string::size_type lineStart = 0;
	string::size_type lineEnd = 0;
	lineEnd = src.find('\n', lineStart);
	if (lineEnd == string::npos) {
		lineEnd = srcLen;
	}
	string::size_type lineLen = lineEnd - lineStart;
	while (lineStart < srcLen) {
		if (lineLen <= (size_t)aMaxLineLen) {
			dst += src.substr(lineStart, lineLen);
			dst += "\n";
		}
		else /* if (lineLen > aMaxLineLen) */ {
			DwString line = src.substr(lineStart, lineLen);
			_WrapOneLongLine(aMaxLineLen, line);
			dst += line;
			dst += "\n";
		}

		// Get next line
		lineStart = lineEnd + 1;
		if (lineStart < srcLen) {
			lineEnd = src.find('\n', lineStart);
			if (lineEnd == string::npos) {
				lineEnd = srcLen;
			}
		}
		else /* if (lineStart >= srcLen) */ {
			lineEnd = lineStart;
		}
		lineLen = lineEnd - lineStart;
	}

	aStr = dst;
}

void CMessage::ParseOriginator(DwMessage& aMessage, CEmailAddress& aAddress)
{
    // Here we assume that there is only one author.  RFC 822 allows
    // more than one individual to be specified as the author, but only
    // if there is also a "Sender" header field.
    //
    // We return the originator's email address in the standard form.
	string chset;
	dca::String sTemp;

    aAddress.Set("", "", "ISO-8859-1");

    DwHeaders& headers = aMessage.Headers();

    int numFields = headers.NumFields();
    for (int i=0; i < numFields; ++i) 
	{
        DwField& field = headers.FieldAt(i);
        if (DwStrcasecmp("from", field.FieldNameStr()) == 0) 
		{
			DwString fBody = field.FieldBody()->AsString();
			fBody.Trim();
			
			chset = GetSubjectCharset(fBody.c_str());
					
			char* sDecoded = rfc2047_decode_simple(fBody.c_str());
			if (strlen(sDecoded) > 0)
				fBody.assign(sDecoded);

			dca::String sDec;
			sDec.assign(sDecoded);
			sTemp.assign(sDec.substr(0,sDec.find("<")-1));
			if (sTemp[0] == '\"' || sTemp[0] == '\'')
			{
				sTemp.erase(0, 1);
			}
			if (sTemp[sTemp.size()-1] == '\"' || sTemp[sTemp.size()-1] == '\'')
			{
				sTemp.erase(sTemp.size()-1, 1);
			}

			if( AddQuotesToRealNames( fBody ) )
			{
				// set and re-parse the field body
				field.FieldBody()->FromString(fBody);
				field.FieldBody()->Parse();
			}
		}
	}


    if (headers.HasFrom()) 
	{
        DwMailboxList& fromList = headers.From();
        if (fromList.NumMailboxes() > 0) 
		{
            // Create new DwMailbox using copy constructor
            const DwMailbox& from = fromList.MailboxAt(0);
            DwString inetName = from.LocalPart();
            inetName += "@";
            inetName += from.Domain();
            DwString personalName, charset;
			from.GetFullName(personalName, charset);

			if ( strcasecmp(chset.c_str(), "iso-8859-1") == 0)
			{
				if(personalName.find('@', 0) != DwString::npos)
				{
					personalName.clear();
				}

				// strip quotes from outside of personal name
				if (personalName[0] == '\"' || 
					personalName[0] == '\'')
					personalName.erase(0, 1);
				if (personalName[personalName.size()-1] == '\"' || 
					personalName[personalName.size()-1] == '\'')
					personalName.erase(personalName.size()-1, 1);

				wstring sData;
				int Len = personalName.length() + 1;
				UINT nCodePage = GetCodePage( chset );
				
				int nWcsSize = MultiByteToWideChar( nCodePage, 0, personalName.c_str(), Len, NULL, 0  );
				sData.resize( nWcsSize, L' ' );
				MultiByteToWideChar( nCodePage, 0, personalName.c_str(), Len, (LPWSTR)(sData.c_str()), nWcsSize  );

				int nChrSize = WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
				char* szBuff = new char[nChrSize+1];
				WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
				szBuff[nChrSize] = 0;
				
				aAddress.Set(inetName.c_str(), szBuff, "utf-8");
			}
			else if ( strcasecmp(chset.c_str(), "utf-8") == 0)
			{
				aAddress.Set(inetName.c_str(), personalName.c_str(), "utf-8");
			}
			else
			{
				wstring sData;
				int Len = sTemp.length() + 1;
				UINT nCodePage = GetCodePage( chset );
				
				int nWcsSize = MultiByteToWideChar( nCodePage, 0, sTemp.c_str(), Len, NULL, 0  );
				sData.resize( nWcsSize, L' ' );
				MultiByteToWideChar( nCodePage, 0, sTemp.c_str(), Len, (LPWSTR)(sData.c_str()), nWcsSize  );

				int nChrSize = WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
				char* szBuff = new char[nChrSize+1];
				WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
				szBuff[nChrSize] = 0;
				
				aAddress.Set(inetName.c_str(), szBuff, "utf-8");				
			}
        }
    }
}

void CMessage::ParseReadReceiptTo(DwMessage& aMessage, CEmailAddress& aAddress)
{
    aAddress.Set("", "", "ISO-8859-1");

    DwHeaders& headers = aMessage.Headers();

	int numFields = headers.NumFields();
    for (int i=0; i < numFields; ++i) 
	{
        DwField& field = headers.FieldAt(i);
        if (DwStrcasecmp("disposition-notification-to", field.FieldNameStr()) == 0) 
		{
			DwString sfb = field.FieldBodyStr();
			DwMailbox addr;
			addr.FromString(sfb.c_str());
			addr.Parse();
			DwString inetName = addr.LocalPart();
			inetName += "@";
			inetName += addr.Domain();
			aAddress.Set(inetName.c_str(), "", "ISO-8859-1");
			break;
		}
	}
}

void CMessage::ParseRecipients(DwMessage& aMessage, const string& aWhich, CEmailAddressList& aAddresses)
{
    string chset;
	const char* which = aWhich.c_str();
    DwHeaders& headers = aMessage.Headers();
	
    // iterate through all header fields, and get the addresses from each
    // "To" field.  Note: it has been recommended in a recent Internet
    // draft that multiple "To" fields not be used.  However, the official
    // standard (RFC 822) allows the use of multiple "To fields, so we will
    // accommodate it.
	
    int numFields = headers.NumFields();
    for (int i=0; i < numFields; ++i) 
	{
        DwField& field = headers.FieldAt(i);
        if (DwStrcasecmp(which, field.FieldNameStr()) == 0) 
		{
			// if there are no brackets, commas, semicolons, parens, braces, or
			// quotes (single & double), yet there are spaces, we make
			// a presumption that the spaces are intended as delimiters
			DwString fBody = field.FieldBody()->AsString();
			fBody.Trim();

			chset = GetSubjectCharset(fBody.c_str());

			char* sDecoded = rfc2047_decode_simple(fBody.c_str());
			if (strlen(sDecoded) > 0)
				fBody.assign(sDecoded);			

			// if spaces are present, but no "space containers" are found
			if ( fBody.find(' ', 0) != DwString::npos 
				&& fBody.find_first_of("<>[](),;\"'", 0) == DwString::npos )
			{
				// replace spaces with commas
				std::string::size_type pos = 0;
				while((pos = fBody.find(" ", pos)) != DwString::npos) 
				{
					fBody.replace(pos, 1, 1, ',');
					pos++;
				}

				// set and re-parse the field body
				field.FieldBody()->FromString(fBody);
				field.FieldBody()->Parse();
			}
			
			// if semis are present replace with commas
			if ( fBody.find(';', 0) != DwString::npos )
			{
				int nSemi = 0;
				int nAt = 0;

				// count semis
				std::string::size_type pos = 0;
				while((pos = fBody.find(";", pos)) != DwString::npos) 
				{
					nSemi++;
					pos++;
				}

				// count @s
				pos = 0;
				while((pos = fBody.find("@", pos)) != DwString::npos) 
				{
					nAt++;
					pos++;
				}


				// if the number of semis equals the number of @s-1 assume they are address separators and replace semis with commas
				if(nAt-1==nSemi)
				{
					pos = 0;
					while((pos = fBody.find(";", pos)) != DwString::npos) 
					{
						fBody.replace(pos, 1, 1, ',');
						pos++;
					}

					// set and re-parse the field body
					field.FieldBody()->FromString(fBody);
					field.FieldBody()->Parse();
				}
			}

			if( AddQuotesToRealNames( fBody ) )
			{
				// set and re-parse the field body
				field.FieldBody()->FromString(fBody);
				field.FieldBody()->Parse();
			}

			// parse out the recipient list
            const DwAddressList& rcptList = *(const DwAddressList*)field.FieldBody();
            int numAddresses = rcptList.NumAddresses();
            for (int j=0; j < numAddresses; ++j) 
			{
                const DwAddress& addr = rcptList.AddressAt(j);
                if (addr.IsMailbox()) 
				{
                    const DwMailbox& rcpt = (const DwMailbox&) addr;
                    DwString inetName = rcpt.LocalPart();
                    inetName += "@";
                    inetName += rcpt.Domain();
                    DwString personalName, charset;
                    rcpt.GetFullName(personalName, charset);

					if ( strcasecmp(chset.c_str(), "utf-8") == 0)
					{
						CEmailAddress eaddr(inetName.c_str(), personalName.c_str(), "utf-8");
						aAddresses.Add(eaddr);
					}
					else
					{
						dca::String sTemp(personalName.c_str());
	
						wstring sData;
						int Len = sTemp.length() + 1;
						UINT nCodePage = GetCodePage( chset );
						
						int nWcsSize = MultiByteToWideChar( nCodePage, 0, sTemp.c_str(), Len, NULL, 0  );
						sData.resize( nWcsSize, L' ' );
						MultiByteToWideChar( nCodePage, 0, sTemp.c_str(), Len, (LPWSTR)(sData.c_str()), nWcsSize  );

						int nChrSize = WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
						char* szBuff = new char[nChrSize+1];
						WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
						szBuff[nChrSize] = 0;
						
						CEmailAddress eaddr(inetName.c_str(), szBuff, "utf-8");
						aAddresses.Add(eaddr);
					}
                    
                }
                else if (addr.IsGroup()) 
				{
                    const DwGroup& rcpt = (const DwGroup&) addr;
                    CEmailAddress eaddr("", rcpt.GroupName().c_str(), "");
                    aAddresses.Add(eaddr);
                }
            }
        }
    }
}

char* CMessage::SearchQuote(const char **ptr)
{
	const char     *p = *ptr;
	char           *s;

	while (**ptr && **ptr != '?')
		++* ptr;
	if ((s = (char*)malloc(*ptr - p + 1)) == 0)
		return (0);
	memcpy(s, p, *ptr - p);
	s[*ptr - p] = 0;
	return (s);
}

char* CMessage::GetSubjectCharset(const char *text)
{
	int             had_last_word = 0;
	const char     *p;
	char           *chset;
	
	chset = "iso-8859-1";
	if (text[0] == '\"' || text[0] == '\'')
	{
		++text;
	}
	
	while (text && *text)
	{
		if (text[0] != '=' || text[1] != '?')
		{
			p = text;
			while (*text)
			{
				if (text[0] == '=' && text[1] == '?')
					break;
				if (!isspace((int)(unsigned char)*text))
				{
					had_last_word = 0;
				}
				else
				{
					had_last_word = 1;
				}
				++text;
			}
			if (text > p && !had_last_word)
			{
				return ("iso-8859-1");
			}
			continue;
		}

		text += 2;
		if ((chset = SearchQuote(&text)) == 0)
		{
			return ("iso-8859-1");	
		}
		else
		{
			return chset;
		}
	}
	return chset;
}

void CMessage::ParseSubject(DwMessage& aMessage, CEmailText& aSubject)
{
    dca::String f;
		
	aSubject.SetText("", "");
    DwHeaders& headers = aMessage.Headers();
    if (headers.HasSubject()) 
	{
        DwText& subject = headers.Subject();	
		
		f.Format("CMessage::ParseSubject - subject from header: %s", subject.AsString().c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				
		char* szSubject = rfc2047_decode_simple(subject.AsString().c_str());
		
		f.Format("CMessage::ParseSubject - subject decoded: %s", szSubject);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		
		string charset = GetSubjectCharset(subject.AsString().c_str());

		f.Format("CMessage::ParseSubject - subject charset: %s", charset.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				
		if ( strcasecmp(charset.c_str(), "utf-8") == 0)
		{
			aSubject.SetText(szSubject, "utf-8");					
		}
		else
		{
			wstring sData;
			int Len = strlen( szSubject ) + 1;
			UINT nCodePage = GetCodePage( charset );
			
			int nWcsSize = MultiByteToWideChar( nCodePage, 0, szSubject, Len, NULL, 0  );
			sData.resize( nWcsSize, L' ' );
			MultiByteToWideChar( nCodePage, 0, szSubject, Len, (LPWSTR)(sData.c_str()), nWcsSize  );

			f.Format("CMessage::ParseSubject - subject as codepage %d: %s", nCodePage, sData.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

			int nChrSize = WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
			char* szBuff = new char[nChrSize+1];
			WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
			szBuff[nChrSize] = 0;
			
			f.Format("CMessage::ParseSubject - subject as UTF-8: %s", szBuff);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

			aSubject.SetText(szBuff, "utf-8");
		}		   
    }
}

void CMessage::ParseMailer(DwMessage& aMessage, CEmailText& aMailer)
{
    dca::String f;
		
	aMailer.SetText("", "");
    DwHeaders& headers = aMessage.Headers();
    
	int numFields = headers.NumFields();
	for (int i=0; i < numFields; ++i) 
	{
		DwField& field = headers.FieldAt(i);
		if (DwStrcasecmp("x-mailer", field.FieldNameStr()) == 0) 
		{
			DwString sfb = field.FieldBodyStr();			
			char* szMailer = rfc2047_decode_simple(sfb.c_str());
			aMailer.SetText(szMailer, "ISO-8859-1");
			break;
		}
	}	
}

void CMessage::ParseInReplyTo(DwMessage& aMessage, CEmailText& aInReplyTo)
{
    aInReplyTo.SetText("", "");
    DwHeaders& headers = aMessage.Headers();
    if (headers.HasInReplyTo()) 
	{
        DwText& inreplyto = headers.InReplyTo();	
		char* szInReplyTo = rfc2047_decode_simple(inreplyto.AsString().c_str());
		aInReplyTo.SetText(szInReplyTo, "ISO-8859-1");		
    }
}

void CMessage::ParseReferences(DwMessage& aMessage, CEmailText& aReferences)
{
    aReferences.SetText("", "");
    DwHeaders& headers = aMessage.Headers();
    if (headers.HasReferences()) 
	{
        DwText& references = headers.References();	
		char* szReferences = rfc2047_decode_simple(references.AsString().c_str());
		aReferences.SetText(szReferences, "ISO-8859-1");			   
    }
}

void CMessage::ParseMsgID(DwMessage& aMessage, CEmailText& aMessageID)
{
    aMessageID.SetText("", "");
    DwHeaders& headers = aMessage.Headers();
    if (headers.HasMessageId()) 
	{
        char* szMessageID = rfc2047_decode_simple(headers.MessageId().AsString().c_str());
		aMessageID.SetText(szMessageID, "ISO-8859-1");			   
    }
}

void DecodeField(DwText& afield, string& sDecoded)
{
    int numEncodedWords = afield.NumEncodedWords();
    for (int i=0; i < numEncodedWords; ++i) 
	{
        DwEncodedWord& word = afield.EncodedWordAt(i);
        if (sDecoded.length() > 0) 
		{
            sDecoded += " ";
        }
        sDecoded += word.DecodedText().c_str();
    }
}

void DecodeField(DwString& afield, DwString& aField)
{
	DwText text(afield);
	text.Parse();
	string sRes = ""; 
	DecodeField(text, sRes);
	aField = sRes.c_str();
}

void CMessage::ParseMemoText(DwMessage& aMessage, CEmailText& aMemoText)
{
    aMemoText.SetText("", "ISO-8859-1");
	
    // Assign default values
	
    int type = DwMime::kTypeText;
    int subtype = DwMime::kSubtypePlain;
    DwHeaders& headers = aMessage.Headers();
    DwBody& body = aMessage.Body();
    if (headers.HasContentType()) 
	{
		aMemoText.SetMediaType(headers.ContentType().TypeStr().c_str());
		aMemoText.SetMediaSubType(headers.ContentType().SubtypeStr().c_str());

        type = headers.ContentType().Type();
        subtype = headers.ContentType().Subtype();
    }
    switch (type) 
	{
    case DwMime::kTypeText:
        ParseMemoText_Text(aMessage, aMemoText);
        break;
    case DwMime::kTypeMessage:
        ParseMemoText_Message(aMessage, aMemoText);
        break;
    case DwMime::kTypeMultipart: 
        ParseMemoText_Multipart(aMessage, aMemoText);
        break;
    case DwMime::kTypeAudio:
    case DwMime::kTypeImage:
    case DwMime::kTypeVideo:
    case DwMime::kTypeApplication:
    case DwMime::kTypeModel:
    default:
        // No memo text -- just an attachment
        break;
    }
}


void CMessage::ParseMemoText_Text(DwMessage& aMessage, CEmailText& aText)
{
    _ParseMemoText_Text(aMessage, aText, GetMailer());
}


void CMessage::ParseMemoText_Message(DwMessage& aMessage, CEmailText& aText)
{
    DwHeaders& headers = aMessage.Headers();
    DwBody& body = aMessage.Body();
    int subtype = headers.ContentType().Subtype();
    switch (subtype) 
	{
    case DwMime::kSubtypePartial:
        ParseMemoText_MessagePartial(aMessage, aText);
        break;
    case DwMime::kSubtypeExternalBody:
        ParseMemoText_MessageExternalBody(aMessage, aText);
        break;
    case DwMime::kSubtypeRfc822:
    default:
        aText.SetText(body.AsString().c_str(), "ISO-8859-1");
        break;
    }
}


void CMessage::ParseMemoText_Multipart(DwMessage& aMessage, CEmailText& aText)
{
    // If the message is a multipart message, then the text memo should
    // be the first text body part.  We can't guarantee that every MUA
    // follows this convention, but when smart people think about it,
    // this is the only right way.  The reason is, that it is the first
    // text that is seen by a non-MIME capable MUA.  For best compatibility
    // with non-MIME capable MUA's, the memo text MUST come first.
	
    // One common structure is that the message is a multipart/mixed,
    // and the first body part is a multipart/alternative containing
    // a text/plain and a text/html (Netscape, Microsoft Outlook, among
    // others).
	
    aText.SetText("", "ISO-8859-1");
	
    ParseMemoText_Multipart_Recursive(aMessage, aText, GetMailer());
}


int CMessage::ParseAttachments(DwMessage& aMessage, CAttachmentList& aList)
{
	int nRet;
    int type = DwMime::kTypeText;
    int subtype = DwMime::kSubtypePlain;
    DwHeaders& headers = aMessage.Headers();
    DwBody& body = aMessage.Body();
    if (headers.HasContentType()) 
	{
        type = headers.ContentType().Type();
        subtype = headers.ContentType().Subtype();
    }
	
    switch (type) 
	{
    case DwMime::kTypeText:
		{
			DwUuencode decoder;

			DwString content = body.AsString();

			decoder.SetAsciiChars(content);

			size_t start = 0;
			size_t end = 0;
			
			nRet = decoder.Decode(&start, &end);

			while(nRet == 0)
			{
				CAttachment attachment;

				DwString data = decoder.BinaryChars();

				attachment.SetFileName(decoder.FileName());
				attachment.SetContent(data);
				attachment.SetMediaType("text", "uuencode");

				// if attachment filename is empty, then ignore it
				if (!attachment.GetFileName().empty())
				{
					m_nUuencodeInText = 1;
					
					if(m_nUuencodeStartPos == DwString::npos)
						m_nUuencodeStartPos = decoder.GetEncodingStart();

					aList.AddAttachment(attachment);
				}

				content.erase(start, end - start);
				start = 0;
				end = 0;
				decoder.Initialize();
				decoder.SetAsciiChars(content);
				nRet = decoder.Decode(&start, &end);
			}
		}
        break;
    case DwMime::kTypeAudio:
    case DwMime::kTypeImage:
    case DwMime::kTypeVideo:
    case DwMime::kTypeApplication:
    case DwMime::kTypeModel:
    default:
        nRet = ParseAttachments_Simple(aMessage, aList);
		if (nRet != 0)
			return nRet;
        break;
    case DwMime::kTypeMessage:
        ParseAttachments_Message(aMessage, aList);
        break;
    case DwMime::kTypeMultipart:
        nRet = ParseAttachments_Multipart(aMessage, aList);
		if (nRet != 0)
			return nRet;
        break;
    }

	return 0;
}


int CMessage::ParseAttachments_Simple(DwMessage& aMessage, CAttachmentList& aAttachments)
{
    CAttachment attachment;
    int nRet = _GetAttachment(aMessage, attachment);

	dca::String f;
	f.Format("CMessage::ParseAttachments_Simple - found %d attachments", nRet);
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

	if (nRet != 0)
		return nRet;

	// if attachment filename is empty, then ignore it
	if (!attachment.GetFileName().empty())
	{
		aAttachments.AddAttachment(attachment);
	}

	return 0;
}


void CMessage::ParseAttachments_Message(DwMessage& aMessage, CAttachmentList& aAttachments)
{
    DwBody& body = aMessage.Body();
    DwMessage* message = body.Message();	

    if (message != 0) 
	{
		// if message is multipart, let's drill into it to extract
		// other message parts as attachments
		if (message->Headers().ContentType().Type() == DwMime::kTypeMultipart)
			ParseAttachments_Multipart(*message, aAttachments);

        CAttachment attachment;
        string type = message->Headers().ContentType().TypeStr().c_str();        
		string subtype = message->Headers().ContentType().SubtypeStr().c_str();
        attachment.SetMediaType(type, subtype);
        attachment.SetContent(message->AsString());

		// formulate a filename for this attachment based upon the
		// message id
		string sAttFileName(message->Headers().Subject().AsString().c_str());
		if (sAttFileName.size() == 0)
			sAttFileName = "message";
		
		string msgid = message->Headers().MessageId().Domain().c_str();
		if (msgid.size() > 0)
		{
			sAttFileName += "-";
			sAttFileName += msgid;
		}		
		sAttFileName += ".eml";
		attachment.SetFileName(sAttFileName);
		tstring sDisposition(_T("attachment"));
		attachment.SetContentDisposition(sDisposition);
		if (type.size() > 0 && subtype.size() > 0)
		{
			aAttachments.AddAttachment(attachment);
		}
    }
}


int CMessage::ParseAttachments_Multipart(DwMessage& aMessage, CAttachmentList& aAttachments)
{
	USES_CONVERSION;

	int nRet;
    int subtype = DwMime::kSubtypePlain;
    DwHeaders& headers = aMessage.Headers();
    DwBody& body1 = aMessage.Body();
    int numParts = body1.NumBodyParts();

	{
		dca::String f;
		f.Format("CMessage::ParseAttachments_Multipart - found %d body parts",numParts);
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	}

    if (numParts > 0) 
	{
		// if the first body part isn't text, retrieve
		// it as an attachment

        const DwBodyPart& part = body1.BodyPartAt(0);
        if (!_PartContainsText(part))
		{
            DwHeaders& headers = part.Headers();
			DwBody& body2 = part.Body();
			int type = DwMime::kTypeUnknown;
			int subtype = DwMime::kSubtypeUnknown;
			if (headers.HasContentType()) 
			{
				type = headers.ContentType().Type();
				subtype = headers.ContentType().Subtype();
			}

			switch(type)
			{
			case DwMime::kTypeMessage:
				{
					if(subtype == DwMime::kSubtypeRfc822 && numParts == 1)
					{
						DwMessageComponent* p = body2.Clone();
						p->Parse();
						DwMessage m(p->AsString(), p);
						m.Parse();
						nRet = this->ParseAttachments(m, aAttachments);
						if (nRet != 0)
							return nRet;
					}
				}
				break;
			case DwMime::kTypeAudio:
				{
					if(numParts == 1)
					{
						CAttachment attachment;
						nRet = _GetAttachment(part, attachment);
						if (nRet != 0)
							return nRet;

						// if it's an attachment with no filename,
						// don't bother keeping it (nothing we can do with it)
						if (!attachment.GetFileName().empty())
							aAttachments.AddAttachment(attachment);
					}
				}
				break;
			default:
				{
					DebugReporter::Instance().DisplayMessage("CMessage::ParseAttachments_Multipart - if the first body part isn't text, retrieve it as an attachment", DebugReporter::MAIL);
					CAttachment attachment;
					nRet = _GetAttachment(part, attachment);
					if (nRet != 0)
						return nRet;
				}
			}
        }
		//else{ // Added by Mark Mohr on 9/7/2007.  Kevin Fortune has a customer who is sending signed data that
		//	  // has an attachment.  Any attachments after text are getting stripped off.  This has always been
		//	  // the case in mailflow.  Now if we find multipart/mixed we will skip the text and get any attachments.
		//	DwHeaders& headers = part.Headers();
		//	subtype = headers.ContentType().Subtype();
		//	if(subtype == DwMime::kSubtypeMixed){
		//		DwBody& body2 = part.Body();
		//		numParts = body2.NumBodyParts();
		//		for(int i = 1; i < numParts; ++i){
		//			const DwBodyPart& part = body2.BodyPartAt(i);
		//			CAttachment attachment;
		//			nRet = _GetAttachment(part, attachment);
		//			if(nRet == 0){
		//				if (!attachment.GetFileName().empty())
		//					aAttachments.AddAttachment(attachment);
		//			}
		//		}
		//	}			
		//}
    }

    for (int i=0; i < numParts; ++i) 
	{
        try
		{
			const DwBodyPart& part = body1.BodyPartAt(i);
			CAttachment attachment;
			nRet = _GetAttachment(part, attachment);
			if (nRet != 0)
				return nRet;

			// handle specific content types
			if (part.Headers().HasContentType())
			{
				switch (part.Headers().ContentType().Type())
				{
				case DwMime::kTypeMessage:
					{
						if(part.Headers().ContentType().Subtype() == DwMime::kSubtypeRfc822)
						{
							try
							{
								dca::String l(part.Body().AsString().c_str());
								dca::String::size_type pos1 = l.find('\n');
								dca::String::size_type pos2 = l.find(':');

								if(pos1 != dca::String::npos && pos2 != dca::String::npos)
								{
									if(pos2 > pos1)
										l.erase(0, pos1 + 1);
								}

								DwMessage m(l.c_str());
								m.Parse();

								//this->ParseAttachments(m, aAttachments);
								ParseAttachments_Message((DwMessage&)part, aAttachments);

							}
							catch(...)
							{
								ParseAttachments_Message((DwMessage&)part, aAttachments);
							}
						}
						else
						{
							ParseAttachments_Message((DwMessage&)part, aAttachments);
						}
					}
					break;
				case DwMime::kTypeMultipart:
					ParseAttachments_Multipart((DwMessage&)part, aAttachments);
					break;
				}
			}

			// if we encouter an appledouble attachment, we'll drill
			// into the applefile segment
			if (attachment.GetSubType() == "appledouble")
			{			
				ParseAttachments_Multipart((DwMessage&)part, aAttachments);
				break;
			}

			// if this is a multipart/report message, the second and
			// third attachments should represent the message delivery status
			// and original message, respectively
			if (headers.ContentType().Type() == DwMime::kTypeMultipart &&
				headers.ContentType().Subtype() == DwMime::kSubtypeReport)
			{
				if (attachment.GetType() == "message" &&
					attachment.GetSubType() == "delivery-status")
				{
					// here's our delivery status; let's save this off
					string sMsgTxt(headers.MessageId().LocalPart().c_str());
					sMsgTxt += "-details.txt";
					attachment.SetFileName(sMsgTxt);
				}
				else if (attachment.GetType() == "message" &&
						(attachment.GetSubType() == "rfc822" || attachment.GetSubType() == "rfc822-headers"))
				{
					// here's our original (returned) message, which we'll
					// parse (without parsing any attachments)
					CInboundMessage inmsg;
					inmsg.ParseFrom(attachment.GetContent(), false, false);

					// if this function succeeds (if subject contains [ticketid:msgid]), it will 
					// have parsed the ticket and message IDs from the subject line of our original message,
					// storing them in member variables (otherwise members remain set to 0);
					// later, when the inbound message is saved, the TicketID and InboundMsgID
					// table fields will be set to these values
					if(!GetTicketIDFromSubject(A2T(inmsg.GetSubject().GetText().c_str()), m_nTicketID, m_nReplyToMsgID))
						GetTicketIDFromBody(A2T(inmsg.GetMemoText().GetText().c_str()), m_nTicketID, m_nReplyToMsgID);

					string sMsgTxt(headers.MessageId().LocalPart().c_str());
					sMsgTxt += "-msg.txt";
					attachment.SetFileName(sMsgTxt);
				}
			}

			// if it's an attachment with no filename,
			// don't bother keeping it (nothing we can do with it)
			if (!attachment.GetFileName().empty())
			{
				aAttachments.AddAttachment(attachment);
			}
			else if (attachment.GetType() == "image")
			{
				// if it's an image, let's give it a name and save it off
				string sMsgTxt(attachment.GetContentID().c_str());
				sMsgTxt += "-renamed." + attachment.GetSubType();
				attachment.SetFileName(sMsgTxt);
				aAttachments.AddAttachment(attachment);
			}
		}
		catch(...)
		{
			DebugReporter::Instance().DisplayMessage("CMessage::ParseAttachments_Multipart - caught exception", DebugReporter::MAIL);			
		}
    }

	return 0;
}

//--------------------------------------------------------------------------
// Static functions
//--------------------------------------------------------------------------


void UnDotStuff(DwString& sBuff)
{
	// if the first line is dot-stuffed, un-stuff it
	if ((sBuff[0] == '.') && (sBuff[1] == '.'))
		sBuff.erase(0, 1);
	
	const DwString stuff_repl("\r\n.");

	// un-dot-stuff the message
	std::string::size_type pos = 0;
	while((pos = sBuff.find("\r\n..", pos)) != std::string::npos) 
	{
		sBuff.replace(pos, 4, stuff_repl);
		pos++;
	}
}

void DoDotStuffing(DwString& sBuf)
{
	// if the first line starts with a dot, stuff it
	if (sBuf[0] == '.')
		sBuf.insert(0, 1, '.');

	// if there are any instances of CR LF '.', then stuff them
	const DwString repl_with("\r\n..");
	std::string::size_type pos = 0;
	while((pos = sBuf.find("\r\n.", pos)) != std::string::npos) 
	{
		sBuf.replace(pos, 3, repl_with);
		pos += repl_with.size();
	}
}

void FixBareNoBreak(DwString& sBuf)
{
	// if there are any instances of =C2=A0 replace them with placeholder =XX=XX
	// if there are any instances of =A0 replace them with =20
	// revert placeholder to =C2=A0
	const DwString repl_with_XX("=XX=XX");
	const DwString repl_with_20("=20");
	const DwString repl_with_C2A0("=C2=A0");
	std::string::size_type pos = 0;
	while((pos = sBuf.find("=C2=A0", pos)) != std::string::npos) 
	{
		sBuf.replace(pos, 6, repl_with_XX);
		pos += repl_with_XX.size();
	}
	pos = 0;
	while((pos = sBuf.find("=A0", pos)) != std::string::npos) 
	{
		sBuf.replace(pos, 3, repl_with_20);
		pos += repl_with_20.size();
	}
	pos = 0;
	while((pos = sBuf.find("=XX=XX", pos)) != std::string::npos) 
	{
		sBuf.replace(pos, 6, repl_with_C2A0);
		pos += repl_with_C2A0.size();
	}
}
static void _ParseMemoText_Text(const DwEntity& aEntity, CEmailText& aText, CEmailText m_Mailer)
{
    
	DwHeaders& headers = aEntity.Headers();
    DwBody& body = aEntity.Body();

    DwString text = body.AsString();
		
	// undotstuff here
	UnDotStuff(text);

   // Get the charset, if it's there

    string charset = "ISO-8859-1";

    if (headers.HasContentType()) 
	{
        int numParams = headers.ContentType().NumParameters();
        for (int i=0; i < numParams; ++i) 
		{
            const DwParameter& param = headers.ContentType().ParameterAt(i);
            const DwString& attrName = param.Attribute();
            if (0 == DwStrcasecmp(attrName, "charset")) 
			{
                charset = param.Value().c_str();
                break;
            }
        }
    }

	// Check content-transfer-encoding, and decode if necessary
	DwString cte;
    if (headers.HasContentTransferEncoding()) 
	{
        cte = headers.ContentTransferEncoding().AsString();
    }
    
	if (0 == DwStrcasecmp(cte, "quoted-printable")) 
	{
        //FixBareNoBreak(text);
		DwDecodeQuotedPrintable(text, text);
		DwToLocalEol(text, text);		
    }
    else if (0 == DwStrcasecmp(cte, "base64")) 
	{
        if ( strcasecmp(charset.c_str(), "utf-16") == 0)
		{
			CkString str;
			str.append(text.c_str());
			str.base64Decode("ISO-8859-1");
			text = str.getString();			
		}
		else
		{
			DwDecodeBase64(text, text);
			DwToLocalEol(text, text);
		}
    }
	
	if ( strcasecmp(charset.c_str(), "iso-2022-kr") == 0)
	{
		string sTemp(m_Mailer.GetText().c_str());

		if (strncasecmp(m_Mailer.GetText().c_str(), "Lotus Notes", 11)==0)
		{
			charset = "ISO-8859-1";
		}
	}

	if ( strcasecmp(charset.c_str(), "utf-8") == 0 || strcasecmp(charset.c_str(), "utf-16") == 0)
	{
		aText.SetText(text.c_str(), charset.c_str());			
	}
	else
	{
		wstring sData;
		dca::String sTemp(text.c_str());
		int Len = sTemp.length() + 1;
		UINT nCodePage = CMessage::GetCodePage( charset );
		
		int nWcsSize = MultiByteToWideChar( nCodePage, 0, sTemp.c_str(), Len, NULL, 0  );
		sData.resize( nWcsSize, L' ' );
		MultiByteToWideChar( nCodePage, 0, sTemp.c_str(), Len, (LPWSTR)(sData.c_str()), nWcsSize  );

		int nChrSize = WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
		char* szBuff = new char[nChrSize+1];
		WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
		szBuff[nChrSize] = 0;
		
		aText.SetText(szBuff, "utf-8");
	}
}


static void ParseMemoText_MessagePartial(const DwEntity& aEntity, CEmailText& aText)
{
    string number;
    string total;
    DwMediaType& contentType = aEntity.Headers().ContentType();
    int numParams = contentType.NumParameters();
    for (int i=0; i < numParams; ++i) 
	{
        const DwParameter& param = contentType.ParameterAt(i);
        const DwString& attr = param.Attribute();
        const DwString& val = param.Value();
        if (DwStrcasecmp(attr, "number") == 0) 
		{
            number = val.c_str();
        }
        if (DwStrcasecmp(attr, "total") == 0) 
		{
            total = val.c_str();
        }
    }
    if (total == "") 
	{
        total = "?";
    }
    string text = "[This is part ";
    text += number;
    text += " of a total of ";
    text += total;
    text += " parts]";
    aText.SetText(text, "ISO-8859-1");
}


static void ParseMemoText_MessageExternalBody(const DwEntity& aEntity, CEmailText& aText)
{
    string text = "[The body of this message is stored elsewhere]"
        DW_EOL DW_EOL;

    // List the attributes, which provide information about how to obtain
    // the message content

    DwMediaType& contentType = aEntity.Headers().ContentType();
    int numParams = contentType.NumParameters();
    for (int i=0; i < numParams; ++i) 
	{
        const DwParameter& param = contentType.ParameterAt(i);
        text += param.Attribute().c_str();
        text += ":";
        text += param.Value().c_str();
        text += DW_EOL;
    }
    text += DW_EOL;

    // List the contained message, which contains the header fields of the
    // external message

    DwMessage* containedMessage = aEntity.Body().Message();
    if (containedMessage != 0) 
	{
        text += containedMessage->AsString().c_str();
    }
    aText.SetText(text, "ISO-8859-1");
}

static void ParseMemoText_Multipart_Recursive(const DwEntity& aEntity, CEmailText& aText, CEmailText m_Mailer)
{
    DwHeaders& headers = aEntity.Headers();
    DwBody& body = aEntity.Body();

    // Assign default values
    int type    = DwMime::kTypeText;
    int subtype = DwMime::kSubtypePlain;
    if (headers.HasContentType()) 
	{
		aText.SetMediaType(headers.ContentType().TypeStr().c_str());
		aText.SetMediaSubType(headers.ContentType().SubtypeStr().c_str());

        type    = headers.ContentType().Type();
        subtype = headers.ContentType().Subtype();
    }

    switch (type) 
	{
    case DwMime::kTypeMultipart: 
        if (body.NumBodyParts() > 0) 
		{
			if (subtype == DwMime::kSubtypeAlternative)
			{
				// for muti-part alternative, we grab the last
				// body representation that we find in the message, as it
				// should be ordered in increasing faithfulness to the original
				for (int i=0; i<body.NumBodyParts(); i++)
				{
					const DwBodyPart& part = body.BodyPartAt(i);
					ParseMemoText_Multipart_Recursive(part, aText, m_Mailer);
				}
			}
			else
			{
				// for any other multi-part messages (mixed, for example),
				// we'll use the first body part as our memo content, and consider
				// the remaining body parts as attachments
				const DwBodyPart& part = body.BodyPartAt(0);
				ParseMemoText_Multipart_Recursive(part, aText, m_Mailer);
			}
        }
        break;
    case DwMime::kTypeText:
        _ParseMemoText_Text(aEntity, aText, m_Mailer);
        break;
    case DwMime::kTypeAudio:
    case DwMime::kTypeImage:
    case DwMime::kTypeVideo:
    case DwMime::kTypeApplication:
    case DwMime::kTypeMessage:
    case DwMime::kTypeModel:
    default:
        break;
    }
}


static DwBool _PartContainsText(const DwEntity& aEntity)
{
    DwBool containsText = DwFalse;

    int type = DwMime::kTypeText;
    int subtype = DwMime::kSubtypePlain;
    DwHeaders& headers = aEntity.Headers();
    DwBody& body = aEntity.Body();
    if (headers.HasContentType()) 
	{
        type = headers.ContentType().Type();
        subtype = headers.ContentType().Subtype();
    }

    int i, numParts;
    switch (type) 
	{
    case DwMime::kTypeText:
        containsText = DwTrue;
        break;
    case DwMime::kTypeMultipart:
        numParts = body.NumBodyParts();
        for (i=0; i < numParts; ++i) 
		{
            const DwBodyPart& part = body.BodyPartAt(i);
            if (_PartContainsText(part)) 
			{
                containsText = DwTrue;
                break;
            }
        }
        break;
    case DwMime::kTypeAudio:
    case DwMime::kTypeImage:
    case DwMime::kTypeVideo:
    case DwMime::kTypeApplication:
    case DwMime::kTypeModel:
    case DwMime::kTypeMessage:
    default:
        break;
    }
    return containsText;
}


static int _GetAttachment(const DwEntity& aEntity, CAttachment& aAttachment)
{
	USES_CONVERSION;

    DwHeaders& headers = aEntity.Headers();
    DwBody& body = aEntity.Body();
    string typeStr = headers.ContentType().TypeStr().c_str();
    string subtypeStr = headers.ContentType().SubtypeStr().c_str();
    string filenameStr;
	dca::String f;

	// we'll prefer the filename as presented in the content-disposition field
	// (if it's present)
    if (headers.HasContentDisposition()) 
	{
		// 6/25/02 - now running these through the encoded-word decoding process;
		// if these aren't decoded before they're saved to the filesystem, the save
		// will bomb
		//DwText fname(headers.ContentDisposition().Filename());
		//fname.Parse();
		//DecodeField(fname, filenameStr);
		char* szFilename = rfc2047_decode_simple(headers.ContentDisposition().Filename().c_str());		

		string charset = CMessage::GetSubjectCharset(headers.ContentDisposition().Filename().c_str());

		f.Format("CMessage::_GetAttachment - attachment charset: %s", charset.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				
		if ( strcasecmp(charset.c_str(), "utf-8") == 0)
		{
			filenameStr.assign(szFilename);								
		}
		else
		{
			wstring sData;
			int Len = strlen( szFilename ) + 1;
			UINT nCodePage = CMessage::GetCodePage( charset );
			
			int nWcsSize = MultiByteToWideChar( nCodePage, 0, szFilename, Len, NULL, 0  );
			sData.resize( nWcsSize, L' ' );
			MultiByteToWideChar( nCodePage, 0, szFilename, Len, (LPWSTR)(sData.c_str()), nWcsSize  );

			f.Format("CMessage::_GetAttachmentt - filename as codepage %d: %s", nCodePage, sData.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

			int nChrSize = WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
			char* szBuff = new char[nChrSize+1];
			WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
			szBuff[nChrSize] = 0;
			
			f.Format("CMessage::_GetAttachment - filename as UTF-8: %s", szBuff);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

			filenameStr.assign(szBuff);				
		}
		
		aAttachment.SetContentDisposition(A2T(headers.ContentDisposition().DispositionTypeStr().c_str()));
    }

	// if filename is still blank, try the "name" field fo the content-type header
    if (filenameStr[0] == 0) 
	{
		// 6/25/02 - now running these through the encoded-word decoding process
		//DwText fname(headers.ContentType().Name());
		//fname.Parse();
		//DecodeField(fname, filenameStr);

		char* szFilename = rfc2047_decode_simple(headers.ContentType().Name().c_str());
		string charset = CMessage::GetSubjectCharset(headers.ContentType().Name().c_str());

		f.Format("CMessage::_GetAttachment - attachment charset: %s", charset.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				
		if ( strcasecmp(charset.c_str(), "utf-8") == 0)
		{
			filenameStr.assign(szFilename);								
		}
		else
		{
			wstring sData;
			int Len = strlen( szFilename ) + 1;
			UINT nCodePage = CMessage::GetCodePage( charset );
			
			int nWcsSize = MultiByteToWideChar( nCodePage, 0, szFilename, Len, NULL, 0  );
			sData.resize( nWcsSize, L' ' );
			MultiByteToWideChar( nCodePage, 0, szFilename, Len, (LPWSTR)(sData.c_str()), nWcsSize  );

			f.Format("CMessage::_GetAttachmentt - filename as codepage %d: %s", nCodePage, sData.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

			int nChrSize = WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), NULL, 0, NULL, NULL );
			char* szBuff = new char[nChrSize+1];
			WideCharToMultiByte( 65001, 0, sData.c_str(), sData.size(), szBuff, nChrSize, NULL, NULL );
			szBuff[nChrSize] = 0;
			
			f.Format("CMessage::_GetAttachment - filename as UTF-8: %s", szBuff);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);

			filenameStr.assign(szBuff);				
		}
    }

	// set attachment filename string (if there is one)
	if (filenameStr.size() > 0)
		aAttachment.SetFileName(filenameStr);

	// set the content id if available
	if (headers.HasContentId())
	{
		// store off the content-id, minus the brackets
		DwString sCID = headers.ContentId().AsString().c_str();
		if ((sCID[0] == '<') && (sCID[sCID.size()-1] == '>'))
		{
			sCID.erase(0, 1);
			sCID.erase(sCID.size()-1, 1);
		}

		aAttachment.SetContentID(sCID.c_str());
	}

	// set content description
    string descStr;
    if (headers.HasContentDescription())
	{
        descStr = headers.ContentDescription().AsString().c_str();
		aAttachment.SetDescription(descStr);
	}

	// media type/substype
    aAttachment.SetMediaType(typeStr, subtypeStr);
  
	// if encoded, then let's decode it
	int nRet;
    DwString content = body.AsString();
    if (headers.HasContentTransferEncoding()) 
	{
        DebugReporter::Instance().DisplayMessage("_GetAttachment - has content transfer encoding", DebugReporter::MAIL);
		const DwString& cte = headers.ContentTransferEncoding().AsString();

		// decode base64
        if (DwStrcasecmp(cte, "base64") == 0) 
		{
            DebugReporter::Instance().DisplayMessage("_GetAttachment - decode base 64", DebugReporter::MAIL);
			nRet = DwDecodeBase64(content, content);
			if(nRet != 0)
			{
				dca::String f;
				f.Format("_GetAttachment - decode base 64 failed for attachment [%s]", filenameStr.c_str() );
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			}
        }

		// decode quoted-printable
        else if (DwStrcasecmp(cte, "quoted-printable") == 0) 
		{
            DebugReporter::Instance().DisplayMessage("_GetAttachment - decode quoted-printable", DebugReporter::MAIL);
			nRet = DwDecodeQuotedPrintable(content, content);
			CkString str;
			str.append(content.c_str());
			if ( str.containsSubstringNoCase("oracle pdf driver") && str.containsSubstringNoCase("oracle reports") && str.containsSubstringNoCase("Oracle10gR2"))
			{
				CkString dot;
				dot.append(".");
				CkString ddot;
				ddot.append("..");
				str.replaceAll(ddot,dot);
				content = str.getString();
			}
			else if ( str.containsSubstringNoCase("oracle pdf driver") && str.containsSubstringNoCase("oracle reports") && str.containsSubstringNoCase("Oracle11gR1"))
			{
				std::string result;
				std::istringstream iss(str.getString());
				std::string dot = ".";

				for (std::string line; std::getline(iss, line); )
				{
					int nLength = line.length();
					int nFind = line.find("..");
					
					if(nLength == 76 && nFind >= 0)
					{
						line.replace(line.find(".."),2,dot);
					}
					result.append(line + "\r\n");
				}
				
				str.clear();
				str.append(result.c_str());
				content = str.getString();
			}
			assert(nRet == 0);
        }

		// decode uuencode
        else if (DwStrcasecmp(cte, "x-uuencode") == 0) 
		{
            DebugReporter::Instance().DisplayMessage("_GetAttachment - decode uuencode", DebugReporter::MAIL);
			DwUuencode decoder;
            decoder.SetAsciiChars(content);
            nRet = decoder.Decode();
			assert(nRet == 0);
			if (nRet == 0)
				content = decoder.BinaryChars();
        }
    }

	// binhex4 attachments do not have a content-transfer-encoding,
	// but rather indicate "mac-binhex50" as the content sub-type
	else if (strcasecmp(subtypeStr.c_str(), "mac-binhex40") == 0) 
	{
		DebugReporter::Instance().DisplayMessage("_GetAttachment - binhex4 attachments do not have a content-transfer-encoding but rather indicate 'mac-binhex50' as the content sub-type", DebugReporter::MAIL);

		DwBinhex binhex;
		binhex.SetBinhexChars(content);
		nRet = binhex.Decode();
		assert(nRet == 0);
		if (nRet == 0)
			content = binhex.DataFork();
	}

	// attachments may be in MacMIME (AppleDouble/AppleSingle) file 
	// format, in which case we'll want to decode it;
	// note that applefile attachments will frequently also be encoded
	// in base64
	else if (strcasecmp(subtypeStr.c_str(), "applefile") == 0)
	{
		DebugReporter::Instance().DisplayMessage("_GetAttachment - ttachments may be in MacMIME (AppleDouble/AppleSingle) file format, in which case we'll want to decode it; note that applefile attachments will frequently also be encoded in base64", DebugReporter::MAIL);
		DwAppleFile afile;
		afile.SetFormat(DwAppleFile::kAppleDoubleHeader);
		afile.SetPackagedData(content);
		nRet = afile.Unpack();
		assert(nRet == 0);
		if (nRet == 0)
			content = afile.DataFork();
	}

	// set the decoded attachment content
    aAttachment.SetContent(content);

	return 0;
}


static void _SetSubject(DwMessage& aMessage, const char* aText, const char* aCharset, int aEncoding=0)
{
    int textLen = strlen(aText);
    int charsetLen = strlen(aCharset);

    // If the encoding type is ISO-8859-?, and if all characters are
    // 7-bit, then don't encode

    int num8BitChars = 0;
    int i;
    for (i=0; i < textLen; ++i) 
	{
        int ch = aText[i] & 0xff;
        if (ch < 32 || 126 < ch) 
		{
            ++num8BitChars;
        }
    }
    if (strcasecmp(aCharset, "US-ASCII") == 0
        || (strncasecmp(aCharset, "ISO-8859", 8) == 0 && num8BitChars == 0)) 
	{

        DwString str = aText;
        DwFoldLine(str, 9);
        aMessage.Headers().Subject().FromString(str);
        aMessage.Headers().Subject().SetModified();
        aMessage.Headers().Subject().Parse();
        return;
    }

    // If no encoding is given, then determine a reasonable default:
    // Use Q encoding for:
    //    ISO-8859-1 (Latin-1)
    //    ISO-8859-2 (Latin-2)
    //    ISO-8859-3 (Latin-3)
    //    ISO-8859-4 (Latin-4)
    //    ISO-8859-9 (Latin-5)
    // Use B encoding for all others

    if (aEncoding == 0) 
	{
        if (strcasecmp(aCharset, "ISO-8859-1") == 0
            || strcasecmp(aCharset, "ISO-8859-2") == 0
            || strcasecmp(aCharset, "ISO-8859-3") == 0
            || strcasecmp(aCharset, "ISO-8859-4") == 0
            || strcasecmp(aCharset, "ISO-8859-9") == 0) 
		{
            aEncoding = 'Q';
        }
        else 
		{
            aEncoding = 'B';
        }
    }
    else if (aEncoding != 'q' && aEncoding != 'Q'
        && aEncoding != 'b' && aEncoding != 'B') 
	{
        aEncoding = 'B';
    }
    if (aEncoding == 'q' || aEncoding == 'Q') 
	{
        aMessage.Headers().Subject().DeleteAllEncodedWords();
        int startPos = 0;
        int endPos = 0;
        int count = 0;
        while (endPos < textLen) 
		{
            int ch = aText[endPos] & 0xff;
            if (ch < 32 || 126 < ch || ch == 61 || ch == 63) 
			{
                count += 2;
            }
            ++count;
            ++endPos;
            if (count + charsetLen + 7 > 64) 
			{
                DwEncodedWord* word = new DwEncodedWord;
                word->SetCharset(aCharset);
                word->SetEncodingType(aEncoding);
                DwString s(aText, startPos, endPos-startPos);
                word->SetDecodedText(s);
                aMessage.Headers().Subject().AddEncodedWord(word);
                count = 0;
                startPos = endPos;
            }
        }
        if (count > 0) 
		{
            DwEncodedWord* word = new DwEncodedWord;
            word->SetCharset(aCharset);
            word->SetEncodingType(aEncoding);
            DwString s(aText, startPos, endPos-startPos);
            word->SetDecodedText(s);
            aMessage.Headers().Subject().AddEncodedWord(word);
        }
        aMessage.Headers().Subject().Assemble();
    }
    else /* if (aEncoding != 'q' && aEncoding != 'Q') */ 
	{
        aMessage.Headers().Subject().DeleteAllEncodedWords();
        int startPos = 0;
        int endPos = 0;
        while (endPos < textLen) 
		{
            endPos += 39;
            endPos = (endPos < textLen) ? endPos : textLen;
            DwEncodedWord* word = new DwEncodedWord;
            word->SetCharset(aCharset);
            word->SetEncodingType(aEncoding);
            DwString s(aText, startPos, endPos-startPos);
            word->SetDecodedText(s);
            aMessage.Headers().Subject().AddEncodedWord(word);
            startPos = endPos;
        }
        aMessage.Headers().Subject().Assemble();
    }
}


static int _RecommendEncoding(const CEmailText& aText)
{
    // Determine the encoding type.  For all ISO-8859-X types, if there
    // are no 8-bit characters, then we don't need to encode.  Otherwise,
    // we use 'Q' encoding or 'B' encoding.

    // To do: add the Windows code pages charsets, such as 1252

    // Check for 8-bit characters.  If there are no 8-bit characters,
    // we may not have to encode the personal name

    const char* ptr = aText.GetText().data();
    int len = (int) aText.GetText().length();
    DwBool has8bitChars = DwFalse;
    for (int i=0; i < len; ++i) 
	{
        int ch = ptr[i] & 0xff;
        if (ch < 32 || 126 < ch) 
		{
            has8bitChars = DwTrue;
            break;
        }
    }

    int encoding;
    if (strcasecmp(aText.GetCharset().c_str(), "US-ASCII") == 0)			// ASCII
	{
        encoding = 0;
    }
    else if (strncasecmp(aText.GetCharset().c_str(), "ISO-8859", 8) == 0)
	{
        if (!has8bitChars) 
		{
            encoding = 0;
        }
        else if (strcasecmp(aText.GetCharset().c_str(), "ISO-8859-1") == 0	// Latin1 (West European)
            || strcasecmp(aText.GetCharset().c_str(), "ISO-8859-2") == 0	// Latin2 (East European)
            || strcasecmp(aText.GetCharset().c_str(), "ISO-8859-3") == 0	// Latin3 (South European)
            || strcasecmp(aText.GetCharset().c_str(), "ISO-8859-4") == 0	// Latin4 (North European)
            || strcasecmp(aText.GetCharset().c_str(), "ISO-8859-9") == 0)	// Latin5 (Turkish)
		{
            encoding = 'Q';
        }
        else		
		{
            encoding = 'B';
        }
    }
    else 
	{
        encoding = 'B';
    }
	
    return encoding;
}


tstring CMessage::GetToListString()
{
	return GetAddressListString(m_ToRecipients);
}

tstring CMessage::GetCcListString()
{
	return GetAddressListString(m_CcRecipients);
}

tstring CMessage::GetReplyToListString()
{
	return GetAddressListString(m_ReplyTo);
}

const tstring CMessage::GetAddressListString(CEmailAddressList& addrList)
{
	USES_CONVERSION;
		
	tstring tolist(_T(""));
	for (int i=0; i<addrList.GetCount(); i++)
	{
		const CEmailAddress& addr = addrList.Get(i);
		
		if (i > 0)
			tolist.append(_T(";"));
		
		if (addr.GetFullname().size() > 0)
		{
			// Strip all spaces
			string x;
			x.assign(addr.GetFullname());
						
			basic_string<char, std::char_traits<char> >::iterator i = x.begin();
			while(i != x.end())
			{
				if(isspace(*i)) i = x.erase(i);
				else i++;
			}
			
			if ( x.length() > 2 )
			{
				tolist.append(A2T(addr.GetFullname().c_str()));
				tolist.append(_T(" "));
			}
		}
		
		if (addr.GetInetName()[0] != '<')
			tolist.append(_T("<"));
		
		tolist.append(A2T(addr.GetInetName().c_str()));

		if (addr.GetInetName()[addr.GetInetName().size()-1] != '>')
			tolist.append(_T(">"));
	}

	if (tolist.size() == 0)
		tolist.append(_T("\0"));

	return tolist;
}

// not presently being used; requires work in the ISAPI filter to work correctly
void CMessage::ResolveContentDepends()
{
	USES_CONVERSION;

	// create an editable buffer
	string sBody = m_MemoText.GetText();

	TCHAR file[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	// iterate over attachment list
	for (int i=0; i<m_Attachments.GetCount(); i++)
	{
		CAttachment* pAtt = m_Attachments.GetAttachment(i);

		// the string we want to amtch is the attachment contentID
		// prefixed with "cid:"
		const string sCID = string("cid:") + pAtt->GetContentID();

		// find this attachment content-id in the body
		size_t nPos = sBody.find(sCID.c_str(), 0);
		if (nPos != string::npos)
		{
			// full attachment path should be unique at this point
			tstring storedname = pAtt->GetFullpath();			
			storedname.resize(ATTACHMENTS_ATTACHMENTLOCATION_LENGTH-1, '\0');
			_tsplitpath(storedname.c_str(), NULL, NULL, file, ext);
			storedname = (tstring)file + (tstring)ext;

			// download.ems?type="inbound"&filename=<blah>
			string sResolve = "download.ems?type=inbound&filename=";
			sResolve += T2A(storedname.c_str());

			// we found a match, now resolve the dependency
			sBody.replace(nPos, sCID.size(), sResolve.c_str());
		}
	}

	// replace the old msg body with the new
	m_MemoText.SetText(sBody, m_MemoText.GetCharset());
}

void CMessage::ParseDate(const DwMessage &message, CEmailDate &edate)
{
	DwHeaders& headers = message.Headers();

	if (headers.HasDate()) 
	{
		// this is the "date" field from the email message,
		// and represents the date the messages was sent (by the sender)
		DwDateTime& date = headers.Date();

		// current date/time
		DwDateTime cur;

		// it's possible that the date parsing process failed, in which case the
		// MIME++ components will set 1 Jan 1970 00:00:00 UTC; we check for this
		// condition and set the date to the current time if it occurs
		DwUint32 nUnixTime = date.AsUnixTime();
		if (nUnixTime == 0)
			date = cur;
		
		// it's also possible that the "sent date" as applied by the originating
		// email client is completely, utterly wrong - possibly even representing
		// a date in the future, which really barfs things up. if we encounter this,
		// then we'll use the current date

		/* STB 2-24-03 - Note: Had to comment this out as this function was not checked in
		 *                      as of build time.
		if (CEmailDate::CompareDwDateTimes(date, cur) == 1)
		{
			// email sent time is later than current date
			date = cur;
		}
		*/

		int year = date.Year();
		int month = date.Month();
		int day = date.Day();
		int hour = date.Hour();
		int minute = date.Minute();
		int second = date.Second();
		int zone = date.Zone();
		int dayofweek = date.DayOfTheWeek();

		// validate the range for SQL TIMESTAMP_STRUCT
		// note: if this date/time value is out of range, we'll use the present
		// date/time, as set in the m_Date constructor
		if (	(year < 9999 && year > 1753) &&
				(month < 13 && month > 0) &&
				(hour >= 0 && hour < 25) &&
				(minute >= 0 && minute < 61) &&
				(second >= 0 && second < 61) &&
				(day < 32 && day > 0))
		{
			// do the local converstion (note: if this function fails,
			// the current local date/time will be used)
			m_Date.SetValuesConvertLocal(	year,
											month,
											day,
											hour,
											minute,
											second,
											dayofweek,
											zone);
		}
	}

	// note: if no date is present in the email headers, the current local date/time
	// will be used
}

////////////////////////////////////////////////////////////////////////////////
// 
// AddQuotesToRealNames -- pre-processing for from, to, reply-to, and cc fields
// 
////////////////////////////////////////////////////////////////////////////////
bool CMessage::AddQuotesToRealNames( DwString& fBody )
{
	// Check for unquoted real names
	std::string::size_type lastpos = 0;
	std::string::size_type pos = 0;
	std::string::size_type quotepos;

	{
		dca::String f;
		f.Format("CMessage::AddQuotesToRealNames - begin addquotestorealNames: %s", fBody.c_str() );
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
	}

	bool bFound = true;

	while((pos = fBody.find('<', lastpos)) != DwString::npos) 
	{
		quotepos = fBody.find( '\"', lastpos );
		if( quotepos == DwString::npos || quotepos > pos )
		{
			std::string::size_type insertpos = lastpos;
			while( fBody.at(insertpos) == ' ')
				insertpos++;
			fBody.insert( insertpos, 1, '\"' );
			pos++;

			insertpos = pos;
			while( fBody.at(insertpos-1) == ' ')
				insertpos--;
			fBody.insert( insertpos, 1, '\"' );
			pos++;
			bFound = true;
		}
		
		lastpos = fBody.find('>',pos);
		if( lastpos != DwString::npos )
		{
			while ( (fBody.at(lastpos) == ',' || fBody.at(lastpos) == ';' || fBody.at(lastpos) == '>') && lastpos < fBody.length() )
			{
				lastpos++;
			}
		}
	}


	LINETRACE( _T("END   AddQuotesToRealNames: %S\n"), fBody.c_str() );

	return bFound;
}

UINT CMessage::GetCodePage( string charset )
{
	if ( strcasecmp(charset.c_str(), "IBM037") == 0 ){return 37;}
	else if ( strcasecmp(charset.c_str(), "IBM437") == 0 ){return 437;}
	else if ( strcasecmp(charset.c_str(), "IBM500") == 0 ){return 500;}
	else if ( strcasecmp(charset.c_str(), "ASMO-708") == 0 ){return 708;}
	else if ( strcasecmp(charset.c_str(), "DOS-720") == 0 ){return 720;}
	else if ( strcasecmp(charset.c_str(), "ibm737") == 0 ){return 737;}
	else if ( strcasecmp(charset.c_str(), "ibm775") == 0 ){return 775;}
	else if ( strcasecmp(charset.c_str(), "ibm850") == 0 ){return 850;}
	else if ( strcasecmp(charset.c_str(), "ibm852") == 0 ){return 852;}
	else if ( strcasecmp(charset.c_str(), "IBM855") == 0 ){return 855;}
	else if ( strcasecmp(charset.c_str(), "ibm857") == 0 ){return 857;}
	else if ( strcasecmp(charset.c_str(), "IBM00858") == 0 ){return 858;}
	else if ( strcasecmp(charset.c_str(), "IBM860") == 0 ){return 860;}
	else if ( strcasecmp(charset.c_str(), "ibm861") == 0 ){return 861;}
	else if ( strcasecmp(charset.c_str(), "DOS-862") == 0 ){return 862;}
	else if ( strcasecmp(charset.c_str(), "IBM863") == 0 ){return 863;}
	else if ( strcasecmp(charset.c_str(), "IBM864") == 0 ){return 864;}
	else if ( strcasecmp(charset.c_str(), "IBM865") == 0 ){return 865;}
	else if ( strcasecmp(charset.c_str(), "cp866") == 0 ){return 866;}
	else if ( strcasecmp(charset.c_str(), "ibm869") == 0 ){return 869;}
	else if ( strcasecmp(charset.c_str(), "IBM870") == 0 ){return 870;}
	else if ( strcasecmp(charset.c_str(), "windows-874") == 0 ){return 874;}
	else if ( strcasecmp(charset.c_str(), "cp875") == 0 ){return 875;}
	else if ( strcasecmp(charset.c_str(), "shift_jis") == 0 ){return 932;}
	else if ( strcasecmp(charset.c_str(), "gb2312") == 0 ){return 936;}
	else if ( strcasecmp(charset.c_str(), "gbk") == 0 ){return 936;}
	else if ( strcasecmp(charset.c_str(), "ks_c_5601-1987") == 0 ){return 949;}
	else if ( strcasecmp(charset.c_str(), "big5") == 0 ){return 950;}
	else if ( strcasecmp(charset.c_str(), "IBM1026") == 0 ){return 1026;}
	else if ( strcasecmp(charset.c_str(), "IBM01047") == 0 ){return 1047;}
	else if ( strcasecmp(charset.c_str(), "IBM01140") == 0 ){return 1140;}
	else if ( strcasecmp(charset.c_str(), "IBM01141") == 0 ){return 1141;}

	if ( strcasecmp(charset.c_str(), "IBM01142") == 0 ){return 1142;}
	else if ( strcasecmp(charset.c_str(), "IBM01143") == 0 ){return 1143;}
	else if ( strcasecmp(charset.c_str(), "IBM01144") == 0 ){return 1144;}
	else if ( strcasecmp(charset.c_str(), "IBM01145") == 0 ){return 1145;}
	else if ( strcasecmp(charset.c_str(), "IBM01146") == 0 ){return 1146;}
	else if ( strcasecmp(charset.c_str(), "IBM01147") == 0 ){return 1147;}
	else if ( strcasecmp(charset.c_str(), "IBM01148") == 0 ){return 1148;}
	else if ( strcasecmp(charset.c_str(), "IBM01149") == 0 ){return 1149;}
	else if ( strcasecmp(charset.c_str(), "utf-16") == 0 ){return 1200;}
	else if ( strcasecmp(charset.c_str(), "unicodeFFFE") == 0 ){return 1201;}
	else if ( strcasecmp(charset.c_str(), "windows-1250") == 0 ){return 1250;}
	else if ( strcasecmp(charset.c_str(), "windows-1251") == 0 ){return 1251;}
	else if ( strcasecmp(charset.c_str(), "windows-1252") == 0 ){return 1252;}
	else if ( strcasecmp(charset.c_str(), "windows-1253") == 0 ){return 1253;}
	else if ( strcasecmp(charset.c_str(), "windows-1254") == 0 ){return 1254;}
	else if ( strcasecmp(charset.c_str(), "windows-1255") == 0 ){return 1255;}
	else if ( strcasecmp(charset.c_str(), "windows-1256") == 0 ){return 1256;}
	else if ( strcasecmp(charset.c_str(), "windows-1257") == 0 ){return 1257;}
	else if ( strcasecmp(charset.c_str(), "windows-1258") == 0 ){return 1258;}
	else if ( strcasecmp(charset.c_str(), "Johab") == 0 ){return 1361;}
	else if ( strcasecmp(charset.c_str(), "macintosh") == 0 ){return 10000;}
	else if ( strcasecmp(charset.c_str(), "x-mac-japanese") == 0 ){return 10001;}
	else if ( strcasecmp(charset.c_str(), "x-mac-chinesetrad") == 0 ){return 10002;}
	else if ( strcasecmp(charset.c_str(), "x-mac-korean") == 0 ){return 10003;}
	else if ( strcasecmp(charset.c_str(), "x-mac-arabic") == 0 ){return 10004;}
	else if ( strcasecmp(charset.c_str(), "x-mac-hebrew") == 0 ){return 10005;}
	else if ( strcasecmp(charset.c_str(), "x-mac-greek") == 0 ){return 10006;}
	else if ( strcasecmp(charset.c_str(), "x-mac-cyrillic") == 0 ){return 10007;}
	else if ( strcasecmp(charset.c_str(), "x-mac-chinesesimp") == 0 ){return 10008;}
	else if ( strcasecmp(charset.c_str(), "x-mac-romanian") == 0 ){return 10010;}
	else if ( strcasecmp(charset.c_str(), "x-mac-ukrainian") == 0 ){return 10017;}
	
	
	if ( strcasecmp(charset.c_str(), "x-mac-thai") == 0 ){return 10021;}
	else if ( strcasecmp(charset.c_str(), "x-mac-ce") == 0 ){return 10029;}
	else if ( strcasecmp(charset.c_str(), "x-mac-icelandic") == 0 ){return 10079;}
	else if ( strcasecmp(charset.c_str(), "x-mac-turkish") == 0 ){return 10081;}
	else if ( strcasecmp(charset.c_str(), "x-mac-croatian") == 0 ){return 10082;}
	else if ( strcasecmp(charset.c_str(), "utf-32") == 0 ){return 12000;}
	else if ( strcasecmp(charset.c_str(), "utf-32BE") == 0 ){return 12001;}
	else if ( strcasecmp(charset.c_str(), "x-Chinese_CNS") == 0 ){return 20000;}
	else if ( strcasecmp(charset.c_str(), "x-cp20001") == 0 ){return 20001;}
	else if ( strcasecmp(charset.c_str(), "x_Chinese-Eten") == 0 ){return 20002;}
	else if ( strcasecmp(charset.c_str(), "x-cp20003") == 0 ){return 20003;}
	else if ( strcasecmp(charset.c_str(), "x-cp20004") == 0 ){return 20004;}
	else if ( strcasecmp(charset.c_str(), "x-cp20005") == 0 ){return 20005;}
	else if ( strcasecmp(charset.c_str(), "x-IA5") == 0 ){return 20105;}
	else if ( strcasecmp(charset.c_str(), "x-IA5-German") == 0 ){return 20106;}
	else if ( strcasecmp(charset.c_str(), "x-IA5-Swedish") == 0 ){return 20107;}
	else if ( strcasecmp(charset.c_str(), "x-IA5-Norwegian") == 0 ){return 20108;}
	else if ( strcasecmp(charset.c_str(), "us-ascii") == 0 ){return 20127;}
	else if ( strcasecmp(charset.c_str(), "x-cp20261") == 0 ){return 20261;}
	else if ( strcasecmp(charset.c_str(), "x-cp20269") == 0 ){return 20269;}
	else if ( strcasecmp(charset.c_str(), "IBM273") == 0 ){return 20273;}
	else if ( strcasecmp(charset.c_str(), "IBM277") == 0 ){return 20277;}
	else if ( strcasecmp(charset.c_str(), "IBM278") == 0 ){return 20278;}
	else if ( strcasecmp(charset.c_str(), "IBM280") == 0 ){return 20280;}
	else if ( strcasecmp(charset.c_str(), "IBM284") == 0 ){return 20284;}
	else if ( strcasecmp(charset.c_str(), "IBM285") == 0 ){return 20285;}
	else if ( strcasecmp(charset.c_str(), "IBM290") == 0 ){return 20290;}
	else if ( strcasecmp(charset.c_str(), "IBM297") == 0 ){return 20297;}
	else if ( strcasecmp(charset.c_str(), "IBM420") == 0 ){return 20420;}
	else if ( strcasecmp(charset.c_str(), "IBM423") == 0 ){return 20423;}
	else if ( strcasecmp(charset.c_str(), "IBM424") == 0 ){return 20424;}
	
	
	if ( strcasecmp(charset.c_str(), "x-EBCDIC-KoreanExtended") == 0 ){return 20833;}
	else if ( strcasecmp(charset.c_str(), "IBM-Thai") == 0 ){return 20838;}
	else if ( strcasecmp(charset.c_str(), "koi8-r") == 0 ){return 20866;}
	else if ( strcasecmp(charset.c_str(), "IBM871") == 0 ){return 20871;}
	else if ( strcasecmp(charset.c_str(), "IBM880") == 0 ){return 20880;}
	else if ( strcasecmp(charset.c_str(), "IBM905") == 0 ){return 20905;}
	else if ( strcasecmp(charset.c_str(), "IBM00924") == 0 ){return 20924;}
	else if ( strcasecmp(charset.c_str(), "EUC-JP") == 0 ){return 20932;}
	else if ( strcasecmp(charset.c_str(), "x-cp20936") == 0 ){return 20936;}
	else if ( strcasecmp(charset.c_str(), "x-cp20949") == 0 ){return 20949;}
	else if ( strcasecmp(charset.c_str(), "cp1025") == 0 ){return 21025;}
	else if ( strcasecmp(charset.c_str(), "koi8-u") == 0 ){return 21866;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-1") == 0 ){return 0;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-2") == 0 ){return 28592;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-3") == 0 ){return 28593;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-4") == 0 ){return 28594;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-5") == 0 ){return 28595;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-6") == 0 ){return 28596;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-7") == 0 ){return 28597;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-8") == 0 ){return 28598;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-9") == 0 ){return 28599;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-13") == 0 ){return 28603;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-15") == 0 ){return 28605;}
	else if ( strcasecmp(charset.c_str(), "x-Europa") == 0 ){return 29001;}
	else if ( strcasecmp(charset.c_str(), "iso-8859-8-i") == 0 ){return 38598;}
	else if ( strcasecmp(charset.c_str(), "iso-2022-jp") == 0 ){return 50220;}
	else if ( strcasecmp(charset.c_str(), "csISO2022JP") == 0 ){return 50221;}
	else if ( strcasecmp(charset.c_str(), "iso-2022-jp") == 0 ){return 50222;}
	else if ( strcasecmp(charset.c_str(), "iso-2022-kr") == 0 ){return 50225;}
	else if ( strcasecmp(charset.c_str(), "x-cp50227") == 0 ){return 50227;}
	else if ( strcasecmp(charset.c_str(), "euc-jp") == 0 ){return 51932;}
	else if ( strcasecmp(charset.c_str(), "EUC-CN") == 0 ){return 51936;}
	else if ( strcasecmp(charset.c_str(), "euc-kr") == 0 ){return 51949;}
	else if ( strcasecmp(charset.c_str(), "hz-gb-2312") == 0 ){return 52936;}
	else if ( strcasecmp(charset.c_str(), "GB18030") == 0 ){return 54936;}
	else if ( strcasecmp(charset.c_str(), "x-iscii-de") == 0 ){return 57002;}
	else if ( strcasecmp(charset.c_str(), "x-iscii-be") == 0 ){return 57003;}
	else if ( strcasecmp(charset.c_str(), "x-iscii-ta") == 0 ){return 57004;}
	else if ( strcasecmp(charset.c_str(), "x-iscii-te") == 0 ){return 57005;}
	else if ( strcasecmp(charset.c_str(), "x-iscii-as") == 0 ){return 57006;}
	else if ( strcasecmp(charset.c_str(), "x-iscii-or") == 0 ){return 57007;}
	else if ( strcasecmp(charset.c_str(), "x-iscii-ka") == 0 ){return 57008;}
	else if ( strcasecmp(charset.c_str(), "x-iscii-ma") == 0 ){return 57009;}
	else if ( strcasecmp(charset.c_str(), "x-iscii-gu") == 0 ){return 57010;}
	else if ( strcasecmp(charset.c_str(), "x-iscii-pa") == 0 ){return 57011;}
	else if ( strcasecmp(charset.c_str(), "utf-7") == 0 ){return 65000;}
	else if ( strcasecmp(charset.c_str(), "utf-8") == 0 ){return 65001;}
	
	return 0;
}