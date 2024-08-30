#pragma once

#include <iostream>
#include <thread>
#include <chrono>

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

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
						for (int i = 0; i < 10; i++) {
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
						printf(" ++ %u : %02x,%02x,%02x.\n", v.size(), v[0], v[1], v[2]);
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
				std::pair<_mp::cwait*, int> pair_p(&waiter, n_event);
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
							std::wcout << std::hex << item.get_interface_number() << std::endl;
						}//end for
						std::wcout << std::dec;
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

						//
						pair_p.first->reset(pair_p.second);
						wptr_dev.lock()->start_read(tp_hid::_cb_rx, &pair_p);
						auto start = std::chrono::high_resolution_clock::now();

						int n_evt = _mp::cwait::const_event_timeout;

						do {
							n_evt = _mp::cwait::const_event_timeout;
							n_evt = pair_p.first->wait_for_one_at_time(0);
							if (n_evt == pair_p.second) {
								std::wcout << L"RX DON" << std::endl;
								break;
							}
							if (wptr_dev.expired()) {
								std::cout << "device is expired" << std::endl;
								i = 1000;
								break;
							}

							std::this_thread::sleep_for(std::chrono::milliseconds(1000));
							std::wcout << L"-_-;;" << std::endl;
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
        public:
            tp_hid();
            ~tp_hid();
        private:
	};
    
}//the end of _test name space