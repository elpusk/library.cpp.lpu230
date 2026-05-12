// EL_Pin_IO_Base.cpp: implementation of the CEL_Pin_IO_Base class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EL_Pin_IO_Base.h"
#include "EL_Pin_ReCode.h"
#include "EL_DeviceInf.h"
#include "EL_PIN_ADevInterface.h"
#include "ELCrcChk.h"
#include <stdio.h>
#include "ELLogGen.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//Device path=\\?\hid#vid_134b&pid_0310#6&6d58be0&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}

//if PINPAD_SIMULATION_ENABLE is defined,
// pinpad' IO is redirected to virtual pinpad.(EL_PinSimX.exe:COM)
//#define	PINPAD_SIMULATION_ENABLE
//PINPAD_SIMULATION_ENABLE must be defined on Project settings->C/C++ tab->Preprocessor definition.

#if defined(PINPAD_SIMULATION_ENABLE)

//#include "objbase.h"
//#import "C:\work\Project\EL_PinSimX\EL_PinSimX.tlb" no_namespace, named_guids
//#import "C:\work\Project\EL_PinSimX\EL_PinSimX.tlb" no_namespace, named_guids

//simulation support function prototype
BOOL SimWriteHID( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize );
BOOL SimWrite64( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize );
BOOL SimWriteV( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize );
BOOL SimReadHID( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize );
BOOL SimRead64( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize );
BOOL SimReadV( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize );

HANDLE SimOpen( TCHAR *sDevPath );
BOOL SimClose();

IStream *gpPinSimServer=NULL;//virtual pinpad device.......

#define	CV_PINPAD_HANDLE		0x1004	//virtual pinpad handle
#endif//PINPAD_SIMULATION_ENABLE

#define	MAX_HID_FOUND_NUM		30
#define	MAX_DEVICE_NAME_TYPE	2

GUID gPINClassGuid=KICPIN_ADevice_CLASS_GUID; // Class GUID used to open device	
GUID gAllUSBClassGuid=ALL_USB_CLASS_GUID;


TCHAR gDevNameFormatType[][50]=
	{
		_T("\\\\.\\K1T_PINA%d"),
		_T("\\\\.\\KLP110UF%d")
	};

int  gnHID_Pin_PIDs[]={PINPAD_PID_DEF};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEL_Pin_IO_Base::CEL_Pin_IO_Base()
{
	//ini
	Initial();
}

CEL_Pin_IO_Base::~CEL_Pin_IO_Base()
{
#if defined(PINPAD_SIMULATION_ENABLE)
	CoUninitialize();
#endif//PINPAD_SIMULATION_ENABLE
}

void CEL_Pin_IO_Base::Initial()//call by constructure
{
	int i;

	m_lpLogCtlObj=NULL; //ini log object

	if( m_sIniChk[0]==_T('I') && 
		m_sIniChk[1]==_T('n') && 
		m_sIniChk[2]==_T('i') && 
		m_sIniChk[3]==_T('-') &&
		m_sIniChk[4]==_T('_') && 
		m_sIniChk[5]==_T('-') && 
		m_sIniChk[6]==_T('O') &&
		m_sIniChk[7]==_T('K') )
		return;//already called this function

	_tcscpy( m_sIniChk,_T("Ini-_-OK") );//save ini ok string

#if defined(PINPAD_SIMULATION_ENABLE)
	CLSID clsid;
	HRESULT hRes;

	CoInitializeEx(NULL,COINIT_MULTITHREADED );
	//CoInitialize(NULL);
	//Get COM server classID
	
	hRes = CLSIDFromProgID(OLESTR("EL_PinSimCOM.Object"),&clsid);
	if( FAILED(hRes) ){
		return;
	}
	
	hRes=CoCreateInstance(clsid,NULL, CLSCTX_LOCAL_SERVER,IID_IStream,(void**)&gpPinSimServer);

#endif//PINPAD_SIMULATION_ENABLE

	m_nOutReportSize=DEFAULT_REPORT_SIZE;
	m_nInReportSize=DEFAULT_REPORT_SIZE;

	memset( m_sDevName,0x00,sizeof(TCHAR)*_MAX_PATH);

	m_hDev=NULL;
	m_bIsHID=FALSE;

	m_nNumConnectedDev=0;
	m_nCurType=PINPAD_TYPE_UNKNOWN;
	m_nCurDrvMode=DRV_MODE_UNKNOWN;
	m_nCurHWType=PINPAD_HW_TYPE_UNKNOWN;
	m_nCurLangType=PINPAD_LANG_TYPE_UNKNOWN;

	m_nMaxInstance=DEFAULT_MAX_DEVICE_INSTANCE;

	memset( m_sVersion,0x00,sizeof(m_sVersion) );

	m_IsInsertedICC=FALSE;

	//
	for( i=0; i<MAX_HID_PINPAD_PID_NUM; i++ )
		gnHID_Pin_PIDs[i]=0;//reset HID pinpad PID arrary

	//initial HID pinpad PID arrary
	gnHID_Pin_PIDs[0]=PINPAD_PID_DEF;


	//
	memset( m_sATR,0x00,MAX_ATR_BUF_SIZE );
	m_nATR=0;//the number of ATR


	//set registered pinpad ID
	for( i=0; i<REGISTERED_PINPAD_NUMBER; i++ ){
		memset( m_ssRigsteredPinpad[i],0x00,sizeof(TCHAR)*PINPAD_ID_BUF_SIZE );
	}//end for

	for( i=0; i<NUMBER_LANG_TYPE; i++ )
		memset( m_ssLangDescriptin[i],0x00,sizeof(TCHAR)*DESCRIPTION_BUF_LENGTH );
	for( i=0; i<NUMBER_HW_TYPE; i++ )
		memset( m_ssLangDescriptin[i],0x00,sizeof(TCHAR)*DESCRIPTION_BUF_LENGTH );
	for( i=0; i<NUMBER_PINPAD_TYPE; i++ )
		memset( m_ssLangDescriptin[i],0x00,sizeof(TCHAR)*DESCRIPTION_BUF_LENGTH );

	/////////////////////////////////
	//set description tables.......
	_tcscpy(	m_ssLangDescriptin[PINPAD_LANG_TYPE_UNKNOWN],_T("Unknown language type") );
	_tcscpy(	m_ssLangDescriptin[PINPAD_LANG_TYPE_00],_T("Korean type") );
	_tcscpy(	m_ssLangDescriptin[PINPAD_LANG_TYPE_01],_T("English type") );

	_tcscpy(	m_ssHWDescriptin[PINPAD_HW_TYPE_UNKNOWN],_T("Unknown hardware type") );
	_tcscpy(	m_ssHWDescriptin[PINPAD_HW_TYPE_00],_T("USB and None voice hardware type(KLP110U)") );//110U-USB 음성무
	_tcscpy(	m_ssHWDescriptin[PINPAD_HW_TYPE_01],_T("USB+RS232 and None voice hardware type(KLP110UR)") );//110UR USB+RS232 음성무
	_tcscpy(	m_ssHWDescriptin[PINPAD_HW_TYPE_02],_T("USB+RS232 and Voice hardware type(KLP110UR2)") );//110UR2 USB+RS232 음성유
	_tcscpy(	m_ssHWDescriptin[PINPAD_HW_TYPE_03],_T("USB and Voice hardware type(KLP140U)") );//140U-USB	음성유

	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_UNKNOWN],_T("Unknown pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_00],_T("서울 저축은행 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_01],_T("신한은행 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_02],_T("프라임상호저축은행 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_03],_T("한신저축은행 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_04],_T("대구은행 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_05],_T("이비카드(시외버스터미날) pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_06],_T("굿모닝 신한증권 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_07],_T("신협 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_08],_T("에카뱅크(토마토저축은행) pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_09],_T("한국증권금융 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_10],_T("서울증권 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_11],_T("수협은행 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_12],_T("우리은행 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_13],_T("유화증권 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_14],_T("하나은행 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_15],_T("서울선물 / KR선물 pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_16],_T("상호저축은행(FSB) pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_17],_T("Sample pinpad type") );
	_tcscpy(	m_ssPinpadDescriptin[PINPAD_TYPE_18],_T("삼성화재 or Basic pinpad type") );

	//TCHAR m_ssPinpadDescriptin[NUMBER_PINPAD_TYPE][DESCRIPTION_BUF_LENGTH];


	/* 2007.04.03-2007.07.25
											0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15
	00서울저축은행		KLP110U-S2 V2.11	K	L	P	1	1	0	U	-	S	2		V	2	.	1	1
	01신한은행			KLP110U-S  V2.36	K	L	P	1	1	0	U	-	S		V	2	.	3	6	
	02프라임상호저축은행KLP110U-P  V1.00	K	L	P	1	1	0	U	-	P		V	1	.	0	0	
	03한신저축은행		KLP110U-S2 V2.11	K	L	P	1	1	0	U	-	S	2		V	2	.	1	1
	04대구은행			KLP110U-D V1.04		K	L	P	1	1	0	U	-	D		V	1	.	0	4	
	05시외버스터미널		KLP110UEB V1.05	K	L	P	1	1	0	U	E	B		V	1	.	0	5	
	06굿모닝신한증권		KLP110UGM V1.05	K	L	P	1	1	0	U	G	M		V	1	.	0	5	
	07신협				KLP110UR  V1.05		K	L	P	1	1	0	U	R			V	1	.	0	5	
	08신협				KLP110UR  V2.08b	K	L	P	1	1	0	U	R			V	2	.	0	8	b
	09토마토저축은행		KLP110U-T V1.06	K	L	P	1	1	0	U	-	T		V	1	.	0	6	
	10한국증권금융		KLP110UR  V1.00H	K	L	P	1	1	0	U	R			V	1	.	0	0	H
	11서울증권			KLP110URs V1.01		K	L	P	1	1	0	U	R	s		V	1	.	0	1	
	12수협				KLP110UF  V1.07		K	L	P	1	1	0	U	F			V	1	.	0	7	
	13우리은행			KLP110UWR V1.04		K	L	P	1	1	0	U	W	R		V	1	.	0	4	
	14유화증권			KLP110UU  V1.07		K	L	P	1	1	0	U	U	U		V	1	.	0	7	
	15하나은행			KLP110UH  V1.12		K	L	P	1	1	0	U	H			V	1	.	1	2	
	16서울선물			KLP110USF V1.02		K	L	P	1	1	0	U	S	F		V	1	.	0	2	
	17상호저축은행(FSB)	KLP110Uf  V2.00		K	L	P	1	1	0	U	f			V	2	.	0	0	
	18상호저축은행(FSB)	KLP110URf V1.05		K	L	P	1	1	0	U	R	f		V	1	.	0	5	
	19우리은행(음성)	KLP140UWR V1.01		K	L	P	1	4	0	U	W	R		V	1	.	0	1
	20신협(차세대)		KLP110URO V1.06		K	L	P	1	1	0	U	R	O		V	1	.	0	6	
	21신협(차세대)		KLP110URN V3.08b	K	L	P	1	1	0	U	R	N		V	3	.	0	8	b
	22sample pinpad		KLP110UEL V1.05 	K	L	P	1	1	0	U	E	L		V	1	.	0	5 
	23삼성화재			KLP140UEL V1.00 	K	L	P	1	4	0	U	E	L		V	1	.	0	0	


  	PINPAD_HW_TYPE_00,	//110U		USB	음성무
	PINPAD_HW_TYPE_01,	//110UR		USB+RS232	음성무
	PINPAD_HW_TYPE_02,	//110UR2	USB+RS232	음성유
	PINPAD_HW_TYPE_03	//140U		USB	음성유

	*/

	i=0;
	_tcscpy( m_ssRigsteredPinpad[i],_T("110U-S2 ") );//00서울저축은행
	m_nDrvTypeMap[i]=DRV_MODE_M_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_00;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110U-S ") );//01신한은행
	m_nDrvTypeMap[i]=DRV_MODE_M_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_01;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110U-P ") );//02프라임상호저축은행
	m_nDrvTypeMap[i]=DRV_MODE_M_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_02;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110U-S2 ") );//03한신저축은행
	m_nDrvTypeMap[i]=DRV_MODE_M_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_03;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110U-D ") );//04대구은행
	m_nDrvTypeMap[i]=DRV_MODE_M_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_04;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110UEB ") );//05시외버스터미널
	m_nDrvTypeMap[i]=DRV_MODE_A_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_05;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110UGM ") );//06굿모닝신한증권
	m_nDrvTypeMap[i]=DRV_MODE_A_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_06;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110UR  ") );//07신협
	m_nDrvTypeMap[i]=DRV_MODE_A_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_07;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_01;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110UR  ") );//08신협
	m_nDrvTypeMap[i]=DRV_MODE_A_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_07;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_02;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110U-T ") );//09토마토저축은행
	m_nDrvTypeMap[i]=DRV_MODE_A_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_08;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110UR  ") );//10한국증권금융
	m_nDrvTypeMap[i]=DRV_MODE_A_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_09;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_01;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110URs ") );//11서울증권
	m_nDrvTypeMap[i]=DRV_MODE_A_VARIABLE;
	m_nTypeMap[i]=PINPAD_TYPE_10;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_01;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110UF  ") );//12수협
	m_nDrvTypeMap[i]=DRV_MODE_A_VARIABLE;
	m_nTypeMap[i]=PINPAD_TYPE_11;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110UWR ") );//13우리은행
	m_nDrvTypeMap[i]=DRV_MODE_A_VARIABLE;
	m_nTypeMap[i]=PINPAD_TYPE_12;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110UU  ") );//14유화증권
	m_nDrvTypeMap[i]=DRV_MODE_A_VARIABLE;
	m_nTypeMap[i]=PINPAD_TYPE_13;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110UH  ") );//15하나은행
	m_nDrvTypeMap[i]=DRV_MODE_A_VARIABLE;
	m_nTypeMap[i]=PINPAD_TYPE_14;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110USF ") );//16서울선물
	m_nDrvTypeMap[i]=DRV_MODE_A_VARIABLE;
	m_nTypeMap[i]=PINPAD_TYPE_15;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110Uf  ") );//17상호저축은행(FSB)
	m_nDrvTypeMap[i]=DRV_MODE_A_VARIABLE;
	m_nTypeMap[i]=PINPAD_TYPE_16;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110URf ") );//18상호저축은행(FSB)
	m_nDrvTypeMap[i]=DRV_MODE_A_VARIABLE;
	m_nTypeMap[i]=PINPAD_TYPE_16;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_02;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("140UWR ") );//19우리은행(음성)
	m_nDrvTypeMap[i]=DRV_MODE_A_VARIABLE;
	m_nTypeMap[i]=PINPAD_TYPE_12;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_03;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110URO ") );//20신협(차세대)-old
	m_nDrvTypeMap[i]=DRV_MODE_A_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_07;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_01;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110URN ") );//21신협(차세대)-new
	m_nDrvTypeMap[i]=DRV_MODE_A_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_07;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_02;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("110UEL ") );//22sample pinpad
	m_nDrvTypeMap[i]=DRV_MODE_A_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_17;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_00;
	i++;

	_tcscpy( m_ssRigsteredPinpad[i],_T("140UEL ") );//삼성화재 or Basic pinpad
	m_nDrvTypeMap[i]=DRV_MODE_A_ONLY_64;
	m_nTypeMap[i]=PINPAD_TYPE_18;
	m_nHWTypeMap[i]=PINPAD_HW_TYPE_03;
	i++;

}

BOOL CEL_Pin_IO_Base::IsOpen()
{
	if( m_hDev )
		return TRUE;
	else
		return FALSE;
}

DWORD CEL_Pin_IO_Base::Open( TCHAR *sDevPath/*=NULL*/ )//open device
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	DWORD dwError;
	int i,j;
	int nInstance=0;
	int nNum=0;

	TCHAR path[50];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR sHIDPreFix[]=_T("\\\\?\\hid");//HID device path prefix
	TCHAR *psHIDPath=NULL;
	TCHAR *psCopyDevPath=NULL;
	PEL_HIDINFO pHidInfos=NULL;
#ifdef UNICODE 
	CHAR strTmp[2*(_MAX_PATH+1)];
#endif

	//
	if( m_hDev ){
		return KLP110UF_W_ALEADY_OPEN;
	}
	//
	if( sDevPath ){

		psCopyDevPath=_tcslwr( _tcsdup( sDevPath ) );
		psHIDPath=_tcsstr( psCopyDevPath,sHIDPreFix );
		if( psHIDPath!=psCopyDevPath ){
			//Not found HID device path
			m_bIsHID=FALSE;
			free( psCopyDevPath );
			psCopyDevPath=NULL;

			memset( dir,0x00,_MAX_DIR*sizeof(TCHAR) );
			memset( fname,0x00,_MAX_DIR*sizeof(TCHAR) );
			_tsplitpath( sDevPath,NULL, dir, fname,NULL);

			if( _tcscmp(sDevPath,_T("") )!=0 ){
				if( dir ){
					if( fname==NULL )
						return KLP110UF_E_INVAILD_PARA;
					//
#if !defined(PINPAD_SIMULATION_ENABLE)
					m_hDev=CreateFile( sDevPath,GENERIC_READ | GENERIC_WRITE,
									  0, NULL, OPEN_EXISTING, 0, NULL);
#else
					m_hDev=SimOpen( (TCHAR*)sDevPath );
#endif//PINPAD_SIMULATION_ENABLE
					//
					if( m_hDev==INVALID_HANDLE_VALUE ){
						m_hDev=NULL;
						return KLP110UF_E_INVALID_HANDLE;
					}
				}
				else{//interface only device(PINPAD_TYPE_04)

			#ifdef UNICODE
				wcstombs(strTmp, (const wchar_t *)sDevPath, sizeof(strTmp)); 
				nInstance=atoi( strTmp );
			#else
				nInstance=atoi( sDevPath );
			#endif


#if !defined(PINPAD_SIMULATION_ENABLE)
					m_hDev=OpenByInterface( &gPINClassGuid,(DWORD)nInstance, &dwError );
#else
					m_hDev=SimOpen( (TCHAR*)sDevPath );
#endif//PINPAD_SIMULATION_ENABLE
					//
					if( m_hDev==INVALID_HANDLE_VALUE ){
						m_hDev=NULL;
						return KLP110UF_E_INVALID_HANDLE;
					}
					//success open device(taegu bank)
				}
			}
			else{//interface only device(PINPAD_TYPE_04)
				nInstance=1;
#if !defined(PINPAD_SIMULATION_ENABLE)
				m_hDev=OpenByInterface( &gPINClassGuid,(DWORD)nInstance, &dwError );
#else
				m_hDev=SimOpen( (TCHAR*)sDevPath );
#endif//PINPAD_SIMULATION_ENABLE
				//
				if( m_hDev==INVALID_HANDLE_VALUE ){
					m_hDev=NULL;
					return KLP110UF_E_INVALID_HANDLE;
				}
				//success open device(taegu bank)
			}
		}
		else{//found HID device path
#if !defined(PINPAD_SIMULATION_ENABLE)
			m_hDev=CreateHIDDev( psCopyDevPath );
#else
			m_hDev=SimOpen( (TCHAR*)psCopyDevPath );
#endif//PINPAD_SIMULATION_ENABLE
			if( m_hDev==INVALID_HANDLE_VALUE ){
				m_hDev=NULL;
				dwResult=KLP110UF_E_INVALID_HANDLE;

				m_bIsHID=FALSE;
				free( psCopyDevPath );
				psCopyDevPath=NULL;
				return dwResult;
			}
			else{//success open HID device
				//Get report size
#if !defined(PINPAD_SIMULATION_ENABLE)
				WriteHIDReport( m_hDev,NULL,0,(LPDWORD)&m_nOutReportSize,NULL ); 
				m_nOutReportSize--;//remove report ID size
				ReadHIDReport( m_hDev,NULL,0,(LPDWORD)&m_nInReportSize,NULL ); 
				m_nInReportSize--;//remove report ID size
#else
				m_nOutReportSize=MAX_PACKET_SIZE;
				m_nInReportSize=MAX_PACKET_SIZE;
#endif//PINPAD_SIMULATION_ENABLE

				_tcscpy( m_sDevName,psCopyDevPath );
				m_bIsHID=TRUE;
				dwResult=KLP110UF_S_SUCCESS;
			}

			free( psCopyDevPath );
			psCopyDevPath=NULL;
		}

	}
	else{//auto search and open device
		for( j=0; j<MAX_DEVICE_NAME_TYPE; j++ ){
			for( i=0; i<m_nMaxInstance;i++ ){
				memset( path,0,50*sizeof(TCHAR) );
				_stprintf( path,gDevNameFormatType[j],i );
				//
#if !defined(PINPAD_SIMULATION_ENABLE)
				m_hDev=CreateFile( path,
								  GENERIC_READ | GENERIC_WRITE,
								  0,
								  NULL,
								  OPEN_EXISTING,
								  0,
								  NULL);
#else
				m_hDev=SimOpen( (TCHAR*)path );
#endif//PINPAD_SIMULATION_ENABLE
				//
				if( m_hDev!=INVALID_HANDLE_VALUE ){
					j=MAX_DEVICE_NAME_TYPE;//exit j-for
					break;
				}
			}//end for
		}

		if( i==m_nMaxInstance ){
			//Taegu device search and open.......
			for( i=0; i<m_nMaxInstance; i++ ){
#if !defined(PINPAD_SIMULATION_ENABLE)
				m_hDev=OpenByInterface( &gPINClassGuid,(DWORD)i, &dwError );
#else
				sprintf( path,"%d",i );//create virtual path
				m_hDev=SimOpen( (TCHAR*)path );
#endif//PINPAD_SIMULATION_ENABLE
				//
				if( m_hDev!=INVALID_HANDLE_VALUE )
					break;
			}
			//
			if( i>=m_nMaxInstance ){
				for( i=0; i<m_nMaxInstance+50; i++ ){
#if !defined(PINPAD_SIMULATION_ENABLE)
					m_hDev=OpenByInterface( &gAllUSBClassGuid,(DWORD)i, &dwError );
#else
					sprintf( path,"%d",i );//create virtual path
					m_hDev=SimOpen( (TCHAR*)path );
#endif//PINPAD_SIMULATION_ENABLE
					if( m_hDev!=INVALID_HANDLE_VALUE )
						break;
				}
				if( i>=m_nMaxInstance+50 ){
					//failure opening TaeGu device
					//try all HID device. part
					pHidInfos=(PEL_HIDINFO)malloc( sizeof(EL_HIDINFO)*MAX_HID_FOUND_NUM );
					if( pHidInfos==NULL ){
						m_bIsHID=FALSE;
						m_hDev=NULL;
						return KLP110UF_E_INVALID_HANDLE;
					}

					memset( pHidInfos,0x00,sizeof(EL_HIDINFO)*MAX_HID_FOUND_NUM );
#if !defined(PINPAD_SIMULATION_ENABLE)
					nNum=FindHIDDevices( 0,0,pHidInfos );
#else
					nNum=1;
					pHidInfos[0].VID=PINPAD_VID;
					pHidInfos[0].PID=PINPAD_PID_DEF;

					//this path is virtual path
					_tcscpy( pHidInfos[0].sPath,"\\\\?\\hid#vid_134b&pid_0310#6&6d58be0&0&0000#{00000000-0000-0000-0000-001122334455}" );
					//pHidInfos[0].HidCaps
#endif//PINPAD_SIMULATION_ENABLE
					if( nNum<=0 ){//NOt found HID device
						m_bIsHID=FALSE;
						free( pHidInfos );
						m_hDev=NULL;
						return KLP110UF_E_INVALID_HANDLE;
					}

					for( i=0; i<nNum; i++ ){
						for( j=0; j<MAX_HID_PINPAD_PID_NUM; j++ ){
							if( (pHidInfos[i].VID==PINPAD_VID) && (pHidInfos[i].PID==gnHID_Pin_PIDs[j]) ){
								break;
							}
						}//end for

						if( j!=MAX_HID_PINPAD_PID_NUM ){
							//found HID pinpad
							break;
						}
					}//end for

					if( i!=nNum ){
						//found HID pinpad
#if !defined(PINPAD_SIMULATION_ENABLE)
						m_hDev=CreateHIDDev( pHidInfos[i].sPath );
#else
						m_hDev=SimOpen( (TCHAR*)pHidInfos[i].sPath );
#endif//PINPAD_SIMULATION_ENABLE
						if( m_hDev==INVALID_HANDLE_VALUE ){
							free( pHidInfos );
							m_bIsHID=FALSE;
							m_hDev=NULL;
							return KLP110UF_E_INVALID_HANDLE;
						}
						else{//success open HID device
							//Get report size
#if !defined(PINPAD_SIMULATION_ENABLE)
							WriteHIDReport( m_hDev,NULL,0,(LPDWORD)&m_nOutReportSize,NULL ); 
							m_nOutReportSize--;//remove report ID size
							ReadHIDReport( m_hDev,NULL,0,(LPDWORD)&m_nInReportSize,NULL ); 
							m_nInReportSize--;//remove report ID size
#else
							m_nOutReportSize=MAX_PACKET_SIZE;
							m_nInReportSize=MAX_PACKET_SIZE;
#endif//PINPAD_SIMULATION_ENABLE

							_tcscpy( m_sDevName,pHidInfos[i].sPath );
							free( pHidInfos );
							m_bIsHID=TRUE;
							dwResult=KLP110UF_S_SUCCESS;
						}

					}
					else{//Not found HID pinpad device
						free( pHidInfos );
						m_bIsHID=FALSE;
						m_hDev=NULL;
						return KLP110UF_E_INVALID_HANDLE;
					}
				}
				else
					m_bIsHID=FALSE;
			}
			else
				m_bIsHID=FALSE;

			//success open device(taegu bank)
		}
		else
			m_bIsHID=FALSE;
	}
	//
	if( dwResult==KLP110UF_S_SUCCESS ){
		if( m_bIsHID )
			m_nCurDrvMode=DRV_MODE_HID;
		//
		dwResult=GetVersionFromPinpad();
		if( dwResult==KLP110UF_S_SUCCESS ){
			SetTypeMode();
		}
		else if( dwResult==KLP110UF_W_ZERO_RESP ){
			//this model dosen't support GetVersion commnad.
			dwResult=KLP110UF_S_SUCCESS;
			//
			if( m_bIsHID ){
				m_nCurType=PINPAD_TYPE_UNKNOWN;
				m_nCurDrvMode=DRV_MODE_HID;
				m_nCurHWType=PINPAD_HW_TYPE_04;//HID USB type
				m_nCurLangType=PINPAD_LANG_TYPE_UNKNOWN;
				m_pV=NULL;
			}
			else{
				m_nCurType=PINPAD_TYPE_UNKNOWN;
				m_nCurDrvMode=DRV_MODE_N_ONLY_64;
				m_nCurHWType=PINPAD_HW_TYPE_UNKNOWN;
				m_nCurLangType=PINPAD_LANG_TYPE_UNKNOWN;
				m_pV=NULL;
			}
		}
		else{//failure GetVersion from pinpad. therefore closing handle of pinpad.
			Close();
			dwResult=KLP110UF_E_INVALID_HANDLE;
		}
	}
	//
	return dwResult;
}

DWORD CEL_Pin_IO_Base::Close()//close current opened device
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//
	if( m_hDev ){
#if !defined(PINPAD_SIMULATION_ENABLE)
		if( m_bIsHID )
			CloseHIDDev(m_hDev);
		else
			CloseHandle(m_hDev);
#else
		SimClose();
#endif//PINPAD_SIMULATION_ENABLE
		m_hDev=NULL;
	}

	m_bIsHID=FALSE;
	//
	return dwResult;
}

DWORD CEL_Pin_IO_Base::Write( BYTE *pData/*=NULL*/,int *pnSize/*=NULL*/ )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BOOL bSelMode=FALSE;
	int nSize;
	//
	if( m_hDev==NULL )
		return KLP110UF_E_INVAILD_PARA;
	//
	if( pData==NULL && pnSize==NULL ){
		//Write 64 bytes(zero)
		bSelMode=TRUE;
		pnSize=&nSize;

		nSize=MAX_PACKET_SIZE;

		pData=(BYTE*)malloc( sizeof(BYTE)*MAX_PACKET_SIZE );
		if( pData==NULL ){
			dwResult=KLP110UF_E_UNKOWN;
			return dwResult;
		}
		memset( pData,0x00,MAX_PACKET_SIZE );//reset 
	}
	else if( pData==NULL || pnSize==NULL ){
		dwResult=KLP110UF_E_INVAILD_PARA;
		return dwResult;
	}

	//send data
	if( *pnSize<0 ){
		if( bSelMode ){
			if( pData )
				free( pData );
		}
		return KLP110UF_E_INVAILD_PARA;
	}
	//
	if( *pnSize>0 ){
		switch(m_nCurDrvMode){
			case DRV_MODE_N_ONLY_64:
			case DRV_MODE_M_ONLY_64:
			case DRV_MODE_A_ONLY_64:
			case DRV_MODE_UNKNOWN:
				dwResult=Write64( pData,(unsigned long *)pnSize );
				break;
			case DRV_MODE_N_VARIABLE:
			case DRV_MODE_M_VARIABLE:
			case DRV_MODE_A_VARIABLE:
				dwResult=WriteV( pData,(unsigned long *)pnSize );
				break;
			case DRV_MODE_HID:
				dwResult=WriteHID( pData,(unsigned long *)pnSize );
				break;
			default:
				dwResult=KLP110UF_E_INVAILD_PARA;
				break;
		}//end switch
	}

	//post processing
	if( bSelMode ){
		if( pData )
			free( pData );
	}

	return dwResult;
}


DWORD CEL_Pin_IO_Base::Write64( LPBYTE lpbData,DWORD *pdwSize )
{
	DWORD dwWSize=0;
	DWORD dwTotalWSize=0;
	DWORD dwError;
	BOOL bISEnd=FALSE;
	BYTE pTXBuf[300];
	BOOL bResult;
	//
	//
	do{
		memset( pTXBuf,0x00,300 );
		//
		if( (*pdwSize-dwTotalWSize )>MAX_PACKET_SIZE ){
			memcpy( pTXBuf,&(lpbData[dwTotalWSize]),MAX_PACKET_SIZE );
		}
		else{
			memcpy( pTXBuf,&(lpbData[dwTotalWSize]),*pdwSize-dwTotalWSize );
		}
		//
#if !defined(PINPAD_SIMULATION_ENABLE)
		bResult=WriteFile(m_hDev, pTXBuf, MAX_PACKET_SIZE, &dwWSize, NULL);
#else
		bResult=SimWrite64( pTXBuf,MAX_PACKET_SIZE,&dwWSize );
#endif//PINPAD_SIMULATION_ENABLE
		if( bResult ){
			dwTotalWSize+=dwWSize;
			//
			if( *pdwSize> dwTotalWSize ){
				memset( pTXBuf,0x00,300 );
			}
			else if( *pdwSize==dwTotalWSize ){//fit end case
				*pdwSize=dwTotalWSize;
				return KLP110UF_S_SUCCESS;
			}
			else{//greater end case
				return KLP110UF_S_SUCCESS;
			}
		}
		else{
			dwError=GetLastError();
			if( dwError==ERROR_DEVICE_REMOVED || dwError==ERROR_DEVICE_NOT_CONNECTED){
				Close();
				//Win32 Error code ERROR_DEVICE_REMOVED
				// is NT_STATUS code STATUS_DEVICE_REMOVED.
				return KLP110UF_E_REMOVED_DEVICE;
			}

			Close();
			return KLP110UF_E_WRITE_FILE;
		}
	}while( !bISEnd );
	//
	return KLP110UF_E_UNKOWN;
}

DWORD CEL_Pin_IO_Base::WriteV( LPBYTE lpbData,DWORD *pdwSize )
{
	DWORD dwWSize=0;
	DWORD dwError;
	BOOL bResult;
	//
	dwWSize=*pdwSize;
	//
#if !defined(PINPAD_SIMULATION_ENABLE)
	bResult=WriteFile(m_hDev, lpbData, dwWSize, pdwSize, NULL);
#else
	bResult=SimWriteV( lpbData,dwWSize,pdwSize );
#endif//PINPAD_SIMULATION_ENABLE
	if( bResult ){
		return KLP110UF_S_SUCCESS;
	}
	else{
		dwError=GetLastError();
		if( dwError==ERROR_DEVICE_REMOVED || dwError==ERROR_DEVICE_NOT_CONNECTED){
			Close();
			//Win32 Error code ERROR_DEVICE_REMOVED
			// is NT_STATUS code STATUS_DEVICE_REMOVED.
			return KLP110UF_E_REMOVED_DEVICE;
		}

		Close();
		return KLP110UF_E_WRITE_FILE;
	}
}

DWORD CEL_Pin_IO_Base::WriteHID( LPBYTE lpbData,DWORD *pdwSize )
{
	DWORD dwWSize=0;
	DWORD dwTotalWSize=0;
	DWORD dwError;
	BOOL bISEnd=FALSE;
	BYTE pTXBuf[300];
	BYTE cReportID=0;
	BOOL bResult;
	//
	//
	do{
		memset( pTXBuf,0x00,300 );
		//
		if( (*pdwSize-dwTotalWSize )>(DWORD)m_nOutReportSize ){
			pTXBuf[0]=cReportID;
			memcpy( &(pTXBuf[1]),&(lpbData[dwTotalWSize]),m_nOutReportSize );
		}
		else{
			pTXBuf[0]=cReportID;
			memcpy( &(pTXBuf[1]),&(lpbData[dwTotalWSize]),*pdwSize-dwTotalWSize );
		}
		//
#if !defined(PINPAD_SIMULATION_ENABLE)
		bResult=WriteHIDReport(m_hDev,pTXBuf,m_nOutReportSize+1,&dwWSize,NULL);
#else
		bResult=SimWriteHID( pTXBuf,m_nOutReportSize+1,&dwWSize );
#endif//PINPAD_SIMULATION_ENABLE
		if( bResult ){
			dwTotalWSize+=(dwWSize-1);//except report ID
			//
			if( *pdwSize> dwTotalWSize ){
				memset( pTXBuf,0x00,300 );
			}
			else if( *pdwSize==dwTotalWSize ){//fit end case
				*pdwSize=dwTotalWSize;
				return KLP110UF_S_SUCCESS;
			}
			else{//greater end case
				return KLP110UF_S_SUCCESS;
			}
		}
		else{
			dwError=GetLastError();
			if( dwError==ERROR_DEVICE_REMOVED || dwError==ERROR_DEVICE_NOT_CONNECTED){
				Close();
				//Win32 Error code ERROR_DEVICE_REMOVED
				// is NT_STATUS code STATUS_DEVICE_REMOVED.
				return KLP110UF_E_REMOVED_DEVICE;
			}

			Close();
			return KLP110UF_E_WRITE_FILE;
		}
	}while( !bISEnd );
	//
	return KLP110UF_E_UNKOWN;
}

DWORD CEL_Pin_IO_Base::Read( BYTE *pData/*=NULL*/,int *pnSize/*=NULL*/ )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BOOL bSelMode=FALSE;
	int nSize;
	//
	if( m_hDev==NULL )
		return KLP110UF_E_INVAILD_PARA;
	//
	if( pData==NULL && pnSize==NULL ){
		//read 64 bytes(zero)
		bSelMode=TRUE;
		pnSize=&nSize;

		nSize=MAX_PACKET_SIZE;

		pData=(BYTE*)malloc( sizeof(BYTE)*MAX_PACKET_SIZE );
		if( pData==NULL ){
			dwResult=KLP110UF_E_UNKOWN;
			return dwResult;
		}
		memset( pData,0x00,MAX_PACKET_SIZE );//reset 
	}
	else if( pData==NULL || pnSize==NULL ){
		dwResult=KLP110UF_E_INVAILD_PARA;
		return dwResult;
	}

	//
	//send data
	if( *pnSize<0 ){
		if( bSelMode ){
			if( pData )
				free( pData );
		}
		return KLP110UF_E_INVAILD_PARA;
	}
	//
	if( *pnSize>0 ){
		switch(m_nCurDrvMode){
			case DRV_MODE_N_ONLY_64:
			case DRV_MODE_M_ONLY_64:
			case DRV_MODE_A_ONLY_64:
			case DRV_MODE_UNKNOWN:
				dwResult=Read64( pData,(unsigned long *)pnSize );
				break;
			case DRV_MODE_N_VARIABLE:
			case DRV_MODE_M_VARIABLE:
			case DRV_MODE_A_VARIABLE:
				dwResult=ReadV( pData,(unsigned long *)pnSize );
				if( *pnSize>0 )
					Trace( m_lpLogCtlObj,2,_T("--Read : Ret(%d-0x%X)\n"),*pnSize,pData[0] );
				break;
			case DRV_MODE_HID:
				dwResult=ReadHID( pData,(unsigned long *)pnSize );
				break;
			default:
				dwResult=KLP110UF_E_INVAILD_PARA;
				break;
		}//end switch
	}

	//post processing
	if( bSelMode ){
		if( pData )
			free( pData );
	}

	return dwResult;
}

DWORD CEL_Pin_IO_Base::Read64( LPBYTE lpbData,DWORD *pdwSize )
{
	DWORD dwRSize=0;
	DWORD dwTotalRSize=0;
	BOOL bISEnd=FALSE;
	BYTE pRXBuf[300];
	DWORD dwError;
	BOOL bResult;
	//
	do{
		memset( pRXBuf,0x00,300 );

#if !defined(PINPAD_SIMULATION_ENABLE)
		bResult=ReadFile(m_hDev, pRXBuf, 64, &dwRSize, NULL);
#else
		bResult=SimRead64( pRXBuf,64,&dwRSize );
#endif//PINPAD_SIMULATION_ENABLE
		if( bResult ){
			if( dwRSize==0 ){//simulation Zero-64 bytes
				dwRSize=64;
				memset( pRXBuf,0x00,64 );
			}
			dwTotalRSize+=dwRSize;
			//
			if( *pdwSize> dwTotalRSize ){
				memcpy( &(lpbData[dwTotalRSize-64]),pRXBuf,64 );
			}
			else if( *pdwSize==dwTotalRSize ){//fit end case
				memcpy( &(lpbData[dwTotalRSize-64]),pRXBuf,64 );
				*pdwSize=dwTotalRSize;
				return KLP110UF_S_SUCCESS;
			}
			else{//greater end case
				memcpy( &(lpbData[dwTotalRSize-64]),pRXBuf,*pdwSize-(dwTotalRSize-64) );
				return KLP110UF_S_SUCCESS;
			}
		}
		else{
			dwError=GetLastError();
			//
			if( dwError==ERROR_DEVICE_REMOVED || dwError==ERROR_DEVICE_NOT_CONNECTED){
				Close();
				//Win32 Error code ERROR_DEVICE_REMOVED
				// is NT_STATUS code STATUS_DEVICE_REMOVED.
				return KLP110UF_E_REMOVED_DEVICE;
			}

			Close();
			return KLP110UF_E_READ_FILE;
		}
	}while( !bISEnd );
	//
	return KLP110UF_E_UNKOWN;
}

DWORD CEL_Pin_IO_Base::ReadV( LPBYTE lpbData,DWORD *pdwSize )
{
	DWORD dwRSize=0;
	DWORD dwTotalRSize=0;
	BOOL bISEnd=FALSE;
	DWORD dwError;
	BOOL bResult;
	//
	//
	memset( lpbData,0x00,*pdwSize );
	dwRSize=*pdwSize;
	//
#if !defined(PINPAD_SIMULATION_ENABLE)
	bResult=ReadFile(m_hDev, lpbData, dwRSize, pdwSize, NULL);
#else
	bResult=SimReadV( lpbData,dwRSize,pdwSize );
#endif//PINPAD_SIMULATION_ENABLE
	if( bResult ){
		return KLP110UF_S_SUCCESS;
	}
	else{
		dwError=GetLastError();
		//
		if( dwError==ERROR_DEVICE_REMOVED || dwError==ERROR_DEVICE_NOT_CONNECTED){
			Close();
			//Win32 Error code ERROR_DEVICE_REMOVED
			// is NT_STATUS code STATUS_DEVICE_REMOVED.
			return KLP110UF_E_REMOVED_DEVICE;
		}

		Close();
		return KLP110UF_E_READ_FILE;
	}
}

DWORD CEL_Pin_IO_Base::ReadHID( LPBYTE lpbData,DWORD *pdwSize )
{
	DWORD dwRSize=0;
	DWORD dwTotalRSize=0;
	BOOL bISEnd=FALSE;
	BYTE pRXBuf[300];
	DWORD dwError;
	BYTE cReportID=0;
	BOOL bResult;
	//
	do{
		memset( pRXBuf,0x00,300 );

		pRXBuf[0]=cReportID;

#if !defined(PINPAD_SIMULATION_ENABLE)
		bResult=ReadHIDReport(m_hDev,pRXBuf,m_nInReportSize+1,&dwRSize,NULL);
#else
		bResult=SimReadHID( pRXBuf,m_nInReportSize+1,&dwRSize );
#endif//PINPAD_SIMULATION_ENABLE
		if( bResult ){
			if( dwRSize==0 ){//simulation Zero-64 bytes
				dwRSize=m_nInReportSize+1;
				memset( pRXBuf,0x00,m_nInReportSize+1 );
			}
			dwTotalRSize+=(dwRSize-1);//except report ID;
			//
			if( *pdwSize> dwTotalRSize ){
				memcpy( &(lpbData[dwTotalRSize-(dwRSize-1)]),&pRXBuf[1],(dwRSize-1) );
			}
			else if( *pdwSize==dwTotalRSize ){//fit end case
				memcpy( &(lpbData[dwTotalRSize-(dwRSize-1)]),&pRXBuf[1],(dwRSize-1) );
				*pdwSize=dwTotalRSize;
				return KLP110UF_S_SUCCESS;
			}
			else{//greater end case
				memcpy( &(lpbData[dwTotalRSize-(dwRSize-1)]),&pRXBuf[1],*pdwSize-(dwTotalRSize-(dwRSize-1)) );
				return KLP110UF_S_SUCCESS;
			}
		}
		else{
			dwError=GetLastError();
			//
			if( dwError==ERROR_DEVICE_REMOVED || dwError==ERROR_DEVICE_NOT_CONNECTED){
				Close();
				//Win32 Error code ERROR_DEVICE_REMOVED
				// is NT_STATUS code STATUS_DEVICE_REMOVED.
				return KLP110UF_E_REMOVED_DEVICE;
			}

			Close();
			return KLP110UF_E_READ_FILE;
		}
	}while( !bISEnd );
	//
	return KLP110UF_E_UNKOWN;
}

//return connected pinpad devices list to string
// return value is the number of device on connected
// negative is error
int CEL_Pin_IO_Base::GetDevList( TCHAR *psDevList )
{
	int nDevCnt=0;
	int i,j;
	HANDLE hDev;
	TCHAR path[50];
	DWORD dwError;
	PEL_HIDINFO pHidInfos=NULL;
	int nNum=0;
	//
	if( psDevList==NULL ){
		//invalied parameter
		nDevCnt=-1;
		return nDevCnt;
	}
	//
	for( j=0; j<MAX_DEVICE_NAME_TYPE; j++ ){
		for( i=0; i<DEFAULT_MAX_DEVICE_INSTANCE;i++ ){
			memset( path,0x00,50*sizeof(TCHAR) );
			_stprintf( path,gDevNameFormatType[j],i );
			//
#if !defined(PINPAD_SIMULATION_ENABLE)
			hDev=CreateFile( path,
							  GENERIC_READ | GENERIC_WRITE,
							  0,
							  NULL,
							  OPEN_EXISTING,
							  0,
							  NULL);
#else
			hDev=SimOpen( (TCHAR*)path );
#endif//PINPAD_SIMULATION_ENABLE			
			//
			if( hDev!=INVALID_HANDLE_VALUE ){
				_tcscpy( psDevList,path );
#if !defined(PINPAD_SIMULATION_ENABLE)
				CloseHandle(hDev);
#else
				SimClose();
#endif//PINPAD_SIMULATION_ENABLE
				psDevList=psDevList+_tcslen(path)*sizeof(TCHAR);
				psDevList=psDevList+sizeof(TCHAR);
				*psDevList=NULL;
				nDevCnt++;
				break;
			}
		}//end for
	}

	//Taegu device search
	for( i=0; i<DEFAULT_MAX_DEVICE_INSTANCE; i++ ){
#if !defined(PINPAD_SIMULATION_ENABLE)
		hDev=OpenByInterface( &gPINClassGuid,(DWORD)i, &dwError );
#else
		hDev=SimOpen( (TCHAR*)path );
#endif//PINPAD_SIMULATION_ENABLE
		//
		if( hDev!=INVALID_HANDLE_VALUE ){
			memset( path,0x00,50*sizeof(TCHAR) );
			_stprintf( path,_T("%d"),i );
#if !defined(PINPAD_SIMULATION_ENABLE)
			CloseHandle(hDev);
#else
			SimClose();
#endif//PINPAD_SIMULATION_ENABLE
			psDevList=psDevList+_tcslen(path)*sizeof(TCHAR);
			psDevList=psDevList+sizeof(TCHAR);
			*psDevList=NULL;
			nDevCnt++;
		}
	}
	//
	//try all HID device. part
	pHidInfos=(PEL_HIDINFO)malloc( sizeof(EL_HIDINFO)*MAX_HID_FOUND_NUM );
	if( pHidInfos ){
		memset( pHidInfos,0x00,sizeof(EL_HIDINFO)*MAX_HID_FOUND_NUM );
#if !defined(PINPAD_SIMULATION_ENABLE)
		nNum=FindHIDDevices( 0,0,pHidInfos );
#else
		nNum=0;
#endif//PINPAD_SIMULATION_ENABLE
		if( nNum>0 ){//found HID device
			for( i=0; i<nNum; i++ ){
				for( j=0; j<MAX_HID_PINPAD_PID_NUM; j++ ){
					if( (pHidInfos[i].VID==PINPAD_VID) && (pHidInfos[i].PID==gnHID_Pin_PIDs[j]) ){
						//found HID pinpad device
						psDevList=psDevList+_tcslen(pHidInfos[i].sPath)*sizeof(TCHAR);
						psDevList=psDevList+sizeof(TCHAR);
						*psDevList=NULL;
						nDevCnt++;
					}
				}//end for
			}//end for
		}
		free( pHidInfos );
	}
	//
	return nDevCnt;
}

/*
	DRV_TYPE_UNKNOWN=0,//unknown type driver
	DRV_TYPE_K1T_PINA,//K1T_PINA type driver
	DRV_TYPE_KICPIN_A,//KICPIN_A type driver
	DRV_TYPE_KLP110U,//KLP110U type driver
	DRV_TYPE_SYS_HID//system default HID type driver
*/
//return connected pinpad devices list to string with type
int CEL_Pin_IO_Base::GetDevListEx( TCHAR *psDevList,enum DrvType Type )
{
	int nDevCnt=0;
	int i,j;
	HANDLE hDev;
	TCHAR path[50];
	DWORD dwError;
	PEL_HIDINFO pHidInfos=NULL;
	int nNum=0;
	//
	if( Type==DRV_TYPE_UNKNOWN )
		return GetDevList( psDevList );//search all device.......

	if( psDevList==NULL ){
		//invalied parameter
		nDevCnt=-1;
		return nDevCnt;
	}
	//
	if( Type==DRV_TYPE_K1T_PINA ){
		j=0;
		for( i=0; i<DEFAULT_MAX_DEVICE_INSTANCE;i++ ){
			memset( path,0x00,50*sizeof(TCHAR) );
			_stprintf( path,gDevNameFormatType[j],i );
			//
#if !defined(PINPAD_SIMULATION_ENABLE)
			hDev=CreateFile( path,
							  GENERIC_READ | GENERIC_WRITE,
							  0,
							  NULL,
							  OPEN_EXISTING,
							  0,
							  NULL);
#else
			hDev=SimOpen( (TCHAR*)path );
			if( hDev!=INVALID_HANDLE_VALUE ){
				hDev=(HANDLE)CV_PINPAD_HANDLE;
				sprintf( path,gDevNameFormatType[j],0 );
			}
			//
			i=DEFAULT_MAX_DEVICE_INSTANCE;//exit for 
#endif//PINPAD_SIMULATION_ENABLE			
			//
			if( hDev!=INVALID_HANDLE_VALUE ){
				_tcscpy( psDevList,path );
#if !defined(PINPAD_SIMULATION_ENABLE)
				CloseHandle(hDev);
#else
				SimClose();
#endif//PINPAD_SIMULATION_ENABLE
				psDevList=psDevList+_tcslen(path)*sizeof(TCHAR);
				psDevList=psDevList+sizeof(TCHAR);
				*psDevList=NULL;
				nDevCnt++;
				break;
			}
		}//end for

		return nDevCnt;
	}//Type==DRV_TYPE_K1T_PINA

	if( Type==DRV_TYPE_KLP110U ){
		j=1;
		for( i=0; i<DEFAULT_MAX_DEVICE_INSTANCE;i++ ){
			memset( path,0x00,50*sizeof(TCHAR) );
			_stprintf( path,gDevNameFormatType[j],i );
			//
#if !defined(PINPAD_SIMULATION_ENABLE)
			hDev=CreateFile( path,
							  GENERIC_READ | GENERIC_WRITE,
							  0,
							  NULL,
							  OPEN_EXISTING,
							  0,
							  NULL);
#else
			hDev=SimOpen( (TCHAR*)path );
			if( hDev!=INVALID_HANDLE_VALUE ){
				hDev=(HANDLE)CV_PINPAD_HANDLE;
				sprintf( path,gDevNameFormatType[j],0 );
			}
			//
			i=DEFAULT_MAX_DEVICE_INSTANCE;//exit for 
#endif//PINPAD_SIMULATION_ENABLE			
			//
			if( hDev!=INVALID_HANDLE_VALUE ){
				_tcscpy( psDevList,path );
#if !defined(PINPAD_SIMULATION_ENABLE)
				CloseHandle(hDev);
#else
				SimClose();
#endif//PINPAD_SIMULATION_ENABLE
				psDevList=psDevList+_tcslen(path)*sizeof(TCHAR);
				psDevList=psDevList+sizeof(TCHAR);
				*psDevList=NULL;
				nDevCnt++;
				break;
			}
		}//end for

		return nDevCnt;
	}//Type==DRV_TYPE_KLP110U

	if( Type==DRV_TYPE_KICPIN_A ){
		//Taegu device search
		for( i=0; i<DEFAULT_MAX_DEVICE_INSTANCE; i++ ){
#if !defined(PINPAD_SIMULATION_ENABLE)
			hDev=OpenByInterface( &gPINClassGuid,(DWORD)i, &dwError );
#else
			memset( path,0x00,50 );
			sprintf( path,"%d",i );

			hDev=SimOpen( (TCHAR*)path );
			//
			i=DEFAULT_MAX_DEVICE_INSTANCE;//exit for 
#endif//PINPAD_SIMULATION_ENABLE
			//
			if( hDev!=INVALID_HANDLE_VALUE ){
				memset( path,0x00,50*sizeof(TCHAR) );
				_stprintf( path,_T("%d"),i );
#if !defined(PINPAD_SIMULATION_ENABLE)
				CloseHandle(hDev);
#else
				SimClose();
#endif//PINPAD_SIMULATION_ENABLE
				_tcscpy( psDevList,path );
				psDevList=psDevList+_tcslen(path)*sizeof(TCHAR);
				psDevList=psDevList+sizeof(TCHAR);
				*psDevList=NULL;
				nDevCnt++;
			}
		}//end for

		return nDevCnt;
	}//Type==DRV_TYPE_KICPIN_A
	//

	if( Type==DRV_TYPE_SYS_HID ){
		//try all HID device. part
		pHidInfos=(PEL_HIDINFO)malloc( sizeof(EL_HIDINFO)*MAX_HID_FOUND_NUM );
		if( pHidInfos ){
			memset( pHidInfos,0x00,sizeof(EL_HIDINFO)*MAX_HID_FOUND_NUM );
#if !defined(PINPAD_SIMULATION_ENABLE)
			nNum=FindHIDDevices( 0,0,pHidInfos );
#else
			nNum=1;
			pHidInfos[0].VID=PINPAD_VID;
			pHidInfos[0].PID=PINPAD_PID_DEF;

			//this path is virstual path
			_tcscpy( pHidInfos[0].sPath,"\\\\?\\hid#vid_134b&pid_0310#6&6d58be0&0&0000#{00000000-0000-0000-0000-001122334455}" );
			//pHidInfos[0].HidCaps
#endif//PINPAD_SIMULATION_ENABLE
			if( nNum>0 ){//found HID device
				for( i=0; i<nNum; i++ ){
					for( j=0; j<MAX_HID_PINPAD_PID_NUM; j++ ){
						if( (pHidInfos[i].VID==PINPAD_VID) && (pHidInfos[i].PID==gnHID_Pin_PIDs[j]) ){
							//found HID pinpad device
							_tcscpy( psDevList,pHidInfos[i].sPath );
							psDevList=psDevList+_tcslen(pHidInfos[i].sPath)*sizeof(TCHAR);
							psDevList=psDevList+sizeof(TCHAR);
							*psDevList=NULL;
							nDevCnt++;
						}
					}//end for
				}//end for
			}
			free( pHidInfos );
		}

		return nDevCnt;
	}//DRV_TYPE_SYS_HID
	//
	return nDevCnt;
}


HANDLE CEL_Pin_IO_Base::OpenByInterface(
		GUID* pClassGuid,	// points to the GUID that identifies the interface class
		DWORD instance,		// specifies which instance of the enumerated devices to open
		PDWORD pError		// address of variable to receive error status
		)
{
	HANDLE hDev;
	CDevInterfaceObj DevObj(pClassGuid, pError);

	if (*pError != ERROR_SUCCESS)
		return INVALID_HANDLE_VALUE;

	CDevInterface DevInterface(&DevObj, instance, pError);

	if (*pError != ERROR_SUCCESS)
		return INVALID_HANDLE_VALUE;

	hDev = CreateFile(
		DevInterface.DevicePath(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	/********************************************
	hDev = CreateFile(
		DevInterface.DevicePath(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL
		);
	***********************************************/
	if (hDev == INVALID_HANDLE_VALUE)
		*pError = GetLastError();

	return hDev;
}

DWORD CEL_Pin_IO_Base::GetVersionFromPinpad( TCHAR *szVersion/*=NULL*/ )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE buf[100];
	int i;
	DWORD dwSize;
	DWORD dwResp;

#ifdef UNICODE
	TCHAR wbuf[100];
#endif
	//
	memset(buf, 0, 100);

	//PINPAD
	buf[0]='V';		//Get version command
	dwSize=10;

	dwResp=Write( buf,(int*)&dwSize );
	if( dwResp!=KLP110UF_S_SUCCESS ){
		return dwResp;
	}
	//

	memset(buf, 0, 100);
	buf[0]=0x00;
	//
	// V+length 4byte +5 bytes+16(version)
	dwSize=MAX_PACKET_SIZE;
	//
	for( i=0; i<5; i++ ){//retry 5times
		buf[0]=0x00;
		dwSize=MAX_PACKET_SIZE;
		//
		dwResp=Read( buf,(int*)&dwSize );
		if( dwResp!=KLP110UF_S_SUCCESS ){
			return dwResp;
		}
		//
		if( dwSize>25 && buf[0]=='V' ){
			buf[26]=NULL;	//make zero string
			//
			if( szVersion ){

			#ifdef UNICODE
				//Change Ascii to Unicode.
				MultiByteToWideChar( CP_ACP,0,(char*)&(buf[10]),-1,wbuf,100 );
				_tcscpy( szVersion,wbuf );//copy verision info
			#else
				_tcscpy( szVersion,(LPCTSTR)&(buf[10]) );//copy verision info
			#endif
				
			}
			else{//szVersion==NULL
				memset( m_sVersion,0x00,sizeof(TCHAR)*MAX_VERSION_BUF_SIZE );
			
			#ifdef UNICODE
				//Change Ascii to Unicode.
				MultiByteToWideChar( CP_ACP,0,(char*)&(buf[10]),-1,wbuf,100 );
				_tcscpy( m_sVersion,wbuf );//copy verision info
			#else
				_tcscpy( m_sVersion,(LPCTSTR)&(buf[10]) );//copy verision info
			#endif
	
			}
			//
			return dwResp;//success
		}
	}//end for

	return KLP110UF_W_ZERO_RESP;//Verion 1.1
}

BOOL CEL_Pin_IO_Base::SetTypeMode()
{
	int i;
	m_pV=NULL;

	if( m_hDev==NULL )
		return FALSE;
	
	//check prefix of version-string
	if( _tcsstr( m_sVersion,_T("KLP") )==NULL )
		return FALSE;//invalied version format
	//
	for( i=0;i<REGISTERED_PINPAD_NUMBER; i++ ){
		if( _tcsstr( m_sVersion,m_ssRigsteredPinpad[i] ) ){
			m_nCurType=m_nTypeMap[i];	//the type of opened device as customer
			m_nCurDrvMode=m_nDrvTypeMap[i]; //the driver mode of opened device as customer
			m_nCurHWType=m_nHWTypeMap[i]; //pinpad hardware type of opened device as customer
			//search OK

			//check for specific mode
			switch(m_nCurType){
				case PINPAD_TYPE_00://서울 저축은행,한신 저축은행
				case PINPAD_TYPE_03:
					m_pV=&(m_sVersion[PINPAD_VER_NUMBER_INDEX+1]);
					//impassible difference ?????
					m_nCurLangType=PINPAD_LANG_TYPE_00;
					break;
				case PINPAD_TYPE_07://신협
				case PINPAD_TYPE_09://한국 증권 금융 
					m_pV=&(m_sVersion[PINPAD_VER_NUMBER_INDEX]);
					if( _tcsstr( m_sVersion,_T("KLP110UR  V1.05") ) ){//구형 신협 미음성지원
						m_nCurType=PINPAD_TYPE_07;
						m_nCurHWType=PINPAD_HW_TYPE_01;
					}
					else if( _tcsstr( m_sVersion,_T("KLP110UR  V2.08b") ) ){//구형 신협 음성지원 
						m_nCurType=PINPAD_TYPE_07;
						m_nCurHWType=PINPAD_HW_TYPE_02;
					}
					else if( _tcsstr( m_sVersion,_T("KLP110UR  V1.00H") ) ){//한국 증권 금융 
						m_nCurType=PINPAD_TYPE_09;
						m_nCurHWType=PINPAD_HW_TYPE_01;
					}
					m_nCurLangType=PINPAD_LANG_TYPE_00;
					break;
				default:
					m_pV=&(m_sVersion[PINPAD_VER_NUMBER_INDEX]);
					m_nCurLangType=GetLangCodeByChar( m_pV[4] );
					break;
			}//end switch

			return TRUE;
		}
	}//end for

	//
	m_nCurType=PINPAD_TYPE_UNKNOWN;
	m_nCurDrvMode=DRV_MODE_UNKNOWN;
	m_nCurHWType=PINPAD_HW_TYPE_UNKNOWN;
	m_nCurLangType=PINPAD_LANG_TYPE_UNKNOWN;
	return FALSE;
}


DWORD CEL_Pin_IO_Base::DL_GetResponse( BYTE *lpdata,DWORD *pdwSize)
{
	int i;
	DWORD dwSize=*pdwSize;
	DWORD dwResult;
	//
	lpdata[0]=0x00;

	dwResult=Read( lpdata,(int*)pdwSize );

	if( dwResult==KLP110UF_S_SUCCESS ){
		for( i=0; i<10; i++ ){
			if( (*pdwSize==0) || (lpdata[0]!=DL_PINPAD_STX) ){
				lpdata[0]=0x00;
				*pdwSize=dwSize;
				//
				dwResult=Read( lpdata,(int*)pdwSize );
				if( dwResult!=KLP110UF_S_SUCCESS )
					return dwResult;
			}
			else{
				return KLP110UF_S_SUCCESS;
			}
		}
		return KLP110UF_E_INVAILD_RESP;//fail retry
	}
	else
		return dwResult;

}


//send download packet
DWORD CEL_Pin_IO_Base::DL_SendPacket(BYTE *pData, WORD wSize, WORD wAdd)
{
	BYTE Packet[300];
	DWORD dwSize;
	DWORD dwResult;
	//
	if( pData==NULL )
		return KLP110UF_E_INVAILD_PARA;
	//
	if( wSize==0 )
		return KLP110UF_E_INVAILD_PARA;
	//
	memset( Packet,0x00,300 );

	Packet[0]=DL_PINPAD_STX;
	Packet[1]='d';
	Packet[2]=HIBYTE( wAdd );
	Packet[3]=LOBYTE( wAdd );
	memcpy( &(Packet[4]),pData,wSize );
	Packet[4+wSize]=DL_PINPAD_ETX;
	Packet[5+wSize]=EL_MakeBCC( Packet,5+wSize );	//generate BCC
	dwSize=6+wSize;
	//
	dwResult=Write( Packet,(int*)&dwSize );
	return dwResult;
}

DWORD CEL_Pin_IO_Base::DL_SendStartPacket()
{
	BYTE Packet[300];
	DWORD dwSize;
	DWORD dwResult;
	//
	memset( Packet,0x00,300 );

	Packet[0]=DL_PINPAD_STX;
	Packet[1]='D';
	Packet[2]=0x00;
	Packet[3]=DL_PINPAD_ETX;
	Packet[4]=EL_MakeBCC( Packet,4 );	//generate BCC
	dwSize=5;
	//
	dwResult=Write( Packet,(int*)&dwSize );
	if( dwResult!=KLP110UF_S_SUCCESS )
		return dwResult;
	else{
		memset( Packet,0x00,300 );
		dwSize=64;
		dwResult=DL_GetResponse( Packet,&dwSize );
		if( dwResult!=KLP110UF_S_SUCCESS )
			return dwResult;
		else{
			if( Packet[0]==DL_PINPAD_STX &&
				Packet[1]=='D' &&
				Packet[2]==0x00 )
				return KLP110UF_S_SUCCESS;
			else
				return KLP110UF_E_INVAILD_RESP;
		}
	}
}

DWORD CEL_Pin_IO_Base::DL_SendStopPacket()
{
	BYTE Packet[300];
	DWORD dwSize;
	DWORD dwResult;
	//
	memset( Packet,0x00,300 );
	Packet[0]=DL_PINPAD_STX;
	Packet[1]='D';
	Packet[2]=0x01;
	Packet[3]=DL_PINPAD_ETX;
	Packet[4]=EL_MakeBCC( Packet,4 );	//generate BCC
	dwSize=5;
	//
	dwResult=Write( Packet,(int*)&dwSize );
	if( dwResult!=KLP110UF_S_SUCCESS )
		return dwResult;
	else{
		memset( Packet,0x00,300 );
		dwSize=64;
		dwResult=DL_GetResponse( Packet,&dwSize );
		if( dwResult!=KLP110UF_S_SUCCESS )
			return dwResult;
		else{
			if( Packet[0]==DL_PINPAD_STX &&
				Packet[1]=='D' &&
				Packet[2]==0x01 )
				return KLP110UF_S_SUCCESS;
			else
				return KLP110UF_E_INVAILD_RESP;
		}
	}
}

//compara given version string and current opened pinpad firmare veriosn.
enum PinPadComparae CEL_Pin_IO_Base::CompareVersion( TCHAR *sVersion )
{
	int i;
	TCHAR *pV=NULL;
	enum PinPadComparae eResult=PINPAD_COM_UNKNOWN;
	enum PinPadType nCurType;	//the type of sVersion
	enum DrvMode nCurDrvMode; //the driver mode of sVersion
	enum PinPadHWType nCurHWType; //the hardware-type type of sVersion
	enum PinPadLangType nCurLangType; //the Language type of sVersion

	if( sVersion==NULL )
		return eResult;
	if( m_hDev==NULL ){
		return eResult;
	}
	if( m_pV==NULL )
		return eResult;
	//
	//check prefix of version-string
	if( _tcsstr( sVersion,_T("KLP") )==NULL )
		return eResult;
	//
	nCurType=PINPAD_TYPE_UNKNOWN;
	nCurDrvMode=DRV_MODE_UNKNOWN;
	nCurHWType=PINPAD_HW_TYPE_UNKNOWN;
	nCurLangType=PINPAD_LANG_TYPE_UNKNOWN;

	for( i=0;i<REGISTERED_PINPAD_NUMBER; i++ ){
		if( _tcsstr( sVersion,m_ssRigsteredPinpad[i] ) ){
			nCurType=m_nTypeMap[i];	//the type of opened device as customer
			nCurDrvMode=m_nDrvTypeMap[i]; //the driver mode of opened device as customer
			nCurHWType=m_nHWTypeMap[i]; 
			//search OK

			//check for specific mode
			switch(nCurType){
				case PINPAD_TYPE_00://서울 저축은행,한신 저축은행
					pV=&(sVersion[PINPAD_VER_NUMBER_INDEX+1]);
					//impassible difference ?????
					nCurLangType=PINPAD_LANG_TYPE_00;
					break;
				case PINPAD_TYPE_07://신협
				case PINPAD_TYPE_09://한국 증권 금융 
					pV=&(sVersion[PINPAD_VER_NUMBER_INDEX]);
					if( _tcsstr( sVersion,_T("KLP110UR  V1.05") ) ){//구형 신협 미음성지원
						m_nCurType=PINPAD_TYPE_07;
						nCurHWType=PINPAD_HW_TYPE_01;
					}
					else if( _tcsstr( sVersion,_T("KLP110UR  V2.08b") ) ){//구형 신협 음성지원 
						m_nCurType=PINPAD_TYPE_07;
						nCurHWType=PINPAD_HW_TYPE_02;
					}
					else if( _tcsstr( sVersion,_T("KLP110UR  V1.00H") ) ){//한국 증권 금융 
						nCurType=PINPAD_TYPE_09;
						nCurHWType=PINPAD_HW_TYPE_01;
					}
					nCurLangType=PINPAD_LANG_TYPE_00;
					break;
				default:
					pV=&(sVersion[PINPAD_VER_NUMBER_INDEX]);
					nCurLangType=GetLangCodeByChar( pV[4] );
					break;
			}//end switch

			break;
		}
	}//end for

	if( i==REGISTERED_PINPAD_NUMBER )
		return eResult;

	//PINPAD_COM_EQUAL,	// 핀패드의 펨웨어와 버전 스트링 값은 동일 함.
	//PINPAD_COM_OLD,	// 핀패드의 펨웨어가 버전 스트링 값보다 낮은 버전임.
	//PINPAD_COM_NEW,	// 핀패드의 펨웨어가 버전 스트링 값보다 높은 버전임.
	if( nCurType==m_nCurType ){
		if( nCurHWType==m_nCurHWType ){
			if( nCurLangType==m_nCurLangType ){
				//compare major version
				if( pV[0]>m_pV[0] )
					eResult=PINPAD_COM_OLD;
				else if( pV[0]<m_pV[0] )
					eResult=PINPAD_COM_NEW;
				else{//equal major version
					//compare minor version
					if( pV[2]>m_pV[2] )
						eResult=PINPAD_COM_OLD;
					else if( pV[2]<m_pV[2] )
						eResult=PINPAD_COM_NEW;
					else{//equal minor version
						//compare patch veriosn
						if( pV[3]>m_pV[3] )
							eResult=PINPAD_COM_OLD;
						else if( pV[3]<m_pV[3] )
							eResult=PINPAD_COM_NEW;
						else
							eResult=PINPAD_COM_EQUAL;
					}
				}
			}
			else{
				eResult=PINPAD_COM_HW_EQUAL;
			}
		}
		else
			eResult=PINPAD_COM_HW_MIS;
	}
	else{
		if( nCurHWType==m_nCurHWType )
			eResult=PINPAD_COM_HW_EQUAL;
		else
			eResult=PINPAD_COM_HW_MIS;
	}

	//
	return eResult;
}


enum PinPadLangType CEL_Pin_IO_Base::GetLangCodeByChar( TCHAR cLang )
{
	switch(cLang){
		case _T('E'):
			return PINPAD_LANG_TYPE_01;//english type
		default:
			return PINPAD_LANG_TYPE_00;//korean type
	}
}

//get hardware description with given Hardware type code
void CEL_Pin_IO_Base::GetHWTypeDescription( TCHAR *sHWDes,enum PinPadHWType nHWType )
{
	if( sHWDes==NULL )
		return;

	switch(nHWType){
		case PINPAD_HW_TYPE_00://110U		USB	음성무
			_tcscpy( sHWDes,m_ssHWDescriptin[nHWType] );
			break;
		case PINPAD_HW_TYPE_01://110UR		USB+RS232	음성무
			_tcscpy( sHWDes,m_ssHWDescriptin[nHWType] );
			break;
		case PINPAD_HW_TYPE_02:	//110UR2	USB+RS232	음성유
			_tcscpy( sHWDes,m_ssHWDescriptin[nHWType] );
			break;
		case PINPAD_HW_TYPE_03://140U		USB	음성유
			_tcscpy( sHWDes,m_ssHWDescriptin[nHWType] );
			break;
		default:
			_tcscpy( sHWDes,m_ssHWDescriptin[PINPAD_HW_TYPE_UNKNOWN] );
			break;
	}
}

//get pinpad description with given pinpad type code
void CEL_Pin_IO_Base::GetPinpadTypeDescription( TCHAR *sPinpadDes,enum PinPadType nType )
{
	if( sPinpadDes==NULL )
		return;

	switch(nType){
	case PINPAD_TYPE_00://서울 저축은행 타입 
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_01://신한은행
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_02://프라임상호저축은행
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_03://한신저축은행
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_04://대구은행
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_05://이비카드(시외버스터미날)
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_06://굿모닝 신한증권 
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_07://신협 - forth
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_08://에카뱅크(토마토저축은행)
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_09://한국증권금융
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_10://서울증권
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_11://수협
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_12://우리은행 - duplicate
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_13://유화증권
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_14://하나은행
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_15://서울선물
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_16://상호저축은행(FSB) - duplicate
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_17://sample pinpad
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	case PINPAD_TYPE_18://삼성화재 or Basic pinpad
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[nType] );
		break;
	default:
		_tcscpy( sPinpadDes,m_ssPinpadDescriptin[PINPAD_TYPE_UNKNOWN] );
		break;
	}//end switch
}

//get language description with given language type code
void CEL_Pin_IO_Base::GetLangTypeDescription( TCHAR *sLangDes,enum PinPadLangType nLangType )
{
	if( sLangDes==NULL )
		return;

	switch(nLangType){
		case PINPAD_LANG_TYPE_00://korean type
			_tcscpy( sLangDes,m_ssLangDescriptin[nLangType] );
			break;
		case PINPAD_LANG_TYPE_01://english type
			_tcscpy( sLangDes,m_ssLangDescriptin[nLangType] );
			break;
		default:
			_tcscpy( sLangDes,m_ssLangDescriptin[PINPAD_LANG_TYPE_UNKNOWN] );
			break;
	}//end switch
}

///////////////////////////////////////////////////////////////////////////
//simulation support function
#if defined(PINPAD_SIMULATION_ENABLE)

BOOL SimWriteHID( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize )
{
	BOOL bResult=TRUE;
	HRESULT hResult;
	//
	LARGE_INTEGER dlibMove;
	ULARGE_INTEGER dlibNewPosition;
	dlibMove.QuadPart=0;	
	dlibNewPosition.QuadPart=0;
	gpPinSimServer->Seek( dlibMove,4,&dlibNewPosition );//4 is HID mode

	//including report ID (1 byte)
	hResult=gpPinSimServer->Write( &(pData[0]),dwSize,pdwOKSize );
	if( S_OK==hResult )
		bResult=TRUE;
	else{
		bResult=FALSE;

		int i=0;
		switch(hResult){
			case E_PENDING :
				i++;
				break;
			case STG_E_MEDIUMFULL:
				i++;
				break;
			case STG_E_ACCESSDENIED:
				i++;
				break;
			case STG_E_CANTSAVE:
				i++;
				break;
			case STG_E_INVALIDPOINTER:
				i++;
				break;
			case STG_E_REVERTED:
				i++;
				break;
			case STG_E_WRITEFAULT:
				i++;
				break;
			default:
				i++;
				break;
		}//end switch
	}

	return bResult;
}

BOOL SimWrite64( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize )
{
	BOOL bResult=TRUE;
	//
	LARGE_INTEGER dlibMove;
	ULARGE_INTEGER dlibNewPosition;
	dlibMove.QuadPart=0;	
	dlibNewPosition.QuadPart=0;
	gpPinSimServer->Seek( dlibMove,2,&dlibNewPosition );//2 is 64 mode

	if( S_OK==gpPinSimServer->Write( pData,dwSize,pdwOKSize ) )
		bResult=TRUE;
	else
		bResult=FALSE;

	return bResult;
}

BOOL SimWriteV( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize )
{
	BOOL bResult=TRUE;
	//
	LARGE_INTEGER dlibMove;
	ULARGE_INTEGER dlibNewPosition;
	dlibMove.QuadPart=0;	
	dlibNewPosition.QuadPart=0;
	gpPinSimServer->Seek( dlibMove,3,&dlibNewPosition );//3 is V mode

	if( S_OK==gpPinSimServer->Write( pData,dwSize,pdwOKSize ) )
		bResult=TRUE;
	else
		bResult=FALSE;

	return bResult;
}

BOOL SimReadHID( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize )
{
	BOOL bResult=TRUE;
	//
	LARGE_INTEGER dlibMove;
	ULARGE_INTEGER dlibNewPosition;
	dlibMove.QuadPart=0;	
	dlibNewPosition.QuadPart=0;
	gpPinSimServer->Seek( dlibMove,4,&dlibNewPosition );//4 is HID mode

	//including report ID (1 byte)
	if( S_OK==gpPinSimServer->Read( pData,dwSize,pdwOKSize ) )
		bResult=TRUE;
	else
		bResult=FALSE;

	return bResult;
}

BOOL SimRead64( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize )
{
	BOOL bResult=TRUE;
	//
	LARGE_INTEGER dlibMove;
	ULARGE_INTEGER dlibNewPosition;
	dlibMove.QuadPart=0;	
	dlibNewPosition.QuadPart=0;
	gpPinSimServer->Seek( dlibMove,2,&dlibNewPosition );//2 is 64 mode

	if( S_OK==gpPinSimServer->Read( pData,dwSize,pdwOKSize ) )
		bResult=TRUE;
	else
		bResult=FALSE;

	return bResult;
}
BOOL SimReadV( BYTE *pData,DWORD dwSize,DWORD *pdwOKSize )
{
	BOOL bResult=TRUE;
	//
	LARGE_INTEGER dlibMove;
	ULARGE_INTEGER dlibNewPosition;
	dlibMove.QuadPart=0;	
	dlibNewPosition.QuadPart=0;
	gpPinSimServer->Seek( dlibMove,3,&dlibNewPosition );//3 is V mode

	if( S_OK==gpPinSimServer->Read( pData,dwSize,pdwOKSize ) )
		bResult=TRUE;
	else
		bResult=FALSE;
	//
	return bResult;
}

HANDLE SimOpen( TCHAR *sDevPath )
{
	HANDLE hDev=NULL;
	DWORD dwSize=0;
	//
	LARGE_INTEGER dlibMove;
	ULARGE_INTEGER dlibNewPosition;
	dlibMove.QuadPart=0;	
	dlibNewPosition.QuadPart=0;

	//0 is open command mode
	if( S_OK==gpPinSimServer->Seek( dlibMove,0,&dlibNewPosition ) )
		hDev=(HANDLE)CV_PINPAD_HANDLE;
	else
		hDev=INVALID_HANDLE_VALUE;
	//
	return hDev;
}

BOOL SimClose()
{
	LARGE_INTEGER dlibMove;
	ULARGE_INTEGER dlibNewPosition;
	dlibMove.QuadPart=0;	
	dlibNewPosition.QuadPart=0;

	//1 is close command mode
	if( S_OK==gpPinSimServer->Seek( dlibMove,1,&dlibNewPosition ) )
		return TRUE;
	else
		return FALSE;
}
#endif//PINPAD_SIMULATION_ENABLE