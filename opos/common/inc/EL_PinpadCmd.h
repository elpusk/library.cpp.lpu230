// EL_PinpadCmd.h: interface for the CEL_PinpadCmd class.
// this header command of all elpusk pinpad.......
//////////////////////////////////////////////////////////////////////

#if !defined(__EL_PINPAD_CMD_H_20070912__)
#define __EL_PINPAD_CMD_H_20070912__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EL_Pin_TransMgmt.h"
#include "EL_Pin_Cmd_Base.h"
#include "EL_Pin_Cmd.h"//including commnad definition 

class CEL_PinpadCmd: public CEL_Pin_Cmd_Base  
{
public:
	CEL_PinpadCmd();
	virtual ~CEL_PinpadCmd();

	//////////////////////////////////////////////////////////////
	//OPU(operation unit) message type
	//Sync
	DWORD DisplayBitmap( int nIndex,int nDuration );
	//Sync
	DWORD DisplayString( int nIndex,int nDuration,char *sMsg  );
	//Sync
	DWORD SoundVoiceMsg( int nIndex );

	//Sync
	DWORD InputPassword( int *pnPassSize,BYTE *pPass,int nMinLen,int nMaxLen,unsigned int uFlag,char *szDisplay );
	//Message
	DWORD InputPasswordEx( HWND hTargetWnd,UINT nSendMsg,int nMinLen,int nMaxLen,unsigned int uFlag,char *szDisplay );
	//callback
	DWORD InputPasswordExB( CallbackFunType lpFun,LPVOID lpUserPara,int nMinLen,int nMaxLen,unsigned int uFlag,char *szDisplay );
	//Sync
	DWORD StopCommand();

	//////////////////////////////////////////////////////////////
	//System message type
	DWORD GetVersion( int *pnSize,BYTE *pData );

	//Sync
	DWORD WriteBitmap( int nIndex,int nInSize,BYTE *pInData );
	DWORD WriteBitmap( int nIndex,char *sFileName );
	//Message
	DWORD WriteBitmapEx( HWND hTargetWnd,UINT nSendMsg,int nIndex,int nInSize,BYTE *pInData );
	DWORD WriteBitmapEx( HWND hTargetWnd,UINT nSendMsg,int nIndex,char *sFileName );
	//callback
	DWORD WriteBitmapExB( CallbackFunType lpFun,LPVOID lpUserPara,int nIndex,int nInSize,BYTE *pInData );
	DWORD WriteBitmapExB( CallbackFunType lpFun,LPVOID lpUserPara,int nIndex,char *sFileName );

	//Sync
	DWORD WriteVoice( int nIndex,int nInSize,BYTE *pInData );
	DWORD WriteVoice( int nIndex,int nSubIndex,int nInSize,BYTE *pInData );
	DWORD WriteVoice( int nIndex,char *sFileName );
	//Message
	DWORD WriteVoiceEx( HWND hTargetWnd,UINT nSendMsg,int nIndex,int nInSize,BYTE *pInData );
	DWORD WriteVoiceEx( HWND hTargetWnd,UINT nSendMsg,int nIndex,char *sFileName );
	//callback
	DWORD WriteVoiceExB( CallbackFunType lpFun,LPVOID lpUserPara,int nIndex,int nInSize,BYTE *pInData );
	DWORD WriteVoiceExB( CallbackFunType lpFun,LPVOID lpUserPara,int nIndex,char *sFileName );

	//Sync
	DWORD WriteCode( int nSubIndex,int nCodeType,int nInSize,BYTE *pInData );
	DWORD WriteCode( int nSubIndex,int nCodeType,char *sFileName );
	//Message
	DWORD WriteCodeEx( HWND hTargetWnd,UINT nSendMsg,int nSubIndex,int nCodeType,int nInSize,BYTE *pInData );
	DWORD WriteCodeEx( HWND hTargetWnd,UINT nSendMsg,int nSubIndex,int nCodeType,char *sFileName );
	//callback
	DWORD WriteCodeExB( CallbackFunType lpFun,LPVOID lpUserPara,int nSubIndex,int nCodeType,int nInSize,BYTE *pInData );
	DWORD WriteCodeExB( CallbackFunType lpFun,LPVOID lpUserPara,int nSubIndex,int nCodeType,char *sFileName );

	//Sync
	DWORD ChangeCode();
	//Message
	DWORD ChangeCodeEx( HWND hTargetWnd,UINT nSendMsg );
	//callback
	DWORD ChangeCodeExB( CallbackFunType lpFun,LPVOID lpUserPara );

	//////////////////////////////////////////////////////////////
	//ICC message type

	//Sync
	DWORD PowerOnIcc( int *pnAtr,BYTE *pATR,int nSlot,unsigned int uPowerFlag );

	//Sync
	DWORD PowerOffIcc( int nSlot );

	//Sync
	DWORD TransmitIccAPDU( int *pnRX,BYTE *pRX,int nTX,BYTE *pTX,int nSlot ); 

	//get (including header)response of Async commad
	DWORD GetPureResponse( BYTE *lpdata,DWORD *pdwSize )
	{	return GetResult(  lpdata,pdwSize );	};

	//get (except header)response of Async commad
	DWORD GetResponse( BYTE *lpdata,DWORD *pdwSize )
	{	return GetDataFieldOfResult( (int*)pdwSize,lpdata );	};

protected:
	virtual DWORD MappingErrorCode( BYTE cError );

	DWORD GetDataFieldOfResult( int *pnOutSize=NULL,BYTE *pOutData=NULL );

};

#endif // !defined(__EL_PINPAD_CMD_H_20070912__)
