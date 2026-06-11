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
#include <tg_lpu237_ibutton.h>
#include <mp_coffee.h>

#include <manager_of_device_of_client.h>
#include <ccb_client.h>
#include <lpu237_of_client.h>
#include <cdef.h>
#include <cdll_ini.h>
#include <cmap_user_cb.h>

#define	LPU237_VID		0x134b
#define	LPU237_PID		0x0206
#define	LPU237_INF		1

class _shutdown_cleaner {

public:
	_shutdown_cleaner() :
		m_b_clean(false)
		, m_cmgmt(_mp::cclient_manager::get_instance())
	{
	}
	~_shutdown_cleaner()
	{
		if (!m_b_clean) {
			// 명시적 클린이 없으면 자동 클린 시도.
			m_cmgmt.enable_dont_release_client_in_destructor(true);
		}
	}

	void set_clean(bool b_clean = true)
	{
		m_b_clean = b_clean;
	}

private:
	bool m_b_clean;
	_mp::cclient_manager& m_cmgmt;
};

static std::shared_ptr<_shutdown_cleaner> _g_ptr_shutdown_clean;

#ifdef _WIN32
int __cdecl _DllExitHandler();


int __cdecl _DllExitHandler()
{
	_g_ptr_shutdown_clean.reset();
	return 0;
}

#else
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

/////////////////////////////////////////////////////////////////////////
// global variable
/////////////////////////////////////////////////////////////////////////
static std::atomic_bool g_b_enable_ibutton_notify(false);
static cmap_user_cb g_map_user_cb; //global user callback map
static std::atomic_ulong g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_SUCCESS;

/////////////////////////////////////////////////////////////////////////
// local class
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// local function prototype
/////////////////////////////////////////////////////////////////////////
static unsigned long _wait_key_with_callback(HANDLE hDev, long n_cb_item_index, type_key_callback pFun, void* pParameter);

/**
* @brief thread function for retrying job.
*/
static void _job_retry_start_key(
	long n_item_index
	, HANDLE h_dev
	, int n_result_index
	, _mp::casync_parameter_result::type_callback p_fun
	, void* p_para
);

/**
* @brief Callback function for LPU237Lock_wait_key_with_callback().
* 
* the user defined callback function will be called by this function.
*/
static void __stdcall _cb_key(void*p_user);

/////////////////////////////////////////////////////////////////////////
// local function body
/////////////////////////////////////////////////////////////////////////
void __stdcall _cb_key(void* p_user)
{
	do {
		//_wait_key_with_callback() 에서 _cb_key() 호출 시, n_item_index(LPU237Lock_get_data() 의 dwBufferIndex )
		long n_item_index = (long)p_user; 
		int n_result_index(-1);
		HANDLE h_dev(INVALID_HANDLE_VALUE);

		if(n_item_index < 0) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : invalid item index %d.\n", __WFUNCTION__, n_item_index);
			continue;
		}
		_mp::casync_parameter_result::type_callback p_fun(nullptr);
		void* p_para(nullptr);
		if (!g_map_user_cb.get_callback(n_item_index, false, n_result_index, h_dev, p_fun, p_para)) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : get_callback fail for item index %d.\n", __WFUNCTION__, n_item_index);
			continue;
		}

		if (g_b_enable_ibutton_notify) {
			if (!p_fun) {
				_mp::clog::get_instance().log_fmt(L" : ERR : %ls : callback function is NULL for item index %d.\n", __WFUNCTION__, n_item_index);
				continue;
			}

			manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
			if (ptr_manager_of_device_of_client) {
				_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = ptr_manager_of_device_of_client->get_async_parameter_result_for_manager_from_all_device(n_result_index);
				if (ptr_result) {
					_mp::type_v_buffer v_out_rx(0);
					if (!ptr_result->get_result(v_out_rx)) {
						if (v_out_rx.size() >= 6 && memcmp(v_out_rx.data(), "cancel", 6) == 0) {
							// 앞 6바이트가 "cancel"
							// 다른 프로세스가 ready 상태에서 close 하는 등의 이유로, 대기 중인 i-button 읽기 작업이 취소된 경우.
							// 알리지 않고, 다시 대기 작업을 시작한다. 
							std::thread retry_job(
								_job_retry_start_key
								, n_item_index
								, h_dev
								, n_result_index
								, p_fun
								, p_para
							);

							retry_job.detach(); // detach thread, so that it can run independently.

							// If the user callback is not enabled, we have to next chance automatically.
							_mp::clog::get_instance().log_fmt(L" : INF : %ls : user callback is retrying, the current reading is canceled by another process close.\n", __WFUNCTION__, n_item_index);
							continue;
						}
					}
				}
			}
			p_fun(p_para); // callback user function
			continue;
		}

		// disable ibutton notify
		std::thread retry_job(
			_job_retry_start_key
			, n_item_index
			, h_dev
			, n_result_index
			, p_fun
			, p_para
		);

		retry_job.detach(); // detach thread, so that it can run independently.
		
		// If the user callback is not enabled, we have to next chance automatically.
		_mp::clog::get_instance().log_fmt(L" : INF : %ls : user callback is not enabled, skip callback for item index %d.\n", __WFUNCTION__, n_item_index);

	} while (false);
}

void _job_retry_start_key(
	long n_item_index
	, HANDLE h_dev
	, int n_result_index
	, _mp::casync_parameter_result::type_callback p_fun
	, void* p_para
)
{
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	do {
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue; // no manager of device of client
		}
		_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = ptr_manager_of_device_of_client->get_async_parameter_result_for_manager_from_all_device(n_result_index);
		if (!ptr_result) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : INVALID_HANDLE_VALUE\n", __WFUNCTION__);
			continue;
		}
		ptr_manager_of_device_of_client->remove_async_result_for_manager(n_result_index); // current result index is removed.(ignore)
		
		// retry job
		_wait_key_with_callback(
			h_dev
			, n_item_index
			, p_fun
			, p_para
		);


	} while (false);
}

/////////////////////////////////////////////////////////////////////////
// exported function body
/////////////////////////////////////////////////////////////////////////
//
// ssDevPaths == NULL : return need size of ssDevPaths. [ unit : byte ]
// ssDevPaths != NULL : return the number of device path.
// wchar_t* ssDevPaths : [in/out] Multi string of devices
unsigned long _CALLTYPE_ LPU237Lock_get_list(wchar_t* ssDevPaths)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237Lock_get_list\n");
	//const std::wstring s_filter(L"hid#vid_134b&pid_0206&mi_01");
	_mp::type_list_wstring list_filter{ L"lpu200",L"ibutton" };
	_mp::type_list_wstring list_dev_path;
	unsigned long dw_dev(0);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		list_dev_path = ptr_manager_of_device_of_client->get_device_list(list_filter);
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
*	equal to LPU237Lock_get_list.
*  LPU237Lock_get_list' unicode version
*/
unsigned long _CALLTYPE_ LPU237Lock_get_list_w(wchar_t* ssDevPaths)
{
	if (ssDevPaths)
		return LPU237Lock_get_list(ssDevPaths);
	else {
		return (LPU237Lock_get_list(ssDevPaths));
	}
}

/*!
* function
*	equal to LPU237Lock_get_list.
*  LPU237Lock_get_list' Multi Byte Code Set version
*/
unsigned long _CALLTYPE_ LPU237Lock_get_list_a(char* ssDevPaths)
{
	if (ssDevPaths) {
		std::vector<wchar_t> vDevPaths(2048, 0);
		unsigned long dwResult = LPU237Lock_get_list(&vDevPaths[0]);

		if (dwResult > 0) {
			auto two = 0;
			for (size_t i = 0; i < vDevPaths.size(); i++) {
				ssDevPaths[i] = (char)vDevPaths[i];
				if (vDevPaths[i] == NULL) {
					two++;
					if (two == 2)
						break;	//exit for
				}
				else {
					two = 0;
				}
			}//end for
		}

		return dwResult;
	}
	else {
		return (LPU237Lock_get_list(NULL) / sizeof(wchar_t));
	}
}

HANDLE _CALLTYPE_ LPU237Lock_open(const wchar_t* sDevPath)
{
	HANDLE h_dev(INVALID_HANDLE_VALUE);
	unsigned long dwResult(0);
	bool b_need_close(false);
	unsigned long n_device_index(i_device_of_client::const_invalied_device_index);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls.\n", __WFUNCTION__);
		if (sDevPath) {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : %ls\n", __WFUNCTION__, sDevPath);
		}
		else {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : the given path null\n", __WFUNCTION__);
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error\n", __WFUNCTION__);
			continue;
		}

		std::wstring s_dev_path(sDevPath);

		// v6.4 부터
		// cf2 에서는 s_dev_path 가 "&ibutton" 로 끝나야 i-buttob 의 virtual device 를 open 하도록 설계되어 있어서
		// "&ibutton" 가 없어도 open 되는 구형 application 들과 호환성을 위해서 s_dev_path 가 "&ibutton" 로 끝나지 않으면	"&ibutton" 를 붙여서 open 하도록 한다.
		const std::wstring suffix = L"&ibutton";

		if (s_dev_path.length() < suffix.length() ){
			s_dev_path += suffix;
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : changed path : %ls\n", __WFUNCTION__, s_dev_path.c_str());
		}
		else if( s_dev_path.compare(s_dev_path.length() - suffix.length(),suffix.length(),	suffix) != 0){
			s_dev_path += suffix;
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : changed path : %ls\n", __WFUNCTION__, s_dev_path.c_str());
		}
		//

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		if (!ptr_manager_of_device_of_client->get_device(s_dev_path)->is_null_device()) {
			//alreay open.
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : already open\n", __WFUNCTION__);
			continue;
		}

		n_device_index = ptr_manager_of_device_of_client->create_device(s_dev_path,true);
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

		/////////////////////////// <<<<<
		/*
		if (!ptr_new_device->cmd_enter_config()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : cmd_enter_config.\n", __WFUNCTION__);
			continue;
		}
		_mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : success : cmd_enter_config.\n", __WFUNCTION__);

		if (!ptr_new_device->cmd_leave_config()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : cmd_leave_config.\n", __WFUNCTION__);
			continue;
		}
		_mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : success : cmd_leave_config.\n", __WFUNCTION__);
		*/
		////////////////////////// >>>>>

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

HANDLE _CALLTYPE_ LPU237Lock_open_w(const wchar_t* sDevPath)
{
	return LPU237Lock_open(sDevPath);
}

HANDLE _CALLTYPE_ LPU237Lock_open_a(const char* sDevPath)
{
	if (sDevPath) {
		std::string dev(sDevPath);
		std::wstring tdev;

		std::for_each(std::begin(dev), std::end(dev), [&](std::string::value_type c) {
			tdev.push_back(static_cast<std::wstring::value_type>(c));
			});

		return LPU237Lock_open(tdev.c_str());
	}
	else {
		return LPU237Lock_open(NULL);
	}
}

unsigned long _CALLTYPE_ LPU237Lock_close(HANDLE hDev)
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

unsigned long _CALLTYPE_ LPU237Lock_enable(HANDLE hDev)
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
		
		g_b_enable_ibutton_notify = true;
		/*
		if (!ptr_device->cmd_ibutton_enable()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_ibutton_enable\n", __WFUNCTION__);
			continue;
		}
		*/
		
		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237Lock_disable(HANDLE hDev)
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
		
		g_b_enable_ibutton_notify = false;
		/*
		if (!ptr_device->cmd_ibutton_disble()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_ibutton_disble\n", __WFUNCTION__);
			continue;
		}
		*/

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237Lock_cancel_wait_key(HANDLE hDev)
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

		if (!ptr_device->reset()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : reset.\n", __WFUNCTION__);
			continue;
		}

		dwResult = ccb_client::const_dll_result_success;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237Lock_wait_key_with_callback(HANDLE hDev, type_key_callback pFun, void* pParameter)
{
	return _wait_key_with_callback(hDev, -1, pFun, pParameter);
}

unsigned long _wait_key_with_callback(HANDLE hDev, long n_cb_item_index, type_key_callback pFun, void* pParameter)
{
	unsigned long dw_result(ccb_client::const_dll_result_error);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : 0x%x\n", __WFUNCTION__, hDev);
		if (pFun == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : pFun==NULL : error\n", __WFUNCTION__);
			continue;
		}
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (ptr_device->is_null_device()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : INVALID_HANDLE_VALUE\n", __WFUNCTION__);
			continue;
		}
		if (!ptr_device->reset()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : reset.\n", __WFUNCTION__);
			continue;
		}

		long n_item_index(-1);
		if (n_cb_item_index == -1) {
			// new case
			n_item_index = g_map_user_cb.add_callback(-1, hDev, pFun, pParameter);
			if (n_item_index < 0) {
				_mp::clog::get_instance().log_fmt(L" : RET : %ls : [critical error]add_callback error.\n", __WFUNCTION__);
				continue;
			}
		}
		else {
			//retry case
			n_item_index = n_cb_item_index;
			g_map_user_cb.change_callback(n_item_index, -1, hDev, pFun, pParameter);
		}

		int n_result_index = ptr_device->cmd_async_waits_data(_cb_key, (void*)n_item_index);
		if (n_result_index < 0) {
			g_map_user_cb.remove_callback(n_item_index);
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_async_waits_ms_card.\n", __WFUNCTION__);
			continue;
		}

		g_map_user_cb.change_result_index(n_item_index, n_result_index);

		dw_result = (unsigned long)n_item_index;

		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success - %u.\n", __WFUNCTION__, n_result_index);
	} while (0);

	return dw_result;
}

unsigned long _CALLTYPE_ LPU237Lock_get_data(unsigned long dwBufferIndex, unsigned char* sKey)
{
	unsigned long dw_client_result(ccb_client::const_dll_result_error);

	static std::map<unsigned long, _mp::type_v_buffer> map_cash;

	int n_result_index(-1);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : %d.\n", __WFUNCTION__, dwBufferIndex);

		long n_item_index = (long)dwBufferIndex;
		HANDLE hDev(INVALID_HANDLE_VALUE);
		_mp::casync_parameter_result::type_callback p_fun(nullptr);
		void* p_para(nullptr);

		auto it = map_cash.find(dwBufferIndex);
		if(it != map_cash.end()) {
			// found in cash
			dw_client_result = (unsigned long)it->second.size();

			if (sKey) {
				std::copy(it->second.begin(), it->second.end(), sKey);
				map_cash.erase(it); // remove from cash
			}
			
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : from cash - %u.\n", __WFUNCTION__, dw_client_result);
			continue;
		}

		if (!g_map_user_cb.get_callback(n_item_index, true, n_result_index, hDev, p_fun, p_para)) {
			g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_INVALID_ITEM_INDEX;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : invalid item index .\n", __WFUNCTION__);
			continue;
		}

		if (!ptr_manager_of_device_of_client) {
			g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_NONE_DEVICE_CLIENT;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = ptr_manager_of_device_of_client->get_async_parameter_result_for_manager_from_all_device(n_result_index);
		if (!ptr_result) {
			// reading ready  에서  그냥 close 하면 여기.
			g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_NONE_DEVICE_CLIENT_RESULT_OBJECT;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : INVALID_HANDLE_VALUE\n", __WFUNCTION__);
			continue;
		}

		_mp::type_v_buffer v_out_rx(0);
		const size_t n_ibutton_key(8);

		if (!ptr_result->get_result(v_out_rx)) {
			ptr_manager_of_device_of_client->remove_async_result_for_manager(n_result_index);
			dw_client_result = ccb_client::const_dll_result_error;

			if(v_out_rx.size() == 0) {
				// size 0이면 그냥 error
				g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_FAILED_NONE_RESPONSE_DATA;
				_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : LPU237LOCK_DLL_GET_DATA_LAST_ERROR_FAILED_NONE_RESPONSE_DATA\n", __WFUNCTION__);
			}
			else if (v_out_rx.size() >= 6 && memcmp(v_out_rx.data(), "cancel", 6) == 0){
				// 앞 6바이트가 "cancel"
				g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_FAILED_CANCEL_STRING;
				_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : LPU237LOCK_DLL_GET_DATA_LAST_ERROR_FAILED_CANCEL_STRING\n", __WFUNCTION__);
			}
			else if (v_out_rx.size() >= 5 && memcmp(v_out_rx.data(), "error", 5) == 0) {
				g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_FAILED_ERROR_STRING;
				_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : LPU237LOCK_DLL_GET_DATA_LAST_ERROR_FAILED_ERROR_STRING\n", __WFUNCTION__);
			}
			else {
				g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_FAILED_ANY_STRING;
				_mp::clog::get_instance().log_fmt(L" : RET : %ls : error : LPU237LOCK_DLL_GET_DATA_LAST_ERROR_FAILED_ANY_STRING\n", __WFUNCTION__);
			}
			continue;
		}
		
		if (v_out_rx.size() < 3 + n_ibutton_key) {
			ptr_manager_of_device_of_client->remove_async_result_for_manager(n_result_index);
			dw_client_result = ccb_client::const_dll_result_error;
			g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_SUCCESS_LESS_THAN_3_PLUS_8;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error(size = %u)\n", __WFUNCTION__, v_out_rx.size());
			continue;
		}
		ptr_result->get_result(dw_client_result);
		// 데이터를 넘기므로 저장된 데이터 삭제.
		ptr_manager_of_device_of_client->remove_async_result_for_manager(n_result_index);

		switch (dw_client_result) {
		case ccb_client::const_dll_result_cancel:
			g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_SUCCESS_CANCEL_CODE;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : canceled\n", __WFUNCTION__);
			continue;
		case ccb_client::const_dll_result_error:
			g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_SUCCESS_ERROR_CODE;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error\n", __WFUNCTION__);
			continue;
		default:
			if (sKey) {
				std::copy(v_out_rx.begin() + 3, v_out_rx.begin() + 3 + n_ibutton_key, sKey);
			}
			else{
				// save to cash
				map_cash[dwBufferIndex] = _mp::type_v_buffer(v_out_rx.begin() + 3, v_out_rx.begin() + 3 + n_ibutton_key);
			}
			dw_client_result = n_ibutton_key;
			g_get_data_last_error = LPU237LOCK_DLL_GET_DATA_LAST_ERROR_SUCCESS;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, dw_client_result);
			continue;
		}//end switch
	} while (0);

	return dw_client_result;
}

unsigned long _CALLTYPE_ LPU237Lock_dll_on()
{
	cdll_ini& cini(cdll_ini::get_instance());

#ifndef _WIN32
	std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
	std::string s_pipe_name_of_trace(_mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME);

	bool b_ini = cini.load_definition_file(_mp::ccoffee_path::get_path_of_coffee_lpu237_ibutton_ini_file());
	
	//setup tracing system
	_mp::clog& log(_mp::clog::get_instance());
	log.enable_trace(s_pipe_name_of_trace, false); //enable trace by client mode

	//setup logging system
	log.config(s_log_root_folder_except_backslash, 6, std::wstring(L"coffee_manager"), std::wstring(L"tg_lpu237_ibutton"), std::wstring(L"tg_lpu237_ibutton"));
	log.remove_log_files_older_then_now_day(cini.get_log_days_to_keep());
	log.enable(cini.get_log_enable());

	log.log_fmt(L"[I] START tg_lpu237_ibutton so or dll.\n");
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
#ifdef _WIN32
		//_DllExitHandler 을 마지막으로 등록해서,
		// 이 dll 의 _execute_onexit_table() 가 실행될때  가장 먼저 호출되도록 한다.(LIFO 에 handler 가 저장, 호출됨) 
		// 디버거로 호출 확인됨.
		if (!_g_ptr_shutdown_clean) {
			_g_ptr_shutdown_clean = std::make_shared<_shutdown_cleaner>();
			_onexit(_DllExitHandler);
		}

		// LPU237_ibutton_off() 가 명시적으로 호출되지 않으면
		// 자동 clean 동작을 하도록 설정 한다.
		_g_ptr_shutdown_clean->set_clean(false);
#endif
		dwResult = ccb_client::const_dll_result_success;
	} while (false);

	_mp::clog::get_instance().log_fmt(L" : RET : %ls.\n", __WFUNCTION__);
	return dwResult;
}

unsigned long _CALLTYPE_ LPU237Lock_dll_off()
{
	unsigned long dwResult(ccb_client::const_dll_result_error);
	_mp::clog::get_instance().log_fmt(L" : CAL : %ls.\n", __WFUNCTION__);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
#ifdef _WIN32
		if (_g_ptr_shutdown_clean) {
			// LPU237_tools_off() 가 명시적으로 호출면,  자동 clean 동작을 억제
			_g_ptr_shutdown_clean->set_clean();
		}
#endif
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

unsigned long _CALLTYPE_ LPU237Lock_get_id(HANDLE hDev, unsigned char* sId)
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

unsigned long _CALLTYPE_ LPU237Lock_get_data_last_error(unsigned long dwRFU)
{
	unsigned long dwResult(g_get_data_last_error);
	_mp::clog::get_instance().log_fmt(L" : CAL : %ls\n", __WFUNCTION__);
	_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, dwResult);
	return dwResult;
}