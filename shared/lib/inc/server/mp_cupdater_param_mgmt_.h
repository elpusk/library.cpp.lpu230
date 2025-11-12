#pragma once
#include <string>
#include <memory>

#include <server/mp_cupdater_param_.h>

namespace _mp {

	/**
	* manager of parameters stroage of lpu230_update 
	*/
	class cupdater_param_mgmt {
	public:
		typedef std::shared_ptr< _mp::cupdater_param_mgmt > type_ptr;
	private:
		typedef std::map< unsigned long, _mp::cupdater_param::type_ptr > _type_map;

	public:
		static cupdater_param_mgmt& get_instance();

	public:
		virtual ~cupdater_param_mgmt();
		/**
		* @brief get cupdater_param by session number.
		* @param n_session_number - session number of client.
		* @return cupdater_param::type_ptr - cupdater_param pointer. if not found, return nullptr.
		*/
		_mp::cupdater_param::type_ptr get(unsigned long n_session_number) const;
		/**
		* @brief insert cupdater_param by session number.
		* @param n_session_number - session number of client.
		* @param ptr_param - cupdater_param pointer.
		* @return first - true : inserted, false : already exist.
		* @retunr second - the created cupdater_param pointer.
		*/
		std::pair<bool,_mp::cupdater_param::type_ptr> insert(unsigned long n_session_number);
		/**
		* @brief erase cupdater_param by session number.
		* @param n_session_number - session number of client.
		* @return true - erased.
		*/
		bool erase(unsigned long n_session_number);

		/**
		* @brief remove all item of map.
		*/
		void clear();

	private:
		cupdater_param_mgmt();
	private:
		cupdater_param_mgmt::_type_map m_map;

	private:
		cupdater_param_mgmt(const cupdater_param_mgmt&) = delete;
		cupdater_param_mgmt& operator=(const cupdater_param_mgmt&) = delete;
	};

} //end _mp