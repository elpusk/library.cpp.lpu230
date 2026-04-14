#pragma once

/**
* 2025.03.18  - release version 6.2 : wss client
*/

#include <mp_os_type.h>

/*!
*	constant definition.
*/
#define	LPU237_FW_SIZE_ID			16			//device ID is 16 bytes.
#define	LPU237_FW_SIZE_NAME			16			//device Name is 16 bytes.
#define	LPU237_FW_SIZE_VERSION		4			//firmware version is 4 bytes.

/*!
*	return value definition.
*/
#define	LPU237_FW_RESULT_SUCCESS			0		//! processing success.
#define	LPU237_FW_RESULT_ERROR			0xFFFFFFFF	//! processing error.( maybe system or communication error ); (-1)
#define	LPU237_FW_RESULT_CANCEL			0xFFFFFFFE	//! processing is canceled by another reqest.(-2)
#define	LPU237_FW_RESULT_TIMEOUT		0xFFFFFFFC	//! processing is timeout.(-4)
#define	LPU237_FW_RESULT_NO_MSR			0xFFFFFFFb	//! processing not found MSR .(-5)

/*!
*	deprecated - In new project, Don't use LPU237_FW_WPARAM_COMPLETE ~ LPU237_FW_WPARAM_ERROR
*	windows' message wparam value.
*   if tg_lpu237_fw.dll version is less than v6.0, wparam value is defined as below.
*/
#define	LPU237_FW_WPARAM_COMPLETE		0	//firmware update complete.
#define	LPU237_FW_WPARAM_SUCCESS		0	//firmware update complete.
#define	LPU237_FW_WPARAM_FOUND_BL		1	//found bootloader.
#define	LPU237_FW_WPARAM_SECTOR_ERASE	2
#define	LPU237_FW_WPARAM_SECTOR_WRITE	3	//includes write and verify sector space.

#define	LPU237_FW_WPARAM_ERROR			0xFFFF

/////////////////////////////////////////////////
// add 2026.04.08. 
// from tg_lpu237_fw.dll(libtg_lpu237.so) v6.x.
/*!
*	windows' message wparam value.
*   if tg_lpu237_fw.dll(libtg_lpu237.so) version is greater than equal v6.0, wparam value is defined as below.
*/

// internal use only. 
// if you are application develeoper, Don't considerate this value.
#define	_LPU237_FW_WPARAM_IGNORE		0x7FFF 

// firmware update step max. this value is for progress bar. 
// if the zero-base current step is greater than or equal to this value-1, the progress bar is full.
#define	LPU237_FW_STEP_MAX			0b11111111111111 // 16383 in decimal. 14 bits for step value.
#define	LPU237_FW_STEP_MIN			0b00000000000001 // 1 in decimal. 14 bits for step value.

#define	LPU237_FW_STEP_MASK			LPU237_FW_STEP_MAX

#define	LPU237_FW_STEP_INDEX_MIN	0	// zero base current step index min value.(10 bits)
#define	LPU237_FW_STEP_INDEX_MAX	(LPU237_FW_STEP_MAX-1) // zero base current step index max value.(10 bits)

#define	LPU237_FW_WPARAM_EX_MASK	0b1111

// NEW combination wparam!
// the format of combination wparam is 0x0000WWWW, where WWWW is defined above. (ex. LPU237_FW_WPARAM_ERROR is 0x0000FFFF)
// NNNN NNNN NNNN NNnn nnnn nnnn nnnn xxxx
// n bits for firmware update step zero-base index. (14 bits, LPU237_FW_STEP_INDEX_MIN~LPU237_FW_STEP_INDEX_MAX)
// N bits for firmware update step max.(14 bits, LPU237_FW_STEP_MIN~LPU237_FW_STEP_MAX)
// x bits for processing result
#define	LPU237_FW_GET_TOTAL_STEP(_wparam)	((_wparam & (LPU237_FW_STEP_MASK<<18))>>18)
#define	LPU237_FW_GET_STEP_INDEX(_wparam)	((_wparam & (LPU237_FW_STEP_MASK<<4))>>4)
#define	LPU237_FW_GET_RESULT_CODE(_wparam)	(_wparam & LPU237_FW_WPARAM_EX_MASK) // get LPU237_FW_WPARAM_EX_.... value.

#define	LPU237_FW_SET_TOTAL_STEP_TO_WPARAM(_wparam,_n_total)	(_wparam = ( ((_n_total & LPU237_FW_STEP_MASK)<<18) | (_wparam & ~(LPU237_FW_STEP_MASK<<18)) ))
#define	LPU237_FW_SET_STEP_INDEX_TO_WPARAM(_wparam,_n_index)	(_wparam = ( ((_n_index & LPU237_FW_STEP_MASK)<<4) | (_wparam & ~(LPU237_FW_STEP_MASK<<4)) ))
#define	LPU237_FW_SET_PROCESS_RESULT_TO_WPARAM(_wparam,_n_result)	(_wparam = ( (_n_result & LPU237_FW_WPARAM_EX_MASK) | (_wparam & ~LPU237_FW_WPARAM_EX_MASK) ))

#define	LPU237_FW_WPARAM_EX_COMPLETE		0x0	//firmware update complete.
#define	LPU237_FW_WPARAM_EX_FOUND_BL		0x1	//found bootloader.detected plug-in of bootloader.
#define	LPU237_FW_WPARAM_EX_DETECT_PLUGIN_BOOTLOADER	0x1	// = LPU237_FW_WPARAM_EX_FOUND_BL
#define	LPU237_FW_WPARAM_EX_SECTOR_ERASE	0x2
#define	LPU237_FW_WPARAM_EX_SECTOR_WRITE	0x3	//includes write and verify sector space.

#define	LPU237_FW_WPARAM_EX_BACKUP_PARAMETERS	0x4	//backup parameters.
#define	LPU237_FW_WPARAM_EX_RUN_BOOTLOADER	0x5	//run bootloader.
#define	LPU237_FW_WPARAM_EX_SETUP_BOOTLOADER	0x6	//set up bootloader.
#define	LPU237_FW_WPARAM_EX_READ_SECTOR		0x7	//read sector from file.
#define	LPU237_FW_WPARAM_EX_DETECT_PLUGOUT_LPU23X	0x8	//detect plug-out of lpu23x device.
#define	LPU237_FW_WPARAM_EX_RUN_APP					0x9	//run application in bootloader.
#define	LPU237_FW_WPARAM_EX_DETECT_PLUGOUT_BOOTLOADER	0xa	//detect plug-out of lpu237 device.
#define	LPU237_FW_WPARAM_EX_DETECT_PLUGIN_LPU23X		0xb	//detect plug-in of lpu23x device.
#define	LPU237_FW_WPARAM_EX_RECOVER_PARAMETERS			0xc	//recover parameters.
#define	LPU237_FW_WPARAM_EX_MORE_PROCESS				0xd	//extented job is proceeed.
#define	LPU237_FW_WPARAM_EX_ERROR				0xF


/*!
*	the callback function type.
*	this type will be used in LPU237_fw_msr_update()
*
*	parameters
*		1'st - user defined data.
*		2'nd - current processing result : LPU237_FW_RESULT_x
*		3'th - (deprecated) LPU237_FW_WPARAM_COMPLETE ~ LPU237_FW_WPARAM_ERROR. 
*			or 4 bytes message combination wparam 
*			you can get the number of fw-update step by LPU237_FW_GET_TOTAL_STEP(combination wparam) macro.
*			you can get the current of fw-update zero base step index by LPU237_FW_GET_STEP_INDEX(combination wparam) macro.
*			you can get the result code of fw-update current step by LPU237_FW_GET_RESULT_CODE(combination wparam) macro.
*/
typedef	unsigned long(_CALLTYPE_* type_lpu237_fw_callback)(void*, unsigned long, unsigned long);


#ifndef _WIN32
extern "C" {
#endif

	/*!
	* function
	*	initial lpu237 internal data.
	*
	* parameters
	*
	* return
	*  	LPU237_FW_RESULT_SUCCESS : only.
	*
	*/
	unsigned long _CALLTYPE_ LPU237_fw_on();

	/*!
	* function
	*	Deinitial lpu237 internal data.
	*
	* parameters
	*
	* return
	*  	LPU237_FW_RESULT_SUCCESS : only.
	*
	*/
	unsigned long _CALLTYPE_ LPU237_fw_off();

	/*!
	* function
	*	set lpu237 firmware update mode.
	*	this function is exported from version 6.0
	*	"Strongly recommend" - this function must be called after calling LPU237_fw_on() immediately, and before calling any other function.
	*	if this function is not called, the firmware update process is performed as old style. (for backward compatibility, LPU237_FW_WPARAM_x value will be used)
	* 
	* 
	* parameters
	*	nMode : 6 - the firmware update process is performed as new format.(LPU237_FW_WPARAM_EX_x value will be used)
	*		else - Don't use the other mode. This value is for future extension.
	*/
	void _CALLTYPE_ LPU237_fw_set_mode(unsigned long nMode);

	/*!
	* function
	*	get connected device list.( unicode version )
	*
	* parameters
	*	ssDevPaths : [in/out] Multi string of devices paths.
	*					this value can be NULL(0).
	*
	*	return
	*		if ssDevPaths = NULL, the number of character.(including NULL). one character size = 2 bytes
	*		else the number of connected lpu237 device.
	*/
	unsigned long _CALLTYPE_ LPU237_fw_get_list_w(wchar_t* ssDevPaths);

	/*!
	* function
	*	get connected device list.( Multi Byte Code Set version )
	*
	* parameters
	*	ssDevPaths : [in/out] Multi string of devices paths.
	*					this value can be NULL(0).
	*
	*	return
	*		if ssDevPaths = NULL, the number of character.(including NULL). one character size = 1 bytes
	*		else the number of connected lpu237 device.
	*/
	unsigned long _CALLTYPE_ LPU237_fw_get_list_a(char* ssDevPaths);


	/*!
	* function
	*	open device.( unicode version )
	*
	* parameters
	*	sDevPath : [in] device path - unicode type zero-string
	*
	* return
	*	if success, return device handle.
	*	else return INVALID_HANDLE_VALUE
	*/
	HANDLE _CALLTYPE_ LPU237_fw_open_w(const wchar_t* sDevPath);

	/*!
	* function
	*	open device.( Multi Byte Code Set version )
	*
	* parameters
	*	sDevPath : [in] device path - multi byte code set type zero-string.
	*
	* return
	*	if success, return device handle.
	*	else return INVALID_HANDLE_VALUE
	*/
	HANDLE _CALLTYPE_ LPU237_fw_open_a(const char* sDevPath);

	/*!
	* function
	*	close lpu237 device.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_fw_open() )
	*
	* return
	*	if success, return LPU237_FW_RESULT_SUCCESS
	*	else return LPU237_FW_RESULT_ERROR
	*/
	unsigned long _CALLTYPE_ LPU237_fw_close(HANDLE hDev);

	/*!
	* function
	*	save the current lpu237 device setting.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_fw_open() )
	*
	* return
	*	if success, return LPU237_FW_RESULT_SUCCESS
	*	else return LPU237_FW_RESULT_ERROR
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_save_setting(HANDLE hDev);

	/*!
	* function
	*	resetting  lpu237 device with saved setting.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_fw_open() )
	*
	* return
	*	if success, return LPU237_FW_RESULT_SUCCESS
	*	else return LPU237_FW_RESULT_ERROR
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_recover_setting(HANDLE hDev);

	/*!
	* function
	*	get device unique ID.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_fw_open() )
	*	sId : [in/out] A pointer to the buffer that save the device ID.( ID is 16 bytes )
	*			this value can be NULL(0).
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else the size of ID.[unit byte]
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_get_id(HANDLE hDev, unsigned char* sId);

	/*!
	* function
	*	get device internal name.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_fw_open() )
	*	sName : [in/out] A pointer to the buffer that save the device name.
	*			this value can be NULL(0).
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else the size of internal name.[unit byte]
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_get_name(HANDLE hDev, unsigned char* sName);

	/*!
	* function
	*	get device firmware version.
	*
	* parameters
	*	hDev : [in] device handle( return value of LPU237_fw_open() )
	*	sName : [in/out] A pointer to the buffer that save the device firmware version.( version 4 bytes )
	*			this value can be NULL(0).
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else the size of version.[unit byte]
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_get_version(HANDLE hDev, unsigned char* sVersion);

	/*!
	* function
	*	get major number from firmware version.
	*
	* parameters
	*	sVersion : [in] device firmware version( return value of LPU237_fw_msr_get_version() ).
	*			this value can be NULL(0).
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else major version number.
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_get_version_major(const unsigned char* sVersion);

	/*!
	* function
	*	get minor number from firmware version.
	*
	* parameters
	*	sVersion : [in] device firmware version( return value of LPU237_fw_msr_get_version() ).
	*			this value can be NULL(0).
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else minor version number.
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_get_version_minor(const unsigned char* sVersion);

	/*!
	* function
	*	stop operation of LPU237_fw_msr_update_x.
	*
	* parameters
	*
	* return
	*	always LPU237_FW_RESULT_ERROR( not support fw update cancel.)
	*
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_cancel_update();

	/*!
	* function
	*	start firmware update.( unicode version )
	*
	* parameters
	*	sId : [in] the device ID.( ID is 16 bytes )
	*	dwWaitTime : [in] waits time.( unit : mm sec )
	*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
	*	dwIndex : [in] the index of firmware in rom file. greater then equal -1
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR or LPU237_FW_RESULT_TIMEOUT.
	*	else LPU237_FW_RESULT_SUCCESS
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_update_w(const unsigned char* sId, unsigned long dwWaitTime, const wchar_t* sRomFileName, unsigned long dwIndex);

	/*!
	* function
	*	start firmware update..( Multi Byte Code Set version )
	*
	* parameters
	*	sId : [in] the device ID.( ID is 16 bytes )
	*	dwWaitTime : [in] waits time.( unit : mm sec )
	*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
	*	dwIndex : [in] the index of firmware in rom file. greater then equal -1
	*
	* return
	* 	if error, return LPU237_DLL_RESULT_ERROR or LPU237_FW_RESULT_TIMEOUT
	*	else LPU237_FW_RESULT_SUCCESS
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_update_a(const unsigned char* sId, unsigned long dwWaitTime, const char* sRomFileName, unsigned long dwIndex);


	/*!
	* function
	*	start firmware update.( unicode version )
	*
	* parameters
	*	sId : [in] the device ID.( ID is 16 bytes )
	*	cbUpdate : [in] callback function for serving updating information.
	*	pUser : [in] user data pointer for calling cbUpdate().
	*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
	*	dwIndex : [in] the index of firmware in rom file. greater then equal -1
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR.
	*	else LPU237_FW_RESULT_SUCCESS
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_update_callback_w(const unsigned char* sId, type_lpu237_fw_callback cbUpdate, void* pUser, const wchar_t* sRomFileName, unsigned long dwIndex);

	/*!
	* function
	*	start firmware update..( Multi Byte Code Set version )
	*
	* parameters
	*	sId : [in] the device ID.( ID is 16 bytes )
	*	cbUpdate : [in] callback function for serving updating information.
	*	pUser : [in] user data pointer for calling cbUpdate().
	*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
	*	dwIndex : [in] the index of firmware in rom file. greater then equal -1
	*
	* return
	* 	if error, return LPU237_DLL_RESULT_ERROR
	*	else LPU237_FW_RESULT_SUCCESS
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_update_callback_a(const unsigned char* sId, type_lpu237_fw_callback cbUpdate, void* pUser, const char* sRomFileName, unsigned long dwIndex);

	/*!
	* function
	*	start firmware update.( unicode version )
	*
	* parameters
	*	sId : [in] the device ID.( ID is 16 bytes )
	*	hWnd : [in] window handle for target window.
	*	uMsg : [in] window message for target window.
	*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
	*	dwIndex : [in] the index of firmware in rom file. greater then equal -1
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else LPU237_FW_RESULT_SUCCESS
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_update_wnd_w(const unsigned char* sId, HWND hWnd, UINT uMsg, const wchar_t* sRomFileName, unsigned long dwIndex);

	/*!
	* function
	*	start firmware update.( Multi Byte Code Set version )
	*
	* parameters
	*	sId : [in] the device ID.( ID is 16 bytes )
	*	hWnd : [in] window handle for target window.
	*	uMsg : [in] window message for target window.
	*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
	*	dwIndex : [in] the index of firmware in rom file. greater then equal -1
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else LPU237_FW_RESULT_SUCCESS
	*/
	unsigned long _CALLTYPE_ LPU237_fw_msr_update_wnd_a(const unsigned char* sId, HWND hWnd, UINT uMsg, const char* sRomFileName, unsigned long dwIndex);

	/*!
	* function
	*	load rom file.( unicode version )
	*
	* parameters
	*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else LPU237_FW_RESULT_SUCCESS
	*/
	unsigned long _CALLTYPE_ LPU237_fw_rom_load_w(const wchar_t* sRomFileName);

	/*!
	* function
	*	load rom file.( Multi Byte Code Set version )
	*
	* parameters
	*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else LPU237_FW_RESULT_SUCCESS
	*/
	unsigned long _CALLTYPE_ LPU237_fw_rom_load_a(const char* sRomFileName);

	/*!
	* function
	*	get firmware index value( greater then equal 0 ) in rom file.( unicode version ).
	*
	* parameters
	*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
	*	sName : [in] device internal name. LPU237_fw_msr_get_name' return value.
	*	sVersion : [in] device firmware version. LPU237_fw_msr_get_version' return value.
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else firmware index value( greater then equal 0 )
	*/
	unsigned long _CALLTYPE_ LPU237_fw_rom_get_index_w(const wchar_t* sRomFileName, const unsigned char* sName, const unsigned char* sVersion);

	/*!
	* function
	*	get firmware index value( greater then equal 0 ) in rom file.( multi-byte-code-set version ).
	*
	* parameters
	*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
	*	sName : [in] device internal name. LPU237_fw_msr_get_name' return value.
	*	sVersion : [in] device firmware version. LPU237_fw_msr_get_version' return value.
	*
	* return
	* 	if error, return LPU237_FW_RESULT_ERROR
	*	else firmware index value( greater then equal 0 )
	*/
	unsigned long _CALLTYPE_ LPU237_fw_rom_get_index_a(const char* sRomFileName, const unsigned char* sName, const unsigned char* sVersion);



#ifndef _WIN32
}
#endif
