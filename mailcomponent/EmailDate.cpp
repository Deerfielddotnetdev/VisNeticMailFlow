// EmailDate.cpp: implementation of the CEmailDate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EmailDate.h"
#include <time.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEmailDate::CEmailDate()
{
    // the DwDateTime class has code to figure out the time zone.
    // we can reuse that code.
	
    DwDateTime date;
	
    m_nYear = date.Year();
    m_nMonth = date.Month();
    m_nDay = date.Day();
    m_nHour = date.Hour();
    m_nMinute = date.Minute();
    m_nSecond = date.Second();
    m_nZone = date.Zone();
	m_nDayOfWeek = date.DayOfTheWeek();
}

CEmailDate::CEmailDate(const DwDateTime& date)
{
    m_nYear = date.Year();
    m_nMonth = date.Month();
    m_nDay = date.Day();
    m_nHour = date.Hour();
    m_nMinute = date.Minute();
    m_nSecond = date.Second();
    m_nZone = date.Zone();
	m_nDayOfWeek = date.DayOfTheWeek();
}

CEmailDate::CEmailDate(const SYSTEMTIME& sysTime)
{
	DwDateTime date;

	m_nYear = sysTime.wYear;
	m_nMonth = sysTime.wMonth;
	m_nDay = sysTime.wDay;
	m_nHour = sysTime.wHour;
	m_nMinute = sysTime.wMinute;
	m_nSecond = sysTime.wSecond;
	m_nDayOfWeek = sysTime.wDayOfWeek;
	m_nZone = date.Zone();
}

CEmailDate::CEmailDate(const TIMESTAMP_STRUCT& sqlTime)
{
	DwDateTime date;

	m_nYear = sqlTime.year;
	m_nMonth = sqlTime.month;
	m_nDay = sqlTime.day;
	m_nHour = sqlTime.hour;
	m_nMinute = sqlTime.minute;
	m_nSecond = sqlTime.second;
	m_nDayOfWeek = 0;
	m_nSecond = sqlTime.second;
	m_nZone = date.Zone();
}

void CEmailDate::FromSqlTime(const TIMESTAMP_STRUCT& sqlTime)
{
	DwDateTime date;

	m_nYear = sqlTime.year;
	m_nMonth = sqlTime.month;
	m_nDay = sqlTime.day;
	m_nHour = sqlTime.hour;
	m_nMinute = sqlTime.minute;
	m_nSecond = sqlTime.second;
	m_nDayOfWeek = 0;
	m_nSecond = sqlTime.second;
	m_nZone = date.Zone();
}

SYSTEMTIME& CEmailDate::GetSystemTime()
{
	m_sysTime.wDay = m_nDay;
	m_sysTime.wHour = m_nHour;
	m_sysTime.wMilliseconds = 0;
	m_sysTime.wMinute = m_nMinute;
	m_sysTime.wMonth = m_nMonth;
	m_sysTime.wSecond = m_nSecond;
	m_sysTime.wYear = m_nYear;
	m_sysTime.wDayOfWeek = m_nDayOfWeek;

	return m_sysTime;
}

TIMESTAMP_STRUCT& CEmailDate::GetSQLTime()
{
	m_sqlTime.day = m_nDay;
	m_sqlTime.hour = m_nHour;
	m_sqlTime.fraction = 0;
	m_sqlTime.minute = m_nMinute;
	m_sqlTime.month = m_nMonth;
	m_sqlTime.second = m_nSecond;
	m_sqlTime.year = m_nYear;
	
	return m_sqlTime;
}

CEmailDate::~CEmailDate()
{

}

CEmailDate& CEmailDate::operator = (const CEmailDate& aDate)
{
    if (this != &aDate) 
	{
        m_nYear = aDate.m_nYear;
        m_nMonth = aDate.m_nMonth;
        m_nDay = aDate.m_nDay;
        m_nHour = aDate.m_nHour;
        m_nMinute = aDate.m_nMinute;
        m_nSecond = aDate.m_nSecond;
        m_nZone = aDate.m_nZone;
		m_nDayOfWeek = aDate.m_nDayOfWeek;
    }
	
    return *this;
}


void CEmailDate::SetValues(int aYear, int aMonth, int aDay, int aHour,
						   int aMinute, int aSecond, int dayofweek)
{
    // the DwDateTime class has code to figure out the time zone.
    // we can reuse that code.
	
    DwDateTime date;
    date.SetValuesLocal(aYear, aMonth, aDay, aHour, aMinute, aSecond);
	
    m_nYear = date.Year();
    m_nMonth = date.Month();
    m_nDay = date.Day();
    m_nHour = date.Hour();
    m_nMinute = date.Minute();
    m_nSecond = date.Second();
    m_nZone = date.Zone();
	m_nDayOfWeek = date.DayOfTheWeek();
}

BOOL CEmailDate::SetValuesConvertLocal(int year, int month, int day, int hour, int minute, 
									   int second, int dayofweek, int zone)
{
	TIME_ZONE_INFORMATION tzi;
	DWORD dwRes = GetTimeZoneInformation(&tzi);
	if (dwRes == TIME_ZONE_ID_INVALID)
	{
		assert(0);
		LINETRACE(_T("GetTimeZoneInformation() failed in SetValuesConvertLocal() (code %d)"), GetLastError());
		return FALSE;
	}

	tzi.Bias += zone;
	
	SYSTEMTIME sysTime;
	sysTime.wDay = day;
	sysTime.wDayOfWeek = dayofweek;
	sysTime.wHour = hour;
	sysTime.wMilliseconds = 0;
	sysTime.wMinute = minute;
	sysTime.wMonth = month;
	sysTime.wSecond = second;
	sysTime.wYear = year;

	SYSTEMTIME sysLocTime;
	if (!SystemTimeToTzSpecificLocalTime(&tzi, &sysTime, &sysLocTime))
	{
		assert(0);
		LINETRACE(_T("SystemTimeToTzSpecificLocalTime() failed in SetValuesConvertLocal() (code %d)"), GetLastError());
		return FALSE;
	}

	m_nYear = sysLocTime.wYear;
    m_nMonth = sysLocTime.wMonth;
    m_nDay = sysLocTime.wDay;
    m_nHour = sysLocTime.wHour;
    m_nMinute = sysLocTime.wMinute;
    m_nSecond = sysLocTime.wSecond;
    m_nZone = zone;
	m_nDayOfWeek = sysLocTime.wDayOfWeek;

	return TRUE;
}

void CEmailDate::SetValues(int aYear, int aMonth, int aDay, int aHour,
						   int aMinute, int aSecond, int dayofweek, int aZone)
{
    m_nYear = aYear;
    m_nMonth = aMonth;
    m_nDay = aDay;
    m_nHour = aHour;
    m_nMinute = aMinute;
    m_nSecond = aSecond;
    m_nZone = aZone;
	m_nDayOfWeek = dayofweek;
}


void CEmailDate::SetCurrent()
{
    *this = CEmailDate();
}

const string& CEmailDate::GetDisplayString()
{
    char scratch[100];
    struct tm stime;
	
    stime.tm_year = m_nYear - 1900;
    stime.tm_mon = m_nMonth - 1;
    stime.tm_mday = m_nDay;
    stime.tm_hour = m_nHour;
    stime.tm_min = m_nMinute;
    stime.tm_sec = m_nSecond;
	mktime(&stime);
    strftime(scratch, sizeof(scratch), "%a, %d %b %Y", &stime);
    m_String = scratch;
    return m_String;
}


BOOL CEmailDate::GetElapsedSystemTime(const SYSTEMTIME &first, const SYSTEMTIME &second, SYSTEMTIME &result)
{
	FILETIME ffirst, fsecond;
	if (!SystemTimeToFileTime(&first, &ffirst) ||
		!SystemTimeToFileTime(&second, &fsecond))
	{
		assert(0);
		int nErr = GetLastError();
		LINETRACE(_T("SystemTimeToFileTime failed: %d\n"), nErr);
		return FALSE;
	}

	ULONGLONG llDiff;
	ULONGLONG llFirst = reinterpret_cast<ULONGLONG&>(ffirst);
	ULONGLONG llSecond = reinterpret_cast<ULONGLONG&>(fsecond);
	if (llFirst > llSecond)
		llDiff = llFirst - llSecond;
	else
		llDiff = llSecond - llFirst;

	FILETIME fDiff = reinterpret_cast<FILETIME&>(llDiff);
	if (!FileTimeToSystemTime(&fDiff, &result))
	{
		assert(0);
		int nErr = GetLastError();
		LINETRACE(_T("FileTimeToSystemTime failed: %d\n"), nErr);
		return FALSE;
	}

	// adjust for 1/1/1601
	result.wYear -= 1601;
	result.wMonth -= 1;
	result.wDay -= 1;

	return TRUE;
}

const BOOL CEmailDate::GetDiffSecs(const SYSTEMTIME &first, const SYSTEMTIME &second, UINT &seconds)
{
	tm t;
	ZeroMemory(&t, sizeof(t));
	t.tm_hour = first.wHour;
	t.tm_mday = first.wDay;
	t.tm_min = first.wMinute;
	t.tm_sec = first.wSecond;
	t.tm_year = first.wYear - 1900;
	t.tm_mon = first.wMonth;
	t.tm_wday = first.wDayOfWeek;

	tm tt;
	ZeroMemory(&tt, sizeof(tt));
	tt.tm_hour = second.wHour;
	tt.tm_mday = second.wDay;
	tt.tm_min = second.wMinute;
	tt.tm_sec = second.wSecond;
	tt.tm_year = second.wYear - 1900;
	tt.tm_mon = second.wMonth;
	tt.tm_wday = second.wDayOfWeek;

	time_t t1 = mktime(&t);
	assert(t1 != -1);
	time_t t2 = mktime(&tt);
	assert(t2 != -1);
	if (t1 == -1 || t2 == -1)
		return FALSE;

	if (t2 >= t1)
		seconds = (UINT)difftime(t2, t1);
	else
		seconds = (UINT)difftime(t1, t2);

	return TRUE;
}


// static function
// converts an SQL_TIMESTAMP_STRUCT to a SYSTEMTIME struct
BOOL CEmailDate::SQLTimeToSystemTime(const TIMESTAMP_STRUCT& sqlTime, SYSTEMTIME &sysTime)
{
	// validation
	if (sqlTime.day < 1 || sqlTime.day > 31 
		|| sqlTime.hour < 0 || sqlTime.hour > 24 
		|| sqlTime.minute < 0 || sqlTime.minute > 60
		|| sqlTime.second < 0 || sqlTime.second > 60
		|| sqlTime.year < 1601 || sqlTime.year > 30827)
	{
		LINETRACE(_T("SQLTimeToSystemTime received invalid SQL_TIMESTAMP_STRUCT: conversion failed\n"));
		return FALSE;
	}

	// note: these values are not converted!
	sysTime.wMilliseconds = 0;
	sysTime.wDayOfWeek = 0;

	sysTime.wDay = sqlTime.day;
	sysTime.wHour = sqlTime.hour;
	sysTime.wMinute = sqlTime.minute;
	sysTime.wMonth = sqlTime.month;
	sysTime.wSecond = sqlTime.second;
	sysTime.wYear = sqlTime.year;

	return TRUE; // success
}

// static function
BOOL CEmailDate::SystemTimeToSQLTime(const SYSTEMTIME &sysTime, TIMESTAMP_STRUCT& sqlTime)
{
	// validation
	if (sysTime.wDay < 1 || sysTime.wDay > 31 
		|| sysTime.wHour < 0 || sysTime.wHour > 24 
		|| sysTime.wMinute < 0 || sysTime.wMinute > 60
		|| sysTime.wSecond < 0 || sysTime.wSecond > 60
		|| sysTime.wYear < 1601 || sysTime.wYear > 30827)
	{
		LINETRACE(_T("SystemTimeToSQLTime received invalid SYSTEMTIME: conversion failed\n"));
		return FALSE;
	}
	
	sqlTime.day = sysTime.wDay;
	sqlTime.hour = sysTime.wHour;
	sqlTime.minute = sysTime.wMinute;
	sqlTime.month = sysTime.wMonth;
	sqlTime.second = sysTime.wSecond;
	sqlTime.year = sysTime.wYear;
	sqlTime.fraction = 0;
	
	return TRUE; // success
}

int CEmailDate::CompareSystemTimes(const SYSTEMTIME &first, const SYSTEMTIME &second)
{
	FILETIME ft1, ft2;

	SystemTimeToFileTime(&first, &ft1);
	SystemTimeToFileTime(&second, &ft2);

	return CompareFileTime(&ft1, &ft2);
}

int CEmailDate::CompareDwDateTimes(const DwDateTime &first, const DwDateTime &second)
{
	CEmailDate date1(first);
	CEmailDate date2(second);

	return CompareSystemTimes(date1.GetSystemTime(), date2.GetSystemTime());
}
