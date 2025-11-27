
#ifndef		__PROTOCOL__H__
#define		 __PROTOCOL__H__

#include <mp_type.h>
#include <cstdint>

// Command or cReplay items
#define	HIDB_REQ_CMD_WRITE		10	//write sector
#define	HIDB_REQ_CMD_READ		20	//read sector
#define	HIDB_REQ_CMD_ERASE		30	//erase sector
#define	HIDB_REQ_CMD_RUN		40	//run application
#define	HIDB_REQ_CMD_GET_SECTOR_INFO	50 //get sector info(from MH1902T), return data field : 4 bytes little endian start sector number, 4 bytes little endian the number of sector(except boot area) 

// result
#define	HIDB_REP_RESULT_SUCCESS		0
#define	HIDB_REP_RESULT_ERROR		0xff

#ifdef	_WIN32
#pragma pack(push,1)
#endif	//_WIN32

//this structure must be packed.
typedef struct tagHidBLRequest {

	unsigned char cCmd;

	union {
		unsigned char sTag[5];
		unsigned short wPara;
		uint32_t dwPara;
	}_PACK_BYTE;

	unsigned short wChain;	//from zero
	unsigned short wLen;
	unsigned char sData[1];

}_PACK_BYTE HidBLRequest;

typedef struct tagHidBLReplay {

	unsigned char cReplay;
	unsigned char cResult;
	unsigned char sTag[4];
	unsigned short wChain;	//from zero
	unsigned short wLen;
	unsigned char sData[1];

}_PACK_BYTE HidBLReplay;


// HID bootloader transaction
#define	HIDB_TRANS_ST_IDLE		10
#define	HIDB_TRANS_ST_ING		20

typedef enum {

	StatusTrans_Idle = 10,
	StatusTrans_OutReport = 20,
	StatusTrans_Job = 30,
	StatusTrans_InReport = 40

} StatusTransaction;

typedef struct tagHidBLIOTran {

	StatusTransaction Status;	//StatusTransaction
	unsigned char cCmd;

	union {
		unsigned char sTag[5];
		unsigned short wPara;
		uint32_t dwPara;
	}_PACK_BYTE;

	unsigned short wChain;	//from zero
	unsigned short wLen;	// total size
	unsigned short wOffset;	//current buffer offset of pData.
	unsigned char* pData;

}_PACK_BYTE HidBLIOTran;

#ifdef	_WIN32
#pragma pack(pop)
#endif	//_WIN32

#endif//	__PROTOCOL__H__
