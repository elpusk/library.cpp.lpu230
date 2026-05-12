// EL_Pin_IO_Base.h: interface for the CEL_Pin_IO_Base class.
// this header base of all elpusk pinpad.......
// 2007.4.30 : Verion 1.1 - support download of Non-supporting GetVeriosn command.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EL_PIN_IO_BASE_H__F6B26DB0_08F6_485B_B4C6_7D8277C04312__INCLUDED_)
#define AFX_EL_PIN_IO_BASE_H__F6B26DB0_08F6_485B_B4C6_7D8277C04312__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EL_HID.h"
#include "stdlib.h"
#include "ELLogGen.h"
#include "EL_Pin_Type.h"	//declaration of PinPadComparae,PinPadLangType,PinPadHWType and PinPadType.

#define	DEFAULT_REPORT_SIZE					64		//the default report size of HID
#define	PINPAD_VID							0x134b	//the pinpad VID
#define	PINPAD_PID_DEF						0x0310	//default the pinpad PID

#define	MAX_HID_PINPAD_PID_NUM				1	//the number of HID pinpad PID.

#define	DESCRIPTION_BUF_LENGTH				128
#define	NUMBER_LANG_TYPE					3	//the number of PinPadLangType
#define	NUMBER_HW_TYPE						6	//the number of PinPadHWType
#define	NUMBER_PINPAD_TYPE					20	//the number of PinPadType

#define	DEFAULT_MAX_DEVICE_INSTANCE			50
#define	MAX_PACKET_SIZE						64
#define	MAX_VERSION_BUF_SIZE				64
#define	DEVICE_LIST_BUF_SIZE				30*127

#define	REGISTERED_PINPAD_NUMBER			(19+2+3)	//plus 2 is duplicated and 2 is forth.
#define	PINPAD_ID_BUF_SIZE					16

#define PINPAD_VER_NUMBER_INDEX				11	//(zero-base)

#define	MAX_ATR_BUF_SIZE					256

//internal constants
#define DL_PINPAD_STX		0x02
#define DL_PINPAD_ETX		0x03

#define	INI_CHK_BUF_LEN		8


enum DrvMode	// Declare enum device-driver supprt mode
{
	DRV_MODE_UNKNOWN=0,//unknown driver mode
	DRV_MODE_N_ONLY_64,//None error recover and only read/write with 64bytes.
	DRV_MODE_M_ONLY_64,//Manual error recover and only read/write with 64bytes.
	DRV_MODE_A_ONLY_64,//Auto error recover and only read/write with 64bytes.
	DRV_MODE_N_VARIABLE,//None error recover and support variable read/write.
	DRV_MODE_M_VARIABLE,//Manual error recover and support variable read/write.
	DRV_MODE_A_VARIABLE,//Auto error recover and support variable read/write.(zero-64:return length zero)
	DRV_MODE_HID		//driver is system default HID driver
};

enum DrvType	// Declare enum device-driver supprt type
{
	DRV_TYPE_UNKNOWN=0,//unknown type driver
	DRV_TYPE_K1T_PINA,//K1T_PINA type driver
	DRV_TYPE_KICPIN_A,//KICPIN_A type driver
	DRV_TYPE_KLP110U,//KLP110U type driver
	DRV_TYPE_SYS_HID//system default HID type driver
};

class CEL_Pin_IO_Base  
{
public:
	CEL_Pin_IO_Base();
	virtual ~CEL_Pin_IO_Base();

	void Initial();//call by constructure

	//////////////////////////////////////////////////////////////////
	//for device access function and exported function
	DWORD Open( TCHAR *sDevPath=NULL );	//open device
	DWORD Close();	//close current opened device

	DWORD Write( BYTE *pData=NULL,int *pnSize=NULL );
	DWORD Read( BYTE *pData=NULL,int *pnSize=NULL );


	///////////////////////////////////////////////////////////////////

	//compara given version string and current opened pinpad firmare veriosn.
	enum PinPadComparae CompareVersion( TCHAR *sVersion=NULL );

	BOOL IsOpen();

	//get hardware description with given Hardware type code
	void GetHWTypeDescription( TCHAR *sHWDes,enum PinPadHWType nHWType );

	//get pinpad description with given pinpad type code
	void GetPinpadTypeDescription( TCHAR *sPinpadDes,enum PinPadType nType );

	//get language description with given language type code
	void GetLangTypeDescription( TCHAR *sLangDes,enum PinPadLangType nLangType );

	//return connected pinpad devices list to string
	static int GetDevList( TCHAR *psDevList );

	//return connected pinpad devices list to string with type
	static int GetDevListEx( TCHAR *psDevList,enum DrvType Type );

	//for download support
	DWORD DL_GetResponse( BYTE *lpdata,DWORD *pdwSize );
	DWORD DL_SendStartPacket();
	DWORD DL_SendStopPacket();
	DWORD DL_SendPacket( BYTE *pData,WORD wSize,WORD wAdd );

	enum PinPadHWType GetCurHWType()
	{	return m_nCurHWType;		};
	enum PinPadType	GetCurType()
	{	return m_nCurType;		};
	enum DrvMode GetCurDrvMode()
	{	return m_nCurDrvMode;	};
	enum PinPadLangType GetCurLangType()
	{	return m_nCurLangType;	};

	enum PinPadLangType GetLangCodeByChar( TCHAR cLang );

	HANDLE GetCurHandle()
	{	return m_hDev;	};

	void SetLogObject( LPLOGGENCTL lpLogCtlObj )//set log object pointer
	{	m_lpLogCtlObj=lpLogCtlObj;	};

protected:
	TCHAR m_sIniChk[INI_CHK_BUF_LEN];
	BYTE m_cCurTransaction;//digit of current transaction
	
	int m_nOutReportSize;
	int m_nInReportSize;

	//int  m_HID_Pin_PIDs[MAX_HID_PINPAD_PID_NUM];

	BOOL m_bIsHID;//Is HID device?
	BYTE m_sATR[MAX_ATR_BUF_SIZE];
	int m_nATR;//the number of ATR
	BOOL m_IsInsertedICC;
	TCHAR m_ssLangDescriptin[NUMBER_LANG_TYPE][DESCRIPTION_BUF_LENGTH];
	TCHAR m_ssHWDescriptin[NUMBER_HW_TYPE][DESCRIPTION_BUF_LENGTH];
	TCHAR m_ssPinpadDescriptin[NUMBER_PINPAD_TYPE][DESCRIPTION_BUF_LENGTH];

	enum PinPadHWType m_nHWTypeMap[REGISTERED_PINPAD_NUMBER];
	enum DrvMode m_nDrvTypeMap[REGISTERED_PINPAD_NUMBER];
	enum PinPadType m_nTypeMap[REGISTERED_PINPAD_NUMBER];
	TCHAR m_ssRigsteredPinpad[REGISTERED_PINPAD_NUMBER][PINPAD_ID_BUF_SIZE];
	TCHAR m_sVersion[MAX_VERSION_BUF_SIZE];
	TCHAR *m_pV;//Version number starting point of m_sVersion
	TCHAR m_sDevName[_MAX_PATH];	//device full path
	HANDLE m_hDev;	//opened device handle

	int m_nNumConnectedDev;//the number of connected device.
	enum PinPadType m_nCurType;	//the type of opened device as customer
	enum DrvMode m_nCurDrvMode; //the driver mode of opened device as customer
	enum PinPadHWType m_nCurHWType; //the hardware-type type of opened device as customer
	enum PinPadLangType m_nCurLangType; //the Language type of opened device as customer

	int m_nMaxInstance;

	DWORD WriteHID( LPBYTE lpbData,DWORD *pdwSize );
	DWORD ReadHID( LPBYTE lpbData,DWORD *pdwSize );

	DWORD Write64( LPBYTE lpbData,DWORD *pdwSize );
	DWORD Read64( LPBYTE lpbData,DWORD *pdwSize );

	DWORD WriteV( LPBYTE lpbData,DWORD *pdwSize );
	DWORD ReadV( LPBYTE lpbData,DWORD *pdwSize );

	static HANDLE OpenByInterface(
			GUID* pClassGuid,	// points to the GUID that identifies the interface class
			DWORD instance,		// specifies which instance of the enumerated devices to open
			PDWORD pError		// address of variable to receive error status
			);

	DWORD GetVersionFromPinpad( TCHAR *szVersion=NULL );
	BOOL SetTypeMode();

	LPLOGGENCTL m_lpLogCtlObj;	//log control object pointer

};

#endif // !defined(AFX_EL_PIN_IO_BASE_H__F6B26DB0_08F6_485B_B4C6_7D8277C04312__INCLUDED_)
