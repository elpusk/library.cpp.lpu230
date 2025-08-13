// tp_wi_lpu237.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <thread>
#include <iostream>
#include <atomic>
#include <utility>
#include <cstdint> // For uintptr_t

#include "cwait.h"
#include "clpu237_msr.h"
#include "tp_main.h"

static std::atomic_bool gb_need_resetup(false);

static _tp_lpu237::cwait g_wait_complete;

static std::tuple<bool, HANDLE, std::wstring> _get_list_open_enable_with_print(clpu237_msr& lib, const std::wstring& s_dev)
{
	HANDLE h_dev(NULL);
	bool b_result(false);
	bool b_need_close(false);

	std::list<std::wstring> list_dev;
	std::wstring s_path;

	do {

		if (s_dev.empty()) {
			if (!lib.LPU237_get_list(list_dev)) {
				std::wcout << L" : E : LPU237_get_list : none device." << std::endl;
				continue;
			}
			std::wcout << L" : I : LPU237_get_list." << std::endl;

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



		if (!lib.LPU237_open(s_path, h_dev)) {
			std::wcout << L" : E : _fun_open_first_device : ." << std::endl;
			continue;
		}
		std::wcout << L" : I : _fun_open_first_device." << std::endl;
		b_need_close = true;

		if (!lib.LPU237_enable(h_dev)) {
			std::wcout << L" : E : LPU237_enable : ." << std::endl;
			continue;
		}

		b_result = true;
	} while (false);

	if (!b_result && b_need_close) {
		if (!lib.LPU237_close(h_dev)) {
			std::wcout << L" : E : _fun_close_first_device : ." << std::endl;
		}
		else {
			std::wcout << L" : I : _fun_close_first_device." << std::endl;
		}

	}
	return std::make_tuple(b_result, h_dev, s_path);
}

static void _CALLTYPE_ _cb_msr_with_thread_safty(void* p_user)
{
	uintptr_t u_evt_read = reinterpret_cast<uintptr_t>(p_user);
	int n_evt_read = (int)(u_evt_read);

	g_wait_complete.set(n_evt_read);
}

/**
* @brief reading msr with thread safety.
*	This function will read msr data with thread safety.
* @param h_dev - device handle.
* @param n_evt_read - event index for reading msr data.
* @param n_evt_end - event index for ending the program.
* @param n_cnt_max_read - maximum count of reading msr data.
* @param n_cnt_cur_read - current count of reading msr data.
* @return true if you can call this function, false else.
* 
*/
static bool _reading_msr_with_thread_safety(
	HANDLE h_dev,int n_evt_read, int n_evt_end
	, int n_cnt_max_read, int n_cnt_cur_read
)
{
	clpu237_msr& lib(clpu237_msr::get_instance());

	bool b_can_be_repeated(false);

	unsigned long n_buffer_index(LPU237_DLL_RESULT_ERROR);
	bool b_result(false);

	do {
		std::tie(b_result, n_buffer_index) = lib.LPU237_wait_swipe_with_callback(h_dev, _cb_msr_with_thread_safty, (void*)n_evt_read);
		if (!b_result) {
			std::wcout << L" : E : LPU237_wait_swipe_with_callback : ." << std::endl;
			continue;
		}
		if (n_buffer_index == LPU237_DLL_RESULT_ERROR) {
			std::wcout << L" : E : LPU237_wait_swipe_with_callback : error buffer index." << std::endl;
			continue;
		}
		b_can_be_repeated = true;

		std::wcout << L" : I : Read card : " << n_cnt_cur_read << L'/' << n_cnt_max_read << std::endl;
		int n_wait_result = g_wait_complete.wait_for_one_at_time();
		if (n_wait_result == _tp_lpu237::cwait::const_event_timeout) {
			std::wcout << L" : E : LPU237_wait_swipe_with_callback : time out." << std::endl;
			continue;
		}
		if (n_wait_result == n_evt_end) {
			std::wcout << L" : I : program will be terminated." << std::endl;
			continue;
		}
		if (n_wait_result != n_evt_read) {
			std::wcout << L" : E : LPU237_wait_swipe_with_callback : unexpected event index." << std::endl;
			continue;
		}

		g_wait_complete.reset(n_evt_read);

		b_can_be_repeated = true;

		std::array<clpu237_msr::type_result_read, 3> ar_result{ clpu237_msr::rr_error,clpu237_msr::rr_error ,clpu237_msr::rr_error };
		std::array<std::wstring, 3> ar_s_iso{ L"",L"" ,L"" };
		std::tie(b_result, ar_result) = lib.LPU237_get_data(n_buffer_index, ar_s_iso);
		for (int i = 0; i < ar_result.size(); i++) {
			if (ar_result[i] == clpu237_msr::rr_success) {
				if (ar_s_iso[i].empty()) {
					std::wcout << L"[S] iso" << i + 1 << L" is none." << std::endl;
				}
				else {
					std::wcout << L"[S] iso" << i + 1 << L" : " << ar_s_iso[i] << std::endl;
				}
			}
			else {
				std::wcout << L"[E] iso" << i + 1 << L" : " << clpu237_msr::get_result_string(ar_result[i]) << std::endl;
			}
		}//end for

	} while (false);

	return b_can_be_repeated;
}


int main_lpu237_dll_threadsafty(const std::set<std::wstring>& set_parameters)
{
	std::wcout << L"Hello World msr with thread safty style!\n";

	bool b_need_close(false);
	bool b_need_off(false);
	HANDLE h_dev(NULL);

#ifdef _WIN32
	std::wstring dll_or_so(L".\\tg_lpu237_dll.dll");

#else
#ifdef _DEBUG
	std::wstring dll_or_so(L"/home/tester/projects/li_lpu237_dll/bin/x64/Debug/libtg_lpu237_dll.so");
#else
	std::wstring dll_or_so(L"/usr/share/elpusk/program/00000006/coffee_manager/so/libtg_lpu237_dll.so");
#endif //_DEBUG

#endif // _WIN32

	clpu237_msr& lib(clpu237_msr::get_instance());
	bool b_result(false);
	std::wstring s_path;

	do {

		std::wcout << L" : I : loading : " << dll_or_so << std::endl;
		if (!lib.load(dll_or_so)) {
			std::wcout << L" : E : load : " << dll_or_so << std::endl;
			continue;
		}
		std::wcout << L" : I : load : " << dll_or_so << std::endl;

		if (!lib.LPU237_dll_on()) {
			std::wcout << L" : E : LPU237_dll_on : ." << std::endl;
			continue;
		}
		std::wcout << L" : I : LPU237_dll_on." << std::endl;
		b_need_off = true;

		std::tie(b_result, h_dev, s_path) = _get_list_open_enable_with_print(lib, s_path);
		if (!b_result) {
			continue;
		}
		b_need_close = true;

		unsigned long n_buffer_index(LPU237_DLL_RESULT_ERROR);
		int n_max_loop = 1000;
		int n_loop = n_max_loop;
		int n = 1;
		
		
		int n_evt_read = g_wait_complete.generate_new_event();
		int n_evt_end = g_wait_complete.generate_new_event();
		int n_cnt_max = 10;

		std::thread th_reading_msr(
			[=](HANDLE _h_dev, int _n_evt_read, int _n_evt_end, int n_cnt_max_read) {
				bool b_i_can_repeat(false);
				int n_cnt_cur_read = 0;
				do {
					b_i_can_repeat = _reading_msr_with_thread_safety(
						_h_dev, _n_evt_read
						, _n_evt_end,n_cnt_max_read, ++n_cnt_cur_read
					);
				} while (b_i_can_repeat && n_cnt_cur_read< n_cnt_max_read);
			}
			,h_dev, n_evt_read, n_evt_end, n_cnt_max
		);

		if (th_reading_msr.joinable()) {
			th_reading_msr.join();
			std::wcout << L" : I : th_reading_msr.join." << std::endl;
		}
		if (!lib.LPU237_disable(h_dev)) {
			std::wcout << L" : E : LPU237_disable : ." << std::endl;
			continue;
		}

		std::wcout << L" : I : LPU237_disable." << std::endl;
	} while (false);

	if (b_need_close) {
		if (!lib.LPU237_close(h_dev)) {
			std::wcout << L" : E : _fun_close_first_device : ." << std::endl;
		}
		else {
			std::wcout << L" : I : _fun_close_first_device." << std::endl;
		}
	}

	if (b_need_off) {
		lib.LPU237_dll_off();
		std::wcout << L" : I : LPU237_dll_off." << std::endl;
	}

	return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////
//

static void _CALLTYPE_ _cb_msr_with_calling_get_data(void* p_user)
{
	std::atomic_bool* p_b_complete = (std::atomic_bool*)p_user;
	
	clpu237_msr& lib(clpu237_msr::get_instance());

	bool b_result(false);
	unsigned long n_index(0xFFFFFFFF);

	do {
		std::wcout << std::endl;

		std::tie(b_result, n_index) = lib.pop_buffer_index();
		if (!b_result) {
			std::wcout << L"[E] None buffer index" << std::endl;
			continue;
		}

		std::array<clpu237_msr::type_result_read, 3> ar_result{ clpu237_msr::rr_error,clpu237_msr::rr_error ,clpu237_msr::rr_error };
		std::array<std::wstring, 3> ar_s_iso{ L"",L"" ,L"" };

		std::tie(b_result, ar_result) = lib.LPU237_get_data(n_index, ar_s_iso);
		if (!b_result) {
			std::wcout << L"[E] reading system error" << std::endl;
			gb_need_resetup = true;
			continue;
		}

		for (int i = 0; i < ar_result.size(); i++) {
			if (ar_result[i] == clpu237_msr::rr_success) {
				if (ar_s_iso[i].empty()) {
					std::wcout << L"[S] iso" << i + 1 << L" is none." << std::endl;
				}
				else {
					std::wcout << L"[S] iso" << i + 1 << L" : " << ar_s_iso[i] << std::endl;
				}
			}
			else {
				std::wcout << L"[E] iso" << i + 1 << L" : " << clpu237_msr::get_result_string(ar_result[i]) << std::endl;
			}
		}//end for

	} while (false);

	if (p_b_complete) {
		*p_b_complete = true;
	}
}


int main_lpu237_dll_getdataincallback(const std::set<std::wstring>& set_parameters)
{
	std::wcout << L"Hello World msr with get_data in callback style!\n";

	bool b_need_close(false);
	HANDLE h_dev(NULL);

#ifdef _WIN32
	std::wstring dll_or_so(L".\\tg_lpu237_dll.dll");

#else
#ifdef _DEBUG
	std::wstring dll_or_so(L"/home/tester/projects/li_lpu237_dll/bin/x64/Debug/libtg_lpu237_dll.so");
#else
	std::wstring dll_or_so(L"/usr/share/elpusk/program/00000006/coffee_manager/so/libtg_lpu237_dll.so");
#endif //_DEBUG

#endif // _WIN32

	clpu237_msr& lib(clpu237_msr::get_instance());
	bool b_result(false);
	std::wstring s_path;

	do {

		std::wcout << L" : I : loading : " << dll_or_so << std::endl;
		if (!lib.load(dll_or_so)) {
			std::wcout << L" : E : load : "<< dll_or_so << std::endl;
			continue;
		}
		std::wcout << L" : I : load : " << dll_or_so << std::endl;

		if (!lib.LPU237_dll_on()) {
			std::wcout << L" : E : LPU237_dll_on : ." << std::endl;
			continue;
		}
		std::wcout << L" : I : LPU237_dll_on." << std::endl;
				

		std::tie(b_result, h_dev, s_path)=_get_list_open_enable_with_print(lib, s_path);
		if (!b_result) {
			continue;
		}
		b_need_close = true;
		
		unsigned long n_buffer_index(LPU237_DLL_RESULT_ERROR);
		int n_max_loop = 1000;
		int n_loop = n_max_loop;
		int n = 1;

		std::atomic_bool b_complete(false);

		do {
			if (gb_need_resetup) {
				//system error resetup need.

				std::wcout << L" : I : RESETUP :LPU237_close." << std::endl;
				lib.LPU237_close(h_dev);

				std::tie(b_result, h_dev, s_path) = _get_list_open_enable_with_print(lib, s_path);
				if (!b_result) {
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					std::wcout << L" : I : FAIL : RESETUP." << std::endl;
					break;//recover fail
					//continue;//retry recovering
				}
				gb_need_resetup = false;
			}

			b_complete = false;
			std::tie(b_result, n_buffer_index ) = lib.LPU237_wait_swipe_with_callback(h_dev, _cb_msr_with_calling_get_data, &b_complete);
			if (!b_result) {
				std::wcout << L" : E : LPU237_wait_swipe_with_callback : ." << std::endl;
				break;
			}
			lib.push_buffer_index(n_buffer_index);
			std::wcout << L"READ MSR" << n++ << L"/" << n_max_loop << L" : " << std::endl;

			int n_max = 10;
			int n_progress = 0;
			bool b_inc(true);

			while (!b_complete) {
				if (b_inc) {
					if (n_progress < n_max) {
						std::wcout << L'+' << std::flush;
						n_progress++;
					}

					if (n_progress == n_max) {
						b_inc = false;
						n_progress = 0;
					}
				}
				else {
					if (n_progress < n_max) {
						std::wcout << L"\b \b" << std::flush;
						n_progress++;
					}

					if (n_progress == n_max) {
						b_inc = true;
						n_progress = 0;
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}//the end of while

			//
			--n_loop;
		} while (n_loop > 0);

		if (!lib.LPU237_disable(h_dev)) {
			std::wcout << L" : E : LPU237_disable : ." << std::endl;
			continue;
		}

	}while(false);

	if (b_need_close) {
		if (!lib.LPU237_close(h_dev)) {
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
