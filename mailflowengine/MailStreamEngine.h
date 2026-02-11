

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Mon Jan 18 22:14:07 2038
 */
/* Compiler settings for MailStreamEngine.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0628 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __MailStreamEngine_h__
#define __MailStreamEngine_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
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
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRoutingEngineComm * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRoutingEngineComm * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRoutingEngineComm * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRoutingEngineComm * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRoutingEngineComm * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRoutingEngineComm * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRoutingEngineComm * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, ReloadConfig)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ReloadConfig )( 
            IRoutingEngineComm * This,
            long nConfigurationItem);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, ProcessInboundQueue)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ProcessInboundQueue )( 
            IRoutingEngineComm * This);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, ProcessOutboundQueue)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ProcessOutboundQueue )( 
            IRoutingEngineComm * This);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, Log)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Log )( 
            IRoutingEngineComm * This,
            long ErrorCode,
            BSTR Text);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, SendAlert)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendAlert )( 
            IRoutingEngineComm * This,
            long AlertEventID,
            long TicketBoxID,
            BSTR Text);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, TestPOP3Auth)
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
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, TestSMTPAuth)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *TestSMTPAuth )( 
            IRoutingEngineComm * This,
            BSTR HostName,
            long nPort,
            BSTR UserName,
            BSTR Password,
            long TimeoutSecs,
            long isSSL,
            /* [retval][out] */ long *pError);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, DoDBMaintenance)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DoDBMaintenance )( 
            IRoutingEngineComm * This,
            /* [retval][out] */ long *pError);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, RestoreArchive)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RestoreArchive )( 
            IRoutingEngineComm * This,
            long ArchiveID,
            /* [retval][out] */ long *pError);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, RestoreBackup)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RestoreBackup )( 
            IRoutingEngineComm * This,
            BSTR BackupFile,
            /* [retval][out] */ long *pError);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, ScanFile)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ScanFile )( 
            IRoutingEngineComm * This,
            BSTR File,
            long Options,
            /* [out] */ BSTR *VirusName,
            /* [out] */ BSTR *QuarantineLoc,
            /* [retval][out] */ long *pError);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, ScanString)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ScanString )( 
            IRoutingEngineComm * This,
            BSTR String,
            /* [out] */ BSTR *VirusName,
            /* [out] */ BSTR *CleanedString,
            /* [retval][out] */ long *pError);
        
        DECLSPEC_XFGVIRT(IRoutingEngineComm, GetAVInfo)
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
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRoutingEngineComm_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRoutingEngineComm_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRoutingEngineComm_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IRoutingEngineComm_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IRoutingEngineComm_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IRoutingEngineComm_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IRoutingEngineComm_ReloadConfig(This,nConfigurationItem)	\
    ( (This)->lpVtbl -> ReloadConfig(This,nConfigurationItem) ) 

#define IRoutingEngineComm_ProcessInboundQueue(This)	\
    ( (This)->lpVtbl -> ProcessInboundQueue(This) ) 

#define IRoutingEngineComm_ProcessOutboundQueue(This)	\
    ( (This)->lpVtbl -> ProcessOutboundQueue(This) ) 

#define IRoutingEngineComm_Log(This,ErrorCode,Text)	\
    ( (This)->lpVtbl -> Log(This,ErrorCode,Text) ) 

#define IRoutingEngineComm_SendAlert(This,AlertEventID,TicketBoxID,Text)	\
    ( (This)->lpVtbl -> SendAlert(This,AlertEventID,TicketBoxID,Text) ) 

#define IRoutingEngineComm_TestPOP3Auth(This,HostName,nPort,IsAPOP,UserName,Password,TimeoutSecs,isSSL,pError)	\
    ( (This)->lpVtbl -> TestPOP3Auth(This,HostName,nPort,IsAPOP,UserName,Password,TimeoutSecs,isSSL,pError) ) 

#define IRoutingEngineComm_TestSMTPAuth(This,HostName,nPort,UserName,Password,TimeoutSecs,isSSL,pError)	\
    ( (This)->lpVtbl -> TestSMTPAuth(This,HostName,nPort,UserName,Password,TimeoutSecs,isSSL,pError) ) 

#define IRoutingEngineComm_DoDBMaintenance(This,pError)	\
    ( (This)->lpVtbl -> DoDBMaintenance(This,pError) ) 

#define IRoutingEngineComm_RestoreArchive(This,ArchiveID,pError)	\
    ( (This)->lpVtbl -> RestoreArchive(This,ArchiveID,pError) ) 

#define IRoutingEngineComm_RestoreBackup(This,BackupFile,pError)	\
    ( (This)->lpVtbl -> RestoreBackup(This,BackupFile,pError) ) 

#define IRoutingEngineComm_ScanFile(This,File,Options,VirusName,QuarantineLoc,pError)	\
    ( (This)->lpVtbl -> ScanFile(This,File,Options,VirusName,QuarantineLoc,pError) ) 

#define IRoutingEngineComm_ScanString(This,String,VirusName,CleanedString,pError)	\
    ( (This)->lpVtbl -> ScanString(This,String,VirusName,CleanedString,pError) ) 

#define IRoutingEngineComm_GetAVInfo(This,Version,RecordCount,LastUpdate,pError)	\
    ( (This)->lpVtbl -> GetAVInfo(This,Version,RecordCount,LastUpdate,pError) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




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
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRoutingEngineAdmin * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRoutingEngineAdmin * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRoutingEngineAdmin * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRoutingEngineAdmin * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRoutingEngineAdmin * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRoutingEngineAdmin * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRoutingEngineAdmin * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IRoutingEngineAdmin, Authenticate)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Authenticate )( 
            IRoutingEngineAdmin * This,
            /* [in] */ BSTR LogonName,
            /* [in] */ BSTR Password,
            /* [retval][out] */ long *pResult);
        
        DECLSPEC_XFGVIRT(IRoutingEngineAdmin, TestODBC)
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
        
        DECLSPEC_XFGVIRT(IRoutingEngineAdmin, GetServerParameter)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetServerParameter )( 
            IRoutingEngineAdmin * This,
            /* [in] */ long nID,
            /* [out] */ BSTR *bstrValue,
            /* [retval][out] */ long *pResult);
        
        DECLSPEC_XFGVIRT(IRoutingEngineAdmin, SetServerParameter)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetServerParameter )( 
            IRoutingEngineAdmin * This,
            /* [in] */ long nID,
            /* [in] */ BSTR bstrValue,
            /* [retval][out] */ long *pResult);
        
        DECLSPEC_XFGVIRT(IRoutingEngineAdmin, SetPassword)
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
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRoutingEngineAdmin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRoutingEngineAdmin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRoutingEngineAdmin_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IRoutingEngineAdmin_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IRoutingEngineAdmin_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IRoutingEngineAdmin_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IRoutingEngineAdmin_Authenticate(This,LogonName,Password,pResult)	\
    ( (This)->lpVtbl -> Authenticate(This,LogonName,Password,pResult) ) 

#define IRoutingEngineAdmin_TestODBC(This,Server,UseWIA,LogonName,Password,Domain,Database,pErrMsg,pResult)	\
    ( (This)->lpVtbl -> TestODBC(This,Server,UseWIA,LogonName,Password,Domain,Database,pErrMsg,pResult) ) 

#define IRoutingEngineAdmin_GetServerParameter(This,nID,bstrValue,pResult)	\
    ( (This)->lpVtbl -> GetServerParameter(This,nID,bstrValue,pResult) ) 

#define IRoutingEngineAdmin_SetServerParameter(This,nID,bstrValue,pResult)	\
    ( (This)->lpVtbl -> SetServerParameter(This,nID,bstrValue,pResult) ) 

#define IRoutingEngineAdmin_SetPassword(This,User,Password,pResult)	\
    ( (This)->lpVtbl -> SetPassword(This,User,Password,pResult) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




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

unsigned long             __RPC_USER  BSTR_UserSize64(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal64(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal64(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree64(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


