#pragma once

#include <vector>
#include <memory>
#include <deque>

#include "Sync.h"

using namespace std;
using namespace ATL;
using namespace SYNC;

class CFireEvent
{
private:
	typedef	std::vector<DISPID> VDISPID;

public:
	class CMsrEvent{
	public:
		enum EventType
		{
			TypeData,
			TypeDirectIO,
			TypeError,
			TypeOutputComplete,
			TypeStatusUpdate
		};

	public:
		~CMsrEvent(){}
		CMsrEvent( EventType Type, long lData );
		CMsrEvent( EventType Type, long lEventNumber, long *plData, BSTR *pString );
		CMsrEvent( 
			EventType Type,
			long lResultCode,
			long lResultCodeExtended,
			long lErrorLocus
		);

		void Fire(CComDispatchDriver & CCODispDrv, VDISPID & vID );
		void Fire(CComDispatchDriver & CCODispDrv, DISPID vID[] );

		long getErrorResponse(){	return m_lErrorResponse;	}

		EventType getType(){	return m_Type; }

	private:
		CMsrEvent();
		void ini();

		void Data(CComDispatchDriver & CCODispDrv, DISPID DispatchID )
		{	
			DataEvent( CCODispDrv, DispatchID, m_lStatus );
		}

		void DirectIO(CComDispatchDriver & CCODispDrv, DISPID DispatchID )
		{
			DirectIOEvent( CCODispDrv, DispatchID, m_lEventNumber, m_plData, m_pString); 
		}

		void Error( CComDispatchDriver & CCODispDrv, DISPID DispatchID )
		{
			ErrorEvent(CCODispDrv, DispatchID, m_lResultCode, m_lResultCodeExtended, m_lErrorLocus, &m_lErrorResponse); 
		}

		void OutputComplete( CComDispatchDriver & CCODispDrv, DISPID DispatchID )
		{
			OutputCompleteEvent(CCODispDrv, DispatchID, m_lOutputID); 
		}

		void StatusUpdate( CComDispatchDriver & CCODispDrv, DISPID DispatchID )
		{
			StatusUpdateEvent( CCODispDrv, DispatchID, m_lData);
		}

		static void DataEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lStatus); 
		static void DirectIOEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lEventNumber, long* plData, BSTR* pString); 
		static void ErrorEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lResultCode, long lResultCodeExtended, long lErrorLocus, long *plErrorResponse); 
		static void OutputCompleteEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lOutputID); 
		static void StatusUpdateEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lData); 

	private:
		EventType m_Type;
		
		//DataEvent
		long m_lStatus;

		//DirectIOEvent
		long m_lEventNumber;
		long *m_plData;
		BSTR *m_pString;

		//ErrorEvent
		long m_lResultCode;
		long m_lResultCodeExtended;
		long m_lErrorLocus;
		long m_lErrorResponse;

		//OutputCompleteEvent
		long m_lOutputID; 

		//StatusUpdateEvent
		long m_lData; 
	};

	typedef	std::shared_ptr<CMsrEvent>	PtrMsrEvent;

private:
	typedef	std::deque<PtrMsrEvent> DPCQ;

	enum{		CONST_EVENT_NUMBER = 5		};

	enum{
		CONST_INDEX_DISPID_DATA					=	0,
		CONST_INDEX_DISPID_DIRECTIO				=	1,
		CONST_INDEX_DISPID_ERROR					=	2,
		CONST_INDEX_DISPID_STATUSUPDATE		=	3,
		CONST_INDEX_DISPID_OUTPUTCOMPLETE	=	4
	};

	static LPOLESTR m_sEventNames[CONST_EVENT_NUMBER];

public:
	CFireEvent(void) : 
			//m_vDispatchIDs(CONST_EVENT_NUMBER)
		  m_bDataEventEnabled(true)
		  ,m_bFreezeEvents(false)
		  ,m_lLastErrorResponse(0)
		  ,m_DEventQ()
	{
		//m_vDispatchIDs.resize(CONST_EVENT_NUMBER);
		ATLASSERT( m_Mutex.IsIniOk() );

		for( int i=0; i<CONST_EVENT_NUMBER; i++ ){
			m_lDispatchIDs[i] = NULL;
		}//end for
	}

	virtual ~CFireEvent(void);

	void setDispatch(IDispatch* pDispatch);

	void setDataEventEnable( bool bEnable = true )
	{
		m_bDataEventEnabled = bEnable;
	}

	void setFreezeEvents( bool bFreeze = true )
	{
		m_bFreezeEvents = bFreeze;
	}

	bool getDataEventEnable(){	return m_bDataEventEnabled;	}
	bool getFreezeEvents(){			return m_bFreezeEvents;	}

	bool Fire( PtrMsrEvent & PtrEvt );
	bool FireQ();

	long getQSize()
	{
		long nSize;

		m_Mutex.Lock( INFINITE );
		nSize = m_DEventQ.size();
		m_Mutex.Unlock();

		return nSize;
	}

	void ClearQ()
	{
		m_Mutex.Lock( INFINITE );//300<<<<<<<<<.. change INFINITE -> 1000. version 1.8.20 (20150327)
		m_DEventQ.clear();
		m_Mutex.Unlock();
	}

	bool EmptyQ()	//add method version 1.8.21 (20150327)
	{
		bool bEmpty(false);
		m_Mutex.Lock( INFINITE );
		bEmpty = m_DEventQ.empty();
		m_Mutex.Unlock();
		return bEmpty;
	}

	long getLastErrorResponse(){	return m_lLastErrorResponse;	}

private:
	bool EnQ( const PtrMsrEvent & Event );
	bool DeQ( PtrMsrEvent &Event );

private:
	// ComQIPtr<IDispatch, &__uuidof(IDispatch)> CComDispatchDriver
	CComDispatchDriver m_CCODispDrv;

	//VDISPID m_vDispatchIDs;
	DISPID m_lDispatchIDs[CONST_EVENT_NUMBER];

	volatile bool m_bDataEventEnabled;
	volatile bool m_bFreezeEvents;

	SYNC::CMutex m_Mutex;

	DPCQ m_DEventQ;	//DPC queue

	long m_lLastErrorResponse;
};

