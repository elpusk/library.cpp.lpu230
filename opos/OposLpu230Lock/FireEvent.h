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
	class CKeyEvent{
	public:
		enum EventType
		{
			TypeDirectIO,
			TypeStatusUpdate
		};

	public:
		~CKeyEvent(){}
		CKeyEvent( EventType Type, long lData );
		CKeyEvent( EventType Type, long lEventNumber, long *plData, BSTR *pString );
		CKeyEvent( 
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
		CKeyEvent();
		void ini();

		void DirectIO(CComDispatchDriver & CCODispDrv, DISPID DispatchID )
		{
			DirectIOEvent( CCODispDrv, DispatchID, m_lEventNumber, m_plData, m_pString); 
		}

		void StatusUpdate( CComDispatchDriver & CCODispDrv, DISPID DispatchID )
		{
			StatusUpdateEvent( CCODispDrv, DispatchID, m_lStatus);
		}

		static void DirectIOEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lEventNumber, long* plData, BSTR* pString); 
		static void StatusUpdateEvent( CComDispatchDriver & CCODispDrv, DISPID DispatchID, long lData); 

	private:
		EventType m_Type;
		
		//DirectIOEvent
		long m_lEventNumber;
		long *m_plData;
		BSTR *m_pString;

		//ErrorEvent
		long m_lResultCode;
		long m_lResultCodeExtended;
		long m_lErrorLocus;
		long m_lErrorResponse;

		//StatusUpdateEvent
		long m_lStatus;
	};

	typedef	std::shared_ptr<CKeyEvent>	PtrKeyEvent;

private:
	typedef	std::deque<PtrKeyEvent> DPCQ;

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
		  m_bFreezeEvents(false)
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

	void setFreezeEvents( bool bFreeze = true )
	{
		m_bFreezeEvents = bFreeze;

		if( !m_bFreezeEvents ){
			while(FireQ());
		}
	}

	bool getFreezeEvents(){			return m_bFreezeEvents;	}

	bool Fire( PtrKeyEvent & PtrEvt );
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
	bool EnQ( const PtrKeyEvent & Event );
	bool DeQ( PtrKeyEvent &Event );

private:
	// ComQIPtr<IDispatch, &__uuidof(IDispatch)> CComDispatchDriver
	CComDispatchDriver m_CCODispDrv;

	//VDISPID m_vDispatchIDs;
	DISPID m_lDispatchIDs[CONST_EVENT_NUMBER];

	volatile bool m_bFreezeEvents;

	SYNC::CMutex m_Mutex;

	DPCQ m_DEventQ;	//DPC queue

	long m_lLastErrorResponse;
};

