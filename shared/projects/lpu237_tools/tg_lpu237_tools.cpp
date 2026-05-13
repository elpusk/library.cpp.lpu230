
#include <websocket/mp_win_nt.h>

#include <iostream>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <list>
#include <vector>
#include <map>
#include <memory>
#include <iterator>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>

#include <mp_clog.h>
#include <mp_cconvert.h>
#include <cprotocol_lpu237.h>
#include <tg_lpu237_tools.h>
#include <mp_coffee.h>
#include <mp_coffee_path.h>

#include <manager_of_device_of_client.h>
#include <ccb_client.h>
#include <lpu237_of_client.h>
#include <cdef.h>
#include <cdll_ini.h>
#include <cmap_user_cb.h>

#define	LPU237_VID		0x134b
#define	LPU237_PID		0x0206
#define	LPU237_INF		1

#ifndef _WIN32
//linux only
static void _so_init(void) __attribute__((constructor));
static void _so_fini(void) __attribute__((destructor));
//when calls dlopen().
void _so_init(void)
{
	//printf("Shared library loaded\n");
	// NOT executed
}

//when calls dlclose().
void _so_fini(void)
{
	//printf("Shared library unloaded\n");
	// NOT executed
}
#endif // _WIN32

static std::filesystem::path _get_module_directory();

/////////////////////////////////////////////////////////////////////////
// global variable
/////////////////////////////////////////////////////////////////////////
static cmap_user_cb g_map_user_cb; //global user callback map

/////////////////////////////////////////////////////////////////////////
// local function prototype
/////////////////////////////////////////////////////////////////////////

static bool PreCheck(const _tstring& sFuntionName, DWORD dwWaitTime = 0, type_lpu237_tools_callback pFun = NULL, void* pParameter = NULL, HWND hWnd = NULL, UINT nMsg = 0);
static bool _pre_check(const std::wstring& sFuntionName, void* p_callback_fun = NULL, void* pParameter = NULL);
static _tg_sub_lpu237::CLinker::type_ptr& PreCheck(const _tstring& sFuntionName, HANDLE h_dev, DWORD dwWaitTime = 0, type_lpu237_tools_callback pFun = NULL, void* pParameter = NULL, HWND hWnd = NULL, UINT nMsg = 0);

/////////////////////////////////////////////////////////////////////////
// local function body
/////////////////////////////////////////////////////////////////////////

std::filesystem::path _get_module_directory()
{
#ifdef _WIN32
	wchar_t buffer[MAX_PATH];
	HMODULE hModule = NULL;
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCWSTR)&_get_module_directory, &hModule);
	GetModuleFileNameW(hModule, buffer, MAX_PATH);
	return std::filesystem::path(buffer).parent_path();
#else
	Dl_info info;
	if (dladdr((void*)&_get_module_directory, &info)) {
		return std::filesystem::absolute(std::filesystem::path(info.dli_fname)).parent_path();
	}
#endif
	return std::filesystem::path(""); // error
}

bool PreCheck(const _tstring& sFuntionName, DWORD dwWaitTime, type_lpu237_tools_callback pFun /*= NULL*/, void* pParameter /*=NULL*/, HWND hWnd /*= NULL*/, UINT nMsg /*= 0*/)
{
	if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : CAL : %s : 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"), sFuntionName.c_str(), dwWaitTime, pFun, pParameter, hWnd, nMsg);

	if (!_tg_sub_lpu237::cupdater::get_instance().is_worker()) {
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : gptrThread == nullptr\n"), sFuntionName.c_str());
		return false;
	}

	return true;
}

bool _pre_check(const std::wstring& sFuntionName, void* p_callback_fun /*= NULL*/, void* pParameter /*=NULL*/)
{
	if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : CAL : %s : 0x%x, 0x%x.\n"), sFuntionName.c_str(), p_callback_fun, pParameter);

	if (!_tg_sub_lpu237::cupdater::get_instance().is_worker()) {
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : gptrThread == nullptr\n"), sFuntionName.c_str());
		return false;
	}

	return true;
}

_tg_sub_lpu237::CLinker::type_ptr& PreCheck(const _tstring& sFuntionName, HANDLE h_dev, DWORD dwWaitTime /*= 0*/, type_lpu237_tools_callback pFun /*= NULL*/, void* pParameter /*= NULL*/, HWND hWnd /*= NULL*/, UINT nMsg /*= 0*/)
{
	static _tg_sub_lpu237::CLinker::type_ptr null_linker(nullptr);

	if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : CAL : %s : 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"), sFuntionName.c_str(), dwWaitTime, pFun, pParameter, hWnd, nMsg);

	if (!_tg_sub_lpu237::cupdater::get_instance().is_worker()) {
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : gptrThread == nullptr\n"), sFuntionName.c_str());
		return null_linker;
	}

	_tg_sub_lpu237::CLinker::type_ptr& linker = _tg_sub_lpu237::cupdater::type_linker_slot_for_fw::get_instance().get_linker_without_worker(h_dev);
	if (linker == nullptr) {
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : INVALID_HANDLE_VALUE\n"), sFuntionName.c_str());
	}

	return linker;
}

/////////////////////////////////////////////////////////////////////////
// exported function body
/////////////////////////////////////////////////////////////////////////
/*!
* function
*	initial lpu237 internal data.
*
* parameters
*
* return
*  	LPU237_TOOLS_RESULT_SUCCESS : only.
*
*/
unsigned long _CALLTYPE_ LPU237_tools_on()
{
	cdll_ini& cini(cdll_ini::get_instance());

#ifndef _WIN32
	std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
	std::string s_pipe_name_of_trace(_mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME);

	bool b_ini = cini.load_definition_file(_mp::ccoffee_path::get_path_of_coffee_lpu237_tools_ini_file());

	//setup tracing system
	_mp::clog& log(_mp::clog::get_instance());
	log.enable_trace(s_pipe_name_of_trace, false); //enable trace by client mode

	//setup logging system
	log.config(s_log_root_folder_except_backslash, 6, std::wstring(L"coffee_manager"), std::wstring(L"tg_lpu237_tools"), std::wstring(L"tg_lpu237_tools"));
	log.remove_log_files_older_then_now_day(cini.get_log_days_to_keep());
	log.enable(cini.get_log_enable());

	log.log_fmt(L"[I] START tg_lpu237_tools so or dll.\n");
	log.log_fmt(L"%ls", cini.get_string().c_str());
#endif
	unsigned long dwResult(ccb_client::const_dll_result_error);
	_mp::clog::get_instance().log_fmt(L" : CAL : %ls.\n", __WFUNCTION__);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		if (!ptr_manager_of_device_of_client->connect(
			ccb_client::get_callbacks()
			, cini.get_msec_timeout_ws_client_wait_for_connect_api()
			, cini.get_msec_timeout_ws_client_wait_for_ssl_handshake_complete()
			, cini.get_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss()
			, cini.get_msec_timeout_ws_client_wait_for_idle_in_wss()
			, cini.get_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss()
		)) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : manager_of_device_of_client<lpu237_of_client>::get_instance().connect().\n", __WFUNCTION__);
			continue;
		}
		dwResult = ccb_client::const_dll_result_success;
	} while (false);

	_mp::clog::get_instance().log_fmt(L" : RET : %ls.\n", __WFUNCTION__);
	return dwResult;
}

/*!
* function
*	Deinitial lpu237 internal data.
*
* parameters
*
* return
*  	LPU237_TOOLS_RESULT_SUCCESS : only.
*
*/
unsigned long _CALLTYPE_ LPU237_tools_off()
{
	unsigned long dwResult(ccb_client::const_dll_result_error);
	_mp::clog::get_instance().log_fmt(L" : CAL : %ls.\n", __WFUNCTION__);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		if (!ptr_manager_of_device_of_client->disconnect()) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : manager_of_device_of_client<lpu237_of_client>::get_instance().disconnect().\n", __WFUNCTION__);
			continue;
		}

		dwResult = ccb_client::const_dll_result_success;
	} while (false);

	manager_of_device_of_client<lpu237_of_client>::get_instance(true);//remove manager
	_mp::clog::get_instance().log_fmt(L" : RET : %ls.\n", __WFUNCTION__);
	return dwResult;
}

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
unsigned long _CALLTYPE_ LPU237_tools_get_list_w(wchar_t* ssDevPaths)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237_tools_get_list_w\n");
	const std::wstring s_filter(L"hid#vid_134b&pid_0206&mi_01");
	//_mp::type_list_wstring list_filter{ L"lpu200" };
	_mp::type_list_wstring list_dev_path;
	unsigned long dw_dev(0);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		list_dev_path = ptr_manager_of_device_of_client->get_device_list(s_filter);

		// 받은 리스트에서 특정 suffix로 끝나는 항목을 제거한다. (예를 들어, msr, ibutton, scr0 등으로 끝나는 항목을 제거한다.)
		std::vector<std::wstring> v_suffix{ L"&msr",L"&ibutton",L"&scr0" };
		list_dev_path.remove_if([&v_suffix](const std::wstring& s) {
			bool b_remove = false;

			for (auto ssuffix : v_suffix) {
				if (s.length() < ssuffix.length()) {
					continue;
				}
				if (s.compare(s.length() - ssuffix.length(), ssuffix.length(), ssuffix) == 0) {
					b_remove = true;
					break;	//exit for
				}
			}
			return b_remove;
			});
		if (list_dev_path.size() == 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : no device.\n", __WFUNCTION__);
			continue;
		}

		//
		if (ssDevPaths == NULL) {
			size_t n = _mp::cconvert::change(NULL, list_dev_path);
			dw_dev = (unsigned long)n;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : device = %u bytes.\n", __WFUNCTION__, dw_dev);
			continue;
		}

		dw_dev = (unsigned long)_mp::cconvert::change(ssDevPaths, list_dev_path);
		for (auto item : list_dev_path) {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : device = %ls.\n", __WFUNCTION__, item.c_str());
		}//end for
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : device = %u strings.\n", __WFUNCTION__, dw_dev);
	} while (0);

	return dw_dev;
}

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
HANDLE _CALLTYPE_ LPU237_tools_open_w(const wchar_t* sDevPath)
{
	HANDLE h_dev(INVALID_HANDLE_VALUE);
	unsigned long dwResult(0);
	bool b_need_close(false);
	unsigned long n_device_index(i_device_of_client::const_invalied_device_index);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls.\n", __WFUNCTION__);
		_mp::clog::get_instance().log_fmt(L" : INF : %ls : %ls\n", __WFUNCTION__, sDevPath);

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		if (!ptr_manager_of_device_of_client->get_device(std::wstring(sDevPath))->is_null_device()) {
			//alreay open.
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : already open\n", __WFUNCTION__);
			continue;
		}

		n_device_index = ptr_manager_of_device_of_client->create_device(std::wstring(sDevPath), false); // exclusive open
		if (n_device_index == i_device_of_client::const_invalied_device_index) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : create_device.\n", __WFUNCTION__);
			continue;
		}

		b_need_close = true;
		lpu237_of_client::type_ptr_lpu237_of_client& ptr_new_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (ptr_new_device->is_null_device()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : get_device.\n", __WFUNCTION__);
			continue;
		}

		if (!ptr_new_device->reset()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : reset.\n", __WFUNCTION__);
			continue;
		}

		if (!ptr_new_device->cmd_get_system_information_with_name()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : cmd_get_system_information_with_name.\n", __WFUNCTION__);
			continue;
		}
		_mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : success : cmd_get_system_information_with_name.\n", __WFUNCTION__);

		if (!ptr_new_device->cmd_get_id()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : cmd_get_id.\n", __WFUNCTION__);
			continue;
		}
		_mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : success : cmd_get_id.\n", __WFUNCTION__);

		b_need_close = false;
		h_dev = (HANDLE)(ptr_new_device->get_device_index());
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : 0x%x\n", __WFUNCTION__, h_dev);
	} while (0);

	if (b_need_close) {
		if (ptr_manager_of_device_of_client)
			ptr_manager_of_device_of_client->remove_device(n_device_index);
		//
		h_dev = INVALID_HANDLE_VALUE;
	}

	return h_dev;
}

/*!
* function
*	close lpu237 device.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*
* return
*	if success, return LPU237_TOOLS_RESULT_SUCCESS
*	else return LPU237_TOOLS_RESULT_ERROR
*/
unsigned long _CALLTYPE_ LPU237_tools_close(HANDLE hDev)
{
	unsigned long dwResult(ccb_client::const_dll_result_error);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : 0x%x\n", __WFUNCTION__, hDev);
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (ptr_device->is_null_device()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : INVALID_HANDLE_VALUE\n", __WFUNCTION__);
			continue;
		}
		/*
		if (!ptr_device->cmd_leave_opos()) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_leave_opos\n", __WFUNCTION__);
		}
		*/
		/*
		if (!ptr_device->cmd_enter_config()) {//for redetecting decoder.
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_enter_config\n", __WFUNCTION__);
		}
		if (!ptr_device->cmd_leave_config()) {//for redetecting decoder.
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_leave_config\n", __WFUNCTION__);
		}
		*/
		if (!ptr_manager_of_device_of_client->remove_device(n_device_index)) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : remove_device\n", __WFUNCTION__);
			continue;
		}

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}

/*!
* function
*	is supported magnetic card reading.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	pc_support : [in/out] 1 - be supported, 0 -not be suported
*
* return
*	if success, return LPU237_TOOLS_RESULT_SUCCESS
*	else return LPU237_TOOLS_RESULT_ERROR
*/
unsigned long __stdcall LPU237_tools_msr_is_support_msr(HANDLE hDev, unsigned char* pc_support)
{
	unsigned long dwResult(ccb_client::const_dll_result_error);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : 0x%x\n", __WFUNCTION__, hDev);
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (ptr_device->is_null_device()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : INVALID_HANDLE_VALUE\n", __WFUNCTION__);
			continue;
		}

		if (!pc_support) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : pc_support must be allocated 1 byte memory\n", __WFUNCTION__);
			continue;
		}

		cprotocol_lpu237::type_function dev_fun = ptr_device->get_device_function();
		if (dev_fun == cprotocol_lpu237::fun_msr || dev_fun == cprotocol_lpu237::fun_msr_ibutton) {
			*pc_support = 1;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : support msr\n", __WFUNCTION__);
		}
		else {
			*pc_support = 0;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : not support msr\n", __WFUNCTION__);
		}
		//
		dwResult = ccb_client::const_dll_result_success;
		
	} while (0);

	return dwResult;
}

/*!
* function
*	is supported ibutton reading.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	pc_support : [in/out] 1 - be supported, 0 -not be suported
*
* return
*	if success, return LPU237_TOOLS_RESULT_SUCCESS
*	else return LPU237_TOOLS_RESULT_ERROR
*/
unsigned long __stdcall LPU237_tools_msr_is_support_ibutton(HANDLE hDev, unsigned char* pc_support)
{
	unsigned long dwResult(ccb_client::const_dll_result_error);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : 0x%x\n", __WFUNCTION__, hDev);
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (ptr_device->is_null_device()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : INVALID_HANDLE_VALUE\n", __WFUNCTION__);
			continue;
		}

		if (!pc_support) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : pc_support must be allocated 1 byte memory\n", __WFUNCTION__);
			continue;
		}

		cprotocol_lpu237::type_function dev_fun = ptr_device->get_device_function();
		if (dev_fun == cprotocol_lpu237::fun_ibutton || dev_fun == cprotocol_lpu237::fun_msr_ibutton) {
			*pc_support = 1;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : support ibutton\n", __WFUNCTION__);
		}
		else {
			*pc_support = 0;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : notsupport ibutton\n", __WFUNCTION__);
		}
		//
		dwResult = ccb_client::const_dll_result_success;

	} while (0);

	return dwResult;
}


/*!
* function
*	get device unique ID.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	sId : [in/out] A pointer to the buffer that save the device ID.( ID is 16 bytes )
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else the size of ID.[unit byte]
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_id(HANDLE hDev, unsigned char* sId)
{
	unsigned long dwResult(ccb_client::const_dll_result_error);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : 0x%x\n", __WFUNCTION__, hDev);
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (ptr_device->is_null_device()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : INVALID_HANDLE_VALUE\n", __WFUNCTION__);
			continue;
		}
		//
		if (sId == NULL) {
			dwResult = cprotocol_lpu237::the_size_of_uid;
			continue;
		}
		_mp::type_v_buffer v_uid = ptr_device->get_device_id();
		//
		std::for_each(std::begin(v_uid), std::end(v_uid), [&](unsigned char id) {
			*sId = id;
			sId++;
			});

		dwResult = cprotocol_lpu237::the_size_of_uid;
	} while (0);

	_mp::clog::get_instance().log_fmt(L" : RET : %ls : %d\n", __WFUNCTION__, cprotocol_lpu237::the_size_of_uid);

	return dwResult;
}

/*!
* function
*	start load parameter from device.( unicode version )
*
* parameters
*	sId : [in] the device ID.( ID is 16 bytes )
*	cb : [in] callback function for get system paramter.
*	pUser : [in] user data pointer for calling cb().
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR.
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_start_get_setting(const unsigned char* sId, type_lpu237_tools_callback_get_parameter cb, void* pUser)
{
	unsigned long dw_result(ccb_client::const_dll_result_error);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		if (sId == NULL) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : ID is NULL.\n", __WFUNCTION__);
			continue;
		}
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		_mp::type_v_buffer v_id(0);
		std::copy(&sId[0], &sId[cprotocol_lpu237::the_size_of_uid], std::back_inserter(v_id));

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(v_id);
		if (ptr_device->is_null_device()) {
			dw_result = ccb_client::const_dll_result_no_msr;
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : not found device is\n", __WFUNCTION__);
			continue;
		}
		if (!ptr_device->reset()) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : reset.\n", __WFUNCTION__);
			continue;
		}
	} while (false);

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : INF : %s\n"), __WFUNCTION__);

	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);
	HANDLE hDev(NULL);
	CDev::typeUid uid;

	do {
		if (!_pre_check(std::wstring(__WFUNCTION__), cb, pUser)) {
			continue;
		}

		if (sId != NULL) {
			uid.resize(CDev::const_size_uid, 0);
			uid.assign(&sId[0], &sId[CDev::const_size_uid]);

			_tg_sub_lpu237::CLinker::type_ptr& linker = _tg_sub_lpu237::cupdater::type_linker_slot_for_fw::get_instance().get_linker_without_worker(uid);
			if (linker == nullptr) {
				if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : no device.\n"), __WFUNCTION__);
				dw_result = LPU237_TOOLS_RESULT_NO_MSR;
				continue;
			}
		}

		// generate request.......
		_tg_sub_lpu237::cupdater::type_mem_q_for_get::typePtrBuffer Req(new _tg_sub_lpu237::cupdater::type_linker_slot_for_get::type_para());
		Req->setParameters(
			_tg_sub_lpu237::cupdater::type_linker_slot_for_get::type_para::ReqCode_Paramter_Get,
			uid,
			std::make_tuple(0, cb, pUser, 0, 0, 0)
		);

		_tg_sub_lpu237::cupdater::get_instance().push_request(Req);

		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : success.\n"), __WFUNCTION__);
	} while (0);

	return dw_result;

}


/*!
* function
*	start save parameter to device.( unicode version )
*
* parameters
*	sId : [in] the device ID.( ID is 16 bytes )
*	cb : [in] callback function for set system paramter.
*	pUser : [in] user data pointer for calling cb().
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR.
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_start_set_setting(const unsigned char* sId, type_lpu237_tools_callback_set_parameter cb, void* pUser)
{
	if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : INF : %s\n"), __WFUNCTION__);

	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);
	HANDLE hDev(NULL);
	CDev::typeUid uid;

	do {
		if (!_pre_check(std::wstring(__WFUNCTION__), cb, pUser)) {
			continue;
		}

		if (sId != NULL) {
			uid.resize(CDev::const_size_uid, 0);
			uid.assign(&sId[0], &sId[CDev::const_size_uid]);

			_tg_sub_lpu237::CLinker::type_ptr& linker = _tg_sub_lpu237::cupdater::type_linker_slot_for_fw::get_instance().get_linker_without_worker(uid);
			if (linker == nullptr) {
				if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : no device.\n"), __WFUNCTION__);
				dw_result = LPU237_TOOLS_RESULT_NO_MSR;
				continue;
			}
		}

		// generate request.......
		_tg_sub_lpu237::cupdater::type_mem_q_for_set::typePtrBuffer Req(new _tg_sub_lpu237::cupdater::type_linker_slot_for_set::type_para());
		Req->setParameters(
			_tg_sub_lpu237::cupdater::type_linker_slot_for_set::type_para::ReqCode_Paramter_Set,
			uid,
			std::make_tuple(0, cb, pUser, 0, 0, 0, 0)
		);

		_tg_sub_lpu237::cupdater::get_instance().push_request(Req);

		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : success.\n"), __WFUNCTION__);
	} while (0);

	return dw_result;

}


/*!
* function
*	start load parameter from device.( unicode version ) except combination parameter
*
* parameters
*	sId : [in] the device ID.( ID is 16 bytes )
*	cb : [in] callback function for get system paramter.
*	pUser : [in] user data pointer for calling cb().
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR.
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_start_get_setting_except_combination(const unsigned char* sId, type_lpu237_tools_callback_get_parameter cb, void* pUser)
{
	if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : INF : %s\n"), __WFUNCTION__);

	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);
	HANDLE hDev(NULL);
	CDev::typeUid uid;

	do {
		if (!_pre_check(std::wstring(__WFUNCTION__), cb, pUser)) {
			continue;
		}

		if (sId != NULL) {
			uid.resize(CDev::const_size_uid, 0);
			uid.assign(&sId[0], &sId[CDev::const_size_uid]);

			_tg_sub_lpu237::CLinker::type_ptr& linker = _tg_sub_lpu237::cupdater::type_linker_slot_for_fw::get_instance().get_linker_without_worker(uid);
			if (linker == nullptr) {
				if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : no device.\n"), __WFUNCTION__);
				dw_result = LPU237_TOOLS_RESULT_NO_MSR;
				continue;
			}
		}

		// generate request.......
		_tg_sub_lpu237::cupdater::type_mem_q_for_get::typePtrBuffer Req(new _tg_sub_lpu237::cupdater::type_linker_slot_for_get::type_para());
		Req->setParameters(
			_tg_sub_lpu237::cupdater::type_linker_slot_for_get::type_para::ReqCode_Paramter_Get_Except_Combination,
			uid,
			std::make_tuple(0, cb, pUser, 0, 0, 0)
		);

		_tg_sub_lpu237::cupdater::get_instance().push_request(Req);

		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : success.\n"), __WFUNCTION__);
	} while (0);

	return dw_result;

}


/*!
* function
*	start save parameter to device.( unicode version ) except combination parameter
*
* parameters
*	sId : [in] the device ID.( ID is 16 bytes )
*	cb : [in] callback function for set system paramter.
*	pUser : [in] user data pointer for calling cb().
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR.
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_start_set_setting_except_combination(const unsigned char* sId, type_lpu237_tools_callback_set_parameter cb, void* pUser)
{
	if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : INF : %s\n"), __WFUNCTION__);

	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);
	HANDLE hDev(NULL);
	CDev::typeUid uid;

	do {
		if (!_pre_check(std::wstring(__WFUNCTION__), cb, pUser)) {
			continue;
		}

		if (sId != NULL) {
			uid.resize(CDev::const_size_uid, 0);
			uid.assign(&sId[0], &sId[CDev::const_size_uid]);

			_tg_sub_lpu237::CLinker::type_ptr& linker = _tg_sub_lpu237::cupdater::type_linker_slot_for_fw::get_instance().get_linker_without_worker(uid);
			if (linker == nullptr) {
				if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : no device.\n"), __WFUNCTION__);
				dw_result = LPU237_TOOLS_RESULT_NO_MSR;
				continue;
			}
		}

		// generate request.......
		_tg_sub_lpu237::cupdater::type_mem_q_for_set::typePtrBuffer Req(new _tg_sub_lpu237::cupdater::type_linker_slot_for_set::type_para());
		Req->setParameters(
			_tg_sub_lpu237::cupdater::type_linker_slot_for_set::type_para::ReqCode_Paramter_Set_Except_Combination,
			uid,
			std::make_tuple(0, cb, pUser, 0, 0, 0, 0)
		);

		_tg_sub_lpu237::cupdater::get_instance().push_request(Req);

		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : success.\n"), __WFUNCTION__);
	} while (0);

	return dw_result;

}

/*!
* function
* Don't use this function.
*	save the current lpu237 device setting.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*
* return
*	if success, return LPU237_TOOLS_RESULT_SUCCESS
*	else return LPU237_TOOLS_RESULT_ERROR
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_save_setting(HANDLE hDev)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		// SAVE 
		//CShare::get()->m_config = linker->get_config_parameters();// this processing is moved to worker thread.

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : success\n"), __WFUNCTION__);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	Don't use this function.
*	resetting  lpu237 device with saved setting.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*
* return
*	if success, return LPU237_TOOLS_RESULT_SUCCESS
*	else return LPU237_TOOLS_RESULT_ERROR
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_recover_setting(HANDLE hDev)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		/* this processing is moved to worker thread.
		linker->setParameters( CShare::get()->m_config );
		if( !linker->enter_config_with_server() ){
			if( CLog::GetLog() )	CLog::GetLog()->Log( true, CLog::LEV_NORMAL, _T(" : RET : %s : df_enterConfig.\n"),__WFUNCTION__ );
			continue;
		}
		if( !linker->set_system_parameters_to_server() ){
			if( CLog::GetLog() )	CLog::GetLog()->Log( true, CLog::LEV_NORMAL, _T(" : RET : %s : df_setSystemParameters.\n"),__WFUNCTION__ );
			continue;
		}
		if( !linker->apply_config_with_server() ){
			if( CLog::GetLog() )	CLog::GetLog()->Log( true, CLog::LEV_NORMAL, _T(" : RET : %s : df_applyConfig.\n"),__WFUNCTION__ );
			continue;
		}
		if( !linker->leave_config_with_server() ){
			if( CLog::GetLog() )	CLog::GetLog()->Log( true, CLog::LEV_NORMAL, _T(" : RET : %s : df_leaveConfig.\n"),__WFUNCTION__ );
			continue;
		}
		*/
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : success\n"), __WFUNCTION__);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}



/*!
* function
*	get device internal name.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	sName : [in/out] A pointer to the buffer that save the device name.
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else the size of internal name.[unit byte]
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_name(HANDLE hDev, unsigned char* sName)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		device_elpusk::CDevHidLpu237Config::type_name name = linker->getName();
		if (sName) {
			for_each(begin(name), end(name), [&](BYTE c) {
				*sName = c;
				sName++;
				});
		}

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, name.size());
		dw_result = name.size();
	} while (0);

	return dw_result;
}


/*!
* function
*	get actived interfafce and valied interfaces
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	interface : [in/out] A pointer to the buffer that save the interface number. each byte contains LPU237_TOOLS_x data.
*			this value can be NULL(0). 1'st byte - active interface number, from 2,nd the list of valied interface.
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else the number of interface +1
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_active_and_valied_interface(HANDLE hDev, unsigned char* s_inteface)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		std::vector<unsigned char> v_inf(0);

		//1'st current active interface.
		device_elpusk::CDevHidLpu237Config::type_interface inf = linker->get_config_parameters().getInterface();
		v_inf.push_back((unsigned char)inf);

		DWORD dw_system_type = linker->getSystemType();

		std::wstring s_name = linker->get_name_string();
		if (s_name.compare(L"europa") == 0) {
			v_inf.push_back((unsigned char)device_elpusk::CDevHidLpu237Config::inf_UsbVcom);
		}
		else {
			v_inf.push_back((unsigned char)device_elpusk::CDevHidLpu237Config::inf_UsbKB);
		}
		if (dw_system_type & device_elpusk::CDevHidLpu237Config::ft_ibutton) {
			v_inf.push_back((unsigned char)device_elpusk::CDevHidLpu237Config::inf_Uart);
		}
		v_inf.push_back((unsigned char)device_elpusk::CDevHidLpu237Config::inf_UsbMsr);
		if (s_inteface) {
			memcpy(s_inteface, &v_inf[0], v_inf.size());
		}
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, v_inf.size());
		dw_result = v_inf.size();
	} while (0);

	return dw_result;

}

/*!
* function
*	set active interfafce
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	c_interface : [in] active interface. LPU237_TOOLS_INF_x data.
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_interface(HANDLE hDev, unsigned char c_inteface)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		linker->get_config_parameters().setInterface((device_elpusk::CDevHidLpu237Config::type_interface)c_inteface);
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s.\n"), __WFUNCTION__);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;

}

/*!
* function
*	set active interfafce to device.(RAM) and be saved.(Flash memory)
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	pc_interface : [in/out] in - active interface, out - deactived interface. LPU237_TOOLS_INF_x data.
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_interface_to_device_and_apply(HANDLE hDev, unsigned char* pc_inteface)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (pc_inteface == NULL) {
			continue;
		}
		device_elpusk::CDevHidLpu237Config::type_pair_result_interface pair_result_interface = linker->set_interface_to_server((device_elpusk::CDevHidLpu237Config::type_interface)*pc_inteface);
		if (!pair_result_interface.first) {
			continue;
		}

		*pc_inteface = (BYTE)pair_result_interface.second;
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s.\n"), __WFUNCTION__);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	get buzzer status.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	pc_on : [in/out] A pointer to unsigned char buffer. buzzer on -> 1, buzzer off -> 0.
*			this value cannot be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_buzzer(HANDLE hDev, unsigned char* pc_on)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (!pc_on)
			continue;

		int n_buzzer = linker->get_config_parameters().getBuzzerFrequency();

		if (n_buzzer > 1000) {
			*pc_on = 1;
		}
		else {
			*pc_on = 0;
		}

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, n_buzzer);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}


#define	_CONST_INTERVAL_OFF_FREQUENCY  500
#define	_CONST_INTERVAL_ON_FREQUENCY	2600

/*!
* function
*	set buzzer status.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	c_on : [in] unsigned char buffer. buzzer on -> 1, buzzer off -> 0.
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_buzzer(HANDLE hDev, unsigned char c_on)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		if (c_on)
			linker->get_config_parameters().setBuzzerFrequency(_CONST_INTERVAL_ON_FREQUENCY);
		else
			linker->get_config_parameters().setBuzzerFrequency(_CONST_INTERVAL_OFF_FREQUENCY);
		//
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s.\n"), __WFUNCTION__);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	get language.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	pc_lang : [in/out] A pointer to unsigned char buffer. language index
*			this value cannot be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_language(HANDLE hDev, unsigned char* pc_lang)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (!pc_lang)
			continue;

		int n_lang = linker->get_config_parameters().getLanguage();
		*pc_lang = (unsigned char)n_lang;


		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, n_lang);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	set language.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	c_lang : [in] unsigned char buffer. language index(LPU237_TOOLS_LANG_x)
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_language(HANDLE hDev, unsigned char c_lang)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		linker->get_config_parameters().setLanguage((device_elpusk::CDevHidLpu237Config::type_language_map_Index)c_lang);

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s .\n"), __WFUNCTION__);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	get track status.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	s_status_3_byte : [in/out] A pointer to unsigned char buffer.
*			each track status , size is 3 bytes. index0 - iso1, index1 - iso2 and index2 - iso3
*			0 : disable, 1 : enable
*			this value cannot be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_track_status(HANDLE hDev, unsigned char* s_status_3_byte)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (!s_status_3_byte)
			continue;

		for (int i = 0; i < 3; i++) {
			if (linker->get_config_parameters().getTrackStatus((device_elpusk::CDevHidLpu237Config::type_msr_track_numer)i))
				s_status_3_byte[i] = 1;
			else
				s_status_3_byte[i] = 0;

		}//end for

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %u,%u,%u\n"), __WFUNCTION__, s_status_3_byte[0], s_status_3_byte[1], s_status_3_byte[2]);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	set track status.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	s_status_3_byte : [in] A pointer to unsigned char buffer.
*			each track status , size is 3 bytes. index0 - iso1, index1 - iso2 and index2 - iso3
*			0 : disable, 1 : enable
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_track_status(HANDLE hDev, const unsigned char* s_status_3_byte)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (!s_status_3_byte)
			continue;

		for (int i = 0; i < 3; i++) {
			if (s_status_3_byte[i])
				linker->get_config_parameters().setTrackStatus((device_elpusk::CDevHidLpu237Config::type_msr_track_numer)i, true);
			else
				linker->get_config_parameters().setTrackStatus((device_elpusk::CDevHidLpu237Config::type_msr_track_numer)i, false);
		}//end for

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %u,%u,%u\n"), __WFUNCTION__, s_status_3_byte[0], s_status_3_byte[1], s_status_3_byte[2]);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}


/*!
* function
*	get private pre/postfix of msr
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	dw_track_zero_base : [in] track number , 0~2
*	b_prefix : [in] 1 - prefix, 0 - postfix
*	s_tag : [in/out] A pointer to the buffer that save the tag.
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else the size of s_tag (unit byte)
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_private_tag(HANDLE hDev, unsigned long dw_track_zero_base, unsigned char b_prefix, unsigned char* s_tag)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (dw_track_zero_base > 2)
			continue;

		device_elpusk::CDevHidLpu237Config::type_tag v_tag;
		if (b_prefix)
			v_tag = linker->get_config_parameters().get_private_prefix((device_elpusk::CDevHidLpu237Config::type_msr_track_numer)dw_track_zero_base, 0);
		else
			v_tag = linker->get_config_parameters().get_private_postfix((device_elpusk::CDevHidLpu237Config::type_msr_track_numer)dw_track_zero_base, 0);

		if (s_tag) {
			memcpy(s_tag, &v_tag[0], v_tag.size());
		}
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, v_tag.size());
		dw_result = v_tag.size();
	} while (0);

	return dw_result;
}

/*!
* function
*	set private pre/postfix of msr
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	dw_track_zero_base : [in] track number , 0~2
*	b_prefix : [in] 1 - prefix, 0 - postfix
*	s_tag : [in] A pointer to the buffer that save the tag. if s_tag is NULL, tag is removed.
*	dw_tag : the size of s_tag. if dw_tag is zero, tag is removed.
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_private_tag(
	HANDLE hDev
	, unsigned long dw_track_zero_base
	, unsigned char b_prefix
	, const unsigned char* s_tag
	, unsigned long dw_tag
)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (dw_track_zero_base > 2)
			continue;

		device_elpusk::CDevHidLpu237Config::type_tag v_tag;
		if (s_tag && dw_tag > 0) {
			v_tag.resize(dw_tag, 0);
			std::copy(&s_tag[0], &s_tag[dw_tag], std::begin(v_tag));
		}

		if (b_prefix)
			linker->get_config_parameters().set_private_prefix((device_elpusk::CDevHidLpu237Config::type_msr_track_numer)dw_track_zero_base, 0, v_tag);
		else
			linker->get_config_parameters().set_private_postfix((device_elpusk::CDevHidLpu237Config::type_msr_track_numer)dw_track_zero_base, 0, v_tag);

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, v_tag.size());
		dw_result = v_tag.size();
	} while (0);

	return dw_result;
}


#define	IS_IBUTTON_ENABLE_F12(cBlnk)	(cBlnk[2]&0x01)
#define	IS_IBUTTON_DISABLE_ZEROS(cBlnk)	(cBlnk[2]&0x02)
#define	IS_IBUTTON_ENABLE_ZERO7(cBlnk)	(cBlnk[2]&0x04)
#define	IS_IBUTTON_ENABLE_ADDIMATT_STYLE(cBlnk)	(cBlnk[2]&0x08)
/*!
* function
*	get i-button mode.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	pc_type : [in/out] A pointer to unsigned char buffer. ibutton mode
*			this value cannot be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/

unsigned long _CALLTYPE_ LPU237_tools_msr_get_ibutton_mode(HANDLE hDev, unsigned char* pc_mode)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (!pc_mode)
			continue;

		std::vector<unsigned char>v_blank(device_elpusk::CDevHidLpu237Config::const_size_blank, 0);
		linker->get_config_parameters().get_blanks(&v_blank[0]);

		if (IS_IBUTTON_ENABLE_F12(v_blank)) {
			*pc_mode = LPU237_TOOLS_IBUTTON_MODE_F12;
		}
		else if (IS_IBUTTON_ENABLE_ZERO7(v_blank)) {
			*pc_mode = LPU237_TOOLS_IBUTTON_MODE_ZEROS7;
		}
		else if (IS_IBUTTON_ENABLE_ADDIMATT_STYLE(v_blank)) {
			*pc_mode = LPU237_TOOLS_IBUTTON_MODE_ADDMIT;
		}
		else if (IS_IBUTTON_DISABLE_ZEROS(v_blank)) {
			*pc_mode = LPU237_TOOLS_IBUTTON_MODE_NONE;
		}
		else {
			*pc_mode = LPU237_TOOLS_IBUTTON_MODE_ZEROS;
		}


		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, *pc_mode);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

#define	_SET_IBUTTON_ENABLE_F12(cBlnk)		(cBlnk[2]=(cBlnk[2]&0xF0)|(0x02|0x01))
#define	_SET_IBUTTON_ENABLE_ZEROS(cBlnk)	(cBlnk[2]=cBlnk[2]&0xF0)
#define	_SET_IBUTTON_ENABLE_ZERO7(cBlnk)	(cBlnk[2]=(cBlnk[2]&0xF0)|(0x02|0x04))
#define	_SET_IBUTTON_ENABLE_ADDIMATT_STYLE(cBlnk)	(cBlnk[2]=(cBlnk[2]&0xF0)|(0x02|0x08))
#define	_SET_IBUTTON_ENABLE_NONE(cBlnk)		(cBlnk[2]=(cBlnk[2]&0xF0)|0x02)
/*!
* function
*	set i-button mode.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	c_mode : [in] A pointer to unsigned char buffer. ibutton mode,(LPU237_TOOLS_IBUTTON_MODE_X)
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/

unsigned long _CALLTYPE_ LPU237_tools_msr_set_ibutton_mode(HANDLE hDev, unsigned char c_mode)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}


		std::vector<unsigned char>v_blank(device_elpusk::CDevHidLpu237Config::const_size_blank, 0);
		linker->get_config_parameters().get_blanks(&v_blank[0]);

		switch (c_mode)
		{
		case LPU237_TOOLS_IBUTTON_MODE_F12:
			_SET_IBUTTON_ENABLE_F12(v_blank);
			break;
		case LPU237_TOOLS_IBUTTON_MODE_ZEROS7:
			_SET_IBUTTON_ENABLE_ZERO7(v_blank);
			break;
		case LPU237_TOOLS_IBUTTON_MODE_ADDMIT:
			_SET_IBUTTON_ENABLE_ADDIMATT_STYLE(v_blank);
			break;
		case LPU237_TOOLS_IBUTTON_MODE_ZEROS:
			_SET_IBUTTON_ENABLE_ZEROS(v_blank);
			break;
		case LPU237_TOOLS_IBUTTON_MODE_NONE:
			_SET_IBUTTON_ENABLE_NONE(v_blank);
			break;
		default:
			continue;
		}//end switch

		linker->get_config_parameters().set_blanks(&v_blank[0]);

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, c_mode);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	get private pre/postfix of i button
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	b_remove : [in] 1 - remove tag, 0 - contact tag
*	b_prefix : [in] 1 - prefix, 0 - postfix
*	s_tag : [in/out] A pointer to the buffer that save the tag.
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else the size of s_tag (unit byte)
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_ibutton_tag(HANDLE hDev, unsigned char b_remove, unsigned char b_prefix, unsigned char* s_tag)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		device_elpusk::CDevHidLpu237Config::type_tag v_tag;

		if (b_remove) {
			if (!linker->get_config_parameters().is_structure_version_greater_then_equal_four())
				continue;//not support
		}

		if (b_prefix) {
			if (b_remove)
				v_tag = linker->get_config_parameters().get_prefix_ibutton_remove();
			else
				v_tag = linker->get_config_parameters().getPrefix_iButton();
		}
		else {
			if (b_remove)
				v_tag = linker->get_config_parameters().get_postfix_ibutton_remove();
			else
				v_tag = linker->get_config_parameters().getPostfix_iButton();
		}

		if (s_tag) {
			memcpy(s_tag, &v_tag[0], v_tag.size());
		}
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, v_tag.size());
		dw_result = v_tag.size();
	} while (0);

	return dw_result;
}


/*!
* function
*	set private pre/postfix of i button
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	b_remove : [in] 1 - remove tag, 0 - contact tag
*	b_prefix : [in] 1 - prefix, 0 - postfix
*	s_tag : [in] A pointer to the buffer that save the tag. if s_tag is NULL, tag is removed.
*	dw_tag : the size of s_tag. if dw_tag is zero, tag is removed.
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_ibutton_tag(HANDLE hDev, unsigned char b_remove, unsigned char b_prefix, const unsigned char* s_tag, unsigned long dw_tag)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		device_elpusk::CDevHidLpu237Config::type_tag v_tag;
		if (s_tag && dw_tag > 0) {
			v_tag.resize(dw_tag, 0);
			std::copy(&s_tag[0], &s_tag[dw_tag], std::begin(v_tag));
		}

		if (b_remove) {
			if (!linker->get_config_parameters().is_structure_version_greater_then_equal_four())
				continue;//not support
		}

		if (b_prefix) {
			if (b_remove)
				linker->get_config_parameters().set_prefix_ibutton_remove(v_tag);
			else
				linker->get_config_parameters().setPrefix_iButton(v_tag);
		}
		else {
			if (b_remove)
				linker->get_config_parameters().set_postfix_ibutton_remove(v_tag);
			else
				linker->get_config_parameters().setPostfix_iButton(v_tag);
		}

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, v_tag.size());
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	get remove-indication tag of i button
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	s_tag : [in/out] A pointer to the buffer that save the tag.
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else the size of s_tag (unit byte)
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_ibutton_remove_indication_tag(HANDLE hDev, unsigned char* s_tag)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		device_elpusk::CDevHidLpu237Config::type_tag v_tag;

		if (!linker->get_config_parameters().is_structure_version_greater_then_equal_four())
			continue;//not support

		v_tag = linker->get_config_parameters().get_ibutton_remove();

		if (s_tag) {
			memcpy(s_tag, &v_tag[0], v_tag.size());
		}
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, v_tag.size());
		dw_result = v_tag.size();
	} while (0);

	return dw_result;
}

/*!
* function
*	set remove-indication tag of i button
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	s_tag : [in] A pointer to the buffer that save the tag.
*	dw_tag : the size of s_tag
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_ibutton_remove_indication_tag(HANDLE hDev, const unsigned char* s_tag, unsigned long dw_tag)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		if (!s_tag)
			continue;
		if (dw_tag == 0)
			continue;

		device_elpusk::CDevHidLpu237Config::type_tag v_tag(&s_tag[0], &s_tag[dw_tag]);

		if (!linker->get_config_parameters().is_structure_version_greater_then_equal_four())
			continue;//not support

		linker->get_config_parameters().set_ibutton_remove(v_tag);
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, v_tag.size());
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	set device parameter to default
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_default(HANDLE hDev)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		if (linker->get_name_string().compare(L"europa") == 0) {
			linker->get_config_parameters().set_default_with_inf_is_vcom();
			if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : vcom-default.\n"), __WFUNCTION__);
		}
		else {
			linker->get_config_parameters().set_default();
			if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : default.\n"), __WFUNCTION__);
		}
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}


/*!
* function
*	get device firmware version.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	sName : [in/out] A pointer to the buffer that save the device firmware version.( version 4 bytes )
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else the size of version.[unit byte]
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_version(HANDLE hDev, unsigned char* sVersion)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		device_elpusk::CDevHidLpu237Config::type_version version = linker->get_config_parameters().get_version();

		if (sVersion) {
			int i(0);
			sVersion[i++] = version.getMajor();
			sVersion[i++] = version.getMinor();
			sVersion[i++] = version.getFix();
			sVersion[i++] = version.getBuild();
		}

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, CDevHidLpu237Config::typeVersion::const_size_typeVersion);
		dw_result = CDevHidLpu237Config::typeVersion::const_size_typeVersion;
	} while (0);

	return dw_result;
}


/*!
* function
*	get major number from firmware version.
*
* parameters
*	sVersion : [in] device firmware version( return value of LPU237_tools_msr_get_version() ).
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else major version number.
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_version_major(const unsigned char* sVersion)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : CAL : %s : 0x%X\n"), __WFUNCTION__, sVersion);
		if (sVersion == 0)
			continue;
		dw_result = sVersion[0];
	} while (0);

	if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s\n"), __WFUNCTION__);

	return dw_result;
}


/*!
* function
*	get minor number from firmware version.
*
* parameters
*	sVersion : [in] device firmware version( return value of LPU237_tools_msr_get_version() ).
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else minor version number.
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_version_minor(const unsigned char* sVersion)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : CAL : %s : 0x%X\n"), __WFUNCTION__, sVersion);
		if (sVersion == 0)
			continue;
		dw_result = sVersion[1];
	} while (0);
	if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s\n"), __WFUNCTION__);
	return dw_result;
}

/*!
* function
*	stop operation of LPU237_tools_msr_update_x.
*
* parameters
*
* return
*	if success, return LPU237_TOOLS_RESULT_SUCCESS
*	else return LPU237_TOOLS_RESULT_ERROR
*
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_cancel()
{
	DWORD dwResult(LPU237_TOOLS_RESULT_ERROR);

	do {
		if (!PreCheck(_tstring(__WFUNCTION__))) {
			continue;
		}

		// generate request.......
		_tg_sub_lpu237::cupdater::type_mem_q_for_fw::typePtrBuffer Req(new _tg_sub_lpu237::cupdater::type_linker_slot_for_fw::type_para());
		Req->setParameters(
			std::wstring(L""),
			(HWND)NULL,
			0,
			_tg_sub_lpu237::cupdater::type_linker_slot_for_fw::type_para::ReqCode_Cancel,
			CDev::typeUid(0),
			-1,
			std::make_tuple(0, nullptr, nullptr, 0, 0)
		);
		_tg_sub_lpu237::cupdater::get_instance().push_request(Req);

		dwResult = LPU237_TOOLS_RESULT_SUCCESS;
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : success\n"), __WFUNCTION__);
	} while (0);

	return dwResult;
}


/*!
* function
*	get the starting offset of i-button key range.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	pc_offset : [in/out] A pointer to unsigned char buffer. offset( 0~ 15 )
*			this value cannot be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_ibutton_start_zero_base_offset_of_range(HANDLE hDev, unsigned char* pc_offset)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (!pc_offset)
			continue;

		*pc_offset = linker->get_config_parameters().get_ibutton_start_zero_base_index();

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, *pc_offset);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	set the starting offset of i-button key range.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	c_offset : [in] unsigned char buffer. offset( 0~ 15 )
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_ibutton_start_zero_base_offset_of_range(HANDLE hDev, unsigned char c_offset)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (c_offset > LPU237_TOOLS_IBUTTON_RANGE_OFFSET_MAX) {
			if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : ERR : %s : offset = %u.\n"), __WFUNCTION__, c_offset);
			continue;
		}

		linker->get_config_parameters().set_ibutton_start_zero_base_index(c_offset);

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s .\n"), __WFUNCTION__);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}


/*!
* function
*	get the ending offset of i-button key range.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	pc_offset : [in/out] A pointer to unsigned char buffer. offset( 0~ 15 )
*			this value cannot be NULL(0).
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_get_ibutton_end_zero_base_offset_of_range(HANDLE hDev, unsigned char* pc_offset)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (!pc_offset)
			continue;

		*pc_offset = linker->get_config_parameters().get_ibutton_end_zero_base_index();

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : %d\n"), __WFUNCTION__, *pc_offset);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	set the ending offset of i-button key range.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	c_offset : [in] unsigned char buffer. offset( 0~ 15 )
*
* return
* 	if error, return LPU237_TOOLS_RESULT_ERROR
*	else LPU237_TOOLS_RESULT_SUCCESS
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_set_ibutton_end_zero_base_offset_of_range(HANDLE hDev, unsigned char c_offset)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}
		if (c_offset > LPU237_TOOLS_IBUTTON_RANGE_OFFSET_MAX) {
			if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : ERR : %s : offset = %u.\n"), __WFUNCTION__, c_offset);
			continue;
		}

		linker->get_config_parameters().set_ibutton_end_zero_base_index(c_offset);

		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s .\n"), __WFUNCTION__);
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
	} while (0);

	return dw_result;
}

/*!
* function
*	is supported ibutton range.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_tools_open() )
*	pc_support : [in/out] 1 - be supported, 0 -not be suported
*
* return
*	if success, return LPU237_TOOLS_RESULT_SUCCESS
*	else return LPU237_TOOLS_RESULT_ERROR
*/
unsigned long _CALLTYPE_ LPU237_tools_msr_is_support_ibutton_range(HANDLE hDev, unsigned char* pc_support)
{
	DWORD dw_result(LPU237_TOOLS_RESULT_ERROR);

	do {
		_tg_sub_lpu237::CLinker::type_ptr& linker = PreCheck(_tstring(__WFUNCTION__), hDev);
		if (linker == nullptr) {
			continue;
		}

		if (!pc_support)
			continue;
		std::wstring s_name = linker->get_name_string();
		device_elpusk::CDevHidLpu237Config::type_version version = linker->get_config_parameters().get_version();

		if (s_name.compare(L"callisto") == 0) {
			if (version < device_elpusk::CDevHidLpu237Config::type_version(3, 23, 0, 0)) {
				*pc_support = 0;
			}
			else {
				*pc_support = 1;
			}
		}
		else if (s_name.compare(L"ganymede") == 0) {
			if (version < device_elpusk::CDevHidLpu237Config::type_version(5, 22, 0, 0)) {
				*pc_support = 0;
			}
			else {
				*pc_support = 1;
			}
		}
		else if (s_name.compare(L"europa") == 0) {
			if (version < device_elpusk::CDevHidLpu237Config::type_version(1, 1, 0, 0)) {
				*pc_support = 0;
			}
			else {
				*pc_support = 1;
			}
		}
		else {
			continue;
		}

		//
		dw_result = LPU237_TOOLS_RESULT_SUCCESS;
		if (CLog::GetLog())	CLog::GetLog()->Log(true, CLog::LEV_NORMAL, _T(" : RET : %s : success\n"), __WFUNCTION__);
	} while (0);

	return dw_result;

}
