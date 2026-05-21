
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

#ifdef _WIN32
#include <atltrace.h>
#endif //_WIN32

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
static void _CALLTYPE_ _cb_get_set_parameter(void*);

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

void _CALLTYPE_ _cb_get_set_parameter(void*p_usr)
{
	static std::mutex mutex_for_cb_get_param;
	std::lock_guard<std::mutex> lock(mutex_for_cb_get_param);

	unsigned long n_device_index(i_device_of_client::const_invalied_device_index);
	int n_result_index(-1);
	long n_item_index = (long)p_usr; // item index of callback function in g_map_user_cb

	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	_mp::casync_parameter_result::type_ptr_ct_async_parameter_result ptr_result;

	_mp::cwait::type_ptr ptr_evt_complete;
	bool b_complete_transaction(true); // success or error or cancel 등으로 transaction 이 complete 된 경우, true. 아직 transaction 이 진행중인 경우, false.
	unsigned long n_cur_zero_based_phase_index(0);
	unsigned long n_resulr_of_phase(LPU237_TOOLS_RESULT_ERROR);
	bool b_must_be_canceled(false);

	do {
		bool b_get(false);
		_mp::type_v_buffer v_dev_id(0);
		size_t n_total_phase(0);
		type_lpu237_tools_callback p_fun(NULL);
		type_lpu237_tools_callback_get_parameter p_fun_get(NULL);
		type_lpu237_tools_callback_set_parameter p_fun_set(NULL);
		void* p_para(NULL);
		std::shared_ptr<std::mutex> ptr_mutex;

		// this is dregon!
		// LPU237_tools_msr_start_get_setting() 가 change_result_index() 에 도달하기 전에 이 callback 이 호출되어서 get_callback() 을 호츨하여,
		// 아직 설정되지 않은 n_result_index 값을 얻는을 수 있다,
		// 따라서
		// 먼저 mutex 만 얻고, lock() 시도해서 걸릴때 까지, 기다린다. 
		// LPU237_tools_msr_start_get_setting() 는 change_result_index() 실행 후, unlock() 하므로.
		// 여기서 lock() 이 되면 change_result_index()가 실행되었다는 의미 이므로,
		// 다시 get_callback() 을 호츨하여, 업데이트된 최종 데이터를 얻는다.
		std::tie(b_get, ptr_evt_complete, ptr_mutex) = g_map_user_cb.get_callback(
			n_item_index
			, false // 계속 콜백 가능성이	있기 때문에 콜백 정보를 얻은 후에도 콜백 정보를 유지한다. (예를 들어, 콜백이 여러번 호출되는 경우)
			, n_result_index
			, v_dev_id
			, n_total_phase
			, p_fun
			, p_fun_get
			, p_fun_set
			, p_para
		);

		if (!b_get) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : get_callback fail for item index %d.\n", __WFUNCTION__, n_item_index);
			continue;
		}

		// cancel 요청이 있으면 여기서 시도함.
		if (g_map_user_cb.is_cancel_requested()) {
			// cancel 요청 있어
			b_must_be_canceled = true;
		}

		if (n_total_phase == 0) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : n_total_phase is zero.\n", __WFUNCTION__);
			continue;
		}

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}
		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(v_dev_id);
		if (!ptr_device) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : not found device is\n", __WFUNCTION__);
			continue;
		}

		n_device_index = ptr_device->get_device_index();

		if (ptr_device->is_null_device()) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : found device is null.\n", __WFUNCTION__);
			continue;
		}

		ptr_result = ptr_manager_of_device_of_client->get_async_parameter_result_for_manager_from_all_device(n_result_index);
		if (!ptr_result) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : INVALID_HANDLE_VALUE : n_result_index = %d.\n", __WFUNCTION__, n_result_index);
			continue;
		}

		_mp::type_v_buffer v_rx;
		size_t n_remainder_phase_num = ptr_device->get_remainder_phase_number();

		n_cur_zero_based_phase_index = n_total_phase - n_remainder_phase_num - 1;//make zero based index for current phase

		if (ptr_result->get_result(v_rx)) {
			if (ptr_device->process_async_result(v_rx)) {
				n_resulr_of_phase = LPU237_TOOLS_RESULT_SUCCESS;
			}
		}

		if (b_must_be_canceled) {
			n_resulr_of_phase = LPU237_TOOLS_RESULT_CANCEL;
		}

		if (p_fun) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : undefined cb is called : n_result_index = %d.\n", __WFUNCTION__, n_result_index);
			p_fun(p_para, n_resulr_of_phase, LPU237_TOOLS_WPARAM_ERROR);
			continue;
		}
		if (p_fun_get) {
			//call back 실행
			p_fun_get(p_para, n_resulr_of_phase, n_cur_zero_based_phase_index, (unsigned long)n_total_phase);
		}
		else if (p_fun_set) {
			//call back 실행, the last parameter is RFU
			p_fun_set(p_para, n_resulr_of_phase, n_cur_zero_based_phase_index, (unsigned long)n_total_phase,0);
		}

		if (n_resulr_of_phase == LPU237_TOOLS_RESULT_CANCEL) {
			_mp::clog::get_instance().log_fmt(L" : CANCEL : %ls : user by cancel\n", __WFUNCTION__);
			continue;
		}
		else if (n_resulr_of_phase != LPU237_TOOLS_RESULT_SUCCESS) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : get_result() - error\n", __WFUNCTION__);
			continue;
		}

		// 다음  phase  시동.
		bool b_async_start(false);
		
		
		//현재 실행하려는 phase 가 마지막 이거나 transaction 의 모든 phase 가 완료된 경우, 0으로 설정된다.
		//입력 n_result_index 는 cmd_start_async_next_phase() 에서 재활용되어서, return 에서도 동일한 값을 유지한다
		std::tie(b_async_start, n_result_index, n_remainder_phase_num, b_complete_transaction) = ptr_device->cmd_start_async_next_phase(_cb_get_set_parameter, p_usr, n_result_index);
		if(!b_async_start) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_start_async_next_phase() fail.\n", __WFUNCTION__);
			continue;
		}

		if (!b_complete_transaction) {
			continue; // 아직 transaction 이 완료되지 않은 경우, 다음 phase 의 callback 이 호출될 때 까지 기다린다.
		}

		// 여기는 남은 transaction 이 complete 된 경우.

	} while (false);

	if (b_complete_transaction) {
		g_map_user_cb.remove_callback(n_item_index); // remove invalid callback
		if (ptr_manager_of_device_of_client) {
			ptr_manager_of_device_of_client->remove_async_result_for_manager(n_device_index, n_result_index);
			ptr_result.reset(); // release result
			ptr_manager_of_device_of_client.reset();
		}
	}

	if (b_must_be_canceled) {
		g_map_user_cb.cancel_done();// 요청된 cancel 을 했다고 알림.
	}
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
unsigned long _CALLTYPE_ LPU237_tools_msr_start_get_setting(
	const unsigned char* sId
	, type_lpu237_tools_callback_get_parameter cb
	, void* pUser
)
{
	unsigned long dw_result(ccb_client::const_dll_result_error);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	long n_item_index(-1);
	size_t n_total_phase_in_this_transaction(0);

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
				
		std::shared_ptr<std::mutex> ptr_mutex;

		std::tie(n_item_index,std::ignore, ptr_mutex) = g_map_user_cb.add_callback(-1, v_id, cb, pUser);
		if (n_item_index < 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : [critical error]add_callback error.\n", __WFUNCTION__);
			continue;
		}

		bool b_result(false);
		int n_result_index(_mp::casync_result_manager::const_invalied_result_index);
		// _cb_get_set_parameter 가 change_result_index() 에 도달하기 전에 호출되어서 get_callback() 을 호츨하여,
		// 아직 설정되지 않은 n_result_index 값을 얻는 것을 방지 한다.
		ptr_mutex->lock();

		// 필요한 모든 parameter 얻기 transaction 를 비동기 방식으로 시작한다.
		// 각 phase 완료 시점(에러 또는 성공)에 _cb_get_set_parameter callback 함수를 호출한다.
		// n_item_index 는 _cb_get_set_parameter() 호출시 전달되는 user data로써, g_map_user_cb 에서 관련 callback 정보를 얻기 위한 key값으로 사용된다.
		std::tie(b_result, n_result_index, n_total_phase_in_this_transaction) = ptr_device->cmd_start_async_get_parameters(
			_cb_get_set_parameter, (void*)n_item_index
		);
		if(!b_result){
			ptr_mutex->unlock();
			g_map_user_cb.remove_callback(n_item_index);
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_start_async_get_parameters error.\n", __WFUNCTION__);
			continue;
		}
		else {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : created result index = %u.\n", __WFUNCTION__, n_result_index);
			g_map_user_cb.change_result_index(n_item_index, n_result_index, n_total_phase_in_this_transaction);// 얻어진 정상적인 result index로 item의 result index를 변경한다.
			ptr_mutex->unlock();
		}

		dw_result = ccb_client::const_dll_result_success;

	} while (false);

	switch (dw_result) {
	case ccb_client::const_dll_result_success:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success - %d.\n", __WFUNCTION__, n_item_index);
		break;

	case ccb_client::const_dll_result_error:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : error - %d.\n", __WFUNCTION__, n_item_index);
		break;
	case ccb_client::const_dll_result_timeout:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : timeout - %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error; // 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	case ccb_client::const_dll_result_no_msr:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : no msr - %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error;// 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	default:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error;// 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	}//end switch
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
unsigned long _CALLTYPE_ LPU237_tools_msr_start_set_setting(
	const unsigned char* sId
	, type_lpu237_tools_callback_set_parameter cb, void* pUser
)
{
	unsigned long dw_result(ccb_client::const_dll_result_error);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	long n_item_index(-1);
	size_t n_total_phase_in_this_transaction(0);

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

		std::shared_ptr<std::mutex> ptr_mutex;

		std::tie(n_item_index, std::ignore, ptr_mutex) = g_map_user_cb.add_callback(-1, v_id, cb, pUser);
		if (n_item_index < 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : [critical error]add_callback error.\n", __WFUNCTION__);
			continue;
		}

		bool b_result(false);
		int n_result_index(_mp::casync_result_manager::const_invalied_result_index);
		// _cb_get_set_parameter 가 change_result_index() 에 도달하기 전에 호출되어서 get_callback() 을 호츨하여,
		// 아직 설정되지 않은 n_result_index 값을 얻는 것을 방지 한다.
		ptr_mutex->lock();

		// 필요한 모든 parameter 얻기 transaction 를 비동기 방식으로 시작한다.
		// 각 phase 완료 시점(에러 또는 성공)에 _cb_get_set_parameter callback 함수를 호출한다.
		// n_item_index 는 _cb_get_set_parameter() 호출시 전달되는 user data로써, g_map_user_cb 에서 관련 callback 정보를 얻기 위한 key값으로 사용된다.
		std::tie(b_result, n_result_index, n_total_phase_in_this_transaction) = ptr_device->cmd_start_async_set_parameters(_cb_get_set_parameter, (void*)n_item_index);
		if (!b_result) {
			ptr_mutex->unlock();
			g_map_user_cb.remove_callback(n_item_index);
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_async_set_parameters error.\n", __WFUNCTION__);
			continue;
		}
		else {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : created result index = %u.\n", __WFUNCTION__, n_result_index);
			g_map_user_cb.change_result_index(n_item_index, n_result_index, n_total_phase_in_this_transaction);// 얻어진 정상적인 result index로 item의 result index를 변경한다.
			ptr_mutex->unlock();
		}

		dw_result = ccb_client::const_dll_result_success;

	} while (false);

	switch (dw_result) {
	case ccb_client::const_dll_result_success:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success - %d.\n", __WFUNCTION__, n_item_index);
		break;

	case ccb_client::const_dll_result_error:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : error - %d.\n", __WFUNCTION__, n_item_index);
		break;
	case ccb_client::const_dll_result_timeout:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : timeout - %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error; // 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	case ccb_client::const_dll_result_no_msr:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : no msr - %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error;// 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	default:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error;// 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	}//end switch
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
unsigned long _CALLTYPE_ LPU237_tools_msr_start_get_setting_except_combination(
	const unsigned char* sId
	, type_lpu237_tools_callback_get_parameter cb
	, void* pUser
)
{
	unsigned long dw_result(ccb_client::const_dll_result_error);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	long n_item_index(-1);
	size_t n_total_phase_in_this_transaction(0);

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

		std::shared_ptr<std::mutex> ptr_mutex;

		std::tie(n_item_index, std::ignore, ptr_mutex) = g_map_user_cb.add_callback(-1, v_id, cb, pUser);
		if (n_item_index < 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : [critical error]add_callback error.\n", __WFUNCTION__);
			continue;
		}

		bool b_result(false);
		int n_result_index(_mp::casync_result_manager::const_invalied_result_index);
		// _cb_get_set_parameter 가 change_result_index() 에 도달하기 전에 호출되어서 get_callback() 을 호츨하여,
		// 아직 설정되지 않은 n_result_index 값을 얻는 것을 방지 한다.
		ptr_mutex->lock();

		// 필요한 모든 parameter 얻기 transaction 를 비동기 방식으로 시작한다.
		// 각 phase 완료 시점(에러 또는 성공)에 _cb_get_set_parameter callback 함수를 호출한다.
		// n_item_index 는 _cb_get_set_parameter() 호출시 전달되는 user data로써, g_map_user_cb 에서 관련 callback 정보를 얻기 위한 key값으로 사용된다.
		std::tie(b_result, n_result_index, n_total_phase_in_this_transaction) = ptr_device->cmd_start_async_get_parameters_except_combination(_cb_get_set_parameter, (void*)n_item_index);
		if (!b_result) {
			ptr_mutex->unlock();
			g_map_user_cb.remove_callback(n_item_index);
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_async_get_parameters_except_combination error.\n", __WFUNCTION__);
			continue;
		}
		else {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : created result index = %u.\n", __WFUNCTION__, n_result_index);
			g_map_user_cb.change_result_index(n_item_index, n_result_index, n_total_phase_in_this_transaction);// 얻어진 정상적인 result index로 item의 result index를 변경한다.
			ptr_mutex->unlock();
		}

		dw_result = ccb_client::const_dll_result_success;

	} while (false);

	switch (dw_result) {
	case ccb_client::const_dll_result_success:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success - %d.\n", __WFUNCTION__, n_item_index);
		break;

	case ccb_client::const_dll_result_error:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : error - %d.\n", __WFUNCTION__, n_item_index);
		break;
	case ccb_client::const_dll_result_timeout:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : timeout - %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error; // 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	case ccb_client::const_dll_result_no_msr:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : no msr - %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error;// 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	default:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error;// 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	}//end switch
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
unsigned long _CALLTYPE_ LPU237_tools_msr_start_set_setting_except_combination(
	const unsigned char* sId
	, type_lpu237_tools_callback_set_parameter cb
	, void* pUser
)
{
	unsigned long dw_result(ccb_client::const_dll_result_error);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	long n_item_index(-1);
	size_t n_total_phase_in_this_transaction(0);

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

		std::shared_ptr<std::mutex> ptr_mutex;

		std::tie(n_item_index, std::ignore, ptr_mutex) = g_map_user_cb.add_callback(-1, v_id, cb, pUser);
		if (n_item_index < 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : [critical error]add_callback error.\n", __WFUNCTION__);
			continue;
		}

		bool b_result(false);
		int n_result_index(_mp::casync_result_manager::const_invalied_result_index);
		// _cb_get_set_parameter 가 change_result_index() 에 도달하기 전에 호출되어서 get_callback() 을 호츨하여,
		// 아직 설정되지 않은 n_result_index 값을 얻는 것을 방지 한다.
		ptr_mutex->lock();

		// 필요한 모든 parameter 얻기 transaction 를 비동기 방식으로 시작한다.
		// 각 phase 완료 시점(에러 또는 성공)에 _cb_get_set_parameter callback 함수를 호출한다.
		// n_item_index 는 _cb_get_set_parameter() 호출시 전달되는 user data로써, g_map_user_cb 에서 관련 callback 정보를 얻기 위한 key값으로 사용된다.
		std::tie(b_result, n_result_index, n_total_phase_in_this_transaction) = ptr_device->cmd_start_async_set_parameters_except_combination(_cb_get_set_parameter, (void*)n_item_index);
		if (!b_result) {
			ptr_mutex->unlock();
			g_map_user_cb.remove_callback(n_item_index);
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_async_set_parameters_except_combination error.\n", __WFUNCTION__);
			continue;
		}
		else {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : created result index = %u.\n", __WFUNCTION__, n_result_index);
			g_map_user_cb.change_result_index(n_item_index, n_result_index, n_total_phase_in_this_transaction);// 얻어진 정상적인 result index로 item의 result index를 변경한다.
			ptr_mutex->unlock();
		}

		dw_result = ccb_client::const_dll_result_success;

	} while (false);

	switch (dw_result) {
	case ccb_client::const_dll_result_success:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success - %d.\n", __WFUNCTION__, n_item_index);
		break;

	case ccb_client::const_dll_result_error:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : error - %d.\n", __WFUNCTION__, n_item_index);
		break;
	case ccb_client::const_dll_result_timeout:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : timeout - %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error; // 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	case ccb_client::const_dll_result_no_msr:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : no msr - %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error;// 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	default:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %d.\n", __WFUNCTION__, n_item_index);
		dw_result = ccb_client::const_dll_result_error;// 기존 return 값 정의가 LPU237_TOOLS_RESULT_ERROR 또는 LPU237_TOOLS_RESULT_SUCCESS 만 있어서, timeout 의 경우 error 로 간주하기로 함.
		break;
	}//end switch
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
		if (sName == NULL) {
			dwResult = cprotocol_lpu237::the_size_of_name;
			continue;
		}
		_mp::type_v_buffer v_n = ptr_device->get_name();
		//
		std::for_each(std::begin(v_n), std::end(v_n), [&](unsigned char c) {
			*sName = c;
			sName++;
			});

		dwResult = cprotocol_lpu237::the_size_of_name;
	} while (0);

	_mp::clog::get_instance().log_fmt(L" : RET : %ls : %d\n", __WFUNCTION__, cprotocol_lpu237::the_size_of_name);

	return dwResult;
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
		_mp::type_v_buffer v_inf(0);
		//1'st current active interface.
		unsigned char c_inf = (unsigned char)ptr_device->get_interface();
		v_inf.push_back(c_inf);

		cprotocol_lpu237::type_function dev_fun = ptr_device->get_device_function();
		std::wstring s_name = ptr_device->get_name_by_wstring();

		if (s_name.compare(L"europa") == 0) {
			v_inf.push_back((unsigned char)cprotocol_lpu237::System_interface_usb_vcom);
		}
		else {
			v_inf.push_back((unsigned char)cprotocol_lpu237::system_interface_usb_keyboard);
		}
		if (dev_fun == cprotocol_lpu237::fun_ibutton || dev_fun == cprotocol_lpu237::fun_msr_ibutton) {
			v_inf.push_back((unsigned char)cprotocol_lpu237::system_interface_uart);
		}
		if (dev_fun == cprotocol_lpu237::fun_msr || dev_fun == cprotocol_lpu237::fun_msr_ibutton) {
			v_inf.push_back((unsigned char)cprotocol_lpu237::system_interface_usb_msr);
		}
		if (s_inteface) {
			memcpy(s_inteface, &v_inf[0], v_inf.size());
		}

		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, v_inf.size());
		dwResult = v_inf.size();
	} while (0);

	return dwResult;
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
		ptr_device->set_interface((cprotocol_lpu237::type_system_interface)c_inteface);

		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, c_inteface);
		dwResult = ccb_client::const_dll_result_success;
	} while (0);

	return dwResult;
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
		if (pc_inteface == NULL) {
			continue;
		}

		unsigned char c_cur_inf = (unsigned char)ptr_device->get_interface();

		ptr_device->set_interface((cprotocol_lpu237::type_system_interface)*pc_inteface);
		*pc_inteface = c_cur_inf;


		if (!ptr_device->cmd_changed_interface_apply()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : fail cmd_changed_interface_apply\n", __WFUNCTION__);
			continue;
		}

		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
		dwResult = ccb_client::const_dll_result_success;
	} while (0);

	return dwResult;
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
		if (pc_on == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : pc_on == null\n", __WFUNCTION__);
			continue;
		}

		// NDM 이나 direct 버전은 device 에서 받은 값을 10으로 나누어서 저장해서 사용했고,
		// device 송신 전에 저장값에 10을 곱해서 device 에게 송신하는 방식을 사용했지만
		// cf2 에서는 device 에서 받은 값을 그대로 저장해서 사용하고, device 송신 전에 저장값을 그대로 송신하는 방식으로 변경했다.
		// 따라서 cprotocol_lpu237::the_frequency_of_off_buzzer, cprotocol_lpu237::the_frequency_of_on_buzzer 값이 기존 버전보다 10배 증가했다.
		// buzzer off 시, callisto 와 ganymede 는  frequency-cnt을 항상 5000 을 사용했음.
		// buzzer off 시, MH1902T 를 사용하는 모델은  frequency-cnt을 항상 50 을 사용했음.
		// 2016.11.17 callisto(v3.15), ganymede(v5.7) 부터 buzzer on 시, fw 에서 frequency-cnt 값을 25000 에서 26000 으로 변경했고,
		// fw 에서 frequency-cnt 에 25000(2.8KHz) 을 설정하려고 하면, 자동적으로 26000 으로 설정되도록 했음.
		// 따라서 26000 이 변경된 기본 값이지만, 25000 을 사용해도, flash 에 저장되는 parameter 는 25000 이지만, frequency-cnt 는 26000(3KHz) 으로 설정되어
		// 동작은 이상 없음.
		// MH1902T 를 사용하는 모델은  buzzer on 시, frequency-cnt을 항상 45000 을 사용했음.
		// MH1902T fw 는 frequency-cnt 설정시, flash 에 설정값이 25000 이나 26000 이면, 45000 으로 설정해서 사용하게 되어 있어서
		// on 시 26000 으로 frequency-cnt 값을 설정해더라도 문제 없고,
		// off 시 5000 으로 frequency-cnt 값을 설정하려고 하면 자동적으로 50 으로 설정되도록 했음.
		// 결론적으로 cf2 에서는 buzzer off 시, 는 5000 을 사용하고,
		// buzzer on 시, 는 26000 을 사용하면 모델에 상관없이 정상적으로 동작함.
		uint32_t n_frequency = ptr_device->get_buzzer_frequency();

		if(n_frequency > cprotocol_lpu237::the_frequency_of_off_buzzer) {
			*pc_on = 1;
		}
		else {
			*pc_on = 0;
		}

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %d\n", __WFUNCTION__, cprotocol_lpu237::the_size_of_name);
	} while (0);

	return dwResult;
}


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

		uint32_t n_frequency = cprotocol_lpu237::the_frequency_of_off_buzzer;
		if (c_on) {
			n_frequency = cprotocol_lpu237::the_frequency_of_on_buzzer;
		}

		// NDM 이나 direct 버전은 device 에서 받은 값을 10으로 나누어서 저장해서 사용했고,
		// device 송신 전에 저장값에 10을 곱해서 device 에게 송신하는 방식을 사용했지만
		// cf2 에서는 device 에서 받은 값을 그대로 저장해서 사용하고, device 송신 전에 저장값을 그대로 송신하는 방식으로 변경했다.
		// 따라서 cprotocol_lpu237::the_frequency_of_off_buzzer, cprotocol_lpu237::the_frequency_of_on_buzzer 값이 기존 버전보다 10배 증가했다.
		// buzzer off 시, callisto 와 ganymede 는  frequency-cnt을 항상 5000 을 사용했음.
		// buzzer off 시, MH1902T 를 사용하는 모델은  frequency-cnt을 항상 50 을 사용했음.
		// 2016.11.17 callisto(v3.15), ganymede(v5.7) 부터 buzzer on 시, fw 에서 frequency-cnt 값을 25000 에서 26000 으로 변경했고,
		// fw 에서 frequency-cnt 에 25000(2.8KHz) 을 설정하려고 하면, 자동적으로 26000 으로 설정되도록 했음.
		// 따라서 26000 이 변경된 기본 값이지만, 25000 을 사용해도, flash 에 저장되는 parameter 는 25000 이지만, frequency-cnt 는 26000(3KHz) 으로 설정되어
		// 동작은 이상 없음.
		// MH1902T 를 사용하는 모델은  buzzer on 시, frequency-cnt을 항상 45000 을 사용했음.
		// MH1902T fw 는 frequency-cnt 설정시, flash 에 설정값이 25000 이나 26000 이면, 45000 으로 설정해서 사용하게 되어 있어서
		// on 시 26000 으로 frequency-cnt 값을 설정해더라도 문제 없고,
		// off 시 5000 으로 frequency-cnt 값을 설정하려고 하면 자동적으로 50 으로 설정되도록 했음.
		// 결론적으로 cf2 에서는 buzzer off 시, 는 5000 을 사용하고,
		// buzzer on 시, 는 26000 을 사용하면 모델에 상관없이 정상적으로 동작함.
		ptr_device->set_buzzer_frequency(n_frequency);

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : set-freq : %u\n", __WFUNCTION__, n_frequency);
	} while (0);

	return dwResult;
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
		if (pc_lang == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : pc_lang == null\n", __WFUNCTION__);
			continue;
		}

		cprotocol_lpu237::type_keyboard_language_index n_lang = ptr_device->get_language();
		*pc_lang = (unsigned char)n_lang;

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, (unsigned char)n_lang);
	} while (0);

	return dwResult;
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
		if (c_lang > (unsigned char)cprotocol_lpu237::language_map_index_turkey) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : undefined language index : %u\n", __WFUNCTION__, c_lang);
			continue;
		}

		ptr_device->set_language((cprotocol_lpu237::type_keyboard_language_index)c_lang);

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : language index : %u\n", __WFUNCTION__, c_lang);
	} while (0);

	return dwResult;
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
		if (s_status_3_byte == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : s_status_3_byte == null\n", __WFUNCTION__);
			continue;
		}

		for( int i = 0; i < 3; i++) {
			s_status_3_byte[i] = ptr_device->get_enable_track(i);
		}//end for


		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
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
		if (s_status_3_byte == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : s_status_3_byte == null\n", __WFUNCTION__);
			continue;
		}

		for( int i = 0; i < 3; i++) {
			if (s_status_3_byte[i]) {
				ptr_device->set_enable_track(i,true );
			}
			else {
				ptr_device->set_enable_track(i, false);
			}
		}//end for

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success : %u\n", __WFUNCTION__);
	} while (0);

	return dwResult;
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
unsigned long _CALLTYPE_ LPU237_tools_msr_get_private_tag(
	HANDLE hDev
	, unsigned long dw_track_zero_base
	, unsigned char b_prefix
	, unsigned char* s_tag
)
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
		bool b_prefix_bool = false;
		if(b_prefix) {
			b_prefix_bool = true;
		}
		_mp::type_v_buffer v;
		v = ptr_device->get_msr_private_tag(dw_track_zero_base, b_prefix_bool);
		dwResult = v.size();

		if(s_tag == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : s_tag == null : %u\n", __WFUNCTION__, dwResult);
			continue;
		}

		std::copy(std::begin(v), std::end(v), s_tag);
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, dwResult);
	} while (0);

	return dwResult;
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
		bool b_prefix_bool = false;
		if (b_prefix) {
			b_prefix_bool = true;
		}
		_mp::type_v_buffer v;
		if (s_tag && dw_tag > 0 && (dw_tag%2 == 0)) {
			std::copy(s_tag, s_tag + dw_tag, std::back_inserter(v));
		}

		ptr_device->set_msr_private_tag(dw_track_zero_base, b_prefix_bool,v);
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, dwResult);
	} while (0);

	return dwResult;
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
		if (pc_mode == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : pc_mode == null\n", __WFUNCTION__);
			continue;
		}

		cprotocol_lpu237::type_ibutton_mode m = ptr_device->get_ibutton_mode();
		switch (m) {
		case cprotocol_lpu237::ibutton_none:
			*pc_mode = LPU237_TOOLS_IBUTTON_MODE_NONE;
			break;
		case cprotocol_lpu237::ibutton_zeros:
			*pc_mode = LPU237_TOOLS_IBUTTON_MODE_ZEROS;
			break;
		case cprotocol_lpu237::ibutton_f12:
			*pc_mode = LPU237_TOOLS_IBUTTON_MODE_F12;
			break;
		case cprotocol_lpu237::ibutton_zeros7:
			*pc_mode = LPU237_TOOLS_IBUTTON_MODE_ZEROS7;
			break;
		case cprotocol_lpu237::ibutton_addmit:
			*pc_mode = LPU237_TOOLS_IBUTTON_MODE_ADDMIT;
			break;
		default:
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : undefined mode : %u\n", __WFUNCTION__, (unsigned char)m);
			continue;
		}//end switch

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, (unsigned char)m);
	} while (0);

	return dwResult;
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
		switch(c_mode) {
				case LPU237_TOOLS_IBUTTON_MODE_NONE:
					ptr_device->set_ibutton_mode(cprotocol_lpu237::ibutton_none);
					break;
				case LPU237_TOOLS_IBUTTON_MODE_ZEROS:
					ptr_device->set_ibutton_mode(cprotocol_lpu237::ibutton_zeros);
					break;
				case LPU237_TOOLS_IBUTTON_MODE_F12:
					ptr_device->set_ibutton_mode(cprotocol_lpu237::ibutton_f12);
					break;
				case LPU237_TOOLS_IBUTTON_MODE_ZEROS7:
					ptr_device->set_ibutton_mode(cprotocol_lpu237::ibutton_zeros7);
					break;
				case LPU237_TOOLS_IBUTTON_MODE_ADDMIT:
					ptr_device->set_ibutton_mode(cprotocol_lpu237::ibutton_addmit);
					break;
				default:
					_mp::clog::get_instance().log_fmt(L" : RET : %ls : undefined mode : %u\n", __WFUNCTION__, c_mode);
					continue;
		}//end switch

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : ibutton mode : %u\n", __WFUNCTION__, c_mode);
	} while (0);

	return dwResult;
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
		bool b_remove_bool = false;
		if (b_remove) {
			b_remove_bool = true;
		}
		bool b_prefix_bool = false;
		if (b_prefix) {
			b_prefix_bool = true;
		}

		_mp::type_v_buffer v = ptr_device->get_ibutton_tag(b_remove_bool, b_prefix_bool);
		dwResult = v.size();

		if(s_tag == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : s_tag == null : %u\n", __WFUNCTION__, v.size());
			continue;
		}

		std::copy(v.begin(), v.end(), s_tag);
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, (unsigned char)v.size());
	} while (0);

	return dwResult;
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
		bool b_remove_bool = false;
		if (b_remove) {
			b_remove_bool = true;
		}
		bool b_prefix_bool = false;
		if (b_prefix) {
			b_prefix_bool = true;
		}

		_mp::type_v_buffer v;
		if (s_tag && dw_tag > 0 && (dw_tag % 2 == 0)) {
			std::copy(s_tag, s_tag + dw_tag, std::back_inserter(v));
		}

		ptr_device->set_ibutton_tag(b_remove_bool, b_prefix_bool,v);
		dwResult = ccb_client::const_dll_result_success;

		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
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
		_mp::type_v_buffer v = ptr_device->get_ibutton_remove_indicate_tag();
		dwResult = v.size();

		if (s_tag == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : s_tag == null : %u\n", __WFUNCTION__, v.size());
			continue;
		}

		std::copy(v.begin(), v.end(), s_tag);
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, (unsigned char)v.size());
	} while (0);

	return dwResult;
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
		_mp::type_v_buffer v;
		if (s_tag && dw_tag > 0 && (dw_tag % 2 == 0)) {
			std::copy(s_tag, s_tag + dw_tag, std::back_inserter(v));
		}

		ptr_device->set_ibutton_remove_indicate_tag(v);
		dwResult = ccb_client::const_dll_result_success;

		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
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

		if( ptr_device->get_name_by_wstring().compare(L"europa") == 0) {
			ptr_device->set_default_with_inf_is_vcom();//lpu238 - LPC1343 chip
		}
		else if (ptr_device->get_name_by_wstring().compare(L"elara") == 0) {
			ptr_device->set_default_with_inf_is_vcom();//lpu238 - MH1902T chip
		}
		else {
			 ptr_device->set_default();//lpu237
		}

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
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
		if (sVersion == NULL) {
			dwResult = cprotocol_lpu237::the_size_of_version;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : sVersion == null\n", __WFUNCTION__);
			continue;
		}

		cprotocol_lpu237::type_version ver = ptr_device->get_system_version();
		_mp::type_v_buffer v = ver.get_by_vector();
		std::copy(v.begin(), v.end(), sVersion);

		dwResult = v.size();
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : version : %ls\n", __WFUNCTION__, ver.get_by_string());
	} while (0);

	return dwResult;
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
	unsigned long dwResult(ccb_client::const_dll_result_error);

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls\n", __WFUNCTION__);
		if (sVersion == 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : sVersion == null\n", __WFUNCTION__);
			continue;
		}
		dwResult = sVersion[0];
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, dwResult);
	} while (false);
	return dwResult;
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
	unsigned long dwResult(ccb_client::const_dll_result_error);

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls\n", __WFUNCTION__);
		if (sVersion == 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : sVersion == null\n", __WFUNCTION__);
			continue;
		}
		dwResult = sVersion[1];
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, dwResult);
	} while (false);
	return dwResult;
}

/*!
* function
*	기존 api 에서 exported 되나, 설명서(lpu230_api_tools_UM_KOR_V5.0.pdf) 에서는 이 함수에 대한 설명이 없다.
* 
*	기존 코드 설명상 "stop operation of LPU237_tools_msr_update_x." 라고 되어 있으나 tg_lpu230_tools.dll 에는
* 
*	LPU237_tools_msr_update_x 라는 함수는 없다. 또한 이 dll 을 사용하는 예시 프로그램 tp_lpu237.exe(vs2019 용으로 제작, https://github.com/elpusk/example.lpu237 )
* 
*	에서도 이 함수는 사용하지 않고 있으며, wrapper class 에서 만, wrapper 하고 있음.
* 
*	따라서 함수는 있어야 하지만 기능은 수정되어야 한다. 왜냐하면, 기존 코드 주석에서 언급한 LPU237_tools_msr_update_x 함수가 아예
* 
*	존재하지 않기 때문이다.
* 
*	따라서 새롭게 주어진 기능은 ...
* 
*	_cb_get_set_parameter() 이 호출 중 일때, 성공이어서 다음 phase 를 실행할수 있어도, 그 것을 중지 시키고 cancel 을 return 하는 것으로 
*
*	기능을 정의 한다.
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
	unsigned long dwResult(ccb_client::const_dll_result_error);

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : 0x%x\n", __WFUNCTION__);
		_mp::cwait::type_ptr ptr_wait = g_map_user_cb.start_cancel();
		if (!ptr_wait) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : start cancel failure\n", __WFUNCTION__);
			continue;
		}

		int n_mm_time_out(1000); // unit mm sec time-out
		if (ptr_wait->wait_for_one_at_time() == _mp::cwait::const_event_timeout) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : timeout : %d mmsec\n", __WFUNCTION__, n_mm_time_out);
			continue;
		}
		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
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
		if (pc_offset == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : pc_offset == null\n", __WFUNCTION__);
			continue;
		}

		int n_val(-1);
		std::tie(n_val,std::ignore) = ptr_device->get_ibutton_range();

		if (n_val < 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : start index : %d\n", __WFUNCTION__, n_val);
			continue;
		}

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : start index : %d\n", __WFUNCTION__, n_val);
	} while (0);

	return dwResult;
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
		ptr_device->set_ibutton_range((int)c_offset, -1);

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : start index : %u\n", __WFUNCTION__, c_offset);
	} while (0);

	return dwResult;
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
		if (pc_offset == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : pc_offset == null\n", __WFUNCTION__);
			continue;
		}

		int n_val(-1);
		std::tie(std::ignore, n_val) = ptr_device->get_ibutton_range();

		if (n_val < 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : stop index : %d\n", __WFUNCTION__, n_val);
			continue;
		}

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : stop index : %d\n", __WFUNCTION__, n_val);
	} while (0);

	return dwResult;
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
		ptr_device->set_ibutton_range(-1,(int)c_offset);

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : stop index : %u\n", __WFUNCTION__, c_offset);
	} while (0);

	return dwResult;
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
		if (pc_support == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : pc_support == null\n", __WFUNCTION__);
			continue;
		}

		std::wstring s_name = ptr_device->get_name_by_wstring();
		cprotocol_lpu237::type_version ver = ptr_device->get_system_version();


		if (s_name.compare(L"callisto") == 0) {
			if (ver < cprotocol_lpu237::type_version(3, 23, 0, 0)) {
				*pc_support = 0;
			}
			else {
				*pc_support = 1;
			}
		}
		else if (s_name.compare(L"ganymede") == 0) {
			if (ver < cprotocol_lpu237::type_version(5, 22, 0, 0)) {
				*pc_support = 0;
			}
			else {
				*pc_support = 1;
			}
		}
		else if (s_name.compare(L"europa") == 0) {
			if (ver < cprotocol_lpu237::type_version(1, 1, 0, 0)) {
				*pc_support = 0;
			}
			else {
				*pc_support = 1;
			}
		}
		else if (s_name.compare(L"himalia") == 0) {
			*pc_support = 1;
		}
		else if (s_name.compare(L"elara") == 0) {
			*pc_support = 1;
		}
		else {
			continue;
		}

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}
