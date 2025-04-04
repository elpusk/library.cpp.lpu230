#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <mp_type.h>
#include <server/mp_cio_packet.h>
#include <server/mp_cworker_ctl.h>

#include <hid/mp_clibhid_dev_info.h>

namespace _mp {

	/**
	 *
	 * this class is sigle tone.
	 * the instance of this class must be created in _mp::cserver instance.
	 * the saved information in this class.
	 * - the relation between session number and session name.
	 * - main (worker) control.
	 * - the relation between device index and device (worker) control.
	 * - the relation betwwen session number and the set of device index.
	 * - already opened device response error to open requset.
	 */
	class cctl_svr
	{
	public:
		typedef	std::set<unsigned short>	type_set_device_index;

	private:
		typedef	std::map<unsigned short, cworker_ctl::type_wptr>	_type_map_device_index_to_wptr_dev_ctl;
		typedef	std::map<unsigned short, cworker_ctl::type_ptr>	_type_map_device_index_to_ptr_dev_ctl;
		typedef	std::map<unsigned long, cworker_ctl::type_ptr>	_type_map_session_to_ptr_kernel_ctl;
		typedef	std::map<unsigned long, type_set_device_index>	_type_map_session_to_set_of_device_index;
		typedef	std::map<unsigned long, unsigned short>	_type_map_session_to_device_index;
		typedef	std::map<unsigned long, std::wstring>	_type_map_session_to_name;

	public:
		static cctl_svr& get_instance()
		{
			static std::shared_ptr<cctl_svr> ptr_obj(new cctl_svr());
			return *ptr_obj;
		}

		virtual ~cctl_svr();

		cctl_svr& create_kernel_ctl(clog* p_log, unsigned long n_session);

		void remove_kernel_ctl(unsigned long n_session);

		cctl_svr& create_main_ctl_and_set_callack(clog* p_log);

		// >> m_map_device_index_to_wptr_dev_ctl
		/**
		* register new device, device index is generated automatically.
		* update m_map_device_index_to_ptr_dev_ctl.
		* @return the created device index, if it is zero, error!
		*/
		unsigned short create_new_dev_ctl(clog* p_log, const clibhid_dev_info& item);


		/**
		* @brief remove cdev_ctl from m_map_device_index_to_ptr_dev_ctl.
		* update m_map_device_index_to_ptr_dev_ctl
		* 
		* @param s_device_path - device path
		* 
		* @return removed device index , fail - 0
		*/
		unsigned short remove_dev_ctl(const std::wstring& s_device_path);

		// >> for m_map_session_to_set_of_device_index
		cctl_svr::type_set_device_index get_device_index_set_of_session(unsigned long n_session);

		unsigned long get_session_number_from_device_index(unsigned short w_device_index);

		/**
		* indicate that the selected device(w_device_index) is used by the session(n_session)
		* open request by client.
		* updates m_map_session_to_set_of_device_index.
		*/
		void insert_device_index_to_device_index_set_of_session(unsigned long n_session, unsigned short w_device_index);

		/**
		* indicate that the selected device(w_device_index) isn't used by the session(n_session)
		* close request by client.
		* updates m_map_session_to_set_of_device_index.
		*/
		void erase_device_index_from_device_index_set_of_session(unsigned long n_session, unsigned short w_device_index);
		// <<

		// >> for m_map_session_to_name
		/**
		 * set_session_name.
		 * if the session have the name, this method will be failed.
		 *
		 * \param n_session - target session number
		 * \param s_session_name - the name of session
		 * \return boolean
		 */
		bool set_session_name(unsigned long n_session, const std::wstring& s_session_name);

		std::wstring get_session_name(unsigned long n_session);

		bool is_exist_session_name(const std::wstring& s_session_name);

		void remove_session_name(unsigned long n_session);
		unsigned long get_session_number_from_session_name(const std::wstring& s_session_name);
		std::wstring generate_session_name(unsigned long n_session);
		// <<

		void display(const std::wstring& s_info);
		void display(unsigned long  dw_color, const std::wstring& s_info);
		void display(unsigned long  dw_color, const type_v_buffer& v_data);

		bool push_request_to_worker_ctl(cio_packet::type_ptr& ptr_request, std::wstring& s_error_reason);

		bool close_all_device_of_session(unsigned long n_session);
		//
	private:
		cctl_svr();

		bool _push_back_request_to_kernel_ctl(cio_packet::type_ptr& ptr_cio_packet);
		bool _push_back_request_to_main_ctl(cio_packet::type_ptr& ptr_cio_packet);
		bool _push_back_request_to_dev_ctl(cio_packet::type_ptr& ptr_cio_packet);
		bool _push_back_request_to_dev_ctl(cio_packet::type_ptr& ptr_cio_packet, const std::wstring& s_device_path);

		void _erase_device_index_set_of_session(unsigned long n_session);

		cworker_ctl::type_ptr _get_dev_ctl(unsigned short w_device_index);
		cworker_ctl::type_ptr _get_dev_ctl(const std::wstring& s_device_path);

		std::wstring _get_device_path_from_device_index(unsigned short w_device_index);
		unsigned short _get_device_index_from_device_path(const std::wstring & s_path);
		
	private:
		//callback for device
		static void _cb_dev_pluginout(
			const clibhid_dev_info::type_set& set_inserted,
			const clibhid_dev_info::type_set& set_removed,
			const clibhid_dev_info::type_set& set_current,
			void* p_usr
		);

	private:
		//for session
		cctl_svr::_type_map_session_to_name m_map_session_to_name;	//save the name of session

		std::mutex m_mutex_map_session_to_kernel;
		cctl_svr::_type_map_session_to_ptr_kernel_ctl m_map_session_to_ptr_kernel_ctl;
		
		//for device
		cworker_ctl::type_ptr m_ptr_main_ctl;//for main worker ctl

		unsigned short m_w_new_device_index;//must be greater then equal zero.
		std::mutex m_mutex_device_map;
		cctl_svr::_type_map_device_index_to_ptr_dev_ctl m_map_device_index_to_ptr_dev_ctl;

		std::mutex m_mutex_map_session_to_device_index;
		cctl_svr::_type_map_session_to_set_of_device_index m_map_session_to_set_of_device_index;

	private://don't call these methods
		cctl_svr(const cctl_svr&);
		cctl_svr& operator=(const cctl_svr&);

	};

}//the end of _mp namespace

