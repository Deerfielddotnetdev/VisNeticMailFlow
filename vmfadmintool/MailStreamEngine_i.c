

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Thu Sep 15 13:01:31 2005
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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IRoutingEngineComm,0x5C804426,0xFB84,0x451B,0xA2,0x55,0xD1,0x31,0xB6,0xF1,0x94,0xE1);


MIDL_DEFINE_GUID(IID, IID_IRoutingEngineAdmin,0x32DCD893,0x06C9,0x4FF7,0xB1,0xC6,0x5C,0x4D,0x91,0x3B,0xCD,0x35);


MIDL_DEFINE_GUID(IID, LIBID_MAILSTREAMENGINELib,0x395873CD,0x6C24,0x44B3,0xB0,0x57,0xE0,0xA9,0x44,0xF6,0xDE,0x3B);


MIDL_DEFINE_GUID(CLSID, CLSID_RoutingEngineComm,0xB47C1233,0x2A8E,0x4D46,0xB9,0x83,0x3E,0x8A,0x10,0xD1,0x7F,0x75);


MIDL_DEFINE_GUID(CLSID, CLSID_RoutingEngineAdmin,0x772926AD,0xF557,0x4010,0xA9,0x2C,0xE0,0xDA,0x95,0xCB,0x0A,0xC6);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

