// EL_Pin_TransMgmt.h: interface for the CEL_Pin_TansMgmt class.
//////////////////////////////////////////////////////////////////////

#if !defined(__EL_PIN_TRANSACTION_MANAGMENT_H_20070706__)
#define __EL_PIN_TRANSACTION_MANAGMENT_H_20070706__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// this file is HID pinpad base IO-transaction manager.......
//////////////////////////////////////////////////////////////////////
#include "EL_Pin_TransMgmt_Base.h"

//Constants
//
/////////////////////////////////////////////////////////
enum enumHOST_IN_status	//status of host on IN phase.
{
	HINS_S000=0,	//idle
	HINS_S010,		//waits
	HINS_S111,		//receiving
	HINS_S110,		//last response
	HINS_S100		//cancel
};

enum enumHOST_IN_pin_event	//event from pinpad on IN phase of host side  
{
	HIN_UNKNOWN=0,	//not ready any event
	HIN_R000,		//busy pinad
	HIN_R001,		//starting response
	HIN_R002,		//middle response
	HIN_R003,		//last response
	HIN_R004,		//unknown starting response
	HIN_R005,		//unknown middle response
	HIN_R006,		//response of cancel-cmd
	HIN_R007,		//unknown idle response
	HIN_R008		//idle response
};

class CEL_Pin_TransMgmt : public CEL_Pin_TransMgmt_Base 
{
public:
	CEL_Pin_TransMgmt();
	virtual ~CEL_Pin_TransMgmt();

	void Initial();//equl to constructure.

	//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////
	//for device access function and exported function

	//implementation of virtual function
	//from PC to device.
	DWORD OutTransaction( enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN,unsigned char *pData=NULL,int *pnSize=NULL );

	//implementation of virtual function
	//from Device to PC.
	DWORD InTransaction( unsigned char *pData=NULL,int *pnSize=NULL );

	//implementation of virtual function
	//cancel Current transaction
	DWORD CancelTransaction();

	//implementation of virtual function
	//check current status is idle
	DWORD CheckIdleResponse();

protected:
	//implementation of virtual function
	DWORD InTransChk();
	//implementation of virtual function
	DWORD OutTransChk( enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN );
	//implementation of virtual function
	DWORD PassDummyResponse();

	enum enumHOST_IN_pin_event GetPinEvent_inPhase( PEL_PACKET_H64 pHeader,enum enumHOST_IN_status CurStatus );

	BOOL m_bIsIng;


};

#endif // !defined(__EL_PIN_TRANSACTION_MANAGMENT_H_20070706__)
