// EL_PinpadCmd.cpp: implementation of the CEL_PinpadCmd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EL_PinpadCmd.h"
#include "EL_Pin_ReCode.h" 

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEL_PinpadCmd::CEL_PinpadCmd()
{
}

CEL_PinpadCmd::~CEL_PinpadCmd()
{
}

//////////////////////////////////////////////////////////////////////
//OPU(operation unit) message type
DWORD CEL_PinpadCmd::DisplayBitmap( int nIndex,int nDuration )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//
	if( nIndex<EL_PIN_MIN_BMP_INDEX || nIndex>EL_PIN_MAX_BMP_INDEX )
		return KLP110UF_E_INVAILD_PARA;
	if( nDuration<0 || nDuration>EL_PIN_MAX_BMP_ON_TIME )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_DIS_BMP;
	Head.cSubType1=(BYTE)nIndex;
	Head.cSubType2=(BYTE)nDuration;

	//
	nSize=sizeof(EL_PACKET_H64);

	dwResult=SyncFun( (BYTE*)&Head,nSize,PIN_TRANS_TYPE_S_OUT );

	return dwResult;
}

DWORD CEL_PinpadCmd::DisplayString( int nIndex,int nDuration,char *sMsg )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	int nStr=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( nIndex<EL_PIN_MIN_STR_INDEX || nIndex>EL_PIN_MAX_STR_INDEX )
		return KLP110UF_E_INVAILD_PARA;
	if( nDuration<0 || nDuration>EL_PIN_MAX_STR_ON_TIME )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex==EL_PIN_MIN_STR_INDEX && sMsg==NULL )
		return KLP110UF_E_INVAILD_PARA;

	if( sMsg )
		nStr=strlen(sMsg);//except NULL of string
	else
		nStr=0;
	//
	nSize=sizeof(EL_PACKET_H64)+nStr;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_DIS_STRING;
	Head.cSubType1=(BYTE)nIndex;
	Head.cSubType2=(BYTE)nDuration;
	Head.ulLength=(ULONG)nStr;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );

	if( nStr>0 )
		memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),sMsg,nStr );


	dwResult=SyncFun( (BYTE*)pBuf,nSize,PIN_TRANS_TYPE_S_OUT );
	free(pBuf);

	return dwResult;
}

DWORD CEL_PinpadCmd::SoundVoiceMsg( int nIndex )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//
	if( nIndex<EL_PIN_MIN_VOICE_INDEX || nIndex>EL_PIN_MAX_VOICE_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_VOC_MSG;
	Head.cSubType1=(BYTE)nIndex;

	//
	nSize=sizeof(EL_PACKET_H64);

	dwResult=SyncFun( (BYTE*)&Head,nSize,PIN_TRANS_TYPE_S_OUT );

	return dwResult;
}

// pPass[out] - input password.
// pnPassSize[out] - input password size.
DWORD CEL_PinpadCmd::InputPassword( int *pnPassSize,BYTE *pPass,int nMinLen,int nMaxLen,unsigned int uFlag,char *szDisplay )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	int nStr=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;
	//
	if( pPass==NULL || pnPassSize==NULL )
		return KLP110UF_E_INVAILD_PARA;
	if( nMinLen<EL_PIN_MIN_PW_LEN || nMaxLen>EL_PIN_MAX_PW_LEN )
		return KLP110UF_E_INVAILD_PARA;

	if( szDisplay )
		nStr=strlen(szDisplay);//except NULL of string
	else
		nStr=0;
	//
	nSize=sizeof(EL_PACKET_H64)+nStr;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_PW_INPUT;
	Head.cSubType1=(BYTE)nMinLen;
	Head.cSubType2=(BYTE)nMaxLen;
	Head.cSubType3=(BYTE)uFlag;
	Head.ulLength=(ULONG)nStr;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );

	if( nStr>0 )
		memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),szDisplay,nStr );


	dwResult=SyncFun( (BYTE*)pBuf,nSize,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);

	if( dwResult==KLP110UF_S_SUCCESS ){
		*pnPassSize=0;

		//get result from header of response &  data field.
		dwResult=GetDataFieldOfResult( pnPassSize,pPass );
	}

	return dwResult;
}

DWORD CEL_PinpadCmd::StopCommand()
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_STOP_CMD;

	//
	nSize=sizeof(EL_PACKET_H64);

	dwResult=SyncFun( (BYTE*)&Head,nSize,PIN_TRANS_TYPE_S_OUT );

	return dwResult;
}


//////////////////////////////////////////////////////////////////////
//System message type
DWORD CEL_PinpadCmd::GetVersion( int *pnSize,BYTE *pData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//
	if( pnSize==NULL || pData==NULL )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_GET_VER;
	//
	nSize=sizeof(EL_PACKET_H64);

	dwResult=SyncFun( (BYTE*)&Head,nSize,PIN_TRANS_TYPE_S_OUT_S_IN );
	if( dwResult==KLP110UF_S_SUCCESS ){
		*pnSize=0;
		dwResult=GetResult( pData,(DWORD*)pnSize );
	}

	return dwResult;
}

DWORD CEL_PinpadCmd::WriteBitmap( int nIndex,int nInSize,BYTE *pInData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pInData==NULL || nInSize<0 )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_BMP_INDEX || nIndex>EL_PIN_MAX_BMP_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nInSize;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_BMP;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=(ULONG)nInSize;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pInData,nInSize );

	dwResult=SyncFun( (BYTE*)pBuf,nSize,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);

	if( dwResult==KLP110UF_S_SUCCESS ){
		nSize=0;

		//get result from header of response.
		dwResult=GetDataFieldOfResult();
	}
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteBitmap( int nIndex,char *sFileName )
{
	EL_PACKET_H64 Head;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	if( sFileName==NULL )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_BMP_INDEX || nIndex>EL_PIN_MAX_BMP_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_BMP;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=0;//data field lenght is set to file-size automatically

	dwResult=SyncFun( &Head,sFileName,PIN_TRANS_TYPE_S_OUT_S_IN );

	if( dwResult==KLP110UF_S_SUCCESS ){

		//get result from header of response.
		dwResult=GetDataFieldOfResult();
	}
	
	return dwResult;
}


DWORD CEL_PinpadCmd::WriteVoice( int nIndex,int nInSize,BYTE *pInData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pInData==NULL || nInSize<0 )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_VOICE_INDEX || nIndex>EL_PIN_MAX_VOICE_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nInSize;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_VOC;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=(ULONG)nInSize;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pInData,nInSize );

	dwResult=SyncFun( (BYTE*)pBuf,nSize,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);

	if( dwResult==KLP110UF_S_SUCCESS ){
		nSize=0;

		//get result from header of response.
		dwResult=GetDataFieldOfResult();
	}
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteVoice( int nIndex,int nSubIndex,int nInSize,BYTE *pInData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pInData==NULL || nInSize<0 )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_VOICE_INDEX || nIndex>EL_PIN_MAX_VOICE_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nInSize;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_VOC;
	Head.cSubType1=(BYTE)nIndex;
	Head.cSubType2=(BYTE)nSubIndex;
	Head.ulLength=(ULONG)nInSize;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pInData,nInSize );

	dwResult=SyncFun( (BYTE*)pBuf,nSize,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);

	if( dwResult==KLP110UF_S_SUCCESS ){
		nSize=0;

		//get result from header of response.
		dwResult=GetDataFieldOfResult();
	}
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteVoice( int nIndex,char *sFileName )
{
	EL_PACKET_H64 Head;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	if( sFileName==NULL )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_VOICE_INDEX || nIndex>EL_PIN_MAX_VOICE_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_VOC;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=0;//data field lenght is set to file-size automatically

	dwResult=SyncFun( &Head,sFileName,PIN_TRANS_TYPE_S_OUT_S_IN );

	if( dwResult==KLP110UF_S_SUCCESS ){

		//get result from header of response.
		dwResult=GetDataFieldOfResult();
	}
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteCode( int nSubIndex,int nCodeType,int nInSize,BYTE *pInData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pInData==NULL || nInSize<0 )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nInSize;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_CODE;
	Head.cSubType1=(BYTE)nCodeType;
	Head.cSubType1=(BYTE)nSubIndex;
	Head.ulLength=(ULONG)nInSize;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pInData,nInSize );

	dwResult=SyncFun( (BYTE*)pBuf,nSize,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);

	if( dwResult==KLP110UF_S_SUCCESS ){
		nSize=0;

		//get result from header of response.
		dwResult=GetDataFieldOfResult();
	}
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteCode( int nSubIndex,int nCodeType,char *sFileName )
{
	EL_PACKET_H64 Head;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	if( sFileName==NULL )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_CODE;
	Head.cSubType1=(BYTE)nCodeType;
	Head.cSubType2=(BYTE)nSubIndex;
	Head.ulLength=0;//data field lenght is set to file-size automatically

	dwResult=SyncFun( &Head,sFileName,PIN_TRANS_TYPE_S_OUT_S_IN );

	if( dwResult==KLP110UF_S_SUCCESS ){

		//get result from header of response.
		dwResult=GetDataFieldOfResult();
	}
	
	return dwResult;
}

DWORD CEL_PinpadCmd::ChangeCode()
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	nSize=sizeof(EL_PACKET_H64);

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_CHR_CODE;

	dwResult=SyncFun( (BYTE*)&Head,nSize,PIN_TRANS_TYPE_S_OUT_S_IN );

	if( dwResult==KLP110UF_S_SUCCESS ){
		nSize=0;

		//get result from header of response.
		dwResult=GetDataFieldOfResult();
	}
	
	return dwResult;
}


//////////////////////////////////////////////////////////////////////
//ICC message type
DWORD CEL_PinpadCmd::PowerOnIcc( int *pnAtr,BYTE *pATR,int nSlot,unsigned int uPowerFlag )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	nSize=sizeof(EL_PACKET_H64);

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_ICC_ON;
	Head.cSubType1=(BYTE)uPowerFlag;
	Head.cSlot=(BYTE)nSlot;

	dwResult=SyncFun( (BYTE*)&Head,nSize,PIN_TRANS_TYPE_S_OUT_S_IN );

	if( dwResult==KLP110UF_S_SUCCESS ){
		nSize=0;

		//get result from header of response.
		dwResult=GetDataFieldOfResult( pnAtr,pATR );
		if( dwResult==KLP110UF_S_SUCCESS ){
			if( *pnAtr<=0 )
				dwResult=KLP110UF_E_NO_ATR;
		}
	}
	
	return dwResult;
}

DWORD CEL_PinpadCmd::PowerOffIcc( int nSlot )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_ICC_OFF;
	Head.cSlot=(BYTE)nSlot;

	//
	nSize=sizeof(EL_PACKET_H64);

	dwResult=SyncFun( (BYTE*)&Head,nSize,PIN_TRANS_TYPE_S_OUT );

	return dwResult;
}

DWORD CEL_PinpadCmd::TransmitIccAPDU( int *pnRX,BYTE *pRX,int nTX,BYTE *pTX,int nSlot )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pTX==NULL || nTX<0 )
		return KLP110UF_E_INVAILD_PARA;
	if( pnRX==NULL || pRX==NULL )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nTX;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_ICC_APDU;
	Head.cSlot=(BYTE)nSlot;
	Head.ulLength=(ULONG)nTX;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pTX,nTX );

	dwResult=SyncFun( (BYTE*)pBuf,nSize,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);

	if( dwResult==KLP110UF_S_SUCCESS ){
		nSize=0;

		//get result from header of response.
		dwResult=GetDataFieldOfResult( pnRX,pRX );
	}
	
	return dwResult;
}

DWORD CEL_PinpadCmd::MappingErrorCode( BYTE cError )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;

	switch(cError){
		case 0:
			dwResult=KLP110UF_S_SUCCESS;
			break;
		case 1:
			dwResult=KLP110UF_E_UNKNOWN;
			break;
		default:
			dwResult=KLP110UF_E_INVAILD_RESP;
			break;
	}//end switch

	return dwResult;
}

DWORD CEL_PinpadCmd::GetDataFieldOfResult( int *pnOutSize/*=NULL*/,BYTE *pOutData/*=NULL*/ )
{
	PEL_PACKET_H64 pReHead=NULL;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	int nSize=0;
	BYTE *pBuf=NULL;

	if( pnOutSize==NULL ){
		if( pOutData==NULL ){
			//get response size
			dwResult=GetResult( NULL,(DWORD*)&nSize );
			if( dwResult==KLP110UF_S_SUCCESS ){

				if( nSize>=(sizeof(EL_PACKET_H64)) ){
					pBuf=(BYTE*)malloc( nSize );
					if( pBuf==NULL )
						return KLP110UF_E_UNKNOWN;
					//
					memset( pBuf,0x00,nSize );

					//get response
					dwResult=GetResult( pBuf,(DWORD*)&nSize );
					if( dwResult==KLP110UF_S_SUCCESS ){
						//analysis of response
						pReHead=(PEL_PACKET_H64)pBuf;

						//pReHead->cSubType2 : error code
						dwResult=MappingErrorCode(pReHead->cSubType2);
					}

					free( pBuf );
				}//nSize>sizeof(EL_PACKET_H64)
				else
					dwResult=KLP110UF_E_INVAILD_RESP;
			}
		}
		else
			dwResult=KLP110UF_E_INVAILD_PARA;
	}
	else{//pnOutSize !=NULL
		if( pOutData==NULL )
			dwResult=GetResult( NULL,(DWORD*)pnOutSize );
		else{//pnOutSize !=NULL && pOutData!=NULL
			//get response size
			dwResult=GetResult( NULL,(DWORD*)&nSize );
			if( dwResult==KLP110UF_S_SUCCESS ){

				if( nSize>=(sizeof(EL_PACKET_H64)) ){
					pBuf=(BYTE*)malloc( nSize );
					if( pBuf==NULL )
						return KLP110UF_E_UNKNOWN;
					//
					memset( pBuf,0x00,nSize );

					//get response
					dwResult=GetResult( pBuf,(DWORD*)&nSize );
					if( dwResult==KLP110UF_S_SUCCESS ){
						//analysis of response
						pReHead=(PEL_PACKET_H64)pBuf;

						//pReHead->cSubType2 : error code
						dwResult=MappingErrorCode(pReHead->cSubType2);

						//copy data field of response
						*pnOutSize=(int)pReHead->ulLength;
						memcpy( pOutData,&(pBuf[sizeof(EL_PACKET_H64)]),*pnOutSize );
					}

					free( pBuf );
				}//nSize>sizeof(EL_PACKET_H64)
				else
					dwResult=KLP110UF_E_INVAILD_RESP;
			}
		}
	}

	return dwResult;
}

DWORD CEL_PinpadCmd::InputPasswordEx( HWND hTargetWnd,UINT nSendMsg,int nMinLen,int nMaxLen,unsigned int uFlag,char *szDisplay )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	int nStr=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;
	
	if( nMinLen<EL_PIN_MIN_PW_LEN || nMaxLen>EL_PIN_MAX_PW_LEN )
		return KLP110UF_E_INVAILD_PARA;

	if( szDisplay )
		nStr=strlen(szDisplay);//except NULL of string
	else
		nStr=0;
	//
	nSize=sizeof(EL_PACKET_H64)+nStr;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_PW_INPUT;
	Head.cSubType1=(BYTE)nMinLen;
	Head.cSubType2=(BYTE)nMaxLen;
	Head.cSubType3=(BYTE)uFlag;
	Head.ulLength=(ULONG)nStr;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );

	if( nStr>0 )
		memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),szDisplay,nStr );

	dwResult=AsyncFun_Msg( (BYTE*)pBuf,nSize,hTargetWnd,nSendMsg,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);
	
	return dwResult;
}

DWORD CEL_PinpadCmd::InputPasswordExB( CallbackFunType lpFun,LPVOID lpUserPara,int nMinLen,int nMaxLen,unsigned int uFlag,char *szDisplay )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	int nStr=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;
	
	if( nMinLen<EL_PIN_MIN_PW_LEN || nMaxLen>EL_PIN_MAX_PW_LEN )
		return KLP110UF_E_INVAILD_PARA;

	if( szDisplay )
		nStr=strlen(szDisplay);//except NULL of string
	else
		nStr=0;
	//
	nSize=sizeof(EL_PACKET_H64)+nStr;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_PW_INPUT;
	Head.cSubType1=(BYTE)nMinLen;
	Head.cSubType2=(BYTE)nMaxLen;
	Head.cSubType3=(BYTE)uFlag;
	Head.ulLength=(ULONG)nStr;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );

	if( nStr>0 )
		memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),szDisplay,nStr );

	dwResult=AsyncFun_CB( (BYTE*)pBuf,nSize,lpFun,lpUserPara,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);
	
	return dwResult;
}

//Message
DWORD CEL_PinpadCmd::WriteBitmapEx( HWND hTargetWnd,UINT nSendMsg,int nIndex,int nInSize,BYTE *pInData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pInData==NULL || nInSize<0 )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_BMP_INDEX || nIndex>EL_PIN_MAX_BMP_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nInSize;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_BMP;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=(ULONG)nInSize;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pInData,nInSize );

	dwResult=AsyncFun_Msg( (BYTE*)pBuf,nSize,hTargetWnd,nSendMsg,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteBitmapEx( HWND hTargetWnd,UINT nSendMsg,int nIndex,char *sFileName )
{
	EL_PACKET_H64 Head;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	if( sFileName==NULL )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_BMP_INDEX || nIndex>EL_PIN_MAX_BMP_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_BMP;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=0;//automatic setting by lower class

	dwResult=AsyncFun_Msg( &Head,sFileName,hTargetWnd,nSendMsg,PIN_TRANS_TYPE_S_OUT_S_IN );
	
	return dwResult;
}

//callback
DWORD CEL_PinpadCmd::WriteBitmapExB( CallbackFunType lpFun,LPVOID lpUserPara,int nIndex,int nInSize,BYTE *pInData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pInData==NULL || nInSize<0 )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_BMP_INDEX || nIndex>EL_PIN_MAX_BMP_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nInSize;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_BMP;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=(ULONG)nInSize;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pInData,nInSize );

	dwResult=AsyncFun_CB( (BYTE*)pBuf,nSize,lpFun,lpUserPara,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteBitmapExB( CallbackFunType lpFun,LPVOID lpUserPara,int nIndex,char *sFileName )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	if( sFileName==NULL )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_BMP_INDEX || nIndex>EL_PIN_MAX_BMP_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64);

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_BMP;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=0;//automatic setting by lower class

	dwResult=AsyncFun_CB( &Head,sFileName,lpFun,lpUserPara,PIN_TRANS_TYPE_S_OUT_S_IN );
	
	return dwResult;
}

//Message
DWORD CEL_PinpadCmd::WriteVoiceEx( HWND hTargetWnd,UINT nSendMsg,int nIndex,int nInSize,BYTE *pInData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pInData==NULL || nInSize<0 )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_VOICE_INDEX || nIndex>EL_PIN_MAX_VOICE_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nInSize;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_VOC;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=(ULONG)nInSize;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pInData,nInSize );

	dwResult=AsyncFun_Msg( (BYTE*)pBuf,nSize,hTargetWnd,nSendMsg,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteVoiceEx( HWND hTargetWnd,UINT nSendMsg,int nIndex,char *sFileName )
{
	EL_PACKET_H64 Head;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//
	if( sFileName==NULL )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_VOICE_INDEX || nIndex>EL_PIN_MAX_VOICE_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_VOC;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=0;//data field lenght is set to file-size automatically

	dwResult=AsyncFun_Msg( &Head,sFileName,hTargetWnd,nSendMsg,PIN_TRANS_TYPE_S_OUT_S_IN );
	
	return dwResult;
}

//callback
DWORD CEL_PinpadCmd::WriteVoiceExB( CallbackFunType lpFun,LPVOID lpUserPara,int nIndex,int nInSize,BYTE *pInData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pInData==NULL || nInSize<0 )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_VOICE_INDEX || nIndex>EL_PIN_MAX_VOICE_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nInSize;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_VOC;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=(ULONG)nInSize;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pInData,nInSize );

	dwResult=AsyncFun_CB( (BYTE*)pBuf,nSize,lpFun,lpUserPara,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteVoiceExB( CallbackFunType lpFun,LPVOID lpUserPara,int nIndex,char *sFileName )
{
	EL_PACKET_H64 Head;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//
	if( sFileName==NULL )
		return KLP110UF_E_INVAILD_PARA;
	if( nIndex<EL_PIN_MIN_VOICE_INDEX || nIndex>EL_PIN_MAX_VOICE_INDEX )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_VOC;
	Head.cSubType1=(BYTE)nIndex;
	Head.ulLength=0;//data field lenght is set to file-size automatically

	dwResult=AsyncFun_CB( &Head,sFileName,lpFun,lpUserPara,PIN_TRANS_TYPE_S_OUT_S_IN );
	
	return dwResult;
}

	//Message
DWORD CEL_PinpadCmd::WriteCodeEx( HWND hTargetWnd,UINT nSendMsg,int nSubIndex,int nCodeType,int nInSize,BYTE *pInData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pInData==NULL || nInSize<0 )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nInSize;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_CODE;
	Head.cSubType1=(BYTE)nCodeType;
	Head.cSubType1=(BYTE)nSubIndex;
	Head.ulLength=(ULONG)nInSize;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pInData,nInSize );

	dwResult=AsyncFun_Msg( (BYTE*)pBuf,nSize,hTargetWnd,nSendMsg,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteCodeEx( HWND hTargetWnd,UINT nSendMsg,int nSubIndex,int nCodeType,char *sFileName )
{
	EL_PACKET_H64 Head;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	if( sFileName==NULL )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_CODE;
	Head.cSubType1=(BYTE)nCodeType;
	Head.cSubType2=(BYTE)nSubIndex;
	Head.ulLength=0;//data field lenght is set to file-size automatically

	dwResult=AsyncFun_Msg( &Head,sFileName,hTargetWnd,nSendMsg,PIN_TRANS_TYPE_S_OUT_S_IN );
	
	return dwResult;
}

//callback
DWORD CEL_PinpadCmd::WriteCodeExB( CallbackFunType lpFun,LPVOID lpUserPara,int nSubIndex,int nCodeType,int nInSize,BYTE *pInData )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE *pBuf=NULL;

	//
	if( pInData==NULL || nInSize<0 )
		return KLP110UF_E_INVAILD_PARA;

	nSize=sizeof(EL_PACKET_H64)+nInSize;
	pBuf=(BYTE*)malloc( nSize );
	if( pBuf==NULL )
		return KLP110UF_E_UNKNOWN;
	//
	memset( pBuf,0x00,nSize );

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_CODE;
	Head.cSubType1=(BYTE)nCodeType;
	Head.cSubType1=(BYTE)nSubIndex;
	Head.ulLength=(ULONG)nInSize;

	//Build TX data
	memcpy( pBuf,&Head,sizeof(EL_PACKET_H64) );
	memcpy( &(pBuf[sizeof(EL_PACKET_H64)]),pInData,nInSize );

	dwResult=AsyncFun_CB( (BYTE*)pBuf,nSize,lpFun,lpUserPara,PIN_TRANS_TYPE_S_OUT_S_IN );
	free(pBuf);
	
	return dwResult;
}

DWORD CEL_PinpadCmd::WriteCodeExB( CallbackFunType lpFun,LPVOID lpUserPara,int nSubIndex,int nCodeType,char *sFileName )
{
	EL_PACKET_H64 Head;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	if( sFileName==NULL )
		return KLP110UF_E_INVAILD_PARA;

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_WR_CODE;
	Head.cSubType1=(BYTE)nCodeType;
	Head.cSubType2=(BYTE)nSubIndex;
	Head.ulLength=0;//data field lenght is set to file-size automatically

	dwResult=AsyncFun_CB( &Head,sFileName,lpFun,lpUserPara,PIN_TRANS_TYPE_S_OUT_S_IN );
	
	return dwResult;
}

//Message
DWORD CEL_PinpadCmd::ChangeCodeEx( HWND hTargetWnd,UINT nSendMsg )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	nSize=sizeof(EL_PACKET_H64);

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_CHR_CODE;

	dwResult=AsyncFun_Msg( (BYTE*)&Head,nSize,hTargetWnd,nSendMsg,PIN_TRANS_TYPE_S_OUT_S_IN );
	
	return dwResult;
}

//callback
DWORD CEL_PinpadCmd::ChangeCodeExB( CallbackFunType lpFun,LPVOID lpUserPara )
{
	EL_PACKET_H64 Head;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;

	//
	nSize=sizeof(EL_PACKET_H64);

	//Build Header
	memset( &Head,0x00,sizeof(EL_PACKET_H64) );
	Head.cType=EL_PIN_CMD_TYPE_CHR_CODE;

	dwResult=AsyncFun_CB( (BYTE*)&Head,nSize,lpFun,lpUserPara,PIN_TRANS_TYPE_S_OUT_S_IN );
	
	return dwResult;
}
