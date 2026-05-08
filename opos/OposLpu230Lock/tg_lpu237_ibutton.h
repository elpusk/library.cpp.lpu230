#pragma once

/*?
 * 2014.11.20 - the first coding
 * 2017.9.12 - update
 */

/*
#ifdef __cplusplus
extern "C" {
#endif 


#ifndef TG_LPU237_IBUTTON_EXPORTS
#define TG_LPU237_IBUTTON_EXPORTS __declspec(dllimport)
#endif /// EXPORT_API

#include <Windows.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// the definition of callback function

typedef void (CALLBACK *CallBackKeyIn)(BYTE * pBuffer, BYTE length);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// exported Function prototype/

TG_LPU237_IBUTTON_EXPORTS BOOL WINAPI XHidDev_LoadDriver(CallBackKeyIn cbOnKeyIn);
TG_LPU237_IBUTTON_EXPORTS void WINAPI XHidDev_UnloadDriver(void);
TG_LPU237_IBUTTON_EXPORTS INT WINAPI UCD_SendData(BYTE * pBuf, DWORD dwBufLen);

#ifdef __cplusplus
}
#endif /// __cplusplus

*/

#include <Windows.h>

/*!
*	return value definition.
*/
#define	LPU237LOCK_DLL_RESULT_SUCCESS			0		//! processing success.
#define	LPU237LOCK_DLL_RESULT_ERROR			0xFFFFFFFF	//! processing error.( maybe system or communication error ); (-1)
#define	LPU237LOCK_DLL_RESULT_CANCEL			0xFFFFFFFE	//! processing is canceled by another reqest.(-2)

#ifdef __cplusplus
extern "C" {
#endif 


#ifndef TG_LPU237_IBUTTON_EXPORTS
#define TG_LPU237_IBUTTON_EXPORTS __declspec(dllimport)
#endif /// EXPORT_API

typedef void (CALLBACK *CallBackKeyIn)(BYTE * pBuffer, BYTE length);

/*!
*	the callback function type.
*	this type will be used in LPU237Lock_wait_key_with_callback()
*/
typedef	void (WINAPI *type_key_callback)(void*);


TG_LPU237_IBUTTON_EXPORTS BOOL WINAPI XHidDev_LoadDriver(CallBackKeyIn cbOnKeyIn);
TG_LPU237_IBUTTON_EXPORTS void WINAPI XHidDev_UnloadDriver(void);
TG_LPU237_IBUTTON_EXPORTS INT WINAPI UCD_SendData(BYTE * pBuf, DWORD dwBufLen);

#ifdef __cplusplus
}
#endif /// __cplusplus

/*!
* function
*	Create lpu237 work thread.
*
* parameters
*
* return
*  	LPU237LOCK_DLL_RESULT_SUCCESS : only.
*
*/
DWORD WINAPI LPU237Lock_dll_on();

/*!
* function
*	Terminate lpu237 work thread.
*
* parameters
*
* return
*  	LPU237LOCK_DLL_RESULT_SUCCESS : only.
*
*/
DWORD WINAPI LPU237Lock_dll_off();

/*!
* function
*	find a connected lpu237 devices.
*
* parameters
*	ssDevPaths : [in/out] Multi string of devices paths
*
* return
*	if ssDevPaths = NULL, return requested memory size [unit byte]
*	else the number of connected lpu237 device.
*/
DWORD WINAPI LPU237Lock_get_list( LPTSTR ssDevPaths	);

/*!
* function
*	equal to LPU237_get_list.
*  LPU237Lock_get_list' unicode version
*
*	return
*		if ssDevPaths = NULL, the number of character.(including NULL). one character size = 2 bytes
*		else the number of connected lpu237 device.
*/
DWORD WINAPI LPU237Lock_get_list_w( WCHAR *ssDevPaths	);

/*!
* function
*	equal to LPU237Lock_get_list.
*  LPU237Lock_get_list' Multi Byte Code Set version
*
*	return
*		if ssDevPaths = NULL, the number of character.(including NULL). one character size = 1 bytes
*		else the number of connected lpu237 device.
*/
DWORD WINAPI LPU237Lock_get_list_a( CHAR *ssDevPaths	);

/*!
* function 
*	open lpu237 device.
*
* parameters
*	sDevPath : [in] device path
*
* return 
*	if success, return device handle.
*	else return INVALID_HANDLE_VALUE
*/
HANDLE WINAPI LPU237Lock_open( LPCTSTR sDevPath );

/*!
* function 
*	LPU237Lock_open' unicode verision.
*
* parameters
*	sDevPath : [in] device path
*
* return 
*	if success, return device handle.
*	else return INVALID_HANDLE_VALUE
*/
HANDLE WINAPI LPU237Lock_open_w( CONST WCHAR *sDevPath );

/*!
* function 
*	LPU237Lock_open' MBCS version.
*
* parameters
*	sDevPath : [in] device path
*
* return 
*	if success, return device handle.
*	else return INVALID_HANDLE_VALUE
*/
HANDLE WINAPI LPU237Lock_open_a( CONST CHAR *sDevPath );

/*!
* function
*	close lpu237 device.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_open() )
*
* return
*	if success, return LPU237LOCK_DLL_RESULT_SUCCESS
*	else return LPU237LOCK_DLL_RESULT_ERROR
*/
DWORD WINAPI LPU237Lock_close( HANDLE hDev );

/*!
* function
*	change to lpu237 reader to ready for reading a magnetic card.
*
* parameters
*	hDev : [in] device handle( return value of LPU237Lock_open() )
*
* return
*	if success, return LPU237LOCK_DLL_RESULT_SUCCESS
*	else return LPU237LOCK_DLL_RESULT_ERROR
*
*/
DWORD WINAPI LPU237Lock_enable( HANDLE hDev );

/*!
* function
*	change to lpu237 reader to ignore reading a magnetic card.
*
* parameters
*	hDev : [in] device handle( return value of LPU237Lock_open() )
*
* return
*	if success, return LPU237LOCK_DLL_RESULT_SUCCESS
*	else return LPU237LOCK_DLL_RESULT_ERROR
*
*/
DWORD WINAPI LPU237Lock_disable( HANDLE hDev );

/*!
* function
*	stop operation of LPU237Lock_wait_key_with_callback()
*
* parameters
*	hDev : [in] device handle( return value of LPU237Lock_open() )
*
* return
*	if success, return LPU237LOCK_DLL_RESULT_SUCCESS
*	else return LPU237LOCK_DLL_RESULT_ERROR
*
*/
DWORD WINAPI LPU237Lock_cancel_wait_key( HANDLE hDev );

/*!
* function
*	wait to contact i-button on lpu237.
*	this function return immediately.
*	and when user contact  a i-button on lpu237, execute the callback funtion(pFun) with parameter(pParameter) .
*
* parameters
*	hDev : [in] device handle( return value of LPU237Lock_open() )
*	pFun : [in] callback funtion' poniter.
*	pParameter : [in] callback funtion' parameter( pFun' parameter )
*
* return 
* 	if error, return LPU237LOCK_DLL_RESULT_ERROR
*	else buffer index number of i-button. 
*	( this buffer index number is parameter of LPU237Lock_get_data() )
*/
DWORD WINAPI LPU237Lock_wait_key_with_callback( HANDLE hDev, type_key_callback pFun, void *pParameter );


/*!
* function
*	getting the i-button data.
*
* parameters
*	dwBufferIndex : [in] buffer index number of i-button data. 
*							return value of LPU237Lock_wait_key_with_callback().
*
* return
*  	LPU237LOCK_DLL_RESULT_ERROR : error in a magnetic card reading operation.
*					may be error between your PC and lpu237 reader.
*	LPU237LOCK_DLL_RESULT_CANCEL : A magnetic card reading operation is canceled by 
*					LPU237Lock_cancel_wait_swipe(), LPU237_wait_key_with_callback() .
*	the number of i-button data : succession of a i-button reading operation.
*
*/
DWORD WINAPI LPU237Lock_get_data( DWORD dwBufferIndex, BYTE *sKey );

/*!
* function
*	get device unique ID.
*
* parameters
*	hDev : [in] device handle( return value of LPU237Lock_open() )
*	sId : [in/out] A pointer to the buffer that save the device ID.
*			this value can be NULL(0).
*
* return 
* 	if error, return LPU237LOCK_DLL_RESULT_ERROR
*	else the size of ID.[unit byte]
*/
DWORD WINAPI LPU237Lock_get_id( HANDLE hDev, BYTE *sId );
