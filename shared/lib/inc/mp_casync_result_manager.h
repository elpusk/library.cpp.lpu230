#pragma once

#include <mutex>
#include <deque>

#include  <mp_casync_parameter_result.h>

// inmportant notice.  all result index must be unique.
namespace _mp
{
	class casync_result_manager
	{
	public:
		enum {
			const_invalied_result_index = -1
		};
	private:
		enum {
			_const_max_result_index = 5
		};
		enum {
			_const_device_index_multiplied_by_value = 1000
		};

	public:
		typedef	std::map<int, casync_parameter_result::type_ptr_ct_async_parameter_result >	_type_map_result_index_ptr_ct_async_parameter_result;
		typedef std::shared_ptr< casync_result_manager::_type_map_result_index_ptr_ct_async_parameter_result >	_type_ptr_map_result_index_ptr_ct_async_parameter_result;

		typedef	std::deque<int>	_type_queue_result_index;
		typedef	std::shared_ptr< casync_result_manager::_type_queue_result_index >	_type_ptr_queue_result_index;

		typedef std::pair< casync_result_manager::_type_ptr_queue_result_index, casync_result_manager::_type_ptr_map_result_index_ptr_ct_async_parameter_result > _type_pair_info;

		typedef	std::map<unsigned long, casync_result_manager::_type_pair_info > _type_map_device_index_pair;

	public:
		static casync_result_manager& get_instance(const std::wstring& s_manager_name = std::wstring(), bool b_remove = false)
		{
			//default manager
			static casync_result_manager empty_mgnt;
			static std::shared_ptr<casync_result_manager> ptr_mgmt_default(std::make_shared<casync_result_manager>());
			static std::map< std::wstring, std::shared_ptr<casync_result_manager> > map_name_ptr_mgmt;

			if (s_manager_name.empty()) {
				if (ptr_mgmt_default) {
					if (b_remove) {
						ptr_mgmt_default.reset();
						return empty_mgnt;
					}
					return *ptr_mgmt_default;
				}
				return empty_mgnt;
			}

			if (map_name_ptr_mgmt.empty())
				return empty_mgnt;
			std::map< std::wstring, std::shared_ptr<casync_result_manager> >::iterator it = map_name_ptr_mgmt.find(s_manager_name);
			if (b_remove) {
				if (it != std::end(map_name_ptr_mgmt)) {
					if (it->second) {
						it->second.reset();
					}
					map_name_ptr_mgmt.erase(it);
				}
				return empty_mgnt;
			}
			if (it != std::end(map_name_ptr_mgmt)) {
				if (it->second) {
					return *(it->second);
				}
				return empty_mgnt;
			}

			map_name_ptr_mgmt[s_manager_name] = std::make_shared<casync_result_manager>();
			return *map_name_ptr_mgmt[s_manager_name];
		}

		// return result index
		int create_async_result(unsigned long n_device_index, casync_parameter_result::type_callback p_fun, void* p_para, HWND h_wnd, UINT n_msg)
		{
			int n_result_index(casync_result_manager::const_invalied_result_index);

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_result_manager);

				_type_map_device_index_pair::iterator it_map = m_map_device_index_pair.find(n_device_index);
				if (it_map == std::end(m_map_device_index_pair)) {
					//create new item
					std::pair<_type_map_device_index_pair::iterator, bool> pair_return =
						m_map_device_index_pair.emplace(
							n_device_index,
							std::make_pair(
								std::make_shared<_type_queue_result_index>()
								, std::make_shared<_type_map_result_index_ptr_ct_async_parameter_result>()
							)
						);

					if (!pair_return.second)
						continue;
					it_map = pair_return.first;
				}

				n_result_index = _generate_result_index(n_device_index);

				//_type_queue_result_index
				it_map->second.first->push_back(n_result_index);

				//operation _type_map_result_index_ptr_ct_async_parameter_result
				_type_map_result_index_ptr_ct_async_parameter_result::iterator it = it_map->second.second->find(n_result_index);
				if (it != std::end(*it_map->second.second)) {
					it_map->second.second->erase(it);
				}

				(*it_map->second.second)[n_result_index] =
					std::make_shared<casync_parameter_result>(p_fun, p_para, h_wnd, n_msg);

			} while (false);
			return n_result_index;
		}

		bool remove_async_result(unsigned long n_device_index, int n_result_index)
		{
			bool b_result(false);
			casync_parameter_result::type_ptr_ct_async_parameter_result ptr_will_be_removed(nullptr);

			do {
				if (n_result_index < 0)
					continue;

				//std::lock_guard<std::mutex> lock(m_mutex_for_result_manager);
				m_mutex_for_result_manager.lock();
				_type_map_device_index_pair::iterator it_map = m_map_device_index_pair.find(n_device_index);
				if (it_map == std::end(m_map_device_index_pair)) {
					m_mutex_for_result_manager.unlock();
					continue;
				}

				//erase q item
				_type_queue_result_index::iterator it_q = std::find(
					std::begin(*(it_map->second.first))
					, std::end(*(it_map->second.first))
					, n_result_index
				);

				if (it_q != std::end(*(it_map->second.first))) {
					it_map->second.first->erase(it_q);
				}

				//erase map item
				casync_result_manager::_type_map_result_index_ptr_ct_async_parameter_result::iterator it_map_async = it_map->second.second->find(n_result_index);
				if (it_map_async != std::end(*it_map->second.second)) {
					ptr_will_be_removed = it_map_async->second;
					it_map->second.second->erase(it_map_async);
				}

				m_mutex_for_result_manager.unlock();
				b_result = true;
			} while (false);

			if (b_result && ptr_will_be_removed) {
				//notince ptr_will_be_removed must be removed after m_mutex_for_result_manager unlock.
				//If you try removing ptr_will_be_removed before m_mutex_for_result_manager unlock, deadlock will be generated.
				ptr_will_be_removed.reset();//real removed.
			}

			return b_result;
		}
		bool remove_async_result(unsigned long n_device_index)
		{
			bool b_result(false);
			casync_result_manager::_type_ptr_map_result_index_ptr_ct_async_parameter_result ptr_map_will_be_removed(nullptr);
			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_result_manager);

				_type_map_device_index_pair::iterator it_map = m_map_device_index_pair.find(n_device_index);
				if (it_map == std::end(m_map_device_index_pair)) {
					continue;
				}

				ptr_map_will_be_removed = it_map->second.second;
				m_map_device_index_pair.erase(it_map);
				b_result = true;
			} while (false);
			if (b_result && ptr_map_will_be_removed) {
				//notince ptr_will_be_removed must be removed after m_mutex_for_result_manager unlock.
				//If you try removing ptr_will_be_removed before m_mutex_for_result_manager unlock, deadlock will be generated.
				ptr_map_will_be_removed.reset();//run real erase.
			}
			return b_result;
		}

		casync_parameter_result::type_ptr_ct_async_parameter_result& get_async_parameter_result_from_all_device(int n_result_index)
		{
			static casync_parameter_result::type_ptr_ct_async_parameter_result ptr_null;

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_result_manager);
				//
				_type_map_device_index_pair::iterator it_map = std::begin(m_map_device_index_pair);

				for (; it_map != std::end(m_map_device_index_pair); ++it_map) {
					_type_map_result_index_ptr_ct_async_parameter_result::iterator it = it_map->second.second->find(n_result_index);
					if (it != std::end(*it_map->second.second)) {
						return it->second;
					}
				}//end for
			} while (false);
			return ptr_null;
		}
		casync_parameter_result::type_ptr_ct_async_parameter_result& get_async_parameter_result(unsigned long n_device_index, int n_result_index)
		{
			static casync_parameter_result::type_ptr_ct_async_parameter_result ptr_null;

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_result_manager);
				_type_map_device_index_pair::iterator it_map = m_map_device_index_pair.find(n_device_index);
				if (it_map == std::end(m_map_device_index_pair)) {
					continue;
				}

				casync_result_manager::_type_map_result_index_ptr_ct_async_parameter_result::iterator it = it_map->second.second->find(n_result_index);
				if (it == std::end(*it_map->second.second)) {
					continue;
				}
				return it->second;
			} while (false);
			return ptr_null;
		}

		bool empty_queue(unsigned long n_device_index)
		{
			bool b_result(true);

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_result_manager);
				_type_map_device_index_pair::iterator it_map = m_map_device_index_pair.find(n_device_index);
				if (it_map == std::end(m_map_device_index_pair)) {
					continue;
				}

				if (!it_map->second.first) {
					continue;
				}
				b_result = it_map->second.first->empty();
			} while (false);
			return b_result;

		}
		int pop_result_index(unsigned long n_device_index, bool b_remove = true)
		{
			int n_result_index(casync_result_manager::const_invalied_result_index);

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_result_manager);
				_type_map_device_index_pair::iterator it_map = m_map_device_index_pair.find(n_device_index);
				if (it_map == std::end(m_map_device_index_pair)) {
					continue;
				}
				if (!it_map->second.first) {
					continue;
				}
				if (it_map->second.first->empty()) {
					continue;
				}

				n_result_index = it_map->second.first->front();
				if (b_remove)
					it_map->second.first->pop_front();
				//
			} while (false);
			return n_result_index;
		}

		void clear()
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result_manager);
			m_map_device_index_pair.clear();
		}

		virtual ~casync_result_manager() {}

		//don't call this default constructor. this member is used in this class member only.
		casync_result_manager() : m_s_manager_name()
		{}
	private:
		casync_result_manager(const std::wstring& s_manager_name) : m_s_manager_name(s_manager_name)
		{}

		int _generate_result_index(unsigned long n_device_index, bool b_remove = false)
		{
			static std::map<unsigned long, int> map_device_index_result_index_counter;
			int n_result_index(casync_result_manager::const_invalied_result_index);

			do {
				std::map<unsigned long, int>::iterator it_map = map_device_index_result_index_counter.find(n_device_index);
				if (it_map == std::end(map_device_index_result_index_counter)) {
					if (b_remove)
						continue;
					map_device_index_result_index_counter[n_device_index] = 0;
					n_result_index = map_device_index_result_index_counter[n_device_index];
					map_device_index_result_index_counter[n_device_index]++;
					n_result_index = n_result_index + n_device_index * casync_result_manager::_const_device_index_multiplied_by_value;
					continue;
				}

				if (b_remove) {
					map_device_index_result_index_counter.erase(it_map);
					continue;
				}
				//
				if (it_map->second > casync_result_manager::_const_max_result_index) {
					it_map->second = 0;
				}
				n_result_index = it_map->second;
				it_map->second++;
				n_result_index = n_result_index + n_device_index * casync_result_manager::_const_device_index_multiplied_by_value;
			} while (false);


			return n_result_index;
		}

	private:
		const std::wstring m_s_manager_name;
		std::mutex m_mutex_for_result_manager;
		_type_map_device_index_pair m_map_device_index_pair;

	private://don't call these methods
		casync_result_manager(const casync_result_manager&);
		casync_result_manager& operator=(const casync_result_manager&);
	};

}//the end of _mp