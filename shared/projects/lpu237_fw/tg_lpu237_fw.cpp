#include <websocket/mp_win_nt.h>

#include <mp_os_type.h>

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
#include <filesystem>

#include <mp_clog.h>
#include <mp_cconvert.h>
#include <cprotocol_lpu237.h>
#include <tg_lpu237_fw.h>
#include <mp_coffee.h>
#include <mp_coffee_path.h>

#include <manager_of_device_of_client.h>
#include <ccb_client.h>
#include <lpu237_of_client.h>
#include <cdef.h>
#include <cdll_ini.h>
#include <cmap_user_cb.h>
#include <tg_rom.h>

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

static std::pair<bool, std::wstring> _setup_rom_dll();
static std::filesystem::path _get_module_directory();

/////////////////////////////////////////////////////////////////////////
// global variable
/////////////////////////////////////////////////////////////////////////
static cmap_user_cb g_map_user_cb; //global user callback map

static std::shared_ptr<CRom> g_ptr_rom_dll; // tg_rom dll warpper

/////////////////////////////////////////////////////////////////////////
// local class
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// local function prototype
/////////////////////////////////////////////////////////////////////////

/**
* @brief initialze g_ptr_rom_dll variable
* @return first - ini OK tg_rom.dll(so), second - debugging message( always have a message )
*/
std::pair<bool,std::wstring> _setup_rom_dll()
{
	bool b_result(false);
	std::wstring s_deb(L"none message")
		;
	do {
		if (g_ptr_rom_dll) {
			if (g_ptr_rom_dll->is_ini()) {
				s_deb = L"tg_dll initialization was tried already successfully";
			}
			else {
				s_deb = L"tg_dll initialization was tried already and failed";
			}
			continue;
		}

		std::error_code ec;
		// 1. 현재 tg_lpu237_fw.dll(libtg_lpu237_fw.so) 가 있는 디렉토리에서 tg_rom 을 찾아 본다.
		std::filesystem::path rom_lib = _get_module_directory();
#ifdef _WIN32
		rom_lib /= "tg_rom.dll";
#else
		rom_lib /= "libtg_rom.so";
#endif
		//
		if (std::filesystem::exists(rom_lib, ec) && std::filesystem::is_regular_file(rom_lib, ec)) {
			g_ptr_rom_dll = std::make_shared<CRom>(rom_lib.wstring().c_str());
			if (g_ptr_rom_dll) {
				if (g_ptr_rom_dll->is_ini()) {
					b_result = true;
					_mp::clog::get_instance().log_fmt(L" : INF :  loaded rg_rom : %ls.\n", rom_lib.wstring().c_str());
					continue;
				}
			}
		}

		//2. 현재 디렉토리에서 라이브러리 못찾으면, 기본path 에서 찾기.
		g_ptr_rom_dll.reset();
		rom_lib = _mp::ccoffee_path::get_abs_full_path_of_rom_dll();
		g_ptr_rom_dll = std::make_shared<CRom>(rom_lib.wstring().c_str());
		if (!g_ptr_rom_dll) {
			s_deb = rom_lib.wstring() + L" is not constructed";
			continue;
		}
		if (!g_ptr_rom_dll->is_ini()) {
			s_deb = rom_lib.wstring() + L" is not initialized";
			continue;
		}

		_mp::clog::get_instance().log_fmt(L" : INF :  loaded rg_rom : %ls.\n", rom_lib.wstring().c_str());

		b_result = true; // 현재 디렉토리에 있는 dll 초기화 성공.

	} while (false);
	return std::make_pair(b_result,s_deb);
}

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

static unsigned long _fw_msr_update_ex_w(const unsigned char* sId,
	unsigned long dwWaitTime
	, type_lpu237_fw_callback cbUpdate
	, void* pUser
	, HWND hWnd
	, UINT uMsg
	, const wchar_t* sRomFileName
	, unsigned long dwFwIndex
);


/**
* @brief Callback function for _fw_msr_update_ex_w().
* 
* the user defined callback function will be called by this function.
*/
static void __stdcall _cb_fw(void*p_user);

/**
* @brief 수신된 _mp::type_v_wstring 값으로 부터 펌웨어 업데이트 진행 상황을 얻는 함수.
* @param v_wstring_result - _mp::type_v_wstring 형식의 펌웨어 업데이트 진행 상황 정보.
* @return tuple of (b_result, s_result, n_fw_step_cur, n_fw_step_max, s_deb)
* 
* b_result : 펌웨어 업데이트 진행 상황 정보가 정상적으로 수신되었는지 여부. (true - 정상, false - 비정상)
* 
* s_result : 펌웨어 업데이트 진행 상황 정보에서 얻은 single string 결과. (예를 들어, "success" 또는 "error" 등)
* 
* n_fw_step_cur : 펌웨어 업데이트 진행 상황 정보에서 얻은 현재 펌웨어 업데이트 단계. (0 ~ n_fw_step_max-1)
* 
* n_fw_step_max : 펌웨어 업데이트 진행 상황 정보에서 얻은 총 펌웨어 업데이트 단계.
* 
* s_message : lpu230_update 가 보낸 펌웨어 업데이트 진행 정보
*/
static std::tuple<bool, std::wstring, long, long, std::wstring>_get_fw_update_progress(
	const _mp::type_v_wstring& v_wstring_result
);

static std::pair<unsigned long, std::wstring> _get_wparam(
	int n_item_index
	, unsigned long dw_client_result
	, int n_fw_step_cur
	, int n_fw_step_max
	, const std::wstring& s_message
);

static std::pair<unsigned long, std::wstring> _get_old_wparam_from_fw_update_message(int n_item_index, const std::wstring& s_message);

static std::pair<unsigned long, std::wstring> _get_v6_process_result_code_from_fw_update_message(int n_item_index, const std::wstring& s_message);

/////////////////////////////////////////////////////////////////////////
// local function body
/////////////////////////////////////////////////////////////////////////

std::pair<unsigned long, std::wstring> _get_wparam(
	int n_item_index
	, unsigned long dw_client_result
	, int n_fw_step_cur
	, int n_fw_step_max
	, const std::wstring& s_message
)
{
	unsigned long wparam = 0;
	unsigned long l_msg = 0;
	std::wstring s_param = L"none";

	// by lpu230_fw_api_UM_KOR_V5.0.pdf
	// 정상 처리의 경우 LPU237 은 LPU237_FW_WPARAM_FOUND_BL 한 번
	// , LPU237_FW_WPARAM_SECTOR_ERASE 한 번
	// , LPU237_FW_WPARAM_SECTOR_WRITE 여섯번
	// , LPU237_FW_WPARAM_COMPLETE 한 번이 전달 된다.
	// lpu230_fw_api_UM_KOR_V5.0.pdf 에 있는 위 문장은 LPC1343 마이컴만을 기준으로 한 것으로.
	// 업데이트 프로그램이 lpu230_update 로 독립해서 완전히 변경되었음.
	// 따라서 위 문자에 따라 프로그래밍한 application 은 문제가 생길 수 있다. 
	// 호환성을 위한 "뭔가"가 필여하다.
	if (g_map_user_cb.get_mode() == 6) {
		LPU237_FW_SET_TOTAL_STEP_TO_WPARAM(wparam, n_fw_step_max);
		LPU237_FW_SET_STEP_INDEX_TO_WPARAM(wparam, n_fw_step_cur);

		switch (dw_client_result) {
		case LPU237_FW_RESULT_SUCCESS:
			if (n_fw_step_max > 0 && (n_fw_step_cur >= (n_fw_step_max - 1))) {
				LPU237_FW_SET_PROCESS_RESULT_TO_WPARAM(wparam, LPU237_FW_WPARAM_EX_COMPLETE);
				s_param = L"LPU237_FW_WPARAM_EX_COMPLETE";
			}
			else {
				std::tie(l_msg, s_param) = _get_v6_process_result_code_from_fw_update_message(n_item_index, s_message);
				LPU237_FW_SET_PROCESS_RESULT_TO_WPARAM(wparam, l_msg);
			}
			break;
		case LPU237_FW_RESULT_CANCEL:
			LPU237_FW_SET_PROCESS_RESULT_TO_WPARAM(wparam, LPU237_FW_WPARAM_EX_ERROR);
			s_param = L"LPU237_FW_WPARAM_EX_ERROR";
			break;
		case LPU237_FW_RESULT_ERROR:
		default:
			LPU237_FW_SET_PROCESS_RESULT_TO_WPARAM(wparam, LPU237_FW_WPARAM_EX_ERROR);
			s_param = L"LPU237_FW_WPARAM_EX_ERROR";
			break;
		}//end switch
	}
	else {
		// 기존 방식 유지.
		switch (dw_client_result) {
		case LPU237_FW_RESULT_SUCCESS:
			if (n_fw_step_max > 0 && (n_fw_step_cur >= (n_fw_step_max - 1))) {
				wparam = LPU237_FW_WPARAM_COMPLETE;
				s_param = L"LPU237_FW_WPARAM_COMPLETE";
			}
			else {
				std::tie(wparam, s_param) = _get_old_wparam_from_fw_update_message(n_item_index, s_message);
				if (LPU237_FW_WPARAM_SECTOR_ERASE == wparam) {
					if (std::get<0>(g_map_user_cb.get_msg_counter(n_item_index)) > 1) {
						// erase sector 메시지가 2번 이상 전달되는 경우, 호환성을 위해 wparam 을 LPU237_FW_WPARAM_IGNORE 로 변경.
						wparam = _LPU237_FW_WPARAM_IGNORE;
					}
				}
				else if (LPU237_FW_WPARAM_SECTOR_WRITE == wparam) {
					if (std::get<1>(g_map_user_cb.get_msg_counter(n_item_index)) > 7) {
						// erase sector 메시지가 8번 이상 전달되는 경우, 호환성을 위해 wparam 을 LPU237_FW_WPARAM_IGNORE 로 변경.
						wparam = _LPU237_FW_WPARAM_IGNORE;
					}
				}
			}
			break;
		case LPU237_FW_RESULT_CANCEL:
			wparam = LPU237_FW_WPARAM_ERROR;
			s_param = L"LPU237_FW_WPARAM_ERROR";
			break;
		case LPU237_FW_RESULT_ERROR:
		default:
			wparam = LPU237_FW_WPARAM_ERROR;
			s_param = L"LPU237_FW_WPARAM_ERROR";
			break;
		}//end switch

	}

	return std::make_pair(wparam, s_param);
}

std::pair<unsigned long, std::wstring> _get_old_wparam_from_fw_update_message(int n_item_index, const std::wstring& s_message)
{
	// 메시지 step 이 전혀 맞질 않아서, 적절하게 조정함.
	// 

	unsigned long wparam = _LPU237_FW_WPARAM_IGNORE;
	std::wstring s_param = L"_LPU237_FW_WPARAM_IGNORE";
	
	if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_SETUP_BOOTLOADER_DOT) != std::wstring::npos) {
		wparam = LPU237_FW_WPARAM_FOUND_BL;
		s_param = L"LPU237_FW_WPARAM_FOUND_BL";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_ERASE_SECTOR_SPACE) != std::wstring::npos) {
		g_map_user_cb.inc_msg_counter(n_item_index, 1, 0, 0);
		wparam = LPU237_FW_WPARAM_SECTOR_ERASE;
		s_param = L"LPU237_FW_WPARAM_SECTOR_ERASE";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_WRITE_AND_VERIFY_SECTOR_SPACE) != std::wstring::npos) {
		g_map_user_cb.inc_msg_counter(n_item_index, 0, 1, 0);
		wparam = LPU237_FW_WPARAM_SECTOR_WRITE;
		s_param = L"LPU237_FW_WPARAM_SECTOR_WRITE";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_RECOVERING_SYSTEM_PARAMETERS) != std::wstring::npos) {
		g_map_user_cb.inc_msg_counter(n_item_index, 0, 1, 0);
		wparam = LPU237_FW_WPARAM_SECTOR_WRITE;
		s_param = L"LPU237_FW_WPARAM_SECTOR_WRITE";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_FIRMWARE_UPDATE_COMPLETE_DOT_SPACE) != std::wstring::npos) {
		g_map_user_cb.inc_msg_counter(n_item_index, 0, 0, 1);
		wparam = LPU237_FW_WPARAM_SECTOR_WRITE;
		s_param = L"LPU237_FW_WPARAM_SECTOR_WRITE";
	}
	else {
		// 나머지는 호환성을 유지하기 위해, 콜백이나 postmessage 가 동작하지 않도록 _LPU237_FW_WPARAM_IGNORE 으로 처리.
	}

	return std::make_pair(wparam,s_param);
}

std::pair<unsigned long, std::wstring> _get_v6_process_result_code_from_fw_update_message(int n_item_index, const std::wstring& s_message)
{
	// 메시지 step 이 전혀 맞질 않아서, 적절하게 조정함.
	// 

	unsigned long n_result_code_of_wparam = 0;
	std::wstring s_result_code_of_wparam;

	if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_BACKUP_SYSTEM_PARAMETERS) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_BACKUP_PARAMETERS;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_BACKUP_PARAMETERS";
	}
	else if(s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_RUN_BOOTLOADER_DOT) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_RUN_BOOTLOADER;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_RUN_BOOTLOADER";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_DETECT_PLUGOUT_LPU23X_DOT) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_DETECT_PLUGOUT_LPU23X;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_DETECT_PLUGOUT_LPU23X";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_DETECT_PLUGOUT_LPU23X_AND_PLUGIN_BOOTLOADER_DOT) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_DETECT_PLUGIN_BOOTLOADER;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_DETECT_PLUGIN_BOOTLOADER";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_DETECT_PLUGIN_BOOTLOADER_DOT) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_DETECT_PLUGIN_BOOTLOADER;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_DETECT_PLUGIN_BOOTLOADER";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_SETUP_BOOTLOADER_DOT) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_SETUP_BOOTLOADER;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_SETUP_BOOTLOADER";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_ERASE_SECTOR_SPACE) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_SECTOR_ERASE;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_SECTOR_ERASE";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_READ_SECTOR_SPACE) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_READ_SECTOR;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_READ_SECTOR";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_WRITE_AND_VERIFY_SECTOR_SPACE) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_SECTOR_WRITE;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_SECTOR_WRITE";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_RUN_APPLICATION_DOT) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_RUN_APP;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_RUN_APP";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_DETECT_PLUGOUT_BOOTLOADER_DOT) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_DETECT_PLUGOUT_BOOTLOADER;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_DETECT_PLUGOUT_BOOTLOADER";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_DETECTED_PLUGIN_LPU23X_DOT) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_DETECT_PLUGIN_LPU23X;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_DETECT_PLUGIN_LPU23X";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_NO_RECOVERY_OF_SYSTEM_PARAMETERS_IS_NEEDED_DOT) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_MORE_PROCESS;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_MORE_PROCESS-MSG_FW_UPDATE_NO_RECOVERY_OF_SYSTEM_PARAMETERS_IS_NEEDED";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_RECOVERING_SYSTEM_PARAMETERS) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_RECOVER_PARAMETERS;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_RECOVER_PARAMETERS";
	}
	else if (s_message.find(_mp::_coffee::CONST_S_MSG_FW_UPDATE_FIRMWARE_UPDATE_COMPLETE_DOT_SPACE) != std::wstring::npos) {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_COMPLETE;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_COMPLETE";
	}
	else {
		n_result_code_of_wparam = LPU237_FW_WPARAM_EX_MORE_PROCESS;
		s_result_code_of_wparam = L"LPU237_FW_WPARAM_EX_MORE_PROCESS-UNDEFINED";
	}




	return std::make_pair(n_result_code_of_wparam, s_result_code_of_wparam);
}

std::tuple<bool, std::wstring, long, long, std::wstring>_get_fw_update_progress(
	const _mp::type_v_wstring& v_wstring_result
)
{
	bool b_result(false);
	std::wstring s_result;
	long n_fw_step_cur(-1), n_fw_step_max(-1);
	std::wstring s_message;
	do {
		if (v_wstring_result.empty()) {
			continue;
		}
		s_result = v_wstring_result[0];
		if (s_result.compare(L"success") != 0 && s_result.compare(L"error") != 0 && s_result.compare(L"cancel") != 0) {
			continue;
		}

		if (v_wstring_result.size() >= 3) {
			n_fw_step_cur = wcstol(v_wstring_result[1].c_str(), nullptr, 10);
			n_fw_step_max = wcstol(v_wstring_result[2].c_str(), nullptr, 10);
			if (n_fw_step_cur < 0 || n_fw_step_max <= 0 || n_fw_step_cur > n_fw_step_max) {
				n_fw_step_cur = -1;
				n_fw_step_max = -1;
			}
		}
		if (v_wstring_result.size() >= 4) {
			s_message = v_wstring_result[3];
		}
		b_result = true;
	} while (false);
	return std::make_tuple(b_result, s_result, n_fw_step_cur, n_fw_step_max, s_message);
}

/**
* @brief ccb_client::_read() 에서 last_action 이 capi_client::act_sub_bootloader 인 경우, 호출되는 콜백 함수.
* 
* 결과는 b_result(bool), n_result_code(unsigned long), std::vector<std::wstring> 형식으로 주어진다.
*/
void __stdcall _cb_fw(void* p_user)
{
	static std::mutex mutex_for_cb_fw;
	std::lock_guard<std::mutex> lock(mutex_for_cb_fw);

	unsigned long n_device_index(i_device_of_client::const_invalied_device_index);
	int n_result_index(-1);
	long n_item_index = (long)p_user; // item index of callback function in g_map_user_cb
	
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	_mp::casync_parameter_result::type_ptr_ct_async_parameter_result ptr_result;

	long n_fw_step_cur(-1), n_fw_step_max(-1);
	_mp::type_v_wstring v_out_wstring_result;

	_mp::cwait::type_ptr ptr_evt_complete;
	bool b_sync(false); // true 이면, g_map_user_cb 에서 콜백 item 정상적으로 얻고, 콜백 item 은 동기식 구성임.
	bool b_set_sync_event(false); // 이 값이 true 이면, sync_event 를 set 해야 함.
	unsigned long n_sync_result(LPU237_FW_RESULT_ERROR);
	bool b_remove_callback_item(true); // g_map_user_cb 에서 n_item_index 을 제거하라.

	do {
		bool b_get(false);
		_mp::type_v_buffer v_dev_id(0);
		type_lpu237_fw_callback p_fun(NULL);
		void* p_para(NULL);
		HWND h_win(NULL);
		UINT n_msg(0);
		std::wstring s_rom_file_name;
		unsigned long dw_fw_index(-1);
		std::shared_ptr<std::mutex> ptr_mutex;

		// this is dregon!
		// _fw_msr_update_ex_w() 가 change_result_index() 에 도달하기 전에 이 callback 이 호출되어서 get_callback() 을 호츨하여,
		// 아직 설정되지 않은 n_result_index 값을 얻는을 수 있다,
		// 따라서
		// 먼저 mutex 만 얻고, lock() 시도해서 걸릴때 까지, 기다린다. 
		// _fw_msr_update_ex_w() 는 change_result_index() 실행 후, unlock() 하므로.
		// 여기서 lock() 이 되면 change_result_index()가 실행되었다는 의미 이므로,
		// 다시 get_callback() 을 호츨하여, 업데이트된 최종 데이터를 얻는다.
		std::tie(b_get, ptr_evt_complete, ptr_mutex) = g_map_user_cb.get_callback(
			n_item_index
			, false // 계속 콜백 가능성이	있기 때문에 콜백 정보를 얻은 후에도 콜백 정보를 유지한다. (예를 들어, 콜백이 여러번 호출되는 경우)
			, n_result_index
			, v_dev_id
			, p_fun
			, p_para
			, h_win
			, n_msg
			, s_rom_file_name
			, dw_fw_index
		);

		if (!b_get) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : get_callback fail for item index %d.\n", __WFUNCTION__, n_item_index);
			continue;
		}

		ptr_mutex->lock();
		std::tie(b_get, ptr_evt_complete, ptr_mutex) = g_map_user_cb.get_callback(
			n_item_index
			, false // 계속 콜백 가능성이	있기 때문에 콜백 정보를 얻은 후에도 콜백 정보를 유지한다. (예를 들어, 콜백이 여러번 호출되는 경우)
			, n_result_index
			, v_dev_id
			, p_fun
			, p_para
			, h_win
			, n_msg
			, s_rom_file_name
			, dw_fw_index
		);
		ptr_mutex->unlock();

		///////////////////////////////////////////////////////////////////
		if (p_fun == 0 && (h_win == INVALID_HANDLE_VALUE || h_win == 0)) {
			b_sync = true;
		}

		if (!ptr_manager_of_device_of_client) {
			if (b_sync) {
				b_set_sync_event = true;	n_sync_result = LPU237_FW_RESULT_ERROR;
				b_remove_callback_item = false;
			}
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}
		
		//
		
		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(v_dev_id);
		
		if (ptr_device) {
			n_device_index = ptr_device->get_device_index();
		}

		_mp::clog::get_instance().log_fmt(L" : INF :  %ls : n_device_index = %u.\n", __WFUNCTION__, n_device_index);

		ptr_result = ptr_manager_of_device_of_client->get_async_parameter_result_for_manager_from_all_device(n_result_index);
		if (!ptr_result) {
			if (b_sync) {
				b_set_sync_event = true;	n_sync_result = LPU237_FW_RESULT_ERROR;
				b_remove_callback_item = false;
			}
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : INVALID_HANDLE_VALUE : n_result_index = %d.\n", __WFUNCTION__, n_result_index);
			continue;
		}

		if (!ptr_result->get_result(v_out_wstring_result)) {
			if (b_sync) {
				b_set_sync_event = true;	n_sync_result = LPU237_FW_RESULT_ERROR;
				b_remove_callback_item = false;
			}
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : get_result() - error\n", __WFUNCTION__);
			continue;
		}

		bool b_parse(false);
		std::wstring s_result, s_message;
		std::tie(b_parse, s_result, n_fw_step_cur, n_fw_step_max, s_message) = _get_fw_update_progress(v_out_wstring_result);

		if (!b_parse) {
			if (b_sync) {
				b_set_sync_event = true;	n_sync_result = LPU237_FW_RESULT_ERROR;
				b_remove_callback_item = false;
			}
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : item index = %d : result invalid format.\n", __WFUNCTION__, n_item_index);
			continue;// g_map_user_cb 에서 n_item_index 을 제거
		}
		if (s_result.compare(L"success") != 0) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : item index = %d : process result = %ls.\n", __WFUNCTION__, n_item_index, v_out_wstring_result[0]);
			if (b_sync) {
				b_set_sync_event = true;	n_sync_result = LPU237_FW_RESULT_ERROR;
				b_remove_callback_item = false;
			}
			continue;// g_map_user_cb 에서 n_item_index 을 제거
		}

		b_remove_callback_item = false; // 재사용을 위해서 콜백 정보를 유지.

		if (v_out_wstring_result.size() == 1) {
			// 첫 "start" 에 대한 응답 이므로, n_fw_step_max 이런 애들 없음.
			// 콜백도 필요 없음.
			if (b_sync) {
				b_set_sync_event = false;
			}
			continue;// 동기 비동기에 무관하게 n_item_index 유지.
		}

		if (b_parse && n_fw_step_max > 0) {
			if (n_fw_step_cur >= (n_fw_step_max - 1)) {
				// 모두 정상으로 끝남.
				// 주의. (n_fw_step_max-1) == n_fw_step_cur 인 message 가 두번 오는데, 마지막 것은 무시해도 됨.
				// 끝난 것을 한 번 더 다름 메시지와 함께 보냄.
				// 이제 result index 를 제거해야 한다
				if (b_sync) {
					b_set_sync_event = true;
				}
				else {
					b_remove_callback_item = true;
				}
			}
		}

		unsigned long dw_client_result(LPU237_FW_RESULT_ERROR);
		std::wstring s_w_param;

		// ptr_result->get_result(v_out_wstring_result) 의 return 이 true 라면,
		// 아래 ptr_result->get_result 도 true 이므로 체크불요.
		ptr_result->get_result(dw_client_result);
		//dw_client_result 값을 재조정.
		switch (dw_client_result) {
		case LPU237_FW_RESULT_SUCCESS:
		case LPU237_FW_RESULT_CANCEL:
		case LPU237_FW_RESULT_ERROR:
			break;
		default:
			dw_client_result = LPU237_FW_RESULT_ERROR;
			break;
		}//end switch
		
		if (b_sync) {
			n_sync_result = dw_client_result;
		}

		unsigned long dw_w_param(_LPU237_FW_WPARAM_IGNORE);

		std::tie(dw_w_param, s_w_param) = _get_wparam(n_item_index, dw_client_result, n_fw_step_cur, n_fw_step_max, s_message);

		if (dw_w_param == _LPU237_FW_WPARAM_IGNORE) {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : item index = %d : ignore callback for this message : wparam = %ls.\n", __WFUNCTION__, n_item_index, s_w_param.c_str());
			continue;
		}
		if (p_fun) {
			// 1'st - user defined data.
			// 2'nd - current processing result : LPU237_FW_RESULT_x
			// 3'th - LPU237_FW_WPARAM_x.

			p_fun(p_para, dw_client_result, dw_w_param);
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : callbacked(item index=%d) : wparam = %ls.\n", __WFUNCTION__, n_item_index, s_w_param.c_str());
		}
#ifdef _WIN32
		else if (h_win != 0 && h_win != INVALID_HANDLE_VALUE) {
			//Windows system only
			//By lpu230_fw_api_UM_KOR_V5.0.pdf
			PostMessage(h_win, n_msg, (WPARAM)dw_w_param, (LPARAM)dw_client_result);
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : messaged(item index=%d) : wparam = %ls.\n", __WFUNCTION__, n_item_index, s_w_param.c_str());
		}
#endif

	} while (false);

	do {
		///////////////////////////
		// log for debugging
		if (v_out_wstring_result.size() > 3) {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : n_item_index =  %d : %ls : (cur,max) = (%d,%d) : %ls.\n"
				, __WFUNCTION__
				, n_item_index
				, v_out_wstring_result[0].c_str()
				, n_fw_step_cur
				, n_fw_step_max
				, v_out_wstring_result[3].c_str()
			);
		}
		else if (v_out_wstring_result.size() > 2) {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : n_item_index =  %d : %ls : (cur,max) = (%d,%d).\n"
				, __WFUNCTION__
				, n_item_index
				, v_out_wstring_result[0].c_str()
				, n_fw_step_cur
				, n_fw_step_max
			);
		}
	}while(false);

	// 여기는 동기식으로 최종끝난는지, -> ptr_evt_complete->set(), g_map_user_cb 에서 n_item_index 제거 금지.
	// 동기식이지만 아직 끝나지 않았는지, -> g_map_user_cb 에서 n_item_index 제거 금지.
	// 비동기식으로 끝났는지, -> g_map_user_cb 에서 n_item_index 제거.
	// 비동기식으로 계속 처리가 필요 한지 -> g_map_user_cb 에서 n_item_index 제거 금지.
	// 여부에 따라 처리 할 것.

	if (b_set_sync_event) {
		// 동기식 이므로 n_item_index 을 여기서 제거하지 말고, 
		// _fw_msr_update_ex_w() 에서 결과를 얻은 후, _fw_msr_update_ex_w() 에서 제거.

		if (ptr_manager_of_device_of_client) {
			ptr_manager_of_device_of_client->remove_async_result_for_manager(n_device_index, n_result_index);
			ptr_result.reset(); // release result
			ptr_manager_of_device_of_client.reset();
		}

		g_map_user_cb.set_sync_result(n_item_index,n_sync_result);
		ptr_evt_complete->set();// 동기식으로 기다리는 _fw_msr_update_ex_w() 에게 알림.
	}
	if (b_remove_callback_item) {// 콜백을 제거한다.
		g_map_user_cb.remove_callback(n_item_index); // remove invalid callback

		if (ptr_manager_of_device_of_client) {
			ptr_manager_of_device_of_client->remove_async_result_for_manager(n_device_index, n_result_index);
			ptr_result.reset(); // release result
		}
	}

}

unsigned long _fw_msr_update_ex_w(
	const unsigned char* sId,
	unsigned long dwWaitTime
	, type_lpu237_fw_callback cbUpdate
	, void* pUser
	, HWND hWnd
	, UINT uMsg
	, const wchar_t* sRomFileName
	, unsigned long dwFwIndex
)
{
	unsigned long dw_result(ccb_client::const_dll_result_error);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		if (sId == NULL) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : ID is NULL.\n", __WFUNCTION__);
			continue;
		}
		std::error_code ec;
		std::filesystem::path abs_path_rom_file = std::filesystem::absolute(sRomFileName, ec);
		if (ec) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : change abs path. - %ls.\n", __WFUNCTION__, sRomFileName);
			continue;
		}
		if (!std::filesystem::exists(abs_path_rom_file) || !std::filesystem::is_regular_file(abs_path_rom_file)) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : ROM file not found - %ls.\n", __WFUNCTION__, sRomFileName);
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

		// set bootloader parameters
		std::wstring s_name = ptr_device->get_name_by_wstring();
		if (!ptr_device->bootloader_operation(L"set",L"model_name", s_name)) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : error bootloader_operation for model_name.\n", __WFUNCTION__);
			continue;
		}
		cprotocol_lpu237::type_version version = ptr_device->get_system_version();
		std::wstring s_version = version.get_by_string();
		if (!ptr_device->bootloader_operation(L"set", L"system_version", s_version)) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : error bootloader_operation for system_version.\n", __WFUNCTION__);
			continue;
		}
		if (!ptr_device->bootloader_operation(L"set", L"_cf_bl_progress_", L"true")) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : error bootloader_operation for _cf_bl_progress_.\n", __WFUNCTION__);
			continue;
		}

#if defined(_WIN32) && defined(_DEBUG)
		if (!ptr_device->bootloader_operation(L"set", L"_cf_bl_window_", L"true")) {// for debugging - true, must be false in release 
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : error bootloader_operation for _cf_bl_window_.\n", __WFUNCTION__);
			continue;
		}
#else
		if (!ptr_device->bootloader_operation(L"set", L"_cf_bl_window_", L"false")) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : error bootloader_operation for _cf_bl_window_.\n", __WFUNCTION__);
			continue;
		}
#endif
		
		// 주의 s_key 가 "file" 이면  abs_path_rom_file.wstring() 의 ":" 은 "::" 로 변경되지 않는다.
		if (!ptr_device->bootloader_operation(L"set", L"file", abs_path_rom_file.wstring())) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : error bootloader_operation for file.\n", __WFUNCTION__);
			continue;
		}
		
		if (dwFwIndex >= 0 && dwFwIndex < 256) {
			if (!ptr_device->bootloader_operation(L"set", L"firmware_index", std::to_wstring(dwFwIndex))) {
				_mp::clog::get_instance().log_fmt(L" : ERR : %ls : %u : error bootloader_operation for firmware_index.\n", __WFUNCTION__, dwFwIndex);
				continue;
			}
		}
		else {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : firmware_index = %u : firmware auto-selection mode.\n", __WFUNCTION__, dwFwIndex);
			// firmware 자동 선택 시도.
		}


		long n_item_index(-1);
		_mp::cwait::type_ptr ptr_evet_complete;
		std::shared_ptr<std::mutex> ptr_mutex;
		std::tie(n_item_index, ptr_evet_complete, ptr_mutex) = g_map_user_cb.add_callback(
			-1
			, v_id
			, cbUpdate
			, pUser
			, hWnd
			, uMsg
			, sRomFileName
			, dwFwIndex
		);
		if (n_item_index < 0) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : [critical error]add_callback error.\n", __WFUNCTION__);
			continue;
		}
		if (!ptr_mutex) {
			g_map_user_cb.remove_callback(n_item_index);
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : add_callback error : create mutex ptr.\n", __WFUNCTION__);
			continue;
		}
		
		bool b_result(false);
		int n_result_index(-1);

		// _cb_fw 가 change_result_index() 에 도달하기 전에 호출되어서 get_callback() 을 호츨하여,
		// 아직 설정되지 않은 n_result_index 값을 얻는 것을 방지 한다.
		ptr_mutex->lock();
		
		std::tie(b_result,n_result_index) = ptr_device->bootloader_operation_start(_cb_fw, (void*)n_item_index);
		if (!b_result) {
			ptr_mutex->unlock();
			g_map_user_cb.remove_callback(n_item_index);
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : error bootloader_operation for starting.\n", __WFUNCTION__);
			continue;
		}
		else {
			_mp::clog::get_instance().log_fmt(L" : INF : %ls : created result index = %u.\n", __WFUNCTION__, n_result_index);
			g_map_user_cb.change_result_index(n_item_index, n_result_index);
			ptr_mutex->unlock();
		}

		bool b_sync(false);
		if (hWnd == INVALID_HANDLE_VALUE || hWnd == 0) {
			if (cbUpdate == 0) {
				b_sync = true;
			}
		}

		if (b_sync) {
			// sync type operation
			// g_map_user_cb 에 생성된 callback item 은 
			// 결과를 얻어야 하므로 _cb_fw 에서 제거 되지 않고 여기서 제거 한다.

			std::vector<int> v_evt_index = ptr_evet_complete->wait_for_at_once((int)dwWaitTime);
			if (v_evt_index.empty()) {
				//timeout 의 경우,
				dw_result = ccb_client::const_dll_result_timeout;
			}
			else{

				dw_result = g_map_user_cb.get_sync_result(n_item_index); // 동기식 처리의 경우. 처리 완료.
			}
			g_map_user_cb.remove_callback(n_item_index); // remove invalid callback
		}
		else {
			dw_result = (unsigned long)n_item_index;
		}
	} while (false);

	switch (dw_result) {
	case ccb_client::const_dll_result_success:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success - %u(cb=0x%x,hwd=0x%x).\n", __WFUNCTION__, dw_result, cbUpdate, hWnd);
		break;

	case ccb_client::const_dll_result_error:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : error - %u(cb=0x%x,hwd=0x%x).\n", __WFUNCTION__, dw_result, cbUpdate, hWnd);
		break;
	case ccb_client::const_dll_result_timeout:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : timeout - %u(cb=0x%x,hwd=0x%x).\n", __WFUNCTION__, dw_result, cbUpdate, hWnd);
		break;
	case ccb_client::const_dll_result_no_msr:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : no msr - %u(cb=0x%x,hwd=0x%x).\n", __WFUNCTION__, dw_result, cbUpdate, hWnd);
		break;
	default:
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u(cb=0x%x,hwd=0x%x).\n", __WFUNCTION__, dw_result, cbUpdate, hWnd);
		break;
	}//end switch
	return dw_result;
}

/////////////////////////////////////////////////////////////////////////
// exported function body
/////////////////////////////////////////////////////////////////////////
unsigned long _CALLTYPE_ LPU237_fw_on()
{
	cdll_ini& cini(cdll_ini::get_instance());

#ifndef _WIN32
	std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
	std::string s_pipe_name_of_trace(_mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME);

	bool b_ini = cini.load_definition_file(_mp::ccoffee_path::get_path_of_coffee_lpu237_fw_ini_file());

	//setup tracing system
	_mp::clog& log(_mp::clog::get_instance());
	log.enable_trace(s_pipe_name_of_trace, false); //enable trace by client mode

	//setup logging system
	log.config(s_log_root_folder_except_backslash, 6, std::wstring(L"coffee_manager"), std::wstring(L"tg_lpu237_fw"), std::wstring(L"tg_lpu237_fw"));
	log.remove_log_files_older_then_now_day(cini.get_log_days_to_keep());
	log.enable(cini.get_log_enable());

	log.log_fmt(L"[I] START tg_lpu237_fw so or dll.\n");
	log.log_fmt(L"%ls", cini.get_string().c_str());
#endif
	unsigned long dwResult(ccb_client::const_dll_result_error);
	_mp::clog::get_instance().log_fmt(L" : CAL : %ls.\n", __WFUNCTION__);

	bool b_setup(false);
	std::wstring s_deb;
	std::tie(b_setup, s_deb) = _setup_rom_dll();
	if (!b_setup) {
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : setup tg_rom library L %ls.\n", __WFUNCTION__, s_deb.c_str());
		return dwResult;
	}
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

unsigned long _CALLTYPE_ LPU237_fw_off()
{
	unsigned long dwResult(ccb_client::const_dll_result_error);
	_mp::clog::get_instance().log_fmt(L" : CAL : %ls.\n", __WFUNCTION__);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {

		g_ptr_rom_dll.reset(); // release tg_rom library.

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

void _CALLTYPE_ LPU237_fw_set_mode(unsigned long nMode)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : %ls : mode = %u.\n", __WFUNCTION__, nMode);

	g_map_user_cb.set_mode(nMode);

	_mp::clog::get_instance().log_fmt(L" : RET : %ls.\n", __WFUNCTION__);
}



//
// ssDevPaths == NULL : return need size of ssDevPaths. [ unit : byte ]
// ssDevPaths != NULL : return the number of device path.
// wchar_t* ssDevPaths : [in/out] Multi string of devices
unsigned long _CALLTYPE_ LPU237_fw_get_list_w(wchar_t* ssDevPaths)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237_fw_get_list_w\n");
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


unsigned long _CALLTYPE_ LPU237_fw_get_list_a(char* ssDevPaths)
{
	if (ssDevPaths) {
		std::vector<wchar_t> vDevPaths(2048, 0);
		unsigned long dwResult = LPU237_fw_get_list_w(&vDevPaths[0]);

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
		return (LPU237_fw_get_list_w(NULL) / sizeof(wchar_t));
	}
}

HANDLE _CALLTYPE_ LPU237_fw_open_w(const wchar_t* sDevPath)
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


HANDLE _CALLTYPE_ LPU237_fw_open_a(const char* sDevPath)
{
	if (sDevPath) {
		std::string dev(sDevPath);
		std::wstring tdev;

		std::for_each(std::begin(dev), std::end(dev), [&](std::string::value_type c) {
			tdev.push_back(static_cast<std::wstring::value_type>(c));
			});

		return LPU237_fw_open_w(tdev.c_str());
	}
	else {
		return LPU237_fw_open_w(NULL);
	}
}

unsigned long _CALLTYPE_ LPU237_fw_close(HANDLE hDev)
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

unsigned long _CALLTYPE_ LPU237_fw_msr_save_setting(HANDLE hDev)
{
	// 과거 버전과 호환성을 위해서 남겨둔 API로, 현재는 별도의 설정이 없으므로 항상 성공을 반환한다.
	return LPU237_FW_RESULT_SUCCESS;
}

unsigned long _CALLTYPE_ LPU237_fw_msr_recover_setting(HANDLE hDev)
{
	// 과거 버전과 호환성을 위해서 남겨둔 API로, 현재는 별도의 설정이 없으므로 항상 성공을 반환한다.
	return LPU237_FW_RESULT_SUCCESS;
}

unsigned long _CALLTYPE_ LPU237_fw_msr_get_id(HANDLE hDev, unsigned char* sId)
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

unsigned long _CALLTYPE_ LPU237_fw_msr_get_name(HANDLE hDev, unsigned char* sName)
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
		_mp::type_v_buffer v_name = ptr_device->get_name();

		if (sName == NULL) {
			dwResult = cprotocol_lpu237::the_size_of_name;
			continue;
		}
		//
		std::for_each(std::begin(v_name), std::end(v_name), [&](unsigned char n) {
			*sName = n;
			sName++;
			});

		dwResult = cprotocol_lpu237::the_size_of_name;
	} while (0);

	_mp::clog::get_instance().log_fmt(L" : RET : %ls : %d\n", __WFUNCTION__, cprotocol_lpu237::the_size_of_name);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_fw_msr_get_version(HANDLE hDev, unsigned char* sVersion)
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
		_mp::type_v_buffer v_version = ptr_device->get_system_version().get_by_vector();

		if (sVersion == NULL) {
			dwResult = cprotocol_lpu237::the_size_of_version;
			continue;
		}
		//
		std::for_each(std::begin(v_version), std::end(v_version), [&](unsigned char n) {
			*sVersion = n;
			sVersion++;
			});

		dwResult = cprotocol_lpu237::the_size_of_version;
	} while (0);

	_mp::clog::get_instance().log_fmt(L" : RET : %ls : %d\n", __WFUNCTION__, cprotocol_lpu237::the_size_of_version);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_fw_msr_get_version_major(const unsigned char* sVersion)
{
	unsigned long dw_result(LPU237_FW_RESULT_ERROR);
	_mp::clog::get_instance().log_fmt(L" : CAL : %ls : 0x%x\n", __WFUNCTION__, sVersion);
	do {
		if (sVersion == 0)
			continue;
		dw_result = sVersion[0];
	} while (0);
	_mp::clog::get_instance().log_fmt(L" : RET : %ls\n", __WFUNCTION__);
	return dw_result;
}

unsigned long _CALLTYPE_ LPU237_fw_msr_get_version_minor(const unsigned char* sVersion)
{
	unsigned long dw_result(LPU237_FW_RESULT_ERROR);
	_mp::clog::get_instance().log_fmt(L" : CAL : %ls : 0x%x\n", __WFUNCTION__, sVersion);
	do {
		if (sVersion == 0)
			continue;
		dw_result = sVersion[1];
	} while (0);
	_mp::clog::get_instance().log_fmt(L" : RET : %ls\n", __WFUNCTION__);
	return dw_result;
}

// 과거 버전 호환성을	위해 남겨둔 API로, 현재는 별도의 업데이트 과정이 없으므로 항상 성공을 반환한다.
unsigned long _CALLTYPE_ LPU237_fw_msr_cancel_update()
{
	unsigned long dwResult(ccb_client::const_dll_result_error);
	_mp::clog::get_instance().log_fmt(L" : CAL : %ls\n", __WFUNCTION__);
	_mp::clog::get_instance().log_fmt(L" : RET : %ls : not support. if update is canceled, fw is blocked.\n", __WFUNCTION__);
	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_fw_msr_update_w(const unsigned char* sId, unsigned long dwWaitTime, const wchar_t* sRomFileName, unsigned long dwIndex)
{
	return _fw_msr_update_ex_w(sId, dwWaitTime, nullptr, nullptr, NULL, 0, sRomFileName, dwIndex);
}

unsigned long _CALLTYPE_ LPU237_fw_msr_update_a(const unsigned char* sId, unsigned long dwWaitTime, const char* sRomFileName, unsigned long dwIndex)
{
	if (sRomFileName) {
		std::string s(sRomFileName);
		std::wstring ws;

		std::for_each(std::begin(s), std::end(s), [&](std::string::value_type c) {
			ws.push_back(static_cast<std::wstring::value_type>(c));
			});

		return LPU237_fw_msr_update_w(sId, dwWaitTime, ws.c_str(), dwIndex);
	}
	else {
		return LPU237_FW_RESULT_ERROR;
	}
}

unsigned long _CALLTYPE_ LPU237_fw_msr_update_callback_w(const unsigned char* sId, type_lpu237_fw_callback cbUpdate, void* pUser, const wchar_t* sRomFileName, unsigned long dwIndex)
{
	return _fw_msr_update_ex_w(sId, 0, cbUpdate, pUser, NULL, 0, sRomFileName, dwIndex);
}

unsigned long _CALLTYPE_ LPU237_fw_msr_update_callback_a(const unsigned char* sId, type_lpu237_fw_callback cbUpdate, void* pUser, const char* sRomFileName, unsigned long dwIndex)
{
	if (sRomFileName) {
		std::string s(sRomFileName);
		std::wstring ws;

		std::for_each(std::begin(s), std::end(s), [&](std::string::value_type c) {
			ws.push_back(static_cast<std::wstring::value_type>(c));
			});

		return LPU237_fw_msr_update_callback_w(sId, cbUpdate, pUser, ws.c_str(), dwIndex);
	}
	else {
		return LPU237_FW_RESULT_ERROR;
	}
}

unsigned long _CALLTYPE_ LPU237_fw_msr_update_wnd_w(const unsigned char* sId, HWND hWnd, UINT uMsg, const wchar_t* sRomFileName, unsigned long dwIndex)
{
#ifdef _WIN32
	return _fw_msr_update_ex_w(sId, 0, nullptr, nullptr, hWnd, uMsg, sRomFileName, dwIndex);
#else
	return LPU237_FW_RESULT_ERROR; // not support in linux.
#endif
}

unsigned long _CALLTYPE_ LPU237_fw_msr_update_wnd_a(const unsigned char* sId, HWND hWnd, UINT uMsg, const char* sRomFileName, unsigned long dwIndex)
{
	if (sRomFileName) {
		std::string s(sRomFileName);
		std::wstring ws;

		std::for_each(std::begin(s), std::end(s), [&](std::string::value_type c) {
			ws.push_back(static_cast<std::wstring::value_type>(c));
			});

		return LPU237_fw_msr_update_wnd_w(sId, hWnd, uMsg, ws.c_str(), dwIndex);
	}
	else {
		return LPU237_FW_RESULT_ERROR;
	}
}

unsigned long _CALLTYPE_ LPU237_fw_rom_load_w(const wchar_t* sRomFileName)
{
	unsigned long dw_result(LPU237_FW_RESULT_ERROR);

	CRom::type_result result_rom(CRom::result_success);
	std::wstring s_romfile;

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : %ls\n", __WFUNCTION__, sRomFileName);

		if (sRomFileName == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none rom file path\n", __WFUNCTION__);
			continue;
		}

		if (!g_ptr_rom_dll) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : ERR : tg_rom library : not constructed\n", __WFUNCTION__);
			continue;
		}
		if (!g_ptr_rom_dll->is_ini()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : ERR : tg_rom library : not initialized\n", __WFUNCTION__);
			continue;
		}

		CRom::ROMFILE_HEAD Header = { 0, };
		result_rom = g_ptr_rom_dll->LoadHeader(sRomFileName, &Header);
		if (result_rom != CRom::result_success) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : ERR : LoadHeader()\n", __WFUNCTION__);
			continue;
		}
		dw_result = LPU237_FW_RESULT_SUCCESS;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dw_result;
}

unsigned long _CALLTYPE_ LPU237_fw_rom_load_a(const char* sRomFileName)
{
	if (sRomFileName) {
		std::string s(sRomFileName);
		std::wstring ws;

		std::for_each(std::begin(s), std::end(s), [&](std::string::value_type c) {
			ws.push_back(static_cast<std::wstring::value_type>(c));
			});

		return LPU237_fw_rom_load_w(ws.c_str());
	}
	else {
		return LPU237_FW_RESULT_ERROR;
	}
}

unsigned long _CALLTYPE_ LPU237_fw_rom_get_index_w(const wchar_t* sRomFileName, const unsigned char* sName, const unsigned char* sVersion)
{
	unsigned long dw_result(LPU237_FW_RESULT_ERROR);
	CRom::type_result result_rom(CRom::result_success);

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : %ls \n", __WFUNCTION__, sRomFileName);

		if (sName == NULL || sVersion == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %s : sName or sVersion\n", __WFUNCTION__);
			continue;
		}

		if (!g_ptr_rom_dll) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : ERR : tg_rom library : not constructed\n", __WFUNCTION__);
			continue;
		}
		if (!g_ptr_rom_dll->is_ini()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : ERR : tg_rom library : not initialized\n", __WFUNCTION__);
			continue;
		}

		CRom::ROMFILE_HEAD Header = { 0, };

		if (sRomFileName != NULL) {
			result_rom = g_ptr_rom_dll->LoadHeader(sRomFileName, &Header);
			if (result_rom != CRom::result_success) {
				_mp::clog::get_instance().log_fmt(" : RET : %s : LPU237_fw_rom_load_w : ERR\n", __WFUNCTION__);
				continue;
			}
		}
		//
		//
		int nIndex = g_ptr_rom_dll->GetUpdatableItemIndex(sName, sVersion[0], sVersion[1], sVersion[2], sVersion[3]);
		if (nIndex < 0) {
			_mp::clog::get_instance().log_fmt(" : RET : %s : grom.GetUpdatableItemIndex\n", __WFUNCTION__);
			continue;
		}

		dw_result = nIndex;
		_mp::clog::get_instance().log_fmt(" : RET : %s : success : %d\n", __WFUNCTION__, dw_result);
	} while (0);

	return dw_result;
}

unsigned long _CALLTYPE_ LPU237_fw_rom_get_index_a(const char* sRomFileName, const unsigned char* sName, const unsigned char* sVersion)
{
	if (sRomFileName) {
		std::string s(sRomFileName);
		std::wstring ws;

		std::for_each(std::begin(s), std::end(s), [&](std::string::value_type c) {
			ws.push_back(static_cast<std::wstring::value_type>(c));
			});

		return LPU237_fw_rom_get_index_w(ws.c_str(), sName, sVersion);
	}
	else {
		return LPU237_FW_RESULT_ERROR;
	}
}






