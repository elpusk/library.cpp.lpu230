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

#include <mp_clog.h>
#include <mp_cconvert.h>
#include <cprotocol_lpu237.h>
#include <tg_lpu237_dll.h>
#include <mp_coffee.h>

#include <manager_of_device_of_client.h>
#include <ccb_client.h>
#include <lpu237_of_client.h>
#include <cdef.h>
#include <cdll_ini.h>

#include "GemCoreScrHelper.h"
#include "cash_msdata.h"

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


/////////////////////////////////////////////////////////////////////////
// local class
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// local function prototype
/////////////////////////////////////////////////////////////////////////
static bool _pre_check_for_scr(unsigned long& dwResult, const std::wstring& sFuntionName, HANDLE hDev = NULL);
static unsigned long _post_process_for_scr_in_success_case(const _mp::type_v_buffer& vCcidRaw, const std::wstring& sFuntionName);
static unsigned long _last_result_code(unsigned long dwResult, bool bSet);
static unsigned char _last_bwi(unsigned char cBWI, bool bSet);

/////////////////////////////////////////////////////////////////////////
// global variable
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// local function body
/////////////////////////////////////////////////////////////////////////

bool _pre_check_for_scr(unsigned long& dwResult, const std::wstring& sFuntionName, HANDLE hDev /*=NULL*/)
{
	dwResult = 0;

	_mp::clog::get_instance().log_fmt(L" : CAL : %ls : 0x%x\n", sFuntionName.c_str(), hDev);

	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	bool b_result(false);

	do {
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls, none manager_of_device_of_client\n", sFuntionName.c_str());
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}
		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (ptr_device->is_null_device()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls, Handle  invalid\n", sFuntionName.c_str());
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}
		b_result = true;
	} while (false);

	return b_result;
}

unsigned long _post_process_for_scr_in_success_case(const _mp::type_v_buffer& vCcidRaw, const std::wstring& sFuntionName)
{
	std::wstring s_hex;
	_mp::cconvert::hex_string_from_binary(s_hex, vCcidRaw, L" ");
	_mp::clog::get_instance().log_fmt(L" : RSP : %ls\n", s_hex.c_str());
	_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", sFuntionName.c_str());

	unsigned long dwResult(0);

	if (vCcidRaw.size() >= sizeof(gemcore_scr_interface::ccid_rdr_to_pc_header)) {
		const gemcore_scr_interface::ccid_rdr_to_pc_header* pRsp(reinterpret_cast<const gemcore_scr_interface::ccid_rdr_to_pc_header*>(&vCcidRaw[0]));
		dwResult = (pRsp->cStatus << 24) | (pRsp->cError << 16);

		if (pRsp->cStatus == 0x46 && pRsp->cError == 0xFE) {
			//inserted notification
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_SUCCESS | LPU237_DLL_SCR_RESULT_STATUS_ICC_PRESENT_INACTIVE;
			_mp::clog::get_instance().log_fmt(L" : MOD : insertion notification.\n");
		}
	}
	else {
		dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN | LPU237_DLL_SCR_RESULT_ERROR_CMD_ABORTED;
	}
	return dwResult;
}

unsigned long _last_result_code(unsigned long dwResult, bool bSet)
{
	static unsigned long dwLastResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_SUCCESS | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	unsigned long dwLast = dwLastResult;

	if (bSet) {
		dwLastResult = dwResult;
	}

	return dwLast;
}

unsigned char _last_bwi(unsigned char cBWI, bool bSet)
{
	static unsigned char cLastBWI = gemcore_scr_interface::CGemCoreScr_Helper::CCardInfo::default_BWI;
	unsigned char cLast = cLastBWI;

	if (bSet)
		cLastBWI = cBWI;

	return cLast;
}

/////////////////////////////////////////////////////////////////////////
// exported function body
/////////////////////////////////////////////////////////////////////////
//
// ssDevPaths == NULL : return need size of ssDevPaths. [ unit : byte ]
// ssDevPaths != NULL : return the number of device path.
// wchar_t* ssDevPaths : [in/out] Multi string of devices
unsigned long _CALLTYPE_ LPU237_get_list(wchar_t* ssDevPaths)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237_get_list\n");
	//const std::wstring s_filter(L"hid#vid_134b&pid_0206&mi_01");
	_mp::type_list_wstring list_filter{ L"lpu200",L"msr" };
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
*	equal to LPU237_get_list.
*  LPU237_get_list' unicode version
*/
unsigned long _CALLTYPE_ LPU237_get_list_w(wchar_t* ssDevPaths)
{
	if (ssDevPaths)
		return LPU237_get_list(ssDevPaths);
	else {
		return (LPU237_get_list(ssDevPaths));
	}
}

/*!
* function
*	equal to LPU237_get_list.
*  LPU237_get_list' Multi Byte Code Set version
*/
unsigned long _CALLTYPE_ LPU237_get_list_a(char* ssDevPaths)
{
	if (ssDevPaths) {
		std::vector<wchar_t> vDevPaths(2048, 0);
		unsigned long dwResult = LPU237_get_list(&vDevPaths[0]);

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
		return (LPU237_get_list(NULL) / sizeof(wchar_t));
	}
}

HANDLE _CALLTYPE_ LPU237_open(const wchar_t* sDevPath)
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

		n_device_index = ptr_manager_of_device_of_client->create_device(std::wstring(sDevPath));
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

HANDLE _CALLTYPE_ LPU237_open_w(const wchar_t* sDevPath)
{
	return LPU237_open(sDevPath);
}

HANDLE _CALLTYPE_ LPU237_open_a(const char* sDevPath)
{
	if (sDevPath) {
		std::string dev(sDevPath);
		std::wstring tdev;

		std::for_each(std::begin(dev), std::end(dev), [&](std::string::value_type c) {
			tdev.push_back(static_cast<std::wstring::value_type>(c));
			});

		return LPU237_open(tdev.c_str());
	}
	else {
		return LPU237_open(NULL);
	}
}

unsigned long _CALLTYPE_ LPU237_close(HANDLE hDev)
{
	unsigned long dwResult(LPU237_DLL_RESULT_ERROR);
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

		if (!ptr_device->cmd_leave_opos()) {
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_leave_opos\n", __WFUNCTION__);
		}
		if (!ptr_device->cmd_enter_config()) {//for redetecting decoder.
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_enter_config\n", __WFUNCTION__);
		}
		if (!ptr_device->cmd_leave_config()) {//for redetecting decoder.
			_mp::clog::get_instance().log_fmt(L" : ERR : %ls : cmd_leave_config\n", __WFUNCTION__);
		}

		if (!ptr_manager_of_device_of_client->remove_device(n_device_index)) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : remove_device\n", __WFUNCTION__);
			continue;
		}

		dwResult = LPU237_DLL_RESULT_SUCCESS;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_enable(HANDLE hDev)
{
	unsigned long dwResult(LPU237_DLL_RESULT_ERROR);
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
		if (!ptr_device->cmd_enter_opos()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_enter_opos\n", __WFUNCTION__);
			continue;
		}
		dwResult = LPU237_DLL_RESULT_SUCCESS;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_disable(HANDLE hDev)
{
	unsigned long dwResult(LPU237_DLL_RESULT_ERROR);
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
		if (!ptr_device->cmd_leave_opos()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_leave_opos\n", __WFUNCTION__);
			continue;
		}
		dwResult = LPU237_DLL_RESULT_SUCCESS;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_enter_opos(HANDLE hDev)
{
	unsigned long dwResult(LPU237_DLL_RESULT_ERROR);
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
		if (!ptr_device->cmd_enter_opos()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_enter_opos\n", __WFUNCTION__);
			continue;
		}
		dwResult = LPU237_DLL_RESULT_SUCCESS;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_leave_opos(HANDLE hDev)
{
	unsigned long dwResult(LPU237_DLL_RESULT_ERROR);
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
		if (!ptr_device->cmd_leave_opos()) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_leave_opos\n", __WFUNCTION__);
			continue;
		}
		dwResult = LPU237_DLL_RESULT_SUCCESS;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_cancel_wait_swipe(HANDLE hDev)
{
	unsigned long dwResult(LPU237_DLL_RESULT_ERROR);
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

		dwResult = LPU237_DLL_RESULT_SUCCESS;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success\n", __WFUNCTION__);
	} while (0);

	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_wait_swipe_with_waits(HANDLE hDev)
{
	unsigned long dw_result(LPU237_DLL_RESULT_ERROR);
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

		int n_result_index = ptr_device->cmd_async_waits_data();
		if (n_result_index < 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_async_waits_ms_card.\n", __WFUNCTION__);
			continue;
		}

		_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_async_parameter_result = ptr_device->get_async_parameter_result_for_transaction(n_result_index);
		if (!ptr_async_parameter_result->waits()) {
			ptr_device->remove_async_result_for_transaction(n_result_index);
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : waits.\n", __WFUNCTION__);
			continue;
		}
		//
		if (!ptr_async_parameter_result->get_result(dw_result)) {
			ptr_device->remove_async_result_for_transaction(n_result_index);
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : get_result.\n", __WFUNCTION__);
			continue;
		}

		if (dw_result == LPU237_DLL_RESULT_ERROR) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : LPU237_DLL_RESULT_ERROR\n", __WFUNCTION__);
			continue;
		}

		dw_result = n_result_index;
		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success - %u.\n", __WFUNCTION__, n_result_index);
	} while (0);

	return dw_result;
}

unsigned long _CALLTYPE_ LPU237_wait_swipe_with_callback(HANDLE hDev, type_callback pFun, void* pParameter)
{
	unsigned long dw_result(LPU237_DLL_RESULT_ERROR);
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

		int n_result_index = ptr_device->cmd_async_waits_data(pFun, pParameter);
		if (n_result_index < 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_async_waits_ms_card.\n", __WFUNCTION__);
			continue;
		}

		dw_result = (unsigned long)n_result_index;

		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success - %u.\n", __WFUNCTION__, n_result_index);
	} while (0);

	return dw_result;
}

unsigned long _CALLTYPE_ LPU237_wait_swipe_with_message(HANDLE hDev, HWND hWnd, UINT nMsg)
{
	unsigned long dw_result(LPU237_DLL_RESULT_ERROR);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : 0x%x\n", __WFUNCTION__, hDev);
		if (hWnd == NULL) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : hWnd == NULL : error\n", __WFUNCTION__);
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

		int n_result_index = ptr_device->cmd_async_waits_data(hWnd, nMsg);
		if (n_result_index < 0) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_async_waits_ms_card.\n", __WFUNCTION__);
			continue;
		}

		dw_result = (unsigned long)n_result_index;

		_mp::clog::get_instance().log_fmt(L" : RET : %ls : success - %u.\n", __WFUNCTION__, n_result_index);

	} while (0);

	return dw_result;
}

unsigned long _CALLTYPE_ LPU237_get_data(unsigned long dwBufferIndex, unsigned long dwIsoTrack, unsigned char* sTrackData)
{
	unsigned long dw_client_result(LPU237_DLL_RESULT_ERROR);

	static cash_msdata cash;
	int n_result_index(-1);
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());

	do {
		_mp::clog::get_instance().log_fmt(L" : CAL : %ls : %d,%d\n", __WFUNCTION__, dwBufferIndex, dwIsoTrack);

		if (dwIsoTrack < 1 || dwIsoTrack> 3) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : dwIsoTrack<1 || dwIsoTrack> 3\n", __WFUNCTION__);
			continue;
		}

		n_result_index = (int)dwBufferIndex;
		if (n_result_index < 0) {
			dw_client_result = n_result_index;
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, dw_client_result);
			continue;
		}

		if (cash.is_cashed(n_result_index)) {
			if (sTrackData) {
				_mp::type_v_buffer v_iso(0);
				dw_client_result = cash.get_data(dwIsoTrack - 1, v_iso);
				if (v_iso.size() > 0) {
					std::copy(std::begin(v_iso), std::end(v_iso), sTrackData);
				}
			}
			else {
				dw_client_result = cash.get_data(dwIsoTrack - 1);
			}
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cashed data.\n", __WFUNCTION__);
			continue;
		}

		cash.reset();

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = ptr_manager_of_device_of_client->get_async_parameter_result_for_manager_from_all_device(n_result_index);
		if (!ptr_result) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : INVALID_HANDLE_VALUE\n", __WFUNCTION__);
			continue;
		}

		_mp::type_v_buffer v_out_rx(0);

		if (!ptr_result->get_result(v_out_rx)) {
			ptr_result->get_result(dw_client_result);
			//dw_client_result = LPU237_DLL_RESULT_ERROR;
			ptr_manager_of_device_of_client->remove_async_result_for_manager(n_result_index);
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error\n", __WFUNCTION__);
			continue;
		}

		ptr_result->get_result(dw_client_result);
		ptr_manager_of_device_of_client->remove_async_result_for_manager(n_result_index);
		cash.cashing(n_result_index, dw_client_result, v_out_rx);

		std::wstring s_track;
		int j = 0;

		dw_client_result = cash.get_data(dwIsoTrack - 1);

		switch (dw_client_result) {
		case LPU237_DLL_RESULT_ICC_INSERTED:
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : inserted ICC\n", __WFUNCTION__);
			continue;
		case LPU237_DLL_RESULT_ICC_REMOVED:
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : removed ICC\n", __WFUNCTION__);
			continue;
		case LPU237_DLL_RESULT_CANCEL:
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : canceled\n", __WFUNCTION__);
			continue;
		case LPU237_DLL_RESULT_ERROR:
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error\n", __WFUNCTION__);
			continue;
		case LPU237_DLL_RESULT_ERROR_MSR:
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : error msr\n", __WFUNCTION__);
			continue;
		case LPU237_DLL_RESULT_SUCCESS:
		default:
			if (dw_client_result > 0 && sTrackData != NULL) {
				_mp::type_v_buffer v_iso(0);
				cash.get_data(dwIsoTrack - 1, v_iso);
				std::copy(std::begin(v_iso), std::end(v_iso), sTrackData);
			}

			_mp::clog::get_instance().log_fmt(L" : RET : %ls : %u\n", __WFUNCTION__, static_cast<unsigned long>(s_track.length()));
			continue;
		}//end switch
	} while (0);

	return dw_client_result;
}

unsigned long _CALLTYPE_ LPU237_dll_on()
{
	cdll_ini& cini(cdll_ini::get_instance());

#ifndef _WIN32
	std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
	std::string s_pipe_name_of_trace(_mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME);

	bool b_ini = cini.load_definition_file(_mp::ccoffee_path::get_path_of_coffee_lpu237_dll_ini_file());

	//setup tracing system
	_mp::clog& log(_mp::clog::get_instance());
	log.enable_trace(s_pipe_name_of_trace, false); //enable trace by client mode

	//setup logging system
	log.config(s_log_root_folder_except_backslash, 6, std::wstring(L"coffee_manager"), std::wstring(L"tg_lpu237_dll"), std::wstring(L"tg_lpu237_dll"));
	log.remove_log_files_older_then_now_day(cini.get_log_days_to_keep());
	log.enable(cini.get_log_enable());

	log.log_fmt(L"[I] START tg_lpu237_dll so or dll.\n");
	log.log_fmt(L"%ls", cini.get_string().c_str());
#endif
	unsigned long dwResult(LPU237_DLL_RESULT_ERROR);
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
		dwResult = LPU237_DLL_RESULT_SUCCESS;
	} while (false);

	_mp::clog::get_instance().log_fmt(L" : RET : %ls.\n", __WFUNCTION__);
	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_dll_off()
{
	unsigned long dwResult(LPU237_DLL_RESULT_ERROR);
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

		dwResult = LPU237_DLL_RESULT_SUCCESS;
	} while (false);

	manager_of_device_of_client<lpu237_of_client>::get_instance(true);//remove manager
	_mp::clog::get_instance().log_fmt(L" : RET : %ls.\n", __WFUNCTION__);
	return dwResult;
}

unsigned long _CALLTYPE_ LPU237_get_id(HANDLE hDev, unsigned char* sId)
{
	unsigned long dwResult(LPU237_DLL_RESULT_ERROR);
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
unsigned long _CALLTYPE_ LPU237_SCR_bypass_IccPowerOn(HANDLE hDev, unsigned char cPower, unsigned char* sRx, unsigned long* lpnRx)
{
	unsigned long dwResult(0);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	do {
		if (!_pre_check_for_scr(dwResult, __WFUNCTION__, hDev))
			continue;
		//
		_mp::type_v_buffer vtx(0), vrx(0);
		gemcore_scr_interface::CGemCoreScr_Helper::build_PC_to_RDR_IccPowerOn(vtx, static_cast<gemcore_scr_interface::typeIccPower>(cPower));
		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (!ptr_device->cmd_bypass(vtx, vrx)) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_bypass.\n", __WFUNCTION__);
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}

		gemcore_scr_interface::CGemCoreScr_Helper::CResponse rsp(vrx);
		_mp::type_v_buffer datafield(rsp.getCcidDataField());
		if (sRx) { std::copy(std::begin(datafield), std::end(datafield), sRx); }
		if (lpnRx)		*lpnRx = (unsigned long)datafield.size();

		dwResult = _post_process_for_scr_in_success_case(rsp.getCcidRaw(), __WFUNCTION__);
	} while (0);
	return dwResult;
}

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
unsigned long _CALLTYPE_ LPU237_SCR_bypass_IccPowerOff(HANDLE hDev)
{
	unsigned long dwResult(0);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	do {
		if (!_pre_check_for_scr(dwResult, __WFUNCTION__, hDev))
			continue;

		_mp::type_v_buffer vtx(0), vrx(0);
		gemcore_scr_interface::CGemCoreScr_Helper::build_PC_to_RDR_IccPowerOff(vtx);

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (!ptr_device->cmd_bypass(vtx, vrx)) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_bypass.\n", __WFUNCTION__);
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}
		gemcore_scr_interface::CGemCoreScr_Helper::CResponse rsp(vrx);
		dwResult = _post_process_for_scr_in_success_case(rsp.getCcidRaw(), __WFUNCTION__);
	} while (0);

	return dwResult;
}


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
)
{
	unsigned long dwResult(0);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	do {
		if (!_pre_check_for_scr(dwResult, __WFUNCTION__, hDev))
			continue;
		//
		_mp::type_v_buffer vtx(0), vrx(0), vdata(nTx, 0);
		std::copy(sTx, &sTx[nTx], std::begin(vdata));
		gemcore_scr_interface::CGemCoreScr_Helper::build_PC_to_RDR_XfrBlock(vtx, cBWI, vdata);

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (!ptr_device->cmd_bypass(vtx, vrx)) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_bypass.\n", __WFUNCTION__);
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}

		gemcore_scr_interface::CGemCoreScr_Helper::CResponse rsp(vrx);
		_mp::type_v_buffer datafield(rsp.getCcidDataField());
		if (sRx) { std::copy(std::begin(datafield), std::end(datafield), sRx); }
		if (lpnRx)		*lpnRx = (unsigned long)datafield.size();

		dwResult = _post_process_for_scr_in_success_case(rsp.getCcidRaw(), __WFUNCTION__);
	} while (0);
	return dwResult;
}


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
unsigned long _CALLTYPE_ LPU237_SCR_bypass_GetParameters(HANDLE hDev, unsigned char* sRx, unsigned long* lpnRx)
{
	unsigned long dwResult(0);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	do {
		if (!_pre_check_for_scr(dwResult, __WFUNCTION__, hDev))
			continue;
		//
		_mp::type_v_buffer vtx(0), vrx(0);
		gemcore_scr_interface::CGemCoreScr_Helper::build_PC_to_RDR_GetParameters(vtx);

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (!ptr_device->cmd_bypass(vtx, vrx)) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_bypass.\n", __WFUNCTION__);
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}
		gemcore_scr_interface::CGemCoreScr_Helper::CResponse rsp(vrx);
		_mp::type_v_buffer datafield(rsp.getCcidDataField());
		if (sRx) {
			gemcore_scr_interface::ccid_rdr_to_pc_header* pRsp = reinterpret_cast<gemcore_scr_interface::ccid_rdr_to_pc_header*>(&rsp.getCcidRaw()[0]);
			sRx[0] = pRsp->asCmd[0];	//get protocol

			std::copy(std::begin(datafield), std::end(datafield), &sRx[1]);
		}
		if (lpnRx)		*lpnRx = (unsigned long)datafield.size() + 1;

		dwResult = _post_process_for_scr_in_success_case(rsp.getCcidRaw(), __WFUNCTION__);
	} while (0);
	return dwResult;
}


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
)
{
	unsigned long dwResult(0);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	do {
		if (!_pre_check_for_scr(dwResult, __WFUNCTION__, hDev))
			continue;
		_mp::type_v_buffer vtx(0), vrx(0);
		gemcore_scr_interface::CGemCoreScr_Helper::build_ex_PC_to_RDR_SetParameters(vtx, static_cast<gemcore_scr_interface::typeIccProtocol>(cProtocol), bmFindexDindex, bmTCCKST, bGuardTime, bWaitingInteger, bIFSC);

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (!ptr_device->cmd_bypass(vtx, vrx)) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_bypass.\n", __WFUNCTION__);
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}
		gemcore_scr_interface::CGemCoreScr_Helper::CResponse rsp(vrx);
		dwResult = _post_process_for_scr_in_success_case(rsp.getCcidRaw(), __WFUNCTION__);
	} while (0);
	return dwResult;
}

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
unsigned long _CALLTYPE_ LPU237_SCR_bypass_ResetParameters(HANDLE hDev)
{
	unsigned long dwResult(0);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	do {
		if (!_pre_check_for_scr(dwResult, __WFUNCTION__, hDev))
			continue;
		_mp::type_v_buffer vtx(0), vrx(0);
		gemcore_scr_interface::CGemCoreScr_Helper::build_PC_to_RDR_ResetParameters(vtx);

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (!ptr_device->cmd_bypass(vtx, vrx)) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_bypass.\n", __WFUNCTION__);
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}
		gemcore_scr_interface::CGemCoreScr_Helper::CResponse rsp(vrx);
		dwResult = _post_process_for_scr_in_success_case(rsp.getCcidRaw(), __WFUNCTION__);
	} while (0);
	return dwResult;
}


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
unsigned long _CALLTYPE_ LPU237_SCR_bypass_GetSlotStatus(HANDLE hDev)
{
	unsigned long dwResult(0);
	unsigned long n_device_index(PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	do {
		if (!_pre_check_for_scr(dwResult, __WFUNCTION__, hDev))
			continue;
		_mp::type_v_buffer vtx(0), vrx(0);
		gemcore_scr_interface::CGemCoreScr_Helper::build_PC_to_RDR_GetSlotStatus(vtx);

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (!ptr_device) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : get_device(E).\n", __WFUNCTION__);
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}
		_mp::clog::get_instance().log_fmt(L" : I : %ls : get_device.\n", __WFUNCTION__);

		if (!ptr_device->cmd_bypass(vtx, vrx)) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_bypass.\n", __WFUNCTION__);
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}
		gemcore_scr_interface::CGemCoreScr_Helper::CResponse rsp(vrx);
		dwResult = _post_process_for_scr_in_success_case(rsp.getCcidRaw(), __WFUNCTION__);
	} while (0);
	return dwResult;
}


/*!
* function
*	used to implement Gemplus-proprietary sub-commands.
*
* parameters
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
)
{
	unsigned long dwResult(0);
	unsigned long n_device_index((unsigned long)PtrToUlong(hDev));
	manager_of_device_of_client<lpu237_of_client>::type_ptr_manager_of_device_of_client ptr_manager_of_device_of_client(manager_of_device_of_client<lpu237_of_client>::get_instance());
	do {
		if (!_pre_check_for_scr(dwResult, __WFUNCTION__, hDev))
			continue;
		_mp::type_v_buffer vtx(0), vrx(0), vdata(nTx, 0);
		std::copy(sTx, &sTx[nTx], std::begin(vdata));
		gemcore_scr_interface::CGemCoreScr_Helper::build_PC_to_RDR_Escape(vtx, vdata);

		if (!ptr_manager_of_device_of_client) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : none manager_of_device_of_client.\n", __WFUNCTION__);
			continue;
		}

		lpu237_of_client::type_ptr_lpu237_of_client& ptr_device = ptr_manager_of_device_of_client->get_device(n_device_index);
		if (!ptr_device->cmd_bypass(vtx, vrx)) {
			_mp::clog::get_instance().log_fmt(L" : RET : %ls : cmd_bypass.\n", __WFUNCTION__);
			dwResult = LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
			continue;
		}
		gemcore_scr_interface::CGemCoreScr_Helper::CResponse rsp(vrx);
		_mp::type_v_buffer datafield(rsp.getCcidDataField());
		if (sRx) {
			std::copy(std::begin(datafield), std::end(datafield), sRx);
		}
		if (lpnRx)		*lpnRx = (unsigned long)datafield.size();

		dwResult = _post_process_for_scr_in_success_case(rsp.getCcidRaw(), __WFUNCTION__);
	} while (0);
	return dwResult;
}


///////////////////////////////////////////////////////////////////////////////////////
// helper

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
unsigned long LPU237_SCR_helper_GetLastError()
{
	return _last_result_code(0, false);
}

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
BOOL _CALLTYPE_ LPU237_SCR_helper_GetFirmwareVersion(HANDLE hDev, unsigned char* sRx, unsigned long* lpnRx)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237_SCR_helper_GetFirmwareVersion.\n");
	_mp::type_v_buffer vtx(1, SCR_ESCAPE_CMD_GET_FW_VERSION);
	unsigned long dwResult = LPU237_SCR_bypass_Escape(hDev, &vtx[0], (unsigned long)vtx.size(), sRx, lpnRx);
	_mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_GetFirmwareVersion.\n");

	_last_result_code(dwResult, true);
	return LPU237_SCR_helper_result_is_success(dwResult);
}

//#define	LPU237_DLL_SCR_MODE_PCSC		0x01
//#define	LPU237_DLL_SCR_MODE_EMV		0x02
//#define	LPU237_DLL_SCR_POWER_5V		0x10
//#define	LPU237_DLL_SCR_POWER_3V		0x20
//#define	LPU237_DLL_SCR_POWER_1V8		0x40
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
* 	0 : result is failure.
*	else : result is success
*/
BOOL _CALLTYPE_ LPU237_SCR_helper_GetReaderMode(HANDLE hDev, unsigned char* lpcMode)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237_SCR_helper_GetReaderMode.\n");

	_mp::type_v_buffer vtx(2, 0), vrx(1, 0);
	unsigned long nRx(0);

	vtx[0] = SCR_ESCAPE_CMD_SLOT_LEVEL;
	vtx[1] = 0x00;
	unsigned long dwResult = LPU237_SCR_bypass_Escape(hDev, &vtx[0], (unsigned long)vtx.size(), &vrx[0], &nRx);
	if (lpcMode)
		*lpcMode = vrx[0];

	_mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_GetReaderMode.\n");

	_last_result_code(dwResult, true);
	return LPU237_SCR_helper_result_is_success(dwResult);
}

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
BOOL _CALLTYPE_ LPU237_SCR_helper_SetReaderMode(HANDLE hDev, unsigned char cMode, unsigned char cPower)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237_SCR_helper_SetReaderMode.\n");

	_mp::type_v_buffer vtx(2, 0), vrx(1, 0);
	unsigned long nRx(0);

	vtx[0] = SCR_ESCAPE_CMD_SLOT_LEVEL;
	vtx[1] = cMode | cPower;
	unsigned long dwResult = LPU237_SCR_bypass_Escape(hDev, &vtx[0], (unsigned long)vtx.size(), &vrx[0], &nRx);

	_mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_SetReaderMode.\n");

	_last_result_code(dwResult, true);
	return LPU237_SCR_helper_result_is_success(dwResult);
}

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
)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237_SCR_helper_XfrBlock.\n");

	unsigned long dwResult = LPU237_SCR_bypass_XfrBlock(hDev, _last_bwi(0, false), sTx, nTx, sRx, lpnRx);

	_mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_XfrBlock.\n");

	_last_result_code(dwResult, true);
	return LPU237_SCR_helper_result_is_success(dwResult);
}

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
BOOL _CALLTYPE_ LPU237_SCR_helper_SetCardParameters(HANDLE hDev, unsigned char cProtocol, unsigned char cParameter, unsigned char cNewValue)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237_SCR_helper_SetCardParameters.\n");

	_mp::type_v_buffer vtx(4, 0), vrx(1, 0);
	unsigned long nRx(0);

	vtx[0] = SCR_ESCAPE_CMD_CARD_PARAMETERS;
	vtx[1] = cProtocol;
	vtx[2] = cParameter;
	vtx[3] = cNewValue;
	unsigned long dwResult = LPU237_SCR_bypass_Escape(hDev, &vtx[0], (unsigned long)vtx.size(), &vrx[0], &nRx);

	_mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_SetCardParameters.\n");

	_last_result_code(dwResult, true);
	return LPU237_SCR_helper_result_is_success(dwResult);
}

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
BOOL _CALLTYPE_ LPU237_SCR_helper_ResetCardParameters(HANDLE hDev)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237_SCR_helper_ResetCardParameters.\n");

	_mp::type_v_buffer vtx(1, SCR_ESCAPE_CMD_RESET_CARD_PARAMETERS);
	unsigned long dwResult = LPU237_SCR_bypass_Escape(hDev, &vtx[0], (unsigned long)vtx.size(), NULL, NULL);

	_mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_ResetCardParameters.\n");

	_last_result_code(dwResult, true);
	return LPU237_SCR_helper_result_is_success(dwResult);
}

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
BOOL _CALLTYPE_ LPU237_SCR_helper_result_is_success(unsigned long dwScrResult)
{
	if ((LPU237_DLL_SCR_RESULT_STATUS_CMD_MASK & dwScrResult) == LPU237_DLL_SCR_RESULT_STATUS_CMD_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

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
BOOL _CALLTYPE_ LPU237_SCR_helper_SetParameters(HANDLE hDev, const unsigned char* sAtr, unsigned long nAtr)
{
	_mp::clog::get_instance().log_fmt(L" : CAL : LPU237_SCR_helper_SetParameters.\n");

	if (sAtr == NULL) {
		_mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_SetParameters : sAtr == NULL.\n");
		_last_result_code(LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN, true);
		return FALSE;
	}
	//
	gemcore_scr_interface::CGemCoreScr_Helper::CCardInfo card;
	_mp::type_v_buffer vatr(nAtr);
	std::copy(&sAtr[0], &sAtr[nAtr], std::begin(vatr));

	if (!gemcore_scr_interface::CGemCoreScr_Helper::decode_atr(card, vatr)) {
		_mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_SetParameters : fail : decoding ATR.\n");
		_last_result_code(LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN, true);
		return FALSE;
	}

	unsigned long dwResult(0);

	_last_bwi(gemcore_scr_interface::CGemCoreScr_Helper::CCardInfo::default_BWI, true);
	gemcore_scr_interface::CGemCoreScr_Helper::CCardInfo::typeVectorParameters vParameters(card.get_parameters());

	unsigned char cProtocol(0), bmFindexDindex(0), bmTCCKST(0), bGuardTime(0), bWaitingInteger(0), bIFSC(0);

	if (card.get_TA2()) {
		_mp::clog::get_instance().log_fmt(L" : INF : LPU237_SCR_helper_SetParameters :  PRESENT TA2.\n");
		//specific mode....... TD1 must be exsit here.
		cProtocol = vParameters[0].cProtocol;
		bmFindexDindex = (vParameters[0].cFI << 4) | (vParameters[0].cDI & 0x0F);
		bmTCCKST = card.get_conversion();
		bGuardTime = vParameters[0].cN;

		if (vParameters[0].cProtocol == 0) {
			bWaitingInteger = vParameters[0].cWI;
		}
		else if (vParameters[0].cProtocol == 1) {
			bmTCCKST |= vParameters[0].cChckSumType;
			bWaitingInteger = (vParameters[0].cBWI << 4) | (vParameters[0].cCWI & 0xF);
			bIFSC = vParameters[0].cIFSC;
			_last_bwi(vParameters[0].cBWI, true);
		}
		else {
			_mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_SetParameters(unsupport protocol) : FALSE.\n");
			return FALSE;
		}
	}
	else {
		_mp::clog::get_instance().log_fmt(L" : INF : LPU237_SCR_helper_SetParameters : ABSENT TA2.\n");
		//negotiable mode.......
		unsigned char cVal(0);

		if (vParameters.empty()) {// using default value.
			_mp::clog::get_instance().log_fmt(L" : INF : LPU237_SCR_helper_SetParameters : DEFAULT PROTOCOL.\n");

			cProtocol = gemcore_scr_interface::Protocol_T0;
			if (card.get_global_FIDI(&cVal)) {
				bmFindexDindex = cVal;
			}
			else {
				bmFindexDindex = (gemcore_scr_interface::CGemCoreScr_Helper::CCardInfo::default_FI << 4) | (gemcore_scr_interface::CGemCoreScr_Helper::CCardInfo::default_DI & 0x0F);
			}
			bmTCCKST = card.get_conversion();

			if (card.get_global_N(&cVal)) { bGuardTime = cVal; }
			else { bGuardTime = gemcore_scr_interface::CGemCoreScr_Helper::CCardInfo::default_N; }

			if (card.get_specific_WI(&cVal)) { bWaitingInteger = cVal; }
			else { bWaitingInteger = gemcore_scr_interface::CGemCoreScr_Helper::CCardInfo::default_WI; }
		}
		else {//setting for the first protocol
			_mp::clog::get_instance().log_fmt(L" : INF : LPU237_SCR_helper_SetParameters : FIRST PROTOCOL.\n");

			cProtocol = vParameters[0].cProtocol;
			bmFindexDindex = (vParameters[0].cFI << 4) | (vParameters[0].cDI & 0x0F);
			bmTCCKST = card.get_conversion();
			bGuardTime = vParameters[0].cN;

			if (vParameters[0].cProtocol == 0) {
				bWaitingInteger = vParameters[0].cWI;
			}
			else if (vParameters[0].cProtocol == 1) {
				bmTCCKST |= vParameters[0].cChckSumType;
				bWaitingInteger = (vParameters[0].cBWI << 4) | (vParameters[0].cCWI & 0xF);
				bIFSC = vParameters[0].cIFSC;
				_last_bwi(vParameters[0].cBWI, true);
			}
			else {
				_mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_SetParameters(unsupport protocol) : FALSE.\n");
				return FALSE;
			}
		}
	}

	_mp::clog::get_instance().log_fmt(L" : INF : ( Protocol, FI/DI, TCCKST, GuardTime, WaitingInteger, IFSC )=( 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x ).\n", cProtocol, bmFindexDindex, bmTCCKST, bGuardTime, bWaitingInteger, bIFSC);

	dwResult = LPU237_SCR_bypass_SetParameters(
		hDev,
		cProtocol,
		bmFindexDindex,
		bmTCCKST,
		bGuardTime,
		bWaitingInteger,
		bIFSC);

	BOOL bResult = LPU237_SCR_helper_result_is_success(dwResult);

	if (bResult) { _mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_SetParameters : TRUE.\n"); }
	else { _mp::clog::get_instance().log_fmt(L" : RET : LPU237_SCR_helper_SetParameters : FALSE.\n"); }

	_last_result_code(dwResult, true);
	return bResult;
}

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
BOOL _CALLTYPE_ LPU237_SCR_helper_GetATRParameterString_w(wchar_t* sOut, unsigned long* lpnOut, const unsigned char* sAtr, unsigned long nAtr, wchar_t cDelimiter)
{
	if (sAtr == NULL || lpnOut == NULL)
		return FALSE;
	//
	gemcore_scr_interface::CGemCoreScr_Helper::CCardInfo card;
	_mp::type_v_buffer vatr(nAtr);
	std::copy(&sAtr[0], &sAtr[nAtr], std::begin(vatr));

	if (!gemcore_scr_interface::CGemCoreScr_Helper::decode_atr(card, vatr)) {
		return FALSE;
	}

	std::wstring sreport = gemcore_scr_interface::CGemCoreScr_Helper::get_atr_report(card, static_cast<wchar_t>(cDelimiter));

	//
	if (sOut) {
		if (*lpnOut < sreport.size()) {
			return FALSE;
		}

		std::fill(sOut, &sOut[*lpnOut], 0);
		std::copy(std::begin(sreport), std::end(sreport), sOut);
	}
	*lpnOut = (unsigned long)sreport.size();
	return TRUE;
}

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
BOOL _CALLTYPE_ LPU237_SCR_helper_GetATRParameterString_a(char* sOut, unsigned long* lpnOut, const unsigned char* sAtr, unsigned long nAtr, char cDelimiter)
{
	if (sAtr == NULL || lpnOut == NULL)
		return FALSE;
	//
	gemcore_scr_interface::CGemCoreScr_Helper::CCardInfo card;
	_mp::type_v_buffer vatr(nAtr);
	std::copy(&sAtr[0], &sAtr[nAtr], std::begin(vatr));

	if (!gemcore_scr_interface::CGemCoreScr_Helper::decode_atr(card, vatr)) {
		return FALSE;
	}

	std::wstring sreport = gemcore_scr_interface::CGemCoreScr_Helper::get_atr_report(card, static_cast<wchar_t>(cDelimiter));

	//
	if (sOut) {
		if (*lpnOut < sreport.size()) {
			return FALSE;
		}

		std::fill(sOut, &sOut[*lpnOut], 0);

		for (size_t i = 0; i < sreport.size(); ++i) {
			sOut[i] = static_cast<char>(sreport[i]);
		}//
	}

	*lpnOut = (unsigned long)sreport.size();
	return TRUE;
}
