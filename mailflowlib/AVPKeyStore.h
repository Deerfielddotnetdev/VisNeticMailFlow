
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0279 */
/* at Mon Apr 22 20:24:59 2002
 */
/* Compiler settings for F:\projects\AVPIKeyStore\AVPIKeyStoreFactory.idl:
    Os (OptLev=s), W1, Zp8, env=Win32 (32b run), ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
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

#ifndef __AVPIKeyStoreFactory_h__
#define __AVPIKeyStoreFactory_h__

/* Forward Declarations */ 

#ifndef __IAVPKeyStore_FWD_DEFINED__
#define __IAVPKeyStore_FWD_DEFINED__
typedef interface IAVPKeyStore IAVPKeyStore;
#endif 	/* __IAVPKeyStore_FWD_DEFINED__ */


#ifndef __AVPKeyStore_FWD_DEFINED__
#define __AVPKeyStore_FWD_DEFINED__

#ifdef __cplusplus
typedef class AVPKeyStore AVPKeyStore;
#else
typedef struct AVPKeyStore AVPKeyStore;
#endif /* __cplusplus */

#endif 	/* __AVPKeyStore_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IAVPKeyStore_INTERFACE_DEFINED__
#define __IAVPKeyStore_INTERFACE_DEFINED__

/* interface IAVPKeyStore */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IAVPKeyStore;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34BB3007-CBA0-45C4-A3DD-7EDBFFAD54F7")
    IAVPKeyStore : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Key( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Key( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AppID( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AppID( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProductCode( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ProductCode( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserSize( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserSize( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_KeyType( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_KeyType( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SerialNum( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SerialNum( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ExpireYear( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ExpireYear( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ExpireMonth( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ExpireMonth( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ExpireDay( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ExpireDay( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CreateYear( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CreateYear( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CreateMonth( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CreateMonth( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CreateDay( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CreateDay( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ResellerID( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ResellerID( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserType( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserType( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddKey( 
            VARIANT_BOOL ReplaceExisting,
            /* [retval][out] */ long __RPC_FAR *Error) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CheckKey( 
            /* [retval][out] */ long __RPC_FAR *Error) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateEvalKey( 
            /* [retval][out] */ long __RPC_FAR *Error) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RemoveKey( 
            /* [retval][out] */ long __RPC_FAR *Error) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAVPKeyStoreVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAVPKeyStore __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAVPKeyStore __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Key )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Key )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AppID )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AppID )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProductCode )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProductCode )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_UserSize )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_UserSize )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_KeyType )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_KeyType )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SerialNum )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SerialNum )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ExpireYear )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ExpireYear )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ExpireMonth )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ExpireMonth )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ExpireDay )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ExpireDay )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CreateYear )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CreateYear )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CreateMonth )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CreateMonth )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CreateDay )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CreateDay )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ResellerID )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ResellerID )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_UserType )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_UserType )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddKey )( 
            IAVPKeyStore __RPC_FAR * This,
            VARIANT_BOOL ReplaceExisting,
            /* [retval][out] */ long __RPC_FAR *Error);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CheckKey )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *Error);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateEvalKey )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *Error);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveKey )( 
            IAVPKeyStore __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *Error);
        
        END_INTERFACE
    } IAVPKeyStoreVtbl;

    interface IAVPKeyStore
    {
        CONST_VTBL struct IAVPKeyStoreVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAVPKeyStore_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAVPKeyStore_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAVPKeyStore_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAVPKeyStore_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAVPKeyStore_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAVPKeyStore_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAVPKeyStore_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAVPKeyStore_get_Key(This,pVal)	\
    (This)->lpVtbl -> get_Key(This,pVal)

#define IAVPKeyStore_put_Key(This,newVal)	\
    (This)->lpVtbl -> put_Key(This,newVal)

#define IAVPKeyStore_get_AppID(This,pVal)	\
    (This)->lpVtbl -> get_AppID(This,pVal)

#define IAVPKeyStore_put_AppID(This,newVal)	\
    (This)->lpVtbl -> put_AppID(This,newVal)

#define IAVPKeyStore_get_ProductCode(This,pVal)	\
    (This)->lpVtbl -> get_ProductCode(This,pVal)

#define IAVPKeyStore_put_ProductCode(This,newVal)	\
    (This)->lpVtbl -> put_ProductCode(This,newVal)

#define IAVPKeyStore_get_UserSize(This,pVal)	\
    (This)->lpVtbl -> get_UserSize(This,pVal)

#define IAVPKeyStore_put_UserSize(This,newVal)	\
    (This)->lpVtbl -> put_UserSize(This,newVal)

#define IAVPKeyStore_get_KeyType(This,pVal)	\
    (This)->lpVtbl -> get_KeyType(This,pVal)

#define IAVPKeyStore_put_KeyType(This,newVal)	\
    (This)->lpVtbl -> put_KeyType(This,newVal)

#define IAVPKeyStore_get_SerialNum(This,pVal)	\
    (This)->lpVtbl -> get_SerialNum(This,pVal)

#define IAVPKeyStore_put_SerialNum(This,newVal)	\
    (This)->lpVtbl -> put_SerialNum(This,newVal)

#define IAVPKeyStore_get_ExpireYear(This,pVal)	\
    (This)->lpVtbl -> get_ExpireYear(This,pVal)

#define IAVPKeyStore_put_ExpireYear(This,newVal)	\
    (This)->lpVtbl -> put_ExpireYear(This,newVal)

#define IAVPKeyStore_get_ExpireMonth(This,pVal)	\
    (This)->lpVtbl -> get_ExpireMonth(This,pVal)

#define IAVPKeyStore_put_ExpireMonth(This,newVal)	\
    (This)->lpVtbl -> put_ExpireMonth(This,newVal)

#define IAVPKeyStore_get_ExpireDay(This,pVal)	\
    (This)->lpVtbl -> get_ExpireDay(This,pVal)

#define IAVPKeyStore_put_ExpireDay(This,newVal)	\
    (This)->lpVtbl -> put_ExpireDay(This,newVal)

#define IAVPKeyStore_get_CreateYear(This,pVal)	\
    (This)->lpVtbl -> get_CreateYear(This,pVal)

#define IAVPKeyStore_put_CreateYear(This,newVal)	\
    (This)->lpVtbl -> put_CreateYear(This,newVal)

#define IAVPKeyStore_get_CreateMonth(This,pVal)	\
    (This)->lpVtbl -> get_CreateMonth(This,pVal)

#define IAVPKeyStore_put_CreateMonth(This,newVal)	\
    (This)->lpVtbl -> put_CreateMonth(This,newVal)

#define IAVPKeyStore_get_CreateDay(This,pVal)	\
    (This)->lpVtbl -> get_CreateDay(This,pVal)

#define IAVPKeyStore_put_CreateDay(This,newVal)	\
    (This)->lpVtbl -> put_CreateDay(This,newVal)

#define IAVPKeyStore_get_ResellerID(This,pVal)	\
    (This)->lpVtbl -> get_ResellerID(This,pVal)

#define IAVPKeyStore_put_ResellerID(This,newVal)	\
    (This)->lpVtbl -> put_ResellerID(This,newVal)

#define IAVPKeyStore_get_UserType(This,pVal)	\
    (This)->lpVtbl -> get_UserType(This,pVal)

#define IAVPKeyStore_put_UserType(This,newVal)	\
    (This)->lpVtbl -> put_UserType(This,newVal)

#define IAVPKeyStore_AddKey(This,ReplaceExisting,Error)	\
    (This)->lpVtbl -> AddKey(This,ReplaceExisting,Error)

#define IAVPKeyStore_CheckKey(This,Error)	\
    (This)->lpVtbl -> CheckKey(This,Error)

#define IAVPKeyStore_CreateEvalKey(This,Error)	\
    (This)->lpVtbl -> CreateEvalKey(This,Error)

#define IAVPKeyStore_RemoveKey(This,Error)	\
    (This)->lpVtbl -> RemoveKey(This,Error)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_Key_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_Key_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_Key_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IAVPKeyStore_put_Key_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_AppID_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_AppID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_AppID_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IAVPKeyStore_put_AppID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_ProductCode_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_ProductCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_ProductCode_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IAVPKeyStore_put_ProductCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_UserSize_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_UserSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_UserSize_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_UserSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_KeyType_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_KeyType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_KeyType_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_KeyType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_SerialNum_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_SerialNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_SerialNum_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_SerialNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_ExpireYear_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_ExpireYear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_ExpireYear_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_ExpireYear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_ExpireMonth_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_ExpireMonth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_ExpireMonth_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_ExpireMonth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_ExpireDay_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_ExpireDay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_ExpireDay_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_ExpireDay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_CreateYear_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_CreateYear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_CreateYear_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_CreateYear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_CreateMonth_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_CreateMonth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_CreateMonth_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_CreateMonth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_CreateDay_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_CreateDay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_CreateDay_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_CreateDay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_ResellerID_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_ResellerID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_ResellerID_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_ResellerID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_get_UserType_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IAVPKeyStore_get_UserType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_put_UserType_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IAVPKeyStore_put_UserType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_AddKey_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    VARIANT_BOOL ReplaceExisting,
    /* [retval][out] */ long __RPC_FAR *Error);


void __RPC_STUB IAVPKeyStore_AddKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_CheckKey_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *Error);


void __RPC_STUB IAVPKeyStore_CheckKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_CreateEvalKey_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *Error);


void __RPC_STUB IAVPKeyStore_CreateEvalKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPKeyStore_RemoveKey_Proxy( 
    IAVPKeyStore __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *Error);


void __RPC_STUB IAVPKeyStore_RemoveKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAVPKeyStore_INTERFACE_DEFINED__ */



#ifndef __AVPKEYSTORELib_LIBRARY_DEFINED__
#define __AVPKEYSTORELib_LIBRARY_DEFINED__

/* library AVPKEYSTORELib */
/* [helpstring][version][uuid] */ 

typedef 
enum UserSizeEnum
    {	UserSize_1	= 0,
	UserSize_3	= 1,
	UserSize_6	= 2,
	UserSize_12	= 3,
	UserSize_25	= 4,
	UserSize_50	= 5,
	UserSize_100	= 6,
	UserSize_250	= 7,
	UserSize_500	= 8,
	UserSize_1000	= 9,
	UserSize_2500	= 10,
	UserSize_Unlimited	= 11,
	UserSize_Site	= 12,
	UserSize_Enterprise	= 13,
	UserSize_Undefined_1	= 14,
	UserSize_Undefined_2	= 15
    }	UserSizeEnum;

typedef 
enum KeyTypeEnum
    {	KeyType_Evaluation	= 0,
	KeyType_Registered	= 1,
	KeyType_Undefined1	= 2,
	KeyType_Undefined2	= 3
    }	KeyTypeEnum;

typedef 
enum ErrorCodesEnum
    {	Success	= 0,
	Error_Invalid_Day	= 1,
	Error_Invalid_Month	= 2,
	Error_Invalid_AppID	= 3,
	Error_Invalid_ProductCode	= 4,
	Error_Invalid_Key	= 5,
	Error_Invalid_Year	= 6,
	Error_No_Key_Found	= 7,
	Error_Key_Expired	= 8,
	Error_Registered_Key_Exists	= 9
    }	ErrorCodesEnum;

typedef 
enum UserTypeEnum
    {	UserType_User	= 0,
	UserType_Pack	= 1,
	UserType_Undefined1	= 2,
	UserType_Undefined2	= 3
    }	UserTypeEnum;


EXTERN_C const IID LIBID_AVPKEYSTORELib;

EXTERN_C const CLSID CLSID_AVPKeyStore;

#ifdef __cplusplus

class DECLSPEC_UUID("12FF2875-999F-49BC-93D4-D0534BC38095")
AVPKeyStore;
#endif
#endif /* __AVPKEYSTORELib_LIBRARY_DEFINED__ */

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


