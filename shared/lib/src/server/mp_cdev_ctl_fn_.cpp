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
#include <server/mp_cdev_ctl_fn_.h>

#include <hid/mp_clibhid.h>

namespace _mp {
		cdev_ctl_fn::~cdev_ctl_fn()
		{
		}
		cdev_ctl_fn::cdev_ctl_fn() : m_n_cnt_open(0)
		{
		}

		bool cdev_ctl_fn::_execute_general_error_response(
			clog* p_log,
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

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_open_sync(clog* p_log, const cio_packet& request, cio_packet& response)
		{
			bool b_complete(true);
			bool b_result(false);

			response = request;
			response.set_cmd(cio_packet::cmd_response);

			do {
				response.set_data_error();

				if (m_n_cnt_open > 0) {//only suuport exclusive using device
					p_log->log_fmt(L"[E] - %ls | open counter = %d : session = %u.\n", __WFUNCTION__, m_n_cnt_open, request.get_session_number());
					p_log->trace(L"[E] - %ls | open counter = %d : session = %u.\n", __WFUNCTION__, m_n_cnt_open, request.get_session_number());
					continue;
				}

				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				std::wstring s_dev_path;

				if (request.get_data_field(s_dev_path) == 0) {
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_device_path), true);
					p_log->log_fmt(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					p_log->trace(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}

				_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
				if (wptr_dev.expired()) {
					p_log->log_fmt(L"[E] - %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					p_log->trace(L"[E] - %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}

				if (!wptr_dev.lock()->is_open()) {
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_device_open), true);
					p_log->log_fmt(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					p_log->trace(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}

				//m_map_parameter_for_independent_bootloader.clear();
				response.set_data_sucesss();
				++m_n_cnt_open;

				b_result = true;
			} while (false);

			if (b_result) {
				//push_info(_ns_tools::ct_color::COLOR_NORMAL, L"_execute_open : success : session = %u", request.get_session_number());
			}

			return std::make_pair(b_result, b_complete);
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_close_sync(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		)
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

				if (n_conected_session_number != request.get_session_number()) {
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_session), true);
					continue;
				}

				
				_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
				if (wptr_dev.expired()) {
					p_log->log_fmt(L"[E] - %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					p_log->trace(L"[E] - %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, request.get_session_number());
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

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_close(clog* p_log, const cio_packet& request, cio_packet& response)
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

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_read(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response, unsigned long n_conected_session_number, const std::wstring& s_dev_path
		)
		{
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, n_conected_session_number, s_dev_path);
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_write(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		)
		{
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, n_conected_session_number, s_dev_path);
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_transmit(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		)
		{
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, n_conected_session_number, s_dev_path);
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_cancel(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		)
		{
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, n_conected_session_number, s_dev_path);
			return pair_bool_result_bool_complete;
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_aync(
			clog* p_log, 
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		)
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

				if (n_conected_session_number != request.get_session_number()) {
					response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_session));
					continue;
				}

				clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
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
					wptr_dev.lock()->start_write_read(v_tx, cdev_ctl_fn::_cb_dev_io, this);
					break;
				case cio_packet::act_dev_cancel:
					_push_back_request_for_response(request);
					wptr_dev.lock()->start_cancel(cdev_ctl_fn::_cb_dev_io, this);
					break;
				case cio_packet::act_dev_write:
					_push_back_request_for_response(request);
					wptr_dev.lock()->start_write(v_tx, cdev_ctl_fn::_cb_dev_io, this);
					break;
				case cio_packet::act_dev_read:
					_push_back_request_for_response(request);
					wptr_dev.lock()->start_read(cdev_ctl_fn::_cb_dev_io, this);
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
		bool cdev_ctl_fn::_cb_dev_io(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

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

