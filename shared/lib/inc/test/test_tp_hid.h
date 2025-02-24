#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <array>
#include <algorithm>

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

#include <_dev/mp_cdev_util.h>
#include <mp_cconvert.h>
#include <mp_cwait.h>
#include <mp_cfile.h>
#include <hid/mp_clibhid.h>
#include <mp_clog.h>
#include <server/mp_cserver_.h>
#include <cert/mp_ccoffee_hlp_cert_.h>
#include <mp_cconsole.h>
#include <mp_cnamed_pipe_.h>

namespace _test{
    class tp_hid{
        public:
			static int wss_serving()
			{
				int n_result(0);
#ifdef _WIN32
				std::wstring s_certificate_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.crt";
				std::wstring s_private_key_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.key";
				std::wstring s_root_folder_except_backslash = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\win\\ProgramData\\elpusk\\00000006\\vroot";
				std::wstring s_log_folder_except_backslash = L"C:\\ProgramData\\Elpusk\\00000006\\elpusk-hid-d\\log";
#else
				std::wstring s_certificate_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt";
				std::wstring s_private_key_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key";
				std::wstring s_root_folder_except_backslash = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/vroot";
				std::wstring s_log_folder_except_backslash = L"/home/tester/fordebug/var/log/elpusk/00000006/elpusk-hid-d";
#endif

				/**
				* setup log
				*/
				_mp::clog::get_instance().config(s_root_folder_except_backslash, 6);
				_mp::clog::get_instance().remove_log_files_older_then_now_day(7);
				_mp::clog::get_instance().enable(true);
				_mp::clog::get_instance().Trace(L"TEST OK.");

				bool b_debug = true;
				bool b_tls = true;
				unsigned short w_port = _mp::_ws_tools::WEBSOCKET_SECURITY_SERVER_PORT_COFFEE_MANAGER;

				int n_thread_for_server(1);

				do {
					_mp::cserver::get_instance(&_mp::clog::get_instance()).set_port(w_port)
						.set_ssl(b_tls)
						.set_cert_file(s_certificate_file)
						.set_private_key_file(s_private_key_file);
					if (!_mp::cserver::get_instance().start(n_thread_for_server, s_root_folder_except_backslash)) {
						_mp::clog::get_instance().Trace(L"[E] %ls | cserver::get_instance().start().\n", __WFUNCTION__);
						continue;
					}

					std::wcout << L"started server." << std::endl;
					wchar_t c = NULL;

					_mp::cnamed_pipe::type_ptr ptr_np;
					std::wstring s_name = L"NAME_PIPE_2024.07.25_";
					int n_cnt = 0;
					std::wstring s_tx;

					do {
						std::wcout << L"press x. for existing\n";
						std::wcin >> c;

					} while (c != L'x');

					_mp::cserver::get_instance().stop();
				} while (false);

				return n_result;
			}
			static int test1()
			{
				_mp::cwait waiter;
				int n_event = waiter.generate_new_event();
				std::pair<_mp::cwait*, int> pair_p(&waiter, n_event);

				do {
					_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
					if (!lib_hid.is_ini()) {
						std::wcout << L"ERROR\n";
						continue;
					}
					std::wcout << L"Lib started.\n";
					for (int i = 5; i >= 0; i--) {
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
						std::wcout << i << L'.' << std::flush;
					}//end for

					std::wcout << std::endl;
					std::wcout << L"Start test." << std::endl;


					auto start = std::chrono::high_resolution_clock::now();

					_mp::clibhid_dev_info::type_set st = lib_hid.get_cur_device_set();
					for (const _mp::clibhid_dev_info& item : st) {
						std::wcout << std::hex << item.get_vendor_id() << L", " << item.get_product_id() << L" : ";
						std::wcout << item.get_path_by_wstring() << std::endl;
						std::wcout << item.get_interface_number() << std::endl;
						//
							//
						_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(item);
						for (int i = 0; i < 1; i++) {
							if (wptr_dev.expired()) {
								continue;
							}
							std::vector<unsigned char> v_tx(1, 0);
							std::vector<unsigned char> v_rx;

							v_tx[0] = 'I';
							wptr_dev.lock()->start_write_read(
								v_tx,
								[](_mp::cqitem_dev& dev, void* p_user) -> bool {
									auto result = dev.get_result_all_by_string();
									std::wcout << std::get<0>(result) << std::endl;
									std::wcout << std::get<1>(result) << std::endl;
									std::wcout << std::get<2>(result) << std::endl;

									std::pair<_mp::cwait*, int>* p_pair_p = (std::pair<_mp::cwait*, int> *)p_user;
									p_pair_p->first->set(p_pair_p->second);
									return true;//complete callback
								}
								, &pair_p
							);
							std::wcout << std::dec << i << L" : start txrx ok." << std::endl;

							waiter.wait_for_at_once();
							std::wcout << std::dec << i << L" : enter opos." << std::endl;

							wptr_dev.lock()->start_read(
								[](_mp::cqitem_dev& dev, void* p_user) -> bool {
									auto result = dev.get_result_all_by_string();
									std::wcout << std::get<0>(result) << std::endl;
									std::wcout << std::get<1>(result) << std::endl;
									std::wcout << std::get<2>(result) << std::endl;

									std::pair<_mp::cwait*, int>* p_pair_p = (std::pair<_mp::cwait*, int> *)p_user;
									p_pair_p->first->set(p_pair_p->second);
									return true;//complete callback
								}
								, &pair_p
							);
							waiter.wait_for_at_once();
							std::wcout << std::dec << i << L" : reading done." << std::endl;

							v_tx[0] = 'J';
							wptr_dev.lock()->start_write_read(
								v_tx,
								[](_mp::cqitem_dev& dev, void* p_user) -> bool {
									auto result = dev.get_result_all_by_string();
									std::wcout << std::get<0>(result) << std::endl;
									std::wcout << std::get<1>(result) << std::endl;
									std::wcout << std::get<2>(result) << std::endl;

									std::pair<_mp::cwait*, int>* p_pair_p = (std::pair<_mp::cwait*, int> *)p_user;
									p_pair_p->first->set(p_pair_p->second);
									return true;//complete callback
								}
								, &pair_p
							);
							std::wcout << std::dec << i << L" : start txrx ok." << std::endl;

							waiter.wait_for_at_once();
							std::wcout << std::dec << i << L" : leave opos." << std::endl;

						}//end for i

						auto end = std::chrono::high_resolution_clock::now();
						std::chrono::duration<double> elapsed = end - start;
						std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;

					}//end for

				} while (false);

				return 0;
			}

			static bool _cb_tx_rx(_mp::cqitem_dev& item, void* p_user)
			{
				bool b_complete(true);
				_mp::cqitem_dev::type_result r;
				_mp::type_v_buffer v;
				std::wstring s;

				do {
					std::tie(r, v, s) = item.get_result_all();
					switch (r) {
					case _mp::cqitem_dev::result_not_yet:
						b_complete = false;
						_mp::clog::Trace(L" ++ _cb_tx_rx : result_not_yet.\n");
						continue;//more processing
					case _mp::cqitem_dev::result_success:
						_mp::clog::Trace(L" ++ _cb_tx_rx : result_success.\n");
						_mp::clog::Trace(L" ++ %u : %02x,%02x,%02x.\n",v.size(),v[0],v[1],v[2]);
						break;
					case _mp::cqitem_dev::result_error:
						_mp::clog::Trace(L" ++ _cb_tx_rx : result_error.\n");
						break;
					case _mp::cqitem_dev::result_cancel:
						_mp::clog::Trace(L" ++ _cb_tx_rx : result_cancel.\n");
						break;
					default:
						_mp::clog::Trace(L" ++ _cb_tx_rx : unknown.\n");
						break;
					}//end switch
				} while (false);

				return b_complete;
			}
			static int test2()
			{
				_mp::cwait waiter;
				int n_event = waiter.generate_new_event();
				std::pair<_mp::cwait*, int> pair_p(&waiter, n_event);

				do {
					_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
					if (!lib_hid.is_ini()) {
						std::wcout << L"ERROR\n";
						continue;
					}
					std::wcout << L"Lib started.\n";
					for (int i = 3; i >= 0; i--) {
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
						std::wcout << i << L'.' << std::flush;
					}//end for

					std::wcout << std::endl;
					std::wcout << L"Start test." << std::endl;

					for (int j = 0; j < 10; j++) {
						auto start = std::chrono::high_resolution_clock::now();

						_mp::clibhid_dev_info::type_set st = lib_hid.get_cur_device_set();
						for (const _mp::clibhid_dev_info& item : st) {
							std::wcout << std::hex << item.get_vendor_id() << L", " << item.get_product_id() << L" : ";
							std::wcout << item.get_path_by_wstring() << std::endl;
							std::wcout << item.get_interface_number() << std::endl;
							//
								//
							_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(item);
							if (wptr_dev.expired()) {
								continue;
							}

							auto v_report_desc = wptr_dev.lock()->get_report_desciptor();
							std::wstring s_report_desc;
							_mp::cconvert::hex_string_from_binary(s_report_desc, v_report_desc);
							std::wcout << s_report_desc << std::endl;
							//
							_mp::type_v_buffer v_tx{ 0x49,0x00,0x00 };
							wptr_dev.lock()->start_write_read(v_tx, tp_hid::_cb_tx_rx, nullptr);

						}//end for

						for (int i = 3; i >= 0; i--) {
							std::this_thread::sleep_for(std::chrono::milliseconds(1000));
							std::wcout << i << L'.' << std::flush;
						}//end for

						auto end = std::chrono::high_resolution_clock::now();
						std::chrono::duration<double> elapsed = end - start;
						std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
					}
				} while (false);

				return 0;
			}

			static bool _cb_rx(_mp::cqitem_dev& item, void* p_user)
			{
				bool b_complete(true);
				_mp::cqitem_dev::type_result r;
				_mp::type_v_buffer v;
				std::wstring s;

				std::pair<_mp::cwait*, int>* p_pair = (std::pair<_mp::cwait*, int>*)p_user;

				do {
					std::tie(r, v, s) = item.get_result_all();
					switch (r) {
					case _mp::cqitem_dev::result_not_yet:
						b_complete = false;
						printf(" ++ _cb_rx : result_not_yet.\n");
						continue;//more processing
					case _mp::cqitem_dev::result_success:
						printf(" ++ _cb_rx : result_success.\n");
						printf(" ++ %u : %02x,%02x,%02x.\n", (unsigned int)v.size(), v[0], v[1], v[2]);
						break;
					case _mp::cqitem_dev::result_error:
						printf(" ++ _cb_rx : result_error.\n");
						break;
					case _mp::cqitem_dev::result_cancel:
						printf(" ++ _cb_rx : result_cancel.\n");
						break;
					default:
						printf(" ++ _cb_rx : unknown.\n");
						break;
					}//end switch
				} while (false);

				fflush(stdout);

				p_pair->first->set(p_pair->second);
				return b_complete;
			}
			/**
			* rx 1000 times
			*/
			static int test2_1()
			{
				_mp::cwait waiter;
				int n_event = waiter.generate_new_event();
				std::pair<_mp::cwait*, int> pair_p_wait_n_event(&waiter, n_event);
#ifndef _WIN32
				setvbuf(stdout, NULL, _IONBF, 0);

#endif // !_WIN32

				do {
					_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
					if (!lib_hid.is_ini()) {
						std::wcout << L"ERROR\n";
						continue;
					}
					std::wcout << L"Lib started.\n";
					for (int i = 3; i >= 0; i--) {
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
						std::wcout << i << L'.' << std::flush;
					}//end for

					std::wcout << std::endl;
					std::wcout << L"Start test." << std::endl;

					_mp::clibhid_dev_info::type_set st = lib_hid.get_cur_device_set();
					if (st.empty()) {
						std::cout << "none device" << std::endl;
						break;
					}
					else {
						for (auto item : st) {
							std::wcout << std::hex << item.get_vendor_id() << L":";
							std::wcout << std::hex << item.get_product_id() << L":";
							std::wcout << std::hex << item.get_interface_number() << L":";
							std::wcout << item.get_extra_path_by_wstring();
							std::wcout << std::endl;
						}//end for
						std::wcout << std::dec;
					}

					//msr device filtering
					_mp::type_list_wstring list_filter{ L"lpu200",L"msr" };

					size_t n_dev(0);
					_mp::type_set_wstring set_dev_path;
					_mp::type_v_bm_dev v_type;
					bool b_lpu200_filter(false);
					std::wstring s_first, s_second;
					std::tie(b_lpu200_filter, s_first, s_second) = _mp::cdev_util::is_lpu200_filter(list_filter);
					if (b_lpu200_filter) {
						//lpu200 filter
						v_type.clear();
						v_type.push_back(_mp::cdev_util::get_device_type_from_lpu200_filter(s_second));

						//get composite type path
						n_dev = _mp::clibhid_dev_info::filter_dev_info_set(st, st, v_type);//filtering
						set_dev_path = _mp::clibhid_dev_info::get_dev_path_by_wstring(st);
					}


					const _mp::clibhid_dev_info& item(*st.begin());
					std::wcout << std::hex << item.get_vendor_id() << L", " << item.get_product_id() << L" : ";
					std::wcout << item.get_path_by_wstring() << std::endl;
					std::wcout << item.get_interface_number() << std::endl;
					//
					
					for (int i = 0; i < 1000; i++) {
						std::cout << "TEST" << i+1 << std::endl;
						_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(item);
						if (wptr_dev.expired()) {
							std::cout << "device is expired" << std::endl;
							break;
						}

						_mp::type_v_buffer v_tx(64, 0);

						if (i % 2 == 0) {
							v_tx[0] = 'X';
						}
						else {
							v_tx[0] = 'Y';
						}

						pair_p_wait_n_event.first->reset(pair_p_wait_n_event.second);

						wptr_dev.lock()->start_write_read(v_tx, tp_hid::_cb_rx, &pair_p_wait_n_event);
						//wptr_dev.lock()->start_read(tp_hid::_cb_rx, &pair_p_wait_n_event);
						auto start = std::chrono::high_resolution_clock::now();

						int n_evt = _mp::cwait::const_event_timeout;

						int n_point = 0;

						do {
							n_evt = _mp::cwait::const_event_timeout;
							n_evt = pair_p_wait_n_event.first->wait_for_one_at_time(0);
							if (n_evt == pair_p_wait_n_event.second) {
								std::wcout << std::endl;
								std::wcout << L"RX DONE" << std::endl;
								break;
							}
							if (wptr_dev.expired()) {
								std::wcout << std::endl;
								std::cout << "device is expired" << std::endl;
								i = 1000;
								break;
							}

							std::this_thread::sleep_for(std::chrono::milliseconds(1000));
							std::wcout << L".";
							++n_point;
							if (n_point % 25 == 0) {
								std::wcout <<  std::endl;
							}
						} while (true);

						auto end = std::chrono::high_resolution_clock::now();
						std::chrono::duration<double> elapsed = end - start;
						std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
					}
				} while (false);

				return 0;
			}

			static int test3()
			{
#ifdef _WIN32
				_mp::clog::get_instance().config(L"C:\\ProgramData\\Elpusk\\00000006\\elpusk-hid-d\\log", 6);
#else
				_mp::clog::get_instance().config(L"/home/tester/fordebug/var/log/elpusk/00000006/elpusk-hid-d", 6);
#endif
				_mp::clog::get_instance().remove_log_files_older_then_now_day(7);
				_mp::clog::get_instance().enable(true);
				_mp::clog::get_instance().Trace(L"TEST OK.");

				return 0;
			}

			static int test4()
			{
				tp_hid::test3();
				//
#ifdef _WIN32
				std::wstring s_certificate_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.crt";
				std::wstring s_private_key_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.key";
				std::wstring s_root_folder_except_backslash = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\win\\ProgramData\\elpusk\\00000006\\vroot";
#else
				std::wstring s_certificate_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt";
				std::wstring s_private_key_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key";
				std::wstring s_root_folder_except_backslash = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/vroot";
#endif
				bool b_debug = true;
				bool b_tls = true;
				unsigned short w_port = _mp::_ws_tools::WEBSOCKET_SECURITY_SERVER_PORT_COFFEE_MANAGER;

				int n_thread_for_server(1);

				do {
					_mp::cserver::get_instance(&_mp::clog::get_instance()).set_port(w_port)
						.set_ssl(b_tls)
						.set_cert_file(s_certificate_file)
						.set_private_key_file(s_private_key_file);
					if (!_mp::cserver::get_instance().start(n_thread_for_server, s_root_folder_except_backslash)) {
						_mp::clog::get_instance().Trace(L"[E] %ls | cserver::get_instance().start().\n", __WFUNCTION__);
						continue;
					}

					std::wcout << L"started server." << std::endl;
					wchar_t c = NULL;

					_mp::cnamed_pipe::type_ptr ptr_np;
					std::wstring s_name = L"NAME_PIPE_2024.07.25_";
					int n_cnt = 0;
					std::wstring s_tx;

					do {
						std::wcout << L"press x. for existing\n";
						std::wcin >> c;

					} while (c != L'x');

					_mp::cserver::get_instance().stop();
				}while(false);
				return 0;
			}

			/**
			* create & strore certificate. and remove it
			*/
			static int test5(_mp::type_set_wstring& set_parameters)
			{
#ifdef _WIN32
				std::wstring s_server_certificate_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.crt";
				std::wstring s_server_private_key_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.key";
#else
				std::wstring s_server_certificate_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt";
				std::wstring s_server_private_key_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key";
#endif

				do {
					if (set_parameters.empty()) {
						wprintf(L"[E] %ls | no commnad parameters.\n", __WFUNCTION__);
						continue;
					}

					if (set_parameters.find(L"/server") != std::end(set_parameters)) {
						//start server
						tp_hid::test4();
						continue;
					}
					//
					if (set_parameters.find(L"/certificate") != std::end(set_parameters)) {
						//found
						if ([&]()->bool {
							bool b_result(false);

							do {
								std::wstring s_ca_cert_common_name(L"coffee-server-root-ca");

								std::wstring s_server_cert_common_name(L"coffee_server.local");
								std::wstring s_server_cert_organization_name(L"coffee-server-ca");

								std::wstring s_file_server_cert(s_server_certificate_file);
								std::wstring s_file_server_key(s_server_private_key_file);

								if (!_mp::ccoffee_hlp_cert::coffee_hlp_cert_create_and_store_cert_for_coffee_manager(
									s_ca_cert_common_name
									, s_server_cert_common_name
									, s_server_cert_organization_name
									, s_file_server_cert
									, s_file_server_key

								)) {

									continue;
								}

								b_result = true;
							} while (false);
							return b_result;
							}()
							) {
							wprintf(L"[I] %ls | config certificates.\n", __WFUNCTION__);
							std::wcout << L"success : config certificates." << std::endl;
						}
						else {
							wprintf(L"[E] %ls | config certificates.\n", __WFUNCTION__);
							std::wcout << L"fail : config certificates." << std::endl;
						}
						continue;
					}
					//
					if (set_parameters.find(L"/remove") != std::end(set_parameters)) {
						//found
						if ([=]()->bool {
							bool b_result(false);

							do {
								std::wstring s_ca_cert_common_name(L"coffee-server-root-ca");

								std::wstring s_server_cert_common_name(L"coffee_server.local");
								std::wstring s_server_cert_organization_name(L"coffee-server-ca");

								std::wstring s_file_server_cert(s_server_certificate_file);
								std::wstring s_file_server_key(s_server_private_key_file);

								if (!_mp::ccoffee_hlp_cert::coffee_hlp_cert_remove_cert_from_system_root_ca(
									s_ca_cert_common_name,
									s_file_server_cert
								)) {

									continue;
								}

								b_result = true;
							} while (false);
							return b_result;
							}()
							) {
							wprintf(L"[I] %ls | remove certificates.\n", __WFUNCTION__);
							std::wcout << L"success : remove certificates." << std::endl;
						}
						else {
							wprintf(L"[E] %ls | remove certificates.\n", __WFUNCTION__);
							std::wcout << L"fail : remove certificates." << std::endl;
						}
						continue;
					}

					wprintf(L"[E] %ls | unsupported parameters.\n", __WFUNCTION__);
					std::wcout << L"fail : unsupported parameters." << std::endl;
				} while (false);
				return 0;
			}

			static int test_named_pipe()
			{
				tp_hid::test3();
				//
				do {
					wchar_t c = NULL;

					_mp::cnamed_pipe::type_ptr ptr_np;
					std::wstring s_name = L"NAME_PIPE_2024.07.25_";
					int n_cnt = 0;
					std::wstring s_tx;

					do {
						std::wcout << L"press x. for existing\n";
						std::wcout << L"press 1. for create named pipe.\n";
						std::wcout << L"press 2. for write to named pipe.\n";
						std::wcout << L"press 3. for open named pipe.\n";
						std::wcout << L"press 4. for read from named pipe.\n";
						std::wcin >> c;
						if (c == L'1') {
							ptr_np = std::make_shared<_mp::cnamed_pipe>(s_name, true);
						}
						else if (c == L'2') {
							if (ptr_np) {
								s_tx = L"NAME_PIPE";
								s_tx += std::to_wstring(++n_cnt);
								s_tx += L"\n";

								if (ptr_np->write(s_tx)) {
									std::wcout << L"write-OK\n";
								}
								else {
									std::wcout << L"write-ER\n";
								}
							}
						}
						else if (c == L'3') {
							ptr_np = std::make_shared<_mp::cnamed_pipe>(s_name, false);

						}
						else if (c == L'4') {
							std::wstring s_rx;
							if (ptr_np) {
								if (ptr_np->read(s_rx)) {
									std::wcout << L"read-OK:" << s_rx << std::endl;
								}
								else {
									std::wcout << L"read-ER\n";
								}
							}
						}

					} while (c != L'x');
				} while (false);
				return 0;
			}

			static int test6()
			{
				_mp::cwait waiter;
				int n_event = waiter.generate_new_event();
				std::pair<_mp::cwait*, int> pair_p_wait_n_event(&waiter, n_event);
#ifndef _WIN32
				setvbuf(stdout, NULL, _IONBF, 0);

#endif // !_WIN32

				do {
					_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
					if (!lib_hid.is_ini()) {
						std::wcout << L"ERROR\n";
						continue;
					}
					std::wcout << L"Lib started.\n";
					for (int i = 3; i >= 0; i--) {
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
						std::wcout << i << L'.' << std::flush;
					}//end for

					std::wcout << std::endl;
					std::wcout << L"Start test." << std::endl;

					_mp::clibhid_dev_info::type_set st = lib_hid.get_cur_device_set();
					if (st.empty()) {
						std::cout << "none device" << std::endl;
						break;
					}
					else {
						for (auto item : st) {
							std::wcout << std::hex << item.get_vendor_id() << L":";
							std::wcout << std::hex << item.get_product_id() << L":";
							std::wcout << std::hex << item.get_interface_number() << L":";
							std::wcout << item.get_extra_path_by_wstring();
							std::wcout << std::endl;
						}//end for
						std::wcout << std::dec;
					}

					//setup device filtering
					size_t n_dev(0);
					std::array<_mp::type_bm_dev,5> ar_filter = {
						_mp::type_bm_dev_hid,
						_mp::type_bm_dev_lpu200_msr,
						_mp::type_bm_dev_lpu200_scr0,
						_mp::type_bm_dev_lpu200_ibutton,
						_mp::type_bm_dev_lpu200_switch0
					};

					std::array<_mp::type_set_wstring, ar_filter.size()> ar_set_wstring;
					std::array<_mp::clibhid_dev_info::type_set, ar_filter.size()> ar_set_dev_info;
					_mp::type_v_bm_dev v_type;

					for (auto i = 0; i < ar_filter.size(); i++) {
						v_type.clear();
						v_type.push_back(ar_filter[i]);
						//
						_mp::clibhid_dev_info::filter_dev_info_set(ar_set_dev_info[i], st, v_type);//filtering
						ar_set_wstring[i] = _mp::clibhid_dev_info::get_dev_path_by_wstring(ar_set_dev_info[i]);
						//
						std::wcout << std::hex << L"the filtered : 0x" << ar_filter[i] << std::endl;
						for (auto _s_path : ar_set_wstring[i]) {
							std::wcout << L"\t" << _s_path << std::endl;
						}
					}//end for type

					const _mp::clibhid_dev_info& item(*st.begin());
					std::wcout << std::hex << item.get_vendor_id() << L", " << item.get_product_id() << L" : ";
					std::wcout << item.get_path_by_wstring() << std::endl;
					std::wcout << item.get_interface_number() << std::endl;
					//

					for (int i = 0; i < 100; i++) {
						std::cout << "TEST" << i + 1 << std::endl;
						_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(item);
						if (wptr_dev.expired()) {
							std::cout << "device is expired" << std::endl;
							break;
						}

						_mp::type_v_buffer v_tx(64, 0);

						if (i % 2 == 0) {
							v_tx[0] = 'X';
						}
						else {
							v_tx[0] = 'Y';
						}

						pair_p_wait_n_event.first->reset(pair_p_wait_n_event.second);

						wptr_dev.lock()->start_write_read(v_tx, tp_hid::_cb_rx, &pair_p_wait_n_event);
						//wptr_dev.lock()->start_read(tp_hid::_cb_rx, &pair_p_wait_n_event);
						auto start = std::chrono::high_resolution_clock::now();

						int n_evt = _mp::cwait::const_event_timeout;

						int n_point = 0;

						do {
							n_evt = _mp::cwait::const_event_timeout;
							n_evt = pair_p_wait_n_event.first->wait_for_one_at_time(0);
							if (n_evt == pair_p_wait_n_event.second) {
								std::wcout << std::endl;
								std::wcout << L"RX DONE" << std::endl;
								break;
							}
							if (wptr_dev.expired()) {
								std::wcout << std::endl;
								std::cout << "device is expired" << std::endl;
								i = 1000;
								break;
							}

							std::this_thread::sleep_for(std::chrono::milliseconds(1000));
							std::wcout << L".";
							++n_point;
							if (n_point % 25 == 0) {
								std::wcout << std::endl;
							}
						} while (true);

						auto end = std::chrono::high_resolution_clock::now();
						std::chrono::duration<double> elapsed = end - start;
						std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
					}
				} while (false);

				return 0;
			}

			/**
			* io test of the first connected primitive device.(100 times)
			* 
			*/
			static int test7()
			{
				int n_result(0);
				bool b_result(false);
				_mp::clibhid_dev_info::type_set st_dev_info;
				_mp::clibhid_dev_info test_dev_info;

				int n_test_count = 10;
				std::vector<double> v_d_time(n_test_count*2, 0.0);

				do {
					// get connected device info.
					std::tie(b_result,st_dev_info) = tp_hid::_get_connected_device_info();
					if (!b_result) {
						std::wcout << L"error - get connected device info." << std::endl;
						n_result = -1;
						continue;
					}
					if (st_dev_info.empty()) {
						std::wcout << L"no device." << std::endl;
						n_result = 0;
						continue;
					}

					// filter device info set by device type.
					_mp::clibhid_dev_info::type_set st_dev_info_filtered;
					std::set<_mp::type_bm_dev> set_filter{ _mp::type_bm_dev_hid };
					st_dev_info_filtered = tp_hid::_filter_device_info_set_by_device_type(st_dev_info, set_filter);
					if (st_dev_info_filtered.empty()) {
						std::wcout << L"none filtered device." << std::endl;
						n_result = 0;
						continue;
					}

					std::wcout << L"= filtered devices. = " << std::endl;
					for (auto item : st_dev_info_filtered) {
						tp_hid::_display_device_info(item);
					}//end for

					// get the first device.
					test_dev_info = *st_dev_info_filtered.begin();
					//tp_hid::_display_device_info(test_dev_info);

					for (int i = 0; i < n_test_count; i++) {
						std::wcout << L"TEST" << std::dec << i + 1 << std::endl;
						
						std::chrono::duration<double> elapsed;
						bool b_test(false);

						std::tie(b_test, elapsed) = tp_hid::_test_one_byte_request(test_dev_info, 'X');
						if (!b_test) {
							std::wcout << L"test fail X - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
							break;
						}

						v_d_time[i] = elapsed.count();
						std::wcout << L"success test - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
						//
						std::tie(b_test, elapsed) = tp_hid::_test_one_byte_request(test_dev_info, 'Y');
						if (!b_test) {
							std::wcout << L"test fail Y - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
							break;
						}

						v_d_time[i+ n_test_count] = elapsed.count();
						std::wcout << L"success test - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;

					}//end for

					double d_t(0.0);
					for (auto c : v_d_time) {
						d_t += c;
					}
					std::wcout << L" * average - " << L"Elapsed time: " << d_t/((double)v_d_time.size()) << L" seconds" << std::endl;


				} while (false);

				return n_result;
			}

			/**
			* io test of the first connected primitive device.(100 times)
			*
			*/
			static int test_filtering()
			{
				int n_result(0);
				bool b_result(false);
				_mp::clibhid_dev_info::type_set st_dev_info;
				_mp::clibhid_dev_info test_dev_info;

				int n_test_count = 1000;

				do {
					// get connected device info.
					std::tie(b_result, st_dev_info) = tp_hid::_get_connected_device_info();
					if (!b_result) {
						std::wcout << L"error - get connected device info." << std::endl;
						n_result = -1;
						continue;
					}
					if (st_dev_info.empty()) {
						std::wcout << L"no device." << std::endl;
						n_result = 0;
						continue;
					}

					// filter device info set by device type.
					_mp::clibhid_dev_info::type_set st_dev_info_filtered;
					//std::set<_mp::type_bm_dev> set_filter;
					//
					std::array< _mp::type_bm_dev, 5 > ar_type{
						_mp::type_bm_dev_hid,
						_mp::type_bm_dev_lpu200_msr,
						_mp::type_bm_dev_lpu200_scr0,
						_mp::type_bm_dev_lpu200_ibutton,
						_mp::type_bm_dev_lpu200_switch0
					};

					
					// 조합을 저장할 벡터
					std::vector<std::set<_mp::type_bm_dev>> all_sets;
					std::set<_mp::type_bm_dev> current_set;

					// 조합 생성 함수 호출
					tp_hid::_generate_combinations(ar_type, current_set, all_sets);

					// 결과 출력
					for (const auto& set_filter : all_sets) {
						std::cout << "{ ";
						for (const auto& elem : set_filter) {
							std::cout << elem << " ";
						}
						std::cout << "}\n";
						//
						st_dev_info_filtered = tp_hid::_filter_device_info_set_by_device_type(st_dev_info, set_filter);
						if (st_dev_info_filtered.empty()) {
							std::wcout << L"none filtered device." << std::endl;
							n_result = 0;
							continue;
						}

						std::wcout << L"= filtered devices. = " << std::endl;
						for (auto item : st_dev_info_filtered) {
							tp_hid::_display_device_info(item);
						}//end for

					}

				} while (false);

				return n_result;
			}

			static int test_msr()
			{
				int n_result(0);
				bool b_result(false);
				_mp::clibhid_dev_info::type_set st_dev_info;
				_mp::clibhid_dev_info test_dev_info;

				int n_test_count = 1000;

				do {
					// get connected device info.
					std::tie(b_result, st_dev_info) = tp_hid::_get_connected_device_info();
					if (!b_result) {
						std::wcout << L"error - get connected device info." << std::endl;
						n_result = -1;
						continue;
					}
					if (st_dev_info.empty()) {
						std::wcout << L"no device." << std::endl;
						n_result = 0;
						continue;
					}

					// filter device info set by device type.
					_mp::clibhid_dev_info::type_set st_dev_info_filtered;
					std::set<_mp::type_bm_dev> set_filter{ _mp::type_bm_dev_lpu200_msr };
					st_dev_info_filtered = tp_hid::_filter_device_info_set_by_device_type(st_dev_info, set_filter);
					if (st_dev_info_filtered.empty()) {
						std::wcout << L"none filtered device." << std::endl;
						n_result = 0;
						continue;
					}

					std::wcout << L"= filtered devices. = " << std::endl;
					for (auto item : st_dev_info_filtered) {
						tp_hid::_display_device_info(item);
					}//end for

					for (int i = 0; i < 3; i++) {
						std::wcout << L'.';
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					}
					//continue;

					// get the first device.
					test_dev_info = *st_dev_info_filtered.begin();
					//tp_hid::_display_device_info(test_dev_info);

					for (int i = 0; i < n_test_count; i++) {
						std::wcout << L"TEST" << std::dec << i + 1 << std::endl;

						std::chrono::duration<double> elapsed;
						bool b_test(false);

						std::tie(b_test, elapsed) = tp_hid::_test_one_byte_request(test_dev_info, 'X');
						if (!b_test) {
							std::wcout << L"test fail X - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
							break;
						}

						std::wcout << L"success test - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
						//
						std::tie(b_test, elapsed) = tp_hid::_test_one_byte_request(test_dev_info, 'Y');
						if (!b_test) {
							std::wcout << L"test fail Y - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
							break;
						}

						std::wcout << L"success test - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;

					}//end for


				} while (false);

				return n_result;
			}
		private:
			static void _generate_combinations(const std::array<_mp::type_bm_dev, 5>& ar_type,
				std::set<_mp::type_bm_dev>& current_set,
				std::vector<std::set<_mp::type_bm_dev>>& all_sets,
				size_t start = 0)
			{
				// 현재 세트를 all_sets에 추가
				all_sets.push_back(current_set);

				for (size_t i = start; i < ar_type.size(); ++i) {
					// 현재 요소를 세트에 추가
					current_set.insert(ar_type[i]);
					// 다음 요소로 재귀 호출
					tp_hid::_generate_combinations(ar_type, current_set, all_sets, i + 1);
					// 현재 요소를 세트에서 제거 (백트래킹)
					current_set.erase(ar_type[i]);
				}
			}

			/**
			* @brief get device info of all connected devices.
			* and display it.
			* @param[in] none.
			* @return std::pair<bool,std::_mp::clibhid_dev_info::type_set> : 
			* first : true - success, false - fail.
			* second : device info set.
			*/
			static std::pair<bool, _mp::clibhid_dev_info::type_set> _get_connected_device_info()
			{
				bool b_result(false);
				_mp::clibhid_dev_info::type_set st;
				do {
					_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
					if (!lib_hid.is_ini()) {
						continue;
					}

					std::wcout << L"the connected device." << std::endl;
					st = lib_hid.get_cur_device_set();
					for (const _mp::clibhid_dev_info& item : st) {
						std::wcout << std::hex << item.get_vendor_id() << L", " << item.get_product_id() << L" : ";
						std::wcout << item.get_path_by_wstring() << std::endl;
						std::wcout << item.get_interface_number() << std::endl;
					}//end for

					b_result = true;
				} while (false);
				return std::make_pair(b_result,st);
			}

			/**
			* @brief filter device info set by device type.
			* @param[in,const] _mp::clibhid_dev_info::type_set& st_in : input device info set.
			* @param[in,const] std::set<_mp::type_bm_dev bm_dev> : device type set. - filtering st_in.
			* @return _mp::clibhid_dev_info::type_set : filtered device info set.
			*/
			static _mp::clibhid_dev_info::type_set _filter_device_info_set_by_device_type(
				const _mp::clibhid_dev_info::type_set& st_in,
				const std::set<_mp::type_bm_dev> set_bm_dev
			)
			{
				_mp::clibhid_dev_info::type_set st_out;
				do {
					if (st_in.empty()) {
						continue;
					}
					if (set_bm_dev.empty()) {
						st_out = st_in;
						continue;
					}
					for (const _mp::clibhid_dev_info& item : st_in) {
						if (set_bm_dev.find(item.get_type()) != std::end(set_bm_dev)) {
							st_out.insert(item);
						}
					}//end for
				} while (false);
				return st_out;
			}


			/**
			* @brief dislay device info.
			* @param[in,const] _mp::clibhid_dev_info dev_info : device info instance.
			* @return void.
			*/
			static void _display_device_info(const _mp::clibhid_dev_info& dev_info)
			{
				std::wcout << L"PTH : " << dev_info.get_path_by_wstring() << std::endl;
				std::wcout << L"VID : " << std::hex << dev_info.get_vendor_id() << std::endl;
				std::wcout << L"PID : " << std::hex << dev_info.get_product_id() << std::endl;
				std::wcout << L"INF : " << std::hex << dev_info.get_interface_number() << std::endl;
			}

			/**
			* @brief test "enter config". 
			* @param[in,const] _mp::clibhid_dev_info dev_info : device info instance.
			* @return std::pair<bool,std::chrono::duration<double>> : 
			* first : true - success, false - fail.
			* second : elapsed time.
			*/
			static std::pair<bool, std::chrono::duration<double>> _test_one_byte_request(const _mp::clibhid_dev_info& dev_info, unsigned char c_cmd)
			{
				bool b_result(false);
				std::chrono::duration<double> elapsed;
				do {
					_mp::cwait waiter;
					int n_event = waiter.generate_new_event();
					std::pair<_mp::cwait*, int> pair_p_wait_n_event(&waiter, n_event);

					_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
					if (!lib_hid.is_ini()) {
						continue;
					}
					_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(dev_info);
					if (wptr_dev.expired()) {
						continue;
					}
					
					//
					_mp::type_v_buffer v_tx(64, 0);
					v_tx[0] = c_cmd;
					pair_p_wait_n_event.first->reset(pair_p_wait_n_event.second);

					auto start = std::chrono::high_resolution_clock::now();//start timer

					wptr_dev.lock()->start_write_read(v_tx, tp_hid::_cb_rx, &pair_p_wait_n_event);

					int n_evt = _mp::cwait::const_event_timeout;
					int n_point = 0;
					int n_wait_cnt = 200;

					//wait for response
					do {
						n_evt = _mp::cwait::const_event_timeout;
						n_evt = pair_p_wait_n_event.first->wait_for_one_at_time(0);
						if (n_evt == pair_p_wait_n_event.second) {
							b_result = true;
							break;
						}
						if (wptr_dev.expired()) {
							std::wcout << std::endl;
							std::cout << "device is expired" << std::endl;
							break;
						}

						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						std::wcout << L".";
						++n_point;
						if (n_point % 25 == 0) {
							std::wcout << std::endl;
						}

						if (n_wait_cnt-- <= 0) {
							std::cout << "processing timeout" << std::endl;
							break;
						}
					} while (true);

					
					auto end = std::chrono::high_resolution_clock::now();//stop timer
					elapsed = end - start;
				} while (false);
				return std::make_pair(b_result, elapsed);
			}


		public:
            tp_hid();
            ~tp_hid();
        private:
	};

    
}//the end of _test name space