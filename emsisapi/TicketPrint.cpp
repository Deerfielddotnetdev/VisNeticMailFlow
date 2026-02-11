
#include "stdafx.h"
#include "TicketPrint.h"
#include "Ticket.h"
#include "TicketActions.h"
#include "InboundMessage.h"
#include "OutboundMessage.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CTicketPrint::CTicketPrint(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CTicketPrint::Run( CURLAction& action )
{
	CEMSString sString;
	int m_TicketID = 0;
	
	// get the TicketID		
	if ( GetISAPIData().GetURLString( _T("TICKETID"), sString, true ) ||
		 GetISAPIData().GetXMLString( _T("TICKETID"), sString, true ))
	{
		sString.CDLGetNextInt( m_TicketID );
	}
	
	// check security
	CTicket Ticket(m_ISAPIData, m_TicketID);
	Ticket.RequireRead();

	if( GetISAPIData().m_sPage.compare( _T("printticket") ) == 0)
	{
		DecodeForm();
	}
	else if( GetISAPIData().m_sPage.compare( _T("printticketselect") ) == 0)
	{
		GetXMLGen().AddChildElem( _T("Ticket") );
		GetXMLGen().AddChildAttrib( _T("TicketID"), m_TicketID);
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Decode the form	              
\*--------------------------------------------------------------------------*/
void CTicketPrint::DecodeForm(void)
{
	GetISAPIData().GetURLLong( _T("ticketid"), m_TicketID);
	GetISAPIData().GetURLLong( _T("ts"), m_TicketSummary,true);
	GetISAPIData().GetURLLong( _T("im"), m_InboundMessages,true);
	GetISAPIData().GetURLLong( _T("om"), m_OutboundMessages,true);
	GetISAPIData().GetURLLong( _T("tn"), m_TicketNotes,true);
	GetISAPIData().GetURLLong( _T("te"), m_TicketEvents,true);
	GenerateXML();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML	              
\*--------------------------------------------------------------------------*/
void CTicketPrint::GenerateXML(void)
{
	GetXMLGen().AddChildElem( _T("ToPrint") );
	GetXMLGen().AddChildAttrib( _T("TicketSummary"), m_TicketSummary);
	GetXMLGen().AddChildAttrib( _T("InboundMessages"), m_InboundMessages);
	GetXMLGen().AddChildAttrib( _T("OutboundMessages"), m_OutboundMessages);
	GetXMLGen().AddChildAttrib( _T("TicketNotes"), m_TicketNotes);
	GetXMLGen().AddChildAttrib( _T("TicketEvents"), m_TicketEvents);
		
	if(m_TicketSummary){GenerateTicketXML();}
	if(m_InboundMessages){GenerateIMXML();}
	if(m_OutboundMessages){GenerateOMXML();}
	if(m_TicketNotes){GenerateTNXML();}
	if(m_TicketSummary){GenerateTEXML();}
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML	              
\*--------------------------------------------------------------------------*/
void CTicketPrint::GenerateTicketXML(void)
{
	CTicket Ticket(m_ISAPIData);
	Ticket.m_TicketID = m_TicketID;
	Ticket.Query();
	Ticket.GenerateXML();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML	              
\*--------------------------------------------------------------------------*/
void CTicketPrint::GenerateIMXML(void)
{
	CInboundMessage im(m_ISAPIData);
	vector<int> imIDs;
	vector<int>::iterator iter;

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketID );
	BINDCOL_LONG( GetQuery(), im.m_InboundMessageID );
	GetQuery().Execute( _T("SELECT InboundMessageID FROM InboundMessages WHERE TicketID=? AND IsDeleted=0 ORDER BY InboundMessageID"));
	while ( GetQuery().Fetch() == S_OK )
	{
		imIDs.push_back(im.m_InboundMessageID);		
	}

	GetXMLGen().AddChildElem( _T("Inbound") );
	GetXMLGen().IntoElem();
	for( iter = imIDs.begin(); iter != imIDs.end(); iter++ )
	{
		im.m_InboundMessageID = (*iter);
		im.Query();
		im.GenerateXML(true);
	}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML	              
\*--------------------------------------------------------------------------*/
void CTicketPrint::GenerateOMXML(void)
{
	COutboundMessage om(m_ISAPIData);
	vector<int> omIDs;
	vector<int>::iterator iter;

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketID );
	BINDCOL_LONG( GetQuery(), om.m_OutboundMessageID );
	GetQuery().Execute( _T("SELECT OutboundMessageID FROM OutboundMessages WHERE TicketID=? AND IsDeleted=0 ORDER BY OutboundMessageID"));
	while ( GetQuery().Fetch() == S_OK )
	{
		omIDs.push_back(om.m_OutboundMessageID);		
	}

	GetXMLGen().AddChildElem( _T("Outbound") );
	GetXMLGen().IntoElem();
	for( iter = omIDs.begin(); iter != omIDs.end(); iter++ )
	{
		om.m_OutboundMessageID = (*iter);
		om.Query();
		om.GenerateXML(true);
	}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML	              
\*--------------------------------------------------------------------------*/
void CTicketPrint::GenerateTNXML(void)
{
	TTicketNotes tn;
	vector<int> tnIDs;
	vector<int>::iterator iter;

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketID );
	BINDCOL_LONG( GetQuery(), tn.m_TicketNoteID );
	GetQuery().Execute( _T("SELECT TicketNoteID FROM TicketNotes WHERE TicketID=? ORDER BY TicketNoteID"));
	while ( GetQuery().Fetch() == S_OK )
	{
		tnIDs.push_back(tn.m_TicketNoteID);		
	}

	GetXMLGen().AddChildElem( _T("TicketNotes") );
	GetXMLGen().IntoElem();
	for( iter = tnIDs.begin(); iter != tnIDs.end(); iter++ )
	{
		tn.m_TicketNoteID = (*iter);
		tn.Query(GetQuery());
		
		long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
		TIMESTAMP_STRUCT tsLocal;
		long tsLocalLen=0;
		
		GetXMLGen().AddChildElem( _T("TicketNote") );	
		
		AddAgentName( _T("Agent"), tn.m_AgentID );

		CEMSString sDateCreated;
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(tn.m_DateCreated,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDateCreated);
			}
			else
			{
				GetDateTimeString( tn.m_DateCreated, tn.m_DateCreatedLen, sDateCreated);
			}
		}
		else
		{
			GetDateTimeString( tn.m_DateCreated, tn.m_DateCreatedLen, sDateCreated);
		}
		GetXMLGen().AddChildAttrib(_T("DateCreated"), sDateCreated.c_str());
		
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("Body") );
		GetXMLGen().SetChildData( tn.m_Note, TRUE );
		GetXMLGen().OutOfElem();		
	}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML	              
\*--------------------------------------------------------------------------*/
void CTicketPrint::GenerateTEXML(void)
{
	CTicketActions TA(m_ISAPIData);
	TA.GetHistory(m_TicketID);
}