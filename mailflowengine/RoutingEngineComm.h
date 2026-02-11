// RoutingEngineComm.h : Declaration of the CRoutingEngineComm

#ifndef __ROUTINGENGINECOMM_H_
#define __ROUTINGENGINECOMM_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CRoutingEngineComm
class ATL_NO_VTABLE CRoutingEngineComm : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRoutingEngineComm, &CLSID_RoutingEngineComm>,
	public ISupportErrorInfo,
	public IDispatchImpl<IRoutingEngineComm, &IID_IRoutingEngineComm, &LIBID_MAILSTREAMENGINELib>
{
public:
	CRoutingEngineComm();
	~CRoutingEngineComm();

DECLARE_REGISTRY_RESOURCEID(IDR_ROUTINGENGINECOMM)
DECLARE_NOT_AGGREGATABLE(CRoutingEngineComm)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRoutingEngineComm)
	COM_INTERFACE_ENTRY(IRoutingEngineComm)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IRoutingEngineComm
public:
	STDMETHOD(SendAlert)(long AlertEventID, long TicketBoxID, BSTR Text);
	STDMETHOD(Log)(long ErrorCode, BSTR Text);
	STDMETHOD(ProcessOutboundQueue)();
	STDMETHOD(ProcessInboundQueue)();
	STDMETHOD(ReloadConfig)( long nConfigurationItem );
	STDMETHOD(TestPOP3Auth)( BSTR HostName, long nPort, VARIANT_BOOL IsAPOP, BSTR UserName,
		                     BSTR Password, long TimeoutSecs, long isSSL, long* pError );
	STDMETHOD(TestSMTPAuth)( BSTR HostName, long nPort, BSTR UserName,
		                     BSTR Password, long TimeoutSecs, long isSSL, long* pError );
	STDMETHOD(DoDBMaintenance)( long* pError );
	STDMETHOD(RestoreArchive)( long ArchiveID, long* pError );
	STDMETHOD(RestoreBackup)( BSTR BackupFile, long* pError );

	STDMETHOD(ScanFile)( BSTR File, long Options, BSTR* VirusName, BSTR* QuarantineLoc, long* pError );
	STDMETHOD(ScanString)( BSTR String, BSTR* VirusName, BSTR* CleanedString, long* pError );

	STDMETHOD(GetAVInfo)( BSTR* Version, BSTR *RecordCount, 
		                  BSTR* LastUpdate, long* pError );


};



#endif //__ROUTINGENGINECOMM_H_
