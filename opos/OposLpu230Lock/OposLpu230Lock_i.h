

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for OposLpu230Lock.idl:
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

#ifndef __OposLpu230Lock_i_h__
#define __OposLpu230Lock_i_h__

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

#ifndef __ISoLpu230Lock_FWD_DEFINED__
#define __ISoLpu230Lock_FWD_DEFINED__
typedef interface ISoLpu230Lock ISoLpu230Lock;

#endif 	/* __ISoLpu230Lock_FWD_DEFINED__ */


#ifndef ___ISoLpu230LockEvents_FWD_DEFINED__
#define ___ISoLpu230LockEvents_FWD_DEFINED__
typedef interface _ISoLpu230LockEvents _ISoLpu230LockEvents;

#endif 	/* ___ISoLpu230LockEvents_FWD_DEFINED__ */


#ifndef __SoLpu230Lock_FWD_DEFINED__
#define __SoLpu230Lock_FWD_DEFINED__

#ifdef __cplusplus
typedef class SoLpu230Lock SoLpu230Lock;
#else
typedef struct SoLpu230Lock SoLpu230Lock;
#endif /* __cplusplus */

#endif 	/* __SoLpu230Lock_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ISoLpu230Lock_INTERFACE_DEFINED__
#define __ISoLpu230Lock_INTERFACE_DEFINED__

/* interface ISoLpu230Lock */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ISoLpu230Lock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E86916C1-4184-4853-B857-4036621FD1CA")
    ISoLpu230Lock : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OpenService( 
            /* [in] */ BSTR DeviceClass,
            /* [in] */ BSTR DeviceName,
            /* [in] */ IDispatch *pDispatch,
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetPropertyNumber( 
            /* [in] */ LONG PropIndex,
            /* [retval][out] */ LONG *pNumber) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetPropertyNumber( 
            /* [in] */ LONG PropIndex,
            /* [in] */ LONG Number) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetPropertyString( 
            /* [in] */ LONG PropIndex,
            /* [retval][out] */ BSTR *pString) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetPropertyString( 
            /* [in] */ LONG PropIndex,
            /* [in] */ BSTR bstrString) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE COFreezeEvents( 
            /* [in] */ VARIANT_BOOL Freeze,
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CheckHealth( 
            /* [in] */ LONG Level,
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ClaimDevice( 
            /* [in] */ LONG lTimeout,
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Close( 
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE DirectIO( 
            /* [in] */ LONG Command,
            /* [out][in] */ LONG *pData,
            /* [out][in] */ BSTR *pString,
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ReleaseDevice( 
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ResetStatistics( 
            /* [in] */ BSTR StatisticsBuffer,
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE RetrieveStatistics( 
            /* [out][in] */ BSTR *StatisticsBuffer,
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE UpdateStatistics( 
            /* [in] */ BSTR StatisticsBuffer,
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE WaitForKeylockChange( 
            /* [in] */ LONG nKeyPosition,
            /* [in] */ LONG nTimeout,
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CompareFirmwareVersion( 
            /* [in] */ BSTR FirmwareFileName,
            /* [out] */ LONG *pResult,
            /* [retval][out] */ LONG *pRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE UpdateFirmware( 
            /* [in] */ BSTR FirmwareFileName,
            /* [retval][out] */ LONG *pRC) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ISoLpu230LockVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoLpu230Lock * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoLpu230Lock * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoLpu230Lock * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISoLpu230Lock * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISoLpu230Lock * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISoLpu230Lock * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISoLpu230Lock * This,
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
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, OpenService)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OpenService )( 
            ISoLpu230Lock * This,
            /* [in] */ BSTR DeviceClass,
            /* [in] */ BSTR DeviceName,
            /* [in] */ IDispatch *pDispatch,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, GetPropertyNumber)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyNumber )( 
            ISoLpu230Lock * This,
            /* [in] */ LONG PropIndex,
            /* [retval][out] */ LONG *pNumber);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, SetPropertyNumber)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetPropertyNumber )( 
            ISoLpu230Lock * This,
            /* [in] */ LONG PropIndex,
            /* [in] */ LONG Number);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, GetPropertyString)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyString )( 
            ISoLpu230Lock * This,
            /* [in] */ LONG PropIndex,
            /* [retval][out] */ BSTR *pString);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, SetPropertyString)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetPropertyString )( 
            ISoLpu230Lock * This,
            /* [in] */ LONG PropIndex,
            /* [in] */ BSTR bstrString);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, COFreezeEvents)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *COFreezeEvents )( 
            ISoLpu230Lock * This,
            /* [in] */ VARIANT_BOOL Freeze,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, CheckHealth)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CheckHealth )( 
            ISoLpu230Lock * This,
            /* [in] */ LONG Level,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, ClaimDevice)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ClaimDevice )( 
            ISoLpu230Lock * This,
            /* [in] */ LONG lTimeout,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, Close)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Close )( 
            ISoLpu230Lock * This,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, DirectIO)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *DirectIO )( 
            ISoLpu230Lock * This,
            /* [in] */ LONG Command,
            /* [out][in] */ LONG *pData,
            /* [out][in] */ BSTR *pString,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, ReleaseDevice)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ReleaseDevice )( 
            ISoLpu230Lock * This,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, ResetStatistics)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ResetStatistics )( 
            ISoLpu230Lock * This,
            /* [in] */ BSTR StatisticsBuffer,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, RetrieveStatistics)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *RetrieveStatistics )( 
            ISoLpu230Lock * This,
            /* [out][in] */ BSTR *StatisticsBuffer,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, UpdateStatistics)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *UpdateStatistics )( 
            ISoLpu230Lock * This,
            /* [in] */ BSTR StatisticsBuffer,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, WaitForKeylockChange)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *WaitForKeylockChange )( 
            ISoLpu230Lock * This,
            /* [in] */ LONG nKeyPosition,
            /* [in] */ LONG nTimeout,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, CompareFirmwareVersion)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CompareFirmwareVersion )( 
            ISoLpu230Lock * This,
            /* [in] */ BSTR FirmwareFileName,
            /* [out] */ LONG *pResult,
            /* [retval][out] */ LONG *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230Lock, UpdateFirmware)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *UpdateFirmware )( 
            ISoLpu230Lock * This,
            /* [in] */ BSTR FirmwareFileName,
            /* [retval][out] */ LONG *pRC);
        
        END_INTERFACE
    } ISoLpu230LockVtbl;

    interface ISoLpu230Lock
    {
        CONST_VTBL struct ISoLpu230LockVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoLpu230Lock_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISoLpu230Lock_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISoLpu230Lock_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISoLpu230Lock_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ISoLpu230Lock_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ISoLpu230Lock_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ISoLpu230Lock_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ISoLpu230Lock_OpenService(This,DeviceClass,DeviceName,pDispatch,pRC)	\
    ( (This)->lpVtbl -> OpenService(This,DeviceClass,DeviceName,pDispatch,pRC) ) 

#define ISoLpu230Lock_GetPropertyNumber(This,PropIndex,pNumber)	\
    ( (This)->lpVtbl -> GetPropertyNumber(This,PropIndex,pNumber) ) 

#define ISoLpu230Lock_SetPropertyNumber(This,PropIndex,Number)	\
    ( (This)->lpVtbl -> SetPropertyNumber(This,PropIndex,Number) ) 

#define ISoLpu230Lock_GetPropertyString(This,PropIndex,pString)	\
    ( (This)->lpVtbl -> GetPropertyString(This,PropIndex,pString) ) 

#define ISoLpu230Lock_SetPropertyString(This,PropIndex,bstrString)	\
    ( (This)->lpVtbl -> SetPropertyString(This,PropIndex,bstrString) ) 

#define ISoLpu230Lock_COFreezeEvents(This,Freeze,pRC)	\
    ( (This)->lpVtbl -> COFreezeEvents(This,Freeze,pRC) ) 

#define ISoLpu230Lock_CheckHealth(This,Level,pRC)	\
    ( (This)->lpVtbl -> CheckHealth(This,Level,pRC) ) 

#define ISoLpu230Lock_ClaimDevice(This,lTimeout,pRC)	\
    ( (This)->lpVtbl -> ClaimDevice(This,lTimeout,pRC) ) 

#define ISoLpu230Lock_Close(This,pRC)	\
    ( (This)->lpVtbl -> Close(This,pRC) ) 

#define ISoLpu230Lock_DirectIO(This,Command,pData,pString,pRC)	\
    ( (This)->lpVtbl -> DirectIO(This,Command,pData,pString,pRC) ) 

#define ISoLpu230Lock_ReleaseDevice(This,pRC)	\
    ( (This)->lpVtbl -> ReleaseDevice(This,pRC) ) 

#define ISoLpu230Lock_ResetStatistics(This,StatisticsBuffer,pRC)	\
    ( (This)->lpVtbl -> ResetStatistics(This,StatisticsBuffer,pRC) ) 

#define ISoLpu230Lock_RetrieveStatistics(This,StatisticsBuffer,pRC)	\
    ( (This)->lpVtbl -> RetrieveStatistics(This,StatisticsBuffer,pRC) ) 

#define ISoLpu230Lock_UpdateStatistics(This,StatisticsBuffer,pRC)	\
    ( (This)->lpVtbl -> UpdateStatistics(This,StatisticsBuffer,pRC) ) 

#define ISoLpu230Lock_WaitForKeylockChange(This,nKeyPosition,nTimeout,pRC)	\
    ( (This)->lpVtbl -> WaitForKeylockChange(This,nKeyPosition,nTimeout,pRC) ) 

#define ISoLpu230Lock_CompareFirmwareVersion(This,FirmwareFileName,pResult,pRC)	\
    ( (This)->lpVtbl -> CompareFirmwareVersion(This,FirmwareFileName,pResult,pRC) ) 

#define ISoLpu230Lock_UpdateFirmware(This,FirmwareFileName,pRC)	\
    ( (This)->lpVtbl -> UpdateFirmware(This,FirmwareFileName,pRC) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISoLpu230Lock_INTERFACE_DEFINED__ */



#ifndef __OposLpu230LockLib_LIBRARY_DEFINED__
#define __OposLpu230LockLib_LIBRARY_DEFINED__

/* library OposLpu230LockLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_OposLpu230LockLib;

#ifndef ___ISoLpu230LockEvents_DISPINTERFACE_DEFINED__
#define ___ISoLpu230LockEvents_DISPINTERFACE_DEFINED__

/* dispinterface _ISoLpu230LockEvents */
/* [uuid] */ 


EXTERN_C const IID DIID__ISoLpu230LockEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("8D5FF47B-7D2E-4162-8C8C-89678EFD7C28")
    _ISoLpu230LockEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _ISoLpu230LockEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ISoLpu230LockEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ISoLpu230LockEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ISoLpu230LockEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ISoLpu230LockEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ISoLpu230LockEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ISoLpu230LockEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ISoLpu230LockEvents * This,
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
        
        END_INTERFACE
    } _ISoLpu230LockEventsVtbl;

    interface _ISoLpu230LockEvents
    {
        CONST_VTBL struct _ISoLpu230LockEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _ISoLpu230LockEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _ISoLpu230LockEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _ISoLpu230LockEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _ISoLpu230LockEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _ISoLpu230LockEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _ISoLpu230LockEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _ISoLpu230LockEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___ISoLpu230LockEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_SoLpu230Lock;

#ifdef __cplusplus

class DECLSPEC_UUID("7336E808-A6C6-4E03-9A7D-02873DE93D7C")
SoLpu230Lock;
#endif
#endif /* __OposLpu230LockLib_LIBRARY_DEFINED__ */

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


