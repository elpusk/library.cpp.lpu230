#include <websocket/mp_win_nt.h>
#include <server/mp_cclient_cb.h>

namespace _mp {
	cclient_cb::cclient_cb() :
		m_cb_user{ nullptr,NULL,nullptr,NULL, nullptr,NULL, nullptr,NULL, nullptr,NULL, nullptr,NULL, nullptr,NULL }
		, m_callback_index_current(cclient_cb_qitem::index_undefined)
		, m_n_resolve_result(0)
		, m_n_connect_result(0)
		, m_n_handshake_result(0)
		, m_n_handshake_ssl_result(0)
		, m_n_write_result(0)
		, m_n_close_result(0)
		, m_read_act_code(cio_packet::act_mgmt_unknown)
		, m_n_read_result(0)
		, m_n_read_device_index(0)
		, m_c_read_in_id(0)
		, vcworker<cclient_cb_qitem>(nullptr)
	{
	}

	cclient_cb::~cclient_cb()
	{
	}

	void cclient_cb::_run_user_callback_resolve(unsigned long n_result)
	{
		do {
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			if (m_cb_user.cb_resolve == nullptr)
				continue;
			m_cb_user.cb_resolve(n_result, m_cb_user.p_user_resolve);
		} while (false);
	}

	void cclient_cb::_run_user_callback_connect(unsigned long n_result)
	{
		do {
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			if (m_cb_user.cb_connect == nullptr)
				continue;
			m_cb_user.cb_connect(n_result, m_cb_user.p_user_connect);
		} while (false);
	}

	void cclient_cb::_run_user_callback_handshake(unsigned long n_result)
	{
		do {
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			if (m_cb_user.cb_handshake == nullptr)
				continue;
			m_cb_user.cb_handshake(n_result, m_cb_user.p_user_handshake);
		} while (false);
	}

	void cclient_cb::_run_user_callback_ssl_handshake(unsigned long n_result)
	{
		do {
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			if (m_cb_user.cb_handshake_ssl == nullptr)
				continue;
			m_cb_user.cb_handshake_ssl(n_result, m_cb_user.p_user_handshake_ssl);
		} while (false);
	}

	void cclient_cb::_run_user_callback_read(
		cio_packet::type_act act_code, 
		unsigned long n_result, 
		unsigned long n_device_index, 
		unsigned char c_in_id, 
		const type_v_buffer& v_rx
	)
	{
		do {
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			if (m_cb_user.cb_read == nullptr)
				continue;
			if (v_rx.empty()) {
				m_cb_user.cb_read((char)act_code, n_result, n_device_index, c_in_id, m_cb_user.p_user_read, 0, NULL);
				continue;
			}
			m_cb_user.cb_read((char)act_code, n_result, n_device_index, c_in_id, m_cb_user.p_user_read, (unsigned long)(v_rx.size()), (const unsigned char*)&v_rx[0]);
		} while (false);
	}

	void cclient_cb::_run_user_callback_write(unsigned long n_result)
	{
		do {
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			if (m_cb_user.cb_write == nullptr)
				continue;
			m_cb_user.cb_write(n_result, m_cb_user.p_user_write);
		} while (false);
	}

	void cclient_cb::_run_user_callback_close(unsigned long n_result)
	{
		do {
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			if (m_cb_user.cb_close == nullptr)
				continue;
			m_cb_user.cb_close(n_result, m_cb_user.p_user_close);
		} while (false);
	}


	/**
	* executed by worker thread.
	* processing request.
	* @paramter request request reference
	* @return true -> complete(with error or success), false -> not complete
	*/
	bool cclient_cb::_execute(cclient_cb_qitem::type_ptr& ptr_request)
	{
		switch (ptr_request->get_type_index()) {
		case cclient_cb_qitem::index_resolve:
			_run_user_callback_resolve(ptr_request->get_result());
			break;
		case cclient_cb_qitem::index_connect:
			_run_user_callback_connect(ptr_request->get_result());
			break;
		case cclient_cb_qitem::index_handshake:
			_run_user_callback_handshake(ptr_request->get_result());
			break;
		case cclient_cb_qitem::index_handshake_ssl:
			_run_user_callback_ssl_handshake(ptr_request->get_result());
			break;
		case cclient_cb_qitem::index_write:
			_run_user_callback_write(ptr_request->get_result());
			break;
		case cclient_cb_qitem::index_close:
			_run_user_callback_close(ptr_request->get_result());
			break;
		case cclient_cb_qitem::index_read:
			_run_user_callback_read(
				ptr_request->get_action_code()
				, ptr_request->get_result()
				, ptr_request->get_device_index()
				, ptr_request->get_in_id()
				, ptr_request->get_rx()
			);
			break;
		default:
			break;
		}//end switch

		return true;
	}

	/**
	* executed by worker thread. when _execute return false(not complete),and none new request
	* @paramter request request reference
	* @return true -> complete(with error or success), false -> not complete(_continue() will be recalled at next time)
	*/
	bool cclient_cb::_continue(cclient_cb_qitem::type_ptr& ptr_request)
	{
		return true;
	}


}//the end of _mp namespace.