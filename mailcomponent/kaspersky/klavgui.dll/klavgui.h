/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Fri Oct 26 12:34:50 2001
 */
/* Compiler settings for D:\sources\avengine\klav\klavgui.dll\klavgui.idl:
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

#ifndef __klavgui_h__
#define __klavgui_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IAVPConfigureScenarioByGUI_FWD_DEFINED__
#define __IAVPConfigureScenarioByGUI_FWD_DEFINED__
typedef interface IAVPConfigureScenarioByGUI IAVPConfigureScenarioByGUI;
#endif 	/* __IAVPConfigureScenarioByGUI_FWD_DEFINED__ */


#ifndef __AVPConfigureScenarioByGUI_FWD_DEFINED__
#define __AVPConfigureScenarioByGUI_FWD_DEFINED__

#ifdef __cplusplus
typedef class AVPConfigureScenarioByGUI AVPConfigureScenarioByGUI;
#else
typedef struct AVPConfigureScenarioByGUI AVPConfigureScenarioByGUI;
#endif /* __cplusplus */

#endif 	/* __AVPConfigureScenarioByGUI_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IAVPConfigureScenarioByGUI_INTERFACE_DEFINED__
#define __IAVPConfigureScenarioByGUI_INTERFACE_DEFINED__

/* interface IAVPConfigureScenarioByGUI */
/* [unique][helpstring][dual][uuid][object] */ 


//EXTERN_C const IID IID_IAVPConfigureScenarioByGUI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F9220ABD-8FBD-11D4-9C9C-00D0B7161E9B")
    IAVPConfigureScenarioByGUI : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ConfigureScenario( 
            /* [in] */ HWND hwndParent,
            /* [out][in] */ DWORD __RPC_FAR *pdwConfigSize,
            /* [size_is][in] */ BYTE __RPC_FAR *pbyInConfig,
            /* [size_is][out] */ BYTE __RPC_FAR *pbyOutConfig) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAVPConfigureScenarioByGUIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAVPConfigureScenarioByGUI __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAVPConfigureScenarioByGUI __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAVPConfigureScenarioByGUI __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IAVPConfigureScenarioByGUI __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IAVPConfigureScenarioByGUI __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IAVPConfigureScenarioByGUI __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IAVPConfigureScenarioByGUI __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ConfigureScenario )( 
            IAVPConfigureScenarioByGUI __RPC_FAR * This,
            /* [in] */ HWND hwndParent,
            /* [out][in] */ DWORD __RPC_FAR *pdwConfigSize,
            /* [size_is][in] */ BYTE __RPC_FAR *pbyInConfig,
            /* [size_is][out] */ BYTE __RPC_FAR *pbyOutConfig);
        
        END_INTERFACE
    } IAVPConfigureScenarioByGUIVtbl;

    interface IAVPConfigureScenarioByGUI
    {
        CONST_VTBL struct IAVPConfigureScenarioByGUIVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAVPConfigureScenarioByGUI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAVPConfigureScenarioByGUI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAVPConfigureScenarioByGUI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAVPConfigureScenarioByGUI_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAVPConfigureScenarioByGUI_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAVPConfigureScenarioByGUI_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAVPConfigureScenarioByGUI_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAVPConfigureScenarioByGUI_ConfigureScenario(This,hwndParent,pdwConfigSize,pbyInConfig,pbyOutConfig)	\
    (This)->lpVtbl -> ConfigureScenario(This,hwndParent,pdwConfigSize,pbyInConfig,pbyOutConfig)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IAVPConfigureScenarioByGUI_ConfigureScenario_Proxy( 
    IAVPConfigureScenarioByGUI __RPC_FAR * This,
    /* [in] */ HWND hwndParent,
    /* [out][in] */ DWORD __RPC_FAR *pdwConfigSize,
    /* [size_is][in] */ BYTE __RPC_FAR *pbyInConfig,
    /* [size_is][out] */ BYTE __RPC_FAR *pbyOutConfig);


void __RPC_STUB IAVPConfigureScenarioByGUI_ConfigureScenario_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAVPConfigureScenarioByGUI_INTERFACE_DEFINED__ */



#ifndef __KLAVGUILib_LIBRARY_DEFINED__
#define __KLAVGUILib_LIBRARY_DEFINED__

/* library KLAVGUILib */
/* [helpstring][version][uuid] */ 


//EXTERN_C const IID LIBID_KLAVGUILib;

//EXTERN_C const CLSID CLSID_AVPConfigureScenarioByGUI;

#ifdef __cplusplus

class DECLSPEC_UUID("F9220AAE-8FBD-11D4-9C9C-00D0B7161E9B")
AVPConfigureScenarioByGUI;
#endif
#endif /* __KLAVGUILib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long __RPC_FAR *, HWND __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
