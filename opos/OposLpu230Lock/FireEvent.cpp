#include "StdAfx.h"
#include "FireEvent.h"
#include <Opos.h>

///////////////////////////////
// CKeyEvent class
void CFireEvent::CKeyEvent::ini()
{
	m_lStatus = m_lEventNumber = 0;
	m_plData =	NULL;
	m_pString = NULL;
	m_lResultCode = m_lResultCodeExtended =m_lErrorLocus = 0;
	m_lErrorResponse = 0;
}

CFireEvent::CKeyEvent::CKeyEvent( EventType Type, long lEventNumber, long *plData, BSTR *pString )
	 : m_Type(Type)
{
	ini();

	//DirectIOEvent 
	m_lEventNumber = lEventNumber;
	m_plData = plData;
	m_pString = pString;
}

CFireEvent::CKeyEvent::CKeyEvent( 
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

CFireEvent::CKeyEvent::CKeyEvent( EventType Type, long lData ) : m_Type(Type)
{
	ini();
	switch( Type ){
		case TypeStatusUpdate:	m_lStatus = lData;		break;
		default:
			break;
	}//end switch
}

void CFireEvent::CKeyEvent::DirectIOEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lEventNumber, long* plData, BSTR* pString)
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

void CFireEvent::CKeyEvent::StatusUpdateEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lData)
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

void CFireEvent::CKeyEvent::Fire(CComDispatchDriver & CCODispDrv, VDISPID & vID )
{
	switch( this->m_Type ){
		case TypeDirectIO:
			DirectIO( CCODispDrv, vID[CONST_INDEX_DISPID_DIRECTIO] );
			break;
		case TypeStatusUpdate:
			StatusUpdate( CCODispDrv, vID[CONST_INDEX_DISPID_STATUSUPDATE] );
			break;
		default:
			break;
	}//end switch
}


void CFireEvent::CKeyEvent::Fire(CComDispatchDriver & CCODispDrv, DISPID vID[] )
{
	switch( this->m_Type ){
		case TypeDirectIO:
			DirectIO( CCODispDrv, vID[CONST_INDEX_DISPID_DIRECTIO] );
			break;
		case TypeStatusUpdate:
			StatusUpdate( CCODispDrv, vID[CONST_INDEX_DISPID_STATUSUPDATE] );
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

	if( !m_bFreezeEvents ){

		//processing DPC.......
		PtrKeyEvent Evt;

		//if( !m_DEventQ.empty() ){//remark version 1.8.21 (20150327)
		if( 	!EmptyQ() ){	//add  version 1.8.21 (20150327)
			if( DeQ( Evt ) ){
				ATLTRACE( _T("FireQ : DeQ\n") );

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

bool CFireEvent::Fire( PtrKeyEvent & PtrEvt )
{
	bool bFired = false;

	if( !m_bFreezeEvents ){
		PtrKeyEvent Evt;//processing DPC.......

		if( 	!EmptyQ() ){	//add version 1.8.21 (20150327)
			if( DeQ( Evt ) ){

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
			PtrEvt->Fire( m_CCODispDrv, m_lDispatchIDs );
			m_lLastErrorResponse = PtrEvt->getErrorResponse();
			bFired = true;
		}
	}
	else{
		//enqueue request to DPC Q
		EnQ( PtrEvt );	//for DPC processing, enqueue
	}

	return bFired;
}

bool CFireEvent::EnQ( const PtrKeyEvent & Event )
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

bool CFireEvent::DeQ( PtrKeyEvent &Event )
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

