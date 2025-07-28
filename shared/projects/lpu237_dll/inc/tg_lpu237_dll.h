#pragma once

/*!
*  2015.04.03	- release version 1.2 : support SCR interface
*  2015.04.07	- release version 1.3 : support MSR function & SCR interface at same time.
*  2015.05.14	- release version 1.4 : support LPU-208D.
*  ?			- release version 1.5.
*  ?			- release version 1.6.
*  2017.11.18	- release version 3.0 : changed to The Next Device Manager( NDM ) client.
*  2020.06.24	- release version 4.0 : changed to coffee manager framework( CF ) client.
*  2024.04.24	- release version 5.0 : support secure
*  2024.08.0x  - release version 6.0 : wss client
*/

#include <mp_os_type.h>

/*!
*	return value definition.
*/
#define	LPU237_DLL_RESULT_SUCCESS			0		//! processing success.
#define	LPU237_DLL_RESULT_ERROR			0xFFFFFFFF	//! processing error.( maybe system or communication error ); (-1)
#define	LPU237_DLL_RESULT_CANCEL			0xFFFFFFFE	//! processing is canceled by another reqest.(-2)
#define	LPU237_DLL_RESULT_ERROR_MSR	0xFFFFFFFD	//! msr reading error (-3)

#define	LPU237_DLL_RESULT_ICC_INSERTED	0xFFFFFFFC	//! msr reading error (-4)
#define	LPU237_DLL_RESULT_ICC_REMOVED	0xFFFFFFFB	//! msr reading error (-5)

/*!
*	the callback function type.
*	this type will be used in LPU237_wait_swipe_with_callback()
*/
typedef	void (_CALLTYPE_* type_callback)(void*);

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
	*  	LPU237_DLL_RESULT_SUCCESS : only.
	*
	*/
	unsigned long _CALLTYPE_ LPU237_dll_on();

	/*!
	* function
	*	Terminate lpu237 work thread.
	*
	* parameters
	*
	* return
	*  	LPU237_DLL_RESULT_SUCCESS : only.
	*
	*/
	unsigned long _CALLTYPE_ LPU237_dll_off();

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
	unsigned long _CALLTYPE_ LPU237_get_list(wchar_t* ssDevPaths);

	/*!
	* function
	* 	Windows OS only.
	*	equal to LPU237_get_list.
	*  LPU237_get_list' unicode version
	*
	*	return
	*		if ssDevPaths = NULL, the number of character.(including NULL). one character size = 2 bytes
	*		else the number of connected lpu237 device.
	*/
	unsigned long _CALLTYPE_ LPU237_get_list_w(wchar_t* ssDevPaths);

	/*!
	* function
	* 	Windows OS only.
	*	equal to LPU237_get_list.
	*  LPU237_get_list' Multi Byte Code Set version
	*
	*	return
	*		if ssDevPaths = NULL, the number of character.(including NULL). one character size = 1 bytes
	*		else the number of connected lpu237 device.
	*/
	unsigned long _CALLTYPE_ LPU237_get_list_a(char* ssDevPaths);

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
	HANDLE _CALLTYPE_ LPU237_open(const wchar_t* sDevPath);

	/*!
	* function
	* 	Windows OS only.
	*	LPU237_open' unicode verision.
	*
	* parameters
	*	sDevPath : [in] device path
	*
	* return
	*	if success, return device handle.
	*	else return INVALID_HANDLE_VALUE
	*/
	HANDLE _CALLTYPE_ LPU237_open_w(const wchar_t* sDevPath);

	/*!
	* function
	* 	Windows OS only.
	*	LPU237_open' MBCS version.
	*
	* parameters
	*	sDevPath : [in] device path
	*
	* return
	*	if success, return device handle.
	*	else return INVALID_HANDLE_VALUE
	*/
	HANDLE _CALLTYPE_ LPU237_open_a(const char* sDevPath);

	/*!
	* function
	*	close lpu237 device.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*
	* return
	*	if success, return LPU237_DLL_RESULT_SUCCESS
	*	else return LPU237_DLL_RESULT_ERROR
	*/
	unsigned long _CALLTYPE_ LPU237_close(HANDLE hDev);

	/*!
	* function
	*	change to lpu237 reader to ready for reading a magnetic card.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*
	* return
	*	if success, return LPU237_DLL_RESULT_SUCCESS
	*	else return LPU237_DLL_RESULT_ERROR
	*
	*/
	unsigned long _CALLTYPE_ LPU237_enable(HANDLE hDev);

	/*!
	* function
	*	change to lpu237 reader to ignore reading a magnetic card.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*
	* return
	*	if success, return LPU237_DLL_RESULT_SUCCESS
	*	else return LPU237_DLL_RESULT_ERROR
	*
	*/
	unsigned long _CALLTYPE_ LPU237_disable(HANDLE hDev);

	/*!
	* function
	*	stop operation of LPU237_wait_swipe_with_waits(), LPU237_wait_swipe_with_callback() and LPU237_wait_swipe_with_message().
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*
	* return
	*	if success, return LPU237_DLL_RESULT_SUCCESS
	*	else return LPU237_DLL_RESULT_ERROR
	*
	*/
	unsigned long _CALLTYPE_ LPU237_cancel_wait_swipe(HANDLE hDev);

	/*!
	* function
	*	Windows OS only.
	*	waits to swip a magnetic card on lpu237.
	*	this function dosen't return until swiping  a magnetic card on lpu237.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*
	* return
	* 	if error, return LPU237_DLL_RESULT_ERROR
	*	else buffer index number of magnetic card data.
	*	( this buffer index number is parameter of LPU237_get_data() )
	*/
	unsigned long _CALLTYPE_ LPU237_wait_swipe_with_waits(HANDLE hDev);


	/*!
	* function
	*	wait to swip a magnetic card on lpu237.
	*	this function return immediately.
	*	and when user swipe  a magnetic card on lpu237, execute the callback funtion(pFun) with parameter(pParameter) .
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	pFun : [in] callback funtion' poniter.
	*	pParameter : [in] callback funtion' parameter( pFun' parameter )
	*
	* return
	* 	if error, return LPU237_DLL_RESULT_ERROR
	*	else buffer index number of magnetic card data.
	*	( this buffer index number is parameter of LPU237_get_data() )
	*/
	unsigned long _CALLTYPE_ LPU237_wait_swipe_with_callback(HANDLE hDev, type_callback pFun, void* pParameter);

	/*!
	* function
	* 	Windows OS only.
	*	wait to swip a magnetic card on lpu237.
	*	this function return immediately.
	*	and when user swipe  a magnetic card on lpu237, post user defined meesage(nMsg) to window(hWnd).
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	hWnd : [in] window handle.
	*	nMsg : [in] user defined meesage( pFun' parameter )
	*
	* return
	* 	if error, return LPU237_DLL_RESULT_ERROR
	*	else buffer index number of magnetic card data.
	*	( this buffer index number is parameter of LPU237_get_data() )
	*/
	unsigned long _CALLTYPE_ LPU237_wait_swipe_with_message(HANDLE hDev, HWND hWnd, UINT nMsg);

	/*!
	* function
	*	getting the magnetic card data.
	*
	* parameters
	*	dwBufferIndex : [in] buffer index number of magnetic card data.
	*							return value of LPU237_wait_swipe_with_waits(), LPU237_wait_swipe_with_callback() or LPU237_wait_swipe_with_message().
	*	dwIsoTrack : [in] magnetic card' track number - 1, 2 or 3
	*	sTrackData : [in/out] A pointer to the buffer that read the magnetic card data.
	*					this value can be NULL(0).
	*
	* return
	*  	LPU237_DLL_RESULT_ERROR : error in a magnetic card reading operation.
	*					may be error between your PC and lpu237 reader.
	*	LPU237_DLL_RESULT_CANCEL : A magnetic card reading operation is canceled by
	*					LPU237_cancel_wait_swipe(), LPU237_wait_swipe_with_waits(), LPU237_wait_swipe_with_callback() or LPU237_wait_swipe_with_message().
	*	the number of track data : succession of a magnetic card reading operation.
	*
	*/
	unsigned long _CALLTYPE_ LPU237_get_data(unsigned long dwBufferIndex, unsigned long dwIsoTrack, unsigned char* sTrackData);

	/*!
	* function
	*	get device unique ID.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	sId : [in/out] A pointer to the buffer that save the device ID.
	*			this value can be NULL(0).
	*
	* return
	* 	if error, return LPU237_DLL_RESULT_ERROR
	*	else the size of ID.[unit byte]
	*/
	unsigned long _CALLTYPE_ LPU237_get_id(HANDLE hDev, unsigned char* sId);

	/*!
	*	return value for command processing status.
	*/
#define	LPU237_DLL_SCR_RESULT_STATUS_CMD_MASK					0xC0000000

#define	LPU237_DLL_SCR_RESULT_STATUS_CMD_SUCCESS				0x00000000
#define	LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL					0x40000000
#define	LPU237_DLL_SCR_RESULT_STATUS_CMD_TIME_EX_REQ		0x80000000

	/*!
	*	the value status for smart card .
	*/
#define	LPU237_DLL_SCR_RESULT_STATUS_ICC_MASK					0x03000000

#define	LPU237_DLL_SCR_RESULT_STATUS_ICC_PRESENT_ACTIVE		0x00000000
#define	LPU237_DLL_SCR_RESULT_STATUS_ICC_PRESENT_INACTIVE	0x01000000
#define	LPU237_DLL_SCR_RESULT_STATUS_ICC_NO_PRESENT			0x02000000
#define	LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN			0x03000000

	/*!
	*	the value error for smart card status.
	*/
#define	LPU237_DLL_SCR_RESULT_ERROR_MASK										0x00FF0000

#define	LPU237_DLL_SCR_RESULT_ERROR_CMD_ABORTED							0x00FF0000
#define	LPU237_DLL_SCR_RESULT_ERROR_ICC_MUTE								0x00FE0000
#define	LPU237_DLL_SCR_RESULT_ERROR_XFR_PARITY_ERROR						0x00FD0000
#define	LPU237_DLL_SCR_RESULT_ERROR_XFR_OVERRUN							0x00FC0000
#define	LPU237_DLL_SCR_RESULT_ERROR_HW_ERROR								0x00FB0000
#define	LPU237_DLL_SCR_RESULT_ERROR_BAD_ATR_TS								0x00F80000
#define	LPU237_DLL_SCR_RESULT_ERROR_BAD_ATR_TCK							0x00F70000
#define	LPU237_DLL_SCR_RESULT_ERROR_ICC_PROTOCOL_NOT_SUPPORTED	0x00F60000
#define	LPU237_DLL_SCR_RESULT_ERROR_ICC_CLASS_NOT_SUPPORTED			0x00F50000
#define	LPU237_DLL_SCR_RESULT_ERROR_PROCEDURE_BYTE_CONFLICT			0x00F40000
#define	LPU237_DLL_SCR_RESULT_ERROR_DEACTIVATED_PROTOCOL				0x00F30000
#define	LPU237_DLL_SCR_RESULT_ERROR_BUSY_WITH_AUTO_SEQUENCE		0x00F20000
#define	LPU237_DLL_SCR_RESULT_ERROR_PIN_TIMEOUT							0x00F00000
#define	LPU237_DLL_SCR_RESULT_ERROR_PIN_CANCELLED							0x00EF0000
#define	LPU237_DLL_SCR_RESULT_ERROR_CMD_SLOT_BUSY						0x00E00000
#define	LPU237_DLL_SCR_RESULT_ERROR_CMD_UNSUPPORT						0x00000000	//The command is not supported or not allowed.
#define	LPU237_DLL_SCR_RESULT_ERROR_CMD_BAD_LENGHT						0x00010000	//Wrong command length.
#define	LPU237_DLL_SCR_RESULT_ERROR_OVER_CURRENT_POWER_OFF			0x00020000	//The reader detects an excessive current. The card is powered off.
#define	LPU237_DLL_SCR_RESULT_ERROR_DEFECTIVE_VOLTAGE_POWER_OFF	0x00030000	//The reader detects a defective voltage. The card is powered off.
#define	LPU237_DLL_SCR_RESULT_ERROR_BAD_SLOT_NUMBER					0x00050000	//The slot number is invalid (it must be set to 0 to 4).
#define	LPU237_DLL_SCR_RESULT_ERROR_BAD_DISPLAYED_BYTE0				0x00070000	//The byte displayed is invalid. 
#define	LPU237_DLL_SCR_RESULT_ERROR_BAD_DISPLAYED_BYTE1				0x00080000
#define	LPU237_DLL_SCR_RESULT_ERROR_BAD_DISPLAYED_BYTE2				0x00090000
#define	LPU237_DLL_SCR_RESULT_ERROR_BAD_DISPLAYED_BYTE3				0x000A0000
#define	LPU237_DLL_SCR_RESULT_ERROR_BAD_DISPLAYED_BYTE4				0x00150000
#define	LPU237_DLL_SCR_RESULT_ERROR_SHORT_CIRCUIT_POWER_OFF			0x00A20000	//The card is short-circuiting. The card is powered off.
#define	LPU237_DLL_SCR_RESULT_ERROR_OVER_LONG_ATR						0x00A30000	// The ATR is too long (the number of bytes is greater than 33).
#define	LPU237_DLL_SCR_RESULT_ERROR_EMV_T1_BUFFER_OVER					0x00B00000	// The reader is in EMV mode and the T=1 message sent by the card is too long. The buffer is limited to 254 bytes under the T=1 protocol.
#define	LPU237_DLL_SCR_RESULT_ERROR_EMV_PROTOCOL_ERROR				0x00BB0000	// The reader has encountered a protocol error in the EMV mode (for example, erroneous first byte of the ATR, bad checksum (TCK) character, parity error, timeout during reception of the ATR, ATR is not EMV-compliant).
#define	LPU237_DLL_SCR_RESULT_ERROR_T1_EXCHANGE_ERROR					0x00BD0000	// Card protocol error during a T=1 exchange.
#define	LPU237_DLL_SCR_RESULT_ERROR_BAD_APDU_CMD_LENGTH				0x00BE0000	//	The APDU command length is wrong.
#define	LPU237_DLL_SCR_RESULT_ERROR_ICC_RESPONSE_BUFFER_OVER		0x00830000	// A buffler overflow occurred during card data reception.


	/*!
	* function
	*	Power on smart card. and get ATR.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	cPower : [in] select power voltage.
	*				ISO mode - 01h = 5.0 V, 02h = 3.0 V, 03h = 1.8 V
	*				EMV mode - 01h = 5.0 V
	*	sRx : [out] A pointer to the buffer that save ATR.
	*			this value can be NULL(0).
	*	lpnRx : [in, out] Supplies the length, in bytes, of the sRx parameter and receives the actual number of bytes received from the smart card.
	*
	* return
	* 	concatenate Status & Error code.
	*/
	unsigned long _CALLTYPE_ LPU237_SCR_bypass_IccPowerOn(HANDLE hDev, unsigned char cPower, unsigned char* sRx, unsigned long* lpnRx);

	/*!
	* function
	*	Power off smart card.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*
	* return
	* 	concatenate Status & Error code.
	*/
	unsigned long _CALLTYPE_ LPU237_SCR_bypass_IccPowerOff(HANDLE hDev);

	/*!
	* function
	*	transmits smart card command.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	cBWI : [in] (0x00~0xFF) To extend the Block Waiting Time-out in the T=1 protocol and slot at the TPDU level.
	*	sTx : [in] A pointer to the actual data to be written to the card.
	*	nTx [in] The length, in bytes, of the sTx parameter.
	*	sRx : [out] Pointer to any data returned from the card.
	*	lpnRx : [in, out] Supplies the length, in bytes, of the sRx parameter and receives the actual number of bytes received from the smart card.
	*
	* return
	* 	concatenate Status & Error code.
	*/
	unsigned long _CALLTYPE_ LPU237_SCR_bypass_XfrBlock(HANDLE hDev, unsigned char cBWI,
		const unsigned char* sTx,
		unsigned long nTx,
		unsigned char* sRx,
		unsigned long* lpnRx
	);

	/*!
	* function
	*	gets parameters.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	sRx : [out] A pointer to the buffer that save the current parameters
	*	( Protocol, F index D index, TCCKS, GuardTime, WaitingInteger, IFSC )
	*	lpnRx : [in, out] Supplies the length, in bytes, of the sRx parameter and receives the actual number of bytes received from the smart card reader.
	*
	* return
	* 	concatenate Status & Error code.
	*/
	unsigned long _CALLTYPE_ LPU237_SCR_bypass_GetParameters(HANDLE hDev, unsigned char* sRx, unsigned long* lpnRx);

	/*!
	* function
	*	change parameters.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	cProtocol : [in] Specifies What protocol data structure follows.
	*					00h: T=0 structure
	*					01h: T=1 structure
	*	bmFindexDindex : [in] To select a baud rate conversion factor FI/DI of Table 7/8 of ISO 7816-3
	*	bmTCCKST : [in] select conversion
	*					0x00 = Direct convention
	*					0x02 = Inverse convention
	*	bGuardTime : [in] (00h?FFh) Extra guardtime between two characters.
	*	bWaitingInteger : [in] (00h?FFh)Wi for T=0 to define WWT.
	*	bIFSC : [in] ( Only used in T1 protocol, 00h?FEh ) Size of negotiated values.

	* return
	* 	concatenate Status & Error code.
	*/
	unsigned long _CALLTYPE_ LPU237_SCR_bypass_SetParameters(
		HANDLE hDev,
		unsigned char cProtocol,
		unsigned char bmFindexDindex,
		unsigned char bmTCCKST,
		unsigned char bGuardTime,
		unsigned char bWaitingInteger,
		unsigned char bIFSC//T1 only
	);

	/*!
	* function
	*	reset parameters.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*
	* return
	* 	concatenate Status & Error code.
	*/
	unsigned long _CALLTYPE_ LPU237_SCR_bypass_ResetParameters(HANDLE hDev);

	/*!
	* function
	*	retrieves information about the state of the slot.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*
	* return
	* 	concatenate Status & Error code.
	*/
	unsigned long _CALLTYPE_ LPU237_SCR_bypass_GetSlotStatus(HANDLE hDev);

	/*!
	* function
	*	used to implement Gemplus-proprietary sub-commands.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	sTx : [in] A pointer to the buffer that data block sent to smart card reader for changing setting.
	*	nTx [in] The length, in bytes, of the sTx parameter.
	*	sRx : [out] A pointer to the buffer that save smart card reader response.
	*	lpnRx : [in, out] Supplies the length, in bytes, of the sRx parameter and receives the actual number of bytes received from the smart card reader.
	*
	* return
	* 	concatenate Status & Error code.
	*/
	unsigned long _CALLTYPE_ LPU237_SCR_bypass_Escape(HANDLE hDev,
		const unsigned char* sTx,
		unsigned long nTx,
		unsigned char* sRx,
		unsigned long* lpnRx
	);

	/*!
	* function
	*	return the last result code of LPU237_SCR_helper_xxx function
	*
	* parameters
	*	none
	*
	* return
	* 	concatenate Status & Error code.
	*/
	unsigned long LPU237_SCR_helper_GetLastError();

	/*!
	* function
	*	return smart card reader module firmware version.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	sRx : [out] A pointer to the buffer that save firmware version.
	*	lpnRx : [in, out] Supplies the length, in bytes, of the sRx parameter and receives the actual number of bytes received from the smart card reader.
	*
	* return
	* 	0 : result is failure.
	*	else : result is success
	*/
	BOOL _CALLTYPE_ LPU237_SCR_helper_GetFirmwareVersion(HANDLE hDev, unsigned char* sRx, unsigned long* lpnRx);


#define	LPU237_DLL_SCR_MODE_PCSC		0x01
#define	LPU237_DLL_SCR_MODE_EMV		0x02
#define	LPU237_DLL_SCR_POWER_5V		0x10
#define	LPU237_DLL_SCR_POWER_3V		0x20
#define	LPU237_DLL_SCR_POWER_1V8		0x40
	/*!
	* function
	*	return the current reader mode(  EMV(APDU) or PC/SC(TPDU) mode ) & power voltage.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	lpcMode : [out] A pointer to the buffer that save firmware version.( one byte buffer pointer )
	*				- 0x01 : Reader in TPDU mode. supports power voltage 5V.
	*				- 0x02 : Reader in APDU mode. supports power voltage 5V.
	*				- 0x12 : Reader in APDU mode. supports power voltage 5V.
	*				- 0x22 : Reader in APDU mode. supports power voltage 3V.
	*				- 0x32 : Reader in APDU mode. supports power voltage 5V and 3V.
	*				- 0x42 : Reader in APDU mode. supports power voltage 1.8V.
	*				- 0x52 : Reader in APDU mode. supports power voltage 5V and 1.8V.
	*				- 0x62 : Reader in APDU mode. supports power voltage 3V and 1.8V.
	*				- 0x72 : Reader in APDU mode. supports power voltage 5V, 3V and 1.8V.
	*
	* return
	* 	concatenate Status & Error code.
	*/
	BOOL _CALLTYPE_ LPU237_SCR_helper_GetReaderMode(HANDLE hDev, unsigned char* lpcMode);

	/*!
	* function
	*	change the current reader mode(  EMV(APDU) or PC/SC(TPDU) mode ) & power voltage.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	cMode : [in] reader mode.
	*				- 0x01 : Reader in TPDU mode. cPower parameter is ignored.
	*				- 0x02 : Reader in APDU mode.
	*	cPower : [in] supports power voltage.
	*				- 0x00 : supports power voltage 5V.
	*				- 0x10 : supports power voltage 5V.
	*				- 0x20 : supports power voltage 3V.
	*				- 0x30 : supports power voltage 5V and 3V.
	*				- 0x40 : supports power voltage 1.8V.
	*				- 0x50 : supports power voltage 5V and 1.8V.
	*				- 0x60 : supports power voltage 3V and 1.8V.
	*				- 0x70 : supports power voltage 5V, 3V and 1.8V.
	*
	* return
	* 	0 : result is failure.
	*	else : result is success
	*/
	BOOL _CALLTYPE_ LPU237_SCR_helper_SetReaderMode(HANDLE hDev, unsigned char cMode, unsigned char cPower);

	/*!
	* function
	*	transmits smart card command.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	sTx : [in] A pointer to the actual data to be written to the card.
	*	nTx [in] The length, in bytes, of the sTx parameter.
	*	sRx : [out] Pointer to any data returned from the card.
	*	lpnRx : [in, out] Supplies the length, in bytes, of the sRx parameter and receives the actual number of bytes received from the smart card.
	*
	* return
	* 	concatenate Status & Error code.
	*/
	BOOL _CALLTYPE_ LPU237_SCR_helper_XfrBlock(HANDLE hDev,
		const unsigned char* sTx,
		unsigned long nTx,
		unsigned char* sRx,
		unsigned long* lpnRx
	);

#define	LPU237_DLL_SCR_CARD_FD_INDEX				0x00
#define	LPU237_DLL_SCR_CARD_TCCKS					0x01
#define	LPU237_DLL_SCR_CARD_GUARD_TIME			0x02
#define	LPU237_DLL_SCR_CARD_WAITING_INTEGER		0x03
#define	LPU237_DLL_SCR_CARD_CLOCK_STOP			0x04
#define	LPU237_DLL_SCR_CARD_IFSC			0x05
	/*!
	* function
	*	change card parameters.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	cProtocol : [in] Specifies What protocol data structure follows.
	*					00h: T=0 structure
	*					01h: T=1 structure
	*	cParameter : [in] Specifies parameter.
	*					- 0x00 : bmFindexDindex : A baud rate conversion factor FI/DI of Table 7/8 of ISO 7816-3
	*					- 0x01 : bmTCCKST : conversion( Direct or Inverse )
	*					- 0x02 : bGuardTime : Extra guardtime between two characters.
	*					- 0x03 : bWaitingInteger : Wi for T=0 to define WWT
	*					- 0x04 : bClockStop : support Clock stop.
	*					- 0x05 : bIFSC :  ( Only used in T1 protocol, 00h?FEh ) Size of negotiated values.
	*
	*	cNewValue : [in] new value of cParameter.
	*
	* return
	* 	0 : result is failure.
	*	else : result is success
	*/
	BOOL _CALLTYPE_ LPU237_SCR_helper_SetCardParameters(HANDLE hDev, unsigned char cProtocol, unsigned char cParameter, unsigned char cNewValue);


	/*!
	* function
	*	reset card parameters.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*
	* return
	* 	0 : result is failure.
	*	else : result is success
	*/
	BOOL _CALLTYPE_ LPU237_SCR_helper_ResetCardParameters(HANDLE hDev);

	/*!
	* function
	*	check result value( function( LPU237_SCR_x ) return value ) if or not success.
	*
	* parameters
	*	dwScrResult : [in] the result value of LPU237_SCR_x function.
	*
	* return
	* 	0 : result is failure.
	*	else : result is success
	*/
	BOOL _CALLTYPE_ LPU237_SCR_helper_result_is_success(unsigned long dwScrResult);

	/*!
	* function
	*	Power on smart card. and get ATR.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_open() )
	*	sAtr : [in] A pointer to the buffer that saved ATR.
	*	nAtr : [in] Supplies the length, in bytes, of the sAtr parameter and receives the actual number of bytes received from the smart card.
	*
	* return
	* return
	* 	0 : result is failure.
	*	else : result is success
	*/
	BOOL _CALLTYPE_ LPU237_SCR_helper_SetParameters(HANDLE hDev, const unsigned char* sAtr, unsigned long nAtr);

	/*!
	* function
	*	get ATR parameter description string( unicode version )
	*
	* parameters
	*	sOut : [out] A pointer to the buffer that will be saved description string.
	*	lpnOut : [in, out] Supplies the length, in unicode character, of the sOut parameter and receives the actual number of bytes received from the smart card reader.
	*	sAtr : [in] A pointer to the buffer that saved ATR.
	*	nAtr : [in] Supplies the length, in bytes, of the sAtr parameter and receives the actual number of bytes received from the smart card.
	*	cDelimiter : [in] delimiter character between ATR-Report item in sOut.
	*
	* return
	* return
	* 	0 : result is failure.
	*	else : result is success
	*/
	BOOL _CALLTYPE_ LPU237_SCR_helper_GetATRParameterString_w(wchar_t* sOut, unsigned long* lpnOut, const unsigned char* sAtr, unsigned long nAtr, wchar_t cDelimiter);

	/*!
	* function
	*	get ATR parameter description string( MBCS version )
	*
	* parameters
	*	sOut : [out] A pointer to the buffer that will be saved description string.
	*	lpnOut : [in, out] Supplies the length, in MBCS character, of the sOut parameter and receives the actual number of bytes received from the smart card reader.
	*	sAtr : [in] A pointer to the buffer that saved ATR.
	*	nAtr : [in] Supplies the length, in bytes, of the sAtr parameter and receives the actual number of bytes received from the smart card.
	*	cDelimiter : [in] delimiter character between ATR-Report item in sOut.
	*
	* return
	* return
	* 	0 : result is failure.
	*	else : result is success
	*/
	BOOL _CALLTYPE_ LPU237_SCR_helper_GetATRParameterString_a(char* sOut, unsigned long* lpnOut, const unsigned char* sAtr, unsigned long nAtr, char cDelimiter);
#ifndef _WIN32
}//extern "C"
#endif
