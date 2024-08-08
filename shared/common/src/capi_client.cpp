#include <websocket/mp_win_nt.h>
#include <capi_client.h>
#include <mp_cconvert.h>


void capi_client::unload()
{

}
void capi_client::_sync_before(unsigned long n_client_index, unsigned long n_device_index, unsigned char c_in_id, char unsigned c_out_id)//need thread safety
{
	capi_client::_get_in_id_for_sync() = c_in_id;
	capi_client::_get_out_id_for_sync() = c_out_id;
	capi_client::_get_device_index_for_sync() = n_device_index;
	capi_client::_get_rx_for_sync().resize(0);
	capi_client::_get_promise_bool_tx_for_sync() = std::make_shared<_type_promise_bool>();
	capi_client::_get_promise_dword_rx_for_sync() = std::make_shared<_type_promise_dword>();

	_set_callback_write(n_client_index, capi_client::_write_cb, nullptr);
	_set_callback_read(n_client_index, capi_client::_read_cb, nullptr);
}
void capi_client::_sync_after(unsigned long n_client_index, unsigned long& n_device_index, unsigned char& c_in_id, std::vector<unsigned char>& v_rx)//need thread safety
{
	//recover
	_set_callback_read(n_client_index, m_map_cur_callbacks[n_client_index]->cb_read, m_map_cur_callbacks[n_client_index]->p_user_read);
	_set_callback_write(n_client_index, m_map_cur_callbacks[n_client_index]->cb_write, m_map_cur_callbacks[n_client_index]->p_user_write);

	v_rx = capi_client::_get_rx_for_sync();
	capi_client::_get_rx_for_sync().resize(0);
	n_device_index = capi_client::_get_device_index_for_sync();
	c_in_id = capi_client::_get_in_id_for_sync();
}

capi_client::~capi_client()
{
	unload();
}

unsigned long capi_client::get_last_result(unsigned long n_device_index) const
{
	capi_client::_type_map_device_index_pair_action_result::const_iterator it = m_map_device_index_pair_action_result.find(n_device_index);
	if (it == std::end(m_map_device_index_pair_action_result)) {
		return _mp::cclient::RESULT_SUCCESS;
	}
	else {
		return it->second.second;
	}
	return m_dw_last_result;
}
capi_client::type_action capi_client::get_last_action(unsigned long n_device_index) const
{
	_type_map_device_index_pair_action_result::const_iterator it = m_map_device_index_pair_action_result.find(n_device_index);
	if (it == std::end(m_map_device_index_pair_action_result)) {
		return capi_client::act_none;
	}
	else {
		return it->second.first;
	}
}
void capi_client::set_last_action_and_result(unsigned long n_device_index, const capi_client::type_action last_action, const unsigned long dw_last_result)
{
	m_map_device_index_pair_action_result[n_device_index] = std::make_pair(last_action, dw_last_result);
}

unsigned long capi_client::create_before_set_callback()
{
	unsigned long n_index(_mp::cclient::UNDEFINED_INDEX);
	n_index = _mp::cclient_manager::get_instance().create_client();

	if (n_index == _mp::cclient::UNDEFINED_INDEX) {
		set_last_action_and_result(0, capi_client::act_create, _mp::cclient::RESULT_ERROR);
	}
	else {
		set_last_action_and_result(0, capi_client::act_create, _mp::cclient::RESULT_SUCCESS);
	}
	return n_index;
}
bool capi_client::destory_after_stop(unsigned long n_client_index)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);

	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		_mp::cclient_manager::get_instance().destory_client(n_client_index);
		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);

	set_last_action_and_result(0, capi_client::act_destory, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}

bool capi_client::start_after_set_callback(unsigned long n_client_index, bool b_tls)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);

	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		if (b_tls == 0) {
			p_client->set_ssl(false).set_port(_mp::_ws_tools::WEBSOCKET_SERVER_PORT_COFFEE_MANAGER);
		}
		else {
			p_client->set_ssl(true).set_port(_mp::_ws_tools::WEBSOCKET_SECURITY_SERVER_PORT_COFFEE_MANAGER);
		}
		if (!p_client->start())
			continue;
		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(0, capi_client::act_connect, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}
bool capi_client::start_after_set_callback_sync(unsigned long n_client_index, bool b_tls, long n_msec_wait_time /*= -1*/)
{
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	std::lock_guard<std::mutex> lock(m_mutex_for_sync);

	_sync_before(n_client_index, 0, 0, 0);
	_set_callback_resolve(n_client_index, capi_client::_before_handshake_cb, nullptr);//set dummy
	_set_callback_connect(n_client_index, capi_client::_before_handshake_cb, nullptr);//set dummy
	_set_callback_handshake_ssl(n_client_index, capi_client::_before_handshake_cb, nullptr);//set dummy
	_set_callback_handshake(n_client_index, capi_client::_handshake_cb, nullptr);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!start_after_set_callback(n_client_index, b_tls))//send request
					continue;

				if (n_msec_wait_time > 0) {
					std::chrono::milliseconds span(n_msec_wait_time);
					if (future_read.wait_for(span) == std::future_status::timeout) {
						n_result = _mp::cclient::RESULT_ERROR;
						continue;
					}
				}
				n_result = future_read.get();//wait connected
			} while (false);
			});

		thread_read.join();
	} while (false);

	//recover
	_set_callback_handshake(n_client_index, m_map_cur_callbacks[n_client_index]->cb_handshake, m_map_cur_callbacks[n_client_index]->p_user_handshake);
	_set_callback_handshake_ssl(n_client_index, m_map_cur_callbacks[n_client_index]->cb_handshake_ssl, m_map_cur_callbacks[n_client_index]->p_user_handshake_ssl);
	_set_callback_connect(n_client_index, m_map_cur_callbacks[n_client_index]->cb_connect, m_map_cur_callbacks[n_client_index]->p_user_connect);
	_set_callback_resolve(n_client_index, m_map_cur_callbacks[n_client_index]->cb_resolve, m_map_cur_callbacks[n_client_index]->p_user_resolve);

	std::vector<unsigned char> v_rx(0);
	unsigned char c_in_id = 0;
	unsigned long n_device_index(0);
	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}
bool capi_client::stop_before_destory(unsigned long n_client_index)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		p_client->stop();
		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);

	set_last_action_and_result(0, capi_client::act_disconnect, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}

bool capi_client::echo(unsigned long n_client_index, const std::vector<unsigned char>& v_tx)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		if (v_tx.empty())
			continue;
		//
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_manager, 0)
			.set_action(_mp::cio_packet::act_mgmt_get_echo)
			.set_data(&v_tx[0], (unsigned long)v_tx.size());

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;
		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);

	set_last_action_and_result(0, capi_client::act_echo, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}
bool capi_client::echo_sync(std::vector<unsigned char>& v_rx, unsigned long n_client_index, const std::vector<unsigned char>& v_tx)
{
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);
	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!echo(n_client_index, v_tx))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned long n_device_index(0);
	unsigned char c_in_id = 0, c_out_id = 0;
	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	return b_result;
}


bool capi_client::echo_list_string(unsigned long n_client_index, const std::list<std::wstring>& list_string)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		if (list_string.empty())
			continue;
		//
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_manager)
			.set_action(_mp::cio_packet::act_mgmt_get_echo);
		//
		packet.set_data_by_utf8(list_string);

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(0, capi_client::act_echo_list_string, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}

bool capi_client::echo_list_string_sync(std::list<std::wstring>& list_response, unsigned long n_client_index, const std::list<std::wstring>& list_string)
{
	const std::wstring s_success(L"success");
	std::vector<unsigned char> v_rx;
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);

	list_response.clear();

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!echo_list_string(n_client_index, list_string))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned long n_device_index(0);
	unsigned char c_in_id = 0, c_out_id = 0;

	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	if (b_result) {
		capi_client::help_strings_from_response(list_response, v_rx);
	}
	return b_result;
}

bool capi_client::ctl_show(unsigned long n_client_index, bool b_show /*= true*/)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;

		std::wstring s_show;
		if (b_show) {
			s_show = L"show";
		}
		else {
			s_show = L"hide";
		}

		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_manager, 0)
			.set_action(_mp::cio_packet::act_mgmt_ctl_show)
			.set_data_by_utf8(s_show);
		packet.set_data_by_utf8(L"espresso", true).set_data_by_utf8(L"cappuccino", true);

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;
		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(0, capi_client::act_ctl_show, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}

bool capi_client::ctl_show_sync(unsigned long n_client_index, bool b_show /*= true*/)
{
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);
	std::vector<unsigned char> v_rx;
	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			_set_callback_write(n_client_index, capi_client::_write_cb, nullptr);
			_set_callback_read(n_client_index, capi_client::_read_cb, nullptr);
			do {
				if (!ctl_show(n_client_index, b_show)) //send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned long n_device_index(0);
	unsigned char c_in_id = 0, c_out_id = 0;
	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	if (b_result) {
		//the analysis of response in manager request.
		b_result = capi_client::help_reponse_contain_success(v_rx);
	}

	return b_result;
}

bool capi_client::get_device_list(unsigned long n_client_index, const std::wstring & s_filter /*= std::wstring()*/)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;

		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_manager, 0)
			.set_action(_mp::cio_packet::act_mgmt_get_device_list)
			.set_data_by_utf8(s_filter);

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(0, capi_client::act_get_device_list, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}

bool capi_client::get_device_list_multi(unsigned long n_client_index, const std::list<std::wstring>&list_s_filter /*= std::list<std::wstring>()*/)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;

		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_manager, 0)
			.set_action(_mp::cio_packet::act_mgmt_get_device_list)
			.set_data_by_utf8(list_s_filter);

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(0, capi_client::act_get_device_list, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}

bool capi_client::get_device_list_sync(std::set<std::wstring> &set_s_path, unsigned long n_client_index, const std::wstring & s_filter /*= std::wstring()*/)
{
	std::vector<unsigned char> v_rx;
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);

	set_s_path.clear();

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!get_device_list(n_client_index, s_filter))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned long n_device_index(0);
	unsigned char c_in_id = 0, c_out_id = 0;

	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	if (b_result) {
		capi_client::help_strings_from_response(set_s_path, v_rx);
	}
	return b_result;
}

bool capi_client::get_device_list_multi_sync(std::set<std::wstring>&set_s_path, unsigned long n_client_index, const std::list<std::wstring>&list_s_filter /*= std::list<std::wstring>()*/)
{
	std::vector<unsigned char> v_rx;
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);

	set_s_path.clear();

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!get_device_list_multi(n_client_index, list_s_filter))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned long n_device_index(0);
	unsigned char c_in_id = 0, c_out_id = 0;

	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	if (b_result) {
		capi_client::help_strings_from_response(set_s_path, v_rx);
	}
	return b_result;
}


bool capi_client::open(unsigned long n_client_index, const std::wstring & s_device_path)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		if (s_device_path.empty())
			continue;
		//
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_device, 0)
			.set_action(_mp::cio_packet::act_dev_open)
			.set_data_by_utf8(std::wstring(s_device_path));

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(0, capi_client::act_open, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}
bool capi_client::open_sync(unsigned long& n_out_device_index, unsigned long n_client_index, const std::wstring & s_device_path)
{
	bool b_result(false);

	if (n_client_index == _mp::cclient::UNDEFINED_INDEX)
		return b_result;
	if (s_device_path.empty())
		return b_result;
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);

	std::vector<unsigned char> v_rx;
	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			_set_callback_write(n_client_index, capi_client::_write_cb, nullptr);
			_set_callback_read(n_client_index, capi_client::_read_cb, nullptr);
			do {
				if (!open(n_client_index, s_device_path)) //send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);
	unsigned long n_device_index(0);
	unsigned char c_in_id = 0;
	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);
	n_out_device_index = n_device_index;
	//In device target request, data field's success, cancel, error is chekced automatically
	return b_result;
}
bool capi_client::close(unsigned long n_client_index, unsigned long n_device_index)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_device, n_device_index)
			.set_action(_mp::cio_packet::act_dev_close);

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//

	set_last_action_and_result(n_device_index, capi_client::act_close, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}
bool capi_client::close_sync(unsigned long n_client_index, unsigned long n_device_index)
{
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);
	std::vector<unsigned char>  v_rx;

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, n_device_index, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {

				if (!close(n_client_index, n_device_index))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);
	unsigned char c_in_id = 0, c_out_id = 0;
	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);
	//In device target request, data field's success, cancel, error is chekced automatically
	return b_result;
}
bool capi_client::write(unsigned long n_client_index, unsigned long n_device_index, unsigned char c_out_id, const std::vector<unsigned char>& v_data)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		//
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_device, n_device_index)
			.set_action(_mp::cio_packet::act_dev_write)
			.set_out_id(c_out_id)
			.set_data(v_data);

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(n_device_index, capi_client::act_write, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}
bool capi_client::write_sync(unsigned long n_client_index, unsigned long n_device_index, unsigned char c_out_id, const std::vector<unsigned char>&v_tx)
{
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);
	std::vector<unsigned char> v_rx;

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, n_device_index, 0, c_out_id);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!write(n_client_index, n_device_index, c_out_id, v_tx))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned char c_in_id = 0;
	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	return b_result;
}
bool capi_client::read(unsigned long n_client_index, unsigned long n_device_index, unsigned char c_in_id)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_device, n_device_index)
			.set_action(_mp::cio_packet::act_dev_read)
			.set_in_id(c_in_id);

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);

	set_last_action_and_result(n_device_index, capi_client::act_read, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}
bool capi_client::read_sync(std::vector<unsigned char>&v_rx, unsigned long n_client_index, unsigned long n_device_index, unsigned char c_in_id)
{
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);
	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, n_device_index, c_in_id, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!read(n_client_index, n_device_index, c_in_id))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);
	unsigned char n_out_id(0);
	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	return b_result;
}
bool capi_client::transmit(unsigned long n_client_index, unsigned long n_device_index, unsigned char c_in_id, unsigned char c_out_id, const std::vector<unsigned char>& v_data)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		//
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_device, n_device_index)
			.set_action(_mp::cio_packet::act_dev_transmit)
			.set_in_id(c_in_id)
			.set_out_id(c_out_id)
			.set_data(v_data);

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(n_device_index, capi_client::act_transmit, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}
bool capi_client::transmit_sync(std::vector<unsigned char>&v_rx, unsigned long n_client_index, unsigned long n_device_index, unsigned char c_in_id, unsigned char c_out_id, const std::vector<unsigned char>&v_tx)
{
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);
	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, n_device_index, c_in_id, c_out_id);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!transmit(n_client_index, n_device_index, c_in_id, c_out_id, v_tx))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);
	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	return b_result;
}
bool capi_client::cancel(unsigned long n_client_index, unsigned long n_device_index)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_device, n_device_index)
			.set_action(_mp::cio_packet::act_dev_cancel);

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(n_device_index, capi_client::act_cancel, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}
bool capi_client::cancel_sync(unsigned long n_client_index, unsigned long n_device_index)
{
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);
	std::vector<unsigned char> v_rx;

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, n_device_index, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!cancel(n_client_index, n_device_index))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);
	unsigned char c_in_id = 0, c_out_id = 0;
	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);
	//In device target request, data field's success, cancel, error is chekced automatically
	return b_result;
}

unsigned long capi_client::send_to_server(unsigned long n_client_index, unsigned char* s_tx, unsigned long n_tx)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);

	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		if (s_tx == NULL)
			continue;
		if (n_tx == 0)
			continue;
		_mp::type_v_buffer v_tx(n_tx);
		memcpy(&v_tx[0], s_tx, n_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;
		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	return n_result;
}

bool capi_client::advance_set_session_name(unsigned long n_client_index, const std::wstring & s_session_name)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		if (s_session_name.empty())
			continue;
		//
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_manager)
			.set_action(_mp::cio_packet::act_mgmt_advance_operation)
			.set_data_by_utf8(L"set_session_name", false)
			.set_data_by_utf8(s_session_name, true);
		//
		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(0, capi_client::act_advance_set_session_name, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}

bool capi_client::advance_set_session_name_sync(unsigned long& n_out_device_index, unsigned long n_client_index, const std::wstring & s_session_name)
{
	std::vector<unsigned char> v_rx;
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!advance_set_session_name(n_client_index, s_session_name))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned long n_device_index(0);
	unsigned char c_in_id = 0, c_out_id = 0;

	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	if (b_result) {
		b_result = capi_client::help_reponse_contain_success(v_rx, false);
	}
	return b_result;
}

bool capi_client::advance_get_session_name(unsigned long n_client_index)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		//
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_manager)
			.set_action(_mp::cio_packet::act_mgmt_advance_operation)
			.set_data_by_utf8(L"get_session_name", false);
		//
		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//

	set_last_action_and_result(0, capi_client::act_advance_get_session_name, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}
bool capi_client::advance_get_session_name_sync(std::wstring & s_out_session_name, unsigned long n_client_index)
{
	const std::wstring s_success(L"success");

	std::vector<unsigned char> v_rx;
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!advance_get_session_name(n_client_index))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned long n_device_index(0);
	unsigned char c_in_id = 0, c_out_id = 0;

	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	do {
		if (!b_result)
			continue;
		b_result = false;
		std::list<std::wstring> list_s_dst;
		capi_client::help_strings_from_response(list_s_dst, v_rx);
		if (list_s_dst.empty())
			continue;
		if (list_s_dst.front().compare(s_success) != 0)
			continue;
		list_s_dst.pop_front();
		if (list_s_dst.empty())
			s_out_session_name.clear();//none name
		else
			s_out_session_name = list_s_dst.front();
		//
		b_result = true;
	} while (false);
	return b_result;
}

bool capi_client::advance_send_data_to_session(unsigned long n_client_index, const std::wstring & s_session_name, const std::list<std::wstring>&list_string)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		if (s_session_name.empty())
			continue;
		//
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_manager)
			.set_action(_mp::cio_packet::act_mgmt_advance_operation)
			.set_data_by_utf8(L"send_data_to_session", false)
			.set_data_by_utf8(s_session_name, true);
		//
		for (auto s_item : list_string) {
			packet.set_data_by_utf8(s_item, true);
		}//end for

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);

	set_last_action_and_result(0, capi_client::act_advance_send_data_to_session, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}

bool capi_client::advance_send_data_to_session_sync(std::list<std::wstring>&list_response, unsigned long n_client_index, const std::wstring & s_session_name, const std::list<std::wstring>&list_string)
{
	const std::wstring s_success(L"success");
	std::vector<unsigned char> v_rx;
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);

	list_response.clear();

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!advance_send_data_to_session(n_client_index, s_session_name, list_string))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned long n_device_index(0);
	unsigned char c_in_id = 0, c_out_id = 0;

	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	if (b_result) {
		capi_client::help_strings_from_response(list_response, v_rx);
	}
	return b_result;
}

bool capi_client::advance_send_data_to_all_session(unsigned long n_client_index, const std::list<std::wstring>&list_string)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		//
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_manager)
			.set_action(_mp::cio_packet::act_mgmt_advance_operation)
			.set_data_by_utf8(L"send_data_to_all", false);
		//
		for (auto s_item : list_string) {
			packet.set_data_by_utf8(s_item, true);
		}//end for

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(0, capi_client::act_advance_send_data_to_all_session, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}

bool capi_client::advance_send_data_to_all_session_sync(std::list<std::wstring>&list_response, unsigned long n_client_index, const std::list<std::wstring>&list_string)
{
	const std::wstring s_success(L"success");
	std::vector<unsigned char> v_rx;
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);

	list_response.clear();

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!advance_send_data_to_all_session(n_client_index, list_string))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned long n_device_index(0);
	unsigned char c_in_id = 0, c_out_id = 0;

	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	if (b_result) {
		capi_client::help_strings_from_response(list_response, v_rx);
	}
	return b_result;
}

bool capi_client::kernel(unsigned long n_client_index, unsigned long n_device_index, const std::list<std::wstring>&list_string)
{
	unsigned long n_result(_mp::cclient::RESULT_ERROR);
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		//
		_mp::cio_packet packet;
		packet.set_cmd(_mp::cio_packet::cmd_request)
			.set_session_number(p_client->get_session_number_from_server())
			.set_owner(_mp::cio_packet::owner_manager, n_device_index)
			.set_action(_mp::cio_packet::act_mgmt_dev_kernel_operation);

		//
		for (auto s_item : list_string) {
			packet.set_data_by_utf8(s_item, true);
		}//end for

		_mp::type_v_buffer v_tx;
		packet.get_packet_by_json_format(v_tx);
		if (!p_client->send_data_to_server_by_ip4(v_tx))
			continue;

		n_result = _mp::cclient::RESULT_SUCCESS;
	} while (false);
	//
	set_last_action_and_result(0, capi_client::act_kernel, n_result);
	if (n_result == _mp::cclient::RESULT_SUCCESS)	return true;
	else	return false;
}

bool capi_client::kernel_sync(std::list<std::wstring>&list_response, unsigned long n_client_index, unsigned long n_device_index, const std::list<std::wstring>&list_string)
{
	const std::wstring s_success(L"success");
	std::vector<unsigned char> v_rx;
	unsigned long n_result(_mp::cclient::RESULT_SUCCESS);
	bool b_result(false);

	list_response.clear();

	std::lock_guard<std::mutex> lock(m_mutex_for_sync);
	_sync_before(n_client_index, 0, 0, 0);

	do {
		std::future<bool> future_write = capi_client::_get_promise_bool_tx_for_sync()->get_future();
		std::future<unsigned long> future_read = capi_client::_get_promise_dword_rx_for_sync()->get_future();

		std::thread thread_read([&] {
			do {
				if (!kernel(n_client_index, n_device_index, list_string))//send request
					continue;
				b_result = future_write.get();//get tx ok?
				if (!b_result)
					continue;
				n_result = future_read.get();//get response
				if (n_result == _mp::cclient::RESULT_SUCCESS)	b_result = true;
				else	b_result = false;
			} while (false);
			});

		thread_read.join();
	} while (false);

	unsigned char c_in_id = 0, c_out_id = 0;

	_sync_after(n_client_index, n_device_index, c_in_id, v_rx);

	if (b_result) {
		capi_client::help_strings_from_response(list_response, v_rx);
	}
	return b_result;
}

void capi_client::set_callbacks(unsigned long n_client_index, const _mp::cclient_cb::type_cb_callbacks & cbs)
{
	_create_cb_buffer(n_client_index);
	m_map_cur_callbacks[n_client_index] = std::make_shared<_mp::cclient_cb::type_cb_callbacks>();
	m_map_cur_callbacks[n_client_index]->cb_resolve = cbs.cb_resolve;
	m_map_cur_callbacks[n_client_index]->p_user_resolve = cbs.p_user_resolve;
	m_map_cur_callbacks[n_client_index]->cb_connect = cbs.cb_connect;
	m_map_cur_callbacks[n_client_index]->p_user_connect = cbs.p_user_connect;
	m_map_cur_callbacks[n_client_index]->cb_handshake = cbs.cb_handshake;
	m_map_cur_callbacks[n_client_index]->p_user_handshake = cbs.p_user_handshake;
	m_map_cur_callbacks[n_client_index]->cb_handshake_ssl = cbs.cb_handshake_ssl;
	m_map_cur_callbacks[n_client_index]->p_user_handshake_ssl = cbs.p_user_handshake_ssl;
	m_map_cur_callbacks[n_client_index]->cb_read = cbs.cb_read;
	m_map_cur_callbacks[n_client_index]->p_user_read = cbs.p_user_read;
	m_map_cur_callbacks[n_client_index]->cb_write = cbs.cb_write;
	m_map_cur_callbacks[n_client_index]->p_user_write = cbs.p_user_write;
	m_map_cur_callbacks[n_client_index]->cb_close = cbs.cb_close;
	m_map_cur_callbacks[n_client_index]->p_user_close = cbs.p_user_close;
	_set_callbacks(n_client_index, cbs);
}
void capi_client::set_callback_resolve(unsigned long n_client_index, const _mp::cclient_cb::type_cb_resolve cb, void* p_user)
{
	_create_cb_buffer(n_client_index);
	m_map_cur_callbacks[n_client_index]->cb_resolve = cb;
	m_map_cur_callbacks[n_client_index]->p_user_resolve = p_user;
	_set_callback_resolve(n_client_index, cb, p_user);
}
void capi_client::set_callback_connect(unsigned long n_client_index, const _mp::cclient_cb::type_cb_connect cb, void* p_user)
{
	_create_cb_buffer(n_client_index);
	m_map_cur_callbacks[n_client_index]->cb_connect = cb;
	m_map_cur_callbacks[n_client_index]->p_user_connect = p_user;
	_set_callback_connect(n_client_index, cb, p_user);
}
void capi_client::set_callback_handshake(unsigned long n_client_index, const _mp::cclient_cb::type_cb_handshake cb, void* p_user)
{
	_create_cb_buffer(n_client_index);
	m_map_cur_callbacks[n_client_index]->cb_handshake = cb;
	m_map_cur_callbacks[n_client_index]->p_user_handshake = p_user;
	_set_callback_handshake(n_client_index, cb, p_user);
}
void capi_client::set_callback_handshake_ssl(unsigned long n_client_index, const _mp::cclient_cb::type_cb_handshake_ssl cb, void* p_user)
{
	_create_cb_buffer(n_client_index);
	m_map_cur_callbacks[n_client_index]->cb_handshake_ssl = cb;
	m_map_cur_callbacks[n_client_index]->p_user_handshake_ssl = p_user;
	_set_callback_handshake_ssl(n_client_index, cb, p_user);
}
void capi_client::set_callback_read(unsigned long n_client_index, const _mp::cclient_cb::type_cb_read cb, void* p_user)
{
	_create_cb_buffer(n_client_index);
	m_map_cur_callbacks[n_client_index]->cb_read = cb;
	m_map_cur_callbacks[n_client_index]->p_user_read = p_user;
	_set_callback_read(n_client_index, cb, p_user);
}
void capi_client::set_callback_write(unsigned long n_client_index, const _mp::cclient_cb::type_cb_write cb, void* p_user)
{
	_create_cb_buffer(n_client_index);
	m_map_cur_callbacks[n_client_index]->cb_write = cb;
	m_map_cur_callbacks[n_client_index]->p_user_write = p_user;
	_set_callback_write(n_client_index, cb, p_user);
}
void capi_client::set_callback_close(unsigned long n_client_index, const _mp::cclient_cb::type_cb_close cb, void* p_user)
{
	_create_cb_buffer(n_client_index);
	m_map_cur_callbacks[n_client_index]->cb_close = cb;
	m_map_cur_callbacks[n_client_index]->p_user_close = p_user;
	_set_callback_close(n_client_index, cb, p_user);
}


capi_client::capi_client() :
	m_last_action(capi_client::act_none)
	, m_dw_last_result(_mp::cclient::RESULT_SUCCESS)
{
	_ini();
}


void capi_client::_create_cb_buffer(unsigned long n_client_index)
{
	_type_map_client_index_ptr_cb_callbacks::iterator it = m_map_cur_callbacks.find(n_client_index);
	if (it == std::end(m_map_cur_callbacks)) {
		m_map_cur_callbacks.insert(std::pair<unsigned long, _type_ptr_cb_callbacks>(
			n_client_index,
			std::make_shared<_mp::cclient_cb::type_cb_callbacks>()
		));
	}
}
void capi_client::_set_callbacks(unsigned long n_client_index, const _mp::cclient_cb::type_cb_callbacks & cbs)
{
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		//
		p_client->set_callbacks(cbs);
	} while (false);
}
void capi_client::_set_callback_resolve(unsigned long n_client_index, const _mp::cclient_cb::type_cb_resolve cb, void* p_user)
{
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		p_client->set_callback_resolve(cb, p_user);
	} while (false);
}
void capi_client::_set_callback_connect(unsigned long n_client_index, const _mp::cclient_cb::type_cb_connect cb, void* p_user)
{
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		p_client->set_callback_connect(cb, p_user);
	} while (false);
}
void capi_client::_set_callback_handshake(unsigned long n_client_index, const _mp::cclient_cb::type_cb_handshake cb, void* p_user)
{
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		p_client->set_callback_handshake(cb, p_user);
	} while (false);
}
void capi_client::_set_callback_handshake_ssl(unsigned long n_client_index, const _mp::cclient_cb::type_cb_handshake_ssl cb, void* p_user)
{
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		p_client->set_callback_handshake_ssl(cb, p_user);
	} while (false);
}
void capi_client::_set_callback_read(unsigned long n_client_index, const _mp::cclient_cb::type_cb_read cb, void* p_user)
{
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		p_client->set_callback_read(cb, p_user);
	} while (false);
}
void capi_client::_set_callback_write(unsigned long n_client_index, const _mp::cclient_cb::type_cb_write cb, void* p_user)
{
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		p_client->set_callback_write(cb, p_user);
	} while (false);
}
void capi_client::_set_callback_close(unsigned long n_client_index, const _mp::cclient_cb::type_cb_close cb, void* p_user)
{
	do {
		_mp::cclient* p_client(_mp::cclient_manager::get_instance().get_client(n_client_index));
		if (p_client == nullptr)
			continue;
		p_client->set_callback_close(cb, p_user);
	} while (false);
}

void capi_client::_ini()
{
}


