
/*
 * system command set header
 *
 */

#ifndef _COMMAND_SET_201007080001H_
#define _COMMAND_SET_201007080001H_

#include "compiler.h"

enum MSR_HostCmd	//Declare host commnad set
{
	Cmd_ChangeAuthKey	='C',	//0x43 this command changes encryption key
								//that is used in encryption of Cmd_ChangeEnKey New Key field.
	Cmd_ChangeEnKey		='K',	//this command changes encryption key
								//that is used in encryption of magnetic card data.
	Cmd_ChangeStatus	='M',	//0x4D change status ( Wait-MS, No-Wait-MS )
	Cmd_ChangeSN		='S',	//this command updates serial number
	Cmd_Config			='A',	//0x41 system configuration command
	Cmd_Apply			='B',	//0x42 apply EEPROM data to system

	Cmd_EnterCS			='X',	//0x58 enter configuration mode.......
								// you must execute this command before do other command.
	Cmd_LeaveCS			='Y',	//0x59 leave configuration mode.......
								// you must execute this command when your command processing.
	Cmd_GotoBootLoader	='G',	//goto boot loader.

	Cmd_EnterOps		='I',	//enter opos mode.
	Cmd_LeaveOps		='J',	//leave opos mode.

	Cmd_HWisStandard	='D',	//current model is standard.
	Cmd_HWisOnlyiButton	='W',	//current model support only i-button.
	Cmd_ReadUID			='U',	//read UID
	Cmd_HWisMMD1000		='N'	//Hardware is MMD1000
};

//the parameter of Cmd_ChangeStatus
enum MSR_StatusValue
{
	Status_Disable = 0,
	Status_Enable = 1
};


#define	MSR_RESP_PREFIX		'R'	//0x52 the prefix code of MSR response.

enum MSR_Response	//Declare MSR Response set
{
	Resp_Good			=0xFF,	//action done. good processing
	Resp_GoodNegative	=0x80,	//action done. negative processing

	Resp_Error_CRC		=0x01,	//CRC error
	Resp_Error_MISLEN	=0x02,	//Mis-matching length of data field
	Resp_Error_MISKEY	=0x03,	//Mis-matching Key
	Resp_Error_MISCHKBLK=0x04,	//Wrong check block
	Resp_Error_INVALID	=0x05,	//invalied commad
	Resp_Error_VERIFY	=0x06	//failure verification.
};

////////////////////////////////////////////////////
//must be byte alignment

#define	MAX_SIZE_HOST_PACKET_DATA_FIELD		255
#define	SIZE_HOST_PACKET_HEADER			3	//the sum of size(cCmd, cSub ,cLen)

#ifdef	WIN32
#pragma pack(push,1)
#endif	//WIN32

typedef struct tagMSR_HOST_PACKET{

	unsigned char cCmd;//MSR_Response value
	unsigned char cSub;
	unsigned char cLen;
	unsigned char sData[MAX_SIZE_HOST_PACKET_DATA_FIELD];

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT MSR_HOST_PACKET, *PMSR_HOST_PACKET, *LPMSR_HOST_PACKET;

#ifdef	WIN32
#pragma pack(pop)
#endif	//WIN32



#define	NUMBER_SUPPORT_LANGUAGE		2
enum Msr_Lang_Index{

	Lang_Index_English		= 0,
	Lang_Index_Spanish		= 1
};

////////////////////////////////////////////////////
//
#define	SYSREQ_SETTER_START_NMBER		0
#define	SYSREQ_GETTER_START_NMBER		50

//config request.......
enum SysRequest_Config{

	SysReqConfig_Set		=	200,
	SysReqConfig_Get		=	201,
};

#endif	//_COMMAND_SET_201007080001H_
