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
				m_map_index_uptr_cclient[m_n_index] = cclient::type_uptr(new cclient());
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
		cclient_manager() : m_n_index(cclient_manager::undefined_index)
		{
		}

	private:
		std::mutex m_mutex_member;
		_type_map_index_uptr_cclient m_map_index_uptr_cclient;
		unsigned long m_n_index;

	private:
		//don't call these methods
		cclient_manager(const cclient_manager&) = delete;
		cclient_manager& operator= (const cclient_manager&) = delete;

	};

}