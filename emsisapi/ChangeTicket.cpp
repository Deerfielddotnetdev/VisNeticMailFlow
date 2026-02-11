#include "stdafx.h"
#include "ChangeTicket.h"
#include "Ticket.h"
#include "ContactFns.h"
#include "TicketHistoryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CChangeTicket::CChangeTicket(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData), m_SourceTicket(ISAPIData), m_InboundMessage(ISAPIData), m_OutboundMessage(ISAPIData)
{
	m_nDestTicketID = 0;
	m_nTicketLink = 0;
	m_nLinkCount = 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CChangeTicket::Run(CURLAction& action)
{
	tstring sType;

	if ( GetISAPIData().GetXMLPost() )
	{
		CEMSString sMergeID;
		CEMSString sLinkID;
		CEMSString sLinkName;
		CEMSString sTicketID;
		tstring sAction;
		CTicket Ticket(m_ISAPIData);
		int nAgentID = GetAgentID();

		if (GetISAPIData().GetURLString( _T("LinkID"), sLinkID, true ))
		{
			if( GetISAPIData().GetURLString( _T("Action"), sAction, true ) )
			{
				int nTicketID = 0;
				m_nDestTicketID = 0;
				bool bDoLinks = true;
				
				if ( sAction.compare( _T("delTickets") ) == 0 )
				{
					bDoLinks = false;
					int nLinkID = 0;
					int nTicketID = 0;
					if (GetISAPIData().GetURLLong( _T("LinkID"), nLinkID, true ))
					{
						if(nLinkID > 0)
						{
							CEMSString sTicketID;
							if (GetISAPIData().GetURLString( _T("TicketID"), sTicketID, true ))
							{
								TTicketLinks tl;
								tl.m_TicketLinkID = nLinkID;
                                tl.Query(GetQuery());
								tstring sName = tl.m_LinkName;
								CEMSString sData1;
								sData1.Format(_T("%s"),sName.c_str());

								// Delete the TicketID(s) from the Ticket Link
								while ( sTicketID.CDLGetNextInt(Ticket.m_TicketID))
								{
									if(!GetIsAdmin())
									{
										// check security
										Ticket.RequireEdit(true);
									}								

									GetQuery().Initialize();
									BINDPARAM_LONG( GetQuery(), Ticket.m_TicketID );		
									BINDPARAM_LONG( GetQuery(), nLinkID );		
									GetQuery().Execute( _T("DELETE FROM TicketLinksTicket ")
													_T("WHERE TicketID=? AND TicketLinkID=? ") );

									// Log Ticket History
									THUnlinkTicket( GetQuery(), Ticket.m_TicketID, nAgentID, sName );		

									//Log it to Agent Activity
									if( _ttoi( Ticket.sAal.c_str() ) > 0 )
									{
										if( _ttoi( Ticket.sTu.c_str() ) > 0 )
										{
											Ticket.LogAgentAction(nAgentID,29,Ticket.m_TicketID,nLinkID,0,sData1,_T(""));
										}
									}		
								}

								// check to see how many Tickets are left in the Ticket Link
								CEMSString sQuery;
								int nCount;

								GetQuery().Initialize();								
								BINDCOL_LONG_NOLEN( GetQuery(), nCount );
								sQuery.Format( _T("SELECT COUNT(*) FROM TicketLinksTicket WHERE TicketLinkID = %d"), nLinkID );
								GetQuery().Execute( sQuery.c_str() );
								GetQuery().Fetch();
								
								if((nCount < 2))
								{
									// Delete the Ticket Link
									GetQuery().Initialize();								
									BINDCOL_LONG_NOLEN( GetQuery(), Ticket.m_TicketID );
									sQuery.Format( _T("SELECT TicketID FROM TicketLinksTicket WHERE TicketLinkID = %d"), nLinkID );
									GetQuery().Execute( sQuery.c_str() );
									GetQuery().Fetch();
									
									if(Ticket.m_TicketID > 0)
									{
										GetQuery().Initialize();
										BINDPARAM_LONG( GetQuery(), Ticket.m_TicketID );		
										BINDPARAM_LONG( GetQuery(), nLinkID );		
										GetQuery().Execute( _T("DELETE FROM TicketLinksTicket ")
														_T("WHERE TicketID=? AND TicketLinkID=? ") );

										// Log Ticket History
										THUnlinkTicket( GetQuery(), Ticket.m_TicketID, nAgentID, sName );		

										//Log it to Agent Activity
										if( _ttoi( Ticket.sAal.c_str() ) > 0 )
										{
											if( _ttoi( Ticket.sTu.c_str() ) > 0 )
											{
												Ticket.LogAgentAction(nAgentID,29,Ticket.m_TicketID,nLinkID,0,sData1,_T(""));
											}
										}
									}
									

									GetQuery().Reset(true);
									BINDPARAM_LONG( GetQuery(), nLinkID );										
									GetQuery().Execute( _T("DELETE FROM TicketLinks ")
													_T("WHERE TicketLinkID=? ") );


									//Log it to Agent Activity
									if( _ttoi( Ticket.sAal.c_str() ) > 0 )
									{
										if( _ttoi( Ticket.sLd.c_str() ) > 0 )
										{
											Ticket.LogAgentAction(nAgentID,27,0,0,0,sData1,_T(""));
										}
									}
								}
							}
							else
							{
								THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, _T("TicketID not defined!"));
							}
						}
					}
					else
					{
						THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, _T("Link ID not defined!"));
					}
				}
				else if ( sAction.compare( _T("linkExist") ) == 0 )
				{
					if (!GetISAPIData().GetURLLong( _T("destTicketId"), m_nDestTicketID, true ))
						GetISAPIData().GetFormLong( _T("destTicketId"), m_nDestTicketID);

					if ( m_nDestTicketID > 0 )
					{
						// check security
						Ticket.m_TicketID =  m_nDestTicketID;
						if(!GetIsAdmin())
						{
							Ticket.RequireEdit(true);
						}							
					}
					else
					{
						// Throw Link Error
						THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, _T("Destination TicketID not defined!"));
					}
				}

				if(bDoLinks)
				{
					sLinkID.CDLInit();
					while ( sLinkID.CDLGetNextInt( Ticket.m_TicketID ) )
					{
						if ( Ticket.m_TicketID != m_nDestTicketID )
						{
							if(!GetIsAdmin())
							{
								Ticket.RequireEdit(true);
							}							
						}							
					}

					// Check TicketBox link settings
					CEMSString sAllTickets;					
					if ( m_nDestTicketID > 0 )
					{
						sAllTickets.Format( _T("%s,%d"),sLinkID.c_str(),m_nDestTicketID);
					}
					else
					{
						sAllTickets = sLinkID;
					}
					TTicketBoxes tb;
					int nNumRows = 0;
					CEMSString sQuery;
					sQuery.Format( _T("SELECT DISTINCT t.TicketBoxID,tb.Name,tb.TicketLink,(SELECT COUNT(DISTINCT t.TicketBoxID) FROM Tickets t INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID WHERE TicketID IN (%s)) ")
								_T("FROM Tickets t INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
								_T("WHERE TicketID IN (%s) "), sAllTickets.c_str(), sAllTickets.c_str());

					GetQuery().Initialize();
					BINDCOL_LONG( GetQuery(), tb.m_TicketBoxID );
					BINDCOL_TCHAR( GetQuery(), tb.m_Name );
					BINDCOL_LONG( GetQuery(), tb.m_TicketLink );
					BINDCOL_LONG_NOLEN( GetQuery(), nNumRows );
					GetQuery().Execute( sQuery.c_str() );

					while( GetQuery().Fetch() == S_OK )
					{
						if((nNumRows == 1 && tb.m_TicketLink == 0) || (nNumRows > 1 && tb.m_TicketLink < 2))
						{
							CEMSString sError;
							if(tb.m_TicketLink == 0)
							{
								sError.Format(_T("%s TicketBox is not enabled for Ticket Linking!"),tb.m_Name);
							}
							else
							{
								sError.Format(_T("%s TicketBox is not enabled for Ticket Linking to Tickets in other TicketBoxes!"),tb.m_Name);
							}
							THROW_EMS_EXCEPTION_NOLOG( E_InvalidParameters, sError.c_str());
						}					
					}

					// Create the Ticket Link and get the ID
					GetISAPIData().GetURLString( _T("linkName"), sLinkName );
					sLinkName.TrimWhiteSpace();

					TCHAR szLinkName[GROUPS_GROUPNAME_LENGTH];
					_tcscpy( szLinkName, sLinkName.c_str() );
									
					GetQuery().Reset();
					BINDPARAM_TCHAR( GetQuery(), szLinkName );
					BINDPARAM_LONG( GetQuery(), GetSession().m_AgentID );
					GetQuery().Execute(	_T("INSERT INTO TicketLinks ")
								_T("(LinkName,OwnerID) ")
								_T("VALUES" )
								_T("(?,?)"));

					int nTicketLinkID;
					GetQuery().Reset();
					BINDCOL_LONG_NOLEN( GetQuery(), nTicketLinkID );
					BINDPARAM_TCHAR( GetQuery(), szLinkName );
					GetQuery().Execute( _T("SELECT TicketLinkID FROM TicketLinks ")
									_T("WHERE LinkName=? ") );

					if( GetQuery().Fetch() != S_OK )
					{
						// Throw Link Error
						THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, _T("Invalid TicketLinkID!")  );
					}
					
					//Log it to Agent Activity
					if( _ttoi( Ticket.sAal.c_str() ) > 0 )
					{
						if( _ttoi( Ticket.sLc.c_str() ) > 0 )
						{
							Ticket.LogAgentAction(nAgentID,26,nTicketLinkID,0,0,sLinkName,_T(""));
						}
					}

					sAllTickets.CDLInit();
					while ( sAllTickets.CDLGetNextInt( Ticket.m_TicketID ) )
					{
						GetQuery().Reset();
						BINDPARAM_LONG( GetQuery(), nTicketLinkID );
						BINDPARAM_LONG( GetQuery(), Ticket.m_TicketID );
						GetQuery().Execute(	_T("INSERT INTO TicketLinksTicket ")
								_T("(TicketLinkID,TicketID) ")
								_T("VALUES" )
								_T("(?,?)"));

						THLinkTicket( GetQuery(), Ticket.m_TicketID, nAgentID, szLinkName );
						if( _ttoi( Ticket.sAal.c_str() ) > 0 )
						{
							if( _ttoi( Ticket.sTl.c_str() ) > 0 )
							{
								Ticket.LogAgentAction(nAgentID,28,Ticket.m_TicketID,nTicketLinkID,0,sLinkName,_T(""));
							}
						}
					}
					
					// Go back to the original TicketBoxView

					GetXMLGen().AddChildElem( _T("DestTicket") );
					GetXMLGen().AddChildAttrib( _T("ID"), nTicketID );

					int nTicketBoxView;
					GetISAPIData().GetURLLong( _T("ChangeTicketBoxView"), nTicketBoxView );
					GetTicketBoxView( nTicketID, nTicketBoxView, false );
				}
			}			
		}
		else if (GetISAPIData().GetURLString( _T("MergeID"), sMergeID, true ))
		{
			if( GetISAPIData().GetURLString( _T("Action"), sAction, true ) )
			{
				int nTicketID = 0;
				m_nDestTicketID = 0;
				
				if( sAction.compare( _T("merge") ) == 0 )
				{
					if ( sMergeID.find( _T(",") ) != CEMSString::npos )
					{
						//determine destination TicketID
						while ( sMergeID.CDLGetNextInt( nTicketID ) )
						{
							if ( nTicketID > m_nDestTicketID )
								m_nDestTicketID = nTicketID;
						}
					}					
				}
				else if ( sAction.compare( _T("mergeExist") ) == 0 )
				{
					//get destination ticketid
					if (!GetISAPIData().GetURLLong( _T("destTicketId"), m_nDestTicketID, true ))
						GetISAPIData().GetFormLong( _T("destTicketId"), m_nDestTicketID);					
				}

				if ( m_nDestTicketID > 0 )
				{
					// check security for the destination ticket
					CTicket destTicket(m_ISAPIData, m_nDestTicketID);
					destTicket.RequireEdit(true);

					sMergeID.CDLInit();

					while ( sMergeID.CDLGetNextInt( nTicketID ) )
					{
						if ( nTicketID != m_nDestTicketID )
						{
							CTicket srcTicket(m_ISAPIData, nTicketID);
							//do merge
							if ( !srcTicket.Merge( m_nDestTicketID, true ) )
							{
								// Throw Merge Error
								THROW_EMS_EXCEPTION_NOLOG( E_EMSException, _T("Ticket Merge operation failed with unknown error!")  );
							}

						}							
					}

					GetXMLGen().AddChildElem( _T("DestTicket") );
					GetXMLGen().AddChildAttrib( _T("ID"), m_nDestTicketID );

					int nTicketBoxView;
					GetISAPIData().GetURLLong( _T("ChangeTicketBoxView"), nTicketBoxView );

					GetTicketBoxView( m_nDestTicketID, nTicketBoxView, true );
				}
				else
				{
					// Throw Merge Error
					THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, _T("Destination TicketID not defined!")  );
				}
			}			
		}
		else if (GetISAPIData().GetURLString( _T("linkName"), sLinkName, true ))
		{
			sLinkName.TrimWhiteSpace();

			TCHAR szLinkName[GROUPS_GROUPNAME_LENGTH];
			_tcscpy( szLinkName, sLinkName.c_str() );
			
			// Check for duplicates
			GetQuery().Initialize();
			BINDPARAM_TCHAR( GetQuery(), szLinkName );
			GetQuery().Execute( _T("SELECT TicketLinkID FROM TicketLinks ")
								_T("WHERE LinkName=? AND IsDeleted=0") );

			GetXMLGen().AddChildElem( _T("Result") );
			if( GetQuery().Fetch() == S_OK )
			{
				GetXMLGen().AddChildAttrib( _T("IsDupe"), 1 );
			}
			else
			{
				GetXMLGen().AddChildAttrib( _T("IsDupe"), 0 );
			}	
		}
		else if (GetISAPIData().GetURLString( _T("srcTicketID"), sTicketID, true ))
		{
			int nLinkID;
			GetISAPIData().GetURLLong( _T("destLinkID"), nLinkID );
			
			TCHAR szLinkName[GROUPS_GROUPNAME_LENGTH];

			// Validate the LinkID
			GetQuery().Initialize();
			BINDCOL_TCHAR_NOLEN( GetQuery(), szLinkName );
			BINDPARAM_LONG( GetQuery(), nLinkID );
			GetQuery().Execute( _T("SELECT LinkName FROM TicketLinks ")
								_T("WHERE TicketLinkID=? AND IsDeleted=0") );

			if( GetQuery().Fetch() != S_OK )
			{
				THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, _T("TicketLinkID is invalid!")  );
			}
			
			tstring sName = szLinkName;
			sLinkName.Format(_T("%s"),sName.c_str());

			//Check ticket security
			sTicketID.CDLInit();
			while ( sTicketID.CDLGetNextInt( Ticket.m_TicketID ) )
			{
				if(!GetIsAdmin())
				{
					Ticket.RequireEdit(true);
				}								
			}

			sTicketID.CDLInit();
			while ( sTicketID.CDLGetNextInt( Ticket.m_TicketID ) )
			{
				int nTicketLinksTicketID;
				GetQuery().Reset();
				BINDCOL_LONG_NOLEN( GetQuery(), nTicketLinksTicketID );
				BINDPARAM_LONG( GetQuery(), Ticket.m_TicketID );
				BINDPARAM_LONG( GetQuery(), nLinkID );
				GetQuery().Execute( _T("SELECT TicketLinksTicketID FROM TicketLinksTicket ")
									_T("WHERE TicketID=? AND TicketLinkID=?") );

				if( GetQuery().Fetch() == S_OK )
				{
					//The ticket is already in the link
				}
				else
				{
					//Add the ticket to the link
					GetQuery().Reset();
					BINDPARAM_LONG( GetQuery(), Ticket.m_TicketID );
					BINDPARAM_LONG( GetQuery(), nLinkID );
					GetQuery().Execute(	_T("INSERT INTO TicketLinksTicket ")
										_T("(TicketID, TicketLinkID) ")
										_T("VALUES" )
										_T("(?,?)"));

					THLinkTicket( GetQuery(), Ticket.m_TicketID, GetSession().m_AgentID, sName );
					if( _ttoi( Ticket.sAal.c_str() ) > 0 )
					{
						if( _ttoi( Ticket.sTl.c_str() ) > 0 )
						{
							Ticket.LogAgentAction(nAgentID,28,Ticket.m_TicketID,nLinkID,0,sLinkName,_T(""));
						}
					}
				}
			}
			
			// Go back to the original TicketBoxView

			GetXMLGen().AddChildElem( _T("DestTicket") );
			GetXMLGen().AddChildAttrib( _T("ID"), Ticket.m_TicketID );

			int nTicketBoxView;
			GetISAPIData().GetURLLong( _T("ChangeTicketBoxView"), nTicketBoxView );
			GetTicketBoxView( Ticket.m_TicketID, nTicketBoxView, false );			
			
		}
		else
		{
			// get parameters as either URL or FORM parameters
			if (!GetISAPIData().GetURLLong( _T("MsgID"), m_SourceMsg.MsgID, true ))
				GetISAPIData().GetFormLong( _T("MsgID"), m_SourceMsg.MsgID);

			if (!GetISAPIData().GetURLString( _T("MsgType"), sType, true ))
				GetISAPIData().GetFormString( _T("MsgType"), sType );
			
			m_SourceMsg.IsInbound = (sType.compare( _T("inbound" ) ) == 0);
			
			if (!GetISAPIData().GetURLLong( _T("DestTicketID"), m_nDestTicketID, true ))
				GetISAPIData().GetFormLong( _T("DestTicketID"), m_nDestTicketID );

			// get the source TicketID
			GetSourceTicketID();

			// check security and lock the source ticket
			m_SourceTicket.RequireDelete(true);
			m_SourceTicket.Lock(false);

			// check security for the destination ticket
			CTicket Ticket(m_ISAPIData, m_nDestTicketID);
			if(!GetIsAdmin())
			{
				Ticket.RequireEdit(true);
			}	
		
			// preform the action
			MoveMessage();
			
			// do we need to delete the source ticket?
			if ( m_SourceTicket.GetMsgCount() < 1 && m_SourceTicket.GetNoteCount() < 1)
			{
				m_SourceTicket.Delete(false);
			}

			GetXMLGen().AddChildElem( _T("DestTicket") );
			GetXMLGen().AddChildAttrib( _T("ID"), m_nDestTicketID );

			tstring sShowDest;
			GetISAPIData().GetURLString( _T("ChangeTicketShowDest"), sShowDest );

			int nTicketBoxView;
			GetISAPIData().GetURLLong( _T("ChangeTicketBoxView"), nTicketBoxView );

			GetTicketBoxView( m_nDestTicketID, nTicketBoxView, sShowDest.compare( _T("true") ) == 0 );
		}
	}
	else
	{
		CEMSString sTicketID;
				
		// get the TicketID		
		if (GetISAPIData().GetURLString( _T("ticketID"), sTicketID, true ))
		{
			bool m_bMultipleTickets = false;
			
			if ( sTicketID.find( _T(",") ) != CEMSString::npos )
			{
				m_bMultipleTickets = true;
			}
			
			if(!m_bMultipleTickets)
			{
				// Check to see if this Ticket is already linked
				int nTicketID = 0;
				
				sTicketID.CDLInit();
				sTicketID.CDLGetNextInt( nTicketID );

				if(nTicketID > 0)
				{
					GetQuery().Initialize();
					BINDCOL_LONG_NOLEN( GetQuery(), m_nLinkCount );
					BINDPARAM_LONG( GetQuery(), nTicketID );
					GetQuery().Execute( _T("SELECT COUNT(*) FROM TicketLinksTicket WHERE TicketID = ?"));
					GetQuery().Fetch();
				}
			}			

			if (!GetISAPIData().GetURLLong( _T("TICKETLINK"), m_nTicketLink, true ))
			{
				GetXMLGen().AddChildElem( _T("srcTicket") );
			}
			else
			{
				GetXMLGen().AddChildElem( _T("linkTicket") );				
			}
			GetXMLGen().AddChildAttrib( _T("ID"), sTicketID.c_str() );
			GetXMLGen().AddChildAttrib( _T("ISMULTIPLE"), m_bMultipleTickets );
			GetXMLGen().AddChildAttrib( _T("LINKCOUNT"), m_nLinkCount );
		}
		else
		{
			// get parameters as either URL or FORM parameters
			if (!GetISAPIData().GetURLLong( _T("MsgID"), m_SourceMsg.MsgID, true ))
				GetISAPIData().GetFormLong( _T("MsgID"), m_SourceMsg.MsgID);
			
			if (!GetISAPIData().GetURLString( _T("MsgType"), sType, true ))
				GetISAPIData().GetFormString( _T("MsgType"), sType );
			
			m_SourceMsg.IsInbound = (sType.compare( _T("inbound" ) ) == 0);
					
			// get the source TicketID
			GetSourceTicketID();

			// don't allow the agent to move the message to a new ticket
			// if all of the messages would be moved
			FindChildMessages();

			if ( m_SourceTicket.GetMsgCount() == m_ChildMessageList.size() )
				GetXMLGen().AddChildElem( _T("DisableNew") );
		}			
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the TicketID of the source message	              
\*--------------------------------------------------------------------------*/
void CChangeTicket::GetSourceTicketID( void )
{
	// get the source ticket id
	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), m_SourceTicket.m_TicketID );
	BINDPARAM_LONG( GetQuery(), m_SourceMsg.MsgID );
	
	m_SourceMsg.IsInbound ? GetQuery().Execute( _T("SELECT TicketID FROM InboundMessages WHERE InboundMessageID=?") ) :
							GetQuery().Execute( _T("SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID=?") );

	if ( GetQuery().Fetch() != S_OK )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("The source message no longer exists") );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the TicketBox of the destination Ticket	              
\*--------------------------------------------------------------------------*/
int CChangeTicket::GetDestTicketBoxID( void )
{
	int nTicketBoxID = 0;

	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID );
	BINDPARAM_LONG( GetQuery(), m_nDestTicketID );
	
	GetQuery().Execute( _T("SELECT TicketBoxID FROM Tickets WHERE TicketID=?") );
	
	if ( GetQuery().Fetch() != S_OK )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("The destination ticket no longer exists") );

	return nTicketBoxID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Moves the message specified and it's children (replies)
||				to the destination ticket id	              
\*--------------------------------------------------------------------------*/
void CChangeTicket::MoveMessage( void )
{
	CEMSString sQuery;
	CEMSString sList;
	tstring sMsgIDs;

	// find the message we need to move
	FindChildMessages();

	sList.reserve(256);
	list<TicketMsg_t>::iterator iter;
	
	// move the inbound messages
	for ( iter = m_ChildMessageList.begin(); iter != m_ChildMessageList.end(); iter++ )
	{
		if ( iter->IsInbound )
		{
			CEMSString sFormat;
			sFormat.Format( _T("%d,"), iter->MsgID);
			
			sList += sFormat;
		
			THDelInboundMsg( GetQuery(), m_SourceTicket.m_TicketID, GetSession().m_AgentID, iter->MsgID, m_SourceTicket.m_TicketBoxID );
			THAddInboundMsg( GetQuery(), m_nDestTicketID, GetSession().m_AgentID, iter->MsgID, GetDestTicketBoxID() );
		}
	}

	sList.CDLInit();
	while( sList.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sMsgIDs ) )
	{
		GetQuery().Initialize();

		BINDPARAM_LONG( GetQuery(), m_nDestTicketID );	

		sQuery.Format( _T("UPDATE InboundMessages SET TicketID=? ")
			           _T("WHERE InboundMessageID IN (%s)"), sMsgIDs.c_str() );

		GetQuery().Execute( sQuery.c_str() );

	}

	// now move the outbound messages
	sList.erase();

	for ( iter = m_ChildMessageList.begin(); iter != m_ChildMessageList.end(); iter++ )
	{
		if ( !iter->IsInbound )
		{
			CEMSString sFormat;
			sFormat.Format( _T("%d,"), iter->MsgID);

			sList += sFormat;

			THDelOutboundMsg( GetQuery(), m_SourceTicket.m_TicketID, GetSession().m_AgentID, iter->MsgID, m_SourceTicket.m_TicketBoxID );
			THAddOutboundMsg( GetQuery(), m_nDestTicketID, GetSession().m_AgentID, iter->MsgID, GetDestTicketBoxID() );
		}
	}
	
	sList.CDLInit();
	while( sList.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sMsgIDs ) )
	{
		GetQuery().Initialize();
		
		BINDPARAM_LONG( GetQuery(), m_nDestTicketID );	
		
		sQuery.Format( _T("UPDATE OutboundMessages SET TicketID=? ")
			           _T("WHERE OutboundMessageID IN (%s)"), sMsgIDs.c_str() );

		GetQuery().Execute( sQuery.c_str() );

	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Moves the message specified and it's children (replies)
||				to the destination ticket id	              
\*--------------------------------------------------------------------------*/
void CChangeTicket::CopyMessage( void )
{
	CEMSString sDestTicketID;
	sDestTicketID.Format(_T("%d"), m_nDestTicketID);
	CAttachment attachment(m_ISAPIData);
	list<CAttachment> AttachmentList;
	list<CAttachment>::iterator iter;
	
	if(m_SourceMsg.IsInbound)
	{
		m_InboundMessage.m_InboundMessageID = m_SourceMsg.MsgID;
		m_InboundMessage.QueryMinimal();
		m_InboundMessage.m_TicketID = m_nDestTicketID;
		m_InboundMessage.Insert(GetQuery());
		attachment.ListInboundMessageAttachments( m_SourceMsg.MsgID, AttachmentList );
		for ( iter = AttachmentList.begin(); iter != AttachmentList.end(); iter++ )
		{
			GetQuery().Initialize();
			BINDPARAM_LONG(GetQuery(), m_InboundMessage.m_InboundMessageID);
			BINDPARAM_LONG(GetQuery(), iter->m_AttachmentID);
			GetQuery().Execute(	_T("INSERT INTO InboundMessageAttachments ")
							_T("(InboundMessageID, AttachmentID) ")
							_T("VALUES" )
							_T("(?,?)"));			
		}
		THAddInboundMsg( GetQuery(), m_nDestTicketID, GetSession().m_AgentID, m_InboundMessage.m_InboundMessageID, GetDestTicketBoxID() );
		if ( _ttoi( m_SourceTicket.sAal.c_str() ) == 1 && _ttoi( m_SourceTicket.sTma.c_str() ) == 1 )
		{
			m_SourceTicket.LogAgentAction(GetSession().m_AgentID,23,m_SourceMsg.MsgID,m_SourceTicket.m_TicketID,m_InboundMessage.m_InboundMessageID,_T("1"),sDestTicketID);
		}
	}
	else
	{
		m_OutboundMessage.m_OutboundMessageID = m_SourceMsg.MsgID;
		m_OutboundMessage.Query();
		m_OutboundMessage.m_TicketID = m_nDestTicketID;
		m_OutboundMessage.Insert(GetQuery());
		attachment.ListOutboundMessageAttachments( m_SourceMsg.MsgID, AttachmentList );
		for ( iter = AttachmentList.begin(); iter != AttachmentList.end(); iter++ )
		{
			GetQuery().Initialize();
			BINDPARAM_LONG(GetQuery(), m_OutboundMessage.m_OutboundMessageID);
			BINDPARAM_LONG(GetQuery(), iter->m_AttachmentID);
			GetQuery().Execute(	_T("INSERT INTO OutboundMessageAttachments ")
							_T("(OutboundMessageID, AttachmentID) ")
							_T("VALUES" )
							_T("(?,?)"));			
		}
		THAddOutboundMsg( GetQuery(), m_nDestTicketID, GetSession().m_AgentID, m_OutboundMessage.m_OutboundMessageID, GetDestTicketBoxID() );
		if ( _ttoi( m_SourceTicket.sAal.c_str() ) == 1 && _ttoi( m_SourceTicket.sTma.c_str() ) == 1 )
		{
			m_SourceTicket.LogAgentAction(GetSession().m_AgentID,23,m_SourceMsg.MsgID,m_SourceTicket.m_TicketID,m_OutboundMessage.m_OutboundMessageID,_T("0"),sDestTicketID);
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Return child messages (replies) of the source message	              
\*--------------------------------------------------------------------------*/
void CChangeTicket::FindChildMessages( void )
{
	TicketMsg_t TicketMsg;
	MsgID_t MapKey;

	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), TicketMsg.MsgID );
	BINDCOL_LONG_NOLEN( GetQuery(), TicketMsg.IsInbound );
	BINDCOL_LONG_NOLEN( GetQuery(), MapKey.MsgID );
	BINDCOL_BIT_NOLEN( GetQuery(), MapKey.IsInbound );

	BINDPARAM_LONG( GetQuery(), m_SourceTicket.m_TicketID );
	BINDPARAM_LONG( GetQuery(), m_SourceTicket.m_TicketID );
	
	GetQuery().Execute( 
		_T("SELECT InboundMessageID,1,ReplyToMsgID,ReplyToIDIsInbound ")
		_T("FROM InboundMessages WHERE TicketID = ? ")
		_T("UNION ")
		_T("SELECT OutboundMessageID,0,ReplyToMsgID,ReplyToIDIsInbound ")
		_T("FROM OutboundMessages WHERE TicketID = ? ") );
	
	// MultiMap based upon the messages ReplyToID.  This allows us to find all
	// messages which are in reply to a given message.
	CReplyToMap map;
	
	// insert the query results into the map
	while( GetQuery().Fetch() == S_OK )
	{		
		map.insert(pair<MsgID_t, TicketMsg_t>(MapKey, TicketMsg));
	}

	// add the first message to the list
	TicketMsg.MsgID = m_SourceMsg.MsgID;
	TicketMsg.IsInbound = m_SourceMsg.IsInbound;

	m_ChildMessageList.push_back( TicketMsg );

	AddChildMessages( m_SourceMsg.MsgID, m_SourceMsg.IsInbound, map );
}

void CChangeTicket::AddChildMessages( int nMsgID, bool bIsInbound, CReplyToMap& ReplyMap )
{	
	CReplyToMap::iterator start;
	CReplyToMap::iterator end;
	
	MsgID_t MapKey;
	MapKey.MsgID = nMsgID;
	MapKey.IsInbound = bIsInbound;

	// find all messages which are replies to nMsgID
	start = ReplyMap.lower_bound(MapKey);
	end   = ReplyMap.upper_bound(MapKey);
	
	// for each child message...
	while ((start != end) && (start != ReplyMap.end()))
	{
		// the message must not have been used already
		// and must be the opposite type to be a reply
		if (start->second.bUsed == false)
		{
			start->second.bUsed = true;
			m_ChildMessageList.push_back( start->second );
		
			// find replies to this message
			AddChildMessages( start->second.MsgID, start->second.IsInbound, ReplyMap );
		}
		
		start++;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the TicketBoxViewID to use when displaying the
||				destination ticket. The view from which the source ticket was
||				opened will be returned first, else a matching public ticketbox
||				view will be used, else a matching agent view will be used.
||				If the agent does not have any TicketBoxViews which can 
||				display the ticket the ticket will be displayed in the public 
||				ticket box without using a view.  	              
\*--------------------------------------------------------------------------*/
void CChangeTicket::GetTicketBoxView( int nTicketID, int nSrcViewID, bool bShowDest )
{
	if ( !bShowDest )
	{
		TTicketBoxViews tbv;
		tbv.m_TicketBoxViewID = nSrcViewID;
		tbv.Query( GetQuery() );

		GetXMLGen().AddChildElem( _T("TicketBoxView") );
		GetXMLGen().AddChildAttrib( _T("ID"), tbv.m_TicketBoxViewID );
		GetXMLGen().AddChildAttrib( _T("TYPE"), tbv.m_TicketBoxViewTypeID );
		return;
	}

	int nViewID = 0;
	int nTypeID = EMS_PUBLIC;
	int nTempViewID;
	int nTempTypeID;

	TTickets Ticket;	
	Ticket.m_TicketID = nTicketID;

	if ( Ticket.Query( GetQuery() ) == S_OK )
	{
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nTempViewID );
		BINDCOL_LONG_NOLEN( GetQuery(), nTempTypeID );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );

		// open and not owned
		if ( Ticket.m_OwnerID == 0 && Ticket.m_TicketStateID > EMS_TICKETSTATEID_CLOSED )
		{
			BINDPARAM_LONG( GetQuery(), Ticket.m_TicketBoxID );
			GetQuery().Execute( _T("SELECT TicketBoxViewID, TicketBoxViewTypeID FROM TicketBoxViews ")
				_T("WHERE AgentID=? AND TicketBoxID=?") );
		}
		// open and owned
		else if ( Ticket.m_OwnerID > 0 && Ticket.m_TicketStateID > 1 )
		{
			BINDPARAM_LONG( GetQuery(), Ticket.m_OwnerID );
			BINDPARAM_LONG( GetQuery(), Ticket.m_TicketBoxID );
			GetQuery().Execute( _T("SELECT TicketBoxViewID, TicketBoxViewTypeID FROM TicketBoxViews ")
				_T("WHERE AgentID=? AND AgentBoxID=? OR (TicketBoxID=? AND ShowOwnedItems=1)") );
		}
		// closed and not owned
		else if ( Ticket.m_OwnerID == 0 && Ticket.m_TicketStateID == EMS_TICKETSTATEID_CLOSED )
		{
			BINDPARAM_LONG( GetQuery(), Ticket.m_TicketBoxID );
			GetQuery().Execute( _T("SELECT TicketBoxViewID, TicketBoxViewTypeID FROM TicketBoxViews ")
				_T("WHERE AgentID=? AND TicketBoxID=? AND ShowClosedItems=1") );
		}
		// closed and owned
		else
		{
			BINDPARAM_LONG( GetQuery(), Ticket.m_OwnerID );
			BINDPARAM_LONG( GetQuery(), Ticket.m_TicketBoxID );
			GetQuery().Execute( _T("SELECT TicketBoxViewID, TicketBoxViewTypeID FROM TicketBoxViews ")
				_T("WHERE AgentID=? AND ShowClosedItems=1 AND AgentBoxID=? OR (TicketBoxID=? AND ShowOwnedItems=1)") );
		}

		while ( GetQuery().Fetch() == S_OK )
		{
			if ( nSrcViewID == nTempViewID )
			{
				nViewID = nTempViewID;
				nTypeID = nTempTypeID;
				break;
			}
			else if ( nTypeID == EMS_PUBLIC || !nViewID )
			{
				nViewID = nTempViewID;
				nTypeID = nTempTypeID;
			}
		}
	}

	if ( !nViewID )
		nViewID = 0 - Ticket.m_TicketBoxID;
		
	GetXMLGen().AddChildElem( _T("TicketBoxView") );
	GetXMLGen().AddChildAttrib( _T("ID"), nViewID );
	GetXMLGen().AddChildAttrib( _T("TYPE"), nTypeID );
}
