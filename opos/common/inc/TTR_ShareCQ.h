
#if !defined(__TTRL_SHARE_CIRCULER_QUEUE_HEADER_20091104__)
#define __TTRL_SHARE_CIRCULER_QUEUE_HEADER_20091104__

///////////////////////////////////////////////
//TTR serise Shared circuler queue header file
//////////////////////////////////////////////	
#include "TTR_def.h"
#include "TTR_etc.h"
#include "TTR_ShareItem.h"

/*
#define	TTR_SHARE_CQ_NAME_MAX_SIZE			TTR_DEF_EVENT_NAME_SIZE
#define	TTR_SHARE_CQ_DEF_BUF_SIZE			512
#define	TTR_SHARE_CQ_BIG_BUF_SIZE			3264
#define	TTR_SHARE_CQ_PARA_NUM				4
*/
#define	TTR_SHARE_CQ_DEF_Q_SIZE				256
#define	TTR_SHARE_CQ_DEF_TIMEOUT			5000L	//5sec. default time-out

//dwDevIndex size + dwResult size + dwSize + sData size - default value
#define	TTR_SHARE_CQ_DEF_RESP_MMF_SIZE		(sizeof(DWORD)+sizeof(DWORD)+sizeof(DWORD)+TTR_SHARE_CQ_DEF_BUF_SIZE)



//this class command circlur queue.
class CTTR_ShareCQueue  
{
public:
	CTTR_ShareCQueue();
	CTTR_ShareCQueue( BOOL bShared,TCHAR *sMutexSyncName=NULL,TCHAR *sMapObjName=NULL );
	virtual ~CTTR_ShareCQueue();
	//
	//copy m_sMutexSyncName data to sMutexSyncName, return length of name
	INT GetSyncMutexName( TCHAR *sMutexSyncName );
	INT GetMapObjName( TCHAR *sMapObjName );

	//
	BOOL IsEmpty();	//Check
	void Empty();	//clear queue.
	BOOL Lock( BOOL bLock = TRUE);	//lock enqueue 

	BOOL EnQueue( TTRSharedItem &Item );		//add item to queue
	BOOL EnQueueFirst( TTRSharedItem &Item );	//add item to statring point of queue
	//
	BOOL DeQueue( LPTTRSharedItem pItem );	//Gets item from queue.
	BOOL Peek( LPTTRSharedItem pItem );
	BOOL PeekMajor( LPDWORD pdwMajCmd );
	BOOL PeekMinor( LPDWORD pdwMinCmd );

	BOOL ResetPara0( DWORD dwConditionPara0 );	//reset parameter0 

	
	///////////////////////////////////////
	// thisn part is static serise
	//Get result from Item(share Q item )
	static BOOL GetSharedResult(
		LPTTRSharedItem pItem,
		LPDWORD pdwResult,
		LPBYTE lpData,
		LPLONG lpDataSize,
		LPDWORD pdwDevIndex
		);
	//Get result from hMapObj
	static BOOL GetSharedResult(
		HANDLE hMapObj,
		LPDWORD pdwResult,
		LPBYTE lpData,
		LPLONG lpDataSize,
		LPDWORD pdwDevIndex
		);

	//set result to Item
	static BOOL SetSharedResult(
		LPTTRSharedItem pItem,
		DWORD dwResult,
		LPBYTE lpData,
		LONG lDataSize,
		DWORD dwDevIndex
		);
	//set result to Map object ( static )
	static BOOL SetSharedResult(
		HANDLE hMapObj,
		DWORD dwResult,
		LPBYTE lpData,
		LONG lDataSize,
		DWORD dwDevIndex
		);
	//Create shared Item
	static LPTTRSharedItem CreateSharedCQItem();

	//Delete shared Item
	static BOOL DeleteSharedCQItem( LPTTRSharedItem pItem );

	//save Tx data to Item 
	static BOOL SetTxToShareItem( LPTTRSharedItem pItem,DWORD dwTx,LPBYTE lpbTx );

protected:
	LPTTRSharedItem m_pCQ;	//buffer starting pointer of circlur queue arrary

	LPINT m_pnNextRPos;		//the postion of next read item.
	LPINT m_pnNextWPos;		//the postion of next Write item.
	LPINT m_pnIsLock;		//isn't zero. impassible Enqueue
	

private:
	
	static const int SQ_MMF_OFF_HMUTEX = 0;//Mutex handle offset in MMF of response 
	static const int SQ_MMF_OFF_DEVINDEX = sizeof(HANDLE);//device index' offset in MMF of response 
	static const int SQ_MMF_OFF_RESULT = sizeof(HANDLE)+sizeof(DWORD);//result' offset in MMF of response 
	static const int SQ_MMF_OFF_SIZE = sizeof(HANDLE)+sizeof(DWORD)*2;//data size' offset in MMF of response 
	static const int SQ_MMF_OFF_DATA = sizeof(HANDLE)+sizeof(DWORD)*2+sizeof(LONG);//data' offset in MMF of response 

	static const int SQ_MMF_MAX_DATA = 1024*4;	//

	//enter critical section( return mutex handle )
	HANDLE EnterQ(void);

	void LeaveQ( HANDLE hMutex );

	//the handle of m_sMutexSyncName
	HANDLE m_hMutexSync;

	//the Mutex name for sync - Q
	TCHAR m_sMutexSyncName[TTR_DEF_EVENT_NAME_SIZE];

	//file map memory. m_pCQ is assigned to this
	LPTTR_MMF m_pCQMmf;	
	
};
/////////////////////////////////////////

#endif	//__TTRL_SHARE_CIRCULER_QUEUE_HEADER_20091104__
