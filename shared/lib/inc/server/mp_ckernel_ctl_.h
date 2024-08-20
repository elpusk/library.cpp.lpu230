#pragma once

#include <mutex>
#include <memory>
#include <map>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <server/mp_cio_packet.h>
#include <server/mp_cworker_ctl.h>


namespace _mp
{

	class ckernel_ctl : public _mp::cworker_ctl
	{
	private:
		typedef	std::pair<size_t, cdll_service::type_ptr_cdll_service>	_type_pair_ref_cnt_ptr_dll;
		typedef	std::map<std::wstring, _type_pair_ref_cnt_ptr_dll>		_type_map_path_pair_ref_cnt_ptr_dll;

	public:
		typedef	std::weak_ptr< ckernel_ctl >	type_wptr;
		typedef	std::shared_ptr< ckernel_ctl >	type_ptr;


	public:
		ckernel_ctl(unsigned long n_session);
		virtual ~ckernel_ctl();

		unsigned long get_session_number() const;

		/**
		 * process_for_manager_service.
		 *
		 * \param req_data_field - the data field of request.
		 * \param list_result - if return value must be started with "success", "pending" or "error"
		 * \param cb
		 * \param p_user
		 * \return
		 */
		type_pair_bool_result_bool_complete process_for_manager_service(const type_list_wstring& req_data_field, type_list_wstring& list_result, const type_cb_sd_execute cb, void* p_user);

		/**
		 * process_for_device_service_execute.
		 *
		 * \param req_data_field - the data field of request.
		 * \param list_result - if return value must be started with "success", "pending" or "error"
		 * \param p_fun_sd_device_io - device io function pointer
		 * \param cb
		 * \param p_user
		 * \return
		 */
		type_pair_bool_result_bool_complete process_for_device_service_execute(
			const type_list_wstring& req_data_field
			, type_list_wstring& list_result
			, const type_fun_sd_device_io p_fun_sd_device_io
			, ct_i_dev* p_dev
			, const type_cb_sd_execute cb
			, void* p_user
		);

		/**
		 * process_for_device_service_cancel.
		 *
		 * \param req_data_field - the data field of request.
		 * \param list_result - if return value must be started with "success", "pending" or "error"
		 * \param p_fun_sd_device_io - device io function pointer
		 * \param cb
		 * \param p_user
		 * \return
		 */
		:type_pair_bool_result_bool_complete process_for_device_service_cancel(
			const type_list_wstring& req_data_field
			, type_list_wstring& list_result
			, const type_fun_sd_device_cancel p_fun_sd_device_cancel
			, ct_i_dev* p_dev
		);

	protected://by _mp::cworker_ctl
		/**
		* executed by worker thread.
		* processing request.
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete
		*/
		virtual bool _execute(cio_packet& request);

		/**
		* executed by worker thread. when _execute return false(not complete),and none new request
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete(_continue() will be recalled at next time)
		*/
		virtual bool _continue(cio_packet& request);

	private:
		bool _execute_general_error_response(cio_packet& request, cio_packet& response, cio_packet::type_error_reason n_reason = cio_packet::error_reason_none);
		bool _execute_kernel_operation(cio_packet& request, cio_packet& response);
		bool _execute_kernel(const cio_packet& request, cio_packet& response);//device dependent kernal operation
		/**
		 * _service_dll_unload.
		 *
		 * \param
		 *		- s_dll : default/xxx.dll or 3thpart/xxx.dll
		 * \return
		 */
		bool _service_dll_unload(const std::wstring& s_dll);

		/**
		 * _service_dll_cancel.
		 *
		 * \param
		 *		- s_dll : default/xxx.dll or 3thpart/xxx.dll
		 *		- p_fun_sd_device_cancel : device cancel fnuction pointer
		 *		- p_dev : p_fun_sd_device_cancel()'s the first parameter.
		 * \return
		 */
		bool _service_dll_cancel(const std::wstring& s_dll, const type_fun_sd_device_cancel p_fun_sd_device_cancel, ct_i_dev* p_dev);

		/**
		 * _service_dll_execute.
		 *
		 * \param s_dll - default/xxx.dll or 3thpart/xxx.dll
		 * \param list_parameters[in] -  wstring list
		 * \param p_fun_sd_device_io[in] - device io function poniter
		 * \param p_dev[in] - p_fun_sd_device_io() 's the first parameter.
		 * \param cb
		 * \param p_user
		 * \return : true - start success.
		 *		   : else - error
		 */
		bool _service_dll_execute(
			const std::wstring& s_dll
			, const type_fun_sd_device_io p_fun_sd_device_io
			, ct_i_dev* p_dev
			, const type_list_wstring& list_parameters
			, const type_cb_sd_execute cb, void* p_user
		);

		/**
		 * _service_dll_removed. - you must call this function when session is removed.
		 *
		 * \param s_dll - default/xxx.dll or 3thpart/xxx.dll
		 * \return : none
		 */
		void _service_dll_removed(const std::wstring& s_dll);

		/**
		 * _check_service_path.
		 *
		 * \param s_dll - default/xxx.dll or 3thpart/xxx.dll
		 * \return dll name( dll name & extention only )
		 */
		std::wstring _check_service_path(bool& b_default, const std::wstring& s_dll);

		/**
		 * _service_dll_load_with_abs_path.
		 *
		 * \param
		 *		- s_dll : default/xxx.dll or 3thpart/xxx.dll
		 * \return
		 */
		bool _service_dll_load_with_abs_path(const std::wstring& s_dll_short);

	private:

	private:
		unsigned long m_n_session;

		std::mutex m_mutex_for_path;
		ckernel_ctl::_type_map_path_pair_ref_cnt_ptr_dll m_map_default_path_pair_ref_cnt_ptr_dll;
		ckernel_ctl::_type_map_path_pair_ref_cnt_ptr_dll m_map_3th_path_pair_ref_cnt_ptr_dll;
		type_set_wstring m_set_loaded_dll_path;// items are - default/xxx.dll or 3thpart/xxx.dll

	private://don't call these method.
		ckernel_ctl();
		ckernel_ctl(const ckernel_ctl&);
		ckernel_ctl& operator=(const ckernel_ctl&);
	};

}//_mp