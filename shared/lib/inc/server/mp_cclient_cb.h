#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include <mp_type.h>
#include <mp_vcworker.h>
#include <server/mp_cclient_cb_qitem.h>


namespace _mp {
	class cclient_cb : public vcworker<cclient_cb_qitem>
	{
	public://the definition of callback
		typedef void(__stdcall* type_cb_resolve)(unsigned long n_result, void* p_user);
		typedef void(__stdcall* type_cb_connect)(unsigned long n_result, void* p_user);
		typedef void(__stdcall* type_cb_handshake)(unsigned long n_result, void* p_user);
		typedef void(__stdcall* type_cb_handshake_ssl)(unsigned long n_result, void* p_user);
		typedef void(__stdcall* type_cb_read)(char c_action_code, unsigned long n_result, unsigned long n_device_index, unsigned char c_in_id, void* p_user, unsigned long n_rx, const unsigned char* s_rx);
		typedef void(__stdcall* type_cb_write)(unsigned long n_result, void* p_user);
		typedef void(__stdcall* type_cb_close)(unsigned long n_result, void* p_user);

		typedef struct {

			cclient_cb::type_cb_resolve cb_resolve;
			void* p_user_resolve;
			cclient_cb::type_cb_connect cb_connect;
			void* p_user_connect;
			cclient_cb::type_cb_handshake cb_handshake;
			void* p_user_handshake;
			cclient_cb::type_cb_handshake_ssl cb_handshake_ssl;
			void* p_user_handshake_ssl;
			cclient_cb::type_cb_read cb_read;
			void* p_user_read;
			cclient_cb::type_cb_write cb_write;
			void* p_user_write;
			cclient_cb::type_cb_close cb_close;
			void* p_user_close;

		}type_cb_callbacks;

	public:
		typedef	std::shared_ptr< cclient_cb>	type_ptr;


	private:
		enum {
			_const_wait_time_for_pushed = 100
		};

	public:
		cclient_cb();
		virtual ~cclient_cb();

		void push_back_resolve(unsigned long n_result)
		{
			cclient_cb_qitem::type_ptr ptr(std::make_shared<cclient_cb_qitem>(cclient_cb_qitem::index_resolve, n_result));
			push_back_request(ptr);
		}
		void push_back_connect(unsigned long n_result)
		{
			cclient_cb_qitem::type_ptr ptr(std::make_shared<cclient_cb_qitem>(cclient_cb_qitem::index_connect, n_result));
			push_back_request(ptr);
		}
		void push_back_handshake(unsigned long n_result)
		{
			cclient_cb_qitem::type_ptr ptr(std::make_shared<cclient_cb_qitem>(cclient_cb_qitem::index_handshake, n_result));
			push_back_request(ptr);
		}
		void push_back_handshake_ssl(unsigned long n_result)
		{
			cclient_cb_qitem::type_ptr ptr(std::make_shared<cclient_cb_qitem>(cclient_cb_qitem::index_handshake_ssl, n_result));
			push_back_request(ptr);
		}
		void push_back_write(unsigned long n_result)
		{
			cclient_cb_qitem::type_ptr ptr(std::make_shared<cclient_cb_qitem>(cclient_cb_qitem::index_write, n_result));
			push_back_request(ptr);
		}
		void push_back_close(unsigned long n_result)
		{
			cclient_cb_qitem::type_ptr ptr(std::make_shared<cclient_cb_qitem>(cclient_cb_qitem::index_close, n_result));
			push_back_request(ptr);
		}
		void push_back_read(
			cio_packet::type_act c_action_code,
			unsigned long n_result,
			unsigned long n_device_index,
			unsigned char c_in_id,
			const type_v_buffer& v_rx
		)
		{
			cclient_cb_qitem::type_ptr ptr(std::make_shared<cclient_cb_qitem>(
				c_action_code,
				n_result,
				n_device_index,
				c_in_id,
				v_rx
			)
			);
			push_back_request(ptr);
		}

		type_cb_callbacks get_callbacks()
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			return m_cb_user;
		}
		type_cb_resolve get_callback_resolve()
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			return m_cb_user.cb_resolve;
		}
		type_cb_connect get_callback_connect()
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			return m_cb_user.cb_resolve;
		}
		type_cb_handshake get_callback_handshake()
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			return m_cb_user.cb_handshake;
		}
		type_cb_handshake_ssl get_callback_ssl_handshake()
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			return m_cb_user.cb_handshake_ssl;
		}
		type_cb_read get_callback_read()
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			return m_cb_user.cb_read;
		}
		type_cb_write get_callback_write()
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			return m_cb_user.cb_write;
		}
		type_cb_close get_callback_close()
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			return m_cb_user.cb_close;
		}

		cclient_cb& set_callbacks(const type_cb_callbacks& cbs)
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			m_cb_user = cbs;
			return *this;
		}
		cclient_cb& set_callback_resolve(const type_cb_resolve cb, void* p_user)
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			m_cb_user.cb_resolve = cb;
			m_cb_user.p_user_resolve = p_user;
			return *this;
		}
		cclient_cb& set_callback_connect(const type_cb_connect cb, void* p_user)
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			m_cb_user.cb_connect = cb;
			m_cb_user.p_user_connect = p_user;
			return *this;
		}
		cclient_cb& set_callback_handshake(const type_cb_handshake cb, void* p_user)
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			m_cb_user.cb_handshake = cb;
			m_cb_user.p_user_handshake = p_user;
			return *this;
		}
		cclient_cb& set_callback_handshake_ssl(const type_cb_handshake_ssl cb, void* p_user)
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			m_cb_user.cb_handshake_ssl = cb;
			m_cb_user.p_user_handshake_ssl = p_user;
			return *this;
		}
		cclient_cb& set_callback_read(const type_cb_read cb, void* p_user)
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			m_cb_user.cb_read = cb;
			m_cb_user.p_user_read = p_user;
			return *this;
		}
		cclient_cb& set_callback_write(const type_cb_write cb, void* p_user)
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			m_cb_user.cb_write = cb;
			m_cb_user.p_user_write = p_user;
			return *this;
		}
		cclient_cb& set_callback_close(const type_cb_close cb, void* p_user)
		{
			std::lock_guard<std::mutex> lock(m_mutex_cb);
			m_cb_user.cb_close = cb;
			m_cb_user.p_user_close = p_user;
			return *this;
		}

	protected:
		/**
		* executed by worker thread.
		* processing request.
		* @paramter new request ptr
		* @paramter current request ptr
		* @return the current request ptr :
		*
		*	if the stored pointer is a null pointer -> complete(with error or success),
		*
		*	else -> not complete( need more running by _continue() ).
		*/
		virtual cclient_cb_qitem::type_ptr _execute(cclient_cb_qitem::type_ptr& ptr_req_new, const cclient_cb_qitem::type_ptr& ptr_req_cur);

		/**
		* executed by worker thread. when the return of _execute() is allocated(not complete),and none new request
		* @paramter current request ptr
		* @return true -> complete(with error or success), false -> not complete(_continue() will be recalled at next time)
		*/
		virtual bool _continue(cclient_cb_qitem::type_ptr& ptr_req_cur);

	private:
		void _run_user_callback_resolve(unsigned long n_result);
		void _run_user_callback_connect(unsigned long n_result);
		void _run_user_callback_handshake(unsigned long n_result);
		void _run_user_callback_ssl_handshake(unsigned long n_result);
		void _run_user_callback_read(
			cio_packet::type_act act_code
			, unsigned long n_result
			, unsigned long n_device_index
			, unsigned char c_in_id
			, const type_v_buffer& v_rx
		);
		void _run_user_callback_write(unsigned long n_result);
		void _run_user_callback_close(unsigned long n_result);


	private:
		std::mutex m_mutex_q;
		//temp parameter for hyper accesslation
		cclient_cb_qitem::type_callback_index m_callback_index_current;
		unsigned long m_n_resolve_result;
		unsigned long m_n_connect_result;
		unsigned long m_n_handshake_result;
		unsigned long m_n_handshake_ssl_result;
		unsigned long m_n_write_result;
		unsigned long m_n_close_result;

		cio_packet::type_act m_read_act_code;
		unsigned long m_n_read_result;
		unsigned long m_n_read_device_index;
		unsigned char m_c_read_in_id;
		type_ptr_v_buffer m_ptr_read_v_rx;


		std::mutex m_mutex_cb;
		type_cb_callbacks m_cb_user;

	private://don't call these methods
		cclient_cb(const cclient_cb&);
		cclient_cb& operator=(const cclient_cb&);

	};

}//the end of _mp namespace