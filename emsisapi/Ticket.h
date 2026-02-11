/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Ticket.h,v 1.2.2.2 2006/02/16 16:09:51 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/
#pragma once

class CTicket : public CXMLDataClass, public TTickets  
{
public:
	CTicket(CISAPIData& ISAPIData, int nTicketID=0);
	virtual ~CTicket();

	int Run(CURLAction& action) { return -1;}
	
	void GenerateXML( void );
	void Query( void );
	int  Insert( void );
	int  ReserveID(void);
	void Delete( bool bCheckSecurity );
	void Commit( void );
	void Lock( bool bCheckSecurity = false );
	void UnLock( bool bForce = false );
	int AdminUnLock();
	
	void SetState(int nTicketStateID );
	void SetTicketCategory( int nTicketCategoryID );
	void Open( bool bLog = true);
	void Close(void);
	void Escalate(void);
	void PlaceOnHold(void);
	void PlaceOnWait(void);
	void SetTickler(void);
	void UnSetTickler(void);
	void SetFolderID(void);
	bool Merge(int nDestTicketID, bool bCheckSecurity = false);	

	void Reassign( int nAgentID );
	void ChangePriority( int nPriorityID );
	void ChangeTicketBox( int nTicketBoxID );
	
	int	 GetMsgCount( bool bCheckSecurity = false );
	void GetTicketBoxAndOwner( int& nTicketBoxID, int& nOwnerID );
	int  GetNoteCount(void);

	// security
	unsigned char RequireRead(bool bQuery = true);
	unsigned char RequireEdit(bool bQuery = true);
	unsigned char RequireDelete(bool bQuery = true);

	void AddTicketFieldTicket( long TicketID, long TicketFieldID, long TicketFieldTypeID, TCHAR* DataValue ); 
	void LogAgentAction( long nAgentID,long nActionID,long nId1,long nId2,long nId3,CEMSString sData1,CEMSString sData2 );

	tstring sAal;
	tstring sTd;
	tstring sTc;
	tstring sTm;
	tstring sTe;
	tstring sTr;
	tstring sTrs;
	tstring sTo;
	tstring sTcl;
	tstring sTma;
	tstring sTmd;
	tstring sTmrev;
	tstring sTmrel;
	tstring sTmret;
	tstring sTmcop;
	tstring sTmerg;
	tstring sTmsav;
	tstring sLc;
	tstring sLd;
	tstring sTl;
	tstring sTu;
	tstring sTcc;
	tstring sTcf;

	void DoNotUnlock(void) { m_bDoNotUnlock = true; }

private:
	void GetSubjectForException( CEMSException Exception, CEMSString& sSubject );
	void GetSubjectForTicket( CEMSString& sSubject );
	void CheckAgentAlerts( int nOwnerID, int nAgentID, TCHAR szSubject[TICKETS_SUBJECT_LENGTH], long szSubjectLen );
	
	int m_nLockCount;
	bool m_bDoNotUnlock;
	unsigned char m_Access;
	tstring sDenyOnline;
	tstring sDenyAway;
	tstring sDenyNotAvail;
	tstring sDenyOffline;
	tstring sDenyOoo;
	tstring sAllowAdmin;
	tstring sAllowAgent;
		
};
