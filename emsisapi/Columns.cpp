/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Columns.cpp,v 1.1 2007/09/22 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Columns
||              
\\*************************************************************************/

#include "stdafx.h"
#include "Columns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CColumns::CColumns(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CColumns::Run( CURLAction& action )
{
	if( GetISAPIData().m_sPage.compare( _T("editcolumns") ) == 0)
	{
		if( GetISAPIData().GetXMLPost() )
		{
			Update();
		}
		else
		{
			Query();
		}
	
		return 0;
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query all Columns	              
\*--------------------------------------------------------------------------*/
void CColumns::Query(void)
{
	GetISAPIData().GetURLLong( _T("TBVIEWID"), m_TBView.m_TicketBoxViewID  );
	
	if ( m_TBView.m_TicketBoxViewID < 1 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Unable to obtain TicketBoxView ID") );
	}
	
	m_TBView.Query( GetQuery() );

	if ( m_TBView.m_UseDefault == 1 )
	{
		//Get default column views
		GetQuery().Initialize();
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowState);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowPriority);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowNumNotes);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowTicketID);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowNumMsgs);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowSubject);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowContact);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowDate);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowCategory);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowOwner);
		BINDCOL_BIT_NOLEN(GetQuery(), m_TBView.m_ShowTicketBox);
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		GetQuery().Execute( _T("SELECT ShowState,ShowPriority,ShowNumNotes,ShowTicketID,ShowNumMsgs, ")
				   _T("ShowSubject,ShowContact,ShowDate,ShowCategory,ShowOwner,ShowTicketBox ")
		           _T("FROM TicketBoxViews ")
		           _T("WHERE AgentID=? AND TicketBoxViewTypeID=9"));

		if ( GetQuery().Fetch() != S_OK )
		{
			//Set the defaults
			m_TBView.m_ShowState = 1;
			m_TBView.m_ShowPriority = 1;
			m_TBView.m_ShowNumNotes = 1;
			m_TBView.m_ShowTicketID = 1;
			m_TBView.m_ShowNumMsgs = 1;
			m_TBView.m_ShowSubject = 1;
			m_TBView.m_ShowContact = 1;
			m_TBView.m_ShowDate = 1;
			m_TBView.m_ShowCategory = 1;
			m_TBView.m_ShowOwner = 1;
			m_TBView.m_ShowTicketBox = 1;

			//Add the view
			GetQuery().Reset( true );
			BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowState);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowPriority);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowNumNotes);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowTicketID);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowNumMsgs);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowSubject);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowContact);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowDate);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowCategory);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowOwner);
			BINDPARAM_BIT(GetQuery(), m_TBView.m_ShowTicketBox);
			GetQuery().Execute( _T("INSERT INTO TicketBoxViews (AgentID,TicketBoxViewTypeID,ShowState,ShowPriority, ")
								_T("ShowNumNotes,ShowTicketID,ShowNumMsgs,ShowSubject,ShowContact, ")
								_T("ShowDate,ShowCategory,ShowOwner,ShowTicketBox) ")
								_T("VALUES (?,9,?,?,?,?,?,?,?,?,?,?,?)") );	
		}
	}

	GetXMLGen().AddChildElem( _T("TicketBoxView") );
	GetXMLGen().AddChildAttrib( _T("TBVIEWID"), m_TBView.m_TicketBoxViewID );
	
	GetXMLGen().AddChildElem( _T("StandardColumns") );
	GetXMLGen().AddChildAttrib( _T("UseDefault"), m_TBView.m_UseDefault );
	GetXMLGen().IntoElem();

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowState") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Ticket State") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowState );

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowPriority") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Ticket Priority") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowPriority );

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowNumNotes") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Ticket Notes") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowNumNotes );

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowTicketID") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Ticket ID") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowTicketID );

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowNumMsgs") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Messages") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowNumMsgs );

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowSubject") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Subject") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowSubject );

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowContact") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Contact") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowContact );

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowDate") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Date") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowDate );

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowCategory") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Category") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowCategory );

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowOwner") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("Owner") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowOwner );

	GetXMLGen().AddChildElem( _T("Column") );
	GetXMLGen().AddChildAttrib( _T("ID"), _T("ShowTicketBox") );
	GetXMLGen().AddChildAttrib( _T("Name"), _T("TicketBox") );
	GetXMLGen().AddChildAttrib( _T("IsEnabled"), m_TBView.m_ShowTicketBox );
	
	GetXMLGen().OutOfElem();

	//Get any custom fields for this TicketBox
	GetXMLGen().AddChildElem( _T("CustomColumns") );
	GetXMLGen().IntoElem();

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tf.m_TicketFieldID );
	BINDCOL_TCHAR( GetQuery(), tf.m_Description );
	if ( m_TBView.m_ViewTypeID != EMS_INBOX )
	{
		BINDPARAM_LONG( GetQuery(), m_TBView.m_TicketBoxID );
		GetQuery().Execute( _T("SELECT TicketFields.TicketFieldID,TicketFields.Description FROM TicketFields ")
							_T("INNER JOIN TicketFieldsTicketBox ON TicketFields.TicketFieldID=TicketFieldsTicketBox.TicketFieldID ")
							_T("WHERE TicketFieldsTicketBox.TicketBoxID=? AND TicketFieldsTicketBox.IsViewed=1") );
	}
	else
	{
		GetQuery().Execute( _T("SELECT TicketFields.TicketFieldID,TicketFields.Description FROM TicketFields") );	
	}
	while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Column") );
		GetXMLGen().AddChildAttrib( _T("ID"), tf.m_TicketFieldID );
		GetXMLGen().AddChildAttrib( _T("Name"), tf.m_Description );		
	}
	
	GetXMLGen().OutOfElem();

	//Get any TicketFieldViews for this TicketBoxView
	GetXMLGen().AddChildElem( _T("TicketFieldViews") );
	GetXMLGen().IntoElem();

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tfv.m_TicketFieldViewID );
	BINDCOL_LONG( GetQuery(), tfv.m_TicketFieldID );
	BINDCOL_BIT( GetQuery(), tfv.m_ShowField );
	BINDPARAM_LONG( GetQuery(), m_TBView.m_TicketBoxViewID );
	GetQuery().Execute( _T("SELECT TicketFieldViewID,TicketFieldID,ShowField ")
						_T("FROM TicketFieldViews ")
						_T("WHERE TicketBoxViewID=?") );
	while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketFieldView") );
		GetXMLGen().AddChildAttrib( _T("TFVID"), tfv.m_TicketFieldViewID );
		GetXMLGen().AddChildAttrib( _T("TFID"), tfv.m_TicketFieldID );	
		GetXMLGen().AddChildAttrib( _T("ShowField"), tfv.m_ShowField );	
	}
	
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Create a new or update an existing TicketField              
\*--------------------------------------------------------------------------*/
void CColumns::Update(void)
{
	bool doUpdate;

	DecodeForm();

	GetQuery().Initialize( true );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowState );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowPriority );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowNumNotes );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowTicketID );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowNumMsgs );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowSubject );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowContact );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowDate );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowCategory );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowOwner );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_ShowTicketBox );
	BINDPARAM_BIT( GetQuery(),  m_TBView.m_UseDefault );
	BINDPARAM_LONG( GetQuery(), m_TBView.m_TicketBoxViewID );
	GetQuery().Execute( _T("UPDATE TicketBoxViews ")
						_T("SET ShowState=?,ShowPriority=?,ShowNumNotes=?,ShowTicketID=?,ShowNumMsgs=?,ShowSubject=?,ShowContact=?,ShowDate=?,ShowCategory=?,ShowOwner=?,ShowTicketBox=?,UseDefault=? ")
						_T("WHERE TicketBoxViewID=?") );		

	if( GetQuery().GetRowCount() == 0 )
	{
		// bail out if m_TicketBoxViewID was invalid
		THROW_EMS_EXCEPTION( E_InvalidID,  _T("Invalid TicketBoxView ID") ); 
	}
		
	//Update any custom fields
	for ( tfvIter = m_tfv.begin(); tfvIter != m_tfv.end(); tfvIter++)
	{
		doUpdate = false;
		tfv.m_TicketBoxViewID = tfvIter->m_TicketBoxViewID;
		tfv.m_TicketFieldID = tfvIter->m_TicketFieldID;
		tfv.m_ShowField = tfvIter->m_ShowField;
		
		GetQuery().Initialize( true );
		BINDCOL_LONG( GetQuery(), tfv.m_TicketFieldViewID );
		BINDPARAM_LONG( GetQuery(), tfv.m_TicketBoxViewID );
		BINDPARAM_LONG( GetQuery(), tfv.m_TicketFieldID );
		GetQuery().Execute( _T("SELECT TicketFieldViewID FROM TicketFieldViews ")
							_T("WHERE TicketBoxViewID=? AND TicketFieldID=?") );
		if( GetQuery().Fetch() == S_OK )
		{
			doUpdate = true;
		}
		
		if ( doUpdate )
		{
			//Update an existing record
			tfv.Update( GetQuery() );	
		}
		else
		{
			//Add a new record
			tfv.Insert( GetQuery() );
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Decode the form	              
\*--------------------------------------------------------------------------*/
void CColumns::DecodeForm(void)
{
	CEMSString fieldName;
	
	GetISAPIData().GetURLLong( _T("TBViewID"), m_TBView.m_TicketBoxViewID);
	GetISAPIData().GetXMLLong( _T("UseDefault"), m_TBView.m_UseDefault);
	GetISAPIData().GetXMLLong( _T("ShowState"), m_TBView.m_ShowState);
	GetISAPIData().GetXMLLong( _T("ShowPriority"), m_TBView.m_ShowPriority);
	GetISAPIData().GetXMLLong( _T("ShowNumNotes"), m_TBView.m_ShowNumNotes);
	GetISAPIData().GetXMLLong( _T("ShowTicketID"), m_TBView.m_ShowTicketID);
	GetISAPIData().GetXMLLong( _T("ShowNumMsgs"), m_TBView.m_ShowNumMsgs);
	GetISAPIData().GetXMLLong( _T("ShowSubject"), m_TBView.m_ShowSubject);
	GetISAPIData().GetXMLLong( _T("ShowContact"), m_TBView.m_ShowContact);
	GetISAPIData().GetXMLLong( _T("ShowDate"), m_TBView.m_ShowDate);
	GetISAPIData().GetXMLLong( _T("ShowCategory"), m_TBView.m_ShowCategory);

	if ( !GetISAPIData().GetXMLLong( _T("ShowOwner"), m_TBView.m_ShowOwner, true ) )
		m_TBView.m_ShowOwner = 0;

	if ( !GetISAPIData().GetXMLLong( _T("ShowTicketBox"), m_TBView.m_ShowTicketBox, true ) )
		m_TBView.m_ShowTicketBox = 0;

	//Get TicketBoxViewTypeID
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), m_TBView.m_ViewTypeID );
	BINDPARAM_LONG( GetQuery(), m_TBView.m_TicketBoxViewID );
	GetQuery().Execute( _T("SELECT TicketBoxViewTypeID FROM TicketBoxViews WHERE TicketBoxViewID=?") );
	GetQuery().Fetch();

	m_tfv.clear();
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), tf.m_TicketFieldID );
	BINDPARAM_LONG( GetQuery(), m_TBView.m_TicketBoxViewID );
	if ( m_TBView.m_ViewTypeID != EMS_INBOX )
	{
		GetQuery().Execute( _T("SELECT TicketFields.TicketFieldID FROM TicketFields ")
							_T("INNER JOIN TicketFieldsTicketBox ON TicketFields.TicketFieldID=TicketFieldsTicketBox.TicketFieldID ")
							_T("WHERE TicketFieldsTicketBox.TicketBoxID=(SELECT TicketBoxID FROM TicketBoxViews WHERE TicketBoxViewID=?) AND TicketFieldsTicketBox.IsViewed=1") );
	}
	else
	{
		GetQuery().Execute( _T("SELECT TicketFields.TicketFieldID,TicketFields.Description FROM TicketFields") );
	}
	while ( GetQuery().Fetch() == S_OK )
	{
		fieldName.Format( _T("%d"), tf.m_TicketFieldID );

		GetISAPIData().GetXMLLong( _T(fieldName.c_str()), tfv.m_ShowField, true);
		tfv.m_TicketBoxViewID = m_TBView.m_TicketBoxViewID;
		tfv.m_TicketFieldID = tf.m_TicketFieldID;
		m_tfv.push_back( tfv );
	}
}