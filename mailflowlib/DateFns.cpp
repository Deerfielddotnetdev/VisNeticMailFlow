/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/DateFns.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||                                         
||  COMMENTS:   Date / Time Helper Functions
||              
\\*************************************************************************/

#include "stdafx.h"
#include <time.h>
#include "DateFns.h"

void VarDateToTimeStamp( DATE& date, TIMESTAMP_STRUCT& timestamp )
{
	SYSTEMTIME systime;
	VariantTimeToSystemTime( date, &systime );	

	timestamp.year = systime.wYear;
    timestamp.month = systime.wMonth;
    timestamp.day = systime.wDay;
    timestamp.hour = systime.wHour;
    timestamp.minute = systime.wMinute;
    timestamp.second = systime.wSecond;
    timestamp.fraction = 0;
}

void TimeStampToVarDate( TIMESTAMP_STRUCT& timestamp, DATE& date )
{
	SYSTEMTIME systime;

	systime.wYear = timestamp.year;
    systime.wMonth = timestamp.month;
    systime.wDay = timestamp.day;
    systime.wHour = timestamp.hour;
    systime.wMinute = timestamp.minute;
    systime.wSecond = timestamp.second;

	SystemTimeToVariantTime( &systime, &date );	
}

void GetTimeStamp( TIMESTAMP_STRUCT& timestamp )
{
	time_t now;
	time( &now );                /* Get time as long integer. */
	SecondsToTimeStamp( now, timestamp );
}

void SecondsToTimeStamp( time_t seconds, TIMESTAMP_STRUCT& timestamp )
{
	struct tm *ltime = localtime( &seconds );

	timestamp.year = ltime->tm_year + 1900;
    timestamp.month = ltime->tm_mon + 1;
    timestamp.day = ltime->tm_mday;
    timestamp.hour = ltime->tm_hour;
    timestamp.minute = ltime->tm_min;
    timestamp.second = ltime->tm_sec;
    timestamp.fraction = 0;
}

void TimeStampToSystemTime(const TIMESTAMP_STRUCT& timestamp, SYSTEMTIME &sysTime)
{	
	// note: these values are not converted!
	sysTime.wMilliseconds = 0;
	sysTime.wDayOfWeek = 0;
	
	sysTime.wDay = timestamp.day;
	sysTime.wHour = timestamp.hour;
	sysTime.wMinute = timestamp.minute;
	sysTime.wMonth = timestamp.month;
	sysTime.wSecond = timestamp.second;
	sysTime.wYear = timestamp.year;
}

void SystemTimeToTimeStamp(const SYSTEMTIME &sysTime, TIMESTAMP_STRUCT& timestamp)
{
	timestamp.day = sysTime.wDay;
	timestamp.hour = sysTime.wHour;
	timestamp.minute = sysTime.wMinute;
	timestamp.month = sysTime.wMonth;
	timestamp.second = sysTime.wSecond;
	timestamp.year = sysTime.wYear;
	timestamp.fraction = 0;
}

BOOL GetDateTimeString(const SYSTEMTIME &sysTime, CEMSString& sDateTime, bool bDate /*=true*/, bool bTime /*=true*/ )
{
	size_t nTCHARs;

	if ( bDate )
	{
		// get the size of the date string
		if ((nTCHARs = GetDateFormat( LOCALE_SYSTEM_DEFAULT, 0, &sysTime, NULL, NULL, 0 )) == 0)
		{
			return FALSE;
		}

		// make room in the tstring
		if (sDateTime.size() < nTCHARs)
			sDateTime.resize(nTCHARs);
		
		// get the date string
		if ((nTCHARs = GetDateFormat( LOCALE_SYSTEM_DEFAULT, 0, &sysTime, NULL, 
			(TCHAR*) sDateTime.data(), nTCHARs )) == 0)
		{
			return FALSE;
		}
	}

	if ( bTime )
	{
		int nSize = 0;

		if ( bDate )
		{
			// save the size of the tstring
			nSize = nTCHARs;

			// write a space over the terminating 0
			sDateTime[nSize - 1] = _T(' ');
		}
	
		// get the size of the time string
		if ((nTCHARs = GetTimeFormat( LOCALE_SYSTEM_DEFAULT, TIME_NOSECONDS, &sysTime, NULL, NULL, 0 )) == 0)
		{
			return FALSE;
		}
		
		// make room in the tstring
		if (sDateTime.size() < (nSize + nTCHARs))
			sDateTime.resize(nSize + nTCHARs);

		// get the time string
		if ((nTCHARs = GetTimeFormat( LOCALE_SYSTEM_DEFAULT, TIME_NOSECONDS, &sysTime, NULL, 
									 (TCHAR*) (sDateTime.data() + nSize), nTCHARs )) == 0)
		{
			return FALSE;
		}
	}

	// get rid of the terminating null that was
	// placed at the end of the string
	sDateTime.erase( sDateTime.length() -1, 1 );

	return TRUE;
}

BOOL GetDateTimeString(const TIMESTAMP_STRUCT& TimeStamp, int TimeStampLen, CEMSString& sDateTime, 
					   bool bDate /*true*/, bool bTime /*=true*/ )
{
	if (TimeStampLen == SQL_NULL_DATA)
	{
		sDateTime.LoadString(EMS_STRING_NULL_DATE);
		return TRUE;
	}
	
	SYSTEMTIME sysTime;
	TimeStampToSystemTime(TimeStamp, sysTime);
	
	return GetDateTimeString(sysTime, sDateTime, bDate, bTime);
}

BOOL GetFullDateTimeString(const SYSTEMTIME &sysTime, CEMSString& sDateTime )
{
	size_t nTCHARs;

	// get the size of the date string
	CEMSString sDateFormat;
	sDateFormat.Format(_T("MM'/'dd'/'yyyy"));
	
	if ((nTCHARs = GetDateFormat( LOCALE_SYSTEM_DEFAULT, 0, &sysTime,(TCHAR*) sDateFormat.c_str(), NULL, 0 )) == 0)
	{
		return FALSE;
	}

	// make room in the tstring
	if (sDateTime.size() < nTCHARs)
		sDateTime.resize(nTCHARs);
	
	// get the date string
	GetDateFormat( LOCALE_SYSTEM_DEFAULT, 0, &sysTime, (TCHAR*) sDateFormat.c_str() , (TCHAR*) sDateTime.data(), nTCHARs );
	int nSize = 0;

	
	// save the size of the tstring
	nSize = nTCHARs;

	// write a space over the terminating 0
	sDateTime[nSize - 1] = _T(' ');
	
	// get the size of the time string
	CEMSString sTimeFormat;
	sTimeFormat.Format(_T("hh':'mm':'ss tt"));
	if ((nTCHARs = GetTimeFormat( LOCALE_SYSTEM_DEFAULT, 0, &sysTime, (TCHAR*) sTimeFormat.c_str(), NULL, 0 )) == 0)
	{
		return FALSE;
	}
	// make room in the tstring
	if (sDateTime.size() < (nSize + nTCHARs))
		sDateTime.resize(nSize + nTCHARs);

	// get the time string
	GetTimeFormat( LOCALE_SYSTEM_DEFAULT, 0, &sysTime, (TCHAR*) sTimeFormat.c_str(), (TCHAR*) (sDateTime.data() + nSize), nTCHARs );
	
	

	// get rid of the terminating null that was
	// placed at the end of the string
	sDateTime.erase( sDateTime.length() -1, 1 );

	return TRUE;
}

BOOL GetFullDateTimeString(const TIMESTAMP_STRUCT& TimeStamp, int TimeStampLen, CEMSString& sDateTime)
{
	if (TimeStampLen == SQL_NULL_DATA)
	{
		sDateTime.LoadString(EMS_STRING_NULL_DATE);
		return TRUE;
	}
	
	SYSTEMTIME sysTime;
	TimeStampToSystemTime(TimeStamp, sysTime);
	
	return GetFullDateTimeString(sysTime, sDateTime);
}

////////////////////////////////////////////////////////////////////////////////
// 
// CompareTimeStamps returns the difference between timestamp Y and timestamp X
// in seconds.
// 
////////////////////////////////////////////////////////////////////////////////
int CompareTimeStamps( const TIMESTAMP_STRUCT& X, const TIMESTAMP_STRUCT& Y)
{
	SYSTEMTIME stx,sty;
	__int64 ix,iy;

	TimeStampToSystemTime( X, stx );
	SystemTimeToFileTime( &stx, (FILETIME*)&ix );

	TimeStampToSystemTime( Y, sty );
	SystemTimeToFileTime( &sty, (FILETIME*)&iy );
		
	return abs((int)( (iy - ix) / __int64( 10000000 ) ));
}

////////////////////////////////////////////////////////////////////////////////
// 
// OrderTimeStamps returns true if timestamp Y is later than timestamp X
// 
////////////////////////////////////////////////////////////////////////////////
int OrderTimeStamps( const TIMESTAMP_STRUCT& X, const TIMESTAMP_STRUCT& Y)
{
	SYSTEMTIME stx,sty;
	__int64 ix,iy;

	TimeStampToSystemTime( X, stx );
	SystemTimeToFileTime( &stx, (FILETIME*)&ix );

	TimeStampToSystemTime( Y, sty );
	SystemTimeToFileTime( &sty, (FILETIME*)&iy );
		
	if(iy > ix)
	{
		return true;
	}
	else
	{
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
//  Get date in human-readable locale-specific format
// 
////////////////////////////////////////////////////////////////////////////////
BOOL GetDateString( int Year, int Month, int Day, CEMSString& sDateTime)
{
	size_t nTCHARs;
	SYSTEMTIME sysTime;

	ZeroMemory( &sysTime, sizeof(sysTime) );

	sysTime.wYear = Year;
	sysTime.wMonth = Month;
	sysTime.wDay = Day;

	// get the size of the date string
	if ((nTCHARs = GetDateFormat( LOCALE_USER_DEFAULT, DATE_LONGDATE, &sysTime, NULL, NULL, 0 )) == 0)
	{
		return FALSE;
	}

	// make room in the tstring
	if (sDateTime.size() < nTCHARs)
		sDateTime.resize(nTCHARs);
	
	// get the date string
	if ((nTCHARs = GetDateFormat( LOCALE_USER_DEFAULT, DATE_LONGDATE, &sysTime, NULL, (TCHAR*) sDateTime.data(), nTCHARs )) == 0)
	{
		return FALSE;
	}
	
	// get rid of the terminating null that was
	// placed at the end of the string
	sDateTime.erase( sDateTime.length() -1, 1 );

	return TRUE;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Get the hour in human-readable locale-specific format	              
\*--------------------------------------------------------------------------*/
BOOL GetHourString( int Hour, CEMSString& sDateTime )
{
	size_t nTCHARs;
	SYSTEMTIME sysTime;
	
	ZeroMemory( &sysTime, sizeof(sysTime) );
	
	sysTime.wHour = Hour;
	
	// get the size of the date string
	if ((nTCHARs = GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOMINUTESORSECONDS, &sysTime, NULL, NULL, 0 )) == 0)
	{
		return FALSE;
	}
	
	// make room in the tstring
	if (sDateTime.size() < nTCHARs)
		sDateTime.resize(nTCHARs);
	
	// get the date string
	if ((nTCHARs = GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOMINUTESORSECONDS, &sysTime, NULL, (TCHAR*) sDateTime.data(), nTCHARs )) == 0)
	{
		return FALSE;
	}
	
	// get rid of the terminating null that was
	// placed at the end of the string
	sDateTime.erase( sDateTime.length() -1, 1 );
	
	return TRUE;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Are we in Office Hours for this object?	              
\*--------------------------------------------------------------------------*/
bool InOfficeHours( int nHrStart, int nMinStart, int nAmPmStart, int nHrEnd, int nMinEnd, int nAmPmEnd )
{
	TIMESTAMP_STRUCT Now;
	GetTimeStamp( Now );

	if ( nAmPmStart == 1 && nHrStart == 12 )
	{
		nHrStart = 0;
	}
	else if ( nAmPmStart == 2 && nHrStart != 12 )
	{
		nHrStart = nHrStart + 12;
	}

	if ( Now.hour < nHrStart )
		return false;

	if ( nHrStart == Now.hour && Now.minute < nMinStart )
		return false;

	if ( nAmPmEnd == 1 && nHrEnd == 12 )
	{
		nHrEnd = 0;
	}
	else if ( nAmPmEnd == 2 && nHrEnd != 12 )
	{
		nHrEnd = nHrEnd + 12;
	}
	
	if ( Now.hour > nHrEnd )
		return false;

	if ( nHrEnd == Now.hour && Now.minute > nMinEnd )
		return false;
	
	return true;
}

BOOL ConvertToTimeZone(const TIMESTAMP_STRUCT& timein, long lBias, TIMESTAMP_STRUCT& timeout)
{
	TIME_ZONE_INFORMATION tzi;
	DWORD dwRes = GetTimeZoneInformation(&tzi);
	if (dwRes == TIME_ZONE_ID_INVALID)
	{
		LINETRACE(_T("GetTimeZoneInformation() failed in ConvertToTimeZone() (code %d)"), GetLastError());
		return FALSE;
	}

	SYSTEMTIME sysTime;
	sysTime.wDay = timein.day;
	sysTime.wDayOfWeek = 0;
	sysTime.wHour = timein.hour;
	sysTime.wMilliseconds = 0;
	sysTime.wMinute = timein.minute;
	sysTime.wMonth = timein.month;
	sysTime.wSecond = timein.second;
	sysTime.wYear = timein.year;

	SYSTEMTIME sysUtcTime;
	if (!TzSpecificLocalTimeToSystemTime(&tzi, &sysTime, &sysUtcTime))
	{
		LINETRACE(_T("TzSpecificLocalTimeToSystemTime() failed in ConvertToTimeZone() (code %d)"), GetLastError());
		return FALSE;
	}

	tzi.Bias = lBias;

	SYSTEMTIME sysLocTime;
	if (!SystemTimeToTzSpecificLocalTime(&tzi, &sysUtcTime, &sysLocTime))
	{
		assert(0);
		LINETRACE(_T("SystemTimeToTzSpecificLocalTime() failed in SetValuesConvertLocal() (code %d)"), GetLastError());
		return FALSE;
	}

	timeout.year = sysLocTime.wYear;
    timeout.month = sysLocTime.wMonth;
    timeout.day = sysLocTime.wDay;
    timeout.hour = sysLocTime.wHour;
    timeout.minute = sysLocTime.wMinute;
    timeout.second = sysLocTime.wSecond;    

	return TRUE;
}

BOOL ConvertFromTimeZone(const TIMESTAMP_STRUCT& timein, long lBias, TIMESTAMP_STRUCT& timeout)
{
	TIME_ZONE_INFORMATION tzi;
	DWORD dwRes = GetTimeZoneInformation(&tzi);
	long lTempBias = tzi.Bias;
	if (dwRes == TIME_ZONE_ID_INVALID)
	{
		LINETRACE(_T("GetTimeZoneInformation() failed in ConvertToTimeZone() (code %d)"), GetLastError());
		return FALSE;
	}

	SYSTEMTIME inTime;
	inTime.wDay = timein.day;
	inTime.wDayOfWeek = 0;
	inTime.wHour = timein.hour;
	inTime.wMilliseconds = 0;
	inTime.wMinute = timein.minute;
	inTime.wMonth = timein.month;
	inTime.wSecond = timein.second;
	inTime.wYear = timein.year;
	
	tzi.Bias = lBias;

	SYSTEMTIME sysUtcTime;
	if (!TzSpecificLocalTimeToSystemTime(&tzi, &inTime, &sysUtcTime))
	{
		LINETRACE(_T("TzSpecificLocalTimeToSystemTime() failed in ConvertToTimeZone() (code %d)"), GetLastError());
		return FALSE;
	}

	tzi.Bias = lTempBias;

	SYSTEMTIME sysLocTime;
	if (!SystemTimeToTzSpecificLocalTime(&tzi, &sysUtcTime, &sysLocTime))
	{
		assert(0);
		LINETRACE(_T("SystemTimeToTzSpecificLocalTime() failed in SetValuesConvertLocal() (code %d)"), GetLastError());
		return FALSE;
	}

	timeout.year = sysLocTime.wYear;
    timeout.month = sysLocTime.wMonth;
    timeout.day = sysLocTime.wDay;
    timeout.hour = sysLocTime.wHour;
    timeout.minute = sysLocTime.wMinute;
    timeout.second = sysLocTime.wSecond;    

	return TRUE;
}

long GetTimeZoneOffset(int TimeZoneID)
{
	//TimeZoneID from TimeZones table
	//TimeZoneOffset is minutes from UTC
	switch(TimeZoneID)
	{
	case 1:
		return 0;
	case 2:
		return 0;
	case 3:
		return 0;
	case 4:
		return -60;
	case 5:
		return -60;
	case 6:
		return -60;
	case 7:
		return -60;
	case 8:
		return -60;
	case 9:
		return -60;
	case 10:
		return -120;
	case 11:
		return -120;
	case 12:
		return -120;
	case 13:
		return -120;
	case 14:
		return -120;
	case 15:
		return -120;
	case 16:
		return -120;
	case 17:
		return -120;
	case 18:
		return -120;
	case 19:
		return -120;
	case 20:
		return -120;
	case 21:
		return -120;
	case 22:
		return -120;
	case 23:
		return -180;
	case 24:
		return -180;
	case 25:
		return -180;
	case 26:
		return -180;
	case 27:
		return -180;
	case 28:
		return -210;
	case 29:
		return -240;
	case 30:
		return -240;
	case 31:
		return -240;
	case 32:
		return -240;
	case 33:
		return -240;
	case 34:
		return -240;
	case 35:
		return -240;
	case 36:
		return -270;
	case 37:
		return -300;
	case 38:
		return -300;
	case 39:
		return -300;
	case 40:
		return -330;
	case 41:
		return -330;
	case 42:
		return -345;
	case 43:
		return -360;
	case 44:
		return -360;
	case 45:
		return -360;
	case 46:
		return -390;
	case 47:
		return -420;
	case 48:
		return -420;
	case 49:
		return -420;
	case 50:
		return -420;
	case 51:
		return -420;
	case 52:
		return -480;
	case 53:
		return -480;
	case 54:
		return -480;
	case 55:
		return -480;
	case 56:
		return -480;
	case 57:
		return -480;
	case 58:
		return -510;
	case 59:
		return -525;
	case 60:
		return -540;
	case 61:
		return -540;
	case 62:
		return -540;
	case 63:
		return -540;
	case 64:
		return -570;
	case 65:
		return -570;
	case 66:
		return -600;
	case 67:
		return -600;
	case 68:
		return -600;
	case 69:
		return -600;
	case 70:
		return -600;
	case 71:
		return -630;
	case 72:
		return -660;
	case 73:
		return -660;
	case 74:
		return -660;
	case 75:
		return -660;
	case 76:
		return -660;
	case 77:
		return -660;
	case 78:
		return -720;
	case 79:
		return -720;
	case 80:
		return -720;
	case 81:
		return -720;
	case 82:
		return -720;
	case 83:
		return -765;
	case 84:
		return -780;
	case 85:
		return -780;
	case 86:
		return -840;
	case 87:
		return 60;
	case 88:
		return 60;
	case 89:
		return 120;
	case 90:
		return 120;
	case 91:
		return 180;
	case 92:
		return 180;
	case 93:
		return 180;
	case 94:
		return 180;
	case 95:
		return 180;
	case 96:
		return 180;
	case 97:
		return 180;
	case 98:
		return 180;
	case 99:
		return 210;
	case 100:
		return 240;
	case 101:
		return 240;
	case 102:
		return 240;
	case 103:
		return 240;
	case 104:
		return 240;
	case 105:
		return 240;
	case 106:
		return 240;
	case 107:
		return 300;
	case 108:
		return 300;
	case 109:
		return 300;
	case 110:
		return 300;
	case 111:
		return 300;
	case 112:
		return 300;
	case 113:
		return 360;
	case 114:
		return 360;
	case 115:
		return 360;
	case 116:
		return 360;
	case 117:
		return 360;
	case 118:
		return 420;
	case 119:
		return 420;
	case 120:
		return 420;
	case 121:
		return 480;
	case 122:
		return 480;
	case 123:
		return 480;
	case 124:
		return 540;
	case 125:
		return 540;
	case 126:
		return 570;
	case 127:
		return 600;
	case 128:
		return 600;
	case 129:
		return 660;
	case 130:
		return 720;

	}
	return 1;
}