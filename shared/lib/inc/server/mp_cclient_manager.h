#pragma once

#include <map>
#include <mutex>

#include <server/mp_cclient.h>

namespace _mp {
	class cclient_manager
	{
	public:
		enum : unsigned long {
			undefined_index = 0
		};
	private:
		typedef	std::map<unsigned long, cclient::type_uptr > _type_map_index_uptr_cclient;

	public:
		static cclient_manager& get_instance()
		{
			static cclient_manager obj;
			return obj;
		}
		~cclient_manager()
		{
			std::for_each(std::begin(m_map_index_uptr_cclient), std::end(m_map_index_uptr_cclient), [=](_type_map_index_uptr_cclient::value_type& pair_item) {
				if (pair_item.second) {
					pair_item.second.reset();
				}
				});
		}

		cclient_manager& set_timeout_ws_client_wait_for_ssl_handshake_complete(long long ll_msec_timeout)
		{
			m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete = ll_msec_timeout;
			return *this;
		}
		cclient_manager& set_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss(long long ll_msec_timeout)
		{
			m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss = ll_msec_timeout;
			return *this;
		}
		cclient_manager& set_timeout_ws_client_wait_for_idle_in_wss(long long ll_msec_timeout)
		{
			m_ll_msec_timeout_ws_client_wait_for_idle_in_wss = ll_msec_timeout;
			return *this;
		}
		cclient_manager& set_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws(long long ll_msec_timeout)
		{
			m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws = ll_msec_timeout;
			return *this;
		}
		cclient_manager& set_timeout_ws_client_wait_for_idle_in_ws(long long ll_msec_timeout)
		{
			m_ll_msec_timeout_ws_client_wait_for_idle_in_ws = ll_msec_timeout;
			return *this;
		}
		cclient_manager& set_timeout_ws_client_wait_for_async_connect_complete_in_wss(long long ll_msec_timeout)
		{
			m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss = ll_msec_timeout;
			return *this;
		}
		cclient_manager& set_timeout_ws_client_wait_for_async_connect_complete_in_ws(long long ll_msec_timeout)
		{
			m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws = ll_msec_timeout;
			return *this;
		}

		cclient* get_client(unsigned long n_index)
		{
			std::lock_guard<std::mutex> lock(m_mutex_member);
			if (m_map_index_uptr_cclient.empty())
				return nullptr;
			_type_map_index_uptr_cclient::iterator it = m_map_index_uptr_cclient.find(n_index);
			if (it == std::end(m_map_index_uptr_cclient))
				return nullptr;
			else
				return it->second.get();
		}

		unsigned long create_client()
		{
			unsigned long n_index(cclient_manager::undefined_index);
			do {
				std::lock_guard<std::mutex> lock(m_mutex_member);
				if (m_n_index == cclient_manager::undefined_index)
					++m_n_index;
				//
				_type_map_index_uptr_cclient::iterator it = m_map_index_uptr_cclient.find(m_n_index);
				if (it != std::end(m_map_index_uptr_cclient))
					continue;
				m_map_index_uptr_cclient[m_n_index] = cclient::type_uptr(new cclient(m_n_index));
				m_map_index_uptr_cclient[m_n_index]->set_timeout_ws_client_wait_for_ssl_handshake_complete(m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete)
					.set_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss(m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss)
					.set_timeout_ws_client_wait_for_idle_in_wss(m_ll_msec_timeout_ws_client_wait_for_idle_in_wss)
					.set_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws(m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws)
					.set_timeout_ws_client_wait_for_idle_in_ws(m_ll_msec_timeout_ws_client_wait_for_idle_in_ws)
					.set_timeout_ws_client_wait_for_async_connect_complete_in_wss(m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss)
					.set_timeout_ws_client_wait_for_async_connect_complete_in_ws(m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws);

				n_index = m_n_index;
				++m_n_index;
			} while (false);

			return n_index;
		}

		void destory_client(unsigned long n_index)
		{
			do {
				std::lock_guard<std::mutex> lock(m_mutex_member);
				_type_map_index_uptr_cclient::iterator it = m_map_index_uptr_cclient.find(n_index);
				if (it == std::end(m_map_index_uptr_cclient))
					continue;

				it->second.reset();
				m_map_index_uptr_cclient.erase(it);
			} while (false);
		}
	private:
		cclient_manager() :
			m_n_index(cclient_manager::undefined_index)
			, m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_SSL_HANSHAKE_COMPLETE_MSEC)
			, m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_WEBSOCKET_HANSHAKE_COMPLETE_IN_WSS_MSEC)
			, m_ll_msec_timeout_ws_client_wait_for_idle_in_wss(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_IDLE_IN_WSS_MSEC)
			, m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_WEBSOCKET_HANSHAKE_COMPLETE_IN_WS_MSEC)
			, m_ll_msec_timeout_ws_client_wait_for_idle_in_ws(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_IDLE_IN_WS_MSEC)
			, m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_ASYNC_CONNECT_COMPLETE_IN_WSS_MSEC)
			, m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_ASYNC_CONNECT_COMPLETE_IN_WS_MSEC)
		{
		}

	private:
		std::mutex m_mutex_member;
		_type_map_index_uptr_cclient m_map_index_uptr_cclient;
		unsigned long m_n_index;

		//timeout for client
		long long m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete;
		long long m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss;
		long long m_ll_msec_timeout_ws_client_wait_for_idle_in_wss;
		long long m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws;
		long long m_ll_msec_timeout_ws_client_wait_for_idle_in_ws;
		long long m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;
		long long m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws;


	private:
		//don't call these methods
		cclient_manager(const cclient_manager&) = delete;
		cclient_manager& operator= (const cclient_manager&) = delete;

	};

}