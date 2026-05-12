
#if !defined(__TTRL_SHARE_ITEM_HEADER_20091110__)
#define __TTRL_SHARE_ITEM_HEADER_20091110__

///////////////////////////////////////////////
//TTR serise Shared circuler queue header file
//////////////////////////////////////////////	
#include "TTR_def.h"


#define	TTR_SHARE_ITEM_NAME_MAX_SIZE			TTR_DEF_EVENT_NAME_SIZE
#define	TTR_SHARE_ITEM_DEF_BUF_SIZE				512
#define	TTR_SHARE_ITEM_BIG_BUF_SIZE				3264
#define	TTR_SHARE_ITEM_PARA_NUM					4


///////////////////////////////////////////////////////////////////////////////////
//Specifies packing alignment for structure
#pragma pack(push,8)

typedef	struct _TTRSharedItem{	//entity of circlur command -queue.

		DWORD dwDevIndex;	//the index of device

		DWORD dwMajCmd;		//Major commaxnd
        DWORD dwMinCmd;		//Minor command

		DWORD dwTx;		//the number of Tx
		DWORD dwRx;		//the number of Rx
		DWORD dwData;	//the number of Data

		BYTE sTx[TTR_SHARE_ITEM_DEF_BUF_SIZE];	//tx buffer
		BYTE sRx[TTR_SHARE_ITEM_DEF_BUF_SIZE];	//rx buffer
		BYTE sData[TTR_SHARE_ITEM_BIG_BUF_SIZE];	//addition buffer

        DWORD dwPara[TTR_SHARE_ITEM_PARA_NUM];	//addtional info

		DWORD dwTimeOut;		// Time out for sync-method
		TCHAR sEventName[TTR_DEF_EVENT_NAME_SIZE];	//named event string for complete processing
		HANDLE hEvent;			//the handle of sEventName

		TTR_CBFunType CBFun;	//callback function
		LPVOID lpUserPara;		//user parameter of callback function

		HWND hWnd;				// window handle
		UINT nMsg;				//the windows message

		DWORD dwResult;

		HANDLE hMapObj;				// file mapping object. for result

} TTRSharedItem, *PTTRSharedItem,*LPTTRSharedItem;


#pragma pack(pop)

/////////////////////////////////////////

#endif	//__TTRL_SHARE_ITEM_HEADER_20091110__
