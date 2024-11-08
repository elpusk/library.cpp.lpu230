#pragma once

#ifdef _WIN32
#define	__CALL_TYPE_DLL_SERVICE__	__stdcall

#else // linux
#define __CALL_TYPE_DLL_SERVICE__

#endif //_WIN32
/*
* warning - service dll( this dll ) can't be client of coffee manager server.!!!!!
* If service dll is client of coffee manager server, the deadlock situation will be occured.
*/


#define	COFFEE_DLL_SERVICE_RETURN_BUFFER_SIZE_MIN	18	//the minimum size of pss_multistring_return buffer by byte unit

#define	COFFEE_DLL_SERVICE_RESULT_SUCCESS		0x00000000
#define	COFFEE_DLL_SERVICE_RESULT_ERROR			0x80000000
#define	COFFEE_DLL_SERVICE_RESULT_CANCEL		0x40000000

/**
 * type_fun_sd_device_io : device sync io function prototype.
 *
 * parameter
 *		- p_dev : device object pointer
 *		- n_tx[in] : the number of tx data.
 *		- ps_tx[in] : the buffer of tx data.
 *		- pn_rx[in/out] : in - the size of ps_rx buffer. out - the received data.
 *		- ps_rx[out] : buffer of the received data.
 *
 * return
 *		- 1 : success
 *		- 0  : need more processing(pending)
 *		- -1  : error
 */
typedef	int(__CALL_TYPE_DLL_SERVICE__* type_fun_sd_device_io)(void* p_dev, unsigned long n_tx, const unsigned char* ps_tx, unsigned long* pn_rx, unsigned char* ps_rx);

/**
 * type_fun_sd_device_cancel : device sync cancel function prototype.
 *
 * parameter
 *		- p_dev : device object pointer
 * return
 *		- 1 : success
 *		- 0 : error
 */
typedef	int(__CALL_TYPE_DLL_SERVICE__* type_fun_sd_device_cancel)(void* p_dev);

/**
 * type_cb_execute : callback function prototype.
 *
 * parameter
 *		- n_result[in] : result of process COFFEE_DLL_SERVICE_RESULT_SUCCESS~COFFEE_DLL_SERVICE_RESULT_CANCEL
 *		- n_session[in] : the session number of client
 *		- pss_multistring_return[in/out] : return value multi string
 *		  the first string of multi string must be "success", "pending" or "error"
 *		  = "success" : requst is completed successfaully.
 *		- p_user[in] : user data pointer.
 */
typedef void(__CALL_TYPE_DLL_SERVICE__* type_cb_sd_execute)(unsigned long n_result, unsigned long n_session, const wchar_t* pss_multistring_response, void* p_user);

/**
 * sd_execute. : start user request. async function
 *
 * parameter
 *		- n_session : session number of this kernel
 *		- ps_device_path : device path( interface apth or symbolic link ) - you can detect device that is used in this operation. , If service dose not use a device, this is null.
 *		- p_fun_sd_device_io : device io function pointer
 *		- p_dev : device object pointer - p_fun_sd_device_io()'s the first parameter
 *		- pss_multistring_parameter[in] : parameters multi string
 *		- n_multistring_parameter_byte_unit : the size of pss_multistring_parameter buffer.(unit byte)
 *		- cb[in] : callback function pointer
 *		- p_user[in] : callback function's 4'th parameter.
 *
 * return - 1 : a execution has been started.
 *		  - 0 : execution is failed.
 */
int __CALL_TYPE_DLL_SERVICE__ sd_execute(
	unsigned long n_session
	, const wchar_t* ps_device_path
	, const type_fun_sd_device_io p_fun_sd_device_io
	, void* p_dev
	, const wchar_t* pss_multistring_parameter
	, size_t n_multistring_parameter_byte_unit
	, const type_cb_sd_execute cb
	, void* p_user
);

/**
 * sd_cancel. : cancel the current request. sync function.
 *
 * parameter
 *		- n_session : session number of this kernel
 *		- ps_device_path : device path( interface apth or symbolic link ) - you can detect device that is used in this operation.  , If service dose not use a device, this is null.
 *		- p_fun_sd_device_cancel : device cancel function pointer
 *		- p_dev : device object pointer ,p_fun_sd_device_cancel()'s the first parameter
 *
 * return - 1 : a execution has been started.
 *		  - 0 : execution is failed.
 */
int __CALL_TYPE_DLL_SERVICE__ sd_cancel(
	unsigned long n_session
	, const wchar_t* ps_device_path
	, const type_fun_sd_device_cancel p_fun_sd_device_cancel
	, void* p_dev
);

/**
 * sd_removed. - this function will be called when the session is removed.
 * sd service dll design may destrurct all object of the removed session.
 */
void __CALL_TYPE_DLL_SERVICE__ sd_removed(unsigned long n_session);