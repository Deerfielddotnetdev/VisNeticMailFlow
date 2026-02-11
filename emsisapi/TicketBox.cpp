// TicketBox.cpp: implementation of the CTicketBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TicketBox.h"
#include "DateFns.h"
#include "TicketHistoryFns.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTicketBox::CTicketBox(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData), m_Ticket(ISAPIData)
{
	m_StdRespName[0] = _T('\0');
	m_bRefreshLeftPane = false;
	m_TicketBoxID = 0;
	m_assignDefaultValues = false;
	m_AutoReplyInTicket = 1;
	m_MultiMail = 0;
}

CTicketBox::~CTicketBox()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	              
\*--------------------------------------------------------------------------*/
int CTicketBox::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;
	
	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_TicketBoxID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_TicketBoxID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_TicketBoxID);
		}
	}

	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( GetISAPIData().m_sPage.compare( _T("editticketbox") ) == 0 )
	{
		if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();
			Update();
			return 0;
		}
		else if ( sAction.compare( _T("move") ) == 0 )
		{
			DISABLE_IN_DEMO();
			MoveTickets();
			return 0;
		}
		else if ( sAction.compare( _T("deleteAll") ) == 0 )
		{
			DISABLE_IN_DEMO();
			if( m_TicketBoxID == 0 )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	
			DeleteTickets();
			return 0;
		}
		else if ( sAction.compare( _T("modify") ) == 0 )
		{
			DISABLE_IN_DEMO();
			if( m_TicketBoxID == 0 )
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	
			ModifyTickets();
			return 0;
		}
		else if( sAction.compare( _T("insert") ) == 0 )
		{
			DISABLE_IN_DEMO();
			New();
			return 0;
		}

		if( m_TicketBoxID == 0 )
		{
			GenerateXML();
			
			// change title if ID is zero
			action.m_sPageTitle.assign( _T("New TicketBox") );
			
			return 0;
		}
		
		return Query();
	}
	
	if( sAction.compare( _T("delete") ) == 0)
	{
		DISABLE_IN_DEMO();

		if( m_TicketBoxID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
	
		Delete();
	}

	return ListAll();
}


////////////////////////////////////////////////////////////////////////////////
// 
// The default action - return all ticket boxes
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketBox::ListAll(void)
{
	CEMSString sDateTime;
	unsigned int nAssignToTicketBox;
	long nAssignToTicketBoxLen;
	unsigned int nTktCount;
	long nTktCountLen;
	map<int,int> TktBoxCnts;
	map<int,int>::iterator iter;

	// Build a map that contains the ticket counts by ticketbox id
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), nAssignToTicketBox );
	BINDCOL_LONG( GetQuery(), nTktCount );
	GetQuery().Execute( _T("SELECT TicketBoxID,COUNT(*) ")
						_T("FROM Tickets ")
						_T("WHERE IsDeleted=0 ")
						_T("GROUP BY TicketBoxID") );

	while( GetQuery().Fetch() == S_OK )
	{
		TktBoxCnts[nAssignToTicketBox] = nTktCount;
	}

	// Now build the list of ticket boxes
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), m_TicketBoxID );
	BINDCOL_TCHAR( GetQuery(), m_Name );
	BINDCOL_TCHAR( GetQuery(), m_Description );
	BINDCOL_LONG( GetQuery(), nAssignToTicketBox );
	GetQuery().Execute( _T("SELECT DISTINCT TicketBoxID,Name,Description,AssignToTicketBox ")
	                    _T("FROM TicketBoxes LEFT OUTER JOIN RoutingRules ")
						_T("ON TicketBoxes.TicketBoxID = RoutingRules.AssignToTicketBox ")
	                    _T("ORDER BY Name ") );

	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem(_T("TicketBox"));
		GetXMLGen().AddChildAttrib( _T("ID"), m_TicketBoxID );
		GetXMLGen().AddChildAttrib( _T("Name"), m_Name );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetXMLGen().AddChildAttrib( _T("InUse"), nAssignToTicketBoxLen == SQL_NULL_DATA ? 0: 1);
		iter = TktBoxCnts.find( m_TicketBoxID );
		nTktCount = ( iter == TktBoxCnts.end() ) ? 0 : iter->second;
		GetXMLGen().AddChildAttrib( _T("Count"), nTktCount );
	}

	GetXMLGen().AddChildElem(_T("RefreshLeftPane"));
	GetXMLGen().AddChildAttrib( _T("Value"), m_bRefreshLeftPane ? 1 : 0 );

	return 0;
}

int CTicketBox::Query(void)
{
	m_MaxPersonalDataID = 0;
	m_MaxAutoActionID = 0;

	// query the TicketBox
	if ( TTicketBoxes::Query(GetQuery()) == S_OK )
	{
		if( m_AutoReplyWithStdResponse != 0 )
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_AutoReplyWithStdResponse );
			BINDCOL_TCHAR( GetQuery(), m_StdRespName );
			GetQuery().Execute( _T("SELECT Subject ")
								_T("FROM StandardResponses ")
								_T("WHERE StandardResponseID=?") );
			GetQuery().Fetch();
		}
		
		// Get Custom Ticket Fields
		GetQuery().Initialize();

		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		BINDCOL_LONG( GetQuery(), tftb.m_TicketFieldID );
		BINDCOL_BIT( GetQuery(), tftb.m_IsRequired );
		BINDCOL_BIT( GetQuery(), tftb.m_IsViewed );
		BINDCOL_BIT( GetQuery(), tftb.m_SetDefault );
		BINDCOL_TCHAR( GetQuery(), tftb.m_DefaultValue );
		GetQuery().Execute( _T("SELECT TicketFieldID,IsRequired,IsViewed,SetDefault,DefaultValue ")
							_T("FROM TicketFieldsTicketBox ")
  							_T("WHERE TicketBoxID=?") );

		//Clear the TicketFields vector
		m_tftb.clear();

		while( GetQuery().Fetch() == S_OK )
		{
			m_tftb.push_back( tftb );
		}

		// Get AutoActions
		GetQuery().Initialize();

		BINDCOL_LONG( GetQuery(), aa.m_AutoActionID );
		BINDCOL_LONG( GetQuery(), aa.m_AutoActionVal );
		BINDCOL_LONG( GetQuery(), aa.m_AutoActionFreq );
		BINDCOL_LONG( GetQuery(), aa.m_AutoActionEventID );
		BINDCOL_LONG( GetQuery(), aa.m_AutoActionTypeID );
		BINDCOL_LONG( GetQuery(), aa.m_AutoActionTargetID );
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		GetQuery().Execute( _T("SELECT AutoActionID,AutoActionVal,AutoActionFreq,AutoActionEventID,AutoActionTypeID,AutoActionTargetID ")
							_T("FROM AutoActions ")
  							_T("WHERE TicketBoxID=?") );

		//Clear the AutoActions vector
		m_aa.clear();

		while( GetQuery().Fetch() == S_OK )
		{
			m_aa.push_back( aa );
			if( aa.m_AutoActionID >= m_MaxAutoActionID )
			{
				m_MaxAutoActionID = aa.m_AutoActionID + 1;
			}
		}
		
		// List personal data
		int nAgentID = -(m_TicketBoxID);
		TPersonalData personaldata;

		GetQuery().Initialize();
		BINDPARAM_LONG(GetQuery(), nAgentID);
		BINDCOL_LONG(GetQuery(), personaldata.m_PersonalDataID );
		BINDCOL_LONG(GetQuery(), personaldata.m_PersonalDataTypeID );
		BINDCOL_TCHAR(GetQuery(), personaldata.m_DataValue );
		BINDCOL_TCHAR(GetQuery(), personaldata.m_Note );

		GetQuery().Execute(	_T("SELECT PersonalDataID,PersonalDataTypeID,DataValue,Note ")
							_T("FROM PersonalData ")
							_T("WHERE AgentID = ? ") );
		
		while( GetQuery().Fetch() == S_OK )
		{
			if( personaldata.m_DataValue != m_DefaultEmailAddress )
			{
				m_Data.push_back( personaldata );
				if( personaldata.m_PersonalDataID >= m_MaxPersonalDataID )
				{
					m_MaxPersonalDataID = personaldata.m_PersonalDataID + 1;
				}
			}
		}	

		GenerateXML();
	}
	else
	{
		// bail out m_TicketBoxID was invalid
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}
	return 0;
}

int CTicketBox::Update()
{
	unsigned char OldEnableAutoReplySetting=0;
	TPersonalData pd;
	set<unsigned int> DBPDList;
	int PersonalDataID;
		
	DecodeForm();

	if( IsDuplicate() )
	{
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Name must be unique")  );
	}

	// These fields are disabled for the unassigned ticket box
	if( m_TicketBoxID == EMS_TICKETBOX_UNASSIGNED )
	{
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		BINDCOL_TCHAR( GetQuery(), m_Name );
		BINDCOL_TCHAR( GetQuery(), m_Description );
		GetQuery().Execute( _T("SELECT Name,Description ")
							_T("FROM TicketBoxes ")
							_T("WHERE TicketBoxID=?") );
		GetQuery().Fetch();
	}

	// If the age alerting enable checkbox isn't set, then the form fields won't post,
	// so we must pull the extra data from the database before doing the update.
	if( m_AgeAlertingEnabled == 0 )
	{
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		BINDCOL_LONG( GetQuery(), m_AgeAlertingThresholdMins );
		GetQuery().Execute( _T("SELECT AgeAlertingThresholdMins ")
							_T("FROM TicketBoxes ")
							_T("WHERE TicketBoxID=?") );
		GetQuery().Fetch();
	}

	// If the auto-reply enable checkbox isn't set, then the form fields won't post,
	// so we must pull the extra data from the database before doing the update.
	if( m_AutoReplyEnable == 0 )
	{
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		BINDCOL_LONG( GetQuery(), m_AutoReplyWithStdResponse );
		BINDCOL_LONG( GetQuery(), m_AutoReplyThreshHoldMins );
		BINDCOL_BIT( GetQuery(), m_AutoReplyQuoteMsg );
		BINDCOL_BIT( GetQuery(), m_AutoReplyCloseTicket );
		BINDCOL_BIT( GetQuery(), m_AutoReplyInTicket );
		GetQuery().Execute( _T("SELECT AutoReplyWithStdResponse,AutoReplyThreshHoldMins,")
							_T("AutoReplyQuoteMsg,AutoReplyCloseTicket,AutoReplyInTicket ")
							_T("FROM TicketBoxes ")
							_T("WHERE TicketBoxID=?") );
		GetQuery().Fetch();
	}

	// Get the current watermark status
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	BINDCOL_LONG( GetQuery(), m_WaterMarkStatus );
	BINDCOL_BIT_NOLEN( GetQuery(), OldEnableAutoReplySetting );
	GetQuery().Execute( _T("SELECT WaterMarkStatus,AutoReplyEnable ")
						_T("FROM TicketBoxes ")
						_T("WHERE TicketBoxID=?") );
	GetQuery().Fetch();

	// Check if AutoReplies has just been enabled
	if( OldEnableAutoReplySetting == 0 && m_AutoReplyEnable == 1 )
	{
		int nReplyToExistingTickets;

		if ( GetISAPIData().GetXMLLong( _T("ReplyToExistingTickets"), nReplyToExistingTickets, true ) )
		{	
			if(!nReplyToExistingTickets)
			{
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
				GetQuery().Execute( _T("UPDATE Tickets ")
									_T("SET AutoReplied=1 ")
									_T("WHERE TicketBoxID=?") );
			}
		}
	}

	if( TTicketBoxes::Update( GetQuery() ) == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 

	//Update Ticket Fields
	AddTicketFields();
	
	//Update AutoActions
	AddAutoActions();

	if ( m_assignDefaultValues )
	{
		AssignDefaultValues();
	}

	int nAgentID = -(m_TicketBoxID);

	GetQuery().Reset(true);
	BINDPARAM_LONG( GetQuery(), nAgentID );
	BINDCOL_LONG_NOLEN( GetQuery(), PersonalDataID );
	GetQuery().Execute( _T("SELECT PersonalDataID FROM PersonalData WHERE AgentID=?") );
	

	while( GetQuery().Fetch() == S_OK )
	{
		DBPDList.insert( PersonalDataID );		
	}
	
	list<TPersonalData>::iterator PDiter;
	
	for ( PDiter = m_Data.begin(); PDiter != m_Data.end(); PDiter++)
	{
		if( DBPDList.find( PDiter->m_PersonalDataID ) == DBPDList.end() )
		{
			PDiter->Insert(GetQuery());
			if( PDiter->m_PersonalDataID >= m_MaxPersonalDataID )
			{
				m_MaxPersonalDataID = PDiter->m_PersonalDataID + 1;
			}
		}
		else
		{
			PDiter->Update(GetQuery());
		}
	}

	set<unsigned int>::iterator DBPDiter;

	for( DBPDiter = DBPDList.begin(); DBPDiter != DBPDList.end(); DBPDiter++ )
	{
		bool bFound = false;
		for ( PDiter = m_Data.begin(); PDiter != m_Data.end(); PDiter++)
		{	
			if( *DBPDiter == PDiter->m_PersonalDataID )
			{
				bFound = true;
				break;
			}
		}
		if( bFound == false ) 
		{
			int nPDID = *DBPDiter;
			GetQuery().Reset(true);
			BINDPARAM_LONG( GetQuery(), nPDID );
			GetQuery().Execute( _T("DELETE FROM PersonalData WHERE PersonalDataID=?") );		
		}
	}	

	UpdateTicketBoxViews();

	InvalidateTicketBoxes();

	return 0;
}

int CTicketBox::Delete()
{
	int ObjectID;
	tstring sOpt;
	int nRoutingRuleID;

	if( m_TicketBoxID < 2 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	BINDCOL_LONG_NOLEN( GetQuery(), ObjectID );
	GetQuery().Execute( _T("SELECT ObjectID ")
		                _T("FROM TicketBoxes ")
						_T("WHERE TicketBoxID=?") );

	if( GetQuery().Fetch() != S_OK )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}

	GetISAPIData().GetXMLString( _T("opt"), sOpt, false );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	BINDCOL_LONG_NOLEN( GetQuery(), nRoutingRuleID );
	GetQuery().Execute( _T("SELECT RoutingRuleID ")
						_T("FROM RoutingRules ")
						_T("WHERE AssignToTicketBox=?") );

	if( GetQuery().Fetch() == S_OK )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, _T("TicketBox is in use by a Routing Rule")); 
	}
	
	if( sOpt.compare( _T("delete") ) == 0 )
	{
		// Delete the tickets
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		GetQuery().Execute( _T("UPDATE Tickets ")
							_T("SET IsDeleted=1, ")
							_T("TicketBoxID=1 ")
							_T("WHERE TicketBoxID=?") );
	}
	else
	{
		// Move the tickets to Unassigned
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		GetQuery().Execute( _T("UPDATE Tickets ")
							_T("SET TicketBoxID=1 ")
							_T("WHERE TicketBoxID=?") );
	}

	// Delete the ticket box
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("DELETE FROM TicketBoxes ")
						_T("WHERE TicketBoxID=?") );
	
	if( GetQuery().GetRowCount() == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}

	// Check if the logged-in agent has any views of this ticketbox
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	BINDPARAM_LONG( GetQuery(), GetSession().m_AgentID );
	GetQuery().Execute( _T("SELECT TicketBoxViewID FROM TicketBoxViews ")
		                _T("WHERE TicketBoxID=? AND AgentID=?") );
	if( GetQuery().Fetch() == S_OK )
	{
		m_bRefreshLeftPane = true;	// if so, refresh left-hand pane
	}

	// Delete any ticketboxowners
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("DELETE FROM TicketBoxTicketBoxOwner ")
						_T("WHERE TicketBoxID=?") );

	// Delete any ticketfields
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("DELETE FROM TicketFieldsTicketBox ")
						_T("WHERE TicketBoxID=?") );

	// Delete any autoactions
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("DELETE FROM AutoActions ")
						_T("WHERE TicketBoxID=?") );

	// Delete any ticketboxviews
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("DELETE FROM TicketBoxViews ")
						_T("WHERE TicketBoxID=?") );

	GetQuery().Reset(false);
	GetQuery().Execute( _T("DELETE FROM Alerts ")
						_T("WHERE TicketBoxID=?") );

	// Delete any access control records
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), ObjectID );
	GetQuery().Execute( _T("DELETE FROM AccessControl ")
						_T("WHERE ObjectID=?") );	

	// Delete the object
	GetQuery().Reset(false);
	GetQuery().Execute( _T("DELETE FROM Objects ")
						_T("WHERE ObjectID=?") );

	// Delete any additional email addresses
	int nAgentID = -(m_TicketBoxID);
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), nAgentID );
	GetQuery().Execute( _T("DELETE FROM PersonalData ")
						_T("WHERE AgentID=?") );
	
	InvalidateTicketBoxes( true );

	return 0;
}

int CTicketBox::New()
{
	int ObjectTypeID = EMS_OBJECT_TYPE_TICKET_BOX;
	TIMESTAMP_STRUCT now;

	DecodeForm();

	if( IsDuplicate() )
	{
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Name must be unique")  );
	}

	GetTimeStamp( now );

	// Create the object ID
	GetQuery().Initialize();
	BINDPARAM_LONG(GetQuery(), ObjectTypeID);
	GetQuery().Execute( _T("INSERT INTO Objects (ObjectTypeID) VALUES (?)") );
	m_ObjectID = GetQuery().GetLastInsertedID();

	TTicketBoxes::Insert( GetQuery() );

	// Update the object row with the actual ID
	GetQuery().Reset( true );

	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	BINDPARAM_TIME_NOLEN( GetQuery(), now );
	BINDPARAM_LONG( GetQuery(), m_ObjectID );

	GetQuery().Execute( _T("UPDATE Objects SET ")
						_T("ActualID = ?, ")
						_T("DateCreated = ? ")
						_T("WHERE ObjectID = ?") );
	//Add Ticket Fields
	AddTicketFields();
	
	//Update AutoActions
	AddAutoActions();

	if ( m_assignDefaultValues )
	{
		AssignDefaultValues();
	}

	list<TPersonalData>::iterator PDiter;
		
	for ( PDiter = m_Data.begin(); PDiter != m_Data.end(); PDiter++)
	{
		PDiter->Insert(GetQuery());
		if( PDiter->m_PersonalDataID >= m_MaxPersonalDataID )
		{
			m_MaxPersonalDataID = PDiter->m_PersonalDataID + 1;
		}
	}

	UpdateTicketBoxViews();

	InvalidateTicketBoxes();

	return 0;
}

void CTicketBox::AssignDefaultValues(void)
{
	//Get list of TicketFields configured for this TicketBox
	m_tftb.clear();
	TTicketFieldsTicketBox tftb;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tftb.m_TicketFieldID );
	BINDCOL_TCHAR( GetQuery(), tftb.m_DefaultValue );
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("SELECT TicketFieldID,DefaultValue ")
	               _T("FROM TicketFieldsTicketBox WHERE TicketBoxID=? AND SetDefault=1") );	              	
	while( GetQuery().Fetch() == S_OK )
	{
		m_tftb.push_back(tftb);
	}

	//Get the TicketID's for this TicketBox
	m_tid.clear();
	int nTicketID;
	long nTicketIDLen;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), nTicketID );
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("SELECT TicketID ")
	               _T("FROM Tickets WHERE TicketBoxID=? AND IsDeleted=0") );	              	
	while( GetQuery().Fetch() == S_OK )
	{
		m_tid.push_back(nTicketID);
	}

	for( mtidIter = m_tid.begin(); mtidIter != m_tid.end(); mtidIter++ )
	{
		nTicketID = *mtidIter;
		for( mtftbIter = m_tftb.begin(); mtftbIter != m_tftb.end(); mtftbIter++ )
		{
			AddTicketFieldTicket(nTicketID,mtftbIter->m_TicketFieldID,mtftbIter->m_DefaultValue);
		}
	}

}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds or updates a TicketField	              
\*--------------------------------------------------------------------------*/
void CTicketBox::AddTicketFieldTicket( long TicketID, long TicketFieldID, TCHAR* DataValue )
{
	long nTicketFieldsTicketID;
	long nTicketFieldsTicketIDLen;
	int numRows = 0;
	tstring sDataValue;
	sDataValue = ( _T("%s"),DataValue);

	try
	{
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), nTicketFieldsTicketID );
		BINDPARAM_LONG( GetQuery(), TicketID );
		BINDPARAM_LONG( GetQuery(), TicketFieldID );
		GetQuery().Execute( _T("SELECT TicketFieldsTicketID FROM TicketFieldsTicket ")
							_T("WHERE TicketID=? AND TicketFieldID=?") );

		
		numRows = GetQuery().GetRowCount();
		if( numRows == 0 )
		{
			if ( sDataValue.length() > 0 )
			{
				//Doesn't exist, let's add it
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), TicketID );
				BINDPARAM_LONG( GetQuery(), TicketFieldID );
				BINDPARAM_TCHAR( GetQuery(), DataValue );
				GetQuery().Execute( _T("INSERT INTO TicketFieldsTicket ")
									_T("(TicketID,TicketFieldID,DataValue) VALUES (?,?,?)") );
			}
		}
		else if ( numRows == 1 )
		{
			//Exists, let's update it
			GetQuery().Initialize();
			BINDPARAM_TCHAR( GetQuery(), DataValue );
			BINDPARAM_LONG( GetQuery(), TicketID );
			BINDPARAM_LONG( GetQuery(), TicketFieldID );
			GetQuery().Execute( _T("UPDATE TicketFieldsTicket ")
								_T("SET DataValue=? WHERE TicketID=? AND TicketFieldID=?") );
		}
		else
		{
			//Found more than one, let's delete them all and then add a new one
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), TicketID );
			BINDPARAM_LONG( GetQuery(), TicketFieldID );
			GetQuery().Execute( _T("DELETE FROM TicketFieldsTicket ")
								_T("WHERE TicketID=? AND TicketFieldID=?") );

			if ( sDataValue.length() > 0 )
			{
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), TicketID );
				BINDPARAM_LONG( GetQuery(), TicketFieldID );
				BINDPARAM_TCHAR( GetQuery(), DataValue );
				GetQuery().Execute( _T("INSERT INTO TicketFieldsTicket ")
									_T("(TicketID,TicketFieldID,DataValue) VALUES (?,?,?)") );
			}
		}
	}
	catch(...)
	{
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Unable to update Custom Ticket Field(s)") );
	}
	
}

void CTicketBox::AddTicketFields(void)
{
	//First delete any existing entries for this TicketBox
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("DELETE FROM TicketFieldsTicketBox WHERE TicketBoxID = ?") );
	
	for( mtftbIter = m_tftb.begin(); mtftbIter != m_tftb.end(); mtftbIter++ )
	{
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		BINDPARAM_LONG( GetQuery(), mtftbIter->m_TicketFieldID );
		BINDPARAM_BIT( GetQuery(), mtftbIter->m_IsRequired );
		BINDPARAM_BIT( GetQuery(), mtftbIter->m_IsViewed );
		BINDPARAM_BIT( GetQuery(), mtftbIter->m_SetDefault );
		BINDPARAM_TCHAR( GetQuery(), mtftbIter->m_DefaultValue );
		GetQuery().Execute( _T("INSERT INTO TicketFieldsTicketBox ")
					_T("(TicketBoxID,TicketFieldID,IsRequired,IsViewed,SetDefault,DefaultValue) ")
					_T("VALUES")
					_T("(?,?,?,?,?,?)") );		
	}
}

void CTicketBox::AddAutoActions(void)
{
	//First delete any existing entries for this TicketBox
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("DELETE FROM AutoActions WHERE TicketBoxID = ?") );
	
	for( aaIter = m_aa.begin(); aaIter != m_aa.end(); aaIter++ )
	{
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		BINDPARAM_LONG( GetQuery(), aaIter->m_AutoActionVal );
		BINDPARAM_LONG( GetQuery(), aaIter->m_AutoActionFreq );
		BINDPARAM_LONG( GetQuery(), aaIter->m_AutoActionEventID );
		BINDPARAM_LONG( GetQuery(), aaIter->m_AutoActionTypeID );
		BINDPARAM_LONG( GetQuery(), aaIter->m_AutoActionTargetID );		
		GetQuery().Execute( _T("INSERT INTO AutoActions ")
					_T("(TicketBoxID,AutoActionVal,AutoActionFreq,AutoActionEventID,AutoActionTypeID,AutoActionTargetID) ")
					_T("VALUES")
					_T("(?,?,?,?,?,?)") );		
	}
}

void CTicketBox::DecodeForm(void)
{
	TPersonalData pd;
	list<TPersonalData>::iterator iter;
	CEMSString sPData;
	CEMSString sField;
	CEMSString sCollection;
	CEMSString sParam;
	CEMSString sValue;
	CEMSString sAutoAction;
	int TFID;
	int AAID;
	int nUnreadMode=0;
	int nFromFormat=0;
	int nAssignDefaultValues=0;
	
	if( m_TicketBoxID != EMS_TICKETBOX_UNASSIGNED )
	{
		GetISAPIData().GetXMLTCHAR( _T("Name"), m_Name, 50 );
		GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, 255 );
	}

	GetISAPIData().GetXMLLong( _T("AgeAlertingEnabled"), m_AgeAlertingEnabled );
	GetISAPIData().GetXMLLong( _T("AgeAlertingThresholdMins"), m_AgeAlertingThresholdMins, true );
	GetISAPIData().GetXMLLong( _T("RequireGetOldest"), m_RequireGetOldest );
	GetISAPIData().GetXMLLong( _T("UnreadMode"), m_UnreadMode );
	GetISAPIData().GetXMLLong( _T("FromFormat"), m_FromFormat );
	GetISAPIData().GetXMLLong( _T("TicketLink"), m_TicketLink );
	GetISAPIData().GetXMLLong( _T("MultiMail"), m_MultiMail, true );
	GetISAPIData().GetXMLLong( _T("RequireTC"), m_RequireTC );
	GetISAPIData().GetXMLLong( _T("LowWaterMark"), m_LowWaterMark );
	GetISAPIData().GetXMLLong( _T("HighWaterMark"), m_HighWaterMark );
	GetISAPIData().GetXMLLong( _T("AutoReplyEnable"), m_AutoReplyEnable );
	GetISAPIData().GetXMLLong( _T("AutoReplyWithStdResponse"), m_AutoReplyWithStdResponse, true );
	GetISAPIData().GetXMLLong( _T("AutoReplyThreshHoldMins"), m_AutoReplyThreshHoldMins, true );
	GetISAPIData().GetXMLLong( _T("AutoReplyQuoteMsg"), m_AutoReplyQuoteMsg );
	GetISAPIData().GetXMLLong( _T("AutoReplyCloseTicket"), m_AutoReplyCloseTicket );
	GetISAPIData().GetXMLLong( _T("AutoReplyInTicket"), m_AutoReplyInTicket );
	GetISAPIData().GetXMLLong( _T("HeaderID"), m_HeaderID );
	GetISAPIData().GetXMLLong( _T("FooterID"), m_FooterID );
	GetISAPIData().GetXMLLong( _T("FooterLocation"), m_FooterLocation );	
	GetISAPIData().GetXMLLong( _T("OwnerID"), m_OwnerID );
	GetISAPIData().GetXMLTCHAR( _T("DefaultEmailAddress"), m_DefaultEmailAddress, 255 );
	GetISAPIData().GetXMLTCHAR( _T("DefaultEmailAddressName"), m_DefaultEmailAddressName, 50 );
	GetISAPIData().GetXMLLong( _T("AssignDefaultValues"), nAssignDefaultValues, true );
	GetISAPIData().GetXMLLong( _T("MessageDestinationID"), m_MessageDestinationID );
	GetISAPIData().GetXMLTCHAR( _T("ReplyToEmailAddress"), m_ReplyToEmailAddress, 255 );
	GetISAPIData().GetXMLTCHAR( _T("ReturnPathEmailAddress"), m_ReturnPathEmailAddress, 255 );
	
	if ( nAssignDefaultValues == 1 )
	{
		m_assignDefaultValues = true;
	}

	CEMSString sString(m_DefaultEmailAddress);
	
	if (sString.size() > 0 && !sString.ValidateEmailAddr())
	{
		CEMSString sError;
		sError.Format( _T("The email address specified [%s] is invalid"), sString.c_str() );
		THROW_VALIDATION_EXCEPTION( _T("DefaultEmailAddress"), sError );
	}

	sString.Format(_T("%s"),m_ReplyToEmailAddress);
	
	if (sString.size() > 0 && !sString.ValidateEmailAddr())
	{
		CEMSString sError;
		sError.Format( _T("The email address specified [%s] is invalid"), sString.c_str() );
		THROW_VALIDATION_EXCEPTION( _T("ReplyToEmailAddress"), sError );
	}
	
	sString.Format(_T("%s"),m_ReturnPathEmailAddress);
	
	if (sString.size() > 0 && !sString.ValidateEmailAddr())
	{
		CEMSString sError;
		sError.Format( _T("The email address specified [%s] is invalid"), sString.c_str() );
		THROW_VALIDATION_EXCEPTION( _T("ReturnPathEmailAddress"), sError );
	}
	// Ticket Fields
	m_tftb.clear();
	GetISAPIData().GetXMLString( _T("TFCollection"), sCollection, true );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( TFID ) )
	{
		tftb.m_TicketFieldID = TFID;
		tftb.m_TicketBoxID = m_TicketBoxID;
		
		sParam.Format( _T("IsRequired%d"), TFID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), tftb.m_IsRequired );
		sParam.Format( _T("IsViewed%d"), TFID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), tftb.m_IsViewed );
		sParam.Format( _T("SetDefault%d"), TFID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), tftb.m_SetDefault );

		sParam.Format( _T("DefaultValue%d"), TFID );
		GetISAPIData().GetXMLTCHAR( _T(sParam.c_str()), tftb.m_DefaultValue, 255 );

		m_tftb.push_back( tftb );
	}

	// Ticket Fields
	m_aa.clear();
	GetISAPIData().GetXMLLong( _T("AutoActionMaxID"), m_MaxAutoActionID, false );
	GetISAPIData().GetXMLString( _T("AACollection"), sAutoAction, true );
	sAutoAction.CDLInit();
	while( sAutoAction.CDLGetNextInt( AAID ) )
	{
		aa.m_AutoActionID = AAID;
		aa.m_TicketBoxID = m_TicketBoxID;
		
		sParam.Format( _T("AutoActionVal%d"), AAID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), aa.m_AutoActionVal );
		sParam.Format( _T("AutoActionFreq%d"), AAID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), aa.m_AutoActionFreq );
		sParam.Format( _T("AutoActionEvent%d"), AAID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), aa.m_AutoActionEventID );
		sParam.Format( _T("AutoActionType%d"), AAID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), aa.m_AutoActionTypeID );
		sParam.Format( _T("AutoActionTarget%d"), AAID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), aa.m_AutoActionTargetID );		

		m_aa.push_back( aa );
	}

	// Get personaldata items (like contact editor)
	m_Data.clear();
	GetISAPIData().GetXMLLong( _T("PersonalDataMaxID"), m_MaxPersonalDataID, false );
	GetISAPIData().GetXMLString( _T("PersonalDataIDCollection"), sPData, false );
	sPData.CDLInit();
	while( sPData.CDLGetNextInt( pd.m_PersonalDataID ) )
	{
		pd.m_AgentID = -(m_TicketBoxID);
		pd.m_ContactID = 0;
		
		sField.Format( _T("PersonalDataType%d"), pd.m_PersonalDataID );
		GetISAPIData().GetXMLLong( sField.c_str(), pd.m_PersonalDataTypeID );

		sField.Format( _T("PersonalData%d"), pd.m_PersonalDataID );
		GetISAPIData().GetXMLTCHAR( sField.c_str(), pd.m_DataValue, 256 );

		m_Data.push_back( pd );		
	}

	CEMSString sAgents;
	int AgentID;
	GetISAPIData().GetXMLString( _T("AgentIDCollection"), sAgents, false );
	sAgents.CDLInit();
	while( sAgents.CDLGetNextInt( AgentID ) )
	{
		m_IDs.push_back( AgentID );
	}
}

bool CTicketBox::IsDuplicate(void)
{
	int nTicketBoxID;

	if( m_TicketBoxID == 0 )
	{
		GetQuery().Initialize();
		BINDPARAM_TCHAR( GetQuery(), m_Name );
		BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID );
		GetQuery().Execute( _T("SELECT TicketBoxID ")
							_T("FROM TicketBoxes ")
							_T("WHERE Name=?") );
	}
	else
	{
		GetQuery().Initialize();
		BINDPARAM_TCHAR( GetQuery(), m_Name );
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID );
		GetQuery().Execute( _T("SELECT TicketBoxID ")
							_T("FROM TicketBoxes ")
							_T("WHERE Name=? AND TicketBoxID <> ?") );
	}

	return ( GetQuery().Fetch() == S_OK );
}

void CTicketBox::GenerateXML(void)
{
	CEMSString sStdRespName;
	list<TPersonalData>::iterator iter2;
		
	GetXMLGen().AddChildElem(_T("TicketBox"));
	GetXMLGen().AddChildAttrib( _T("ID"), m_TicketBoxID );
	GetXMLGen().AddChildAttrib( _T("Name"), m_Name );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	GetXMLGen().AddChildAttrib( _T("AgeAlertingEnabled"), m_AgeAlertingEnabled );
	GetXMLGen().AddChildAttrib( _T("AgeAlertingThresholdMins"), m_AgeAlertingThresholdMins );
	GetXMLGen().AddChildAttrib( _T("RequireGetOldest"), m_RequireGetOldest );
	GetXMLGen().AddChildAttrib( _T("UnreadMode"), m_UnreadMode );
	GetXMLGen().AddChildAttrib( _T("FromFormat"), m_FromFormat );	
	GetXMLGen().AddChildAttrib( _T("TicketLink"), m_TicketLink );	
	GetXMLGen().AddChildAttrib( _T("MultiMail"), m_MultiMail );	
	GetXMLGen().AddChildAttrib( _T("RequireTC"), m_RequireTC );	
	GetXMLGen().AddChildAttrib( _T("LowWaterMark"), m_LowWaterMark );
	GetXMLGen().AddChildAttrib( _T("HighWaterMark"), m_HighWaterMark );
	GetXMLGen().AddChildAttrib( _T("AutoReplyEnable"), m_AutoReplyEnable );
	GetXMLGen().AddChildAttrib( _T("AutoReplyWithStdResponse"), m_AutoReplyWithStdResponse );
	GetXMLGen().AddChildAttrib( _T("AutoReplyThreshHoldMins"), m_AutoReplyThreshHoldMins );
	GetXMLGen().AddChildAttrib( _T("AutoReplyQuoteMsg"), m_AutoReplyQuoteMsg );
	GetXMLGen().AddChildAttrib( _T("AutoReplyCloseTicket"), m_AutoReplyCloseTicket );
	GetXMLGen().AddChildAttrib( _T("AutoReplyInTicket"), m_AutoReplyInTicket );
	GetXMLGen().AddChildAttrib( _T("HeaderID"), m_HeaderID );
	GetXMLGen().AddChildAttrib( _T("FooterID"), m_FooterID );
	GetXMLGen().AddChildAttrib( _T("FooterLocation"), m_FooterLocation );
	GetXMLGen().AddChildAttrib( _T("OwnerID"), m_OwnerID );
	GetXMLGen().AddChildAttrib( _T("DefaultEmailAddress"), m_DefaultEmailAddress );
	GetXMLGen().AddChildAttrib( _T("DefaultEmailAddressName"), m_DefaultEmailAddressName );
	GetXMLGen().AddChildAttrib( _T("MaxPersonalDataID"), m_MaxPersonalDataID );
	GetXMLGen().AddChildAttrib( _T("MaxAutoActionID"), m_MaxAutoActionID );
	GetXMLGen().AddChildAttrib( _T("MessageDestinationID"), m_MessageDestinationID );
	GetXMLGen().AddChildAttrib( _T("ReplyToEmailAddress"), m_ReplyToEmailAddress );
	GetXMLGen().AddChildAttrib( _T("ReturnPathEmailAddress"), m_ReturnPathEmailAddress );
	
	GetXMLGen().IntoElem();
	for( iter2 = m_Data.begin(); iter2 != m_Data.end(); iter2++ )
	{
		GetXMLGen().AddChildElem( _T("PersonalData") );
		GetXMLGen().AddChildAttrib( _T("ID"), iter2->m_PersonalDataID );
		GetXMLGen().AddChildAttrib( _T("TypeID"), iter2->m_PersonalDataTypeID );
		GetXMLGen().AddChildAttrib( _T("Value"), iter2->m_DataValue );
		GetXMLGen().AddChildAttrib( _T("Note"), iter2->m_Note );
	}
	GetXMLGen().OutOfElem();

	sStdRespName.assign( m_StdRespName );
	sStdRespName.EscapeJavascript();
	GetXMLGen().AddChildAttrib( _T("AutoReplyStdResponseName"), sStdRespName.c_str() );

	//Add Custom Ticket Fields
	GetXMLGen().IntoElem();
	for( mtftbIter = m_tftb.begin(); mtftbIter != m_tftb.end(); mtftbIter++ )
	{
		GetXMLGen().AddChildElem(_T("TicketFields"));
		GetXMLGen().AddChildAttrib( _T("TFID"), mtftbIter->m_TicketFieldID );
		GetXMLGen().AddChildAttrib( _T("IsRequired"), mtftbIter->m_IsRequired );
		GetXMLGen().AddChildAttrib( _T("IsViewed"), mtftbIter->m_IsViewed );
		GetXMLGen().AddChildAttrib( _T("SetDefault"), mtftbIter->m_SetDefault );
		GetXMLGen().AddChildAttrib( _T("DefaultValue"), mtftbIter->m_DefaultValue );		
	}
	GetXMLGen().OutOfElem();

	//Add AutoActions
	GetXMLGen().IntoElem();
	for( aaIter = m_aa.begin(); aaIter != m_aa.end(); aaIter++ )
	{
		GetXMLGen().AddChildElem(_T("AutoActions"));
		GetXMLGen().AddChildAttrib( _T("ID"), aaIter->m_AutoActionID );
		GetXMLGen().AddChildAttrib( _T("Val"),aaIter->m_AutoActionVal );
		GetXMLGen().AddChildAttrib( _T("Freq"), aaIter->m_AutoActionFreq );
		GetXMLGen().AddChildAttrib( _T("EventID"), aaIter->m_AutoActionEventID );
		GetXMLGen().AddChildAttrib( _T("TypeID"), aaIter->m_AutoActionTypeID );		
		GetXMLGen().AddChildAttrib( _T("TargetID"), aaIter->m_AutoActionTargetID );
	}
	GetXMLGen().OutOfElem();

	ListTicketBoxHeaders();
	ListTicketBoxFooters();
	ListTicketBoxOwners();
	ListTicketBoxNames( GetXMLGen() );
	ListPersonalDataTypes();
	ListAutoActionTypes();
	ListAutoActionEvents();
	ListTicketStates();	

	TTicketFields tf;
	tf.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("TicketFields") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketField") );
		GetXMLGen().AddChildAttrib( _T("ID"), tf.m_TicketFieldID );
		GetXMLGen().AddChildAttrib( _T("TYPE"), tf.m_TicketFieldTypeID );
		GetXMLGen().AddChildAttrib( _T("Description"), tf.m_Description );
	}
	GetXMLGen().OutOfElem();

	TTicketFieldOptions tfo;
	tfo.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("TicketFieldOptions") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketFieldOption") );
		GetXMLGen().AddChildAttrib( _T("TFID"), tfo.m_TicketFieldID );
		GetXMLGen().AddChildAttrib( _T("TFOID"), tfo.m_TicketFieldOptionID );
		GetXMLGen().AddChildAttrib( _T("OptionValue"), tfo.m_OptionValue );
	}
	GetXMLGen().OutOfElem();

	GetXMLGen().SavePos();
	
	// message destinations
	ListMessageDestinations();

	GetXMLGen().RestorePos();
	GetXMLGen().FindChildElem( _T("MessageDestinations") );
	GetXMLGen().IntoElem();
	GetXMLGen().InsertChildElem( _T("MessageDestination") );
	GetXMLGen().AddChildAttrib( _T("ID"), 0 );
	GetXMLGen().AddChildAttrib( _T("Address"), _T("Select Message Destination") );

	GetXMLGen().OutOfElem();

	if( m_TicketBoxID == 0 )
	{
		//List All Agents
		ListAgentNames();
	}
	else
	{
		TAgents aList;
			
		//List Agents that have this TicketBox View
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		BINDCOL_TCHAR( GetQuery(), aList.m_Name );
		BINDCOL_LONG( GetQuery(), aList.m_AgentID );
		GetQuery().Execute( _T("SELECT a.Name,a.AgentID FROM Agents a ")
							_T("INNER JOIN TicketBoxViews tbv on a.AgentID=tbv.AgentID ")
							_T("WHERE tbv.TicketBoxID=?") );

		GetXMLGen().AddChildElem( _T("Group") );
		GetXMLGen().IntoElem();
		while( GetQuery().Fetch() == S_OK )
		{
			GetXMLGen().AddChildElem( _T("Agent") );
			GetXMLGen().AddChildAttrib( _T("ID"), aList.m_AgentID );
			GetXMLGen().AddChildAttrib( _T("Name"), aList.m_Name );
		}
		GetXMLGen().OutOfElem();

		//List Agents that have at least list rights to this TicketBox
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
		BINDCOL_TCHAR( GetQuery(), aList.m_Name );
		BINDCOL_LONG( GetQuery(), aList.m_AgentID );
		GetQuery().Execute( _T("SELECT a.Name,a.AgentID FROM Agents a ")
							_T("WHERE a.IsDeleted=0 AND a.IsEnabled=1 ")
							_T("AND a.AgentID NOT IN (SELECT a.AgentID FROM Agents a ")
							_T("INNER JOIN TicketBoxViews tbv on a.AgentID=tbv.AgentID ")
							_T("WHERE tbv.TicketBoxID=?)") );  

		GetXMLGen().AddChildElem( _T("AgentNames") );
		GetXMLGen().IntoElem();
		while( GetQuery().Fetch() == S_OK )
		{
			GetXMLGen().AddChildElem( _T("Agent") );
			GetXMLGen().AddChildAttrib( _T("ID"), aList.m_AgentID );
			GetXMLGen().AddChildAttrib( _T("Name"), aList.m_Name );
		}
		GetXMLGen().OutOfElem();
	}
}

void CTicketBox::MoveTickets(void)
{
	int nNewBoxID;
	GetISAPIData().GetXMLLong( _T("NewBox"), nNewBoxID );
	
	//Get the TicketID's for this TicketBox
	m_tid.clear();
	int nTicketID;
	long nTicketIDLen;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), nTicketID );
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("SELECT TicketID ")
	               _T("FROM Tickets WHERE TicketBoxID=?") );	              	
	while( GetQuery().Fetch() == S_OK )
	{
		m_tid.push_back(nTicketID);
	}

	for( mtidIter = m_tid.begin(); mtidIter != m_tid.end(); mtidIter++ )
	{
		m_Ticket.m_TicketID = *mtidIter;
		m_Ticket.ChangeTicketBox(nNewBoxID);		
	}
}

void CTicketBox::DeleteTickets(void)
{
	//Get the TicketID's for this TicketBox
	m_tid.clear();
	int nTicketID;
	long nTicketIDLen;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), nTicketID );
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	GetQuery().Execute( _T("SELECT TicketID ")
	               _T("FROM Tickets WHERE TicketBoxID=? AND IsDeleted=0") );	              	
	while( GetQuery().Fetch() == S_OK )
	{
		m_tid.push_back(nTicketID);
	}

	for( mtidIter = m_tid.begin(); mtidIter != m_tid.end(); mtidIter++ )
	{
		m_Ticket.m_TicketID = *mtidIter;
		m_Ticket.Delete(false);		
	}
}

void CTicketBox::ModifyTickets(void)
{
	int nOldStateID;
	int nNewStateID;

	GetISAPIData().GetXMLLong( _T("oldStateID"), nOldStateID );
	GetISAPIData().GetXMLLong( _T("newStateID"), nNewStateID );

	//Get the TicketID's for this TicketBox
	m_tid.clear();
	int nTicketID;
	long nTicketIDLen;
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), nTicketID );
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	if(nOldStateID==0)
	{
		GetQuery().Execute( _T("SELECT TicketID ")
	               _T("FROM Tickets WHERE TicketBoxID=? AND IsDeleted=0") );
	}
	else
	{
		BINDPARAM_LONG( GetQuery(), nOldStateID );
		GetQuery().Execute( _T("SELECT TicketID ")
	               _T("FROM Tickets WHERE TicketBoxID=? AND TicketStateID=? AND IsDeleted=0") );	
	}
		              	
	while( GetQuery().Fetch() == S_OK )
	{
		m_tid.push_back(nTicketID);
	}

	for( mtidIter = m_tid.begin(); mtidIter != m_tid.end(); mtidIter++ )
	{
		m_Ticket.m_TicketID = *mtidIter;
		
		switch( nNewStateID )
		{
		case EMS_TICKETSTATEID_CLOSED:
			m_Ticket.Close();
			break;
			
		case EMS_TICKETSTATEID_OPEN:
			m_Ticket.Open();
			break;
			
		case EMS_TICKETSTATEID_ESCALATED:
			m_Ticket.Escalate();
			break;
			
		case EMS_TICKETSTATEID_ONHOLD:
			m_Ticket.PlaceOnHold();
			break;
		}
	}	
}

void CTicketBox::UpdateTicketBoxViews(void)
{
	int AgentID;
	set<int> m_DBIDs;
	set<int>::iterator DBiter;
	list<int>::iterator iter;
	int nViewType = EMS_PUBLIC;
	int nSortField = EMS_COLUMN_DATE;

	//List Agents that have this TicketBox View
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
	BINDCOL_LONG_NOLEN( GetQuery(), AgentID );
	GetQuery().Execute( _T("SELECT a.AgentID FROM Agents a ")
						_T("INNER JOIN TicketBoxViews tbv on a.AgentID=tbv.AgentID ")
						_T("WHERE tbv.TicketBoxID=?") );
	
	while( GetQuery().Fetch() == S_OK )
	{
		m_DBIDs.insert( AgentID );
	}

	for( iter = m_IDs.begin(); iter != m_IDs.end(); iter++ )
	{
		if( m_DBIDs.find(*iter) == m_DBIDs.end() )
		{
			AgentID = *iter;
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), AgentID );
			BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
			BINDPARAM_LONG( GetQuery(), nViewType );
			BINDPARAM_LONG( GetQuery(), nSortField );
			GetQuery().Execute( _T("INSERT INTO TicketBoxViews ")
								_T("(AgentID,TicketBoxID,TicketBoxViewTypeID,SortField,SortAscending) ")
								_T("VALUES (?,?,?,?,1)") );

		}
	}

	for( DBiter = m_DBIDs.begin(); DBiter != m_DBIDs.end(); DBiter++ )
	{
		bool bFound = false;

		for( iter = m_IDs.begin(); iter != m_IDs.end(); iter++ )
		{
			if( *DBiter == *iter )
			{
				bFound = true;
				break;
			}
		}

		if( bFound == false )
		{
			AgentID = *DBiter;
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), AgentID );
			BINDPARAM_LONG( GetQuery(), m_TicketBoxID );
			GetQuery().Execute( _T("DELETE FROM TicketBoxViews ")
								_T("WHERE AgentID=? AND TicketBoxID=?") );

		}
	}
}

