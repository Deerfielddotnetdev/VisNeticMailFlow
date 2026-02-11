// EmailDate.h: interface for the CEmailDate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMAILDATE_H__CDFA37C1_048B_4B42_B3AF_FADB57B36110__INCLUDED_)
#define AFX_EMAILDATE_H__CDFA37C1_048B_4B42_B3AF_FADB57B36110__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
using namespace std;
#include <mimepp/mimepp.h>

class CEmailDate  
{
public:
	static const BOOL GetDiffSecs(const SYSTEMTIME& first, const SYSTEMTIME& second, UINT& seconds);
	static BOOL GetElapsedSystemTime(const SYSTEMTIME& first, const SYSTEMTIME& second, SYSTEMTIME& result);
	CEmailDate();

	virtual ~CEmailDate();

	// conversions
	CEmailDate(const SYSTEMTIME& sysTime);
	CEmailDate(const TIMESTAMP_STRUCT& sqlTime);
	CEmailDate(const DwDateTime& date);

	void FromSqlTime(const TIMESTAMP_STRUCT& sqlTime);

	// copy constructor
	CEmailDate(const CEmailDate& date);
	
	// assignment
	CEmailDate& operator = (const CEmailDate& date);

	SYSTEMTIME& GetSystemTime();
	TIMESTAMP_STRUCT& GetSQLTime();

	int GetYear() const				{ return m_nYear; }
    int GetMonth() const			{ return m_nMonth; }
    int GetDay() const				{ return m_nDay; }
    int GetHour() const				{ return m_nHour; }
    int GetMinute() const			{ return m_nMinute; }
    int GetSecond() const			{ return m_nSecond; }
    int GetZone() const				{ return m_nZone; }
	int GetDayOfWeek() const		{ return m_nDayOfWeek; }

	void SetValues(int year, int month, int day, int hour, int minute, int second, int dayofweek);
    void SetValues(int year, int month, int day, int hour, int minute, int second, int dayofweek, int zone);
	BOOL SetValuesConvertLocal(int year, int month, int day, int hour, int minute, int second, int dayofweek, int zone);
	void SetCurrent();

    virtual const string& GetDisplayString();
	
	// statics
	static BOOL SQLTimeToSystemTime(const TIMESTAMP_STRUCT& sqlTime, SYSTEMTIME &sysTime);
	static BOOL SystemTimeToSQLTime(const SYSTEMTIME &sysTime, TIMESTAMP_STRUCT& sqlTime);
	static int CompareDwDateTimes(const DwDateTime& first, const DwDateTime& second);
	static int CompareSystemTimes(const SYSTEMTIME& first, const SYSTEMTIME& second);

private:

	int m_nYear;
    int m_nMonth;
    int m_nDay;
    int m_nHour;
    int m_nMinute;
    int m_nSecond;
    int m_nZone;
	int m_nDayOfWeek;
    string m_String;
	SYSTEMTIME m_sysTime;
	TIMESTAMP_STRUCT m_sqlTime;
};

#endif // !defined(AFX_EMAILDATE_H__CDFA37C1_048B_4B42_B3AF_FADB57B36110__INCLUDED_)
