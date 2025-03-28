#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <array>
#include <algorithm>
#include <functional>

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
#include <hid/_vhid_info_lpu237.h>

namespace _test{
    class tp_hid{
	private:

		typedef	enum :int {
			_index_cb_tx_rx = 0,
			_index_cb_rx,
			_index_cb_cancel,
			_index_cb_msr,
			_index_cb_ibutton,
			_index_cb_msr_ibutton,
			_index_cb_total
		};

	private:
		/**
		* inneer class
		*/
		class _event {
		public:
			_event() : m_result(_mp::cqitem_dev::result_not_yet)
			{
				m_n_evt = m_evt.generate_new_event();
			}

			~_event()
			{
			}

			void reset()
			{
				m_result = _mp::cqitem_dev::result_not_yet;
				m_v_rx.resize(0);
				m_s_info.clear();
				m_evt.reset();
			}

			bool is_triggered()
			{
				if (m_evt.wait_for_one_at_time(0) == m_n_evt)
					return true;
				else
					return false;
			}

			void trigger()
			{
				m_evt.set(m_n_evt);
			}

			_mp::cqitem_dev::type_result get_result() const
			{
				return m_result;
			}

			_mp::type_v_buffer get_rx() const
			{
				return m_v_rx;
			}

			std::wstring get_info() const
			{
				return m_s_info;
			}

			_event& set_result(const _mp::cqitem_dev::type_result result)
			{
				m_result = result;
				return *this;
			}
			_event& set_rx(const _mp::type_v_buffer& v_rx)
			{
				m_v_rx = v_rx;
				return *this;
			}
			_event& set_info(const std::wstring & s_info)
			{
				m_s_info = s_info;
				return *this;
			}

			


		private:
			_mp::cwait m_evt;
			int m_n_evt;
			_mp::cqitem_dev::type_result m_result;
			_mp::type_v_buffer m_v_rx;
			std::wstring m_s_info;
		};

    public:

		/**
		* none event
		*/
		int wss_serving()
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
		

		static bool _cb_txrx(_mp::cqitem_dev& item, void* p_user)
		{
			bool b_complete(true);
			_mp::cqitem_dev::type_result r;
			_mp::type_v_buffer v;
			std::wstring s;

			_test::tp_hid::_event* p_evt = (_test::tp_hid::_event*)p_user;

			do {
				std::tie(r, v, s) = item.get_result_all();
				p_evt->set_result(r).set_rx(v).set_info(s);

				switch (r) {
				case _mp::cqitem_dev::result_not_yet:
					b_complete = false;
					std::wcout << L" ++ _cb_txrx : result_not_yet.\n";
					continue;//more processing
				case _mp::cqitem_dev::result_success:
					std::wcout << L" ++ _cb_txrx : result_success.\n";
					std::wcout << L" ++ " << std::dec <<(unsigned int)v.size() << L" : " << std::hex << v[0] << L"." << v[1] << L"." << v[2] << L"." << std::endl;
					break;
				case _mp::cqitem_dev::result_error:
					std::wcout << L" ++ _cb_txrx : result_error.( " << s << L" )\n";
					break;
				case _mp::cqitem_dev::result_cancel:
					std::wcout << L" ++ _cb_txrx : result_cancel.\n";
					break;
				default:
					std::wcout << L" ++ _cb_txrx : unknown.\n";
					break;
				}//end switch
			} while (false);

			fflush(stdout);

			p_evt->trigger();
			return b_complete;
		}

		static bool _cb_cancel(_mp::cqitem_dev& item, void* p_user)
		{
			bool b_complete(true);
			_mp::cqitem_dev::type_result r;
			_mp::type_v_buffer v;
			std::wstring s;

			_test::tp_hid::_event* p_evt = (_test::tp_hid::_event*)p_user;

			do {
				std::tie(r, v, s) = item.get_result_all();
				p_evt->set_result(r).set_rx(v).set_info(s);

				switch (r) {
				case _mp::cqitem_dev::result_not_yet:
					b_complete = false;
					std::wcout << L" ++ _cb_cancel : result_not_yet.\n";
					continue;//more processing
				case _mp::cqitem_dev::result_success:
					std::wcout << L" ++ _cb_cancel : result_success.\n";
					break;
				case _mp::cqitem_dev::result_error:
					std::wcout << L" ++ _cb_cancel : result_error.( " << s << L" )\n";
					break;
				case _mp::cqitem_dev::result_cancel:
					std::wcout << L" ++ _cb_cancel : result_cancel.\n";
					break;
				default:
					std::wcout << L" ++ _cb_cancel : unknown.\n";
					break;
				}//end switch
			} while (false);

			fflush(stdout);

			p_evt->trigger();
			return b_complete;
		}


		static bool _cb_msr(_mp::cqitem_dev& item, void* p_user)
		{
			bool b_complete(true);
			_mp::cqitem_dev::type_result r;
			_mp::type_v_buffer v;
			std::wstring s;

			_test::tp_hid::_event* p_evt = (_test::tp_hid::_event*)p_user;

			do {
				std::tie(r, v, s) = item.get_result_all();
				p_evt->set_result(r).set_rx(v).set_info(s);

				switch (r) {
				case _mp::cqitem_dev::result_not_yet:
					b_complete = false;
					std::wcout << L" ++ _cb_txrx : result_not_yet.\n";
					continue;//more processing
				case _mp::cqitem_dev::result_success:
					std::wcout << L" ++ _cb_txrx : result_success.\n";
					_print_msr(v);
					break;
				case _mp::cqitem_dev::result_error:
					std::wcout << L" ++ _cb_txrx : result_result_error.( " << s << L" )\n";
					break;
				case _mp::cqitem_dev::result_cancel:
					std::wcout << L" ++ _cb_txrx : result_cancel.\n";
					break;
				default:
					std::wcout << L" ++ _cb_txrx : unknown.\n";
					break;
				}//end switch
			} while (false);

			fflush(stdout);

			p_evt->trigger();
			return b_complete;
		}

		static bool _cb_ibutton(_mp::cqitem_dev& item, void* p_user)
		{
			bool b_complete(true);
			_mp::cqitem_dev::type_result r;
			_mp::type_v_buffer v;
			std::wstring s;

			_test::tp_hid::_event* p_evt = (_test::tp_hid::_event*)p_user;

			do {
				std::tie(r, v, s) = item.get_result_all();
				p_evt->set_result(r).set_rx(v).set_info(s);

				switch (r) {
				case _mp::cqitem_dev::result_not_yet:
					b_complete = false;
					std::wcout << L" ++ _cb_txrx : result_not_yet.\n";
					continue;//more processing
				case _mp::cqitem_dev::result_success:
					std::wcout << L" ++ _cb_txrx : result_success.\n";
					_print_ibutton(v);
					break;
				case _mp::cqitem_dev::result_error:
					std::wcout << L" ++ _cb_txrx : result_result_error.( " << s << L" )\n";
					break;
				case _mp::cqitem_dev::result_cancel:
					std::wcout << L" ++ _cb_txrx : result_cancel.\n";
					break;
				default:
					std::wcout << L" ++ _cb_txrx : unknown.\n";
					break;
				}//end switch
			} while (false);

			fflush(stdout);

			p_evt->trigger();
			return b_complete;
		}

		static bool _cb_msr_ibutton(_mp::cqitem_dev& item, void* p_user)
		{
			bool b_complete(true);
			_mp::cqitem_dev::type_result r;
			_mp::type_v_buffer v;
			std::wstring s;

			_test::tp_hid::_event* p_evt = (_test::tp_hid::_event*)p_user;

			do {
				std::tie(r, v, s) = item.get_result_all();
				p_evt->set_result(r).set_rx(v).set_info(s);

				switch (r) {
				case _mp::cqitem_dev::result_not_yet:
					b_complete = false;
					std::wcout << L" ++ _cb_msr_ibutton : result_not_yet.\n";
					continue;//more processing
				case _mp::cqitem_dev::result_success:
					std::wcout << L" ++ _cb_msr_ibutton : result_success.\n";
					if (_vhid_info_lpu237::is_rx_ibutton(v).first) {
						tp_hid::_print_ibutton(v);
					}
					else if (!_vhid_info_lpu237::is_rx_pair_txrx(v)) {
						tp_hid::_print_msr(v);
					}
					else {
						std::wcout << L"mis-response : (len,v[0],v[1],v[3]) = " << (unsigned int)v.size() << std::hex << v[0] << v[1] << v[2] << std::endl;
					}
					break;
				case _mp::cqitem_dev::result_error:
					std::wcout << L" ++ _cb_msr_ibutton : result_result_error.( " << s << L" )\n";
					break;
				case _mp::cqitem_dev::result_cancel:
					std::wcout << L" ++ _cb_msr_ibutton : result_cancel.\n";
					break;
				default:
					std::wcout << L" ++ _cb_msr_ibutton : unknown.\n";
					break;
				}//end switch
			} while (false);

			fflush(stdout);

			p_evt->trigger();
			return b_complete;
		}

		static bool _cb_ibutton_multi(_mp::cqitem_dev& item, void* p_user)
		{
			bool b_complete(true);
			_mp::cqitem_dev::type_result r;
			_mp::type_v_buffer v;
			std::wstring s;

			_test::tp_hid::_event* p_evt = (_test::tp_hid::_event*)p_user;

			do {
				std::tie(r, v, s) = item.get_result_all();
				p_evt->set_result(r).set_rx(v).set_info(s);

				switch (r) {
				case _mp::cqitem_dev::result_not_yet:
					b_complete = false;
					std::wcout << L" ++ _cb_msr_ibutton : result_not_yet.\n";
					continue;//more processing
				case _mp::cqitem_dev::result_success:
					std::wcout << L" ++ _cb_msr_ibutton : result_success.\n";
					if (_vhid_info_lpu237::is_rx_ibutton(v).first) {
						tp_hid::_print_ibutton(v);
					}
					else if (!_vhid_info_lpu237::is_rx_pair_txrx(v)) {
						tp_hid::_print_msr(v);
					}
					else {
						std::wcout << L"mis-response : (len,v[0],v[1],v[3]) = " << (unsigned int)v.size() << std::hex << v[0] << v[1] << v[2] << std::endl;
					}
					break;
				case _mp::cqitem_dev::result_error:
					std::wcout << L" ++ _cb_msr_ibutton : result_result_error.( " << s << L" )\n";
					break;
				case _mp::cqitem_dev::result_cancel:
					std::wcout << L" ++ _cb_msr_ibutton : result_cancel.\n";
					break;
				default:
					std::wcout << L" ++ _cb_msr_ibutton : unknown.\n";
					break;
				}//end switch
			} while (false);

			fflush(stdout);

			p_evt->trigger();
			return b_complete;
		}

		int test_start_logging()
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

		int test_wss_start_stop()
		{
			tp_hid::test_start_logging();
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
		int test_wss_certificate(_mp::type_set_wstring& set_parameters)
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
					tp_hid::test_wss_start_stop();
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

		int test_named_pipe()
		{
			tp_hid::test_start_logging();
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

		/**
		* io test of the first connected primitive device.(100 times)
		* 
		*/
		int test_hid_io(int n_test_count = 1000)
		{
			int n_result(0);
			bool b_result(false);
			_mp::clibhid_dev_info::type_set st_dev_info;
			_mp::clibhid_dev_info test_dev_info;

			std::vector<double> v_d_time(n_test_count*2, 0.0);

			tp_hid::test_start_logging();

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
		int test_filtering()
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

		int test_msr(int n_test_count = 1000)
		{
			int n_result(0);
			bool b_result(false);
			_mp::clibhid_dev_info::type_set st_dev_info;
			_mp::clibhid_dev_info test_dev_info;

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

				// get the first device.
				test_dev_info = *st_dev_info_filtered.begin();
				//tp_hid::_display_device_info(test_dev_info);

				std::chrono::duration<double> elapsed;
				bool b_test(false);

				std::tie(b_test, std::ignore) = tp_hid::_test_one_byte_request(test_dev_info,'I');
				if (!b_test) {
					std::wcout << L"test fail I - enter opos" << std::endl;
					continue;
				}
				else {
					std::wcout << L"OK - enter opos" << std::endl;
				}

				for (int i = 0; i < n_test_count; i++) {
					std::wcout << L"TEST" << std::dec << i + 1 << L" : " << L"swipe a card" << std::endl;


					std::tie(b_test, elapsed) = tp_hid::_test_reading_msr(test_dev_info,10);
					if (!b_test) {
						std::wcout << L"test fail X - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
						break;
					}

					std::wcout << L"success test - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
					//

				}//end for

				std::tie(b_test, std::ignore) = tp_hid::_test_one_byte_request(test_dev_info, 'J');
				if (!b_test) {
					std::wcout << L"test fail I - leave opos" << std::endl;
					continue;
				}
				else {
					std::wcout << L"OK - leave opos" << std::endl;
				}


			} while (false);

			return n_result;
		}

		int test_ibutton(int n_test_count = 1000)
		{
			int n_result(0);
			bool b_result(false);
			_mp::clibhid_dev_info::type_set st_dev_info;
			_mp::clibhid_dev_info test_dev_info;

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
				std::set<_mp::type_bm_dev> set_filter{ _mp::type_bm_dev_lpu200_ibutton };
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

				std::chrono::duration<double> elapsed;
				bool b_test(false);

				std::tie(b_test, std::ignore) = tp_hid::_test_one_byte_request(test_dev_info, 'I');
				if (!b_test) {
					std::wcout << L"test fail I - enter opos" << std::endl;
					continue;
				}
				else {
					std::wcout << L"OK - enter opos" << std::endl;
				}

				for (int i = 0; i < n_test_count; i++) {
					std::wcout << L"TEST" << std::dec << i + 1 << L" : " << L"contacts a i-button" << std::endl;


					std::tie(b_test, elapsed) = tp_hid::_test_reading_ibutton(test_dev_info, 10);
					if (!b_test) {
						std::wcout << L"test fail X - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
						break;
					}

					std::wcout << L"success test - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
					//

				}//end for

				std::tie(b_test, std::ignore) = tp_hid::_test_one_byte_request(test_dev_info, 'J');
				if (!b_test) {
					std::wcout << L"test fail I - leave opos" << std::endl;
					continue;
				}
				else {
					std::wcout << L"OK - leave opos" << std::endl;
				}


			} while (false);

			return n_result;
		}

		int test_multi_ibutton(int n_test_count = 1000)
		{
			int n_result(0);
			bool b_result(false);
			_mp::clibhid_dev_info::type_set st_dev_info;
			_mp::clibhid_dev_info test_dev_info;

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
				std::set<_mp::type_bm_dev> set_filter{ _mp::type_bm_dev_lpu200_ibutton };
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

				std::chrono::duration<double> elapsed;
				bool b_test(false);
				int n_wait_sec = -1; //infinite wait
				int n_worker_id = 100;
				//
				std::thread th_read_i_0(std::bind(&tp_hid::_test_reading_ibutton_worker, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), &test_dev_info, n_wait_sec, n_worker_id);

				for (int i = 0; i < 5; i++) {// unit second
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					std::wcout << L'*';
				}//delay for
				std::wcout << std::endl;

				n_worker_id += 100;
				std::thread th_read_i_1(std::bind(&tp_hid::_test_reading_ibutton_worker, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), &test_dev_info, n_wait_sec, n_worker_id);
							

				if (th_read_i_0.joinable()) {
					th_read_i_0.join();
				}
				std::wcout << L"DONE join() - th_read_i_0" << std::endl;

				if (th_read_i_1.joinable()) {
					th_read_i_1.join();
				}
				std::wcout << L"DONE join() - th_read_i_1" << std::endl;

			} while (false);

			return n_result;
		}


		int test_cancelmsr(int n_loop=1,int n_cancel_time_msec = 1000)
		{
			int n_result(0);
			bool b_result(false);
			_mp::clibhid_dev_info::type_set st_dev_info;
			_mp::clibhid_dev_info test_dev_info;

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

				// get the first device.
				test_dev_info = *st_dev_info_filtered.begin();

				std::chrono::duration<double> elapsed;
				bool b_test(false);

				std::tie(b_test, std::ignore) = tp_hid::_test_one_byte_request(test_dev_info, 'I');
				if (!b_test) {
					std::wcout << L"test fail I - enter opos" << std::endl;
					continue;
				}
				else {
					std::wcout << L"OK - enter opos" << std::endl;
				}

				int n_wait_sec = -1;// negative is infinate.

				for (int i = 0; i < n_loop; i++) {
					std::wcout << L" = MSR cancel test " << std::dec << i+1 << L".= " << std::endl;
					std::thread th_read_msr(std::bind(&tp_hid::_test_reading, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), &test_dev_info, nullptr, n_wait_sec);

					
					if (n_cancel_time_msec >= 0) {
						std::wcout << L"Readiong will be canceled after " << std::dec << n_cancel_time_msec << L" msec." << std::endl;
						std::this_thread::sleep_for(std::chrono::milliseconds(n_cancel_time_msec));
					}
					else {
						std::wcout << L"For cancel, press x " << std::endl;
						std::wstring s_in;

						do {
							std::wcin >> s_in;
							std::wcout << L"your input : " << s_in << std::endl;
						} while (s_in.compare(L"x") != 0);
					}

					std::wcout << L"start cancel." << std::endl;
					tp_hid::_test_cancel(test_dev_info, 10);

					if (th_read_msr.joinable()) {
						std::wcout << L"Waiting Join" << std::endl;
						th_read_msr.join();
					}
					std::wcout << L"Done Join" << std::endl;
				}//end for test loop
				//
				
				std::tie(b_test, std::ignore) = tp_hid::_test_one_byte_request(test_dev_info, 'J');
				if (!b_test) {
					std::wcout << L"test fail I - leave opos" << std::endl;
					continue;
				}
				else {
					std::wcout << L"OK - leave opos" << std::endl;
				}
				

			} while (false);

			return n_result;
		}

		int test_cancelibutton(int n_loop = 1, int n_cancel_time_msec = 1000)
		{
			int n_result(0);
			bool b_result(false);
			_mp::clibhid_dev_info::type_set st_dev_info;
			_mp::clibhid_dev_info test_dev_info;

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
				std::set<_mp::type_bm_dev> set_filter{ _mp::type_bm_dev_lpu200_ibutton };
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

				std::chrono::duration<double> elapsed;
				bool b_test(false);

				std::tie(b_test, std::ignore) = tp_hid::_test_one_byte_request(test_dev_info, 'I');
				if (!b_test) {
					std::wcout << L"test fail I - enter opos" << std::endl;
					continue;
				}
				else {
					std::wcout << L"OK - enter opos" << std::endl;
				}

				int n_wait_sec = -1;// negative is infinate.

				for (int i = 0; i < n_loop; i++) {
					std::wcout << L" = ibutton cancel test " << std::dec << i + 1 << L".= " << std::endl;
					std::thread th_read_ibutton(std::bind(&tp_hid::_test_reading, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), nullptr, &test_dev_info, n_wait_sec);


					if (n_cancel_time_msec >= 0) {
						std::wcout << L"Readiong will be canceled after " << std::dec << n_cancel_time_msec << L" msec." << std::endl;
						std::this_thread::sleep_for(std::chrono::milliseconds(n_cancel_time_msec));
					}
					else {
						std::wcout << L"For cancel, press x " << std::endl;
						std::wstring s_in;

						do {
							std::wcin >> s_in;
							std::wcout << L"your input : " << s_in << std::endl;
						} while (s_in.compare(L"x") != 0);
					}

					std::wcout << L"start cancel." << std::endl;
					tp_hid::_test_cancel(test_dev_info, 10);

					if (th_read_ibutton.joinable()) {
						std::wcout << L"Waiting Join" << std::endl;
						th_read_ibutton.join();
					}
					std::wcout << L"Done Join" << std::endl;
				}//end for test loop
				//

				std::tie(b_test, std::ignore) = tp_hid::_test_one_byte_request(test_dev_info, 'J');
				if (!b_test) {
					std::wcout << L"test fail I - leave opos" << std::endl;
					continue;
				}
				else {
					std::wcout << L"OK - leave opos" << std::endl;
				}


			} while (false);

			return n_result;
		}

		/**
		* msr 읽기를 위한 thread msra 와 ibutton 읽기를 위한 thread ibb, ibc 를 만들고,
		* 각 thread 가 독립적으로 읽기를 시도한다.
		* ms 카드를 읽으면, msra 에만 값이 표시되고, ibutton 을 contact 또는 remove 하면, ibb, ibc 두 곳에 값이 표시되면 정상.
		*/
		int test_msr_ibutton(int n_test_count = 1000)
		{
			int n_result(0);
			bool b_result(false);
			_mp::clibhid_dev_info::type_set st_dev_info;
			_mp::clibhid_dev_info test_msr_info, test_ibutton_info;

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
				_mp::clibhid_dev_info::type_set st_msr_info_filtered, st_ibutton_info_filtered;
				std::set<_mp::type_bm_dev> set_filter_msr{ _mp::type_bm_dev_lpu200_msr };
				std::set<_mp::type_bm_dev> set_filter_ibutton{ _mp::type_bm_dev_lpu200_ibutton };
				st_msr_info_filtered = tp_hid::_filter_device_info_set_by_device_type(st_dev_info, set_filter_msr);
				if (st_msr_info_filtered.empty()) {
					std::wcout << L"none filtered msr." << std::endl;
					n_result = 0;
					continue;
				}
				st_ibutton_info_filtered = tp_hid::_filter_device_info_set_by_device_type(st_dev_info, set_filter_ibutton);
				if (st_ibutton_info_filtered.empty()) {
					std::wcout << L"none filtered ibutton." << std::endl;
					n_result = 0;
					continue;
				}

				std::wcout << L"= filtered devices. = " << std::endl;
				for (auto item : st_msr_info_filtered) {
					tp_hid::_display_device_info(item);
				}//end for
				for (auto item : st_ibutton_info_filtered) {
					tp_hid::_display_device_info(item);
				}//end for

				// get the first device.
				test_msr_info = *st_msr_info_filtered.begin();
				test_ibutton_info = *st_ibutton_info_filtered.begin();

				std::chrono::duration<double> elapsed;
				bool b_test(false);

				std::tie(b_test, std::ignore) = tp_hid::_test_one_byte_request(test_msr_info, 'I');
				if (!b_test) {
					std::wcout << L"test fail I - enter opos" << std::endl;
					continue;
				}
				else {
					std::wcout << L"OK - enter opos" << std::endl;
				}

				//////////////////////////////////
				for (int i = 0; i < n_test_count; i++) {
					std::wcout << L"TEST" << std::dec << i + 1 << L" : " << L"swipe a card or contancts a ibutton" << std::endl;


					std::tie(b_test, elapsed) = tp_hid::_test_reading(&test_msr_info,&test_ibutton_info, -1);
					if (!b_test) {
						std::wcout << L"test fail X - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
						break;
					}

					std::wcout << L"success test - " << L"Elapsed time: " << elapsed.count() << L" seconds" << std::endl;
					//

				}//end for
				//////////////////////////////////
				std::tie(b_test, std::ignore) = tp_hid::_test_one_byte_request(test_msr_info, 'J');
				if (!b_test) {
					std::wcout << L"test fail I - leave opos" << std::endl;
					continue;
				}
				else {
					std::wcout << L"OK - leave opos" << std::endl;
				}


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

		static void _print_msr(const _mp::type_v_buffer& v_rx)
		{
			do {
				if (v_rx.size() < 3) {
					continue;
				}

				std::array<signed char, 3> ar_c_len{ v_rx[0],v_rx[1],v_rx[2] };
				std::array<signed char, 3> ar_c_add{ 0x20, 0x30, 0x30 };
				std::array<bool, 3> ar_b_error{ false, false, false };
				std::array<_mp::type_v_buffer, 3> ar_v_iso{ _mp::type_v_buffer(0), };
				size_t n_offset(0);

				for (size_t i = 0; i < ar_c_len.size(); i++) {
					if (ar_c_len[i] > 0) {
						std::copy(v_rx.begin() + 3 + n_offset, v_rx.begin() + 3 + n_offset + (size_t)ar_c_len[i], std::back_inserter(ar_v_iso[i]));
						n_offset += (size_t)ar_c_len[i];
					}
					else if (ar_c_len[i] < 0) {
						ar_b_error[i] = true;
					}
				}//end for

				for (size_t i = 0; i < ar_c_len.size(); i++) {
					if (ar_b_error[i]) {
						std::wcout << L" track" << i + 1 << L" : error.\n";
					}
					else {
						std::wcout << L" track" << i + 1 << L" : ";
						if (ar_c_len[i] > 0) {
							for (auto c_d : ar_v_iso[i]) {
								std::wcout << (wchar_t)(c_d + ar_c_add[i]);
							}//
						}
						else {
							std::wcout << L"NONE";
						}
						std::wcout << std::endl;;
					}
				}//end for
			} while (false);
		}

		static void _print_ibutton(const _mp::type_v_buffer& v_rx)
		{
			const std::string s_ibutton_postfix("this_is_ibutton_data");
			const size_t n_size_button_data(8);
			const size_t n_len_bytes = 3;

			do {
				if (v_rx.size() < n_len_bytes + n_size_button_data + s_ibutton_postfix.size()) {
					std::wcout << L"the length of rx is " << v_rx.size() << L"." << std::endl;
					std::wcout << L"It have to be greater then eqaul " << n_len_bytes + n_size_button_data + s_ibutton_postfix.size() << L" bytes." << std::endl;
					continue;
				}

				// 순수한 ibutton 데이터를 얻는다.
				_mp::type_v_buffer v_ibutton;
				std::copy(v_rx.begin() + n_len_bytes, v_rx.begin() + n_len_bytes + n_size_button_data, std::back_inserter(v_ibutton));

				for (auto c : v_ibutton) {
					std::wcout << std::hex << std::setw(2) << std::setfill(L'0') << c << L'.';
				}//end for
				std::wcout << std::endl;

			} while (false);
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
		std::pair<bool, std::chrono::duration<double>> _test_one_byte_request(const _mp::clibhid_dev_info& dev_info, unsigned char c_cmd)
		{
			bool b_result(false);
			std::chrono::duration<double> elapsed;
			do {
				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					std::wcout << L"!lib_hid.is_ini()" << std::endl;
					continue;
				}
				_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(dev_info);
				if (wptr_dev.expired()) {
					std::wcout << L"wptr_dev.expired()" << std::endl;
					continue;
				}

				m_ar_evt[tp_hid::_index_cb_tx_rx].reset();
				//
				_mp::type_v_buffer v_tx(64, 0);
				v_tx[0] = c_cmd;

				auto start = std::chrono::high_resolution_clock::now();//start timer

				wptr_dev.lock()->start_write_read(v_tx, tp_hid::_cb_txrx, &m_ar_evt[tp_hid::_index_cb_tx_rx]);

				int n_point = 0;
				int n_wait_cnt = 200;

				//wait for response
				do {
					if (m_ar_evt[tp_hid::_index_cb_tx_rx].is_triggered()) {
						std::wcout << L"_test_one_byte_request() - result code = "<< _mp::cqitem_dev::get_result_string(m_ar_evt[tp_hid::_index_cb_tx_rx].get_result()) << std::endl;
						b_result = true;
						break;
					}
					if (wptr_dev.expired()) {
						std::wcout << std::endl;
						std::wcout << L"_test_one_byte_request() - device is expired" << std::endl;
						break;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					std::wcout << L".";
					++n_point;
					if (n_point % 25 == 0) {
						std::wcout << std::endl;
					}

					if (n_wait_cnt-- <= 0) {
						std::wcout << L"_test_one_byte_request() - processing timeout" << std::endl;
						break;
					}
				} while (true);

					
				auto end = std::chrono::high_resolution_clock::now();//stop timer
				elapsed = end - start;
			} while (false);
			return std::make_pair(b_result, elapsed);
		}

		/**
		* @brief test reaing msr
		* @param[in,const] _mp::clibhid_dev_info dev_info : device info instance.
		* @param[in,int] n_timeout_sec : timeout unit is second. negative value is inifinite
		* @return std::pair<bool,std::chrono::duration<double>> :
		* first : true - success, false - fail.
		* second : elapsed time.
		*/
		std::pair<bool, std::chrono::duration<double>> _test_reading_msr(const _mp::clibhid_dev_info& dev_info,int n_timeout_sec)
		{
			bool b_result(false);
			std::chrono::duration<double> elapsed;
			do {
				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					std::wcout << L"!lib_hid.is_ini()" << std::endl;
					continue;
				}
				_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(dev_info);
				if (wptr_dev.expired()) {
					std::wcout << L"wptr_dev.expired()" << std::endl;
					continue;
				}

				m_ar_evt[tp_hid::_index_cb_msr].reset();
				//
				auto start = std::chrono::high_resolution_clock::now();//start timer

				wptr_dev.lock()->start_read( tp_hid::_cb_msr, &m_ar_evt[tp_hid::_index_cb_msr]);

				int n_point = 0;
				int n_wait_cnt = n_timeout_sec*1000/10; //10mmsec counter

				bool b_run(true);
				//wait for response
				do {
					if (m_ar_evt[tp_hid::_index_cb_msr].is_triggered()) {
						b_result = true;
						b_run = false;
						continue;
					}
					if (wptr_dev.expired()) {
						std::wcout << std::endl;
						std::wcout << L"_test_reading_msr() - device is expired" << std::endl;
						b_run = false;
						continue;
					}
					if (n_timeout_sec == 0) {
						std::wcout << L"_test_reading_msr() - processing timeout" << std::endl;
						b_run = false;
						continue;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					if (n_timeout_sec < 0) {
						continue;//infinite wait
					}
					//std::wcout << L".";
					++n_point;
					if (n_point % 25 == 0) {
						//std::wcout << std::endl;
					}

					if (n_wait_cnt-- <= 0) {
						std::wcout << L"_test_reading_msr() - processing timeout" << std::endl;
						b_run = false;
						continue;
					}
				} while (b_run);


				auto end = std::chrono::high_resolution_clock::now();//stop timer
				elapsed = end - start;
			} while (false);
			return std::make_pair(b_result, elapsed);
		}


		/**
		* @brief test reaing ibutton
		* @param[in,const] _mp::clibhid_dev_info dev_info : device info instance.
		* @param[in,int] n_timeout_sec : timeout unit is second. negative value is inifinite
		* @return std::pair<bool,std::chrono::duration<double>> :
		* first : true - success, false - fail.
		* second : elapsed time.
		*/
		std::pair<bool, std::chrono::duration<double>> _test_reading_ibutton(const _mp::clibhid_dev_info& dev_info, int n_timeout_sec)
		{
			bool b_result(false);
			std::chrono::duration<double> elapsed;
			do {
				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					std::wcout << L"!lib_hid.is_ini()" << std::endl;
					continue;
				}
				_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(dev_info);
				if (wptr_dev.expired()) {
					std::wcout << L"wptr_dev.expired()" << std::endl;
					continue;
				}

				m_ar_evt[tp_hid::_index_cb_ibutton].reset();
				//
				auto start = std::chrono::high_resolution_clock::now();//start timer

				wptr_dev.lock()->start_read(tp_hid::_cb_ibutton, &m_ar_evt[tp_hid::_index_cb_ibutton]);

				int n_point = 0;
				int n_wait_cnt = n_timeout_sec * 1000 / 10; //10mmsec counter

				bool b_run(true);
				//wait for response
				do {
					if (m_ar_evt[tp_hid::_index_cb_ibutton].is_triggered()) {
						b_result = true;
						b_run = false;
						continue;
					}
					if (wptr_dev.expired()) {
						std::wcout << std::endl;
						std::wcout << L"_test_reading_ibutton() - device is expired" << std::endl;
						b_run = false;
						continue;
					}
					if (n_timeout_sec == 0) {
						std::wcout << L"_test_reading_ibutton() - processing timeout" << std::endl;
						b_run = false;
						continue;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					if (n_timeout_sec < 0) {
						continue;//infinite wait
					}
					//std::wcout << L".";
					++n_point;
					if (n_point % 25 == 0) {
						//std::wcout << std::endl;
					}

					if (n_wait_cnt-- <= 0) {
						std::wcout << L"_test_reading_ibutton() - processing timeout" << std::endl;
						b_run = false;
						continue;
					}
				} while (b_run);


				auto end = std::chrono::high_resolution_clock::now();//stop timer
				elapsed = end - start;
			} while (false);
			return std::make_pair(b_result, elapsed);
		}

		/**
		* @brief test reaing msr or ibutton
		* @param[in,const] _mp::clibhid_dev_info* p_ibutton_info : ibutton device info instance.
		* @param[in,int] n_timeout_sec : timeout unit is second. negative value is infinite
		* @param[in,int] n_worker_id : worker id
		* @return std::pair<bool,std::chrono::duration<double>> :
		* first : true - success, false - fail.
		* second : elapsed time.
		*/
		std::pair<bool, std::chrono::duration<double>> _test_reading_ibutton_worker(const _mp::clibhid_dev_info* p_ibutton_info, int n_timeout_sec, int n_worker_id)
		{
			std::wcout << std::dec << n_worker_id << L"> " << L"START." << std::endl;
			bool b_result(false);
			std::chrono::duration<double> elapsed;
			std::chrono::steady_clock::time_point start;

			do {
				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					std::wcout << std::dec << n_worker_id << L"> " << L"!lib_hid.is_ini()" << std::endl;
					continue;
				}

				_mp::clibhid_dev::type_wptr wptr_msr, wptr_ibutton;

				start = std::chrono::high_resolution_clock::now();//start timer

				wptr_ibutton = lib_hid.get_device(*p_ibutton_info);
				if (wptr_ibutton.expired()) {
					std::wcout << std::dec << n_worker_id << L"> " << L"wptr_ibutton.expired()" << std::endl;
					continue;
				}

				bool b_txrx_io_test(false);
				std::tie(b_txrx_io_test, std::ignore) = tp_hid::_test_one_byte_request(*p_ibutton_info, 'J');
				if (!b_txrx_io_test) {
					std::wcout << std::dec << n_worker_id << L"> " << L"test fail I - TXRXIO" << std::endl;
					continue;
				}
				else {
					std::wcout << std::dec << n_worker_id << L"> " << L"OK -  TXRXIO" << std::endl;
				}

				m_ar_evt[tp_hid::_index_cb_ibutton].reset();
				wptr_ibutton.lock()->start_read(tp_hid::_cb_ibutton_multi, &m_ar_evt[tp_hid::_index_cb_ibutton]);

				int n_point = 0;
				int n_wait_cnt = n_timeout_sec * 1000 / 10; //10mmsec counter

				bool b_run(true);
				//wait for response
				do {
					if (m_ar_evt[tp_hid::_index_cb_ibutton].is_triggered()) {
						b_result = true;
						b_run = false;
						continue;
					}
					if (wptr_ibutton.expired()) {
						std::wcout << std::endl;
						std::wcout << std::dec << n_worker_id << L"> " << L"_test_reading() - ibutton device is expired" << std::endl;
						b_run = false;
						continue;
					}

					if (n_timeout_sec == 0) {
						std::wcout << std::dec << n_worker_id << L"> " << L"_test_reading() - processing timeout" << std::endl;
						b_run = false;
						continue;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					if (n_timeout_sec < 0) {
						continue;//infinite wait
					}
					//std::wcout << L".";
					++n_point;
					if (n_point % 25 == 0) {
						//std::wcout << std::endl;
					}

					if (n_wait_cnt-- <= 0) {
						std::wcout << std::dec << n_worker_id << L"> " << L"_test_reading() - processing timeout" << std::endl;
						b_run = false;
						continue;
					}
				} while (b_run);


				auto end = std::chrono::high_resolution_clock::now();//stop timer
				elapsed = end - start;
			} while (false);

			std::wcout << std::dec << n_worker_id << L"> " << L"EXIT." << std::endl;
			return std::make_pair(b_result, elapsed);
		}

		/**
		* @brief test reaing msr or ibutton
		* @param[in,const] _mp::clibhid_dev_info* p_msr_info : msr device info instance.
		* @param[in,const] _mp::clibhid_dev_info* p_ibutton_info : ibutton device info instance.
		* @param[in,int] n_timeout_sec : timeout unit is second. negative value is infinite
		* @return std::pair<bool,std::chrono::duration<double>> :
		* first : true - success, false - fail.
		* second : elapsed time.
		*/
		std::pair<bool, std::chrono::duration<double>> _test_reading(const _mp::clibhid_dev_info* p_msr_info, const _mp::clibhid_dev_info* p_ibutton_info, int n_timeout_sec)
		{
			bool b_result(false);
			std::chrono::duration<double> elapsed;
			std::chrono::steady_clock::time_point start;

			do {
				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					std::wcout << L"!lib_hid.is_ini()" << std::endl;
					continue;
				}

				_mp::clibhid_dev::type_wptr wptr_msr, wptr_ibutton;

				start = std::chrono::high_resolution_clock::now();//start timer

				if (p_msr_info) {
					wptr_msr = lib_hid.get_device(*p_msr_info);
					if (wptr_msr.expired()) {
						std::wcout << L"wptr_msr.expired()" << std::endl;
						continue;
					}
					m_ar_evt[tp_hid::_index_cb_msr].reset();
					wptr_msr.lock()->start_read(tp_hid::_cb_msr_ibutton, &m_ar_evt[tp_hid::_index_cb_msr]);
				}

				if (p_ibutton_info) {
					wptr_ibutton = lib_hid.get_device(*p_ibutton_info);
					if (wptr_ibutton.expired()) {
						std::wcout << L"wptr_ibutton.expired()" << std::endl;
						continue;
					}
					m_ar_evt[tp_hid::_index_cb_ibutton].reset();
					wptr_ibutton.lock()->start_read(tp_hid::_cb_msr_ibutton, &m_ar_evt[tp_hid::_index_cb_ibutton]);
				}

				int n_point = 0;
				int n_wait_cnt = n_timeout_sec * 1000 / 10; //10mmsec counter

				bool b_run(true);
				//wait for response
				do {
					if (p_msr_info) {
						if (m_ar_evt[tp_hid::_index_cb_msr].is_triggered()) {
							b_result = true;
							b_run = false;
							continue;
						}
						if (wptr_msr.expired()) {
							std::wcout << std::endl;
							std::wcout << L"_test_reading() - msr device is expired" << std::endl;
							b_run = false;
							continue;
						}
					}

					if (p_ibutton_info) {
						if (m_ar_evt[tp_hid::_index_cb_ibutton].is_triggered()) {
							b_result = true;
							b_run = false;
							continue;
						}
						if (wptr_ibutton.expired()) {
							std::wcout << std::endl;
							std::wcout << L"_test_reading() - ibutton device is expired" << std::endl;
							b_run = false;
							continue;
						}
					}

					if (n_timeout_sec == 0) {
						std::wcout << L"_test_reading() - processing timeout" << std::endl;
						b_run = false;
						continue;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					if (n_timeout_sec < 0) {
						continue;//infinite wait
					}
					//std::wcout << L".";
					++n_point;
					if (n_point % 25 == 0) {
						//std::wcout << std::endl;
					}

					if (n_wait_cnt-- <= 0) {
						std::wcout << L"_test_reading() - processing timeout" << std::endl;
						b_run = false;
						continue;
					}
				} while (b_run);


				auto end = std::chrono::high_resolution_clock::now();//stop timer
				elapsed = end - start;
			} while (false);
			return std::make_pair(b_result, elapsed);
		}

		/**
		* @brief test cancel reaing msr or ibutton
		* @param[in,const] _mp::clibhid_dev_info dev_info : device info instance.
		* @param[in,int] n_timeout_sec : timeout unit is second. negative value is inifinite
		* @return std::pair<bool,std::chrono::duration<double>> :
		* first : true - success, false - fail.
		* second : elapsed time.
		*/
		std::pair<bool, std::chrono::duration<double>> _test_cancel(const _mp::clibhid_dev_info& dev_info, int n_timeout_sec)
		{
			bool b_result(false);
			std::chrono::duration<double> elapsed;
			do {
				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					std::wcout << L"!lib_hid.is_ini()" << std::endl;
					continue;
				}
				_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(dev_info);
				if (wptr_dev.expired()) {
					std::wcout << L"wptr_dev.expired()" << std::endl;
					continue;
				}

				//
				m_ar_evt[tp_hid::_index_cb_cancel].reset();

				auto start = std::chrono::high_resolution_clock::now();//start timer

				wptr_dev.lock()->start_cancel(tp_hid::_cb_cancel, &m_ar_evt[tp_hid::_index_cb_cancel]);

				int n_point = 0;
				int n_wait_cnt = n_timeout_sec * 1000 / 10; //10mmsec counter

				bool b_run(true);
				//wait for response
				do {
					if (m_ar_evt[tp_hid::_index_cb_cancel].is_triggered()) {
						b_result = true;
						b_run = false;
						std::wcout << L"_test_cancel() - request is canceled." << std::endl;
						continue;
					}
					if (wptr_dev.expired()) {
						std::wcout << std::endl;
						std::wcout << L"_test_cancel() - device is expired" << std::endl;
						b_run = false;
						continue;
					}
					if (n_timeout_sec == 0) {
						std::wcout << L"_test_cancel() - processing timeout" << std::endl;
						b_run = false;
						continue;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					if (n_timeout_sec < 0) {
						continue;//infinite wait
					}
					//std::wcout << L".";
					++n_point;
					if (n_point % 25 == 0) {
						//std::wcout << std::endl;
					}

					if (n_wait_cnt-- <= 0) {
						std::wcout << L"_test_cancel() - processing timeout" << std::endl;
						b_run = false;
						continue;
					}
				} while (b_run);


				auto end = std::chrono::high_resolution_clock::now();//stop timer
				elapsed = end - start;
			} while (false);
			return std::make_pair(b_result, elapsed);
		}

	public:
		~tp_hid(){}

		static tp_hid& get_instance()
		{
			static tp_hid obj;
			return obj;
		}

    private:
		tp_hid()
		{
		}

	private:
		std::array<tp_hid::_event, tp_hid::_index_cb_total> m_ar_evt;

	};
}//the end of _test name space