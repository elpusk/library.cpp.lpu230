#if !defined(__PINPAD_INTERNAL_MISCELLANY_H_20071101__)
#define __PINPAD_INTERNAL_MISCELLANY_H_20071101__

#define	WRB_RECORD_SIZE			32
#define	WRB_ACCOUNT_NUM_SIZE	16
#define WRB_ACCOUNT_INFO_SIZE	16
#define	WRB_MAX_RECORDS_DATA_SIZE	1920

//////////////////////////////////////////////////////
//structure

//On DisplayAccDataPINDevice, return value sharing structure

typedef	struct TagAccDataItem{	//entity of circlur command -queue.
	int nSelRec;	//selected record number.......
	
	unsigned char sSelRecData[WRB_RECORD_SIZE*2];	//selected record data.
	int nSelRecData;	//data size of sSelRecData buffer.
	
} AccDataItem, *PAccDataItem,*LPAccDataItem;


//
#endif//__PINPAD_INTERNAL_MISCELLANY_H_20071101__