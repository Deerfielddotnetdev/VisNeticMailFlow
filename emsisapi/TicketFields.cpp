/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketFields.h,v 1.1 2007/09/22 18:48:44 mikec Exp $
||
||  Copyright © 2007 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Class for managing Ticket Fields
||              
\\*************************************************************************/

#include "stdafx.h"
#include "TicketFields.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CTicketFields::CTicketFields(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CTicketFields::Run( CURLAction& action )
{
	// Check security
	RequireAdmin();

	if( GetISAPIData().m_sPage.compare( _T("editticketfield") ) == 0)
	{
		if( GetISAPIData().GetXMLPost() )
		{
			DISABLE_IN_DEMO();
			Update();
		}
		else
		{
			QueryOne(action);
		}
	
		return 0;
	}

	if ( GetISAPIData().GetXMLPost() )
	{
		DISABLE_IN_DEMO();
		Delete();
	}
	else
	{
		QueryAll();
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query all TicketFields	              
\*--------------------------------------------------------------------------*/
void CTicketFields::QueryAll(void)
{
	PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("TicketFields") );
	GetXMLGen().IntoElem();

	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketField") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_TicketFieldID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		switch(m_TicketFieldTypeID)
		{
		case 1:
			GetXMLGen().AddChildAttrib( _T("Type"), _T("Text") );
			break;
		case 2:
			GetXMLGen().AddChildAttrib( _T("Type"), _T("CheckBox") );
			break;
		case 3:
			GetXMLGen().AddChildAttrib( _T("Type"), _T("Select") );
			break;
		default:
			GetXMLGen().AddChildAttrib( _T("Type"), _T("Text") );
			break;
		}
	}

	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query a single TicketField
\*--------------------------------------------------------------------------*/
void CTicketFields::QueryOne( CURLAction& action )
{
	m_MaxOptionID = 0;
	int nNewID = 0;

	GetISAPIData().GetURLLong( _T("ID"), m_TicketFieldID  );

	if ( m_TicketFieldID )
	{
		Query(GetQuery());
	}
	else
	{
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nNewID );
		GetQuery().Execute( _T("SELECT IDENT_CURRENT('TicketFields')") );
		if( GetQuery().Fetch() == S_OK )
		{
			nNewID = nNewID + 1;
		}

		action.m_sPageTitle = _T("New Ticket Field");
	}

	GetXMLGen().AddChildElem( _T("TicketField") );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );	
	GetXMLGen().AddChildAttrib( _T("ID"), m_TicketFieldID );
    GetXMLGen().AddChildAttrib( _T("NEWID"), nNewID );
    GetXMLGen().AddChildAttrib( _T("Type"), m_TicketFieldTypeID );		

	if ( m_TicketFieldTypeID == 3 )
	{
		//Get list of Options for TicketField Type of Select
		
		int nMaxOptionID=0;
		GetQuery().Initialize();
		m_tfo.clear();

		BINDPARAM_LONG( GetQuery(), m_TicketFieldID );
		BINDCOL_LONG( GetQuery(), tfo.m_TicketFieldOptionID );
		BINDCOL_LONG( GetQuery(), tfo.m_OptionOrder );
		BINDCOL_TCHAR( GetQuery(), tfo.m_OptionValue );
		BINDCOL_LONG_NOLEN( GetQuery(), nMaxOptionID );		
		GetQuery().Execute( _T("SELECT TicketFieldOptionID,OptionOrder,OptionValue, ")
							_T("(SELECT MAX(TicketFieldOptionID) FROM TicketFieldOptions) ")
  							_T("FROM TicketFieldOptions ")
  							_T("WHERE TicketFieldID=? ORDER BY OptionOrder") );
		while( GetQuery().Fetch() == S_OK )
		{
			m_tfo.push_back( tfo );
			m_MaxOptionID = nMaxOptionID + 1;
		}
		
		GetXMLGen().AddChildAttrib( _T("MaxOptionID"), m_MaxOptionID );

		GetXMLGen().AddChildElem( _T("Options") );
		GetXMLGen().IntoElem();		
		for( mtfoIter = m_tfo.begin(); mtfoIter != m_tfo.end(); mtfoIter++ )
		{
			GetXMLGen().AddChildElem(_T("Option"));
			GetXMLGen().AddChildAttrib( _T("OptionID"), mtfoIter->m_TicketFieldOptionID );
			GetXMLGen().AddChildAttrib( _T("OptionValue"), mtfoIter->m_OptionValue );
			GetXMLGen().AddChildAttrib( _T("OptionOrder"), mtfoIter->m_OptionOrder );
		}
		GetXMLGen().OutOfElem();
	}
	
	//Get list of TicketFields configured for TicketBoxes
	GetXMLGen().AddChildElem( _T("TicketFieldTicketBoxes") );
	GetXMLGen().IntoElem();

	GetQuery().Initialize();

	BINDPARAM_LONG( GetQuery(), m_TicketFieldID );
	BINDCOL_LONG( GetQuery(), tftb.m_TicketBoxID );
	BINDCOL_BIT( GetQuery(), tftb.m_IsRequired );
	BINDCOL_BIT( GetQuery(), tftb.m_IsViewed );
	BINDCOL_BIT( GetQuery(), tftb.m_SetDefault );
	BINDCOL_TCHAR( GetQuery(), tftb.m_DefaultValue );
	GetQuery().Execute( _T("SELECT TicketBoxID,IsRequired,IsViewed,SetDefault,DefaultValue ")
						_T("FROM TicketFieldsTicketBox ")
  						_T("WHERE TicketFieldID=?") );
		
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem(_T("TicketFieldTicketBox"));
		GetXMLGen().AddChildAttrib( _T("TBID"), tftb.m_TicketBoxID );
		GetXMLGen().AddChildAttrib( _T("IsRequired"), tftb.m_IsRequired );
		GetXMLGen().AddChildAttrib( _T("IsViewed"), tftb.m_IsViewed );
		GetXMLGen().AddChildAttrib( _T("SetDefault"), tftb.m_SetDefault );
		GetXMLGen().AddChildAttrib( _T("DefaultValue"), tftb.m_DefaultValue );
	}

	GetXMLGen().OutOfElem();

	ListTicketBoxNames( GetXMLGen() );

}

/*---------------------------------------------------------------------------\                     
||  Comments:	Create a new or update an existing TicketField              
\*--------------------------------------------------------------------------*/
void CTicketFields::Update(void)
{
	CEMSString sCollection;
	CEMSString sParam;
	CEMSString sValue;
	int TBID;
	
	GetISAPIData().GetXMLLong( _T("ID"),m_TicketFieldID  );
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, TICKETFIELDS_DESCRIPTION_LENGTH );
	GetISAPIData().GetXMLLong( _T("TicketFieldTypeID"),m_TicketFieldTypeID  );
	
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketFieldID );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	GetQuery().Execute( _T("SELECT TicketFieldID FROM TicketFields ")
						_T("WHERE TicketFieldID<>? AND Description=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("That Ticket Field Name is already in use!")  );

	if ( m_TicketFieldID )
	{
		TTicketFields::Update( GetQuery() );
	}
	else
	{
		TTicketFields::Insert( GetQuery() );
	}

	if ( m_TicketFieldTypeID == 3 )
	{
		// Get any existing TicketFieldOptions for this TicketField
		GetQuery().Initialize();
		m_tfo.clear();

		BINDPARAM_LONG( GetQuery(), m_TicketFieldID );
		BINDCOL_LONG( GetQuery(), tfo.m_TicketFieldOptionID );
		BINDCOL_LONG( GetQuery(), tfo.m_OptionOrder );
		BINDCOL_TCHAR( GetQuery(), tfo.m_OptionValue );			
		GetQuery().Execute( _T("SELECT TicketFieldOptionID,OptionOrder,OptionValue ")
							_T("FROM TicketFieldOptions ")
  							_T("WHERE TicketFieldID=? ORDER BY OptionOrder") );
		while( GetQuery().Fetch() == S_OK )
		{
			m_tfo.push_back( tfo );			
		}

		// Get the collection
		vector<TTicketFieldOptions> vtfo;
		vector<TTicketFieldOptions>::iterator tfoIter;
		TTicketFieldOptions ttfo;
		int nTfoID;
		bool bFound = false;

		sCollection.Format(_T(""));
		GetISAPIData().GetXMLString( _T("OptCollection"), sCollection, true );
		sCollection.CDLInit();
		while( sCollection.CDLGetNextInt( nTfoID ) )
		{
			ttfo.m_TicketFieldOptionID = nTfoID;
			ttfo.m_TicketFieldID = m_TicketFieldID;
			
			sParam.Format( _T("OptionVal%d"), nTfoID );
			GetISAPIData().GetXMLTCHAR( _T(sParam.c_str()), ttfo.m_OptionValue, 50 );
			sParam.Format( _T("OptionOrder%d"), nTfoID );
			GetISAPIData().GetXMLLong( _T(sParam.c_str()), ttfo.m_OptionOrder );
			
			vtfo.push_back( ttfo );
		}
		
		// Add missing Options
		for( tfoIter = vtfo.begin(); tfoIter != vtfo.end(); tfoIter++ )
		{
			bFound = false;
			for( mtfoIter = m_tfo.begin(); mtfoIter != m_tfo.end(); mtfoIter++ )
			{
				if ( mtfoIter->m_TicketFieldOptionID == tfoIter->m_TicketFieldOptionID )
				{
					bFound = true;
					break;
				}				
			}
			
			if(bFound)
			{
				// This option already exists, let's update it
				tfo.m_TicketFieldOptionID = tfoIter->m_TicketFieldOptionID;
				tfo.m_OptionOrder = tfoIter->m_OptionOrder;
				strncpy(tfo.m_OptionValue, tfoIter->m_OptionValue, TICKETFIELDS_DESCRIPTION_LENGTH);				
				tfo.m_TicketFieldID = m_TicketFieldID;
				tfo.Update( GetQuery() );
			}
			else
			{
				// This option does not exist, let's add it
				tfo.m_OptionOrder = tfoIter->m_OptionOrder;
				strncpy(tfo.m_OptionValue, tfoIter->m_OptionValue, TICKETFIELDS_DESCRIPTION_LENGTH);
				tfo.m_TicketFieldID = m_TicketFieldID;
				tfo.Insert( GetQuery() );
			}
		}
		
		// Delete extra Options
		for( mtfoIter = m_tfo.begin(); mtfoIter != m_tfo.end(); mtfoIter++ )
		{
			bFound = false;
			for( tfoIter = vtfo.begin(); tfoIter != vtfo.end(); tfoIter++ )
			{
				if ( mtfoIter->m_TicketFieldOptionID == tfoIter->m_TicketFieldOptionID )
				{
					bFound = true;
					break;
				}
			}
			if ( !bFound )
			{
				// It's an extra, let's delete it
				tfo.m_TicketFieldOptionID = mtfoIter->m_TicketFieldOptionID;
				tfo.Delete( GetQuery() );

				// Remove TicketFieldTicket Entries that were set to this value
				GetQuery().Initialize();
				BINDPARAM_LONG( GetQuery(), m_TicketFieldID );
				BINDPARAM_LONG( GetQuery(), mtfoIter->m_TicketFieldOptionID );			
				GetQuery().Execute( _T("DELETE ")
									_T("FROM TicketFieldsTicket ")
  									_T("WHERE TicketFieldID=? AND DataValue=?") );
			}
		}
	}
	else
	{
		// Remove any existing TicketFieldOptions for this TicketField
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), m_TicketFieldID );
		GetQuery().Execute( _T("DELETE FROM TicketFieldOptions WHERE TicketFieldID = ?") );
	}

	m_tftb.clear();
	GetISAPIData().GetXMLString( _T("TFCollection"), sCollection, true );
	sCollection.CDLInit();
	while( sCollection.CDLGetNextInt( TBID ) )
	{
		tftb.m_TicketFieldID = m_TicketFieldID;
		tftb.m_TicketBoxID = TBID;
		
		sParam.Format( _T("IsRequired%d"), TBID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), tftb.m_IsRequired );
		sParam.Format( _T("IsViewed%d"), TBID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), tftb.m_IsViewed );
		sParam.Format( _T("SetDefault%d"), TBID );
		GetISAPIData().GetXMLLong( _T(sParam.c_str()), tftb.m_SetDefault );

		sParam.Format( _T("DefaultValue%d"), TBID );
		GetISAPIData().GetXMLTCHAR( _T(sParam.c_str()), tftb.m_DefaultValue, 255 );

		m_tftb.push_back( tftb );
	}

	AddTicketFields();
}

void CTicketFields::AddTicketFields(void)
{
	//First delete any existing entries for this Ticket Field
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_TicketFieldID );
	GetQuery().Execute( _T("DELETE FROM TicketFieldsTicketBox WHERE TicketFieldID = ?") );
	
	for( mtftbIter = m_tftb.begin(); mtftbIter != m_tftb.end(); mtftbIter++ )
	{
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), mtftbIter->m_TicketBoxID );
		BINDPARAM_LONG( GetQuery(), m_TicketFieldID );
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

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes one or more TicketFields	              
\*--------------------------------------------------------------------------*/
void CTicketFields::Delete(void)
{
	CEMSString sIDs;
	TTicketFields tf;

	GetISAPIData().GetXMLString( _T("SELECTID"), sIDs  );
	
	while ( sIDs.CDLGetNextInt( tf.m_TicketFieldID ) )
	{
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), tf.m_TicketFieldID );
		GetQuery().Execute( _T("DELETE FROM TicketFieldsTicket WHERE TicketFieldID=?") );

		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM TicketFieldsTicketBox WHERE TicketFieldID=?") );

		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM TicketFieldViews WHERE TicketFieldID=?") );

		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM TicketFieldOptions WHERE TicketFieldID=?") );

		GetQuery().Reset(false);
		GetQuery().Execute( _T("DELETE FROM TicketFields WHERE TicketFieldID=?") );

	}
}
