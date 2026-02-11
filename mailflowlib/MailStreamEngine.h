

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Fri Aug 19 09:38:48 2005
 */
/* Compiler settings for .\MailStreamEngine.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
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

#ifndef __MailStreamEngine_h__
#define __MailStreamEngine_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IRoutingEngineComm_FWD_DEFINED__
#define __IRoutingEngineComm_FWD_DEFINED__
typedef interface IRoutingEngineComm IRoutingEngineComm;
#endif 	/* __IRoutingEngineComm_FWD_DEFINED__ */


#ifndef __IRoutingEngineAdmin_FWD_DEFINED__
#define __IRoutingEngineAdmin_FWD_DEFINED__
typedef interface IRoutingEngineAdmin IRoutingEngineAdmin;
#endif 	/* __IRoutingEngineAdmin_FWD_DEFINED__ */


#ifndef __RoutingEngineComm_FWD_DEFINED__
#define __RoutingEngineComm_FWD_DEFINED__

#ifdef __cplusplus
typedef class RoutingEngineComm RoutingEngineComm;
#else
typedef struct RoutingEngineComm RoutingEngineComm;
#endif /* __cplusplus */

#endif 	/* __RoutingEngineComm_FWD_DEFINED__ */


#ifndef __RoutingEngineAdmin_FWD_DEFINED__
#define __RoutingEngineAdmin_FWD_DEFINED__

#ifdef __cplusplus
typedef class RoutingEngineAdmin RoutingEngineAdmin;
#else
typedef struct RoutingEngineAdmin RoutingEngineAdmin;
#endif /* __cplusplus */

#endif 	/* __RoutingEngineAdmin_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IRoutingEngineComm_INTERFACE_DEFINED__
#define __IRoutingEngineComm_INTERFACE_DEFINED__

/* interface IRoutingEngineComm */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IRoutingEngineComm;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5C804426-FB84-451B-A255-D131B6F194E1")
    IRoutingEngineComm : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReloadConfig( 
            long nConfigurationItem) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ProcessInboundQueue( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ProcessOutboundQueue( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Log( 
            long ErrorCode,
            BSTR Text) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SendAlert( 
            long AlertEventID,
            long TicketBoxID,
            BSTR Text) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE TestPOP3Auth( 
            BSTR HostName,
            long nPort,
            VARIANT_BOOL IsAPOP,
            BSTR UserName,
            BSTR Password,
            long TimeoutSecs,
			long isSSL,
            /* [retval][out] */ long *pError) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE TestSMTPAuth( 
            BSTR HostName,
            long nPort,
            BSTR UserName,
            BSTR Password,
            long TimeoutSecs,
			long isSSL,
            /* [retval][out] */ long *pError) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DoDBMaintenance( 
            /* [retval][out] */ long *pError) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RestoreArchive( 
            long ArchiveID,
            /* [retval][out] */ long *pError) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RestoreBackup( 
            BSTR BackupFile,
            /* [retval][out] */ long *pError) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ScanFile( 
            BSTR File,
            long Options,
            /* [out] */ BSTR *VirusName,
            /* [out] */ BSTR *QuarantineLoc,
            /* [retval][out] */ long *pError) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ScanString( 
            BSTR String,
            /* [out] */ BSTR *VirusName,
            /* [out] */ BSTR *CleanedString,
            /* [retval][out] */ long *pError) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAVInfo( 
            /* [out] */ BSTR *Version,
            /* [out] */ BSTR *RecordCount,
            /* [out] */ BSTR *LastUpdate,
            /* [retval][out] */ long *pError) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRoutingEngineCommVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRoutingEngineComm * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRoutingEngineComm * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRoutingEngineComm * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRoutingEngineComm * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRoutingEngineComm * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRoutingEngineComm * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRoutingEngineComm * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ReloadConfig )( 
            IRoutingEngineComm * This,
            long nConfigurationItem);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ProcessInboundQueue )( 
            IRoutingEngineComm * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ProcessOutboundQueue )( 
            IRoutingEngineComm * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Log )( 
            IRoutingEngineComm * This,
            long ErrorCode,
            BSTR Text);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendAlert )( 
            IRoutingEngineComm * This,
            long AlertEventID,
            long TicketBoxID,
            BSTR Text);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *TestPOP3Auth )( 
            IRoutingEngineComm * This,
            BSTR HostName,
            long nPort,
            VARIANT_BOOL IsAPOP,
            BSTR UserName,
            BSTR Password,
            long TimeoutSecs,
			long isSSL,
            /* [retval][out] */ long *pError);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *TestSMTPAuth )( 
            IRoutingEngineComm * This,
            BSTR HostName,
            long nPort,
            BSTR UserName,
            BSTR Password,
            long TimeoutSecs,
			long isSSL,
            /* [retval][out] */ long *pError);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DoDBMaintenance )( 
            IRoutingEngineComm * This,
            /* [retval][out] */ long *pError);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RestoreArchive )( 
            IRoutingEngineComm * This,
            long ArchiveID,
            /* [retval][out] */ long *pError);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RestoreBackup )( 
            IRoutingEngineComm * This,
            BSTR BackupFile,
            /* [retval][out] */ long *pError);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ScanFile )( 
            IRoutingEngineComm * This,
            BSTR File,
            long Options,
            /* [out] */ BSTR *VirusName,
            /* [out] */ BSTR *QuarantineLoc,
            /* [retval][out] */ long *pError);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ScanString )( 
            IRoutingEngineComm * This,
            BSTR String,
            /* [out] */ BSTR *VirusName,
            /* [out] */ BSTR *CleanedString,
            /* [retval][out] */ long *pError);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAVInfo )( 
            IRoutingEngineComm * This,
            /* [out] */ BSTR *Version,
            /* [out] */ BSTR *RecordCount,
            /* [out] */ BSTR *LastUpdate,
            /* [retval][out] */ long *pError);
        
        END_INTERFACE
    } IRoutingEngineCommVtbl;

    interface IRoutingEngineComm
    {
        CONST_VTBL struct IRoutingEngineCommVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRoutingEngineComm_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRoutingEngineComm_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRoutingEngineComm_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRoutingEngineComm_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRoutingEngineComm_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRoutingEngineComm_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRoutingEngineComm_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRoutingEngineComm_ReloadConfig(This,nConfigurationItem)	\
    (This)->lpVtbl -> ReloadConfig(This,nConfigurationItem)

#define IRoutingEngineComm_ProcessInboundQueue(This)	\
    (This)->lpVtbl -> ProcessInboundQueue(This)

#define IRoutingEngineComm_ProcessOutboundQueue(This)	\
    (This)->lpVtbl -> ProcessOutboundQueue(This)

#define IRoutingEngineComm_Log(This,ErrorCode,Text)	\
    (This)->lpVtbl -> Log(This,ErrorCode,Text)

#define IRoutingEngineComm_SendAlert(This,AlertEventID,TicketBoxID,Text)	\
    (This)->lpVtbl -> SendAlert(This,AlertEventID,TicketBoxID,Text)

#define IRoutingEngineComm_TestPOP3Auth(This,HostName,nPort,IsAPOP,UserName,Password,TimeoutSecs,isSSL,pError)	\
    (This)->lpVtbl -> TestPOP3Auth(This,HostName,nPort,IsAPOP,UserName,Password,TimeoutSecs,isSSL,pError)

#define IRoutingEngineComm_TestSMTPAuth(This,HostName,nPort,UserName,Password,TimeoutSecs,isSSL,pError)	\
    (This)->lpVtbl -> TestSMTPAuth(This,HostName,nPort,UserName,Password,TimeoutSecs,isSSL,pError)

#define IRoutingEngineComm_DoDBMaintenance(This,pError)	\
    (This)->lpVtbl -> DoDBMaintenance(This,pError)

#define IRoutingEngineComm_RestoreArchive(This,ArchiveID,pError)	\
    (This)->lpVtbl -> RestoreArchive(This,ArchiveID,pError)

#define IRoutingEngineComm_RestoreBackup(This,BackupFile,pError)	\
    (This)->lpVtbl -> RestoreBackup(This,BackupFile,pError)

#define IRoutingEngineComm_ScanFile(This,File,Options,VirusName,QuarantineLoc,pError)	\
    (This)->lpVtbl -> ScanFile(This,File,Options,VirusName,QuarantineLoc,pError)

#define IRoutingEngineComm_ScanString(This,String,VirusName,CleanedString,pError)	\
    (This)->lpVtbl -> ScanString(This,String,VirusName,CleanedString,pError)

#define IRoutingEngineComm_GetAVInfo(This,Version,RecordCount,LastUpdate,pError)	\
    (This)->lpVtbl -> GetAVInfo(This,Version,RecordCount,LastUpdate,pError)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_ReloadConfig_Proxy( 
    IRoutingEngineComm * This,
    long nConfigurationItem);


void __RPC_STUB IRoutingEngineComm_ReloadConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_ProcessInboundQueue_Proxy( 
    IRoutingEngineComm * This);


void __RPC_STUB IRoutingEngineComm_ProcessInboundQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_ProcessOutboundQueue_Proxy( 
    IRoutingEngineComm * This);


void __RPC_STUB IRoutingEngineComm_ProcessOutboundQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_Log_Proxy( 
    IRoutingEngineComm * This,
    long ErrorCode,
    BSTR Text);


void __RPC_STUB IRoutingEngineComm_Log_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_SendAlert_Proxy( 
    IRoutingEngineComm * This,
    long AlertEventID,
    long TicketBoxID,
    BSTR Text);


void __RPC_STUB IRoutingEngineComm_SendAlert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_TestPOP3Auth_Proxy( 
    IRoutingEngineComm * This,
    BSTR HostName,
    long nPort,
    VARIANT_BOOL IsAPOP,
    BSTR UserName,
    BSTR Password,
    long TimeoutSecs,
	long isSSL,
    /* [retval][out] */ long *pError);


void __RPC_STUB IRoutingEngineComm_TestPOP3Auth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_TestSMTPAuth_Proxy( 
    IRoutingEngineComm * This,
    BSTR HostName,
    long nPort,
    BSTR UserName,
    BSTR Password,
    long TimeoutSecs,
	long isSSL,
    /* [retval][out] */ long *pError);


void __RPC_STUB IRoutingEngineComm_TestSMTPAuth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_DoDBMaintenance_Proxy( 
    IRoutingEngineComm * This,
    /* [retval][out] */ long *pError);


void __RPC_STUB IRoutingEngineComm_DoDBMaintenance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_RestoreArchive_Proxy( 
    IRoutingEngineComm * This,
    long ArchiveID,
    /* [retval][out] */ long *pError);


void __RPC_STUB IRoutingEngineComm_RestoreArchive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_RestoreBackup_Proxy( 
    IRoutingEngineComm * This,
    BSTR BackupFile,
    /* [retval][out] */ long *pError);


void __RPC_STUB IRoutingEngineComm_RestoreBackup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_ScanFile_Proxy( 
    IRoutingEngineComm * This,
    BSTR File,
    long Options,
    /* [out] */ BSTR *VirusName,
    /* [out] */ BSTR *QuarantineLoc,
    /* [retval][out] */ long *pError);


void __RPC_STUB IRoutingEngineComm_ScanFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_ScanString_Proxy( 
    IRoutingEngineComm * This,
    BSTR String,
    /* [out] */ BSTR *VirusName,
    /* [out] */ BSTR *CleanedString,
    /* [retval][out] */ long *pError);


void __RPC_STUB IRoutingEngineComm_ScanString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineComm_GetAVInfo_Proxy( 
    IRoutingEngineComm * This,
    /* [out] */ BSTR *Version,
    /* [out] */ BSTR *RecordCount,
    /* [out] */ BSTR *LastUpdate,
    /* [retval][out] */ long *pError);


void __RPC_STUB IRoutingEngineComm_GetAVInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRoutingEngineComm_INTERFACE_DEFINED__ */


#ifndef __IRoutingEngineAdmin_INTERFACE_DEFINED__
#define __IRoutingEngineAdmin_INTERFACE_DEFINED__

/* interface IRoutingEngineAdmin */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IRoutingEngineAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("32DCD893-06C9-4FF7-B1C6-5C4D913BCD35")
    IRoutingEngineAdmin : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Authenticate( 
            /* [in] */ BSTR LogonName,
            /* [in] */ BSTR Password,
            /* [retval][out] */ long *pResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE TestODBC( 
            /* [in] */ BSTR Server,
            /* [in] */ long UseWIA,
            /* [in] */ BSTR LogonName,
            /* [in] */ BSTR Password,
            /* [in] */ BSTR Domain,
            /* [in] */ BSTR Database,
            /* [out] */ BSTR *pErrMsg,
            /* [retval][out] */ long *pResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetServerParameter( 
            /* [in] */ long nID,
            /* [out] */ BSTR *bstrValue,
            /* [retval][out] */ long *pResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetServerParameter( 
            /* [in] */ long nID,
            /* [in] */ BSTR bstrValue,
            /* [retval][out] */ long *pResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetPassword( 
            /* [in] */ BSTR User,
            /* [in] */ BSTR Password,
            /* [retval][out] */ long *pResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRoutingEngineAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRoutingEngineAdmin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRoutingEngineAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRoutingEngineAdmin * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRoutingEngineAdmin * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRoutingEngineAdmin * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRoutingEngineAdmin * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRoutingEngineAdmin * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Authenticate )( 
            IRoutingEngineAdmin * This,
            /* [in] */ BSTR LogonName,
            /* [in] */ BSTR Password,
            /* [retval][out] */ long *pResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *TestODBC )( 
            IRoutingEngineAdmin * This,
            /* [in] */ BSTR Server,
            /* [in] */ long UseWIA,
            /* [in] */ BSTR LogonName,
            /* [in] */ BSTR Password,
            /* [in] */ BSTR Domain,
            /* [in] */ BSTR Database,
            /* [out] */ BSTR *pErrMsg,
            /* [retval][out] */ long *pResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetServerParameter )( 
            IRoutingEngineAdmin * This,
            /* [in] */ long nID,
            /* [out] */ BSTR *bstrValue,
            /* [retval][out] */ long *pResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetServerParameter )( 
            IRoutingEngineAdmin * This,
            /* [in] */ long nID,
            /* [in] */ BSTR bstrValue,
            /* [retval][out] */ long *pResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetPassword )( 
            IRoutingEngineAdmin * This,
            /* [in] */ BSTR User,
            /* [in] */ BSTR Password,
            /* [retval][out] */ long *pResult);
        
        END_INTERFACE
    } IRoutingEngineAdminVtbl;

    interface IRoutingEngineAdmin
    {
        CONST_VTBL struct IRoutingEngineAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRoutingEngineAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRoutingEngineAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRoutingEngineAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRoutingEngineAdmin_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRoutingEngineAdmin_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRoutingEngineAdmin_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRoutingEngineAdmin_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRoutingEngineAdmin_Authenticate(This,LogonName,Password,pResult)	\
    (This)->lpVtbl -> Authenticate(This,LogonName,Password,pResult)

#define IRoutingEngineAdmin_TestODBC(This,Server,UseWIA,LogonName,Password,Domain,Database,pErrMsg,pResult)	\
    (This)->lpVtbl -> TestODBC(This,Server,UseWIA,LogonName,Password,Domain,Database,pErrMsg,pResult)

#define IRoutingEngineAdmin_GetServerParameter(This,nID,bstrValue,pResult)	\
    (This)->lpVtbl -> GetServerParameter(This,nID,bstrValue,pResult)

#define IRoutingEngineAdmin_SetServerParameter(This,nID,bstrValue,pResult)	\
    (This)->lpVtbl -> SetServerParameter(This,nID,bstrValue,pResult)

#define IRoutingEngineAdmin_SetPassword(This,User,Password,pResult)	\
    (This)->lpVtbl -> SetPassword(This,User,Password,pResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineAdmin_Authenticate_Proxy( 
    IRoutingEngineAdmin * This,
    /* [in] */ BSTR LogonName,
    /* [in] */ BSTR Password,
    /* [retval][out] */ long *pResult);


void __RPC_STUB IRoutingEngineAdmin_Authenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineAdmin_TestODBC_Proxy( 
    IRoutingEngineAdmin * This,
    /* [in] */ BSTR Server,
    /* [in] */ long UseWIA,
    /* [in] */ BSTR LogonName,
    /* [in] */ BSTR Password,
    /* [in] */ BSTR Domain,
    /* [in] */ BSTR Database,
    /* [out] */ BSTR *pErrMsg,
    /* [retval][out] */ long *pResult);


void __RPC_STUB IRoutingEngineAdmin_TestODBC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineAdmin_GetServerParameter_Proxy( 
    IRoutingEngineAdmin * This,
    /* [in] */ long nID,
    /* [out] */ BSTR *bstrValue,
    /* [retval][out] */ long *pResult);


void __RPC_STUB IRoutingEngineAdmin_GetServerParameter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineAdmin_SetServerParameter_Proxy( 
    IRoutingEngineAdmin * This,
    /* [in] */ long nID,
    /* [in] */ BSTR bstrValue,
    /* [retval][out] */ long *pResult);


void __RPC_STUB IRoutingEngineAdmin_SetServerParameter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IRoutingEngineAdmin_SetPassword_Proxy( 
    IRoutingEngineAdmin * This,
    /* [in] */ BSTR User,
    /* [in] */ BSTR Password,
    /* [retval][out] */ long *pResult);


void __RPC_STUB IRoutingEngineAdmin_SetPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRoutingEngineAdmin_INTERFACE_DEFINED__ */



#ifndef __MAILSTREAMENGINELib_LIBRARY_DEFINED__
#define __MAILSTREAMENGINELib_LIBRARY_DEFINED__

/* library MAILSTREAMENGINELib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_MAILSTREAMENGINELib;

EXTERN_C const CLSID CLSID_RoutingEngineComm;

#ifdef __cplusplus

class DECLSPEC_UUID("B47C1233-2A8E-4D46-B983-3E8A10D17F75")
RoutingEngineComm;
#endif

EXTERN_C const CLSID CLSID_RoutingEngineAdmin;

#ifdef __cplusplus

class DECLSPEC_UUID("772926AD-F557-4010-A92C-E0DA95CB0AC6")
RoutingEngineAdmin;
#endif
#endif /* __MAILSTREAMENGINELib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


