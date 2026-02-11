/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Tue Nov 27 19:17:02 2001
 */
/* Compiler settings for D:\sources\avengine\klav\klav.exe\klav.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __klav_h__
#define __klav_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IAVPAntivirus_FWD_DEFINED__
#define __IAVPAntivirus_FWD_DEFINED__
typedef interface IAVPAntivirus IAVPAntivirus;
#endif 	/* __IAVPAntivirus_FWD_DEFINED__ */


#ifndef __IAVPScenario_FWD_DEFINED__
#define __IAVPScenario_FWD_DEFINED__
typedef interface IAVPScenario IAVPScenario;
#endif 	/* __IAVPScenario_FWD_DEFINED__ */


#ifndef __AVPAntivirus_FWD_DEFINED__
#define __AVPAntivirus_FWD_DEFINED__

#ifdef __cplusplus
typedef class AVPAntivirus AVPAntivirus;
#else
typedef struct AVPAntivirus AVPAntivirus;
#endif /* __cplusplus */

#endif 	/* __AVPAntivirus_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_klav_0000 */
/* [local] */ 

typedef /* [v1_enum][helpstring][uuid] */ 
enum tagAVRESULT
    {	AV_FILE_IS_CLEAN	= 0,
	AV_FILE_IS_INFECTED	= AV_FILE_IS_CLEAN + 1,
	AV_FILE_IS_CLEANED	= AV_FILE_IS_INFECTED + 1,
	AV_FILE_IS_SUSPICIOUS	= AV_FILE_IS_CLEANED + 1,
	AV_FILE_IS_NONSCANNED	= AV_FILE_IS_SUSPICIOUS + 1,
	AV_UNEXPECTED_FAILURE	= -1
    }	AVRESULT;

typedef /* [public][public] */ struct  __MIDL___MIDL_itf_klav_0000_0001
    {
    unsigned long m_ulMode;
    unsigned long m_ulCompoundLimitSizeKB;
    unsigned long m_ulRedundantLimitSizeKB;
    }	AVOPTIONS;



extern RPC_IF_HANDLE __MIDL_itf_klav_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_klav_0000_v0_0_s_ifspec;

#ifndef __IAVPAntivirus_INTERFACE_DEFINED__
#define __IAVPAntivirus_INTERFACE_DEFINED__

/* interface IAVPAntivirus */
/* [unique][helpstring][dual][uuid][object] */ 


//EXTERN_C const IID IID_IAVPAntivirus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1468BF70-57D5-11D4-9C40-00D0B7161E9B")
    IAVPAntivirus : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ BSTR bstrDatabasePath,
            /* [in] */ BSTR bstrKeyPath,
            /* [in] */ BSTR bstrTempPath) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Uninitialize( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ScanStream( 
            /* [in] */ DWORD dwScenarioID,
            /* [in] */ IStream __RPC_FAR *pstmScanStream,
            /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
            /* [out] */ BSTR __RPC_FAR *pbstrInfo) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ScanAndCleanStream( 
            /* [in] */ DWORD dwScenarioID,
            /* [in] */ IStream __RPC_FAR *pstmScanStream,
            /* [in] */ IStream __RPC_FAR *pstmCleanStream,
            /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
            /* [out] */ BSTR __RPC_FAR *pbstrInfo) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ScanFile( 
            /* [in] */ DWORD dwScenarioID,
            /* [in] */ BSTR bstrScanFileName,
            /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
            /* [out] */ BSTR __RPC_FAR *pbstrInfo) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ScanAndCleanFile( 
            /* [in] */ DWORD dwScenarioID,
            /* [in] */ BSTR bstrScanFileName,
            /* [in] */ BSTR bstrCleanFileName,
            /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
            /* [out] */ BSTR __RPC_FAR *pbstrInfo) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetErrorInfo( 
            /* [in] */ HRESULT hr,
            /* [out] */ BSTR __RPC_FAR *pbstrErrorInfo) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetVersions( 
            /* [out] */ BSTR __RPC_FAR *pbstrEngineVersion,
            /* [out] */ BSTR __RPC_FAR *pbstrEngineDate,
            /* [out] */ BSTR __RPC_FAR *pbstrDataBaseRecCount,
            /* [out] */ BSTR __RPC_FAR *pbstrDataBaseCurDate) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetLicenceInfo( 
            /* [out] */ BSTR __RPC_FAR *pbstrExpirationDate,
            /* [out] */ BOOL __RPC_FAR *boValid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReloadDatabases( 
            /* [in] */ BSTR bstrDatabasesPath) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReloadOnUpdate( 
            /* [in] */ BOOL boEnable) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAVPAntivirusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAVPAntivirus __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAVPAntivirus __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ BSTR bstrDatabasePath,
            /* [in] */ BSTR bstrKeyPath,
            /* [in] */ BSTR bstrTempPath);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Uninitialize )( 
            IAVPAntivirus __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ScanStream )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ DWORD dwScenarioID,
            /* [in] */ IStream __RPC_FAR *pstmScanStream,
            /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
            /* [out] */ BSTR __RPC_FAR *pbstrInfo);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ScanAndCleanStream )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ DWORD dwScenarioID,
            /* [in] */ IStream __RPC_FAR *pstmScanStream,
            /* [in] */ IStream __RPC_FAR *pstmCleanStream,
            /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
            /* [out] */ BSTR __RPC_FAR *pbstrInfo);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ScanFile )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ DWORD dwScenarioID,
            /* [in] */ BSTR bstrScanFileName,
            /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
            /* [out] */ BSTR __RPC_FAR *pbstrInfo);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ScanAndCleanFile )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ DWORD dwScenarioID,
            /* [in] */ BSTR bstrScanFileName,
            /* [in] */ BSTR bstrCleanFileName,
            /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
            /* [out] */ BSTR __RPC_FAR *pbstrInfo);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorInfo )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ HRESULT hr,
            /* [out] */ BSTR __RPC_FAR *pbstrErrorInfo);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVersions )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrEngineVersion,
            /* [out] */ BSTR __RPC_FAR *pbstrEngineDate,
            /* [out] */ BSTR __RPC_FAR *pbstrDataBaseRecCount,
            /* [out] */ BSTR __RPC_FAR *pbstrDataBaseCurDate);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLicenceInfo )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrExpirationDate,
            /* [out] */ BOOL __RPC_FAR *boValid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReloadDatabases )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ BSTR bstrDatabasesPath);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReloadOnUpdate )( 
            IAVPAntivirus __RPC_FAR * This,
            /* [in] */ BOOL boEnable);
        
        END_INTERFACE
    } IAVPAntivirusVtbl;

    interface IAVPAntivirus
    {
        CONST_VTBL struct IAVPAntivirusVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAVPAntivirus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAVPAntivirus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAVPAntivirus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAVPAntivirus_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAVPAntivirus_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAVPAntivirus_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAVPAntivirus_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAVPAntivirus_Initialize(This,bstrDatabasePath,bstrKeyPath,bstrTempPath)	\
    (This)->lpVtbl -> Initialize(This,bstrDatabasePath,bstrKeyPath,bstrTempPath)

#define IAVPAntivirus_Uninitialize(This)	\
    (This)->lpVtbl -> Uninitialize(This)

#define IAVPAntivirus_ScanStream(This,dwScenarioID,pstmScanStream,pavresultResult,pbstrInfo)	\
    (This)->lpVtbl -> ScanStream(This,dwScenarioID,pstmScanStream,pavresultResult,pbstrInfo)

#define IAVPAntivirus_ScanAndCleanStream(This,dwScenarioID,pstmScanStream,pstmCleanStream,pavresultResult,pbstrInfo)	\
    (This)->lpVtbl -> ScanAndCleanStream(This,dwScenarioID,pstmScanStream,pstmCleanStream,pavresultResult,pbstrInfo)

#define IAVPAntivirus_ScanFile(This,dwScenarioID,bstrScanFileName,pavresultResult,pbstrInfo)	\
    (This)->lpVtbl -> ScanFile(This,dwScenarioID,bstrScanFileName,pavresultResult,pbstrInfo)

#define IAVPAntivirus_ScanAndCleanFile(This,dwScenarioID,bstrScanFileName,bstrCleanFileName,pavresultResult,pbstrInfo)	\
    (This)->lpVtbl -> ScanAndCleanFile(This,dwScenarioID,bstrScanFileName,bstrCleanFileName,pavresultResult,pbstrInfo)

#define IAVPAntivirus_GetErrorInfo(This,hr,pbstrErrorInfo)	\
    (This)->lpVtbl -> GetErrorInfo(This,hr,pbstrErrorInfo)

#define IAVPAntivirus_GetVersions(This,pbstrEngineVersion,pbstrEngineDate,pbstrDataBaseRecCount,pbstrDataBaseCurDate)	\
    (This)->lpVtbl -> GetVersions(This,pbstrEngineVersion,pbstrEngineDate,pbstrDataBaseRecCount,pbstrDataBaseCurDate)

#define IAVPAntivirus_GetLicenceInfo(This,pbstrExpirationDate,boValid)	\
    (This)->lpVtbl -> GetLicenceInfo(This,pbstrExpirationDate,boValid)

#define IAVPAntivirus_ReloadDatabases(This,bstrDatabasesPath)	\
    (This)->lpVtbl -> ReloadDatabases(This,bstrDatabasesPath)

#define IAVPAntivirus_ReloadOnUpdate(This,boEnable)	\
    (This)->lpVtbl -> ReloadOnUpdate(This,boEnable)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_Initialize_Proxy( 
    IAVPAntivirus __RPC_FAR * This,
    /* [in] */ BSTR bstrDatabasePath,
    /* [in] */ BSTR bstrKeyPath,
    /* [in] */ BSTR bstrTempPath);


void __RPC_STUB IAVPAntivirus_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_Uninitialize_Proxy( 
    IAVPAntivirus __RPC_FAR * This);


void __RPC_STUB IAVPAntivirus_Uninitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_ScanStream_Proxy( 
    IAVPAntivirus __RPC_FAR * This,
    /* [in] */ DWORD dwScenarioID,
    /* [in] */ IStream __RPC_FAR *pstmScanStream,
    /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
    /* [out] */ BSTR __RPC_FAR *pbstrInfo);


void __RPC_STUB IAVPAntivirus_ScanStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_ScanAndCleanStream_Proxy( 
    IAVPAntivirus __RPC_FAR * This,
    /* [in] */ DWORD dwScenarioID,
    /* [in] */ IStream __RPC_FAR *pstmScanStream,
    /* [in] */ IStream __RPC_FAR *pstmCleanStream,
    /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
    /* [out] */ BSTR __RPC_FAR *pbstrInfo);


void __RPC_STUB IAVPAntivirus_ScanAndCleanStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_ScanFile_Proxy( 
    IAVPAntivirus __RPC_FAR * This,
    /* [in] */ DWORD dwScenarioID,
    /* [in] */ BSTR bstrScanFileName,
    /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
    /* [out] */ BSTR __RPC_FAR *pbstrInfo);


void __RPC_STUB IAVPAntivirus_ScanFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_ScanAndCleanFile_Proxy( 
    IAVPAntivirus __RPC_FAR * This,
    /* [in] */ DWORD dwScenarioID,
    /* [in] */ BSTR bstrScanFileName,
    /* [in] */ BSTR bstrCleanFileName,
    /* [out] */ AVRESULT __RPC_FAR *pavresultResult,
    /* [out] */ BSTR __RPC_FAR *pbstrInfo);


void __RPC_STUB IAVPAntivirus_ScanAndCleanFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_GetErrorInfo_Proxy( 
    IAVPAntivirus __RPC_FAR * This,
    /* [in] */ HRESULT hr,
    /* [out] */ BSTR __RPC_FAR *pbstrErrorInfo);


void __RPC_STUB IAVPAntivirus_GetErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_GetVersions_Proxy( 
    IAVPAntivirus __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrEngineVersion,
    /* [out] */ BSTR __RPC_FAR *pbstrEngineDate,
    /* [out] */ BSTR __RPC_FAR *pbstrDataBaseRecCount,
    /* [out] */ BSTR __RPC_FAR *pbstrDataBaseCurDate);


void __RPC_STUB IAVPAntivirus_GetVersions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_GetLicenceInfo_Proxy( 
    IAVPAntivirus __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrExpirationDate,
    /* [out] */ BOOL __RPC_FAR *boValid);


void __RPC_STUB IAVPAntivirus_GetLicenceInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_ReloadDatabases_Proxy( 
    IAVPAntivirus __RPC_FAR * This,
    /* [in] */ BSTR bstrDatabasesPath);


void __RPC_STUB IAVPAntivirus_ReloadDatabases_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPAntivirus_ReloadOnUpdate_Proxy( 
    IAVPAntivirus __RPC_FAR * This,
    /* [in] */ BOOL boEnable);


void __RPC_STUB IAVPAntivirus_ReloadOnUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAVPAntivirus_INTERFACE_DEFINED__ */


#ifndef __IAVPScenario_INTERFACE_DEFINED__
#define __IAVPScenario_INTERFACE_DEFINED__

/* interface IAVPScenario */
/* [unique][helpstring][dual][uuid][object] */ 


//EXTERN_C const IID IID_IAVPScenario;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1468BF71-57D5-11D4-9C40-00D0B7161E9B")
    IAVPScenario : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ DWORD dwConfigSize,
            /* [size_is][in] */ BYTE __RPC_FAR *pbyConfig,
            /* [out] */ DWORD __RPC_FAR *pdwScenarioID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ DWORD dwScenarioID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetConfig( 
            /* [in] */ DWORD dwScenatioID,
            /* [out][in] */ DWORD __RPC_FAR *pdwConfigSize,
            /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *pbyConfig) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetConfig( 
            /* [in] */ DWORD dwScenarioID,
            /* [in] */ DWORD dwConfigSize,
            /* [size_is][in] */ BYTE __RPC_FAR *pbyConfig) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddOptions( 
            /* [in] */ AVOPTIONS __RPC_FAR *pavOptions,
            /* [out] */ DWORD __RPC_FAR *pdwScenarioID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAVPScenarioVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAVPScenario __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAVPScenario __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAVPScenario __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IAVPScenario __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IAVPScenario __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IAVPScenario __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IAVPScenario __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            IAVPScenario __RPC_FAR * This,
            /* [in] */ DWORD dwConfigSize,
            /* [size_is][in] */ BYTE __RPC_FAR *pbyConfig,
            /* [out] */ DWORD __RPC_FAR *pdwScenarioID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Remove )( 
            IAVPScenario __RPC_FAR * This,
            /* [in] */ DWORD dwScenarioID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetConfig )( 
            IAVPScenario __RPC_FAR * This,
            /* [in] */ DWORD dwScenatioID,
            /* [out][in] */ DWORD __RPC_FAR *pdwConfigSize,
            /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *pbyConfig);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetConfig )( 
            IAVPScenario __RPC_FAR * This,
            /* [in] */ DWORD dwScenarioID,
            /* [in] */ DWORD dwConfigSize,
            /* [size_is][in] */ BYTE __RPC_FAR *pbyConfig);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddOptions )( 
            IAVPScenario __RPC_FAR * This,
            /* [in] */ AVOPTIONS __RPC_FAR *pavOptions,
            /* [out] */ DWORD __RPC_FAR *pdwScenarioID);
        
        END_INTERFACE
    } IAVPScenarioVtbl;

    interface IAVPScenario
    {
        CONST_VTBL struct IAVPScenarioVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAVPScenario_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAVPScenario_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAVPScenario_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAVPScenario_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAVPScenario_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAVPScenario_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAVPScenario_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAVPScenario_Add(This,dwConfigSize,pbyConfig,pdwScenarioID)	\
    (This)->lpVtbl -> Add(This,dwConfigSize,pbyConfig,pdwScenarioID)

#define IAVPScenario_Remove(This,dwScenarioID)	\
    (This)->lpVtbl -> Remove(This,dwScenarioID)

#define IAVPScenario_GetConfig(This,dwScenatioID,pdwConfigSize,pbyConfig)	\
    (This)->lpVtbl -> GetConfig(This,dwScenatioID,pdwConfigSize,pbyConfig)

#define IAVPScenario_SetConfig(This,dwScenarioID,dwConfigSize,pbyConfig)	\
    (This)->lpVtbl -> SetConfig(This,dwScenarioID,dwConfigSize,pbyConfig)

#define IAVPScenario_AddOptions(This,pavOptions,pdwScenarioID)	\
    (This)->lpVtbl -> AddOptions(This,pavOptions,pdwScenarioID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPScenario_Add_Proxy( 
    IAVPScenario __RPC_FAR * This,
    /* [in] */ DWORD dwConfigSize,
    /* [size_is][in] */ BYTE __RPC_FAR *pbyConfig,
    /* [out] */ DWORD __RPC_FAR *pdwScenarioID);


void __RPC_STUB IAVPScenario_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPScenario_Remove_Proxy( 
    IAVPScenario __RPC_FAR * This,
    /* [in] */ DWORD dwScenarioID);


void __RPC_STUB IAVPScenario_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPScenario_GetConfig_Proxy( 
    IAVPScenario __RPC_FAR * This,
    /* [in] */ DWORD dwScenatioID,
    /* [out][in] */ DWORD __RPC_FAR *pdwConfigSize,
    /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *pbyConfig);


void __RPC_STUB IAVPScenario_GetConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPScenario_SetConfig_Proxy( 
    IAVPScenario __RPC_FAR * This,
    /* [in] */ DWORD dwScenarioID,
    /* [in] */ DWORD dwConfigSize,
    /* [size_is][in] */ BYTE __RPC_FAR *pbyConfig);


void __RPC_STUB IAVPScenario_SetConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPScenario_AddOptions_Proxy( 
    IAVPScenario __RPC_FAR * This,
    /* [in] */ AVOPTIONS __RPC_FAR *pavOptions,
    /* [out] */ DWORD __RPC_FAR *pdwScenarioID);


void __RPC_STUB IAVPScenario_AddOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAVPScenario_INTERFACE_DEFINED__ */



#ifndef __KLAVLib_LIBRARY_DEFINED__
#define __KLAVLib_LIBRARY_DEFINED__

/* library KLAVLib */
/* [helpstring][version][uuid] */ 


//EXTERN_C const IID LIBID_KLAVLib;

//EXTERN_C const CLSID CLSID_AVPAntivirus;

#ifdef __cplusplus

class DECLSPEC_UUID("D039D7D1-570B-11D4-9C3F-00D0B7161E9B")
AVPAntivirus;
#endif
#endif /* __KLAVLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
