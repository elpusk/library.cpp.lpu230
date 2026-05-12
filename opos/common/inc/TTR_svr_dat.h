
#if !defined(__TTRL_SERVER_DAT_HEADER_2009122300__)
#define __TTRL_SERVER_DAT_HEADER_2009122300__

/////////////////////////////
//TTR serise etc header file
/////////////////////////////

//////////////////
//1. include file
//////////////////
#include "TTR_tchar.h"
#include <list>


////////////////////////////////////////////
//Specifies packing alignment for structure
#pragma pack(push,1)

typedef	struct _TTR_SVR_DAT_ITEM{

		WORD wProductId;						//one base number.
		TCHAR sPdllName[TTR_DEF_NAME_BUF_SIZE];	//protocol dll name

	}TTR_SVR_DAT_ITEM, *PTTR_SVR_DAT_ITEM, *LPTTR_SVR_DAT_ITEM;

typedef	struct _TTR_SVR_DAT_HEAD{

		WORD wCntItem;							//the number of item in dat-file
		TCHAR sVersion[TTR_DEF_NAME_BUF_SIZE];	//version string
		DWORD dwCRC;	//CRC value of dat-file(total file xor)

	}TTR_SVR_DAT_HEAD, *PTTR_SVR_DAT_HEAD, *LPTTR_SVR_DAT_HEAD;

#pragma pack(pop)



#endif	//__TTRL_SERVER_DAT_HEADER_2009122300__