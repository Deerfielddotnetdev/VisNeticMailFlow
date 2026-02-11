/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/UploadMap.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Singleton object which stores file upload progress. 
||              
\\*************************************************************************/

// only include this file once
#pragma once

#include <time.h>

class CUploadItem
{
public:
	CUploadItem()
	{
		m_nPercentage = 0;
		time(&m_Time);
	}
	
	int	 GetPercentage(void)	{ return m_nPercentage; }
	void SetPercentage(int nPercentage) { m_nPercentage = nPercentage; }
	
	time_t GetTime(  void ) { return m_Time; }
	void UpdateTime( void )	{ time(&m_Time); }

private:
	int m_nPercentage;
	time_t m_Time;
};

class CUploadMap  
{
public:
	
	// this is the only way to get an instance of this singleton class
	static CUploadMap& GetInstance();

	virtual ~CUploadMap();

	int  GetProgress(unsigned int UploadID);
	void UpdateProgress(unsigned int UploadID, int Percent);
	int	 GetUploadID( void );
	void RemoveOldItems( void );

private:

	CUploadMap();
	void RemoveProgress(unsigned int UploadID);

	map<int, CUploadItem> m_ProgressMap;
	int m_nUploadID;
};
