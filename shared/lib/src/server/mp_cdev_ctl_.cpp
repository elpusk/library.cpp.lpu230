#include <websocket/mp_win_nt.h>

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <mp_type.h>
#include <mp_cqueue.h>

#include <server/mp_cserver_.h>
#include <server/mp_cdev_ctl_.h>

#include <hid/mp_clibhid.h>

namespace _mp {
		cdev_ctl::~cdev_ctl()
		{
		}
		cdev_ctl::cdev_ctl(clog* p_log) : cworker_ctl(p_log), m_n_cnt_open(0)
		{
		}


		/**
		* executed by worker thread.
		* processing request.
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete
		*/
		bool cdev_ctl::_execute(cio_packet& request)
		{
			cio_packet response;
			type_v_buffer v_rsp;
			bool b_completet(true);
			//_ns_tools::ct_warp::cevent* p_event_start_worker_for_next_step(nullptr);

			do {
				if (request.is_response()) {
					b_completet = _execute_general_error_response(request, response, cio_packet::error_reason_request_type);
					continue;
				}
				size_t n_data(request.get_data_field_size());
				if (n_data > cws_server::const_default_max_rx_size_bytes) {
					if (request.get_data_field_type() == cio_packet::data_field_string_utf8) {
						m_p_log->get_instance().log_fmt(L"[E] %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					else {
						m_p_log->log_fmt(L"[E] %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					b_completet = _execute_general_error_response(request, response, cio_packet::error_reason_overflow_buffer);
					continue;
				}

				switch (request.get_action())
				{
				case cio_packet::act_dev_open:
					b_completet = _execute_open_sync(request, response).second;
					break;
				case cio_packet::act_dev_close:
					b_completet = _execute_close(request, response).second;
					break;
				case cio_packet::act_dev_transmit:
					b_completet = _execute_transmit(request, response).second;
					break;
				case cio_packet::act_dev_cancel:
					b_completet = _execute_cancel(request, response).second;
					break;
				case cio_packet::act_dev_write:
					b_completet = _execute_write(request, response).second;
					break;
				case cio_packet::act_dev_read:
					b_completet = _execute_read(request, response).second;
					break;
				/*
				case cio_packet::act_dev_sub_bootloader:
					std::tie(b_completet, p_event_start_worker_for_next_step) = _execute_sub_bootloader(request, response);
					break;
				case cio_packet::act_dev_independent_bootloader:
					b_completet = _execute_independent_bootloader(request, response);
					break;
				case cio_packet::act_mgmt_dev_kernel_operation:
					b_completet = _execute_kernel(request, response);
					break;
					*/
				default:
					b_completet = _execute_general_error_response(request, response, cio_packet::error_reason_action_code);
					break;
				}//end switch

			} while (false);

			do {
				if (!b_completet)
					continue;//response is defferd
				if (request.is_self())
					continue;//no response need, this request is issued from server-self.
				//send response
				response.get_packet_by_json_format(v_rsp);
				cserver::get_instance().send_data_to_client_by_ip4(v_rsp, request.get_session_number());
				/*
				if (p_event_start_worker_for_next_step) {
					p_event_start_worker_for_next_step->set();
				}
				*/
			} while (false);

			return b_completet;
		}

		bool cdev_ctl::_execute_general_error_response(
			cio_packet& request, 
			cio_packet& response, 
			cio_packet::type_error_reason n_reason /*= cio_packet::error_reason_none*/
		)
		{
			response = request;
			response.set_cmd(cio_packet::cmd_response);
			if (n_reason == cio_packet::error_reason_none)
				response.set_data_error();
			else
				response.set_data_error(cio_packet::get_error_message(n_reason));
			//
			return true;//complete with error
		}

		type_pair_bool_result_bool_complete cdev_ctl::_execute_open_sync(const cio_packet& request, cio_packet& response)
		{
			bool b_complete(true);
			bool b_result(false);

			response = request;
			response.set_cmd(cio_packet::cmd_response);

			do {
				response.set_data_error();

				if (m_n_cnt_open > 0) {//only suuport exclusive using device
					m_p_log->log_fmt(L"[E] %ls | open counter = %d : session = %u.\n", __WFUNCTION__, m_n_cnt_open, request.get_session_number());
					continue;
				}

				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				std::wstring s_dev_path;

				if (request.get_data_field(s_dev_path) == 0) {
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_device_path), true);
					m_p_log->log_fmt(L"[E] %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}

				_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
				if (wptr_dev.expired()) {
					m_p_log->log_fmt(L"[E] %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}

				if (!wptr_dev.lock()->is_open()) {
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_device_open), true);
					m_p_log->log_fmt(L"[E] %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}

				//m_map_parameter_for_independent_bootloader.clear();
				response.set_data_sucesss();
				set_connected_session(request.get_session_number());
				++m_n_cnt_open;

				b_result = true;
			} while (false);

			if (b_result) {
				//push_info(_ns_tools::ct_color::COLOR_NORMAL, L"_execute_open : success : session = %u", request.get_session_number());
			}

			return std::make_pair(b_result, b_complete);
		}

		type_pair_bool_result_bool_complete cdev_ctl::_execute_close_sync(const cio_packet& request, cio_packet& response)
		{
			bool b_complete(true);
			bool b_result(false);

			response = request;
			response.set_cmd(cio_packet::cmd_response);

			do {
				response.set_data_error();

				if (m_n_cnt_open <= 0) {
					continue;
				}

				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				if (get_connected_session() != request.get_session_number()) {
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_session), true);
					continue;
				}

				
				_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(get_dev_path());
				if (wptr_dev.expired()) {
					m_p_log->log_fmt(L"[E] %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}

				response.set_data_sucesss();
				--m_n_cnt_open;

				b_result = true;
			} while (false);

			if (b_result) {
				//push_info(_ns_tools::ct_color::COLOR_NORMAL, L"%ls : success : session = %u", __WFUNCTION__, request.get_session_number());
			}
			return std::make_pair(b_result, b_complete);
		}

		type_pair_bool_result_bool_complete cdev_ctl::_execute_close(const cio_packet& request, cio_packet& response)
		{
			response = request;
			response.set_cmd(cio_packet::cmd_response);

			if (m_n_cnt_open > 0) {
				response.set_data_sucesss();
				--m_n_cnt_open;
			}
			else {
				response.set_data_error();
			}

			type_pair_bool_result_bool_complete pair_bool_result_bool_complete(true, true);
			return pair_bool_result_bool_complete;
		}

		type_pair_bool_result_bool_complete cdev_ctl::_execute_read(const cio_packet& request, cio_packet& response)
		{
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(request, response);
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl::_execute_write(const cio_packet& request, cio_packet& response)
		{
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(request, response);
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl::_execute_transmit(const cio_packet& request, cio_packet& response)
		{
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(request, response);
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl::_execute_cancel(const cio_packet& request, cio_packet& response)
		{
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(request, response);
			return pair_bool_result_bool_complete;
		}

		type_pair_bool_result_bool_complete cdev_ctl::_execute_aync(const cio_packet& request, cio_packet& response)
		{
			bool b_complete(true);
			bool b_result(false);

			response = request;
			response.set_cmd(cio_packet::cmd_response);

			do {
				response.set_data_error();

				if (m_n_cnt_open <= 0) {
					continue;
				}

				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				if (get_connected_session() != request.get_session_number()) {
					response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_session));
					continue;
				}

				clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(get_dev_path());
				if (wptr_dev.expired()) {
					continue;
				}

				b_complete = false;

				type_v_buffer v_tx;
				request.get_data_field(v_tx);

				switch (request.get_action())
				{
				case cio_packet::act_dev_transmit:
					_push_back_request_for_response(request);
					wptr_dev.lock()->start_write_read(v_tx, cdev_ctl::_cb_dev_io, this);
					break;
				case cio_packet::act_dev_cancel:
					_push_back_request_for_response(request);
					wptr_dev.lock()->start_cancel(cdev_ctl::_cb_dev_io, this);
					break;
				case cio_packet::act_dev_write:
					_push_back_request_for_response(request);
					wptr_dev.lock()->start_write(v_tx, cdev_ctl::_cb_dev_io, this);
					break;
				case cio_packet::act_dev_read:
					_push_back_request_for_response(request);
					wptr_dev.lock()->start_read(cdev_ctl::_cb_dev_io, this);
					break;
				default:
					b_complete = true;//error complete.
					continue;
				}//end switch

				b_result = true;
			} while (false);

			if (b_result) {
				//push_info(_ns_tools::ct_color::COLOR_NORMAL, L"_execute_aync[%c] : pushed : session = %u", (wchar_t)request.get_action(), request.get_session_number());
			}
			return std::make_pair(b_result, b_complete);
		}

		/**
		* callback of clibhid_dev.start_read, start_write, start_write_read or start_cancel. 
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		bool cdev_ctl::_cb_dev_io(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl* p_obj((cdev_ctl*)p_user);

			cqitem_dev::type_result r(cqitem_dev::result_not_yet);//processing result
			type_v_buffer v;//received data
			std::wstring s;//information
			cio_packet response;
			cio_packet::type_act c_act = cio_packet::act_mgmt_unknown;
			unsigned long n_session = _MP_TOOLS_INVALID_SESSION_NUMBER;
			unsigned long n_device_index = 0;

			do {
				std::tie(r, v, s) = qi.get_result_all();
				switch (r) {
				case cqitem_dev::result_not_yet:
					b_complete = false;
					continue;//more processing
				case cqitem_dev::result_success:
				case cqitem_dev::result_error:
				case cqitem_dev::result_cancel:
				default:
					if (!p_obj->_pop_front_cur_reqeust_for_response_and_remve(response)) {
						continue;
					}
					c_act = response.get_action();
					n_session = response.get_session_number();
					n_device_index = response.get_device_index();
					response.set_cmd(cio_packet::cmd_response);
					break;
				}//end switch

				switch (r) {
				case cqitem_dev::result_success:
					if (v.empty()) {
						response.set_data_sucesss();
					}
					else {
						response.set_data(v);
					}
					break;
				case cqitem_dev::result_error:
					response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_device_operation));
					break;
				case cqitem_dev::result_cancel:
					response.set_data_cancel();
					break;
				default:
					response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_device_operation));
					break;
				}//end switch
				//
				type_v_buffer v_rsp;
				response.get_packet_by_json_format(v_rsp);

				if (!cserver::get_instance().send_data_to_client_by_ip4(v_rsp, n_session)) {
					//p_obj->push_info(_ns_tools::ct_color::COLOR_ERROR, L"_callback_request_result : error send data : session = %u : device_index = %u", n_session, n_device_index);
				}

			} while (false);

			return b_complete;
		}

}//the end of _mp namespace

