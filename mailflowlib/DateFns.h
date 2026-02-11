/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/DateFns.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Date / Time Helper Functions
||              
\\*************************************************************************/

#ifndef DATE_FUNCTIONS_H
#define DATE_FUNCTIONS_H

void VarDateToTimeStamp( DATE& date, TIMESTAMP_STRUCT& timestamp );
void TimeStampToVarDate( TIMESTAMP_STRUCT& timestamp, DATE& date );
void GetTimeStamp( TIMESTAMP_STRUCT& timestamp );
void SecondsToTimeStamp( time_t seconds, TIMESTAMP_STRUCT& timestamp );

void TimeStampToSystemTime(const TIMESTAMP_STRUCT& timestamp, SYSTEMTIME &sysTime);
void SystemTimeToTimeStamp(const SYSTEMTIME &sysTime, TIMESTAMP_STRUCT& timestamp);

BOOL GetDateTimeString(const SYSTEMTIME &sysTime, CEMSString& sDateTime, 
					   bool bDate = true, bool bTime = true );
BOOL GetDateTimeString(const TIMESTAMP_STRUCT& TimeStamp, int TimeStampLen, CEMSString& sDateTime, 
					   bool bDate = true, bool bTime = true );
BOOL GetFullDateTimeString(const SYSTEMTIME &sysTime, CEMSString& sDateTime);
BOOL GetFullDateTimeString(const TIMESTAMP_STRUCT& TimeStamp, int TimeStampLen, CEMSString& sDateTime);

int CompareTimeStamps( const TIMESTAMP_STRUCT& X, const TIMESTAMP_STRUCT& Y);
BOOL OrderTimeStamps( const TIMESTAMP_STRUCT& X, const TIMESTAMP_STRUCT& Y);
BOOL GetDateString( int Year, int Month, int Day, CEMSString& sDateTime);
BOOL GetHourString( int Hour, CEMSString& sDateTime );
BOOL ConvertToTimeZone(const TIMESTAMP_STRUCT& timein, long lBias, TIMESTAMP_STRUCT& timeout);
BOOL ConvertFromTimeZone(const TIMESTAMP_STRUCT& timein, long lBias, TIMESTAMP_STRUCT& timeout);
long GetTimeZoneOffset(int TimeZoneID);

bool InOfficeHours( int nHrStart, int nMinStart, int nAmPmStart, int nHrEnd, int nMinEnd, int nAmPmEnd );

#endif // DATE_FUNCTIONS_H