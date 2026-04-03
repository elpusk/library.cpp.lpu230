#include <thread>
#include <iostream>
#include <atomic>
#include <utility>
#include <cstdint> // For uintptr_t
#include <algorithm>

#include "cwait.h"
#include "tp_main.h"
#include "clpu237_fw.h"

static _tp_lpu237::cwait g_wait_complete;

static unsigned long _CALLTYPE_ _cbUpdate(void* pPara, unsigned long dw_result, unsigned long dw_wparam)
{
	std::atomic_bool* p_b_complete = (std::atomic_bool*)pPara;
	//
	if (dw_result == LPU237_FW_RESULT_SUCCESS) {
		switch (dw_wparam) {
		case LPU237_FW_WPARAM_FOUND_BL:
			std::wcout << L" : I : found bootloader." << std::endl;
			break;
		case LPU237_FW_WPARAM_SECTOR_ERASE:
			std::wcout << L" : I : erease secotor." << std::endl;
			break;
		case LPU237_FW_WPARAM_SECTOR_WRITE:
			std::wcout << L" : I : write & verify secotor." << std::endl;
			break;
		case LPU237_FW_WPARAM_COMPLETE:
			std::wcout << L" : I : complete." << std::endl;
			if (p_b_complete) {
				*p_b_complete = true;
			}
			break;
		default:
			break;
		}//end switch
	}
	else if (dw_result == LPU237_FW_RESULT_CANCEL) {
		//cancel
		std::wcout << L" : W : canceled update OMG." << std::endl;

		if (p_b_complete) {
			*p_b_complete = true;
		}
	}
	else {
		//error
		std::wcout << L" : E :  update Error" << std::endl;

		if (p_b_complete) {
			*p_b_complete = true;
		}
	}
	return 0;
}


static std::wstring _get_hex_string_from_vector(const std::vector<unsigned char>& v)
{
	std::wstring s;
	for (auto item : v) {
		wchar_t buffer[3] = { 0 };
		swprintf(buffer, sizeof(buffer) / sizeof(wchar_t), L"%02X", item);
		s += buffer;
	}
	return s;
}

static std::wstring _get_string_from_vector_name(const std::vector<unsigned char>& v)
{
	std::wstring s;
	for (auto item : v) {
		if (item == 0 || item == ' ' ) {
			break;
		}
		s += (wchar_t)item;
	}
	return s;
}

/**
* @brief Get version string from vector of unsigned char.
* @param v : vector of unsigned char for version. (for example, {1, 2, 3} -> "1.2.3.")
*/
static std::wstring _get_string_from_vector_version(const std::vector<unsigned char>& v)
{
	std::wstring s;
	for (auto item : v) {
		s += std::to_wstring((unsigned long)item);
		s += L'.';
	}
	return s;
}

static std::tuple<bool, HANDLE, std::wstring> _get_list_open_with_print(clpu237_fw& lib, const std::wstring& s_dev)

{
	HANDLE h_dev(NULL);
	bool b_result(false);
	bool b_need_close(false);

	std::list<std::wstring> list_dev;
	std::wstring s_path;

	do {

		if (s_dev.empty()) {
			if (!lib.LPU237_fw_get_list(list_dev)) {
				std::wcout << L" : E : LPU237_fw_get_list : none device." << std::endl;
				continue;
			}
			std::wcout << L" : I : LPU237_fw_get_list." << std::endl;

			if (list_dev.empty()) {
				std::wcout << L" : I : None device." << std::endl;
				continue;
			}

			for (auto item : list_dev) {
				std::wcout << L" : I : found : " << item << std::endl;
			}// end for

			std::wcout << L" : I : selected device : " << *(list_dev.begin()) << std::endl;
			s_path = *(list_dev.begin());
		}
		else {
			s_path = s_dev;
		}



		if (!lib.LPU237_fw_open(s_path, h_dev)) {
			std::wcout << L" : E : _fun_open_first_device : ." << std::endl;
			continue;
		}
		std::wcout << L" : I : _fun_open_first_device." << std::endl;
		b_need_close = true;

		b_result = true;
	} while (false);

	if (!b_result && b_need_close) {
		if (!lib.LPU237_fw_close(h_dev)) {
			std::wcout << L" : E : _fun_close_first_device : ." << std::endl;
		}
		else {
			std::wcout << L" : I : _fun_close_first_device." << std::endl;
		}

	}
	return std::make_tuple(b_result, h_dev, s_path);
}

int main_lpu237_fw(const std::list<std::wstring>& list_parameters)
{
	std::wcout << L"Hello World lpu23x fw update test!\n";

#ifdef _WIN32
	std::wstring dll_or_so(L".\\tg_lpu237_fw.dll");

#else
#ifdef _DEBUG
	std::wstring dll_or_so(L"/home/tester/projects/li_lpu237_fw/bin/x64/Debug/libtg_lpu237_fw.so");
#else
	std::wstring dll_or_so(L"/usr/share/elpusk/program/00000006/coffee_manager/so/libtg_lpu237_fw.so");
#endif //_DEBUG

#endif // _WIN32

	bool b_result(false);
	std::wstring s_path;
	bool b_need_close(false);
	bool b_need_off(false);
	HANDLE h_dev(NULL);

	std::atomic_bool b_complete(false);

	clpu237_fw& lib(clpu237_fw::get_instance());

	do {
		
		std::list<std::wstring>::const_iterator it = std::find(list_parameters.begin(), list_parameters.end(), L"/file");
		if (it == std::end(list_parameters)) {
			std::wcout << L"error : /file option must be set." << std::endl;
			continue;
		}

		++it;
		if (it == std::end(list_parameters)) {
			std::wcout << L"error : rom file must be set." << std::endl;
			continue;
		}

		std::wstring s_rom_file = *it;
		//
		int n_fw_index = -1;
		it = std::find(list_parameters.begin(), list_parameters.end(), L"/firmware_index");
		if (it != std::end(list_parameters)) {
			++it;
			if (it != std::end(list_parameters)) {
				try {
					n_fw_index = std::stoi(*it);
				}
				catch (const std::exception& e) {
					std::wcout << L"error : invalid firmware index." << std::endl;
					continue;
				}
			}
			else {
				std::wcout << L"error : firmware index must be set in /firmware_index option." << std::endl;
				continue;
			}
		}
		//
		

		std::wcout << L" : I : loading : " << dll_or_so << std::endl;
		if (!lib.load(dll_or_so)) {
			std::wcout << L" : E : load : " << dll_or_so << std::endl;
			continue;
		}
		std::wcout << L" : I : load : " << dll_or_so << std::endl;

		if(!lib.LPU237_fw_dll_on()) {
			std::wcout << L" : E : LPU237_fw_dll_on : ." << std::endl;
			continue;
		}
		std::wcout << L" : I : LPU237_fw_dll_on." << std::endl;
		b_need_off = true;

		std::tie(b_result, h_dev, s_path) = _get_list_open_with_print(lib, s_path);
		if (!b_result) {
			continue;
		}
		b_need_close = true;

		auto vId = lib.LPU237_fw_msr_get_id(h_dev);
		auto vName = lib.LPU237_fw_msr_get_name(h_dev);
		auto vVersion = lib.LPU237_fw_msr_get_version(h_dev);

		if(vId.empty()) {
			std::wcout << L" : E : get device ID." << std::endl;
			continue;
		}
		std::wcout << L" : I : ID : " << _get_hex_string_from_vector(vId) << std::endl;

		if (vName.empty()) {
			std::wcout << L" : E : get device name." << std::endl;
			continue;
		}
		std::wcout << L" : I : NAME : " << _get_string_from_vector_name(vName) << std::endl;

		if (vVersion.empty()) {
			std::wcout << L" : E : get device version." << std::endl;
			continue;
		}
		std::wcout << L" : I : ID : " << _get_string_from_vector_version(vVersion) << std::endl;

		if(lib.LPU237_fw_rom_load(s_rom_file)) {
			std::wcout << L" : I : LPU237_fw_rom_load : " << s_rom_file << std::endl;
		}
		else {
			std::wcout << L" : E : LPU237_fw_rom_load : " << s_rom_file << std::endl;
			continue;
		}

		auto pair_result_fw_index = lib.LPU237_fw_rom_get_index(s_rom_file, vName, vVersion);
		if (!pair_result_fw_index.first) {
			std::wcout << L" : E : LPU237_fw_rom_get_index : " << s_rom_file << std::endl;
			continue;
		}
		std::wcout << L" : I : LPU237_fw_rom_get_index : " << s_rom_file << L" : index : " << pair_result_fw_index.second << std::endl;

		lib.LPU237_fw_msr_save_setting(h_dev);// for old compatibility, but it is not necessary to call this function before update.
		std::wcout << L" : I : LPU237_fw_msr_save_setting." << std::endl;

		b_complete = false;

		auto pair_result_result_index = lib.LPU237_fw_msr_update_callback(vId, _cbUpdate, &b_complete, s_rom_file, pair_result_fw_index.second);
		if (!pair_result_result_index.first) {
			std::wcout << L" : E : LPU237_fw_msr_update_callback : " << s_rom_file << std::endl;
			continue;
		}
		std::wcout << L" : I : Started LPU237_fw_msr_update_callback : " << s_rom_file << std::endl;
		std::wcout << L" : I : the given result index : " << pair_result_result_index.second << std::endl;

		int n_wait_cnt = 0;
		while (!b_complete) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			std::wcout << n_wait_cnt;
			++n_wait_cnt;
			if (n_wait_cnt > 10) {
				n_wait_cnt = 0;
				std::wcout << std::endl;
			}
		}
		//
		lib.LPU237_msr_recover_setting(h_dev);// for old compatibility, but it is not necessary to call this function before update.

	} while (false);

	if (b_need_close) {
		if (!lib.LPU237_fw_close(h_dev)) {
			std::wcout << L" : E : _fun_close_first_device : ." << std::endl;
		}
		else {
			std::wcout << L" : I : _fun_close_first_device." << std::endl;
		}
	}
	lib.LPU237_dll_off();
	std::wcout << L" : I : LPU237_dll_off." << std::endl;

	return EXIT_SUCCESS;
}