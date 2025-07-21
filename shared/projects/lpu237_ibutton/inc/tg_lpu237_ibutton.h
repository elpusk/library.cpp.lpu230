#pragma once

/**
* 2025.03.18  - release version 6.0 : wss client
*/


#ifdef _WIN32
#include <Windows.h>
#define	_CALLTYPE_	__stdcall
#else 
//linux
#define _PACK_BYTE  __attribute__((packed))
#define	_CALLTYPE_	__attribute__((__stdcall__))

#define PtrToUlong(_a)      ((unsigned long)(_a))
#define INVALID_HANDLE_VALUE    ((unsigned long)-1)
#define FALSE   0
#define TRUE    1

typedef unsigned long HWND;
typedef unsigned long UINT;
typedef unsigned long HANDLE;
typedef unsigned long BOOL;
#endif

/*!
*	return value definition.
*/
#define	LPU237LOCK_DLL_RESULT_SUCCESS			0		//! processing success.
#define	LPU237LOCK_DLL_RESULT_ERROR			0xFFFFFFFF	//! processing error.( maybe system or communication error ); (-1)
#define	LPU237LOCK_DLL_RESULT_CANCEL			0xFFFFFFFE	//! processing is canceled by another reqest.(-2)

/*!
*	the callback function type.
*	this type will be used in LPU237Lock_wait_key_with_callback()
*/
typedef	void (_CALLTYPE_* type_key_callback)(void*);

#ifndef _WIN32
extern "C" {
#endif

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
	unsigned long _CALLTYPE_ LPU237Lock_dll_on();

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
	unsigned long _CALLTYPE_ LPU237Lock_dll_off();

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
	unsigned long _CALLTYPE_ LPU237Lock_get_list(wchar_t* ssDevPaths);

	/*!
	* function
	*	equal to LPU237Lock_get_list.
	*  LPU237Lock_get_list' unicode version
	*
	*	return
	*		if ssDevPaths = NULL, the number of character.(including NULL). one character size = 2 bytes
	*		else the number of connected lpu237 device.
	*/
	unsigned long _CALLTYPE_ LPU237Lock_get_list_w(wchar_t* ssDevPaths);

	/*!
	* function
	*	equal to LPU237Lock_get_list.
	*  LPU237Lock_get_list' Multi Byte Code Set version
	*
	*	return
	*		if ssDevPaths = NULL, the number of character.(including NULL). one character size = 1 bytes
	*		else the number of connected lpu237 device.
	*/
	unsigned long _CALLTYPE_ LPU237Lock_get_list_a(char* ssDevPaths);

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
	HANDLE _CALLTYPE_ LPU237Lock_open(const wchar_t* sDevPath);

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
	HANDLE _CALLTYPE_ LPU237Lock_open_w(const wchar_t* sDevPath);

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
	HANDLE _CALLTYPE_ LPU237Lock_open_a(const char* sDevPath);

	/*!
	* function
	*	close lpu237 device.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237Lock_open() )
	*
	* return
	*	if success, return LPU237LOCK_DLL_RESULT_SUCCESS
	*	else return LPU237LOCK_DLL_RESULT_ERROR
	*/
	unsigned long _CALLTYPE_ LPU237Lock_close(HANDLE hDev);

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
	unsigned long _CALLTYPE_ LPU237Lock_enable(HANDLE hDev);

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
	unsigned long _CALLTYPE_ LPU237Lock_disable(HANDLE hDev);

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
	unsigned long _CALLTYPE_ LPU237Lock_cancel_wait_key(HANDLE hDev);

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
	unsigned long _CALLTYPE_ LPU237Lock_wait_key_with_callback(HANDLE hDev, type_key_callback pFun, void* pParameter);


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
	*					LPU237Lock_cancel_wait_swipe(), LPU237Lock_wait_key_with_callback() .
	*	the number of i-button data : succession of a i-button reading operation.
	*
	*/
	unsigned long _CALLTYPE_ LPU237Lock_get_data(unsigned long dwBufferIndex, unsigned char* sKey);

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
	unsigned long _CALLTYPE_ LPU237Lock_get_id(HANDLE hDev, unsigned char* sId);


#ifndef _WIN32
}
#endif
