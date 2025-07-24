// tp_wi_lpu237.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <thread>
#include <iostream>
#include <atomic>
#include <utility>

#include "clpu237_msr.h"
#include "tp_main.h"

static std::atomic_bool gb_need_resetup(false);

static void _CALLTYPE_ cb_msr(void* p_user)
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

static std::tuple<bool, HANDLE, std::wstring> _get_list_open_enable(clpu237_msr& lib, const std::wstring &s_dev)
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
	}while(false);

	if (!b_result && b_need_close) {
		if (!lib.LPU237_close(h_dev)) {
			std::wcout << L" : E : _fun_close_first_device : ." << std::endl;
		}
		else {
			std::wcout << L" : I : _fun_close_first_device." << std::endl;
		}

	}
	return std::make_tuple(b_result, h_dev,s_path);
}

int main_lpu237_dll(const std::set<std::wstring>& set_parameters)
{
    std::wcout << L"Hello World msr!\n";

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

		if (!lib.load(dll_or_so)) {
			std::wcout << L" : E : load : tg_lpu237_dll.dll." << std::endl;
			continue;
		}
		std::wcout << L" : I : load : tg_lpu237_dll.dll." << std::endl;

		if (!lib.LPU237_dll_on()) {
			std::wcout << L" : E : LPU237_dll_on : ." << std::endl;
			continue;
		}
		std::wcout << L" : I : LPU237_dll_on." << std::endl;
				

		std::tie(b_result, h_dev, s_path)=_get_list_open_enable(lib, s_path);
		if (!b_result) {
			continue;
		}
		b_need_close = true;
		
		unsigned long n_buffer_index(LPU237_DLL_RESULT_ERROR);
		int n_loop = 1000;
		int n = 1;

		std::atomic_bool b_complete(false);

		do {
			if (gb_need_resetup) {
				//system error resetup need.

				std::wcout << L" : I : RESETUP :LPU237_close." << std::endl;
				lib.LPU237_close(h_dev);

				std::tie(b_result, h_dev, s_path) = _get_list_open_enable(lib, s_path);
				if (!b_result) {
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					std::wcout << L" : I : FAIL : RESETUP." << std::endl;
					break;//recover fail
					//continue;//retry recovering
				}
				gb_need_resetup = false;
			}

			b_complete = false;
			std::tie(b_result, n_buffer_index ) = lib.LPU237_wait_swipe_with_callback(h_dev, cb_msr, &b_complete);
			if (!b_result) {
				std::wcout << L" : E : LPU237_wait_swipe_with_callback : ." << std::endl;
				break;
			}
			lib.push_buffer_index(n_buffer_index);
			std::wcout << L"READ MSR" << n++ << L" : " << std::endl;

			int n_max = 10;
			int n_progress = 0;
			bool b_inc(true);

			while (!b_complete) {
				if (b_inc) {
					if (n_progress < n_max) {
						std::wcout << L'+';
						n_progress++;
					}

					if (n_progress == n_max) {
						b_inc = false;
						n_progress = 0;
					}
				}
				else {
					if (n_progress < n_max) {
						std::wcout << L"\b \b";
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
