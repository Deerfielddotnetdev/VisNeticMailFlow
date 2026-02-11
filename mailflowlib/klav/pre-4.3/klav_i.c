/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Tue Nov 27 19:17:02 2001
 */
/* Compiler settings for D:\sources\avengine\klav\klav.exe\klav.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


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

const IID IID_IAVPAntivirus = {0x1468BF70,0x57D5,0x11D4,{0x9C,0x40,0x00,0xD0,0xB7,0x16,0x1E,0x9B}};


const IID IID_IAVPScenario = {0x1468BF71,0x57D5,0x11D4,{0x9C,0x40,0x00,0xD0,0xB7,0x16,0x1E,0x9B}};


const IID LIBID_KLAVLib = {0x1468BF64,0x57D5,0x11D4,{0x9C,0x40,0x00,0xD0,0xB7,0x16,0x1E,0x9B}};


const CLSID CLSID_AVPAntivirus = {0xD039D7D1,0x570B,0x11D4,{0x9C,0x3F,0x00,0xD0,0xB7,0x16,0x1E,0x9B}};


#ifdef __cplusplus
}
#endif

