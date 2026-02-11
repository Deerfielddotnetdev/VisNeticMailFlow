/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ViewMsg.cpp,v 1.2.2.1 2006/02/23 20:37:12 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "ViewMsg.h"
#include "ArchiveFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CViewMsg::CViewMsg(CISAPIData& ISAPIData)
	:CXMLDataClass(ISAPIData),
	m_Ticket(ISAPIData),
	m_InboundMessage(ISAPIData),
	m_OutboundMessage(ISAPIData),
	m_nUpdateRead(1),
	m_tResult(0)
{
	m_bIsInbound = false;
	m_bRefreshRequired = false;
	m_nArchiveID = 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Perform the appropriate action	              
\*--------------------------------------------------------------------------*/
int CViewMsg::Run( CURLAction& action )
{
	tstring sMsgType;
	tstring sAction;

	// get the source message ID and type
	GetSourceMsgIDAndType();
	
	// preform the appropriate action
	if(!GetISAPIData().GetFormString( _T("Action"), sAction, true))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true);
		
	if(m_Ticket.m_TicketID > 0)
	{
		return 0;
	}
	
	if ( sAction.compare(_T("delete")) == 0 )
	{
		// delete the message
		m_bIsInbound ? m_InboundMessage.Delete() : m_OutboundMessage.Delete();
		
		GetXMLGen().AddChildElem( _T("Refresh") );
		GetXMLGen().AddChildAttrib( _T("Required"), m_bRefreshRequired ? 1 : 0 );
	}
	if ( sAction.compare(_T("release")) == 0 )
	{
		// release the message
		if(m_bIsInbound)
		{
			return 0;
		}
		else
		{
			m_OutboundMessage.Release();
		}
		
		GetXMLGen().AddChildElem( _T("Refresh") );
		GetXMLGen().AddChildAttrib( _T("Required"), m_bRefreshRequired ? 1 : 0 );
	}
	if ( sAction.compare(_T("return")) == 0 )
	{
		// return the message
		if(m_bIsInbound)
		{
			return 0;
		}
		else
		{
			m_OutboundMessage.Return();
		}
		
		GetXMLGen().AddChildElem( _T("Refresh") );
		GetXMLGen().AddChildAttrib( _T("Required"), m_bRefreshRequired ? 1 : 0 );
	}
	if( sAction.compare(_T("update")) == 0 )
	{
		tstring sBeenRead;
		long lAgentID = GetISAPIData().m_pSession->m_AgentID;
		bool bExist = false;
		
		if(GetISAPIData().GetXMLString( _T("beenread"), sBeenRead, true))
			m_nUpdateRead = atoi(sBeenRead.c_str());

		GetQuery().Initialize();
		dca::String sCmd;
		sCmd.Format(_T("SELECT InboundMessageReadID FROM InboundMessageRead WHERE InboundMessageID = %d AND AgentID = %d"), m_InboundMessage.m_InboundMessageID, lAgentID);
		GetQuery().Execute(sCmd.c_str());
		if(GetQuery().Fetch() == S_OK)
			bExist = true;

		if(m_nUpdateRead)
		{
			if(!bExist)
			{
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_InboundMessage.m_InboundMessageID );
				BINDPARAM_LONG( GetQuery(), lAgentID);
				GetQuery().Execute(_T("INSERT INTO InboundMessageRead (InboundMessageID, AgentID) VALUES (?, ?)"));

				long nTicketID;
				long nTicketBoxID;
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_InboundMessage.m_InboundMessageID);
				BINDCOL_LONG_NOLEN( GetQuery(), nTicketID); 
				BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID); 
				GetQuery().Execute(_T("SELECT t.TicketID,t.TicketBoxID FROM InboundMessages im INNER JOIN Tickets t ON im.TicketID=t.TicketID WHERE (InboundMessageID = ?)"));

				if(GetQuery().Fetch() == S_OK)
				{
					CInboundMessage::UpdateTicketHistoryMsgRead(GetQuery(), lAgentID, nTicketID, 1, m_InboundMessage.m_InboundMessageID, nTicketBoxID);
				}
			}
		}
		else
		{
			GetXMLGen().AddChildElem( _T("postresults") );
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("complete"), _T("false") );
			GetXMLGen().OutOfElem();
		}
	}
	else
	{
		GetISAPIData().GetURLLong(_T("tsresult"), m_tResult, true);
		ViewMsg();
		GenerateXML();
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\            
||  Comments:	Get the message id and type from the URL params	              
\*--------------------------------------------------------------------------*/
void CViewMsg::GetSourceMsgIDAndType(void)
{
	CEMSString sMsgID;
	CEMSString sTicketID;
	tstring sMsgType;
	tstring sArchiveID;

	if( GetISAPIData().GetURLString( _T("ARCHIVEID"), sArchiveID, true ) )
	{
		m_nArchiveID = _ttoi( sArchiveID.c_str() );
	}

	if ( GetISAPIData().GetURLString( _T("MSG"), sMsgID, true ) )
	{
		GetISAPIData().GetURLString( _T("TYPE"), sMsgType );
		m_bIsInbound = ( sMsgType.compare( _T("inbound") ) == 0 );
		
		sMsgID.CDLGetNextInt( m_bIsInbound ? m_InboundMessage.m_InboundMessageID : 
		m_OutboundMessage.m_OutboundMessageID );
	}
	else if ( GetISAPIData().GetURLString( _T("TICKETID"), sTicketID, true ) )
	{
		sTicketID.CDLGetNextInt(m_Ticket.m_TicketID);
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generates XML to view a message	              
\*--------------------------------------------------------------------------*/
void CViewMsg::ViewMsg(const TCHAR* pAction)
{
	// query the message
	
	if( m_nArchiveID )
	{
		list<Attachments_t> attachList;
		list<Attachments_t>::iterator iter;
		CAttachment att( GetISAPIData() );
		int retval;
		tstring sArchive;

		att.m_FileExists = true;

		if( m_bIsInbound )
		{
			retval = GetInboundMessageFromArchive( GetQuery(), m_nArchiveID, 
				                                   m_InboundMessage, m_Ticket, 
												   attachList, sArchive );
			if( retval == Arc_Success )
			{
				for( iter = attachList.begin(); iter != attachList.end(); iter++ )
				{
					att.Copy( &(*iter) );
					m_InboundMessage.GetAttachmentList().push_back( att );
				}
			}
		}
		else 
		{
			retval = GetOutboundMessageFromArchive( GetQuery(), m_nArchiveID, 
				                                    m_OutboundMessage, m_Ticket,
													attachList, sArchive );
			if( retval == Arc_Success )
			{
				for( iter = attachList.begin(); iter != attachList.end(); iter++ )
				{
					att.Copy( &(*iter) );
					m_OutboundMessage.GetAttachmentList().push_back( att );
				}
			}
		}

		switch( retval )
		{
		case Arc_Success:
			break;

		default:
			{
				CEMSString sErrMsg;
				sErrMsg.Format( _T("Error %d reading archive: %s"), retval, sArchive.c_str() );
				THROW_EMS_EXCEPTION_NOLOG( E_ReadingArchive, sErrMsg.c_str() );
				break;
			}
		}
	}
	else
	{
		m_lStart = GetTickCount();
		m_bIsInbound ? m_InboundMessage.Query() : m_OutboundMessage.Query();
		m_lEnd = GetTickCount();
		m_lElapsed = m_lEnd-m_lStart;
		f.Format("CViewMsg::ViewMsg - Query Message Begin:%d - End:%d - Elapsed:%d", m_lStart,m_lEnd,m_lElapsed );
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

		// query the Ticket
		m_lStart = GetTickCount();
		m_Ticket.m_TicketID = m_bIsInbound ? m_InboundMessage.m_TicketID : m_OutboundMessage.m_TicketID;
		if(m_Ticket.m_TicketID > 0)
			m_Ticket.Query();

		m_lEnd = GetTickCount();
		m_lElapsed = m_lEnd-m_lStart;
		f.Format("CViewMsg::ViewMsg - Query Ticket Begin:%d - End:%d - Elapsed:%d", m_lStart,m_lEnd,m_lElapsed );
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());
		
	}

	// if the message doesn't have a subject, use the place holder [NO SUBJECT]
	if ( m_bIsInbound && m_InboundMessage.m_SubjectLen == 0 )
	{
		_tcscpy( m_InboundMessage.m_Subject, _T("[NO SUBJECT]") );
		m_InboundMessage.m_SubjectLen = _tcslen( _T("[NO SUBJECT]") );
	}
	else if ( !m_bIsInbound && m_OutboundMessage.m_SubjectLen == 0 )
	{
		_tcscpy( m_OutboundMessage.m_Subject, _T("[NO SUBJECT]") );
		m_OutboundMessage.m_SubjectLen = _tcslen( _T("[NO SUBJECT]") );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML from class members	              
\*--------------------------------------------------------------------------*/
void CViewMsg::GenerateXML(void)
{	
	// generate message XML
	m_lStart = GetTickCount();
	m_bIsInbound ? m_InboundMessage.GenerateXML( true, m_nArchiveID ) 
		         : m_OutboundMessage.GenerateXML( true, m_nArchiveID );

	GetXMLGen().AddChildAttrib( _T("IsInbound"), m_bIsInbound );
		
	GetXMLGen().AddChildAttrib( _T("MessageAction"), _T("View") );

	GetXMLGen().AddChildAttrib( _T("ArchiveID"), m_nArchiveID );

	GetXMLGen().AddChildAttrib( _T("tsresult"), m_tResult);
	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CViewMsg::GenerateXML - Generate Message XML Begin:%d - End:%d - Elapsed:%d", m_lStart,m_lEnd,m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

    // generate TicketXML
	m_lStart = GetTickCount();
	m_Ticket.GenerateXML();
	m_lEnd = GetTickCount();
	m_lElapsed = m_lEnd-m_lStart;
	f.Format("CViewMsg::GenerateXML - Generate Ticket XML Begin:%d - End:%d - Elapsed:%d", m_lStart,m_lEnd,m_lElapsed );
	DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());

	// add the TicketBoxViewID
	int nTicketBoxView = 0;
	
	if ( GetISAPIData().GetURLLong( _T("TicketBoxView"), nTicketBoxView, true ) )
	{
		GetXMLGen().AddChildElem( _T("TicketBoxView") );
		GetXMLGen().AddChildAttrib( _T("ID"), nTicketBoxView );
	}
}