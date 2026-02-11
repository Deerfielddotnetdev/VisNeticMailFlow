#include "stdafx.h"
#include ".\TicklerSystemThread.h"

TicklerSystemThread::TicklerSystemThread(void)
	:_futuretime(0)
	,_mfDataConn(0)
{
	DebugReporter::Instance().DisplayMessage("TicklerSystemThread::TicklerSystemThread - Constructor", DebugReporter::ENGINE);
}

TicklerSystemThread::~TicklerSystemThread(void)
{
	DebugReporter::Instance().DisplayMessage("TicklerSystemThread::TicklerSystemThread - Destructor", DebugReporter::ENGINE);
}

void TicklerSystemThread::Initialize()
{
	try
	{
		DebugReporter::Instance().DisplayMessage("TicklerSystemThread::Initialize", DebugReporter::ENGINE);

		_mfDataConn.Initialize();
	}
	catch(dca::Exception& e)
	{
		try
		{
			dca::String er;
			dca::String x(e.GetMessage());
			er.Format("TicklerSystemThread::Initialize - %s", x.c_str());
			DebugReporter::Instance().DisplayMessage(er.c_str(), DebugReporter::ENGINE);
			Log( E_TicklerError, L"%s\n", e.GetMessage());
			
			_mfDataConn.Uninitialize();
		}
		catch(...)
		{}
	}
	catch(...)
	{
		try
		{
			DebugReporter::Instance().DisplayMessage("TicklerSystemThread::Initialize - Database was not initialized", DebugReporter::ENGINE);
			Log( E_TicklerError, L"An Unkown or undefined exception has occurred in the TicklerSystem Thread\n");

			_mfDataConn.Uninitialize();
		}
		catch(...)
		{
		}
	}
}

void TicklerSystemThread::Uninitialize()
{
	try
	{
		_mfDataConn.Uninitialize();
	}
	catch(...)
	{
	}
}

void TicklerSystemThread::Flush()
{
	_event.Set();
}

unsigned int TicklerSystemThread::Run()
{
	DWORD dwRet;
	BOOL bRun = TRUE;
	DWORD dwWaitTime;

	SetEvent( m_hReadyEvent );

	// This is the normal loop
	while ( bRun )
	{
		dwWaitTime = 60000;
		dwRet = MsgWaitForMultipleObjects( 1, &m_hStopEvent, FALSE, dwWaitTime, QS_ALLEVENTS );
		
		switch( dwRet )
		{
		case WAIT_TIMEOUT:
			{
				if(!_DBMaintenanceMutex.IsLocked())
				{
					__int64 currenttime = 0;
					// Get current date and time
					{
						SYSTEMTIME local;
						::GetLocalTime(&local);
						::SystemTimeToFileTime(&local, (FILETIME*)&currenttime);
					}

					/*long rc = ::CompareFileTime((const FILETIME*)&currenttime, (const FILETIME*)&_futuretime);
					if(rc > 0)
					{*/
						DebugReporter::Instance().DisplayMessage("TicklerSystemThread::Run - Checking for Tickets to Reopen", DebugReporter::ENGINE);

						if(this->TryUpdate())
						{
							for(int i = 0; i < 3; i++)
							{
								this->TryUpdate();

								_event.Wait(900);								
							}
						}

						//this->UpdateFutureTime();
					/*}*/
				}				
			}
			break;
		
		case WAIT_OBJECT_0:			// Stop Event is set.
			{
				bRun = FALSE;
			}
			break;
		}
	}
	
	Uninitialize();
	
	return 0;	
}

int TicklerSystemThread::TryUpdate()
{
	try
	{
		DcaTrace(_T("DCA - Testing db connection.\n"));

		if(_mfDataConn.IsConnected())
		{
			DcaTrace(_T("DCA - Ready to update tickets.\n"));

			UpdateTickets();
		}
		else
		{
			_mfDataConn.Initialize();

			DcaTrace(_T("DCA - Re-Testing db connection.\n"));

			if(_mfDataConn.IsConnected() )
			{
				DcaTrace(_T("DCA - Ready to update tickets.\n"));

				UpdateTickets();
			}
		}

		return 0;
	}
	catch(dca::Exception e)
	{
		try
		{
			DcaTrace(_T("DCA - Error occurring in Update Tickets"));
			DcaTrace(e.GetMessage());
			Log( E_TicklerError, L"%s\n", e.GetMessage());
			_mfDataConn.Uninitialize();
		}
		catch(...)
		{
		}
	}
	catch(...)
	{
		try
		{
			DcaTrace(_T("DCA - Error occurring in Update Tickets"));
			Log( E_TicklerError, L"An Unknown or undefined exception has occurred in the TicklerSystem Thread\n");
			_mfDataConn.Uninitialize();
		}
		catch(...)
		{
		}
	}

	return 1;
}

void TicklerSystemThread::UpdateTickets()
{
	std::list<dca::TicketsTable> ticketListToReopen;
	std::list<dca::TicketsTable> ticketListAlreadyOpen;

	SYSTEMTIME stNow;
	ZeroMemory(&stNow, sizeof(SYSTEMTIME));

	FILETIME ftNow;
	ZeroMemory(&ftNow, sizeof(FILETIME));
	
	::GetLocalTime(&stNow);

	if(SystemTimeToFileTime(&stNow, &ftNow))
	{
		dca::SQLStatement sqlStmt(_mfDataConn);

		dca::TicketsTable dcaTT;

		dcaTT.PrepareListFromUseTickler(sqlStmt);

		while(sqlStmt.Fetch())
		{
			if(dcaTT.GetTicketStateID() == EMS_TICKETSTATEID_CLOSED && dcaTT.GetTicklerDateToReopenLen() != SQL_NULL_DATA)
			{
				SYSTEMTIME stReOpenTime;
				ZeroMemory(&stReOpenTime, sizeof(SYSTEMTIME));

				FILETIME ftReOpenTime;
				ZeroMemory(&ftReOpenTime, sizeof(FILETIME));

				dcaTT.ConvertTicklerDateToReopenToSystemTime(stReOpenTime);

				if(SystemTimeToFileTime(&stReOpenTime, &ftReOpenTime))
				{
					if(CompareFileTime(&ftReOpenTime, &ftNow) != 1)
					{
						ticketListToReopen.push_back(dcaTT);
					}
				}
			}
			else
			{
				ticketListAlreadyOpen.push_back(dcaTT);
			}
		}
	}

	{
		std::list<dca::TicketsTable>::iterator iter;
		dca::SQLStatement sqlStmt(_mfDataConn);

		if(ticketListToReopen.size() == 0)
		{
			Log( E_TicklerInfo, L"Did not find any tickets to reopen\n");
			DebugReporter::Instance().DisplayMessage("TicklerSystemThread::UpdateTickets - Did not find any Tickets to Reopen", DebugReporter::ENGINE);
		}

		for(iter = ticketListToReopen.begin();
			iter != ticketListToReopen.end();
			iter++)
		{
			long id = iter->GetUniqueID();

			sqlStmt.BindLongParameter(id);
			sqlStmt.ExecuteDirect(_T("UPDATE Tickets SET TicketStateID = 6, UseTickler = 0, TicklerDateToReopen = NULL, OpenTimeStamp = GETDATE() WHERE TicketID = ?"));

			dca::TicketHistoryTable dcaTHT;
			dcaTHT.SetAgentID(iter->GetOwnerID());
			dcaTHT.SetTicketID(iter->GetUniqueID());
			dcaTHT.SetTicketActionID(EMS_TICKETACTIONID_MODIFIED);
			dcaTHT.SetID1(3);
			dcaTHT.SetID2(iter->GetTicketStateID());

			dcaTHT.Insert(sqlStmt);

			Log( E_TicklerInfo, L"Reopened TicketID %d. Found it was scheduled to open on %s\n",id,iter->GetTicklerDateToReopenAsString());
			dca::String x;
			x.Format("TicklerSystemThread::UpdateTickets - Reopened TicketID %d",id);
			DebugReporter::Instance().DisplayMessage(x.c_str(), DebugReporter::ENGINE);
		}
	}

	
	{
		std::list<dca::TicketsTable>::iterator iter;
		dca::SQLStatement sqlStmt(_mfDataConn);

		for(iter = ticketListAlreadyOpen.begin();
			iter != ticketListAlreadyOpen.end();
			iter++)
		{
			long id = iter->GetUniqueID();

			sqlStmt.BindLongParameter(id);
			sqlStmt.ExecuteDirect(_T("UPDATE Tickets SET UseTickler = 0, TicklerDateToReopen = NULL WHERE TicketID = ?"));
			dca::String x;
			x.Format("TicklerSystemThread::UpdateTickets - TicketID %d was already Open",id);
			DebugReporter::Instance().DisplayMessage(x.c_str(), DebugReporter::ENGINE);
		}
	}
}

void TicklerSystemThread::UpdateFutureTime()
{
	_futuretime = 0;
	
	// Get current date and time and reset time to 12:01am 
	{
		SYSTEMTIME local;
		::GetLocalTime(&local);
		local.wHour = 0;
		local.wMinute = 1;
		local.wSecond = 0;
		local.wMilliseconds = 0;

		::SystemTimeToFileTime(&local, (FILETIME*)&_futuretime);
	}

	// Move current data and time 1 day up 
	_futuretime += ((unsigned __int64)24 * ((unsigned __int64)60 * ((unsigned __int64)60 * (unsigned __int64)10000000)));
}
