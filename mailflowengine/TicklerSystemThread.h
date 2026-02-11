#pragma once

#include <EMSMutex.h>

class TicklerSystemThread : public CThread
{
public:
	TicklerSystemThread(void);
	~TicklerSystemThread(void);	

	virtual unsigned int Run( void );

protected:
	virtual void Initialize();
	virtual void Flush();
	virtual void Uninitialize();
	void UpdateTickets();
	int TryUpdate();
	void UpdateFutureTime();
protected:
	unsigned __int64    _futuretime;
	dca::Event	        _event;
	dca::DataConnector  _mfDataConn;
	CDBMaintenanceMutex _DBMaintenanceMutex;
};
