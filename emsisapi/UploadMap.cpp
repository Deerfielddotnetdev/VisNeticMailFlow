/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/UploadMap.cpp,v 1.2 2005/11/29 21:16:28 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Singleton object which stores file upload progress. 
||              
\\*************************************************************************/

#include "stdafx.h"
#include "UploadMap.h"

CUploadMapMutex	m_UPMutex;

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a reference to the staic class object.  This
||				is the only way to reference the class.	              
\*--------------------------------------------------------------------------*/
CUploadMap& CUploadMap::GetInstance()
{
	static CUploadMap singleton;
    return singleton;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Private Constructor
\*--------------------------------------------------------------------------*/
CUploadMap::CUploadMap()
{
	m_nUploadID = 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Destruction
\*--------------------------------------------------------------------------*/
CUploadMap::~CUploadMap()
{
	
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the uploads progress	              
\*--------------------------------------------------------------------------*/
int CUploadMap::GetProgress(unsigned int UploadID)
{
	int Percent = 0;
	
	if(m_UPMutex.AcquireLock(1000))
	{
		map<int, CUploadItem>::iterator iter;
	
		if( (iter = m_ProgressMap.find( UploadID )) != m_ProgressMap.end() )
			Percent = iter->second.GetPercentage();

		m_UPMutex.ReleaseLock();
	}
	return Percent;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Update the uploads progress	              
\*--------------------------------------------------------------------------*/
void CUploadMap::UpdateProgress(unsigned int UploadID, int Percent)
{
	if(m_UPMutex.AcquireLock(1000))
	{
		map<int, CUploadItem>::iterator iter;
	
		if( (iter = m_ProgressMap.find( UploadID )) != m_ProgressMap.end() )
		{
			iter->second.SetPercentage(Percent);
			iter->second.UpdateTime();
		}
		else
		{
			CUploadItem UploadItem;
			UploadItem.SetPercentage(Percent);
			m_ProgressMap.insert( pair<int, CUploadItem> ( UploadID, UploadItem) );
		}

		m_UPMutex.ReleaseLock();
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Called when an upload is complete	              
\*--------------------------------------------------------------------------*/
void CUploadMap::RemoveProgress(unsigned int UploadID)
{
	if(m_UPMutex.AcquireLock(1000))
	{
		map<int, CUploadItem>::iterator iter;
	
		if( (iter = m_ProgressMap.find( UploadID )) != m_ProgressMap.end() )
			m_ProgressMap.erase( iter );

		m_UPMutex.ReleaseLock();
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a unique ID for file uploads		              
\*--------------------------------------------------------------------------*/
int CUploadMap::GetUploadID( void )
{
	InterlockedIncrement((long*) &m_nUploadID);
	return m_nUploadID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Remove uploads which haven't been updated in more then
||				5 min from the map
\*--------------------------------------------------------------------------*/
void CUploadMap::RemoveOldItems( void )
{
	time_t CurrentTime;
	time(&CurrentTime);

	if(m_UPMutex.AcquireLock(1000))
	{
		map<int, CUploadItem>::iterator iter = m_ProgressMap.begin();

		while ( iter != m_ProgressMap.end() )
		{
			if ( CurrentTime - iter->second.GetTime() > 300 )
			{
				iter = m_ProgressMap.erase(iter);
			}
			else
				iter++;
		}

		m_UPMutex.ReleaseLock();
	}
}