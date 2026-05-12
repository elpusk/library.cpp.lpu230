// CmdQueue.h: interface for the CCmdQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CMDQUEUE_H__9B917958_2AAB_45BD_A39E_D970F412087B__INCLUDED_)
#define AFX_CMDQUEUE_H__9B917958_2AAB_45BD_A39E_D970F412087B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	MAX_CMD_QUEUE_SIZE			256		//maximum cmmand queue size.
#define ITEM_BUFFER_SIZE			512
#define ADD_INFO_ITEM_BUFFER_SIZE	3264
#define	EVENT_NAME_SIZE				256

typedef LRESULT (CALLBACK* SVRCallbackFunType)(LPVOID lpUserPara,DWORD dwResult,LPVOID lpData,int nSize);

#pragma pack(push,8)

typedef	struct _CmdItem{	//entity of circlur command -queue.
        BYTE cMajCmd;	//Major command
        BYTE cMinCmd;	//Minor command
		DWORD dwDevIndex;	//the index of device
        BYTE cInfo;		//addtional info
		BYTE cPara;		//addtional Info
		BYTE pData[ADD_INFO_ITEM_BUFFER_SIZE];	//addtional info
		BYTE pBuf[ITEM_BUFFER_SIZE];	//RX or TX buffer
		DWORD dwSize;
		BYTE pBuf1[ITEM_BUFFER_SIZE];	//RX or TX buffer
		DWORD dwSize1;
		HWND hWnd;
		UINT nMsg;		//the first windows message
		UINT nMsg1;		//the second windows message
		SVRCallbackFunType pCallBackFun;
		TCHAR sEventName[EVENT_NAME_SIZE];	//named event string
} CmdItem, *PCmdItem,*LPCmdItem;

#pragma pack(pop)

//this class command circlur queue.
class CCmdQueue  
{
public:
	CCmdQueue();
	virtual ~CCmdQueue();
	//
	BOOL NewClass( int nindex );
	BOOL DeleteClass();
	//
	BOOL IsEmpty();	//Check
	void Empty();	//clear queue.
	BOOL EnQueue( CmdItem &Item );		//add item to queue
	BOOL EnQueueFirst( CmdItem &Item );		//add item to statring point of queue
	//
	BOOL DeQueue( LPCmdItem pItem );	//Gets item from queue.
	BOOL CheckQueue( LPCmdItem pItem );
	BOOL CheckMajItem( LPBYTE pcMajCmd );
	BOOL CheckMinQueue( LPBYTE pcMinCmd );
	//
	BOOL GetResult( DWORD *pdwResult,LPBYTE lpData,LONG* plDataSize,DWORD *pdwDevIndex=NULL );
	BOOL SetResult( DWORD dwResult,LPBYTE lpData,LONG lDataSize,DWORD dwDevIndex=0 );
	HANDLE m_hMutexSync;
	HANDLE m_hMutexResult;

protected:
	CmdItem m_CirQueue[MAX_CMD_QUEUE_SIZE];		//buffer of circlur queue
	int m_nNextReadPos;		//the postion of next read item.
	int m_nNextWritePos;	//the postion of next Write item.
	
	BYTE m_sRXData[ITEM_BUFFER_SIZE];
	LONG m_lRXSize;
	DWORD m_dwResult;
	DWORD m_dwDevIndex;
	TCHAR m_sMutexSyncName[EVENT_NAME_SIZE];
	TCHAR m_sMutexResultName[EVENT_NAME_SIZE];
};

#endif // !defined(AFX_CMDQUEUE_H__9B917958_2AAB_45BD_A39E_D970F412087B__INCLUDED_)
