#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include <mp_type.h>
#include <mp_cio_packet.h>

namespace _mp {
	/**
	* the queue of client callback
	*/
	class cclient_cb_qitem
	{
	public:
		typedef	std::shared_ptr< cclient_cb_qitem > type_ptr;


		/**
		* the summation of callbacks
		*/
		typedef	enum : int {
			index_undefined = -1,
			index_resolve = 0,
			index_connect,
			index_handshake,
			index_handshake_ssl,
			index_read,
			index_write,
			index_close,
			the_number_of_index // this member must be defined in the last.
		}type_callback_index;

	public:
		cclient_cb_qitem()
		{
			_reset();
		}

		cclient_cb_qitem(cclient_cb_qitem::type_callback_index index, unsigned long n_result)
		{
			_reset();
			m_callback_index = index;
			m_n_result = n_result;
		}
		cclient_cb_qitem(
			cio_packet::type_act c_action_code,
			unsigned long n_result,
			unsigned long n_device_index,
			unsigned char c_in_id,
			const type_v_buffer& v_rx) : 
			m_callback_index(cclient_cb_qitem::index_read),
			m_c_action_code(c_action_code),
			m_n_result(n_result),
			m_n_device_index(n_device_index),
			m_c_in_id(c_in_id),
			m_v_rx(v_rx)
		{

		}
		virtual ~cclient_cb_qitem()
		{

		}

		cclient_cb_qitem::type_callback_index get_type_index() const
		{
			return m_callback_index;
		}

		unsigned long get_result() const
		{
			return m_n_result;
		}

		cio_packet::type_act get_action_code() const
		{
			return m_c_action_code;
		}

		unsigned long get_device_index() const
		{
			return m_n_device_index;
		}

		unsigned char get_in_id() const
		{
			return m_c_in_id;
		}

		type_v_buffer get_rx() const
		{
			return m_v_rx;
		}


	private:
		void _reset()
		{
			m_callback_index = cclient_cb_qitem::index_undefined;
			m_n_result = 0;

			m_c_action_code = cio_packet::act_mgmt_unknown;
			m_n_device_index = 0;
			m_c_in_id = 0;
			m_v_rx.resize(0);
		}
	private:
		
		cclient_cb_qitem::type_callback_index m_callback_index;
		unsigned long m_n_result;//except index_read

		//for only index_read
		cio_packet::type_act m_c_action_code;
		unsigned long m_n_device_index;
		unsigned char m_c_in_id;
		type_v_buffer m_v_rx;

	};

}//the end of _mp namespace