
/*
 * msr task header file
 */


#if !defined( _INFO_MSR_HEADER_201007260001_ )
#define _INFO_MSR_HEADER_201007260001_

#include "compiler.h"
#include "config/cfg_msrlib.h"

#define	REMOVE_IBUTTON_DEF_TAG_SIZE	40	//the Tag  size
#define	MSROBJ_INFO_DEF_TAG_SIZE			14	//the Tag  size

#define	MSROBJ_INFO_DEF_ITEM_SIZE	4	//default item size of system information structure
#define	MSROBJ_INFO_DEF_MODE_SIZE	2	//the number of mode item of tagINFO_MSR_OBJ structure.
#define	MSROBJ_INFO_DEF_KEY_SIZE	16	//the number of key item of tagINFO_MSR_OBJ structure

#define	MSROBJ_CB_FUNS				7	//the number of callback function
#define	MSROBJ_CB_FUN_INDEX_INI		0	//the index of initial callback funcation in callback arrary.
#define	MSROBJ_CB_FUN_INDEX_DEL		1	//the index of delete callback funcation in callback arrary.
#define	MSROBJ_CB_FUN_INDEX_ECS		2	//the index of enter-critical-section callback funcation in callback arrary.
#define	MSROBJ_CB_FUN_INDEX_LCS		3	//the index of leave-critical-section callback funcation in callback arrary.
#define	MSROBJ_CB_FUN_INDEX_READ	4	//the index of read callback funcation in callback arrary.
#define	MSROBJ_CB_FUN_INDEX_SEND	5	//the index of send callback funcation in callback arrary.
#define	MSROBJ_CB_FUN_INDEX_CPD		6	//the index of CPD callback funcation in callback arrary.

////////////////////////////////
//determine MSROBJ_INFO_NUM.
#if( (CONF_MSROBJ_NUMBER>3) || (CONF_MSROBJ_NUMBER<1) )
	#error Error msr obejct is >1 and < 4.
#else
	#define	MSROBJ_INFO_NUM			CONF_MSROBJ_NUMBER	//msr object number
#endif

////////////////////////////////
//determine MSR_OBJ_ALL_RAW_BUF_SIZE.
#ifndef CONF_MSROBJ_RAW_BUFFER_SIZE
	#error you must define CONF_MSROBJ_RAW_BUFFER_SIZE.
#elif( CONF_MSROBJ_RAW_BUFFER_SIZE==0 )		//automatic

	#if(MSROBJ_INFO_NUM==1)
		#define	MSR_OBJ_ALL_RAW_BUF_SIZE	C_MSR_ISO1_RAW_BUF_SIZE
	#elif(MSROBJ_INFO_NUM==2)
		#define	MSR_OBJ_ALL_RAW_BUF_SIZE	(C_MSR_ISO1_RAW_BUF_SIZE*2)
	#else//MSROBJ_INFO_NUM==3
		#define	MSR_OBJ_ALL_RAW_BUF_SIZE	(C_MSR_ISO1_RAW_BUF_SIZE*3)
	#endif

#else
	#define	MSR_OBJ_ALL_RAW_BUF_SIZE		CONF_MSROBJ_RAW_BUFFER_SIZE
#endif


////////////////////////////////
//determine MSROBJ_INFO_COMB_NUM.
#if( CONF_MSROBJ_NUMBER_COMB<1 )
	#error msr obejct combination number is greater then equal one.
#elif( CONF_MSROBJ_NUMBER_COMB>4 )
	#error msr obejct combination number is less then equal FOUR.
#else
	#define	MSROBJ_INFO_COMB_NUM	CONF_MSROBJ_NUMBER_COMB	//msr combination number
#endif


#ifdef	WIN32
#pragma pack(push,1)
#endif	//WIN32

typedef struct tagMSR_MAP_TABLE{

	//the index of key Mapping table' l'st index gASCToHIDKeyMap or gASCToPS2KeyMap
	unsigned long nMappingTableIndex;

	unsigned long nNumMapTableItem;	//the numnber of pMappingTable' item
									//default: nNumMapTableItem[x] = 0
}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT MSR_MAP_TABLE, *PMSR_MAP_TABLE,*LPMSR_MAP_TABLE;

//global tag , pre / post and private pre /postfix
typedef struct tagMSR_TAG{

	unsigned char cSize;	//the number of data of sTag

	unsigned char sTag[MSROBJ_INFO_DEF_TAG_SIZE];
	//USB K/B interface  tag data
	//sTag[2n]   - modified key(0<=n<=MSROBJ_INFO_DEF_TAG_SIZE/2)
	//sTag[2n+1] - USB HID key(0<=n<=MSROBJ_INFO_DEF_TAG_SIZE/2)
	//default:  all zeros

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT MSR_TAG, *PMSR_TAG, *LPMSR_TAG;

//in i-button removed
typedef struct tagREMOVE_IBUTTON_TAG {

	unsigned char cSize;	//the number of data of sTag

	unsigned char sTag[REMOVE_IBUTTON_DEF_TAG_SIZE];
	//USB K/B interface  tag data
	//sTag[2n]   - modified key(0<=n<=REMOVE_IBUTTON_DEF_TAG_SIZE/2)
	//sTag[2n+1] - USB HID key(0<=n<=REMOVE_IBUTTON_DEF_TAG_SIZE/2)
	//default:  all zeros

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT REMOVE_IBUTTON_TAG, * PREMOVE_IBUTTON_TAG, * LPREMOVE_IBUTTON_TAG;

//this structure is msr object information
typedef struct tagINFO_MSR_OBJ{

	unsigned char cEnableTrack;
									//4(36) : enable(1) or disable(0)
									//default: sEnableTrack = 1

	unsigned char cSupportNum;
									//4(40) : the number of supported combination MSR object
									//default: cSupportNum[]=1

	unsigned char cActiveCombination;	//active combination
										//default: cActiveCombination = 0;

	unsigned char cMaxSize[MSROBJ_INFO_COMB_NUM];
									//16(56): the maximum number of data except STX,ETX,LRC
									//default:	cMaxSize[x]=C_MSR_ISO2_MAX_DATA

	unsigned char cBitSize[MSROBJ_INFO_COMB_NUM];
									//16(72): the bit size of datem(including Error check bit).......
									//default:	cBitSize[x]=5

	unsigned char cDataMask[MSROBJ_INFO_COMB_NUM];
									//16(88): (including Error check bit).......
									//if cBitSize==7 then cDataMask=0xFE.
									//if cBitSize==5 then cDataMask=0xF8. and so on.......
									//default:	DataMask[x]=MSR_DataMask_BIT5

	//Parity section
	unsigned char bUseParity[MSROBJ_INFO_COMB_NUM];
									//16(104): Use or not parity(use:1,not use:0)
									//default:	bUseParity[x]=1

	unsigned char cParityType[MSROBJ_INFO_COMB_NUM];	//16(120):Parity type
									//default:	bUseParity[x]=ParityOdd


	//Pattern section
	unsigned char cSTX_L[MSROBJ_INFO_COMB_NUM];
									//16(136):STX pattern including parity ,left alignment
									//default:	cSTX_L[x]=C_MSR_ISO2_STX_L

	unsigned char cETX_L[MSROBJ_INFO_COMB_NUM];
									//16(152):ETX pattern including parity ,left alignment
									//default:	cETX_L[x]=C_MSR_ISO2_ETX_L

	//Error correction.
	unsigned char bUseErrorCorrect[MSROBJ_INFO_COMB_NUM];
									//16(168): Use or not Error correction code(use:1,not use:0)
									//default:	bUseErrorCorrect[x]=1

	//Error correction Method.
	unsigned char cECMType[MSROBJ_INFO_COMB_NUM];
									//16(184)://support only ECMTypeLRC & ECMTypeINVLRC .2008.4.1
									//default:	bECMType[x]=ECMTypeLRC

	unsigned char cRDirect[MSROBJ_INFO_COMB_NUM];
									//16(188):reading direction
									//default:	RDirect[x]=RDirectBoth

	unsigned long nBufSize;
									//4(204): raw data buffer size.
									//default:	cBufSize=C_MSR_ISO2_RAW_BUF_SIZE

	unsigned char cAddValue[MSROBJ_INFO_COMB_NUM];
									//16(220):value of adding to raw datem.
									//default:	cAddValue[x]=0x30 - ISO2 data to ascii
									//			- ISO1 data to ascii

	unsigned char bEnableEncryption;
									//default: 0 - disable encryption

	unsigned char sMasterKey[MSROBJ_INFO_DEF_KEY_SIZE];
									//16(260):tripple des key
									//default: sMasterKey[]={'0','1','2','3','4','5','6','7','0','1','2','3','4','5','6','7'}

	unsigned char sChangeKey[MSROBJ_INFO_DEF_KEY_SIZE];
									//16(268):tripple des key
									//default: sChangeKey[]={0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF}

	//private prefix data(for MSR object) for usb keyboard.
	MSR_TAG PrivatePrefix[MSROBJ_INFO_COMB_NUM];

	//private postfix data(for MSR object) for usb keyboard.
	MSR_TAG PrivatePostfix[MSROBJ_INFO_COMB_NUM];

	//New add this member
	//keyboard mapping table
	MSR_MAP_TABLE KeyMap[MSROBJ_INFO_COMB_NUM];

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT INFO_MSR_OBJ, *PINFO_MSR_OBJ,*LPINFO_MSR_OBJ;


////////////////////////////////////////////////////
//this container structure of Info msr objects
typedef struct tagCONTAINER_INFO_MSR_OBJ{

	// Info msr object' pointer arrary
#ifdef	_WIN64
	unsigned long pInfoMsrObj[MSROBJ_INFO_NUM];
#else
	PINFO_MSR_OBJ pInfoMsrObj[MSROBJ_INFO_NUM];
#endif	//_WIN64

	unsigned long nCpdSysTickMin;	//the minimum system tick value of CPD low status
	unsigned long nCpdSysTickMax;	//the maximum system tick value of CPD low status

	// Global pre/postfix sending condition.
	unsigned long nGlobalTagCondition;
							//default:	0 - any track is good & good track' length of data. > 0
							//			1 - all track is good & any track' length of data. > 0

	//the number of contained info object.
	unsigned long nNumItem;	//nNumItem = MSROBJ_INFO_NUM

	//the processing order of info object in InfoMsrObj members.
	unsigned long nOrderObject[MSROBJ_INFO_NUM];
							//default:	OrderObject[0] = 0
							//			OrderObject[1] = 1
							//			OrderObject[2] = 2
							//			.......
							//			OrderObject[n-1] = n-1

	//keyboard mapping table
	MSR_MAP_TABLE KeyMap;

	/////////////////////////////////////////////////////////////////////
	//TagPre & Post is displayed always.

	//if all MsrObject occured error, display.
	MSR_TAG TagPre;
	MSR_TAG TagPost;

	/////////////////////////////////////////////////////////////////////
	//GlobalPre & Post is displayed when a MsrObject have been processed.
	//if all MsrObject occured error, not display.
	MSR_TAG GlobalPrefix;
	MSR_TAG GlobalPostfix;

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT CONTAINER_INFO_MSR_OBJ, *PCONTAINER_INFO_MSR_OBJ,*LPCONTAINER_INFO_MSR_OBJ;

///////////////////////////////////////////////////
// old part
typedef struct tagMSR_MAP_TABLE_OLD{

	//if this value isn't zero, system use mapping table
	const unsigned char *pMappingTable;
									//default:  pMappingTable[x] = NULL

	unsigned long nNumMapTableItem;	//the numnber of pMappingTable' item
									//default: nNumMapTableItem[x] = 0
}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT MSR_MAP_TABLE_OLD, *PMSR_MAP_TABLE_OLD,*LPMSR_MAP_TABLE_OLD;


//this structure is msr object information
typedef struct tagINFO_MSR_OBJ_OLD{

	unsigned char cEnableTrack;
									//4(36) : enable(1) or disable(0)
									//default: sEnableTrack = 1

	unsigned char cSupportNum;
									//4(40) : the number of supported combination MSR object
									//default: cSupportNum[]=1

	unsigned char cActiveCombination;	//active combination
										//default: cActiveCombination = 0;

	unsigned char cMaxSize[MSROBJ_INFO_COMB_NUM];
									//16(56): the maximum number of data except STX,ETX,LRC
									//default:	cMaxSize[x]=C_MSR_ISO2_MAX_DATA

	unsigned char cBitSize[MSROBJ_INFO_COMB_NUM];
									//16(72): the bit size of datem(including Error check bit).......
									//default:	cBitSize[x]=5

	unsigned char cDataMask[MSROBJ_INFO_COMB_NUM];
									//16(88): (including Error check bit).......
									//if cBitSize==7 then cDataMask=0xFE.
									//if cBitSize==5 then cDataMask=0xF8. and so on.......
									//default:	DataMask[x]=MSR_DataMask_BIT5

	//Parity section
	unsigned char bUseParity[MSROBJ_INFO_COMB_NUM];
									//16(104): Use or not parity(use:1,not use:0)
									//default:	bUseParity[x]=1

	unsigned char cParityType[MSROBJ_INFO_COMB_NUM];	//16(120):Parity type
									//default:	bUseParity[x]=ParityOdd


	//Pattern section
	unsigned char cSTX_L[MSROBJ_INFO_COMB_NUM];
									//16(136):STX pattern including parity ,left alignment
									//default:	cSTX_L[x]=C_MSR_ISO2_STX_L

	unsigned char cETX_L[MSROBJ_INFO_COMB_NUM];
									//16(152):ETX pattern including parity ,left alignment
									//default:	cETX_L[x]=C_MSR_ISO2_ETX_L

	//Error correction.
	unsigned char bUseErrorCorrect[MSROBJ_INFO_COMB_NUM];
									//16(168): Use or not Error correction code(use:1,not use:0)
									//default:	bUseErrorCorrect[x]=1

	//Error correction Method.
	unsigned char cECMType[MSROBJ_INFO_COMB_NUM];
									//16(184)://support only ECMTypeLRC & ECMTypeINVLRC .2008.4.1
									//default:	bECMType[x]=ECMTypeLRC

	unsigned char cRDirect[MSROBJ_INFO_COMB_NUM];
									//16(188):reading direction
									//default:	RDirect[x]=RDirectBoth

	unsigned long nBufSize;
									//4(204): raw data buffer size.
									//default:	cBufSize=C_MSR_ISO2_RAW_BUF_SIZE

	unsigned char cAddValue[MSROBJ_INFO_COMB_NUM];
									//16(220):value of adding to raw datem.
									//default:	cAddValue[x]=0x30 - ISO2 data to ascii
									//			- ISO1 data to ascii

	unsigned char bEnableEncryption;
									//default: 0 - disable encryption

	unsigned char sMasterKey[MSROBJ_INFO_DEF_KEY_SIZE];
									//16(260):tripple des key
									//default: sMasterKey[]={'0','1','2','3','4','5','6','7','0','1','2','3','4','5','6','7'}

	unsigned char sChangeKey[MSROBJ_INFO_DEF_KEY_SIZE];
									//16(268):tripple des key
									//default: sChangeKey[]={0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF}

	//private prefix data(for MSR object) for usb keyboard.
	MSR_TAG PrivatePrefix[MSROBJ_INFO_COMB_NUM];

	//private postfix data(for MSR object) for usb keyboard.
	MSR_TAG PrivatePostfix[MSROBJ_INFO_COMB_NUM];

	//New add this member
	//keyboard mapping table
	MSR_MAP_TABLE_OLD KeyMap[MSROBJ_INFO_COMB_NUM];

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT INFO_MSR_OBJ_OLD, *PINFO_MSR_OBJ_OLD,*LPINFO_MSR_OBJ_OLD;


////////////////////////////////////////////////////
//this container structure of Info msr objects
typedef struct tagCONTAINER_INFO_MSR_OBJ_OLD{

	// Info msr object' pointer arrary
#ifdef	_WIN64
	unsigned long pInfoMsrObj[MSROBJ_INFO_NUM];
#else
	PINFO_MSR_OBJ pInfoMsrObj[MSROBJ_INFO_NUM];
#endif	//_WIN64

	unsigned long nCpdSysTickMin;	//the minimum system tick value of CPD low status
	unsigned long nCpdSysTickMax;	//the maximum system tick value of CPD low status

	// Global pre/postfix sending condition.
	unsigned long nGlobalTagCondition;
							//default:	0 - any track is good & good track' length of data. > 0
							//			1 - all track is good & any track' length of data. > 0

	//the number of contained info object.
	unsigned long nNumItem;	//nNumItem = MSROBJ_INFO_NUM

	//the processing order of info object in InfoMsrObj members.
	unsigned long nOrderObject[MSROBJ_INFO_NUM];
							//default:	OrderObject[0] = 0
							//			OrderObject[1] = 1
							//			OrderObject[2] = 2
							//			.......
							//			OrderObject[n-1] = n-1

	//keyboard mapping table
	MSR_MAP_TABLE_OLD KeyMap;

	/////////////////////////////////////////////////////////////////////
	//TagPre & Post is displayed always.

	//if all MsrObject occured error, display.
	MSR_TAG TagPre;
	MSR_TAG TagPost;

	/////////////////////////////////////////////////////////////////////
	//GlobalPre & Post is displayed when a MsrObject have been processed.
	//if all MsrObject occured error, not display.
	MSR_TAG GlobalPrefix;
	MSR_TAG GlobalPostfix;

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT CONTAINER_INFO_MSR_OBJ_OLD, *PCONTAINER_INFO_MSR_OBJ_OLD,*LPCONTAINER_INFO_MSR_OBJ_OLD;

#ifdef	WIN32
#pragma pack(pop)
#endif	//WIN32


#endif	//_INFO_MSR_HEADER_201007260001_

