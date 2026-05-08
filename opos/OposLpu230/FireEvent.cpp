#include "StdAfx.h"
#include "FireEvent.h"
#include <Opos.h>

///////////////////////////////
// CMsrEvent class
void CFireEvent::CMsrEvent::ini()
{
	m_lStatus = m_lEventNumber = 0;
	m_plData =	NULL;
	m_pString = NULL;
	m_lResultCode = m_lResultCodeExtended =m_lErrorLocus = 0;
	m_lErrorResponse = 0;
	m_lOutputID = m_lData =0;
}

CFireEvent::CMsrEvent::CMsrEvent( EventType Type, long lEventNumber, long *plData, BSTR *pString )
	 : m_Type(Type)
{
	ini();

	//DirectIOEvent 
	m_lEventNumber = lEventNumber;
	m_plData = plData;
	m_pString = pString;
}

CFireEvent::CMsrEvent::CMsrEvent( 
	EventType Type,
	long lResultCode,
	long lResultCodeExtended,
	long lErrorLocus
	)
	: m_Type(Type)
{
	ini();

	//ErrorEvent
	m_lResultCode = lResultCode;
	m_lResultCodeExtended = lResultCodeExtended;
	m_lErrorLocus = lErrorLocus;
}

CFireEvent::CMsrEvent::CMsrEvent( EventType Type, long lData ) : m_Type(Type)
{
	ini();
	switch( Type ){
		case TypeData:	m_lStatus = lData;	break;
		case TypeOutputComplete:	m_lOutputID = lData;	break;
		case TypeStatusUpdate:	m_lData = lData;		break;
		default:
			break;
	}//end switch
}

void CFireEvent::CMsrEvent::DataEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lStatus)
{
	VARIANT Result, Vars;   
	Vars.lVal = lStatus;   
	Vars.vt = VT_I4;   
	DISPPARAMS Disp = {&Vars, NULL, 1, 0};   
	HRESULT hRC = CCODispDrv->Invoke(DispatchID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &Disp, &Result, NULL, NULL);   
	if (hRC != S_OK){
		ATLTRACE(_T("E : firing DataEvent.\n") );
	}
	else{
		ATLTRACE(_T("S : firing DataEvent.\n") );
	}
}

void CFireEvent::CMsrEvent::DirectIOEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lEventNumber, long* plData, BSTR* pString)
{
	VARIANT Result, Vars[3];   
	Vars[0].lVal = lEventNumber;   
	Vars[0].vt = VT_I4;   
	Vars[1].plVal = plData;   
	Vars[1].vt = VT_I4 | VT_BYREF;   
	Vars[2].pbstrVal = pString;   
	Vars[2].vt = VT_BSTR | VT_BYREF;   
	DISPPARAMS Disp = {&Vars[0], NULL, 3, 0};   
	HRESULT hRC = CCODispDrv->Invoke(DispatchID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &Disp, &Result, NULL, NULL);   
	if (hRC != S_OK){
		ATLTRACE(_T("E : firing DirectIOEvent.\n") );
	}
	else{
		ATLTRACE(_T("S : firing DirectIOEvent.\n") );
	}
}

void CFireEvent::CMsrEvent::ErrorEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lResultCode, long lResultCodeExtended, long lErrorLocus, long *plErrorResponse)
{
	*plErrorResponse = OPOS_ER_CLEAR;

	VARIANT Result, Vars[4];   
	Vars[3].lVal = lResultCode;   
	Vars[3].vt = VT_I4;

	Vars[2].lVal = lResultCodeExtended;   
	Vars[2].vt = VT_I4;

	Vars[1].lVal = lErrorLocus;   
	Vars[1].vt = VT_I4;

	Vars[0].plVal = plErrorResponse;   
	Vars[0].vt = VT_I4 | VT_BYREF;   
	DISPPARAMS Disp = {&Vars[0], NULL, 4, 0};   
	HRESULT hRC = CCODispDrv->Invoke(DispatchID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &Disp, &Result, NULL, NULL);   
	if (hRC != S_OK){
		ATLTRACE(_T("E : firing ErrorEvent.\n") );
	}
	else{
		ATLTRACE(_T("S : firing ErrorEvent.\n") );
	}
}

void CFireEvent::CMsrEvent::OutputCompleteEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lOutputID)
{
	VARIANT Result, Vars;   
	Vars.lVal = lOutputID;   
	Vars.vt = VT_I4;   
	DISPPARAMS Disp = {&Vars, NULL, 1, 0};   
	HRESULT hRC = CCODispDrv->Invoke(DispatchID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &Disp, &Result, NULL, NULL);   
	if (hRC != S_OK){
		ATLTRACE(_T("E : firing OutputCompleteEvent.\n") );
	}
	else{
		ATLTRACE(_T("S : firing OutputCompleteEvent.\n") );
	}
}

void CFireEvent::CMsrEvent::StatusUpdateEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lData)
{
	VARIANT Result, Vars;   
	Vars.lVal = lData;   
	Vars.vt = VT_I4;   
	DISPPARAMS Disp = {&Vars, NULL, 1, 0};   
	HRESULT hRC = CCODispDrv->Invoke(DispatchID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &Disp, &Result, NULL, NULL);   
	if (hRC != S_OK){
		ATLTRACE(_T("E : firing StatusUpdateEvent.\n") );
	}
	else{
		ATLTRACE(_T("S : firing StatusUpdateEvent.\n") );
	}
}

void CFireEvent::CMsrEvent::Fire(CComDispatchDriver & CCODispDrv, VDISPID & vID )
{
	switch( this->m_Type ){
		case TypeData:
			Data( CCODispDrv, vID[CONST_INDEX_DISPID_DATA] );
			break;
		case TypeDirectIO:
			DirectIO( CCODispDrv, vID[CONST_INDEX_DISPID_DIRECTIO] );
			break;
		case TypeError:
			Error( CCODispDrv, vID[CONST_INDEX_DISPID_ERROR] );
			break;
		case TypeOutputComplete:
			OutputComplete( CCODispDrv, vID[CONST_INDEX_DISPID_STATUSUPDATE] );
			break;
		case TypeStatusUpdate:
			StatusUpdate( CCODispDrv, vID[CONST_INDEX_DISPID_OUTPUTCOMPLETE] );
			break;
		default:
			break;
	}//end switch
}


void CFireEvent::CMsrEvent::Fire(CComDispatchDriver & CCODispDrv, DISPID vID[] )
{
	switch( this->m_Type ){
		case TypeData:
			Data( CCODispDrv, vID[CONST_INDEX_DISPID_DATA] );
			break;
		case TypeDirectIO:
			DirectIO( CCODispDrv, vID[CONST_INDEX_DISPID_DIRECTIO] );
			break;
		case TypeError:
			Error( CCODispDrv, vID[CONST_INDEX_DISPID_ERROR] );
			break;
		case TypeOutputComplete:
			OutputComplete( CCODispDrv, vID[CONST_INDEX_DISPID_STATUSUPDATE] );
			break;
		case TypeStatusUpdate:
			StatusUpdate( CCODispDrv, vID[CONST_INDEX_DISPID_OUTPUTCOMPLETE] );
			break;
		default:
			break;
	}//end switch
}

///////////////////////////////
// CFireEvent class
LPOLESTR CFireEvent::m_sEventNames[] = {   
    L"SOData",   
    L"SODirectIO",   
    L"SOError",   
    L"SOStatusUpdate",   
    L"SOOutputComplete"   
}; 

CFireEvent::~CFireEvent(void)
{
	m_DEventQ.clear();
	m_CCODispDrv.Detach();
}



void CFireEvent::setDispatch(IDispatch* pDispatch)
{
	m_CCODispDrv = pDispatch;

	HRESULT hResult;
	for( int i=0; i<CONST_EVENT_NUMBER; i++ ){
		//hResult = pDispatch->GetIDsOfNames(IID_NULL, &m_sEventNames[i], 1, LOCALE_SYSTEM_DEFAULT, &m_vDispatchIDs[i]);
		hResult = pDispatch->GetIDsOfNames(IID_NULL, &m_sEventNames[i], 1, LOCALE_SYSTEM_DEFAULT, &m_lDispatchIDs[i]);
		if( hResult != S_OK ){
			//m_vDispatchIDs[i] = NULL;
			m_lDispatchIDs[i] = NULL;
		}
	}//end for
}

bool CFireEvent::FireQ()
{
	bool bFired = false;
	//m_Mutex.Lock( INFINITE ); //remark version 1.8.21 (20150327)

	if( m_bDataEventEnabled && !m_bFreezeEvents ){

		//processing DPC.......
		PtrMsrEvent Evt;

		//if( !m_DEventQ.empty() ){//remark version 1.8.21 (20150327)
		if( 	!EmptyQ() ){	//add  version 1.8.21 (20150327)
			if( DeQ( Evt ) ){
				ATLTRACE( _T("FireQ : DeQ\n") );

				if( Evt->getType() != CMsrEvent::TypeError )
					m_bDataEventEnabled = false;
				//Evt->Fire( m_CCODispDrv, m_vDispatchIDs );
				Evt->Fire( m_CCODispDrv, m_lDispatchIDs );
				m_lLastErrorResponse = Evt->getErrorResponse();
				bFired = true;
			}

		}
	}

	//m_Mutex.Unlock(); //remark version 1.8.21 (20150327)

	return bFired;
}

bool CFireEvent::Fire( PtrMsrEvent & PtrEvt )
{
	bool bFired = false;

	//m_Mutex.Lock( INFINITE ); // remark version 1.8.21 (20150327)

	if( m_bDataEventEnabled && !m_bFreezeEvents ){

		//processing DPC.......
		PtrMsrEvent Evt;

		//if( !m_DEventQ.empty() ){ // remark version 1.8.21 (20150327)
		if( 	EmptyQ() ){	//add version 1.8.21 (20150327)
			if( DeQ( Evt ) ){

				if( Evt->getType() != CMsrEvent::TypeError )
					m_bDataEventEnabled = false;
				//Evt->Fire( m_CCODispDrv, m_vDispatchIDs );
				Evt->Fire( m_CCODispDrv, m_lDispatchIDs );
				m_lLastErrorResponse = Evt->getErrorResponse();
				bFired = true;
			}
			//enqueue request to DPC Q
			EnQ( PtrEvt );	//for DPC processing, enqueue
		}
		else{
			//processing new request
			m_bDataEventEnabled = false;
			//PtrEvt->Fire( m_CCODispDrv, m_vDispatchIDs );
			PtrEvt->Fire( m_CCODispDrv, m_lDispatchIDs );
			m_lLastErrorResponse = PtrEvt->getErrorResponse();
			bFired = true;
		}
	}
	else{
		//enqueue request to DPC Q
		EnQ( PtrEvt );	//for DPC processing, enqueue
	}

	//m_Mutex.Unlock(); // remark version 1.8.21 (20150327)

	return bFired;
}

bool CFireEvent::EnQ( const PtrMsrEvent & Event )
{
	m_Mutex.Lock( INFINITE );//add version 1.8.21 (20150327)
	if( !m_DEventQ.empty() ){
		//m_DEventQ.pop_front();
		m_DEventQ.clear();
	}
	//
	m_DEventQ.push_back( Event );
	m_Mutex.Unlock();//add version 1.8.21 (20150327)
	return true;
}

bool CFireEvent::DeQ( PtrMsrEvent &Event )
{
	bool bResult = true;

	m_Mutex.Lock( INFINITE );//add version 1.8.21 (20150327)
	if( !m_DEventQ.empty() ){
		Event = m_DEventQ.front();
		m_DEventQ.pop_front();
	}
	else
		bResult = false;
	m_Mutex.Unlock();//add version 1.8.21 (20150327)

	return bResult;
}

