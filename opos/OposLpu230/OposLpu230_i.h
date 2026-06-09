

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for OposLpu230.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
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

#ifndef __OposLpu230_i_h__
#define __OposLpu230_i_h__

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

#ifndef __ISoLpu230_FWD_DEFINED__
#define __ISoLpu230_FWD_DEFINED__
typedef interface ISoLpu230 ISoLpu230;

#endif 	/* __ISoLpu230_FWD_DEFINED__ */


#ifndef ___ISoLpu230Events_FWD_DEFINED__
#define ___ISoLpu230Events_FWD_DEFINED__
typedef interface _ISoLpu230Events _ISoLpu230Events;

#endif 	/* ___ISoLpu230Events_FWD_DEFINED__ */


#ifndef __SoLpu230_FWD_DEFINED__
#define __SoLpu230_FWD_DEFINED__

#ifdef __cplusplus
typedef class SoLpu230 SoLpu230;
#else
typedef struct SoLpu230 SoLpu230;
#endif /* __cplusplus */

#endif 	/* __SoLpu230_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ISoLpu230_INTERFACE_DEFINED__
#define __ISoLpu230_INTERFACE_DEFINED__

/* interface ISoLpu230 */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ISoLpu230;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F03BACCD-1CC1-4F07-A38D-EBE531D74BC0")
    ISoLpu230 : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenService( 
            /* [in] */ BSTR DeviceClass,
            /* [in] */ BSTR DeviceName,
            /* [in] */ IDispatch *pDispatch,
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPropertyNumber( 
            /* [in] */ long PropIndex,
            /* [retval][out] */ long *pNumber) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetPropertyNumber( 
            /* [in] */ long PropIndex,
            /* [in] */ long Number) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPropertyString( 
            /* [in] */ long PropIndex,
            /* [retval][out] */ BSTR *pString) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetPropertyString( 
            /* [in] */ long PropIndex,
            /* [in] */ BSTR bstrString) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetOpenResult( 
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE COFreezeEvents( 
            /* [in] */ VARIANT_BOOL Freeze,
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CheckHealth( 
            /* [in] */ long Level,
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ClaimDevice( 
            /* [in] */ long lTimeout,
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ClearInput( 
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Close( 
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DirectIO( 
            /* [in] */ long Command,
            /* [out][in] */ long *pData,
            /* [out][in] */ BSTR *pString,
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReleaseDevice( 
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ResetStatistics( 
            /* [in] */ BSTR StatisticsBuffer,
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RetrieveStatistics( 
            /* [out][in] */ BSTR *StatisticsBuffer,
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UpdateStatistics( 
            /* [in] */ BSTR StatisticsBuffer,
            /* [retval][out] */ long *pRC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UpdateKey( 
            /* [in] */ BSTR Key,
            /* [in] */ BSTR KeyName,
            /* [retval][out] */ long *pRC) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ISoLpu230Vtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoLpu230 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoLpu230 * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoLpu230 * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISoLpu230 * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISoLpu230 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISoLpu230 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISoLpu230 * This,
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
        
        DECLSPEC_XFGVIRT(ISoLpu230, OpenService)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OpenService )( 
            ISoLpu230 * This,
            /* [in] */ BSTR DeviceClass,
            /* [in] */ BSTR DeviceName,
            /* [in] */ IDispatch *pDispatch,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, GetPropertyNumber)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyNumber )( 
            ISoLpu230 * This,
            /* [in] */ long PropIndex,
            /* [retval][out] */ long *pNumber);
        
        DECLSPEC_XFGVIRT(ISoLpu230, SetPropertyNumber)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetPropertyNumber )( 
            ISoLpu230 * This,
            /* [in] */ long PropIndex,
            /* [in] */ long Number);
        
        DECLSPEC_XFGVIRT(ISoLpu230, GetPropertyString)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyString )( 
            ISoLpu230 * This,
            /* [in] */ long PropIndex,
            /* [retval][out] */ BSTR *pString);
        
        DECLSPEC_XFGVIRT(ISoLpu230, SetPropertyString)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetPropertyString )( 
            ISoLpu230 * This,
            /* [in] */ long PropIndex,
            /* [in] */ BSTR bstrString);
        
        DECLSPEC_XFGVIRT(ISoLpu230, GetOpenResult)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetOpenResult )( 
            ISoLpu230 * This,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, COFreezeEvents)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *COFreezeEvents )( 
            ISoLpu230 * This,
            /* [in] */ VARIANT_BOOL Freeze,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, CheckHealth)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CheckHealth )( 
            ISoLpu230 * This,
            /* [in] */ long Level,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, ClaimDevice)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ClaimDevice )( 
            ISoLpu230 * This,
            /* [in] */ long lTimeout,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, ClearInput)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ClearInput )( 
            ISoLpu230 * This,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, Close)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Close )( 
            ISoLpu230 * This,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, DirectIO)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DirectIO )( 
            ISoLpu230 * This,
            /* [in] */ long Command,
            /* [out][in] */ long *pData,
            /* [out][in] */ BSTR *pString,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, ReleaseDevice)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ReleaseDevice )( 
            ISoLpu230 * This,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, ResetStatistics)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ResetStatistics )( 
            ISoLpu230 * This,
            /* [in] */ BSTR StatisticsBuffer,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, RetrieveStatistics)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RetrieveStatistics )( 
            ISoLpu230 * This,
            /* [out][in] */ BSTR *StatisticsBuffer,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, UpdateStatistics)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *UpdateStatistics )( 
            ISoLpu230 * This,
            /* [in] */ BSTR StatisticsBuffer,
            /* [retval][out] */ long *pRC);
        
        DECLSPEC_XFGVIRT(ISoLpu230, UpdateKey)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *UpdateKey )( 
            ISoLpu230 * This,
            /* [in] */ BSTR Key,
            /* [in] */ BSTR KeyName,
            /* [retval][out] */ long *pRC);
        
        END_INTERFACE
    } ISoLpu230Vtbl;

    interface ISoLpu230
    {
        CONST_VTBL struct ISoLpu230Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoLpu230_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISoLpu230_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISoLpu230_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISoLpu230_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ISoLpu230_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ISoLpu230_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ISoLpu230_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ISoLpu230_OpenService(This,DeviceClass,DeviceName,pDispatch,pRC)	\
    ( (This)->lpVtbl -> OpenService(This,DeviceClass,DeviceName,pDispatch,pRC) ) 

#define ISoLpu230_GetPropertyNumber(This,PropIndex,pNumber)	\
    ( (This)->lpVtbl -> GetPropertyNumber(This,PropIndex,pNumber) ) 

#define ISoLpu230_SetPropertyNumber(This,PropIndex,Number)	\
    ( (This)->lpVtbl -> SetPropertyNumber(This,PropIndex,Number) ) 

#define ISoLpu230_GetPropertyString(This,PropIndex,pString)	\
    ( (This)->lpVtbl -> GetPropertyString(This,PropIndex,pString) ) 

#define ISoLpu230_SetPropertyString(This,PropIndex,bstrString)	\
    ( (This)->lpVtbl -> SetPropertyString(This,PropIndex,bstrString) ) 

#define ISoLpu230_GetOpenResult(This,pRC)	\
    ( (This)->lpVtbl -> GetOpenResult(This,pRC) ) 

#define ISoLpu230_COFreezeEvents(This,Freeze,pRC)	\
    ( (This)->lpVtbl -> COFreezeEvents(This,Freeze,pRC) ) 

#define ISoLpu230_CheckHealth(This,Level,pRC)	\
    ( (This)->lpVtbl -> CheckHealth(This,Level,pRC) ) 

#define ISoLpu230_ClaimDevice(This,lTimeout,pRC)	\
    ( (This)->lpVtbl -> ClaimDevice(This,lTimeout,pRC) ) 

#define ISoLpu230_ClearInput(This,pRC)	\
    ( (This)->lpVtbl -> ClearInput(This,pRC) ) 

#define ISoLpu230_Close(This,pRC)	\
    ( (This)->lpVtbl -> Close(This,pRC) ) 

#define ISoLpu230_DirectIO(This,Command,pData,pString,pRC)	\
    ( (This)->lpVtbl -> DirectIO(This,Command,pData,pString,pRC) ) 

#define ISoLpu230_ReleaseDevice(This,pRC)	\
    ( (This)->lpVtbl -> ReleaseDevice(This,pRC) ) 

#define ISoLpu230_ResetStatistics(This,StatisticsBuffer,pRC)	\
    ( (This)->lpVtbl -> ResetStatistics(This,StatisticsBuffer,pRC) ) 

#define ISoLpu230_RetrieveStatistics(This,StatisticsBuffer,pRC)	\
    ( (This)->lpVtbl -> RetrieveStatistics(This,StatisticsBuffer,pRC) ) 

#define ISoLpu230_UpdateStatistics(This,StatisticsBuffer,pRC)	\
    ( (This)->lpVtbl -> UpdateStatistics(This,StatisticsBuffer,pRC) ) 

#define ISoLpu230_UpdateKey(This,Key,KeyName,pRC)	\
    ( (This)->lpVtbl -> UpdateKey(This,Key,KeyName,pRC) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISoLpu230_INTERFACE_DEFINED__ */



#ifndef __OposLpu230Lib_LIBRARY_DEFINED__
#define __OposLpu230Lib_LIBRARY_DEFINED__

/* library OposLpu230Lib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_OposLpu230Lib;

#ifndef ___ISoLpu230Events_DISPINTERFACE_DEFINED__
#define ___ISoLpu230Events_DISPINTERFACE_DEFINED__

/* dispinterface _ISoLpu230Events */
/* [uuid] */ 


EXTERN_C const IID DIID__ISoLpu230Events;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("BB42FD0A-547F-4F10-8661-A79D67F4697E")
    _ISoLpu230Events : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _ISoLpu230EventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ISoLpu230Events * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ISoLpu230Events * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ISoLpu230Events * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ISoLpu230Events * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ISoLpu230Events * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ISoLpu230Events * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ISoLpu230Events * This,
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
    } _ISoLpu230EventsVtbl;

    interface _ISoLpu230Events
    {
        CONST_VTBL struct _ISoLpu230EventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _ISoLpu230Events_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _ISoLpu230Events_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _ISoLpu230Events_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _ISoLpu230Events_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _ISoLpu230Events_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _ISoLpu230Events_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _ISoLpu230Events_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___ISoLpu230Events_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_SoLpu230;

#ifdef __cplusplus

class DECLSPEC_UUID("84DCB695-74B1-4B09-A6F3-8E374304B9D6")
SoLpu230;
#endif
#endif /* __OposLpu230Lib_LIBRARY_DEFINED__ */

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


