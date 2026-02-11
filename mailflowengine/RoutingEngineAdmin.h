// RoutingEngineAdmin.h : Declaration of the CRoutingEngineAdmin

#ifndef __ROUTINGENGINEADMIN_H_
#define __ROUTINGENGINEADMIN_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CRoutingEngineAdmin
class ATL_NO_VTABLE CRoutingEngineAdmin : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRoutingEngineAdmin, &CLSID_RoutingEngineAdmin>,
	public IDispatchImpl<IRoutingEngineAdmin, &IID_IRoutingEngineAdmin, &LIBID_MAILSTREAMENGINELib>
{
public:
	CRoutingEngineAdmin() : m_query(m_db)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ROUTINGENGINEADMIN)
DECLARE_NOT_AGGREGATABLE(CRoutingEngineAdmin)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRoutingEngineAdmin)
	COM_INTERFACE_ENTRY(IRoutingEngineAdmin)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IRoutingEngineAdmin
public:

		STDMETHOD(Authenticate)( BSTR LogonName, BSTR Password, long* pResult );
		STDMETHOD(TestODBC)( BSTR Server, long UseWIA, BSTR LogonName, BSTR Password,
						     BSTR Domain, BSTR Database, BSTR* pErrMsg, long* pResult );
		STDMETHOD(GetServerParameter)( long nID, BSTR* bstrValue, long* pResult );
		STDMETHOD(SetServerParameter)( long nID, BSTR bstrValue, long* pResult );
		STDMETHOD(SetPassword)( BSTR User, BSTR Password, long* pResult );
		
protected:
	BOOL NTUserAuth( BSTR LogonName, BSTR Password, LPTSTR NTDomain );
	BOOL LocalUserAuth( BSTR Password, LPTSTR szDBPassword );
	BOOL VMSUserAuth( BSTR LogonName, BSTR Password );

	CODBCConn m_db;
	CODBCQuery m_query;

};

#endif //__ROUTINGENGINEADMIN_H_
