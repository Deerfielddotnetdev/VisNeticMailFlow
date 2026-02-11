#include "stdafx.h"
#include ".\OfficeHours.h"

OfficeHours::OfficeHours(CISAPIData& ISAPIData)
	:CXMLDataClass(ISAPIData)
{
	nTypeID=0;
	nActualID=0;
	nUseDefault=1;
}

OfficeHours::~OfficeHours(void)
{
}

int OfficeHours::Run(CURLAction& action)
{
	// Check security
	RequireAdmin();

	if (!GetISAPIData().GetURLLong( _T("TypeID"), nTypeID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("TypeID"), nTypeID, true ))
		{
			GetISAPIData().GetFormLong( _T("TypeID"), nTypeID, true );				
		}
	}

	if (!GetISAPIData().GetURLLong( _T("ActualID"), nActualID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ActualID"), nActualID, true ))
		{
			GetISAPIData().GetFormLong( _T("ActualID"), nActualID, true );				
		}
	}

	if (!GetISAPIData().GetURLLong( _T("chkUseGlobalHours"), nUseDefault, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("chkUseGlobalHours"), nUseDefault, true ))
		{
			GetISAPIData().GetFormLong( _T("chkUseGlobalHours"), nUseDefault, true );				
		}
	}
	
	std::string sAction;
	if( !GetISAPIData().GetXMLString( _T("action"), sAction, true ))
	{
		GetISAPIData().GetXMLString("action", sAction, true);
	}
	
	if(!lstrcmpi(sAction.c_str(),"update"))
	{
		UpdateSettings();
	}
	
	return GetSettings(action);
}

int OfficeHours::GetSettings(CURLAction& action)
{
	// Delete old Office Hour Exceptions
	TIMESTAMP_STRUCT now;
	GetTimeStamp( now );
	GetQuery().Initialize();
	BINDPARAM_TIME_NOLEN( GetQuery(), now );
	GetQuery().Execute( _T("DELETE FROM OfficeHours ")
						_T("WHERE OfficeHourID>7 AND TypeID<>0 AND TimeEnd < ?") );
	
	
	bool bIsCustom = false;
	vector<TOfficeHours> too;
	vector<TOfficeHours>::iterator tooIter;

	TOfficeHours oh;
	oh.PrepareList( GetQuery() );
	while(GetQuery().Fetch() == S_OK)
	{
		if(oh.m_TypeID == nTypeID && oh.m_ActualID == nActualID)
		{
			bIsCustom = true;
		}
		too.push_back(oh);
	}

	GetXMLGen().AddChildElem( _T("OfficeHours") );
	for( tooIter = too.begin(); tooIter != too.end(); tooIter++ )
	{
		if((tooIter->m_TypeID == nTypeID && tooIter->m_ActualID == nActualID && bIsCustom) || (tooIter->m_TypeID == 0 && tooIter->m_ActualID == 0 && !bIsCustom))
		{
			switch ( tooIter->m_DayID )
			{
			case 1:
				GetXMLGen().AddChildAttrib( _T("SunHrStart"), tooIter->m_StartHr );
				GetXMLGen().AddChildAttrib( _T("SunMinStart"), tooIter->m_StartMin );
				GetXMLGen().AddChildAttrib( _T("SunAmPmStart"), tooIter->m_StartAmPm );
				GetXMLGen().AddChildAttrib( _T("SunHrEnd"), tooIter->m_EndHr );
				GetXMLGen().AddChildAttrib( _T("SunMinEnd"), tooIter->m_EndMin );
				GetXMLGen().AddChildAttrib( _T("SunAmPmEnd"), tooIter->m_EndAmPm );
				break;
			case 2:
				GetXMLGen().AddChildAttrib( _T("MonHrStart"), tooIter->m_StartHr );
				GetXMLGen().AddChildAttrib( _T("MonMinStart"), tooIter->m_StartMin );
				GetXMLGen().AddChildAttrib( _T("MonAmPmStart"), tooIter->m_StartAmPm );
				GetXMLGen().AddChildAttrib( _T("MonHrEnd"), tooIter->m_EndHr );
				GetXMLGen().AddChildAttrib( _T("MonMinEnd"), tooIter->m_EndMin );
				GetXMLGen().AddChildAttrib( _T("MonAmPmEnd"), tooIter->m_EndAmPm );
				break;
			case 3:
				GetXMLGen().AddChildAttrib( _T("TueHrStart"), tooIter->m_StartHr );
				GetXMLGen().AddChildAttrib( _T("TueMinStart"), tooIter->m_StartMin );
				GetXMLGen().AddChildAttrib( _T("TueAmPmStart"), tooIter->m_StartAmPm );
				GetXMLGen().AddChildAttrib( _T("TueHrEnd"), tooIter->m_EndHr );
				GetXMLGen().AddChildAttrib( _T("TueMinEnd"), tooIter->m_EndMin );
				GetXMLGen().AddChildAttrib( _T("TueAmPmEnd"), tooIter->m_EndAmPm );
				break;
			case 4:
				GetXMLGen().AddChildAttrib( _T("WedHrStart"), tooIter->m_StartHr );
				GetXMLGen().AddChildAttrib( _T("WedMinStart"), tooIter->m_StartMin );
				GetXMLGen().AddChildAttrib( _T("WedAmPmStart"), tooIter->m_StartAmPm );
				GetXMLGen().AddChildAttrib( _T("WedHrEnd"), tooIter->m_EndHr );
				GetXMLGen().AddChildAttrib( _T("WedMinEnd"), tooIter->m_EndMin );
				GetXMLGen().AddChildAttrib( _T("WedAmPmEnd"), tooIter->m_EndAmPm );
				break;
			case 5:
				GetXMLGen().AddChildAttrib( _T("ThuHrStart"), tooIter->m_StartHr );
				GetXMLGen().AddChildAttrib( _T("ThuMinStart"), tooIter->m_StartMin );
				GetXMLGen().AddChildAttrib( _T("ThuAmPmStart"), tooIter->m_StartAmPm );
				GetXMLGen().AddChildAttrib( _T("ThuHrEnd"), tooIter->m_EndHr );
				GetXMLGen().AddChildAttrib( _T("ThuMinEnd"), tooIter->m_EndMin );
				GetXMLGen().AddChildAttrib( _T("ThuAmPmEnd"), tooIter->m_EndAmPm );
				break;
			case 6:
				GetXMLGen().AddChildAttrib( _T("FriHrStart"), tooIter->m_StartHr );
				GetXMLGen().AddChildAttrib( _T("FriMinStart"), tooIter->m_StartMin );
				GetXMLGen().AddChildAttrib( _T("FriAmPmStart"), tooIter->m_StartAmPm );
				GetXMLGen().AddChildAttrib( _T("FriHrEnd"), tooIter->m_EndHr );
				GetXMLGen().AddChildAttrib( _T("FriMinEnd"), tooIter->m_EndMin );
				GetXMLGen().AddChildAttrib( _T("FriAmPmEnd"), tooIter->m_EndAmPm );
				break;
			case 7:
				GetXMLGen().AddChildAttrib( _T("SatHrStart"), tooIter->m_StartHr );
				GetXMLGen().AddChildAttrib( _T("SatMinStart"), tooIter->m_StartMin );
				GetXMLGen().AddChildAttrib( _T("SatAmPmStart"), tooIter->m_StartAmPm );
				GetXMLGen().AddChildAttrib( _T("SatHrEnd"), tooIter->m_EndHr );
				GetXMLGen().AddChildAttrib( _T("SatMinEnd"), tooIter->m_EndMin );
				GetXMLGen().AddChildAttrib( _T("SatAmPmEnd"), tooIter->m_EndAmPm );
				break;
			}
		}
	}
	GetXMLGen().AddChildAttrib( _T("UseGlobalHours"), bIsCustom?0:1 );
	return 0;
}

void OfficeHours::UpdateSettings(void)
{
	if(nTypeID > 0 && nActualID > 0)
	{
		if(nUseDefault == 1)
		{		
			//Delete any custom office hours for this object
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), nTypeID );
			BINDPARAM_LONG( GetQuery(), nActualID );
			GetQuery().Execute( _T("DELETE FROM OfficeHours ")
								_T("WHERE TypeID=? AND ActualID=?") );		
		}
		else
		{
			TOfficeHours oh;
		
			// Sunday
			GetISAPIData().GetXMLLong( _T("SunHrStart"), oh.m_StartHr );
			GetISAPIData().GetXMLLong( _T("SunMinStart"), oh.m_StartMin );
			GetISAPIData().GetXMLLong( _T("SunAmPmStart"), oh.m_StartAmPm );
			GetISAPIData().GetXMLLong( _T("SunHrEnd"), oh.m_EndHr );
			GetISAPIData().GetXMLLong( _T("SunMinEnd"), oh.m_EndMin );
			GetISAPIData().GetXMLLong( _T("SunAmPMEnd"), oh.m_EndAmPm );
			
			if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Sunday Office Hours End Time must be after Start Time!") );}

			// get the ID
			GetQuery().Initialize();
			BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
			BINDPARAM_LONG( GetQuery(), nTypeID );
			BINDPARAM_LONG( GetQuery(), nActualID );
			GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
								_T("WHERE TypeID=? AND ActualID=? AND DayID=1") );
			GetQuery().Fetch();

			if( oh.m_OfficeHourID > 0 )
			{
				// update
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=1;
				oh.Update( GetQuery() );
			}
			else
			{
				// insert
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=1;
				oh.Insert( GetQuery() );
			}

			oh.m_OfficeHourID = 0;

			// Monday
			GetISAPIData().GetXMLLong( _T("MonHrStart"), oh.m_StartHr );
			GetISAPIData().GetXMLLong( _T("MonMinStart"), oh.m_StartMin );
			GetISAPIData().GetXMLLong( _T("MonAmPmStart"), oh.m_StartAmPm );
			GetISAPIData().GetXMLLong( _T("MonHrEnd"), oh.m_EndHr );
			GetISAPIData().GetXMLLong( _T("MonMinEnd"), oh.m_EndMin );
			GetISAPIData().GetXMLLong( _T("MonAmPMEnd"), oh.m_EndAmPm );

			if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Monday Office Hours End Time must be after Start Time!") );}

			// get the ID
			GetQuery().Initialize();
			BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
			BINDPARAM_LONG( GetQuery(), nTypeID );
			BINDPARAM_LONG( GetQuery(), nActualID );
			GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
								_T("WHERE TypeID=? AND ActualID=? AND DayID=2") );
			GetQuery().Fetch();

			if( oh.m_OfficeHourID > 0 )
			{
				// update
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=2;
				oh.Update( GetQuery() );
			}
			else
			{
				// insert
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=2;
				oh.Insert( GetQuery() );
			}

			oh.m_OfficeHourID = 0;

			// Tuesday
			GetISAPIData().GetXMLLong( _T("TueHrStart"), oh.m_StartHr );
			GetISAPIData().GetXMLLong( _T("TueMinStart"), oh.m_StartMin );
			GetISAPIData().GetXMLLong( _T("TueAmPmStart"), oh.m_StartAmPm );
			GetISAPIData().GetXMLLong( _T("TueHrEnd"), oh.m_EndHr );
			GetISAPIData().GetXMLLong( _T("TueMinEnd"), oh.m_EndMin );
			GetISAPIData().GetXMLLong( _T("TueAmPMEnd"), oh.m_EndAmPm );

			if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Tuesday Office Hours End Time must be after Start Time!") );}

			// get the ID
			GetQuery().Initialize();
			BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
			BINDPARAM_LONG( GetQuery(), nTypeID );
			BINDPARAM_LONG( GetQuery(), nActualID );
			GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
								_T("WHERE TypeID=? AND ActualID=? AND DayID=3") );
			GetQuery().Fetch();

			if( oh.m_OfficeHourID > 0 )
			{
				// update
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=3;
				oh.Update( GetQuery() );
			}
			else
			{
				// insert
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=3;
				oh.Insert( GetQuery() );
			}

			oh.m_OfficeHourID = 0;

			// Wednesday
			GetISAPIData().GetXMLLong( _T("WedHrStart"), oh.m_StartHr );
			GetISAPIData().GetXMLLong( _T("WedMinStart"), oh.m_StartMin );
			GetISAPIData().GetXMLLong( _T("WedAmPmStart"), oh.m_StartAmPm );
			GetISAPIData().GetXMLLong( _T("WedHrEnd"), oh.m_EndHr );
			GetISAPIData().GetXMLLong( _T("WedMinEnd"), oh.m_EndMin );
			GetISAPIData().GetXMLLong( _T("WedAmPMEnd"), oh.m_EndAmPm );

			if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Wednesday Office Hours End Time must be after Start Time!") );}

			// get the ID
			GetQuery().Initialize();
			BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
			BINDPARAM_LONG( GetQuery(), nTypeID );
			BINDPARAM_LONG( GetQuery(), nActualID );
			GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
								_T("WHERE TypeID=? AND ActualID=? AND DayID=4") );
			GetQuery().Fetch();

			if( oh.m_OfficeHourID > 0 )
			{
				// update
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=4;
				oh.Update( GetQuery() );
			}
			else
			{
				// insert
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=4;
				oh.Insert( GetQuery() );
			}

			oh.m_OfficeHourID = 0;

			// Thursday
			GetISAPIData().GetXMLLong( _T("ThuHrStart"), oh.m_StartHr );
			GetISAPIData().GetXMLLong( _T("ThuMinStart"), oh.m_StartMin );
			GetISAPIData().GetXMLLong( _T("ThuAmPmStart"), oh.m_StartAmPm );
			GetISAPIData().GetXMLLong( _T("ThuHrEnd"), oh.m_EndHr );
			GetISAPIData().GetXMLLong( _T("ThuMinEnd"), oh.m_EndMin );
			GetISAPIData().GetXMLLong( _T("ThuAmPMEnd"), oh.m_EndAmPm );

			if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Thursday Office Hours End Time must be after Start Time!") );}

			// get the ID
			GetQuery().Initialize();
			BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
			BINDPARAM_LONG( GetQuery(), nTypeID );
			BINDPARAM_LONG( GetQuery(), nActualID );
			GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
								_T("WHERE TypeID=? AND ActualID=? AND DayID=5") );
			GetQuery().Fetch();

			if( oh.m_OfficeHourID > 0 )
			{
				// update
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=5;
				oh.Update( GetQuery() );
			}
			else
			{
				// insert
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=5;
				oh.Insert( GetQuery() );
			}

			oh.m_OfficeHourID = 0;

			// Friday
			GetISAPIData().GetXMLLong( _T("FriHrStart"), oh.m_StartHr );
			GetISAPIData().GetXMLLong( _T("FriMinStart"), oh.m_StartMin );
			GetISAPIData().GetXMLLong( _T("FriAmPmStart"), oh.m_StartAmPm );
			GetISAPIData().GetXMLLong( _T("FriHrEnd"), oh.m_EndHr );
			GetISAPIData().GetXMLLong( _T("FriMinEnd"), oh.m_EndMin );
			GetISAPIData().GetXMLLong( _T("FriAmPMEnd"), oh.m_EndAmPm );

			if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Friday Office Hours End Time must be after Start Time!") );}

			// get the ID
			GetQuery().Initialize();
			BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
			BINDPARAM_LONG( GetQuery(), nTypeID );
			BINDPARAM_LONG( GetQuery(), nActualID );
			GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
								_T("WHERE TypeID=? AND ActualID=? AND DayID=6") );
			GetQuery().Fetch();

			if( oh.m_OfficeHourID > 0 )
			{
				// update
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=6;
				oh.Update( GetQuery() );
			}
			else
			{
				// insert
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=6;
				oh.Insert( GetQuery() );
			}

			oh.m_OfficeHourID = 0;

			// Saturday
			GetISAPIData().GetXMLLong( _T("SatHrStart"), oh.m_StartHr );
			GetISAPIData().GetXMLLong( _T("SatMinStart"), oh.m_StartMin );
			GetISAPIData().GetXMLLong( _T("SatAmPmStart"), oh.m_StartAmPm );
			GetISAPIData().GetXMLLong( _T("SatHrEnd"), oh.m_EndHr );
			GetISAPIData().GetXMLLong( _T("SatMinEnd"), oh.m_EndMin );
			GetISAPIData().GetXMLLong( _T("SatAmPMEnd"), oh.m_EndAmPm );

			if(!IsTimeLater(oh)){THROW_EMS_EXCEPTION( E_InvalidParameters, _T("The Saturday Office Hours End Time must be after Start Time!") );}

			// get the ID
			GetQuery().Initialize();
			BINDCOL_LONG(GetQuery(), oh.m_OfficeHourID );
			BINDPARAM_LONG( GetQuery(), nTypeID );
			BINDPARAM_LONG( GetQuery(), nActualID );
			GetQuery().Execute( _T("SELECT OfficeHourID FROM OfficeHours ")
								_T("WHERE TypeID=? AND ActualID=? AND DayID=7") );
			GetQuery().Fetch();

			if( oh.m_OfficeHourID > 0 )
			{
				// update
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=7;
				oh.Update( GetQuery() );
			}
			else
			{
				// insert
				oh.m_TypeID=nTypeID;
				oh.m_ActualID=nActualID;
				oh.m_DayID=7;
				oh.Insert( GetQuery() );
			}
		}
	}	
}

bool OfficeHours::IsTimeLater(TOfficeHours oh)
{
	if(oh.m_StartAmPm==0 || oh.m_StartHr==0 || oh.m_StartMin==60 || oh.m_EndAmPm==0 || oh.m_EndHr==0 || oh.m_EndMin==60)
	{
		if(oh.m_StartAmPm!=0 || oh.m_StartHr!=0 || oh.m_StartMin!=60 || oh.m_EndAmPm!=0 || oh.m_EndHr!=0 || oh.m_EndMin!=60)
		{
			THROW_EMS_EXCEPTION( E_InvalidParameters, _T("Invalid Time in Office Hours!") );
		}
		else
		{
			return true;
		}
	}
		
	TIMESTAMP_STRUCT StartTime;
	TIMESTAMP_STRUCT EndTime;
	GetTimeStamp( StartTime );
	GetTimeStamp( EndTime );
	if(oh.m_StartHr == 12 && oh.m_StartAmPm == 1)
	{
		StartTime.hour = 0;
	}
	else if(oh.m_StartAmPm == 2 && oh.m_StartHr == 12)
	{
		StartTime.hour = 12;
	}
	else if (oh.m_StartAmPm == 2)
	{
		StartTime.hour = oh.m_StartHr + 12;
	}
	else
	{
		StartTime.hour = oh.m_StartHr;
	}
	StartTime.minute = oh.m_StartMin;
	StartTime.second = 0;
	StartTime.fraction = 0;
	if(oh.m_EndHr == 12 && oh.m_EndAmPm == 1)
	{
		EndTime.hour = 0;
	}
	else if(oh.m_EndAmPm == 2 && oh.m_EndHr == 12)
	{
		EndTime.hour = 12;
	}
	else if (oh.m_EndAmPm == 2)
	{
		EndTime.hour = oh.m_EndHr + 12;
	}
	else
	{
		EndTime.hour = oh.m_EndHr;
	}
	EndTime.minute = oh.m_EndMin;
	EndTime.second = 0;
	EndTime.fraction = 0;
	BOOL bIsFuture = OrderTimeStamps( StartTime , EndTime );	
	if(bIsFuture == 0)
	{
		return false;
	}
	
	return true;
}