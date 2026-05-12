// EL_Pin_Cmd.h: defintion of pinpad commad.
//////////////////////////////////////////////////////////////////////

#if !defined(__EL_PIN_CMD_H_20070706__)
#define __EL_PIN_CMD_H_20070706__

/////////////////////////////////////////////
//Message type definition
#define	EL_PIN_CMD_TYPE_DIS_BMP			0x40
#define	EL_PIN_CMD_TYPE_DIS_STRING		0x41
#define	EL_PIN_CMD_TYPE_VOC_MSG			0x42
#define	EL_PIN_CMD_TYPE_PW_INPUT		0x45
#define	EL_PIN_CMD_TYPE_SEL_MSG			0x4C
#define	EL_PIN_CMD_TYPE_STOP_CMD		0x58

#define	EL_PIN_CMD_TYPE_GET_VER			0x50
#define	EL_PIN_CMD_TYPE_SET_CFG			0x51
#define	EL_PIN_CMD_TYPE_GET_CFG			0x52
#define	EL_PIN_CMD_TYPE_WR_TMK			0x53
#define	EL_PIN_CMD_TYPE_WR_BMP			0x54
#define	EL_PIN_CMD_TYPE_WR_VOC			0x55
#define	EL_PIN_CMD_TYPE_WR_CODE			0x56
#define	EL_PIN_CMD_TYPE_CHR_CODE		0x5F

#define	EL_PIN_CMD_TYPE_ICC_ON			0x62
#define	EL_PIN_CMD_TYPE_ICC_OFF			0x63
#define	EL_PIN_CMD_TYPE_GET_SLT_ST		0x65
#define	EL_PIN_CMD_TYPE_ICC_APDU		0x6F
#define	EL_PIN_CMD_TYPE_GET_ICC_MOD		0x64
#define	EL_PIN_CMD_TYPE_SET_ICC_MOD		0x66


////////////////////////////////////////////
//constants definition as Message type

////EL_PIN_CMD_TYPE_DIS_BMP
#define	EL_PIN_MAX_BMP_INDEX			5
#define	EL_PIN_MIN_BMP_INDEX			0
#define	EL_PIN_MAX_BMP_ON_TIME			255	//unit[sec]

////EL_PIN_CMD_TYPE_DIS_STRING
#define	EL_PIN_MAX_STR_INDEX			255
#define	EL_PIN_MIN_STR_INDEX			0
#define	EL_PIN_MAX_STR_ON_TIME			255	//unit[sec]

////EL_PIN_CMD_TYPE_VOC_MSG
#define	EL_PIN_MAX_VOICE_INDEX			16
#define	EL_PIN_MIN_VOICE_INDEX			0

////EL_PIN_CMD_TYPE_PW_INPUT
#define	EL_PIN_MAX_PW_LEN				16
#define	EL_PIN_MIN_PW_LEN				0
//each flag can be logical oring for uFlag-parameter
#define	EL_PIN_FLAG_PW_CONFIRM			0x01//confirm flag
#define	EL_PIN_FLAG_PW_ENCRYTION		0x40//encrytion flag
#define	EL_PIN_FLAG_PW_DISPLAY			0x80//display flag

////EL_PIN_CMD_TYPE_SEL_MSG
////EL_PIN_CMD_TYPE_STOP_CMD

////EL_PIN_CMD_TYPE_GET_VER
////EL_PIN_CMD_TYPE_SET_CFG
////EL_PIN_CMD_TYPE_GET_CFG
////EL_PIN_CMD_TYPE_WR_TMK

////EL_PIN_CMD_TYPE_WR_BMP

////EL_PIN_CMD_TYPE_WR_VOC

////EL_PIN_CMD_TYPE_WR_CODE
#define	EL_PIN_FLAG_WR_CODE_MAIN		0x00//main flag
#define	EL_PIN_FLAG_WR_CODE_BIOS		0x01//update bios flag

////EL_PIN_CMD_TYPE_CHR_CODE

////EL_PIN_CMD_TYPE_ICC_ON
#define	EL_PIN_FLAG_ICC_ON_AUTO			0x00//automatic voltage
#define	EL_PIN_FLAG_ICC_ON_5V			0x01//5 voltage
#define	EL_PIN_FLAG_ICC_ON_3_3V			0x02//3.3 voltage

////EL_PIN_CMD_TYPE_ICC_OFF

////EL_PIN_CMD_TYPE_GET_SLT_ST

////EL_PIN_CMD_TYPE_ICC_APDU
////EL_PIN_CMD_TYPE_GET_ICC_MOD
////EL_PIN_CMD_TYPE_SET_ICC_MOD



#endif	//__EL_PIN_CMD_H_20070706__
