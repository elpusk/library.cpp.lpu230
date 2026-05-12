// EL_Pin_TransMgmt_Vender.h: interface for the CEL_Pin_TansMgmt class.
//////////////////////////////////////////////////////////////////////

#if !defined(__EL_PIN_TRANSACTION_MANAGMENT_VENDER_H_20071008__)
#define __EL_PIN_TRANSACTION_MANAGMENT_VENDER_H_20071008__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// this file is vender defined pinpad base IO-transaction manager.......
//////////////////////////////////////////////////////////////////////
#include "EL_Pin_TransMgmt_Base.h"

//Constants

//
/////////////////////////////////////////////////////////
enum enumV_HOST_IN_status	//status of host on IN phase.
{
	V_HINS_STATUS_IDLE=0,				//check only command Queue
	V_HINS_STATUS_CHECK_LINE,			//Check command Queue and USB line
	V_HINS_GETPIN_RESPONSE,				//check command Queue,GetPin,GetPinEx
										//and GetPinExAuto response check
	V_HINS_STATUS_RECOVER_LINE=100,		//Recover usb line
	V_HINS_STATUS_RECOVER_LINE_RETRY	//Recover usb line and retry
										//last GetPinEx or GetPinExAuto
};

enum enumV_HOST_IN_pin_event	//event from pinpad on IN phase of host side  
{
	V_HIN_UNKNOWN=0,	//not ready any event
	V_HIN_IDLE=0,		//idle 
	V_HIN_PIN='I',		//pin response
	V_HIN_ACCOUNT='A',	//account response
	V_HIN_ICC_ST=0x50,	//ICC in/out response
	V_HIN_DIS_MSG='L',	//display message response
	V_HIN_VER='V'		//version response
};

class CEL_Pin_TransMgmt_Vender : public CEL_Pin_TransMgmt_Base 
{
public:
	CEL_Pin_TransMgmt_Vender();
	virtual ~CEL_Pin_TransMgmt_Vender();

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
	//cancel Current transaction.
	DWORD CancelTransaction();

	//implementation of virtual function
	//check current status is idle.
	DWORD CheckIdleResponse();

protected:
	//implementation of virtual function
	DWORD InTransChk();
	//implementation of virtual function
	DWORD OutTransChk( enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN );
	//implementation of virtual function
	DWORD PassDummyResponse();

	enum enumV_HOST_IN_pin_event GetPinEvent_inPhase( PEL_PACKET_H64 pHeader,enum enumV_HOST_IN_status CurStatus );

	BOOL m_bIsIng;

};

#endif // !defined(__EL_PIN_TRANSACTION_MANAGMENT_VENDER_H_20071008__)
