// EL_Pin_Cmd_Base.h: interface for the CEL_Pin_Cmd_Base class.
// this header command base of all elpusk pinpad.......
//////////////////////////////////////////////////////////////////////

#if !defined(__EL_PIN_IO_CMD_H_20070712__)
#define __EL_PIN_IO_CMD_H_20070712__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EL_Pin_TransMgmt_Base.h"
#include "EL_Pin_TransMgmt.h"
#include "EL_Pin_TransMgmt_Vender.h"
#include "EL_Pin_MisH.h"

//internal constants
#define	EVENT_WAIT_DEFAULT_TIME				2000	//2000msec
//#define	WOKER_WAIT_DEFAULT_TIME				30		//30msec
#define	WOKER_WAIT_DEFAULT_TIME				1000		//30msec

#define	PROTECT_DEFAULT_TIME_OUT			2000	//2000msec
#define	MAX_RX_BUFFER_SIZE					4096

//Thread input parameter
typedef	struct TagWokerThreadItem{
		HANDLE hKillMe;		//for event handle to kill the thread 
							//created by main thread

		//woker will notify that woker executs the last phase of dead, to main thread 
		//this event is created by woker thread. 
		HANDLE hIamKilled;	

		HANDLE hStartUp;//Start up event
						//work thread notify self-Start-up to main thread.
						//this event is created bymain thread 

		HANDLE hRequest;//request event handle
						//created by main thread.
						//main thread requests a processing kby this event

		//this is mutex name that indicate main thread is alive
		char sMainALiveMutexName[_MAX_PATH];//

		char sIamKilledEventName[_MAX_PATH];//

		LPVOID lpUserPara;//may be user data by callback function......

} WokerThreadItem, *PWokerThreadItem,*LPWokerThreadItem;

//this structure is used by between main thread and worker thread
typedef	struct TagParamterItem{
	int nInSize;//input data size,from device
	unsigned char *pInData;//input data
	int nOutSize;//Out data size,from app
	unsigned char *pOutData;//Output data

	BOOL bUsedFile;	//hFileData file is used to data field.
	HANDLE hFileData;//read only file handle
	//worker notify end of processing to mail thread
	HANDLE hNotifyEvent;

	//for windows message type
	HWND hTargetWnd;
	UINT uTargetMsg;

	//for callback type
	CallbackFunType lpCBFun;
	LPVOID lpUserPara;//may be user data by callback function......

	DWORD dwResult;//the result of processing

	enum enumPin_Trans_Type TransType;//the type of transaction.......

} ParamterItem, *PParamterItem,*LPParamterItem;

class CEL_Pin_Cmd_Base  
{
public:
	CEL_Pin_Cmd_Base();
	virtual ~CEL_Pin_Cmd_Base();

	void Initial();//call by constructure
	void Destory();//call by destructure

	//////////////////////////////////////////////////////////////////
	//for device access function and exported function
	//base function
	DWORD Open( char *sDevPath=NULL ){	//open device
		return m_pTransObj->Open( sDevPath );
	};
	
	DWORD Close(){	//close current opened device
		return m_pTransObj->Close();
	};

	DWORD Write( unsigned char *pData=NULL,int *pnSize=NULL ){
		return m_pTransObj->Write( pData,pnSize );
	};
	DWORD Read( unsigned char *pData=NULL,int *pnSize=NULL ){
		return m_pTransObj->Read( pData,pnSize );
	};

	//command base function as function-type 
	DWORD SyncFun( BYTE *lpdata,DWORD dwSize,enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN );//execute sync-type function
	DWORD SyncFun( PEL_PACKET_H64 pHeader,char *sFileName,enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN );//execute sync-type function

	DWORD AsyncFun_Pol( BYTE *lpdata,DWORD dwSize,enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN );//execute Async-type function by polling method
	DWORD AsyncFun_Pol( PEL_PACKET_H64 pHeader,char *sFileName,enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN );//execute Async-type function by polling method

	DWORD AsyncFun_Msg( BYTE *lpdata,DWORD dwSize,HWND hTarget,UINT uMsg,enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN );//execute sync-type function by windows Messaging method
	DWORD AsyncFun_Msg( PEL_PACKET_H64 pHeader,char *sFileName,HWND hTarget,UINT uMsg,enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN );//execute sync-type function by windows Messaging method

	DWORD AsyncFun_CB( BYTE *lpdata,DWORD dwSize,CallbackFunType lpFun,LPVOID lpUserPara,enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN );//execute sync-type function by callback method
	DWORD AsyncFun_CB( PEL_PACKET_H64 pHeader,char *sFileName,CallbackFunType lpFun,LPVOID lpUserPara,enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN );//execute sync-type function by callback method

	DWORD Cancel();	//cancel current processing.......

	//get result of command processing by sync asynac type function
	DWORD GetResult(  BYTE *lpdata,DWORD *pdwSize );

	HANDLE GetCurHandle()
	{	return m_pTransObj->GetCurHandle();	};

	int GetDevList( char *psDevList )
	{	return m_pTransObj->GetDevList( psDevList );	};

	int GetDevListEx( char *psDevList,enum DrvType Type )
	{	return m_pTransObj->GetDevListEx( psDevList,Type );	};
	///////////////////////////////////////////////////////////////////

protected:

	BOOL CopyInPara( PParamterItem pDesItem );

	ParamterItem m_InParaItem;
	static UINT	WokerThread(LPVOID pParam);
	HANDLE m_hInOkEvent;//worker notifiy ok signal to main-thread after accepting command
	static void NotifyResult( CEL_Pin_Cmd_Base *pObj,PParamterItem pCurItem,DWORD dwResult );

	unsigned char *m_pInParaBuf;
	int m_nRXSize;
	unsigned char m_pRX[MAX_RX_BUFFER_SIZE];
	DWORD m_dwResult;

	HANDLE m_hWThreadhandle;
	DWORD m_dwThID;

	HANDLE	m_hMutexProtect;//protects m_dwResult,m_nRXSize and m_pRX member.......
	HANDLE m_hRequestMutex;
	HANDLE m_hRequestEvent;
	HANDLE m_hWThreadKillEvent;
	HANDLE m_hWThreadDeadEvent;
	char m_sDeadEventName[_MAX_PATH];

	BOOL CreateWokerThread();
	BOOL TerminateWokerThread();

	CEL_Pin_TransMgmt_Base *m_pTransObj;//pinpad object with transaction control

	char m_sMainAliveMutexName[_MAX_PATH];
	HANDLE m_hMainAliveMutex;

	BOOL LockProtect( DWORD dwTimeOut=INFINITE );
	BOOL ReleaseProtect();

};

#endif // !defined(__EL_PIN_IO_CMD_H_20070712__)
